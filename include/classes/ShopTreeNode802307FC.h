// ComShotPositionTreeNode: layout view; methods in shop_product_set_items.cpp (ComThinkShotPosMap_InsertNode inserts children; ComThinkShotPosMap_DestroySubtree recursively frees them).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_SHOPTREENODE802307FC_H
#define CLASSES_SHOPTREENODE802307FC_H

class ComShotPositionTreeNode {
public:
    void* left; // 0x0
    void* right; // 0x4
    void* parent; // 0x8
    char pad_00C[0x4];
    unsigned int stringFlags; // 0x10
    char pad_014[0x4];
    void* stringHeap; // 0x18
};

#define OFF(f, o) typedef char ShopTreeNode802307FC_off_##f[((int)&((ComShotPositionTreeNode*)0)->f == (o)) ? 1 : -1]
OFF(left, 0x0);
OFF(right, 0x4);
OFF(parent, 0x8);
OFF(stringFlags, 0x10);
OFF(stringHeap, 0x18);
#undef OFF

#define AsShopTreeNode802307FC(p) ((ComShotPositionTreeNode*)(p))

#endif
