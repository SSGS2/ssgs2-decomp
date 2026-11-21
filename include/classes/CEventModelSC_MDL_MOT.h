// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CEVENTMODELSC_MDL_MOT_H
#define CLASSES_CEVENTMODELSC_MDL_MOT_H

#include "game/types.h"

class CEventModelSC_MDL_MOT {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* modelSlot;                        // 0x4: 3 uses in CEventModelSC_MDL_MOT_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CEVENTMODELSC_MDL_MOT_OFF(f, o) typedef char CEventModelSC_MDL_MOT_off_##f[((int)&((CEventModelSC_MDL_MOT*)0)->f == (o)) ? 1 : -1]
CEVENTMODELSC_MDL_MOT_OFF(modelSlot, 0x4);
#undef CEVENTMODELSC_MDL_MOT_OFF

#endif  // CLASSES_CEVENTMODELSC_MDL_MOT_H
