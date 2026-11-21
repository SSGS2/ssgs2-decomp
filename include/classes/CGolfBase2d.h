// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CGOLFBASE2D_H
#define CLASSES_CGOLFBASE2D_H

#include "game/types.h"

class CGolfBase2d {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x58];                 // 0x4
    int showExtraMeterImage;                         // 0x5c: 2 uses in CGolfBase2d_Ctor, CGolfBase2d_Draw
    int displayMode;                         // 0x60: 7 uses in CGolfBase2d_RebuildDirtyLayout, CGolfBase2d_Ctor, CGolfBase2d_Draw
    int markersEnabled;                         // 0x64: 2 uses in CGolfBase2d_Ctor, CGolfBase2d_RebuildDirtyLayout
    int markerIndex;                         // 0x68: 5 uses in CGolfBase2d_RebuildDirtyLayout, CGolfBase2d_Ctor
    int markerCount;                         // 0x6c: 4 uses in CGolfBase2d_RebuildDirtyLayout, CGolfBase2d_Ctor
    float markerOffsetY0;                       // 0x70: 1 use in CGolfBase2d_Ctor
    int markerColor0;                         // 0x74: 1 use in CGolfBase2d_Ctor
    float markerOffsetY1;                       // 0x78: 1 use in CGolfBase2d_Ctor
    int markerColor1;                         // 0x7c: 1 use in CGolfBase2d_Ctor
    float markerOffsetY2;                       // 0x80: 1 use in CGolfBase2d_Ctor
    int markerColor2;                         // 0x84: 1 use in CGolfBase2d_Ctor
    float markerOffsetY3;                       // 0x88: 1 use in CGolfBase2d_Ctor
    int markerColor3;                         // 0x8c: 1 use in CGolfBase2d_Ctor
    float markerOffsetY4;                       // 0x90: 1 use in CGolfBase2d_Ctor
    int markerColor4;                         // 0x94: 1 use in CGolfBase2d_Ctor
    float markerOffsetY5;                       // 0x98: 1 use in CGolfBase2d_Ctor
    int markerColor5;                         // 0x9c: 1 use in CGolfBase2d_Ctor
    float markerOffsetY6;                       // 0xa0: 1 use in CGolfBase2d_Ctor
    int markerColor6;                         // 0xa4: 1 use in CGolfBase2d_Ctor
    float markerOffsetY7;                       // 0xa8: 1 use in CGolfBase2d_Ctor
    int markerColor7;                         // 0xac: 1 use in CGolfBase2d_Ctor
    float markerOffsetY8;                       // 0xb0: 1 use in CGolfBase2d_Ctor
    int markerColor8;                         // 0xb4: 1 use in CGolfBase2d_Ctor
    float markerOffsetY9;                       // 0xb8: 1 use in CGolfBase2d_Ctor
    int markerColor9;                         // 0xbc: 1 use in CGolfBase2d_Ctor
    float markerOffsetY10;                       // 0xc0: 1 use in CGolfBase2d_Ctor
    int markerColor10;                         // 0xc4: 1 use in CGolfBase2d_Ctor
    float markerOffsetY11;                       // 0xc8: 1 use in CGolfBase2d_Ctor
    int markerColor11;                         // 0xcc: 1 use in CGolfBase2d_Ctor
    float markerOffsetY12;                       // 0xd0: 1 use in CGolfBase2d_Ctor
    int markerColor12;                         // 0xd4: 1 use in CGolfBase2d_Ctor
    float markerOffsetY13;                       // 0xd8: 1 use in CGolfBase2d_Ctor
    int markerColor13;                         // 0xdc: 1 use in CGolfBase2d_Ctor
    float markerOffsetY14;                       // 0xe0: 1 use in CGolfBase2d_Ctor
    int markerColor14;                         // 0xe4: 1 use in CGolfBase2d_Ctor
    float markerOffsetY15;                       // 0xe8: 1 use in CGolfBase2d_Ctor
    int markerColor15;                         // 0xec: 1 use in CGolfBase2d_Ctor
    float markerOffsetY16;                       // 0xf0: 1 use in CGolfBase2d_Ctor
    int markerColor16;                         // 0xf4: 1 use in CGolfBase2d_Ctor
    float markerOffsetY17;                       // 0xf8: 1 use in CGolfBase2d_Ctor
    int markerColor17;                         // 0xfc: 1 use in CGolfBase2d_Ctor
    float markerOffsetY18;                      // 0x100: 1 use in CGolfBase2d_Ctor
    int markerColor18;                        // 0x104: 1 use in CGolfBase2d_Ctor
    float markerOffsetY19;                      // 0x108: 1 use in CGolfBase2d_Ctor
    int markerColor19;                        // 0x10c: 1 use in CGolfBase2d_Ctor
    float markerOffsetY20;                      // 0x110: 1 use in CGolfBase2d_Ctor
    int markerColor20;                        // 0x114: 1 use in CGolfBase2d_Ctor
    int confirmButtonEnabled;                        // 0x118: 8 uses in CGolfBase2d_SetConfirmButtonEnabled, CGolfBase2d_RebuildDirtyLayout, CGolfBase2d_Ctor...
    int actionButtonsEnabled;                        // 0x11c: 3 uses in CGolfBase2d_RebuildDirtyLayout, CGolfBase2d_Ctor
    int actionButtonsVisible;                        // 0x120: 2 uses in CGolfBase2d_Ctor, CGolfBase2d_RebuildDirtyLayout
    int confirmButtonActivated;                        // 0x124: 9 uses in CGolfBase2d_ActivateConfirmButton, CGolfBase2d_Ctor, CGolfBase2d_SetConfirmButtonEnabled...
    int cancelButtonActivated;                        // 0x128: 8 uses in CGolfBase2d_ActivateCancelButton, CGolfBase2d_Ctor, CGolfBase2d_StartFadeInAndClearActionButtons...
    int tintColor;                        // 0x12c: 4 uses in CGolfBase2d_RebuildDirtyLayout, CGolfBase2d_Ctor, CGolfBase2d_SetTintColor
    float displayValue;                      // 0x130: 6 uses in CGolfBase2d_RebuildDirtyLayout, CGolfBase2d_SetDisplayValue, CGolfBase2d_Ctor
    float pangyaTargetX;                      // 0x134: 4 uses in CGolfBase2d_SetPangyaTargetX, CGolfBase2d_Ctor, CGolfBase2d_RebuildDirtyLayout
    float pangyaZoneWidth;                      // 0x138: 4 uses in CGolfBase2d_SetPangyaZoneWidth, CGolfBase2d_Ctor, CGolfBase2d_RebuildDirtyLayout
    float meterOverlayWidth;                      // 0x13c: 4 uses in CGolfBase2d_SetMeterOverlayWidth, CGolfBase2d_Ctor, CGolfBase2d_RebuildDirtyLayout
    float transitionStartValue;                      // 0x140: 4 uses in CGolfBase2d_RebuildDirtyLayout, CGolfBase2d_Ctor, CGolfBase2d_StartMeterValueTransition
    int valueTransitionFramesRemaining;                        // 0x144: 4 uses in CGolfBase2d_UpdateFadeAndMeterTransition, CGolfBase2d_StartMeterValueTransition, CGolfBase2d_RebuildDirtyLayout
    int valueTransitionActive;                        // 0x148: 3 uses in CGolfBase2d_StopValueTransition, CGolfBase2d_StartMeterValueTransition, CGolfBase2d_RebuildDirtyLayout
    int fadeFramesRemaining;                        // 0x14c: 7 uses in CGolfBase2d_UpdateFadeAndMeterTransition, CGolfBase2d_StartFadeInAndClearActionButtons, CGolfBase2d_StartFadeOut...
    int fadeState;                        // 0x150: 15 uses in CGolfBase2d_UpdateFadeAndMeterTransition, CGolfBase2d_StartFadeOut, CGolfBase2d_ShowImmediately...
    float fadeOpacity;                      // 0x154: 8 uses in CGolfBase2d_UpdateFadeAndMeterTransition, CGolfBase2d_RebuildDirtyLayout, CGolfBase2d_Ctor...
    int layoutReady;                        // 0x158: 4 uses in CGolfBase2d_RebuildDirtyLayout, CGolfBase2d_vfunc_2, CGolfBase2d_Ctor
    int layoutDirty;                        // 0x15c: 20 uses in CGolfBase2d_UpdateFadeAndMeterTransition, CGolfBase2d_ProcessActivatedActionButtons, CGolfBase2d_RebuildDirtyLayout...
    float markerBaseY;                      // 0x160: 5 uses in CGolfBase2d_vfunc_2, CGolfBase2d_Ctor, CGolfBase2d_RebuildDirtyLayout
    void* golfBaseVtable;                      // 0x164: 5 uses in CGolfBase2d_RebuildDirtyLayout, CGolfBase2d_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x8
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CGOLFBASE2D_OFF(f, o) typedef char CGolfBase2d_off_##f[((int)&((CGolfBase2d*)0)->f == (o)) ? 1 : -1]
CGOLFBASE2D_OFF(showExtraMeterImage, 0x5c);
CGOLFBASE2D_OFF(displayMode, 0x60);
CGOLFBASE2D_OFF(markersEnabled, 0x64);
CGOLFBASE2D_OFF(markerIndex, 0x68);
CGOLFBASE2D_OFF(markerCount, 0x6c);
CGOLFBASE2D_OFF(markerOffsetY0, 0x70);
CGOLFBASE2D_OFF(markerColor0, 0x74);
CGOLFBASE2D_OFF(markerOffsetY1, 0x78);
CGOLFBASE2D_OFF(markerColor1, 0x7c);
CGOLFBASE2D_OFF(markerOffsetY2, 0x80);
CGOLFBASE2D_OFF(markerColor2, 0x84);
CGOLFBASE2D_OFF(markerOffsetY3, 0x88);
CGOLFBASE2D_OFF(markerColor3, 0x8c);
CGOLFBASE2D_OFF(markerOffsetY4, 0x90);
CGOLFBASE2D_OFF(markerColor4, 0x94);
CGOLFBASE2D_OFF(markerOffsetY5, 0x98);
CGOLFBASE2D_OFF(markerColor5, 0x9c);
CGOLFBASE2D_OFF(markerOffsetY6, 0xa0);
CGOLFBASE2D_OFF(markerColor6, 0xa4);
CGOLFBASE2D_OFF(markerOffsetY7, 0xa8);
CGOLFBASE2D_OFF(markerColor7, 0xac);
CGOLFBASE2D_OFF(markerOffsetY8, 0xb0);
CGOLFBASE2D_OFF(markerColor8, 0xb4);
CGOLFBASE2D_OFF(markerOffsetY9, 0xb8);
CGOLFBASE2D_OFF(markerColor9, 0xbc);
CGOLFBASE2D_OFF(markerOffsetY10, 0xc0);
CGOLFBASE2D_OFF(markerColor10, 0xc4);
CGOLFBASE2D_OFF(markerOffsetY11, 0xc8);
CGOLFBASE2D_OFF(markerColor11, 0xcc);
CGOLFBASE2D_OFF(markerOffsetY12, 0xd0);
CGOLFBASE2D_OFF(markerColor12, 0xd4);
CGOLFBASE2D_OFF(markerOffsetY13, 0xd8);
CGOLFBASE2D_OFF(markerColor13, 0xdc);
CGOLFBASE2D_OFF(markerOffsetY14, 0xe0);
CGOLFBASE2D_OFF(markerColor14, 0xe4);
CGOLFBASE2D_OFF(markerOffsetY15, 0xe8);
CGOLFBASE2D_OFF(markerColor15, 0xec);
CGOLFBASE2D_OFF(markerOffsetY16, 0xf0);
CGOLFBASE2D_OFF(markerColor16, 0xf4);
CGOLFBASE2D_OFF(markerOffsetY17, 0xf8);
CGOLFBASE2D_OFF(markerColor17, 0xfc);
CGOLFBASE2D_OFF(markerOffsetY18, 0x100);
CGOLFBASE2D_OFF(markerColor18, 0x104);
CGOLFBASE2D_OFF(markerOffsetY19, 0x108);
CGOLFBASE2D_OFF(markerColor19, 0x10c);
CGOLFBASE2D_OFF(markerOffsetY20, 0x110);
CGOLFBASE2D_OFF(markerColor20, 0x114);
CGOLFBASE2D_OFF(confirmButtonEnabled, 0x118);
CGOLFBASE2D_OFF(actionButtonsEnabled, 0x11c);
CGOLFBASE2D_OFF(actionButtonsVisible, 0x120);
CGOLFBASE2D_OFF(confirmButtonActivated, 0x124);
CGOLFBASE2D_OFF(cancelButtonActivated, 0x128);
CGOLFBASE2D_OFF(tintColor, 0x12c);
CGOLFBASE2D_OFF(displayValue, 0x130);
CGOLFBASE2D_OFF(pangyaTargetX, 0x134);
CGOLFBASE2D_OFF(pangyaZoneWidth, 0x138);
CGOLFBASE2D_OFF(meterOverlayWidth, 0x13c);
CGOLFBASE2D_OFF(transitionStartValue, 0x140);
CGOLFBASE2D_OFF(valueTransitionFramesRemaining, 0x144);
CGOLFBASE2D_OFF(valueTransitionActive, 0x148);
CGOLFBASE2D_OFF(fadeFramesRemaining, 0x14c);
CGOLFBASE2D_OFF(fadeState, 0x150);
CGOLFBASE2D_OFF(fadeOpacity, 0x154);
CGOLFBASE2D_OFF(layoutReady, 0x158);
CGOLFBASE2D_OFF(layoutDirty, 0x15c);
CGOLFBASE2D_OFF(markerBaseY, 0x160);
CGOLFBASE2D_OFF(golfBaseVtable, 0x164);
#undef CGOLFBASE2D_OFF

#endif  // CLASSES_CGOLFBASE2D_H
