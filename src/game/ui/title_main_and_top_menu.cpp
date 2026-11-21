#include <decomp/static_init.h>
// Byte offsets preserve the pooled-string storage and original relocation addends.
enum ModeTransitionStringsOffset {
    kModeTransitionStrings_CTourTask = 0xB, // CTourTask
    kModeTransitionStrings_emptyArgument = 0x15, // 
    kModeTransitionStrings_TourMain = 0x16, // TourMain
    kModeTransitionStrings_CSingleTaskRev = 0x1F, // CSingleTaskRev
    kModeTransitionStrings_SingleMainRev = 0x2E, // SingleMainRev
    kModeTransitionStrings_CTrainingTask = 0x3C, // CTrainingTask
    kModeTransitionStrings_CShop2Task = 0x4A, // CShop2Task
    kModeTransitionStrings_Shop2 = 0x55, // Shop2
    kModeTransitionStrings_CRoundDolTask = 0x5B, // CRoundDolTask
    kModeTransitionStrings_CBgmCollectionTask = 0x69, // CBgmCollectionTask
    kModeTransitionStrings_CPasswordTask = 0x7C, // CPasswordTask
    kModeTransitionStrings_CPartyTask = 0x8A, // CPartyTask
    kModeTransitionStrings_CGolfDoc = 0x95, // CGolfDoc
    kModeTransitionStrings_PartyMain = 0x9E, // PartyMain
    kModeTransitionStrings_CFightTask = 0xA8, // CFightTask
    kModeTransitionStrings_FightMain = 0xB3, // FightMain
    kModeTransitionStrings_CGolfTask = 0xBD, // CGolfTask
};

// Byte offsets preserve the pooled-string storage and original relocation addends.
enum TopMenuResourceStringsOffset {
    kTopMenuResourceStrings_top_menu_srd = 0xD, // top_menu.srd
    kTopMenuResourceStrings_top_menu_b2d = 0x1A, // top_menu.b2d
    kTopMenuResourceStrings_topmenu_zip = 0x27, // topmenu.zip
};

enum TitleScreenState { kTitleInactive = -1, kTitleWarning = 1, kTitleStartScreen = 2,
    kTitleSingleMenu = 5, kTitleSaveLoad = 6, kTitleMultiMenu = 7, kTitleWaitForMenuCompletion = 8 };

// menuState describes the owned activeMenu, not the chosen game mode:
// -1 idle; 1 warning -> 2 title; 2 choice -> 6 save/load; 6 -> 5 single/7 multi;
// 5/7 either schedule a task transition or return to 2 on cancellation.
// State 8 waits for the menu's completion byte, then replaces it with the title.
// Committed result codes are a separate namespace, consumed by OnProcess below.
enum TitleMenuResult {
    kResultTour = 0, kResultSingle0 = 1, kResultSingle1 = 2,
    kResultParty3 = 3, kResultParty1 = 4, kResultParty2 = 5,
    kResultFight4 = 6, kResultFight5 = 7,
    kResultPractice = 8, kResultAlternatePractice = 9, kResultTraining = 10,
    kResultShop = 11, kResultRoundDol = 13, kResultBgmCollection = 14,
    kResultPassword = 15, kResultCancelled = -1
};
// Suffixes on single/party/fight results describe forwarded numeric arguments;
// gameplay labels for those variants have not been established by this unit.
enum TopMenuLimits { kTopMenuLayoutCount = 3, kTopMenuItemCount = 20 };

// Practice setup uses three six-entry rows; the two tables have a one-byte gap.
struct PracticeSharedDocFields {
    char prefix[0x82A];
    unsigned char playerFlags[18];
    char reserved83C;
    unsigned char courseIds[18];
    char reserved84F[0x72];
    char ruleName[0x104];
    unsigned char holeCount;
    unsigned char roundOption;
};
struct PracticeSaveFlags { char prefix[0x4E8]; unsigned int flags; };
enum PracticeMode { kPracticeAlternateSavedVariant = 2, kPracticeAlternateInitial = 3, kPracticeDefault = 4,
    kPracticeCourseId = 0xFD, kPracticeRuleType = 2 };
typedef char Practice_ruleName_offset[((int)&((PracticeSharedDocFields*)0)->ruleName == 0x8C1) ? 1 : -1];
typedef char Practice_roundOption_offset[((int)&((PracticeSharedDocFields*)0)->roundOption == 0x9C6) ? 1 : -1];

#include "classes/SavePlayerRecord.h"
#include "classes/CMouseCursor.h"
#include "classes/CSharedDoc.h"
#include "classes/CEnemyRenderRecord.h"
#include "classes/CEnemyRenderTable.h"
#include "classes/CFlashMemoryMenuLoad.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))


// Normal destruction remains explicit. This owner handles exceptional exits.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Own destruction of a partially constructed menu base during unwinding.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Leave the completed base under the containing menu's ownership.
    void release() { active = false; }
    // Destroy the base only if construction exited before release().
    ~EH5Cleanup() { if (active) destroy(object, -1); }
};

// Translation unit 0x802BD994-0x802BEFE0 (12 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CPangya2TitleMain.h"
#include "classes/CPangya2TopMenu.h"

#define AsCPangya2TopMenu(p) ((CPangya2TopMenu*)(p))

#define AsCPangya2TitleMain(p) ((CPangya2TitleMain*)(p))
class CObjBase {
public:
    virtual void v2(int);
};

class CObjBase4 {
public:
    virtual void v0();
    virtual int v1();
};

extern "C" {

extern unsigned char s_titleMainRttiDescriptor[1]; // s_titleMainRttiDescriptor
extern const float kFloat_6_2831855;
extern float s_titleMainAngleWrapConstant; // s_titleMainAngleWrapConstant

extern signed char s_movieControllerInitialized;
extern char s_movieController[0x2EC0];
char s_fadeOverlayDtorRegistration_TitleMain[12]; // s_fadeOverlayDtorRegistration_TitleMain
char s_movieControllerDtorRegistration_TitleMain[12]; // s_movieControllerDtorRegistration_TitleMain
char lbl_80511AB0[12]; // s_systemResetDtorRegistration_TitleMain

void* CPangya2TitleMain_dtor(void*, int);

void MovieController_Ctor(void*); // MovieController_Ctor
void MovieController_Dtor(); // MovieController_Dtor
void* __register_global_object(void*, void*, void*);
void MovieController_InvokeNoOpHook(void*);
void MoviePlayer_DrawCurrentSamples(void*);
void MPBase1_DrawIfInitialized(void*);

// Return the existing title-main runtime type descriptor.
void* CPangya2TitleMain_GetRTTI() {
    return s_titleMainRttiDescriptor; // s_titleMainRttiDescriptor
}

// Initialize the title-main angle-wrap constant at startup.
void TitleMain_InitAngleWrapConstant() {
    s_titleMainAngleWrapConstant = kFloat_6_2831855; // s_titleMainAngleWrapConstant
}

// Adjust the secondary-base pointer before destroying title-main.
void* CPangya2TitleMain_DtorThunk24(void* p, int deleting) {
    return CPangya2TitleMain_dtor((char*)p - 0x18, deleting);
}

int CPangya2TopMenu_RequestSubMenu(void*, int);

#pragma dont_inline on
// Write the committed menu result only when it is ready.
int CPangya2TopMenu_TryGetResult(void* self, int* out) {
    // Return 1 and write *out when committed; otherwise return 0, leaving *out alone.
    if (AsCPangya2TopMenu(self)->resultReady) {
        *out = AsCPangya2TopMenu(self)->result;
        return 1;
    }
    return 0;
}

void* __nw__FUl(unsigned long);
void __dl__FPv(void*);
void CPangya2TitleTop_Ctor(void*);
void* CPangya2TopMenu_Ctor(void*);
void __ct__12CWarningLogoFv(void*);
void __ct__17CMultiPlayTopMenuFv(void*);

extern int MPBase1_Process(void*);
extern int CWarningLogo_IsFinished(void*);
extern int Object_GetByte30(void*);
extern int CPangya2TitleTop_TryGetPlayMode(void*, int*);
extern int CFlashMemoryMenuLoad_Create(int);
extern void MovieController_StopAndFreeSamples(void*);
extern void* Runtime_DynamicCast(void*, int, void*, void*, int); // Runtime_DynamicCast
extern void FadeOverlay_Init(void*);
extern void ResourceState_Dtor();
extern void FadeState_StartFadeOut(void*, int);
extern void VISetBlack(int);
extern void SystemResetState_Ctor(void*); // SystemResetState_Ctor
extern void SystemResetState_Dtor(); // SystemResetState_Dtor
extern void SystemReset_RequestDeferredMode(void*, int);
extern void SoundManager_ApplySavedAudioSettings(void*, float);
extern void CTask_RequestTransition(void*, const char*, const char*);
extern void TaskCommandQueue_AppendNamedRequest(void*, int, const char*, int, int, int, int, int);
extern void CTask_RequestTransitionWithFade(void*, const char*, const char*, int);
extern int CMultiPlayTopMenu_TryGetSelection(void*, int*); // CMultiPlayTopMenu_TryGetResult
extern void FadeState_StartFadeIn(void*, int);
extern void* g_soundManager;
extern void* g_taskManager;
extern signed char s_fadeOverlayInitialized;
extern signed char s_systemResetStateInitialized;
extern char __RTTI__20CFlashMemoryMenuLoad[] __attribute__((section(".sdata")));
extern char __RTTI__9IMenuBase[] __attribute__((section(".sdata")));
extern char s_fadeOverlay[];
extern char s_systemResetStorage[];
extern char modeTransitionStrings[];
extern char s_fadeOverlayDtorRegistration_TitleMain[]; // s_fadeOverlayDtorRegistration_TitleMain
extern char s_movieControllerDtorRegistration_TitleMain[]; // s_movieControllerDtorRegistration_TitleMain
extern const float kFloat_1_TitleMainAudio; // kFloat_1_TitleMainAudio
void CPangya2TitleMain_StartPracticeSession(void*, int);

#pragma dont_inline off
#pragma always_inline on
struct TitleTop {
    char storage[0x40];
    // Construct the title-start screen in newly allocated menu storage.
    TitleTop() { CPangya2TitleTop_Ctor(this); }
};
struct TopMenu {
    char storage[0x84];
    // Construct the single-player top menu in newly allocated storage.
    TopMenu() { CPangya2TopMenu_Ctor(this); }
};
struct MultiTop {
    char storage[0x60];
    // Construct the multiplayer top menu in newly allocated storage.
    MultiTop() { __ct__17CMultiPlayTopMenuFv(this); }
};
#pragma always_inline reset
#pragma inline_depth(32)
#pragma inline_max_size(100000)
#pragma inline_max_total_size(100000)
#pragma opt_strength_reduction off
// Advance title menus, handle save-load failure and route chosen modes to their tasks.
void CPangya2TitleMain_OnProcess(void* self) {
    // MPBase completion gates ALL state handling. Replacing a menu runs its
    // deleting virtual destructor first; a scheduled task transition leaves
    // title ownership to the enclosing task lifecycle.
    char* title = (char*)self;
    void* nextMenu;
    if (AsCPangya2TitleMain(title)->menuState == kTitleInactive) return;
    if (!MPBase1_Process(AsCPangya2TitleMain(title)->activeMenu)) return;

    switch (AsCPangya2TitleMain(title)->menuState) {
    case kTitleWarning: {
        if (!CWarningLogo_IsFinished(AsCPangya2TitleMain(title)->activeMenu)) return;
        {
            void* p = AsCPangya2TitleMain(title)->activeMenu;
            if (p) {
                if (p) ((CObjBase*)p)->v2(1);
                AsCPangya2TitleMain(title)->activeMenu = 0;
            }
        }
        {
            nextMenu = new TitleTop;
            AsCPangya2TitleMain(title)->activeMenu = nextMenu;
            AsCPangya2TitleMain(title)->menuState = kTitleStartScreen;
        }
        return;
    }
    case kTitleWaitForMenuCompletion: {
        if (!Object_GetByte30(AsCPangya2TitleMain(title)->activeMenu)) return;
        {
            void* p = AsCPangya2TitleMain(title)->activeMenu;
            if (p) {
                if (p) ((CObjBase*)p)->v2(1);
                AsCPangya2TitleMain(title)->activeMenu = 0;
            }
        }
        {
            nextMenu = new TitleTop;
            AsCPangya2TitleMain(title)->activeMenu = nextMenu;
            AsCPangya2TitleMain(title)->menuState = kTitleStartScreen;
        }
        return;
    }
    case kTitleStartScreen: {
        if (!CPangya2TitleTop_TryGetPlayMode(AsCPangya2TitleMain(title)->activeMenu, (int*)(title + (int)&((CPangya2TitleMain*)0)->topMenuResult))) return;
        {
            void* p = AsCPangya2TitleMain(title)->activeMenu;
            if (p) {
                if (p) ((CObjBase*)p)->v2(1);
                AsCPangya2TitleMain(title)->activeMenu = 0;
            }
        }
        int ok = 1;
        if (AsCPangya2TitleMain(title)->topMenuResult == 0) ok = 0;
        void* obj = (void*)CFlashMemoryMenuLoad_Create(ok);
        AsCPangya2TitleMain(title)->activeMenu = obj;
        AsCPangya2TitleMain(title)->menuState = kTitleSaveLoad;
        if (!Sing_805DC950()) {
            if (!s_movieControllerInitialized) {
                MovieController_Ctor(s_movieController); // MovieController_Ctor
                __register_global_object(s_movieController, (void*)MovieController_Dtor, s_movieControllerDtorRegistration_TitleMain);
                s_movieControllerInitialized = 1;
            }
            Sing_805DC950() = (void*)s_movieController;
        }
        MovieController_StopAndFreeSamples(Sing_805DC950());
        return;
    }
    case kTitleSaveLoad: {
        // Failure closes the loader, blanks video and requests deferred reset.
        // Success reapplies saved audio settings before choosing single/multi UI.
        void* obj = AsCPangya2TitleMain(title)->activeMenu;
        if (!obj) return;
        if (((CObjBase4*)obj)->v1()) {
            void* found = Runtime_DynamicCast(AsCPangya2TitleMain(title)->activeMenu, 0, __RTTI__20CFlashMemoryMenuLoad, __RTTI__9IMenuBase, 0); // Runtime_DynamicCast
            unsigned char storageCheckFailed = 0;
            if (found) storageCheckFailed = ((CFlashMemoryMenuLoad*)found)->storageCheckFailed;
            unsigned char invalidSaveData = 0;
            if (found) invalidSaveData = ((CFlashMemoryMenuLoad*)found)->invalidSaveData;
            {
                void* p = AsCPangya2TitleMain(title)->activeMenu;
                if (p) {
                    if (p) ((CObjBase*)p)->v2(1);
                    AsCPangya2TitleMain(title)->activeMenu = 0;
                }
            }
            if (storageCheckFailed || invalidSaveData) {
                if (!Sing_805DC608()) {
                    if (!s_fadeOverlayInitialized) {
                        FadeOverlay_Init(s_fadeOverlay);
                        __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration_TitleMain); // s_fadeOverlayDtorRegistration_TitleMain
                        s_fadeOverlayInitialized = 1;
                    }
                    Sing_805DC608() = (void*)s_fadeOverlay;
                }
                FadeState_StartFadeOut(Sing_805DC608(), 0);
                if (!Sing_805DC950()) {
                    if (!s_movieControllerInitialized) {
                        MovieController_Ctor(s_movieController); // MovieController_Ctor
                        __register_global_object(s_movieController, (void*)MovieController_Dtor, s_movieControllerDtorRegistration_TitleMain);
                        s_movieControllerInitialized = 1;
                    }
                    Sing_805DC950() = (void*)s_movieController;
                }
                MovieController_StopAndFreeSamples(Sing_805DC950());
                VISetBlack(1);
                if (!Sing_805DC948()) {
                    if (!s_systemResetStateInitialized) {
                        SystemResetState_Ctor(s_systemResetStorage); // SystemResetState_Ctor
                        __register_global_object(s_systemResetStorage, (void*)SystemResetState_Dtor, lbl_80511AB0);
                        s_systemResetStateInitialized = 1;
                    }
                    Sing_805DC948() = (void*)s_systemResetStorage;
                }
                SystemReset_RequestDeferredMode(Sing_805DC948(), 2);
            } else {
                {
                    nextMenu = new TitleTop;
                    AsCPangya2TitleMain(title)->activeMenu = nextMenu;
                    AsCPangya2TitleMain(title)->menuState = kTitleStartScreen;
                }
            }
            return;
        }
    }
    {
        if (g_soundManager) SoundManager_ApplySavedAudioSettings(g_soundManager, kFloat_1_TitleMainAudio); // kFloat_1_TitleMainAudio
        {
            void* p = AsCPangya2TitleMain(title)->activeMenu;
            if (p) {
                if (p) ((CObjBase*)p)->v2(1);
                AsCPangya2TitleMain(title)->activeMenu = 0;
            }
        }
        if (AsCPangya2TitleMain(title)->topMenuResult == 0) {
            nextMenu = new TopMenu;
            AsCPangya2TitleMain(title)->activeMenu = nextMenu;
            AsCPangya2TitleMain(title)->menuState = kTitleSingleMenu;
        } else {
            nextMenu = new MultiTop;
            AsCPangya2TitleMain(title)->activeMenu = nextMenu;
            AsCPangya2TitleMain(title)->menuState = kTitleMultiMenu;
        }
        return;
    }
    case kTitleSingleMenu: {
        int mode;
        if (CPangya2TopMenu_TryGetResult(AsCPangya2TitleMain(title)->activeMenu, &mode)) {        {
            void* p = AsCPangya2TitleMain(title)->activeMenu;
            if (p) {
                if (p) ((CObjBase*)p)->v2(1);
                AsCPangya2TitleMain(title)->activeMenu = 0;
            }
        }
        // Route the committed result; task name and initial queued command stay paired.
        switch (mode) {
        case kResultTour: {
            const char* s = modeTransitionStrings;
            CTask_RequestTransition(g_taskManager, s + kModeTransitionStrings_CTourTask, s + kModeTransitionStrings_emptyArgument);
            TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, s + kModeTransitionStrings_TourMain, 0, 0, 0, 0, 0);
            if (!Sing_805DC608()) {
                if (!s_fadeOverlayInitialized) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration_TitleMain); // s_fadeOverlayDtorRegistration_TitleMain
                    s_fadeOverlayInitialized = 1;
                }
                Sing_805DC608() = (void*)s_fadeOverlay;
            }
            FadeState_StartFadeIn(Sing_805DC608(), 0xf);
            break;
        }
        case kResultSingle0: {
            const char* s = modeTransitionStrings;
            CTask_RequestTransitionWithFade(g_taskManager, s + kModeTransitionStrings_CSingleTaskRev, s + kModeTransitionStrings_emptyArgument, 0);
            TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, s + kModeTransitionStrings_SingleMainRev, 0, 0, 0, 0, 0);
            break;
        }
        case kResultSingle1: {
            const char* s = modeTransitionStrings;
            CTask_RequestTransitionWithFade(g_taskManager, s + kModeTransitionStrings_CSingleTaskRev, s + kModeTransitionStrings_emptyArgument, 0);
            TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, s + kModeTransitionStrings_SingleMainRev, 1, 0, 0, 0, 0);
            break;
        }
        case kResultPractice:
            CPangya2TitleMain_StartPracticeSession(self, 0);
            break;
        case kResultAlternatePractice:
            CPangya2TitleMain_StartPracticeSession(self, 1);
            break;
        case kResultTraining: {
            const char* s = modeTransitionStrings;
            CTask_RequestTransitionWithFade(g_taskManager, s + kModeTransitionStrings_CTrainingTask, s + kModeTransitionStrings_emptyArgument, 0);
            break;
        }
        case kResultShop: {
            const char* s = modeTransitionStrings;
            CTask_RequestTransitionWithFade(g_taskManager, s + kModeTransitionStrings_CShop2Task, s + kModeTransitionStrings_emptyArgument, 0);
            TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, s + kModeTransitionStrings_Shop2, 1, 0, 0, 0, 0);
            break;
        }
        case kResultRoundDol: {
            const char* s = modeTransitionStrings;
            CTask_RequestTransitionWithFade(g_taskManager, s + kModeTransitionStrings_CRoundDolTask, s + kModeTransitionStrings_emptyArgument, 0);
            break;
        }
        case kResultBgmCollection: {
            const char* s = modeTransitionStrings;
            CTask_RequestTransitionWithFade(g_taskManager, s + kModeTransitionStrings_CBgmCollectionTask, s + kModeTransitionStrings_emptyArgument, 0);
            break;
        }
        case kResultPassword: {
            const char* s = modeTransitionStrings;
            CTask_RequestTransitionWithFade(g_taskManager, s + kModeTransitionStrings_CPasswordTask, s + kModeTransitionStrings_emptyArgument, 0);
            break;
        }
        default:
            break;
        }
        } else {
            {
                void* p = AsCPangya2TitleMain(title)->activeMenu;
                if (p) {
                    if (p) ((CObjBase*)p)->v2(1);
                    AsCPangya2TitleMain(title)->activeMenu = 0;
                }
            }
            nextMenu = new TitleTop;
            AsCPangya2TitleMain(title)->activeMenu = nextMenu;
            AsCPangya2TitleMain(title)->menuState = kTitleStartScreen;
        }
        return;
    }
    case kTitleMultiMenu: {
        int res;
        if (CMultiPlayTopMenu_TryGetSelection(AsCPangya2TitleMain(title)->activeMenu, &res)) {
        {
            void* p = AsCPangya2TitleMain(title)->activeMenu;
            if (p) {
                if (p) ((CObjBase*)p)->v2(1);
                AsCPangya2TitleMain(title)->activeMenu = 0;
            }
        }
        switch (res) {
        case kResultParty3: {
            const char* s = modeTransitionStrings;
            CTask_RequestTransition(g_taskManager, s + kModeTransitionStrings_CPartyTask, s + kModeTransitionStrings_CGolfDoc);
            TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, s + kModeTransitionStrings_PartyMain, 4, 3, 0, 0, 0);
            break;
        }
        case kResultParty1: {
            const char* s = modeTransitionStrings;
            CTask_RequestTransition(g_taskManager, s + kModeTransitionStrings_CPartyTask, s + kModeTransitionStrings_CGolfDoc);
            TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, s + kModeTransitionStrings_PartyMain, 4, 1, 0, 0, 0);
            break;
        }
        case kResultParty2: {
            const char* s = modeTransitionStrings;
            CTask_RequestTransition(g_taskManager, s + kModeTransitionStrings_CPartyTask, s + kModeTransitionStrings_CGolfDoc);
            TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, s + kModeTransitionStrings_PartyMain, 4, 2, 0, 0, 0);
            break;
        }
        case kResultFight4: {
            const char* s = modeTransitionStrings;
            CTask_RequestTransition(g_taskManager, s + kModeTransitionStrings_CFightTask, s + kModeTransitionStrings_CGolfDoc);
            TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, s + kModeTransitionStrings_FightMain, 4, 0, 0, 0, 0);
            break;
        }
        case kResultFight5: {
            const char* s = modeTransitionStrings;
            CTask_RequestTransition(g_taskManager, s + kModeTransitionStrings_CFightTask, s + kModeTransitionStrings_CGolfDoc);
            TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, s + kModeTransitionStrings_FightMain, 5, 0, 0, 0, 0);
            break;
        }
        default:
            break;
        }
        } else {
            {
                void* p = AsCPangya2TitleMain(title)->activeMenu;
                if (p) {
                    if (p) ((CObjBase*)p)->v2(1);
                    AsCPangya2TitleMain(title)->activeMenu = 0;
                }
            }
            nextMenu = new TitleTop;
            AsCPangya2TitleMain(title)->activeMenu = nextMenu;
            AsCPangya2TitleMain(title)->menuState = kTitleStartScreen;
        }
        return;
    }
    default:
        break;
    }
}

#pragma dont_inline on
#pragma opt_strength_reduction reset
#pragma inline_depth reset
#pragma inline_max_size reset
#pragma inline_max_total_size reset
#pragma always_inline reset
#pragma auto_inline reset
#pragma dont_inline off
#pragma always_inline on
struct WarningLogo {
    char storage[0x7c];
    WarningLogo() { __ct__12CWarningLogoFv(this); }
};
// Create the menu and optional submenu requested by a title transition message.
void CPangya2TitleMain_HandleMsg(void* self, const void* arg) {
    // Transition packet word +4 selects: 0 title, 5 warning, 3 multiplayer;
    // 1/6/8 plain single menu; 2 single+submenu 0; 4 single+submenu 2;
    // 7 single+submenu 3. This selector is NOT a TitleScreenState/result code.
    // Caller must have dealt with any old activeMenu: this overwrites its pointer.
    unsigned int transitionKind = *(const unsigned int*)((const char*)arg + 4);
    switch (transitionKind) {
    case 0: {
        void* nextMenu = new TitleTop;
        AsCPangya2TitleMain(self)->activeMenu = nextMenu;
        AsCPangya2TitleMain(self)->menuState = kTitleStartScreen;
        break;
    }
    case 1:
    case 6:
    case 8: {
        void* nextMenu = new TopMenu;
        AsCPangya2TitleMain(self)->activeMenu = nextMenu;
        AsCPangya2TitleMain(self)->menuState = kTitleSingleMenu;
        break;
    }
    case 5: {
        void* nextMenu = new WarningLogo;
        AsCPangya2TitleMain(self)->activeMenu = nextMenu;
        AsCPangya2TitleMain(self)->menuState = kTitleWarning;
        break;
    }
    case 2: {
        void* nextMenu = new TopMenu;
        AsCPangya2TitleMain(self)->activeMenu = nextMenu;
        AsCPangya2TitleMain(self)->menuState = kTitleSingleMenu;
        CPangya2TopMenu_RequestSubMenu(nextMenu, 0);
        break;
    }
    case 3: {
        void* nextMenu = new MultiTop;
        AsCPangya2TitleMain(self)->activeMenu = nextMenu;
        AsCPangya2TitleMain(self)->menuState = kTitleMultiMenu;
        break;
    }
    case 4: {
        void* nextMenu = new TopMenu;
        AsCPangya2TitleMain(self)->activeMenu = nextMenu;
        AsCPangya2TitleMain(self)->menuState = kTitleSingleMenu;
        CPangya2TopMenu_RequestSubMenu(nextMenu, 2);
        break;
    }
    case 7: {
        void* nextMenu = new TopMenu;
        AsCPangya2TitleMain(self)->activeMenu = nextMenu;
        AsCPangya2TitleMain(self)->menuState = kTitleSingleMenu;
        CPangya2TopMenu_RequestSubMenu(nextMenu, 3);
        break;
    }
    }
}
#pragma always_inline reset
#pragma dont_inline on

// Mark the requested submenu mode for activation.
int CPangya2TopMenu_RequestSubMenu(void* self, int subMenuMode) {
    // Deferred until the next process call; returns 1 without validating mode.
    // UpdateMenuState maps 0 -> (0,2), 2 -> (1,3), 3 -> (2,3).
    AsCPangya2TopMenu(self)->subMenuMode = subMenuMode;
    AsCPangya2TopMenu(self)->subMenuActive = 1;
    return 1;
}

#pragma dont_inline off
#pragma always_inline on
// Draw the title movie controller and the currently active menu.
void CPangya2TitleMain_OnDisplay(void* self) {
    if (!Sing_805DC950()) {
        if (!s_movieControllerInitialized) {
            MovieController_Ctor(s_movieController); // MovieController_Ctor
            __register_global_object(s_movieController, (void*)MovieController_Dtor, s_movieControllerDtorRegistration_TitleMain);
            s_movieControllerInitialized = 1;
        }
        Sing_805DC950() = (void*)s_movieController;
    }
    MovieController_InvokeNoOpHook(Sing_805DC950());
    if (!Sing_805DC950()) {
        if (!s_movieControllerInitialized) {
            MovieController_Ctor(s_movieController); // MovieController_Ctor
            __register_global_object(s_movieController, (void*)MovieController_Dtor, s_movieControllerDtorRegistration_TitleMain);
            s_movieControllerInitialized = 1;
        }
        Sing_805DC950() = (void*)s_movieController;
    }
    MoviePlayer_DrawCurrentSamples(Sing_805DC950());
    if (AsCPangya2TitleMain(self)->activeMenu)
        MPBase1_DrawIfInitialized(AsCPangya2TitleMain(self)->activeMenu);
}
#pragma always_inline reset
#pragma dont_inline reset

extern char __vt__15CPangya2TopMenu[];
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_TopMenu[]; // s_bpCtrlDtorRegistration_TopMenu
extern char s_bpCtrlHandleDtorRegistration_TopMenu[]; // s_bpCtrlHandleDtorRegistration_TopMenu
extern signed char s_bpCtrlInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];

void Layout_Destroy(void*, int);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void*);
void CBpCtrl_DeleteEntryById(void*, int);
void CBpCtrlHandle_Dtor(void*);
void CBpCtrlHandle_ReleaseEntry(void*);
void __dt__7MPBase2Fv(void*, int);

#pragma use_lmw_stmw on
#pragma push
#pragma dont_inline off
#pragma auto_inline off
// Destroy menu layouts and texture, delete script entries and release the script handle.
void* CPangya2TopMenu_dtor(void* self, int flag) {
    if (self) {
    void** vp = (void**)self;
    vp[0] = (void*)__vt__15CPangya2TopMenu;
    vp[6] = (void*)(__vt__15CPangya2TopMenu + 0x28);
    {
        char* p = (char*)self;
        int i;
        for (i = 0; i < kTopMenuLayoutCount; i++) {
            void* x = AsCPangya2TopMenu(p)->firstMenuLayout;
            if (x) {
                Layout_Destroy(x, 1);
                AsCPangya2TopMenu(p)->firstMenuLayout = 0;
            }
            if (AsCPangya2TopMenu(p)->firstMenuEntryId != -1) {
                if (!Sing_805DC5F0()) {
                    if (!s_bpCtrlInitialized) {
                        __ct__7CBpCtrlFv(s_bpCtrl);
                        __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_TopMenu); // s_bpCtrlDtorRegistration_TopMenu
                        s_bpCtrlInitialized = 1;
                    }
                    Sing_805DC5F0() = (void*)s_bpCtrl;
                }
                CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCPangya2TopMenu(p)->firstMenuEntryId);
                AsCPangya2TopMenu(p)->firstMenuEntryId = -1;
            }
            p += 4;
        }
    }
    {
        void* obj = AsCPangya2TopMenu(self)->menuTexture;
        if (obj) {
            if (obj) {
                ((CObjBase*)obj)->v2(1);
            }
            AsCPangya2TopMenu(self)->menuTexture = 0;
        }
    }
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_TopMenu); // s_bpCtrlHandleDtorRegistration_TopMenu
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = (void*)s_bpCtrlHandle;
    }
    CBpCtrlHandle_ReleaseEntry(Sing_805DC5F8());
    __dt__7MPBase2Fv((void*)((int*)self + 6), 0);
    if (flag > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop
#pragma use_lmw_stmw off

extern void MPBase1_Ctor(void*);
extern void CTopMenuBase_Ctor(void*, const char*, const char*, const char*, const char*, int, void*);
extern void HomeButtonMenu_InitializeState(void*);
extern void HomeButtonResources_dtor();
extern void HomeMenu_SetBlockedState(void*, int);
extern void HomeMenu_OverrideBlockedState(void*, int);
extern void Layout_Init(void*, int);
extern void Layout_LoadExtended(void*, int, const char*);
extern void CBpCtrl_CreateEntry(void*, void*, void**, int);
extern int IdMap_Lookup(void*, int);
extern void CBpTex_Init(void*);
extern void* __register_global_object(void*, void*, void*);
extern signed char s_homeMenuInitialized;
extern char __vt__13IFadeMenuBase[], topMenuResourceStrings[], s_topMenuBaseResourceArgument[], coursePreviewTextureNames[];
// Three corresponding layout-resource names and script-entry names for the top menu.
extern const char* s_topMenuLayoutNames[3];
extern const char* s_topMenuScriptNames[3];
extern char s_homeMenuDtorRegistration_TopMenu[]; // s_homeMenuDtorRegistration_TopMenu
extern char s_homeButtonResourceState[];
extern void* g_sharedDoc;
extern void* g_mouseCursor;
extern void* g_taskManager;
extern void* g_soundManager;
extern signed char s_saveDataInitialized;
extern char s_saveData[];
extern char modeTransitionStrings[];
extern void SaveData_Ctor(void*);
extern void* SaveData_GetCurrentRecord(void*);
extern void SharedDoc_SelectCourse(int);
extern char* strcpy(char*, const char*);
extern void __construct_array(void*, void*, void*, int, int);
extern void CCharacterPreviewRecord_FillFromSaveData(void*, int, void*);
extern void GolfPlayerList_BuildFromRenderTable(void*);
extern void CMouseCursor_SetPointerInputEnabled(void*, int);
extern void CTask_RequestTransitionWithFade(void*, const char*, const char*, int);
extern void SoundManager_RequestBgmStop(void*, int);
extern void CharacterPreviewTable_dtor(void*, int);
extern void PartyItemRecord_InitDefaults();
extern void PlayerRenderRecord_dtor();

#pragma dont_inline off
#pragma always_inline on
// Configure a one-hole practice round from the save, disable records and start the golf task.
void CPangya2TitleMain_StartPracticeSession(void* self, int alternatePractice) {
    // Phase 1: derive practice variant from save bits 31/30. Bit 31 selects
    // alternate game mode 2 vs 3; its unlock meaning is not established here.
    int savedPracticeMode, savedPlayerFlag;
    void* saveData;
    if (!Sing_805DC5E0()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        Sing_805DC5E0() = s_saveData;
    }
    unsigned int practiceFlags;
    {
        void* mgr = SaveData_GetCurrentRecord(Sing_805DC5E0());
        practiceFlags = ((PracticeSaveFlags*)mgr)->flags;
    }
    savedPracticeMode = practiceFlags >> 31;
    savedPlayerFlag = (practiceFlags >> 30) & 1;
    if (!alternatePractice && savedPracticeMode) { savedPracticeMode = 0; savedPlayerFlag = 0; }
    if (!alternatePractice) (*(int*)&AsCSharedDoc(g_sharedDoc)->gameMode) = kPracticeDefault;
    else if (savedPracticeMode) (*(int*)&AsCSharedDoc(g_sharedDoc)->gameMode) = kPracticeAlternateSavedVariant;
    else (*(int*)&AsCSharedDoc(g_sharedDoc)->gameMode) = kPracticeAlternateInitial;
    // Phase 2: configure the built-in FD course, one hole and disabled records.
    SharedDoc_SelectCourse(kPracticeCourseId);
    AsCSharedDoc(g_sharedDoc)->golfRuleType = kPracticeRuleType;
    strcpy(((PracticeSharedDocFields*)g_sharedDoc)->ruleName, (*(char**)&AsCSharedDoc(g_sharedDoc)->caddiePracticeName));
        int i = 0;
AsCSharedDoc(g_sharedDoc)->roundHoleCount = 1;
    for (; i < 3; ++i) {
        unsigned char off = i * 6;
        for (int j = 0; j < 6; ++j) {
            ((PracticeSharedDocFields*)g_sharedDoc)->playerFlags[off + j] = 0;
            ((PracticeSharedDocFields*)g_sharedDoc)->courseIds[off + j] = 0;
        }
    }
    ((PracticeSharedDocFields*)g_sharedDoc)->playerFlags[0] = 1;
    ((PracticeSharedDocFields*)g_sharedDoc)->courseIds[0] = kPracticeCourseId;
    (*(int*)&AsCSharedDoc(g_sharedDoc)->turnTimeLimitMs) = 0;
    ((PracticeSharedDocFields*)g_sharedDoc)->roundOption = 0;
    AsCSharedDoc(g_sharedDoc)->courseRecordUpdatesEnabled = 0;
    // Phase 3: materialize one saved player in a temporary four-record table;
    // BuildFromRenderTable consumes its values before the temporary is destroyed.
    unsigned char local[0x298];
    __construct_array(local, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4);
    ((CEnemyRenderTable*)local)->count = 1; // 0x802BE544: four-record table +0x290
    EH5Cleanup eh5Cleanup16(local, (void (*)(void*, int))CharacterPreviewTable_dtor);
    if (!Sing_805DC5E0()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        Sing_805DC5E0() = s_saveData;
    }
    saveData = Sing_805DC5E0();
    if (!saveData) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        Sing_805DC5E0() = s_saveData;
        saveData = s_saveData;
    }
    void* mgr2 = SaveData_GetCurrentRecord(saveData);
    void* other = SaveData_GetCurrentRecord(saveData);
    CCharacterPreviewRecord_FillFromSaveData(local, AsSavePlayerRecord(other)->characterIndex, mgr2);
    ((CEnemyRenderRecord*)local)->caddieInventoryIndex = 1; // 0x802BE544: first record +0xC
    ((CEnemyRenderRecord*)local)->caddieItemInventoryIndex = 7;
    if (savedPracticeMode) {
        unsigned short bits = ((CEnemyRenderRecord*)local)->flags;
        bits |= 0x2000;
        ((CEnemyRenderRecord*)local)->flags = bits;
    } else {
        unsigned short bits = ((CEnemyRenderRecord*)local)->flags;
        bits = bits & ~0x3000;
        bits |= savedPlayerFlag << 12;
        ((CEnemyRenderRecord*)local)->flags = bits;
    }
    GolfPlayerList_BuildFromRenderTable(local);
    // Phase 4: switch input/task ownership to golf, then release table resources.
    ((CMouseCursor*)g_mouseCursor)->hitTestMode = 1;
    CMouseCursor_SetPointerInputEnabled(g_mouseCursor, 0);
    CTask_RequestTransitionWithFade(g_taskManager, modeTransitionStrings + kModeTransitionStrings_CGolfTask, modeTransitionStrings + kModeTransitionStrings_CGolfDoc, 0);
    SoundManager_RequestBgmStop(g_soundManager, 1);
    { eh5Cleanup16.release(); CharacterPreviewTable_dtor(local, -1); }
}
#pragma always_inline reset
#pragma dont_inline reset

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
#pragma dont_inline off
#pragma always_inline on
struct TopLayout {
    char storage[0x5c];
    TopLayout() { Layout_Init(this, 5); }
};
struct TopTexture {
    char storage[0x28];
    TopTexture() { CBpTex_Init(this); }
};
class TopTextureVirtual {
public:
    virtual void v0();
    virtual void load(void*);
};
inline void* GetTopMenuBp(void*& cachedGlobal) {
    if (!cachedGlobal) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_TopMenu);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
        cachedGlobal = s_bpCtrl;
    }
    return Sing_805DC5F0();
}
// Construct the top-menu base, its three layouts/script entries and background texture.
void* CPangya2TopMenu_Ctor(void* argument) {
    void* obj;
    void* cachedGlobal;
    CPangya2TopMenu* self;
    int i;
    const char** scriptNames;
    const char** layoutNames;
    int invalidEntry;
    int emptyValue;
    int offset;
    self = (CPangya2TopMenu*)argument;
    // Phase 1: construct both bases and set the Home-menu override.
    MPBase1_Ctor(self);
    *(void**)self = __vt__13IFadeMenuBase;
    CTopMenuBase_Ctor((char*)&AsCPangya2TopMenu(self)->secondaryVtable, topMenuResourceStrings, topMenuResourceStrings + kTopMenuResourceStrings_top_menu_srd,
                topMenuResourceStrings + kTopMenuResourceStrings_top_menu_b2d, topMenuResourceStrings + kTopMenuResourceStrings_topmenu_zip, 5, s_topMenuBaseResourceArgument);
    *(void**)self = __vt__15CPangya2TopMenu;
    AsCPangya2TopMenu(self)->secondaryVtable = __vt__15CPangya2TopMenu + 0x28;
    (*(signed char*)&AsCPangya2TopMenu(self)->subMenuActive) = 0;
    (*(int*)&AsCPangya2TopMenu(self)->subMenu) = 0;
    AsCPangya2TopMenu(self)->subMenuMode = -1;
    AsCPangya2TopMenu(self)->menuTexture = 0;
    if (!Sing_805DC820()) {
        if (!s_homeMenuInitialized) {
            HomeButtonMenu_InitializeState(s_homeButtonResourceState);
            __register_global_object(s_homeButtonResourceState, (void*)HomeButtonResources_dtor, s_homeMenuDtorRegistration_TopMenu);
            s_homeMenuInitialized = 1;
        }
        Sing_805DC820() = s_homeButtonResourceState;
    }
    HomeMenu_SetBlockedState(Sing_805DC820(), 0);
    if (!Sing_805DC820()) {
        if (!s_homeMenuInitialized) {
            HomeButtonMenu_InitializeState(s_homeButtonResourceState);
            __register_global_object(s_homeButtonResourceState, (void*)HomeButtonResources_dtor, s_homeMenuDtorRegistration_TopMenu);
            s_homeMenuInitialized = 1;
        }
        Sing_805DC820() = s_homeButtonResourceState;
    }
    HomeMenu_OverrideBlockedState(Sing_805DC820(), 1);
    layoutNames = s_topMenuLayoutNames;
    scriptNames = s_topMenuScriptNames;
    i = 0;
    offset = 0;
    emptyValue = 0;
    invalidEntry = -1;
    cachedGlobal = Sing_805DC5F0();
    while (i < kTopMenuLayoutCount) {
        AsCPangya2TopMenu((char*)self + offset)->menuLayouts[0] = (void*)emptyValue;
        AsCPangya2TopMenu((char*)self + offset)->menuObjects[0] = (void*)emptyValue;
        AsCPangya2TopMenu((char*)self + offset)->menuEntryIds[0] = invalidEntry;
        obj = new TopLayout;
        AsCPangya2TopMenu((char*)self + offset)->menuLayouts[0] = obj;
        Layout_LoadExtended(obj, 0, *layoutNames);
        void* bp = GetTopMenuBp(cachedGlobal);
        void** entryId = (void**)&AsCPangya2TopMenu((char*)self + offset)->menuEntryIds[0];
        CBpCtrl_CreateEntry(bp, (void*)*scriptNames, entryId, 0);
        (*(int*)&AsCPangya2TopMenu((char*)self + offset)->menuObjects[0]) = IdMap_Lookup(GetTopMenuBp(cachedGlobal), AsCPangya2TopMenu((char*)self + offset)->menuEntryIds[0]);
        ++i;
        offset += 4;
        ++layoutNames;
        ++scriptNames;
    }
    {
        // Phase 3: create the menu texture chosen by SharedDoc::menuTextureIndex.
        obj = new TopTexture;
        AsCPangya2TopMenu(self)->menuTexture = obj;
        ((TopTextureVirtual*)obj)->load(
            ((void**)coursePreviewTextureNames)[(*(int*)&AsCSharedDoc(g_sharedDoc)->menuTextureIndex)]);
    }
    return self;
}
#pragma always_inline reset
#pragma dont_inline reset
#pragma use_lmw_stmw off
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

extern void CTopMenuBase_SyncItemPosition(void*, int);
extern int BpObject_IsCurrentScriptComplete(void*);
extern void COptionDialog_Ctor(void*, int);
extern int MPBase1_Process(void*);
extern void CTopSubMenu_Ctor(void*, int, int, int, int);
extern void BpObject_SetEnabled(void*, int);
extern void MenuMessage_ResetTextState(void*);
extern void CTopSubMenu_TryGetResult(void*, int*);
extern void CPangya2TopMenu_ActivateMenu(void*);
extern void Layout_Update(void*);
extern void CTopMenuBase_UpdateHoverMessage(void*, char*);
extern void CTopMenuBase_UpdateHoverElementPairs(void*);
extern void CTopMenuBase_ProcessInput(void*);
extern void BpObject_TryInvokeCurrentCallback(void*, int);
extern void MenuMessage_UpdateReveal(void*);
extern char s_topMenuHoverMessages[];

#pragma dont_inline off
#pragma always_inline on
struct TopOptionDialog {
    char storage[0x860];
    // Construct an options dialog with the caller's dialog mode.
    TopOptionDialog(int mode) { COptionDialog_Ctor(this, mode); }
};
struct TopSubMenu {
    char storage[0x64];
    // Borrow the selected layout and companion word; the top menu owns this submenu.
    TopSubMenu(void* owner, int layoutIndex, int companionOffset) {
        CTopSubMenu_Ctor(this, (*(int*)&AsCPangya2TopMenu(owner)->menuLayouts[layoutIndex]), (*(int*)((char*)&AsCPangya2TopMenu(owner)->menuLayouts[1] + companionOffset * 4)), AsCPangya2TopMenu(owner)->subMenuMode, companionOffset);
    }
};
#pragma always_inline reset

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
// Update fades, option/submenu processing and main-menu selection each frame.
int CPangya2TopMenu_UpdateMenuState(void* self, float dt) {
    // Priority: running script, requested exit, options, submenu, normal input.
    // Return 1 means this menu finished; resultReady separately marks selection.
    // Item IDs 0..19 synchronize with the base script. Layout/highlight element
    // IDs live in that base/resources and are not defined by this translation unit.
    void* submenu;
    int companionOffset;
    int layoutIndex;
    int i;
    int result = 0;
    if (AsCPangya2TopMenu(self)->scriptRunning) {
        for (i = 0; i < kTopMenuItemCount; ++i) CTopMenuBase_SyncItemPosition((char*)&AsCPangya2TopMenu(self)->secondaryVtable, i);
        int _nz = BpObject_IsCurrentScriptComplete(AsCPangya2TopMenu(self)->bpObject);
        (*(bool*)&AsCPangya2TopMenu(self)->scriptRunning) = (_nz == 0);
    } else if (AsCPangya2TopMenu(self)->exitRequested) {
        result = 1;
    } else if (AsCPangya2TopMenu(self)->optionDialogActive) {
        if (!AsCPangya2TopMenu(self)->optionDialog) {
            void* p = new TopOptionDialog(1);
            AsCPangya2TopMenu(self)->optionDialog = p;
        }
        if (((int(*)(void*, float))MPBase1_Process)(AsCPangya2TopMenu(self)->optionDialog, dt)) {
            void* p = AsCPangya2TopMenu(self)->optionDialog;
            if (p) {
                if (p) ((CObjBase*)p)->v2(1);
                AsCPangya2TopMenu(self)->optionDialog = 0;
            }
            AsCPangya2TopMenu(self)->optionDialogActive = 0;
        }
    } else if (AsCPangya2TopMenu(self)->subMenuActive) {
        if (!AsCPangya2TopMenu(self)->subMenu) {
            int mode = AsCPangya2TopMenu(self)->subMenuMode;
            // mode -> layout index and companion offset. Mode 1 gives offset 4
            // but index -1; caller must not request it as a supported submenu here.
            switch (mode) {
            case 0: companionOffset = 2; break;
            case 1: companionOffset = 4; break;
            case 2: companionOffset = 3; break;
            case 3: companionOffset = 3; break;
            default: companionOffset = -1; break;
            }
            switch (mode) {
            case 0: layoutIndex = 0; break;
            case 2: layoutIndex = 1; break;
            case 3: layoutIndex = 2; break;
            default: layoutIndex = -1; break;
            }
            submenu = new TopSubMenu(self, layoutIndex, companionOffset);
            AsCPangya2TopMenu(self)->subMenu = submenu;
            BpObject_SetEnabled(AsCPangya2TopMenu(self)->bpObject, 0);
            MenuMessage_ResetTextState(AsCPangya2TopMenu(self)->message);
        }
        if (((int(*)(void*, float))MPBase1_Process)(AsCPangya2TopMenu(self)->subMenu, dt)) {
            CTopSubMenu_TryGetResult(AsCPangya2TopMenu(self)->subMenu, &AsCPangya2TopMenu(self)->result);
            void* p = AsCPangya2TopMenu(self)->subMenu;
            if (p) {
                if (p) ((CObjBase*)p)->v2(1);
                AsCPangya2TopMenu(self)->subMenu = 0;
            }
            if (AsCPangya2TopMenu(self)->result == kResultCancelled) {
                CPangya2TopMenu_ActivateMenu(self);
                AsCPangya2TopMenu(self)->subMenuActive = 0;
            } else {
                AsCPangya2TopMenu(self)->resultReady = 1;
                result = 1;
            }
        }
    } else {
        Layout_Update(AsCPangya2TopMenu(self)->layout);
        CTopMenuBase_UpdateHoverMessage((char*)&AsCPangya2TopMenu(self)->secondaryVtable, s_topMenuHoverMessages);
        CTopMenuBase_UpdateHoverElementPairs((char*)&AsCPangya2TopMenu(self)->secondaryVtable);
        CTopMenuBase_ProcessInput((char*)&AsCPangya2TopMenu(self)->secondaryVtable);
    }
    BpObject_TryInvokeCurrentCallback(AsCPangya2TopMenu(self)->bpObject, 0);
    MenuMessage_UpdateReveal(AsCPangya2TopMenu(self)->message);
    return result;
}
#pragma use_lmw_stmw off
#pragma dont_inline reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804624E4", _ctors_u802BD994_0, TitleMain_InitAngleWrapConstant); // TitleMain_InitAngleWrapConstant
#pragma pop
