#include <windows.h>
#include <psapi.h> // For access to GetModuleFileNameEx
#include <tlhelp32.h>
#include <locale>
#include <codecvt>

#include "Process.h"
#include "Log.h"
#include "Config.h"
#include "Blackboard.h"
#include "String.h"

const std::string kGameExecName { "RichardBurnsRally_SSE.exe" };
const std::chrono::duration<float> kCheckInterval { 2.f };

ProcessManager& ProcessManager::getSingleton()
{
    static ProcessManager s_singleton;
    return s_singleton;
}

ProcessManager::ProcessManager()
{

}

ProcessManager::~ProcessManager()
{

}

void ProcessManager::init()
{
    TimingManager::getSingleton().registerUpdateable(this);
}

void ProcessManager::deinit()
{
    blackboard::gamePath.clear();
    TimingManager::getSingleton().unregisterUpdateable(this);
}

void ProcessManager::update(timing::seconds deltaTime)
{
    // Every few seconds, check if the game is still running.
    auto time = std::chrono::steady_clock::now();
    if (time - m_lastCheckTime < kCheckInterval)
    {
        return;
    }

    m_lastCheckTime = time;
    DWORD pid = findProcessId(kGameExecName);
    if (pid == 0)
    {
        if (!m_gamePath.empty())
        {
            LOG_INFO("Game closed");
            m_gamePath.clear();
            blackboard::gamePath.clear();
        }        
        return;
    }

    if (!m_gamePath.empty())
    {
        return;
    }

    LOG_INFO("Game running (pid %ul)", pid);
    m_gamePath = findProcessPath(pid);
    size_t pos = m_gamePath.rfind(kGameExecName);
    if (pos == std::string::npos)
    {
        LOG_ERROR("Failed to parse path. Exec name not found. %s", m_gamePath.c_str());
        return;
    }

    m_gamePath.erase(pos, pos + kGameExecName.length());
    m_gamePath.erase(std::find(m_gamePath.begin(), m_gamePath.end(), '\0'), m_gamePath.end());
    blackboard::gamePath = m_gamePath;
    LOG_INFO("Game path: %s", m_gamePath.c_str());
}

DWORD ProcessManager::findProcessId(const std::string& name) const
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32 info;
    info.dwSize = sizeof(info);
    Process32First(snapshot, &info);
    try
    {
        std::string exeFileName = string::convertFromWide(info.szExeFile);
        if (!name.compare(exeFileName))
        {
            CloseHandle(snapshot);
            return info.th32ProcessID;
        }

        while (Process32Next(snapshot, &info))
        {
            exeFileName = string::convertFromWide(info.szExeFile);
            if (!name.compare(exeFileName))
            {
                CloseHandle(snapshot);
                return info.th32ProcessID;
            }
        }
    }
    catch (const std::exception& exception)
    {
        LOG_ERROR(exception);
    }

    CloseHandle(snapshot);
    return 0;
}

std::string ProcessManager::findProcessPath(DWORD pid) const
{
    HANDLE processHandle = NULL;
    processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (processHandle == NULL)
    {
        return std::string{ };
    }

    std::wstring fileName(MAX_PATH, 0);
    GetModuleFileNameEx(processHandle, NULL, fileName.data(), (DWORD)fileName.size());
    CloseHandle(processHandle);

    try
    {
        return string::convertFromWide(fileName);
    }
    catch (const std::exception &exception)
    {
        LOG_ERROR(exception);
    }

    return std::string{ };
}
