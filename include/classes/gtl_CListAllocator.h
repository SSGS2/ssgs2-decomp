// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_GTL_CLISTALLOCATOR_H
#define CLASSES_GTL_CLISTALLOCATOR_H

#include "game/types.h"

class gtl_CListAllocator {
public:
    unsigned int referenceCount;                 // 0x0: gtl_CListAllocator_Initialize and gtl_CListAllocator_AllocateBlocks/2/3: arena at +0x18, shift, block size, allocated/free list heads, extra blocks
    void* vtable;                       // 0x4: gtl_CListAllocator_Initialize and gtl_CListAllocator_AllocateBlocks/2/3: arena at +0x18, shift, block size, allocated/free list heads, extra blocks
    unsigned int bestFitEnabled;                 // 0x8: gtl_CListAllocator_Initialize and gtl_CListAllocator_AllocateBlocks/2/3: arena at +0x18, shift, block size, allocated/free list heads, extra blocks
    unsigned int blockShift;            // 0xc: gtl_CListAllocator_Initialize and gtl_CListAllocator_AllocateBlocks/2/3: arena at +0x18, shift, block size, allocated/free list heads, extra blocks
    unsigned int blockSize;             // 0x10: gtl_CListAllocator_Initialize and gtl_CListAllocator_AllocateBlocks/2/3: arena at +0x18, shift, block size, allocated/free list heads, extra blocks
    char pad_014[0x4];                  // 0x14
    unsigned int arenaAddress;          // 0x18: gtl_CListAllocator_Initialize and gtl_CListAllocator_AllocateBlocks/2/3: arena at +0x18, shift, block size, allocated/free list heads, extra blocks
    void* allocatedHead;                // 0x1c: gtl_CListAllocator_Initialize and gtl_CListAllocator_AllocateBlocks/2/3: arena at +0x18, shift, block size, allocated/free list heads, extra blocks
    void* freeHead;                     // 0x20: gtl_CListAllocator_Initialize and gtl_CListAllocator_AllocateBlocks/2/3: arena at +0x18, shift, block size, allocated/free list heads, extra blocks
    unsigned int arenaSize;                // 0x24: gtl_CListAllocator_Initialize and gtl_CListAllocator_AllocateBlocks/2/3: arena at +0x18, shift, block size, allocated/free list heads, extra blocks
    unsigned int overheadBlocks;        // 0x28: gtl_CListAllocator_Initialize and gtl_CListAllocator_AllocateBlocks/2/3: arena at +0x18, shift, block size, allocated/free list heads, extra blocks
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define GTL_CLISTALLOCATOR_OFF(f, o) typedef char gtl_CListAllocator_off_##f[((int)&((gtl_CListAllocator*)0)->f == (o)) ? 1 : -1]
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

#endif  // CLASSES_GTL_CLISTALLOCATOR_H
