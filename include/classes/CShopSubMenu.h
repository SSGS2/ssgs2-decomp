#ifndef CLASSES_CSHOPSUBMENU_H
#define CLASSES_CSHOPSUBMENU_H

#include "game/types.h"

class CShopSubMenu {
public:
    void* vtbl;        // 0x0: original vtable pointer
    char pad_004[0x4]; // 0x4
    void* layout;      // 0x8

    int IsLoaded();
};

#define CSHOPSUBMENU_OFF(f, o) typedef char CShopSubMenu_off_##f[((int)&((CShopSubMenu*)0)->f == (o)) ? 1 : -1]
CSHOPSUBMENU_OFF(layout, 0x8);
#undef CSHOPSUBMENU_OFF

#endif // CLASSES_CSHOPSUBMENU_H
