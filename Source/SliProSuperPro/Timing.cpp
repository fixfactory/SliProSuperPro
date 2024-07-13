//
// SliProSuperPro
// A Shift Light Indicator controller
// Copyright 2023 Fixfactory
//
// This file is part of SliProSuperPro.
//
// SliProSuperPro is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or any later version.
//
// SliProSuperPro is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along
// with SliProSuperPro. If not, see <http://www.gnu.org/licenses/>.
//

#include <Windows.h>
#include <thread>
#include <math.h>
#include <timeapi.h>

#include "Timing.h"
#include "Log.h"
#include "Config.h"

TimingManager &TimingManager::getSingleton()
{
    static TimingManager s_singleton;
    return s_singleton;
}

TimingManager::TimingManager()
{
}

TimingManager::~TimingManager()
{
}

void TimingManager::init()
{
    // Ask for 1ms timer so sleeps are more precise
    timeBeginPeriod(1);

    // The first delta time is the smallest value allowed.
    m_deltaTime = std::chrono::duration_cast<timing::seconds>(timing::kMinFrameTime);
}

void TimingManager::deinit()
{
    timeEndPeriod(1);
}

void TimingManager::registerUpdateable(Updateable *updateable)
{
    if (std::find(begin(m_updateables), end(m_updateables), updateable) != m_updateables.end())
    {
        LOG_ERROR("Registering Updateable twice.");
        return;
    }
    m_updateables.push_back(updateable);
}

void TimingManager::unregisterUpdateable(Updateable *updateable)
{
    if (std::find(begin(m_updateables), end(m_updateables), updateable) == m_updateables.end())
    {
        LOG_ERROR("Unregistering unkown Updateable.");
        return;
    }
    std::erase(m_updateables, updateable);
}

void TimingManager::run()
{
    auto before = std::chrono::steady_clock::now();

    // Update all updateables in the order they were registered.
    for (auto updateable : m_updateables)
    {
        updateable->update(m_deltaTime);
    }

    timing::seconds updateTime = std::chrono::steady_clock::now() - before;
    m_deltaTime = updateTime;

    // Framerate limiter. Prevent from updating too often and creating a busy loop.
    timing::seconds sleepTime{ 0.f };
    if (m_deltaTime < timing::kMinFrameTime)
    {
        sleepTime = timing::kMinFrameTime - m_deltaTime - timing::kMinWaitTime;
        if (sleepTime >= timing::kMinWaitTime)
        {
            preciseSleep(sleepTime);
        }
        else
        {
            sleepTime = timing::seconds{ 0.f };
        }
        m_deltaTime = std::chrono::steady_clock::now() - before;
    }

    // Prevent excessively long frames.
    // Can happen when debugging on a break-point.
    if (m_deltaTime > timing::kMaxFrameTime)
    {
        m_deltaTime = timing::kMaxFrameTime;
    }

    m_elapsedTime += m_deltaTime;
    m_frameNumber++;

    if (config::debugTiming)
    {
        LOG_INFO("Frame %i, updateTime %.3f, sleepTime %.3f, deltaTimeSecs %.3f, elapsedTime %.3f", m_frameNumber,
                 updateTime.count(), sleepTime.count(), m_deltaTime.count(), m_elapsedTime.count());
    }
}

void TimingManager::preciseSleep(timing::seconds duration) const
{
    // Sleep in multiple steps in an attempt to increase accuracy.
    constexpr int kSleepSteps = 4;
    timing::seconds totalSleepTime{ 0 };
    while (totalSleepTime < duration)
    {
        auto before = std::chrono::steady_clock::now();
        auto desiredSleepTime = (duration - totalSleepTime) / kSleepSteps;
        std::this_thread::sleep_for(desiredSleepTime);
        auto actualSleepTime = std::chrono::steady_clock::now() - before;
        totalSleepTime += actualSleepTime;
    }
}
