#include <iostream>

#include "SLIProDevice.h"
#include "Log.h"
#include "String.h"

#include "hidapi/hidapi.h"

constexpr size_t kMaxStr = 1024;

SLIProDevice::SLIProDevice()
{
}

SLIProDevice::~SLIProDevice()
{
}

void SLIProDevice::init()
{
    // Initialize the hidapi library
    int res = hid_init();

    memset(&m_boardGlobal, 0, sizeof(m_boardGlobal));
    m_boardGlobal.reportType = 1;

    memset(&m_boardBrightness, 0, sizeof(m_boardBrightness));
    m_boardBrightness.reportType = 2;
    m_boardBrightness.globalBrightness = sliPro::kMaxBrightness;
}

void SLIProDevice::deinit()
{
    // Finalize the hidapi library
    hid_exit();
}

void SLIProDevice::open()
{
    // Open the device using the VID & PID.
    // If multiple SLI-Pro devices are connected, only the first one
    // will be used.
    m_handle = hid_open(sliPro::kVendorId, sliPro::kProductId, NULL);
    if (!m_handle)
    {
        return;
    }

    LOG_INFO("SLI-Pro device found");

    // Read the Manufacturer String
    wchar_t wstr[kMaxStr];
    int res = hid_get_manufacturer_string(m_handle, wstr, kMaxStr);
    if (res < 0)
    {
        lost();
        return;
    }
    LOG_INFO("Manufacturer: %s", string::convertFromWide(wstr).c_str());

    // Read the Product String
    res = hid_get_product_string(m_handle, wstr, kMaxStr);
    if (res < 0)
    {
        lost();
        return;
    }
    LOG_INFO("Product: %s", string::convertFromWide(wstr).c_str());

    // Read the Serial Number String
    res = hid_get_serial_number_string(m_handle, wstr, kMaxStr);
    if (res < 0)
    {
        lost();
        return;
    }
    LOG_INFO("Serial Number: %s", string::convertFromWide(wstr).c_str());
}

void SLIProDevice::close()
{
    hid_close(m_handle);
    m_handle = nullptr;
}

bool SLIProDevice::isOpen()
{
    return m_handle != nullptr;
}

void SLIProDevice::clear()
{
    m_boardGlobal.gear = ' ';
    memset(&m_boardGlobal.rpmLED, 0, sizeof(m_boardGlobal.rpmLED));
    memset(&m_boardGlobal.LED, 0, sizeof(m_boardGlobal.LED));
    memset(&m_boardGlobal.leftSegments, ' ', sizeof(m_boardGlobal.leftSegments));
    memset(&m_boardGlobal.rightSegments, ' ', sizeof(m_boardGlobal.rightSegments));
    m_boardGlobalChanged = true;
}

void SLIProDevice::setBrightness(int brightness)
{
    m_boardBrightness.globalBrightness = (unsigned char)(brightness * sliPro::kMaxBrightness / 100);
    m_boardBrightnessChanged = true;
}

void SLIProDevice::setGear(unsigned char gear)
{
    m_boardGlobal.gear = gear;
    m_boardGlobalChanged = true;
}

void SLIProDevice::setRpmLed(float percent)
{
    int count = (int)(percent * 13);
    for (int i = 0; i < 13; ++i)
    {
        unsigned char isOn = i < count ? 1 : 0;
        m_boardGlobal.rpmLED[i] = isOn;
    }

    m_boardGlobalChanged = true;
}

void SLIProDevice::setShiftLights(bool isOn)
{
    m_boardGlobal.LED[0] = isOn;
    m_boardGlobal.LED[5] = isOn;
    m_boardGlobalChanged = true;
}

void SLIProDevice::setLeftString(const char *string)
{
    int len = std::min<int>((int)strlen(string), 6);
    for (int i = 0; i < len; ++i)
    {
        m_boardGlobal.leftSegments[i] = string[i];
    }

    m_boardGlobalChanged = true;
}

void SLIProDevice::setRightString(const char *string)
{
    int len = std::min<int>((int)strlen(string), 6);
    for (int i = 0; i < len; ++i)
    {
        m_boardGlobal.rightSegments[i] = string[i];
    }

    m_boardGlobalChanged = true;
}

bool SLIProDevice::write()
{
    if (m_boardGlobalChanged)
    {
        int res = hid_write(m_handle, reinterpret_cast<const unsigned char *>(&m_boardGlobal), sizeof(m_boardGlobal));
        if (res < 0)
        {
            lost();
            return false;
        }
        m_boardGlobalChanged = false;
    }

    if (m_boardBrightnessChanged)
    {
        int res =
            hid_write(m_handle, reinterpret_cast<const unsigned char *>(&m_boardBrightness), sizeof(m_boardBrightness));
        if (res < 0)
        {
            lost();
            return false;
        }
        m_boardBrightnessChanged = false;
    }

    return true;
}

void SLIProDevice::lost()
{
    LOG_INFO("SLI-Pro device lost!");
    close();
}
