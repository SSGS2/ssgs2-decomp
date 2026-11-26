// Texture palettes, texture formats, mip storage sizes and vertex formats for the graphics layer.
// Covers the palette descriptors, the platform-7 image copy functions, vertex declaration copies,
// primitive and index counts, and the header setup for graphics buffer and resource objects.

#include "classes/TextureMipLayout8042.h"

#include "classes/TexturePaletteState.h"
#include "classes/GtlBuffer.h"

// These are GTL format encodings; the high bits distinguish platform storage.
enum TextureFormat8042 {
    FormatI4 = 0,
    FormatI8 = 1,
    FormatIA4 = 2,
    FormatRGB565 = 4,
    FormatRGBA8 = 6,
    FormatC4 = 8,
    FormatC8 = 9,
    FormatCMPR = 14,
    PlatformI4 = (int)0x80000000,
    PlatformI8 = (int)0x80000001,
    PlatformIA4 = (int)0x80000002,
    PlatformIA8 = (int)0x80000003,
    PlatformRGB565 = (int)0x80000004,
    PlatformRGB5A3 = (int)0x80000005,
    PlatformRGBA8 = (int)0x80000006,
    PlatformC4 = (int)0x80000008,
    PlatformC8 = (int)0x80000009,
    PlatformRGB24 = (int)0x8000000D,
    PlatformCMPR = (int)0x8000000E
};
enum PaletteType8042 {
    Palette256Entries = 0,
    Palette16Entries = 4,
    InvalidPaletteType = -1
};
enum PrimitiveType8042 {
    PrimitiveQuads = 0x80,
    PrimitiveTriangles = 0x90,
    PrimitiveTriangleStrip = 0x98,
    PrimitiveTriangleFan = 0xA0,
    PrimitiveLines = 0xA8,
    PrimitiveLineStrip = 0xB0,
    PrimitivePoints = 0xB8
};
enum KnownVertexFormat8042 {
    FVF_Position3_Normal3_TexCoord2 = 0x110401,
    FVF_Position3_Normal3 = 0x401,
    FVF_Position3_Normal3_Color = 0xF401,
    FVF_Position3_Color = 0xF001,
    FVF_Position4_Normal4_TexCoord2 = 0x510521,
    FVF_PrefixByte_Position3_Normal3_TexCoord2 = 0x910401
};
enum VertexElementSemantic8042 {
    VertexPosition = 0,
    VertexNormal = 3,
    VertexTexCoord = 5,
    VertexColor = 10
};
enum GraphicsLimits8042 {
    DefaultMipLevelCount = 8,
    CacheLineBytes = 32,
    PlatformFormat7 = 7,
    PaletteDescriptionFlagsShift = 13,
    PaletteDescriptionFlagsMask = 7,
    TileWidth = 4,
    TilePlaneBytes = 32,
    RGBA8BytesPerPixel = 4
};

// Format numbers without a demonstrated semantic name remain literal in the
// switches. GTL's high-bit platform encodings are distinct from bare GX enums.
// APIs below trust caller-supplied dimensions, counts and output buffer sizes.
enum GraphicsResourceKinds8042 {
    ResourceType6 = 6,
    ResourceType7 = 7,
    VertexDeclarationKind = 8
};

// Partial descriptors: preserve the 32-bit argument representations stored by the original API.
struct BufferDescriptor8042 {
    int flags;
    int dataAddress;
    int reserved;
    int indexCountOrVertexBytes;
    int elementFormat;
    int indexStrideBytes;
};
struct PaletteDescriptor8042 {
    unsigned int flags;
    int dataAddress;
    int reserved;
    int paletteType;
    int textureFormat;
    int gxPaletteFormat;
};
enum GXPaletteFormat8042 {
    GXPaletteIA8 = 0,
    GXPaletteRGB565 = 1,
    GXPaletteRGB5A3 = 2
};
struct ResourcePayload8042 {
    int header[3];
    union {
        int value;
        void* pointer;
    } payload;
};
typedef char BufferDescriptor8042_size_check[(sizeof(BufferDescriptor8042) == 0x18) ? 1 : -1];
typedef char PaletteDescriptor8042_size_check[(sizeof(PaletteDescriptor8042) == 0x18) ? 1 : -1];
struct VertexElement8042 {
    unsigned int words[3];
};
struct VertexDeclaration8042 {
    unsigned int header[3];
    VertexElement8042 elements[1];
};

extern "C" {

// Store the index-buffer descriptor arguments without touching its header or reserved word.
void IndexBuffer_InitializeDescriptorFields(int indexCount, int unused1, int elementFormat, int unused2,
                                            char* descriptor, int dataAddress, int indexStride) {
    ((BufferDescriptor8042*)descriptor)->elementFormat = elementFormat;
    ((BufferDescriptor8042*)descriptor)->indexCountOrVertexBytes = indexCount; // legacy name: count, not resource kind
    ((BufferDescriptor8042*)descriptor)->dataAddress = dataAddress;
    ((BufferDescriptor8042*)descriptor)->indexStrideBytes = indexStride; // legacy field name; byte stride at +0x14
}

// Store the vertex-buffer descriptor arguments without touching its header or reserved word.
void VertexBuffer_InitializeDescriptorFields(int byteCount, int unused1, int elementFormat, int unused2,
                                             char* descriptor, int dataAddress) {
    ((BufferDescriptor8042*)descriptor)->indexCountOrVertexBytes = byteCount; // legacy name: bytes, not resource kind
    ((BufferDescriptor8042*)descriptor)->dataAddress = dataAddress;
    ((BufferDescriptor8042*)descriptor)->elementFormat = elementFormat;
}

extern "C" void gtl_Graphics_FreeWithCallback(void* p);

int TexturePalette_GetEntryCountForType(int paletteType);
// Legacy ABI has four meaningful inputs. The three trailing words are never read;
// four-argument call sites intentionally leave them unspecified. Keep those call
// shapes for matching; do not infer extra size/alignment controls from the casts.
int Texture_CalculateMipStorageSize(int width, int height, int count, int format, int, int, int);
void DCFlushRange(void* addr, unsigned int nBytes);

// Flush the palette data range computed from its entry count and texture format.
int TexturePalette_Flush(char* palette) {
    int Texture_CalculateMipStorageSize(int width, int height, int count, int format);
    int entryCount = TexturePalette_GetEntryCountForType(((TexturePaletteState*)palette)->paletteType);
    int byteCount = Texture_CalculateMipStorageSize(entryCount, 1, 1, ((TexturePaletteState*)palette)->textureFormat);
    DCFlushRange(((TexturePaletteState*)palette)->data, byteCount);
    return 1;
}

#pragma optimization_level 1
// Return four bits per index for C4 formats; other formats are unrecognized here.
int TextureFormat_GetC4IndexBitCount(int format) {
    switch (format) {
    case FormatC4:
    case PlatformC4:
        return 4;
    }
    return 0;
}
#pragma optimization_level reset

// Translate the palette descriptor kind into 256 or 16 entries.
int TexturePalette_GetEntryCountForType(int paletteType) {
    switch (paletteType) {
    case Palette256Entries:
        return 0x100;
    case Palette16Entries:
        return 0x10;
    }
    return 0;
}

// Translate a supported palette entry count into its descriptor kind.
int TexturePalette_GetTypeForEntryCount(int entryCount) {
    switch (entryCount) {
    case 0x100:
        return Palette256Entries;
    case 0x10:
        return Palette16Entries;
    }
    return -1;
}

#pragma push
#pragma optimization_level 0
// Recognize the two platform-encoded indexed texture formats.
int TextureFormat_IsPaletted(int format) {
    switch (format) {
    case PlatformC4:
    case PlatformC8:
        return 1;
    case (int)0xC0000006:
    case PlatformCMPR:
    case PlatformRGBA8:
    case PlatformI8:
    case PlatformIA4:
    case PlatformIA8:
    case (int)0x80000013:
    case (int)0x80000016:
    case FormatRGB565:
    case (int)0xC0000032:
    case (int)0xC0000030:
    case FormatI8:
    case (int)0x40000006:
    case FormatRGBA8:
        return 0;
    default:
        return 0;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Find a supported semantic in one of the fixed vertex layouts, or return -1.
int VertexFormat_GetElementOffset(int vertexFormat, int semantic) {
    switch (vertexFormat) {
    case FVF_Position3_Normal3_TexCoord2:
        switch (semantic) {
        case VertexPosition:
            return 0;
        case VertexNormal:
            return 0xC;
        case VertexTexCoord:
            return 0x18;
        default:
            return -1;
        }
    case FVF_Position3_Normal3:
        switch (semantic) {
        case VertexPosition:
            return 0;
        case VertexNormal:
            return 0xC;
        default:
            return -1;
        }
    case FVF_Position3_Normal3_Color:
        switch (semantic) {
        case VertexPosition:
            return 0;
        case VertexNormal:
            return 0xC;
        case VertexColor:
            return 0x18;
        default:
            return -1;
        }
    case FVF_Position3_Color:
        switch (semantic) {
        case VertexPosition:
            return 0;
        case VertexColor:
            return 0xC;
        default:
            return -1;
        }
    case FVF_Position4_Normal4_TexCoord2:
        switch (semantic) {
        case VertexPosition:
            return 0;
        case VertexNormal:
            return 0x10;
        case VertexTexCoord:
            return 0x20;
        default:
            return -1;
        }
    case FVF_PrefixByte_Position3_Normal3_TexCoord2:
        switch (semantic) {
        case VertexPosition:
            return 1;
        case VertexNormal:
            return 0xD;
        case VertexTexCoord:
            return 0x19;
        default:
            return -1;
        }
    }
    return -1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Export texture format, packed description flags, and palette kind as three words.
int TexturePalette_GetDescription(void* palette, int* description) {
    description[0] = ((TexturePaletteState*)palette)->textureFormat;
    description[2] = ((TexturePaletteState*)palette)->paletteType;
    int descriptionFlags =
        (((TexturePaletteState*)palette)->flags >> PaletteDescriptionFlagsShift) & PaletteDescriptionFlagsMask;
    description[1] = descriptionFlags;
    return 1;
}
#pragma pop

void GraphicsResource_InitializeHeader(void* p, int a, int b, int c);

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma optimize_for_size on
// Sum mip sizes, clamping each dimension to one and rounding each accumulated level to 32 bytes.
int Texture_CalculateMipStorageSize(int width, int height, int levelCount, int format, int, int, int) {
    // The three trailing ABI arguments are intentionally ignored; some callers supply only four.
    // Result is bytes, including per-level cache-line padding. Unknown formats
    // contribute zero; this is not a validator for dimensions or format support.
    int level;
    int totalBytes = 0;
    int levelsToMeasure = 0;
    unsigned int mipWidth;
    int alignedTotalBytes;
    int savedMipWidth;
    bool isCacheLineAligned;
    if (levelCount == 0) {
        levelsToMeasure = DefaultMipLevelCount;
    } else {
        levelsToMeasure = levelCount;
    }
    for (level = 0; level < levelsToMeasure; level++) {
        int mipHeight;
        int levelBytes;
        int levelWidth;
        mipWidth = (unsigned int)width >> level;
        mipHeight = (unsigned int)height >> level;
        if (mipWidth == 0)
            mipWidth = 1;
        if (mipHeight == 0)
            mipHeight = 1;
        levelWidth = mipWidth;
        savedMipWidth = levelWidth;
        (void)level;
        // Measure this level's payload before rounding the running total.
        switch (format) {
        case PlatformI4:
        case FormatI4:
            levelBytes = (unsigned int)(levelWidth * mipHeight) >> 1;
            break;
        case FormatRGBA8:
        case 0x16:
        case (int)0x40000006:
        case PlatformRGBA8:
        case (int)0x80000016:
        case (int)0xC0000006:
            levelBytes = (levelWidth * mipHeight) << 2;
            break;
        case PlatformRGB24:
            levelBytes = levelWidth * mipHeight * 3;
            break;
        case FormatC4:
        case PlatformC4:
            levelBytes = (unsigned int)(levelWidth * mipHeight) >> 1;
            break;
        case FormatC8:
        case PlatformC8:
            levelBytes = levelWidth * mipHeight;
            break;
        case FormatRGB565:
        case 0x2C:
        case PlatformRGB5A3:
        case (int)0x8000002C:
            levelBytes = (levelWidth * mipHeight) << 1;
            break;
        case FormatCMPR:
        case PlatformCMPR:
            levelBytes = (unsigned int)(levelWidth * mipHeight) >> 1;
            break;
        case (int)0xC0000030:
        case (int)0xC0000032:
        case (int)0x40000030:
        case (int)0x40000032:
            levelBytes = levelWidth * mipHeight;
            break;
        case FormatIA4:
        case PlatformIA4:
            levelBytes = levelWidth * mipHeight;
            break;
        case FormatI8:
        case PlatformI8:
            levelBytes = levelWidth * mipHeight;
            break;
        case PlatformIA8:
            levelBytes = (levelWidth * mipHeight) << 1;
            break;
        case (int)0x80000013:
            levelBytes = (levelWidth * mipHeight) << 1;
            break;
        default:
            levelBytes = 0;
            break;
        }
        totalBytes += levelBytes;
        isCacheLineAligned = ((totalBytes / CacheLineBytes) << 5) == totalBytes;
        if (!isCacheLineAligned) {
            alignedTotalBytes = totalBytes + (CacheLineBytes - (totalBytes - ((totalBytes / CacheLineBytes) << 5)));
        } else {
            alignedTotalBytes = totalBytes;
        }
        totalBytes = alignedTotalBytes;
    }
    return totalBytes;
}
#pragma pop
void DCFlushRange(void* addr, unsigned int nBytes);
int VertexDeclaration_GetElementCount(char* elements);
void VertexElement_Copy(void* dst, void* src);
extern "C" void* memset(void* dst, int value, unsigned int size);

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Initialize the declaration header and copy the source elements including their terminator.
void* VertexDeclaration_CopyConstruct(void* self, void* source) {
    GraphicsResource_InitializeHeader(self, VertexDeclarationKind, 1, 0);
    unsigned int count = (unsigned int)VertexDeclaration_GetElementCount((char*)source) + 1;
    for (unsigned int i = 0; i < count; i++) {
        VertexElement8042* dst = (VertexElement8042*)((char*)self + i * sizeof(VertexElement8042) +
                                                      sizeof(((VertexDeclaration8042*)0)->header));
        VertexElement_Copy(dst, (VertexElement8042*)source + i);
    }
    return self;
}
#pragma pop

#pragma optimize_for_size on
// Convert a primitive count into the required number of indices for the topology.
int Primitive_GetIndexCount(int primitiveCount, int primitiveType) {
    switch (primitiveType) {
    case PrimitiveTriangleStrip:
    case PrimitiveTriangleFan:
        return primitiveCount + 2;
    case PrimitiveLines:
        return primitiveCount * 2;
    case PrimitiveLineStrip:
        return primitiveCount + 1;
    case PrimitiveTriangles:
        return primitiveCount * 3;
    case PrimitiveQuads:
        return primitiveCount << 2;
    case PrimitivePoints:
        return primitiveCount;
    }
    return 0;
}
#pragma optimize_for_size reset

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Clear eight mip slots and record dimensions, offsets, sizes, and descriptor arguments.
void Texture_BuildMipLayout(int baseWidth, int baseHeight, unsigned int levelCount, int descriptorValue, int format,
                            int unused, char* layout) {
    // A zero count clears the slots and advertises eight levels, but the loop still runs zero times.
    // Requires levelCount <= 8. Offsets/sizes are bytes from the backing allocation;
    // dimensions are texels. This routine neither allocates nor acquires pixels.
    memset(((TextureMipLayout8042*)layout)->levels, 0, sizeof(((TextureMipLayout8042*)layout)->levels));
    int totalBytes = 0;
    for (unsigned int level = 0; level < levelCount; level++) {
        // Preserve base + scaled index + member offset ordering under the original -O0 pragma.
        char* mip = layout + level * sizeof(TextureMipLevel8042) + (int)&((TextureMipLayout8042*)0)->levels;
        ((TextureMipLevel8042*)mip)->width = baseWidth >> level;
        ((TextureMipLevel8042*)mip)->height = baseHeight >> level;
        if (((TextureMipLevel8042*)mip)->width <= 0)
            ((TextureMipLevel8042*)mip)->width = 1;
        if (((TextureMipLevel8042*)mip)->height <= 0)
            ((TextureMipLevel8042*)mip)->height = 1;
        ((TextureMipLevel8042*)mip)->dataOffset = totalBytes;
        int levelBytes = ((int (*)(int, int, int, int))Texture_CalculateMipStorageSize)(
            ((TextureMipLevel8042*)mip)->width, ((TextureMipLevel8042*)mip)->height, 1, format);
        ((TextureMipLevel8042*)mip)->byteSize = levelBytes;
        totalBytes += levelBytes;
    }
    ((TextureMipLayout8042*)layout)->levelCount = levelCount ? levelCount : DefaultMipLevelCount;
    ((TextureMipLayout8042*)layout)->format = format;
    ((TextureMipLayout8042*)layout)->descriptorValue =
        descriptorValue; // caller descriptor value; not interpreted by the mip builder
    ((int (*)(int, int, unsigned int, int))Texture_CalculateMipStorageSize)(baseWidth, baseHeight, levelCount, format);
}
#pragma pop

int VertexDeclaration_GetElementCount(char* elements);
void VertexElement_Copy(void* dst, void* src);

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Copy all declaration elements including the terminator and report the copied count.
void VertexDeclaration_GetElements(char* declaration, void* outElements, unsigned int* outCount) {
    // outElements must hold GetElementCount()+1 records; outCount includes the terminator.
    unsigned int count =
        (unsigned int)VertexDeclaration_GetElementCount((char*)((VertexDeclaration8042*)declaration)->elements) + 1;
    for (unsigned int i = 0; i < count; i++) {
        VertexElement_Copy((VertexElement8042*)outElements + i,
                           declaration + i * sizeof(VertexElement8042) + sizeof(((VertexDeclaration8042*)0)->header));
    }
    *outCount = count;
}
#pragma pop

// Initialize a type-7 resource header and its integer payload.
void* GraphicsResource_InitType7(void* resource, int payloadValue) {
    volatile int savedPayloadValue = payloadValue;
    GraphicsResource_InitializeHeader(resource, ResourceType7, 1, 0);
    ((ResourcePayload8042*)resource)->payload.value = savedPayloadValue;
    return resource;
}

#pragma push
#pragma optimization_level 0
// Recognize the platform CMPR, C8, and RGBA8 formats selected by this predicate.
int TextureFormat_IsPlatformCMPR_C8OrRGBA8(int format) {
    switch (format) {
    case PlatformCMPR:
    case PlatformC8:
    case PlatformRGBA8:
        return 1;
    case FormatRGBA8:
        return 0;
    }
    return 0;
}
#pragma pop

// Store palette arguments and select the GX palette encoding for RGB565, RGB5A3, or RGBA8.
void TexturePalette_InitializeDescriptorFields(int paletteType, char* palette, int dataAddress, int format) {
    ((TexturePaletteState*)palette)->paletteType = paletteType;
    ((PaletteDescriptor8042*)palette)->dataAddress = dataAddress;
    ((TexturePaletteState*)palette)->textureFormat = format;
    // GX TLUT formats: IA8 = 0, RGB565 = 1, RGB5A3 = 2. Keep the original ordered tests.
    if (format < PlatformRGB5A3) {
        if (format >= PlatformRGB565)
            ((PaletteDescriptor8042*)palette)->gxPaletteFormat = GXPaletteRGB565;
    } else if (format > PlatformRGB5A3) {
        if (format < (PlatformRGBA8 + 1))
            ((PaletteDescriptor8042*)palette)->gxPaletteFormat = GXPaletteIA8;
    } else {
        ((PaletteDescriptor8042*)palette)->gxPaletteFormat = GXPaletteRGB5A3;
    }
}

#pragma push
#pragma optimization_level 0
// Return bytes per texel for the supported RGBA8 and platform IA8 formats.
int TextureFormat_GetRGBA8OrIA8BytesPerTexel(int format) {
    switch (format) {
    case PlatformRGBA8:
    case FormatRGBA8:
        return 4;
    case PlatformIA8:
        return 2;
    }
    return 0;
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Return the byte stride of each supported packed flexible-vertex-format encoding.
// Proven layouts: float3 position=12, float3 normal=12, packed color=4,
// float2 UV=8; the named prefix layout adds one byte (no alignment padding).
// Float4 position/normal use 16 bytes each. Other encodings remain table keys:
// this table alone does not prove a general bitfield decoder. Unsupported => 0.
int FlexibleVertexFormat_GetStride(int value) {
    switch (value) {
    case 0x1:
        return 0x0C;
    case FVF_Position3_Normal3:
        return 0x18;
    case 0x11F401:
        return 0x24;
    case FVF_Position3_Normal3_TexCoord2:
        return 0x20;
    case 0x120401:
        return 0x28;
    case 0x130401:
        return 0x30;
    case 0x140401:
        return 0x38;
    case FVF_Position3_Color:
        return 0x10;
    case FVF_Position3_Normal3_Color:
        return 0x1C;
    case 0xF005:
        return 0x10;
    case 0x110005:
        return 0x14;
    case 0x130005:
        return 0x24;
    case 0x11F005:
        return 0x18;
    case 0x110001:
        return 0x14;
    case 0x11F001:
        return 0x18;
    case 0x12F001:
        return 0x20;
    case 0x13F001:
        return 0x28;
    case 0x14F001:
        return 0x30;
    case 0x40F121:
        return 0x18;
    case 0x12F005:
        return 0x20;
    case 0x13F005:
        return 0x28;
    case 0x14F005:
        return 0x30;
    case 0xF00D:
        return 0x10;
    case 0x11000D:
        return 0x18;
    case 0x400521:
        return 0x20;
    case FVF_Position4_Normal4_TexCoord2:
        return 0x28;
    case 0x520521:
        return 0x30;
    case 0x530521:
        return 0x38;
    case 0x510531:
        return 0x2C;
    case 0x520531:
        return 0x34;
    case 0x530531:
        return 0x3C;
    case 0x510541:
        return 0x30;
    case 0x520541:
        return 0x38;
    case 0x530541:
        return 0x40;
    case 0x800401:
        return 0x19;
    case FVF_PrefixByte_Position3_Normal3_TexCoord2:
        return 0x21;
    case 0x920401:
        return 0x29;
    case 0x930401:
        return 0x31;
    default:
        return 0;
    }
}
#pragma pop

#pragma optimize_for_size on
// Convert an index count into the number of complete primitives for the topology.
int GX_GetPrimitiveCount(unsigned int indexCount, int primitiveType) {
    // Caller supplies at least 2 indices for strips/fans and 1 for line strips:
    // subtraction is unsigned and underflows otherwise. Points return zero here.
    switch (primitiveType) {
    case PrimitiveTriangleFan:
    case PrimitiveTriangleStrip:
        return indexCount - 2;
    case PrimitiveLines:
        return indexCount >> 1;
    case PrimitiveTriangles:
        return indexCount / 3;
    case PrimitiveLineStrip:
        return indexCount - 1;
    case PrimitiveQuads:
        return indexCount >> 2;
    }
    return 0;
}
#pragma optimize_for_size reset

extern "C" void gtl_Image_GetDescription(void* out, void* out2, void* a, void* b);
extern "C" int gtl_ImageFormat_ToPlatformFormat(int a, int b);
#pragma push
#pragma optimization_level 0
// Export the image dimensions and format into the six-word platform-7 description.
// First two inputs are forwarded opaquely: their image/level roles are unresolved.
// out owns six writable words: {description[1], description[2], 0,
// description[3], convertedFormat, 3}; interpretation of the last tag is unresolved.
int gtl_Image_GetPlatform7Description(void* imageArgument0, void* imageArgument1, void* out) {
    int values[7];
    gtl_Image_GetDescription(values + 2, values, imageArgument0, imageArgument1);
    ((int*)out)[0] = values[3];
    ((int*)out)[1] = values[4];
    ((int*)out)[2] = 0;
    ((int*)out)[3] = values[5];
    ((int*)out)[4] = gtl_ImageFormat_ToPlatformFormat(values[2], PlatformFormat7);
    ((int*)out)[5] = 3;
    return 1;
}
#pragma pop

#include "classes/GtlPrimitive.h"
extern "C" void VertexFormat_BuildDeclaration(void* mask, void* entries);
extern "C" void* gtl_Graphics_AllocateWithCallback(unsigned int size);
// Build a vertex declaration, allocate a primitive, and publish the constructed object.
// argument0/1/2/4 are constructor pass-throughs with unresolved semantics.
// vertexFormat is consumed by BuildDeclaration; its pointer representation is ABI.
int gtl::IG3DXPrimitive::CreateFromFVF(void* argument0, void* argument1, void* argument2, void* vertexFormat,
                                       void* argument4, void** outPrimitive) {
    // Return is always 1, including allocation failure; inspect *outPrimitive.
    // A non-null result is callback-allocated and must use the graphics release API.
    void* volatile savedArgument0 = argument0;
    void* volatile savedArgument1 = argument1;
    void* volatile savedArgument2 = argument2;
    void* volatile savedVertexFormat = vertexFormat;
    void* volatile savedArgument4 = argument4;
    char vertexDeclaration[0x310];
    VertexFormat_BuildDeclaration(savedVertexFormat, vertexDeclaration);
    void* primitive = gtl_Graphics_AllocateWithCallback(0x24);
    if (primitive)
        ((gtl::IG3DXPrimitive*)primitive)
            ->Init((int)savedArgument0, (int)savedArgument1, (unsigned int)savedArgument2, vertexDeclaration,
                   savedArgument4);
    *outPrimitive = primitive;
    return 1;
}

extern "C" void* gtl_ImageFormat_FromPlatformFormat(void* p, int n);
extern "C" void gtl_Image_CopyToPlatformLayout(int a, int b, int c, int d, int e, int f, int g, void* p, int h);
extern "C" void gtl_Image_CopyFromPlatformLayout(int a, int b, int c, int d, int e, int f, int g, void* p, int h);

#pragma push
#pragma optimization_level 0
// Translate the supplied format and forward the image copy into platform-7 storage.
void Image_CopyToPlatformLayoutFormat7(int a, int b, int c, int d, int e, int f, int g, void* h) {
    void* p = gtl_ImageFormat_FromPlatformFormat(h, PlatformFormat7);
    gtl_Image_CopyToPlatformLayout(a, b, c, d, e, f, g, p, PlatformFormat7);
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Translate the supplied format and forward the image copy out of platform-7 storage.
void gtl_Image_CopyFromPlatform7Layout(int a, int b, int c, int d, int e, int f, int g, void* h) {
    void* p = gtl_ImageFormat_FromPlatformFormat(h, PlatformFormat7);
    gtl_Image_CopyFromPlatformLayout(a, b, c, d, e, f, g, p, PlatformFormat7);
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Write four-pixel runs into 4x4 GX RGBA8 tiles, separating AR and GB planes, then flush.
void Texture_ConvertRGBA8ToTiledARGBPlanes(unsigned char* sourceRGBA, unsigned int flushRowBytes,
                                           unsigned int unusedHeight, unsigned char* dst, unsigned int rowWidthPixels,
                                           unsigned int rows, unsigned int unused, unsigned int bytesPerPixel) {
    // Requires RGBA bytes (bytesPerPixel == 4), complete four-pixel runs and
    // caller-owned source/destination storage. No padding or bounds check is added.
    // Argument 2 affects only DCFlushRange; argument 5 controls pixel addressing.
    unsigned int flushByteCount = flushRowBytes * rows;
    unsigned int unusedTileCount = flushByteCount >> 6;
    unsigned int pixelIndex = 0;
    while (pixelIndex < rows * rowWidthPixels) {
        unsigned int tileOffset;
        unsigned char* sourceRun;
        unsigned char* tileRun;
        int pixelInRun;
        unsigned char* sourcePixel;
        int planePixelOffset;
        unsigned char* savedARPlane;
        unsigned char* gbPlane;
        unsigned int computedTileOffset;
        unsigned int columnInTile;
        unsigned int rowInTile;
        unsigned int tileColumn;
        unsigned int tileRow;
        unsigned int planeRowStride;
        unsigned int tileByteSize;
        unsigned int tileRowStride;
        unsigned int tileRowPixels;
        unsigned int tilePixels;
        unsigned int tileWidth;
        unsigned int tileHeight;
        unsigned char* sourceRunStart;
        unsigned char* tileRunStart;
        unsigned char* rgbaPixel;
        unsigned char* arPlane;
        unsigned char* gbPlaneStart;
        // Locate a 64-byte tile: 32 bytes AR followed by 32 bytes GB.
        // Within either plane, each tile row is 4 pixels * 2 bytes.
        if (bytesPerPixel == RGBA8BytesPerPixel) {
            tileHeight = TileWidth;
            tileRowPixels = rowWidthPixels * tileHeight;
            tileWidth = tileHeight;
            tileRowStride = tileRowPixels * 4;
            planeRowStride = tileHeight * 2;
            tileRow = pixelIndex / tileRowPixels;
            tileColumn = (pixelIndex % rowWidthPixels) / tileWidth;
            rowInTile = (pixelIndex % tileRowPixels) / rowWidthPixels;
            tilePixels = tileWidth * tileWidth;
            tileByteSize = tilePixels * 4;
            columnInTile = (pixelIndex % rowWidthPixels) % tileWidth;
            computedTileOffset =
                rowInTile * planeRowStride + tileRow * tileRowStride + (columnInTile * 2 + tileColumn * tileByteSize);
        } else {
            computedTileOffset = 0;
        }
        tileOffset = computedTileOffset;
        sourceRunStart = sourceRGBA + bytesPerPixel * pixelIndex;
        tileRunStart = dst + tileOffset;
        sourceRun = sourceRunStart;
        tileRun = tileRunStart;
        // Scatter the linear RGBA run into corresponding AR and GB plane slots.
        for (pixelInRun = 0; pixelInRun < TileWidth; pixelInRun++) {
            sourcePixel = sourceRun + bytesPerPixel * pixelInRun;
            planePixelOffset = pixelInRun * 2;
            rgbaPixel = sourcePixel;
            arPlane = tileRun + planePixelOffset;
            gbPlaneStart = planePixelOffset + tileRun + TilePlaneBytes;
            gbPlane = gbPlaneStart;
            arPlane[0] = rgbaPixel[3];
            arPlane[1] = rgbaPixel[0];
            savedARPlane = arPlane;
            gbPlane[0] = sourcePixel[1];
            gbPlane[1] = sourcePixel[2];
        }
        pixelIndex += TileWidth;
    }
    DCFlushRange(dst, flushRowBytes * rows);
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Store the texture description arguments and invoke the one-level storage calculation.
void TextureDescription_Initialize(int width, int height, int format, char* description, int storageArgument,
                                   int unused1, int sizeArgument, int unused2, int unused3) {
    int savedWidth = width;
    int savedHeight = height;
    int savedFormat = format;
    char* savedDescription = description;
    ((TextureDescription8042*)savedDescription)->width = savedWidth;
    ((TextureDescription8042*)savedDescription)->height = savedHeight;
    ((TextureDescription8042*)savedDescription)->format = savedFormat;
    ((TextureDescription8042*)savedDescription)->storageArgument = storageArgument;
    ((TextureDescription8042*)savedDescription)->sizeArgument =
        sizeArgument; // forwarded sizing argument, not the calculated byte count
    ((TextureDescription8042*)savedDescription)->unk_1C = 0;
    Texture_CalculateMipStorageSize(savedWidth, savedHeight, 1, savedFormat, sizeArgument, storageArgument,
                                    sizeArgument);
}
#pragma pop

// Initialize a type-6 resource header and its pointer payload.
void* GraphicsResource_InitType6(void* resource, void* payloadPointer) {
    void* volatile savedPayloadPointer = payloadPointer;
    GraphicsResource_InitializeHeader(resource, ResourceType6, 1, 0);
    ((ResourcePayload8042*)resource)->payload.pointer = savedPayloadPointer;
    return resource;
}

extern "C" void* gtl_Graphics_AllocateWithCallback(unsigned int size);

} // extern "C"

// Allocate a graphics buffer, construct it when allocation succeeds, and publish the pointer.
int gtl::IG3DXBuffer::Create(void* data, void** outBuffer) {
    // Return is always 1; allocation failure is reported by *outBuffer == 0.
    void* volatile savedData = data;
    gtl::IG3DXBuffer* buffer = (gtl::IG3DXBuffer*)gtl_Graphics_AllocateWithCallback(0x10);
    if (buffer)
        buffer->Init(savedData);
    *outBuffer = buffer;
    return 1;
}

extern "C" {

// Release the graphics object through the registered allocator callback.
void dtor_8042D58C(void* resource) {
    void* volatile savedResource = resource;
    gtl_Graphics_FreeWithCallback(resource);
}

// Release the graphics object through the registered allocator callback.
void dtor_8042D5B0(void* resource) {
    void* volatile savedResource = resource;
    gtl_Graphics_FreeWithCallback(resource);
}

} // extern "C"
