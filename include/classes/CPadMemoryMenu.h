// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPADMEMORYMENU_H
#define CLASSES_CPADMEMORYMENU_H

#include "game/types.h"

class CPadMemoryMenu {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* fileIoAccess;                       // 0x18: 3 uses in CPadMemoryMenu_CreateWriteFileIo
    char pad_01C[0xc];                  // 0x1c
    void* saveForm;                       // 0x28: 6 uses in CPadMemoryMenu_CreateWriteFileIo
    char pad_02C[0x14];                 // 0x2c
    void* controllerIndex;                       // 0x40: 2 uses in CPadMemoryMenu_Ctor, CPadMemoryMenu_CreateWriteFileIo
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPADMEMORYMENU_OFF(f, o) typedef char CPadMemoryMenu_off_##f[((int)&((CPadMemoryMenu*)0)->f == (o)) ? 1 : -1]
CPADMEMORYMENU_OFF(fileIoAccess, 0x18);
CPADMEMORYMENU_OFF(saveForm, 0x28);
CPADMEMORYMENU_OFF(controllerIndex, 0x40);
#undef CPADMEMORYMENU_OFF

#endif  // CLASSES_CPADMEMORYMENU_H
