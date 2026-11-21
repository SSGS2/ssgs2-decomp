// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CVSMAIN_H
#define CLASSES_CVSMAIN_H

#include "game/types.h"

class CVSMain {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* taskMainVtable;                       // 0x18: 1 use in CVSMain_dtor
    char pad_01C[0x18];                 // 0x1c
    void* primaryItems;                       // 0x34: 2 uses in CVSMain_Ctor, CVSMain_dtor
    int primaryItemCount;                         // 0x38: 3 uses in CVSMain_dtor, CVSMain_Ctor
    char pad_03C[0x4];                  // 0x3c
    void* secondaryItems;                       // 0x40: 2 uses in CVSMain_Ctor, CVSMain_dtor
    int secondaryItemCount;                         // 0x44: 3 uses in CVSMain_dtor, CVSMain_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CVSMAIN_OFF(f, o) typedef char CVSMain_off_##f[((int)&((CVSMain*)0)->f == (o)) ? 1 : -1]
CVSMAIN_OFF(taskMainVtable, 0x18);
CVSMAIN_OFF(primaryItems, 0x34);
CVSMAIN_OFF(primaryItemCount, 0x38);
CVSMAIN_OFF(secondaryItems, 0x40);
CVSMAIN_OFF(secondaryItemCount, 0x44);
#undef CVSMAIN_OFF

#endif  // CLASSES_CVSMAIN_H
