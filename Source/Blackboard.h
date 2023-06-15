#pragma once

#include <string>

#include "PhysicsNG/rbr.telemetry.data.TelemetryData.h"
#include "NGP.h"

using TelemetryData = rbr::telemetry::data::TelemetryData;

// The blackboard is for global variables to be exchanged between systems
// without needing to create a direct dependency.
namespace blackboard
{
    // Path to the game folder. Empty when the game is not running.
    extern std::string gamePath;

    // Live telemetry data received. `nullptr` when not driving.
    extern const TelemetryData *telemetryData;

    // Constant car physics properties for the last car driven.
    extern const ngp::CarPhysics *carPhysics;
} // namespace blackboard