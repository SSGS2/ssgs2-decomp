#ifndef CLASSES_CSHOPPRODUCTSETITEM_H
#define CLASSES_CSHOPPRODUCTSETITEM_H

#include "game/types.h"

class CShopProductSetItem {
public:
    void* vtbl;                      // 0x0: original vtable pointer
    char pad_004[0x4];               // 0x4
    void* itemRecords;               // 0x8
    unsigned int itemRecordCount;    // 0xc
    unsigned int itemRecordCapacity; // 0x10
    char pad_014[0x50];              // 0x14
    unsigned int characterTypeId;    // 0x64: CShopProductSetItem_IsSetOwnedForCharacter

    int CopySelectedCommonHeader(void* out, unsigned int index);
    static int ReturnFalse();
    int CopyRecord(void* dst, unsigned idx);
    int RebuildFilteredRecords(void* filterContext);
    CShopProductSetItem(void* recordPayload, void* context, void* initializationArgument);
    ~CShopProductSetItem();

    int IsSetOwnedForCharacter(void* entry, void* partFlagContext);
};

#define CSHOPPRODUCTSETITEM_OFF(f, o) \
    typedef char CShopProductSetItem_off_##f[((int)&((CShopProductSetItem*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTSETITEM_OFF(itemRecords, 0x8);
CSHOPPRODUCTSETITEM_OFF(itemRecordCount, 0xc);
CSHOPPRODUCTSETITEM_OFF(itemRecordCapacity, 0x10);
CSHOPPRODUCTSETITEM_OFF(characterTypeId, 0x64);
#undef CSHOPPRODUCTSETITEM_OFF

#endif // CLASSES_CSHOPPRODUCTSETITEM_H
