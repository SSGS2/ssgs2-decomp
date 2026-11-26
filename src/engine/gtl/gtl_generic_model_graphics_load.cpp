// GTL model graphics upload and release: textures/palettes, vertex declarations,
// vertex/index buffers and optional secondary vertex-buffer slots.
// Mod index: external vs embedded texture choice -> LoadGraphicsResources desc+0;
// creation flags -> desc+4..13; secondary-buffer slot allocation -> desc+18.
// Chunk lookup/format rules: gtl_image_decode_and_tiling.cpp; allocator/vector storage destruction:
// gtl_model_resource_storage.cpp. These methods release element references but keep vector capacity.
// There are no gameplay tuning constants here. Input is a trusted native-word
// resource file (32-bit Wii big-endian); offsets below are bytes, not pointers
#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>

extern "C" {

void CubeResourceStorage_dtor(void*, int);
void* Object_GetField8(void* obj);
void* gtl_GenericModel_GetDataAtOffset(void* base, unsigned int offset);
void* gtl_GenericModel_GetIndexDataAtOffset(void* base, unsigned int offset);
int gtl_ResourceFile_GetChunkElementCount(void* file, void* data, unsigned int tag);
void* gtl_ResourceFile_GetChunkElement(void* file, void* data, unsigned int tag, unsigned int index);
void* gtl_ResourceFile_FindChunk(void* file, void* data, unsigned int tag);
void gtl_TextureVector_Resize(void* vec, unsigned int count);
void** gtl_TextureVector_GetDestinationSlot(void* vec, unsigned int index);
void gtl_TexturePaletteVector_Resize(void* vec, unsigned int count);
void** gtl_TexturePaletteVector_GetDestinationSlot(void* vec, unsigned int index);
void gtl_VertexDeclarationVector_Resize(void* vec, unsigned int count);
void** gtl_VertexDeclarationVector_GetDestinationSlot(void* vec, unsigned int index);
void gtl_VertexBufferVector_Resize(void* vec, unsigned int count);
void** gtl_VertexBufferVector_GetDestinationSlot(void* vec, unsigned int index);
void gtl_IndexBufferVector_Resize(void* vec, unsigned int count);
void** gtl_IndexBufferVector_GetDestinationSlot(void* vec, unsigned int index);
unsigned int gtl_GenericResourceVector_GetSize(void* vec);
unsigned int gtl_GenericRenderer_GetIndexedAttributeCount(void* decl);
void Texture_Create2D(unsigned int w, unsigned int h, unsigned int d, unsigned int pool, unsigned int format,
                      unsigned int flags, void** tex, unsigned int unk);
void Texture_LockLevel(void* tex, unsigned int level, void* lock, unsigned int a, unsigned int b);
void Texture_UnlockLevel(void* tex, unsigned int level);
int gtl_ImageFormat_FromPlatformFormat(unsigned int format, unsigned int x);
int gtl_ImageFormat_IsPaletted(int format);
unsigned int TexturePalette_GetTypeForEntryCount(unsigned int count);
void TexturePalette_Create(unsigned int type, unsigned int a, unsigned int b, unsigned int c, void** pal,
                           unsigned int d);
void TexturePalette_GetData(void* pal, void** out, unsigned int x);
void TexturePalette_CopyOrDeinterleaveEntries(void* dst, void* src, unsigned int count, unsigned int stride);
void TexturePalette_Flush(void* pal);
void VertexDeclaration_CreateCopy(void* src, void** out);
void VertexBuffer_Create(unsigned int count, unsigned int a, unsigned int vertexFormat, unsigned int c, void** out,
                         unsigned int d);
void Resource_GetOffsetPtr(void* res, unsigned int a, unsigned int size, void** out, unsigned int b);
void GraphicsBuffer_Flush(void* buf);
void IndexBuffer_Create(unsigned int a, unsigned int b, unsigned int c, unsigned int d, void** out, unsigned int e,
                        unsigned int f);
void IndexBuffer_GetOffsetPtr(void* ib, unsigned int a, unsigned int size, void** out, unsigned int b);
void IndexBuffer_Flush(void* ib);
void RepeatHalfwords(void* dst, void* src, unsigned int sourceBytes, unsigned int unusedFormat,
                     unsigned int repetitions);
void ImageResource_LoadFileAndDecode(const char* path, void** tex, void** pal, void* info, void* extra);
extern char s_sharedStringPool[];
extern char s_textureDirectoryPrefix[];

enum ModelChunkTag {
    kChunkTextureNames = 0x6E786574,
    kChunkStringBytes = 0x646273,
    kChunkTextureDescriptions = 0x786574,
    kChunkTextureBytes = 0x64786574,
    kChunkPaletteDescriptions = 0x6C6170,
    kChunkPaletteBytes = 0x646C6170,
    kChunkVertexDeclarationIndices = 0x6476,
    kChunkVertexDeclarations = 0x646576,
    kChunkVertexBytes = 0x646276,
    kChunkPrimitives = 0x6D7270,
    kChunkVertexBufferHeaders = 0x686276,
    kChunkIndexBufferHeaders = 0x686269,
    kChunkIndexBytes = 0x646269
};

struct GtlRefCounted {
    CW_BITS2(unsigned int refs : 16, unsigned int flags : 16)
};

// Six adjacent 12-byte vector prefixes: {owned slot array, live count, capacity}.
// Each nonnull slot owns one reference (first word high 16 bits); vector storage
// itself is freed by the enclosing destructor, not ReleaseGraphicsResources.
// Offset-to-role: +00 textures, +0C palettes, +18 vertex buffers,
// +24 vertex declarations, +30 index buffers, +3C secondary vertex buffers.
// Named arrays/counts below preserve the six adjacent vector prefixes.
struct GtlGenericModel {
    void** textures;
    int textureCount;
    char textureCapacity[4];
    void** texturePalettes;
    int texturePaletteCount;
    char texturePaletteCapacity[4];
    void** vertexBuffers;
    int vertexBufferCount;
    char vertexBufferCapacity[4];
    void** vertexDeclarations;
    int vertexDeclarationCount;
    char vertexDeclarationCapacity[4];
    void** indexBuffers;
    int indexBufferCount;
    char indexBufferCapacity[4];
    void** secondaryVertexBuffers;
    int secondaryVertexBufferCount;
};

struct GtlResourceFile {
    char unk0[0xC];
    void* resolverContext; // forwarded resolver context; current chunk resolvers ignore it
};

struct GtlChunk {
    int tag;                   // serialized directory tag
    unsigned int elementCount; // element count; full directory entry is 16 bytes
};

struct GtlModelDesc {
    int loadExternalTextures;               // enable external texture loading when a texn chunk exists
    char texParamsBlock[0x14];              // +4..13 GtlTexParams; +14..17 not read here
    int allocateSecondaryVertexBufferSlots; // allocate secondary VB slots; does not populate those buffers
};

struct GtlTexParams {
    unsigned int textureCreationFlags;      // texture creation flags; stored in ignored external-load packet too
    unsigned int paletteCreationFlags;      // palette creation flags; external decoder reads request+10
    unsigned int vertexBufferCreationFlags; // vertex-buffer creation flags; separate from vbh vertex format
    unsigned int indexBufferCreationFlags;  // index-buffer creation flags
};

extern GtlTexParams s_defaultTextureParams;

struct GtlTexLock {
    unsigned int unk0;
    void* data;
};

// External-image request, 0x38 bytes, native Wii u32 words. This combines
// the old adjacent GtlExtra (12 bytes) / GtlImageLoadInfo (44 bytes) views:
// old compiled caller: request@SP+30, old info@SP+3C, palette flags@SP+40.
// The callee reads fifthArgument+10 as palette creation flags, so splitting
// these into independent locals concealed an actual cross-object dependency.
// Fourth argument is request+14 and is forwarded to an UNUSED decoder input;
// it is not an image-description output. Only paletteCreationFlags is consumed
// by the present decoder (Texture_LoadResourceWithOptionalPalette, engine_render_cache_and_texture_load.cpp).
// All other words remain opaque/unused, including the stored texture flags.
// Fields without stores are deliberately uninitialized and must not be read.
struct GtlExternalImageRequest {
    unsigned int unusedPrefix[3];      // +00..0B: not initialized or consumed
    unsigned int legacyPaletteWord;    // +0C: stored 0x200; decoder does not read
    unsigned int paletteCreationFlags; // +10: input to TexturePalette_Create
    unsigned int unusedDecodeWords[2]; // +14/+18: fourth argument; not consumed
    unsigned int legacyTextureWord;    // +1C: stored 0x200; decoder does not read
    unsigned int textureCreationFlags; // +20: stored, not consumed by decoder
    char unusedTail[0x14];             // +24..37: not initialized or consumed
};
typedef char GtlExternalRequestSize[sizeof(GtlExternalImageRequest) == 0x38 ? 1 : -1];
typedef char GtlExternalPaletteOffset[(int)&((GtlExternalImageRequest*)0)->paletteCreationFlags == 0x10 ? 1 : -1];

struct GtlTexElem {
    unsigned int unk0;              // external texture entry word not consumed here
    unsigned int texturePathOffset; // byte offset to NUL-terminated path in sbd chunk
};

struct GtlTexDesc {
    unsigned int platformFormat;  // platform format, interpreted with platform table 5
    unsigned int widthPixels;     // width in pixels
    unsigned int height;          // height in pixels
    unsigned int mipLevelCount;   // mip/depth argument passed to Texture_Create2D
    unsigned int level0ByteCount; // bytes copied into locked level zero
    unsigned int texDataOffset;   // byte offset into texd data
};

struct GtlPalDesc {
    unsigned int paletteFormat;         // palette format
    unsigned int entryCount;            // palette entry count (converted to palette type)
    unsigned int paletteDataByteOffset; // byte offset into pald data
};

struct GtlVBData {
    unsigned int vertexFormatFlags;    // vertex format FLAGS (vbh word 0), not a byte stride
    unsigned int payloadBytes;         // allocation/lock/copy size argument, in bytes for copy
    unsigned int vertexDataByteOffset; // byte offset into vbd data
};

struct GtlIBData {
    unsigned int indexDataSourceBytes; // index-buffer size/copy-count argument
    unsigned int indexDataByteOffset;  // byte offset into ibd data; copied as repeated halfwords
};

struct GtlMeshDesc {
    char unk0[0x10];
    unsigned short indexBufferSlot;   // index-buffer slot, selecting GtlIBData
    unsigned short vertexBufferIndex; // vertex-buffer slot, selecting GtlVBData
    char unk14[4];
    unsigned short vertexDeclarationIndex; // vertex-declaration slot for indexed attribute count
    char unk1A[6];
};

// Relinquish one graphics reference in every slot of [begin,end), destroying at
// zero and clearing released slots to nul (all callers pass null). Borrow the
// slot array; count/capacity and allocation remain the caller's responsibility.
static inline void GtlGenericModel_ReleaseRange(void** begin, void** end, void* nul) {
    while (begin != end) {
        GtlRefCounted* obj = (GtlRefCounted*)*begin;
        if (obj != 0) {
            if (--obj->refs == 0) {
                CubeResourceStorage_dtor(obj, 1);
            }
            *begin = nul;
        }
        begin++;
    }
}

// Retain the original subtraction adapter used by reconstructed vector code.
static inline int GtlSub(int a, int b) {
    return a - b;
}

// Required model/vector prefixes, serialized against rendering/loading. Release
// in texture, palette, VB, declaration, IB, secondary-VB order, then zero each
// count. Keep the subtraction-shaped count clears: their memory reloads are part
// of the reconstructed vector operation. Capacities and slot-array addresses
// survive, allowing later Resize to reuse storage. No failure status is returned.
void gtl_GenericModel_ReleaseGraphicsResources(GtlGenericModel* self) {
    {
        void** begin = self->textures;
        void* nul = 0;
        void** end = begin + self->textureCount;
        GtlGenericModel_ReleaseRange(begin, end, nul);
        self->textureCount = self->textureCount - (int)*(unsigned int*)&self->textureCount;
    }
    {
        void** begin = self->texturePalettes;
        void* nul = 0;
        void** end = begin + self->texturePaletteCount;
        GtlGenericModel_ReleaseRange(begin, end, nul);
        self->texturePaletteCount = self->texturePaletteCount - (int)*(unsigned int*)&self->texturePaletteCount;
    }
    {
        void** begin = self->vertexBuffers;
        void* nul = 0;
        void** end = begin + self->vertexBufferCount;
        GtlGenericModel_ReleaseRange(begin, end, nul);
        self->vertexBufferCount = self->vertexBufferCount - (int)*(unsigned int*)&self->vertexBufferCount;
    }
    {
        void** begin = self->vertexDeclarations;
        void* nul = 0;
        void** end = begin + self->vertexDeclarationCount;
        GtlGenericModel_ReleaseRange(begin, end, nul);
        self->vertexDeclarationCount =
            self->vertexDeclarationCount - (int)*(unsigned int*)&self->vertexDeclarationCount;
    }
    {
        void** begin = self->indexBuffers;
        void* nul = 0;
        void** end = begin + self->indexBufferCount;
        GtlGenericModel_ReleaseRange(begin, end, nul);
        self->indexBufferCount = self->indexBufferCount - (int)*(unsigned int*)&self->indexBufferCount;
    }
    {
        void** begin = self->secondaryVertexBuffers;
        void* nul = 0;
        void** end = begin + self->secondaryVertexBufferCount;
        GtlGenericModel_ReleaseRange(begin, end, nul);
        self->secondaryVertexBufferCount =
            self->secondaryVertexBufferCount - (int)*(unsigned int*)&self->secondaryVertexBufferCount;
    }
}

#pragma use_lmw_stmw on
// Replace all graphics resources of self from loader's backing file. loader,
// self and desc must be valid; required chunks/indices/offset extents must already
// be validated. desc must not alias the vectors being cleared. No null/length
// checks or rollback: old references are released first, each new resource is
// published directly into an owning slot. Exceptions or callee failures can
// leave partial arrays; callers must release them before retry/destruction.
// Returns 1 after traversal, not aggregate upload success. Missing mandatory
// declarations/primitive chunks may dereference null; absent optional textures
// or ibh simply skip those paths. A texn chunk takes priority even when desc+0
// disables external loads: it creates null slots, without embedded fallback.
// The params==0 default test is retained verbatim; with a valid desc, desc+4
// cannot be null. Do not use a null desc as a request for defaults.
// Four phases below: textures -> declarations -> VB/IB upload -> secondary slots.
int gtl_GenericModel_LoadGraphicsResources(void* loader, GtlGenericModel* self, GtlModelDesc* desc) {
    void* dbs;
    GtlTexElem* tex;
    GtlChunk* chunk;
    GtlResourceFile* file;
    GtlTexParams* params;
    GtlExternalImageRequest externalImageRequest;
    file = (GtlResourceFile*)Object_GetField8(loader);
    gtl_GenericModel_ReleaseGraphicsResources(self);
    params = (GtlTexParams*)((char*)desc + 4);
    if (params == 0) {
        params = &s_defaultTextureParams;
    }
    // Chunk tags are serialized numeric words, conventionally read low byte
    // first: texn=0x6E786574 (external names), sbd=0x00646273 (string bytes),
    // tex/texd=0x00786574/0x64786574, pal/pald=0x006C6170/0x646C6170.
    // External path formatting uses s_sharedStringPool+4B and s_textureDirectoryPrefix; the
    // 256-byte stack path is unchecked sprintf storage, not a truncating buffer.
    if (gtl_ResourceFile_GetChunkElementCount(file, file->resolverContext, kChunkTextureNames) != 0) {
        chunk = (GtlChunk*)gtl_ResourceFile_FindChunk(file, file->resolverContext, kChunkTextureNames);
        tex = (GtlTexElem*)gtl_ResourceFile_GetChunkElement(file, file->resolverContext, kChunkTextureNames, 0);
        dbs = gtl_ResourceFile_GetChunkElement(file, file->resolverContext, kChunkStringBytes, 0);
        gtl_TextureVector_Resize(self, chunk->elementCount);
        gtl_TexturePaletteVector_Resize(&self->texturePalettes, chunk->elementCount);
        for (unsigned int i = 0; i < chunk->elementCount; i++) {
            void* tex_out = 0;
            void* pal_out = 0;
            if (desc->loadExternalTextures != 0) {
                char path[0x100];
                sprintf(path, s_sharedStringPool + 0x4B, s_textureDirectoryPrefix,
                        gtl_GenericModel_GetDataAtOffset(dbs, tex->texturePathOffset));
                externalImageRequest.textureCreationFlags = params->textureCreationFlags;
                externalImageRequest.legacyTextureWord = 0x200;
                externalImageRequest.paletteCreationFlags = params->paletteCreationFlags;
                externalImageRequest.legacyPaletteWord = 0x200;
                // Outputs begin null. Open failure clears both; read failure
                // preserves them; decode can publish a texture before failure.
                // Status is ignored and whatever handles remain are adopted by
                // the model. Call consumes path synchronously; the request is not
                // retained. Fifth argument spans the complete
                // request; only its +10 palette flags are consumed.
                ImageResource_LoadFileAndDecode(path, &tex_out, &pal_out, &externalImageRequest.unusedDecodeWords[0],
                                                &externalImageRequest);
            }
            *gtl_TextureVector_GetDestinationSlot(self, i) = tex_out;
            *gtl_TexturePaletteVector_GetDestinationSlot(&self->texturePalettes, i) = pal_out;
            tex++;
        }
    } else if (gtl_ResourceFile_GetChunkElementCount(file, file->resolverContext, kChunkTextureDescriptions) != 0 &&
               gtl_ResourceFile_GetChunkElementCount(file, file->resolverContext, kChunkTextureBytes) != 0) {
        unsigned int texCount =
            gtl_ResourceFile_GetChunkElementCount(file, file->resolverContext, kChunkTextureDescriptions);
        unsigned int palCount =
            gtl_ResourceFile_GetChunkElementCount(file, file->resolverContext, kChunkPaletteDescriptions);
        GtlTexDesc* texDesc =
            (GtlTexDesc*)gtl_ResourceFile_GetChunkElement(file, file->resolverContext, kChunkTextureDescriptions, 0);
        GtlPalDesc* palDesc =
            (GtlPalDesc*)gtl_ResourceFile_GetChunkElement(file, file->resolverContext, kChunkPaletteDescriptions, 0);
        void* dtx = gtl_ResourceFile_GetChunkElement(file, file->resolverContext, kChunkTextureBytes, 0);
        void* dpl = gtl_ResourceFile_GetChunkElement(file, file->resolverContext, kChunkPaletteBytes, 0);
        gtl_TextureVector_Resize(self, texCount);
        gtl_TexturePaletteVector_Resize(&self->texturePalettes, palCount);
        for (unsigned int i = 0; i < texCount; i++) {
            void** slot = gtl_TextureVector_GetDestinationSlot(self, i);
            Texture_Create2D(texDesc->widthPixels, texDesc->height, texDesc->mipLevelCount, 0x200,
                             texDesc->platformFormat, params->textureCreationFlags, slot, 0);
            GtlTexLock lock;
            Texture_LockLevel(*slot, 0, &lock, 0, 0);
            Memory_Copy(lock.data, gtl_GenericModel_GetDataAtOffset(dtx, texDesc->texDataOffset),
                        texDesc->level0ByteCount);
            Texture_UnlockLevel(*slot, 0);
            if (gtl_ImageFormat_IsPaletted(gtl_ImageFormat_FromPlatformFormat(texDesc->platformFormat, 5)) != 0) {
                void** pslot = gtl_TexturePaletteVector_GetDestinationSlot(&self->texturePalettes, i);
                TexturePalette_Create(TexturePalette_GetTypeForEntryCount(palDesc->entryCount), 0,
                                      palDesc->paletteFormat, params->paletteCreationFlags, pslot, 0);
                unsigned int type = TexturePalette_GetTypeForEntryCount(palDesc->entryCount);
                void* pdata;
                TexturePalette_GetData(*pslot, &pdata, 0);
                TexturePalette_CopyOrDeinterleaveEntries(
                    pdata, gtl_GenericModel_GetDataAtOffset(dpl, palDesc->paletteDataByteOffset), type,
                    palDesc->paletteFormat);
                TexturePalette_Flush(*pslot);
            }
            texDesc++;
            palDesc++;
        }
    }
    // vd (0x6476) holds declaration indices into ved (0x646576).
    // prm (0x6D7270) records select vbh (0x686276) and optional ibh (0x686269)
    // records; vbd (0x646276) / ibd (0x646269) supply their payload bytes.
    // Slot indices and declared vector sizes must agree; there is no remapping.
    GtlChunk* declChunk =
        (GtlChunk*)gtl_ResourceFile_FindChunk(file, file->resolverContext, kChunkVertexDeclarationIndices);
    gtl_VertexDeclarationVector_Resize(&self->vertexDeclarations, declChunk->elementCount);
    for (unsigned int i = 0; i < declChunk->elementCount; i++) {
        unsigned int* declIndex = (unsigned int*)gtl_ResourceFile_GetChunkElement(file, file->resolverContext,
                                                                                  kChunkVertexDeclarationIndices, i);
        void* declSrc =
            gtl_ResourceFile_GetChunkElement(file, file->resolverContext, kChunkVertexDeclarations, *declIndex);
        void* decl;
        VertexDeclaration_CreateCopy(declSrc, &decl);
        *gtl_VertexDeclarationVector_GetDestinationSlot(&self->vertexDeclarations, i) = decl;
    }
    void* dbv = gtl_ResourceFile_GetChunkElement(file, file->resolverContext, kChunkVertexBytes, 0);
    GtlChunk* vbChunk = (GtlChunk*)gtl_ResourceFile_FindChunk(file, file->resolverContext, kChunkPrimitives);
    GtlMeshDesc* mesh =
        (GtlMeshDesc*)gtl_ResourceFile_GetChunkElement(file, file->resolverContext, kChunkPrimitives, 0);
    gtl_VertexBufferVector_Resize(&self->vertexBuffers, vbChunk->elementCount);
    GtlVBData* vbd =
        (GtlVBData*)gtl_ResourceFile_GetChunkElement(file, file->resolverContext, kChunkVertexBufferHeaders, 0);
    GtlChunk* ibChunk = (GtlChunk*)gtl_ResourceFile_FindChunk(file, file->resolverContext, kChunkIndexBufferHeaders);
    if (ibChunk != 0) {
        gtl_IndexBufferVector_Resize(&self->indexBuffers, ibChunk->elementCount);
    }
    GtlIBData* ibd =
        (GtlIBData*)gtl_ResourceFile_GetChunkElement(file, file->resolverContext, kChunkIndexBufferHeaders, 0);
    void* dbi = gtl_ResourceFile_GetChunkElement(file, file->resolverContext, kChunkIndexBytes, 0);
    for (unsigned int i = 0; i < vbChunk->elementCount; i++) {
        unsigned short vbIndex = mesh->vertexBufferIndex;
        GtlVBData* vd = (GtlVBData*)((char*)vbd + vbIndex * 0xC);
        void* vbuf;
        void* vbPtr;
        // vbh: +0 vertex format flags, +4 payload bytes, +8 vbd byte offset.
        // Allocate/lock/copy the SAME byte count; format is never a divisor.
        VertexBuffer_Create(vd->payloadBytes, 0, vd->vertexFormatFlags, params->vertexBufferCreationFlags, &vbuf, 0);
        Resource_GetOffsetPtr(vbuf, 0, vd->payloadBytes, &vbPtr, 0);
        Memory_Copy(vbPtr, gtl_GenericModel_GetDataAtOffset(dbv, vd->vertexDataByteOffset), vd->payloadBytes);
        GraphicsBuffer_Flush(vbuf);
        *gtl_VertexBufferVector_GetDestinationSlot(&self->vertexBuffers, vbIndex) = vbuf;
        if (ibChunk != 0) {
            unsigned short ibIndex = mesh->indexBufferSlot;
            GtlIBData* id = (GtlIBData*)((char*)ibd + ibIndex * 8);
            // Indexed attribute count excludes blend-weight entries and the
            // usage-2/type-5 blend-index form (gtl_generic_renderer_vertex_decl.cpp); it is NOT vertices.
            // Used both as the IB stride argument and repetitions per source
            // halfword. Format 3 is the stored index format, not byte stride.
            // id->indexDataSourceBytes is passed as source BYTES to RepeatHalfwords: source
            // requires that many readable bytes; output writes
            // 2*(id->indexDataSourceBytes/2)*max(attributeCount,1) bytes. Odd tail is skipped.
            // Products/allocation alignment must not overflow native u32. The
            // creator allocates AlignUp(id->indexDataSourceBytes*max(attributeCount,1)+4,32)
            // bytes INCLUDING its four-byte prefix (gtl_graphics_texture_buffer_resources.cpp IndexBuffer_Ctor).
            // Attribute count is forwarded verbatim: its role as a repetition
            // count here does not make it a measured vertex/index byte size.
            // The created buffer must expose that complete destination extent;
            // this loader checks neither allocation nor lock success.
            unsigned int indexedAttributeCount =
                gtl_GenericRenderer_GetIndexedAttributeCount(*gtl_VertexDeclarationVector_GetDestinationSlot(
                    &self->vertexDeclarations, mesh->vertexDeclarationIndex));
            void* ibuf;
            void* ibPtr;
            IndexBuffer_Create(id->indexDataSourceBytes, 0, 3, params->indexBufferCreationFlags, &ibuf, 0,
                               indexedAttributeCount);
            IndexBuffer_GetOffsetPtr(ibuf, 0, id->indexDataSourceBytes, &ibPtr, 0);
            RepeatHalfwords(ibPtr, gtl_GenericModel_GetIndexDataAtOffset(dbi, id->indexDataByteOffset),
                            id->indexDataSourceBytes, 3, indexedAttributeCount);
            IndexBuffer_Flush(ibuf);
            *gtl_IndexBufferVector_GetDestinationSlot(&self->indexBuffers, ibIndex) = ibuf;
        }
        mesh++;
    }
    if (desc->allocateSecondaryVertexBufferSlots != 0) {
        gtl_VertexBufferVector_Resize(&self->secondaryVertexBuffers,
                                      gtl_GenericResourceVector_GetSize(&self->vertexBuffers));
    }
    return 1;
}
#pragma use_lmw_stmw reset

// Borrow base+offset bytes in index data; no validation, allocation or retention.
void* gtl_GenericModel_GetIndexDataAtOffset(void* base, unsigned int offset) {
    return (char*)base + offset;
}

// Borrow base+offset bytes in string/texture/vertex data; same lifetime as base.
void* gtl_GenericModel_GetDataAtOffset(void* base, unsigned int offset) {
    return (char*)base + offset;
}

} // extern "C"
