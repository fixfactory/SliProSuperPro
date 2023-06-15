#pragma once

// Forward declaration
extern "C"
{
    typedef struct hid_device_ hid_device;
}

namespace sliPro
{
// Vendor id for an SLI-Pro
constexpr int kVendorId = 0x1dd2;

// Product id for an SLI-Pro
constexpr int kProductId = 0x0103;

// Max brightness value
constexpr unsigned int kMaxBrightness = 254;

// Receive inputs
struct BoardInput
{
    unsigned char id;        // zero
    unsigned char button[2]; // digital in first byte from 0 to 7 and second from 8 to 15
};

// Receive inputs
struct ProBoardInput : public BoardInput
{
    unsigned char button2[2];   // digital in first byte from 16 to 23 and second from 24 to 31
    unsigned char switches[12]; // switches 1 to 6
    unsigned char pots[4];      // 2 pots
};

// Controls the individual LEDs
// 1 = set, 0 = deactivated
struct BoardGlobalOutput
{
    unsigned char reportId;         // 0
    unsigned char reportType;       // 1
    unsigned char gear;             // gear
    unsigned char rpmLED[13];       // RPM leds
    unsigned char LED[11];          // 6 extra + 5 external leds
    unsigned char leftSegments[6];  // left segments as 6 chars (speed) +128 for dot
    unsigned char rightSegments[6]; // right segments as 6 chars (time)
    unsigned char spare[15];
};

// Controls global brightness
struct BoardGlobalBrightnessOutput
{
    unsigned char reportID;         // 0
    unsigned char reportType;       // 2
    unsigned char globalBrightness; // brightness 1 to 254
    unsigned char spare[51];
};

// Controls individual brightness: 1 to 254, 0 = skipped
struct BoardBrightnessOutput
{
    unsigned char reportID;                // 0
    unsigned char reportType;              // 4
    unsigned char brightnessGear;          // gear digit brightness
    unsigned char brightnessLED[24];       // 13 RPM leds brightness
    unsigned char brightnessLeftSegments;  // left segments brightness (speed)
    unsigned char brightnessRightSegments; // right segments brightness (time)
    unsigned char spare[25];
};
} // namespace sliPro

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

    hid_device* m_handle = nullptr;

    sliPro::BoardGlobalOutput m_boardGlobal = {};
    bool m_boardGlobalChanged = true;

    sliPro::BoardGlobalBrightnessOutput m_boardBrightness = {};
    bool m_boardBrightnessChanged = true;
};
