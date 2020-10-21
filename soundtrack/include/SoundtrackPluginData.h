
// Copyright Twitch Interactive, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0

#pragma once

#include "LockFreeList.h"
#include <libsoundtrackutil/SoundtrackIPC.h>
#include <util/threading.h>
#include <atomic>

using namespace Twitch::Audio;

struct SoundtrackPluginData {
    bool initialized_thread;
    pthread_t thread;
    os_event_t *event;
    obs_source_t *source;
    os_event_t *dataReady;

    std::atomic<bool> isConnected{false};

    std::unique_ptr<SoundtrackIPC> ipc;
    Twitch::Utility::LockFreeList<TwitchAudioPacket> receiveQueue;
};
