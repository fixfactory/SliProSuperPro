#pragma once

#include <chrono>
#include <vector>

namespace timing
{
    using seconds = std::chrono::duration<float>;
    constexpr int kMaxFps = 60;
    using FrameDuration = std::chrono::duration<long, std::ratio<1, kMaxFps>>;
    constexpr FrameDuration kMinFrameTime{ 1 };
    constexpr std::chrono::seconds kMaxFrameTime{ 2 };
    constexpr std::chrono::milliseconds kMinWaitTime{ 1 };
}

class Updateable
{
public:
    virtual void update(timing::seconds deltaTime) = 0;
};

class TimingManager
{
public:
    static TimingManager& getSingleton();

    TimingManager();
    ~TimingManager();

    void init();
    void deinit();

    void registerUpdateable(Updateable *updateadle);
    void unregisterUpdateable(Updateable* updateadle);

    void run();

private:
    void preciseSleep(timing::seconds duration) const;

    std::vector<Updateable*> m_updateables;
    int m_frameNumber{ 0 };
    timing::seconds m_deltaTime{ 0.f };
    timing::seconds m_elapsedTime{ 0.f };
};