#ifndef CLASSES_CITEMCART_H
#define CLASSES_CITEMCART_H

#include "game/types.h"

class CItemCart {
public:
    void* vtbl;             // 0x0: original vtable pointer
    void* layout;           // 0x4
    int itemIcons;          // 0x8
    int itemRecords;        // 0xc
    void* playerData;       // 0x10
    void* itemCount;        // 0x14
    int refreshDelayFrames; // 0x18
    int scriptObject;       // 0x1c
    int scriptEntryId;      // 0x20
    void* message;          // 0x24
    int selectedItemIndex;  // 0x28

    signed char
        pendingInventoryIndices[8]; // 0x2c: compared with item-inventory indices; constructor/timeout fill with -1

    CItemCart* Init(void* playerRecord, void* itemCount);
    int UpdateLayout();
    int Draw();
    static int DrawItems(void* object);
    static int Update(void* object);
    static int InitLayout(void* object);
    int AddItem(int itemId);
    int IsFull();

    int SetItemIcons(void* value);
    int SetItemRecords(void* value);
    int SetItemCount(void* value);

    ~CItemCart();
};

#define CITEMCART_OFF(f, o) typedef char CItemCart_off_##f[((int)&((CItemCart*)0)->f == (o)) ? 1 : -1]
CITEMCART_OFF(layout, 0x4);
CITEMCART_OFF(itemIcons, 0x8);
CITEMCART_OFF(itemRecords, 0xc);
CITEMCART_OFF(playerData, 0x10);
CITEMCART_OFF(itemCount, 0x14);
CITEMCART_OFF(refreshDelayFrames, 0x18);
CITEMCART_OFF(scriptObject, 0x1c);
CITEMCART_OFF(scriptEntryId, 0x20);
CITEMCART_OFF(message, 0x24);
CITEMCART_OFF(selectedItemIndex, 0x28);
CITEMCART_OFF(pendingInventoryIndices, 0x2c);
#undef CITEMCART_OFF

#endif // CLASSES_CITEMCART_H
