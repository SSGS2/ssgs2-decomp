// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHOPSUBMENUC_H
#define CLASSES_CSHOPSUBMENUC_H

#include "game/types.h"

class CShopSubMenuC {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* layout;                        // 0x4: 1 use in CShopSubMenuC_IsLoaded
    char pad_008[0x10];                 // 0x8
    void* clubHouseMessage;                       // 0x18: 1 use in CShopSubMenuC_IsLoaded
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHOPSUBMENUC_OFF(f, o) typedef char CShopSubMenuC_off_##f[((int)&((CShopSubMenuC*)0)->f == (o)) ? 1 : -1]
CSHOPSUBMENUC_OFF(layout, 0x4);
CSHOPSUBMENUC_OFF(clubHouseMessage, 0x18);
#undef CSHOPSUBMENUC_OFF

#endif  // CLASSES_CSHOPSUBMENUC_H
