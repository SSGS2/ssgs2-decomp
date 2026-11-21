// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CEVENTMODELSC_H
#define CLASSES_CEVENTMODELSC_H

#include "game/types.h"

class CEventModelSC {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    unsigned int unk_4;                 // 0x4: 3 uses in CEventModelSC_MDL_MOVE_UpdatePosition, CEventModelSC_MDL_MOT_IsMotionQueueIdle
    void* unk_8;                        // 0x8: 2 uses in CEventModelSC_MDL_MOVE_UpdatePosition

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0xc, 0x18
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CEVENTMODELSC_OFF(f, o) typedef char CEventModelSC_off_##f[((int)&((CEventModelSC*)0)->f == (o)) ? 1 : -1]
CEVENTMODELSC_OFF(unk_4, 0x4);
CEVENTMODELSC_OFF(unk_8, 0x8);
#undef CEVENTMODELSC_OFF

#endif  // CLASSES_CEVENTMODELSC_H
