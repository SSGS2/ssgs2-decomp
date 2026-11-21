// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFONTNUMDRAW_H
#define CLASSES_CFONTNUMDRAW_H

#include "game/types.h"

class CFontNumDraw {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    void* unk_8;                        // 0x8: 1 use in CFontNumDraw_FONT_NUM_DRAW_INFO_dtor
    char pad_00C[0x4];                  // 0xc
    unsigned int unk_10;                // 0x10: 1 use in CFontNumDraw_FONT_NUM_DRAW_INFO_dtor
    char pad_014[0x4];                  // 0x14
    void* unk_18;                       // 0x18: 2 uses in CFontNumDraw_FONT_NUM_DRAW_INFO_dtor, CFontNumDraw_dtor
    void* unk_1C;                       // 0x1c: 2 uses in CFontNumDraw_FONT_NUM_DRAW_INFO_dtor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x18, 0x1c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFONTNUMDRAW_OFF(f, o) typedef char CFontNumDraw_off_##f[((int)&((CFontNumDraw*)0)->f == (o)) ? 1 : -1]
CFONTNUMDRAW_OFF(unk_8, 0x8);
CFONTNUMDRAW_OFF(unk_10, 0x10);
CFONTNUMDRAW_OFF(unk_18, 0x18);
CFONTNUMDRAW_OFF(unk_1C, 0x1c);
#undef CFONTNUMDRAW_OFF

#endif  // CLASSES_CFONTNUMDRAW_H
