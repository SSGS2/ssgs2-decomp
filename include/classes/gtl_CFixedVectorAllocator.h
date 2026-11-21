// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_GTL_CFIXEDVECTORALLOCATOR_H
#define CLASSES_GTL_CFIXEDVECTORALLOCATOR_H

#include "game/types.h"

class gtl_CFixedVectorAllocator {
public:
    unsigned int referenceCount;                 // 0x0: gtl_CFixedVectorAllocator_Allocate/2/3: slots at +0x18 and slot count at +0x24; list heads at +0x1c/+0x20
    void* vtable;                       // 0x4: gtl_CFixedVectorAllocator_Allocate/2/3: slots at +0x18 and slot count at +0x24; list heads at +0x1c/+0x20
    unsigned int bestFit;               // 0x8: gtl_CFixedVectorAllocator_Allocate/2/3: slots at +0x18 and slot count at +0x24; list heads at +0x1c/+0x20
    unsigned int blockShift;            // 0xc: gtl_CFixedVectorAllocator_Allocate/2/3: slots at +0x18 and slot count at +0x24; list heads at +0x1c/+0x20
    unsigned int blockSize;             // 0x10: gtl_CFixedVectorAllocator_Allocate/2/3: slots at +0x18 and slot count at +0x24; list heads at +0x1c/+0x20
    char pad_014[0x4];                  // 0x14
    void* slots;                        // 0x18: gtl_CFixedVectorAllocator_Allocate/2/3: slots at +0x18 and slot count at +0x24; list heads at +0x1c/+0x20
    void* allocatedHead;                // 0x1c: gtl_CFixedVectorAllocator_Allocate/2/3: slots at +0x18 and slot count at +0x24; list heads at +0x1c/+0x20
    unsigned int freeHead;              // 0x20: gtl_CFixedVectorAllocator_Allocate/2/3: slots at +0x18 and slot count at +0x24; list heads at +0x1c/+0x20
    unsigned int slotCount;             // 0x24: gtl_CFixedVectorAllocator_Allocate/2/3: slots at +0x18 and slot count at +0x24; list heads at +0x1c/+0x20
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define GTL_CFIXEDVECTORALLOCATOR_OFF(f, o) typedef char gtl_CFixedVectorAllocator_off_##f[((int)&((gtl_CFixedVectorAllocator*)0)->f == (o)) ? 1 : -1]
GTL_CFIXEDVECTORALLOCATOR_OFF(vtable, 0x4);
GTL_CFIXEDVECTORALLOCATOR_OFF(bestFit, 0x8);
GTL_CFIXEDVECTORALLOCATOR_OFF(blockShift, 0xc);
GTL_CFIXEDVECTORALLOCATOR_OFF(blockSize, 0x10);
GTL_CFIXEDVECTORALLOCATOR_OFF(slots, 0x18);
GTL_CFIXEDVECTORALLOCATOR_OFF(allocatedHead, 0x1c);
GTL_CFIXEDVECTORALLOCATOR_OFF(freeHead, 0x20);
GTL_CFIXEDVECTORALLOCATOR_OFF(slotCount, 0x24);
GTL_CFIXEDVECTORALLOCATOR_OFF(referenceCount, 0x0);
#undef GTL_CFIXEDVECTORALLOCATOR_OFF

#endif  // CLASSES_GTL_CFIXEDVECTORALLOCATOR_H
