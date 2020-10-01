// Copyright Twitch Interactive, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0

#pragma once

namespace Twitch::Audio {
static constexpr int kSoundtrackArchiveEncoderIdx = 1;
static constexpr int kSoundtrackArchiveTrackIdx = 5;

static inline void setMixer(obs_source_t *source, const int mixerIdx, const bool checked)
{
    uint32_t mixers = obs_source_get_audio_mixers(source);
    uint32_t new_mixers = mixers;
    if(checked) {
        new_mixers |= (1 << mixerIdx);
    } else {
        new_mixers &= ~(1 << mixerIdx);
    }
    obs_source_set_audio_mixers(source, new_mixers);
}

} // namespace Twitch::Audio
