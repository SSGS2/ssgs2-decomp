#ifndef CLASSES_GTL_CFIXEDVECTORALLOCATOR_H
#define CLASSES_GTL_CFIXEDVECTORALLOCATOR_H

namespace gtl {

// Fixed-vector allocator: payloads are arena ranges tracked by 32-byte header slots.
class CFixedVectorAllocator {
public:
    void* Ctor(void* source);
    unsigned int Initialize(void* source);
    void* Allocate(unsigned int size);
    unsigned int ReallocPayload(void* oldPayload, unsigned int requestedBytes);
    unsigned int FreeBlock(void* value);
    unsigned int InsertFreeBlock(unsigned int head, unsigned int item);
    unsigned int GetFreeByteCount();
    int GetLargestFreeByteSpan();
    int GetAllocatedByteCount();
    int CountListNodes(int which);
    int GetNextRange(void* rangeOut, int listSelector);
    void* GetCapacityBytes();
    void* Destroy(int deletingFlag);

    unsigned int referenceCount; // 0x0
    void* vtable;                // 0x4
    unsigned int bestFit;        // 0x8
    unsigned int blockShift;     // 0xc, log2 of blockSize
    unsigned int blockSize;      // 0x10
    void* cursor;                // 0x14, range iteration position
    union {
        void* slots; // 0x18, header slot array (Allocate, Initialize)
        void* base;  // 0x18, arena base pointer word (GetNextRange)
    };
    void* allocatedHead;    // 0x1c
    unsigned int freeHead;  // 0x20
    unsigned int slotCount; // 0x24
    int capacity;           // 0x28, arena bytes
};

#define GTL_CFIXEDVECTORALLOCATOR_OFF(f, o) \
    typedef char CFixedVectorAllocator_off_##f[((int)&((gtl::CFixedVectorAllocator*)0)->f == (o)) ? 1 : -1]
typedef char CFixedVectorAllocator_size[(sizeof(gtl::CFixedVectorAllocator) == 0x2C) ? 1 : -1];
GTL_CFIXEDVECTORALLOCATOR_OFF(referenceCount, 0x0);
GTL_CFIXEDVECTORALLOCATOR_OFF(vtable, 0x4);
GTL_CFIXEDVECTORALLOCATOR_OFF(bestFit, 0x8);
GTL_CFIXEDVECTORALLOCATOR_OFF(blockShift, 0xc);
GTL_CFIXEDVECTORALLOCATOR_OFF(blockSize, 0x10);
GTL_CFIXEDVECTORALLOCATOR_OFF(cursor, 0x14);
GTL_CFIXEDVECTORALLOCATOR_OFF(slots, 0x18);
GTL_CFIXEDVECTORALLOCATOR_OFF(allocatedHead, 0x1c);
GTL_CFIXEDVECTORALLOCATOR_OFF(freeHead, 0x20);
GTL_CFIXEDVECTORALLOCATOR_OFF(slotCount, 0x24);
GTL_CFIXEDVECTORALLOCATOR_OFF(capacity, 0x28);
#undef GTL_CFIXEDVECTORALLOCATOR_OFF

} // namespace gtl

#endif // CLASSES_GTL_CFIXEDVECTORALLOCATOR_H
