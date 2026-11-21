// Memory / heap reservation and the engine's list/fixed-vector allocators.
// MemoryManager_InitHeaps consumes OS MEM1/MEM2 arenas; application allocation
// wrappers in memory_heap_init_and_alloc.cpp/gtl_heap_allocators.cpp use the published heap handles.
// Shared layouts: gtl_CListAllocator and gtl_CFixedVectorAllocator; defaults and
// error tables retain their linked definitions (listAllocatorInitDefaults etc.).
// Modder lookup:
// - MEM1 reservation: kMem1ReservedBytes = 0x100000 bytes (1 MiB), subtracted
//   before MEMCreateExpHeapEx. Raising it shrinks the usable MEM1 heap and leaves
//   more unconsumed arena space; MEM2 uses its entire reported arena.
// - Default suballocator: kDefaultAllocatorBytes = 0x10000 bytes, allocated from
//   MEM1 at kDefaultAllocatorAlignment = 128 bytes. The initialization template's
//   arenaBytes must agree with the actual backing size; only arenaAddress is
//   overlaid below. Review both settings before enlarging model/course buffers.
// - Allocation alignment queries: shifts [2,7] encode powers of two, 4..128 bytes.
// - Format conversion: Memory_CopyAndReverseStridedBlocks documents its unusual
//   packed-count endpoint; do not reinterpret it as a conventional element reverse.
// Allocation/free/reallocation mutate allocator-owned lists over borrowed arenas;
// arenas must outlive every payload. No resolution, aspect or frame-rate setting
// lives here. Preserve free-before-reallocate, copy lengths, integer rounding,
// OS arena publication and mutex/registration order. Safe reservation limits
// depend on the caller's OS arena and the game's allocation demand, not this file.
// MWCC shape constraints: file IPA permits analysis of this unit's known callees;
// auto_inline off keeps the public allocator/copy/query boundaries as calls, so
// local optimizer controls below apply to their own bodies and exception frames.
// Neither pragma provides locking; callers serialize list mutation/enumeration.
#pragma ipa file
#include "classes/gtl_CFixedVectorAllocator.h"
#define Asgtl_CFixedVectorAllocator(p) ((gtl_CFixedVectorAllocator*)(p))
#include "classes/gtl_CListAllocator.h"
#define Asgtl_CListAllocator(p) ((gtl_CListAllocator*)(p))
#pragma auto_inline off
// Translation unit 0x80420D68-0x80421FE8 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

// List allocations store a header immediately before the returned payload.
// Fixed-vector allocations keep their headers separately in 32-byte slots.
struct ListFreeBlock {
    unsigned int previous, next, blockCount, inUse;
};
struct VectorFreeBlock {
    unsigned int payloadAddress, previous, next, blockCount, inUse;
    char slotPadding[12];
};
struct AllocatorInitParameters {
    unsigned int reserved, bestFit, blockShift, arenaAddress, arenaBytes;
    unsigned int slotAddress, slotBytes;
};
// Complete fixed-vector receiver extent is 0x2C bytes, aligned for 32-bit words.
// gtl_CFixedVectorAllocator.h describes ONLY the common +00..+27 prefix; it is
// not an allocation-size contract. This complementary view names the final
// +28 arena-byte word written by Initialize. Both views alias ONE allocation.
// Evidence: gtl_heap_allocators.cpp SubObjB::operator new requests kAllocatorObjectBytes
// (0x2C), not sizeof(gtl_CFixedVectorAllocator). Never allocate just that prefix.
struct FixedVectorInitTail { char prefix[0x28]; unsigned int arenaBytes; };
// Sizes and offsets are bytes unless named Blocks/Count. blockShift is log2 of
// blockSize. List block counts include overhead; fixed-vector counts cover payload.
// Caller-owned arenas and the fixed allocator's slot array must outlive the
// allocator; freeing a payload only returns its range to a null-terminated list.
// List payloads begin at header+blockSize; fixed payloads use separate 32-byte
// slots whose inUse bit describes header-slot reuse, not payload ownership.
struct MainMemoryStateView {
    char mutexStorage[0x18];
    char destructorRegistration[12];
    gtl_CListAllocator defaultAllocator;
};
typedef char MainMemoryAllocatorOffset[((int)&((MainMemoryStateView*)0)->defaultAllocator == 0x24) ? 1 : -1];
struct AllocatorIterationState {
    char prefix[0x14];
    unsigned int nextBlock;
};
enum MemoryConstants {
    kMem1Heap = 0,
    kMem2Heap = 1,
    kMinimumAlignmentShift = 2,
    kMaximumAlignmentShift = 7,
    kMem1ReservedBytes = 0x100000, // byte tail excluded from the MEM1 expandable heap
    kDefaultAllocatorBytes = 0x10000, // backing allocation; pair with template arenaBytes
    kDefaultAllocatorAlignment = 0x80, // positive alignment => allocate from heap start
    kFileErrorCount = 1490,
    kFileErrorEntryBytes = 8,
    kFixedVectorSlotBytes = 0x20
};
typedef char VectorFreeBlockSize[(sizeof(VectorFreeBlock) == kFixedVectorSlotBytes) ? 1 : -1];

extern "C" {

void* memcpy(void* dst, const void* src, size_t n);
void* memset(void* dst, int value, size_t n);
extern const char s_unknownFileErrorMessage[];
extern const char s_supportedFileErrorTable[];
extern unsigned int g_lastFileError;
extern unsigned int s_appHeapsInitialized;
extern unsigned int g_mem1ExpHeap;
extern unsigned int g_mem2ExpHeap;
extern char __vt__Q23gtl16IMemoryAllocator[];
extern char __vt__Q23gtl14CListAllocator[];
extern char __vt__Q23gtl21CFixedVectorAllocator[];
void GtlDefaultAllocator_FreeLocked(void*);
unsigned int gtl_CListAllocator_Initialize(void*, void* = 0);
unsigned int gtl_CFixedVectorAllocator_Initialize(void*, void*);
unsigned int gtl_CListAllocator_InsertFreeBlock(void*, unsigned int, unsigned int);
unsigned int gtl_CFixedVectorAllocator_InsertFreeBlock(void*, unsigned int, unsigned int);
unsigned int MEMGetAllocatableSizeForExpHeapEx(void*, unsigned int);
void* OSGetMEM1ArenaHi();
void* OSGetMEM2ArenaHi();
void* OSGetMEM1ArenaLo();
void* OSGetMEM2ArenaLo();
void OSSetMEM1ArenaLo(void*);
void OSSetMEM2ArenaLo(void*);
void* MEMCreateExpHeapEx(void*, unsigned int, int);
void* MEMAllocFromExpHeapEx(void*, unsigned int, int);
extern char s_mainMemoryAllocatorState[];
extern unsigned int g_defaultListAllocator;
extern unsigned int s_defaultListAllocatorArena;
extern unsigned int lbl_805DD640; // Auxiliary memory-manager state reset after heap initialization.
extern unsigned char s_defaultListAllocatorInitialized;
extern unsigned int listAllocatorInitDefaults[];
struct CompareValue {
    char padding_00[4];
    int value;
};

int bsearch(void**, const char*, int, int,
                int (*)(const CompareValue*, const CompareValue*));

// Compare file-error keys for bsearch; preserve the original negative return value of -11.
// Required a/b: readable 8-byte records (ignored word +0, signed code +4),
// naturally word aligned. Returns -11 / 0 / 1 for less / equal / greater;
// no writes, allocation, or retained pointers. Table ordering uses signed codes.
int FileError_CompareCode(const CompareValue* a, const CompareValue* b) {
    if (a->value < b->value)
        return -11;
    return a->value > b->value;
}

struct ObjectVTable {
    char padding_00[0x1c];
    unsigned int (*getValue)(void*);
};

struct ObjectWithValue {
    char padding_00[4];
    ObjectVTable* vtable;
    char padding_08[0x1c];
    unsigned int value;
};

struct AllocatorReferencePrefix { char pad[4]; };
class AllocatorSizeCalls : public AllocatorReferencePrefix { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18(); virtual unsigned int v1C();
};
// Return arena bytes minus the total size of the free blocks.
unsigned int gtl_CListAllocator_GetUsedBytes(ObjectWithValue* p) {
    unsigned int value = ((AllocatorSizeCalls*)p)->v1C();
    return p->value - value;
}

// Publish the error only if its code appears in the supported file-error table.
// p is a 32-bit error CODE transported as void*, never dereferenced or retained
// as an object. Search 1490 eight-byte records, comparing their signed +4 code.
// The temporary key's first word is unused by CompareCode; only +4 is initialized.
// Supported codes replace g_lastFileError; unsupported codes leave it unchanged.
// No return status or synchronization; serialize callers that need a stable error.
void File_SetLastError(void* p) {
    void* local[2];
    local[1] = p;
    if (bsearch(local, s_supportedFileErrorTable, kFileErrorCount, kFileErrorEntryBytes, FileError_CompareCode))
        g_lastFileError = (unsigned int)p;
}

struct ListNode {
    char padding_00[4];
    ListNode* next;
    unsigned int value;
};

struct FreeListView {
    char padding_00[0x0c];
    unsigned int shift;
    char padding_10[0x10];
    ListNode* first;
};

struct AllocationListsView {
    char padding_00[0x0c];
    unsigned int shift;
    char padding_10[0x0c];
    ListNode* first;
    ListNode* second;
};

struct ReductionState {
    unsigned int* result;
    unsigned int shift;
};

struct AllocationInfo {
    unsigned int payloadAddress;
    unsigned int arenaOffset;
    unsigned int blockBytes;
};
// Selector API table (do not assume these three entry points reject alike):
// API                            which=0       which=1       other
// ListAllocator_vfunc_9 enumerate allocated     free          free
// ListAllocator_vfunc_8 count     allocated     free          returns 0
// AppHeap_QueryMaxAllocatableSize MEM1          MEM2          fails, out unchanged
enum AllocatorListSelector { kAllocatedBlocks = 0, kFreeBlocks = 1 };

struct VectorBlockLinks {
    unsigned int value;
    VectorBlockLinks* prev;
    VectorBlockLinks* next;
    unsigned int blockCount;
};

struct ListAllocatorIterationView {
    char padding_00[0x0c];
    unsigned int blockShift;
    unsigned int blockSize;
    unsigned int iterationCursor;
    unsigned int arenaAddress;
    unsigned int allocatedHead;
    unsigned int freeHead;
};

// Insert a header in address order and coalesce adjacent free ranges in both directions.
unsigned int gtl_CListAllocator_InsertFreeBlock(void* owner, unsigned int head, unsigned int item) {
    unsigned int prev = 0;
    unsigned int next = head;
    while (next) {
        if (item <= next)
            break;
        prev = next;
        next = ((ListFreeBlock*)next)->next;
    }
    int hasNext = (next != 0);
    ((ListFreeBlock*)item)->next = next;
    ((ListFreeBlock*)item)->previous = prev;
    if (hasNext) {
        ((ListFreeBlock*)next)->previous = item;
        unsigned int size;
        unsigned int count;
        size = Asgtl_CListAllocator(owner)->blockSize;
        count = ((ListFreeBlock*)item)->blockCount;
        unsigned int end = item + count * size;
        if (end == next) {
            count += ((ListFreeBlock*)next)->blockCount;
            ((ListFreeBlock*)item)->blockCount = count;
            ((ListFreeBlock*)item)->next = ((ListFreeBlock*)next)->next;
            ((ListFreeBlock*)next)->inUse = 0;
            next = ((ListFreeBlock*)next)->next;
            if (next)
                ((ListFreeBlock*)next)->previous = item;
        }
    }
    if (prev) {
        ((ListFreeBlock*)prev)->next = item;
        unsigned int size;
        unsigned int count;
        size = Asgtl_CListAllocator(owner)->blockSize;
        count = ((ListFreeBlock*)prev)->blockCount;
        unsigned int end = prev + count * size;
        if (end == item) {
            ((ListFreeBlock*)prev)->blockCount = count + ((ListFreeBlock*)item)->blockCount;
            ((ListFreeBlock*)prev)->next = next;
            ((ListFreeBlock*)item)->inUse = 0;
            if (next)
                ((ListFreeBlock*)next)->previous = prev;
        }
        return head;
    }
    return item;
}

// Insert a slot in address order, coalesce contiguous payload ranges, and release redundant slots.
// Ordering compares slot addresses, not payload addresses. Coalescing separately
// compares payload endpoints; changing the sort key would change behavior.
unsigned int gtl_CFixedVectorAllocator_InsertFreeBlock(void* owner, unsigned int head, unsigned int item) {
    unsigned int next = head;
    unsigned int prev = 0;
    while (next) {
        if (item <= next)
            break;
        prev = next;
        next = ((VectorFreeBlock*)next)->next;
    }
    ((VectorFreeBlock*)item)->next = next;
    ((VectorFreeBlock*)item)->previous = prev;
    if (next) {
        ((VectorFreeBlock*)next)->previous = item;
        unsigned int size = Asgtl_CFixedVectorAllocator(owner)->blockSize;
        unsigned int count = ((VectorFreeBlock*)item)->blockCount;
        if (((VectorFreeBlock*)item)->payloadAddress + count * size == ((VectorFreeBlock*)next)->payloadAddress) {
            ((VectorFreeBlock*)item)->blockCount = count + ((VectorFreeBlock*)next)->blockCount;
            ((VectorFreeBlock*)item)->next = ((VectorFreeBlock*)next)->next;
            ((VectorFreeBlock*)next)->inUse = 0;
            next = ((VectorFreeBlock*)next)->next;
            if (next)
                ((VectorFreeBlock*)next)->previous = item;
        }
    }
    if (prev) {
        ((VectorFreeBlock*)prev)->next = item;
        unsigned int size = Asgtl_CFixedVectorAllocator(owner)->blockSize;
        unsigned int count = ((VectorFreeBlock*)prev)->blockCount;
        if (((VectorFreeBlock*)prev)->payloadAddress + count * size == ((VectorFreeBlock*)item)->payloadAddress) {
            ((VectorFreeBlock*)prev)->blockCount = count + ((VectorFreeBlock*)item)->blockCount;
            ((VectorFreeBlock*)prev)->next = next;
            ((VectorFreeBlock*)item)->inUse = 0;
            if (next)
                ((VectorFreeBlock*)next)->previous = prev;
        }
        return head;
    }
    return item;
}

// Initialize the separate header-slot array and one free range covering the rounded-up arena.
// Parameters are borrowed. arenaBytes rounds UP, so backing storage must cover
// the rounded extent. slotBytes supplies complete 32-byte headers. Returns 0 for
// absent parameters or zero blocks/slots; fields already written are not rolled back.
// dst requires the FULL 0x2C-byte receiver described by FixedVectorInitTail,
// not merely the 0x28-byte shared prefix. source is a seven-word configuration;
// keep it disjoint from dst and the arena/slot storage written below. blockShift
// must permit 1u<<shift and the rounded-byte sum without unsigned overflow.
unsigned int gtl_CFixedVectorAllocator_Initialize(void* dst, void* source) {
    if (!source)
        return 0;
    gtl_CFixedVectorAllocator* allocator = (gtl_CFixedVectorAllocator*)dst;
    AllocatorInitParameters* parameters = (AllocatorInitParameters*)source;
    unsigned int one = 1;
    unsigned int zero = 0;
    (*(unsigned int*)&allocator->slots) = parameters->slotAddress;
    (*(unsigned int*)&allocator->allocatedHead) = zero;
    allocator->freeHead = parameters->slotAddress;
    unsigned int slotCount = parameters->slotBytes >> 5;
    allocator->slotCount = slotCount;
    unsigned int arenaBytes = parameters->arenaBytes;
    ((FixedVectorInitTail*)allocator)->arenaBytes = arenaBytes;
    allocator->bestFit = parameters->bestFit;
    unsigned int shift = parameters->blockShift;
    allocator->blockShift = shift;
    unsigned int value = (one << shift) + arenaBytes - 1;
    unsigned int arenaBlocks = value >> shift;
    unsigned int blockBytes = one << parameters->blockShift;
    allocator->blockSize = blockBytes;
    if (!arenaBlocks)
        return 0;
    if (!slotCount)
        return 0;
    ((unsigned int*)(*(unsigned int*)&allocator->slots))[0] = parameters->arenaAddress;
    ((unsigned int*)(*(unsigned int*)&allocator->slots))[1] = zero;
    ((unsigned int*)(*(unsigned int*)&allocator->slots))[2] = zero;
    ((unsigned int*)(*(unsigned int*)&allocator->slots))[3] = arenaBlocks;
    ((unsigned int*)(*(unsigned int*)&allocator->slots))[4] = one;
    unsigned int i = 1;
    while (i < allocator->slotCount) {
        ((unsigned int*)(*(unsigned int*)&allocator->slots))[i * 8 + 4] = zero;
        ++i;
    }
    return one;
}

// Enumerate allocated/free list headers using the stored cursor and return payload/offset/size.
// Set out->payloadAddress=0 to start; keep it nonzero to continue. which==0 selects
// allocations, anything else selects free blocks. The cursor belongs to p, so
// interleaved enumerations overwrite each other. Returns 1 per item, 0 at end or
// for a null output. blockBytes includes overhead, not just usable payload bytes.
// out is borrowed writable 12-byte AllocationInfo, word aligned, not inside the
// allocator/arena headers. On 1 all three words are valid and iterationCursor
// advances. On 0 both output and cursor are UNCHANGED (including stale last item);
// don't use payloadAddress as a Boolean end marker. Start again with zero after
// any list mutation or selector change. Concurrent/interleaved walks are invalid.
// For a FREE item payloadAddress is only a diagnostic range address, not a live
// allocation to dereference/free. arenaOffset names HEADER offset, while
// payloadAddress is HEADER+blockSize. blockBytes includes the header blocks;
// consumers must subtract overheadBlocks*blockSize for payload capacity.
unsigned int gtl_CListAllocator_GetNextBlockInfo(ListAllocatorIterationView* p, AllocationInfo* out, int which) {
    if (!out)
        return 0;
    ListFreeBlock* selected;
    if (which == kAllocatedBlocks)
        selected = (ListFreeBlock*)p->allocatedHead;
    else
        selected = (ListFreeBlock*)p->freeHead;
    if (!selected)
        return 0;
    ListFreeBlock* item;
    unsigned int first = out->payloadAddress; // caller passes zero to begin enumeration
    if (first == 0) {
        item = selected;
    } else {
        item = (ListFreeBlock*)p->iterationCursor;
        if (!item)
            return 0;
    }
    out->payloadAddress = (unsigned int)item + p->blockSize;
    out->arenaOffset = (unsigned int)item - p->arenaAddress;
    out->blockBytes = item->blockCount << p->blockShift;
    p->iterationCursor = item->next;
    return 1;
}

struct LinkNode {
    LinkNode* prev;
    LinkNode* next;
};

// Unlink an allocated payload's preceding header and coalesce it into the free list.
// item must be a live payload from p, or null (returns 0). A foreign pointer is
// not validated. Successful free invalidates the payload and returns 1.
unsigned int gtl_CListAllocator_FreeBlock(ListAllocatorIterationView* p, VectorBlockLinks* item) {
    if (!item)
        return 0;
    unsigned int head = p->allocatedHead;
    LinkNode* node = (LinkNode*)((char*)item - p->blockSize);
    if (node->next)
        node->next->prev = node->prev;
    if (!node->prev)
        head = (unsigned int)node->next;
    else
        node->prev->next = node->next;
    p->allocatedHead = head;
    p->freeHead = gtl_CListAllocator_InsertFreeBlock(p, p->freeHead, (unsigned int)node);
    return 1;
}

// Find the payload's header slot, unlink it, and coalesce it into the free list.
// value must be null or a live allocation from p. The search assumes membership
// and has no end check for an unknown non-null pointer.
unsigned int gtl_CFixedVectorAllocator_FreeBlock(void* p, void* value) {
    if (!value)
        return 0;
    VectorBlockLinks* first = (*(VectorBlockLinks**)&Asgtl_CFixedVectorAllocator(p)->allocatedHead);
    VectorBlockLinks* item = first;
    for (;;) {
        if (item->value == (unsigned int)value)
            break;
        item = item->next;
    }
    if (item->next)
        item->next->prev = item->prev;
    if (!item->prev)
        first = item->next;
    else
        item->prev->next = item->next;
    (*(VectorBlockLinks**)&Asgtl_CFixedVectorAllocator(p)->allocatedHead) = first;
    Asgtl_CFixedVectorAllocator(p)->freeHead = gtl_CFixedVectorAllocator_InsertFreeBlock(p, Asgtl_CFixedVectorAllocator(p)->freeHead, (unsigned int)item);
    return 1;
}

// Round the arena down to whole blocks, calculate header overhead, and install one free header.
// source is borrowed and required for success. Fields are written before the
// size check returns 0 for an arena too small to hold header plus payload.
unsigned int gtl_CListAllocator_Initialize(void* p, void* source) {
    if (!source)
        return 0;
    gtl_CListAllocator* allocator = (gtl_CListAllocator*)p;
    AllocatorInitParameters* parameters = (AllocatorInitParameters*)source;
    unsigned int* head;
    unsigned int zero;
    unsigned int one;
    unsigned int workingCount;
    unsigned int workingBytes;
    unsigned int roundedBytes;
    unsigned int alignmentShift;
    unsigned int blockShiftCopy;
    unsigned int headerBlocks;
    head = (unsigned int*)parameters->arenaAddress;
    zero = 0;
    one = 1;
    allocator->arenaAddress = (unsigned int)head;
    (*(unsigned int*)&allocator->allocatedHead) = zero;
    workingCount = parameters->arenaAddress;
    (*(unsigned int*)&allocator->freeHead) = workingCount;
    workingCount = parameters->blockShift;
    workingBytes = parameters->arenaBytes;
    roundedBytes = one << workingCount;
    workingCount = roundedBytes - 1;
    roundedBytes = workingBytes & ~workingCount;
    allocator->arenaSize = roundedBytes;
    workingCount = parameters->bestFit;
    allocator->bestFitEnabled = workingCount;
    alignmentShift = parameters->blockShift;
    workingBytes = one << alignmentShift;
    allocator->blockShift = alignmentShift;
    roundedBytes = workingBytes + roundedBytes;
    workingCount = roundedBytes - 1;
    blockShiftCopy = parameters->blockShift;
    workingBytes = workingBytes + 15;
    headerBlocks = workingBytes >> alignmentShift;
    workingCount = workingCount >> alignmentShift;
    roundedBytes = one << blockShiftCopy;
    allocator->overheadBlocks = headerBlocks;
    allocator->blockSize = roundedBytes;
    if (workingCount <= headerBlocks)
        return 0;
    head[0] = zero;
    unsigned int* headerWords = (unsigned int*)allocator->arenaAddress;
    headerWords[1] = zero;
    headerWords = (unsigned int*)allocator->arenaAddress;
    headerWords[2] = workingCount;
    return 1;
}

// Query MEM1/MEM2 capacity at a power-of-two alignment from four through 128 bytes.
unsigned int AppHeap_QueryMaxAllocatableSize(int which, unsigned int shift, void* out) {
    if (!out)
        return 0;
    if (!s_appHeapsInitialized)
        return 0;
    if (shift < kMinimumAlignmentShift || shift > kMaximumAlignmentShift)
        return 0;
    unsigned int result;
    switch (which) {
    case kMem1Heap:
        result = MEMGetAllocatableSizeForExpHeapEx((void*)g_mem1ExpHeap, 1u << shift);
        break;
    case kMem2Heap:
        result = MEMGetAllocatableSizeForExpHeapEx((void*)g_mem2ExpHeap, 1u << shift);
        break;
    default:
        return 0;
    }
    *(unsigned int*)out = result;
    return 1;
}

// Install the fixed-vector allocator interface and initialize it from optional arena parameters.
// Requires fresh 0x2C-byte storage. Return p is an object address, NOT success:
// Initialize's status is discarded. Null source leaves arena/slot/list fields
// uninitialized; zero-block/zero-slot configurations leave partial fields and
// no usable free header. Do not allocate/free/query/walk such an object. Before
// use, provide valid backing storage and require Initialize(p,parameters)==1.
// Failed initialization owns no backing allocation to release: arenas/slots
// stay borrowed. The heap factory (gtl_heap_allocators.cpp) supplies real buffers but tests
// only the returned pointer; that caller adds no initialization-status check.
void* gtl_CFixedVectorAllocator_Ctor(void* p, void* source) {
    gtl_CFixedVectorAllocator* fields = (gtl_CFixedVectorAllocator*)p;
    fields->referenceCount = 1;
    (*(unsigned int*)&fields->vtable) = (unsigned int)__vt__Q23gtl16IMemoryAllocator;
    ((AllocatorIterationState*)fields)->nextBlock = 0;
    if (source) {
        AllocatorInitParameters* src = (AllocatorInitParameters*)source;
        fields->bestFit = src->bestFit;
        fields->blockShift = src->blockShift;
        fields->blockSize = 1 << src->blockShift;
    }
    (*(unsigned int*)&fields->vtable) = (unsigned int)__vt__Q23gtl21CFixedVectorAllocator;
    gtl_CFixedVectorAllocator_Initialize(p, source);
    return p;
}

// Sum free-list block counts after converting them to bytes.
unsigned int gtl_CListAllocator_SumFreeBlockBytes(FreeListView* p) {
    volatile unsigned int result;
    unsigned int shift = p->shift;
    volatile ReductionState stateB;
    volatile ReductionState stateA;
    volatile unsigned int* presult = &result;
    ListNode* node = p->first;
    stateA.result = (unsigned int*)presult;
    stateA.shift = shift;
    result = 0;
    stateB.result = (unsigned int*)presult;
    stateB.shift = shift;
    while (node) {
        *presult += node->value << shift;
        node = node->next;
    }
    return result;
}

// Counting keeps a volatile node/counter and a pointer to that counter. Disable
// propagation/lifetime merging so MWCC retains each reload and loop-carried store.
#pragma opt_propagation off
#pragma opt_lifetimes off
// Count the headers in the requested allocated (0) or free (1) list.
unsigned int gtl_CListAllocator_CountListHeaders(AllocationListsView* p, int which) {
    volatile ListNode* node;
    switch (which) {
    case 0:
        node = p->first;
        break;
    case 1:
        node = p->second;
        break;
    default:
        return 0;
    }
    volatile unsigned int count = 0;
    volatile unsigned int* pcount = &count;
    while (node) {
        ++(*pcount);
        node = node->next;
    }
    return count;
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset

// Largest-range reduction retains two state copies and the address of the chosen
// temporary/result. Disabling propagation/lifetime/common-subexpression folding
// preserves that address selection and the volatile load/store order per node.
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_common_subs off
// Return the largest free-list range in bytes.
unsigned int gtl_CListAllocator_GetLargestFreeRange(const FreeListView* p) {
    unsigned int shift = p->shift;
    unsigned int one = 1;
    volatile unsigned int result;
    volatile unsigned int temporary;
    volatile ListNode* node = p->first;
    struct State { unsigned int* result; unsigned int shift; unsigned int one; };
    volatile State stateB;
    volatile State stateA;
    stateB.result = (unsigned int*)&result;
    stateB.shift = shift;
    stateB.one = one;
    result = 0;
    stateA.result = (unsigned int*)&result;
    stateA.shift = shift;
    stateA.one = one;
    while (node) {
        temporary = node->value << shift;
        volatile unsigned int* chosen;
        if (result < temporary)
            chosen = &temporary;
        else
            chosen = (volatile unsigned int*)&result;
        result = *chosen;
        node = node->next;
    }
    return result;
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma opt_common_subs reset

// Install the list allocator interface and initialize it from optional arena parameters.
// Requires fresh 0x2C-byte storage. Like the fixed-vector constructor above,
// returns p even when Initialize returns 0. Null source leaves list/arena state
// uninitialized; an undersized arena leaves partial fields without a valid free
// header. Only a subsequent successful gtl_CListAllocator_Initialize makes it
// usable for allocation/free/queries. No failure status/rollback is synthesized;
// callers retain backing storage ownership and must check Initialize explicitly.
void* CListAllocator_Ctor(void* p, void* source) {
    gtl_CListAllocator* fields = (gtl_CListAllocator*)p;
    fields->referenceCount = 1;
    (*(unsigned int*)&fields->vtable) = (unsigned int)__vt__Q23gtl16IMemoryAllocator;
    ((AllocatorIterationState*)fields)->nextBlock = 0;
    if (source) {
        fields->bestFitEnabled = ((AllocatorInitParameters*)source)->bestFit;
        fields->blockShift = ((AllocatorInitParameters*)source)->blockShift;
        fields->blockSize = 1 << ((AllocatorInitParameters*)source)->blockShift;
    }
    (*(unsigned int*)&fields->vtable) = (unsigned int)__vt__Q23gtl14CListAllocator;
    gtl_CListAllocator_Initialize(p, source);
    return p;
}

// Invoke the allocator deleter only for a positive deleting-destructor argument.
void* gtl_CListAllocator_dtor(void* p, int deletingFlag) {
    if (p && deletingFlag > 0)
        GtlDefaultAllocator_FreeLocked(p);
    return p;
}

// Invoke the base allocator deleter only for a positive deleting-destructor argument.
void* gtl_IMemoryAllocator_dtor(void* p, int deletingFlag) {
    if (p && deletingFlag > 0)
        GtlDefaultAllocator_FreeLocked(p);
    return p;
}

// Copy bytes with memcpy; source and destination must not overlap.
void* Memory_Copy(void* dst, const void* src, size_t n) {
    return memcpy(dst, src, n);
}

// Copy bytes with memmove, allowing overlapping source and destination.
void* Memory_Move(void* dst, const void* src, size_t n) {
    return memmove(dst, src, n);
}

// Read the last supported file-error code.
// Returns the stored 32-bit code, with no clearing or validation; shared global
// lifetime, no ownership. A rejected SetLastError leaves this previous value.
unsigned int File_GetLastError() {
    return g_lastFileError;
}

// Return the fallback text for an unknown file error.
// Returns borrowed static NUL-terminated text, independent of g_lastFileError;
// no lookup, allocation or mutation. Caller must not free or modify it.
const char* FileError_GetUnknownMessage() {
    return s_unknownFileErrorMessage;
}

// Fill the requested byte range; the byte count precedes the fill value in this wrapper.
void* Memory_Set(void* dst, size_t n, int value) {
    return memset(dst, value, n);
}

// Zero the requested byte range.
void* Memory_Clear(void* p, size_t n) {
    return memset(p, 0, n);
}

struct ReallocBase { char pad[4]; };
class ReallocOwner : public ReallocBase {
public:
    virtual void v08();
    virtual void* alloc(unsigned int, int);
    virtual void v10();
    virtual void free(void*);
};

// Reallocation retains header-size reads BEFORE free, then performs a virtual
// allocation and the original copy length. Keep propagation/CSE/lifetime merging
// off so the freed header is not re-read to derive those lengths. lmw/stmw fixes
// the nonvolatile register save/restore shape across the free/alloc/move calls.
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
// Resize a payload via free/allocate/move, retrying its old payload size if allocation fails.
// oldPayload must be live and non-null. This frees it BEFORE allocating, and
// copies requestedBytes on success even when growing. The fallback copies the old
// payload length; there is no second-allocation null check or preserving-on-failure
// realloc guarantee. Keep these existing lengths when modifying the allocator.
unsigned int gtl_CListAllocator_Reallocate(void* owner, void* oldPayload, unsigned int requestedBytes) {
    unsigned int total;
    unsigned int blockSize;
    unsigned int shift;
    unsigned int overheadBlocks;
    unsigned int oldBlockBytes;
    unsigned int overheadBytes;
    shift = Asgtl_CListAllocator(owner)->blockShift;
    blockSize = Asgtl_CListAllocator(owner)->blockSize;
    unsigned int roundingBlockBytes = 1u << shift;
    overheadBlocks = Asgtl_CListAllocator(owner)->overheadBlocks;
    total = requestedBytes + roundingBlockBytes;
    void* base = (char*)oldPayload - blockSize;
    unsigned int oldSize = ((ListFreeBlock*)base)->blockCount;
    unsigned int requestedBlocks = (total - 1) >> shift;
    overheadBytes = overheadBlocks << shift;
    unsigned int newBlockCount = overheadBlocks + requestedBlocks;
    oldBlockBytes = oldSize << shift;
    if (newBlockCount == oldSize) {
        return (unsigned int)oldPayload;
    }
    ((ReallocOwner*)owner)->free(oldPayload);
    void* newPayload = ((ReallocOwner*)owner)->alloc(requestedBytes, 0);
    if (!newPayload) {
        unsigned int oldPayloadBytes = oldBlockBytes - overheadBytes;
        newPayload = ((ReallocOwner*)owner)->alloc(oldPayloadBytes, 0);
        Memory_Move(newPayload, oldPayload, oldPayloadBytes);
    } else {
        Memory_Move(newPayload, oldPayload, requestedBytes);
    }
    return (unsigned int)newPayload;
}
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

// Fixed-vector resize likewise preserves the slot-search result across free and
// fallback allocation; these controls keep saved counts/copy lengths separate.
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Find and resize a payload via its separate slot; retry the old size if allocation fails.
// Null payload returns 0. For a NONNULL payload the allocated chain MUST be
// nonempty: allocatedHead is dereferenced before a null-head check. An unmatched
// payload returns 0 only after traversing a valid nonempty chain to its end.
// A size change frees before allocating;
// copy lengths and the unchecked fallback follow the list resize contract above.
unsigned int gtl_CFixedVectorAllocator_ReallocPayload(ReallocOwner* owner, void* oldPayload, unsigned int requestedBytes) {
    if (!oldPayload)
        return 0;
    void* node = Asgtl_CFixedVectorAllocator(owner)->allocatedHead;
    do {
        if ((void*)((VectorFreeBlock*)node)->payloadAddress == oldPayload) {
            break;
        } else {
            void* next = (void*)((VectorFreeBlock*)node)->next;
            if (!next)
                return 0;
            else
                node = next;
        }
    } while (1);
    unsigned int shift = Asgtl_CFixedVectorAllocator(owner)->blockShift;
    unsigned int roundingBlockBytes = 1u << shift;
    unsigned int oldBlockCount = ((VectorFreeBlock*)node)->blockCount;
    unsigned int total = requestedBytes + roundingBlockBytes;
    unsigned int requestedBlocks = (total - 1) >> shift;
    unsigned int oldBlockBytes = oldBlockCount << shift;
    if (requestedBlocks == oldBlockCount) {
        return (unsigned int)oldPayload;
    }
    owner->free(oldPayload);
    void* newPayload = owner->alloc(requestedBytes, 0);
    if (!newPayload) {
        newPayload = owner->alloc(oldBlockBytes, 0);
        Memory_Move(newPayload, oldPayload, oldBlockBytes);
    } else {
        Memory_Move(newPayload, oldPayload, requestedBytes);
    }
    return (unsigned int)newPayload;
}
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

// Startup keeps nonvolatile OS-arena/heap/template values across publication
// calls in the original multi-register frame; lmw/stmw changes prologue shape.
#pragma use_lmw_stmw on
// Reserve MEM1's tail, consume MEM2, and publish the default allocator; preserve creation/publication order.
// Startup-only global mutation, with no rollback or null/underflow checks. Requires
// MEM1 arena bytes > kMem1ReservedBytes, successful heap creation and enough MEM1
// space for the default arena. OS arena lows advance even though errors are not
// reported. Repeated invocation would recreate heaps: the guard only protects
// the embedded allocator's construction/registration, not this entire operation.
void MemoryManager_InitHeaps() {
    // Phase 1: consume the OS arenas, reserving the final MEM1 megabyte.
    char* base = s_mainMemoryAllocatorState;
    void* mem1lo = OSGetMEM1ArenaLo();
    void* mem1hi = OSGetMEM1ArenaHi();
    void* mem2lo = OSGetMEM2ArenaLo();
    void* mem2hi = OSGetMEM2ArenaHi();
    unsigned int mem1size = (unsigned int)mem1hi - (unsigned int)mem1lo;
    unsigned int mem2size = (unsigned int)mem2hi - (unsigned int)mem2lo;
    unsigned int mem1HeapBytes = mem1size - kMem1ReservedBytes;
    void* mem1Heap = MEMCreateExpHeapEx(mem1lo, mem1HeapBytes, 0);
    g_mem1ExpHeap = (unsigned int)mem1Heap;
    void* mem2Heap = MEMCreateExpHeapEx(mem2lo, mem2size, 0);
    g_mem2ExpHeap = (unsigned int)mem2Heap;
    OSSetMEM1ArenaLo((char*)mem1lo + mem1HeapBytes);
    OSSetMEM2ArenaLo((char*)mem2lo + mem2size);
    // Phase 2: reserve the default list arena and copy its initialization options.
    void* defaultArena = MEMAllocFromExpHeapEx((void*)g_mem1ExpHeap, kDefaultAllocatorBytes, kDefaultAllocatorAlignment);
    unsigned int initParameters[7];
    unsigned char allocatorInitializedByte = s_defaultListAllocatorInitialized;
    int allocatorWasInitialized = (signed char)allocatorInitializedByte;
    s_defaultListAllocatorArena = (unsigned int)defaultArena;
    unsigned int t = listAllocatorInitDefaults[3];
    initParameters[0] = listAllocatorInitDefaults[0];
    initParameters[1] = listAllocatorInitDefaults[1];
    initParameters[2] = listAllocatorInitDefaults[2];
    initParameters[3] = t;
    initParameters[4] = listAllocatorInitDefaults[4];
    initParameters[5] = listAllocatorInitDefaults[5];
    initParameters[6] = listAllocatorInitDefaults[6];
    initParameters[3] = s_defaultListAllocatorArena;
    // Phase 3: register the singleton once, initialize it, and publish heap readiness.
    if (allocatorWasInitialized == 0) {
        CListAllocator_Ctor(base + (int)&((MainMemoryStateView*)0)->defaultAllocator, 0);
        __register_global_object(base + (int)&((MainMemoryStateView*)0)->defaultAllocator, (void*)gtl_CListAllocator_dtor, base + (int)&((MainMemoryStateView*)0)->destructorRegistration);
        s_defaultListAllocatorInitialized = 1;
    }
    gtl_CListAllocator_Initialize(base + (int)&((MainMemoryStateView*)0)->defaultAllocator, initParameters);
    unsigned int referenceCount = *(unsigned int*)(base + (int)&((MainMemoryStateView*)0)->defaultAllocator);
    g_defaultListAllocator = (unsigned int)(base + (int)&((MainMemoryStateView*)0)->defaultAllocator);
    OSInitMutex((OSMutex*)base);
    *(unsigned int*)(base + (int)&((MainMemoryStateView*)0)->defaultAllocator) = referenceCount + 1;
    s_appHeapsInitialized = 1;
    lbl_805DD640 = 0;
}
#pragma use_lmw_stmw reset

// Strided reverse retains separate byte cursors and width-specific endpoint
// decrements. Disable strength reduction/lifetime merging to preserve their
// unusual packed-count arithmetic, rather than forming a conventional reverse.
#pragma use_lmw_stmw on
#pragma opt_strength_reduction off
#pragma opt_lifetimes off
// Copy strided blocks, then reverse supported unit widths using the original packed-count endpoint.
// Worked example (width 2, packedCount 6, one row): 12 bytes are copied; the reversal then swaps
// the 2-byte units at byte offsets 0 and 4 and stops (begin 2 is not below end 2), so bytes
// 2..3 and 6..11 stay as copied. Width 1 with packedCount 6 reverses all six bytes. Width 4 with
// packedCount 6 does a single swap of the (overlapping) 4-byte windows at offsets 0 and 2.
// That width-4/count-6 arithmetic example is NOT a valid aligned Wii input:
// base+2 is unaligned for the word load/store even when base is aligned. For
// widths 2/4/8, callers must align EVERY accessed window, including
// rowBegin+packedCount-unitBytes; aligned base/stride alone is insufficient.
// packedLayout low 16 bits = unit width, high 16 bits = packedCount. Each row
// copies unitBytes*packedCount bytes at the SAME rowStride in both buffers, then
// reverses units inside [rowBegin,rowBegin+packedCount). Widths 1/2/4/8 reverse;
// other widths only copy. This unusual endpoint is existing behavior. Callers
// provide non-overlapping, sufficiently sized and aligned storage for every row.
void Memory_CopyAndReverseStridedBlocks(void* destination, void* source, unsigned int packedLayout, unsigned int rowStride, unsigned int rowCount) {
    unsigned int packedCount;
    unsigned int unitBytes;
    unsigned int copyBytes;
    unsigned int rowIndex = 0;
    if (rowCount != 0) {
        unitBytes = packedLayout & 0xFFFF;
        packedCount = packedLayout >> 16;
        copyBytes = unitBytes * packedCount;
        unsigned int offset = 0;
        for (; rowIndex < rowCount; rowIndex++, offset += rowStride) {
            char* begin = (char*)destination + offset;
            memcpy(begin, (char*)source + offset, copyBytes);
            switch (unitBytes) {
            case 0:
            case 3:
            case 5:
            case 6:
            case 7:
                break;
            case 1: {
                char* end = begin + packedCount;
                if (begin != end) {
                    while (begin < --end) {
                        char t = *begin;
                        *begin = *end;
                        begin++;
                        *end = t;
                    }
                }
                break;
            }
            case 2: {
                // The original uses the high half as a byte endpoint here, not a unit count.
                char* end = begin + packedCount;
                if (begin != end) {
                    while (begin < (end -= 2)) {
                        unsigned short t = *(unsigned short*)begin;
                        *(unsigned short*)begin = *(unsigned short*)end;
                        begin += 2;
                        *(unsigned short*)end = t;
                    }
                }
                break;
            }
            case 4: {
                char* end = begin + packedCount;
                if (begin != end) {
                    while (begin < (end -= 4)) {
                        unsigned int t = *(unsigned int*)begin;
                        *(unsigned int*)begin = *(unsigned int*)end;
                        begin += 4;
                        *(unsigned int*)end = t;
                    }
                }
                break;
            }
            case 8: {
                char* end = begin + packedCount;
                if (begin != end) {
                    while (begin < (end -= 8)) {
                        unsigned int t0 = *(unsigned int*)begin;
                        unsigned int t1 = *(unsigned int*)(begin + 4);
                        unsigned int u0 = *(unsigned int*)end;
                        unsigned int u1 = *(unsigned int*)(end + 4);
                        *(unsigned int*)(begin + 4) = u1;
                        *(unsigned int*)begin = u0;
                        begin += 8;
                        *(unsigned int*)(end + 4) = t1;
                        *(unsigned int*)end = t0;
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }
}
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset

// Header at the start of every free or allocated range of the list allocator.
// `count` is the range length in allocator blocks (including overheadBlocks of bookkeeping); the payload handed to the
// caller starts one block (blockSize bytes) after the header. The same prev/next links serve the
// free list (freeHead) and the allocated list (allocatedHead).
struct ListBlock {
    ListBlock* prev;
    ListBlock* next;
    unsigned int count;
};

// Scan state of gtl_CListAllocator_AllocateBlocks. Two volatile copies (ListBlockState `a` and
// ListBlockCursor `b`) hold identical values; the volatile copies exist to reproduce the original
// code and do not mean the state is shared with other threads. `best` (a) is the result slot
// (the chosen free range, 0 if none) and `ref` / b.result point at it; `gap` is the smallest
// excess blocks seen so far when best-fit is enabled.
struct ListBlockState {
    ListBlock* best;
    ListBlock** ref;
    unsigned int bestFitEnabled;
    unsigned int shift;
    unsigned int needed;
    unsigned int gap;
};

struct ListBlockCursor {
    ListBlock** result;
    unsigned int bestFitEnabled;
    unsigned int shift;
    unsigned int needed;
    unsigned int gap;
};

struct ListAllocOwner {
    char padding_00[0x08];
    unsigned int bestFitEnabled;
    unsigned int shift;
    unsigned int esize;
    char padding_14[0x04];
    unsigned int enabled;
    ListBlock* freeHead;
    ListBlock* first;
    char padding_24[0x04];
    unsigned int limit;
};

#pragma opt_common_subs off
#pragma opt_propagation off
// Choose a first/best-fit free range, split usable excess, and link the allocated header.
// owner: the list allocator object; sizeArg: payload bytes wanted. Returns the payload address
// (header address + blockSize) or 0 if the arena is unset, sizeArg is 0, or no free range fits.
// Range sizing: needed = ceil(sizeArg / (1 << blockShift)) + overheadBlocks (blocks).
// Shared allocation contract (also fixed-vector vfunc_1 below): initialized owner
// required, arena/slot bytes borrowed for allocator lifetime; result is a live
// payload to return to this SAME allocator's vfunc_3, never OS/free/delete.
// Set B=1u<<blockShift. Shift must be <32; list B must be >=16 so its header fits
// BEFORE the returned header+B payload. Base addresses must have B alignment for
// B-aligned results (initializer does not align them). Fixed-vector slot array
// must be word aligned and contain slotCount complete 32-byte records.
// No overflow checks: require size <= UINT32_MAX-(B-1), rounded block count plus
// list overhead representable, and all count*B/address+extent calculations fit
// uint32. Requests must fit the actual arena after overhead; no larger alignment
// argument is honored at these two-argument bodies. Query's shifts 2..7 apply to
// OS heap capacity, not validation of this suballocator's initialization shift.
// List initialization rounds arenaBytes DOWN; fixed-vector rounds UP, requiring
// backing storage for that whole extent. Neither may wrap arenaBytes+B-1.
// Free/allocated chains: acyclic, null-terminated, reciprocal previous/next links,
// each head.previous=0; positive block counts; ranges disjoint and within arena.
// List free chain sorted by header address, adjacent ranges coalesced. Fixed free
// chain sorted by SLOT address (not payload), inUse=1 for every linked header;
// unlinked reusable slots have inUse=0. Never edit links/counts or free twice.
// Fixed splits require a spare unused slot even if payload bytes fit; exhaustion
// is an unchecked null dereference. No compaction or safety net fixes corruption.
// Null returns before mutation for unset arena, zero size or no fitting range;
// success mutates both chains. No thread safety; external synchronization needed.
// CSE/propagation off preserves the volatile a/b scan-state copies and the chosen
// header address through split/unlink/publication; merging them changes MWCC code.
void* gtl_CListAllocator_AllocateBlocks(void* owner, unsigned int sizeArg) {
    // Phase 1: round payload bytes to blocks and include header overhead.
    ListBlock* best;
    unsigned int gap;
    unsigned int shiftCopy;
    unsigned int bestFitEnabled;
    ListBlock** ref;
    unsigned int one;
    int zero;
    ListBlock* node;
    unsigned int shift;
    unsigned int limit;
    unsigned int needed;
    if (Asgtl_CListAllocator(owner)->arenaAddress == 0)
        return 0;
    if (sizeArg == 0)
        return 0;
    zero = 0;
    shift = Asgtl_CListAllocator(owner)->blockShift;
    volatile ListBlockCursor b;
    volatile ListBlockState a;
    ref = (ListBlock**)&a.best;
    needed = 1;
    limit = Asgtl_CListAllocator(owner)->overheadBlocks;
    a.ref = ref;
    needed = ((sizeArg + (needed << shift) - 1) >> shift) + limit;
    a.best = (ListBlock*)zero;
    bestFitEnabled = Asgtl_CListAllocator(owner)->bestFitEnabled;
    shiftCopy = Asgtl_CListAllocator(owner)->blockShift;
    node = (*(ListBlock**)&Asgtl_CListAllocator(owner)->freeHead);
    a.bestFitEnabled = bestFitEnabled;
    a.shift = shiftCopy;
    a.needed = needed;
    a.gap = zero;
    a.best = (ListBlock*)zero;
    b.result = ref;
    b.bestFitEnabled = bestFitEnabled;
    b.shift = shiftCopy;
    b.needed = needed;
    b.gap = zero;

    // Scan the free list: first-fit takes the first range with count >= needed; best-fit keeps
    // the range with the smallest (count - needed) and walks the whole list.
    while (node) {
        bool stop;
        unsigned int need = b.needed;
        unsigned int count = node->count;
        if (count < need) {
            stop = 0;
        } else if (b.bestFitEnabled == 0) {
            *b.result = node;
            stop = 1;
        } else {
            if (*b.result == 0 || count - need < b.gap) {
                ListBlock** result;
                unsigned int gap;
                result = b.result;
                gap = count - need;
                b.gap = gap;
                *result = node;
            }
            stop = 0;
        }
        if (stop)
            break;
        node = node->next;
    }

    // The scan above stops at first fit, or retains the smallest excess for best fit.
    best = a.best;
    if (!best)
        return 0;
    // Phase 2: split only if the excess can support another header and payload;
    // otherwise unlink the entire free range.
    gap = best->count - needed;
    if (gap > Asgtl_CListAllocator(owner)->overheadBlocks) {
        unsigned int esize = Asgtl_CListAllocator(owner)->blockSize;
        ListBlock* newnode = (ListBlock*)((char*)best + needed * esize);
        newnode->count = gap;
        newnode->prev = a.best->prev;
        newnode->next = a.best->next;
        a.best->count = needed;
        if (newnode->next)
            newnode->next->prev = newnode;
        if (newnode->prev)
            newnode->prev->next = newnode;
        else
            (*(ListBlock**)&Asgtl_CListAllocator(owner)->freeHead) = newnode;
    } else {
        ListBlock* next = best->next;
        ListBlock* head = (*(ListBlock**)&Asgtl_CListAllocator(owner)->freeHead);
        if (next)
            next->prev = best->prev;
        ListBlock* prev = best->prev;
        if (!prev)
            head = best->next;
        else
            prev->next = best->next;
        (*(ListBlock**)&Asgtl_CListAllocator(owner)->freeHead) = head;
    }

    // Phase 3: prepend the chosen header to allocatedHead and return its payload.
    ListBlock* allocatedHead = (*(ListBlock**)&Asgtl_CListAllocator(owner)->allocatedHead);
    ListBlock* found = a.best;
    found->next = allocatedHead;
    found->prev = 0;
    if (allocatedHead)
        allocatedHead->prev = found;
    (*(ListBlock**)&Asgtl_CListAllocator(owner)->allocatedHead) = found;
    unsigned int esize = Asgtl_CListAllocator(owner)->blockSize;
    ListBlock* allocated = a.best;
    return (char*)allocated + esize;
}
#pragma opt_propagation reset
#pragma opt_common_subs reset

struct BlockNode {
    unsigned int base;
    BlockNode* prev;
    BlockNode* next;
    unsigned int count;
    unsigned int flag;
};

struct FindState {
    BlockNode* best;
    BlockNode** ref;
    unsigned int bestFitEnabled;
    unsigned int shift;
    unsigned int needed;
    unsigned int gap;
};

struct FindCursor {
    BlockNode** result;
    unsigned int bestFitEnabled;
    unsigned int shift;
    unsigned int needed;
    unsigned int gap;
};

#pragma opt_common_subs off
#pragma opt_propagation off
// Choose a first/best-fit range and use an unused header slot to describe any remainder.
// A split requires an unused slot; callers must provision enough because slot
// exhaustion is not checked before dereference. Returns null for zero size,
// absent slot storage, or no suitable range; payloads remain arena-owned.
// Size/alignment/nonoverflow/list invariants: shared allocation contract above
// gtl_CListAllocator_AllocateBlocks. These two optimizer controls retain FindState and
// FindCursor as distinct scan/commit views, including the unused-slot search.
void* gtl_CFixedVectorAllocator_Allocate(void* owner, unsigned int size) {
    // Phase 1: round the request to payload blocks, without list-header overhead.
    unsigned int shiftCopy;
    unsigned int bestFitEnabled;
    BlockNode** ref;
    int zero;
    unsigned int one;
    BlockNode* node;
    unsigned int shift;
    unsigned int needed;
    if ((*(unsigned int*)&Asgtl_CFixedVectorAllocator(owner)->slots) == 0)
        return 0;
    if (size == 0)
        return 0;
    zero = 0;
    shift = Asgtl_CFixedVectorAllocator(owner)->blockShift;
    volatile FindCursor b;
    volatile FindState a;
    a.best = (BlockNode*)zero;
    ref = (BlockNode**)&a.best;
    one = 1;
    needed = (size + (one << shift) - 1) >> shift;
    bestFitEnabled = Asgtl_CFixedVectorAllocator(owner)->bestFit;
    shiftCopy = Asgtl_CFixedVectorAllocator(owner)->blockShift;
    node = (*(BlockNode**)&Asgtl_CFixedVectorAllocator(owner)->freeHead);
    a.ref = ref;
    a.bestFitEnabled = bestFitEnabled;
    a.shift = shiftCopy;
    a.needed = needed;
    a.gap = zero;
    a.best = (BlockNode*)zero;
    b.result = ref;
    b.bestFitEnabled = bestFitEnabled;
    b.shift = shiftCopy;
    b.needed = needed;
    b.gap = zero;
    while (node) {
        int stop;
        unsigned int need = b.needed;
        unsigned int count = node->count;
        if (count < need) {
            stop = 0;
        } else if (b.bestFitEnabled == 0) {
            *b.result = node;
            stop = 1;
        } else {
            if (*b.result == 0 || count - need < b.gap) {
                BlockNode** result;
                unsigned int gap;
                result = b.result;
                gap = count - need;
                b.gap = gap;
                *result = node;
            }
            stop = 0;
        }
        if (stop)
            break;
        node = node->next;
    }
    // Phase 2: the first/best-fit scan is complete; split or unlink the chosen range.
    BlockNode* best = a.best;
    if (!best)
        return 0;
    unsigned int remainder = best->count - needed;
    if (remainder != 0) {
        unsigned int slotCount = Asgtl_CFixedVectorAllocator(owner)->slotCount;
        BlockNode* slot = 0;
        for (unsigned int i = 0; i < slotCount; i++) {
            BlockNode* s = (BlockNode*)((char*)(*(BlockNode**)&Asgtl_CFixedVectorAllocator(owner)->slots) + i * kFixedVectorSlotBytes);
            if (s->flag == 0) {
                slot = s;
                break;
            }
        }
        unsigned int elemSize = Asgtl_CFixedVectorAllocator(owner)->blockSize;
        slot->base = best->base + needed * elemSize;
        slot->prev = a.best->prev;
        slot->next = a.best->next;
        slot->count = a.best->count - needed;
        slot->flag = 1;
        a.best->count = needed;
        if (slot->next)
            slot->next->prev = slot;
        if (slot->prev)
            slot->prev->next = slot;
        else
            (*(BlockNode**)&Asgtl_CFixedVectorAllocator(owner)->freeHead) = slot;
    } else {
        BlockNode* nxt = best->next;
        BlockNode* head = (*(BlockNode**)&Asgtl_CFixedVectorAllocator(owner)->freeHead);
        if (nxt)
            nxt->prev = best->prev;
        BlockNode* prv = best->prev;
        if (!prv)
            head = best->next;
        else
            prv->next = best->next;
        (*(BlockNode**)&Asgtl_CFixedVectorAllocator(owner)->freeHead) = head;
    }
    // Phase 3: prepend the chosen slot to allocatedHead; its base is the payload.
    BlockNode* oldHead = (*(BlockNode**)&Asgtl_CFixedVectorAllocator(owner)->allocatedHead);
    BlockNode* found = a.best;
    found->next = oldHead;
    found->prev = 0;
    if (oldHead)
        oldHead->prev = found;
    (*(BlockNode**)&Asgtl_CFixedVectorAllocator(owner)->allocatedHead) = found;
    return (void*)a.best->base;
}
#pragma opt_propagation reset
#pragma opt_common_subs reset

} // extern "C"
