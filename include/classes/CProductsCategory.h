// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPRODUCTSCATEGORY_H
#define CLASSES_CPRODUCTSCATEGORY_H

#include "game/types.h"

class CProductsCategory {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* layout;                        // 0x4: 4 uses in CProductsCategory_dtor, CProductsCategory_Ctor
    int categoryCount;                          // 0x8: 1 use in CProductsCategory_Ctor
    int selectedCategoryIndex;                          // 0xc: 1 use in CProductsCategory_Ctor
    int selectionY[5]; // 0x10: CProductsCategory_InitLayout writes five Y positions used by CProductsCategory_UpdateSelectionLayout
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPRODUCTSCATEGORY_OFF(f, o) typedef char CProductsCategory_off_##f[((int)&((CProductsCategory*)0)->f == (o)) ? 1 : -1]
CPRODUCTSCATEGORY_OFF(layout, 0x4);
CPRODUCTSCATEGORY_OFF(categoryCount, 0x8);
CPRODUCTSCATEGORY_OFF(selectedCategoryIndex, 0xc);
CPRODUCTSCATEGORY_OFF(selectionY, 0x10);
#undef CPRODUCTSCATEGORY_OFF

#endif  // CLASSES_CPRODUCTSCATEGORY_H
