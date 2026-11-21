// WFont: partial non-virtual layout view, with no generated vtable or RTTI.
// Evidence: WFont_dtor and WTitleFont_dtor in font_markup_text_rendering.cpp set the WFont
// vtable and pass the pointer at +0x10 to Object_DeleteVirtual for each resource.
#ifndef CLASSES_WFONT_H
#define CLASSES_WFONT_H

class WFont {
public:
    void* vtbl; // 0x00: __vt__5WFont
    char pad_004[0xC];
    void* resourceOwner; // 0x10: Object_DeleteVirtual receiver
    char pad_014[0x14];
    int coordMode; // 0x28: WFont_SetCoordMode (0x80219ED0) stores supplied flags OR 0x400
};

#define WFONT_OFF(f, o) typedef char WFont_off_##f[((int)&((WFont*)0)->f == (o)) ? 1 : -1]
WFONT_OFF(vtbl, 0x00);
WFONT_OFF(resourceOwner, 0x10);
WFONT_OFF(coordMode, 0x28);
#undef WFONT_OFF

#endif
