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

#include <string>

#include "PluginInterface.h"

namespace ngp
{
    constexpr unsigned int kFolderNameCount = 8;
    extern const char *kFolderNames[kFolderNameCount];
    constexpr unsigned int kGearCount = 8;

    struct Drive
    {
        int numberOfGears;
    };

    struct ControlUnit
    {
        float gearUpShift[kGearCount];
        float gearDownShift[kGearCount];
        float rpmLimit;
    };

    struct CarPhysics
    {
        Drive drive;
        ControlUnit controlUnit;
    };
} // namespace ngp

class NgpManager
{
public:
    static NgpManager &getSingleton();

    NgpManager();
    ~NgpManager();

    void init();
    void deinit();

    bool fetchPhysicsData(const std::string &gamePath);
    const plugin::PhysicsData &getPhysicsData() const;

private:
    bool readCommon(unsigned int carIndex, const std::string &gamePath);

    bool m_wasReceivingTelemetry = false;
    ngp::CarPhysics m_carPhysics = {};
    plugin::PhysicsData m_physicsData{};
};
