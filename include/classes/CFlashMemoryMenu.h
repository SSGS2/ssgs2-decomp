// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFLASHMEMORYMENU_H
#define CLASSES_CFLASHMEMORYMENU_H

#include "game/types.h"

class CFlashMemoryMenu {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* fileIo;                       // 0x18: 1 use in CFlashMemoryMenu_QueryResourcePredicate8
    char pad_01C[0x24];                 // 0x1c
    void* banner;                       // 0x40: 2 uses in CFlashMemoryMenu_dtor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFLASHMEMORYMENU_OFF(f, o) typedef char CFlashMemoryMenu_off_##f[((int)&((CFlashMemoryMenu*)0)->f == (o)) ? 1 : -1]
CFLASHMEMORYMENU_OFF(fileIo, 0x18);
CFLASHMEMORYMENU_OFF(banner, 0x40);
#undef CFLASHMEMORYMENU_OFF

#endif  // CLASSES_CFLASHMEMORYMENU_H
