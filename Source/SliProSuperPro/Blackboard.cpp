#include "Blackboard.h"

namespace blackboard
{
    const Plugin *activePlugin{ nullptr };
    std::string gamePath{ "" };
    const plugin::TelemetryData *telemetryData{ nullptr };
    const plugin::PhysicsData *physicsData{ nullptr };
} // namespace blackboard
