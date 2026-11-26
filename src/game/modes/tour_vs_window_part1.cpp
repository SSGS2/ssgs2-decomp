// Tour versus window: the VS banner shown before a tour match, with its staggered panel expansion,
// player name colors and fade-out. Also the tour task's actor hooks (create, load, process, draw, destroy).

#include <decomp/static_init.h>
// Tour versus window: staggered panel expansion/settling, player banners and BP
// scripts. Modding lookup (settings are shared, preserve coupled relationships):
// - UpdateTransition: kTourVsPanelAcceleration (layout X/update^2), expansion
//   duration and tourVsPanelStartDelays (update units); speed carries into settle.
// - Settling: 660 start -> kTourVsSettledPanelX=320; distance factor 680 must
//   remain 2*(start-center). Change these together, including draw clipping.
// - Draw: tourVsDrawDefaults/s_tourVsDrawDefaults holds origins, two six-pair stage
//   presets, full/banner scissors and the textured quad. The Region's w/h store
//   player B's origin. kTourVsQuad*/Outer*/Border* set panel and border geometry.
// - Texture sampling: width 256, height 242 and reciprocal 1/256 form the UVs;
//   trail offsets 64/32 and kTourVsBrightnessEaseDivisor affect name layers.
// - SetEntryIndices / InitLayout: selected banner indices also choose name-strip
//   frames (30 layout units per entry). Strings/group names and initial geometry
//   live in the data section and the related Tour-map constant pools; search each
//   extern symbol below before editing a shared scalar.
// ABI/layout: TourVSWindowData.h; BP controller entry lifetime belongs to window.
extern "C" {
extern unsigned char s_tourVsDrawDefaults[]; // VS banner geometry, clipping and state presets.
}
enum TourVsTransitionState {
    kVsHidden = 0,
    kVsExpanding = 1,
    kVsSettling = 2,
    kVsAnimating = 3,
    kVsReady = 4
};
enum TourTaskActorStringOffset {
    kTourActorClassName = 1,
    kTourActorInstanceName = 0xB
};
enum TourVsConstants {
    kVsHiddenStatus = 3,
    kVsNameFrameHeight = 30,
    kVsBannerCount = 5,
    kVsFadingGroupCount = 9
};
enum TourVsDrawConstants {
    kVsQuadPrimitive = 0x98,
    kVsTexturedVertexFormat = 0x11F005,
    kVsTexturedVertexBytes = 0x18,
    kVsViewportWidth = 640,
    kVsPanelMaxAlpha = 0xA0,
    kVsNameDimBrightness = 80,
    kVsFullByte = 255
};
// Byte offsets into the existing packed string pool; no new string storage.
enum TourVsStringOffset {
    kVsDefaultScript = 0xC,
    kVsPlayerOneCharacterGroup = 0x13,
    kVsPlayerTwoCharacterGroup = 0x1E,
    kVsPlayerOneNameGroup = 0x29,
    kVsPlayerOneNameBaseGroup = 0x38,
    kVsPlayerTwoNameGroup = 0x46,
    kVsPlayerTwoNameBaseGroup = 0x55,
    kVsScrollGroup = 0x63,
    kVsEntranceSound = 0x6C,
    kVsAnimationScript = 0x77,
    kVsIconSound = 0x80,
    kVsExitScript = 0x8D,
};

#include "classes/TourVSWindowData.h"
#include "sdk_protos.h"
#include "game/types.h"

#define GAME_NSTRING_RANGE_CONSTRUCTOR
#include "game/string.h"

// Stack-local narrow-string view; the canonical string ABI remains external.
class LocalNString_u802F61E4 {
public:
    unsigned int flags;
    unsigned int length;
    char* data;

    // Construct an empty inline narrow string.
    LocalNString_u802F61E4() : flags(0), length(0), data(0) {}
#if defined(GAME_NSTRING_TEXT_CONSTRUCTOR)
    // Copy a borrowed C string into owned inline or heap storage.
    LocalNString_u802F61E4(const char* text) {
        MWString_ConstructCString(this, text);
    }
#endif
#if defined(GAME_NSTRING_RANGE_CONSTRUCTOR)
    // Copy a borrowed C string into owned inline or heap storage.
    LocalNString_u802F61E4(const char* text) {
        LocalNString_u802F61E4* object = this;
        unsigned int size;
        const char* source = text;
        object->flags = 0;
        object->length = 0;
        object->data = 0;
        size = nstring_detail::strlen(source);
        nstring_detail::String_ReserveCapacity(object, size);
        int allocator;
        char copy = *(char*)&allocator;
        nstring_detail::String_ReplaceRange(object, 0, 0, source, source + size, &copy);
    }
#endif
    // Free heap-backed character storage; inline characters need no cleanup.
    ~LocalNString_u802F61E4() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
    // Return the logical length for either string-storage representation.
    unsigned int size() const {
        return (flags >> 31) == 0 ? (*(const unsigned char*)this & 0x7f) : length;
    }
    // Borrow the NUL-terminated character buffer until the string changes.
    const char* c_str() const {
        return (flags >> 31) == 0 ? (const char*)this + 1 : data;
    }
};

// Tour task and VS window, 0x802F61E4-0x802F85E0.

#include "game/singletons.h"
#include "classes/CTourTask.h"
#include "classes/CTask.h"
#include "classes/CTourVSWindow.h"
#include "classes/CFade.h"
#include "classes/CBpCtrl.h"
#include "classes/CTourMessage.h"

extern "C" {

extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char __vt__13CTourVSWindow[];
extern char s_bpCtrlDtorRecord_TourMap[];
extern char tourVsWindowStrings[];
extern char tourTaskActorStrings[]; // Packed empty string, CTourMain class name, and TourMain instance name.
extern char s_tourTaskRuntimeType;  // Tour-task runtime type descriptor address.
extern const float kFloat_0_0_TourMap;
extern const float kFloat_neg30_TourVsInitialMiddle; // -30: kFloat_neg30_TourVsInitialMiddle.
extern const float kFloat_neg55_TourVsInitialEdge;   // -55: kFloat_neg55_TourVsInitialEdge.
extern float kFloat_6_2831855;
extern float s_tourVsFullTurnAngle; // Module angle storage initialized to one full turn.

void CBpCtrl_dtor(void*, int);
void* IdMap_Lookup(void*, unsigned int);
void BpObject_SetEnabled(void*, int);
void BpObject_StartScriptByName(void*, char*, int);
void* BpObject_GetOrCreateNamedGroup(void*, void*);
void UIGroup_SetEnabled(void*, int);
void UIGroup_SetSourcePosition(void*, void*, int);
int BpObject_IsCurrentScriptComplete(void*);

void __dt__5CTaskFv(void*, int);
void __dl__FPv(void*);
void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);
extern void* g_soundManager;
extern const float kFloat_0_5_TourVSWindow;
extern const float kFloat_200_TourVsSound;   // 200: kFloat_200_TourVsSound.
extern const float kFloat_1_TourVsSound;     // 1: kFloat_1_TourVsSound.
extern const float kTourVsPanelAcceleration; // 3.299999952316284: kTourVsPanelAcceleration.
extern const float kTourVsExpansionDuration; // 20: kTourVsExpansionDuration.
extern const float kFloat_660_TourMap;
extern const float kTourVsSettleDistanceFactor; // 680: kTourVsSettleDistanceFactor.
extern const float kTourVsSettledPanelX;        // 320: kTourVsSettledPanelX.

struct BpIdSet20 {
    unsigned int id[5];
};

struct BpIdSet36 {
    unsigned int id[9];
};

extern BpIdSet20 tourVsIconGroupNames;              // Club, VS-letter, and flash group names (five pointers).
extern BpIdSet36 tourVsMessageWindowHideGroupNames; // Nine message-window group names, copied before hiding.
extern BpIdSet36
    tourVsMessageWindowOpacityGroupNames; // Nine message-window group names, copied before changing opacity.

extern Arr3 tourVsPanelStartDelays; // Three staggered panel start delays.

void* TaskActorTable_CreateNamedActor(void*, const char*, const char*, int, void*);
void* __destroy_new_array(void*, void*);
extern "C" void __dt__6CBpTexFv(void*, int);
void UIGroup_SetOpacity(void*, void*);
void* __dt__15TourMessageBaseFv(void*, int);

struct CtlValue {
    unsigned int zero;
    float value;
};

// Resolve the lazily constructed BP controller at each original call site.
static inline void EnsureTourVsBpController() {
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_TourMap);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

struct FmtBlock {
    unsigned int w[12];
};
struct ScissorRect {
    int x, y, w, h;
};
struct Region {
    float x, y, w, h;
};
struct Vtx16 {
    float x, y, z;
    unsigned char r, g, b, a;
    float u, v;
};
struct Vtx16Quad {
    Vtx16 v[4];
};
struct Vtx12 {
    float x, y, z;
    unsigned char r, g, b, a;
};
struct VsLayout {
    char pad0[0x1B0];
    Region region;     // 0x1B0
    FmtBlock fmt2;     // 0x1C0
    FmtBlock fmt1;     // 0x1F0
    ScissorRect rectA; // 0x220
    ScissorRect rectB; // 0x230
    Vtx16Quad quad;    // 0x240
};

struct BannerEntry {
    virtual void vfunc_0();
    virtual void vfunc_1();
    virtual void vfunc_2();
    virtual void Draw();
    char pad[0x24];
};

// Original table at 0x8048E430: retain the scalar aggregate copy types.
#define OFF(f, o) typedef char VsLayout_off_##f[((int)&((VsLayout*)0)->f == (o)) ? 1 : -1]
OFF(region, 0x1B0);
OFF(fmt2, 0x1C0);
OFF(fmt1, 0x1F0);
OFF(rectA, 0x220);
OFF(rectB, 0x230);
OFF(quad, 0x240);
#undef OFF
extern VsLayout tourVsDrawDefaults;          // Default VS geometry, scissor rectangles, and render-state blocks.
extern float kFloat_256_TourVsTextureWidth;  // 256: kFloat_256_TourVsTextureWidth.
extern float kFloat_242_TourVsTextureHeight; // 242: kFloat_242_TourVsTextureHeight.
extern const float kFloat_OneOver256_TourVsTexcoord; // 0.00390625: kFloat_OneOver256_TourVsTexcoord.
extern const float kTourVsQuadTopY;
extern const float kTourVsQuadBottomY;
extern const float kTourVsBannerOffsetX64;
extern const float kTourVsBannerOffsetX32;
extern const float kTourVsOuterTopY;
extern const float kFloat_51_TourVsInnerTopY;     // 51: kFloat_51_TourVsInnerTopY.
extern const float kFloat_289_TourVsInnerBottomY; // 289: kFloat_289_TourVsInnerBottomY.
extern const float kTourVsOuterBottomY;
extern const float kTourVsBorderHalfWidth;
extern const float kTourVsQuadRightEdgeX;
extern const float kTourVsBrightnessEaseDivisor;

void Render_SetColoredQuadState();                 // Render_SetColoredQuadState.
void Render_DrawColoredPrimitive(int, void*, int); // Render_DrawColoredPrimitive.
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetFlexibleVertexFormat(int);
int GX_GetPrimitiveCount(int, int);
void DrawPrimitiveUP(int, int, void*, int);
void Engine_SetScissorRect(void*);
void BpObject_Draw(void*);

// Create and attach the tour task actor from the task actor-name pool.
void CTourTask::CreateActors(void* task) {
    char* actorNames = tourTaskActorStrings;
    LocalNString_u802F61E4 text(actorNames);
    void* actor = TaskActorTable_CreateNamedActor(task, actorNames + kTourActorClassName,
                                                  actorNames + kTourActorInstanceName, 0, &text);
    ((CTask*)task)->SetMainActor(actor);
}

// Start loading all actors owned by this tour task.
void CTourTask::StartActorLoading(void* forwarded0) {
    ((CTask*)forwarded0)->StartActorLoading();
}

// Advance the tour task actors by the supplied frame time.
void CTourTask::ProcessActors(void* forwarded0, float forwarded1) {
    ((CTask*)forwarded0)->ProcessActors(forwarded1);
}

// Draw the tour task actors in priority order.
void CTourTask::DisplayActorsByPriority(void* forwarded0) {
    ((CTask*)forwarded0)->DisplayActorsByPriority();
}

// Destroy the task base and optionally release its allocation.
void* CTourTask::Destroy(int deleting) {
    if (this) {
        __dt__5CTaskFv(this, 0);
        if (0 < deleting)
            __dl__FPv(this);
    }
    return this;
}

// Return the tour task runtime type descriptor.
void* CTourTask::GetRTTI() {
    return &s_tourTaskRuntimeType;
}

// Initialize the tour/VS module full-turn angle constant.
void TourVsWindow_InitAngleConstant() {
    s_tourVsFullTurnAngle = kFloat_6_2831855;
}

// Adjust the +8 interface pointer back to the task before destroying it.
void CTourTask::DtorThunk8(int deleting) {
    ((CTourTask*)((char*)this - 8))->Destroy(deleting);
}

// Initialize VS-window interfaces, banner storage, fade state and hidden geometry.
CTourVSWindow::CTourVSWindow() {
    ((CTourMessage*)this)->__ct();
    TourVSWindowData* p = (TourVSWindowData*)this;
    float v40, v3C, v38;
    ((CFade*)(p->fadeControl))->Init();
    v38 = kFloat_0_0_TourMap;
    v3C = kFloat_neg30_TourVsInitialMiddle;
    v40 = kFloat_neg55_TourVsInitialEdge;
    p->vt = __vt__13CTourVSWindow;
    p->layoutId = 0;
    p->state = kVsHidden;
    p->panelSplitX = v38;
    p->middle = v3C;
    p->edge = v40;
    p->entries = 0;
    p->entryA = 0;
    p->entryB = 0;
    p->scrollGroupEnabled = 0;
    p->status = 0;
    p->alphaA = 0xFF;
    p->alphaB = 0xFF;
    p->mode = 0;
}

#pragma push
#pragma opt_propagation off
// Hide the VS layout and release banners, BP entry, fade state, and base storage.
void* CTourVSWindow::Destroy(int deleting) {
    if (this != 0) {
        void* vt = __vt__13CTourVSWindow;

        *(void**)this = vt;
        status = kVsHiddenStatus;
        state = kVsHidden;
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_TourMap);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), layoutId), 0);
        if (entries != 0) {
            __destroy_new_array(entries, (void*)__dt__6CBpTexFv);
            entries = 0;
        }
        if ((*(int*)&layoutId) != -1) {
            EnsureTourVsBpController();
            ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(layoutId);
            layoutId = -1;
        }
        ((CFade*)fadeControl)->Dtor(0);
        __dt__15TourMessageBaseFv(this, 0);
        if (deleting > 0)
            __dl__FPv(this);
    }
    return this;
}
#pragma pop

#pragma push
#pragma opt_propagation off
// Reset the VS transition and disable its BP layout.
void CTourVSWindow::ResetAndHide() {
    int hiddenStatus;
    hiddenStatus = kVsHiddenStatus;
    status = hiddenStatus;
    state = kVsHidden;
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_TourMap);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), layoutId), 0);
}
#pragma pop

// Select the two banner entries displayed for the versus players.
// Both are zero-based indices in window->entries (BannerEntry stride 0x28),
// not BP layout IDs or the five icon-group indices. Also used as 30*index name
// source Y in InitLayout. The array is owned by the window and released by its
// destructor; indices remain cached until overwritten. Caller must install an
// array covering both nonnegative indices before InitLayout/Draw: neither this
// setter nor Draw checks bounds. The allocated count is not stored in this view.
void CTourVSWindow::SetEntryIndices(int playerOneIndex, int playerTwoIndex) {
    entryA = playerOneIndex;
    entryB = playerTwoIndex;
}

// Hide a named BP group; the caller retains its name temporary and borrowed window.
static inline void HideVsLayoutGroup128(void* window, char* name) {
    EnsureTourVsBpController();
    void* h = IdMap_Lookup(Sing_805DC5F0(), ((TourVSWindowData*)window)->layoutId);
    if (h == 0) {
        h = 0;
    } else {
        h = BpObject_GetOrCreateNamedGroup(h, name);
    }
    if (h)
        UIGroup_SetEnabled(h, 0);
}

// Reset transition geometry, configure player-name frames, and hide the VS groups.
void CTourVSWindow::InitLayout(short mode) {
    // Phase 1: activate the BP layout and start its default script.
    unsigned int i;
    float v40, v3C, v38;

    {
        void* cbp = Sing_805DC5F0();
        this->mode = mode;
        if (cbp == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_TourMap);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
    }
    BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), layoutId), 1);

    EnsureTourVsBpController();
    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), layoutId), tourVsWindowStrings + kVsDefaultScript, 1);

    {
        // Phase 2: hide banners and character groups.
        BpIdSet20 ids = tourVsIconGroupNames;
        for (i = 0; i < kVsBannerCount; i++) {
            unsigned int id = ids.id[i];
            EnsureTourVsBpController();
            void* h = IdMap_Lookup(Sing_805DC5F0(), layoutId);
            if (h == 0) {
                h = 0;
            } else {
                h = BpObject_GetOrCreateNamedGroup(h, (void*)id);
            }
            if (h)
                UIGroup_SetEnabled(h, 0);
        }
    }

    {
        char* name = tourVsWindowStrings + kVsPlayerOneCharacterGroup;
        HideVsLayoutGroup128(this, name);
    }

    {
        char* name = tourVsWindowStrings + kVsPlayerTwoCharacterGroup;
        HideVsLayoutGroup128(this, name);
    }

    {
        void* h;
        // Phase 3: select player name-strip frames before hiding both name/base groups.
        void* name = tourVsWindowStrings + kVsPlayerOneNameGroup;
        EnsureTourVsBpController();
        h = IdMap_Lookup(Sing_805DC5F0(), layoutId);
        if (h == 0) {
            h = 0;
        } else {
            h = BpObject_GetOrCreateNamedGroup(h, name);
        }
        if (h) {
            CtlValue value = {0, 0.0f};
            int amount = entryA;
            value.value = (float)(amount * kVsNameFrameHeight);
            UIGroup_SetSourcePosition(h, &value, amount);
            UIGroup_SetEnabled(h, 0);
        }
    }

    {
        char* name = tourVsWindowStrings + kVsPlayerOneNameBaseGroup;
        HideVsLayoutGroup128(this, name);
    }

    {
        void* h;
        void* name = tourVsWindowStrings + kVsPlayerTwoNameGroup;
        EnsureTourVsBpController();
        h = IdMap_Lookup(Sing_805DC5F0(), layoutId);
        if (h == 0) {
            h = 0;
        } else {
            h = BpObject_GetOrCreateNamedGroup(h, name);
        }
        if (h) {
            CtlValue value = {0, 0.0f};
            int amount = entryB;
            value.value = (float)(amount * kVsNameFrameHeight);
            UIGroup_SetSourcePosition(h, &value, amount);
            UIGroup_SetEnabled(h, 0);
        }
    }

    {
        char* name = tourVsWindowStrings + kVsPlayerTwoNameBaseGroup;
        HideVsLayoutGroup128(this, name);
    }

    {
        char* name = tourVsWindowStrings + kVsScrollGroup;
        HideVsLayoutGroup128(this, name);
    }

    {
        // Phase 4: hide fading groups, then reset geometry and transition state.
        BpIdSet36 ids = tourVsMessageWindowHideGroupNames;
        unsigned int* cursor = ids.id;
        for (i = 0; i < kVsFadingGroupCount; i++, cursor++) {
            unsigned int id = *cursor;
            EnsureTourVsBpController();
            void* h = IdMap_Lookup(Sing_805DC5F0(), layoutId);
            if (h == 0) {
                h = 0;
            } else {
                h = BpObject_GetOrCreateNamedGroup(h, (void*)id);
            }
            if (h)
                UIGroup_SetEnabled(h, 0);
        }
    }

    v38 = kFloat_0_0_TourMap;
    v3C = kFloat_neg30_TourVsInitialMiddle;
    v40 = kFloat_neg55_TourVsInitialEdge;
    panelSplitX = v38;
    middle = v3C;
    edge = v40;
    time = v38;
    alphaA = 0xFF;
    alphaB = 0xFF;
    (*(unsigned int*)&playerOneNameEnabled) = 1;
    (*(unsigned int*)&playerTwoNameEnabled) = 1;
    (*(unsigned int*)&scrollGroupEnabled) = 0;
    (*(unsigned int*)&state) = 0;
}

// Play the entrance sound and start expanding the VS panels.
void CTourVSWindow::BeginTransition() {
    Sound_PlayByName(g_soundManager, tourVsWindowStrings + kVsEntranceSound, 0, 0, 0, 0, kFloat_0_5_TourVSWindow,
                     kFloat_200_TourVsSound, kFloat_1_TourVsSound);
    state = kVsExpanding;
}

// Expand and settle three panel edges, then wait for the VS script to finish.
// Geometry uses the 640-wide layout coordinate system. panelSplitX/middle/edge are
// three adjacent float X coordinates, staggered by tourVsPanelStartDelays.
// deltaTime and window->time are animation-frame units (the observed caller in
// event_conte_script_interpreter.cpp supplies 1 per update), not seconds. Return 1 only on the update that
// completes the script and changes Animating -> Ready; an already Ready window returns 0.
int CTourVSWindow::UpdateTransition(float deltaTime) {
    unsigned int* cursor;
    int completed = 0;
    Arr3 startDelays = tourVsPanelStartDelays;
    unsigned int i;
    float panelX, halfAcceleration, delay, elapsed, time, acceleration;
    float limit, timePastLastDelay, transitionDelay, transitionNow, transitionSpeed;
    float twiceSettleDistance, speedSquared, halfFactor, settledX, offscreenX, settlingX, halfDeceleration, stopTime,
        speed, panelDelay, settleTime, settleDeceleration, panelElapsed;

    switch (this->state) {
    case kVsExpanding: {
        // Phase 1: x_i = 0.5*expansionAcceleration*max(time-delay_i,0)^2, evaluated BEFORE this tick's
        // time increment. After the third delay + expansion stopTime, snap all X
        // coordinates to 660 (off the right edge), retain expansionAcceleration*updatedTime as settling speed.
        float* out = &this->panelSplitX;

        acceleration = kTourVsPanelAcceleration;
        // panelX temporarily carries the half coefficient before receiving each panel position.
        panelX = kFloat_0_5_TourVSWindow;
        halfAcceleration = panelX * acceleration;
        for (i = 0; i < 3; i++) {
            time = this->time;
            delay = startDelays.v[i];
            elapsed = time - delay;
            panelX = (time >= delay) ? elapsed * (halfAcceleration * elapsed) : kFloat_0_0_TourMap;
            out[i] = panelX;
        }

        {
            transitionDelay = startDelays.v[2];
            transitionNow = this->time;
            transitionNow += deltaTime;
            limit = kTourVsExpansionDuration;
            timePastLastDelay = transitionNow - transitionDelay;
            this->time = transitionNow;
            if (timePastLastDelay >= limit) {
                transitionSpeed = acceleration * transitionNow;
                limit = kFloat_660_TourMap;
                timePastLastDelay = kFloat_0_0_TourMap;
                this->edge = limit;
                this->speed = transitionSpeed;
                this->time = timePastLastDelay;
                this->middle = limit;
                this->panelSplitX = limit;
                this->state = kVsSettling;
            }
        }
        break;
    }
    case kVsSettling: {
        // Phase 2: deceleration = speed^2/680. Starting at 660, each edge travels
        // speed*t - 0.5*deceleration*t^2 then clamps to 320 (screen center).
        // 680 = 2*(660-320), so that speed and deceleration stop exactly at center.
        // All three edges must finish before the BP animation is started.
        speed = this->speed;
        twiceSettleDistance = kTourVsSettleDistanceFactor;
        speedSquared = speed * speed;
        settleDeceleration = speedSquared / twiceSettleDistance;
        halfFactor = kFloat_0_5_TourVSWindow;
        halfDeceleration = halfFactor * settleDeceleration;
        offscreenX = kFloat_660_TourMap;
        settledX = kTourVsSettledPanelX;
        float* out = &this->panelSplitX;
        for (i = 0; i < 3; i++) {
            speed = this->speed;
            settleTime = this->time;
            panelDelay = startDelays.v[i];
            stopTime = speed / settleDeceleration;
            panelElapsed = settleTime - panelDelay;
            if (panelElapsed < stopTime) {
                settlingX = (settleTime >= panelDelay)
                                ? offscreenX - (speed * panelElapsed - panelElapsed * (halfDeceleration * panelElapsed))
                                : kFloat_660_TourMap;
                out[i] = settlingX;
            } else {
                out[i] = settledX;
            }
        }

        if (this->time - startDelays.v[2] >= this->speed / settleDeceleration) {
            short mode = this->mode;
            settledX = kTourVsSettledPanelX;
            this->edge = settledX;
            this->middle = settledX;
            this->panelSplitX = settledX;
            if (mode != 1) {
                BpIdSet20 ids = tourVsIconGroupNames;
                cursor = ids.id;
                for (i = 0; i < kVsBannerCount; i++, cursor++) {
                    unsigned int id = *cursor;
                    void* layoutGroup;
                    EnsureTourVsBpController();
                    layoutGroup = IdMap_Lookup(Sing_805DC5F0(), this->layoutId);
                    if (layoutGroup == 0) {
                        layoutGroup = 0;
                    } else {
                        layoutGroup = BpObject_GetOrCreateNamedGroup(layoutGroup, (void*)id);
                    }
                    if (layoutGroup)
                        UIGroup_SetEnabled(layoutGroup, 1);
                }

                EnsureTourVsBpController();
                BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), this->layoutId),
                                           tourVsWindowStrings + kVsAnimationScript, 1);
                Sound_PlayByName(g_soundManager, tourVsWindowStrings + kVsIconSound, 0, 0, 0, 0,
                                 kFloat_0_5_TourVSWindow, kFloat_200_TourVsSound, kFloat_1_TourVsSound);
            }
            this->state = kVsAnimating;
        }

        this->time += deltaTime;
        break;
    }
    case kVsAnimating: {
        // Phase 3: poll the BP script; completion starts the default script and
        // produces the single completed=1 notification. Other states leave it zero.
        void* bpObject;

        EnsureTourVsBpController();
        bpObject = IdMap_Lookup(Sing_805DC5F0(), this->layoutId);
        if (BpObject_IsCurrentScriptComplete(bpObject) != 0) {
            EnsureTourVsBpController();
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), this->layoutId),
                                       tourVsWindowStrings + kVsDefaultScript, 1);
            this->state = kVsReady;
            completed = 1;
        }
        break;
    }
    }

    return completed;
}

#pragma push
#pragma use_lmw_stmw on
// Apply the supplied opacity to the nine VS layout groups.
// opacity encodes an unsigned byte scalar (0 transparent .. 255 opaque) in the
// pointer-sized argument register; it is NOT dereferenced. The caller in
// event_conte_script_interpreter.cpp declares unsigned char; UIGroup_SetOpacity stores the word as-is.
// Keep this local void* declaration for the matching ABI; no ownership transfers.
void CTourVSWindow::SetGroupOpacity(void* opacity) {
    unsigned int i;
    BpIdSet36 ids = tourVsMessageWindowOpacityGroupNames;

    for (i = 0; i < kVsFadingGroupCount; i++) {
        unsigned int id = ids.id[i];
        EnsureTourVsBpController();
        void* h = IdMap_Lookup(Sing_805DC5F0(), layoutId);
        if (h == 0) {
            h = 0;
        } else {
            h = BpObject_GetOrCreateNamedGroup(h, (void*)id);
        }
        if (h)
            UIGroup_SetOpacity(h, opacity);
    }
}
#pragma pop

// Start the VS layout exit script.
void TourVSWindow_StartFadeOut(TourVSWindowData* window) {
    void* cbp = Sing_805DC5F0();
    if (cbp == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_TourMap);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), window->layoutId), tourVsWindowStrings + kVsExitScript, 1);
}

// RGB palette for player panels, name trails and borders; alpha remains per-draw state.
enum VsDrawPalette128 {
    kVsPlayerOnePanelRgb128 = 0x0040FF,
    kVsPlayerTwoPanelRgb128 = 0xFF40C0,
    kVsWhiteRgb128 = 0xFFFFFF,
    kVsPlayerOneFarTrailRgb128 = 0xF0F5FF,
    kVsPlayerOneNearTrailRgb128 = 0xCCD9FF,
    kVsPlayerTwoFarTrailRgb128 = 0xFFF0FA,
    kVsPlayerTwoNearTrailRgb128 = 0xFFD9F2,
};

// Apply a packed RGB palette entry to the four panel vertices, retaining alpha and store order.
static inline void SetVsPanelRgb128(Vtx12* quad, unsigned int rgb) {
    quad[0].r = (unsigned char)(rgb >> 16);
    quad[0].g = (unsigned char)(rgb >> 8);
    quad[0].b = (unsigned char)rgb;
    quad[1].r = (unsigned char)(rgb >> 16);
    quad[1].g = (unsigned char)(rgb >> 8);
    quad[1].b = (unsigned char)rgb;
    quad[2].r = (unsigned char)(rgb >> 16);
    quad[2].g = (unsigned char)(rgb >> 8);
    quad[2].b = (unsigned char)rgb;
    quad[3].r = (unsigned char)(rgb >> 16);
    quad[3].g = (unsigned char)(rgb >> 8);
    quad[3].b = (unsigned char)rgb;
}

// Tint four name-layer vertices in their existing store order; alpha is retained.
static inline void SetVsNameRgb128(Vtx16* quad, unsigned int rgb) {
    quad[0].r = (unsigned char)(rgb >> 16);
    quad[0].g = (unsigned char)(rgb >> 8);
    quad[0].b = (unsigned char)rgb;
    quad[1].r = (unsigned char)(rgb >> 16);
    quad[1].g = (unsigned char)(rgb >> 8);
    quad[1].b = (unsigned char)rgb;
    quad[2].r = (unsigned char)(rgb >> 16);
    quad[2].g = (unsigned char)(rgb >> 8);
    quad[2].b = (unsigned char)rgb;
    quad[3].r = (unsigned char)(rgb >> 16);
    quad[3].g = (unsigned char)(rgb >> 8);
    quad[3].b = (unsigned char)rgb;
}

// Draw clipped player banners, colored panels, borders, and flashing overlays.
// Pass map: Expanding draws player A in [0,panelSplitX], its trailing name layers and
// growing borders; Settling/Animating/Ready draw A left of panelSplitX and B right of it,
// then full-width borders and the moving edge gradient. Hidden skips these quads.
// The BP layout draws last in every state. name brightness converges toward 255
// when enabled and 80 when disabled by one third of the remaining gap PER DRAW.
// bannerOrigins.x/y are A's origin; .w/.h are B's origin, not a rectangle size.
// fullClip is the table's default scissor, not a snapshot of caller scissor state.
// Positions are layout coordinates; U/V use 1/256 texture units (256x242 source).
// Volatile float/int bit copies preserve the original loads and must not be folded
// into literals without comparing generated code. All banner pointers are borrowed.
// Draw clipped player panels, name layers, borders and the final BP overlay.
void CTourVSWindow::Draw() {
    // Phase 0: copy immutable defaults into mutable scratch geometry/state tables.
    volatile float textureWidth;
    volatile float textureHeight;
    *(int*)&textureWidth = *(int*)&kFloat_256_TourVsTextureWidth;
    *(int*)&textureHeight = *(int*)&kFloat_242_TourVsTextureHeight;
    ScissorRect bannerClip = *(ScissorRect*)((char*)(&tourVsDrawDefaults.rectB));
    ScissorRect fullClip = *(ScissorRect*)((char*)(&tourVsDrawDefaults.rectA));
    Region bannerOrigins = *(Region*)((char*)(&tourVsDrawDefaults.region));
    Vtx12 coloredQuad[4];
    FmtBlock backdropStages = *(FmtBlock*)((char*)(&tourVsDrawDefaults.fmt1));
    FmtBlock nameStages = *(FmtBlock*)((char*)(&tourVsDrawDefaults.fmt2));
    Vtx16Quad texturedQuad = *(Vtx16Quad*)((char*)(&tourVsDrawDefaults.quad));

    float u = textureWidth * kFloat_OneOver256_TourVsTexcoord;
    float v = textureHeight * kFloat_OneOver256_TourVsTexcoord;
    texturedQuad.v[1].u = u;
    texturedQuad.v[2].v = v;
    texturedQuad.v[3].u = u;
    texturedQuad.v[3].v = v;

    int state = this->state;
    unsigned char windowAlpha = this->opacity;

    switch (state) {
    case kVsExpanding: {
        // Phase 1a: translucent blue panel, growing player-A clip, then its banner.
        Render_SetColoredQuadState();
        int panelAlpha = (windowAlpha * kVsPanelMaxAlpha) / 255;
        SetVsPanelRgb128(coloredQuad, kVsPlayerOnePanelRgb128);
        coloredQuad[0].a = panelAlpha;
        coloredQuad[1].a = panelAlpha;
        coloredQuad[2].a = panelAlpha;
        coloredQuad[3].a = panelAlpha;
        coloredQuad[2].x = coloredQuad[0].x = kFloat_0_0_TourMap;
        coloredQuad[3].x = coloredQuad[1].x = this->panelSplitX;
        coloredQuad[1].y = coloredQuad[0].y = kTourVsQuadTopY;
        coloredQuad[3].y = coloredQuad[2].y = kTourVsQuadBottomY;
        coloredQuad[3].z = coloredQuad[2].z = kFloat_0_0_TourMap;
        coloredQuad[1].z = coloredQuad[0].z = kFloat_0_0_TourMap;
        Render_DrawColoredPrimitive(kVsQuadPrimitive, coloredQuad, 4);

        bannerClip.x = 0;
        bannerClip.w = (int)this->panelSplitX;
        Engine_SetScissorRect(&bannerClip);

        BannerEntry* banner = &((BannerEntry*)this->entries)[this->entryA];
        banner->Draw();

        Engine_SetFlexibleVertexFormat(kVsTexturedVertexFormat);
        texturedQuad.v[0].a = windowAlpha;
        texturedQuad.v[1].a = windowAlpha;
        texturedQuad.v[2].a = windowAlpha;
        texturedQuad.v[3].a = windowAlpha;
        int count = GX_GetPrimitiveCount(4, kVsQuadPrimitive);
        // Name backdrop layers trail the main layer by -64 and -32 layout units.
        Engine_SetTextureStageStateList(0, &backdropStages, 6);

        SetVsNameRgb128(texturedQuad.v, kVsPlayerOneFarTrailRgb128);
        texturedQuad.v[2].x = texturedQuad.v[0].x = bannerOrigins.x - kTourVsBannerOffsetX64;
        texturedQuad.v[3].x = texturedQuad.v[1].x = (bannerOrigins.x + textureWidth) - kTourVsBannerOffsetX64;
        texturedQuad.v[1].y = texturedQuad.v[0].y = bannerOrigins.y;
        texturedQuad.v[3].y = texturedQuad.v[2].y = bannerOrigins.y + textureHeight;
        texturedQuad.v[3].z = texturedQuad.v[2].z = kFloat_0_0_TourMap;
        texturedQuad.v[1].z = texturedQuad.v[0].z = kFloat_0_0_TourMap;
        DrawPrimitiveUP(kVsQuadPrimitive, count, texturedQuad.v, kVsTexturedVertexBytes);

        SetVsNameRgb128(texturedQuad.v, kVsPlayerOneNearTrailRgb128);
        texturedQuad.v[2].x = texturedQuad.v[0].x = bannerOrigins.x - kTourVsBannerOffsetX32;
        texturedQuad.v[3].x = texturedQuad.v[1].x = (bannerOrigins.x + textureWidth) - kTourVsBannerOffsetX32;
        texturedQuad.v[1].y = texturedQuad.v[0].y = bannerOrigins.y;
        texturedQuad.v[3].y = texturedQuad.v[2].y = bannerOrigins.y + textureHeight;
        texturedQuad.v[3].z = texturedQuad.v[2].z = kFloat_0_0_TourMap;
        texturedQuad.v[1].z = texturedQuad.v[0].z = kFloat_0_0_TourMap;
        DrawPrimitiveUP(kVsQuadPrimitive, count, texturedQuad.v, kVsTexturedVertexBytes);

        // Main name layer uses the second six-pair texture-stage preset.
        Engine_SetTextureStageStateList(0, &nameStages, 6);

        SetVsNameRgb128(texturedQuad.v, kVsWhiteRgb128);
        texturedQuad.v[2].x = texturedQuad.v[0].x = bannerOrigins.x;
        texturedQuad.v[3].x = texturedQuad.v[1].x = bannerOrigins.x + textureWidth;
        texturedQuad.v[1].y = texturedQuad.v[0].y = bannerOrigins.y;
        texturedQuad.v[3].y = texturedQuad.v[2].y = bannerOrigins.y + textureHeight;
        texturedQuad.v[3].z = texturedQuad.v[2].z = kFloat_0_0_TourMap;
        texturedQuad.v[1].z = texturedQuad.v[0].z = kFloat_0_0_TourMap;
        DrawPrimitiveUP(kVsQuadPrimitive, count, texturedQuad.v, kVsTexturedVertexBytes);

        // Restore the default clip for white horizontal/vertical border quads.
        Engine_SetScissorRect(&fullClip);
        Render_SetColoredQuadState();

        windowAlpha = (windowAlpha * 255) / 255;
        SetVsPanelRgb128(coloredQuad, kVsWhiteRgb128);
        coloredQuad[0].a = windowAlpha;
        coloredQuad[1].a = windowAlpha;
        coloredQuad[2].a = windowAlpha;
        coloredQuad[3].a = windowAlpha;
        coloredQuad[2].x = coloredQuad[0].x = kFloat_0_0_TourMap;
        coloredQuad[3].x = coloredQuad[1].x = this->panelSplitX;
        coloredQuad[1].y = coloredQuad[0].y = kTourVsOuterTopY;
        coloredQuad[3].y = coloredQuad[2].y = kFloat_51_TourVsInnerTopY;
        coloredQuad[3].z = coloredQuad[2].z = kFloat_0_0_TourMap;
        coloredQuad[1].z = coloredQuad[0].z = kFloat_0_0_TourMap;
        Render_DrawColoredPrimitive(kVsQuadPrimitive, coloredQuad, 4);

        coloredQuad[2].x = coloredQuad[0].x = kFloat_0_0_TourMap;
        coloredQuad[3].x = coloredQuad[1].x = this->panelSplitX;
        coloredQuad[1].y = coloredQuad[0].y = kFloat_289_TourVsInnerBottomY;
        coloredQuad[3].y = coloredQuad[2].y = kTourVsOuterBottomY;
        coloredQuad[3].z = coloredQuad[2].z = kFloat_0_0_TourMap;
        coloredQuad[1].z = coloredQuad[0].z = kFloat_0_0_TourMap;
        Render_DrawColoredPrimitive(kVsQuadPrimitive, coloredQuad, 4);

        coloredQuad[1].y = kTourVsOuterTopY;
        coloredQuad[2].x = coloredQuad[0].x = this->panelSplitX - kTourVsBorderHalfWidth;
        coloredQuad[3].x = coloredQuad[1].x = this->panelSplitX + kTourVsBorderHalfWidth;
        coloredQuad[0].y = kTourVsOuterTopY;
        coloredQuad[3].y = coloredQuad[2].y = kTourVsOuterBottomY;
        coloredQuad[3].z = coloredQuad[2].z = kFloat_0_0_TourMap;
        coloredQuad[1].z = coloredQuad[0].z = kFloat_0_0_TourMap;
        Render_DrawColoredPrimitive(kVsQuadPrimitive, coloredQuad, 4);

        coloredQuad[2].a = 0;
        coloredQuad[0].a = 0;
        coloredQuad[3].a = windowAlpha;
        coloredQuad[1].a = windowAlpha;
        coloredQuad[2].x = coloredQuad[0].x = this->edge;
        coloredQuad[3].x = coloredQuad[1].x = this->panelSplitX - kTourVsBorderHalfWidth;
        coloredQuad[1].y = coloredQuad[0].y = kTourVsOuterTopY;
        coloredQuad[3].y = coloredQuad[2].y = kTourVsOuterBottomY;
        coloredQuad[3].z = coloredQuad[2].z = kFloat_0_0_TourMap;
        coloredQuad[1].z = coloredQuad[0].z = kFloat_0_0_TourMap;
        Render_DrawColoredPrimitive(kVsQuadPrimitive, coloredQuad, 4);
        break;
    }
    case kVsSettling:
    case kVsAnimating:
    case kVsReady: {
        // Phase 1b: blue left and pink right panels, split at this->panelSplitX.
        Render_SetColoredQuadState();
        int panelAlpha = (windowAlpha * kVsPanelMaxAlpha) / 255;
        SetVsPanelRgb128(coloredQuad, kVsPlayerOnePanelRgb128);
        coloredQuad[0].a = panelAlpha;
        coloredQuad[1].a = panelAlpha;
        coloredQuad[2].a = panelAlpha;
        coloredQuad[3].a = panelAlpha;
        coloredQuad[2].x = coloredQuad[0].x = kFloat_0_0_TourMap;
        coloredQuad[3].x = coloredQuad[1].x = this->panelSplitX;
        coloredQuad[1].y = coloredQuad[0].y = kTourVsQuadTopY;
        coloredQuad[3].y = coloredQuad[2].y = kTourVsQuadBottomY;
        coloredQuad[3].z = coloredQuad[2].z = kFloat_0_0_TourMap;
        coloredQuad[1].z = coloredQuad[0].z = kFloat_0_0_TourMap;
        Render_DrawColoredPrimitive(kVsQuadPrimitive, coloredQuad, 4);

        SetVsPanelRgb128(coloredQuad, kVsPlayerTwoPanelRgb128);
        coloredQuad[0].a = panelAlpha;
        coloredQuad[1].a = panelAlpha;
        coloredQuad[2].a = panelAlpha;
        coloredQuad[3].a = panelAlpha;
        coloredQuad[2].x = coloredQuad[0].x = this->panelSplitX;
        coloredQuad[3].x = coloredQuad[1].x = kTourVsQuadRightEdgeX;
        coloredQuad[1].y = coloredQuad[0].y = kTourVsQuadTopY;
        coloredQuad[3].y = coloredQuad[2].y = kTourVsQuadBottomY;
        coloredQuad[3].z = coloredQuad[2].z = kFloat_0_0_TourMap;
        coloredQuad[1].z = coloredQuad[0].z = kFloat_0_0_TourMap;
        Render_DrawColoredPrimitive(kVsQuadPrimitive, coloredQuad, 4);

        bannerClip.x = 0;
        bannerClip.w = (int)this->panelSplitX;
        Engine_SetScissorRect(&bannerClip);

        BannerEntry* banner = &((BannerEntry*)this->entries)[this->entryA];
        banner->Draw();

        Engine_SetFlexibleVertexFormat(kVsTexturedVertexFormat);
        texturedQuad.v[0].a = windowAlpha;
        texturedQuad.v[1].a = windowAlpha;
        texturedQuad.v[2].a = windowAlpha;
        texturedQuad.v[3].a = windowAlpha;
        int count = GX_GetPrimitiveCount(4, kVsQuadPrimitive);
        Engine_SetTextureStageStateList(0, &backdropStages, 6);

        SetVsNameRgb128(texturedQuad.v, kVsPlayerOneFarTrailRgb128);
        texturedQuad.v[2].x = texturedQuad.v[0].x = bannerOrigins.x - kTourVsBannerOffsetX64;
        texturedQuad.v[3].x = texturedQuad.v[1].x = (bannerOrigins.x + textureWidth) - kTourVsBannerOffsetX64;
        texturedQuad.v[1].y = texturedQuad.v[0].y = bannerOrigins.y;
        texturedQuad.v[3].y = texturedQuad.v[2].y = bannerOrigins.y + textureHeight;
        texturedQuad.v[3].z = texturedQuad.v[2].z = kFloat_0_0_TourMap;
        texturedQuad.v[1].z = texturedQuad.v[0].z = kFloat_0_0_TourMap;
        DrawPrimitiveUP(kVsQuadPrimitive, count, texturedQuad.v, kVsTexturedVertexBytes);

        SetVsNameRgb128(texturedQuad.v, kVsPlayerOneNearTrailRgb128);
        texturedQuad.v[2].x = texturedQuad.v[0].x = bannerOrigins.x - kTourVsBannerOffsetX32;
        texturedQuad.v[3].x = texturedQuad.v[1].x = (bannerOrigins.x + textureWidth) - kTourVsBannerOffsetX32;
        texturedQuad.v[1].y = texturedQuad.v[0].y = bannerOrigins.y;
        texturedQuad.v[3].y = texturedQuad.v[2].y = bannerOrigins.y + textureHeight;
        texturedQuad.v[3].z = texturedQuad.v[2].z = kFloat_0_0_TourMap;
        texturedQuad.v[1].z = texturedQuad.v[0].z = kFloat_0_0_TourMap;
        DrawPrimitiveUP(kVsQuadPrimitive, count, texturedQuad.v, kVsTexturedVertexBytes);

        Engine_SetTextureStageStateList(0, &nameStages, 6);

        // Phase 2a: A's main name brightness eases once each draw. It modulates
        // RGB while all four vertex alpha values retain the window opacity.
        unsigned char playerOneBrightness;
        if (this->playerOneNameEnabled) {
            playerOneBrightness = this->alphaA;
            playerOneBrightness = (unsigned char)((float)(unsigned int)playerOneBrightness +
                                                  (float)(255 - playerOneBrightness) / kTourVsBrightnessEaseDivisor);
            this->alphaA = playerOneBrightness;
            texturedQuad.v[0].r = playerOneBrightness;
            texturedQuad.v[0].g = playerOneBrightness;
            texturedQuad.v[0].b = playerOneBrightness;
            texturedQuad.v[1].r = playerOneBrightness;
            texturedQuad.v[1].g = playerOneBrightness;
            texturedQuad.v[1].b = playerOneBrightness;
            texturedQuad.v[2].r = playerOneBrightness;
            texturedQuad.v[2].g = playerOneBrightness;
            texturedQuad.v[2].b = playerOneBrightness;
            texturedQuad.v[3].r = playerOneBrightness;
            texturedQuad.v[3].g = playerOneBrightness;
            texturedQuad.v[3].b = playerOneBrightness;
        } else {
            playerOneBrightness = this->alphaA;
            playerOneBrightness = (unsigned char)((float)(unsigned int)playerOneBrightness +
                                                  (float)(80 - playerOneBrightness) / kTourVsBrightnessEaseDivisor);
            this->alphaA = playerOneBrightness;
            texturedQuad.v[0].r = playerOneBrightness;
            texturedQuad.v[0].g = playerOneBrightness;
            texturedQuad.v[0].b = playerOneBrightness;
            texturedQuad.v[1].r = playerOneBrightness;
            texturedQuad.v[1].g = playerOneBrightness;
            texturedQuad.v[1].b = playerOneBrightness;
            texturedQuad.v[2].r = playerOneBrightness;
            texturedQuad.v[2].g = playerOneBrightness;
            texturedQuad.v[2].b = playerOneBrightness;
            texturedQuad.v[3].r = playerOneBrightness;
            texturedQuad.v[3].g = playerOneBrightness;
            texturedQuad.v[3].b = playerOneBrightness;
        }
        texturedQuad.v[2].x = texturedQuad.v[0].x = bannerOrigins.x;
        texturedQuad.v[3].x = texturedQuad.v[1].x = bannerOrigins.x + textureWidth;
        texturedQuad.v[1].y = texturedQuad.v[0].y = bannerOrigins.y;
        texturedQuad.v[3].y = texturedQuad.v[2].y = bannerOrigins.y + textureHeight;
        texturedQuad.v[3].z = texturedQuad.v[2].z = kFloat_0_0_TourMap;
        texturedQuad.v[1].z = texturedQuad.v[0].z = kFloat_0_0_TourMap;
        DrawPrimitiveUP(kVsQuadPrimitive, count, texturedQuad.v, kVsTexturedVertexBytes);

        // Phase 2b: mirror the banner/name sequence for B in the right-hand clip;
        // its two backdrop layers lead the main layer by +64 and +32 units.
        bannerClip.x = (int)this->panelSplitX;
        bannerClip.w = kVsViewportWidth;
        Engine_SetScissorRect(&bannerClip);

        banner = &((BannerEntry*)this->entries)[this->entryB];
        banner->Draw();

        Engine_SetFlexibleVertexFormat(kVsTexturedVertexFormat);
        texturedQuad.v[0].a = windowAlpha;
        texturedQuad.v[1].a = windowAlpha;
        texturedQuad.v[2].a = windowAlpha;
        texturedQuad.v[3].a = windowAlpha;
        count = GX_GetPrimitiveCount(4, kVsQuadPrimitive);
        Engine_SetTextureStageStateList(0, &backdropStages, 6);

        SetVsNameRgb128(texturedQuad.v, kVsPlayerTwoFarTrailRgb128);
        texturedQuad.v[2].x = texturedQuad.v[0].x = bannerOrigins.w + kTourVsBannerOffsetX64;
        texturedQuad.v[3].x = texturedQuad.v[1].x = (bannerOrigins.w + textureWidth) + kTourVsBannerOffsetX64;
        texturedQuad.v[1].y = texturedQuad.v[0].y = bannerOrigins.h;
        texturedQuad.v[3].y = texturedQuad.v[2].y = bannerOrigins.h + textureHeight;
        texturedQuad.v[3].z = texturedQuad.v[2].z = kFloat_0_0_TourMap;
        texturedQuad.v[1].z = texturedQuad.v[0].z = kFloat_0_0_TourMap;
        DrawPrimitiveUP(kVsQuadPrimitive, count, texturedQuad.v, kVsTexturedVertexBytes);

        SetVsNameRgb128(texturedQuad.v, kVsPlayerTwoNearTrailRgb128);
        texturedQuad.v[2].x = texturedQuad.v[0].x = bannerOrigins.w + kTourVsBannerOffsetX32;
        texturedQuad.v[3].x = texturedQuad.v[1].x = (bannerOrigins.w + textureWidth) + kTourVsBannerOffsetX32;
        texturedQuad.v[1].y = texturedQuad.v[0].y = bannerOrigins.h;
        texturedQuad.v[3].y = texturedQuad.v[2].y = bannerOrigins.h + textureHeight;
        texturedQuad.v[3].z = texturedQuad.v[2].z = kFloat_0_0_TourMap;
        texturedQuad.v[1].z = texturedQuad.v[0].z = kFloat_0_0_TourMap;
        DrawPrimitiveUP(kVsQuadPrimitive, count, texturedQuad.v, kVsTexturedVertexBytes);

        Engine_SetTextureStageStateList(0, &nameStages, 6);

        // Independent per-draw brightness cache for B's main name layer.
        unsigned char playerTwoBrightness;
        if (this->playerTwoNameEnabled) {
            playerTwoBrightness = this->alphaB;
            playerTwoBrightness = (unsigned char)((float)(unsigned int)playerTwoBrightness +
                                                  (float)(255 - playerTwoBrightness) / kTourVsBrightnessEaseDivisor);
            this->alphaB = playerTwoBrightness;
            texturedQuad.v[0].r = playerTwoBrightness;
            texturedQuad.v[0].g = playerTwoBrightness;
            texturedQuad.v[0].b = playerTwoBrightness;
            texturedQuad.v[1].r = playerTwoBrightness;
            texturedQuad.v[1].g = playerTwoBrightness;
            texturedQuad.v[1].b = playerTwoBrightness;
            texturedQuad.v[2].r = playerTwoBrightness;
            texturedQuad.v[2].g = playerTwoBrightness;
            texturedQuad.v[2].b = playerTwoBrightness;
            texturedQuad.v[3].r = playerTwoBrightness;
            texturedQuad.v[3].g = playerTwoBrightness;
            texturedQuad.v[3].b = playerTwoBrightness;
        } else {
            playerTwoBrightness = this->alphaB;
            playerTwoBrightness = (unsigned char)((float)(unsigned int)playerTwoBrightness +
                                                  (float)(80 - playerTwoBrightness) / kTourVsBrightnessEaseDivisor);
            this->alphaB = playerTwoBrightness;
            texturedQuad.v[0].r = playerTwoBrightness;
            texturedQuad.v[0].g = playerTwoBrightness;
            texturedQuad.v[0].b = playerTwoBrightness;
            texturedQuad.v[1].r = playerTwoBrightness;
            texturedQuad.v[1].g = playerTwoBrightness;
            texturedQuad.v[1].b = playerTwoBrightness;
            texturedQuad.v[2].r = playerTwoBrightness;
            texturedQuad.v[2].g = playerTwoBrightness;
            texturedQuad.v[2].b = playerTwoBrightness;
            texturedQuad.v[3].r = playerTwoBrightness;
            texturedQuad.v[3].g = playerTwoBrightness;
            texturedQuad.v[3].b = playerTwoBrightness;
        }
        texturedQuad.v[2].x = texturedQuad.v[0].x = bannerOrigins.w;
        texturedQuad.v[3].x = texturedQuad.v[1].x = bannerOrigins.w + textureWidth;
        texturedQuad.v[1].y = texturedQuad.v[0].y = bannerOrigins.h;
        texturedQuad.v[3].y = texturedQuad.v[2].y = bannerOrigins.h + textureHeight;
        texturedQuad.v[3].z = texturedQuad.v[2].z = kFloat_0_0_TourMap;
        texturedQuad.v[1].z = texturedQuad.v[0].z = kFloat_0_0_TourMap;
        DrawPrimitiveUP(kVsQuadPrimitive, count, texturedQuad.v, kVsTexturedVertexBytes);

        Engine_SetScissorRect(&fullClip);
        Render_SetColoredQuadState();

        windowAlpha = (windowAlpha * 255) / 255;
        SetVsPanelRgb128(coloredQuad, kVsWhiteRgb128);
        coloredQuad[0].a = windowAlpha;
        coloredQuad[1].a = windowAlpha;
        coloredQuad[2].a = windowAlpha;
        coloredQuad[3].a = windowAlpha;
        coloredQuad[2].x = coloredQuad[0].x = kFloat_0_0_TourMap;
        coloredQuad[3].x = coloredQuad[1].x = kTourVsQuadRightEdgeX;
        coloredQuad[1].y = coloredQuad[0].y = kTourVsOuterTopY;
        coloredQuad[3].y = coloredQuad[2].y = kFloat_51_TourVsInnerTopY;
        coloredQuad[3].z = coloredQuad[2].z = kFloat_0_0_TourMap;
        coloredQuad[1].z = coloredQuad[0].z = kFloat_0_0_TourMap;
        Render_DrawColoredPrimitive(kVsQuadPrimitive, coloredQuad, 4);

        coloredQuad[2].x = coloredQuad[0].x = kFloat_0_0_TourMap;
        coloredQuad[3].x = coloredQuad[1].x = kTourVsQuadRightEdgeX;
        coloredQuad[1].y = coloredQuad[0].y = kFloat_289_TourVsInnerBottomY;
        coloredQuad[3].y = coloredQuad[2].y = kTourVsOuterBottomY;
        coloredQuad[3].z = coloredQuad[2].z = kFloat_0_0_TourMap;
        coloredQuad[1].z = coloredQuad[0].z = kFloat_0_0_TourMap;
        Render_DrawColoredPrimitive(kVsQuadPrimitive, coloredQuad, 4);

        coloredQuad[1].y = kTourVsOuterTopY;
        coloredQuad[2].x = coloredQuad[0].x = this->panelSplitX - kTourVsBorderHalfWidth;
        coloredQuad[3].x = coloredQuad[1].x = this->panelSplitX + kTourVsBorderHalfWidth;
        coloredQuad[0].y = kTourVsOuterTopY;
        coloredQuad[3].y = coloredQuad[2].y = kTourVsOuterBottomY;
        coloredQuad[3].z = coloredQuad[2].z = kFloat_0_0_TourMap;
        coloredQuad[1].z = coloredQuad[0].z = kFloat_0_0_TourMap;
        Render_DrawColoredPrimitive(kVsQuadPrimitive, coloredQuad, 4);

        coloredQuad[2].a = windowAlpha;
        coloredQuad[0].a = windowAlpha;
        coloredQuad[3].a = 0;
        coloredQuad[1].a = 0;
        // Phase 3: edge gradient between the settled divider and animated edge.
        coloredQuad[3].x = this->edge;
        coloredQuad[2].x = coloredQuad[0].x = this->panelSplitX + kTourVsBorderHalfWidth;
        coloredQuad[1].x = this->edge;
        coloredQuad[1].y = coloredQuad[0].y = kTourVsOuterTopY;
        coloredQuad[3].y = coloredQuad[2].y = kTourVsOuterBottomY;
        coloredQuad[3].z = coloredQuad[2].z = kFloat_0_0_TourMap;
        coloredQuad[1].z = coloredQuad[0].z = kFloat_0_0_TourMap;
        Render_DrawColoredPrimitive(kVsQuadPrimitive, coloredQuad, 4);
        break;
    }

    case kVsHidden:
        break;
    }

    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)&s_bpCtrl[0])->__ct();
            __register_global_object(&s_bpCtrl[0], (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_TourMap);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl[0];
    }
    // Phase 4: scripted BP icons/characters/flash draw over the manual geometry.
    void* item = (void*)IdMap_Lookup(Sing_805DC5F0(), this->layoutId);
    BpObject_Draw(item);
}

} // extern "C"

#undef GAME_NSTRING_INLINE
#undef GAME_NSTRING_RANGE_CONSTRUCTOR

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462598", _ctors_u802F61E4_0, TourVsWindow_InitAngleConstant);
#pragma pop
