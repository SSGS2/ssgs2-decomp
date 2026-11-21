// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CALLCOURSE_H
#define CLASSES_CALLCOURSE_H

#include "game/types.h"

class CAllCourse {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    void* resourceManager;                       // 0x10: 1 use in CAllCourse_LoadRoundHoleRecords
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CALLCOURSE_OFF(f, o) typedef char CAllCourse_off_##f[((int)&((CAllCourse*)0)->f == (o)) ? 1 : -1]
CALLCOURSE_OFF(resourceManager, 0x10);
#undef CALLCOURSE_OFF

#endif  // CLASSES_CALLCOURSE_H
