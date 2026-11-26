// ShopProductCharFields: layout view; methods in shop_product_set_items.cpp (CShopProduct<IFF_STRUCT::sChar> constructor/destructor).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_SHOPPRODUCTCHARFIELDS_H
#define CLASSES_SHOPPRODUCTCHARFIELDS_H

class ShopProductCharFields {
public:
    void* vtable;  // 0x0
    void* context; // 0x4
    int data;      // 0x8
    int size;      // 0xC
    int capacity;  // 0x10
};

#define OFF(f, o) typedef char ShopProductCharFields_off_##f[((int)&((ShopProductCharFields*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x0);
OFF(context, 0x4);
OFF(data, 0x8);
OFF(size, 0xC);
OFF(capacity, 0x10);
#undef OFF

#define AsShopProductCharFields(p) ((ShopProductCharFields*)(p))

#endif
