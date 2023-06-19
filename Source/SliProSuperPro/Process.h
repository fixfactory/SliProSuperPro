#pragma once

#include <Windows.h>
#include <string>
#include <chrono>

#include "Timing.h"

class ProcessManager : public Updateable
{
public:
    static ProcessManager &getSingleton();

    ProcessManager();
    ~ProcessManager();

    void init();
    void deinit();

    void update(timing::seconds deltaTimeSecs) override;

    const std::string &getGamePath() const;

private:
    DWORD findProcessId(const std::string &name) const;
    std::string findProcessPath(DWORD pid) const;

    using time_point = std::chrono::steady_clock::time_point;
    time_point m_lastCheckTime = {};

    std::string m_gamePath{};
};
