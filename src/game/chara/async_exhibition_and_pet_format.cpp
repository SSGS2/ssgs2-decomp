// Asynchronous exhibition previews: loading, readiness, the loading indicator and camera input.
// Also the pet-format external-file and mesh chunk readers, message drawing objects,
// and the byte-vector helpers that the loaders use.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include <decomp/ct_member.h>
enum PreviewInputAction {
    kPreviewRotateLeft = 2,
    kPreviewRotateRight = 3,
    kPreviewZoomOut = 0,
    kPreviewZoomIn = 1,
    kPreviewPointerDrag = 5
};
enum {
    kLoadingIndicatorGroup = 1000
};

namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char messageDrawFontAndErrorStrings[];
}
}

#include "game/singletons.h"
#include "game/types.h"

#include "sdk_protos.h"
#include "game/exception.h"
#include "classes/CAsyncExhibition.h"
#include "classes/CPangyaMsgDrawObject.h"
#include "classes/CPangyaMsgResourceObject.h"
#include "classes/CPetFormatMesh.h"
#include "classes/CPetFormatExternalFile.h"
#include "classes/CExhibition.h"
#include "classes/CBpCtrl.h"
#include "classes/CMsgDraw.h"

#define AsCPangyaMsgDrawObject(p) ((CPangyaMsgDrawObject*)(p))

#define AsCAsyncExhibition(p) ((CAsyncExhibition*)(p))

extern "C" {
void appFreeUni__FPv(void*);

// Narrow and wide strings share the heap flag and pointer words.
struct StringUnwindCleanup_u80268730 {
    const unsigned int* words;
    bool active;
    // Borrow a temporary string representation and arm its exceptional cleanup.
    explicit StringUnwindCleanup_u80268730(const void* string) : words((const unsigned int*)string), active(true) {}
    // Disarm before normal explicit disposal; never free twice.
    void release() {
        active = false;
    }
    // Free only heap-backed storage while unwinding an armed temporary.
    ~StringUnwindCleanup_u80268730() {
        if (active && (words[0] & 0x80000000u)) {
            appFreeUni__FPv((void*)words[2]);
        }
    }
};

extern float kFloat_6_2831855;
extern float s_petMeshTau;
extern float s_petFormatTau;
extern float s_messageResourceTau;
extern float s_petPreviewTau;
extern char __vt__14CPetFormatMesh[];
extern char __vt__22CPetFormatExternalFile[];
extern void* ItemSelection_CopyValidatedChanges(void*, void*, unsigned int);
// Initialize this pet-mesh unit's angle-conversion constant.
void PetMesh_InitAngleConstant() {
    s_petMeshTau = kFloat_6_2831855;
}

// Initialize this pet-format unit's angle-conversion constant.
void PetFormat_InitAngleConstant() {
    s_petFormatTau = kFloat_6_2831855;
}

// Return the external-file chunk payload starting eight bytes after its header.
void* CPetFormatExternalFile::GetPayload() {
    return (char*)chunkData + 8;
}

// Initialize this message-resource unit's angle-conversion constant.
void MessageResource_InitAngleConstant() {
    s_messageResourceTau = kFloat_6_2831855;
}

// Initialize this asynchronous-preview unit's angle-conversion constant.
void AsyncExhibition_InitAngleConstant() {
    s_petPreviewTau = kFloat_6_2831855;
}

// Bind the mesh chunk storage and install the mesh-format interface.
CPetFormatMesh::CPetFormatMesh(int value) {
    chunkData = value;
    vtbl = (void*)__vt__14CPetFormatMesh;
}

// Bind external-file chunk storage and install its format interface.
CPetFormatExternalFile::CPetFormatExternalFile(int value) {
    chunkData = value;
    vtbl = (void*)__vt__22CPetFormatExternalFile;
}

void CAsyncExhibition::SetLoadingIndicatorPosition(const float* position) {
    loadingIndicatorX = position[0];
    loadingIndicatorY = position[1];
}

// Resolve the external-file chunk's relative data offset from its header.
void* CPetFormatExternalFile::GetChunkData() {
    int* addr = &chunkData;
    void* p = *(void**)addr;
    if ((char*)p + 8 != 0) {
        return (char*)p + *(int*)((char*)p + 8);
    }
    return 0;
}

// Read the first mesh data word when the literal chunk-base-plus-eight guard passes;
// the guard does not test whether the chunk pointer or its stored offset is null.
void* CPetFormatMesh::GetFirstDataWord() {
    int* addr = &chunkData;
    void* p = *(void**)addr;
    void* q;
    if ((char*)p + 8 != 0) {
        q = (char*)p + *(int*)((char*)p + 8);
    } else {
        q = 0;
    }
    if (q == 0) {
        return 0;
    }
    return *(void**)q;
}

// Resolve the mesh chunk's second relative offset against the chunk base.
void* CPetFormatMesh::GetSecondaryData() {
    int* addr = &chunkData;
    void* p = *(void**)addr;
    void* q;
    if ((char*)p + 8 != 0) {
        q = (char*)p + *(int*)((char*)p + 8);
    } else {
        q = 0;
    }
    if (q == 0) {
        return 0;
    }
    return (char*)p + *(int*)((char*)q + 4);
}

// Copy validated item changes into the preview descriptor when an exhibition exists.
void* CAsyncExhibition::CopyValidatedItemChanges(void* other, unsigned int key) {
    if ((*(int*)&exhibition) != 0) {
        return ItemSelection_CopyValidatedChanges(displayDescriptor, other,
                                                  key); // original +0x24
    }
    return 0;
}

extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);

// Release the pet-format wrapper when called as a deleting destructor.
CPetFormatMesh::~CPetFormatMesh() {}

// Release the external-file wrapper when called as a deleting destructor.
CPetFormatExternalFile::~CPetFormatExternalFile() {}

struct Blk73 {
    void* begin;
    void* end;
};
// Clear and release the first byte-buffer storage before optional deletion.
void* dtor_80268730(void* self, int shouldDelete) {
    if (self != 0) {
        Blk73* c = (Blk73*)self;
        c->end = 0;
        if (c != 0) {
            if (c->begin != 0) {
                c->end = 0;
                appFreeUni__FPv(c->begin);
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

extern void CBpCtrl_dtor(void*, int);
extern void* IdMap_Lookup(void*, int);
extern void BpObject_SetEnabled(void*, int);
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_asyncExhibitionBpCtrlDtorRegistration[];
extern int BpObject_PollLoad(void*);
extern void* UIGroup_GetOrCreateById(void*, int);
extern void UIGroup_SetPosition(void*, void*);
extern void BpObject_StartScriptByName(void*, char*, int);
extern void BpObject_TryInvokeCurrentCallback(void*, int);
extern char asyncExhibitionLoadingScriptName[];

struct UpdateTarget {
    // Alias view of CAsyncExhibition: object=exhibition (+0x110),
    // index=loading BP id (+0x11C), initialized=indicator latch (+0x120).
    // This latch does NOT mean the exhibition or preloader is initialized.
    char pad[0x110];
    void* object;
    char pad114[8];
    int index;
    unsigned char initialized;
};

// Test exhibition readiness before optional preloader readiness; null objects satisfy their gate.
// Output references are caller register temporaries; preloader is set only if
// the exhibition gate passes. Virtual slot 2 is used here as a readiness query.
inline int ExhibitionObjectsReady(UpdateTarget* self, void*& exhibition, void*& preloader, int& ready) {
    do {
        exhibition = self->object;
        if (exhibition != 0 && (void*)((CExhibition*)exhibition)->IsReady() == 0) {
            ready = 0;
            break;
        }
        preloader = AsCAsyncExhibition(self)->preloadObject;
        struct VBase {
            virtual int m0() DECOMP_LOCAL_VIRTUAL;
            virtual int m1() DECOMP_LOCAL_VIRTUAL;
            virtual int m2() DECOMP_LOCAL_VIRTUAL;
        };
        if (preloader != 0 && ((VBase*)preloader)->m2() == 0) {
            ready = 0;
            break;
        }
        ready = 1;
    } while (0);
    return ready;
}
// Ensure the shared BP controller exists; callers borrow its singleton storage.
static inline void EnsureAsyncPreviewBpController() {
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_asyncExhibitionBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

// Poll exhibition/preloader readiness and show or hide the animated loading indicator.
// Return 0 only when no exhibition exists, otherwise 1 even while loading.
// delta is forwarded unchanged in the caller's update-time unit.
int CAsyncExhibition::UpdateLoadingStateAndIndicator(float delta) {
    UpdateTarget* self = (UpdateTarget*)this;
    // Phase 1: a ready/absent preloader makes the exhibition visible and clears
    // the indicator latch, BEFORE UpdateLoadState advances the exhibition.
    // The exhibition, optional preloader, and indicator have independent readiness.
    void* exhibition = self->object;
    if (exhibition == 0) {
        return 0;
    }
    void* preloader = AsCAsyncExhibition(self)->preloadObject;
    struct VBase {
        virtual int m0() DECOMP_LOCAL_VIRTUAL;
        virtual int m1() DECOMP_LOCAL_VIRTUAL;
        virtual int m2() DECOMP_LOCAL_VIRTUAL;
    };
    if (preloader == 0 || ((VBase*)preloader)->m2() != 0) {
        ((CExhibition*)exhibition)->SetVisible(1);
        self->initialized = 0;
    }
    ((CExhibition*)exhibition)->UpdateLoadState(delta, 1);
    EnsureAsyncPreviewBpController();
    void* indicator = IdMap_Lookup(Sing_805DC5F0(), self->index);
    // Phase 2: only a loaded BP exhibition may show the indicator. Check exhibition
    // readiness first, then preloader readiness; a loaded pair hides it instead.
    int ready;
    if (BpObject_PollLoad(indicator) != 0 && ExhibitionObjectsReady(self, exhibition, preloader, ready) == 0) {
        // Phase 3: first waiting frame positions group 1000, starts its script,
        // enables the BP exhibition, then latches. Subsequent waiting frames invoke
        // callbacks only. Singleton initialization flags belong to the shared
        // BP controller, separate from this preview's indicator latch.
        if (self->initialized == 0) {
            EnsureAsyncPreviewBpController();
            indicator = IdMap_Lookup(Sing_805DC5F0(), self->index);
            UIGroup_SetPosition(UIGroup_GetOrCreateById(indicator, kLoadingIndicatorGroup),
                                ((char*)&AsCAsyncExhibition(self)->loadingIndicatorX));
            EnsureAsyncPreviewBpController();
            indicator = IdMap_Lookup(Sing_805DC5F0(), self->index);
            BpObject_StartScriptByName(indicator, asyncExhibitionLoadingScriptName, 1);
            EnsureAsyncPreviewBpController();
            indicator = IdMap_Lookup(Sing_805DC5F0(), self->index);
            BpObject_SetEnabled(indicator, 1);
            self->initialized = 1;
        }
        EnsureAsyncPreviewBpController();
        indicator = IdMap_Lookup(Sing_805DC5F0(), self->index);
        BpObject_TryInvokeCurrentCallback(indicator, 0);
    } else {
        // Ready pair OR an unloaded indicator BP: disable the overlay this frame.
        EnsureAsyncPreviewBpController();
        indicator = IdMap_Lookup(Sing_805DC5F0(), self->index);
        BpObject_SetEnabled(indicator, 0);
    }
    return 1;
}

// Clear and release the second byte-buffer storage before optional deletion.
void* dtor_802687A4(void* self, int shouldDelete) {
    if (self != 0) {
        Blk73* c = (Blk73*)self;
        c->end = 0;
        if (c != 0) {
            if (c->begin != 0) {
                c->end = 0;
                appFreeUni__FPv(c->begin);
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Report ready only after both the exhibition and optional preloader are ready.
int CAsyncExhibition::PollReady() {
    int exhibitionObject = (*(int*)&exhibition);
    if (exhibitionObject != 0) {
        int exhibitionReady = (int)(void*)((CExhibition*)exhibitionObject)->IsReady();
        if (exhibitionReady == 0) {
            return 0;
        }
    }
    void* preloader = preloadObject;
    if (preloader != 0) {
        struct VBase {
            virtual int m0() DECOMP_LOCAL_VIRTUAL;
            virtual int m1() DECOMP_LOCAL_VIRTUAL;
            virtual int m2() DECOMP_LOCAL_VIRTUAL;
        };
        if (((VBase*)preloader)->m2() == 0) {
            return 0;
        }
    }
    return 1;
}

// Enable or disable the loading script when its entry ID is valid.
void CAsyncExhibition::SetScriptEnabled(int value) {
    if (bpEntryId != -1) {
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_asyncExhibitionBpCtrlDtorRegistration);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        void* p = IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
        BpObject_SetEnabled(p, value);
    }
}

extern void* __nw__FUl(unsigned int);
extern void ResourceManager_ReleaseCachedObject(void*, void*);
extern void* g_resourceManager;
extern char __vt__20CPangyaMsgDrawObject[];
extern char __vt__24CPangyaMsgResourceObject[];

// Release the message renderer, cached font and heap-backed string before optional deletion.
CPangyaMsgDrawObject::~CPangyaMsgDrawObject() {
    void* p = messageDraw;
    drawVtable = (void*)__vt__20CPangyaMsgDrawObject;
    if (p != 0) {
        if (p != 0) {
            struct VBase {
                virtual void m0(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((VBase*)p)->m0(1);
        }
        messageDraw = 0;
    }
    void* resourceManager = g_resourceManager;
    if (resourceManager != 0) {
        void* cachedObject = textFactory;
        if (cachedObject != 0) {
            ResourceManager_ReleaseCachedObject(resourceManager, cachedObject);
            textFactory = 0;
        }
    }
    {
        struct RC {
            char pad[0x10];
            int count;
        };
        RC* rc = (RC*)this;
        if (&rc->count != 0) {
            if (((unsigned int)rc->count >> 31) != 0) {
                appFreeUni__FPv(stringData);
            }
        }
    }
}

// Allocate message-resource storage and return it only after renderer creation succeeds.
int CPangyaMsgResourceObject::Create(void** out) {
    if (out == 0) {
        return 0;
    }
    void* p = __nw__FUl(0x24);
    if (p != 0) {
        AsCPangyaMsgResourceObject(p)->referenceCount = 1;                             // original +0x0
        AsCPangyaMsgResourceObject(p)->vtbl = (void*)__vt__24CPangyaMsgResourceObject; // original +0x4
        AsCPangyaMsgResourceObject(p)->resourceBytes = 0;                              // original +0x8
        AsCPangyaMsgResourceObject(p)->resourceByteCount = 0;                          // original +0xC
        AsCPangyaMsgResourceObject(p)->resourceByteCapacity = 0;                       // original +0x10
        AsCPangyaMsgResourceObject(p)->secondaryBytes = 0;                             // original +0x14
        AsCPangyaMsgResourceObject(p)->secondaryByteCount = 0;                         // original +0x18
        AsCPangyaMsgResourceObject(p)->secondaryByteCapacity = 0;                      // original +0x1C
        AsCPangyaMsgResourceObject(p)->messageDraw = 0;                                // original +0x20
    }
    if (((CPangyaMsgResourceObject*)p)->CreateMessageDraw(this) != 0) {
        *out = p;
        return 1;
    }
    if (p != 0) {
        if (p != 0) {
            struct VB4 {
                char pad[4];
                virtual void m0(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((VB4*)p)->m0(1);
        }
    }
    return 0;
}

struct VC48 {
    virtual void m0(int);
    virtual void m1();
    virtual void m2();
};

struct C48Target {
    char referenceCountStorage[4];
    void* vtbl;
    void* resourceBytes;
    int resourceByteCount;
    char resourceCapacityStorage[4];
    void* secondaryBytes;
    int secondaryByteCount;
    char secondaryCapacityStorage[4];
    void* messageDraw;
};

#pragma opt_propagation off
// Release the secondary owned byte vector through its expanded destructor layers.
static inline void ReleaseMessageSecondaryBytes(C48Target* self) {
    if (&self->secondaryBytes != 0) {
        if (&self->secondaryBytes != 0) {
            if (&self->secondaryBytes != 0) {
                if (&self->secondaryBytes != 0) {
                    if (&self->secondaryBytes != 0) {
                        void* q = self->secondaryBytes;
                        if (q != 0) {
                            int e = self->secondaryByteCount;
                            int f = self->secondaryByteCount;
                            self->secondaryByteCount = e - f;
                            appFreeUni__FPv(q);
                        }
                    }
                }
            }
        }
    }
}

// Release the primary owned byte vector after the renderer and secondary bytes are gone.
static inline void ReleaseMessagePrimaryBytes(C48Target* self) {
    if (&self->resourceBytes != 0) {
        if (&self->resourceBytes != 0) {
            if (&self->resourceBytes != 0) {
                if (&self->resourceBytes != 0) {
                    if (&self->resourceBytes != 0) {
                        void* q = self->resourceBytes;
                        if (q != 0) {
                            int g = self->resourceByteCount;
                            int h = self->resourceByteCount;
                            self->resourceByteCount = g - h;
                            appFreeUni__FPv(q);
                        }
                    }
                }
            }
        }
    }
}

// Destroy the renderer and free both resource buffers before optional deletion.
CPangyaMsgResourceObject::~CPangyaMsgResourceObject() {
    C48Target* self = (C48Target*)this;
    void* p = self->messageDraw;
    self->vtbl = (void*)__vt__24CPangyaMsgResourceObject;
    if (p != 0) {
        if (p != 0) {
            ((VC48*)p)->m0(1);
        }
        self->messageDraw = 0;
    }
    int a = self->resourceByteCount;
    int b = self->resourceByteCount;
    int c = self->secondaryByteCount;
    int d = self->secondaryByteCount;
    self->resourceByteCount = a - b;
    self->secondaryByteCount = c - d;
    // Release the secondary allocation before the primary allocation. Repeated
    // address guards are the expanded member-destructor layers; the byte pointers own storage.
    ReleaseMessageSecondaryBytes(self);
    ReleaseMessagePrimaryBytes(self);
}
#pragma opt_propagation reset

extern void BpObject_Draw(void*);
extern void* ResourceManager_GetOrLoadDefaultFixedFont(void*, char*);
extern void* WFixedFont_CreateMsgEcbResource(void*, void*, void*, int);
extern void NString_AssignCString(void*, void*);
extern void* Resource_OpenResolvedPath(void*, void*, int, int, int, int);
extern void Resource_Release(void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void* __nwa__FUl(unsigned int);
extern void* memset(void*, int, unsigned int);
extern void* memcpy(void*, const void*, unsigned int);
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern int Resource_ReadFileIntoByteVector(void*, void*, void*);
extern char messageDrawFontAndErrorStrings[];

// Owned byte vector: begin points at size initialized bytes within cap allocated bytes.
struct E68Vec {
    void* begin;
    unsigned int size;
    unsigned int cap;
};

struct E68Guard {
    void** pp;
    // Track a local resource pointer so normal release can null/disarm it.
    E68Guard(void*& p) : pp(&p) {}
    // Release an open resource on exceptional or early exit.
    ~E68Guard() {
        if (*pp != 0) {
            if (*pp != 0) {
                Resource_Release(*pp);
                *pp = 0;
            }
        }
    }
};

struct E68Vt {
    virtual void m0();
    virtual void m1(void*, unsigned int);
    virtual void m2(void*, unsigned int);
};

extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern void* __dt__Q23std12length_errorFv(void*, short);
extern void __throw(const char*, void*, void*);
static void e68_throw(const char* t, void* o, void* d) __attribute__((noreturn));
// Forward a nonreturning exception throw through the SDK declaration used by this unit.
static void e68_throw(const char* t, void* o, void* d) {
    __throw(t, o, d);
    for (;;) {
    }
}

struct E68Rep {
    void* vt;
    unsigned int a;
    unsigned int b;
    void* buf;
};

struct E68Exception : std::exception {};

struct E68ErrBase : E68Exception {
    void* buf;
    void* rep;
    E68ErrBase(const char*);
};

namespace std {
struct length_error : E68ErrBase {
    // Copy the error message into shared storage and install the derived table.
    length_error(const char* msg) : E68ErrBase(msg) {
        *(void**)this = __vt__Q23std12length_error;
    }
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
struct E68Tmp {
    void* buf;
    unsigned char* volatile flag;
    // Release message bytes until ownership reaches the shared control block.
    ~E68Tmp() {
        if (buf != 0)
            __dla__FPv(buf);
    }
};

struct E68Grow : E68Vec {
    unsigned int* pcap;
    unsigned int offset;
    // Free replacement storage on failure, or the old allocation after commit.
    ~E68Grow() {
        if (begin != 0) {
            size = 0;
            appFreeUni__FPv(begin);
        }
    }
};

// Return the larger growth candidate by reference, matching the vector allocator contract.
static inline const unsigned int& e68_max(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

// Copy a NUL-terminated exception message and adopt it into shared ownership.
inline E68ErrBase::E68ErrBase(const char* msg) {
    unsigned char flag[4];
    *(void**)this = __vt__Q23std11logic_error;
    flag[0] = 0;
    unsigned int n = (unsigned int)strlen(msg) + 1;
    char* buffer = (char*)__nwa__FUl(n * 2 - n);
    this->buf = buffer;
    {
        E68Tmp tmp;
        tmp.buf = buffer;
        tmp.flag = flag;
        E68Rep* rep = (E68Rep*)__nw__FUl(0x10);
        if (rep != 0) {
            rep->a = 1;
            rep->b = 1;
            rep->vt = s_sharedCharDeleterVtable;
            rep->buf = buffer;
        }
        this->rep = rep;
        tmp.buf = 0;
    }
    strcpy((char*)this->buf, msg);
}

// Construct and throw the byte-vector length error without changing its unwind storage.
static inline void e68_len_error() {
    char* msg = messageDrawFontAndErrorStrings + 0x12;
    {
        unsigned char storage[sizeof(std::length_error)];
        std::length_error* error = (std::length_error*)storage;
        error->__ct(msg);
        exception_runtime::__throw(exception_runtime::messageDrawFontAndErrorStrings + 0x26, error,
                                   (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}

// Recommend byte-vector capacity with overflow checks and the original piecewise growth policy.
static inline unsigned int e68_capacity(unsigned int& capacity, unsigned int need) {
    unsigned int cap = capacity;
    if (need > 0xFFFFFFFFu - cap)
        e68_len_error();
    if (cap < 0x55555555u) {
        unsigned int candidate = (cap + 1) * 3 / 5;
        return cap + e68_max(candidate, need);
    } else if (cap < 0xAAAAAAAAu) {
        unsigned int candidate = (cap + 1) / 2;
        return cap + e68_max(candidate, need);
    }
    return 0xFFFFFFFFu;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Role aliases retain the existing implementation types and their mangled cleanup symbols.
typedef E68Vec OwnedResourceBytes;
typedef E68Guard OpenResourceGuard;
typedef E68Grow ResourceBytesReplacement;
typedef E68Vt ResourceReadInterface;

// Open a resource, resize the destination byte vector and read the complete file.
// input borrows a resolved path; vecp owns its byte allocation and retains it on
// success. Return 0 for missing input/resource or empty files, 1 after invoking
// the resource read virtual call (its return is not checked). Existing bytes may
// be replaced before that call; this is not a transactional read.
int Resource_ReadFileIntoByteVector(void*, void* input, void* vecp) {
    // Phase 1: open a guarded resource and obtain the required byte length.
    if (input == 0)
        return 0;
    void* resource = Resource_OpenResolvedPath(g_resourceManager, input, -1, 0, 0, 0);
    OpenResourceGuard guard(resource);
    if (resource == 0)
        return 0;
    unsigned int fileByteCount = *(unsigned int*)((char*)resource + 4);
    if (fileByteCount == 0)
        return 0;
    OwnedResourceBytes* bytes = (OwnedResourceBytes*)vecp;
    // Phase 2: resize in-place if capacity suffices; otherwise allocate, copy
    // existing bytes and zero the tail before swapping ownership into bytes.
    if (fileByteCount > bytes->size) {
        // Original 0x80268730 unit: in-place resize skips allocation.
        do {
            unsigned int appendByteCount = fileByteCount - bytes->size;
            if (appendByteCount > bytes->cap || bytes->size > bytes->cap - appendByteCount) {
                e68_capacity(bytes->cap, ((volatile OwnedResourceBytes*)bytes)->size + appendByteCount - bytes->cap);
            } else {
                memset((char*)bytes->begin + bytes->size, 0, appendByteCount);
                bytes->size = bytes->size + appendByteCount;
                break;
            }
            {
                ResourceBytesReplacement replacement;
                replacement.begin = 0;
                replacement.size = 0;
                replacement.cap = 0;
                replacement.pcap = &bytes->cap;
                replacement.offset = 0;
                unsigned int capacityIncrease = (bytes->size + appendByteCount) - bytes->cap;
                unsigned int previousCapacity = bytes->cap;
                if (capacityIncrease > 0xFFFFFFFFu - previousCapacity) {
                    e68_len_error();
                }
                unsigned int replacementCapacity;
                unsigned int halfGrowth;
                unsigned int threeFifthsGrowth;
                if (previousCapacity < 0x55555555u) {
                    threeFifthsGrowth = (previousCapacity + 1) * 3 / 5;
                    replacementCapacity = previousCapacity + e68_max(threeFifthsGrowth, capacityIncrease);
                } else if (previousCapacity < 0xAAAAAAAAu) {
                    halfGrowth = (previousCapacity + 1) / 2;
                    replacementCapacity = previousCapacity + e68_max(halfGrowth, capacityIncrease);
                } else {
                    replacementCapacity = 0xFFFFFFFFu;
                }
                replacement.begin = appMallocDummy__FUl(replacementCapacity);
                replacement.cap = replacementCapacity;
                replacement.offset = bytes->size;
                memset((char*)replacement.begin + replacement.offset + replacement.size, 0, appendByteCount);
                char* oldBytes = (char*)bytes->begin;
                unsigned int copiedByteCount = ((char*)bytes->begin + bytes->size) - oldBytes;
                replacement.size += appendByteCount;
                replacement.offset -= copiedByteCount;
                memcpy((char*)replacement.begin + replacement.offset, oldBytes, copiedByteCount);
                memset(oldBytes, 0, copiedByteCount);
                unsigned int replacementSize = replacement.size + copiedByteCount;
                replacement.size = replacementSize;
                void* newBytes = replacement.begin;
                void* retiredBytes = bytes->begin;
                unsigned int retiredCapacity = bytes->cap;
                bytes->size = 0;
                replacement.begin = retiredBytes;
                bytes->cap = replacement.cap;
                replacement.cap = retiredCapacity;
                bytes->begin = newBytes;
                bytes->size = replacementSize;
                replacement.size = 0;
            }
        } while (0);
    } else if (fileByteCount < bytes->size) {
        bytes->size -= bytes->size - fileByteCount;
    }
    // Phase 3: read into the committed vector, then close and disarm the guard.
    ((ResourceReadInterface*)resource)->m2(bytes->begin, fileByteCount);
    if (resource != 0) {
        Resource_Release(resource);
        resource = 0;
    }
    return 1;
}
#pragma pop

// Draw the exhibition and overlay its loading script while either object is unready.
int CAsyncExhibition::Draw() {
    void* owner = this;
    int exhibition = (*(int*)&this->exhibition);
    if (exhibition == 0) {
        return 0;
    }
    ((CExhibition*)exhibition)->DrawScene();
    int ready;
    {
        // Original CAsyncExhibition::Draw: keep the two readiness checks ordered.
        do {
            void* p = this->exhibition;
            if (p != 0 && (void*)((CExhibition*)p)->IsReady() == 0) {
                ready = 0;
                break;
            }
            {
                void* q = preloadObject;
                if (q != 0) {
                    struct VBase {
                        virtual int m0() DECOMP_LOCAL_VIRTUAL;
                        virtual int m1() DECOMP_LOCAL_VIRTUAL;
                        virtual int m2() DECOMP_LOCAL_VIRTUAL;
                        virtual int m3() DECOMP_LOCAL_VIRTUAL;
                        virtual int m4() DECOMP_LOCAL_VIRTUAL;
                    };
                    if (((VBase*)q)->m2() == 0) {
                        ready = 0;
                        break;
                    }
                }
                ready = 1;
            }
        } while (0);
    }
    if (ready == 0) {
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_asyncExhibitionBpCtrlDtorRegistration);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        void* receiver740_BpObject_Draw = (void*)IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
        BpObject_Draw(receiver740_BpObject_Draw);
    }
    return 1;
}

struct D50Obj {
    char pad0[0x8];
    void* f8;
    char pad1[0x14];
    void* f20;
};

#pragma opt_common_subs off
// Load the default font and optional message resource, then create the message renderer.
int CPangyaMsgResourceObject::CreateMessageDraw(void* resourcePath) {
    // Phase 1: borrow the cached default font; no renderer can be created without it.
    void* obj;
    obj = ResourceManager_GetOrLoadDefaultFixedFont(g_resourceManager, messageDrawFontAndErrorStrings);
    if (obj == 0) {
        return 0;
    }
    // Phase 2: optional path is copied temporarily; resourceBytes retain the
    // loaded data consumed by the renderer. Null path requests font-only creation.
    if (resourcePath != 0) {
        struct {
            unsigned int a;
            unsigned int b;
            void* c;
        } s;
        s.a = 0;
        s.b = 0;
        s.c = 0;
        StringUnwindCleanup_u80268730 stringCleanup0(&s);
        NString_AssignCString(&s, resourcePath);
        Resource_ReadFileIntoByteVector(this, (((s.a >> 31) & 1) == 0) ? (void*)((char*)&s + 1) : s.c,
                                        &resourceBytes);                      // original +0x8
        WFixedFont_CreateMsgEcbResource(obj, &messageDraw, resourceBytes, 0); // original +0x20/+0x8
        if (((s.a >> 31) & 1) != 0) {
            {
                stringCleanup0.release();
                appFreeUni__FPv(s.c);
            }
        }
    } else {
        WFixedFont_CreateMsgEcbResource(obj, &messageDraw, 0, 0); // original +0x20
    }
    // Phase 3: report creation, releasing the cached font reference on success.
    if (messageDraw == 0) { // original +0x20
        return 0;
    }
    if (g_resourceManager != 0 && obj != 0) {
        ResourceManager_ReleaseCachedObject(g_resourceManager, obj);
    }
    return 1;
}
#pragma opt_common_subs reset

struct Tmp4bc_8026897C {
    char pad[0x4bc];
    // Initialize allocated message-renderer storage from the font provider.
    Tmp4bc_8026897C(void* a) {
        ((CMsgDraw*)this)->__ct(a);
    }
};
struct DrawRefBase_89 {
    int refs;
};
struct DrawVirtual_89 : DrawRefBase_89 {
    virtual void destroy(int);
};

#pragma opt_common_subs off
// Allocate a shared draw object and create its renderer from the default fixed font.
int CPangyaMsgDrawObject::Create() {
    void* drawObject;
    void* messageDraw;
    void* owner = this;
    if (this == 0) {
        return 0;
    }
    drawObject = __nw__FUl(0x1c);
    if (drawObject != 0) {
        *(int*)drawObject = 1;
        AsCPangyaMsgDrawObject(drawObject)->drawVtable /* 0x4 */ = (void*)__vt__20CPangyaMsgDrawObject;
        AsCPangyaMsgDrawObject(drawObject)->textFactory /* 0x8 */ = 0;
        AsCPangyaMsgDrawObject(drawObject)->messageDraw /* 0xC */ = 0;
        AsCPangyaMsgDrawObject(drawObject)->stringFlags /* 0x10 */ = 0;
        AsCPangyaMsgDrawObject(drawObject)->stringLength /* 0x14 */ = 0;
        AsCPangyaMsgDrawObject(drawObject)->stringData /* 0x18 */ = 0;
    }
    int created;
    void* addr = messageDrawFontAndErrorStrings;
    if (addr == 0) {
        created = 0;
    } else {
        void* defaultFixedFont = ResourceManager_GetOrLoadDefaultFixedFont(g_resourceManager, (char*)addr);
        AsCPangyaMsgDrawObject(drawObject)->textFactory /* 0x8 */ = defaultFixedFont;
        if (defaultFixedFont == 0) {
            created = 0;
        } else {
            messageDraw =
                new Tmp4bc_8026897C(*(void**)((char*)AsCPangyaMsgDrawObject(drawObject)->textFactory /* 0x8 */ + 0xac));
            created = (messageDraw != 0) ? 1 : 0;
            AsCPangyaMsgDrawObject(drawObject)->messageDraw /* 0xC */ = messageDraw;
        }
    }
    // Original join at 0x80268A48.
    if (created != 0) {
        AsCPangyaMsgDrawObject(owner)->unk_0 = drawObject;
        return 1;
    }
    if (drawObject != 0) {
        if (drawObject != 0) {
            ((DrawVirtual_89*)drawObject)->destroy(1);
        }
    }
    return 0;
}

#pragma opt_common_subs reset
extern void* ResolvedItemSelection_Destroy(void*, int);
extern char __vt__16CAsyncExhibition[];
extern int Input_IsActionButtonDown(void*, int, int);
extern void InputManager_Init(void*);
extern void InputManager_Dtor(void*, int);
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_80268730[]; // Destructor-registration storage for the shared input manager.
extern signed char s_inputManagerInitialized;
extern float kFloat_5_PetMesh;
// Preview yaw input scale: used by horizontal pointer drag and left/right actions.
extern float s_previewYawInputScale;
extern float kFloat_0_017453292_PetMesh;
extern double kDouble_0_1_ExhibitionCamera;
extern int InputManager_GetPointerPosition(void*, void*, int);
extern int InputManager_GetCursorDepth(void*, void*, int);
extern float s_previewVerticalDragScale;

// A pair of screen coordinates, as the input manager reports the pointer.
struct Vec2f {
    float x;
    float y;
};
struct Vec3b {
    float x, y, z;
};

// While dragging, rotate the exhibition horizontally and offset its viewport vertically.
// Return 0 if receiver/input/pointer/depth is unavailable; 1 when handled,
// including release and first-anchor frames. Directional actions take priority.
// Horizontal delta: yaw += yawScale * deltaPixelsX * (pi/180), in radians.
// Vertical delta: viewport shift = trunc(verticalScale * deltaPixelsY), in
// viewport integer units. Depth-only movement does not trigger either update.
int CAsyncExhibition::UpdatePointerRotation() {
    // Phase 1: service button controls before checking drag state.
    int result = HandleCameraInput();
    if (result != 0) {
        return 1;
    }
    void* object = exhibition;
    if (object == 0) {
        return 0;
    }
    if (GetInputManagerSlot() == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, s_inputManagerDtorRegistration_80268730);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    void* input = GetInputManagerSlot();
    if (input == 0) {
        return 0;
    }
    if (Input_IsActionButtonDown(input, kPreviewPointerDrag, -1) == 0) {
        pointerDragActive = 0;
        return 1;
    }
    Vec2f current;
    float target;
    if (InputManager_GetPointerPosition(input, &current, -1) == 0) {
        return 0;
    }
    if (InputManager_GetCursorDepth(input, &target, -1) == 0) {
        return 0;
    }
    // Phase 2: first drag frame records an anchor without moving the exhibition.
    if (pointerDragActive == 0) {
        previousPointerX = current.x;
        previousPointerY = current.y;
        dragStartDepth = target;
        pointerDragActive = 1;
        return 1;
    }
    // Phase 3: apply incremental deltas, then advance the pixel anchor (not depth).
    if (current.x != previousPointerX || current.y != previousPointerY) {
        Vec3f* pos = (Vec3f*)((CExhibition*)object)->GetRotation();
        Vec3f adjusted = *pos;
        adjusted.y = s_previewYawInputScale * (current.x - previousPointerX) * kFloat_0_017453292_PetMesh + adjusted.y;
        ((CExhibition*)object)->SetRotation((const float*)&adjusted);
        ((CExhibition*)object)->OffsetViewport(0, (int)(s_previewVerticalDragScale * (current.y - previousPointerY)));
        previousPointerX = current.x;
        previousPointerY = current.y;
    }
    return 1;
}

// Release owned preload objects, script entry, exhibition and display descriptor.
CAsyncExhibition::~CAsyncExhibition() {
    // Phase 1: detach and destroy only internally owned preloaders.
    unsigned char flag = preloaderExternallyOwned;
    *(void**)this = (void*)__vt__16CAsyncExhibition;
    if (flag == 0) {
        void* p110 = exhibition;
        if (p110 != 0) {
            ((CExhibition*)p110)->ClearModelPreloader();
        }
        void* p1c = preloadObject;
        if (p1c != 0) {
            if (p1c != 0) {
                struct VBase {
                    virtual void m0(int) DECOMP_LOCAL_VIRTUAL;
                    virtual void m1(int) DECOMP_LOCAL_VIRTUAL;
                    virtual void m2(int) DECOMP_LOCAL_VIRTUAL;
                };
                ((VBase*)p1c)->m0(1);
            }
            preloadObject = 0;
        }
    }
    // Phase 2: unregister the loading BP entry before destroying its 3D exhibition.
    int f11c = bpEntryId;
    if (f11c != -1) {
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_asyncExhibitionBpCtrlDtorRegistration);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(bpEntryId);
        bpEntryId = -1;
    }
    void* p110b = exhibition;
    if (p110b != 0) {
        if (p110b != 0) {
            struct VBase {
                virtual void m0(int) DECOMP_LOCAL_VIRTUAL;
                virtual void m1(int) DECOMP_LOCAL_VIRTUAL;
                virtual void m2(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((VBase*)p110b)->m0(1);
        }
        exhibition = 0;
    }
    // Phase 3: destroy resolved selection storage, then optionally the receiver.
    ResolvedItemSelection_Destroy(displayDescriptor, -1);
}

// Consume directional camera actions, rotating the preview or changing camera distance.
// At most one action/frame: left, right, zoom out, zoom in (in that priority).
// Yaw increments use degrees converted to radians; zoom changes distance by
// 0.1 engine distance units per call, with no delta-time scaling or clamp here.
int CAsyncExhibition::HandleCameraInput() {
    void* exhibition = this->exhibition;
    if (exhibition == 0) {
        return 0;
    }
    if (GetInputManagerSlot() == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, s_inputManagerDtorRegistration_80268730);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    void* input = GetInputManagerSlot();
    if (input == 0) {
        return 0;
    }
    if (Input_IsActionButtonDown(input, kPreviewRotateLeft, -1) != 0) {
        Vec3b* rotation = (Vec3b*)((CExhibition*)exhibition)->GetRotation();
        Vec3b adjustedRotation = *rotation;
        adjustedRotation.y =
            adjustedRotation.y - kFloat_0_017453292_PetMesh * (kFloat_5_PetMesh * s_previewYawInputScale);
        ((CExhibition*)exhibition)->SetRotation((const float*)&adjustedRotation);
    } else if (Input_IsActionButtonDown(input, kPreviewRotateRight, -1) != 0) {
        Vec3b* rotation = (Vec3b*)((CExhibition*)exhibition)->GetRotation();
        Vec3b adjustedRotation = *rotation;
        adjustedRotation.y =
            adjustedRotation.y + kFloat_0_017453292_PetMesh * (kFloat_5_PetMesh * s_previewYawInputScale);
        ((CExhibition*)exhibition)->SetRotation((const float*)&adjustedRotation);
    } else if (Input_IsActionButtonDown(input, kPreviewZoomOut, -1) != 0) {
        float cameraDistance = ((CExhibition*)exhibition)->GetCameraDistance();
        ((CExhibition*)exhibition)->SetCameraDistance((float)((double)cameraDistance + kDouble_0_1_ExhibitionCamera));
    } else if (Input_IsActionButtonDown(input, kPreviewZoomIn, -1) != 0) {
        float cameraDistance = ((CExhibition*)exhibition)->GetCameraDistance();
        ((CExhibition*)exhibition)->SetCameraDistance((float)((double)cameraDistance - kDouble_0_1_ExhibitionCamera));
    } else {
        return 0;
    }
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046232C", _ctors_u80268730_0, PetMesh_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046232C", _ctors_u80268730_1, PetFormat_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046232C", _ctors_u80268730_2, MessageResource_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046232C", _ctors_u80268730_3, AsyncExhibition_InitAngleConstant);
#pragma pop
