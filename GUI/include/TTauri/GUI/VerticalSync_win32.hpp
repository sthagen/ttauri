// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/hires_utc_clock.hpp"
#include <nonstd/span>
#include <thread>

namespace TTauri {

class VerticalSync_win32 {
    enum class State {
        ADAPTER_OPEN,
        ADAPTER_CLOSED,
        FALLBACK
    };

    State state;

    void *gdi;
    unsigned int adapter;
    unsigned int videoPresentSourceID;

    std::thread verticalSyncThreadID;
    bool stop = false;
    std::function<void(void *,hires_utc_clock::time_point)> callback;
    void *callbackData;

    hires_utc_clock::time_point previousFrameTimestamp;
    std::array<hires_utc_clock::duration,15> frameDurationData;
    size_t frameDurationDataCounter = 0;

    void openAdapter() noexcept;
    void closeAdapter() noexcept;

    /** Returns the median duration between two frames.
     */
    [[nodiscard]] hires_utc_clock::duration averageFrameDuration(hires_utc_clock::time_point frameTimestamp) noexcept;

    /** Waits for vertical-sync
     * @return Timestamp when the current frame will be displayed.
     */
    [[nodiscard]] hires_utc_clock::time_point wait() noexcept;

    void verticalSyncThread() noexcept;

public:
    VerticalSync_win32(std::function<void(void *,hires_utc_clock::time_point)> callback, void *callbackData) noexcept;
    ~VerticalSync_win32();
};

}