// Startup warning and publisher logo screens: the logo timeline, the wait for resident resources,
// and the sprite, texture and puppet preloaders used by the startup sequence.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
// Startup warning screens, publisher logos, resident-resource readiness and logo movie.
// Modding lookup (frame rate): CWarningLogo::UpdateSequence increments timelineUpdateCount once
// on each pending call; WarningTiming holds ABSOLUTE timeline thresholds in
// update calls. delayCounter is a separate Nintendo-only count. No dt is used.
// CWarningLogo::StartWarningSequence starts the normal strap path at timeline 105, so the
// 150 skip / 705 auto-exit thresholds are not durations measured from zero.
// Increasing thresholds delays their gate at a fixed update cadence; compare
// all skip/end/reset sites together and keep skip <= end. Timing safe ranges
// beyond the stock ordered sequence have not been established.
// Resolution lookup: CWarningLogo::CWarningLogo initializes 640x480 configuration
// words at +44/+48. The lifecycle below never passes the +30/+44 suffix to the
// renderer: these are initialized metadata, with no demonstrated application by
// this task. Active output is configured by Game_InitializeEngineAndMedia /
// Engine_ChangeDisplayMode (engine_startup_and_ani_sprite.cpp/gx_renderer_state_and_resources.cpp). Scripts choose layouts
// by language; the movie controller owns movie playback/buffers/audio.
// Shared layouts: CWarningLogo.h; string/config companions keep the ABI pool.
// BP entries are registry-owned and deleted by ID; singleton lifetime is global.
// Preserve animation readiness, HOME suppression, and SharedResources' delayed
// ready return while changing timing. Synchronous preload drains may busy-wait.
struct WarningLogoStrings {
    char residentEffectsArchive[15]; // +0x0: "reside_eff.zip"
    char commonUiArchive[14];        // +0xF: "common_2d.zip"
    char frenchLayout[24];           // +0x1D: "P2_logo_warning_fre.b2d"
    char spanishLayout[24];          // +0x35: "P2_logo_warning_spa.b2d"
    char defaultLayout[20];          // +0x4D: "P2_logo_warning.b2d"
    char strapEnter[17];             // +0x61: "warning_strap_IN"
    char lightWarningExit[18];       // +0x72: "warning_light_OUT"
    char strapExit[18];              // +0x84: "warning_strap_OUT"
    char nintendoEnter[12];          // +0x96: "Nintendo_IN"
    char nintendoHold[17];           // +0xA2: "Nintendo_default"
    char nintendoExit[13];           // +0xB3: "Nintendo_OUT"
    char tecmoEnter[9];              // +0xC0: "TECMO_IN"
    char tecmoExit[10];              // +0xC9: "TECMO_OUT"
    char ntreevEnter[10];            // +0xD3: "Ntreev_IN"
    char ntreevExit[11];             // +0xDD: "Ntreev_OUT"
    char tachyonMovie[12];           // +0xE8: "tachyonlogo"
};

#include "classes/CWarningLogo.h"
#include "classes/CPreloadWPuppetAsync.h"
#include "classes/Context8028A588.h"

enum WarningLogoState {
    kWaitLightDelay = 1,
    kWaitLightExit = 2,
    kHoldStrapWarning = 3,
    kWaitNintendoEnter = 4,
    kHoldNintendo = 5,
    kWaitNintendoHold = 6,
    kWaitLogoExit = 7,
    kHoldTecmo = 8,
    kWaitTecmoExit = 9,
    kHoldNtreev = 10,
    kWaitNtreevExit = 11,
    kWaitMovieReady = 12,
    kPlayMovie = 13
};
// Update calls, not elapsed seconds, drive both counters. timelineUpdateCount is a
// shared timeline: several skip paths jump it forward to their end threshold;
// delayCounter belongs only to the Nintendo hold. State 4 is an alternate entry
// path; normal startup enters state 3 and skips the light/Nintendo path.
// Transitions: 1->2->3->7->8->9->10->11->12->13->finished,
//              4->5->6->7. Each animation wait must finish before its successor.
enum WarningTiming {
    kLightWarningFrames = 0x69,
    kStrapSkippableFrame = 0x96,
    kStrapEndFrame = 0x2C1,
    kNintendoHoldFrames = 0x1E,
    kTecmoSkippableFrame = 0x2DF,
    kTecmoEndFrame = 0x339,
    kNtreevSkippableFrame = 0x366,
    kNtreevEndFrame = 0x3B1,
    kMovieSkippableFrame = 0x3C0
};
// Stock absolute timeline gates in decimal update calls:
// light 105; strap skip/end 150/705; TECMO skip/end 735/825;
// Ntreev skip/end 870/945; movie skip 960 after a reset to 945 (15 calls).
// Nintendo's 30-call delay is independent. Animation-completion gates can add
// more updates, so dividing thresholds by 30 does not give each logo's duration.
enum WarningOptions {
    kFrenchLanguage = 3,
    kSpanishLanguage = 4,
    kLogoConfigurationWidth = 640,
    kLogoConfigurationHeight = 480,
    kLogoMovieBufferBytes = 0x80000,
    kLogoMovieMode = 2
};
struct StartupRegistrations {
    char bpCtrl[12];
    char inputManager[12];
    char reserved[12];
    char movieController[12];
    char homeMenu[12];
};

#include "game/singletons.h"
#include "classes/CPreloadSpr.h"
#include "classes/CPreloadWPuppetSync.h"
#include "classes/CPreloadSprSync.h"
#include "classes/CPreloadFileSync.h"
#include "classes/CBpCtrl.h"
#include "classes/CProjectG.h"
#include "classes/CPreloadTextureSync.h"
#include "classes/CPreloadWPuppet.h"
extern "C" {

void __dt__11CPreloadSprFv(void*, int);
extern int SpritePreload_DestroyOwnedObjects(void*);
extern void __dl__FPv(void*);
extern void CBpCtrl_dtor(void*);
extern void __register_global_object(void*, void*, void*);
extern void* IdMap_Lookup(void*, void*);
extern void BpObject_StartScriptByName(void*, const char*, int);
extern int BpObject_IsCurrentScriptComplete(void*);
extern void BpObject_TryInvokeCurrentCallback(void*, int);
extern void InputManager_Init(void*);
extern void InputManager_Dtor(void);
extern int InputManager_IsAnyMenuAdvanceActionActive(void*);
extern int HomeButtonMenu_InitializeState(void*);
extern void HomeButtonResources_dtor(void);
extern void MovieController_Ctor(void*);
extern void MovieController_Dtor(void);
extern void MovieController_LoadNamedMovie(void*, const char*, int, int);
extern int MovieController_PollReadyLatch(void*);
extern void MovieController_StartPlayback(void*, int);
extern void MovieController_ReleaseAudioAndResetLatches(void*);
extern int Movie_GetPlaybackState(void*);
extern int SharedResources_PollStartupLoads(void);
extern void BpObject_Draw(void*);
extern void BpObject_StartScriptByName(void*, const char*, int);
extern void Game_LoadResidentResources(void*);
extern void MPBase1_Ctor(void*);
extern void MPBase1_SetAutoStart(void*, int);
extern unsigned char SCGetLanguage(void);
extern void* __nw__FUl(unsigned int);
extern char __vt__15CPreloadSprSync[0x100];
extern char __vt__20CPreloadWPuppetAsync[0x100];
extern char __vt__12CWarningLogo[0x100];
extern const char startupArchiveAndWarningStrings[0x100];
extern signed char s_bpCtrlInitialized;
extern signed char s_inputManagerInitialized;
extern signed char s_homeMenuInitialized;
extern signed char s_movieControllerInitialized;
extern void* g_project;
extern char s_homeButtonResourceState[0x200];
extern char s_gameInputManager[0x10000];
extern char s_movieController[0x10000];
extern char s_bpCtrl[0x10000];
extern char s_taskSingletonRegistrations_80509C30[0x10000];
extern const float kFloat_6_2831855;
extern float s_twoPi_PreloadSprSync;
extern float s_twoPi_PreloadWPuppetAsync;

extern const int kWarningLogoInitialConfigurationWords[5];

// Lazy singleton set-up (construct on first use, register its destructor): these three helpers replace the
// block the original code pasted inline at every use. After the call, Sing_805DC5F0() / GetInputManagerSlot() /
// Sing_805DC950() hold the CBpCtrl / game input manager / movie controller instance.
static inline void EnsureBpCtrl(char* registrations) {
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, registrations);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}
// Acquire the shared input manager and register destruction in stable startup storage.
static inline void EnsureInputManager(char* registrations) {
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     ((StartupRegistrations*)registrations)->inputManager);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
}
// Acquire the shared movie controller; registration storage must outlive all logo tasks.
static inline void EnsureMovieController(char* registrations) {
    if (!Sing_805DC950()) {
        if (!s_movieControllerInitialized) {
            MovieController_Ctor(s_movieController);
            __register_global_object(s_movieController, (void*)MovieController_Dtor,
                                     ((StartupRegistrations*)registrations)->movieController);
            s_movieControllerInitialized = 1;
        }
        Sing_805DC950() = s_movieController;
    }
}

extern int GolfRule_PollSequencePreloads(void);
extern int Get__15ISharedResourceFv(void*);
extern void SharedFilePointerVector_Destroy(void);
extern void SharedFilePointerVector_InitReserved1024(void*);
extern int FileList_GetLoadedBufferByName(void*, void*);
extern void NString_AssignCString(void*, void*);
extern void appFreeUni__FPv(void*);
extern int g_sharedResourcesReady;
extern void* g_residentEffectArchive;
extern signed char s_sharedFileListInitialized;
extern void* sharedFileResourceNames[2];
extern void* sharedLabelResourceNames;
extern void* s_startupSharedResourceHandles[3];
extern char s_sharedFileListStorage[];
extern char s_sharedFileListDtorRegistration_8026B004[];
extern void* sharedGolfUiArchiveNames[3];

} // extern "C"

// The warning/logo task has no extra completion hook.
void CWarningLogo::vfunc_7() {}

// Advance warning/logo gates once, polling resident loads and the skippable movie.
// Returns 0 while pending, 1 on
// terminal movie exit after releasing audio/resetting movie latches and setting
// finished. Terminal returns skip callback pumping and counter increment.
// No elapsed-time input: preserve absolute thresholds, startup/reset counter
// values, animation-completion gates and HOME side effects for a cadence mod.
int CWarningLogo::UpdateSequence() {
    char* registrations = s_taskSingletonRegistrations_80509C30;
    // Common prelude: progress shared loads and suppress HOME's blocked-message
    // only while the strap warning is held. Singleton initialization is local
    // to each use; its registration storage must remain stable for shutdown.
    SharedResources_PollStartupLoads();
    if (state == kHoldStrapWarning) {
        if (!((int&)Sing_805DC820())) {
            if (!s_homeMenuInitialized) {
                HomeButtonMenu_InitializeState(s_homeButtonResourceState);
                __register_global_object(s_homeButtonResourceState, (void*)HomeButtonResources_dtor,
                                         ((StartupRegistrations*)registrations)->homeMenu);
                s_homeMenuInitialized = 1;
            }
            ((int&)Sing_805DC820()) = (int)s_homeButtonResourceState;
        }
        (*(signed char*)&((Context8028A588*)(void*)((int&)Sing_805DC820()))->blockedMessageSuppressed) = 1;
    } else {
        if (!((int&)Sing_805DC820())) {
            if (!s_homeMenuInitialized) {
                HomeButtonMenu_InitializeState(s_homeButtonResourceState);
                __register_global_object(s_homeButtonResourceState, (void*)HomeButtonResources_dtor,
                                         ((StartupRegistrations*)registrations)->homeMenu);
                s_homeMenuInitialized = 1;
            }
            ((int&)Sing_805DC820()) = (int)s_homeButtonResourceState;
        }
        (*(signed char*)&((Context8028A588*)(void*)((int&)Sing_805DC820()))->blockedMessageSuppressed) = 0;
    }

    switch (state) {
    case kWaitLightDelay:
        // Timed light hold -> light exit; no input skip is accepted here.
        if (timelineUpdateCount < kLightWarningFrames)
            break;
        EnsureBpCtrl(registrations);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId),
                                   ((WarningLogoStrings*)startupArchiveAndWarningStrings)->lightWarningExit, 1);
        state = kWaitLightExit;
        break;
    case kWaitLightExit:
        // Wait for light exit before installing the strap entry animation.
        EnsureBpCtrl(registrations);
        void* lightExitScript = (void*)IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId);
        if (!BpObject_IsCurrentScriptComplete(lightExitScript))
            break;
        EnsureBpCtrl(registrations);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId),
                                   ((WarningLogoStrings*)startupArchiveAndWarningStrings)->strapEnter, 1);
        state = kHoldStrapWarning;
        break;
    case kHoldStrapWarning:
        // Advance on input after the minimum hold, or automatically at the end.
        if (timelineUpdateCount < kStrapEndFrame) {
            if (timelineUpdateCount < kStrapSkippableFrame)
                break;
            EnsureInputManager(registrations);
            if (!InputManager_IsAnyMenuAdvanceActionActive(GetInputManagerSlot()))
                break;
        }
        timelineUpdateCount = kStrapEndFrame;
        EnsureBpCtrl(registrations);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId),
                                   ((WarningLogoStrings*)startupArchiveAndWarningStrings)->strapExit, 1);
        state = kWaitLogoExit;
        break;
    case kWaitNintendoEnter:
        // Alternate Nintendo path: previous script must finish before Nintendo_IN.
        EnsureBpCtrl(registrations);
        void* script190 = (void*)IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId);
        if (!BpObject_IsCurrentScriptComplete(script190))
            break;
        EnsureBpCtrl(registrations);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId),
                                   ((WarningLogoStrings*)startupArchiveAndWarningStrings)->nintendoEnter, 1);
        state = kHoldNintendo;
        break;
    case kHoldNintendo:
        // Start Nintendo_default once; hold for delayCounter's 30-update window.
        if (delayCounter == 0) {
            EnsureBpCtrl(registrations);
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId),
                                       ((WarningLogoStrings*)startupArchiveAndWarningStrings)->nintendoHold, 1);
        } else if (delayCounter >= kNintendoHoldFrames) {
            state = kWaitNintendoHold;
        }
        ++delayCounter;
        break;
    case kWaitNintendoHold:
        // Only a completed hold script may transition to Nintendo_OUT.
        EnsureBpCtrl(registrations);
        void* nintendoHoldScript = (void*)IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId);
        if (!BpObject_IsCurrentScriptComplete(nintendoHoldScript))
            break;
        EnsureBpCtrl(registrations);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId),
                                   ((WarningLogoStrings*)startupArchiveAndWarningStrings)->nintendoExit, 1);
        state = kWaitLogoExit;
        break;
    case kWaitLogoExit:
        // Shared join for strap/Nintendo exits; launch TECMO only after completion.
        EnsureBpCtrl(registrations);
        void* logoExitScript = (void*)IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId);
        if (!BpObject_IsCurrentScriptComplete(logoExitScript))
            break;
        EnsureBpCtrl(registrations);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId),
                                   ((WarningLogoStrings*)startupArchiveAndWarningStrings)->tecmoEnter, 1);
        state = kHoldTecmo;
        break;
    case kHoldTecmo:
        // Minimum hold, optional input skip, then timeline snap and TECMO_OUT.
        if (timelineUpdateCount < kTecmoEndFrame) {
            if (timelineUpdateCount < kTecmoSkippableFrame)
                break;
            EnsureInputManager(registrations);
            if (!InputManager_IsAnyMenuAdvanceActionActive(GetInputManagerSlot()))
                break;
        }
        timelineUpdateCount = kTecmoEndFrame;
        EnsureBpCtrl(registrations);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId),
                                   ((WarningLogoStrings*)startupArchiveAndWarningStrings)->tecmoExit, 1);
        state = kWaitTecmoExit;
        break;
    case kWaitTecmoExit:
        // Completion transfers the BP entry to Ntreev_IN.
        EnsureBpCtrl(registrations);
        void* tecmoExitScript = (void*)IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId);
        if (!BpObject_IsCurrentScriptComplete(tecmoExitScript))
            break;
        EnsureBpCtrl(registrations);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId),
                                   ((WarningLogoStrings*)startupArchiveAndWarningStrings)->ntreevEnter, 1);
        state = kHoldNtreev;
        break;
    case kHoldNtreev:
        // Hold/skip gate, set the existing project property, then Ntreev_OUT.
        if (timelineUpdateCount < kNtreevEndFrame) {
            if (timelineUpdateCount < kNtreevSkippableFrame)
                break;
            EnsureInputManager(registrations);
            if (!InputManager_IsAnyMenuAdvanceActionActive(GetInputManagerSlot()))
                break;
        }
        ((CProjectG*)g_project)->SetExtensionProperty(-0x100);
        EnsureBpCtrl(registrations);
        BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId),
                                   ((WarningLogoStrings*)startupArchiveAndWarningStrings)->ntreevExit, 1);
        state = kWaitNtreevExit;
        break;
    case kWaitNtreevExit:
        // Both the exit animation AND shared loads must finish before movie load.
        EnsureBpCtrl(registrations);
        void* ntreevExitScript = (void*)IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId);
        if (!BpObject_IsCurrentScriptComplete(ntreevExitScript))
            break;
        if (!SharedResources_PollStartupLoads())
            break;
        EnsureMovieController(registrations);
        MovieController_LoadNamedMovie(Sing_805DC950(),
                                       ((WarningLogoStrings*)startupArchiveAndWarningStrings)->tachyonMovie,
                                       kLogoMovieBufferBytes, kLogoMovieMode);
        state = kWaitMovieReady;
        break;
    case kWaitMovieReady:
        // Loading owns the movie request; start playback only after its ready latch.
        EnsureMovieController(registrations);
        if (!MovieController_PollReadyLatch(Sing_805DC950()))
            break;
        EnsureMovieController(registrations);
        MovieController_StartPlayback(Sing_805DC950(), 0);
        state = kPlayMovie;
        timelineUpdateCount = kNtreevEndFrame;
        break;
    case kPlayMovie:
        // Input is accepted only after the movie minimum. Playback states 0/2
        // also terminate; both paths release audio/reset latches before returning 1.
        if (timelineUpdateCount >= kMovieSkippableFrame) {
            EnsureInputManager(registrations);
            if (InputManager_IsAnyMenuAdvanceActionActive(GetInputManagerSlot())) {
                EnsureMovieController(registrations);
                MovieController_ReleaseAudioAndResetLatches(Sing_805DC950());
                finished = 1;
                return 1;
            }
        }
        EnsureMovieController(registrations);
        switch (Movie_GetPlaybackState(Sing_805DC950())) {
        case 0:
        case 2: {
            EnsureMovieController(registrations);
            MovieController_ReleaseAudioAndResetLatches(Sing_805DC950());
            finished = 1;
            return 1;
        }
        case 1:
        default:
            break;
        case 3:
            break;
        }
        break;
    default:
        break;
    }
    EnsureBpCtrl(registrations);
    // Common pending tail: pump the BP callback and advance one timeline update.
    // Terminal returns above deliberately skip both operations.
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId), 0);
    ++timelineUpdateCount;
    return 0;
}
namespace app {
template <class T> class app_allocator {};
}
namespace std {
template <class T> class char_traits {};
template <class T, class Traits, class Alloc> class LocalBasicString_u8026B004 {
public:
    unsigned int data[3];
    // Initialize an empty packed inline string.
    LocalBasicString_u8026B004() {
        data[0] = 0;
        data[1] = 0;
        data[2] = 0;
    }
    // Release only heap storage owned by this temporary string.
    ~LocalBasicString_u8026B004() {
        if (data[0] >> 31)
            appFreeUni__FPv((void*)data[2]);
    }
};
}
extern "C" {
#pragma use_lmw_stmw on
// Poll required shared resources and release the resident effect archive once all are ready.
// The return value is latched: 1 only when g_sharedResourcesReady was already
// set on entry. The call that first observes all resources ready still returns 0.
int SharedResources_PollStartupLoads(void) {
    if (g_sharedResourcesReady)
        return 1;
    // Optimistically mark ready, then clear the latch for any missing dependency.
    g_sharedResourcesReady = 1;
    if (!GolfRule_PollSequencePreloads())
        g_sharedResourcesReady = 0;
    for (unsigned i = 0; i < 3; ++i) {
        if (!Get__15ISharedResourceFv(s_startupSharedResourceHandles[i]))
            g_sharedResourcesReady = 0;
    }
    for (unsigned i = 0; i < 2; ++i) {
        if (!((int&)Sing_805DC680())) {
            if (!s_sharedFileListInitialized) {
                SharedFilePointerVector_InitReserved1024(&s_sharedFileListStorage);
                __register_global_object(&s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy,
                                         &s_sharedFileListDtorRegistration_8026B004);
                s_sharedFileListInitialized = 1;
            }
            ((int&)Sing_805DC680()) = (int)&s_sharedFileListStorage;
        }
        if (!FileList_GetLoadedBufferByName((void*)((int&)Sing_805DC680()), sharedFileResourceNames[i]))
            g_sharedResourcesReady = 0;
    }
    for (unsigned i = 0; i < 3; ++i) {
        if (!((int&)Sing_805DC680())) {
            if (!s_sharedFileListInitialized) {
                SharedFilePointerVector_InitReserved1024(&s_sharedFileListStorage);
                __register_global_object(&s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy,
                                         &s_sharedFileListDtorRegistration_8026B004);
                s_sharedFileListInitialized = 1;
            }
            ((int&)Sing_805DC680()) = (int)&s_sharedFileListStorage;
        }
        if (!FileList_GetLoadedBufferByName((void*)((int&)Sing_805DC680()), sharedGolfUiArchiveNames[i]))
            g_sharedResourcesReady = 0;
    }
    for (unsigned i = 0; i < 1; ++i) {
        std::LocalBasicString_u8026B004<char, std::char_traits<char>, app::app_allocator<char> > str;
        NString_AssignCString(&str, (&sharedLabelResourceNames)[i]);
        unsigned char* strBytes = (unsigned char*)&str;
        if (!((int&)Sing_805DC680())) {
            if (!s_sharedFileListInitialized) {
                SharedFilePointerVector_InitReserved1024(s_sharedFileListStorage);
                __register_global_object(s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy,
                                         s_sharedFileListDtorRegistration_8026B004);
                s_sharedFileListInitialized = 1;
            }
            ((int&)Sing_805DC680()) = (int)s_sharedFileListStorage;
        }
        void* item;
        item = (!((*(unsigned int*)strBytes) >> 31)) ? (void*)(strBytes + 1) : *(void**)(strBytes + 8);
        if (!FileList_GetLoadedBufferByName((void*)((int&)Sing_805DC680()), item))
            g_sharedResourcesReady = 0;
    }
    // Transfer no dependencies: only the temporary resident archive is released.
    if (g_sharedResourcesReady && g_residentEffectArchive) {
        struct V {
            virtual ~V() DECOMP_LOCAL_VIRTUAL;
        };
        delete ((V*)g_residentEffectArchive);
        g_residentEffectArchive = 0;
    }
    return 0;
}
#pragma use_lmw_stmw reset

} // extern "C"

// Delete the warning animation entry and optionally free the task.
void* CWarningLogo::Destroy(int shouldDelete) {
    if (this) {
        int id = scriptId;
        vtbl = __vt__12CWarningLogo;
        if (id != -1) {
            if (!Sing_805DC5F0()) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_taskSingletonRegistrations_80509C30);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(scriptId);
            scriptId = -1;
        }
        if (shouldDelete > 0)
            __dl__FPv(this);
    }
    return this;
}

// Preloader ABI view: destroy at vtable +0x08 takes deletingFlag; initialize at
// +0x14 consumes the filename described below (nonzero setup success). v0C/v10 are
// unused slots; their order fixes the dispatch offsets.
class Z2 {
public:
    virtual void destroy(int);
    virtual void v0C();
    virtual void v10();
    virtual int initialize(const char*);
};
// Construct the puppet-preloader registrations, install async dispatch, and clear load state.
inline CPreloadWPuppetAsync::CPreloadWPuppetAsync() {
    ((CPreloadWPuppet*)this)->__ct();
    vtbl = __vt__20CPreloadWPuppetAsync;
    loadState = 0;
}
// Allocate an asynchronous puppet preloader; destroy it if resource setup fails.
// Returns caller-owned initialized storage, or null; failed initialization calls
// the deleting destructor with 1. sourcePath is a required NUL-terminated narrow
// filename, not a pointer to a request struct, file bytes or an existing loader.
// Vtable +14 = CPreloadWPuppet_OpenSourceFile (preload_wpuppet.cpp) -> +18 async file factory
// (preload_avatar_wpuppet_async.cpp) -> CPreloadFile_OpenResource (preload_file_and_sequence.cpp) -> Resource_OpenResolvedPath
// (resource_manager_cache_and_loading.cpp). The last two validate nonnull/path existence and own a stream;
// Resource_OpenResolvedPath/SharedFile_CreateFromDvdPath COPY the path before
// returning. Caller bytes need survive only this call, even for async loading.
// Avatar callers in preload_avatar_wpuppet_async.cpp pass temporary model-derived .bpet/.apet names;
// the loaded data must satisfy LoadFileResources' mesh/external-reference parser
// in preload_wpuppet.cpp. No broader extension/content whitelist is checked by this factory.
// Nonnull return means the source opened, NOT that mesh/textures are ready:
// UpdateLoadState advances 0(file)->1(children)->2(ready); IsResourcesReady tests2.
// The loader owns its file and child vector; transfer it to a preload parent's
// owned list, or release with CPreloadWPuppetAsync_dtor(...,1). That destructor
// destroys file/children and storage; never free it as a bare integer handle.
// Missing path/stream yields null after deleting the partial loader; allocation
// exceptions propagate. No future access to the sourcePath argument is required.
void* CPreloadWPuppetAsync::Create(const char* sourcePath) {
    CPreloadWPuppetAsync* preloader = new CPreloadWPuppetAsync;
    if (!preloader)
        return 0;
    if (!((Z2*)preloader)->initialize(sourcePath)) {
        if (preloader)
            ((Z2*)preloader)->destroy(1);
        return 0;
    }
    return preloader;
}

struct Fn8026B278Obj {
    char bytes[0x7C];
};
// Initialize warning configuration and request the French/Spanish/default BP layout.
// Caller supplies fresh 0x7C-byte storage; configuration width/height words use
// the 640x480 baseline. The controller owns the entry; this task retains its ID
// until Destroy deletes it. Preserve layout choice and auto-start.
CWarningLogo::CWarningLogo() {
    // Base task registrations come first.
    MPBase1_Ctor(this);
    Fn8026B278Obj* self = (Fn8026B278Obj*)this;
    struct FourInts {
        int a, b, c, d;
    };
    FourInts* initialConfigWords = (FourInts*)kWarningLogoInitialConfigurationWords;
    *(void**)self = __vt__12CWarningLogo;
    // Task prefix: +18 script ID (-1 until requested), +1c state, +20 timeline,
    // +24 Nintendo delay, +28 completion latch. These are the same fields used
    // through the update and destructor methods.
    *(int*)(self->bytes + 0x24) = 0;
    *(int*)(self->bytes + 0x18) = -1;
    *(int*)(self->bytes + 0x20) = 0;
    *(int*)(self->bytes + 0x1C) = 0;
    *(int*)(self->bytes + 0x28) = 0;
    // Complete suffix schema (u32 words, four-byte alignment, same shape as
    // EngineViRect followed by EnginePresentParams; NO pointer ownership):
    // +2C=0 spare task word, +30/+34=0/0 output origin pixels,
    // +38/+3C=640/480 output size pixels, +40=2 NTSC output-format code;
    // +44/+48=640/480 presentation size, +4C=6 reserved08, +50=0 reserved0C,
    // +54=0 mode type; +58..6C={0,1,0,0,1,22} reserved14[6];
    // +70=40 progressive-scan flags, +74=0 system TV selection,
    // +78=2 swap interval in retraces. These encodings agree with
    // Engine_BuildRenderMode / EnginePresentParams (gx_renderer_state_and_resources.cpp); 6/22 remain
    // opaque reserved values, not new tuning knobs. The first four words are
    // Consumer audit: StartWarningSequence loads resident resources/starts BP; Draw draws
    // BP; UpdateSequence advances the logo/movie states; the poll checks BP; one slot is
    // empty. The inherited base function returns 0 and one more slot is empty
    // (event_face_and_mpbase1_menu.cpp). None reads this suffix or submits it to ChangeDisplayMode.
    // Keep initialized representation, but use the startup/render-mode APIs for
    // an output mod; merely changing these dormant sizes has no verified effect.
    *(FourInts*)(self->bytes + 0x30) = *initialConfigWords;
    *(int*)(self->bytes + 0x40) = 2;
    *(int*)(self->bytes + 0x44) = kLogoConfigurationWidth;
    *(int*)(self->bytes + 0x48) = kLogoConfigurationHeight;
    *(int*)(self->bytes + 0x4C) = 6;
    *(int*)(self->bytes + 0x50) = 0;
    *(int*)(self->bytes + 0x54) = 0;
    *(int*)(self->bytes + 0x58) = 0;
    *(int*)(self->bytes + 0x5C) = 1;
    *(int*)(self->bytes + 0x60) = 0;
    *(int*)(self->bytes + 0x64) = 0;
    *(int*)(self->bytes + 0x68) = 1;
    *(int*)(self->bytes + 0x6C) = 0x16;
    *(int*)(self->bytes + 0x70) = 0x40;
    *(int*)(self->bytes + 0x74) = 0;
    *(int*)(self->bytes + 0x78) = 2;
    *(int*)(self->bytes + 0x2C) = 0;
    // The controller writes the resource ID into +18; this task deletes it later.
    switch (SCGetLanguage()) {
    case kFrenchLanguage:
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_taskSingletonRegistrations_80509C30);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())
            ->CreateEntry(((WarningLogoStrings*)startupArchiveAndWarningStrings)->frenchLayout, self->bytes + 0x18, 0);
        break;
    case kSpanishLanguage:
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_taskSingletonRegistrations_80509C30);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())
            ->CreateEntry(((WarningLogoStrings*)startupArchiveAndWarningStrings)->spanishLayout, self->bytes + 0x18, 0);
        break;
    default:
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_taskSingletonRegistrations_80509C30);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())
            ->CreateEntry(((WarningLogoStrings*)startupArchiveAndWarningStrings)->defaultLayout, self->bytes + 0x18, 0);
        break;
    }
    MPBase1_SetAutoStart(self, 1);
}

// Draw the current warning/logo animation entry.
void CWarningLogo::Draw() {
    /* singleton init */
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_taskSingletonRegistrations_80509C30);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* warningBpObject = (void*)IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId);
    BpObject_Draw(warningBpObject);
}

struct Fn8026C4B8Vtable {
    void* reservedSlot00;
    void* reservedSlot04;
    void (*destroy)(void*, int);
    void* reservedSlot0C;
    void* reservedSlot10;
    int (*init)(void*, void*);
};
struct Fn8026C4B8Obj {
    Fn8026C4B8Vtable* vtable;
};
// Same destroy/initialize slot contract as Z2, for the synchronous sprite factory.
class Z1 {
public:
    virtual void destroy(int);
    virtual void v0C();
    virtual void v10();
    virtual int initialize(const char*);
};
struct Fn8026C4B8New : Fn8026C4B8Obj {
    char pad[0x10];
    // Construct sprite-preloader registrations storage and select synchronous dispatch.
    Fn8026C4B8New() {
        ((CPreloadSpr*)this)->__ct();
        vtable = (Fn8026C4B8Vtable*)__vt__15CPreloadSprSync;
    }
};
// Allocate a synchronous sprite preloader; destroy it if resource setup fails.
// sourcePath is a required NUL-terminated narrow filename of a sprite resource
// directive script, not the script buffer or a request record. +14 dispatches to
// InitSynchronously below: OpenNamedSourceFile (pad_memory_menu_save.cpp) creates an owned
// CPreloadFileSync via +18; the same copied-path stream chain described by the
// puppet factory above applies. Caller storage can expire at factory return.
// Script schema/consumer: CPreloadSpr::ProcessScriptEntries scans trimmed lines;
// CPreloadSpr::ParseResourceDirective in pad_memory_menu_save.cpp consumes key=first,...,sixteenth:
// source uses the first argument (*pet NAME [suffix] selects a puppet via table+1C; otherwise
// a texture via +20), tail uses the seventh argument as a texture path via +20, and sound/unknown
// keys load nothing. The script's own source file uses +18, a separate factory.
// Its puppetPreloadParserStrings
// defines the exact case-sensitive tokens. Unknown directives are skipped,
// not rejected. Source lines/tokens must
// fit that parser's 0x400/0x104-byte scratch buffers. Do not feed arbitrary binary
// sprite bytes here. Readiness drains synchronously without timeout; a nonnull
// result means initialization finished, not a status code. The parent owns file
// and successful child loaders; release using CPreloadSprSync::~CPreloadSprSync(...,1), or
// transfer to a preload parent's owning list. Null setup deletes the partial
// loader. Exceptions propagate; there is no transaction over already loaded data.
void* CPreloadSprSync::Create(const char* sourcePath) {
    Fn8026C4B8Obj* preloader = new Fn8026C4B8New;
    if (!preloader)
        return 0;
    if (!((Z1*)preloader)->initialize(sourcePath)) {
        if (preloader)
            ((Z1*)preloader)->destroy(1);
        return 0;
    }
    return preloader;
}

// Begin resident-resource loading and start the strap-warning animation.
void CWarningLogo::StartWarningSequence() {
    Game_LoadResidentResources(this);
    timelineUpdateCount = (Sing_805DC5F0(), kLightWarningFrames);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_taskSingletonRegistrations_80509C30);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), (void*)scriptId),
                               ((WarningLogoStrings*)startupArchiveAndWarningStrings)->strapEnter, 1);
    state = kHoldStrapWarning;
}

// The synchronous preloader reports this lifecycle stage as ready.
int CPreloadSprSync::UpdateLoadState() {
    return 1;
}

// The synchronous preloader needs no deferred work for this lifecycle stage.
int CPreloadSprSync::vfunc_2() {
    return 1;
}

// Factory-wrapper contract for the three methods below: a is an unused ABI
// receiver word; sourcePath borrows a NUL-terminated resource filename through the factory
// call. int return is a 32-bit pointer to an OWNED preloader, or zero on factory
// setup failure (not a readiness status). File/puppet/texture factories are in
// preload_file_parts_async_loaders.cpp/pad_memory_menu_save.cpp/password_des_base64_texture_preload.cpp; each destroys a rejected new object itself.
// CPreloadSpr::ParseResourceDirective (pad_memory_menu_save.cpp) appends successful pointers to
// its +4 owned-object vector. SpritePreload_DestroyOwnedObjects delegates to
// OwnedObjectList_DestroyAll; destroying the parent CPreloadSpr also tears down
// those children through their deleting virtual destructor(1). A direct caller
// must either transfer the pointer to that owner or destroy it the same way,
// not appFreeUni the integer. Wrappers neither retain sourcePath nor own returned children.
// Create the script-requested synchronous file preload.
void* CPreloadSprSync::CreateFileSync(int unusedReceiver, char* sourcePath) {
    return CPreloadFileSync::Create(sourcePath);
}

// Create the script-requested synchronous puppet preload.
void* CPreloadSprSync::CreateWPuppetSync(int unusedReceiver, char* sourcePath) {
    return CPreloadWPuppetSync::Create(sourcePath);
}

// Create the script-requested synchronous texture preload.
void* CPreloadSprSync::CreateTextureSync(int unusedReceiver, char* sourcePath) {
    return CPreloadTextureSync::Create(sourcePath);
}

extern "C" {
// Initialize the synchronous-preloader helper's two-pi cache.
void PreloadSprSync_InitTwoPiCache(void) {
    s_twoPi_PreloadSprSync = kFloat_6_2831855;
}

// Initialize the asynchronous-preloader helper's two-pi cache.
void PreloadWPuppetAsync_InitTwoPiCache(void) {
    s_twoPi_PreloadWPuppetAsync = kFloat_6_2831855;
}
} // extern "C"

// Report whether the warning/logo task has finished.
int CWarningLogo::IsFinished() {
    return finished != 0;
}

// Destroy the sprite-preloader registrations and optionally free the object.
CPreloadSprSync::~CPreloadSprSync() {
    __dt__11CPreloadSprFv(this, 0);
}

// Set up the sprite preload, clear owned objects, and synchronously drain its script entries.
int CPreloadSprSync::InitSynchronously(void* preloader, void* name) {
    if (!((CPreloadSpr*)preloader)->OpenNamedSourceFile(name))
        return 0;
    if (!SpritePreload_DestroyOwnedObjects(preloader))
        return 0;
    while (!((CPreloadSpr*)preloader)->ProcessScriptEntries()) {
    }
    return 1;
}

// Poll loading of all animation-controller entries.
int CWarningLogo::PollAnimationEntriesLoaded() {
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_taskSingletonRegistrations_80509C30);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    return ((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded();
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462344", _ctors_u8026B004_0, PreloadSprSync_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU80462344", _ctors_u8026B004_1, PreloadWPuppetAsync_InitTwoPiCache);
#pragma pop
