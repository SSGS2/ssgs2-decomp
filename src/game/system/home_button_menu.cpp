// The Wii HOME menu: lifecycle, controller admission, the render state it draws with, and localized
// resource loading. Also the shared-file registry and its destruction.
// HomeButtonMenu_ProcessState runs from the game update; HomeButtonMenu_HandleResult acts on the menu result.

#include <decomp/static_init.h>
// Wii HOME menu lifecycle, controller admission, render-state backup and localized
// resource loading; also shared-file registry destruction. Game update calls
// HomeButtonMenu_ProcessState; HBMCalc/HBMDraw borrow this wrapper's buffers.
// Modding lookup:
// - Classic HOME admission: ProcessState's WPADProbe gate and action 0x13 loop.
//   Stock logic requires a successful probe before testing all four action channels.
// - Aspect ratio: Render_SaveStateAndSetupOrthographic2D uses centered 608x456
//   HOME drawing units (4:3), near 0/far 500, from the data section. This is
//   an orthographic rectangle, not the framebuffer or GX viewport/scissor.
// - Frame rate/audio: kHomePauseAudioDelayUpdates is nine UPDATE calls; the
//   separate HBM create frameStep is a leading float 2.0 in an eight-byte slot.
// - Textures: HomeMenu_LoadResourceBuffers selects localized archives and binds
//   the icon TPL. Format/dimensions belong to those assets and HBM's draw code.
// Preserve save/setup/draw/restore order and controller sample/descriptor strides.
// HOME input gate-word layout and release of four retained resource copies have
// unresolved contracts documented below; changing them requires further evidence.
// Evidence boundary for those three modding questions: src/include provide no
// implementation of HomeMenu_Create, HBMCalc or HomeMenu_Delete. Their ABI names
// alone cannot establish count layout, buffer transfer, or frame-step time base.
// Keep this file below readability A until those library contracts are recovered.
extern "C" {
extern unsigned char s_homeButtonSoundState[];
}
#include "MSL/vector.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
typedef std::vector<void*, app::app_allocator<void*> > MSLPointerVector;
#include "classes/GameInputManager.h"
// HOME menu lifecycle and common Wii controller limits.
enum HomeMenuState {
    kHomeIdle = 0,
    kHomeInitialize = 1,
    kHomeActive = 2
};
enum HomeMenuConstants {
    kHomeAction = 0x13,
    kControllerCount = 4,
    kSamplesPerController = 16,
    kKpadSampleBytes = 0x84,
    kControllerSampleBytes = 0x840,
    kHomePauseAudioDelayUpdates = 9,
    kInvalidScriptEntry = -1
};
enum HomeInputStrides {
    kHomeDescriptorStride = 0x10,
    kHomeProbeWordStride = 4,
    kHomeDescriptorPairStride = 0x20,
    kHomeProbePairStride = 8,
    kHomeControllerPairCount = kControllerCount / 2
};
enum HomeProbeResult {
    kHomeProbeOk = 0,
    kHomeProbeMinus2 = -2,
    kHomeProbeMinus3 = -3
};
enum HomeSampleField {
    kHomeSampleModeByte = 0x5C,
    kHomeSampleStatusByte = 0x5D,
    kHomeExtensionMode2 = 2,
    kHomeNextControllerModeByte = kControllerSampleBytes + kHomeSampleModeByte
};
// Observed HBM input descriptor, at owner+0x40 with a 0x10-byte stride. Fields
// +4/+8 are not identified here; the sample count used by this code lives elsewhere.
struct HomePadDescriptorView {
    void* samples;
    unsigned int unresolved04, unresolved08;
    unsigned int extensionMode;
};
// First KPAD sample's observed prefix; the two bytes survive a -2 probe reset.
// Their full SDK meanings are unresolved locally; only mode value 2 is tested.
struct HomePadSamplePrefix {
    char payload[0x5C];
    unsigned char mode, status;
};
// Two overlapping interpretations are required by the actual accesses. KPADRead
// results are written every 16 bytes, but extension checks consume adjacent words.
// Only resultWords[0] is known to contain this call's controller-0 read count.
struct HomeKpadResultSlot {
    int readResult;
    char unusedWords[12];
};
struct HomeInputWordsView {
    char prefix[0x120];
    union {
        HomeKpadResultSlot kpadResultSlots[kControllerCount];
        int extensionGateWords[kControllerCount];
    } resultWords;
};
typedef char HomeKpadResultOffsetCheck[(int)&((HomeInputWordsView*)0)->resultWords == 0x120 ? 1 : -1];
// Context8028A588 is also used as a moving view, not always the owner base:
// per-controller stores: probe status = +0xE0+4*i, probe type = +0xF0+4*i,
// descriptor = +0x40+0x10*i, sample count = +0x120+0x10*i.
// Paired checks instead read counts at +0x120/+0x124 then +0x128/+0x12C while
// descriptors advance by 0x20. Concrete access matrix, relative to owner self:
//   controller             0       1       2       3
//   descriptor           +040    +050    +060    +070
//   probe status         +0e0    +0e4    +0e8    +0ec
//   probe type           +0f0    +0f4    +0f8    +0fc
//   count WRITE          +120    +130    +140    +150
//   paired count READ    +120    +124    +128    +12c
// Only controller 0's count addresses coincide. Nothing here proves that the
// other reads hold counts written by KPADRead in this call. The discrepancy is
// in the compiled behavior, not a license to choose one array stride. A change
// to controller extent/count handling needs a separate layout/caller audit.
// Sample data is separate: moduleStorage+0x30 + controller*0x840, 16*0x84 bytes.
// Source-only audit boundary: HomeButtonMenu_InitializeState (small_common_dialog.cpp) does
// not initialize +124/+128/+12C; this active loop does not write them. HBMCalc
// receives self+40, not this result area. No visible producer establishes that
// these three words equal controllers 1..3's fresh counts. Therefore the exact
// explainable rule is: extension reset for channel i requires gateWord[i]>0,
// probe success, a sample pointer, previous extensionMode==2 and sample mode!=2.
// For i=0 that gate is a fresh count; for i=1..3 it is retained storage, not a
// count proven by the producer. Descriptors and actual KPADRead calls still cover
// all four controllers; only extension-reset admission has this unresolved gate.
// Do not reinterpret the union as a coherent count array. Establish the missing
// producer or audit the original library boundary before a controller-stride mod.
// These words do NOT control idle HOME-button admission: that uses successful
// WPADProbe plus action 0x13 before the active sample loop. Their only visible
// use is active extension-mode reset. No conclusion about physical channel
// connectivity may be drawn from gateWords[1..3]. A safe future layout fix must
// first establish a producer; simply substituting the freshly written +130/+140/
// +150 counts would CHANGE this admission policy and break matching.
enum HomeBackupOffset {
    kHomeProjectionBackup = 0,
    kHomeTransform0Backup = 0x40,
    kHomeTransform11Backup = 0x80,
    kHomeRenderStateBackup = 0xC0,
    kHomeTextureStateBackup = 0x100,
    kHomeBackupStorageOffset = 0x2130
};
// Engine-local state IDs, verified against Engine_SetCachedRenderState's
// dispatch (gx_renderer_state_and_efb_copy.cpp) and Render_Apply* consumers (gx_render_state_and_texture_upload.cpp). GX values are
// separate: compare 3=LEQUAL, compare 7=ALWAYS, cull 0=NONE, factors 0=ZERO.
enum HomeRenderStateId {
    kHomeCullMode = 0,
    kHomeDepthCompare = 3,
    kHomeAlphaCompare = 4,
    kHomeDepthWrite = 5,
    kHomeDepthTest = 6,
    kHomeBlendEnabled = 10,
    kHomeSourceBlend = 13,
    kHomeDestinationBlend = 14,
    kHomeMaterialColorSource = 18,
    kHomeLightingChannels = 31
};
enum HomeTextureStateId {
    kHomeColorOperation = 0,
    kHomeColorArgument1 = 1,
    kHomeAlphaOperation = 3,
    kHomeAlphaArgument1 = 4
};
enum HomeRenderSetup {
    kHomeVertexFormat = 0xF001,
    kHomeSelectTextureArgument1 = 0x11,
    kHomeCompareLessEqual = 3,
    kHomeCompareAlways = 7,
    kHomeProjectionTransform = 1,
    kHomeViewTransform = 0,
    kHomeTextureTransform = 11,
    kHomeResetGXOption = 7
};
// Render backup map (0x120 bytes): transforms 1/0/11 at +0/+0x40/+0x80;
// eight {stateId,value} pairs at +0xC0; four stage-0 pairs at +0x100.
// IDs are supplied by preinitialized pair tables. Save replaces only values.
// The draw path restores those pairs/transforms, then explicitly sets option 7=1
// and cached states 0x12=0, 4=7. Vertex format/index binding and other setup state
// are not restored by this snapshot; callers rely on the next engine setup.
struct SharedBufferVectorView {
    void* data;
    int count;
    int capacity;
};
struct SharedTableEntryView {
    unsigned int key;
    void* value;
};
struct BpDrawEnableView {
    char reserved[0x30];
    int drawEnabled;
};
// Fields initialized for the Wii HOME library; unobserved option words stay neutral.
struct HomeCreateInfoView {
    void* layoutArchive;
    void* speakerArchive;
    void* messages;
    void* configuration;
    void* workMemory;
    int option14, option18, language, option20;
    void* callback;
    int configurationBytes, option2C;
    float frameStep, adjustX, adjustY;
};
enum {
    kHomeConfigSizeOffset = (int)&((HomeCreateInfoView*)0)->configurationBytes
};

#include "classes/Context8028A588.h"
#include "classes/ResourceState8028A270.h"
// Borrow the HOME owner's partial view, including explicitly documented byte cursors.
static inline Context8028A588* AsContext8028A588(void* object) {
    return (Context8028A588*)object;
}
// Compare borrowed values without changing their iterator representation.
template <class T> inline bool same_pos(const T& lhs, const T& rhs) {
    return lhs == rhs;
}
// Return the same equality result as the integer form expected by callers.
template <class T> inline int same_int(const T& lhs, const T& rhs) {
    return lhs == rhs;
}

#include "game/singletons.h"
#include "classes/CSharedFileRegistry.h"
#include "classes/CFx.h"
#include "classes/CMouseCursor.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CGallery.h"
#include "classes/CSharedFilePtr.h"
#include "classes/CBpCtrl.h"

// Preserve the compiler's byte-address expression for destructor subobject checks.
enum {
    kFileRecordsOffset = (int)&((CSharedFileRegistry*)0)->fileRecords,
    kFilePointersOffset = (int)&((CSharedFileRegistry*)0)->filePointers
};
struct CSharedFileElem {
    virtual void vdtor(int flag);
};

extern "C" {
extern int SCGetLanguage();
extern void strcat(char*, const char*);
extern char* strcpy(char*, const char*);
extern void* SharedFile_Create(char*, int);
extern int CSharedFilePtr_GetLoadedBuffer(void*);
extern void* appMallocUni__FUl(unsigned long);
extern void* memcpy(void*, const void*, unsigned int);
extern void Game_DrawReloadingFrame();
extern void TPLBind(void*);
extern float kFloat_1_FileDialog, s_homeMenuFrameStepWithTrailingWord;
extern const float kFloat_6_2831855;
extern float s_homeButtonMenuTwoPi;
extern float s_sharedFileRegistryTwoPi;
extern void* g_homeButtonIconTpl;
extern void* Table_GetEntry(void*, int);
extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);
extern void HomeMenu_DeleteSound();
extern void HomeMenu_Delete();
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_homeMenuInputStorage[];
extern const char homeButtonResourceStrings[];

// Packed external string pool; member extents come from the global catalog.
struct HomeResourcePool {
    char soundArchive[17];
    char unavailableLayout[13];
    char defaultArchive[12];
    char englishArchive[16];
    char germanArchive[16];
    char frenchArchive[16];
    char spanishArchive[16];
    char italianArchive[16];
    char dutchArchive[16];
    char speakerArchive[14];
    char messageTable[9];
    char configuration[11];
    char iconTextures[16];
    char unavailableScript[20];
};
// Borrow the packed HOME resource paths without copying or changing pooled storage.
static inline HomeResourcePool* HomeResources() {
    return (HomeResourcePool*)homeButtonResourceStrings;
}

extern char __vt__19CSharedFileRegistry[];
extern void __ct__7CBpCtrlFv(void*);
extern void CBpCtrl_dtor(void*, int);
extern void __register_global_object(void*, void (*)(void*, int), void*);
extern void* IdMap_Lookup(void*, int);
extern int BpObject_PollLoad(void*);
extern int BpObject_IsCurrentScriptComplete(void*);
extern void BpObject_StartScriptByName(void*, const char*, int);
extern void BpObject_TryInvokeCurrentCallback(void*, int);
extern void BpObject_Draw(void*);
extern void Engine_GetTransform(int, void*);
extern void Engine_GetTextureStageState(int, void*, void*);
extern void Engine_GetRenderState(void*, void*);
extern void Engine_SetFlexibleVertexFormat(int);
extern void Engine_SetIndexBuffer(int, int);
extern void Engine_SetTextureStageState(int, int, int);
extern void Engine_SetCachedRenderState(int, int);
extern void MtxOrthoOffCenter(float*, float, float, float, float, float, float);
extern void SetTransform(int, float*);
extern void Engine_FlushRenderState();
extern float kFloat_neg304_HomeMenuProjection, kFloat_304_HomeMenuProjection, kFloat_neg228_HomeMenuProjection,
    kFloat_228_HomeMenuProjection, kFloat_0_FileDialog, kFloat_500_HomeMenuProjectionFar;
extern float kIdentityMatrix4x4[];
extern signed char s_systemResetStateInitialized;
extern void* g_soundManager;
extern void* g_mouseCursor;
extern void* g_fx;
extern signed char s_inputManagerInitialized;
extern signed char s_movieControllerInitialized;
extern char s_systemResetStorage[];
extern char s_gameInputManager[];
extern char s_movieController[];
extern const float kFloat_0_3_HomeMenuVolumeTransition;
extern void SystemResetState_Ctor(void*);
extern void SystemResetState_Dtor(void*, int);
extern void SystemReset_RequestDeferredMode(void*, int);
extern void SoundManager_ResumePrimaryStreams(void*);
extern void SoundManager_ResumePlayback(void*);
extern void SoundManager_SetStreamVolume(void*, float, float);
extern void SoundManager_SetBufferVolume(void*, float, float);
extern void ValueAnimation_SetUpdateMode4();
extern void InputManager_Init(void*);
extern void InputManager_Dtor(void*, int);
extern void InputManager_ResumeRumble(void*);
extern void MovieController_Ctor(void*);
extern void MovieController_Dtor(void*, int);
extern void MovieController_ReleasePlaybackReference(void*);
extern void RemoteAudio_RequestEnableAllLazyForwarder(void*);
extern void CBpCtrl_dtor(void*, int);
extern int HomeButtonMenu_DisableRemoteAudio(void*);
extern void HBMInit();
extern void HBMSetAdjustFlag(int);
extern void MovieController_AcquirePlaybackReference(void*);
extern float SoundManager_GetVolumeByMode(void*, int);
extern float SoundValuePair_GetSelectedValue(void*, int);
extern void ValueAnimation_SetUpdateMode1();
extern int WPADProbe(int, void*);
extern int InputManager_IsAnyMenuAdvanceActionActive(void*);
extern int Input_IsActionPressed(void*, int, int);
extern void InputManager_PauseRumble(void*);
extern int KPADRead(int, void*, int);
extern void* memset(void*, int, unsigned int);
extern int HBMCalc(void*);
extern int HBMGetSelectBtnNum();
extern void HBMUpdateSound();
extern void HBMDraw();
extern void Engine_SetRenderStateList(void*, int);
extern void Engine_SetTextureStageStateList(int, void*, int);
extern void Render_SetSystemOption(int, int);
extern void SoundManager_PausePlayback(void*);
extern void SoundManager_PausePrimaryStreams(void*);
extern void HomeButtonMenu_HandleResult(void*, int);
extern void Render_SaveStateAndSetupOrthographic2D(void*, char*);
extern void HomeButtonMenu_UpdateScript(void*);
extern void __ct__7CBpCtrlFv(void*);
extern void CBpCtrl_dtor(void*, int);
extern void InputManager_Dtor(void*, int);
extern void CBpCtrl_dtor(void*, int);
extern void InputManager_ResumeRumble(void*);
extern void __ct__7CBpCtrlFv(void*);
extern void __register_global_object(void*, void (*)(void*, int), void*);

// Handle HOME requests, pause game services, sample four remotes, run the menu, and restore render state.
// Called once per game update; returns 1 only while state 2 is active. A request
// found in idle moves to state 1 now; HBM initialization occurs on the next call.
// Pausing owns one movie playback reference until the ordinary resume result.
// self required and resources preloaded; active HBM borrows four 16-sample
// buffers in module storage. No asynchronous storage may move before shutdown.
// action ordering and initialize->active fallthrough when adding other devices.
int HomeButtonMenu_ProcessState(char* self) {
    // State routes: initialize -> active below in the same call; idle can only
    // request initialize for the NEXT call. Active sampling never runs for a
    // merely accepted idle request. HandleResult may set idle before draw.
    char* base = s_homeMenuInputStorage;
    if (AsContext8028A588(self)->state == kHomeInitialize) {
        // Phase 1 (one update): initialize HBM, save volumes, begin their fade,
        // pause effects/pointer control and acquire the movie playback reference.
        HomeButtonMenu_DisableRemoteAudio(g_soundManager);
        HBMInit();
        AsContext8028A588(self)->state = kHomeActive;
        CMouseCursor::SetPointerInputEnabled(g_mouseCursor, 0);
        AsContext8028A588(self)->savedStreamVolume = SoundManager_GetVolumeByMode(g_soundManager, 1);
        SoundManager_SetStreamVolume(g_soundManager, kFloat_0_FileDialog, kFloat_0_3_HomeMenuVolumeTransition);
        AsContext8028A588(self)->savedBufferVolume = SoundValuePair_GetSelectedValue(g_soundManager, 1);
        SoundManager_SetBufferVolume(g_soundManager, kFloat_0_FileDialog, kFloat_0_3_HomeMenuVolumeTransition);
        AsContext8028A588(self)->soundStopDelayFrames = kHomePauseAudioDelayUpdates;
        ((CFx*)g_fx)->SetPaused(1);
        HBMSetAdjustFlag((unsigned char)AsContext8028A588(self)->screenAdjustmentEnabled);
        ValueAnimation_SetUpdateMode1();
        if (!Sing_805DC950()) {
            if (!s_movieControllerInitialized) {
                MovieController_Ctor(s_movieController);
                __register_global_object(s_movieController, MovieController_Dtor, base + 0x24);
                s_movieControllerInitialized = 1;
            }
            Sing_805DC950() = s_movieController;
        }
        MovieController_AcquirePlaybackReference(Sing_805DC950());
    } else if (AsContext8028A588(self)->state == kHomeIdle) {
        // Phase 2 (idle): gate HOME on connected-controller/action availability.
        // A blocked request opens the unavailable script instead of activating HBM.
        int padTransitionMode = 0;
        int requestHome = 0;
        int actionPlayer;
        void* input;
        char* probeTypeCursor = self + 0xF0;
        char* probeStatusCursor = self;
        int homePressed;
        for (int i = 0; i < kControllerCount; i++, probeStatusCursor += 4, probeTypeCursor += 4) {
            int id = WPADProbe(i, probeTypeCursor);
            AsContext8028A588(probeStatusCursor)->padProbeStatus = id;
            if (id == 0) {
                if (!GetInputManagerSlot()) {
                    if (!s_inputManagerInitialized) {
                        InputManager_Init(s_gameInputManager);
                        __register_global_object(s_gameInputManager, InputManager_Dtor, base + 0x0C);
                        s_inputManagerInitialized = 1;
                    }
                    GetInputManagerSlot() = s_gameInputManager;
                }
                input = GetInputManagerSlot();
                homePressed = 0;
                actionPlayer = 0;
                for (;;) {
                    if (InputManager_IsAnyMenuAdvanceActionActive(input)) {
                        homePressed = 0;
                        break;
                    }
                    if (Input_IsActionPressed(input, kHomeAction, actionPlayer)) {
                        homePressed = 1;
                        break;
                    }
                    actionPlayer++;
                    if (actionPlayer >= kControllerCount) {
                        homePressed = 0;
                        break;
                    }
                }
                if (homePressed != 0) {
                    if ((unsigned char)AsContext8028A588(self)->homeMenuBlocked != 0) {
                        if ((unsigned char)AsContext8028A588(self)->blockedMessageSuppressed == 0 &&
                            AsContext8028A588(self)->scriptEntryId == -1) {
                            if (!Sing_805DC5F0()) {
                                if (!s_bpCtrlInitialized) {
                                    ((CBpCtrl*)s_bpCtrl)->__ct();
                                    __register_global_object(s_bpCtrl, CBpCtrl_dtor, base);
                                    s_bpCtrlInitialized = 1;
                                }
                                Sing_805DC5F0() = s_bpCtrl;
                            }
                            ((CBpCtrl*)Sing_805DC5F0())
                                ->CreateEntry(HomeResources()->unavailableLayout, (int*)(self + 0xC4), 0);
                        }
                    } else {
                        requestHome = 1;
                        padTransitionMode = 0;
                    }
                    break;
                }
            }
        }
        if (requestHome) {
            // Defer activation while the unavailable script is still running.
            int scriptRunning;
            if (AsContext8028A588(self)->scriptEntryId == -1) {
                scriptRunning = 0;
            } else {
                if (!Sing_805DC5F0()) {
                    if (!s_bpCtrlInitialized) {
                        ((CBpCtrl*)s_bpCtrl)->__ct();
                        __register_global_object(s_bpCtrl, CBpCtrl_dtor, base);
                        s_bpCtrlInitialized = 1;
                    }
                    Sing_805DC5F0() = s_bpCtrl;
                }
                void* scriptObject = IdMap_Lookup(Sing_805DC5F0(), AsContext8028A588(self)->scriptEntryId);
                scriptRunning = (scriptObject != 0) && (BpObject_PollLoad(scriptObject) != 0) &&
                                (BpObject_IsCurrentScriptComplete(scriptObject) == 0);
            }
            if (!scriptRunning) {
                char* pad = base + 0x30;
                for (int j = 0; j < kControllerCount; j++) {
                    if (*(int*)&AsContext8028A588(self + j * 0x10)->padSamples != 0) {
                        if (padTransitionMode == 2 && (unsigned char)pad[0x5C + j * kControllerSampleBytes] == 2)
                            *(int*)&AsContext8028A588(self + j * 0x10)->padExtensionMode = 2;
                        else
                            *(int*)&AsContext8028A588(self + j * 0x10)->padExtensionMode = 0;
                    }
                }
                padTransitionMode = 1;
                AsContext8028A588(self)->state = padTransitionMode;
                if (!Sing_805DC5F0()) {
                    if (!s_bpCtrlInitialized) {
                        ((CBpCtrl*)s_bpCtrl)->__ct();
                        __register_global_object(s_bpCtrl, CBpCtrl_dtor, base);
                        s_bpCtrlInitialized = 1;
                    }
                    Sing_805DC5F0() = s_bpCtrl;
                }
                ((CBpCtrl*)Sing_805DC5F0())->RequestCallbackPause();
                if (!GetInputManagerSlot()) {
                    if (!s_inputManagerInitialized) {
                        InputManager_Init(s_gameInputManager);
                        __register_global_object(s_gameInputManager, InputManager_Dtor, base + 0x0C);
                        s_inputManagerInitialized = 1;
                    }
                    GetInputManagerSlot() = s_gameInputManager;
                }
                // Once accepted, pause BP callbacks/rumble and block secondary
                // input actions 0..38; HandleResult's resume branch clears them.
                InputManager_PauseRumble(GetInputManagerSlot());
                if (!GetInputManagerSlot()) {
                    if (!s_inputManagerInitialized) {
                        InputManager_Init(s_gameInputManager);
                        __register_global_object(s_gameInputManager, InputManager_Dtor, base + 0x0C);
                        s_inputManagerInitialized = 1;
                    }
                    GetInputManagerSlot() = s_gameInputManager;
                }
                void* _inp = GetInputManagerSlot();
                ((GameInputManager*)_inp)->actionBlockedSecondary[0] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[1] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[2] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[3] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[4] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[5] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[6] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[7] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[8] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[9] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[10] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[11] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[12] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[13] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[14] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[15] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[16] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[17] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[18] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[19] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[20] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[21] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[22] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[23] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[24] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[25] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[26] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[27] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[28] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[29] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[30] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[31] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[32] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[33] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[34] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[35] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[36] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[37] = 1;
                ((GameInputManager*)_inp)->actionBlockedSecondary[38] = 1;
            }
        }
    }
    if (AsContext8028A588(self)->state == kHomeActive) {
        // Phase 3 (active): wait nine update ticks for the volume fade before
        // pausing game audio. Sample up to sixteen 0x84-byte records per remote.
        int timer = AsContext8028A588(self)->soundStopDelayFrames;
        if (timer > 0) {
            timer--;
            AsContext8028A588(self)->soundStopDelayFrames = timer;
            if (!timer) {
                SoundManager_PausePlayback(g_soundManager);
                SoundManager_PausePrimaryStreams(g_soundManager);
            }
        }
        char* probeStatusCursor = self;
        char* padDescriptorCursor = self;
        char* probeTypeCursor = self + 0xF0;
        char* pad = base + 0x30;
        for (int i = 0; i < kControllerCount; i++, probeStatusCursor += kHomeProbeWordStride,
                 padDescriptorCursor += kHomeDescriptorStride, probeTypeCursor += kHomeProbeWordStride,
                 pad += kControllerSampleBytes) {
            int probeResult = WPADProbe(i, probeTypeCursor);
            AsContext8028A588(probeStatusCursor)->padProbeStatus = probeResult;
            int sampleCount = KPADRead(i, pad, kSamplesPerController);
            ((HomeInputWordsView*)padDescriptorCursor)->resultWords.kpadResultSlots[0].readResult = sampleCount;
            switch (AsContext8028A588(probeStatusCursor)->padProbeStatus) {
            case kHomeProbeMinus2: {
                // -2 clears only the first sample, retaining its mode/status
                // bytes, then falls through to supply its pointer to HBM.
                unsigned char savedMode = (unsigned char)pad[kHomeSampleModeByte];
                unsigned char savedStatus = (unsigned char)pad[kHomeSampleStatusByte];
                memset(pad, 0, kKpadSampleBytes);
                pad[kHomeSampleModeByte] = savedMode;
                pad[kHomeSampleStatusByte] = savedStatus;
            }
            case kHomeProbeMinus3:
            case kHomeProbeOk:
                // -3 and success also expose sample storage; all other statuses
                // suppress that controller by publishing a null descriptor pointer.
                AsContext8028A588(padDescriptorCursor)->padSamples = pad;
                break;
            default:
                AsContext8028A588(padDescriptorCursor)->padSamples = 0;
                break;
            }
        }
        // Phase 4: reconcile extension mode for two controller pairs. Keep the
        // gate-word/status views distinct from the descriptor/sample strides.
        char* pairedProbeCursor = self;
        char* pairedDescriptorCursor = self;
        char* pairedSamples = base + 0x30;
        for (int k = 0; k < kHomeControllerPairCount; k++, pairedProbeCursor += kHomeProbePairStride,
                 pairedDescriptorCursor += kHomeDescriptorPairStride, pairedSamples += (2 * kControllerSampleBytes)) {
            // Pair member 0: status/gate word from the word cursor, descriptor from
            // the descriptor cursor, mode byte from separate KPAD sample storage.
            if (AsContext8028A588(pairedProbeCursor)->padProbeStatus == kHomeProbeOk &&
                ((HomeInputWordsView*)pairedProbeCursor)->resultWords.extensionGateWords[0] > 0 &&
                AsContext8028A588(pairedDescriptorCursor)->padSamples != 0 &&
                AsContext8028A588(pairedDescriptorCursor)->padExtensionMode == kHomeExtensionMode2 &&
                (unsigned char)pairedSamples[kHomeSampleModeByte] != kHomeExtensionMode2)
                (*(int*)&AsContext8028A588(pairedDescriptorCursor)->padExtensionMode) = 0;
            // Pair member 1: +4 status/gate word, +0x10 descriptor, +0x840 sample.
            if (AsContext8028A588(pairedProbeCursor)->nextPadProbeStatus == kHomeProbeOk &&
                ((HomeInputWordsView*)pairedProbeCursor)->resultWords.extensionGateWords[1] > 0 &&
                AsContext8028A588(pairedDescriptorCursor)->nextPadSamples != 0 &&
                AsContext8028A588(pairedDescriptorCursor)->nextPadExtensionMode == kHomeExtensionMode2 &&
                (unsigned char)pairedSamples[kHomeNextControllerModeByte] != kHomeExtensionMode2)
                (*(int*)&AsContext8028A588(pairedDescriptorCursor)->nextPadExtensionMode) = 0;
        }
        // Phase 5: hand four borrowed descriptors to HBM, then process a returned
        // selection. Result handling may change state before the drawing test below.
        int menuResult = HBMCalc(
            (char*)&AsContext8028A588(self)->padSamples); // HomeButtonMenu_ProcessState: first pad descriptor at +0x40
        if (menuResult >= 0) {
            int v = HBMGetSelectBtnNum();
            HomeButtonMenu_HandleResult(self, v);
        }
        HBMUpdateSound();
    }
    if (AsContext8028A588(self)->state == kHomeActive) {
        // Phase 6: save/setup/draw/restore using HomeBackupOffset's map above.
        Render_SaveStateAndSetupOrthographic2D(self, ((char*)(base + kHomeBackupStorageOffset)));
        HBMDraw();
        SetTransform(1, (float*)(((char*)(base + kHomeBackupStorageOffset))));
        char* mtx = ((char*)(base + kHomeBackupStorageOffset));
        SetTransform(0, (float*)(mtx + kHomeTransform0Backup));
        SetTransform(0xB, (float*)(mtx + kHomeTransform11Backup));
        Engine_SetRenderStateList(mtx + kHomeRenderStateBackup, 8);
        Engine_SetTextureStageStateList(0, mtx + kHomeTextureStateBackup, 4);
        Render_SetSystemOption(kHomeResetGXOption, 1);
        Engine_SetCachedRenderState(kHomeMaterialColorSource, 0);
        Engine_SetCachedRenderState(kHomeAlphaCompare, kHomeCompareAlways);
    }
    if (AsContext8028A588(self)->scriptEntryId >= 0)
        HomeButtonMenu_UpdateScript(self);
    return AsContext8028A588(self)->state == kHomeActive;
}

// Queue reset/menu actions or resume paused services, clear input blocks, and return HOME to idle.
void HomeButtonMenu_HandleResult(void* selfRaw, int mode) {
    // Results 1/2 queue reset modes 2/0; 3/4 skip service restoration here.
    // Other results resume audio/effects/input and release the movie reference.
    // Every branch resumes BP callbacks and returns the local state to idle.
    char* self = (char*)selfRaw;
    char* moduleStorage = s_homeMenuInputStorage;
    switch (mode) {
    case 1: {
        if (!Sing_805DC948()) {
            if (!s_systemResetStateInitialized) {
                SystemResetState_Ctor(s_systemResetStorage);
                __register_global_object(s_systemResetStorage, SystemResetState_Dtor, moduleStorage + 0x18);
                s_systemResetStateInitialized = 1;
            }
            Sing_805DC948() = s_systemResetStorage;
        }
        SystemReset_RequestDeferredMode(Sing_805DC948(), 2);
        break;
    }
    case 2: {
        if (!Sing_805DC948()) {
            if (!s_systemResetStateInitialized) {
                SystemResetState_Ctor(s_systemResetStorage);
                __register_global_object(s_systemResetStorage, SystemResetState_Dtor, moduleStorage + 0x18);
                s_systemResetStateInitialized = 1;
            }
            Sing_805DC948() = s_systemResetStorage;
        }
        SystemReset_RequestDeferredMode(Sing_805DC948(), 0);
        break;
    }
    case 3:
    case 4:
        break;
    default:
        SoundManager_ResumePrimaryStreams(g_soundManager);
        SoundManager_ResumePlayback(g_soundManager);
        SoundManager_SetStreamVolume(g_soundManager, AsContext8028A588(self)->savedStreamVolume,
                                     kFloat_0_3_HomeMenuVolumeTransition);
        SoundManager_SetBufferVolume(g_soundManager, AsContext8028A588(self)->savedBufferVolume,
                                     kFloat_0_3_HomeMenuVolumeTransition);
        CMouseCursor::SetPointerInputEnabled(g_mouseCursor, 1);
        ((CFx*)g_fx)->SetPaused(0);
        ValueAnimation_SetUpdateMode4();
        if (!GetInputManagerSlot()) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, InputManager_Dtor, moduleStorage + 0x0C);
                s_inputManagerInitialized = 1;
            }
            GetInputManagerSlot() = s_gameInputManager;
        }
        InputManager_ResumeRumble(GetInputManagerSlot());
        if (!GetInputManagerSlot()) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, InputManager_Dtor, moduleStorage + 0x0C);
                s_inputManagerInitialized = 1;
            }
            GetInputManagerSlot() = s_gameInputManager;
        }
        int zero = 0;
        void* input = GetInputManagerSlot();
        void* chk = Sing_805DC950();
        ((GameInputManager*)input)->actionBlockedSecondary[0] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[1] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[2] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[3] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[4] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[5] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[6] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[7] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[8] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[9] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[10] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[11] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[12] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[13] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[14] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[15] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[16] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[17] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[18] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[19] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[20] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[21] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[22] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[23] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[24] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[25] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[26] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[27] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[28] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[29] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[30] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[31] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[32] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[33] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[34] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[35] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[36] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[37] = zero;
        ((GameInputManager*)input)->actionBlockedSecondary[38] = zero;
        if (!chk) {
            if (!s_movieControllerInitialized) {
                MovieController_Ctor(s_movieController);
                __register_global_object(s_movieController, MovieController_Dtor, moduleStorage + 0x24);
                s_movieControllerInitialized = 1;
            }
            Sing_805DC950() = s_movieController;
        }
        MovieController_ReleasePlaybackReference(Sing_805DC950());
        RemoteAudio_RequestEnableAllLazyForwarder(g_soundManager);
    }
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, moduleStorage);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())->ResumeCallbacks();
    AsContext8028A588(self)->state = kHomeIdle;
}

// Default gallery-image lifecycle hook: succeed without additional work.
int CGallery::image_vfunc_1() {
    return 1;
}

// Save transforms and selected render states, then establish the orthographic HOME drawing state.
void Render_SaveStateAndSetupOrthographic2D(void* unusedOwner, char* stateBackup) {
    // Caller owns a writable, preinitialized 0x120-byte backup (map above).
    // The orthographic screen rectangle is [-304,+304] x [-228,+228], near 0,
    // far 500, with identity view/texture transforms. Coordinates are HOME drawing
    // units, not normalized clip coordinates. See HomeRenderStateId for the map.
    // Widescreen: change left/right together to retain center; larger horizontal
    // span fits more HOME units at a fixed viewport. Keep the vertical pair and
    // HBM adjustX/adjustY coherent. No viewport or scissor is set or backed up
    // here. unusedOwner is ignored. This helper leaves the new render state live;
    // ProcessState restores the snapshot AFTER HBMDraw, plus its explicit resets.
    // Phase 1: snapshot only the caller-provided state IDs and three transforms.
    Engine_GetTransform(kHomeProjectionTransform, stateBackup);
    Engine_GetTransform(kHomeViewTransform, stateBackup + kHomeTransform0Backup);
    Engine_GetTransform(kHomeTextureTransform, stateBackup + kHomeTransform11Backup);
    char* p = stateBackup + kHomeTextureStateBackup;
    for (unsigned int i = 0; i < 4; i++, p += 8) {
        Engine_GetTextureStageState(0, *(void**)p, p + 4);
    }
    p = stateBackup + kHomeRenderStateBackup;
    for (unsigned int i = 0; i < 8; i++, p += 8) {
        Engine_GetRenderState(*(void**)p, p + 4);
    }
    // Phase 2: immediate vertices, stage-0 operations, depth/blend/cull policy.
    // Blend is disabled; the zero factors do not imply a transparent HOME menu.
    Engine_SetFlexibleVertexFormat(kHomeVertexFormat);
    Engine_SetIndexBuffer(0, 0);
    Engine_SetTextureStageState(0, kHomeColorOperation, kHomeSelectTextureArgument1);
    Engine_SetTextureStageState(0, kHomeColorArgument1, 0);
    Engine_SetTextureStageState(0, kHomeAlphaOperation, kHomeSelectTextureArgument1);
    Engine_SetTextureStageState(0, kHomeAlphaArgument1, 0);
    Engine_SetCachedRenderState(kHomeBlendEnabled, 0);
    Engine_SetCachedRenderState(kHomeSourceBlend, 0);
    Engine_SetCachedRenderState(kHomeDestinationBlend, 0);
    Engine_SetCachedRenderState(kHomeDepthTest, 1);
    Engine_SetCachedRenderState(kHomeDepthCompare, kHomeCompareLessEqual);
    Engine_SetCachedRenderState(kHomeDepthWrite, 1);
    Engine_SetCachedRenderState(kHomeCullMode, 0);
    Engine_SetCachedRenderState(kHomeLightingChannels, 0);
    // Phase 3: build the centered orthographic projection and flush cached state.
    float mtx[16];
    MtxOrthoOffCenter(mtx, kFloat_neg304_HomeMenuProjection, kFloat_304_HomeMenuProjection,
                      kFloat_neg228_HomeMenuProjection, kFloat_228_HomeMenuProjection, kFloat_0_FileDialog,
                      kFloat_500_HomeMenuProjectionFar);
    SetTransform(kHomeProjectionTransform, mtx);
    SetTransform(kHomeViewTransform, kIdentityMatrix4x4);
    SetTransform(kHomeTextureTransform, kIdentityMatrix4x4);
    Engine_FlushRenderState();
}
// Default gallery-image lifecycle hook: succeed without additional work.
int CGallery::image_vfunc_2() {
    return 1;
}
// Default gallery-image lifecycle hook: succeed without additional work.
int CGallery::image_vfunc_3() {
    return 1;
}
// Default gallery-image lifecycle hook: succeed without additional work.
int CGallery::image_vfunc_4() {
    return 1;
}

// Set both the current and saved HOME-menu block flags.
void HomeMenu_SetBlockedState(char* self, unsigned char v) {
    AsContext8028A588(self)->homeMenuBlocked = v;
    AsContext8028A588(self)->savedHomeMenuBlocked = v;
}

// Initialize the HOME-menu module's cached full-turn angle.
void HomeButtonMenu_InitTwoPiCache() {
    s_homeButtonMenuTwoPi = kFloat_6_2831855;
}

// Initialize the shared-file module's cached full-turn angle.
void SharedFileRegistry_InitTwoPiCache() {
    s_sharedFileRegistryTwoPi = kFloat_6_2831855;
}

// Report whether the HOME library is in its active processing state.
int HomeButtonMenu_IsActive(char* self) {
    int homeState = AsContext8028A588(self)->state;
    return !(homeState - 2);
}

// Temporarily block HOME while preserving the prior flag, or restore that prior flag.
void HomeMenu_OverrideBlockedState(char* self, unsigned char v) {
    unsigned char cur = AsContext8028A588(self)->homeMenuBlocked;
    if (v == cur)
        return;
    if (v == 1) {
        AsContext8028A588(self)->savedHomeMenuBlocked = cur;
        AsContext8028A588(self)->homeMenuBlocked = v;
    } else {
        AsContext8028A588(self)->homeMenuBlocked = AsContext8028A588(self)->savedHomeMenuBlocked;
    }
}

// Return the mapped value for a table index, or null when the entry is absent.
void* SharedFileTable_GetMappedValueAt(const void* self, int index) {
    void* table = *(void**)self;
    void* entry;
    entry = Table_GetEntry(table, index);
    if (entry != 0) {
        return ((SharedTableEntryView*)entry)->value;
    }
    return 0;
}

// Shut down the HOME library, then explicitly free the separate work/sound and icon buffers.
// Release/transfer of the four archive/configuration copies remains unproved (see loader audit).
void HomeButton_ShutdownResources(void* self) {
    HomeMenu_DeleteSound();
    HomeMenu_Delete();
    void* p = ((ResourceState8028A270*)self)->resourceBuffer;
    if (p != 0) {
        appFreeUni__FPv(p);
        ((ResourceState8028A270*)self)->resourceBuffer = 0;
    }
    void* g = g_homeButtonIconTpl;
    if (g != 0) {
        appFreeUni__FPv(g);
        g_homeButtonIconTpl = 0;
    }
}

// Release the vector backing buffer, clear its count and optionally delete the vector owner.
void* dtor_8028BBF4(MSLPointerVector* self, int flag) {
    return ::std::__vector_deleting_dtor(self, flag);
}

// Destroy shared-file records in reverse order, free their storage and optionally delete the owner.
void* SharedBufferVector_dtor(void* self, int flag) {
    if (self) {
        if (self) {
            if (self) {
                if (self) {
                    if (((SharedBufferVectorView*)self)->data) {
                        char* p;
                        int count = ((SharedBufferVectorView*)self)->count;
                        int offset = count * 0x14;
                        ((SharedBufferVectorView*)self)->count -= count;
                        p = (char*)((SharedBufferVectorView*)self)->data + offset;
                        while (count != 0) {
                            int deleteFlag = -1;
                            p = p - 0x14;
                            void* obj = (void*)p;
                            void** vt = *(void***)obj;
                            ((void (*)(void*, int))vt[2])(obj, deleteFlag);
                            count = count - 1;
                        }
                        appFreeUni__FPv(((SharedBufferVectorView*)self)->data);
                    }
                }
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Advance the HOME-unavailable script through exit, remove completed entries and draw when enabled.
void HomeButtonMenu_UpdateScript(void* selfRaw) {
    unsigned char* self = (unsigned char*)selfRaw;
    int scriptEntryId = AsContext8028A588(self)->scriptEntryId;
    if (scriptEntryId == -1)
        return;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_homeMenuInputStorage);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* scriptObject = IdMap_Lookup(Sing_805DC5F0(), AsContext8028A588(self)->scriptEntryId);
    if (scriptObject == 0)
        return;
    if (BpObject_PollLoad(scriptObject) == 0)
        return;
    if (BpObject_IsCurrentScriptComplete(scriptObject) != 0) {
        if ((unsigned char)AsContext8028A588(self)->scriptExitStarted) {
            AsContext8028A588(self)->scriptExitStarted = 0;
            if (AsContext8028A588(self)->scriptEntryId != -1) {
                if (!Sing_805DC5F0()) {
                    if (!s_bpCtrlInitialized) {
                        ((CBpCtrl*)s_bpCtrl)->__ct();
                        __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_homeMenuInputStorage);
                        s_bpCtrlInitialized = 1;
                    }
                    Sing_805DC5F0() = s_bpCtrl;
                }
                ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(AsContext8028A588(self)->scriptEntryId);
                AsContext8028A588(self)->scriptEntryId = -1;
            }
            return;
        } else {
            AsContext8028A588(self)->scriptExitStarted = 1;
            BpObject_StartScriptByName(scriptObject, HomeResources()->unavailableScript, 1);
        }
    }
    BpObject_TryInvokeCurrentCallback(scriptObject, 0);
    if (((BpDrawEnableView*)scriptObject)->drawEnabled != 0) {
        BpObject_Draw(scriptObject);
    }
}

#pragma opt_propagation off
// Delete registered handles, destroy embedded file records and release both backing vectors.
CSharedFileRegistry::~CSharedFileRegistry() {
    // Phase 1: delete pointed-to handles, then destroy inline 0x14-byte file
    // records in reverse order. Counts reach zero before either buffer is freed.
    int recordBytes;
    void* recordBase;
    int zero8;
    int off3;
    void* base3;
    vtbl = (void*)__vt__19CSharedFileRegistry;
    char* cur = (char*)filePointers;
    char* e;
    while (e = (char*)filePointers + filePointerCount * 4, !(e == cur)) {
        void* elem = *(void**)cur;
        if (elem) {
            if (elem)
                ((CSharedFileElem*)elem)->vdtor(1);
            *(void**)cur = 0;
        }
        cur = cur + 4;
    }
    int recordCount = fileRecordCount;
    int n8 = filePointerCount;
    recordBase = fileRecords;
    recordBytes = recordCount * 0x14;
    zero8 = filePointerCount - n8;
    filePointerCount = zero8;
    fileRecordCount -= recordCount;
    char* p2 = (char*)recordBase + recordBytes;
    while (recordCount != 0) {
        p2 = p2 - 0x14;
        ((CSharedFileElem*)p2)->vdtor(-1);
        recordCount = recordCount - 1;
    }
    // Phase 2: vector subobject cleanup frees allocations. The repeated address
    // guards are inlined destruction layers; only backing pointers are optional.
    if (((char*)this + kFileRecordsOffset)) {
        if (((char*)this + kFileRecordsOffset)) {
            if (((char*)this + kFileRecordsOffset)) {
                if (((char*)this + kFileRecordsOffset)) {
                    base3 = fileRecords;
                    if (base3) {
                        char* p3;
                        int c3 = fileRecordCount;
                        off3 = c3 * 0x14;
                        fileRecordCount -= c3;
                        p3 = (char*)base3 + off3;
                        while (c3 != 0) {
                            p3 -= 0x14;
                            ((CSharedFileElem*)p3)->vdtor(-1);
                            c3 = c3 - 1;
                        }
                        appFreeUni__FPv(fileRecords);
                    }
                }
            }
        }
    }
    if (((char*)this + kFilePointersOffset)) {
        if (((char*)this + kFilePointersOffset)) {
            if (((char*)this + kFilePointersOffset)) {
                if (((char*)this + kFilePointersOffset)) {
                    if (((char*)this + kFilePointersOffset)) {
                        void* arr4 = filePointers;
                        if (arr4) {
                            int n4 = filePointerCount;
                            filePointerCount = filePointerCount - n4;
                            appFreeUni__FPv(arr4);
                        }
                    }
                }
            }
        }
    }
}
#pragma opt_propagation reset

// Load localized HOME archives, messages, configuration and icons into retained copied buffers.
// self/out required; out must cover HomeCreateInfoView and normally aliases self.
// Paths use a 260-byte stack buffer and fixed pools. There is no timeout or
// recoverable allocation/load failure result: pending handles keep drawing
// reload frames, and successful copies are published incrementally.
// Publication/lifetime map (five independent appMallocUni allocations):
// phase | published destination | consumer / last required use | visible release
// 1 layout | out+00 | HBMCreate/HBMDraw until HomeMenu_Delete | none in wrapper
// 2 speaker| out+04 | HBM sound setup until DeleteSound/Delete | none in wrapper
// 3 message| out+08 | HBMCreate/text drawing until Delete | none in wrapper
// 4 config | out+0C and byte size out+28 | HBMCreate until Delete | none in wrapper
// 5 icon   | g_homeButtonIconTpl | TPLBind/HOME draw until Delete | appFreeUni
// The separately allocated owner+10 work/sound buffer is NOT any of these five;
// HomeButtonMenu's explicit shutdown and HomeButtonResources_dtor free it after
// HomeMenu_DeleteSound -> HomeMenu_Delete, then free icons and remove the BP ID.
// Four retained copies have no proven release/transfer in visible src/include;
// external HomeMenu_Delete/internal child destruction is not implemented here.
// Source audit endpoints: HomeButton_InitializeSoundArchive (small_common_dialog.cpp) calls this
// loader, allocates owner+10 separately, then passes owner to HomeMenu_Create.
// HomeButton_ShutdownResources here and HomeButtonResources_dtor (small_common_dialog.cpp)
// call DeleteSound/Delete, appFreeUni(+10), appFreeUni(icon), but neither has a
// release call for +00/+04/+08/+0C. There is no source-visible named operation
// that a caller can safely use to discharge those four copies. This is an OPEN
// ownership contract, not evidence authorizing four new appFreeUni calls.
// Treat them as requiring lifetime through Delete; do not claim either a proved
// leak or library ownership. Their eventual release remains an evidence gap.
// Incremental failure: after phase N, earlier destinations remain published;
// later ones keep prior contents. A pending load can wait forever; null malloc
// is used by memcpy without a check; there is no rollback, aggregate success
// result, or automatic release of copies on a later failure. The current handle
// in self->sharedFile remains owned until that phase's Destroy(handle,1).
// Normal caller uses self==out with initialized state. Do not call again over
// live copies: overwritten pointers are not released. Source bytes are borrowed
// only until memcpy, handle destruction follows the copy and precedes publication.
void HomeMenu_LoadResourceBuffers(char* self, char* out) {
    // Every phase waits by drawing reload frames, copies bytes to appMallocUni
    // storage, and destroys the temporary shared-file handle. HBM borrows copied
    // buffers through out. The sole source caller passes (self,self), so these
    // are persistent owner fields, not temporary output pointers. Retain the copies
    // through the HomeMenu_Delete call. out is not zeroed.
    // Destination map: out+00 layout, +04 speaker, +08 messages, +0c config;
    // +28 config byte length. Icons are published separately to g_homeButtonIconTpl.
    // Ownership audit: layout/speaker/messages/config are four distinct allocations
    // at owner+00/+04/+08/+0c. The explicit shutdown path frees only the work buffer
    // at +10 and global icons. Neither that path nor HomeButtonResources_dtor
    // explicitly frees these four copies. HomeMenu_Delete is only an external
    // declaration in src/include; its internal child ownership cannot be checked
    // from these source consumers. Do not free the copies here
    // or treat +10 as their backing allocation: that would change existing behavior.
    // Phase 1: select the localized layout archive (unsupported language -> 0).
    char empty = 0;
    char resourcePath[260];
    strcpy(resourcePath, (char*)&empty);
    unsigned char language = (unsigned char)SCGetLanguage();
    ((HomeCreateInfoView*)out)->language = language;
    switch (language) {
    case 0:
        strcat(resourcePath, HomeResources()->defaultArchive);
        break;
    case 1:
        strcat(resourcePath, HomeResources()->englishArchive);
        break;
    case 2:
        strcat(resourcePath, HomeResources()->germanArchive);
        break;
    case 3:
        strcat(resourcePath, HomeResources()->frenchArchive);
        break;
    case 4:
        strcat(resourcePath, HomeResources()->spanishArchive);
        break;
    case 5:
        strcat(resourcePath, HomeResources()->italianArchive);
        break;
    case 6:
        strcat(resourcePath, HomeResources()->dutchArchive);
        break;
    default:
        ((HomeCreateInfoView*)out)->language = 0;
        strcat(resourcePath, HomeResources()->defaultArchive);
        break;
    }
    void* fileHandle = SharedFile_Create(resourcePath, 0);
    AsContext8028A588(self)->sharedFile = fileHandle;
    while (CSharedFilePtr_GetLoadedBuffer(fileHandle) == 0) {
        Game_DrawReloadingFrame();
        fileHandle = AsContext8028A588(self)->sharedFile;
    }
    fileHandle = AsContext8028A588(self)->sharedFile;
    int layoutBytes = CSharedFilePtr::GetSize(fileHandle);
    void* layoutCopy = appMallocUni__FUl(layoutBytes);
    fileHandle = AsContext8028A588(self)->sharedFile;
    void* layoutSource = (void*)CSharedFilePtr_GetLoadedBuffer(fileHandle);
    memcpy(layoutCopy, layoutSource, layoutBytes);
    fileHandle = AsContext8028A588(self)->sharedFile;
    if (fileHandle) {
        CSharedFileHandle::Destroy(fileHandle, 1);
        AsContext8028A588(self)->sharedFile = 0;
    }
    ((HomeCreateInfoView*)out)->layoutArchive = layoutCopy;
    // Phase 2: copy the speaker archive, release its shared-file handle, publish +04.
    strcpy(resourcePath, (char*)&empty);
    strcat(resourcePath, HomeResources()->speakerArchive);
    fileHandle = SharedFile_Create(resourcePath, 0);
    AsContext8028A588(self)->sharedFile = fileHandle;
    while (CSharedFilePtr_GetLoadedBuffer(fileHandle) == 0) {
        Game_DrawReloadingFrame();
        fileHandle = AsContext8028A588(self)->sharedFile;
    }
    fileHandle = AsContext8028A588(self)->sharedFile;
    int speakerBytes = CSharedFilePtr::GetSize(fileHandle);
    void* speakerCopy = appMallocUni__FUl(speakerBytes);
    fileHandle = AsContext8028A588(self)->sharedFile;
    void* speakerSource = (void*)CSharedFilePtr_GetLoadedBuffer(fileHandle);
    memcpy(speakerCopy, speakerSource, speakerBytes);
    fileHandle = AsContext8028A588(self)->sharedFile;
    if (fileHandle) {
        CSharedFileHandle::Destroy(fileHandle, 1);
        AsContext8028A588(self)->sharedFile = 0;
    }
    ((HomeCreateInfoView*)out)->speakerArchive = speakerCopy;
    // Phase 3: copy the message table and publish +08; source belongs to the handle.
    strcpy(resourcePath, (char*)&empty);
    strcat(resourcePath, HomeResources()->messageTable);
    fileHandle = SharedFile_Create(resourcePath, 0);
    AsContext8028A588(self)->sharedFile = fileHandle;
    while (CSharedFilePtr_GetLoadedBuffer(fileHandle) == 0) {
        Game_DrawReloadingFrame();
        fileHandle = AsContext8028A588(self)->sharedFile;
    }
    fileHandle = AsContext8028A588(self)->sharedFile;
    int messageBytes = CSharedFilePtr::GetSize(fileHandle);
    void* messageCopy = appMallocUni__FUl(messageBytes);
    fileHandle = AsContext8028A588(self)->sharedFile;
    void* messageSource = (void*)CSharedFilePtr_GetLoadedBuffer(fileHandle);
    memcpy(messageCopy, messageSource, messageBytes);
    fileHandle = AsContext8028A588(self)->sharedFile;
    if (fileHandle) {
        CSharedFileHandle::Destroy(fileHandle, 1);
        AsContext8028A588(self)->sharedFile = 0;
    }
    ((HomeCreateInfoView*)out)->messages = messageCopy;
    // Phase 4: copy configuration, publish byte length at +28 and pointer at +0c.
    strcpy(resourcePath, (char*)&empty);
    strcat(resourcePath, HomeResources()->configuration);
    fileHandle = SharedFile_Create(resourcePath, 0);
    AsContext8028A588(self)->sharedFile = fileHandle;
    while (CSharedFilePtr_GetLoadedBuffer(fileHandle) == 0) {
        Game_DrawReloadingFrame();
        fileHandle = AsContext8028A588(self)->sharedFile;
    }
    fileHandle = AsContext8028A588(self)->sharedFile;
    int configurationBytes = CSharedFilePtr::GetSize(fileHandle);
    void* configurationCopy = appMallocUni__FUl(configurationBytes);
    fileHandle = AsContext8028A588(self)->sharedFile;
    void* configurationSource = (void*)CSharedFilePtr_GetLoadedBuffer(fileHandle);
    memcpy(configurationCopy, configurationSource, configurationBytes);
    if (out + kHomeConfigSizeOffset) {
        *(int*)(out + kHomeConfigSizeOffset) = configurationBytes;
    }
    fileHandle = AsContext8028A588(self)->sharedFile;
    if (fileHandle) {
        CSharedFileHandle::Destroy(fileHandle, 1);
        AsContext8028A588(self)->sharedFile = 0;
    }
    ((HomeCreateInfoView*)out)->configuration = configurationCopy;
    ((HomeCreateInfoView*)out)->option14 = 0;
    ((HomeCreateInfoView*)out)->option18 = 0;
    ((HomeCreateInfoView*)out)->option20 = 0;
    // Baseline 1,1 are HBM adjustment factors; frameStep is float 2.0, not an
    // fps value. HBMSetAdjustFlag controls whether its adjustment policy applies.
    // The HBM clock's physical unit is not established here; do not halve this
    // merely because presentation changes from 30 to 60 fps.
    // Producer/consumer chain: this assignment writes create-info +30 once;
    // HomeButton_InitializeSoundArchive passes it to external HomeMenu_Create.
    // Active ProcessState calls external HBMCalc once per game update with pad
    // descriptors only (no dt argument), so this file performs no seconds/ticks
    // conversion for HBM. Whether 2.0 advances animation frames, timer ticks or
    // another library unit per call is unproved. The nine-call audio delay is a
    // separate counter and supplies no evidence for this scalar's physical unit.
    ((HomeCreateInfoView*)out)->adjustX = kFloat_1_FileDialog;
    ((HomeCreateInfoView*)out)->adjustY = kFloat_1_FileDialog;
    ((HomeCreateInfoView*)out)->frameStep = s_homeMenuFrameStepWithTrailingWord;
    // Phase 5: icon TPL has a separate global owner; bind only after copying and
    // releasing the file handle, so the retained texture data survives that release.
    strcpy(resourcePath, (char*)&empty);
    strcat(resourcePath, HomeResources()->iconTextures);
    fileHandle = SharedFile_Create(resourcePath, 0);
    AsContext8028A588(self)->sharedFile = fileHandle;
    while (CSharedFilePtr_GetLoadedBuffer(fileHandle) == 0) {
        Game_DrawReloadingFrame();
        fileHandle = AsContext8028A588(self)->sharedFile;
    }
    fileHandle = AsContext8028A588(self)->sharedFile;
    int iconBytes = CSharedFilePtr::GetSize(fileHandle);
    void* iconCopy = appMallocUni__FUl(iconBytes);
    fileHandle = AsContext8028A588(self)->sharedFile;
    void* iconSource = (void*)CSharedFilePtr_GetLoadedBuffer(fileHandle);
    memcpy(iconCopy, iconSource, iconBytes);
    fileHandle = AsContext8028A588(self)->sharedFile;
    if (fileHandle) {
        CSharedFileHandle::Destroy(fileHandle, 1);
        AsContext8028A588(self)->sharedFile = 0;
    }
    g_homeButtonIconTpl = iconCopy;
    TPLBind(iconCopy);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462418", _ctors_u8028A588_0, HomeButtonMenu_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU80462418", _ctors_u8028A588_1, SharedFileRegistry_InitTwoPiCache);
#pragma pop
