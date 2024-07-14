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

#include "Telemetry.h"
#include "Log.h"

#include "irsdk/irsdk_client.h"
#include "irsdk/yaml_parser.h"

irsdkCVar g_Gear("Gear");
irsdkCVar g_RPM("RPM");
irsdkCVar g_Speed("Speed");
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
}

TelemetryManager::~TelemetryManager()
{
}

void TelemetryManager::init()
{

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
        m_telemetryData.gear = g_Gear.getInt() + 1;
        m_telemetryData.rpm = g_RPM.getFloat();
        m_telemetryData.speedKph = g_Speed.getFloat();

        if (irsdkClient::instance().wasSessionStrUpdated())
        {
            const char *sessionStr = irsdkClient::instance().getSessionStr();
            if (sessionStr && sessionStr[0])
            {
                int gearNumForward;
                float firstRPM;
                float shiftRPM;
                float lastRPM;
                float blinkRPM;

                parseYamlInt(sessionStr, "DriverInfo:DriverCarGearNumForward:", &gearNumForward);
                parseYamlFloat(sessionStr, "DriverInfo:DriverCarSLFirstRPM:", &firstRPM);
                parseYamlFloat(sessionStr, "DriverInfo:DriverCarSLShiftRPM:", &shiftRPM);
                parseYamlFloat(sessionStr, "DriverInfo:DriverCarSLLastRPM:", &lastRPM);
                parseYamlFloat(sessionStr, "DriverInfo:DriverCarSLBlinkRPM:", &blinkRPM);

                m_physicsData.gearCount = gearNumForward + 2; // Add reverse and neutral

                // This is iRacing's old way of specifying Shift Light RPM. It was specified in Session Data
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