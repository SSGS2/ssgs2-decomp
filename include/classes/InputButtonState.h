// InputButtonState: explicit layout view; no inheritance, virtual methods, or RTTI.
// Button records in Pointer_Reset/Pointer_Update have stride 0x28.
#ifndef CLASSES_INPUTBUTTONSTATE_H
#define CLASSES_INPUTBUTTONSTATE_H

#include "game/types.h"

class InputButtonState {
public:
    unsigned int state; // 0x0: Pointer_Update: low bit current state, next bit prior state
    float pressPosition[3]; // 0x4: Pointer_Update: copied on state 1
    float releasePosition[3]; // 0x10: Pointer_Update: copied on state 2
    unsigned int pressIntervalMs; // 0x1C: Pointer_Update: current tick minus last press tick
    unsigned int lastPressMs; // 0x20: Pointer_Update: updated on state 1
    unsigned char forcePressed; // 0x24: Pointer_Update: forces low state bit to 1 then clears
    unsigned char suppressPressed; // 0x25: Pointer_Update: suppresses sampled low state bit
    unsigned short unk_26; // 0x26: record stride 0x28 in Pointer_Reset/Pointer_Update
};

#define OFF(f, o) typedef char InputButtonState_off_##f[((int)&((InputButtonState*)0)->f == (o)) ? 1 : -1]
OFF(state, 0x0);
OFF(pressPosition, 0x4);
OFF(releasePosition, 0x10);
OFF(pressIntervalMs, 0x1C);
OFF(lastPressMs, 0x20);
OFF(forcePressed, 0x24);
OFF(suppressPressed, 0x25);
OFF(unk_26, 0x26);
#undef OFF

#endif
