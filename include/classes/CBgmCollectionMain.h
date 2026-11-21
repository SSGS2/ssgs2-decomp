// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBGMCOLLECTIONMAIN_H
#define CLASSES_CBGMCOLLECTIONMAIN_H

#include "game/types.h"

class CBgmCollectionMain {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* taskMainVtable;                       // 0x18: 2 uses in CBgmCollectionMain_Ctor, CBgmCollectionMain_dtor
    void* collection;                       // 0x1c: 10 uses in CBgmCollectionMain_OnLoad, CBgmCollectionMain_OnDestroy, CBgmCollectionMain_dtor...
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBGMCOLLECTIONMAIN_OFF(f, o) typedef char CBgmCollectionMain_off_##f[((int)&((CBgmCollectionMain*)0)->f == (o)) ? 1 : -1]
CBGMCOLLECTIONMAIN_OFF(taskMainVtable, 0x18);
CBGMCOLLECTIONMAIN_OFF(collection, 0x1c);
#undef CBGMCOLLECTIONMAIN_OFF

#endif  // CLASSES_CBGMCOLLECTIONMAIN_H
