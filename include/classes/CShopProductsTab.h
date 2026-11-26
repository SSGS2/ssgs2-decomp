#ifndef CLASSES_CSHOPPRODUCTSTAB_H
#define CLASSES_CSHOPPRODUCTSTAB_H

#include "game/types.h"

class CShopProductsTab {
public:
    void* vtbl;                            // 0x0
    int selectedProductIndex;              // 0x4
    int productCount;                      // 0x8
    int drawOffsetX;                       // 0xc
    int drawOffsetY;                       // 0x10
    int drawOffsetZ;                       // 0x14
    int productDrawEntries;                // 0x18
    void* layout;                          // 0x1c
    int navigationDelayFrames;             // 0x20
    int productRecords;                    // 0x24
    void* secondaryLayout;                 // 0x28
    void* tertiaryLayout;                  // 0x2c
    void* scriptObject;                    // 0x30
    int scriptEntryId;                     // 0x34
    void* playerRecord;                    // 0x38
    unsigned char selectedProductNew;      // 0x3c
    unsigned char selectedProductOwned;    // 0x3d
    unsigned char selectedProductEquipped; // 0x3e
    char pad_03F[0x1];                     // 0x3f
    int selectedProductQuantity;           // 0x40
    void* numberDraw;                      // 0x44

    void* Init(int productCount, const void* drawOffset, void* playerRecord);
    int Destroy(int flag);
    int Reset();
    int PollLoad();
    int UpdateLayouts();
    int UpdateNavigationDelay();
    int Draw();
    void* GetSelectedProductIndex();
    int RefreshVisibleSlotIndicators();
    int IsSelectedProductOwned();
    int RefreshVisibleProducts();
    int HandleNavigationInput();
    int HandleNavigationActivation();
    int UpdateProductStatus(unsigned int typeId);
    int UpdateCharacterStatus(const char* typeIdArg);
    int UpdatePartStatus(void* typeIdArg);
    int UpdateCaddieStatus(void* typeIdArg);
    int UpdateCaddieItemStatus(void* typeIdArg);
    int UpdateClubSetStatus(void* typeIdArg);
    int UpdateBallStatus(void* typeIdArg);
    int UpdateSlotIndicators(int visibleSlot);
    int ApplyDrawOffsetY();
    int ClearProductFlag(void* typeIdArg);
    int SetProductArrays(void* entries, void* arg);
    int SetProductCount(int count);
    int SetEnabled(int enable);
    int SelectProductByTypeId(void* typeIdArg);
    int DrawProductQuads() const;
};

#define CSHOPPRODUCTSTAB_OFF(f, o) \
    typedef char CShopProductsTab_off_##f[((int)&((CShopProductsTab*)0)->f == (o)) ? 1 : -1]
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

#endif // CLASSES_CSHOPPRODUCTSTAB_H
