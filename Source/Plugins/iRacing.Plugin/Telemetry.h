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
    json m_overrides;
    std::string m_carPath;

    bool m_hasOverride{ false };
    float m_rpmDownshiftOverride[plugin::kMaxGearCount];
    float m_rpmUpshiftOverride[plugin::kMaxGearCount];

    void readOverrides();
    void parseOverrides();
};
