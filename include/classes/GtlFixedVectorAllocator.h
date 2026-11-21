// GtlFixedVectorAllocator: layout view owned by gtl_heap_allocators.cpp.
// Evidence: gtl_CFixedVectorAllocator_GetLargestFreeByteSpan/8/9. No virtual declarations or runtime objects.
#ifndef CLASSES_GTLFIXEDVECTORALLOCATOR_H
#define CLASSES_GTLFIXEDVECTORALLOCATOR_H

class GtlFixedVectorAllocator {
public:
    char pad_0[0xC];
    unsigned int elementShift; // 0xC; gtl_CFixedVectorAllocator_GetLargestFreeByteSpan/8/9
    char pad_10[0x4];
    void* cursor; // 0x14; gtl_CFixedVectorAllocator_GetLargestFreeByteSpan/8/9
    void* base; // 0x18; gtl_CFixedVectorAllocator_GetLargestFreeByteSpan/8/9
    void* firstList; // 0x1C; gtl_CFixedVectorAllocator_GetLargestFreeByteSpan/8/9
    void* secondList; // 0x20; gtl_CFixedVectorAllocator_GetLargestFreeByteSpan/8/9
    char pad_24[0x4];
    int capacity; // 0x28; gtl_CFixedVectorAllocator_GetLargestFreeByteSpan/8/9
};

#define OFF(f, o) typedef char GtlFixedVectorAllocator_off_##f[((int)&((GtlFixedVectorAllocator*)0)->f == (o)) ? 1 : -1]
OFF(elementShift, 0xC);
OFF(cursor, 0x14);
OFF(base, 0x18);
OFF(firstList, 0x1C);
OFF(secondList, 0x20);
OFF(capacity, 0x28);
#undef OFF

#endif
