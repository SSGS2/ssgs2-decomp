// ComShotPositionTreeView: layout view; methods in shop_product_set_items.cpp (ComThinkShotPosMap_InsertNode increments count, rebalances root, and optionally sets firstNode).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_SHOPTREE802307FC_H
#define CLASSES_SHOPTREE802307FC_H

class ComShotPositionTreeView {
public:
    unsigned int count; // 0x0
    void* root;         // 0x4
    char pad_008[0x4];
    void* firstNode; // 0xC
};

#define OFF(f, o) typedef char ShopTree802307FC_off_##f[((int)&((ComShotPositionTreeView*)0)->f == (o)) ? 1 : -1]
OFF(count, 0x0);
OFF(root, 0x4);
OFF(firstNode, 0xC);
#undef OFF

#define AsShopTree802307FC(p) ((ComShotPositionTreeView*)(p))

#endif
