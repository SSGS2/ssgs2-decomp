// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHOPMENU_H
#define CLASSES_CSHOPMENU_H

#include "game/types.h"

class CShopMenu {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* layout;                       // 0x4: 1 use in CShopMenu_IsLayoutLoaded
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHOPMENU_OFF(f, o) typedef char CShopMenu_off_##f[((int)&((CShopMenu*)0)->f == (o)) ? 1 : -1]
CSHOPMENU_OFF(layout, 0x4);
#undef CSHOPMENU_OFF

#endif  // CLASSES_CSHOPMENU_H
