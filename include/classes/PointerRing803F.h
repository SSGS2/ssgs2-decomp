// PointerRing803F: layout-only view owned by gtl_pointer_ring_and_sample_deque.cpp (PointerRing_AllocateCapacity, PointerRing803F_PrependRange, PointerRing_AppendCursorRange).
// No virtual methods, constructors, or generated RTTI.
#ifndef CLASSES_POINTERRING803F_H
#define CLASSES_POINTERRING803F_H

class PointerRing803F {
public:
    // Initialize an empty ring with its allocator token.
    void Init(int allocatorToken);
    // Shrink the logical pointer range by one without freeing or clearing its last slot.
    void DecrementSize();
    // Borrow the address of the ring's first pointer slot; requires a nonempty ring.
    void* GetFrontSlot();
    // Prepend a cursor range, copying its wrapped segments in destination order.
    void PrependRange(void* rangeBegin, void* rangeEnd);

    unsigned int dataAddress; // 0x0: PointerRing_AllocateCapacity allocates four-byte elements
    int firstIndex;           // 0x4: PointerRing803F_PrependRange subtracts prepended count
    unsigned int size;        // 0x8: PointerRing803F_PrependRange/PointerRing_AppendCursorRange add inserted count
    unsigned int
        capacity; // 0xc: PointerRing_AllocateCapacity stores limit + 1; PointerRing_AppendCursorRange wraps modulo this
    int allocator; // 0x10: PointerRing803F_Init stores its second argument
};

#define OFF(f, o) typedef char PointerRing803F_off_##f[((int)&((PointerRing803F*)0)->f == (o)) ? 1 : -1]
OFF(dataAddress, 0x0);
OFF(firstIndex, 0x4);
OFF(size, 0x8);
OFF(capacity, 0xc);
OFF(allocator, 0x10);
#undef OFF

#endif
