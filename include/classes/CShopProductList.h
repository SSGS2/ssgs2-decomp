#ifndef CLASSES_CSHOPPRODUCTLIST_H
#define CLASSES_CSHOPPRODUCTLIST_H

#include "game/types.h"

class CShopProductList {
public:
    void* vtbl;        // 0x0: original vtable pointer
    char pad_004[0x4]; // 0x4
    void* products;    // 0x8
    int productCount;  // 0xc

    int GetCount();
    int DeleteAll();
};

#define CSHOPPRODUCTLIST_OFF(f, o) \
    typedef char CShopProductList_off_##f[((int)&((CShopProductList*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTLIST_OFF(products, 0x8);
CSHOPPRODUCTLIST_OFF(productCount, 0xc);
#undef CSHOPPRODUCTLIST_OFF

#endif // CLASSES_CSHOPPRODUCTLIST_H
