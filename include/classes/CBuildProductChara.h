// CBuildProductChara: the character product builder. It holds the eight category
// providers, the five flattened record and icon texture groups, the character's
// fashion archive, and the costume-set records.
//
// Its base layout is the same as CBuildProductCharaFields, which the caddie builder
// still reads. The group tables at +10, +14 and +18 share group numbers but hold
// different things: record arrays, record counts and icon texture arrays.
#ifndef CLASSES_CBUILDPRODUCTCHARA_H
#define CLASSES_CBUILDPRODUCTCHARA_H

class CBuildProductChara {
public:
    CBuildProductChara(int count, unsigned char flag);

    // Destroy the archive, providers, name table and set records; optionally free the object.
    void* Destroy(int flag);
    // Advance the character texture archive's asynchronous load.
    int UpdateTextureArchiveLoad();
    // Find the icon texture for an item ID in groups 1..4.
    void* FindResourceByTypeId(void* itemTypeId);
    // Find the costume set that contains an item ID.
    void* FindSetContainingItem(void* itemTypeId);
    // Select the fashion archive for a character and rebuild its products and icons.
    int LoadProductTextures(int characterIndex);
    // Rebuild the category providers and the five record groups for a character.
    int BuildProducts(int characterIndex);
    // Set the character-product angle conversion constant at startup.
    static void InitAngleConstant();

    void* vtbl; // 0x00
    union {
        void* products_ptr; // 0x04
        void** products_pp; // 0x04
    };
    unsigned int productCount;    // 0x08
    unsigned int productCapacity; // 0x0C
    union {
        unsigned int recordArrays_u32; // 0x10
        int* recordArrays_s32ptr;      // 0x10
        void* recordArrays_ptr;        // 0x10
    };
    union {
        int recordCountsAddress_s32;   // 0x14
        void** recordCounts_pp;        // 0x14
        unsigned int recordCounts_u32; // 0x14
        int* recordCounts_s32ptr;      // 0x14
        void* recordCounts_ptr;        // 0x14
    };
    union {
        void* resourceArrays_ptr;        // 0x18
        void** resourceArrays_pp;        // 0x18
        unsigned int resourceArrays_u32; // 0x18
    };
    char pad_01C[0x4];                 // 0x1C
    unsigned char flag;                // 0x20
    char pad_021[0x2D4F];              // 0x21
    unsigned char textureEntriesBuilt; // 0x2D70
    char pad_2D71[0x3];                // 0x2D71
    union {
        void* textureArchive_ptr; // 0x2D74
        int texture_s32;          // 0x2D74
    };
    union {
        void* resourceNames_ptr;      // 0x2D78
        char** resourceNames_strings; // 0x2D78
    };
    union {
        int baseProduct_s32;   // 0x2D7C
        void* baseProduct_ptr; // 0x2D7C
    };
    union {
        int extraRecords_s32;   // 0x2D80
        void* extraRecords_ptr; // 0x2D80
    };
    union {
        int extraRecordCount_s32;          // 0x2D84
        unsigned int extraRecordCount_u32; // 0x2D84
    };
};

typedef char CBuildProductChara_size[(sizeof(CBuildProductChara) == 0x2D88) ? 1 : -1];
typedef char CBuildProductChara_off_flag[((int)&((CBuildProductChara*)0)->flag == 0x20) ? 1 : -1];
typedef char CBuildProductChara_off_textureEntriesBuilt[((int)&((CBuildProductChara*)0)->textureEntriesBuilt == 0x2D70)
                                                            ? 1
                                                            : -1];
typedef char
    CBuildProductChara_off_textureArchive[((int)&((CBuildProductChara*)0)->textureArchive_ptr == 0x2D74) ? 1 : -1];
typedef char
    CBuildProductChara_off_resourceNames[((int)&((CBuildProductChara*)0)->resourceNames_ptr == 0x2D78) ? 1 : -1];
typedef char
    CBuildProductChara_off_extraRecordCount[((int)&((CBuildProductChara*)0)->extraRecordCount_s32 == 0x2D84) ? 1 : -1];

#endif // CLASSES_CBUILDPRODUCTCHARA_H
