#include <decomp/ct_member.h>
#include "iff/IffStruct.h"
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char iffSchemaAndCourseStrings[];
}
}

// Translation unit 0x8018AD5C-0x8018C1F8 (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "game/exception.h"

// Three specializations of the same loader. On disk: 8-byte header (native
// big-endian u16 count at +0; other six bytes ignored), then count packed IFF
// records. No byte swapping or input-length check exists. Caller must supply
// at least 8 + count * diskStride readable bytes in the game's Wii byte order.
// Disk stride -> owned entry stride: club set 0x8c -> 0x90; item 0x60 -> 0x64;
// ball 0x2c0 -> 0x2c4. Entry word0 caches payload.common.typeId, not an index.
// Each loader owns begin; input is borrowed only for the duration of the call.
// Resize precedes deserialization: prefix entries survive growth temporarily,
// but EVERY entry is overwritten by the incoming records in the final phase.
enum IffLoaderLimits {
    kClubSetMaxEntries = 0x1C71C71, kClubSetSmallGrowth = 0x97B425, kClubSetHalfGrowth = 0x12F684A,
    kItemMaxEntries = 0x28F5C28, kItemSmallGrowth = 0xDA740D, kItemHalfGrowth = 0x1B4E81A,
    kBallMaxEntries = 0x5C90A1, kBallSmallGrowth = 0x1EDAE0, kBallHalfGrowth = 0x3DB5C0
};
// Max entries = floor(0xffffffff / entryStride), preventing byte-size overflow.
// Growth thresholds are floor(max/3), floor(2*max/3): add max(needed, 3/5 of
// cap+1), then max(needed, 1/2 of cap+1), then saturate at max. needed is the
// ADDITION beyond current capacity, not the final requested element count.
// Snapshot temporaries below keep each phase's size/capacity distinct for MWCC; keep
// statement order and lifetimes when tuning growth, then compare object code.

#pragma use_lmw_stmw on
#pragma does_not_return __throw
extern "C" void __throw(const char*, void*, void*);

// Placement allocation borrows an existing destination without allocating.
inline void* operator new(size_t, void* address) throw() { return address; }
// Placement-construction failure leaves deallocation to the buffer owner.
inline void operator delete(void*, void*) throw() {}

extern "C" {

struct BallRecordStorage {
    union { IFF_STRUCT::sBall value; unsigned int words[sizeof(IFF_STRUCT::sBall) / 4]; };
    // Leave raw record storage uninitialized until the deserialize phase.
    BallRecordStorage() {}
};
struct BallLoadEntry {
    unsigned int tag;
    BallRecordStorage payload;
    // Copy raw payload with an initially unset type tag.
    BallLoadEntry(const BallRecordStorage& value) : tag(0), payload(value) {}
    // Copy both cached type tag and inline ball payload.
    BallLoadEntry(const BallLoadEntry& value) : tag(value.tag), payload(value.payload) {}
    // Replace payload and cache its externally supplied typeId.
    void assign(const BallRecordStorage& value, unsigned int id) {
        tag = id;
        payload = value;
    }
};
struct BallTableLoader {
    unsigned char pad[0x50];
    BallLoadEntry* begin;
    unsigned int size;
    unsigned int capacity;
};
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void* memcpy(void*, const void*, unsigned int);
extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);

struct ClubSetRecordStorage {
    union { IFF_STRUCT::sClubSet value; unsigned int words[sizeof(IFF_STRUCT::sClubSet) / 4]; };
    // Leave raw record storage uninitialized until the deserialize phase.
    ClubSetRecordStorage() {}
};
struct Allocator8018AD5C {
    volatile char unused;
    // Initialize the allocator marker used by the original inlined paths.
    Allocator8018AD5C() : unused(0) {}
};
struct ClubSetLoadEntry {
    unsigned int tag;
    ClubSetRecordStorage payload;
    // Copy raw payload with an initially unset type tag.
    ClubSetLoadEntry(const ClubSetRecordStorage& value) : tag(0), payload(value) {}
    // Copy both cached type tag and inline club-set payload.
    ClubSetLoadEntry(const ClubSetLoadEntry& value) : tag(value.tag), payload(value.payload) {}
    // Replace payload and cache its externally supplied typeId.
    void assign(const ClubSetRecordStorage& value, unsigned int id) {
        tag = id;
        payload = value;
    }
};
struct ClubSetTableLoader {
    unsigned char pad[0x44];
    ClubSetLoadEntry* begin;
    unsigned int size;
    unsigned int capacity;
};

struct ItemRecordStorage {
    union { IFF_STRUCT::sItem value; unsigned int words[sizeof(IFF_STRUCT::sItem) / 4]; };
    // Leave raw record storage uninitialized until the deserialize phase.
    ItemRecordStorage() {}
};
struct ItemLoadEntry {
    unsigned int tag;
    ItemRecordStorage payload;
    // Copy raw payload with an initially unset type tag.
    ItemLoadEntry(const ItemRecordStorage& value) : tag(0), payload(value) {}
    // Copy both cached type tag and inline item payload.
    ItemLoadEntry(const ItemLoadEntry& value) : tag(value.tag), payload(value.payload) {}
    // Replace payload and cache its externally supplied typeId.
    void assign(const ItemRecordStorage& value, unsigned int id) {
        tag = id;
        payload = value;
    }
};
struct ItemTableLoader {
    unsigned char pad[0x5C];
    ItemLoadEntry* begin;
    unsigned int size;
    unsigned int capacity;
};

extern char iffSchemaAndCourseStrings[];
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12length_error[];
extern char s_sharedCharDeleterVtable[];
extern void* __nw__FUl(unsigned long);
extern void __dla__FPv(void*);

struct ErrorRef8018BB2C {
    void* vtable;
    unsigned int strong;
    unsigned int weak;
    char* text;
};
struct TextGuard8018BB2C {
    char* text;
    char* allocator;
    // Own temporary exception text until its shared holder is established.
    TextGuard8018BB2C(char* ptr, char* owner) : text(ptr), allocator(owner) {}
    // Free exception text only if ownership was not released to a holder.
    ~TextGuard8018BB2C() { if (text) __dla__FPv(text); }
};

extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct ErrorBase8018BB2C : std::exception {
    // Install the exception subtype's original vtable pointer.
    void SetTable(const void* vt) { *(const void**)this = vt; }

    // Begin exception construction with the logic_error base vtable.
    ErrorBase8018BB2C() { SetTable(__vt__Q23std11logic_error); }
};
namespace std {
struct length_error : ErrorBase8018BB2C {
    char* text;
    void* control;
    // Allocate shared message text, then install the length_error vtable.
    length_error() {
        const char* message = iffSchemaAndCourseStrings + 0x2E5;
        char allocator = 0;
        unsigned int length = strlen(message) + 1;
        text = (char*)__nwa__FUl(length * 2 - length);
        {
            char* p = text;
            TextGuard8018BB2C guard(p, &allocator);
            ErrorRef8018BB2C* ref = (ErrorRef8018BB2C*)__nw__FUl(0x10);
            if (ref) {
                ref->strong = 1;
                ref->weak = 1;
                ref->vtable = s_sharedCharDeleterVtable;
                ref->text = p;
            }
            control = ref;
            guard.text = 0;
        }
        strcpy(text, message);
        SetTable(__vt__Q23std12length_error);
    }

    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}

extern void __dt__Q23std12length_errorFv();
struct LengthError8018B460 {
    void* table;
    char* text;
    void* control;
    // Unwind the exception base without deleting this stack object.
    ~LengthError8018B460() { __dt__Q23std9exceptionFv(this, -1); }
};
#pragma does_not_return __throw
struct LengthThrow8018B460 : LengthError8018B460 {
    // Construct shared error text and throw this ball-table length exception.
    LengthThrow8018B460() {
        table = __vt__Q23std11logic_error;
        const char* message = iffSchemaAndCourseStrings + 0x2E5;
        char inner = 0;
        unsigned int length = strlen(message) + 1;
        text = (char*)__nwa__FUl(length * 2 - length);
        {
            char* p = text;
            TextGuard8018BB2C guard(p, &inner);
            ErrorRef8018BB2C* ref = (ErrorRef8018BB2C*)__nw__FUl(0x10);
            if (ref) {
                ref->strong = 1;
                ref->weak = 1;
                ref->vtable = s_sharedCharDeleterVtable;
                ref->text = p;
            }
            control = ref;
            guard.text = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
        __throw(iffSchemaAndCourseStrings + 0x2F9, this, (void*)__dt__Q23std12length_errorFv);
    }
};

struct Resize8018AD5C {
    // Transaction owns next.begin until commit. remaining marks the unfilled
    // prefix; size counts constructed suffix+copied prefix. Commit swaps in the
    // old buffer so this guard frees it; it does not destroy record payloads.
    ClubSetLoadEntry* begin;
    unsigned int size;
    unsigned int capacity;
    unsigned int* ownerCapacity;
    unsigned int remaining;
    // Start an empty club-set resize transaction linked to owner capacity.
    Resize8018AD5C(unsigned int* owner)
        : begin(0), size(0), capacity(0), ownerCapacity(owner), remaining(0) {}
    // Free the uncommitted buffer, or the old buffer swapped here at commit.
    ~Resize8018AD5C() {
        if (begin) {
            Allocator8018AD5C allocator;
            size -= size;
            appFreeUni__FPv(begin);
        }
    }
};

// Throw the allocator's length_error before a club-set byte-count overflow.
inline void lengthError8018AD5C() {
    Allocator8018AD5C allocator;
    {
        union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct();
        exception_runtime::__throw(exception_runtime::iffSchemaAndCourseStrings + 0x2F9, error,
            (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}

// Recommend a club-set capacity using the shared three-tier growth rule.
inline unsigned int recommend8018AD5C(const unsigned int& capacity, unsigned int needed) {
    Allocator8018AD5C allocator;
    unsigned int cap = capacity;
    if (needed > kClubSetMaxEntries - cap) lengthError8018AD5C();
    if (cap < kClubSetSmallGrowth) {
        unsigned int increment = ((cap + 1) * 3) / 5;
        const unsigned int* amount = &increment;
        if (increment < needed) amount = &needed;
        return cap + *amount;
    } else if (cap < kClubSetHalfGrowth) {
        unsigned int increment = (cap + 1) / 2;
        const unsigned int* amount = &increment;
        if (increment < needed) amount = &needed;
        return cap + *amount;
    }
    return kClubSetMaxEntries;
}

// Allocate owned storage for count club-set entries, or throw on size overflow.
inline void* allocate8018AD5C(unsigned int count) {
    Allocator8018AD5C allocator;
    if (count > kClubSetMaxEntries) lengthError8018AD5C();
    return appMallocDummy__FUl(count * 0x90);
}
// Placement-construct a tagged club-set entry in already allocated storage.
inline void construct8018AD5C(ClubSetLoadEntry* dest, const ClubSetRecordStorage& value, const Allocator8018AD5C&) {
    new (dest) ClubSetLoadEntry(value);
}
// Copy a club-set entry without transferring or allocating its enclosing buffer.
inline void copyConstruct8018AD5C(ClubSetLoadEntry* dest, const ClubSetLoadEntry& value, const Allocator8018AD5C&) {
    new (dest) ClubSetLoadEntry(value);
}
// Borrow the larger growth amount; caller must keep both arguments alive.
inline const unsigned int& max8018AD5C(const unsigned int& first, const unsigned int& second) {
    if (first < second) return second;
    return first;
}
struct Index8018AD5C {
    unsigned int value;
    // Start deserialization at entry zero.
    Index8018AD5C() : value(0) {}
    // Expose the current destination entry index.
    operator unsigned int() const { return value; }
    // Advance to the next destination entry.
    void operator++() { ++value; }
};

// Copy the old club-set prefix backwards into the transaction's reserved prefix.
inline void moveRecords8018AD5C(ClubSetTableLoader* self, Resize8018AD5C& next) {
    ClubSetLoadEntry* first = self->begin;
    ClubSetLoadEntry* old = first + self->size;
    ClubSetLoadEntry* moved = next.begin + next.remaining;
    Allocator8018AD5C moveAllocator;
    while (old > first) {
        --moved;
        --old;
        copyConstruct8018AD5C(moved, *old, moveAllocator);
        --next.remaining;
        ++next.size;
    }
}

// Resize the owned club-set table, then replace every entry from borrowed IFF bytes.
void ItemInventory_ReadClubSetTable(ClubSetTableLoader* self, const unsigned char* input) { ClubSetRecordStorage noGrowInitial;
    const unsigned char* src;
    unsigned int extra;
    ClubSetRecordStorage raw;

    ClubSetRecordStorage readValue;
    ClubSetRecordStorage growInitial;
    ClubSetRecordStorage growValue;
    ClubSetRecordStorage noGrowValue;
    struct Header { unsigned short count; unsigned char rest[6]; } header;
    memcpy(&header, input, 8);
    unsigned int count = header.count;
    const unsigned char* savedInput = 8 + input;
    src = savedInput;
    // Phase 1: match logical size to header.count. The no-growth path fills only
    // the new suffix; growth constructs that suffix before copying the prefix.
    if (count > self->size) {
        Allocator8018AD5C allocator;
        unsigned int sizeBeforeAppend = self->size;
        unsigned int requestedCount = count;
        extra = requestedCount - sizeBeforeAppend;
        // if: preserve fill-before-growth block order.
        do {
            if (extra > self->capacity || self->size > self->capacity - extra) {
                recommend8018AD5C(self->capacity, *(volatile unsigned int*)&self->size + extra - self->capacity);
            } else {
                unsigned int suffixStartIndex = self->size;
                ClubSetLoadEntry* suffixBegin = self->begin + suffixStartIndex;
                ClubSetLoadEntry* dest = suffixBegin;
                Allocator8018AD5C allocator;
                noGrowValue = noGrowInitial;
                while (extra) {
                    Allocator8018AD5C savedAllocator = allocator;
                    construct8018AD5C(dest, noGrowValue, savedAllocator);
                    extra--;
                    dest++;
                    ++self->size;
                }
                break;
            }

            { int target;
                unsigned int cap;
                unsigned int prefixSizeBeforeGrowth = self->size;
                Resize8018AD5C next(&self->capacity);
                unsigned int entriesToAppend = extra;
                unsigned int needed = prefixSizeBeforeGrowth + entriesToAppend - self->capacity;
                unsigned int capacityBeforeRecommendation = self->capacity;
                cap = capacityBeforeRecommendation;
                if (needed > kClubSetMaxEntries - cap) lengthError8018AD5C();
                if (cap < kClubSetSmallGrowth) {
                    unsigned int capacityPlusOneForThreeFifths = cap + 1;
                    unsigned int increment = ((capacityPlusOneForThreeFifths) * 3) / 5;
                    unsigned int baseCapacity = cap;
                    unsigned int threeFifthsIncrement = increment;
                    target = baseCapacity + max8018AD5C(threeFifthsIncrement, needed);
                } else if (cap < kClubSetHalfGrowth) {
                    unsigned int capacityPlusOneForHalf = 1 + cap;
                    target = cap + max8018AD5C(((capacityPlusOneForHalf) / 2), needed);
                } else target = kClubSetMaxEntries;
                int allocationCount = target;
                next.begin = (ClubSetLoadEntry*)allocate8018AD5C(allocationCount);
                next.remaining = self->size;
                ClubSetLoadEntry* prefixEndInNewBuffer = next.begin + self->size;
                ClubSetLoadEntry* suffixInsertionBegin = (prefixEndInNewBuffer + next.size);
                ClubSetLoadEntry* dest = suffixInsertionBegin;
                int newBufferCapacity = target;
                next.capacity = newBufferCapacity;
                Allocator8018AD5C allocator;
                growValue = growInitial;
                while (extra) {
                    construct8018AD5C(dest, growValue, allocator);
                    dest++;
                    ++next.size;
                    extra = extra - 1;
                }
                // Phase 2: copy the prefix, then swap ownership so next's
                // destructor releases the old allocation, not the committed one.
                moveRecords8018AD5C(self, next);
                self->size = 0;
                ClubSetLoadEntry* newBegin = next.begin;
                ClubSetLoadEntry* oldBegin = self->begin;
                unsigned int oldCapacity = self->capacity;
                unsigned int newSize = next.size;
                unsigned int committedCapacity = next.capacity;
                next.begin = oldBegin;
                Allocator8018AD5C swapAllocator;
                unsigned int capacityToRelease = oldCapacity;
                next.capacity = capacityToRelease;
                self->capacity = committedCapacity;
                self->begin = newBegin;
                unsigned int committedSize = newSize;
                next.size = 0;
                self->size = committedSize;
            }
        } while (0);
    } else if (count < self->size) {
        Allocator8018AD5C allocator;
        self->size -= self->size - count;
    }

    // Phase 3: size == count. Copy each packed record and cache its typeId.
    // Initial filler bytes are not meaningful serialized defaults.
    Index8018AD5C index;
    int i = 0;
    while (i < header.count) {
        memcpy(&raw, src, 0x8C);
        src += 0x8C;

        readValue = raw;
        self->begin[index].assign(readValue, ((raw.value.common.typeId)));
        ++index;
        i += 1;
    }
}

struct Resize8018BB2C {
    ItemLoadEntry* begin;
    unsigned int size;
    unsigned int capacity;
    unsigned int* ownerCapacity;
    unsigned int remaining;
    // Start an empty item resize transaction linked to owner capacity.
    Resize8018BB2C(unsigned int* owner)
        : begin(0), size(0), capacity(0), ownerCapacity(owner), remaining(0) {}
    // Free the uncommitted buffer, or the old buffer swapped here at commit.
    ~Resize8018BB2C() {
        if (begin) {
            Allocator8018AD5C allocator;
            size -= size;
            appFreeUni__FPv(begin);
        }
    }
};

// Throw the allocator's length_error before an item byte-count overflow.
inline void lengthError8018BB2C() {
    Allocator8018AD5C allocator;
    {
        union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct();
        exception_runtime::__throw(exception_runtime::iffSchemaAndCourseStrings + 0x2F9, error,
            (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}

// Recommend an item capacity using the shared three-tier growth rule.
inline unsigned int recommend8018BB2C(const unsigned int& capacity, unsigned int needed) {
    Allocator8018AD5C allocator;
    unsigned int cap = capacity;
    if (needed > kItemMaxEntries - cap) lengthError8018BB2C();
    if (cap < kItemSmallGrowth) {
        unsigned int increment = ((cap + 1) * 3) / 5;
        const unsigned int* amount = &increment;
        if (increment < needed) amount = &needed;
        return cap + *amount;
    } else if (cap < kItemHalfGrowth) {
        unsigned int increment = (cap + 1) / 2;
        const unsigned int* amount = &increment;
        if (increment < needed) amount = &needed;
        return cap + *amount;
    }
    return kItemMaxEntries;
}

// Allocate owned storage for count item entries, or throw on size overflow.
inline void* allocate8018BB2C(unsigned int count) {
    Allocator8018AD5C allocator;
    if (count > kItemMaxEntries) lengthError8018BB2C();
    return appMallocDummy__FUl(count * 0x64);
}
// Placement-construct a tagged item entry in already allocated storage.
inline void construct8018BB2C(ItemLoadEntry* dest, const ItemRecordStorage& value, const Allocator8018AD5C&) {
    new (dest) ItemLoadEntry(value);
}
// Copy one item entry into the transaction's reserved prefix.
inline void copyConstruct8018BB2C(ItemLoadEntry* dest, const ItemLoadEntry& value, const Allocator8018AD5C&) {
    new (dest) ItemLoadEntry(value);
}

// Copy the old item prefix backwards, accounting for each constructed entry.
inline void moveRecords8018BB2C(ItemTableLoader* self, Resize8018BB2C& next) {
    ItemLoadEntry* first = self->begin;
    ItemLoadEntry* old = first + self->size;
    ItemLoadEntry* moved = next.begin + next.remaining;
    Allocator8018AD5C allocator;
    while (old > first) {
        --moved;
        --old;
        copyConstruct8018BB2C(moved, *old, allocator);
        --next.remaining;
        ++next.size;
    }
}

// Resize the owned item table, then replace every entry from borrowed IFF bytes.
void ItemInventory_ReadItemTable(ItemTableLoader* self, const unsigned char* input) {
    ItemRecordStorage noGrowInitial;
    const unsigned char* src;
    unsigned int extra;
    ItemRecordStorage raw;
    ItemRecordStorage readValue;
    ItemRecordStorage growInitial;
    ItemRecordStorage growValue;
    ItemRecordStorage noGrowValue;
    struct Header { unsigned short count; unsigned char rest[6]; } header;
    memcpy(&header, input, 8);
    unsigned int count = header.count;
    src = input + 8;
    // Phase 1: grow/fill or shrink to count; capacity remains on shrink.
    if (count > self->size) {
        Allocator8018AD5C allocator;
        extra = count - self->size;
        // if: preserve fill-before-growth block order.
        do {
            if (extra > self->capacity || self->size > self->capacity - extra) {
                recommend8018BB2C(self->capacity, *(volatile unsigned int*)&self->size + extra - self->capacity);
            } else {
                ItemLoadEntry* dest = self->begin + self->size;
                Allocator8018AD5C allocator;
                noGrowValue = noGrowInitial;
                while (extra != 0) {
                    construct8018BB2C(dest, noGrowValue, allocator);
                    --extra;
                    ++dest;
                    ++self->size;
                }
                break;
            }

            {
                int target;
                unsigned int cap;
                unsigned int oldSize = self->size;
                unsigned int oldCap = self->capacity;
                Resize8018BB2C next(&self->capacity);
                unsigned int needed = oldSize + extra - self->capacity;
                cap = self->capacity;
                if (needed > kItemMaxEntries - cap) lengthError8018BB2C();
                if (cap < kItemSmallGrowth) {
                    unsigned int increment = ((cap + 1) * 3) / 5;
                    target = cap + max8018AD5C(increment, needed);
                } else if (cap < kItemHalfGrowth) {
                    unsigned int increment = (cap + 1) / 2;
                    target = cap + max8018AD5C(increment, needed);
                } else target = kItemMaxEntries;
                next.begin = (ItemLoadEntry*)allocate8018BB2C(target);
                next.remaining = self->size;
                ItemLoadEntry* dest = next.begin + self->size + next.size;
                next.capacity = target;
                Allocator8018AD5C allocator;
                growValue = growInitial;
                while (extra != 0) {
                    construct8018BB2C(dest, growValue, allocator);
                    --extra;
                    ++dest;
                    ++next.size;
                }
                // Phase 2: copy prefix and commit; next then owns the old buffer.
                moveRecords8018BB2C(self, next);
                self->size = 0;
                ItemLoadEntry* oldBegin = self->begin;
                ItemLoadEntry* newBegin = next.begin;
                unsigned int newSize = next.size;
                unsigned int oldCapacity = self->capacity;
                next.begin = oldBegin;
                Allocator8018AD5C swapAllocator;
                self->capacity = next.capacity;
                next.capacity = oldCapacity;
                self->begin = newBegin;
                self->size = newSize;
                next.size = 0;
            }
        } while (0);
    } else if (count < self->size) {
        Allocator8018AD5C allocator;
        self->size -= self->size - count;
    }

    // Phase 3: overwrite all count entries from fixed-stride packed item bytes.
    Index8018AD5C index;
    int i = 0;
    while (i < header.count) {
        memcpy(&raw, src, 0x60);
        src += 0x60;
        readValue = raw;
        self->begin[index].assign(readValue, raw.value.common.typeId);
        ++index;
        ++i;
    }
}

struct Resize8018B460 {
    BallLoadEntry* begin;
    unsigned int size;
    unsigned int capacity;
    unsigned int* ownerCapacity;
    unsigned int remaining;
    // Start an empty ball resize transaction linked to owner capacity.
    Resize8018B460(unsigned int* owner)
        : begin(0), size(0), capacity(0), ownerCapacity(owner), remaining(0) {}
    // Free the uncommitted buffer, or the old buffer swapped here at commit.
    ~Resize8018B460() {
        if (begin) {
            Allocator8018AD5C allocator;
            size -= size;
            appFreeUni__FPv(begin);
        }
    }
};

// Construct and throw the ball allocator's length_error.
inline void lengthError8018B460() {
    Allocator8018AD5C allocator;
    LengthThrow8018B460 error;
}

// Recommend a ball capacity using the shared three-tier growth rule.
inline unsigned int recommend8018B460(const unsigned int& capacity, unsigned int needed) {
    Allocator8018AD5C allocator;
    unsigned int cap = capacity;
    if (needed > kBallMaxEntries - cap) lengthError8018B460();
    if (cap < kBallSmallGrowth) {
        unsigned int increment = ((cap + 1) * 3) / 5;
        const unsigned int* amount = &increment;
        if (increment < needed) amount = &needed;
        return cap + *amount;
    } else if (cap < kBallHalfGrowth) {
        unsigned int increment = (cap + 1) / 2;
        const unsigned int* amount = &increment;
        if (increment < needed) amount = &needed;
        return cap + *amount;
    }
    return kBallMaxEntries;
}

// Allocate owned storage for count ball entries, or throw on size overflow.
inline void* allocate8018B460(unsigned int count) {
    Allocator8018AD5C allocator;
    if (count > kBallMaxEntries) lengthError8018B460();
    return appMallocDummy__FUl(count * 0x2C4);
}
// Placement-construct a tagged ball entry in already allocated storage.
inline void construct8018B460(BallLoadEntry* dest, const BallRecordStorage& value, const Allocator8018AD5C&) {
    new (dest) BallLoadEntry(value);
}
// Copy one ball entry into the transaction's reserved prefix.
inline void copyConstruct8018B460(BallLoadEntry* dest, const BallLoadEntry& value, const Allocator8018AD5C&) {
    new (dest) BallLoadEntry(value);
}

// Resize the owned ball table, then replace every entry from borrowed IFF bytes.
void ItemInventory_ReadBallTable(BallTableLoader* self, const unsigned char* input) {
    unsigned int extra;
    const unsigned char* src;
    BallRecordStorage raw;
    BallRecordStorage readValue;
    BallRecordStorage noGrowInitial;
    BallRecordStorage growInitial;
    BallRecordStorage growValue;
    BallRecordStorage noGrowValue;
    // Phase 1: read the unvalidated native-endian header, then match logical
    // size to count; a shrink retains the existing allocation and capacity.
    struct Header { unsigned short count; unsigned char rest[6]; } header;
    memcpy(&header, input, 8);
    unsigned int count = header.count;
    src = input + 8;
    if (count > self->size) {
        Allocator8018AD5C allocator;
        extra = count - self->size;
        // if: preserve fill-before-growth block order.
        do {
            if (extra > self->capacity || self->size > self->capacity - extra) {
                recommend8018B460(self->capacity, *(volatile unsigned int*)&self->size + extra - self->capacity);
            } else {
                BallLoadEntry* dest = self->begin + self->size;
                Allocator8018AD5C allocator;
                noGrowValue = noGrowInitial;
                while (extra != 0) {
                    construct8018B460(dest, noGrowValue, allocator);
                    --extra;
                    ++dest;
                    ++self->size;
                }
                break;
            }

            {
                unsigned int target;
                unsigned int cap;
                unsigned int needed = self->size + extra - self->capacity;
                Resize8018B460 next(&self->capacity);
                cap = self->capacity;
                if (needed > kBallMaxEntries - cap) lengthError8018B460();
                if (cap < kBallSmallGrowth) {
                    unsigned int increment = ((cap + 1) * 3) / 5;
                    target = cap + max8018AD5C(increment, needed);
                } else if (cap < kBallHalfGrowth) {
                    unsigned int increment = (cap + 1) / 2;
                    target = cap + max8018AD5C(increment, needed);
                } else target = kBallMaxEntries;
                next.begin = (BallLoadEntry*)allocate8018B460(target);
                next.remaining = self->size;
                BallLoadEntry* dest = next.begin + self->size + next.size;
                next.capacity = target;
                Allocator8018AD5C allocator;
                growValue = growInitial;
                while (extra != 0) {
                    construct8018B460(dest, growValue, allocator);
                    --extra;
                    ++dest;
                    ++next.size;
                }
                // Phase 2: copy the old prefix backwards and commit ownership.
                BallLoadEntry* first = self->begin;
                BallLoadEntry* old = first + self->size;
                BallLoadEntry* moved = next.begin + next.remaining;
                Allocator8018AD5C moveAllocator;
                while (old > first) {
                    --moved;
                    --old;
                    copyConstruct8018B460(moved, *old, moveAllocator);
                    --next.remaining;
                    ++next.size;
                }
                self->size = 0;
                BallLoadEntry* oldBegin = self->begin;
                BallLoadEntry* newBegin = next.begin;
                unsigned int newSize = next.size;
                unsigned int oldCapacity = self->capacity;
                next.begin = oldBegin;
                Allocator8018AD5C swapAllocator;
                self->capacity = next.capacity;
                next.capacity = oldCapacity;
                self->begin = newBegin;
                self->size = newSize;
                next.size = 0;
            }
        } while (0);
    } else if (count < self->size) {
        Allocator8018AD5C allocator;
        self->size -= self->size - count;
    }

    // Phase 3: overwrite all count entries from fixed-stride packed ball bytes.
    int index = 0;
    int i = 0;
    while (i < header.count) {
        memcpy(&raw, src, 0x2C0);
        src += 0x2C0;
        readValue = raw;
        self->begin[index].assign(readValue, raw.value.common.typeId);
        ++index;
        ++i;
    }
}

} // extern "C"
