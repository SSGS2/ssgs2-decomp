// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CNOWRELOADING_H
#define CLASSES_CNOWRELOADING_H

#include "game/types.h"

class CNowReloading {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* vtable;                        // 0x4: 2 uses in CNowReloading_dtor, CNowReloading_Init
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CNOWRELOADING_OFF(f, o) typedef char CNowReloading_off_##f[((int)&((CNowReloading*)0)->f == (o)) ? 1 : -1]
CNOWRELOADING_OFF(vtable, 0x4);
#undef CNOWRELOADING_OFF

#endif  // CLASSES_CNOWRELOADING_H
