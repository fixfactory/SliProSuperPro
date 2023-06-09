#include <iostream>
#include <chrono>
#include <Windows.h>

#include "Libraries.h"
#include "Version.h"
#include "Log.h"
#include "CommandLine.h"
#include "Application.h"
#include "Timing.h"
#include "Config.h"

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

    Application app;
    app.init();

    timing::seconds deltaTime = std::chrono::duration_cast<timing::seconds>(timing::kMinFrameTime);

    while (!g_programShouldExit.load())
    {
        auto before = std::chrono::steady_clock::now();
        app.update(deltaTime);

        timing::seconds appTime = std::chrono::steady_clock::now() - before;
        timing::seconds waitTime{ 0.f };

        deltaTime = appTime;
        if (deltaTime < timing::kMinFrameTime)
        {
            waitTime = timing::kMinFrameTime - deltaTime - timing::kMinWaitTime;
            if (waitTime >= timing::kMinWaitTime)
            {
                timing::preciseSleep(waitTime);
            }
            else
            {
                waitTime = timing::seconds{ 0.f };
            }
            deltaTime = std::chrono::steady_clock::now() - before;
        }

        if (deltaTime > timing::kMaxFrameTime)
        {
            deltaTime = timing::kMaxFrameTime;
        }

        timing::elapsedTime += deltaTime;
        timing::frameTime = deltaTime;
        timing::frameNumber++;

        if (config::debugTiming)
        {
            LOG_INFO("Frame %i, appTime %.3f, waitTime %.3f, frameTime %.3f, elapsedTime %.3f\n",
                timing::frameNumber,
                appTime.count(),
                waitTime.count(),
                timing::frameTime.count(),
                timing::elapsedTime.count());
        }
    }
    
    LOG_INFO("Program terminating...");
    app.deinit();
    LogManager::getSingleton().deinit();
    return 0;
}
