#ifndef CLASSES_CSHOPPRODUCTEQUIPITEM_H
#define CLASSES_CSHOPPRODUCTEQUIPITEM_H

#include "game/types.h"
#include "classes/ObjCtorVt.h"
#include "classes/CShopBaseE.h"

struct Fn80238B40Arg;
struct ShopProductRecordView;
struct CShopProductEquipItem : CShopBaseE {
    CShopProductEquipItem(void* a, void* b);

    void* Destroy(int shouldDelete);
    static int vfunc_6(Fn80238B40Arg* product, ShopProductRecordView* productRecord, int testEquipped);
    int* GetCount();

    char pad_04[0x8];
    unsigned int size; // 0x0C
};

#define CSHOPPRODUCTEQUIPITEM_OFF(f, o) \
    typedef char CShopProductEquipItem_off_##f[((int)&((CShopProductEquipItem*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTEQUIPITEM_OFF(size, 0xc);
#undef CSHOPPRODUCTEQUIPITEM_OFF

#endif // CLASSES_CSHOPPRODUCTEQUIPITEM_H
