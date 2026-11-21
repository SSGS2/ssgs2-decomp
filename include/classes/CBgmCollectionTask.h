// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBGMCOLLECTIONTASK_H
#define CLASSES_CBGMCOLLECTIONTASK_H

#include "game/types.h"

class CBgmCollectionTask {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* interfaceVtable;                        // 0x4: CBgmCollectionTask_Ctor installs vtable + 0x10
    void* secondaryInterfaceVtable;                        // 0x8: CBgmCollectionTask_Ctor installs vtable + 0x1c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBGMCOLLECTIONTASK_OFF(f, o) typedef char CBgmCollectionTask_off_##f[((int)&((CBgmCollectionTask*)0)->f == (o)) ? 1 : -1]
CBGMCOLLECTIONTASK_OFF(interfaceVtable, 0x4);
CBGMCOLLECTIONTASK_OFF(secondaryInterfaceVtable, 0x8);
#undef CBGMCOLLECTIONTASK_OFF

#endif  // CLASSES_CBGMCOLLECTIONTASK_H
