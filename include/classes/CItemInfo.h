// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CITEMINFO_H
#define CLASSES_CITEMINFO_H

#include "game/types.h"

class CItemInfo {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* layout;                        // 0x4: 4 uses in CItemInfo_dtor, CItemInfo_Ctor
    int bpObject;                          // 0x8: 2 uses in CItemInfo_Ctor
    int bpEntryId;                          // 0xc: 5 uses in CItemInfo_dtor, CItemInfo_Ctor
    int itemTexture;                         // 0x10: 1 use in CItemInfo_Ctor
    void* textTable;                       // 0x14: 5 uses in CItemInfo_dtor, CItemInfo_Ctor
    void* clubHouseMessage;                       // 0x18: 3 uses in CItemInfo_dtor, CItemInfo_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CITEMINFO_OFF(f, o) typedef char CItemInfo_off_##f[((int)&((CItemInfo*)0)->f == (o)) ? 1 : -1]
CITEMINFO_OFF(layout, 0x4);
CITEMINFO_OFF(bpObject, 0x8);
CITEMINFO_OFF(bpEntryId, 0xc);
CITEMINFO_OFF(itemTexture, 0x10);
CITEMINFO_OFF(textTable, 0x14);
CITEMINFO_OFF(clubHouseMessage, 0x18);
#undef CITEMINFO_OFF

#endif  // CLASSES_CITEMINFO_H
