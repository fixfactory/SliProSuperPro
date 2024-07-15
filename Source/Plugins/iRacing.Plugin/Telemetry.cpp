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

#include "irsdk/irsdk_client.h"
#include "irsdk/yaml_parser.h"

irsdkCVar g_IsReplayPlaying("IsReplayPlaying");
irsdkCVar g_Voltage("Voltage");
irsdkCVar g_Gear("Gear");
irsdkCVar g_RPM("RPM");
irsdkCVar g_Speed("Speed");
irsdkCVar g_SpeedLimiter("dcPitSpeedLimiterToggle");
irsdkCVar g_PlayerCarSLFirstRPM("PlayerCarSLFirstRPM");
irsdkCVar g_PlayerCarSLShiftRPM("PlayerCarSLShiftRPM");
irsdkCVar g_PlayerCarSLLastRPM("PlayerCarSLLastRPM");
irsdkCVar g_PlayerCarSLBlinkRPM("PlayerCarSLBlinkRPM");

bool parseYamlInt(const char *yamlStr, const char *path, int *dest)
{
    if (dest)
    {
        (*dest) = 0;

        if (yamlStr && path)
        {
            int count;
            const char *strPtr;

            if (parseYaml(yamlStr, path, &strPtr, &count))
            {
                (*dest) = atoi(strPtr);
                return true;
            }
        }
    }

    return false;
}

bool parseYamlFloat(const char *yamlStr, const char *path, float *dest)
{
    if (dest)
    {
        (*dest) = 0.0f;

        if (yamlStr && path)
        {
            int count;
            const char *strPtr;

            if (parseYaml(yamlStr, path, &strPtr, &count))
            {
                (*dest) = (float)atof(strPtr);
                return true;
            }
        }
    }

    return false;
}

bool parseYamlStr(const char *yamlStr, const char *path, char *dest, int maxCount)
{
    if (dest && maxCount > 0)
    {
        dest[0] = '\0';

        if (yamlStr && path)
        {
            int count;
            const char *strPtr;

            if (parseYaml(yamlStr, path, &strPtr, &count))
            {
                // strip leading quotes
                if (*strPtr == '"')
                {
                    strPtr++;
                    count--;
                }

                int l = min(count, maxCount);
                strncpy_s(dest, maxCount, strPtr, l);
                dest[l] = '\0';

                // strip trailing quotes
                if (l >= 1 && dest[l - 1] == '"')
                    dest[l - 1] = '\0';

                return true;
            }
        }
    }

    return false;
}

TelemetryManager &TelemetryManager::getSingleton()
{
    static TelemetryManager s_singleton;
    return s_singleton;
}

TelemetryManager::TelemetryManager()
{
    memset(m_rpmDownshiftOverride, 0, sizeof(m_rpmDownshiftOverride));
    memset(m_rpmUpshiftOverride, 0, sizeof(m_rpmDownshiftOverride));
}

TelemetryManager::~TelemetryManager()
{
}

void TelemetryManager::init()
{
    readOverrides();
}

void TelemetryManager::deinit()
{

}

bool TelemetryManager::fetchTelemetryData()
{
    // Wait up to 100 ms for start of session or new data
    // We could put this on a thread if iRacing's rate fluctuates too much and is 
    // causing issues with the animations.
    if (irsdkClient::instance().waitForData(100))
    {
        // Voltage is 0 when out of the car.
        int voltage = g_Voltage.getInt();
        bool isReplayPlaying = g_IsReplayPlaying.getBool();
        if (voltage <= 0 || isReplayPlaying)
        {
            return false;
        }

        m_telemetryData.gear = g_Gear.getInt() + 1;
        m_telemetryData.rpm = g_RPM.getFloat();
        m_telemetryData.speedKph = g_Speed.getFloat() * 3.6f;
        
        // Not every car has a speed limiter.
        if (g_SpeedLimiter.isValid())
        {
            m_telemetryData.speedLimiter = g_SpeedLimiter.getBool();
        }        

        if (irsdkClient::instance().wasSessionStrUpdated())
        {
            const char *sessionStr = irsdkClient::instance().getSessionStr();
            if (sessionStr && sessionStr[0])
            {
                char tstr[256];

                int driverCarIdx;
                char driverCarPath[256];
                int gearNumForward;
                float redLineRPM;
                float firstRPM;
                float shiftRPM;
                float lastRPM;
                float blinkRPM;

                parseYamlInt(sessionStr, "DriverInfo:DriverCarIdx:", &driverCarIdx);
                sprintf_s(tstr, "DriverInfo:Drivers:CarIdx:{%d}CarPath:", driverCarIdx);
                parseYamlStr(sessionStr, tstr, driverCarPath, sizeof(driverCarPath) - 1);
                m_carPath = driverCarPath;
                parseOverrides();

                parseYamlInt(sessionStr, "DriverInfo:DriverCarGearNumForward:", &gearNumForward);
                parseYamlFloat(sessionStr, "DriverInfo:DriverCarRedLine:", &redLineRPM);
                parseYamlFloat(sessionStr, "DriverInfo:DriverCarSLFirstRPM:", &firstRPM);
                parseYamlFloat(sessionStr, "DriverInfo:DriverCarSLShiftRPM:", &shiftRPM);
                parseYamlFloat(sessionStr, "DriverInfo:DriverCarSLLastRPM:", &lastRPM);
                parseYamlFloat(sessionStr, "DriverInfo:DriverCarSLBlinkRPM:", &blinkRPM);

                m_physicsData.gearCount = gearNumForward + 2; // Add reverse and neutral
                m_physicsData.rpmLimit = redLineRPM;

                // This was iRacing's old way of specifying Shift Light RPM. It was specified in Session Data
                // and each gear had the same RPM values.
                for (int i = 0; i < plugin::kMaxGearCount; i++)
                {
                    m_physicsData.rpmDownshift[i] = firstRPM;
                    m_physicsData.rpmUpshift[i] = lastRPM;
                }
            }
        }

        // This is iRacing's new way of specifying Shift Light RPM. It is now specified in Live Telemetry
        // and each gear can have different RPM values. The values are for the current gear.
        float gearFirstRPM = g_PlayerCarSLFirstRPM.getFloat();
        float gearShiftRPM = g_PlayerCarSLShiftRPM.getFloat();
        float gearLastRPM = g_PlayerCarSLLastRPM.getFloat();
        float gearBlinkRPM = g_PlayerCarSLBlinkRPM.getFloat();

        if (gearFirstRPM != 0.f && gearLastRPM != 0.f && m_telemetryData.gear >= 0 &&
            m_telemetryData.gear < plugin::kMaxGearCount)
        {
            m_physicsData.rpmDownshift[m_telemetryData.gear] = gearFirstRPM;
            m_physicsData.rpmUpshift[m_telemetryData.gear] = gearLastRPM;
        }

        if (m_hasOverride)
        {
            memcpy(m_physicsData.rpmDownshift, m_rpmDownshiftOverride, sizeof(m_physicsData.rpmDownshift));
            memcpy(m_physicsData.rpmUpshift, m_rpmUpshiftOverride, sizeof(m_physicsData.rpmUpshift));
        }

        return true;
    }

    return false;
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
    std::ifstream file("iRacing.Overrides.json");
    if (file.good())
    {
        LOG_INFO("Reading iRacing.Overrides.json");
        m_overrides = json::parse(file);
    }
}

void TelemetryManager::parseOverrides()
{
    m_hasOverride = false;
    memset(m_rpmDownshiftOverride, 0, sizeof(m_rpmDownshiftOverride));
    memset(m_rpmUpshiftOverride, 0, sizeof(m_rpmDownshiftOverride));
    json overrides = nullptr;

    if (m_carPath.empty() || m_overrides.empty())
    {
        return;
    }

    for (auto &element : m_overrides["cars"].items())
    {
        auto carId = element.value()["id"].template get<std::string>();
        if (carId == m_carPath)
        {
            overrides = element.value();
            break;
        }
    }

    if (overrides.empty())
    {
        return;
    }

    if (!overrides["firstRPM"].empty() && !overrides["lastRPM"].empty())
    {
        int firstRPM = overrides["firstRPM"].template get<int>();
        int lastRPM = overrides["lastRPM"].template get<int>();

        for (int i = 0; i < plugin::kMaxGearCount; i++)
        {
            m_rpmDownshiftOverride[i] = (float)firstRPM;
            m_rpmUpshiftOverride[i] = (float)lastRPM;
        }
    }
    else
    {
        for (auto &gear : overrides["gears"].items())
        {
            if (gear.value()["gear"].empty() || gear.value()["firstRPM"].empty() || gear.value()["lastRPM"].empty())
            {
                continue;
            }

            std::string gearName = gear.value()["gear"].template get<std::string>();
            int firstRPM = gear.value()["firstRPM"].template get<int>();
            int lastRPM = gear.value()["lastRPM"].template get<int>();

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
                m_rpmDownshiftOverride[gearIdx] = (float)firstRPM;
                m_rpmUpshiftOverride[gearIdx] = (float)lastRPM;
            }
        }
    }

    m_hasOverride = true;
}

