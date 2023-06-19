#pragma once

namespace plugin
{
    constexpr int kInterfaceVersion = 1;

    struct TelemetryData
    {
        int gear;
        float rpm;
        float speedKph;
    };

    constexpr int kMaxGearCount = 10;

    struct PhysicsData
    {
        int gearCount;
        float rpmLimit;
        float rpmDownshift[kMaxGearCount];
        float rpmUpshift[kMaxGearCount];
    };
} // namespace plugin