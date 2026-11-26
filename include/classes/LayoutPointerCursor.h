// Cursor view used by Layout_CursorFromPointer/GetCursorPosition/GetCursorDepth in layout_cursor_pointer_controller.cpp.
// Separate from the 0x5C-byte controller initialized by Layout_Init and from CSPDPangyaIcon.
#ifndef CLASSES_LAYOUTCURSOR800779EC_H
#define CLASSES_LAYOUTCURSOR800779EC_H

struct LayoutPointerCursor {
    void* player;      // 0x0: Layout_UpdateCursorFromPlayer (0x80077BC8) passes to gtl_InputEx_GetScreenPointer.
    float minX;        // 0x4: Layout_CursorFromPointer (0x800779F8) clamps cursorX from below.
    float minY;        // 0x8: same, lower y bound.
    float maxX;        // 0xC: same, upper x bound.
    float maxY;        // 0x10: same, upper y bound.
    float cursorX;     // 0x14: Layout_GetCursorPosition / LayoutCursor_InitBounds.
    float cursorY;     // 0x18: Layout_GetCursorPosition / LayoutCursor_InitBounds.
    float cursorDepth; // 0x1C: Layout_GetCursorDepth (0x80077C48).
};

#define OFF(f, o) typedef char LayoutCursor800779EC_off_##f[((int)&((LayoutPointerCursor*)0)->f == (o)) ? 1 : -1]
OFF(player, 0x0);
OFF(minX, 0x4);
OFF(minY, 0x8);
OFF(maxX, 0xC);
OFF(maxY, 0x10);
OFF(cursorX, 0x14);
OFF(cursorY, 0x18);
OFF(cursorDepth, 0x1C);
#undef OFF
#define AsLayoutCursor800779EC(p) ((LayoutPointerCursor*)(p))

#endif
