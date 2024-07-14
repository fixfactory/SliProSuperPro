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
typedef bool(__stdcall *GetTelemetryData)(plugin::TelemetryData *, size_t);
typedef bool(__stdcall *GetPhysicsData)(plugin::PhysicsData *, size_t);
typedef bool(__stdcall *GetPhysicsDataEveryFrame)();

struct Plugin
{
    HINSTANCE library{ nullptr };
    std::string libraryPath{};
    int interfaceVersion{ 0 };
    std::string gameExecFileName{};
    SetGameIsRunning setGameIsRunning{ nullptr };
    GetTelemetryData getTelemetryData{ nullptr };
    GetPhysicsData getPhysicsData{ nullptr };
    GetPhysicsDataEveryFrame getPhysicsDataEveryFrame{ nullptr };
};

class PluginManager : public Updateable
{
public:
    static PluginManager &getSingleton();

    PluginManager();
    ~PluginManager();

    void init();
    void deinit();

    void update(timing::seconds deltaTimeSecs) override;

    using PluginList = std::vector<Plugin *>;
    const PluginList &getPluginList();

    void setActivePlugin(const Plugin *plugin);
    const Plugin *getActivePlugin() const;

private:
    void loadPlugins();
    void unloadPlugins();

    PluginList m_plugins;
    const Plugin *m_activePlugin{ nullptr };
};
