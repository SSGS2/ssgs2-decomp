// CBuildProductCharaFields: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_CBUILDPRODUCTCHARAFIELDS_H
#define CLASSES_CBUILDPRODUCTCHARAFIELDS_H

class CBuildProductCharaFields {
public:
    void* vtbl; // 0x0: CBuildProductChara_Ctor, CBuildProductChara_dtor
    union {
        void* products_ptr; // 0x4: CBuildProductChara_BuildProducts
        void** products_pp; // 0x4: CBuildProductChara_BuildProducts
    };
    unsigned int productCount; // 0x8: CBuildProductChara_BuildProducts
    unsigned int productCapacity; // 0xc: CBuildProductChara_BuildProducts
    union {
        unsigned int recordArrays_u32; // 0x10: CBuildProductCaddie_LoadIconGroups, CBuildProductChara_LoadProductTextures
        int* recordArrays_s32ptr; // 0x10: CBuildProductChara_FindResourceByTypeId
        void* recordArrays_ptr; // 0x10: CBuildProductChara_BuildProducts
    };
    union {
        int recordCountsAddress_s32; // 0x14: CBuildProductCaddie_LoadIconGroups
        void** recordCounts_pp; // 0x14: CBuildProductCaddie_LoadIconGroups
        unsigned int recordCounts_u32; // 0x14: CBuildProductCaddie_LoadIconGroups, CBuildProductChara_LoadProductTextures
        int* recordCounts_s32ptr; // 0x14: CBuildProductChara_FindResourceByTypeId
        void* recordCounts_ptr; // 0x14: CBuildProductChara_BuildProducts, CBuildProductChara_LoadProductTextures
    };
    union {
        void* resourceArrays_ptr; // 0x18: CBuildProductCaddie_LoadIconGroups, CBuildProductChara_LoadProductTextures
        void** resourceArrays_pp; // 0x18: CBuildProductCaddie_LoadIconGroups, CBuildProductChara_FindResourceByTypeId
        unsigned int resourceArrays_u32; // 0x18: CBuildProductCaddie_LoadIconGroups, CBuildProductChara_LoadProductTextures
    };
    char pad_01C[0x4];
    unsigned char flag; // 0x20: CBuildProductCaddie_LoadIconGroups, CBuildProductChara_BuildProducts, CBuildProductChara_LoadProductTextures
    char pad_021[0x2d4f];
    unsigned char textureEntriesBuilt; // 0x2d70: CBuildProductCaddie_LoadIconGroups, CBuildProductChara_BuildProducts, CBuildProductChara_LoadProductTextures
    char pad_2D71[0x3];
    union {
        void* textureArchive_ptr; // 0x2d74: CBuildProductChara_dtor, CBuildProductChara_UpdateTextureArchiveLoad, CBuildProductChara_LoadProductTextures
        int texture_s32; // 0x2d74: CBuildProductChara_Ctor
    };
    union {
        void* resourceNames_ptr; // 0x2d78: CBuildProductChara_Ctor, CBuildProductChara_dtor, CBuildProductChara_LoadProductTextures
        char** resourceNames_strings; // 0x2d78: CBuildProductChara_LoadProductTextures
    };
    union {
        int baseProduct_s32; // 0x2d7c: CBuildProductChara_Ctor
        void* baseProduct_ptr; // 0x2d7c: CBuildProductChara_dtor, CBuildProductChara_BuildProducts
    };
    union {
        int extraRecords_s32; // 0x2d80: CBuildProductChara_Ctor
        void* extraRecords_ptr; // 0x2d80: CBuildProductChara_dtor, CBuildProductChara_BuildProducts, CBuildProductChara_FindSetContainingItem
    };
    union {
        int extraRecordCount_s32; // 0x2d84: CBuildProductChara_Ctor, CBuildProductChara_BuildProducts, CBuildProductChara_FindSetContainingItem
        unsigned int extraRecordCount_u32; // 0x2d84: CBuildProductChara_BuildProducts
    };
};

#define OFF(f, o) typedef char CBuildProductCharaFields_off_##f[((int)&((CBuildProductCharaFields*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(products_ptr, 0x4);
OFF(products_pp, 0x4);
OFF(productCount, 0x8);
OFF(productCapacity, 0xc);
OFF(recordArrays_u32, 0x10);
OFF(recordArrays_s32ptr, 0x10);
OFF(recordArrays_ptr, 0x10);
OFF(recordCountsAddress_s32, 0x14);
OFF(recordCounts_pp, 0x14);
OFF(recordCounts_u32, 0x14);
OFF(recordCounts_s32ptr, 0x14);
OFF(recordCounts_ptr, 0x14);
OFF(resourceArrays_ptr, 0x18);
OFF(resourceArrays_pp, 0x18);
OFF(resourceArrays_u32, 0x18);
OFF(flag, 0x20);
OFF(textureEntriesBuilt, 0x2d70);
OFF(textureArchive_ptr, 0x2d74);
OFF(texture_s32, 0x2d74);
OFF(resourceNames_ptr, 0x2d78);
OFF(resourceNames_strings, 0x2d78);
OFF(baseProduct_s32, 0x2d7c);
OFF(baseProduct_ptr, 0x2d7c);
OFF(extraRecords_s32, 0x2d80);
OFF(extraRecords_ptr, 0x2d80);
OFF(extraRecordCount_s32, 0x2d84);
OFF(extraRecordCount_u32, 0x2d84);
#undef OFF

#endif
