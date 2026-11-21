// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSINGLEMAINREV_H
#define CLASSES_CSINGLEMAINREV_H

#include "game/types.h"

class CSingleMainRev {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* secondaryVtable;                       // 0x18: 1 use in CSingleMainRev_Ctor
    int state;                         // 0x1c: 6 uses in CSingleMainRev_OnProcess, CSingleMainRev_HandleMsg, CSingleMainRev_Ctor
    void* childObject;                       // 0x20: 19 uses in CSingleMainRev_OnProcess, CSingleMainRev_HandleMsg, CSingleMainRev_Ctor...
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSINGLEMAINREV_OFF(f, o) typedef char CSingleMainRev_off_##f[((int)&((CSingleMainRev*)0)->f == (o)) ? 1 : -1]
CSINGLEMAINREV_OFF(secondaryVtable, 0x18);
CSINGLEMAINREV_OFF(state, 0x1c);
CSINGLEMAINREV_OFF(childObject, 0x20);
#undef CSINGLEMAINREV_OFF

#endif  // CLASSES_CSINGLEMAINREV_H
