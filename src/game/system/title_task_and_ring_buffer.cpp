// The title task and its main actor, and the ring buffers it uses for queued task commands and
// iterators.
// Also the copy and advance helpers for those rings and the task command record allocation.

#include <decomp/static_init.h>
#pragma ipa file
#pragma auto_inline off

#include "sdk_protos.h"
#include "classes/MWStringStorage.h"
#include "classes/CTask.h"
#include "classes/CTitleTask.h"
// Construct in caller-provided storage; this placement overload never allocates.
inline void* operator new(size_t, void* p) {
    return p;
}
struct TaskScoreStrings {
    char scoreActor[6], dequeLengthError[21], lengthErrorType[55], circularDequeLengthError[20];
    char reserveLengthError[35], outOfRange[27], outOfRangeType[55], stringLengthError[27];
    char taskType[6], taskMainType[10], dummyActor[6];
};
struct TitleMainActorStrings {
    char empty[1], factoryType[18], actorName[11];
};
enum {
    kStringHeapFlag = 0x80000000u,
    kTaskCommandRecordBytes = 0x2C,
    kTaskCommandRecordsPerBlock = 23,
    kRingWordBytes = 4,
    kReservedRingSlots = 1,
    kTaskCommandMaxSizeSentinel = -0x23,
    kTaskCommandPayloadFirstWord = 3,
    kTaskCommandRecordWords = kTaskCommandRecordBytes / kRingWordBytes,
    kFourByteVectorMaxElements = 0x3ffffffe,
    kTitleActorFactoryOffset = (int)&((TitleMainActorStrings*)0)->factoryType,
    kTitleActorNameOffset = (int)&((TitleMainActorStrings*)0)->actorName
};
extern "C" {
void appFreeUni__FPv(void*);

// Narrow and wide strings share the heap flag and pointer words.
struct StringUnwindCleanup_u80200A14 {
    const MWStringStorage* storage;
    bool active;
    // Guard the string's heap allocation until explicit cleanup takes responsibility.
    explicit StringUnwindCleanup_u80200A14(const void* string)
        : storage((const MWStringStorage*)string), active(true) {}
    // Disarm after ownership has been transferred or explicit cleanup has begun.
    void release() {
        active = false;
    }
    // Release heap storage during unwinding; inline string bytes need no disposal.
    ~StringUnwindCleanup_u80200A14() {
        if (active && (storage->flags & kStringHeapFlag)) {
            appFreeUni__FPv(storage->data);
        }
    }
};

// Local ring contract: [allocation, firstIndex, count, capacity], all counts in
// 32-bit words. One allocation slot is reserved to distinguish full from empty.
// Iterators carry [current, allocationBegin, logicalOrigin, allocationEnd]; the
// origin disambiguates wrapped distances. Advance/retreat helpers wrap once only.
// Representation guide (all headers still store index/count/capacity in WORDS):
//   RingWordIterator32 / Copy*/Insert*/Iter13: pointers to words; subtraction = words.
//   RingByteIterator32 / RingRetreatIterator32: byte pointers; one word step = 4 bytes.
//   RingIteratorAddressWords / RingIteratorWordsAndFlag: integer byte ADDRESSES,
//       not indexes; the optional flag is copied, with no meaning established here.
//   RingEndIterator32: current is a word pointer; other three fields are byte addresses.
//   RingByteStorage32: data is a byte pointer, but header counts remain word counts.
// These views share the same 16-byte prefix. Keep their distinct pointer types:
// choosing a byte/word view changes the scale of +, -, and pointer differences.
// These containers own their allocations; iterator views borrow them and become
// invalid after storage growth/swap. The command-record container embeds its ring at +8.
struct RingWithPayload {
    unsigned int header[4];
    void* payload;
};
struct PointerRingHead {
    void** data;
    unsigned int index;
};
struct RingByteIterator32 {
    unsigned char* current;
    unsigned char* begin;
    unsigned char* origin;
    unsigned char* end;
};
struct RingIndexState {
    unsigned int allocationAddress, currentIndex, remaining, capacity;
};
struct RingIteratorWordsAndFlag {
    unsigned int current, begin, origin, end;
    unsigned char flag;
};
struct RingByteStorage32 {
    unsigned char* data;
    unsigned int index;
    unsigned int count;
    unsigned int capacity;
};
struct RingWordStorage32 {
    unsigned int* data;
    unsigned int firstIndex, count, capacity;
};
struct RingHeaderWords {
    unsigned int allocationAddress, firstIndex, count, capacity;
};
struct RingWordIterator32 {
    unsigned int* current;
    unsigned int* begin;
    unsigned int* origin;
    unsigned int* end;
};
struct RingEndStorage32 {
    unsigned int* data;
    unsigned int index;
    unsigned int count;
    unsigned int capacity;
};
struct RingEndIterator32 {
    unsigned int* current;
    unsigned int begin, origin, end;
};
struct TitleTaskStorage {
    void* vtable;
    void* interfaceVtable04;
    void* interfaceVtable08;
    char reserved[0x394];
    unsigned int state;
};
// Retreat shares the byte iterator representation; arithmetic remains in bytes.
typedef RingByteIterator32 RingRetreatIterator32;
struct RingIteratorAddressWords {
    unsigned int current, begin, origin, end;
};
// Pointer ring of task-command record blocks: each allocated block contains 23 records of 44 bytes.
// recordOffset + extraOffset is measured in RECORDS; the embedded ring counts BLOCKS.
struct TaskScoreBlockRingState {
    unsigned int recordOffset, extraOffset;
    unsigned int* data;
    unsigned int firstBlockIndex, blockCount, blockCapacity;
};
// Alias the observed block layout without renaming the shared type.
typedef TaskScoreBlockRingState TaskCommandBlockRingView;
void RingBuffer32_AppendWord(void*, void*);
void __dt__5CTaskFv(void*, int);
void* appMallocDummy__FUl(unsigned int);
void* IntPair_Set(void*, const char*, void*);
void* ActorFactoryRegistry_GetInstance(void*);
void StringMap_SetByName(void*, void*, const char*);
void* TaskRegistry_GetInstance();
void* TaskRegistry_CreateActorByName(void*, const char*);
void CTask_Create();
void CTaskMain_Create();
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, const char*, const char*, const char*);
void appFreeUni__FPv(void*);
void* TaskActorTable_CreateNamedActor(void*, const char*, const char*, int, void*);
extern const float kFloat_6_2831855;
extern float s_initializedTwoPi_TaskFactories;
extern void* s_taskTypeInfo;
extern void* s_taskMainTypeInfo;
extern void* s_iObjectTypeInfo;
extern void* s_actorTypeInfo;
extern void* g_dummyTaskRegistryActor;
extern char s_taskFactoryClassName;
extern const char s_taskMainFactoryClassName[];
extern const char taskScoreExceptionStrings[];
extern const char s_titleMainActorCreationStrings[];
extern unsigned char s_titleTaskRttiDescriptor;
extern int s_taskScoreRingBufferMaxSize;
extern signed char s_taskScoreRingMaxSizeInitialized;
extern unsigned char __vt__10CTitleTask[];
extern int g_defaultActorSkipMask;

// Initialize title-task interfaces and clear the default actor skip mask.
CTitleTask::CTitleTask() {
    TitleTaskStorage* titleStorage = (TitleTaskStorage*)this;
    ((CTask*)titleStorage)->Init();
    unsigned char* base = __vt__10CTitleTask;
    int zero = 0;
    int one = 1;
    void* interfaceVtable04 = base + 0x10;
    titleStorage->vtable = base;
    void* interfaceVtable08 = base + 0x1C;
    titleStorage->interfaceVtable04 = interfaceVtable04;
    titleStorage->interfaceVtable08 = interfaceVtable08;
    g_defaultActorSkipMask = zero;
    titleStorage->state = one;
}
// Compute signed iterator distance across the ring's allocation boundary, relative to its logical origin.
int RingIterator32_GetSignedDistance(RingWordIterator32* first, RingWordIterator32* last) {
    unsigned int* lastCurrentCopy = last->current;
    unsigned int* lastCurrentAlias = lastCurrentCopy;
    unsigned int* beginCopy = last->begin;
    unsigned int* currentSchedulingCopy = lastCurrentAlias;
    unsigned int* beginAlias = beginCopy;
    unsigned int* beginSchedulingCopy = beginAlias;
    unsigned int* lastCurrent = last->current;

    unsigned int* firstCurrent = first->current;
    int distance = lastCurrent - firstCurrent;
    unsigned int* originCopy = last->origin;
    unsigned int* originAlias = originCopy;
    unsigned int* originSchedulingCopy = originAlias;
    unsigned int* allocationEnd = last->end;
    unsigned int* origin = last->origin;
    unsigned int* allocationBegin = last->begin;
    // Logical-origin comparisons distinguish the two sides of a wrapped range.
    if (lastCurrent < origin) {
        if (firstCurrent < origin)
            return distance;
        return distance + (allocationEnd - allocationBegin);
    }
    if (firstCurrent >= origin)
        return distance;
    return distance - (allocationEnd - allocationBegin);
}
// Build an end iterator from the ring's first index, count, and capacity.
// Empty/zero-capacity storage yields index zero; allocationEnd is one-past capacity.
void RingBuffer32_BuildEndWordIterator(RingEndIterator32* result, RingEndStorage32* ring) {
    int capacityBytes = ring->capacity;
    int remainder;
    if (ring->capacity)
        remainder = (ring->index + ring->count) % ring->capacity;
    else
        remainder = 0;

    remainder = remainder << 2;

    unsigned int data = (unsigned int)ring->data;
    unsigned int firstIndexBytes = ring->index;
    firstIndexBytes = firstIndexBytes << 2;

    capacityBytes = capacityBytes << 2;
    result->current = (unsigned int*)(data + remainder);
    result->begin = data;
    unsigned int firstAddressCopy = data + firstIndexBytes;
    unsigned int firstAddressAlias = firstAddressCopy;
    unsigned int firstAddressSchedulingCopy = firstAddressAlias;
    int endAddress = (int)(data + capacityBytes);
    // The end address uses capacity in bytes.
    result->origin = (firstIndexBytes + data);
    result->end = endAddress;
}

// Forward a single-word append to the shared ring implementation.
void RingBuffer32_AppendWordThunk(void* a, void* b) {
    RingBuffer32_AppendWord(a, b);
}
// Copy all iterator words and its flag with the original volatile read order.
RingIteratorWordsAndFlag* RingIterator32_CopyVolatile(RingIteratorWordsAndFlag* destination,
                                                      volatile RingIteratorWordsAndFlag* source) {
    unsigned int endAddress;
    unsigned int originAddress;
    unsigned int beginAddress;
    unsigned int currentAddress;
    unsigned char auxiliaryFlag;
    currentAddress = source->current;
    beginAddress = source->begin;
    originAddress = source->origin;
    endAddress = source->end;
    auxiliaryFlag = source->flag;
    destination->current = currentAddress;
    destination->begin = beginAddress;
    destination->origin = originAddress;
    destination->end = endAddress;
    destination->flag = auxiliaryFlag;
    return destination;
}
// Consume one ring element, wrapping the read index and reducing the remaining count.
void RingBuffer32_PopFrontIndex(RingIndexState* ring) {
    unsigned int current = ring->currentIndex + 1;
    ring->currentIndex = current;
    if (current == ring->capacity)
        ring->currentIndex = 0;
    ring->remaining -= 1;
}
// Return a borrowed pointer to the last occupied word; requires count>0 and capacity>0.
unsigned int* RingBuffer32_GetBackWord(RingWordStorage32* ring) {
    return ring->data + ((ring->firstIndex + ring->count - 1) % ring->capacity);
}
// Swap all four words of two ring headers.
void RingBuffer32_SwapHeaderWords(RingHeaderWords* left, RingHeaderWords* right) {
    unsigned int word;
    word = left->allocationAddress;
    left->allocationAddress = right->allocationAddress;
    right->allocationAddress = word;
    word = left->firstIndex;
    left->firstIndex = right->firstIndex;
    right->firstIndex = word;
    word = left->count;
    left->count = right->count;
    right->count = word;
    word = left->capacity;
    left->capacity = right->capacity;
    right->capacity = word;
}
// Build a begin iterator with the first occupied word as its logical origin.
void RingBuffer32_BuildBeginByteIterator(RingByteIterator32* iterator, RingByteStorage32* ring) {
    unsigned int end = ring->capacity;
    unsigned int index = ring->index;
    unsigned char* data = ring->data;
    end = end << 2;
    index = index << 2;
    unsigned char* currentPointer = data + index;
    unsigned char* endPointer = data + end;
    iterator->begin = data;
    iterator->current = currentPointer;
    iterator->origin = currentPointer;
    iterator->end = endPointer;
}
// Forward actor destruction notification to the task base.
void CTitleTask::NotifyActorsDestroy(void* task) {
    ((CTask*)task)->NotifyActorsDestroy();
}
// Forward actor loading to the task base.
void CTitleTask::StartActorLoading(void* task) {
    ((CTask*)task)->StartActorLoading();
}
extern void __dl__FPv(void*);
// Destroy the task base and optionally free the title-task storage.
CTitleTask::~CTitleTask() {
    ((void (*)(void*, int))__dt__5CTaskFv)(this, 0);
}

// Return the pointer stored after the four-word ring header.
void* RingStorage_GetTrailingPayload(void* ring) {
    return ((RingWithPayload*)ring)->payload;
}
// Return the ring's allocation pointer.
void* RingBuffer32_GetAllocation(void* a) {
    return *(void**)a;
}
// Allocate storage for the requested number of 44-byte task-command records.
void* TaskCommandRecord_AllocateArray(void* allocator, int count) {
    return appMallocDummy__FUl(count * kTaskCommandRecordBytes);
}

// Adjust an interface pointer back to the containing task before destruction.
void CTask::vfunc_1_ThunkMinus8(void* interfacePointer, int forwarded1) {
    __dt__5CTaskFv((char*)interfacePointer - 8, forwarded1);
}
// Copy an iterator and move it backward by the given number of words, wrapping once.
void RingIterator32_SubtractSignedOffset(RingRetreatIterator32* result, const RingRetreatIterator32* iterator,
                                         int index) {
    int allocationWords = (iterator->end - iterator->begin) / 4;
    int offset = (iterator->current - iterator->begin) / 4;
    int position = -index;
    position = position + offset;
    if (position < 0)
        position = position + allocationWords;
    else if (position >= allocationWords)
        position = position - allocationWords;
    RingRetreatIterator32 tmp = *iterator;
    tmp.current = iterator->begin + position * 4;
    *result = tmp;
}
// Register task actor factories and create the registry's dummy actor.
void Task_RegisterFactories() {
    s_initializedTwoPi_TaskFactories = kFloat_6_2831855;
    void* o1 = ActorFactoryRegistry_GetInstance(IntPair_Set(
        &s_taskTypeInfo, ((const TaskScoreStrings*)taskScoreExceptionStrings)->taskType, &s_iObjectTypeInfo));
    StringMap_SetByName(o1, (void*)CTask_Create, &s_taskFactoryClassName);
    void* o2 = ActorFactoryRegistry_GetInstance(IntPair_Set(
        &s_taskMainTypeInfo, ((const TaskScoreStrings*)taskScoreExceptionStrings)->taskMainType, &s_actorTypeInfo));
    StringMap_SetByName(o2, (void*)CTaskMain_Create, s_taskMainFactoryClassName);
    void* t = TaskRegistry_GetInstance();
    g_dummyTaskRegistryActor =
        TaskRegistry_CreateActorByName(t, ((const TaskScoreStrings*)taskScoreExceptionStrings)->dummyActor);
}
void RingBuffer32_Release(void*, int);
// Insertion and copying use the same word-pointer iterator, without an auxiliary flag.
typedef RingWordIterator32 InsertRingIterator32;
struct InsertRingIteratorCleanup {
    unsigned int* current;
    unsigned int* begin;
    unsigned int* origin;
    unsigned int* end;
    // Run the existing iterator cleanup hook during exception unwinding.
    ~InsertRingIteratorCleanup() {
        RingBuffer32_Release(this, -1);
    }
};
typedef RingWordIterator32 CopyRingIteratorBase;
struct CopyRingIterator32 {
    unsigned int* current;
    unsigned int* begin;
    unsigned int* origin;
    unsigned int* end;
    unsigned char flag;
    // Advance in word units, wrapping once without changing logical origin.
    void advance(int n) {
        int size = end - begin;
        int offset = current - begin;
        int index = n + offset;
        if (index < 0)
            index += size;
        else if (index >= size)
            index -= size;
        current = begin + index;
    }
};
typedef CopyRingIterator32 CopyRingIteratorResult;
typedef RingWordIterator32 CopyRingDestination32;
// Copy the four iterator pointers and the caller's auxiliary flag.
static inline void fn_80200FF4_copy(CopyRingIterator32& d, const CopyRingIterator32& s) {
    d.current = s.current;
    d.begin = s.begin;
    d.origin = s.origin;
    d.end = s.end;
    d.flag = s.flag;
}
// Return an advanced iterator copy, leaving the source iterator unchanged.
static inline CopyRingIterator32 AdvanceIteratorCopy(const CopyRingIterator32& s, int n) {
    CopyRingIterator32 tmp;
    *(CopyRingDestination32*)&tmp = *(const CopyRingDestination32*)&s;
    tmp.flag = s.flag;
    tmp.advance(n);
    return tmp;
}
// Copy a wrapped input range into a wrapped output range and return the final output iterator.
void RingIterator32_CopyRangeToRing(CopyRingIterator32*, CopyRingIterator32*, CopyRingIterator32*,
                                    CopyRingDestination32*);
void* RingBuffer32_GetCapacityAddressForInsert(void*);
unsigned int RingBuffer32_GetUsableCapacity(void*);
void RingBuffer32_MakeBeginIterator(InsertRingIterator32*, void*);
void* RingIterator32_Copy(void*, void*);
void RingIterator32_RetreatInPlace(void*, unsigned int);
void* RingIterator32_GetCurrentPointer(void*);
void RingIterator32_AddOffset(void*, void*, unsigned int);
void RingIterator32_CopyForward(void*, void*, void*, void*);
void RingIterator32_CopyWords(void*, void*);
void RingBuffer32_MakeEndIteratorVolatile(void*, void*);
void RingIterator32_SubtractOffset(void*, void*, unsigned int);
void RingIterator32_CopyBackward(void*, void*, void*, void*);
void* RingBuffer32_GetCapacityAddressForAllocation(void*);
void RingBuffer32_InitializeHeader(void*, void*);
void* RingBuffer32_CalculateGrowth(void*, unsigned int);
void RingBuffer32_AllocateWords(void*, unsigned int, void*);
void* RingBuffer32_GetAllocationWord(void*);
void RingBuffer32_ConstructWord(void*, unsigned int*, void*);
void RingBuffer32_AppendIteratorRange(void*, void*, void*);
void RingBuffer32_CopyWrappedRangeToTail(void*, void*, unsigned int);
void RingBuffer32_ClearCount(void*);
void Swap_Int32ForRingStorage(void*, void*);
void Swap_Int32(void*, void*);
void* RingBuffer32_GetCapacityAddress(void*);
void RingBuffer32_Release(void*, int);
// Dereference the current ring word as a pointer.
void* RingIterator32_ReadPointer(void*);
// Advance a word iterator and wrap at the allocation end.
void RingIterator32_AdvanceOneWord(RingByteIterator32*);
// Insert a ring range by shifting the shorter side, or grow storage and copy all ranges.
void RingBuffer32_InsertIteratorRange(void*, RingIteratorAddressWords*, RingIteratorWordsAndFlag*,
                                      RingIteratorWordsAndFlag*);
#pragma push
#pragma use_lmw_stmw on
// Copy range iterator arguments before forwarding to ring insertion.
void RingBuffer32_InsertRangeFromWordIterators(void* destination, const RingIteratorAddressWords* position,
                                               const RingIteratorAddressWords* rangeBegin,
                                               const RingIteratorAddressWords* rangeEnd) {
    unsigned int rangeEndCurrent = rangeEnd->current;
    unsigned int positionCurrent = position->current;
    unsigned int rangeEndAllocationBegin = rangeEnd->begin;
    unsigned int rangeEndOrigin = rangeEnd->origin;
    unsigned int rangeEndAllocationEnd = rangeEnd->end;
    unsigned int rangeBeginCurrent = rangeBegin->current;
    unsigned int rangeBeginAllocationBegin = rangeBegin->begin;
    unsigned int rangeBeginOrigin = rangeBegin->origin;
    unsigned int rangeBeginAllocationEnd = rangeBegin->end;
    unsigned int positionAllocationBegin = position->begin;
    unsigned int positionOrigin = position->origin;
    unsigned int positionAllocationEnd = position->end;
    RingIteratorWordsAndFlag endCopy;
    RingIteratorWordsAndFlag beginCopy;
    RingIteratorAddressWords positionCopy;
    unsigned char flag = 0;
    endCopy.current = rangeEndCurrent;
    endCopy.begin = rangeEndAllocationBegin;
    endCopy.origin = rangeEndOrigin;
    endCopy.end = rangeEndAllocationEnd;
    beginCopy.current = rangeBeginCurrent;
    beginCopy.begin = rangeBeginAllocationBegin;
    beginCopy.origin = rangeBeginOrigin;
    beginCopy.end = rangeBeginAllocationEnd;
    positionCopy.current = positionCurrent;
    positionCopy.begin = positionAllocationBegin;
    positionCopy.origin = positionOrigin;
    positionCopy.end = positionAllocationEnd;
    flag = 0;
    RingBuffer32_InsertIteratorRange(destination, &positionCopy, &beginCopy, &endCopy);
}
struct InsertRingStorage32 {
    unsigned int* data;
    unsigned int index;
    unsigned int size;
    unsigned int cap;
};
// Keep this EH class name: its emitted destructor is part of the object relocation ABI.
struct Fn80200CA4DqEH {
    unsigned int* data;
    unsigned int index;
    unsigned int size;
    unsigned int cap;
    unsigned int pad[2];
    // Dispose staged storage, or the old allocation after the final header swap.
    ~Fn80200CA4DqEH() {
        RingBuffer32_Release(this, -1);
    }
};
// Publish the replacement header; the caller's staged owner then releases the old allocation.
static inline void PublishStagedRingStorage(void* destination, InsertRingStorage32* ring, Fn80200CA4DqEH& stagedRing) {
    RingBuffer32_ClearCount(destination);
    Swap_Int32ForRingStorage(destination, &stagedRing);
    Swap_Int32(&ring->index, &stagedRing.index);
    Swap_Int32(&ring->size, &stagedRing.size);
    void* stagedCapacityField = RingBuffer32_GetCapacityAddress(&stagedRing);
    Swap_Int32(RingBuffer32_GetCapacityAddressForInsert(destination), stagedCapacityField);
}

// Insert a range by shifting the shorter occupied side, or allocate new storage and transfer all ranges.
void RingBuffer32_InsertIteratorRange(void* destination, RingIteratorAddressWords* position,
                                      RingIteratorWordsAndFlag* rangeBegin, RingIteratorWordsAndFlag* rangeEnd) {
    // Decision map: N=insertedWordCount, P=prefixWordCount, S=suffixWordCount.
    //   N==0: no stores. Spare capacity: P<S shifts front, otherwise shifts back.
    //   Growth: inserted range at P -> old suffix -> old prefix -> swap all four words.
    // Element words are copied without retaining pointees; allocation ownership alone moves.
    // Phase 1: measure the borrowed [rangeBegin, rangeEnd) and the insertion's
    // logical prefix length. Input iterators must stay valid through the copies.
    int insertedWordCount;
    {
        RingIteratorWordsAndFlag second, first;
        RingIteratorWordsAndFlag* endSnapshot = RingIterator32_CopyVolatile(&first, rangeEnd);
        RingIteratorWordsAndFlag* beginSnapshot = RingIterator32_CopyVolatile(&second, rangeBegin);
        insertedWordCount =
            RingIterator32_GetSignedDistance((RingWordIterator32*)beginSnapshot, (RingWordIterator32*)endSnapshot);
    }
    if (!insertedWordCount)
        return;
    InsertRingStorage32* ring = (InsertRingStorage32*)destination;
    unsigned int* insertionPointer = (unsigned int*)position->current;
    int prefixWordCount = (int)(insertionPointer - (ring->data + ring->index));
    if ((unsigned int)insertionPointer < (unsigned int)(ring->data + ring->index))
        prefixWordCount += *(unsigned int*)RingBuffer32_GetCapacityAddressForInsert(destination);
    unsigned int usableCapacity = RingBuffer32_GetUsableCapacity(destination);
    if ((unsigned int)insertedWordCount <= usableCapacity &&
        ring->size <= usableCapacity - (unsigned int)insertedWordCount) {
        // Phase 2a: spare slots suffice. Move only the shorter occupied side;
        // forward copy protects the prefix, backward copy protects the suffix.
        RingIteratorWordsAndFlag inputBeginCopy, inputEndCopy;
        InsertRingIterator32 gap, oldBegin, prefixEnd, prefixBegin, prefixLimit, newBegin, newEnd, suffixBegin,
            suffixFirst, suffixLast, suffixLimit;
        InsertRingIterator32 insertedEnd;
        InsertRingIterator32 insertionBegin;
        RingBuffer32_MakeBeginIterator(&gap, destination);
        unsigned int suffixWordCount = ring->size - (unsigned int)prefixWordCount;
        if ((unsigned int)prefixWordCount < suffixWordCount) {
            // Claim insertedWordCount slots before the old front; suffix stays in place.
            RingIterator32_Copy(&oldBegin, &gap);
            RingIterator32_RetreatInPlace(&gap, (unsigned int)insertedWordCount);
            void* newFirstPointer = RingIterator32_GetCurrentPointer(&gap);
            unsigned int newFirstIndex = (unsigned int*)newFirstPointer - ring->data;
            ring->size = ring->size + (unsigned int)insertedWordCount;
            ring->index = newFirstIndex;
            if (prefixWordCount != 0) {
                void* prefixDestination = RingIterator32_Copy(&newBegin, &gap);
                RingIterator32_AddOffset(&prefixLimit, &oldBegin, (unsigned int)prefixWordCount);
                RingIterator32_CopyForward(&prefixEnd, RingIterator32_Copy(&prefixBegin, &oldBegin), &prefixLimit,
                                           prefixDestination);
                RingIterator32_CopyWords(&gap, &prefixEnd);
            }
        } else {
            // Claim slots at the tail; prefix and logical first index stay fixed.
            ring->size = ring->size + (unsigned int)insertedWordCount;
            RingBuffer32_MakeEndIteratorVolatile(&newEnd, destination);
            RingIterator32_CopyWords(&gap, &newEnd);
            if (suffixWordCount != 0) {
                void* tailDestination = RingIterator32_Copy(&suffixLimit, &gap);
                RingIterator32_SubtractOffset(&suffixLast, &gap, (unsigned int)insertedWordCount);
                RingIterator32_SubtractOffset(&suffixFirst, &gap, (unsigned int)insertedWordCount + suffixWordCount);
                RingIterator32_CopyBackward(&suffixBegin, &suffixFirst, &suffixLast, tailDestination);
                RingIterator32_CopyWords(&gap, &suffixBegin);
            }
            RingIterator32_RetreatInPlace(&gap, (unsigned int)insertedWordCount);
        }
        // Both branches leave gap at the first writable insertion slot.
        RingIteratorWordsAndFlag* inputEnd;
        void* outputBegin = RingIterator32_Copy(&insertionBegin, &gap);
        inputEnd = RingIterator32_CopyVolatile(&inputEndCopy, rangeEnd);
        RingIteratorWordsAndFlag* inputBegin = RingIterator32_CopyVolatile(&inputBeginCopy, rangeBegin);
        RingIterator32_CopyRangeToRing((CopyRingIterator32*)&insertedEnd, (CopyRingIterator32*)inputBegin,
                                       (CopyRingIterator32*)inputEnd, (CopyRingDestination32*)outputBegin);
        return;
    }
    // Phase 2b: stage new storage. Construct inserted words at their final prefixWordCount,
    // append the old suffix, then prepend/copy the old prefix across any wrap.
    // Capacity-address helpers return field addresses, not loaded element counts.
    void* capacityField = RingBuffer32_GetCapacityAddressForAllocation(destination);
    Fn80200CA4DqEH stagedRing;
    RingBuffer32_InitializeHeader(&stagedRing, capacityField);
    void* growthRequest = RingBuffer32_CalculateGrowth(destination, (unsigned int)insertedWordCount);
    RingBuffer32_AllocateWords(&stagedRing, ring->size + (unsigned int)insertedWordCount, growthRequest);
    stagedRing.index = prefixWordCount;
    unsigned int* insertedDestination = stagedRing.data + prefixWordCount;
    while (insertedWordCount != 0) {
        unsigned int value = (unsigned int)RingIterator32_ReadPointer(rangeBegin);
        void* slot = RingBuffer32_GetAllocationWord(&stagedRing);
        RingBuffer32_ConstructWord(slot, insertedDestination, &value);
        --insertedWordCount;
        ++insertedDestination;
        ++stagedRing.size;
        RingIterator32_AdvanceOneWord((RingByteIterator32*)rangeBegin);
    }
    {
        InsertRingIterator32 oldEnd, oldBeginForGrowth;
        RingBuffer32_MakeEndIteratorVolatile(&oldEnd, destination);
        RingBuffer32_AppendIteratorRange(&stagedRing, position, &oldEnd);
        RingBuffer32_MakeBeginIterator(&oldBeginForGrowth, destination);
        RingBuffer32_CopyWrappedRangeToTail(&stagedRing, &oldBeginForGrowth, (unsigned int)position);
    }
    // Phase 3: publish the new header; stagedRing now owns and frees the old allocation.
    PublishStagedRingStorage(destination, ring, stagedRing);
}
#pragma pop
// Return the title-task runtime type descriptor.
void* CTitleTask::GetRTTI() {
    return &s_titleTaskRttiDescriptor;
}
#pragma push
#pragma opt_lifetimes off
// Build a task-command iterator by splitting its logical offset into 23-record blocks.
void TaskCommandDeque_BuildEndRecordIterator(void* result, volatile TaskCommandBlockRingView* ring) {
    volatile RingWordIterator32 blockIterator;
    unsigned int blockCapacityBytes;
    unsigned int* begin;
    unsigned int extraRecordOffset;
    unsigned int baseRecordOffset;
    unsigned int firstBlockIndex;
    unsigned int* end;
    unsigned int logicalRecordOffset;
    unsigned int* current;
    int allocationBlocks;
    int physicalBlockIndex;
    unsigned int blockCount;
    unsigned int* resultCurrent;
    unsigned int blockAddress;
    unsigned int* resultBegin;
    unsigned int* resultOrigin;
    unsigned int* resultEnd;
    // Phase 1: snapshot the pointer-ring allocation and logical record offset.
    blockCapacityBytes = ring->blockCapacity;
    begin = ring->data;
    blockCapacityBytes <<= 2;
    extraRecordOffset = ring->extraOffset;
    baseRecordOffset = ring->recordOffset;
    end = (unsigned int*)((char*)begin + blockCapacityBytes);
    blockIterator.end = end;
    logicalRecordOffset = extraRecordOffset + baseRecordOffset;
    firstBlockIndex = ring->firstBlockIndex;
    current = begin + firstBlockIndex;
    blockIterator.begin = begin;
    allocationBlocks = (int)((char*)end - (char*)begin) / 4;
    // Phase 2: quotient selects a block, wrapping its physical index once.
    physicalBlockIndex = (int)(logicalRecordOffset / kTaskCommandRecordsPerBlock);
    physicalBlockIndex += (int)((char*)current - (char*)begin) / 4;
    blockIterator.current = current;
    blockIterator.origin = current;
    if (physicalBlockIndex < 0)
        physicalBlockIndex += allocationBlocks;
    else if (physicalBlockIndex >= allocationBlocks)
        physicalBlockIndex -= allocationBlocks;
    blockCount = ring->blockCount;
    resultCurrent = blockIterator.begin + physicalBlockIndex;
    resultBegin = blockIterator.begin;
    resultOrigin = blockIterator.origin;
    blockIterator.current = resultCurrent;
    resultEnd = blockIterator.end;
    if (blockCount == 0)
        blockAddress = 0;
    else
        blockAddress = *resultCurrent;
    // Phase 3: publish the borrowed block iterator and byte address within that block.
    // Empty rings use blockAddress=0; retain the original remainder-address arithmetic.
    ((RingWordIterator32*)result)->current = resultCurrent;
    ((RingWordIterator32*)result)->begin = resultBegin;
    ((RingWordIterator32*)result)->origin = resultOrigin;
    ((RingWordIterator32*)result)->end = resultEnd;
    *((unsigned int*)result + 4) =
        blockAddress + (logicalRecordOffset % kTaskCommandRecordsPerBlock) * kTaskCommandRecordBytes;
}
#pragma pop
// Create the title task's main actor from the packed factory/name string.
void CTitleTask::CreateActors(void* task) {
    struct Local {
        unsigned int state;
        unsigned int unused;
        void* data;
        unsigned int pad;
    } local;
    local.state = 0;
    local.unused = 0;
    local.data = 0;
    unsigned char flag;
    char* s = (char*)s_titleMainActorCreationStrings;
    unsigned int n = strlen(s);
    String_ReserveCapacity(&local, n);
    flag = *((unsigned char*)&local - 4);
    String_ReplaceRange(&local, 0, 0, s, s + n, (const char*)&flag);
    StringUnwindCleanup_u80200A14 stringCleanup0(&local);
    void* mainActor =
        TaskActorTable_CreateNamedActor(task, s + kTitleActorFactoryOffset, s + kTitleActorNameOffset, 0, &local);
    ((CTask*)task)->SetMainActor(mainActor);
    if (local.state >> 31) {
        stringCleanup0.release();
        appFreeUni__FPv(local.data);
    }
}
// Storage contract only: the record owns its string key; the handler defines
// the interpretation of all eight payload words. None is proven to be a score.
struct TaskCommandRecordView_u80200A14 {
    MWStringStorage key;
    unsigned int payload[8];
};
typedef char
    TaskCommandRecordView_u80200A14_size[sizeof(TaskCommandRecordView_u80200A14) == kTaskCommandRecordBytes ? 1 : -1];

// Copy a task-command record, deep-copying heap string storage and preserving its remaining words.
// Record layout: +0..+0x0B MWStringStorage (inline bytes or owned heap key),
// +0x0C..+0x2B eight opaque 32-bit payload words, copied without retaining pointers.
// These routines establish no score/hole/player meanings for those words; callers
// must resolve the handler's payload contract before changing them. destination
// is uninitialized storage; the allocator argument carries no data.
void TaskCommandRecord_CopyConstruct(void* allocator, void* destination, const void* source) {
    if (!destination)
        return;
    try {
        unsigned int state = *(const unsigned int*)source;
        if (!(state >> 31)) {
            *(unsigned int*)destination = state;
            *((unsigned int*)destination + 1) = *((const unsigned int*)source + 1);
            *((unsigned int*)destination + 2) = *((const unsigned int*)source + 2);
        } else {
            *(unsigned int*)destination = 0;
            *((unsigned int*)destination + 1) = 0;
            *((unsigned int*)destination + 2) = 0;
            String_ReserveCapacity(destination, *((const unsigned int*)source + 1));
            struct EmptyAllocatorToken {
            } token;
            String_ReplaceRange(destination, 0, 0, (char*)*((const unsigned int*)source + 2),
                                (char*)*((const unsigned int*)source + 2) + *((const unsigned int*)source + 1),
                                (char*)&token);
        }
        // Copy opaque payload words verbatim: ownership applies only to the key above.
        for (int payloadWord = kTaskCommandPayloadFirstWord; payloadWord < kTaskCommandRecordWords; ++payloadWord)
            *((unsigned int*)destination + payloadWord) = *((const unsigned int*)source + payloadWord);
    } catch (...) {
        throw;
    }
}
// Dereference the current ring word as a pointer.
void* RingIterator32_ReadPointer(void* iterator) {
    return **(void***)iterator;
}
// Return the address of the ring's indexed pointer slot.
void* PointerRing_GetHeadSlot(PointerRingHead* ring) {
    return ring->data + ring->index;
}
// Advance one word and wrap at the allocation end.
void RingIterator32_AdvanceOneWord(RingByteIterator32* iterator) {
    iterator->current += kRingWordBytes;
    if (iterator->current == iterator->end)
        iterator->current = iterator->begin;
}
// Copy to contiguous output while wrapping the borrowed source at its allocation end.
static inline void fn_80200FF4_loop(const CopyRingIterator32& f, unsigned int* last, unsigned int* dst) {
    unsigned int* src = f.current;
    while (src != last) {
        *dst = *src;
        src++;
        if (src == f.end)
            src = f.begin;
        dst++;
    }
}
// Copy between wrapped ranges and return the final output iterator.
void RingIterator32_CopyRangeToRing(CopyRingIterator32* out, CopyRingIterator32* source, CopyRingIterator32* sourceEnd,
                                    CopyRingDestination32* destination) {
    unsigned int* logicalOrigin;
    unsigned int* sourceCurrent = source->current;
    unsigned int* sourceLimit = sourceEnd->current;
    // Phase 1: measure the logical input range using its origin to detect wrap.
    if (sourceCurrent != sourceLimit) {
        logicalOrigin = sourceEnd->origin;
        int wordDistance = sourceLimit - sourceCurrent;
        if (sourceLimit >= logicalOrigin) {
            if (sourceCurrent < logicalOrigin) {
                int allocationWords = sourceEnd->end - sourceEnd->begin;
                wordDistance = wordDistance - allocationWords;
            }
        } else if (sourceCurrent >= logicalOrigin) {
            int allocationWords = sourceEnd->end - sourceEnd->begin;
            wordDistance = wordDistance + allocationWords;
        }
        int remaining = wordDistance;
        // Phase 2: limit each chunk to contiguous output space, wrapping input
        // word-by-word. Output wraps only after the complete contiguous chunk.
        do {
            unsigned int available = destination->end - destination->current;
            int amount = available < remaining ? available : remaining;
            CopyRingIteratorResult chunkEnd = AdvanceIteratorCopy(*source, amount);
            unsigned int* destinationWord = destination->current;
            unsigned int* sourceWord = source->current;
            unsigned int* sourceAllocationBegin = source->begin;
            unsigned int* sourceAllocationEnd = source->end;
            unsigned char sourceFlag = chunkEnd.flag;
            while (sourceWord != chunkEnd.current) {
                *destinationWord = *sourceWord;
                sourceWord++;
                if (sourceWord == sourceAllocationEnd)
                    sourceWord = sourceAllocationBegin;
                destinationWord++;
            }
            unsigned int* destinationAfterChunk = destination->current + (amount - 1);
            destinationAfterChunk++;
            destination->current = destinationAfterChunk;
            if (destinationAfterChunk == destination->end)
                destination->current = destination->begin;
            remaining -= amount;
            if (remaining == 0)
                break;
            source->current = chunkEnd.current;
            source->begin = chunkEnd.begin;
            source->origin = chunkEnd.origin;
            source->end = chunkEnd.end;
            source->flag = sourceFlag;
        } while (1);
    }
    // Phase 3: return destination's final borrowed iterator. The input copy is
    // advanced only between chunks, so callers must use out for the final position.
    unsigned int* resultOrigin = destination->origin;
    unsigned int* resultBegin = destination->begin;
    unsigned int* resultEnd = destination->end;
    unsigned int* resultCurrent = destination->current;
    out->current = resultCurrent;
    out->begin = resultBegin;
    out->origin = resultOrigin;
    out->end = resultEnd;
}
// Lazily cache the task-command ring's original maximum-size sentinel.
int TaskCommandDeque_GetMaxSize() {
    if (!s_taskScoreRingMaxSizeInitialized) {
        s_taskScoreRingBufferMaxSize = kTaskCommandMaxSizeSentinel;
        s_taskScoreRingMaxSizeInitialized = 1;
    }
    return s_taskScoreRingBufferMaxSize;
}
extern signed char s_uintVectorMaxSizeInitialized;
extern unsigned int s_fourByteVectorMaxElements;
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned long);
void __dla__FPv(void*);
void __dt__Q23std12length_errorFv();
struct Guard13 {
    char* value;
    unsigned char* flag;
    // Guard an exception's character allocation until the reference block owns it.
    Guard13(char* p, unsigned char* f) : value(p), flag(f) {}
    // Dispose an exception message still owned by this temporary guard.
    ~Guard13() {
        if (value)
            __dla__FPv(value);
    }
};
struct Ring13 {
    unsigned int* data;
    unsigned int index, size, capacity;
};
struct Iter13 {
    unsigned int* current;
    unsigned int* begin;
    unsigned int* origin;
    unsigned int* end;
    // Leave iterator words uninitialized for the original staged assignments.
    Iter13() {}
    // Build a borrowed iterator with ordered volatile pointer stores.
    Iter13(unsigned int* c, unsigned int* b, unsigned int* u, unsigned int* e) {
        volatile Iter13* out = this;
        out->current = c;
        out->begin = b;
        out->origin = u;
        out->end = e;
    }
    // Copy iterator pointers without changing their logical origin.
    Iter13(const Iter13& x) {
        unsigned int* c = x.current;
        unsigned int* b = x.begin;
        unsigned int* u = x.origin;
        unsigned int* e = x.end;
        volatile Iter13* out = this;
        out->current = c;
        out->begin = b;
        out->origin = u;
        out->end = e;
    }
};
// Assign iterator pointer words in the original volatile store order.
inline void set13(Iter13& out, const Iter13& x) {
    volatile Iter13* p = &out;
    p->current = x.current;
    p->begin = x.begin;
    p->origin = x.origin;
    p->end = x.end;
}
struct Error13 {
    void* vt;
    char* str;
    void* ref;
};
struct Frame13 {
    Iter13 source, first, last, next, mid;
    Iter13 lastSecond, nextSecond, fSecond;
    Iter13 midFirst, midCopy, fFirst;
    Error13 error;
    unsigned int padding;
    Iter13 firstCopy, lastCopy;
    Ring13 local;
};
// Measure signed word distance using b's logical origin to account for wrapping.
inline int distance13(const Iter13& first, const Iter13& last) {
    unsigned int* firstCurrent;
    unsigned int* lastCurrent;
    unsigned int* logicalOrigin;
    logicalOrigin = last.origin;
    lastCurrent = last.current;
    firstCurrent = first.current;
    bool lastOnHighSide = lastCurrent >= logicalOrigin;
    int wordDistance = lastCurrent - firstCurrent;
    if (lastOnHighSide) {
        if (firstCurrent < logicalOrigin)
            wordDistance -= last.end - last.begin;
    } else {
        if (firstCurrent >= logicalOrigin)
            wordDistance += last.end - last.begin;
    }
    return wordDistance;
}
// Advance a borrowed iterator by words, with at most one allocation wrap.
inline void advance13(Iter13& iterator, int wordOffset) {
    unsigned int* begin = iterator.begin;
    unsigned int* end = iterator.end;
    int allocationWords = end - begin;
    int physicalIndex = wordOffset + (iterator.current - begin);
    if (physicalIndex < 0)
        physicalIndex += allocationWords;
    else if (physicalIndex >= allocationWords)
        physicalIndex -= allocationWords;
    ((volatile Iter13*)&iterator)->current = ((volatile Iter13*)&iterator)->begin + physicalIndex;
}
// Copy each contiguous source run to output, advancing f until it equals last.
inline void copy13(const Iter13& last, Iter13& source, unsigned int* destinationWords) {
    unsigned int* sourceLimit = last.current;
    unsigned int limitOnHighSide = last.current >= last.origin;
    while (source.current != sourceLimit) {
        int chunkWords;
        unsigned int* current = ((volatile Iter13*)&source)->current;
        if ((unsigned int)(current >= source.origin) == limitOnHighSide)
            chunkWords = sourceLimit - current;
        else
            chunkWords = source.end - current;
        current = ((volatile Iter13*)&source)->current;
        unsigned int* chunkEnd = current + chunkWords;
        unsigned int chunkBytes = ((int)((unsigned int)chunkEnd - (unsigned int)current) / 4) * 4;
        memmove(destinationWords, current, chunkBytes);
        source.current += chunkWords - 1;
        destinationWords = (unsigned int*)((char*)destinationWords + chunkBytes);
        source.current++;
        if (source.current == source.end)
            source.current = source.begin;
    }
}
// Return a wrapped end index, defining zero-capacity end as index zero.
inline unsigned int endIndex13(const Ring13& r, unsigned int cap) {
    if (cap)
        return (r.index + r.size) % cap;
    else
        return 0;
}
#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
void RingContainer_dtor(void*, int);
struct BaseRing13 : Ring13 {
    // Destroy any storage constructed before a later initialization failure.
    ~BaseRing13() {
        RingContainer_dtor(this, -1);
    }
};
struct ConstructRing13 : BaseRing13 {
    inline ConstructRing13(Frame13& frame);
};
// Copy a source ring through the generic wrapped-range construction path.
// Frame13 names the compiler's iterator snapshots: first/last delimit the source,
// mid ends the first output run, next starts its wrapped remainder. *First/*Copy
// members preserve volatile copy order and exception-frame placement, not ownership.
// BaseRing13 owns destination storage if construction throws; Guard13 owns only
// exception-message characters until its shared control block takes them.
inline ConstructRing13::ConstructRing13(Frame13& frame) {
    // Phase 1: snapshot source endpoints and measure their logical distance.
    // InitializeEmpty supplies an all-zero source: sourceWordCount=0 skips every allocation,
    // length-error and copy branch below. Keep the generic path for exact codegen.
    unsigned int* sourceEnd;
    unsigned int* sourceOrigin;
    unsigned int* sourceBegin;
    unsigned int* destinationWords;
    int contiguousWords;
    int remainingWords;
    int sourceWordCount;
    Ring13& sourceRing = frame.local;
    Ring13& destinationRing = *(Ring13*)this;
    unsigned int zero = 0;
    unsigned int index;
    index = endIndex13(sourceRing, zero);
    unsigned int capacity;
    unsigned int* start;
    unsigned int startIndex;
    startIndex = sourceRing.index;
    start = sourceRing.data;
    capacity = ((volatile Ring13*)&sourceRing)->capacity;
    Iter13& last = frame.last;
    Iter13& first = frame.first;
    Iter13& source = frame.source;
    volatile Iter13* iteratorOutput = &last;
    iteratorOutput->current = start + index;
    iteratorOutput->begin = start;
    iteratorOutput->origin = start + startIndex;
    iteratorOutput->end = start + capacity;
    iteratorOutput = &first;
    iteratorOutput->current = last.origin;
    iteratorOutput->begin = last.begin;
    iteratorOutput->origin = last.origin;
    iteratorOutput->end = last.end;
    set13(source, first);
    set13(frame.lastCopy, last);
    set13(frame.firstCopy, first);
    sourceWordCount = distance13(frame.firstCopy, frame.lastCopy);
    if (sourceWordCount) {
        // Phase 2: validate the word count; exception message ownership transfers
        // from Guard13 to its shared control block before __throw.
        if (!s_uintVectorMaxSizeInitialized) {
            s_fourByteVectorMaxElements = kFourByteVectorMaxElements;
            s_uintVectorMaxSizeInitialized = 1;
        }
        if ((unsigned int)sourceWordCount > s_fourByteVectorMaxElements) {
            Error13& error = frame.error;
            unsigned char flag = 0;
            const char* message = ((const TaskScoreStrings*)taskScoreExceptionStrings)->circularDequeLengthError;
            error.vt = __vt__Q23std11logic_error;
            unsigned int messageBytes = strlen(message) + 1;
            error.str = (char*)__nwa__FUl((messageBytes << 1) - messageBytes);
            {
                char* messageCopy = error.str;
                Guard13 guard(messageCopy, &flag);
                struct Ref13 {
                    void* vt;
                    int a, b;
                    char* str;
                };
                Ref13* sharedMessage = (Ref13*)__nw__FUl(16);
                if (sharedMessage) {
                    sharedMessage->a = 1;
                    sharedMessage->b = 1;
                    sharedMessage->vt = s_sharedCharDeleterVtable;
                    sharedMessage->str = messageCopy;
                }
                error.ref = sharedMessage;
                guard.value = 0;
            }
            strcpy(error.str, message);
            error.vt = __vt__Q23std12length_error;
            __throw(((const TaskScoreStrings*)taskScoreExceptionStrings)->lengthErrorType, &error,
                    (void*)__dt__Q23std12length_errorFv);
        }
        // Phase 3: allocate sourceWordCount live words plus one reserved slot, then copy the
        // first contiguous destination run and any remainder after wrapping.
        destinationRing.data =
            (unsigned int*)appMallocDummy__FUl((sourceWordCount + kReservedRingSlots) * kRingWordBytes);
        destinationRing.capacity = sourceWordCount + kReservedRingSlots;
        if (destinationRing.data) {
            destinationWords =
                destinationRing.data + (destinationRing.index + destinationRing.size) % destinationRing.capacity;
            remainingWords = distance13(source, last);
            contiguousWords = destinationRing.data + destinationRing.capacity - destinationWords;
            const int* chosen = remainingWords < contiguousWords ? &remainingWords : &contiguousWords;
            contiguousWords = *chosen;
            sourceBegin = source.begin;
            sourceOrigin = source.origin;
            sourceEnd = source.end;
            destinationRing.size += remainingWords;
            volatile int* copyWordsVolatile = &contiguousWords;
            Iter13& mid = frame.mid;
            volatile Iter13* midOutput = &mid;
            midOutput->current = source.current;
            midOutput->begin = sourceBegin;
            midOutput->origin = sourceOrigin;
            midOutput->end = sourceEnd;
            advance13(mid, *copyWordsVolatile);
            set13(frame.midFirst, mid);
            set13(frame.midCopy, frame.midFirst);
            set13(frame.fFirst, first);
            copy13(frame.midCopy, frame.fFirst, destinationWords);
            remainingWords -= *copyWordsVolatile;
            if (remainingWords) {
                Iter13& next = frame.next;
                volatile Iter13* nextOutput = &next;
                nextOutput->current = source.current;
                nextOutput->begin = sourceBegin;
                nextOutput->origin = sourceOrigin;
                nextOutput->end = sourceEnd;
                advance13(next, *copyWordsVolatile);
                set13(frame.nextSecond, next);
                set13(frame.lastSecond, last);
                set13(frame.fSecond, frame.nextSecond);
                copy13(frame.lastSecond, frame.fSecond, destinationRing.data);
            }
        }
    }
}
// Initialize an empty task-command container and its embedded ring storage.
void* TaskCommandDeque_InitializeEmpty(void* container) {
    // Clear record offsets (+0,+4) and the four-word embedded pointer-ring header
    // (+8..+0x17). ConstructRing13 receives an empty source and owns any new ring.
    Frame13 frame;
    Ring13& local = frame.local;
    volatile Ring13* init = &local;
    unsigned int zero = 0;
    init->capacity = zero;
    init->data = (unsigned int*)zero;
    init->index = zero;
    init->size = zero;
    unsigned int* p = (unsigned int*)container;
    p[0] = zero;
    p[1] = zero;
    p[2] = zero;
    p[3] = zero;
    p[4] = zero;
    p[5] = zero;
    new (p + 2) ConstructRing13(frame);
    if (local.data) {
        local.size -= local.size;
        appFreeUni__FPv(local.data);
    }
    return container;
}
#pragma pop
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. With "#pragma ipa file" the code is generated at
   the end of the file, so force_active must still be on there: a push/pop around the entry drops the flag, the linker
   dead-strips the entry and the initialiser never runs. */
#pragma force_active on
STATIC_INIT_ENTRY_NC(".CRT$XCU804621DC", _ctors_u80200A14_0, Task_RegisterFactories);
