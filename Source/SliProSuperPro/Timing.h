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
} // namespace timing

class Updateable
{
public:
    virtual void update(timing::seconds deltaTimeSecs) = 0;
};

class TimingManager
{
public:
    static TimingManager &getSingleton();

    TimingManager();
    ~TimingManager();

    void init();
    void deinit();

    void registerUpdateable(Updateable *updateadle);
    void unregisterUpdateable(Updateable *updateadle);

    void run();

private:
    void preciseSleep(timing::seconds duration) const;

    std::vector<Updateable *> m_updateables;
    int m_frameNumber{ 0 };
    timing::seconds m_deltaTime{ 0.f };
    timing::seconds m_elapsedTime{ 0.f };
};
