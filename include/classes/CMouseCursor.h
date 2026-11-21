// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CMOUSECURSOR_H
#define CLASSES_CMOUSECURSOR_H

#include "game/types.h"

class CMouseCursor {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    float screenX;                      // 0x10: CMouseCursor_MoveTo (0x80197724), CMouseCursor_HitTestRegions (0x8019631C), fn_8019631C_hit
    float screenY;                      // 0x14: CMouseCursor_MoveTo (0x80197724), CMouseCursor_HitTestRegions (0x8019631C), fn_8019631C_hit
    float savedScreenX; // 0x18: CMouseCursor_SavePosition (0x801976A4), restored when pointer input is enabled (0x80197614)
    float savedScreenY; // 0x1c: second component of that Vec2_Copy
    char pad_020[0x8];
    float targetX;                      // 0x28: CMouseCursor_MoveTo (0x80197724)
    float targetY;                      // 0x2c: CMouseCursor_MoveTo (0x80197724)
    float moveDeltaX; // 0x30: CMouseCursor_MoveTo (0x80197724), screen position minus target
    float moveDeltaY; // 0x34: second component of that Vec2_Subtract
    int hitTestMode;                         // 0x38: 1 use in CMouseCursor_Ctor (0x80195AE4)
    char pad_03C[0x4];                  // 0x3c
    float idleTime;                       // 0x40: CMouseCursor_MoveTo (0x80197724), CMouseCursor_SetPointerInputEnabled (0x80197614), CMouseCursor_SetRadialMenuEnabled (0x801976B4)
    char pad_044[0x8];                  // 0x44
    unsigned char visible;               // 0x4c: CMouseCursor_MoveTo (0x80197724), CMouseCursor_SetPointerInputEnabled (0x80197614), CMouseCursor_SetRadialMenuEnabled (0x801976B4)
    unsigned char pointerInputEnabled;               // 0x4d: CMouseCursor_SetPointerInputEnabled (0x80197614)
    char pad_04E[0x2];                  // 0x4e
    int hitRegion;                      // 0x50: CMouseCursor_MoveTo (0x80197724), CMouseCursor_HitTestRegions (0x8019631C), CMouseCursor_UpdateGolfActions (0x80196760)
    int cursorState;                         // 0x54: CMouseCursor_MoveTo (0x80197724), CMouseCursor_SetPointerInputEnabled (0x80197614), CMouseCursor_SetRadialMenuEnabled (0x801976B4)
    unsigned char primaryActionEnabled;               // 0x58: CMouseCursor_UpdateGolfActions (0x80196760)
    unsigned char secondaryActionEnabled;               // 0x59: CMouseCursor_UpdateGolfActions (0x80196760)
    char pad_05A[0x6];                  // 0x5a
    unsigned char* clipRect;            // 0x60: CMouseCursor_UpdateGolfActions (0x80196760)
    unsigned char radialMenuEnabled;               // 0x64: CMouseCursor_UpdateGolfActions (0x80196760), CMouseCursor_SetRadialMenuEnabled (0x801976B4)
    char pad_065[0x3];                  // 0x65
    float moveWeight;                       // 0x68: CMouseCursor_MoveTo (0x80197724)
    float moveWeightDecayRate;                       // 0x6c: CMouseCursor_MoveTo (0x80197724)
    char pad_070[0x10];                 // 0x70
    unsigned char alternateHudMode;               // 0x80: CMouseCursor_UpdateGolfActions (0x80196760)
    char pad_081[0x3];                  // 0x81
    int buttonDownRegions[3];           // 0x84: CMouseCursor_Update records hitRegion for each newly pressed button
    unsigned char radialMenuDragActive;               // 0x90: CMouseCursor_UpdateGolfActions (0x80196760)
    unsigned char pointerUpdateOverrideEnabled;               // 0x91: CMouseCursor_Reset clears; CMouseCursor_Update gates pointer updates
    char pad_092[0x2];                  // 0x92
    float overlayAnchorX;                       // 0x94: CMouseCursor_UpdateGolfActions (0x80196760)
    float overlayAnchorY;                       // 0x98: CMouseCursor_UpdateGolfActions (0x80196760)
    float minWorldX;                    // 0x9c: CMouseCursor_HitTestRegions (0x8019631C)
    float minWorldZ;                    // 0xa0: CMouseCursor_HitTestRegions (0x8019631C)
    float maxWorldX;                    // 0xa4: CMouseCursor_HitTestRegions (0x8019631C)
    float maxWorldZ;                    // 0xa8: CMouseCursor_HitTestRegions (0x8019631C)
    char pad_0AC[0x4];                  // 0xac
    float rayDirectionY;                       // 0xb0: CMouseCursor_HitTestRegions (0x8019631C)
    char pad_0B4[0x4];                  // 0xb4
    float rayIntersectionScale;                       // 0xb8: CMouseCursor_HitTestRegions (0x8019631C)
    unsigned char autoHideEnabled;      // 0xbc: CMouseCursor_Update accumulates idle time and clears visibility
    unsigned char regionEnabled[0x48];  // 0xbd: CMouseCursor_Reset fills with 1; CMouseCursor_Update indexes by hitRegion
    unsigned char enabled;              // 0x105: 1 use in CMouseCursor_Ctor (0x80195AE4)
    unsigned char extendedStateActive;              // 0x106: CMouseCursor_SetState (0x80197800)
    char pad_107[0x1];                  // 0x107
    int moveToActive;                        // 0x108: CMouseCursor_MoveTo (0x80197724)
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CMOUSECURSOR_OFF(f, o) typedef char CMouseCursor_off_##f[((int)&((CMouseCursor*)0)->f == (o)) ? 1 : -1]
CMOUSECURSOR_OFF(screenX, 0x10);
CMOUSECURSOR_OFF(screenY, 0x14);
CMOUSECURSOR_OFF(savedScreenX, 0x18);
CMOUSECURSOR_OFF(savedScreenY, 0x1c);
CMOUSECURSOR_OFF(moveDeltaX, 0x30);
CMOUSECURSOR_OFF(moveDeltaY, 0x34);
CMOUSECURSOR_OFF(targetX, 0x28);
CMOUSECURSOR_OFF(targetY, 0x2c);
CMOUSECURSOR_OFF(hitTestMode, 0x38);
CMOUSECURSOR_OFF(idleTime, 0x40);
CMOUSECURSOR_OFF(visible, 0x4c);
CMOUSECURSOR_OFF(pointerInputEnabled, 0x4d);
CMOUSECURSOR_OFF(hitRegion, 0x50);
CMOUSECURSOR_OFF(cursorState, 0x54);
CMOUSECURSOR_OFF(primaryActionEnabled, 0x58);
CMOUSECURSOR_OFF(secondaryActionEnabled, 0x59);
CMOUSECURSOR_OFF(clipRect, 0x60);
CMOUSECURSOR_OFF(radialMenuEnabled, 0x64);
CMOUSECURSOR_OFF(moveWeight, 0x68);
CMOUSECURSOR_OFF(moveWeightDecayRate, 0x6c);
CMOUSECURSOR_OFF(alternateHudMode, 0x80);
CMOUSECURSOR_OFF(buttonDownRegions, 0x84);
CMOUSECURSOR_OFF(radialMenuDragActive, 0x90);
CMOUSECURSOR_OFF(pointerUpdateOverrideEnabled, 0x91);
CMOUSECURSOR_OFF(overlayAnchorX, 0x94);
CMOUSECURSOR_OFF(overlayAnchorY, 0x98);
CMOUSECURSOR_OFF(minWorldX, 0x9c);
CMOUSECURSOR_OFF(minWorldZ, 0xa0);
CMOUSECURSOR_OFF(maxWorldX, 0xa4);
CMOUSECURSOR_OFF(maxWorldZ, 0xa8);
CMOUSECURSOR_OFF(rayDirectionY, 0xb0);
CMOUSECURSOR_OFF(rayIntersectionScale, 0xb8);
CMOUSECURSOR_OFF(autoHideEnabled, 0xbc);
CMOUSECURSOR_OFF(regionEnabled, 0xbd);
CMOUSECURSOR_OFF(enabled, 0x105);
CMOUSECURSOR_OFF(extendedStateActive, 0x106);
CMOUSECURSOR_OFF(moveToActive, 0x108);
CMOUSECURSOR_OFF(vtbl, 0x0);
#undef CMOUSECURSOR_OFF

#endif  // CLASSES_CMOUSECURSOR_H
