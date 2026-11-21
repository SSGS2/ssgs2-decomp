// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CTASK_H
#define CLASSES_CTASK_H

#include "game/types.h"

class CTask {
public:
    char pad_000[4]; // 0x0: unmodelled bytes
    char pad_004[0xc];                  // 0x4
    unsigned char transitionPending;    // 0x10: CTask_RequestTransition original object access
    char pad_011[0x3];                  // 0x11
    void* transitionArg0;               // 0x14: CTask_RequestTransition original object access
    void* transitionArg1;               // 0x18: CTask_RequestTransition original object access
    void* child;                        // 0x1c: Object_GetChild original object access
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CTASK_OFF(f, o) typedef char CTask_off_##f[((int)&((CTask*)0)->f == (o)) ? 1 : -1]
CTASK_OFF(transitionPending, 0x10);
CTASK_OFF(transitionArg0, 0x14);
CTASK_OFF(transitionArg1, 0x18);
CTASK_OFF(child, 0x1c);
#undef CTASK_OFF

#endif  // CLASSES_CTASK_H
