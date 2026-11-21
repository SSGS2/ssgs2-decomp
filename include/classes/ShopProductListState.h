// Non-virtual shared layout view for the two shop product lists in shop_product_lists_part2.cpp.
// 0x802335CC / CShopBaseB ctor initialize context and empty vector;
// CShopProductHair_CopyRecord / CShopProductCaddie_CopyRecord bounds-check itemCount before copying an item.
#ifndef CLASSES_SHOPPRODUCTLISTSTATE_H
#define CLASSES_SHOPPRODUCTLISTSTATE_H
struct ShopProductListState {
    void* vtable; // 0x0
    void* context; // 0x4: constructor argument; stronger ownership unknown
    void** items; // 0x8
    unsigned int itemCount; // 0xC
    unsigned int itemCapacity; // 0x10: grow routines CShopProductParts_RebuildFromPartTable / CShopProductHair_BuildProductList
};
#define OFF(f, o) typedef char ShopProductListState_off_##f[((int)&((ShopProductListState*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x0);
OFF(context, 0x4);
OFF(items, 0x8);
OFF(itemCount, 0xC);
OFF(itemCapacity, 0x10);
#undef OFF
#define AsShopProductListState(p) ((ShopProductListState*)(p))
#endif
