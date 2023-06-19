#pragma once

#include "Timing.h"
#include "PluginInterface.h"

class TelemetryManager : public Updateable
{
public:
    static TelemetryManager &getSingleton();

    TelemetryManager();
    ~TelemetryManager();

    void init();
    void deinit();

    void update(timing::seconds deltaTimeSecs) override;

    bool isReceivingTelemetry() const;
    const plugin::TelemetryData &getTelemetryData() const;

private:
    plugin::TelemetryData m_telemetryData{};
    bool m_receivingTelemetry{ false };
};
