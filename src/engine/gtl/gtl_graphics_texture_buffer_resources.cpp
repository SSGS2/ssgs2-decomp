// Graphics resource storage: texture, index and vertex buffers, render surfaces and texture palettes.
// Each kind answers description and lock queries, flushes data for the GPU and frees its storage.
// The cube-face storage is here too.

#include "classes/RenderQueryOutput8042.h"

extern "C" {

#include "sdk_protos.h"

// Distinct query layouts share the legacy Fn8042Output pointer ABI. Description
// queries need nine words; lock/data queries write ONLY pitch and address. Never
// reuse a description's format/kind as a lock result without changing its view.
// Addresses are borrowed CPU storage, pitch is bytes (four texel rows for block
// compression), dimensions are texels, and storageBytes includes the selected mips.
struct GraphicsDescriptionResult {
    unsigned int format, resourceKind, usage, reserved0C, storageBytes;
    unsigned int reserved14, reserved18, width, height;
};
struct GraphicsLockResult {
    unsigned int pitchBytes, storageAddress;
};
typedef char DescriptionResultSize[(sizeof(GraphicsDescriptionResult) == sizeof(Fn8042Output)) ? 1 : -1];
typedef char LockResultSize[(sizeof(GraphicsLockResult) == 8) ? 1 : -1];

unsigned int Render_AlignSizeToMultiple(void* data, unsigned int size);

struct TextureMipRecord {
    unsigned int width;
    unsigned int height;
    unsigned int offset;
    unsigned int storageSize;
};

struct TextureMipLayout {
    char reserved00[4];
    unsigned int storageAddress;
    char reserved08[0xC];
    TextureMipRecord entries[8];
    unsigned int format;
};

// Kind-4 textures describe square faces using one edge length.
struct SquareTextureDescription {
    char reserved00[0x10];
    unsigned int usage;
    unsigned int edgeLength;
    unsigned int format;
};
// The byte cursor in GetLevelDescription points at each 16-byte mip record.
struct MipDescriptionCursor {
    char reserved00[0x14];
    unsigned int width, height, offset, storageSize;
};
enum GraphicsConstants {
    kCacheLineBytes = 0x20,
    kResourceKindShift = 13,
    kResourceKindMask = 7,
    kCubeFaceCount = 6,
    kPlatformWii = 7,
    kVertexBufferKind = 0,
    kIndexBufferKind = 1,
    kSurfaceKind = 2,
    kTexture2DKind = 3,
    kCubeTextureKind = 4,
    kPaletteKind = 5
};
// Resource header: refs in bits 16..31, kind in 13..15, allocation bookkeeping
// in 9..12, aligned allocator in bit 8, owned storage in bit 7. The +4 storage
// word is a base address; index/vertex payload access adds a four-byte prefix.
enum GraphicsStorageConstants {
    kDefaultMipLevels = 8,
    kBufferPrefixBytes = 4,
    kPackedRGBA8 = 6,
    kPlatformRGBA8 = (int)0x80000006
};
enum GraphicsHeaderBits {
    kHeaderLowHalfMask = 0xFFFF,
    kHeaderOneReference = 0x10000,
    kHeaderResourceKindMask = 0xE000,
    kHeaderAllocationBookkeepingMask = 0x1E00,
    kHeaderAlignedAllocator = 0x100,
    kHeaderOwnsStorage = 0x80
};
// Only the allocator selection is established here: 1 sets aligned callbacks,
// 0/2 select ordinary callbacks, and other values preserve the previous bit.
enum GraphicsAllocationMode {
    kOrdinaryAllocation = 0,
    kAlignedAllocation = 1,
    kOtherOrdinaryAllocation = 2,
    kKnownAllocationModeEnd = 3
};
// Decoder output format IDs; names state the proven storage density. Channel
// interpretation is left to the image-format layer rather than guessed here.
enum DecodedAlphaFormat {
    kHalfBytePerTexelFormat = 0x21,
    kBytePerTexelFormat = 0x1C
};
static const unsigned int kDXTAlpha4Format = 0xC0000030u;
static const unsigned int kDXTSeparateAlphaFormat = 0xC0000032u;

struct GraphicsStorageView {
    char reserved00[4];
    void* storage;
};

struct AssociatedGraphicsResource {
    char reserved00[0xC];
    void* associatedResource;
};

struct GraphicsAddressView {
    char reserved00[4];
    unsigned int storageAddress;
};

struct IndexBufferDescription {
    unsigned int flags;
    char reserved04[8];
    unsigned int indexCount;
    unsigned int format;
    unsigned int indexStride;
};

struct VertexBufferDescription {
    unsigned int flags;
    char reserved04[8];
    unsigned int byteCount;
    unsigned int vertexFormat;
};

struct GraphicsBufferStorage {
    char reserved00[4];
    void* storage;
    void* auxiliaryStorage;
    unsigned int byteCount;
};

struct TextureLevelCountView {
    char reserved00[0xC];
    unsigned int levelCount;
};

struct GraphicsResourceHeader {
    unsigned int flags;
    unsigned int storage;
    unsigned int auxiliaryStorage;
};

struct IndexBufferStorage {
    char reserved00[4];
    char* base;
    unsigned int reserved08;
    unsigned int count;
    unsigned int reserved10;
    unsigned int stride;
};

struct RenderSurfaceDescription {
    unsigned int flags;
    char reserved04[8];
    unsigned int width;
    unsigned int height;
    unsigned int format;
};

struct GraphicsResourceStorage {
    unsigned int flags;
    void* buffer;
};

struct GraphicsMemoryCallbacks {
    void* (*alloc)(unsigned int);
    void* releaseStorage;
    void* (*allocAligned)(unsigned int);
    void (*freeAligned)(void*);
    void* reservedCallback;
    void (*free)(void*);
};

extern GraphicsMemoryCallbacks* g_graphicsMemoryCallbacks;

unsigned int Texture_CalculateMipStorageSize(unsigned int, unsigned int, unsigned int, unsigned int);
unsigned int gtl_ImageFormat_FromPlatformFormat(unsigned int, unsigned int);
unsigned int gtl_Image_GetRowPitch(unsigned int, unsigned int, unsigned int);
int gtl_ImageFormat_IsBlockCompressed(unsigned int);

struct CallbackByteBuffer {
    void* storage;
    unsigned int size;
    char allocatorState[4];
    unsigned int capacity;
};

struct Fn8042TexBuf;

void ByteBuffer_InitEmpty(Fn8042TexBuf* self);
void ByteBuffer_ResizeZeroFilled(Fn8042TexBuf* self, unsigned int size);
void* ByteBuffer_DeletingDtor(Fn8042TexBuf* self, short flag);
void gtl_Image_CopyFromPlatformLayout(void*, unsigned int, unsigned int, unsigned int, void*, unsigned int,
                                      unsigned int, unsigned int, unsigned int);
void gtl_DXT_DecompressImage(void*, unsigned int, void*, unsigned int, unsigned int, unsigned int);
void gtl_Image_CopyToPlatformLayout(void*, unsigned int, unsigned int, void*, unsigned int, unsigned int, unsigned int,
                                    unsigned int, unsigned int);

struct Fn8042TexBuf {
    void* data;
    unsigned int field04;
    unsigned int field08;
    // Construct empty temporary conversion storage.
    Fn8042TexBuf() {
        ByteBuffer_InitEmpty(this);
    }
    // Release temporary pixels through the byte-buffer destructor.
    ~Fn8042TexBuf() {
        ByteBuffer_DeletingDtor(this, -1);
    }
};

struct RenderSurfaceFormatView {
    char reserved00[0x14];
    unsigned int format;
};

void GraphicsResource_InitializeHeader(GraphicsResourceHeader* self, unsigned int arg2, unsigned int arg3, int arg4);
int GraphicsResource_AllocateStorage(GraphicsResourceStorage* self, unsigned int size);
void TextureDescription_Initialize(unsigned int, unsigned int, unsigned int, void*, unsigned int, unsigned int,
                                   unsigned int);
void __dl__FPv(void*);
void* CubeResourceStorage_dtor(void* self, short flag);
void gtl_Graphics_FreeWithCallback(void* self);
void* memset(void*, int, unsigned int);
void TexturePalette_InitializeDescriptorFields(void*, void*, unsigned int, unsigned int, unsigned int);
unsigned int TexturePalette_GetEntryCountForType(unsigned int);
unsigned int Texture_CalculateMipStorageSize(unsigned int, unsigned int, unsigned int, unsigned int);

void* memcpy(void*, const void*, unsigned int);

void Texture_BuildMipLayout(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, void*,
                            unsigned int);

#pragma push
#pragma optimization_level 1
#pragma deadcode off
// Release a callback-owned byte buffer and optionally delete the wrapper; volatile allocator bookkeeping is intentional.
void* CallbackByteBuffer_Destroy(volatile CallbackByteBuffer* self, volatile short flag) {
    if (self) {
        if (self->storage != 0) {
            unsigned int prev = self->size;
            volatile unsigned char zero = 0;
            self->size = self->size - prev;
            volatile char* member = self->allocatorState;
            volatile unsigned char copy = zero;
            unsigned int unused = self->capacity;
            ((void (*)(void*))g_graphicsMemoryCallbacks->releaseStorage)(self->storage);
        }
        if (flag > 0) {
            __dl__FPv((void*)self);
        }
    }
    return (void*)self;
}
#pragma pop

void* __alloca(unsigned int);
#pragma push
#pragma optimization_level 0
#pragma deadcode on
#pragma use_lmw_stmw on
#pragma opt_dead_assignments on
// Split each compressed mip's color blocks from alpha data, decode alpha and append it in Wii layout.
int Texture_SplitCompressedColorAndAlphaMipData(char* dst, register char* src, register unsigned int width,
                                                register unsigned int height, register unsigned int format,
                                                register unsigned int levels) {
    // Borrowed, disjoint buffers; caller reserves the color chain followed by
    // the alpha chain. Dimensions are texels and are halved without clamping.
    // Return is always 1; valid format/level sizes are a caller precondition.
    (void)__alloca(0);
    char* sourceMip = src;
    char* colorOutput = dst;
    register unsigned int mipWidth = width;
    register unsigned int mipHeight = height;
    // Phase 1: gather the final eight color bytes of each 16-byte block.
    for (unsigned int i = 0; i < levels; i++) {
        unsigned int blockCount = (mipWidth >> 2) * (mipHeight >> 2);
        for (unsigned int j = 0; j < blockCount; j++) {
            char* from = sourceMip + j * 16 + 8;
            char* to = colorOutput + j * 8;
            memcpy(to, from, 8);
        }
        sourceMip += blockCount * 16;
        colorOutput += blockCount * 8;
        mipWidth >>= 1;
        mipHeight >>= 1;
    }
    // Phase 2: append decoded alpha mips after ALL color mips, not per mip.
    char* alphaOutput = colorOutput;
    sourceMip = src;
    mipWidth = width;
    mipHeight = height;
    for (unsigned int i = 0; i < levels; i++) {
        unsigned int pixelCount = mipWidth * mipHeight;
        unsigned int fmt = gtl_ImageFormat_FromPlatformFormat(format, kPlatformWii);
        Fn8042TexBuf srcTex;
        ByteBuffer_ResizeZeroFilled(&srcTex, pixelCount);
        unsigned int decodedFormat;
        unsigned int decodedBytes;
        void* srcData = srcTex.data;
        void* srcCopy = srcData;
        gtl_Image_CopyFromPlatformLayout(sourceMip, mipWidth, mipHeight, 0, srcCopy, mipWidth, 0, fmt, kPlatformWii);
        if (format == kDXTAlpha4Format) {
            decodedFormat = kHalfBytePerTexelFormat;
            decodedBytes = pixelCount >> 1;
        } else {
            decodedFormat = kBytePerTexelFormat;
            decodedBytes = pixelCount;
        }
        Fn8042TexBuf midTex;
        ByteBuffer_ResizeZeroFilled(&midTex, decodedBytes);
        void* fromData = srcTex.data;
        void* fromCopy;
        void* midData = midTex.data;
        fromCopy = fromData;
        void* midCopy = midData;
        gtl_DXT_DecompressImage(midCopy, decodedFormat, fromCopy, mipWidth, mipHeight, fmt);
        Fn8042TexBuf outTex;
        ByteBuffer_ResizeZeroFilled(&outTex, decodedBytes);
        void* outCopy;
        void* outData;
        void* midInputCopy;
        void* midInput = midTex.data;
        outData = outTex.data;
        midInputCopy = midInput;
        outCopy = outData;
        gtl_Image_CopyToPlatformLayout(midInputCopy, mipWidth, 0, outCopy, mipWidth, mipHeight, 0, decodedFormat,
                                       kPlatformWii);
        void* copyData = outTex.data;
        void* copyDataCopy = copyData;
        memcpy(alphaOutput, copyDataCopy, decodedBytes);
        sourceMip += mipWidth * mipHeight;
        alphaOutput += decodedBytes;
        mipWidth >>= 1;
        mipHeight >>= 1;
    }
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Initialize a surface description, allocate its backing pixels and flush the resource header.
RenderSurfaceFormatView* RenderSurface_Ctor(RenderSurfaceFormatView* self, unsigned int width, unsigned int height,
                                            volatile unsigned int format, volatile unsigned int allocationMode) {
    GraphicsResource_InitializeHeader((GraphicsResourceHeader*)self, kSurfaceKind, 1, 1);
    TextureDescription_Initialize(width, height, format, self, 0, 0, allocationMode);
    GraphicsResource_AllocateStorage((GraphicsResourceStorage*)self,
                                     Texture_CalculateMipStorageSize(width, height, 1, self->format));
    DCFlushRange(self, kCacheLineBytes);
    return self;
}
#pragma pop

void IndexBuffer_InitializeDescriptorFields(unsigned int, unsigned int, unsigned int, unsigned int, void*, unsigned int,
                                            unsigned int);
void VertexBuffer_InitializeDescriptorFields(unsigned int, unsigned int, unsigned int, unsigned int, void*,
                                             unsigned int);
unsigned int FlexibleVertexFormat_GetStride(unsigned int);
void VertexInput_FindAndCopyFormatDescriptor(void*, unsigned int);
unsigned int gtl_ImageFormat_GetBytesPerElement(unsigned int);

#pragma push
#pragma optimization_level 0
// Allocate aligned or ordinary backing storage according to flags and fill it with 0xFF bytes.
int GraphicsResource_AllocateStorage(GraphicsResourceStorage* self, unsigned int size) {
    if ((self->flags >> 8) & 1) {
        self->buffer = g_graphicsMemoryCallbacks->allocAligned(size);
    } else {
        self->buffer = g_graphicsMemoryCallbacks->alloc(size);
    }
    memset(self->buffer, 0xFF0000FF, size);
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 1
#pragma inline_depth(0)
#pragma use_lmw_stmw on
// Build a palette descriptor and allocate enough storage for its entry format.
GraphicsResourceStorage* TexturePalette_Ctor(GraphicsResourceStorage* self, void* paletteTypeWord, unsigned int unused,
                                             unsigned int textureFormat, unsigned int allocationMode) {
    // paletteTypeWord carries a numeric palette type in the legacy pointer ABI.
    unsigned int g4 = allocationMode;
    unsigned int g3 = textureFormat;
    void* paletteType = paletteTypeWord;
    GraphicsResourceStorage* s = self;
    GraphicsResource_InitializeHeader((GraphicsResourceHeader*)s, kPaletteKind, 1, (int)g4);
    TexturePalette_InitializeDescriptorFields(paletteType, s, 0, g3, g4);
    GraphicsResource_AllocateStorage(
        s, Texture_CalculateMipStorageSize(TexturePalette_GetEntryCountForType((unsigned int)paletteType), 1, 1, g3));
    return s;
}
#pragma pop

#pragma push
#pragma optimization_level 1
// Describe a square kind-4 texture, including total storage for the requested levels.
int Texture_GetKind4LevelDescription(void* self, unsigned int levels, GraphicsDescriptionResult* result) {
    // Output order is format, resource kind, usage, a zero word, byte size,
    // two zero words, width and height. Keep the three unresolved words zero.
    result->format = ((SquareTextureDescription*)self)->format;
    result->resourceKind = kCubeTextureKind;
    result->usage = ((SquareTextureDescription*)self)->usage;
    result->reserved0C = 0;
    result->storageBytes = Texture_CalculateMipStorageSize(((SquareTextureDescription*)self)->edgeLength,
                                                           ((SquareTextureDescription*)self)->edgeLength, levels,
                                                           ((SquareTextureDescription*)self)->format);
    result->reserved14 = 0;
    result->reserved18 = 0;
    result->width = ((SquareTextureDescription*)self)->edgeLength;
    result->height = ((SquareTextureDescription*)self)->edgeLength;
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Initialize a texture header, defaulting an unspecified level count to eight.
TextureLevelCountView* TextureResource_InitHeader(TextureLevelCountView* self, void* resourceKind,
                                                  unsigned int requestedLevels, void* allocationMode) {
    unsigned int levelCount;
    GraphicsResource_InitializeHeader((GraphicsResourceHeader*)self, (unsigned int)resourceKind, 1,
                                      (int)allocationMode);
    if (requestedLevels == 0) {
        levelCount = kDefaultMipLevels;
    } else {
        levelCount = requestedLevels;
    }
    self->levelCount = levelCount;
    return self;
}
#pragma pop

// Reset storage words and encode reference count, kind, ownership and allocation flags.
void GraphicsResource_InitializeHeader(GraphicsResourceHeader* self, unsigned int resourceKind,
                                       unsigned int ownsStorage, int allocationMode) {
    self->storage = 0;
    self->auxiliaryStorage = 0;
    unsigned int flags = self->flags;
    flags = (flags & kHeaderLowHalfMask) | kHeaderOneReference;
    flags = (flags & ~kHeaderResourceKindMask) | ((resourceKind << kResourceKindShift) & kHeaderResourceKindMask);
    flags &= ~kHeaderAllocationBookkeepingMask;
    if (allocationMode == kAlignedAllocation) {
        flags |= kHeaderAlignedAllocator;
    } else if (allocationMode >= kOrdinaryAllocation && allocationMode < kKnownAllocationModeEnd) {
        flags &= ~kHeaderAlignedAllocator;
    }
    flags = (flags & ~kHeaderOwnsStorage) | ((ownsStorage << 7) & kHeaderOwnsStorage);
    self->flags = flags;
}

#pragma push
#pragma optimization_level 1
// Flush the index payload plus its four-byte prefix, rounded to a cache line.
int IndexBuffer_Flush(IndexBufferStorage* self) {
    register unsigned int count;
    register unsigned int mul;
    if (self->stride == 0) {
        count = 1;
    } else {
        count = self->stride;
    }
    mul = count;
    unsigned int size = Render_AlignSizeToMultiple((void*)(self->count * mul + kBufferPrefixBytes), kCacheLineBytes);
    DCFlushRange(self->base, size);
    return 1;
}
#pragma pop

struct TextureMipRange {
    char reserved00[8];
    unsigned int offset;
    unsigned int size;
};

struct TextureMipRanges {
    char reserved00[4];
    char* base;
    char reserved08[0xC];
    TextureMipRange entries[1];
};

#pragma push
#pragma optimization_level 0
// Flush the modified mip region to memory for the GPU.
int Texture_UnlockLevel(TextureMipRanges* self, unsigned int index) {
    DCFlushRange(self->base + self->entries[index].offset, self->entries[index].size);
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Destroy graphics resource storage and optionally release its wrapper through the callback.
void* dtor_8042BD10(void* self, volatile short flag) {
    if (self) {
        CubeResourceStorage_dtor(self, false);
        if (flag > 0) {
            gtl_Graphics_FreeWithCallback(self);
        }
    }
    return self;
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Write format, resource kind, index count and stride into the four-word output.
int IndexBuffer_GetDescription(IndexBufferDescription* data, unsigned int* result) {
    register unsigned int type;
    result[0] = data->format;
    result[2] = data->indexCount;
    type = (data->flags >> kResourceKindShift) & kResourceKindMask;
    result[1] = type;
    result[3] = data->indexStride;
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Write the vertex-buffer kind, byte count and vertex format into the four-word output.
int VertexBuffer_GetDescription(VertexBufferDescription* data, unsigned int* result) {
    result[0] = 0x7FFFFFFF;
    unsigned int type = (data->flags >> kResourceKindShift) & kResourceKindMask;
    result[1] = type;
    result[2] = data->byteCount;
    result[3] = data->vertexFormat;
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 1
// Flush the vertex payload and its prefix, rounded to a cache line.
int GraphicsBuffer_Flush(GraphicsBufferStorage* data) {
    unsigned int size = Render_AlignSizeToMultiple((char*)data->byteCount + kBufferPrefixBytes, kCacheLineBytes);
    DCFlushRange(data->storage, size);
    return 1;
}
#pragma pop

// Return the backing storage pointer.
void* GraphicsResource_GetStorage(GraphicsStorageView* data) {
    return data->storage;
}

// Return success for the no-op graphics resource hook.
int GraphicsResource_NoOpSuccess() {
    return 1;
}

// Return the resource associated with this wrapper.
void* GraphicsResource_GetHeaderWord0C(AssociatedGraphicsResource* data) {
    return data->associatedResource;
}

#pragma push
#pragma optimization_level 1
// Report surface format, kind, storage size, width and height.
int RenderSurface_GetDescription(RenderSurfaceDescription* data, GraphicsDescriptionResult* result) {
    register RenderSurfaceDescription* self = data;
    register GraphicsDescriptionResult* out = result;
    register unsigned int type;
    out->format = self->format;
    type = (self->flags >> kResourceKindShift) & kResourceKindMask;
    out->resourceKind = type;
    out->storageBytes = Texture_CalculateMipStorageSize(self->width, self->height, 1, self->format);
    out->width = self->width;
    out->height = self->height;
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Report the selected mip's dimensions and storage size when backing storage exists.
int Texture_GetLevelDescription(char* data, unsigned int index, GraphicsDescriptionResult* result) {
    // index is caller-validated. On failure result is untouched; on success
    // only format, kind, size and dimensions are written, not the other words.
    if (((TextureMipLayout*)data)->storageAddress == 0) {
        return 0;
    }
    char* item = data + (index << 4);
    result->format = ((TextureMipLayout*)data)->format;
    result->height = ((MipDescriptionCursor*)item)->height;
    result->width = ((MipDescriptionCursor*)item)->width;
    result->storageBytes = ((MipDescriptionCursor*)item)->storageSize;
    result->resourceKind = (*(unsigned int*)data >> kResourceKindShift) & kResourceKindMask;
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 1
// Return the surface address and row pitch; compressed formats use four rows per block.
int RenderSurface_GetDataAndPitch(RenderSurfaceDescription* rawData, GraphicsLockResult* rawResult) {
    // Output words 0/1 are byte pitch/address, unlike the description queries.
    // Returned storage is borrowed; no allocation or reference increment occurs.
    register GraphicsLockResult* result = rawResult;
    register RenderSurfaceDescription* data = rawData;
    register unsigned int ptr;
    register unsigned int base = *(unsigned int*)&((GraphicsResourceStorage*)data)->buffer;
    result->storageAddress = base;
    ptr = gtl_ImageFormat_FromPlatformFormat(data->format, kPlatformWii);
    result->pitchBytes = gtl_Image_GetRowPitch(data->width, ptr, kPlatformWii);
    if (gtl_ImageFormat_IsBlockCompressed(ptr)) {
        result->pitchBytes <<= 2;
    }
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Return the palette backing address through the output parameter.
int TexturePalette_GetData(GraphicsAddressView* data, void** result) {
    register unsigned int v = data->storageAddress;
    *result = (void*)v;
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Return backing storage plus the caller's byte offset and the four-byte prefix.
// The legacy char* parameter encodes a numeric byte offset, not a source buffer.
// Caller validates range; returned storage is borrowed and the result is always 1.
int Resource_GetOffsetPtr(GraphicsAddressView* data, char* callerByteOffset, void* unused, void** result) {
    unsigned int storageBaseAddress = data->storageAddress;
    *result = callerByteOffset + storageBaseAddress + kBufferPrefixBytes;
    return 1;
}

// Return index storage plus the caller's byte offset and the four-byte prefix.
// Same offset-as-pointer ABI and unchecked borrowed-address contract as above.
int IndexBuffer_GetOffsetPtr(GraphicsAddressView* data, char* callerByteOffset, void* unused, void** result) {
    unsigned int storageBaseAddress = data->storageAddress;
    *result = callerByteOffset + storageBaseAddress + kBufferPrefixBytes;
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 1
#pragma inline_depth(0)
// Return a mip address/pitch and invalidate its cache range before CPU access.
int Texture_LockLevel(TextureMipLayout* self, unsigned int index, GraphicsLockResult* out) {
    // index must identify an allocated mip. Output words 0/1 are byte pitch/address.
    // This is cache management, not a mutex: pair CPU writes with UnlockLevel.
    unsigned int fmt;
    out->storageAddress = self->storageAddress + self->entries[index].offset;
    fmt = gtl_ImageFormat_FromPlatformFormat(self->format, kPlatformWii);
    out->pitchBytes = gtl_Image_GetRowPitch(self->entries[index].width, fmt, kPlatformWii);
    if (gtl_ImageFormat_IsBlockCompressed(fmt) != 0) {
        out->pitchBytes <<= 2;
    }
    DCInvalidateRange((void*)out->storageAddress, self->entries[index].storageSize);
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma inline_depth(0)
// Repeat each input halfword a requested number of times; zero means one copy.
int RepeatHalfwords(void* dest, void* src, unsigned int size, unsigned int unused, unsigned int repeatCount) {
    // size is source bytes (odd trailing byte ignored); destination needs
    // 2*(size/2)*max(repeatCount,1) bytes. Returns 1 without validating buffers.
    char* output = (char*)dest;
    char* input = (char*)src;
    unsigned int halfwordBytes = 2;
    unsigned int halfwordCount = size / halfwordBytes;
    unsigned int i = 0;
    while (i < halfwordCount) {
        unsigned int copies;
        unsigned int copiesRemaining;
        copies = (repeatCount == 0) ? 1 : repeatCount;
        copiesRemaining = copies;
        unsigned int j = 0;
        while (j < copiesRemaining) {
            memcpy(output, input, halfwordBytes);
            output += halfwordBytes;
            j++;
        }
        input += halfwordBytes;
        i++;
    }
    return 1;
}
#pragma pop

#pragma inline_depth(0)
// Build mip descriptors and allocate pixels, adding separate alpha storage for the special DXT format.
void* Texture_Ctor2D(void* self, unsigned int width, unsigned int height, unsigned int levels, unsigned int layoutFlags,
                     unsigned int format, unsigned int allocationMode) {
    volatile unsigned int saved = layoutFlags;
    TextureResource_InitHeader((TextureLevelCountView*)self, (void*)kTexture2DKind, levels, (void*)allocationMode);
    Texture_BuildMipLayout(width, height, levels, saved, format, allocationMode, self, 0);
    unsigned int platformFormat = ((TextureMipLayout*)self)->format;
    unsigned int storageBytes = Texture_CalculateMipStorageSize(width, height, levels, platformFormat);
    storageBytes = (format != kDXTSeparateAlphaFormat) ? storageBytes : storageBytes + ((int)storageBytes >> 1);
    GraphicsResource_AllocateStorage((GraphicsResourceStorage*)self, storageBytes);
    return self;
}

#pragma push
#pragma optimization_level 0
// Initialize an index buffer, allocate/cache-align its payload and clear it.
void* IndexBuffer_Ctor(IndexBufferDescription* self, unsigned int indexCount, unsigned int usage, unsigned int format,
                       unsigned int allocationMode, unsigned int indexStride) {
    GraphicsResource_InitializeHeader((GraphicsResourceHeader*)self, kIndexBufferKind, 1, (int)allocationMode);
    self->indexCount = indexCount;
    self->format = format;
    IndexBuffer_InitializeDescriptorFields(indexCount, usage, format, allocationMode, (char*)self, 0, indexStride);
    unsigned int v;
    v = self->indexStride == 0 ? 1 : self->indexStride;
    unsigned int size = Render_AlignSizeToMultiple((void*)(indexCount * v + kBufferPrefixBytes), kCacheLineBytes);
    GraphicsResource_AllocateStorage((GraphicsResourceStorage*)self, size);
    memset(((GraphicsResourceStorage*)self)->buffer, 0, size);
    DCFlushRange(self, 0x18);
    return (void*)self;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Copy packed palette entries or split alternating halfwords into two planes.
int TexturePalette_CopyOrDeinterleaveEntries(void* dest, void* src, unsigned int paletteType, int textureFormat) {
    // Caller owns both buffers. Platform RGBA8 writes two halfword planes;
    // packed RGBA8 copies entries verbatim. Unsupported formats still return 1.
    switch (textureFormat) {
    case kPlatformRGBA8: {
        unsigned short* d = (unsigned short*)dest;
        unsigned short* s = (unsigned short*)src;
        unsigned int n = TexturePalette_GetEntryCountForType(paletteType);
        for (unsigned int i = 0; i < n; i++) {
            d[i] = s[i * 2];
            d[i + n] = s[i * 2 + 1];
        }
        break;
    }
    case kPackedRGBA8: {
        unsigned int count = gtl_ImageFormat_FromPlatformFormat(textureFormat, kPlatformWii);
        unsigned int stride = TexturePalette_GetEntryCountForType(paletteType);
        unsigned int size = gtl_ImageFormat_GetBytesPerElement(count) * stride;
        memcpy(dest, src, size);
        break;
    }
    }
    return 1;
}
#pragma pop

struct CubeResourceStorage {
    int flags;
    void* storage;
    char reserved08[0x14];
    void* faces[6];
    void* auxiliaryResource;
};

void GraphicsResource_Release(void*);

#pragma push
#pragma optimization_level 0
// Release cube faces and owned pixel storage, then optionally free the resource header.
void* CubeResourceStorage_dtor(void* rawSelf, short rawFlag) {
    CubeResourceStorage* base;
    int i;
    if (rawSelf != 0) {
        if (((((CubeResourceStorage*)rawSelf)->flags >> kResourceKindShift) & 7) == 4) {
            base = (CubeResourceStorage*)rawSelf;
            for (i = 0; i < kCubeFaceCount; i++) {
                void* p = base->faces[i];
                if (p != 0) {
                    GraphicsResource_Release(p);
                    p = 0;
                }
            }
            void* q = base->auxiliaryResource;
            if (q != 0) {
                GraphicsResource_Release(q);
                q = 0;
            }
        }
        if (((CubeResourceStorage*)rawSelf)->storage != 0) {
            if ((((CubeResourceStorage*)rawSelf)->flags >> 7) & 1) {
                if ((((CubeResourceStorage*)rawSelf)->flags >> 8) & 1) {
                    g_graphicsMemoryCallbacks->freeAligned(((CubeResourceStorage*)rawSelf)->storage);
                } else {
                    ((void (*)(void*))g_graphicsMemoryCallbacks->releaseStorage)(
                        ((CubeResourceStorage*)rawSelf)->storage);
                }
            }
        }
        ((CubeResourceStorage*)rawSelf)->storage = 0;
        if (rawFlag > 0)
            gtl_Graphics_FreeWithCallback(rawSelf);
    }
    return rawSelf;
}
#pragma pop

// Allocate vertex storage and any extra native-format stream, then clear and flush the header.
void* VertexBuffer_Ctor(GraphicsResourceStorage* self, unsigned int byteCount, unsigned int usage,
                        unsigned int vertexFormat, unsigned int allocationMode) {
    char local[0x30];
    GraphicsResource_InitializeHeader((GraphicsResourceHeader*)self, kVertexBufferKind, 1, (int)allocationMode);
    VertexBuffer_InitializeDescriptorFields(byteCount, usage, vertexFormat, allocationMode, (char*)self, 0);
    unsigned int size = Render_AlignSizeToMultiple((void*)(byteCount + kBufferPrefixBytes), kCacheLineBytes);
    if (vertexFormat != 0) {
        VertexInput_FindAndCopyFormatDescriptor(local + 0xC, vertexFormat);
        unsigned int nativeStride = FlexibleVertexFormat_GetStride(vertexFormat);
        bool needsConvertedStream = nativeStride - *(unsigned int*)(local + 0x24);
        if (needsConvertedStream) {
            unsigned int vertexCount = byteCount / FlexibleVertexFormat_GetStride(vertexFormat);
            size += Render_AlignSizeToMultiple(
                (void*)(*(unsigned int*)(local + 0x24) * vertexCount + kBufferPrefixBytes), kCacheLineBytes);
        }
    }
    GraphicsResource_AllocateStorage(self, size);
    memset(self->buffer, 0, size);
    DCFlushRange(self, 0x14);
    return (void*)self;
}

} // extern "C"
