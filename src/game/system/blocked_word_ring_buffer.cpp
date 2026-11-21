#include <decomp/static_init.h>
#include "game/golfball.h"
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
// Normal destruction remains explicit. This owner handles exceptional exits.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Adopt cleanup responsibility without owning the enclosing object's storage.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Hand destruction back to the explicit normal-exit path.
    void release() { active = false; }
    // Destroy the temporary's contents on exceptional exit.
    ~EH5Cleanup() { if (active) destroy(object, -1); }
};

extern "C" char s_movieSampleFreeQueue[];
extern "C" char s_movieSampleReadyQueue[];
// Translation unit 0x8025BBD0-0x8025D2A4 (26 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

enum RingAndAudioConstants {
    kWordsPerBlock = 256,
    kBlockWordShift = 8,
    kBlockWordMask = kWordsPerBlock - 1,
    kClubCount = 13,
    kRandomClubExcludedGroundType = 2,
    kApplyPartyEffectState = 8,
    kQ15FractionBits = 15,
    kSampleMinimum = -32768,
    kSampleMaximum = 32767,
    kMessageNoWait = 0
};
enum PartyEffectStringOffset {
    kBlockLengthError = 0x7E,
    kLengthErrorType = 0x93,
    kVectorLengthError = 0xCA
};
struct RandomClubEffectView { char baseStorage[0x10]; unsigned int applied; };
struct ClubSelectionView { char baseStorage[0x41]; unsigned char clubIndex; };
struct MovieSampleBuffer {
    unsigned int producerState;
    short* samples;
    unsigned int remainingFrames;
};


struct BlockedRingSizeView;
struct RingDistanceIterator;

void RingBuffer32_AppendWord(void*, const unsigned int&);
void *Container_GetMember8(void *);
void *BlockedWordRing_Init(void *, void *);
int BlockedWordRing_GetMaxWords(void *);
void *BlockedWordRing_GetConsumedWords(void **);
unsigned int BlockedWordRing_GetUsableWordCapacity(BlockedRingSizeView *);
unsigned int RingBuffer32_GetUsableCapacity(void *);
unsigned int RingBuffer32_GetCountWord(void *);
int RingBuffer_IsEmpty(void *);
void RingBuffer32_ReallocateStorage(void *, unsigned int);
void *UnsignedWord_SelectMaxPointer(unsigned int *, unsigned int *);
void RingBuffer_ThrowLengthError(const char *);
void RingBuffer32_ResetCount(void *);
extern char partyGoodsEffectStrings[];
void RingBuffer32_InsertRangeImpl(void *, void *, void *, void *, void *);
void *RingIterator32_CopyWithAllocatorTag(void *, const void *);
int RingIterator32_GetDistanceFromCurrentWord(unsigned int *, const RingDistanceIterator *);
void *RingBuffer32_MakeBeginIterator(void *, void *);
void *RingBuffer32_MakeEndIteratorVolatile(void *, void *);
void *RingIterator32_Copy(void *, void *);
void *RingIterator32_RetreatInPlace(void *, unsigned int);
void *RingIterator32_GetCurrentPointer(void *);
void *RingIterator32_AddOffset(void *, void *, unsigned int);
void *RingIterator32_CopyForward(void *, void *, void *, void *);
void *RingIterator32_CopyWords(void *, void *);
void *RingIterator32_SubtractOffset(void *, void *, unsigned int);
void *RingIterator32_CopyBackward(void *, void *, void *, void *);
void *RingBuffer32_GetCapacityAddressForAllocation(void *);
void *RingBuffer32_InitializeHeader(void *, void *);
void *RingBuffer32_CalculateGrowth(void *, unsigned int);
void *RingBuffer32_AllocateWords(void *, unsigned int, void *);
void *RingBuffer32_GetAllocationWord(void *);
void *RingBuffer32_ConstructWord(void *, void *, void *);
void *RingBuffer32_AppendIteratorRange(void *, unsigned int, void *);
void *RingBuffer32_CopyWrappedRangeToTail(void *, void *, unsigned int);
void *RingBuffer32_ClearCount(void *);
void *Swap_Int32ForRingStorage(void *, void *);
void *Swap_Int32(void *, void *);
void *RingBuffer32_GetCapacityAddress(void *);
void RingBuffer32_Release(void *, int);
void *RingBuffer32_GetCapacityAddressForInsert(void *);
void appFreeUni__FPv(void *);
void __dl__FPv(void *);
void *__nwa__FUl(unsigned int);
void *__nw__FUl(unsigned int);
void __dla__FPv(void *);
void __throw(const char *, void *, void *);
unsigned int strlen(const char *);
char *strcpy(char *, const char *);
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12length_error[];
void __dt__Q23std12length_errorFv();
extern signed char s_uintVectorMaxSizeInitialized;
extern unsigned int s_fourByteVectorMaxElements;
int OSReceiveMessage(const char *, unsigned int *, int);
void OSSendMessage(const char *, int, int);
void *memcpy(void *, const void *, unsigned int);
void *memmove(void *, const void *, unsigned int);
void *memset(void *, int, unsigned int);
void *appMallocDummy__FUl(int);
int GolfBall__Fv();
int InstanceTable_Get();
int rand();
void ClubSelection_SetClubIndex(int, unsigned char);
extern CSharedDoc* g_sharedDoc;
extern unsigned int s_randomClubEffectWeights[13];
extern float kFloat_6_2831855;
extern float s_partyGoodsTwoPi;
extern int s_blockedWordRingMaxWords;
extern signed char s_ringBufferMaxSizeInitialized;
extern unsigned int g_audioMixCurrentBuffer;
extern unsigned int s_audioVolumeQ15;
extern int s_audioMixPreserveStereo;

struct RingCurrentWordView {
    unsigned int *values;
    unsigned int index;
};

// Two levels of storage, both measured in 32-bit words:
//   ordinary ring header = {values, firstIndex, logicalCount, physicalCapacity};
//   blocked owner = {consumedWords, logicalWordCount, ordinary ring of block pointers}.
// Each block holds 256 words. The block ring's last logical entry is a reserved
// slot, so usable capacity is (blockCount - 1) * 256, with zero handled separately.
// Its physical array also retains the ordinary ring's empty/full sentinel space.
// Iterator = {current, storageBegin, logicalOrigin, storageEnd}; end is exclusive.
// wrapOrigin orders wrapped iterators; it is not an unused allocator word.
// Owners free blocks AND the pointer array. Iterators borrow storage and must not
// survive reallocation. Single-wrap advances require offsets within one ring span.

struct RingBeginSource {
    unsigned int *values;
    unsigned int begin;
    unsigned int wrapOrigin;
    unsigned int end;
};

struct RingBeginIterator {
    unsigned int *current;
    unsigned int *begin;
    unsigned int *wrapOrigin;
    unsigned int *end;
};

struct RingLastWordView {
    unsigned int *values;
    unsigned int begin;
    unsigned int current;
    unsigned int size;
};

struct RingMutableIterator {
    unsigned int *current;
    unsigned int *begin;
    unsigned int *wrapOrigin;
    unsigned int *end;
};

struct BlockedRingSizeView {
    unsigned int consumedWords;
    unsigned int logicalWordCount;
    unsigned int storageAddress;
    unsigned int firstBlock;
    unsigned int blockCount;
};

struct RingDistanceIterator {
    unsigned int *current;
    unsigned int *begin;
    unsigned int *wrapOrigin;
    unsigned int *end;
};

struct RingEndIterator {
    unsigned int *current;
    unsigned int *begin;
    unsigned int *wrapOrigin;
    unsigned int *end;
};

struct RingEndSource {
    unsigned int *values;
    unsigned int begin;
    unsigned int index;
    unsigned int size;
};

struct RingRetreatIterator {
    unsigned int *current;
    unsigned int *begin;
    unsigned int *wrapOrigin;
    unsigned int *end;
};

struct BlockedRingOwner { unsigned int consumedWords, logicalWordCount; void **items; unsigned int begin, count, capacity; };

struct ClubChoiceWeights { int v[13]; };

struct RingCopySourceIterator { unsigned int *current, *begin, *wrapOrigin, *end; unsigned char allocatorTag; };
struct RingCopyDestinationIterator { unsigned int *current, *begin, *wrapOrigin, *end; };

void RingIterator32_CopyRangeWrapped(RingCopyDestinationIterator *out, RingCopySourceIterator *a, RingCopySourceIterator *b, RingCopyDestinationIterator *queue);

struct RingIteratorWords { unsigned int *current, *begin, *wrapOrigin, *end; };
struct RingPopFrontView { unsigned int storageAddress, current, remaining, end; };
void RingBuffer32_PushBackWord(void *, const unsigned int &);
unsigned int *RingBuffer_GetCurrentPtr(RingCurrentWordView *);
unsigned int *RingBuffer_GetLastElementPtr(RingLastWordView *);
void RingBuffer32_AdvanceFrontAndDecrementCount(RingPopFrontView *);
void *BlockedWordRing_dtor(BlockedRingOwner *, int);
void RingBuffer32_BuildEndIterator(RingEndIterator *, const RingEndSource *);
void RingIterator32_CopyAndRetreat(RingRetreatIterator *, const RingRetreatIterator *, int);
void RingBuffer32_BuildBeginIterator(RingBeginIterator *, const RingBeginSource *);
void RingBuffer32_InsertRangeUsingLocalTag(void *, unsigned int *, unsigned int *, unsigned int *, unsigned int *);
void RingBuffer32_SwapHeader(RingBeginIterator *, RingBeginIterator *);
void *BlockedWordRing_GetBlockRing(void *);
void *AllocateWordArray(void *, int);
unsigned int RingBuffer32_GetUsableCapacity(void *);
unsigned int RingBuffer32_GetCountWord(void *);
int RingBuffer_IsEmpty(void *);
void RingBuffer32_ReallocateStorage(void *, unsigned int);
void *UnsignedWord_SelectMaxPointer(unsigned int *, unsigned int *);
void RingBuffer_ThrowLengthError(const char *);
void RingBuffer32_ResetCount(void *);
void *BlockedWordRing_Init(void *, void *);

#pragma push
#pragma use_lmw_stmw on
// Ensure enough 256-word blocks, recycling consumed blocks or growing the owning block ring.
void BlockedWordRing_EnsureAdditionalCapacity(void *self, unsigned int additionalWords) {
    RingPopFrontView *blockRing = (RingPopFrontView *)Container_GetMember8(self);
    unsigned int *allocator = (unsigned int *)BlockedWordRing_GetBlockRing(self);
    unsigned int blocksNeeded = (additionalWords >> kBlockWordShift) + ((additionalWords & kBlockWordMask) != 0);
    // Phase 1: move wholly consumed front blocks to the tail without reallocating them.
    while (blocksNeeded != 0 && (unsigned int)BlockedWordRing_GetConsumedWords((void **)self) >= kWordsPerBlock) {
        unsigned int value = *RingBuffer_GetCurrentPtr((RingCurrentWordView *)blockRing);
        RingBuffer32_AdvanceFrontAndDecrementCount(blockRing);
        *RingBuffer_GetLastElementPtr((RingLastWordView *)blockRing) = value;
        unsigned int reservedTailSlot = 0;
        RingBuffer32_PushBackWord(blockRing, reservedTailSlot);
        --blocksNeeded;
        *(unsigned int *)self -= kWordsPerBlock;
    }
    // Phase 2: allocate missing blocks, growing the pointer ring only when necessary.
    if (blocksNeeded != 0) {
        unsigned int wordsNeeded = blocksNeeded << kBlockWordShift;
        if (wordsNeeded > BlockedWordRing_GetMaxWords(self) ||
            BlockedWordRing_GetUsableWordCapacity((BlockedRingSizeView *)self) > BlockedWordRing_GetMaxWords(self) - wordsNeeded)
            RingBuffer_ThrowLengthError(partyGoodsEffectStrings + kBlockLengthError);
        unsigned int capacity = RingBuffer32_GetUsableCapacity(blockRing);
        unsigned int used = RingBuffer32_GetCountWord(blockRing);
        if (blocksNeeded + used <= capacity) {
            while (blocksNeeded != 0) {
                *RingBuffer_GetLastElementPtr((RingLastWordView *)blockRing) = (unsigned int)AllocateWordArray(allocator, kWordsPerBlock);
                unsigned int reservedTailSlot = 0;
                RingBuffer32_PushBackWord(blockRing, reservedTailSlot);
                --blocksNeeded;
            }
        } else {
            BlockedRingOwner replacement;
            BlockedWordRing_Init(&replacement, allocator);
            EH5Cleanup eh5Cleanup21(&replacement, (void (*)(void*, int))BlockedWordRing_dtor);
            RingPopFrontView *newBlockRing = (RingPopFrontView *)Container_GetMember8(&replacement);
            unsigned int used2 = RingBuffer32_GetCountWord(blockRing);
            unsigned int size;
            unsigned int total = blocksNeeded + used2 + 1;
            size = RingBuffer32_GetUsableCapacity(blockRing) * 2;
            void *memory = UnsignedWord_SelectMaxPointer(&size, &total);
            RingBuffer32_ReallocateStorage(newBlockRing, *(unsigned int *)memory);
            {
                unsigned int reservedTailSlot = 0;
                RingBuffer32_PushBackWord(newBlockRing, reservedTailSlot);
            }
            while (blocksNeeded != 0) {
                *RingBuffer_GetLastElementPtr((RingLastWordView *)newBlockRing) = (unsigned int)AllocateWordArray(allocator, kWordsPerBlock);
                unsigned int reservedTailSlot = 0;
                RingBuffer32_PushBackWord(newBlockRing, reservedTailSlot);
                --blocksNeeded;
            }
            if (RingBuffer_IsEmpty(blockRing) == 0) {
                RingBeginIterator copyIter;
                RingBeginIterator srcIter;
                RingRetreatIterator other;
                RingEndIterator range;
                RingBuffer32_BuildEndIterator(&range, (RingEndSource *)blockRing);
                RingIterator32_CopyAndRetreat((RingRetreatIterator *)&other, (RingRetreatIterator *)&range, 1);
                RingBuffer32_BuildBeginIterator(&srcIter, (RingBeginSource *)blockRing);
                RingBuffer32_BuildBeginIterator(&copyIter, (RingBeginSource *)newBlockRing);
                RingBuffer32_InsertRangeUsingLocalTag(newBlockRing, (unsigned int *)&copyIter, (unsigned int *)&srcIter, (unsigned int *)&other, 0);
            }
            // Commit: pointers now belong to self; reset the old ring to avoid freeing moved blocks.
            RingBuffer32_SwapHeader((RingBeginIterator *)blockRing, (RingBeginIterator *)newBlockRing);
            RingBuffer32_ResetCount(newBlockRing);
            { eh5Cleanup21.release(); BlockedWordRing_dtor(&replacement, -1); }
        }
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Apply the random-club effect once when permitted, choosing among allowed clubs by their weights.
// Eligible clubs are currentClub..12, not all clubs. Their total weight must be
// positive (rand() % total has no zero check). Always returns 1, even when deferred.
int PartyGoods_ApplyWeightedRandomClub(void *effect) {
    if (((RandomClubEffectView*)effect)->applied == 0) {
        void *sharedDoc = (void *)g_sharedDoc;
        void *golfDoc = (void *)((CSharedDoc *)sharedDoc)->golfDoc; // 0x8025BBD0.
        if (((unsigned int)((CGolfDoc *)golfDoc)->gameStateFlags & kApplyPartyEffectState) != 0) {
            int *ball = (int *)GolfBall__Fv();
            if (((CGolfBall*)ball)->m_groundType != kRandomClubExcludedGroundType) {
                int clubIndex = ((ClubSelectionView*)InstanceTable_Get())->clubIndex;
                int total = 0;
                ClubChoiceWeights weights = *(const ClubChoiceWeights *)s_randomClubEffectWeights;
                int i = clubIndex;
                for (; i < kClubCount; ++i) total += weights.v[i];
                int weightedRoll = rand() % total + 1;
                for (i = clubIndex; i < kClubCount; ++i) {
                    weightedRoll -= weights.v[i];
                    if (weightedRoll <= 0) { clubIndex = i; break; }
                }
                ClubSelection_SetClubIndex(InstanceTable_Get(), clubIndex);
                ((RandomClubEffectView*)effect)->applied = 1;
            }
        }
    }
    return 1;
}
#pragma pop

// Copy a wrapped iterator range into a wrapped destination and return the advanced destination iterator.
// Source/destination ranges must be valid and non-overlapping for the forward copy.
// result receives the final destination; first is also advanced between chunks.
void RingIterator32_CopyRangeWrapped(RingCopyDestinationIterator *result, RingCopySourceIterator *first, RingCopySourceIterator *last, RingCopyDestinationIterator *destination) {
    if (first->current != last->current) {
        int offset = last->current - first->current;
        if (last->current >= last->wrapOrigin) {
            if (last->wrapOrigin > first->current) offset -= last->end - last->begin;
        } else if (last->wrapOrigin <= first->current) {
            offset += last->end - last->begin;
        }
        // Chunk at destination end; the inner loop handles source wrap independently.
        int wordsLeft = offset;
        for (;;) {
            unsigned int *destinationPointer = destination->current;
            int destinationContiguous = (int)(destination->end - destinationPointer);
            int *pn = (destinationContiguous < wordsLeft) ? &destinationContiguous : &wordsLeft;
            int copyCount = *pn;

            RingCopySourceIterator tmp = *first;
            int size = (int)(tmp.end - tmp.begin);
            int index = (int)(tmp.current - tmp.begin);
            index = index + copyCount;
            if (index < 0) {
                index += size;
            } else if (index >= size) {
                index -= size;
            }
            tmp.current = tmp.begin + index;

            RingCopySourceIterator cur = tmp;

            unsigned int *sourcePointer = first->current;
            unsigned int *writePointer = destination->current;
            unsigned int *sourceEnd = first->end;
            unsigned int *sourceBegin = first->begin;
            unsigned int *chunkEnd = tmp.current;
            while (sourcePointer != chunkEnd) {
                *writePointer = *sourcePointer;
                ++sourcePointer;
                ++writePointer;
                if (sourcePointer == sourceEnd) sourcePointer = sourceBegin;
            }
            destination->current = writePointer;
            if (destination->current == destination->end) destination->current = destination->begin;
            wordsLeft -= copyCount;
            if (wordsLeft == 0) break;
            *first = cur;
        }
    }
    *result = *destination;
}

// Return signed iterator distance across the ring wrap boundary.
// currentPtr addresses an iterator's current-pointer WORD, not the pointed-to element.
// Call shape: GetDistance((unsigned int*)&first.current, &last) = last-first,
// measured in words. Passing first.current instead reads sample data as an
// address. Both iterators must borrow the same ring and logical wrapOrigin.
// Physical segments in logical order: [wrapOrigin,end), then [begin,wrapOrigin).
// Opposite segments need +/- (end-begin); within one segment subtraction suffices.
int RingIterator32_GetDistanceFromCurrentWord(unsigned int *currentPtr, const RingDistanceIterator *endIterator) {
    unsigned int *endPointer = endIterator->current;
    unsigned int *storageBegin = endIterator->begin;
    unsigned int *startPointer = (unsigned int *)*currentPtr;
    int distance = endPointer - startPointer;
    unsigned int *storageEnd = endIterator->end;
    if (endPointer >= endIterator->wrapOrigin) {
        if (startPointer >= endIterator->wrapOrigin) return distance;
        return distance - (storageEnd - storageBegin);
    }
    if (startPointer < endIterator->wrapOrigin) return distance;
    return distance + (storageEnd - storageBegin);
}

// Build an iterator at the logical end of a ring, wrapping its physical index.
void RingBuffer32_BuildEndIterator(RingEndIterator *out, const RingEndSource *in) {
    unsigned int size = in->size;
    unsigned int index = size ? (in->begin + in->index) % size : 0;
    unsigned int *base = in->values;
    unsigned int begin = in->begin;
    out->current = base + index;
    out->begin = base;
    out->wrapOrigin = base + begin;
    out->end = base + size;
}

// Copy an iterator and move it backward by the requested word count, wrapping once.
void RingIterator32_CopyAndRetreat(RingRetreatIterator *result, const RingRetreatIterator *source, int retreatWords) {
    int negativeOffset = -retreatWords;
    RingRetreatIterator iterator = *source;
    int size = (int)(iterator.end - iterator.begin);
    int index = negativeOffset + (int)(iterator.current - iterator.begin);
    if (index < 0)
        index += size;
    else if (index >= size)
        index -= size;
    iterator.current = iterator.begin + index;
    *result = iterator;
}

// Forward a word append to the ring implementation.
void RingBuffer32_PushBackWord(void* self, const unsigned int& value) {
    RingBuffer32_AppendWord(self, value);
}

// Swap the four ring storage/header words between two owners.
void RingBuffer32_SwapHeader(RingBeginIterator *left, RingBeginIterator *right) {
    unsigned int *savedWord = left->current;
    left->current = right->current;
    right->current = savedWord;
    savedWord = left->begin;
    left->begin = right->begin;
    right->begin = savedWord;
    savedWord = left->wrapOrigin;
    left->wrapOrigin = right->wrapOrigin;
    right->wrapOrigin = savedWord;
    savedWord = left->end;
    left->end = right->end;
    right->end = savedWord;
}

// Build an iterator whose current position and wrap origin are the ring's first element.
void RingBuffer32_BuildBeginIterator(RingBeginIterator *out, const RingBeginSource *in) {
    unsigned int *endp = in->values + in->end;
    unsigned int *cur = in->values + in->begin;
    out->begin = in->values;
    out->current = cur;
    out->wrapOrigin = cur;
    out->end = endp;
}

struct VolatileRingHeader {
    unsigned int *volatile values;
    volatile unsigned int start;
    volatile unsigned int used;
    volatile unsigned int cap;
};

struct RingConstructionIterator {
    unsigned int *cur;
    unsigned int *base;
    unsigned int *wrapOrigin;
    unsigned int *end;
};

struct VolatileRingTemporary {
    unsigned int *volatile values;
    volatile unsigned int a;
    volatile unsigned int b;
    volatile unsigned int c;
};

struct LengthErrorStringControl {
    void *vt;
    unsigned int rc;
    unsigned int controlWord08; // Historical name: initialized to 1, not the diagnostic byte length.
    char *data;
};

struct LengthErrorStringOwner {
    char *ptr;
    char *flag;
};

struct RingLengthError {
    void *vt;
    char *msg;
    void *rep;
};

// Compute the distance between construction iterators using the shared wrap rule.
inline int fn_8025CA88Dist(RingConstructionIterator a, RingConstructionIterator b) {
    return RingIterator32_GetDistanceFromCurrentWord((unsigned int *)&a.cur, (const RingDistanceIterator *)&b);
}

// Advance a construction iterator with a single wrap at either storage boundary.
inline RingConstructionIterator fn_8025CA88Adv(RingConstructionIterator it, int offset) {
    int size = (int)(it.end - it.base);
    int index = offset + (int)(it.cur - it.base);
    if (index < 0)
        index += size;
    else if (index >= size)
        index -= size;
    it.cur = it.base + index;
    return it;
}

// Return a reference to the smaller unsigned count.
inline const unsigned int &fn_8025CA88Min(const unsigned int &a, const unsigned int &b) {
    return a < b ? a : b;
}

// Copy a wrapped iterator range into consecutive destination bytes using memmove.
inline void fn_8025CA88Copy(char *dst, RingConstructionIterator first, RingConstructionIterator last) {
    unsigned int flag = (unsigned int)((unsigned int)last.cur < (unsigned int)last.wrapOrigin);
    while (first.cur != last.cur) {
        unsigned int n;
        if ((unsigned int)((unsigned int)first.cur < (unsigned int)first.wrapOrigin) == flag)
            n = (unsigned int)(last.cur - first.cur);
        else
            n = (unsigned int)(first.end - first.cur);
        unsigned int *mid = first.cur + n;
        unsigned int bytes = (unsigned int)(mid - first.cur) * 4;
        memmove(dst, first.cur, bytes);
        dst += bytes;
        first.cur = mid;
        if (first.cur == first.end)
            first.cur = first.base;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Initialize an empty blocked-ring owner, retaining the original volatile range-construction path.
void *BlockedWordRing_Init(void *arg, void *unusedArg) {
    // Result for this empty-source construction:
    //   owner {consumedWords=0, logicalWordCount=0,
    //          blockRing={values=0, start=0, used=0, cap=0}}.
    // The remaining machinery is the generic range constructor: measure source,
    // validate length, allocate n+1 pointer words, copy up to two wrapped spans.
    // Its volatile source fields prevent erasing those paths in the matching C++.
    // arg is caller-owned header storage; unusedArg is never dereferenced.
    // Phase 1: create empty source/target headers. Volatile reloads preserve the
    // original range-construction path even though the source starts empty.
    VolatileRingTemporary tmp;
    VolatileRingHeader *self = (VolatileRingHeader *)((char *)arg + 8);
    unsigned int *hdr = (unsigned int *)arg;
    RingConstructionIterator first;
    RingConstructionIterator last;

    int n;
    int d;
    unsigned int *dst;
    unsigned int cnt;
    unsigned int space;

    (void)unusedArg;

    tmp.values = 0;
    tmp.a = 0;
    tmp.b = 0;
    tmp.c = 0;
    hdr[0] = 0;
    hdr[1] = 0;
    self->values = 0;
    self->start = 0;
    self->used = 0;
    self->cap = 0;

    unsigned int *v = tmp.values;
    unsigned int begin = tmp.a;
    unsigned int cap = tmp.c;
    unsigned int idx = 0;

    first.cur = v + idx;
    first.base = v;
    first.wrapOrigin = v + begin;
    first.end = v + cap;

    last.cur = v + begin;
    last.base = v;
    last.wrapOrigin = v + begin;
    last.end = v + cap;

    // Phase 2: check the source range length before allocating replacement storage.
    n = fn_8025CA88Dist(last, first);

    if (n != 0) {
        if (s_uintVectorMaxSizeInitialized == 0) {            s_uintVectorMaxSizeInitialized = 1;
            s_fourByteVectorMaxElements = 0x3FFFFFFEu;
        }
        if ((unsigned int)n > s_fourByteVectorMaxElements) {
            unsigned char flag = 0;
            const char *msg = partyGoodsEffectStrings + kVectorLengthError;
            RingLengthError err;
            LengthErrorStringOwner str;
            LengthErrorStringControl *rep;
            char *buf;

            err.vt = __vt__Q23std11logic_error;
            buf = (char *)__nwa__FUl(strlen(msg) + 1);
            err.msg = buf;
            str.ptr = buf;
            str.flag = (char *)&flag;
            rep = (LengthErrorStringControl *)__nw__FUl(0x10);
            if (rep != 0) {
                rep->rc = 1;
                rep->controlWord08 = 1;
                rep->vt = s_sharedCharDeleterVtable;
                rep->data = buf;
            }
            err.rep = rep;
            str.ptr = 0;
            strcpy(buf, msg);
            err.vt = __vt__Q23std12length_error;
            __throw(partyGoodsEffectStrings + kLengthErrorType, &err, (void *)&__dt__Q23std12length_errorFv);
        }
    }

    // Phase 3: reserve one sentinel word, then copy chunks across physical wrap.
    if (n != 0) {
        self->values = (unsigned int *)appMallocDummy__FUl((n + 1) * 4);
        self->cap = (unsigned int)(n + 1);

        if (self->values != 0) {
            dst = self->values + ((self->start + self->used) % (unsigned int)(n + 1));
            d = fn_8025CA88Dist(last, first);
            space = (unsigned int)((self->values + (unsigned int)(n + 1)) - dst);
            cnt = fn_8025CA88Min((const unsigned int &)d, space);
            self->used = self->used + (unsigned int)d;

            RingConstructionIterator target = fn_8025CA88Adv(last, (int)cnt);
            fn_8025CA88Copy((char *)dst, last, target);

            if (d - (int)cnt != 0)
                fn_8025CA88Copy((char *)self->values, fn_8025CA88Adv(last, (int)cnt), first);
        }
    }

    // Phase 4: only the temporary source allocation is released here.
    if (tmp.values != 0) {
        tmp.b = 0;
        appFreeUni__FPv(tmp.values);
    }
    return arg;
}
#pragma pop

// Return the physical address of the ring's final logical element.
unsigned int *RingBuffer_GetLastElementPtr(RingLastWordView *arg) {
    unsigned int index = (arg->begin + arg->current - 1) % arg->size;
    return arg->values + index;
}

// Return the embedded block-ring header after the two leading owner words.
void *BlockedWordRing_GetBlockRing(void *arg) {
    return (char *)arg + 8;
}

// Load the first pointer-sized word from the supplied header.
void *BlockedWordRing_GetConsumedWords(void **arg) {
    return *arg;
}

// Dereference the word pointer stored at an iterator's current position.
void *RingIterator32_Dereference(void ***arg) {
    return **arg;
}

// Allocate the requested number of four-byte words from the dummy application heap.
void *AllocateWordArray(void *arg, int wordCount) {
    return appMallocDummy__FUl(wordCount * 4);
}

// Initialize this unit's stored full-circle angle.
void PartyGoods_InitStaticTwoPi() {
    s_partyGoodsTwoPi = kFloat_6_2831855;
}

// Copy four iterator words and its allocator tag, preserving the destination address.
void *RingIterator32_CopyWithAllocatorTag(void *dst, const void *src) {
    ((unsigned int *)dst)[0] = ((const unsigned int *)src)[0];
    ((unsigned int *)dst)[1] = ((const unsigned int *)src)[1];
    ((unsigned int *)dst)[2] = ((const unsigned int *)src)[2];
    ((unsigned int *)dst)[3] = ((const unsigned int *)src)[3];
    ((unsigned char *)dst)[16] = ((const unsigned char *)src)[16];
    return dst;
}

// Lazily initialize and return the maximum blocked-ring word count.
int BlockedWordRing_GetMaxWords(void *) {
    if (!s_ringBufferMaxSizeInitialized) {
        s_blockedWordRingMaxWords = -512;
        s_ringBufferMaxSizeInitialized = 1;
    }
    return s_blockedWordRingMaxWords;
}

// Pop the logical front by advancing its index with wrap and decrementing the element count.
void RingBuffer32_AdvanceFrontAndDecrementCount(RingPopFrontView *arg) {
    unsigned int current = arg->current + 1;
    arg->current = current;
    if (current == arg->end)
        arg->current = 0;
    arg->remaining = arg->remaining - 1;
}

// Return the address at the ring's current logical index.
unsigned int *RingBuffer_GetCurrentPtr(RingCurrentWordView *arg) {
    return arg->values + arg->index;
}

// Return the word capacity of all blocks except the ring's reserved block, or zero when empty.
unsigned int BlockedWordRing_GetUsableWordCapacity(BlockedRingSizeView *arg) {
    unsigned int value = arg->blockCount;
    unsigned int mask = 0u - (value == 0);
    unsigned int result = (value - 1) << 8;
    return result & ~mask;
}

// Advance a mutable word iterator and wrap at the storage end.
void RingIterator32_AdvanceOne(RingMutableIterator *arg) {
    ++arg->current;
    if (arg->current == arg->end)
        arg->current = arg->begin;
}

#pragma push
#pragma use_lmw_stmw on
// Copy insertion iterators to local storage and dispatch the range-insertion implementation.
void RingBuffer32_InsertRangeUsingLocalTag(void *ring, unsigned int *position, unsigned int *rangeFirst, unsigned int *rangeLast, unsigned int *unusedAllocator) {
    unsigned int positionCopy[4] = {position[0], position[1], position[2], position[3]};
    unsigned int firstCopy[4] = {rangeFirst[0], rangeFirst[1], rangeFirst[2], rangeFirst[3]};
    unsigned int lastCopy[4] = {rangeLast[0], rangeLast[1], rangeLast[2], rangeLast[3]};
    unsigned char allocatorTag = 0;
    RingBuffer32_InsertRangeImpl(ring, positionCopy, firstCopy, lastCopy, &allocatorTag);
    (void)unusedAllocator;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma use_lmw_stmw on

struct RingInsertHeader { unsigned int *begin; unsigned int pos; unsigned int size; };

// Each byte-array slot is an iterator/header passed to the existing ring helpers.
// Byte alignment and slot order are retained for the original stack allocation.
// Stack slots overlap the original helper arguments exactly; do not reorder them.
struct RingInsertScratch {
    // 16-byte slots hold four-word iterators; 20-byte input slots append the
    // allocator tag. replacementStorage holds the complete five-word header;
    // offsets 4 and 8 are its position and size. valueWord stages one inserted word.
    char valueWord[8];
    char oldBegin[16], oldEnd[16], destinationCopy[16], copyResult[16];
    char backwardDestination[16], backwardEnd[16], backwardBegin[16], backwardResult[16];
    char newEnd[16], forwardDestination[16], forwardEnd[16], forwardBegin[16], forwardResult[16];
    char savedBegin[16], destination[16];
    char inputEndCopy[20], inputBeginCopy[20], inputEnd[20], inputBegin[20];
    char replacementStorage[20];
    char tailPadding[4];
};
typedef char RingInsertScratchSize[(sizeof(RingInsertScratch) == 0x160) ? 1 : -1];

// Insert an iterator range, shifting the shorter side in place or allocating and swapping replacement storage.
// Arguments borrow valid iterators from their respective rings. insertPosition
// belongs to self; first/last delimit a forward range with nonnegative distance.
// Source storage must remain valid throughout copying; no snapshot is made for
// an overlapping source. The growth path advances first itself as it constructs.
//
// Logical spans (physical wrap does not change this order):
//   before: [prefix P][suffix S]        insertCount = N
//   after:  [prefix P][new N][suffix S]
//   P < S:  retreat begin by N, copy P FORWARD, leave S in place
//   P >= S: extend end by N, copy S BACKWARD, leave P in place
// Both paths leave destination at the gap start, ready for CopyRangeWrapped.
// Growth constructs N at replacement offset P, appends S, prepends P, then
// swaps all four header words. The cleanup guard finally owns the old array.
void RingBuffer32_InsertRangeImpl(void *self, void *insertPosition, void *first, void *last, void *allocatorArgument) {
    RingInsertScratch frame;
    RingInsertHeader *ring = (RingInsertHeader *)self;

    // Phase 1: count inserted words and locate the insertion point in logical order.
    void *pd = RingIterator32_CopyWithAllocatorTag(frame.inputEnd, last);
    void *pc = RingIterator32_CopyWithAllocatorTag(frame.inputBegin, first);
    int insertCount = RingIterator32_GetDistanceFromCurrentWord((unsigned int *)pc, (RingDistanceIterator *)pd);
    if (insertCount) {
        unsigned int *base = ring->begin + ring->pos;
        unsigned int *position = *(unsigned int **)insertPosition;
        unsigned int prefixCount = (unsigned int)(position - base);
        if (position < base)
            prefixCount += *(unsigned int *)RingBuffer32_GetCapacityAddressForInsert(self);
        unsigned int capacity = RingBuffer32_GetUsableCapacity(self);
        if ((unsigned int)insertCount <= capacity && ring->size <= capacity - (unsigned int)insertCount) {
            RingBuffer32_MakeBeginIterator(frame.destination, self);
            unsigned int suffixCount = ring->size - prefixCount;
            // Phase 2a: enough space exists; open a gap by moving the shorter side.
            if (prefixCount < suffixCount) {
                RingIterator32_Copy(frame.savedBegin, frame.destination);
                RingIterator32_RetreatInPlace(frame.destination, (unsigned int)insertCount);
                void *np = RingIterator32_GetCurrentPointer(frame.destination);
                unsigned int nsize = ring->size + (unsigned int)insertCount;
                unsigned int npos = (unsigned int)((unsigned int *)np - ring->begin);
                ring->size = nsize;
                ring->pos = npos;
                if (prefixCount != 0) {
                    void *prefixDestination = RingIterator32_Copy(frame.forwardDestination, frame.destination);
                    RingIterator32_AddOffset(frame.forwardEnd, frame.savedBegin, prefixCount);
                    void *prefixSource = RingIterator32_Copy(frame.forwardBegin, frame.savedBegin);
                    RingIterator32_CopyForward(frame.forwardResult, prefixSource, frame.forwardEnd, prefixDestination);
                    RingIterator32_CopyWords(frame.destination, frame.forwardResult);
                }
            } else {
                ring->size += (unsigned int)insertCount;
                RingBuffer32_MakeEndIteratorVolatile(frame.newEnd, self);
                RingIterator32_CopyWords(frame.destination, frame.newEnd);
                if (suffixCount != 0) {
                    void *suffixDestinationEnd = RingIterator32_Copy(frame.backwardDestination, frame.destination);
                    RingIterator32_SubtractOffset(frame.backwardEnd, frame.destination, (unsigned int)insertCount);
                    RingIterator32_SubtractOffset(frame.backwardBegin, frame.destination, (unsigned int)insertCount + suffixCount);
                    RingIterator32_CopyBackward(frame.backwardResult, frame.backwardBegin, frame.backwardEnd, suffixDestinationEnd);
                    RingIterator32_CopyWords(frame.destination, frame.backwardResult);
                }
                RingIterator32_RetreatInPlace(frame.destination, (unsigned int)insertCount);
            }
            // The gap is now open; copy the caller's range into it with wrap handling.
            void *sourceEnd;
            void *gapBegin = RingIterator32_Copy(frame.destinationCopy, frame.destination);
            sourceEnd = RingIterator32_CopyWithAllocatorTag(frame.inputEndCopy, last);
            void *sourceBegin = RingIterator32_CopyWithAllocatorTag(frame.inputBeginCopy, first);
            RingIterator32_CopyRangeWrapped((RingCopyDestinationIterator *)frame.copyResult, (RingCopySourceIterator *)sourceBegin, (RingCopySourceIterator *)sourceEnd, (RingCopyDestinationIterator *)gapBegin);
        } else {
            // Phase 2b: build replacement storage as [prefix, inserted range, suffix].
            // The guard owns replacement storage until the final header swaps.
            void *pc1 = RingBuffer32_GetCapacityAddressForAllocation(self);
            RingBuffer32_InitializeHeader(frame.replacementStorage, pc1);
            EH5Cleanup eh5Cleanup6(frame.replacementStorage, (void (*)(void*, int))RingBuffer32_Release);
            void *pc2 = RingBuffer32_CalculateGrowth(self, (unsigned int)insertCount);
            RingBuffer32_AllocateWords(frame.replacementStorage, ring->size + (unsigned int)insertCount, pc2);
            unsigned int *ptr = *(unsigned int **)frame.replacementStorage + prefixCount;
            *(unsigned int *)(frame.replacementStorage + 4) = prefixCount;
            while (insertCount != 0) {
                *(void **)frame.valueWord = RingIterator32_Dereference((void ***)first);
                RingBuffer32_ConstructWord(RingBuffer32_GetAllocationWord(frame.replacementStorage), ptr, frame.valueWord);
                ++*(unsigned int *)(frame.replacementStorage + 8);
                --insertCount;
                ++ptr;
                RingIterator32_AdvanceOne((RingMutableIterator *)first);
            }
            RingBuffer32_MakeEndIteratorVolatile(frame.oldEnd, self);
            RingBuffer32_AppendIteratorRange(frame.replacementStorage, (unsigned int)insertPosition, frame.oldEnd);
            RingBuffer32_MakeBeginIterator(frame.oldBegin, self);
            RingBuffer32_CopyWrappedRangeToTail(frame.replacementStorage, frame.oldBegin, (unsigned int)insertPosition);
            // Commit the replacement and let its guard free the former allocation.
            RingBuffer32_ClearCount(self);
            Swap_Int32ForRingStorage(self, frame.replacementStorage);
            Swap_Int32(&ring->pos, frame.replacementStorage + 4);
            Swap_Int32(&ring->size, frame.replacementStorage + 8);
            void *r = RingBuffer32_GetCapacityAddress(frame.replacementStorage);
            Swap_Int32(RingBuffer32_GetCapacityAddressForInsert(self), r);
            { eh5Cleanup6.release(); RingBuffer32_Release(frame.replacementStorage, -1); }
        }
    }
}

#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Free every owned block and its pointer ring, optionally deleting the blocked-ring owner.
void *BlockedWordRing_dtor(BlockedRingOwner *arg, int deleting) {
    if (arg) {
        void **items;
        unsigned int begin;
        unsigned int capacity;
        unsigned int end;
        void **p;
        void **limit;
        void **e;
        arg->logicalWordCount = 0;
        if (arg->count) {
            items = arg->items;
            begin = arg->begin;
            capacity = arg->capacity;
            end = capacity ? (begin + arg->count) % capacity : 0;
            p = items + begin;
            limit = items + capacity;
            e = items + end;
            while (p != e) {
                if (*p) appFreeUni__FPv(*p);
                ++p;
                if (p == limit) p = items;
            }
        }
        if (arg->items) {
            arg->count = 0;
            appFreeUni__FPv(arg->items);
        }
        if (deleting > 0) __dl__FPv(arg);
    }
    return arg;
}
#pragma pop

// Mix queued stereo frames at Q15 volume, optionally fold to mono, saturate and return exhausted buffers.
// frameCount counts L/R pairs of signed 16-bit samples; output needs 4*frameCount
// bytes. input may be null (silence) or the same buffer as output. Q15 gain uses
// 32768 as unity. Only the queued/movie contribution is folded to mono, before
// adding input. Queue messages transfer MovieSampleBuffer ownership, never PCM copies.
void Audio_MixQueuedStereoSamples(short *output, const short *input, unsigned int frameCount) {
    const char *readyQueue = (const char *)((char*)&s_movieSampleReadyQueue);
    const char *freeQueue = (const char *)((char*)&s_movieSampleFreeQueue);
    unsigned int remaining = frameCount;
    int mixToMono = !s_audioMixPreserveStereo;
    /* The original reads the volumeQ15 once before the loop (0x8025D0F0 lwz r27, s_audioVolumeQ15)
       and runs the body at least once: the only loop test is at the bottom (0x8025D25C-0x8025D260),
       so a call with frameCount 0 still fetches a buffer from the ready queue. */
    int volumeQ15 = (int)s_audioVolumeQ15;
    do {
        unsigned int bufferAddress = g_audioMixCurrentBuffer;
        if (!bufferAddress) {
            unsigned int found = 0;
            if (OSReceiveMessage(readyQueue, &found, 0) == 1) bufferAddress = found;
            g_audioMixCurrentBuffer = bufferAddress;
        }
        if (!bufferAddress || !((MovieSampleBuffer*)bufferAddress)->remainingFrames) {
            if (input) memcpy(output, input, remaining * 4);
            else memset(output, 0, remaining * 4);
            return;
        }
        unsigned int framesToMix = ((MovieSampleBuffer*)bufferAddress)->remainingFrames;
        framesToMix = (framesToMix >= remaining) ? remaining : framesToMix;
        short *samples = (short *)((MovieSampleBuffer*)bufferAddress)->samples;
        for (unsigned int i = 0; i < framesToMix; ++i) {
            int leftSample = volumeQ15 * samples[0] >> kQ15FractionBits;
            int rightSample = volumeQ15 * samples[1] >> kQ15FractionBits;
            samples += 2;
            if (mixToMono) leftSample = rightSample = (leftSample + rightSample) >> 1;
            int inputSample = input ? *input++ : 0;
            int mixedSample = leftSample + inputSample; if (mixedSample < kSampleMinimum) mixedSample = kSampleMinimum; if (mixedSample > kSampleMaximum) mixedSample = kSampleMaximum;
            output[0] = (short)mixedSample;
            inputSample = input ? *input++ : 0;
            mixedSample = rightSample + inputSample; if (mixedSample < kSampleMinimum) mixedSample = kSampleMinimum; if (mixedSample > kSampleMaximum) mixedSample = kSampleMaximum;
            output[1] = (short)mixedSample;
            output += 2;
        }
        remaining -= framesToMix;
        ((MovieSampleBuffer*)bufferAddress)->remainingFrames -= framesToMix;
        ((MovieSampleBuffer*)bufferAddress)->samples = samples;
        if (!((MovieSampleBuffer*)bufferAddress)->remainingFrames) {
            /* The original sends the emptied buffer back to the free queue as the message:
               0x8025D238 lwz r4, g_audioMixCurrentBuffer / 0x8025D250 bl OSSendMessage (r3 = s_movieSampleFreeQueue, r5 = 0).
               Sending 0 left the producer (movie_audio_mixing_and_playback.cpp OSReceiveMessage(s_movieSampleFreeQueue, ...)) with a null buffer. */
            OSSendMessage(freeQueue, (int)g_audioMixCurrentBuffer, 0);
            g_audioMixCurrentBuffer = 0;
        }
    } while (remaining);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462300", _ctors_u8025BBD0_0, PartyGoods_InitStaticTwoPi);
#pragma pop
