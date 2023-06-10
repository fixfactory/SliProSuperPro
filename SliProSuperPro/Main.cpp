#include <Windows.h>

#include "Libraries.h"
#include "Version.h"
#include "Log.h"
#include "CommandLine.h"
#include "Timing.h"
#include "Config.h"
#include "Process.h"
#include "Device.h"
#include "NGP.h"
#include "Telemetry.h"

std::atomic<bool> g_programShouldExit = false;
constexpr DWORD kProgramCloseTimeoutMs = 2000;

BOOL consoleCtrlHandler(DWORD ctrlType)
{
    switch (ctrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    {
        // Sleep for a few seconds to let the application finish on the main thread.
        // After this returns the program will be terminated if it hasn't already.
        g_programShouldExit = true;
        Sleep(kProgramCloseTimeoutMs);
        return(TRUE);
    }

    default:
        return FALSE;
    }
}

int main(int argc, char* argv[])
{
    LogManager::getSingleton().init();
    LOG_INFO("%s version %s", version::kName.c_str(), version::getString().c_str());

    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)consoleCtrlHandler, TRUE))
    {
        LOG_WARN("SetConsoleCtrlHandler() failed.");
    }

    if (!cmdLine::parseOptions(argc, argv))
    {
        return -1;
    }

    TimingManager::getSingleton().init();
    TelemetryManager::getSingleton().init();
    ProcessManager::getSingleton().init();
    NgpManager::getSingleton().init();
    DeviceManager::getSingleton().init();

    while (!g_programShouldExit.load())
    {
        TimingManager::getSingleton().run();
    }
    
    LOG_INFO("Program terminating...");

    DeviceManager::getSingleton().deinit();
    NgpManager::getSingleton().deinit();
    ProcessManager::getSingleton().deinit();
    TelemetryManager::getSingleton().deinit();
    TimingManager::getSingleton().deinit();
    LogManager::getSingleton().deinit();

    return 0;
}
