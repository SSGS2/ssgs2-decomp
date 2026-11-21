// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBALLTRACE_H
#define CLASSES_CBALLTRACE_H

#include "game/types.h"

class CBallTrace {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x110];                // 0x4
    void* quadRenderer;                      // 0x114: 2 uses in CBallTrace_Destroy
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBALLTRACE_OFF(f, o) typedef char CBallTrace_off_##f[((int)&((CBallTrace*)0)->f == (o)) ? 1 : -1]
CBALLTRACE_OFF(quadRenderer, 0x114);
#undef CBALLTRACE_OFF

#endif  // CLASSES_CBALLTRACE_H
