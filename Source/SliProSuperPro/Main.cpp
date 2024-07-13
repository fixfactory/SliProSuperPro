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
#include <atomic>

#include "Libraries.h"
#include "Version.h"
#include "Log.h"
#include "CommandLine.h"
#include "Timing.h"
#include "Config.h"
#include "Process.h"
#include "Device.h"
#include "Plugin.h"
#include "Telemetry.h"
#include "Physics.h"

std::atomic<bool> g_programShouldExit = false;
constexpr DWORD kProgramCloseTimeoutMs = 2000;

BOOL consoleCtrlHandler(DWORD ctrlType)
{
    switch (ctrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT: {
        // Sleep for a few seconds to let the application finish on the main thread.
        // After this returns the program will be terminated if it hasn't already.
        g_programShouldExit = true;
        Sleep(kProgramCloseTimeoutMs);
        return (TRUE);
    }

    default:
        return FALSE;
    }
}

int main(int argc, char *argv[])
{
    LogManager::getSingleton().init();
    LOG_INFO("%s version %s", version::kName.c_str(), version::getString().c_str());

    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)consoleCtrlHandler, TRUE))
    {
        LOG_WARN("SetConsoleCtrlHandler() failed.");
    }

    if (!cmdLine::parseOptions(argc, argv))
    {
        return EXIT_FAILURE;
    }

    TimingManager::getSingleton().init();
    PluginManager::getSingleton().init();
    ProcessManager::getSingleton().init();
    TelemetryManager::getSingleton().init();
    PhysicsManager::getSingleton().init();
    DeviceManager::getSingleton().init();

    while (!g_programShouldExit.load())
    {
        TimingManager::getSingleton().run();
    }

    LOG_INFO("Program terminating...");

    DeviceManager::getSingleton().deinit();
    PhysicsManager::getSingleton().deinit();
    TelemetryManager::getSingleton().deinit();
    ProcessManager::getSingleton().deinit();
    PluginManager::getSingleton().deinit();
    TimingManager::getSingleton().deinit();
    LogManager::getSingleton().deinit();

    return EXIT_SUCCESS;
}
