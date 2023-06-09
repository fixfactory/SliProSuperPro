#pragma once

#include "Telemetry.h"
#include "NGP.h"
#include "Timing.h"

class TelemetryManager;
class ProcessManager;
class NgpManager;
class DeviceManager;

class Application
{
public:
    Application();
    ~Application();

    void init();
    void deinit();

    void update(timing::seconds deltaTime);

private:
    TelemetryManager *m_telemetryManager = nullptr;
    ProcessManager *m_processManager = nullptr;
    NgpManager* m_ngpManager = nullptr;
    DeviceManager* m_deviceManager = nullptr;
};