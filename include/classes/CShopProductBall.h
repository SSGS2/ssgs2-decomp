#ifndef CLASSES_CSHOPPRODUCTBALL_H
#define CLASSES_CSHOPPRODUCTBALL_H

#include "game/types.h"
#include "classes/ObjCtorVt.h"
#include "classes/CShopBaseA.h"

struct CShopProductBall : CShopBaseA {
    CShopProductBall(void* a, void* b);

    void* Destroy(int shouldDelete);
    int IsOwnedOrEquipped(void* productRecord, int testEquipped);
    int AppendFilteredRecords(int filter);

    unsigned int productFilter; // 0x04
    void** begin;               // 0x08 pointer array of owned records
    unsigned int size;          // 0x0C
    unsigned int capacity;      // 0x10
};

#define CSHOPPRODUCTBALL_OFF(f, o) \
    typedef char CShopProductBall_off_##f[((int)&((CShopProductBall*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTBALL_OFF(begin, 0x8);
CSHOPPRODUCTBALL_OFF(size, 0xc);
CSHOPPRODUCTBALL_OFF(capacity, 0x10);
#undef CSHOPPRODUCTBALL_OFF

#endif // CLASSES_CSHOPPRODUCTBALL_H
