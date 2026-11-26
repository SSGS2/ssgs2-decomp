// CPreloadParent: the shared base of the resource preloaders.
//
// It owns a growing list of child preloaders. Destroying it deletes every child
// in the list, clears the list, and frees the list's storage.
#ifndef CLASSES_CPRELOADPARENT_H
#define CLASSES_CPRELOADPARENT_H

#include "game/types.h"

class CPreloadParent {
public:
    CPreloadParent();
    ~CPreloadParent();

    void* vtbl;                // 0x00
    void** ownedItems;         // 0x04  child preloaders
    unsigned int itemCount;    // 0x08
    unsigned int itemCapacity; // 0x0C
};

#define CPRELOADPARENT_OFF(f, o) typedef char CPreloadParent_off_##f[((int)&((CPreloadParent*)0)->f == (o)) ? 1 : -1]
CPRELOADPARENT_OFF(ownedItems, 0x04);
CPRELOADPARENT_OFF(itemCount, 0x08);
CPRELOADPARENT_OFF(itemCapacity, 0x0C);
#undef CPRELOADPARENT_OFF
typedef char CPreloadParent_size[(sizeof(CPreloadParent) == 0x10) ? 1 : -1];

#endif // CLASSES_CPRELOADPARENT_H
