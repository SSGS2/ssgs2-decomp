// GtlAllocationRange: layout view owned by gtl_heap_allocators.cpp.
// Evidence: gtl_CFixedVectorAllocator_GetNextRange output. No virtual declarations or runtime objects.
#ifndef CLASSES_GTLALLOCATIONRANGE_H
#define CLASSES_GTLALLOCATIONRANGE_H

class GtlAllocationRange {
public:
    int address; // 0x0; gtl_CFixedVectorAllocator_GetNextRange output
    int offsetFromBase; // 0x4; gtl_CFixedVectorAllocator_GetNextRange output
    int byteSize; // 0x8; gtl_CFixedVectorAllocator_GetNextRange output
};

#define OFF(f, o) typedef char GtlAllocationRange_off_##f[((int)&((GtlAllocationRange*)0)->f == (o)) ? 1 : -1]
OFF(address, 0x0);
OFF(offsetFromBase, 0x4);
OFF(byteSize, 0x8);
#undef OFF

#endif
