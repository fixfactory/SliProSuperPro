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

#include "Timing.h"

class SLIProDevice;

class DeviceManager : public Updateable
{
public:
    static DeviceManager &getSingleton();

    DeviceManager();
    ~DeviceManager();

    void init();
    void deinit();

    void update(timing::seconds deltaTimeSecs) override;

private:
    using time_point = std::chrono::steady_clock::time_point;

    enum class State
    {
        kIdle,
        kStartupAnimation,
        kGameRunningNoTelemetry,
        kReceivingTelemetry
    };

    void setStartupAnimation(std::chrono::milliseconds openedDuration);
    void setTelemetry();
    void setDashes();
    void setState(State state);
    SLIProDevice *m_sliPro = nullptr;
    time_point m_openedTime = {};
    State m_state = State::kIdle;
};
