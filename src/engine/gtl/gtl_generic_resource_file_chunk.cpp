// GTL model/animation resource: borrow an element from a tagged file chunk.
// API index: gtl::IGenericResourceFile::GetChunkElement forwards a tag/index to
// gtl_ResourceFile_GetChunkElement (gtl_image_decode_and_tiling.cpp). This is format infrastructure;
// it exposes no gameplay tuning constants. Lifetime ends when the backing file
// is released, including through the resource destructor in gtl_generic_animation_resource_part1.cpp.
// 32-bit native-word schema (no byte swapping here): wrapper +8 holds the
// retained backing-resource address; backing +8 is the file-header pointer,
// backing +C is a forwarded data/context word (unused by the current resolver).
// File +14 is directory count; +20 starts 16-byte directory records:
// {u32 tag, u32 elementCount, u32 elementStrideBytes, u32 fileByteOffset}.
// Object_GetField8/C are offset accessors reused across unrelated types: the
// primitive-buffer names at their definitions in gtl_render_state_and_primitive.cpp do not apply here.

#include "classes/GtlIGenericResourceFile.h"

extern "C" {

void* Object_GetField8(unsigned int);
void* Object_GetFieldC(unsigned int);
void* gtl_ResourceFile_GetChunkElement(void*, void*, int, int);

// self and its backing-resource pointer must be valid; no reference is acquired.
// chunkTag is a 32-bit serialized tag bit pattern (e.g. 0x00786574 for tex),
// elementIndex is interpreted unsigned by the resolver, so negatives miss.
// Returns borrowed file + record.offset + index*stride, or null for null file,
// missing tag, zero count*stride, or index >= count. Null is data absence, not
// an error code; no error/output state is written. File bounds/overflow are not
// validated. Preserve the context-before-file accessor evaluation order.
} // extern "C"

void* gtl::IGenericResourceFile::GetChunkElement(int chunkTag, int elementIndex) {
    void* forwardedChunkContext = Object_GetFieldC((unsigned int)resourceBuffer);
    return gtl_ResourceFile_GetChunkElement(Object_GetField8((unsigned int)resourceBuffer), forwardedChunkContext,
                                            chunkTag, elementIndex);
}
