// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPRELOADPARTSASYNC_H
#define CLASSES_CPRELOADPARTSASYNC_H

#include "game/types.h"

class CPreloadPartsAsync {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    int state;                          // 0x10: 1 use in CPreloadPartsAsync_IsComplete

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x10
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPRELOADPARTSASYNC_OFF(f, o) typedef char CPreloadPartsAsync_off_##f[((int)&((CPreloadPartsAsync*)0)->f == (o)) ? 1 : -1]
CPRELOADPARTSASYNC_OFF(state, 0x10);
#undef CPRELOADPARTSASYNC_OFF

#endif  // CLASSES_CPRELOADPARTSASYNC_H
