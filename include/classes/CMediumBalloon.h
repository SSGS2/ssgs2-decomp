// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CMEDIUMBALLOON_H
#define CLASSES_CMEDIUMBALLOON_H

#include "game/types.h"

class CMediumBalloon {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x20];                 // 0x4
    void* items;                       // 0x24: 1 use in CMediumBalloon_dtor
    int itemCount;                         // 0x28: 3 uses in CMediumBalloon_dtor
    char pad_02C[0x74];                 // 0x2c
    int sizeClass;                         // 0xa0: 1 use in CMediumBalloon_ctor
    char pad_0A4[0x18];                 // 0xa4
    void* attachedEffectPosition;                       // 0xbc: 1 use in CMediumBalloon_StartExplosionEffect
    char pad_0C0[0x10];                 // 0xc0
    void* balloonModel;                       // 0xd0: 1 use in CMediumBalloon_dtor
    unsigned short baseScale;              // 0xd4: 2 uses in CMediumBalloon_StartExplosionEffect

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x7c, 0xa4
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CMEDIUMBALLOON_OFF(f, o) typedef char CMediumBalloon_off_##f[((int)&((CMediumBalloon*)0)->f == (o)) ? 1 : -1]
CMEDIUMBALLOON_OFF(items, 0x24);
CMEDIUMBALLOON_OFF(itemCount, 0x28);
CMEDIUMBALLOON_OFF(sizeClass, 0xa0);
CMEDIUMBALLOON_OFF(attachedEffectPosition, 0xbc);
CMEDIUMBALLOON_OFF(balloonModel, 0xd0);
CMEDIUMBALLOON_OFF(baseScale, 0xd4);
#undef CMEDIUMBALLOON_OFF

#endif  // CLASSES_CMEDIUMBALLOON_H
