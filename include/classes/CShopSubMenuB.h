// The shop's item cart and its equipment submenu. It shows the products the
// player has put in the cart and the consumables equipped on the current
// character, and keeps the save record in step as items are added or removed.
//
// The class shares its layout with CItemCart, which only declares the fields up
// to the equipped-item slot array.
#ifndef CLASSES_CSHOPSUBMENUB_H
#define CLASSES_CSHOPSUBMENUB_H

#include "game/types.h"

class CShopSubMenuB {
public:
    int IsLoaded();
    int SetEnabled(int enabled);
    int SetItemMessage(int itemId);
    int RemoveActivatedItemSlot();
    int RemoveItemSlotByKey(int value);
    int UpdateItemSlotFlags();

    void* vtbl;            // 0x00
    void* layout;          // 0x04
    void* productTextures; // 0x08
    void* products;        // 0x0C
    void* itemRecord;      // 0x10  the save record the equipped items live in
    void* productCount;    // 0x14
    char pad_018[4];
    void* bpObject; // 0x1C  the script object that draws the submenu
    char pad_020[4];
    void* menuMessage;                // 0x24
    int activatedElementIndex;        // 0x28  element that was activated in the layout
    signed char previousItemSlots[8]; // 0x2C  equipped slots before the last removal
};

#define CSHOPSUBMENUB_OFF(f, o) typedef char CShopSubMenuB_off_##f[((int)&((CShopSubMenuB*)0)->f == (o)) ? 1 : -1]
CSHOPSUBMENUB_OFF(layout, 0x4);
CSHOPSUBMENUB_OFF(productTextures, 0x8);
CSHOPSUBMENUB_OFF(products, 0xC);
CSHOPSUBMENUB_OFF(itemRecord, 0x10);
CSHOPSUBMENUB_OFF(productCount, 0x14);
CSHOPSUBMENUB_OFF(bpObject, 0x1C);
CSHOPSUBMENUB_OFF(menuMessage, 0x24);
CSHOPSUBMENUB_OFF(activatedElementIndex, 0x28);
CSHOPSUBMENUB_OFF(previousItemSlots, 0x2C);
#undef CSHOPSUBMENUB_OFF
typedef char CShopSubMenuB_size[(sizeof(CShopSubMenuB) == 0x34) ? 1 : -1];

#endif // CLASSES_CSHOPSUBMENUB_H
