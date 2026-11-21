// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHOPPRODUCTSTAB_H
#define CLASSES_CSHOPPRODUCTSTAB_H

#include "game/types.h"

class CShopProductsTab {
public:
    void* vtbl;                        // 0x0: 2 uses in CShopProductsTab_Ctor, CShopProductsTab_dtor
    int selectedProductIndex;                          // 0x4: 2 uses in CShopProductsTab_Ctor, CShopProductsTab_Reset
    int productCount;                          // 0x8: 3 uses in CShopProductsTab_Reset, CShopProductsTab_Ctor
    int drawOffsetX;                          // 0xc: 1 use in CShopProductsTab_Ctor
    int drawOffsetY;                         // 0x10: 1 use in CShopProductsTab_Ctor
    int drawOffsetZ;                         // 0x14: 1 use in CShopProductsTab_Ctor
    int productDrawEntries;                         // 0x18: 3 uses in CShopProductsTab_Reset, CShopProductsTab_Ctor
    void* layout;                       // 0x1c: 7 uses in CShopProductsTab_dtor, CShopProductsTab_Ctor, CShopProductsTab_Reset
    int navigationDelayFrames;                         // 0x20: 1 use in CShopProductsTab_Ctor
    int productRecords;                         // 0x24: 1 use in CShopProductsTab_Ctor
    void* secondaryLayout;                       // 0x28: 5 uses in CShopProductsTab_dtor, CShopProductsTab_Ctor
    void* tertiaryLayout;                       // 0x2c: 4 uses in CShopProductsTab_dtor, CShopProductsTab_Ctor
    void* scriptObject;                       // 0x30: 5 uses in CShopProductsTab_Reset, CShopProductsTab_Ctor
    int scriptEntryId;                         // 0x34: 5 uses in CShopProductsTab_dtor, CShopProductsTab_Ctor
    void* playerRecord;                       // 0x38: 1 use in CShopProductsTab_Ctor
    unsigned char selectedProductNew;               // 0x3c: 1 use in CShopProductsTab_Ctor
    unsigned char selectedProductOwned;               // 0x3d: 1 use in CShopProductsTab_Ctor
    unsigned char selectedProductEquipped;               // 0x3e: 1 use in CShopProductsTab_Ctor
    char pad_03F[0x1];                  // 0x3f
    int selectedProductQuantity;                         // 0x40: 1 use in CShopProductsTab_Ctor
    void* numberDraw;                       // 0x44: 7 uses in CShopProductsTab_dtor, CShopProductsTab_Ctor, CShopProductsTab_Reset

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x34
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHOPPRODUCTSTAB_OFF(f, o) typedef char CShopProductsTab_off_##f[((int)&((CShopProductsTab*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTSTAB_OFF(selectedProductIndex, 0x4);
CSHOPPRODUCTSTAB_OFF(productCount, 0x8);
CSHOPPRODUCTSTAB_OFF(drawOffsetX, 0xc);
CSHOPPRODUCTSTAB_OFF(drawOffsetY, 0x10);
CSHOPPRODUCTSTAB_OFF(drawOffsetZ, 0x14);
CSHOPPRODUCTSTAB_OFF(productDrawEntries, 0x18);
CSHOPPRODUCTSTAB_OFF(layout, 0x1c);
CSHOPPRODUCTSTAB_OFF(navigationDelayFrames, 0x20);
CSHOPPRODUCTSTAB_OFF(productRecords, 0x24);
CSHOPPRODUCTSTAB_OFF(secondaryLayout, 0x28);
CSHOPPRODUCTSTAB_OFF(tertiaryLayout, 0x2c);
CSHOPPRODUCTSTAB_OFF(scriptObject, 0x30);
CSHOPPRODUCTSTAB_OFF(scriptEntryId, 0x34);
CSHOPPRODUCTSTAB_OFF(playerRecord, 0x38);
CSHOPPRODUCTSTAB_OFF(selectedProductNew, 0x3c);
CSHOPPRODUCTSTAB_OFF(selectedProductOwned, 0x3d);
CSHOPPRODUCTSTAB_OFF(selectedProductEquipped, 0x3e);
CSHOPPRODUCTSTAB_OFF(selectedProductQuantity, 0x40);
CSHOPPRODUCTSTAB_OFF(numberDraw, 0x44);
#undef CSHOPPRODUCTSTAB_OFF

#endif  // CLASSES_CSHOPPRODUCTSTAB_H
