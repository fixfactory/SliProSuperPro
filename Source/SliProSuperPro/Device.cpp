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

#include <algorithm>
#include <sstream>

#include "Device.h"
#include "SLIProDevice.h"
#include "Config.h"
#include "PluginInterface.h"
#include "Process.h"
#include "Telemetry.h"
#include "Physics.h"

constexpr float kShiftLightBlinkHz = 4.f;
constexpr float kStalledBlinkHz = 8.f;
constexpr float kSpeedLimiterBlinkHz = 2.f;
constexpr float kStalledRPM = 750.f;
const std::chrono::milliseconds kStartupAnimationDuration{ 2000 };

DeviceManager &DeviceManager::getSingleton()
{
    static DeviceManager s_singleton;
    return s_singleton;
}

DeviceManager::DeviceManager()
{
}

DeviceManager::~DeviceManager()
{
}

void DeviceManager::init()
{
    TimingManager::getSingleton().registerUpdateable(this);

    m_sliPro = new SLIProDevice();
    m_sliPro->init();
}

void DeviceManager::deinit()
{
    TimingManager::getSingleton().unregisterUpdateable(this);

    if (m_sliPro->isOpen())
    {
        // Write a clear before closing so we don't leave the device illuminated.
        m_sliPro->clear();
        m_sliPro->write();
        m_sliPro->close();
    }
    m_sliPro->deinit();
    delete m_sliPro;
    m_sliPro = nullptr;
}

void DeviceManager::update(timing::seconds deltaTimeSecs)
{
    if (!m_sliPro->isOpen())
    {
        m_openedTime = {};
        m_sliPro->open();
    }

    if (m_sliPro->isOpen())
    {
        if (m_openedTime == time_point{})
        {
            m_openedTime = std::chrono::steady_clock::now();
            m_sliPro->clear();
            m_sliPro->setBrightness(config::brightness);
        }

        auto openedDuration = std::chrono::steady_clock::now() - m_openedTime;
        if (openedDuration < kStartupAnimationDuration)
        {
            setState(State::kStartupAnimation);
            setStartupAnimation(std::chrono::duration_cast<std::chrono::milliseconds>(openedDuration));
        }
        else if (TelemetryManager::getSingleton().isReceivingTelemetry() &&
                 PhysicsManager::getSingleton().hasPhysicsData())
        {
            setState(State::kReceivingTelemetry);
            setTelemetry();
        }
        else if (!ProcessManager::getSingleton().getGamePath().empty())
        {
            setState(State::kGameRunningNoTelemetry);
            setDashes();
        }
        else
        {
            setState(State::kIdle);
        }

        m_sliPro->write();
    }
}

void DeviceManager::setStartupAnimation(std::chrono::milliseconds openedDuration)
{
    float progress = (float)openedDuration.count() / (float)kStartupAnimationDuration.count();
    m_sliPro->setRpmLed(progress);
    m_sliPro->setLeftString("   Sli");
    m_sliPro->setRightString("Pro   ");
}

void DeviceManager::setTelemetry()
{
    const plugin::PhysicsData &physics = PhysicsManager::getSingleton().getPhysicsData();
    const plugin::TelemetryData &telemetry = TelemetryManager::getSingleton().getTelemetryData();

    const int gearIndex = std::clamp<int>(telemetry.gear, 0, plugin::kMaxGearCount - 1);
    const bool isReverse = gearIndex == 0;
    const bool isNeutral = gearIndex == 1;
    const bool isLastGear = gearIndex == physics.gearCount - 1;

    const float rpm = std::max<float>(telemetry.rpm, 0.f);
    float lowRPM = std::max<float>(physics.rpmDownshift[gearIndex], 0.f);
    float highRPM = std::max<float>(physics.rpmUpshift[gearIndex], 0.f);

    if (isReverse || isNeutral)
    {
        // Use the shift points of 1st gear so the range feels familiar.
        lowRPM = physics.rpmDownshift[2];
        highRPM = physics.rpmUpshift[2];
    }
    else if (isLastGear)
    {
        // Use the UpShift rpm from the previous gear so the range feels familiar.
        highRPM = physics.rpmUpshift[gearIndex - 1];
    }

    if (highRPM < lowRPM)
    {
        highRPM = lowRPM;
    }

    float rpmClamped = std::clamp<float>(rpm, lowRPM, highRPM);
    float rpmPercent = (rpmClamped - lowRPM) / (highRPM - lowRPM);

    int speed = std::max<int>((int)telemetry.speedKph % 1000, 0);
    char leftString[7];
    sprintf_s(leftString, "   %3i", speed);

    char rightString[7];
    if (physics.rpmLimit < 10000)
    {
        sprintf_s(rightString, "%4i  ", (int)std::max<float>(rpm, 0.f) % 10000);
    }
    else
    {
        sprintf_s(rightString, "%5i ", (int)std::max<float>(rpm, 0.f) % 100000);
    }

    bool isShiftLightOn = false;
    auto duration = std::chrono::steady_clock::now().time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    if (rpm >= highRPM && !isReverse && !isLastGear)
    {
        int blinkSteps = (int)(milliseconds.count() / (500 / kShiftLightBlinkHz));
        isShiftLightOn = blinkSteps % 2;
    }

    if (telemetry.speedLimiter)
    {
        int blinkSteps = (int)(milliseconds.count() / (500 / kSpeedLimiterBlinkHz));
        rpmPercent = blinkSteps % 2 ? 1.f : 0.f;
    }

    // Engine is assumed stalled when lower than half the idle RPM.
    float stalledRPM = physics.rpmIdle > 0.f ? physics.rpmIdle / 2.f : kStalledRPM;
    bool isStalled = rpm < stalledRPM;   
    if (isStalled)
    {
        int blinkSteps = (int)(milliseconds.count() / (500 / kStalledBlinkHz));
        rpmPercent = blinkSteps % 2 ? 1.f : 0.f;
    }

    char gearString[2];
    if (isNeutral)
    {
        gearString[0] = 'n';
    }
    else if (isReverse)
    {
        gearString[0] = 'r';
    }
    else
    {
        sprintf_s(gearString, "%u", (gearIndex - 1) % 10);
    }

    m_sliPro->setGear(gearString[0]);
    m_sliPro->setRpmLed(rpmPercent);
    m_sliPro->setShiftLights(isShiftLightOn);
    m_sliPro->setLeftString(leftString);
    m_sliPro->setRightString(rightString);
}

void DeviceManager::setDashes()
{
    m_sliPro->setLeftString("------");
    m_sliPro->setRightString("------");
    m_sliPro->setGear('_');
}

void DeviceManager::setState(State state)
{
    if (m_state != state)
    {
        // Clear when changing state so there isn't any left-over LEDs turned on.
        m_sliPro->clear();
    }
    m_state = state;
}
