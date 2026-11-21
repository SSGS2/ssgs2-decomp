// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPUTTLINE_H
#define CLASSES_CPUTTLINE_H

#include "game/types.h"

class CPuttLine {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    void* textureId;                        // 0x8: 1 use in CPuttLine_Display
    char pad_00C[0x10];                 // 0xc
    unsigned char active;               // 0x1c: 3 uses in CPuttLine_Process, CPuttLine_Init
    char pad_01D[0xf7];                 // 0x1d
    void* polygon;                      // 0x114: 7 uses in CPuttLine_Display, CPuttLine_dtor
    void* segments;                      // 0x118: 22 uses in CPuttLine_Init, CPuttLine_Process, CPuttLine_Display...
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPUTTLINE_OFF(f, o) typedef char CPuttLine_off_##f[((int)&((CPuttLine*)0)->f == (o)) ? 1 : -1]
CPUTTLINE_OFF(textureId, 0x8);
CPUTTLINE_OFF(active, 0x1c);
CPUTTLINE_OFF(polygon, 0x114);
CPUTTLINE_OFF(segments, 0x118);
#undef CPUTTLINE_OFF

#endif  // CLASSES_CPUTTLINE_H
