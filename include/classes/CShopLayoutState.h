#ifndef CLASSES_CSHOPLAYOUTSTATE_H
#define CLASSES_CSHOPLAYOUTSTATE_H

#include "game/types.h"

class CShopLayoutState {
public:
    void* vtbl;   // 0x0: original vtable pointer
    void* layout; // 0x4

    int IsLoaded();
};

#define CSHOPLAYOUTSTATE_OFF(f, o) \
    typedef char CShopLayoutState_off_##f[((int)&((CShopLayoutState*)0)->f == (o)) ? 1 : -1]
CSHOPLAYOUTSTATE_OFF(layout, 0x4);
#undef CSHOPLAYOUTSTATE_OFF

#endif // CLASSES_CSHOPLAYOUTSTATE_H
