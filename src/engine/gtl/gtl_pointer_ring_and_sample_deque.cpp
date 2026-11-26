// Pointer rings and the sample deque: cursor arithmetic and copies, ring allocation and
// initialization, capacity queries, and the show counter functions.

#include "classes/GtlShowCounter.h"
#include "classes/PointerRing803F.h"
#include "classes/GtlSampleDeque.h"
// Borrow the five-word ring header without constructing it. Keep these two cast
// macros: inline accessors change scheduling in AppendCursorRange under MWCC.
#define AsPointerRing803F(p) ((PointerRing803F*)(p))
#include "classes/RingCursor803F.h"
// Borrow the four-word cursor view without changing its position.
#define AsRingCursor803F(p) ((RingCursor803F*)(p))

// Two layouts coexist here; they are not interchangeable:
//   PointerRing803F: data, firstIndex, size, capacity, allocator token (5 words).
//   GtlSampleDeque: position, count, rows, head, rowCount, capacity (6 words).
// A ring owns its slot allocation, not the objects addressed by the stored words.
// capacity includes one spare slot. Cursors contain current/begin/preserved/limit
// addresses with an exclusive limit; copying only current loses wrap information.
// The sample deque owns a page table: rowCount includes its trailing sentinel,
// leaving (rowCount-1)*256 element slots. Its first word is a logical position,
// not an address. The shared first-word accessor below is also used on that view.
// Count getters require non-null outputs and return 1. State methods return 1
// for a transition and 0 for no change; state 2 defers clearing until the next start.

extern "C" {

// Return the allocator token stored with the ring.
void* PointerRing_GetAllocatorValue(void* p);

// Expose the capacity word for allocator/container helpers.
void* PointerRing_GetCapacityAddress(void* p);

// Expose the same capacity word during construction.
void* PointerRing_GetCapacityAddressForConstruction(void* p);

// Read the first bookkeeping word through the existing pointer-sized return ABI.
// gtl_media_sample_pool.cpp passes a deque and compares the result as an integer position; this
// function neither dereferences the word as a pointer nor requires a cursor.
void* GtlWordStorage_ReadFirstWord(void* p);

void* gtl_Alloc(unsigned int);
void* memmove(void*, const void*, unsigned int);
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void __dla__FPv(void*);
void __throw(const char*, void*, void*);
unsigned int strlen(const char*);
char* strcpy(char*, const char*);

extern signed char s_gtlSampleDequeMaxSizeInitialized;
extern int s_gtlSampleDequeMaxSize;
extern signed char s_gtlWordDequeMaxSizeInitialized;
extern unsigned int s_gtlWordDequeMaxSize;
struct DequeErrorStringPool {
    char dequeLengthError[0x15];
    char lengthErrorType[0x37];
    char circularDequeLengthError[0x14];
};
extern DequeErrorStringPool audioDequeErrorStrings;
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12length_error[];
void __dt__Q23std12length_errorFv(void*);

struct ErrObj {
    void* vtable;
    char* what;
};

// Allocate one extra ring slot; reject limits that would overflow four-byte storage.
// self must not own an allocation that needs releasing: this overwrites dataAddress.
// value is unused. limit is usable slots; stored capacity includes the spare slot.
// Allocation belongs to the ring and is released by PointerRing_dtor.
void PointerRing_AllocateCapacity(void* self, unsigned int value, unsigned int limit);

enum ShowCounterState {
    kCounting = 0,
    kPaused = 1,
    kResetOnStart = 2
};
enum RingStorageConstants {
    kPointerSlotBytes = 4,
    kElementsPerSamplePage = 256,
    kSampleDequeReservedElements = 512,
    kSampleDequePageCountOffset = 0x10
};
enum ShowCounterVirtualSlots {
    kCounterArmResetSlot = 3,
    kCounterUpdateSlot = 5
};

// Prepend a cursor range, copying its wrapped segments in destination order.
// [rangeBegin, rangeEnd) belongs to one source ring. Destination is already
// reserved and linear: enough slots must precede firstIndex without wrapping.
// Copies words only; pointee ownership/refcounts do not change. Cursors are borrowed.

// Original 0x803F2E58: slwi r3,r4,2; b gtl_Alloc (tail call, so the allocation is returned;
// gtl_media_sample_pool.cpp callers use it as the new block).
// Allocate slotCount four-byte words; caller owns the returned block via gtl_Free.
void* SampleDeque_AllocatePointerSlots(void* unused, unsigned int slotCount);

// Load the pointer stored in the current cursor slot.
// Requires a valid dereferenceable slot; the returned pointee is borrowed.
void* RingCursor_Dereference(void* p);

// Return element capacity of the sample deque's pages, excluding the sentinel row.
// This uses the six-word deque view, not PointerRing803F's allocator at the same offset.
unsigned int SampleDeque_GetUsableElementCapacity(void* p);

struct FourWords {
    unsigned int a, b, c, d;
};

// Copy the complete four-word cursor, including its wrap boundaries.
void RingCursor_CopyFourWords(void* dst, const void* src);

// Advance one pointer slot and wrap at the end of the ring.
void RingCursor_Increment(void* p);

extern char __vt__Q23gtl12IShowCounter[];

// Copy one word if an output address was supplied; retain the original volatile store.
void Memory_CopyWordIfDestinationNonNull(void* unused, int* out, void* src);

// Invoke virtual slots 5 then 3 and return 1 regardless of either hook's result.
// Slot 3 arms reset in the base counter. Slot 5's signature is unresolved in
// this one-argument dispatch; preserve it rather than inventing update parameters.
struct CounterDispatchPrefix {
    unsigned int references;
};
struct CounterDispatch : CounterDispatchPrefix {
    virtual void destroy(int);
    virtual void armReset();
    virtual void pause();
    virtual void update();
};

void gtl_Free(void*);
extern unsigned int s_gtlWordContainerMaxSize;
void __dl__FPv(void*);

// Free the ring allocation, retain its volatile size clear, and optionally delete the ring.
// Stored pointees are not released. do_delete > 0 uses operator delete on p;
// zero/negative only destroys storage. Returns p, possibly dangling; accepts null.
void* PointerRing_dtor(void* p, int do_delete);

struct RingCursorSourceView {
    int* first;
    int* start;
    int* preservedBoundary;
    int* storageEnd;
};
struct RingCursorDestinationView {
    int* first;
    int* start;
    int* preservedBoundary;
    int* storageEnd;
};
// Four-word cursor views: current, begin, preserved boundary, exclusive limit.
// preservedBoundary is copied without interpretation here; it does not participate in wrapping.

// Add an element offset, wrap once, and preserve the original cursor boundaries.
// Valid nonempty storage is required; index must need at most one wrap. dst may
// alias src because all four source words are captured before any destination write.
void RingCursor_AddOffset(RingCursorDestinationView* dst, RingCursorSourceView* src, int index);

// Subtract an element offset, wrapping once, and preserve the cursor boundaries.
// Same nonempty/one-wrap preconditions as AddOffset; dst may alias src.
void RingCursor_SubtractOffset(void* dst, void* src, int elementOffset);

// Append a cursor range at the ring's logical end, copying both source segments if wrapped.
// Requires nonzero destination capacity and contiguous free storage at the computed
// insertion index for the ENTIRE range: only the source wrap is split here.
// The growth callers in gtl_word_deque_ring_buffer.cpp/gtl_pointer_ring_and_paged_queue.cpp provide that linear destination.
// Copies stored words without changing pointee ownership or either source cursor.
void PointerRing_AppendCursorRange(void* dst, void* rangeBegin, void* rangeEnd);

struct RingVec {
    unsigned int* data;
    unsigned int size;
    unsigned int cap;
};
void gtl_Free(void*);

struct DequeStorage {
    unsigned int metadataWords[2];
    unsigned int* data;
    unsigned int firstIndex, size, capacity;
};

// Initialize empty deque bookkeeping; retain the original volatile range-construction work.
// self is at least 0x18 writable bytes and must not own storage needing cleanup.
// Returns self. The volatile scratch is intentionally retained: deleting it or
// folding its zero-derived range changes this reconstruction's emitted code.
void* SampleDeque_InitializeEmptyStorage(void* self);

} // extern "C"

// Game entry points in original address order.
extern "C" {

void RingCursor_AddOffset(RingCursorDestinationView* dst, RingCursorSourceView* src, int index) {
    int* first = src->first;
    int* start = src->start;
    int* preservedBoundary = src->preservedBoundary;
    int* storageLimit = src->storageEnd;
    int slotCount = (int)(((char*)storageLimit - (char*)start) / kPointerSlotBytes);
    int currentIndex = (int)(((char*)first - (char*)start) / kPointerSlotBytes);
    int newIndex = index + currentIndex;
    if (newIndex < 0)
        newIndex += slotCount;
    else if (newIndex >= slotCount)
        newIndex -= slotCount;
    dst->first = start + newIndex;
    dst->start = start;
    dst->preservedBoundary = preservedBoundary;
    dst->storageEnd = storageLimit;
}

void RingCursor_CopyFourWords(void* dst, const void* src) {
    *(FourWords*)dst = *(const FourWords*)src;
}

void RingCursor_SubtractOffset(void* dst, void* src, int elementOffset) {
    unsigned int currentAddress = AsRingCursor803F(src)->currentAddress;
    unsigned int beginAddress = AsRingCursor803F(src)->beginAddress;
    unsigned int splitBoundaryAddress = AsRingCursor803F(src)->splitBoundaryAddress;
    unsigned int limitAddress = AsRingCursor803F(src)->limitAddress;
    int slotCount = (int)(limitAddress - beginAddress) / kPointerSlotBytes;
    int currentIndex = (int)(currentAddress - beginAddress) / kPointerSlotBytes;
    int newIndex = currentIndex - elementOffset;
    if (newIndex < 0)
        newIndex += slotCount;
    else if (newIndex >= slotCount)
        newIndex -= slotCount;
    AsRingCursor803F(dst)->currentAddress = beginAddress + (newIndex << 2);
    AsRingCursor803F(dst)->beginAddress = beginAddress;
    AsRingCursor803F(dst)->splitBoundaryAddress = splitBoundaryAddress;
    AsRingCursor803F(dst)->limitAddress = limitAddress;
}

void* RingCursor_Dereference(void* p) {
    return *(void**)AsRingCursor803F(p)->currentAddress;
}

void RingCursor_Increment(void* p) {
    unsigned int limitAddress = AsRingCursor803F(p)->limitAddress;
    unsigned int nextAddress = AsRingCursor803F(p)->currentAddress + kPointerSlotBytes;
    AsRingCursor803F(p)->currentAddress = nextAddress;
    if (nextAddress == limitAddress) {
        AsRingCursor803F(p)->currentAddress = AsRingCursor803F(p)->beginAddress;
    }
}

void PointerRing_AllocateCapacity(void* self, unsigned int value, unsigned int limit) {
    // Cache (UINT_MAX / 4) - 1 so (limit + 1) * 4 cannot overflow.
    if (!s_gtlWordDequeMaxSizeInitialized) {
        s_gtlWordDequeMaxSizeInitialized = 1;
        s_gtlWordDequeMaxSize = 0x3ffffffe; // Maximum usable four-byte slots, excluding one spare.
    }
    if (limit > s_gtlWordDequeMaxSize) {
        // Build a length_error with a shared string. The guard owns the string
        // until its control block has been installed, then relinquishes cleanup.
        struct Error {
            void* vtable;
            char* what;
            void* control;
        } error;
        struct Control {
            void* vtable;
            int strong;
            int weak;
            char* data;
        };
        struct Guard {
            char* data;
            ~Guard() {
                if (data)
                    __dla__FPv(data);
            }
        } guard = {0};
        extern char s_sharedCharDeleterVtable[];
        error.vtable = __vt__Q23std11logic_error;
        unsigned int n = strlen(audioDequeErrorStrings.circularDequeLengthError) + 1;
        char* str = (char*)__nwa__FUl(n);
        guard.data = str;
        error.what = str;
        Control* control = (Control*)__nw__FUl(sizeof(Control));
        if (control) {
            control->vtable = s_sharedCharDeleterVtable;
            control->strong = control->weak = 1;
            control->data = str;
        }
        error.control = control;
        guard.data = 0;
        strcpy(str, audioDequeErrorStrings.circularDequeLengthError);
        error.vtable = __vt__Q23std12length_error;
        __throw(audioDequeErrorStrings.lengthErrorType, &error, (void*)__dt__Q23std12length_errorFv);
    }
    // Commit the allocation and its physical slot count, leaving size/origin alone.
    unsigned int count = limit + 1;
    AsPointerRing803F(self)->dataAddress = (unsigned int)gtl_Alloc(count * sizeof(void*));
    AsPointerRing803F(self)->capacity = count;
}

void* PointerRing_GetAllocatorValue(void* p) {
    return (void*)AsPointerRing803F(p)->allocator;
}

} // extern "C"

void PointerRing803F::PrependRange(void* rangeBegin, void* rangeEnd) {
    unsigned int beginAddress = AsRingCursor803F(rangeBegin)->currentAddress;
    unsigned int endAddress = AsRingCursor803F(rangeEnd)->currentAddress;
    unsigned int* insertionEnd = (unsigned int*)dataAddress + (unsigned int)firstIndex;
    if (beginAddress <= endAddress) {
        // Contiguous source: place it immediately before the destination's origin.
        int count = (int)(endAddress - beginAddress) / kPointerSlotBytes;
        memmove(insertionEnd - count, (void*)beginAddress, count * kPointerSlotBytes);
        int moved = (int)(endAddress - beginAddress) / kPointerSlotBytes;
        size += moved;
        firstIndex -= moved;
    } else {
        // Wrapped source: place its head nearest insertionEnd, then its tail
        // before that head. Logical source order is still tail followed by head.
        unsigned int wrappedBegin = AsRingCursor803F(rangeEnd)->beginAddress;
        int headCount = (int)(endAddress - wrappedBegin) / kPointerSlotBytes;
        unsigned int* headDestination = insertionEnd - headCount;
        memmove(headDestination, (void*)wrappedBegin, headCount * kPointerSlotBytes);
        unsigned int wrappedEnd = AsRingCursor803F(rangeBegin)->limitAddress;
        int tailCount = (int)(wrappedEnd - beginAddress) / kPointerSlotBytes;
        memmove(headDestination - tailCount, (void*)beginAddress, tailCount * kPointerSlotBytes);
        firstIndex -= headCount + tailCount;
        size += headCount + tailCount;
    }
}

// Initialize an empty ring with its allocator token.
void PointerRing803F::Init(int allocatorToken) {
    allocator = allocatorToken;
    dataAddress = 0;
    firstIndex = 0;
    size = 0;
    capacity = 0;
}

extern "C" {

void PointerRing_AppendCursorRange(void* dst, void* rangeBegin, void* rangeEnd) {
    unsigned int endAddress = AsRingCursor803F(rangeEnd)->currentAddress;
    unsigned int beginAddress = AsRingCursor803F(rangeBegin)->currentAddress;
    int firstIndex = AsPointerRing803F(dst)->firstIndex;
    int currentSize = (int)AsPointerRing803F(dst)->size;
    unsigned int capacity = AsPointerRing803F(dst)->capacity;
    unsigned int insertionIndex = (unsigned int)(firstIndex + currentSize) % capacity;
    unsigned int writeAddress = AsPointerRing803F(dst)->dataAddress + insertionIndex * kPointerSlotBytes;
    if (beginAddress <= endAddress) {
        unsigned int bytes = endAddress - beginAddress;
        memmove((void*)writeAddress, (void*)beginAddress, bytes & ~(unsigned int)(kPointerSlotBytes - 1));
        unsigned int count = bytes / kPointerSlotBytes;
        unsigned int copiedSlots =
            (AsRingCursor803F(rangeEnd)->currentAddress - AsRingCursor803F(rangeBegin)->currentAddress) /
            kPointerSlotBytes;
        AsPointerRing803F(dst)->size += copiedSlots;
    } else {
        // Copy tail then head into consecutive destination slots; update size
        // only after both source segments have been copied.
        unsigned int wrappedEnd = AsRingCursor803F(rangeBegin)->limitAddress;
        unsigned int tailBytes = wrappedEnd - beginAddress;
        unsigned int tailSlots = tailBytes / kPointerSlotBytes;
        memmove((void*)writeAddress, (void*)beginAddress, tailBytes & ~(unsigned int)(kPointerSlotBytes - 1));
        unsigned int wrappedBegin = AsRingCursor803F(rangeEnd)->beginAddress;
        unsigned int headBytes = endAddress - wrappedBegin;
        memmove((void*)(writeAddress + tailSlots * kPointerSlotBytes), (void*)wrappedBegin,
                headBytes & ~(unsigned int)(kPointerSlotBytes - 1));
        unsigned int headSlots = headBytes / kPointerSlotBytes;
        AsPointerRing803F(dst)->size += tailSlots + headSlots;
    }
}

void* PointerRing_GetCapacityAddress(void* p) {
    return &AsPointerRing803F(p)->capacity;
}

void* PointerRing_GetCapacityAddressForConstruction(void* p) {
    return &AsPointerRing803F(p)->capacity;
}

void* GtlWordStorage_ReadFirstWord(void* p) {
    return (void*)*(unsigned int*)p;
}

} // extern "C"

int GtlSampleDeque::GetMaxSize(void* unused) {
    if (!s_gtlSampleDequeMaxSizeInitialized) {
        s_gtlSampleDequeMaxSize = -kSampleDequeReservedElements;
        s_gtlSampleDequeMaxSizeInitialized = 1;
    }
    return s_gtlSampleDequeMaxSize;
}

extern "C" {

void* SampleDeque_AllocatePointerSlots(void* unused, unsigned int slotCount) {
    return gtl_Alloc(slotCount * kPointerSlotBytes);
}

void* SampleDeque_InitializeEmptyStorage(void* self) {
    DequeStorage* storage = (DequeStorage*)self;
    storage->metadataWords[0] = 0;
    storage->metadataWords[1] = 0;
    storage->data = 0;
    storage->firstIndex = 0;
    storage->size = 0;
    storage->capacity = 0;

    // Zero all matching scratch, then read the four range words. Their original
    // higher-level aggregate layout is unresolved. With these zero writes the
    // range is empty; volatile reads keep the following generic path in .text.
    enum ScratchWords {
        kScratchWordCount = 68,
        kRangeBeginWord = 16,
        kRangeEndWord = 17,
        kRangePositionWord = 18,
        kRangeLimitWord = 19
    };
    volatile unsigned int scratchWords[kScratchWordCount];
    for (int i = 0; i < kScratchWordCount; ++i)
        scratchWords[i] = 0;
    unsigned int* begin = (unsigned int*)scratchWords[kRangeBeginWord];
    unsigned int* end = (unsigned int*)scratchWords[kRangeEndWord];
    unsigned int* pos = (unsigned int*)scratchWords[kRangePositionWord];
    unsigned int* rangeLimit = (unsigned int*)scratchWords[kRangeLimitWord];
    unsigned int count = (unsigned int)(end - begin);
    if (count != 0) {
        // Generic nonempty path: reject overflow before allocating a ring with
        // one spare slot. It remains present even though the scratch was zeroed.
        if (count > s_gtlWordContainerMaxSize) {
            ErrObj error;
            error.vtable = __vt__Q23std11logic_error;
            error.what = (char*)0;
            unsigned int n = (strlen(audioDequeErrorStrings.circularDequeLengthError) + 1) * 2;
            char* str = (char*)__nwa__FUl(n);
            error.what = str;
            strcpy(str, audioDequeErrorStrings.circularDequeLengthError);
            error.vtable = __vt__Q23std12length_error;
            __throw(audioDequeErrorStrings.lengthErrorType, &error, (void*)__dt__Q23std12length_errorFv);
        }
        unsigned int* data = (unsigned int*)gtl_Alloc((count + 1) * kPointerSlotBytes);
        ((DequeStorage*)self)->data = data;
        ((DequeStorage*)self)->capacity = count + 1;
        unsigned int* dest = data;
        // Preserve the reconstructed range-copy loop and its existing increments.
        // Do not infer a normal STL iterator contract from this scratch-only path.
        while (pos != end) {
            unsigned int* next = pos + 1;
            memmove(dest, pos, (unsigned int)((char*)rangeLimit - (char*)pos));
            dest += (rangeLimit - pos);
            pos = next;
        }
    }
    return self;
}

unsigned int SampleDeque_GetUsableElementCapacity(void* p) {
    // See GtlSampleDeque::rowCount in classes/GtlSampleDeque.h. Use an offset
    // view here so no new type declarations renumber the earlier local EH type.
    unsigned int pageCount = *(unsigned int*)((char*)p + kSampleDequePageCountOffset);
    if (pageCount == 0)
        return 0;
    return (pageCount - 1) * kElementsPerSamplePage;
}

void Memory_CopyWordIfDestinationNonNull(void* unused, int* out, void* src) {
    volatile int dummy = 0;
    (void)dummy;
    if (out != 0) {
        int sourceWord = *(int*)src;
        *out = sourceWord;
    }
}

void* PointerRing_dtor(void* p, int do_delete) {
    if (p != 0) {
        void* data = (void*)AsPointerRing803F(p)->dataAddress;
        if (data != 0) {
            volatile int* field = (volatile int*)&AsPointerRing803F(p)->size;
            *field = *field - *field;
            gtl_Free(data);
        }
        if (do_delete > 0)
            __dl__FPv(p);
    }
    return p;
}

} // extern "C"

// Counter member definitions. The counter is borrowed from its owner.
void gtl::IShowCounter::Init() {
    unk_0 = 1;
    vtbl = (void*)__vt__Q23gtl12IShowCounter;
    state = kResetOnStart;
    count = 0;
    previousCount = 0;
}

int gtl::IShowCounter::GetCount(int* out) {
    *out = count;
    return 1;
}

int gtl::IShowCounter::GetPreviousCount(int* out) {
    *out = previousCount;
    return 1;
}

int gtl::IShowCounter::StartCounting() {
    int previousState = state;
    if (previousState == kCounting) {
        return 0;
    }
    if (previousState == kResetOnStart) {
        count = 0;
        previousCount = 0;
    }
    state = kCounting;
    return 1;
}

int gtl::IShowCounter::Pause() {
    if (state == kPaused) {
        return 0;
    }
    state = kPaused;
    return 1;
}

int gtl::IShowCounter::ArmResetOnStart() {
    if (state == kResetOnStart) {
        return 0;
    }
    state = kResetOnStart;
    return 1;
}

int gtl::IShowCounter::ResetAndStartCounting() {
    ((CounterDispatch*)this)->update();
    ((CounterDispatch*)this)->armReset();
    return 1;
}
