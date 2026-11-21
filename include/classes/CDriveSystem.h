// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CDRIVESYSTEM_H
#define CLASSES_CDRIVESYSTEM_H

#include "game/types.h"

class CDriveSystem {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    int unk_8;                          // 0x8: 1 use in CDriveSystem_Ctor
    char pad_00C[0xc];                  // 0xc
    void* miniGameAim;                       // 0x18: 13 uses in CDriveSystem_OnPreLoadInit, CDriveSystem_OnDestroy, CDriveSystem_Ctor...
    void* dialog;                       // 0x1c: 13 uses in CDriveSystem_HandleMsg, CDriveSystem_OnDestroy, CDriveSystem_OnProcess...
    int dialogRequested;                         // 0x20: 5 uses in CDriveSystem_HandleMsg, CDriveSystem_Ctor, CDriveSystem_OnInit
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CDRIVESYSTEM_OFF(f, o) typedef char CDriveSystem_off_##f[((int)&((CDriveSystem*)0)->f == (o)) ? 1 : -1]
CDRIVESYSTEM_OFF(unk_8, 0x8);
CDRIVESYSTEM_OFF(miniGameAim, 0x18);
CDRIVESYSTEM_OFF(dialog, 0x1c);
CDRIVESYSTEM_OFF(dialogRequested, 0x20);
#undef CDRIVESYSTEM_OFF

#endif  // CLASSES_CDRIVESYSTEM_H
