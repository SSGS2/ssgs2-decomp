// GtlListAllocator: layout view owned by gtl_heap_allocators.cpp.
// Evidence: gtl_CListAllocator_vfunc_4. No virtual declarations or runtime objects.
#ifndef CLASSES_GTLLISTALLOCATOR_H
#define CLASSES_GTLLISTALLOCATOR_H

class GtlListAllocator {
public:
    char pad_0[0x24];
    void* arenaSize; // 0x24; gtl_CListAllocator_vfunc_4
};

#define OFF(f, o) typedef char GtlListAllocator_off_##f[((int)&((GtlListAllocator*)0)->f == (o)) ? 1 : -1]
OFF(arenaSize, 0x24);
#undef OFF

#endif
