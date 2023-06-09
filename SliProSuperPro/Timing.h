#pragma once

#include <chrono>

namespace timing
{
    using seconds = std::chrono::duration<float>;
    constexpr int kMaxFps = 60;
    using FrameDuration = std::chrono::duration<long, std::ratio<1, kMaxFps>>;
    constexpr FrameDuration kMinFrameTime{ 1 };
    constexpr std::chrono::seconds kMaxFrameTime{ 2 };
    constexpr std::chrono::milliseconds kMinWaitTime{ 1 };

    extern seconds frameTime;
    extern seconds elapsedTime;
    extern int frameNumber;

    void preciseSleep(std::chrono::duration<float> duration);
}
