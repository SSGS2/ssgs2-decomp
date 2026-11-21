// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSMALLBALLOON_H
#define CLASSES_CSMALLBALLOON_H

#include "game/types.h"

class CSmallBalloon {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xb8];                 // 0x4
    void* effectPosition;                       // 0xbc: 1 use in CSmallBalloon_StartExplosionEffect

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x24, 0x70, 0x7c, 0xa0, 0xa4, 0xec, 0xee
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSMALLBALLOON_OFF(f, o) typedef char CSmallBalloon_off_##f[((int)&((CSmallBalloon*)0)->f == (o)) ? 1 : -1]
CSMALLBALLOON_OFF(effectPosition, 0xbc);
#undef CSMALLBALLOON_OFF

#endif  // CLASSES_CSMALLBALLOON_H
