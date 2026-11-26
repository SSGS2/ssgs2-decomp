// Preloaders for game sequences, sprites, puppets and textures, in synchronous and asynchronous forms.
//
// CPreloadParent owns a list of child preloaders. CPreloadSeqAsync and CPreloadSeqSync load a script's
// file, its entries and their children. CPreloadSprAsync does the same for sprite entries.

#include <decomp/static_init.h>
#include "classes/CPreloadParent.h"
#include "classes/CPreloadSeqAsync.h"
#include "classes/CPreloadSeqSync.h"
#include "classes/CPreloadWPuppet.h"
#include "classes/CPreloadSeq.h"
#include "classes/CPreloadSpr.h"
#include "classes/CPreloadSprAsync.h"
#include "classes/CPreloadFileSync.h"
#include "classes/CPreloadFileAsync.h"
#include "classes/CPreloadTextureAsync.h"
#include "classes/CPreloadSprSync.h"
#include "classes/CPreloadWPuppetAsync.h"

#define AsCPreloadSeqAsync(p) ((CPreloadSeqAsync*)(p))
// Asynchronous sequence/sprite loading moves through these stages once per update.
enum PreloadState {
    kWaitForFile = 0,
    kReadScriptEntries = 1,
    kWaitForChildren = 2,
    kPreloadComplete = 3
};

template <class T> inline bool same_pos(const T& lhs, const T& rhs) {
    return lhs == rhs;
}

extern "C" {

void* __dt__11CPreloadSprFv(void*, int);
int SequencePreload_DestroyOwnedObjects(void*);
int SequencePreload_QueryFileCompletion(void*);
int SpritePreload_DestroyOwnedObjects(void*);
void appFreeUni__FPv(void*);
void* __nw__FUl(unsigned long);
extern char __vt__14CPreloadParent[];
extern char __vt__15CPreloadSeqSync[];
extern char __vt__16CPreloadSeqAsync[];
extern char __vt__16CPreloadSprAsync[];
extern float kFloat_6_2831855;
extern float s_preloadSeqAsyncFullTurn;
extern float s_preloadSeqSyncFullTurn;
extern float s_preloadSprAsyncFullTurn;
extern float s_preloadParentFullTurn;

struct Fn8027008CArg {
    char reservedPrefix[0x14];
    unsigned int loadState;
};

// Construct the child resource loader required by this script entry.
void* CPreloadSeqAsync::CreateFileAsync(void* arg) {
    return CPreloadFileAsync::Create(arg);
}
// Construct the child resource loader required by this script entry.
void* CPreloadSeqAsync::CreateSprAsync(void* arg) {
    return CPreloadSprAsync::Create(arg);
}
// Initialize the translation-unit full-turn angle cache.
void PreloadSeqAsync_InitFullTurnConstant() {
    s_preloadSeqAsyncFullTurn = kFloat_6_2831855;
}
// Synchronous preloaders are already updated and complete when queried.
int CPreloadSeqSync::IsLoadDone() {
    return 1;
}
// Its completion query answers the same way.
int CPreloadSeqSync::IsComplete() {
    return 1;
}
// Construct the child resource loader required by this script entry.
void* CPreloadSeqSync::CreateFileSync(void* arg) {
    return CPreloadFileSync::Create(arg);
}
// Construct the child resource loader required by this script entry.
void* CPreloadSeqSync::CreateSprSync(void* arg) {
    return CPreloadSprSync::Create((const char*)arg);
}
// Initialize the translation-unit full-turn angle cache.
void PreloadSeqSync_InitFullTurnConstant() {
    s_preloadSeqSyncFullTurn = kFloat_6_2831855;
}
// Construct the child resource loader required by this script entry.
void* CPreloadSprAsync::CreateFileAsync(void* arg) {
    return CPreloadFileAsync::Create(arg);
}
// Construct the child resource loader required by this script entry.
void* CPreloadSprAsync::CreateWPuppetAsync(void* arg) {
    return CPreloadWPuppetAsync::Create((const char*)arg);
}
// Construct the child resource loader required by this script entry.
void* CPreloadSprAsync::CreateTextureAsync(void* arg) {
    return CPreloadTextureAsync::Create(arg);
}
// Initialize the translation-unit full-turn angle cache.
void PreloadSprAsync_InitFullTurnConstant() {
    s_preloadSprAsyncFullTurn = kFloat_6_2831855;
}
// Report completion only after the file, script, and child stages finish.
int CPreloadSeqAsync::IsComplete(Fn8027008CArg* arg) {
    return arg->loadState == kPreloadComplete;
}
// Report completion only after the file, script, and child stages finish.
int CPreloadSprAsync::IsComplete(Fn8027008CArg* arg) {
    return arg->loadState == kPreloadComplete;
}
// Initialize the translation-unit full-turn angle cache.
void PreloadParent_InitFullTurnConstant() {
    s_preloadParentFullTurn = kFloat_6_2831855;
}

// Initialize an empty owned-preloader list and install the parent interface.
CPreloadParent::CPreloadParent() {
    ownedItems = 0;
    vtbl = __vt__14CPreloadParent;
    itemCount = 0;
    itemCapacity = 0;
}

// Destroy sequence-owned resources and optionally free this asynchronous preloader.
CPreloadSeqAsync::~CPreloadSeqAsync() {
    ((CPreloadSeq*)this)->Destroy(0);
}

// Destroy sequence-owned resources and optionally free this synchronous preloader.
CPreloadSeqSync::~CPreloadSeqSync() {
    ((CPreloadSeq*)this)->Destroy(0);
}

// Destroy sprite-owned resources and optionally free this asynchronous preloader.
CPreloadSprAsync::~CPreloadSprAsync() {
    __dt__11CPreloadSprFv(this, 0);
}

// Open a sequence, discard prior children, then run its script to completion.
int CPreloadSeqSync::Init(void* name) {
    if (((CPreloadSeq*)this)->OpenStreamFromPath(name) == 0)
        return 0;
    if (SequencePreload_DestroyOwnedObjects(this) == 0)
        return 0;
    while (ProcessScriptEntries() == 0) {
    }
    return 1;
}

struct Fn80270680Object {
    char reservedPrefix[4];
    void** items;
    unsigned int count;
};

class Fn80270680Item {
public:
    virtual void v0();
    virtual int check();
};

#pragma dont_inline on
#pragma optimization_level 2
// Update every non-null owned child; stop on the first failed update.
int OwnedObjectList_UpdateAll(Fn80270680Object* self) {
    void** item = self->items;
    // 0x80270680: inspect each item, reloading the end after callbacks.
    while (self->items + self->count != item) {
        {
            void* object = *item;
            if (object != 0) {
                if (((Fn80270680Item*)object)->check() == 0)
                    return 0;
            }
        }
        ++item;
    }
    return 1;
}
#pragma optimization_level reset
#pragma dont_inline off

struct CPreloadParentView {
    char reservedPrefix[4];
    void** items;
    unsigned int count;
};

class Fn802707Obj {
public:
    virtual void destroy(int);
    virtual void v4();
    virtual int check();
};

#pragma dont_inline on
#pragma optimization_level 2
// Return whether all non-null owned children report completion.
int OwnedObjectList_AreAllComplete(CPreloadParentView* self) {
    void** item = self->items;
    // 0x80270704: inspect each item, reloading the end after callbacks.
    while (self->items + self->count != item) {
        Fn802707Obj* object = (Fn802707Obj*)*item;
        if (object != 0) {
            if (object->check() == 0)
                return 0;
        }
        ++item;
    }
    return 1;
}
#pragma optimization_level reset
#pragma dont_inline off

class Fn802705ECObj {
public:
    virtual void destroy(int);
};

// Delete owned children, clear their slots, and reset the logical list length.
int OwnedObjectList_DestroyAll(CPreloadParentView* self) {
    void** item = self->items;
    unsigned int count;
    // 0x802705EC: retain the count loaded by the final bounds test.
    while (!same_pos(self->items + (count = self->count), item)) {
        Fn802705ECObj* object = (Fn802705ECObj*)*item;
        if (object != 0) {
            if (object != 0)
                object->destroy(1);
            *item = 0;
        }
        ++item;
    }
    self->count = count - *(unsigned int*)&self->count; // 0x802705EC: list count at +8
    return 1;
}

// Delete child preloaders, clear their slots, and reset the logical list length.
int PreloadObjectList_DestroyAll(CPreloadParentView* self) {
    void** item = self->items;
    Fn802705ECObj* object;
    while (!same_pos(self->items + self->count, item)) {
        object = (Fn802705ECObj*)*item;
        if (object != 0) {
            if (object != 0)
                object->destroy(1);
            *item = 0;
        }
        ++item;
    }
    self->count = *(unsigned int*)&self->count - self->count; // 0x80270788: retain the original count subtraction
    return 1;
}

// The factory calls these slots through the vtable it installs.
class SeqSyncInterface {
public:
    virtual void destroy(int);
    virtual void v0C();
    virtual void v10();
    virtual int init(void*);
};

// Allocate and initialize a synchronous sequence preloader; delete it if initialization fails.
void* CPreloadSeqSync::Create(void* arg) {
    CPreloadSeqSync* self = new CPreloadSeqSync;
    if (self) {
        ((CPreloadSeq*)self)->Init();
        self->vtbl = __vt__15CPreloadSeqSync;
    }
    if (!self)
        return 0;
    SeqSyncInterface* iface = (SeqSyncInterface*)self;
    if (!iface->init(arg)) {
        if (self)
            iface->destroy(1);
        return 0;
    }
    return self;
}

struct Fn8026FEF0Vtable {
    void* reservedRtti;
    void* reservedSlot;
    void (*dtor)(void*, int);
    void* update;
    void* isComplete;
    int (*init)(void*, void*);
};

struct Fn8026FEF0Obj {
    void* vtable;
    char reservedBaseFields[0x10];
    int loadState;
    Fn8026FEF0Obj();
};

inline Fn8026FEF0Obj::Fn8026FEF0Obj() {
    ((CPreloadSeq*)this)->Init();
    vtable = (void*)__vt__16CPreloadSeqAsync;
    AsCPreloadSeqAsync(this)->loadState = 0;
}

class Fn8026FEF0Virtual {
public:
    virtual void destroy(int);
    virtual void v0C();
    virtual void v10();
    virtual int init(void*);
};

// Allocate and initialize an asynchronous sequence preloader; delete it on failure.
void* CPreloadSeqAsync::Create(void* arg) {
    Fn8026FEF0Obj* self = new Fn8026FEF0Obj();
    if (!self)
        return 0;
    Fn8026FEF0Virtual* vself = (Fn8026FEF0Virtual*)self;
    if (!vself->init(arg)) {
        if (self)
            vself->destroy(1);
        return 0;
    }
    return self;
}

// Allocate and initialize an asynchronous sprite preloader; delete it on failure.
void* CPreloadSprAsync::Create(void* arg) {
    Fn8026FEF0Obj* self = (Fn8026FEF0Obj*)__nw__FUl(0x18);
    if (self) {
        ((CPreloadSpr*)self)->__ct();
        self->vtable = (void*)__vt__16CPreloadSprAsync;
        self->loadState = kWaitForFile;
    }
    if (!self)
        return 0;
    Fn8026FEF0Virtual* vself = (Fn8026FEF0Virtual*)self;
    if (!vself->init(arg)) {
        if (self)
            vself->destroy(1);
        return 0;
    }
    return self;
}

struct Fn8026FFA8Obj {
    char reservedPrefix[0x10];
    void* sourceFile; // 0x10: polled before advancing script/child work
    int loadState;    // 0x14: PreloadState
};

class Fn8026FFA8Sub {
public:
    virtual void v0();
    virtual void PollSource();
};

// Poll the source file, parse sequence entries, then wait for all child preloaders.
int CPreloadSeqAsync::UpdateLoadState() {
    Fn8026FFA8Obj* self = (Fn8026FFA8Obj*)this;
    if (self->sourceFile != 0) {
        ((Fn8026FFA8Sub*)self->sourceFile)->PollSource();
    }
    OwnedObjectList_UpdateAll((Fn80270680Object*)self);
    switch (self->loadState) {
    case kWaitForFile:
        if (SequencePreload_QueryFileCompletion(self) != 0) {
            if (SequencePreload_DestroyOwnedObjects(self) == 0)
                return 0;
            self->loadState = kReadScriptEntries;
        }
        break;
    case kReadScriptEntries:
        if (((CPreloadSeq*)self)->ProcessScriptEntries() != 0)
            self->loadState = kWaitForChildren;
        break;
    case kWaitForChildren:
        if (OwnedObjectList_AreAllComplete((CPreloadParentView*)self) != 0)
            self->loadState = kPreloadComplete;
        break;
    }
    return 1;
}

class Fn80270368Sub {
public:
    virtual void v0();
    virtual void PollSource();
};

// Poll the puppet source, parse sprite entries, then wait for all child preloaders.
int CPreloadSprAsync::UpdateLoadState() {
    Fn8026FFA8Obj* self = (Fn8026FFA8Obj*)this;
    if (self->sourceFile != 0) {
        ((Fn80270368Sub*)self->sourceFile)->PollSource();
    }
    OwnedObjectList_UpdateAll((Fn80270680Object*)self);
    switch (self->loadState) {
    case kWaitForFile:
        if (((CPreloadWPuppet*)self)->QueryFileCompletion() != 0) {
            if (SpritePreload_DestroyOwnedObjects(self) == 0)
                return 0;
            self->loadState = kReadScriptEntries;
        }
        break;
    case kReadScriptEntries:
        if (((CPreloadSpr*)self)->ProcessScriptEntries() != 0)
            self->loadState = kWaitForChildren;
        break;
    case kWaitForChildren:
        if (OwnedObjectList_AreAllComplete((CPreloadParentView*)self) != 0)
            self->loadState = kPreloadComplete;
        break;
    }
    return 1;
}

// Destroy owned children before freeing list storage and, if requested, the parent.
CPreloadParent::~CPreloadParent() {
    vtbl = (void*)__vt__14CPreloadParent;
    void** item = ownedItems;
    // Children are destroyed before the list's storage is freed.
    while (!same_pos(ownedItems + itemCount, item)) {
        {
            Fn802705ECObj* object = (Fn802705ECObj*)*item;
            if (object != 0) {
                if (object != 0)
                    object->destroy(1);
                *item = 0;
            }
        }
        ++item;
    }
    unsigned int count = itemCount;
    count = (volatile unsigned int)count - count;
    itemCount = count;
    // The repeated address tests reproduce the guard the original emits for the
    // owned list's inlined destructor.
    if ((int)this + 4 && (int)this + 4 && (int)this + 4 && (int)this + 4 && (int)this + 4) {
        if (ownedItems != 0) {
            itemCount = (volatile unsigned int)count - count;
            appFreeUni__FPv(ownedItems);
        }
    }
}

// Compile-time guards for private ABI views used above.
#define LOCAL_FIELD(t, f, o) typedef char local_##t##_##f[((int)&((t*)0)->f == (o)) ? 1 : -1]
LOCAL_FIELD(Fn8027008CArg, loadState, 0x14);
LOCAL_FIELD(Fn8026FEF0Obj, loadState, 0x14);
LOCAL_FIELD(Fn8026FFA8Obj, loadState, 0x14);
#undef LOCAL_FIELD

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462360", _ctors_u8026FEF0_0, PreloadSeqAsync_InitFullTurnConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462360", _ctors_u8026FEF0_1, PreloadSeqSync_InitFullTurnConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462360", _ctors_u8026FEF0_2, PreloadSprAsync_InitFullTurnConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462360", _ctors_u8026FEF0_3, PreloadParent_InitFullTurnConstant);
#pragma pop
