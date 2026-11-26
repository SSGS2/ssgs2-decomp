// FontTreeLinks80250: layout view owned by font_num_draw_entries.cpp.
// Evidence: CFontNumDraw_dtor; FontNumDrawEntries_DestroyTree; CFontNumDraw_CreateNumericEntry recursively release left/right children. No virtual declarations or runtime objects.
#ifndef CLASSES_FONTTREELINKS80250_H
#define CLASSES_FONTTREELINKS80250_H

class FontTreeLinks80250 {
public:
    void* left;  // 0x0; CFontNumDraw_dtor
    void* right; // 0x4; CFontNumDraw_dtor
    char pad_8[0x4];
    int key;     // 0xC; CFontNumDraw_dtor
    void* value; // 0x10; CFontNumDraw_dtor
};

#define OFF(f, o) typedef char FontTreeLinks80250_off_##f[((int)&((FontTreeLinks80250*)0)->f == (o)) ? 1 : -1]
OFF(left, 0x0);
OFF(right, 0x4);
OFF(key, 0xC);
OFF(value, 0x10);
#undef OFF

#endif
