// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPRELOADFILEASYNC_H
#define CLASSES_CPRELOADFILEASYNC_H

#include "game/types.h"

class CPreloadFileAsync {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x10];                 // 0x4
    int state;                         // 0x14: 1 use in CPreloadFileAsync_IsComplete

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x14
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPRELOADFILEASYNC_OFF(f, o) typedef char CPreloadFileAsync_off_##f[((int)&((CPreloadFileAsync*)0)->f == (o)) ? 1 : -1]
CPRELOADFILEASYNC_OFF(state, 0x14);
#undef CPRELOADFILEASYNC_OFF

#endif  // CLASSES_CPRELOADFILEASYNC_H
