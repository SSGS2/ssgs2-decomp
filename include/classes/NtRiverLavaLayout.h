// ntRiver_Lava: methods owned by river_lava_layout.cpp. Separate from CRiver's layout.
// RiverLava_InitializeTextureLayers initializes two layers; vfunc_1 builds strips; vfunc_2 scrolls V;
// vfunc_3 (0x801E5A08) fills and draws the two vertex buffers.
#ifndef CLASSES_NTRIVERLAVALAYOUT_H
#define CLASSES_NTRIVERLAVALAYOUT_H
struct NtRiverLavaLayer {
    float vOffset;         // layer+0x0: vfunc_2 subtracts delta * vScrollSpeed
    float vScrollSpeed;    // layer+0x4
    void* bitmap;          // layer+0x8: RiverLava_InitializeTextureLayers loads bitmap; vfunc_3 binds texture
    float vScale;          // layer+0xC: vfunc_1 multiplies segment length
    float uExtent;         // layer+0x10: vfunc_1 writes vertex U
    float heightOffset;    // layer+0x14: vfunc_1 adds to strip height
    unsigned char enabled; // layer+0x18: vfunc_1/2/3 gate layer work
    char pad_19[3];
};
struct NtRiverLavaLayout {
    char pad_00[8];
    NtRiverLavaLayer layers[2]; // 0x8: layer stride 0x1C
    void* stripPoints;          // 0x40: vfunc_1 allocates 0x20-byte StripPoint entries
    unsigned int stripCount;    // 0x44: vfunc_1 appends; vfunc_3 computes primitive count
    unsigned int stripCapacity; // 0x48: vfunc_1 grows allocation
    char pad_4C[0x10];
    void* materialVtable; // 0x5C: vfunc_3 invokes material subobject at +0x58
    char pad_60[4];
    void* vertexBuffers[2]; // 0x64: vfunc_1 creates, vfunc_3 flushes and draws
};
#define OFF(f, o) typedef char NtRiverLavaLayer_off_##f[((int)&((NtRiverLavaLayer*)0)->f == (o)) ? 1 : -1]
OFF(vOffset, 0x0);
OFF(vScrollSpeed, 0x4);
OFF(bitmap, 0x8);
OFF(vScale, 0xC);
OFF(uExtent, 0x10);
OFF(heightOffset, 0x14);
OFF(enabled, 0x18);
#undef OFF
#define OFF(f, o) typedef char NtRiverLavaLayout_off_##f[((int)&((NtRiverLavaLayout*)0)->f == (o)) ? 1 : -1]
OFF(layers, 0x8);
OFF(stripPoints, 0x40);
OFF(stripCount, 0x44);
OFF(stripCapacity, 0x48);
OFF(materialVtable, 0x5C);
OFF(vertexBuffers, 0x64);
#undef OFF
typedef char NtRiverLavaLayer_stride[(sizeof(NtRiverLavaLayer) == 0x1C) ? 1 : -1];
#define AsNtRiverLava(p) ((NtRiverLavaLayout*)(p))
#endif
