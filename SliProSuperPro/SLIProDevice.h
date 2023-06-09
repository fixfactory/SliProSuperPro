#pragma once

#include "SLI-Pro/SLI-Pro_samplecode.h"

// Forward declaration
extern "C" { typedef struct hid_device_ hid_device; }

class SLIProDevice
{
public:
    SLIProDevice();
    ~SLIProDevice();

    void init();
    void deinit();

    void open();
    void close();
    bool isOpen();

    void clear();
    void setBrightness(int brightness);
    void setGear(unsigned char gear);
    void setRpmLed(float percent);
    void setShiftLights(bool isOn);
    void setLeftString(const char* string);
    void setRightString(const char* string);

    bool write();

private:
    void lost();

    hid_device *m_handle = nullptr;

    _SLI_PROboardGlobalOUT m_boardGlobal = {};
    bool m_boardGlobalChanged = true;

    _SLI_PROboardOUT2 m_boardBrightness = {};
    bool m_boardBrightnessChanged = true;
};