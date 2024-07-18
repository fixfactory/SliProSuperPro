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

#include <string>
#include <iostream>
#include <atomic>

#include "Log.h"
#include "PluginInterface.h"
#include "Defines.h"
#include "Telemetry.h"

std::atomic<int> g_dllAttachCount = 0;
std::string g_gameExecPath{};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
        // Initialize library the first time it is attached.
        // operator++ on atomic is thread-safe.
        if (g_dllAttachCount++ == 0)
        {
            LogManager::getSingleton().init();
            LOG_INFO("Loaded ATS Plugin version 0.1.0");
        }
        break;

    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        // Deinitialize library the last time it detached.
        // operator-- on atomic is thread-safe.
        if (--g_dllAttachCount == 0)
        {
            LOG_INFO("ATS Plugin unloaded");
            LogManager::getSingleton().deinit();
        }
        break;
    }

    return TRUE;
}

extern "C"
{
    int DLL_EXPORT getPluginInterfaceVersion()
    {
        return plugin::kInterfaceVersion;
    }

    bool DLL_EXPORT supportsInterfaceVersion(int interfaceVersion)
    {
        return interfaceVersion >= 1 && interfaceVersion <= 1;
    }

    void DLL_EXPORT getGameExecFileName(std::string &outExecFileName)
    {
        outExecFileName = "iRacingSim64DX11.exe";
    }

    void DLL_EXPORT setGameIsRunning(bool isRunning, std::string execPath)
    {
        if (isRunning)
        {
            g_gameExecPath = execPath;
            TelemetryManager::getSingleton().init();
        }
        else
        {
            TelemetryManager::getSingleton().deinit();
            g_gameExecPath.clear();
        }
    }

    bool DLL_EXPORT getTelemetryData(plugin::TelemetryData *outTelemetryData, size_t telemetryDataSize)
    {
        if (TelemetryManager::getSingleton().fetchTelemetryData())
        {
            auto &telemetryData = TelemetryManager::getSingleton().getTelemetryData();
            if (sizeof(telemetryData) >= telemetryDataSize)
            {
                memcpy(outTelemetryData, &telemetryData, telemetryDataSize);
                return true;
            }
        }
        return false;
    }

    bool DLL_EXPORT getPhysicsData(plugin::PhysicsData *outPhysicsData, size_t physicsDataSize)
    {
        if (TelemetryManager::getSingleton().fetchTelemetryData())
        {
            auto &physicsData = TelemetryManager::getSingleton().getPhysicsData();
            if (sizeof(physicsData) >= physicsDataSize)
            {
                memcpy(outPhysicsData, &physicsData, physicsDataSize);
                return true;
            }
        }
        return false;
    }

    bool DLL_EXPORT getPhysicsDataEveryFrame()
    {
        // In iRacing, some cars will have different Shift Light RPM per gear and we get those in live telemetry
        // rather than in Session Data. So the app should keep polling for new Physics Data every frame.
        return true;
    }
}
