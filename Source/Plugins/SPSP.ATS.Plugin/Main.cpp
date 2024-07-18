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

// Windows stuff.
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

// SDK
#include "scs_sdk/include/scssdk_telemetry.h"
#include "scs_sdk/include/eurotrucks2/scssdk_eut2.h"
#include "scs_sdk/include/eurotrucks2/scssdk_telemetry_eut2.h"
#include "scs_sdk/include/amtrucks/scssdk_ats.h"
#include "scs_sdk/include/amtrucks/scssdk_telemetry_ats.h"

#define UNUSED(x)

const char *kSharedMemoryName = "SPSP.ATS.Plugin";

/**
 * @name Callbacks remembered from the initialization info.
 */
//@{
scs_log_t g_game_log = NULL;
//@}

/**
 * @brief Prints message to game log.
 */
void log_line(const scs_log_type_t type, const char *const text, ...)
{
    if (!g_game_log)
    {
        return;
    }

    char formated[1000];
    va_list args;
    va_start(args, text);
    vsnprintf_s(formated, sizeof(formated), _TRUNCATE, text, args);
    formated[sizeof(formated) - 1] = 0;
    va_end(args);

    g_game_log(type, formated);
}

#pragma pack(push)
#pragma pack(1)

/**
 * @brief The layout of the shared memory.
 */
struct telemetry_state_t
{
    scs_u8_t running; // Is the telemetry running or it is paused?

    scs_float_t speedometer_speed; // SCS_TELEMETRY_TRUCK_CHANNEL_speed
    scs_float_t rpm;               // SCS_TELEMETRY_TRUCK_CHANNEL_engine_rpm
    scs_float_t rpmLimit;
    scs_s32_t gear;                // SCS_TELEMETRY_TRUCK_CHANNEL_engine_gear
    scs_s32_t gearForwardCount;
    scs_s32_t gearReverseCount;
};

#pragma pack(pop)

/**
 * @brief Handle of the memory mapping.
 */
HANDLE g_memory_mapping = NULL;

/**
 * @brief Block inside the shared memory.
 */
telemetry_state_t *g_shared_memory = NULL;

/**
 * @brief Deinitialize the shared memory objects.
 */
void deinitialize_shared_memory(void)
{
    if (g_shared_memory)
    {
        UnmapViewOfFile(g_shared_memory);
        g_shared_memory = NULL;
    }

    if (g_memory_mapping)
    {
        CloseHandle(g_memory_mapping);
        g_memory_mapping = NULL;
    }
}

/**
 * @brief Initialize the shared memory objects.
 */
bool initialize_shared_memory(void)
{
    // Setup the mapping.
    const DWORD memory_size = sizeof(telemetry_state_t);
    log_line(SCS_LOG_TYPE_message, "Creating memory file %s size=%i", kSharedMemoryName, memory_size);
    g_memory_mapping = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, memory_size,
                                        kSharedMemoryName);

    if (!g_memory_mapping)
    {
        log_line(SCS_LOG_TYPE_error, "Unable to create shared memory %08X", GetLastError());
        deinitialize_shared_memory();
        return false;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        log_line(SCS_LOG_TYPE_error, "Shared memory is already in use.");
        deinitialize_shared_memory();
        return false;
    }

    g_shared_memory = static_cast<telemetry_state_t *>(MapViewOfFile(g_memory_mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0));
    if (!g_shared_memory)
    {
        log_line(SCS_LOG_TYPE_error, "Unable to map the view %08X", GetLastError());
        deinitialize_shared_memory();
        return false;
    }

    // Defaults in the structure.
    memset(g_shared_memory, 0, memory_size);

    // We are always initialized in the paused state.
    g_shared_memory->running = 0;

    return true;
}

/**
 * @brief Float storage callback.
 *
 * Can be used together with SCS_TELEMETRY_CHANNEL_FLAG_no_value in which case it
 * will store zero if the value is not available.
 */
SCSAPI_VOID telemetry_store_float(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value,
                                  const scs_context_t context)
{
    assert(context);
    scs_float_t *const storage = static_cast<scs_float_t *>(context);

    if (value)
    {
        assert(value->type == SCS_VALUE_TYPE_float);
        *storage = value->value_float.value;
    }
    else
    {
        *storage = 0.0f;
    }
}

/**
 * @brief s32 storage callback.
 *
 * Can be used together with SCS_TELEMETRY_CHANNEL_FLAG_no_value in which case it
 * will store zero if the value is not available.
 */
SCSAPI_VOID telemetry_store_s32(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value,
                                const scs_context_t context)
{
    assert(context);
    scs_s32_t *const storage = static_cast<scs_s32_t *>(context);

    if (value)
    {
        assert(value->type == SCS_VALUE_TYPE_s32);
        *storage = value->value_s32.value;
    }
    else
    {
        *storage = 0;
    }
}

/**
 * @brief Orientation storage callback.
 *
 * Can be used together with SCS_TELEMETRY_CHANNEL_FLAG_no_value in which case it
 * will store zero if the value is not available.
 */
SCSAPI_VOID telemetry_store_orientation(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value,
                                        const scs_context_t context)
{
    assert(context);
    scs_value_euler_t *const storage = static_cast<scs_value_euler_t *>(context);

    if (value)
    {
        assert(value->type == SCS_VALUE_TYPE_euler);
        *storage = value->value_euler;
    }
    else
    {
        storage->heading = 0.0f;
        storage->pitch = 0.0f;
        storage->roll = 0.0f;
    }
}

/**
 * @brief Vector storage callback.
 *
 * Can be used together with SCS_TELEMETRY_CHANNEL_FLAG_no_value in which case it
 * will store zero if the value is not available.
 */
SCSAPI_VOID telemetry_store_fvector(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value,
                                    const scs_context_t context)
{
    assert(context);
    scs_value_fvector_t *const storage = static_cast<scs_value_fvector_t *>(context);

    if (value)
    {
        assert(value->type == SCS_VALUE_TYPE_fvector);
        *storage = value->value_fvector;
    }
    else
    {
        storage->x = 0.0f;
        storage->y = 0.0f;
        storage->z = 0.0f;
    }
}

/**
 * @brief Placement storage callback.
 *
 * Can be used together with SCS_TELEMETRY_CHANNEL_FLAG_no_value in which case it
 * will store zeros if the value is not available.
 */
SCSAPI_VOID telemetry_store_dplacement(const scs_string_t name, const scs_u32_t index, const scs_value_t *const value,
                                       const scs_context_t context)
{
    assert(context);
    scs_value_dplacement_t *const storage = static_cast<scs_value_dplacement_t *>(context);

    if (value)
    {
        assert(value->type == SCS_VALUE_TYPE_dplacement);
        *storage = value->value_dplacement;
    }
    else
    {
        storage->position.x = 0.0;
        storage->position.y = 0.0;
        storage->position.z = 0.0;
        storage->orientation.heading = 0.0f;
        storage->orientation.pitch = 0.0f;
        storage->orientation.roll = 0.0f;
    }
}

/**
 * @brief Finds attribute with specified name in the configuration structure.
 *
 * Returns NULL if the attribute was not found or if it is not of the expected type.
 */
const scs_named_value_t *find_attribute(const scs_telemetry_configuration_t &configuration, const char *const name,
                                        const scs_u32_t index, const scs_value_type_t expected_type)
{
    for (const scs_named_value_t *current = configuration.attributes; current->name; ++current)
    {
        if ((current->index != index) || (strcmp(current->name, name) != 0))
        {
            continue;
        }

        if (current->value.type == expected_type)
        {
            return current;
        }

        log_line(SCS_LOG_TYPE_error, "Attribute %s has unexpected type %u", name,
                 static_cast<unsigned>(current->value.type));
        break;
    }
    return NULL;
}

/**
 * @brief Called whenever the game pauses or unpauses its telemetry output.
 */
SCSAPI_VOID telemetry_pause(const scs_event_t event, const void *const UNUSED(event_info),
                            const scs_context_t UNUSED(context))
{
    g_shared_memory->running = (event == SCS_TELEMETRY_EVENT_started) ? 1 : 0;
    log_line(SCS_LOG_TYPE_message, "Telemetry Running %i", g_shared_memory->running);
}

/**
 * @brief Called whenever configuration changes.
 */
SCSAPI_VOID telemetry_configuration(const scs_event_t event, const void *const event_info,
                                    const scs_context_t UNUSED(context))
{
    // We currently only care for the truck telemetry info.
    const struct scs_telemetry_configuration_t *const info =
        static_cast<const scs_telemetry_configuration_t *>(event_info);

    if (strcmp(info->id, SCS_TELEMETRY_CONFIG_truck) != 0)
    {
        return;
    }

    const scs_named_value_t *const rpm_limit_attr =
        find_attribute(*info, SCS_TELEMETRY_CONFIG_ATTRIBUTE_rpm_limit, SCS_U32_NIL, SCS_VALUE_TYPE_float);

    const scs_named_value_t *const fwd_gear_count_attr =
        find_attribute(*info, SCS_TELEMETRY_CONFIG_ATTRIBUTE_forward_gear_count, SCS_U32_NIL, SCS_VALUE_TYPE_u32);

    const scs_named_value_t *const reverse_gear_count_attr =
        find_attribute(*info, SCS_TELEMETRY_CONFIG_ATTRIBUTE_reverse_gear_count, SCS_U32_NIL, SCS_VALUE_TYPE_u32);

    g_shared_memory->rpmLimit = rpm_limit_attr ? rpm_limit_attr->value.value_float.value : 0.f;
    g_shared_memory->gearForwardCount = fwd_gear_count_attr ? fwd_gear_count_attr->value.value_u32.value : 0;
    g_shared_memory->gearReverseCount = reverse_gear_count_attr ? reverse_gear_count_attr->value.value_u32.value : 0;

    log_line(SCS_LOG_TYPE_message, "Updated configuration rpmLimit=%f gearForward=%i gearReverse=%i", g_shared_memory->rpmLimit,
             g_shared_memory->gearForwardCount, g_shared_memory->gearReverseCount);
}

/**
 * @brief Telemetry API initialization function.
 *
 * See scssdk_telemetry.h
 */
extern "C"
{
    scs_result_t __declspec(dllexport) __stdcall scs_telemetry_init(const scs_u32_t version,
                                                                    const scs_telemetry_init_params_t *const params)
    {
        // We currently support only one version of the API.
        if (version != SCS_TELEMETRY_VERSION_1_00)
        {
            return SCS_RESULT_unsupported;
        }

        const scs_telemetry_init_params_v100_t *const version_params =
            static_cast<const scs_telemetry_init_params_v100_t *>(params);

        g_game_log = version_params->common.log;
        log_line(SCS_LOG_TYPE_message, "Initializing SPSP.ATS.Plugin");

        // Check application version.
        log_line(SCS_LOG_TYPE_message, "Game '%s' %u.%u", version_params->common.game_id,
                 SCS_GET_MAJOR_VERSION(version_params->common.game_version),
                 SCS_GET_MINOR_VERSION(version_params->common.game_version));

        if (strcmp(version_params->common.game_id, SCS_GAME_ID_EUT2) == 0)
        {
            // Below the minimum version there might be some missing features (only minor change) or
            // incompatible values (major change).
            if (version_params->common.game_version < SCS_TELEMETRY_EUT2_GAME_VERSION_1_03)
            { // Fixed the wheels.count attribute
                log_line(SCS_LOG_TYPE_error, "Too old version of the game");
                g_game_log = NULL;
                return SCS_RESULT_unsupported;
            }

            if (version_params->common.game_version < SCS_TELEMETRY_EUT2_GAME_VERSION_1_07)
            { // Fixed the angular acceleration calculation
                log_line(SCS_LOG_TYPE_warning,
                         "This version of the game has less precise output of angular acceleration of the cabin");
            }

            // Future versions are fine as long the major version is not changed.
            const scs_u32_t IMPLEMENTED_VERSION = SCS_TELEMETRY_EUT2_GAME_VERSION_CURRENT;
            if (SCS_GET_MAJOR_VERSION(version_params->common.game_version) > SCS_GET_MAJOR_VERSION(IMPLEMENTED_VERSION))
            {
                log_line(SCS_LOG_TYPE_warning,
                         "Too new major version of the game, some features might behave incorrectly");
            }
        }
        else if (strcmp(version_params->common.game_id, SCS_GAME_ID_ATS) == 0)
        {
            // Below the minimum version there might be some missing features (only minor change) or
            // incompatible values (major change).
            const scs_u32_t MINIMAL_VERSION = SCS_TELEMETRY_ATS_GAME_VERSION_1_00;
            if (version_params->common.game_version < MINIMAL_VERSION)
            {
                log_line(SCS_LOG_TYPE_warning,
                         "WARNING: Too old version of the game, some features might behave incorrectly");
            }

            // Future versions are fine as long the major version is not changed.
            const scs_u32_t IMPLEMENTED_VERSION = SCS_TELEMETRY_ATS_GAME_VERSION_CURRENT;
            if (SCS_GET_MAJOR_VERSION(version_params->common.game_version) > SCS_GET_MAJOR_VERSION(IMPLEMENTED_VERSION))
            {
                log_line(SCS_LOG_TYPE_warning,
                         "WARNING: Too new major version of the game, some features might behave incorrectly");
            }
        }
        else
        {
            log_line(SCS_LOG_TYPE_warning, "Unsupported game, some features or values might behave incorrectly");
        }

        // Register for events. Note that failure to register those basic events
        // likely indicates invalid usage of the api or some critical problem.
        const bool events_registered =
            (version_params->register_for_event(SCS_TELEMETRY_EVENT_paused, telemetry_pause, NULL) == SCS_RESULT_ok) &&
            (version_params->register_for_event(SCS_TELEMETRY_EVENT_started, telemetry_pause, NULL) == SCS_RESULT_ok) &&
            (version_params->register_for_event(SCS_TELEMETRY_EVENT_configuration, telemetry_configuration, NULL) ==
             SCS_RESULT_ok);

        if (!events_registered)
        {
            // Registrations created by unsuccessful initialization are
            // cleared automatically so we can simply exit.
            log_line(SCS_LOG_TYPE_error, "Unable to register event callbacks");
            g_game_log = NULL;
            return SCS_RESULT_generic_error;
        }

        // Initialize the shared memory.
        if (!initialize_shared_memory())
        {
            log_line(SCS_LOG_TYPE_error, "Unable to initialize shared memory");
            g_game_log = NULL;
            return SCS_RESULT_generic_error;
        }

        // Register all changes we are interested in. Note that some wheel-related channels will be initialized when we
        // receive a configuration event. The channel might be missing if the game does not support it
        // (SCS_RESULT_not_found) or if does not support the requested type (SCS_RESULT_unsupported_type).
        version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_speed, SCS_U32_NIL, SCS_VALUE_TYPE_float,
                                             SCS_TELEMETRY_CHANNEL_FLAG_no_value, telemetry_store_float,
                                             &g_shared_memory->speedometer_speed);

        version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_engine_rpm, SCS_U32_NIL, SCS_VALUE_TYPE_float,
                                             SCS_TELEMETRY_CHANNEL_FLAG_no_value, telemetry_store_float,
                                             &g_shared_memory->rpm);

        version_params->register_for_channel(SCS_TELEMETRY_TRUCK_CHANNEL_engine_gear, SCS_U32_NIL, SCS_VALUE_TYPE_s32,
                                             SCS_TELEMETRY_CHANNEL_FLAG_no_value, telemetry_store_s32,
                                             &g_shared_memory->gear);

        // We are done.
        log_line(SCS_LOG_TYPE_message, "SPSP.ATS.Plugin initialized");
        return SCS_RESULT_ok;
    }
}

/**
 * @brief Telemetry API deinitialization function.
 *
 * See scssdk_telemetry.h
 */
extern "C"
{
    void __declspec(dllexport) __stdcall scs_telemetry_shutdown(void)
    {
        // Any cleanup needed. The registrations will be removed automatically
        // so there is no need to do that manually.
        deinitialize_shared_memory();

        g_game_log = NULL;
    }
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason_for_call, LPVOID reseved)
{
    return TRUE;
}
