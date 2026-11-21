enum IffSchemaTextOffsets { kVectorLengthErrorText = 0x2E5 };
enum IffTableStorageLimits {
    kClubMaximumEntries = 0x01D41D41,
    kClubSmallGrowthLimit = 0x9C09C0,
    kClubMediumGrowthLimit = 0x01381380,
    kPartMaximumEntries = 0x9D89D8,
    kPartSmallGrowthLimit = 0x348348,
    kPartMediumGrowthLimit = 0x690690,
    kIffFileHeaderBytes = 8
};
// Growth limits are the allocator's byte ceiling divided by the in-memory entry stride.
// Item ids are 32-bit values carried in void* by the original C ABI; never dereference a key.
#include "classes/CItemManager.h"
#include "iff/IffStruct.h"
// Translation unit 0x801898A8-0x8018AD5C (23 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "game/exception.h"

inline void* operator new(size_t, void* p) throw() { return p; }
inline void operator delete(void*, void*) throw() {}

extern "C" {

// Return the table index of this item id, or -1 for a null or missing id.
int ItemInventory_FindCharacterIndex(CItemManager* table, void* typeId) {
    CharacterTableEntry* end;
    CharacterTableEntry* entry;
    int entryIndex;
    entryIndex = 0;
    if (typeId == 0) {
        return -1;
    }
    entry = table->characters.data;
    end = entry + table->characters.size;
    while (entry != end) {
        if ((unsigned int)typeId == entry->key) {
            return entryIndex;
        }
        ++entry;
        ++entryIndex;
    }
    return -1;
}

// Return the item id at a table index; invalid indices return zero.
void* ItemInventory_GetCharacterIdAt(CItemManager* table, int index) {
    CharacterTableEntry* entry = table->characters.data;
    if (index < 0 || (unsigned int)index >= table->characters.size) {
        return 0;
    }
    if (index != 0) {
        int blocksOfEight = (unsigned int)index >> 3;
        if (blocksOfEight != 0) {
            do { entry += 8; } while (--blocksOfEight);
        }
        index &= 7;
        if (index != 0) {
            do {
                ++entry;
            } while (--index);
        }
    }
    return (void*)entry->key;
}

// Return the table index of this item id, or -1 for a null or missing id.
int ItemInventory_FindPartIndex(CItemManager* table, void* typeId) {
    int entryIndex = 0;
    if (typeId == 0) {
        return -1;
    }
    int entryCount = table->parts.size;
    PartTableEntry* entry = table->parts.data;
    PartTableEntry* end = entry + entryCount;
    while (entry != end) {
        if ((unsigned int)typeId == entry->key) {
            return entryIndex;
        }
        ++entryIndex;
        ++entry;
    }
    return -1;
}

// Return the item id at a table index; invalid indices return zero.
void* ItemInventory_GetPartIdAt(CItemManager* table, int index) {
    PartTableEntry* entry = table->parts.data;
    if (index < 0 || (unsigned int)index >= table->parts.size) {
        return 0;
    }
    if (index != 0) {
        int blocksOfEight = (unsigned int)index >> 3;
        if (blocksOfEight != 0) {
            do { entry += 8; } while (--blocksOfEight);
        }
        index &= 7;
        if (index != 0) {
            do {
                ++entry;
            } while (--index);
        }
    }
    return (void*)entry->key;
}

// Return the table index of this item id, or -1 for a null or missing id.
int ItemInventory_FindItemIndex(CItemManager* table, void* typeId) {
    int entryIndex = 0;
    if (typeId == 0) {
        return -1;
    }

    ItemTableEntry* entry = table->items.data;
    ItemTableEntry* end = entry + (table->items.size);
    while (entry != end) {
        if ((unsigned int)typeId == entry->key) {
            return entryIndex;
        }
        entryIndex++;
        ++entry;
    }
    return -1;
}

// Return the item id at a table index; invalid indices return zero.
void* ItemInventory_GetItemIdAt(CItemManager* table, int index) {
    ItemTableEntry* entry = table->items.data;
    if (index < 0 || (unsigned int)index >= table->items.size) {
        return 0;
    }
    while (index != 0) {
        ++entry;
        --index;
    }
    return (void*)entry->key;
}

// Return the table index of this item id, or -1 for a null or missing id.
int ItemInventory_FindClubSetIndex(CItemManager* table, void* typeId) {
    int entryIndex = 0;
    if (typeId == 0) {
        return -1;
    }

    ClubSetTableEntry* entry = table->clubSets.data;
    unsigned int entryCount = table->clubSets.size; ClubSetTableEntry* end = (((entry + entryCount)));
    while (entry != end) {
        if ((unsigned int)typeId == entry->key) {
            return entryIndex;
        }
        entryIndex++;
        ++entry;
    }
    return -1;
}

// Return the item id at a table index; invalid indices return zero.
void* ItemInventory_GetClubSetIdAt(CItemManager* table, int index) {
    ClubSetTableEntry* entry = table->clubSets.data;
    if (index < 0 || (unsigned int)index >= table->clubSets.size) {
        return 0;
    }
    while (index != 0) {
        ++entry;
        --index;
    }
    return (void*)entry->key;
}

// Return the item id at a table index; invalid indices return zero.
void* ItemInventory_GetBallIdAt(CItemManager* table, int index) {
    BallTableEntry* entry = table->balls.data;
    if (index < 0 || (unsigned int)index >= table->balls.size) {
        return 0;
    }
    while (index != 0) {
        ++entry;
        --index;
    }
    return (void*)entry->key;
}

// Return the table index of this item id, or -1 for a null or missing id.
int ItemInventory_FindCaddieIndex(CItemManager* table, void* typeId) {
    int entryIndex = 0;
    if (typeId == 0) {
        return -1;
    }
    unsigned int entryCount = table->caddies.size;
    CaddieTableEntry* entry = table->caddies.data;
    CaddieTableEntry* end = entry + entryCount;
    while (entry != end) {
        if ((unsigned int)typeId == entry->key) {
            return entryIndex;
        }
        ++entryIndex;
        ++entry;
    }
    return -1;
}

// Return the item id at a table index; invalid indices return zero.
void* ItemInventory_GetCaddieIdAt(CItemManager* table, int index) {
    CaddieTableEntry* entry = table->caddies.data;
    if (index < 0 || (unsigned int)index >= table->caddies.size) {
        return 0;
    }
    while (index != 0) {
        ++entry;
        --index;
    }
    return (void*)entry->key;
}

// Return the table index of this item id, or -1 for a null or missing id.
int ItemInventory_FindBallIndex(CItemManager* table, void* typeId) {
    int entryIndex = 0;
    if (typeId == 0) {
        return -1;
    }
    BallTableEntry* entry = table->balls.data;
    unsigned int entryCount = table->balls.size;
    BallTableEntry* end = (entryCount + entry);
    while (end != entry) {
        if ((unsigned int)typeId == entry->key) {
            return entryIndex;
        }
        ++entryIndex;
        ++entry;
    }
    return -1;
}

// Return the table index of this item id, or -1 for a null or missing id.
int ItemInventory_FindCaddieItemIndex(CItemManager* table, void* typeId) {
    int entryIndex = 0;
    if (typeId == 0) {
        return -1;
    }
    unsigned int entryCount = table->caddieItems.size;
    CaddieItemTableEntry* entry = table->caddieItems.data;
    CaddieItemTableEntry* end = entry + entryCount;
    while (end != entry) {
        if ((unsigned int)typeId == entry->key) {
            return entryIndex;
        }
        ++entryIndex;
        ++entry;
    }
    return -1;
}

// Return the item id at a table index; invalid indices return zero.
void* ItemInventory_GetCaddieItemIdAt(CItemManager* table, int index) {
    CaddieItemTableEntry* entry = table->caddieItems.data;
    if (index < 0 || (unsigned int)index >= table->caddieItems.size) {
        return 0;
    }
    while (index != 0) {
        ++entry;
        --index;
    }
    return (void*)entry->key;
}

// Return the table index of this item id, or -1 for a null or missing id.
int ItemInventory_FindHairStyleIndex(CItemManager* table, void* typeId) { int entryIndex = 0;  

    if (typeId == 0) {
        return -1;
    }

    unsigned int entryCount = table->hairStyles.size;
    HairStyleTableEntry* entry = table->hairStyles.data; HairStyleTableEntry* end = ((entryCount + entry));
    while (entry != end) {
        if ((unsigned int)typeId == entry->key) {
            return entryIndex;
        }
        entryIndex += 1;
        entry++;
    }
    return -1;
}

// Return the item id at a table index; invalid indices return zero.
void* ItemInventory_GetHairStyleIdAt(CItemManager* table, int index) {
    HairStyleTableEntry* entry = table->hairStyles.data;
    if (index < 0 || (unsigned int)index >= table->hairStyles.size) {
        return 0;
    }
    while (index != 0) {
        ++entry;
        --index;
    }
    return (void*)entry->key;
}

// Return the table index of this item id, or -1 for a null or missing id.
int ItemInventory_FindAuxPartIndex(CItemManager* table, void* typeId) {
    int entryIndex = 0;
    if (typeId == 0) {
        return -1;
    }
    unsigned int entryCount = table->auxParts.size;
    AuxPartTableEntry* entry = table->auxParts.data;
    AuxPartTableEntry* end = entry + entryCount;
    while (entry != end) {
        if ((unsigned int)typeId == entry->key) {
            return entryIndex;
        }
        ++entryIndex;
        ++entry;
    }
    return -1;
}

// Return the item id at a table index; invalid indices return zero.
void* ItemInventory_GetAuxPartIdAt(CItemManager* table, int index) {
    AuxPartTableEntry* entry = table->auxParts.data;
    if (index < 0 || (unsigned int)index >= table->auxParts.size) {
        return 0;
    }
    while (index != 0) {
        ++entry;
        --index;
    }
    return (void*)entry->key;
}

// Return the item id at a table index; invalid indices return zero.
void* ItemInventory_GetSetItemIdAt(CItemManager* table, int index) {
    SetItemTableEntry* entry = table->setItems.data;
    if (index < 0 || (unsigned int)index >= table->setItems.size) {
        return 0;
    }
    while (index != 0) {
        ++entry;
        --index;
    }
    return (void*)entry->key;
}

// Return the table index of this item id, or -1 for a null or missing id.
int ItemInventory_FindCourseIndex(CItemManager* table, void* typeId) {
    unsigned int entryIndex = 0;
    if (!typeId) {
        return -1;
    }
    unsigned int entryCount = table->courses.size;
    CourseTableEntry* entry = table->courses.data;
    CourseTableEntry* end = (entry + entryCount);
    while (entry != end) {
        if ((unsigned int)typeId == entry->key) {
            return entryIndex;
        }
        ++entryIndex;
        ++entry;
    }
    return -1;
}

// Return the item id at a table index; invalid indices return zero.
void* ItemInventory_GetCourseIdAt(CItemManager* table, int index) {
    CourseTableEntry* entry = table->courses.data;
    if (index < 0 || (unsigned int)index >= table->courses.size) {
        return 0;
    }
    while (index != 0) {
        ++entry;
        --index;
    }
    return (void*)entry->key;
}

struct ClubRecordStorage {
    union { IFF_STRUCT::sClub value; unsigned int words[sizeof(IFF_STRUCT::sClub) / 4]; };
    ClubRecordStorage() {}
    ClubRecordStorage(const ClubRecordStorage& value) { *this = value; }
};

struct ClubLoadEntry {
    void* key;
    ClubRecordStorage data;
    ClubLoadEntry(const ClubRecordStorage& value) : key(0), data(value) {}
    ClubLoadEntry(const ClubLoadEntry& value) : key(value.key), data(value.data) {}
};

struct ClubTableLoader {
    unsigned char precedingTables[0x38];
    ClubLoadEntry* first;
    unsigned int size;
    unsigned int capacity;
};

void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv();
void* memcpy(void*, const void*, unsigned int);

struct PartRecordStorage {
    union { IFF_STRUCT::sPart value; unsigned int words[sizeof(IFF_STRUCT::sPart) / 4]; };
    PartRecordStorage() {}
    PartRecordStorage(const PartRecordStorage& value) { *this = value; }
};
struct PartLoadEntry {
    void* key;
    PartRecordStorage data;
    PartLoadEntry(const PartRecordStorage& value) : key(0), data(value) {}
    PartLoadEntry(const PartLoadEntry& value) : key(value.key), data(value.data) {}
};
struct PartTableLoader {
    unsigned char precedingTables[0x2C];
    PartLoadEntry* first;
    unsigned int size;
    unsigned int capacity;
};
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned long);
void __dla__FPv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
void __dt__Q23std12length_errorFv(void*, int);
extern char iffSchemaAndCourseStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

struct ErrorControl1A0 {
    void* vtable;
    int strong;
    int weak;
    char* text;
};

struct ErrorBuffer1A0 {
    char* data;
    unsigned char* allocator;
    ~ErrorBuffer1A0() { if (data) __dla__FPv(data); }
};

struct EmptyAllocator8C;

namespace std { struct logic_error : exception {
    void SetTable(void* table) { *(void**)this = table; }
    char* text;
    ErrorControl1A0* control;
    logic_error() {}
    logic_error(const char*, const EmptyAllocator8C&);
    ~logic_error();
}; }

struct EmptyAllocator8C;

} // extern "C"
namespace std {
struct length_error : logic_error {
    length_error(const char* message, const EmptyAllocator8C&);
inline length_error(const char* message) {
    unsigned char allocator = 0;
    const char* textValue = message;
    SetTable(__vt__Q23std11logic_error);
    unsigned int length = strlen(textValue) + 1;
    char* allocation = (char*)__nwa__FUl(length * 2 - length);
    text = allocation;
    {
        ErrorBuffer1A0 buffer;
        buffer.data = allocation;
        buffer.allocator = &allocator;
        ErrorControl1A0* fresh = (ErrorControl1A0*)__nw__FUl(0x10);
        if (fresh) {
            fresh->strong = 1;
            fresh->weak = 1;
            fresh->vtable = s_sharedCharDeleterVtable;
            fresh->text = allocation;
        }
        control = fresh;
        buffer.data = 0;
    }
    strcpy(text, textValue);
    SetTable(__vt__Q23std12length_error);
}
    ~length_error();
};
}
extern "C" {
inline void lengthError1A0();

inline void subtractSize8C(unsigned int* size, unsigned int amount);

struct Allocation1A0 {
    PartLoadEntry* first;
    unsigned int size;
    unsigned int capacity;
    unsigned int* allocator;
    unsigned int prefix;
    ~Allocation1A0() {
        if (first) {
            subtractSize8C(&size, size);
            ((void (*)(void*))appFreeUni__FPv)(first);
        }
    }
};

// Store a part record and its common item id, then advance the destination index.
inline int assignRecord1A0(PartTableLoader* self, int index, const PartRecordStorage& source, PartRecordStorage value) {
    PartLoadEntry* entry = (PartLoadEntry*)((unsigned char*)self->first + index * sizeof(PartLoadEntry));
    entry->key = (void*)source.value.common.typeId;
    entry->data = value;
    return index + 1;
}

struct Allocation8C {
    ClubLoadEntry* first;
    unsigned int size;
    unsigned int capacity;
    unsigned int* allocator;
    unsigned int prefix;
    ~Allocation8C() {
        if (first) {
            subtractSize8C(&size, size);
            ((void (*)(void*))appFreeUni__FPv)(first);
        }
    }
};

#pragma push
#pragma use_lmw_stmw on
#pragma inline_max_size(10000)
#pragma inline_max_total_size(10000)
#pragma inline_depth(1024)

struct EmptyAllocator8C {};

// Reduce the live element count while preserving the allocator lifetime used by this library build.
inline void subtractSize8C(unsigned int* size, unsigned int amount) {
    EmptyAllocator8C allocator = EmptyAllocator8C();
    *size -= amount;
}

// Transfer the exception message buffer into shared ownership, freeing it if allocation throws.
inline void makeErrorControl8C(ErrorControl1A0*& control, char* allocation, unsigned char* allocator) {
        ErrorBuffer1A0 buffer;
        buffer.data = allocation;
        buffer.allocator = allocator;
        ErrorControl1A0* fresh = (ErrorControl1A0*)__nw__FUl(0x10);
        if (fresh) {
            fresh->strong = 1;
            fresh->weak = 1;
            fresh->vtable = s_sharedCharDeleterVtable;
            fresh->text = allocation;
        }
        control = fresh;
        buffer.data = 0;
}

inline std::logic_error::logic_error(const char* message, const EmptyAllocator8C&) {
    const char* textValue = message;
    SetTable(__vt__Q23std11logic_error);
    EmptyAllocator8C allocator = {};
    unsigned int length = strlen(textValue) + 1;
    char* allocation = (char*)__nwa__FUl(length * 2 - length);
    text = allocation;
    makeErrorControl8C(control, allocation, (unsigned char*)&allocator);
    strcpy(text, textValue);
}

} // extern "C"
namespace std {
inline length_error::length_error(const char* message, const EmptyAllocator8C& allocator)
    : logic_error(message, allocator) {
    SetTable(__vt__Q23std12length_error);
}
}
extern "C" {

// Throw the original vector length error when a club table would exceed its allocator limit.
inline void lengthError8C() {
    EmptyAllocator8C allocator = {};
    throw std::length_error(iffSchemaAndCourseStrings + kVectorLengthErrorText, allocator);
}

// Append the requested number of default club entries, retaining the original uninitialized record payload.
inline void fillRecords8C(ClubLoadEntry* dst, unsigned int& extra, unsigned int& size) {
    EmptyAllocator8C fillAllocator = {};
    ClubRecordStorage uninitialized;
    ClubRecordStorage value = uninitialized;
    while (extra != 0) {
        new (dst) ClubLoadEntry(value);
        --extra;
        ++dst;
        ++size;
    }
}

// Return the larger required/growth element count without copying the allocator arguments.
inline const unsigned int& maxGrowth8C(const unsigned int& left, const unsigned int& right) {
    return left < right ? right : left;
}

// Choose club-table capacity with the original three-tier growth rule and overflow guard.
inline unsigned int growth8C(const unsigned int* current, unsigned int needed, bool initializeAllocator) {
    unsigned int result;
    unsigned int capacity = *current;
    if (initializeAllocator) {
        EmptyAllocator8C allocator = {};
    }
    if (needed > kClubMaximumEntries - capacity) lengthError8C();
    if (capacity < kClubSmallGrowthLimit) {
        unsigned int grown = ((capacity + 1) * 4 - (capacity + 1)) / 5;
        result = capacity + maxGrowth8C(grown, needed);
    } else if (capacity < kClubMediumGrowthLimit) {
        unsigned int grown = (capacity + 1) / 2;
        result = capacity + maxGrowth8C(grown, needed);
    } else result = kClubMaximumEntries;
    return result;
}

// Allocate storage for the checked number of in-memory club entries.
inline void reserveAllocation8C(Allocation8C* allocation, unsigned int capacity) {
    EmptyAllocator8C reserveAllocator = {};
    if (capacity > kClubMaximumEntries) lengthError8C();
    allocation->first = (ClubLoadEntry*)appMallocDummy__FUl(capacity * sizeof(ClubLoadEntry));
    allocation->capacity = capacity;
}

// Copy existing club entries backward into the reserved prefix, updating ownership counts.
inline void copyRecords8C(Allocation8C* allocation, ClubLoadEntry* oldBegin, ClubLoadEntry* old) {
    ClubLoadEntry* dest = allocation->first + allocation->prefix;
    EmptyAllocator8C copyAllocator = {};
    while (old > oldBegin) {
        --dest;
        --old;
        new (dest) ClubLoadEntry(*old);
        --allocation->prefix;
        ++allocation->size;
    }
}

inline int assignRecord8C(ClubTableLoader* self, int index, const ClubRecordStorage& source, ClubRecordStorage value) {
    ClubLoadEntry* entry = (ClubLoadEntry*)((unsigned char*)self->first + index * 0x8C);
    entry->key = (void*)source.value.common.typeId;
    entry->data = value;
    return index + 1;
}

// Resize the club table, preserving existing entries across growth and releasing the old allocation.
inline void resizeVector8C(ClubTableLoader* self, unsigned int count) {
    unsigned int newCapacity;
    unsigned int extra;
    // Original 0x801898A8 unit: in-place growth skips the reallocation path.
    do {
        if (count > self->size) {
            extra = count - self->size;
            EmptyAllocator8C allocator = {};
            if (extra > self->capacity || self->size > self->capacity - extra) {
                growth8C(&self->capacity, self->size + extra - self->capacity, true);
            } else {
                ClubLoadEntry* dst = self->first + self->size;
                fillRecords8C(dst, extra, self->size);
                break;
            }
            {
                unsigned int needed = self->size + extra - self->capacity;
                Allocation8C allocation;
                allocation.first = 0;
                allocation.size = 0;
                allocation.capacity = 0;
                allocation.allocator = &self->capacity;
                allocation.prefix = 0;
                newCapacity = growth8C(&self->capacity, needed, false);
                reserveAllocation8C(&allocation, newCapacity);
                allocation.prefix = self->size;
                ClubLoadEntry* out = allocation.size + (self->size + allocation.first);
                fillRecords8C(out, extra, allocation.size);
                copyRecords8C(&allocation, self->first, self->first + self->size);
                self->size = 0;
                ClubLoadEntry* oldFirst = self->first;
                ClubLoadEntry* newFirst = allocation.first;
                unsigned int newSize = allocation.size;
                unsigned int oldCapacity = self->capacity;
                allocation.first = oldFirst;
                EmptyAllocator8C swapAllocator = {};
                self->capacity = allocation.capacity;
                allocation.capacity = oldCapacity;
                self->first = newFirst;
                self->size = newSize;
                allocation.size = 0;
            }
        } else if (count < self->size) {
            unsigned int amount = self->size - count;
            EmptyAllocator8C shrinkAllocator = {};
            self->size -= amount;
        }
    } while (0);
}

// Resize the inventory table and copy each decoded IFF record, using common.typeId as its lookup key.
void ItemInventory_ReadClubTable(ClubTableLoader* self, unsigned char* tableBytes) {
    int recordIndex;
    int destinationIndex;
    unsigned char* recordBytes;
    struct { unsigned short count; unsigned char pad[6]; } header;
    memcpy(&header, tableBytes, kIffFileHeaderBytes);
    recordBytes = tableBytes + kIffFileHeaderBytes;
    resizeVector8C(self, header.count);
    destinationIndex = 0;
    for (recordIndex=0; recordIndex<header.count; ++recordIndex) {
        ClubRecordStorage packetValue;
        memcpy(&packetValue,recordBytes,0x88);
        recordBytes+=0x88;
        destinationIndex = assignRecord8C(self, destinationIndex, packetValue, packetValue);
    }
}

#pragma pop

// Throw the original vector length error when a part table would exceed its allocator limit.
inline void lengthError1A0() {
    EmptyAllocator8C allocator = {};
    throw std::length_error(iffSchemaAndCourseStrings + kVectorLengthErrorText, allocator);
}

// Append the requested number of default part entries, retaining the original uninitialized record payload.
inline void fillRecords1A0(PartLoadEntry* dst, unsigned int& extra, unsigned int& size) {
    EmptyAllocator8C fillAllocator = {};
    PartRecordStorage uninitialized;
    PartRecordStorage value = uninitialized;
    while (extra != 0) {
        new (dst) PartLoadEntry(value);
        --extra;
        ++dst;
        ++size;
    }
}

// Return the larger required/growth element count for part-table allocation.
inline const unsigned int& maxGrowth1A0(const unsigned int& left, const unsigned int& right) {
    return left < right ? right : left;
}

// Choose part-table capacity with the original three-tier growth rule and overflow guard.
inline unsigned int growth1A0(const unsigned int* current, unsigned int needed, bool initializeAllocator) {
    unsigned int result;
    unsigned int capacity = *current;
    if (initializeAllocator) {
        EmptyAllocator8C allocator = {};
    }
    if (needed > kPartMaximumEntries - capacity) lengthError1A0();
    if (capacity < kPartSmallGrowthLimit) {
        unsigned int grown = ((capacity + 1) * 4 - (capacity + 1)) / 5;
        result = capacity + maxGrowth1A0(grown, needed);
    } else if (capacity < kPartMediumGrowthLimit) {
        unsigned int grown = (capacity + 1) / 2;
        result = capacity + maxGrowth1A0(grown, needed);
    } else result = kPartMaximumEntries;
    return result;
}

// Allocate storage for the checked number of in-memory part entries.
inline void reserveAllocation1A0(Allocation1A0* allocation, unsigned int capacity) {
    EmptyAllocator8C reserveAllocator = {};
    if (capacity > kPartMaximumEntries) lengthError1A0();
    allocation->first = (PartLoadEntry*)appMallocDummy__FUl(capacity * sizeof(PartLoadEntry));
    allocation->capacity = capacity;
}

// Copy existing part entries backward into the reserved prefix, updating ownership counts.
inline void copyRecords1A0(Allocation1A0* allocation, PartLoadEntry* oldBegin, PartLoadEntry* old) {
    PartLoadEntry* dest = allocation->first + allocation->prefix;
    EmptyAllocator8C copyAllocator = {};
    while (old > oldBegin) {
        --dest;
        --old;
        new (dest) PartLoadEntry(*old);
        --allocation->prefix;
        ++allocation->size;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Resize the inventory table and copy each decoded IFF record, using common.typeId as its lookup key.
void ItemInventory_ReadPartTable(PartTableLoader* self, unsigned char* tableBytes) {
    unsigned int newCapacity;
    unsigned int extra;
    int recordIndex;
    int destinationIndex;
    unsigned char* recordBytes;
    struct { unsigned short count; unsigned char pad[6]; } header;
    memcpy(&header, tableBytes, kIffFileHeaderBytes);
    unsigned int count = header.count;
    recordBytes = tableBytes + kIffFileHeaderBytes;
    do {
        if (count > self->size) {
            extra = count - self->size;
            EmptyAllocator8C allocator = {};
            if (extra > self->capacity || self->size > self->capacity - extra) {
                growth1A0(&self->capacity, self->size + extra - self->capacity, true);
            } else {
                PartLoadEntry* dst = self->first + self->size;
                fillRecords1A0(dst, extra, self->size);
                break;
            }
            {
                unsigned int needed = self->size + extra - self->capacity;
                Allocation1A0 allocation;
                allocation.first = 0;
                allocation.size = 0;
                allocation.capacity = 0;
                allocation.allocator = &self->capacity;
                allocation.prefix = 0;
                newCapacity = growth1A0(&self->capacity, needed, false);
                reserveAllocation1A0(&allocation, newCapacity);
                allocation.prefix = self->size;
                PartLoadEntry* out = allocation.size + (self->size + allocation.first);
                fillRecords1A0(out, extra, allocation.size);
                copyRecords1A0(&allocation, self->first, self->first + self->size);
                self->size = 0;
                PartLoadEntry* oldFirst = self->first;
                PartLoadEntry* newFirst = allocation.first;
                unsigned int newSize = allocation.size;
                unsigned int oldCapacity = self->capacity;
                allocation.first = oldFirst;
                EmptyAllocator8C swapAllocator = {};
                self->capacity = allocation.capacity;
                allocation.capacity = oldCapacity;
                self->first = newFirst;
                self->size = newSize;
                allocation.size = 0;
            }
        } else if (count < self->size) {
            unsigned int amount = self->size - count;
            EmptyAllocator8C shrinkAllocator = {};
            self->size -= amount;
        }
    } while (0);
    destinationIndex = 0;
    for (recordIndex=0; recordIndex<header.count; ++recordIndex) {
        PartRecordStorage packetValue;
        memcpy(&packetValue,recordBytes,sizeof(IFF_STRUCT::sPart));
        recordBytes+=sizeof(IFF_STRUCT::sPart);
        destinationIndex = assignRecord1A0(self, destinationIndex, packetValue, packetValue);
    }
}
#pragma pop

} // extern "C"
