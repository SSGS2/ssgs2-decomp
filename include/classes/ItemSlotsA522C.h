#ifndef CLASSES_ITEMSLOTSA522C_H
#define CLASSES_ITEMSLOTSA522C_H

#include "game/types.h"

class ItemSlotsA522C {
public:
    void*
        entries; // 0x0: PartyItemSlots_ctor initializes; PartyItemSlots_GetItemType bounds-checks selected index; PartyItemSlots_RefreshDisplay lazily creates display
    unsigned int
        entryCount; // 0x4: PartyItemSlots_ctor initializes; PartyItemSlots_GetItemType bounds-checks selected index; PartyItemSlots_RefreshDisplay lazily creates display
    unsigned int
        capacity; // 0x8: PartyItemSlots_ctor initializes; PartyItemSlots_GetItemType bounds-checks selected index; PartyItemSlots_RefreshDisplay lazily creates display
    unsigned int
        selectedIndex; // 0xc: PartyItemSlots_ctor initializes; PartyItemSlots_GetItemType bounds-checks selected index; PartyItemSlots_RefreshDisplay lazily creates display
    char pad_010[0x2c]; // 0x10
    float
        unk_3C; // 0x3c: PartyItemSlots_ctor initializes; PartyItemSlots_GetItemType bounds-checks selected index; PartyItemSlots_RefreshDisplay lazily creates display
    char pad_040[0x28]; // 0x40
    void*
        display; // 0x68: PartyItemSlots_ctor initializes; PartyItemSlots_GetItemType bounds-checks selected index; PartyItemSlots_RefreshDisplay lazily creates display
};

#define ITEMSLOTSA522C_OFF(f, o) typedef char ItemSlotsA522C_off_##f[((int)&((ItemSlotsA522C*)0)->f == (o)) ? 1 : -1]
ITEMSLOTSA522C_OFF(entryCount, 0x4);
ITEMSLOTSA522C_OFF(capacity, 0x8);
ITEMSLOTSA522C_OFF(selectedIndex, 0xc);
ITEMSLOTSA522C_OFF(unk_3C, 0x3c);
ITEMSLOTSA522C_OFF(display, 0x68);
ITEMSLOTSA522C_OFF(entries, 0x0);
#undef ITEMSLOTSA522C_OFF

#endif // CLASSES_ITEMSLOTSA522C_H
