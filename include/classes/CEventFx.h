// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CEVENTFX_H
#define CLASSES_CEVENTFX_H

#include "game/types.h"

class CEventFx {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    unsigned int nameLength;                 // 0x4: 1 use in CEventFx_Ctor
    void* nameData;                        // 0x8: 4 uses in CEventFx_Ctor, CEventFx_dtor
    void* spray;                        // 0xc: 8 uses in CEventFx_Ctor, CEventFx_dtor
    void* sequence;                       // 0x10: 8 uses in CEventFx_Ctor, CEventFx_dtor
    unsigned int bypassResourceCache;                // 0x14: 2 uses in CEventFx_Ctor
    void* positionOffset;                       // 0x18: 2 uses in CEventFx_Ctor
    char pad_01C[0x18];                 // 0x1c
    void* texture;                       // 0x34: 2 uses in CEventFx_Ctor
    void* eventVtable;                       // 0x38: 2 uses in CEventFx_Ctor, CEventFx_dtor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x1, 0x1c, 0x28
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CEVENTFX_OFF(f, o) typedef char CEventFx_off_##f[((int)&((CEventFx*)0)->f == (o)) ? 1 : -1]
CEVENTFX_OFF(nameLength, 0x4);
CEVENTFX_OFF(nameData, 0x8);
CEVENTFX_OFF(spray, 0xc);
CEVENTFX_OFF(sequence, 0x10);
CEVENTFX_OFF(bypassResourceCache, 0x14);
CEVENTFX_OFF(positionOffset, 0x18);
CEVENTFX_OFF(texture, 0x34);
CEVENTFX_OFF(eventVtable, 0x38);
#undef CEVENTFX_OFF

#endif  // CLASSES_CEVENTFX_H
