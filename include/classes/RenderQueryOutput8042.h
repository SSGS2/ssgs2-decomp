// Layout owned by gtl_graphics_texture_buffer_resources.cpp. Texture_GetKind4LevelDescription (0x8042BF9C) writes all nine words.
// Existing field names are retained; unknown result words keep their offsets.
#ifndef CLASSES_RENDERQUERYOUTPUT8042_H
#define CLASSES_RENDERQUERYOUTPUT8042_H

struct Fn8042Output {
    unsigned int field00;
    unsigned int field04;
    unsigned int usage;
    unsigned int unk_C;
    unsigned int storageBytes;
    unsigned int unk_14;
    unsigned int unk_18;
    unsigned int width;
    unsigned int height;
};

#define OFF(f, o) typedef char Fn8042Output_off_##f[((int)&((Fn8042Output*)0)->f == (o)) ? 1 : -1]
OFF(field00, 0x0);
OFF(field04, 0x4);
OFF(usage, 0x8);
OFF(unk_C, 0xc);
OFF(storageBytes, 0x10);
OFF(unk_14, 0x14);
OFF(unk_18, 0x18);
OFF(width, 0x1c);
OFF(height, 0x20);
#undef OFF

#endif
