// FontAtlas: layout view; FontAtlas_LoadPages lives in font_atlas_page_loading.cpp.
// No inheritance, virtual declarations, constructors, or emitted code.
#ifndef CLASSES_FONTATLAS_H
#define CLASSES_FONTATLAS_H

struct FontAtlas {
    char pad_000[4];
    void* fontBuffer; // 0x4: FontAtlas_LoadPages supplies this buffer to gtl_FontData_InitView
    void* fontData; // 0x8: FontAtlas_LoadPages stores reader; FontAtlas_GetFontData in font_atlas_and_mwstring.cpp returns it
    char pages[0xC]; // 0xC: FontAtlas_LoadPages appends page pairs through FontAtlasPageVector_PushBack
    int lineHeight; // 0x18: FontAtlas_LoadPages copies reader result; FontAtlas_GetLineHeight returns this word
};

#define OFF(f, o) typedef char FontAtlas_off_##f[((int)&((FontAtlas*)0)->f == (o)) ? 1 : -1]
OFF(fontBuffer, 0x4);
OFF(fontData, 0x8);
OFF(pages, 0xC);
OFF(lineHeight, 0x18);
#undef OFF
#define AsFontAtlas(p) ((FontAtlas*)(p))

#endif
