// Layout views for Texture_BuildMipLayout / TextureDescription_Initialize in gtl_texture_vertex_formats.cpp.
// No constructors, inheritance, virtual declarations, or runtime data.
#ifndef CLASSES_TEXTUREMIPLAYOUT8042_H
#define CLASSES_TEXTUREMIPLAYOUT8042_H
struct TextureMipLevel8042 {
    int width; // Texture_BuildMipLayout: max(base width >> level, 1).
    int height; // max(base height >> level, 1).
    int dataOffset; // sum of the preceding levels' byte sizes.
    int byteSize; // Texture_CalculateMipStorageSize(width, height, 1, format).
};
struct TextureMipLayout8042 {
    char pad_000[0xC];
    int levelCount; // 0xC: incoming level count, or 8 when zero.
    int descriptorValue; // 0x10: saved incoming argument; meaning unresolved.
    TextureMipLevel8042 levels[8]; // 0x14: Texture_BuildMipLayout clears 0x80 bytes.
    int format; // 0x94: forwarded to Texture_CalculateMipStorageSize.
};
struct TextureDescription8042 {
    char pad_000[4];
    int storageArgument; // TextureDescription_Initialize forwards this argument to the size calculator.
    char pad_008[4];
    int width; // 0xC: first size-calculator argument.
    int height; // 0x10: second size-calculator argument.
    int format; // 0x14: fourth size-calculator argument.
    int sizeArgument; // forwarded to the size calculator; meaning unresolved.
    int unk_1C; // cleared by TextureDescription_Initialize.
};
#define OFF(f, o) typedef char TextureMipLevel8042_off_##f[((int)&((TextureMipLevel8042*)0)->f == (o)) ? 1 : -1]
OFF(width, 0x0);
OFF(height, 0x4);
OFF(dataOffset, 0x8);
OFF(byteSize, 0xC);
#undef OFF
typedef char TextureMipLevel8042_size[(sizeof(TextureMipLevel8042) == 0x10) ? 1 : -1];
#define OFF(f, o) typedef char TextureMipLayout8042_off_##f[((int)&((TextureMipLayout8042*)0)->f == (o)) ? 1 : -1]
OFF(levelCount, 0xC);
OFF(descriptorValue, 0x10);
OFF(levels, 0x14);
OFF(format, 0x94);
#undef OFF
#define OFF(f, o) typedef char TextureDescription8042_off_##f[((int)&((TextureDescription8042*)0)->f == (o)) ? 1 : -1]
OFF(storageArgument, 0x4);
OFF(width, 0xC);
OFF(height, 0x10);
OFF(format, 0x14);
OFF(sizeArgument, 0x18);
OFF(unk_1C, 0x1C);
#undef OFF
#endif
