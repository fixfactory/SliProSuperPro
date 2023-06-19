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
        constexpr short kTempPort = 6777;
        m_udpSocket->bindTo(kTempPort);
        LOG_INFO("Listening to telemetry on port %i", kTempPort);
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
        return m_receivingTelemetry;
    }
    return false;
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
