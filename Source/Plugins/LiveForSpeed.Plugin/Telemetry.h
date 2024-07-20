//
// SliProSuperPro
// A Shift Light Indicator controller
// Copyright 2024 Fixfactory
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

#include "PluginInterface.h"

#include "json/json.hpp"
using json = nlohmann::json;

class TelemetryManager
{
public:
    static TelemetryManager &getSingleton();

    TelemetryManager();
    ~TelemetryManager();

    void init();
    void deinit();

    bool fetchTelemetryData();
    const plugin::TelemetryData &getTelemetryData() const;
    const plugin::PhysicsData &getPhysicsData() const;

private:
    bool m_receivingTelemetry{ false };
    plugin::TelemetryData m_telemetryData{};
    plugin::PhysicsData m_physicsData{};
    json m_carData;
    std::string m_carPath;

    std::string m_inSimHostname;
    int m_inSimPort{ 0 };
    std::string m_inSimPassword;

    void readConfig();
    void readCarData();
    void parseCarData();
};
