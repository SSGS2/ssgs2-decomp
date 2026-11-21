typedef unsigned long new_size_t;
#include "classes/CItemManager.h"
#include "classes/CSwapEndianFormat.h"
#include "iff/IffStruct.h"
// Translation unit 0x8018839C-0x801898A8 (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/exception.h"
// Construct records in caller-provided storage; allocation is handled by their vector.
inline void* operator new(new_size_t, void* p) { return p; }

extern "C" {

void* memcpy(void*, const void*, unsigned long);
void* memset(void*, int, unsigned long);
unsigned long strlen(const char*);
char* strcpy(char*, const char*);

void* appMallocDummy__FUl(unsigned long);
void appFreeUni__FPv(void*);
void* __nwa__FUl(unsigned long);
void* __nw__FUl(unsigned long);
void* SharedFile_Create(const char*, int);
int CSharedFilePtr_GetLoadedBuffer(void*);
// Pointer-shaped ABI declarations below carry the byte count in one 32-bit word;
// GetSize's definition in shared_file_loading.cpp returns an integer, not a string address.
const char* CSharedFilePtr_GetSize(void*);
void* Archive_Open(const char*, const char*, int);
void Archive_GetEntryInfo(void*, int, void*);
void Archive_Close(void*);
void CSharedFileHandle_Destroy(void*, short);
void __dla__FPv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern const char iffSchemaAndCourseStrings[];

// Limits derive from the 32-bit allocation size divided by the keyed record stride.
enum {
    kMaxDescriptionEntries = 0x5555555U,
    kSmallDescriptionGrowthLimit = 0x1C71C71U,
    kMediumDescriptionGrowthLimit = 0x38E38E2U,
    kMaxCharacterEntries = 0xE38E38U,
    kSmallCharacterGrowthLimit = 0x4BDA12U,
    kMediumCharacterGrowthLimit = 0x97B424U,
    kIffArchiveReadMode = 3,
    kIffFormatContext = 1,
    kArchiveRootEntry = -1
};

struct IffSchemaStringPool {
    char archiveName[0xB];
    char headerFormat[4];
    char characterFile[0xe];
    char characterFormat[0x44];
    char partFile[0x9];
    char partFormat[0x3f];
    char clubFile[0x9];
    char clubFormat[0x28];
    char clubSetFile[0xc];
    char clubSetFormat[0x31];
    char ballFile[0x9];
    char ballFormat[0x35];
    char itemFile[0x9];
    char itemFormat[0x23];
    char caddieFile[0xb];
    char caddieFormat[0x28];
    char caddieItemFile[0xf];
    char caddieItemFormat[0x27];
    char setItemFile[0xc];
    char setItemFormat[0x28];
    char courseFile[0xb];
    char courseFormat[0x3d];
    char hairStyleFile[0xe];
    char hairStyleFormat[0x25];
    char auxPartFile[0xc];
    char auxPartFormat[0x31];
    char descriptionFile[0x9];
    char descriptionFormat[0x6];
    char vectorLengthError[0x14];
};
enum {
    kIffArchiveName = IFF_OFFSET(IffSchemaStringPool, archiveName),
    kIffHeaderFormat = IFF_OFFSET(IffSchemaStringPool, headerFormat),
    kIffCharacterFile = IFF_OFFSET(IffSchemaStringPool, characterFile),
    kIffCharacterFormat = IFF_OFFSET(IffSchemaStringPool, characterFormat),
    kIffPartFile = IFF_OFFSET(IffSchemaStringPool, partFile),
    kIffPartFormat = IFF_OFFSET(IffSchemaStringPool, partFormat),
    kIffClubFile = IFF_OFFSET(IffSchemaStringPool, clubFile),
    kIffClubFormat = IFF_OFFSET(IffSchemaStringPool, clubFormat),
    kIffClubSetFile = IFF_OFFSET(IffSchemaStringPool, clubSetFile),
    kIffClubSetFormat = IFF_OFFSET(IffSchemaStringPool, clubSetFormat),
    kIffBallFile = IFF_OFFSET(IffSchemaStringPool, ballFile),
    kIffBallFormat = IFF_OFFSET(IffSchemaStringPool, ballFormat),
    kIffItemFile = IFF_OFFSET(IffSchemaStringPool, itemFile),
    kIffItemFormat = IFF_OFFSET(IffSchemaStringPool, itemFormat),
    kIffCaddieFile = IFF_OFFSET(IffSchemaStringPool, caddieFile),
    kIffCaddieFormat = IFF_OFFSET(IffSchemaStringPool, caddieFormat),
    kIffCaddieItemFile = IFF_OFFSET(IffSchemaStringPool, caddieItemFile),
    kIffCaddieItemFormat = IFF_OFFSET(IffSchemaStringPool, caddieItemFormat),
    kIffSetItemFile = IFF_OFFSET(IffSchemaStringPool, setItemFile),
    kIffSetItemFormat = IFF_OFFSET(IffSchemaStringPool, setItemFormat),
    kIffCourseFile = IFF_OFFSET(IffSchemaStringPool, courseFile),
    kIffCourseFormat = IFF_OFFSET(IffSchemaStringPool, courseFormat),
    kIffHairStyleFile = IFF_OFFSET(IffSchemaStringPool, hairStyleFile),
    kIffHairStyleFormat = IFF_OFFSET(IffSchemaStringPool, hairStyleFormat),
    kIffAuxPartFile = IFF_OFFSET(IffSchemaStringPool, auxPartFile),
    kIffAuxPartFormat = IFF_OFFSET(IffSchemaStringPool, auxPartFormat),
    kIffDescriptionFile = IFF_OFFSET(IffSchemaStringPool, descriptionFile),
    kIffDescriptionFormat = IFF_OFFSET(IffSchemaStringPool, descriptionFormat),
    kIffVectorLengthError = IFF_OFFSET(IffSchemaStringPool, vectorLengthError)
};

extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];

struct VectorAllocation {
    char* pointer; char* allocator;
    // Guard a new exception-message allocation until its control block owns it.
    VectorAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Release the message only if ownership has not been transferred.
    ~VectorAllocation() { if (pointer) __dla__FPv(pointer); }
};
struct VectorControlBase {
    void* table; unsigned strong, weak;
    // Initialize the message control block with one strong and one weak reference.
    VectorControlBase() { strong = 1; weak = 1; table = s_sharedCharDeleterVtable; }
};
struct VectorControl : VectorControlBase {
    char* pointer;
    // Attach the allocated message bytes to their reference-counted deleter.
    VectorControl(char* p) { pointer = p; }
};
namespace std { struct logic_error : exception {
    // Replace the exception's dynamic type table during staged construction.
    void SetTable(void* table) { *(void**)this = table; }
    // Initialize the logic_error base before the concrete length_error.
    logic_error() { SetTable(__vt__Q23std11logic_error); }
    ~logic_error();
}; }
} // extern "C"

namespace std {
struct length_error : logic_error {
    char* text; void* control;
    // Build the standard vector-length exception with a shared copy of the pooled message.
    length_error() {
        const char* message = iffSchemaAndCourseStrings + kIffVectorLengthError;
        struct Allocator { char tag; char pad[3]; } allocator;
        allocator.tag = 0;
        unsigned n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            VectorAllocation allocation(p, &allocator.tag);
            control = new VectorControl(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        SetTable(__vt__Q23std12length_error);
    }

    ~length_error();
};
}
extern "C" {
struct ElemVector;
// Eight-byte native-endian IFF header; count controls the following packed records.
struct Header8839C { unsigned short count, bindingId; unsigned int version; };
// Return a reference to the larger growth term, preserving the vector template's evaluation.
inline const unsigned& MaxRef8839C(const unsigned& a, const unsigned& b) { return a < b ? b : a; }
void Archive_ReadEntry(void*, int, void*, unsigned, int);
void CSwapEndianFormat_ctor(void*);
void CSwapEndianFormat_SetContext(void*, int);
void CSwapEndianFormat_RegisterFormat(void*, const char*, unsigned, unsigned);
void CSwapEndianFormat_CopyAndSwap(void*, void*, void*, unsigned);
void CSwapEndianFormat_dtor(void*, short);
int String_CompareCString(const char*, const char*);
// Compare an archive member name using the game's string comparator (zero means equal).
static inline int CompareIffMemberName(const char* actual, const char* expected) {
    return String_CompareCString(actual, expected);
}
void ItemInventory_ReadCharacterTable(ElemVector*, const unsigned char*);
void ItemInventory_ReadPartTable(void*, void*, unsigned);
void ItemInventory_ReadClubTable(void*, void*, unsigned);
void ItemInventory_ReadClubSetTable(void*, void*, unsigned);
void ItemInventory_ReadBallTable(void*, void*, unsigned);
void ItemInventory_ReadItemTable(void*, void*, unsigned);
void ItemInventory_ReadCaddieTable(void*, void*, unsigned);
void ItemInventory_ReadCaddieItemTable(void*, void*, unsigned);
void ItemInventory_ReadSetItemTable(void*, void*, unsigned);
void ItemInventory_ReadCourseTable(void*, void*, unsigned);
void ItemInventory_ReadHairStyleTable(void*, void*, unsigned);
void ItemInventory_ReadAuxPartTable(void*, void*, unsigned);
struct Reader8839C {
    CSwapEndianFormat format;
    // Begin an endian-format context for this archive member.
    Reader8839C() { CSwapEndianFormat_ctor(this); }
    // Tear down registered formats on normal exit or exception unwinding.
    ~Reader8839C() { CSwapEndianFormat_dtor(this, -1); }
};
struct DescLoadEntry {
    unsigned lookupKey;
    typedef IffRecordStorage<IFF_STRUCT::sDesc> Payload;
    Payload body;
    // Leave scratch storage uninitialized; real records are assigned before publication.
    DescLoadEntry() {}
    // Copy the payload with a reset lookup key (unlike CharacterLoadEntry's copy below).
    DescLoadEntry(const DescLoadEntry& x) : lookupKey(0), body(x.body) {}
    // Wrap a description payload with a zero lookup key.
    DescLoadEntry(const Payload& x) : lookupKey(0), body(x) {}
};
// Reallocation transaction: data is owned temporary storage, offset is the old-prefix
// boundary not yet copied, and size counts constructed entries. After the swap, data
// owns the old allocation and the destructor frees it. owner refers to capacity metadata.
struct Buffer8839C {
    DescLoadEntry* data; volatile unsigned size; unsigned capacity; unsigned* owner; volatile unsigned offset;
    // Begin an empty, scoped allocation transaction for the destination vector.
    Buffer8839C(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Free whichever allocation remains owned by the transaction.
    ~Buffer8839C() { if (data) { size = size - size; appFreeUni__FPv(data); } }
};
// Contiguous keyed records; size/capacity are entry counts, not byte counts.
struct Vector8839C { DescLoadEntry* data; unsigned size, capacity; };
#pragma use_lmw_stmw on
#pragma opt_strength_reduction off
#pragma opt_loop_invariants off
#pragma opt_propagation off
// Load every recognized pangya.iff archive member, endian-swap its records, and populate item tables.
// self is a valid CItemManager. Blocking busy-wait; no timeout or member-size validation.
// Return 0 only when Archive_Open fails, otherwise 1 (unrecognized members are skipped).
// A valid member has an eight-byte header and count complete records in the registered schema.
// Replaces logical sizes in the existing manager storage; exceptions may leave a partial load.
int ItemInventory_LoadIffArchive(void* self) {
    __alloca(0);
    // Acquire loaded archive bytes, then open the in-memory archive before clearing tables.
    void* sharedFile = SharedFile_Create(iffSchemaAndCourseStrings, 0);
    while (!CSharedFilePtr_GetLoadedBuffer(sharedFile)) {}
    char* archiveBytes = (char*)CSharedFilePtr_GetLoadedBuffer(sharedFile);
    const char* archiveSize = CSharedFilePtr_GetSize(sharedFile);
    void* archive = Archive_Open(archiveBytes, archiveSize, kIffArchiveReadMode);
    // Preserve asymmetric failure cleanup: bytes are deleted here, but the handle is not destroyed.
    if (!archive) { __dla__FPv(archiveBytes); return 0; }
    CItemManager* manager = (CItemManager*)self;
    // Reset logical contents while retaining allocations. Club-set size is not reset here;
    // its recognized-member reader controls it. Preserve this list when changing reload behavior.
    manager->characters.size -= manager->characters.size;
    manager->parts.size -= manager->parts.size;
    manager->clubs.size -= manager->clubs.size;
    manager->balls.size -= manager->balls.size;
    manager->items.size -= manager->items.size;
    manager->caddies.size -= manager->caddies.size;
    manager->caddieItems.size -= manager->caddieItems.size;
    manager->setItems.size -= manager->setItems.size;
    manager->courses.size -= manager->courses.size;
    manager->hairStyles.size -= manager->hairStyles.size;
    manager->descriptions.size -= manager->descriptions.size;
    manager->auxParts.size -= manager->auxParts.size;
    manager->lockEnabled = 0;
    struct Properties { int count; char name[0x124]; unsigned size; char tail[8]; } properties;
    DescLoadEntry inPlaceDefault, reallocationDefault;
    Archive_GetEntryInfo(archive, kArchiveRootEntry, &properties);
    DescLoadEntry inPlaceFill(inPlaceDefault), reallocationFill(reallocationDefault);
    int count = properties.count;
    // Each iteration owns one temporary member buffer and one scoped endian-format reader.
    for (int archiveEntryIndex = 0; archiveEntryIndex < count; ++archiveEntryIndex) {
        Archive_GetEntryInfo(archive, archiveEntryIndex, &properties);
        unsigned char* data = (unsigned char*)__nwa__FUl(properties.size);
        Archive_ReadEntry(archive, archiveEntryIndex, data, properties.size, kIffArchiveReadMode);
        Reader8839C reader;
        CSwapEndianFormat_SetContext(&reader, kIffFormatContext);
        const char* strings = iffSchemaAndCourseStrings;
        CSwapEndianFormat_RegisterFormat(&reader, strings + kIffHeaderFormat, sizeof(Header8839C), 1);
        // Decode count first, then register the selected payload schema and swap in place.
        Header8839C header;
        CSwapEndianFormat_CopyAndSwap(&reader, &header, data, sizeof(Header8839C));
        if (!CompareIffMemberName(properties.name, strings + kIffCharacterFile)) {
            CSwapEndianFormat_RegisterFormat(&reader, strings + kIffCharacterFormat, sizeof(IFF_STRUCT::sChar), header.count);
            CSwapEndianFormat_CopyAndSwap(&reader, data, data, properties.size);
            ((void (*)(void*, void*, unsigned))ItemInventory_ReadCharacterTable)(self, data, properties.size);
        } else
        if (!CompareIffMemberName(properties.name, strings + kIffPartFile)) {
            CSwapEndianFormat_RegisterFormat(&reader, strings + kIffPartFormat, sizeof(IFF_STRUCT::sPart), header.count);
            CSwapEndianFormat_CopyAndSwap(&reader, data, data, properties.size);
            ItemInventory_ReadPartTable(self, data, properties.size);
        } else
        if (!CompareIffMemberName(properties.name, strings + kIffClubFile)) {
            CSwapEndianFormat_RegisterFormat(&reader, strings + kIffClubFormat, sizeof(IFF_STRUCT::sClub), header.count);
            CSwapEndianFormat_CopyAndSwap(&reader, data, data, properties.size);
            ItemInventory_ReadClubTable(self, data, properties.size);
        } else
        if (!CompareIffMemberName(properties.name, strings + kIffClubSetFile)) {
            CSwapEndianFormat_RegisterFormat(&reader, strings + kIffClubSetFormat, sizeof(IFF_STRUCT::sClubSet), header.count);
            CSwapEndianFormat_CopyAndSwap(&reader, data, data, properties.size);
            ItemInventory_ReadClubSetTable(self, data, properties.size);
        } else
        if (!CompareIffMemberName(properties.name, strings + kIffBallFile)) {
            CSwapEndianFormat_RegisterFormat(&reader, strings + kIffBallFormat, sizeof(IFF_STRUCT::sBall), header.count);
            CSwapEndianFormat_CopyAndSwap(&reader, data, data, properties.size);
            ItemInventory_ReadBallTable(self, data, properties.size);
        } else
        if (!CompareIffMemberName(properties.name, strings + kIffItemFile)) {
            CSwapEndianFormat_RegisterFormat(&reader, strings + kIffItemFormat, sizeof(IFF_STRUCT::sItem), header.count);
            CSwapEndianFormat_CopyAndSwap(&reader, data, data, properties.size);
            ItemInventory_ReadItemTable(self, data, properties.size);
        } else
        if (!CompareIffMemberName(properties.name, strings + kIffCaddieFile)) {
            CSwapEndianFormat_RegisterFormat(&reader, strings + kIffCaddieFormat, sizeof(IFF_STRUCT::sCaddie), header.count);
            CSwapEndianFormat_CopyAndSwap(&reader, data, data, properties.size);
            ItemInventory_ReadCaddieTable(self, data, properties.size);
        } else
        if (!CompareIffMemberName(properties.name, strings + kIffCaddieItemFile)) {
            CSwapEndianFormat_RegisterFormat(&reader, strings + kIffCaddieItemFormat, sizeof(IFF_STRUCT::sCadItem), header.count);
            CSwapEndianFormat_CopyAndSwap(&reader, data, data, properties.size);
            ItemInventory_ReadCaddieItemTable(self, data, properties.size);
        } else
        if (!CompareIffMemberName(properties.name, strings + kIffSetItemFile)) {
            CSwapEndianFormat_RegisterFormat(&reader, strings + kIffSetItemFormat, sizeof(IFF_STRUCT::sSetItem), header.count);
            CSwapEndianFormat_CopyAndSwap(&reader, data, data, properties.size);
            ItemInventory_ReadSetItemTable(self, data, properties.size);
        } else
        if (!CompareIffMemberName(properties.name, strings + kIffCourseFile)) {
            CSwapEndianFormat_RegisterFormat(&reader, strings + kIffCourseFormat, sizeof(IFF_STRUCT::sCourse), header.count);
            CSwapEndianFormat_CopyAndSwap(&reader, data, data, properties.size);
            ItemInventory_ReadCourseTable(self, data, properties.size);
        } else
        if (!CompareIffMemberName(properties.name, strings + kIffHairStyleFile)) {
            CSwapEndianFormat_RegisterFormat(&reader, strings + kIffHairStyleFormat, sizeof(IFF_STRUCT::sHairStyle), header.count);
            CSwapEndianFormat_CopyAndSwap(&reader, data, data, properties.size);
            ItemInventory_ReadHairStyleTable(self, data, properties.size);
        } else
        if (!CompareIffMemberName(properties.name, strings + kIffAuxPartFile)) {
            CSwapEndianFormat_RegisterFormat(&reader, strings + kIffAuxPartFormat, sizeof(IFF_STRUCT::sAuxPart), header.count);
            CSwapEndianFormat_CopyAndSwap(&reader, data, data, properties.size);
            ItemInventory_ReadAuxPartTable(self, data, properties.size);
        } else
        if (!CompareIffMemberName(properties.name, strings + kIffDescriptionFile)) {
            CSwapEndianFormat_RegisterFormat(&reader, strings + kIffDescriptionFormat, sizeof(IFF_STRUCT::sDesc), header.count);
            CSwapEndianFormat_CopyAndSwap(&reader, data, data, properties.size);
            // Description-table specialization: resize keyed entries, then overwrite each payload/key.
            Header8839C descriptionHeader;
            memcpy(&descriptionHeader, data, sizeof(Header8839C));
            const unsigned char* source = data + sizeof(Header8839C);
            Vector8839C* descriptions = (Vector8839C*)&manager->descriptions;
            if (descriptionHeader.count > descriptions->size) {
                unsigned add = descriptionHeader.count - descriptions->size;
                if (add > descriptions->capacity || descriptions->size > descriptions->capacity - add) {
                    // Reserve enough entries using the original 3/5 or 1/2 growth policy.
                    unsigned capacity = descriptions->capacity;
                    unsigned needed = descriptions->size + add - capacity;
                    if (needed > kMaxDescriptionEntries - capacity) throw std::length_error();
                    if (capacity < kSmallDescriptionGrowthLimit) {} else if (capacity < kMediumDescriptionGrowthLimit) {}
                    Buffer8839C temp(&descriptions->capacity);
                    needed = descriptions->size + add - descriptions->capacity;
                    capacity = descriptions->capacity;
                    if (needed > kMaxDescriptionEntries - capacity) throw std::length_error();
                    if (capacity < kSmallDescriptionGrowthLimit) capacity += MaxRef8839C(3 * (capacity + 1) / 5, needed);
                    else if (capacity < kMediumDescriptionGrowthLimit) capacity += MaxRef8839C((capacity + 1) / 2, needed);
                    else capacity = kMaxDescriptionEntries;
                    if (capacity > kMaxDescriptionEntries) throw std::length_error();
                    DescLoadEntry* allocation = (DescLoadEntry*)appMallocDummy__FUl(capacity * sizeof(DescLoadEntry));
                    unsigned oldSize = descriptions->size;
                    temp.data = allocation; temp.capacity = capacity; temp.offset = oldSize;
                    // Construct the new suffix first; copy the old prefix backward into its final slots.
                    DescLoadEntry* out = allocation + oldSize + temp.size;
                    while (add != 0) { if (out) new (out) DescLoadEntry(reallocationFill); ++temp.size; --add; ++out; }
                    DescLoadEntry* old = descriptions->data;
                    DescLoadEntry* end = old + descriptions->size;
                    DescLoadEntry* dest = temp.data + temp.offset;
                    while (end > old) { --dest; --end; if (dest) *dest = *end; --temp.offset; ++temp.size; }
                    // Commit the replacement; temp now owns and frees the previous allocation.
                    DescLoadEntry* previous = descriptions->data;
                    DescLoadEntry* replacement = temp.data;
                    unsigned size = temp.size, oldCapacity = descriptions->capacity;
                    descriptions->size = 0; temp.data = previous;
                    descriptions->capacity = temp.capacity; temp.capacity = oldCapacity;
                    descriptions->data = replacement; descriptions->size = size; temp.size = 0;
                } else {
                    // Existing capacity suffices: append directly and advance logical size.
                    DescLoadEntry* out = descriptions->data + descriptions->size;
                    while (add != 0) { if (out) new (out) DescLoadEntry(inPlaceFill); ++descriptions->size; --add; ++out; }
                }
            } else if (descriptionHeader.count < descriptions->size) descriptions->size = descriptions->size - (descriptions->size - descriptionHeader.count);
            // The destination now has exactly header.count slots; source records omit the lookup key.
            for (int descriptionIndex = 0; descriptionIndex < descriptionHeader.count; ++descriptionIndex) {
                DescLoadEntry::Payload body;
                memcpy(&body, source, sizeof(IFF_STRUCT::sDesc)); source += sizeof(IFF_STRUCT::sDesc);
                DescLoadEntry copy;
                copy.body = body;
                copy.lookupKey = copy.body.value.typeId;
                descriptions->data[descriptionIndex] = copy;
            }
        }
        // Readers have copied their records; the per-member byte buffer can be released.
        __dla__FPv(data);
    }
    // Release archive state before destroying the shared-file handle backing its bytes.
    Archive_Close(archive);
    if (sharedFile) CSharedFileHandle_Destroy(sharedFile, 1);
    return 1;
}
#pragma opt_strength_reduction reset
#pragma opt_loop_invariants reset
#pragma opt_propagation reset
}

extern "C" {
typedef IffRecordStorage<IFF_STRUCT::sChar> CharacterRecordStorage;
struct CharacterLoadEntry {
    int lookupKey;
    CharacterRecordStorage body;
    // Leave scratch/default-fill storage uninitialized, as in the original vector specialization.
    CharacterLoadEntry() {}
    // Wrap a disk record, leaving its lookup key to be filled separately.
    CharacterLoadEntry(const CharacterRecordStorage& record) { lookupKey = 0; body = record; }
    // Preserve both key and payload when relocating an existing entry.
    CharacterLoadEntry(const CharacterLoadEntry& o) { lookupKey = o.lookupKey; body = o.body; }
};

struct ElemVector {
    char managerPrefix[0x20]; // CItemManager prefix before the character table.
    CharacterLoadEntry* data;
    unsigned int size;
    unsigned int capacity;
};

struct Header {
    unsigned short count, bindingId;
    unsigned int version;
};

// Same transaction/ownership convention as Buffer8839C, specialized for keyed character records.
// The vector owns entry storage; payloads are inline records, with no separately owned buffers.
struct ElemBuffer {
    CharacterLoadEntry* data; volatile unsigned size; unsigned capacity; unsigned* owner; volatile unsigned offset;
    // Begin an empty scoped character-vector allocation.
    ElemBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Release the uncommitted allocation, or the old allocation after a successful swap.
    ~ElemBuffer() { if (data) { size = 0; appFreeUni__FPv(data); } }
};

// Choose the larger requested growth term by reference.
inline const unsigned& MaxRef(const unsigned& a, const unsigned& b) { return a < b ? b : a; }

#pragma use_lmw_stmw on
// Resize the keyed character table and copy each character record from an already-swapped IFF member.
// self points to the MANAGER BASE (ElemVector includes the 0x20-byte manager prefix),
// not to the vector subobject. input is borrowed: eight-byte native-endian header followed
// by count sChar records. No byte length is checked. Storage is retained/grown, never transferred in.
void ItemInventory_ReadCharacterTable(ElemVector* self, const unsigned char* input) {
    Header header;
    memcpy(&header, input, sizeof(Header));
    const unsigned char* source = input + sizeof(Header);

    // Resize phase: reserve only when the existing capacity cannot hold the requested count.
    if (header.count > self->size) {
        unsigned int add = header.count - self->size;
        CharacterLoadEntry fill;
        if (add > self->capacity || self->size > self->capacity - add) {
            unsigned int capacity = self->capacity;
            unsigned int needed = self->size + add - capacity;
            if (needed > kMaxCharacterEntries - capacity) throw std::length_error();
            if (capacity < kSmallCharacterGrowthLimit) {
            } else if (capacity < kMediumCharacterGrowthLimit) {
            }
            ElemBuffer temp(&self->capacity);
            capacity = self->capacity;
            if (needed > kMaxCharacterEntries - capacity) throw std::length_error();
            if (capacity < kSmallCharacterGrowthLimit) {
                capacity += MaxRef((3 * (capacity + 1)) / 5, needed);
            } else if (capacity < kMediumCharacterGrowthLimit) {
                capacity += MaxRef((capacity + 1) / 2, needed);
            } else {
                capacity = kMaxCharacterEntries;
            }
            if (capacity > kMaxCharacterEntries) throw std::length_error();
            // Construct the appended suffix before relocating the old prefix backward.
            CharacterLoadEntry* data = (CharacterLoadEntry*)appMallocDummy__FUl(capacity * sizeof(CharacterLoadEntry));
            unsigned int oldSize = self->size;
            temp.data = data;
            temp.capacity = capacity;
            temp.offset = oldSize;
            CharacterLoadEntry x(fill.body);
            CharacterLoadEntry* out = data + oldSize + temp.size;
            for (; add != 0; --add) {
                if (out) new (out) CharacterLoadEntry(x);
                ++temp.size;
                ++out;
            }
            CharacterLoadEntry* oldData = self->data;
            CharacterLoadEntry* srcEnd = oldData + self->size;
            CharacterLoadEntry* dstEnd = temp.data + temp.offset;
            while (srcEnd > oldData) {
                --dstEnd;
                --srcEnd;
                if (dstEnd) new (dstEnd) CharacterLoadEntry(*srcEnd);
                --temp.offset;
                ++temp.size;
            }
            // Commit: swap allocations/metadata; the transaction releases old storage on exit.
            CharacterLoadEntry* replacement = temp.data;
            CharacterLoadEntry* previousData = self->data;
            unsigned newSize = temp.size;
            unsigned oldCapacity = self->capacity;
            self->size = 0;
            temp.data = previousData;
            self->capacity = temp.capacity;
            temp.capacity = oldCapacity;
            self->data = replacement;
            self->size = newSize;
            temp.size = 0;
        } else {
            // Append in place when no reallocation is needed.
            CharacterLoadEntry x(fill.body);
            CharacterLoadEntry* out = self->data + self->size;
            for (; add != 0; --add) {
                if (out) new (out) CharacterLoadEntry(x);
                ++self->size;
                ++out;
            }
        }
    } else if (header.count < self->size) {
        // Shrink logical size only; allocation capacity is retained.
        self->size = self->size - (self->size - header.count);
    }

    // Populate phase: overwrite every slot with a packed payload and a key copied from common.typeId.
    for (int recordIndex = 0, outputIndex = 0; recordIndex < header.count; ++recordIndex, ++outputIndex) {
        CharacterLoadEntry character;
        // Byte-addressed copy preserves CodeWarrior's original copy scheduling.
        memcpy((char*)&character + IFF_OFFSET(CharacterLoadEntry, body), source, sizeof(IFF_STRUCT::sChar));
        source += sizeof(IFF_STRUCT::sChar);
        character.lookupKey = character.body.value.common.typeId;
        self->data[outputIndex] = character;
    }
}

} // extern "C"
