// Task factories for the game modes: each CXxx_Create() function builds one mode's task or
// main object into caller storage, and Game_DrawReloadingFrame draws the reload screen.

#include "classes/CPartyMain.h"
#include "classes/CPartyTask.h"
#include "classes/CTourTask.h"
#include "classes/CTutorial.h"
#include "classes/CBalloonSystem.h"
#define AsCTutorial(p) ((CTutorial*)(p))

#include "game/singletons.h"
#include "classes/CGuideLine.h"
#include "classes/CFightMain.h"
#include "classes/CFightTask.h"
#include "classes/CNowReloading.h"
#include "classes/CScreen.h"
#include "classes/CBpCtrl.h"
#include "classes/CCheckDriveStatus.h"
#include "classes/CBgmCollectionMain.h"
#include "classes/CBgmCollectionTask.h"
#include "classes/CDartsSystem.h"
#include "classes/CDriveSystem.h"
#include "classes/CPasswordMain.h"
#include "classes/CPasswordTask.h"
#include "classes/CRoundDolMain.h"
#include "classes/CRoundDolTask.h"
#include "classes/CSaveProcessMain.h"
#include "classes/CSaveProcessTask.h"
#include "classes/CSingleMainRev.h"
#include "classes/CSingleTaskRev.h"
#include "classes/CSpot.h"
#include "classes/CStoryConte.h"
#include "classes/CTourMain.h"
#include "classes/CTrainingMain.h"
#include "classes/CTrainingTask.h"
#include "classes/CVSMain.h"

#pragma use_simple_stdpnew on
inline void* operator new(unsigned long, void* storage) {
    return storage;
}

extern "C" {

void __ct__19NativeOwner80201D18Fv(void*);
void __ct__18NativeBase80201D18Fv(void*);
void gtl_InputEx_Update();
void* InputManager_Init(void*);
void InputManager_Dtor(void*);
void __register_global_object(void*, void (*)(void*), const char*);
void InputManager_Update(void*);
void Engine_ClearFramebuffer(int, int, int, float, int, int);
void __dt__13CNowReloadingFv(void*);
void B2dPlayer_UpdateAndDraw(void*);
void CBpCtrl_dtor(void*);
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

struct CSpotStorage {
    char storage[0x348];
    CSpotStorage() {
        ((CSpot*)this)->__ct();
    }
};

struct CGuideLineStorage {
    char storage[0xDAC];
    CGuideLineStorage() {
        new (this) CGuideLine;
    }
};

struct CFightTaskStorage {
    char storage[0x3A8];
    CFightTaskStorage() {
        new (this) CFightTask;
    }
};

struct CFightMainStorage {
    char storage[0x4C];
    CFightMainStorage() {
        new (this) CFightMain;
    }
};

struct CPartyTaskStorage {
    char storage[0x3A8];
    CPartyTaskStorage() {
        new (this) CPartyTask;
    }
};

struct CBalloonSystemStorage {
    char storage[0x2C];
    CBalloonSystemStorage() {
        ((CBalloonSystem*)this)->Init();
    }
};

struct CDartsSystemStorage {
    char storage[0x94];
    CDartsSystemStorage() {
        ((CDartsSystem*)this)->__ct();
    }
};

struct CDriveSystemStorage {
    char storage[0x24];
    CDriveSystemStorage() {
        ((CDriveSystem*)this)->__ct();
    }
};

struct CStoryConteStorage {
    char storage[0xA0];
    CStoryConteStorage() {
        ((CStoryConte*)this)->__ct();
    }
};

struct CTourTaskStorage {
    char storage[0x3A8];
    CTourTaskStorage() {
        new (this) CTourTask;
    }
};

struct CTourMainStorage {
    char storage[0x860];
    CTourMainStorage() {
        ((CTourMain*)this)->__ct();
    }
};

struct CScreenStorage {
    char storage[0x3C0];
    CScreenStorage() {
        new (this) CScreen;
    }
};

struct CTrainingTaskStorage {
    char storage[0x3A8];
    CTrainingTaskStorage() {
        ((CTrainingTask*)this)->__ct();
    }
};

struct CTrainingMainStorage {
    char storage[0x2C];
    CTrainingMainStorage() {
        ((CTrainingMain*)this)->__ct();
    }
};

struct CSingleTaskRevStorage {
    char storage[0x3A8];
    CSingleTaskRevStorage() {
        ((CSingleTaskRev*)this)->__ct();
    }
};

struct CSingleMainRevStorage {
    char storage[0x24];
    CSingleMainRevStorage() {
        ((CSingleMainRev*)this)->__ct();
    }
};

struct Obj80216944 {
    char storage[0x50];
    Obj80216944() {
        __ct__19NativeOwner80201D18Fv(this);
    }
};

struct CSaveProcessTaskStorage {
    char storage[0x3A8];
    CSaveProcessTaskStorage() {
        ((CSaveProcessTask*)this)->__ct();
    }
};

struct CSaveProcessMainStorage {
    char storage[0x40];
    CSaveProcessMainStorage() {
        ((CSaveProcessMain*)this)->__ct();
    }
};

struct CRoundDolTaskStorage {
    char storage[0x3A8];
    CRoundDolTaskStorage() {
        ((CRoundDolTask*)this)->__ct();
    }
};

struct CRoundDolMainStorage {
    char storage[0x20];
    CRoundDolMainStorage() {
        ((CRoundDolMain*)this)->__ct();
    }
};

struct CBgmCollectionTaskStorage {
    char storage[0x3A8];
    CBgmCollectionTaskStorage() {
        ((CBgmCollectionTask*)this)->__ct();
    }
};

struct CBgmCollectionMainStorage {
    char storage[0x20];
    CBgmCollectionMainStorage() {
        ((CBgmCollectionMain*)this)->__ct();
    }
};

struct CPasswordTaskStorage {
    char storage[0x3A8];
    CPasswordTaskStorage() {
        ((CPasswordTask*)this)->__ct();
    }
};

struct CPasswordMainStorage {
    char storage[0x30];
    CPasswordMainStorage() {
        ((CPasswordMain*)this)->__ct();
    }
};

// Construction view of CPartyMain; canonical members: classes/CPartyMain.h.
// Factory allocates 0x4c bytes; shared header describes only the first 0x2c.
struct CPartyMainConstructorView {
    char storage[0x4C];
    CPartyMainConstructorView() {
        ((CVSMain*)this)->__ct();
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
CSpotStorage* CSpot_Create() {
    return new CSpotStorage;
}
CGuideLineStorage* CGuideLine_Create() {
    return new CGuideLineStorage;
}
CFightTaskStorage* CFightTask_Create() {
    return new CFightTaskStorage;
}
CFightMainStorage* CFightMain_Create() {
    return new CFightMainStorage;
}
CPartyTaskStorage* CPartyTask_Create() {
    return new CPartyTaskStorage;
}
CBalloonSystemStorage* CBalloonSystem_Create() {
    return new CBalloonSystemStorage;
}
CDartsSystemStorage* CDartsSystem_Create() {
    return new CDartsSystemStorage;
}
CDriveSystemStorage* CDriveSystem_Create() {
    return new CDriveSystemStorage;
}
CStoryConteStorage* CStoryConte_Create() {
    return new CStoryConteStorage;
}
CTourTaskStorage* CTourTask_Create() {
    return new CTourTaskStorage;
}
CTourMainStorage* CTourMain_Create() {
    return new CTourMainStorage;
}
CScreenStorage* CScreen_Create() {
    return new CScreenStorage;
}
CTrainingTaskStorage* CTrainingTask_Create() {
    return new CTrainingTaskStorage;
}
CTrainingMainStorage* CTrainingMain_Create() {
    return new CTrainingMainStorage;
}
CSingleTaskRevStorage* CSingleTaskRev_Create() {
    return new CSingleTaskRevStorage;
}
CSingleMainRevStorage* CSingleMainRev_Create() {
    return new CSingleMainRevStorage;
}
Obj80216944* CTrajectory_Create() {
    return new Obj80216944;
}
CSaveProcessTaskStorage* CSaveProcessTask_Create() {
    return new CSaveProcessTaskStorage;
}
CSaveProcessMainStorage* CSaveProcessMain_Create() {
    return new CSaveProcessMainStorage;
}
CRoundDolTaskStorage* CRoundDolTask_Create() {
    return new CRoundDolTaskStorage;
}
CRoundDolMainStorage* CRoundDolMain_Create() {
    return new CRoundDolMainStorage;
}
CBgmCollectionTaskStorage* CBgmCollectionTask_Create() {
    return new CBgmCollectionTaskStorage;
}
CBgmCollectionMainStorage* CBgmCollectionMain_Create() {
    return new CBgmCollectionMainStorage;
}
CPasswordTaskStorage* CPasswordTask_Create() {
    return new CPasswordTaskStorage;
}
CPasswordMainStorage* CPasswordMain_Create() {
    return new CPasswordMainStorage;
}
CPartyMainConstructorView* CPartyMain_Create() {
    return new CPartyMainConstructorView;
}
CTutorial* CTutorial_Create() {
    return new CTutorial;
}

void Game_DrawReloadingFrame() {
    char* _g = gameStartupDtorRegistrations;
    CCheckDriveStatus::SetSkipDialogDelay(0);
    gtl_InputEx_Update();
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            char* p = s_gameInputManager;
            InputManager_Init(p);
            __register_global_object(p, InputManager_Dtor, _g + 0x24);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    InputManager_Update(GetInputManagerSlot());
    Engine_ClearFramebuffer(0, 0, 7, kFloat_1_FramebufferClear, 0, 0);
    if (!Sing_805DC610()) {
        if (!s_nowReloadingInitialized) {
            ((CNowReloading*)(&s_nowReloading))->Init();
            __register_global_object(&s_nowReloading, __dt__13CNowReloadingFv, _g + 0x30);
            s_nowReloadingInitialized = 1;
        }
        Sing_805DC610() = &s_nowReloading;
    }
    B2dPlayer_UpdateAndDraw(Sing_805DC610());
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            char* p = s_bpCtrl;
            ((CBpCtrl*)p)->__ct();
            __register_global_object(p, CBpCtrl_dtor, _g + 0x18);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* bpCtrl = Sing_805DC5F0();
    ((CBpCtrl*)bpCtrl)->InvokeEntryCallbacks(0);
    ((CBpCtrl*)bpCtrl)->DrawEntries();
    ((CBpCtrl*)bpCtrl)->ResetEntryFrameState();
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
    CCheckDriveStatus::SetSkipDialogDelay(1);
    Engine_PresentFrame(0, 0, 0, 0);
}
}
