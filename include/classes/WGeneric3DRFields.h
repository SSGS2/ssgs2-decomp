// WGeneric3DRFields: layout view owned by methods in wgeneric3dr_renderer_core.cpp: WGeneric3DR_ctor/Initialize/vfunc_32 initialize these offsets; vfunc_10 releases resources. resourceName is allocated then filled by strcpy and append in ctor.
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_WGENERIC3DRFIELDS_H
#define CLASSES_WGENERIC3DRFIELDS_H

struct WGeneric3DRFields {
    char pad_000[0x4868];
    int initialized; // 0x4868
    int unk_486C;    // 0x486C
    char pad_4870[0x8];
    int windowFormatFlags; // 0x4878
    char pad_487C[0xC];
    int unk_4888; // 0x4888
    char pad_488C[0x38];
    int defaultTextureWidth;  // 0x48C4
    int defaultTextureHeight; // 0x48C8
    char pad_48CC[0x34];
    int unk_4900; // 0x4900
    int unk_4904; // 0x4904
    char pad_4908[0xC];
    int unk_4914;   // 0x4914
    int unk_4918;   // 0x4918
    float unk_491C; // 0x491C
    char pad_4920[0x10];
    int unk_4930; // 0x4930
    char pad_4934[0x4];
    int unk_4938; // 0x4938
    char pad_493C[0x4];
    int boundTextureIndices; // 0x4940
    int cachedRenderFlags;   // 0x4944
    int cachedBlendMode;     // 0x4948
    int drawMode;            // 0x494C
    int cachedDrawSettings;  // 0x4950
    int retainedResource0;   // 0x4954
    int retainedResource1;   // 0x4958
    int retainedResource2;   // 0x495C
    int retainedResource3;   // 0x4960
    char pad_4964[0x38];
    int cachedVertexFormat; // 0x499C
    char pad_49A0[0x4];
    int entryHelper;                  // 0x49A4
    int textureResource;              // 0x49A8
    int unk_49AC;                     // 0x49AC
    void* entryResource;              // 0x49B0
    int unk_49B4;                     // 0x49B4
    int unk_49B8;                     // 0x49B8
    int entryResourceExternallyOwned; // 0x49BC
    int unk_49C0;                     // 0x49C0
    int unk_49C4;                     // 0x49C4
    int unk_49C8;                     // 0x49C8
    int unk_49CC;                     // 0x49CC
    char pad_49D0[0x15FD4];
    int unk_1A9A4;            // 0x1A9A4
    void* resourceName;       // 0x1A9A8
    int factoryIndex;         // 0x1A9AC
    int factoryContext;       // 0x1A9B0
    void* presentationTarget; // 0x1A9B4
    int unk_1A9B8;            // 0x1A9B8
    int unk_1A9BC;            // 0x1A9BC
    char pad_1A9C0[0x14];
    int unk_1A9D4; // 0x1A9D4
    char pad_1A9D8[0x18];
    float unk_1A9F0; // 0x1A9F0
    float unk_1A9F4; // 0x1A9F4
    float unk_1A9F8; // 0x1A9F8
    float unk_1A9FC; // 0x1A9FC
    float unk_1AA00; // 0x1AA00
    float unk_1AA04; // 0x1AA04
    float unk_1AA08; // 0x1AA08
    float unk_1AA0C; // 0x1AA0C
    char pad_1AA10[0x24];
    int unk_1AA34;  // 0x1AA34
    int unk_1AA38;  // 0x1AA38
    char unk_1AA3C; // 0x1AA3C
    char pad_1AA3D[0x3];
    int unk_1AA40; // 0x1AA40
    int unk_1AA44; // 0x1AA44
    int unk_1AA48; // 0x1AA48
};

#define OFF(f, o) typedef char WGeneric3DRFields_off_##f[((int)&((WGeneric3DRFields*)0)->f == (o)) ? 1 : -1]
OFF(initialized, 0x4868);
OFF(unk_486C, 0x486C);
OFF(windowFormatFlags, 0x4878);
OFF(unk_4888, 0x4888);
OFF(defaultTextureWidth, 0x48C4);
OFF(defaultTextureHeight, 0x48C8);
OFF(unk_4900, 0x4900);
OFF(unk_4904, 0x4904);
OFF(unk_4914, 0x4914);
OFF(unk_4918, 0x4918);
OFF(unk_491C, 0x491C);
OFF(unk_4930, 0x4930);
OFF(unk_4938, 0x4938);
OFF(boundTextureIndices, 0x4940);
OFF(cachedRenderFlags, 0x4944);
OFF(cachedBlendMode, 0x4948);
OFF(drawMode, 0x494C);
OFF(cachedDrawSettings, 0x4950);
OFF(retainedResource0, 0x4954);
OFF(retainedResource1, 0x4958);
OFF(retainedResource2, 0x495C);
OFF(retainedResource3, 0x4960);
OFF(cachedVertexFormat, 0x499C);
OFF(entryHelper, 0x49A4);
OFF(textureResource, 0x49A8);
OFF(unk_49AC, 0x49AC);
OFF(entryResource, 0x49B0);
OFF(unk_49B4, 0x49B4);
OFF(unk_49B8, 0x49B8);
OFF(entryResourceExternallyOwned, 0x49BC);
OFF(unk_49C0, 0x49C0);
OFF(unk_49C4, 0x49C4);
OFF(unk_49C8, 0x49C8);
OFF(unk_49CC, 0x49CC);
OFF(unk_1A9A4, 0x1A9A4);
OFF(resourceName, 0x1A9A8);
OFF(factoryIndex, 0x1A9AC);
OFF(factoryContext, 0x1A9B0);
OFF(presentationTarget, 0x1A9B4);
OFF(unk_1A9B8, 0x1A9B8);
OFF(unk_1A9BC, 0x1A9BC);
OFF(unk_1A9D4, 0x1A9D4);
OFF(unk_1A9F0, 0x1A9F0);
OFF(unk_1A9F4, 0x1A9F4);
OFF(unk_1A9F8, 0x1A9F8);
OFF(unk_1A9FC, 0x1A9FC);
OFF(unk_1AA00, 0x1AA00);
OFF(unk_1AA04, 0x1AA04);
OFF(unk_1AA08, 0x1AA08);
OFF(unk_1AA0C, 0x1AA0C);
OFF(unk_1AA34, 0x1AA34);
OFF(unk_1AA38, 0x1AA38);
OFF(unk_1AA3C, 0x1AA3C);
OFF(unk_1AA40, 0x1AA40);
OFF(unk_1AA44, 0x1AA44);
OFF(unk_1AA48, 0x1AA48);
#undef OFF
#define AsWGeneric3DRFields(p) ((WGeneric3DRFields*)(p))

#endif
