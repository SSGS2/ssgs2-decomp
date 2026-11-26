// Story cutscene (CStoryConte): event/model loading, shared messages and sound
// bank lifetime, scene playback, skip fade, and upper-right skip-caption drawing.
// Modding lookup (frame rate / UI counters / models / widescreen):
// - CStoryConte::OnProcess, kStorySkipFade: skipFadeUpdateCount counts PROCESS CALLS,
//   not video retraces or milliseconds. kSkipFadeScaledUpdates=15 and
//   kSkipFadeDefaultUpdates=18 select duration-scaling policy (25/30 main-frame
//   updates/s respectively, main_frame_letter_and_mouse_cursor.cpp); they are not 50/60-Hz constants.
//   Alpha is truncated float(counter*255)/selectedLimit. Keep completion and
//   divisor thresholds paired, counter preincrement and later cleanup return.
// - CStoryConte::OnDisplay: kFloat_640_StoryConte is the reference UI width;
//   right margin=20, top=48, width=128, height=32 come from named bit-pattern
//   constants in the data section. Widescreen edits need this UI reference and
//   input/layout consumers coordinated, not just a physical framebuffer width.
//   Format 0x11F005/24-byte stride belongs to the four-vertex caption template.
// - OnPreLoadInit/OnLoad/ProcessPlayerEventScripts resolve player event names,
//   append the pooled object extension, and create loaders; HandleMsg controls
//   shared model-slot lifetime (five 0x50-byte slots, array runtime cookie 0x10).
//   Event files use little-endian byte offsets; never replace them with pointers.
//   Model source names and face-data buffers must outlive their script handles.
// State flow and gates are beside OnProcess. It may fall through multiple ready
// stages in one call; pending polls return without advancing. Blocking setup
// loops have no timeout/status propagation. No simulation or presentation rate
// is set here. The frame-rate priority row covers skip fade, not every script.
// Shared layouts: CStoryConte.h, CEventHandle.h, CEventLoad.h and local phase
// views of the SAME storage below. Story task state +0x1C is distinct from the
// event index +0x20 (also used by the shutdown view).
// CEventHandle overlays are access views, not inheritance.
// Lifetimes: task owns event/face loaders, active handle and caption. Handle owns
// command handlers/container allocations and borrows source buffers. OnProcess
// destroys handle before playback cleanup; OnDestroy releases loaders/handle/
// caption in its existing order, then shared message resources. Call OnDestroy
// before the task dtor, which frees strings/interfaces only. Shared singletons
// and model-slot table are borrowed; HandleMsg ENTER/LEAVE owns their sessions.
// All operations require serial lifecycle use. Preserve volatile singleton
// samples, registration storage, constructor publication, destructor flags,
// tree sentinel checks, endian conversion and all floating-point operations.
#include "MSL/vector.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
typedef std::vector<void*, app::app_allocator<void*> > MSLPointerVector;
// String-pool view; byte lengths include terminators and original padding.
struct StoryStringPool {
    char headBone[11];        // +0x0
    char emptyPathPrefix[1];  // +0xB
    char objectExtension[5];  // +0xC
    char scriptOwnerName[11]; // +0x11
    char skipCaptionFile[19]; // +0x1C
    char soundBank[9];        // +0x2F
};
#define StoryStrings (*((StoryStringPool*)storyConteStrings))

#define FIELD_OFFSET(type, field) ((int)&((type*)0)->field)
#include "classes/CEventLoad.h"
#include "classes/EventScriptByteTreeNode.h"
#define AsCEventLoad(p) ((CEventLoad*)(p))
#include "game/string.h"
#include "game/eventconte.h"

#include "game/singletons.h"
#include "classes/CEventHandle.h"
#include "classes/CEventHandleTreeIterator.h"
#include "classes/CEventEffect.h"
#include "classes/CStoryConte.h"
#include "classes/CEventConteProc.h"
#include <decomp/cw_bitfields.h>
#include "classes/CBpPlayer.h"
#include "classes/CB2dPlayerHandle.h"
#include "classes/CNowReloading.h"
#include "classes/CEffectFade.h"
#include "classes/CBpTex.h"

#define AsCEventConteProc(p) ((CEventConteProc*)(p))

// One call site passes a second pointer that CEventEffect::Init ignores; keep it
// by calling the initializer through its symbol.
extern "C" void* Init__12CEventEffectFv(CEventEffect* self, void* ignored);

#define AsCStoryConte(p) ((CStoryConte*)(p))

#define AsCEventHandle(p) ((CEventHandle*)(p))
enum StoryState {
    kStoryIdle = 0,
    kStoryLoading = 1,
    kStoryWaitForSkipCaption = 2,
    kStoryPlaying = 3,
    kStoryCleanup = 4,
    kStorySkipFade = 5,
    kStoryFinished = 6
};
enum StoryMessage {
    kStoryEnter = 0,
    kStoryLeave = 1,
    kStoryStart = 2,
    kStoryQueryComplete = 3,
    kStoryQueryEventNames = 4
};
// These thresholds select duration-scaling policy, not video refresh rate:
// the main-frame clock also selects 25 versus 30 updates/s with the same query.
// Both baselines are 0.6 s at those update rates; increasing a threshold slows
// the skip fade. A later frame rate mod must scale BOTH thresholds and keep the
// alpha divisor equal to the selected completion threshold (two query calls).
enum {
    kStoryPlayerCount = 2,
    kStoryModelSlotCount = 5,
    kSkipFadeScaledUpdates = 15,
    kSkipFadeDefaultUpdates = 18,
    kSkipActionPrimary = 0x11,
    kSkipActionSecondary = 0x12,
    kStoryOpaqueAlpha = 255,
    kStoryCaptionVertexFormat = 0x11F005,
    kStoryCaptionStrip = 0x98,
    kStoryCaptionVertexCount = 4,
    kStoryCaptionVertexBytes = 0x18,
    kStoryScriptHandleBytes = 0x4A8,
    kStoryFileLoaderBytes = 0x30,
    kStoryModelSlotBytes = 0x50,
    kStoryModelArrayCookieBytes = 0x10,
    kStoryModelArrayBytes = kStoryModelSlotCount * kStoryModelSlotBytes + kStoryModelArrayCookieBytes
};

struct ScriptObjectVtable20 {
    char reserved00[0x20];
    void* vtable;
};
struct ScriptObjectVtable30 {
    char reserved00[0x30];
    void* vtable;
};
struct BoneControllerView {
    char reserved00[0x68];
    void* controller;
};
enum StoryVirtualTableConstants {
    kTaskMainVtableOffset = 0x3C
};

extern "C" {
void* __dl__FPv(void*);

struct StoryPathString {
    union {
        unsigned int flags;
        unsigned char shortData[4];
    };
    unsigned int longLength;
};
struct StoryLoadView {
    unsigned char pad00[0x24];
    StoryPathString string;
    unsigned char pad2C[4];
    void* eventLoader;
    unsigned char pad34[4];
    void* faceDataLoader;
    unsigned char pad3C[0x60];
    void* skipCaption;
};
extern char* SharedStringState_GetFirstString(int);
extern void String_ReplaceRange(StoryPathString*, int, int, char*, char*, void*);
extern void __ct__9CBpPlayerFv(void*);
extern void __dt__9CBpPlayerFv();
extern void __register_global_object(void*, void (*)(), void*);
extern int EventText_PollMessageResources(void*);
extern void* __nw__FUl(unsigned int);
extern int GameHook_ReservedFalse(); // Optional external-file loading hook; the shipped implementation returns false.
extern int StoryConte_AreRequiredFilesAvailable();
extern int g_eventFaceDataFilename;
extern signed char s_bpPlayerInitialized;
extern unsigned char storyConteStrings[];
extern unsigned int __vt__12CEventHandle[];
extern unsigned char s_bpPlayer[];
extern unsigned char s_conteModelSlots[];
extern unsigned char s_partyBpPlayerDtorRegistration[];
extern unsigned char s_storyConteDtorRegistrations[];
extern unsigned char s_effectFade[];
extern unsigned int strlen(const char*);
extern void* memset(void*, int, unsigned int);
extern void ScriptSharedFiles_ResetRequiredFilesAndHandlers(void*);
extern void ScriptState_RunSharedFileCommandsUntilComplete(void*, int);
extern void ScriptByteMap_FreeSubtree(void*, void*);
extern void appFreeUni__FPv(void*);
extern unsigned char storyConteStrings[];

struct AsyncLoadInterface {
    unsigned char pad[0x24];
    virtual void Destroy(int);
};
struct ResourceInterface {
    virtual void Destroy(int);
};
// Return the length from inline or heap-backed narrow-string storage.
static inline unsigned int StoryPathLength(StoryPathString* str) {
    return !(str->flags >> 31) ? str->shortData[0] & 0x7f : str->longLength;
}
#pragma push
#pragma use_lmw_stmw on
// Resolve an event path, wait for its resources, and execute both player setup scripts.
// Required task; prefers player 0 then 1. No path destroys loaders/caption and
// sets idle. Otherwise face loader is required; loader/player/file readiness may
// block indefinitely. Shared file hook can return early before setup. Path string
// is task-owned, resource buffers remain loader-owned; no completion result.
void CStoryConte::OnLoad() {
    StoryLoadView* self = (StoryLoadView*)this;
    // Phase 1: prefer the first player path, then the second; append the object extension.
    char temps[0x30];
    char* text = SharedStringState_GetFirstString(0);
    char* data;
    if (text == 0) {
        data = 0;
    } else {
        register char* prefix = StoryStrings.emptyPathPrefix;
        unsigned int size = StoryPathLength(&self->string);
        temps[28] = temps[24];
        String_ReplaceRange(&self->string, 0, size, prefix, strlen(prefix) + prefix, &temps[28]);
        unsigned int size2 = StoryPathLength(&self->string);
        temps[36] = temps[32];
        String_ReplaceRange(&self->string, size2, 0, text, text + strlen(text), &temps[36]);
        register char* extension = StoryStrings.objectExtension;
        unsigned int size3 = StoryPathLength(&self->string);
        temps[44] = temps[40];
        String_ReplaceRange(&self->string, size3, 0, extension, strlen(extension) + extension, &temps[44]);
        data = !(self->string.flags >> 31) ? (char*)&AsCStoryConte(self)->inlineEventPath
                                           : (*(char**)&AsCStoryConte(self)->eventPathData);
    }
    if (data == 0) {
        char* secondText = SharedStringState_GetFirstString(1);
        if (secondText == 0) {
            data = 0;
        } else {
            register char* prefix = StoryStrings.emptyPathPrefix;
            unsigned int size = StoryPathLength(&self->string);
            temps[4] = temps[0];
            String_ReplaceRange(&self->string, 0, size, prefix, strlen(prefix) + prefix, &temps[4]);
            unsigned int size2 = StoryPathLength(&self->string);
            temps[12] = temps[8];
            String_ReplaceRange(&self->string, size2, 0, secondText, secondText + strlen(secondText), &temps[12]);
            register char* extension = StoryStrings.objectExtension;
            unsigned int size3 = StoryPathLength(&self->string);
            temps[20] = temps[16];
            String_ReplaceRange(&self->string, size3, 0, extension, strlen(extension) + extension, &temps[20]);
            data = !(self->string.flags >> 31) ? (char*)&AsCStoryConte(self)->inlineEventPath
                                               : (*(char**)&AsCStoryConte(self)->eventPathData);
        }
    }
    // Phase 2: no path tears down loaders/caption; a path polls resources and runs setup.
    if (data == 0) {
        if (self->eventLoader != 0) {
            typedef void (*Destroy)(void*, int);
            if (self->eventLoader != 0) {
                ((AsyncLoadInterface*)self->eventLoader)->Destroy(1);
            }
            self->eventLoader = 0;
        }
        if (self->faceDataLoader != 0) {
            typedef void (*Destroy)(void*, int);
            if (self->faceDataLoader != 0) {
                ((AsyncLoadInterface*)self->faceDataLoader)->Destroy(1);
            }
            self->faceDataLoader = 0;
        }
        if (self->skipCaption != 0) {
            typedef void (*Destroy)(void*, int);
            if (self->skipCaption != 0) {
                ((ResourceInterface*)self->skipCaption)->Destroy(1);
            }
            self->skipCaption = 0;
        }
        AsCStoryConte(self)->state = 0;
    } else {
        if (self->eventLoader != 0) {
            ((CEventLoad*)(self->eventLoader))->Poll();
        }
        ((CEventLoad*)(self->faceDataLoader))->Poll();
        if (((volatile int&)Sing_805DC670()) == 0) {
            if (s_bpPlayerInitialized == 0) {
                ((CBpPlayer*)s_bpPlayer)->__ct();
                __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, s_partyBpPlayerDtorRegistration);
                s_bpPlayerInitialized = 1;
            }
            ((volatile int&)Sing_805DC670()) = (int)s_bpPlayer;
        }
        EventText_PollMessageResources((void*)((volatile int&)Sing_805DC670()));
        for (;;) {
            if (((volatile int&)Sing_805DC670()) == 0) {
                if (s_bpPlayerInitialized == 0) {
                    ((CBpPlayer*)s_bpPlayer)->__ct();
                    __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, s_partyBpPlayerDtorRegistration);
                    s_bpPlayerInitialized = 1;
                }
                ((volatile int&)Sing_805DC670()) = (int)s_bpPlayer;
            }
            if (((CBpPlayer*)(((volatile int&)Sing_805DC670())))->PollResourceLoad() != 0)
                break;
        }
        if (GameHook_ReservedFalse() != 0)
            return;
        ProcessPlayerEventScripts(self, 1);
        while (StoryConte_AreRequiredFilesAvailable() == 0) {
        }
    }
}

#pragma pop
struct StoryScriptStorage {
    unsigned char data[kStoryScriptHandleBytes];
};

// Release both child subtrees, then free this tree node.
static inline void cleanLeaf(void* list, void* n) {
    void* child = *(void**)n;
    if (child != 0)
        ScriptByteMap_FreeSubtree(list, child);
    child = AsEventScriptByteTreeNode(n)->right;
    if (child != 0)
        ScriptByteMap_FreeSubtree(list, child);
    appFreeUni__FPv(n);
}
// Free two levels of nodes before delegating deeper branches to the shared tree cleanup.
static inline void cleanLevel1(void* list, void* n) {
    void* child = *(void**)n;
    if (child != 0)
        cleanLeaf(list, child);
    child = AsEventScriptByteTreeNode(n)->right;
    if (child != 0)
        cleanLeaf(list, child);
    appFreeUni__FPv(n);
}
// Free three levels of nodes before delegating deeper branches to the shared tree cleanup.
static inline void cleanLevel2(void* list, void* n) {
    void* child = *(void**)n;
    if (child != 0)
        cleanLevel1(list, child);
    child = AsEventScriptByteTreeNode(n)->right;
    if (child != 0)
        cleanLevel1(list, child);
    appFreeUni__FPv(n);
}
// Destroy a subtree in postorder using the original unrolled cleanup depth.
static inline void cleanTree(void* list, void* n) {
    void* child = *(void**)n;
    if (child != 0)
        cleanLevel2(list, child);
    child = AsEventScriptByteTreeNode(n)->right;
    if (child != 0)
        cleanLevel2(list, child);
    appFreeUni__FPv(n);
}

// The five owned handlers and five pointer lists retain separate storage/lifetimes.
// Field view of the 0x4A8-byte CEventHandle (the object ~CEventHandle tears down), used for script-handle
// set-up in ProcessPlayerEventScripts and by the destructor helpers below.
// These are disjoint regions of ONE handle, not a CStoryConte base subobject:
// +000..007 flags/condition counters; +008..407 256 script-variable words;
// +408..413 owning tagged string; +414 borrowed script base; +418 cursor;
// +41C elapsed command ticks; +420..430 pending handlers; +434..46F queues;
// +470 model-event map; +47C embedded effect map; +48C pending face alias;
// +490 retained face map; +49C facial script base; +4A0 manager; +4A4 vtable.
// The two memsets clear ONLY +000..407, ending just before name at +408.
// They neither erase the newly initialized name/containers nor end their lifetimes.
// CEventHandle.h also carries task-phase fields in this prefix: those are views
// of a STORY TASK at self, not live task fields/vtable inside an interpreter.
// In particular handle+0 is a flag word; its actual vtable is handle+4A4.
// The resets of already-empty queues/maps preserve constructor/reset ordering.
// No payload has been published yet; clearing counts here leaks no event.
//
// Payload -> pending handler -> published queue / cleanup role map:
// camera commands -> no established pending-handler slot / list0 +434 cameras;
// +420 handler0 is owned but has no opcode producer found in event_conte_script_interpreter.cpp;
// SC_MDL_MOT/MOVE -> handler1 +424 model event / list1 +440 model events;
// effect-fade commands -> handler2 +428 effect event / list2 +44C effects;
// SC_MSG_REQ -> handler3 +42C message request / list3 +458 messages;
// sound commands -> handler4 +430 sound event / list4 +464 sounds.
// Command definitions, ScriptQueueView and ScriptInterpreterStorage in event_conte_script_interpreter.cpp
// provide the producer schemas. Each list is {owned wrapper-pointer buffer,
// size, capacity}; wrappers carry elapsed polling ticks and an owned callback.
// Async publication transfers pending-event ownership into the wrapper; a
// synchronous command keeps its pending event until completion. Keyed model
// moves can instead publish into +470, separate from the model queue.
// EventScript_RunAndSweep/SweepPendingCallbacks poll these collections;
// ~CEventHandle deletes pending handlers, then queued wrappers, map payloads,
// then releases vector/tree storage (and the embedded effect map/name). Script
// buffers and manager are borrowed throughout; face objects are separately
// retained/released by the resource-map path, not freed with variable words.
struct StoryScriptInitView {
    unsigned char pad000[0x408];
    StoryPathString name;             // 0x408
    unsigned int ownerNameData;       // 0x410
    void* sourceBuffer;               // 0x414
    void* commandCursor;              // 0x418
    unsigned int commandCount;        // 0x41c
    unsigned int ownedHandler0;       // 0x420
    unsigned int ownedHandler1;       // 0x424
    unsigned int ownedHandler2;       // 0x428
    unsigned int ownedHandler3;       // 0x42c
    unsigned int ownedHandler4;       // 0x430
    unsigned int objectList0Data;     // 0x434
    unsigned int objectList0Count;    // 0x438
    unsigned int objectList0Capacity; // 0x43c
    unsigned int objectList1Data;     // 0x440
    unsigned int objectList1Count;    // 0x444
    unsigned int objectList1Capacity; // 0x448
    unsigned int objectList2Data;     // 0x44c
    unsigned int objectList2Count;    // 0x450
    unsigned int objectList2Capacity; // 0x454
    unsigned int objectList3Data;     // 0x458
    unsigned int objectList3Count;    // 0x45c
    unsigned int objectList3Capacity; // 0x460
    unsigned int objectList4Data;     // 0x464
    unsigned int objectList4Count;    // 0x468
    unsigned int objectList4Capacity; // 0x46c
    unsigned int objectTreeCount;     // 0x470
    void* objectTreeRoot;             // 0x474
    void** objectTreeEnd;             // 0x478
    unsigned char pad47c[0x10];       // 0x47c
    unsigned int pendingResource;     // 0x48c
    unsigned int resourceTreeCount;   // 0x490
    void* resourceTreeRoot;           // 0x494
    void** resourceTreeEnd;           // 0x498
    unsigned int faceScriptTableBase; // 0x49c
    unsigned int manager;             // 0x4a0
    void** vtable;                    // 0x4a4
};

#pragma push
#pragma use_lmw_stmw on
// Build a temporary script handle for each player and run its shared-file commands.
// Required task; null eventLoader returns after resetting shared requirements.
// loadMode is Boolean: 0 requests async file creation, nonzero synchronous file
// creation on a cache miss. OnLoad passes 1. The script runner (event_conte_script_interpreter.cpp) reduces
// it to (loadMode!=0), registers every requested path regardless of that value,
// and passes it through SharedFileList_CreateAndAppend -> SharedFile_Create ->
// SharedFileCache_GetOrCreate (shared_file_loading.cpp/fade_overlay_and_shared_file.cpp/shared_file_registry.cpp). Cache hits reuse and
// retain the existing file without changing its load mode. This is not a command
// filter or a promise that an already asynchronous cache hit is complete.
// For each nonnull player path, constructs an owning 0x4A8-byte handle over borrowed
// event bytes, runs commands synchronously, destroys it. Buffer must contain both
// player table entries and valid little-endian offsets; no file-length validation.
// Allocation failure is not recovered here. Preserve player order and per-player
// string/constructor resets; they can affect shared requirement registration.
// The task is passed explicitly; declaring this non-static changes the argument registers.
void CStoryConte::ProcessPlayerEventScripts(StoryLoadView* self, int loadMode) {
    char temps[0x24];
    unsigned int i;
    char* text;
    StoryScriptInitView* obj;
    char* data;
    char* sepBase;

    // Phase 1: reset shared-file requirements, then resolve each player path independently.
    ScriptSharedFiles_ResetRequiredFilesAndHandlers(self);
    if (self->eventLoader == 0)
        return;
    sepBase = (char*)storyConteStrings;
    for (i = 0; 1 >= i; i++) {
        text = SharedStringState_GetFirstString((unsigned char)i);
        if (text == 0) {
            data = 0;
        } else {
            unsigned int size;
            char* sep;
            sep = sepBase + FIELD_OFFSET(StoryStringPool, emptyPathPrefix);
            size = !(self->string.flags >> 31) ? self->string.shortData[0] & 0x7f : self->string.longLength;
            temps[0xc] = temps[8];
            String_ReplaceRange(&self->string, 0, (int)size, sep, sep + strlen(sep), &temps[0xc]);
            size = !(self->string.flags >> 31) ? self->string.shortData[0] & 0x7f : self->string.longLength;
            temps[0x14] = temps[0x10];
            String_ReplaceRange(&self->string, (int)size, 0, text, text + strlen(text), &temps[0x14]);
            sep = sepBase + FIELD_OFFSET(StoryStringPool, objectExtension);
            size = !(self->string.flags >> 31) ? self->string.shortData[0] & 0x7f : self->string.longLength;
            temps[0x1c] = temps[0x18];
            String_ReplaceRange(&self->string, (int)size, 0, sep, sep + strlen(sep), &temps[0x1c]);
            data = !(self->string.flags >> 31) ? (char*)&self->string.shortData[1]
                                               : (*(char**)&AsCEventHandle(self)->eventPathData);
        }
        if (data != 0) {
            unsigned int packed;
            char* selected;
            char* table;

            void** tail;
            void* sub;
            char* sep;
            // Phase 2: construct the temporary handle, its owned lists/trees and borrowed script cursor.
            obj = (StoryScriptInitView*)__nw__FUl(kStoryScriptHandleBytes);
            if (0 != obj) {
                tail = &obj->objectTreeRoot;
                table = (char*)AsCEventLoad(self->eventLoader)->buffer;
                packed = ((unsigned int*)table)[(unsigned char)i];
                sep = sepBase + FIELD_OFFSET(StoryStringPool, scriptOwnerName);
                sub = (char*)&obj->pad47c;
                obj->vtable = (void**)__vt__12CEventHandle;
                obj->name.flags = 0;
                obj->name.longLength = 0;
                obj->ownerNameData = 0;
                obj->objectList0Data = 0;
                obj->objectList0Count = 0;
                obj->objectList0Capacity = 0;
                obj->objectList1Data = 0;
                obj->objectList1Count = 0;
                obj->objectList1Capacity = 0;
                obj->objectList2Data = 0;
                obj->objectList2Count = 0;
                obj->objectList2Capacity = 0;
                obj->objectList3Data = 0;
                obj->objectList3Count = 0;
                obj->objectList3Capacity = 0;
                obj->objectList4Data = 0;
                obj->objectList4Count = 0;
                *(unsigned int*)(temps + 0x20) = packed;
                obj->objectList4Capacity = 0;
                {
                    unsigned char b0 = temps[0x20];
                    unsigned char b3 = temps[0x23];
                    obj->objectTreeCount = 0;
                    char b1 = temps[0x21];
                    unsigned char b2 = temps[0x22];
                    obj->objectTreeRoot = 0;
                    obj->objectTreeEnd = tail;
                    temps[0x20] = b3;
                    temps[0x23] = b0;
                    temps[0x21] = b2;
                    temps[0x22] = b1;
                }
                selected = table + *(unsigned int*)(temps + 0x20);
                ((CEventEffect*)sub)->Init();
                obj->resourceTreeCount = 0;
                obj->resourceTreeRoot = 0;
                obj->resourceTreeEnd = &obj->resourceTreeRoot;
                {
                    unsigned int size;
                    size = !(obj->name.flags >> 31) ? obj->name.shortData[0] & 0x7f : obj->name.longLength;
                    temps[4] = temps[0];
                    String_ReplaceRange(&obj->name, 0, (int)size, sep, sep + strlen(sep), &temps[4]);
                }
                obj->sourceBuffer = table;
                obj->commandCursor = selected;
                memset(obj, 0, 8);
                memset((char*)obj + 8, 0, 0x400);
                obj->commandCount = 0;
                obj->ownedHandler0 = 0;
                obj->ownedHandler1 = 0;
                obj->ownedHandler2 = 0;
                obj->ownedHandler3 = 0;
                obj->ownedHandler4 = 0;
                {
                    volatile int* _p = (volatile int*)&obj->objectList0Count;
                    int _o = *_p;
                    union {
                        int a;
                        unsigned int u;
                    } _b;
                    _b.a = _o;
                    *_p = _o - _b.u;
                }
                {
                    volatile int* _p = (volatile int*)&obj->objectList1Count;
                    int _o = *_p;
                    union {
                        int a;
                        unsigned int u;
                    } _b;
                    _b.a = _o;
                    *_p = _o - _b.u;
                }
                {
                    volatile int* _p = (volatile int*)&obj->objectList2Count;
                    int _o = *_p;
                    union {
                        int a;
                        unsigned int u;
                    } _b;
                    _b.a = _o;
                    *_p = _o - _b.u;
                }
                {
                    volatile int* _p = (volatile int*)&obj->objectList3Count;
                    int _o = *_p;
                    union {
                        int a;
                        unsigned int u;
                    } _b;
                    _b.a = _o;
                    *_p = _o - _b.u;
                }
                {
                    volatile int* _p = (volatile int*)&obj->objectList4Count;
                    int _o = *_p;
                    union {
                        int a;
                        unsigned int u;
                    } _b;
                    _b.a = _o;
                    *_p = _o - _b.u;
                }
                if (obj->objectTreeRoot != 0) {
                    cleanTree((char*)&obj->objectTreeCount, obj->objectTreeRoot);
                    obj->objectTreeCount = 0;
                    obj->objectTreeRoot = 0;
                    obj->objectTreeEnd = &obj->objectTreeRoot;
                }
                obj->pendingResource = 0;
                if (obj->resourceTreeRoot != 0) {
                    cleanTree((char*)&obj->resourceTreeCount, (void*)obj->resourceTreeRoot);
                    obj->resourceTreeCount = 0;
                    obj->resourceTreeRoot = 0;
                    obj->resourceTreeEnd = &obj->resourceTreeRoot;
                }
                obj->faceScriptTableBase = 0;
                obj->manager = 0;
            }
            ScriptState_RunSharedFileCommandsUntilComplete(obj, loadMode);
            if (obj != 0) {
                if (obj != 0) {
                    typedef void (*Destroy)(void*, int);
                    Destroy destroy = *(Destroy*)((char*)obj->vtable + 8);
                    destroy(obj, 1);
                }
            }
        }
    }
}

#pragma pop

// Create the event, face-data, message and skip-caption resource loaders.
// Required task; replaces owned loaders after path resolution, then queues shared
// messages/caption. Constructor exceptions free their raw allocation and rethrow;
// previous resources already released stay released (no transactional rollback).
void CStoryConte::OnPreLoadInit() {
    StoryLoadView* self = (StoryLoadView*)this;
    // Phase 1: resolve the event path before replacing any asynchronous loaders.
    char temps[0x34];
    char* text = SharedStringState_GetFirstString(0);
    char* data;
    if (text == 0) {
        data = 0;
    } else {
        unsigned int size;
        char* start = StoryStrings.emptyPathPrefix;
        size = !(self->string.flags >> 31) ? self->string.shortData[0] & 0x7f : self->string.longLength;
        temps[28] = temps[24];
        String_ReplaceRange(&self->string, 0, size, start, start + strlen(start), &temps[28]);
        size = !(self->string.flags >> 31) ? self->string.shortData[0] & 0x7f : self->string.longLength;
        temps[36] = temps[32];
        String_ReplaceRange(&self->string, size, 0, text, text + strlen(text), &temps[36]);
        start = StoryStrings.objectExtension;
        size = !(self->string.flags >> 31) ? self->string.shortData[0] & 0x7f : self->string.longLength;
        temps[44] = temps[40];
        String_ReplaceRange(&self->string, size, 0, start, start + strlen(start), &temps[44]);
        data = self->string.flags >> 31 ? (*(char**)&AsCStoryConte(self)->eventPathData)
                                        : (char*)&AsCStoryConte(self)->inlineEventPath;
    }
    if (data == 0) {
        char* text2 = SharedStringState_GetFirstString(1);
        if (text2 == 0) {
            data = 0;
        } else {
            unsigned int size;
            char* start = StoryStrings.emptyPathPrefix;
            size = !(self->string.flags >> 31) ? self->string.shortData[0] & 0x7f : self->string.longLength;
            temps[4] = temps[0];
            String_ReplaceRange(&self->string, 0, size, start, start + strlen(start), &temps[4]);
            size = !(self->string.flags >> 31) ? self->string.shortData[0] & 0x7f : self->string.longLength;
            temps[12] = temps[8];
            String_ReplaceRange(&self->string, size, 0, text2, text2 + strlen(text2), &temps[12]);
            start = StoryStrings.objectExtension;
            size = !(self->string.flags >> 31) ? self->string.shortData[0] & 0x7f : self->string.longLength;
            temps[20] = temps[16];
            String_ReplaceRange(&self->string, size, 0, start, start + strlen(start), &temps[20]);
            data = self->string.flags >> 31 ? (*(char**)&AsCStoryConte(self)->eventPathData)
                                            : (char*)&AsCStoryConte(self)->inlineEventPath;
        }
    }
    typedef void (*Destroy)(void*, int);
    if (data == 0) {
        void* item = self->eventLoader;
        if (item != 0) {
            if (item != 0) {
                Destroy destroy = *(Destroy*)((unsigned int)AsCEventLoad(item)->vtable + 8);
                destroy(item, 1);
            }
            self->eventLoader = 0;
        }
        item = self->faceDataLoader;
        if (item != 0) {
            if (item != 0) {
                Destroy destroy = *(Destroy*)((unsigned int)AsCEventLoad(item)->vtable + 8);
                destroy(item, 1);
            }
            self->faceDataLoader = 0;
        }
        item = self->skipCaption;
        if (item != 0) {
            if (item != 0) {
                Destroy destroy = *(Destroy*)(*(unsigned int*)item + 8);
                destroy(item, 1);
            }
            self->skipCaption = 0;
        }
        AsCStoryConte(self)->state = 0;
    } else {
        void* object;
        if (data != 0) {
            object = __nw__FUl(kStoryFileLoaderBytes);
            if (object != 0) {
                try {
                    ((CEventLoad*)object)->__ct(data, (void*)1, 0);
                } catch (...) {
                    __dl__FPv(object);
                    throw;
                }
            }
            self->eventLoader = object;
        }
        object = __nw__FUl(kStoryFileLoaderBytes);
        if (object != 0) {
            try {
                ((CEventLoad*)object)->__ct((const char*)g_eventFaceDataFilename, (void*)1, 0);
            } catch (...) {
                __dl__FPv(object);
                throw;
            }
        }
        self->faceDataLoader = object;
        void* target = (void*)((volatile int&)Sing_805DC670());
        if (target == 0) {
            if (s_bpPlayerInitialized == 0) {
                ((CBpPlayer*)s_bpPlayer)->__ct();
                __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, s_partyBpPlayerDtorRegistration);
                s_bpPlayerInitialized = 1;
            }
            target = s_bpPlayer;
            ((volatile int&)Sing_805DC670()) = (int)target;
        }
        ((CBpPlayer*)target)->ReleaseResources();
        target = (void*)((volatile int&)Sing_805DC670());
        if (target == 0) {
            if (s_bpPlayerInitialized == 0) {
                ((CBpPlayer*)s_bpPlayer)->__ct();
                __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, s_partyBpPlayerDtorRegistration);
                s_bpPlayerInitialized = 1;
            }
            target = s_bpPlayer;
            ((volatile int&)Sing_805DC670()) = (int)target;
        }
        // The name is the string at +0x3C: original 0x8008D4F0 `lwz r0,0x3c(r31)` tests its long-form bit and
        // picks +0x44 (long data) or +0x3D (inline data). It tested the +0x24 string's flag instead.
        void* name = ((*(unsigned int*)&AsCStoryConte(self)->playerResourceNameFlags) >> 31)
                         ? AsCStoryConte(self)->playerResourceNameData
                         : (char*)&AsCStoryConte(self)->inlinePlayerResourceName;
        ((CBpPlayer*)target)->StartLoad((const char*)name, 1, 0);
        target = (void*)((volatile int&)Sing_805DC670());
        if (target == 0) {
            if (s_bpPlayerInitialized == 0) {
                ((CBpPlayer*)s_bpPlayer)->__ct();
                __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, s_partyBpPlayerDtorRegistration);
                s_bpPlayerInitialized = 1;
            }
            target = s_bpPlayer;
            ((volatile int&)Sing_805DC670()) = (int)target;
        }
        ((CBpPlayer*)target)->LoadMessageResources();
        void* item = self->skipCaption;
        if (item != 0) {
            if (item != 0) {
                Destroy destroy = *(Destroy*)(*(unsigned int*)item + 8);
                destroy(item, 1);
            }
            self->skipCaption = 0;
        }
        item = __nw__FUl(0x28);
        if (item != 0)
            ((CBpTex*)item)->__ct();
        self->skipCaption = item;
        typedef void (*SetName)(void*, void*);
        SetName setName = *(SetName*)(*(unsigned int*)item + 0xc);
        setName(item, StoryStrings.skipCaptionFile);
    }
}

// The story task has no additional initialization at this lifecycle stage.
void CStoryConte::OnInit() {}

extern int StoryMovieConfig_PollDisplaySettled(void*);
extern int StoryMovieConfig_SetDisplayModeIfChanged(void*, int);
extern void StoryMovieConfig_ApplyDisplayMode(void*);
extern void EventMessage_ResetChildAndText(void*);
extern void StoryMessage_DisableAndResetPanel(void*);
extern int EventScript_RunAndSweep(void*);
extern int EventTiming_IsDurationScalingEnabled();
extern int Input_IsActionPressed(void*, int, int);
extern void InputManager_Init(void*);
extern void InputManager_Dtor();
extern void __ct__11CEffectFadeFv(void*);
extern void FadeQuad_SetRgb(void*, int);
extern void EventScript_RunBlocking(void*);
extern void __dt__13CNowReloadingFv();
extern int LoadingIndicator_IsAbsentOrState3(void*);
extern void WPuppet_EraseLayerControllerEntry(void*, int);
extern void* WPuppet_FindOrCacheBone(void*, void*);
extern void WPuppet_EraseBoneControllerEntry(void*, void*);
extern void __destroy_new_array(void*, void*);
extern signed char s_nowReloadingInitialized;
extern volatile int s_nowReloading;
extern signed char s_courseSlotTableInitialized;
extern signed char s_effectFadeInitialized;
extern signed char s_inputManagerInitialized;
extern signed char s_bpPlayerInitialized;

extern unsigned char s_gameInputManager[];
extern void __dt__11CEffectFadeFv();
extern void __dt__13CNowReloadingFv();
// Clear one script-vector count with the original volatile load/store and unsigned subtraction.
inline void ClearStoryVectorCount(unsigned char* script, int offset) {
    volatile int* p = (volatile int*)(script + offset);
    int old = *p;
    union {
        int a;
        unsigned int u;
    } bits;
    bits.a = old;
    *p = old - bits.u;
}
// Initialize/register the shared service once, then publish its singleton pointer.
inline void EnsurePlaybackPlayer(unsigned char* registrationTags) {
    if (!((volatile int&)Sing_805DC670())) {
        if (!s_bpPlayerInitialized) {
            ((CBpPlayer*)s_bpPlayer)->__ct();
            __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, (registrationTags + 0x18));
            s_bpPlayerInitialized = 1;
        }
        ((volatile int&)Sing_805DC670()) = (int)s_bpPlayer;
    }
}
// Initialize/register the shared service once, then publish its singleton pointer.
inline void EnsureStoryFade(unsigned char* registrationTags) {
    if (!((volatile int&)Sing_805DC660())) {
        if (!s_effectFadeInitialized) {
            ((CEffectFade*)s_effectFade)->__ct();
            __register_global_object(s_effectFade, __dt__11CEffectFadeFv, registrationTags);
            s_effectFadeInitialized = 1;
        }
        ((volatile int&)Sing_805DC660()) = (int)s_effectFade;
    }
}
// Initialize/register the shared service once, then publish its singleton pointer.
inline void EnsureStoryInput(unsigned char* registrationTags) {
    if (!((volatile int&)GetInputManagerSlot())) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, registrationTags + 0xc);
            s_inputManagerInitialized = 1;
        }
        ((volatile int&)GetInputManagerSlot()) = (int)s_gameInputManager;
    }
}
// Initialize/register the shared service once, then publish its singleton pointer.
inline void EnsurePlaybackModelSlots(unsigned char* registrationTags) {
    if (!((volatile int&)GetConteModelSlotsSlot())) {
        if (!s_courseSlotTableInitialized) {
            *(unsigned short*)s_conteModelSlots = 0xffff;
            *(int*)(s_conteModelSlots + 8) = 0;
            *(int*)(s_conteModelSlots + 4) = 0;
            do {
                break;
                __destroy_new_array((void*)0x10000, s_conteModelSlots);
            } while (0);
            *(int*)(s_conteModelSlots + 8) = 0;
            s_courseSlotTableInitialized = 1;
        }
        ((volatile int&)GetConteModelSlotsSlot()) = (int)s_conteModelSlots;
    }
}
// Playback checkpoints (all successful gates may fall through in one tick):
// load buffers/resources -> caption ready -> replace/initialize script handle
// -> bind face data/run blocking setup -> normal script execution -> cleanup.
// The expanded handle initialization below is the constructor operation:
// it allocates no source buffer; eventLoad owns the event bytes, eventDataLoad
// owns face bytes, and eventHandle only borrows both until cleanup destroys it.
// The handle is published before initialization; preserve that ordering and
// the explicit per-member constructors rather than replacing with memset.
// Advance loading, playback, skip fading and cleanup; switch fallthrough intentionally advances ready stages.
// State flow: loading -> caption ready -> playing -> cleanup -> finished -> idle.
// Skip input diverts playing -> skip fade -> cleanup; each fade tick returns.
// tags is persistent destructor-registration storage: fade +0, input +0x0C,
// BP player +0x18, loading indicator +0x24. Helpers can construct shared services.
// Resource buffers remain borrowed by the script handle until it is destroyed.
// Entry requires task state +0x1C!=idle AND display-settled poll success; otherwise
// no state-machine work occurs. Normal completion can clean up in the same call.
// Skip requires either action and ZERO current fade alpha; skip entry sets alpha
// zero and returns, so first fade increment occurs on the next eligible call.
// At the limit, only state changes to cleanup: resource release is a later call.
// skipFadeUpdateCount is an UNSIGNED BYTE; thresholds must be reachable before wrapping
// at 256. Raising a limit above 255 can prevent completion; no range check exists.
// Preserve both duration-policy queries, integer multiply -> float divide -> int
// truncation, low-byte alpha forwarding, and the explicit returns between phases.
void CStoryConte::OnProcess() {
    unsigned char* tags = s_storyConteDtorRegistrations;
    if (AsCEventHandle(this)->storyState != kStoryIdle &&
        StoryMovieConfig_PollDisplaySettled(&displayConfiguration) != 0)
        switch (AsCEventHandle(this)->storyState) {
        case kStoryLoading: {
            // Gate A: both file buffers and both BP resource queues must be ready.
            // Every pending poll returns with the same state; ready falls through.
            if (!((CEventLoad*)(AsCEventHandle(this)->eventLoad))->Poll())
                return;
            if (!((CEventLoad*)(AsCEventHandle(this)->eventDataLoad))->Poll())
                return;
            EnsurePlaybackPlayer(tags);
            if (!EventText_PollMessageResources((void*)((volatile int&)Sing_805DC670())))
                return;
            EnsurePlaybackPlayer(tags);
            if (!((CBpPlayer*)(((volatile int&)Sing_805DC670())))->PollResourceLoad())
                return;
            AsCEventHandle(this)->storyState = kStoryWaitForSkipCaption;
        }
        case kStoryWaitForSkipCaption: {
            // Gate B: caption readiness permits replacement of the owned handle.
            // No script pointer may outlive eventLoad/eventDataLoad's buffers.
            void* captionResource = AsCEventHandle(this)->storySkipCaptionResource;
            typedef int (*Check)(void*);
            Check check = ((Check*)*(void**)captionResource)[6];
            if (!check(captionResource))
                return;
            EnsurePlaybackPlayer(tags);
            EventMessage_ResetChildAndText((void*)((volatile int&)Sing_805DC670()));
            EnsurePlaybackModelSlots(tags);
            void* previousHandle = AsCEventHandle(this)->eventHandle;
            if (previousHandle) {
                typedef void (*D)(void*, int);
                D destroy = ((D*)AsCEventHandle(previousHandle)->eventVtable)[2];
                destroy(previousHandle, 1);
                AsCEventHandle(this)->eventHandle = 0;
            }
            void* scriptHandle = __nw__FUl(kStoryScriptHandleBytes);
            AsCEventHandle(this)->eventHandle = scriptHandle;
            if (scriptHandle) {
                // Constructor phase A: initialize strings, five pointer-vector headers,
                // tree sentinels and the embedded effect parser. Keep store order:
                // these are expanded constructors, not one interchangeable zero-fill.
                unsigned char* scriptBytes = (unsigned char*)scriptHandle;
                void* eventBuffer = AsCEventLoad(AsCEventHandle(this)->eventLoad)->buffer;
                unsigned int tableByteOffset = ((AsCEventHandle(this)->storyIndex & 0xff) << 2);
                union {
                    unsigned int w;
                    unsigned char c[4];
                } littleEndianOffset;
                char temps[8];
                char* ownerName = StoryStrings.scriptOwnerName;
                unsigned int packed = *(unsigned int*)((char*)eventBuffer + tableByteOffset);
                AsCEventHandle(scriptBytes)->eventVtable = __vt__12CEventHandle;
                ((StoryScriptInitView*)scriptBytes)->name.flags = 0;
                ((StoryScriptInitView*)scriptBytes)->name.longLength = 0;
                ((StoryScriptInitView*)scriptBytes)->ownerNameData = 0;
                ((StoryScriptInitView*)scriptBytes)->objectList0Data = 0;
                *(volatile int*)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, objectList0Count)) = 0;
                ((StoryScriptInitView*)scriptBytes)->objectList0Capacity = 0;
                ((StoryScriptInitView*)scriptBytes)->objectList1Data = 0;
                ((StoryScriptInitView*)scriptBytes)->objectList1Count = 0;
                ((StoryScriptInitView*)scriptBytes)->objectList1Capacity = 0;
                ((StoryScriptInitView*)scriptBytes)->objectList2Data = 0;
                ((StoryScriptInitView*)scriptBytes)->objectList2Count = 0;
                ((StoryScriptInitView*)scriptBytes)->objectList2Capacity = 0;
                ((StoryScriptInitView*)scriptBytes)->objectList3Data = 0;
                ((StoryScriptInitView*)scriptBytes)->objectList3Count = 0;
                ((StoryScriptInitView*)scriptBytes)->objectList3Capacity = 0;
                ((StoryScriptInitView*)scriptBytes)->objectList4Data = 0;
                ((StoryScriptInitView*)scriptBytes)->objectList4Count = 0;
                // Constructor phase B: the event buffer stores a little-endian byte
                // offset. Reverse its bytes before adding it to the borrowed buffer.
                littleEndianOffset.w = packed;
                unsigned char b0 = littleEndianOffset.c[0], b3 = littleEndianOffset.c[3], b1 = littleEndianOffset.c[1],
                              b2 = littleEndianOffset.c[2];
                littleEndianOffset.c[0] = b3;
                littleEndianOffset.c[3] = b0;
                littleEndianOffset.c[1] = b2;
                littleEndianOffset.c[2] = b1;
                unsigned int scriptOffset = littleEndianOffset.w;
                ((StoryScriptInitView*)scriptBytes)->objectList4Capacity = 0;
                ((StoryScriptInitView*)scriptBytes)->objectTreeCount = 0;
                *(unsigned int*)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, objectTreeRoot)) = 0;
                *(unsigned int*)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, objectTreeEnd)) =
                    (unsigned int)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, objectTreeRoot));
                Init__12CEventEffectFv((CEventEffect*)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, pad47c)),
                                       (char*)eventBuffer + scriptOffset);
                ((StoryScriptInitView*)scriptBytes)->resourceTreeCount = 0;
                *(unsigned int*)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, resourceTreeRoot)) = 0;
                *(unsigned int*)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, resourceTreeEnd)) =
                    (unsigned int)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, resourceTreeRoot));
                unsigned int oldStringLength =
                    (((StoryScriptInitView*)scriptBytes)->name.flags >> 31)
                        ? ((StoryScriptInitView*)scriptBytes)->name.longLength
                        : (*(unsigned char*)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, name)) & 0x7f);
                temps[4] = temps[0];
                String_ReplaceRange((StoryPathString*)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, name)), 0,
                                    (int)oldStringLength, ownerName, ownerName + strlen(ownerName), &temps[4]);
                // Constructor phase C: bind source/cursor, clear command scratch and
                // owned handlers, then clear lists/trees to their empty invariants.
                *(void**)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, sourceBuffer)) = eventBuffer;
                *(void**)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, commandCursor)) =
                    (char*)eventBuffer + scriptOffset;
                memset(scriptBytes, 0, 8);
                memset(scriptBytes + 8, 0, 0x400);
                ((StoryScriptInitView*)scriptBytes)->commandCount = 0;
                (*(unsigned int*)&AsCEventHandle(scriptBytes)->ownedCommandHandler0) = 0;
                (*(unsigned int*)&AsCEventHandle(scriptBytes)->pendingModelEvent) = 0;
                (*(unsigned int*)&AsCEventHandle(scriptBytes)->pendingEffectEvent) = 0;
                (*(unsigned int*)&AsCEventHandle(scriptBytes)->pendingMessageRequest) = 0;
                (*(unsigned int*)&AsCEventHandle(scriptBytes)->pendingSoundEvent) = 0;
                ClearStoryVectorCount(scriptBytes, FIELD_OFFSET(StoryScriptInitView, objectList0Count));
                ClearStoryVectorCount(scriptBytes, FIELD_OFFSET(StoryScriptInitView, objectList1Count));
                ClearStoryVectorCount(scriptBytes, FIELD_OFFSET(StoryScriptInitView, objectList2Count));
                ClearStoryVectorCount(scriptBytes, FIELD_OFFSET(StoryScriptInitView, objectList3Count));
                ClearStoryVectorCount(scriptBytes, FIELD_OFFSET(StoryScriptInitView, objectList4Count));
                (*(unsigned int*)&AsCEventHandle(scriptBytes)->pendingResource) = 0;
                void* root = *(void**)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, objectTreeRoot));
                if (root)
                    cleanTree(scriptBytes + FIELD_OFFSET(StoryScriptInitView, objectTreeCount), root);
                ((StoryScriptInitView*)scriptBytes)->objectTreeCount = 0;
                *(unsigned int*)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, objectTreeRoot)) = 0;
                *(unsigned int*)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, objectTreeEnd)) =
                    (unsigned int)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, objectTreeRoot));
                root = *(void**)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, resourceTreeRoot));
                if (root)
                    cleanTree(scriptBytes + FIELD_OFFSET(StoryScriptInitView, resourceTreeCount), root);
                ((StoryScriptInitView*)scriptBytes)->resourceTreeCount = 0;
                *(unsigned int*)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, resourceTreeRoot)) = 0;
                *(unsigned int*)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, resourceTreeEnd)) =
                    (unsigned int)(scriptBytes + FIELD_OFFSET(StoryScriptInitView, resourceTreeRoot));
                ((StoryScriptInitView*)scriptBytes)->faceScriptTableBase = 0;
                ((StoryScriptInitView*)scriptBytes)->manager = 0;
            }
            // Publication complete: bind the face-data script, run its setup,
            // enter playback, then notify the shared loading indicator.
            AsCEventHandle(this)->eventHandle = scriptHandle;
            (*(void**)&((StoryScriptInitView*)scriptHandle)->faceScriptTableBase) =
                AsCEventLoad(AsCEventHandle(this)->eventDataLoad)->buffer;
            EventScript_RunBlocking(
                (*(void**)&((StoryScriptInitView*)AsCEventHandle(this)->eventHandle)->faceScriptTableBase));
            AsCEventHandle(this)->storyState = kStoryPlaying;
            if (!((volatile int&)Sing_805DC610())) {
                if (!s_nowReloadingInitialized) {
                    ((CNowReloading*)(&s_nowReloading))->Init();
                    __register_global_object((void*)&s_nowReloading, __dt__13CNowReloadingFv, (tags + 0x24));
                    s_nowReloadingInitialized = 1;
                }
                ((volatile int&)Sing_805DC610()) = (int)(void*)&s_nowReloading;
            }
            void* mgr = (void*)((volatile int&)Sing_805DC610());
            if (mgr && LoadingIndicator_IsAbsentOrState3(mgr))
                CB2dPlayerHandle::RequestPendingScript(mgr);
        }
        case kStoryPlaying: {
            // Skip request. Original: neither action 0x11 nor 0x12 pressed -> li r0,0 (0x8008E5FC); one pressed ->
            // r0 = !CEffectFade::GetAlpha() (cntlzw/srwi at 0x8008E65C). At 0x8008E668 `beq` sends r0 == 0 to the
            // EventScript_RunAndSweep load check (0x8008E730); r0 != 0 falls into the fade-out / state 5 path. The test was
            // inverted (`!ready`), which faded out and skipped the scene when no button was pressed.
            int skip;
            do {
                EnsureStoryInput(tags);
                if (!Input_IsActionPressed((void*)((volatile int&)GetInputManagerSlot()), kSkipActionPrimary, -1)) {
                    EnsureStoryInput(tags);
                    if (!Input_IsActionPressed((void*)((volatile int&)GetInputManagerSlot()), kSkipActionSecondary,
                                               -1)) {
                        skip = 0;
                        break;
                    }
                }
                EnsureStoryFade(tags);
                skip = !(unsigned char)((CEffectFade*)(((volatile int&)Sing_805DC660())))->GetAlpha();
            } while (0);
            if (skip) {
                AsCEventHandle(this)->skipFadeUpdateCount = 0;
                EnsureStoryFade(tags);
                FadeQuad_SetRgb((void*)((volatile int&)Sing_805DC660()), 0);
                EnsureStoryFade(tags);
                ((CEffectFade*)(((volatile int&)Sing_805DC660())))->SetAlpha(0);
                AsCEventHandle(this)->storyState = kStorySkipFade;
                return;
            }
            if (!EventScript_RunAndSweep(AsCEventHandle(this)->eventHandle))
                return;
            AsCEventHandle(this)->storyState = kStoryCleanup;
        }
        case kStoryCleanup: {
            // Release the handle before detaching model controllers and hiding messages;
            // models in the shared slot table outlive their borrowed script references.
            void* object = AsCEventHandle(this)->eventHandle;
            if (object) {
                typedef void (*D)(void*, int);
                D destroy = ((D*)AsCEventHandle(object)->eventVtable)[2];
                destroy(object, 1);
                AsCEventHandle(this)->eventHandle = 0;
            }
            EnsurePlaybackModelSlots(tags);
            void* mgr = (void*)((volatile int&)GetConteModelSlotsSlot());
            for (unsigned int i = 0; i < kStoryModelSlotCount; i++) {
                void* array = *(void**)((char*)mgr + 8);
                void* slot = (array && (unsigned char)i < kStoryModelSlotCount)
                                 ? (char*)array + (unsigned char)i * kStoryModelSlotBytes
                                 : 0;
                void* asset = slot ? *(void**)((char*)slot + 4) : 0;
                if (asset) {
                    WPuppet_EraseLayerControllerEntry(asset, 0);
                    WPuppet_EraseLayerControllerEntry(asset, 1);
                    void* found = WPuppet_FindOrCacheBone(asset, storyConteStrings);
                    if (found)
                        WPuppet_EraseBoneControllerEntry(asset, ((BoneControllerView*)found)->controller);
                }
            }
            EnsurePlaybackPlayer(tags);
            StoryMessage_DisableAndResetPanel((void*)((volatile int&)Sing_805DC670()));
            if (StoryMovieConfig_SetDisplayModeIfChanged((void*)((int)this + 0x4c), 2))
                StoryMovieConfig_ApplyDisplayMode((void*)((int)this + 0x4c));
            AsCEventHandle(this)->storyState = kStoryFinished;
            return;
        }
        case kStorySkipFade: {
            unsigned int limit =
                EventTiming_IsDurationScalingEnabled() ? kSkipFadeScaledUpdates : kSkipFadeDefaultUpdates;
            if (AsCEventHandle(this)->skipFadeUpdateCount >= limit) {
                AsCEventHandle(this)->storyState = kStoryCleanup;
                return;
            }
            AsCEventHandle(this)->skipFadeUpdateCount = AsCEventHandle(this)->skipFadeUpdateCount + 1;
            unsigned int limit2 =
                EventTiming_IsDurationScalingEnabled() ? kSkipFadeScaledUpdates : kSkipFadeDefaultUpdates;
            int progress =
                (int)((float)(AsCEventHandle(this)->skipFadeUpdateCount * kStoryOpaqueAlpha) / (float)limit2);
            EnsureStoryFade(tags);
            ((CEffectFade*)(((volatile int&)Sing_805DC660())))->SetAlpha(progress & 0xff);
            return;
        }
        case kStoryFinished:
            AsCEventHandle(this)->storyState = kStoryIdle;
        }
}

// Clear the owned pointer-vector count while preserving the original volatile read and write.
void ScriptPointerVector_ClearCount(volatile int* value) {
    int old = value[1];
    union {
        int a;
        unsigned int b;
    } bits;
    bits.a = old;
    value[1] = old - bits.b;
}

// Return the end of an array of four-byte object pointers.
void* ScriptPointerVector_End(void** value) {
    return (int*)value[0] + (int)value[1];
}

extern unsigned char s_storyConteTypeInfo;
// Return the runtime type descriptor used to identify this story task.
void* CStoryConte::GetRTTI() {
    return &s_storyConteTypeInfo;
}

// Return the beginning of an owned pointer array.
void* ScriptPointerVector_Begin(void** value) {
    return *value;
}

// Copy the first (leftmost) object-tree node to a borrowed iterator output.
void ScriptObjectTree_GetBeginIterator(void** destination, void* source) {
    *destination = ((void**)source)[2];
}

// Form the object-tree END sentinel from its embedded root link; no allocation.
void ScriptObjectTree_GetEndIterator(void** destination, void* source) {
    *destination = (char*)source + 4;
}

// Copy the first (leftmost) resource-tree node to a borrowed iterator output.
void ScriptResourceTree_GetBeginIterator(void** destination, void* source) {
    *destination = ((void**)source)[2];
}

// Return the key/value payload following a tree node header.
void* CEventHandleTreeIterator::GetValue() {
    return &node->key;
}

// Test whether two iterator node pointers differ.
int ScriptTreeIterator_NotEqual(unsigned int* left, unsigned int* right) {
    return *left != *right;
}

// Form the resource-tree END sentinel from its embedded root link; no allocation.
void ScriptResourceTree_GetEndIterator(void** destination, void* source) {
    *destination = (char*)source + 4;
}
void* __dl__FPv(void*);

// Save the current iterator and advance in sorted order; bit zero tags parent color.
void CEventHandleTreeIterator::PostIncrement(CEventHandleTreeIterator* destination, CEventHandleTreeIterator* iterator,
                                             void* unused) {
    destination->node = iterator->node;
    EventHandleTreeNode* node = iterator->node->right;
    if (node != 0) {
        while (node->left != 0)
            node = node->left;
        iterator->node = node;
        return;
    }
    for (;;) {
        node = iterator->node;
        EventHandleTreeNode* parent = (EventHandleTreeNode*)(node->parentAndColor & ~1u);
        iterator->node = parent;
        if (node == parent->left)
            return;
    }
}

void appFreeUni__FPv(void*);
// Clear the pointer-vector count, release its allocation, and optionally free the container.
void* ScriptPointerVector_Dtor(void* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor((MSLPointerVector*)self, shouldDelete);
}

// Destroy every nonnull object in a pointer range, clearing each slot after release.
// unused is an unused ABI receiver. cursor/limit are required iterator WORD
// addresses holding [begin,end) pointers; writes *cursor through to end and
// clears payload slots. Storage itself stays owned by its vector. Empty is safe.
// Payload vtable is at +0x20, destructor index 2 receives deleting flag 1.
void ScriptPointerRange_DestroyObjects(void* unused, void** cursor, void** limit) {
    while (*cursor != *limit) {
        void* item = *(void**)*cursor;
        if (item != 0) {
            if (item != 0) {
                typedef void (*Destroy)(void*, int);
                ((Destroy*)((ScriptObjectVtable20*)item)->vtable)[2](item, 1);
            }
            *(void**)*cursor = 0;
        }
        *cursor = (void*)((char*)*cursor + 4);
    }
}

// Finish base event-conte destruction; this base has no additional cleanup here.
CEventConte::~CEventConte() {}

// Ownership map for the views used by the handle destructor:
// CEventHandle (0x4A8 bytes) owns handlers +420..430 and five vectors +434..46C.
// Each vector owns its objects (vtable at object+0x20) AND its pointer allocation.
// Object tree +470 and resource tree +490: {count, root, end}; end on an empty
// tree is &root. Node header +00 left, +04 right, +08 tagged parent/color;
// +0C key, +10 object. Iterators return the +0C payload, not the object itself.
// Object-tree payloads use a vtable at object+0x20; resource-tree payloads and the
// pending resource +0x48C use object+0x30. Destroy payloads before freeing nodes.
// +47C is embedded effect-parser state; +408 is the owned tagged owner name.
// Cleanup tags below are partial views of these same objects, not extra owners.
// shouldDelete > 0 frees the enclosing allocation; -1 tears down a member only.
struct EventTreeLinksView {
    EventTreeLinksView* left;
    EventTreeLinksView* right;
};
// Free a destructor fringe node, leaving enclosing-header reset to the caller.
inline void FreeEventTreeFringe(void* owner, EventTreeLinksView* leaf) {
    if (leaf != 0) {
        if (leaf->left != 0)
            ScriptByteMap_FreeSubtree(owner, leaf->left);
        if (leaf->right != 0)
            ScriptByteMap_FreeSubtree(owner, leaf->right);
        appFreeUni__FPv(leaf);
    }
}
struct EventPointerRangeView {
    void* begin;
    int count;
};
struct EventResourceNodeView {
    EventResourceNodeView* left;
    EventResourceNodeView* right;
    unsigned int parentColor;
    unsigned int pad0C;
    void* item;
};
struct EventResourcePayloadView {
    unsigned int pad00[1];
    void* item;
};
struct EventResourceTreeView {
    void* first;
    EventResourceNodeView* root;
    EventResourceNodeView** tail;
};
extern unsigned int __vt__12CEventHandle[];

struct EventObjectNodeView;
struct EventObjectTreeLinksView;
void Tree_Clear(EventObjectTreeLinksView**);
struct EventResourceTreeHeaderView;
void* ScriptResourceTree_FreeNodes(void*, int);
void* ScriptObjectTree_FreeNodes(void*, int);
void* ScriptPointerVector_Dtor(void*, int);
#pragma dont_inline on
// Release owned command handlers, pointer arrays, object/resource trees and the script name.
// The duplicate null tests on each handler are the nested destructor layers; collapsing them
// changes register allocation.
CEventHandle::~CEventHandle() {
    // Phase 1: destroy owned handlers/list payloads before freeing container storage.
    void* child = ownedCommandHandler0;
    eventVtable = __vt__12CEventHandle;
    if (child != 0) {
        if (child != 0) {
            typedef void (*Destroy)(void*, int);
            Destroy destroy = ((Destroy*)*(void**)child)[2];
            destroy(child, 1);
        }
        ownedCommandHandler0 = 0;
    }
    child = pendingModelEvent;
    if (child != 0) {
        if (child != 0) {
            typedef void (*Destroy)(void*, int);
            Destroy destroy = ((Destroy*)*(void**)child)[2];
            destroy(child, 1);
        }
        pendingModelEvent = 0;
    }
    child = pendingEffectEvent;
    if (child != 0) {
        if (child != 0) {
            typedef void (*Destroy)(void*, int);
            Destroy destroy = ((Destroy*)*(void**)child)[2];
            destroy(child, 1);
        }
        pendingEffectEvent = 0;
    }
    child = pendingMessageRequest;
    if (child != 0) {
        if (child != 0) {
            typedef void (*Destroy)(void*, int);
            Destroy destroy = ((Destroy*)*(void**)child)[2];
            destroy(child, 1);
        }
        pendingMessageRequest = 0;
    }
    child = pendingSoundEvent;
    if (child != 0) {
        if (child != 0) {
            typedef void (*Destroy)(void*, int);
            Destroy destroy = ((Destroy*)*(void**)child)[2];
            destroy(child, 1);
        }
        pendingSoundEvent = 0;
    }
    void* list0End;
    void* list0Cursor;
    list0End = ScriptPointerVector_End((void**)&objectLists[0]);
    list0Cursor = ScriptPointerVector_Begin((void**)&objectLists[0]);
    ScriptPointerRange_DestroyObjects(this, &list0Cursor, &list0End);
    void* list1End;
    void* list1Cursor;
    list1End = ScriptPointerVector_End((void**)&objectLists[1]);
    list1Cursor = ScriptPointerVector_Begin((void**)&objectLists[1]);
    ScriptPointerRange_DestroyObjects(this, &list1Cursor, &list1End);
    void* list2End;
    void* list2Cursor;
    list2End = ScriptPointerVector_End((void**)&objectLists[2]);
    list2Cursor = ScriptPointerVector_Begin((void**)&objectLists[2]);
    ScriptPointerRange_DestroyObjects(this, &list2Cursor, &list2End);
    void* list3End;
    void* list3Cursor;
    list3End = ScriptPointerVector_End((void**)&objectLists[3]);
    list3Cursor = ScriptPointerVector_Begin((void**)&objectLists[3]);
    ScriptPointerRange_DestroyObjects(this, &list3Cursor, &list3End);
    void* list4End;
    void* list4Cursor;
    list4End = ScriptPointerVector_End((void**)&objectLists[4]);
    list4Cursor = ScriptPointerVector_Begin((void**)&objectLists[4]);
    ScriptPointerRange_DestroyObjects(this, &list4Cursor, &list4End);
    ScriptPointerVector_ClearCount((volatile int*)&objectLists[0]);
    ScriptPointerVector_ClearCount((volatile int*)&objectLists[1]);
    ScriptPointerVector_ClearCount((volatile int*)&objectLists[2]);
    ScriptPointerVector_ClearCount((volatile int*)&objectLists[3]);
    ScriptPointerVector_ClearCount((volatile int*)&objectLists[4]);
    // Phase 2: object payloads first, then their tree nodes. The two iterator
    // helpers yield the root-link sentinel and first element respectively.
    void* objectTreeSentinel;
    void* objectTreeCursor;
    ScriptObjectTree_GetEndIterator(&objectTreeSentinel, (char*)&objectTree.count);
    ScriptObjectTree_GetBeginIterator(&objectTreeCursor, (char*)&objectTree.count);
    DestroyTreeRangeObjects((EventObjectNodeView**)&objectTreeCursor, (EventObjectNodeView**)&objectTreeSentinel);
    Tree_Clear((EventObjectTreeLinksView**)&objectTree.count);
    child = pendingResource;
    if (child != 0) {
        if (child != 0) {
            typedef void (*Destroy)(void*, int);
            Destroy destroy = ((Destroy*)((ScriptObjectVtable30*)child)->vtable)[2];
            destroy(child, 1);
        }
        pendingResource = 0;
    }
    // Phase 3: pending resource, then resource-tree payloads (vtable +30).
    // The empty-tree sentinel must be tested before dereferencing a payload.
    EventResourceNodeView* cursor;
    ScriptResourceTree_GetBeginIterator((void**)&cursor, (char*)&resourceTree.count);
    void* zero = 0;
    EventResourceNodeView* next;
    EventResourceNodeView* limit;
    // The condition computes the sentinel, so the original compiles this as a
    // rotated while loop that tests before the first pass; a do-while would
    // dereference the header node as an item.
    while ((ScriptResourceTree_GetEndIterator((void**)&limit, (char*)&resourceTree.count),
            ScriptTreeIterator_NotEqual((unsigned int*)&cursor, (unsigned int*)&limit))) {
        EventResourcePayloadView* node = (EventResourcePayloadView*)((CEventHandleTreeIterator*)&cursor)->GetValue();
        if (node->item != 0) {
            node = (EventResourcePayloadView*)((CEventHandleTreeIterator*)&cursor)->GetValue();
            child = node->item;
            if (child != 0) {
                typedef void (*Destroy)(void*, int);
                Destroy destroy = ((Destroy*)((ScriptObjectVtable30*)child)->vtable)[2];
                destroy(child, 1);
            }
            node = (EventResourcePayloadView*)((CEventHandleTreeIterator*)&cursor)->GetValue();
            node->item = zero;
        }
        CEventHandleTreeIterator::PostIncrement((CEventHandleTreeIterator*)&next, (CEventHandleTreeIterator*)&cursor,
                                                0);
    }
    // ClearTree is declared on the handle but operates on this embedded tree header.
    ((CEventHandle*)&resourceTree)->ClearTree();
    // Phase 4: payloads are gone; release trees, effect state, pointer buffers and owner name.
    ScriptResourceTree_FreeNodes((char*)&resourceTree.count, -1);
    ((CEventEffect*)pad_47C)->Destroy(-1);
    ScriptObjectTree_FreeNodes((char*)&objectTree.count, -1);
    ScriptPointerVector_Dtor((char*)&objectLists[4], -1);
    ScriptPointerVector_Dtor((char*)&objectLists[3], -1);
    ScriptPointerVector_Dtor((char*)&objectLists[2], -1);
    ScriptPointerVector_Dtor((char*)&objectLists[1], -1);
    ScriptPointerVector_Dtor((char*)&objectLists[0], -1);
    ((NString*)((char*)&nameFlags))->~NString();
}

// Both destructors below own nodes only; payloads were released by ~CEventHandle.
// self+4 is the root. left/right are child links. Walk children before freeing
// their parent, unroll four levels, then delegate deeper branches to FreeSubtree.
// Repeated null guards come from nested destructors. Collapsing them changes
// MWCC register allocation; they do not represent additional ownership conditions.
extern void ScriptByteMap_FreeSubtree(void*, void*);
#pragma use_lmw_stmw on
// Destroy the object-tree nodes without destroying their already-released payloads.
#pragma dont_inline off
// Release an expanded branch after both fringe subtrees; tree payloads are already gone.
static inline void FreeEventTreeBranch(void* self, EventTreeLinksView* branch, EventTreeLinksView*& leaf) {
    leaf = branch->left;
    FreeEventTreeFringe(self, leaf);
    leaf = branch->right;
    FreeEventTreeFringe(self, leaf);
    appFreeUni__FPv(branch);
}

// Release object-tree node storage, then optionally delete its embedded-tree wrapper.
void* ScriptObjectTree_FreeNodes(void* self, int shouldDelete) {
    // Snapshot aliases retain the original variable lifetimes/register allocation.
    // They are repeated reads of child links, not additional owned nodes.
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (0 != self) {
                    if (self) {
                        if (0 != self) {
                            EventTreeLinksView* branch;
                            EventTreeLinksView* root;
                            EventTreeLinksView* leaf;
                            EventTreeLinksView* owner = *(EventTreeLinksView**)((char*)self + 4);
                            if (0 != owner) {
                                __typeof__(owner->left) ownerLeftSnapshot1 = owner->left;
                                __typeof__(ownerLeftSnapshot1) ownerLeftSnapshot2 = owner->left;
                                root = ownerLeftSnapshot2;
                                if (0 != root) {
                                    __typeof__(root->left) rootLeftSnapshot1 = root->left;
                                    __typeof__(root->left) rootLeftSnapshot2 = root->left;
                                    branch = rootLeftSnapshot1;
                                    if (branch != 0) {
                                        __typeof__(branch->left) branchLeftSnapshot1 = branch->left;
                                        leaf = branchLeftSnapshot1;
                                        FreeEventTreeFringe(self, leaf);
                                        __typeof__(branch->right) branchRightSnapshot1 = branch->right;
                                        leaf = branchRightSnapshot1;
                                        FreeEventTreeFringe(self, leaf);
                                        appFreeUni__FPv(branch);
                                    }
                                    __typeof__(root->right) rootRightSnapshot1 = root->right;
                                    branch = rootRightSnapshot1;
                                    if (branch) {
                                        __typeof__(branch->left) branchLeftSnapshot2 = branch->left;
                                        __typeof__(branch->left) branchLeftSnapshot3 = branch->left;
                                        leaf = branchLeftSnapshot2;
                                        FreeEventTreeFringe(self, leaf);
                                        __typeof__(branch->right) branchRightSnapshot2 = branch->right;
                                        leaf = branchRightSnapshot2;
                                        FreeEventTreeFringe(self, leaf);
                                        appFreeUni__FPv(branch);
                                    }
                                    appFreeUni__FPv(root);
                                }
                                __typeof__(owner->right) ownerRightSnapshot1 = owner->right;
                                __typeof__(owner->right) ownerRightSnapshot2 = owner->right;
                                root = ownerRightSnapshot1;
                                if (root) {
                                    branch = root->left;
                                    if (branch) {
                                        __typeof__(branch->left) branchLeftSnapshot4 = branch->left;
                                        leaf = branchLeftSnapshot4;
                                        FreeEventTreeFringe(self, leaf);
                                        __typeof__(branch->right) branchRightSnapshot3 = branch->right;
                                        leaf = branchRightSnapshot3;
                                        FreeEventTreeFringe(self, leaf);
                                        appFreeUni__FPv(branch);
                                    }
                                    __typeof__(root->right) rootRightSnapshot2 = root->right;
                                    __typeof__(root->right) rootRightSnapshot3 = root->right;
                                    __typeof__(root->right) rootRightSnapshot4 = root->right;
                                    __typeof__(rootRightSnapshot2) rootRightSnapshot5 = rootRightSnapshot4;
                                    __typeof__(root->right) rootRightSnapshot6 = rootRightSnapshot5;
                                    __typeof__(rootRightSnapshot3) rootRightSnapshot7 = root->right;
                                    __typeof__(root->right) rootRightSnapshot8 = rootRightSnapshot7;
                                    __typeof__(root->right) rootRightSnapshot9 = root->right;
                                    __typeof__(rootRightSnapshot9) rootRightSnapshot10 = root->right;
                                    __typeof__(root->right) rootRightSnapshot11 = root->right;
                                    __typeof__(rootRightSnapshot8) rootRightSnapshot12 = rootRightSnapshot10;
                                    __typeof__(rootRightSnapshot6) rootRightSnapshot13 = rootRightSnapshot11;
                                    __typeof__(rootRightSnapshot13) rootRightSnapshot14 = rootRightSnapshot12;
                                    branch = rootRightSnapshot14;
                                    if (branch != 0) {
                                        leaf = branch->left;
                                        FreeEventTreeFringe(self, leaf);
                                        __typeof__(branch->right) branchRightSnapshot4 = branch->right;
                                        leaf = branchRightSnapshot4;
                                        FreeEventTreeFringe(self, leaf);
                                        appFreeUni__FPv(branch);
                                    }
                                    appFreeUni__FPv(root);
                                }
                                appFreeUni__FPv(owner);
                            }
                        }
                    }
                }
            }
        }
        if (0 < shouldDelete)
            __dl__FPv(self);
    }
    return self;
}

#pragma use_lmw_stmw on
// Destroy the resource-tree nodes without destroying their already-released payloads.
void* ScriptResourceTree_FreeNodes(void* self, int shouldDelete) {
    EventTreeLinksView* root;
    EventTreeLinksView* branch;
    EventTreeLinksView* leaf;
    EventTreeLinksView* owner;
    // Snapshot aliases retain the original variable lifetimes/register allocation.
    // They are repeated reads of child links, not additional owned nodes.
    if (self != 0) {
        if (0 != self) {
            if (0 != self) {
                if (0 != self) {
                    if (0 != self) {
                        if (0 != self) {
                            owner = *(EventTreeLinksView**)((char*)self + 4);
                            if (owner != 0) {
                                root = owner->left;
                                if (0 != root) {
                                    branch = root->left;
                                    if (branch != 0) {
                                        __typeof__(branch->left) branchLeftSnapshot1 = branch->left;
                                        leaf = branchLeftSnapshot1;
                                        FreeEventTreeFringe(self, leaf);
                                        __typeof__(branch->right) branchRightSnapshot1 = branch->right;
                                        __typeof__(branch->right) branchRightSnapshot2 = branch->right;
                                        __typeof__(branch->right) branchRightSnapshot3 = branch->right;
                                        __typeof__(branchRightSnapshot3) branchRightSnapshot4 = branchRightSnapshot2;
                                        __typeof__(branchRightSnapshot1) branchRightSnapshot5 = branchRightSnapshot4;
                                        __typeof__(branch->right) branchRightSnapshot6 = branch->right;
                                        __typeof__(branchRightSnapshot5) branchRightSnapshot7 = branchRightSnapshot6;
                                        __typeof__(branch->right) branchRightSnapshot8 = branch->right;
                                        __typeof__(branch->right) branchRightSnapshot9 = branchRightSnapshot8;
                                        __typeof__(branchRightSnapshot7) branchRightSnapshot10 = branchRightSnapshot9;
                                        leaf = branchRightSnapshot10;
                                        FreeEventTreeFringe(self, leaf);
                                        appFreeUni__FPv(branch);
                                    }
                                    branch = root->right;
                                    if (0 != branch) {
                                        leaf = branch->left;
                                        FreeEventTreeFringe(self, leaf);
                                        __typeof__(branch->right) branchRightSnapshot11 = branch->right;
                                        leaf = branchRightSnapshot11;
                                        FreeEventTreeFringe(self, leaf);
                                        appFreeUni__FPv(branch);
                                    }
                                    appFreeUni__FPv(root);
                                }
                                root = owner->right;
                                if (0 != root) {
                                    __typeof__(root->left) rootLeftSnapshot1 = root->left;
                                    __typeof__(rootLeftSnapshot1) rootLeftSnapshot2 = root->left;
                                    branch = rootLeftSnapshot2;
                                    if (branch != 0) {
                                        __typeof__(branch->left) branchLeftSnapshot2 = branch->left;
                                        leaf = branchLeftSnapshot2;
                                        FreeEventTreeFringe(self, leaf);
                                        leaf = branch->right;
                                        FreeEventTreeFringe(self, leaf);
                                        appFreeUni__FPv(branch);
                                    }
                                    __typeof__(root->right) rootRightSnapshot1 = root->right;
                                    branch = rootRightSnapshot1;
                                    if (branch != 0) {
                                        FreeEventTreeBranch(self, branch, leaf);
                                    }
                                    appFreeUni__FPv(root);
                                }
                                appFreeUni__FPv(owner);
                            }
                        }
                    }
                }
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

#pragma dont_inline on
void __ct__18NativeBase80201D18Fv(void*);
void StoryMovieConfig_InitDefaults(void*);
void SaveData_GetSelectedTourConteScriptName(void*);
void __dt__18NativeBase80201D18Fv(void*, int);
extern unsigned int __vt__11CStoryConte[16];
extern unsigned int __vt__15CEventConteProc[16];

struct StoryOwnedStringView {
    unsigned int flags;
    unsigned int length;
    unsigned char* resource;
};
struct StoryTaskStringInitView {
    unsigned char pad00[0x24];
    StoryOwnedStringView first;
    unsigned char pad30[0x0C];
    StoryOwnedStringView second;
};
// Initialize task interfaces, resource paths and the idle story state.
CStoryConte::CStoryConte() {
    StoryTaskStringInitView* self = (StoryTaskStringInitView*)this;
    __ct__18NativeBase80201D18Fv(self);
    int zero = 0;
    self->first.flags = zero;
    *(void**)self = &__vt__11CStoryConte;
    AsCStoryConte(self)->taskMainVtable = (char*)&__vt__11CStoryConte + kTaskMainVtableOffset;
    self->first.length = zero;
    self->first.resource = (unsigned char*)zero;
    self->second.flags = zero;
    self->second.length = zero;
    self->second.resource = (unsigned char*)zero;
    StoryMovieConfig_InitDefaults((char*)&AsCStoryConte(self)->displayConfiguration);
    AsCStoryConte(self)->unk_8 = 4;
    AsCStoryConte(self)->eventIndex = 3;
    if ((self->first.flags >> 31) == 0) {
        AsCStoryConte(self)->inlineEventPath = 0;
        AsCStoryConte(self)->eventPathFlags &= ~0x7f;
    } else {
        *self->first.resource = 0;
        self->first.length = 0;
    }
    (*(unsigned int*)&AsCStoryConte(self)->eventLoader) = 0;
    (*(unsigned int*)&AsCStoryConte(self)->eventScript) = 0;
    (*(unsigned int*)&AsCStoryConte(self)->playerResourceLoader) = 0;
    AsCStoryConte(self)->skipCaptionResource = 0;
    if ((self->second.flags >> 31) == 0) {
        (*(signed char*)&AsCStoryConte(self)->inlinePlayerResourceName) = 0;
        AsCStoryConte(self)->playerResourceNameFlags &= ~0x7f;
    } else {
        *self->second.resource = 0;
        self->second.length = 0;
    }
    SaveData_GetSelectedTourConteScriptName(&self->second);
    AsCStoryConte(self)->skipFadeUpdateCount = 0;
    (*(unsigned int*)&AsCStoryConte(self)->state) = 0;
}

void* __dl__FPv(void*);
struct StoryTaskTeardownVtable {
    virtual void v00();
    virtual void v04();
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
};
struct StoryInlineStringTagView {
    CW_BITS2(unsigned char flag : 1, unsigned char len : 7)
};
// CStoryConte's two owning strings: first = event path +24, second = player
// resource name +3C. OnDestroy releases loaders/script/caption; this destructor
// calls task vslot +20, then frees heap-backed strings and the native task base.
struct StoryTaskStringCleanupView {
    void** vtable;
    unsigned char pad04[0x20];
    StoryOwnedStringView first;
    unsigned char pad30[0x0C];
    StoryOwnedStringView second;
};
// Destroy task interfaces and owned strings; resource shutdown belongs to OnDestroy.
CStoryConte::~CStoryConte() {
    StoryTaskStringCleanupView* self = (StoryTaskStringCleanupView*)this;
    self->vtable = (void**)&__vt__11CStoryConte;
    AsCStoryConte(self)->taskMainVtable = (char*)&__vt__11CStoryConte + kTaskMainVtableOffset;
    ((StoryTaskTeardownVtable*)self)->v20();
    if ((self->second.flags >> 31) == 0) {
        ((StoryInlineStringTagView*)((char*)&AsCStoryConte(self)->playerResourceNameFlags))->len = 0;
        AsCStoryConte(self)->inlinePlayerResourceName = 0;
    } else {
        *self->second.resource = 0;
        self->second.length = 0;
    }
    if (&self->second != 0 && (self->second.flags >> 31) != 0)
        appFreeUni__FPv(self->second.resource);
    if (&self->first != 0 && (self->first.flags >> 31) != 0)
        appFreeUni__FPv(self->first.resource);
    if (self != 0)
        __dt__18NativeBase80201D18Fv(self, 0);
}

struct ConteOwnedStringView {
    unsigned int flags;
    unsigned int length;
    unsigned char* resource;
};
struct ConteProcessorCleanupView {
    unsigned char pad00[4];
    void** ownedObject;
    ConteOwnedStringView eventName;
    ConteOwnedStringView eventKind;
    void* vtable;
};
// Release the processor object and both heap-backed string buffers.
CEventConteProc::~CEventConteProc() {
    ConteProcessorCleanupView* self = (ConteProcessorCleanupView*)this;
    self->vtable = &__vt__15CEventConteProc;
    if ((self->eventName.flags >> 31) == 0) {
        AsCEventConteProc(self)->inlineEventName = 0;
        AsCEventConteProc(self)->eventNameTag &= ~0x7f;
    } else {
        *self->eventName.resource = 0;
        self->eventName.length = 0;
    }
    if ((self->eventKind.flags >> 31) == 0) {
        AsCEventConteProc(self)->inlineEventKind = 0;
        AsCEventConteProc(self)->eventKindTag &= ~0x7f;
    } else {
        *self->eventKind.resource = 0;
        self->eventKind.length = 0;
    }
    if (self->ownedObject != 0) {
        if (self->ownedObject != 0) {
            ((ResourceInterface*)self->ownedObject)->Destroy(1);
        }
        self->ownedObject = 0;
    }
    if (&self->eventKind != 0 && (self->eventKind.flags >> 31) != 0)
        appFreeUni__FPv(self->eventKind.resource);
    if (&self->eventName != 0 && (self->eventName.flags >> 31) != 0)
        appFreeUni__FPv(self->eventName.resource);
}

void Vec3_Zero(void*);
extern float kFloat_0_StoryConte;
struct StoryModelSlotStorage {
    unsigned char active;
    unsigned char pad01[3];
    unsigned int modelStateWords[6]; // Model/controller words: only reset behavior is established here.
    unsigned int vectorA[3];
    unsigned int vectorB[3];
    unsigned int entries;
    unsigned int entryCount;
    unsigned int entryCapacity;
    float scalarDefault;
    unsigned int nameFlags;
    unsigned int nameLength;
    unsigned int nameData;
};
// Initialize an inactive story model slot with empty transforms, entries and name.
void* StoryModelSlot_Init(StoryModelSlotStorage* self) {
    self->entries = 0;
    self->entryCount = 0;
    self->entryCapacity = 0;
    self->nameFlags = 0;
    self->nameLength = 0;
    self->nameData = 0;
    self->active = 0;
    self->modelStateWords[0] = 0;
    self->modelStateWords[1] = 0;
    self->modelStateWords[2] = 0;
    self->modelStateWords[3] = 0;
    self->modelStateWords[4] = 0;
    self->modelStateWords[5] = 0;
    Vec3_Zero(&self->vectorA);
    Vec3_Zero(&self->vectorB);
    self->scalarDefault = kFloat_0_StoryConte;
    if ((self->nameFlags >> 31) == 0) {
        ((signed char*)self)[0x45] = 0;
        ((signed char*)self)[0x44] &= ~0x7f;
    } else {
        *(unsigned char*)self->nameData = 0;
        self->nameLength = 0;
    }
    return self;
}

struct EventObjectNodeView {
    EventObjectNodeView* child0;
    EventObjectNodeView* child4;
    unsigned int parentColor;
    unsigned int pad0C;
    void* item;
};
// Destroy tree payload objects across an iterator range without freeing the tree nodes.
void CEventHandle::DestroyTreeRangeObjects(EventObjectNodeView** cursor, EventObjectNodeView** limit) {
    int zero = 0;
    if (*cursor == *limit)
        return;
    do {
        void* item = (*cursor)->item;
        if (item != 0) {
            if (item != 0) {
                typedef void (*Destroy)(void*, int);
                Destroy destroy = *(Destroy*)((unsigned int)((ScriptObjectVtable20*)item)->vtable + 8);
                destroy(item, 1);
            }
            (*cursor)->item = (void*)zero;
        }
        if (((EventObjectNodeView*)*cursor)->child4 != 0) {
            EventObjectNodeView* node = ((EventObjectNodeView*)*cursor)->child4;
            while (node->child0 != 0)
                node = node->child0;
            *cursor = node;
        } else {
            for (;;) {
                EventObjectNodeView* node = (EventObjectNodeView*)*cursor;
                EventObjectNodeView* parent = (EventObjectNodeView*)(node->parentColor & ~1u);
                if (node == parent->child0) {
                    *cursor = parent;
                    break;
                }
                *cursor = parent;
            }
        }
    } while (*cursor != *limit);
}

void appFreeUni__FPv(void*);
struct StoryNamedEntryStorage {
    unsigned char active;
    unsigned char pad01[3];
    unsigned int entryState;
    unsigned int flags;
    unsigned int nameLength;
    void* nameData;
};
struct StoryNamedEntryArray {
    StoryNamedEntryStorage* entries;
    int count;
};
// Destroy each named entry in reverse order, then release the entry allocation.
// self is nullable; otherwise entries owns count constructed 0x14-byte records
// (count>=0). Set count to zero BEFORE visiting the suffix; deactivate/reset each
// entry, empty its name, then free heap-name bytes only when flags bit31 is set.
// Inline names occupy entry+9, with length in flags' first-byte low seven bits;
// heap names use nameData/nameLength. Emptying retains the heap flag, so the
// following branch still releases its buffer. Finally free entries, then self
// only for shouldDelete>0; return the original address (freed in that case).
// entries is not nulled: a second destruction is invalid even with flag<=0.
// Repeated enclosing null tests represent destructor layers over this same
// owner, not independent resources. Flattening them loses this function's original
// exact match with the current compiler; the phases below explain their scope.
void* StoryNamedEntryArray_Destroy(StoryNamedEntryArray* self, int shouldDelete) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    if (self->entries != 0) {
                        StoryNamedEntryStorage* entry = self->entries + self->count;
                        int count = self->count;
                        // Phase 1: unpublish the constructed range; local count still owns the walk.
                        self->count = count;
                        self->count -= count;
                        while (count != 0) {
                            --entry;
                            if (entry != 0) {
                                entry->active = 0;
                                entry->entryState = 0;
                                // Phase 2: empty inline or heap name without changing its heap flag.
                                if ((entry->flags >> 31) == 0) {
                                    ((signed char*)entry)[9] = 0;
                                    ((signed char*)&entry->flags)[0] &= ~0x7f;
                                } else {
                                    *(unsigned char*)entry->nameData = 0;
                                    entry->nameLength = 0;
                                }
                                // Phase 3: release the heap name, never the inline bytes.
                                if (&entry->flags != 0 && (entry->flags >> 31) != 0)
                                    appFreeUni__FPv(entry->nameData);
                            }
                            --count;
                        }
                        // Phase 4: release entry storage, then optionally the containing wrapper.
                        appFreeUni__FPv(self->entries);
                    }
                }
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

struct StoryModelEntryStorage {
    unsigned char active;
    unsigned char pad01[3];
    unsigned int entryState;
    unsigned int flags;
    unsigned int nameLength;
    void* nameData;
};
extern void appFreeUni__FPv(void*);
// Reset a story model slot and release its name and nested entry strings.
void* StoryModelSlot_Destroy(StoryModelSlotStorage* self, int shouldDelete) {
    if (self != 0) {
        self->active = 0;
        self->modelStateWords[0] = 0;
        self->modelStateWords[1] = 0;
        self->modelStateWords[2] = 0;
        self->modelStateWords[3] = 0;
        self->modelStateWords[4] = 0;
        self->modelStateWords[5] = 0;
        Vec3_Zero(&self->vectorA);
        Vec3_Zero(&self->vectorB);
        self->scalarDefault = kFloat_0_StoryConte;
        if ((self->nameFlags >> 31) == 0) {
            ((signed char*)self)[0x45] = 0;
            signed char flags = ((signed char*)self)[0x44];
            flags &= ~0x7f;
            ((signed char*)self)[0x44] = flags;
        } else {
            *(unsigned char*)self->nameData = 0;
            self->nameLength = 0;
        }
        if (&self->nameFlags != 0 && (self->nameFlags >> 31) != 0)
            appFreeUni__FPv((void*)self->nameData);
        if (&self->entries != 0) {
            if (&self->entries != 0) {
                if (&self->entries != 0) {
                    if (&self->entries != 0) {
                        if (self->entries != 0) {
                            int count = self->entryCount;
                            StoryModelEntryStorage* base = (StoryModelEntryStorage*)self->entries;
                            int offset = count * 0x14;
                            self->entryCount = count - count;
                            StoryModelEntryStorage* entry = (StoryModelEntryStorage*)((char*)base + offset);
                            while (count != 0) {
                                --entry;
                                if (entry != 0) {
                                    entry->active = 0;
                                    entry->entryState = 0;
                                    if ((entry->flags >> 31) == 0) {
                                        ((signed char*)entry)[9] = 0;
                                        signed char flags = ((signed char*)&entry->flags)[0];
                                        flags &= ~0x7f;
                                        ((signed char*)&entry->flags)[0] = flags;
                                    } else {
                                        *(unsigned char*)entry->nameData = 0;
                                        entry->nameLength = 0;
                                    }
                                    if (&entry->flags != 0 && (entry->flags >> 31) != 0)
                                        appFreeUni__FPv(entry->nameData);
                                }
                                --count;
                            }
                            appFreeUni__FPv((*(void**)&((StoryModelSlotStorage*)self)->entries));
                        }
                    }
                }
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

struct StoryShutdownPathStorage {
    unsigned int flags;
    unsigned int length;
    unsigned char* resource;
};
struct DA14Base24 {
    unsigned char pad24[0x24];
};
struct DA14Poly24 : DA14Base24 {
    virtual void Destroy(int);
};
struct DA14Base4A4 {
    unsigned char pad4A4[0x4A4];
};
struct DA14Poly4A4 : DA14Base4A4 {
    virtual void Destroy(int);
};
struct DA14Poly0 {
    virtual void Destroy(int);
};
// Shutdown view owns the three child handles and caption; destroy them before
// releasing shared BP resources. +0x20 is CStoryConte::eventIndex, not playback
// state +0x1C; the shutdown view names the event index separately.
struct StoryTaskShutdownView {
    unsigned char pad00[0x20];
    int eventIndex; // +0x20 event index; OnDestroy sets it to 3 before releasing resources.
    StoryShutdownPathStorage ref;
    void* eventLoader;
    void* scriptHandle;
    void* faceDataLoader;
    unsigned char pad3C[0x60];
    void* skipCaption;
    unsigned char padA0[0x14];
};
// Display borrows the caption handle; OnDestroy owns its destruction.
struct StoryTaskDisplayView {
    unsigned char pad00[0x1c];
    int state;
    unsigned char pad20[0x7c];
    void* skipCaption;
};
extern signed char s_bpPlayerInitialized;
extern unsigned char s_bpPlayer[];
extern unsigned char s_conteModelSlots[];
extern unsigned char s_partyBpPlayerDtorRegistration[];
extern unsigned char s_storyConteDtorRegistrations[];
extern unsigned char s_effectFade[];
extern void __ct__9CBpPlayerFv(void*);
extern void __dt__9CBpPlayerFv();
extern void __register_global_object(void*, void (*)(), void*);
extern void ScriptSharedFiles_RemoveRequiredFilesAndRelease();
// Node-only postorder cleanup for CEventHandle::objectTree (+470).
// The header owns nodes; payload destruction has already cleared each item.
// The three scopes below express postorder cleanup at increasing unroll depth.
// An isolated inline-helper experiment changed Tree_Clear's code/relocations;
// retain macro-local lifetimes for the required byte identity. Each node input
// is evaluated once; owner is borrowed, and no payload destructor runs here.
// Release one fringe node after delegating both child subtrees; null is a no-op.
#define FREE_OBJECT_TREE_FRINGE(owner, p)                       \
    do {                                                        \
        EventObjectTreeLinksView* leaf = (p);                   \
        if (leaf != 0) {                                        \
            if (leaf->first != 0)                               \
                ScriptByteMap_FreeSubtree(owner, leaf->first);  \
            if (leaf->second != 0)                              \
                ScriptByteMap_FreeSubtree(owner, leaf->second); \
            appFreeUni__FPv(leaf);                              \
        }                                                       \
    } while (0)
// Release two levels in child-first order, without destroying payloads again.
#define FREE_OBJECT_TREE_BRANCH(owner, p)                   \
    do {                                                    \
        EventObjectTreeLinksView* branch = (p);             \
        if (branch != 0) {                                  \
            FREE_OBJECT_TREE_FRINGE(owner, branch->first);  \
            FREE_OBJECT_TREE_FRINGE(owner, branch->second); \
            appFreeUni__FPv(branch);                        \
        }                                                   \
    } while (0)
// Release three levels with the same postorder; deeper children use shared cleanup.
#define FREE_OBJECT_TREE_SUBTREE(owner, p)                   \
    do {                                                     \
        EventObjectTreeLinksView* subtree = (p);             \
        if (subtree != 0) {                                  \
            FREE_OBJECT_TREE_BRANCH(owner, subtree->first);  \
            FREE_OBJECT_TREE_BRANCH(owner, subtree->second); \
            appFreeUni__FPv(subtree);                        \
        }                                                    \
    } while (0)
struct EventObjectTreeLinksView {
    EventObjectTreeLinksView* first;
    EventObjectTreeLinksView* second;
};
extern void ScriptByteMap_FreeSubtree(void*, void*);
// Free the entire object tree and restore its empty sentinel links.
void Tree_Clear(EventObjectTreeLinksView** self) {
    EventObjectTreeLinksView** owner = self;
    EventObjectTreeLinksView* root = (EventObjectTreeLinksView*)self[1];
    if (root != 0) {
        FREE_OBJECT_TREE_SUBTREE(owner, root->first);
        FREE_OBJECT_TREE_SUBTREE(owner, root->second);
        appFreeUni__FPv(root);
        owner[0] = 0;
        owner[1] = 0;
        owner[2] = (EventObjectTreeLinksView*)&owner[1];
    }
}
struct EventResourceTreeLinksView {
    EventResourceTreeLinksView* first;
    EventResourceTreeLinksView* second;
};
struct EventResourceTreeHeaderView {
    void* first;
    EventResourceTreeLinksView* root;
    EventResourceTreeLinksView** tail;
};
extern void ScriptByteMap_FreeSubtree(void*, void*);
// Free the entire resource tree and restore its empty sentinel links.
void CEventHandle::ClearTree() {
    EventResourceTreeHeaderView* self = (EventResourceTreeHeaderView*)this;
    EventResourceTreeLinksView* root = self->root;
    if (root != 0) {
        EventResourceTreeLinksView* branch = root->first;
        if (branch != 0) {
            EventResourceTreeLinksView* leaf = branch->first;
            if (leaf != 0) {
                if (leaf->first != 0)
                    ScriptByteMap_FreeSubtree(self, leaf->first);
                if (leaf->second != 0)
                    ScriptByteMap_FreeSubtree(self, leaf->second);
                appFreeUni__FPv(leaf);
            }
            leaf = branch->second;
            if (leaf != 0) {
                if (leaf->first != 0)
                    ScriptByteMap_FreeSubtree(self, leaf->first);
                if (leaf->second != 0)
                    ScriptByteMap_FreeSubtree(self, leaf->second);
                appFreeUni__FPv(leaf);
            }
            appFreeUni__FPv(branch);
        }
        branch = root->second;
        if (branch != 0) {
            EventResourceTreeLinksView* subbranch = branch->first;
            if (subbranch != 0) {
                EventResourceTreeLinksView* leaf = subbranch->first;
                if (leaf != 0) {
                    if (leaf->first != 0)
                        ScriptByteMap_FreeSubtree(self, leaf->first);
                    if (leaf->second != 0)
                        ScriptByteMap_FreeSubtree(self, leaf->second);
                    appFreeUni__FPv(leaf);
                }
                leaf = subbranch->second;
                if (leaf != 0) {
                    if (leaf->first != 0)
                        ScriptByteMap_FreeSubtree(self, leaf->first);
                    if (leaf->second != 0)
                        ScriptByteMap_FreeSubtree(self, leaf->second);
                    appFreeUni__FPv(leaf);
                }
                appFreeUni__FPv(subbranch);
            }
            subbranch = branch->second;
            if (subbranch != 0) {
                EventResourceTreeLinksView* leaf = subbranch->first;
                if (leaf != 0) {
                    if (leaf->first != 0)
                        ScriptByteMap_FreeSubtree(self, leaf->first);
                    if (leaf->second != 0)
                        ScriptByteMap_FreeSubtree(self, leaf->second);
                    appFreeUni__FPv(leaf);
                }
                leaf = subbranch->second;
                if (leaf != 0) {
                    if (leaf->first != 0)
                        ScriptByteMap_FreeSubtree(self, leaf->first);
                    if (leaf->second != 0)
                        ScriptByteMap_FreeSubtree(self, leaf->second);
                    appFreeUni__FPv(leaf);
                }
                appFreeUni__FPv(subbranch);
            }
            appFreeUni__FPv(branch);
        }
        appFreeUni__FPv(root);
        self->first = 0;
        self->root = 0;
        self->tail = &self->root;
    }
}
#undef FREE_OBJECT_TREE_SUBTREE
#undef FREE_OBJECT_TREE_BRANCH
#undef FREE_OBJECT_TREE_FRINGE
#pragma push
#pragma dont_inline off
#pragma auto_inline off
// Release story loaders, active script and skip caption, then reset shared message resources.
// Required task, called before dtor; writes +0x20 event index=3 first and +0x1C
// playback state=idle last. Existing order releases event loader before handle;
// cleanup must not start executing scripts again. Preserve this order and the
// repeated null guards/virtual receiver adjustments; no error/status result.
void CStoryConte::OnDestroy() {
    StoryTaskShutdownView* self = (StoryTaskShutdownView*)this;
    self->eventIndex = 3;
    if ((self->ref.flags >> 31) == 0) {
        AsCStoryConte(self)->inlineEventPath = 0;
        AsCStoryConte(self)->eventPathFlags &= ~0x7f;
    } else {
        *self->ref.resource = 0;
        self->ref.length = 0;
    }
    void* child = self->eventLoader;
    if (child != 0) {
        if (child != 0) {
            ((DA14Poly24*)child)->Destroy(1);
        }
        self->eventLoader = 0;
    }
    child = self->scriptHandle;
    if (child != 0) {
        if (child != 0) {
            ((DA14Poly4A4*)child)->Destroy(1);
        }
        self->scriptHandle = 0;
    }
    child = self->faceDataLoader;
    if (child != 0) {
        if (child != 0) {
            ((DA14Poly24*)child)->Destroy(1);
        }
        self->faceDataLoader = 0;
    }
    child = self->skipCaption;
    if (child != 0) {
        if (child != 0) {
            ((DA14Poly0*)child)->Destroy(1);
        }
        self->skipCaption = 0;
    }
    if (((volatile int&)Sing_805DC670()) == 0) {
        if (s_bpPlayerInitialized == 0) {
            ((CBpPlayer*)s_bpPlayer)->__ct();
            __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, s_partyBpPlayerDtorRegistration);
            s_bpPlayerInitialized = 1;
        }
        ((volatile int&)Sing_805DC670()) = (int)s_bpPlayer;
    }
    ((CBpPlayer*)(((volatile int&)Sing_805DC670())))->ReleaseResources();
    AsCStoryConte(self)->state = 0;
    if (GameHook_ReservedFalse() == 0)
        ScriptSharedFiles_RemoveRequiredFilesAndRelease();
}
#pragma pop

extern unsigned int kFloatBits_20_StorySkipCaptionRightMargin;
extern unsigned int kFloatBits_48_StorySkipCaptionTop;
extern unsigned int kFloatBits_128_StorySkipCaptionWidth;
extern unsigned int kFloatBits_32_StorySkipCaptionHeight;
extern float kFloat_640_StoryConte;
extern unsigned char s_storySkipCaptionQuadTemplate[];
extern void StoryConte_DrawPanelAndMessages(void*);
extern void Engine_SetFlexibleVertexFormat(int);
extern void* GX_GetPrimitiveCount(int, int);
extern void DrawPrimitiveUP(int, void*, void*, int);
struct StoryCaptionVertex {
    float x, y;
    unsigned int color;
    float u, v;
    unsigned int pad;
};
struct StoryCaptionDrawData {
    unsigned int temp[6];
    StoryCaptionVertex vertices[4];
};
// Draw messages during playback/fading and the skip-caption quad at the upper right.
// Required task; draws only states 3/5. Null caption skips its quad, not messages.
// Caption slot +0x14 prepares binding. Vertex xy uses reference layout pixels,
// x right/y down; UV/tint stay from the template. No allocations, output status,
// viewport or scissor writes. Preserve volatile template word-copy ordering.
void CStoryConte::OnDisplay() {
    StoryTaskDisplayView* self = (StoryTaskDisplayView*)this;
    // Draw in 640-wide UI coordinates: left=640-width(128)-margin(20), top=48,
    // bottom=top+height(32). UV/color data comes from the quad template.
    if (kStoryPlaying == self->state || kStorySkipFade == self->state) {
        if (((volatile int&)Sing_805DC670()) == 0) {
            if (s_bpPlayerInitialized == 0) {
                ((CBpPlayer*)s_bpPlayer)->__ct();
                __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, s_partyBpPlayerDtorRegistration);
                s_bpPlayerInitialized = 1;
            }
            ((volatile int&)Sing_805DC670()) = (int)s_bpPlayer;
        }
        StoryConte_DrawPanelAndMessages((void*)((volatile int&)Sing_805DC670()));
        if (self->skipCaption != 0) {
            StoryCaptionDrawData data;
            data.temp[5] = kFloatBits_48_StorySkipCaptionTop;

            data.temp[2] = kFloatBits_128_StorySkipCaptionWidth;
            data.temp[4] = kFloatBits_20_StorySkipCaptionRightMargin;
            float left = kFloat_640_StoryConte - *(volatile float*)&data.temp[2] - *(volatile float*)&data.temp[4];

            volatile unsigned int* src = (volatile unsigned int*)s_storySkipCaptionQuadTemplate - 1;
            volatile unsigned int* dst = (volatile unsigned int*)data.vertices - 1;
            int i = 12;
            data.temp[3] = kFloatBits_32_StorySkipCaptionHeight;
            do {
                *++dst = *++src;
                *++dst = *++src;
            } while (--i != 0);
            float top = *(volatile float*)&data.temp[5];
            float captionHeight = *(volatile float*)&data.temp[3];
            data.vertices[0].x = left;
            float x2 = left + (*(volatile float*)&data.temp[2]);
            float y2 = (captionHeight + top);
            data.vertices[1].x = x2;
            data.vertices[0].y = top;
            data.vertices[1].y = top;
            data.vertices[2].y = y2;
            data.vertices[3].x = x2;
            data.vertices[2].x = left;
            data.vertices[3].y = y2;
            ((void (*)(void*))(*(void**)(*(unsigned int*)self->skipCaption + 0x14)))(self->skipCaption);
            Engine_SetFlexibleVertexFormat(kStoryCaptionVertexFormat);
            void* primitiveCount = GX_GetPrimitiveCount(kStoryCaptionVertexCount, kStoryCaptionStrip);
            DrawPrimitiveUP(kStoryCaptionStrip, primitiveCount, data.vertices, kStoryCaptionVertexBytes);
        }
    }
}

extern signed char s_effectFadeInitialized;
extern signed char s_courseSlotTableInitialized;
extern volatile int g_soundManager;
extern volatile int s_nowReloading;
extern void* __nwa__FUl(unsigned int);
extern void __ct__11CEffectFadeFv(void*);
extern void __dt__11CEffectFadeFv();
extern void SoundManager_RequestBgmStop(void*, int);
extern void SoundManager_StopAndClearChannels(void*);
extern void SoundManager_AcquireSoundBank(void*, void*);
extern void SoundManager_ReleaseSoundBank(void*, void*);
extern void __destroy_new_array(void*, void*);
extern void* __construct_new_array(void*, void*, void*, unsigned int, int);
extern int StoryMovieConfig_SetDisplayModeIfChanged(void*, int);
extern void StoryMovieConfig_ApplyDisplayMode(void*);
extern void __dt__13CNowReloadingFv();
extern int LoadingIndicator_IsAbsentOrState3(void*);
extern void LoadingIndicator_CreatePlayer(void*);

// Tagged message ABI (all pointers borrowed for the duration of HandleMsg):
// type 0 ENTER / 1 LEAVE: primaryPayload/secondaryPayload unused; acquires/releases shared model slots and audio bank.
// type 2 START: primaryPayload is an integer event index encoded in a pointer-sized word,
//              NOT a pointer to an index; secondaryPayload is a nonzero flag requesting fresh async loads.
//              Index domain is 0/1: SharedStringState_GetFirstString (story_event_script_shared_state.cpp)
//              selects the first string of the primary/secondary pair respectively.
//              Other integers return null there and create no event loader on
//              the fresh-load path; they do not select another table entry.
//              ctor/OnDestroy use 3 as the no-event sentinel, not a third event.
//              START still publishes the supplied word/state without validating
//              it. With secondaryPayload==0 the caller must already have loaded
//              buffers for the chosen event; changing the index does not load them.
// type 3 QUERY_COMPLETE: primaryPayload points to writable int, always set to (state==idle);
//              a true result also makes the fade opaque. secondaryPayload unused.
// type 4 QUERY_EVENT_NAMES: primaryPayload points to int, set if player-0 name exists;
//              optional secondaryPayload points to int and is cleared to zero (not filled with a name).
// Payload word meanings depend on type; neither word owns the pointee.
// The names describe their order, not a common payload structure.
struct StoryCommandMessage {
    int pad0;
    int type;
    void* primaryPayload;
    void* secondaryPayload;
};

struct EB68V24 {
    unsigned char pad[0x24];
    virtual void Destroy(int);
};
struct EB68V0 {
    virtual void Destroy(int);
    virtual void SetName(void*);
};

void __dla__FPv(void*);
void* __dl__FPv(void*);

// Handle story entry, exit, scene-start and status queries, including audio/resource lifetime.
// ENTER stops audio and replaces the shared slot array; LEAVE frees it and releases
// the bank. START replaces owned async loaders and caption; queries borrow output words.
// Service getters preserve separate volatile samples and persistent registration storage.
// Publication/failure ledger (exceptions propagate; this is not a transaction):
// ENTER acquires the bank BEFORE resetting the shared table to {FFFF,0,null}.
// Reset destroys the old array. The new array is published at table+8 only
// after __construct_new_array returns; a constructor throw frees raw storage,
// leaves table+8 null, and leaves the bank acquired and fade/audio changes in
// effect. There is no catch here to release the bank; lifecycle LEAVE is the
// corresponding release. A null allocation publishes null too; it is not retried.
// START first publishes state=2 and eventIndex, applies display configuration,
// and may create the loading indicator. With secondaryPayload==0 it stops there
// for OnLoad; with nonzero payload the replacement sequence is:
//   resolve path -> destroy/null old event -> construct/publish new event ->
//   destroy/null old face -> construct new face -> ensure BP -> publish face ->
//   release/start/load BP messages -> destroy/null old caption -> construct/
//   publish new caption -> SetName -> state=1.
// An event-constructor throw frees its raw block, leaving event=null and the OLD
// face/caption intact. A face-constructor throw leaves the NEW event installed,
// face=null and old caption intact. A caption-constructor throw leaves both new
// loaders installed and caption=null; BP resources have already changed. State
// remains 2 on these throws. None restores released predecessors or the old
// event index/display mode. Face publication is deliberately AFTER Ensure BP:
// if that setup throws after face construction, the local face is not yet in
// self and no local guard releases it. Likewise SetName can fail after caption
// publication, before state=1. These ordering limits must remain visible to mods.
// Normal task OnDestroy releases published loaders/caption; shared slots/bank
// are paired by ENTER/LEAVE, rather than by a rollback inside START.
void CStoryConte::HandleMsg(StoryCommandMessage* arg) {
    StoryLoadView* self = (StoryLoadView*)this;
    unsigned char* tags = s_storyConteDtorRegistrations;
    switch (arg->type) {
    case kStoryEnter: {
        // ENTER A: stop prior audio and acquire the story sound bank.
        do {
            if (!((volatile int&)Sing_805DC660())) {
                if (!s_effectFadeInitialized) {
                    ((CEffectFade*)s_effectFade)->__ct();
                    __register_global_object(s_effectFade, __dt__11CEffectFadeFv, tags);
                    s_effectFadeInitialized = 1;
                }
                ((volatile int&)Sing_805DC660()) = (int)s_effectFade;
            }
        } while (0);
        ((CEffectFade*)(((volatile int&)Sing_805DC660())))->SetAlpha(0xff);
        SoundManager_RequestBgmStop((void*)g_soundManager, 0);
        SoundManager_StopAndClearChannels((void*)g_soundManager);
        SoundManager_AcquireSoundBank((void*)g_soundManager, StoryStrings.soundBank);
        do {
            if (!((volatile int&)GetConteModelSlotsSlot())) {
                if (!s_courseSlotTableInitialized) {
                    *(int*)(s_conteModelSlots + 8) = 0;
                    do {
                        unsigned char* slotTable = (s_conteModelSlots);
                        *(unsigned short*)slotTable = 0xffff;
                        *(int*)(slotTable + 4) = 0;
                        void* oldSlotArray = *(void**)(slotTable + 8);
                        if (oldSlotArray != 0) {
                            __destroy_new_array(oldSlotArray, (void*)StoryModelSlot_Destroy);
                            *(int*)(slotTable + 8) = 0;
                        }
                    } while (0);
                    s_courseSlotTableInitialized = 1;
                }
                ((volatile int&)GetConteModelSlotsSlot()) = (int)s_conteModelSlots;
            }
        } while (0);
        // ENTER/LEAVE B: reset shared slots; this table owns its array and slot payloads.
        unsigned char* modelSlotTable = (unsigned char*)((volatile int&)GetConteModelSlotsSlot());
        do {
            unsigned char* slotTable = (modelSlotTable);
            *(unsigned short*)slotTable = 0xffff;
            *(int*)(slotTable + 4) = 0;
            void* oldSlotArray = *(void**)(slotTable + 8);
            if (oldSlotArray != 0) {
                __destroy_new_array(oldSlotArray, (void*)StoryModelSlot_Destroy);
                *(int*)(slotTable + 8) = 0;
            }
        } while (0);
        // ENTER replaces the shared model-slot array after stopping audio
        // and acquiring the story bank. Array storage includes the runtime
        // cookie: 0x1A0 bytes = 5 * 0x50-byte slots + 0x10-byte header.
        // Construct failure frees the raw allocation; publication is last.
        void* slotArrayStorage = __nwa__FUl(kStoryModelArrayBytes);
        if (slotArrayStorage) {
            try {
                slotArrayStorage = __construct_new_array(slotArrayStorage, StoryModelSlot_Init, StoryModelSlot_Destroy,
                                                         kStoryModelSlotBytes, kStoryModelSlotCount);
            } catch (...) {
                __dla__FPv(slotArrayStorage);
                throw;
            }
        }
        *(void**)((char*)modelSlotTable + 8) = slotArrayStorage;
        break;
    }
    case kStoryLeave: {
        // LEAVE destroys all shared model slots before releasing the bank.
        do {
            if (!((volatile int&)GetConteModelSlotsSlot())) {
                if (!s_courseSlotTableInitialized) {
                    *(int*)(s_conteModelSlots + 8) = 0;
                    do {
                        unsigned char* slotTable = (s_conteModelSlots);
                        *(unsigned short*)slotTable = 0xffff;
                        *(int*)(slotTable + 4) = 0;
                        void* oldSlotArray = *(void**)(slotTable + 8);
                        if (oldSlotArray != 0) {
                            __destroy_new_array(oldSlotArray, (void*)StoryModelSlot_Destroy);
                            *(int*)(slotTable + 8) = 0;
                        }
                    } while (0);
                    s_courseSlotTableInitialized = 1;
                }
                ((volatile int&)GetConteModelSlotsSlot()) = (int)s_conteModelSlots;
            }
        } while (0);
        // ENTER/LEAVE B: reset shared slots; this table owns its array and slot payloads.
        unsigned char* modelSlotTable = (unsigned char*)((volatile int&)GetConteModelSlotsSlot());
        do {
            unsigned char* slotTable = (modelSlotTable);
            *(unsigned short*)slotTable = 0xffff;
            *(int*)(slotTable + 4) = 0;
            void* oldSlotArray = *(void**)(slotTable + 8);
            if (oldSlotArray != 0) {
                __destroy_new_array(oldSlotArray, (void*)StoryModelSlot_Destroy);
                *(int*)(slotTable + 8) = 0;
            }
        } while (0);
        SoundManager_RequestBgmStop((void*)g_soundManager, 0);
        SoundManager_StopAndClearChannels((void*)g_soundManager);
        SoundManager_ReleaseSoundBank((void*)g_soundManager, StoryStrings.soundBank);
        do {
            if (!((volatile int&)Sing_805DC660())) {
                if (!s_effectFadeInitialized) {
                    ((CEffectFade*)s_effectFade)->__ct();
                    __register_global_object(s_effectFade, __dt__11CEffectFadeFv, tags);
                    s_effectFadeInitialized = 1;
                }
                ((volatile int&)Sing_805DC660()) = (int)s_effectFade;
            }
        } while (0);
        ((CEffectFade*)(((volatile int&)Sing_805DC660())))->SetAlpha(0);
        break;
    }
    case kStoryStart: {
        // START A: settle display mode, record scene index, and show the loading indicator.
        if (StoryMovieConfig_SetDisplayModeIfChanged((char*)self + FIELD_OFFSET(CStoryConte, displayConfiguration),
                                                     4) != 0)
            StoryMovieConfig_ApplyDisplayMode(&AsCStoryConte(self)->displayConfiguration);
        int indicatorSingletonWord = ((volatile int&)Sing_805DC610());
        void* eventIndexWord = arg->primaryPayload;
        AsCStoryConte(self)->state = 2;
        (*(void**)&AsCStoryConte(self)->eventIndex) = eventIndexWord;
        if (!indicatorSingletonWord) {
            if (!s_nowReloadingInitialized) {
                ((CNowReloading*)(&s_nowReloading))->Init();
                __register_global_object((void*)&s_nowReloading, __dt__13CNowReloadingFv, (tags + 0x24));
                s_nowReloadingInitialized = 1;
            }
            ((volatile int&)Sing_805DC610()) = (int)(void*)&s_nowReloading;
        }
        void* loadingIndicator = (void*)((volatile int&)Sing_805DC610());
        if (loadingIndicator != 0) {
            if (LoadingIndicator_IsAbsentOrState3(loadingIndicator) != 0)
                LoadingIndicator_CreatePlayer(loadingIndicator);
        }
        // START B: a nonzero mode word requests new async loaders; otherwise use OnLoad.
        if (arg->secondaryPayload != 0) {
            char* text = SharedStringState_GetFirstString((*(int*)&AsCStoryConte(self)->eventIndex));
            char* eventPath;
            if (0 == text) {
                eventPath = 0;
            } else {
                unsigned char temps[0x18];
                char* start = StoryStrings.emptyPathPrefix;
                int size;
                size = (self->string.flags >> 31) == 0 ? (unsigned int)(self->string.shortData[0] & 0x7f)
                                                       : self->string.longLength;
                temps[4] = temps[0];
                String_ReplaceRange(&self->string, 0, size, start, start + strlen(start), &temps[4]);
                size = (self->string.flags >> 31) == 0 ? (unsigned int)(self->string.shortData[0] & 0x7f)
                                                       : self->string.longLength;
                temps[12] = temps[8];
                String_ReplaceRange(&self->string, size, 0, text, text + strlen(text), &temps[12]);
                start = StoryStrings.objectExtension;
                size = (self->string.flags >> 31) == 0 ? (unsigned int)(self->string.shortData[0] & 0x7f)
                                                       : self->string.longLength;
                temps[20] = temps[16];
                String_ReplaceRange(&self->string, size, 0, start, start + strlen(start), &temps[20]);
                eventPath = (self->string.flags >> 31) == 0 ? (char*)&AsCStoryConte(self)->inlineEventPath
                                                            : (*(char**)&AsCStoryConte(self)->eventPathData);
            }
            // START C: replace owned loaders only after resolving the event path.
            if (self->eventLoader != 0) {
                typedef void (*Destroy)(void*, int);
                void* ownedResource = self->eventLoader;
                if (ownedResource != 0) {
                    ((EB68V24*)ownedResource)->Destroy(1);
                }
                self->eventLoader = 0;
            }
            void* eventLoader = __nw__FUl(kStoryFileLoaderBytes);
            if (eventLoader != 0) {
                try {
                    ((CEventLoad*)eventLoader)->__ct(eventPath, 0, (void*)1);
                } catch (...) {
                    __dl__FPv(eventLoader);
                    throw;
                }
            }
            self->eventLoader = eventLoader;
            if (self->faceDataLoader != 0) {
                typedef void (*Destroy)(void*, int);
                void* ownedResource = self->faceDataLoader;
                if (ownedResource != 0) {
                    ((EB68V24*)ownedResource)->Destroy(1);
                }
                self->faceDataLoader = 0;
            }
            void* faceDataLoader = __nw__FUl(kStoryFileLoaderBytes);
            if (faceDataLoader) {
                try {
                    ((CEventLoad*)faceDataLoader)->__ct((const char*)g_eventFaceDataFilename, 0, (void*)1);
                } catch (...) {
                    __dl__FPv(faceDataLoader);
                    throw;
                }
            }
            do {
                if (!((volatile int&)Sing_805DC670())) {
                    if (!s_bpPlayerInitialized) {
                        ((CBpPlayer*)s_bpPlayer)->__ct();
                        __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, (tags + 0x18));
                        s_bpPlayerInitialized = 1;
                    }
                    ((volatile int&)Sing_805DC670()) = (int)s_bpPlayer;
                }
            } while (0);
            self->faceDataLoader = faceDataLoader;
            ((CBpPlayer*)(((volatile int&)Sing_805DC670())))->ReleaseResources();
            do {
                if (!((volatile int&)Sing_805DC670())) {
                    if (!s_bpPlayerInitialized) {
                        ((CBpPlayer*)s_bpPlayer)->__ct();
                        __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, (tags + 0x18));
                        s_bpPlayerInitialized = 1;
                    }
                    ((volatile int&)Sing_805DC670()) = (int)s_bpPlayer;
                }
            } while (0);
            void* tgt = (void*)((volatile int&)Sing_805DC670());
            void* name = ((*(unsigned int*)&AsCStoryConte(self)->playerResourceNameFlags) >> 31) == 0
                             ? (void*)((char*)&AsCStoryConte(self)->inlinePlayerResourceName)
                             : AsCStoryConte(self)->playerResourceNameData;
            ((CBpPlayer*)tgt)->StartLoad((const char*)name, 0, (void*)1);
            do {
                if (!((volatile int&)Sing_805DC670())) {
                    if (!s_bpPlayerInitialized) {
                        ((CBpPlayer*)s_bpPlayer)->__ct();
                        __register_global_object(s_bpPlayer, __dt__9CBpPlayerFv, (tags + 0x18));
                        s_bpPlayerInitialized = 1;
                    }
                    ((volatile int&)Sing_805DC670()) = (int)s_bpPlayer;
                }
            } while (0);
            ((CBpPlayer*)((volatile int&)Sing_805DC670()))->LoadMessageResources();
            if (self->skipCaption != 0) {
                typedef void (*Destroy)(void*, int);
                void* ownedResource = self->skipCaption;
                if (0 != ownedResource) {
                    ((EB68V0*)ownedResource)->Destroy(1);
                }
                self->skipCaption = 0;
            }
            void* ownedResource = __nw__FUl(0x28);
            if (ownedResource != 0) {
                try {
                    ((CBpTex*)ownedResource)->__ct();
                } catch (...) {
                    __dl__FPv(ownedResource);
                    throw;
                }
            }
            self->skipCaption = ownedResource;
            ((EB68V0*)ownedResource)->SetName(StoryStrings.skipCaptionFile);
            AsCStoryConte(self)->state = 1;
        }
        break;
    }
    case kStoryQueryComplete: {
        // QUERY: output words are borrowed writable storage; no loader ownership changes.
        unsigned int isIdle = (AsCStoryConte(self)->state == 0) ? 1 : 0;
        *(int*)arg->primaryPayload = isIdle;
        if (isIdle) {
            do {
                if (!((volatile int&)Sing_805DC660())) {
                    if (!s_effectFadeInitialized) {
                        ((CEffectFade*)s_effectFade)->__ct();
                        __register_global_object(s_effectFade, __dt__11CEffectFadeFv, tags);
                        s_effectFadeInitialized = 1;
                    }
                    ((volatile int&)Sing_805DC660()) = (int)s_effectFade;
                }
            } while (0);
            ((CEffectFade*)(((volatile int&)Sing_805DC660())))->SetAlpha(0xff);
        }
        break;
    }
    case kStoryQueryEventNames: {
        unsigned int v = (int)SharedStringState_GetFirstString(0);
        int* secondaryPayload = (int*)arg->secondaryPayload;
        *(int*)arg->primaryPayload = (((v != 0)) ? 1 : 0);
        if (0 != secondaryPayload)
            *secondaryPayload = 0;
        break;
    }
    }
}

} // extern "C"
