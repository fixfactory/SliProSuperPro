#pragma once

#include <string>

#include "Plugin.h"
#include "PluginInterface.h"

// The blackboard is for global variables to be exchanged between systems
// without needing to create a direct dependency.
namespace blackboard
{
    // Constant car physics properties for the last car driven.
    extern const plugin::PhysicsData *physicsData;
} // namespace blackboard
