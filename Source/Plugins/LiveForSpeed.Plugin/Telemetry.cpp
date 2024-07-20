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

#include "cinsim/CInsim.h" // Must include before Windows.h to avoid redefines

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
    openInSim();
}

void TelemetryManager::deinit()
{
    closeInSim();
}

bool TelemetryManager::fetchTelemetryData()
{
    if (CInsim::getInstance()->next_packet() < 0)
    {
        return false;
    }

    char packetType = CInsim::getInstance()->peek_packet();
    void *packet = CInsim::getInstance()->get_packet();

    switch (packetType)
    {
    case ISP_NPL: {
        // New player joining race
        IS_NPL *npl = reinterpret_cast<IS_NPL *>(packet);

        // 0 is local player
        if (npl->UCID == 0)
        {
            const size_t kSize = 8;
            char expanded_name[kSize];
            expand_prefix(npl->CName, expanded_name, kSize);
            m_carPath = expanded_name;
        }
    }
        break;

    default:
        break;
    }

    m_telemetryData.gear = 0;
    m_telemetryData.rpm = 0.f;
    m_telemetryData.speedKph = 3.6f;

    if (m_carPath != m_lastCarPath)
    {
        LOG_INFO("Player entered car id=%s", m_carPath.c_str());
        parseCarData();
        m_lastCarPath = m_carPath;
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
            else
            {
                LOG_ERROR("Missing hostname in config file");
            }

            auto port = inSim["port"];
            if (!port.empty())
            {
                m_inSimPort = port.template get<int>();
            }
            else
            {
                LOG_ERROR("Missing port in config file");
            }

            auto password = inSim["password"];
            if (!password.empty())
            {
                m_inSimPassword = password.template get<std::string>();
            }
        }
        else
        {
            LOG_ERROR("Missing InSim settings in config file");
        }
    }
    else
    {
        LOG_ERROR("Empty config file");
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
    memset(&m_physicsData, 0, sizeof(m_physicsData));

    if (m_carPath.empty())
    {
        return;
    }

    if (m_carData.empty())
    {
        LOG_WARN("Empty car data file");
        return;
    }

    auto carData = m_carData["cars"][m_carPath];
    if (carData.empty())
    {
        LOG_WARN("Car not found in car data file");
        return;
    }

    auto finalGear = carData["finalGear"];
    if (!finalGear.empty())
    {
        m_physicsData.gearCount = finalGear.template get<int>() + 2; // Add reverse and neutral
    }
    else
    {
        LOG_WARN("finalGear not specified in car data file");
    }
    
    auto rpmLimit = carData["rpmLimit"];
    if (!rpmLimit.empty())
    {
        m_physicsData.rpmLimit = rpmLimit.template get<float>();
    }    
    else
    {
        LOG_WARN("rpmLimit not specified in car data file");
    }

    int gearFound = 0;
    if (!carData["firstRPM"].empty() && !carData["lastRPM"].empty())
    {
        float firstRPM = carData["firstRPM"].template get<float>();
        float lastRPM = carData["lastRPM"].template get<float>();

        for (int i = 0; i < plugin::kMaxGearCount; i++)
        {
            m_physicsData.rpmDownshift[i] = firstRPM;
            m_physicsData.rpmUpshift[i] = lastRPM;
            gearFound++;
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
                gearFound++;
            }
        }
    }

    if (gearFound < m_physicsData.gearCount)
    {
        LOG_WARN("missing rpm info in car data file");
    }
}

void TelemetryManager::openInSim()
{
    CInsim::getInstance()->setHost(m_inSimHostname);
    CInsim::getInstance()->setTCPPort(m_inSimPort);
    CInsim::getInstance()->setPassword(m_inSimPassword);
    CInsim::getInstance()->setProduct("SliProSuperPro");
    CInsim::getInstance()->setVersion(9);

    if (CInsim::getInstance()->init() < 0)
    {
        LOG_ERROR("Could not open InSim connection at %s:%i", m_inSimHostname.c_str(), m_inSimPort);
        return;
    }

    LOG_INFO("InSim connection established at %s:%i", m_inSimHostname.c_str(), m_inSimPort);
}

void TelemetryManager::closeInSim()
{
    CInsim::getInstance()->disconnect();
    CInsim::getInstance()->removeInstance();
}
