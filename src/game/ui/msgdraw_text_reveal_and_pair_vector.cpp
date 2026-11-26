// Message text helpers for CMsgDraw: advancing the reveal of message lists and drawing their glyphs,
// plus the vector containers of pointer/integer pairs used by the formatting and glyph lists.
// Also has the full-turn angle constants for this unit.

#include <decomp/static_init.h>
#include "game/exception.h"
#include "sdk_protos.h"
#include "classes/CMsgDraw.h"

extern "C" {

enum PairVectorLimits {
    kMaxPairCount = 0x1fffffff,
    kSmallPairGrowthLimit = 0x0aaaaaaa,
    kMediumPairGrowthLimit = 0x15555554,
    kInitialPairCapacity = 32,
    kPairBytes = 8,
    kFormattingTokenCount = 6
};

extern const float kFloat_6_2831855;
extern float s_twoPi_800A8920;
extern float s_twoPi_800A8920_805E2888;
extern void appFreeUni__FPv(void*);
extern void Text_DrawGlyphs(void*);
extern void __dl__FPv(void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void* __nwa__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void __dt__Q23std12length_errorFv(void*);
struct FormattingToken {
    unsigned short* text;
    int id;
};
extern FormattingToken messageFormattingTokens[];
extern const char kPointerIntVectorLengthErrorText[];
extern const char __vt__Q23std12length_error[];
extern const char __vt__Q23std11logic_error[];
extern const char s_sharedCharDeleterVtable[];

struct OwnedVectorView {
    void* object;
    volatile int count;
};

// Clear a vector's count, free its backing allocation, and optionally delete the vector.
void* OwnedVectorView_dtor(OwnedVectorView* self, int deleting) {
    if (self != 0) {
        int zero = 0;
        self->count = zero;
        if (self != 0) {
            if (self->object != 0) {
                self->count = zero;
                appFreeUni__FPv(self->object);
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Release the second vector specialization with identical storage ownership.
void* dtor_800A8994(OwnedVectorView* self, int deleting) {
    if (self != 0) {
        int zero = 0;
        self->count = zero;
        if (self != 0) {
            if (self->object != 0) {
                self->count = zero;
                appFreeUni__FPv(self->object);
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Remove all trivial entries before freeing this vector's backing allocation.
void* dtor_800A91FC(OwnedVectorView* self, int deleting) {
    if (self != 0) {
        void* object = self->object;
        if (object != 0) {
            int count = self->count;
            self->count = self->count - count;
            appFreeUni__FPv(object);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Initialize the first independent full-turn angle slot.
void InitializeFullTurnAngle_800A8A08() {
    s_twoPi_800A8920 = kFloat_6_2831855;
}

// Initialize the second independent full-turn angle slot.
void InitializeFullTurnAngle_800A92E0() {
    s_twoPi_800A8920_805E2888 = kFloat_6_2831855;
}

struct MsgDrawRevealEntry {
    void* object;
    void* associatedValue; // The reveal traversal only uses object.
};

struct MsgDrawRevealList {
    MsgDrawRevealEntry* entries;
    unsigned int count;
};

// Advance text reveal for every message object in the list.
int MsgDrawList_UpdateReveal(MsgDrawRevealList* self) {
    MsgDrawRevealEntry* entry = self->entries;
    while (entry != self->entries + self->count) {
        ((CMsgDraw*)(entry->object))->UpdateReveal();
        entry++;
    }
    return 1;
}

struct Dtor_800A926C {
    void* object;
    int count;
};

// Reset and release another trivial-entry vector, optionally deleting its wrapper.
void* dtor_800A926C(Dtor_800A926C* self, int deleting) {
    if (self != 0) {
        int zero = 0;
        self->count = zero;
        if (self != 0) {
            if (self->object != 0) {
                self->count = zero;
                appFreeUni__FPv(self->object);
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

struct Dtor_800A8B44 {
    void* object;
    volatile int count;
};

// Release the pair vector; nested guards retain the original destruction scopes.
void* dtor_800A8B44(Dtor_800A8B44* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    void* object = self->object;
                    if (object != 0) {
                        int count = self->count;
                        self->count = count - count;
                        appFreeUni__FPv(object);
                    }
                }
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

struct MsgDrawGlyphEntry {
    void* object;
    int state;
};

#pragma exceptions on
} // extern "C"
inline void* operator new(size_t, void* p) {
    return p;
}
extern "C" {
extern void* __nw__FUl(unsigned long);
extern void __dt__Q33std6detail16char_array_guardFv(void*, int);
#pragma opt_propagation off
#pragma opt_common_subs on
// Preserve the allocator's scope objects: their lifetimes generate exception cleanup records.
struct Scope_8CA8 {
    volatile char value;
    Scope_8CA8() : value(0) {}
    ~Scope_8CA8() {}
};
struct AllocTag {
    volatile char value;
    AllocTag() : value(0) {}
};
} // extern "C"
namespace std {
namespace detail {
struct CharArrayGuard_u800A8920 {
    void* data;
    void* allocator;
    ~CharArrayGuard_u800A8920() {
        if (data)
            __dla__FPv(data);
    }
};
}
}
extern "C" {
typedef std::detail::CharArrayGuard_u800A8920 CharArrayGuard;
struct SharedCharRef {
    const void* vtable;
    int a, b;
    void* data;
    SharedCharRef(void* p) {
        a = 1;
        b = 1;
        vtable = s_sharedCharDeleterVtable;
        data = p;
    }
};
struct SharedCharString {
    char* text;
    SharedCharRef* ref;
    SharedCharString(const char* const& s, const AllocTag&) {
        AllocTag allocator;
        int n = (int)strlen(s) + 1;
        char* p = (char*)__nwa__FUl((unsigned int)(n * 2 - n));
        text = p;
        {
            CharArrayGuard guard = {p, &allocator};
            ref = new SharedCharRef(p);
            guard.data = 0;
        }
        strcpy(text, s);
    }
};
} // extern "C"
namespace std {
struct logic_error : exception {
    SharedCharString message;
    logic_error(const char* s) : message(s, AllocTag()) {}
    ~logic_error();
};
struct length_error : logic_error {
    length_error(const char* s) : logic_error(s) {}
    ~length_error();
};
}
extern "C" {
// Reject vector capacities that exceed the maximum pointer/integer pair allocation.
inline void ThrowVectorLengthError() {
    throw std::length_error(kPointerIntVectorLengthErrorText);
}
struct PtrIntPair {
    void* volatile first;
    int second;
    PtrIntPair(void* a, int b) : first(a), second(b) {}
    PtrIntPair(const PtrIntPair& p) {
        *this = p;
    }
};
struct PtrIntVector {
    PtrIntPair* entries;
    unsigned int count;
    unsigned int capacity;
};
// Construct one trivial pair inside the library's original placement-construction scope.
inline void ConstructPair(PtrIntPair* dst, const PtrIntPair& value) {
    if (dst) {
        void* first = value.first;
        int second = value.second;
        try {
            dst->first = first;
            dst->second = second;
        } catch (...) {
            throw;
        }
    }
}
struct Buffer_8CA8 {
    PtrIntPair* entries;
    unsigned int count, capacity;
    unsigned int* allocator;
    unsigned int remaining;
    Buffer_8CA8(unsigned int* a) : entries(0), count(0), capacity(0), allocator(a), remaining(0) {}
    ~Buffer_8CA8() {
        if (entries) {
            Scope_8CA8 scope;
            unsigned int n = count;
            count = count - n;
            appFreeUni__FPv(entries);
        }
    }
};

// Grow by the larger of the requested increment and the size-dependent growth step.
// Growth step is 3/5 of (capacity+1) while small, 1/2 while medium, then the maximum (library vector policy).
inline unsigned int NextPairCapacity(unsigned int oldCapacity, const unsigned int& needed) {
    unsigned int newCapacity;
    unsigned int grown;
    unsigned int half;
    if (needed > (unsigned int)kMaxPairCount - oldCapacity)
        ThrowVectorLengthError();
    if (oldCapacity < (unsigned int)kSmallPairGrowthLimit) {
        grown = ((oldCapacity + 1) * 3) / 5;
        const unsigned int* p = grown < needed ? &needed : &grown;
        newCapacity = oldCapacity + *p;
    } else if (oldCapacity < (unsigned int)kMediumPairGrowthLimit) {
        half = (oldCapacity + 1) >> 1;
        const unsigned int* p = half < needed ? &needed : &half;
        newCapacity = oldCapacity + *p;
    } else
        newCapacity = kMaxPairCount;
    return newCapacity;
}
#pragma use_lmw_stmw on
// Allocate a checked number of eight-byte pairs.
inline PtrIntPair* AllocatePairs(unsigned int capacity) {
    Scope_8CA8 allocating;
    if (capacity > (unsigned int)kMaxPairCount)
        ThrowVectorLengthError();
    return (PtrIntPair*)appMallocDummy__FUl(capacity * kPairBytes);
}
// Append a pointer/integer pair, preserving entries and exception cleanup on growth.
int PointerIntVector_AppendPair(PtrIntVector* self, void* key, int value) {
    Scope_8CA8 scope;
    unsigned int count = self->count;
    unsigned int capacity = self->capacity;
    if (count < capacity) {
        unsigned int size = count + 1;
        self->count = size;
        PtrIntPair* entry = self->entries + (size - 1);
        entry->first = key;
        entry->second = value;
    } else {
        Scope_8CA8 reserving;
        AllocTag allocator;
        if ((unsigned int)kMaxPairCount - capacity < 1)
            ThrowVectorLengthError();
        unsigned int needed = self->count + 1 - self->capacity;
        Buffer_8CA8 fresh(&self->capacity);
        unsigned int oldCapacity = self->capacity;
        unsigned int newCapacity = NextPairCapacity(oldCapacity, needed);
        fresh.entries = AllocatePairs(newCapacity);
        unsigned int insertionCount = self->count;
        fresh.capacity = newCapacity;
        PtrIntPair* insert = fresh.entries + insertionCount;
        (insert + fresh.count)->first = key;
        (insert + fresh.count)->second = value;
        PtrIntPair* begin = self->entries;
        PtrIntPair* src = begin + self->count;
        fresh.remaining = insertionCount;
        Scope_8CA8 inserting;
        fresh.count++;
        PtrIntPair* dst = insert;
        Scope_8CA8 copying;
        while (src > begin) {
            --dst;
            --src;
            ConstructPair(dst, *src);
            fresh.remaining--;
            fresh.count++;
        }
        self->count = 0;
        PtrIntPair* old = self->entries;
        unsigned int oldCap = self->capacity;
        unsigned int oldCount = self->count;
        self->capacity = fresh.capacity;
        fresh.capacity = oldCap;
        self->entries = fresh.entries;
        fresh.entries = old;
        Scope_8CA8 swapping;
        self->count = fresh.count;
        fresh.count = oldCount;
    }
    return 1;
}

#pragma use_lmw_stmw reset
#pragma opt_common_subs reset
#pragma opt_propagation reset

struct MsgDrawGlyphList {
    MsgDrawGlyphEntry* entries;
    unsigned int count;
};

// Draw each message, optionally reset its text state, and clear draw suppression.
int MsgDrawList_DrawGlyphs(MsgDrawGlyphList* self) {
    MsgDrawGlyphEntry* entry = self->entries;
    while (entry != self->entries + self->count) {
        Text_DrawGlyphs(entry->object);
        if (entry->state == 0)
            ((CMsgDraw*)(entry->object))->ResetTextState();
        void* object = entry->object;
        entry++;
        ((CMsgDraw*)object)->drawSuppressed = 0;
    }
    return 1;
}

struct PointerPairEntry {
    void* key;
    void* value;
};
struct PointerPairList {
    PointerPairEntry* entries;
    unsigned int count;
};

// Remove the first matching pointer key and compact the remaining pairs in order.
int PointerPairList_RemoveByKey(PointerPairList* self, void* key) {
    PointerPairEntry* end = self->entries + self->count;
    PointerPairEntry* entry = self->entries;
    while (entry != end) {
        if (entry->key == key) {
            volatile PointerPairList* vself = self;
            memmove(entry, entry + 1, (unsigned int)(vself->entries + vself->count - entry - 1) * kPairBytes);
            self->count--;
            return 1;
        }
        entry++;
    }
    return 0;
}

#pragma exceptions on
} // extern "C"
extern "C" {
struct InitEntry_800A8A14 {
    void* object;
    void* value;
    InitEntry_800A8A14(const InitEntry_800A8A14& other) : object(other.object), value(other.value) {}
};
struct InitScope_800A8A14 {
    volatile char value;
    InitScope_800A8A14() : value(0) {}
    ~InitScope_800A8A14() {}
};
struct InitBuffer_800A8A14 {
    InitEntry_800A8A14* entries;
    unsigned int count;
    unsigned int capacity;
    InitBuffer_800A8A14(int initialWord)
        : entries((InitEntry_800A8A14*)initialWord), count(initialWord), capacity(initialWord) {}
    ~InitBuffer_800A8A14() {
        if (entries) {
            InitScope_800A8A14 scope;
            int n = count;
            count = count - n;
            appFreeUni__FPv(entries);
        }
    }
};
// Copy a pointer pair while preserving the reserved vector's construction cleanup scope.
inline void initCopy_800A8A14(InitEntry_800A8A14* dst, const InitEntry_800A8A14& value) {
    if (dst) {
        void* object = value.object;
        void* second = value.value;
        try {
            dst->object = object;
            dst->value = second;
        } catch (...) {
            throw;
        }
    }
}

struct InitList_800A8A14 {
    InitEntry_800A8A14* entries;
    unsigned int count;
    unsigned int capacity;
};
// Reserve the initial pair block with guards matching the allocator's cleanup scopes.
inline void initReserve_800A8A14(InitList_800A8A14* self, unsigned int size) {
    int initialSize = size;
    if (size < kInitialPairCapacity) {
        InitBuffer_800A8A14 fresh(initialSize);
        InitScope_800A8A14 allocation;
        fresh.entries = (InitEntry_800A8A14*)appMallocDummy__FUl(kInitialPairCapacity * kPairBytes);
        fresh.capacity = kInitialPairCapacity;
        InitEntry_800A8A14* old = self->entries;
        InitEntry_800A8A14* end = old + self->count;
        InitEntry_800A8A14* dst = fresh.entries + initialSize;
        InitScope_800A8A14 copying;
        while (old < end) {
            initCopy_800A8A14(dst, *old);
            old++;
            dst++;
            fresh.count++;
        }
        InitEntry_800A8A14* oldEntries = self->entries;
        volatile unsigned int* count = &self->count;
        *count = 0;
        InitEntry_800A8A14* newEntries = fresh.entries;
        unsigned int oldCapacity = self->capacity;
        unsigned int freshCapacity = fresh.capacity;
        unsigned int oldCount = *count;
        unsigned int freshCount = fresh.count;
        fresh.entries = oldEntries;
        InitScope_800A8A14 swapping;
        self->capacity = freshCapacity;
        fresh.capacity = oldCapacity;
        self->entries = newEntries;
        self->count = freshCount;
        fresh.count = oldCount;
    }
}
// Construct an empty pair vector with space for the initial 32 entries.
InitList_800A8A14* PairVector_InitReserved(InitList_800A8A14* self) {
    self->entries = 0;
    self->count = 0;
    self->capacity = 0;
    initReserve_800A8A14(self, self->count);
    return self;
}

// Return the position after a matching UTF-16 token, stopping at the excluded boundary.
// In plain C: from searchPosition, find the next char equal to token[0] (looking no further than 'excluded'); if the
// rest of token matches the text after it, return the end of the match, else continue from the next position.
// Returns 0 when nothing matches.
inline unsigned short* MatchFormattingToken(unsigned short* token, unsigned short* searchPosition,
                                            unsigned short* excluded) {
    unsigned short* matchEnd;
    unsigned short* boundaryCursor;
    unsigned short* scan;
    int characterOffset;
    int mismatch;
    unsigned short firstCharacter;
    if (searchPosition == 0)
        return 0;
    while (*searchPosition != 0) {
        firstCharacter = *token;
        if (searchPosition == 0) {
            searchPosition = 0;
        } else {
            boundaryCursor = searchPosition;
            scan = searchPosition;
            characterOffset = 0;
            // 0x800A92EC: exhaustion clears searchPosition; a match keeps it non-null before adding characterOffset.
            while (*scan != 0 || (searchPosition = 0, false)) {
                if (excluded == boundaryCursor) {
                    searchPosition = 0;
                    break;
                }
                if (firstCharacter == *scan) {
                    searchPosition += characterOffset;
                    break;
                }
                boundaryCursor++;
                scan++;
                characterOffset++;
            }
        }
        if (searchPosition == 0)
            return 0;
        matchEnd = searchPosition + 1;
        scan = token + 1;
        mismatch = 0;
        while (*scan != 0) {
            if (*matchEnd != *scan) {
                mismatch = 1;
                break;
            }
            matchEnd++;
            scan++;
        }
        if (mismatch == 0)
            return matchEnd;
        searchPosition++;
    }
    return 0;
}

// Find the first formatting token in table order and return its ID and end position.
int CMsgDraw::FindFormattingToken(unsigned short* first, unsigned short* excluded, unsigned short** out) {
    FormattingToken* entry = messageFormattingTokens;
    unsigned int i = 0;
    for (; i < kFormattingTokenCount; entry++, i++) {
        unsigned short* match = MatchFormattingToken(entry->text, first, excluded);
        *out = match;
        if (match != 0)
            return messageFormattingTokens[i].id;
    }
    return -1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462074", _ctors_u800A8920_0, InitializeFullTurnAngle_800A8A08);
STATIC_INIT_ENTRY(".CRT$XCU80462074", _ctors_u800A8920_1, InitializeFullTurnAngle_800A92E0);
#pragma pop
