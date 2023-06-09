#pragma once

#include <windows.h>
#include <string>
#include <chrono>

#include "Timing.h"

class ProcessManager : public Updateable
{
public:
    static ProcessManager& getSingleton();

    ProcessManager();
    ~ProcessManager();

    void init();
    void deinit();

    void update(timing::seconds deltaTime) override;

    bool isGameRunning() const { return !m_gamePath.empty(); }
    const std::string& getGamePath() const { return m_gamePath;  }

private:
    DWORD findProcessId(const std::string& name) const;
    std::string findProcessPath(DWORD pid) const;

    std::string m_gamePath;

    using time_point = std::chrono::steady_clock::time_point;
    time_point m_lastCheckTime = {};
};