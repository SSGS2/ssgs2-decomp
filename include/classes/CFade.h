// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFADE_H
#define CLASSES_CFADE_H

#include "game/types.h"

class CFade {
public:
    void* direction;                         // 0x0: non-polymorphic alpha-ramp direction/state word
    char pad_004[0x1];                  // 0x4
    unsigned char step;                // 0x5: 1 use in CFade_SetStep
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFADE_OFF(f, o) typedef char CFade_off_##f[((int)&((CFade*)0)->f == (o)) ? 1 : -1]
CFADE_OFF(step, 0x5);
#undef CFADE_OFF

#endif  // CLASSES_CFADE_H
