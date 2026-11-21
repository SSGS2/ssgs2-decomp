#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include <decomp/ct_member.h>
#include "iff/IffStruct.h"
// Resource suffixes and exception strings are entries in the existing packed pool.
enum PreloadStringOffset {
    kPetExtension = 0, kVectorLengthError = 6, kLengthErrorType = 0x1A,
    kStoryExtension = 0x51, kMenuExtension = 0x58, kSwingExtension = 0x5E,
    kCommendationExtension = 0x65, kAnimationPetExtension = 0x73
};
static const unsigned kMaxChildLoaderCount = 0x3FFFFFFFu;
static const unsigned kChildGrowthThreeFifthsLimit = 0x15555555u;
static const unsigned kChildGrowthHalfLimit = 0x2AAAAAAAu;

enum PuppetLoadState { kWaitingForFile, kWaitingForResources, kResourcesReady };
enum AvatarAnimationKind { kMenuAnimation = 0, kSwingAnimation = 2, kStoryAnimation = 3, kCommendationAnimation = 4 };
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char petPathStrings[];
}
}

// Translation unit 0x8026C714-0x8026D98C (17 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#pragma use_lmw_stmw on

#include "sdk_protos.h"
#include "classes/CPreloadAvatarAsync.h"
#include "classes/CPreloadWPuppetAsync.h"

#define AsCPreloadWPuppetAsync(p) ((CPreloadWPuppetAsync*)(p))

#define AsCPreloadAvatarAsync(p) ((CPreloadAvatarAsync*)(p))

struct C714Field {
    virtual void v0();
    virtual void update();
    // Second declared slot: per-file update, not slot 3.
};

// Select the larger value while preserving its reference identity.
template <class T> inline const T& cmax(const T& a, const T& b) { return a < b ? b : a; }

struct CB1CObj {
// The first vtable slot is the deleting destructor.
    virtual void destroy(int); // First declared slot: deleting destruction.
    virtual void d1();
    virtual void d2();
    virtual void d3();
    virtual int initializeDependencies(void*, void*, void*, void*);
    // Fifth declared slot: initialize selection dependencies.
};

extern "C" {
void* CPreloadFileAsync_Create(void*);
void* CPreloadSprAsync_Create(void*);
void* CPreloadSeqAsync_Create(void*);
void* CPreloadTextureAsync_Create(void*);
void* CPreloadWPuppet_dtor(void*, int);
void* CPreloadParent_dtor(void*, int);
void OwnedObjectList_UpdateAll(void*);
int OwnedObjectList_AreAllComplete(void*);
void* CPreloadPartsAsync_Create(void*, int);
int PreloadObjectList_DestroyAll(void*);
void* CPreloadParent_Ctor(void*);
int OwnedObjectList_DestroyAll(void*);
int CPreloadWPuppet_PollFileReady(void*);
int CPreloadWPuppet_LoadFileResources(void*);
void* SharedDoc_GetItemInventory();
void* ItemInventory_FindCharacterData(void*, void*);
int CPreloadAvatarAsync_QueuePetPuppet(void*, void*);
int CPreloadAvatarAsync_QueueAnimationPuppet(void*, void*, void*, void*);
void CPreloadAvatarAsync_QueueModelFiles(void*, void*);
void CPreloadAvatarAsync_QueueFaceTexture(void*, void*);
void CPreloadAvatarAsync_QueueBaseTexture(void*, void*);
void CPreloadAvatarAsync_QueueTextureAndDigitVariant(void*, void*, int);
void PreloadAvatar_ReplacePartsLoader(void*, void*, void*);
void __dl__FPv(void*);
void __dla__FPv(void*);
void* String_ReserveCapacity(void*, unsigned int);
void* String_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, const char*);
void* CPreloadWPuppetAsync_Create(const char*);
void* __nwa__FUl(unsigned int);
void* appMallocDummy__FUl(unsigned int);
void* appFreeUni__FPv(void*);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
extern char petPathStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern void __dt__Q23std12length_errorFv(void*);
extern char __vt__19CPreloadAvatarAsync[];

// Advance file and resource loading, updating the puppet loader state when each stage finishes.
int CPreloadWPuppetAsync_UpdateLoadState(void* self) {
    C714Field* sourceFile = (*(C714Field**)&AsCPreloadWPuppetAsync(self)->sourceFile);
    if (sourceFile != 0) {
        sourceFile->update();
    }
    OwnedObjectList_UpdateAll(self);
    int state = AsCPreloadWPuppetAsync(self)->loadState;
    switch (state) {
    case kWaitingForFile:
        if (CPreloadWPuppet_PollFileReady(self)) {
            if (!CPreloadWPuppet_LoadFileResources(self)) return 0;
            AsCPreloadWPuppetAsync(self)->loadState = kWaitingForResources;
        }
        break;
    case kWaitingForResources:
        if (OwnedObjectList_AreAllComplete(self)) {
            AsCPreloadWPuppetAsync(self)->loadState = kResourcesReady;
        }
        break;
    }
    return 1;
}

struct C868Alloc {
    char data[0x18];
    C868Alloc();
};
// Construct an empty avatar loader with no pending parts loader.
C868Alloc::C868Alloc() {
    CPreloadParent_Ctor(this);
    *(void**)this = __vt__19CPreloadAvatarAsync;
    AsCPreloadAvatarAsync(this)->loadComplete = 1;
    AsCPreloadAvatarAsync(this)->partsPreload = 0; // CPreloadAvatarAsync_Create: +0x14
}

// Selector ABI: animationKind and storyOverride carry integer WORDS encoded as
// void* by existing callers; neither is dereferenced or owned. animationKind uses
// AvatarAnimationKind; any nonzero storyOverride forces the story suffix.
// selection/partsContext are borrowed object pointers used during queue setup.
// Allocate and initialize an avatar loader, destroying it if virtual initialization fails.
void* CPreloadAvatarAsync_Create(void* selection, void* animationKind, void* storyOverride, void* partsContext) {
    struct Obj { 
    virtual void d2(int) DECOMP_LOCAL_VIRTUAL; virtual void e1() DECOMP_LOCAL_VIRTUAL; virtual void e2() DECOMP_LOCAL_VIRTUAL; virtual void e3() DECOMP_LOCAL_VIRTUAL; virtual int init(void*, void*, void*, void*) DECOMP_LOCAL_VIRTUAL; virtual void e5() DECOMP_LOCAL_VIRTUAL; virtual void e6() DECOMP_LOCAL_VIRTUAL;
    };
    Obj* loader = (Obj*)new C868Alloc;
    if (!loader) return 0;
    if (!loader->init(selection,animationKind,storyOverride,partsContext)) { if (loader) loader->d2(1); return 0; }
    return loader;
}

// Queue character puppet, animation, model, appearance and parts resources for the requested selection.
int CPreloadAvatarAsync_InitializeDependencies(void* self, void* selection, void* animationKind, void* storyOverride, void* partsContext) {
    if (!OwnedObjectList_DestroyAll(self)) return 0;
    void* characterRecord = ItemInventory_FindCharacterData(SharedDoc_GetItemInventory(), *(void**)selection);
    if (!characterRecord) return 0;
    CPreloadAvatarAsync_QueuePetPuppet(self,characterRecord);
    CPreloadAvatarAsync_QueueAnimationPuppet(self,characterRecord,animationKind,storyOverride);
    CPreloadAvatarAsync_QueueModelFiles(self,characterRecord);
    CPreloadAvatarAsync_QueueFaceTexture(self,characterRecord);
    CPreloadAvatarAsync_QueueBaseTexture(self,characterRecord);
    CPreloadAvatarAsync_QueueTextureAndDigitVariant(self,characterRecord,((unsigned char*)selection)[6]);
    PreloadAvatar_ReplacePartsLoader(self,selection,partsContext);
    AsCPreloadAvatarAsync(self)->loadComplete=0;
    return 1;
}

extern float kFloat_6_2831855;
extern float s_preloadAvatarAnglePeriod;

// Create a generic file loader for a puppet dependency.
void* CPreloadWPuppetAsync_CreateFileAsync(void*, void* path) { return CPreloadFileAsync_Create(path); }
// Create a sprite loader for a puppet dependency.
void* CPreloadWPuppetAsync_CreateSprAsync(void*, void* path) { return CPreloadSprAsync_Create(path); }
// Create a sequence loader for a puppet dependency.
void* CPreloadWPuppetAsync_CreateSeqPreload(void*, void* path) { return CPreloadSeqAsync_Create(path); }

// Create a texture loader for a puppet dependency.
void* CPreloadWPuppetAsync_CreateTextureAsync(void*, void* path) { return CPreloadTextureAsync_Create(path); }

struct PathStr3 { unsigned int flag; unsigned int length; char* ptr; };

extern void* __nw__FUl(unsigned long);

// The high flag bit selects heap storage; a short string stores its length in byte zero.
static inline unsigned int PathLength(PathStr3* p) {
    if ((p->flag >> 31) == 0) return *(unsigned char*)p & 0x7f;
    else return p->length;
}

struct PsSite { char dst; char pad0[3]; char src; char pad1[3]; };
// Append a suffix to an owned small/heap path using caller-local allocator tags.
static inline void AppendPreloadPath(PathStr3& path, PsSite& site, unsigned int length, const char* suffixBytes) {
    site.dst = site.src;
    String_ReplaceRange(&path, length, 0, suffixBytes, suffixBytes + strlen(suffixBytes), &site.dst);
}

struct C714Vec { void** volatile data; unsigned size, capacity; };
struct C714VectorBuffer {
    void** data; volatile unsigned size; unsigned capacity; unsigned* owner; volatile unsigned offset;
    // Guard replacement storage; after commit, this buffer owns the retired pointer array.
    C714VectorBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Release storage only; transferred child-loader slots are cleared before commit.
    ~C714VectorBuffer() { if (data) { size = 0; appFreeUni__FPv(data); } }
};
// Select a growth increment without copying the caller temporary.
inline const unsigned& C714VecMax(const unsigned& a, const unsigned& b) { return a < b ? b : a; }

extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct C714VectorAllocation {
    char* pointer; char* allocator;
    // Guard diagnostic text until the shared control block adopts it.
    C714VectorAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Free diagnostic text when shared adoption did not complete.
    ~C714VectorAllocation() { if (pointer) __dla__FPv(pointer); }
};
struct C714VectorControlBase {
    void* table; unsigned strong, weak;
    // Initialize the diagnostic control block with one strong and one weak owner.
    C714VectorControlBase() { strong = 1; weak = 1; table = s_sharedCharDeleterVtable; }
};
struct C714VectorControl : C714VectorControlBase {
    char* pointer;
    // Adopt the allocated diagnostic character buffer.
    C714VectorControl(char* p) { pointer = p; }
};
struct C714VectorErrorBase {
    void* table;
    // Leave exception-base initialization to the derived length_error constructor.
    C714VectorErrorBase() {}
    // Destroy the standard exception base without deleting enclosing storage.
    ~C714VectorErrorBase() { __dt__Q23std9exceptionFv(this, -1); }
};
namespace std {
struct length_error : C714VectorErrorBase {
    char* text; void* control;
    // Build the shared vector-length diagnostic and install its exception vtable.
    length_error() {
        const char* message = petPathStrings + kVectorLengthError;
        struct Allocator { char value; char pad[3]; } allocator;
        allocator.value = 0;
        table = __vt__Q23std11logic_error;
        unsigned n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            C714VectorAllocation allocation(p, &allocator.value);
            control = new C714VectorControl(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}

struct C714OwnedPath : PathStr3 {
    // Release a path heap buffer after the child loader has copied the path.
    ~C714OwnedPath() { if (flag >> 31) appFreeUni__FPv(ptr); }
};
struct C714PlainVec { void** data; unsigned size, capacity; };
struct PreloadChildrenView { void* vtable; C714PlainVec children; };

#pragma opt_propagation off
#pragma opt_common_subs off
// Initialize and throw a caller-aligned length_error object; this path never returns.
static inline void ThrowPreloadVectorLengthError(std::length_error* error) {
    error->__ct();
    exception_runtime::__throw(exception_runtime::petPathStrings + kLengthErrorType, error,
        (void*)exception_runtime::__dt__Q23std12length_errorFv);
}

// Build the character animation .apet path for menu, swing, story or commendation and queue its loader.
int CPreloadAvatarAsync_QueueAnimationPuppet(void* loader, void* characterRecord, void* animationKind, void* storyOverride) {
    // Phase 1: build a model-derived owned path; the created loader copies it.
    char* self = (char*)loader;
    IFF_STRUCT::sChar* character = (IFF_STRUCT::sChar*)characterRecord;
    int animation = (int)animationKind;
    int useStoryAnimation = (int)storyOverride;
    PsSite s1;
    C714OwnedPath path;
    path.flag = 0;
    path.length = 0;
    path.ptr = 0;
    unsigned int n = strlen(character->model);
    String_ReserveCapacity(&path, n);
    s1.dst = s1.src;
    String_ReplaceRange(&path, 0, 0, character->model, character->model + n, &s1.dst);
    if (useStoryAnimation) {
        { PsSite s2; unsigned int length; const char* suffix; suffix = petPathStrings + kStoryExtension; length = PathLength(&path); AppendPreloadPath(path, s2, length, suffix); }
    } else {
        switch (animation) {
        case kMenuAnimation: { PsSite s3; unsigned int length; const char* suffix; suffix = petPathStrings + kMenuExtension; length = PathLength(&path); AppendPreloadPath(path, s3, length, suffix); } break;
        case kSwingAnimation: { PsSite s4; unsigned int length; const char* suffix; suffix = petPathStrings + kSwingExtension; length = PathLength(&path); AppendPreloadPath(path, s4, length, suffix); } break;
        case kStoryAnimation: { PsSite s5; unsigned int length; const char* suffix; suffix = petPathStrings + kStoryExtension; length = PathLength(&path); AppendPreloadPath(path, s5, length, suffix); } break;
        case kCommendationAnimation: { PsSite s6; unsigned int length; const char* suffix; suffix = petPathStrings + kCommendationExtension; length = PathLength(&path); AppendPreloadPath(path, s6, length, suffix); } break;
        }
    }
    { PsSite s7; unsigned int length; const char* suffix; suffix = petPathStrings + kAnimationPetExtension; length = PathLength(&path); AppendPreloadPath(path, s7, length, suffix); }
    char* data;
    if ((path.flag >> 31) == 0) data = (char*)&path + 1;
    else data = path.ptr;
    void* puppetLoader = CPreloadWPuppetAsync_Create(data);
    if (puppetLoader == 0) {
        return 0;
    }
    // Phase 2: append the owned child loader; growth transfers slots, then frees the old array.
    C714PlainVec* children = &((PreloadChildrenView*)self)->children;
    unsigned int size = children->size;
    unsigned int capacity = children->capacity;
    if (size < capacity) {
        unsigned int newsize = size + 1;
        ((PreloadChildrenView*)self)->children.size = newsize;
        ((PreloadChildrenView*)self)->children.data[newsize - 1] = puppetLoader;
    } else {
        // Validate growth, stage the new tail, transfer old pointers, then commit metadata.
        void** oldData; unsigned int count;
        if (0x3FFFFFFFu - children->capacity < 1) {
            union { unsigned alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            ThrowPreloadVectorLengthError(error);
        }
        unsigned needed = children->size + 1 - children->capacity;
        C714VectorBuffer temp(&children->capacity);
        unsigned capacity = children->capacity;
        if (needed > 0x3FFFFFFFu - capacity) {
            union { unsigned alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            ThrowPreloadVectorLengthError(error);
        }
        if (capacity < 0x15555555u) {
            capacity += C714VecMax((3 * (capacity + 1)) / 5, needed);
        } else if (capacity < 0x2AAAAAAAu) {
            capacity += C714VecMax((capacity + 1) / 2, needed);
        } else capacity = 0x3FFFFFFF;
        if (capacity > 0x3FFFFFFFu) {
            union { unsigned alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            ThrowPreloadVectorLengthError(error);
        }
        void** data2 = (void**)appMallocDummy__FUl(capacity * 4);
        unsigned int oldSize = children->size;
        temp.offset = oldSize;
        temp.data = data2;
        temp.capacity = capacity;
        (data2 + oldSize)[temp.size++] = puppetLoader;
        unsigned sourceSize = children->size;
        oldData = children->data;
        count = ((char*)(oldData + sourceSize) - (char*)oldData) / 4;
        int offset = oldSize - count;
        temp.offset = offset;
        int bytes = count * 4;
        memcpy(data2 + offset, oldData, bytes);
        memset(oldData, 0, bytes);
        unsigned int newSize = temp.size + count;
        oldData = children->data;
        unsigned oldCapacity = children->capacity;
        data2 = temp.data;
        temp.size = newSize;
        children->size = 0;
        temp.data = oldData;
        children->capacity = temp.capacity;
        temp.capacity = oldCapacity;
        children->data = data2;
        children->size = newSize;
        temp.size = 0;

    }
    return 1;
}

#pragma opt_common_subs reset
#pragma opt_propagation reset
// Initialize this preload unit angle-period constant to two pi.
void PreloadAvatar_InitAnglePeriod() { s_preloadAvatarAnglePeriod = kFloat_6_2831855; }

// Report whether the puppet loader reached the resources-ready state.
int CPreloadWPuppetAsync_IsResourcesReady(void* self) {
    int state = AsCPreloadWPuppetAsync(self)->loadState;
    return state == kResourcesReady;
}

// Report whether all avatar resources and any parts loader are complete.
int CPreloadAvatarAsync_IsLoadComplete(void* self) {
    int loadComplete = AsCPreloadAvatarAsync(self)->loadComplete;
    return loadComplete == 1;
}

// Destroy the puppet loader and optionally release its allocation.
void* CPreloadWPuppetAsync_dtor(void* self, int deleteStorage) {
    if (self != 0) {
        CPreloadWPuppet_dtor(self, 0);
        if (deleteStorage > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Destroy the parts loader, then the parent loader, and optionally release the allocation.
void* CPreloadAvatarAsync_dtor(void* self, int deleteStorage) {
    if (self != 0) {
        void* partsLoader = AsCPreloadAvatarAsync(self)->partsPreload;
        *(void**)self = (void*)__vt__19CPreloadAvatarAsync;
        if (partsLoader != 0) {
            ((void (**)(void*, int))*(void**)partsLoader)[2](partsLoader, 1);
            AsCPreloadAvatarAsync(self)->partsPreload = 0;
        }
        CPreloadParent_dtor(self, 0);
        if (deleteStorage > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Poll avatar dependencies and the optional parts loader, marking completion when both are ready.
int CPreloadAvatarAsync_UpdateAndLatchComplete(void* loader) {
    void* self = loader;
    OwnedObjectList_UpdateAll(self);
    void* partsLoader = AsCPreloadAvatarAsync(self)->partsPreload;
    if (partsLoader != 0) {
        ((void (**)(void*))*(void**)partsLoader)[3](partsLoader);
    }
    if (AsCPreloadAvatarAsync(self)->loadComplete == 0 && OwnedObjectList_AreAllComplete(self)) {
        void* pendingPartsLoader = AsCPreloadAvatarAsync(self)->partsPreload;
        if (pendingPartsLoader == 0 || ((int (***)(void*))pendingPartsLoader)[0][4](pendingPartsLoader) != 0) {
            AsCPreloadAvatarAsync(self)->loadComplete = 1;
        }
    }
    return 1;
}

// One-slot destruction view; destroy denotes the vtable +8 entry after its ABI header.
class Z3 { public: 
virtual void destroy(int);
};
// Replace the parts loader and mark the avatar as awaiting completion.
int CPreloadAvatarAsync_ReloadParts(void* self, void* selection, int flags) {
    void* newPartsLoader = CPreloadPartsAsync_Create(selection, flags);
    int replaced;
    if (newPartsLoader == 0) {
        replaced = 0;
    } else {
        void* oldPartsLoader = AsCPreloadAvatarAsync(self)->partsPreload; // CPreloadAvatarAsync_ReloadParts: +0x14
        if (oldPartsLoader != 0) {
            if (oldPartsLoader != 0) {
                ((Z3*)oldPartsLoader)->destroy(1);
            }
            AsCPreloadAvatarAsync(self)->partsPreload = 0;
        }
        AsCPreloadAvatarAsync(self)->partsPreload = newPartsLoader;
        replaced = 1;
    }
    if (replaced == 0) {
        return 0;
    }
    AsCPreloadAvatarAsync(self)->loadComplete = 0; // CPreloadAvatarAsync_ReloadParts: +0x10
    return 1;
}

// Clear old dependencies and parts, then dispatch virtual initialization for a new selection.
int CPreloadAvatarAsync_ClearAndRestart(void* self, void* selection, void* animationKind, void* storyOverride, void* partsContext) {
    int cleared;
    if (!PreloadObjectList_DestroyAll(self)) {
        cleared = 0;
    } else {
        CB1CObj* partsLoader = (*(CB1CObj**)&AsCPreloadAvatarAsync(self)->partsPreload);
        if (partsLoader != 0) {
            if (partsLoader != 0) {
                partsLoader->destroy(1);
            }
            AsCPreloadAvatarAsync(self)->partsPreload = 0;
        }
        AsCPreloadAvatarAsync(self)->loadComplete = 1;
        cleared = 1;
    }
    if (!cleared) {
        return 0;
    }
    return ((CB1CObj*)self)->initializeDependencies(selection, animationKind, storyOverride, partsContext);
}


struct C714Site { char dst; char pad0[3]; char src; char pad1[3]; };

#pragma opt_propagation off
// Append .bpet to the character model name and add the resulting puppet loader to the child vector.
int CPreloadAvatarAsync_QueuePetPuppet(void* loader, void* characterRecord) {
    unsigned int len;
    const char* suffix;
    // Phase 1: build a model-derived owned path; the created loader copies it.
    char* self = (char*)loader;
    IFF_STRUCT::sChar* character = (IFF_STRUCT::sChar*)characterRecord;
    C714Site s1;
    C714Site s2;
    C714OwnedPath path;
    path.flag = 0;
    path.length = 0;
    path.ptr = 0;
    unsigned int n = strlen(character->model);
    String_ReserveCapacity(&path, n);
    s1.dst = s1.src;
    String_ReplaceRange(&path, 0, 0, character->model, character->model + n, &s1.dst);
    suffix = petPathStrings;
    if ((path.flag >> 31) == 0) {
        len = *(unsigned char*)&path & 0x7f;
    } else {
        len = path.length;
    }
    s2.dst = s2.src;
    String_ReplaceRange(&path, len, 0, suffix, suffix + strlen(suffix), &s2.dst);
    char* data;
    if ((path.flag >> 31) == 0) {
        data = (char*)&path + 1;
    } else {
        data = path.ptr;
    }
    void* puppetLoader = CPreloadWPuppetAsync_Create(data);
    if (puppetLoader == 0) {
        return 0;
    }
    // Phase 2: append the owned child loader; growth transfers slots, then frees the old array.
    C714PlainVec* children = &((PreloadChildrenView*)self)->children;
    unsigned int size = children->size;
    if (size < children->capacity) {
        void** dst;
        unsigned int next;
        next = 1 + size;
        dst = children->data;
        *(volatile unsigned*)&children->size = next;
        dst[next - 1] = puppetLoader;
    } else {
        // Validate growth, stage the new tail, transfer old pointers, then commit metadata.
        void** oldData; unsigned int count;
        if (0x3FFFFFFFu - children->capacity < 1) {
            union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            ThrowPreloadVectorLengthError(error);
        }
        unsigned needed = children->size + 1 - children->capacity;
        C714VectorBuffer temp(&children->capacity);
        unsigned capacity = children->capacity;
        if (needed > 0x3FFFFFFFu - capacity) {
            union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            ThrowPreloadVectorLengthError(error);
        }
        if (capacity < 0x15555555u) {
            capacity += C714VecMax((3 * (capacity + 1)) / 5, needed);
        } else if (capacity < 0x2AAAAAAAu) {
            capacity += C714VecMax((capacity + 1) / 2, needed);
        } else capacity = 0x3FFFFFFF;
        if (capacity > 0x3FFFFFFFu) {
            union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            ThrowPreloadVectorLengthError(error);
        }
        void** data2 = (void**)appMallocDummy__FUl(capacity * 4);
        unsigned int oldSize = children->size;
        temp.offset = oldSize;
        temp.data = data2;
        temp.capacity = capacity;
        (data2 + oldSize)[temp.size++] = puppetLoader;
        unsigned sourceSize = children->size;
        oldData = children->data;
        count = ((char*)(oldData + sourceSize) - (char*)oldData) / 4;
        int offset = oldSize - count;
        temp.offset = offset;
        int bytes = count * 4;
        memcpy(data2 + offset, oldData, bytes);
        memset(oldData, 0, bytes);
        unsigned int newSize = temp.size + count;
        oldData = children->data;
        unsigned oldCapacity = children->capacity;
        data2 = temp.data;
        temp.size = newSize;
        children->size = 0;
        temp.data = oldData;
        children->capacity = temp.capacity;
        temp.capacity = oldCapacity;
        children->data = data2;
        children->size = newSize;
        temp.size = 0;

    }
    return 1;
}
#pragma opt_propagation reset
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046234C", _ctors_u8026C714_0, PreloadAvatar_InitAnglePeriod);
#pragma pop
