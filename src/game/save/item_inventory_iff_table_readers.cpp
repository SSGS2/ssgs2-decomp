#include "iff/IffStruct.h"
// Translation unit 0x8018C1F8-0x8018D65C (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "game/exception.h"

// Inventory table import contract (all three readers below):
// input is borrowed, already in native byte order, and contains an 8-byte header
// followed by recordCount complete records of the selected IFF type. No byte-length
// validation or endian conversion occurs here. The destination owns its allocation;
// importing may invalidate every previously borrowed pointer into that table.
// Entries are [type-id key, IFF payload]; size/capacity count entries, not bytes.
// Resize first, then overwrite every entry from input. Temporary default payloads
// are deliberately uninitialized until that final copy; they are not default items.
// Growth uses 3/5, then 1/2, then the maximum representable allocation size.
// Header of each native-order IFF table; only recordCount is consumed here.
struct IffTableHeader {
    unsigned short recordCount;
    unsigned char metadata[6];
};
// Size derivation (key word is part of the allocation stride):
// table        payload + key  max=floor(UINT_MAX/stride)  fast/half thresholds
// caddie       0x90 + 4      0x01bacf91                  0x939a85 / 0x127350a
// caddie item  0x88 + 4      0x01d41d41                  0x9c09c0 / 0x1381380
// set item     0x80 + 4      0x01f07c1f                  0xa57eb5 / 0x14afd6a
// Fast and half thresholds are floor(max/3) and 2*floor(max/3). They select
// delta=max(requiredGrowth, floor(3*(old+1)/5)), then floor((old+1)/2),
// then max itself. A final max check catches growth that exceeds byte capacity.
// The three expanded readers share this policy but retain separate constructor,
// unwind and copy paths; their statement order is part of the matching output.
static const unsigned int kCaddieMaxEntries = 0x01BACF91U;
static const unsigned int kCaddieFastGrowthLimit = 0x939A85U;
static const unsigned int kCaddieHalfGrowthLimit = 0x0127350AU;
static const unsigned int kCaddieItemMaxEntries = 0x01D41D41U;
static const unsigned int kCaddieItemFastGrowthLimit = 0x9C09C0U;
static const unsigned int kCaddieItemHalfGrowthLimit = 0x01381380U;
static const unsigned int kSetItemMaxEntries = 0x01F07C1FU;
static const unsigned int kSetItemFastGrowthLimit = 0xA57EB5U;
static const unsigned int kSetItemHalfGrowthLimit = 0x014AFD6AU;
static const unsigned int kVectorLengthErrorOffset = 0x2e5;
static const unsigned int kLengthErrorTypeOffset = 0x2f9;

extern "C" {

struct CaddieRecordStorage {
    union { IFF_STRUCT::sCaddie value; unsigned int words[sizeof(IFF_STRUCT::sCaddie) / 4]; };
    // Leave record bytes uninitialized until table import.
    CaddieRecordStorage() {}
    // Copy every byte of the source IFF record using its value assignment.
    CaddieRecordStorage(const CaddieRecordStorage& other) { *this = other; }
};
struct CaddieLoadEntry {
    unsigned int tag;
    CaddieRecordStorage data;
    // Construct a zero-key entry from the supplied caddie payload.
    CaddieLoadEntry(const CaddieRecordStorage& value) : tag(0), data(value) {}
    // Copy the existing key and caddie payload.
    CaddieLoadEntry(const CaddieLoadEntry& other) : tag(other.tag), data(other.data) {}
};

struct CaddieTableLoader {
    unsigned char padBeforeTable[0x68];
    CaddieLoadEntry* begin;
    unsigned int size;
    unsigned int capacity;
};

void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
extern void* memcpy(void*, const void*, unsigned int);

struct SetItemRecordStorage {
    union { IFF_STRUCT::sSetItem value; unsigned int words[sizeof(IFF_STRUCT::sSetItem) / 4]; };
    // Leave record bytes uninitialized until table import.
    SetItemRecordStorage() {}
    // Copy every byte of the source IFF record using its value assignment.
    SetItemRecordStorage(const SetItemRecordStorage& other) { *this = other; }
};
struct SetItemLoadEntry {
    unsigned int tag;
    SetItemRecordStorage data;
    // Construct a zero-key entry from the supplied set-item payload.
    SetItemLoadEntry(const SetItemRecordStorage& value) try : tag(0), data(value) {} catch (...) { throw; }
    // Copy the existing entry key and set-item payload.
    SetItemLoadEntry(const SetItemLoadEntry& other) try : tag(other.tag), data(other.data) {} catch (...) { throw; }
    // Construct an entry with the explicit key and supplied set-item payload.
    SetItemLoadEntry(unsigned int t, const SetItemRecordStorage& value) : tag(t), data(value) {}
};

struct SetItemTableLoader {
    unsigned char padBeforeTable[0x80];
    SetItemLoadEntry* begin;
    unsigned int size;
    unsigned int capacity;
};

extern "C" {
extern char iffSchemaAndCourseStrings[];
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
void* __nwa__FUl(unsigned long);
void* __nw__FUl(unsigned long);
void __dla__FPv(void*, int);
void __dt__Q23std12length_errorFv();
void __dt__Q33std6detail16char_array_guardFv(void*, int);
}

namespace std {
struct logic_error : exception {
    // Set the exception object vtable used by the runtime.
    void SetTable(const void* vt) { *(const void**)this = vt; }

    char* text;
    void* control;

    ~logic_error();
};
}
struct BufferGuard_u8018C1F8 {
    char* data;
    char* flag;
    // Release the error-text array if control-block construction did not take ownership.
    ~BufferGuard_u8018C1F8() { if (data) __dla__FPv(data, -1); }
};
struct Control {
    void* vtable;
    unsigned int refs, weak;
    char* text;
    // Own the error-text array with initial strong/weak counts of one.
    Control(char* p) : refs(1), weak(1) { vtable=s_sharedCharDeleterVtable; text=p; }
};
namespace std {
struct length_error : logic_error {
    // Construct the owned length-error text and its shared control block.
    length_error(const char* str) {
        *(void**)this=__vt__Q23std11logic_error;
        struct Flag { char value; char pad[3]; };
        Flag flag; flag.value=0;
        unsigned long n = strlen(str)+1;
        char* allocated=(char*)__nwa__FUl(2*n-n);
        text=allocated;
        {
        BufferGuard_u8018C1F8 guard;
        guard.data=allocated;
        guard.flag=&flag.value;
        control=new Control(allocated);
        guard.data=0;
        }
        strcpy(text,str);
        *(void**)this=__vt__Q23std12length_error;
    }

    ~length_error();
};
}
// Throw the vector length error for an impossible entry count.
inline void capacityError(const char* message) { throw std::length_error(message); }

#pragma does_not_return __throw
struct LengthThrow : std::length_error {
    // Build and transfer a length-error object to the exception runtime.
    LengthThrow(const char* message) : std::length_error(message) {
        __throw(iffSchemaAndCourseStrings + kLengthErrorTypeOffset, this, (void*)__dt__Q23std12length_errorFv);
    }
};
// Construct in caller-owned storage without allocating.
inline void* operator new(size_t, void* p) { return p; }
// Throw the set-item specialization's vector length error.
inline void throwLength(const char* const& message) {
    throw std::length_error(message);
}

// Growth guards own pending storage, then the old allocation after commit.
// size counts constructed entries; prefix counts old entries not yet copied.
// destinationCapacity borrows the destination capacity counter.
struct Allocation90 {
    CaddieLoadEntry* begin;
    unsigned int size;
    unsigned int capacity;
    unsigned int* destinationCapacity; // Borrowed &destination->capacity.
    unsigned int prefix;
    // Initialize an empty growth guard borrowing the destination capacity counter.
    Allocation90(unsigned int* a) : begin(0), size(0), capacity(0), destinationCapacity(a), prefix(0) {}
    // Free the pending or swapped-out allocation still owned by this guard.
    ~Allocation90() {
        if (begin) {
            size = 0;
            appFreeUni__FPv(begin);
        }
    }
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Resize the caddie lookup table and copy each IFF record with its type-id key.
void ItemInventory_ReadCaddieTable(CaddieTableLoader* self, const unsigned char* input) {
    IffTableHeader header;
    memcpy(&header, input, sizeof(header));
    const unsigned char* recordCursor = input + sizeof(IffTableHeader);
    // Phase 1: resize the entry array to the header count without reading payloads.
    if (header.recordCount > self->size) {
        unsigned int entriesToAppend = header.recordCount - self->size;
        unsigned int availableCapacity = self->capacity;
        // Resize in place when possible; otherwise build new entries, copy old ones,
        // and swap allocations only after all constructors succeed.
        do {
            if (entriesToAppend > availableCapacity || self->size > availableCapacity - entriesToAppend) {
                unsigned int oldCapacity = self->capacity;
                if (self->size + entriesToAppend - availableCapacity > kCaddieMaxEntries - oldCapacity)
                    capacityError(iffSchemaAndCourseStrings + kVectorLengthErrorOffset);
                // Retained empty threshold branches: no mutation or payload initialization.
                // The replacement-allocation block below computes the actual growth.
                if (oldCapacity < kCaddieFastGrowthLimit) {
                } else if (oldCapacity < kCaddieHalfGrowthLimit) {
                }
            } else {
                {
                    CaddieLoadEntry* destination = self->begin + self->size;
                    // Stage only the entry lifetime/key; import below supplies every payload byte.
                    CaddieRecordStorage initial;
                    CaddieRecordStorage value = initial;
                    while (entriesToAppend != 0) {
                        new (destination) CaddieLoadEntry(value);
                        --entriesToAppend;
                        ++destination;
                        ++self->size;
                    }
                }
                break;
            }
            {
                unsigned int requiredGrowth = self->size + entriesToAppend - self->capacity;
                Allocation90 allocation(&self->capacity);
                unsigned int oldCapacity = self->capacity;
                if (requiredGrowth > kCaddieMaxEntries - oldCapacity)
                    capacityError(iffSchemaAndCourseStrings + kVectorLengthErrorOffset);
                unsigned int newCapacity;
                if (oldCapacity < kCaddieFastGrowthLimit) {
                    unsigned int growth = ((oldCapacity + 1) * 3) / 5;
                    const unsigned int* amount = &growth;
                    amount = (growth >= requiredGrowth) ? amount : &requiredGrowth;
                    newCapacity = oldCapacity + *amount;
                } else if (oldCapacity < kCaddieHalfGrowthLimit) {
                    unsigned int growth = (oldCapacity + 1) >> 1;
                    const unsigned int* amount = &growth;
                    if (growth < requiredGrowth) amount = &requiredGrowth;
                    newCapacity = oldCapacity + *amount;
                } else {
                    newCapacity = kCaddieMaxEntries;
                }
                if (newCapacity > kCaddieMaxEntries)
                    capacityError(iffSchemaAndCourseStrings + kVectorLengthErrorOffset);
                // Allocate a replacement; self still owns the live table.
                allocation.begin = (CaddieLoadEntry*)appMallocDummy__FUl(newCapacity * sizeof(CaddieLoadEntry));
                allocation.capacity = newCapacity;
                allocation.prefix = self->size;
                CaddieLoadEntry* destination = allocation.begin + self->size + allocation.size;
                CaddieRecordStorage initial;
                CaddieRecordStorage value = initial;
                while (entriesToAppend != 0) {
                    new (destination) CaddieLoadEntry(value);
                    --entriesToAppend;
                    ++destination;
                    ++allocation.size;
                }
                // Copy the old prefix backward; the guard tracks constructed entries.
                CaddieLoadEntry* oldCursor = self->begin + self->size;
                CaddieLoadEntry* copyDestination = allocation.begin + allocation.prefix;
                while (oldCursor > self->begin) {
                    --copyDestination;
                    --oldCursor;
                    new (copyDestination) CaddieLoadEntry(*oldCursor);
                    --allocation.prefix;
                    ++allocation.size;
                }
                // Commit: publish new storage; the guard now frees the old allocation.
                CaddieLoadEntry* oldBegin = self->begin;
                CaddieLoadEntry* newBegin = allocation.begin;
                unsigned int newSize = allocation.size;
                self->size = 0;
                unsigned int previousCapacity = self->capacity;
                allocation.begin = oldBegin;
                self->capacity = allocation.capacity;
                allocation.capacity = previousCapacity;
                self->begin = newBegin;
                self->size = newSize;
                allocation.size = 0;
            }
        } while (0);
    } else if (header.recordCount < self->size) {
        self->size -= self->size - header.recordCount;
    }
    // Resize invariant: size equals recordCount; payloads may still contain staging bytes.
    // Phase 2: overwrite all live entries with type-id keys and native-order payloads.
    unsigned int index = 0;
    int i = 0;
    while (i < header.recordCount) {
        CaddieRecordStorage raw;
        memcpy(&raw, recordCursor, sizeof(CaddieRecordStorage));
        CaddieRecordStorage value = raw;
        recordCursor += sizeof(CaddieRecordStorage);
        CaddieLoadEntry* destination = self->begin + index;
        destination->tag = raw.value.common.typeId;
        destination->data = value;
        ++index;
        ++i;
    }
}

#pragma pop

typedef IffRecordStorage<IFF_STRUCT::sCadItem> CaddieItemRecordStorage;
struct CaddieItemLoadEntry {
    unsigned int tag;
    CaddieItemRecordStorage data;
    // Construct a zero-key entry or copy an existing key and caddie-item payload.
    CaddieItemLoadEntry(const CaddieItemRecordStorage& value) : tag(0), data(value) {}
    // Construct a zero-key entry or copy an existing key and caddie-item payload.
    CaddieItemLoadEntry(const CaddieItemLoadEntry& other) : tag(other.tag), data(other.data) {}
};
struct CaddieItemTableLoader {
    unsigned char padBeforeTable[0x74];
    CaddieItemLoadEntry* begin;
    unsigned int size;
    unsigned int capacity;
};
struct Allocation88 {
    CaddieItemLoadEntry* begin;
    unsigned int size;
    unsigned int capacity;
    unsigned int* destinationCapacity; // Borrowed &destination->capacity.
    unsigned int prefix;
    // Initialize an empty growth guard borrowing the destination capacity counter.
    Allocation88(unsigned int* a) : begin(0), size(0), capacity(0), destinationCapacity(a), prefix(0) {}
    // Free the pending or swapped-out allocation still owned by this guard.
    ~Allocation88() {
        if (begin) {
            size = 0;
            appFreeUni__FPv(begin);
        }
    }
};

#pragma push
#pragma use_lmw_stmw on
// Resize the caddie-item lookup table and copy each IFF record with its type-id key.
void ItemInventory_ReadCaddieItemTable(CaddieItemTableLoader* self, const unsigned char* input) {
    union Allocator { volatile char value; unsigned int alignment; };
    IffTableHeader header;
    memcpy(&header, input, sizeof(header));
    unsigned int count = header.recordCount;
    const unsigned char* recordCursor = input + sizeof(IffTableHeader);
    unsigned int currentSize = self->size;
    // Phase 1: resize the entry array to the header count without reading payloads.
    if (count > currentSize) {
        unsigned int availableCapacity = self->capacity;
        unsigned int entriesToAppend = count - currentSize;
        Allocator allocator; allocator.value = 0;
        // Resize in place when possible; otherwise build new entries, copy old ones,
        // and swap allocations only after all constructors succeed.
        do {
            if (entriesToAppend > availableCapacity || self->size > availableCapacity - entriesToAppend) {
                unsigned int size = ((volatile CaddieItemTableLoader*)self)->size;
                unsigned int oldCapacity = ((volatile CaddieItemTableLoader*)self)->capacity;
                Allocator allocator2; allocator2.value = 0;
                if (size + entriesToAppend - availableCapacity > kCaddieItemMaxEntries - oldCapacity) {
                    Allocator allocator3; allocator3.value = 0;
                    const char* volatile message = iffSchemaAndCourseStrings + kVectorLengthErrorOffset;
                    capacityError(message);
                }
                // Retained empty threshold branches: no mutation or payload initialization.
                // The replacement-allocation block below computes the actual growth.
                if (oldCapacity < kCaddieItemFastGrowthLimit) {
                } else if (oldCapacity < kCaddieItemHalfGrowthLimit) {
                }
            } else {
                {
                    CaddieItemLoadEntry* destination = self->begin + self->size;
                    // Stage only the entry lifetime/key; import below supplies every payload byte.
                    CaddieItemRecordStorage initial;
                    CaddieItemRecordStorage value = initial;
                    while (entriesToAppend != 0) {
                        new (destination) CaddieItemLoadEntry(value);
                        --entriesToAppend;
                        ++destination;
                        ++self->size;
                    }
                }
                break;
            }
            {
                unsigned int requiredGrowth = self->size + entriesToAppend - self->capacity;
                Allocation88 allocation(&self->capacity);
                unsigned int oldCapacity = self->capacity;
                if (requiredGrowth > kCaddieItemMaxEntries - oldCapacity)
                    capacityError(iffSchemaAndCourseStrings + kVectorLengthErrorOffset);
                unsigned int newCapacity;
                if (oldCapacity < kCaddieItemFastGrowthLimit) {
                    unsigned int growth = ((oldCapacity + 1) * 3) / 5;
                    const unsigned int* amount = &growth;
                    amount = (growth >= requiredGrowth) ? amount : &requiredGrowth;
                    newCapacity = oldCapacity + *amount;
                } else if (oldCapacity < kCaddieItemHalfGrowthLimit) {
                    unsigned int growth = (oldCapacity + 1) >> 1;
                    const unsigned int* amount = &growth;
                    amount = (growth >= requiredGrowth) ? amount : &requiredGrowth;
                    newCapacity = oldCapacity + *amount;
                } else {
                    newCapacity = kCaddieItemMaxEntries;
                }
                if (newCapacity > kCaddieItemMaxEntries)
                    capacityError(iffSchemaAndCourseStrings + kVectorLengthErrorOffset);
                // Allocate a replacement; self still owns the live table.
                allocation.begin = (CaddieItemLoadEntry*)appMallocDummy__FUl(newCapacity * sizeof(CaddieItemLoadEntry));
                allocation.capacity = newCapacity;
                allocation.prefix = self->size;
                CaddieItemLoadEntry* destination = allocation.begin + self->size + allocation.size;
                CaddieItemRecordStorage initial;
                CaddieItemRecordStorage value = initial;
                while (entriesToAppend != 0) {
                    new (destination) CaddieItemLoadEntry(value);
                    --entriesToAppend;
                    ++destination;
                    ++allocation.size;
                }
                // Copy the old prefix backward; the guard tracks constructed entries.
                CaddieItemLoadEntry* oldCursor = self->begin + self->size;
                CaddieItemLoadEntry* copyDestination = allocation.begin + allocation.prefix;
                while (oldCursor > self->begin) {
                    --copyDestination;
                    --oldCursor;
                    new (copyDestination) CaddieItemLoadEntry(*oldCursor);
                    --allocation.prefix;
                    ++allocation.size;
                }
                // Commit: publish new storage; the guard now frees the old allocation.
                CaddieItemLoadEntry* oldBegin = self->begin;
                CaddieItemLoadEntry* newBegin = allocation.begin;
                unsigned int newSize = allocation.size;
                self->size = 0;
                unsigned int previousCapacity = self->capacity;
                allocation.begin = oldBegin;
                self->capacity = allocation.capacity;
                allocation.capacity = previousCapacity;
                self->begin = newBegin;
                self->size = newSize;
                allocation.size = 0;
            }
        } while (0);
    } else if (header.recordCount < self->size) {
        self->size -= self->size - header.recordCount;
    }
    // Resize invariant: size equals recordCount; payloads may still contain staging bytes.
    // Phase 2: overwrite all live entries with type-id keys and native-order payloads.
    unsigned int index = 0;
    int i = 0;
    while (i < header.recordCount) {
        CaddieItemRecordStorage raw;
        memcpy(&raw, recordCursor, sizeof(CaddieItemRecordStorage));
        CaddieItemRecordStorage value = raw;
        recordCursor += sizeof(CaddieItemRecordStorage);
        CaddieItemLoadEntry* destination = self->begin + index;
        destination->tag = raw.value.common.typeId;
        destination->data = value;
        ++index;
        ++i;
    }
}

#pragma pop

union Allocator80 { volatile char value; unsigned int alignment; };
// Return the larger growth count by reference, preserving the template ABI.
inline const unsigned int& max80(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
// Grow set-item capacity by 3/5, then 1/2, saturating at the allocator limit.
inline unsigned int growth80(unsigned int oldCapacity, const unsigned int& requiredGrowth) {
    if (oldCapacity < kSetItemFastGrowthLimit)
        return oldCapacity + max80(((oldCapacity + 1) * 3) / 5, requiredGrowth);
    else if (oldCapacity < kSetItemHalfGrowthLimit)
        return oldCapacity + max80((oldCapacity + 1) >> 1, requiredGrowth);
    else
        return kSetItemMaxEntries;
}
// Reduce the constructed-entry count; no payload owns separate storage.
inline void erase80(unsigned int& size, unsigned int amount) {
    size -= amount;
}
struct Allocation80 {
    SetItemLoadEntry* begin;
    unsigned int size;
    unsigned int capacity;
    unsigned int* destinationCapacity; // Borrowed &destination->capacity.
    unsigned int prefix;
    // Initialize an empty growth guard borrowing the destination capacity counter.
    Allocation80(unsigned int* a) : begin(0), size(0), capacity(0), destinationCapacity(a), prefix(0) {}
    // Free the pending or swapped-out allocation still owned by this guard.
    ~Allocation80() {
        if (begin) {
            Allocator80 flag; flag.value = 0;
            erase80(size, size);
            appFreeUni__FPv(begin);
        }
    }
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Resize the set-item lookup table and copy each IFF record with its type-id key.
void ItemInventory_ReadSetItemTable(SetItemTableLoader* self, const unsigned char* input) { unsigned int availableCapacity;

    unsigned int entriesToAppend;

    SetItemRecordStorage inplaceInitial;
    SetItemRecordStorage growInitial;
    const unsigned char* recordCursor;
    SetItemRecordStorage raw;
    SetItemRecordStorage growValue;
    IffTableHeader header;

    memcpy(&header, input, sizeof(header));
    recordCursor = ((input + sizeof(IffTableHeader)));
    // Phase 1: resize the entry array to the header count without reading payloads.
    if (header.recordCount > self->size) {
        typedef Allocator80 Allocator;

        entriesToAppend = header.recordCount - self->size;
        availableCapacity = self->capacity;
        Allocator allocator; allocator.value = 0;
        // Resize in place when possible; otherwise build new entries, copy old ones,
        // and swap allocations only after all constructors succeed.
        do {
            if (entriesToAppend > availableCapacity || self->size > availableCapacity - entriesToAppend) {
                unsigned int size = ((volatile SetItemTableLoader*)self)->size;
                unsigned int oldCapacity = ((volatile SetItemTableLoader*)self)->capacity;
                Allocator allocator2; allocator2.value = 0;
                if (size + entriesToAppend - availableCapacity > kSetItemMaxEntries - oldCapacity) {
                    Allocator allocator3; allocator3.value = 0;
                    throwLength(iffSchemaAndCourseStrings + kVectorLengthErrorOffset);
                }
                growth80(oldCapacity, entriesToAppend + size - availableCapacity);
            } else {
                {
                    Allocator allocator4; SetItemLoadEntry* destination = self->begin + self->size;
                    SetItemRecordStorage inplaceValue;
                    allocator4.value = 0; inplaceValue = inplaceInitial;
                    while (entriesToAppend) {
                        new (destination) SetItemLoadEntry(inplaceValue);
                        --entriesToAppend;
                        ++destination;
                        ++self->size;
                    }
                }
                break;
            }
            { Allocator80 allocator8;
                Allocation80 allocation(&self->capacity);
                unsigned int requiredGrowth = ((self->size + entriesToAppend)) - self->capacity;
                int oldCapacity = self->capacity;
                if (requiredGrowth > kSetItemMaxEntries - oldCapacity) {
                    Allocator allocator5; allocator5.value = 0;
                    throwLength(iffSchemaAndCourseStrings + kVectorLengthErrorOffset);
                }
                unsigned int newCapacity; newCapacity = growth80(oldCapacity, requiredGrowth);
                Allocator allocator6; allocator6.value = 0;
                if (newCapacity > kSetItemMaxEntries) {
                    Allocator allocator7; allocator7.value = 0;
                    throwLength(iffSchemaAndCourseStrings + kVectorLengthErrorOffset);
                }
                // Allocate replacement storage while self still owns the live table.
                allocation.begin = (SetItemLoadEntry*)appMallocDummy__FUl(newCapacity * sizeof(SetItemLoadEntry));
                allocation.prefix = self->size;
                allocation.capacity = newCapacity;
                 allocator8.value = 0;
                SetItemLoadEntry* destination = self->size + allocation.begin + allocation.size;
                growValue = growInitial;
                while (entriesToAppend) {
                    new (destination) SetItemLoadEntry(growValue);
                    --entriesToAppend;
                    ++destination;
                    allocation.size += 1;
                }
                Allocator80 allocator9; SetItemLoadEntry* first = self->begin;
                SetItemLoadEntry* oldCursor = first + self->size;
                allocator9.value = 0;
                // Copy the old prefix backward; the guard counts completed entries.
                SetItemLoadEntry* copyDestination = allocation.begin + allocation.prefix;
                while (first < oldCursor) {
                    --copyDestination;
                    --oldCursor;
                    new (copyDestination) SetItemLoadEntry(*oldCursor);
                    allocation.prefix = allocation.prefix - 1;
                    ++allocation.size;
                }
                Allocator80 allocator10;
                allocator10.value = 0;
                // Commit: publish new entries; the guard takes the old allocation.
                SetItemLoadEntry* newBegin = allocation.begin;
                SetItemLoadEntry* oldBegin = self->begin;

                self->size = 0;
                allocation.begin = oldBegin;
                unsigned int previousCapacity = self->capacity;
                self->capacity = allocation.capacity;
                allocation.capacity = previousCapacity;
                self->begin = newBegin;
                self->size = (allocation.size);
                allocation.size = 0;
            }
        } while (0);
    } else if (header.recordCount < self->size) {
        Allocator80 allocator11; allocator11.value = 0;
        self->size -= self->size - header.recordCount;
    }
    // Resize invariant: size equals recordCount; import replaces every staging byte.
    // Phase 2: overwrite the resized table with keyed native-order IFF records.
    int index = 0;
    int i = 0;
    while (i < header.recordCount) {
        memcpy(&raw, recordCursor, sizeof(SetItemRecordStorage));
        recordCursor += sizeof(SetItemRecordStorage);
        self->begin[index++] = SetItemLoadEntry(raw.value.common.typeId, raw);
        ++i;
    }
}

#pragma pop

} // extern "C"
