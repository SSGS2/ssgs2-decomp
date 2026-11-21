// InputPointerState: explicit layout view; no inheritance, virtual methods, or RTTI.
// Explicit pointer state view used by Pointer_Reset/Pointer_Update/Pointer_GetButtonState.
#ifndef CLASSES_INPUTPOINTERSTATE_H
#define CLASSES_INPUTPOINTERSTATE_H

#include "game/types.h"
#include "classes/InputButtonState.h"

class InputPointerState {
public:
    char pad_000[0x30];
    char mutex[0x18]; // 0x30: next86BE0: lock at 0x30
    float motionDelta[3]; // 0x48: Pointer_Update: new position minus previous position
    float position[3]; // 0x54: Pointer_Update: sampled axes 1, 2, 3
    InputButtonState buttons[3]; // 0x60: Pointer_Update: three button records
    unsigned int stateFlags; // 0xD8: Pointer_Reset clears; Pointer_Update tests mask 6
    unsigned int mode; // 0xDC: Pointer_Reset clears
    char pad_0E0[0x4];
    unsigned char enabled; // 0xE4: Pointer_Reset writes at 0xE4; Pointer_Update tests 0xE4
};

#define OFF(f, o) typedef char InputPointerState_off_##f[((int)&((InputPointerState*)0)->f == (o)) ? 1 : -1]
OFF(mutex, 0x30);
OFF(motionDelta, 0x48);
OFF(position, 0x54);
OFF(buttons, 0x60);
OFF(stateFlags, 0xD8);
OFF(mode, 0xDC);
OFF(enabled, 0xE4);
#undef OFF

#endif
