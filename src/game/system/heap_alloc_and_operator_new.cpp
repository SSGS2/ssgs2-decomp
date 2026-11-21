// Translation unit 0x80042B00-0x800441A0 (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

// OS clock word 0x800000F8 is bus frequency in Hz; timer frequency is bus/4.
// The unsigned tick expression scales nominally to microseconds before conversion.
// Evaluate the legacy timestamp conversion; the float result is intentionally discarded.
// The argument still affects code generation and exception metadata.
static inline void TimestampSink(const double& value) {
    float truncated = (float)value;
    (void)truncated;
}

enum HeapPolicy {
    kMem1Heap = 0, kMem2Heap = 1, kHeapArena = 5,
    kDescriptorLimitWord = 5, kStickyMem2ThresholdBytes = 0x200,
    kMem2AddressBit = 0x10000000, kTimestampDivisor = 125000,
    kTimestampScaleShift = 3, kBusToTimerShift = 2
};

// Allocations are owned by their callers; free selects the heap from address bit 28.
// Policy: try MEM1 then MEM2 unless the sticky MEM2 preference is already set.
// A failed MEM1 request below 512 bytes sets the preference even if MEM2 fails.
// Freeing a MEM1 address clears it. Zero-byte allocations return null.
// Counters measure operator-new calls minus delete calls, including failed/zero calls.
extern "C" {

extern int s_preferMem2Heap;
extern int g_mem1Heap;
extern int g_mem2Heap;
extern int s_scalarNewCallBalance;
extern int s_arrayNewCallBalance;

// Twelve-word descriptor copied from a template; word 5 is the allocation limit in bytes.
struct Descriptor {
    unsigned int words[12];
};

void GtlNoticeOwner_InvokeFreeLocked(void* a, void* b, int c);
void MemoryManager_InitHeaps(void* p);
void AppHeap_QueryMaxAllocatableSize(int a, int b, int* p);
void gtl_IHeapObject_Create(Descriptor* a, void* b);
void* GtlNoticeOwner_AllocateWithOptionalLock(void* a, unsigned int b, int c);
extern unsigned int kArrayHeap0Descriptor[12];
extern unsigned int mem2HeapDescriptorDefaults[12];
extern char s_arrayHeapMutex[24];

// Create two adjacent heap handles from borrowed descriptor templates and arena IDs.
// This retained helper has no callers in this unit.
static void setup_pair(int* heapHandles, unsigned int* firstTemplate, unsigned int* secondTemplate,
                       int firstHeapId, int secondHeapId) {
    int firstLimitBytes;
    int secondLimitBytes;
    MemoryManager_InitHeaps(&firstLimitBytes);
    firstLimitBytes = 0;
    AppHeap_QueryMaxAllocatableSize(firstHeapId, kHeapArena, &firstLimitBytes);
    Descriptor firstDescriptor = *(Descriptor*)firstTemplate;
    firstDescriptor.words[kDescriptorLimitWord] = (unsigned int)firstLimitBytes;
    gtl_IHeapObject_Create(&firstDescriptor, heapHandles);
    secondLimitBytes = 0;
    AppHeap_QueryMaxAllocatableSize(secondHeapId, kHeapArena, &secondLimitBytes);
    Descriptor secondDescriptor = *(Descriptor*)secondTemplate;
    secondDescriptor.words[kDescriptorLimitWord] = (unsigned int)secondLimitBytes;
    gtl_IHeapObject_Create(&secondDescriptor, heapHandles + 1);
}

// Scoped lock on the allocator mutex (original symbol __dt__9AllocLockFv).
struct AllocLock {
    OSMutex* m;
    int locked;
    // Acquire the shared allocator mutex for this scope.
    AllocLock(OSMutex* mutex) {
        m = mutex;
        locked = 1;
        OSLockMutex(m);
    }
    // Release the mutex on normal exit and exception unwinding.
    ~AllocLock() {
        if (locked) OSUnlockMutex(m);
    }
    // Delete guard storage through the same address-based heap policy as scalar delete.
    static void operator delete(void* self) throw() {
        s_scalarNewCallBalance = s_scalarNewCallBalance - 1;
        if (((unsigned int)self & kMem2AddressBit) != 0) {
            void* selectedHeap = (void*)g_mem2Heap;
            // Integer tick scaling is nominally microseconds; truncation/overflow precede double conversion.
            TimestampSink((double)((OSGetTick() << kTimestampScaleShift) / ((*(unsigned int*)0x800000F8 >> kBusToTimerShift) / kTimestampDivisor)));
            GtlNoticeOwner_InvokeFreeLocked(selectedHeap, self, 0);
        } else {
            s_preferMem2Heap = 0;
            void* selectedHeap = (void*)g_mem1Heap;
            // Integer tick scaling is nominally microseconds; truncation/overflow precede double conversion.
            TimestampSink((double)((OSGetTick() << kTimestampScaleShift) / ((*(unsigned int*)0x800000F8 >> kBusToTimerShift) / kTimestampDivisor)));
            GtlNoticeOwner_InvokeFreeLocked(selectedHeap, self, 0);
        }
    }
};

struct InitTmp {
    int v;
    // Preserve the heap-manager initialization token used by this reconstruction.
    InitTmp() { MemoryManager_InitHeaps(this); }
};

// Create both heap handles, then initialize their common mutex (lazy scalar path).
static inline void init_heap() {
    Descriptor mem2Descriptor, mem1Descriptor;
    int heapManagerToken, mem2LimitBytes, mem1LimitBytes;
    MemoryManager_InitHeaps(&heapManagerToken);
    mem1LimitBytes = 0;
    AppHeap_QueryMaxAllocatableSize(kMem1Heap, kHeapArena, &mem1LimitBytes);
    mem1Descriptor = *(Descriptor*)kArrayHeap0Descriptor;
    mem1Descriptor.words[kDescriptorLimitWord] = (unsigned int)mem1LimitBytes;
    gtl_IHeapObject_Create(&mem1Descriptor, &g_mem1Heap);
    mem2LimitBytes = 0;
    AppHeap_QueryMaxAllocatableSize(kMem2Heap, kHeapArena, &mem2LimitBytes);
    mem2Descriptor = *(Descriptor*)mem2HeapDescriptorDefaults;
    mem2Descriptor.words[kDescriptorLimitWord] = (unsigned int)mem2LimitBytes;
    gtl_IHeapObject_Create(&mem2Descriptor, &g_mem2Heap);
    OSInitMutex((OSMutex*)&s_arrayHeapMutex);
}

// Allocate bytes under the common mutex; return null for zero size or allocation failure.
// Retained helper; the exported allocation paths below spell out their lock scopes.
static inline void* heap_alloc(void* heap, unsigned int size) {
    void* result = heap;
    if (!size) return 0;
    AllocLock lockGuard((OSMutex*)&s_arrayHeapMutex);
    result = GtlNoticeOwner_AllocateWithOptionalLock(result, size, 0);
    return result;
}

#pragma push
#pragma use_lmw_stmw on
// Scalar new: count the call, lazily initialize heaps, and apply the fallback policy.
// Returns owned storage or null; this implementation does not throw on allocation failure.
void* __nw__FUl(unsigned int size) {
    void* result;
    s_scalarNewCallBalance = s_scalarNewCallBalance + 1;
    if (size == 0) {
        result = 0;
    } else if (s_preferMem2Heap) {
        // A sticky preference skips the MEM1 attempt.
        if (!g_mem1Heap) init_heap();
        result = (void*)g_mem2Heap;
        if (!size) {
            result = 0;
        } else {
            AllocLock lockGuard((OSMutex*)&s_arrayHeapMutex);
            result = GtlNoticeOwner_AllocateWithOptionalLock(result, size, 0);
        }
    } else {
        // Repeated lazy-init tests are intentional: preserve the original call/lock order.
        if (!g_mem1Heap) init_heap();
        if (!g_mem1Heap) init_heap();
        result = (void*)g_mem1Heap;
        if (!size) {
            result = 0;
        } else {
            AllocLock lockGuard((OSMutex*)&s_arrayHeapMutex);
            result = GtlNoticeOwner_AllocateWithOptionalLock(result, size, 0);
        }
        // MEM1 failed: retry on MEM2 and possibly make that preference sticky.
        if (!result) {
            if (!g_mem1Heap) init_heap();
            result = (void*)g_mem2Heap;
            if (!size) {
                result = 0;
            } else {
                AllocLock lockGuard((OSMutex*)&s_arrayHeapMutex);
                result = GtlNoticeOwner_AllocateWithOptionalLock(result, size, 0);
            }
            if (size < kStickyMem2ThresholdBytes) s_preferMem2Heap = 1;
        }
    }
    return result;
}
#pragma pop

// Initialize the heap-manager token before constructing array heap descriptors.
static inline void InitializeArrayHeaps() {
    int heapManagerToken;
    MemoryManager_InitHeaps(&heapManagerToken);
}

// Create MEM1/MEM2 handles and their mutex for the array-allocation path.
static inline void init_array_heap() {
    Descriptor mem2Descriptor, mem1Descriptor;
    int mem2LimitBytes, mem1LimitBytes;
    InitializeArrayHeaps();
    mem1LimitBytes = 0;
    AppHeap_QueryMaxAllocatableSize(kMem1Heap, kHeapArena, &mem1LimitBytes);
    mem1Descriptor = *(Descriptor*)kArrayHeap0Descriptor;
    mem1Descriptor.words[kDescriptorLimitWord] = (unsigned int)mem1LimitBytes;
    gtl_IHeapObject_Create(&mem1Descriptor, &g_mem1Heap);
    mem2LimitBytes = 0;
    AppHeap_QueryMaxAllocatableSize(kMem2Heap, kHeapArena, &mem2LimitBytes);
    mem2Descriptor = *(Descriptor*)mem2HeapDescriptorDefaults;
    mem2Descriptor.words[kDescriptorLimitWord] = (unsigned int)mem2LimitBytes;
    gtl_IHeapObject_Create(&mem2Descriptor, &g_mem2Heap);
    OSInitMutex((OSMutex*)&s_arrayHeapMutex);
}

// Same heap initialization with the original local declaration order retained.
static inline void init_array_heap_second() {
    Descriptor mem1Descriptor, mem2Descriptor;
    int mem1LimitBytes, mem2LimitBytes;
    InitializeArrayHeaps();
    mem1LimitBytes = 0;
    AppHeap_QueryMaxAllocatableSize(kMem1Heap, kHeapArena, &mem1LimitBytes);
    mem1Descriptor = *(Descriptor*)kArrayHeap0Descriptor;
    mem1Descriptor.words[kDescriptorLimitWord] = (unsigned int)mem1LimitBytes;
    gtl_IHeapObject_Create(&mem1Descriptor, &g_mem1Heap);
    mem2LimitBytes = 0;
    AppHeap_QueryMaxAllocatableSize(kMem2Heap, kHeapArena, &mem2LimitBytes);
    mem2Descriptor = *(Descriptor*)mem2HeapDescriptorDefaults;
    mem2Descriptor.words[kDescriptorLimitWord] = (unsigned int)mem2LimitBytes;
    gtl_IHeapObject_Create(&mem2Descriptor, &g_mem2Heap);
    OSInitMutex((OSMutex*)&s_arrayHeapMutex);
}

#pragma push
#pragma use_lmw_stmw on
// Array new: count the call and return owned storage using the shared fallback policy.
// Callers must pair successful allocations with array delete; failure returns null.
void* __nwa__FUl(unsigned int size) {
    s_arrayNewCallBalance = s_arrayNewCallBalance + 1;
    void* result;
    if (size == 0) {
        result = 0;
    } else if (s_preferMem2Heap) {
        // A sticky preference skips the MEM1 attempt.
        if (!g_mem1Heap) {
            init_array_heap();
        }
        result = (void*)g_mem2Heap;
        if (!size) {
            result = 0;
        } else {
            AllocLock lockGuard((OSMutex*)&s_arrayHeapMutex);
            result = GtlNoticeOwner_AllocateWithOptionalLock(result, size, 0);
        }
    } else {
        if (!g_mem1Heap) {
            init_array_heap_second();
        }
        if (!g_mem1Heap) {
            init_array_heap();
        }
        result = (void*)g_mem1Heap;
        if (!size) {
            result = 0;
        } else {
            AllocLock lockGuard((OSMutex*)&s_arrayHeapMutex);
            result = GtlNoticeOwner_AllocateWithOptionalLock(result, size, 0);
        }
        // MEM1 failed: retry on MEM2 and possibly make that preference sticky.
        if (!result) {
            if (!g_mem1Heap) {
                init_array_heap();
            }
            result = (void*)g_mem2Heap;
            if (!size) {
                result = 0;
            } else {
                AllocLock lockGuard((OSMutex*)&s_arrayHeapMutex);
                result = GtlNoticeOwner_AllocateWithOptionalLock(result, size, 0);
            }
            if (size < kStickyMem2ThresholdBytes) s_preferMem2Heap = 1;
        }
    }
    return result;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Allocate owned bytes with the same fallback policy but without updating a new counter.
// Return null for zero size/failure; this wrapper adds no ownership metadata.
void* AppMemory_AllocateWithHeapFallback(unsigned int size) {
    void* result;
    if (size == 0) {
        result = 0;
    } else if (s_preferMem2Heap) {
        // A sticky preference skips the MEM1 attempt.
        if (!g_mem1Heap) {
            init_array_heap();
        }
        result = (void*)g_mem2Heap;
        if (!size) {
            result = 0;
        } else {
            AllocLock lockGuard((OSMutex*)&s_arrayHeapMutex);
            result = GtlNoticeOwner_AllocateWithOptionalLock(result, size, 0);
        }
    } else {
        if (!g_mem1Heap) {
            init_array_heap_second();
        }
        if (!g_mem1Heap) {
            init_array_heap();
        }
        result = (void*)g_mem1Heap;
        if (!size) {
            result = 0;
        } else {
            AllocLock lockGuard((OSMutex*)&s_arrayHeapMutex);
            result = GtlNoticeOwner_AllocateWithOptionalLock(result, size, 0);
        }
        // MEM1 failed: retry on MEM2 and possibly make that preference sticky.
        if (!result) {
            if (!g_mem1Heap) {
                init_array_heap();
            }
            result = (void*)g_mem2Heap;
            if (!size) {
                result = 0;
            } else {
                AllocLock lockGuard((OSMutex*)&s_arrayHeapMutex);
                result = GtlNoticeOwner_AllocateWithOptionalLock(result, size, 0);
            }
            if (size < kStickyMem2ThresholdBytes) s_preferMem2Heap = 1;
        }
    }
    return result;
}
#pragma pop

// Scalar delete: decrement the call balance and free through the address-selected heap.
// No null special case is added here; even null follows the MEM1 branch.
void __dl__FPv(void* p) throw() {
    s_scalarNewCallBalance = s_scalarNewCallBalance - 1;
    if (((unsigned int)p & kMem2AddressBit) != 0) {
        void* selectedHeap = (void*)g_mem2Heap;
        TimestampSink((double)((OSGetTick() << kTimestampScaleShift) / ((*(unsigned int*)0x800000F8 >> kBusToTimerShift) / kTimestampDivisor)));
        GtlNoticeOwner_InvokeFreeLocked(selectedHeap, p, 0);
    } else {
        s_preferMem2Heap = 0;
        void* selectedHeap = (void*)g_mem1Heap;
        TimestampSink((double)((OSGetTick() << kTimestampScaleShift) / ((*(unsigned int*)0x800000F8 >> kBusToTimerShift) / kTimestampDivisor)));
        GtlNoticeOwner_InvokeFreeLocked(selectedHeap, p, 0);
    }
}

// Array delete: decrement the array call balance and use the address-selected heap.
void __dla__FPv(void* p) throw() {
    s_arrayNewCallBalance = s_arrayNewCallBalance - 1;
    if (((unsigned int)p & kMem2AddressBit) != 0) {
        void* selectedHeap = (void*)g_mem2Heap;
        TimestampSink((double)((OSGetTick() << kTimestampScaleShift) / ((*(unsigned int*)0x800000F8 >> kBusToTimerShift) / kTimestampDivisor)));
        GtlNoticeOwner_InvokeFreeLocked(selectedHeap, p, 0);
    } else {
        s_preferMem2Heap = 0;
        void* selectedHeap = (void*)g_mem1Heap;
        TimestampSink((double)((OSGetTick() << kTimestampScaleShift) / ((*(unsigned int*)0x800000F8 >> kBusToTimerShift) / kTimestampDivisor)));
        GtlNoticeOwner_InvokeFreeLocked(selectedHeap, p, 0);
    }
}

} // extern "C"
