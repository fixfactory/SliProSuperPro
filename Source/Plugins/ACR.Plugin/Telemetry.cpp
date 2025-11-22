//
// SliProSuperPro
// A Shift Light Indicator controller
// Copyright 2023-2025 Fixfactory
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

#include "SharedMemoryACCS/SharedFileOut.h"

#include "StringHelper.h"
#include "Telemetry.h"
#include "Log.h"

void dismiss(SMElement element)
{
    UnmapViewOfFile(element.mapFileBuffer);
    CloseHandle(element.hMapFile);

}

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
    memset(&m_telemetryData, 0, sizeof(m_telemetryData));
    memset(&m_physicsData, 0, sizeof(m_physicsData));
    memset(m_rpmDownshiftOverride, 0, sizeof(m_rpmDownshiftOverride));
    memset(m_rpmUpshiftOverride, 0, sizeof(m_rpmDownshiftOverride));

    m_lastCarPath.clear();
    m_hasOverride = false;
    m_gearCountOverride = 0;

    readOverrides();

    initPhysics();
    initGraphics();
    initStatic();
}

void TelemetryManager::initPhysics()
{
    TCHAR szName[] = TEXT("Local\\acpmf_physics");
    m_physics.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPageFilePhysics), szName);
    if (!m_physics.hMapFile)
    {
        LOG_ERROR("CreateFileMapping failed");
        return;
    }

    m_physics.mapFileBuffer = (unsigned char *)MapViewOfFile(m_physics.hMapFile, FILE_MAP_READ, 0, 0, sizeof(SPageFilePhysics));
    if (!m_physics.mapFileBuffer)
    {
        LOG_ERROR("MapViewOfFile failed");
        return;
    }
}

void TelemetryManager::initGraphics()
{
    TCHAR szName[] = TEXT("Local\\acpmf_graphics");
    m_graphics.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPageFileGraphic), szName);
    if (!m_graphics.hMapFile)
    {
        LOG_ERROR("CreateFileMapping failed");
        return;
    }

    m_graphics.mapFileBuffer = (unsigned char *)MapViewOfFile(m_graphics.hMapFile, FILE_MAP_READ, 0, 0, sizeof(SPageFileGraphic));
    if (!m_graphics.mapFileBuffer)
    {
        LOG_ERROR("MapViewOfFile failed");
        return;
    }
}

void TelemetryManager::initStatic()
{
    TCHAR szName[] = TEXT("Local\\acpmf_static");
    m_static.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SPageFileStatic), szName);
    if (!m_static.hMapFile)
    {
        LOG_ERROR("CreateFileMapping failed");
        return;
    }

    m_static.mapFileBuffer = (unsigned char *)MapViewOfFile(m_static.hMapFile, FILE_MAP_READ, 0, 0, sizeof(SPageFileStatic));
    if (!m_static.mapFileBuffer)
    {
        LOG_ERROR("MapViewOfFile failed");
        return;
    }
}

void TelemetryManager::deinit()
{
    dismiss(m_graphics);
    dismiss(m_physics);
    dismiss(m_static);
}

bool TelemetryManager::fetchTelemetryData()
{
    SPageFilePhysics *pfPhysics = (SPageFilePhysics *)m_physics.mapFileBuffer;
    SPageFileGraphic *pfGraphics = (SPageFileGraphic *)m_graphics.mapFileBuffer;
    SPageFileStatic *pfStatic = (SPageFileStatic *)m_static.mapFileBuffer;
    if (!pfPhysics || !pfGraphics || !pfStatic)
    {
        return false;
    }

    if (pfPhysics->rpms < 20)
    {
        return false;
    }

    m_carPath = string::convertFromWide(pfStatic->carModel);
    if (m_carPath != m_lastCarPath)
    {
        m_lastCarPath = m_carPath;
        LOG_INFO("Changed car: %s", m_carPath.c_str());
        parseOverrides();
    }

    m_telemetryData.gear = pfPhysics->gear;
    m_telemetryData.rpm = (float)pfPhysics->rpms;
    m_telemetryData.speedKph = pfPhysics->speedKmh;

    m_physicsData.gearCount = 8;
    m_physicsData.rpmIdle = 800.0f;
    m_physicsData.rpmLimit = (float)pfPhysics->currentMaxRpm;

    for (int i = 0; i < plugin::kMaxGearCount; i++)
    {
        m_physicsData.rpmDownshift[i] = m_physicsData.rpmLimit * 0.20f;
        m_physicsData.rpmUpshift[i] = m_physicsData.rpmLimit * 0.90f;
    }

    if (m_hasOverride)
    {
        m_physicsData.gearCount = m_gearCountOverride;
        memcpy(m_physicsData.rpmDownshift, m_rpmDownshiftOverride, sizeof(m_physicsData.rpmDownshift));
        memcpy(m_physicsData.rpmUpshift, m_rpmUpshiftOverride, sizeof(m_physicsData.rpmUpshift));
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

void TelemetryManager::readOverrides()
{
    std::ifstream file("ACR.Overrides.json");
    if (file.good())
    {
        LOG_INFO("Reading ACR.Overrides.json");
        m_overrides = json::parse(file);
    }
}

void TelemetryManager::parseOverrides()
{
    m_hasOverride = false;
    m_gearCountOverride = 0;
    memset(m_rpmDownshiftOverride, 0, sizeof(m_rpmDownshiftOverride));
    memset(m_rpmUpshiftOverride, 0, sizeof(m_rpmDownshiftOverride));

    if (m_carPath.empty() || m_overrides.empty())
    {
        return;
    }

    json overrides = m_overrides["cars"][m_carPath];  
    if (overrides.empty())
    {
        return;
    }

    if (!overrides["gearCount"].empty() && !overrides["rpmDownshift"].empty() && !overrides["rpmUpshift"].empty())
    {
        m_gearCountOverride = overrides["gearCount"].template get<int>() + 2; // Add reverse and neutral
        int rpmDownshift = overrides["rpmDownshift"].template get<int>();
        int rpmUpshift = overrides["rpmUpshift"].template get<int>();

        for (int i = 0; i < plugin::kMaxGearCount; i++)
        {
            m_rpmDownshiftOverride[i] = (float)rpmDownshift;
            m_rpmUpshiftOverride[i] = (float)rpmUpshift;
        }
    }
    else
    {
        for (auto &gear : overrides["gears"].items())
        {
            if (gear.value()["gear"].empty() || gear.value()["rpmDownshift"].empty() || gear.value()["rpmUpshift"].empty())
            {
                continue;
            }

            std::string gearName = gear.value()["gear"].template get<std::string>();
            int rpmDownshift = gear.value()["rpmDownshift"].template get<int>();
            int rpmUpshift = gear.value()["rpmUpshift"].template get<int>();

            int gearIdx = -1;
            if (gearName == "R")
            {
                gearIdx = 0;
            }
            else if (gearName == "N")
            {
                gearIdx = 1;
            }
            else
            {
                gearIdx = atoi(gearName.c_str());
                gearIdx++; // e.g. gear "1" is at index 2.
            }

            if (gearIdx >= 0 && gearIdx < plugin::kMaxGearCount)
            {
                m_rpmDownshiftOverride[gearIdx] = (float)rpmDownshift;
                m_rpmUpshiftOverride[gearIdx] = (float)rpmUpshift;
            }
        }
    }

    m_hasOverride = true;
}

