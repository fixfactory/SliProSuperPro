#pragma once

#include <chrono>

#include "Timing.h"

class SLIProDevice;

class DeviceManager
{
public:
    DeviceManager();
    ~DeviceManager();

    void init();
    void deinit();

    void update(timing::seconds deltaTime);

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
    SLIProDevice* m_sliPro = nullptr;
    time_point m_openedTime = {};
    State m_state = State::kIdle;
};