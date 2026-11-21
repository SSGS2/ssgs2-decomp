#include "classes/CPartyMain.h"
#include "classes/CTutorial.h"
#define AsCTutorial(p) ((CTutorial*)(p))
// Translation unit 0x80216460-0x80216D5C (28 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
extern "C" {
void CSpot_Ctor(void*);
void CGuideLine_Ctor(void*);
void CFightTask_Ctor(void*);
void CFightMain_Ctor(void*);
void CPartyTask_Ctor(void*);
void CBalloonSystem_ctor(void*);
void CDartsSystem_Ctor(void*);
void CDriveSystem_Ctor(void*);
void CStoryConte_ctor(void*);
void CTourTask_Ctor(void*);
void CTourMain_Ctor(void*);
void CScreen_Ctor(void*);
void CTrainingTask_Ctor(void*);
void CTrainingMain_Ctor(void*);
void CSingleTaskRev_Ctor(void*);
void CSingleMainRev_Ctor(void*);
void __ct__19NativeOwner80201D18Fv(void*);
void CSaveProcessTask_Ctor(void*);
void CSaveProcessMain_Ctor(void*);
void CRoundDolTask_Ctor(void*);
void CRoundDolMain_Ctor(void*);
void CBgmCollectionTask_Ctor(void*);
void CBgmCollectionMain_Ctor(void*);
void CPasswordTask_Ctor(void*);
void CPasswordMain_Ctor(void*);
void CVSMain_Ctor(void*);
void __ct__18NativeBase80201D18Fv(void*);
void CCheckDriveStatus_SetSkipDialogDelay(int);
void gtl_InputEx_Update();
void* InputManager_Init(void*);
void InputManager_Dtor(void*);
void __register_global_object(void*, void (*)(void*), const char*);
void InputManager_Update(void*);
void Engine_ClearFramebuffer(int, int, int, float, int, int);
void CNowReloading_Init(void*);
void CNowReloading_dtor(void*);
void B2dPlayer_UpdateAndDraw(void*);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void*);
void CBpCtrl_InvokeEntryCallbacks(void*, int);
void CBpCtrl_DrawEntries(void*);
void CBpCtrl_ResetEntryFrameState(void*);
void HomeButtonMenu_InitializeState(void*);
void HomeButtonResources_dtor(void*);
void HomeButtonMenu_ProcessState(void*);
void Engine_PresentFrame(int, int, int, int);
extern signed char s_inputManagerInitialized;
extern signed char s_nowReloadingInitialized;
extern char s_nowReloading;
extern signed char s_bpCtrlInitialized;
extern signed char s_homeMenuInitialized;
extern char gameStartupDtorRegistrations[];
extern char s_gameInputManager[];
extern char s_bpCtrl[];
extern char s_homeButtonResourceState[];
extern float kFloat_1_FramebufferClear;
extern char __vt__10CPartyMain[];
extern char __vt__9CTutorial[];

} // extern "C"

struct Obj80216460 {
    char storage[0x348];
    Obj80216460() { CSpot_Ctor(this); }
};

struct Obj802164A0 {
    char storage[0xDAC];
    Obj802164A0() { CGuideLine_Ctor(this); }
};

struct Obj80216570 {
    char storage[0x3A8];
    Obj80216570() { CFightTask_Ctor(this); }
};

struct Obj802165B0 {
    char storage[0x4C];
    Obj802165B0() { CFightMain_Ctor(this); }
};

struct Obj802165F0 {
    char storage[0x3A8];
    Obj802165F0() { CPartyTask_Ctor(this); }
};

struct Obj80216684 {
    char storage[0x2C];
    Obj80216684() { CBalloonSystem_ctor(this); }
};

struct Obj802166C4 {
    char storage[0x94];
    Obj802166C4() { CDartsSystem_Ctor(this); }
};

struct Obj80216704 {
    char storage[0x24];
    Obj80216704() { CDriveSystem_Ctor(this); }
};

struct Obj80216744 {
    char storage[0xA0];
    Obj80216744() { CStoryConte_ctor(this); }
};

struct Obj80216784 {
    char storage[0x3A8];
    Obj80216784() { CTourTask_Ctor(this); }
};

struct Obj802167C4 {
    char storage[0x860];
    Obj802167C4() { CTourMain_Ctor(this); }
};

struct Obj80216804 {
    char storage[0x3C0];
    Obj80216804() { CScreen_Ctor(this); }
};

struct Obj80216844 {
    char storage[0x3A8];
    Obj80216844() { CTrainingTask_Ctor(this); }
};

struct Obj80216884 {
    char storage[0x2C];
    Obj80216884() { CTrainingMain_Ctor(this); }
};

struct Obj802168C4 {
    char storage[0x3A8];
    Obj802168C4() { CSingleTaskRev_Ctor(this); }
};

struct Obj80216904 {
    char storage[0x24];
    Obj80216904() { CSingleMainRev_Ctor(this); }
};

struct Obj80216944 {
    char storage[0x50];
    Obj80216944() { __ct__19NativeOwner80201D18Fv(this); }
};

struct Obj80216984 {
    char storage[0x3A8];
    Obj80216984() { CSaveProcessTask_Ctor(this); }
};

struct Obj802169C4 {
    char storage[0x40];
    Obj802169C4() { CSaveProcessMain_Ctor(this); }
};

struct Obj80216A04 {
    char storage[0x3A8];
    Obj80216A04() { CRoundDolTask_Ctor(this); }
};

struct Obj80216A44 {
    char storage[0x20];
    Obj80216A44() { CRoundDolMain_Ctor(this); }
};

struct Obj80216A84 {
    char storage[0x3A8];
    Obj80216A84() { CBgmCollectionTask_Ctor(this); }
};

struct Obj80216AC4 {
    char storage[0x20];
    Obj80216AC4() { CBgmCollectionMain_Ctor(this); }
};

struct Obj80216B04 {
    char storage[0x3A8];
    Obj80216B04() { CPasswordTask_Ctor(this); }
};

struct Obj80216B44 {
    char storage[0x30];
    Obj80216B44() { CPasswordMain_Ctor(this); }
};

// Construction view of CPartyMain; canonical members: classes/CPartyMain.h.
// Factory allocates 0x4c bytes; shared header describes only the first 0x2c.
struct CPartyMainConstructorView {
    char storage[0x4C];
    CPartyMainConstructorView() {
        CVSMain_Ctor(this);
        *(void**)storage = __vt__10CPartyMain;
        *(void**)(storage + 0x18) = __vt__10CPartyMain + 0x3C;
    }
};

inline CTutorial::CTutorial() {
    __ct__18NativeBase80201D18Fv(this);
    vtbl = __vt__9CTutorial;
    secondaryVtable = __vt__9CTutorial + 0x3C;
    *(void**)&commandReadOffset = 0;
    *(void**)&pendingCommandCount = 0;
    commandBlocks = 0;
    *(void**)&firstCommandBlockIndex = 0;
    *(void**)&commandBlockCount = 0;
    *(void**)&commandBlockCapacity = 0;
    forcedActionIndex = -1;
    textMapCount = 0;
    textMapRoot = 0;
    textMapLast = &textMapRoot;
    messageDialog = 0;
    modeMessageDialog = 0;
}

extern "C" {
Obj80216460* CSpot_Create() { return new Obj80216460; }
Obj802164A0* CGuideLine_Create() { return new Obj802164A0; }
Obj80216570* CFightTask_Create() { return new Obj80216570; }
Obj802165B0* CFightMain_Create() { return new Obj802165B0; }
Obj802165F0* CPartyTask_Create() { return new Obj802165F0; }
Obj80216684* CBalloonSystem_Create() { return new Obj80216684; }
Obj802166C4* CDartsSystem_Create() { return new Obj802166C4; }
Obj80216704* CDriveSystem_Create() { return new Obj80216704; }
Obj80216744* CStoryConte_Create() { return new Obj80216744; }
Obj80216784* CTourTask_Create() { return new Obj80216784; }
Obj802167C4* CTourMain_Create() { return new Obj802167C4; }
Obj80216804* CScreen_Create() { return new Obj80216804; }
Obj80216844* CTrainingTask_Create() { return new Obj80216844; }
Obj80216884* CTrainingMain_Create() { return new Obj80216884; }
Obj802168C4* CSingleTaskRev_Create() { return new Obj802168C4; }
Obj80216904* CSingleMainRev_Create() { return new Obj80216904; }
Obj80216944* CTrajectory_Create() { return new Obj80216944; }
Obj80216984* CSaveProcessTask_Create() { return new Obj80216984; }
Obj802169C4* CSaveProcessMain_Create() { return new Obj802169C4; }
Obj80216A04* CRoundDolTask_Create() { return new Obj80216A04; }
Obj80216A44* CRoundDolMain_Create() { return new Obj80216A44; }
Obj80216A84* CBgmCollectionTask_Create() { return new Obj80216A84; }
Obj80216AC4* CBgmCollectionMain_Create() { return new Obj80216AC4; }
Obj80216B04* CPasswordTask_Create() { return new Obj80216B04; }
Obj80216B44* CPasswordMain_Create() { return new Obj80216B44; }
CPartyMainConstructorView* CPartyMain_Create() { return new CPartyMainConstructorView; }
CTutorial* CTutorial_Create() { return new CTutorial; }

void Game_DrawReloadingFrame() {
    char* _g = gameStartupDtorRegistrations;
    CCheckDriveStatus_SetSkipDialogDelay(0);
    gtl_InputEx_Update();
    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) {
            char* p = s_gameInputManager;
            InputManager_Init(p);
            __register_global_object(p, InputManager_Dtor, _g + 0x24);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = s_gameInputManager;
    }
    InputManager_Update(Sing_805DC5C8());
    Engine_ClearFramebuffer(0, 0, 7, kFloat_1_FramebufferClear, 0, 0);
    if (!Sing_805DC610()) {
        if (!s_nowReloadingInitialized) {
            CNowReloading_Init(&s_nowReloading);
            __register_global_object(&s_nowReloading, CNowReloading_dtor, _g + 0x30);
            s_nowReloadingInitialized = 1;
        }
        Sing_805DC610() = &s_nowReloading;
    }
    B2dPlayer_UpdateAndDraw(Sing_805DC610());
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            char* p = s_bpCtrl;
            __ct__7CBpCtrlFv(p);
            __register_global_object(p, CBpCtrl_dtor, _g + 0x18);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* r30 = Sing_805DC5F0();
    CBpCtrl_InvokeEntryCallbacks(r30, 0);
    CBpCtrl_DrawEntries(r30);
    CBpCtrl_ResetEntryFrameState(r30);
    if (!Sing_805DC820()) {
        if (!s_homeMenuInitialized) {
            char* p = s_homeButtonResourceState;
            HomeButtonMenu_InitializeState(p);
            __register_global_object(p, HomeButtonResources_dtor, _g + 0x0C);
            s_homeMenuInitialized = 1;
        }
        Sing_805DC820() = s_homeButtonResourceState;
    }
    HomeButtonMenu_ProcessState(Sing_805DC820());
    CCheckDriveStatus_SetSkipDialogDelay(1);
    Engine_PresentFrame(0, 0, 0, 0);
}
}
