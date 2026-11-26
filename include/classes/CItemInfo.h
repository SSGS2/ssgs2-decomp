#ifndef CLASSES_CITEMINFO_H
#define CLASSES_CITEMINFO_H

#include "game/types.h"

// Values for the item-detail pane, filled from the shop selection. The two message
// IDs are 40-byte names, not necessarily NUL-terminated.
struct ItemInfoDisplayData {
    int price;                     // 0x00
    short power;                   // 0x04
    short control;                 // 0x06
    short spin;                    // 0x08
    char reserved[4];              // 0x0A
    char nameMessage[0x28];        // 0x0E
    char descriptionMessage[0x28]; // 0x36
};

class CItemInfo {
public:
    void* vtbl;             // 0x0: original vtable pointer
    void* layout;           // 0x4
    int bpObject;           // 0x8
    int bpEntryId;          // 0xc
    int itemTexture;        // 0x10
    void* textTable;        // 0x14
    void* clubHouseMessage; // 0x18

    ~CItemInfo();
    CItemInfo();

    int Draw();
    int HandleDismissInput();
    int UpdateLayout();
    int UpdateLayoutAndMessage();
    int IsLoaded();
    int InitializeDetailsUI();
    int Show();
    int SetItemTexture(int value);
    int SetDisplayedData(ItemInfoDisplayData* data);
    int SetNotBuyGroupEnabled(int enabled);
};

#define CITEMINFO_OFF(f, o) typedef char CItemInfo_off_##f[((int)&((CItemInfo*)0)->f == (o)) ? 1 : -1]
CITEMINFO_OFF(layout, 0x4);
CITEMINFO_OFF(bpObject, 0x8);
CITEMINFO_OFF(bpEntryId, 0xc);
CITEMINFO_OFF(itemTexture, 0x10);
CITEMINFO_OFF(textTable, 0x14);
CITEMINFO_OFF(clubHouseMessage, 0x18);
#undef CITEMINFO_OFF

#endif // CLASSES_CITEMINFO_H
