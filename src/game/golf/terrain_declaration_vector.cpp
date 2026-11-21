#include "MSL/error_message.h"
// Translation unit 0x80138554-0x801398B0 (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

void* operator new(size_t);
// Construct into caller-provided vector storage without allocating.
inline void* operator new(size_t, void* p) throw() { return p; }

extern "C" {

// Terrain-declaration records are produced by GolfParameters_LoadTerrainDeclarationsXml
// (golf_doc_turns_and_parameter_xml.cpp): nine words at +0x00..+0x20, a type byte at +0x24, then two
// 0x40-byte text buffers at +0x25 and +0x65 (wave extensions are stripped there).
// x/y are legacy float views of unresolved words 0/1, not verified coordinates.
// bNN fields below are private copy-schedule compatibility storage. For text edits
// use TerrainDeclarationTextView: firstText[64] and secondText[64], not individual bytes.
// The tail is the second text buffer. Explicit byte assignments retain the original
// copy schedule; replacing them with strcpy would change fixed-size/binary behavior.
struct EntryTail80138554 {
    unsigned char b65;
    unsigned char b66;
    unsigned char b67;
    unsigned char b68;
    unsigned char b69;
    unsigned char b6a;
    unsigned char b6b;
    unsigned char b6c;
    unsigned char b6d;
    unsigned char b6e;
    unsigned char b6f;
    unsigned char b70;
    unsigned char b71;
    unsigned char b72;
    unsigned char b73;
    unsigned char b74;
    unsigned char b75;
    unsigned char b76;
    unsigned char b77;
    unsigned char b78;
    unsigned char b79;
    unsigned char b7a;
    unsigned char b7b;
    unsigned char b7c;
    unsigned char b7d;
    unsigned char b7e;
    unsigned char b7f;
    unsigned char b80;
    unsigned char b81;
    unsigned char b82;
    unsigned char b83;
    unsigned char b84;
    unsigned char b85;
    unsigned char b86;
    unsigned char b87;
    unsigned char b88;
    unsigned char b89;
    unsigned char b8a;
    unsigned char b8b;
    unsigned char b8c;
    unsigned char b8d;
    unsigned char b8e;
    unsigned char b8f;
    unsigned char b90;
    unsigned char b91;
    unsigned char b92;
    unsigned char b93;
    unsigned char b94;
    unsigned char b95;
    unsigned char b96;
    unsigned char b97;
    unsigned char b98;
    unsigned char b99;
    unsigned char b9a;
    unsigned char b9b;
    unsigned char b9c;
    unsigned char b9d;
    unsigned char b9e;
    unsigned char b9f;
    unsigned char ba0;
    unsigned char ba1;
    unsigned char ba2;
    unsigned char ba3;
    unsigned char ba4;
    // Leave tail bytes untouched until the containing record copies them.
    EntryTail80138554() {}
    // Copy all 0x40 bytes, including bytes after any string terminator.
    EntryTail80138554(const EntryTail80138554& other) {
        b65 = other.b65;
        b66 = other.b66;
        b67 = other.b67;
        b68 = other.b68;
        b69 = other.b69;
        b6a = other.b6a;
        b6b = other.b6b;
        b6c = other.b6c;
        b6d = other.b6d;
        b6e = other.b6e;
        b6f = other.b6f;
        b70 = other.b70;
        b71 = other.b71;
        b72 = other.b72;
        b73 = other.b73;
        b74 = other.b74;
        b75 = other.b75;
        b76 = other.b76;
        b77 = other.b77;
        b78 = other.b78;
        b79 = other.b79;
        b7a = other.b7a;
        b7b = other.b7b;
        b7c = other.b7c;
        b7d = other.b7d;
        b7e = other.b7e;
        b7f = other.b7f;
        b80 = other.b80;
        b81 = other.b81;
        b82 = other.b82;
        b83 = other.b83;
        b84 = other.b84;
        b85 = other.b85;
        b86 = other.b86;
        b87 = other.b87;
        b88 = other.b88;
        b89 = other.b89;
        b8a = other.b8a;
        b8b = other.b8b;
        b8c = other.b8c;
        b8d = other.b8d;
        b8e = other.b8e;
        b8f = other.b8f;
        b90 = other.b90;
        b91 = other.b91;
        b92 = other.b92;
        b93 = other.b93;
        b94 = other.b94;
        b95 = other.b95;
        b96 = other.b96;
        b97 = other.b97;
        b98 = other.b98;
        b99 = other.b99;
        b9a = other.b9a;
        b9b = other.b9b;
        b9c = other.b9c;
        b9d = other.b9d;
        b9e = other.b9e;
        b9f = other.b9f;
        ba0 = other.ba0;
        ba1 = other.ba1;
        ba2 = other.ba2;
        ba3 = other.ba3;
        ba4 = other.ba4;
    }
};

struct Entry80138554 {
    float x, y;
    unsigned words[7];
    unsigned char b24;
    unsigned char b25;
    unsigned char b26;
    unsigned char b27;
    unsigned char b28;
    unsigned char b29;
    unsigned char b2a;
    unsigned char b2b;
    unsigned char b2c;
    unsigned char b2d;
    unsigned char b2e;
    unsigned char b2f;
    unsigned char b30;
    unsigned char b31;
    unsigned char b32;
    unsigned char b33;
    unsigned char b34;
    unsigned char b35;
    unsigned char b36;
    unsigned char b37;
    unsigned char b38;
    unsigned char b39;
    unsigned char b3a;
    unsigned char b3b;
    unsigned char b3c;
    unsigned char b3d;
    unsigned char b3e;
    unsigned char b3f;
    unsigned char b40;
    unsigned char b41;
    unsigned char b42;
    unsigned char b43;
    unsigned char b44;
    unsigned char b45;
    unsigned char b46;
    unsigned char b47;
    unsigned char b48;
    unsigned char b49;
    unsigned char b4a;
    unsigned char b4b;
    unsigned char b4c;
    unsigned char b4d;
    unsigned char b4e;
    unsigned char b4f;
    unsigned char b50;
    unsigned char b51;
    unsigned char b52;
    unsigned char b53;
    unsigned char b54;
    unsigned char b55;
    unsigned char b56;
    unsigned char b57;
    unsigned char b58;
    unsigned char b59;
    unsigned char b5a;
    unsigned char b5b;
    unsigned char b5c;
    unsigned char b5d;
    unsigned char b5e;
    unsigned char b5f;
    unsigned char b60;
    unsigned char b61;
    unsigned char b62;
    unsigned char b63;
    unsigned char b64;
    EntryTail80138554 tail;
    // Copy the complete declaration value without interpreting opaque word payloads.
    Entry80138554(const Entry80138554& other) {
        x = other.x;
        y = other.y;
        { unsigned a = other.words[0]; unsigned b = other.words[1]; words[0] = a; words[1] = b; }
        words[2] = other.words[2];
        { unsigned a = other.words[3]; unsigned b = other.words[4]; words[3] = a; words[4] = b; }
        words[5] = other.words[5];
        words[6] = other.words[6];
        b24 = other.b24;
        { unsigned char a = other.b25; unsigned char b = other.b26; b25 = a; b26 = b; }
        { unsigned char a = other.b27; unsigned char b = other.b28; b27 = a; b28 = b; }
        { unsigned char a = other.b29; unsigned char b = other.b2a; b29 = a; b2a = b; }
        { unsigned char a = other.b2b; unsigned char b = other.b2c; b2b = a; b2c = b; }
        { unsigned char a = other.b2d; unsigned char b = other.b2e; b2d = a; b2e = b; }
        { unsigned char a = other.b2f; unsigned char b = other.b30; b2f = a; b30 = b; }
        { unsigned char a = other.b31; unsigned char b = other.b32; b31 = a; b32 = b; }
        { unsigned char a = other.b33; unsigned char b = other.b34; b33 = a; b34 = b; }
        { unsigned char a = other.b35; unsigned char b = other.b36; b35 = a; b36 = b; }
        { unsigned char a = other.b37; unsigned char b = other.b38; b37 = a; b38 = b; }
        { unsigned char a = other.b39; unsigned char b = other.b3a; b39 = a; b3a = b; }
        { unsigned char a = other.b3b; unsigned char b = other.b3c; b3b = a; b3c = b; }
        { unsigned char a = other.b3d; unsigned char b = other.b3e; b3d = a; b3e = b; }
        { unsigned char a = other.b3f; unsigned char b = other.b40; b3f = a; b40 = b; }
        { unsigned char a = other.b41; unsigned char b = other.b42; b41 = a; b42 = b; }
        { unsigned char a = other.b43; unsigned char b = other.b44; b43 = a; b44 = b; }
        { unsigned char a = other.b45; unsigned char b = other.b46; b45 = a; b46 = b; }
        { unsigned char a = other.b47; unsigned char b = other.b48; b47 = a; b48 = b; }
        { unsigned char a = other.b49; unsigned char b = other.b4a; b49 = a; b4a = b; }
        { unsigned char a = other.b4b; unsigned char b = other.b4c; b4b = a; b4c = b; }
        { unsigned char a = other.b4d; unsigned char b = other.b4e; b4d = a; b4e = b; }
        { unsigned char a = other.b4f; unsigned char b = other.b50; b4f = a; b50 = b; }
        { unsigned char a = other.b51; unsigned char b = other.b52; b51 = a; b52 = b; }
        { unsigned char a = other.b53; unsigned char b = other.b54; b53 = a; b54 = b; }
        { unsigned char a = other.b55; unsigned char b = other.b56; b55 = a; b56 = b; }
        { unsigned char a = other.b57; unsigned char b = other.b58; b57 = a; b58 = b; }
        { unsigned char a = other.b59; unsigned char b = other.b5a; b59 = a; b5a = b; }
        { unsigned char a = other.b5b; unsigned char b = other.b5c; b5b = a; b5c = b; }
        { unsigned char a = other.b5d; unsigned char b = other.b5e; b5d = a; b5e = b; }
        { unsigned char a = other.b5f; unsigned char b = other.b60; b5f = a; b60 = b; }
        { unsigned char a = other.b61; unsigned char b = other.b62; b61 = a; b62 = b; }
        { unsigned char a = other.b63; unsigned char b = other.b64; b63 = a; b64 = b; }
        { unsigned char a = other.tail.b65; unsigned char b = other.tail.b66; tail.b65 = a; tail.b66 = b; }
        { unsigned char a = other.tail.b67; unsigned char b = other.tail.b68; tail.b67 = a; tail.b68 = b; }
        { unsigned char a = other.tail.b69; unsigned char b = other.tail.b6a; tail.b69 = a; tail.b6a = b; }
        { unsigned char a = other.tail.b6b; unsigned char b = other.tail.b6c; tail.b6b = a; tail.b6c = b; }
        { unsigned char a = other.tail.b6d; unsigned char b = other.tail.b6e; tail.b6d = a; tail.b6e = b; }
        { unsigned char a = other.tail.b6f; unsigned char b = other.tail.b70; tail.b6f = a; tail.b70 = b; }
        { unsigned char a = other.tail.b71; unsigned char b = other.tail.b72; tail.b71 = a; tail.b72 = b; }
        { unsigned char a = other.tail.b73; unsigned char b = other.tail.b74; tail.b73 = a; tail.b74 = b; }
        { unsigned char a = other.tail.b75; unsigned char b = other.tail.b76; tail.b75 = a; tail.b76 = b; }
        { unsigned char a = other.tail.b77; unsigned char b = other.tail.b78; tail.b77 = a; tail.b78 = b; }
        { unsigned char a = other.tail.b79; unsigned char b = other.tail.b7a; tail.b79 = a; tail.b7a = b; }
        { unsigned char a = other.tail.b7b; unsigned char b = other.tail.b7c; tail.b7b = a; tail.b7c = b; }
        { unsigned char a = other.tail.b7d; unsigned char b = other.tail.b7e; tail.b7d = a; tail.b7e = b; }
        { unsigned char a = other.tail.b7f; unsigned char b = other.tail.b80; tail.b7f = a; tail.b80 = b; }
        { unsigned char a = other.tail.b81; unsigned char b = other.tail.b82; tail.b81 = a; tail.b82 = b; }
        { unsigned char a = other.tail.b83; unsigned char b = other.tail.b84; tail.b83 = a; tail.b84 = b; }
        { unsigned char a = other.tail.b85; unsigned char b = other.tail.b86; tail.b85 = a; tail.b86 = b; }
        { unsigned char a = other.tail.b87; unsigned char b = other.tail.b88; tail.b87 = a; tail.b88 = b; }
        { unsigned char a = other.tail.b89; unsigned char b = other.tail.b8a; tail.b89 = a; tail.b8a = b; }
        { unsigned char a = other.tail.b8b; unsigned char b = other.tail.b8c; tail.b8b = a; tail.b8c = b; }
        { unsigned char a = other.tail.b8d; unsigned char b = other.tail.b8e; tail.b8d = a; tail.b8e = b; }
        { unsigned char a = other.tail.b8f; unsigned char b = other.tail.b90; tail.b8f = a; tail.b90 = b; }
        { unsigned char a = other.tail.b91; unsigned char b = other.tail.b92; tail.b91 = a; tail.b92 = b; }
        { unsigned char a = other.tail.b93; unsigned char b = other.tail.b94; tail.b93 = a; tail.b94 = b; }
        { unsigned char a = other.tail.b95; unsigned char b = other.tail.b96; tail.b95 = a; tail.b96 = b; }
        { unsigned char a = other.tail.b97; unsigned char b = other.tail.b98; tail.b97 = a; tail.b98 = b; }
        { unsigned char a = other.tail.b99; unsigned char b = other.tail.b9a; tail.b99 = a; tail.b9a = b; }
        { unsigned char a = other.tail.b9b; unsigned char b = other.tail.b9c; tail.b9b = a; tail.b9c = b; }
        { unsigned char a = other.tail.b9d; unsigned char b = other.tail.b9e; tail.b9d = a; tail.b9e = b; }
        { unsigned char a = other.tail.b9f; unsigned char b = other.tail.ba0; tail.b9f = a; tail.ba0 = b; }
        { unsigned char a = other.tail.ba1; unsigned char b = other.tail.ba2; tail.ba1 = a; tail.ba2 = b; }
        { unsigned char a = other.tail.ba3; unsigned char b = other.tail.ba4; tail.ba3 = a; tail.ba4 = b; }
    }
};

enum TerrainVectorLimits {
    kTerrainMaxRecords = 0x1861861, kTerrainFirstGrowthThreshold = 0x820820,
    kTerrainSecondGrowthThreshold = 0x1041040, kTerrainLengthErrorTextOffset = 0xAC
};

struct EntryArray80138554 {
    Entry80138554* begin;
    unsigned end;
    unsigned capacity;
};

extern void* appMallocDummy__FUl(unsigned);
extern void appFreeUni__FPv(void*);
extern void* __nwa__FUl(unsigned);
extern void __dla__FPv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern void __dt__Q33std6detail16char_array_guardFv(void*, int);
extern void dtor_8013A2F0(void*, int);
extern void __dt__Q23std12length_errorFv(void*, int);
extern char golfParameterXmlStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

struct ErrorChars80138554 {
    char* p;
    unsigned char* allocator;
    // Free the error-message allocation unless its ownership has been transferred.
    ~ErrorChars80138554() { if (p) __dla__FPv(p); }
};

// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h; keep this local alias for callers.
typedef ::std::detail::__message_control ErrorRef80138554;

} // extern "C"

namespace std {
struct exception {
    void* vtable;
};

struct logic_error : exception {
    char* text;
    ErrorRef80138554* ref;
    // Copy a borrowed C message into owned, reference-counted exception storage.
    logic_error(const char* message) {
        vtable = __vt__Q23std11logic_error;
        unsigned char allocator = 0;
        unsigned n = strlen(message) + 1;
        char* p = (char*)__nwa__FUl(n * 2 - n);
        text = p;
        {
            ErrorChars80138554 guard = { p, &allocator };
            ref = new ErrorRef80138554(p);
            guard.p = 0;
        }
        strcpy(text, message);
    }
    ~logic_error();
};

struct length_error : logic_error {
    // Build the logic-error message and install the derived length-error interface.
    length_error(const char* message) : logic_error(message) {
        vtable = __vt__Q23std12length_error;
    }
    ~length_error();
};
}

extern "C" {

struct GrowBuffer80138554 {
    Entry80138554* begin;
    unsigned count;
    unsigned capacity;
    unsigned* allocator;
    unsigned remaining;
    // Begin with no allocation; a identifies the vector's allocator/capacity state.
    GrowBuffer80138554(unsigned* a) : begin(0), count(0), capacity(0), allocator(a), remaining(0) {}
    // Release whichever allocation remains in the guard, including after the commit swap.
    ~GrowBuffer80138554() {
        if (begin) {
            count = 0 - count;
            appFreeUni__FPv(begin);
        }
    }
};

#pragma opt_propagation off
#pragma use_lmw_stmw on
// Append a copied declaration, growing storage when full; caller retains value.
// end/capacity count records (0xA8-byte stride, including alignment padding).
// Limit 0x1861861 = floor(0xFFFFFFFF / 0xA8); growth thresholds are one and two
// thirds of that limit (0x820820 and 0x1041040). Below them growth adds max(needed,
// 3*(cap+1)/5), then max(needed,(cap+1)/2); the final tier saturates to the limit.
// golfParameterXmlStrings+0xAC is the pooled length-error message, not terrain data.
void TerrainDeclarationVector_Append(EntryArray80138554* self, const Entry80138554* value) {
    // Fast path: publish the new count and copy into the existing final slot.
    unsigned end = self->end;
    if (end < self->capacity) {
        self->end = end + 1;
        Entry80138554* out = self->begin + (self->end - 1);
        out->x = value->x;
        *out = *value;
        return;
    }

    // Reserve phase: prove capacity arithmetic fits before allocating any replacement.
    if ((unsigned)kTerrainMaxRecords - self->capacity < 1) {
        throw std::length_error(golfParameterXmlStrings + kTerrainLengthErrorTextOffset);
    }

    unsigned needed = self->end + 1 - self->capacity;
    GrowBuffer80138554 buffer(&self->capacity);
    unsigned cap = self->capacity;
    if (needed > (unsigned)kTerrainMaxRecords - cap) {
        throw std::length_error(golfParameterXmlStrings + kTerrainLengthErrorTextOffset);
    }
    unsigned newCap;
    if (cap < kTerrainFirstGrowthThreshold) {
        unsigned grown = (cap + 1) * 3 / 5;
        newCap = cap + *(grown < needed ? &needed : &grown);
    } else if (cap < kTerrainSecondGrowthThreshold) {
        unsigned grown = (cap + 1) / 2;
        newCap = cap + *(grown < needed ? &needed : &grown);
    } else {
        newCap = kTerrainMaxRecords;
    }
    if (newCap > kTerrainMaxRecords) {
        throw std::length_error(golfParameterXmlStrings + kTerrainLengthErrorTextOffset);
    }

    // Copy phase: place the appended value FIRST (value may alias an existing record),
    // then copy old records backwards. buffer owns the replacement until commit.
    Entry80138554* fresh = (Entry80138554*)appMallocDummy__FUl(newCap * sizeof(Entry80138554));
    unsigned size = self->end;
    Entry80138554* dst = fresh + size;
    Entry80138554* inserted = dst + buffer.count;
    *inserted = *value;
    buffer.begin = fresh;
    buffer.capacity = newCap;
    buffer.remaining = size;
    ++buffer.count;
    Entry80138554* src = self->begin + self->end;
    Entry80138554* start = self->begin;
    while (src > start) {
        --dst;
        --src;
        new (dst) Entry80138554(*src);
        --buffer.remaining;
        ++buffer.count;
    }

    // Commit phase: vector adopts the completed replacement; guard frees old storage.
    Entry80138554* old = self->begin;
    Entry80138554* replacement = buffer.begin;
    unsigned count = buffer.count;
    unsigned oldCap = self->capacity;
    buffer.begin = old;
    self->capacity = buffer.capacity;
    buffer.capacity = oldCap;
    self->begin = replacement;
    self->end = count;
    buffer.count = 0;
}

extern void appFreeUni__FPv(void*);
struct String801396E4 {
    union { unsigned word; unsigned char bytes[4]; } rep;
    unsigned length;
    char* ptr;
    // Free only heap-backed keys; inline bytes have no separate allocation.
    ~String801396E4() { if (rep.word >> 31) appFreeUni__FPv(ptr); }
};

extern void* StringTree_FindInsertionPosition(void*, void*, void*, unsigned char*, unsigned char*);
extern void* NamedObjectMap_InsertNode(void*, void*, unsigned char, unsigned char, void* entry); // entry: key string, then value
extern void String_ReserveCapacity(void*, unsigned);
extern void String_ReplaceRange(void*, unsigned, unsigned, const char*, const char*, void*);
extern unsigned g_defaultNamedObjectValue;

// A tree node holds three link words followed by its string key and mapped value.
struct NamedObjectValueNode {
    void* links[3];
    String801396E4 key;
    unsigned value;
};
typedef char NamedObjectValueNode_value_offset[(int)&((NamedObjectValueNode*)0)->value == 0x18 ? 1 : -1];

// Find the lower-bound key or insert a copy, then return its writable mapped value.
// Returned unsigned* is borrowed from a tree node (not the node address); deletion
// invalidates it. The XML loader writes the declaration index through this pointer.
// insertFlagA/B are opaque hints passed unchanged from position search to insertion;
// this unit does not establish which flag encodes which balancing/link decision.
unsigned* NamedObjectValueTree_GetOrInsert(void* self, String801396E4* requestedKey) {
    void* node;
    unsigned char insertFlagA, insertFlagB;
    void* insertionPosition = StringTree_FindInsertionPosition(self, requestedKey, &node, &insertFlagA, &insertFlagB);
    void* valueNode;
    // Original 0x801396E4: an existing lower-bound key can share the insert tail.
    do {
        if (node != 0) {
            String801396E4* existing = &((NamedObjectValueNode*)node)->key;
            unsigned left;
            const char* leftData;
            unsigned right;
            const char* rightData;
            if (!(requestedKey->rep.word >> 31)) { left = requestedKey->rep.bytes[0] & 127; leftData = (char*)&requestedKey->rep.bytes[1]; }
            else { left = requestedKey->length; leftData = requestedKey->ptr; }
            if (!(existing->rep.word >> 31)) { right = existing->rep.bytes[0] & 127; rightData = (char*)&existing->rep.bytes[1]; }
            else { right = existing->length; rightData = existing->ptr; }
            // Compare existing key against requested key: common prefix, then lengths.
            // Short-string length uses byte0 & 0x7F; a set high bit selects heap storage.
            unsigned count = right < left ? right : left;
            int cmp = memcmp(rightData, leftData, count);
            if (cmp == 0) cmp = (right < left) ? -1 : (right == left ? 0 : 1);
            if (cmp >= 0) { valueNode = node; break; }
        }
        {
            // Key and value passed together: the key string, then the default value.
            struct { String801396E4 key; unsigned value; } entry = {};
            String801396E4& copy = entry.key;
            if (!(requestedKey->rep.word >> 31)) {
                copy.rep.word = requestedKey->rep.word;
                copy.length = requestedKey->length;
                copy.ptr = requestedKey->ptr;
            } else {
                String_ReserveCapacity(&copy, requestedKey->length);
                unsigned char flag = 0;
                String_ReplaceRange(&copy, 0, 0, requestedKey->ptr, requestedKey->ptr + requestedKey->length, &flag);
            }
            entry.value = g_defaultNamedObjectValue;
            valueNode = NamedObjectMap_InsertNode(self, insertionPosition, insertFlagA, insertFlagB, &entry);
            // copy's destructor frees its buffer
        }
    } while (0);
    return &((NamedObjectValueNode*)valueNode)->value;
}

// Modding view of both complete fixed-size fields; preserves all 64 bytes,
// including any terminator and trailing bytes. The copy ABI above deliberately
// remains scalar: making its members arrays changes CodeWarrior's copy schedule.
struct TerrainDeclarationTextView {
    char declarationWordsAndType[0x25];
    unsigned char firstText[64];
    unsigned char secondText[64];
};
typedef char TerrainFirstTextOffset[(int)&((TerrainDeclarationTextView*)0)->firstText == 0x25 ? 1 : -1];
typedef char TerrainSecondTextOffset[(int)&((TerrainDeclarationTextView*)0)->secondText == 0x65 ? 1 : -1];

// Borrow both writable text buffers from a declaration; vector reallocation invalidates the view.
static inline TerrainDeclarationTextView* TerrainDeclarationTexts(Entry80138554* entry) {
    return (TerrainDeclarationTextView*)entry;
}

} // extern "C"
