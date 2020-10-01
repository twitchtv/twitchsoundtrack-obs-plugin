// Copyright Twitch Interactive, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0

#pragma once

#include <obs-frontend-api.h>
#include <functional>

class OBSCallback {
public:
    OBSCallback(std::function<void(obs_frontend_event event)> callback)
        : _callback(std::move(callback))
    {
        obs_frontend_add_event_callback(handleEvent, reinterpret_cast<void *>(this));
    }
    ~OBSCallback()
    {
        obs_frontend_remove_event_callback(handleEvent, reinterpret_cast<void *>(this));
    }

private:
    std::function<void(obs_frontend_event event)> _callback;

    static void handleEvent(obs_frontend_event event, void *data)
    {
        reinterpret_cast<OBSCallback *>(data)->_callback(event);
    }
};
