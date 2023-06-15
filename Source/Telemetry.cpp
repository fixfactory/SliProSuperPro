#include "Telemetry.h"
#include "Network.h"
#include "Log.h"
#include "Blackboard.h"
#include "Config.h"

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
    TimingManager::getSingleton().registerUpdateable(this);

    m_session = new WSASession();
    m_udpSocket = new UDPSocket();

    try
    {
        m_udpSocket->bindTo(config::udpPort);
        LOG_INFO("Listening to telemetry on port %i", config::udpPort);
    }
    catch (const std::system_error &error)
    {
        LOG_ERROR(error);
    }

    m_recvBuf.resize(sizeof(TelemetryData));
    memset(&m_telemetryData, 0, sizeof(TelemetryData));
}

void TelemetryManager::deinit()
{
    TimingManager::getSingleton().unregisterUpdateable(this);

    delete m_udpSocket;
    m_udpSocket = nullptr;

    delete m_session;
    m_session = nullptr;

    blackboard::telemetryData = nullptr;
}

void TelemetryManager::update(timing::seconds deltaTime)
{
    if (m_udpSocket->isBound())
    {
        recvTelemetry();
    }
}

void TelemetryManager::sleep(float sleepTime)
{
    try
    {
        m_udpSocket->sleep(sleepTime);
    }
    catch (const std::system_error &error)
    {
        LOG_ERROR(error);
    }
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
            if (m_recvBuf.size() != sizeof(TelemetryData))
            {
                LOG_ERROR("Malformed TelemetryData (size %i)", m_recvBuf.size());
                continue;
            }

            m_telemetryData = *reinterpret_cast<TelemetryData *>(m_recvBuf.data());

            if (!m_receivingTelemetry)
            {
                LOG_INFO("Started receiving telemetry data");
                m_receivingTelemetry = true;
                blackboard::telemetryData = &m_telemetryData;
            }

            m_lastDataTime = time;
        }

        if (m_receivingTelemetry && time - m_lastDataTime > kDataTimeout)
        {
            LOG_INFO("Stopped receiving telemetry data");
            m_receivingTelemetry = false;
            blackboard::telemetryData = nullptr;
        }
    }
    catch (const std::system_error &error)
    {
        LOG_ERROR(error);
        m_receivingTelemetry = false;
        blackboard::telemetryData = nullptr;
    }
}
