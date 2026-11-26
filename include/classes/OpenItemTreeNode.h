// OpenItemTreeNode: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_OPENITEMTREENODE_H
#define CLASSES_OPENITEMTREENODE_H

class OpenItemTreeNode {
public:
    void* left;                  // 0x0: F2FC_Unused
    void* right;                 // 0x4: F2FC_Draw, F2FC_Unused, CTourMain_UpdateItemMenuVisuals
    unsigned int parentAndColor; // 0x8: F2FC_Draw, F2FC_Unused, CTourMain_UpdateItemMenuVisuals
    char pad_00C[0x4];
    void* item; // 0x10: F2FC_Draw, F2FC_Unused, CTourMain_UpdateItemMenuVisuals, OpenItemMenu_UpdateEntryVisuals
};

#define OFF(f, o) typedef char OpenItemTreeNode_off_##f[((int)&((OpenItemTreeNode*)0)->f == (o)) ? 1 : -1]
OFF(left, 0x0);
OFF(right, 0x4);
OFF(parentAndColor, 0x8);
OFF(item, 0x10);
#undef OFF

#endif
