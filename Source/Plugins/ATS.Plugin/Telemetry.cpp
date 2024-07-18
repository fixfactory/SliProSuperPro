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
    m_sharedMemory = new SharedMemory(kSharedMemoryName, sizeof(TelemetryState));
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
    TelemetryState *telemetryState =
        reinterpret_cast<TelemetryState *>(m_sharedMemory ? m_sharedMemory->getBuffer() : NULL);

    if (!telemetryState || !telemetryState->running)
    {
        return false;
    }
    
    m_telemetryData.speedKph = telemetryState->speedometer_speed;
    m_telemetryData.gear = telemetryState->gear;
    m_telemetryData.rpm = telemetryState->rpm;
    
    m_physicsData.rpmLimit = telemetryState->rpmLimit;
    m_physicsData.gearCount = telemetryState->gearForwardCount;

    for (int i = 0; i < plugin::kMaxGearCount; ++i)
    {
        m_physicsData.rpmDownshift[i] = 300;
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
