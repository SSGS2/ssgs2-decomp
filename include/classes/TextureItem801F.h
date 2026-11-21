// TextureItem801F: layout view of Item801F used by TextureItemQueue_ReplaceFront and TextureItemList_ClearAndRelease.
// Layout-only view; no virtual declarations or generated RTTI.
#ifndef CLASSES_TEXTUREITEM801F_H
#define CLASSES_TEXTUREITEM801F_H

class TextureItem801F {
public:
    int unk_0; // 0x0: TextureItemQueue_ReplaceFront clears before destroying the item
    void* data; // 0x4: TextureItemList_ClearAndRelease frees after clearing dataCount
    int dataCount; // 0x8: TextureItemList_ClearAndRelease resets before freeing data
    void* dataCapacity; // 0xC: Item801F declaration at 0x801EDA60 unit
    void** resources; // 0x10: TextureItemList_ClearAndRelease iterates and releases referenced texture resources
    int resourceCount; // 0x14: TextureItemList_ClearAndRelease bounds the resource iteration
    void* resourceCapacity; // 0x18: Item801F declaration at 0x801EDA60 unit
};

#define OFF(f, o) typedef char TextureItem801F_off_##f[((int)&((TextureItem801F*)0)->f == (o)) ? 1 : -1]
OFF(unk_0, 0x0);
OFF(data, 0x4);
OFF(dataCount, 0x8);
OFF(dataCapacity, 0xC);
OFF(resources, 0x10);
OFF(resourceCount, 0x14);
OFF(resourceCapacity, 0x18);
#undef OFF

#endif
