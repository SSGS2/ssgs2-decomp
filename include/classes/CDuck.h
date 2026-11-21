// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CDUCK_H
#define CLASSES_CDUCK_H

#include "game/types.h"

class CDuck {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* secondaryVtable;                        // 0x4: 1 use in CDuck_Dtor
    char pad_008[0x1c];                 // 0x8
    void* puppet;                       // 0x24: 1 use in CDuck_Dtor
    char pad_028[0xf8];                 // 0x28
    void* instanceMatrices;                      // 0x120: 2 uses in CDuck_Dtor
    int linkCount;                     // 0x124: 2 uses in CDuck_Dtor
    char pad_128[0x4];                  // 0x128
    void* listNext;                    // 0x12c: 2 uses in CDuck_Dtor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0xc, 0x18, 0xb8, 0x124, 0x128
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CDUCK_OFF(f, o) typedef char CDuck_off_##f[((int)&((CDuck*)0)->f == (o)) ? 1 : -1]
CDUCK_OFF(secondaryVtable, 0x4);
CDUCK_OFF(puppet, 0x24);
CDUCK_OFF(instanceMatrices, 0x120);
CDUCK_OFF(linkCount, 0x124);
CDUCK_OFF(listNext, 0x12c);
#undef CDUCK_OFF

#endif  // CLASSES_CDUCK_H
