//
// SliProSuperPro
// A Shift Light Indicator controller
// Copyright 2024 Fixfactory
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

const char *kConfigFileName = "LiveForSpeed.Config.json";
const char *kCarDataFileName = "LiveForSpeed.CarData.json";

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
    readCarData();
    readConfig();
}

void TelemetryManager::deinit()
{
}

bool TelemetryManager::fetchTelemetryData()
{
    m_telemetryData.gear = 0;
    m_telemetryData.rpm = 0.f;
    m_telemetryData.speedKph = 3.6f;

    m_carPath = "fox";
    parseCarData();

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

void TelemetryManager::readConfig()
{
    std::ifstream file(kConfigFileName);
    if (!file.good())
    {
        LOG_ERROR("Could not open %s", kConfigFileName);
        return;
    }

    auto config = json::parse(file);

    if (!config.empty())
    {
        auto inSim = config["InSim"];
        if (!inSim.empty())
        {
            auto hostname = inSim["hostname"];
            if (!hostname.empty())
            {
                m_inSimHostname = hostname.template get<std::string>();
            }

            auto port = inSim["port"];
            if (!port.empty())
            {
                m_inSimPort = port.template get<int>();
            }

            auto password = inSim["password"];
            if (!password.empty())
            {
                m_inSimPassword = password.template get<std::string>();
            }
        }
    }
}

void TelemetryManager::readCarData()
{
    std::ifstream file(kCarDataFileName);
    if (file.good())
    {
        LOG_INFO("Reading %s", kCarDataFileName);
        m_carData = json::parse(file);
    }
}

void TelemetryManager::parseCarData()
{
    memset(m_physicsData.rpmDownshift, 0, sizeof(m_physicsData.rpmDownshift));
    memset(m_physicsData.rpmUpshift, 0, sizeof(m_physicsData.rpmUpshift));

    if (m_carPath.empty() || m_carData.empty())
    {
        return;
    }

    auto carData = m_carData["cars"][m_carPath];
    if (carData.empty())
    {
        return;
    }

    auto finalGear = carData["finalGear"];
    if (!finalGear.empty())
    {
        m_physicsData.gearCount = finalGear.template get<int>() + 2; // Add reverse and neutral
    }
    
    auto rpmLimit = carData["rpmLimit"];
    if (!rpmLimit.empty())
    {
        m_physicsData.rpmLimit = rpmLimit.template get<float>();
    }    

    if (!carData["firstRPM"].empty() && !carData["lastRPM"].empty())
    {
        float firstRPM = carData["firstRPM"].template get<float>();
        float lastRPM = carData["lastRPM"].template get<float>();

        for (int i = 0; i < plugin::kMaxGearCount; i++)
        {
            m_physicsData.rpmDownshift[i] = firstRPM;
            m_physicsData.rpmUpshift[i] = lastRPM;
        }
    }
    else
    {
        for (auto &gear : carData["gears"].items())
        {
            if (gear.value()["gear"].empty() || gear.value()["firstRPM"].empty() || gear.value()["lastRPM"].empty())
            {
                continue;
            }

            std::string gearName = gear.value()["gear"].template get<std::string>();
            float firstRPM = gear.value()["firstRPM"].template get<float>();
            float lastRPM = gear.value()["lastRPM"].template get<float>();

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
                m_physicsData.rpmDownshift[gearIdx] = firstRPM;
                m_physicsData.rpmUpshift[gearIdx] = lastRPM;
            }
        }
    }
}
