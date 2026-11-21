#include <decomp/static_init.h>
// Translation unit 0x8007F694-0x80080CF4 (25 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"
#define GAME_WSTRING_IMPLEMENTATION
#include "game/wstring.h"

#include "game/string.h"
#include "classes/CShop2Main.h"
#include <decomp/cw_bitfields.h>

namespace std {
// Release only the heap-backed form of this wide string.
basic_string<wchar_t, char_traits<wchar_t>, app::app_allocator<wchar_t> >::~basic_string() {
    if (flags >> 31) appFreeUni__FPv(data);
}
}

extern "C" {

static const unsigned int kSharedTextureInitialCapacity = 0x400;
static const unsigned int kSharedTextureInitialBytes = 0x1000;
static const unsigned int kResourceReadAlignment = 0x20;
enum SharedTextureGrowthLimits {
    kSharedTextureMaxPointers = 0x3FFFFFFF,
    kSharedTextureThreeFifthsLimit = 0x15555555, kSharedTextureHalfLimit = 0x2AAAAAAA
};

extern float kFloat_6_2831855;
extern float s_twoPi_8007F694_805E27D0;
extern float s_twoPi_8007F694;
extern float s_twoPi_8007F694_805E27E0;
extern float s_twoPi_8007F694_805E27E8;
extern char __vt__10CShop2Main[];
extern char __vt__15ISharedResource[];

void __ct__18NativeBase80201D18Fv(void*);
void __ct__13WInstanceLockFv(void*);
void __dt__13WInstanceLockFv(void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void CubeResourceStorage_dtor(void*, int);
void gtl_Graphics_FreeWithCallback(void*);
void gtl_Image_GetPlatform7Description(void*, void*, void*);
void Texture_LoadResourceWithOptionalPalette(void*, void*, void*, void*, void*, void*);
void String_ReplaceRange(void*, int, unsigned int, const char*, const char*, const char*);
void* CHashPath_FindPath(const char*);
void CFileAsync_ctorWithPath(void*, void*, int);
void* __nw__FUl(unsigned int);
void TableLock_ctor(void*, int, int);
void String_ReserveCapacity(void*, unsigned int);
void* appMallocDummy__FUl(unsigned int);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
void __dt__9TableLockFv(void*, int);
struct SharedResourceReadView;
// Poll file readiness, start an aligned read, then convert the completed buffer into a resource.
int Get__15ISharedResourceFv(SharedResourceReadView*);
struct SharedResourceDestructionView;
// Release pending I/O, loaded resources, registry membership, and the owned resource name.
void* ISharedResource_dtor(SharedResourceDestructionView*, int);
int CFileAsync_PollRead(void*, void*, int, int);
int CFileAsync_GetSize(void*);
void* appMallocUni__FUl(unsigned int);
void CFileAsync_StartRead(void*, void*, int);
void TableLock_ctor(void*, int, int);
void* __nwa__FUl(unsigned int);
void* wmemcpy(void*, const void*, unsigned int);
void* wmemmove(void*, const void*, unsigned int);
extern unsigned short kWideStringGrowthTerminator;
extern unsigned short kWideStringInPlaceTerminator_8007F694;
extern char nameEntryAndStringErrorStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12out_of_range[];
extern char __vt__Q23std12length_error[];
void __dla__FPv(void*);
void __dt__Q23std12out_of_rangeFv(void*);
void __dt__Q23std12length_errorFv(void*);
void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
char* MSLString_LowercaseInPlace(void*);
struct fn_8007FCDC_Layout;
// Construct a named resource, attaching its registry and optional asynchronous file source.
void* ISharedResource_ctor(fn_8007FCDC_Layout*, const char*, void*, void*);
extern char s_sharedTexturePointerList[12];
extern char s_sharedTexturePointerListDtorRegistration[];
extern signed char s_sharedTexturePointerListInitialized;
struct fn_8007FF9C_Layout;
// Initialize the shared-texture registry with room for 1024 resource pointers.
void* SharedTexturePointerList_Init(fn_8007FF9C_Layout*);
struct dtor_800800B4_Layout;
// Release the shared-texture registry's pointer-array storage.
void* SharedTexturePointerList_Dtor(dtor_800800B4_Layout*, int);
extern char sharedTextureRegistryExceptionStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern char __vt__22CSharedTextureResource[];

struct fn_8007F6F8_Layout { unsigned int flags; unsigned int size; void* data; };

// ISharedResource's name is narrow; its original destructor inlines this cleanup.
struct ResourceNameString {
    unsigned int flags;
    unsigned int length;
    char* data;
    // Free the narrow resource name only when its high-bit tag selects heap storage.
    ~ResourceNameString() { if (flags >> 31) appFreeUni__FPv(data); }
};

// Select the smaller length while preserving reference-temporary behavior.
template <class T> inline const T& f7Min(const T& a, const T& b) { return a < b ? a : b; }

struct F7Exc {
    void* vtbl;
    char* str;
    void* ref;
};

struct F7ArrGuard {
    void* p;
    char* flag;
    // Guard an array allocation during shared exception-text construction.
    F7ArrGuard(void* q, char* f) : p(q), flag(f) {}
    // Free the array unless construction transferred its ownership.
    ~F7ArrGuard() { if (p) __dla__FPv(p); }
};

// Preserve the exception construction scope: moving this block into an inline
// function changes register allocation in the surrounding wide-string operation.
// The guard owns raw message bytes until e.ref adopts them; __throw receives &e.
#define F7_THROW_EXC(msgOff, typeOff, vtable, dtor) \
    { \
        char eflag = 0; \
        F7Exc e; \
        const char* msg = nameEntryAndStringErrorStrings + (msgOff); \
        F7Exc* ep = &e; \
        e.vtbl = __vt__Q23std11logic_error; \
        unsigned int mlen = strlen(msg) + 1; \
        char* buf = static_cast<char*>(__nwa__FUl(mlen * 2 - mlen)); \
        e.str = buf; \
        { \
            F7ArrGuard guard(buf, &eflag); \
            void** r = static_cast<void**>(__nw__FUl(0x10)); \
            if (r) { \
                reinterpret_cast<int*>(r)[1] = 1; \
                reinterpret_cast<int*>(r)[2] = 1; \
                r[0] = s_sharedCharDeleterVtable; \
                r[3] = buf; \
            } \
            e.ref = r; \
            guard.p = 0; \
        } \
        strcpy(e.str, msg); \
        e.vtbl = vtable; \
        __throw(nameEntryAndStringErrorStrings + (typeOff), ep, dtor); \
    }

struct F7WCap {
    CW_BITS2(
        unsigned int isLong : 1,
        unsigned int cap : 31)
    unsigned int size;
    void* data;
};

struct F7WSize {
    CW_BITS2(
        unsigned char isLong : 1,
        unsigned char size : 7)
};

#pragma push
#pragma use_lmw_stmw on
// Replace a wide-string range with repeated characters, growing owned storage if needed.
// pos/count/n are wchar_t units, not bytes (Gekko wchar_t is 2 bytes).
// Remove min(count,size-pos), insert n copies of value, return self; range/length
// errors throw. Short strings hold five units including their terminator capacity.
fn_8007F6F8_Layout* WString_ReplaceRepeatedChar(fn_8007F6F8_Layout* self, unsigned int pos,
                                  unsigned int count, unsigned int n, wchar_t value) {
    unsigned int isLong = self->flags >> 31;
    unsigned int cap;
    int size;
    wchar_t* data;
    if (!isLong) {
        data = reinterpret_cast<wchar_t*>(reinterpret_cast<char*>(self) + 2);
        cap = 5;
        size = reinterpret_cast<F7WSize*>(self)->size;
    } else {
        cap = self->flags & 0x7fffffff;

        data = static_cast<wchar_t*>(self->data);
        size = self->size;
    }
    if (pos > size) {
        F7_THROW_EXC(0x1c, 0x37, __vt__Q23std12out_of_range, __dt__Q23std12out_of_rangeFv);
    }

    int copy = f7Min((size - pos), count);
    if (n > 0x7ffffffe || (size - copy) > 0x7ffffffe - n) {
        F7_THROW_EXC(0x6e, 0x89, __vt__Q23std12length_error, __dt__Q23std12length_errorFv);
    }
    // Phase 1 complete: validated range/length; now reserve and move the retained suffix.
    unsigned int total = size - copy + n;
    int remaining = size - ((pos + copy));
    if (total >= cap) {
        int newCap = ((7 + cap)) & ~7u;

        while (newCap < (total + 1)) newCap = (2 * newCap + 7) & ~7u;
        wchar_t* fresh = static_cast<wchar_t*>(appMallocDummy__FUl((newCap * 2)));
        if (pos) wmemcpy(fresh, data, pos);
        wmemset((fresh + pos), value, n);
        if (remaining) wmemcpy(pos + fresh + n, pos + data + copy, remaining);
        fresh[total] = kWideStringGrowthTerminator;
        if (isLong) appFreeUni__FPv(data); else self->flags |= 0x80000000;
        self->size = total;
        self->data = fresh;
        reinterpret_cast<F7WCap*>(self)->cap = newCap;
    } else {
        if (remaining) wmemmove(data + pos + n, data + pos + copy, remaining);
        wmemset(pos + data, value, n);
        data[total] = kWideStringInPlaceTerminator_8007F694;
        if (!isLong) reinterpret_cast<F7WSize*>(self)->size = total;
        else self->size = total;
    }
    return self;
}
#pragma pop

struct dtor_800800B4_Layout {
    void* field00;
    int field04;
};

// 0x800800B4: the outer null check already guards the base cleanup.
// Release an owned contiguous buffer and optionally free its wrapper.
void* dtor_800800B4(dtor_800800B4_Layout* self, int deleting) {
    if (self != 0) {
        if (self != 0 && self != 0 && self != 0 && self != 0) {
            if (self->field00 != 0) {
                void* data = self->field00;
                self->field04 -= ((char*)self + self->field04) - (char*)self;
                appFreeUni__FPv(data);
            }
        }
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// 0x80080C18: the outer null check already guards the base cleanup.
// Release the shared-texture registry's pointer array, then optionally delete it.
void* SharedTexturePointerList_Dtor(dtor_800800B4_Layout* self, int deleting) {
    if (self != 0) {
        if (self != 0 && self != 0 && self != 0 && self != 0 && self != 0) {
            if (self->field00 != 0) {
                void* data = self->field00;
                self->field04 -= ((char*)self + self->field04) - (char*)self;
                appFreeUni__FPv(data);
            }
        }
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

struct fn_8007FA94_Layout {
    char pad_00[0x28];
    int count;
    char pad_02C[4];
    char mutex[0x18];
};

// Increment the resource reference count under its instance mutex.
int ISharedResource_AddRef(fn_8007FA94_Layout* self) {
    void* lock = self->mutex;
    OSLockMutex(reinterpret_cast<OSMutex*>(lock));
    int result = self->count + 1;
    self->count = result;
    OSUnlockMutex(reinterpret_cast<OSMutex*>(lock));
    return result;
}

struct WInstanceLock {
    void* target;
    // Preserve the default lock wrapper's receiver; it performs no acquisition here.
    WInstanceLock() : target(this) {}
    // Acquire the borrowed mutex and remember its address for scope exit.
    WInstanceLock(void* p) : target(p) { OSLockMutex(reinterpret_cast<OSMutex*>(p)); }
    // Release exactly the mutex address captured by this wrapper.
    ~WInstanceLock() { OSUnlockMutex(reinterpret_cast<OSMutex*>(target)); }
};

struct fn_800802C8_Layout {
    char pad_00[0x30];
    char mutex[0x18];
};

// Poll under the instance mutex; return the cached converted-resource address as an ABI word, or 0.
int ISharedResource_GetLocked(fn_800802C8_Layout* self) {
    WInstanceLock lock(self->mutex);
    int result = Get__15ISharedResourceFv(reinterpret_cast<SharedResourceReadView*>(self));
    return result;
}

// Initialize the shop controller's base and both interface vtables.
void* CShop2Main_ctor(void* self) {
    __ct__18NativeBase80201D18Fv(self);
    void* table = __vt__10CShop2Main;
    ((CShop2Main*)self)->shop = 0;
    ((CShop2Main*)self)->vtbl = table;
    ((CShop2Main*)self)->vtbl2 = static_cast<char*>(table) + 0x3C;
    return self;
}

// Initialize the first shared-resource two-pi cache.
void InitTwoPiCache_8007FA70() {
    s_twoPi_8007F694_805E27D0 = kFloat_6_2831855;
}

// Initialize the second shared-resource two-pi cache.
void InitTwoPiCache_8007FA80() {
    s_twoPi_8007F694 = kFloat_6_2831855;
}

// Initialize the texture-list two-pi cache.
void InitTwoPiCache_80080134() {
    s_twoPi_8007F694_805E27E0 = kFloat_6_2831855;
}

// Initialize the texture-registry two-pi cache.
void InitTwoPiCache_80080C9C() {
    s_twoPi_8007F694_805E27E8 = kFloat_6_2831855;
}

// Default task hook with no work.
void CTask_vfunc_16() {}

void CTask_LoadActorsCallback(void*, void*, void*);
// Adjust the secondary-interface receiver before forwarding the actor-load callback.
void CTask_LoadActorsCallback_Thunk4(void* self, void* forwarded1, void* forwarded2) {
    CTask_LoadActorsCallback(static_cast<char*>(self) - 4, forwarded1, forwarded2);
}

struct fn_80080318_Layout {
    char pad_00[0x48];
    char textureDescription[0x1C];
    void* palette;
};

// Return the optional native palette object stored at shared resource +0x64.
void* ISharedResource_GetPaletteObject(fn_80080318_Layout* self) {
    return self->palette;
}

// Return the embedded platform texture description.
void* ISharedResource_GetTextureDescription(void* self) {
    return ((fn_80080318_Layout*)self)->textureDescription;
}

struct SharedResourceNameView {
    char pad_00[0x8];
    unsigned int flags;
    char pad_0C[0x4];
    void* longNameData;
};

#pragma push
#pragma dont_inline on
// Return the resource name from its inline or heap string representation.
void* ISharedResource_GetName(SharedResourceNameView* self) {
    if ((self->flags >> 31) == 0) {
        return reinterpret_cast<char*>(self) + 9;
    }
    return self->longNameData;
}
#pragma pop

struct fn_80080328_Layout {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual int compute(int, int);
    int cached;
};

struct TableLock { char bytes[8]; ~TableLock(); };

struct SharedResourceReadView {
    void** vtable;
    int result; // Cached converter word, not the readiness token written by PollRead.
    char pad_08[0x0c];
    int readStarted;
    char pad_18[4];
    void* source; // Owned CFileAsync handle.
    void* buffer; // Owned aligned staging allocation; freed immediately after conversion.
    unsigned int size; // Aligned bytes to read/convert, not the logical file size.
    char pad_028[8];
    char mutex[0x18];
};

// Shared-resource base layout used by the separate constructor/poll/destructor views:
// +04 cached conversion result (a graphics-resource address on successful conversion),
// +08..+13 owned tagged narrow name, +14 read-started flag, +18 unresolved,
// +1c owned async source, +20 owned staging buffer, +24 aligned byte size,
// +28 signed instance reference count, +2c borrowed registry, +30 instance mutex.
// The constructor's f1c/f20/f24/f28/f2c aliases refer to those same offsets.
// Partial views (including F7Mutex) are NOT allocation-size definitions: the texture
// factory supplies a full 0x68-byte object; OS mutex operations use the storage at +30.

// Poll source readiness/start one aligned read, then convert and free its staging bytes.
// PollRead writes a readiness token through its second argument; only nonzero
// completion plus a nonnull token advances either phase. That token is not the
// source handle, staging bytes, or cached converted-resource word. No resource
// reference is acquired by this getter. If conversion returns zero after a read,
// readStarted stays set while buffer is cleared; there is no new-read retry here.
// Returns the cached result verbatim, 0 until conversion succeeds (not necessarily bool).
int Get__15ISharedResourceFv(SharedResourceReadView* self) {
    WInstanceLock instanceLock(self->mutex);
    void* readinessToken;
    int convertedResourceWord;
    if (self->result == 0 && self->source != 0) {
        // Phase 1: wait for source metadata, reserve a 32-byte-aligned read size, start I/O once.
        if (self->readStarted == 0) {
            readinessToken = 0;
            if (CFileAsync_PollRead(self->source, &readinessToken, 0, 0) && readinessToken != 0) {
                self->size = (CFileAsync_GetSize(self->source) + kResourceReadAlignment - 1) & ~(kResourceReadAlignment - 1);
                self->buffer = appMallocUni__FUl(self->size);
                CFileAsync_StartRead(self->source, self->buffer, self->size);
                self->readStarted = 1;
            }
        } else {
            // Phase 2: on completed read, invoke converter slot 5, then free staging bytes.
            readinessToken = 0;
            if (CFileAsync_PollRead(self->source, &readinessToken, 0, 0) && readinessToken != 0) {
                self->result = reinterpret_cast<int (*)(void*, void*, int)>(self->vtable[5])
                    (self, self->buffer, self->size);
                appFreeUni__FPv(self->buffer);
                self->buffer = 0;
            }
        }
    }
    convertedResourceWord = self->result;
    return convertedResourceWord;
}

struct SharedResourceDestructionView {
    void** vtable;
    int result;
    unsigned int flags;
    void* field0C;
    void* longNameData;
    int readStarted;
    char pad_18[4];
    void* source;
    void* buffer;
    unsigned int size;
    unsigned int referenceCount;
    void* table;
};

struct FdfSource {
    char data[0xc];
    virtual ~FdfSource();
};

struct FdfRef {
    CW_BITS2(
        unsigned int count : 16,
        unsigned int flags : 16)
};

struct FdfTable {
    void** items;
    unsigned int count;
};

// Remove one registry pointer and compact the remaining entries.
// offset is a byte offset of an existing entry; caller holds table lock 4.
// Compaction does not release the removed resource (its destructor called us).
inline void FdfErase(FdfTable* table, unsigned int offset) {
    unsigned int count = *reinterpret_cast<volatile unsigned int*>(&table->count);
    void** item = reinterpret_cast<void**>(reinterpret_cast<char*>(table->items) + offset);
    void** next = item + 1;
    memmove(item, next, ((table->items + count) - item - 1) * 4);
    --table->count;
}

// Find and remove a resource from its table while holding the table lock.
inline void FdfRemove(FdfTable* table, void* object) {
    char tableLock[8];
    TableLock_ctor(&tableLock, 4, 1);
    int offset = 0;
    for (unsigned int i = 0; i < table->count; ++i, offset += 4) {
        if (*reinterpret_cast<void**>(reinterpret_cast<char*>(table->items) + offset) == object) {
            FdfErase(table, offset);
            __dt__9TableLockFv(&tableLock, -1);
            return;
        }
    }
    __dt__9TableLockFv(&tableLock, -1);
}

// Destroy async source/buffer, release cached graphics result, unregister and free name.
// deleting>0 also frees self; registry membership itself does not keep the object alive.
void* ISharedResource_dtor(SharedResourceDestructionView* self, int deleting) {
    if (self != 0) {
        {
        self->vtable = reinterpret_cast<void**>(__vt__15ISharedResource);
        WInstanceLock instanceLock(reinterpret_cast<char*>(self) + 0x30);
        // Phase 1 (instance locked): source, staging bytes, loaded-result reference, registry.
        if (self->source != 0) {
            delete static_cast<FdfSource*>(self->source);
            self->source = 0;
        }
        if (self->buffer != 0) { appFreeUni__FPv(self->buffer); self->buffer = 0; }
        if (self->result != 0) {
            FdfRef* ref = reinterpret_cast<FdfRef*>(self->result);
            if (--ref->count == 0) CubeResourceStorage_dtor(ref, 1);
            self->result = 0;
        }
        FdfRemove(static_cast<FdfTable*>(self->table), self);
        }
        // Phase 2 (lock released): name storage and optional outer allocation.
        reinterpret_cast<ResourceNameString*>(&self->flags)->~ResourceNameString();
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

struct fn_80080140_Layout { char pad_00[0x24]; void* resource; char pad_28[0x1C]; void* field44; char textureDescription[0x1C]; void* palette; };
// Optional native palette output is stored at shared resource +0x64.
static const int kSharedTextureObjectOffset = (int)&((fn_80080140_Layout*)0)->palette;
// Load the native texture and optional palette; path/instance are borrowed, and result is the texture handle.
int CSharedTextureResource_CreateTextureFromMemory(fn_80080140_Layout* self, void* path, void* instance) {
    WInstanceLock instanceLock(reinterpret_cast<char*>(self) + 0x30);
    int result;
    int tmp24;
    unsigned int tmp10[5];
    tmp10[4] = 0;
    void* old;
    result = 0;
    gtl_Image_GetPlatform7Description(path, instance, self->textureDescription);
    self->resource = instance;
    old = *reinterpret_cast<void**>(reinterpret_cast<char*>(self) + kSharedTextureObjectOffset);
    if (old != 0) {
        if (old != 0) {
            CubeResourceStorage_dtor(old, 0);
            gtl_Graphics_FreeWithCallback(old);
        }
        *reinterpret_cast<void**>(reinterpret_cast<char*>(self) + kSharedTextureObjectOffset) = 0;
    }
    Texture_LoadResourceWithOptionalPalette(path, instance, &result, reinterpret_cast<char*>(self) + kSharedTextureObjectOffset,
                 &tmp24, &tmp10);
    return result;
}

struct F7Str {
    unsigned int w8;
    unsigned int wc;
    unsigned int w10;
    unsigned int w14;
    // Initialize empty name storage plus the original extra scratch word.
    F7Str() : w8(0), wc(0), w10(0), w14(0) {}
    // Destroy the NString prefix; the fourth word is not a second allocation.
    ~F7Str() { ((NString*)(this))->~NString(); }
};

struct F7Item {
    char data[0x10];
    // Resolve a borrowed path name and construct the resource's async file source.
    F7Item(const char* name) { CFileAsync_ctorWithPath(this, CHashPath_FindPath(name), 0); }
};

// Return caller-provided placement storage without allocation.
inline void* operator new(size_t, void* p) throw() { return p; }

struct F7Mutex {
    char data[0x10];
    // Initialize the mutex at this address in the full resource allocation (see layout note).
    F7Mutex() { OSInitMutex(reinterpret_cast<OSMutex*>(this)); }
};

struct fn_8007FCDC_Layout {
    void* vtable;
    int f4;
    F7Str str;
    int f18;
    void* f1c;
    int f20;
    int f24;
    int f28;
    void* f2c;
    F7Mutex mutex;
    fn_8007FCDC_Layout(const char* name, void* value, void* optional);
    // Use the full derived resource's supplied storage, not sizeof this partial base view.
    static void* operator new(size_t, void* p) throw() { return p; }
};

// Copy the name, borrow registry value, start with one ref; optional!=0 suppresses async source creation.
inline fn_8007FCDC_Layout::fn_8007FCDC_Layout(const char* name, void* value, void* optional)
    : vtable(__vt__15ISharedResource), f4(0), f1c(0), f20(0), f24(0), f2c(0) {
    WInstanceLock lock(&mutex);
    unsigned int oldLength = (str.w8 >> 31)
        ? str.wc
        : *reinterpret_cast<unsigned char*>(&str.w8) & 0x7f;
    char first;
    char sentinel = first;
    String_ReplaceRange(&str, 0, oldLength, name, name + strlen(name), &sentinel);
    if (!optional) {
        f1c = new F7Item(name);
    }
    f2c = value;
    f28 = 1;
}

struct F9Buffer {
    void** begin;
    unsigned int size;
    unsigned int capacity;
    // Initialize empty registry pointer-array storage.
    F9Buffer() : begin(0), size(0), capacity(0) {}
    // Exchange array storage/capacity only; callers move sizes separately.
    void swap(F9Buffer& other) {
        void** p = begin;
        void** q = other.begin;
        *reinterpret_cast<void** volatile*>(&other.begin) = p;
        unsigned int n = capacity;
        capacity = other.capacity;
        other.capacity = n;
        begin = q;
    }
    // Free pointer-array storage without releasing its resource entries.
    ~F9Buffer() {
        if (begin) {
            unsigned int n = size;
            size = n - n;
            appFreeUni__FPv(begin);
        }
    }
};
#pragma push
#pragma use_lmw_stmw on
struct fn_8007FF9C_Layout : F9Buffer {};
// Initialize registry storage under the table lock, reserving 1024 pointer slots.
void* SharedTexturePointerList_Init(fn_8007FF9C_Layout* self) {
    self->begin = 0; self->size = 0; self->capacity = 0;
    TableLock lock;
    TableLock_ctor(&lock, 4, 1);
    if (self->capacity < kSharedTextureInitialCapacity) {
        F9Buffer fresh;
        unsigned int initialSize = fresh.size;
        fresh.begin = static_cast<void**>(appMallocDummy__FUl(kSharedTextureInitialBytes));
        unsigned int count;
        void** first = self->begin;
        void** last = self->begin + self->size;
        fresh.capacity = kSharedTextureInitialCapacity;
        count = last - first;
        memcpy(fresh.begin + initialSize, first, count * 4);
        memset(first, 0, count * 4);
        self->swap(fresh);
        fresh.size += count;
        self->size = 0;
        self->size = fresh.size;
        fresh.size = 0;
    }
    return self;
}
#pragma pop
// Create the resource from memory once while holding the instance mutex.
void CreateOnMemoryResource__22CSharedTextureResourceFPvUl(fn_80080328_Layout* self, int a2, int a3) {
    WInstanceLock instanceLock(reinterpret_cast<char*>(self) + 0x30);
    if (self->cached != 0) {
        return;
    }
    self->cached = self->compute(a2, a3);
}

// Free the optional native palette before destroying the shared-resource base.
void* CSharedTextureResource_dtor(void* object, int deleting) {
    if (object != 0) {
        *reinterpret_cast<void**>(object) = &__vt__22CSharedTextureResource;
        {
            WInstanceLock instanceLock(static_cast<char*>(object) + 0x30);
            void* fieldVal = ((fn_80080318_Layout*)object)->palette;
            if (fieldVal != 0) {
                if (fieldVal != 0) {
                    CubeResourceStorage_dtor(fieldVal, 0);
                    gtl_Graphics_FreeWithCallback(fieldVal);
                }
                ((fn_80080318_Layout*)object)->palette = 0;
            }
        }
        ISharedResource_dtor(reinterpret_cast<SharedResourceDestructionView*>(object), 0);
        if (deleting > 0)
            __dl__FPv(object);
    }
    return object;
}

struct fn_8007FAE8_Layout {
    void** vtable;
    char pad_04[0x24];
    int count;
    char pad_2C[4];
    int words[6];
};

struct F7DestroyInterface {
    virtual void slot0() = 0;
    virtual void slot1() = 0;
    virtual void destroy(int) = 0;
};

// Decrement references using the original mutex snapshot; destroy the resource at zero.
// This copies OSMutex by value and locks the STACK COPY, unlike AddRef/GetLocked.
// It does not establish synchronization with those methods; preserve this existing
// behavior during cleanup. Returns remaining signed count, or 0 after deletion.
int ISharedResource_Release(fn_8007FAE8_Layout* self) {
    OSMutex mutex = *reinterpret_cast<OSMutex*>(reinterpret_cast<char*>(self) + 0x30);
    WInstanceLock instanceLock(&mutex);
    int count = self->count - 1;
    self->count = count;
    if (count == 0) {
        if (self != 0) {
            reinterpret_cast<F7DestroyInterface*>(self)->destroy(1);
        }
        return 0;
    }
    return count;
}

// Non-owning resource pointer vector: items owns only the pointer array. size is
// the live pointer count, capacity is allocated pointer slots. All membership
// changes use table lock 4; no registry insertion acquires a resource reference.
struct SharedTextureRegistryView { void** items; unsigned int size; unsigned int capacity; };

#pragma push
#pragma opt_lifetimes on
struct RegName {
    unsigned int flags;
    unsigned int size;
    char* data;
    // Initialize an empty owned lookup-name copy.
    RegName() : flags(0), size(0), data(0) {}
    // Release only the long form of the lookup name.
    ~RegName() { if (flags >> 31) appFreeUni__FPv(data); }
    // Decode the tagged name's byte length.
    unsigned int length() const {
        if (!(flags >> 31)) return *reinterpret_cast<const unsigned char*>(this) & 0x7f;
        return size;
    }
    // Borrow the tagged name's text without transferring its storage.
    const char* chars() const {
        if (!(flags >> 31)) return reinterpret_cast<const char*>(this) + 1;
        return data;
    }
};

struct RegGrow {
    void** begin;
    unsigned int n;
    unsigned int cap;
    unsigned int* ownerCap;
    unsigned int mem;
    // Start a pending vector growth; old/new allocation ownership swaps at commit.
    RegGrow(unsigned int* o) : begin(0), n(0), cap(0), ownerCap(o), mem(0) {}
    // Free remaining array storage; resources pointed to by it are not released here.
    ~RegGrow() {
        if (begin) {
            unsigned int t = n;
            n = t - t;
            appFreeUni__FPv(begin);
        }
    }
};

struct RegChar {
    char value;
    // Preserve the original uninitialized allocator-marker byte.
    RegChar() {}
    // Copy a normal allocator marker without interpreting it.
    RegChar(const RegChar& other) : value(other.value) {}
    // Preserve the volatile marker read used by exception construction.
    RegChar(const volatile RegChar& other) : value(other.value) {}
};
struct RegAutoArr {
    void* p;
    char* flag;
    // Guard newly allocated exception text until shared ownership is established.
    RegAutoArr(void* q, char* f) : p(q), flag(f) {}
    // Release exception text still owned by the guard.
    ~RegAutoArr() { if (p) __dla__FPv(p); }
};

#pragma pop

static inline SharedTextureRegistryView* fn_800803B8_Registry();
struct RegResource {
    char bytes[0x68];
    // Construct shared-resource base in full texture storage, then initialize its native texture.
    RegResource(RegName& name, void* value) {
        SharedTextureRegistryView* reg = fn_800803B8_Registry();
        const char* chars;
        if (!(name.flags >> 31)) chars = reinterpret_cast<char*>(&name) + 1;
        else chars = name.data;
        ISharedResource_ctor(reinterpret_cast<fn_8007FCDC_Layout*>(this), chars, reg, value);
        *reinterpret_cast<void**>(this) = __vt__22CSharedTextureResource;
        *reinterpret_cast<void**>(bytes + 0x64) = 0;
    }
};

struct RegInterface { 
virtual void acquire() = 0;
};

// Select the shorter string range length by reference.
template <class T> inline const T& regMin(const T& a, const T& b) { return a < b ? a : b; }
// Select the larger vector growth increment by reference.
template <class T> inline const T& regMax(const T& a, const T& b) { return a < b ? b : a; }

#define REG_ERROR_VARS_0 const char* msg = sharedTextureRegistryExceptionStrings; ErrorStorage* ep = &e;
#define REG_ERROR_VARS_1 ErrorStorage* ep = &e; const char* msg = sharedTextureRegistryExceptionStrings;
#define REG_THROW_LENGTH_ERROR(order) \
    { \
        struct ErrorStorage { void* vtbl; char* str; void* ref; } e; \
        e.vtbl = __vt__Q23std11logic_error; \
        REG_ERROR_VARS_##order \
        RegChar eflag; \
        eflag.value = 0; \
        unsigned int mlen = strlen(msg) + 1; \
        char* buf = static_cast<char*>(__nwa__FUl(mlen * 2 - mlen)); \
        e.str = buf; \
        { \
            RegAutoArr guard(buf, &eflag.value); \
            void** r = static_cast<void**>(__nw__FUl(0x10)); \
            if (r) { \
                reinterpret_cast<int*>(r)[1] = 1; \
                reinterpret_cast<int*>(r)[2] = 1; \
                r[0] = s_sharedCharDeleterVtable; \
                r[3] = buf; \
            } \
            e.ref = r; \
            guard.p = 0; \
        } \
        strcpy(e.str, msg); \
        e.vtbl = __vt__Q23std12length_error; \
        __throw(sharedTextureRegistryExceptionStrings + 0x14, ep, __dt__Q23std12length_errorFv); \
    }

#pragma push
#pragma opt_propagation off
#pragma opt_lifetimes off
// Compare a tagged-name subrange with n bytes, breaking equal-prefix ties by length.
inline int regCompare(const RegName& name, unsigned int pos, unsigned int count, const char* other, const unsigned int& n, unsigned int isLong) {
    unsigned int compareLength = n;
    unsigned int length;
    const char* data;
    if (!isLong) {
        data = reinterpret_cast<const char*>(&name) + 1;
        length = *reinterpret_cast<const unsigned char*>(&name) & 0x7f;
    } else {
        data = name.data;
        length = name.size;
    }
    unsigned int limit = regMin(count, length - pos);
    int result = memcmp(data + pos, other, regMin(n, limit));
    if (result == 0) {
        if (limit < compareLength) return -1;
        if (limit == compareLength) return 0;
        return 1;
    }
    return result;
}
#pragma pop

#pragma push
#pragma opt_common_subs off
// Lazily initialize and register destruction of the non-owning shared-texture pointer table.
static inline SharedTextureRegistryView* fn_800803B8_Registry() {
    if (*reinterpret_cast<void* volatile*>(&Sing_805DC630()) == 0) {
        if (s_sharedTexturePointerListInitialized == 0) {
            SharedTexturePointerList_Init(reinterpret_cast<fn_8007FF9C_Layout*>(s_sharedTexturePointerList));
            __register_global_object(s_sharedTexturePointerList, reinterpret_cast<void*>(SharedTexturePointerList_Dtor), s_sharedTexturePointerListDtorRegistration);
            s_sharedTexturePointerListInitialized = 1;
        }
        Sing_805DC630() = s_sharedTexturePointerList;
    }
    return static_cast<SharedTextureRegistryView*>(Sing_805DC630());
}
#pragma pop

} // extern "C"

#pragma push
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
#pragma opt_loop_invariants off
#pragma opt_common_subs off
// Find a case-normalized shared texture by name, or construct and register it.
// Legacy exported signature retains const char*, but writableName MUST point to
// writable NUL-terminated bytes. The name documents a precondition the type cannot:
// lowercase conversion mutates the caller's buffer (and any aliases) in place.
// Lookup owns a separate temporary name; resources own their own copied names.
// Returns an acquired reference on hit, or a new one-ref resource on miss; caller
// must release it. suppressAsyncSource!=0 suppresses async file creation on a miss.
// Table stores borrowed pointers; resource destruction removes its own membership.
// Passing a string literal/read-only buffer violates this existing API contract.
// To preserve caller bytes, supply a writable copy whose lifetime covers this call.
void* CreateSharedTextureResource(const char* writableName, int suppressAsyncSource) {
    unsigned int nameByteCount;
    const char* normalizedName;
    void* acquiredResource;
    TableLock lock;
    TableLock_ctor(&lock, 4, 1);
    if (!writableName) {
        return 0;
    }
    {
    // Phase 1: normalize caller bytes, then construct an owned lookup key.
    normalizedName = MSLString_LowercaseInPlace(const_cast<char*>(writableName));
    RegName name;
    nameByteCount = strlen(normalizedName);
    String_ReserveCapacity(&name, nameByteCount);
    volatile RegChar first;
    RegChar sentinel(first);
    String_ReplaceRange(&name, 0, 0, normalizedName, normalizedName + nameByteCount, &sentinel.value);
    // Phase 2: linear name lookup; a hit acquires before returning.
    unsigned int i = 0;
    int offset = 0;
    for (; i < fn_800803B8_Registry()->size; ++i, offset += 4) {
        char* candidate = static_cast<char*>(ISharedResource_GetName(
            reinterpret_cast<SharedResourceNameView*>(*reinterpret_cast<void**>(reinterpret_cast<char*>(fn_800803B8_Registry()->items) + offset))));
        unsigned int candidateByteCount = strlen(candidate);
        unsigned int isLong = name.flags >> 31;
        unsigned int count;
        count = (!isLong) ? *reinterpret_cast<unsigned char*>(&name) & 0x7f : name.size;
        int cmp = regCompare(name, 0, count, candidate, candidateByteCount, isLong);
        if (!cmp) {
            reinterpret_cast<RegInterface*>(*reinterpret_cast<void**>(reinterpret_cast<char*>(fn_800803B8_Registry()->items) + offset))->acquire();
            acquiredResource = *reinterpret_cast<void**>(reinterpret_cast<char*>(fn_800803B8_Registry()->items) + offset);
            return acquiredResource;
        }
    }
    // Phase 3: construct on miss, then append its pointer (growing storage if needed).
    void* createdResource = new RegResource(name, reinterpret_cast<void*>(suppressAsyncSource));
    SharedTextureRegistryView* reg = fn_800803B8_Registry();
    if (reg->size < reg->capacity) {
        unsigned int n = reg->size + 1;
        reg->size = n;
        reg->items[n - 1] = createdResource;
    } else {
        if (kSharedTextureMaxPointers - reg->capacity < 1) REG_THROW_LENGTH_ERROR(0);
        unsigned int copiedPointerCount;
        void** oldBegin;
        unsigned int moveByteCount;
        RegGrow growthStorage(&reg->capacity);
        unsigned int growth = reg->size + 1 - reg->capacity;
        unsigned int newCapacity = reg->capacity;
        if (growth > kSharedTextureMaxPointers - newCapacity) REG_THROW_LENGTH_ERROR(1);
        if (newCapacity < kSharedTextureThreeFifthsLimit) {
            unsigned int t = (newCapacity + 1) * 3 / 5;
            newCapacity += regMax(t, growth);
        } else if (newCapacity < kSharedTextureHalfLimit) {
            unsigned int t = (newCapacity + 1) / 2;
            newCapacity += regMax(t, growth);
        } else {
            newCapacity = kSharedTextureMaxPointers;
        }
        if (newCapacity > kSharedTextureMaxPointers) REG_THROW_LENGTH_ERROR(1);
        // Reserve -> place new pointer -> move existing pointers -> swap; growthStorage frees old array.
        void** fresh = static_cast<void**>(appMallocDummy__FUl(newCapacity * 4));
        growthStorage.begin = fresh;
        growthStorage.mem = reg->size;
        (growthStorage.begin + growthStorage.mem)[growthStorage.n] = createdResource;
        unsigned int nextN = growthStorage.n + 1;
        oldBegin = reg->items;
        void** end = oldBegin + reg->size;
        growthStorage.cap = newCapacity;
        growthStorage.n = nextN;
        copiedPointerCount = end - oldBegin;
        growthStorage.mem = growthStorage.mem - copiedPointerCount;
        moveByteCount = copiedPointerCount * 4;
        memcpy(growthStorage.begin + growthStorage.mem, oldBegin, moveByteCount);
        memset(oldBegin, 0, moveByteCount);
        reg->size = 0;
        growthStorage.n += copiedPointerCount;
        unsigned int oldCapacity = reg->capacity;
        reg->capacity = growthStorage.cap;
        growthStorage.cap = oldCapacity;
        void** oldStorage = reg->items;
        reg->items = growthStorage.begin;
        growthStorage.begin = oldStorage;
        reg->size = growthStorage.n;
        growthStorage.n = 0;
    }
    acquiredResource = createdResource;
    }
    return acquiredResource;
}
#pragma pop

extern "C" {

// Placement-construct the resource base in supplied full object storage; return self.
void* ISharedResource_ctor(fn_8007FCDC_Layout* self, const char* name, void* value, void* optional) {
    return new (self) fn_8007FCDC_Layout(name, value, optional);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046201C", _ctors_u8007F694_0, InitTwoPiCache_8007FA70);
STATIC_INIT_ENTRY(".CRT$XCU8046201C", _ctors_u8007F694_1, InitTwoPiCache_8007FA80);
STATIC_INIT_ENTRY(".CRT$XCU8046201C", _ctors_u8007F694_2, InitTwoPiCache_80080134);
STATIC_INIT_ENTRY(".CRT$XCU8046201C", _ctors_u8007F694_3, InitTwoPiCache_80080C9C);
#pragma pop
