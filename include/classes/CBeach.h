// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBEACH_H
#define CLASSES_CBEACH_H

#include "game/types.h"

class CBeach {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* stripRows;                        // 0x4: 2 uses in CBeach_Draw
    unsigned int stripRowCount;                 // 0x8: 7 uses in CBeach_Draw
    char pad_00C[0x4];                  // 0xc
    unsigned int bitmap;                // 0x10: loaded bitmap resolved to texture and texture state
    char pad_014[0x18];                 // 0x14
    void* stripParameters;                       // 0x2c: 6 uses in CBeach_Draw
    unsigned int stripCount;                // 0x30: 3 uses in CBeach_Draw
    char pad_034[0x4];                  // 0x34
    void* vertexBuffers;                       // 0x38: 3 uses in CBeach_Draw
    char pad_03C[0x14];                 // 0x3c
    float maxRowHeightDelta;                       // 0x50: 2 uses in CBeach_Draw

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x10, 0x44
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBEACH_OFF(f, o) typedef char CBeach_off_##f[((int)&((CBeach*)0)->f == (o)) ? 1 : -1]
CBEACH_OFF(stripRows, 0x4);
CBEACH_OFF(stripRowCount, 0x8);
CBEACH_OFF(bitmap, 0x10);
CBEACH_OFF(stripParameters, 0x2c);
CBEACH_OFF(stripCount, 0x30);
CBEACH_OFF(vertexBuffers, 0x38);
CBEACH_OFF(maxRowHeightDelta, 0x50);
#undef CBEACH_OFF

#endif  // CLASSES_CBEACH_H
