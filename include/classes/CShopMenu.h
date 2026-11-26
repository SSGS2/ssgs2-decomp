#ifndef CLASSES_CSHOPMENU_H
#define CLASSES_CSHOPMENU_H

#include "game/types.h"

class CShopMenu {
public:
    void* vtbl;   // 0x0: original vtable pointer
    void* layout; // 0x4

    int IsLayoutLoaded();
};

#define CSHOPMENU_OFF(f, o) typedef char CShopMenu_off_##f[((int)&((CShopMenu*)0)->f == (o)) ? 1 : -1]
CSHOPMENU_OFF(layout, 0x4);
#undef CSHOPMENU_OFF

#endif // CLASSES_CSHOPMENU_H
