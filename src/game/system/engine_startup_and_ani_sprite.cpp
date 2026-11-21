#include <decomp/static_init.h>
// Engine startup presentation and animated texture sprites (original ABI names below).
// main -> Game_InitializeEngineAndMedia -> video/render targets, input, audio,
// loading UI and media; W3dAniSpr owns frame records and retained bitmap handles.
// Shared layouts: WView, W3dAniSpr, W3dSprVertex. Scalar/UV pool: u80216D5C_data.cpp.
// Modder lookup:
// - Resolution/output: s_engineStartupDefaults at 0x80473488, defined in
//   VI rectangle, viewport and scissor are all 640x480; edit these as a set.
//   TV-derived width/height overwrite display/VI dimensions in the final switch.
// - Frame rate: defaults display word 13 (+0x34, 0x804734BC) is swapInterval=2
//   retraces; Engine_PresentFrame/Engine_BuildRenderMode in gx_renderer_state_and_resources.cpp consume
//   the presentation policy. This is separate from simulation seconds/update.
// - Aspect ratio: the startup loading camera uses kStartupProjectionAspect below
//   (4/3); the main camera uses kViewPerspectiveAspectRatio in u80065F78_data.cpp. Neither
//   sets framebuffer resolution or automatically repositions HUD/layout groups.
// - Viewport/scissor: startup-default word offsets 0x32/0x38; six words are
//   x,y,width,height,nearDepth,farDepth; four scissor words are x,y,width,height
//   (Engine_SetScissorRect forwards them to GXSetScissor). Pixel origin is the top
//   left, +X right, +Y down; scissor's last two words are sizes, not endpoints.
// - Texture capture: kSharedCaptureWidthPixels/HeightPixels request a 256x256
//   shared target; this is independent of the output resolution.
// - Sprite textures: LoadBitmapGrid retains a bitmap; AddBitmapGridFrames only
//   borrows it. UV selectors in the data companion map to texel-center UVs.
// Preserve initialization/publication order, callback order, shutdown ordering,
// projection handedness and FP expression order. No resource-load timeout is
// provided; startup pumps loading frames until all required resources report ready.
#include "classes/CSceneManager.h"
#include "classes/WView.h"
// Borrow the view layout; callers retain both storage and lifetime.
static inline WView* AsWView(void* p) { return (WView*)p; }
// W3dAniSpr_DrawFrame / W3dAniSpr_DrawIndexedFrameWithMatrix: transform arguments are WView objects.
#include "classes/W3dAniSpr.h"
#include "classes/W3dSprVertex.h"
// W3dAniSpr_Ctor (0x80217E9C): shared 0x38-byte vertex color/UV layout.
// Borrow the animated-sprite layout; direct expansion preserves the constructor's
// original load/store scheduling (a function wrapper changes W3dAniSpr_Ctor).
#define AsW3dAniSpr(p) ((W3dAniSpr*)(p))
#include "sdk_protos.h"
#include "game/types.h"
#include "game/singletons.h"
#include "game/wvector.h"
#include <revolution/OS/OSFastCast.h>

#define OSInitMutex(p) OSInitMutex((OSMutex*)(p))
#define OSLockMutex(p) OSLockMutex((OSMutex*)(p))
#define OSUnlockMutex(p) OSUnlockMutex((OSMutex*)(p))

extern "C" {

enum SpriteConstants {
    kBitmapList = 0x14, kFrameList = 0x4C,
    kBitmapMutexOffset = (int)&((W3dAniSpr*)0)->bitmapMutex,
    kDefaultNodeBatch = 16, kSpriteCorners = 4,
    kOpaqueWhiteSprite = -1,
    kUntexturedRenderFlags = 0x06000000,
    kBitmapIdMask = 0x7FF,
    kSubmitSpriteQuad = 0x10,
    kIndexedDrawOpacity = 5 // actual packet alpha byte, not a render-mode ID; preserve the low value
};
// Owned frame-list payload (20 bytes): integer resource key followed by normalized UVs.
// UV order is V,U,V,U in storage; bitmapId is never a float or owned by this record.
struct SpriteFrame {
    int bitmapId;
    float topV, leftU, bottomV, rightU;
};
// Projected vertex view; UV/color members agree with W3dSprVertex.
struct ProjectedSpriteVertex {
    float x, y, z, reciprocalW;
    unsigned int color;
    char padding14[4];
    float textureU, textureV;
    char padding20[0x14];
    unsigned char clipFlags;
    char padding35[3];
};
typedef char SpriteVertexSize[(sizeof(ProjectedSpriteVertex) == 0x38) ? 1 : -1];


void* appMallocUni__FUl(unsigned long);
void appFreeUni__FPv(void*);
void AppHeap_AllocateFromHeap0(int);
void Memory_FreeFromArrayHeap0(void*);
void BaseObject_ctor(void*);

int Game_InitializeProjectAndSharedResources(int);
void Game_Shutdown();
void Game_MainLoop(int);
void Game_InitializeEngineAndMedia();

void CommonDialogue_ReleaseSharedResources();
void CCheckDriveStatus_StopThread();
void GameDialogue_ReleaseSharedResources();
void Language_InitializeFromSystemSettings();

void HashPath_Shutdown();

void MediaRegistry_ResetGlobalPointer();
void gtl_Audio_Shutdown();

void FileRegistry_ClearOwnedDevices();
void Renderer_EmptyHook();
void InputDriver_InitStub();
void Projection_ResetDepthConvention();
void Projection_SetDepthConvention(void*);
void FileIo_InitializeDevices(void*);
void VideoConfig_InitFromTvFormat(void*);
unsigned int Video_GetDtvOutputCode();
void Engine_InitializeVideoAndRenderTargets(void*);
void Engine_SetViewport(void*);
void Engine_SetScissorRect(void*);
void gtl_BitmapFont_Initialize(int);
void gtl_GenericResource_Initialize(void*);
void Render_SetSystemOption(int, int);
void MtxPerspectiveFovLH(void*, float, float, float, float);
void SetTransform(int, void*);
void gtl_GenericRenderer_SetProjectionMatrix(void*);
void MTXLookAt(void*, void*, void*, void*);
void gtl_GenericRenderer_SetViewMatrix(void*);
void gtl_InputEx_Initialize(void*);
void gtl_Audio_Initialize(void*);
void SystemResetState_Ctor(void*);
void SystemReset_InstallCallbacks(void*);
void CCheckDriveStatus_StartThread();
void Load_HashPathTable(void*);
void InputManager_Init(void*);
void InputManager_LoadCursorResources(void*, int);
void HomeButtonMenu_InitializeState(void*);
void HomeButton_InitializeSoundArchive(void*);
void CNowReloading_Init(void*);
void LoadingIndicator_CreatePlayer(void*);
void LoadingScreen_LoadB2dResources();
void CommonDialogue_LoadSharedResources();
void GameDialogue_LoadSharedResources();
void CCheckDriveStatus_SetSkipDialogDelay(int);
void gtl_InputEx_Update();
void InputManager_Update(void*);
void Engine_ClearFramebuffer(float, int, int, int, int, int);
void B2dPlayer_UpdateAndDraw(void*);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_InvokeEntryCallbacks(void*, int);
void CBpCtrl_DrawEntries(void*);
void CBpCtrl_ResetEntryFrameState(void*);
void HomeButtonMenu_ProcessState(void*);
void Engine_PresentFrame(int, int, int, int);
int LoadingIndicator_IsAbsentOrState3(void*);
void LoadingScreen_PollResourceReadiness();
int GameDialogue_PollSharedResourcesLoaded();
int CommonDialogue_PollSharedResourcesLoaded();
int EventTiming_IsDurationScalingEnabled();
void gtl_Media_Init(void*);
void VIEnableDimming(int);
void TextureCapture_SetSharedRenderTargetSize(int, int);
unsigned int VIGetScanMode();
void Engine_SetOutputRectangle(void*);
void Engine_ChangeDisplayMode(void*);
void SystemResetState_Dtor();
void InputManager_Dtor();
void HomeButtonResources_dtor();
void CNowReloading_dtor();
void CBpCtrl_dtor();
extern unsigned char s_engineStartupDefaults[], gameStartupDtorRegistrations[], s_systemResetStorage[], s_gameInputManager[], s_homeButtonResourceState[], s_bpCtrl[];
extern unsigned char s_nowReloading[1], s_screenDimmingState;
extern signed char s_systemResetStateInitialized, s_inputManagerInitialized, s_homeMenuInitialized, s_nowReloadingInitialized, s_bpCtrlInitialized, s_videoDimmingStateInitialized;
extern int s_initialProjectionDepthConvention; // Initial projection depth convention.
extern int s_initialGenericResourceConfigWord0, s_initialGenericResourceConfigWord1; // Initial generic-resource configuration words.
extern int s_initialExtendedInputConfigWord0, s_initialExtendedInputConfigWord1; // Initial extended-input configuration words.
extern float kFloat_1_FramebufferClear;
struct W3 { int w[3]; };
struct W4 { int w[4]; };
struct W5 { int w[5]; };
struct W6 { int w[6]; };
struct W14 { int w[14]; };
// Packet slots are grouped by their proven source, not by guessed option meanings.
// The first 25 words are seeded from the engine template; all but word 24 in
// the presentation/output-rectangle ranges are then replaced below. Mirror fields duplicate
// selected presentation/output-rectangle words. Change the source
// defaults to tune these together. Display word 11 has no mirror entry;
// flags 0x20/0x40 select interlaced/progressive scan.
// Engine_BuildRenderMode (gx_renderer_state_and_resources.cpp) consumes this 14-word presentation record.
// Dimensions/origins are pixels; swapInterval is the retrace policy. Unknown words stay neutral.
// Destination audit against Engine_InitializeVideoAndRenderTargets (gx_renderer_state_and_resources.cpp):
// packet +00/+04 = uniform allocate/free callbacks; +08/+0C = heap-0 array
// allocate/free callbacks. They are 32-bit CODE addresses, not numeric options.
// The callee copies only +00..63 (25 words) to the engine header and publishes
// that header as g_graphicsMemoryCallbacks; +04 destroys graphics allocations,
// +08 allocates the FIFO, +00 allocates graphics objects. +10 is zero-initialized
// retained header storage with no named consumer established; preserve as opaque.
// +14..4B is EnginePresentParams: width/height, reserved08=6/reserved0C=0,
// type=0, reserved14[6]={0,1,0,0,1,22}, flags, tvFormat=0, swapInterval=2.
// BuildRenderMode reads width/height/type/flags/tvFormat; PresentFrame reads
// swapInterval. The reserved words are copied by Engine_ChangeDisplayMode but
// have no demonstrated setting effect; do not invent controls from defaults.
// +4C..5F is EngineViRect x/y/width/height/format (default 0,0,640,480,2).
// +60 (templateWord24) is copied to offscreenRenderingEnabled, initially zero;
// the frame-presentation path branches on that field. +64..8F mirrors are
// populated here but outside the callee's 100-byte copy and unused by it.
// Consequently they are not extra live resolution knobs. Defaults are defined
struct StartupPresentation {
    int width, height, reserved08, reserved0C, modeType;
    int reserved14[6];
    int scanFlags, tvFormat, swapInterval;
};
// Despite the shared setter's name, these five words are EngineViRect, not vertex attributes.
struct StartupOutputRectangle { int x, y, width, height, format; };

struct EngineStartupPacket {
    int engineOptions[5];       // packet 0..3 allocator ABI addresses; 4 opaque header word
    StartupPresentation displayMode; // packet 5..18: EnginePresentParams
    StartupOutputRectangle outputRect; // packet 19..23: EngineViRect
    int templateWord24;         // packet 24: initial offscreenRenderingEnabled
    int displayMirror[6];       // packet 25..30: display words 6,7,8,9,10,12
    int outputRectMirror[5];    // packet 31..35: output rect words 4,1,2,3,3
};
typedef char EngineStartupPacket_size[(sizeof(EngineStartupPacket) == 36 * 4) ? 1 : -1];
struct W12 { int w[12]; };
struct F16 { float f[16]; };
// Wn types describe ABI word counts, not inferred C++ engine classes. The
// startup locals below now name their destination subsystem; unknown packet
// unknown words remain neutral rather than acquiring speculative semantic names.
enum StartupDefaultsWord {
    kInitialDisplayDefaults = 0, kInitialOutputRectDefaults = 0xE,
    kFileIoCallbackDefaults = 0x14, kEnginePacketDefaults = 0x18,
    kViewportDefaults = 0x32, kScissorDefaults = 0x38,
    kCameraEyeDefaults = 0x3C, kCameraUpDefaults = 0x3F,
    kAudioDefaults = 0x46, kMediaDefaults = 0x4B
};

// Proven option positions (VideoConfig_InitFromTvFormat and the TV override below).
// Keep the copyable word packets: changing their aggregate type changes codegen.
enum StartupOptionWord {
    kTvWidthWord = 0, kTvHeightWord = 1, kTvInterlacedWord = 2,
    kDisplayWidthWord = 0, kDisplayHeightWord = 1, kDisplayScanFlagsWord = 11,
    kOutputRectWidthWord = 2, kOutputRectHeightWord = 3,
    kInterlacedPresentation = 32, kProgressivePresentation = 64
};

// Local literal aliases: these are not new linked settings or replacement pools.
// Projection FOV is vertical radians; clips are scene distances. Raising aspect
// at fixed vertical FOV widens this startup view; safe clip/FOV limits are unestablished.
const float kStartupVerticalFovRadians = 0.7853981852531433f;
const float kStartupProjectionAspect = 1.3333333730697632f;
const float kStartupNearClip = 3.0f;
const float kStartupFarClip = 100000.0f;
enum StartupRenderConstants {
    kStartupPacketTemplateBytes = 100,
    kSharedCaptureWidthPixels = 256, kSharedCaptureHeightPixels = 256,
    kScanSwitchBlackPresentCalls = 100,
    kTransformView = 0, kTransformProjection = 1,
    kStartupClearFlags = 7 // existing mask; renderer consumes color/depth bits
};

void __dl__FPv(void*);
void* __memcpy(void*, const void*, unsigned long);

extern unsigned char __vt__9W3dAniSpr[];
extern unsigned char s_animatedSpriteDefaultUvSelectors[];
extern const float kFloat_255_VertexTexCoord;
extern const float kFloat_0_5_TextureCoordinates;
extern const float kFloat_0_00390625_TextureCoords;
extern const float kFloat_6_2831855;
extern float s_engineTwoPi;
extern unsigned char s_titleTaskRttiDescriptor[1], s_titleMainRttiDescriptor[1], s_shop2TaskRttiDescriptor[1], s_shop2MainTypeInfo[1];
extern unsigned char s_gridTypeInfo[1], s_spinTypeInfo[1], s_letterTypeInfo[1], s_spotTypeInfo[1];
extern unsigned char s_guideLineTypeInfo[1], s_tutorialRttiDescriptor[1], s_fightTaskTypeDescriptor[1], s_fightMainTypeDescriptor[1];
extern unsigned char s_partyTaskTypeDescriptor[1], s_partyMainTypeDescriptor[1], s_balloonSystemTypeInfo[1], s_dartsSystemRttiDescriptor[1];
extern unsigned char s_driveSystemTypeInfo[1], s_storyConteTypeInfo[1], s_tourTaskRuntimeType[1], s_tourMainTypeInfo[1];
extern unsigned char s_screenRttiDescriptor[1], s_trainingTaskRttiDescriptor[1], s_trainingMainTypeInfo[1], s_singleTaskRevTypeDescriptor[1];
extern unsigned char s_singleMainRevTypeDescriptor[1], s_trajectoryTypeInfo[1], s_saveProcessTaskRtti[1], s_saveProcessMainRtti[1];
extern unsigned char s_roundDolTaskTypeInfo[1], s_roundDolMainTypeInfo[1], s_bgmCollectionTaskRttiDescriptor[1], s_bgmCollectionMainTypeInfo[1];
extern unsigned char s_passwordTaskTypeDescriptor[1], s_passwordMainTypeDescriptor[1];
extern unsigned char s_taskTypeInfo[1], s_taskMainTypeInfo[1], s_actorTypeInfo[1];
extern unsigned char s_gridFactoryName[1], s_gridTaskRegistryName[1], s_spinFactoryName[1], s_spinTaskRegistryName[1];
extern unsigned char s_letterFactoryName[1], s_letterTaskRegistryName[1], s_spotFactoryName[1], s_spotTaskRegistryName[1];
extern unsigned char s_screenFactoryName[1], s_screenTaskRegistryName[1];
extern unsigned char s_titleTaskFactoryName[], s_pangya2TitleMainFactoryName[], s_pangya2TitleMainTaskRegistryName[], s_shop2TaskFactoryName[], s_shop2MainFactoryName[], s_shop2MainTaskRegistryName[];
extern unsigned char s_guideLineFactoryName[], s_guideLineTaskRegistryName[], s_tutorialFactoryName[], s_tutorialTaskRegistryName[];
extern unsigned char s_fightTaskFactoryName[], s_fightMainFactoryName[], s_fightMainTaskRegistryName[], s_partyTaskFactoryName[], s_partyMainFactoryName[], s_partyMainTaskRegistryName[];
extern unsigned char s_balloonSystemFactoryName[], s_balloonSystemTaskRegistryName[], s_dartsSystemFactoryName[], s_dartsSystemTaskRegistryName[], s_driveSystemFactoryName[], s_driveSystemTaskRegistryName[];
extern unsigned char s_storyConteFactoryName[], s_storyConteTaskRegistryName[], s_tourTaskFactoryName[], s_tourMainFactoryName[], s_tourMainTaskRegistryName[];
extern unsigned char s_trainingTaskFactoryName[], s_trainingMainFactoryName[], s_trainingMainTaskRegistryName[], s_singleTaskRevFactoryName[], s_singleMainRevFactoryName[], s_singleMainRevTaskRegistryName[];
extern unsigned char s_trajectoryFactoryName[], s_trajectoryTaskRegistryName[], s_saveProcessTaskFactoryName[], s_saveProcessMainFactoryName[], s_saveProcessMainTaskRegistryName[];
extern unsigned char s_roundDolTaskFactoryName[], s_roundDolMainFactoryName[], s_roundDolMainTaskRegistryName[], s_bgmCollectionTaskFactoryName[], s_bgmCollectionMainFactoryName[], s_bgmCollectionMainTaskRegistryName[];
extern unsigned char s_passwordTaskFactoryName[], s_passwordMainFactoryName[], s_passwordMainTaskRegistryName[];
extern unsigned char s_engineStartupStrings[];
void IntPair_Set(void*, const char*, void*);
void* ActorFactoryRegistry_GetInstance();
void StringMap_SetByName(void*, void*, const char*);
void* TaskRegistry_GetInstance();
void TaskRegistry_AppendFactory(void*, const char*);
void CTitleTask_Create();
void CPangya2TitleMain_Create();
void CShop2Task_Create();
void CShop2Main_Create();
void CGrid_Create();
void CSpin_Create();
void CLetter_Create();
void CSpot_Create();
void CGuideLine_Create();
void CTutorial_Create();
void CFightTask_Create();
void CFightMain_Create();
void CPartyTask_Create();
void CPartyMain_Create();
void CBalloonSystem_Create();
void CDartsSystem_Create();
void CDriveSystem_Create();
void CStoryConte_Create();
void CTourTask_Create();
void CTourMain_Create();
void CScreen_Create();
void CTrainingTask_Create();
void CTrainingMain_Create();
void CSingleTaskRev_Create();
void CSingleMainRev_Create();
void CTrajectory_Create();
void CSaveProcessTask_Create();
void CSaveProcessMain_Create();
void CRoundDolTask_Create();
void CRoundDolMain_Create();
void CBgmCollectionTask_Create();
void CBgmCollectionMain_Create();
void CPasswordTask_Create();
void CPasswordMain_Create();

extern void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern void __ml__FRC7WVectorf(void*, const void*, float);
extern void Matrix_TransformPoint(void*, const void*, const void*);
extern void WView_ProjectViewPoint(void*, void*, void*, int);
extern void W3dAniSpr_DrawIndexedFrameWithMatrix(void*, void*, int, void*, int);
extern float Math_CosApprox(float);
extern float Math_SinApprox(float);
extern int ResourceManager_GetBitmapWidth(int, int);
extern int ResourceManager_GetBitmapHeight(int, int);
extern int ResourceManager_LoadBitmap(int, int, int, int, int);
extern void* __nw__FUl(unsigned int);
extern void BaseObject_dtor(void*, int);
extern void* g_sceneManager;
extern void W3dAniSpr_DrawIndexedFrame(void*, void*, int, int, int, float);
extern const float kFloat_0_AniSprite;
extern const float kFloat_1_AniSprite;

struct Pair { int a; int b; };
extern Pair s_animatedSpriteCornerSelectors[4];

extern void ResourceManager_ReleaseId(void*, int);

// Pool/list ownership: each 24-byte node participates in ONE doubly linked
// circular ring through back/forward. No separate sentinel: a one-node ring
// links to itself and a null head is empty. hashNext is a separate bucket chain.
// head = active payload ring; free = reusable nodes; blocks = allocation-header
// ring. In each allocated block, slot 0 is reserved for the blocks ring; slots
// 1..blockSize-1 enter free. Payload ownership depends on the list: frame data
// owns a SpriteFrame allocation; bitmap data retains a ResourceManager id.
// A node owns its key only when ownsKey != 0. Recycle/free of pool nodes alone
// does not destroy payloads; W3dAniSpr_dtor releases payloads before pool blocks.
struct SpriteHashNode {
    unsigned int data;
    unsigned char* key;
    unsigned char ownsKey;
    char pad[3];
    SpriteHashNode* back;
    SpriteHashNode* forward;
    SpriteHashNode* hashNext;
};
// One of the two per-sprite tables (bitmaps, frames): a circular node ring (head/free/blocks) plus a hash-bucket
// array (bucketCount slots, mask = bucketCount-1) guarded by an OS mutex. Hash = first/last character bits and the
// name length, as in the name registry.
struct SpriteHashList {
    SpriteHashNode* head;
    SpriteHashNode* cursor;
    SpriteHashNode* blocks;
    SpriteHashNode* free;
    int blockSize;
    unsigned int mask;
    int bucketCount;
    SpriteHashNode** buckets;
    char mutex[24];
};
struct SpriteListOwner {
    char pad[0x14];
    SpriteHashList lists[2];
};

struct VecStore { unsigned char data[0x38]; };
struct SpriteVertexStorageView { char objectHeader[0xA8]; VecStore vertices[4]; };

struct WInstanceLock {
    void* lock;
    // Lock a receiver's recursive OS mutex for this lexical scope.
    WInstanceLock(void* l) { lock = l; OSLockMutex(l); }
    // Release this scope's acquisition even during exception unwinding.
    ~WInstanceLock() { OSUnlockMutex(lock); }
};

struct SpriteCamera {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void submit(void*, int, int, int);
};

// Allocate memory from the uniform application heap.
void* AppUniformAllocator_Allocate(unsigned long size) { return appMallocUni__FUl(size); }
// Free memory through the uniform application heap.
void AppUniformAllocator_Free(void* p) { appFreeUni__FPv(p); }
// Forward an allocation request to application heap zero.
void AppHeap_AllocateFromHeap0Thunk(int forwarded0) { AppHeap_AllocateFromHeap0(forwarded0); }
// Forward an array release to application heap zero.
void AppArrayHeap0_FreeThunk(void* forwarded0) { Memory_FreeFromArrayHeap0(forwarded0); }
// Allocate memory from the uniform application heap (second allocator entry).
void* AppUniformAllocator2_Allocate(unsigned long size) { return appMallocUni__FUl(size); }
// Free memory through the uniform application heap (second allocator entry).
void AppUniformAllocator2_Free(void* p) { appFreeUni__FPv(p); }

// Ensure reset callback storage exists; register its destructor before publishing it.
static inline void EnsureStartupResetState(unsigned char* registrations) {
    if (Sing_805DC948() == 0) {
        if (s_systemResetStateInitialized == 0) {
            SystemResetState_Ctor(s_systemResetStorage);
            __register_global_object(s_systemResetStorage, (void*)SystemResetState_Dtor, registrations);
            s_systemResetStateInitialized = 1;
        }
        Sing_805DC948() = s_systemResetStorage;
    }
}

// Ensure input storage exists; registration +0x24 is a byte offset in the startup block.
static inline void EnsureStartupInput(unsigned char* registrations) {
    if (Sing_805DC5C8() == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, registrations + 0x24);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = s_gameInputManager;
    }
}

// Ensure HOME-menu storage exists before its archive or per-frame processing is used.
static inline void EnsureStartupHomeMenu(unsigned char* registrations) {
    if (Sing_805DC820() == 0) {
        if (s_homeMenuInitialized == 0) {
            HomeButtonMenu_InitializeState(s_homeButtonResourceState);
            __register_global_object(s_homeButtonResourceState, (void*)HomeButtonResources_dtor, registrations + 0xC);
            s_homeMenuInitialized = 1;
        }
        Sing_805DC820() = s_homeButtonResourceState;
    }
}

// Ensure loading-indicator storage exists before polling or drawing; publish after registration.
static inline void EnsureStartupLoadingIndicator(unsigned char* registrations) {
    if (Sing_805DC610() == 0) {
        if (s_nowReloadingInitialized == 0) {
            CNowReloading_Init(s_nowReloading);
            __register_global_object(s_nowReloading, (void*)CNowReloading_dtor, registrations + 0x30);
            s_nowReloadingInitialized = 1;
        }
        Sing_805DC610() = s_nowReloading;
    }
}

// Ensure BP-controller storage exists before callbacks; retain separate guard/published-pointer tests.
static inline void EnsureStartupBpController(unsigned char* registrations) {
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, registrations + 0x18);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

// Initialize video/input/audio and pump loading frames; preserve the seven dependency phases below.
// Called once before Game_InitializeProjectAndSharedResources. Owns global service
// initialization; main shuts down media/file/audio services and registered globals
// have their own destructors. Failure is not returned; loading can wait indefinitely.
// Presentation interval, projection, viewport/scissor and capture size are distinct
// controls in the opening lookup. Changing frame rate must also review BP callbacks.
void Game_InitializeEngineAndMedia() {
    // Recheck each singleton at its original use: constructors/callbacks between
    // uses can change globals. Helpers borrow the 12-byte registration records;
    // construction guard and published pointer deliberately remain separate.
    W12 tvFormatConfig;    EngineStartupPacket engineInitPacket;
    F16 projectionMatrix;
    F16 viewMatrix;
    W14 displayModeConfig;
    W14 initialDisplayMode;
    W6 viewportConfig;
    W5 audioConfig;
    W5 outputRectangleConfig;
    W5 initialOutputRectangle;
    W4 fileIoCallbacks;
    W4 scissorConfig;
    W3 cameraEye, cameraTarget, cameraUp;
    W4 extendedInputConfig;
    W3 mediaConfig;
    int genericResourceConfig[2];
    int depthConvention;
    unsigned char* dtorRegistrationBase = gameStartupDtorRegistrations;
    int* startupDefaults = (int*)s_engineStartupDefaults;
    // Phase 1: CPU casts/heap warmup, file-device callbacks, and depth convention.
    OSInitFastCast();
    appFreeUni__FPv(appMallocUni__FUl(10));
    fileIoCallbacks = *(W4*)(startupDefaults + kFileIoCallbackDefaults);
    FileIo_InitializeDevices(&fileIoCallbacks);
    DVDSetAutoFatalMessaging(1);
    depthConvention = s_initialProjectionDepthConvention;
    Projection_SetDepthConvention(&depthConvention);
    // Phase 2: assemble the 36-word engine-init packet. Start with defaults
    // words 0x18..0x30 (100 bytes), then overlay presentation and the VI output rectangle.
    // Packet words 5..18 derive from default display words 0..13; word 16 is
    // overridden to interlaced/progressive scan flags by the TV/DTV test.
    // Engine_BuildRenderMode consumes flags 0x20/0x40. The returned TV
    // config's pixel width/height later replace presentation/output-rectangle dimensions;
    // its interlaced flag controls that branch. Preserve copy order.
    int presentationScanFlags = kInterlacedPresentation;
    VideoConfig_InitFromTvFormat(&tvFormatConfig);
    if (tvFormatConfig.w[kTvInterlacedWord] == 0 && Video_GetDtvOutputCode() == 1) presentationScanFlags = kProgressivePresentation;
    __memcpy(&engineInitPacket, startupDefaults + kEnginePacketDefaults, kStartupPacketTemplateBytes);
    initialDisplayMode = *(W14*)startupDefaults;
    engineInitPacket.displayMirror[0] = startupDefaults[6];
    engineInitPacket.displayMode.scanFlags = startupDefaults[11];
    engineInitPacket.displayMirror[1] = startupDefaults[7];
    engineInitPacket.displayMode.reserved14[0] = startupDefaults[5];
    engineInitPacket.outputRectMirror[4] = startupDefaults[17];
    engineInitPacket.displayMirror[2] = startupDefaults[8];
    engineInitPacket.displayMode.reserved14[1] = startupDefaults[6];
    engineInitPacket.outputRectMirror[0] = startupDefaults[18];
    engineInitPacket.displayMirror[3] = startupDefaults[9];
    engineInitPacket.displayMode.reserved14[2] = startupDefaults[7];
    engineInitPacket.outputRectMirror[1] = startupDefaults[15];
    engineInitPacket.displayMode.reserved14[3] = startupDefaults[8];
    engineInitPacket.outputRectMirror[2] = startupDefaults[16];
    engineInitPacket.displayMirror[4] = startupDefaults[10];
    engineInitPacket.displayMirror[5] = startupDefaults[12];
    engineInitPacket.displayMode.reserved14[4] = startupDefaults[9];
    engineInitPacket.displayMode.swapInterval = startupDefaults[13];
    engineInitPacket.displayMode.reserved14[5] = startupDefaults[10];
    engineInitPacket.outputRect.x = startupDefaults[14];
    engineInitPacket.outputRectMirror[3] = startupDefaults[17];
    engineInitPacket.outputRect.y = startupDefaults[15];
    engineInitPacket.outputRect.width = startupDefaults[16];
    engineInitPacket.displayMode.width = startupDefaults[0];
    engineInitPacket.displayMode.height = startupDefaults[1];
    engineInitPacket.displayMode.reserved08 = startupDefaults[2];
    engineInitPacket.displayMode.reserved0C = startupDefaults[3];
    engineInitPacket.displayMode.modeType = startupDefaults[4];
    engineInitPacket.displayMode.tvFormat = startupDefaults[12];
    initialOutputRectangle = *(W5*)(startupDefaults + kInitialOutputRectDefaults);
    engineInitPacket.outputRect.height = startupDefaults[17];
    engineInitPacket.outputRect.format = startupDefaults[18];
    engineInitPacket.displayMode.scanFlags = presentationScanFlags;
    Engine_InitializeVideoAndRenderTargets(&engineInitPacket);
    // Phase 3: viewport/scissor, resources and loading camera. Perspective uses
    // vertical FOV pi/4 radians, aspect 4/3, near 3 and far 100000 world units.
    viewportConfig = *(W6*)(startupDefaults + kViewportDefaults);
    Engine_SetViewport(&viewportConfig);
    scissorConfig = *(W4*)(startupDefaults + kScissorDefaults);
    Engine_SetScissorRect(&scissorConfig);
    gtl_BitmapFont_Initialize(0);
    genericResourceConfig[0] = s_initialGenericResourceConfigWord0; genericResourceConfig[1] = s_initialGenericResourceConfigWord1;
    gtl_GenericResource_Initialize(genericResourceConfig);
    Render_SetSystemOption(8, 1);
    MtxPerspectiveFovLH(projectionMatrix.f, kStartupVerticalFovRadians, kStartupProjectionAspect, kStartupNearClip, kStartupFarClip);
    SetTransform(kTransformProjection, projectionMatrix.f);
    gtl_GenericRenderer_SetProjectionMatrix(projectionMatrix.f);
    cameraEye = *(W3*)(startupDefaults + kCameraEyeDefaults);
    cameraTarget.w[0] = 0; cameraTarget.w[1] = 0; cameraTarget.w[2] = 0;
    cameraUp = *(W3*)(startupDefaults + kCameraUpDefaults);
    MTXLookAt(viewMatrix.f, &cameraEye, &cameraTarget, &cameraUp);
    SetTransform(kTransformView, viewMatrix.f);
    gtl_GenericRenderer_SetViewMatrix(viewMatrix.f);
    // Phase 4: configure input/audio, then reset callbacks, drive thread, cursor,
    // HOME-menu resources and shared loading/dialog UI in that dependency order.
    extendedInputConfig.w[0] = s_initialExtendedInputConfigWord0; extendedInputConfig.w[1] = s_initialExtendedInputConfigWord1;
    extendedInputConfig.w[2] = startupDefaults[0x44]; extendedInputConfig.w[3] = startupDefaults[0x45];
    gtl_InputEx_Initialize(&extendedInputConfig);
    audioConfig = *(W5*)(startupDefaults + kAudioDefaults);
    gtl_Audio_Initialize(&audioConfig);
    EnsureStartupResetState(dtorRegistrationBase);
    SystemReset_InstallCallbacks(Sing_805DC948());
    CCheckDriveStatus_StartThread();
    Load_HashPathTable(s_engineStartupStrings);
    EnsureStartupInput(dtorRegistrationBase);
    InputManager_LoadCursorResources(Sing_805DC5C8(), 0);
    EnsureStartupHomeMenu(dtorRegistrationBase);
    HomeButton_InitializeSoundArchive(Sing_805DC820());
    EnsureStartupLoadingIndicator(dtorRegistrationBase);
    LoadingIndicator_CreatePlayer(Sing_805DC610());
    LoadingScreen_LoadB2dResources();
    CommonDialogue_LoadSharedResources();
    GameDialogue_LoadSharedResources();
    int sharedUiReady;
    void* bpController;
    // Phase 5: pump input, loading UI/BP callbacks, HOME menu and present until
    // both dialogue archives and the loading state are ready. Test before drawing.
    for (;;) {
        EnsureStartupLoadingIndicator(dtorRegistrationBase);
        if (LoadingIndicator_IsAbsentOrState3(Sing_805DC610()) == 0) {
            LoadingScreen_PollResourceReadiness();
            sharedUiReady = 0;
        } else if (GameDialogue_PollSharedResourcesLoaded() != 0 && CommonDialogue_PollSharedResourcesLoaded() != 0) {
            sharedUiReady = 1;
        } else {
            sharedUiReady = 0;
        }
        if (sharedUiReady != 0) break;
        CCheckDriveStatus_SetSkipDialogDelay(0);
        gtl_InputEx_Update();
        EnsureStartupInput(dtorRegistrationBase);
        InputManager_Update(Sing_805DC5C8());
        Engine_ClearFramebuffer(kFloat_1_FramebufferClear, 0, 0, kStartupClearFlags, 0, 0);
        EnsureStartupLoadingIndicator(dtorRegistrationBase);
        B2dPlayer_UpdateAndDraw(Sing_805DC610());
        EnsureStartupBpController(dtorRegistrationBase);
        bpController = Sing_805DC5F0();
        CBpCtrl_InvokeEntryCallbacks(bpController, 0);
        CBpCtrl_DrawEntries(bpController);
        CBpCtrl_ResetEntryFrameState(bpController);
        EnsureStartupHomeMenu(dtorRegistrationBase);
        HomeButtonMenu_ProcessState(Sing_805DC820());
        CCheckDriveStatus_SetSkipDialogDelay(1);
        Engine_PresentFrame(0, 0, 0, 0);
    }
    // Phase 6: initialize media duration scaling, enable dimming and capture size.
    mediaConfig = *(W3*)(startupDefaults + kMediaDefaults);
    mediaConfig.w[2] = EventTiming_IsDurationScalingEnabled() != 0;
    gtl_Media_Init(&mediaConfig);
    if (Sing_805DC960() == 0) {
        if (s_videoDimmingStateInitialized == 0) {
            s_screenDimmingState = 1;
            VIEnableDimming(1);
            s_videoDimmingStateInitialized = 1;
        }
        Sing_805DC960() = &s_screenDimmingState;
    }
    unsigned char* dimmingEnabled = (unsigned char*)Sing_805DC960();
    if (*dimmingEnabled != 1) {
        *dimmingEnabled = 1;
        VIEnableDimming(1);
    }
    Render_SetSystemOption(6, 0);
    TextureCapture_SetSharedRenderTargetSize(kSharedCaptureWidthPixels, kSharedCaptureHeightPixels);
    // Phase 7: optional TV/DTV display switch after loading. Copy initial settings
    // before patching their TV words, then present up to 100 black frames when
    // scan mode is not 2. modeChangeOrFrame reuses the branch flag as a loop index.
    outputRectangleConfig = initialOutputRectangle;
    displayModeConfig = initialDisplayMode;
    int modeChangeOrFrame = 0, blackFrameCount = 0;
    VideoConfig_InitFromTvFormat(&tvFormatConfig);
    if (tvFormatConfig.w[kTvInterlacedWord] == 0 && Video_GetDtvOutputCode() == 1) {
        modeChangeOrFrame = 1;
        if (VIGetScanMode() != 2) blackFrameCount = kScanSwitchBlackPresentCalls;
        displayModeConfig.w[kDisplayScanFlagsWord] = kProgressivePresentation;
        displayModeConfig.w[kDisplayWidthWord] = tvFormatConfig.w[kTvWidthWord]; displayModeConfig.w[kDisplayHeightWord] = tvFormatConfig.w[kTvHeightWord];
        outputRectangleConfig.w[kOutputRectWidthWord] = tvFormatConfig.w[kTvWidthWord]; outputRectangleConfig.w[kOutputRectHeightWord] = tvFormatConfig.w[kTvHeightWord];
    }
    if (modeChangeOrFrame != 0) {
        Engine_SetOutputRectangle(&outputRectangleConfig);
        Engine_ChangeDisplayMode(&displayModeConfig);
        for (modeChangeOrFrame = 0; modeChangeOrFrame < blackFrameCount; ++modeChangeOrFrame) {
            Engine_ClearFramebuffer(kFloat_1_FramebufferClear, 0, 0, kStartupClearFlags, 0, 0);
            Engine_PresentFrame(0, 0, 0, 0);
        }
    }
}

// Run language setup and the game loop, then shut down game, media and engine services.
void main() {
    Language_InitializeFromSystemSettings();
    Game_InitializeEngineAndMedia();
    if (Game_InitializeProjectAndSharedResources(0) != 0) {
        Game_MainLoop(0);
    }
    Game_Shutdown();
    HashPath_Shutdown();
    CCheckDriveStatus_StopThread();
    CommonDialogue_ReleaseSharedResources();
    GameDialogue_ReleaseSharedResources();
    FileRegistry_ClearOwnedDevices();
    InputDriver_InitStub();
    gtl_Audio_Shutdown();
    Renderer_EmptyHook();
    Projection_ResetDepthConvention();
    MediaRegistry_ResetGlobalPointer();
}

// Retail logging entry point; messages are discarded.
void OSReport(const char*, ...) { }

enum RuntimeTypeNameOffset {
    kTypeNameTitleTask = 12,
    kTypeNamePangya2TitleMain = 23,
    kTypeNameShop2Task = 41,
    kTypeNameShop2Main = 52,
    kTypeNameGrid = 63,
    kTypeNameSpin = 69,
    kTypeNameLetter = 75,
    kTypeNameSpot = 83,
    kTypeNameGuideLine = 89,
    kTypeNameTutorial = 100,
    kTypeNameFightTask = 110,
    kTypeNameFightMain = 121,
    kTypeNamePartyTask = 132,
    kTypeNamePartyMain = 143,
    kTypeNameBalloonSystem = 154,
    kTypeNameDartsSystem = 169,
    kTypeNameDriveSystem = 182,
    kTypeNameStoryConte = 195,
    kTypeNameTourTask = 207,
    kTypeNameTourMain = 217,
    kTypeNameScreen = 227,
    kTypeNameTrainingTask = 235,
    kTypeNameTrainingMain = 249,
    kTypeNameSingleTaskRev = 263,
    kTypeNameSingleMainRev = 278,
    kTypeNameTrajectory = 293,
    kTypeNameSaveProcessTask = 305,
    kTypeNameSaveProcessMain = 322,
    kTypeNameRoundDolTask = 339,
    kTypeNameRoundDolMain = 353,
    kTypeNameBgmCollectionTask = 367,
    kTypeNameBgmCollectionMain = 386,
    kTypeNamePasswordTask = 405,
    kTypeNamePasswordMain = 419
};
// Register runtime type descriptors, actor constructors and task factory names during static initialization.
void TaskRegistry_RegisterFactories() {
    // Each group first publishes RTTI (name+base), then its actor constructor;
    // task-backed actors additionally append their task name. Calls deliberately
    // reacquire registry singletons between registrations; strings are borrowed.
    s_engineTwoPi = kFloat_6_2831855;
    unsigned char* typeNameStrings = s_engineStartupStrings;
    void* actorFactories;
    void* taskFactories;
    IntPair_Set(s_titleTaskRttiDescriptor, (const char*)typeNameStrings + kTypeNameTitleTask, s_taskTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CTitleTask_Create, (const char*)s_titleTaskFactoryName);
    IntPair_Set(s_titleMainRttiDescriptor, (const char*)typeNameStrings + kTypeNamePangya2TitleMain, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CPangya2TitleMain_Create, (const char*)s_pangya2TitleMainFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_pangya2TitleMainTaskRegistryName);
    IntPair_Set(s_shop2TaskRttiDescriptor, (const char*)typeNameStrings + kTypeNameShop2Task, s_taskTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CShop2Task_Create, (const char*)s_shop2TaskFactoryName);
    IntPair_Set(s_shop2MainTypeInfo, (const char*)typeNameStrings + kTypeNameShop2Main, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CShop2Main_Create, (const char*)s_shop2MainFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_shop2MainTaskRegistryName);
    IntPair_Set(s_gridTypeInfo, (const char*)typeNameStrings + kTypeNameGrid, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CGrid_Create, (const char*)s_gridFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_gridTaskRegistryName);
    IntPair_Set(s_spinTypeInfo, (const char*)typeNameStrings + kTypeNameSpin, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CSpin_Create, (const char*)s_spinFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_spinTaskRegistryName);
    IntPair_Set(s_letterTypeInfo, (const char*)typeNameStrings + kTypeNameLetter, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CLetter_Create, (const char*)s_letterFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_letterTaskRegistryName);
    IntPair_Set(s_spotTypeInfo, (const char*)typeNameStrings + kTypeNameSpot, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CSpot_Create, (const char*)s_spotFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_spotTaskRegistryName);
    IntPair_Set(s_guideLineTypeInfo, (const char*)typeNameStrings + kTypeNameGuideLine, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CGuideLine_Create, (const char*)s_guideLineFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_guideLineTaskRegistryName);
    IntPair_Set(s_tutorialRttiDescriptor, (const char*)typeNameStrings + kTypeNameTutorial, s_taskMainTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CTutorial_Create, (const char*)s_tutorialFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_tutorialTaskRegistryName);
    IntPair_Set(s_fightTaskTypeDescriptor, (const char*)typeNameStrings + kTypeNameFightTask, s_taskTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CFightTask_Create, (const char*)s_fightTaskFactoryName);
    IntPair_Set(s_fightMainTypeDescriptor, (const char*)typeNameStrings + kTypeNameFightMain, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CFightMain_Create, (const char*)s_fightMainFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_fightMainTaskRegistryName);
    IntPair_Set(s_partyTaskTypeDescriptor, (const char*)typeNameStrings + kTypeNamePartyTask, s_taskTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CPartyTask_Create, (const char*)s_partyTaskFactoryName);
    IntPair_Set(s_partyMainTypeDescriptor, (const char*)typeNameStrings + kTypeNamePartyMain, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CPartyMain_Create, (const char*)s_partyMainFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_partyMainTaskRegistryName);
    IntPair_Set(s_balloonSystemTypeInfo, (const char*)typeNameStrings + kTypeNameBalloonSystem, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CBalloonSystem_Create, (const char*)s_balloonSystemFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_balloonSystemTaskRegistryName);
    IntPair_Set(s_dartsSystemRttiDescriptor, (const char*)typeNameStrings + kTypeNameDartsSystem, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CDartsSystem_Create, (const char*)s_dartsSystemFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_dartsSystemTaskRegistryName);
    IntPair_Set(s_driveSystemTypeInfo, (const char*)typeNameStrings + kTypeNameDriveSystem, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CDriveSystem_Create, (const char*)s_driveSystemFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_driveSystemTaskRegistryName);
    IntPair_Set(s_storyConteTypeInfo, (const char*)typeNameStrings + kTypeNameStoryConte, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CStoryConte_Create, (const char*)s_storyConteFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_storyConteTaskRegistryName);
    IntPair_Set(s_tourTaskRuntimeType, (const char*)typeNameStrings + kTypeNameTourTask, s_taskTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CTourTask_Create, (const char*)s_tourTaskFactoryName);
    IntPair_Set(s_tourMainTypeInfo, (const char*)typeNameStrings + kTypeNameTourMain, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CTourMain_Create, (const char*)s_tourMainFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_tourMainTaskRegistryName);
    IntPair_Set(s_screenRttiDescriptor, (const char*)typeNameStrings + kTypeNameScreen, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CScreen_Create, (const char*)s_screenFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_screenTaskRegistryName);
    IntPair_Set(s_trainingTaskRttiDescriptor, (const char*)typeNameStrings + kTypeNameTrainingTask, s_taskTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CTrainingTask_Create, (const char*)s_trainingTaskFactoryName);
    IntPair_Set(s_trainingMainTypeInfo, (const char*)typeNameStrings + kTypeNameTrainingMain, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CTrainingMain_Create, (const char*)s_trainingMainFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_trainingMainTaskRegistryName);
    IntPair_Set(s_singleTaskRevTypeDescriptor, (const char*)typeNameStrings + kTypeNameSingleTaskRev, s_taskTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CSingleTaskRev_Create, (const char*)s_singleTaskRevFactoryName);
    IntPair_Set(s_singleMainRevTypeDescriptor, (const char*)typeNameStrings + kTypeNameSingleMainRev, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CSingleMainRev_Create, (const char*)s_singleMainRevFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_singleMainRevTaskRegistryName);
    IntPair_Set(s_trajectoryTypeInfo, (const char*)typeNameStrings + kTypeNameTrajectory, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CTrajectory_Create, (const char*)s_trajectoryFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_trajectoryTaskRegistryName);
    IntPair_Set(s_saveProcessTaskRtti, (const char*)typeNameStrings + kTypeNameSaveProcessTask, s_taskTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CSaveProcessTask_Create, (const char*)s_saveProcessTaskFactoryName);
    IntPair_Set(s_saveProcessMainRtti, (const char*)typeNameStrings + kTypeNameSaveProcessMain, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CSaveProcessMain_Create, (const char*)s_saveProcessMainFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_saveProcessMainTaskRegistryName);
    IntPair_Set(s_roundDolTaskTypeInfo, (const char*)typeNameStrings + kTypeNameRoundDolTask, s_taskTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CRoundDolTask_Create, (const char*)s_roundDolTaskFactoryName);
    IntPair_Set(s_roundDolMainTypeInfo, (const char*)typeNameStrings + kTypeNameRoundDolMain, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CRoundDolMain_Create, (const char*)s_roundDolMainFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_roundDolMainTaskRegistryName);
    IntPair_Set(s_bgmCollectionTaskRttiDescriptor, (const char*)typeNameStrings + kTypeNameBgmCollectionTask, s_taskTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CBgmCollectionTask_Create, (const char*)s_bgmCollectionTaskFactoryName);
    IntPair_Set(s_bgmCollectionMainTypeInfo, (const char*)typeNameStrings + kTypeNameBgmCollectionMain, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CBgmCollectionMain_Create, (const char*)s_bgmCollectionMainFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_bgmCollectionMainTaskRegistryName);
    IntPair_Set(s_passwordTaskTypeDescriptor, (const char*)typeNameStrings + kTypeNamePasswordTask, s_taskTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CPasswordTask_Create, (const char*)s_passwordTaskFactoryName);
    IntPair_Set(s_passwordMainTypeDescriptor, (const char*)typeNameStrings + kTypeNamePasswordMain, s_actorTypeInfo);
    actorFactories = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactories, (void*)CPasswordMain_Create, (const char*)s_passwordMainFactoryName);
    taskFactories = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(taskFactories, (const char*)s_passwordMainTaskRegistryName);
}


#pragma push
#pragma opt_propagation off
#pragma opt_strength_reduction off
// Initialize bitmap/frame containers and four white vertices with default corner UVs.
void* W3dAniSpr_Ctor(void* self) {
    // Phase 1: initialize independent bitmap/frame rings, hash buckets and mutexes.
    // In plain C: for each of the two tables { init mutex; lock; empty ring; blockSize = bucketCount = 16;
    // buckets = calloc(16 pointers) with mask 15; unlock; }.
    BaseObject_ctor(self);
    AsW3dAniSpr(self)->resourceManager = 0;
    AsW3dAniSpr(self)->vtbl = __vt__9W3dAniSpr;
    AsW3dAniSpr(self)->bitmapBuckets = 0;
    OSInitMutex(AsW3dAniSpr(self)->bitmapMutex);
    void* lock1 = AsW3dAniSpr(self)->bitmapMutex;
    OSLockMutex(lock1);
    AsW3dAniSpr(self)->bitmapHead = 0;
    AsW3dAniSpr(self)->bitmapFree = 0;
    AsW3dAniSpr(self)->bitmapBlocks = 0;
    AsW3dAniSpr(self)->bitmapBlockSize = 16;
    AsW3dAniSpr(self)->bitmapCursor = 0;
    AsW3dAniSpr(self)->bitmapBucketCount = 16;
    SpriteHashList* first = (SpriteHashList*)((char*)&AsW3dAniSpr(self)->bitmapHead);
    if (first->bucketCount > 0) {
        first->buckets = (SpriteHashNode**)appMallocUni__FUl(first->bucketCount * 4);
        first->mask = first->bucketCount - 1;
        for (int i = 0; i < first->bucketCount; ++i) first->buckets[i] = 0;
    } else {
        first->buckets = 0;
    }
    OSUnlockMutex(lock1);

    AsW3dAniSpr(self)->frameBuckets = 0;
    OSInitMutex(AsW3dAniSpr(self)->frameMutex);
    void* lock2 = AsW3dAniSpr(self)->frameMutex;
    OSLockMutex(lock2);
    AsW3dAniSpr(self)->frameHead = 0;
    AsW3dAniSpr(self)->frameFree = 0;
    AsW3dAniSpr(self)->frameBlocks = 0;
    AsW3dAniSpr(self)->frameBlockSize = 16;
    AsW3dAniSpr(self)->frameCursor = 0;
    AsW3dAniSpr(self)->frameBucketCount = 16;
    SpriteHashList* second = (SpriteHashList*)((char*)&AsW3dAniSpr(self)->frameHead);
    if (second->bucketCount > 0) {
        second->buckets = (SpriteHashNode**)appMallocUni__FUl(second->bucketCount * 4);
        second->mask = second->bucketCount - 1;
        for (int i = 0; i < second->bucketCount; ++i) second->buckets[i] = 0;
    } else {
        second->buckets = 0;
    }
    OSUnlockMutex(lock2);

    // Phase 2: vertex pointers borrow embedded 0x38-byte storage; the fifth pointer
    // is a null terminator. This is a V,U selector table, NOT U,V: the actual
    // (U,V) corners are (lo,lo),(lo,hi),(hi,hi),(hi,lo), lo=.5/256 and
    // hi=255.5/256. Each selector is 0 or 1, mapped by
    // (255*selector + 0.5)/256 to texel centers in a nominal 256x256 texture.
    // This establishes the stored axes without inferring artistic intent.
    // W3dAniSpr_DrawFrame and DrawFrameWithMatrix replace these defaults with
    // frame leftU/topV/rightU/bottomV before textured submission; their corner
    // order is separate. To rotate/flip a frame, change that rectangle/corner
    // policy too; changing constructor selectors alone is overwritten there.
    VecStore* vertex = ((SpriteVertexStorageView*)self)->vertices;
    VecStore** vertices = (VecStore**)((char*)&AsW3dAniSpr(self)->vertex0);
    int* coords = (int*)s_animatedSpriteDefaultUvSelectors;
    int k;
    for (k = 0; k < 4; k += 2) {
        vertices[k] = vertex++;
        ((ProjectedSpriteVertex*)vertices[k])->textureV = (kFloat_255_VertexTexCoord * (float)coords[0] + kFloat_0_5_TextureCoordinates) * kFloat_0_00390625_TextureCoords;
        ((ProjectedSpriteVertex*)vertices[k])->textureU = (kFloat_255_VertexTexCoord * (float)coords[1] + kFloat_0_5_TextureCoordinates) * kFloat_0_00390625_TextureCoords;
        (*(int*)&AsW3dSprVertex(vertices[k]->data)->color) = -1;
        vertices[k + 1] = vertex++;
        ((ProjectedSpriteVertex*)vertices[k + 1])->textureV = (kFloat_255_VertexTexCoord * (float)coords[2] + kFloat_0_5_TextureCoordinates) * kFloat_0_00390625_TextureCoords;
        ((ProjectedSpriteVertex*)vertices[k + 1])->textureU = (kFloat_255_VertexTexCoord * (float)coords[3] + kFloat_0_5_TextureCoordinates) * kFloat_0_00390625_TextureCoords;
        (*(int*)&AsW3dSprVertex(vertices[k + 1]->data)->color) = -1;
        coords += 4;
    }
    vertices[k] = 0;
    AsW3dAniSpr(self)->bitmapCount = 0;
    AsW3dAniSpr(self)->frameCount = 0;
    return self;
}
#pragma pop

// Release all container allocation blocks and hash buckets, optionally deleting the container.
// Caller must already dispose active payloads/owned keys. This releases backing
// allocations only. Mutex must be recursive: inner unlink takes it again.
void* SpriteCleanupList_dtor(void* self, int shouldDelete) {
    if (self == 0) return self;
    {
        WInstanceLock guard(((SpriteHashList*)self)->mutex);
        if ((*(void**)&((SpriteHashList*)self)->blocks) != 0) {
            void* lockPtr = ((SpriteHashList*)self)->mutex;
            while ((*(void**)&((SpriteHashList*)self)->blocks) != 0) {
                void* node = (*(void**)&((SpriteHashList*)self)->blocks);
                OSLockMutex(lockPtr);
                void* next = (*(void**)&((SpriteHashNode*)node)->back);
                void* prev = (*(void**)&((SpriteHashNode*)node)->forward);
                void* newHead = prev;
                (*(void**)&((SpriteHashNode*)prev)->back) = next;
                (*(void**)&((SpriteHashNode*)next)->forward) = prev;
                if (newHead == node) newHead = 0;
                OSUnlockMutex(lockPtr);
                (*(void**)&((SpriteHashList*)self)->blocks) = newHead;
                appFreeUni__FPv(node);
            }
        }
        void* p = (*(void**)&((SpriteHashList*)self)->buckets);
        if (p != 0) {
            appFreeUni__FPv(p);
            (*(void**)&((SpriteHashList*)self)->buckets) = 0;
        }
    }
    if (shouldDelete > 0) __dl__FPv(self);
    return self;
}

// One view macro keeps the container expression visible to CodeWarrior's inliner;
// an accessor function increases destructor register pressure. Members stay explicit.
// Borrow the bitmap or frame cleanup ring; preserve fallback to frames for every
// listKind other than kBitmapList (internal callers use only the two enum values).
// Keep direct expansion: the reference-returning inline form changes code and
// cleanup relocations. SpriteListOwner above is the equivalent typed schema.
#define SpriteCleanupList(self, listKind) (((SpriteListOwner*)(self))->lists[(listKind) == kBitmapList ? 0 : 1])
// Hash a string key into the selected container's bucket mask while holding its mutex.
static inline unsigned int cleanupHash(void* self, int offset, unsigned char* key) {
    OSLockMutex(SpriteCleanupList(self, offset).mutex);
    int length = strlen((char*)key);
    unsigned int index = length > 0 ? length - 1 : 0;
    unsigned int hash = ((key[0] & 0x2a) | (key[index] & 0x15) | (length << 6)) & SpriteCleanupList(self, offset).mask;
    OSUnlockMutex(SpriteCleanupList(self, offset).mutex);
    return hash;
}

// Detach a node from a circular list and update its head if necessary.
static inline SpriteHashNode* cleanupUnlink(void* self, int offset, SpriteHashNode* head, SpriteHashNode* node) {
    OSLockMutex(SpriteCleanupList(self, offset).mutex);
    node->forward->back = node->back;
    node->back->forward = node->forward;
    if (node != head) {
    } else {
        if (node->forward == head) head = 0;
        else head = node->forward;
    }
    OSUnlockMutex(SpriteCleanupList(self, offset).mutex);
    return head;
}

// Detach a node and return its successor, or null for the final node.
static inline SpriteHashNode* cleanupPop(void* self, int offset, SpriteHashNode* node) {
    OSLockMutex(SpriteCleanupList(self, offset).mutex);
    node->forward->back = node->back;
    node->back->forward = node->forward;
    SpriteHashNode* next = node->forward;
    if (next == node) next = 0;
    OSUnlockMutex(SpriteCleanupList(self, offset).mutex);
    return next;
}

// Insert a node before a circular list head, creating a one-node ring when empty.
static inline SpriteHashNode* cleanupInsert(void* self, int offset, SpriteHashNode* head, SpriteHashNode* node) {
    OSLockMutex(SpriteCleanupList(self, offset).mutex);
    if (!head) {
        node->back = node;
        node->forward = node;
        OSUnlockMutex(SpriteCleanupList(self, offset).mutex);
        return node;
    }
    node->forward = head;
    node->back = head->back;
    head->back = node;
    node->back->forward = node;
    OSUnlockMutex(SpriteCleanupList(self, offset).mutex);
    return head;
}

// Remove a node from its string-key hash chain.
static inline void cleanupRemoveKey(void* self, int offset, SpriteHashNode* node) {
    WInstanceLock lock(SpriteCleanupList(self, offset).mutex);
    unsigned int hash = cleanupHash(self, offset, node->key);
    SpriteHashNode* cur = SpriteCleanupList(self, offset).buckets[hash];
    if (cur == node) SpriteCleanupList(self, offset).buckets[hash] = node->hashNext;
    else {
        while (cur) {
            if (cur->hashNext == node) {
                cur->hashNext = node->hashNext;
                return;
            }
            cur = cur->hashNext;
        }
    }
}

// Remove the first matching payload, release its owned key and recycle the node.
static inline void cleanupRemove(void* self, int offset, unsigned int data) {
    WInstanceLock lock(SpriteCleanupList(self, offset).mutex);
    SpriteHashNode* head = SpriteCleanupList(self, offset).head;
    SpriteHashNode* node = head;
    if (!head) return;
    do {
        if (offset == kBitmapList ? (int)node->data == (int)data : node->data == data) {
            if (SpriteCleanupList(self, offset).cursor == node) {
                SpriteHashNode* next = SpriteCleanupList(self, offset).cursor->forward;
                if (next == head) next = 0;
                SpriteCleanupList(self, offset).cursor = next;
            }
            if (node->key) {
                cleanupRemoveKey(self, offset, node);
                if (node->ownsKey) appFreeUni__FPv(node->key);
            }
            SpriteCleanupList(self, offset).head = cleanupUnlink(self, offset, SpriteCleanupList(self, offset).head, node);
            SpriteCleanupList(self, offset).free = cleanupInsert(self, offset, SpriteCleanupList(self, offset).free, node);
            if (!SpriteCleanupList(self, offset).head) SpriteCleanupList(self, offset).cursor = 0;
            return;
        }
        node = node->forward;
    } while (node != head);
}

// Reset iteration to the head and return its payload, advancing the shared cursor.
static inline unsigned int cleanupFirst(void* self, int offset) {
    unsigned int data;
    WInstanceLock lock(SpriteCleanupList(self, offset).mutex);
    SpriteCleanupList(self, offset).cursor = SpriteCleanupList(self, offset).head;
    OSLockMutex(SpriteCleanupList(self, offset).mutex);
    SpriteHashNode* node = SpriteCleanupList(self, offset).cursor;
    if (node) {
        SpriteHashNode* next = node->forward;
        data = node->data;
        if (next == SpriteCleanupList(self, offset).head) next = 0;
        SpriteCleanupList(self, offset).cursor = next;
        OSUnlockMutex(SpriteCleanupList(self, offset).mutex);
    } else {
        OSUnlockMutex(SpriteCleanupList(self, offset).mutex);
        data = 0;
    }
    return data;
}

// Recycle all active nodes and clear the hash buckets.
static inline void cleanupClear(void* self, int offset) {
    WInstanceLock lock(SpriteCleanupList(self, offset).mutex);
    if (SpriteCleanupList(self, offset).head && SpriteCleanupList(self, offset).free) {
        SpriteCleanupList(self, offset).head->back->forward = SpriteCleanupList(self, offset).free->forward;
        SpriteCleanupList(self, offset).free->forward->back = SpriteCleanupList(self, offset).head->back;
        SpriteCleanupList(self, offset).free->forward = SpriteCleanupList(self, offset).head;
        SpriteCleanupList(self, offset).head->back = SpriteCleanupList(self, offset).free;
        SpriteCleanupList(self, offset).head = 0;
    } else if (SpriteCleanupList(self, offset).head) {
        SpriteHashNode* node;
        while ((node = SpriteCleanupList(self, offset).head) != 0) {
            SpriteHashNode* next = cleanupPop(self, offset, node);
            SpriteCleanupList(self, offset).free = cleanupInsert(self, offset, SpriteCleanupList(self, offset).free, SpriteCleanupList(self, offset).head);
            SpriteCleanupList(self, offset).head = next;
        }
    }
    for (int i = 0; i < SpriteCleanupList(self, offset).bucketCount; ++i) SpriteCleanupList(self, offset).buckets[i] = 0;
}


#pragma use_lmw_stmw on
// Keep the byte address forms below: member-address forms change destructor register allocation.
// Delete frame descriptors, release bitmap resources and destroy both containers.
void* W3dAniSpr_dtor(void* self, int doDelete) {
    if (self) {
        // Phase 1: remove/delete frame payloads and release retained bitmap ids;
        // removing a node recycles it into its pool without freeing its block.
        AsW3dAniSpr(self)->vtbl = &__vt__9W3dAniSpr;
        unsigned int data;
        while ((data = cleanupFirst(self, kFrameList)) != 0) {
            if (data) {
                cleanupRemove(self, kFrameList, data);
                __dl__FPv((void*)data);
                AsW3dAniSpr(self)->frameCount -= 1;
            }
        }
        cleanupClear(self, kFrameList);
        while ((data = cleanupFirst(self, kBitmapList)) != 0) {
            ResourceManager_ReleaseId((*(void**)&AsW3dAniSpr(self)->resourceManager), (int)data);
            cleanupRemove(self, kBitmapList, data);
        }
        if ((char*)self + kFrameList) {
            WInstanceLock lock((char*)&AsW3dAniSpr(self)->frameMutex);
            if ((*(SpriteHashNode**)&AsW3dAniSpr(self)->frameBlocks)) {
                SpriteHashNode* node;
                while ((node = (*(SpriteHashNode**)&AsW3dAniSpr(self)->frameBlocks)) != 0) {
                    (*(SpriteHashNode**)&AsW3dAniSpr(self)->frameBlocks) = cleanupPop(self, kFrameList, node);
                    appFreeUni__FPv(node);
                }
            }
            if ((*(void**)&AsW3dAniSpr(self)->frameBuckets)) {
                appFreeUni__FPv((*(void**)&AsW3dAniSpr(self)->frameBuckets));
                (*(void**)&AsW3dAniSpr(self)->frameBuckets) = 0;
            }
        }
        if ((char*)self + kBitmapList) {
            WInstanceLock lock((char*)self + kBitmapMutexOffset);
            if ((*(SpriteHashNode**)&AsW3dAniSpr(self)->bitmapBlocks)) {
                SpriteHashNode* node;
                while ((node = (*(SpriteHashNode**)&AsW3dAniSpr(self)->bitmapBlocks)) != 0) {
                    (*(SpriteHashNode**)&AsW3dAniSpr(self)->bitmapBlocks) = cleanupPop(self, kBitmapList, node);
                    appFreeUni__FPv(node);
                }
            }
            if ((*(void**)&AsW3dAniSpr(self)->bitmapBuckets)) {
                appFreeUni__FPv((*(void**)&AsW3dAniSpr(self)->bitmapBuckets));
                (*(void**)&AsW3dAniSpr(self)->bitmapBuckets) = 0;
            }
        }
        if (self) BaseObject_dtor(self, 0);
        if (doDelete > 0) __dl__FPv(self);
    }
    return self;
}

// Return the next borrowed SpriteFrame payload; stop when the ring wraps.
// Lifetime/mutation contract shared by DrawFrame, DrawFrameWithMatrix and
// W3dAniSpr_GetFrameDescription (font_markup_text_rendering.cpp): the lock protects one cursor step,
// not the returned payload or a whole traversal. The sprite owns each separately
// allocated frame; the borrower must not delete it. Appending grid frames grows
// node pools without moving existing frame allocations, but changes traversal
// order and is not permitted during a draw/lookup. Every traversal resets/advances
// the SAME frameCursor, so even a second read traversal must be serialized.
// cleanupRemove unlinks/recycles the node and repairs a cursor pointing to it;
// it does not free data. W3dAniSpr_dtor then deletes that data and releases bitmap
// handles. Thus cursor repair provides no lifetime pin for previously returned
// pointers. Stop drawing/lookup/editing before teardown, and discard all borrowed
// frame pointers then. No public frame-replacement synchronization is implemented.
// A future UV edit must serialize against projection/submission and keep bitmapId
// live: LoadBitmapGrid owns its retained id, AddBitmapGridFrames borrows the id
// from its caller. Copying a descriptor does not retain that bitmap. Draws also
// mutate shared vertex scratch, so disjoint frame indices do not allow parallel
// draws on one sprite. CSnow_Ctor (pvsfxbox_sky_and_wind_particles.cpp) copies frame 0's values immediately;
// it does not take ownership of the frame or extend the texture's lifetime.
static inline void* spriteNext(SpriteHashList* L) {
    void* data;
    void* m = &L->mutex;
    OSLockMutex(m);
    SpriteHashNode* node = L->cursor;
    if (node) {
        SpriteHashNode* next = node->forward;
        data = (void*)node->data;
        if (next == L->head) next = 0;
        L->cursor = next;
        OSUnlockMutex(m);
    } else {
        OSUnlockMutex(m);
        data = 0;
    }
    return data;
}

// Set the sprite's world position and return the assigned vector.
WVector& W3dAniSpr_SetPosition(void* self, const WVector& value) {
    return AsW3dAniSpr(self)->position = value;
}

#pragma opt_common_subs off
#pragma opt_strength_reduction off
#pragma opt_lifetimes off
#pragma push
#pragma always_inline on
// Complete the projected rectangle by copying only position, reciprocal-W and clipping fields.
static inline void ReconstructBillboardProjectedCorners(void* self) {
    // Phase 4: reconstruct the other two projected corners by component mixing:
    // v1 = (v2.x, v0.y, v0.z, v0.reciprocalW, v0.clipFlags),
    // v3 = (v0.x, v2.y, v2.z, v2.reciprocalW, v2.clipFlags).
    // This is a screen-aligned rectangle from two projected corners, not a
    // general four-corner rotated polygon. Keep the individual copy scopes:
    {
        void* sourceVertex = AsW3dAniSpr(self)->vertex0;
        void* destinationVertex = AsW3dAniSpr(self)->vertex1;
        ((ProjectedSpriteVertex*)destinationVertex)->reciprocalW = ((ProjectedSpriteVertex*)sourceVertex)->reciprocalW;
    }
    {
        void* sourceVertex = AsW3dAniSpr(self)->vertex2;
        void* destinationVertex = AsW3dAniSpr(self)->vertex1;
        ((ProjectedSpriteVertex*)destinationVertex)->x = ((ProjectedSpriteVertex*)sourceVertex)->x;
    }
    {
        void* sourceVertex = AsW3dAniSpr(self)->vertex0;
        void* destinationVertex = AsW3dAniSpr(self)->vertex1;
        ((ProjectedSpriteVertex*)destinationVertex)->y = ((ProjectedSpriteVertex*)sourceVertex)->y;
    }
    {
        void* sourceVertex = AsW3dAniSpr(self)->vertex0;
        void* destinationVertex = AsW3dAniSpr(self)->vertex1;
        ((ProjectedSpriteVertex*)destinationVertex)->z = ((ProjectedSpriteVertex*)sourceVertex)->z;
    }
    {
        void* sourceVertex = AsW3dAniSpr(self)->vertex0;
        void* destinationVertex = AsW3dAniSpr(self)->vertex1;
        ((ProjectedSpriteVertex*)destinationVertex)->clipFlags = ((ProjectedSpriteVertex*)sourceVertex)->clipFlags;
    }
    {
        void* sourceVertex = AsW3dAniSpr(self)->vertex2;
        void* destinationVertex = AsW3dAniSpr(self)->vertex3;
        ((ProjectedSpriteVertex*)destinationVertex)->reciprocalW = ((ProjectedSpriteVertex*)sourceVertex)->reciprocalW;
    }
    {
        void* sourceVertex = AsW3dAniSpr(self)->vertex0;
        void* destinationVertex = AsW3dAniSpr(self)->vertex3;
        ((ProjectedSpriteVertex*)destinationVertex)->x = ((ProjectedSpriteVertex*)sourceVertex)->x;
    }
    {
        void* sourceVertex = AsW3dAniSpr(self)->vertex2;
        void* destinationVertex = AsW3dAniSpr(self)->vertex3;
        ((ProjectedSpriteVertex*)destinationVertex)->y = ((ProjectedSpriteVertex*)sourceVertex)->y;
    }
    {
        void* sourceVertex = AsW3dAniSpr(self)->vertex2;
        void* destinationVertex = AsW3dAniSpr(self)->vertex3;
        ((ProjectedSpriteVertex*)destinationVertex)->z = ((ProjectedSpriteVertex*)sourceVertex)->z;
    }
    {
        void* sourceVertex = AsW3dAniSpr(self)->vertex2;
        void* destinationVertex = AsW3dAniSpr(self)->vertex3;
        ((ProjectedSpriteVertex*)destinationVertex)->clipFlags = ((ProjectedSpriteVertex*)sourceVertex)->clipFlags;
    }

}
#pragma pop

// Select a frame, project a rotated billboard and submit its textured or white quad.
// view borrows a WView. The float argument has TWO observed meanings: radians
// in the direct billboard path, but scale in the textured indexed fallback
// (font_markup_text_rendering.cpp stores unsigned(1024*value) and alpha byte 5). Do not normalize
// these into a single unit or change that byte while adjusting aspect. The
// original scene-phase gate chooses the meaning. frameIndex traverses from the
// first frame (negative values also select the first); out-of-range returns.
// Traversal locks each cursor step, not the entire draw: caller must keep frame
// payloads alive throughout projection/submission. Draw overwrites embedded vertices.
// Full mutation/invalidation rules are beside spriteNext, including the shared
// cursor and borrowed bitmap case; a cursor-step mutex is not a draw lock.
void W3dAniSpr_DrawFrame(void* self, void* view, float rotationRadiansOrIndexedScale, int flags, int frameIndex) {
    int skippedFrames;
    void* frameMutex;
    // Scene render policy (course_scene_render_and_geometry.cpp): the deferred/effect phase sets effectRenderPassActive.
    // Textured sprites then use the indexed draw packet (UVs + placement, projected by its consumer).
    // Ordinary scene drawing projects vertices here; untextured flags always require that path.
    // With no scene manager, use the same indexed fallback for textured sprites.
    if (g_sceneManager == 0 || ((CSceneManager*)g_sceneManager)->effectRenderPassActive != 0) {
        if (!(flags & kUntexturedRenderFlags)) {
            W3dAniSpr_DrawIndexedFrame(self, view, flags, kIndexedDrawOpacity, frameIndex, rotationRadiansOrIndexedScale);
            return;
        }
    }

    // Phase 1: select the frame in the circular container; stop at wrap rather than repeat.
    void* frame;
    {
        WInstanceLock guard(AsW3dAniSpr(self)->frameMutex);
        AsW3dAniSpr(self)->frameCursor = AsW3dAniSpr(self)->frameHead;
        frame = spriteNext((SpriteHashList*)((char*)&AsW3dAniSpr(self)->frameHead));
    }
    frameMutex = AsW3dAniSpr(self)->frameMutex;
    skippedFrames = 0;
    while (skippedFrames < frameIndex && frame) {
        SpriteHashNode* next;
        SpriteHashNode* node;
        OSLockMutex(frameMutex);
        SpriteHashList* frameList = (SpriteHashList*)((char*)&AsW3dAniSpr(self)->frameHead);
        node = frameList->cursor;
        if (node) {
            next = node->forward;
            frame = (void*)node->data;
            if (next == frameList->head) next = 0;
            frameList->cursor = next;
            OSUnlockMutex(frameMutex);
        } else {
            OSUnlockMutex(frameMutex);
            frame = 0;
        }
        ++skippedFrames;
    }
    if (!frame) return;

    // Phase 2: assign the frame rectangle's UVs independently of its projected geometry.
    AsW3dAniSpr(self)->vertex0U = ((SpriteFrame*)frame)->leftU;
    AsW3dAniSpr(self)->vertex0V = ((SpriteFrame*)frame)->topV;
    AsW3dAniSpr(self)->vertex1U = ((SpriteFrame*)frame)->rightU;
    AsW3dAniSpr(self)->vertex1V = ((SpriteFrame*)frame)->topV;
    AsW3dAniSpr(self)->vertex2U = ((SpriteFrame*)frame)->rightU;
    AsW3dAniSpr(self)->vertex2V = ((SpriteFrame*)frame)->bottomV;
    AsW3dAniSpr(self)->vertex3U = ((SpriteFrame*)frame)->leftU;
    AsW3dAniSpr(self)->vertex3V = ((SpriteFrame*)frame)->bottomV;

    // Phase 3: rotate two diagonal local corners (left,bottom) and (right,top).
    // rightOffset = cos(angle)*localX - sin(angle)*localY;
    // upOffset = sin(angle)*localX + cos(angle)*localY.
    // WView viewMatrix[0..2]/[3..5] supply right/up basis vectors. Add their
    // scaled offsets to the WORLD position, transform by inputTransform into
    // the view/projection-input space, then WView_ProjectViewPoint writes the
    // projected position, reciprocal W and clip flags. No model matrix is used.
    // cosineOrRightOffset is reused as the second corner's rightOffset after rotation.
    float cosineOrRightOffset = Math_CosApprox(rotationRadiansOrIndexedScale);
    float sinAngle = Math_SinApprox(rotationRadiansOrIndexedScale);

    float cosineProduct, localX, localYOrSineProduct, upOffsetOrLocalY, rightOffsetScalar;
    localYOrSineProduct = AsW3dAniSpr(self)->bottom;
    localX = AsW3dAniSpr(self)->left;
    cosineProduct = cosineOrRightOffset * localYOrSineProduct;
    localYOrSineProduct = sinAngle * localYOrSineProduct;
    upOffsetOrLocalY = sinAngle * localX + cosineProduct;
    rightOffsetScalar = cosineOrRightOffset * localX - localYOrSineProduct;

    float viewCorner[3];
    float worldCorner[3];
    float centerPlusRight[3];
    float rightOffset[3];
    float upOffset[3];
    __ml__FRC7WVectorf(upOffset, (void*)&AsWView(view)->viewMatrix[3], upOffsetOrLocalY);
    __ml__FRC7WVectorf(rightOffset, (void*)AsWView(view)->viewMatrix, rightOffsetScalar);
    __pl__FRC7WVectorRC7WVector(centerPlusRight, (char*)&AsW3dAniSpr(self)->position, rightOffset);
    __pl__FRC7WVectorRC7WVector(worldCorner, centerPlusRight, upOffset);
    Matrix_TransformPoint(viewCorner, worldCorner, (void*)AsWView(view)->inputTransform);
    WView_ProjectViewPoint(view, AsW3dAniSpr(self)->vertex0, viewCorner, 0);

    upOffsetOrLocalY = AsW3dAniSpr(self)->top;
    localX = AsW3dAniSpr(self)->right;
    cosineProduct = cosineOrRightOffset * upOffsetOrLocalY;
    localYOrSineProduct = sinAngle * upOffsetOrLocalY;
    upOffsetOrLocalY = sinAngle * localX + cosineProduct;
    cosineOrRightOffset = cosineOrRightOffset * localX - localYOrSineProduct;

    float oppositeViewCorner[3];
    float oppositeWorldCorner[3];
    float oppositeCenterPlusRight[3];
    float oppositeRightOffset[3];
    float oppositeUpOffset[3];
    __ml__FRC7WVectorf(oppositeUpOffset, (void*)&AsWView(view)->viewMatrix[3], upOffsetOrLocalY);
    __ml__FRC7WVectorf(oppositeRightOffset, (void*)AsWView(view)->viewMatrix, cosineOrRightOffset);
    __pl__FRC7WVectorRC7WVector(oppositeCenterPlusRight, (char*)&AsW3dAniSpr(self)->position, oppositeRightOffset);
    __pl__FRC7WVectorRC7WVector(oppositeWorldCorner, oppositeCenterPlusRight, oppositeUpOffset);
    Matrix_TransformPoint(oppositeViewCorner, oppositeWorldCorner, (void*)AsWView(view)->inputTransform);
    WView_ProjectViewPoint(view, AsW3dAniSpr(self)->vertex2, oppositeViewCorner, 0);

    ReconstructBillboardProjectedCorners(self);
    // Phase 5: untextured flags force white corners; textured submission merges
    // only the low 11 bitmap-id bits with caller flags.
    if (flags & kUntexturedRenderFlags) {
        void** vertices = (void**)((char*)&AsW3dAniSpr(self)->vertex0);
        (*(int*)&AsW3dSprVertex(vertices[0])->color) = -1;
        (*(int*)&AsW3dSprVertex(vertices[1])->color) = -1;
        (*(int*)&AsW3dSprVertex(vertices[2])->color) = -1;
        (*(int*)&AsW3dSprVertex(vertices[3])->color) = -1;
        ((SpriteCamera*)view)->submit(vertices, kSubmitSpriteQuad, flags, 0);
    } else {
        int bitmapFlags = ((SpriteFrame*)frame)->bitmapId;
        bitmapFlags = bitmapFlags & kBitmapIdMask;
        ((SpriteCamera*)view)->submit((char*)&AsW3dAniSpr(self)->vertex0, kSubmitSpriteQuad, bitmapFlags | flags, 0);
    }
}
#pragma opt_common_subs reset
#pragma opt_lifetimes reset

// Select a frame, transform all four corners through a model matrix and submit the projected quad.
// view borrows WView, offsetTransform borrows the local-to-offset transform. Unlike
// DrawFrame's two-corner rectangle, this path projects all four transformed corners.
// The same scene-phase gate instead forwards offsetTransform's twelve float
// words unchanged to the indexed packet consumer; it returns before projection.
// Preserve that fallback, matrix order and corner-to-UV association when modding.
// Frame pointers/bitmap ids obey spriteNext's lifetime contract; serialize this
// path with all other traversals, grid appends, UV edits and sprite destruction.
void W3dAniSpr_DrawFrameWithMatrix(void* self, void* view, void* offsetTransform, int renderFlags, int frameIndex) {
    // Scene render policy (course_scene_render_and_geometry.cpp): the deferred/effect phase sets effectRenderPassActive.
    // Textured sprites then use the indexed draw packet (UVs + placement, projected by its consumer).
    // Ordinary scene drawing projects vertices here; untextured flags always require that path.
    // With no scene manager, use the same indexed fallback for textured sprites.
    if (g_sceneManager == 0 || ((CSceneManager*)g_sceneManager)->effectRenderPassActive != 0) {
        if (!(renderFlags & kUntexturedRenderFlags)) {
            W3dAniSpr_DrawIndexedFrameWithMatrix(self, view, renderFlags, offsetTransform, frameIndex);
            return;
        }
    }

    int frameAddress = 0;
    {
        WInstanceLock g(AsW3dAniSpr(self)->frameMutex);
        AsW3dAniSpr(self)->frameCursor = AsW3dAniSpr(self)->frameHead;
        OSLockMutex(AsW3dAniSpr(self)->frameMutex);
        void* node = (*(void**)&AsW3dAniSpr(self)->frameCursor);
        if (node != 0) {
            unsigned int nextNode = (unsigned int)((SpriteHashNode*)node)->forward;
            unsigned int firstNode = (*(unsigned int*)&AsW3dAniSpr(self)->frameHead);
            frameAddress = *(int*)node;
            if (nextNode == firstNode) nextNode = 0;
            AsW3dAniSpr(self)->frameCursor = (int)nextNode;
            OSUnlockMutex(AsW3dAniSpr(self)->frameMutex);
        } else {
            OSUnlockMutex(AsW3dAniSpr(self)->frameMutex);
            frameAddress = 0;
        }
    }

    void* frameMutex = AsW3dAniSpr(self)->frameMutex;
    int skippedFrames = 0;
    while (skippedFrames < frameIndex && frameAddress != 0) {
        OSLockMutex(frameMutex);
        void* node = (*(void**)&AsW3dAniSpr(self)->frameCursor);
        if (node != 0) {
            unsigned int nextNode = (unsigned int)((SpriteHashNode*)node)->forward;
            unsigned int firstNode = (*(unsigned int*)&AsW3dAniSpr(self)->frameHead);
            frameAddress = *(int*)node;
            if (nextNode == firstNode) nextNode = 0;
            AsW3dAniSpr(self)->frameCursor = (int)nextNode;
            OSUnlockMutex(frameMutex);
        } else {
            OSUnlockMutex(frameMutex);
            frameAddress = 0;
        }
        skippedFrames++;
    }

    if (frameAddress != 0) {
        float uv = ((SpriteFrame*)frameAddress)->leftU;
        float zero = kFloat_0_AniSprite;
        AsW3dAniSpr(self)->vertex0U = uv;
        void* cornerPointerCursor = self;
        Pair* cornerSelectors = s_animatedSpriteCornerSelectors;
        int cornerIndex = 0;
        uv = ((SpriteFrame*)frameAddress)->topV;
        AsW3dAniSpr(self)->vertex0V = uv;
        uv = ((SpriteFrame*)frameAddress)->rightU;
        AsW3dAniSpr(self)->vertex1U = uv;
        uv = ((SpriteFrame*)frameAddress)->topV;
        AsW3dAniSpr(self)->vertex1V = uv;
        uv = ((SpriteFrame*)frameAddress)->rightU;
        AsW3dAniSpr(self)->vertex2U = uv;
        uv = ((SpriteFrame*)frameAddress)->bottomV;
        AsW3dAniSpr(self)->vertex2V = uv;
        uv = ((SpriteFrame*)frameAddress)->leftU;
        AsW3dAniSpr(self)->vertex3U = uv;
        uv = ((SpriteFrame*)frameAddress)->bottomV;
        AsW3dAniSpr(self)->vertex3V = uv;

        // Local corner -> model-transformed offset -> add sprite WORLD position
        // -> WView inputTransform -> projected vertex. cornerSelectors chooses
        // left/right and top/bottom; byte cursor advances through vertex pointers.
        WVector transformedOffset;
        float localCorner[3];

        float viewCorner[3];

        for (cornerIndex = 0; cornerIndex < kSpriteCorners; cornerIndex++) {
            float cornerY;
            if (cornerSelectors->b == 1) cornerY = AsW3dAniSpr(self)->top;
            else cornerY = AsW3dAniSpr(self)->bottom;
            float cornerX;
            if (cornerSelectors->a == 1) cornerX = AsW3dAniSpr(self)->left;
            else cornerX = AsW3dAniSpr(self)->right;
            localCorner[0] = cornerX;
            localCorner[1] = cornerY;
            localCorner[2] = zero;
            Matrix_TransformPoint(&transformedOffset, localCorner, offsetTransform);
            WVector worldCorner = AsW3dAniSpr(self)->position + transformedOffset;
            Matrix_TransformPoint(viewCorner, &worldCorner, (void*)AsWView(view)->inputTransform);
            int projectedVertex = (int)AsW3dAniSpr(cornerPointerCursor)->vertex0;
            WView_ProjectViewPoint(view, (void*)projectedVertex, viewCorner, 0);
            cornerPointerCursor = (char*)cornerPointerCursor + 4;
            cornerSelectors = cornerSelectors + 1;
        }

        if (renderFlags & kUntexturedRenderFlags) {
            void* firstVertex = AsW3dAniSpr(self)->vertex0;
            void* projectedVertex = (char*)&AsW3dAniSpr(self)->vertex0;
            (*(int*)&AsW3dSprVertex(firstVertex)->color) = -1;
            void* vertex = AsW3dAniSpr(self)->vertex1;
            (*(int*)&AsW3dSprVertex(vertex)->color) = -1;
            vertex = AsW3dAniSpr(self)->vertex2;
            (*(int*)&AsW3dSprVertex(vertex)->color) = -1;
            vertex = AsW3dAniSpr(self)->vertex3;
            (*(int*)&AsW3dSprVertex(vertex)->color) = -1;
            ((SpriteCamera*)view)->submit(((char*)&AsW3dAniSpr(self)->vertex0), kSubmitSpriteQuad, renderFlags, 0);
        } else {
            int bitmapId = ((SpriteFrame*)frameAddress)->bitmapId;
            bitmapId = (bitmapId & kBitmapIdMask);
            int submitFlags = bitmapId | renderFlags;
            ((SpriteCamera*)view)->submit((char*)&AsW3dAniSpr(self)->vertex0, kSubmitSpriteQuad, submitFlags, 0);
        }
    }
}

#pragma opt_strength_reduction reset
// Set local quad bounds from its dimensions and pivot position.
void W3dAniSpr_SetSizeAndPivot(void* self, float width, float height, float pivotX, float pivotY) {
    AsW3dAniSpr(self)->left = -pivotX;
    AsW3dAniSpr(self)->right = width - pivotX;
    AsW3dAniSpr(self)->top = -pivotY;
    AsW3dAniSpr(self)->bottom = height - pivotY;
}

// Apply the same packed color to all four corner vertices.
void W3dAniSpr_SetColor(void* self, int color) {
    (*(int*)&AsW3dSprVertex(AsW3dAniSpr(self)->vertex0)->color) = color;
    (*(int*)&AsW3dSprVertex(AsW3dAniSpr(self)->vertex1)->color) = color;
    (*(int*)&AsW3dSprVertex(AsW3dAniSpr(self)->vertex2)->color) = color;
    void* lastVertex = AsW3dAniSpr(self)->vertex3;
    (*(int*)&AsW3dSprVertex(lastVertex)->color) = color;
}

// Insert a node before the list head while holding the list mutex.
static inline SpriteHashNode* listInsert(SpriteHashList* L, SpriteHashNode* head, SpriteHashNode* node) {
    void* m = &L->mutex;
    OSLockMutex(m);
    if (head == 0) {
        node->back = node;
        node->forward = node;
        OSUnlockMutex(m);
        return node;
    }
    node->forward = head;
    node->back = head->back;
    head->back = node;
    node->back->forward = node;
    OSUnlockMutex(m);
    return head;
}

// Refill the node pool if needed and detach the first free node.
// Require blockSize >= 2: slot 0 is a permanent block header, so a batch of N
// provides N-1 payload nodes (default 16 -> 15). The allocation is exactly
// blockSize*24 bytes. Never hand out/free slot 0 as a payload node.
// Require a RECURSIVE OS mutex: listPush -> listTake -> listInsert nests locks
// on the same mutex, and listTake's final ring unlink locks it once more.
static inline SpriteHashNode* listTake(SpriteHashList* L) {
    SpriteHashNode* blk;
    int k;
    WInstanceLock lockB(&L->mutex);
    if (L->free == 0) {
        blk = (SpriteHashNode*)appMallocUni__FUl(L->blockSize * 0x18);
        L->blocks = listInsert(L, L->blocks, blk);
        k = 1;
        if (L->blockSize > 1) {
            SpriteHashNode* p = blk + 1;
            for (; k < L->blockSize; ++k) {
                L->free = listInsert(L, L->free, p);
                p = p + 1;
            }
        }
    }
    SpriteHashNode* node = L->free;
    void* m = &L->mutex;
    OSLockMutex(m);
    node->forward->back = node->back;
    node->back->forward = node->forward;
    SpriteHashNode* next = node->forward;
    if (next == node) next = 0;
    OSUnlockMutex(m);
    L->free = next;
    return node;
}

// Allocate a list node and append an unkeyed payload.
static inline void listPush(SpriteHashList* L, void* data) {
    SpriteHashNode* node;
    WInstanceLock lockA(&L->mutex);
    node = listTake(L);
    node->data = (unsigned int)data;
    node->ownsKey = 0;
    node->key = 0;
    L->head = listInsert(L, L->head, node);
}

// Load and retain a bitmap, then append one UV rectangle per grid cell in row order.
// cellWidth/Height are positive pixel dimensions; bitmap dimensions must be
// nonzero. Fractional cells are truncated; remainder pixels are not separate
// frames. Sprite retains the loaded bitmap id until destruction. Return is 0
// unconditionally, not a success boolean; allocation/load failure is not checked here.
// bitmapName is a 32-bit Wii address of borrowed NUL-terminated narrow filename
// bytes, despite the int ABI; it is not a numeric bitmap ID or UTF-16 string.
// The caller W3dAniSpr_Create (resource_manager_cache_and_loading.cpp) forwards that filename and
// loadFlags. ResourceManager_LoadBitmap copies/normalizes the path synchronously:
// requires a suffix; .bres is retained, other suffixes become .gsr; entry?archive
// syntax selects an archive entry. Its fixed buffers require path<0x400 bytes,
// basename<0x40 bytes and archive entry key<0x104 bytes including NUL.
// loadFlags is the manager's u32 option word, NOT a Boolean or async selector;
// async is separately passed as zero here. Known manager bits (resource_manager_cache_and_loading.cpp):
// 80000000=24-bit memory accounting, 40000000=extra half-size accounting,
// 20000000=name '!' policy, 00000800=name '{' policy; any of 10003000 suppresses
// manager initialization. '{' also clears 40000000; '!' sets 20000000.
// Factory map (phase1_class_tables.cpp WGeneric3D/WGeneric3DR table +28/+2C):
// WGeneric3D_CreateTexture / UpdateTextureSlot (wgeneric3d_render_queue_and_soft_keyboard.cpp).
// Complete consumed mask for these backends plus manager: 0xF0003800. Zero is
// the ordinary loaded-grid request used by the emitter/snow callers. Bit effects:
// 20000000 selects reserved slots 1..127 (else general slots 128..2047);
// 00001000 reserves ONLY: no resource binding, then manager skips initialize;
// this is not a ready texture for a new sprite draw. With descriptor input
// (BitmapLayout.resource==0), 80000000 selects creation arg 24 instead of 16,
// 40000000 selects type 3 instead of 1, 2000 selects filter1 instead of 0, and
// 800 selects mode1 instead of 0 when descriptor format<32 (else mode2).
// With a GSR resource these creation selectors are bypassed: slot40 binds it via
// WGeneric3DR_LoadTextureSlot (wgeneric3dr_immediate_renderer.cpp). In recovered Wii tables descriptor
// storage/data hooks are NO-OP, so arbitrary raw-pixel descriptors are not a
// supported replacement upload path. 10000000/2000 also suppress manager's
// initialize call; they are not independent image-quality controls.
// Bits outside F0003800 are unconsumed by the recovered manager/factory/hooks;
// keep them zero for new requests. A new backend must define its own domain.
// Require a free slot in the chosen pool: exhaustion has no safe failure result
// and can reuse an occupied slot. Cache hits retain existing handles and ignore
// the new flags. A flag-only asset mod therefore also needs a fresh cache entry.
// Filename storage is borrowed only for the load; the sprite records the retained
// handle and releases it via ResourceManager_ReleaseId in its destructor.
int W3dAniSpr_LoadSpritesInOneTexture(void* self, int bitmapName, int loadFlags, float cellWidth, float cellHeight) {
    int handle = ResourceManager_LoadBitmap(AsW3dAniSpr(self)->resourceManager, bitmapName, loadFlags, 0, 0);
    float width = (float)ResourceManager_GetBitmapWidth(AsW3dAniSpr(self)->resourceManager, handle);
    float height = (float)ResourceManager_GetBitmapHeight(AsW3dAniSpr(self)->resourceManager, handle);
    float dx = kFloat_1_AniSprite / (width / cellWidth);
    int columns = (int)(width / cellWidth);
    float dy = kFloat_1_AniSprite / (height / cellHeight);
    int rows = (int)(height / cellHeight);
    float x = kFloat_0_AniSprite;
    float y = x;
    SpriteHashList* frames = (SpriteHashList*)((char*)&AsW3dAniSpr(self)->frameHead);
    SpriteHashList* bitmaps = (SpriteHashList*)((char*)&AsW3dAniSpr(self)->bitmapHead);
    for (int row = 0; row < rows; ++row) {
        int column = 0;
        for (; column < columns; ++column) {
            SpriteFrame* item = (SpriteFrame*)__nw__FUl(sizeof(SpriteFrame));
            item->bitmapId = handle;
            item->topV = y;
            item->bottomV = y + dy;
            item->leftU = x;
            item->rightU = x + dx;
            listPush(frames, item);
            AsW3dAniSpr(self)->frameCount = AsW3dAniSpr(self)->frameCount + 1;
            x += dx;
        }
        y += dy;
        x = kFloat_0_AniSprite;
    }
    listPush(bitmaps, (void*)handle);
    AsW3dAniSpr(self)->bitmapCount = AsW3dAniSpr(self)->bitmapCount + 1;
    return 0;
}

// Append grid-cell UV rectangles for an existing bitmap without taking bitmap ownership.
// Same positive pixel-size preconditions as LoadBitmapGrid; caller must keep
// bitmapId alive. Return 1 unconditionally, with frames owned by this sprite.
int W3dAniSpr_LoadTexture(void* self, int bitmapId, float cellWidth, float cellHeight) {
    float width;
    float height;
    float dx;
    float dy;
    float x;
    float y;
    int columns;
    int rows;
    int widthInt = ResourceManager_GetBitmapWidth(AsW3dAniSpr(self)->resourceManager, bitmapId);
    width = (float)widthInt;
    int heightInt = ResourceManager_GetBitmapHeight(AsW3dAniSpr(self)->resourceManager, bitmapId);
    height = (float)heightInt;
    columns = (int)(width / cellWidth);
    dx = kFloat_1_AniSprite / (width / cellWidth);
    rows = (int)(height / cellHeight);
    dy = kFloat_1_AniSprite / (height / cellHeight);
    x = kFloat_0_AniSprite;
    y = x;
    SpriteHashList* frames = (SpriteHashList*)((char*)&AsW3dAniSpr(self)->frameHead);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < columns; ++col) {
            SpriteFrame* data = (SpriteFrame*)__nw__FUl(sizeof(SpriteFrame));
            data->bitmapId = bitmapId;
            float xdx = x + dx;
            data->topV = y;
            data->bottomV = y + dy;
            data->leftU = x;
            data->rightU = xdx;
            listPush(frames, data);
            AsW3dAniSpr(self)->frameCount += 1;
            x += dx;
        }
        y += dy;
        x = kFloat_0_AniSprite;
    }
    return 1;
}

} // extern "C"

#undef OSInitMutex
#undef OSLockMutex
#undef OSUnlockMutex

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621FC", _ctors_u80216D5C_0, TaskRegistry_RegisterFactories);
#pragma pop
