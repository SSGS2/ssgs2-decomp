// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCOMTHINKSHOTPOS_H
#define CLASSES_CCOMTHINKSHOTPOS_H

#include "game/types.h"

class CComThinkShotPos {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int cachedPositions;                          // 0x4: 1 use in CComThinkShotPos_Ctor
    int cachedPositionCount;                          // 0x8: 1 use in CComThinkShotPos_Ctor
    int cachedPositionCapacity;                          // 0xc: 1 use in CComThinkShotPos_Ctor
    int cachedPositionMapCount;                         // 0x10: 1 use in CComThinkShotPos_Ctor
    int cachedPositionMapRoot;                         // 0x14: 1 use in CComThinkShotPos_Ctor
    char pad_018[0x4];                  // 0x18
    void* cachedPositionMapLast;                       // 0x1c: 1 use in CComThinkShotPos_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x10, 0x14
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCOMTHINKSHOTPOS_OFF(f, o) typedef char CComThinkShotPos_off_##f[((int)&((CComThinkShotPos*)0)->f == (o)) ? 1 : -1]
CCOMTHINKSHOTPOS_OFF(cachedPositions, 0x4);
CCOMTHINKSHOTPOS_OFF(cachedPositionCount, 0x8);
CCOMTHINKSHOTPOS_OFF(cachedPositionCapacity, 0xc);
CCOMTHINKSHOTPOS_OFF(cachedPositionMapCount, 0x10);
CCOMTHINKSHOTPOS_OFF(cachedPositionMapRoot, 0x14);
CCOMTHINKSHOTPOS_OFF(cachedPositionMapLast, 0x1c);
#undef CCOMTHINKSHOTPOS_OFF

#endif  // CLASSES_CCOMTHINKSHOTPOS_H
