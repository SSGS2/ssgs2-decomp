// CFlyInfo: the flight-distance readout in the HUD.
//
// It owns the ground/pin text entries, the three result layouts and the text
// buffer, and it draws the distance to the pin and the carry details.
//
// The class is not declared with virtual functions: its vtable comes from the
// original data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CFLYINFO_H
#define CLASSES_CFLYINFO_H

#include "game/types.h"

class CFlyInfo {
public:
    CFlyInfo();
    void* Destroy(int deleting);
    void UpdateFlightFadesAndDistances(float deltaSeconds);
    void DrawFlightHud();

    void* vtbl;                     // 0x00
    void* fontResource;             // 0x04
    char pad_008[4];                // 0x08
    unsigned int color;             // 0x0C  ARGB
    int textMode;                   // 0x10
    char* textBuffer;               // 0x14
    float elapsedTime;              // 0x18  seconds since the shot
    void* groundText;               // 0x1C
    void* pinText;                  // 0x20
    void* dartsResultLayout;        // 0x24
    void* modeTwoResultLayout;      // 0x28
    void* modeTwoShotDetailLayout;  // 0x2C
    char labelFont[8];              // 0x30
    float groundTextOpacity;        // 0x38
    float pinTextOpacity;           // 0x3C
    float secondaryPinTextOpacity;  // 0x40
    float carryValueOpacity;        // 0x44
    float carryLabelsOpacity;       // 0x48
    float currentShotValueOpacity;  // 0x4C
    float shotDetailOpacities[5];   // 0x50
    float dartsTotalOpacity;        // 0x64
    float dartsBreakdownOpacity;    // 0x68
    char pad_06C[8];                // 0x6C
    float distanceFromStart;        // 0x74  yards travelled
    float distanceToPin;            // 0x78  yards remaining
    char groundName[8];             // 0x7C
    unsigned int previousGolfState; // 0x84
};

#define CFLYINFO_OFF(f, o) typedef char CFlyInfo_off_##f[((int)&((CFlyInfo*)0)->f == (o)) ? 1 : -1]
CFLYINFO_OFF(color, 0x0C);
CFLYINFO_OFF(textMode, 0x10);
CFLYINFO_OFF(textBuffer, 0x14);
CFLYINFO_OFF(elapsedTime, 0x18);
CFLYINFO_OFF(dartsResultLayout, 0x24);
CFLYINFO_OFF(modeTwoShotDetailLayout, 0x2C);
CFLYINFO_OFF(labelFont, 0x30);
CFLYINFO_OFF(shotDetailOpacities, 0x50);
CFLYINFO_OFF(distanceFromStart, 0x74);
CFLYINFO_OFF(distanceToPin, 0x78);
CFLYINFO_OFF(groundName, 0x7C);
CFLYINFO_OFF(previousGolfState, 0x84);
#undef CFLYINFO_OFF
typedef char CFlyInfo_size[(sizeof(CFlyInfo) == 0x88) ? 1 : -1];

#endif // CLASSES_CFLYINFO_H
