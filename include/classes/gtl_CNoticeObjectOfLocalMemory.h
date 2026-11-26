// gtl::CNoticeObjectOfLocalMemory: the memory notice for the MEM2 expandable heap. It is an 8-byte object
// (reference count and vtable) created by CMemoryNoticeFactory_Create (gtl_heap_allocators.cpp). Its
// methods are in gtl_heap_allocators.cpp. The vtable comes from the original data; no virtual functions here.
#ifndef CLASSES_GTL_CNOTICEOBJECTOFLOCALMEMORY_H
#define CLASSES_GTL_CNOTICEOBJECTOFLOCALMEMORY_H

namespace gtl {

class CNoticeObjectOfLocalMemory {
public:
    // Allocate a block of byteCount bytes from MEM2 aligned to 1 << alignmentShift, store it at blockOut.
    // Returns 1 when the allocation succeeded, 0 when blockOut is null or the allocation failed.
    int AllocMem2Block(void* blockOut, int byteCount, int alignmentShift);
    // Free a non-null block back to the MEM2 expandable heap. Returns 1, or 0 for a null block.
    int FreeMem2(void* arg);
    // Free this object through the default allocator when deletingFlag is positive. Returns this.
    void* Destroy(int deletingFlag);

    int references; // 0x0
    void* vtbl;     // 0x4
};

#define GTL_CNOTICEOBJECTOFLOCALMEMORY_OFF(f, o) \
    typedef char                                 \
        gtl_CNoticeObjectOfLocalMemory_off_##f[((int)&((gtl::CNoticeObjectOfLocalMemory*)0)->f == (o)) ? 1 : -1]
GTL_CNOTICEOBJECTOFLOCALMEMORY_OFF(references, 0x0);
GTL_CNOTICEOBJECTOFLOCALMEMORY_OFF(vtbl, 0x4);
#undef GTL_CNOTICEOBJECTOFLOCALMEMORY_OFF

} // namespace gtl

#endif // CLASSES_GTL_CNOTICEOBJECTOFLOCALMEMORY_H
