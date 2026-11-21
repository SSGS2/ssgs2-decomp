// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CDARTSPLATES_H
#define CLASSES_CDARTSPLATES_H

#include "game/types.h"

class CDartsPlates {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    float heading;                       // 0x10: 1 use in CDartsPlates_Draw
    void* plates;                       // 0x14: 8 uses in CDartsPlates_dtor, CDartsPlates_Draw
    unsigned int plateCount;                // 0x18: 7 uses in CDartsPlates_dtor, CDartsPlates_Draw
    unsigned int plateCapacity;         // 0x1c: OnPreLoadInit zeroes the trailing word of plates/count/capacity vector
    void* texture;                       // 0x20: 4 uses in CDartsPlates_dtor, CDartsPlates_Draw
    void* balloonModel;                       // 0x24: 3 uses in CDartsPlates_dtor
    int renderSuppressed;                         // 0x28: 1 use in CDartsPlates_Draw

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x14, 0x20
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CDARTSPLATES_OFF(f, o) typedef char CDartsPlates_off_##f[((int)&((CDartsPlates*)0)->f == (o)) ? 1 : -1]
CDARTSPLATES_OFF(heading, 0x10);
CDARTSPLATES_OFF(plates, 0x14);
CDARTSPLATES_OFF(plateCount, 0x18);
CDARTSPLATES_OFF(plateCapacity, 0x1c);
CDARTSPLATES_OFF(texture, 0x20);
CDARTSPLATES_OFF(balloonModel, 0x24);
CDARTSPLATES_OFF(renderSuppressed, 0x28);
#undef CDARTSPLATES_OFF

#endif  // CLASSES_CDARTSPLATES_H
