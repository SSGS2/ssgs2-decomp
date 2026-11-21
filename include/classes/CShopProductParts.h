// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHOPPRODUCTPARTS_H
#define CLASSES_CSHOPPRODUCTPARTS_H

#include "game/types.h"

class CShopProductParts {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* saveData;                        // 0x4: 1 use in CShopProductParts_AcceptsPart
    char pad_008[0x128];                // 0x8
    unsigned int partTypeMask;               // 0x130: 1 use in CShopProductParts_AcceptsPart
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHOPPRODUCTPARTS_OFF(f, o) typedef char CShopProductParts_off_##f[((int)&((CShopProductParts*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTPARTS_OFF(saveData, 0x4);
CSHOPPRODUCTPARTS_OFF(partTypeMask, 0x130);
#undef CSHOPPRODUCTPARTS_OFF

#endif  // CLASSES_CSHOPPRODUCTPARTS_H
