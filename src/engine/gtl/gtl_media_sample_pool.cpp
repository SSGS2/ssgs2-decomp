// Media sample pool: a fixed set of media samples recycled through a deque of pages.
// The pool hands samples out, takes them back after use and reclaims drained pages.
// The file also has the media init, filter-manager and update entry points.

#include "classes/GtlMediaSample.h"
#include "classes/GtlSamplePool.h"
#define AsGtlSamplePool(p) ((GtlSamplePool*)(p))
#include "classes/GtlSampleDeque.h"
#define AsGtlSampleDeque(p) ((GtlSampleDeque*)(p))

// Normal destruction remains explicit. This owner handles exceptional exits.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Arm destruction of a partially built container on an exceptional exit.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Hand cleanup back to the explicit normal-path destructor.
    void release() {
        active = false;
    }
    // Destroy only while the enclosing operation still owns unwind cleanup.
    ~EH5Cleanup() {
        if (active)
            destroy(object, -1);
    }
};

// The page pointer table uses void pointers during destruction.
struct SamplePageStorageView {
    unsigned int position, count;
    void** rows;
    unsigned int head;
    int rowCount;
    unsigned int capacity;
};

enum SamplePageConstants {
    kPageSlotBits = 8,
    kSlotsPerPage = 256,
    kPageSlotMask = 255,
    kInitialPagePosition = 128,
    kPageRecycleThreshold = 512,
    kAvailableDequeOffset = 0xC,
    kQueuedDequeOffset = 0x24,
    kReturnedDequeOffset = 0x3C
};

// Pool membership protocol (one pool reference follows each sample):
// available --CommitSample--> queued --DequeueSample--> returned
// returned --RecycleReturnedSample--> available; RecycleAll drains queued then returned.
// Peek/Dequeue additionally retain for the caller; queue transfers do not retain.
// Drain loops below intentionally keep separate queue cursors. Within each:
// read front -> release sample ref -> advance -> reclaim old pages -> reset
// empty origin. Transfer routines instead append the same pointer before
// consuming it; Dequeue consumes first after acquiring the caller reference.
// Four-word iterator scratch is opaque: external iterator routines initialize it.
struct SamplePageRingIterator {
    unsigned int opaqueWords[4];
};
typedef char SamplePageRingIterator_size[(sizeof(SamplePageRingIterator) == 0x10) ? 1 : -1];
typedef char SampleDequeScratch_size[(sizeof(SamplePageStorageView) == 0x18) ? 1 : -1];

// Each deque owns a circular table of page pointers and its 256-word pages.
// position is the logical element origin relative to head, not a table index:
// page = (head + (position >> 8)) % capacity, slot = position & 255.
// rowCount includes a trailing sentinel entry; only rowCount-1 pages contribute
// back capacity. Pop retains front slack until position >= 512, then drops a page
// and subtracts 256. An empty deque resets position to 128 (half-page slack).
// Sample references belong to the pool queues independently of these raw pages.

extern "C" {

extern unsigned int g_gtlMediaRegistry;
extern unsigned int g_gtlRuntimeConfig[3];
void gtl_Free(void* p);
void* gtl_MediaRegistry_UpdateAll(unsigned int);
int gtl_MediaRegistry_GetInstance(void* p);
void* gtl_Alloc(unsigned int size);
void SampleDeque_EnsureBackBlocks(void* p, int requestedElementCount);
void* SampleDeque_dtor(void* p, int flag);
void __dl__FPv(void* p);
void GtlIntStorage_SetValue(void* p, int value);
int gtl_FilterManagerFactory_Create(void* p, unsigned int* out, int flag);
void gtl_MediaRegistry_Add(unsigned int p, unsigned int key);
void* GtlSampleDeque_GetBackPageSlot(void* p);
void* PageRing_GetFrontSlot(void* p);
void* PageRing_PopFront(void* p);
void GtlWordDeque_ForwardPushBack(void* p, void* value);
unsigned int GtlWordStorage_ReadFirstWord(void* p);
unsigned int SampleDeque_GetUsableElementCapacity(void* p);
void* PointerRing_GetUsableCapacity(void* p);
void* PointerRing_GetSize(void* p);
void* SampleDeque_AllocatePointerSlots(void* p, unsigned int n);
void* SampleDeque_InitializeEmptyStorage(void* dst, void* src);
void* GtlWordDeque_Reserve(void* p, unsigned int n);
int PointerRing_IsEmpty(void* p);
void* PageRing_GetEndIterator(void* dst, void* src);
void* RingBufferIterator_Subtract(void* dst, void* src, int flag);
void* PageRing_GetBeginIterator(void* dst, void* src);
void GtlSampleDeque_SwapPageRingState(void* a, void* b);
void PointerRing_InsertRangeWithIteratorCopies(void* p, void* a, void* b, void* c, int flag);
void PointerRing_ClearSize(void* p);
void RingBuffer_ThrowLengthError(const char*);
const unsigned int* UnsignedWord_SelectMaxPointer(const unsigned int*, const unsigned int*);
// Return the first available sample with an extra reference, leaving it in the queue.
int SamplePool_PeekAvailableSample(void* p, void* out);

// Destroy the page deque and optionally delete its wrapper.
void* SampleDeque_DestroyWrapper(void* p, int freeFlag);

// Create a filter-manager handle, register it globally, and return its registry key.
int gtl_Media_CreateFilterManager(unsigned int* out, void* source);

// Update all media instances if the registry exists.
void* gtl_Media_Update();

// Write the sample size and sample count when an output descriptor is supplied.
int SamplePool_GetDimensions(void* p, void* out);

// Copy runtime media configuration and acquire the global registry.
int gtl_Media_Init(const unsigned int* p);

// Clear the global media-registry pointer without releasing the registry.
void MediaRegistry_ResetGlobalPointer();

// Release the supplied media allocation.
void dtor_803EFD58(void* p);

// Report whether the available-sample queue is empty.
int SamplePool_IsEmpty(void* p);

// Expose the deque page-pointer ring beginning at its rows field.
void* SampleDeque_GetPageRing(void* p);
// Return the address of the deque head-page index.
void* Container_GetWord0CAddress(void* p);
// Expose the deque page-pointer ring for storage-management operations.
void* SampleDeque_GetPageRingStorage(void* p);

struct PoolSampleReference {
    unsigned int refs;
    virtual void dispose(int);
};
#pragma push
#pragma always_inline on
// Drop the queue reference and delete its sample wrapper only at zero.
inline void ReleasePoolQueueReference(PoolSampleReference* entry) {
    if (entry != 0) {
        unsigned int refs = entry->refs - 1;
        entry->refs = refs;
        if (refs == 0 && entry != 0)
            entry->dispose(1);
    }
}
#pragma pop

struct PoolQueueDrainView {
    unsigned int pos, count;
    unsigned int** rows;
    unsigned int head, rowCount, capacity;
};
#pragma push
#pragma always_inline on
// Consume one released front sample, reclaim old pages, and reset the empty origin.
inline void ConsumeDrainedPoolFront(PoolQueueDrainView* q) {
    --q->count;
    ++q->pos;
    while (q->pos >= kPageRecycleThreshold) {
        gtl_Free(q->rows[q->head]);
        ++q->head;
        if (q->head == q->capacity)
            q->head = 0;
        --q->rowCount;
        q->pos -= kSlotsPerPage;
    }
    if (q->count == 0)
        q->pos = kInitialPagePosition;
}
#pragma pop

// Release queued sample references, raw sample storage and all three page deques.
void* SamplePool_Dtor(void* p, int freeFlag);

// Add ceil(requestedElementCount/256) back pages, recycling whole consumed front pages first.
// requestedElementCount is a nonnegative element quantity, not a flag or target size.
// May move position and the pointer table: callers must reload both after this call.
void SampleDeque_EnsureBackBlocks(void* p, int requestedElementCount);

// Destroy the deque wrapper and optionally delete it. In plain C: if the first word holds an
// allocation, zero the +8 word and free it. The repeated 'p != 0' tests and 'x -= x' (== x = 0)
// are inlined-destructor leftovers kept so the compiler emits the original code; do not simplify.
void* dtor_803EFD68(void* p, int freeFlag);

// Move every queued and returned sample back to the available queue without changing sample references.
// Each deque is a ring of 256-entry pages (slot = position & 255, page = (head + position/256) % capacity);
// the two while-loops below are identical (queued, then returned): pop front, push back on 'available'
// (growing it if full), and free front pages once position reaches 512.
void SamplePool_RecycleAllSamples(void* p);

// Destroy the deque wrapper and optionally delete it.
void* dtor_803EFDE4(void* p, int freeFlag);

// Free each allocated page in the circular pointer table, then release the table and optional wrapper.
void* SampleDeque_dtor(void* p, int freeFlag);

// Allocate the sample buffer and sample wrappers and enqueue each new wrapper as available.
void* SamplePool_Ctor(void* self, void* dimensions, void* sampleContext);

// Move the oldest queued sample to the returned queue and return it with an extra reference.
int SamplePool_DequeueSample(void* self, unsigned int* out);

// Move one returned sample to the available queue, growing its page ring if required.
int SamplePool_RecycleReturnedSample(void* self);

// Fill an available sample through the media-sample API and move it to the queued ring.
int SamplePool_CommitSample(void* self, void* bufferArgument);

int gtl_Media_Init(const unsigned int* p) {
    struct Config {
        unsigned int a, b, c;
    };
    *(Config*)g_gtlRuntimeConfig = *(const Config*)p;
    g_gtlMediaRegistry = (unsigned int)(unsigned long)gtl_MediaRegistry_GetInstance((void*)p);
    return 1;
}

int gtl_Media_CreateFilterManager(unsigned int* out, void* source) {
    unsigned int handle;
    GtlIntStorage_SetValue(&handle, (int)source);
    unsigned int key = 0;
    if (!gtl_FilterManagerFactory_Create(&handle, &key, 0)) {
        *out = 0;
        return 0;
    }
    gtl_MediaRegistry_Add(g_gtlMediaRegistry, key);
    *out = key;
    return 1;
}

void* gtl_Media_Update() {
    unsigned int p = g_gtlMediaRegistry;
    if (p != 0)
        return gtl_MediaRegistry_UpdateAll(p);
    return 0;
}

void MediaRegistry_ResetGlobalPointer() {
    g_gtlMediaRegistry = 0;
}

void* SamplePool_Ctor(void* self, void* dimensions, void* sampleContext) {
    // dimensions borrows two words (buffer bytes per sample, number of samples).
    // sampleContext is forwarded to each wrapper. The pool owns one raw allocation
    // partitioned into fixed-size slices and one queue reference per sample.
    // Phase 1: establish dimensions, empty deques and their unwind guards.
    unsigned int sampleSize = *(unsigned int*)dimensions;
    unsigned int sampleCount = *(unsigned int*)((char*)dimensions + 4);
    AsGtlSamplePool(self)->sampleSize = sampleSize;
    AsGtlSamplePool(self)->sampleCount = sampleCount;
    for (int i = 0x0C; i <= 0x50; i += 4)
        *(unsigned int*)((char*)self + i) = 0;
    EH5Cleanup eh5Cleanup28(((char*)&AsGtlSamplePool(self)->available),
                            (void (*)(void*, int))SampleDeque_DestroyWrapper);
    EH5Cleanup eh5Cleanup27(((char*)&AsGtlSamplePool(self)->queued), (void (*)(void*, int))SampleDeque_DestroyWrapper);
    EH5Cleanup eh5Cleanup26(((char*)&AsGtlSamplePool(self)->returned),
                            (void (*)(void*, int))SampleDeque_DestroyWrapper);
    // Phase 2: allocate backing bytes and publish each wrapper in available.
    char* base = (char*)gtl_Alloc(sampleSize * sampleCount);
    AsGtlSamplePool(self)->storage = base;
    unsigned int sampleIndex = 0;
    if (sampleCount != 0) {
        do {
            char* sampleBuffer = base + sampleSize * sampleIndex;
            void* sample = gtl_Alloc(0x14);
            if (sample != 0) {
                ((gtl::CMediaSample*)sample)->Init((int)sampleBuffer, (int)sampleSize, (int)sampleContext);
            }
            unsigned int availablePages = AsGtlSamplePool(self)->available.rowCount;
            unsigned int backCapacity = (availablePages == 0) ? 0 : ((availablePages - 1) << kPageSlotBits);
            unsigned int availablePosition = *(unsigned int*)(((char*)&AsGtlSamplePool(self)->available));
            unsigned int availableCount = AsGtlSamplePool(self)->available.count;
            unsigned int backPosition = availablePosition + availableCount;
            if (backCapacity - backPosition == 0) {
                SampleDeque_EnsureBackBlocks(((char*)&AsGtlSamplePool(self)->available), 1);
            }
            // the original reloads +0x0C after the grow call (lwz r3, 0xc(r28) at 0x803EFC88); the grow
            // can move the start (SampleDeque_EnsureBackBlocks subtracts kSlotsPerPage from it), so the cached value is stale
            availablePosition = *(unsigned int*)(((char*)&AsGtlSamplePool(self)->available));
            unsigned int pageCapacity = AsGtlSamplePool(self)->available.capacity;
            unsigned int pageTableAddress = (*(unsigned int*)&AsGtlSamplePool(self)->available.rows);
            unsigned int pageTableEnd = pageTableAddress + (pageCapacity << 2);
            unsigned int availableCountAfterGrowth = AsGtlSamplePool(self)->available.count;
            unsigned int headPage = AsGtlSamplePool(self)->available.head;
            unsigned int headPageAddress = pageTableAddress + (headPage << 2);
            int backPageIndex =
                (int)(((availableCountAfterGrowth + availablePosition) >> kPageSlotBits)) + (int)headPage;
            if (backPageIndex < 0)
                backPageIndex += (int)pageCapacity;
            else if (backPageIndex >= (int)pageCapacity)
                backPageIndex -= (int)pageCapacity;
            unsigned int pageCountAfterGrowth = AsGtlSamplePool(self)->available.rowCount;
            unsigned int* pageSlotAddress = (unsigned int*)(pageTableAddress + (backPageIndex << 2));
            unsigned int* backPage = 0;
            if (pageCountAfterGrowth != 0)
                backPage = *(unsigned int**)pageSlotAddress;
            unsigned int slotByteOffset = (((availableCountAfterGrowth + availablePosition) & kPageSlotMask) << 2);
            unsigned int* sampleSlot = (unsigned int*)((char*)backPage + slotByteOffset);
            if (sampleSlot != 0) {
                *sampleSlot = (unsigned int)sample;
            }
            unsigned int finalAvailableCount = AsGtlSamplePool(self)->available.count;
            finalAvailableCount += 1;
            AsGtlSamplePool(self)->available.count = finalAvailableCount;
            sampleIndex += 1;
        } while (sampleIndex < sampleCount);
    }
    // Phase 3: completed pool assumes normal-path ownership of all three deques.
    eh5Cleanup26.release();
    eh5Cleanup27.release();
    eh5Cleanup28.release();
    return self;
}

void dtor_803EFD58(void* p) {
    if (p != 0)
        gtl_Free(p);
}

#pragma push
#pragma opt_propagation off
void* dtor_803EFD68(void* p, int freeFlag) {
    if (p != 0) {
        if (p != 0 && p != 0 && p != 0) {
            if (*(void**)p != 0) {
                int* value = (int*)&((SamplePageStorageView*)p)->rows;
                int count = *value;
                *value -= count;
                gtl_Free(*(void**)p);
            }
        }
        if (freeFlag > 0)
            __dl__FPv(p);
    }
    return p;
}
#pragma pop

#pragma push
#pragma opt_propagation off
void* dtor_803EFDE4(void* p, int freeFlag) {
    if (p != 0) {
        if (p != 0 && p != 0 && p != 0 && p != 0 && p != 0) {
            if (*(void**)p != 0) {
                int* value = (int*)&((SamplePageStorageView*)p)->rows;
                int count = *value;
                *value -= count;
                gtl_Free(*(void**)p);
            }
        }
        if (freeFlag > 0)
            __dl__FPv(p);
    }
    return p;
}
#pragma pop

void* SampleDeque_DestroyWrapper(void* p, int freeFlag) {
    if (p != 0) {
        if (p != 0 && p != 0 && p != 0 && p != 0 && p != 0)
            SampleDeque_dtor(p, 0);
        if (freeFlag > 0)
            __dl__FPv(p);
    }
    return p;
}

void* SamplePool_Dtor(void* p, int freeFlag) {
    // Sample buffer slices cannot outlive this pool's backing allocation, even
    // if an external reference keeps a sample wrapper alive after queue release.
    // Phase 1: release one reference for every sample in each of the three queues.
    // A sample with no remaining external references disposes its own wrapper.
    if (p != 0) {
        char* self = (char*)p;
        {
            // Release available: unused samples.
            PoolQueueDrainView* q = (PoolQueueDrainView*)(&AsGtlSamplePool(self)->available);
            if (q->count != 0) {
                while (q->count != 0) {
                    unsigned int value = q->pos;
                    unsigned int row = (q->head + (value >> kPageSlotBits)) % q->capacity;
                    PoolSampleReference* entry = (PoolSampleReference*)q->rows[row][value & kPageSlotMask];
                    ReleasePoolQueueReference(entry);
                    ConsumeDrainedPoolFront(q);
                }
            }
        }
        {
            // Release queued: pending delivery.
            PoolQueueDrainView* q = (PoolQueueDrainView*)(&AsGtlSamplePool(self)->queued);
            if (q->count != 0) {
                while (q->count != 0) {
                    unsigned int value = q->pos;
                    unsigned int row = (q->head + (value >> kPageSlotBits)) % q->capacity;
                    PoolSampleReference* entry = (PoolSampleReference*)q->rows[row][value & kPageSlotMask];
                    ReleasePoolQueueReference(entry);
                    ConsumeDrainedPoolFront(q);
                }
            }
        }
        {
            // Release returned: delivered samples awaiting reuse.
            PoolQueueDrainView* q = (PoolQueueDrainView*)(&AsGtlSamplePool(self)->returned);
            if (q->count != 0) {
                while (q->count != 0) {
                    unsigned int value = q->pos;
                    unsigned int row = (q->head + (value >> kPageSlotBits)) % q->capacity;
                    PoolSampleReference* entry = (PoolSampleReference*)q->rows[row][value & kPageSlotMask];
                    ReleasePoolQueueReference(entry);
                    ConsumeDrainedPoolFront(q);
                }
            }
        }
        // Phase 2: free the pool's shared raw buffer after dropping queue refs;
        // page deques own pointer storage separately and are destroyed last.
        if (*(void**)self != 0) {
            gtl_Free(*(void**)self);
            *(void**)self = 0;
        }
        if (self + kReturnedDequeOffset != 0 && self + kReturnedDequeOffset != 0 && self + kReturnedDequeOffset != 0 &&
            self + kReturnedDequeOffset != 0 && self + kReturnedDequeOffset != 0 && self + kReturnedDequeOffset != 0)
            SampleDeque_dtor(self + kReturnedDequeOffset, 0);
        if (self + kQueuedDequeOffset != 0 && self + kQueuedDequeOffset != 0 && self + kQueuedDequeOffset != 0 &&
            self + kQueuedDequeOffset != 0 && self + kQueuedDequeOffset != 0 && self + kQueuedDequeOffset != 0)
            SampleDeque_dtor(self + kQueuedDequeOffset, 0);
        if (self + kAvailableDequeOffset != 0 && self + kAvailableDequeOffset != 0 &&
            self + kAvailableDequeOffset != 0 && self + kAvailableDequeOffset != 0 &&
            self + kAvailableDequeOffset != 0 && self + kAvailableDequeOffset != 0)
            SampleDeque_dtor(self + kAvailableDequeOffset, 0);
        if (freeFlag > 0 && self != 0)
            gtl_Free(p);
    }
    return p;
}

int SamplePool_IsEmpty(void* p) {
    return AsGtlSamplePool(p)->available.count == 0;
}

int SamplePool_DequeueSample(void* self, unsigned int* out) {
    // out must be writable when queued is nonempty. Return 0 leaves it untouched;
    // return 1 gives the caller one additional reference to release. The queue's
    // existing reference moves queued -> returned and remains owned by the pool.
    // Phase 1: locate the oldest pointer by page/slot and acquire the caller's ref.
    if (AsGtlSamplePool(self)->queued.count == 0)
        return 0;
    unsigned int queuedPosition = AsGtlSamplePool(self)->queued.position;
    unsigned int queuedHeadPage = AsGtlSamplePool(self)->queued.head;
    unsigned int pageOffset = queuedPosition >> kPageSlotBits;
    unsigned int queuedPageCapacity = AsGtlSamplePool(self)->queued.capacity;
    unsigned int frontPageIndex = (queuedHeadPage + pageOffset) % queuedPageCapacity;
    unsigned int frontSlotByteOffset = (queuedPosition & kPageSlotMask) << 2;
    unsigned int** queuedPages = AsGtlSamplePool(self)->queued.rows;
    unsigned int* frontPage = queuedPages[frontPageIndex];
    unsigned int* sample = *(unsigned int**)((char*)frontPage + frontSlotByteOffset);
    *out = (unsigned int)sample;
    unsigned int references = *sample;
    *sample = references + 1;
    unsigned int queuedCount = AsGtlSamplePool(self)->queued.count;
    // Phase 2: advance the queued origin, freeing only pages fully behind it.
    unsigned int nextQueuedPosition = AsGtlSamplePool(self)->queued.position;
    queuedCount -= 1;
    nextQueuedPosition += 1;
    AsGtlSamplePool(self)->queued.count = queuedCount;
    AsGtlSamplePool(self)->queued.position = nextQueuedPosition;
    while (AsGtlSamplePool(self)->queued.position >= kPageRecycleThreshold) {
        unsigned int consumedPageIndex = AsGtlSamplePool(self)->queued.head;
        unsigned int** queuedPagesAfterPop = AsGtlSamplePool(self)->queued.rows;
        unsigned int* consumedPage = queuedPagesAfterPop[consumedPageIndex];
        gtl_Free(consumedPage);
        unsigned int nextHeadPage = AsGtlSamplePool(self)->queued.head;
        nextHeadPage += 1;
        unsigned int pageCapacityAfterFree = AsGtlSamplePool(self)->queued.capacity;
        if (nextHeadPage == pageCapacityAfterFree)
            nextHeadPage = 0;
        AsGtlSamplePool(self)->queued.head = nextHeadPage;
        unsigned int queuedPageCount = AsGtlSamplePool(self)->queued.rowCount;
        unsigned int positionAfterFree = AsGtlSamplePool(self)->queued.position;
        queuedPageCount -= 1;
        positionAfterFree -= kSlotsPerPage;
        AsGtlSamplePool(self)->queued.rowCount = queuedPageCount;
        AsGtlSamplePool(self)->queued.position = positionAfterFree;
    }
    if (AsGtlSamplePool(self)->queued.count == 0) {
        AsGtlSamplePool(self)->queued.position = kInitialPagePosition;
    }
    // Phase 3: reserve a returned slot, reload any moved origin/table, then append.
    // This operation is not transactional if page allocation throws after removal.
    unsigned int returnedPageCount = AsGtlSamplePool(self)->returned.rowCount;
    unsigned int returnedBackCapacity = (returnedPageCount == 0) ? 0 : ((returnedPageCount - 1) << kPageSlotBits);
    unsigned int returnedPosition = AsGtlSamplePool(self)->returned.position;
    unsigned int returnedCount = AsGtlSamplePool(self)->returned.count;
    unsigned int returnedBackPosition = returnedPosition + returnedCount;
    if (returnedBackCapacity - returnedBackPosition == 0) {
        SampleDeque_EnsureBackBlocks(((char*)&AsGtlSamplePool(self)->returned), 1);
    }
    // the original reloads +0x3C after the grow call (lwz r3, 0x3c(r30) at 0x803F0454); the grow
    // can move the start (SampleDeque_EnsureBackBlocks subtracts kSlotsPerPage from it), so the cached value is stale
    returnedPosition = AsGtlSamplePool(self)->returned.position;
    unsigned int returnedPageCapacity = AsGtlSamplePool(self)->returned.capacity;
    unsigned int** returnedPageTable = AsGtlSamplePool(self)->returned.rows;
    unsigned int returnedCountAfterGrowth = AsGtlSamplePool(self)->returned.count;
    unsigned int returnedHeadPage = AsGtlSamplePool(self)->returned.head;
    int returnedBackPageIndex =
        (int)(((returnedCountAfterGrowth + returnedPosition) >> kPageSlotBits)) + (int)returnedHeadPage;
    if (returnedBackPageIndex < 0)
        returnedBackPageIndex += (int)returnedPageCapacity;
    else if (returnedBackPageIndex >= (int)returnedPageCapacity)
        returnedBackPageIndex -= (int)returnedPageCapacity;
    unsigned int returnedPagesAfterGrowth = AsGtlSamplePool(self)->returned.rowCount;
    unsigned int* returnedPageSlotAddress = (unsigned int*)(returnedPageTable + returnedBackPageIndex);
    unsigned int* returnedBackPage = 0;
    if (returnedPagesAfterGrowth != 0)
        returnedBackPage = *(unsigned int**)returnedPageSlotAddress;
    unsigned int returnedSlotByteOffset = (((returnedCountAfterGrowth + returnedPosition) & kPageSlotMask) << 2);
    unsigned int* returnedSampleSlot = (unsigned int*)((char*)returnedBackPage + returnedSlotByteOffset);
    if (returnedSampleSlot != 0) {
        *returnedSampleSlot = (unsigned int)sample;
    }
    unsigned int finalReturnedCount = AsGtlSamplePool(self)->returned.count;
    AsGtlSamplePool(self)->returned.count = finalReturnedCount + 1;
    return 1;
}

void* SampleDeque_GetPageRing(void* p) {
    return &AsGtlSampleDeque(p)->rows;
}

void* Container_GetWord0CAddress(void* p) {
    return &AsGtlSampleDeque(p)->head;
}

int SamplePool_RecycleReturnedSample(void* self) {
    // Requires a nonempty returned queue. The sample's pool reference moves to
    // available; this routine neither acquires nor releases a sample reference.
    // Phase 1: restore the sample buffer descriptor before publishing it as available.
    unsigned int sampleData = 0;
    unsigned int returnedPosition = AsGtlSamplePool(self)->returned.position;
    unsigned int returnedHeadPage = AsGtlSamplePool(self)->returned.head;
    unsigned int pageOffset = returnedPosition >> kPageSlotBits;
    unsigned int returnedPageCapacity = AsGtlSamplePool(self)->returned.capacity;
    unsigned int frontPageIndexBeforeWrap = returnedHeadPage + pageOffset;
    unsigned int** returnedPageTable = AsGtlSamplePool(self)->returned.rows;
    unsigned int pageWrapCount = frontPageIndexBeforeWrap / returnedPageCapacity;
    unsigned int frontPageIndex = frontPageIndexBeforeWrap - pageWrapCount * returnedPageCapacity;
    unsigned int* frontPage = ((unsigned int**)returnedPageTable)[frontPageIndex];
    unsigned int sample = frontPage[returnedPosition & kPageSlotMask];
    unsigned int sampleToRecycle = sample;
    sampleData = 0;
    ((gtl::CMediaSample*)sample)->GetData((int*)&sampleData);
    ((gtl::CMediaSample*)sample)->SetBuffer((int)sampleData, (*(int*)&AsGtlSamplePool(self)->sampleSize));
    unsigned int availablePageCount = AsGtlSamplePool(self)->available.rowCount;
    unsigned int availableBackCapacity = (availablePageCount == 0) ? 0 : ((availablePageCount - 1) << kPageSlotBits);
    unsigned int availablePosition = AsGtlSamplePool(self)->available.position;
    unsigned int availableCount = AsGtlSamplePool(self)->available.count;
    unsigned int availableBackPosition = availablePosition + availableCount;
    if (availableBackCapacity == availableBackPosition) {
        SampleDeque_EnsureBackBlocks(((char*)&AsGtlSamplePool(self)->available), 1);
    }
    unsigned int availablePageCapacity = AsGtlSamplePool(self)->available.capacity;
    unsigned int** availablePageTable = AsGtlSamplePool(self)->available.rows;
    unsigned int availableHeadPage = AsGtlSamplePool(self)->available.head;
    // the original reloads +0x10 and +0x0C after the grow call (lwz r5, 0x10(r30) / lwz r3, 0xc(r30) at
    // 0x803F05F4 / 0x803F05FC); the grow can move the start, so the values cached before it are stale
    unsigned int availableBackPositionAfterGrowth =
        AsGtlSamplePool(self)->available.count + AsGtlSamplePool(self)->available.position;
    unsigned int pageCapacity = availablePageCapacity;
    unsigned int headPage = availableHeadPage;
    int backPageIndex = (int)(availableBackPositionAfterGrowth >> kPageSlotBits) + (int)headPage;
    if (backPageIndex < 0)
        backPageIndex += (int)pageCapacity;
    else if (backPageIndex >= (int)pageCapacity)
        backPageIndex -= (int)pageCapacity;
    unsigned int availablePagesAfterGrowth = AsGtlSamplePool(self)->available.rowCount;
    unsigned int* backPageSlotAddress = (unsigned int*)(availablePageTable + backPageIndex);
    unsigned int* backPage = 0;
    if (availablePagesAfterGrowth != 0)
        backPage = *(unsigned int**)backPageSlotAddress;
    unsigned int backSlotByteOffset = ((availableBackPositionAfterGrowth & kPageSlotMask) << 2);
    unsigned int* sampleSlot = (unsigned int*)((char*)backPage + backSlotByteOffset);
    if (sampleSlot != 0) {
        *sampleSlot = sampleToRecycle;
    }
    // Phase 2: commit the available append, then consume/reclaim the returned front.
    unsigned int finalAvailableCount = AsGtlSamplePool(self)->available.count;
    unsigned int returnedCount = AsGtlSamplePool(self)->returned.count;
    unsigned int nextReturnedPosition = AsGtlSamplePool(self)->returned.position;
    finalAvailableCount += 1;
    AsGtlSamplePool(self)->available.count = finalAvailableCount;
    returnedCount -= 1;
    AsGtlSamplePool(self)->returned.count = returnedCount;
    nextReturnedPosition += 1;
    AsGtlSamplePool(self)->returned.position = nextReturnedPosition;
    while (AsGtlSamplePool(self)->returned.position >= kPageRecycleThreshold) {
        unsigned int consumedPageIndex = AsGtlSamplePool(self)->returned.head;
        unsigned int** returnedPagesAfterPop = AsGtlSamplePool(self)->returned.rows;
        unsigned int* consumedPage = returnedPagesAfterPop[consumedPageIndex];
        gtl_Free(consumedPage);
        unsigned int nextHeadPage = AsGtlSamplePool(self)->returned.head;
        nextHeadPage += 1;
        unsigned int capacityAfterFree = AsGtlSamplePool(self)->returned.capacity;
        if (nextHeadPage == capacityAfterFree)
            nextHeadPage = 0;
        AsGtlSamplePool(self)->returned.head = nextHeadPage;
        unsigned int returnedPagesAfterFree = AsGtlSamplePool(self)->returned.rowCount;
        returnedPagesAfterFree -= 1;
        AsGtlSamplePool(self)->returned.rowCount = returnedPagesAfterFree;
        unsigned int positionAfterFree = AsGtlSamplePool(self)->returned.position;
        positionAfterFree -= kSlotsPerPage;
        AsGtlSamplePool(self)->returned.position = positionAfterFree;
    }
    unsigned int finalReturnedCount = AsGtlSamplePool(self)->returned.count;
    if (finalReturnedCount == 0) {
        AsGtlSamplePool(self)->returned.position = kInitialPagePosition;
    }
    return 1;
}

#pragma push
#pragma opt_propagation off
int SamplePool_GetDimensions(void* p, void* out) {
    if (out == 0)
        return 0;
    unsigned int sampleCount;
    unsigned int sampleSize;
    sampleSize = AsGtlSamplePool(p)->sampleSize;
    sampleCount = AsGtlSamplePool(p)->sampleCount;
    *(unsigned int*)out = sampleSize;
    *((unsigned int*)out + 1) = sampleCount;
    return 1;
}
#pragma pop

void SamplePool_RecycleAllSamples(void* p) {
    // Drain queued first, then returned. Only page-pointer storage moves/frees;
    // each sample's existing pool reference follows it back to available.
    unsigned char* self = (unsigned char*)p;
    while (AsGtlSamplePool(self)->queued.count != 0) {
        unsigned int value = AsGtlSamplePool(self)->queued.position;
        unsigned int index =
            (AsGtlSamplePool(self)->queued.head + (value >> kPageSlotBits)) % AsGtlSamplePool(self)->queued.capacity;
        unsigned int* rows = (*(unsigned int**)&AsGtlSamplePool(self)->queued.rows);
        unsigned int* item = (unsigned int*)rows[index];
        unsigned int entry = item[value & kPageSlotMask];
        unsigned int limit = AsGtlSamplePool(self)->available.rowCount;
        unsigned int total = AsGtlSamplePool(self)->available.position + AsGtlSamplePool(self)->available.count;
        if (limit != 0)
            limit = (limit - 1) << kPageSlotBits;
        if (limit == total)
            SampleDeque_EnsureBackBlocks(self + kAvailableDequeOffset, 1);
        // the original adds the two fields again after the grow call (it may have moved the start)
        total = AsGtlSamplePool(self)->available.count + AsGtlSamplePool(self)->available.position;
        unsigned int* first = (*(unsigned int**)&AsGtlSamplePool(self)->available.rows);
        unsigned int* end = first + AsGtlSamplePool(self)->available.capacity;
        unsigned int* start = first + AsGtlSamplePool(self)->available.head;
        unsigned int count = end - first;
        unsigned int pos = start - first;
        int wrapped = (int)(total >> kPageSlotBits) + (int)pos;
        if (wrapped < 0)
            wrapped += (int)count;
        else if (wrapped >= (int)count)
            wrapped -= (int)count;
        unsigned int* slot = first + wrapped;
        unsigned int* base = AsGtlSamplePool(self)->available.rowCount ? (unsigned int*)*slot : 0;
        unsigned int* dst = (unsigned int*)((unsigned char*)base + ((total & kPageSlotMask) << 2));
        if (dst)
            *dst = entry;
        ++AsGtlSamplePool(self)->available.count;
        --AsGtlSamplePool(self)->queued.count;
        ++AsGtlSamplePool(self)->queued.position;
        while (AsGtlSamplePool(self)->queued.position >= kPageRecycleThreshold) {
            gtl_Free((void*)*(unsigned int*)((*(unsigned int**)&AsGtlSamplePool(self)->queued.rows) +
                                             AsGtlSamplePool(self)->queued.head));
            unsigned int i = AsGtlSamplePool(self)->queued.head + 1;
            if (i == AsGtlSamplePool(self)->queued.capacity)
                i = 0;
            AsGtlSamplePool(self)->queued.head = i;
            --AsGtlSamplePool(self)->queued.rowCount;
            AsGtlSamplePool(self)->queued.position -= kSlotsPerPage;
        }
        if (AsGtlSamplePool(self)->queued.count == 0)
            AsGtlSamplePool(self)->queued.position = kInitialPagePosition;
    }
    while (AsGtlSamplePool(self)->returned.count != 0) {
        unsigned int value = AsGtlSamplePool(self)->returned.position;
        unsigned int index = (AsGtlSamplePool(self)->returned.head + (value >> kPageSlotBits)) %
                             AsGtlSamplePool(self)->returned.capacity;
        unsigned int* rows = (*(unsigned int**)&AsGtlSamplePool(self)->returned.rows);
        unsigned int* item = (unsigned int*)rows[index];
        unsigned int entry = item[value & kPageSlotMask];
        unsigned int limit = AsGtlSamplePool(self)->available.rowCount;
        unsigned int total = AsGtlSamplePool(self)->available.position + AsGtlSamplePool(self)->available.count;
        if (limit != 0)
            limit = (limit - 1) << kPageSlotBits;
        if (limit == total)
            SampleDeque_EnsureBackBlocks(self + kAvailableDequeOffset, 1);
        // the original adds the two fields again after the grow call (it may have moved the start)
        total = AsGtlSamplePool(self)->available.count + AsGtlSamplePool(self)->available.position;
        unsigned int* first = (*(unsigned int**)&AsGtlSamplePool(self)->available.rows);
        unsigned int* end = first + AsGtlSamplePool(self)->available.capacity;
        unsigned int* start = first + AsGtlSamplePool(self)->available.head;
        unsigned int count = end - first;
        unsigned int pos = start - first;
        int wrapped = (int)(total >> kPageSlotBits) + (int)pos;
        if (wrapped < 0)
            wrapped += (int)count;
        else if (wrapped >= (int)count)
            wrapped -= (int)count;
        unsigned int* slot = first + wrapped;
        unsigned int* base = AsGtlSamplePool(self)->available.rowCount ? (unsigned int*)*slot : 0;
        unsigned int* dst = (unsigned int*)((unsigned char*)base + ((total & kPageSlotMask) << 2));
        if (dst)
            *dst = entry;
        ++AsGtlSamplePool(self)->available.count;
        --AsGtlSamplePool(self)->returned.count;
        ++AsGtlSamplePool(self)->returned.position;
        while (AsGtlSamplePool(self)->returned.position >= kPageRecycleThreshold) {
            gtl_Free((void*)*(unsigned int*)((*(unsigned int**)&AsGtlSamplePool(self)->returned.rows) +
                                             AsGtlSamplePool(self)->returned.head));
            unsigned int i = AsGtlSamplePool(self)->returned.head + 1;
            if (i == AsGtlSamplePool(self)->returned.capacity)
                i = 0;
            AsGtlSamplePool(self)->returned.head = i;
            --AsGtlSamplePool(self)->returned.rowCount;
            AsGtlSamplePool(self)->returned.position -= kSlotsPerPage;
        }
        if (AsGtlSamplePool(self)->returned.count == 0)
            AsGtlSamplePool(self)->returned.position = kInitialPagePosition;
    }
}

int SamplePool_PeekAvailableSample(void* p, void* out) {
    if (out == 0 || AsGtlSamplePool(p)->available.count == 0)
        return 0;
    unsigned int value = AsGtlSamplePool(p)->available.position;
    unsigned int index =
        (AsGtlSamplePool(p)->available.head + (value >> kPageSlotBits)) % AsGtlSamplePool(p)->available.capacity;
    unsigned int* row = (*(unsigned int**)&AsGtlSamplePool(p)->available.rows);
    unsigned int* item = (unsigned int*)row[index];
    unsigned int result = item[(value & kPageSlotMask)];
    *(unsigned int*)out = result;
    ++*(unsigned int*)result;
    return 1;
}

int SamplePool_CommitSample(void* self, void* bufferArgument) {
    // Requires an available sample. bufferArgument is forwarded as the third
    // SetBuffer argument via an integer ABI slot; its exact meaning is unresolved.
    // Phase 1: update that sample's buffer, then reserve/append it to queued.
    unsigned int availablePosition = AsGtlSamplePool(self)->available.position;
    unsigned int availableHeadPage = AsGtlSamplePool(self)->available.head;
    unsigned int availablePageCapacity = AsGtlSamplePool(self)->available.capacity;
    unsigned int** availablePageTable = AsGtlSamplePool(self)->available.rows;
    unsigned int pageOffset = availablePosition >> kPageSlotBits;
    unsigned int frontPageOffset = availableHeadPage + pageOffset;
    unsigned int frontPageCapacity = availablePageCapacity;
    unsigned int frontPageIndexBeforeWrap = availableHeadPage + pageOffset;
    unsigned int pageWrapCount = frontPageIndexBeforeWrap / frontPageCapacity;
    unsigned int frontPageIndex = frontPageIndexBeforeWrap - pageWrapCount * frontPageCapacity;
    unsigned int** availablePages = (unsigned int**)((*(unsigned int*)&AsGtlSamplePool(self)->available.rows));
    unsigned int* frontPage = ((unsigned int**)availablePages)[frontPageIndex];
    unsigned int frontSlotByteOffset = (availablePosition & kPageSlotMask) << 2;
    unsigned int sample = frontPage[availablePosition & kPageSlotMask];
    unsigned int sampleToQueue = sample;
    unsigned int sampleData = 0;
    ((gtl::CMediaSample*)sample)->GetData((int*)&sampleData);
    ((gtl::CMediaSample*)sample)->SetBuffer((int)sampleData, (int)bufferArgument);
    unsigned int queuedPageCount = AsGtlSamplePool(self)->queued.rowCount;
    unsigned int queuedBackCapacity = (queuedPageCount == 0) ? 0 : ((queuedPageCount - 1) << kPageSlotBits);
    unsigned int queuedPosition = AsGtlSamplePool(self)->queued.position;
    unsigned int queuedCount = AsGtlSamplePool(self)->queued.count;
    unsigned int queuedBackPosition = queuedPosition + queuedCount;
    if (queuedBackCapacity == queuedBackPosition) {
        SampleDeque_EnsureBackBlocks(((char*)&AsGtlSamplePool(self)->queued), 1);
    }
    unsigned int queuedPageCapacity = AsGtlSamplePool(self)->queued.capacity;
    unsigned int** queuedPageTable = AsGtlSamplePool(self)->queued.rows;
    unsigned int queuedHeadPage = AsGtlSamplePool(self)->queued.head;
    unsigned int queuedCountAfterGrowth = AsGtlSamplePool(self)->queued.count;
    unsigned int queuedPositionAfterGrowth = AsGtlSamplePool(self)->queued.position;
    unsigned int queuedBackPositionAfterGrowth = queuedCountAfterGrowth + queuedPositionAfterGrowth;
    unsigned int pageCapacity = queuedPageCapacity;
    unsigned int headPage = queuedHeadPage;
    int backPageIndex = (int)(queuedBackPositionAfterGrowth >> kPageSlotBits) + (int)headPage;
    if (backPageIndex < 0)
        backPageIndex += (int)pageCapacity;
    else if (backPageIndex >= (int)pageCapacity)
        backPageIndex -= (int)pageCapacity;
    unsigned int queuedPagesAfterGrowth = AsGtlSamplePool(self)->queued.rowCount;
    unsigned int* backPageSlotAddress = (unsigned int*)(queuedPageTable + backPageIndex);
    unsigned int* backPage = 0;
    if (queuedPagesAfterGrowth != 0)
        backPage = *(unsigned int**)backPageSlotAddress;
    unsigned int backSlotByteOffset = ((queuedBackPositionAfterGrowth & kPageSlotMask) << 2);
    unsigned int* sampleSlot = (unsigned int*)((char*)backPage + backSlotByteOffset);
    if (sampleSlot != 0) {
        *sampleSlot = sampleToQueue;
    }
    // Phase 2: commit queued membership, then advance/reclaim available storage.
    unsigned int finalQueuedCount = AsGtlSamplePool(self)->queued.count;
    unsigned int availableCount = AsGtlSamplePool(self)->available.count;
    unsigned int nextAvailablePosition = AsGtlSamplePool(self)->available.position;
    finalQueuedCount += 1;
    AsGtlSamplePool(self)->queued.count = finalQueuedCount;
    availableCount -= 1;
    AsGtlSamplePool(self)->available.count = availableCount;
    nextAvailablePosition += 1;
    AsGtlSamplePool(self)->available.position = nextAvailablePosition;
    while (AsGtlSamplePool(self)->available.position >= kPageRecycleThreshold) {
        unsigned int consumedPageIndex = AsGtlSamplePool(self)->available.head;
        unsigned int** availablePagesAfterPop = AsGtlSamplePool(self)->available.rows;
        unsigned int* consumedPage = availablePagesAfterPop[consumedPageIndex];
        gtl_Free(consumedPage);
        unsigned int nextHeadPage = AsGtlSamplePool(self)->available.head;
        nextHeadPage += 1;
        unsigned int capacityAfterFree = AsGtlSamplePool(self)->available.capacity;
        if (nextHeadPage == capacityAfterFree)
            nextHeadPage = 0;
        AsGtlSamplePool(self)->available.head = nextHeadPage;
        unsigned int availablePagesAfterFree = AsGtlSamplePool(self)->available.rowCount;
        availablePagesAfterFree -= 1;
        AsGtlSamplePool(self)->available.rowCount = availablePagesAfterFree;
        unsigned int positionAfterFree = AsGtlSamplePool(self)->available.position;
        positionAfterFree -= kSlotsPerPage;
        AsGtlSamplePool(self)->available.position = positionAfterFree;
    }
    unsigned int finalAvailableCount = AsGtlSamplePool(self)->available.count;
    if (finalAvailableCount == 0) {
        AsGtlSamplePool(self)->available.position = kInitialPagePosition;
    }
    return 1;
}

void* SampleDeque_dtor(void* p, int freeFlag) {
    if (p != 0) {
        unsigned int cap = AsGtlSampleDeque(p)->capacity;
        unsigned int idx = AsGtlSampleDeque(p)->head;
        unsigned int cnt = AsGtlSampleDeque(p)->rowCount;
        void** base = ((SamplePageStorageView*)p)->rows;
        AsGtlSampleDeque(p)->count = 0;
        if (cnt != 0) {
            unsigned int start;
            if (cap != 0) {
                unsigned int sum = idx + cnt;
                start = sum % cap;
            } else {
                start = 0;
            }
            void** cur = base + idx;
            void** end = base + cap;
            void** target = base + start;
            while (cur != target) {
                if (*cur != 0)
                    gtl_Free(*cur);
                ++cur;
                if (cur == end)
                    cur = base;
            }
        }
        if ((char*)&((SamplePageStorageView*)p)->rows != 0 && (char*)&((SamplePageStorageView*)p)->rows != 0 &&
            (char*)&((SamplePageStorageView*)p)->rows != 0 && (char*)&((SamplePageStorageView*)p)->rows != 0 &&
            (char*)&((SamplePageStorageView*)p)->rows != 0 && (char*)&((SamplePageStorageView*)p)->rows != 0) {
            if (((SamplePageStorageView*)p)->rows != 0) {
                volatile int* value = (volatile int*)&((SamplePageStorageView*)p)->rowCount;
                *value = *value - *value;
                gtl_Free(((SamplePageStorageView*)p)->rows);
            }
        }
        if (freeFlag > 0)
            __dl__FPv(p);
    }
    return p;
}

void* SampleDeque_GetPageRingStorage(void* p) {
    return &AsGtlSampleDeque(p)->rows;
}

void SampleDeque_EnsureBackBlocks(void* p, int requestedElementCount) {
    // Phase 1: round to pages and move reusable front pages to the back.
    void* queue = SampleDeque_GetPageRing(p);
    void* queue2 = SampleDeque_GetPageRingStorage(p);
    unsigned int count = (unsigned int)requestedElementCount >> kPageSlotBits;
    if ((unsigned char)requestedElementCount != 0)
        ++count;
    while (count != 0 && GtlWordStorage_ReadFirstWord(p) >= kSlotsPerPage) {
        unsigned int x = *(unsigned int*)PageRing_GetFrontSlot(queue);
        PageRing_PopFront(queue);
        *(unsigned int*)GtlSampleDeque_GetBackPageSlot(queue) = x;
        unsigned int zero = 0;
        GtlWordDeque_ForwardPushBack(queue, &zero);
        --count;
        AsGtlSampleDeque(p)->position -= kSlotsPerPage;
    }
    if (count != 0) {
        // Phase 2: reject length overflow, then append into the existing pointer ring
        // if it has enough usable entries. No sample reference counts change.
        unsigned int need = count << kPageSlotBits;
        unsigned int capacity = GtlSampleDeque::GetMaxSize(p);
        if (need > capacity || SampleDeque_GetUsableElementCapacity(p) > capacity - need)
            RingBuffer_ThrowLengthError("deque:: length error");
        unsigned int size = (unsigned int)PointerRing_GetUsableCapacity(queue);
        unsigned int used = (unsigned int)PointerRing_GetSize(queue);
        if (count + used <= size) {
            for (; count != 0; --count) {
                void* block = SampleDeque_AllocatePointerSlots(queue2, kSlotsPerPage);
                *(void**)GtlSampleDeque_GetBackPageSlot(queue) = block;
                unsigned int zero = 0;
                GtlWordDeque_ForwardPushBack(queue, &zero);
            }
        } else {
            // Phase 3: build a replacement ring with a trailing sentinel and new
            // owned pages. temp is a six-word deque; the guard destroys it on unwind.
            SamplePageStorageView replacement; // Six-word deque initialized by the runtime.
            SampleDeque_InitializeEmptyStorage((&replacement), queue2);
            EH5Cleanup eh5Cleanup29((&replacement), (void (*)(void*, int))SampleDeque_dtor);
            void* grown = SampleDeque_GetPageRing((&replacement));
            unsigned int oldused = (unsigned int)PointerRing_GetSize(queue);
            unsigned int newsize = (unsigned int)PointerRing_GetUsableCapacity(queue);
            unsigned int a = oldused + count + 1;
            unsigned int b = newsize * 2;
            const unsigned int* max = UnsignedWord_SelectMaxPointer(&a, &b);
            GtlWordDeque_Reserve(grown, *max);
            unsigned int zero = 0;
            GtlWordDeque_ForwardPushBack(grown, &zero);
            for (; count != 0; --count) {
                void* block = SampleDeque_AllocatePointerSlots(queue2, kSlotsPerPage);
                *(void**)GtlSampleDeque_GetBackPageSlot(grown) = block;
                GtlWordDeque_ForwardPushBack(grown, &zero);
            }
            if (PointerRing_IsEmpty(queue) == 0) {
                // Iterators are four-word ABI buffers: insert [old begin, old end-1)
                // before grown begin, excluding the old trailing sentinel.
                SamplePageRingIterator insertionIterator, sourceBegin, sourceEndWithoutSentinel;
                PageRing_GetEndIterator(&insertionIterator, queue);
                RingBufferIterator_Subtract(&sourceEndWithoutSentinel, &insertionIterator, 1);
                PageRing_GetBeginIterator(&sourceBegin, queue);
                PageRing_GetBeginIterator(&insertionIterator, grown);
                PointerRing_InsertRangeWithIteratorCopies(grown, &insertionIterator, &sourceBegin,
                                                          &sourceEndWithoutSentinel, 0);
            }
            GtlSampleDeque_SwapPageRingState(queue, grown);
            // Commit: queue now owns all pages. grown holds the old pointer table;
            // clearing its size prevents freeing page pointers transferred to queue.
            PointerRing_ClearSize(grown);
            {
                eh5Cleanup29.release();
                SampleDeque_dtor((&replacement), -1);
            }
        }
    }
}

} // extern "C"
