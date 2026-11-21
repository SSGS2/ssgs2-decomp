// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHOPSUBMENUB_H
#define CLASSES_CSHOPSUBMENUB_H

#include "game/types.h"

class CShopSubMenuB {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* layout;                        // 0x4: 1 use in CShopSubMenuB_IsLoaded
    void* productTextures; // CItemCart_SetItemIcons (0x802DDB80): caller-supplied pointer.
    void* products; // CItemCart_SetItemRecords (0x802DDB8C): caller-supplied pointer.
    void* itemRecord; // 0x10: CShopSubMenuB_RemoveActivatedItemSlot/CShopSubMenuB_RemoveItemSlotByKey compact eight signed item slots at record+0x31.
    void* productCount; // CItemCart_SetItemCount (0x802DDBE8): caller-supplied pointer.
    char pad_018[4];
    void* bpObject; // CShopSubMenuB_SetEnabled (0x802DDB98): BpObject_SetEnabled receiver.
    char pad_020[4];
    void* menuMessage; // CShopSubMenuB_SetItemMessage (0x802DDBF4): MenuMessage_SetText receiver.
    int activatedElementIndex; // CShopSubMenuB_RemoveActivatedItemSlot: first Layout_CollectActivated result.
    signed char previousItemSlots[8]; // CShopSubMenuB_RemoveActivatedItemSlot: copies eight record+0x31 bytes before removing slot.
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
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

#endif  // CLASSES_CSHOPSUBMENUB_H
