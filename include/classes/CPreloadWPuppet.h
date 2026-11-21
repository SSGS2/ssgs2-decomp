// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPRELOADWPUPPET_H
#define CLASSES_CPRELOADWPUPPET_H

#include "game/types.h"

class CPreloadWPuppet {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    void* file;                       // 0x10: 3 uses in CPreloadWPuppet_dtor, CPreloadWPuppet_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPRELOADWPUPPET_OFF(f, o) typedef char CPreloadWPuppet_off_##f[((int)&((CPreloadWPuppet*)0)->f == (o)) ? 1 : -1]
CPRELOADWPUPPET_OFF(file, 0x10);
#undef CPRELOADWPUPPET_OFF

#endif  // CLASSES_CPRELOADWPUPPET_H
