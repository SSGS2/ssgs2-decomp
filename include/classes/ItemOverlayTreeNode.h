// ItemOverlayTreeNode: layout view owned by item_window_hud.cpp.
// Evidence: CItemWindow_OnDestroy: +8 masked with ~1 for parent; +0x10 released with Object_DeleteVirtual. No virtual declarations or runtime objects.
#ifndef CLASSES_ITEMOVERLAYTREENODE_H
#define CLASSES_ITEMOVERLAYTREENODE_H

class ItemOverlayTreeNode {
public:
    void* left;                  // 0x0; CItemWindow_OnDestroy: +8 masked with ~1 for parent
    void* right;                 // 0x4; CItemWindow_OnDestroy: +8 masked with ~1 for parent
    unsigned int parentAndColor; // 0x8; CItemWindow_OnDestroy: +8 masked with ~1 for parent
    char pad_C[0x4];
    void* overlay; // 0x10; CItemWindow_OnDestroy: +8 masked with ~1 for parent
};

#define OFF(f, o) typedef char ItemOverlayTreeNode_off_##f[((int)&((ItemOverlayTreeNode*)0)->f == (o)) ? 1 : -1]
OFF(left, 0x0);
OFF(right, 0x4);
OFF(parentAndColor, 0x8);
OFF(overlay, 0x10);
#undef OFF

#endif
