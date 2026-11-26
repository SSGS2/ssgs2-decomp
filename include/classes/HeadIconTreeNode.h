// HeadIconTreeNode: non-virtual layout view; methods/accesses in head_icon_and_aux_menu_ui.cpp.
// No inheritance or virtual declarations: the original vtables remain external symbols.
#ifndef CLASSES_HEADICONTREENODE_H
#define CLASSES_HEADICONTREENODE_H

class HeadIconTreeNode {
public:
    void* left;              // 0x0: HeadIconMap_FindInsertionPosition: key less than nodeKey follows +0
    void* right;             // 0x4: HeadIconMap_FindInsertionPosition: other keys follow +4
    unsigned parentAndColor; // 0x8: CHeadIcon_dtor masks bit 0 before following parent
    unsigned key;            // 0xC: HeadIconMap_FindInsertionPosition compares unsigned keys
    void* overlay;           // 0x10: CHeadIcon_dtor: Object_DeleteVirtual and clear
};

#define OFF(f, o) typedef char HeadIconTreeNode_off_##f[((int)&((HeadIconTreeNode*)0)->f == (o)) ? 1 : -1]
OFF(left, 0x0);
OFF(right, 0x4);
OFF(parentAndColor, 0x8);
OFF(key, 0xC);
OFF(overlay, 0x10);
#undef OFF

#endif
