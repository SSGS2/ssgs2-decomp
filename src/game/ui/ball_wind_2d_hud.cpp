#include <decomp/static_init.h>
extern "C" char s_dolfiniNumberDrawDtorRegistration[];
extern "C" char s_dolfiniReloadingDtorRegistration[];
extern "C" char s_roundDolDrawBpDestructorRecord[];
#include "classes/PlayerRecordOwner8030E168.h"
#define AsPlayerRecordOwner8030E168(p) ((PlayerRecordOwner8030E168*)(p))
#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CBallWind2d.h"

#define AsCBallWind2d(p) ((CBallWind2d*)(p))
// Translation unit 0x8030D804-0x8030EE64 (27 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct VTable8030D804 {
    char pad[0x68];
    int (*fn68)(void*);
    int (*fn6c)(void*);
    int (*fn70)(void*);
};

// Ball/wind layout map (asset labels for the auxiliary elements are unresolved):
// 2 = interactive selector; selectedElementIndex = selected table-driven image;
// 4/5 = enabled with selector; 1/3 = separately drawn, horizontally centered indicators.
// 0,22,23,24,25 = independent auxiliary visibility; 26/27 share visibility;
// 25/26 share brightness; 27 follows selectionActive brightness. 28/29 are mutually
// exclusive overlays and also require ballControlsVisible. Setters change cached
// words; vfunc_26 applies them only after layoutReady. Selection states 0/1/2
// are forwarded unchanged to BOTH value and feedback state; labels are unresolved.
enum BallWindUi {
    kBallWindSelector = 2, kBallWindIndicatorA = 1, kBallWindIndicatorB = 3,
    kBallWindElementCount = 30, kBallWindDimColor = -136, kBallWindBrightColor = -1,
    kSelectorCompanion4 = 4, kSelectorCompanion5 = 5, kSelectorPlayerFlag = 5,
    // Auxiliary asset labels are unverified; numeric suffixes are intentional.
    kAuxElement0 = 0, kAuxElement22 = 22, kAuxElement23 = 23, kAuxElement24 = 24,
    kAuxElement25 = 25, kAuxElement26 = 26, kAuxElement27 = 27,
    kBallControlOverlay28 = 28, kBallControlOverlay29 = 29
};
struct U8030D804Obj {
    char layoutBase[0x5c];
    int selectedElementIndex;
    int selectionActive;
    int selectionState;
    int ballControlsVisible;
    int element0Visible;
    int elements26And27Visible;
    int element22Visible;
    int element25Visible;
    int elements25And26Bright;
    int element23Visible;
    int element24Visible;
    int element28Visible;
    int element29Visible;
    int element3DrawnSeparately;
    int element1DrawnSeparately;
    float indicatorCenterX;
    int indicatorColor;
    int layoutReady;
    VTable8030D804* vtable;
};

extern "C" {

extern float kFloat_6_2831855;
extern float kFloat_1_IngameStatus;
extern float s_ballWind2dFullTurn;
extern int ballWindSelectionElementIndices[17];
extern int ballWindScissorRectangle[4];
extern "C" int Layout_Update(void*);
extern "C" int Layout_CollectActivated(void*, void*, int);
extern "C" void Layout_DrawElementWithOpacity(void*, int, float);
extern "C" void* __nwa__FUl(unsigned int);
extern "C" void appFreeUni__FPv(void*);
extern "C" void* __nw__FUl(unsigned long);
extern "C" void __dla__FPv(void*);
extern "C" void Sound_PlayByName(void*, char*, int, int, int, int, float, float, float);
extern float kFloat_0_5_BallWind2d;
extern float kBallWindClickSoundRange; // 200.0f: click-sound distance parameter.
extern int g_soundManager;
struct BallWindStringPool {
    char layoutFile[27];
    char vectorBoundsError[30];
    char outOfRangeType[55];
    char clickSound[14];
    char padding[2];
};
extern BallWindStringPool golfBallWindLayoutStrings;
extern "C" void Engine_GetScissorRect(void*);
extern "C" void Engine_SetScissorRect(void*);
extern "C" void BpObject_Draw(void*);
extern "C" void Layout_DrawWithOpacity(void*, float);
extern "C" void Layout_SetElementPlayerFlagsAndState(void*, int, int);
extern "C" void Layout_SetElementPlayerFlag(void*, int, int, int);
extern "C" void Layout_SetElementValueState(void*, int, int);
extern "C" void Layout_SetElementFeedbackState(void*, int, int);
extern "C" void Layout_SetElementEnabled(void*, int, int);
extern "C" void Layout_SetElementColor(void*, int, int);
extern "C" void Layout_GetElementSize(void*, int, float*);
extern "C" void Layout_GetElementPosition(void*, int, float*);
extern "C" void Layout_SetElementPosition(void*, int, float*);

struct CBallWind2dBase { char layoutBase[0xA4]; };
class CBallWind2dV : public CBallWind2dBase {
public:
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void v16();
    virtual void v17();
    virtual void v18();
    virtual void v19();
    virtual void v20();
    virtual void v21();
    virtual void v22();
    virtual void v23();
    virtual int vfunc68();
    virtual int vfunc6C();
    virtual int vfunc70();
};
// Update layout state and combine the three derived update results.
int CBallWind2d_vfunc_3(U8030D804Obj* self) {
    int updated = Layout_Update(self);
    int result = updated & 1;
    CBallWind2dV* obj = (CBallWind2dV*)self;
    result &= obj->vfunc68();
    result &= obj->vfunc6C();
    result &= obj->vfunc70();
    return result;
}

// Return whether the ball-control elements are visible.
int CBallWind2d_vfunc_10(U8030D804Obj* self) {
    return self->ballControlsVisible;
}

// Accept the layout readiness query.
int CBallWind2d_vfunc_24() {
    return 1;
}

// Map a selection to its layout element without changing the original table bounds.
int CBallWind2d_vfunc_5(U8030D804Obj* self, int val) {
    // Preserve the original 17-entry copy and accepted range (including index 17).
    int elementIndices[18];
    int* s = ballWindSelectionElementIndices - 1;
    int* d = elementIndices - 1;
    register int i = 8;
    do {
        d[1] = s[1];
        d[2] = s[2];
        s += 2;
        d += 2;
    } while (--i);
    d[1] = s[1];
    if ((unsigned int)val > 0x11 || val < 0) {
        return 0;
    }
    AsCBallWind2d(self)->selectedElementIndex = elementIndices[val];
    return 1;
}

// Draw optional indicators, clip the supplied object, then restore scissoring and draw the layout.
int CBallWind2d_vfunc_4(U8030D804Obj* self, void* arg) {
    if (self->element3DrawnSeparately) {
        Layout_DrawElementWithOpacity(self, kBallWindIndicatorB, kFloat_1_IngameStatus);
    }
    if (self->element1DrawnSeparately) {
        Layout_DrawElementWithOpacity(self, kBallWindIndicatorA, kFloat_1_IngameStatus);
    }
    int savedScissor[4];
    Engine_GetScissorRect(savedScissor);
    int ballScissor[4];
    int* p = ballWindScissorRectangle - 1;
    ballScissor[0] = *++p;
    ballScissor[1] = p[1];
    ballScissor[2] = p[2];
    ballScissor[3] = p[3];
    Engine_SetScissorRect(ballScissor);
    BpObject_Draw(arg);
    Engine_SetScissorRect(savedScissor);
    Layout_DrawWithOpacity(self, kFloat_1_IngameStatus);
    return 1;
}

// Set the selector active flag.
int CBallWind2d_vfunc_6(U8030D804Obj* self, int val) {
    self->selectionActive = val;
    return 1;
}

// Set the selector value and feedback state.
int CBallWind2d_vfunc_7(U8030D804Obj* self, int val) {
    self->selectionState = val;
    return 1;
}

// Set ball-control visibility.
int CBallWind2d_vfunc_9(U8030D804Obj* self, int val) {
    self->ballControlsVisible = val;
    return 1;
}

// Set element 0 visibility.
int CBallWind2d_SetElement0Visible(U8030D804Obj* self, int val) {
    self->element0Visible = val;
    return 1;
}

// Set the visibility of elements 26 and 27.
int CBallWind2d_SetElements26And27Visible(U8030D804Obj* self, int val) {
    self->elements26And27Visible = val;
    return 1;
}

// Set elements 22 and 25 visibility together.
int CBallWind2d_SetElements22And25Visible(U8030D804Obj* self, int val) {
    self->element22Visible = val;
    self->element25Visible = val;
    return 1;
}

// Set element 22 visibility.
int CBallWind2d_SetElement22Visible(U8030D804Obj* self, int val) {
    self->element22Visible = val;
    return 1;
}

// Set the bright/dim state for elements 25 and 26.
int CBallWind2d_SetElements25And26Bright(U8030D804Obj* self, int val) {
    self->elements25And26Bright = val;
    return 1;
}

// Set element 28 visibility, hiding mutually exclusive element 29.
int CBallWind2d_SetElement28Visible(U8030D804Obj* self, int val) {
    self->element28Visible = val;
    if (val != 0) {
        self->element29Visible = 0;
    }
    return 1;
}

// Set element 29 visibility, hiding mutually exclusive element 28.
int CBallWind2d_SetBallControlOverlay29Visibility(U8030D804Obj* self, int val) {
    self->element29Visible = val;
    if (val != 0) {
        self->element28Visible = 0;
    }
    return 1;
}

// Set element 23 visibility.
int CBallWind2d_SetElement23Visible(U8030D804Obj* self, int val) {
    self->element23Visible = val;
    return 1;
}

// Set element 24 visibility.
int CBallWind2d_SetElement24Visible(U8030D804Obj* self, int val) {
    self->element24Visible = val;
    return 1;
}

// Set separate drawing of indicator element 3.
int CBallWind2d_vfunc_20(U8030D804Obj* self, int val) {
    self->element3DrawnSeparately = val;
    return 1;
}

// Set separate drawing of indicator element 1.
int CBallWind2d_vfunc_21(U8030D804Obj* self, int val) {
    self->element1DrawnSeparately = val;
    return 1;
}

// Set the horizontal center used by both indicator elements.
int CBallWind2d_vfunc_22(U8030D804Obj* self, float val) {
    self->indicatorCenterX = val;
    return 1;
}

// Set the color applied to both indicator elements.
int CBallWind2d_vfunc_23(U8030D804Obj* self, int val) {
    self->indicatorColor = val;
    return 1;
}

struct ActivatedItemsCleanup {
    int* items;
    // Free the activated-element array returned by Layout_CollectActivated.
    ~ActivatedItemsCleanup() {
        if (items[0]) {
            items[1] = 0;
            appFreeUni__FPv((void*)items[0]);
        }
    }
};

// Return whether activated layout elements include the ball selector (element 2).
int CBallWind2d_vfunc_8(U8030D804Obj* self) {
    int items[3] = {0, 0, 0};
    ActivatedItemsCleanup cleanup = { items };
    if (!Layout_CollectActivated(self, items, 0)) {
        return 0;
    }
    int* data = (int*)items[0];
    unsigned int size = items[1];
    int* p = data;
    for (unsigned int i = 0; i < size; i++, p++) {
        if (*p == kBallWindSelector) {
            return 1;
        }
    }
    return 0;
}

// Play the UI click sound for each activated ball-selector element.
int CBallWind2d_vfunc_25(U8030D804Obj* self) {
    int items[3] = {0, 0, 0};
    ActivatedItemsCleanup cleanup = { items };
    if (!Layout_CollectActivated(self, items, 0)) {
        return 1;
    }
    int* data = (int*)items[0];
    unsigned int size = items[1];
    int* p = data;
    for (unsigned int i = 0; i < size; i++, p++) {
        if (*p == kBallWindSelector) {
            Sound_PlayByName((void*)g_soundManager, golfBallWindLayoutStrings.clickSound, 0, 0, 0, 0, kFloat_0_5_BallWind2d, kBallWindClickSoundRange, kFloat_1_IngameStatus);
        }
    }
    return 1;
}

// Apply visibility, selection, color, and centered indicator positions to the ball/wind layout.
int CBallWind2d_vfunc_26(U8030D804Obj* self) {
    // Phase 1: reset all element player states, then apply selector interaction state.
    if (self->layoutReady == 0) return 0;
    for (int i = 0; i < kBallWindElementCount; i++) Layout_SetElementPlayerFlagsAndState(self, i, 0);
    if (self->selectionActive) Layout_SetElementPlayerFlag(self, kBallWindSelector, 1, kSelectorPlayerFlag);
    else Layout_SetElementPlayerFlag(self, kBallWindSelector, 0, kSelectorPlayerFlag);
    switch (self->selectionState) {
    case 0: Layout_SetElementValueState(self, kBallWindSelector, 0); Layout_SetElementFeedbackState(self, kBallWindSelector, 0); break;
    case 1: Layout_SetElementValueState(self, kBallWindSelector, 1); Layout_SetElementFeedbackState(self, kBallWindSelector, 1); break;
    case 2: Layout_SetElementValueState(self, kBallWindSelector, 2); Layout_SetElementFeedbackState(self, kBallWindSelector, 2); break;
    }
    // Phase 2: enable the cached visibility groups; see the map at U8030D804Obj.
    if (self->ballControlsVisible) {
        Layout_SetElementEnabled(self, kSelectorCompanion4, 1); Layout_SetElementEnabled(self, kSelectorCompanion5, 1);
        Layout_SetElementEnabled(self, kBallWindSelector, 1); Layout_SetElementEnabled(self, self->selectedElementIndex, 1);
    }
    if (self->element0Visible) Layout_SetElementEnabled(self, kAuxElement0, 1);
    if (self->elements26And27Visible) { Layout_SetElementEnabled(self, kAuxElement27, 1); Layout_SetElementEnabled(self, kAuxElement26, 1); }
    if (self->element22Visible) Layout_SetElementEnabled(self, kAuxElement22, 1);
    if (self->element25Visible) Layout_SetElementEnabled(self, kAuxElement25, 1);
    if (self->element23Visible) Layout_SetElementEnabled(self, kAuxElement23, 1);
    if (self->element24Visible) Layout_SetElementEnabled(self, kAuxElement24, 1);
    if (self->element28Visible && self->ballControlsVisible) Layout_SetElementEnabled(self, kBallControlOverlay28, 1);
    if (self->element29Visible && self->ballControlsVisible) Layout_SetElementEnabled(self, kBallControlOverlay29, 1);
    // Phase 3: apply brightness/indicator color, then convert center X to left edge.
    if (self->selectionActive == 0) Layout_SetElementColor(self, kAuxElement27, kBallWindDimColor);
    else Layout_SetElementColor(self, kAuxElement27, kBallWindBrightColor);
    if (self->elements25And26Bright == 0) {
        Layout_SetElementColor(self, kAuxElement25, kBallWindDimColor);
        Layout_SetElementColor(self, kAuxElement26, kBallWindDimColor);
    } else {
        Layout_SetElementColor(self, kAuxElement25, kBallWindBrightColor);
        Layout_SetElementColor(self, kAuxElement26, kBallWindBrightColor);
    }
    Layout_SetElementColor(self, kBallWindIndicatorB, self->indicatorColor);
    Layout_SetElementColor(self, kBallWindIndicatorA, self->indicatorColor);
    float f[2];
    Layout_GetElementSize(self, kBallWindIndicatorB, f);
    float old = f[0];
    Layout_GetElementPosition(self, kBallWindIndicatorB, f);
    f[0] = self->indicatorCenterX - kFloat_0_5_BallWind2d * old;
    Layout_SetElementPosition(self, kBallWindIndicatorB, f);
    Layout_GetElementSize(self, kBallWindIndicatorA, f);
    old = f[0];
    Layout_GetElementPosition(self, kBallWindIndicatorA, f);
    f[0] = self->indicatorCenterX - kFloat_0_5_BallWind2d * old;
    Layout_SetElementPosition(self, kBallWindIndicatorA, f);
    return 1;
}

// Initialize the ball/wind full-turn angle cache.
void BallWind2d_InitFullTurnConstant() {
    s_ballWind2dFullTurn = kFloat_6_2831855;
}

extern "C" void SaveData_InitPlayerRecord(void*);
extern "C" void CNowReloading_Init(void*);
extern "C" void CNowReloading_dtor();
extern "C" int LoadingIndicator_IsAbsentOrState3(void*);
extern "C" void LoadingIndicator_CreatePlayer(void*);
extern "C" void CRoundDol3D_Ctor(void*);
extern "C" void Layout_Init(void*, int);
extern "C" void PointerVector_InitReserved(void*);
extern "C" void PointerVector_DestroyStorage();
extern "C" void* CFontNumDraw_CreateInVector(void*);
extern "C" void CBpTex_Init(void*);
extern "C" void __dl__FPv(void*);

extern signed char s_nowReloadingInitialized;
extern char s_nowReloading[8];
extern char s_dolfiniReloadingDtorRegistration[];
extern signed char s_pointerVectorInitialized;
extern char s_fontNumDrawRegistry[];
extern char s_dolfiniNumberDrawDtorRegistration[];

// Allocate session-owned helpers through the game's operator-new entry point.
inline void* operator new(size_t size) { return __nw__FUl(size); }
void operator delete(void*) throw();

// Construct the owned 3D session renderer in its fixed-size storage.
struct S614 { char buf[0x614]; S614() { CRoundDol3D_Ctor(this); } };
// Construct the owned layout with the existing initialization mode 5.
struct S5c { char buf[0x5c]; S5c() { Layout_Init(this, 5); } };
// Construct one owned BP texture wrapper.
struct S28 { char buf[0x28]; S28() { CBpTex_Init(this); } };
struct Copy25c { int data[151]; };
struct Copy70 { int data[28]; };
struct Copy1a0 { unsigned short data[208]; };
struct Copy33c { unsigned short data[414]; };
struct Copy40 { int data[16]; };
struct Copy18 { unsigned char data[24]; };
struct Copy8 { unsigned char data[8]; };
struct Copy14 { int data[5]; };
// Record copies retain the original word/halfword widths, including opaque save fields.
// Source schema: +25C..2CB course records; +2CC..46B eight 0x34-byte
// character equipment rows (24 part indices and a hairstyle byte); +46C..4C3
// eleven 8-byte caddie records. ShopSaveEquipment (shop_product_selection_and_purchase.cpp) and the snapshot
// writer (in_game_dialog_and_snapshot_copy.cpp) establish these regions. Coefficient/statistic units inside
// profile, the six following attribute bytes, +4CA and tail remain unresolved.

// Session copy map: source [0,0x4D2) is copied to self+0x58. The six bytes at
// source+0x4C4 become self+0x51C; +0x4CA becomes +0x522. Source +0x4D2..4D4
// becomes self+0x52A..52C, then aligned words/tail follow. Opaque names retain
// SOURCE byte offsets, not destination addresses or independent owners.
// These are value blobs; only the separately allocated UI/render pointers are owned.
struct DolfiniSessionRecordView {
    char pad0[0x58];
    Copy25c profile;
    Copy70 courseRecords;
    Copy1a0 characterEquipment;
    Copy40 caddieRecordsFirstEight;
    Copy18 caddieRecordsLastThree;
    unsigned char attributes[6];
    Copy8 opaqueProfileData4ca;
    unsigned char field52a;
    unsigned char field52b;
    unsigned char field52c;
    char pad52d[3];
    Copy14 words;
    Copy33c tail;
    int field880;
    void* field884;
    char pad888[0x10];
    int field898;
};

struct Source8030E168 {
    Copy25c profile;
    Copy70 courseRecords;
    Copy1a0 characterEquipment;
    Copy40 caddieRecordsFirstEight;
    Copy18 caddieRecordsLastThree;
    unsigned char attributes[6];
    Copy8 opaqueProfileData4ca;
    unsigned char a, b, c;
    char pad4d5[3];
    Copy14 words;
    Copy33c tail;
};

// Copy the player record and create the session renderer, layout, text registry entry, and textures.
// self is session storage despite its legacy ball/wind-shaped signature: only
// DolfiniSessionRecordView/PlayerRecordOwner8030E168 views apply here. arg is borrowed and copied;
// it must include all Source8030E168 fields, including the words and tail.
U8030D804Obj* DolfiniSession_Ctor(U8030D804Obj* self, Source8030E168* arg) {
    // Phase 1: establish a default profile and ensure the loading indicator exists.
    SaveData_InitPlayerRecord(&((DolfiniSessionRecordView*)self)->profile);
    int initialManager = ((int&)Sing_805DC610());
    AsPlayerRecordOwner8030E168(self)->resourcesLoaded = 0;
    if (initialManager == 0) {
        if (s_nowReloadingInitialized == 0) {
            CNowReloading_Init(s_nowReloading);
            __register_global_object(s_nowReloading, (void*)CNowReloading_dtor, s_dolfiniReloadingDtorRegistration);
            s_nowReloadingInitialized = 1;
        }
        ((int&)Sing_805DC610()) = (int)s_nowReloading;
    }
    int gv = ((int&)Sing_805DC610());
    if (gv != 0) {
        if (LoadingIndicator_IsAbsentOrState3((void*)gv)) {
            LoadingIndicator_CreatePlayer((void*)gv);
        }
    }

    // Phase 2: one coherent input view supplies all profile/equipment fields.
    // No ball/wind fields are accessed. Shared receiver typing is kept as-is
    // separately; preserve its ABI while using the session view in this body.
    // Copy the packed input record (map above); leave source ownership alone.
    // Field-derived byte address prevents CodeWarrior retaining this address across loading calls.
    *(Copy25c*)((char*)self + (int)&((DolfiniSessionRecordView*)0)->profile) = arg->profile;
    ((DolfiniSessionRecordView*)self)->courseRecords = arg->courseRecords;
    ((DolfiniSessionRecordView*)self)->characterEquipment = arg->characterEquipment;

    // Caddie table spans 11 eight-byte records at source+46C..4C3. Keep the
    // 8+3 copy groups: they are one table, not two kinds of caddie data.
    ((DolfiniSessionRecordView*)self)->caddieRecordsFirstEight = arg->caddieRecordsFirstEight;

    ((DolfiniSessionRecordView*)self)->caddieRecordsLastThree = arg->caddieRecordsLastThree;
    // Six packed profile attributes are copied verbatim; do not widen these byte stores.
    AsPlayerRecordOwner8030E168(self)->profileAttribute0 = arg->attributes[0];
    AsPlayerRecordOwner8030E168(self)->profileAttribute1 = arg->attributes[1];
    AsPlayerRecordOwner8030E168(self)->profileAttribute2 = arg->attributes[2];
    AsPlayerRecordOwner8030E168(self)->profileAttribute3 = arg->attributes[3];
    AsPlayerRecordOwner8030E168(self)->profileAttribute4 = arg->attributes[4];
    AsPlayerRecordOwner8030E168(self)->profileAttribute5 = arg->attributes[5];
    ((DolfiniSessionRecordView*)self)->opaqueProfileData4ca = arg->opaqueProfileData4ca;
    ((DolfiniSessionRecordView*)self)->field52a = arg->a;
    ((DolfiniSessionRecordView*)self)->field52b = arg->b;
    ((DolfiniSessionRecordView*)self)->field52c = arg->c;

    ((DolfiniSessionRecordView*)self)->words = arg->words;

    ((DolfiniSessionRecordView*)self)->tail = arg->tail;

    // Phase 3: mark registry IDs absent before allocating independently owned UI helpers.
    AsPlayerRecordOwner8030E168(self)->bpEntryId0 = -1;
    AsPlayerRecordOwner8030E168(self)->bpEntryId5 = -1;
    AsPlayerRecordOwner8030E168(self)->bpEntryId6 = -1;
    AsPlayerRecordOwner8030E168(self)->bpEntryId1 = -1;
    AsPlayerRecordOwner8030E168(self)->bpEntryId2 = -1;
    AsPlayerRecordOwner8030E168(self)->bpEntryId3 = -1;
    AsPlayerRecordOwner8030E168(self)->bpEntryId4 = -1;
    AsPlayerRecordOwner8030E168(self)->bpEntryId7 = -1;
    AsPlayerRecordOwner8030E168(self)->speedLevel = 0;
    AsPlayerRecordOwner8030E168(self)->peakSpeedLevel = 0;
    AsPlayerRecordOwner8030E168(self)->preloadAnimationIndex = 0;
    AsPlayerRecordOwner8030E168(self)->rewardKind = 0;
    (*(int*)&AsPlayerRecordOwner8030E168(self)->state) = 0;

    AsPlayerRecordOwner8030E168(self)->roundDol3D = new S614();
    S5c* second = new S5c();
    int initialController = ((int&)Sing_805DC830());
    AsPlayerRecordOwner8030E168(self)->layout = second;
    (*(int*)&AsPlayerRecordOwner8030E168(self)->dialog) = 0;
    if (initialController == 0) {
        if (s_pointerVectorInitialized == 0) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, s_dolfiniNumberDrawDtorRegistration);
            s_pointerVectorInitialized = 1;
        }
        ((int&)Sing_805DC830()) = (int)s_fontNumDrawRegistry;
    }
    AsPlayerRecordOwner8030E168(self)->numberDraw = CFontNumDraw_CreateInVector((void*)((int&)Sing_805DC830()));

    AsPlayerRecordOwner8030E168(self)->texture0 = new S28();
    AsPlayerRecordOwner8030E168(self)->texture1 = new S28();

    AsPlayerRecordOwner8030E168(self)->messageResource = 0;
    (*(int*)&AsPlayerRecordOwner8030E168(self)->textFactory) = 0;
    AsPlayerRecordOwner8030E168(self)->messageDraw = 0;
    AsPlayerRecordOwner8030E168(self)->rewardTable = 0;
    AsPlayerRecordOwner8030E168(self)->galleryImageTable = 0;
    AsPlayerRecordOwner8030E168(self)->prizeGalleryTable = 0;
    (*(int*)&AsPlayerRecordOwner8030E168(self)->saveMenu) = 0;
    AsPlayerRecordOwner8030E168(self)->spinInputArmed = 0;

    return self;
}

extern "C" void __ct__7CBpCtrlFv(void*);
extern "C" void CBpCtrl_dtor();
extern "C" void CBpCtrl_DeleteEntryById(void*, int);
extern "C" void Layout_Destroy(void*, int);
extern "C" void PairVector_InitReserved(void*);
extern "C" void MessageDrawPairList_Dtor();
extern "C" void PointerPairList_RemoveByKey(void*, int);
extern "C" void UITextTable_ClearEntries(void*);
extern "C" void PointerArray_DeleteValue(void*, void*);
extern "C" void SharedFilePointerVector_InitReserved1024(void*);
extern "C" void SharedFilePointerVector_Destroy();
extern "C" void SharedFileHandleList_RemoveByDataName(void*, void*);
extern "C" void NString_AssignCString(void*, void*);
extern "C" char* strcpy(char*, const char*);
extern "C" void CProjectG_SetExtensionProperty(void*, int);
extern "C" void CBpCtrlHandle_ReleaseEntry(void*);
extern "C" void CBpCtrlHandle_Dtor();

extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_roundDolInputDestructorRecord[];

extern signed char s_pointerPairListInitialized;
extern char s_messageDrawPairList[];

extern signed char s_sharedFileListInitialized;
extern char s_sharedFileListStorage[];
struct DolfiniResourceStringPool {
    char rewardTable[9];
    char galleryImageTable[18];
    char prizeGalleryTable[18];
    char messageKeyFile[12];
};
extern DolfiniResourceStringPool roundDolPrizeStrings;

extern int g_project;

extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];

extern signed char s_pointerVectorInitialized;

struct DObj {
    // Empty base teardown; delete dispatches to the concrete owned object.
    virtual ~DObj() {}
};
struct DObj4 {
    char pad[4];
    // Empty base teardown for owners whose virtual interface starts at +4.
    virtual ~DObj4() {}
};

struct StrLocal {
    unsigned int flag;
    unsigned int size;
    char* ptr;
    // Free the temporary lookup string only if it used heap storage.
    ~StrLocal() { if (flag >> 31) appFreeUni__FPv(ptr); }
};

// Unregister one BP entry and clear its ID; the registry owns the entry object.
// entry borrows a session ID word. Registry initialization occurs at point of use.
static inline void ReleaseSessionBpEntry(int& entry, char* regbase) {
    if (entry != -1) {
        if (((int&)Sing_805DC5F0()) == 0) {
            if (s_bpCtrlInitialized == 0) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (regbase + 0xc));
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), entry);
        entry = -1;
    }
}
#pragma use_lmw_stmw on
// Unregister session UI entries and release all owned resources and shared file handles.
void* DolfiniSession_dtor(void* self, int flag) {
    // Positive flag deletes session storage; zero/negative performs member teardown only.
    // Singleton initialization remains interleaved because each registry is queried
    // at the point of use. Registry removals and object deletions have different owners.
    char* regbase = s_roundDolInputDestructorRecord;
    // 0x8030E5E4: release owned objects only for a live owner.
    if (self != 0) {
        // Phase 1: unregister all eight BP IDs, resetting each to the absent value -1.
        ReleaseSessionBpEntry(AsPlayerRecordOwner8030E168(self)->bpEntryId0, regbase);
        ReleaseSessionBpEntry(AsPlayerRecordOwner8030E168(self)->bpEntryId1, regbase);
        ReleaseSessionBpEntry(AsPlayerRecordOwner8030E168(self)->bpEntryId2, regbase);
        ReleaseSessionBpEntry(AsPlayerRecordOwner8030E168(self)->bpEntryId3, regbase);
        ReleaseSessionBpEntry(AsPlayerRecordOwner8030E168(self)->bpEntryId4, regbase);
        ReleaseSessionBpEntry(AsPlayerRecordOwner8030E168(self)->bpEntryId5, regbase);
        ReleaseSessionBpEntry(AsPlayerRecordOwner8030E168(self)->bpEntryId6, regbase);
        ReleaseSessionBpEntry(AsPlayerRecordOwner8030E168(self)->bpEntryId7, regbase);
        // Phase 2: delete renderer/layout, prize tables, textures and active dialog.
        // DObj4 places its virtual interface at +4; DObj places it at +0.
        {
            DObj* p = (*(DObj**)&AsPlayerRecordOwner8030E168(self)->roundDol3D);
            if (p != 0) {
                delete p;
                AsPlayerRecordOwner8030E168(self)->roundDol3D = 0;
            }
        }
        // 0x884
        {
            void* p = AsPlayerRecordOwner8030E168(self)->layout;
            if (p != 0) {
                Layout_Destroy(p, 1);
                AsPlayerRecordOwner8030E168(self)->layout = 0;
            }
        }
        // 0x40,0x44,0x48 (vtable at +4)
        /*UNROLL40*/
        {
            DObj4* p = (*(DObj4**)&AsPlayerRecordOwner8030E168(self)->rewardTable);
            if (p != 0) {
                delete p;
                (*(void**)&AsPlayerRecordOwner8030E168(self)->rewardTable) = 0;
            }
        }
        // 0x44
        {
            DObj4* p = (*(DObj4**)&AsPlayerRecordOwner8030E168(self)->galleryImageTable);
            if (p != 0) {
                delete p;
                (*(void**)&AsPlayerRecordOwner8030E168(self)->galleryImageTable) = 0;
            }
        }
        // 0x48
        {
            DObj4* p = (*(DObj4**)&AsPlayerRecordOwner8030E168(self)->prizeGalleryTable);
            if (p != 0) {
                delete p;
                (*(void**)&AsPlayerRecordOwner8030E168(self)->prizeGalleryTable) = 0;
            }
        }
        // 0x50,0x54 (vtable at 0)
        {
            DObj* p = (*(DObj**)&AsPlayerRecordOwner8030E168(self)->texture0);
            if (p != 0) {
                delete p;
                AsPlayerRecordOwner8030E168(self)->texture0 = 0;
            }
        }
        {
            DObj* p = (*(DObj**)&AsPlayerRecordOwner8030E168(self)->texture1);
            if (p != 0) {
                delete p;
                AsPlayerRecordOwner8030E168(self)->texture1 = 0;
            }
        }
        // 0x880
        {
            DObj* p = (*(DObj**)&AsPlayerRecordOwner8030E168(self)->dialog);
            if (p != 0) {
                delete p;
                AsPlayerRecordOwner8030E168(self)->dialog = 0;
            }
        }
        // Phase 3: unlink messageDraw from its registry BEFORE deleting message objects.
        if (((int&)Sing_805DC650()) == 0) {
            if (s_pointerPairListInitialized == 0) {
                PairVector_InitReserved(s_messageDrawPairList);
                __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor, (regbase + 0x24));
                s_pointerPairListInitialized = 1;
            }
            ((int&)Sing_805DC650()) = (int)s_messageDrawPairList;
        }
        PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), AsPlayerRecordOwner8030E168(self)->messageDraw);
        // 0x888
        {
            DObj* p = (*(DObj**)&AsPlayerRecordOwner8030E168(self)->messageResource);
            if (p != 0) {
                delete p;
                (*(void**)&AsPlayerRecordOwner8030E168(self)->messageResource) = 0;
            }
        }
        // 0x890
        {
            DObj* p = (*(DObj**)&AsPlayerRecordOwner8030E168(self)->messageDraw);
            if (p != 0) {
                delete p;
                (*(void**)&AsPlayerRecordOwner8030E168(self)->messageDraw) = 0;
            }
        }
        // 0x894
        {
            DObj* p = (*(DObj**)&AsPlayerRecordOwner8030E168(self)->saveMenu);
            if (p != 0) {
                delete p;
                AsPlayerRecordOwner8030E168(self)->saveMenu = 0;
            }
        }
        // Phase 4: clear the number renderer, then remove its registry-owned entry.
        UITextTable_ClearEntries(AsPlayerRecordOwner8030E168(self)->numberDraw);
        if (AsPlayerRecordOwner8030E168(self)->numberDraw != 0) {
            UITextTable_ClearEntries(AsPlayerRecordOwner8030E168(self)->numberDraw);
            if (((int&)Sing_805DC830()) == 0) {
                if (s_pointerVectorInitialized == 0) {
                    PointerVector_InitReserved(s_fontNumDrawRegistry);
                    __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, (regbase + 0x48));
                    s_pointerVectorInitialized = 1;
                }
                ((int&)Sing_805DC830()) = (int)s_fontNumDrawRegistry;
            }
            PointerArray_DeleteValue((void*)((int&)Sing_805DC830()), AsPlayerRecordOwner8030E168(self)->numberDraw);
        }
        // Phase 5: release file handles by pooled data name, then the message-key file.
        // These calls release shared registry ownership, not the file bytes directly.
        // first
        {
            if (((int&)Sing_805DC680()) == 0) {
                if (s_sharedFileListInitialized == 0) {
                    SharedFilePointerVector_InitReserved1024(s_sharedFileListStorage);
                    __register_global_object(s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy, (regbase + 0x3c));
                    s_sharedFileListInitialized = 1;
                }
                ((int&)Sing_805DC680()) = (int)s_sharedFileListStorage;
            }
            SharedFileHandleList_RemoveByDataName((void*)((int&)Sing_805DC680()), roundDolPrizeStrings.rewardTable);
        }
        // second
        {
            if (((int&)Sing_805DC680()) == 0) {
                if (s_sharedFileListInitialized == 0) {
                    SharedFilePointerVector_InitReserved1024(s_sharedFileListStorage);
                    __register_global_object(s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy, (regbase + 0x3c));
                    s_sharedFileListInitialized = 1;
                }
                ((int&)Sing_805DC680()) = (int)s_sharedFileListStorage;
            }
            SharedFileHandleList_RemoveByDataName((void*)((int&)Sing_805DC680()), roundDolPrizeStrings.galleryImageTable);
        }
        // third
        {
            if (((int&)Sing_805DC680()) == 0) {
                if (s_sharedFileListInitialized == 0) {
                    SharedFilePointerVector_InitReserved1024(s_sharedFileListStorage);
                    __register_global_object(s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy, (regbase + 0x3c));
                    s_sharedFileListInitialized = 1;
                }
                ((int&)Sing_805DC680()) = (int)s_sharedFileListStorage;
            }
            SharedFileHandleList_RemoveByDataName((void*)((int&)Sing_805DC680()), roundDolPrizeStrings.prizeGalleryTable);
        }
        if (((int&)Sing_805DC680()) == 0) {
            if (s_sharedFileListInitialized == 0) {
                SharedFilePointerVector_InitReserved1024(s_sharedFileListStorage);
                __register_global_object(s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy, (regbase + 0x3c));
                s_sharedFileListInitialized = 1;
            }
            ((int&)Sing_805DC680()) = (int)s_sharedFileListStorage;
        }
        // Materialize the message-key name before removing its shared file handle.
        {
            int* r28 = (int*)((int&)Sing_805DC680());
            char* r27;
            {
                StrLocal str;
                str.flag = 0; str.size = 0; str.ptr = 0;
                NString_AssignCString(&str, roundDolPrizeStrings.messageKeyFile);
                unsigned int v = str.flag;
                char* dest = (regbase + 0x60);
                char* src;
                if ((v >> 31) == 0) src = (char*)&str + 1; // r1+0x9
                else src = str.ptr;
                strcpy(dest, src);
                r27 = (regbase + 0x60);
            }
            SharedFileHandleList_RemoveByDataName(r28, r27);
            CProjectG_SetExtensionProperty((void*)g_project, 0);
            // Initialize the shared BP entry-handle singleton.
            if (((int&)Sing_805DC5F8()) == 0) {
                if (s_bpCtrlHandleInitialized == 0) {
                    s_bpCtrlHandle[1] = -1;
                    s_bpCtrlHandle[0] = -1;
                    __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (regbase + 0x18));
                    s_bpCtrlHandleInitialized = 1;
                }
                ((int&)Sing_805DC5F8()) = (int)s_bpCtrlHandle;
            }
            CBpCtrlHandle_ReleaseEntry((void*)((int&)Sing_805DC5F8()));
        }
        // Phase 6: extension/entry-handle state is released; optionally free the owner.
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return (void*)self;
}
#pragma use_lmw_stmw reset

// Compile-time guards for private ABI views used above.
#define LOCAL_FIELD(t, f, o) typedef char local_##t##_##f[((int)&((t*)0)->f == (o)) ? 1 : -1]
LOCAL_FIELD(U8030D804Obj, selectedElementIndex, 0x5C);
LOCAL_FIELD(U8030D804Obj, layoutReady, 0xA0);
LOCAL_FIELD(DolfiniSessionRecordView, profile, 0x58);
LOCAL_FIELD(DolfiniSessionRecordView, attributes, 0x51C);
LOCAL_FIELD(DolfiniSessionRecordView, opaqueProfileData4ca, 0x522);
LOCAL_FIELD(Source8030E168, courseRecords, 0x25C);
LOCAL_FIELD(Source8030E168, characterEquipment, 0x2CC);
LOCAL_FIELD(Source8030E168, caddieRecordsFirstEight, 0x46C);
LOCAL_FIELD(Source8030E168, caddieRecordsLastThree, 0x4AC);
LOCAL_FIELD(Source8030E168, a, 0x4D2);
LOCAL_FIELD(Source8030E168, attributes, 0x4C4);
LOCAL_FIELD(BallWindStringPool, clickSound, 0x70);
LOCAL_FIELD(DolfiniResourceStringPool, messageKeyFile, 0x2D);
#undef LOCAL_FIELD

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804625BC", _ctors_u8030D804_0, BallWind2d_InitFullTurnConstant);
#pragma pop
