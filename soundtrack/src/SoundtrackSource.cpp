// Copyright Twitch Interactive, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0

// Copyright Twitch Interactive, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0

#define TWITCH_DEBUG_RAWDATA 0
#define TWITCH_DEBUG_TIMESTAMPS 0

#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include <Windows.h>

#include <libsoundtrackutil/SoundtrackIPC.h>
#include <math.h>
#include <obs-frontend-api.h>
#include <obs.h>
#include <util/bmem.h>
#include <util/platform.h>
#include <util/threading.h>
#include <algorithm>

#include "LockFreeList.h"
#include "Util.h"

using namespace Twitch::Audio;

struct SoundtrackPluginData {
    bool initialized_thread;
    pthread_t thread;
    os_event_t *event;
    obs_source_t *source;
    os_event_t *dataReady;

    std::unique_ptr<SoundtrackIPC> ipc;
    Twitch::Utility::LockFreeList<TwitchAudioPacket> receiveQueue;
};

void initIpc(SoundtrackPluginData *pluginData)
{
    auto callbacks = IPCCallbacks{[pluginData]() {},
        [pluginData]() {},
        [pluginData](TwitchAudioPacket packet) {
            pluginData->receiveQueue.push(packet);
            os_event_signal(pluginData->dataReady);
        }};

    pluginData->ipc = std::make_unique<SoundtrackIPC>(callbacks);
}

void destroyIpc(SoundtrackPluginData *pluginData)
{
    pluginData->ipc.reset();
}

static audio_format convertAudioFormat(const TwitchAudioPacket &p)
{
    switch(p.sampleFormat) {
    case SampleFormat::Unsigned8Bit:
        return p.isPlanar ? audio_format::AUDIO_FORMAT_U8BIT_PLANAR
                          : audio_format::AUDIO_FORMAT_U8BIT;

    case SampleFormat::Signed16Bit:
        return p.isPlanar ? audio_format::AUDIO_FORMAT_16BIT_PLANAR
                          : audio_format::AUDIO_FORMAT_16BIT;

    case SampleFormat::Signed32Bit:
        return p.isPlanar ? audio_format::AUDIO_FORMAT_32BIT_PLANAR
                          : audio_format::AUDIO_FORMAT_32BIT;

    case SampleFormat::Float:
        return p.isPlanar ? audio_format::AUDIO_FORMAT_FLOAT_PLANAR
                          : audio_format::AUDIO_FORMAT_FLOAT;

    default:
        return audio_format::AUDIO_FORMAT_UNKNOWN;
    }
}

static int64_t firstTs = 0;
static uint64_t lastTs = 0;

static void *soundtrackPluginThread(void *pdata)
{
    SoundtrackPluginData *pluginData = static_cast<SoundtrackPluginData *>(pdata);
    initIpc(pluginData);

    while(os_event_try(pluginData->event) == EAGAIN) {
        os_event_timedwait(pluginData->dataReady, 1000);
        pluginData->receiveQueue.consumeData([pluginData](const auto &p) {
            struct obs_source_audio data;
            data.frames = p.frameCount;
            data.speakers = static_cast<speaker_layout>(std::min(p.channelCount, MAX_AV_PLANES));
            data.samples_per_sec = p.sampleRate;
            data.format = convertAudioFormat(p);

#if !TWITCH_DEBUG_TIMESTAMPS
            data.timestamp = p.timestampInNs;
#else
            if(!firstTs) {
                firstTs = p.timestampInNs;
            }
            data.timestamp = p.timestampInNs - firstTs;
            //firstTs += 1000000000LL * data.frames / data.samples_per_sec;

            if(!lastTs) {
                lastTs = data.timestamp;
            }
            if(lastTs > 10000000000LL && lastTs != data.timestamp) {
                __debugbreak();
            }
            lastTs = data.timestamp + 1000000000LL * data.frames / data.samples_per_sec;
#endif

            auto bytesPerChannel = p.sampleSize() * p.frameCount;

            if(p.isPlanar) {
                for(auto i = 0; i < std::min(p.channelCount, MAX_AV_PLANES); i++) {
                    data.data[i] = p.audioData.data() + i * bytesPerChannel;
                }
            } else {
                data.data[0] = p.audioData.data();
            }

#if TWITCH_DEBUG_RAWDATA
            if(p.isPlanar) {
                FILE *file = fopen("out.wav", "rb+");
                fseek(file, 0, SEEK_END);
                for(int i = 0; i < p.channelCount; i++) {
                    int pos = ftell(file);
                    FILE *file2 = fopen("out.txt", "ab");
                    fprintf(file2, "planar %d : %6d frames @ %08x\n", i, p.frameCount, pos);
                    fclose(file2);

                    fwrite(data.data[i], 1, data.frames * p.sampleSize(), file);
                }
                fclose(file);
            } else {
                FILE *file = fopen("out.wav", "rb+");
                fseek(file, 0, SEEK_END);

                long pos = ftell(file);
                FILE *file2 = fopen("out.txt", "ab");
                fprintf(file2, "interleaved : %6d frames @ %08lx\n", p.frameCount, pos);
                fclose(file2);

                fwrite(data.data[0], 1, data.frames * p.sampleSize() * p.channelCount, file);
                fclose(file);
            }
#endif

            obs_source_output_audio(pluginData->source, &data);
        });
    }

    destroyIpc(pluginData);
    return NULL;
}

static const char *soundtrackGetName(void *)
{
    return "Soundtrack by Twitch Companion Audio Source";
}

static void soundtrackDestroy(void *data)
{
    SoundtrackPluginData *pluginData = static_cast<SoundtrackPluginData *>(data);
    if(pluginData) {
        if(pluginData->initialized_thread) {
            void *ret;
            os_event_signal(pluginData->event);
            os_event_signal(pluginData->dataReady);
            pthread_join(pluginData->thread, &ret);
        }

        os_event_destroy(pluginData->dataReady);
        os_event_destroy(pluginData->event);
        bfree(pluginData);
    }
}

static void *soundtrackCreate(obs_data_t *, obs_source_t *source)
{
    SoundtrackPluginData *pluginData =
        static_cast<SoundtrackPluginData *>(bzalloc(sizeof(SoundtrackPluginData)));
    pluginData->source = source;

    // Set output channels
    // Obs has by default 6 audio tracks that it supports. Under the advanced audio properties,
    // users can change which tracks their audio source points to. We set our custom archive encoder to
    // run on the 6th track (0-indexed). When we initialize this plugin, we want to only output this on
    // the custom encoder track and not on any of the others.
    setMixer(source, 0, false);
    setMixer(source, 1, false);
    setMixer(source, 2, false);
    setMixer(source, 3, false);
    setMixer(source, 4, false);
    setMixer(source, kSoundtrackArchiveTrackIdx, true);

    if(os_event_init(&pluginData->event, os_event_type::OS_EVENT_TYPE_MANUAL) != 0) {
        soundtrackDestroy(pluginData);
        return NULL;
    }
    if(os_event_init(&pluginData->dataReady, os_event_type::OS_EVENT_TYPE_AUTO) != 0) {
        soundtrackDestroy(pluginData);
        return NULL;
    }
    if(pthread_create(&pluginData->thread, NULL, soundtrackPluginThread, pluginData) != 0) {
        soundtrackDestroy(pluginData);
        return NULL;
    }
    pluginData->initialized_thread = true;
    return pluginData;
}

struct obs_source_info soundtrack_source {
    "soundtrack_source", OBS_SOURCE_TYPE_INPUT, OBS_SOURCE_AUDIO, soundtrackGetName,
        soundtrackCreate, soundtrackDestroy,
};
