#ifndef CLASSES_CSHOPPRODUCTCADDIEITEM_H
#define CLASSES_CSHOPPRODUCTCADDIEITEM_H

#include "game/types.h"
#include "classes/CShopBaseC.h"

// Caddie-item product list: a vector of owned copies of caddie-item IFF records.
// The vector's storage and record copies are released by DeleteAll, which keeps the
// pointer capacity. The vtable is the CShopBaseC vtable.
class CShopProductCaddieItem : public CShopBaseC {
public:
    CShopProductCaddieItem(void* caddieTypeIdValue, void* b, void* c);

    int DeleteAll();
    int GetCount();
    int CopyRecord(void* destination, unsigned int productIndex);
    int LoadFromInventory(void* filterContext);
    int CopyCommonRecord(void* destination, unsigned int index);
    int IsAvailableForCaddie(void* productRecord, int testEquipped);
    void* Destroy(int shouldDelete);

    void* context;         // 0x04
    void** begin;          // 0x08 pointer array of owned records
    unsigned int size;     // 0x0C
    unsigned int capacity; // 0x10
    void* caddieTypeId;    // 0x14
};

#define CSHOPPRODUCTCADDIEITEM_OFF(f, o) \
    typedef char CShopProductCaddieItem_off_##f[((int)&((CShopProductCaddieItem*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTCADDIEITEM_OFF(begin, 0x8);
CSHOPPRODUCTCADDIEITEM_OFF(size, 0xc);
CSHOPPRODUCTCADDIEITEM_OFF(capacity, 0x10);
CSHOPPRODUCTCADDIEITEM_OFF(caddieTypeId, 0x14);
#undef CSHOPPRODUCTCADDIEITEM_OFF

#endif // CLASSES_CSHOPPRODUCTCADDIEITEM_H
