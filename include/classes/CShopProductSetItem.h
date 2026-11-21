// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHOPPRODUCTSETITEM_H
#define CLASSES_CSHOPPRODUCTSETITEM_H

#include "game/types.h"

class CShopProductSetItem {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    void* itemRecords;                        // 0x8: 5 uses in CShopProductSetItem_RebuildFilteredRecords, CShopProductSetItem_CopyRecord
    unsigned int itemRecordCount;                 // 0xc: 8 uses in CShopProductSetItem_RebuildFilteredRecords, CShopProductSetItem_CopyRecord
    unsigned int itemRecordCapacity;                // 0x10: 5 uses in CShopProductSetItem_RebuildFilteredRecords
    char pad_014[0x50];                 // 0x14
    unsigned int characterTypeId;       // 0x64: CShopProductSetItem_IsSetOwnedForCharacter

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x10
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHOPPRODUCTSETITEM_OFF(f, o) typedef char CShopProductSetItem_off_##f[((int)&((CShopProductSetItem*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTSETITEM_OFF(itemRecords, 0x8);
CSHOPPRODUCTSETITEM_OFF(itemRecordCount, 0xc);
CSHOPPRODUCTSETITEM_OFF(itemRecordCapacity, 0x10);
CSHOPPRODUCTSETITEM_OFF(characterTypeId, 0x64);
#undef CSHOPPRODUCTSETITEM_OFF

#endif  // CLASSES_CSHOPPRODUCTSETITEM_H
