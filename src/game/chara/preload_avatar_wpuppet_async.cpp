// Asynchronous preloading of puppets and avatars: the puppet loader, the avatar loader that waits
// on its parts and pet puppet, and the path strings and vectors they use.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include <decomp/ct_member.h>
#include "iff/IffStruct.h"
// Resource suffixes and exception strings are entries in the existing packed pool.
enum PreloadStringOffset {
    kPetExtension = 0,
    kVectorLengthError = 6,
    kLengthErrorType = 0x1A,
    kStoryExtension = 0x51,
    kMenuExtension = 0x58,
    kSwingExtension = 0x5E,
    kCommendationExtension = 0x65,
    kAnimationPetExtension = 0x73
};
static const unsigned kMaxChildLoaderCount = 0x3FFFFFFFu;
static const unsigned kChildGrowthThreeFifthsLimit = 0x15555555u;
static const unsigned kChildGrowthHalfLimit = 0x2AAAAAAAu;

enum PuppetLoadState {
    kWaitingForFile,
    kWaitingForResources,
    kResourcesReady
};
enum AvatarAnimationKind {
    kMenuAnimation = 0,
    kSwingAnimation = 2,
    kStoryAnimation = 3,
    kCommendationAnimation = 4
};
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char petPathStrings[];
}
}

#pragma use_lmw_stmw on

#include "sdk_protos.h"
#include "classes/CPreloadAvatarAsync.h"
#include "classes/CPreloadWPuppetAsync.h"
#include "classes/CPreloadWPuppet.h"
#include "classes/CPreloadSeqAsync.h"
#include "classes/CPreloadSprAsync.h"
#include "classes/CPreloadFileAsync.h"
#include "classes/CPreloadPartsAsync.h"
#include "classes/CPreloadTextureAsync.h"
#include "classes/CPreloadParent.h"

#define AsCPreloadAvatarAsync(p) ((CPreloadAvatarAsync*)(p))

struct C714Field {
    virtual void v0();
    virtual void update();
    // Second declared slot: per-file update, not slot 3.
};

// Select the larger value while preserving its reference identity.
template <class T> inline const T& cmax(const T& a, const T& b) {
    return a < b ? b : a;
}

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
void* __dt__15CPreloadWPuppetFv(void*, int);
void* __dt__14CPreloadParentFv(void*, int);
void OwnedObjectList_UpdateAll(void*);
int OwnedObjectList_AreAllComplete(void*);
int PreloadObjectList_DestroyAll(void*);
int OwnedObjectList_DestroyAll(void*);
void* SharedDoc_GetItemInventory();
void* ItemInventory_FindCharacterData(void*, void*);
void PreloadAvatar_ReplacePartsLoader(void*, void*, void*);
void __dl__FPv(void*);
void __dla__FPv(void*);
void* String_ReserveCapacity(void*, unsigned int);
void* String_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, const char*);
void* __nw__FUl(unsigned long);
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
int CPreloadWPuppetAsync::UpdateLoadState() {
    C714Field* sourceFile = (*(C714Field**)&this->sourceFile);
    if (sourceFile != 0) {
        sourceFile->update();
    }
    OwnedObjectList_UpdateAll(this);
    int state = loadState;
    switch (state) {
    case kWaitingForFile:
        if (((CPreloadWPuppet*)this)->PollFileReady()) {
            if (!((CPreloadWPuppet*)this)->LoadFileResources())
                return 0;
            loadState = kWaitingForResources;
        }
        break;
    case kWaitingForResources:
        if (OwnedObjectList_AreAllComplete(this)) {
            loadState = kResourcesReady;
        }
        break;
    }
    return 1;
}

// animationKind uses AvatarAnimationKind and any nonzero storyOverride forces the
// story animation. selection is an item selection borrowed while its resources are
// queued; partsContext is a numeric switch for the parts loader.
// Allocate and initialize an avatar loader, destroying it if virtual initialization fails.
CPreloadAvatarAsync* CPreloadAvatarAsync::Create(void* selection, int animationKind, int storyOverride,
                                                 int partsContext) {
    // The loader's vtable, in slot order.
    struct Obj {
        virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
        virtual int UpdateAndLatchComplete(void*) DECOMP_LOCAL_VIRTUAL;
        virtual int IsLoadComplete() DECOMP_LOCAL_VIRTUAL;
        virtual int ClearAndRestart(void*, void*, void*, void*) DECOMP_LOCAL_VIRTUAL;
        virtual int InitializeDependencies(void*, void*, void*, void*) DECOMP_LOCAL_VIRTUAL;
    };
    CPreloadAvatarAsync* loader = (CPreloadAvatarAsync*)__nw__FUl(sizeof(CPreloadAvatarAsync));
    if (loader) {
        ((CPreloadParent*)loader)->__ct();
        *(void**)loader = __vt__19CPreloadAvatarAsync;
        loader->loadComplete = 1;
        loader->partsPreload = 0;
    }
    if (!loader)
        return 0;
    if (!((Obj*)loader)
             ->InitializeDependencies(selection, (void*)animationKind, (void*)storyOverride, (void*)partsContext)) {
        if (loader)
            ((Obj*)loader)->destroy(1);
        return 0;
    }
    return loader;
}

// Queue character puppet, animation, model, appearance and parts resources for the requested selection.
int CPreloadAvatarAsync::InitializeDependencies(void* selection, void* animationKind, void* storyOverride,
                                                void* partsContext) {
    if (!OwnedObjectList_DestroyAll(this))
        return 0;
    IFF_STRUCT::sChar* characterRecord =
        (IFF_STRUCT::sChar*)ItemInventory_FindCharacterData(SharedDoc_GetItemInventory(), *(void**)selection);
    if (!characterRecord)
        return 0;
    CPreloadAvatarAsync::QueuePetPuppet(this, characterRecord);
    CPreloadAvatarAsync::QueueAnimationPuppet(this, characterRecord, animationKind, storyOverride);
    QueueModelFiles(characterRecord);
    QueueFaceTexture(characterRecord);
    QueueBaseTexture(characterRecord);
    ((CPreloadAvatarAsync*)this)
        ->QueueTextureAndDigitVariant((const char*)characterRecord, ((unsigned char*)selection)[6]);
    PreloadAvatar_ReplacePartsLoader(this, selection, partsContext);
    loadComplete = 0;
    return 1;
}

extern float kFloat_6_2831855;
extern float s_preloadAvatarAnglePeriod;

// Create a generic file loader for a puppet dependency.
void* CPreloadWPuppetAsync::CreateFileAsync(void* path) {
    return CPreloadFileAsync::Create(path);
}
// Create a sprite loader for a puppet dependency.
void* CPreloadWPuppetAsync::CreateSprAsync(void* path) {
    return CPreloadSprAsync::Create(path);
}
// Create a sequence loader for a puppet dependency.
void* CPreloadWPuppetAsync::CreateSeqPreload(void* path) {
    return CPreloadSeqAsync::Create(path);
}

// Create a texture loader for a puppet dependency.
void* CPreloadWPuppetAsync::CreateTextureAsync(void* path) {
    return CPreloadTextureAsync::Create(path);
}

struct PathStr3 {
    unsigned int flag;
    unsigned int length;
    char* ptr;
};

// The high flag bit selects heap storage; a short string stores its length in byte zero.
static inline unsigned int PathLength(PathStr3* p) {
    if ((p->flag >> 31) == 0)
        return *(unsigned char*)p & 0x7f;
    else
        return p->length;
}

struct PsSite {
    char dst;
    char pad0[3];
    char src;
    char pad1[3];
};
// Append a suffix to an owned small/heap path using caller-local allocator tags.
static inline void AppendPreloadPath(PathStr3& path, PsSite& site, unsigned int length, const char* suffixBytes) {
    site.dst = site.src;
    String_ReplaceRange(&path, length, 0, suffixBytes, suffixBytes + strlen(suffixBytes), &site.dst);
}

struct C714Vec {
    void** volatile data;
    unsigned size, capacity;
};
struct C714VectorBuffer {
    void** data;
    volatile unsigned size;
    unsigned capacity;
    unsigned* owner;
    volatile unsigned offset;
    // Guard replacement storage; after commit, this buffer owns the retired pointer array.
    C714VectorBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Release storage only; transferred child-loader slots are cleared before commit.
    ~C714VectorBuffer() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
// Select a growth increment without copying the caller temporary.
inline const unsigned& C714VecMax(const unsigned& a, const unsigned& b) {
    return a < b ? b : a;
}

extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct C714VectorAllocation {
    char* pointer;
    char* allocator;
    // Guard diagnostic text until the shared control block adopts it.
    C714VectorAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Free diagnostic text when shared adoption did not complete.
    ~C714VectorAllocation() {
        if (pointer)
            __dla__FPv(pointer);
    }
};
struct C714VectorControlBase {
    void* table;
    unsigned strong, weak;
    // Initialize the diagnostic control block with one strong and one weak owner.
    C714VectorControlBase() {
        strong = 1;
        weak = 1;
        table = s_sharedCharDeleterVtable;
    }
};
struct C714VectorControl : C714VectorControlBase {
    char* pointer;
    // Adopt the allocated diagnostic character buffer.
    C714VectorControl(char* p) {
        pointer = p;
    }
};
struct C714VectorErrorBase {
    void* table;
    // Leave exception-base initialization to the derived length_error constructor.
    C714VectorErrorBase() {}
    // Destroy the standard exception base without deleting enclosing storage.
    ~C714VectorErrorBase() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
namespace std {
struct length_error : C714VectorErrorBase {
    char* text;
    void* control;
    // Build the shared vector-length diagnostic and install its exception vtable.
    length_error() {
        const char* message = petPathStrings + kVectorLengthError;
        struct Allocator {
            char value;
            char pad[3];
        } allocator;
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
    ~C714OwnedPath() {
        if (flag >> 31)
            appFreeUni__FPv(ptr);
    }
};
struct C714PlainVec {
    void** data;
    unsigned size, capacity;
};
struct PreloadChildrenView {
    void* vtable;
    C714PlainVec children;
};

#pragma opt_propagation off
#pragma opt_common_subs off
// Initialize and throw a caller-aligned length_error object; this path never returns.
static inline void ThrowPreloadVectorLengthError(std::length_error* error) {
    error->__ct();
    exception_runtime::__throw(exception_runtime::petPathStrings + kLengthErrorType, error,
                               (void*)exception_runtime::__dt__Q23std12length_errorFv);
}

// Build the character animation .apet path for menu, swing, story or commendation and queue its loader.
int CPreloadAvatarAsync::QueueAnimationPuppet(void* loader, void* characterRecord, void* animationKind,
                                              void* storyOverride) {
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
        {
            PsSite s2;
            unsigned int length;
            const char* suffix;
            suffix = petPathStrings + kStoryExtension;
            length = PathLength(&path);
            AppendPreloadPath(path, s2, length, suffix);
        }
    } else {
        switch (animation) {
        case kMenuAnimation: {
            PsSite s3;
            unsigned int length;
            const char* suffix;
            suffix = petPathStrings + kMenuExtension;
            length = PathLength(&path);
            AppendPreloadPath(path, s3, length, suffix);
        } break;
        case kSwingAnimation: {
            PsSite s4;
            unsigned int length;
            const char* suffix;
            suffix = petPathStrings + kSwingExtension;
            length = PathLength(&path);
            AppendPreloadPath(path, s4, length, suffix);
        } break;
        case kStoryAnimation: {
            PsSite s5;
            unsigned int length;
            const char* suffix;
            suffix = petPathStrings + kStoryExtension;
            length = PathLength(&path);
            AppendPreloadPath(path, s5, length, suffix);
        } break;
        case kCommendationAnimation: {
            PsSite s6;
            unsigned int length;
            const char* suffix;
            suffix = petPathStrings + kCommendationExtension;
            length = PathLength(&path);
            AppendPreloadPath(path, s6, length, suffix);
        } break;
        }
    }
    {
        PsSite s7;
        unsigned int length;
        const char* suffix;
        suffix = petPathStrings + kAnimationPetExtension;
        length = PathLength(&path);
        AppendPreloadPath(path, s7, length, suffix);
    }
    char* data;
    if ((path.flag >> 31) == 0)
        data = (char*)&path + 1;
    else
        data = path.ptr;
    void* puppetLoader = CPreloadWPuppetAsync::Create(data);
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
        void** oldData;
        unsigned int count;
        if (0x3FFFFFFFu - children->capacity < 1) {
            union {
                unsigned alignment;
                unsigned char bytes[sizeof(std::length_error)];
            } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            ThrowPreloadVectorLengthError(error);
        }
        unsigned needed = children->size + 1 - children->capacity;
        C714VectorBuffer temp(&children->capacity);
        unsigned capacity = children->capacity;
        if (needed > 0x3FFFFFFFu - capacity) {
            union {
                unsigned alignment;
                unsigned char bytes[sizeof(std::length_error)];
            } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            ThrowPreloadVectorLengthError(error);
        }
        if (capacity < 0x15555555u) {
            capacity += C714VecMax((3 * (capacity + 1)) / 5, needed);
        } else if (capacity < 0x2AAAAAAAu) {
            capacity += C714VecMax((capacity + 1) / 2, needed);
        } else
            capacity = 0x3FFFFFFF;
        if (capacity > 0x3FFFFFFFu) {
            union {
                unsigned alignment;
                unsigned char bytes[sizeof(std::length_error)];
            } storage;
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
void PreloadAvatar_InitAnglePeriod() {
    s_preloadAvatarAnglePeriod = kFloat_6_2831855;
}

// Report whether the puppet loader reached the resources-ready state.
int CPreloadWPuppetAsync::IsResourcesReady() {
    int state = loadState;
    return state == kResourcesReady;
}

// Report whether all avatar resources and any parts loader are complete.
int CPreloadAvatarAsync::IsLoadComplete() {
    int loadComplete = this->loadComplete;
    return loadComplete == 1;
}

// Destroy the puppet loader and optionally release its allocation.
CPreloadWPuppetAsync::~CPreloadWPuppetAsync() {
    __dt__15CPreloadWPuppetFv(this, 0);
}

// Destroy the parts loader, then the parent loader, and optionally release the allocation.
CPreloadAvatarAsync::~CPreloadAvatarAsync() {
    void* partsLoader = partsPreload;
    *(void**)this = (void*)__vt__19CPreloadAvatarAsync;
    if (partsLoader != 0) {
        ((void (**)(void*, int)) * (void**)partsLoader)[2](partsLoader, 1);
        partsPreload = 0;
    }
    __dt__14CPreloadParentFv(this, 0);
}

// Poll avatar dependencies and the optional parts loader, marking completion when both are ready.
int CPreloadAvatarAsync::UpdateAndLatchComplete(void* loader) {
    void* self = loader;
    OwnedObjectList_UpdateAll(self);
    void* partsLoader = AsCPreloadAvatarAsync(self)->partsPreload;
    if (partsLoader != 0) {
        ((void (**)(void*)) * (void**)partsLoader)[3](partsLoader);
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
class CPreloadPartsAsyncView {
public:
    virtual void destroy(int);
};
// Replace the parts loader and mark the avatar as awaiting completion.
int CPreloadAvatarAsync::ReloadParts(void* selection, int flags) {
    void* newPartsLoader = CPreloadPartsAsync::Create(selection, (void*)flags);
    int replaced;
    if (newPartsLoader == 0) {
        replaced = 0;
    } else {
        void* oldPartsLoader = partsPreload; // CPreloadAvatarAsync::ReloadParts: +0x14
        if (oldPartsLoader != 0) {
            if (oldPartsLoader != 0) {
                ((CPreloadPartsAsyncView*)oldPartsLoader)->destroy(1);
            }
            partsPreload = 0;
        }
        partsPreload = newPartsLoader;
        replaced = 1;
    }
    if (replaced == 0) {
        return 0;
    }
    loadComplete = 0; // CPreloadAvatarAsync::ReloadParts: +0x10
    return 1;
}

// Clear old dependencies and parts, then dispatch virtual initialization for a new selection.
int CPreloadAvatarAsync::ClearAndRestart(void* selection, void* animationKind, void* storyOverride,
                                         void* partsContext) {
    int cleared;
    if (!PreloadObjectList_DestroyAll(this)) {
        cleared = 0;
    } else {
        CB1CObj* partsLoader = (*(CB1CObj**)&partsPreload);
        if (partsLoader != 0) {
            if (partsLoader != 0) {
                partsLoader->destroy(1);
            }
            partsPreload = 0;
        }
        loadComplete = 1;
        cleared = 1;
    }
    if (!cleared) {
        return 0;
    }
    return ((CB1CObj*)this)->initializeDependencies(selection, animationKind, storyOverride, partsContext);
}

struct C714Site {
    char dst;
    char pad0[3];
    char src;
    char pad1[3];
};

#pragma opt_propagation off
// Append .bpet to the character model name and add the resulting puppet loader to the child vector.
int CPreloadAvatarAsync::QueuePetPuppet(void* loader, void* characterRecord) {
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
    void* puppetLoader = CPreloadWPuppetAsync::Create(data);
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
        void** oldData;
        unsigned int count;
        if (0x3FFFFFFFu - children->capacity < 1) {
            union {
                double alignment;
                unsigned char bytes[sizeof(std::length_error)];
            } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            ThrowPreloadVectorLengthError(error);
        }
        unsigned needed = children->size + 1 - children->capacity;
        C714VectorBuffer temp(&children->capacity);
        unsigned capacity = children->capacity;
        if (needed > 0x3FFFFFFFu - capacity) {
            union {
                double alignment;
                unsigned char bytes[sizeof(std::length_error)];
            } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            ThrowPreloadVectorLengthError(error);
        }
        if (capacity < 0x15555555u) {
            capacity += C714VecMax((3 * (capacity + 1)) / 5, needed);
        } else if (capacity < 0x2AAAAAAAu) {
            capacity += C714VecMax((capacity + 1) / 2, needed);
        } else
            capacity = 0x3FFFFFFF;
        if (capacity > 0x3FFFFFFFu) {
            union {
                double alignment;
                unsigned char bytes[sizeof(std::length_error)];
            } storage;
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
