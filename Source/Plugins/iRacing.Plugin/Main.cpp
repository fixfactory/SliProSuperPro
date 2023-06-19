#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>

#include <string>
#include <iostream>
#include <atomic>

#include "Log.h"
#include "PluginInterface.h"
#include "Defines.h"

std::atomic<int> g_attachCount = 0;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
        // Initialize library the first time it is attached.
        // operator++ on atomic is thread-safe.
        if (g_attachCount++ == 0)
        {
            LogManager::getSingleton().init();
            LOG_INFO("Loaded iRacing Plugin version 0.1.0");
        }
        break;

    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        // Deinitialize library the last time it detached.
        // operator-- on atomic is thread-safe.
        if (--g_attachCount == 0)
        {
            LOG_INFO("iRacing Plugin unloaded");
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
    }

    bool DLL_EXPORT fetchTelemetryData()
    {
        return false;
    }

    bool DLL_EXPORT fetchPhysicsData()
    {
        return false;
    }

    bool DLL_EXPORT getTelemetryData(plugin::TelemetryData *outTelemetryData, size_t telemetryDataSize)
    {
        return false;
    }

    bool DLL_EXPORT getPhysicsData(plugin::PhysicsData *outPhysicsData, size_t physicsDataSize)
    {
        return false;
    }
}
