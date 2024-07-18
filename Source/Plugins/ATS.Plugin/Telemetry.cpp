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
#include <fstream>

#include "Telemetry.h"
#include "Log.h"
#include "SharedMemory.h"

// Name of shared memory map. Must match the name used in SPSP.ATS.Plugin.dll
const std::string kSharedMemoryName("SPSP.ATS.Plugin");

const float kMpsToKph = 3.6f;
const float kKphtoMph = 0.62137119f;

const std::chrono::duration<float> kTryOpenInterval{ 1.f };

#pragma pack(push)
#pragma pack(1)

// The layout of the shared memory.
struct TelemetryState
{
    unsigned __int8 running;
    float speedometer_speed;
    float rpm;
    float rpmLimit;
    signed __int32 gear;
    signed __int32 gearForwardCount;
    signed __int32 gearReverseCount;
};

#pragma pack(pop)

TelemetryManager &TelemetryManager::getSingleton()
{
    static TelemetryManager s_singleton;
    return s_singleton;
}

TelemetryManager::TelemetryManager()
{
}

TelemetryManager::~TelemetryManager()
{
}

void TelemetryManager::init()
{
    m_sharedMemory = new SharedMemory();
}

void TelemetryManager::deinit()
{
    if (m_sharedMemory)
    {
        delete m_sharedMemory;
        m_sharedMemory = nullptr;
    }
}

bool TelemetryManager::fetchTelemetryData()
{
    if (!m_sharedMemory->isOpened())
    {
        auto time = std::chrono::steady_clock::now();
        if (time - m_lastTryOpen > kTryOpenInterval)
        {
            m_sharedMemory->open(kSharedMemoryName, sizeof(TelemetryState), true);
            m_lastTryOpen = time;
        }
    }

    TelemetryState *telemetryState =
        reinterpret_cast<TelemetryState *>(m_sharedMemory->getBuffer());

    if (!telemetryState || !telemetryState->running)
    {
        return false;
    }
    
    // Return the speed in MPH because this is America.
    // TODO: Configurable speed units.
    m_telemetryData.speedKph = telemetryState->speedometer_speed * kMpsToKph * kKphtoMph;

    if (telemetryState->gear < 0)
    {
        m_telemetryData.gear = 0;
    }
    else
    {
        m_telemetryData.gear = telemetryState->gear + 1;
    }
    
    m_telemetryData.rpm = telemetryState->rpm;
    
    m_physicsData.rpmLimit = telemetryState->rpmLimit;
    m_physicsData.rpmIdle = 650.f;
    m_physicsData.gearCount = telemetryState->gearForwardCount;

    for (int i = 0; i < plugin::kMaxGearCount; ++i)
    {
        m_physicsData.rpmDownshift[i] = m_physicsData.rpmIdle;
        m_physicsData.rpmUpshift[i] = telemetryState->rpmLimit;
    }

    return true;
}

const plugin::TelemetryData &TelemetryManager::getTelemetryData() const
{
    return m_telemetryData;
}

const plugin::PhysicsData &TelemetryManager::getPhysicsData() const
{
    return m_physicsData;
}
