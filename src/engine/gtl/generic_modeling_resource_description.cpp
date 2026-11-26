// Modeling resource summary API; no gameplay tuning. GetDescription walks the
// loaded resource-file chunk directory, not the renderer's live buffer vectors.
// For model/primitive/material counts see DescriptionWord; for vertex totals
// follow primitive -> vertex-data group -> declaration AND vertex-buffer header.
// The resource-file loader must have resolved native-endian chunk records first.
enum DescriptionWord {
    kModelCount,
    kPrimitiveCount,
    kReferencedVertexCount,
    kIndexCount,
    kMaterialCount
};
// Tag integers spell the listed ASCII identifiers from low to high byte. These
// are identifiers passed to the directory API, not a request to swap file bytes.
enum ModelingChunkTag {
    kModelChunk = 0x6c646d,              // mdl
    kPrimitiveChunk = 0x6d7270,          // prm
    kVertexDataChunk = 0x6476,           // vd
    kVertexDeclarationChunk = 0x646576,  // ved
    kVertexBufferHeaderChunk = 0x686276, // vbh
    kIndexBufferHeaderChunk = 0x686269,  // ibh
    kMaterialChunk = 0x6c72746d          // mtrl
};
// Partial record schemas; unconsumed bytes are deliberately opaque.
struct ModelingPrimitiveView {
    char reserved00[0x12];
    unsigned short vertexBufferIndex; // index into vbh, NOT a byte offset
    char reserved14[4];
    unsigned short vertexDataIndex; // index into vd
};
struct ModelingVertexDataView {
    unsigned int declarationIndex;
}; // index into ved
struct ModelingVertexBufferHeaderView {
    unsigned int reserved00;
    unsigned int byteSize;
};
struct ModelingIndexBufferHeaderView {
    unsigned int byteSize;
};

extern "C" {

void* Object_GetField8(void* obj);
void* Object_GetFieldC(void* obj);
int gtl_ResourceFile_GetChunkElementCount(void* file, void* chunks, int tag);
void* gtl_ResourceFile_GetChunkElement(void* file, void* chunks, int tag, int index);
unsigned int VertexDeclaration_GetStride(void* decl, int index);
}

#include "classes/GtlIGenericModelingResource.h"

#pragma use_lmw_stmw on
// Write five u32 words in DescriptionWord order and return 1. backingResource is the borrowed
// resource-file wrapper; its Field8 and FieldC supply the file/directory operands to every chunk
// query. All records must exist and declarations must yield nonzero vertex stride. No null or
// zero-stride checks exist: malformed resources may fault or divide by zero after partial writes.
// out must not alias resource records or this object.
// Writes are progressive: model/primitive counts, zero+accumulate vertices,
// zero+accumulate indices, then material count. Vertices are counted PER PRIMITIVE
// reference (shared buffers may be counted twice); no vbh chunks means zero.
// Each ibh byte size is divided by two, assuming 16-bit indices, with truncation.
int gtl::IGenericModelingResource::GetDescription(unsigned int* out) {
    void* chunks;
    unsigned int vertexBufferHeaderCount;
    unsigned int primitiveIndex;
    void* primitive;
    void* vertexData;
    void* vertexDeclaration;
    unsigned int vertexStrideBytes;
    void* bufferHeader;
    chunks = Object_GetFieldC(backingResource);
    out[kModelCount] = gtl_ResourceFile_GetChunkElementCount(Object_GetField8(backingResource), chunks, kModelChunk);

    chunks = Object_GetFieldC(backingResource);
    out[kPrimitiveCount] =
        gtl_ResourceFile_GetChunkElementCount(Object_GetField8(backingResource), chunks, kPrimitiveChunk);

    chunks = Object_GetFieldC(backingResource);
    vertexBufferHeaderCount =
        gtl_ResourceFile_GetChunkElementCount(Object_GetField8(backingResource), chunks, kVertexBufferHeaderChunk);
    out[kReferencedVertexCount] = 0;
    if (vertexBufferHeaderCount != 0) {
        primitiveIndex = 0;
        while (primitiveIndex < out[kPrimitiveCount]) {
            chunks = Object_GetFieldC(backingResource);
            primitive = gtl_ResourceFile_GetChunkElement(Object_GetField8(backingResource), chunks, kPrimitiveChunk,
                                                         primitiveIndex);

            chunks = Object_GetFieldC(backingResource);
            vertexData = gtl_ResourceFile_GetChunkElement(Object_GetField8(backingResource), chunks, kVertexDataChunk,
                                                          ((ModelingPrimitiveView*)primitive)->vertexDataIndex);

            chunks = Object_GetFieldC(backingResource);
            vertexDeclaration =
                gtl_ResourceFile_GetChunkElement(Object_GetField8(backingResource), chunks, kVertexDeclarationChunk,
                                                 ((ModelingVertexDataView*)vertexData)->declarationIndex);
            vertexStrideBytes = VertexDeclaration_GetStride(vertexDeclaration, 0);

            chunks = Object_GetFieldC(backingResource);
            bufferHeader =
                gtl_ResourceFile_GetChunkElement(Object_GetField8(backingResource), chunks, kVertexBufferHeaderChunk,
                                                 ((ModelingPrimitiveView*)primitive)->vertexBufferIndex);
            primitiveIndex++;
            out[kReferencedVertexCount] = out[kReferencedVertexCount] +
                                          ((ModelingVertexBufferHeaderView*)bufferHeader)->byteSize / vertexStrideBytes;
        }
    }

    chunks = Object_GetFieldC(backingResource);
    unsigned int indexBufferHeaderCount =
        gtl_ResourceFile_GetChunkElementCount(Object_GetField8(backingResource), chunks, kIndexBufferHeaderChunk);
    out[kIndexCount] = 0;
    unsigned int indexBufferHeaderIndex = 0;
    while (indexBufferHeaderIndex < indexBufferHeaderCount) {
        chunks = Object_GetFieldC(backingResource);
        void* bufferHeader = gtl_ResourceFile_GetChunkElement(Object_GetField8(backingResource), chunks,
                                                              kIndexBufferHeaderChunk, indexBufferHeaderIndex);
        indexBufferHeaderIndex++;
        out[kIndexCount] = out[kIndexCount] + (((ModelingIndexBufferHeaderView*)bufferHeader)->byteSize >> 1);
    }

    chunks = Object_GetFieldC(backingResource);
    out[kMaterialCount] =
        gtl_ResourceFile_GetChunkElementCount(Object_GetField8(backingResource), chunks, kMaterialChunk);
    return 1;
}
#pragma use_lmw_stmw reset
