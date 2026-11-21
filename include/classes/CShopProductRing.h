// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHOPPRODUCTRING_H
#define CLASSES_CSHOPPRODUCTRING_H

#include "game/types.h"

class CShopProductRing {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* saveData;                        // 0x4: 1 use in CShopProductRing_IsOwned
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHOPPRODUCTRING_OFF(f, o) typedef char CShopProductRing_off_##f[((int)&((CShopProductRing*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTRING_OFF(saveData, 0x4);
#undef CSHOPPRODUCTRING_OFF

#endif  // CLASSES_CSHOPPRODUCTRING_H
