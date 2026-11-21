typedef unsigned long new_size_t;
#include "MSL/error_message.h"
// Translation unit 0x8013798C-0x80138554 (5 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

static const unsigned int kMaxTerrainDeclarations = 0x1861861U;
static const unsigned int kVectorLengthErrorOffset = 0xac;

void* operator new(new_size_t);
// Construct in caller-owned storage without allocating.
inline void* operator new(new_size_t, void* p) throw() { return p; }

extern "C" {

#include "sdk_protos.h"
void StringValueTree_DestroySubtree(void*, void*);
void appFreeUni__FPv(void*);
void* appMallocDummy__FUl(unsigned int);
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void __dla__FPv(void*);
void __dt__Q23std12length_errorFv(void*, int);

struct CleanupNode {
    // Node: first/second are its two child links; +8 is a parent/flag word.
    // Header view: second supplies the root; Clear restores +8 to &second.
    // Thus this same storage view must not be treated as a heap node at self.
    void* first;
    void* second;
    unsigned int parentFlags;
    unsigned int flags;
    unsigned int stringLength;
    void* owned;
};

// A node stores its small/heap string immediately after the three tree links.
// Retain the integer address test used by the original nullable-string destructor.
static inline void ReleaseNodeString(CleanupNode* node) {
    if ((unsigned int)(node) + 0xC) {
        if ((unsigned int)(node) + 0xC) {
            if (node->flags >> 31) appFreeUni__FPv(node->owned);
        }
    }
}

// Finish both recursive branches before releasing the node string and allocation.
static inline void DestroyTreeNode(CleanupNode* node, void* owner) {
    if (node->first) StringValueTree_DestroySubtree(owner, node->first);
    if (node->second) StringValueTree_DestroySubtree(owner, node->second);
    ReleaseNodeString(node);
    appFreeUni__FPv(node);
}

// Release one expanded child level before its parent; deeper branches use the shared walker.
static inline void DestroyTreeChild(CleanupNode* child, void* owner) {
    CleanupNode* leaf = (CleanupNode*)child->first;
    if (leaf) DestroyTreeNode(leaf, owner);
    leaf = (CleanupNode*)child->second;
    if (leaf) DestroyTreeNode(leaf, owner);
    ReleaseNodeString(child);
    appFreeUni__FPv(child);
}

// Release both expanded child levels without changing the postorder ownership sequence.
static inline void DestroyTreeBranch(CleanupNode* branch, void* owner) {
    CleanupNode* child = (CleanupNode*)branch->first;
    if (child) DestroyTreeChild(child, owner);
    child = (CleanupNode*)branch->second;
    if (child) DestroyTreeChild(child, owner);
    ReleaseNodeString(branch);
    appFreeUni__FPv(branch);
}

// Clear the string-value tree and restore the empty-tree sentinel links.
void StringValueTree_ClearUnrolled(CleanupNode* self) {
    CleanupNode* root;
    CleanupNode* branch;
    root = (CleanupNode*)self->second;
    if (root) {
        branch = (CleanupNode*)root->first;
        if (branch) DestroyTreeBranch(branch, self);
        branch = (CleanupNode*)root->second;
        if (branch) DestroyTreeBranch(branch, self);
        ReleaseNodeString(root);
        appFreeUni__FPv(root);
        self->first = 0;
        self->second = 0;
        self->parentFlags = (unsigned int)&self->second;
    }
}

// Forward a C-string substring search, preserving the library return pointer.
char* CString_FindSubstringForwarder(const char* s, const char* needle) {
    return strstr(s, needle);
}

struct StringLike {
    union {
        struct { unsigned char length; char data[15]; } small;
        struct { unsigned int capacity; unsigned int length; char* data; } large;
    } value;
};

struct Holder {
    unsigned int storageAddress;
    union { unsigned int value; signed int signedValue; };
};

// Clear the count word while preserving the adjacent storage address.
void VectorStorage_ClearCount(Holder* self) {
    self->value -= self->signedValue;
}

// One terrain declaration (0xA8 stride): +00/+04 two floats, +08..+23 seven
// opaque words, +24..+A4 an opaque 129-byte payload, +A5..+A7 padding.
// The gameplay roles/units are unresolved by this storage-copy specialization;
// words/data are inline record content; any pointer ownership is unresolved.
// This copy performs no allocation, retain or release for those payload words.
// Copies include all payload bytes but leave the three padding bytes untouched.
struct Entry8013798C {
    float x, y;
    unsigned words[7];
    unsigned char data[0x81];
    unsigned char pad[3];
    // Preserve the original no-op default construction (content is uninitialized).
    Entry8013798C() {}
    // Copy one declaration's editable content, preserving paired load/store order.
    Entry8013798C(const Entry8013798C& other) {
        unsigned secondWord, firstWord;
        unsigned char firstByte, secondByte;
        x = other.x;
        y = other.y;
        firstWord = other.words[0]; secondWord = other.words[1];
        words[0] = firstWord; words[1] = secondWord;
        words[2] = other.words[2];
        firstWord = other.words[3]; secondWord = other.words[4];
        words[3] = firstWord; words[4] = secondWord;
        words[5] = other.words[5];
        words[6] = other.words[6];
        data[0] = other.data[0];
        // Copy the remaining 128 payload bytes in load/load/store/store pairs.
        // An inline helper changes this compiler's output; retain local expansion.
#define COPY_PAYLOAD_PAIR(n) { firstByte = other.data[n]; secondByte = other.data[(n) + 1]; data[n] = firstByte; data[(n) + 1] = secondByte; }
        COPY_PAYLOAD_PAIR(1); COPY_PAYLOAD_PAIR(3); COPY_PAYLOAD_PAIR(5); COPY_PAYLOAD_PAIR(7);
        COPY_PAYLOAD_PAIR(9); COPY_PAYLOAD_PAIR(11); COPY_PAYLOAD_PAIR(13); COPY_PAYLOAD_PAIR(15);
        COPY_PAYLOAD_PAIR(17); COPY_PAYLOAD_PAIR(19); COPY_PAYLOAD_PAIR(21); COPY_PAYLOAD_PAIR(23);
        COPY_PAYLOAD_PAIR(25); COPY_PAYLOAD_PAIR(27); COPY_PAYLOAD_PAIR(29); COPY_PAYLOAD_PAIR(31);
        COPY_PAYLOAD_PAIR(33); COPY_PAYLOAD_PAIR(35); COPY_PAYLOAD_PAIR(37); COPY_PAYLOAD_PAIR(39);
        COPY_PAYLOAD_PAIR(41); COPY_PAYLOAD_PAIR(43); COPY_PAYLOAD_PAIR(45); COPY_PAYLOAD_PAIR(47);
        COPY_PAYLOAD_PAIR(49); COPY_PAYLOAD_PAIR(51); COPY_PAYLOAD_PAIR(53); COPY_PAYLOAD_PAIR(55);
        COPY_PAYLOAD_PAIR(57); COPY_PAYLOAD_PAIR(59); COPY_PAYLOAD_PAIR(61); COPY_PAYLOAD_PAIR(63);
        COPY_PAYLOAD_PAIR(65); COPY_PAYLOAD_PAIR(67); COPY_PAYLOAD_PAIR(69); COPY_PAYLOAD_PAIR(71);
        COPY_PAYLOAD_PAIR(73); COPY_PAYLOAD_PAIR(75); COPY_PAYLOAD_PAIR(77); COPY_PAYLOAD_PAIR(79);
        COPY_PAYLOAD_PAIR(81); COPY_PAYLOAD_PAIR(83); COPY_PAYLOAD_PAIR(85); COPY_PAYLOAD_PAIR(87);
        COPY_PAYLOAD_PAIR(89); COPY_PAYLOAD_PAIR(91); COPY_PAYLOAD_PAIR(93); COPY_PAYLOAD_PAIR(95);
        COPY_PAYLOAD_PAIR(97); COPY_PAYLOAD_PAIR(99); COPY_PAYLOAD_PAIR(101); COPY_PAYLOAD_PAIR(103);
        COPY_PAYLOAD_PAIR(105); COPY_PAYLOAD_PAIR(107); COPY_PAYLOAD_PAIR(109); COPY_PAYLOAD_PAIR(111);
        COPY_PAYLOAD_PAIR(113); COPY_PAYLOAD_PAIR(115); COPY_PAYLOAD_PAIR(117); COPY_PAYLOAD_PAIR(119);
        COPY_PAYLOAD_PAIR(121);
        {
            unsigned char secondByte, firstByte;
            COPY_PAYLOAD_PAIR(123); COPY_PAYLOAD_PAIR(125); COPY_PAYLOAD_PAIR(127);
        }
#undef COPY_PAYLOAD_PAIR
    }
};

struct EntryVector8013798C {
    Entry8013798C* begin;
    unsigned size;
    unsigned capacity;
};

// Allocator tag only; its volatile byte preserves allocator lifetime bookkeeping.
union Allocator8013798C {
    volatile char value;
    unsigned int alignment;
};

// Allocation owner used during reserve; after commit it owns the retired block.
struct GrowBuffer8013798C {
    Entry8013798C* begin;
    union { unsigned int count; int signedCount; };
    unsigned int capacity;
    // Start an empty temporary allocation owner for reserve.
    GrowBuffer8013798C() : begin(0), count(0), capacity(0) {}
    // Release the staged allocation on failure or the retired allocation on commit.
    ~GrowBuffer8013798C() {
        if (begin) {
            Allocator8013798C flag;
            flag.value = 0;
            count -= signedCount;
            appFreeUni__FPv(begin);
        }
    }
};

extern char golfParameterXmlStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern "C" void __dt__Q23std9exceptionFv(void*, int);

struct ErrorChars8013798C {
    char* p;
    unsigned char* allocator;
    // Delete diagnostic text unless the reference-counted owner has adopted it.
    ~ErrorChars8013798C() { if (p) __dla__FPv(p); }
};

// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h; keep this local alias for callers.
typedef ::std::detail::__message_control ErrorRef8013798C;

} // extern "C"

namespace std {
struct exception {
    void* vtable;
};

struct logic_error : exception {
    char* text;
    ErrorRef8013798C* ref;
    // Duplicate the diagnostic into shared storage using the MSL exception layout.
    logic_error(const char* message) {
        vtable = __vt__Q23std11logic_error;
        Allocator8013798C allocator;
        allocator.value = 0;
        Allocator8013798C allocator2;
        allocator2.value = 0;
        unsigned n = strlen(message) + 1;
        char* p = (char*)__nwa__FUl(n * 2 - n);
        text = p;
        {
            ErrorChars8013798C guard = { p, (unsigned char*)&allocator2 };
            ref = new ErrorRef8013798C(p);
            guard.p = 0;
        }
        strcpy(text, message);
    }
    ~logic_error();
};

struct length_error : logic_error {
    // Construct the base message owner, then install the length-error vtable.
    length_error(const char* message) : logic_error(message) {
        vtable = __vt__Q23std12length_error;
    }
    ~length_error();
};
}

extern "C" {

// Reserve terrain-declaration storage, copy live records, then free the old block.
// requested is an element capacity, never bytes; size remains unchanged.
// self owns the array; no external record payload is retained or freed here.
void TerrainDeclarationVector_Reserve(EntryVector8013798C* self, unsigned int requested) { Allocator8013798C loopTag;  Allocator8013798C swapTag;
    if (requested <= self->capacity) return;

    // Phase 1: validate capacity and allocate a guarded replacement.
    GrowBuffer8013798C buffer;

    Allocator8013798C firstTag; firstTag.value = 0;
    if (requested > kMaxTerrainDeclarations) {
        const char* message = golfParameterXmlStrings + kVectorLengthErrorOffset;
        throw std::length_error(message);
    }

    Entry8013798C* fresh = (Entry8013798C*)appMallocDummy__FUl(requested * sizeof(Entry8013798C));
    Entry8013798C* source = self->begin;
    Entry8013798C* end = (((self->size + source)));
    buffer.begin = fresh;
    buffer.capacity = requested;

    // Phase 2: copy live records; self still owns the old storage until commit.
    loopTag.value = 0;
    Entry8013798C* destination = buffer.count + fresh;
    // 0x80137F20: test the live-range end before copying each live entry.
    while (end > source) {
        if (destination) {
            try { new (destination) Entry8013798C(*source); }
            catch (...) { throw; }
        }
        ++source;
        destination = destination + 1;
        ++buffer.count;
    }

    // Phase 3: exchange allocation metadata; the guard releases the retired array.
    // self   = {fresh, old size, requested};
    // buffer = {old begin, old size, old capacity}, freed by its destructor.
    Entry8013798C* retiredData = self->begin;
    unsigned replacementCapacity;
    Entry8013798C* replacementData = buffer.begin;
    unsigned retiredCapacity = self->capacity;
    unsigned retiredSize = self->size;
    replacementCapacity = buffer.capacity;
    swapTag.value = 0;
    buffer.begin = retiredData;
    self->capacity = replacementCapacity;
    buffer.capacity = retiredCapacity;
    self->begin = replacementData;
    unsigned replacementSize = buffer.count;
    self->size = replacementSize;
    buffer.count = retiredSize;
}

// Compare small or heap string content and length against a NUL-terminated string.
int String_EqualsCString(const StringLike* self, const char* other) {
    unsigned int otherLength = strlen(other);
    unsigned int size = (self->value.large.capacity >> 31) ? self->value.large.length : (self->value.small.length & 0x7f);
    const char* data;
    unsigned int size2;
    if (self->value.large.capacity >> 31) {
        data = self->value.large.data;
        size2 = self->value.large.length;
    } else {
        data = self->value.small.data;
        size2 = self->value.small.length & 0x7f;
    }
    unsigned int compareLength = *(size < size2 ? &size : &size2);
    int result = memcmp(data, other, *(otherLength < compareLength ? &otherLength : &compareLength));
    if (result == 0) {
        if (compareLength < otherLength) result = -1;
        else if (compareLength == otherLength) result = 0;
        else result = 1;
    }
    return result == 0;
}

} // extern "C"
