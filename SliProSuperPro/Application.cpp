#include "Application.h"
#include "Device.h"
#include "NGP.h"
#include "Process.h"
#include "Device.h"
#include "Timing.h"

Application::Application()
{
}

Application::~Application()
{

}

void Application::init()
{
    m_telemetryManager = new TelemetryManager();
    m_telemetryManager->init();

    m_processManager = new ProcessManager();
    m_processManager->init();

    m_ngpManager = new NgpManager();
    m_ngpManager->init();

    m_deviceManager = new DeviceManager();
    m_deviceManager->init();
}

void Application::deinit()
{
    m_deviceManager->deinit();
    delete m_deviceManager;
    m_deviceManager = nullptr;

    m_ngpManager->deinit();
    delete m_ngpManager;
    m_ngpManager = nullptr;

    m_processManager->deinit();
    delete m_processManager;
    m_processManager = nullptr;

    m_telemetryManager->deinit();
    delete m_telemetryManager;
    m_telemetryManager = nullptr;
}

void Application::update(timing::seconds deltaTime)
{
    m_processManager->update(deltaTime);
    m_telemetryManager->update(deltaTime);
    m_ngpManager->update(deltaTime);
    m_deviceManager->update(deltaTime);
}
