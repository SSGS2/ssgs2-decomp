// Sample history buffers for the pad (PadAccState): constructs and destroys the vectors that
// hold each pad's recent samples, and initializes a player's pad state.

#include <decomp/static_init.h>
#include "classes/PadAccState.h"
typedef unsigned long new_size_t;
// Construct copied history entries in caller-owned vector storage.
inline void* operator new(new_size_t, void* p) {
    return p;
}
// Placement-delete does not free the caller's vector allocation on unwind.
inline void operator delete(void*, void*) {}
extern "C" {

float s_padSampleHistoryTwoPi;
float kFloat_6_2831855 = 6.2831854820251465f;
extern float kFloat_0_UiAnimation;

void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void* __dla__FPv(void*);

// Initialize this unit's full-turn angle cache.
void PadSampleHistory_InitTwoPiCache(void) {
    s_padSampleHistoryTwoPi = kFloat_6_2831855;
}

struct SampleHistoryBuffer {
    void* data;
    volatile int count;
};

// Clear the vector count, free its buffer, and optionally delete the wrapper.
SampleHistoryBuffer* SampleHistoryBuffer_dtor(SampleHistoryBuffer* self, int flags) {
    if (self != 0) {
        if (self != 0 && self != 0 && self != 0 && self->data != 0) {
            volatile int* countPointer = &self->count;
            int oldCount = *countPointer;
            volatile int* snapshotPointer = &oldCount;
            self->count = oldCount - *snapshotPointer;
            appFreeUni__FPv(self->data);
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

enum {
    kVectorLengthErrorTypeOffset = 0x14
};

/* ---- std::length_error-style throw helper (same idiom as elsewhere in the game) ---- */
void* __nw__FUl(unsigned int);
void* __nwa__FUl(unsigned int);
void* appMallocDummy__FUl(unsigned int);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
unsigned strlen(const char*);
char* strcpy(char*, const char*);
void __throw(const char* a, void* b, void (*c)()) __attribute__((noreturn));
void* __dt__Q23std12length_errorFv(void*, short);

extern char __vt__9CAccState[];
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12length_error[];
extern char s_padSampleHistoryVectorVTable[];
extern char s_padAuxiliaryHistoryVectorVTable[];
extern char s_sharedCharDeleterVtable[];
extern char vectorLengthErrorStrings_8026700C[];

struct Ref {
    int vt;
    int c1;
    int c2;
    char* s;
};
struct Holder_u8026700C {
    char* p;
    char* flag;
    // Guard the exception-message allocation before its shared owner is installed.
    Holder_u8026700C(char* b, char* f) {
        p = b;
        flag = f;
    }
    // Release a message buffer whose ownership has not been transferred.
    ~Holder_u8026700C() {
        if (p)
            __dla__FPv(p);
    }
};
struct AllocTag {
    volatile char value;
    char pad[3];
    // Initialize the allocator tag retained by the original template expansion.
    AllocTag() : value(0) {}
};
struct ErrBase {
    void* vt;
    char* buf;
    Ref* ref;
    // Copy exception text and adopt it in a shared character-buffer control block.
    ErrBase(const char* msg0) {
        AllocTag errorAlloc;
        vt = __vt__Q23std11logic_error;
        char done __attribute__((aligned(4))) = 0;
        const char* msg = msg0;
        unsigned messageBytes = strlen(msg) + 1;
        char* b = (char*)__nwa__FUl(messageBytes * 2 - messageBytes);
        buf = b;
        {
            Holder_u8026700C h(b, &done);
            Ref* r = (Ref*)__nw__FUl(16);
            if (r) {
                int table = (int)s_sharedCharDeleterVtable;
                r->c1 = 1;
                r->c2 = 1;
                r->vt = table;
                r->s = b;
            }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, msg);
    }
    // Local wrapper has no extra cleanup beyond the runtime exception machinery.
    ~ErrBase() {}
};
struct Err2 : ErrBase {
    // Select the length-error type and throw the constructed exception.
    Err2(const char* msg) : ErrBase(msg) {
        vt = __vt__Q23std12length_error;
        __throw(vectorLengthErrorStrings_8026700C + kVectorLengthErrorTypeOffset, this,
                (void (*)())__dt__Q23std12length_errorFv);
    }
    // No additional cleanup is implemented by this local length-error wrapper.
    ~Err2() {}
};

/* ---- the two embedded containers ---- */
enum AccStateVectorLimits {
    kInitialAccStateEntryCount = 120,
    kInitialAccStateAuxiliaryWordCount = 10,
    kMaxAccStateEntries = 0x71c71c7,
    kSmallEntryGrowthLimit = 0x25ed097,
    kMediumEntryGrowthLimit = 0x4bda12e,
    kMaxAccStateAuxiliaryWords = 0x3fffffff,
    kSmallAuxiliaryGrowthLimit = 0x15555555,
    kMediumAuxiliaryGrowthLimit = 0x2aaaaaaa,
    kAccStateEntryBytes = 0x24
};

struct Entry {
    int words[9];
}; /* 0x24-byte controller samples: this unit only zeroes and copies them. */
// Sample format is nine opaque 32-bit words, NOT nine established integer
// measurements. This constructor establishes only their zero initial state.
// Do not label axes, timestamps or button words without producer/consumer evidence.
// Both embedded histories own their begin allocations (appMallocDummy/appFreeUni).
// Sizes/capacities count elements; allocation products below count bytes.

// Own newly constructed elements until vector growth commits; retain the type name for unwind relocations.
template <class T> struct TmpBuf {
    T* data;
    unsigned size;
    unsigned capacity;
    unsigned* oldCapacity;
    unsigned remaining;
    // Start a growth transaction; p refers to the destination capacity word.
    TmpBuf(unsigned* p) : data(0), size(0), capacity(0), oldCapacity(p), remaining(0) {}
    // Free uncommitted storage, or the old allocation received during commit.
    ~TmpBuf() {
        if (data != 0) {
            AllocTag alloc;
            size -= size;
            appFreeUni__FPv(data);
        }
    }
};

struct SampleHistoryVector {
    void* vt;          /* 0x00 */
    int currentIndex;  /* 0x04: -1 until the first recorded sample */
    int highestIndex;  /* 0x08: highest initialized history index */
    Entry* begin;      /* 0x0C */
    unsigned size;     /* 0x10 */
    unsigned capacity; /* 0x14 */
};
struct AuxiliaryHistoryVector {
    void* vt;          /* 0x00 */
    int currentIndex;  /* 0x04: -1 until the first recorded sample */
    int highestIndex;  /* 0x08: highest initialized history index */
    int* begin;        /* 0x0C */
    unsigned size;     /* 0x10 */
    unsigned capacity; /* 0x14 */
};
struct PadAccStateView {
    void* vt;                                /* 0x00 */
    SampleHistoryVector samples;             /* 0x04 */
    AuxiliaryHistoryVector auxiliaryHistory; /* 0x1C */
    float horizontalImpulse;                 /* 0x34: accumulated sample X, not current acceleration */
    float gestureMagnitude;                  /* 0x38 */
    int playerIndex;                         /* 0x3C */
    int savedSamplingParameter0;             /* 0x40 */
    int savedSamplingParameter1;             /* 0x44 */
    int samplingAdjusted;                    /* 0x48 */
};
// Sampling lifecycle: construction sets samplingAdjusted to zero but does NOT
// initialize savedSamplingParameter0/1. They are reserved storage for another
// operation; reading them requires that operation to have populated them first.
// This routine neither saves sampling parameters nor changes a sampling rate.

} // extern "C"

#pragma opt_strength_reduction off
#pragma opt_propagation off
// Construct in place: allocate 120 zeroed samples and 10 auxiliary words, then initialize gesture/player state.
// Typed view of the pad object with its two history vectors laid out in full.
static inline PadAccStateView* View(PadAccState* p) {
    return (PadAccStateView*)p;
}
void* PadAccState::Init(int playerIndex) {
    // Phase 1: establish an empty owned sample history with no current/highest index.
    SampleHistoryVector* samples = &View(this)->samples;
    View(this)->vt = __vt__9CAccState;
    samples->vt = s_padSampleHistoryVectorVTable;
    samples->currentIndex = -1;
    samples->highestIndex = -1;
    samples->begin = 0;
    samples->size = 0;
    samples->capacity = 0;

    unsigned oldSampleCount = samples->size;
    // Phase 2: resize to 120 records. The expanded container machinery is kept
    // for matching: oldSampleCount is zero on this constructor's live path.
    do {
        if (oldSampleCount < kInitialAccStateEntryCount) {
            unsigned appendCount = kInitialAccStateEntryCount - oldSampleCount;
            AllocTag resizeAlloc;
            // This size-based gate and the empty growth-limit arms below are
            // retained matching artifacts. They are NOT a spare-capacity policy;
            // meaningful allocation sizing occurs in the TmpBuf block that follows.
            if (appendCount > oldSampleCount || oldSampleCount > oldSampleCount - appendCount) {
                AllocTag gate2;
                unsigned firstRequired = (samples->size + appendCount) - oldSampleCount;
                unsigned firstCap = samples->capacity;
                if (firstRequired > kMaxAccStateEntries - firstCap) {
                    Err2 e(vectorLengthErrorStrings_8026700C);
                }
                if (firstCap < kSmallEntryGrowthLimit) {
                } else if (firstCap < kMediumEntryGrowthLimit) {
                }
            } else {
                AllocTag insertAlloc;
                Entry* p = samples->begin + oldSampleCount;
                while (appendCount != 0) {
                    Entry emptyEntry = {};
                    new (p) Entry(emptyEntry);
                    --appendCount;
                    ++p;
                    ++samples->size;
                }
                break;
            }
            {
                TmpBuf<Entry> tmp(&samples->capacity);
                // Reserve: select bounded growth and allocate temporary storage.
                unsigned capacity = samples->capacity;
                unsigned needed = (samples->size + appendCount) - samples->capacity;
                if (needed > kMaxAccStateEntries - samples->capacity) {
                    Err2 e(vectorLengthErrorStrings_8026700C);
                }
                unsigned newCapacity;
                if (capacity < kSmallEntryGrowthLimit) {
                    unsigned growthAmount = (3u * (capacity + 1u)) / 5u;
                    unsigned* chosen = &growthAmount;
                    if (growthAmount < needed) {
                        chosen = &needed;
                    }
                    newCapacity = capacity + *chosen;
                } else if (capacity < kMediumEntryGrowthLimit) {
                    unsigned growthAmount = (capacity + 1u) / 2u;
                    unsigned* chosen = &growthAmount;
                    if (growthAmount < needed) {
                        chosen = &needed;
                    }
                    newCapacity = capacity + *chosen;
                } else {
                    newCapacity = kMaxAccStateEntries;
                }
                AllocTag reserveAlloc;
                if (newCapacity > kMaxAccStateEntries) {
                    Err2 e(vectorLengthErrorStrings_8026700C);
                }
                tmp.data = (Entry*)appMallocDummy__FUl(newCapacity * sizeof(Entry));
                tmp.capacity = newCapacity;
                tmp.remaining = samples->size;
                Entry* p = tmp.data + tmp.size + samples->size;
                {
                    AllocTag constructAlloc;
                    unsigned c = appendCount;
                    while (c != 0) {
                        Entry emptyEntry = {};
                        new (p) Entry(emptyEntry);
                        --c;
                        ++p;
                        ++tmp.size;
                    }
                }
                // Copy: place existing records before the newly zeroed suffix.
                Entry* dst = tmp.data + tmp.remaining;
                Entry* src = samples->begin + samples->size;
                {
                    AllocTag copyAlloc;
                    Entry* begin = samples->begin;
                    while (src > begin) {
                        --dst;
                        --src;
                        new (dst) Entry(*src);
                        --tmp.remaining;
                        ++tmp.size;
                    }
                }
                // Commit: install the completed array; tmp receives/frees old storage.
                Entry* old = samples->begin;
                unsigned oldCapacity = samples->capacity;
                {
                    AllocTag swapAlloc;
                    samples->size = 0;
                    samples->capacity = tmp.capacity;
                    tmp.capacity = oldCapacity;
                    samples->begin = tmp.data;
                    tmp.data = old;
                    samples->size = tmp.size;
                    tmp.size = 0;
                }
            }
        } else if (kInitialAccStateEntryCount < oldSampleCount) {
            AllocTag eraseAlloc;
            samples->size -= oldSampleCount - kInitialAccStateEntryCount;
        }

    } while (0);
    // Phase 3: independently initialize/resize the auxiliary word history to 10.
    // Its relationship to individual sample words is not established by this unit.
    AuxiliaryHistoryVector& auxiliaryHistory = View(this)->auxiliaryHistory;
    auxiliaryHistory.vt = s_padAuxiliaryHistoryVectorVTable;
    auxiliaryHistory.currentIndex = -1;
    auxiliaryHistory.highestIndex = -1;
    auxiliaryHistory.begin = 0;
    auxiliaryHistory.size = 0;
    auxiliaryHistory.capacity = 0;

    unsigned oldAuxiliaryCount = auxiliaryHistory.size;
    do {
        if (oldAuxiliaryCount < kInitialAccStateAuxiliaryWordCount) {
            unsigned appendCount = kInitialAccStateAuxiliaryWordCount - oldAuxiliaryCount;
            AllocTag resizeAlloc2;
            // Same retained size gate/empty threshold arms as the sample resize;
            // these do not initialize savedSamplingParameter0/1 or adjust sampling.
            if (appendCount > oldAuxiliaryCount || oldAuxiliaryCount > oldAuxiliaryCount - appendCount) {
                AllocTag gate2b;
                unsigned firstRequired = (auxiliaryHistory.size + appendCount) - oldAuxiliaryCount;
                unsigned firstCap = auxiliaryHistory.capacity;
                if (firstRequired > kMaxAccStateAuxiliaryWords - firstCap) {
                    Err2 e(vectorLengthErrorStrings_8026700C);
                }
                if (firstCap < kSmallAuxiliaryGrowthLimit) {
                } else if (firstCap < kMediumAuxiliaryGrowthLimit) {
                }
            } else {
                int* p = auxiliaryHistory.begin + oldAuxiliaryCount;
                memset(p, 0, appendCount * 4u);
                auxiliaryHistory.size += appendCount;
                break;
            }
            {
                TmpBuf<int> tmp(&auxiliaryHistory.capacity);
                // Reserve and zero the suffix, copy the prefix, then swap ownership.
                unsigned capacity = auxiliaryHistory.capacity;
                unsigned needed = (auxiliaryHistory.size + appendCount) - auxiliaryHistory.capacity;
                if (needed > kMaxAccStateAuxiliaryWords - auxiliaryHistory.capacity) {
                    Err2 e(vectorLengthErrorStrings_8026700C);
                }
                unsigned newCapacity;
                if (capacity < kSmallAuxiliaryGrowthLimit) {
                    unsigned growthAmount = (3u * (capacity + 1u)) / 5u;
                    unsigned* chosen = &growthAmount;
                    if (growthAmount < needed) {
                        chosen = &needed;
                    }
                    newCapacity = capacity + *chosen;
                } else if (capacity < kMediumAuxiliaryGrowthLimit) {
                    unsigned growthAmount = (capacity + 1u) / 2u;
                    unsigned* chosen = &growthAmount;
                    if (growthAmount < needed) {
                        chosen = &needed;
                    }
                    newCapacity = capacity + *chosen;
                } else {
                    newCapacity = kMaxAccStateAuxiliaryWords;
                }
                AllocTag reserveAlloc2;
                if (newCapacity > kMaxAccStateAuxiliaryWords) {
                    Err2 e(vectorLengthErrorStrings_8026700C);
                }
                tmp.data = (int*)appMallocDummy__FUl(newCapacity * 4u);
                tmp.capacity = newCapacity;
                tmp.remaining = auxiliaryHistory.size;
                int* p = tmp.data + auxiliaryHistory.size + tmp.size;
                {
                    AllocTag constructAlloc2;
                    memset(p, 0, appendCount * 4u);
                }
                int* begin = auxiliaryHistory.begin;
                int* src = begin + auxiliaryHistory.size;
                unsigned copyCount = src - begin;
                tmp.size += appendCount;
                tmp.remaining -= copyCount;
                int* dst = tmp.data + tmp.remaining;
                unsigned bytes = copyCount * 4u;
                {
                    AllocTag copyAlloc2;
                    memcpy(dst, begin, bytes);
                }
                {
                    memset(begin, 0, bytes);
                }
                tmp.size += copyCount;
                int* old = auxiliaryHistory.begin;
                unsigned oldCapacity = auxiliaryHistory.capacity;
                {
                    AllocTag swapAlloc2;
                    auxiliaryHistory.size = 0;
                    auxiliaryHistory.capacity = tmp.capacity;
                    tmp.capacity = oldCapacity;
                    auxiliaryHistory.begin = tmp.data;
                    tmp.data = old;
                    auxiliaryHistory.size = tmp.size;
                    tmp.size = 0;
                }
            }
        } else if (kInitialAccStateAuxiliaryWordCount < oldAuxiliaryCount) {
            AllocTag eraseAlloc2;
            auxiliaryHistory.size -= oldAuxiliaryCount - kInitialAccStateAuxiliaryWordCount;
        }

    } while (0);
    // Phase 4: initialize per-player gesture scalars and mark sampling unadjusted.
    float zero = kFloat_0_UiAnimation;
    View(this)->playerIndex = playerIndex;
    View(this)->horizontalImpulse = zero;
    View(this)->gestureMagnitude = zero;
    View(this)->samplingAdjusted = 0;
    return this;
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462328", _ctors_u8026700C_0, PadSampleHistory_InitTwoPiCache);
#pragma pop
