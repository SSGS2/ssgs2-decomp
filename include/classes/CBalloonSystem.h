// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBALLOONSYSTEM_H
#define CLASSES_CBALLOONSYSTEM_H

#include "game/types.h"

class CBalloonSystem {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    int unk_8;                          // 0x8: 1 use in CBalloonSystem_ctor
    char pad_00C[0xc];                  // 0xc
    int lastHitSlotIndex;                         // 0x18: 1 use in CBalloonSystem_ctor
    char pad_01C[0xc];                  // 0x1c
    unsigned int balloonsHitThisFrame;                // 0x28: 5 uses in CBalloonSystem_OnProcess, CBalloonSystem_ctor, CBalloonSystem_HandleMsg

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x18
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBALLOONSYSTEM_OFF(f, o) typedef char CBalloonSystem_off_##f[((int)&((CBalloonSystem*)0)->f == (o)) ? 1 : -1]
CBALLOONSYSTEM_OFF(unk_8, 0x8);
CBALLOONSYSTEM_OFF(lastHitSlotIndex, 0x18);
CBALLOONSYSTEM_OFF(balloonsHitThisFrame, 0x28);
#undef CBALLOONSYSTEM_OFF

#endif  // CLASSES_CBALLOONSYSTEM_H
