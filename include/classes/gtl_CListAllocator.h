#ifndef CLASSES_GTL_CLISTALLOCATOR_H
#define CLASSES_GTL_CLISTALLOCATOR_H

struct AllocationInfo;
struct VectorBlockLinks;

namespace gtl {

// List allocator: first/best-fit ranges with an in-arena header per block.
class CListAllocator {
public:
    unsigned int SumFreeBlockBytes();
    unsigned int GetUsedBytes();
    unsigned int GetNextBlockInfo(AllocationInfo* out, int which);
    unsigned int FreeBlock(VectorBlockLinks* item);
    unsigned int CountListHeaders(int which);
    unsigned int Initialize(void* source);
    unsigned int InsertFreeBlock(unsigned int head, unsigned int item);
    unsigned int Reallocate(void* oldPayload, unsigned int requestedBytes);
    void* AllocateBlocks(unsigned int sizeArg);
    void* Ctor(void* source);
    void* GetArenaSize();
    void* Destroy(int deletingFlag);
    unsigned int GetLargestFreeRange() const;

    unsigned int referenceCount; // 0x0
    void* vtable;                // 0x4
    unsigned int bestFitEnabled; // 0x8
    unsigned int blockShift;     // 0xc
    unsigned int blockSize;      // 0x10
    char pad_014[0x4];           // 0x14
    unsigned int arenaAddress;   // 0x18
    void* allocatedHead;         // 0x1c
    void* freeHead;              // 0x20
    unsigned int arenaSize;      // 0x24
    unsigned int overheadBlocks; // 0x28
};

#define GTL_CLISTALLOCATOR_OFF(f, o) \
    typedef char CListAllocator_off_##f[((int)&((gtl::CListAllocator*)0)->f == (o)) ? 1 : -1]
GTL_CLISTALLOCATOR_OFF(vtable, 0x4);
GTL_CLISTALLOCATOR_OFF(bestFitEnabled, 0x8);
GTL_CLISTALLOCATOR_OFF(blockShift, 0xc);
GTL_CLISTALLOCATOR_OFF(blockSize, 0x10);
GTL_CLISTALLOCATOR_OFF(arenaAddress, 0x18);
GTL_CLISTALLOCATOR_OFF(allocatedHead, 0x1c);
GTL_CLISTALLOCATOR_OFF(freeHead, 0x20);
GTL_CLISTALLOCATOR_OFF(arenaSize, 0x24);
GTL_CLISTALLOCATOR_OFF(overheadBlocks, 0x28);
GTL_CLISTALLOCATOR_OFF(referenceCount, 0x0);
#undef GTL_CLISTALLOCATOR_OFF

} // namespace gtl

#endif // CLASSES_GTL_CLISTALLOCATOR_H
