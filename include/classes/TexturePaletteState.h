// Non-virtual layout view; TexturePalette_Flush / GetDescription in gtl_texture_vertex_formats.cpp.
// Flush selects entry count from +0xC, storage size from +0x10, and flushes +4.
#ifndef CLASSES_TEXTUREPALETTESTATE_H
#define CLASSES_TEXTUREPALETTESTATE_H
struct TexturePaletteState {
    unsigned int flags; // 0x0: GetDescription extracts bits 13..15
    void* data;         // 0x4: DCFlushRange argument
    char pad_008[4];
    int paletteType;   // 0xC: TexturePalette_GetEntryCountForType input
    int textureFormat; // 0x10: Texture_CalculateMipStorageSize format input
};
#define OFF(f, o) typedef char TexturePaletteState_off_##f[((int)&((TexturePaletteState*)0)->f == (o)) ? 1 : -1]
OFF(flags, 0x0);
OFF(data, 0x4);
OFF(paletteType, 0xC);
OFF(textureFormat, 0x10);
#undef OFF
#endif
