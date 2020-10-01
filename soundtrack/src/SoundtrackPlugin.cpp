// Copyright Twitch Interactive, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0

#include <obs-frontend-api.h>
#include <obs-module.h>

#include "OBSCallback.h"
#include "Util.h"
#include "obs-source.h"
#include <util/config-file.h>
#include <util/dstr.h>
#include <optional>

OBS_DECLARE_MODULE()

OBS_MODULE_USE_DEFAULT_LOCALE("soundtrack-plugin", "en-US")

extern struct obs_source_info soundtrack_source;

namespace Twitch::Audio {
std::optional<OBSCallback> callback;
static obs_encoder_t *archiveEncoder = nullptr;
} // namespace Twitch::Audio

bool obs_module_load(void)
{
    auto minVersion = static_cast<uint32_t>(MAKE_SEMANTIC_VERSION(26, 0, 0));
    if(obs_get_version() < minVersion) {
        return false;
    }

    obs_register_source(&soundtrack_source);

    Twitch::Audio::callback.emplace([](obs_frontend_event event) {
        if(event == OBS_FRONTEND_EVENT_STREAMING_STARTING) {
            bool sourceExists = false;
            obs_enum_sources(
                [](void *p, obs_source_t *source) {
                    auto id = obs_source_get_id(source);
                    if(strcmp(id, "soundtrack_source") == 0) {
                        *reinterpret_cast<bool *>(p) = true;
                        return false;
                    }
                    return true;
                },
                reinterpret_cast<void *>(&sourceExists));

            // GATE on if the source is connected / initialized
            if(sourceExists) {
                // These are magic ints provided by OBS for default sources:
                // 0 is the main scene/transition which you'd see on the main preview,
                // 1-2 are desktop audio 1 and 2 as you'd see in audio settings,
                // 2-4 are mic/aux 1-3 as you'd see in audio settings
                auto desktopSource1 = obs_get_output_source(1);
                auto desktopSource2 = obs_get_output_source(2);

                // Since our plugin duplicates all of the desktop sources, we want to ensure that both of the
                // default desktop sources, provided by OBS, are not set to mix on our custom encoder track.
                Twitch::Audio::setMixer(
                    desktopSource1, Twitch::Audio::kSoundtrackArchiveTrackIdx, false);
                Twitch::Audio::setMixer(
                    desktopSource2, Twitch::Audio::kSoundtrackArchiveTrackIdx, false);

                obs_source_release(desktopSource1);
                obs_source_release(desktopSource2);

                if(!Twitch::Audio::archiveEncoder) {
                    Twitch::Audio::archiveEncoder = obs_audio_encoder_create("ffmpeg_aac",
                        "Soundtrack by Twitch Archive Encoder",
                        nullptr,
                        5, // This is hardcoded to the sixth audio track
                        nullptr); //release with obs_encoder_release() when encoder no longer being used
                    obs_encoder_set_audio(Twitch::Audio::archiveEncoder, obs_get_audio());
                }

                obs_output_t *streamingOutput = obs_frontend_get_streaming_output();
                obs_output_set_audio_encoder(streamingOutput,
                    Twitch::Audio::archiveEncoder,
                    Twitch::Audio::kSoundtrackArchiveEncoderIdx);
                obs_output_release(streamingOutput);

                config_t *config =
                    obs_frontend_get_profile_config(); //config does not need to be freed/released
                if(config) {
                    const char *mode = config_get_string(config, "Output", "Mode");
                    bool advanced = astrcmpi(mode, "Advanced") == 0;
                    int bitrate;
                    if(advanced) {
                        bitrate =
                            static_cast<int>(config_get_uint(config, "AdvOut", "Track1Bitrate"));
                    } else {
                        bitrate =
                            static_cast<int>(config_get_uint(config, "SimpleOutput", "ABitrate"));
                    }
                    obs_data_t *aacSettings = obs_data_create();
                    obs_data_set_int(aacSettings, "bitrate", bitrate);
                    obs_encoder_update(Twitch::Audio::archiveEncoder, aacSettings);
                    obs_data_release(aacSettings);
                }
            } else {
                auto desktopSource1 = obs_get_output_source(1);
                auto desktopSource2 = obs_get_output_source(2);

                // Since our plugin duplicates all of the desktop sources, we want to ensure that both of the
                // default desktop sources, provided by OBS, are not set to mix on our custom encoder track.
                Twitch::Audio::setMixer(
                    desktopSource1, Twitch::Audio::kSoundtrackArchiveTrackIdx, true);
                Twitch::Audio::setMixer(
                    desktopSource2, Twitch::Audio::kSoundtrackArchiveTrackIdx, true);

                obs_source_release(desktopSource1);
                obs_source_release(desktopSource2);

                obs_output_t *streamingOutput = obs_frontend_get_streaming_output();
                obs_output_set_audio_encoder(
                    streamingOutput, nullptr, Twitch::Audio::kSoundtrackArchiveEncoderIdx);
                obs_output_release(streamingOutput);
                if(Twitch::Audio::archiveEncoder) {
                    obs_encoder_release(Twitch::Audio::archiveEncoder);
                    Twitch::Audio::archiveEncoder = nullptr;
                }
            }
        }
    });

    return true;
}
