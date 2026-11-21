#include <decomp/static_init.h>
#include "classes/CPreloadSeqAsync.h"

#define AsCPreloadSeqAsync(p) ((CPreloadSeqAsync*)(p))
// Asynchronous sequence/sprite loading moves through these stages once per update.
enum PreloadState {
    kWaitForFile = 0,
    kReadScriptEntries = 1,
    kWaitForChildren = 2,
    kPreloadComplete = 3
};

template <class T> inline bool same_pos(const T& a, const T& b) { return a == b; }
// Translation unit 0x8026FEF0-0x80270828 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void* CPreloadFileAsync_Create(void*);
void* CPreloadWPuppetAsync_Create(void*);
void* CPreloadTextureAsync_Create(void*);
void* CPreloadFileSync_Create(void*);
void* CPreloadSprSync_Create(void*);
void* CPreloadSprAsync_Create(void*);
void* CPreloadSeq_dtor(void*, int);
void* CPreloadSpr_dtor(void*, int);
int CPreloadSeq_OpenStreamFromPath(void*, void*);
int SequencePreload_DestroyOwnedObjects(void*);
int CPreloadSeq_ProcessScriptEntries(void*);
int SequencePreload_QueryFileCompletion(void*);
int CPreloadWPuppet_QueryFileCompletion(void*);
int SpritePreload_DestroyOwnedObjects(void*);
int CPreloadSpr_ProcessScriptEntries(void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void* __nw__FUl(unsigned long);
void CPreloadSeq_Ctor(void*);
void CPreloadSpr_Ctor(void*);
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
void* CPreloadSeqAsync_CreateFileAsync(void*, void* arg) { return CPreloadFileAsync_Create(arg); }
// Construct the child resource loader required by this script entry.
void* CPreloadSeqAsync_CreateSprAsync(void*, void* arg) { return CPreloadSprAsync_Create(arg); }
// Initialize the translation-unit full-turn angle cache.
void PreloadSeqAsync_InitFullTurnConstant() { s_preloadSeqAsyncFullTurn = kFloat_6_2831855; }
// Synchronous preloaders are already updated and complete when queried.
int CPreloadSeqSync_IsLoadDone() { return 1; }
// Synchronous preloaders are already updated and complete when queried.
int CPreloadSeqSync_vfunc_2() { return 1; }
// Construct the child resource loader required by this script entry.
void* CPreloadSeqSync_CreateFileSync(void*, void* arg) { return CPreloadFileSync_Create(arg); }
// Construct the child resource loader required by this script entry.
void* CPreloadSeqSync_CreateSprSync(void*, void* arg) { return CPreloadSprSync_Create(arg); }
// Initialize the translation-unit full-turn angle cache.
void PreloadSeqSync_InitFullTurnConstant() { s_preloadSeqSyncFullTurn = kFloat_6_2831855; }
// Construct the child resource loader required by this script entry.
void* CPreloadSprAsync_CreateFileAsync(void*, void* arg) { return CPreloadFileAsync_Create(arg); }
// Construct the child resource loader required by this script entry.
void* CPreloadSprAsync_CreateWPuppetAsync(void*, void* arg) { return CPreloadWPuppetAsync_Create(arg); }
// Construct the child resource loader required by this script entry.
void* CPreloadSprAsync_CreateTextureAsync(void*, void* arg) { return CPreloadTextureAsync_Create(arg); }
// Initialize the translation-unit full-turn angle cache.
void PreloadSprAsync_InitFullTurnConstant() { s_preloadSprAsyncFullTurn = kFloat_6_2831855; }
// Report completion only after the file, script, and child stages finish.
int CPreloadSeqAsync_IsComplete(Fn8027008CArg* arg) { return arg->loadState == kPreloadComplete; }
// Report completion only after the file, script, and child stages finish.
int CPreloadSprAsync_IsComplete(Fn8027008CArg* arg) { return arg->loadState == kPreloadComplete; }
// Initialize the translation-unit full-turn angle cache.
void PreloadParent_InitFullTurnConstant() { s_preloadParentFullTurn = kFloat_6_2831855; }

struct Fn802705CCObject {
    char* vtable;
    unsigned int* ownedItems;
    unsigned int itemCount;
    unsigned int itemCapacity;
};

// Initialize an empty owned-preloader list and install the parent interface.
void CPreloadParent_Ctor(Fn802705CCObject* self) {
    self->ownedItems = 0;
    self->vtable = __vt__14CPreloadParent;
    self->itemCount = 0;
    self->itemCapacity = 0;
}

// Destroy sequence-owned resources and optionally free this asynchronous preloader.
void* CPreloadSeqAsync_dtor(void* self, int deleting) {
    if (self != 0) {
        CPreloadSeq_dtor(self, 0);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy sequence-owned resources and optionally free this synchronous preloader.
void* CPreloadSeqSync_dtor(void* self, int deleting) {
    if (self != 0) {
        CPreloadSeq_dtor(self, 0);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy sprite-owned resources and optionally free this asynchronous preloader.
void* CPreloadSprAsync_dtor(void* self, int deleting) {
    if (self != 0) {
        CPreloadSpr_dtor(self, 0);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Open a sequence, discard prior children, then run its script to completion.
int CPreloadSeqSync_Init(void* self, void* name) {
    if (CPreloadSeq_OpenStreamFromPath(self, name) == 0)
        return 0;
    if (SequencePreload_DestroyOwnedObjects(self) == 0)
        return 0;
    while (CPreloadSeq_ProcessScriptEntries(self) == 0) {}
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

struct Fn802707List {
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
int OwnedObjectList_AreAllComplete(Fn802707List* self) {
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
int OwnedObjectList_DestroyAll(Fn802707List* self) {
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
int PreloadObjectList_DestroyAll(Fn802707List* self) {
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

class Fn80270114Virtual {
public:
    virtual void v0();
    virtual void v4();
    virtual void destroy(int);
    virtual void v0C();
    virtual void v10();
    virtual int init(void*);
};

struct Fn80270114Obj {
    void* vtable;
    char reservedBaseFields[0x10];
};

struct SeqBaseReal {
    SeqBaseReal() { CPreloadSeq_Ctor(this); }
};
struct CPreloadSeqSync : SeqBaseReal {
    virtual void destroy(int);
    virtual void v0C();
    virtual void v10();
    virtual int init(void*);
    char fields[0x10];
    CPreloadSeqSync() {
    }
};

// Allocate and initialize a synchronous sequence preloader; delete it if initialization fails.
void* CPreloadSeqSync_Create(void* arg) {
    Fn80270114Obj* self = (Fn80270114Obj*)new CPreloadSeqSync;
    if (!self) return 0;
    Fn80270114Virtual* vself = (Fn80270114Virtual*)self;
    if (!((CPreloadSeqSync*)self)->init(arg)) {
        if (self) ((CPreloadSeqSync*)self)->destroy(1);
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
    CPreloadSeq_Ctor(this);
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
void* CPreloadSeqAsync_Create(void* arg) {
    Fn8026FEF0Obj* self = new Fn8026FEF0Obj();
    if (!self) return 0;
    Fn8026FEF0Virtual* vself = (Fn8026FEF0Virtual*)self;
    if (!vself->init(arg)) {
        if (self) vself->destroy(1);
        return 0;
    }
    return self;
}

// Allocate and initialize an asynchronous sprite preloader; delete it on failure.
void* CPreloadSprAsync_Create(void* arg) {
    Fn8026FEF0Obj* self = (Fn8026FEF0Obj*)__nw__FUl(0x18);
    if (self) {
        CPreloadSpr_Ctor(self);
        self->vtable = (void*)__vt__16CPreloadSprAsync;
        self->loadState = kWaitForFile;
    }
    if (!self) return 0;
    Fn8026FEF0Virtual* vself = (Fn8026FEF0Virtual*)self;
    if (!vself->init(arg)) {
        if (self) vself->destroy(1);
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
    virtual void fn0C();
};

// Poll the source file, parse sequence entries, then wait for all child preloaders.
int CPreloadSeqAsync_UpdateLoadState(Fn8026FFA8Obj* self) {
    if (self->sourceFile != 0) {
        ((Fn8026FFA8Sub*)self->sourceFile)->fn0C();
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
            if (CPreloadSeq_ProcessScriptEntries(self) != 0)
                self->loadState = kWaitForChildren;
            break;
        case kWaitForChildren:
            if (OwnedObjectList_AreAllComplete((Fn802707List*)self) != 0)
                self->loadState = kPreloadComplete;
            break;
    }
    return 1;
}

class Fn80270368Sub {
public:
    virtual void v0();
    virtual void fn0C();
};

// Poll the puppet source, parse sprite entries, then wait for all child preloaders.
int CPreloadSprAsync_UpdateLoadState(Fn8026FFA8Obj* self) {
    if (self->sourceFile != 0) {
        ((Fn80270368Sub*)self->sourceFile)->fn0C();
    }
    OwnedObjectList_UpdateAll((Fn80270680Object*)self);
    switch (self->loadState) {
        case kWaitForFile:
            if (CPreloadWPuppet_QueryFileCompletion(self) != 0) {
                if (SpritePreload_DestroyOwnedObjects(self) == 0)
                    return 0;
                self->loadState = kReadScriptEntries;
            }
            break;
        case kReadScriptEntries:
            if (CPreloadSpr_ProcessScriptEntries(self) != 0)
                self->loadState = kWaitForChildren;
            break;
        case kWaitForChildren:
            if (OwnedObjectList_AreAllComplete((Fn802707List*)self) != 0)
                self->loadState = kPreloadComplete;
            break;
    }
    return 1;
}

struct Fn802704DCObj {
    void* vtable;
    void** items;
    unsigned int count;
};

struct Fn802704DCVtable {
    char reservedPrefix[0x08];
    void (*destroy)(void*, int);
};

// Destroy owned children before freeing list storage and, if requested, the parent.
void* CPreloadParent_dtor(Fn802704DCObj* self, int deleting) {
    if (self != 0) {
        self->vtable = (void*)__vt__14CPreloadParent;
        void** item = self->items;
        // 0x802704DC: destroy the list before freeing its storage.
        while (!same_pos(self->items + self->count, item)) {
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
        unsigned int count = self->count;
        count = (volatile unsigned int)count - count;
        self->count = count;
        if ((int)self + 4 && (int)self + 4 && (int)self + 4 && (int)self + 4 && (int)self + 4) {
            if (self->items != 0) {
                self->count = (volatile unsigned int)count - count;
                appFreeUni__FPv(self->items);
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Compile-time guards for private ABI views used above.
#define LOCAL_FIELD(t, f, o) typedef char local_##t##_##f[((int)&((t*)0)->f == (o)) ? 1 : -1]
LOCAL_FIELD(Fn8027008CArg, loadState, 0x14);
LOCAL_FIELD(Fn802705CCObject, ownedItems, 4);
LOCAL_FIELD(Fn802705CCObject, itemCount, 8);
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
