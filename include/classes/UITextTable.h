// UITextTable: partial memory view for UITextTable_SetInteger / Draw, UITextTable_SetFloat,
// and UITextTable_ClearEntries in font_num_draw_text_table.cpp. No methods, inheritance, or emitted vtable.
#ifndef CLASSES_UITEXTTABLE_H
#define CLASSES_UITEXTTABLE_H

struct UITextTableEntries {
    int count; // 0x0: UITextTable_Draw decrements after erasing a null entry
    void* root; // 0x4: setters search from this node; its address is the sentinel
    void* first; // 0x8: Draw and UITextTable_ClearEntries start iteration here
};

struct UITextTable {
    char pad_0[0xC];
    unsigned int packedColor; // 0xC: UITextTable_Draw packs the color components returned by UIGroup_GetTintColor
    int horizontalAlignment; // 0x10: Draw selects left (0), center (1), or right (2) from descriptor flags
    char pad_14[0x4];
    UITextTableEntries entries; // 0x18: UITextTable_ClearEntries passes this tree to RBTree_FreeTrivialSubtree
    int drawProcessed; // 0x24: Draw returns immediately when set, otherwise sets it to 1
    int drawEnabled; // 0x28: Draw returns before traversal when zero
    float offsetX; // 0x2C: Draw adds this to the x coordinate passed to HudFont_DrawAlignedTextWithVariant
    float offsetY; // 0x30: Draw adds this to the y coordinate passed to HudFont_DrawAlignedTextWithVariant
};

#define OFF(f, o) typedef char UITextTableEntries_off_##f[((int)&((UITextTableEntries*)0)->f == (o)) ? 1 : -1]
OFF(count, 0x0);
OFF(root, 0x4);
OFF(first, 0x8);
#undef OFF
#define OFF(f, o) typedef char UITextTable_off_##f[((int)&((UITextTable*)0)->f == (o)) ? 1 : -1]
OFF(packedColor, 0xC);
OFF(horizontalAlignment, 0x10);
OFF(entries, 0x18);
OFF(drawProcessed, 0x24);
OFF(drawEnabled, 0x28);
OFF(offsetX, 0x2C);
OFF(offsetY, 0x30);
#undef OFF
#define AsUITextTable(p) ((UITextTable*)(p))

#endif
