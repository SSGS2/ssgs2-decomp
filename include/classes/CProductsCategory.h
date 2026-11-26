#ifndef CLASSES_CPRODUCTSCATEGORY_H
#define CLASSES_CPRODUCTSCATEGORY_H

#include "game/types.h"

class CProductsCategory {
public:
    void* vtbl;                // 0x0: original vtable pointer
    void* layout;              // 0x4
    int categoryCount;         // 0x8
    int selectedCategoryIndex; // 0xc
    int selectionY
        [5]; // 0x10: CProductsCategory_InitLayout writes five Y positions used by CProductsCategory_UpdateSelectionLayout

    int UpdateLayout();
    int Draw();
    static int UpdateSelectionFlags(void* p);
    int SetMarkerVisible(int idx, int val);
    int SetSelectedIndex(int idx);
    int SetLayoutMode(int v);
    int SetModeAndSelection(int v, int idx);
    int InitLayout(int mode, int selected);
    int UpdateSelectionLayout();
    int SetCategoryCount(int count);
    int CollectActivatedIndices(void* indices, int unused);

    ~CProductsCategory();
    CProductsCategory(int arg);
};

#define CPRODUCTSCATEGORY_OFF(f, o) \
    typedef char CProductsCategory_off_##f[((int)&((CProductsCategory*)0)->f == (o)) ? 1 : -1]
CPRODUCTSCATEGORY_OFF(layout, 0x4);
CPRODUCTSCATEGORY_OFF(categoryCount, 0x8);
CPRODUCTSCATEGORY_OFF(selectedCategoryIndex, 0xc);
CPRODUCTSCATEGORY_OFF(selectionY, 0x10);
#undef CPRODUCTSCATEGORY_OFF

#endif // CLASSES_CPRODUCTSCATEGORY_H
