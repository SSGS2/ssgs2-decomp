// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CITEMCART_H
#define CLASSES_CITEMCART_H

#include "game/types.h"

class CItemCart {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* layout;                        // 0x4: 4 uses in CItemCart_Ctor, CItemCart_dtor
    int itemIcons;                          // 0x8: 1 use in CItemCart_Ctor
    int itemRecords;                          // 0xc: 1 use in CItemCart_Ctor
    void* playerData;                       // 0x10: 1 use in CItemCart_Ctor
    void* itemCount;                       // 0x14: 1 use in CItemCart_Ctor
    int refreshDelayFrames;                         // 0x18: 1 use in CItemCart_Ctor
    int scriptObject;                         // 0x1c: 2 uses in CItemCart_Ctor
    int scriptEntryId;                         // 0x20: 5 uses in CItemCart_dtor, CItemCart_Ctor
    void* message;                       // 0x24: 3 uses in CItemCart_dtor, CItemCart_Ctor
    int selectedItemIndex;                         // 0x28: 1 use in CItemCart_Ctor

    signed char pendingInventoryIndices[8]; // 0x2c: compared with item-inventory indices; constructor/timeout fill with -1

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x20, 0x2c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
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

#endif  // CLASSES_CITEMCART_H
