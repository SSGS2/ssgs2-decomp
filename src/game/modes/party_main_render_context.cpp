#include <decomp/static_init.h>
#include <decomp/ct_member.h>
#include "iff/IffStruct.h"
#include "classes/CEnemyRenderTable.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

#include "game/vector.h"
// Translation unit 0x80284D14-0x802861B0 (7 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CPartyMain.h"

#define AsCPartyMain(p) ((CPartyMain*)(p))
extern "C" {

void RenderContext_ResetPlayerRecords(void*);
void* CharacterPreviewTable_dtor(void*, int);
void appFreeUni__FPv(void*);

struct RenderContextConstructionCleanup_u80284D14 {
    char* base;
    bool active;
    // Arm cleanup of embedded player/vector storage during singleton construction.
    explicit RenderContextConstructionCleanup_u80284D14(void* object)
        : base((char*)object), active(true) {}
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

void CVSMain_dtor(void*, int);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor();
void __register_global_object(void*, void*, void*);
void CBpCtrl_DeleteEntryById(void*, int);
void* IdMap_Lookup(void*, void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void CSharedFileHandle_Destroy(void*, int);
void MPBase1_DrawIfInitialized(void*);
void CBpCtrlHandle_SetScriptIndex(void*, int);
void __dl__FPv(void*);
void CBpCtrlHandle_Dtor();
void* __nw__FUl(unsigned int);
void CCharaSelect_Ctor(void*);
void SaveData_InitPlayerRecord(void*);


void __construct_array(void*, void*, void*, int, int);
void PartyItemRecord_InitDefaults(void*);
void PlayerRenderRecord_dtor(void*);
void RenderContext_ResetPlayerRecords(void*);
void* RenderContext_GetOrCreate();
void __dt__13RenderContextFv(void*);
extern char s_renderContext[];
extern char s_renderContextDestructorRegistration_PartyMain[];
void CCourseSelect_Ctor(void*);
void CCourseSelect_SetMode(void*, void*);
extern void* g_sharedDoc;
void BpObject_SetEnabled(void*, int);
void CharacterLoading_InvokeAnimationCallback(void*, float);
int CBpCtrl_PollAllEntriesLoaded(void*);
void* IdMap_Lookup(void*, void*);
void* BpObject_StartScriptByName(void*, const char*, int);
void* BpObject_GetOrCreateNamedGroup(void*, void*);
void CCharaEntry_Ctor(void*);
void CCharaNum_Ctor(void*);
void UIGroup_SetEnabled(void*, int);
extern char titleTaskStrings[];
extern int partyTitleGroupNames[];
extern void* g_sharedDoc;
extern void* g_taskManager;
extern char s_renderContext[];
int MPBase1_Process(void*, float);
void CharaCount_InitInvalidBpHandle(void*);
int CCharaNum_GetSelectedPlayerCount(void*, void*);
void CTask_RequestTransitionWithFade(void*, const char*, const char*, int);
void TaskCommandQueue_AppendNamedRequest(void*, int, const char*, int, int, int, int, int);
int CCharaSelect_IsSelectionConfirmed(void*);
int CCharaEntry_IsSelectionConfirmed(void*);
int CCourseSelect_GetSelectionResult(void*, void*);
void CCharacterPreviewTable_DestroyRenderObjects(void*);
void CourseSelection_ApplyRoundSettings(void*);
void PartyRound_ChooseHoleVariants();
void GolfPlayerList_BuildFromRenderTable(void*);
int CPartyItemSelect_CopyCompletedSelection(void*, void*);
void CPartyItemSelect_ctor(void*, void*);
// Own fixed-size storage and invoke the CharacterSelectScreen constructor on allocation.
struct CharacterSelectScreen {
    char pad[0x60];
    // Initialize the owned selection-screen allocation.
    CharacterSelectScreen() { CCharaSelect_Ctor(this); }
};
// Own fixed-size storage and invoke the CourseSelectScreen constructor on allocation.
struct CourseSelectScreen {
    char pad[0x80];
    // Initialize the owned selection-screen allocation.
    CourseSelectScreen() { CCourseSelect_Ctor(this); }
};
static void ensure_805DC5F0();

enum PartySelectionState { kSelectPlayerCount = 1, kSelectCharacter = 2, kConfirmCharacters = 3,
    kSelectCourse = 4, kSelectPartyItems = 5, kSelectionIdle = 6 };
enum { kPartyGameMode = 7, kMatchPlayRule = 1, kPartyPlayerCount = 4,
    kPlayerSaveRecordSize = 0x828, kTitleGroupCount = 10,
    // Party-mode captions use a separate domain from selection-screen states.
    kPartyMode1 = 1, kPartyMode2 = 2, kPartyMode3 = 3 };
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

struct VObj { 
virtual void destroy(int);
};
// Own fixed-size storage and invoke the CharacterEntryScreen constructor on allocation.
struct CharacterEntryScreen {
    char pad[0x68];
    // Initialize the owned selection-screen allocation.
    CharacterEntryScreen() { CCharaEntry_Ctor(this); }
};
// Own fixed-size storage and invoke the CharacterCountScreen constructor on allocation.
struct CharacterCountScreen {
    char pad[0x3C];
    // Initialize the owned selection-screen allocation.
    CharacterCountScreen() { CCharaNum_Ctor(this); }
};
struct TenWords { int v[10]; };

// Delete the owned screen through virtual slot +0x08, then clear its pointer.
// Retain the duplicate null checks used by the original inlined ownership cleanup.
inline void CloseSelectionScreen(TaskData* self) {
    if (self->selectionScreen) {
        if (self->selectionScreen) reinterpret_cast<VObj*>(self->selectionScreen)->destroy(1);
        self->selectionScreen = 0;
    }
}
// Return the shared BP controller to the singleton slot, constructing it only once.
inline void EnsurePartyBpControllerInline() {
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
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
            do { SaveData_InitPlayerRecord(p); p += kPlayerSaveRecordSize; } while (p < end);
            ((IntVectorArray*)(end))->__ct();
            item = base + IFF_OFFSET(RenderContext, players);
            __construct_array(item, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4);
            ((CEnemyRenderTable*)item)->count = 0;
            InitializeNewRenderContext(base);
            __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv, s_renderContextDestructorRegistration_PartyMain);
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
            do { SaveData_InitPlayerRecord(p); p += kPlayerSaveRecordSize; } while (p < end);
            ((IntVectorArray*)(end))->__ct();
            item = base + IFF_OFFSET(RenderContext, players);
            __construct_array(item, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4);
            ((CEnemyRenderTable*)item)->count = 0;
            InitializeNewRenderContext(base);
            __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv, s_renderContextDestructorRegistration_PartyMain);
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
        CPartyItemSelect_ctor(this, ((RenderContext*)Sing_805DC648())->players);
    }
};

#pragma push
#pragma use_lmw_stmw on
// Advance player count, character, course, and party-item screens; build players and enter golf on completion.
// delta is the frame time forwarded unchanged to animation and active-screen update.
// Resource setup is polled until loaded. Task requests are asynchronous; ownership
// of the active screen is resolved before requesting a transition.
void CPartyMain_ProcessSelectionScreens(TaskData* self, float delta) {
    int screenFinished = 0;
    // Phase 1: always advance preview animation, then poll one-time BP layer setup.
    CharacterLoading_InvokeAnimationCallback(self, delta);
    if (self->initialized == 0) {
        EnsurePartyBpControllerInline();
        if (CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0()) != 0) {
            EnsurePartyBpControllerInline();
            void* backgroundEntry = IdMap_Lookup(Sing_805DC5F0(), AsCPartyMain(self)->primaryBpEntryId);
            BpObject_StartScriptByName(backgroundEntry, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, backgroundScript), 1);
            TenWords titleGroups = *(TenWords*)partyTitleGroupNames;
            EnsurePartyBpControllerInline();
            void* captionEntry = IdMap_Lookup(Sing_805DC5F0(), AsCPartyMain(self)->secondaryBpEntryId);
            BpObject_StartScriptByName(captionEntry, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, captionScript), 1);
            for (unsigned int i = 0; i < 10; ++i) {
                UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(captionEntry, (void*)titleGroups.v[i]), 0);
            }
            if (AsCSharedDoc(g_sharedDoc)->gameMode == kPartyGameMode) {
                int mode = (int)AsCSharedDoc(g_sharedDoc)->partyGameMode;
                void* h = 0;
                switch (mode) {
                case kPartyMode1: h = BpObject_GetOrCreateNamedGroup(captionEntry, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, partyTitle1)); break;
                case kPartyMode2: h = BpObject_GetOrCreateNamedGroup(captionEntry, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, partyTitle2)); break;
                case kPartyMode3: h = BpObject_GetOrCreateNamedGroup(captionEntry, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, partyTitle3)); break;
                }
                UIGroup_SetEnabled(h, 1);
            } else {
                unsigned char flag = AsCSharedDoc(g_sharedDoc)->golfRuleType;
                if (flag == 0) {
                    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(captionEntry, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, strokeTitle)), 1);
                } else if (flag == 1) {
                    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(captionEntry, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, matchTitle)), 1);
                }
            }
            self->initialized = 1;
        }
    }
    // Phase 2: process the active owned screen; its result is valid only while it lives.
    if (self->selectionScreen) screenFinished = MPBase1_Process(self->selectionScreen, delta);
    // Phase 3: consume its result, delete it, then install the next screen or request a task.
    if (screenFinished) switch (self->state) {
    case kSelectPlayerCount: {
        int data[2];
        CharaCount_InitInvalidBpHandle(data);
        if (CCharaNum_GetSelectedPlayerCount(self->selectionScreen, data) != 0) {
            CloseSelectionScreen(self);
            (void)EnsurePartyRenderContext();
            ((CEnemyRenderTable*)((RenderContext*)Sing_805DC648())->players)->count = data[0];
            self->selectionScreen = new CharacterSelectScreen;
            self->state = kSelectCharacter;
        } else {
            CloseSelectionScreen(self);
            CTask_RequestTransitionWithFade(g_taskManager, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, titleTask), titleTaskStrings + IFF_OFFSET(PartyTitleStrings, emptyArgument), 0);
            TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, titleMain), 3, 0, 0, 0, 0);
        }
        break;
    }
    case kSelectCharacter:
        if (CCharaSelect_IsSelectionConfirmed(self->selectionScreen) != 0) {
            CloseSelectionScreen(self);
            self->selectionScreen = new CharacterEntryScreen;
            self->state = kConfirmCharacters;
        } else {
            CloseSelectionScreen(self);
            if (AsCSharedDoc(g_sharedDoc)->golfRuleType == kMatchPlayRule) {
                CTask_RequestTransitionWithFade(g_taskManager, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, titleTask), titleTaskStrings + IFF_OFFSET(PartyTitleStrings, emptyArgument), 0);
                TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, titleMain), 3, 0, 0, 0, 0);
            } else {
                self->selectionScreen = new CharacterCountScreen;
                self->state = kSelectPlayerCount;
            }
        }
        break;
    case kConfirmCharacters:
        if (CCharaEntry_IsSelectionConfirmed(self->selectionScreen) != 0) {
            CloseSelectionScreen(self);
            self->selectionScreen = new CourseSelectScreen;
            if (AsCSharedDoc(g_sharedDoc)->gameMode == kPartyGameMode)
                CCourseSelect_SetMode(self->selectionScreen, (void*)AsCSharedDoc(g_sharedDoc)->partyGameMode);
            self->state = kSelectCourse;
        } else {
            CloseSelectionScreen(self);
            self->selectionScreen = new CharacterSelectScreen;
            self->state = kSelectCharacter;
        }
        break;
    case kSelectCourse:
        if (CCourseSelect_GetSelectionResult(self->selectionScreen, self->courseSelection) != 0) {
            CloseSelectionScreen(self);
            if ((int)AsCSharedDoc(g_sharedDoc)->partyGameMode == 3) {
                self->selectionScreen = new PartyItemSelectScreen;
                self->state = kSelectPartyItems;
            } else {
                CCharacterPreviewTable_DestroyRenderObjects(((RenderContext*)RenderContext_GetOrCreate())->players);
                // Commit round settings and variant choices before constructing the golf players.
                CourseSelection_ApplyRoundSettings(self->courseSelection);
                PartyRound_ChooseHoleVariants();
                EnsurePartyRenderContextForTransition();
                GolfPlayerList_BuildFromRenderTable(((RenderContext*)Sing_805DC648())->players);
                CTask_RequestTransitionWithFade(g_taskManager, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, golfTask), titleTaskStrings + IFF_OFFSET(PartyTitleStrings, golfDoc), 0);
            }
        } else {
            CloseSelectionScreen(self);
            self->selectionScreen = new CharacterEntryScreen;
            self->state = kConfirmCharacters;
        }
        break;
    case kSelectPartyItems:
        EnsurePartyRenderContextForTransition();
        if (CPartyItemSelect_CopyCompletedSelection(self->selectionScreen, ((RenderContext*)Sing_805DC648())->players) != 0) {
            CloseSelectionScreen(self);
            CCharacterPreviewTable_DestroyRenderObjects(((RenderContext*)RenderContext_GetOrCreate())->players);
            CourseSelection_ApplyRoundSettings(self->courseSelection);
            PartyRound_ChooseHoleVariants();
            EnsurePartyRenderContextForTransition();
            GolfPlayerList_BuildFromRenderTable(((RenderContext*)Sing_805DC648())->players);
            CTask_RequestTransitionWithFade(g_taskManager, titleTaskStrings + IFF_OFFSET(PartyTitleStrings, golfTask), titleTaskStrings + IFF_OFFSET(PartyTitleStrings, golfDoc), 0);
        } else {
            CloseSelectionScreen(self);
            self->selectionScreen = new CourseSelectScreen;
            if (AsCSharedDoc(g_sharedDoc)->gameMode == kPartyGameMode)
                CCourseSelect_SetMode(self->selectionScreen, (void*)AsCSharedDoc(g_sharedDoc)->partyGameMode);
            self->state = kSelectCourse;
        }
        break;
    case kSelectionIdle:
        break;
    }
}
#pragma pop

// Adjust the secondary-base pointer before forwarding destruction to CVSMain.
void CVSMain_DtorThunk18(void* ptr, int forwarded1) {
    CVSMain_dtor((char*)ptr - 0x18, forwarded1);
}

// Initialize the party-menu angle constant.
void PartyMain_InitTwoPiConstant() {
    s_twoPi_PartyMain = kFloat_6_2831855;
}

struct F3C_vtable {
    void* unk[6];
    void (*init)(void*, void*, void*);
};

// Pop-item dialog layout ONLY. Party-main drawing previously reused this view,
// although +24/+28 mean BP entry IDs there. The draw path now uses CPartyMain.
// Pop-item fields: +04/+10 borrowed constructor inputs (exact roles unresolved),
// +08 BP entry ID (-1 absent), +18/+1C/+20/+28 owned virtual objects,
// +24 owned shared-file handle, +2C saved BP script index restored at destruction.
// +0C/+14 are initialized state words; their remaining meanings are unresolved.
struct F3C_object {
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
            __ct__7CBpCtrlFv(s_bpCtrl);
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

struct VCall80285FB4 { 
virtual void invoke(int);
};

// Release the BP entry, owned screens and shared file, then restore the saved script index.
F3C_object* CPopItemDialog_dtor(F3C_object* obj, int deleting) {
    if (obj) {
        obj->vtable = (F3C_vtable*)__vt__14CPopItemDialog;
        if (obj->field08 != -1) {
            if (!Sing_805DC5F0()) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_PopItem);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), obj->field08);
            obj->field08 = -1;
        }
        if (obj->field18) { if (obj->field18) reinterpret_cast<VCall80285FB4*>(obj->field18)->invoke(1); obj->field18 = 0; }
        if (obj->field1C) { if (obj->field1C) reinterpret_cast<VCall80285FB4*>(obj->field1C)->invoke(1); obj->field1C = 0; }
        if (obj->field24) { CSharedFileHandle_Destroy(obj->field24, 1); obj->field24 = 0; }
        if (obj->field20) { if (obj->field20) reinterpret_cast<VCall80285FB4*>(obj->field20)->invoke(1); obj->field20 = 0; }
        if (obj->field28) { if (obj->field28) reinterpret_cast<VCall80285FB4*>(obj->field28)->invoke(1); obj->field28 = 0; }
        ensure_805DC5F8();
        CBpCtrlHandle_SetScriptIndex(Sing_805DC5F8(), obj->field2C);
        if (deleting > 0) __dl__FPv(obj);
    }
    return obj;
}

// Draw background and caption BP objects around the active selection screen in the configured order.
void CPartyMain_DrawSelectionScreens(TaskData* obj) {
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
    if (obj->selectionScreen) MPBase1_DrawIfInitialized(obj->selectionScreen);
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
virtual void u0(); virtual void u1(); virtual void u2(); virtual void u3(); virtual void init(void*, void*);
};

// Initialize pop-item dialog ownership and invoke its virtual resource initializer.
F3C_object* CPopItemDialog_Ctor(F3C_object* obj, void* arg1, void* arg2) {
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
    return obj;
}

// Selection task event ABI: command type at +04. This handler consumes ONLY type;
// payload words +20/+24 are present in the general event record but unused here.
// 0 switches to character selection and clears per-player vectors; 1 switches to
// course selection; 2/3 disable/enable both BP layers. No payload ownership transfers.
enum SelectionTaskMessage { kShowCharacterSelection = 0, kShowCourseSelection = 1,
    kHideSelectionLayers = 2, kShowSelectionLayers = 3 };
// EventOwner is a party/VS task view: ptr is the owned screen at +20;
// first and second are BP entry IDs at +24/+28, not object pointers to delete.
struct EventData { int unk0; int type; char pad[0x18]; void* first; void* second; };
struct EventOwner { char pad[0x1c]; int state; void* ptr; void* first; void* second; };
// VObj's first virtual is the deleting destructor at vtable +0x08.

struct MessageSaveRecord {
    char data[0x828];
    MessageSaveRecord() { SaveData_InitPlayerRecord(this); }
};
struct MessagePlayerTable {
    char records[0x290];
    int count;
    MessagePlayerTable() {
        MessagePlayerTable* table = this;
        __construct_array(records, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4);
        table->count = 0;
    }
    ~MessagePlayerTable() { CharacterPreviewTable_dtor(this, -1); }
};
struct MessageRenderContext {
    MessageSaveRecord records[4];
    IntVector vectors[4];
    MessagePlayerTable players;
    MessageRenderContext() { RenderContext_ResetPlayerRecords(this); }
    DECOMP_CT_MEMBER(MessageRenderContext)
};

static inline void* EnsureMessageRenderContext(void* base) {
    if (!Sing_805DC648()) {
        if (!s_renderContextInitialized) {
            ((MessageRenderContext*)base)->__ct();
            __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv, s_renderContextDestructorRegistration_PartyMain);
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
void CVSMain_HandleMsg(EventOwner* owner, const EventData* event) {
    int off;
    int i;
    void* context;
    switch (event->type) {
    case kShowCharacterSelection:
        if (owner->ptr) {
            if (owner->ptr) reinterpret_cast<VObj*>(owner->ptr)->destroy(1);
            owner->ptr = 0;
        }
        owner->ptr = new CharacterSelectScreen;
        owner->state = kSelectCharacter;
        {
            context = s_renderContext;
            off = 0;
            for (i = 0; i < 4; ++i) {
                volatile int* slot = (volatile int*)((char*)EnsureMessageRenderContext(context) + IFF_OFFSET(RenderContext, vectors) + IFF_OFFSET(IntVector, size) + off);
                *slot = *(unsigned int*)slot - *(int*)slot;
                off += 0xC;
            }
        }
        break;
    case kShowCourseSelection:
        if (owner->ptr) {
            if (owner->ptr) reinterpret_cast<VObj*>(owner->ptr)->destroy(1);
            owner->ptr = 0;
        }
        owner->ptr = new CourseSelectScreen;
        if (AsCSharedDoc(g_sharedDoc)->gameMode == kPartyGameMode)
            CCourseSelect_SetMode(owner->ptr, (void*)AsCSharedDoc(g_sharedDoc)->partyGameMode);
        owner->state = kSelectCourse;
        break;
    case kHideSelectionLayers:
    {
        ensure_805DC5F0();
        void* manager = IdMap_Lookup(Sing_805DC5F0(), owner->first);
        BpObject_SetEnabled(manager, 0);
        ensure_805DC5F0();
        manager = IdMap_Lookup(Sing_805DC5F0(), owner->second);
        BpObject_SetEnabled(manager, 0);
        break;
    }
    case kShowSelectionLayers:
    {
        ensure_805DC5F0();
        void* manager = IdMap_Lookup(Sing_805DC5F0(), owner->first);
        BpObject_SetEnabled(manager, 1);
        ensure_805DC5F0();
        manager = IdMap_Lookup(Sing_805DC5F0(), owner->second);
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
