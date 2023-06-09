#include <Windows.h>
#include <thread>
#include <math.h>

#include "Timing.h"
#include "Log.h"
#include "Config.h"

TimingManager& TimingManager::getSingleton()
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
    // The first delta time is the smallest value allowed.
    m_deltaTime = std::chrono::duration_cast<timing::seconds>(timing::kMinFrameTime);
}

void TimingManager::deinit()
{
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

void TimingManager::unregisterUpdateable(Updateable* updateable)
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
        LOG_INFO("Frame %i, updateTime %.3f, sleepTime %.3f, deltaTime %.3f, elapsedTime %.3f",
            m_frameNumber,
            updateTime.count(),
            sleepTime.count(),
            m_deltaTime.count(),
            m_elapsedTime.count());
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
