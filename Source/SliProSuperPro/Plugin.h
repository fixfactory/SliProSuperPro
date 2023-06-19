#pragma once

#include <Windows.h>
#include <string>
#include <vector>

#include "Timing.h"
#include "PluginInterface.h"

typedef int(__stdcall *GetPluginInterfaceVersion)();
typedef bool(__stdcall *SupportsInterfaceVersion)(int);
typedef void(__stdcall *GetGameExecFileName)(std::string &);
typedef void(__stdcall *SetGameIsRunning)(bool, std::string);
typedef bool(__stdcall *FetchTelemetryData)();
typedef bool(__stdcall *FetchPhysicsData)();
typedef bool(__stdcall *GetTelemetryData)(plugin::TelemetryData *, size_t);
typedef bool(__stdcall *GetPhysicsData)(plugin::PhysicsData *, size_t);

struct Plugin
{
    HINSTANCE library{ nullptr };
    std::string libraryPath{};
    int interfaceVersion{ 0 };
    std::string gameExecFileName{};
    SetGameIsRunning setGameIsRunning{ nullptr };
    FetchTelemetryData fetchTelemetryData{ nullptr };
    FetchPhysicsData fetchPhysicsData{ nullptr };
    GetTelemetryData getTelemetryData{ nullptr };
    GetPhysicsData getPhysicsData{ nullptr };
};

class PluginManager : public Updateable
{
public:
    static PluginManager &getSingleton();

    PluginManager();
    ~PluginManager();

    void init();
    void deinit();

    void update(timing::seconds deltaTime) override;

    using PluginList = std::vector<Plugin *>;
    const PluginList &getPluginList();

private:
    void loadPlugins();
    void unloadPlugins();

    PluginList m_plugins;
};
