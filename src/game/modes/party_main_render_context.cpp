// Party-mode selection: the screens the player steps through before a round,
// the shared render context that holds the player records while they are set
// up, and the BP layers drawn behind the screens.
#include <decomp/static_init.h>
#include <decomp/ct_member.h>
#include "iff/IffStruct.h"
#include "classes/CEnemyRenderTable.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

#include "game/vector.h"

#include "game/singletons.h"
#include "classes/CPartyMain.h"
#include "classes/CVSMain.h"
#include "classes/CCharaNum.h"
#include "classes/CPartyItemSelect.h"
#include "classes/CCharaEntry.h"
#include "classes/CCharaSelect.h"
#include "classes/CCharacterPreviewTable.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CPopItemDialog.h"
#include "classes/CCourseSelect.h"
#include "classes/CTask.h"
#include "classes/CBpCtrl.h"
#include "classes/CBpCtrlHandle.h"

#define AsCPartyMain(p) ((CPartyMain*)(p))
extern "C" {

void RenderContext_ResetPlayerRecords(void*);
void* CharacterPreviewTable_dtor(void*, int);
void appFreeUni__FPv(void*);

struct RenderContextConstructionCleanup_u80284D14 {
    char* base;
    bool active;
    // Arm cleanup of embedded player/vector storage during singleton construction.
    explicit RenderContextConstructionCleanup_u80284D14(void* object) : base((char*)object), active(true) {}
    // Unwind constructed embedded storage if player-record reset throws.
    ~RenderContextConstructionCleanup_u80284D14() {
        if (active) {
            CharacterPreviewTable_dtor(base + IFF_OFFSET(RenderContext, players), -1);
            for (int index = 3; index >= 0; --index) {
                unsigned int* vector = (unsigned int*)(base + IFF_OFFSET(RenderContext, vectors) + index * 12);
                if (vector[0]) {
                    vector[1] = 0;
                    appFreeUni__FPv((void*)vector[0]);
                }
            }
        }
    }
};

// Reset a newly constructed render context and commit its embedded storage ownership.
static void InitializeNewRenderContext(void* base) {
    RenderContextConstructionCleanup_u80284D14 cleanup(base);
    RenderContext_ResetPlayerRecords(base);
    cleanup.active = false;
}

extern float kFloat_6_2831855;
extern float s_twoPi_PartyMain;
extern void* __vt__14CPopItemDialog[];
extern signed char s_bpCtrlInitialized;
extern signed char s_renderContextInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern unsigned char s_partyItemDetailDrawFirst;
extern char s_bpCtrl[];
extern char s_partyMainBpCtrlDestructorRecord[];
extern char s_bpCtrlDtorRegistration_PopItem[];
extern char s_partyBpHandleDestructorRegistration[];

void __dt__7CVSMainFv(void*, int);
void CBpCtrl_dtor();
void __register_global_object(void*, void*, void*);
void* IdMap_Lookup(void*, void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void MPBase1_DrawIfInitialized(void*);
void __dl__FPv(void*);
void CBpCtrlHandle_Dtor();
void* __nw__FUl(unsigned int);
void SaveData_InitPlayerRecord(void*);

void __construct_array(void*, void*, void*, int, int);
void PartyItemRecord_InitDefaults(void*);
void PlayerRenderRecord_dtor(void*);
void RenderContext_ResetPlayerRecords(void*);
void* RenderContext_GetOrCreate();
void __dt__13RenderContextFv(void*);
extern char s_renderContext[];
extern char s_renderContextDestructorRegistration_PartyMain[];
extern void* g_sharedDoc;
void BpObject_SetEnabled(void*, int);
void CharacterLoading_InvokeAnimationCallback(void*, float);
void* IdMap_Lookup(void*, void*);
void* BpObject_StartScriptByName(void*, const char*, int);
void* BpObject_GetOrCreateNamedGroup(void*, void*);
void UIGroup_SetEnabled(void*, int);
extern char titleTaskStrings[];
extern int partyTitleGroupNames[];
extern void* g_sharedDoc;
extern void* g_taskManager;
extern char s_renderContext[];
int MPBase1_Process(void*, float);
void CharaCount_InitInvalidBpHandle(void*);
void TaskCommandQueue_AppendNamedRequest(void*, int, const char*, int, int, int, int, int);
void CourseSelection_ApplyRoundSettings(void*);
void PartyRound_ChooseHoleVariants();
void GolfPlayerList_BuildFromRenderTable(void*);
void __ct__16CPartyItemSelectFv(void*, void*);
// Own fixed-size storage and invoke the CharacterSelectScreen constructor on allocation.
struct CharacterSelectScreen {
    char pad[0x60];
    // Initialize the owned selection-screen allocation.
    CharacterSelectScreen() {
        ((CCharaSelect*)this)->__ct();
    }
};
// Own fixed-size storage and invoke the CourseSelectScreen constructor on allocation.
struct CourseSelectScreen {
    char pad[0x80];
    // Initialize the owned selection-screen allocation.
    CourseSelectScreen() {
        ((CCourseSelect*)this)->__ct();
    }
};
static void ensure_805DC5F0();

enum PartySelectionState {
    kSelectPlayerCount = 1,
    kSelectCharacter = 2,
    kConfirmCharacters = 3,
    kSelectCourse = 4,
    kSelectPartyItems = 5,
    kSelectionIdle = 6
};
enum {
    kPartyGameMode = 7,
    kMatchPlayRule = 1,
    kPartyPlayerCount = 4,
    kPlayerSaveRecordSize = 0x828,
    kTitleGroupCount = 10,
    // Party-mode captions use a separate domain from selection-screen states.
    kPartyMode1 = 1,
    kPartyMode2 = 2,
    kPartyMode3 = 3
};
// titleTaskStrings is one packed pool; lengths include terminators and the empty task argument.
struct PartyTitleStrings {
    char backgroundFile[0x11], captionFile[0x15], selectSound[0xB];
    char backgroundScript[3], captionScript[6];
    char partyTitle1[0x11], partyTitle2[0x11], partyTitle3[0x11];
    char strokeTitle[0x11], matchTitle[0x11], titleTask[0xB], emptyArgument[1];
    char titleMain[0xB], golfTask[0xA], golfDoc[9];
};
typedef char PartyTitleStrings_background_offset[(IFF_OFFSET(PartyTitleStrings, backgroundScript) == 0x31) ? 1 : -1];
typedef char PartyTitleStrings_empty_offset[(IFF_OFFSET(PartyTitleStrings, emptyArgument) == 0x9A) ? 1 : -1];
typedef char PartyTitleStrings_golf_offset[(IFF_OFFSET(PartyTitleStrings, golfTask) == 0xA6) ? 1 : -1];

// Party selection owns exactly one active selectionScreen. Delete the previous
// screen before replacing it; read its completion result before deletion.
// State transitions on confirm: count -> character -> entry -> course -> [items].
// Cancel goes back one screen; count cancel and match-play character cancel return
// to the title task. Course/item confirmation applies settings before entering golf.
// +24/+28 (first/second) alias CPartyMain's BP entry IDs, not owned UI pointers.
// initialized gates only background/caption setup, not selection-screen processing.
struct TaskData {
    char pad00[0x1C];
    int state;
    void* selectionScreen;
    void* first;
    void* second;
    int initialized;
    char courseSelection[0xC];
};

// The first virtual is the deleting destructor at vtable +0x08.
struct VObj {
    virtual void destroy(int);
};
// Own fixed-size storage and invoke the CharacterEntryScreen constructor on allocation.
struct CharacterEntryScreen : CCharaEntry {};
// Own fixed-size storage and invoke the CharacterCountScreen constructor on allocation.
struct CharacterCountScreen {
    char pad[0x3C];
    // Initialize the owned selection-screen allocation.
    CharacterCountScreen() {
        ((CCharaNum*)this)->__ct();
    }
};
struct TenWords {
    int v[10];
};

// Delete the owned screen through virtual slot +0x08, then clear its pointer.
// Retain the duplicate null checks used by the original inlined ownership cleanup.
inline void CloseSelectionScreen(CPartyMain* self) {
    if (self->selectionScreen) {
        if (self->selectionScreen)
            reinterpret_cast<VObj*>(self->selectionScreen)->destroy(1);
        self->selectionScreen = 0;
    }
}
// Return the shared BP controller to the singleton slot, constructing it only once.
inline void EnsurePartyBpControllerInline() {
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_partyMainBpCtrlDestructorRecord);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

// Borrow the shared render context, constructing save records and owned vectors once.
static inline void* EnsurePartyRenderContext() {
    if (!Sing_805DC648()) {
        if (!s_renderContextInitialized) {
            char* base;
            char* end;
            char* p;
            char* item;
            base = s_renderContext;
            p = base;
            end = base + IFF_OFFSET(RenderContext, vectors);
            do {
                SaveData_InitPlayerRecord(p);
                p += kPlayerSaveRecordSize;
            } while (p < end);
            ((IntVectorArray*)(end))->__ct();
            item = base + IFF_OFFSET(RenderContext, players);
            __construct_array(item, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4);
            ((CEnemyRenderTable*)item)->count = 0;
            InitializeNewRenderContext(base);
            __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv,
                                     s_renderContextDestructorRegistration_PartyMain);
            s_renderContextInitialized = 1;
        }
        Sing_805DC648() = s_renderContext;
    }
    return Sing_805DC648();
}

// Ensure the shared player/render state exists before committing a golf-task transition.
static inline void EnsurePartyRenderContextForTransition() {
    if (!Sing_805DC648()) {
        if (!s_renderContextInitialized) {
            char* end;
            char* base;
            char* p;
            char* item;
            base = s_renderContext;
            p = base;
            end = base + IFF_OFFSET(RenderContext, vectors);
            do {
                SaveData_InitPlayerRecord(p);
                p += kPlayerSaveRecordSize;
            } while (p < end);
            ((IntVectorArray*)(end))->__ct();
            item = base + IFF_OFFSET(RenderContext, players);
            __construct_array(item, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4);
            ((CEnemyRenderTable*)item)->count = 0;
            InitializeNewRenderContext(base);
            __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv,
                                     s_renderContextDestructorRegistration_PartyMain);
            s_renderContextInitialized = 1;
        }
        Sing_805DC648() = s_renderContext;
    }
}

struct PartyItemSelectScreen {
    char pad[0x74];
    // Construct item selection against the shared render context's player table.
    PartyItemSelectScreen() {
        (void)EnsurePartyRenderContext();
        __ct__16CPartyItemSelectFv(this, ((RenderContext*)Sing_805DC648())->players);
    }
};

#pragma push
#pragma use_lmw_stmw on
// Advance player count, character, course, and party-item screens; build players and enter golf on completion.
// delta is the frame time forwarded unchanged to animation and active-screen update.
// Resource setup is polled until loaded. Task requests are asynchronous; ownership
// of the active screen is resolved before requesting a transition.
void CPartyMain::ProcessSelectionScreens(float delta) {
    int screenFinished = 0;
    // Phase 1: always advance preview animation, then poll one-time BP layer setup.
    CharacterLoading_InvokeAnimationCallback(this, delta);
    if (initialized == 0) {
        EnsurePartyBpControllerInline();
        if (((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded() != 0) {
            EnsurePartyBpControllerInline();
            void* backgroundEntry = IdMap_Lookup(Sing_805DC5F0(), primaryBpEntryId);
            BpObject_StartScriptByName(backgroundEntry,
                                       titleTaskStrings + IFF_OFFSET(PartyTitleStrings, backgroundScript), 1);
            TenWords titleGroups = *(TenWords*)partyTitleGroupNames;
            EnsurePartyBpControllerInline();
            void* captionEntry = IdMap_Lookup(Sing_805DC5F0(), secondaryBpEntryId);
            BpObject_StartScriptByName(captionEntry, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, captionScript),
                                       1);
            for (unsigned int i = 0; i < 10; ++i) {
                UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(captionEntry, (void*)titleGroups.v[i]), 0);
            }
            if (AsCSharedDoc(g_sharedDoc)->gameMode == kPartyGameMode) {
                int mode = (int)AsCSharedDoc(g_sharedDoc)->partyGameMode;
                void* h = 0;
                switch (mode) {
                case kPartyMode1:
                    h = BpObject_GetOrCreateNamedGroup(captionEntry,
                                                       titleTaskStrings + IFF_OFFSET(PartyTitleStrings, partyTitle1));
                    break;
                case kPartyMode2:
                    h = BpObject_GetOrCreateNamedGroup(captionEntry,
                                                       titleTaskStrings + IFF_OFFSET(PartyTitleStrings, partyTitle2));
                    break;
                case kPartyMode3:
                    h = BpObject_GetOrCreateNamedGroup(captionEntry,
                                                       titleTaskStrings + IFF_OFFSET(PartyTitleStrings, partyTitle3));
                    break;
                }
                UIGroup_SetEnabled(h, 1);
            } else {
                unsigned char flag = AsCSharedDoc(g_sharedDoc)->golfRuleType;
                if (flag == 0) {
                    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(
                                           captionEntry, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, strokeTitle)),
                                       1);
                } else if (flag == 1) {
                    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(
                                           captionEntry, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, matchTitle)),
                                       1);
                }
            }
            initialized = 1;
        }
    }
    // Phase 2: process the active owned screen; its result is valid only while it lives.
    if (selectionScreen)
        screenFinished = MPBase1_Process(selectionScreen, delta);
    // Phase 3: consume its result, delete it, then install the next screen or request a task.
    if (screenFinished)
        switch (state) {
        case kSelectPlayerCount: {
            int data[2];
            CharaCount_InitInvalidBpHandle(data);
            if (((CCharaNum*)(selectionScreen))->GetSelectedPlayerCount(data) != 0) {
                CloseSelectionScreen(this);
                (void)EnsurePartyRenderContext();
                ((CEnemyRenderTable*)((RenderContext*)Sing_805DC648())->players)->count = data[0];
                selectionScreen = new CharacterSelectScreen;
                state = kSelectCharacter;
            } else {
                CloseSelectionScreen(this);
                ((CTask*)g_taskManager)
                    ->RequestTransitionWithFade(titleTaskStrings + IFF_OFFSET(PartyTitleStrings, titleTask),
                                                titleTaskStrings + IFF_OFFSET(PartyTitleStrings, emptyArgument), 0);
                TaskCommandQueue_AppendNamedRequest(
                    g_taskManager, 0, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, titleMain), 3, 0, 0, 0, 0);
            }
            break;
        }
        case kSelectCharacter:
            if (((CCharaSelect*)(selectionScreen))->IsSelectionConfirmed() != 0) {
                CloseSelectionScreen(this);
                selectionScreen = new CharacterEntryScreen;
                state = kConfirmCharacters;
            } else {
                CloseSelectionScreen(this);
                if (AsCSharedDoc(g_sharedDoc)->golfRuleType == kMatchPlayRule) {
                    ((CTask*)g_taskManager)
                        ->RequestTransitionWithFade(titleTaskStrings + IFF_OFFSET(PartyTitleStrings, titleTask),
                                                    titleTaskStrings + IFF_OFFSET(PartyTitleStrings, emptyArgument), 0);
                    TaskCommandQueue_AppendNamedRequest(
                        g_taskManager, 0, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, titleMain), 3, 0, 0, 0, 0);
                } else {
                    selectionScreen = new CharacterCountScreen;
                    state = kSelectPlayerCount;
                }
            }
            break;
        case kConfirmCharacters:
            if (((CCharaEntry*)(selectionScreen))->IsSelectionConfirmed() != 0) {
                CloseSelectionScreen(this);
                selectionScreen = new CourseSelectScreen;
                if (AsCSharedDoc(g_sharedDoc)->gameMode == kPartyGameMode)
                    ((CCourseSelect*)selectionScreen)->SetMode(AsCSharedDoc(g_sharedDoc)->partyGameMode);
                state = kSelectCourse;
            } else {
                CloseSelectionScreen(this);
                selectionScreen = new CharacterSelectScreen;
                state = kSelectCharacter;
            }
            break;
        case kSelectCourse:
            if (((CCourseSelect*)selectionScreen)->GetSelectionResult(courseSelection) != 0) {
                CloseSelectionScreen(this);
                if ((int)AsCSharedDoc(g_sharedDoc)->partyGameMode == 3) {
                    selectionScreen = new PartyItemSelectScreen;
                    state = kSelectPartyItems;
                } else {
                    CCharacterPreviewTable::DestroyRenderObjects(
                        ((RenderContext*)RenderContext_GetOrCreate())->players);
                    // Commit round settings and variant choices before constructing the golf players.
                    CourseSelection_ApplyRoundSettings(courseSelection);
                    PartyRound_ChooseHoleVariants();
                    EnsurePartyRenderContextForTransition();
                    GolfPlayerList_BuildFromRenderTable(((RenderContext*)Sing_805DC648())->players);
                    ((CTask*)g_taskManager)
                        ->RequestTransitionWithFade(titleTaskStrings + IFF_OFFSET(PartyTitleStrings, golfTask),
                                                    titleTaskStrings + IFF_OFFSET(PartyTitleStrings, golfDoc), 0);
                }
            } else {
                CloseSelectionScreen(this);
                selectionScreen = new CharacterEntryScreen;
                state = kConfirmCharacters;
            }
            break;
        case kSelectPartyItems:
            EnsurePartyRenderContextForTransition();
            if (((CPartyItemSelect*)(selectionScreen))
                    ->CopyCompletedSelection(((RenderContext*)Sing_805DC648())->players) != 0) {
                CloseSelectionScreen(this);
                CCharacterPreviewTable::DestroyRenderObjects(((RenderContext*)RenderContext_GetOrCreate())->players);
                CourseSelection_ApplyRoundSettings(courseSelection);
                PartyRound_ChooseHoleVariants();
                EnsurePartyRenderContextForTransition();
                GolfPlayerList_BuildFromRenderTable(((RenderContext*)Sing_805DC648())->players);
                ((CTask*)g_taskManager)
                    ->RequestTransitionWithFade(titleTaskStrings + IFF_OFFSET(PartyTitleStrings, golfTask),
                                                titleTaskStrings + IFF_OFFSET(PartyTitleStrings, golfDoc), 0);
            } else {
                CloseSelectionScreen(this);
                selectionScreen = new CourseSelectScreen;
                if (AsCSharedDoc(g_sharedDoc)->gameMode == kPartyGameMode)
                    ((CCourseSelect*)selectionScreen)->SetMode(AsCSharedDoc(g_sharedDoc)->partyGameMode);
                state = kSelectCourse;
            }
            break;
        case kSelectionIdle:
            break;
        }
}
#pragma pop

// Adjust the secondary-interface pointer back to the object before destruction.
void CVSMain::DestroySecondaryInterface(int deleteFlag) {
    __dt__7CVSMainFv((char*)this - 0x18, deleteFlag);
}

// Initialize the party-menu angle constant.
void PartyMain_InitTwoPiConstant() {
    s_twoPi_PartyMain = kFloat_6_2831855;
}

struct F3C_vtable {
    void* unk[6];
    void (*init)(void*, void*, void*);
};

// The pop-item dialog's fields. +08 is the BP entry ID (-1 when absent),
// +18/+1C/+20/+28 are owned virtual objects, +24 is an owned shared-file
// handle, and +2C is the BP script index restored at destruction.
struct PopItemDialogView {
    F3C_vtable* vtable;
    void* field04;
    int field08;
    int field0C;
    void* field10;
    unsigned char field14;
    char pad15[3];
    void* field18;
    void* field1C;
    void* field20;
    void* field24;
    void* field28;
    int field2C;
};

// Ensure the borrowed BP controller singleton is available to dialog/draw operations.
static void ensure_805DC5F0() {
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_partyMainBpCtrlDestructorRecord);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

// Initialize the shared invalid BP handle once and register its global destructor.
static void ensure_805DC5F8() {
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_partyBpHandleDestructorRegistration);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
}

struct VOwnedObject {
    virtual void invoke(int);
};

// Release the BP entry, owned screens and shared file, then restore the saved script index.
CPopItemDialog::~CPopItemDialog() {
    PopItemDialogView* obj = (PopItemDialogView*)this;
    obj->vtable = (F3C_vtable*)__vt__14CPopItemDialog;
    if (obj->field08 != -1) {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_PopItem);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(obj->field08);
        obj->field08 = -1;
    }
    if (obj->field18) {
        if (obj->field18)
            reinterpret_cast<VOwnedObject*>(obj->field18)->invoke(1);
        obj->field18 = 0;
    }
    if (obj->field1C) {
        if (obj->field1C)
            reinterpret_cast<VOwnedObject*>(obj->field1C)->invoke(1);
        obj->field1C = 0;
    }
    if (obj->field24) {
        CSharedFileHandle::Destroy(obj->field24, 1);
        obj->field24 = 0;
    }
    if (obj->field20) {
        if (obj->field20)
            reinterpret_cast<VOwnedObject*>(obj->field20)->invoke(1);
        obj->field20 = 0;
    }
    if (obj->field28) {
        if (obj->field28)
            reinterpret_cast<VOwnedObject*>(obj->field28)->invoke(1);
        obj->field28 = 0;
    }
    ensure_805DC5F8();
    ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(obj->field2C);
}

// Draw background and caption BP objects around the active selection screen in the configured order.
void CPartyMain::DrawSelectionScreens() {
    TaskData* obj = (TaskData*)this;
    ensure_805DC5F0();
    void* m = IdMap_Lookup(Sing_805DC5F0(), AsCPartyMain(obj)->primaryBpEntryId);
    BpObject_TryInvokeCurrentCallback(m, 0);
    ensure_805DC5F0();
    m = IdMap_Lookup(Sing_805DC5F0(), AsCPartyMain(obj)->primaryBpEntryId);
    BpObject_Draw(m);
    if (s_partyItemDetailDrawFirst) {
        ensure_805DC5F0();
        m = IdMap_Lookup(Sing_805DC5F0(), AsCPartyMain(obj)->secondaryBpEntryId);
        BpObject_TryInvokeCurrentCallback(m, 0);
        ensure_805DC5F0();
        m = IdMap_Lookup(Sing_805DC5F0(), AsCPartyMain(obj)->secondaryBpEntryId);
        BpObject_Draw(m);
    }
    if (obj->selectionScreen)
        MPBase1_DrawIfInitialized(obj->selectionScreen);
    if (!s_partyItemDetailDrawFirst) {
        ensure_805DC5F0();
        m = IdMap_Lookup(Sing_805DC5F0(), AsCPartyMain(obj)->secondaryBpEntryId);
        BpObject_TryInvokeCurrentCallback(m, 0);
        ensure_805DC5F0();
        m = IdMap_Lookup(Sing_805DC5F0(), AsCPartyMain(obj)->secondaryBpEntryId);
        BpObject_Draw(m);
    }
}

struct VCallF3C {
    virtual void u0();
    virtual void u1();
    virtual void u2();
    virtual void u3();
    virtual void init(void*, void*);
};

// Initialize pop-item dialog ownership and invoke its virtual resource initializer.
CPopItemDialog::CPopItemDialog(void* arg1, void* arg2) {
    PopItemDialogView* obj = (PopItemDialogView*)this;
    obj->vtable = (F3C_vtable*)__vt__14CPopItemDialog;
    obj->field04 = arg1;
    obj->field08 = -1;
    obj->field0C = 0;
    obj->field10 = arg2;
    obj->field14 = 0;
    obj->field18 = 0;
    obj->field1C = 0;
    obj->field20 = 0;
    obj->field24 = 0;
    obj->field28 = 0;
    obj->field2C = -1;
    reinterpret_cast<VCallF3C*>(obj)->init(arg1, arg2);
}

// Party selection task messages: 0 switches to character selection and clears
// the per-player vectors, 1 switches to course selection, and 2/3 disable or
// enable both BP layers.
enum SelectionTaskMessage {
    kShowCharacterSelection = 0,
    kShowCourseSelection = 1,
    kHideSelectionLayers = 2,
    kShowSelectionLayers = 3
};

struct MessageSaveRecord {
    char data[0x828];
    MessageSaveRecord() {
        SaveData_InitPlayerRecord(this);
    }
};
struct MessagePlayerTable {
    char records[0x290];
    int count;
    MessagePlayerTable() {
        MessagePlayerTable* table = this;
        __construct_array(records, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4);
        table->count = 0;
    }
    ~MessagePlayerTable() {
        CharacterPreviewTable_dtor(this, -1);
    }
};
struct MessageRenderContext {
    MessageSaveRecord records[4];
    IntVector vectors[4];
    MessagePlayerTable players;
    MessageRenderContext() {
        RenderContext_ResetPlayerRecords(this);
    }
    DECOMP_CT_MEMBER(MessageRenderContext)
};

static inline void* EnsureMessageRenderContext(void* base) {
    if (!Sing_805DC648()) {
        if (!s_renderContextInitialized) {
            ((MessageRenderContext*)base)->__ct();
            __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv,
                                     s_renderContextDestructorRegistration_PartyMain);
            s_renderContextInitialized = 1;
        }
        Sing_805DC648() = s_renderContext;
    }
    return Sing_805DC648();
}

#pragma push
#pragma use_lmw_stmw on
// Switch selection screens or enable/disable both BP layers in response to task messages.
#pragma opt_propagation off
void CVSMain::HandleMsg(const CVSMainMessage* event) {
    int off;
    int i;
    void* context;
    switch (event->type) {
    case kShowCharacterSelection:
        if (selectionScreen) {
            if (selectionScreen)
                reinterpret_cast<VObj*>(selectionScreen)->destroy(1);
            selectionScreen = 0;
        }
        selectionScreen = new CharacterSelectScreen;
        state = kSelectCharacter;
        {
            context = s_renderContext;
            off = 0;
            for (i = 0; i < 4; ++i) {
                volatile int* slot =
                    (volatile int*)((char*)EnsureMessageRenderContext(context) + IFF_OFFSET(RenderContext, vectors) +
                                    IFF_OFFSET(IntVector, size) + off);
                *slot = *(unsigned int*)slot - *(int*)slot;
                off += 0xC;
            }
        }
        break;
    case kShowCourseSelection:
        if (selectionScreen) {
            if (selectionScreen)
                reinterpret_cast<VObj*>(selectionScreen)->destroy(1);
            selectionScreen = 0;
        }
        selectionScreen = new CourseSelectScreen;
        if (AsCSharedDoc(g_sharedDoc)->gameMode == kPartyGameMode)
            ((CCourseSelect*)selectionScreen)->SetMode(AsCSharedDoc(g_sharedDoc)->partyGameMode);
        state = kSelectCourse;
        break;
    case kHideSelectionLayers: {
        ensure_805DC5F0();
        void* manager = IdMap_Lookup(Sing_805DC5F0(), primaryBpEntryId);
        BpObject_SetEnabled(manager, 0);
        ensure_805DC5F0();
        manager = IdMap_Lookup(Sing_805DC5F0(), secondaryBpEntryId);
        BpObject_SetEnabled(manager, 0);
        break;
    }
    case kShowSelectionLayers: {
        ensure_805DC5F0();
        void* manager = IdMap_Lookup(Sing_805DC5F0(), primaryBpEntryId);
        BpObject_SetEnabled(manager, 1);
        ensure_805DC5F0();
        manager = IdMap_Lookup(Sing_805DC5F0(), secondaryBpEntryId);
        BpObject_SetEnabled(manager, 1);
        break;
    }
    }
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804623F8", _ctors_u80284D14_0, PartyMain_InitTwoPiConstant);
#pragma pop
