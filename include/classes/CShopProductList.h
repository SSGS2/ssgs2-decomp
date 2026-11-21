// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHOPPRODUCTLIST_H
#define CLASSES_CSHOPPRODUCTLIST_H

#include "game/types.h"

class CShopProductList {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    void* products;                        // 0x8: 2 uses in CShopProductList_DeleteAll
    int productCount;                          // 0xc: 4 uses in CShopProductList_DeleteAll, CShopProductList_GetCount
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHOPPRODUCTLIST_OFF(f, o) typedef char CShopProductList_off_##f[((int)&((CShopProductList*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTLIST_OFF(products, 0x8);
CSHOPPRODUCTLIST_OFF(productCount, 0xc);
#undef CSHOPPRODUCTLIST_OFF

#endif  // CLASSES_CSHOPPRODUCTLIST_H
