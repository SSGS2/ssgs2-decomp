// ShopPointerArray802307FC: layout view; methods in shop_product_set_items.cpp (CShopProductAvatar_CopyRecord bounds-checks count; PtrArray_DeleteAllAndClear deletes data entries).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_SHOPPOINTERARRAY802307FC_H
#define CLASSES_SHOPPOINTERARRAY802307FC_H

class ShopPointerArray802307FC {
public:
    char pad_000[0x8];
    void** data; // 0x8
    unsigned int count; // 0xC
};

#define OFF(f, o) typedef char ShopPointerArray802307FC_off_##f[((int)&((ShopPointerArray802307FC*)0)->f == (o)) ? 1 : -1]
OFF(data, 0x8);
OFF(count, 0xC);
#undef OFF

#define AsShopPointerArray802307FC(p) ((ShopPointerArray802307FC*)(p))

#endif
