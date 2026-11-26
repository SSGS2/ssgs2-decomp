// The shared file registry (CSharedFileRegistry): one table of open shared files, handed out by path or
// from a buffer, and released when the caller is done with them.
//
// Also the vector storage helpers used by its target and destructor-entry lists.

#include <decomp/static_init.h>
#include <decomp/ct_member.h>
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char vectorLengthErrorStrings_LocalVector[];
}
}

#include "sdk_protos.h"
#include "classes/CSharedFileRegistry.h"
#include "classes/CSharedFile.h"
#include "classes/CSharedFileAsync.h"
#include "classes/CSharedFileSync.h"
// Borrow a typed view of the registry; this cast neither checks nor changes ownership.
// Keep this as a macro: adding an inline declaration renumbers local relocation targets.
#define AsCSharedFileRegistry(p) ((CSharedFileRegistry*)(p))

// Registry owns two independent contiguous allocations: +4 holds file pointers
// (count +8, capacity +0xC), +0x10 holds 20-byte store objects (count +0x14,
// capacity +0x18). Counts/capacities are entries, not bytes. Pointer erasure
// releases/deletes the file; store destruction calls each object's virtual dtor.
// Capacity fields complement the shared header's two array views.
struct RegistryCapacityView {
    char prefix[0xC];
    unsigned int pointerCapacity;
    char records[8];
    unsigned int recordCapacity;
};

// Lock domain 6 protects both arrays; each starts with capacity for 1024 entries.
enum SharedFileLimits {
    kRegistryLock = 6,
    kInitialRegistryCapacity = 1024,
    kFilePointerBytes = 4,
    kFileStoreBytes = 20,
    kMaxFilePointerCount = 0x3FFFFFFF,
    kThreeFifthsGrowthLimit = 0x15555555,
    kHalfGrowthLimit = 0x2AAAAAAA
};

extern "C" {

extern float kFloat_6_2831855;
extern float s_twoPi_8028BC74;
extern char s_sharedFileRegistryInstance[];
extern char s_sharedFileRegistryDtorRegistration[];
extern char __vt__19CSharedFileRegistry[];
extern unsigned char s_sharedFileRegistryInitialized;

extern void appFreeUni__FPv(void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void __dl__FPv(void*);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
extern void __ct__16CSharedFileStoreFRC16CSharedFileStore(void*, void*);
extern void __dt__19CSharedFileRegistryFv(void*, int);
extern void dtor_80046918(void*, int);
extern void SharedBufferVector_dtor(void*, int);
extern void dtor_8028BBF4(void*, int);

// POD reservation bookkeeping; keep its declaration to stabilize generated local symbol names.
struct ReservationScratch {
    void* resource;
    void* data;
    int constructedCount;
    short allocatorFlag;
    float reservedFloat;
    short reservedShort;
};
extern void TableLock_ctor(void*, int, int);
extern void __dt__9TableLockFv(void*, int);

struct TableLock {
    char bytes[8];
};

extern char vectorLengthErrorStrings_LocalVector[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern void __dla__FPv(void*);
extern void* __nwa__FUl(unsigned);

// Exception-string construction uses an array guard until the shared control
// block takes ownership. These ABI helpers preserve the original unwind paths.
struct LVAlloc {
    char* pointer;
    char* allocator;
    // Guard a newly allocated message until ownership is explicitly released.
    LVAlloc(char* p, char* a) : pointer(p), allocator(a) {}
    // Free the message if construction did not transfer it to a control block.
    ~LVAlloc() {
        if (pointer)
            __dla__FPv(pointer);
    }
};
struct LVControlBase {
    void* table;
    unsigned strong, weak;
    // Start a shared-message control block with one strong and one weak reference.
    LVControlBase() {
        strong = 1;
        weak = 1;
        table = (void*)s_sharedCharDeleterVtable;
    }
};
struct LVControl : LVControlBase {
    char* pointer;
    // Adopt the message pointer into the initialized shared control block.
    LVControl(char* p) {
        pointer = p;
    }
};
struct LVErrBase {
    void* table;
    // Install the logic_error base vtable before derived construction.
    LVErrBase() {
        table = (void*)__vt__Q23std11logic_error;
    }
    // Destroy the exception base without freeing the enclosing stack storage.
    ~LVErrBase() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
namespace std {
struct length_error : LVErrBase {
    char* text;
    void* control;
    // Copy the secondary error message into a shared, owned character array.
    length_error() {
        const char* message = vectorLengthErrorStrings_LocalVector + 0x14;
        struct Allocator {
            char value;
            char pad[3];
        } allocator;
        allocator.value = 0;
        unsigned n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            LVAlloc allocation(p, &allocator.value);
            control = new LVControl(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = (void*)__vt__Q23std12length_error;
    }
    ~length_error();
    // Copy the primary vector-length message; the tag selects this ABI constructor.
    length_error(int) {
        const char* message = vectorLengthErrorStrings_LocalVector;
        struct Allocator {
            char value;
            char pad[3];
        } allocator;
        allocator.value = 0;
        unsigned n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            LVAlloc allocation(p, &allocator.value);
            control = new LVControl(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = (void*)__vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}
// Construct and throw the vector length exception using the original runtime ABI.
inline void LVThrow() {
    {
        union {
            double alignment;
            unsigned char bytes[sizeof(std::length_error)];
        } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct(0);
        exception_runtime::__throw(exception_runtime::vectorLengthErrorStrings_LocalVector + 0x14, error,
                                   (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}

// Temporary pointer-array allocation: [offset, offset+size) is populated. It owns
// storage only, not file references. After swap, data owns the OLD allocation.
struct LVBuffer {
    void* data;
    unsigned size;
    unsigned capacity;
    unsigned* owner;
    unsigned offset;
    // Start an empty reservation associated with the destination capacity word.
    LVBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Free whichever allocation remains after publication or unwinding.
    ~LVBuffer() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};

// Return a reference to the larger unsigned value; arguments must outlive its use.
static inline const unsigned& LVMax(const unsigned& a, const unsigned& b) {
    const unsigned* p = &a;
    if (a < b)
        p = &b;
    return *p;
}

// Initialize the shared-file-cache unit's writable two-pi cache.
void SharedFileCache_InitTwoPiCache() {
    s_twoPi_8028BC74 = kFloat_6_2831855;
}

struct DtorEntry {
    virtual ~DtorEntry();
    char data[16];
};

// Destroy the file-store array in reverse order and optionally free the vector wrapper.
// self is {data,count,...}; each store is 20 bytes. shouldDelete > 0 frees the
// wrapper too. Returned self is the destructor ABI result and may be dangling.
void* DtorEntryVector_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        if (*(void**)self != 0) {
            DtorEntry* endptr;
            int count = ((int*)self)[1]; // Vector count follows the data pointer.
            endptr = (DtorEntry*)*(void**)self + count;
            ((int*)self)[1] -= count;
            while (count != 0) {
                --endptr;
                endptr->~DtorEntry();
                count = count - 1;
            }
            appFreeUni__FPv(*(void**)self);
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Release a registered file under the lock, erasing it when its reference count reaches zero.
// file is an identity key, not a path. Return 0 if absent, 1 if a reference was
// released (even if other references keep the file in the array).
int SharedFileRegistry_ReleaseFile(void* self, void* file) {
    TableLock lock;
    TableLock_ctor(&lock, kRegistryLock, 1);
    struct Unlock {
        TableLock* lock;
        // Release the already acquired lock on return or exception unwinding.
        ~Unlock() {
            __dt__9TableLockFv(lock, -1);
        }
    } unlock = {&lock};
    unsigned int count = (*(unsigned int*)&AsCSharedFileRegistry(self)->filePointerCount);
    void** base = (*(void***)&AsCSharedFileRegistry(self)->filePointers);
    void** end = base + count;
    void** cur = base;
    // Find the exact object while holding lock domain 6 across release/compaction.
    while (cur != end && *cur != file) {
        cur++;
    }
    if (end == cur) {
        return 0;
    }
    if (((CSharedFile*)(*cur))->ReleaseRefIsZero() != 0) {
        if (*cur != 0) {
            delete (DtorEntry*)*cur;
            *cur = 0;
        }
        // Close the erased slot; memmove preserves the order of surviving pointers.
        unsigned int registeredCount = (*(unsigned int*)&AsCSharedFileRegistry(self)->filePointerCount);
        void* pointerBase = AsCSharedFileRegistry(self)->filePointers;
        int endAddress = (int)pointerBase + (int)registeredCount * kFilePointerBytes;
        int trailingBytes = endAddress - (int)cur;
        int trailingSlots = trailingBytes / kFilePointerBytes;
        int copyBytes = (trailingSlots - 1) * kFilePointerBytes;
        void* dst = cur;
        void* src = cur + 1;
        memmove(dst, src, (unsigned int)copyBytes);
        unsigned int c = (*(unsigned int*)&AsCSharedFileRegistry(self)->filePointerCount);
        (*(unsigned int*)&AsCSharedFileRegistry(self)->filePointerCount) = c - 1;
    }
    return 1;
}

// Initialize and register the shared-file registry with reserved pointer and store arrays.
// Returns a borrowed singleton; callers must not delete it. Initialization here
// is guarded by a byte, not the table lock, so first use requires external serialization.
void* CSharedFileRegistry_GetInstance() {
    if (!s_sharedFileRegistryInitialized) {
        char* registry = s_sharedFileRegistryInstance;
        *(void**)registry = (void*)__vt__19CSharedFileRegistry;
        (*(int*)&AsCSharedFileRegistry(registry)->filePointers) = 0;
        (*(int*)&AsCSharedFileRegistry(registry)->filePointerCount) = 0;
        ((RegistryCapacityView*)registry)->pointerCapacity = 0;
        (*(int*)&AsCSharedFileRegistry(registry)->fileRecords) = 0;
        (*(int*)&AsCSharedFileRegistry(registry)->fileRecordCount) = 0;
        ((RegistryCapacityView*)registry)->recordCapacity = 0;

        // Reserve the array of shared-file pointers.
        ReservationScratch localA;
        localA.resource = 0;
        localA.data = 0;
        localA.constructedCount = 0;
        localA.allocatorFlag = 0;
        localA.reservedFloat = 0;
        localA.reservedShort = 0;

        void* newA = appMallocDummy__FUl(kInitialRegistryCapacity * kFilePointerBytes);
        localA.resource = newA;
        void* oldBaseA = AsCSharedFileRegistry(registry)->filePointers;
        unsigned int oldCountA = (*(unsigned int*)&AsCSharedFileRegistry(registry)->filePointerCount);
        memcpy(newA, oldBaseA, oldCountA * 4);
        memset(oldBaseA, 0, oldCountA * 4);
        (*(unsigned int*)&AsCSharedFileRegistry(registry)->filePointerCount) = 0;
        ((RegistryCapacityView*)registry)->pointerCapacity = kInitialRegistryCapacity;
        *(unsigned int*)(&localA.constructedCount) = 0;
        AsCSharedFileRegistry(registry)->filePointers = newA;
        // Retain the old allocation in the reservation bookkeeping.
        localA.resource = oldBaseA;
        (*(unsigned int*)&AsCSharedFileRegistry(registry)->filePointerCount) = 0;
        if (oldBaseA != 0) {
            (*(unsigned int*)&AsCSharedFileRegistry(registry)->filePointerCount) = 0; // r0=0
            appFreeUni__FPv(oldBaseA);
        }

        // Reserve and copy-construct the array of 20-byte file-store records.
        if (((RegistryCapacityView*)registry)->recordCapacity < kInitialRegistryCapacity) {
            ReservationScratch localB;
            localB.resource = 0;
            localB.data = 0;
            localB.constructedCount = 0;
            localB.allocatorFlag = 0;
            localB.reservedFloat = 0;
            localB.reservedShort = 0;
            void* newB = appMallocDummy__FUl(kInitialRegistryCapacity * kFileStoreBytes);
            localB.resource = newB;
            unsigned int oldCountB = (*(unsigned int*)&AsCSharedFileRegistry(registry)->fileRecordCount);
            void* pNew = newB;
            void* pOld = AsCSharedFileRegistry(registry)->fileRecords;
            void* endOld = (DtorEntry*)pOld + oldCountB;
            unsigned int constructed = 0;
            // Copy-construct the file-store entries into the replacement allocation.
            while ((char*)pOld < (char*)endOld) {
                if (pNew != 0) {
                    __ct__16CSharedFileStoreFRC16CSharedFileStore(pNew, pOld);
                }
                pOld = (DtorEntry*)pOld + 1;
                pNew = (DtorEntry*)pNew + 1;
                constructed++;
            }
            unsigned int oldBaseB2 = (*(unsigned int*)&AsCSharedFileRegistry(registry)->fileRecords);
            if (oldBaseB2 != 0) {
                void* endPtr = (DtorEntry*)oldBaseB2 + oldCountB;
                int cnt = oldCountB;
                while (cnt != 0) {
                    endPtr = (DtorEntry*)endPtr - 1;
                    void** vt = *(void***)endPtr;
                    void (*fn)(void*, int) = (void (*)(void*, int))vt[2];
                    fn(endPtr, -1);
                    cnt--;
                }
                appFreeUni__FPv((void*)oldBaseB2);
            }
            // Publish the replacement store array.
            ((RegistryCapacityView*)registry)->recordCapacity = kInitialRegistryCapacity;
            AsCSharedFileRegistry(registry)->fileRecords = newB;
            (*(unsigned int*)&AsCSharedFileRegistry(registry)->fileRecordCount) = constructed;
            // Retain the old allocation in the reservation bookkeeping.
            localB.resource = (void*)oldBaseB2;
        }

        // Both allocations are published; register process-exit destruction before marking ready.
        __register_global_object(registry, (void*)__dt__19CSharedFileRegistryFv,
                                 (void*)s_sharedFileRegistryDtorRegistration);
        s_sharedFileRegistryInitialized = 1;
    }
    return (void*)s_sharedFileRegistryInstance;
}

extern int CString_Compare(void*, void*);
extern void __dla__FPv(void*);
extern void* __nwa__FUl(unsigned);

struct TargetLock {
    char bytes[8];
    // Acquire the shared registry's table-lock domain for the entire operation.
    TargetLock() {
        TableLock_ctor(this, kRegistryLock, 1);
    }
    // Release the lock on normal return or exception unwinding.
    ~TargetLock() {
        __dt__9TableLockFv(this, -1);
    }
};
extern void* __nw__FUl(unsigned int);
extern void __dt__Q23std12length_errorFv(void*, int);
struct TargetExc {
    void* vt;
    char* text;
    void* control;
};
// Build the primary vector-length message and throw it via the runtime exception ABI.
inline void TargetThrow() {
    TargetExc e;
    TargetExc* mp = &e;
    const char* message = vectorLengthErrorStrings_LocalVector;
    mp->vt = (void*)__vt__Q23std11logic_error;
    char alloc __attribute__((aligned(4))) = 0;
    unsigned len = strlen(message);
    char* text = (char*)__nwa__FUl((len + 1) * 2 - (len + 1));
    mp->text = text;
    {
        LVAlloc guard(text, &alloc);
        unsigned* rep = (unsigned*)__nw__FUl(0x10);
        if (rep) {
            rep[1] = 1;
            rep[2] = 1;
            rep[0] = (unsigned)&s_sharedCharDeleterVtable;
            rep[3] = (unsigned)text;
        }
        mp->control = rep;
        guard.pointer = 0;
    }
    strcpy(mp->text, message);
    mp->vt = (void*)&__vt__Q23std12length_error;
    __throw(vectorLengthErrorStrings_LocalVector + 0x14, mp, (void*)__dt__Q23std12length_errorFv);
}

struct TargetVector {
    void* table;
    void** data;
    unsigned size, capacity;
};
// Append one uninitialized pointer slot; caller must ensure size < capacity.
inline void*& TargetAppendSlot(TargetVector* self) {
    unsigned newSize = self->size + 1;
    void** base = self->data;
    self->size = newSize;
    return base[newSize - 1];
}

#pragma push
#pragma use_lmw_stmw on
// Reuse a named shared file or create, append, and retain it under the registry lock.
// path is a borrowed CString, not a C string. synchronous != 0 chooses the sync
// factory. Returns null on factory failure; otherwise returns a retained file
// that the caller must eventually pass to SharedFileRegistry_ReleaseFile.
void* SharedFileCache_GetOrCreate(TargetVector* self, void* path, int synchronous) {
    void** cur;
    void* found;
    void* newObj;
    int count;
    TargetLock lock;

    // Original 0x8028BC74: exhaustion supplies null; a hit breaks with the file.
    cur = self->data;
    while (self->data + self->size != cur || (found = 0, false)) {
        if (*cur != 0) {
            void* key = (void*)(unsigned long)(int)((CSharedFile*)(*cur))->GetPath();
            if (key != 0 && CString_Compare(key, path) == 0) {
                found = *cur;
                break;
            }
        }
        ++cur;
    }
    if (found != 0) {
        ((CSharedFile*)found)->AddRef();
        return found;
    }

    // Only a cache miss reaches a factory; existing files ignore the requested mode.
    if (synchronous != 0)
        newObj = CSharedFileSync::Create(path);
    else
        newObj = CSharedFileAsync::Create(path);
    if (newObj == 0) {
        return 0;
    }

    unsigned curSize = self->size;
    if (curSize < self->capacity) {
        TargetAppendSlot(self) = newObj;
    } else {
        // Reserve enough pointer slots, preserving the original piecewise growth policy.
        unsigned cap = self->capacity;
        if (kMaxFilePointerCount - cap < 1)
            TargetThrow();

        LVBuffer buffer(&self->capacity);
        unsigned growThree;
        unsigned growHalf;
        unsigned needed = (self->size + 1) - self->capacity;
        unsigned oldCap = self->capacity;
        if (needed > kMaxFilePointerCount - oldCap)
            TargetThrow();
        unsigned newCap;
        if (oldCap < kThreeFifthsGrowthLimit) {
            growThree = (3 * (oldCap + 1)) / 5;
            newCap = oldCap + LVMax(growThree, needed);
        } else if (oldCap < kHalfGrowthLimit) {
            growHalf = (oldCap + 1) / 2;
            newCap = oldCap + LVMax(growHalf, needed);
        } else {
            newCap = kMaxFilePointerCount;
        }
        if (newCap > kMaxFilePointerCount)
            TargetThrow();

        // Place the new file at the end, then move the old pointers before it.
        void* newBase = appMallocDummy__FUl(newCap * kFilePointerBytes);
        buffer.offset = self->size;
        buffer.data = newBase;
        ((void**)newBase + buffer.offset)[buffer.size] = newObj;
        buffer.capacity = newCap;
        buffer.size++;
        cur = self->data;
        int oldEnd = (int)cur + self->size * 4;
        count = (oldEnd - (int)cur) / 4;
        buffer.offset -= count;
        unsigned byteCount = count * 4;
        memcpy((void**)buffer.data + buffer.offset, cur, byteCount);
        memset(cur, 0, byteCount);
        buffer.size += count;
        // Publish the complete array; buffer's destructor now frees the old allocation.
        void* oldBasePtr = self->data;
        self->size = 0;
        unsigned oldCapacity = self->capacity;
        self->capacity = buffer.capacity;
        buffer.capacity = oldCapacity;
        self->data = (void**)buffer.data;
        buffer.data = oldBasePtr;
        self->size = buffer.size;
        buffer.size = 0;
    }

    ((CSharedFile*)newObj)->AddRef();
    return newObj;
}
#pragma pop

// Return the larger value by reference for immediate capacity arithmetic.
static inline const unsigned& maxu(const unsigned& a, const unsigned& b) {
    return a < b ? b : a;
}

#pragma push
#pragma use_lmw_stmw on
// Reuse a named shared file or create and retain a buffer-backed file under the registry lock.
// path is a borrowed CString; bytes are COPIED on a cache miss, not adopted.
// byteCount is a byte length (forwarded to the unsigned factory parameter).
// A cache hit ignores bytes/byteCount entirely. On a miss, null bytes or zero
// count returns null. Caller releases each nonnull result through the registry.
void* SharedFileCache_GetOrCreateFromBuffer(void* self, void* path, void* bytes, int byteCount) {
    TableLock lock;
    TableLock_ctor(&lock, kRegistryLock, 1);
    struct Unlock {
        TableLock* lock;
        // Attach scope cleanup to an already acquired lock.
        Unlock(TableLock* p) : lock(p) {}
        // Release the lock on normal return or exception unwinding.
        ~Unlock() {
            __dt__9TableLockFv(lock, -1);
        }
    } unlock(&lock);

    // Search by path first: a hit retains the existing file without consulting the buffer.
    void** cur = (*(void***)&AsCSharedFileRegistry(self)->filePointers);
    void* found = 0;
    while ((*(void***)&AsCSharedFileRegistry(self)->filePointers) +
               (*(unsigned int*)&AsCSharedFileRegistry(self)->filePointerCount) !=
           cur) {
        void* obj = *cur;
        if (obj != 0) {
            int key = (int)((CSharedFile*)obj)->GetPath();
            if (key != 0 && CString_Compare((void*)key, path) == 0) {
                found = *cur;
                break;
            }
        }
        cur++;
    }
    if (found != 0) {
        ((CSharedFile*)found)->AddRef();
        return found;
    }

    if (bytes == 0 || byteCount == 0) {
        return 0;
    }

    // Factory owns its own copied buffer; the caller's memory remains borrowed.
    void* newObj = ((CSharedFileSync*)path)->CreateFromBuffer(bytes, byteCount);
    if (newObj == 0) {
        return 0;
    }

    unsigned curSize = (*(unsigned int*)&AsCSharedFileRegistry(self)->filePointerCount);
    if (curSize < ((RegistryCapacityView*)self)->pointerCapacity) {
        void** b = (*(void***)&AsCSharedFileRegistry(self)->filePointers);
        unsigned newSize = curSize + 1;
        (*(unsigned int*)&AsCSharedFileRegistry(self)->filePointerCount) = newSize;
        b[newSize - 1] = newObj;
        ((CSharedFile*)newObj)->AddRef();
        return newObj;
    }

    // Reserve for one more pointer using the same growth thresholds as the path-only overload.
    unsigned cap = ((RegistryCapacityView*)self)->pointerCapacity;
    if (kMaxFilePointerCount - cap < 1) {
        LVThrow();
    }

    unsigned need = ((*(unsigned int*)&AsCSharedFileRegistry(self)->filePointerCount) + 1) - cap;
    if (need > kMaxFilePointerCount - cap) {
        LVThrow();
    }
    unsigned newCap;
    if (cap < kThreeFifthsGrowthLimit) {
        newCap = cap + maxu(3 * (cap + 1) / 5, need);
    } else if (cap < kHalfGrowthLimit) {
        newCap = cap + maxu((cap + 1) / 2, need);
    } else {
        newCap = kMaxFilePointerCount;
    }
    if (newCap > kMaxFilePointerCount) {
        LVThrow();
    }

    void* newBase = appMallocDummy__FUl(newCap * 4);
    void** oldBase = (*(void***)&AsCSharedFileRegistry(self)->filePointers);
    ((void**)newBase)[curSize] = newObj;
    memcpy(newBase, oldBase, curSize * 4);
    memset(oldBase, 0, curSize * 4);
    // Publish count/capacity/data only after all old pointers and the new file are in place.
    void* oldBasePtr = AsCSharedFileRegistry(self)->filePointers;
    (*(unsigned int*)&AsCSharedFileRegistry(self)->filePointerCount) = 0;
    ((RegistryCapacityView*)self)->pointerCapacity = newCap;
    AsCSharedFileRegistry(self)->filePointers = newBase;
    (*(unsigned int*)&AsCSharedFileRegistry(self)->filePointerCount) = curSize + 1;
    if (oldBasePtr != 0)
        appFreeUni__FPv(oldBasePtr);

    ((CSharedFile*)newObj)->AddRef();
    return newObj;
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462420", _ctors_u8028BC74_0, SharedFileCache_InitTwoPiCache);
#pragma pop
