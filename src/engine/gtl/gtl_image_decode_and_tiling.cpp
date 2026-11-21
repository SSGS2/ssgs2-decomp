// Texture resource decoding: GSR chunk lookup, GIM/PICT descriptions and Wii tiling.
// Used by gtl_Image_GetDescription/GetImageData/GetPaletteData (gtl_image_format_dispatch.cpp), then
// Texture_LoadResourceWithOptionalPalette (engine_render_cache_and_texture_load.cpp); this file never uploads GX data.
// Mod lookup:
// - GSR dimensions/format: GetDescription reads tex[0] (+4/+8 pixels, +0 format).
// - Palette count/format: pal[0] (+4 count, +0 format); no pal => {-1,0}.
// - Pixel/palette offsets: GetImageData uses tex+0x14 into texd; GetPaletteData
//   uses pal+8 into pald. These return borrowed addresses inside the encoded file.
// - Wii block geometry: GetTiledOffset and Tile* below; preserve format, stride,
//   dimensions and allocation padding together. Larger assets need larger buffers.
// Resource schema (32-bit target, native loads; no endian conversion in GSR paths):
// file+0x14 = chunk count; +0x20 = directory of 0x10-byte entries
// {tag,count,stride,fileRelativeOffset}. Tags below are loaded numeric words,
// not C strings. In particular 0x00786574 is the serialized tex tag.
// All pointers are required unless a function documents a null case. Buffers are
// borrowed, suitably aligned and already validated by the caller: probes check
// signatures only, not file length. There is no general malformed-file safety.
// No allocation/retention in parsing/tiling. Outputs must not overlap source data.
// Opaque header/description words retain their ABI names.
// Original function range: 0x803D3044-0x803D4098 (merged decoder units).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
#include <types.h>
#include "sdk_protos.h"

extern "C" {

void* memset(void* dst, int c, u32 n);
void* memcpy(void* dst, const void* src, u32 n);
void __dl__FPv(void* p);

s32 gtl_ImagePICT_IsValid(u8* arg0);
s32 gtl_ImageDDS_IsValid(u8* arg0);

#pragma dont_inline on

#pragma dont_inline off

#pragma push
#pragma use_lmw_stmw on

#pragma pop

// The second argument selects a platform format table, NOT bits per pixel.
s32 gtl_ImageFormat_FromPlatformFormat(u32 format, s32 platformLayout);
enum ResourceImageConstants {
    kResourceDirectoryOffset = 0x20, kResourceDirectoryEntryBytes = 0x10,
    kResourceDirectoryEntryShift = 4, // index*16; keep shift arithmetic in resolver
    // Engine format IDs, distinct from the platform selector and GX format enum.
    // FromSwizzled (gtl_image_format_dispatch.cpp) maps the tiled member of each pair to linear.
    kImageIndexed8Linear = 0x1E, kImageIndexed8Tiled = 0x58,
    kImageIndexed4Linear = 0x1F, kImageIndexed4Tiled = 0x59,
    kImageFourBitLinear = 0x21, kImageFourBitTiled = 0x5B,
    kImageBlock4Linear = 0x2E, kImageBlock4Tiled = 0x62,
    kImageBlock8LinearA = 0x2F, kImageBlock8TiledA = 0x63,
    kImageBlock8LinearB = 0x30, kImageBlock8TiledB = 0x64,
    kImageBlock8LinearC = 0x31, kImageBlock8TiledC = 0x65,
    kImageBlock8LinearD = 0x32, kImageBlock8TiledD = 0x66,
    kGsrTextureDescriptionTag = 0x00786574, kGsrPaletteDescriptionTag = 0x006C6170,
    kGsrTextureDataTag = 0x64786574, kGsrPaletteDataTag = 0x646C6170,
    kGsrSignature = 0x00727367, kNoImageFormat = -1,
    kGimImageChunk = 4, kGimPaletteChunk = 5, kGimPlatformLayout = 8,
    kGsrType2Layout = 4, kGsrType3Layout = 6, kGsrType5Layout = 7,
    kGsrType1Layout = 5, kGsrType4Layout = 3,
    kIndexed8PaletteEntries = 256, kIndexed4PaletteEntries = 16,
    kCompressedBlockPixels = 4, kCompressedColorBlockBytes = 8,
    kCompressedExtendedBlockBytes = 16,
    kRgbaPlaneSeparationBytes = 0x20,
    kRgbaBlueByteOffset = kRgbaPlaneSeparationBytes + 1,
    kPictSignature = 0x34F68053, kPictTag = 0x54434950,
    kBmpSignature = 0x4D42, kTim2Signature = 0x324D4954,
    kGimSignature = 0x2E47494D, kGimVersion = 0x312E3030, kGimPlatformTag = 0x00505350
};
s32 gtl_ImageFormat_FromSwizzled(s32 format);
s32 gtl_ImageFormat_IsPaletted(s32 format);

typedef struct GimHeader {
    char unk0[8];
    u32 childChunkOffset;
} GimHeader;

typedef struct GimChunk {
    u16 type;
    char reserved2[2];
    u32 size;
    char unk8[4];
    u32 child;
} GimChunk;

typedef struct GimEntry {
    char unk0[4];
    u16 format;
    u16 flag;
    u16 width;
    u16 height;
    char unkC[0x1E];
    // Native-endian decoded GIM image entry +0x2A: mip levels INCLUDING level 0.
    // Forwarded unchanged to the generic description, not a last-level index.
    u16 mipLevelCount;
} GimEntry;

typedef struct GimDescription {
    s32 format;
    s32 width;
    s32 height;
    // Generic description +0x0C, consumed by Texture_CreateAndUploadMipChain
    // (gx_render_state_and_texture_upload.cpp): Texture_Create2D(...,count,...) and a count-iteration upload.
    s32 mipLevelCount;
} GimDescription;

typedef struct GimPaletteDescription {
    s32 format;
    s32 entryCount;
} GimPaletteDescription;

#pragma push
#pragma use_lmw_stmw on

#pragma pop

s32 gtl_ImageFormat_GetBytesPerElement(s32 format);
void gtl_Image_Tile4BitPixels(u8* arg0, u8* arg1, u32 arg2, u32 arg3, s32 arg4);
void gtl_Image_Tile8Or16BitPixels(u8* arg0, u8* arg1, u32 arg2, u32 arg3, s32 arg4);

// Classify the two engine compressed-format families; unknown IDs return false.
static inline s32 IsCompressedFormat(s32 f) {
    switch (f) {
    case kImageBlock4Linear:
    case kImageBlock8LinearA:
    case kImageBlock8LinearB:
    case kImageBlock8LinearC:
    case kImageBlock8LinearD:
    case kImageBlock4Tiled:
    case kImageBlock8TiledA:
    case kImageBlock8TiledB:
    case kImageBlock8TiledC:
    case kImageBlock8TiledD:
        return 1;
    default:
        return 0;
    }
}

// Return stored bits/pixel for engine format IDs (NOT a GX/platform-format enum).
// Closed lookup domain from GetBytesPerElement in gtl_image_layout_and_sizing.cpp, plus special cases:
// 4 bits: 0x1F/21/59/5B and 0x2E/62;
// 8 bits: 0x2F..32/63..66 and ordinary byte formats 0x1C/1E/22/25/48/58/5C/5F;
// 16 bits: 0x05/08/14/15/24/2C/37/46/50/52/53/60/6B;
// 24 bits: 0x00/0D/3E/4B;
// 32 bits: 0x01/02/0E/0F/10/11/34/36/38/3F/40/4C/4D/4E/4F/68/6A/6C;
// 64 bits: 0x19. Sentinel -1 and other unsupported IDs return zero; zero is not
// a usable pixel stride or proof that the caller can process that format.
// This is a STORAGE-size query, not admission to LinearToTiled: that converter's
// ordinary fallback requires four-byte pixels; 24/64-bit entries are not safe
// merely because their sizes are known. Compressed paths have narrower rules.
static inline u32 GetFormatBits(s32 f) {
    u32 bits;
    switch (f) {
    case kImageIndexed4Linear:
    case kImageFourBitLinear:
    case kImageIndexed4Tiled:
    case kImageFourBitTiled:
        bits = 4;
        break;
    case kImageBlock4Linear:
    case kImageBlock4Tiled:
        bits = 4;
        break;
    case kImageBlock8LinearA:
    case kImageBlock8LinearB:
    case kImageBlock8LinearC:
    case kImageBlock8LinearD:
    case kImageBlock8TiledA:
    case kImageBlock8TiledB:
    case kImageBlock8TiledC:
    case kImageBlock8TiledD:
        bits = 8;
        break;
    default:
        bits = gtl_ImageFormat_GetBytesPerElement(f) * 8;
        break;
    case -1:
        bits = 0;
        break;
    }
    return bits;
}

#pragma push
#pragma use_lmw_stmw on

#pragma pop


extern u32 s_resourceDataTypeByteStrides[];


typedef struct DtorData {
    void* buffer;
    s32 count;
} DtorData;


typedef struct DtorData2 {
    void* entries;
    unsigned int volatile count;
} DtorData2;


void* gtl_ResourceFile_GetChunkElement(void* file, void* data, u32 tag, u32 index);
void Memory_CopyAndReverseStridedBlocks(void* destination, void* source, unsigned int packedLayout, unsigned int rowStride, unsigned int rowCount);

typedef struct PictDescription {
    s32 format;
    s32 width;
    s32 height;
    s32 mipLevelCount;
    s32 sourcePlatformLayout;
} PictDescription;

// GSR byte layouts used by the functions below (native 32-bit target words).
// These schemas describe record-kind-specific meanings; GsrEntry is the legacy
// shared prefix view and its fields are kept as-is.
struct GsrTextureRecordSchema {
    u32 platformFormat;       // +00: interpreted using file type's platform table
    u32 widthPixels;          // +04
    u32 heightPixels;         // +08
    u32 mipLevelCount;        // +0C: forwarded to the generic upload description
    u32 unresolvedWord10;     // +10: not consumed by this unit
    u32 imageDataByteOffset;  // +14: relative to the texd chunk's element 0
};
struct GsrPaletteRecordSchema {
    u32 platformFormat;       // +00
    u32 entryCount;           // +04: not a byte count
    u32 paletteDataByteOffset;// +08: relative to pald element 0
};
typedef char GsrTextureSchemaSize[sizeof(GsrTextureRecordSchema) == 0x18 ? 1 : -1];
typedef char GsrPaletteSchemaSize[sizeof(GsrPaletteRecordSchema) == 0x0C ? 1 : -1];
// Header +0x0C is merely forwarded to an unused resolver argument here;
// +0x10 selects the platform-layout conversion table, not bits per pixel.
typedef struct GsrFile {
    char unk0[0xc];
    u32 forwardedDataWord;
    s32 fileType;
} GsrFile;


typedef struct GsrEntry {
    u32 format;
    u32 unk4;
    u32 unk8;
    u32 unkC;
} GsrEntry;

// Generic upload description: engine format, pixel width/height, mip count,
// then decoder-specific metadata. GSR writes its platform selector at +0x10;
// consumers such as gx_render_state_and_texture_upload.cpp use the first four words for mip-chain creation.
typedef struct GsrDescription {
    s32 format;
    s32 width;
    s32 height;
    s32 mipLevelCount;
    s32 sourcePlatformLayout;
} GsrDescription;

#pragma push
#pragma use_lmw_stmw on

#pragma pop


#pragma push
#pragma use_lmw_stmw on

#pragma pop

#pragma push
#pragma use_lmw_stmw on

#pragma pop

s32 gtl_ImageGIM_GetDescription(GimDescription* arg0, GimPaletteDescription* arg1, u8* arg2);
void* gtl_ResourceFile_GetChunkByIndex(void* arg0, s32 arg1, u32 arg2);
void* gtl_ResourceFile_FindChunk(void* file, void* data, u32 tag);
u32 gtl_ResourceFile_GetChunkElementCount(void* file, void* data, u32 tag);
void* gtl_ResourceFile_GetChunkElement(void* file, void* data, u32 tag, u32 index);
s32 gtl_ResourceFile_HasChunk(void* file, void* data, u32 tag);
u32 gtl_Resource_GetDataTypeByteStride(s32 arg0);
s32 gtl_ImageGSR_GetDescription(GsrDescription* out, GimPaletteDescription* paletteDesc, const GsrFile* file);
s32 gtl_ImageGSR_GetImageData(u32* out, const GsrFile* file);
s32 gtl_ImageGSR_GetPaletteData(u32* out, const GsrFile* file);
void* dtor_803D35A0(DtorData2* self, int dealloc);
void* dtor_803D361C(DtorData* this_, s32 arg1);
s32 gtl_ImagePICT_IsValid(u8* arg0);
s32 gtl_ImagePICT_GetDescription(PictDescription* desc, void* paletteDesc, u8* file);
s32 gtl_Image_GetTiledOffset(u32 arg0, u32 arg1, u32 arg2, u32 arg3);
void gtl_Image_ReorderCompressedBlocks(u8* arg0, u8* arg1, u32 arg2, u32 arg3, s32 arg4, s32 arg5);
s32 gtl_ImageTGA_IsValid(const u8* arg0);
s32 gtl_ImageBMP_IsValid(u8* arg0);
s32 gtl_ImageTIM2_IsValid(u8* arg0);
s32 gtl_ImageGSR_IsType2(u8* arg0);
s32 gtl_ImageGSR_IsType1(u8* arg0);
s32 gtl_ImageGSR_IsType3(u8* arg0);
s32 gtl_ImageGSR_IsType5(u8* arg0);
s32 gtl_ImageGSR_IsType4(u8* arg0);
s32 gtl_ImagePICT_Probe(u8* arg0);
s32 gtl_ImageGIM_IsValid(u8* arg0);
s32 gtl_ImageDDS_Probe(u8* arg0);
void gtl_Image_Tile4BitPixels(u8* arg0, u8* arg1, u32 arg2, u32 arg3, s32 arg4);
void gtl_Image_Tile8Or16BitPixels(u8* arg0, u8* arg1, u32 arg2, u32 arg3, s32 arg4);
void gtl_Image_LinearToTiled(u8* arg0, u32 arg1, u32 arg2, u8* arg3, u32 arg4, u32 arg5, u32 arg6, s32 arg7);

#pragma push

#pragma use_lmw_stmw on
// Walk trusted GIM chunks for image and optional palette metadata; always return 1.
// Both outputs are required. Missing required chunks have no bounded failure exit.
// Width/height are pixels. flag==1 applies FromSwizzled: tiled/swizzled engine
// format -> LINEAR counterpart (gtl_image_format_dispatch.cpp, e.g. 0x58->0x1E, 0x59->0x1F).
// This branch does not tile/untile pixel bytes and does not select tiled output.
// The flag's wider file-format meaning is not needed for this conversion. Unknown
// palette formats leave entry-count output unchanged. No file storage is retained.
// Entry+0x2A is widened into output.mipLevelCount without validation; upload in
// gx_render_state_and_texture_upload.cpp uses it to create and walk consecutive mip images, halving dimensions
// without a minimum-one clamp. Require >=1 complete levels and nonzero dimensions
// at every level. Changing this metadata alone cannot create the missing pixels.
// GIM fills four words only; a caller's possible fifth description word is untouched.
s32 gtl_ImageGIM_GetDescription(GimDescription* imageDescription, GimPaletteDescription* paletteDescription, u8* fileBytes) {
    GimEntry* imageEntry = 0;
    GimEntry* paletteEntry = 0;
    GimChunk* chunk;
    s32 isPaletted = 0;
    u8* rootChunk = fileBytes + 0x10;
    u8* pictureChunk = rootChunk + ((GimHeader*)rootChunk)->childChunkOffset;
    chunk = (GimChunk*)(pictureChunk + ((GimHeader*)pictureChunk)->childChunkOffset);

loop_1:
    if (imageEntry == 0 && (s32)chunk->type == kGimImageChunk) {
        imageEntry = (GimEntry*)((u8*)chunk + chunk->child);
        isPaletted = gtl_ImageFormat_IsPaletted(gtl_ImageFormat_FromPlatformFormat(imageEntry->format, kGimPlatformLayout));
    }
    if (paletteEntry == 0 && (s32)chunk->type == kGimPaletteChunk) {
        paletteEntry = (GimEntry*)((u8*)chunk + chunk->child);
    }
    if (imageEntry == 0 || (isPaletted != 0 && paletteEntry == 0)) {
        chunk = (GimChunk*)((u8*)chunk + chunk->size);
        goto loop_1;
    }

    imageDescription->width = imageEntry->width;
    imageDescription->height = imageEntry->height;
    imageDescription->format = gtl_ImageFormat_FromPlatformFormat(imageEntry->format, kGimPlatformLayout);
    imageDescription->mipLevelCount = imageEntry->mipLevelCount;
    if (imageEntry->flag == 1) {
        imageDescription->format = gtl_ImageFormat_FromSwizzled(imageDescription->format);
    }
    if (isPaletted == 0) {
        paletteDescription->format = -1;
        paletteDescription->entryCount = 0;
        return 1;
    }
    paletteDescription->format = gtl_ImageFormat_FromPlatformFormat(paletteEntry->format, kGimPlatformLayout);
    switch (imageDescription->format) {
    case kImageIndexed8Tiled:
    case kImageIndexed8Linear:
        paletteDescription->entryCount = kIndexed8PaletteEntries;
        break;
    case kImageIndexed4Tiled:
    case kImageIndexed4Linear:
        paletteDescription->entryCount = kIndexed4PaletteEntries;
        break;
    }
    return 1;
}

#pragma use_lmw_stmw reset
// Return a borrowed directory entry at index; arg1 is unused and index is unchecked.
void* gtl_ResourceFile_GetChunkByIndex(void* file, s32 unusedData, u32 directoryIndex) {
    return (u8*)file + (directoryIndex << kResourceDirectoryEntryShift) + kResourceDirectoryOffset;
}

// Find the first tagged directory entry, borrowing file storage; null file/miss returns null.
// data is an unused ABI argument; offsets are relative to file, not data.
void* gtl_ResourceFile_FindChunk(void* file, void* data, u32 tag) {
    if (file == 0) {
        return 0;
    }
    u32 count = *(u32*)((u8*)file + 0x14);
    u8* ptr;
    u8* base;
    base = (u8*)file + kResourceDirectoryOffset;
    ptr = base;
    u32 i;
    for (i = 0; i < count; i++) {
        if (tag == *(u32*)ptr) {
            return base + (i << kResourceDirectoryEntryShift);
        }
        ptr += kResourceDirectoryEntryBytes;
    }
    return 0;
}

// Return the first matching directory count, or zero on miss; data is unused.
// Unlike FindChunk, file must be nonnull; directory extent is trusted.
u32 gtl_ResourceFile_GetChunkElementCount(void* file, void* data, u32 tag) {
    u32 count = *(u32*)((u8*)file + 0x14);
    u8* ptr;
    u8* base;
    base = (u8*)file + kResourceDirectoryOffset;
    ptr = base;
    u32 i;
    for (i = 0; i < count; i++) {
        if (tag == *(u32*)ptr) {
            return *(u32*)(base + (i << kResourceDirectoryEntryShift) + 4);
        }
        ptr += kResourceDirectoryEntryBytes;
    }
    return 0;
}

// Borrow one element from the first matching chunk; data is unused.
// Null file, missing tag, zero count*stride or index>=count returns null. Product
// and offset arithmetic is 32-bit and unchecked; this is not a bounds validator.
void* gtl_ResourceFile_GetChunkElement(void* file, void* data, u32 tag, u32 index) {
    u8* chunk;
    if (file == 0) {
        return 0;
    }
    if (file == 0) {
        chunk = 0;
    } else {
        u32 count = *(u32*)((u8*)file + 0x14);
        u32 i;
        u8* base;
        u8* ptr;
        base = (u8*)file + kResourceDirectoryOffset;
        ptr = base;
        for (i = 0; i < count; i++) {
            if (tag == *(u32*)ptr) {
                chunk = base + (i << kResourceDirectoryEntryShift);
                goto found;
            }
            ptr += kResourceDirectoryEntryBytes;
        }
        chunk = 0;
    }
found:
    if (chunk == 0) {
        return 0;
    }
    {
        u32 size;
        u32 stride;
        stride = *(u32*)(chunk + 8);
        size = *(u32*)(chunk + 4);
        if (size * stride == 0) {
            return 0;
        }
        if (size <= index) {
            return 0;
        }
        return (u8*)file + *(u32*)(chunk + 0xC) + stride * index;
    }
}

// Test for a directory tag; required file is borrowed and data is unused.
s32 gtl_ResourceFile_HasChunk(void* file, void* data, u32 tag) {
    u32 count = *(u32*)((u8*)file + 0x14);
    u8* ptr = (u8*)file + kResourceDirectoryOffset;
    u32 i;
    for (i = 0; i < count; i++) {
        if (tag == *(u32*)ptr) {
            return 1;
        }
        ptr += kResourceDirectoryEntryBytes;
    }
    return 0;
}

// Index the resource data-type stride table: accepted ordinal tags are 0..9.
// s_resourceDataTypeByteStrides is 0x28 bytes (symbols.txt), ten native u32 entries:
// tag    0   1   2   3   4  5   6   7   8   9
// value  8  16  32  16  48  0  32  48  64  96
// These are opaque resource type tags, NOT engine image-format or GX IDs. No
// semantic type names are established; tag 5's zero is stored data, not a bounds
// check. Invalid/negative tags read outside the table, with no failure sentinel.
// Keep the table and serialized producer's tag mapping together when adding types.
u32 gtl_Resource_GetDataTypeByteStride(s32 dataType) {
    return s_resourceDataTypeByteStrides[dataType];
}

#pragma use_lmw_stmw on
// Decode texture/palette descriptions from a trusted GSR file; return 1.
// Both outputs/file are required. tex[0] must exist; absence is dereferenced.
// Writes {engine format,widthPixels,heightPixels,mipCount,platformLayout} and
// {palette engine format,entryCount}. GsrTextureRecordSchema documents tex;
// GsrPaletteRecordSchema documents pal, which reuses different +4/+8 meanings.
// Missing pal[0] clears palette format/count to {-1,0}. Unsupported file type
// passes layout -1 to conversion; it does not return a validation failure.
s32 gtl_ImageGSR_GetDescription(GsrDescription* out, GimPaletteDescription* paletteDesc, const GsrFile* file) {
    GsrEntry* textureEntry = (GsrEntry*)gtl_ResourceFile_GetChunkElement((void*)file, (void*)file->forwardedDataWord, kGsrTextureDescriptionTag, 0);
    GsrEntry* paletteEntry = (GsrEntry*)gtl_ResourceFile_GetChunkElement((void*)file, (void*)file->forwardedDataWord, kGsrPaletteDescriptionTag, 0);
    s32 platformLayout = kNoImageFormat;
    switch (file->fileType) {
    case 2:
        platformLayout = kGsrType2Layout;
        break;
    case 3:
        platformLayout = kGsrType3Layout;
        break;
    case 5:
        platformLayout = kGsrType5Layout;
        break;
    case 1:
        platformLayout = kGsrType1Layout;
        break;
    case 4:
        platformLayout = kGsrType4Layout;
        break;
    default:
        break;
    }
    out->width = textureEntry->unk4;
    out->height = textureEntry->unk8;
    out->mipLevelCount = textureEntry->unkC;
    out->format = gtl_ImageFormat_FromPlatformFormat(textureEntry->format, platformLayout);
    out->sourcePlatformLayout = platformLayout;
    if (paletteEntry != 0) {
        paletteDesc->format = gtl_ImageFormat_FromPlatformFormat(paletteEntry->format, platformLayout);
        paletteDesc->entryCount = paletteEntry->unk4;
    } else {
        paletteDesc->format = -1;
        paletteDesc->entryCount = 0;
    }
    return 1;
}

#pragma use_lmw_stmw reset
// Return 1 and write the borrowed pixel address texd[0]+tex[0].offset(+0x14).
// out/file and the texture description are required; no allocation or bounds check.
// Raw +0x14 equals GsrTextureRecordSchema::imageDataByteOffset; the returned
// word is a target address (u32 ABI), not a size, resource handle or owned buffer.
// Even a missing texd is not reported as failure; validate the resource upstream.
s32 gtl_ImageGSR_GetImageData(u32* out, const GsrFile* file) {
    u8* textureData = (u8*)gtl_ResourceFile_GetChunkElement((void*)file, (void*)file->forwardedDataWord, kGsrTextureDataTag, 0);
    u8* textureEntry = (u8*)gtl_ResourceFile_GetChunkElement((void*)file, (void*)file->forwardedDataWord, kGsrTextureDescriptionTag, 0);
    *out = (u32)textureData + *(u32*)(textureEntry + 0x14);
    return 1;
}

// Return 1 and write borrowed pald[0]+pal[0].offset(+8), or zero if pald is absent.
// out/file are required. Present pald requires pal[0]; buffer must outlive use.
// Raw +8 equals GsrPaletteRecordSchema::paletteDataByteOffset, not heightPixels.
// Palette upload/count/format and flush are handled by engine_render_cache_and_texture_load.cpp, not this parser.
s32 gtl_ImageGSR_GetPaletteData(u32* out, const GsrFile* file) {
    u8* paletteData = (u8*)gtl_ResourceFile_GetChunkElement((void*)file, (void*)file->forwardedDataWord, kGsrPaletteDataTag, 0);
    u8* paletteEntry = (u8*)gtl_ResourceFile_GetChunkElement((void*)file, (void*)file->forwardedDataWord, kGsrPaletteDescriptionTag, 0);
    if (paletteData == 0) {
        *out = 0;
        return 1;
    }
    *out = (u32)paletteData + *(u32*)(paletteEntry + 0x8);
    return 1;
}

// Release the owned word-vector buffer; positive dealloc also frees the nullable wrapper.
// Volatile count reload/subtraction and nested null checks preserve cleanup code.
// Returned address identifies the old wrapper, and may already be freed.
void* dtor_803D35A0(DtorData2* self, int dealloc) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    if (self->entries != 0) {
                        int previousCount = *(int*)&self->count;
                        self->count = previousCount - *(unsigned int*)&self->count;
                        __dl__FPv(self->entries);
                    }
                }
            }
        }
        if (dealloc > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Clear count, release the owned buffer, and optionally free the nullable wrapper.
// Return is the incoming address, potentially freed when arg1>0; not live data.
void* dtor_803D361C(DtorData* self, s32 dealloc) {
    if (self != 0) {
        self->count = 0;
        if (self != 0 && self->buffer != 0) {
            self->count = 0;
            __dl__FPv(self->buffer);
        }
        if (dealloc > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma push
#pragma dont_inline on
// Probe PICT numeric signatures at +0/+0x58; require at least 0x5C readable bytes.
s32 gtl_ImagePICT_IsValid(u8* fileBytes) {
    if (*(u32*)fileBytes != kPictSignature) {
        return 0;
    }
    return *(u32*)(fileBytes + 0x58) == kPictTag;
}

#pragma pop

// Read byte-reversed signed 16-bit PICT dimensions; write fixed format metadata and return 1.
// desc/file required (file >=0x60 bytes); paletteDesc is unused and untouched.
// Format 0x3F, word +0xC=1 and layout 9 are the existing PICT representation.
s32 gtl_ImagePICT_GetDescription(PictDescription* desc, void* paletteDesc, u8* file) {
    struct {
        s16 h;
        s16 w;
    } d;
    Memory_CopyAndReverseStridedBlocks(&d.w, file + 0x5c, 0x00020001, 2, 1);
    desc->width = d.w;
    Memory_CopyAndReverseStridedBlocks(&d.h, file + 0x5e, 0x00020001, 2, 1);
    desc->height = d.h;
    desc->format = 0x3f;
    desc->mipLevelCount = 1;
    desc->sourcePlatformLayout = 9;
    return 1;
}

#pragma dont_inline on
// Compute a Wii destination byte offset from x/y, row width and element-byte selector.
// Selector 0 addresses packed 4-bit BYTES (width/x already halved); 1/2 select
// 8x4/4x4 pixel tiles, 8/16 select 2x2 compressed-block tiles, and the default
// path addresses the AR half of 4x4 two-plane RGBA tiles. Width must be nonzero
// in that path. All coordinates are unchecked; the GB plane lies +0x20 bytes.
s32 gtl_Image_GetTiledOffset(u32 x, u32 y, u32 rowWidth, u32 elementBytes) {
    switch (elementBytes) {
    case 8:
    case 16:
        return elementBytes * ((((x >> 1) + ((rowWidth >> 1) * (y >> 1))) * 4) + ((x & 1) + ((y * 2) & 2)));
    case 0:
        return (((x >> 2) + ((rowWidth >> 2) * (y >> 3))) << 5) + ((x & 3) + ((y * 4) & 0x1C));
    case 1:
        return elementBytes * ((((x >> 3) + ((rowWidth >> 3) * (y >> 2))) << 5) + ((x & 7) + ((y * 8) & 0x18)));
    case 2:
        return elementBytes * ((((x >> 2) + ((rowWidth >> 2) * (y >> 2))) * 0x10) + ((x & 3) + ((y * 4) & 0xC)));
    default: {
        u32 linearRowOffset = rowWidth * y;
        u32 linearPixel;
        u32 fourRowsPixels = rowWidth * 4;
        u32 tileBytes = elementBytes * 0x10;
        linearPixel = x + linearRowOffset;
        u32 pixelColumn = linearPixel % rowWidth;
        u32 tileRow = linearPixel / fourRowsPixels;
        u32 rowWithinTile = (linearPixel % fourRowsPixels) / rowWidth;
        u32 fourRowsBytes = elementBytes * fourRowsPixels;
        return ((pixelColumn * 2) & 6) + (rowWithinTile * 8) + ((tileRow * fourRowsBytes) + ((pixelColumn >> 2) * tileBytes));
    }
    }
}

#pragma dont_inline off
#pragma use_lmw_stmw on
// Copy 4x4-pixel compressed blocks between linear and tiled offsets.
// arg0 source, arg1 destination, arg2/3 pixel width/height; arg5!=0 means linear
// to tiled, zero reverses it. Format pairs 0x2E/0x62 use 8 bytes/block;
// 0x30/0x64 and 0x32/0x66 use 16. Other formats use zero bytes here.
// Buffers are distinct. Width/height are pixel multiples of four, each at least
// four; block columns C=width/4 and rows R=height/4. For a reversible full image,
// each block count must be even or one. C==1 additionally requires R<=2: the
// tiled formula uses C>>1 as row stride, so taller single-column images alias.
// Odd counts>1 have incomplete/aliased tiles; partial blocks are truncated.
// Concrete capacity rule for supported formats (B=8 or 16 bytes/block):
// T = 4*((C-1)/2 + (C/2)*((R-1)/2)) + ((C-1)&1) + 2*((R-1)&1)
// P = max(C==1 ? 1 : 0, R==1 ? C : 0)
// Allocate at least B * (max(C*R, T+1) + P) bytes for EACH buffer; divisions are
// integer. This covers both directions, the highest tiled address, and padding
// stores at output+B (single column) or output+C*B (single row). It is a safe
// bound, not a promise that all padding is initialized. In particular a 16x4
// image needs 10*B, not the naive 8*B padded tile size: preserve these extra
// single-row clears when changing allocation. Zero dimensions copy nothing.
void gtl_Image_ReorderCompressedBlocks(u8* source, u8* destination, u32 widthPixels, u32 heightPixels, s32 format, s32 linearToTiled) {
    u32 linearOffset;
    u32 linearRowBlock;
    u32 rowBytes;
    u32 blockColumns;
    u32 blockRows;
    u32 blockBytes;
    u32 blockY;
    u32 blockX;
    blockColumns = widthPixels >> 2;
    blockRows = heightPixels >> 2;
    switch (format) {
    case kImageBlock4Tiled:
    case kImageBlock4Linear:
        blockBytes = 8;
        break;
    case kImageBlock8TiledB:
    case kImageBlock8LinearB:
        blockBytes = 0x10;
        break;
    case kImageBlock8TiledD:
    case kImageBlock8LinearD:
        blockBytes = 0x10;
        break;
    default:
        blockBytes = 0;
        break;
    }
    rowBytes = blockColumns * blockBytes;
    blockY = 0;
    linearRowBlock = 0;
    while (blockY < blockRows) {
        blockX = 0;
        while (blockX < blockColumns) {
            linearOffset = blockBytes * (blockX + linearRowBlock);
            u32 destinationOffset = gtl_Image_GetTiledOffset(blockX, blockY, blockColumns, blockBytes);
            u32 sourceOffset;
            if (linearToTiled != 0) {
                sourceOffset = linearOffset;
            } else {
                sourceOffset = destinationOffset;
                destinationOffset = linearOffset;
            }
            u8* outputBlock = destination + destinationOffset;
            Memory_Copy(outputBlock, source + sourceOffset, blockBytes);
            if (blockColumns == 1) {
                memset(outputBlock + blockBytes, 0, blockBytes);
            }
            if (blockRows == 1) {
                memset(outputBlock + rowBytes, 0, blockBytes);
            }
            blockX += 1;
        }
        linearRowBlock += blockColumns;
        blockY += 1;
    }
}

#pragma use_lmw_stmw reset
// Probe selected TGA header fields without length checks (at least 9 bytes required).
// Colormap length is copied as a native halfword; preserve that endian behavior.
s32 gtl_ImageTGA_IsValid(const u8* fileBytes) {
    u16 colormapLength;
    u8 idType = fileBytes[1];
    switch (idType) {
    case 0:
    case 1:
        break;
    default:
        return 0;
    }
    switch (fileBytes[7]) {
    case 0:
    case 0xf:
    case 0x10:
    case 0x18:
    case 0x20:
        break;
    default:
        return 0;
    }
    memcpy(&colormapLength, fileBytes + 5, 2);
    switch (colormapLength) {
    case 0:
    case 0x10:
    case 0x100:
        break;
    default:
        return 0;
    }
    switch (fileBytes[2]) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 0xa:
        break;
    default:
        return 0;
    }
    return 1;
}

// Probe the native halfword BMP signature; require two aligned readable bytes.
s32 gtl_ImageBMP_IsValid(u8* fileBytes) {
    u32 diff = *(u16*)fileBytes - kBmpSignature;
    if (diff == 0) {
        return 1;
    }
    return 0;
}

// Probe TIM2 signature, versions 3/4 and layout bytes 0/1; require six readable bytes.
s32 gtl_ImageTIM2_IsValid(u8* fileBytes) {
    if (*(u32*)fileBytes != kTim2Signature) {
        return 0;
    }
    if (fileBytes[4] != 3 && fileBytes[4] != 4) {
        return 0;
    }
    if (fileBytes[5] != 0 && fileBytes[5] != 1) {
        return 0;
    }
    return 1;
}

// Probe GSR signature and file type 2; require 0x14 aligned readable bytes.
// This distinguishes a format-table selector, not bit depth or a complete file validation.
s32 gtl_ImageGSR_IsType2(u8* fileBytes) {
    if (*(u32*)(fileBytes + 8) == kGsrSignature && *(u32*)(fileBytes + 0x10) == 2) {
        return 1;
    }
    return 0;
}

// Probe GSR signature and file type 1; require 0x14 aligned readable bytes.
// This distinguishes a format-table selector, not bit depth or a complete file validation.
s32 gtl_ImageGSR_IsType1(u8* fileBytes) {
    if (*(u32*)(fileBytes + 8) == kGsrSignature && *(u32*)(fileBytes + 0x10) == 1) {
        return 1;
    }
    return 0;
}

// Probe GSR signature and file type 3; require 0x14 aligned readable bytes.
// This distinguishes a format-table selector, not bit depth or a complete file validation.
s32 gtl_ImageGSR_IsType3(u8* fileBytes) {
    if (*(u32*)(fileBytes + 8) == kGsrSignature && *(u32*)(fileBytes + 0x10) == 3) {
        return 1;
    }
    return 0;
}

// Probe GSR signature and file type 5; require 0x14 aligned readable bytes.
// This distinguishes a format-table selector, not bit depth or a complete file validation.
s32 gtl_ImageGSR_IsType5(u8* fileBytes) {
    if (*(u32*)(fileBytes + 8) == kGsrSignature && *(u32*)(fileBytes + 0x10) == 5) {
        return 1;
    }
    return 0;
}

// Probe GSR signature and file type 4; require 0x14 aligned readable bytes.
// This distinguishes a format-table selector, not bit depth or a complete file validation.
s32 gtl_ImageGSR_IsType4(u8* fileBytes) {
    if (*(u32*)(fileBytes + 8) == kGsrSignature && *(u32*)(fileBytes + 0x10) == 4) {
        return 1;
    }
    return 0;
}

// Forward the PICT signature probe without acquiring or decoding the borrowed buffer.
s32 gtl_ImagePICT_Probe(u8* fileBytes) {
    return gtl_ImagePICT_IsValid(fileBytes);
}

// Probe four GIM signature/version/platform words; require 16 aligned readable bytes.
s32 gtl_ImageGIM_IsValid(u8* fileBytes) {
    if (*(u32*)fileBytes != kGimSignature) {
        return 0;
    }
    if (*(u32*)(fileBytes + 4) != kGimVersion) {
        return 0;
    }
    if (*(u32*)(fileBytes + 8) != kGimPlatformTag) {
        return 0;
    }
    return *(u32*)(fileBytes + 0xC) == 0;
}

// Forward the DDS signature probe; borrowed input must satisfy ImageDDS_IsValid's contract.
s32 gtl_ImageDDS_Probe(u8* fileBytes) {
    return gtl_ImageDDS_IsValid(fileBytes);
}

#pragma use_lmw_stmw on
#pragma push
#pragma opt_propagation off
// Uncompressed tiling contract, shared by the two bodies and public wrappers:
// source/destination are required NONOVERLAPPING byte buffers, borrowed only for
// this call. No allocation, dimensions check, status, or rollback. All products,
// rounded capacities and offset arithmetic must fit u32 and the actual buffers.
// Input is tightly packed, row-major, no source stride argument. Capacities:
//   4-bit: even W>=2,H>=1; source W/2*H bytes; tiles 8x8, 32 bytes each.
//   8-bit: W,H>=1; source W*H; tiles 8x4, 32 bytes each.
//  16-bit: W,H>=1; source 2*W*H; tiles 4x4, 32 bytes each.
// destination capacity >= ceil(W/tileW)*ceil(H/tileH)*32, for each selector.
// Complete image-preserving shapes: BOTH dimensions positive multiples of their
// tile dimensions. These work for rectangular multi-tile images too, e.g.
// 4-bit 24x16, 8-bit 24x8, 16-bit 12x8. The floor shifts in GetTiledOffset do
// not round the row stride: a larger nonmultiple width aliases successive tile
// rows. More destination capacity does not repair that addressing.
// Additional complete minimum-tile case for 4/8-bit only: W<=tileW,H<=tileH;
// missing columns/rows are zeroed. A narrow image taller than one tile, or a
// wide image with partial final tile rows, is not covered by that guarantee.
// 16-bit subminimum padding is legacy BYTE-only writes: one byte per missing
// pixel and bottom rows at H*max(W,4), without *2. It can overwrite copied
// pixels and leave padding unchanged. Capacity above makes writes fit, but
// cannot promise a complete valid padded texture; use full 4x4 multiples for
// image-preserving 16-bit conversion. Do not silently fix this compatibility path.
// Odd 4-bit W truncates the final nibble via W>>1 and is outside the image contract.
// The dispatcher LinearToTiled and inverse bodies in gtl_image_layout_and_sizing.cpp share the same
// GetTiledOffset schema; a future resize mod must update pixel storage AND metadata.
// Tile packed 4-bit source bytes and zero the minimum 8x8-pixel tile padding.
// bitsPerPixel must be 4; other values leave minimum sizes uninitialized.
static inline void Tile4PixelsBody(u8* source, u8* destination, u32 widthPixels, u32 height, s32 bitsPerPixel) {
    // Source row count first, then increments while filling the minimum tile.
    u32 copiedOrPaddedRows = height;
    u32 storedRowBytes;
    u32 byteX;
    u32 y;
    u32 minimumRows;
    u32 minimumRowBytes;
    u32 sourceRowByteOffset;
    u32 paddingByteOffset;
    switch (bitsPerPixel) {
    case 4:
        minimumRowBytes = 4;
        minimumRows = 8;
        break;
    default:
        break;
    }
    y = 0;
    sourceRowByteOffset = 0;
    // Copy source rows in packed bytes, then zero their minimum-tile suffix.
    storedRowBytes = widthPixels >> 1;
    paddingByteOffset = 0;
    while (y < copiedOrPaddedRows) {
        byteX = 0;
        while (byteX < storedRowBytes) {
            u32 off = gtl_Image_GetTiledOffset(byteX, y, storedRowBytes, 0);
            Memory_Copy(destination + off, source + (byteX + sourceRowByteOffset), 1);
            byteX += 1;
        }
        byteX = storedRowBytes;
        while (byteX < minimumRowBytes) {
            u32 off = gtl_Image_GetTiledOffset(byteX, y, 4, 0);
            destination[off] = 0;
            byteX += 1;
        }
        sourceRowByteOffset += storedRowBytes;
        y += 1;
    }
    // Source rows are complete; append all-zero rows to the minimum tile height.
    if (storedRowBytes < minimumRowBytes) {
        storedRowBytes = 4;
    }
    paddingByteOffset = copiedOrPaddedRows * storedRowBytes;
    while (copiedOrPaddedRows < minimumRows) {
        memset(destination + paddingByteOffset, 0, storedRowBytes);
        paddingByteOffset += storedRowBytes;
        copiedOrPaddedRows += 1;
    }
}

// Tile borrowed packed 4-bit pixels; arg4 must be 4, buffers distinct and padded as Tile4PixelsBody.
void gtl_Image_Tile4BitPixels(u8* source, u8* destination, u32 widthPixels, u32 heightPixels, s32 bitsPerPixel) {
    Tile4PixelsBody(source, destination, widthPixels, heightPixels, bitsPerPixel);
}

// Tile 1/2-byte pixels, padding to minimum 8x4/4x4 geometry respectively.
// format is BYTES/pixel, restricted to 1 or 2, not an engine/GX format ID.
// Keep the original byte-only padding stores and byte counts, even for 16-bit pixels.
// See the complete capacity/shape contract above Tile4PixelsBody; in particular
// the 16-bit minimum-size padding does not promise to preserve the copied pixels.
static inline void Tile8Or16PixelsBody(u8* source, u8* destination, u32 width, u32 height, s32 elementBytes) {
    u32 storedRowPixels = width;
    u32 copiedOrPaddedRows = height;
    s32 bytesPerPixel = elementBytes;
    u32 x;
    u32 y;
    u32 minimumRows;
    u32 minimumRowPixels;
    u32 sourceRowPixels;
    // Zero while padding columns; becomes the row-padding BYTE cursor afterwards.
    u32 zeroOrPaddingByteOffset;
    switch (bytesPerPixel) {
    case 1:
        minimumRowPixels = 8;
        minimumRows = 4;
        break;
    case 2:
        minimumRowPixels = 4;
        minimumRows = 4;
        break;
    default:
        break;
    }
    y = 0;
    sourceRowPixels = 0;
    zeroOrPaddingByteOffset = 0;
    while (y < copiedOrPaddedRows) {
        x = 0;
        while (x < storedRowPixels) {
            u32 off = gtl_Image_GetTiledOffset(x, y, storedRowPixels, bytesPerPixel);
            Memory_Copy(destination + off, source + (x + sourceRowPixels) * bytesPerPixel, bytesPerPixel);
            x += 1;
        }
        x = storedRowPixels;
        while (x < minimumRowPixels) {
            u32 off = gtl_Image_GetTiledOffset(x, y, minimumRowPixels, bytesPerPixel);
            destination[off] = zeroOrPaddingByteOffset;
            x += 1;
        }
        sourceRowPixels += storedRowPixels;
        y += 1;
    }
    if (storedRowPixels < minimumRowPixels) {
        storedRowPixels = minimumRowPixels;
    }
    // Preserve this byte cursor (no bytesPerPixel multiplier) and the row memset
    // byte count: changing either would change the original 16-bit padding writes.
    zeroOrPaddingByteOffset += copiedOrPaddedRows * storedRowPixels;
    while (copiedOrPaddedRows < minimumRows) {
        memset(destination + zeroOrPaddingByteOffset, 0, storedRowPixels);
        zeroOrPaddingByteOffset += storedRowPixels;
        copiedOrPaddedRows += 1;
    }
}

// Tile borrowed 8/16-bit pixels; arg4 is 1/2 bytes per pixel (see Tile8Or16PixelsBody).
void gtl_Image_Tile8Or16BitPixels(u8* source, u8* destination, u32 widthPixels, u32 heightPixels, s32 bytesPerPixel) {
    Tile8Or16PixelsBody(source, destination, widthPixels, heightPixels, bytesPerPixel);
}

#pragma pop
#pragma dont_inline on
// Dispatch borrowed source pixels to Wii tiling, writing the required padded destination.
// arg0 source, arg3 destination, arg4/5 pixel width/height, arg7 engine format.
// arg1/2/6 are unused ABI words. No validation or allocation: caller supplies
// nonoverlapping buffers of sufficient size. The fallback assumes four-byte
// source pixels: source[3,0] -> AR plane, source[1,2] -> GB plane at +0x20.
// Compressed/packed formats retain their own tile rules; no palette is uploaded.
// Ordinary fallback admission: GetBytesPerElement(format)==4, hence engine IDs
// 0x01/02/0E/0F/10/11/34/36/38/3F/40/4C/4D/4E/4F/68/6A/6C. The operation
// preserves four source bytes by their positions; it does not establish color
// semantics for every such ID. Producer must supply the desired RGBA byte order.
// Require positive W,H, source capacity 4*W*H; every product/offset must fit u32.
// Destination capacity >= 64*ceil(W/4)*ceil(H/4) bytes is sufficient. Complete
// reversible rectangular image shapes have W a multiple of 4; H may be partial
// (e.g. 12x5 needs source 240 bytes and destination 384). The fallback writes
// only actual pixels, leaving unused rows/pixels UNINITIALIZED, so clear padding
// separately if the consumer requires it. It does not replicate edge pixels.
// Narrow W<=4,H<=4 also fits a single 64-byte tile without aliasing; for H>4 a
// nonmultiple W causes tile rows to overlap (row advance is 16*W bytes, not
// rounded tile count*64). Extra allocation cannot restore those overwritten
// pixels. For general resizing use W%4==0 or a separate padded-source workflow.
// Unsupported/-1/three-byte/eight-byte formats reach this fallback unchecked;
// they are NOT accepted: byte reads/plane writes can overrun or alias. No status
// is returned. The size-query's zero result is not an empty-image fallback.
void gtl_Image_LinearToTiled(u8* source, u32 unusedSourceWord1, u32 unusedSourceWord2, u8* destination, u32 widthPixels, u32 heightPixels, u32 unusedDestinationWord, s32 format) {
    u32 bitsPerPixel;
    u32 sourceRowPixels;
    s32 bytesPerPixel;
    u32 y;
    u32 x;
    const u8* inputPixel;
    u32 sourceByteOffset;
    u8* outputPixel;
    u32 redThenBlue;
    u32 alphaThenGreen;

    s32 compressed;
    switch (format) {
    case kImageBlock4Linear:
    case kImageBlock8LinearA:
    case kImageBlock8LinearB:
    case kImageBlock8LinearC:
    case kImageBlock8LinearD:
    case kImageBlock4Tiled:
    case kImageBlock8TiledA:
    case kImageBlock8TiledB:
    case kImageBlock8TiledC:
    case kImageBlock8TiledD:
        compressed = 1;
        break;
    default:
        compressed = 0;
        break;
    }
    if (compressed) {
        gtl_Image_ReorderCompressedBlocks(source, destination, widthPixels, heightPixels, format, 1);
        return;
    }
    switch (format) {
    case kImageIndexed4Linear:
    case kImageFourBitLinear:
    case kImageIndexed4Tiled:
    case kImageFourBitTiled:
        bitsPerPixel = 4;
        break;
    case kImageBlock4Linear:
    case kImageBlock4Tiled:
        bitsPerPixel = 4;
        break;
    case kImageBlock8LinearA:
    case kImageBlock8LinearB:
    case kImageBlock8LinearC:
    case kImageBlock8LinearD:
    case kImageBlock8TiledA:
    case kImageBlock8TiledB:
    case kImageBlock8TiledC:
    case kImageBlock8TiledD:
        bitsPerPixel = 8;
        break;
    default:
        bitsPerPixel = gtl_ImageFormat_GetBytesPerElement(format) * 8;
        break;
    case -1:
        bitsPerPixel = 0;
        break;
    }
    if (bitsPerPixel == 4) {
        gtl_Image_Tile4BitPixels(source, destination, widthPixels, heightPixels, bitsPerPixel);
        return;
    }
    bytesPerPixel = gtl_ImageFormat_GetBytesPerElement(format);
    if ((u32)(bytesPerPixel - 1) <= 1U) {
        gtl_Image_Tile8Or16BitPixels(source, destination, widthPixels, heightPixels, bytesPerPixel);
        return;
    }
    y = 0;
    sourceRowPixels = 0;
    while (y < heightPixels) {
        x = 0;
        while (x < widthPixels) {
            outputPixel = destination + gtl_Image_GetTiledOffset(x, y, widthPixels, bytesPerPixel);
            sourceByteOffset = bytesPerPixel * (x + sourceRowPixels);
            x += 1;
            inputPixel = source + sourceByteOffset;
            redThenBlue = source[sourceByteOffset];
            alphaThenGreen = inputPixel[3];
            *outputPixel = alphaThenGreen;
            alphaThenGreen = inputPixel[1];
            outputPixel[1] = redThenBlue;
            redThenBlue = inputPixel[2];
            outputPixel[0x20] = alphaThenGreen;
            outputPixel[kRgbaBlueByteOffset] = redThenBlue;
        }
        sourceRowPixels += widthPixels;
        y += 1;
    }
}

#pragma pop

} // extern "C"
