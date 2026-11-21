typedef unsigned long new_size_t;
#include "classes/TextureCaptureQueue.h"
// CShadowManager_dtor: the queue created by CShadowManager_Ctor (0x801EEF2C)
// supplies captureWidth/captureHeight at +0x1C/+0x20 to Texture_Create2D.
#include "classes/ShadowList801EF.h"
#include "classes/ShadowSelection801F.h"
#include "classes/CShadowManager.h"
#include "MSL/vector.h"
#include "MSL/list.h"
#include <decomp/cw_bitfields.h>
// Child list of a shadow entry: {size, end.prev, end.next}; nodes carry one pointer value.
typedef std::list<void*, app::app_allocator<void*> > ShadowChildList;

#define AsCShadowManager(p) ((CShadowManager*)(p))
// Translation unit 0x801EEF2C-0x801F05C4 (11 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
void* operator new(new_size_t);
void operator delete(void*) throw();

extern "C" {

void BaseObject_ctor(void*);
void ResourceManager_ReleaseObject(void*, void*, int);
void BaseObject_dtor(void*, int);
void appFreeUni__FPv(void*);
void Memory_Clear(void*, int);
void TextureCaptureQueue_SetSize(void*, void*, void*);
void* __nw__FUl(unsigned int);
void __dl__FPv(void*);
void CubeResourceStorage_dtor(void*, int);
extern char s_shadowManagerBaseVtable[];
extern char __vt__14CShadowManager[];
extern float kFloat_1000_ShadowManager;
void* g_shadowResourceList;
void* g_resourceManager;
extern unsigned char s_shadowResourceLookupEnabled;
int g_shadowManager;
struct Link801EF { Link801EF* next; Link801EF* prev; void* value; };
// Ownership map: manager list at +0x1C owns entry objects; each entry's list owns
// payload objects, whose reference vector decrements 16-bit texture reference counts.
// Circular list sentinel links occupy two words; Link801EF::value is valid only
// for allocated nodes, never the sentinel. Traversals here run through prev links.
// Payload deletion, node unlink/free, and container deletion are separate phases.
// CShadowManager_dtor is the primary game entry point. fn_801EF4D8_v2 and _old
// below are retained emitted reconstruction variants, not alternate contracts to
// combine with it. Keep their distinct bodies/ABI symbols while preserving .text.
#define kShadowPreviousLinkOffset ((int)&((ShadowList801EF*)0)->sentinelPrevious)
#define kShadowSentinelOffset ((int)&((ShadowList801EF*)0)->sentinelNext)
#define kCaptureResourceOffset ((int)&((TextureCaptureQueue*)0)->resource)
#define kCaptureCountOffset ((int)&((TextureCaptureQueue*)0)->captureCount)
#define kCaptureSentinelOffset ((int)&((TextureCaptureQueue*)0)->headPrevious)
#define kCaptureNextLinkOffset ((int)&((TextureCaptureQueue*)0)->headNext)
#define kCaptureParametersOffset ((int)&((TextureCaptureQueue*)0)->pad_014)
#define kManagerSentinelOffset ((int)&((CShadowManager*)0)->listSentinelNext)
// Entry schema (Wii offsets; Item801F below provides the complete allocation view):
// +00 child count; +04/+08 embedded end links; +0C primary key[0x40];
// +4C secondary key[0x40]; +8C copied render settings[0x68]; +F4 selection value;
// +F8..177 unresolved storage; +178 unresolved initialized word; +17C flags.
// Child nodes own payload objects: +00 cleared head word, +04 pointer vector,
// +10 retained-texture vector. Both vectors are {data, count, capacity}; only
// the latter owns one texture reference per non-null element (signed 16-bit in
// constructor cleanup; unsigned 16-bit in the destructor reconstructions).
// LegacyShadowPayload below is a DISTINCT reconstruction using begin/end/cap:
// do not apply the count-vector schema to that retained emitted body.
struct ShadowEntryKeyView { ShadowList801EF children; char key[0x40]; };
struct Vec801EF { void** begin; void** end; void** cap; };
enum ShadowConstants { kShadowKeyBytes = 0x40, kCaptureWidth = 0x100, kCaptureHeight = 0x100,
    kEntrySettingsCopied = 1, kEntryOwnsCapture = 2, kDefaultSelectionFlags = 1 };
// Engine format token; the pixel/channel interpretation has not been established.
static const unsigned int kShadowCaptureTextureFormat = 0x80000006;

// Delete the selection's owned allocation, then its wrapper, and clear the manager slot.
static inline void ReleaseShadowSelection(void* self, void** owned) {
    if (owned) {
        if (owned) {
            if (*owned) {
                __dl__FPv(*owned);
                *owned = 0;
            }
            __dl__FPv(owned);
        }
        AsCShadowManager(self)->selection = 0;
    }
}

// Detach a nonempty reverse-traversed node range, then free links and decrement its owner count.
// Payloads must already be released. The embedded sentinel has no value and is never freed.
static inline void FreeShadowListLinksAfterFirst(Link801EF*& unlinkNode, Link801EF* sentinel, unsigned int& count, Link801EF* first) {
    unlinkNode->next->prev = first->prev;
    first->prev->next = unlinkNode->next;
    while (unlinkNode != sentinel) {
        Link801EF* old = unlinkNode;
        unlinkNode = unlinkNode->prev;
        appFreeUni__FPv(old);
        --count;
    }
}

// Snapshot the first link before applying the shared nonempty-range teardown.
static inline void FreeShadowListLinks(Link801EF*& unlinkNode, Link801EF* sentinel, unsigned int& count) {
    Link801EF* first = sentinel->next;
    FreeShadowListLinksAfterFirst(unlinkNode, sentinel, count, first);
}

// Child payload ownership: both vectors own buffers; retainedTextures also owns texture references.
typedef std::vector<void*, app::app_allocator<void*> > ShadowPointerVector;
struct ShadowOwnedPayload { void* head; ShadowPointerVector pointerStorage; ShadowPointerVector retainedTextures; };

typedef char ShadowOwnedPayload_size[sizeof(ShadowOwnedPayload) == 0x1C ? 1 : -1];
typedef char ShadowOwnedPayload_pointer_offset[(int)&((ShadowOwnedPayload*)0)->pointerStorage == 4 ? 1 : -1];
typedef char ShadowOwnedPayload_texture_offset[(int)&((ShadowOwnedPayload*)0)->retainedTextures == 0x10 ? 1 : -1];

// Free the two vector buffers after reference release, then delete the child payload itself.
// Keep reverse member destruction order; no texture reference is released by vector<void*>.
static inline void DeleteShadowPayloadStorage(ShadowOwnedPayload* payload) {
    payload->retainedTextures.~vector();
    payload->pointerStorage.~vector();
    __dl__FPv(payload);
}

// Release constructor-cleanup texture references with its emitted SIGNED 16-bit count view.
// This differs from the unsigned destructor helpers; preserve the signed decrement semantics.
static inline void ReleaseConstructorShadowPayloadReferences(ShadowOwnedPayload* payload, void**& referenceCursor) {
    payload->head = 0;
    if (payload->retainedTextures.size()) {
        for (referenceCursor = payload->retainedTextures.begin(); referenceCursor != payload->retainedTextures.end(); ++referenceCursor) {
            void* v = *referenceCursor;
            if (v) {
                struct Ref { CW_BITS2( int count:16, int flags:16) };
                if (--((Ref*)v)->count == 0) CubeResourceStorage_dtor(v, 1);
                *referenceCursor = 0;
            }
        }
    }
}

// Initialize selection, capture storage and the shared resource list, releasing prior owned entries.
void* CShadowManager_Ctor(char* self, void*, unsigned char selectionFlags) {
    // Phase 1: construct the base, publish the singleton, initialize empty entry links.
    // Separate enums preserve CodeWarrior's generated cleanup-symbol numbering.
    enum { kCaptureQueueBytes = 0xA4 };
    enum { kKeyBytes = 0x40 };
    enum { kCaptureWidth = 0x100 };
    enum { kCaptureHeight = 0x100 };
    enum { kResourceListBytes = 0xC };
    enum { kCaptureParameterBytes = 0x18 };
    typedef ShadowOwnedPayload Payload;
    char* entryBytes;
    Link801EF* childSentinel;
    Payload* payload;
    Link801EF* entryNode;
    Link801EF* entrySentinel;
    Link801EF* cleanupNode;
    void** referenceCursor;
    BaseObject_ctor(self);
    void* baseVtable = &s_shadowManagerBaseVtable;
    void* managerVtable = &__vt__14CShadowManager;
    float initialMinimumSelection = kFloat_1000_ShadowManager;
    AsCShadowManager(self)->vtbl = baseVtable;
    g_shadowManager = (int)self;
    AsCShadowManager(self)->vtbl = managerVtable;
    AsCShadowManager(self)->selectionFlags = selectionFlags;
    (*(int*)&AsCShadowManager(self)->captureQueue) = 0;
    AsCShadowManager(self)->minimumSelectionValue = initialMinimumSelection;
    (*(int*)&AsCShadowManager(self)->entryCount) = 0;
    AsCShadowManager(self)->listSentinelPrevious = (char*)self + kManagerSentinelOffset;
    *(void**)((char*)self + kManagerSentinelOffset) = (char*)self + kManagerSentinelOffset;
    (*(int*)&AsCShadowManager(self)->selection) = 0;
    AsCShadowManager(self)->projectionDrawArgument = 0;

    struct Tail {
        char data[kCaptureQueueBytes];
        // Initialize an empty capture queue with the existing 256x256 dimensions.
        Tail() {
            *(int*)this = 0;
            *(int*)((char*)this+4)=0;
            void* childEnd=(char*)this+8;
            *(void**)((char*)this+kCaptureNextLinkOffset)=childEnd;
            *(void**)((char*)this+8)=childEnd;
            Memory_Clear((char*)this+kCaptureParametersOffset,kCaptureParameterBytes);
            TextureCaptureQueue_SetSize(this,(void*)kCaptureWidth,(void*)kCaptureHeight);
        }
    };
    // Phase 2: allocate the capture queue, then release any preexisting nested entry
    // payloads and link storage before creating the shared registration container.
    cleanupNode = (Link801EF*)new Tail;
    entryNode = (*(Link801EF**)&AsCShadowManager(self)->listSentinelPrevious);
    void* tail = cleanupNode;
    struct Outer { char pad[0x20]; Link801EF sentinel; };
    AsCShadowManager(self)->captureQueue = tail;
    entrySentinel = &((Outer*)self)->sentinel;
    while (entrySentinel != entryNode) {
        entryBytes = (char*)entryNode->value;
        if (entryBytes) {
            Memory_Clear(((ShadowEntryKeyView*)entryBytes)->key, kShadowKeyBytes);
            cleanupNode = *(Link801EF**)(entryBytes + kShadowPreviousLinkOffset);
            childSentinel = (Link801EF*)(kShadowSentinelOffset + entryBytes);
            while (cleanupNode != childSentinel) {
                payload = (ShadowOwnedPayload*)cleanupNode->value;
                if (payload) {
                  if (payload) {
                    ReleaseConstructorShadowPayloadReferences(payload, referenceCursor);
                    DeleteShadowPayloadStorage(payload);
                  }
                    cleanupNode->value = 0;
                }
                cleanupNode = cleanupNode->prev;
            }
            {
                struct List { unsigned int count; Link801EF sentinel; };
                Link801EF* childEnd = &((List*)entryBytes)->sentinel;
                Link801EF* unlinkNode = *(Link801EF**)(kShadowPreviousLinkOffset + entryBytes);
                if (unlinkNode != childEnd) {
                    Link801EF* rangeFirst = childEnd->next;
                    unlinkNode->next->prev = rangeFirst->prev;
                    rangeFirst->prev->next = unlinkNode->next;
                    while (unlinkNode != childEnd) { Link801EF* releasedNode=unlinkNode;  unlinkNode=(unlinkNode->prev); appFreeUni__FPv(releasedNode); --AsShadowList801EF(entryBytes)->count; }
                }
            }
            if (entryBytes) { if (entryBytes) { if (entryBytes) { if (entryBytes) {
                struct List { int count; Link801EF sentinel; };
                Link801EF* childEnd = (Link801EF*)(kShadowSentinelOffset+entryBytes);
                Link801EF* unlinkNode = *(Link801EF**)(kShadowPreviousLinkOffset + entryBytes);
                if (unlinkNode != childEnd) {
                    Link801EF* rangeFirst = (childEnd->next);
                    unlinkNode->next->prev = rangeFirst->prev; rangeFirst->prev->next = unlinkNode->next;
                    while (unlinkNode != childEnd) {    Link801EF* releasedNode=unlinkNode; unlinkNode=(unlinkNode->prev); appFreeUni__FPv(releasedNode); --AsShadowList801EF(entryBytes)->count; }
                }
            } } } }
            __dl__FPv(entryBytes);
        }
        entryNode = (entryNode->prev);
    }
    {
        Link801EF* entryLinkCleanup = (*(Link801EF**)&AsCShadowManager(self)->listSentinelPrevious);
        if (entryLinkCleanup != entrySentinel) {
            Link801EF* rangeFirst=entrySentinel->next;
            entryLinkCleanup->next->prev=(rangeFirst->prev); rangeFirst->prev->next=entryLinkCleanup->next;
            while(entryLinkCleanup!=entrySentinel) { Link801EF* releasedNode=entryLinkCleanup; entryLinkCleanup=entryLinkCleanup->prev; appFreeUni__FPv(releasedNode); --AsCShadowManager(self)->entryCount; }
        }
    }
    void** selectionWrapper = (*(void***)&AsCShadowManager(self)->selection);
    ReleaseShadowSelection(self, selectionWrapper);
    { void* captureQueue = AsCShadowManager(self)->captureQueue; if (captureQueue) TextureCaptureQueue_SetSize(captureQueue, (void*)kCaptureWidth, (void*)kCaptureHeight); }

    void* resourceRegistrations = g_shadowResourceList;
    if (resourceRegistrations) {
      if (resourceRegistrations) {
        Link801EF* registrationNode = (*(Link801EF**)&AsShadowList801EF(resourceRegistrations)->sentinelPrevious);
        struct Global { unsigned int count; Link801EF sentinel; };
        Link801EF* registrationEnd = (Link801EF*)((char*)resourceRegistrations + kShadowSentinelOffset);
        while (registrationNode != registrationEnd) { ResourceManager_ReleaseObject(g_resourceManager, ((void**)registrationNode)[2], 0); registrationNode=registrationNode->prev; }
        {
            struct List { unsigned int count; Link801EF sentinel; };
            Link801EF* registrationEnd=&((List*)resourceRegistrations)->sentinel;
            Link801EF* registrationNode=(*(Link801EF**)&AsShadowList801EF(resourceRegistrations)->sentinelPrevious);
            if (registrationNode!=registrationEnd) { Link801EF* rangeFirst=registrationEnd->next; registrationNode->next->prev=rangeFirst->prev; rangeFirst->prev->next=registrationNode->next;
                while(registrationNode!=registrationEnd) { Link801EF* releasedNode=registrationNode; registrationNode=registrationNode->prev; appFreeUni__FPv(releasedNode); --AsShadowList801EF(resourceRegistrations)->count; }
            }
        }
        if (resourceRegistrations) { if (resourceRegistrations) { if (resourceRegistrations) { if (resourceRegistrations) {
            struct List { Link801EF sentinel; unsigned int count; };
            Link801EF* registrationEnd=(Link801EF*)((char*)resourceRegistrations + kShadowSentinelOffset);
            Link801EF* registrationNode=(*(Link801EF**)&AsShadowList801EF(resourceRegistrations)->sentinelPrevious);
            if(registrationEnd!=registrationNode) {  Link801EF* rangeFirst=(registrationEnd->next);   registrationNode->next->prev=(rangeFirst->prev);  rangeFirst->prev->next=(registrationNode->next);
                while(registrationEnd!=registrationNode) { Link801EF* releasedNode=registrationNode; registrationNode=registrationNode->prev; appFreeUni__FPv(releasedNode); --AsShadowList801EF(resourceRegistrations)->count; }
            }
        } } } }
        __dl__FPv(resourceRegistrations);
      }
        g_shadowResourceList=0;
    }
    void* newResourceList=__nw__FUl(kResourceListBytes);
    if (newResourceList) { (*(int*)&AsShadowList801EF(newResourceList)->count)=0; void* childEnd=(char*)&AsShadowList801EF(newResourceList)->sentinelNext; AsShadowList801EF(newResourceList)->sentinelPrevious=childEnd; AsShadowList801EF(newResourceList)->sentinelNext=childEnd; }
    s_shadowResourceLookupEnabled=1;
    g_shadowResourceList=newResourceList;
    return self;
}

// Store selection flags in both the manager and its current selection record.
void CShadowManager_SetSelectionFlags(void* self, unsigned char value) {
    AsCShadowManager(self)->selectionFlags = value;
    void* p = AsCShadowManager(self)->selection;
    AsShadowSelection801F(p)->flags = value;
}

void BaseObject_dtor(void*, int);
void __dl__FPv(void*);

// Clear the singleton, destroy the base object, and optionally free its allocation.
void* dtor_801EF3BC(void* self, int deleteFlag) {
    if (self != 0) {
        g_shadowManager = 0;
        BaseObject_dtor(self, 0);
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

int strcmp(void*, void*);
int strcmp(void*, void*);
void TextureItemQueue_ReplaceFront(void*, void*);
void PuppetMaterial_InitMaxUsedSlotCount(void*, void*);
void strncpy(void*, void*, int);
void Vec3_Scale(void*, void*, float);
void* __nw__FUl(unsigned int);
void appFreeUni__FPv(void*);
void Memory_Clear(void*, int);
void TextureCaptureQueue_SetSize(void*, void*, void*);
extern const float kFloat_neg1_SceneVector;
extern const float kFloat_0_Shadow;

int strcmp(void*, void*);
void strncpy(void*, void*, int);
void PuppetMaterial_InitMaxUsedSlotCount(void*, void*);
void* appMallocDummy__FUl(unsigned int);
int Texture_Create2D(void*, void*, int, int, int, int, void**, int);
void CubeResourceStorage_dtor(void*, int);

void TextureItemList_dtor(void*, int);
// Render snapshot copied to entry+0x8C. shadow_manager_capture.cpp's capture pass uses
// +0x00 as the caster anchor and +0x40 as the light direction. The latter
// three float BIT WORDS are negated through Vec3_Scale after copying.
// Other words/floats retain offset names until their consumers are established.
// Keep word copies: changing these members to float changes MWCC load/store code.
struct Src801EF_ {
    unsigned int anchorXBits, anchorYBits, anchorZBits, unresolvedWord0C, unresolvedWord10, unresolvedWord14, unresolvedWord18, unresolvedWord1C, unresolvedWord20, unresolvedWord24, unresolvedWord28, unresolvedWord2C, unresolvedWord30, unresolvedWord34, unresolvedWord38, unresolvedWord3C, lightDirectionXBits, lightDirectionYBits, lightDirectionZBits;
    float unresolvedFloat4C, unresolvedFloat50, unresolvedFloat54, unresolvedFloat58, unresolvedFloat5C, unresolvedFloat60, unresolvedFloat64;
};

} // extern "C"

struct Sent801F { Link801EF* next; Link801EF* prev; };
struct List801F {
    unsigned int count;
    Sent801F sentinel;
    // Return the embedded end marker; it has links but no value payload.
    Link801EF* end() { return (Link801EF*)&sentinel; }
    // Initialize an empty circular list.
    List801F() { count = 0; sentinel.prev = end(); sentinel.next = end(); }
    // Destroy list storage without deleting this embedded list object.
    ~List801F() { TextureItemList_dtor(this, -1); }
    // Free link nodes only; payload destruction is the caller's responsibility.
    void clear() {
        Link801EF* sentinel = end();
        Link801EF* e = sentinel->prev;
        if (e != sentinel) {
            Link801EF* first = sentinel->next;
            e->next->prev = first->prev;
            first->prev->next = e->next;
            while (e != sentinel) { Link801EF* old = e; e = e->prev; appFreeUni__FPv(old); --count; }
        }
    }
    struct Iter { Link801EF* resourceNode; };
    // Borrow the entry matching a primary key, or return null on exhaustion.
    struct Item801F* find(void* key) {
        Iter first; first.resourceNode = sentinel.prev;
        Iter node = first;
        Link801EF* res = (Link801EF*)((char*)this + 4);
        for (; node.resourceNode != res; node.resourceNode = node.resourceNode->prev) {
            if (strcmp(((ShadowEntryKeyView*)node.resourceNode->value)->key, key) == 0) { res = (Link801EF*)node.resourceNode->value; return (struct Item801F*)res; }
        }
        res = 0;
        return (struct Item801F*)res;
    }
    // Allocate a guarded link and attach the borrowed pointer value at the front.
    void push_front(void* v) {
        Link801EF* pos = end();
        struct Alloc {};
        Alloc al = Alloc();
        Link801EF* resourceNode = (Link801EF*)appMallocDummy__FUl(0xC);
        struct Pair {
            Link801EF* a; Link801EF* b;
            // Carry the new link and its allocation guard state together.
            Pair(Link801EF* x, const Alloc&) : a(x), b(x) {}
        } pr(resourceNode, al);
        struct Guard {
            Link801EF* pos; Link801EF* resourceNode;
            // Own the unlinked allocation until the splice succeeds.
            Guard(Link801EF* p, const Pair& x) : pos(p), resourceNode(x.a) {}
            // Free only an allocation that has not been linked into the list.
            ~Guard() { if (resourceNode) appFreeUni__FPv(resourceNode); }
        } registrations(pos, pr);
        try {
            void** p = &resourceNode->value;
            if (p) *p = v;
        } catch (...) {
            appFreeUni__FPv(resourceNode);
            throw;
        }
        registrations.resourceNode = 0;
        pos->next->prev = resourceNode;
        resourceNode->next = pos->next;
        pos->next = resourceNode;
        resourceNode->prev = pos;
        ++count;
    }
};

struct ShadowTextureReferenceView { CW_BITS2( unsigned int count:16, unsigned int flags:16) };

// Drop one retained unsigned 16-bit reference; delete the resource only at zero.
// The constructor's signed-bitfield reconstruction stays separate from this view.
static inline void ReleaseShadowTextureReference(void* texture) {
    ShadowTextureReferenceView* ref = (ShadowTextureReferenceView*)texture;
    if (--ref->count == 0) CubeResourceStorage_dtor(texture, 1);
}

// Release unsigned texture references using the reconstruction's byte cursor.
// Caller retains cursor storage and destroys vector buffers/payload after this pass.
static inline void ReleaseShadowPayloadReferencesWithByteCursor(ShadowOwnedPayload* payload, char*& referenceCursor) {
    payload->head = 0;
    if (payload->retainedTextures.size() != 0) {
        for (referenceCursor = (char*)payload->retainedTextures.begin(); referenceCursor != (char*)(payload->retainedTextures.end()); referenceCursor += 4) {
            void* v = *(void**)referenceCursor;
            if (v != 0) {
                void* retainedTexture = v;
                ReleaseShadowTextureReference(retainedTexture);
                *(void**)referenceCursor = 0;
            }
        }
    }

}

// Release unsigned texture references using the reconstruction's pointer cursor.
// Caller retains cursor storage and destroys vector buffers/payload after this pass.
static inline void ReleaseShadowPayloadReferencesWithPointerCursor(ShadowOwnedPayload* payload, void**& referenceCursor) {
    payload->head = 0;
    if (payload->retainedTextures.size() != 0) {
        for (referenceCursor = payload->retainedTextures.begin(); referenceCursor != payload->retainedTextures.end(); ++referenceCursor) {
            void* v = *referenceCursor;
            if (v != 0) {
                void* retainedTexture = v;
                ReleaseShadowTextureReference(retainedTexture);
                *referenceCursor = 0;
            }
        }
    }

}

struct Ref801F { CW_BITS2( unsigned int count:16, unsigned int flags:16) };

struct RefPtr801F {
    Ref801F* p;
    // Start without a retained capture texture.
    RefPtr801F() : p(0) {}
    // Drop this retained texture reference and clear the local pointer.
    void release() {
        if (p) { if (--p->count == 0) ((void (*)(void*, short))CubeResourceStorage_dtor)(p, 1); p = 0; }
    }
};

struct Item801F {
    List801F list;
    char key[0x40];
    char copyKey[0x40];
    Src801EF_ src;
    float value;
    char pad[0x178 - 0xF8];
    unsigned int unresolvedState178; // Initialized here; no consumer establishes its role.
    unsigned int flags;
    // Copy the primary key and initial selection value/flags into a fresh entry.
    Item801F(void* k, float v, unsigned int f) {
        value = v;
        unresolvedState178 = 0;
        list.clear();
        strncpy(key, k, kShadowKeyBytes);
        flags = f | kEntryOwnsCapture;
    }
    void set(void* ck, Src801EF_* sentinel);
};

extern "C" {

#pragma push
#pragma opt_propagation off
#pragma use_lmw_stmw on
// Update a named shadow entry or create it with a capture texture and copied render settings.
// key: borrowed primary lookup string; value/flags apply ONLY when creating an entry.
// source is a by-value settings snapshot, installed on BOTH paths with all three
// light-direction components negated (the X/Y/Z fields retain their float bits).
// Non-null copyKey replaces the secondary key on both paths; null leaves it untouched
// (including on a new entry). strncpy is bounded to 0x40 and need not add a terminator.
// Returned entry is manager-owned, invalidated by clear/destruction. A new capture
// texture is queued only on insertion; texture failure does not undo the new entry.
#pragma push
#pragma always_inline on
// Copy the optional secondary key and settings, then reverse the light direction.
// The entry owns its copied bytes; source and key are never retained.
static inline void InstallShadowEntrySettings(Item801F* object, void* copyKey, const Src801EF_& source) {
        if (copyKey) strncpy(object->copyKey, copyKey, 0x40);
        object->src = source;
        Vec3_Scale(&object->src.lightDirectionXBits, &object->src.lightDirectionXBits, kFloat_neg1_SceneVector);
        object->flags |= kEntrySettingsCopied;
}
#pragma pop

// Find/create a manager-owned entry; only insertion queues a capture texture reference.
void* CShadowManager_GetOrCreateEntry(void* self, void* key, float value, unsigned int flags, void* copyKey, Src801EF_ source) {
#define list ((List801F*)((char*)&AsCShadowManager(self)->entryCount))
    Item801F* object;
    {
        List801F::Iter first; first.resourceNode = list->sentinel.prev;
        List801F::Iter node = first;
        object = (Item801F*)((char*)&AsCShadowManager(self)->listSentinelNext);
        // Original 0x801EFF14: only exhaustion clears the search result.
        // Exhaustion side effect: reaching the sentinel clears object; a match
        // replaces that sentinel-shaped temporary with the borrowed entry payload.
        while (node.resourceNode != (Link801EF*)object || (object = 0, false)) {
            if (strcmp(((ShadowEntryKeyView*)node.resourceNode->value)->key, key) == 0) { object = (Item801F*)node.resourceNode->value; break; }
            node.resourceNode = node.resourceNode->prev;
        }
    }
    if (object != 0) {
        InstallShadowEntrySettings(object, copyKey, source);
    } else {
        object = new Item801F(key, value, flags);
        InstallShadowEntrySettings(object, copyKey, source);
        list->push_front(object);
#undef list
        char* container = (*(char**)&AsCShadowManager(self)->captureQueue);
        RefPtr801F result;
        // The entry is already linked. Failure releases only the texture result;
        // success transfers that reference to the capture queue.
        if (Texture_Create2D(((TextureCaptureQueue*)container)->captureWidth, ((TextureCaptureQueue*)container)->captureHeight, 1, 0, kShadowCaptureTextureFormat, 0, (void**)&result.p, 0) == 0)
            result.release();
        else
            ((List801F*)(container + 4))->push_front(result.p);
    }
    return object;
}
#pragma pop

struct Src801EF {
    unsigned int anchorXBits, anchorYBits, anchorZBits, unresolvedWord0C, unresolvedWord10, unresolvedWord14, unresolvedWord18, unresolvedWord1C, unresolvedWord20, unresolvedWord24, unresolvedWord28, unresolvedWord2C, unresolvedWord30, unresolvedWord34, unresolvedWord38, unresolvedWord3C, lightDirectionXBits, lightDirectionYBits, lightDirectionZBits;
    float unresolvedFloat4C, unresolvedFloat50, unresolvedFloat54, unresolvedFloat58, unresolvedFloat5C, unresolvedFloat60, unresolvedFloat64;
};
struct Obj801EF {
    char pad0[0x4C]; char copyKey[0x40];
    Src801EF src;
    char pad1[0x17C - 0x8C - sizeof(Src801EF)];
    unsigned int flags;
};

// Copy render settings to every entry whose secondary key matches, reversing its direction vector.
void CShadowManager_CopyNamedRecordSettings(Link801EF* node, void* key, const Src801EF* source) {
    Link801EF* sentinel = (Link801EF*)(((char*)&AsCShadowManager(node)->listSentinelNext));
    Obj801EF* object;
    node = (Link801EF*)AsCShadowManager(node)->listSentinelPrevious;
    while (node != sentinel) {
        object = (Obj801EF*)node->value;
        if (strcmp(object->copyKey, key) == 0) {
            Src801EF* dst = &object->src;
            char* vec = (char*)&object->src.lightDirectionXBits;
            dst->anchorXBits = source->anchorXBits; dst->anchorYBits = source->anchorYBits; dst->anchorZBits = source->anchorZBits; dst->unresolvedWord0C = source->unresolvedWord0C;
            dst->unresolvedWord10 = source->unresolvedWord10; dst->unresolvedWord14 = source->unresolvedWord14; dst->unresolvedWord18 = source->unresolvedWord18; dst->unresolvedWord1C = source->unresolvedWord1C;
            dst->unresolvedWord20 = source->unresolvedWord20; dst->unresolvedWord24 = source->unresolvedWord24; dst->unresolvedWord28 = source->unresolvedWord28; dst->unresolvedWord2C = source->unresolvedWord2C;
            dst->unresolvedWord30 = source->unresolvedWord30; dst->unresolvedWord34 = source->unresolvedWord34; dst->unresolvedWord38 = source->unresolvedWord38; dst->unresolvedWord3C = source->unresolvedWord3C;
            dst->lightDirectionXBits = source->lightDirectionXBits; dst->lightDirectionYBits = source->lightDirectionYBits; dst->lightDirectionZBits = source->lightDirectionZBits;
            dst->unresolvedFloat4C = source->unresolvedFloat4C; dst->unresolvedFloat50 = source->unresolvedFloat50; dst->unresolvedFloat54 = source->unresolvedFloat54; dst->unresolvedFloat58 = source->unresolvedFloat58;
            dst->unresolvedFloat5C = source->unresolvedFloat5C; dst->unresolvedFloat60 = source->unresolvedFloat60; dst->unresolvedFloat64 = source->unresolvedFloat64;
            Vec3_Scale(vec, vec, kFloat_neg1_SceneVector);
            object->flags |= kEntrySettingsCopied;
        }
        node = node->prev;
    }
}

// Retained legacy settings-copy variant with the same key and direction handling.
static void fn_801EFDC4_old(void* self, void* key, void* source) {
    Link801EF* sentinel = (Link801EF*)((char*)&AsCShadowManager(self)->listSentinelNext);
    Link801EF* node = (*(Link801EF**)&AsCShadowManager(self)->listSentinelPrevious);
    while (node != sentinel) {
        char* object = (char*)node->value;
        if (strcmp(object + 0x4C, key) == 0) {
            char* vec = object + 0xCC;
            char* vec2 = vec;
            ((Obj801EF*)object)->src.anchorXBits = ((Src801EF*)source)->anchorXBits;
            ((Obj801EF*)object)->src.anchorYBits = ((Src801EF*)source)->anchorYBits;
            ((Obj801EF*)object)->src.anchorZBits = ((Src801EF*)source)->anchorZBits;
            ((Obj801EF*)object)->src.unresolvedWord0C = ((Src801EF*)source)->unresolvedWord0C;
            ((Obj801EF*)object)->src.unresolvedWord10 = ((Src801EF*)source)->unresolvedWord10;
            ((Obj801EF*)object)->src.unresolvedWord14 = ((Src801EF*)source)->unresolvedWord14;
            ((Obj801EF*)object)->src.unresolvedWord18 = ((Src801EF*)source)->unresolvedWord18;
            ((Obj801EF*)object)->src.unresolvedWord1C = ((Src801EF*)source)->unresolvedWord1C;
            ((Obj801EF*)object)->src.unresolvedWord20 = ((Src801EF*)source)->unresolvedWord20;
            ((Obj801EF*)object)->src.unresolvedWord24 = ((Src801EF*)source)->unresolvedWord24;
            ((Obj801EF*)object)->src.unresolvedWord28 = ((Src801EF*)source)->unresolvedWord28;
            ((Obj801EF*)object)->src.unresolvedWord2C = ((Src801EF*)source)->unresolvedWord2C;
            ((Obj801EF*)object)->src.unresolvedWord30 = ((Src801EF*)source)->unresolvedWord30;
            ((Obj801EF*)object)->src.unresolvedWord34 = ((Src801EF*)source)->unresolvedWord34;
            ((Obj801EF*)object)->src.unresolvedWord38 = ((Src801EF*)source)->unresolvedWord38;
            ((Obj801EF*)object)->src.unresolvedWord3C = ((Src801EF*)source)->unresolvedWord3C;
            ((Obj801EF*)object)->src.lightDirectionXBits = ((Src801EF*)source)->lightDirectionXBits;
            ((Obj801EF*)object)->src.lightDirectionYBits = ((Src801EF*)source)->lightDirectionYBits;
            ((Obj801EF*)object)->src.lightDirectionZBits = ((Src801EF*)source)->lightDirectionZBits;
            ((Obj801EF*)object)->src.unresolvedFloat4C = ((Src801EF*)source)->unresolvedFloat4C;
            ((Obj801EF*)object)->src.unresolvedFloat50 = ((Src801EF*)source)->unresolvedFloat50;
            ((Obj801EF*)object)->src.unresolvedFloat54 = ((Src801EF*)source)->unresolvedFloat54;
            ((Obj801EF*)object)->src.unresolvedFloat58 = ((Src801EF*)source)->unresolvedFloat58;
            ((Obj801EF*)object)->src.unresolvedFloat5C = ((Src801EF*)source)->unresolvedFloat5C;
            ((Obj801EF*)object)->src.unresolvedFloat60 = ((Src801EF*)source)->unresolvedFloat60;
            ((Obj801EF*)object)->src.unresolvedFloat64 = ((Src801EF*)source)->unresolvedFloat64;
            Vec3_Scale(vec2, vec2, kFloat_neg1_SceneVector);
            ((Obj801EF*)object)->flags |= 1;
        }
        node = node->prev;
    }
}

void CubeResourceStorage_dtor(void*, int);
void appFreeUni__FPv(void*);

#pragma push
#pragma use_lmw_stmw on
// Destroy entries and selection, then resize the retained queue to the incoming width/height.
// On entry: (manager, width, height), with integer dimensions carried in pointer words.
// After saving dimensions, those same slots hold (child node, entry node, manager).
// Entries/payloads/selection are owned here; the capture queue survives teardown.
void CShadowManager_ClearEntriesAndResizeCaptureQueue(Link801EF* managerThenChildNode, Link801EF* widthThenEntryNode, void* heightThenManager) {
    typedef ShadowOwnedPayload Payload;
    char* entryBytes;
    Link801EF* childSentinel;
    Link801EF* entrySentinel;
    void** referenceCursor;
    Payload* payload;
    void* captureHeight = heightThenManager;
    void* captureWidth = widthThenEntryNode;
    heightThenManager = managerThenChildNode;
    entrySentinel = (Link801EF*)((char*)heightThenManager + kManagerSentinelOffset);
    widthThenEntryNode = (*(Link801EF**)&AsCShadowManager(heightThenManager)->listSentinelPrevious);
    while (widthThenEntryNode != entrySentinel) {
        entryBytes = (char*)widthThenEntryNode->value;
        if (entryBytes != 0) {
            Memory_Clear(((ShadowEntryKeyView*)entryBytes)->key, kShadowKeyBytes);
            managerThenChildNode = (*(Link801EF**)&AsShadowList801EF(entryBytes)->sentinelPrevious);
            childSentinel = (Link801EF*)(entryBytes + kShadowSentinelOffset);
            while (managerThenChildNode != childSentinel) {
                payload = (Payload*)managerThenChildNode->value;
                if (payload != 0) {
                  if (payload != 0) {
                    ReleaseShadowPayloadReferencesWithPointerCursor(payload, referenceCursor);
                    DeleteShadowPayloadStorage(payload);
                  }
                    managerThenChildNode->value = 0;
                }
                managerThenChildNode = managerThenChildNode->prev;
            }
            // Destroy the node's child list: empty it, run the list destructor, free the container.
            ShadowChildList* children = (ShadowChildList*)entryBytes;
            children->clear();
            children->~ShadowChildList();
            __dl__FPv(entryBytes);
        }
        widthThenEntryNode=widthThenEntryNode->prev;
    }
    // Outer list cleanup and owned pointer.
    struct Outer { char pad[0x20]; Link801EF sentinel; };
    Link801EF* end = &((Outer*)heightThenManager)->sentinel;
    Link801EF* node = (*(Link801EF**)&AsCShadowManager(heightThenManager)->listSentinelPrevious);
    if (node != end) {
        Link801EF* first=end->next;
        node->next->prev=first->prev; first->prev->next=node->next;
        while (node!=end) { Link801EF* old=node; node=node->prev; appFreeUni__FPv(old); --AsCShadowManager(heightThenManager)->entryCount; }
    }
    void** owned=(*(void***)&AsCShadowManager(heightThenManager)->selection);
    if (owned) { if (owned) { if (*owned) { __dl__FPv(*owned); *owned=0; } __dl__FPv(owned); } AsCShadowManager(heightThenManager)->selection=0; }
    void* tail=AsCShadowManager(heightThenManager)->captureQueue;
    if (tail) TextureCaptureQueue_SetSize(tail,captureWidth,captureHeight);
}
#pragma pop

struct Entry801F {
    void* a;
    void* b;
};

// Replace the selection payload and lower the manager's minimum selection value when needed.
void CShadowManager_SetSelectionValue(void* self, float value, void*, void* key) {
    void** holder = (*(void***)&AsCShadowManager(self)->selection);
    if (*holder != 0) {
        __dl__FPv(*holder);
        *holder = 0;
    }
    Entry801F* entry = (Entry801F*)__nw__FUl(8);
    if (entry != 0)
        PuppetMaterial_InitMaxUsedSlotCount(entry, key);
    *holder = entry;
    AsShadowSelection801F(AsCShadowManager(self)->selection)->value = value;
    if (AsCShadowManager(self)->minimumSelectionValue > value)
        AsCShadowManager(self)->minimumSelectionValue = value;
}

#pragma push
#pragma opt_propagation off
// Replace the current selection record with an empty payload, copied key and default flags.
void* CShadowManager_ReplaceSelection(void* self, void* key) {
    void* holder = AsCShadowManager(self)->selection;
    if (holder != 0) {
      if (holder != 0) {
        void** p = (void**)holder;
        if (*p != 0) {
            __dl__FPv(*p);
            *p = 0;
        }
        __dl__FPv(holder);
      }
        AsCShadowManager(self)->selection = 0;
    }
    char* result = (char*)__nw__FUl(0x4C);
    if (result != 0) {
        AsShadowSelection801F(result)->owned = 0;
        AsShadowSelection801F(result)->value = kFloat_0_Shadow;
        (*(int*)&AsShadowSelection801F(result)->flags) = kDefaultSelectionFlags;
        strncpy(AsShadowSelection801F(result)->key, key, 0x40);
    }
    AsCShadowManager(self)->selection = result;
    return result;
}
#pragma pop

// Unlink and free every list node, then optionally delete the list container.
void* dtor_801EF41C(ShadowChildList* self, int deleteFlag) {
    return ::std::__list_deleting_dtor(self, deleteFlag);
}

struct Node801F {
    Node801F* next;
    Node801F* prev;
    void* value;
};

#pragma push
#pragma opt_propagation off
// Find an entry by its primary key and replace the front of its texture queue.
void CShadowManager_ReplaceNamedTextureQueueFront(void* self, void* key, void* arg) {
    Node801F* node = (*(Node801F**)&AsCShadowManager(self)->listSentinelPrevious);
    Node801F* end = (Node801F*)((char*)&AsCShadowManager(self)->listSentinelNext);
    // self initially supplies the manager address. At exhaustion the comma expression
    // assigns null and ends the loop; on a match self becomes the entry payload.
    // The post-loop call therefore receives an entry, never the original manager.
    while (node != end || (self = 0, false)) {
        if (strcmp(((ShadowEntryKeyView*)node->value)->key, key) == 0) {
            self = node->value;
            break;
        }
        node = node->prev;
    }
    if (self != 0)
        TextureItemQueue_ReplaceFront(self, arg);
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_common_subs off
// Typed teardown views: capture owns a registration list; manager embeds its entry list at +0x1C.
struct ShadowTextureRef_u801EEF2C { CW_BITS2( unsigned int count:16, unsigned int flags:16) };
struct ShadowRegistrationList_u801EEF2C { unsigned int count; Link801EF sentinel; };
struct ShadowCaptureResource_u801EEF2C { int head; ShadowRegistrationList_u801EEF2C list; };
struct ShadowManagerList_u801EEF2C { char pad[0x1C]; ShadowRegistrationList_u801EEF2C list; };

// Release every retained capture texture while its list node remains available for traversal.
static inline void ReleaseShadowCaptureTextureReferences(Link801EF*& node, Link801EF*& sentinel) {
    while (node != sentinel) {
        ShadowTextureRef_u801EEF2C* ref = (ShadowTextureRef_u801EEF2C*)node->value;
        ReleaseShadowTextureReference(ref);
        node = node->prev;
    }
}

// Hand registered resources back to their shared manager before freeing registration links.
static inline void ReleaseShadowRegisteredResources(Link801EF*& node, Link801EF*& sentinel) {
    while (node != sentinel) {
        ResourceManager_ReleaseObject(g_resourceManager, node->value, 0);
        node = node->prev;
    }
}

// Destructor selector evidence: phase1_class_tables.cpp installs CShadowManager_dtor
// in __vt__14CShadowManager; symbols.txt maps that entry to original address 0x801EF4D8.
// Neither fn_801EF4D8_v2 nor fn_801EF4D8_old has a tracked caller or table reference.
// They are retained reconstruction definitions, with no established runtime selector.
// Preserve their distinct ABIs/cleanup shapes for object identity; do not combine their ownership rules.
// Primary uses byte reference cursors; v2 uses pointer cursors; legacy uses begin/end/cap vectors
// and omits capture deletion. Each keeps its emitted null/delete gates and destructor passes.
// Release entry payloads, capture textures and shared resources before destroying the manager.
#pragma push
#pragma always_inline on
#pragma inline_depth(16)

// Release child payloads with the primary destructor's byte cursor, keeping links alive for traversal.
static inline void ReleaseShadowChildrenWithByteCursor(Link801EF*& cleanupNode, Link801EF*& childSentinel,
        ShadowOwnedPayload*& payload, char*& referenceCursor) {
    while (cleanupNode != childSentinel) {
        payload = (ShadowOwnedPayload*)cleanupNode->value;
        if (payload != 0) {
          if (payload != 0) {
            ReleaseShadowPayloadReferencesWithByteCursor(payload, referenceCursor);
            DeleteShadowPayloadStorage(payload);
          }
            cleanupNode->value = 0;
        }
        cleanupNode = cleanupNode->prev;
    }
}

// Release the capture list's references, run both link-cleanup passes, then free its container.
// Caller supplies cursor slots because the original destructor disables lifetime optimization.
static inline void DestroyShadowCaptureStorage(ShadowCaptureResource_u801EEF2C* captureQueue,
        Link801EF*& unlinkNode, Link801EF*& captureNode, Link801EF*& captureEnd, Link801EF*& finalEnd) {
    if (captureQueue != 0) {
        if (captureQueue->list.count != 0) {
            captureNode = captureQueue->list.sentinel.prev;
            captureEnd = &captureQueue->list.sentinel;
            ReleaseShadowCaptureTextureReferences(captureNode, captureEnd);
            unlinkNode = captureQueue->list.sentinel.prev;
            captureEnd = &captureQueue->list.sentinel;
            if (unlinkNode != captureEnd) {
                FreeShadowListLinks(unlinkNode, captureEnd, captureQueue->list.count);
            }
        }
        if (&captureQueue->list) { if (&captureQueue->list) { if (&captureQueue->list) { if (&captureQueue->list) {
            unlinkNode = captureQueue->list.sentinel.prev;
            finalEnd = &captureQueue->list.sentinel;
            if (unlinkNode != finalEnd) {
                FreeShadowListLinks(unlinkNode, finalEnd, captureQueue->list.count);
            }
        } } } }
        __dl__FPv(captureQueue);
    }
}

// Run entry clear/destructor link passes after child payload release, then free the entry container.
static inline void DestroyShadowEntryContainer(char* entryBytes, Link801EF*& unlinkNode,
        Link801EF*& clearEnd, Link801EF*& destroyEnd) {
    {
        clearEnd = (Link801EF*)(entryBytes + kShadowSentinelOffset);
        unlinkNode = (*(Link801EF**)&AsShadowList801EF(entryBytes)->sentinelPrevious);
        if (unlinkNode != clearEnd) {
            FreeShadowListLinks(unlinkNode, clearEnd, AsShadowList801EF(entryBytes)->count);
        }
    }
    if (entryBytes) { if (entryBytes) { if (entryBytes) { if (entryBytes) {
        destroyEnd = (Link801EF*)(entryBytes + kShadowSentinelOffset);
        unlinkNode = (*(Link801EF**)&AsShadowList801EF(entryBytes)->sentinelPrevious);
        if (unlinkNode != destroyEnd) {
            FreeShadowListLinks(unlinkNode, destroyEnd, AsShadowList801EF(entryBytes)->count);
        }
    } } } }
    __dl__FPv(entryBytes);
}

// Release an entry's owned child payloads, clear its links, and delete the entry container.
static inline void DestroyPrimaryShadowEntry(char* entryBytes, Link801EF*& cleanupNode, Link801EF*& childSentinel,
        ShadowOwnedPayload*& payload, char*& referenceCursor, Link801EF*& unlinkNode,
        Link801EF*& clearEnd, Link801EF*& destroyEnd) {
    Memory_Clear(((ShadowEntryKeyView*)entryBytes)->key, kShadowKeyBytes);
    cleanupNode = (*(Link801EF**)&AsShadowList801EF(entryBytes)->sentinelPrevious);
    childSentinel = (Link801EF*)(entryBytes + kShadowSentinelOffset);
    ReleaseShadowChildrenWithByteCursor(cleanupNode, childSentinel, payload, referenceCursor);
    DestroyShadowEntryContainer(entryBytes, unlinkNode, clearEnd, destroyEnd);
}

// Release an entry's owned child payloads, clear its links, and delete the entry container.
static inline void DestroyPointerCursorShadowEntry(char* entryBytes, Link801EF*& cleanupNode, Link801EF*& childSentinel,
        ShadowOwnedPayload*& payload, void**& referenceCursor, Link801EF*& unlinkNode,
        Link801EF*& clearEnd, Link801EF*& destroyEnd) {
    Memory_Clear(((ShadowEntryKeyView*)entryBytes)->key, kShadowKeyBytes);
    cleanupNode = (*(Link801EF**)&AsShadowList801EF(entryBytes)->sentinelPrevious);
    childSentinel = (Link801EF*)(entryBytes + kShadowSentinelOffset);
    while (cleanupNode != childSentinel) {
        payload = (ShadowOwnedPayload*)cleanupNode->value;
        if (payload != 0) {
          if (payload != 0) {
            ReleaseShadowPayloadReferencesWithPointerCursor(payload, referenceCursor);
            DeleteShadowPayloadStorage(payload);
          }
            cleanupNode->value = 0;
        }
        cleanupNode = cleanupNode->prev;
    }
    DestroyShadowEntryContainer(entryBytes, unlinkNode, clearEnd, destroyEnd);
}

// Release entries, selection, capture ownership and shared registrations, then the base.
void* CShadowManager_dtor(void* self, int deleteFlag) {
    // Teardown order: nested payloads/references -> entry links -> selection ->
    // capture queue -> shared resource registrations -> base -> optional owner free.
    typedef ShadowOwnedPayload Payload;
    char* entryBytes;
    Link801EF* childSentinel;
    Link801EF* entrySentinel;
    char* referenceCursor;
    Payload* payload;
    Link801EF* node;
    Link801EF* cleanupNode;
    Link801EF* unlinkNode;
    void* registrations;

    // Original 0x801EF4D8: null destruction skips all owned resources.
    if (self != 0) {
        AsCShadowManager(self)->vtbl = __vt__14CShadowManager;
        entrySentinel = (Link801EF*)((char*)&AsCShadowManager(self)->listSentinelNext);
        node = (*(Link801EF**)&AsCShadowManager(self)->listSentinelPrevious);
        while (node != entrySentinel) {
            entryBytes = (char*)node->value;
            if (entryBytes != 0) {
                Link801EF* clearEnd;
                Link801EF* destroyEnd;
                DestroyPrimaryShadowEntry(entryBytes, cleanupNode, childSentinel, payload, referenceCursor, unlinkNode, clearEnd, destroyEnd);
            }
            node = node->prev;
        }
        // Entry payloads are gone; now unlink/free the manager's outer link nodes.
        {
            cleanupNode = (Link801EF*)((char*)&AsCShadowManager(self)->listSentinelNext);
            node = (*(Link801EF**)&AsCShadowManager(self)->listSentinelPrevious);
            if (node != cleanupNode) {
                FreeShadowListLinks(node, cleanupNode, AsCShadowManager(self)->entryCount);
            }
        }
        // Selection owns both its inner allocation and the wrapper.
        {
            void** owned = (*(void***)&AsCShadowManager(self)->selection);
            ReleaseShadowSelection(self, owned);
        }
        // Reset the capture queue's dimensions before destroying its retained textures.
        {
            void* tail = AsCShadowManager(self)->captureQueue;
            if (tail) TextureCaptureQueue_SetSize(tail, (void*)kCaptureWidth, (void*)kCaptureHeight);
        }
        // Release capture references, then node storage, then the queue container.
        {
            ShadowCaptureResource_u801EEF2C* captureQueue = (*(ShadowCaptureResource_u801EEF2C**)&AsCShadowManager(self)->captureQueue);
            if (captureQueue != 0) {
                Link801EF* captureNode;
                Link801EF* captureEnd;
                Link801EF* finalEnd;
                DestroyShadowCaptureStorage(captureQueue, unlinkNode, captureNode, captureEnd, finalEnd);
                AsCShadowManager(self)->captureQueue = 0;
            }
        }
        // Release resources through the shared manager before freeing registration nodes.
        registrations = g_shadowResourceList;
        if (registrations != 0) {
            ShadowRegistrationList_u801EEF2C* list = (ShadowRegistrationList_u801EEF2C*)registrations;
            Link801EF* sentinel = &list->sentinel;
            Link801EF* resourceNode = sentinel->prev;
            ReleaseShadowRegisteredResources(resourceNode, sentinel);
            unlinkNode = list->sentinel.prev;
            sentinel = &list->sentinel;
            if (unlinkNode != sentinel) {
                FreeShadowListLinks(unlinkNode, sentinel, list->count);
            }
        }
        // Finish global registration container destruction after the first clear.
        registrations = g_shadowResourceList;
        if (registrations != 0) {
            if (registrations != 0) {
                ShadowRegistrationList_u801EEF2C* list = (ShadowRegistrationList_u801EEF2C*)registrations;
                Link801EF* sentinel = &list->sentinel;
                Link801EF* resourceNode = sentinel->prev;
                ReleaseShadowRegisteredResources(resourceNode, sentinel);
                unlinkNode = list->sentinel.prev;
                sentinel = &list->sentinel;
                if (unlinkNode != sentinel) {
                    FreeShadowListLinks(unlinkNode, sentinel, list->count);
                }
                if (registrations) { if (registrations) { if (registrations) { if (registrations) {
                    Link801EF* sentinel = &list->sentinel;
                    unlinkNode = sentinel->prev;
                    if (unlinkNode != sentinel) {
                        FreeShadowListLinks(unlinkNode, sentinel, list->count);
                    }
                } } } }
                __dl__FPv(registrations);
            }
            g_shadowResourceList = 0;
        }
        // Final embedded-list cleanup (normally empty after the first pass).
        {
            ShadowManagerList_u801EEF2C* manager = (ShadowManagerList_u801EEF2C*)self;
            if (&manager->list) { if (&manager->list) { if (&manager->list) { if (&manager->list) {
                unlinkNode = manager->list.sentinel.prev;
                Link801EF* sentinel = &manager->list.sentinel;
                if (unlinkNode != sentinel) {
                    FreeShadowListLinks(unlinkNode, sentinel, manager->list.count);
                }
            } } } }
        }
        if (self != 0) {
            g_shadowManager = 0;
            BaseObject_dtor(self, 0);
        }
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Retained compiled destructor variant; release entries, capture ownership and shared resources.
// Navigation matches the primary destructor: payload references first, then link
// storage, selection, capture references, global registrations and base destruction.
// Local Array/Payload views here belong to this emitted variant; they do not add
// another layer of ownership to CShadowManager_dtor's similarly named local types.
#pragma push
#pragma always_inline on
#pragma inline_depth(16)

// Destroy retained capture textures and link storage using this reconstruction's pointer cursor.
static inline void DestroyPointerCursorShadowCaptureStorage(void* captureQueue, Link801EF*& node, Link801EF*& cleanupNode) {
    if (captureQueue != 0) {
        if (((ShadowCaptureResource_u801EEF2C*)captureQueue)->list.count != 0) {
            node = &((ShadowCaptureResource_u801EEF2C*)captureQueue)->list.sentinel;
            cleanupNode = node->prev;
            ReleaseShadowCaptureTextureReferences(cleanupNode, node);
            node = &((ShadowCaptureResource_u801EEF2C*)captureQueue)->list.sentinel;
            cleanupNode = node->prev;
            if (cleanupNode != node) {
                FreeShadowListLinks(cleanupNode, node, ((ShadowCaptureResource_u801EEF2C*)captureQueue)->list.count);
            }
        }
        if (&((ShadowCaptureResource_u801EEF2C*)captureQueue)->list) { if (&((ShadowCaptureResource_u801EEF2C*)captureQueue)->list) { if (&((ShadowCaptureResource_u801EEF2C*)captureQueue)->list) { if (&((ShadowCaptureResource_u801EEF2C*)captureQueue)->list) {
            node = &((ShadowCaptureResource_u801EEF2C*)captureQueue)->list.sentinel;
            cleanupNode = node->prev;
            if (cleanupNode != node) {
                FreeShadowListLinks(cleanupNode, node, ((ShadowCaptureResource_u801EEF2C*)captureQueue)->list.count);
            }
        } } } }
        __dl__FPv(captureQueue);
    }
}

// Retained pointer-cursor reconstruction: destroy entries, capture storage, registrations and base.
void* fn_801EF4D8_v2(void* self, int deleteFlag) {
    typedef ShadowOwnedPayload Payload;
    char* entryBytes;
    Link801EF* childSentinel;
    Link801EF* entrySentinel;
    Payload* payload;
    void** referenceCursor;
    Link801EF* node;
    Link801EF* cleanupNode;
    Link801EF* unlinkNode;

    if (self == 0)
        return self;
    AsCShadowManager(self)->vtbl = __vt__14CShadowManager;
    entrySentinel = (Link801EF*)((char*)&AsCShadowManager(self)->listSentinelNext);
    node = (*(Link801EF**)&AsCShadowManager(self)->listSentinelPrevious);
    while (node != entrySentinel) {
        entryBytes = (char*)node->value;
        if (entryBytes != 0) {
            Link801EF* clearEnd;
            Link801EF* destroyEnd;
            DestroyPointerCursorShadowEntry(entryBytes, cleanupNode, childSentinel, payload, referenceCursor, unlinkNode, clearEnd, destroyEnd);
        }
        node = node->prev;
    }
    // Payloads are released; unlink/free outer entry nodes before selection teardown.
    {
        cleanupNode = (Link801EF*)((char*)&AsCShadowManager(self)->listSentinelNext);
        node = (*(Link801EF**)&AsCShadowManager(self)->listSentinelPrevious);
        if (node != cleanupNode) {
            FreeShadowListLinks(node, cleanupNode, AsCShadowManager(self)->entryCount);
        }
    }
    // Delete the selection payload and its owning wrapper.
    {
        void** owned = (*(void***)&AsCShadowManager(self)->selection);
        ReleaseShadowSelection(self, owned);
    }
    // Reset dimensions of the retained capture queue before releasing it.
    {
        void* tail = AsCShadowManager(self)->captureQueue;
        if (tail) TextureCaptureQueue_SetSize(tail, (void*)kCaptureWidth, (void*)kCaptureHeight);
    }
    // Release retained capture textures, then queue nodes and container.
    {
        void* captureQueue = AsCShadowManager(self)->captureQueue;
        if (captureQueue != 0) {
            DestroyPointerCursorShadowCaptureStorage(captureQueue, node, cleanupNode);
            AsCShadowManager(self)->captureQueue = 0;
        }
    }
    // Release each global registration through the resource manager, then free links.
    {
        void* registrations = g_shadowResourceList;
        if (registrations != 0) {
            node = (Link801EF*)AsShadowList801EF(registrations)->sentinelPrevious;
            cleanupNode = (Link801EF*)((char*)registrations + kShadowSentinelOffset);
            ReleaseShadowRegisteredResources(node, cleanupNode);
            node = (Link801EF*)AsShadowList801EF(registrations)->sentinelPrevious;
            cleanupNode = (Link801EF*)((char*)registrations + kShadowSentinelOffset);
            if (node != cleanupNode) {
                Link801EF* first = cleanupNode->next;
                FreeShadowListLinksAfterFirst(node, cleanupNode, *(unsigned int*)registrations, first);
            }
        }
    }
    // Finish global registration container destruction after the first clear.
    {
        void* registrations = g_shadowResourceList;
        if (registrations != 0) {
            if (registrations != 0) {
                node = (Link801EF*)AsShadowList801EF(registrations)->sentinelPrevious;
                cleanupNode = (Link801EF*)((char*)registrations + kShadowSentinelOffset);
                ReleaseShadowRegisteredResources(node, cleanupNode);
                node = (Link801EF*)AsShadowList801EF(registrations)->sentinelPrevious;
                cleanupNode = (Link801EF*)((char*)registrations + kShadowSentinelOffset);
                if (node != cleanupNode) {
                    Link801EF* first = cleanupNode->next;
                    FreeShadowListLinksAfterFirst(node, cleanupNode, *(unsigned int*)registrations, first);
                }
                if (registrations) { if (registrations) { if (registrations) { if (registrations) {
                    node = (Link801EF*)AsShadowList801EF(registrations)->sentinelPrevious;
                    cleanupNode = (Link801EF*)((char*)registrations + kShadowSentinelOffset);
                    if (node != cleanupNode) {
                        Link801EF* first = cleanupNode->next;
                        FreeShadowListLinksAfterFirst(node, cleanupNode, *(unsigned int*)registrations, first);
                    }
                } } } }
                __dl__FPv(registrations);
            }
            g_shadowResourceList = 0;
        }
    }
    // Final embedded-list cleanup (normally empty after the first pass).
    {
        if ((char*)&AsCShadowManager(self)->entryCount) { if ((char*)&AsCShadowManager(self)->entryCount) { if ((char*)&AsCShadowManager(self)->entryCount) { if ((char*)&AsCShadowManager(self)->entryCount) {
            cleanupNode = (Link801EF*)((char*)&AsCShadowManager(self)->listSentinelNext);
            node = (*(Link801EF**)&AsCShadowManager(self)->listSentinelPrevious);
            if (node != cleanupNode) {
                FreeShadowListLinks(node, cleanupNode, AsCShadowManager(self)->entryCount);
            }
        } } } }
    }
    if (self != 0) {
        g_shadowManager = 0;
        BaseObject_dtor(self, 0);
    }
    if (deleteFlag > 0)
        __dl__FPv(self);
    return self;
}
#pragma pop
#pragma pop

struct LegacyShadowPayload { void* head; Vec801EF secondary; Vec801EF references; };
// Decrement and clear each retained texture slot in the legacy begin/end/cap vector.
// Caller owns cursor storage and separately frees the vector buffer after iteration.
static inline void ReleaseLegacyShadowReferenceElements(Vec801EF* retainedReferences, void**& referenceCursor, void*& texture, unsigned int& referenceWord) {
    enum { kReferenceCountShift = 16, kReferenceFlagsMask = 0xFFFF };
    for (referenceCursor = retainedReferences->begin; referenceCursor != retainedReferences->end; ++referenceCursor) {
        texture = *referenceCursor;
        if (texture != 0) {
            referenceWord = *(unsigned int*)texture;
            referenceWord = (referenceWord & kReferenceFlagsMask) | (((referenceWord >> kReferenceCountShift) - 1) << kReferenceCountShift);
            *(unsigned int*)texture = referenceWord;
            if ((referenceWord >> kReferenceCountShift) == 0) CubeResourceStorage_dtor(texture, 1);
            *referenceCursor = 0;
        }
    }
}

// Retained compiled legacy destructor variant; release nested payloads and both resource lists.
// This reconstruction expresses vectors as begin/end/cap rather than count/capacity.
// Follow the same lifetime layers, but do not copy its link arithmetic into the
// primary destructor: these are separate compiled bodies retained for code identity.
#pragma push
#pragma always_inline on
#pragma inline_depth(16)

#pragma push
#pragma always_inline on
#pragma inline_depth(16)
// Unlink and free legacy nodes after payload release; this reconstruction rewires last->next.
static inline void FreeLegacyShadowLinks(Link801EF*& unlinkNode, Link801EF*& sentinel, unsigned int& count) {
    Link801EF* first = sentinel->next;
    Link801EF* last = unlinkNode->prev;
    last->next = first->next;
    first->next->prev = last;
    while (unlinkNode != sentinel) {
        Link801EF* old = unlinkNode;
        unlinkNode = unlinkNode->prev;
        appFreeUni__FPv(old);
        --count;
    }
}

// Retained begin/end/cap reconstruction: release entries and registrations, then the base.
void* fn_801EF4D8_old(void* self, int deleteFlag) {
    // Phase 1: release child reference payloads while their owning nodes are live.
    if (self != 0) {
        AsCShadowManager(self)->vtbl = __vt__14CShadowManager;
        Link801EF* entrySentinel = (Link801EF*)((char*)&AsCShadowManager(self)->listSentinelNext);
        Link801EF* entryNode = (*(Link801EF**)&AsCShadowManager(self)->listSentinelPrevious);
        int zero = 0;
        while (entryNode != entrySentinel) {
            Link801EF* entry = (Link801EF*)entryNode->value;
            if (entry != 0) {
                Memory_Clear(((ShadowEntryKeyView*)entry)->key, 0x40);
                Link801EF* childNode = (*(Link801EF**)&AsShadowList801EF(entry)->sentinelPrevious);
                Link801EF* childSentinel = (Link801EF*)((char*)&AsShadowList801EF(entry)->sentinelNext);
                while (childNode != childSentinel) {
                    void* payload = childNode->value;
                    if (payload != 0) {
                        *(void**)payload = 0;
                        Vec801EF* retainedReferences = &((LegacyShadowPayload*)payload)->references;
                        if (retainedReferences->end != 0) {
                            void** referenceCursor;
                            void* texture;
                            unsigned int referenceWord;
                            ReleaseLegacyShadowReferenceElements(retainedReferences, referenceCursor, texture, referenceWord);
                            if (retainedReferences->begin != 0) { retainedReferences->end = retainedReferences->begin; appFreeUni__FPv(retainedReferences->begin); }
                        }
                        Vec801EF* pointerStorage = &((LegacyShadowPayload*)payload)->secondary;
                        if (pointerStorage->begin != 0) { pointerStorage->end = pointerStorage->begin; appFreeUni__FPv(pointerStorage->begin); }
                        __dl__FPv(payload);
                        childNode->value = 0;
                    }
                    childNode = childNode->prev;
                }
                // Phase 2: unlink child nodes after their retained resources are released.
                {
                    Link801EF* unlinkNode = (*(Link801EF**)&AsShadowList801EF(entry)->sentinelPrevious);
                    Link801EF* sentinel = (Link801EF*)((char*)&AsShadowList801EF(entry)->sentinelNext);
                    if (unlinkNode != sentinel) {
                        FreeLegacyShadowLinks(unlinkNode, sentinel, AsShadowList801EF(entry)->count);
                    }
                }
                // Repeat the emitted child-container destructor cleanup; normally empty.
                if (entry != 0) {
                    Link801EF* unlinkNode = (*(Link801EF**)&AsShadowList801EF(entry)->sentinelPrevious);
                    Link801EF* sentinel = (Link801EF*)((char*)&AsShadowList801EF(entry)->sentinelNext);
                    if (unlinkNode != sentinel) {
                        FreeLegacyShadowLinks(unlinkNode, sentinel, AsShadowList801EF(entry)->count);
                    }
                }
                __dl__FPv(entry);
            }
            entryNode = entryNode->prev;
        }
        // Phase 3: unlink outer entry nodes, then release the selection wrapper.
        {
            Link801EF* entryCleanupNode = entrySentinel->prev;
            if (entryCleanupNode != entrySentinel) {
                Link801EF* first = entrySentinel->next;
                Link801EF* last = entryCleanupNode->prev;
                last->next = first->next;
                first->next->prev = last;
                while (entryCleanupNode != entrySentinel) { Link801EF* old = entryCleanupNode; entryCleanupNode = entryCleanupNode->prev; appFreeUni__FPv(old); --AsCShadowManager(self)->entryCount; }
            }
        }
        void** owned = (*(void***)&AsCShadowManager(self)->selection);
        if (owned) { if (*owned) { __dl__FPv(*owned); *owned = 0; } __dl__FPv(owned); AsCShadowManager(self)->selection = 0; }
        void* tail = AsCShadowManager(self)->captureQueue;
        if (tail) TextureCaptureQueue_SetSize(tail, (void*)kCaptureWidth, (void*)kCaptureHeight);
        // Phase 4: release shared resource registrations, then their link storage.
        void* registrations = g_shadowResourceList;
        if (registrations != 0) {
            // traverse prev chain: entryNode = registrations->prev (offset 8), sentinel = registrations->next (offset 4)
            void* registrationNode = AsShadowList801EF(registrations)->sentinelPrevious;
            void* registrationSentinel = (void*)((char*)&AsShadowList801EF(registrations)->sentinelNext);
            while (registrationNode != registrationSentinel) {
                ResourceManager_ReleaseObject(g_resourceManager, ((Link801EF*)registrationNode)->value, 0);
                registrationNode = ((Link801EF*)registrationNode)->prev;
            }
            // cleanup pass 1
            {
                void* registrationCleanupNode = AsShadowList801EF(registrations)->sentinelPrevious;
                void* registrationEnd = (void*)((char*)&AsShadowList801EF(registrations)->sentinelNext);
                if (registrationCleanupNode != registrationEnd) {
                    // unlink first/last
                    // first = registrationEnd->next? entryNode next at 0
                    Link801EF* first = *(Link801EF**)registrationEnd; // next
                    // Read the next link from the embedded sentinel.
                }
            }
            // simpler: replicate assembly two-pass unlink+delete
            for (int pass = 0; pass < 2; ++pass) {
                void* registrationCleanupNode = AsShadowList801EF(registrations)->sentinelPrevious;
                void* registrationEnd = (void*)((char*)&AsShadowList801EF(registrations)->sentinelNext);
                if (registrationCleanupNode != registrationEnd) {
                    Link801EF* first = ((Link801EF*)registrationEnd)->next;
                    Link801EF* last = ((Link801EF*)registrationCleanupNode)->prev;
                    last->next = first->next;
                    first->next->prev = last;
                    while (registrationCleanupNode != registrationEnd) {
                        Link801EF* old = (Link801EF*)registrationCleanupNode;
                        registrationCleanupNode = old->prev;
                        appFreeUni__FPv(old);
                        --AsShadowList801EF(registrations)->count;
                    }
                    AsShadowList801EF(registrations)->sentinelPrevious = registrationCleanupNode; // Publish the retained tail link.
                }
            }
            __dl__FPv(registrations);
            g_shadowResourceList = 0;
        }
        // guarded outer cleanup (self+0x1C address check) -> always
        if ((void*)((char*)&AsCShadowManager(self)->entryCount) != 0) {
            Link801EF* entryCleanupNode = (*(Link801EF**)&AsCShadowManager(self)->listSentinelPrevious);
            Link801EF* sentinel = entrySentinel;
            if (entryCleanupNode != sentinel) {
                Link801EF* first = entrySentinel->next;
                Link801EF* last = entryCleanupNode->prev;
                last->next = first->next;
                first->next->prev = last;
                while (entryCleanupNode != entrySentinel) { Link801EF* old = entryCleanupNode; entryCleanupNode = entryCleanupNode->prev; appFreeUni__FPv(old); --AsCShadowManager(self)->entryCount; }
            }
        }
        if (self != 0) {
            g_shadowManager = 0;
            BaseObject_dtor(self, 0);
            if (deleteFlag > 0) __dl__FPv(self);
        }
    }
    return self;
}
#pragma pop
#pragma pop

} // extern "C"

