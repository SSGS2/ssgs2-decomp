// Training and single-player tasks: the Training and single-player (Stroke and Match) main menus and
// course-selection actors, their task transitions, and the release of their selection results.

#include <decomp/static_init.h>
// Free Training and single-player Stroke/Match selection actors and task transitions.
// Modding lookup:
// - Frame rate/readiness: CSingleMainRev::OnProcess forwards dt to the owned
//   selector through ProcessSelectorWithTime; CTrainingMain_OnProcess separately
//   waits for BP loads before starting scripts. Neither defines a frame limiter,
//   fixed delay, save/load pacing counter or a numeric clock scalar here.
// - Course loading: CourseSelection_ApplyRoundSettings publishes menu results;
//   GolfPlayerList_BuildFromRenderTable publishes saved equipment/players before
//   CTask::RequestTransitionWithFade requests the golf task. GBIN/PET file loading
//   occurs downstream, not at the course-index assignments in this file.
// - Widescreen: CTrainingMain::OnDisplay orders background -> selector -> caption.
//   context at 0x8024904C. It is a draw consumer, not an aspect/resolution knob.
//   Edit B2D/SPD layout and the shared projection policy together for title tabs.
// Default hole count is kDefaultHoleCount (9 holes); title return IDs are
// kSingleTitleMenuIndex / kTrainingTitleMenuIndex, not frame counts.
// Shared schemas: CSharedDoc.h, SavePlayerRecord.h, MPBase1Layout.h; local result
// vectors own allocations only until round publication finishes. Preserve the
// result-query/completion distinction, float dt forwarding and cleanup order.
// Message lookup: CSingleMainRev::HandleMsg is the actor's +2C message hook
// (phase1_class_tables.cpp), receiving the synchronous IActor_DispatchMessage
// packet {sender,+4 messageType,...}; only types 0/1 are read here. See the
// local transition API contract there before injecting a selector event.
#include "MSL/vector.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
typedef std::vector<void*, app::app_allocator<void*> > MSLPointerVector;
enum {
    kSingleMainSecondaryVtableOffset = 0x3C
};
#include "iff/IffStruct.h"
// Address-only layout of the existing string pool; no new string storage.
struct SingleTransitionStringPool {
    char selectBgm[11];        // 0x0
    char golfTask[10];         // 0xB
    char golfDocument[9];      // 0x15
    char titleTask[11];        // 0x1E
    char emptyTitleContext[1]; // 0x29
    char titleMain[11];        // 0x2A
};

// Address-only layout of the existing string pool; no new string storage.
struct TrainingStringPool {
    char backgroundResource[17]; // 0x0
    char captionResource[21];    // 0x11
    char selectBgm[11];          // 0x26
    char backgroundGroup[3];     // 0x31
    char titleGroup[6];          // 0x34
    char golfTask[10];           // 0x3A
    char golfDocument[9];        // 0x44
    char titleTask[11];          // 0x4D
    char emptyTitleContext[1];   // 0x58
    char titleMain[11];          // 0x59
};

#include "classes/SavePlayerRecord.h"
#include "classes/CMouseCursor.h"
#include "classes/MPBase1Layout.h"

enum SingleMenuState {
    kSingleMenuChoose = 0,
    kSingleMenuStroke = 1,
    kSingleMenuMatch = 2,
    kSingleMenuStartingRound = 3
};
enum {
    kStrokeRule = 0,
    kMatchRule = 1,
    kSinglePlayerGameMode = 0,
    kTrainingGameMode = 5,
    kDefaultHoleCount = 9,
    kSingleTitleMenuIndex = 2,
    kTrainingTitleMenuIndex = 4,
    kPlayerRenderRecordBytes = 0xA4,
    kPlayerRenderRecordCount = 4,
    kTrainingHiddenGroupCount = 9,
    kGolfCursorHitTestMode = 1
};

// Single selection state: 0 waits for HandleMsg, 1 owns a stroke selector, 2 owns
// a match selector, 3 has requested the golf transition and removed the selector.
// Process completion is followed by a separate selection-result query: true
// accepts the course, false cancels to title. Both paths must release result data.
// The selection prefix is four bytes, followed by two pointer/count/capacity
// vectors. Stroke results additionally carry selection metadata at +0x1C..+0x2B.
// Render scratch storage holds four 0xA4-byte records and a count at +0x290.

// Selection result subobjects are pointer/count/capacity vectors, not arbitrary integer words.
struct SelectionVector {
    void* data;
    volatile int size;
    unsigned int capacity;
};
struct CourseSelectionStorage {
    void* vtable;
    SelectionVector holeNumbers;
    SelectionVector courseIds;
    char renderRecords[1]; // address-only view; owned by the selection-result destructor
};
struct MenuEvent {
    void* sender;
    int type;
};
// Address-only extension views for CSharedDoc regions not yet placed in its shared header.
struct SharedRuleName {
    char padding[0x8C1];
    char name[1];
};
struct SharedSingleSelection {
    char padding[0xB70];
    short code;
    char bytes[12];
};

#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
extern "C" void* dtor_80046918(void*, int);

// Normal destruction remains explicit. This owner handles exceptional exits.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Adopt exceptional-exit cleanup; normal destruction remains explicit.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Disarm exceptional cleanup after explicit normal-path destruction.
    void release() {
        active = false;
    }
    // Run the subobject destructor only while this guard owns cleanup.
    ~EH5Cleanup() {
        if (active)
            destroy(object, -1);
    }
};

#include "game/string.h"

// Stack-local narrow-string view; the canonical string ABI remains external.
class LocalNString_u80248934 {
public:
    unsigned int flags;
    unsigned int length;
    char* data;

    // Initialize the stack-local engine string using the selected constructor ABI.
    LocalNString_u80248934() : flags(0), length(0), data(0) {}
#if defined(GAME_NSTRING_TEXT_CONSTRUCTOR)
    // Initialize the stack-local engine string using the selected constructor ABI.
    LocalNString_u80248934(const char* text) {
        MWString_ConstructCString(this, text);
    }
#endif
#if defined(GAME_NSTRING_RANGE_CONSTRUCTOR)
    // Initialize the stack-local engine string using the selected constructor ABI.
    LocalNString_u80248934(const char* text) {
        LocalNString_u80248934* object = this;
        unsigned int size;
        const char* source = text;
        object->flags = 0;
        object->length = 0;
        object->data = 0;
        size = nstring_detail::strlen(source);
        nstring_detail::String_ReserveCapacity(object, size);
        int allocator;
        char copy = *(char*)&allocator;
        nstring_detail::String_ReplaceRange(object, 0, 0, source, source + size, &copy);
    }
#endif
    // Free characters only when the string owns heap storage.
    ~LocalNString_u80248934() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
    // Return the character count from inline or heap string storage.
    unsigned int size() const {
        return (flags >> 31) == 0 ? (*(const unsigned char*)this & 0x7f) : length;
    }
    // Borrow the inline or heap NUL-terminated character buffer.
    const char* c_str() const {
        return (flags >> 31) == 0 ? (const char*)this + 1 : data;
    }
};

#include "game/singletons.h"
#include "classes/CSingleMainRev.h"
#include "classes/CTrainingMain.h"
#include "classes/CTrainingTask.h"
#include "classes/CSingleTaskRev.h"
#include "classes/CCharacterPreviewRecord.h"
#include "classes/CTask.h"
#include "classes/CCourseSelect.h"
#include "classes/CBpCtrl.h"

extern "C" {

extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern CSharedDoc* g_sharedDoc;
extern char s_bpCtrlDestructorRegistration4745C[];
void CBpCtrl_dtor(void*);
void __register_global_object(void*, void*, void*);
void* IdMap_Lookup(void*, void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void MPBase1_DrawIfInitialized(void*);
void* __ct__16CComSelectStrokeFv(void*);
void* __ct__15CComSelectMatchFv(void*);
void* __nw__FUl(unsigned int);
unsigned int strcpy(char*, const char*);

extern char s_trainingMainTypeInfo;
extern char s_trainingTaskRttiDescriptor;
extern char s_singleMainRevTypeDescriptor;
extern char s_singleTaskRevTypeDescriptor;
extern void* g_soundManager;
extern float s_trainingMainTwoPi;  // s_trainingMainTwoPi.
extern float s_trainingTaskTwoPi;  // s_trainingTaskTwoPi.
extern float s_singleMainRevTwoPi; // s_singleMainRevTwoPi.
extern float s_singleTaskRevTwoPi; // s_singleTaskRevTwoPi.
extern float kFloat_6_2831855;
extern char __vt__14CSingleMainRev[0x4C];
extern char singleMainTransitionStrings[0x90];
extern char __vt__13CTrainingTask[0x2C];
extern char __vt__14CSingleTaskRev[0x2C];
extern int g_defaultActorSkipMask;
extern int g_actorSkipMask;

void* __dt__5CTaskFv(void*, int);
void __dt__18NativeBase80201D18Fv(void*, int);
void appFreeUni__FPv(void*);

void CharacterPreviewTable_dtor(void*, int);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, const char*, const char*, char*);
unsigned int strlen(const char*);
void* TaskActorTable_CreateNamedActor(void*, const char*, const char*, int, void*);
void __dl__FPv(void*);
extern char trainingTaskEmptyActorArguments[]; // trainingTaskEmptyActorArguments.
extern char singleTaskEmptyActorArguments[];   // singleTaskEmptyActorArguments.
extern char trainingMainActorClassName[];      // trainingMainActorClassName.
extern char trainingMainActorInstanceName[];   // trainingMainActorInstanceName.
extern char singleMainRevActorClassName[];     // singleMainRevActorClassName.
extern char singleMainRevActorInstanceName[];  // singleMainRevActorInstanceName.

// Construct the training task and clear actor skip masks for the new scene.
CTrainingTask::CTrainingTask() {
    ((CTask*)this)->Init();
    *(void**)this = __vt__13CTrainingTask;
    secondaryVtable = __vt__13CTrainingTask + 0x10;
    tertiaryVtable = __vt__13CTrainingTask + 0x1C;
    g_defaultActorSkipMask = 0;
    g_actorSkipMask = 0;
}

// Destroy the task base and optionally free the training-task allocation.
CTrainingTask::~CTrainingTask() {
    __dt__5CTaskFv(this, 0);
}

// Destroy the task base and optionally free the single-player-task allocation.
CSingleTaskRev::~CSingleTaskRev() {
    __dt__5CTaskFv(this, 0);
}

// Construct the single-player task and clear actor skip masks for the new scene.
CSingleTaskRev::CSingleTaskRev() {
    ((CTask*)this)->Init();
    *(void**)this = __vt__14CSingleTaskRev;
    secondaryVtable = __vt__14CSingleTaskRev + 0x10;
    tertiaryVtable = __vt__14CSingleTaskRev + 0x1C;
    g_defaultActorSkipMask = 0;
    g_actorSkipMask = 0;
}

// Return the static training-main type descriptor.
void* CTrainingMain::GetRTTI() {
    return &s_trainingMainTypeInfo;
}

void __ct__18NativeBase80201D18Fv(void*);
// Construct the single-player menu actor with no active selection menu.
CSingleMainRev::CSingleMainRev() {
    void* primary;
    void* secondary;
    __ct__18NativeBase80201D18Fv(this);
    primary = __vt__14CSingleMainRev;
    secondary = (char*)primary + kSingleMainSecondaryVtableOffset;
    *(void**)this = primary;
    secondaryVtable = secondary;
    (*(void**)&state) = 0;
    childObject = 0;
}

void* SoundManager_RequestBgmChange(void*, void*, int, int);
// Request selection BGM (playOnce=0, allowFade=1); g_soundManager must be live.
// Actual callee in sound_manager.cpp returns VOID. The pointer-shaped declaration/return
// below preserves this unit's ABI spelling, not a handle, status or nullable data.
// Actor loading invokes a void loadResources hook (task_actor_loading.cpp); callers must ignore
// the return register. Disabled sound makes the callee do nothing; loading or
// playback failure is not reported here. The pooled name is borrowed for this
// call; manager-owned stream records copy paths and own subsequent file/stream
// lifetimes. This actor neither owns a music handle nor waits for readiness.
void* CSingleMainRev::OnLoad() {
    return SoundManager_RequestBgmChange(g_soundManager, &singleMainTransitionStrings, 0, 1);
}

// Return the static single-player-task type descriptor.
void* CSingleTaskRev::GetRTTI() {
    return &s_singleTaskRevTypeDescriptor;
}

// Initialize the training-main module's mutable two-pi constant at startup.
void TrainingMain_InitModuleConstants() {
    s_trainingMainTwoPi = kFloat_6_2831855;
}

void* __dt__13CTrainingMainFv(void*, int);
// Adjust the task-main interface pointer back to the training-main owner and
// forward destruction. The destructor is called by its mangled name because a
// destructor cannot take a delete flag through C++ syntax.
void* CTrainingMain::DestroyThunkMinus24(int deleting) {
    return __dt__13CTrainingMainFv((char*)this - 0x18, deleting);
}

// Return the static training-task type descriptor.
void* CTrainingTask::GetRTTI() {
    return &s_trainingTaskRttiDescriptor;
}

// Initialize the training-task module's mutable two-pi constant at startup.
void TrainingTask_InitModuleConstants() {
    s_trainingTaskTwoPi = kFloat_6_2831855;
}

void* __dt__13CTrainingTaskFv(void*, int);
// Adjust the task subobject pointer by -8 and forward destruction.
void* CTrainingTask::DestroyThunkMinus8(int deleting) {
    return ((void* (*)(void*, int))__dt__13CTrainingTaskFv)((char*)this - 0x8, deleting);
}

// Return the static single-player-main type descriptor.
void* CSingleMainRev::GetRTTI() {
    return &s_singleMainRevTypeDescriptor;
}

void* SoundManager_RequestBgmStop(void*, int);
// Request BGM stop with allowFade=1; ignore the pointer-shaped return, since
// SoundManager_RequestBgmStop (sound_manager.cpp) actually returns void, as above.
// Positive manager transitionDuration with a non-loading selected stream sets
// playState=3 and starts a seconds-based fade; SoundManager_UpdateStreamChannels
// later calls AdvanceFadingStream to release its resources. Otherwise the stop
// path releases the selected stream/file immediately. Return does not establish
// silence or fade completion. Manager/stream state must outlive deferred updates;
// the menu actor may be destroyed because the fade retains no pointer to it.
void* CSingleMainRev::OnDestroy() {
    return SoundManager_RequestBgmStop(g_soundManager, 1);
}

// Initialize the single-player-main module's mutable two-pi constant at startup.
void SingleMainRev_InitModuleConstants() {
    s_singleMainRevTwoPi = kFloat_6_2831855;
}

void* __dt__14CSingleMainRevFv(void*, int);
// Destroy the native actor base and optionally free the single-player-main allocation.
CSingleMainRev::~CSingleMainRev() {
    if (this)
        __dt__18NativeBase80201D18Fv(this, 0);
}
// Adjust the task-main interface pointer back to the single-player-main owner
// and forward destruction. The destructor is called by its mangled name because
// a destructor cannot take a delete flag through C++ syntax.
void* CSingleMainRev::DestroyThunkMinus24(int deleting) {
    return ((void* (*)(void*, int))__dt__14CSingleMainRevFv)((char*)this - 0x18, deleting);
}

void MPBase1_DrawIfInitialized(void*);
// Invoke the initialized selection menu's display hook through MPBase1.
void CSingleMainRev::OnDisplay() {
    void* p = childObject;
    if (p)
        MPBase1_DrawIfInitialized(p);
}

// Initialize the single-player-task module's mutable two-pi constant at startup.
void SingleTaskRev_InitModuleConstants() {
    s_singleTaskRevTwoPi = kFloat_6_2831855;
}

#pragma opt_common_subs off
// Release a selection-result vector while preserving its explicit volatile size reset.
void* dtor_80248D58(MSLPointerVector* self, int deleting) {
    return ::std::__vector_deleting_dtor(self, deleting);
}
#pragma opt_common_subs reset

// The repeated null tests preserve the original inlined vector-base destructor
// branches. Keep them inside these helpers; each allocation is freed only once.
#pragma push
#pragma always_inline on
// Free the courseIds allocation after resetting its volatile element count.
static inline void ReleaseSelectionCourseIds(void* self) {
    if (&((CourseSelectionStorage*)self)->courseIds) {
        if (&((CourseSelectionStorage*)self)->courseIds) {
            if (&((CourseSelectionStorage*)self)->courseIds) {
                if (&((CourseSelectionStorage*)self)->courseIds) {
                    if (&((CourseSelectionStorage*)self)->courseIds) {
                        void* p = ((CourseSelectionStorage*)self)->courseIds.data;
                        if (p) {
                            int count = ((CourseSelectionStorage*)self)->courseIds.size;
                            ((CourseSelectionStorage*)self)->courseIds.size = count - count;
                            appFreeUni__FPv(p);
                        }
                    }
                }
            }
        }
    }
}
// Free the holeNumbers allocation after resetting its volatile element count.
static inline void ReleaseSelectionHoleNumbers(void* self) {
    if (&((CourseSelectionStorage*)self)->holeNumbers) {
        if (&((CourseSelectionStorage*)self)->holeNumbers) {
            if (&((CourseSelectionStorage*)self)->holeNumbers) {
                if (&((CourseSelectionStorage*)self)->holeNumbers) {
                    if (&((CourseSelectionStorage*)self)->holeNumbers) {
                        void* p = ((CourseSelectionStorage*)self)->holeNumbers.data;
                        if (p) {
                            int count = ((CourseSelectionStorage*)self)->holeNumbers.size;
                            ((CourseSelectionStorage*)self)->holeNumbers.size = count - count;
                            appFreeUni__FPv(p);
                        }
                    }
                }
            }
        }
    }
}
// Preserve the render-result destructor's alias-sensitive count-reset expression.
static inline void ReleaseSelectionCourseIdsAliased(void* self) {
    if (&((CourseSelectionStorage*)self)->courseIds) {
        if (&((CourseSelectionStorage*)self)->courseIds) {
            if (&((CourseSelectionStorage*)self)->courseIds) {
                if (&((CourseSelectionStorage*)self)->courseIds) {
                    if (&((CourseSelectionStorage*)self)->courseIds) {
                        void* p = ((CourseSelectionStorage*)self)->courseIds.data;
                        if (p) {
                            int count = ((CourseSelectionStorage*)self)->courseIds.size;
                            ((CourseSelectionStorage*)self)->courseIds.size = count - *(int*)&count;
                            appFreeUni__FPv(p);
                        }
                    }
                }
            }
        }
    }
}
// Preserve the render-result destructor's alias-sensitive count-reset expression.
static inline void ReleaseSelectionHoleNumbersAliased(void* self) {
    if (&((CourseSelectionStorage*)self)->holeNumbers) {
        if (&((CourseSelectionStorage*)self)->holeNumbers) {
            if (&((CourseSelectionStorage*)self)->holeNumbers) {
                if (&((CourseSelectionStorage*)self)->holeNumbers) {
                    if (&((CourseSelectionStorage*)self)->holeNumbers) {
                        void* p = ((CourseSelectionStorage*)self)->holeNumbers.data;
                        if (p) {
                            int count = ((CourseSelectionStorage*)self)->holeNumbers.size;
                            ((CourseSelectionStorage*)self)->holeNumbers.size = count - *(int*)&count;
                            appFreeUni__FPv(p);
                        }
                    }
                }
            }
        }
    }
}
#pragma pop

// Release the two selection-result lists and optionally free their owner.
// Guard ladders below reproduce the compiler's inlined vector-base destructor chain.
// Each vector frees its owned allocation once; the self-subtraction resets logical size.
// Keep those expressions: flattening the guard chain changes the original branches.
void* CourseSelection_ReleaseAndFree(void* self, int deleting) {
    if (self) {
        ReleaseSelectionCourseIds(self);
        ReleaseSelectionHoleNumbers(self);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
// Release the two single-player selection-result lists and optionally free their owner.
void* dtor_802497B8(void* self, int deleting) {
    if (self) {
        if (self) {
            ReleaseSelectionCourseIds(self);
            ReleaseSelectionHoleNumbers(self);
            if (deleting > 0)
                __dl__FPv(self);
        }
    }
    return self;
}

// Create the named training-main actor with the fixed pooled argument string and install it as task main.
// Creation/ownership ledger shared with CSingleTaskRev::CreateMainActor:
// - local owns its string storage; destructor frees it on return or unwind.
// - TaskActorTable_CreateNamedActor (task_actor_loading.cpp) factory-creates the actor, assigns
//   task owner/context, allocates a holder, and publishes holder/key in the task's
//   hash and active ring before dispatching optional startup text synchronously.
//   The pooled instance-name key remains borrowed for the registry lifetime;
//   the stack startup message/text must not be retained by the actor.
// - Factory null returns null; SetMainActor then stores null, with no retry/status.
//   On success SetMainActor is just a borrowed main-actor pointer assignment
//   (tail_effect_spin_and_bubble.cpp), not a second ownership transfer. __dt__5CTaskFv releases the registry
//   actor and holder, then node blocks/hash storage. Never delete result here.
// - The creation routine guards locks but has no actor/holder rollback guard:
//   a throw before registration can orphan allocations; after registration the
//   task retains the entry even if startup dispatch throws before SetMainActor.
//   Allocation nulls after the factory are unchecked. This wrapper offers no
//   partial-construction recovery; the local string is the only cleanup it owns.
void CTrainingTask::CreateActors() {
    char ch;
    int pad;
    LocalNString_u80248934 local;
    char* base = (char*)(unsigned int)trainingTaskEmptyActorArguments;
    unsigned int len = strlen(base);
    String_ReserveCapacity(&local, len);
    ch = *(char*)&pad;
    String_ReplaceRange(&local, 0, 0, base, base + len, &ch);
    void* result =
        TaskActorTable_CreateNamedActor(this, trainingMainActorClassName, trainingMainActorInstanceName, 0, &local);
    ((CTask*)this)->SetMainActor(result);
}

#pragma opt_common_subs off
// Destroy render records, release both selection lists, and optionally free the result object.
void* CourseSelectionStorage_dtor(void* self, int deleting) {
    if (self) {
        CharacterPreviewTable_dtor(((CourseSelectionStorage*)self)->renderRecords, -1);
        if (self) {
            ReleaseSelectionCourseIdsAliased(self);
            ReleaseSelectionHoleNumbersAliased(self);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma opt_common_subs reset

// Create the named single-player-main actor with the fixed pooled argument string and install it as task main.
// Same registration/failure ledger as CTrainingTask::CreateActors above; only the
// pooled class/key/initial text differ. self must be a constructed, live task.
void CSingleTaskRev::CreateMainActor() {
    char ch;
    int pad;
    LocalNString_u80248934 local;
    char* base = (char*)(unsigned int)singleTaskEmptyActorArguments;
    unsigned int len = strlen(base);
    String_ReserveCapacity(&local, len);
    ch = *(char*)&pad;
    String_ReplaceRange(&local, 0, 0, base, base + len, &ch);
    void* result =
        TaskActorTable_CreateNamedActor(this, singleMainRevActorClassName, singleMainRevActorInstanceName, 0, &local);
    ((CTask*)this)->SetMainActor(result);
}

} // pause C linkage for class definitions

// Stroke-selection menu storage; retain the constructor symbol used by exception tables.
struct Obj_B8 {
    char storage[0xB8];
    // Construct stroke-selection menu state in its fixed-size allocation.
    Obj_B8() {
        __ct__16CComSelectStrokeFv(this);
    }
};

// Match-selection menu storage; retain the constructor symbol used by exception tables.
struct Obj_8C {
    char storage[0x8C];
    // Construct match-selection menu state in its fixed-size allocation.
    Obj_8C() {
        __ct__15CComSelectMatchFv(this);
    }
};

struct ObjBase {
    virtual ~ObjBase();
};

extern "C" {

// Replace the active menu with stroke or match selection and update shared rule settings.
// event is borrowed {sender,type}; type 0 selects stroke, 1 selects match. The
// previous child is deleted before constructing the replacement. Other types do
// nothing. Rule type and overall game mode are separate protocols even when 0.
// Required nonnull initialized self and event with at least two readable words;
// sender and trailing payload/time words are unused. IActor_DispatchMessage
// (task_registry_actor_and_debug_text.cpp) copies these words on its stack, delivers synchronously and
// transfers no ownership. This handler does not check a due timestamp.
// Admission: state Choose normally receives 0/1; the implementation also replaces
// a live selector in Stroke/Match, and would reopen from StartingRound. Producers
// must stop sending selection events once a golf transition has been requested.
// Map: 0 -> Stroke/new CComSelectStroke, rule 0, gameMode 0; 1 -> Match/new
// CComSelectMatch, rule 1, gameMode 0; every other signed int -> no change.
// State is published BEFORE deleting/constructing child; an allocation/ctor
// failure is not transactional. OnProcess's null child does no work; on a throw
// earlier mutations remain. On successful allocation child ownership lasts to
// result acceptance/cancellation, replacement, or actor destruction.
void CSingleMainRev::HandleMsg(const void* event) {
    int type = ((const MenuEvent*)event)->type;
    switch (type) {
    case kStrokeRule: {
        state = kSingleMenuStroke;
        ObjBase* old = (*(ObjBase**)&childObject);
        if (old) {
            delete old;
            childObject = 0;
        }
        Obj_B8* obj = new Obj_B8();
        childObject = obj;
        int zero = 0;
        AsCSharedDoc(g_sharedDoc)->golfRuleType = zero; // original +0x8C0
        strcpy(((SharedRuleName*)g_sharedDoc)->name, ((char*)AsCSharedDoc(g_sharedDoc)->strokePlayName));
        AsCSharedDoc(g_sharedDoc)->gameMode = zero; // original +0x808
        break;
    }
    case kMatchRule: {
        state = kSingleMenuMatch;
        ObjBase* old = (*(ObjBase**)&childObject);
        if (old) {
            delete old;
            childObject = 0;
        }
        Obj_8C* obj = new Obj_8C();
        childObject = obj;
        AsCSharedDoc(g_sharedDoc)->golfRuleType = kMatchRule; // original +0x8C0
        strcpy(((SharedRuleName*)g_sharedDoc)->name, ((char*)AsCSharedDoc(g_sharedDoc)->matchPlayName));
        AsCSharedDoc(g_sharedDoc)->gameMode = kSinglePlayerGameMode; // original +0x808
        break;
    }
    }
}

// Compose Free Training background, course selector and caption in that order.
// self/courseSelect and both registered BP entries must exist. An uninitialized
// selector suppresses the entire display. Registry lookups are borrowed; callbacks
// run immediately before each BP draw and may update scripts. This consumer
// supplies no pixel dimensions/aspect ratio; preserve layering and callback order.
void CTrainingMain::OnDisplay() {
    if (!AsMPBase1(courseSelect)->initialized)
        return;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDestructorRegistration4745C);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup(Sing_805DC5F0(), backgroundEntryId), 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDestructorRegistration4745C);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    BpObject_Draw(IdMap_Lookup(Sing_805DC5F0(), backgroundEntryId));
    MPBase1_DrawIfInitialized(courseSelect);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDestructorRegistration4745C);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup(Sing_805DC5F0(), captionEntryId), 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDestructorRegistration4745C);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    BpObject_Draw(IdMap_Lookup(Sing_805DC5F0(), captionEntryId));
}

extern void* g_mouseCursor;
extern void* g_taskManager;
extern signed char s_saveDataInitialized;
extern char s_saveData[];
extern char trainingMainStrings[];
extern int trainingMainHiddenGroupNames[]; // trainingMainHiddenGroupNames.
extern void SaveData_Ctor(void*);
extern void* SaveData_GetCurrentRecord(void*);
extern void BpObject_StartScriptByName(void*, const char*, int);
extern void* BpObject_GetOrCreateNamedGroup(void*, void*);
extern void UIGroup_SetEnabled(void*, int);
extern int MPBase1_Process(void*, float);
extern void __construct_array(void*, void*, void*, int, int);
extern void GolfPlayerList_BuildFromRenderTable(void*);
extern void CourseSelection_ApplyRoundSettings(void*);
extern void TaskCommandQueue_AppendNamedRequest(void*, int, const char*, int, int, int, int, int);
extern void PartyItemRecord_InitDefaults(void*);
extern void PlayerRenderRecord_dtor(void*);

int MPBase1_Process(void*, float);
CSharedDoc* g_sharedDoc;
extern signed char s_saveDataInitialized;
extern void* g_taskManager;
extern char s_saveData[];
extern char singleMainTransitionStrings[];
extern void __construct_array(void*, void*, void*, int, int);
extern void* SaveData_GetCurrentRecord(void*);
extern void GolfPlayerList_BuildFromRenderTable(void*);
extern void CourseSelection_ApplyRoundSettings(void*);
extern void TaskCommandQueue_AppendNamedRequest(void*, int, const char*, int, int, int, int, int);
extern void appFreeUni__FPv(void*);
extern void CharacterPreviewTable_dtor(void*, int);
// Forward actor time unchanged to the selector's MPBase1 update; return completion.
// dt is the upstream animation/update time step in seconds; no frame-rate scale
// is applied locally. Required selector remains borrowed for this call. The
// incoming time stays in f1 through the original selector call.
static inline int ProcessSelectorWithTime(void* selector, float dt) {
    return MPBase1_Process(selector, dt);
}

#pragma opt_propagation off
#pragma opt_common_subs off
// Process Stroke/Match selection; publish an accepted round or request title on cancel.
// self is required, childObject may be null (no work); dt seconds is forwarded
// once. MPBase1 completion does not imply acceptance: virtual +28 queries the
// result into stack-owned vectors. Accept publishes shared settings/players
// before requesting a transition, clears the selector and enters StartingRound.
// Cancel also deletes the selector. Preserve borrowed save-record lifetimes,
// vector cleanup on both outcomes/unwind, and the incoming float operation order.
void CSingleMainRev::OnProcess(float dt) {
    // Phase 1: poll the owned selector; no transition occurs until it completes.
    void* currentSaveRecord;
    void* selectionResult;
    void* renderTable;
    void* saveDataForCharacter;
    struct VObj {
        // Keep the local dispatch view's empty virtual base destructor.
        virtual ~VObj() {}
        virtual void _v0() = 0;
        virtual void _v1() = 0;
        virtual void _v2() = 0;
        virtual void _v3() = 0;
        virtual void _v4() = 0;
        virtual void _v5() = 0;
        virtual void _v6() = 0;
        virtual int Call(void*) = 0;
    };
    // Result ABI common prefix: +0 course ID, +1 first-hole/order byte,
    // +2 round option, +3 hole count; +4/+10 byte-vector {data,count,capacity}.
    // Stroke producer CComSelectStroke_BuildRoundSettings (stroke_match_selection_text.cpp) and Match producer
    // CComSelectMatch_BuildRoundSettings (com_select_match_screen.cpp) both write +1 as ZERO-BASED
    // firstHole and +2=0, and append paired one-based holes / course IDs. The
    // generic ApplyRoundSettings consumer interprets +1 as order 0/front,
    // 1/back,3/shuffle ONLY WHEN count != holeCount. These producers supply
    // matching counts, so their explicit lists are preserved. Do not regenerate
    // these lists using that byte as an order enum; the overload is intentional.
    // Require 1..18 paired entries, course IDs resolving in loaded inventory,
    // and 1<=firstHole+1<=firstHole+holeCount<=18. No local validation/clamp.
    // Stroke +2C and Match +2B0 are selected table ordinals, not pointers or
    // course IDs. Stroke publishes ordinal unchanged; Match publishes ordinal
    // +0x20 into CSharedDoc::courseRecordIndex. Save snapshot/read/write in
    // in_game_dialog_and_snapshot_copy.cpp indexes signed-byte courseScores[0x44] with this value. Thus
    // require final index 0..0x43 (stroke ordinal 0..0x43, match 0..0x23), AND
    // actual selected-table bounds; conventional match table is 8 pages x4.
    struct SelectionLists {
        int holeData, holeCount, holeCapacity, courseData, courseCount, courseCapacity;
    };
    struct StrokeSelectionResult {
        char courseIndex, firstHole, roundOption, holeCount;
        SelectionLists lists;
        short selectionCode;
        char selectionBytes[12];
        int courseRecordIndex;
    };
    struct MatchRenderTable {
        char records[4 * 0xA4];
        volatile int playerCount;
    };
    struct MatchSelectionResult {
        char courseIndex, firstHole, roundOption, holeCount;
        SelectionLists lists;
        MatchRenderTable renderTable;
        volatile int courseRecordValue;
    };
    struct SelectionScratch {
        StrokeSelectionResult strokeResult;
        MatchSelectionResult matchResult;
        char strokeRenderTable[0x294];
    };
    SelectionScratch scratch;
    int result = 0;
    void* selector = childObject;
    if (selector)
        result = ProcessSelectorWithTime(selector, dt);
    if (result) {
        switch (state) {
        case kSingleMenuStroke: {
            // Phase 2a: acquire a stroke result with two independently owned vectors.
            scratch.strokeResult.lists.holeData = 0;
            scratch.strokeResult.lists.holeCount = 0;
            scratch.strokeResult.lists.holeCapacity = 0;
            scratch.strokeResult.lists.courseData = 0;
            scratch.strokeResult.lists.courseCount = 0;
            scratch.strokeResult.lists.courseCapacity = 0;
            scratch.strokeResult.courseIndex = 0;
            scratch.strokeResult.firstHole = 0;
            scratch.strokeResult.roundOption = 0;
            scratch.strokeResult.holeCount = kDefaultHoleCount;
            EH5Cleanup eh5Cleanup42(&scratch.strokeResult.lists.holeData, (void (*)(void*, int))dtor_80046918);
            EH5Cleanup eh5Cleanup43(&scratch.strokeResult.lists.courseData, (void (*)(void*, int))dtor_80046918);
            void* objNow = childObject;
            if (((VObj*)objNow)->Call(&scratch.strokeResult)) {
                // Accepted: publish round settings, build the local player's render
                // table from save data, then request the golf task transition.
                CourseSelection_ApplyRoundSettings(&scratch.strokeResult);
                SharedSingleSelection* globals = (SharedSingleSelection*)g_sharedDoc;
                globals->code = scratch.strokeResult.selectionCode;
                globals->bytes[0] = scratch.strokeResult.selectionBytes[0];
                globals->bytes[1] = scratch.strokeResult.selectionBytes[1];
                globals->bytes[2] = scratch.strokeResult.selectionBytes[2];
                globals->bytes[3] = scratch.strokeResult.selectionBytes[3];
                globals->bytes[4] = scratch.strokeResult.selectionBytes[4];
                globals->bytes[5] = scratch.strokeResult.selectionBytes[5];
                globals->bytes[6] = scratch.strokeResult.selectionBytes[6];
                globals->bytes[7] = scratch.strokeResult.selectionBytes[7];
                globals->bytes[8] = scratch.strokeResult.selectionBytes[8];
                globals->bytes[9] = scratch.strokeResult.selectionBytes[9];
                globals->bytes[10] = scratch.strokeResult.selectionBytes[10];
                globals->bytes[11] = scratch.strokeResult.selectionBytes[11];
                __construct_array(scratch.strokeRenderTable, (void*)PartyItemRecord_InitDefaults,
                                  (void*)PlayerRenderRecord_dtor, 0xA4, 4);
                *(int*)(scratch.strokeRenderTable + 0x290) = 1;
                EH5Cleanup eh5Cleanup44(scratch.strokeRenderTable, (void (*)(void*, int))CharacterPreviewTable_dtor);
                void* saveData = GetSaveDataSlot();
                if (!saveData) {
                    if (!s_saveDataInitialized) {
                        SaveData_Ctor(s_saveData);
                        s_saveDataInitialized = 1;
                    }
                    GetSaveDataSlot() = s_saveData;
                }
                saveDataForCharacter = GetSaveDataSlot();
                if (!saveDataForCharacter) {
                    if (!s_saveDataInitialized) {
                        SaveData_Ctor(s_saveData);
                        s_saveDataInitialized = 1;
                    }
                    GetSaveDataSlot() = s_saveData;
                }
                currentSaveRecord = SaveData_GetCurrentRecord(GetSaveDataSlot());
                void* characterSaveRecord = SaveData_GetCurrentRecord(saveDataForCharacter);
                unsigned char index = (unsigned char)AsSavePlayerRecord(characterSaveRecord)->characterIndex;
                ((CCharacterPreviewRecord*)scratch.strokeRenderTable)
                    ->FillFromSaveData((signed char)index, currentSaveRecord);
                GolfPlayerList_BuildFromRenderTable(scratch.strokeRenderTable);
                *(void**)&AsCSharedDoc(g_sharedDoc)->courseRecordIndex =
                    *(void**)&scratch.strokeResult.courseRecordIndex; // original +0xB80
                ((CTask*)g_taskManager)
                    ->RequestTransitionWithFade(
                        singleMainTransitionStrings + IFF_OFFSET(SingleTransitionStringPool, golfTask),
                        singleMainTransitionStrings + IFF_OFFSET(SingleTransitionStringPool, golfDocument), 0);
                AsCSharedDoc(g_sharedDoc)->pendingCourseClearNotification = 0; // original +0xB8C
                state = kSingleMenuStartingRound;
                void* obj = childObject;
                if (obj) {
                    delete (ObjBase*)obj;
                    childObject = 0;
                }
                {
                    eh5Cleanup44.release();
                    CharacterPreviewTable_dtor(scratch.strokeRenderTable, -1);
                }
            } else {
                // Cancelled: remove the selector and restore title selection 2.
                void* obj = childObject;
                if (obj) {
                    delete (ObjBase*)obj;
                    childObject = 0;
                }
                ((CTask*)g_taskManager)
                    ->RequestTransitionWithFade(
                        singleMainTransitionStrings + IFF_OFFSET(SingleTransitionStringPool, titleTask),
                        singleMainTransitionStrings + IFF_OFFSET(SingleTransitionStringPool, emptyTitleContext), 0);
                TaskCommandQueue_AppendNamedRequest(
                    g_taskManager, 0, singleMainTransitionStrings + IFF_OFFSET(SingleTransitionStringPool, titleMain),
                    kSingleTitleMenuIndex, 0, 0, 0, 0);
            }
            // Result vectors remain local on both paths; release each exactly once.
            if (scratch.strokeResult.lists.courseData) {
                int t = scratch.strokeResult.lists.courseCount;
                scratch.strokeResult.lists.courseCount -= t;
                {
                    eh5Cleanup43.release();
                    appFreeUni__FPv((void*)scratch.strokeResult.lists.courseData);
                }
            }
            if (scratch.strokeResult.lists.holeData) {
                int t = scratch.strokeResult.lists.holeCount;
                scratch.strokeResult.lists.holeCount -= t;
                {
                    eh5Cleanup42.release();
                    appFreeUni__FPv((void*)scratch.strokeResult.lists.holeData);
                }
            }
            break;
        }
        case kSingleMenuMatch: {
            // Phase 2b: match selection fills its result and four-record render table.
            scratch.matchResult.lists.holeData = 0;
            scratch.matchResult.lists.holeCount = 0;
            scratch.matchResult.lists.holeCapacity = 0;
            scratch.matchResult.lists.courseData = 0;
            scratch.matchResult.lists.courseCount = 0;
            scratch.matchResult.lists.courseCapacity = 0;
            scratch.matchResult.courseIndex = 0;
            scratch.matchResult.firstHole = 0;
            scratch.matchResult.roundOption = 0;
            scratch.matchResult.holeCount = kDefaultHoleCount;
            EH5Cleanup eh5Cleanup45(&scratch.matchResult.lists.holeData, (void (*)(void*, int))dtor_80046918);
            EH5Cleanup eh5Cleanup46(&scratch.matchResult.lists.courseData, (void (*)(void*, int))dtor_80046918);
            renderTable = &scratch.matchResult.renderTable;
            selectionResult = &scratch.matchResult;
            __construct_array(renderTable, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4,
                              4);
            scratch.matchResult.renderTable.playerCount = 0;
            EH5Cleanup eh5Cleanup47(&scratch.matchResult.renderTable, (void (*)(void*, int))CharacterPreviewTable_dtor);
            void* objNow = childObject;
            if (((VObj*)objNow)->Call(selectionResult)) {
                CourseSelection_ApplyRoundSettings(selectionResult);
                GolfPlayerList_BuildFromRenderTable(renderTable);
                *(void**)&AsCSharedDoc(g_sharedDoc)->courseRecordIndex =
                    (void*)(scratch.matchResult.courseRecordValue + 0x20); // original +0xB80
                ((CTask*)g_taskManager)
                    ->RequestTransitionWithFade(
                        singleMainTransitionStrings + IFF_OFFSET(SingleTransitionStringPool, golfTask),
                        singleMainTransitionStrings + IFF_OFFSET(SingleTransitionStringPool, golfDocument), 0);
                AsCSharedDoc(g_sharedDoc)->pendingCourseClearNotification = 0; // original +0xB8C
                state = kSingleMenuStartingRound;
                void* obj = childObject;
                if (obj) {
                    delete (ObjBase*)obj;
                    childObject = 0;
                }
            } else {
                void* obj = childObject;
                if (obj) {
                    delete (ObjBase*)obj;
                    childObject = 0;
                }
                ((CTask*)g_taskManager)
                    ->RequestTransitionWithFade(
                        singleMainTransitionStrings + IFF_OFFSET(SingleTransitionStringPool, titleTask),
                        singleMainTransitionStrings + IFF_OFFSET(SingleTransitionStringPool, emptyTitleContext), 0);
                TaskCommandQueue_AppendNamedRequest(
                    g_taskManager, 0, singleMainTransitionStrings + IFF_OFFSET(SingleTransitionStringPool, titleMain),
                    2, 0, 0, 0, 0);
            }
            {
                eh5Cleanup47.release();
                CharacterPreviewTable_dtor(&scratch.matchResult.renderTable, -1);
            }
            if (scratch.matchResult.lists.courseData) {
                int t = scratch.matchResult.lists.courseCount;
                scratch.matchResult.lists.courseCount -= t;
                {
                    eh5Cleanup46.release();
                    appFreeUni__FPv((void*)scratch.matchResult.lists.courseData);
                }
            }
            if (scratch.matchResult.lists.holeData) {
                int t = scratch.matchResult.lists.holeCount;
                scratch.matchResult.lists.holeCount -= t;
                {
                    eh5Cleanup45.release();
                    appFreeUni__FPv((void*)scratch.matchResult.lists.holeData);
                }
            }
            break;
        }
        case kSingleMenuStartingRound:
            break;
        default:
            break;
        }
    }
}
#pragma opt_propagation reset
#pragma opt_common_subs reset

#pragma opt_propagation off
#pragma opt_common_subs off
// Start loaded Free Training scripts, then publish course/players or return to title.
// self/courseSelect are required; dt seconds is forwarded unchanged to MPBase1.
// scriptsStarted latches only after all BP entries load; readiness polls have no
// timeout here. Selector completion still needs GetSelectionResult acceptance.
// Result vectors/render records are temporary owned storage, destroyed after
// transition and do not turn completion into an unconditional course launch.
void CTrainingMain::OnProcess(float dt) {
    // The original keeps the actor pointer and the borrowed save record in one
    // register; binding the record to this local reproduces that.
    void* actor = this;
    // Phase 1: start background/caption scripts once all controller entries load.
    void* saveDataForCharacter;
    struct Data9 {
        int v[9];
    };
    struct Params {
        // CCourseSelect::GetSelectionResult (course_select_menu.cpp) deep-copies:
        // bytes[0]=selectedCourseIndex, [1]=holeOrder (0/front,1/back,3/shuffle),
        // [2]=weatherMode -> round option at shared+9C6, [3]=holeCount (1..18).
        // x[0..2]={hole-number byte buffer,count,capacity}, x[3..5] likewise
        // course-ID bytes, paired by index. Hole numbers 1..18; course IDs must
        // resolve in loaded course inventory. Counts must agree; if holes.count
        // differs from bytes[3], ApplyRoundSettings clears/rebuilds BOTH vectors
        // using the order byte. Equal counts preserve explicit lists, including
        // mixed-course entries. Weather option is copied unchanged; this code
        // does not interpret its gameplay domain. Selection returns 0 untouched
        // before confirmation; allocation exceptions may leave partial copies.
        // These are native Wii 32-bit pointer/count/capacity words, not file
        // offsets. Keep their initialization and destructor order together.
        char bytes[4];
        int x[6];
        // Free the two locally owned selection-result vectors.
        ~Params() {
            if (x[3]) {
                int t = x[4];
                x[4] -= t;
                appFreeUni__FPv((void*)x[3]);
            }
            if (x[0]) {
                int t = x[1];
                x[1] -= t;
                appFreeUni__FPv((void*)x[0]);
            }
        }
    };
    int data[9];
    if (scriptsStarted == 0) {
        void* manager = Sing_805DC5F0();
        if (!manager) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDestructorRegistration4745C);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        if (((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded()) {
            manager = Sing_805DC5F0();
            if (!manager) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDestructorRegistration4745C);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), backgroundEntryId),
                                       trainingMainStrings + IFF_OFFSET(TrainingStringPool, backgroundGroup), 1);
            *(Data9*)data = *(Data9*)trainingMainHiddenGroupNames;
            manager = Sing_805DC5F0();
            if (!manager) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDestructorRegistration4745C);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            int* dp;
            void* item = IdMap_Lookup(Sing_805DC5F0(), captionEntryId);
            BpObject_StartScriptByName(item, trainingMainStrings + IFF_OFFSET(TrainingStringPool, titleGroup), 1);
            dp = data;
            for (unsigned int i = 0; i < kTrainingHiddenGroupCount; ++i) {
                UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(item, (void*)*dp), 0);
                ++dp;
            }
            scriptsStarted = 1;
        }
    }
    // Phase 2: poll course selection; a completed menu can still represent cancel.
    if (!MPBase1_Process(courseSelect, dt))
        return;
    struct Buf {
        char obj[0x290];
        int one;
        // Construct four render records; the caller sets the trailing active count.
        Buf() {
            __construct_array(obj, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor,
                              kPlayerRenderRecordBytes, kPlayerRenderRecordCount);
        }
        // Destroy all render records through their matching array-owner destructor.
        ~Buf() {
            CharacterPreviewTable_dtor(obj, -1);
        }
    };
    Params params;
    params.x[0] = 0;
    params.x[1] = 0;
    params.x[2] = 0;
    params.x[3] = 0;
    params.x[4] = 0;
    params.x[5] = 0;
    params.bytes[0] = 0;
    params.bytes[1] = 0;
    params.bytes[2] = 0;
    params.bytes[3] = kDefaultHoleCount;
    if (((CCourseSelect*)courseSelect)->GetSelectionResult((char*)&params)) {
        // Phase 3: accepted course -> local-player render data, training settings,
        Buf buf;
        buf.one = 1;
        void* saveData = GetSaveDataSlot();
        if (!saveData) {
            if (!s_saveDataInitialized) {
                SaveData_Ctor(s_saveData);
                s_saveDataInitialized = 1;
            }
            GetSaveDataSlot() = s_saveData;
        }
        saveDataForCharacter = GetSaveDataSlot();
        if (!saveDataForCharacter) {
            if (!s_saveDataInitialized) {
                SaveData_Ctor(s_saveData);
                s_saveDataInitialized = 1;
            }
            GetSaveDataSlot() = s_saveData;
        }
        // Actor access has ended. The register that held the actor pointer now
        // holds the borrowed save record.
        void*& currentSaveRecord = actor;
        currentSaveRecord = SaveData_GetCurrentRecord(GetSaveDataSlot());
        void* second = SaveData_GetCurrentRecord(saveDataForCharacter);
        unsigned char index = (unsigned char)AsSavePlayerRecord(second)->characterIndex;
        ((CCharacterPreviewRecord*)(buf.obj))->FillFromSaveData((signed char)index, currentSaveRecord);
        GolfPlayerList_BuildFromRenderTable(buf.obj);
        CourseSelection_ApplyRoundSettings(&params);
        AsCSharedDoc(g_sharedDoc)->gameMode = kTrainingGameMode; // original +0x808
        *(((char*)&AsCSharedDoc(g_sharedDoc)->golfRuleType)) = 0;
        strcpy(((SharedRuleName*)g_sharedDoc)->name, ((char*)AsCSharedDoc(g_sharedDoc)->strokePlayName));
        ((CMouseCursor*)g_mouseCursor)->hitTestMode = kGolfCursorHitTestMode;
        CMouseCursor::SetPointerInputEnabled(g_mouseCursor, 0);
        ((CTask*)g_taskManager)
            ->RequestTransitionWithFade(trainingMainStrings + IFF_OFFSET(TrainingStringPool, golfTask),
                                        trainingMainStrings + IFF_OFFSET(TrainingStringPool, golfDocument), 0);
    } else {
        ((CTask*)g_taskManager)
            ->RequestTransitionWithFade(trainingMainStrings + IFF_OFFSET(TrainingStringPool, titleTask),
                                        trainingMainStrings + IFF_OFFSET(TrainingStringPool, emptyTitleContext), 0);
        TaskCommandQueue_AppendNamedRequest(g_taskManager, 0,
                                            trainingMainStrings + IFF_OFFSET(TrainingStringPool, titleMain),
                                            kTrainingTitleMenuIndex, 0, 0, 0, 0);
    }
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804622C4", _ctors_u80248934_0,
                  TrainingMain_InitModuleConstants); // TrainingMain_InitModuleConstants.
STATIC_INIT_ENTRY(".CRT$XCU804622C4", _ctors_u80248934_1,
                  TrainingTask_InitModuleConstants); // TrainingTask_InitModuleConstants.
STATIC_INIT_ENTRY(".CRT$XCU804622C4", _ctors_u80248934_2,
                  SingleMainRev_InitModuleConstants); // SingleMainRev_InitModuleConstants.
STATIC_INIT_ENTRY(".CRT$XCU804622C4", _ctors_u80248934_3,
                  SingleTaskRev_InitModuleConstants); // SingleTaskRev_InitModuleConstants.
#pragma pop
