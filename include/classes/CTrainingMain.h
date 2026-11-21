// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CTRAININGMAIN_H
#define CLASSES_CTRAININGMAIN_H

#include "game/types.h"

class CTrainingMain {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* taskMainVtable;                       // 0x18: 1 use in CTrainingMain_dtor
    void* courseSelect;                       // 0x1c: 7 uses in CTrainingMain_dtor, CTrainingMain_OnDisplay, CTrainingMain_OnProcess...
    void* backgroundEntryId;                       // 0x20: 3 uses in CTrainingMain_OnDisplay, CTrainingMain_OnProcess
    void* captionEntryId;                       // 0x24: 3 uses in CTrainingMain_OnDisplay, CTrainingMain_OnProcess
    int scriptsStarted;                         // 0x28: 2 uses in CTrainingMain_OnProcess

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x20, 0x24
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CTRAININGMAIN_OFF(f, o) typedef char CTrainingMain_off_##f[((int)&((CTrainingMain*)0)->f == (o)) ? 1 : -1]
CTRAININGMAIN_OFF(taskMainVtable, 0x18);
CTRAININGMAIN_OFF(courseSelect, 0x1c);
CTRAININGMAIN_OFF(backgroundEntryId, 0x20);
CTRAININGMAIN_OFF(captionEntryId, 0x24);
CTRAININGMAIN_OFF(scriptsStarted, 0x28);
#undef CTRAININGMAIN_OFF

#endif  // CLASSES_CTRAININGMAIN_H
