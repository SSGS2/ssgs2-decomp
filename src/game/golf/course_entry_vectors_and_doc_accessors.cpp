// Course entries and point vectors: the iterator over course entries, the growable point
// vector that course entries use, and borrowed views of the golf and shared documents.
// CGolfBg::OnPreLoadInit creates the golf cup and shadow manager before a course loads.

#include "MSL/error_message.h"
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

#include "sdk_protos.h"
#include "game/wvector.h"
#include "game/exception.h"
#include "classes/CGolfBg.h"
#include "classes/CGolfCup.h"
#include "classes/CShadowManager.h"

// Construct in caller-owned storage without allocating.
inline void* operator new(size_t, void* p) {
    return p;
}

extern "C" {

// Return the stored player count as an unsigned byte.
unsigned char CGolfDoc::GetPlayerCount() {
    return playerCount; // 0x801228E8: byte at +0x2B9
}

void* g_sceneManager;
CSharedDoc* g_sharedDoc;

// Borrow the current scene-manager pointer; no reference or ownership transfer.
void* SceneManager_GetCurrent() {
    return g_sceneManager;
}

// Borrow the first course entry; valid only until the vector changes storage.
void* CourseEntryVector_Begin(void* self) {
    return *(void**)self;
}

// Dereference the iterator's borrowed entry pointer.
void* CourseEntryIterator_GetEntry(void* self) {
    return *(void**)self;
}

// Return this round's stored hole count.
unsigned char SharedDoc_GetRoundHoleCount() {
    return AsCSharedDoc(g_sharedDoc)->roundHoleCount;
}

// Course iteration advances over fixed-size entries; point vectors contain three raw float words.
struct CoursePointWords {
    unsigned int xBits, yBits, zBits;
};
struct CoursePointVector {
    CoursePointWords* data;
    unsigned int size, capacity;
};
struct CourseEntryView {
    char metadata[0x14];
    CoursePointVector points;
};
struct CourseEntryVectorView {
    CourseEntryView* data;
    unsigned int size, capacity;
};
typedef char CourseEntryView_size[(sizeof(CourseEntryView) == 0x20) ? 1 : -1];

// Returns dst (r3 untouched, 0x80123C1C); callers pass it on (0x801225F8, 0x801EADEC).
void* CourseEntryIterator_CopyConstruct(void* dst, void* src) {
    *(unsigned int*)dst = *(unsigned int*)src;
    return dst;
}

// Return count-1 narrowed to a byte; zero holes therefore yields 255.
unsigned char SharedDoc_GetLastRoundHoleIndex() {
    return (unsigned char)(AsCSharedDoc(g_sharedDoc)->roundHoleCount - 1);
}

// Copy the borrowed entry pointer between initialized iterators.
void CourseEntryIterator_Assign(void* dst, void* src) {
    *(unsigned int*)dst = *(unsigned int*)src;
}

// Advance by one 32-byte entry and return the previously referenced entry.
void* CourseEntryIterator_PostIncrement(void* self) {
    void* value = *(void**)self;
    *(void**)self = (CourseEntryView*)value + 1;
    return value;
}

// Return whether two iterators refer to different entries (0 or 1).
unsigned int CourseEntryIterator_NotEqual(void* a, void* b) {
    return *(unsigned int*)a != *(unsigned int*)b;
}

// Return the borrowed one-past-last entry pointer, which must not be dereferenced.
void* CourseEntryVector_End(void* self) {
    return ((CourseEntryVectorView*)self)->data + ((CourseEntryVectorView*)self)->size;
}

extern void* __nw__FUl(unsigned int);
extern void* g_resourceManager;
struct GolfCupConstruction {
    char data[0xBC];
    // Initialize the allocated cup through its original constructor.
    GolfCupConstruction() {
        ((CGolfCup*)this)->__ct();
    }
};
struct ShadowManagerConstruction {
    char data[0x30];
    // Initialize shadow-manager storage with its resource provider and mode.
    ShadowManagerConstruction(void* resourceManager, int b) {
        ((CShadowManager*)this)->__ct(resourceManager, b);
    }
};

// Construct the golf cup and its resource-manager-backed shadow manager before course loading.
void CGolfBg::OnPreLoadInit() {
    new GolfCupConstruction;
    new ShadowManagerConstruction(g_resourceManager, 1);
}

// An empty iterator constructor intentionally leaves its pointer uninitialized, matching the container API.
void CourseEntryIterator_DefaultConstruct() {}

// Return the numeric point count in r3 using the recovered void* ABI declaration.
// This is NOT an address or borrowed object: callers must interpret r3 as an
// unsigned 32-bit count (on this target), never dereference or free it. Changing
// the shared declaration to an integer return requires auditing callers first.
void* Vec3Vector_GetPointCount(void* self) {
    return (void*)((CoursePointVector*)self)->size;
}

struct PointVectorStorage {
    // Own data[0..size); capacity is an element count, not bytes (12 bytes/point).
    void* data;
    unsigned int size;
    unsigned int capacity;
};

enum PointVectorLimits {
    kMaxPointCount = 0x15555555U,
    kPointGrowthThreeFifthsLimit = 0x071C71C7U,
    kPointGrowthOneHalfLimit = 0x0E38E38EU
};
struct GolfBackgroundStringPool {
    char resourceStrings[0x67];
    char vectorLengthError[20];
    char remainingStrings[0x1D0 - 0x7B];
};
typedef char GolfBackgroundStringPool_size[(sizeof(GolfBackgroundStringPool) == 0x1D0) ? 1 : -1];
extern GolfBackgroundStringPool golfBackgroundStrings;
static const int kVectorLengthErrorOffset = (int)&((GolfBackgroundStringPool*)0)->vectorLengthError;
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern unsigned int __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void __dla__FPv(void*);
extern void __dt__Q23std12length_errorFv(void*);

inline void LengthError8012369C();

// Grow point storage without changing its contents; reject counts that overflow the allocation size.
void Vec3Vector_ReservePointCapacity(PointVectorStorage* self, unsigned int requested) {
    if (requested <= self->capacity)
        return;
    if (requested > kMaxPointCount)
        LengthError8012369C();
    CoursePointWords* replacement = (CoursePointWords*)appMallocDummy__FUl(requested * 12);
    unsigned int copied = 0;
    CoursePointWords* oldData = (CoursePointWords*)self->data;
    CoursePointWords* oldEnd = oldData + self->size;
    CoursePointWords* out = replacement;
    while (oldData < oldEnd) {
        if (out)
            *out = *oldData;
        ++oldData;
        ++out;
        ++copied;
    }
    // Commit only after the replacement contains every live point. The saved
    // old allocation is released after all three descriptor fields are installed.
    PointVectorStorage temporary;
    temporary.data = (CoursePointWords*)self->data;
    temporary.size = 0;
    temporary.capacity = self->capacity;
    self->data = replacement;
    self->size = copied;
    self->capacity = requested;
    if (temporary.data)
        appFreeUni__FPv(temporary.data);
}

// Borrow the embedded point-vector descriptor; its owner remains the course entry.
void* CourseEntry_GetPointVector(void* self) {
    return &((CourseEntryView*)self)->points;
}

struct PointVector {
    CoursePointWords* data;
    unsigned int size, capacity;
};
extern void __as__7WVectorFRC7WVector(void*, void*);
extern float kHoleSetupZero;

struct BufferGuard801228E8 {
    char* data;
    char* allocator;
    // Free exception text if construction of its shared owner throws.
    ~BufferGuard801228E8() {
        if (data)
            __dla__FPv(data);
    }
};
// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h; keep this local alias for callers.
typedef ::std::detail::__message_control SharedBuffer801228E8;
extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct Allocator801228E8 {
    // Addressable allocator ABI tag; separate instances preserve exception lifetimes.
    volatile char flag;
    // Initialize the allocator tag used by the reconstructed STL paths.
    Allocator801228E8() : flag(0) {}
};
namespace std {
struct logic_error : exception {
    // Install the requested exception vtable during staged construction.
    void SetTable(void* table) {
        *(void**)this = table;
    }
    char* text;
    SharedBuffer801228E8* shared;
    // Copy the message and transfer its allocation to a shared control block.
    logic_error(const char* message) {
        char allocator = 0;
        SetTable(&__vt__Q23std11logic_error);
        unsigned int length = strlen(message) + 1;
        char* allocated = (char*)__nwa__FUl(length * 2 - length);
        text = allocated;
        {
            BufferGuard801228E8 buffer = {allocated, &allocator};
            shared = new SharedBuffer801228E8(allocated);
            buffer.data = 0;
        }
        strcpy(text, message);
    }
    // Build the same owned message through the allocator-taking ABI variant.
    logic_error(const char* message, Allocator801228E8& outerAllocator) {
        SetTable(&__vt__Q23std11logic_error);
        Allocator801228E8 allocator;
        unsigned int length = strlen(message) + 1;
        char* allocated = (char*)__nwa__FUl(length * 2 - length);
        text = allocated;
        {
            BufferGuard801228E8 buffer = {allocated, (char*)&allocator};
            shared = new SharedBuffer801228E8(allocated);
            buffer.data = 0;
        }
        strcpy(text, message);
    }
    ~logic_error();
};
struct length_error : logic_error {
    // Construct the shared-message base then specialize the exception vtable.
    length_error(const char* message) : logic_error(message) {
        SetTable(&__vt__Q23std12length_error);
    }
    // Construct length_error through its allocator-taking variant.
    length_error(const char* message, Allocator801228E8& allocator) : logic_error(message, allocator) {
        SetTable(&__vt__Q23std12length_error);
    }
    ~length_error();
};
}
#define LENGTH_ERROR_801228E8() throw std::length_error((const char*)&golfBackgroundStrings + kVectorLengthErrorOffset)

struct PointReserveTemp {
    CoursePointWords* data;
    unsigned int size, capacity;
    // Release replacement storage on unwind, or old storage after a successful swap.
    ~PointReserveTemp() {
        if (data) {
            Allocator801228E8 allocator;
            size -= size;
            appFreeUni__FPv((PointVectorStorage*)data);
        }
    }
};
struct PointAppendTemp {
    // During append, the constructed range begins at offset and contains size
    // points; copying backwards grows it toward zero. After commit, data owns
    // the old buffer and size is zero. Point words need no element destructor.
    CoursePointWords* data;
    unsigned int size, capacity;
    unsigned int* sourceCapacity; // Borrowed &self->capacity; not an allocator.
    unsigned int offset;
    // Free the allocation still held by this temporary owner.
    ~PointAppendTemp() {
        if (data) {
            Allocator801228E8 allocator;
            size -= size;
            appFreeUni__FPv((PointVectorStorage*)data);
        }
    }
};

// Copy-construct one point in caller-owned storage. Keep try/catch in the
// caller: a typed inline helper changed relocations in isolated verification.
// This macro only constructs/rethrows; it does not return or change vector size.
#define CONSTRUCT_8012369C(dst, src)     \
    try {                                \
        new (dst) CoursePointWords(src); \
    } catch (...) {                      \
        throw;                           \
    }

struct CopyTemp8012369C {
    unsigned int a, b, c, d, e;
    volatile void* frame;
};

struct InsertTmp8012369C {
    CoursePointWords* data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* sourceCapacity; // Same descriptor layout as PointAppendTemp.
    unsigned int offset;
};

// Throw the shared vector-length error while retaining allocator construction and exception cleanup order.
inline void LengthError8012369C() {
    Allocator801228E8 errorAllocator;
    const char* message = (const char*)&golfBackgroundStrings + kVectorLengthErrorOffset;
    throw std::length_error(message, errorAllocator);
}

// Capacity policy for ALL three growth sites in this unit:
// required increment = requested size - current capacity; reject overflow;
// low range: capacity + max(additional, ((capacity+1)*3)/5);
// middle range: capacity + max(additional, (capacity+1)/2);
// high range: kMaxPointCount. Integer division truncates.
// CopyAppendAndFlattenHeight expands the same selection to preserve its
// allocator/exception lifetimes; keep those copies in sync with this one.
// Choose three-fifths or one-half growth while keeping the point allocation below the 32-bit size limit.
inline unsigned int ComputePointVectorGrowth(unsigned int* capacityPointer, unsigned int additional) {
    unsigned int capacity = *capacityPointer;
    if (additional > kMaxPointCount - capacity)
        LengthError8012369C();
    unsigned int newCapacity;
    if (capacity < kPointGrowthThreeFifthsLimit) {
        unsigned int growth = ((capacity + 1) * 3) / 5;
        const unsigned int* amount = &growth;
        if (growth < additional)
            amount = &additional;
        newCapacity = capacity + *amount;
    } else if (capacity < kPointGrowthOneHalfLimit) {
        unsigned int growth = (capacity + 1) / 2;
        const unsigned int* amount = &growth;
        if (growth < additional)
            amount = &additional;
        newCapacity = capacity + *amount;
    } else {
        newCapacity = kMaxPointCount;
    }
    return newCapacity;
}

// Append one point, growing storage with exception-safe ownership transfer if necessary.
// value may refer into self: growth constructs the new tail BEFORE moving the
// old prefix or releasing storage. Keep allocator-tag scopes and guard lifetimes
// in order; their apparent redundancy records exception cleanup boundaries.
void Vec3Vector_AppendCopy(PointVector* self, const CoursePointWords* value) {
    unsigned int newCapacity;
    unsigned int additional;
    CoursePointWords* out;
    CoursePointWords* src;
    CoursePointWords* begin;
    Allocator801228E8 allocator0;
    // Phase 1: spare capacity commits size only after the new point is constructed.
    if (self->size < self->capacity) {
        out = self->data + self->size;
        CONSTRUCT_8012369C(out, *value);
        ++self->size;
        return;
    } else {
        Allocator801228E8 allocator1;
        Allocator801228E8 allocator2;
        if (kMaxPointCount - self->capacity < 1) {
            Allocator801228E8 errorAllocator;
            const char* message = (const char*)&golfBackgroundStrings + kVectorLengthErrorOffset;
            throw std::length_error(message, errorAllocator);
        }
        additional = self->size + 1 - self->capacity;
        PointAppendTemp temporary = {0, 0, 0, &self->capacity, 0};
        newCapacity = ComputePointVectorGrowth(&self->capacity, additional);
        Allocator801228E8 allocator3;
        if (newCapacity > kMaxPointCount) {
            Allocator801228E8 errorAllocator;
            const char* message = (const char*)&golfBackgroundStrings + kVectorLengthErrorOffset;
            throw std::length_error(message, errorAllocator);
        }
        // Phase 2: temporary owns replacement storage; build the appended point first.
        temporary.data = (CoursePointWords*)appMallocDummy__FUl(newCapacity * 12);
        temporary.capacity = newCapacity;
        temporary.offset = self->size;
        Allocator801228E8 allocator4;
        out = temporary.data + self->size + temporary.size;
        unsigned int remaining = 1;
        while (remaining != 0) {
            CONSTRUCT_8012369C(out, *value);
            --remaining;
            ++out;
            ++temporary.size;
        }
        // Phase 3: prepend existing points backwards until temporary.offset reaches zero.
        begin = self->data;
        src = begin + self->size;
        Allocator801228E8 allocator5;
        out = temporary.data + temporary.offset;
        while (src > begin) {
            --out;
            --src;
            CONSTRUCT_8012369C(out, *src);
            --temporary.offset;
            ++temporary.size;
        }
        // Phase 4: descriptor swap is the ownership commit; temporary frees oldData on scope exit.
        CoursePointWords* oldData = self->data;
        CoursePointWords* newData = temporary.data;
        unsigned int newSize = temporary.size;
        self->size = 0;
        unsigned int oldCapacity = self->capacity;
        unsigned int newCapacity2 = temporary.capacity;
        temporary.data = oldData;
        Allocator801228E8 allocator6;
        self->capacity = newCapacity2;
        temporary.capacity = oldCapacity;
        self->data = newData;
        self->size = newSize;
        temporary.size = 0;
    }
}

// Copy the source points, append one point, then flatten every point to zero height (Y).
// Safe editing contract: sourceVector must describe a valid vector; exact
// self-assignment is handled by the assignment loops, but separately owned
// vectors must not share/overlap their allocations. value must stay valid
// through reserve AND assignment; use an external value rather than an element
// of self that reserve may invalidate or assignment may overwrite. The operation
// has multiple commits, not one transaction: reserve/assignment can complete
// before append fails; flattening occurs only after append completes.
void Vec3Vector_CopyAppendAndFlattenHeight(PointVector* self, PointVector* sourceVector, CoursePointWords* value) {
    // Result policy: self = sourceVector followed by *value, then every Y
    // becomes zero. Source Y and appended Y are both intentionally discarded.
    // Reserve/assign/append below implement that policy with separate commits.
    // Phase 1: reserve enough room for the source plus appended value, retaining
    // the destination's current contents until this first storage swap commits.
    unsigned int n = sourceVector->size;
    if (n + 1 > self->capacity) {
        PointReserveTemp temporary = {0, 0, 0};
        Allocator801228E8 allocator;
        if (n + 1 > kMaxPointCount) {
            const char* message = (const char*)&golfBackgroundStrings + kVectorLengthErrorOffset;
            Allocator801228E8 errorAllocator;
            throw std::length_error(message, errorAllocator);
        }
        temporary.data = (CoursePointWords*)appMallocDummy__FUl((n + 1) * 12);
        CoursePointWords* first = self->data;
        CoursePointWords* last = first + self->size;
        temporary.capacity = n + 1;
        Allocator801228E8 allocator1;
        CoursePointWords* out = temporary.data + temporary.size;
        while (first < last) {
            CONSTRUCT_8012369C(out, *first);
            ++first;
            ++out;
            ++temporary.size;
        }
        CoursePointWords* oldData = self->data;
        self->size = 0;
        unsigned int oldCapacity = self->capacity;
        unsigned int oldSize = self->size;
        Allocator801228E8 allocator2;
        self->capacity = temporary.capacity;
        temporary.capacity = oldCapacity;
        self->data = temporary.data;
        temporary.data = oldData;
        self->size = temporary.size;
        temporary.size = oldSize;
    }

    // Phase 2: assign the source range, reusing common live elements and constructing
    // any tail; the replacement branch releases destination storage before allocating.
    CoursePointWords* first = sourceVector->data;
    CoursePointWords* last = first + sourceVector->size;
    unsigned int count = last - first;
    Allocator801228E8 allocator3;
    unsigned int assignedCapacity;
    // Original unit 0x801228E8: reuse the range or replace its storage.
    do {
        if (count > self->capacity) {
            Allocator801228E8 allocator4;
            unsigned int additional = count - self->capacity;
            unsigned int capacity = self->capacity;
            if (additional > kMaxPointCount - capacity)
                LENGTH_ERROR_801228E8();
            unsigned int newCapacity;
            if (capacity < kPointGrowthThreeFifthsLimit) {
                unsigned int growth = ((capacity + 1) * 3) / 5;
                const unsigned int* amount = &growth;
                if (growth < additional)
                    amount = &additional;
                newCapacity = capacity + *amount;
            } else if (capacity < kPointGrowthOneHalfLimit) {
                unsigned int growth = (capacity + 1) / 2;
                const unsigned int* amount = &growth;
                if (growth < additional)
                    amount = &additional;
                newCapacity = capacity + *amount;
            } else {
                newCapacity = kMaxPointCount;
            }
            assignedCapacity = newCapacity;
        } else {
            const unsigned int* common;
            if (self->size < count)
                common = &self->size;
            else
                common = &count;
            CoursePointWords* middle = first + *common;
            CoursePointWords* input = first;
            CoursePointWords* output = self->data;
            while (input < middle) {
                *(WVector*)output = *(WVector*)input;
                ++input;
                ++output;
            }
            if (count < self->size) {
                Allocator801228E8 allocator5;
                self->size -= self->size - count;
            } else if (self->size < count) {
                Allocator801228E8 allocator6;
                CoursePointWords* out = self->data + self->size;
                first = middle;
                while (first != last) {
                    CONSTRUCT_8012369C(out, *first);
                    ++out;
                    ++first;
                    ++self->size;
                }
            }
            break;
        }
        {
            Allocator801228E8 allocator7;
            self->size -= self->size;
            if (self->data) {
                appFreeUni__FPv((PointVectorStorage*)self->data);
                self->data = 0;
                self->capacity = 0;
            }
            Allocator801228E8 allocator8;
            if (assignedCapacity > kMaxPointCount)
                LENGTH_ERROR_801228E8();
            self->data = (CoursePointWords*)appMallocDummy__FUl(assignedCapacity * 12);
            self->capacity = assignedCapacity;
            CoursePointWords* out = self->data + self->size;
            while (first != last) {
                CONSTRUCT_8012369C(out, *first);
                ++out;
                ++first;
                ++self->size;
            }
        }
    } while (0);

    // Phase 3: append value, building its new tail before the backwards copy if growth is needed.
    Allocator801228E8 allocator9;
    if (self->size < self->capacity) {
        CoursePointWords* out = self->data + self->size;
        CONSTRUCT_8012369C(out, *value);
        ++self->size;
    } else {
        Allocator801228E8 allocator10;
        Allocator801228E8 allocator11;
        if (kMaxPointCount - self->capacity < 1)
            LENGTH_ERROR_801228E8();
        unsigned int additional = self->size + 1 - self->capacity;
        PointAppendTemp temporary = {0, 0, 0, &self->capacity, 0};
        unsigned int capacity = self->capacity;
        if (additional > kMaxPointCount - capacity)
            LENGTH_ERROR_801228E8();
        unsigned int newCapacity;
        if (capacity < kPointGrowthThreeFifthsLimit) {
            unsigned int growth = ((capacity + 1) * 3) / 5;
            const unsigned int* amount = &growth;
            if (growth < additional)
                amount = &additional;
            newCapacity = capacity + *amount;
        } else if (capacity < kPointGrowthOneHalfLimit) {
            unsigned int growth = (capacity + 1) / 2;
            const unsigned int* amount = &growth;
            if (growth < additional)
                amount = &additional;
            newCapacity = capacity + *amount;
        } else {
            newCapacity = kMaxPointCount;
        }
        Allocator801228E8 allocator12;
        if (newCapacity > kMaxPointCount)
            LENGTH_ERROR_801228E8();
        temporary.data = (CoursePointWords*)appMallocDummy__FUl(newCapacity * 12);
        temporary.capacity = newCapacity;
        temporary.offset = self->size;
        Allocator801228E8 allocator13;
        CoursePointWords* out = temporary.data + self->size + temporary.size;
        unsigned int remaining = 1;
        do {
            CONSTRUCT_8012369C(out, *value);
            --remaining;
            ++out;
            ++temporary.size;
        } while (remaining != 0);
        CoursePointWords* begin = self->data;
        CoursePointWords* end = begin + self->size;
        Allocator801228E8 allocator14;
        out = temporary.data + temporary.offset;
        while (end > begin) {
            --out;
            --end;
            CONSTRUCT_8012369C(out, *end);
            --temporary.offset;
            ++temporary.size;
        }
        CoursePointWords* oldData = self->data;
        self->size = 0;
        unsigned int oldCapacity = self->capacity;
        Allocator801228E8 allocator15;
        self->capacity = temporary.capacity;
        temporary.capacity = oldCapacity;
        self->data = temporary.data;
        temporary.data = oldData;
        self->size = temporary.size;
        temporary.size = 0;
    }
    // Phase 4: commit the height policy to ALL points, including the appended point.
    for (unsigned int i = 0; i < self->size; ++i)
        *(float*)&self->data[i].yBits = kHoleSetupZero;
}

} // extern "C"
