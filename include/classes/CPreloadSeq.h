// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPRELOADSEQ_H
#define CLASSES_CPRELOADSEQ_H

#include "game/types.h"

class CPreloadSeq {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    void* scriptStream;                       // 0x10: 2 uses in CPreloadSeq_dtor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPRELOADSEQ_OFF(f, o) typedef char CPreloadSeq_off_##f[((int)&((CPreloadSeq*)0)->f == (o)) ? 1 : -1]
CPRELOADSEQ_OFF(scriptStream, 0x10);
#undef CPRELOADSEQ_OFF

#endif  // CLASSES_CPRELOADSEQ_H
