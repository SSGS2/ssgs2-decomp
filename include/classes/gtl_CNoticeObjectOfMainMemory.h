// gtl::CNoticeObjectOfMainMemory: the memory notice for the MEM1 expandable heap. It is an 8-byte object
// (reference count and vtable) created by CMemoryNoticeFactory_Create (gtl_heap_allocators.cpp). Its
// methods are in gtl_heap_allocators.cpp. The vtable comes from the original data; no virtual functions here.
#ifndef CLASSES_GTL_CNOTICEOBJECTOFMAINMEMORY_H
#define CLASSES_GTL_CNOTICEOBJECTOFMAINMEMORY_H

namespace gtl {

class CNoticeObjectOfMainMemory {
public:
    // Allocate a block of byteCount bytes from MEM1 aligned to 1 << alignmentShift, store it at blockOut.
    // Returns 1 when the allocation succeeded, 0 when blockOut is null or the allocation failed.
    int AllocMem1Block(void* blockOut, int byteCount, int alignmentShift);
    // Free a non-null block back to the MEM1 expandable heap. Returns 1, or 0 for a null block.
    int FreeBlock(void* arg);
    // Free this object through the default allocator when deletingFlag is positive. Returns this.
    void* Destroy(int deletingFlag);

    int references; // 0x0
    void* vtbl;     // 0x4
};

#define GTL_CNOTICEOBJECTOFMAINMEMORY_OFF(f, o) \
    typedef char gtl_CNoticeObjectOfMainMemory_off_##f[((int)&((gtl::CNoticeObjectOfMainMemory*)0)->f == (o)) ? 1 : -1]
GTL_CNOTICEOBJECTOFMAINMEMORY_OFF(references, 0x0);
GTL_CNOTICEOBJECTOFMAINMEMORY_OFF(vtbl, 0x4);
#undef GTL_CNOTICEOBJECTOFMAINMEMORY_OFF

} // namespace gtl

#endif // CLASSES_GTL_CNOTICEOBJECTOFMAINMEMORY_H
