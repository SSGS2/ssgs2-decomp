// InputActionState: explicit layout view; no inheritance, virtual methods, or RTTI.
// Partial action entry view for CInputManager_dtor and Pointer_Reset/80186E08.
#ifndef CLASSES_INPUTACTIONSTATE_H
#define CLASSES_INPUTACTIONSTATE_H

#include "game/types.h"

class InputActionState {
public:
    char pad_000[0x4];
    void* bindings; // 0x4: CInputManager_dtor: freed binding storage
    int bindingCount; // 0x8: CInputManager_dtor: storage count cleared
    char pad_00C[0xc];
    unsigned int active; // 0x18: Pointer_Update: computed from bindings
    unsigned int previousActive; // 0x1C: Pointer_Update: previous active low bit
    char pad_020[0x4];
    unsigned char forceActive; // 0x24: Pointer_Update: forces active then clears
};

#define OFF(f, o) typedef char InputActionState_off_##f[((int)&((InputActionState*)0)->f == (o)) ? 1 : -1]
OFF(bindings, 0x4);
OFF(bindingCount, 0x8);
OFF(active, 0x18);
OFF(previousActive, 0x1C);
OFF(forceActive, 0x24);
#undef OFF

#endif
