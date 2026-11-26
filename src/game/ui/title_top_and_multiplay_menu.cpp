// The title start screen (CPangya2TitleTop) and its multiplayer choice menu (CMultiPlayTopMenu):
// waiting for a press to start, choosing single or multiplayer, and the course-preview background.

#include <decomp/static_init.h>
// Title start prompt, single/multiplayer choice and course-preview background.
// Modding lookup (frame rate): kAttractIdleUpdates controls the title attract
// timeout in CPangya2TitleTop::UpdateTitleSequence. It is 900 calls in AwaitStart (nominally
// 30 seconds at 30 calls/s); fades freeze this state machine. It is independent
// of CMainFrame's seconds/update and the movie's presentation clock.
// Increasing it delays attract entry at fixed cadence; reset/increment sites
// must retain their order. An end-to-end safe custom range is not established.
// Textures / widescreen: CPangya2TitleTop::CPangya2TitleTop chooses one of 14 preview names
// in coursePreviewTextureNames. DrawBackgroundAndScene derives background quad edges from the
// B2D group's layout position/source size. There is no hard-coded screen size
// or aspect ratio here; source rectangles/UVs must agree with replacement sheets.
// Keep kQuadVertexStride, kTexturedVertexFormat and template Z/color/UV lanes
// together; they describe vertex storage, not an editable output resolution.
// Owned layout/texture wrappers and the BP entry are released by the title
// destructor; bpObject is borrowed from that entry. Global singleton storage
// and destructor-registration blocks survive this menu. Shared layouts are
// CPangya2TitleTop.h, CMultiPlayTopMenu.h and CSharedDoc.h; constants have a data
// companion the data section. Preserve script/fade/movie gates and RNG calls.
// Byte layout of the existing string pool; no new string storage.
struct MultiPlayMenuStringPool {
    char top_menu_spd[13]; // "top_menu.spd"
    char top_menu_srd[13]; // "top_menu.srd"
    char top_menu_b2d[13]; // "top_menu.b2d"
    char topmenu_zip[12];  // "topmenu.zip"
};
enum {
    kMultiPlayMenuStringPool_top_menu_spd = (int)&((MultiPlayMenuStringPool*)0)->top_menu_spd,
    kMultiPlayMenuStringPool_top_menu_srd = (int)&((MultiPlayMenuStringPool*)0)->top_menu_srd,
    kMultiPlayMenuStringPool_top_menu_b2d = (int)&((MultiPlayMenuStringPool*)0)->top_menu_b2d,
    kMultiPlayMenuStringPool_topmenu_zip = (int)&((MultiPlayMenuStringPool*)0)->topmenu_zip,
};

// Byte layout of the existing string pool; no new string storage.
struct TitleStringPool {
    char title_zip[10];            // "title.zip"
    char p2_title_start_b2d[19];   // "p2_title_start.b2d"
    char p2_title_start_spd[19];   // "p2_title_start.spd"
    char introScript[3];           // "in"
    char menu_wav[9];              // "menu.wav"
    char se_op_title[12];          // "se_op_title"
    char loop_A[7];                // "loop_A"
    char r_copyright_1[14];        // "R_copyright_1"
    char out_all[8];               // "out_all"
    char advertise[10];            // "advertise"
    char out[4];                   // "out"
    char mode_button_ok_click[21]; // "mode_button_ok_click"
    char r_title_bg[11];           // "R_title_bg"
};
enum {
    kTitleStringPool_title_zip = (int)&((TitleStringPool*)0)->title_zip,
    kTitleStringPool_p2_title_start_b2d = (int)&((TitleStringPool*)0)->p2_title_start_b2d,
    kTitleStringPool_p2_title_start_spd = (int)&((TitleStringPool*)0)->p2_title_start_spd,
    kTitleStringPool_introScript = (int)&((TitleStringPool*)0)->introScript,
    kTitleStringPool_menu_wav = (int)&((TitleStringPool*)0)->menu_wav,
    kTitleStringPool_se_op_title = (int)&((TitleStringPool*)0)->se_op_title,
    kTitleStringPool_loop_A = (int)&((TitleStringPool*)0)->loop_A,
    kTitleStringPool_r_copyright_1 = (int)&((TitleStringPool*)0)->r_copyright_1,
    kTitleStringPool_out_all = (int)&((TitleStringPool*)0)->out_all,
    kTitleStringPool_advertise = (int)&((TitleStringPool*)0)->advertise,
    kTitleStringPool_out = (int)&((TitleStringPool*)0)->out,
    kTitleStringPool_mode_button_ok_click = (int)&((TitleStringPool*)0)->mode_button_ok_click,
    kTitleStringPool_r_title_bg = (int)&((TitleStringPool*)0)->r_title_bg,
};

// Constants retain the original integer types and encoded values.
enum TitlePlayModeElement {
    kSinglePlayerButton = 0,
    kSinglePlayerHighlight = 1,
    kMultiPlayerButton = 2,
    kMultiPlayerHighlight = 3
};
enum {
    kSelectedElementValue = 2,
    kLockedElementPlayerFlag = 5
};

// Constants retain the original integer types and encoded values.
// Phase path: movie readiness -> intro -> await start -> choose play mode -> complete.
// Await-start timeout enters attract loading/playback, then returns to intro.
// Active fades pause this state machine, including its frame-based idle timer.
enum TitlePhase {
    kTitleWaitForMovie = 0,
    kTitleIntro = 1,
    kTitleAwaitStart = 2,
    kTitleChoosePlayMode = 3,
    kTitleComplete = 4,
    kTitleWaitForAttractMovie = 5,
    kTitlePlayAttractMovie = 6
};
enum MenuAction {
    kActionConfirm = 4,
    kActionCancel = 6,
    kSelectedInputPlayer = -1
};
enum {
    kAttractIdleUpdates = 900,
    kMenuItemCount = 20,
    kCoursePreviewCount = 14,
    kMenuSecondaryBaseOffset = 0x18,
    kMenuSecondaryVtableOffset = 0x28,
    kQuadVertexCount = 4,
    kTriangleStripPrimitive = 0x98,
    kQuadVertexStride = 0x18,
    kTexturedVertexFormat = 0x11F005,
    kAttractMovieBufferBytes = 0x100000,
    kAttractMovieMode = 4,
    kMenuExtensionHidden = -0x100
};
enum TitleResourceLayout {
    kTitlePackTextureBytes = 0x20,
    kTitleLayoutBytes = 0x5C,
    kTitleTextureBytes = 0x28,
    kTitleLayoutKind = 5,
    kBpControllerRegistrationOffset = 0x18,
    kHomeResourcesRegistrationOffset = 0x30,
    kResourceTableRegistrationOffset = 0x3C
};
static const unsigned kCopyrightTint = 0x80808080U;

#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/types.h"
#include "classes/CMultiPlayTopMenu.h"
#define GAME_VECTOR_AGGREGATE

#include "game/vector.h"

// Stack-local storage view; external vector operations retain the original ABI.
struct LocalIntVector_u802BADE4 {
    int* items;
    volatile unsigned int size;
    unsigned int capacity;
    // Free the collected menu-element array while preserving the cleanup ABI.
    ~LocalIntVector_u802BADE4() {
        if (items) {
            size = *(unsigned int*)&size - *(int*)&size;
            appFreeUni__FPv(items);
        }
    }
};

#include "game/singletons.h"
// Named references to the existing singleton slots; each retains its original static storage.
#pragma push
#pragma always_inline on
// Access the existing InputManager singleton pointer.
static inline void*& TitleInputManagerSlot() {
    return GetInputManagerSlot();
}
// Access the existing BpController singleton pointer.
static inline void*& TitleBpControllerSlot() {
    return Sing_805DC5F0();
}
// Access the existing BpHandle singleton pointer.
static inline void*& TitleBpHandleSlot() {
    return Sing_805DC5F8();
}
// Access the existing FadeOverlay singleton pointer.
static inline void*& TitleFadeOverlaySlot() {
    return Sing_805DC608();
}
// Access the existing ResourceTable singleton pointer.
static inline void*& TitleResourceTableSlot() {
    return Sing_805DC620();
}
// Access the existing HomeMenu singleton pointer.
static inline void*& TitleHomeMenuSlot() {
    return Sing_805DC820();
}
// Access the existing MovieController singleton pointer.
static inline void*& TitleMovieControllerSlot() {
    return Sing_805DC950();
}
#pragma pop

#include "classes/CPangya2TitleTop.h"
#include "classes/CPackTextureResource.h"
#include "classes/CTopMenuBase.h"
#include "classes/CBpCtrl.h"
#include "classes/CProjectG.h"
#include "classes/CBpCtrlHandle.h"
#include "classes/CBpTex.h"
#include "classes/COptionDialog.h"

#define AsCPangya2TitleTop(p) ((CPangya2TitleTop*)(p))
extern "C" {
extern char s_titleInputManagerDtorRegistration[];

extern const float kFloat_6_2831855;
extern const float kTitleFullOpacity;
extern float s_titleScreenFullCircleAngle;
extern float s_multiPlayTopMenuFullCircleAngle;

extern int Layout_PollLoadComplete(void*);
extern void Layout_Update(void*);
extern void Layout_ReadElementRect(void*, int, void*);
extern void Layout_WriteElementRect(void*, int, void*);
extern void IFadeMenuBase_StartMenuFadeOut(void*);
extern void CBpCtrl_dtor(void*, int);
extern void __register_global_object(void*, void (*)(void*, int), void*);
extern void Layout_Destroy(void*, int);
extern void CBpCtrlHandle_Dtor(void*);
extern void __dt__7MPBase2Fv(void*, int);
extern void __dl__FPv(void*);

extern signed char s_resourceTableInitialized;
extern void* s_resourceTableStorage;
extern void* g_project;
extern signed char s_bpCtrlInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern char s_bpCtrl[];
char s_titleInputManagerDtorRegistration[12];
char lbl_8051149C[12]; // Destructor-chain registration for the title fade overlay.
char s_titleBpControllerDtorRegistration[12];
char lbl_805114B4[12]; // Destructor-chain registration for the movie controller.
char lbl_805114C0[12]; // Destructor-chain registration for the title home-menu resources.
extern char s_titleBpControllerDtorRegistration[];
extern char s_multiPlayBpHandleDtorRegistration[];
extern char __vt__16CPangya2TitleTop[];
extern char __vt__17CMultiPlayTopMenu[];
extern int s_multiPlayTopMenuHoverMessage[];
extern char __vt__13IFadeMenuBase[];
extern char multiPlayTopMenuStrings[];
extern char s_multiPlayMenuLayoutData[]; // Additional layout data passed to the multiplayer menu base constructor.
extern char s_homeButtonResourceState[];
extern char s_multiPlayHomeMenuDtorRegistration[];
extern char coursePreviewTextureNames[];
extern signed char s_homeMenuInitialized;
extern CSharedDoc* g_sharedDoc;
extern void* __nw__FUl(unsigned long);
extern void MPBase1_Ctor(void*);
extern void Allocation_Dtor(void*, int);
extern void* ResourceTable_FindEntryByName(void*, void*, void*);
extern void* IdMap_Lookup(void*, int);
extern void Layout_Init(void*, int);
extern void Layout_LoadExtended(void*, int, void*);
extern int rand(void);
extern void HomeButtonMenu_InitializeState(void*);
extern void HomeButtonResources_dtor();
extern void HomeMenu_SetBlockedState(void*, int);
extern void HomeMenu_OverrideBlockedState(void*, int);
extern int BpObject_IsCurrentScriptComplete(void*);
extern int MPBase1_Process(void*, float);
extern void Layout_SetElementValueState(void*, int, int);
extern void Layout_SetElementPlayerFlag(void*, int, int, int);
extern int Layout_CollectActivated(void*, void*, void*);
extern void BpObject_TryInvokeCurrentCallback(void*, int);
extern void* BpObject_GetOrCreateNamedGroup(void*, void*);
extern void UIGroup_GetPosition(void*, void*);
extern void UIGroup_GetSourceSize(void*, void*);
extern int UIGroup_GetCombinedOpacity(void*);
extern void UIRender_SetDefaultState(void);
extern void* GX_GetPrimitiveCount(int, int);
extern void Engine_SetFlexibleVertexFormat(int);
extern void BpObject_Draw(void*);
extern void Layout_DrawWithOpacity(void*, float);
extern void DrawPrimitiveUP(int, int, void*, int);
extern char titleStartStrings[];
extern const char s_titleBackgroundQuadVertices[];
extern void MenuMessage_UpdateReveal(void*);
extern void appFreeUni__FPv(void*);

extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[];
extern void FadeOverlay_Init(void*);
extern void ResourceState_Dtor(void*, int);
extern int FadeState_GetState(void*);
extern signed char s_movieControllerInitialized;
extern char s_movieController[];
extern void MovieController_Ctor(void*);
extern void MovieController_Dtor(void*, int);
extern int Movie_GetPlaybackState(void*);
extern int MovieController_PollReadyLatch(void*);
extern int MovieController_StartPlayback(void*, int);
extern void MovieController_StopAndFreeSamples(void*);
extern void MovieController_LoadNamedMovie(void*, void*, int, int);
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern void InputManager_Init(void*);
extern void InputManager_Dtor(void*, int);
extern int Input_IsActionPressed(void*, int, int);
extern int InputManager_IsAnyMenuAdvanceActionActive(void*);
extern void BpObject_StartScriptByName(void*, void*, int);
extern void SoundManager_RequestBgmChange(void*, void*, int, int);
extern void Sound_PlayByName(void*, void*, int, int, int, int, float, float, float);
extern void* g_soundManager;
extern void UIGroup_SetTintColor(void*, unsigned int);
extern void SoundManager_RequestBgmStop(void*, int);
extern const float kFloat_0_5_TitleStart;
extern const float kFloat_200_TitleTop;
struct TitleMenuState {
    char baseMenuStorage[0x18];
    int selectionComplete;
    char introStateStorage[0x04];
    int bpEntryId;
    int phase;
    void* bpObject;
    char idleAndSelectionStorage[0x08];
    void* layout;
    void* texturePack;
    void* texture;
};

#pragma push
#pragma always_inline on
// Initialize/register the title fade-overlay ResourceState once, then publish
// its borrowed singleton slot. FadeOverlay_Init constructs this overlay's state;
// ResourceState_Dtor owns shutdown cleanup. This is separate from movie/BP controllers.
static inline void EnsureTitleFadeOverlay() {
    if (!TitleFadeOverlaySlot()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void (*)(void*, int))ResourceState_Dtor, lbl_8051149C);
            s_fadeOverlayInitialized = 1;
        }
        TitleFadeOverlaySlot() = s_fadeOverlay;
    }
}

// Construct/register the shared controller once, then publish its borrowed singleton slot.
static inline void EnsureTitleMovieController() {
    if (!TitleMovieControllerSlot()) {
        if (!s_movieControllerInitialized) {
            MovieController_Ctor(s_movieController);
            __register_global_object(s_movieController, (void (*)(void*, int))MovieController_Dtor, lbl_805114B4);
            s_movieControllerInitialized = 1;
        }
        TitleMovieControllerSlot() = s_movieController;
    }
}

// Construct/register the shared controller once, then publish its borrowed singleton slot.
static inline void EnsureTitleInputManager() {
    if (!TitleInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void (*)(void*, int))InputManager_Dtor,
                                     s_titleInputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        TitleInputManagerSlot() = s_gameInputManager;
    }
}

// Construct/register the shared controller once, then publish its borrowed singleton slot.
static inline void EnsureTitleHomeMenu() {
    if (!TitleHomeMenuSlot()) {
        if (!s_homeMenuInitialized) {
            HomeButtonMenu_InitializeState(s_homeButtonResourceState);
            __register_global_object(s_homeButtonResourceState, (void (*)(void*, int))HomeButtonResources_dtor,
                                     lbl_805114C0);
            s_homeMenuInitialized = 1;
        }
        TitleHomeMenuSlot() = s_homeButtonResourceState;
    }
}
#pragma pop

// Returns 1 on the update after a choice is accepted; all waiting frames return 0.
// The 900-frame idle threshold is measured in update calls, not elapsed seconds.
// BP scripts gate each input phase; the movie, input and HOME controllers are shared.
// Advance the title scripts, play-mode choice, and idle attract-movie state machine.
// self is required with loaded BP/layout/texture resources. Returns 1 one call
// after accepted selection, setting selectionComplete and resetting phase to
// Intro. Fades return early without advancing idle time; AwaitStart increments
// idleUpdateCount even on the update that resets it for attract loading (0 -> 1).
// Preserve completion timing, BP gates, movie cleanup and HOME overrides.
int CPangya2TitleTop::UpdateTitleSequence() {
    TitleMenuState* self = (TitleMenuState*)this;
    EnsureTitleFadeOverlay();
    if (FadeState_GetState(TitleFadeOverlaySlot()))
        return 0;

    switch (self->phase) {
    case kTitleWaitForMovie:
        EnsureTitleMovieController();
        if (Movie_GetPlaybackState(TitleMovieControllerSlot()) != 1) {
            EnsureTitleMovieController();
            if (MovieController_PollReadyLatch(TitleMovieControllerSlot()) == 0)
                break;
            EnsureTitleMovieController();
            if (MovieController_StartPlayback(TitleMovieControllerSlot(), 1) == 0)
                break;
            self->phase = kTitleIntro;
            break;
        }
        self->phase = kTitleIntro;
        break;

    // Entry script publishes the copyright tint; completion enables the start prompt.
    case kTitleIntro:
        if (AsCPangya2TitleTop(self)->introStarted == 0) {
            BpObject_StartScriptByName(self->bpObject, titleStartStrings + kTitleStringPool_introScript, 1);
            ((CProjectG*)g_project)->SetExtensionProperty(0);
            SoundManager_RequestBgmChange(g_soundManager, titleStartStrings + kTitleStringPool_menu_wav, 0, 1);
            Sound_PlayByName(g_soundManager, titleStartStrings + kTitleStringPool_se_op_title, 0, 0, 0, 0,
                             kFloat_0_5_TitleStart, kFloat_200_TitleTop, kTitleFullOpacity);
            AsCPangya2TitleTop(self)->introStarted = 1;
        }
        if (BpObject_IsCurrentScriptComplete(self->bpObject)) {
            BpObject_StartScriptByName(self->bpObject, titleStartStrings + kTitleStringPool_loop_A, 1);
            self->phase = kTitleAwaitStart;
        }
        UIGroup_SetTintColor(
            BpObject_GetOrCreateNamedGroup(self->bpObject, titleStartStrings + kTitleStringPool_r_copyright_1),
            kCopyrightTint);
        break;

    // Choose between an idle attract transition and a confirmed start transition.
    case kTitleAwaitStart:
        if (AsCPangya2TitleTop(self)->idleUpdateCount >= kAttractIdleUpdates) {
            BpObject_StartScriptByName(self->bpObject, titleStartStrings + kTitleStringPool_out_all, 1);
            EnsureTitleHomeMenu();
            HomeMenu_OverrideBlockedState(TitleHomeMenuSlot(), 1);
            EnsureTitleMovieController();
            MovieController_LoadNamedMovie(TitleMovieControllerSlot(), titleStartStrings + kTitleStringPool_advertise,
                                           kAttractMovieBufferBytes, kAttractMovieMode);
            self->phase = kTitleWaitForAttractMovie;
            ((CProjectG*)g_project)->SetExtensionProperty(kMenuExtensionHidden);
            AsCPangya2TitleTop(self)->idleUpdateCount = 0;
            SoundManager_RequestBgmStop(g_soundManager, 1);
        } else {
            if (BpObject_IsCurrentScriptComplete(self->bpObject)) {
                EnsureTitleInputManager();
                if (Input_IsActionPressed(TitleInputManagerSlot(), kActionConfirm, kSelectedInputPlayer)) {
                    BpObject_StartScriptByName(self->bpObject, titleStartStrings + kTitleStringPool_out, 1);
                    self->phase = kTitleChoosePlayMode;
                    Sound_PlayByName(g_soundManager, titleStartStrings + kTitleStringPool_mode_button_ok_click, 0, 0, 0,
                                     0, kFloat_0_5_TitleStart, kFloat_200_TitleTop, kTitleFullOpacity);
                }
            }
        }
        AsCPangya2TitleTop(self)->idleUpdateCount = AsCPangya2TitleTop(self)->idleUpdateCount + 1;
        break;

    // Buttons become active only after the prompt exit script completes.
    case kTitleChoosePlayMode:
        if (BpObject_IsCurrentScriptComplete(self->bpObject)) {
            if (((CPangya2TitleTop*)self)->HandlePairedSelection()) {
                self->phase = kTitleComplete;
                EnsureTitleMovieController();
                MovieController_StopAndFreeSamples(TitleMovieControllerSlot());
                return 0;
            }
            EnsureTitleInputManager();
            if (Input_IsActionPressed(TitleInputManagerSlot(), kActionCancel, kSelectedInputPlayer)) {
                BpObject_StartScriptByName(self->bpObject, titleStartStrings + kTitleStringPool_loop_A, 1);
                self->phase = kTitleAwaitStart;
            }
        }
        break;

    case kTitleComplete:
        self->phase = kTitleIntro;
        AsCPangya2TitleTop(self)->selectionComplete = 1;
        return 1;

    case kTitleWaitForAttractMovie:
        if (BpObject_IsCurrentScriptComplete(self->bpObject)) {
            EnsureTitleMovieController();
            if (MovieController_PollReadyLatch(TitleMovieControllerSlot())) {
                EnsureTitleMovieController();
                if (MovieController_StartPlayback(TitleMovieControllerSlot(), 0))
                    self->phase = kTitlePlayAttractMovie;
            }
        }
        break;

    // Natural movie completion and a menu-advance action both restore title resources.
    case kTitlePlayAttractMovie:
        EnsureTitleMovieController();
        if (Movie_GetPlaybackState(TitleMovieControllerSlot()) == 2) {
            UIRender_SetDefaultState();
            AsCPangya2TitleTop(self)->introStarted = 0;
            EnsureTitleMovieController();
            MovieController_StopAndFreeSamples(TitleMovieControllerSlot());
            self->phase = kTitleIntro;
            EnsureTitleHomeMenu();
            HomeMenu_OverrideBlockedState(TitleHomeMenuSlot(), 0);
        }
        EnsureTitleInputManager();
        if (InputManager_IsAnyMenuAdvanceActionActive(TitleInputManagerSlot())) {
            UIRender_SetDefaultState();
            AsCPangya2TitleTop(self)->introStarted = 0;
            EnsureTitleMovieController();
            MovieController_StopAndFreeSamples(TitleMovieControllerSlot());
            self->phase = kTitleIntro;
            EnsureTitleHomeMenu();
            HomeMenu_OverrideBlockedState(TitleHomeMenuSlot(), 0);
        }
        break;
    }
    return 0;
}

// On true, writes 0 for single player or 1 for multiplayer; on false, leaves *out.
// out must be writable once selectionComplete is set.
// Return the selected play-mode result only after the title selection completes.
bool CPangya2TitleTop::TryGetPlayMode(int* out) {
    if (selectionComplete) {
        *out = !singlePlayerSelected;
        return true;
    }
    return false;
}

// Poll the title texture pack and advance layout loading.
// self/layout must exist; texturePack may be null. This retains every wrapper,
// BP handle and preview texture while work is incomplete: polling returns no
// success/error result, clears nothing, releases nothing and performs no rollback.
// Only an incomplete layout is advanced; the pack is polled whenever present.
// PollLoadComplete separately checks readiness; StartMenuFadeOut later releases texturePack on entry,
// and dtor releases all remaining ownership. A resource that never becomes ready
// leaves the menu waiting indefinitely; this helper has no timeout/retry policy.
void CPangya2TitleTop::PollLoadAndUpdateLayout() {
    TitleMenuState* self = (TitleMenuState*)this;
    if (self->texturePack)
        ((CPackTextureResource*)(self->texturePack))->PollLoad();
    if (!Layout_PollLoadComplete((void*)self->layout))
        Layout_Update((void*)self->layout);
}

struct MenuOwnedObject {
    virtual void destroy(int);
};

struct MenuTextureLoadInterface {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual int isLoaded();
};
struct MenuTextureOpacityInterface {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void setOpacity(int);
};

// Enter the title screen and align each play-mode highlight with its button.
void CPangya2TitleTop::StartMenuFadeOut() {
    TitleMenuState* self = (TitleMenuState*)this;
    self->phase = kTitleIntro;
    if (self->texturePack) {
        if (self->texturePack) {
            ((MenuOwnedObject*)self->texturePack)->destroy(1);
        }
        self->texturePack = 0;
    }
    int pairedElementRect[4]; // Layout_ReadElementRect/WriteElementRect move 4 ints
    Layout_ReadElementRect((void*)self->layout, kSinglePlayerButton, pairedElementRect);
    Layout_WriteElementRect((void*)self->layout, kSinglePlayerHighlight, pairedElementRect);
    Layout_ReadElementRect((void*)self->layout, kMultiPlayerButton, pairedElementRect);
    Layout_WriteElementRect((void*)self->layout, kMultiPlayerHighlight, pairedElementRect);
    IFadeMenuBase_StartMenuFadeOut(self);
}

// Initialize this title unit's cached full-circle angle.
void TitleScreen_InitializeAngleCache() {
    s_titleScreenFullCircleAngle = kFloat_6_2831855;
}

// Initialize the multiplayer-menu cached full-circle angle.
void MultiPlayMenu_InitializeAngleCache() {
    s_multiPlayTopMenuFullCircleAngle = kFloat_6_2831855;
}

// out is written only on true. The stored selection code is passed through unchanged;
// its game-mode interpretation belongs to the menu caller.
// Return the multiplayer choice only after its selection-complete flag is set.
bool CMultiPlayTopMenu::TryGetSelection(int* out) {
    if (resultReady) {
        *out = result;
        return true;
    }
    return false;
}

// Report readiness after the title textures, BP entries, and layout finish loading.
int CPangya2TitleTop::PollLoadComplete() {
    TitleMenuState* self = (TitleMenuState*)this;
    if (self->texturePack) {
        if (!((CPackTextureResource*)(self->texturePack))->IsLoadFinished())
            return 0;
    }
    if (!TitleBpControllerSlot()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_titleBpControllerDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        TitleBpControllerSlot() = s_bpCtrl;
    }
    if (!((CBpCtrl*)TitleBpControllerSlot())->PollAllEntriesLoaded())
        return 0;
    if (!Layout_PollLoadComplete((void*)self->layout))
        return 0;
    void* obj = self->texture;
    return ((MenuTextureLoadInterface*)obj)->isLoaded() != 0 ? 1 : 0;
}

// Release the title BP entry, texture pack, layout, and preview texture.
CPangya2TitleTop::~CPangya2TitleTop() {
    TitleMenuState* self = (TitleMenuState*)this;
    *(void**)self = (void*)__vt__16CPangya2TitleTop;
    if (self->bpEntryId != -1) {
        if (!TitleBpControllerSlot()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_titleBpControllerDtorRegistration);
                s_bpCtrlInitialized = 1;
            }
            TitleBpControllerSlot() = s_bpCtrl;
        }
        ((CBpCtrl*)TitleBpControllerSlot())->DeleteEntryById(self->bpEntryId);
        self->bpEntryId = -1;
    }
    if (self->texturePack) {
        if (self->texturePack) {
            ((MenuOwnedObject*)self->texturePack)->destroy(1);
        }
        self->texturePack = 0;
    }
    if (self->layout) {
        Layout_Destroy((void*)self->layout, 1);
        self->layout = 0;
    }
    if (self->texture) {
        if (self->texture) {
            ((MenuOwnedObject*)self->texture)->destroy(1);
        }
        self->texture = 0;
    }
}

// Release the multiplayer background texture and shared BP entry, then destroy menu bases.
// this is null or the COMPLETE CMultiPlayTopMenu object, never its +0x18 base.
// Constructor schema below: MPBase1/IFadeMenuBase at +0 (0x18 bytes), MPBase2/
// CTopMenuBase at +0x18 (0x48 bytes). Install complete/secondary vtables before
// virtual resource destruction. destroy(1) deletes the owned menuTexture; the
// shared BP handle remains global, but ReleaseEntry drops its registered entry.
// __dt__7MPBase2Fv receives the ADJUSTED +0x18 pointer and deleting=0: release
// base-owned menu resources without freeing an interior allocation. No separate
// MPBase1 destructor call is present on this path. Only deleting>0 frees this,
// once, after base cleanup. deleting<=0 leaves the enclosing storage to its owner.
// Returns this (possibly freed); null is a no-op. No rollback/catch if cleanup throws.
void* CMultiPlayTopMenu::Dtor(int deleting) {
    if (this != 0) {
        char* vtBase = (char*)__vt__17CMultiPlayTopMenu;
        void* obj0 = menuTexture;
        vtbl = (void*)vtBase;
        secondaryVtable = (void*)(vtBase + kMenuSecondaryVtableOffset);
        if (obj0 != 0) {
            if (obj0 != 0) {
                ((MenuOwnedObject*)obj0)->destroy(1);
            }
            menuTexture = 0;
        }
        if (!TitleBpHandleSlot()) {
            if (!s_bpCtrlHandleInitialized) {
                s_bpCtrlHandle[0] = -1;
                s_bpCtrlHandle[1] = -1;
                __register_global_object(s_bpCtrlHandle, (void (*)(void*, int))CBpCtrlHandle_Dtor,
                                         s_multiPlayBpHandleDtorRegistration);
                s_bpCtrlHandleInitialized = 1;
            }
            TitleBpHandleSlot() = (void*)s_bpCtrlHandle;
        }
        ((CBpCtrlHandle*)TitleBpHandleSlot())->ReleaseEntry();
        __dt__7MPBase2Fv((char*)((unsigned long)this + kMenuSecondaryBaseOffset), 0);
        if (deleting > 0)
            __dl__FPv(this);
    }
    return this;
}

struct Fn802BCOption {
    char data[0x860];
    // Construct an owned options dialog with the forwarded mode.
    Fn802BCOption(int arg) {
        ((COptionDialog*)this)->__ct(arg);
    }
};

// Priority is script playback, exit, options dialog, then normal menu input.
// dt is elapsed SECONDS, forwarded unchanged through MPBase1_Process
// (event_face_and_mpbase1_menu.cpp) to COptionDialog_UpdateAndProcessInput (option_dialog_control_mode_audio.cpp), where inputDelay -= dt and
// the same delta is passed to its modal save menu. This function accumulates no
// timer itself. Title attract timeout remains a separate update-call count.
// return 1 means exitRequested.
// Process multiplayer menu initialization, options, and normal selection input.
int CMultiPlayTopMenu::UpdateMenu(float dt) {
    char* self;
    int i;
    int exitFlag;
    void* p;
    int zero;
    self = (char*)this;
    exitFlag = 0;
    zero = 0;
    if (scriptRunning) {
        for (i = 0; i < kMenuItemCount; ++i)
            ((CTopMenuBase*)(self + kMenuSecondaryBaseOffset))->SyncItemPosition(i);
        unsigned int __r = (unsigned int)BpObject_IsCurrentScriptComplete(bpObject);
        *(bool*)&scriptRunning = (__r == 0);
    } else if (exitRequested) {
        exitFlag = 1;
    } else if (optionDialogRequested) {
        if (!optionDialog) {
            p = new Fn802BCOption(0);
            optionDialog = p;
        }
        if (MPBase1_Process(optionDialog, dt)) {
            p = optionDialog;
            if (p) {
                if (optionDialog) {
                    ((MenuOwnedObject*)p)->destroy(1);
                }
                optionDialog = 0;
            }
            zero = 0;
            optionDialogRequested = (unsigned char)zero;
        }
    } else {
        Layout_Update(layout);
        ((CTopMenuBase*)(self + kMenuSecondaryBaseOffset))->UpdateHoverMessage(s_multiPlayTopMenuHoverMessage);
        ((CTopMenuBase*)(self + kMenuSecondaryBaseOffset))->UpdateHoverElementPairs();
        ((CTopMenuBase*)(self + kMenuSecondaryBaseOffset))->ProcessInput();
    }
    BpObject_TryInvokeCurrentCallback(bpObject, 0);
    MenuMessage_UpdateReveal(menuMessage);
    return exitFlag;
}

typedef LocalIntVector_u802BADE4 ActivatedMenuElements;

#pragma opt_propagation off
// UI IDs: 0/1 are the single-player button/overlay, 2/3 multiplayer.
// Only the first collected activation is used. Any collected ID locks all four
// elements and returns 1; an unrecognized ID preserves the previous play-mode choice.
// The local activation vector owns its array and frees it on all exits.
// Treat each button/highlight pair as one play-mode choice and lock all four elements.
int CPangya2TitleTop::HandlePairedSelection() {
    TitleMenuState* self = (TitleMenuState*)this;
    Layout_Update((void*)self->layout);
    ActivatedMenuElements list = {0, 0, 0};
    if (Layout_CollectActivated((void*)self->layout, &list.items, 0) != 0) {
        int code = *list.items;
        switch (code) {
        case kSinglePlayerButton:
        case kSinglePlayerHighlight:
            Layout_SetElementValueState((void*)self->layout, kSinglePlayerButton, kSelectedElementValue);
            Layout_SetElementValueState((void*)self->layout, kSinglePlayerHighlight, kSelectedElementValue);
            AsCPangya2TitleTop(self)->singlePlayerSelected = 1;
            break;
        case kMultiPlayerButton:
        case kMultiPlayerHighlight:
            Layout_SetElementValueState((void*)self->layout, kMultiPlayerButton, kSelectedElementValue);
            Layout_SetElementValueState((void*)self->layout, kMultiPlayerHighlight, kSelectedElementValue);
            AsCPangya2TitleTop(self)->singlePlayerSelected = 0;
            break;
        default:
            break;
        }
        Layout_SetElementPlayerFlag((void*)self->layout, kSinglePlayerButton, 0, kLockedElementPlayerFlag);
        Layout_SetElementPlayerFlag((void*)self->layout, kSinglePlayerHighlight, 0, kLockedElementPlayerFlag);
        Layout_SetElementPlayerFlag((void*)self->layout, kMultiPlayerButton, 0, kLockedElementPlayerFlag);
        Layout_SetElementPlayerFlag((void*)self->layout, kMultiPlayerHighlight, 0, kLockedElementPlayerFlag);
        return 1;
    }
    return 0;
}

#pragma opt_propagation reset

struct BpV_ {
    virtual void a0();
    virtual void a1(void*);
};
struct MPNew {
    char storage[0x28]; /* Construct an owned BP texture wrapper. */
    MPNew() {
        ((CBpTex*)this)->__ct();
    }
};
// Construct menu bases, block HOME during entry, and reuse the title's chosen background.
CMultiPlayTopMenu::CMultiPlayTopMenu() {
    // The fade-menu base comes first at +0 and installs its vtable; the top-menu base sits at +0x18.
    MPBase1_Ctor(this);
    *(void**)this = (void*)__vt__13IFadeMenuBase;
    ((CTopMenuBase*)((char*)this + kMenuSecondaryBaseOffset))
        ->__ct(multiPlayTopMenuStrings, multiPlayTopMenuStrings + kMultiPlayMenuStringPool_top_menu_srd,
               multiPlayTopMenuStrings + kMultiPlayMenuStringPool_top_menu_b2d,
               multiPlayTopMenuStrings + kMultiPlayMenuStringPool_topmenu_zip, 5, s_multiPlayMenuLayoutData);
    char* vtBase = (char*)__vt__17CMultiPlayTopMenu;
    vtbl = (void*)vtBase;
    secondaryVtable = (void*)(vtBase + kMenuSecondaryVtableOffset);
    menuTexture = 0;
    if (!TitleHomeMenuSlot()) {
        if (!s_homeMenuInitialized) {
            HomeButtonMenu_InitializeState(s_homeButtonResourceState);
            __register_global_object(s_homeButtonResourceState, (void (*)(void*, int))HomeButtonResources_dtor,
                                     s_multiPlayHomeMenuDtorRegistration);
            s_homeMenuInitialized = 1;
        }
        TitleHomeMenuSlot() = s_homeButtonResourceState;
    }
    HomeMenu_SetBlockedState(TitleHomeMenuSlot(), 0);
    if (!TitleHomeMenuSlot()) {
        if (!s_homeMenuInitialized) {
            HomeButtonMenu_InitializeState(s_homeButtonResourceState);
            __register_global_object(s_homeButtonResourceState, (void (*)(void*, int))HomeButtonResources_dtor,
                                     s_multiPlayHomeMenuDtorRegistration);
            s_homeMenuInitialized = 1;
        }
        TitleHomeMenuSlot() = s_homeButtonResourceState;
    }
    HomeMenu_OverrideBlockedState(TitleHomeMenuSlot(), 1);
    void* obj = new MPNew();
    menuTexture = obj;
    void** table = (void**)coursePreviewTextureNames;
    int idx = *(int*)&AsCSharedDoc(g_sharedDoc)->menuTextureIndex;
    void* previewTextureName = table[idx];
    ((BpV_*)obj)->a1(previewTextureName);
}

#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_common_subs off
// Draw the title background quad, BP scene, and play-mode layout during selection.
// self and its BP group/texture/layout must be ready. Layout position plus source
// dimensions supplies pixel-like UI coordinates; no normalized aspect correction
// happens here. Edges are left/top and left+width/top+height, strip order TL/TR/BL/BR.
// Opacity source is 7-bit fixed point (128 full); the texture and vertex-alpha
// paths intentionally differ for exceptional/out-of-range values. Keep template
// UVs, signed alpha arithmetic and BP callback-before-draw order when modding fades.
void CPangya2TitleTop::DrawBackgroundAndScene() {
    TitleMenuState* self = (TitleMenuState*)this;
    enum {
        kRectWidth = 0,
        kRectHeight = 1,
        kRectLeft = 2,
        kRectTop = 3
    };
    enum {
        kVertexX = 0,
        kVertexY = 4,
        kVertexAlpha = 15
    };
    // Words preserve the original integer zero stores; UIGroup writes float bits.
    int backgroundRect[4];
    // Four 24-byte vertices: XYZ, RGBA (alpha is byte 15), and UV.
    char backgroundVertices[96];
    void* backgroundGroup;
    int groupOpacity;
    int doubledOpacityMinusOne;
    int quadAlpha;
    float bottom;
    float right;
    float top;
    float left;
    int* templateWords;
    int* vertexWords;
    int wordPairsRemaining;
    int primitiveCount;
    BpObject_TryInvokeCurrentCallback(self->bpObject, 0);
    backgroundRect[kRectLeft] = 0;
    backgroundRect[kRectTop] = 0;
    backgroundRect[kRectWidth] = 0;
    backgroundRect[kRectHeight] = 0;
    backgroundGroup =
        BpObject_GetOrCreateNamedGroup(self->bpObject, (void*)(titleStartStrings + kTitleStringPool_r_title_bg));
    UIGroup_GetPosition(backgroundGroup, (void*)&backgroundRect[kRectLeft]);
    UIGroup_GetSourceSize(backgroundGroup, (void*)&backgroundRect[kRectWidth]);
    // The rectangle is in layout coordinates; source size supplies width/height.
    groupOpacity = UIGroup_GetCombinedOpacity(backgroundGroup);
    // For d=2*opacity-1 >= 0, vertex alpha stores the low byte of d, with no
    // upper clamp (opacity=129 gives d=257 and alpha=1). For d=-1, the signed
    // mask gives zero. Keep signed shifts and the final byte conversion.
    // Texture receives 2*opacity-1; vertex alpha keeps the original signed bit-mask
    // expression and byte conversion. Preserve its wrap behavior when editing fades.
    doubledOpacityMinusOne = groupOpacity * 2;
    doubledOpacityMinusOne = doubledOpacityMinusOne - 1;
    quadAlpha = doubledOpacityMinusOne & (((-doubledOpacityMinusOne) & ~doubledOpacityMinusOne) >> 31);
    ((MenuTextureOpacityInterface*)self->texture)->setOpacity(doubledOpacityMinusOne);
    templateWords = (int*)s_titleBackgroundQuadVertices - 1;
    vertexWords = (int*)backgroundVertices - 1;
    wordPairsRemaining = 12;
    do {
        *++vertexWords = *++templateWords;
        *++vertexWords = *++templateWords;
    } while (--wordPairsRemaining);
    // Reconstruct the four strip corners; template Z/color/UV lanes stay intact.
    left = *(float*)&backgroundRect[kRectLeft];
    top = *(float*)&backgroundRect[kRectTop];
    right = *(float*)&backgroundRect[kRectWidth];
    bottom = *(float*)&backgroundRect[kRectHeight];
    right = left + right;
    *(float*)(backgroundVertices + 0 * kQuadVertexStride + kVertexX) = left;
    bottom = top + bottom;
    *(float*)(backgroundVertices + 0 * kQuadVertexStride + kVertexY) = top;
    backgroundVertices[0 * kQuadVertexStride + kVertexAlpha] = (char)quadAlpha;
    *(float*)(backgroundVertices + 1 * kQuadVertexStride + kVertexX) = right;
    *(float*)(backgroundVertices + 1 * kQuadVertexStride + kVertexY) = top;
    backgroundVertices[1 * kQuadVertexStride + kVertexAlpha] = (char)quadAlpha;
    *(float*)(backgroundVertices + 2 * kQuadVertexStride + kVertexX) = left;
    *(float*)(backgroundVertices + 2 * kQuadVertexStride + kVertexY) = bottom;
    backgroundVertices[2 * kQuadVertexStride + kVertexAlpha] = (char)quadAlpha;
    *(float*)(backgroundVertices + 3 * kQuadVertexStride + kVertexX) = right;
    *(float*)(backgroundVertices + 3 * kQuadVertexStride + kVertexY) = bottom;
    backgroundVertices[3 * kQuadVertexStride + kVertexAlpha] = (char)quadAlpha;
    primitiveCount = (int)GX_GetPrimitiveCount(kQuadVertexCount, kTriangleStripPrimitive);
    UIRender_SetDefaultState();
    Engine_SetFlexibleVertexFormat(kTexturedVertexFormat);
    DrawPrimitiveUP(kTriangleStripPrimitive, primitiveCount, backgroundVertices, kQuadVertexStride);
    // Compose the BP scene over the background; draw choices only in phases 3/4.
    BpObject_Draw(self->bpObject);
    if (BpObject_IsCurrentScriptComplete(self->bpObject)) {
        if ((unsigned int)(self->phase - kTitleChoosePlayMode) <= 1) {
            Layout_DrawWithOpacity((void*)self->layout, kTitleFullOpacity);
        }
    }
}
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset

#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma use_lmw_stmw on
// Load title resources and choose one of the fourteen course-preview textures.
// self is required fresh title storage; returns self. Owns texturePack/layout/
// texture; BP registry owns bpObject until the retained bpEntryId is deleted.
// Consumes exactly one rand() value for menuTextureIndex (0..13), shared with
// multiplayer. Allocation-null paths are not safe fallbacks for later dereferences.
// Preserve resource names/IDs, RNG count, initialization order and unwind cleanup.
// Preview-table replacement contract: coursePreviewTextureNames is 14 borrowed
// const-char* entries (32-bit Wii pointers, 4-byte stride, 56 bytes) defined in
// Index -> asset/course stem, preserving the source spellings:
// 0 bluelagoon, 1 bluewater, 2 sepiawind, 3 windhill, 4 wizz, 5 west, 6 moon,
// 7 silvia, 8 icee_01, 9 white, 10 sand, 11 pinkwind, 12 inferno, 13 icespa;
// every entry appends .gsr. These are PREVIEW indices, not course database IDs:
// title chooses rand()%14 and stores menuTextureIndex; multiplayer, single top
// menu (title_main_and_top_menu.cpp), character selection (chara_num_select_and_training_main.cpp) and darts_dialog_and_load_data_menu.cpp reuse it.
// No selected-course lookup occurs here. Replacing entry i changes all those
// backgrounds; retain loadable GSR/palette data and the BP source rectangle/UV
// contract in DrawBackgroundAndScene. Preserve all 14 slots and the RNG call when swapping art.
CPangya2TitleTop::CPangya2TitleTop() {
    char* registrationStorage = s_titleInputManagerDtorRegistration;
    MPBase1_Ctor(this);
    *(void**)this = (void*)__vt__16CPangya2TitleTop;
    selectionComplete = 0;
    introStarted = 0;
    bpEntryId = -1;
    singlePlayerSelected = 1;
    layout = 0;
    texturePack = 0;
    texture = 0;
    // Phase 1: resolve the title archive and create its owned texture-pack wrapper.
    int archiveEntryInfo = 0;
    if (!Sing_805DC620()) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, (void (*)(void*, int))Allocation_Dtor,
                                     (void*)((registrationStorage + kResourceTableRegistrationOffset)));
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }
    void* ownedResource;
    void* archiveEntry;
    archiveEntry = ResourceTable_FindEntryByName(Sing_805DC620(), (void*)titleStartStrings, &archiveEntryInfo);
    ownedResource = __nw__FUl(kTitlePackTextureBytes);
    if (ownedResource) {
        try {
            ((CPackTextureResource*)ownedResource)
                ->Init((const char*)titleStartStrings, (void**)archiveEntryInfo, (unsigned int)archiveEntry);
        } catch (...) {
            __dl__FPv(ownedResource);
            throw;
        }
    }
    (*(void**)&texturePack) = ownedResource;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, (registrationStorage + kBpControllerRegistrationOffset));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry((void*)(titleStartStrings + kTitleStringPool_p2_title_start_b2d), &bpEntryId, 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, (registrationStorage + kBpControllerRegistrationOffset));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    // Phase 2: retain the BP entry ID and borrow its script object from the registry.
    void* scriptObject = IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
    bpObject = scriptObject;
    idleUpdateCount = 0;
    ((CProjectG*)g_project)->SetExtensionProperty(kMenuExtensionHidden);
    if (!Sing_805DC820()) {
        if (!s_homeMenuInitialized) {
            HomeButtonMenu_InitializeState(s_homeButtonResourceState);
            __register_global_object(s_homeButtonResourceState, (void (*)(void*, int))HomeButtonResources_dtor,
                                     (registrationStorage + kHomeResourcesRegistrationOffset));
            s_homeMenuInitialized = 1;
        }
        Sing_805DC820() = s_homeButtonResourceState;
    }
    HomeMenu_SetBlockedState(Sing_805DC820(), 0);
    ownedResource = __nw__FUl(kTitleLayoutBytes);
    if (ownedResource)
        Layout_Init(ownedResource, kTitleLayoutKind);
    (*(void**)&layout) = ownedResource;
    Layout_LoadExtended(ownedResource, 0, (void*)(titleStartStrings + kTitleStringPool_p2_title_start_spd));
    // Phase 3: publish one of 14 preview indices for the title and multiplayer menus.
    unsigned int randomPreviewValue = (unsigned int)rand();
    *(int*)&AsCSharedDoc(g_sharedDoc)->menuTextureIndex = (int)(randomPreviewValue % kCoursePreviewCount);
    ownedResource = __nw__FUl(kTitleTextureBytes);
    if (ownedResource)
        ((CBpTex*)ownedResource)->__ct();
    (*(void**)&texture) = ownedResource;
    int previewIndex = *(int*)&AsCSharedDoc(g_sharedDoc)->menuTextureIndex;
    void* previewTextureName = ((void**)coursePreviewTextureNames)[previewIndex];
    ((BpV_*)ownedResource)->a1(previewTextureName);
}
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804624D4", _ctors_u802BADE4_0, TitleScreen_InitializeAngleCache);
STATIC_INIT_ENTRY(".CRT$XCU804624D4", _ctors_u802BADE4_1, MultiPlayMenu_InitializeAngleCache);
#pragma pop
