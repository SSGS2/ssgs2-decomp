//
// Free helpers select a heap from the pointer's MEM2 address bit. The timestamp conversion
// in TimestampSink has an unused result, but it still sets stack and exception data.

#include "sdk_protos.h"
#include "classes/GtlNoticeOwner.h"
#include "classes/GtlHeapObject.h"

extern "C" {

extern OSMutex s_arrayHeapMutex;
extern unsigned int s_preferMem2Heap;
extern unsigned int g_mem1Heap;
extern unsigned int g_mem2Heap;
extern unsigned int kArrayHeap0Descriptor[12];
extern unsigned int mem2HeapDescriptorDefaults[12];

unsigned int OSGetTick(void);
void MemoryManager_InitHeaps(void*);
void AppHeap_QueryMaxAllocatableSize(int, int, int*);
}

// Scoped lock on the allocator mutex (original symbol __dt__9AllocLockFv).
struct AllocLock_u800441A0 {
    OSMutex* p;
    int active;
    AllocLock_u800441A0(OSMutex* mutex) : p(mutex), active(1) {
        OSLockMutex(p);
    }
    AllocLock_u800441A0(OSMutex* mutex, int lock) : p(mutex), active(lock) {
        if (active)
            OSLockMutex(p);
    }
    ~AllocLock_u800441A0() {
        if (active)
            OSUnlockMutex(p);
    }
};

// The game passes the timestamp to an inlined, empty sink taking `const double&`, and converts it to float there;
// both are discarded, but they leave the stack store and the extab "has vector" bit in the compiled code.
static inline void TimestampSink(const double& value) {
    float truncated = (float)value;
    (void)truncated;
}

// Descriptor word 5 is overwritten with the arena's current allocation limit.
struct InitData {
    unsigned int descriptorPrefix[5];
    unsigned int maxAllocationBytes;
    unsigned int descriptorTail[6];
};

enum {
    kMem1Heap = 0,
    kMem2Heap = 1,
    kHeapArena = 5,
    kStickyMem2Threshold = 0x200,
    kMem2AddressBit = 0x10000000,
    kTickScaleShift = 3,
    kBusToTimerShift = 2,
    kTimerMicrosecondDivisor = 125000
};
static const unsigned int kBusClockHzAddress = 0x800000F8;

#pragma push
#pragma use_lmw_stmw on
// Lazily create both heaps and their shared mutex from the descriptor templates.
static inline void InitializeArrayHeaps(int& token, int& maxMem1, int& maxMem2, InitData& mem1, InitData& mem2) {
    if (!g_mem1Heap) {
        MemoryManager_InitHeaps(&token);
        maxMem1 = 0;
        AppHeap_QueryMaxAllocatableSize(kMem1Heap, kHeapArena, &maxMem1);
        mem1 = *(InitData*)kArrayHeap0Descriptor;
        mem1.maxAllocationBytes = maxMem1;
        gtl::IHeapObject::Create(&mem1, &g_mem1Heap);
        maxMem2 = 0;
        AppHeap_QueryMaxAllocatableSize(kMem2Heap, kHeapArena, &maxMem2);
        mem2 = *(InitData*)mem2HeapDescriptorDefaults;
        mem2.maxAllocationBytes = maxMem2;
        gtl::IHeapObject::Create(&mem2, &g_mem2Heap);
        OSInitMutex(&s_arrayHeapMutex);
    }
}

extern "C" {

// Allocate from the preferred heap, falling back to MEM2 and retaining that preference for small requests.
unsigned int AppHeap_AllocateArrayWithMem2Fallback(unsigned int byteCount) {
    InitData data7, data6, data5, data4, data2, data3, data1, data0;
    int zero7, zero6, zero5, zero4, zero2, zero3, zero1, zero0;
    int token3, token2, token1, token0;
    struct Helpers {
        static int Lookup(unsigned int heap, unsigned int byteCount) {
            if (!byteCount)
                return 0;
            AllocLock_u800441A0 scope(&s_arrayHeapMutex, 0);
            return ((GtlNoticeOwner*)heap)->AllocateWithOptionalLock(byteCount, 1);
        }
    };
    int result;
    // 0x80045028, 0x800448E4, 0x80044274: zero, preferred pool, or fallback.
    if (!byteCount) {
        result = 0;
    } else if (s_preferMem2Heap) {
        InitializeArrayHeaps(token3, zero7, zero6, data7, data6);
        result = Helpers::Lookup(g_mem2Heap, byteCount);
    } else {
        InitializeArrayHeaps(token2, zero4, zero5, data4, data5);
        InitializeArrayHeaps(token1, zero2, zero3, data2, data3);
        result = Helpers::Lookup(g_mem1Heap, byteCount);
        if (!result) {
            InitializeArrayHeaps(token0, zero1, zero0, data1, data0);
            result = Helpers::Lookup(g_mem2Heap, byteCount);
            if (byteCount < kStickyMem2Threshold)
                s_preferMem2Heap = 1;
        }
    }
    return result;
}
#pragma pop
#pragma push
#pragma use_lmw_stmw on
// Semantic public name: AppHeap_AllocateWithMem2FallbackLocked.
// appMallocDummy__FUl is the retained ABI symbol: this performs real allocation
// with the allocator mutex held, falling back from MEM1 to MEM2 on failure.
unsigned int appMallocDummy__FUl(unsigned int byteCount) {
    InitData data7, data6, data5, data4, data2, data3, data1, data0;
    int zero7, zero6, zero5, zero4, zero2, zero3, zero1, zero0;
    int token3, token2, token1, token0;
    struct Helpers {
        static int Lookup(unsigned int heap, unsigned int byteCount) {
            if (!byteCount)
                return 0;
            AllocLock_u800441A0 scope(&s_arrayHeapMutex);
            return ((GtlNoticeOwner*)heap)->AllocateWithOptionalLock(byteCount, 0);
        }
    };
    int result;
    // 0x80045028, 0x800448E4, 0x80044274: zero, preferred pool, or fallback.
    if (!byteCount) {
        result = 0;
    } else if (s_preferMem2Heap) {
        InitializeArrayHeaps(token0, zero0, zero1, data0, data1);
        result = Helpers::Lookup(g_mem2Heap, byteCount);
    } else {
        InitializeArrayHeaps(token1, zero2, zero3, data2, data3);
        InitializeArrayHeaps(token2, zero4, zero5, data4, data5);
        result = Helpers::Lookup(g_mem1Heap, byteCount);
        if (!result) {
            InitializeArrayHeaps(token3, zero6, zero7, data6, data7);
            result = Helpers::Lookup(g_mem2Heap, byteCount);
            if (byteCount < kStickyMem2Threshold)
                s_preferMem2Heap = 1;
        }
    }
    return result;
}

// Allocate with the explicitly enabled allocator lock and MEM2 fallback.
unsigned int appMallocUni__FUl(unsigned int byteCount) {
    InitData data7, data6, data5, data4, data2, data3, data1, data0;
    int zero7, zero6, zero5, zero4, zero2, zero3, zero1, zero0;
    int token3, token2, token1, token0;
    struct Helpers {
        static int Lookup(unsigned int heap, unsigned int byteCount) {
            if (!byteCount)
                return 0;
            AllocLock_u800441A0 scope(&s_arrayHeapMutex, 1);
            return ((GtlNoticeOwner*)heap)->AllocateWithOptionalLock(byteCount, 0);
        }
    };
    int result;
    // 0x80045028, 0x800448E4, 0x80044274: zero, preferred pool, or fallback.
    if (!byteCount) {
        result = 0;
    } else if (s_preferMem2Heap) {
        InitializeArrayHeaps(token0, zero0, zero1, data0, data1);
        result = Helpers::Lookup(g_mem2Heap, byteCount);
    } else {
        InitializeArrayHeaps(token1, zero2, zero3, data2, data3);
        InitializeArrayHeaps(token2, zero4, zero5, data4, data5);
        result = Helpers::Lookup(g_mem1Heap, byteCount);
        if (!result) {
            InitializeArrayHeaps(token3, zero6, zero7, data6, data7);
            result = Helpers::Lookup(g_mem2Heap, byteCount);
            if (byteCount < kStickyMem2Threshold)
                s_preferMem2Heap = 1;
        }
    }
    return result;
}

#pragma pop

// Free through the pointer-selected heap and clear MEM2 preference after a MEM1 free.
void AppHeap_FreeByAddress(unsigned int address) {
    if (address & kMem2AddressBit) {
        unsigned int heap = g_mem2Heap;
        TimestampSink((double)((OSGetTick() << kTickScaleShift) /
                               ((*(unsigned int*)kBusClockHzAddress >> kBusToTimerShift) / kTimerMicrosecondDivisor)));
        ((GtlNoticeOwner*)heap)->InvokeFreeLocked((void*)address, 0);
    } else {
        s_preferMem2Heap = 0;
        unsigned int heap = g_mem1Heap;
        TimestampSink((double)((OSGetTick() << kTickScaleShift) /
                               ((*(unsigned int*)kBusClockHzAddress >> kBusToTimerShift) / kTimerMicrosecondDivisor)));
        ((GtlNoticeOwner*)heap)->InvokeFreeLocked((void*)address, 0);
    }
}

// Release a unified-heap allocation using its MEM2 address bit.
void appFreeUni__FPv(unsigned int address) {
    if (address & kMem2AddressBit) {
        unsigned int heap = g_mem2Heap;
        TimestampSink((double)((OSGetTick() << kTickScaleShift) /
                               ((*(unsigned int*)kBusClockHzAddress >> kBusToTimerShift) / kTimerMicrosecondDivisor)));
        ((GtlNoticeOwner*)heap)->InvokeFreeLocked((void*)address, 0);
    } else {
        s_preferMem2Heap = 0;
        unsigned int heap = g_mem1Heap;
        TimestampSink((double)((OSGetTick() << kTickScaleShift) /
                               ((*(unsigned int*)kBusClockHzAddress >> kBusToTimerShift) / kTimerMicrosecondDivisor)));
        ((GtlNoticeOwner*)heap)->InvokeFreeLocked((void*)address, 0);
    }
}

// Release through the pointer-selected heap with the alternate free-mode address enabled.
void AppHeap_FreeByAddressAlternateMode(unsigned int address) {
    if (address & kMem2AddressBit) {
        unsigned int heap = g_mem2Heap;
        TimestampSink((double)((OSGetTick() << kTickScaleShift) /
                               ((*(unsigned int*)kBusClockHzAddress >> kBusToTimerShift) / kTimerMicrosecondDivisor)));
        ((GtlNoticeOwner*)heap)->InvokeFreeLocked((void*)address, 1);
    } else {
        s_preferMem2Heap = 0;
        unsigned int heap = g_mem1Heap;
        TimestampSink((double)((OSGetTick() << kTickScaleShift) /
                               ((*(unsigned int*)kBusClockHzAddress >> kBusToTimerShift) / kTimerMicrosecondDivisor)));
        ((GtlNoticeOwner*)heap)->InvokeFreeLocked((void*)address, 1);
    }
}

} // extern "C"
