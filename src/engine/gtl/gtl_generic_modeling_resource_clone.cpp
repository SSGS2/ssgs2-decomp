// Model resource clone policy: selectively duplicate graphics payloads while
// retaining other handles. Mod flags below together with destination ownership;
// vector implementations are separate units, buffer/texture layouts are in gtl_graphics_texture_buffer_resources.cpp.
#include "classes/GtlIGenericModelingResource.h"

enum ModelingClonePolicy {
    kCopyVertexBuffers = 1,
    kCopyIndexBuffers = 2,
    kCopyTextures = 4,
    kOmitNonNullPalettes = 8,
    kCopySecondaryGeometry = 0x10
};

struct GtlVBDesc {
    unsigned int unk0;
    unsigned int unk4;
    unsigned int size;
    unsigned int vertexFormat;
};

// Native four-word description outputs (gtl_graphics_texture_buffer_resources.cpp): VB +0 is a sentinel,
// +4 is resource kind, +8 is byte count, +C is vertex format flags.
// IB +0 is index format, +4 kind, +8 source size/count word, +C index stride.
// RepeatHalfwords interprets +8 as SOURCE BYTES and +C as repetitions per
// halfword; do not turn either format into a count or a byte-copy stride.
struct GtlIBDesc {
    unsigned int indexFormat;
    unsigned int unk4;
    unsigned int size;
    unsigned int indexStride;
};

struct GtlTexLevelDesc {
    unsigned int format;
    char unk4[0xC];
    unsigned int size;
    char unk14[8];
    unsigned int width;
    unsigned int height;
};

struct GtlLock {
    unsigned int pitch;
    void* ptr;
};

// Legacy scratch is 16 bytes, NOT an array of 16 declaration elements.
// A declaration element is 12 bytes (gtl_texture_vertex_formats.cpp); GetElements writes all real
// records PLUS one terminator and treats count as output, never capacity.
// This storage therefore fits ONE record, i.e. only an empty declaration.
// Deep declaration cloning (bit 0x10) requires that restriction. A populated
// declaration overruns this function's scratch; sharing avoids that copy path.
// A future corrective change must size this for (realCount+1)*12 bytes; this
// code deliberately preserves storage and the unchecked copy.
struct GtlDeclElems {
    char unk0[0x10];
};

extern "C" {

void* gtl_GenericResource_AllocateModel(unsigned int size);
unsigned int gtl_GenericResourceVector_GetSize(GenericResourceVector* v);
void gtl_VertexBufferVector_Resize(GenericResourceVector* v, unsigned int n);
void** gtl_VertexBufferVector_GetSourceSlot(GenericResourceVector* v, unsigned int i);
void** gtl_VertexBufferVector_GetDestinationSlot(GenericResourceVector* v, unsigned int i);
void gtl_IndexBufferVector_Resize(GenericResourceVector* v, unsigned int n);
void** gtl_IndexBufferVector_GetSourceSlot(GenericResourceVector* v, unsigned int i);
void** gtl_IndexBufferVector_GetDestinationSlot(GenericResourceVector* v, unsigned int i);
void gtl_TextureVector_Resize(GenericResourceVector* v, unsigned int n);
void** gtl_TextureVector_GetSourceSlot(GenericResourceVector* v, unsigned int i);
void** gtl_TextureVector_GetDestinationSlot(GenericResourceVector* v, unsigned int i);
void gtl_TexturePaletteVector_Resize(GenericResourceVector* v, unsigned int n);
void** gtl_TexturePaletteVector_GetSourceSlot(GenericResourceVector* v, unsigned int i);
void** gtl_TexturePaletteVector_GetDestinationSlot(GenericResourceVector* v, unsigned int i);
void gtl_VertexDeclarationVector_Resize(GenericResourceVector* v, unsigned int n);
void** gtl_VertexDeclarationVector_GetSourceSlot(GenericResourceVector* v, unsigned int i);
void** gtl_VertexDeclarationVector_GetDestinationSlot(GenericResourceVector* v, unsigned int i);
void gtl_GraphicsResource_AddRefToHandle(void* src, void** dst);
void VertexBuffer_GetDescription(void* vb, GtlVBDesc* desc);
void VertexBuffer_Create(unsigned int size, unsigned int a, unsigned int vertexFormat, unsigned int b, void** out,
                         unsigned int c);
void Resource_GetOffsetPtr(void* res, unsigned int a, unsigned int b, void** out, unsigned int c);
void Memory_Copy(void* dst, void* src, unsigned int size);
void GraphicsBuffer_Flush(void* res);
void IndexBuffer_GetDescription(void* ib, GtlIBDesc* desc);
void IndexBuffer_Create(unsigned int size, unsigned int a, unsigned int indexFormat, unsigned int b, void** out,
                        unsigned int c, unsigned int indexStride);
void IndexBuffer_GetOffsetPtr(void* res, unsigned int a, unsigned int b, void** out, unsigned int c);
void RepeatHalfwords(void* dst, void* src, unsigned int sourceBytes, unsigned int unusedFormat,
                     unsigned int repetitions);
void IndexBuffer_Flush(void* res);
void Texture_GetLevelDescription(void* tex, unsigned int level, GtlTexLevelDesc* desc);
unsigned int GraphicsResource_GetHeaderWord0C(void* tex);
void Texture_Create2D(unsigned int w, unsigned int h, unsigned int levels, unsigned int a, unsigned int format,
                      unsigned int b, void** out, unsigned int c);
unsigned int TextureFormat_GetC4IndexBitCount(unsigned int format);
void Texture_LockLevel(void* tex, unsigned int level, GtlLock* lock, unsigned int a, unsigned int b);
void Texture_UnlockLevel(void* tex, unsigned int level);
void VertexDeclaration_GetElements(void* decl, GtlDeclElems* elems, unsigned int* count);
void VertexDeclaration_CreateCopy(GtlDeclElems* elems, void** out);
}

#pragma push
#pragma use_lmw_stmw on
// out is required and must not alias this object or one of its slots. Source
// handles are borrowed during copying. Publish the newly allocated 0x54-byte
// owner immediately, then fill its vectors in the order below. Shared slots
// acquire references; deep slots own newly created resources released with the
// model. This is not transactional: allocation/create/lock results are unchecked,
// failures may leave a partial *out or fault; return 1 is not a success proof.
// Empty source slots are copied as null. Bits 8/0x10 deliberately omit resources
// where no deep-copy implementation exists: see palette and associated-owner
// branches. Unknown flag bits have no effect. No source payload is modified,
// but deep-copy paths flush BOTH source and destination and lock texture levels.
// Release model ownership through gtl::IGenericModelingResource::Destroy
// (gtl_generic_modeling_resource_dtor.cpp): graphics handles first, vector storage next, associated parent
// last; its positive deleting flag also frees model storage via resource allocator.
int gtl::IGenericModelingResource::Clone(unsigned int flags, IGenericModelingResource** out) {
    IGenericModelingResource* m = (IGenericModelingResource*)gtl_GenericResource_AllocateModel(0x54);
    if (m != 0)
        m->InitEmpty();
    *out = m;

    {
        unsigned int n = gtl_GenericResourceVector_GetSize(&this->vertexBuffers);
        gtl_VertexBufferVector_Resize(&(*out)->vertexBuffers, n);
        for (unsigned int i = 0; i < n; i++) {
            void* src = *gtl_VertexBufferVector_GetSourceSlot(&this->vertexBuffers, i);
            void** dst = gtl_VertexBufferVector_GetDestinationSlot(&(*out)->vertexBuffers, i);
            if (src == 0) {
                *dst = src;
            } else if (flags & kCopyVertexBuffers) {
                GtlVBDesc desc;
                void* srcPtr;
                void* dstPtr;
                VertexBuffer_GetDescription(src, &desc);
                VertexBuffer_Create(desc.size, 0, desc.vertexFormat, 0, dst, 0);
                Resource_GetOffsetPtr(src, 0, 0, &srcPtr, 0);
                Resource_GetOffsetPtr(*dst, 0, 0, &dstPtr, 0);
                Memory_Copy(dstPtr, srcPtr, desc.size);
                GraphicsBuffer_Flush(src);
                GraphicsBuffer_Flush(*dst);
            } else {
                gtl_GraphicsResource_AddRefToHandle(src, dst);
            }
        }
    }
    {
        unsigned int n = gtl_GenericResourceVector_GetSize(&this->indexBuffers);
        gtl_IndexBufferVector_Resize(&(*out)->indexBuffers, n);
        for (unsigned int i = 0; i < n; i++) {
            void* src = *gtl_IndexBufferVector_GetSourceSlot(&this->indexBuffers, i);
            void** dst = gtl_IndexBufferVector_GetDestinationSlot(&(*out)->indexBuffers, i);
            if (src == 0) {
                *dst = src;
            } else if (flags & kCopyIndexBuffers) {
                GtlIBDesc desc;
                void* srcPtr;
                void* dstPtr;
                IndexBuffer_GetDescription(src, &desc);
                IndexBuffer_Create(desc.size, 0, desc.indexFormat, 0, dst, 0, desc.indexStride);
                IndexBuffer_GetOffsetPtr(src, 0, 0, &srcPtr, 0);
                IndexBuffer_GetOffsetPtr(*dst, 0, 0, &dstPtr, 0);
                // Fourth argument is ignored. Destination must hold
                // 2*(size/2)*max(indexStride,1) bytes; odd final byte is ignored.
                RepeatHalfwords(dstPtr, srcPtr, desc.size, desc.indexFormat, desc.indexStride);
                IndexBuffer_Flush(src);
                IndexBuffer_Flush(*dst);
            } else {
                gtl_GraphicsResource_AddRefToHandle(src, dst);
            }
        }
    }
    {
        unsigned int n = gtl_GenericResourceVector_GetSize(&this->textures);
        gtl_TextureVector_Resize(&(*out)->textures, n);
        for (unsigned int i = 0; i < n; i++) {
            void* src = *gtl_TextureVector_GetSourceSlot(&this->textures, i);
            void** dst = gtl_TextureVector_GetDestinationSlot(&(*out)->textures, i);
            if (src == 0) {
                *dst = src;
            } else if (flags & kCopyTextures) {
                GtlTexLevelDesc desc;
                unsigned int levels;
                Texture_GetLevelDescription(src, 0, &desc);
                // Legacy ABI name for a +0xC word getter. On textures that word
                // is levelCount, written by TextureResource_InitHeader in
                // gtl_graphics_texture_buffer_resources.cpp (zero requested levels defaults to eight). It is
                // NOT a resource pointer in this call; no extra retain occurs.
                levels = GraphicsResource_GetHeaderWord0C(src);
                Texture_Create2D(desc.width, desc.height, levels, 0, desc.format, 0, dst, 0);
                TextureFormat_GetC4IndexBitCount(desc.format);
                for (unsigned int j = 0; j < levels; j++) {
                    GtlTexLevelDesc ld;
                    GtlLock srcLock;
                    GtlLock dstLock;
                    Texture_GetLevelDescription(src, j, &ld);
                    Texture_LockLevel(src, j, &srcLock, 0, 0);
                    Texture_LockLevel(*dst, j, &dstLock, 0, 0);
                    Memory_Copy(dstLock.ptr, srcLock.ptr, ld.size);
                    Texture_UnlockLevel(src, j);
                    Texture_UnlockLevel(*dst, j);
                }
            } else {
                gtl_GraphicsResource_AddRefToHandle(src, dst);
            }
        }
    }
    {
        unsigned int n = gtl_GenericResourceVector_GetSize(&this->palettes);
        gtl_TexturePaletteVector_Resize(&(*out)->palettes, n);
        for (unsigned int i = 0; i < n; i++) {
            void* src = *gtl_TexturePaletteVector_GetSourceSlot(&this->palettes, i);
            void** dst = gtl_TexturePaletteVector_GetDestinationSlot(&(*out)->palettes, i);
            if (src == 0) {
                *dst = src;
            } else if (!(flags & kOmitNonNullPalettes)) {
                gtl_GraphicsResource_AddRefToHandle(src, dst);
            }
            // Bit 8 with nonnull src performs no assignment: preserve the
            // destination vector's initialized slot, rather than clone/share.
            // Do not implement a palette copy as part of a readability edit.
        }
    }
    if (!(flags & kCopySecondaryGeometry)) {
        ((IGenericModelingResource*)backingResource)->AddRef(&(*out)->backingResource);
    }
    {
        unsigned int n = gtl_GenericResourceVector_GetSize(&this->vertexBuffers2);
        gtl_VertexBufferVector_Resize(&(*out)->vertexBuffers2, n);
        for (unsigned int i = 0; i < n; i++) {
            void* src = *gtl_VertexBufferVector_GetSourceSlot(&this->vertexBuffers2, i);
            void** dst = gtl_VertexBufferVector_GetDestinationSlot(&(*out)->vertexBuffers2, i);
            if (src == 0) {
                *dst = src;
            } else if (flags & kCopySecondaryGeometry) {
                GtlVBDesc desc;
                void* srcPtr;
                void* dstPtr;
                VertexBuffer_GetDescription(src, &desc);
                VertexBuffer_Create(desc.size, 0, desc.vertexFormat, 0, dst, 0);
                Resource_GetOffsetPtr(src, 0, 0, &srcPtr, 0);
                Resource_GetOffsetPtr(*dst, 0, 0, &dstPtr, 0);
                Memory_Copy(dstPtr, srcPtr, desc.size);
                GraphicsBuffer_Flush(src);
                GraphicsBuffer_Flush(*dst);
            } else {
                gtl_GraphicsResource_AddRefToHandle(src, dst);
            }
        }
    }
    {
        unsigned int n = gtl_GenericResourceVector_GetSize(&this->vertexDecls);
        gtl_VertexDeclarationVector_Resize(&(*out)->vertexDecls, n);
        for (unsigned int i = 0; i < n; i++) {
            void* src = *gtl_VertexDeclarationVector_GetSourceSlot(&this->vertexDecls, i);
            void** dst = gtl_VertexDeclarationVector_GetDestinationSlot(&(*out)->vertexDecls, i);
            if (src == 0) {
                *dst = src;
            } else if (flags & kCopySecondaryGeometry) {
                // Empty declarations only: one 12-byte terminator fits the
                // 16-byte legacy scratch. count below includes that record.
                GtlDeclElems elems;
                unsigned int count;
                VertexDeclaration_GetElements(src, &elems, &count);
                VertexDeclaration_CreateCopy(&elems, dst);
            } else {
                gtl_GraphicsResource_AddRefToHandle(src, dst);
            }
        }
    }
    return 1;
}
#pragma pop

extern "C" {} // extern "C"
