// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CROUNDDOLTASK_H
#define CLASSES_CROUNDDOLTASK_H

#include "game/types.h"

class CRoundDolTask {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    unsigned int secondaryVtable;                 // 0x4: 1 use in CRoundDolTask_Ctor
    unsigned int tertiaryVtable;                 // 0x8: 1 use in CRoundDolTask_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CROUNDDOLTASK_OFF(f, o) typedef char CRoundDolTask_off_##f[((int)&((CRoundDolTask*)0)->f == (o)) ? 1 : -1]
CROUNDDOLTASK_OFF(secondaryVtable, 0x4);
CROUNDDOLTASK_OFF(tertiaryVtable, 0x8);
#undef CROUNDDOLTASK_OFF

#endif  // CLASSES_CROUNDDOLTASK_H
