#include <Windows.h>
#include <thread>
#include <math.h>

#include "Timing.h"

namespace timing
{
    seconds frameTime{ kMinFrameTime };
    seconds elapsedTime{ 0.f };
    int frameNumber{ 0 };

    void preciseSleep(seconds duration)
    {
        // Sleep in multiple steps in an attempt to increase accuracy.
        constexpr int kSleepSteps = 4;
        seconds totalSleepTime{ 0 };
        while (totalSleepTime < duration)
        {
            auto before = std::chrono::steady_clock::now();
            auto desiredSleepTime = (duration - totalSleepTime) / kSleepSteps;
            std::this_thread::sleep_for(desiredSleepTime);
            auto actualSleepTime = std::chrono::steady_clock::now() - before;
            totalSleepTime += actualSleepTime;
        }
    }
}