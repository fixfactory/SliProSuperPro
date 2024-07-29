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
irsdkCVar g_OnPitRoad("OnPitRoad");
irsdkCVar g_PlayerCarSLFirstRPM("PlayerCarSLFirstRPM");
irsdkCVar g_PlayerCarSLShiftRPM("PlayerCarSLShiftRPM");
irsdkCVar g_PlayerCarSLLastRPM("PlayerCarSLLastRPM");
irsdkCVar g_PlayerCarSLBlinkRPM("PlayerCarSLBlinkRPM");

const float kMpsToKph = 3.6f;

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

    m_hasOverride = false;
    m_hardcoreLevel = 0;

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
    if (!irsdkClient::instance().waitForData(100))
    {
        return false;
    }

    // Voltage is 0 when out of the car.
    int voltage = g_Voltage.isValid() ? g_Voltage.getInt() : 0;
    bool isReplayPlaying = g_IsReplayPlaying.isValid() ? g_IsReplayPlaying.getBool() : true;
    if (voltage <= 0 || isReplayPlaying)
    {
        return false;
    }

    // Session string is not updated every frame.
    if (irsdkClient::instance().wasSessionStrUpdated())
    {
        const char *sessionStr = irsdkClient::instance().getSessionStr();
        if (sessionStr && sessionStr[0])
        {
            int driverCarIdx;
            parseYamlInt(sessionStr, "DriverInfo:DriverCarIdx:", &driverCarIdx);

            char tstr[256];
            sprintf_s(tstr, "DriverInfo:Drivers:CarIdx:{%d}CarPath:", driverCarIdx);
            char driverCarPath[256];
            parseYamlStr(sessionStr, tstr, driverCarPath, sizeof(driverCarPath) - 1);
            m_carPath = driverCarPath;
            
            int gearNumForward;
            parseYamlInt(sessionStr, "DriverInfo:DriverCarGearNumForward:", &gearNumForward);
            m_physicsData.gearCount = gearNumForward + 2; // Add reverse and neutral

            float redLineRPM;
            parseYamlFloat(sessionStr, "DriverInfo:DriverCarRedLine:", &redLineRPM);
            m_physicsData.rpmLimit = redLineRPM;

            parseYamlInt(sessionStr, "WeekendInfo:WeekendOptions:HardcoreLevel:", &m_hardcoreLevel);

            // This was iRacing's old way of specifying Shift Light RPM. It was specified in Session Data
            // and each gear had the same RPM values.
            float firstRPM;
            float shiftRPM;
            float lastRPM;
            float blinkRPM;

            parseYamlFloat(sessionStr, "DriverInfo:DriverCarSLFirstRPM:", &firstRPM);
            parseYamlFloat(sessionStr, "DriverInfo:DriverCarSLShiftRPM:", &shiftRPM);
            parseYamlFloat(sessionStr, "DriverInfo:DriverCarSLLastRPM:", &lastRPM);
            parseYamlFloat(sessionStr, "DriverInfo:DriverCarSLBlinkRPM:", &blinkRPM);

            for (int i = 0; i < plugin::kMaxGearCount; i++)
            {
                m_physicsData.rpmDownshift[i] = firstRPM;
                m_physicsData.rpmUpshift[i] = lastRPM;
            }

            parseOverrides();
        }
    }

    m_telemetryData.gear = g_Gear.isValid() ? g_Gear.getInt() + 1 : 0;
    m_telemetryData.rpm = g_RPM.isValid() ? g_RPM.getFloat() : 0.f;
    m_telemetryData.speedKph = g_Speed.isValid() ? g_Speed.getFloat() * kMpsToKph : 0.f;
    bool onPitRoad = g_OnPitRoad.isValid() ? g_OnPitRoad.getBool() : false;

    // With Hardcore Level 0 the player has to manually activate the speed limiter.
    // At Hardcore Level 1 the speed limiter is automatically activated on pit road.
    if (m_hardcoreLevel > 0 && onPitRoad)
    {
        m_telemetryData.speedLimiter = true;
    }
    else if (g_SpeedLimiter.isValid())
    {
        // Not every car has a speed limiter.
        m_telemetryData.speedLimiter = g_SpeedLimiter.getBool();
    }
    else
    {
        m_telemetryData.speedLimiter = false;
    }

    // This is iRacing's new way of specifying Shift Light RPM. It is now specified in Live Telemetry
    // and each gear can have different RPM values. The values are for the current gear.
    float gearFirstRPM = g_PlayerCarSLFirstRPM.isValid() ? g_PlayerCarSLFirstRPM.getFloat() : 0.f;
    float gearShiftRPM = g_PlayerCarSLShiftRPM.isValid() ? g_PlayerCarSLShiftRPM.getFloat() : 0.f;
    float gearLastRPM = g_PlayerCarSLLastRPM.isValid() ? g_PlayerCarSLLastRPM.getFloat() : 0.f;
    float gearBlinkRPM = g_PlayerCarSLBlinkRPM.isValid() ? g_PlayerCarSLBlinkRPM.getFloat() : 0.f;

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

    if (m_carPath.empty() || m_overrides.empty())
    {
        return;
    }

    json overrides = m_overrides["cars"][m_carPath];  
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

