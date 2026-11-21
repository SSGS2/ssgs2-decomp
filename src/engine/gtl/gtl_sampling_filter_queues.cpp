#include "classes/PointerRing803F.h"
#include "classes/GtlSourceQueue803F.h"
#include "classes/GtlPagedQueue803F.h"

struct SamplingInterfacePrefix { unsigned int references; };
struct SamplingInterface : SamplingInterfacePrefix {
    virtual void Release(int);
    virtual void Slot1();
    virtual void Start();
    virtual void Slot3();
    virtual void Reset();
};

// Normal destruction remains explicit. This owner handles exceptional exits.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Arm destruction of the temporary queue on an exceptional exit.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Disarm before the explicit normal-path destruction.
    void release() { active = false; }
    // Destroy the temporary subobject without deleting its stack storage.
    ~EH5Cleanup() { if (active) destroy(object, -1); }
};

// Translation unit 0x803F5F00-0x803F74E4 (27 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Two nested containers, with different units:
//   sample queue: firstIndex/size count 8-byte {sample*, consumedBytes} entries;
//   pages ring: firstIndex/size/capacity count 4-byte page-pointer slots.
// Each owned page is 0x400 bytes = 128 entries. The logical page-pointer range
// ends with a null sentinel, so (pageCapacity - 1) * 128 is entry capacity.
// The ring allocation also reserves one slot outside its logical range.
// Address entry i by pos=sample.firstIndex+i, then ring[firstPage+pos/128]
// (wrapped in the ring), then (pos%128)*8 bytes within that page.
// Example: pages [A, B, null], firstIndex=64, size=80 uses A[64..127]
// and B[0..15]; front spare=64 entries, back spare=112 entries.
// Page rotation changes firstIndex by 128 to preserve each logical sample.
// Queue destruction frees page storage and the ring, not sample payloads.
// Unsigned word views retain the original loads/stores (including volatile reloads).
struct RingWordState { unsigned int dataAddress, firstIndex, size, capacity; };
struct PagedSampleQueue { unsigned int firstIndex, size; RingWordState pages; };
struct SampleReference { unsigned int references; void** vtable; };
struct RingIteratorWords { unsigned int current, base, wrapPoint, limit; };
enum SamplingQueueConstants {
    kPageIndexShift = 7, kEntriesPerPage = 0x80, kPageIndexMask = 0x7F,
    kSampleEntryShift = 3, kSampleEntryBytes = 8, kSamplePageBytes = 0x400,
    kRetireFrontThreshold = 0x100, kEmptyQueueStart = 0x40,
    kWordContainerMaxSize = 0x3FFFFFFE, kDestroySlot = 2,
    kSamplingResetSlotOffset = 0x18, kSamplingStartSlotOffset = 0x10,
    kSamplingRequiredSizeSlotOffset = 0x30
};

// Forward the supplied status as a boolean.

int gtl_CSamplingFilter_vfunc_8(void* unused1, void* unused2, int value) {
    return value != 0;
}

// Reset the sampling stage, then start it through its virtual interface.
int gtl_CSamplingFilter_ResetAndStart(void* self) {
    ((SamplingInterface*)self)->Reset();
    ((SamplingInterface*)self)->Start();
    return 1;
}

void gtl_CMediaSample_GetData(void*, unsigned int*);
void* UnsignedWord_SelectMaxPointer(void*, void*);

// Take the upstream sample-pool pointer from a media sample's data word.
int gtl_CSamplingFilter_SetSourceFromSample(void* self, void* value) {
    if (value == 0) return 0;
    unsigned int result;
    gtl_CMediaSample_GetData(value, &result);
    (*(unsigned int*)&((GtlSourceQueue803F*)self)->source) = result;
    return 1;
}

// Return and retain the filter's shared reference-count handle.
int gtl_CSamplingFilter_AcquireSharedRef(void* self, void* unused, unsigned int* out) {
    if (out == 0) {
        return 0;
    }
    unsigned int* count = ((GtlSourceQueue803F*)self)->sharedCounter;
    *out = (unsigned int)count;
    ++*count;
    return 1;
}

// Shrink the logical pointer range by one without freeing or clearing its last slot.
void PointerRing803F_DecrementSize(void* self) {
    --(*(int*)&((PointerRing803F*)self)->size);
}

// Borrow the address of the ring's first pointer slot; requires a nonempty ring.
void* PointerRing803F_GetFrontSlot(void* self) {
    return (char*)*(void**)self + 4 * ((PointerRing803F*)self)->firstIndex;
}

#include "sdk_protos.h"

extern unsigned char s_gtlWordContainerMaxSizeInitialized;
extern unsigned char s_gtlWordDequeMaxSizeInitialized;
extern unsigned int s_gtlWordContainerMaxSize;
extern unsigned int s_gtlWordDequeMaxSize;
struct DequeLengthErrorPool {
    char dequeLengthError[21];
    char lengthErrorType[55];
    char circularDequeLengthError[20];
};
extern DequeLengthErrorPool dequeLengthErrorStrings;
extern unsigned int __vt__Q23std11logic_error[];
extern unsigned int __vt__Q23std12length_error[];
extern unsigned int s_sharedCharDeleterVtable[];
extern void __dt__Q23std12length_errorFv(void*);
extern void* __nwa__FUl(unsigned int);
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void __dl__FPv(void*);
extern void* gtl_Free(void*);
extern void* gtl_Alloc(unsigned int);
// Construct an owned diagnostic string and throw std::length_error with the supplied type chain.
static void AllocSize_ThrowLength(const char* text, const char* typeName, void* destroy) {
    struct Error { void* table; char* message; void* control; } error;
    struct Control { void* table; int strong; int weak; char* data; };
    struct Guard {
        char* data;
        // Free diagnostic text if control-block allocation exits exceptionally.
        ~Guard() { if (data) __dla__FPv(data); }
    } guard = { 0 };
    error.table = (void*)__vt__Q23std11logic_error;
    guard.data = (char*)__nwa__FUl(strlen(text) + 1);
    error.message = guard.data;
    Control* control = (Control*)__nw__FUl(0x10);
    if (control) {
        control->table = (void*)s_sharedCharDeleterVtable;
        control->strong = control->weak = 1;
        control->data = guard.data;
    }
    error.control = control;
    guard.data = 0;
    strcpy(error.message, text);
    error.table = (void*)__vt__Q23std12length_error;
    __throw(typeName, &error, destroy);
}
// Prepend one word; grow and linearize the ring when its sentinel slot would be consumed.
void GtlWordDeque_PushFront(void* self, void* value) {
    volatile RingWordState* ring = (volatile RingWordState*)self;
    unsigned int size = ring->size;
    unsigned int capacity = ring->capacity;
    if (size + 1 < capacity) {
        unsigned int insertionIndex = ring->firstIndex;
        if (ring->firstIndex == 0) insertionIndex = capacity;
        --insertionIndex;
        unsigned int* slot = (unsigned int*)ring->dataAddress + insertionIndex;
        if (slot != 0) {
            unsigned int item = *(unsigned int*)value;
            *slot = item;
        }
        ring->firstIndex = insertionIndex;
        ring->size = size + 1;
        return;
    }

    // Grow the circular buffer and move its elements around the insertion point.
    unsigned int first = capacity != 0 ? capacity - 1 : 0;
    if (!s_gtlWordContainerMaxSizeInitialized) {
        s_gtlWordContainerMaxSizeInitialized = 1;
        s_gtlWordContainerMaxSize = kWordContainerMaxSize;
    }
    unsigned int maxSize = s_gtlWordContainerMaxSize;
    if (1u > maxSize - first)
        AllocSize_ThrowLength(dequeLengthErrorStrings.circularDequeLengthError,
                              dequeLengthErrorStrings.lengthErrorType,
                              (void*)__dt__Q23std12length_errorFv);
    unsigned int newCapacity;
    if (first < maxSize / 3) {
        unsigned int grown = (first + 1) * 3 / 5;
        newCapacity = first + (grown < 1 ? 1 : grown);
    } else if (first < (maxSize / 3) * 2) {
        unsigned int grown = (first + 1) / 2;
        newCapacity = first + (grown < 1 ? 1 : grown);
    } else {
        newCapacity = maxSize;
    }
    if (!s_gtlWordDequeMaxSizeInitialized) {
        s_gtlWordDequeMaxSizeInitialized = 1;
        s_gtlWordDequeMaxSize = kWordContainerMaxSize;
    }
    if (newCapacity > s_gtlWordDequeMaxSize)
        AllocSize_ThrowLength(dequeLengthErrorStrings.circularDequeLengthError,
                              dequeLengthErrorStrings.lengthErrorType,
                              (void*)__dt__Q23std12length_errorFv);
    unsigned int* newData = (unsigned int*)gtl_Alloc((newCapacity + 1) * 4);
    if (newData != 0) *newData = *(unsigned int*)value;
    unsigned int used = ring->size;
    if (used != 0) {
        unsigned int start = ring->firstIndex;
        unsigned int* base = (unsigned int*)ring->dataAddress;
        for (unsigned int i = 0; i < used; ++i)
            newData[1 + i] = base[(start + i) % ring->capacity];
    }
    unsigned int* oldData = (unsigned int*)ring->dataAddress;
    ring->dataAddress = (unsigned int)newData;
    ring->firstIndex = 0;
    ring->size = used + 1;
    ring->capacity = newCapacity + 1;
    if (oldData != 0) gtl_Free(oldData);
}
void GtlWordDeque_PushBack(void*, void*);
void* SamplePool_RecycleReturnedSample(void*);

// Forward insertion of the word pointed to by value; no payload retain occurs here.
void GtlWordDeque_ForwardPushFront(void* self, void* value) {
    GtlWordDeque_PushFront(self, value);
}

// Append the supplied pointer word through the shared deque implementation.
void PointerRing_PushBack(void* self, void* value) {
    GtlWordDeque_PushBack(self, value);
}

// Delegate recycling to the upstream source pool and return its result unchanged.
void* gtl_CSamplingFilter_RecycleReturnedSample(void* self) {
    return SamplePool_RecycleReturnedSample(((GtlSourceQueue803F*)self)->source);
}

// Borrow the embedded ring state at paged-queue +8.
void* GtlPagedQueue803F_GetPageRing(void* self) {
    return &((PagedSampleQueue*)self)->pages;
}

// Return the allocator context, which shares the embedded ring's address.
void* GtlPagedQueue803F_GetPageAllocator(void* self) {
    return &((PagedSampleQueue*)self)->pages;
}

// Return front spare ENTRY count encoded in the pointer-sized return register.
void* GtlPagedQueue803F_GetFrontSpareCount(void* self) {
    return (void*)((GtlPagedQueue803F*)self)->firstIndex;
}

// Return back spare entries, excluding the final null page-pointer sentinel.
unsigned int GtlPagedQueue803F_GetBackSpareCount(void* self) {
    unsigned int count = ((GtlPagedQueue803F*)self)->pageCapacity;
    unsigned int offset = count != 0 ? (count - 1) << kPageIndexShift : 0;
    return offset - (((GtlPagedQueue803F*)self)->firstIndex + ((GtlPagedQueue803F*)self)->size);
}

// Remove one logical pointer slot and wrap the origin; do not free its page.
void PointerRing_PopFront(void* self) {
    unsigned int index = (*(unsigned int*)&((PointerRing803F*)self)->firstIndex) + 1;
    unsigned int capacity = ((PointerRing803F*)self)->capacity;
    (*(unsigned int*)&((PointerRing803F*)self)->firstIndex) = index;
    if (index == capacity) {
        (*(unsigned int*)&((PointerRing803F*)self)->firstIndex) = 0;
    }
    --((PointerRing803F*)self)->size;
}

// Exchange all four ring words; page storage ownership follows the exchanged state.
void GtlPagedQueue803F_SwapPageRingState(unsigned int* a, unsigned int* b) {
    unsigned int t = a[0];
    a[0] = b[0];
    b[0] = t;
    t = a[1];
    a[1] = b[1];
    b[1] = t;
    t = a[2];
    a[2] = b[2];
    b[2] = t;
    t = a[3];
    a[3] = b[3];
    b[3] = t;
}

struct PageRingIterator { unsigned int* current; unsigned int* first; unsigned int* last; unsigned int* limit; };
struct PageRingSource { unsigned int* base; unsigned int first; unsigned int last; unsigned int count; };
// Build the wrapped end iterator from the ring's first index, size, and capacity.
void GtlPagedQueue803F_BuildPageRingEndIterator(PageRingIterator* out, PageRingSource* source) {
    volatile RingWordState* src = (volatile RingWordState*)source;
    unsigned int count = src->capacity;
    unsigned int index;
    if (count != 0) index = (src->firstIndex + src->size) % count;
    else index = 0;
    unsigned int first = src->firstIndex;
    unsigned int base = src->dataAddress;
    unsigned int endCount = src->capacity;
    out->first = (unsigned int*)base;
    out->current = (unsigned int*)(base + (index << 2));
    out->last = (unsigned int*)(base + (first << 2));
    out->limit = (unsigned int*)(base + (endCount << 2));
}

// Build a begin iterator with the ring's allocation limits and wrap point.
void GtlPagedQueue803F_MakeBeginIterator(unsigned int* out, unsigned int* source) {
    RingIteratorWords* iterator = (RingIteratorWords*)out;
    volatile RingWordState* src = (volatile RingWordState*)source;
    unsigned int first = src->firstIndex;
    unsigned int last = src->capacity;
    unsigned int base = src->dataAddress;
    first <<= 2;
    last <<= 2;
    unsigned int begin = base + first;
    iterator->base = base;
    unsigned int end = base + last;
    iterator->current = begin;
    iterator->wrapPoint = begin;
    iterator->limit = end;
}

// Return the ring slot immediately preceding its logical end (capacity must be nonzero).
unsigned int* GtlPagedQueue803F_GetBackPageSlot(unsigned int* self) {
    // This word view preserves aliasing when the helper is inlined into page growth.
    unsigned int start = self[1];
    unsigned int offset = self[2];
    unsigned int count = self[3];
    unsigned int* base = (unsigned int*)self[0];
    unsigned int index = start + offset;
    unsigned int last = index - 1;
    unsigned int quotient = last / count;
    unsigned int rem = last - quotient * count;
    return (unsigned int*)((char*)base + (rem << 2));
}

void* gtl_Free(void* p);
void __dl__FPv(void* p);
void PointerRing_InsertRingIteratorRange(void*, void*, void*, void*, void*);

// Copy a ring iterator retreated by param slots, wrapping once within its allocation.
void GtlPagedQueue803F_CopyRetreatedIterator(unsigned int* out, unsigned int* in, int param) {
    int back = -param;
    unsigned int first = in[1];
    unsigned int end = in[3];
    unsigned int current = in[0];
    unsigned int last = in[2];
    int count = (int)(end - first) / 4;
    int index = back + (int)(current - first) / 4;
    if (index < 0) index += count;
    else if (index >= count) index -= count;
    unsigned int* cur = (unsigned int*)(first + index * 4);
    out[0] = (unsigned int)cur;
    out[1] = first;
    out[2] = last;
    out[3] = end;
}

struct IteratorCopyWords { int x[4]; };
#pragma use_lmw_stmw on
// Pass private iterator copies to range insertion so the caller's iterators are unchanged.
void PointerRing_InsertCopiedIteratorRange(void* a, const void* pA, const void* pB, const void* pC) {
    IteratorCopyWords bufC = *(const IteratorCopyWords*)pC;
    IteratorCopyWords bufB = *(const IteratorCopyWords*)pB;
    IteratorCopyWords bufA = *(const IteratorCopyWords*)pA;
    char flag = 0;
    PointerRing_InsertRingIteratorRange(a, &bufA, &bufB, &bufC, &flag);
}
#pragma use_lmw_stmw reset

void* gtl_Free(void*);
void GtlPagedQueue803F_EnsureFrontSpareCapacity(void*, unsigned int);

unsigned int SamplePool_DequeueSample(void*, void**);
void gtl_CMediaSample_GetSize(void*, unsigned int*);
void GtlPagedQueue803F_EnsureBackSpareCapacity(void*, unsigned int);

// Retain all upstream samples in the paged queue and test their unread bytes against the required size.
// A dequeued item contributes a temporary reference; enqueue adds its own, then
// the temporary is released. New entries start at consumedBytes=0. Return 1 if
// sum(sample size in bytes - consumedBytes) meets the virtual required-byte count.
// Assumes valid sample pointers and consumedBytes <= sample size; no validation.
int gtl_CSamplingFilter_CollectAndCheckInput(void* self) {
    void* item = 0;
    while (SamplePool_DequeueSample(((GtlSourceQueue803F*)self)->source, &item) == 1) {
        void* queuedSample = item;
        void* zeroConsumedBytes = 0;
        ++((SampleReference*)queuedSample)->references;
        // Phase 1: make room, then resolve the possibly moved page-ring origin.
        unsigned int pageCapacity = ((GtlSourceQueue803F*)self)->pageCapacity;
        unsigned int entryPosition = ((GtlSourceQueue803F*)self)->firstIndex + ((GtlSourceQueue803F*)self)->size;
        if ((((pageCapacity != 0) ? pageCapacity - 1 : 0) << kPageIndexShift) == entryPosition) {
            GtlPagedQueue803F_EnsureBackSpareCapacity(&((GtlSourceQueue803F*)self)->firstIndex, 1);
            // 0x803F6348/0x803F6350: the original reloads +0x20 and +0x1c after the grow call at 0x803F6330;
            // GtlPagedQueue803F_EnsureBackSpareCapacity can move a spare front block to the back and lower +0x1c by kEntriesPerPage, so the old pos is stale.
            entryPosition = ((GtlSourceQueue803F*)self)->firstIndex + ((GtlSourceQueue803F*)self)->size;
        }

        unsigned int* pagePointers = (*(unsigned int**)&((GtlSourceQueue803F*)self)->pagesAddress);
        unsigned int pointerSlotCount = ((GtlSourceQueue803F*)self)->pageCount;
        unsigned int firstPageSlot = ((GtlSourceQueue803F*)self)->firstPage;
        unsigned int pageSlot = firstPageSlot + (entryPosition >> kPageIndexShift);
        if ((int)pageSlot < 0) pageSlot += pointerSlotCount;
        else if (pageSlot >= pointerSlotCount) pageSlot -= pointerSlotCount;
        // Phase 2: publish {retained sample, zero consumed bytes} at the logical tail.
        unsigned int pageAddress = pagePointers[pageSlot];
        void* tailEntry = (void*)(pageAddress + ((entryPosition & kPageIndexMask) << kSampleEntryShift));
        if (tailEntry) { ((void**)tailEntry)[0] = queuedSample; ((void**)tailEntry)[1] = zeroConsumedBytes; }
        ++((GtlSourceQueue803F*)self)->size;

        // Release the dequeued sample's temporary reference; destroy it if the count reaches zero.
        unsigned int referencesAfterRelease = --((SampleReference*)item)->references;
        if (referencesAfterRelease == 0)
            (*(void (**)(void*, short))((char*)((SampleReference*)item)->vtable + 8))(item, 1);
        item = 0;
    }
    if (item) {
        unsigned int referencesAfterRelease = --((SampleReference*)item)->references;
        if (referencesAfterRelease == 0) (*(void (**)(void*, short))((char*)((SampleReference*)item)->vtable + 8))(item, 1);
    }
    // Phase 3: query the demand, then scan queued entries without consuming them.
    unsigned int requiredBytes = 0;
    (*(void (**)(void*, unsigned int*))((char*)(*(void**)&((GtlSourceQueue803F*)self)->vtable) + kSamplingRequiredSizeSlotOffset))(self, &requiredBytes);

    // Sum (sample size - consumedBytes) over every queued entry, then compare with requiredBytes.
    unsigned int unreadBytes = 0;
    unsigned int queuedEntries = ((GtlSourceQueue803F*)self)->size;
    if (((GtlSourceQueue803F*)self)->pageCapacity != 0) {
        for (unsigned int i = 0; i < queuedEntries; ++i) {
            unsigned int entryPosition = ((GtlSourceQueue803F*)self)->firstIndex + i;
            unsigned int pointerSlotCount = ((GtlSourceQueue803F*)self)->pageCount;
            unsigned int pageSlot = ((GtlSourceQueue803F*)self)->firstPage + (entryPosition >> kPageIndexShift);
            if (pageSlot >= pointerSlotCount) pageSlot -= pointerSlotCount;
            unsigned int* sampleEntry = (unsigned int*)(((unsigned int*)((GtlSourceQueue803F*)self)->pagesAddress)[pageSlot] + ((entryPosition & kPageIndexMask) << kSampleEntryShift));
            unsigned int sampleBytes = 0;
            gtl_CMediaSample_GetSize((void*)sampleEntry[0], &sampleBytes);
            unreadBytes += sampleBytes - sampleEntry[1];
        }
    }
    return unreadBytes >= requiredBytes;
}

// Remove the first sample/consumed-byte pair, retiring fully passed pages and recentering an empty queue.
// Return 0 with both outputs untouched if empty; otherwise write the sample word
// to out and consumed BYTE count to out2 and return 1. Both outputs must be valid.
// A nonempty queue must contain a nonnull sample: its count is read before the
// source's null comparison, so that comparison does not make null entries safe.
// No reference increment OR decrement occurs here. After publishing the pointer,
// an already-zero reference count invokes deleting destruction; *out can then be
// stale. With a nonzero count the caller receives the existing reference as-is.
// This does not provide an unconditional retained/usable sample-pointer contract.
int gtl_CSamplingFilter_DequeueInputSample(void* self, void* out, void* out2) {
    if (((GtlSourceQueue803F*)self)->size == 0) return 0;
    unsigned int cursor = ((GtlSourceQueue803F*)self)->firstIndex;
    unsigned int slot = (((GtlSourceQueue803F*)self)->firstPage + (cursor >> kPageIndexShift)) % ((GtlSourceQueue803F*)self)->pageCount;
    unsigned int offset = (cursor & kPageIndexMask) << kSampleEntryShift;
    unsigned int* page = ((unsigned int**)((GtlSourceQueue803F*)self)->pagesAddress)[slot];
    unsigned int* item = (unsigned int*)((char*)page + offset);
    unsigned int* object = (unsigned int*)*item;
    unsigned int result = item[1];
    *(unsigned int*)out = (unsigned int)object;
    if (object[0] == 0 && object != 0) {
        ((SamplingInterface*)object)->Release(1);
    }
    *(unsigned int*)out2 = result;
    // Advance the logical front, keeping one page's worth of front slack.
    --((GtlSourceQueue803F*)self)->size;
    ++((GtlSourceQueue803F*)self)->firstIndex;
    while (((GtlSourceQueue803F*)self)->firstIndex >= kRetireFrontThreshold) {
        gtl_Free(((void**)((GtlSourceQueue803F*)self)->pagesAddress)[((GtlSourceQueue803F*)self)->firstPage]);
        ++((GtlSourceQueue803F*)self)->firstPage;
        if (((GtlSourceQueue803F*)self)->firstPage == ((GtlSourceQueue803F*)self)->pageCount) ((GtlSourceQueue803F*)self)->firstPage = 0;
        --((GtlSourceQueue803F*)self)->pageCapacity;
        ((GtlSourceQueue803F*)self)->firstIndex -= kEntriesPerPage;
    }
    if (((GtlSourceQueue803F*)self)->size == 0) ((GtlSourceQueue803F*)self)->firstIndex = kEmptyQueueStart;
    return 1;
}

// Retain and prepend a sample with its consumed-byte count, growing front spare space if needed.
int gtl_CSamplingFilter_PrependInputSample(void* self, unsigned int* sample, unsigned int consumedBytes) {
    ++*sample;
    GtlSourceQueue803F* queue = (GtlSourceQueue803F*)self;
    if (queue->firstIndex == 0) GtlPagedQueue803F_EnsureFrontSpareCapacity(&queue->firstIndex, 1);
    unsigned int position = queue->firstIndex;
    unsigned int* base = (unsigned int*)queue->pagesAddress;
    unsigned int pageCount = queue->pageCount;
    unsigned int pageIndex = queue->firstPage + (position >> kPageIndexShift);
    if ((int)pageIndex < 0) pageIndex += pageCount;
    else if (pageIndex >= pageCount) pageIndex -= pageCount;
    unsigned int* node = base + pageIndex;
    unsigned int page = queue->pageCapacity != 0 ? *node : 0;
    unsigned int entryAddress = page + ((position & kPageIndexMask) << kSampleEntryShift);
    if (entryAddress == *node) {
        // at the start of a page: step back to the end of the previous page
        if (node == base) node = base + pageCount;
        entryAddress = node[-1] + kSamplePageBytes;
    }
    unsigned int* entry = (unsigned int*)(entryAddress - 8);
    if (entry != 0) {
        entry[0] = (unsigned int)sample;
        entry[1] = consumedBytes;
    }
    --queue->firstIndex;
    ++queue->size;
    return 1;
}

unsigned int GtlPagedQueue803F_GetMaxSize(void* self);
unsigned int GtlPagedQueue803F_GetStorageEntryCapacity(void* self);
unsigned int PointerRing_GetUsableCapacity(void* self);
void RingBuffer_ThrowLengthError(void*);
unsigned int PointerRing_GetSize(void* self);
unsigned int PointerRing_IsEmpty(void* self);
void* GtlPagedQueue803F_AllocatePageEntries(void* self, unsigned int size);
void GtlPagedQueue803F_Ctor(void* out, void* in);
void GtlWordDeque_Reserve(void* self, unsigned int size);
void PointerRing_ClearSize(void* self);
void* PagedSampleQueue_dtor(void* self, int flag);

#pragma inline off
// Reuse back spare pages at the front, allocating a larger page ring only when necessary.
// size is additional ENTRY space, rounded up to whole 128-entry pages, not bytes.
// self points to the paged queue (the filter's firstIndex field), not the filter.
void GtlPagedQueue803F_EnsureFrontSpareCapacity(void* self, unsigned int size) {
    void* queue = GtlPagedQueue803F_GetPageRing(self);
    void* buffer = GtlPagedQueue803F_GetPageAllocator(self);
    unsigned int remaining = (size >> kPageIndexShift) + ((size & kPageIndexMask) != 0);
    unsigned int count = GtlPagedQueue803F_GetBackSpareCount(self);
    // Phase 1: rotate a wholly spare back page before the front. Remove the null
    // sentinel and page, prepend that page, then restore a final null sentinel.
    while (remaining != 0 && count >= kEntriesPerPage) {
        PointerRing803F_DecrementSize(queue);
        unsigned int value = *(unsigned int*)GtlPagedQueue803F_GetBackPageSlot((unsigned int*)queue);
        PointerRing803F_DecrementSize(queue);
        GtlWordDeque_ForwardPushFront(queue, &value);
        unsigned int zero = 0;
        PointerRing_PushBack(queue, &zero);
        count -= kEntriesPerPage;
        --remaining;
        *(unsigned int*)self += kEntriesPerPage;
    }
    if (remaining == 0) return;
    // Phase 2: check entry limits, then allocate into unused pointer-ring slots.
    unsigned int entryCount = remaining << kPageIndexShift;
    if (entryCount > GtlPagedQueue803F_GetMaxSize(self) || GtlPagedQueue803F_GetStorageEntryCapacity(self) > GtlPagedQueue803F_GetMaxSize(self) - entryCount)
        RingBuffer_ThrowLengthError((void*)&dequeLengthErrorStrings);
    unsigned int tail = PointerRing_GetUsableCapacity(queue);
    if (remaining + PointerRing_GetSize(queue) <= tail) {
        while (remaining != 0) {
            void* item = GtlPagedQueue803F_AllocatePageEntries(buffer, kEntriesPerPage);
            GtlWordDeque_ForwardPushFront(queue, &item);
            --remaining;
            *(unsigned int*)self += kEntriesPerPage;
        }
        return;
    }
    // Not enough room: build a larger queue, fill it, move the old entries over and swap it in.
    unsigned char local[0x60];
    GtlPagedQueue803F_Ctor(local, buffer);
    EH5Cleanup eh5Cleanup30(local, (void (*)(void*, int))PagedSampleQueue_dtor);
    void* replacement = GtlPagedQueue803F_GetPageRing(local);
    unsigned int target = remaining + PointerRing_GetSize(queue) + 1;
    unsigned int doubled = PointerRing_GetUsableCapacity(queue) << 1;
    unsigned int* larger = (unsigned int*)UnsignedWord_SelectMaxPointer(&doubled, &target);
    GtlWordDeque_Reserve(replacement, *larger);
    unsigned int zero = 0;
    PointerRing_PushBack(replacement, &zero);
    while (remaining != 0) {
        void* item = GtlPagedQueue803F_AllocatePageEntries(buffer, kEntriesPerPage);
        GtlWordDeque_ForwardPushFront(replacement, &item);
        --remaining;
    }
    unsigned int added = PointerRing_GetSize(replacement) - 1;
    if (PointerRing_IsEmpty(queue) == 0) {
        unsigned int a[4], b[4], c[4], d[4], e[4];
        GtlPagedQueue803F_BuildPageRingEndIterator((PageRingIterator*)a, (PageRingSource*)queue);
        GtlPagedQueue803F_CopyRetreatedIterator(b, a, 1);
        GtlPagedQueue803F_MakeBeginIterator(c, (unsigned int*)queue);
        GtlPagedQueue803F_BuildPageRingEndIterator((PageRingIterator*)d, (PageRingSource*)replacement);
        GtlPagedQueue803F_CopyRetreatedIterator(e, d, 1);
        PointerRing_InsertCopiedIteratorRange(replacement, e, c, b);
    }
    GtlPagedQueue803F_SwapPageRingState((unsigned int*)queue, (unsigned int*)replacement);
    // The old page pointers were copied, not cloned: disown them before cleanup.
    PointerRing_ClearSize(replacement);
    *(unsigned int*)self += added << kPageIndexShift;
    { eh5Cleanup30.release(); PagedSampleQueue_dtor(local, -1); }
}
#pragma inline reset

// Reuse front spare pages at the back, allocating a larger page ring only when necessary.
// size is additional entries rounded up to pages; ring capacity is pointer slots.
void GtlPagedQueue803F_EnsureBackSpareCapacity(void* self, unsigned int size) {
    void* queue = GtlPagedQueue803F_GetPageRing(self);
    void* buffer = GtlPagedQueue803F_GetPageAllocator(self);
    unsigned int remaining = (size >> kPageIndexShift) + ((size & kPageIndexMask) != 0);
    // Phase 1: move a spare front page onto the old null sentinel, append a new
    // sentinel, and subtract 128 from the entry origin to preserve live entries.
    while (remaining != 0 && (unsigned int)GtlPagedQueue803F_GetFrontSpareCount(self) >= kEntriesPerPage) {
        unsigned int value = *(unsigned int*)PointerRing803F_GetFrontSlot(queue);
        PointerRing_PopFront(queue);
        *GtlPagedQueue803F_GetBackPageSlot((unsigned int*)queue) = value;
        unsigned int zero = 0;
        PointerRing_PushBack(queue, &zero);
        --remaining;
        *(unsigned int*)self -= kEntriesPerPage;
    }
    if (remaining == 0) return;
    unsigned int entryCount = remaining << kPageIndexShift;
    // Phase 2: enforce entry limits and extend in place if pointer slots suffice.
    unsigned int capacity = GtlPagedQueue803F_GetMaxSize(self);
    if (entryCount > capacity || GtlPagedQueue803F_GetStorageEntryCapacity(self) > capacity - entryCount)
        RingBuffer_ThrowLengthError(&dequeLengthErrorStrings);

    unsigned int tail = PointerRing_GetUsableCapacity(queue);
    if (remaining + PointerRing_GetSize(queue) <= tail) {
        while (remaining != 0) {
            void* item = GtlPagedQueue803F_AllocatePageEntries(buffer, kEntriesPerPage);
            *GtlPagedQueue803F_GetBackPageSlot((unsigned int*)queue) = (unsigned int)item;
            unsigned int zero = 0;
            PointerRing_PushBack(queue, &zero);
            --remaining;
        }
        return;
    }

    // Phase 3: build a replacement ring, insert old pages before new back pages,
    // then exchange storage. Clearing its size prevents freeing transferred pages.
    unsigned char local[0x60];
    GtlPagedQueue803F_Ctor(local, buffer);
    EH5Cleanup eh5Cleanup31(local, (void (*)(void*, int))PagedSampleQueue_dtor);
    void* replacement = GtlPagedQueue803F_GetPageRing(local);
    PointerRing_GetSize(queue);
    unsigned int target = PointerRing_GetSize(queue) + remaining + 1;
    unsigned int doubled = PointerRing_GetUsableCapacity(queue) * 2;
    unsigned int* larger = (unsigned int*)UnsignedWord_SelectMaxPointer(&doubled, &target);
    GtlWordDeque_Reserve(replacement, *larger);
    unsigned int zero = 0;
    PointerRing_PushBack(replacement, &zero);
    while (remaining != 0) {
        void* item = GtlPagedQueue803F_AllocatePageEntries(buffer, kEntriesPerPage);
        *GtlPagedQueue803F_GetBackPageSlot((unsigned int*)replacement) = (unsigned int)item;
        PointerRing_PushBack(replacement, &zero);
        --remaining;
    }
    if (PointerRing_IsEmpty(queue) == 0) {
        unsigned int source[4], rotated[4], oldBegin[4], newBegin[4];
        GtlPagedQueue803F_BuildPageRingEndIterator((PageRingIterator*)source, (PageRingSource*)queue);
        GtlPagedQueue803F_CopyRetreatedIterator(rotated, source, 1);
        GtlPagedQueue803F_MakeBeginIterator(oldBegin, (unsigned int*)queue);
        GtlPagedQueue803F_MakeBeginIterator(newBegin, (unsigned int*)replacement);
        PointerRing_InsertCopiedIteratorRange(replacement, newBegin, oldBegin, rotated);
    }
    GtlPagedQueue803F_SwapPageRingState((unsigned int*)queue, (unsigned int*)replacement);
    PointerRing_ClearSize(replacement);
    { eh5Cleanup31.release(); PagedSampleQueue_dtor(local, -1); }
}

#pragma use_lmw_stmw on
// Free every owned sample page, the page-pointer ring, and optionally the queue object itself.
void* PagedSampleQueue_dtor(void* self, int flag) {
    PagedSampleQueue* queue = (PagedSampleQueue*)self;
    if (self != 0) {
    queue->size = 0;
    if (queue->pages.size != 0) {
        unsigned int divisor = queue->pages.capacity;
        unsigned int remainder;
        if (divisor != 0) {
            remainder = (queue->pages.firstIndex + queue->pages.size) % divisor;
        } else {
            remainder = 0;
        }
        void** base = (void**)queue->pages.dataAddress;
        void** start = base + queue->pages.firstIndex;
        void** endwrap = base + divisor;
        void** stop = base + remainder;
        while (start != stop) {
            if (*start != 0) gtl_Free(*start);
            ++start;
            if (start == endwrap) start = base;
        }
    }
    void** base2 = (void**)queue->pages.dataAddress;
    if (base2 != 0) {
        queue->pages.size = 0;
        gtl_Free(base2);
    }
    if (flag > 0) __dl__FPv(self);
    }
    return self;
}
#pragma use_lmw_stmw reset

} // extern "C"
