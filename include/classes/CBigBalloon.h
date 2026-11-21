// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBIGBALLOON_H
#define CLASSES_CBIGBALLOON_H

#include "game/types.h"

class CBigBalloon {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x24];                 // 0x4
    unsigned int scaleCount;                // 0x28: 3 uses in CBigBalloon_ResetScaleSequence, CBigBalloon_dtor
    char pad_02C[0x44];                 // 0x2c
    float scale;                       // 0x70: 1 use in CBigBalloon_ResetScaleSequence
    char pad_074[0x2c];                 // 0x74
    int sizeClass;                         // 0xa0: 1 use in CBigBalloon_ctor
    char pad_0A4[0x2c];                 // 0xa4
    void* model;                       // 0xd0: 1 use in CBigBalloon_dtor
    unsigned short baseScale;              // 0xd4: 2 uses in CBigBalloon_StartExplosionEffect
    char pad_0D6[0x16];                 // 0xd6
    unsigned short targetScale;              // 0xec: 1 use in CBigBalloon_ResetScaleSequence
    unsigned short scaleDelta;              // 0xee: 1 use in CBigBalloon_ResetScaleSequence

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x24, 0x7c, 0xa4
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBIGBALLOON_OFF(f, o) typedef char CBigBalloon_off_##f[((int)&((CBigBalloon*)0)->f == (o)) ? 1 : -1]
CBIGBALLOON_OFF(scaleCount, 0x28);
CBIGBALLOON_OFF(scale, 0x70);
CBIGBALLOON_OFF(sizeClass, 0xa0);
CBIGBALLOON_OFF(model, 0xd0);
CBIGBALLOON_OFF(baseScale, 0xd4);
CBIGBALLOON_OFF(targetScale, 0xec);
CBIGBALLOON_OFF(scaleDelta, 0xee);
#undef CBIGBALLOON_OFF

#endif  // CLASSES_CBIGBALLOON_H
