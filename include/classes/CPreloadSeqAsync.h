// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPRELOADSEQASYNC_H
#define CLASSES_CPRELOADSEQASYNC_H

#include "game/types.h"

class CPreloadSeqAsync {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x10];                 // 0x4
    int loadState;                         // 0x14: 1 use in Fn8026FEF0Obj
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPRELOADSEQASYNC_OFF(f, o) typedef char CPreloadSeqAsync_off_##f[((int)&((CPreloadSeqAsync*)0)->f == (o)) ? 1 : -1]
CPRELOADSEQASYNC_OFF(loadState, 0x14);
#undef CPRELOADSEQASYNC_OFF

#endif  // CLASSES_CPRELOADSEQASYNC_H
