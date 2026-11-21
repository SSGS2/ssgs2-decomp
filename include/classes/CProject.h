// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPROJECT_H
#define CLASSES_CPROJECT_H

#include "game/types.h"

class CProject {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x28];                 // 0x4
    unsigned char projectNameTagByte;               // 0x2c: 1 use in CProject_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x10, 0x88
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPROJECT_OFF(f, o) typedef char CProject_off_##f[((int)&((CProject*)0)->f == (o)) ? 1 : -1]
CPROJECT_OFF(projectNameTagByte, 0x2c);
#undef CPROJECT_OFF

#endif  // CLASSES_CPROJECT_H
