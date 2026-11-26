// Game startup and shutdown, the main frame loop, and creating the title, shop, grid and letter objects.
// Game_MainLoop runs each frame's services (HOME menu, input, save data, fades) in order.
// Game_InitializeSingletons creates the shared controllers and reserves common rendering buffers.

#include <decomp/static_init.h>
// Game startup/shutdown and frame scheduling, including HOME, save and overlays.
// Modding lookup: Game_MainLoop's service policy below controls which subsystems
// run; Engine_PresentFrame (gx_renderer_state_and_resources.cpp) controls presentation waits; project m12
// leads to the simulation clock in main_frame_letter_and_mouse_cursor.cpp. These are separate controls.
// Render_DrawFilledRect consumes packed RGBA and the depths in the data section;
// that companion also defines the one-shot runtime-service gate (initially 1).
extern "C" char s_shutdownHomeMenuDtorRegistration[];

#include "game/singletons.h"
#include "game/spin.h"
#include "classes/MWStreamState.h"
#include "classes/CWind.h"
#include "classes/CPangyaEnemyData.h"
#include "classes/CProjectG.h"
#include "classes/CNowReloading.h"
#include "classes/CMouseCursor.h"
#include "classes/CB2dPlayerHandle.h"
#include "classes/CEffectFade.h"
#include "classes/CShop2Main.h"
#include "classes/CBpCtrl.h"
#include "classes/CCheckDriveStatus.h"
#include "classes/CBpPlayer.h"
#include "classes/CInfoBase.h"
#include "classes/CTourHintWindow.h"
#include "classes/CTourVSWindow.h"
#include "classes/CLetter.h"
#include "classes/CPangya2TitleMain.h"
#include "classes/CShop2Task.h"
#include "classes/CTitleTask.h"
// Lazy singleton destructor records occupy this fixed registration block.
enum GameRegistrationOffset {
    kHomeMenuRegistration = 0xC,
    kBpControllerRegistration = 0x24,
    kMessageDrawRegistration = 0x30,
    kBpHandleRegistration = 0x3C,
    kInputRegistration = 0x48,
    kMovieRegistration = 0x54,
    kFadeRegistration = 0x60,
    kEffectFadeRegistration = 0x6C,
    kEnemyDataRegistration = 0x78,
    kReloadingRegistration = 0x84,
    kNumberDrawRegistration = 0x90,
    kBpPlayerRegistration = 0x9C,
    kTourHintRegistration = 0xA8,
    kItemRequestRegistration = 0xB4,
    kTourVsRegistration = 0xC0
};
enum ImmediateDrawValue {
    kPositionDiffuseFormat = 0xF005,
    kTriangleStrip = 0x98
};
enum StreamStateMask {
    kStreamBad = 1,
    kStreamFailOrBad = 5,
    kUnitBuffered = 0x2000,
    kIntegerBaseMask = 0x4A,
    kHexBase = 8,
    kOctalBase = 0x40
};
enum StreamWarmupConstants {
    kVirtualBaseOffsetWord = 0x3C,
    kStringBufferReadArea = 8,
    kStringBufferWriteArea = 0x10,
    kWarmupStringCapacity = 0x40,
    kPuppetTransformCapacity = 0x140
};
struct OutputStreamView {
    unsigned char prefix[0x24];
    unsigned int streamBuffer;
    unsigned int precision, width;
    unsigned short flags;
    unsigned char stateBits, exceptionMask;
    unsigned int reserved34;
    signed char fillCharacter;
};
struct SaveCounterView {
    unsigned char prefix[0x20];
    int incrementedCounter;
};
struct AlphabetPool {
    char alphabet[27], digits[11], passwordMarker[9];
};
enum {
    kSaveCounterMaximum = 0xA4CB62
};

extern "C" {
// First word of an eight-byte pool. Nonzero requests startup servicing here;
// Game_MainLoop then clears it and skips subsystem servicing for one iteration.
extern int s_skipNextRuntimeSubsystemServiceStorage;
extern signed char s_pangyaEnemyDataInitialized, s_effectFadeInitialized, s_nowReloadingInitialized;
extern void* g_mouseCursor;
extern void* s_nowReloading;
extern unsigned int s_vcgAllocatorCallbackWord0, s_vcgAllocatorCallbackWord1;
extern char s_systemResetDtorRegistration_GameMain[], s_pangyaEnemyData[], s_effectFade[];
extern void FileIO_FlushAllDevices(void);
extern void gtl_Audio_Update(void);
extern void gtl_InputEx_Update(void);
extern void gtl_Media_Update(void);
extern void __ct__8CProjectFv(void*, void*);
extern void Game_DrawReloadingFrame(void);
extern void Game_InitializeSingletons(void);
extern void __dt__16CPangyaEnemyDataFv(void);
extern void* ModelRenderParams_GetShared();
extern void CharacterEnvironment_CopyFirstLightDescription(void*);
extern void Vcg_SetAllocatorCallbacks(void*);
extern void __ct__11CEffectFadeFv(void*);
extern void __dt__11CEffectFadeFv(void);
extern void __dt__13CNowReloadingFv(void);
extern void GameHook_OnInitializationComplete(void);
extern void Engine_SetTextureStageResource(int, int);
extern void Engine_SetTexturePalette(int, int);
extern void Engine_SetFlexibleVertexFormat(int);
extern void Engine_SetType7Resource(int);
extern void* GX_GetPrimitiveCount(int, int);
extern void DrawPrimitiveUP(int, int, void*, int);
extern void __dl__FPv(void*);
extern signed char s_bpSharedStartupGuard;
extern signed char s_bpSecondaryStartupGuard;
extern float kFloat_6_2831855;
extern float s_gameLoopFullTurn;
extern void* __nw__FUl(unsigned int);
extern void __dl__FPv(void*);
extern void __ct__5CGridFv(void*);
extern void GameHook_OnShutdown(void);
extern void VCG_ClearAllocatorCallbacks(void);
extern void HomeButtonMenu_InitializeState(void*);
extern void HomeButtonResources_dtor(void*);
extern void HomeButton_ShutdownResources(void*);
extern void OStreamSentry_Ctor(void*, void*);
extern void MWStream_CopyLocale(void*, void*);
extern void MWLocale_InstallFacet(void*, void*, unsigned int);
extern void MWSharedOwner_ReleaseReference(void*);
extern void MWStream_ThrowFailure(void*, const char*);
extern void OStream_Flush(void*);
extern void LocaleRef_dtor(void*);
extern void OStreamSentry_dtor(void*);
extern void __unexpected(void*);
extern void __throw(void*, void*, void*);
extern unsigned int s_streamLocaleFacetId, s_localeFacetIdCounter;
extern char stdNumPutCharVtable[], iosClearFailureMessage[];
extern void __register_global_object(void*, void*, void*);
// Partial CProject virtual contract, in declaration/ABI order:
// The CProjectG table in phase1_class_tables.cpp identifies these slots:
// m0(1) -> CProjectG::~CProjectG (delete project);
// m12(0) -> CProjectG_UpdateVirtualHooks (refresh frame clock, dispatch update hooks);
// m13() -> CProjectG_InitializeGameObjects (view/input/UI + title task);
// m14(0,0,0,0) -> CProjectG::DispatchRegisteredObjectCallback (fan out
// three null arguments to registered objects; first argument is unused).
// Other slots include secondary-interface metadata, not callable methods.
// This partial view reserves their positions only. Do not call m6..m11.
// Keep m12 then m14 after clear/fade and before UI; zeros are not a timestep.
// Shared method names remain unchanged pending naming review.
struct Obj805DC784 {
    virtual void m0(int);
    virtual void m1(int);
    virtual void m2(int);
    virtual void m3(int);
    virtual void m4(int);
    virtual void m5(int);
    virtual void m6(int);
    virtual void m7(int);
    virtual void m8(int);
    virtual void m9(int);
    virtual void m10(int);
    virtual void m11(int);
    virtual void m12(int);
    virtual void m13();
    virtual void m14(int, int, int, int);
    virtual void m15(int);
    virtual void m16(int);
};
extern Obj805DC784* g_project;
extern signed char s_systemResetStateInitialized;
extern void* g_taskManager;
extern signed char s_inputManagerInitialized;
extern signed char s_saveDataInitialized;
extern signed char s_fadeOverlayInitialized;
extern signed char s_bpCtrlInitialized;
extern signed char s_pointerPairListInitialized;
extern signed char s_pointerVectorInitialized;
extern float kFloat_1_GameClearDepth, kFloat_0_ReloadingOverlayDepth;
extern void* g_helpMsg;
extern unsigned int gameLoopOverlayRectangleWords[4];
extern unsigned int gameLoopOverlayTextParameters[3];
extern char alphabetAndStringErrorStrings[];
extern char s_saveData[];
extern char s_fadeOverlay[];
extern char s_homeButtonResourceState[];
extern char s_gameInputManager[];
extern char s_bpCtrl[];
extern char s_messageDrawPairList[];
extern char s_fontNumDrawRegistry[];
extern char s_effectFade[];
extern char s_systemResetStorage[];
extern void GameHook_OnMainLoopEnter(void);
extern void GameHook_OnFrameBegin(void);
extern void SystemResetState_Ctor(void*);
extern void SystemResetState_Dtor(void);
extern int SystemReset_ProcessPendingReset(void*, int);
extern void HomeButtonMenu_InitializeState(void*);
extern void HomeButtonResources_dtor(void*);
extern int HomeButtonMenu_IsActive(void*);
extern void InputManager_Init(void*);
extern void InputManager_Dtor(void);
extern void InputManager_Update(void*);
extern void SaveData_Ctor(void*);
extern int SaveData_HasSlotSelectionChanged(void*);
extern void* SaveData_GetCurrentRecord(void*);
extern void SaveRecord_CopyCtor(void*, void*);
extern void SaveData_SetCurrentRecord(void*, void*);
extern void LoadingScreen_PollResourceReadiness(void);
extern void Engine_ClearFramebuffer(int, int, int, float, void*, int);
extern void FadeOverlay_Init(void*);
extern void ResourceState_Dtor(void);
extern void FadeState_DrawAlternatePhase(void*);
extern int ActorLoading_GetSecondaryStatus(void*);
extern void CBpCtrl_dtor(void);
extern void NoticeQueue_UpdateAndDraw(void*);
extern void PairVector_InitReserved(void*);
extern void MessageDrawPairList_Dtor(void);
extern void MsgDrawList_UpdateReveal(void*);
extern void MsgDrawList_DrawGlyphs(void*);
extern void PointerVector_InitReserved(void*);
extern void PointerVector_DestroyStorage(void);
extern void UITextTable_DrawList(void*);
extern void __ct__11CEffectFadeFv(void*);
extern void __dt__11CEffectFadeFv(void);
extern void ImmediateOverlay_DrawUntexturedVertices(void*);
extern void FadeState_Update(void*);
extern void FadeState_DrawPrimaryPhase(void*);
extern void InputManager_DrawCursors(void*);
extern void HomeButtonMenu_ProcessState(void*);
extern void Game_PreOverlayFrameHook(void);
extern void __dt__13CNowReloadingFv(void);
extern void B2dPlayer_UpdateAndDraw(void*);
extern void GameHook_OnFrameRenderComplete(void);
extern void gtl_BitmapFont_FormatTextVertices(void*, const char*);
extern void gtl_BitmapFont_Draw(void);
extern void Engine_PresentFrame(int, int, int, int);
extern signed char s_homeMenuInitialized;
extern char s_homeButtonResourceState[], s_shutdownHomeMenuDtorRegistration[];
// Initialize the game-loop angle cache and mark both BP startup guards.
void Game_InitializeBpStartupGuardsAndFullTurn(void) {
    s_gameLoopFullTurn = kFloat_6_2831855;
    if (!s_bpSharedStartupGuard)
        s_bpSharedStartupGuard = 1;
    if (s_bpSecondaryStartupGuard)
        return;
    s_bpSecondaryStartupGuard = 1;
}

struct CTitleTaskStorage {
    char data[0x3a8];
    // Initialize a title task in its ABI-sized allocation.
    CTitleTaskStorage() {
        ((CTitleTask*)this)->__ct();
    }
};
struct CPangya2TitleMainStorage {
    char data[0x34];
    // Initialize the title main actor in its ABI-sized allocation.
    CPangya2TitleMainStorage() {
        ((CPangya2TitleMain*)this)->__ct();
    }
};

// Allocate and initialize the title task using its original object size.
void* CTitleTask_Create(void) {
    return new CTitleTaskStorage;
}
// Allocate and initialize the title's main actor.
void* CPangya2TitleMain_Create(void) {
    return new CPangya2TitleMainStorage;
}

struct CShop2TaskStorage {
    char data[0x3a8];
    // Initialize a shop task in its ABI-sized allocation.
    CShop2TaskStorage() {
        ((CShop2Task*)this)->__ct();
    }
};
struct CShop2MainStorage {
    char data[0x28];
    // Initialize the shop main actor in its ABI-sized allocation.
    CShop2MainStorage() {
        ((CShop2Main*)this)->Init();
    }
};
struct CGridStorage {
    char data[0x18c0];
    // Initialize a grid in its ABI-sized allocation.
    CGridStorage() {
        __ct__5CGridFv(this);
    }
};

// Allocate and initialize the shop task using its original object size.
void* CShop2Task_Create(void) {
    return new CShop2TaskStorage;
}
// Allocate and initialize the shop's main actor.
void* CShop2Main_Create(void) {
    return new CShop2MainStorage;
}
// Allocate and initialize a grid actor.
void* CGrid_Create(void) {
    return new CGridStorage;
}

struct CLetterStorage {
    char data[0xac];
    // Initialize the letter overlay actor in its ABI-sized allocation.
    CLetterStorage() {
        ((CLetter*)this)->__ct();
    }
};

// Allocate and initialize a spin indicator.
void* CSpin_Create(void) {
    return new CSpin;
}
// Allocate and initialize the letter/message actor.
void* CLetter_Create(void) {
    return new CLetterStorage;
}

struct CProjectStorage {
    char data[0xC0];
    // Construct the project with the borrowed startup context.
    CProjectStorage(void* startupContext) {
        __ct__8CProjectFv(this, startupContext);
    }
};
// Create the project and shared gameplay resources, drawing loading frames between setup stages.
int Game_InitializeProjectAndSharedResources(void* startupContext) {
    // Phase 1: service pending subsystems, create the project, and initialize devices.
    // A device failure returns zero after project creation; no rollback occurs here.
    void* registrationBase = (void*)s_systemResetDtorRegistration_GameMain;
    // Startup services the subsystems when this flag is set, without consuming it.
    if (s_skipNextRuntimeSubsystemServiceStorage) {
        FileIO_FlushAllDevices();
        gtl_Audio_Update();
        gtl_InputEx_Update();
        gtl_Media_Update();
    }
    void* projectStorage = new CProjectStorage(startupContext);
    if (!(void*)((CProjectG*)g_project)->InitializeLegacyDevices()) {
        return 0;
    }
    Game_DrawReloadingFrame();
    // Construct the view, input/UI objects, and initial title task.
    g_project->m13();
    // Phase 2: warm shared singletons and request enemy/model rendering resources.
    Game_DrawReloadingFrame();
    Game_InitializeSingletons();
    if (!Sing_805DC678()) {
        if (!s_pangyaEnemyDataInitialized) {
            ((CPangyaEnemyData*)s_pangyaEnemyData)->Init();
            __register_global_object(s_pangyaEnemyData, (void*)__dt__16CPangyaEnemyDataFv,
                                     (void*)(((char*)registrationBase + kEnemyDataRegistration)));
            s_pangyaEnemyDataInitialized = 1;
        }
        Sing_805DC678() = s_pangyaEnemyData;
    }
    ((CPangyaEnemyData*)Sing_805DC678())->LoadSharedFile();
    Game_DrawReloadingFrame();
    void* sharedRenderParams = (void*)ModelRenderParams_GetShared();
    CharacterEnvironment_CopyFirstLightDescription(sharedRenderParams);
    Game_DrawReloadingFrame();
    // Preserve the two allocator callback words in their original order.
    unsigned int allocatorCallbacks[2] = {s_vcgAllocatorCallbackWord0, s_vcgAllocatorCallbackWord1};
    Vcg_SetAllocatorCallbacks(allocatorCallbacks);
    if (!Sing_805DC660()) {
        if (!s_effectFadeInitialized) {
            ((CEffectFade*)s_effectFade)->__ct();
            __register_global_object(s_effectFade, (void*)__dt__11CEffectFadeFv,
                                     (void*)(((char*)registrationBase + kEffectFadeRegistration)));
            s_effectFadeInitialized = 1;
        }
        Sing_805DC660() = s_effectFade;
    }
    // Phase 3: enable pointer input, request the loading script, notify completion.
    ((CEffectFade*)Sing_805DC660())->ClearColor();
    CMouseCursor::SetPointerInputEnabled(g_mouseCursor, 1);
    if (!Sing_805DC610()) {
        if (!s_nowReloadingInitialized) {
            ((CNowReloading*)(&s_nowReloading))->Init();
            __register_global_object(&s_nowReloading, (void*)__dt__13CNowReloadingFv,
                                     (void*)(((char*)registrationBase + kReloadingRegistration)));
            s_nowReloadingInitialized = 1;
        }
        Sing_805DC610() = &s_nowReloading;
    }
    CB2dPlayerHandle::RequestPendingScript(Sing_805DC610());
    GameHook_OnInitializationComplete();
    return 1;
}

struct Vtx {
    float x, y, z;
    unsigned char rgba[4];
};
struct Rect {
    int x1, y1, x2, y2;
};
// Draw a colored rectangle with input X/Y copied directly to position vertices
// (x1,y1),(x2,y1),(x1,y2),(x2,y2). color is 0xRRGGBBAA; depth is copied to Z.
// This helper supplies no matrix, viewport or scissor: the caller must establish
// screen/layout projection (and its Y direction), depth test/blend state and clip.
// The main-loop caller uses the overlay rectangle pool and normalized depth 0;
// framebuffer clear depth is separately 1. No endpoint sorting or clipping occurs.
// Null rect or either zero extent returns 0 without renderer changes; otherwise
// return 1 after synchronous submission of the stack vertices (stride 16 bytes).
// Unbind texture stage/palette 0 and type-7 resource, select position+diffuse FVF;
// those renderer changes persist after return. No caller state is restored.
int Render_DrawFilledRect(Rect* rect, unsigned int color, float depth) {
    if (!rect)
        return 0;
    if (rect->x1 == rect->x2 || rect->y1 == rect->y2)
        return 0;
    unsigned char blue, green, red, alpha;
    red = color >> 24;
    green = color >> 16;
    blue = color >> 8;
    alpha = color;
    Vtx vertices[4] = {};
    vertices[0].x = (float)rect->x1;
    vertices[0].y = (float)rect->y1;
    vertices[0].z = depth;
    vertices[1].x = (float)rect->x2;
    vertices[1].y = (float)rect->y1;
    vertices[1].z = depth;
    vertices[2].x = (float)rect->x1;
    vertices[2].y = (float)rect->y2;
    vertices[2].z = depth;
    vertices[3].x = (float)rect->x2;
    vertices[3].y = (float)rect->y2;
    vertices[3].z = depth;
    vertices[0].rgba[0] = red;
    vertices[0].rgba[1] = green;
    vertices[0].rgba[2] = blue;
    vertices[0].rgba[3] = alpha;
    vertices[3].rgba[0] = red;
    vertices[3].rgba[1] = green;
    vertices[3].rgba[2] = blue;
    vertices[3].rgba[3] = alpha;
    vertices[2].rgba[0] = red;
    vertices[2].rgba[1] = green;
    vertices[2].rgba[2] = blue;
    vertices[2].rgba[3] = alpha;
    vertices[1].rgba[3] = alpha;
    vertices[1].rgba[0] = red;
    vertices[1].rgba[1] = green;
    vertices[1].rgba[2] = blue;
    Engine_SetTextureStageResource(0, 0);
    Engine_SetTexturePalette(0, 0);
    Engine_SetFlexibleVertexFormat(kPositionDiffuseFormat);
    Engine_SetType7Resource(0);
    void* primitiveCount = GX_GetPrimitiveCount(4, kTriangleStrip);
    DrawPrimitiveUP(kTriangleStrip, (int)primitiveCount, vertices, 0x10);
    return 1;
}

// Run shutdown hooks, destroy the project, and release HOME-menu resources.
void Game_Shutdown(void) {
    GameHook_OnShutdown();
    if (g_project) {
        if (g_project) {
            g_project->m0(1);
        }
    }
    VCG_ClearAllocatorCallbacks();
    if (!((volatile void*&)Sing_805DC820())) {
        if (!s_homeMenuInitialized) {
            HomeButtonMenu_InitializeState(s_homeButtonResourceState);
            __register_global_object(s_homeButtonResourceState, (void*)HomeButtonResources_dtor,
                                     s_shutdownHomeMenuDtorRegistration);
            s_homeMenuInitialized = 1;
        }
        ((volatile void*&)Sing_805DC820()) = s_homeButtonResourceState;
    }
    HomeButton_ShutdownResources((void*)((volatile void*&)Sing_805DC820()));
}

struct VecLike {
    void* ptr;
    unsigned int size;
};
struct Wrapper {
    void* object;
};
struct OutIter {
    unsigned int sbuf;
    // Borrow the stream buffer for locale-facet output.
    OutIter(unsigned int s) : sbuf(s) {}
    // Copy the borrowed output iterator without acquiring buffer ownership.
    OutIter(const OutIter& o) : sbuf(o.sbuf) {}
};
// Numeric facet ABI slots: m0 puts signed long; m1 puts unsigned long.
// Octal/hex dispatch reinterprets the integer's same 32 bits as unsigned;
// decimal/default dispatch retains its sign. The first argument is iterator
// result storage; the last duplicates the borrowed streambuf handle.
struct NumberPutFacet {
    virtual void _p0();
    virtual void _p1();
    virtual int m0(void*, void*, signed char, void*, unsigned int) = 0;
    virtual int m1(void*, void*, signed char, void*, unsigned int) = 0;
};
struct Sentry800B2380 {
    unsigned char b;
    unsigned char flag39;
    char pad[2];
    Wrapper* p;
    // Start formatted output and remember whether the stream is ready.
    Sentry800B2380(Wrapper* s) {
        OStreamSentry_Ctor(this, s);
    }
    // Flush unit-buffered successful output unless an exception is in flight.
    ~Sentry800B2380() {
        unsigned char* o = (unsigned char*)p->object;
        if (!(((OutputStreamView*)o)->stateBits & kStreamFailOrBad) && (AsMWStreamState(o)->flags & kUnitBuffered) &&
            !flag39) {
            OStream_Flush(p);
        }
    }
};
struct LocaleFacetRef {
    VecLike* p;
    void* ref;
    // Release the temporary locale reference; facets remain locale-owned.
    ~LocaleFacetRef() throw() {
        if (ref)
            MWSharedOwner_ReleaseReference(ref);
    }
};
// Allocate the process-wide numeric facet ID on first use.
static inline unsigned int GetNumericFacetId() {
    if (!s_streamLocaleFacetId) {
        ++s_localeFacetIdCounter;
        s_streamLocaleFacetId = s_localeFacetIdCounter;
    }
    return s_streamLocaleFacetId;
}
// Borrow the stream's numeric facet, installing a locale-owned default if absent.
static inline NumberPutFacet* FetchNumericFacet(Wrapper* self) {
    NumberPutFacet* item;
    LocaleFacetRef v;
    MWStream_CopyLocale(&v, self->object);
    volatile char dummy = 0;
    unsigned int i = GetNumericFacetId();
    if (!(i < v.p->size) || !(item = (NumberPutFacet*)((void**)v.p->ptr)[i])) {
        struct Alloc {
            void* vtable;
            int size;
        };
        Alloc* a = (Alloc*)__nw__FUl(8);
        if (a) {
            a->size = 0;
            a->vtable = stdNumPutCharVtable;
        }
        VecLike* vp = v.p;
        MWLocale_InstallFacet(vp, a, GetNumericFacetId());
        item = (NumberPutFacet*)((void**)v.p->ptr)[i];
    }
    return item;
}

#pragma use_lmw_stmw on
// Insert an integer through the stream locale facet, preserving base flags and failure/exception handling.
// integerValue carries a 32-bit integer in a pointer-typed ABI slot; it is never dereferenced.
void* OStream_InsertLong(Wrapper* self, void* integerValue) {
    // Phase 1: start formatted output. integerValue is passed as 32 raw bits
    // to num_put; octal/hex reinterpret signed input as unsigned, without dereference.
    // Sentry setup may flush a tied stream. Destruction handles unit buffering;
    // flag39 suppresses that flush while the outer catch rethrows.
    Sentry800B2380 sentry(self);
    // Failure path contract: caught facet exceptions set badbit; the failure
    // path below retains its original locals. If the facet throws before
    // assigning failed and the exception is suppressed, failed is uninitialized;
    // no replacement value is introduced here.
    // These volatile iterator/result spills preserve the original ABI/EH stores;
    // they are not independent formatting state or extra writes to the stream.
    volatile int unsignedPutResultSpill;
    volatile unsigned int unsignedBufferSpill;
    volatile int signedPutResultSpill;
    volatile unsigned int signedBufferSpill;
    unsigned int unsignedIteratorResult, signedIteratorResult;
    int unsignedPutResult, signedPutResult;
    try {
        if (sentry.b) {
            bool failed;
            unsigned char* streamState;
            try {
                // Phase 2: borrow the locale facet and select signed or unsigned output.
                NumberPutFacet* numberFacet = FetchNumericFacet(self);
                streamState = (unsigned char*)self->object;
                unsigned short flags = AsMWStreamState(streamState)->flags;
                unsigned short integerBase = flags & kIntegerBaseMask;
                if (integerBase == kOctalBase || integerBase == kHexBase) {
                    unsigned int streamBuffer = ((OutputStreamView*)streamState)->streamBuffer;
                    signed char fillCharacter = ((OutputStreamView*)streamState)->fillCharacter;
                    unsignedBufferSpill = streamBuffer;
                    unsignedIteratorResult = streamBuffer;
                    int putResult = numberFacet->m1(&unsignedIteratorResult, streamState, fillCharacter, integerValue,
                                                    streamBuffer);
                    failed = putResult == 0;
                    unsignedPutResult = putResult;
                    unsignedPutResultSpill = putResult;
                } else {
                    unsigned int streamBuffer = ((OutputStreamView*)streamState)->streamBuffer;
                    signed char fillCharacter = ((OutputStreamView*)streamState)->fillCharacter;
                    signedBufferSpill = streamBuffer;
                    signedIteratorResult = streamBuffer;
                    int putResult =
                        numberFacet->m0(&signedIteratorResult, streamState, fillCharacter, integerValue, streamBuffer);
                    failed = putResult == 0;
                    signedPutResult = putResult;
                    signedPutResultSpill = putResult;
                }
            } catch (...) {
                // A facet exception marks badbit; rethrow only when badbit is enabled.
                ((OutputStreamView*)self->object)->stateBits |= kStreamBad;
                if (((OutputStreamView*)self->object)->exceptionMask & kStreamBad)
                    throw;
            }
            // Phase 3: a failed facet result marks failbit|badbit, then applies the stream's
            // exception mask. Preserve this separate path from a thrown facet call.
            if (failed) {
                streamState = (unsigned char*)self->object;
                ((OutputStreamView*)streamState)->stateBits |= kStreamFailOrBad;
                if (((OutputStreamView*)streamState)->streamBuffer == 0)
                    ((OutputStreamView*)streamState)->stateBits |= kStreamBad;
                unsigned char b = ((OutputStreamView*)streamState)->exceptionMask;
                if (b & ((OutputStreamView*)streamState)->stateBits)
                    MWStream_ThrowFailure(streamState, iosClearFailureMessage);
            }
        }
    } catch (...) {
        sentry.flag39 = 1;
        throw;
    }
    return self;
}

#pragma use_lmw_stmw reset

// Acquire shared frame services at EACH original use: callbacks can replace
// singleton pointers. Static storage owns the objects; registrationBase points
// to persistent destructor records. These helpers do not cache returned pointers.
// Initialize/register ResetState if needed, then publish its borrowed singleton.
inline void EnsureFrameResetState(char* registrationBase) {
    if (!Sing_805DC948()) {
        if (!s_systemResetStateInitialized) {
            SystemResetState_Ctor(s_systemResetStorage);
            __register_global_object(s_systemResetStorage, (void*)SystemResetState_Dtor, registrationBase);
            s_systemResetStateInitialized = 1;
        }
        Sing_805DC948() = s_systemResetStorage;
    }
}
// Initialize/register HomeMenu if needed, then publish its borrowed singleton.
inline void EnsureFrameHomeMenu(char* registrationBase) {
    if (!((volatile void*&)Sing_805DC820())) {
        if (!s_homeMenuInitialized) {
            HomeButtonMenu_InitializeState(s_homeButtonResourceState);
            __register_global_object(s_homeButtonResourceState, (void*)HomeButtonResources_dtor,
                                     (registrationBase + kHomeMenuRegistration));
            s_homeMenuInitialized = 1;
        }
        ((volatile void*&)Sing_805DC820()) = s_homeButtonResourceState;
    }
}
// Initialize/register Input if needed, then publish its borrowed singleton.
inline void EnsureFrameInput(char* registrationBase) {
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     (registrationBase + kInputRegistration));
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
}
// Construct SaveData once if needed, then publish its borrowed singleton.
// This helper registers no destructor; registrationBase is an unused ABI-shaped
// helper argument retained to keep the EnsureFrame call sites uniform.
inline void EnsureFrameSaveData(char* registrationBase) {
    if (!GetSaveDataSlot()) {
        if (s_saveDataInitialized == 0) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        GetSaveDataSlot() = s_saveData;
    }
}
// Initialize/register Fade if needed, then publish its borrowed singleton.
inline void EnsureFrameFade(char* registrationBase) {
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, (registrationBase + kFadeRegistration));
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
}
// Initialize/register BpController if needed, then publish its borrowed singleton.
inline void EnsureFrameBpController(char* registrationBase) {
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (registrationBase + kBpControllerRegistration));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}
// Initialize/register LoadingIndicator if needed, then publish its borrowed singleton.
inline void EnsureFrameLoadingIndicator(char* registrationBase) {
    if (!Sing_805DC610()) {
        if (!s_nowReloadingInitialized) {
            ((CNowReloading*)(&s_nowReloading))->Init();
            __register_global_object(&s_nowReloading, (void*)__dt__13CNowReloadingFv,
                                     (registrationBase + kReloadingRegistration));
            s_nowReloadingInitialized = 1;
        }
        Sing_805DC610() = &s_nowReloading;
    }
}
// Initialize/register MessageDraw if needed, then publish its borrowed singleton.
inline void EnsureFrameMessageDraw(char* registrationBase) {
    if (!Sing_805DC650()) {
        if (!s_pointerPairListInitialized) {
            PairVector_InitReserved(s_messageDrawPairList);
            __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor,
                                     (registrationBase + kMessageDrawRegistration));
            s_pointerPairListInitialized = 1;
        }
        Sing_805DC650() = s_messageDrawPairList;
    }
}
// Initialize/register NumberDraw if needed, then publish its borrowed singleton.
inline void EnsureFrameNumberDraw(char* registrationBase) {
    if (Sing_805DC830() == 0) {
        if (s_pointerVectorInitialized == 0) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                     (registrationBase + kNumberDrawRegistration));
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = s_fontNumDrawRegistry;
    }
}
// Initialize/register EffectFade if needed, then publish its borrowed singleton.
inline void EnsureFrameEffectFade(char* registrationBase) {
    if (!Sing_805DC660()) {
        if (!s_effectFadeInitialized) {
            ((CEffectFade*)s_effectFade)->__ct();
            __register_global_object(s_effectFade, (void*)__dt__11CEffectFadeFv,
                                     (registrationBase + kEffectFadeRegistration));
            s_effectFadeInitialized = 1;
        }
        Sing_805DC660() = s_effectFade;
    }
}

// Run reset/input/task updates and all UI render phases, then present each frame.
// Service controls and writers (lookup before changing frame cadence):
// - s_skipNextRuntimeSubsystemServiceStorage: first int in an eight-byte slot,
//   initialized to 1 in the data section; startup reads it without clearing, this
//   loop is the source writer that clears it after skipping ALL five services.
// - HomeButtonMenu_IsActive / HomeButtonMenu_ProcessState: HOME state is owned
//   by home_button_menu.cpp's menu API; while active only
//   audio/media run. File flush, InputEx and InputManager_Update are skipped.
// - SystemReset_ProcessPendingReset: nonzero restarts the iteration before any
//   service/save/render/present work. Its state comes from EnsureFrameResetState.
// - ActorLoading_GetSecondaryStatus and CNowReloading::IsPlayerAbsent gate BP
//   callbacks/drawing only; they do not gate the text/fade/HOME/present phases.
// - CCheckDriveStatus::SetSkipDialogDelay is forced 0 before project update and
//   1 before presentation each serviced iteration, independently of HOME.
// No service clock scalar is defined here: simulation delta and presentation
// interval changes must follow the separate entry points in the opening index.
void Game_MainLoop(void) {
    // Scheduling map (same-frame order):
    // reset -> runtime service -> save change -> clear/alternate fade -> project
    // -> conditional BP callbacks/draw -> message/number text -> effect/primary
    // fade -> loading player -> cursors/HOME -> hooks -> final overlay/present.
    // HOME pauses file/input service, but audio/media still update. BP rendering
    // is independently gated below; all later overlay phases still execute.
    // Frame order is observable: keep singleton acquisition at each use, since
    // callbacks may affect shared state. The loop exits through reset/application
    // machinery elsewhere; a pending reset skips the remainder of this iteration.
    char* registrationBase = s_systemResetDtorRegistration_GameMain;
    GameHook_OnMainLoopEnter();
    unsigned int* rectData = gameLoopOverlayRectangleWords;
    unsigned int* textData = gameLoopOverlayTextParameters;
    char* message = alphabetAndStringErrorStrings;
    while (1) {
        int loadingOverlayActive;
        // Phase 1: reset handling, then HOME-aware subsystem/input servicing.
        // The startup flag skips this service block once, then is consumed below.
        GameHook_OnFrameBegin();
        EnsureFrameResetState(registrationBase);
        if (SystemReset_ProcessPendingReset(Sing_805DC948(), 1))
            continue;

        if (s_skipNextRuntimeSubsystemServiceStorage) {
            s_skipNextRuntimeSubsystemServiceStorage = 0;
        } else {
            EnsureFrameHomeMenu(registrationBase);
            if (HomeButtonMenu_IsActive((void*)((volatile void*&)Sing_805DC820()))) {
                gtl_Audio_Update();
                gtl_Media_Update();
            } else {
                FileIO_FlushAllDevices();
                gtl_Audio_Update();
                gtl_InputEx_Update();
                gtl_Media_Update();
                EnsureFrameInput(registrationBase);
                InputManager_Update(GetInputManagerSlot());
            }
        }

        // Phase 2: process slot changes and clamp the copied record's counter
        // before committing it back. recordCopy is temporary stack storage.
        EnsureFrameSaveData(registrationBase);
        if (SaveData_HasSlotSelectionChanged(GetSaveDataSlot())) {
            EnsureFrameSaveData(registrationBase);
            void* currentRecord = SaveData_GetCurrentRecord(GetSaveDataSlot());
            char recordCopy[0x830];
            SaveRecord_CopyCtor(recordCopy, currentRecord);
            int counter = ((SaveCounterView*)recordCopy)->incrementedCounter;
            int incrementedCounter = 1 + counter;
            counter = incrementedCounter;
            ((SaveCounterView*)recordCopy)->incrementedCounter = counter;
            if (0 > counter)
                counter = 0;
            counter = (counter > kSaveCounterMaximum) ? kSaveCounterMaximum : counter;
            ((SaveCounterView*)recordCopy)->incrementedCounter = counter;
            EnsureFrameSaveData(registrationBase);
            SaveData_SetCurrentRecord(GetSaveDataSlot(), recordCopy);
        }

        // Phase 3: poll loading, clear the frame, draw alternate fade, then run
        // project clock/update hooks, then registered-object callbacks.
        LoadingScreen_PollResourceReadiness();
        CCheckDriveStatus::SetSkipDialogDelay(0);
        void* extensionProperty = (void*)((CProjectG*)g_project)->GetExtensionProperty();
        Engine_ClearFramebuffer(0, 0, 7, kFloat_1_GameClearDepth, extensionProperty, 0);

        EnsureFrameFade(registrationBase);
        FadeState_DrawAlternatePhase(Sing_805DC608());

        // Refresh frame time and dispatch main-frame update hooks.
        g_project->m12(0);
        if (g_project) {
            // Invoke each registered object with three null callback arguments.
            g_project->m14(0, 0, 0, 0);
        }

        EnsureFrameBpController(registrationBase);
        // Phase 4: BP callbacks/drawing are suppressed while a loading actor is
        // active. Reset BP per-frame state in either case, then draw help/text.
        void* uiRegistry = Sing_805DC5F0();

        // Secondary status zero suppresses BP immediately. Only a nonzero
        // status consults the loading player; an existing player suppresses BP.
        if (ActorLoading_GetSecondaryStatus(g_taskManager) == 0) {
            loadingOverlayActive = 1;
        } else {
            EnsureFrameLoadingIndicator(registrationBase);
            loadingOverlayActive = CNowReloading::IsPlayerAbsent(Sing_805DC610()) == 0 ? 1 : 0;
            if (!loadingOverlayActive) {
                ((CBpCtrl*)uiRegistry)->InvokeEntryCallbacks(0);
                ((CBpCtrl*)uiRegistry)->DrawEntries();
            }
        }
        ((CBpCtrl*)uiRegistry)->ResetEntryFrameState();
        if (g_helpMsg)
            NoticeQueue_UpdateAndDraw(g_helpMsg);

        EnsureFrameMessageDraw(registrationBase);
        uiRegistry = Sing_805DC650();
        MsgDrawList_UpdateReveal(uiRegistry);
        MsgDrawList_DrawGlyphs(uiRegistry);

        EnsureFrameNumberDraw(registrationBase);
        UITextTable_DrawList(Sing_805DC830());

        EnsureFrameEffectFade(registrationBase);
        // Phase 5: effect overlay, fade update/primary draw, loading script,
        // cursors, HOME state, then frame-completion hooks (in that order).
        ImmediateOverlay_DrawUntexturedVertices(Sing_805DC660());

        EnsureFrameFade(registrationBase);
        FadeState_Update(Sing_805DC608());

        EnsureFrameFade(registrationBase);
        FadeState_DrawPrimaryPhase(Sing_805DC608());

        EnsureFrameLoadingIndicator(registrationBase);
        B2dPlayer_UpdateAndDraw(Sing_805DC610());

        EnsureFrameInput(registrationBase);
        InputManager_DrawCursors(GetInputManagerSlot());

        EnsureFrameHomeMenu(registrationBase);
        HomeButtonMenu_ProcessState((void*)((volatile void*&)Sing_805DC820()));

        Game_PreOverlayFrameHook();
        GameHook_OnFrameRenderComplete();

        // Phase 6: draw the final rectangle/text overlay and present the frame.
        Rect rect = {(int)rectData[0], (int)rectData[1], (int)rectData[2], (int)rectData[3]};
        Render_DrawFilledRect(&rect, 0x80u, (float)kFloat_0_ReloadingOverlayDepth);

        unsigned int textParams[3] = {textData[0], textData[1], textData[2]};
        gtl_BitmapFont_FormatTextVertices(textParams, message + (int)&((AlphabetPool*)0)->passwordMarker);

        gtl_BitmapFont_Draw();
        CCheckDriveStatus::SetSkipDialogDelay(1);
        Engine_PresentFrame(0, 0, 0, 0);
    }
}
extern void InstanceTable_Get(void);
extern void GolfBall__Fv(void);
extern void CQuadTree_GetInstance(void);
extern void String_ReserveCapacity(void*, unsigned int);
extern void CBpCtrlHandle_Dtor(void);
extern void MovieController_Ctor(void*);
extern void MovieController_Dtor(void);
extern void SharedFilePointerVector_InitReserved1024(void*);
extern void SharedFilePointerVector_Destroy(void);
extern void __dt__9CBpPlayerFv(void);
extern void CTourHintWindow_dtor(void);
extern void ItemRequestState_Ctor(void*);
extern void ItemRequestState_Dtor(void);
extern void Destroy__13CTourVSWindowFi(void);
extern void MWIOS_InitializeStreamBuffer(void*, void*);
extern void* MWLocale_GetDefault();
extern void std_ios_base_dtor(void*, short);
extern void* OStream_WriteCString(void*, const char*);
extern void* OStream_InsertLong(Wrapper*, void*);
extern char s_loadingDrawStringStorage[];
extern int s_bpCtrlHandle[2];
extern char s_movieController[];
extern char s_sharedFileListStorage[];
extern char s_bpPlayer[];
extern char s_tourHintWindow[];
extern char s_itemRequestState[];
extern char s_tourVSWindow[];
extern unsigned char s_infoBaseFontRegistryInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern signed char s_movieControllerInitialized;
extern signed char s_sharedFileListInitialized;
extern signed char s_bpPlayerInitialized;
extern signed char s_tourHintWindowInitialized;
extern signed char s_itemRequestStateInitialized;
extern signed char s_tourVsWindowInitialized;
extern char iosBaseVtable[];
extern char iStreamVtableGroup[];
extern char s_basicOstreamVtable[];
extern char outputStringStreamVtables[];
extern char s_appStringstreamVTables[];
extern char stringStreamBufferVtable[];
extern char s_appStringbufVTable[];
extern char alphabetAndStringErrorStrings[];
extern unsigned int s_ctypeFacetId;
extern void MWCtypeFacet_Construct(void*, void*, void*, int);
extern void WPuppet_EnsureTransformBufferCapacity(int);
extern void String_InitTemporaryFormatBuffers();
extern void SharedStringState_ReserveFirstStrings();
extern void appFreeUni__FPv(void*);
struct InitItem {
    char data[0x18];
    // Construct the locale-owned character-classification facet.
    InitItem() {
        MWCtypeFacet_Construct(this, 0, 0, 0);
    }
};
struct InitVirtual {
    virtual void a();
    virtual void b();
    virtual void c();
    virtual void d();
    virtual void e();
    virtual signed char value(signed char);
};
// Copy the default locale's two-word handle, acquiring its reference-counted owner.
inline void CopyInitShared(void** destination) throw() {
    void** shared = (void**)MWLocale_GetDefault();
    destination[0] = shared[0];
    destination[1] = shared[1];
    if (destination[1])
        ++((int*)destination[1])[1];
}
struct InitVector {
    VecLike* p;
    void* ref;
    // Drop the locale reference used while looking up its character facet.
    ~InitVector() throw() {
        if (ref)
            MWSharedOwner_ReleaseReference(ref);
    }
};
// Release a copied locale owner if one exists.
inline void ReleaseInitShared(void* ref) throw() {
    if (ref)
        MWSharedOwner_ReleaseReference(ref);
}
// Stack stringstream used to initialize locale facets and allocator-backed formatting state.
// Private MW multiple-inheritance layout (32-bit words):
// +00 base -> backing ios storage; +08 input vtable; +0C base2 -> same backing;
// +10 output vtable; +14 stringbuf vtable; +18/+1C/+20 read-area pointers;
// +24/+28/+2C write-area pointers; +30/+34 locale handle/reference;
// +38 mode bits; +3C active character buffer; +40 string flags/inline bytes;
// +48 heap pointer (owned only when length's high bit is set); +4C ios backing.
// The +0x3C words INSIDE ios backing are virtual-base adjustments, not stream
// data pointers. Each vtable construction/destruction stage rewrites that offset
// to the subobject required by that stage. Keep those writes and table order.
// Read area = (p18,p1c,p20), write area = (p24,p28,p2c). All six start
// at the same empty inline buffer, so this constructor alone cannot prove
// begin/current/end roles. These are borrowed cursors, never six owners.
// Layout offsets are bytes; capacity below is reserved character count.
struct InitStream {
    char* base;              // +00 input-side pointer to shared ios backing
    int zero;                // +04 cleared during construction; role unresolved
    void* table;             // +08 input vtable, rewritten at each derived stage
    char* base2;             // +0C output-side pointer to the same ios backing
    void* table2;            // +10 output vtable
    void* table3;            // +14 embedded stringbuf vtable
    void* readAreaPointer0;  // +18 read-area pointer 0
    void* readAreaPointer1;  // +1C read-area pointer 1
    void* readAreaPointer2;  // +20 read-area pointer 2
    void* writeAreaPointer0; // +24 write-area pointer 0
    void* writeAreaPointer1; // +28 write-area pointer 1
    void* writeAreaPointer2; // +2C write-area pointer 2
    void* shared;            // +30 borrowed locale implementation
    void* ref;               // +34 retained locale reference owner
    unsigned char flags;     // +38 read/write-area enable bits (8/16)
    char pad[3];
    void* buffer;        // +3C active string characters
    unsigned int length; // +40 packed string tag/inline bytes; high bit owns heap
    int x44;             // +44 second packed-string word, initially zero
    void* heap;          // +48 long-string buffer; not owned for inline form
    char backing[0x40];  // +4C shared virtual ios base storage
    // Build the stream bases, borrow/install locale facets, then initialize empty read/write areas.
    InitStream() {
        InitStream& s = *this;
        // Phase 1: construct ios/input bases and install the locale-derived fill byte.
        s.base = s.backing;
        *(void**)s.backing = iosBaseVtable;
        *(void**)s.backing = iStreamVtableGroup + 12;
        s.base2 = s.backing;
        s.table = iStreamVtableGroup;
        *(int*)(s.base + kVirtualBaseOffsetWord) = (char*)&s.base2 - s.base;
        char* base = s.base;
        s.zero = 0;
        MWIOS_InitializeStreamBuffer(base, &s.table3);
        ((OutputStreamView*)base)->reserved34 = 0;
        signed char value;
        {
            InitVector v;
            MWStream_CopyLocale(&v, base);
            volatile unsigned char flag = 0;
            if (!s_ctypeFacetId) {
                ++s_localeFacetIdCounter;
                s_ctypeFacetId = s_localeFacetIdCounter;
            }
            unsigned int index = s_ctypeFacetId;
            InitVirtual* item;
            if (index >= v.p->size || !(item = ((InitVirtual**)v.p->ptr)[index])) {
                InitItem* allocated = new InitItem;
                if (!s_ctypeFacetId) {
                    ++s_localeFacetIdCounter;
                    s_ctypeFacetId = s_localeFacetIdCounter;
                }
                MWLocale_InstallFacet(v.p, allocated, s_ctypeFacetId);
                item = ((InitVirtual**)v.p->ptr)[index];
            }
            value = item->value(0x20);
        }
        ((OutputStreamView*)base)->fillCharacter = value;
        // Phase 2: install output, stringstream, then application vtable stages.
        s.table2 = s_basicOstreamVtable;
        *(void**)s.base2 = s_basicOstreamVtable + 12;
        *(int*)(s.base2 + kVirtualBaseOffsetWord) = (char*)&s.table3 - s.base2;
        s.table = outputStringStreamVtables;
        *(void**)s.base = outputStringStreamVtables + 24;
        s.table2 = outputStringStreamVtables + 12;
        *(int*)(s.base + kVirtualBaseOffsetWord) = (char*)&s.table3 - s.base;
        s.table = s_appStringstreamVTables;
        *(void**)s.base = s_appStringstreamVTables + 24;
        s.table2 = s_appStringstreamVTables + 12;
        *(int*)(s.base + kVirtualBaseOffsetWord) = s.backing - s.base;
        // Phase 3: initialize an empty owning stringbuf; both cursor areas borrow
        // its inline characters. The locale handle owns its retained reference.
        s.table3 = stringStreamBufferVtable;
        s.readAreaPointer0 = 0;
        s.readAreaPointer1 = 0;
        s.readAreaPointer2 = 0;
        s.writeAreaPointer0 = 0;
        s.writeAreaPointer1 = 0;
        s.writeAreaPointer2 = 0;
        CopyInitShared(&s.shared);
        s.length = 0;
        s.table3 = s_appStringbufVTable;
        s.flags = kStringBufferReadArea | kStringBufferWriteArea;
        s.x44 = 0;
        s.heap = 0;
        void* buffer = (s.length >> 31) ? s.heap : (char*)&s.length + 1;
        s.buffer = buffer;
        if (s.flags & kStringBufferWriteArea) {
            s.writeAreaPointer1 = buffer;
            s.writeAreaPointer0 = buffer;
            s.writeAreaPointer2 = buffer;
        }
        if (s.flags & kStringBufferReadArea) {
            s.readAreaPointer0 = buffer;
            s.readAreaPointer1 = buffer;
            s.readAreaPointer2 = buffer;
        }
    }
    // Free heap characters and locale reference, then unwind virtual-base stages in reverse.
    ~InitStream() {
        InitStream& s = *this;
        *(int*)(s.base + kVirtualBaseOffsetWord) = s.backing - s.base;
        if (s.length >> 31)
            appFreeUni__FPv(s.heap);
        ReleaseInitShared(s.ref);
        *(int*)(s.base + kVirtualBaseOffsetWord) = (char*)&s.table3 - s.base;
        *(int*)(s.base2 + kVirtualBaseOffsetWord) = (char*)&s.table3 - s.base2;
        *(int*)(s.base + kVirtualBaseOffsetWord) = (char*)&s.base2 - s.base;
        std_ios_base_dtor(s.backing, 0);
    }
};

// Create shared controllers, warm up string/locale storage, and reserve common rendering buffers.
void Game_InitializeSingletons(void) {
    // Phase 1: initialize shared controllers in registration order. A non-null
    // singleton borrows static storage; global destructor records own teardown.
    char* globals = s_systemResetDtorRegistration_GameMain;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (globals + kBpControllerRegistration));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[1] = -1;
            s_bpCtrlHandle[0] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (globals + kBpHandleRegistration));
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, (globals + kInputRegistration));
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, (globals + kFadeRegistration));
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    if (!Sing_805DC950()) {
        if (!s_movieControllerInitialized) {
            MovieController_Ctor(s_movieController);
            __register_global_object(s_movieController, (void*)MovieController_Dtor, (globals + kMovieRegistration));
            s_movieControllerInitialized = 1;
        }
        Sing_805DC950() = s_movieController;
    }
    if (!Sing_805DC660()) {
        if (!s_effectFadeInitialized) {
            ((CEffectFade*)s_effectFade)->__ct();
            __register_global_object(s_effectFade, (void*)__dt__11CEffectFadeFv, (globals + kEffectFadeRegistration));
            s_effectFadeInitialized = 1;
        }
        Sing_805DC660() = s_effectFade;
    }
    if (!Sing_805DC680()) {
        if (!s_sharedFileListInitialized) {
            SharedFilePointerVector_InitReserved1024(s_sharedFileListStorage);
            __register_global_object(s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy, (globals + 0x18));
            s_sharedFileListInitialized = 1;
        }
        Sing_805DC680() = s_sharedFileListStorage;
    }
    if (!Sing_805DC670()) {
        if (!s_bpPlayerInitialized) {
            ((CBpPlayer*)s_bpPlayer)->__ct();
            __register_global_object(s_bpPlayer, (void*)__dt__9CBpPlayerFv, (globals + kBpPlayerRegistration));
            s_bpPlayerInitialized = 1;
        }
        Sing_805DC670() = s_bpPlayer;
    }
    if (!Singleton_TourHintWindow()) {
        if (!s_tourHintWindowInitialized) {
            ((CTourHintWindow*)s_tourHintWindow)->__ct();
            __register_global_object(s_tourHintWindow, (void*)CTourHintWindow_dtor, (globals + kTourHintRegistration));
            s_tourHintWindowInitialized = 1;
        }
        Singleton_TourHintWindow() = s_tourHintWindow;
    }
    if (!Sing_805DC958()) {
        if (!s_itemRequestStateInitialized) {
            ItemRequestState_Ctor(s_itemRequestState);
            __register_global_object(s_itemRequestState, (void*)ItemRequestState_Dtor,
                                     (globals + kItemRequestRegistration));
            s_itemRequestStateInitialized = 1;
        }
        Sing_805DC958() = s_itemRequestState;
    }
    if (!Sing_805DC690()) {
        if (!s_tourVsWindowInitialized) {
            ((CTourVSWindow*)s_tourVSWindow)->__ct();
            __register_global_object(s_tourVSWindow, (void*)Destroy__13CTourVSWindowFi,
                                     (globals + kTourVsRegistration));
            s_tourVsWindowInitialized = 1;
        }
        Sing_805DC690() = s_tourVSWindow;
    }
    // Phase 2: force registry construction and reserve formatting/model buffers.
    if (!s_infoBaseFontRegistryInitialized) {
        char temporary[0x18];
        ((CInfoBase*)temporary)->__ct();
        ((CInfoBase*)temporary)->~CInfoBase();
    }
    InstanceTable_Get();
    GolfBall__Fv();
    CWind::GetInstance();
    CQuadTree_GetInstance();
    String_ReserveCapacity(s_loadingDrawStringStorage, kWarmupStringCapacity);
    // The stack stream exercises both text and numeric formatting; its destructor
    // drops temporary text/locale ownership before initialization returns.
    InitStream s;
    void* result = OStream_WriteCString((void*)&s.base2, alphabetAndStringErrorStrings);
    result = OStream_WriteCString(result, alphabetAndStringErrorStrings + (int)&((AlphabetPool*)0)->digits);
    OStream_InsertLong((Wrapper*)result, (void*)12345678);
    WPuppet_EnsureTransformBufferCapacity(kPuppetTransformCapacity);
    String_InitTemporaryFormatBuffers();
    SharedStringState_ReserveFirstStrings();
}
// Compile-time guards for private ABI views used above.
#define LOCAL_FIELD(t, f, o) typedef char local_##t##_##f[((int)&((t*)0)->f == (o)) ? 1 : -1]
LOCAL_FIELD(OutputStreamView, streamBuffer, 0x24);
LOCAL_FIELD(OutputStreamView, flags, 0x30);
LOCAL_FIELD(OutputStreamView, stateBits, 0x32);
LOCAL_FIELD(OutputStreamView, exceptionMask, 0x33);
LOCAL_FIELD(OutputStreamView, fillCharacter, 0x38);
LOCAL_FIELD(SaveCounterView, incrementedCounter, 0x20);
LOCAL_FIELD(AlphabetPool, passwordMarker, 0x26);
#undef LOCAL_FIELD

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621F8", _ctors_u80214D14_0, Game_InitializeBpStartupGuardsAndFullTurn);
#pragma pop
