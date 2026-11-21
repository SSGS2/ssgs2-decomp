// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSKIDMARK_H
#define CLASSES_CSKIDMARK_H

#include "game/types.h"

class CSkidMark {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    void* textureResource;                        // 0x8: 3 uses in CSkidMark_dtor, CSkidMark_Draw
    char pad_00C[0x10];                 // 0xc
    unsigned char enabled;               // 0x1c: 1 use in CSkidMark_Draw
    char pad_01D[0xf7];                 // 0x1d
    void* quadRenderer;                      // 0x114: 6 uses in CSkidMark_Draw, CSkidMark_dtor
    void* trailPoints;                      // 0x118: 5 uses in CSkidMark_Draw, CSkidMark_dtor
    int pointCapacity;                 // 0x11c: 2 uses in CSkidMark_Draw, CSkidMark_dtor
    int pointCount;                    // 0x120: 3 uses in CSkidMark_Draw, CSkidMark_dtor
    char pad_124[0x4];                  // 0x124
    int endFrame;                      // 0x128: 1 use in CSkidMark_Draw
    int lastProcessedFrame;            // 0x12c: 1 use in CSkidMark_Draw
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSKIDMARK_OFF(f, o) typedef char CSkidMark_off_##f[((int)&((CSkidMark*)0)->f == (o)) ? 1 : -1]
CSKIDMARK_OFF(textureResource, 0x8);
CSKIDMARK_OFF(enabled, 0x1c);
CSKIDMARK_OFF(quadRenderer, 0x114);
CSKIDMARK_OFF(trailPoints, 0x118);
CSKIDMARK_OFF(pointCapacity, 0x11c);
CSKIDMARK_OFF(pointCount, 0x120);
CSKIDMARK_OFF(endFrame, 0x128);
CSKIDMARK_OFF(lastProcessedFrame, 0x12c);
#undef CSKIDMARK_OFF

#endif  // CLASSES_CSKIDMARK_H
