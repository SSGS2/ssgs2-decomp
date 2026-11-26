// Pointer rings and paged queues: iterator advance and distance, the ring's cursor copy,
// and page-entry allocation for the paged queue.

#include "classes/PointerRing803F.h"
#include "classes/GtlPagedQueue803F.h"
// Borrow the ring's fixed ABI header; no ownership is transferred.
#define AsPointerRing(p) ((PointerRing803F*)(p))

// Normal destruction remains explicit. This owner handles exceptional exits.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Arm cleanup for an initialized temporary whose allocation has not been committed.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Transfer responsibility to the explicit normal-path destructor call.
    void release() {
        active = false;
    }
    // Destroy the temporary on unwind, without deleting its stack header.
    ~EH5Cleanup() {
        if (active)
            destroy(object, -1);
    }
};

extern "C" {

// Read the pointer stored at a ring iterator's current slot.
void* PointerRingIterator_ReadPointer(void** p) {
    return **(void***)p;
}

struct Iterator803F7AC4 {
    unsigned int current;
    unsigned int begin;
    unsigned int logicalOrigin; // Logical origin retained for distance calculations; advance uses only begin/end.
    unsigned int end;
};

// Advance one pointer slot and wrap at the physical buffer limit.
void PointerRingIterator_Advance(Iterator803F7AC4* p) {
    unsigned int current = p->current + 4;
    p->current = current;
    if (current == p->end) {
        p->current = p->begin;
    }
}

void* gtl_Alloc(unsigned int);

struct Ring803F7A2C {
    unsigned int current;
    unsigned int allocationBegin;
    unsigned int logicalOrigin;
    unsigned int allocationEnd;
};

// Count pointer slots between two cursors, accounting for their wrap boundary.
int PointerRing_IteratorDistance(Ring803F7A2C* position, Ring803F7A2C* range) {
    int n = ((int)range->current - (int)position->current) / 4;
    if (range->current < range->logicalOrigin) {
        if (position->current < range->logicalOrigin)
            return n;
        return n + ((int)range->allocationEnd - (int)range->allocationBegin) / 4;
    }
    if (position->current >= range->logicalOrigin)
        return n;
    return n - ((int)range->allocationEnd - (int)range->allocationBegin) / 4;
}

// Copy the four iterator words and its trailing status byte, returning the destination.
// Returns dst (r3 untouched, 0x803F7AB4); PointerRing_InsertRingIteratorRange passes it on (0x803F7510, 0x803F76E0).
void* PointerRing_CopyIteratorWithStatus(void* dst, const void* src) {
    unsigned int* d = (unsigned int*)dst;
    const unsigned int* s = (const unsigned int*)src;
    d[0] = s[0];
    d[1] = s[1];
    d[2] = s[2];
    d[3] = s[3];
    ((unsigned char*)dst)[16] = ((const unsigned char*)src)[16];
    return dst;
}

} // extern "C"

extern "C" {
extern signed char s_pagedQueueMaxCountInitialized;
extern int s_gtlPagedQueueMaxSize;
}

// Cache limit bits 0xFFFFFF00 (4294967040 unsigned), shared by every paged queue.
unsigned int GtlPagedQueue803F::GetMaxSize() {
    if (!s_pagedQueueMaxCountInitialized) {
        s_gtlPagedQueueMaxSize = -256;
        s_pagedQueueMaxCountInitialized = 1;
    }
    return s_gtlPagedQueueMaxSize;
}

// Allocate a page-entry array with eight bytes per entry.
void* GtlPagedQueue803F::AllocatePageEntries(void*, unsigned int pageEntryCount) {
    return gtl_Alloc(pageEntryCount * 8);
}

extern "C" {

struct RingIterator803F7834 {
    unsigned int current;
    unsigned int begin;
    unsigned int logicalOrigin;
    unsigned int end;
    unsigned char flag;
};

// Copy a wrapped source range into a wrapped destination and return its final cursor.
// position/end delimit the half-open SOURCE range; range is the DESTINATION cursor.
// All cursors borrow pointer slots, never the pointed-to objects. Source endpoints must
// share one ring and logicalOrigin; destination must have room for the entire range.
// Copy is forward, one word at a time, not memmove: overlapping writes must not destroy
// unread source slots. Each logical advance crosses the physical wrap at most once.
// range advances to the end of the copy. position advances only BETWEEN chunks; after
// the last chunk it still marks that chunk's start (or is unchanged for an empty range).
// result receives range's four cursor words; its status byte is deliberately untouched.
void PointerRing_CopyWrappedIteratorRange(RingIterator803F7834* result, RingIterator803F7834* position,
                                          RingIterator803F7834* end, RingIterator803F7834* range) {
    int index;
    if (position->current != end->current) {
        index = ((int)end->current - (int)position->current) / 4;
        if (end->current >= end->logicalOrigin) {
            if (position->current < end->logicalOrigin) {
                index -= ((int)end->end - (int)end->begin) / 4;
            }
        } else if (position->current >= end->logicalOrigin) {
            index += ((int)end->end - (int)end->begin) / 4;
        }
        for (;;) {
            int rangeCount = ((int)range->end - (int)range->current) / 4;
            int count = rangeCount < index ? rangeCount : index;
            int size = ((int)position->end - (int)position->begin) / 4;
            int slot = ((int)position->current - (int)position->begin) / 4 + count;
            if (slot < 0)
                slot += size;
            else if (slot >= size)
                slot -= size;
            unsigned int newCurrent = position->begin + slot * 4;
            unsigned int* src = (unsigned int*)position->current;
            unsigned int* dst = (unsigned int*)range->current;
            unsigned int* stop = (unsigned int*)position->end;
            unsigned int* wrap = (unsigned int*)position->begin;
            while ((unsigned int)src != newCurrent) {
                unsigned int v = *src++;
                if (src == stop)
                    src = wrap;
                *dst++ = v;
            }
            range->current += count * 4;
            index -= count;
            if (range->current == range->end)
                range->current = range->begin;
            if (index == 0)
                break;
            position->current = newCurrent;
        }
    }
    result->current = range->current;
    result->begin = range->begin;
    result->logicalOrigin = range->logicalOrigin;
    result->end = range->end;
}

// The helpers share this ABI scratch type, but the meanings are distinct:
// cursor: a=current, b=allocationBegin, c=logicalOrigin, d=allocationEnd, byte 16=status;
// storage: a=dataAddress, b=firstIndex, c=size, d=capacity, e=allocator.
// Storage owns only the slot allocation; pointer payloads remain borrowed. Physical
// capacity includes one sentinel slot; PointerRing_GetUsableCapacity excludes it.
struct Temp803F74E4 {
    unsigned int a, b, c, d, e;
};
typedef Temp803F74E4 RingCursorScratch;
typedef Temp803F74E4 RingStorageScratch;
// Borrow the storage header's allocation word without treating it as an iterator.
static inline unsigned int& RingStorageDataAddress(RingStorageScratch& storage) {
    return storage.a;
}
// Borrow the storage header's logical first-slot index.
static inline unsigned int& RingStorageFirstIndex(RingStorageScratch& storage) {
    return storage.b;
}
// Borrow the storage header's occupied-slot count.
static inline unsigned int& RingStorageSize(RingStorageScratch& storage) {
    return storage.c;
}
// The shared +0x0C accessor exposes capacity in this pointer-ring specialization.
extern void* Container_GetWord0CAddress(void*);
extern unsigned int PointerRing_GetUsableCapacity(void*);
extern void WordRing_GetBeginIterator(Temp803F74E4*, void*);
extern void WordRing_GetEndIterator(Temp803F74E4*, void*);
extern unsigned int RingBufferIterator_CopyBounds(Temp803F74E4*, void*);
extern unsigned int RingBufferIterator_GetCurrentSlot(Temp803F74E4*);
extern void RingBufferIterator_SubtractAssign(Temp803F74E4*, unsigned int);
extern void RingCursor_AddOffset(Temp803F74E4*, Temp803F74E4*, unsigned int);
extern void RingBufferIterator_CopyForward(Temp803F74E4*, Temp803F74E4*, Temp803F74E4*, unsigned int);
extern void RingCursor_CopyFourWords(Temp803F74E4*, Temp803F74E4*);
extern void RingCursor_SubtractOffset(Temp803F74E4*, Temp803F74E4*, unsigned int);
extern void RingBufferIterator_CopyBackward(Temp803F74E4*, Temp803F74E4*, Temp803F74E4*, unsigned int);
extern void* PointerRing_GetCapacityAddressForConstruction(void*);
extern unsigned int PointerRing_ComputeGrowthCapacity(void*, unsigned int);
extern void PointerRing_AllocateCapacity(Temp803F74E4*, unsigned int, unsigned int);
extern void* PointerRing_GetAllocatorValue(Temp803F74E4*);
extern void Memory_CopyWordIfDestinationNonNull(void*, unsigned int*, unsigned int*);
extern void PointerRing_AppendCursorRange(Temp803F74E4*, RingIterator803F7834*, Temp803F74E4*);
extern unsigned int RingBuffer_ClearSize(void*);
extern unsigned int* PointerRing_GetCapacityAddress(Temp803F74E4*);
extern void Swap_Int32ForRingStorage(void*, void*);
extern void Swap_Int32(unsigned int*, unsigned int*);
extern void PointerRing_dtor(Temp803F74E4*, int);

// Insert the entries [first, last) before position.
// position belongs to self; first/last share a source ring and logical origin. Growth
// consumes first; the in-place path consumes copies. All self iterators can be invalidated.
// The source must survive opening the gap (self-aliasing is not staged or validated).
// No pointee is allocated/deleted. In-place header changes precede the copy; growth
// owns its replacement until the final swaps and cleanup then releases the old slots.
void PointerRing_InsertRingIteratorRange(void* self, RingIterator803F7834* position, RingIterator803F7834* first,
                                         RingIterator803F7834* last) {
    RingCursorScratch lastCopy, firstCopy, current, scratch;
    RingCursorScratch boundaryCursor, destinationCursor, sourceEndCursor, sourceBeginCursor, copiedEndCursor;
    PointerRing_CopyIteratorWithStatus(&lastCopy, last);
    PointerRing_CopyIteratorWithStatus(&firstCopy, first);
    unsigned int n = PointerRing_IteratorDistance((Ring803F7A2C*)&firstCopy, (Ring803F7A2C*)&lastCopy);
    if (!n)
        return;
    unsigned int* start = (unsigned int*)AsPointerRing(self)->dataAddress + AsPointerRing(self)->firstIndex;
    unsigned int index = ((int)position->current - (int)start) / 4;
    if (position->current < (unsigned int)start)
        index += *(unsigned int*)Container_GetWord0CAddress(self);
    unsigned int usableCapacity = PointerRing_GetUsableCapacity(self);
    if (n <= usableCapacity && AsPointerRing(self)->size <= usableCapacity - n) {
        // Room in place: open a gap of n at the nearer end, then copy the entries in.
        WordRing_GetBeginIterator(&current, self);
        unsigned int tail = AsPointerRing(self)->size - index;
        if (index < tail) {
            // Move prefix [oldBegin, position) forward into newBegin=oldBegin-n.
            // boundaryCursor retains oldBegin while current tracks the new gap boundary.
            RingBufferIterator_CopyBounds(&boundaryCursor, &current);
            RingBufferIterator_SubtractAssign(&current, n);
            unsigned int newStart = RingBufferIterator_GetCurrentSlot(&current);
            AsPointerRing(self)->size += n;
            AsPointerRing(self)->firstIndex = (int)(newStart - AsPointerRing(self)->dataAddress) / 4;
            if (index) {
                RingBufferIterator_CopyBounds(&destinationCursor, &current);
                RingCursor_AddOffset(&sourceEndCursor, &boundaryCursor, index);
                RingBufferIterator_CopyBounds(&sourceBeginCursor, &boundaryCursor);
                RingBufferIterator_CopyForward(&copiedEndCursor, &sourceBeginCursor, &sourceEndCursor,
                                               (unsigned int)&destinationCursor);
                RingCursor_CopyFourWords(&current, &copiedEndCursor);
            }
        } else {
            // Move suffix [position, oldEnd) backward into a range ending at oldEnd+n.
            // Backward copying preserves unread suffix entries when the ranges overlap.
            AsPointerRing(self)->size += n;
            WordRing_GetEndIterator(&boundaryCursor, self);
            RingCursor_CopyFourWords(&current, &boundaryCursor);
            if (tail) {
                RingBufferIterator_CopyBounds(&destinationCursor, &current);
                RingCursor_SubtractOffset(&sourceEndCursor, &current, n);
                RingCursor_SubtractOffset(&sourceBeginCursor, &current, n + tail);
                RingBufferIterator_CopyBackward(&copiedEndCursor, &sourceBeginCursor, &sourceEndCursor,
                                                (unsigned int)&destinationCursor);
                RingCursor_CopyFourWords(&current, &copiedEndCursor);
            }
            RingBufferIterator_SubtractAssign(&current, n);
        }
        // Both branches leave current at the opened gap; fill it from source copies.
        RingBufferIterator_CopyBounds(&boundaryCursor, &current);
        PointerRing_CopyIteratorWithStatus(&lastCopy, last);
        PointerRing_CopyIteratorWithStatus(&firstCopy, first);
        PointerRing_CopyWrappedIteratorRange((RingIterator803F7834*)&scratch, (RingIterator803F7834*)&firstCopy,
                                             (RingIterator803F7834*)&lastCopy, (RingIterator803F7834*)&boundaryCursor);
    } else {
        // No room: build a larger queue with the new entries at index, add the old ones
        // after and before them, then swap it in.
        RingStorageScratch grown;
        ((PointerRing803F*)(&grown))->Init((int)PointerRing_GetCapacityAddressForConstruction(self));
        EH5Cleanup eh5Cleanup32(&grown, (void (*)(void*, int))PointerRing_dtor);
        unsigned int capacity = PointerRing_ComputeGrowthCapacity(self, n);
        PointerRing_AllocateCapacity(&grown, AsPointerRing(self)->size + n, capacity);
        // Stage the inserted range at its final logical index in an otherwise empty ring.
        unsigned int* dest = (unsigned int*)RingStorageDataAddress(grown) + index;
        RingStorageFirstIndex(grown) = index;
        while (n) {
            unsigned int value = (unsigned int)PointerRingIterator_ReadPointer((void**)first);
            Memory_CopyWordIfDestinationNonNull(PointerRing_GetAllocatorValue(&grown), dest, &value);
            --n;
            ++dest;
            ++RingStorageSize(grown);
            PointerRingIterator_Advance((Iterator803F7AC4*)first);
        }
        // Complete suffix then prefix; these scratch cursors now delimit OLD self.
        WordRing_GetEndIterator(&boundaryCursor, self);
        PointerRing_AppendCursorRange(&grown, position, &boundaryCursor);
        WordRing_GetBeginIterator(&destinationCursor, self);
        ((PointerRing803F*)(&grown))->PrependRange(&destinationCursor, position);
        // Commit all four storage words. grown then owns the old allocation for cleanup.
        RingBuffer_ClearSize(self);
        Swap_Int32ForRingStorage(self, &grown);
        Swap_Int32((unsigned int*)&AsPointerRing(self)->firstIndex, &RingStorageFirstIndex(grown));
        Swap_Int32(&AsPointerRing(self)->size, &RingStorageSize(grown));
        unsigned int* grownCap = PointerRing_GetCapacityAddress(&grown);
        Swap_Int32((unsigned int*)Container_GetWord0CAddress(self), grownCap);
        {
            eh5Cleanup32.release();
            PointerRing_dtor(&grown, -1);
        }
    }
}

} // extern "C"
