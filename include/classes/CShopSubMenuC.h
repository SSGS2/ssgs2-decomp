#ifndef CLASSES_CSHOPSUBMENUC_H
#define CLASSES_CSHOPSUBMENUC_H

#include "game/types.h"

class CShopSubMenuC {
public:
    void* vtbl;             // 0x0: original vtable pointer
    void* layout;           // 0x4
    char pad_008[0x10];     // 0x8
    void* clubHouseMessage; // 0x18

    int IsLoaded();
};

#define CSHOPSUBMENUC_OFF(f, o) typedef char CShopSubMenuC_off_##f[((int)&((CShopSubMenuC*)0)->f == (o)) ? 1 : -1]
CSHOPSUBMENUC_OFF(layout, 0x4);
CSHOPSUBMENUC_OFF(clubHouseMessage, 0x18);
#undef CSHOPSUBMENUC_OFF

#endif // CLASSES_CSHOPSUBMENUC_H
