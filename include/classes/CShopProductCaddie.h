#ifndef CLASSES_CSHOPPRODUCTCADDIE_H
#define CLASSES_CSHOPPRODUCTCADDIE_H

#include "game/types.h"
#include "classes/CShopBaseB.h"

// Caddie product list: a vector of owned copies of caddie IFF records.
// The vtable is the CShopBaseB vtable.
class CShopProductCaddie : public CShopBaseB {
public:
    CShopProductCaddie(void* a, void* b);

    int LoadFromInventory(void* filterContext);
    int GetCount();
    int CopyCommonRecord(void* destination, unsigned int index);
    int CopyRecord(void* outRecord, unsigned int productIndex);
    int DeleteAll();
    int IsOwnedOrEquipped(void* productRecord, int testEquipped);
    void* Destroy(int shouldDelete);

    void* context;         // 0x04
    void** begin;          // 0x08 pointer array of owned records
    unsigned int size;     // 0x0C
    unsigned int capacity; // 0x10
};

#define CSHOPPRODUCTCADDIE_OFF(f, o) \
    typedef char CShopProductCaddie_off_##f[((int)&((CShopProductCaddie*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTCADDIE_OFF(begin, 0x8);
CSHOPPRODUCTCADDIE_OFF(size, 0xc);
CSHOPPRODUCTCADDIE_OFF(capacity, 0x10);
#undef CSHOPPRODUCTCADDIE_OFF

#endif // CLASSES_CSHOPPRODUCTCADDIE_H
