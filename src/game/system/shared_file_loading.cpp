#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include "MSL/error_message.h"
// Shared file loading, handle lookup and Conte (story scene) material resources.
// Mod lookup: CSharedFileAsync_Create/PollRead own asynchronous open/read state;
// CSharedFileSync_LoadFile normalizes DVD paths and reads padded byte counts;
// ConteResource_SetCategoryResourcePair selects material resources consumed by
// CConteChara in event_face_and_mpbase1_menu.cpp. Shared layouts: CSharedFileAsync.h/Sync.h.
// File table operations use lock 5; a borrowed lookup result is NOT protected
// after return. Keep the table owner alive and prevent concurrent removal.
// No gameplay clock tuning here: file lengths/alignment are bytes.
#include "MSL/vector.h"
// Vector destroyed by dtor_8023F0E4; element type not known yet (Elem_8023F0E4 is a tag);
// its size reset loads the size twice (form 2).
struct Elem_8023F0E4;
namespace std { template <> struct __vec_destroys_elements<Elem_8023F0E4*> { enum { value = 2 }; static void destroy(Elem_8023F0E4**) {} }; }
typedef std::vector<Elem_8023F0E4*, app::app_allocator<Elem_8023F0E4*> > Elem_8023F0E4Vector;
#include "iff/IffStruct.h"
// Compare vector cursors without changing the original reference-based expression.
template <class T> inline bool same_pos(const T& a, const T& b) { return a == b; }
// Translation unit 0x8023E86C-0x8023F8DC (24 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "classes/CSharedFileAsync.h"
#include "classes/CSharedFileSync.h"
#include <decomp/cw_bitfields.h>

// Borrow the sync layout; retain expression expansion to preserve unwind relocations.
#define AsCSharedFileSync(p) ((CSharedFileSync*)(p))

// Borrow the async layout; retain expression expansion to preserve unwind relocations.
#define AsCSharedFileAsync(p) ((CSharedFileAsync*)(p))

// Pointer vector layout shared by the file lookup, removal, and reserve paths.
// Slots own handle wrappers; wrappers retain the shared file. Lookup results and buffers
// are borrowed until removal/destruction. Table lock 5 protects the vector, not a retained result.
struct SharedFilePointerVector { void** data; unsigned count, capacity; };
struct SharedFileAsyncStorage { CSharedFileAsync fields; OSMutex mutex; };
typedef char SharedFileAsyncStorage_mutex_offset[(IFF_OFFSET(SharedFileAsyncStorage, mutex) == 0x24) ? 1 : -1];
typedef char SharedFileAsyncStorage_size[(sizeof(SharedFileAsyncStorage) == 0x3C) ? 1 : -1];
enum { kSharedFileReserveCount = 1024, kFileTableLockId = 5,
    kDvdAlignmentMask = 31, kAsyncReadStarted = 0x40000000,
    kAsyncReadComplete = 0x80000000, kOpenExistingFile = 3,
    kFileReadAccess = 0x80000000, kFileSequentialAccess = 0x20000000 };

extern "C" {
 
extern float kFloat_6_2831855;

extern int CSharedFile_GetLoadedBuffer(void*);
extern int CSharedFile_GetPath(void*);
extern int CFileAsync_GetSize(void*);
extern float s_twoPi_SharedFile;
extern float s_twoPi_SharedFileSync;
extern float s_twoPi_SharedFileBuffer;
extern float s_twoPi_ConteResource;

// Two resource ids per Conte model slot (see ConteResource_SetCategoryResourcePair); the type name is the original's address tag.
struct Pair8023F890 { int firstMaterialResource; int secondMaterialResource; };
extern void TableLock_ctor(void*, int, int);
// Acquire the requested file-table lock; destruction releases it via the shared runtime.
struct TableLock { int state[2]; TableLock(int a, int b) { TableLock_ctor(this, a, b); } ~TableLock(); };
extern Pair8023F890 s_conteModelResourcePairs_Mode0[];
extern Pair8023F890 s_conteCategory1ResourcePairs[];
extern char __vt__15CSharedFileSync[256];
extern char __vt__16CSharedFileAsync[28];

extern void CSharedFile_dtor(void*, int);
extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);
extern void* appMallocDummy__FUl(unsigned long);
extern void* __nwa__FUl(unsigned long);
extern void* memcpy(void*, const void*, unsigned long);
extern void* memset(void*, int, unsigned long);
extern void* __nw__FUl(unsigned long);
extern void TableLock_ctor(void*, int, int);
extern void __dt__9TableLockFv(void*, int);
extern void* FileList_FindByName(void*, void*);
extern void CSharedFile_Ctor(void*);
extern void CSharedFile_SetPath(void*, void*);
extern void CSharedFileHandle_Destroy(void*, int);
extern int CSharedFileSync_LoadFile(void*, void*);
extern int CFileAsync_PollRead(void*, void*, int, int);
extern int CFileAsync_StartRead(void*, void*, unsigned int);
extern void String_ReserveCapacity(void*, unsigned int);

extern void* CHashPath_FindPath(void*);
extern void CFileAsync_ctor(void*);
extern int CFileAsync_Open(void*, void*, int);
// The definition takes ten arguments; the original call leaves the last two registers unset (and unread).
extern int FileIO_OpenFile(void*, unsigned int, unsigned int, unsigned int, int, unsigned int, unsigned int, unsigned int);
extern int FileIO_GetFileSize(unsigned int, unsigned int, unsigned int, unsigned int);
extern int FileIO_ReadFile(int, void*, unsigned int, void*, int, int, unsigned int, unsigned int);
extern void FileIO_CloseFile(int);
extern int DVDGetDriveStatus(void);
extern void FilePath_EnsureDvdPrefix(void*);
extern void String_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, const char*);

// Small-string layout: bit 31 selects heap data; otherwise text starts at byte
// +1 INSIDE the tag word and overlaps the remaining words of this 12-byte object.
// Only heap mode interprets capacity/data as separate fields and frees data.
// The type name is an address tag from the original symbols.
struct String_80049268 {
    unsigned int tag;
    unsigned int capacity;
    char* data;
    // Free the normalized path's heap buffer, if any.
    ~String_80049268() { if (tag >> 31) appFreeUni__FPv(data); }
};

// Return the handle's borrowed loaded-buffer address through the 32-bit integer ABI, or zero.
int CSharedFilePtr_GetLoadedBuffer(void* a) {
    void* p = *(void**)a;
// BinaryTable files (.ecb/.eky) are big-endian on disc.
    if (p != 0) return CSharedFile_GetLoadedBuffer(p);
    return 0;
}

struct V8023E884 { 
virtual int m0(); virtual int m1(); virtual int m2(); virtual int m3();
};
// Return the referenced file's virtual size, or zero for an empty handle.
int CSharedFilePtr_GetSize(void* a) {
    V8023E884* p = (V8023E884*)*(void**)a;
    if (p != 0) return p->m1();
    return 0;
}

#pragma dont_inline on
// Return the handle's borrowed path address through the integer ABI, or zero for no file.
int CSharedFilePtr_GetPath(void* a) {
    void* p = *(void**)a;
    if (p != 0) return CSharedFile_GetPath(p);
    return 0;
}
#pragma dont_inline off

// Return the async handle's logical byte length, or zero when no async handle exists.
int CSharedFileAsync_GetSize(void* a) {
    void* p = AsCSharedFileAsync(a)->fileAsync;
    if (p != 0) return CFileAsync_GetSize(p);
    return 0;
}

// Destroy the synchronous file base and optionally free the allocation.
void* CSharedFileSync_dtor(void* self, int flag) {
    if (self != 0) {
        CSharedFile_dtor(self, 0);
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

// Return logical file bytes, excluding any alignment padding in the allocation.
int CSharedFileSync_vfunc_1(void* a) {
    return AsCSharedFileSync(a)->fileSize;
}

// Synchronous files are ready immediately.
int CSharedFileSync_vfunc_3(void* a) {
    return 1;
}

// Initialize the shared-file angle constant.
void SharedFile_InitializeAngleConstant(void) {
    s_twoPi_SharedFile = kFloat_6_2831855;
}

extern void* SharedFile_Create(void*, int);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern void __dt__Q33std6detail16char_array_guardFv(void*, int);
extern void __dla__FPv(void*);
extern void __dt__Q23std12length_errorFv(void*, int);
extern char sharedFileVectorErrorStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

struct BufGuard_8023E8CC {
    char* data;
    void* alloc;
    // Guard message storage until its shared owner is constructed.
    BufGuard_8023E8CC(char* p, void* a):data(p),alloc(a) {}
    // Transfer message storage out of the guard and return the pointer.
    char* release() { char* p = data; data = 0; return p; }
    // Free message storage if ownership was not transferred.
    ~BufGuard_8023E8CC() { if (data) __dla__FPv(data); }
};

// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h; keep this local alias for callers.
typedef ::std::detail::__message_control ExcCount_8023E8CC;

struct ErrBase_8023E8CC {
    void* table;
    char* data;
    ExcCount_8023E8CC* owner;
    // Allocate and copy the exception text under a buffer guard.
    void init(const char* const& text) {
        table = (void*)__vt__Q23std11logic_error;
        struct Alloc {
            unsigned char z;
            char pad[3];
            // Initialize the allocator marker byte for message construction.
            Alloc():z(0) {}
        } z;
        unsigned len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufGuard_8023E8CC guard(p, &z);
            owner = new ExcCount_8023E8CC(p);
            guard.release();
        }
        strcpy(data, text);
    }
    // Construct the logic-error message and shared control block.
    ErrBase_8023E8CC(const char* const& text) { init(text); }
    // Destroy the exception base without deleting this storage.
    ~ErrBase_8023E8CC() { __dt__Q23std9exceptionFv(this, 0); }
};

namespace std {
struct length_error : ErrBase_8023E8CC {
    // Install the length-error table after constructing its message base.
    length_error(const char* const& p):ErrBase_8023E8CC(p) {
        table = (void*)__vt__Q23std12length_error;
    }
    ~length_error();
};
}
// Construct the vector length exception in temporary storage and transfer it to the runtime.
inline void throwLE_8023E8CC() {
    char storage[sizeof(ErrBase_8023E8CC)];
    ErrBase_8023E8CC* e = (ErrBase_8023E8CC*)storage;
    e->init((const char*)sharedFileVectorErrorStrings);
    e->table = (void*)__vt__Q23std12length_error;
    __throw(sharedFileVectorErrorStrings + 0x14, e, (void*)__dt__Q23std12length_errorFv);
}

struct GrowBuffer_8023E8CC {
    void** data;
    unsigned int count, capacity;
    unsigned int* allocator;
    unsigned int offset;
    // Track pending pointer-vector storage until an ownership exchange.
    GrowBuffer_8023E8CC(unsigned int* a, unsigned int n):data(0),count(n),capacity(n),allocator(a),offset(n) {}
    // Free whichever pointer-vector backing block remains with this guard.
    ~GrowBuffer_8023E8CC() {
        if (data) {
            count = 0;
            appFreeUni__FPv(data);
        }
    }
};

// Select the greater growth count by reference, preserving temporary lifetimes in the caller.
inline const unsigned int& maxGrow_8023E8CC(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

// Compute a pointer-count capacity with overflow checks; g1/g2 hold branch-specific growth.
inline unsigned int capacity_8023E8CC(unsigned int cap, const unsigned int& minGrow, unsigned int& g1, unsigned int& g2) {
    if (minGrow > 0x3FFFFFFFu - cap) throwLE_8023E8CC();
    unsigned int newCap;
    if (cap < 0x15555555u) {
        g1 = ((cap + 1) * 3) / 5;
        newCap = cap + maxGrow_8023E8CC(g1, minGrow);
    } else if (cap < 0x2AAAAAAAu) {
        g2 = (cap + 1) >> 1;
        newCap = cap + maxGrow_8023E8CC(g2, minGrow);
    } else {
        newCap = 0x3FFFFFFFu;
    }
    if (newCap > 0x3FFFFFFFu) throwLE_8023E8CC();
    return newCap;
}

#pragma push
#pragma use_lmw_stmw on
// Create the requested shared-file handle, grow the pointer vector if needed, and append it under the file-table lock.
// Return 1 after append, or 0 if handle creation fails; the list owns the new handle.
// Capacity/allocation errors propagate. Returned lookup handles remain borrowed.
int SharedFileList_CreateAndAppend(void* self_, void* path, int loadMode) {
    char* self = (char*)self_;
    TableLock lock(kFileTableLockId, 1);
    void* newHandle = SharedFile_Create(path, loadMode);
    if (newHandle == 0) return 0;
    unsigned int cap;
    unsigned int count = *(unsigned int*)(self + IFF_OFFSET(SharedFilePointerVector, count));
    cap = *(unsigned int*)(self + IFF_OFFSET(SharedFilePointerVector, capacity));
    if (count < cap) {
        unsigned int n = count + 1;
        *(unsigned int*)(self + IFF_OFFSET(SharedFilePointerVector, count)) = n;
        ((void**) *(void**)self)[n - 1] = newHandle;
    } else {
    // Reserve a guarded replacement buffer, construct the new tail, then copy old slots.
    if (0x3FFFFFFFu - cap < 1) throwLE_8023E8CC();
    unsigned int g1, g2;
    unsigned int* allocator = &*(unsigned int*)(self + IFF_OFFSET(SharedFilePointerVector, capacity));
    unsigned int initialCount = 0;
    GrowBuffer_8023E8CC buffer(allocator, initialCount);
    unsigned int minGrow = *(unsigned int*)(self + IFF_OFFSET(SharedFilePointerVector, count)) + 1 - *(unsigned int*)(self + IFF_OFFSET(SharedFilePointerVector, capacity));
    cap = *(unsigned int*)(self + IFF_OFFSET(SharedFilePointerVector, capacity));
    unsigned int newCap = capacity_8023E8CC(cap, minGrow, g1, g2);
    void** data = (void**)appMallocDummy__FUl((unsigned long)newCap * 4);
    unsigned int size = *(unsigned int*)(self + IFF_OFFSET(SharedFilePointerVector, count));
    buffer.offset = size;
    buffer.data = data;
    (data + size)[buffer.count] = newHandle;
    void** oldBase = *(void***)self;
    int moved;
    unsigned long moveBytes;
    buffer.capacity = newCap;
    buffer.count += 1;
    moved = (int)((char*)(oldBase + *(unsigned int*)(self + IFF_OFFSET(SharedFilePointerVector, count))) - (char*)oldBase) / 4;
    buffer.offset = size - moved;
    moveBytes = (unsigned long)moved * 4;
    memcpy(data + buffer.offset, oldBase, moveBytes);
    memset(oldBase, 0, moveBytes);
    // Commit new storage; the guard takes the old block and frees it on scope exit.
    void** newData;
    unsigned int zero = 0;
    unsigned int newCapacity;
    unsigned int total;
    total = buffer.count + moved;
    buffer.count = total;
    void** oldData = *(void***)self;
    newData = buffer.data;
    unsigned int oldCap = *(unsigned int*)(self + IFF_OFFSET(SharedFilePointerVector, capacity));
    newCapacity = buffer.capacity;
    *(unsigned int*)(self + IFF_OFFSET(SharedFilePointerVector, count)) = zero;
    buffer.data = oldData;
    *(unsigned int*)(self + IFF_OFFSET(SharedFilePointerVector, capacity)) = newCapacity;
    buffer.capacity = oldCap;
    *(void***)self = newData;
    *(unsigned int*)(self + IFF_OFFSET(SharedFilePointerVector, count)) = total;
    buffer.count = zero;
    }
    return 1;
}
#pragma pop

// Initialize the synchronous-file angle constant.
void SharedFileSync_InitializeAngleConstant(void) {
    s_twoPi_SharedFileSync = kFloat_6_2831855;
}

// Initialize the file-buffer angle constant.
void SharedFileBuffer_InitializeAngleConstant(void) {
    s_twoPi_SharedFileBuffer = kFloat_6_2831855;
}

// Initialize the resource-pair angle constant.
void ConteResource_InitializeAngleConstant(void) {
    s_twoPi_ConteResource = kFloat_6_2831855;
}

// Update one of the two supported category tables using the low byte of the model index.
// Only categories 0 and 1 are accepted: 0 writes s_conteModelResourcePairs_Mode0, 1 writes s_conteCategory1ResourcePairs;
// anything else is ignored. In the != 1 branch, 'category >= 1' can only mean category > 1 and '< 0' rejects negatives.
// Bounds are caller obligations, NOT validated by the low-byte mask: category 0
// has 8 pairs (64 bytes at 0x80504098), category 1 has 26 (208 at 0x805040D8).
// Require (modelIndex & 255) < 8 or < 26 respectively; higher index bits alias.
// firstResource/secondResource are pointer-shaped resource words, forwarded by
// event_face_and_mpbase1_menu.cpp to WPuppet_ApplyNamedMaterialLayerAnimation for its two channels.
// This setter stores bits only: no AddRef, allocation, release or validation.
// A resource owner must keep any addressed object alive until the pair is replaced
// and all scene users finish; replacing a pair does not release the old resources.
// Zero/sentinel meanings belong to that consumer, not to this setter. Serialize
// table edits with scene animation. No result or failure code is returned.
void ConteResource_SetCategoryResourcePair(int category, int modelIndex, int firstResource, int secondResource) {
    if (category != 1) {
        if (category >= 1) return;
        if (category < 0) return;
        {
            int tableIndex = modelIndex & 0xFF;
            s_conteModelResourcePairs_Mode0[tableIndex].firstMaterialResource = firstResource;
            s_conteModelResourcePairs_Mode0[tableIndex].secondMaterialResource = secondResource;
        }
        return;
    }
    {
        int tableIndex = modelIndex & 0xFF;
        s_conteCategory1ResourcePairs[tableIndex].firstMaterialResource = firstResource;
        s_conteCategory1ResourcePairs[tableIndex].secondMaterialResource = secondResource;
    }
}

// Release the pointer-vector allocation and optionally delete its wrapper.
// The five nested 'self != 0' tests are repeats from inlined destructor layers; in plain C: free(data) if set, then delete self.
void* dtor_8023F0E4(void* self, int flag) {
    return ::std::__vector_deleting_dtor((Elem_8023F0E4Vector*)self, flag);
}

// Find the named file and return its loaded-buffer result.
// The return is a borrowed address via int, not a byte count. Zero means missing/not loaded.
int FileList_GetLoadedBufferByName(void* self, void* arg2) {
    TableLock lock(kFileTableLockId, 1);
    void* r = FileList_FindByName(self, arg2);
    if (r == 0) {
        return 0;
    }
    int (*f)(void*) = CSharedFilePtr_GetLoadedBuffer;
    int r2 = f((void*)r);
    return r2;
}

// Allocate a synchronous shared file and destroy it if loading its path fails.
void* CSharedFileSync_Create(void* arg) {
    struct PlainF170 {
        char data[0x1c];
        // Initialize an empty synchronous shared file before disk loading.
        PlainF170() {
            CSharedFile_Ctor(this);
            *(void**)this = (void*)&__vt__15CSharedFileSync;
            AsCSharedFileSync(this)->fileSize = 0;
        }
    };
    struct FooF170 { 
    virtual void del(int) DECOMP_LOCAL_VIRTUAL;
    };
    PlainF170* r31 = new PlainF170;
    if (r31 == 0) {
        return 0;
    }
    if (CSharedFileSync_LoadFile(r31, arg) == 0) {
        if (r31 != 0) {
            ((FooF170*)r31)->del(1);
        }
        return 0;
    }
    return r31;
}

// Find a shared-file handle by its stored data name while holding the file-table lock.
// The returned wrapper is borrowed; the lock is released before the caller uses it.
void* FileList_FindByName(void* self, void* arg) {
    TableLock lock(kFileTableLockId, 1);
    if (arg == 0) {
        return 0;
    }
    char* p = (char*)*(void**)self;
    while (!same_pos((char*)*(void***)self + (int)((SharedFilePointerVector*)self)->count * 4, p)) {
        const char* s = (const char*)CSharedFilePtr_GetPath(*(void**)p);
        if (s != 0 && strcmp(s, (const char*)arg) == 0) {
            p = *(char**)p;
            return p;
        }
        p += 4;
    }
    return 0;
}

#pragma push
#pragma opt_propagation off
#pragma use_lmw_stmw on
struct BufEFEC {
    void** data;
    unsigned int count, capacity;
    // Initialize the reserve-path buffer guard from the supplied zero count.
    BufEFEC(unsigned int* a, unsigned int n): data((void**)n), count(n), capacity(n) {}
    // Reduce the guard's tracked pointer-slot count.
    void clear(unsigned int n) { count -= n; }
    // Clear the tracked slot count and free reserve-path backing storage.
    ~BufEFEC() {
        void** p = data;
        if (p) {
            clear(count);
            appFreeUni__FPv(p);
        }
    }
};
// Swap vector counts/capacities without changing pointer payloads.
inline void swapWordEFEC(unsigned int& a, unsigned int& b) {
    unsigned int t = a; a = b; b = t;
}
// Exchange vector backing pointers for ownership transfer.
inline void swapDataEFEC(void**& a, void**& b) {
    void** t = a; a = b; b = t;
}

#pragma push
#pragma use_lmw_stmw on
// Initialize an empty file-pointer vector and reserve space for 1024 handles.
void* SharedFilePointerVector_InitReserved1024(void* a) {
    char* self = (char*)a;
    unsigned int r30 = 0;
    *(unsigned int*)&((SharedFilePointerVector*)self)->data = r30;
    ((SharedFilePointerVector*)self)->count = r30;
    ((SharedFilePointerVector*)self)->capacity = r30;
    if (r30 < (unsigned)kSharedFileReserveCount) {
        BufEFEC buffer(&((SharedFilePointerVector*)self)->capacity, r30);
        void** data = (void**)appMallocDummy__FUl(kSharedFileReserveCount * sizeof(void*));
        int moved;
        void** base = *(void***)self;
        buffer.data = data;
        buffer.capacity = kSharedFileReserveCount;
        moved = (int)((char*)(base + ((SharedFilePointerVector*)self)->count) - (char*)base) / 4;
        unsigned long bytes = (unsigned long)moved * 4;
        memcpy(data + ((SharedFilePointerVector*)self)->count, base, bytes);
        memset(base, 0, bytes);
            buffer.count += moved;
            ((SharedFilePointerVector*)self)->count = r30;
            swapWordEFEC(((SharedFilePointerVector*)self)->capacity, buffer.capacity);
            swapDataEFEC(*(void***)self, buffer.data);
            swapWordEFEC(((SharedFilePointerVector*)self)->count, buffer.count);
    }
    return self;
}
#pragma pop

#pragma pop

// Poll pending I/O to completion, release its async handle, and destroy the shared-file base.
void* CSharedFileAsync_dtor(void* self, int flag) {
    if (self != 0) {
        *(void**)self = (void*)__vt__16CSharedFileAsync;
        while (1) {
            void* p = AsCSharedFileAsync(self)->fileAsync;
            if (p == 0) break;
            struct FooF698 { 
            virtual void m0() DECOMP_LOCAL_VIRTUAL; virtual void m1() DECOMP_LOCAL_VIRTUAL; virtual void m2() DECOMP_LOCAL_VIRTUAL; virtual int m3() DECOMP_LOCAL_VIRTUAL;
            };
            int r = ((FooF698*)self)->m3();
            if (r != 0) break;
        }
        void* p = AsCSharedFileAsync(self)->fileAsync;
        if (p != 0) {
            if (p != 0) {
                struct Pad8023F698 { int d[3]; };
                struct VTail8023F698 : Pad8023F698 { 
                virtual void m0(int) DECOMP_LOCAL_VIRTUAL; virtual void m1(int) DECOMP_LOCAL_VIRTUAL;
                };
                ((VTail8023F698*)p)->m0(1);
            }
            AsCSharedFileAsync(self)->fileAsync = 0;
        }
        CSharedFile_dtor(self, 0);
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

// Create a named synchronous shared file and copy the caller's bytes into its buffer.
// self is the path/name argument, not an existing receiver; src supplies n borrowed bytes.
// Return a newly owned file object, or zero if construction/buffer allocation fails.
void* CSharedFileSync_CreateFromBuffer(void* self, void* src, unsigned int n) {
    struct PlainF21C {
        char data[0x1c];
        // Initialize an empty synchronous shared file before copying caller bytes.
        PlainF21C() {
            CSharedFile_Ctor(this);
            *(void**)this = (void*)&__vt__15CSharedFileSync;
            AsCSharedFileSync(this)->fileSize = 0;
        }
    };
    PlainF21C* r31 = new PlainF21C;
    if (r31 == 0) {
        return 0;
    }
    CSharedFile_SetPath(r31, self);
    (*(unsigned int*)&AsCSharedFileSync(r31)->fileSize) = n;
    struct FooF21C0 { 
    virtual void m0() DECOMP_LOCAL_VIRTUAL; virtual void m1() DECOMP_LOCAL_VIRTUAL; virtual int m2(unsigned int) DECOMP_LOCAL_VIRTUAL;
    };
    int r = ((FooF21C0*)r31)->m2(n);
    int ok = 0;
    if (r == 0) {
        ok = 0;
    } else {
        Memory_Copy(AsCSharedFileSync(r31)->buffer, src, n);
        ok = 1;
    }
    if (ok == 0) {
        if (r31 != 0) {
            struct FooF21C1 { 
            virtual void del(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((FooF21C1*)r31)->del(1);
        }
        return 0;
    }
    return r31;
}

// Destroy the matching handle and compact the pointer vector under the file-table lock.
// Return 1 after the first match is removed, 0 if absent. name and stored paths must be nonnull.
int SharedFileHandleList_RemoveByDataName(void* self, void* name) {
    TableLock lock(kFileTableLockId, 1);
    unsigned int byteOffset;
    unsigned int index = 0;
    byteOffset = 0;
    while (index < ((SharedFilePointerVector*)self)->count) {
        void* handle = *(void**)((char*)*(void**)self + byteOffset);
        int (*f)(void*) = CSharedFilePtr_GetPath;
        int dataName = f(handle);
        if (strcmp((const char*)dataName, (const char*)name) == 0) {
            void* matchedHandle = *(void**)((char*)*(void**)self + byteOffset);
            if (matchedHandle != 0) {
                CSharedFileHandle_Destroy(matchedHandle, 1);
            }
            *(void**)((char*)*(void**)self + byteOffset) = 0;
            unsigned int count = ((SharedFilePointerVector*)self)->count;
            void** dst = *(void***)self + index;
            void** end = *(void***)self + count;
            memmove(dst, dst + 1, (unsigned int)(end - dst - 1) * 4);
            ((SharedFilePointerVector*)self)->count = ((SharedFilePointerVector*)self)->count - 1;
            return 1;
        }
        index++;
        byteOffset += 4;
    }
    return 0;
}

struct WInstanceLock {
    void* mutex;
    // Acquire the async object's recursive mutex.
    WInstanceLock(void* m): mutex(m) { OSLockMutex((OSMutex*)m); }
    // Release the async mutex on normal exit or unwinding.
    ~WInstanceLock() { OSUnlockMutex((OSMutex*)mutex); }
};

struct VMethods8023F76C { 
virtual void m0(); virtual void m1(); virtual void m2(unsigned int);
};
struct Flags8023F76C { CW_BITSP2(int, 30, int bit31 : 1, int bit30 : 1) };

// Under the mutex, start the aligned read after open completion, then latch read completion.
// Return 1 only when completion is latched, 0 while pending. readFlags bit 30 means
// read issued and bit 31 means complete; alignedReadSize rounds logical size up to 32 bytes.
// Requires a successfully created live self/fileAsync. Size must be nonnegative
// and size+31 must fit int. The allocation virtual slot must supply readBuffer
// with alignedReadSize writable bytes and DVD alignment; this routine ignores
// allocation status and StartRead's rejection status, then sets bit30 anyway.
// State map: 00 wait for open -> 01 read issued -> 11 completion latched (bits
// listed as complete/started). One successful ready poll starts the read; another
// successful ready poll latches completion. Failed polls and ready==0 leave the
// flags unchanged; there is NO terminal-error state or error result in this API.
// Thus 0 can mean pending forever after an I/O failure/rejected start. 1 means
// the low-level poll reported ready, not that this routine checked byte validity.
// Do not consume readBuffer before completion or free it while I/O can use it.
// Object/base destruction owns buffer cleanup; async destruction waits through
// this poll then deletes the low-level handle. It can spin forever on that same
// unresolved failure path. The mutex protects transitions; the early completion
// read is outside it, so callers still need a valid object lifetime throughout.
int CSharedFileAsync_PollRead(void* self) {
    int r0 = AsCSharedFileAsync(self)->readFlags;
    r0 = r0 >> 31;
    if (r0 != 0) {
        return 1;
    }
    int ready;
    int aligned;
    WInstanceLock lock(&((SharedFileAsyncStorage*)self)->mutex);
    // Phase 1: wait for open; allocate/read once after the file size becomes available.
    if (!((Flags8023F76C*)(((char*)&AsCSharedFileAsync(self)->readFlags)))->bit30) {
        if (CFileAsync_PollRead(AsCSharedFileAsync(self)->fileAsync, &ready, 0, 0) != 0 && ready != 0) {
            int sz = CFileAsync_GetSize(AsCSharedFileAsync(self)->fileAsync);
            int tmp = sz + kDvdAlignmentMask;
            aligned = tmp & ~kDvdAlignmentMask;
            (*(int*)&AsCSharedFileAsync(self)->alignedReadSize) = aligned;
            ((VMethods8023F76C*)self)->m2(aligned);
            CFileAsync_StartRead(AsCSharedFileAsync(self)->fileAsync, AsCSharedFileAsync(self)->readBuffer, AsCSharedFileAsync(self)->alignedReadSize);
            AsCSharedFileAsync(self)->readFlags |= kAsyncReadStarted;
        }
    } else {
        // Phase 2: poll the issued read; completion remains latched for future calls.
        if (CFileAsync_PollRead(AsCSharedFileAsync(self)->fileAsync, &ready, 0, 0) != 0 && ready != 0) {
            AsCSharedFileAsync(self)->readFlags |= (int)kAsyncReadComplete;
            return 1;
        }
    }
    return 0;
}

#pragma rtti off
// Create an async file object, resolve its path, and start opening it; unwind on failure.
void* CSharedFileAsync_Create(void* path) {
    struct SelfV { 
    virtual void del(int) DECOMP_LOCAL_VIRTUAL;
    };
    struct HandlePad { char pad[0xc]; };
    struct HandleV : HandlePad { 
    virtual void del(int) DECOMP_LOCAL_VIRTUAL;
    };

    void* self = __nw__FUl(0x3c);
    void* file = self;
    if (self != 0) {
        CSharedFile_Ctor(self);
        *(void**)self = (void*)__vt__16CSharedFileAsync;
        unsigned int v = (*(unsigned int*)&AsCSharedFileAsync(self)->readFlags);
        v = v & 0x3FFFFFFF;
        (*(unsigned int*)&AsCSharedFileAsync(self)->readFlags) = v;
        OSInitMutex((OSMutex*)(&((SharedFileAsyncStorage*)self)->mutex));
    }
    if (file == 0) {
        return 0;
    }
    CSharedFile_SetPath(file, path);
    void* p = __nw__FUl(0x10);
    void* asyncHandle = p;
    if (p != 0) {
        CFileAsync_ctor(p);
    }
    AsCSharedFileAsync(file)->fileAsync = asyncHandle;
    unsigned int opened;
    if (asyncHandle == 0) {
        opened = 0;
    } else {
        void* r = CHashPath_FindPath(path);
        int ok = CFileAsync_Open(AsCSharedFileAsync(file)->fileAsync, r, 0);
        if (ok == 0) {
            if (AsCSharedFileAsync(file)->fileAsync != 0) {
                if (AsCSharedFileAsync(file)->fileAsync != 0) {
                    void* obj = AsCSharedFileAsync(file)->fileAsync;
                    ((HandleV*)obj)->del(1);
                }
            }
            AsCSharedFileAsync(file)->fileAsync = 0;
            opened = 0;
        } else {
            opened = 1;
        }
    }
    if (opened == 0) {
        if (file != 0) {
            ((SelfV*)file)->del(1);
        }
        return 0;
    }
    return file;
}
#pragma rtti reset

union Slot8023F380 { char c; int i; };

struct VFile8023F380 {
    virtual void m0();
    virtual void m1();
    virtual int m2(unsigned int);
};

// Resolve the DVD path, allocate an aligned buffer, read synchronously, and close the handle.
// path must resolve to a NUL-terminated path. Return 0 on open/empty-file/allocation/read
// failure, otherwise 1. The stored size is logical bytes; the read request includes padding.
int CSharedFileSync_LoadFile(void* self, void* path) {
    int readResult;
    int handle;
    CSharedFile_SetPath(self, path);
    unsigned int len;
    char* resolvedPath = (char*)CHashPath_FindPath(path);
    readResult = 0;
    int bytesRead;
    String_80049268 local;
    local.tag = 0;
    local.capacity = 0;
    local.data = 0;
    len = strlen(resolvedPath);
    String_ReserveCapacity(&local, len);
    // uninitSlot/flagSlot and the volatile z1/z2 below are dead values kept only so the original stack frame and
    // register use are reproduced; they carry no information. The string 'local' is the DVD-normalized path.
    volatile Slot8023F380 uninitSlot;
    Slot8023F380 flagSlot;
    flagSlot.c = uninitSlot.c;
    String_ReplaceRange(&local, 0, 0, resolvedPath, resolvedPath + len, &flagSlot.c);
    FilePath_EnsureDvdPrefix(&local);
    volatile int z1;
    volatile int z2;
    z2 = 0;
    z1 = 0;
    char* strPtr;
    if ((*(volatile unsigned int*)&local.tag >> 31) == 0) {
        strPtr = (char*)&local + 1;
    } else {
        strPtr = local.data;
    }
    // Phase 2: open the normalized path and request the aligned read allocation.
    handle = FileIO_OpenFile(strPtr, kFileReadAccess, 0, 0, kOpenExistingFile, kFileSequentialAccess, 0, 0);
    // handle + 0x10000 == 0xFFFF is the unsigned form of 'handle == -1': FileIO_OpenFile failed.
    if ((unsigned int)handle + 0x10000u == 0xFFFFu) {
        return 0;
    }
    int fileSize = FileIO_GetFileSize(handle, 0, 0, 0);
    AsCSharedFileSync(self)->fileSize = fileSize;
    if (fileSize == 0) {
        readResult = 0;
    } else {
        readResult = ((unsigned int)fileSize + kDvdAlignmentMask) & ~(unsigned)kDvdAlignmentMask;
        int allocated = ((VFile8023F380*)self)->m2(readResult);
        if (allocated == 0) {
            readResult = 0;
        } else {
            void* readBuffer = AsCSharedFileSync(self)->buffer;
            int readStatus = FileIO_ReadFile(handle, readBuffer, readResult, &bytesRead, 0, -1, 0, 0);
            readResult = (readStatus != 0);
        }
    }
    // Phase 3: wait out the drive's -1 status before closing, including failed reads.
    do {
    } while (DVDGetDriveStatus() == -1);
    FileIO_CloseFile(handle);
    return readResult;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046229C", _ctors_u8023E86C_0, SharedFile_InitializeAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046229C", _ctors_u8023E86C_1, SharedFileSync_InitializeAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046229C", _ctors_u8023E86C_2, SharedFileBuffer_InitializeAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046229C", _ctors_u8023E86C_3, ConteResource_InitializeAngleConstant);
#pragma pop
