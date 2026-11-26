// Layout view for the receiver of TourAreaSelect_Destroy / TourAreaSelect_PollResourcesAndCreateUi / TourAreaSelect_StartDisplayAndFadeOut.
// Game class name unresolved; no inheritance or virtual methods.
#ifndef CLASSES_OBJECT802F9B40_H
#define CLASSES_OBJECT802F9B40_H
class Object802F9B40 {
public:
    char pad_000[4];
    int bpEntryIds
        [2]; // 0x4: TourAreaSelect_PollResourcesAndCreateUi creates two entries; TourAreaSelect_Destroy deletes them.
    char pad_00C[4];
    int bpEntryCount; // 0x10: loop bound; TourAreaSelect_PollResourcesAndCreateUi sets 2.
    char pad_014[4];
    void* primaryLayout;   // 0x18: Layout_LoadExtended / Layout_Destroy.
    void* secondaryLayout; // 0x1C: Layout_LoadExtended / Layout_Destroy.
    void* packedFile;      // 0x20: CPackedFile_PollLoad / CPackedFile_IsLoadFinished; class identity unresolved.
    void* textureResource; // 0x24: CPackTextureResource_PollLoad / CPackTextureResource_IsLoadFinished.
    char pad_028[8];
    void* treeRoot; // 0x30: TourAreaSelect_Destroy recursively frees this binary tree.
};
#define OFF(f, o) typedef char Object802F9B40_off_##f[((int)&((Object802F9B40*)0)->f == (o)) ? 1 : -1]
OFF(bpEntryIds, 0x4);
OFF(bpEntryCount, 0x10);
OFF(primaryLayout, 0x18);
OFF(secondaryLayout, 0x1C);
OFF(packedFile, 0x20);
OFF(textureResource, 0x24);
OFF(treeRoot, 0x30);
#undef OFF
#define AsObject802F9B40(p) ((Object802F9B40*)(p))
#endif
