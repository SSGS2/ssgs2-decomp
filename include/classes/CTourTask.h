// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CTOURTASK_H
#define CLASSES_CTOURTASK_H

#include "game/types.h"

class CTourTask {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* secondaryVtable;                        // 0x4: 1 use in CTourTask_Ctor
    void* tertiaryVtable;                        // 0x8: 1 use in CTourTask_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CTOURTASK_OFF(f, o) typedef char CTourTask_off_##f[((int)&((CTourTask*)0)->f == (o)) ? 1 : -1]
CTOURTASK_OFF(secondaryVtable, 0x4);
CTOURTASK_OFF(tertiaryVtable, 0x8);
#undef CTOURTASK_OFF

#endif  // CLASSES_CTOURTASK_H
