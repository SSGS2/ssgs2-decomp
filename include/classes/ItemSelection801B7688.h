// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_ITEMSELECTION801B7688_H
#define CLASSES_ITEMSELECTION801B7688_H

#include "game/types.h"

class ItemSelection801B7688 {
public:
    char pad_000[4]; // 0x0: unmodelled bytes
    unsigned char primaryCount;         // 0x4: ItemSelection_Init original object access
    unsigned char totalCount;           // 0x5: ItemSelection_Init original object access
    unsigned char textureVariant;                // 0x6: ItemSelection_Init original object access
    unsigned char bitmapVariant;                // 0x7: ItemSelection_Init original object access
    unsigned int firstPartId;                 // 0x8: ItemSelection_SetBaseItem original object access
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define ITEMSELECTION801B7688_OFF(f, o) typedef char ItemSelection801B7688_off_##f[((int)&((ItemSelection801B7688*)0)->f == (o)) ? 1 : -1]
ITEMSELECTION801B7688_OFF(primaryCount, 0x4);
ITEMSELECTION801B7688_OFF(totalCount, 0x5);
ITEMSELECTION801B7688_OFF(textureVariant, 0x6);
ITEMSELECTION801B7688_OFF(bitmapVariant, 0x7);
ITEMSELECTION801B7688_OFF(firstPartId, 0x8);
#undef ITEMSELECTION801B7688_OFF

#endif  // CLASSES_ITEMSELECTION801B7688_H
