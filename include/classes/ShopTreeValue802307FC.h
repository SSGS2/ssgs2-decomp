// ComShotPositionTreeValue: layout view; methods in shop_product_set_items.cpp (ComThinkShotPosMap_InsertNode copies string metadata; ComThinkShotPosMap_DestroyStringValue checks heap flag and frees string data).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_SHOPTREEVALUE802307FC_H
#define CLASSES_SHOPTREEVALUE802307FC_H

class ComShotPositionTreeValue {
public:
    unsigned int key;          // 0x0
    unsigned int stringFlags;  // 0x4
    unsigned int stringLength; // 0x8
    unsigned int stringData;   // 0xC
};

#define OFF(f, o) typedef char ShopTreeValue802307FC_off_##f[((int)&((ComShotPositionTreeValue*)0)->f == (o)) ? 1 : -1]
OFF(key, 0x0);
OFF(stringFlags, 0x4);
OFF(stringLength, 0x8);
OFF(stringData, 0xC);
#undef OFF

#define AsShopTreeValue802307FC(p) ((ComShotPositionTreeValue*)(p))

#endif
