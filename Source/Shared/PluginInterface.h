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

namespace plugin
{
    constexpr int kInterfaceVersion = 1;

    struct TelemetryData
    {
        int gear;
        float rpm;
        float speedKph;
        bool speedLimiter;
    };

    constexpr int kMaxGearCount = 10;

    struct PhysicsData
    {
        int gearCount;
        float rpmLimit;
        float rpmIdle;
        float rpmDownshift[kMaxGearCount];
        float rpmUpshift[kMaxGearCount];
    };
} // namespace plugin