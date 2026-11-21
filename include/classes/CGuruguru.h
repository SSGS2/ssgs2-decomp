// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CGURUGURU_H
#define CLASSES_CGURUGURU_H

#include "game/types.h"

class CGuruguru {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* unk_4;                        // 0x4: 2 uses in fn_8030F460_old
    void* unk_8;                        // 0x8: 3 uses in fn_8030F460_old
    void* unk_C;                        // 0xc: 3 uses in fn_8030F460_old
    void* unk_10;                       // 0x10: 1 use in fn_8030F460_old
    char pad_014[0x8];                  // 0x14
    void* unk_1C;                       // 0x1c: 2 uses in fn_8030F460_old
    char pad_020[0x86c];                // 0x20
    void* fontResource;                      // 0x88c: 2 uses in fn_8030F460_old

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x888, 0x890
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CGURUGURU_OFF(f, o) typedef char CGuruguru_off_##f[((int)&((CGuruguru*)0)->f == (o)) ? 1 : -1]
CGURUGURU_OFF(unk_4, 0x4);
CGURUGURU_OFF(unk_8, 0x8);
CGURUGURU_OFF(unk_C, 0xc);
CGURUGURU_OFF(unk_10, 0x10);
CGURUGURU_OFF(unk_1C, 0x1c);
CGURUGURU_OFF(fontResource, 0x88c);
#undef CGURUGURU_OFF

#endif  // CLASSES_CGURUGURU_H
