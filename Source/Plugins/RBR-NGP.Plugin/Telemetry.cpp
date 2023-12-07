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

#include "Telemetry.h"
#include "Network.h"
#include "Log.h"

static const std::chrono::duration<float> kDataTimeout{ 2.f };

TelemetryManager &TelemetryManager::getSingleton()
{
    static TelemetryManager s_singleton;
    return s_singleton;
}

TelemetryManager::TelemetryManager()
{
}

TelemetryManager::~TelemetryManager()
{
}

void TelemetryManager::init()
{
    m_session = new WSASession();
    m_udpSocket = new UDPSocket();

    try
    {
        // TODO: Port should be user configurable.
        constexpr short kTempPort = 6776;
        m_udpSocket->bindTo(kTempPort);
        LOG_INFO("Listening to telemetry on port %i", kTempPort);
    }
    catch (const std::system_error &error)
    {
        LOG_ERROR(error);
    }

    m_recvBuf.resize(sizeof(RBRTelemetryData));
    memset(&m_rbrTelemetryData, 0, sizeof(RBRTelemetryData));
}

void TelemetryManager::deinit()
{
    delete m_udpSocket;
    m_udpSocket = nullptr;

    delete m_session;
    m_session = nullptr;
}

bool TelemetryManager::fetchTelemetryData()
{
    if (m_udpSocket->isBound())
    {
        recvTelemetry();
        if (m_receivingTelemetry)
        {
            m_telemetryData.gear = m_rbrTelemetryData.control_.gear_;
            m_telemetryData.rpm = m_rbrTelemetryData.car_.engine_.rpm_;
            m_telemetryData.speedKph = m_rbrTelemetryData.car_.speed_;
            return true;
        }
    }
    return false;
}

bool TelemetryManager::isReceivingTelemetry() const
{
    return m_receivingTelemetry;
}

const plugin::TelemetryData &TelemetryManager::getTelemetryData() const
{
    return m_telemetryData;
}

const RBRTelemetryData &TelemetryManager::getRBRTelemetryData() const
{
    return m_rbrTelemetryData;
}

void TelemetryManager::recvTelemetry()
{
    sockaddr_in fromAddr;

    try
    {
        auto time = std::chrono::steady_clock::now();
        while (m_udpSocket->hasData())
        {
            m_udpSocket->recvData(m_recvBuf, fromAddr);
            if (m_recvBuf.size() != sizeof(RBRTelemetryData))
            {
                LOG_ERROR("Malformed TelemetryData (size %i)", m_recvBuf.size());
                continue;
            }

            m_rbrTelemetryData = *reinterpret_cast<RBRTelemetryData *>(m_recvBuf.data());

            if (!m_receivingTelemetry)
            {
                LOG_INFO("Started receiving telemetry data");
                m_receivingTelemetry = true;
            }

            m_lastDataTime = time;
        }

        if (m_receivingTelemetry && time - m_lastDataTime > kDataTimeout)
        {
            LOG_INFO("Stopped receiving telemetry data");
            m_receivingTelemetry = false;
        }
    }
    catch (const std::system_error &error)
    {
        LOG_ERROR(error);
        m_receivingTelemetry = false;
    }
}
