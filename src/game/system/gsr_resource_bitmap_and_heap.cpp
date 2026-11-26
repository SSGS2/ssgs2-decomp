// Texture resources (CGsrResource) and bitmaps: encoded image bytes, file paths, and shared texture handles.
// Also the heap allocators they use, the exception message owners and the BaseObject bookkeeping.
// Decoding image files happens in the engine's texture loader, not in these classes.

#include <decomp/static_init.h>
// Texture resource ownership: CGsrResource, Bitmap, heap allocation and BaseObject.
// Modding lookup: CGsrResource(const void*, unsigned int) copies an encoded image into owned memory;
// CGsrResource(const char*) stores a path; CGsrResource::LoadSharedTexture acquires a
// separate shared handle and copies its descriptor. File decoding/palette upload
// is in Texture_LoadResourceWithOptionalPalette (engine_render_cache_and_texture_load.cpp), not this wrapper.
// Texture width/height/format are asset/descriptor inputs; no resolution, aspect
// ratio or frame-rate setting is defined here. Do not edit heap sizes or the
// two-pi caches to scale replacement textures. Follow CGsrResource.h and
// BitmapLayout.h for byte offsets; the vtables are not both at object offset 0.
// Preserve release order: secondary handle -> primary handle -> image bytes ->
// path bytes -> optional object storage. Bitmap pixels may instead be borrowed.
// A shared-handle readiness failure can block forever; see the load contract.
#include "classes/BitmapLayout.h"
// View existing bitmap storage without constructing or taking ownership of it.
static inline BitmapLayout* AsBitmapLayout(void* object) {
    return (BitmapLayout*)object;
}
#include "classes/BaseObjectLayout.h"
// View the bookkeeping fields of an existing BaseObject.
static inline BaseObjectLayout* AsBaseObjectLayout(void* object) {
    return (BaseObjectLayout*)object;
}
#include "sdk_protos.h"
#include "game/exception.h"
#include "classes/CGsrResource.h"
#include <decomp/cw_bitfields.h>
#include "classes/GtlNoticeOwner.h"
#include "classes/GtlHeapObject.h"

// CGsrResource owns imageData at +0, two counted handles at +8/+12, and a
// small/heap string at +0x10. Its resourceVtable is a separate word at +0x38.
// The path's top bit selects heap storage; otherwise bytes start at +0x11.
// Bitmap owns its resource argument and owns pixels unless pixelsBorrowed != 0.
// Deleting-destructor ABI throughout this unit: shouldDelete > 0 also frees the
// object; returned object addresses must not be used after that deletion.

// logic_error stores a message pointer and its shared ownership block after the vtable.
struct LogicErrorStorage {
    void* vtable;
    void* message;
    void* messageOwner;
};
enum HeapQueryOptions {
    kMaximumBlockQuery = 5,
    kTickMicrosecondDivisorScale = 125000,
    kHeapDescriptorSizeWord = 5
};

// The double is converted to float and the result is discarded.
static inline void TimestampSink(const double& value) {
    float truncated = (float)value;
    (void)truncated;
}

extern "C" {
extern int kArrayHeap0Descriptor[12];
extern int mem2HeapDescriptorDefaults[12];
extern char s_arrayHeapMutex[];
extern void* MemoryManager_InitHeaps(void*);
extern void AppHeap_QueryMaxAllocatableSize(int, int, int*);
extern void CubeResourceStorage_dtor(void*, int);
extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);
extern size_t strlen(const char*);
extern void* String_ReplaceRange(void*, int, int, const char*, const char*, void*);
extern float kFloat_6_2831855;
extern float s_twoPi_8004576C;
extern float s_resourceRuntimeSecondaryTwoPi;
extern char __vt__6Bitmap[];
extern char s_stdExceptionDefaultMessage[];
extern char __vt__10BaseObject[];
extern char __vt__12CGsrResource[];
extern int s_baseObjectCreationIndex;
extern int s_liveBaseObjectCount;
extern int g_mem2Heap;
extern int g_mem1Heap;
extern void (*g_baseObjectConstructionHook)(void*);
extern void (*g_baseObjectDestructionCallback)(void*);
void __dl__FPv(void*);
size_t strlen(const char*);
unsigned int OSGetTick();
void* appMallocUni__FUl(unsigned int);
void gtl_Image_GetPlatform7Description(void*, unsigned int, void*);
void MWSharedOwner_ReleaseReference(void*);
void* memcpy(void*, const void*, unsigned int);

// Bitmap interface hook with no work for this implementation.
void Bitmap_vfunc_1() {}

struct ResourceRefCount {
    CW_BITS2(unsigned int refs : 16, unsigned int flags : 16)
};

class BitmapResource {
    char pad[0x38];

public:
    virtual ~BitmapResource();
};

#pragma opt_common_subs off
// Release cached texture handles, the copied image data, and any heap path string.
CGsrResource::~CGsrResource() {
    resourceVtable = __vt__12CGsrResource;
    // Drop counted handles first; destroy each only when its 16-bit count reaches zero.
    void* secondaryHandle = secondarySharedResource;
    if (secondaryHandle != 0) {
        if (--((ResourceRefCount*)secondaryHandle)->refs == 0)
            CubeResourceStorage_dtor(secondaryHandle, 1);
        secondarySharedResource = 0;
    }
    void* primaryHandle = primarySharedResource;
    if (primaryHandle != 0) {
        if (--((ResourceRefCount*)primaryHandle)->refs == 0)
            CubeResourceStorage_dtor(primaryHandle, 1);
        primarySharedResource = 0;
    }
    // Release the owned image copy and heap path, then optional object storage.
    void* imageData = this->imageData;
    if (imageData != 0) {
        appFreeUni__FPv(imageData);
        this->imageData = 0;
    }
    if (((char*)&texturePathStorageFlags) != 0 && (((texturePathStorageFlags) >> 31) != 0))
        appFreeUni__FPv(texturePathHeapData);
}
#pragma opt_common_subs reset

#pragma push
#pragma use_lmw_stmw on
// Original cleanup symbol: __dt__9AllocLockFv (__dt__9AllocLockFv).
struct AllocLock_u8004576C {
    void* p;
    int active;
    // Lock the borrowed mutex for this allocation scope, including exception unwind.
    AllocLock_u8004576C(void* mutex) : p(mutex), active(1) {
        OSLockMutex((OSMutex*)p);
    }
    // Unlock only while the guard owns the lock.
    ~AllocLock_u8004576C() {
        if (active)
            OSUnlockMutex((OSMutex*)p);
    }
};
// Lazily create both application heaps, then allocate from MEM2 under the shared mutex.
// Accepted byteCount: 0..INT_MAX bytes; zero returns null AFTER lazy initialization.
// Negative sizes are not rejected: their unchanged 32-bit word reaches the heap
// callback as a size, so they are outside this API's supported domain. No clamping.
// GtlNoticeOwner::AllocateWithOptionalLock (gtl_heap_allocators.cpp) returns zero if its backing
// object is absent, otherwise forwards the allocator result; null is propagated,
// never converted to an exception here. Heap/callback exceptions propagate, with
// AllocLock releasing the outer mutex. A nonnull block is caller-owned; release
// with Memory_FreeFromMem2Heap. First-use heap/mutex publication is NOT locked:
// callers must serialize first use; the mutex protects subsequent allocations.
void* AppHeap_AllocateFromHeap1(int byteCount) {
    if (g_mem2Heap == 0) {
        // Initialize both descriptors to each backing heap's maximum block size.
        int heap0MaxBytes;
        int mem2MaxBytes;
        int initializationResult;
        MemoryManager_InitHeaps(&initializationResult);
        heap0MaxBytes = 0;
        AppHeap_QueryMaxAllocatableSize(0, kMaximumBlockQuery, &heap0MaxBytes);
        struct InitData {
            unsigned int words[12];
        };
        InitData data = *(InitData*)kArrayHeap0Descriptor;
        data.words[kHeapDescriptorSizeWord] = heap0MaxBytes;
        gtl::IHeapObject::Create(&data, &g_mem1Heap);
        mem2MaxBytes = 0;
        AppHeap_QueryMaxAllocatableSize(1, kMaximumBlockQuery, &mem2MaxBytes);
        InitData mem2HeapDescriptor = *(InitData*)mem2HeapDescriptorDefaults;
        mem2HeapDescriptor.words[kHeapDescriptorSizeWord] = mem2MaxBytes;
        gtl::IHeapObject::Create(&mem2HeapDescriptor, &g_mem2Heap);
        OSInitMutex((OSMutex*)s_arrayHeapMutex);
    }
    int result = g_mem2Heap;
    // Allocation uses the now-initialized owner; result changes from owner to address.
    if (byteCount == 0)
        result = 0;
    else {
        AllocLock_u8004576C scope(s_arrayHeapMutex);
        result = ((GtlNoticeOwner*)result)->AllocateWithOptionalLock(byteCount, 0);
    }
    return (void*)result;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Lazily create both application heaps, then allocate from heap zero under the shared mutex.
// Same size/null/exception/first-use contract as AppHeap_AllocateFromHeap1 above;
// the successful block must instead be released with Memory_FreeFromArrayHeap0.
void* AppHeap_AllocateFromHeap0(int byteCount) {
    if (g_mem1Heap == 0) {
        // Keep declaration/order differences from the MEM2 wrapper for code identity.
        int mem2MaxBytes;
        int heap0MaxBytes;
        int initializationResult;
        struct InitData {
            unsigned int words[12];
        };
        InitData mem2HeapDescriptor;
        InitData data;
        MemoryManager_InitHeaps(&initializationResult);
        heap0MaxBytes = 0;
        AppHeap_QueryMaxAllocatableSize(0, kMaximumBlockQuery, &heap0MaxBytes);
        data = *(InitData*)kArrayHeap0Descriptor;
        data.words[kHeapDescriptorSizeWord] = heap0MaxBytes;
        gtl::IHeapObject::Create(&data, &g_mem1Heap);
        mem2MaxBytes = 0;
        AppHeap_QueryMaxAllocatableSize(1, kMaximumBlockQuery, &mem2MaxBytes);
        mem2HeapDescriptor = *(InitData*)mem2HeapDescriptorDefaults;
        mem2HeapDescriptor.words[kHeapDescriptorSizeWord] = mem2MaxBytes;
        gtl::IHeapObject::Create(&mem2HeapDescriptor, &g_mem2Heap);
        OSInitMutex((OSMutex*)s_arrayHeapMutex);
    }
    int result = g_mem1Heap;
    if (byteCount == 0)
        result = 0;
    else {
        AllocLock_u8004576C scope(s_arrayHeapMutex);
        result = ((GtlNoticeOwner*)result)->AllocateWithOptionalLock(byteCount, 0);
    }
    return (void*)result;
}
#pragma pop

void appFreeUni__FPv(void*);
void* CreateSharedTextureResource__FPCci(void*, int);
void* ISharedResource_GetLocked(void*);
void* ISharedResource_GetTextureDescription(void*);

// Return borrowed message text, valid while the exception's message owner remains alive.
void* std_logic_error_GetMessage(void* p) {
    return ((LogicErrorStorage*)p)->message;
}

// Initialize the first writable two-pi constant for this unit.
void ResourceRuntime_InitializeTwoPi() {
    s_twoPi_8004576C = kFloat_6_2831855;
}

// Initialize the second writable two-pi constant for this unit.
void ResourceRuntime_InitializeSecondaryTwoPi() {
    s_resourceRuntimeSecondaryTwoPi = kFloat_6_2831855;
}

// Take ownership of the supplied bitmap resource and start with no pixel allocation.
void Bitmap_ctor(void* p, void* ownedResource) {
    AsBitmapLayout(p)->resource = ownedResource;
    AsBitmapLayout(p)->vtable = (void*)__vt__6Bitmap;
    (*(int*)&AsBitmapLayout(p)->pixels) = 0;
    AsBitmapLayout(p)->unk_10 = 0;
    AsBitmapLayout(p)->pixelsBorrowed = 0;
}

// The base exception has no owned message storage to release.
std::exception::~exception() {}

// Return the static default message, whose storage is never transferred to the caller.
const char* std::exception::what() const {
    return s_stdExceptionDefaultMessage;
}

// Optionally delete the shared-pointer deleter storage.
void* std_tr1_detail_shared_ptr_deleter_common_dtor(void* p, int shouldDelete) {
    if (p != 0 && shouldDelete > 0) {
        __dl__FPv(p);
    }
    return p;
}

// Copy in-memory image data and query its platform texture descriptor.
// this is required fresh 0x3C-byte resource storage; source is borrowed for size
// encoded bytes. Allocation must succeed; no null/decoder failure is handled.
// Owns the new copy until CGsrResource::~CGsrResource (appFreeUni), so callers
// may release source after return. Preserve bytes/length, not just visible size.
// Platform-7 descriptor export writes six words (gtl_texture_vertex_formats.cpp); the seventh word
// in textureInfo is not initialized on this path. Do not assume it is zero.
CGsrResource::CGsrResource(const void* source, unsigned int size) {
    resourceVtable = __vt__12CGsrResource;
    (*(int*)&texturePathStorageFlags) = 0;
    texturePathLength = 0;
    (*(int*)&texturePathHeapData) = 0;
    bufferSize = size;
    imageData = appMallocUni__FUl(size);
    memcpy(imageData, source, bufferSize);
    (*(int*)&primarySharedResource) = 0;
    (*(int*)&secondarySharedResource) = 0;
    gtl_Image_GetPlatform7Description(imageData, bufferSize, textureInfo);
}

// Release the reference-counted exception-message owner during destruction.
static inline void LogicErrorDestroy(void* p) throw() {
    if (&((LogicErrorStorage*)p)->message != 0) {
        if (&((LogicErrorStorage*)p)->message != 0) {
            void* sub = ((LogicErrorStorage*)p)->messageOwner;
            if (sub != 0)
                MWSharedOwner_ReleaseReference(sub);
        }
    }
}

// Release the exception message and optionally delete the out-of-range object.
void* __dt__Q23std12out_of_rangeFv(void* p, short shouldDelete) {
    if (p != 0) {
        if (p != 0)
            LogicErrorDestroy(p);
        if (shouldDelete > 0)
            __dl__FPv(p);
    }
    return p;
}

// Release the exception message and optionally delete the logic-error object.
void* std_logic_error_dtor(void* p, short shouldDelete) {
    if (p != 0) {
        LogicErrorDestroy(p);
        if (shouldDelete > 0)
            __dl__FPv(p);
    }
    return p;
}

// Assign a creation index, update the live-object count, and call the construction hook.
// +8/+C are cleared before the hook observes the object. Their later lifecycle
// roles are unresolved here; BaseObject_dtor neither reads nor frees them.
// Required fresh storage >=0x10 bytes. The hook borrows this BASE-INITIALIZED
// object synchronously; derived members/vtables are not ready. It must not delete
// p, retain it for use before derived construction completes, or reenter its
// construction/destruction. Constructing another object in the hook observes the
// already incremented globals. There is no lock/atomic counter or hook snapshot:
// serialize construction/destruction and hook replacement externally. No writer
// of the hook is recovered in src; the data section initializes it to zero.
// If the hook throws, the index and live count remain incremented and p remains
// base-initialized: no rollback/callback/cleanup occurs here. A hook used for
// bookkeeping must therefore return normally to preserve balanced live counts.
void* BaseObject_ctor(void* p) {
    AsBaseObjectLayout(p)->unk_C = 0;
    AsBaseObjectLayout(p)->vtable = __vt__10BaseObject;
    AsBaseObjectLayout(p)->unk_8 = 0;
    int creationIndex = s_baseObjectCreationIndex;
    AsBaseObjectLayout(p)->creationIndex = creationIndex;
    int nextCreationIndex = creationIndex + 1;
    int liveCount = s_liveBaseObjectCount;
    int nextLiveCount = liveCount + 1;
    s_baseObjectCreationIndex = nextCreationIndex;
    s_liveBaseObjectCount = nextLiveCount;
    if (g_baseObjectConstructionHook != 0) {
        g_baseObjectConstructionHook(p);
    }
    return p;
}

// Sample the hardware tick source and free through the MEM2 heap owner.
void Memory_FreeFromMem2Heap(void* p) {
    void* heapOwner = (void*)g_mem2Heap;
    void* allocation = p;
    unsigned int tick = OSGetTick();
    unsigned int busClockHz = *(volatile unsigned int*)0x800000F8;
    // Bus clock word / 4 is the time-base frequency. 8 * 125000 = 1000000:
    // integer arithmetic approximates microseconds (and retains 32-bit wrap/truncation).
    TimestampSink((double)((tick << 3) / ((busClockHz >> 2) / kTickMicrosecondDivisorScale)));
    ((GtlNoticeOwner*)heapOwner)->InvokeFreeLocked(allocation, 0);
}

// Sample the hardware tick source and free through the array-heap owner.
void Memory_FreeFromArrayHeap0(void* p) {
    void* heapOwner = (void*)g_mem1Heap;
    void* allocation = p;
    unsigned int tick = OSGetTick();
    unsigned int busClockHz = *(volatile unsigned int*)0x800000F8;
    // Ticks are scaled to microseconds as in the MEM2 wrapper; the result is discarded.
    TimestampSink((double)((tick << 3) / ((busClockHz >> 2) / kTickMicrosecondDivisorScale)));
    ((GtlNoticeOwner*)heapOwner)->InvokeFreeLocked(allocation, 0);
}

struct ResourceAllocatorByte {
    unsigned char byte;
};
struct ResourceAllocator {
    ResourceAllocatorByte value;
    char pad[3];
};
struct ResourceStringFlag {
    CW_BITS2(unsigned char allocated : 1, unsigned char length : 7)
};

#pragma opt_common_subs off
// Initialize an empty image resource and copy its texture path into string storage.
// this is required fresh resource storage and texturePath is required borrowed NUL-terminated
// text. String_ReplaceRange copies it; any allocated path is released by the
// destructor. No texture is loaded yet. Retain the volatile writes
// and allocator byte: they reproduce the original empty-string construction.
CGsrResource::CGsrResource(const char* texturePath) {
    (*(volatile int*)&texturePathStorageFlags) = 0;
    *(volatile int*)&imageData = 0;
    (*(volatile int*)&bufferSize) = 0;
    (*(volatile int*)&primarySharedResource) = 0;
    (*(volatile int*)&secondarySharedResource) = 0;
    (*(volatile int*)&texturePathLength) = 0;
    (*(volatile int*)&texturePathHeapData) = 0;
    unsigned char flag = ((volatile ResourceStringFlag*)(((char*)&texturePathStorageFlags)))->length;
    *(void* volatile*)(((char*)&resourceVtable)) = (void*)__vt__12CGsrResource;
    char buf[8];
    // Empty, stateless string allocator token: buf[0] has no initialized payload;
    // its copied byte at +4 supplies the sixth (const Alloc&) ABI argument, NOT
    // a character, size, heap selector or ownership flag. String_ReplaceRange
    // (std_string_and_shared_ptr_deleter.cpp, Alloc/extra) neither reads nor retains that argument. Keep
    // the copy/call shape; initializing/removing it alters the existing
    // generated code even though no string decision consumes the byte.
    buf[4] = buf[0];
    size_t len = strlen(texturePath);
    String_ReplaceRange(((char*)&texturePathStorageFlags), 0, flag, texturePath, texturePath + len, &buf[4]);
}
#pragma opt_common_subs reset

// Free owned pixels and release the bitmap resource before optional deletion.
void* Bitmap_dtor(void* p, int shouldDelete) {
    if (p != 0) {
        unsigned char pixelsBorrowed = AsBitmapLayout(p)->pixelsBorrowed;
        AsBitmapLayout(p)->vtable = (void*)__vt__6Bitmap;
        if (pixelsBorrowed == 0) {
            void* ownedPixels = AsBitmapLayout(p)->pixels;
            if (ownedPixels != 0) {
                if (ownedPixels != 0) {
                    appFreeUni__FPv(ownedPixels);
                    AsBitmapLayout(p)->pixels = 0;
                }
            }
        }
        void* ownedResource = AsBitmapLayout(p)->resource;
        if (ownedResource != 0) {
            delete (BitmapResource*)ownedResource;
            AsBitmapLayout(p)->resource = 0;
        }
        if (shouldDelete > 0)
            __dl__FPv(p);
    }
    return p;
}

// Acquire a shared texture by the stored path, wait, and copy seven descriptor words.
// p is a required initialized resource. Uses no frame delay or delta time;
// changing the main clock cannot change this tight readiness-polling policy.
// Returns null on creation failure, otherwise the acquired shared handle. It is returned
// to the caller, not installed in either member; waiting has no timeout or error exit.
// Call only where loading can complete independently: the polling loop neither
// yields nor checks a failure state. A created handle that never becomes ready
// therefore never returns. The caller assumes the acquired handle's release duty;
// the copied descriptor does not keep a separate reference to description storage.
// Acquisition convention: CreateSharedTextureResource (shared_texture_resource.cpp) adds ONE
// reference on a cache hit; a new resource starts with count 1. Release exactly
// that reference with ISharedResource_Release (virtual release slot), which
// decrements +28 and invokes the deleting destructor at zero. Never delete the
// borrowed locked image/description separately. WGeneric3DR::LoadTextureSlot
// (wgeneric3dr_immediate_renderer.cpp) stores this reference in its slot; replacing that slot does
// not release its old handle, so the slot owner must arrange release separately.
// The factory lowercases path bytes IN PLACE: p's inline/heap path must remain
// writable; acquisition can change its spelling even on an existing-cache hit.
void* CGsrResource::LoadSharedTexture() {
    // Select inline/heap text, then acquire a handle independent of this object's members.
    unsigned int pathOnHeap = texturePathStorageFlags >> 31;
    void* path;
    if (pathOnHeap == 0)
        path = (char*)&texturePathStorageFlags + 1;
    else
        path = texturePathHeapData;
    void* sharedTexture = CreateSharedTextureResource__FPCci(path, 0);
    if (sharedTexture == 0)
        return 0;
    // A non-null locked resource is the readiness condition for the descriptor copy.
    void* readyResource = 0;
    while (readyResource == 0) {
        readyResource = ISharedResource_GetLocked(sharedTexture);
    }
    void* description = ISharedResource_GetTextureDescription(sharedTexture);
    textureInfo[0] = ((int*)description)[0];
    textureInfo[1] = ((int*)description)[1];
    textureInfo[2] = ((int*)description)[2];
    textureInfo[3] = ((int*)description)[3];
    textureInfo[4] = ((int*)description)[4];
    textureInfo[5] = ((int*)description)[5];
    textureInfo[6] = ((int*)description)[6];
    return sharedTexture;
}

// Update the live-object count, call the destruction hook, and optionally delete storage.
// Null p does nothing. Otherwise p must be a live BaseObject, destroyed once;
// derived cleanup has already run. The hook synchronously BORROWS that base-only
// object after vtable restoration and live-count decrement. It must not delete p
// or reenter p's destructor: a second entry decrements again and can double-free.
// It must not retain p beyond this call when shouldDelete>0. Constructing or
// destroying OTHER objects observes the already decremented count. As with the
// construction hook, serialize lifecycle/hook replacement externally (no lock).
// No source writer of this hook was found; the data companion initializes zero.
// A throwing hook propagates: the decrement/vtable write remain, and optional
// __dl__ is skipped. There is no count rollback or retry-safe destruction path;
// bookkeeping hooks must return normally. Return p may denote freed storage.
void* BaseObject_dtor(void* p, int shouldDelete) {
    if (p != 0) {
        AsBaseObjectLayout(p)->vtable = __vt__10BaseObject;
        --s_liveBaseObjectCount;
        if (g_baseObjectDestructionCallback != 0)
            g_baseObjectDestructionCallback(p);
        if (shouldDelete > 0)
            __dl__FPv(p);
    }
    return p;
}
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80461FA4", _ctors_u8004576C_0, ResourceRuntime_InitializeTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU80461FA4", _ctors_u8004576C_1, ResourceRuntime_InitializeSecondaryTwoPi);
#pragma pop

// Constructor-unwind adapter: these reconstructed bases own no completed message.
// Their std::exception base destruction with -1/0 has no field access or side effect.
extern "C" void* __dt__Q23std11logic_errorFv(void* object, int deleting) {
    if (object && deleting > 0)
        __dl__FPv(object);
    return object;
}

// Original exception metadata. External destructor declarations must not leave
// ownership with compiler-generated vtables for the construction-only adapters.
extern "C" {
extern char __RTTI__Q23std9exception[], __RTTI__Q23std11logic_error[];
extern const char s_stdOutOfRangeTypeName[] = "std::out_of_range";
extern const char s_stdLengthErrorTypeName[] = "std::length_error";
DATA_SECTION(".data")
const void* s_stdOutOfRangeBaseTypes[6] = {__RTTI__Q23std9exception, 0, __RTTI__Q23std11logic_error, 0, 0, 0};
DATA_SECTION(".data")
const void* s_stdLengthErrorBaseTypes[6] = {__RTTI__Q23std9exception, 0, __RTTI__Q23std11logic_error, 0, 0, 0};
struct ExceptionRTTI {
    const void* name;
    const void* bases;
};
DATA_SECTION(".sdata") ExceptionRTTI __RTTI__Q23std12out_of_range = {s_stdOutOfRangeTypeName, s_stdOutOfRangeBaseTypes};
DATA_SECTION(".sdata")
ExceptionRTTI __RTTI__Q23std12length_error = {s_stdLengthErrorTypeName, s_stdLengthErrorBaseTypes};
void __dt__Q23std12length_errorFv();
struct ExceptionVTable {
    const void* rtti;
    unsigned int offset;
    const void* destroy;
    const void* what;
};
DATA_SECTION(".data")
ExceptionVTable __vt__Q23std11logic_error = {__RTTI__Q23std11logic_error, 0, (const void*)std_logic_error_dtor,
                                             (const void*)std_logic_error_GetMessage};
DATA_SECTION(".data")
ExceptionVTable __vt__Q23std12length_error = {&__RTTI__Q23std12length_error, 0,
                                              (const void*)__dt__Q23std12length_errorFv,
                                              (const void*)std_logic_error_GetMessage};
DATA_SECTION(".data")
ExceptionVTable __vt__Q23std12out_of_range = {&__RTTI__Q23std12out_of_range, 0,
                                              (const void*)__dt__Q23std12out_of_rangeFv,
                                              (const void*)std_logic_error_GetMessage};
}
