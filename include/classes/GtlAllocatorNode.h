// GtlAllocatorNode: layout view owned by gtl_heap_allocators.cpp.
// Evidence: gtl::CFixedVectorAllocator::GetLargestFreeByteSpan/9. No virtual declarations or runtime objects.
#ifndef CLASSES_GTLALLOCATORNODE_H
#define CLASSES_GTLALLOCATORNODE_H

class GtlAllocatorNode {
public:
    int address; // 0x0; gtl::CFixedVectorAllocator::GetLargestFreeByteSpan/9
    char pad_4[0x4];
    void* next;                // 0x8; gtl::CFixedVectorAllocator::GetLargestFreeByteSpan/9
    unsigned int elementCount; // 0xC; gtl::CFixedVectorAllocator::GetLargestFreeByteSpan/9
};

#define OFF(f, o) typedef char GtlAllocatorNode_off_##f[((int)&((GtlAllocatorNode*)0)->f == (o)) ? 1 : -1]
OFF(address, 0x0);
OFF(next, 0x8);
OFF(elementCount, 0xC);
#undef OFF

#endif
