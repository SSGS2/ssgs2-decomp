// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSPOT_H
#define CLASSES_CSPOT_H

#include "game/types.h"

class CSpot {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x18];                 // 0x4
    void* distanceText;                       // 0x1c: 5 uses in CSpot_OnDestroy, CSpot_OnLoad, CSpot_Ctor
    void* playerNumberText;                       // 0x20: 5 uses in CSpot_OnDestroy, CSpot_OnLoad, CSpot_Ctor
    float position[3]; // 0x24: CSpot_OnProcess copies targetPosition here
    float previousPosition[3]; // 0x30: CSpot_OnInit Vec3_Zero
    float transitionReferencePosition[3]; // 0x3C: CSpot_HandleMsg case 6 copies the supplied vector
    float heading;                       // 0x48: 3 uses in CSpot_HandleMsg, CSpot_OnInit
    float positionBlend;                       // 0x4c: 5 uses in CSpot_OnProcess, CSpot_HandleMsg, CSpot_OnInit
    float unk_50[3]; // 0x50: CSpot_OnInit Vec3_Zero
    float initialRandomOffset[3]; // 0x5C: CSpot_OnInit stores scaled random horizontal vector
    float targetPosition[3]; // 0x68: CSpot_OnProcess stores GetGroundPoint result
    char pad_074[0x4];
    int groundHitIndex; // 0x78: CSpot_OnProcess GetGroundPoint out parameter
    float rotationPhase; // 0x7C: CSpot_DrawFallingMarker rotates the marker by 0.7 * this value
    unsigned char enabled;               // 0x80: 5 uses in CSpot_HandleMsg, CSpot_OnInit, CSpot_OnProcess
    char pad_081[3];
    float arrowWidth; // 0x84: CSpot_DrawMarkersAndLabels arrowRect/shadowRect width
    float endpointSize; // 0x88: CSpot_DrawMarkersAndLabels validRect/invalidRect width and height
    void* clubRangeInfo;                       // 0x8c: 6 uses in CSpot_OnDestroy, CSpot_OnLoad, CSpot_Ctor
    unsigned int markerColor; // 0x90: CSpot_DrawMarkersAndLabels interpolates packed colors and passes to drawing
    char pad_094[4];
    float labelHeightOffset; // 0x98: CSpot_DrawMarkersAndLabels subtracts this from projected label y
    void* indicatorBitmap;                       // 0x9c: 2 uses in CSpot_OnLoad, CSpot_Ctor
    void* fallingBitmap;                       // 0xa0: 2 uses in CSpot_OnLoad, CSpot_Ctor
    void* slopeBitmap;                       // 0xa4: 2 uses in CSpot_OnLoad, CSpot_Ctor
    void* targetHolePositionBitmap;                       // 0xa8: 2 uses in CSpot_OnLoad, CSpot_Ctor
    float pulsePhase;                       // 0xac: 6 uses in CSpot_OnProcess, CSpot_OnInit, CSpot_HandleMsg
    float arrowProgress;                       // 0xb0: 1 use in CSpot_OnProcess
    char pad_0B4[0x8];                  // 0xb4
    void* validTargetOverlay;                       // 0xbc: 2 uses in CSpot_OnLoad, CSpot_Ctor
    void* invalidTargetOverlay;                       // 0xc0: 2 uses in CSpot_OnLoad, CSpot_Ctor
    void* arrowOverlay;                       // 0xc4: 2 uses in CSpot_OnLoad, CSpot_Ctor
    void* playerMarkerOverlay;                       // 0xc8: 2 uses in CSpot_OnLoad, CSpot_Ctor
    char pad_0CC[0x16c];                // 0xcc
    float markerHeading;                      // 0x238: 3 uses in CSpot_OnProcess, CSpot_OnInit
    void* scoreTargetBitmap;                      // 0x23c: 2 uses in CSpot_OnLoad, CSpot_Ctor
    char pad_240[0x14];
    float markerScreen[2]; // 0x254: CSpot_DrawMarkersAndLabels projected marker x/y
    char pad_25C[0x30];
    float ballScreen[2]; // 0x28C: CSpot_DrawMarkersAndLabels projected ball x/y
    char pad_294[0xA0];
    unsigned char targetDirty;              // 0x334: 7 uses in CSpot_OnProcess, CSpot_HandleMsg, CSpot_OnInit
    char pad_335[0xb];                  // 0x335
    unsigned char playerMarkersVisible;              // 0x340: 3 uses in CSpot_OnProcess, CSpot_OnPreLoadInit
    char pad_341[0x3];                  // 0x341
    int messageValueAtLoad;                        // 0x344: 2 uses in CSpot_OnLoad, CSpot_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x18, 0x24, 0x30, 0x3c, 0x50, 0x5c, 0x68, 0x78, 0xd0, 0x254, 0x28c, 0x2c4, 0x2fc
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSPOT_OFF(f, o) typedef char CSpot_off_##f[((int)&((CSpot*)0)->f == (o)) ? 1 : -1]
CSPOT_OFF(position, 0x24);
CSPOT_OFF(transitionReferencePosition, 0x3C);
CSPOT_OFF(rotationPhase, 0x7C);
CSPOT_OFF(arrowWidth, 0x84);
CSPOT_OFF(endpointSize, 0x88);
CSPOT_OFF(markerColor, 0x90);
CSPOT_OFF(labelHeightOffset, 0x98);
CSPOT_OFF(markerScreen, 0x254);
CSPOT_OFF(ballScreen, 0x28C);
CSPOT_OFF(previousPosition, 0x30);
CSPOT_OFF(unk_50, 0x50);
CSPOT_OFF(initialRandomOffset, 0x5C);
CSPOT_OFF(targetPosition, 0x68);
CSPOT_OFF(groundHitIndex, 0x78);
CSPOT_OFF(distanceText, 0x1c);
CSPOT_OFF(playerNumberText, 0x20);
CSPOT_OFF(heading, 0x48);
CSPOT_OFF(positionBlend, 0x4c);
CSPOT_OFF(enabled, 0x80);
CSPOT_OFF(clubRangeInfo, 0x8c);
CSPOT_OFF(indicatorBitmap, 0x9c);
CSPOT_OFF(fallingBitmap, 0xa0);
CSPOT_OFF(slopeBitmap, 0xa4);
CSPOT_OFF(targetHolePositionBitmap, 0xa8);
CSPOT_OFF(pulsePhase, 0xac);
CSPOT_OFF(arrowProgress, 0xb0);
CSPOT_OFF(validTargetOverlay, 0xbc);
CSPOT_OFF(invalidTargetOverlay, 0xc0);
CSPOT_OFF(arrowOverlay, 0xc4);
CSPOT_OFF(playerMarkerOverlay, 0xc8);
CSPOT_OFF(markerHeading, 0x238);
CSPOT_OFF(scoreTargetBitmap, 0x23c);
CSPOT_OFF(targetDirty, 0x334);
CSPOT_OFF(playerMarkersVisible, 0x340);
CSPOT_OFF(messageValueAtLoad, 0x344);
#undef CSPOT_OFF

#endif  // CLASSES_CSPOT_H
