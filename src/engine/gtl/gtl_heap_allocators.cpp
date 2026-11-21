// Heap configuration crosses a legacy pointer-word ABI at explicit casts below.
// Numeric configuration; void* casts occur only at the legacy callback/constructor ABI.
// Sizes/flags/kinds/shifts never denote owned allocations. Notice arguments stay opaque.
struct HeapCreateParameters {
    unsigned int allocatorKind;  // ListAllocator=0, FixedVectorAllocator=1
    unsigned int allocatorFlags; // policy forwarded unchanged; bit meanings unresolved
    unsigned int elementShift;   // alignment is 1 << elementShift bytes
    unsigned int flags;          // bit 0 skips heap-object locking
    int mainNoticeKind;          // MEM1=0, MEM2=1, no notice=2
    unsigned int mainSize;       // bytes requested from first provider
    void* mainNoticeArgument0;
    void* mainNoticeArgument1;
    int localNoticeKind;
    unsigned int localSize;      // bytes requested from second provider
    void* localNoticeArgument0;
    void* localNoticeArgument1;
};
struct HeapObjectTailView {
    char objectPrefix[0x24]; void* initialState; void* flags; char mutex[0x18];
};
struct NoticeObjectView { int references; void* vtable; };
struct MutexGuardView { void* mutex; int locked; };
typedef char HeapCreateParameters_size[(sizeof(HeapCreateParameters) == 0x30) ? 1 : -1];
typedef char HeapObjectTail_mutex_offset[((int)&((HeapObjectTailView*)0)->mutex == 0x2C) ? 1 : -1];
enum NoticeKind { MainMemoryNotice = 0, LocalMemoryNotice = 1, NoMemoryNotice = 2 };
enum AllocatorKind { ListAllocator = 0, FixedVectorAllocator = 1 };
enum { kSkipAllocatorLock = 1, kShiftJisSubtableCount = 9,
       kShiftJisAutoLength = -1, kShiftJisScanLimit = 1024,
       kAllocatorObjectBytes = 0x2C, kHeapObjectBytes = 0x44 };

#include "classes/GtlNoticeOwner.h"
#include "classes/GtlListAllocator.h"
#include "classes/GtlHeapObject.h"
#include "classes/GtlFixedVectorAllocator.h"
#include "classes/GtlAllocatorNode.h"
#include "classes/GtlAllocationRange.h"
// Translation unit 0x80421FE8-0x80422F10 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

extern "C" {

extern void* g_graphicsMemoryCallbacks;
extern void* g_mem1ExpHeap;
extern void* g_mem2ExpHeap;
extern unsigned int g_defaultListAllocator;
extern unsigned int g_memoryNoticeFactory;
extern signed char s_memoryNoticeFactoryInitialized;
extern char s_memoryNoticeFactory;
extern char s_memoryNoticeFactoryDtorRegistration[];
extern char s_mainMemoryAllocatorState[];
extern char __vt__Q23gtl11IHeapObject[];
extern char __vt__Q23gtl25CNoticeObjectOfMainMemory[];
extern char __vt__Q23gtl26CNoticeObjectOfLocalMemory[];
extern unsigned int shiftJisConversionSubtables[];
void* GtlDefaultAllocator_AllocateLocked(void*);
// MEMFreeToExpHeap takes (heap, block): it tests r4 and frees r4-0x10 (0x80385800-0x80385840).
void MEMFreeToExpHeap(void*, void*);
int MEMAllocFromExpHeapEx(void*, int, int);
void GtlDefaultAllocator_FreeLocked(void*);
void __dl__FPv(void*);
int gtl_HeapObject_InitializeAllocators(void*, void*);
void* CListAllocator_Ctor(void*, void*);
void* gtl_CFixedVectorAllocator_Ctor(void*, void*);
void* CMemoryNoticeFactory_Dtor(void*, int);
int CMemoryNoticeFactory_Create(void*, int*, void**);

struct RcBase {
    int rc;
    virtual void rel(int);
    virtual void s1();
    virtual void run(void*, void*);
};

// Run both memory-release notices and drop reference-counted allocator ownership.
void* gtl_IHeapObject_dtor(void* self, int deleting) {
    if (self) {
        ((GtlHeapObject*)self)->vtable = (void*)__vt__Q23gtl11IHeapObject;
        {
            RcBase* p = (*(RcBase**)&((GtlHeapObject*)self)->mainNotice);
            if (p) {
                p->run(((GtlHeapObject*)self)->mainMemory, ((GtlHeapObject*)self)->mainArgument);
                p = (*(RcBase**)&((GtlHeapObject*)self)->mainNotice);
                if (--p->rc == 0 && p)
                    p->rel(1);
            }
        }
        {
            RcBase* p = (*(RcBase**)&((GtlHeapObject*)self)->localNotice);
            if (p) {
                p->run(((GtlHeapObject*)self)->localMemory, ((GtlHeapObject*)self)->localArgument);
                p = (*(RcBase**)&((GtlHeapObject*)self)->localNotice);
                if (--p->rc == 0 && p)
                    p->rel(1);
            }
        }
        {
            RcBase* p = (*(RcBase**)&((GtlHeapObject*)self)->allocator);
            if (p) {
                if (--p->rc == 0 && p)
                    p->rel(1);
            }
        }
        if (deleting > 0)
            GtlDefaultAllocator_FreeLocked(self);
    }
    return self;
}

struct SubTable {
    unsigned int table;
    unsigned short lo;
    unsigned short hi;
};

struct SubTableList {
    SubTable entries[kShiftJisSubtableCount];
};

// Count input bytes or map Shift-JIS codes to wide codes; auto-length scanning includes the NUL byte.
// byteLimit bounds input bytes, not output characters. -1 requires a NUL within
// the first 1024 bytes; otherwise the original code leaves the limit at -1.
// useTable == 0 only counts bytes and never touches result. Otherwise result
// must have space for every decoded code (including NUL in auto-length mode).
// A double-byte lead requires an accessible following byte even at the limit.
int Text_ConvertShiftJisUsingCodeTable(void*, void*, unsigned char* text, int byteLimit, unsigned short* result, int useTable) {
    int inputByteCount = byteLimit;
    if (inputByteCount == kShiftJisAutoLength) {
        unsigned char* inputCursor = text;
        for (int i = 0; i < kShiftJisScanLimit; i += 8, inputCursor += 8) {
            if (inputCursor[0] == 0) { inputByteCount = i + 1; break; }
            if (inputCursor[1] == 0) { inputByteCount = i + 2; break; }
            if (inputCursor[2] == 0) { inputByteCount = i + 3; break; }
            if (inputCursor[3] == 0) { inputByteCount = i + 4; break; }
            if (inputCursor[4] == 0) { inputByteCount = i + 5; break; }
            if (inputCursor[5] == 0) { inputByteCount = i + 6; break; }
            if (inputCursor[6] == 0) { inputByteCount = i + 7; break; }
            if (inputCursor[7] == 0) { inputByteCount = i + 8; break; }
        }
    }
    // Phase 1: counting mode consumes one or two input bytes per character.
    if (!useTable) {
        int bytes = 0;
        int bytesConsumed = 0;
        unsigned char* inputCursor = text;
        while (bytesConsumed < inputByteCount) {
            unsigned int leadByte = *inputCursor;
            int isDoubleByte;
            if (leadByte <= 0x7f || (leadByte >= 0xa0 && leadByte <= 0xdf))
                isDoubleByte = 0;
            else
                isDoubleByte = 1;
            if (isDoubleByte) { bytes += 2; ++bytesConsumed; ++inputCursor; }
            else ++bytes;
            ++bytesConsumed; ++inputCursor;
        }
        return bytes;
    }
    // Phase 2: combine lead/trail bytes, map the first matching table range,
    // and copy unmapped codes through unchanged. No extra terminator is added.
    int outputCharCount = 0;
    int inputIndex = 0;
    unsigned char* inputCursor;
    while (inputIndex < inputByteCount) {
        inputCursor = text + inputIndex;
        unsigned char leadByte = text[inputIndex];
        int isDoubleByte;
        if (leadByte <= 0x7f || (leadByte >= 0xa0 && leadByte <= 0xdf))
            isDoubleByte = 0;
        else
            isDoubleByte = 1;
        unsigned short characterCode;
        if (isDoubleByte) {
            characterCode = inputCursor[1];
            characterCode |= leadByte << 8;
            ++inputIndex;
        } else {
            characterCode = leadByte;
        }
        SubTableList codeTables = *(SubTableList*)shiftJisConversionSubtables;
        int i = 0;
        SubTable* subtable = codeTables.entries;
        for (; i < kShiftJisSubtableCount; ++subtable, ++i) {
            if (subtable->lo <= characterCode && characterCode <= subtable->hi) {
                characterCode = ((unsigned short*)codeTables.entries[i].table)[characterCode - codeTables.entries[i].lo];
                break;
            }
        }
        *result++ = characterCode;
        ++outputCharCount; ++inputIndex;
    }
    return outputCharCount;
}

// Return the fixed-vector capacity through the pointer-sized allocator ABI.
void* gtl_CFixedVectorAllocator_GetCapacityBytes(void* self) { return (*(void**)&((GtlFixedVectorAllocator*)self)->capacity); }

// Return the list allocator value stored at byte 0x24.
void* gtl_CListAllocator_vfunc_4(void* self) { return ((GtlListAllocator*)self)->arenaSize; }

// Free a non-null block from the MEM1 expandable heap.
int gtl_CNoticeObjectOfMainMemory_FreeBlock(void* self, void* arg) {
    if (!arg)
        return 0;
    // Original 0x80422A70-0x80422A8C: r4 (arg) is left untouched into bl MEMFreeToExpHeap, so arg is the block freed;
    // ours dropped it, freeing whatever r4 held.
    MEMFreeToExpHeap(g_mem1ExpHeap, arg);
    return 1;
}

// Free a non-null block from the MEM2 expandable heap.
int gtl_CNoticeObjectOfLocalMemory_FreeMem2(void* self, void* arg) {
    if (!arg)
        return 0;
    // Original 0x80422B04-0x80422B20: r4 (arg) is left untouched into bl MEMFreeToExpHeap, so arg is the block freed;
    // ours dropped it, freeing whatever r4 held.
    MEMFreeToExpHeap(g_mem2ExpHeap, arg);
    return 1;
}

// Volatile cursor copies are compiler spills; p borrows the shared accumulator,
// s is the element-to-byte shift, and z is a zero comparison seed. No ownership.
struct CmpCursor {
    volatile unsigned int* p;
    unsigned int s;
    unsigned int z;
};

// Find the largest free byte span: secondList is the free-list head at +0x20.
// Each node stores elementCount; shifting converts it to bytes.
int gtl_CFixedVectorAllocator_GetLargestFreeByteSpan(void* self) {
    unsigned int shift = ((GtlFixedVectorAllocator*)self)->elementShift;
    volatile unsigned int largestByteSpan;
    volatile unsigned int* p = &largestByteSpan;
    unsigned char* node = (*(unsigned char**)&((GtlFixedVectorAllocator*)self)->secondList);
    volatile unsigned int candidateByteSpan;
    volatile CmpCursor comparisonCursorSpill;
    volatile CmpCursor maximumCursorSpill;
    maximumCursorSpill.p = p;
    maximumCursorSpill.s = shift;
    maximumCursorSpill.z = 0;
    largestByteSpan = 0;
    comparisonCursorSpill.p = p;
    comparisonCursorSpill.s = shift;
    comparisonCursorSpill.z = 0;
    while (node) {
        unsigned int nodeBytes = ((GtlAllocatorNode*)node)->elementCount << shift;
        candidateByteSpan = nodeBytes;
        volatile unsigned int* selectedMaximum = (*p < nodeBytes) ? &candidateByteSpan : p;
        *p = *selectedMaximum;
        node = (*(unsigned char**)&((GtlAllocatorNode*)node)->next);
    }
    return largestByteSpan;
}

struct VBase {
    int rc;
    virtual int s0() = 0;
    virtual int s1() = 0;
    virtual int s2() = 0;
    virtual int s3() = 0;
    virtual int s4() = 0;
    virtual int s5() = 0;
};

// Return allocated bytes = capacity - free bytes (virtual s5 sums secondList).
int gtl_CFixedVectorAllocator_GetAllocatedByteCount(void* self) {
    VBase* o = (VBase*)self;
    return ((GtlFixedVectorAllocator*)self)->capacity - o->s5();
}

// Round an integer up to the next multiple, retaining values already aligned.
// Caller supplies m != 0; intended for nonnegative sizes and positive multiples.
int Math_RoundUpToMultiple(int n, int m) {
    int r = n % m;
    return r ? n + (m - r) : n;
}

// Classify bytes outside ASCII and half-width katakana as two-byte Shift-JIS leads.
int ShiftJIS_IsLeadByte(void* self, unsigned int c) {
    if (c <= 0x7F || (c >= 0xA0 && c <= 0xDF))
        return 0;
    return 1;
}

// Returns the allocator callback's result (bctrl at 0x80422E58, r3 kept to the blr); callers test it
// (e.g. 0x8042D55C).
// Allocate graphics memory through the installed allocation callback.
void* gtl_Graphics_AllocateWithCallback(unsigned int size) {
    typedef void* (*Callback)(unsigned int);
    unsigned int volatile keep = size;
    return ((Callback*)g_graphicsMemoryCallbacks)[0](size);
}

// Free graphics memory through the installed release callback.
void gtl_Graphics_FreeWithCallback(void* self) {
    typedef void (*Callback)(void*);
    void* volatile keep = self;
    ((Callback*)g_graphicsMemoryCallbacks)[1](self);
}

// Copy a transposed 4-by-4 matrix into the 3-by-4 destination, omitting the last row.
// Row-major indexing: destination[row*4+column] = source[column*4+row],
// row in [0,2], column in [0,3]. This is a storage conversion, not an inverse.
void Mtx44_TransposeToMtx34(void* dst, void* src) {
    float* destination = (float*)dst;
    float* source = (float*)src;
    float s0 = source[0], s1 = source[1];
    float s4 = source[4], s5 = source[5];
    float s2 = source[2];
    destination[0] = s0;
    destination[1] = s4;
    float s6 = source[6];
    destination[4] = s1;
    destination[5] = s5;
    float s8 = source[8], s9 = source[9];
    destination[8] = s2;
    destination[9] = s6;
    float s12 = source[12], s13 = source[13];
    float s10 = source[10];
    destination[2] = s8;
    destination[3] = s12;
    float s14 = source[14];
    destination[6] = s9;
    destination[7] = s13;
    destination[10] = s10;
    destination[11] = s14;
}

} // extern "C"

struct SubObjA {
    // Allocate the fixed-size list-allocator object through the default heap.
    static void* operator new(size_t size) { return GtlDefaultAllocator_AllocateLocked((void*)kAllocatorObjectBytes); }
    // Return allocator-object storage to the default heap under its mutex.
    static void operator delete(void* p) { GtlDefaultAllocator_FreeLocked(p); }
    // Construct a list allocator from the seven-word buffer/configuration record.
    SubObjA(void** args) { CListAllocator_Ctor(this, args); }
};

struct SubObjB {
    // Allocate the fixed-size vector-allocator object through the default heap.
    static void* operator new(size_t size) { return GtlDefaultAllocator_AllocateLocked((void*)kAllocatorObjectBytes); }
    // Return allocator-object storage to the default heap under its mutex.
    static void operator delete(void* p) { GtlDefaultAllocator_FreeLocked(p); }
    // Construct a fixed-vector allocator from the seven-word configuration record.
    SubObjB(void** args) { gtl_CFixedVectorAllocator_Ctor(this, args); }
};

extern "C" {

struct Arr3 { void* v[3]; };

struct ChildBase {
    int rc;
    virtual int s0() = 0;
    virtual int op(void*, void*, void*, void*, void*) = 0;
};

// Create memory notices, allocate their buffers, construct the selected allocator, and initialize its mutex.
// srcv borrows HeapCreateParameters; selfv retains the created notices/buffers.
// Return 0 on the first failed step, 1 after mutex initialization. Earlier owned
// state is left installed for destruction; this function does not unwind it.
int gtl_HeapObject_InitializeAllocators(void* selfv, void* srcv) {
    // Phase 1: publish sizes/flags and create both memory-notice providers.
    GtlHeapObject* self = (GtlHeapObject*)selfv;
    HeapCreateParameters* src = (HeapCreateParameters*)srcv;
    void* mainSize = (void*)src->mainSize;
    void* lockingFlags = (void*)src->flags;
    void* zero = 0;
    unsigned int noticeFactory = g_memoryNoticeFactory;
    void* localSize = (void*)src->localSize;
    ((HeapObjectTailView*)self)->initialState = (void*)1;
    ((HeapObjectTailView*)self)->flags = lockingFlags;
    self->mainMemory = zero;
    self->mainArgument = mainSize;
    self->localMemory = zero;
    self->localArgument = localSize;

    if (!noticeFactory) {
        if (!s_memoryNoticeFactoryInitialized) {
            __register_global_object(&s_memoryNoticeFactory, (void*)CMemoryNoticeFactory_Dtor, s_memoryNoticeFactoryDtorRegistration);
            s_memoryNoticeFactoryInitialized = 1;
        }
        g_memoryNoticeFactory = (unsigned int)&s_memoryNoticeFactory;
    }
    void* manager = (void*)g_memoryNoticeFactory;
    if (!CMemoryNoticeFactory_Create(manager, (int*)&src->mainNoticeKind, (void**)&self->mainNotice)) return 0;
    if (!CMemoryNoticeFactory_Create(manager, (int*)&src->localNoticeKind, (void**)&self->localNotice)) return 0;

    // Phase 2: ask each present provider for its buffer. NoMemoryNotice skips allocation.
    ChildBase* child = (ChildBase*)self->mainNotice;
    if (child) {
        if (!child->op(&self->mainMemory, (void*)src->mainSize, (void*)src->elementShift, src->mainNoticeArgument0, src->mainNoticeArgument1)) return 0;
    }
    child = (ChildBase*)self->localNotice;
    if (child) {
        if (!child->op(&self->localMemory, (void*)src->localSize, (void*)src->elementShift, src->localNoticeArgument0, src->localNoticeArgument1)) return 0;
    }

    // Phase 3: flatten the two buffer/size pairs into the allocator constructor ABI.
    void* args[7];
    void* allocatorFlags = (void*)src->allocatorFlags;
    void* allocatorKind = (void*)src->allocatorKind;
    void* elementShift = (void*)src->elementShift;
    args[0] = allocatorKind;
    args[1] = allocatorFlags;
    args[2] = elementShift;
    args[3] = self->mainMemory;
    args[4] = self->mainArgument;
    args[5] = self->localMemory;
    args[6] = self->localArgument;
    int ok;
    if (&self->allocator == 0) {
        ok = 0;
    } else {
        switch ((int)allocatorKind) {
        case ListAllocator:
            self->allocator = new SubObjA(args);
            break;
        case FixedVectorAllocator:
            self->allocator = new SubObjB(args);
            break;
        default:
            self->allocator = 0;
            break;
        }
        ok = self->allocator != 0;
    }
    if (!ok) return 0;
    // The allocator is now installed; only a successful path initializes this mutex.
    OSInitMutex((OSMutex*)&((HeapObjectTailView*)self)->mutex);
    return 1;
}

#pragma auto_inline off
struct W2B { char pad[4]; };
class W2 : public W2B { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(void*);
};
// Free a block through the default allocator while holding its mutex.
void GtlDefaultAllocator_FreeLocked(void* self) {
    if (g_defaultListAllocator) {
        OSLockMutex((OSMutex*)s_mainMemoryAllocatorState);
        void* obj = (void*)g_defaultListAllocator;
((W2*)obj)->v14(self);
        OSUnlockMutex((OSMutex*)s_mainMemoryAllocatorState);
    }
}
#pragma auto_inline on

// Optionally delete a fixed-vector allocator through the default allocator.
void* gtl_CFixedVectorAllocator_dtor(void* p, int deletingFlag) {
    if (p && deletingFlag > 0)
        GtlDefaultAllocator_FreeLocked(p);
    return p;
}

// Optionally delete the memory-notice factory through the default allocator.
void* CMemoryNoticeFactory_Dtor(void* p, int deletingFlag) {
    if (p && deletingFlag > 0)
        GtlDefaultAllocator_FreeLocked(p);
    return p;
}

// Optionally delete a MEM1 memory notice through the default allocator.
void* gtl_CNoticeObjectOfMainMemory_dtor(void* p, int deletingFlag) {
    if (p && deletingFlag > 0)
        GtlDefaultAllocator_FreeLocked(p);
    return p;
}

struct AccCursor {
    volatile unsigned int* p;
    unsigned int s;
};

// Sum free bytes: each secondList node contributes elementCount << elementShift.
// Allocation/free in gtl_memory_allocators_and_heaps.cpp establish firstList as allocated spans and
// secondList as free spans. Duplicate volatile cursor records borrow acc.
unsigned int gtl_CFixedVectorAllocator_GetFreeByteCount(void* self) {
    volatile unsigned int freeByteTotal;
    unsigned int shift = ((GtlFixedVectorAllocator*)self)->elementShift;
    volatile unsigned int* p = &freeByteTotal;
    void* node = ((GtlFixedVectorAllocator*)self)->secondList;
    volatile AccCursor iterationCursorSpill;
    volatile AccCursor sumCursorSpill;
    sumCursorSpill.p = p;
    sumCursorSpill.s = shift;
    freeByteTotal = 0;
    iterationCursorSpill.p = p;
    iterationCursorSpill.s = shift;
    while (node) {
        *p = *p + (((GtlAllocatorNode*)node)->elementCount << shift);
        node = ((GtlAllocatorNode*)node)->next;
    }
    return freeByteTotal;
}

// Allocate a MEM1 block with alignment specified as a power-of-two shift.
int gtl_CNoticeObjectOfMainMemory_AllocMem1Block(void* self, void* blockOut, int byteCount, int alignmentShift) {
    if (!blockOut)
        return 0;
    int allocatedBlock = MEMAllocFromExpHeapEx(g_mem1ExpHeap, byteCount, 1 << alignmentShift);
    *(int*)blockOut = allocatedBlock;
    return allocatedBlock != 0;
}

// Optionally delete a MEM2 memory notice through the default allocator.
void* gtl_CNoticeObjectOfLocalMemory_dtor(void* p, int deletingFlag) {
    if (p && deletingFlag > 0)
        GtlDefaultAllocator_FreeLocked(p);
    return p;
}

// Allocate a MEM2 block with alignment specified as a power-of-two shift.
int gtl_CNoticeObjectOfLocalMemory_AllocMem2Block(void* self, void* blockOut, int byteCount, int alignmentShift) {
    if (!blockOut)
        return 0;
    int allocatedBlock = MEMAllocFromExpHeapEx(g_mem2ExpHeap, byteCount, 1 << alignmentShift);
    *(int*)blockOut = allocatedBlock;
    return allocatedBlock != 0;
}

// Unlock the mutex only when its guard owns the lock, then optionally delete the guard.
void* MutexGuard_dtor(void* self, int flag) {
    if (self) {
        if (((MutexGuardView*)self)->locked != 0) {
            OSUnlockMutex((OSMutex*)((MutexGuardView*)self)->mutex);
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

struct NoticeOwnerMutexGuard {
    void* mutex;
    int locked;
    // Unlock on scope exit only if this guard acquired the borrowed mutex.
    ~NoticeOwnerMutexGuard() { MutexGuard_dtor(this, 0); }
};

struct VSlot3 {
    int rc;
    virtual void s0();
    virtual int call3(void*, int);
};

// Allocate byteCountWord bytes; return the allocator address word or zero.
// byteCountWord is a numeric size, not borrowed storage. The caller owns the block.
// (owner flags | lockOptions) bit 0 skips locking; other option bits are ignored.
// call3 is allocation(size, zero), as at W3::v0C; lockOptions is not forwarded.
int GtlNoticeOwner_AllocateWithOptionalLock(void* self, void* byteCountWord, int lockOptions) {
    if (!((GtlNoticeOwner*)self)->object)
        return 0;
    int flags = ((GtlNoticeOwner*)self)->flags;
    NoticeOwnerMutexGuard guard;
    guard.mutex = ((GtlNoticeOwner*)self)->mutex; // GtlNoticeOwner_AllocateWithOptionalLock: +0x2C
    guard.locked = !((flags | lockOptions) & kSkipAllocatorLock);
    if (guard.locked)
        OSLockMutex((OSMutex*)guard.mutex);
    VSlot3* object = (*(VSlot3**)&((GtlNoticeOwner*)self)->object);
    int result = object->call3(byteCountWord, 0);
    return result;
}

// Invoke the owned release callback with optional mutex protection.
int GtlNoticeOwner_InvokeFreeLocked(void* self, void* arg, int condition) {
    if (!((GtlNoticeOwner*)self)->object)
        return 0;
    int flags = ((GtlNoticeOwner*)self)->flags;
    NoticeOwnerMutexGuard guard = { ((GtlNoticeOwner*)self)->mutex, (int)!((flags | condition) & kSkipAllocatorLock) }; // GtlNoticeOwner_InvokeFreeLocked: +0x2C
    if (guard.locked)
        OSLockMutex((OSMutex*)guard.mutex);
    typedef int (*Callback)(void*, void*);
    void* object = ((GtlNoticeOwner*)self)->object;
    Callback fn = (Callback)((*(void***)&((NoticeObjectView*)object)->vtable))[5];
    int result = fn(object, arg);
    return result;
}

// Create a MEM1 notice, MEM2 notice, or empty notice according to the requested kind.
// out is required; mode is borrowed and must be readable. Success means a valid
// kind was selected, even if allocation returned null (or NoMemoryNotice was chosen).
// A non-null result carries one owned reference, released by the heap object.
int CMemoryNoticeFactory_Create(void* self, int* mode, void** out) {
    if (!out)
        return 0;
    int value = *mode;
    switch (value) {
    case MainMemoryNotice: {
        void* allocated = GtlDefaultAllocator_AllocateLocked((void*)8);
        if (allocated) {
            ((NoticeObjectView*)allocated)->references = 1;
            ((NoticeObjectView*)allocated)->vtable = (void*)__vt__Q23gtl25CNoticeObjectOfMainMemory;
        }
        *out = allocated;
        break;
    }
    case LocalMemoryNotice: {
        void* allocated = GtlDefaultAllocator_AllocateLocked((void*)8);
        if (allocated) {
            ((NoticeObjectView*)allocated)->references = 1;
            ((NoticeObjectView*)allocated)->vtable = (void*)__vt__Q23gtl26CNoticeObjectOfLocalMemory;
        }
        *out = allocated;
        break;
    }
    case NoMemoryNotice:
        *out = 0;
        break;
    default:
        *out = 0;
        return 0;
    }
    return 1;
}

// Count nodes in the selected allocator list, rejecting invalid list selectors.
int gtl_CFixedVectorAllocator_CountListNodes(void* self, int mode) {
    void* node;
    switch (mode) {
        case 0:
            node = ((GtlFixedVectorAllocator*)self)->firstList;
            break;
        case 1:
            node = ((GtlFixedVectorAllocator*)self)->secondList;
            break;
        default:
            return 0;
    }
    volatile int count = 0;
    volatile int* p = &count;
    while (node) {
        *p = *p + 1;
        node = ((GtlAllocatorNode*)node)->next;
    }
    return *p;
}

#pragma auto_inline off
struct W3B { char pad[4]; };
class W3 : public W3B { public: 
virtual void v08(); virtual void* v0C(void*,int);
};
// Allocate from the default allocator under its mutex, returning null if it is unavailable.
void* GtlDefaultAllocator_AllocateLocked(void* arg) {
    void* result;
    if (g_defaultListAllocator == 0) {
        result = 0;
    } else {
        OSLockMutex((OSMutex*)s_mainMemoryAllocatorState);
        result = ((W3*)g_defaultListAllocator)->v0C(arg, 0);
        OSUnlockMutex((OSMutex*)s_mainMemoryAllocatorState);
    }
    return result;
}
#pragma auto_inline on

// Iterate allocator ranges, returning each base-relative offset and byte size.
// rangeOut's first word is the iteration token: zero starts, nonzero resumes the
// allocator's shared cursor. Selector zero uses firstList; every other value uses
// secondList. Returns 1 for a range, 0 at end/invalid output; do not interleave scans.
int gtl_CFixedVectorAllocator_GetNextRange(void* self, void* rangeOut, int listSelector) {
    if (!rangeOut)
        return 0;
    void* node;
    if (listSelector == 0)
        node = ((GtlFixedVectorAllocator*)self)->firstList;
    else
        node = ((GtlFixedVectorAllocator*)self)->secondList;
    if (!node)
        return 0;
    do {
        if (*(int*)rangeOut == 0)
            break;
        node = ((GtlFixedVectorAllocator*)self)->cursor;
        if (!node)
            return 0;
    } while (0);
    *(int*)rangeOut = *(int*)node;
    int baseAddress = *(int*)(((GtlFixedVectorAllocator*)self)->base);
    ((GtlAllocationRange*)rangeOut)->offsetFromBase = *(int*)node - baseAddress;
    ((GtlAllocationRange*)rangeOut)->byteSize = (*(int*)&((GtlAllocatorNode*)node)->elementCount) << (*(int*)&((GtlFixedVectorAllocator*)self)->elementShift);
    ((GtlFixedVectorAllocator*)self)->cursor = ((GtlAllocatorNode*)node)->next;
    return 1;
}

// Allocate and initialize a heap object, preserving the original initializer-result handling.
// out must be writable. Returns 0 only on allocation failure; initializer failure
// is ignored, so return 1 does not guarantee a fully initialized allocator.
int gtl_IHeapObject_Create(void* self, void* out) {
    void* allocated = GtlDefaultAllocator_AllocateLocked((void*)kHeapObjectBytes);
    if (allocated) {
        ((GtlHeapObject*)allocated)->references = 1;
        ((GtlHeapObject*)allocated)->vtable = (void*)__vt__Q23gtl11IHeapObject;
        (*(int*)&((GtlHeapObject*)allocated)->allocator) = 0;
        (*(int*)&((GtlHeapObject*)allocated)->mainNotice) = 0;
        (*(int*)&((GtlHeapObject*)allocated)->localNotice) = 0;
        (*(int*)&((GtlHeapObject*)allocated)->mainMemory) = 0;
        (*(int*)&((GtlHeapObject*)allocated)->mainArgument) = 0;
        (*(int*)&((GtlHeapObject*)allocated)->localMemory) = 0;
        (*(int*)&((GtlHeapObject*)allocated)->localArgument) = 0;
        ((GtlHeapObject*)allocated)->unk_24 = 0; // Reserved heap-object state, initialized independently of flags.
        ((GtlHeapObject*)allocated)->flags = 0;
    }
    *(void**)out = allocated;
    if (!allocated) {
        return 0;
    }
    gtl_HeapObject_InitializeAllocators(allocated, self);
    return 1;
}

} // extern "C"
