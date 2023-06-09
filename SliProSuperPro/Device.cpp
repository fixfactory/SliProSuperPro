#include <algorithm>
#include <sstream>

#include "Device.h"
#include "SLIProDevice.h"
#include "Blackboard.h"
#include "NGP.h"
#include "Config.h"

#include "PhysicsNG/rbr.telemetry.data.TelemetryData.h"

constexpr float kShiftLightBlinkHz = 4.f;
constexpr float kStalledBlinkHz = 8.f;
constexpr float kStalledRPM = 750.f;
const std::chrono::milliseconds kStartupAnimationDuration { 2000 };

DeviceManager::DeviceManager()
{

}

DeviceManager::~DeviceManager()
{

}

void DeviceManager::init()
{
    m_sliPro = new SLIProDevice();
    m_sliPro->init();
}

void DeviceManager::deinit()
{
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

void DeviceManager::update(timing::seconds deltaTime)
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
        else if (blackboard::telemetryData != nullptr && blackboard::carPhysics != nullptr)
        {
            setState(State::kReceivingTelemetry);
            setTelemetry();
        }
        else if (!blackboard::gamePath.empty())
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
    const ngp::CarPhysics& car = *blackboard::carPhysics;
    int gearIndex = std::clamp<int>(blackboard::telemetryData->control_.gear_, 0, ngp::kGearCount - 1);
    float rpm = std::max<float>(blackboard::telemetryData->car_.engine_.rpm_, 0.f);
    float lowRPM = std::max<float>(car.controlUnit.gearDownShift[gearIndex], 0.f);
    float highRPM = std::max<float>(car.controlUnit.gearUpShift[gearIndex], 0.f);
    bool isReverse = gearIndex == 0;
    bool isNeutral = gearIndex == 1;
    bool isLastGear = gearIndex == car.drive.numberOfGears - 1;
    if (isReverse || isNeutral)
    {
        highRPM = car.controlUnit.rpmLimit;
    }
    else if (isLastGear)
    {
        // Use the UpShift rpm from the previous gear so the range feels familiar.
        highRPM = car.controlUnit.gearUpShift[gearIndex - 1];
    }

    if (highRPM < lowRPM)
    {
        highRPM = lowRPM;
    }

    float rpmClamped = std::clamp<float>(rpm, lowRPM, highRPM);
    float rpmPercent = (rpmClamped - lowRPM) / (highRPM - lowRPM);

    int speed = std::max<int>((int)blackboard::telemetryData->car_.speed_ % 1000, 0);
    char leftString[7];
    sprintf_s(leftString, "   %3i", speed);

    char rightString[7];
    sprintf_s(rightString, "%4i  ", (int)std::max<float>(rpm, 0.f) % 10000);

    bool isShiftLightOn = false;
    auto duration = std::chrono::steady_clock::now().time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    if (rpm >= highRPM && !isReverse && !isLastGear)
    {        
        int blinkSteps = (int)(milliseconds.count() / (500 / kShiftLightBlinkHz));
        isShiftLightOn = blinkSteps % 2;
    }

    bool isStalled = rpm < kStalledRPM;
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
    m_sliPro->setLeftString("   ---");
    m_sliPro->setRightString("----  ");
    m_sliPro->setGear('_');
}

void DeviceManager::setState(State state)
{
    if (m_state != state)
    {
        // Clear when changing state so there isn't left-over LEDs turned on.
        m_sliPro->clear();
    }
    m_state = state;
}