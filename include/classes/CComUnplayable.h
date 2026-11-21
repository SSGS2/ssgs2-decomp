// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCOMUNPLAYABLE_H
#define CLASSES_CCOMUNPLAYABLE_H

#include "game/types.h"

class CComUnplayable {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int state;                          // 0x4: 1 use in CComUnplayable_Ctor
    float dropPosition[3]; // 0x8: CComUnplayable_UpdateDropSequence passes this to the drop-position query and request 0x11
    unsigned int startTimeMs; // 0x14: CComUnplayable_UpdateDropSequence waits 0xBB8 ms after OS_GetTimeMilliseconds
    int shotPositionThinker;                         // 0x18: 1 use in CComUnplayable_Ctor
    int rival;                         // 0x1c: 1 use in CComUnplayable_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCOMUNPLAYABLE_OFF(f, o) typedef char CComUnplayable_off_##f[((int)&((CComUnplayable*)0)->f == (o)) ? 1 : -1]
CCOMUNPLAYABLE_OFF(state, 0x4);
CCOMUNPLAYABLE_OFF(dropPosition, 0x8);
CCOMUNPLAYABLE_OFF(startTimeMs, 0x14);
CCOMUNPLAYABLE_OFF(shotPositionThinker, 0x18);
CCOMUNPLAYABLE_OFF(rival, 0x1c);
#undef CCOMUNPLAYABLE_OFF

#endif  // CLASSES_CCOMUNPLAYABLE_H
