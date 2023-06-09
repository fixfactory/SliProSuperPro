#pragma once

// SLI-Pro Example Code
// http://www.leobodnar.com/products/SLI-PRO/SLI-Pro_samplecode.html

// Vendor id for an SLI-Pro
static constexpr int kSLI_PRO_vendorId = 0x1dd2;

// Product id for an SLI-Pro
static constexpr int kSLI_PRO_productId = 0x0103;

// INPUT
typedef struct _SLI_MboardIN 
{
    unsigned char ID; // zero
    unsigned char Btn[2];   // digital in first byte from 0 to 7 and second from 8 to 15
} _SLI_MboardIN, * P_SLI_MboardIN;


typedef struct _SLI_PROboardIN : public _SLI_MboardIN
{
    unsigned char Btn2[2];   // digital in first byte from 16 to 23 and second from 24 to 31
    unsigned char SWT[12];   // switches 1 to 6  
    unsigned char Pots[4];   // 2 pots
} _SLI_PROboardIN, * P_SLI_PROboardIN;


// OUTPUT 1 = set ; 0 = deactivated
typedef struct _SLI_PROboardGlobalOUT 
{
    unsigned char ReportID;      //  0
    unsigned char ReportType;      //1
    unsigned char Gear;         //  gear
    unsigned char RPMLED[13];      //  RPM leds
    unsigned char LED[11];      //  6 extra + 5 external leds
    unsigned char Left7Segs[6];   //  7segs left as 6 chars (speed) +128 for dot
    unsigned char Right7Segs[6];   //  7segs right as 6 chars (time)
    unsigned char Spare[15];
} _SLI_PROboardGlobalOUT, * P_SLI_PROboardGlobalOUT;

// global brightness - 1 to 254

typedef struct _SLI_PROboardOUT2 
{
    unsigned char ReportID;         //  0
    unsigned char ReportType;         //2
    unsigned char GlobalBrightness;   //brightness
    unsigned char Spare[51];
} _SLI_PROboardOUT2, * P_SLI_PROboardOUT2;


// individual brightness - 1 to 254 ; 0 = skiped

typedef struct _SLI_PROboardOUT4 
{
    unsigned char ReportID;         //  0
    unsigned char ReportType;         //4
    unsigned char BritGear;         //  gear digit brightness
    unsigned char BritLED[24];      //  13 RPM leds brightness
    unsigned char BritLeft7Segs;      //  7segs left brightness (speed)
    unsigned char BritRight7Segs;      //  7segs right brightness (time)
    unsigned char Spare[25];
} _SLI_PROboardOUT4, * P_SLI_PROboardOUT4;
