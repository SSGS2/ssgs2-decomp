#include <decomp/static_init.h>
typedef unsigned long new_size_t;
// Translation unit 0x8005FE74-0x8006028C (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
extern "C" {
    void dtor_800641E8(void*, int);
    void RecordList_dtor(void*, int);
    void dtor_80064448(void*, int);
    void StringVector_dtor(void*, int);
    void CacheVectorOwner_dtor(void*, int);
    void dtor_8006599C(void*, int);
    extern float s_twoPi_8005FE74;
    extern float kFloat_6_2831855;
    extern const char s_loaderPrimaryEntryVectorBaseVtable[];
    extern const char s_loaderPrimaryEntryVectorVtable[];
    extern const char s_loaderSecondaryEntryVectorBaseVtable[];
    extern const char s_loaderSecondaryEntryVectorVtable[];
    extern const char sharedResourceLoaderStrings[];
    void* appMallocDummy__FUl(unsigned int);
    void appFreeUni__FPv(void*);
    void* appMallocUni__FUl(unsigned int);
    void OSInitMutex(void*);
    void OSLockMutex(void*);
    void OSUnlockMutex(void*);
    void fn_8036C0D8(const char*);
    void* memcpy(void*, const void*, unsigned int);
    void* memset(void*, int, unsigned int);
    // Initialize this module's two-pi cache at startup.
    void SharedResourceLoader_InitAngleCache() {
        s_twoPi_8005FE74 = kFloat_6_2831855;
    }
    enum {
        kPrimaryEntryCapacity = 0x400, kSecondaryEntryCapacity = 0x20,
        kPrimaryLookupSlots = 16, kSecondaryLookupSlots = 8,
        kLoaderInitialMode = 6 // Constructor value; the runtime meaning of this mode is unresolved.
    };

    // Both pools contain 16-byte OwnedEntry records, not raw resource bytes.
    // Their different vtables/capacities establish distinct containers; this unit does
    // not establish which resource kinds belong to each. Do not interchange them.
    struct OwnedEntryVector {
        void* vtable;
        void* data;
        unsigned int size, cap;
        void reserveA();
        void reserveB();
    };

    struct OwnedEntryVectorPrefix {
        void* vtable;
        void* data;
        unsigned int size;
    };

    struct OwnedEntry {
        unsigned int flags, unresolvedWord;
        void* data;
        unsigned int trailingWord;
        // Release only the payload marked owned by the high flag bit.
        ~OwnedEntry() {
            if (this) {
                if (flags >> 31) appFreeUni__FPv(data);
            }
        }
    };

    // Primary-pool temporary owner; its original type name is retained for unwind relocations.
    struct TempA {
        OwnedEntry* data;
        unsigned int size, cap;
        // Start an empty owner for a replacement primary-pool allocation.
        TempA():data(0), size(0), cap(0) {
        }
        // Destroy live entries in reverse order, then release the backing allocation.
        ~TempA() {
            if (data) {
                unsigned int n = size;
                size -= n;
                OwnedEntry* p = data+n;
                while (n) {
                    --p;
                    p->~OwnedEntry();
                    --n;
                }
                appFreeUni__FPv(data);
            }
        }
    };

    // Secondary-pool temporary owner; destroys moved-out entries after the storage swap.
    struct TempB {
        OwnedEntry* data;
        unsigned int size, cap;
        // Start an empty owner for a replacement secondary-pool allocation.
        TempB():data(0), size(0), cap(0) {
        }
        // Destroy live entries in reverse order, then release the backing allocation.
        ~TempB() {
            if (data) {
                unsigned int n = size;
                size -= n;
                OwnedEntry* p = data+n;
                while (n) {
                    --p;
                    p->~OwnedEntry();
                    --n;
                }
                appFreeUni__FPv(data);
            }
        }
    };
    // Initialize fixed primary-pool storage when size is below the pool limit.
    // The guard tests size, not capacity; existing payload ownership is transferred.
    inline void OwnedEntryVector::reserveA() {
        OwnedEntryVector* a = this;
        if (a->size < kPrimaryEntryCapacity) {
            // Allocate/copy/clear: clearing old records relinquishes their payloads.
            TempA t;
            t.data = (OwnedEntry*)appMallocDummy__FUl(kPrimaryEntryCapacity * sizeof(OwnedEntry));
            t.cap = kPrimaryEntryCapacity;
            OwnedEntry* first = (OwnedEntry*)a->data;
            int n = ((OwnedEntry*)a->data+a->size)-first;
            memcpy(t.data+t.size, first, n*16);
            memset(first, 0, n*16);
            t.size += n;
            // Commit: the temporary now owns only the cleared old allocation.
            OwnedEntry* p = (OwnedEntry*)a->data;
            unsigned int sz = a->size, cp = a->cap;
            a->cap = t.cap;
            t.cap = cp;
            a->data = t.data;
            t.data = p;
            a->size = t.size;
            t.size = sz;
        }
    }
    // Initialize fixed secondary-pool storage with the same size-guarded transfer.
    inline void OwnedEntryVector::reserveB() {
        OwnedEntryVector* b = this;
        if (b->size < kSecondaryEntryCapacity) {
            // Same transfer as reserveA; the trigger is size, not cap.
            TempB t;
            t.data = (OwnedEntry*)appMallocDummy__FUl(kSecondaryEntryCapacity * sizeof(OwnedEntry));
            t.cap = kSecondaryEntryCapacity;
            OwnedEntry* first = (OwnedEntry*)b->data;
            int n = ((OwnedEntry*)b->data+b->size)-first;
            memcpy(t.data+t.size, first, n*16);
            memset(first, 0, n*16);
            t.size += n;
            OwnedEntry* p = (OwnedEntry*)b->data;
            unsigned int sz = b->size, cp = b->cap;
            b->cap = t.cap;
            t.cap = cp;
            b->data = t.data;
            t.data = p;
            b->size = t.size;
            t.size = sz;
        }
    }

    // Mutex-protected table layout (byte offsets): a/b/c/d +00/+04/+08/+0c,
    // quantum +10, mask +14, count +18, owned slot array +1c, mutex +20.
    // Only construction is present here: a/b/c/d are zeroed, not proven indices
    // or pointers. quantum records the requested block size; count is allocated
    // slots and mask=count-1. Callers here use positive powers of two (16/8).
    // Changing those sizes requires auditing the external indexing/growth code;
    // mask is left untouched on the nonpositive path. Slots initially contain 0.
    struct Table {
        int a, b, c, d, quantum, mask, count;
        int* data;
        char lock[0x18];
        struct LockGuard {
            void* mutex;
            // Acquire the borrowed mutex for this scope.
            LockGuard(void* m) : mutex(m) {
                OSLockMutex(mutex);
            }
            // Release the mutex without destroying its storage.
            ~LockGuard() {
                OSUnlockMutex(mutex);
            }
        };
        // Allocate and clear n pointer-sized slots under the newly initialized lock.
        Table(int n) {
            data = 0;
            OSInitMutex(lock);
            LockGuard guard(lock);
            a = 0;
            d = 0;
            c = 0;
            quantum = n;
            b = 0;
            count = n;
            if (count > 0) {
                data = (int*)appMallocUni__FUl(count*4);
                mask = count-1;
                for (int i = 0;i < count;++i) data[i] = 0;
            } else data = 0;
        }
    };
}
// Placement construction borrows p; the caller retains storage ownership.
inline void* operator new(new_size_t, void* p) {
    return p;
}

extern "C" {
    // Construct both owned-entry pools, their locked lookup tables, and loader state.
    struct LoaderLinks {
        void* previous;
        void* next;
    };

    struct ResourceLoaderLayout {
        unsigned int headerWords[2];
        // Meaning of the two cleared words is unresolved.
        unsigned int pendingCount;
        LoaderLinks pendingLinks;
        unsigned int pendingFlags;
        OwnedEntryVector primaryEntries;
        OwnedEntryVector secondaryEntries;
        Table primaryLookup;
        unsigned int lookupState;
        char reservedBeforeFlags[0x80];
        // Four independent bytes cleared at startup; no consumer is defined here.
        unsigned char flags[4];
        Table secondaryLookup;
        unsigned int activeCount, activeCapacity;
        char reservedTail[0x40c];
        unsigned int mode;
    };
    typedef char LoaderFlagsOffset[((int)&((ResourceLoaderLayout*)0)->flags == 0xf4) ? 1 : -1];
    typedef char LoaderModeOffset[((int)&((ResourceLoaderLayout*)0)->mode == 0x544) ? 1 : -1];
    // Construct owned entry pools and lookup tables, then reset pending-load state.
    // self supplies at least 0x548 writable bytes; return self. On success the
    // pending list is an empty self-linked sentinel, both entry sizes are zero,
    // both lookup arrays contain nulls, and active counters/flag bytes are zero.
    // Only the fields written below are initialized; padding is not blanket-cleared.
    void* SharedResourceLoader_Ctor(void* self) {
        unsigned int zero = 0;
        OwnedEntryVector* primaryPool = &((ResourceLoaderLayout*)self)->primaryEntries;
        ((ResourceLoaderLayout*)self)->pendingCount = 0;
        ((ResourceLoaderLayout*)self)->pendingLinks.next = &((ResourceLoaderLayout*)self)->pendingLinks;
        ((ResourceLoaderLayout*)self)->pendingLinks.previous = &((ResourceLoaderLayout*)self)->pendingLinks;
        // Phase 1: create pools in order, with reverse-order cleanup on failure.
        primaryPool->vtable = (void*)s_loaderPrimaryEntryVectorBaseVtable;
        primaryPool->data = 0;
        primaryPool->size = 0;
        primaryPool->cap = 0;
        try {
            primaryPool->reserveA();
        } catch (...) {
            dtor_800641E8(&primaryPool->data, -1);
            RecordList_dtor(&((ResourceLoaderLayout*)self)->pendingCount, -1);
            throw;
        }
        primaryPool->vtable = (void*)s_loaderPrimaryEntryVectorVtable;
        *(unsigned int*)&((ResourceLoaderLayout*)self)->secondaryEntries.data = 0;
        OwnedEntryVector* secondaryPool = &((ResourceLoaderLayout*)self)->secondaryEntries;
        secondaryPool->vtable = (void*)s_loaderSecondaryEntryVectorBaseVtable;
        secondaryPool->data = 0;
        secondaryPool->size = 0;
        secondaryPool->cap = 0;
        try {
            secondaryPool->reserveB();
        } catch (...) {
            dtor_80064448(&secondaryPool->data, -1);
            StringVector_dtor(primaryPool, -1);
            RecordList_dtor(&((ResourceLoaderLayout*)self)->pendingCount, -1);
            throw;
        }
        secondaryPool->vtable = (void*)s_loaderSecondaryEntryVectorVtable;
        OwnedEntryVectorPrefix* secondaryPrefix = (OwnedEntryVectorPrefix*)&((ResourceLoaderLayout*)self)->secondaryEntries;
        (void)secondaryPrefix;
        // Phase 2: construct independently locked lookup arrays after both pools exist.
        bool firstTableConstructed = false;
        try {
            new (&((ResourceLoaderLayout*)self)->primaryLookup) Table(kPrimaryLookupSlots);
            firstTableConstructed = true;
            new (&((ResourceLoaderLayout*)self)->secondaryLookup) Table(kSecondaryLookupSlots);
        } catch (...) {
            if (firstTableConstructed)
            dtor_8006599C(&((ResourceLoaderLayout*)self)->primaryLookup, -1);
            CacheVectorOwner_dtor(secondaryPool, -1);
            StringVector_dtor(primaryPool, -1);
            RecordList_dtor(&((ResourceLoaderLayout*)self)->pendingCount, -1);
            throw;
        }
        // Phase 3: publish the constructor's initial state after ownership is established.
        ((ResourceLoaderLayout*)self)->headerWords[0] = 0;
        ((ResourceLoaderLayout*)self)->headerWords[1] = 0;
        fn_8036C0D8(sharedResourceLoaderStrings);
        ((ResourceLoaderLayout*)self)->flags[3] = 0;
        ((ResourceLoaderLayout*)self)->lookupState = 0;
        ((ResourceLoaderLayout*)self)->pendingFlags = 0;
        ((ResourceLoaderLayout*)self)->flags[0] = 0;
        ((ResourceLoaderLayout*)self)->flags[1] = 0;
        ((ResourceLoaderLayout*)self)->flags[2] = 0;
        ((ResourceLoaderLayout*)self)->activeCount = 0;
        ((ResourceLoaderLayout*)self)->activeCapacity = 0;
        ((ResourceLoaderLayout*)self)->mode = kLoaderInitialMode;
        return self;
    }

    struct VObj {
        virtual void m(int);
    };
    // Invoke the object's deleting virtual destructor when it exists.
    void Object_DeleteVirtual(void* ignored, VObj* object) {
        if (object == 0) return;
        if (object == 0) return;
        object->m(1);
    }
}
// extern "C"
/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80461FDC", _ctors_u8005FE74_0, SharedResourceLoader_InitAngleCache);
#pragma pop
