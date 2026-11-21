// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHOPPRODUCTHAIR_H
#define CLASSES_CSHOPPRODUCTHAIR_H

#include "game/types.h"

class CShopProductHair {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* saveData;                        // 0x4: 1 use in CShopProductHair_CheckUnlockAndOwnership
    char pad_008[0x5C];
    unsigned int characterTypeId; // 0x64: CShopProductHair_CheckUnlockAndOwnership compares low 26 bits to product's character index

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x10
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHOPPRODUCTHAIR_OFF(f, o) typedef char CShopProductHair_off_##f[((int)&((CShopProductHair*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTHAIR_OFF(saveData, 0x4);
CSHOPPRODUCTHAIR_OFF(characterTypeId, 0x64);
#undef CSHOPPRODUCTHAIR_OFF

#endif  // CLASSES_CSHOPPRODUCTHAIR_H
