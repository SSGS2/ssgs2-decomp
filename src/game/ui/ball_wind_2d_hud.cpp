// The in-round HUD panel for ball selection and wind (CBallWind2d): its layout
// updates, drawing, selector handling and the cached visibility state it applies
// to the layout. The file also holds the Dolfini session constructor and
// destructor, which sit next to it in the original binary.
#include <decomp/local_class.h>
#include <decomp/static_init.h>
#include "classes/CBallWind2d.h"
#include "classes/PlayerRecordOwner8030E168.h"
#include "game/singletons.h"
#include "game/stdexcept_local.h"
#include "sdk_protos.h"
#include "classes/CNowReloading.h"
#include "classes/CRoundDol3D.h"
#include "classes/CBpCtrl.h"
#include "classes/CFontNumDraw.h"
#include "classes/CProjectG.h"
#include "classes/CBpCtrlHandle.h"
#include "classes/CBpTex.h"

// The layout elements this panel addresses by number. Elements 0 and 22 to 29
// have no further name; 1 and 3 are the indicators drawn and centred separately.
enum BallWindElement {
    kElement0 = 0,
    kIndicatorA = 1,
    kSelector = 2,
    kIndicatorB = 3,
    kSelectorCompanion4 = 4,
    kSelectorCompanion5 = 5,
    kElement22 = 22,
    kElement23 = 23,
    kElement24 = 24,
    kElement25 = 25,
    kElement26 = 26,
    kElement27 = 27,
    kOverlay28 = 28,
    kOverlay29 = 29,
    kElementCount = 30
};
enum {
    kDimColor = -136,
    kBrightColor = -1,
    kSelectorPlayerFlag = 5
};

// Offsets into golfBallWindLayoutStrings.
enum {
    kVectorAtError = 0x1B,  // "vector::at index out of range"
    kOutOfRangeType = 0x39, // "!std::exception!!std::logic_error!!std::out_of_range!!"
    kClickSound = 0x70      // "click"
};

// The HUD interface as seen through its vtable, which starts at 0xA4. Only the
// slots called below are named.
struct BallWindLayoutBase {
    char storage[0xA4];
};
struct BallWindSlots : BallWindLayoutBase {
    virtual void Slot0() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot1() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot2() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot3() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot4() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot5() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot6() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot7() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot8() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot9() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot10() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot11() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot12() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot13() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot14() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot15() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot16() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot17() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot18() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot19() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot20() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot21() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot22() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot23() DECOMP_LOCAL_VIRTUAL;
    virtual int vfunc_24() DECOMP_LOCAL_VIRTUAL;
    virtual int PlayClickSounds() DECOMP_LOCAL_VIRTUAL;
    virtual int ApplyState() DECOMP_LOCAL_VIRTUAL;
};

extern "C" {

extern float kFloat_6_2831855;
extern float kFloat_1_IngameStatus;
extern float kFloat_0_5_BallWind2d;
extern float kBallWindClickSoundRange; // 200.0f
extern float s_ballWind2dFullTurn;
extern int ballWindSelectionElementIndices[17];
extern int ballWindScissorRectangle[4];
extern const char golfBallWindLayoutStrings[];
extern int g_soundManager;

extern int Layout_Update(void*);
extern int Layout_CollectActivated(void*, void*, int);
extern void Layout_DrawElementWithOpacity(void*, int, float);
extern void Layout_DrawWithOpacity(void*, float);
extern void Layout_SetElementPlayerFlagsAndState(void*, int, int);
extern void Layout_SetElementPlayerFlag(void*, int, int, int);
extern void Layout_SetElementValueState(void*, int, int);
extern void Layout_SetElementFeedbackState(void*, int, int);
extern void Layout_SetElementEnabled(void*, int, int);
extern void Layout_SetElementColor(void*, int, int);
extern void Layout_GetElementSize(void*, int, float*);
extern void Layout_GetElementPosition(void*, int, float*);
extern void Layout_SetElementPosition(void*, int, float*);
extern void* __nwa__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void* __nw__FUl(unsigned long);
extern void __dla__FPv(void*);
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern void Engine_GetScissorRect(void*);
extern void Engine_SetScissorRect(void*);
extern void BpObject_Draw(void*);

} // extern "C"

#define kHalf kFloat_0_5_BallWind2d
#define kOne kFloat_1_IngameStatus
#define kTwoPi kFloat_6_2831855

// Update the layout and combine the results of the three slots after it.
// The names Update and Draw are inferred from the calls they make.
int CBallWind2d::Update() {
    int updated = Layout_Update(this);
    int result = updated & 1;
    BallWindSlots* slots = (BallWindSlots*)this;
    result &= slots->vfunc_24();
    result &= slots->PlayClickSounds();
    result &= slots->ApplyState();
    return result;
}

// Draw the separately drawn indicators, clip ballObject to the ball area, then
// restore the scissor rectangle and draw the layout.
int CBallWind2d::Draw(void* ballObject) {
    if (element3DrawnSeparately) {
        Layout_DrawElementWithOpacity(this, kIndicatorB, kOne);
    }
    if (element1DrawnSeparately) {
        Layout_DrawElementWithOpacity(this, kIndicatorA, kOne);
    }
    int savedScissor[4];
    Engine_GetScissorRect(savedScissor);
    int ballScissor[4];
    int* scissorSource = ballWindScissorRectangle - 1;
    ballScissor[0] = *++scissorSource;
    ballScissor[1] = scissorSource[1];
    ballScissor[2] = scissorSource[2];
    ballScissor[3] = scissorSource[3];
    Engine_SetScissorRect(ballScissor);
    BpObject_Draw(ballObject);
    Engine_SetScissorRect(savedScissor);
    Layout_DrawWithOpacity(this, kOne);
    return 1;
}

// Map a selection value (0 to 17) to the layout element it shows.
int CBallWind2d::SelectElementForValue(int value) {
    // The original copies the 17-entry table to the stack and accepts 0 to 17.
    int elementIndices[18];
    int* source = ballWindSelectionElementIndices - 1;
    int* destination = elementIndices - 1;
    register int i = 8;
    do {
        destination[1] = source[1];
        destination[2] = source[2];
        source += 2;
        destination += 2;
    } while (--i);
    destination[1] = source[1];
    if ((unsigned int)value > 0x11 || value < 0) {
        return 0;
    }
    selectedElementIndex = elementIndices[value];
    return 1;
}

int CBallWind2d::SetSelectionActive(int active) {
    selectionActive = active;
    return 1;
}

int CBallWind2d::SetSelectionState(int state) {
    selectionState = state;
    return 1;
}

// The layout elements activated this frame, as collected by Layout_CollectActivated:
// an int vector of {data, size, capacity}. The size is read afresh on every use.
struct ActivatedList {
    int* items;
    volatile unsigned int size;
    unsigned int capacity;
    ~ActivatedList() {
        if (items) {
            size = *(unsigned int*)&size - *(int*)&size;
            appFreeUni__FPv(items);
        }
    }
};

// Bounds-checked element access, as vector::at does it.
static inline int ActivatedAt(ActivatedList& list, unsigned int index) {
    if (index >= list.size) {
        const char* message = golfBallWindLayoutStrings + kVectorAtError;
        std::out_of_range error(message, golfBallWindLayoutStrings + kOutOfRangeType);
    }
    return (*(int* volatile*)&list.items)[index];
}

// The original saves registers with stmw/lmw in the two functions that inline the exception text.
#pragma use_lmw_stmw on
// Whether the selector element is among the layout elements activated this frame.
int CBallWind2d::IsSelectorActivated() {
    ActivatedList activated = {0, 0, 0};
    Layout_CollectActivated(this, &activated, 0);
    for (unsigned int i = 0; i < activated.size; i++) {
        if (ActivatedAt(activated, i) == kSelector) {
            return 1;
        }
    }
    return 0;
}

#pragma use_lmw_stmw reset

int CBallWind2d::SetBallControlsVisible(int visible) {
    ballControlsVisible = visible;
    return 1;
}

int CBallWind2d::IsBallControlsVisible() {
    return ballControlsVisible;
}

int CBallWind2d::SetElement0Visible(int visible) {
    element0Visible = visible;
    return 1;
}

int CBallWind2d::SetElements26And27Visible(int visible) {
    elements26And27Visible = visible;
    return 1;
}

int CBallWind2d::SetElements22And25Visible(int visible) {
    element22Visible = visible;
    element25Visible = visible;
    return 1;
}

int CBallWind2d::SetElement22Visible(int visible) {
    element22Visible = visible;
    return 1;
}

int CBallWind2d::SetElements25And26Bright(int bright) {
    elements25And26Bright = bright;
    return 1;
}

// Showing element 28 hides element 29.
int CBallWind2d::SetElement28Visible(int visible) {
    element28Visible = visible;
    if (visible != 0) {
        element29Visible = 0;
    }
    return 1;
}

// Showing element 29 hides element 28.
int CBallWind2d::SetElement29Visible(int visible) {
    element29Visible = visible;
    if (visible != 0) {
        element28Visible = 0;
    }
    return 1;
}

int CBallWind2d::SetElement23Visible(int visible) {
    element23Visible = visible;
    return 1;
}

int CBallWind2d::SetElement24Visible(int visible) {
    element24Visible = visible;
    return 1;
}

int CBallWind2d::SetElement3DrawnSeparately(int separate) {
    element3DrawnSeparately = separate;
    return 1;
}

int CBallWind2d::SetElement1DrawnSeparately(int separate) {
    element1DrawnSeparately = separate;
    return 1;
}

int CBallWind2d::SetIndicatorCenterX(float centerX) {
    indicatorCenterX = centerX;
    return 1;
}

int CBallWind2d::SetIndicatorColor(int color) {
    indicatorColor = color;
    return 1;
}

int CBallWind2d::vfunc_24() {
    return 1;
}

#pragma use_lmw_stmw on
// Play the click sound once for each activated selector element.
int CBallWind2d::PlayClickSounds() {
    ActivatedList activated = {0, 0, 0};
    Layout_CollectActivated(this, &activated, 0);
    for (unsigned int i = 0; i < activated.size; i++) {
        if (ActivatedAt(activated, i) == kSelector) {
            Sound_PlayByName((void*)g_soundManager, golfBallWindLayoutStrings + kClickSound, 0, 0, 0, 0, kHalf,
                             kBallWindClickSoundRange, kOne);
        }
    }
    return 1;
}

#pragma use_lmw_stmw reset

// Push the cached state into the layout: reset every element, then apply the
// selector state, the visibility groups, the colours and the indicator positions.
int CBallWind2d::ApplyState() {
    if (layoutReady == 0) {
        return 0;
    }
    for (int i = 0; i < kElementCount; i++) {
        Layout_SetElementPlayerFlagsAndState(this, i, 0);
    }
    if (selectionActive) {
        Layout_SetElementPlayerFlag(this, kSelector, 1, kSelectorPlayerFlag);
    } else {
        Layout_SetElementPlayerFlag(this, kSelector, 0, kSelectorPlayerFlag);
    }
    // States 0, 1 and 2 go unchanged to both the value and the feedback state.
    switch (selectionState) {
    case 0:
        Layout_SetElementValueState(this, kSelector, 0);
        Layout_SetElementFeedbackState(this, kSelector, 0);
        break;
    case 1:
        Layout_SetElementValueState(this, kSelector, 1);
        Layout_SetElementFeedbackState(this, kSelector, 1);
        break;
    case 2:
        Layout_SetElementValueState(this, kSelector, 2);
        Layout_SetElementFeedbackState(this, kSelector, 2);
        break;
    }
    if (ballControlsVisible) {
        Layout_SetElementEnabled(this, kSelectorCompanion4, 1);
        Layout_SetElementEnabled(this, kSelectorCompanion5, 1);
        Layout_SetElementEnabled(this, kSelector, 1);
        Layout_SetElementEnabled(this, selectedElementIndex, 1);
    }
    if (element0Visible) {
        Layout_SetElementEnabled(this, kElement0, 1);
    }
    if (elements26And27Visible) {
        Layout_SetElementEnabled(this, kElement27, 1);
        Layout_SetElementEnabled(this, kElement26, 1);
    }
    if (element22Visible) {
        Layout_SetElementEnabled(this, kElement22, 1);
    }
    if (element25Visible) {
        Layout_SetElementEnabled(this, kElement25, 1);
    }
    if (element23Visible) {
        Layout_SetElementEnabled(this, kElement23, 1);
    }
    if (element24Visible) {
        Layout_SetElementEnabled(this, kElement24, 1);
    }
    if (element28Visible && ballControlsVisible) {
        Layout_SetElementEnabled(this, kOverlay28, 1);
    }
    if (element29Visible && ballControlsVisible) {
        Layout_SetElementEnabled(this, kOverlay29, 1);
    }
    // Element 27 follows the selector; elements 25 and 26 share one flag.
    if (selectionActive == 0) {
        Layout_SetElementColor(this, kElement27, kDimColor);
    } else {
        Layout_SetElementColor(this, kElement27, kBrightColor);
    }
    if (elements25And26Bright == 0) {
        Layout_SetElementColor(this, kElement25, kDimColor);
        Layout_SetElementColor(this, kElement26, kDimColor);
    } else {
        Layout_SetElementColor(this, kElement25, kBrightColor);
        Layout_SetElementColor(this, kElement26, kBrightColor);
    }
    Layout_SetElementColor(this, kIndicatorB, indicatorColor);
    Layout_SetElementColor(this, kIndicatorA, indicatorColor);
    // Move each indicator so that it is centred on indicatorCenterX.
    float f[2];
    Layout_GetElementSize(this, kIndicatorB, f);
    float old = f[0];
    Layout_GetElementPosition(this, kIndicatorB, f);
    f[0] = indicatorCenterX - kHalf * old;
    Layout_SetElementPosition(this, kIndicatorB, f);
    Layout_GetElementSize(this, kIndicatorA, f);
    old = f[0];
    Layout_GetElementPosition(this, kIndicatorA, f);
    f[0] = indicatorCenterX - kHalf * old;
    Layout_SetElementPosition(this, kIndicatorA, f);
    return 1;
}

extern "C" {

// Cache 2*pi for this unit.
void BallWind2d_InitFullTurnConstant() {
    s_ballWind2dFullTurn = kTwoPi;
}

extern "C" void SaveData_InitPlayerRecord(void*);
extern "C" void __dt__13CNowReloadingFv();
extern "C" int LoadingIndicator_IsAbsentOrState3(void*);
extern "C" void LoadingIndicator_CreatePlayer(void*);
extern "C" void Layout_Init(void*, int);
extern "C" void PointerVector_InitReserved(void*);
extern "C" void PointerVector_DestroyStorage();
extern "C" void __dl__FPv(void*);

extern signed char s_nowReloadingInitialized;
extern char s_nowReloading[8];
extern char s_dolfiniReloadingDtorRegistration[];
extern signed char s_pointerVectorInitialized;
extern char s_fontNumDrawRegistry[];
extern char s_dolfiniNumberDrawDtorRegistration[];

// Allocate session-owned helpers through the game's operator-new entry point.
inline void* operator new(size_t size) {
    return __nw__FUl(size);
}
void operator delete(void*) throw();

// Construct the owned layout with the existing initialization mode 5.
struct S5c {
    char buf[0x5c];
    S5c() {
        Layout_Init(this, 5);
    }
};
// Construct one owned BP texture wrapper.
struct S28 {
    char buf[0x28];
    S28() {
        ((CBpTex*)this)->__ct();
    }
};
struct Copy25c {
    int data[151];
};
struct Copy70 {
    int data[28];
};
struct Copy1a0 {
    unsigned short data[208];
};
struct Copy33c {
    unsigned short data[414];
};
struct Copy40 {
    int data[16];
};
struct Copy18 {
    unsigned char data[24];
};
struct Copy8 {
    unsigned char data[8];
};
struct Copy14 {
    int data[5];
};
// Record copies keep the original word and halfword widths.
// Source layout: +25C..2CB course records; +2CC..46B eight 0x34-byte character
// equipment rows (24 part indices and a hairstyle byte); +46C..4C3 eleven 8-byte
// caddie records.

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

struct DolfiniSessionInput {
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
// self is the session object. arg is borrowed and copied; it must include all
// DolfiniSessionInput fields, including the words and tail.
DolfiniSessionRecordView* DolfiniSession_Ctor(DolfiniSessionRecordView* self, DolfiniSessionInput* arg) {
    PlayerRecordOwner8030E168* owner = (PlayerRecordOwner8030E168*)self;
    // Phase 1: establish a default profile and ensure the loading indicator exists.
    SaveData_InitPlayerRecord(&self->profile);
    int initialManager = ((int&)Sing_805DC610());
    owner->resourcesLoaded = 0;
    if (initialManager == 0) {
        if (s_nowReloadingInitialized == 0) {
            ((CNowReloading*)s_nowReloading)->Init();
            __register_global_object(s_nowReloading, (void*)__dt__13CNowReloadingFv,
                                     s_dolfiniReloadingDtorRegistration);
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
    self->courseRecords = arg->courseRecords;
    self->characterEquipment = arg->characterEquipment;

    // Caddie table spans 11 eight-byte records at source+46C..4C3. Keep the
    // 8+3 copy groups: they are one table, not two kinds of caddie data.
    self->caddieRecordsFirstEight = arg->caddieRecordsFirstEight;

    self->caddieRecordsLastThree = arg->caddieRecordsLastThree;
    // Six packed profile attributes are copied verbatim; do not widen these byte stores.
    owner->profileAttribute0 = arg->attributes[0];
    owner->profileAttribute1 = arg->attributes[1];
    owner->profileAttribute2 = arg->attributes[2];
    owner->profileAttribute3 = arg->attributes[3];
    owner->profileAttribute4 = arg->attributes[4];
    owner->profileAttribute5 = arg->attributes[5];
    self->opaqueProfileData4ca = arg->opaqueProfileData4ca;
    self->field52a = arg->a;
    self->field52b = arg->b;
    self->field52c = arg->c;

    self->words = arg->words;

    self->tail = arg->tail;

    // Phase 3: mark registry IDs absent before allocating independently owned UI helpers.
    owner->bpEntryId0 = -1;
    owner->bpEntryId5 = -1;
    owner->bpEntryId6 = -1;
    owner->bpEntryId1 = -1;
    owner->bpEntryId2 = -1;
    owner->bpEntryId3 = -1;
    owner->bpEntryId4 = -1;
    owner->bpEntryId7 = -1;
    owner->speedLevel = 0;
    owner->peakSpeedLevel = 0;
    owner->preloadAnimationIndex = 0;
    owner->rewardKind = 0;
    (*(int*)&owner->state) = 0;

    // Construct the owned 3D session renderer.
    owner->roundDol3D = new CRoundDol3D();
    S5c* second = new S5c();
    int initialController = ((int&)Sing_805DC830());
    owner->layout = second;
    (*(int*)&owner->dialog) = 0;
    if (initialController == 0) {
        if (s_pointerVectorInitialized == 0) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                     s_dolfiniNumberDrawDtorRegistration);
            s_pointerVectorInitialized = 1;
        }
        ((int&)Sing_805DC830()) = (int)s_fontNumDrawRegistry;
    }
    owner->numberDraw = CFontNumDraw::CreateInVector((PtrVector*)((int&)Sing_805DC830()));

    owner->texture0 = new S28();
    owner->texture1 = new S28();

    owner->messageResource = 0;
    (*(int*)&owner->textFactory) = 0;
    owner->messageDraw = 0;
    owner->rewardTable = 0;
    owner->galleryImageTable = 0;
    owner->prizeGalleryTable = 0;
    (*(int*)&owner->saveMenu) = 0;
    owner->spinInputArmed = 0;

    return self;
}

extern "C" void CBpCtrl_dtor();
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
    ~StrLocal() {
        if (flag >> 31)
            appFreeUni__FPv(ptr);
    }
};

// Unregister one BP entry and clear its ID; the registry owns the entry object.
// entry borrows a session ID word. Registry initialization occurs at point of use.
static inline void ReleaseSessionBpEntry(int& entry, char* regbase) {
    if (entry != -1) {
        if (((int&)Sing_805DC5F0()) == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (regbase + 0xc));
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(entry);
        entry = -1;
    }
}
#pragma use_lmw_stmw on
// Unregister session UI entries and release all owned resources and shared file handles.
void* DolfiniSession_dtor(void* self, int flag) {
    PlayerRecordOwner8030E168* owner = (PlayerRecordOwner8030E168*)self;
    // Positive flag deletes session storage; zero/negative performs member teardown only.
    // Singleton initialization remains interleaved because each registry is queried
    // at the point of use. Registry removals and object deletions have different owners.
    char* regbase = s_roundDolInputDestructorRecord;
    // Release owned objects only for a live owner.
    if (self != 0) {
        // Phase 1: unregister all eight BP IDs, resetting each to the absent value -1.
        ReleaseSessionBpEntry(owner->bpEntryId0, regbase);
        ReleaseSessionBpEntry(owner->bpEntryId1, regbase);
        ReleaseSessionBpEntry(owner->bpEntryId2, regbase);
        ReleaseSessionBpEntry(owner->bpEntryId3, regbase);
        ReleaseSessionBpEntry(owner->bpEntryId4, regbase);
        ReleaseSessionBpEntry(owner->bpEntryId5, regbase);
        ReleaseSessionBpEntry(owner->bpEntryId6, regbase);
        ReleaseSessionBpEntry(owner->bpEntryId7, regbase);
        // Phase 2: delete renderer/layout, prize tables, textures and active dialog.
        // DObj4 places its virtual interface at +4; DObj places it at +0.
        {
            DObj* p = (*(DObj**)&owner->roundDol3D);
            if (p != 0) {
                delete p;
                owner->roundDol3D = 0;
            }
        }
        // 0x884
        {
            void* p = owner->layout;
            if (p != 0) {
                Layout_Destroy(p, 1);
                owner->layout = 0;
            }
        }
        // 0x40,0x44,0x48 (vtable at +4)
        /*UNROLL40*/
        {
            DObj4* p = (*(DObj4**)&owner->rewardTable);
            if (p != 0) {
                delete p;
                (*(void**)&owner->rewardTable) = 0;
            }
        }
        // 0x44
        {
            DObj4* p = (*(DObj4**)&owner->galleryImageTable);
            if (p != 0) {
                delete p;
                (*(void**)&owner->galleryImageTable) = 0;
            }
        }
        // 0x48
        {
            DObj4* p = (*(DObj4**)&owner->prizeGalleryTable);
            if (p != 0) {
                delete p;
                (*(void**)&owner->prizeGalleryTable) = 0;
            }
        }
        // 0x50,0x54 (vtable at 0)
        {
            DObj* p = (*(DObj**)&owner->texture0);
            if (p != 0) {
                delete p;
                owner->texture0 = 0;
            }
        }
        {
            DObj* p = (*(DObj**)&owner->texture1);
            if (p != 0) {
                delete p;
                owner->texture1 = 0;
            }
        }
        // 0x880
        {
            DObj* p = (*(DObj**)&owner->dialog);
            if (p != 0) {
                delete p;
                owner->dialog = 0;
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
        PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), owner->messageDraw);
        // 0x888
        {
            DObj* p = (*(DObj**)&owner->messageResource);
            if (p != 0) {
                delete p;
                (*(void**)&owner->messageResource) = 0;
            }
        }
        // 0x890
        {
            DObj* p = (*(DObj**)&owner->messageDraw);
            if (p != 0) {
                delete p;
                (*(void**)&owner->messageDraw) = 0;
            }
        }
        // 0x894
        {
            DObj* p = (*(DObj**)&owner->saveMenu);
            if (p != 0) {
                delete p;
                owner->saveMenu = 0;
            }
        }
        // Phase 4: clear the number renderer, then remove its registry-owned entry.
        UITextTable_ClearEntries(owner->numberDraw);
        if (owner->numberDraw != 0) {
            UITextTable_ClearEntries(owner->numberDraw);
            if (((int&)Sing_805DC830()) == 0) {
                if (s_pointerVectorInitialized == 0) {
                    PointerVector_InitReserved(s_fontNumDrawRegistry);
                    __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                             (regbase + 0x48));
                    s_pointerVectorInitialized = 1;
                }
                ((int&)Sing_805DC830()) = (int)s_fontNumDrawRegistry;
            }
            PointerArray_DeleteValue((void*)((int&)Sing_805DC830()), owner->numberDraw);
        }
        // Phase 5: release file handles by pooled data name, then the message-key file.
        // These calls release shared registry ownership, not the file bytes directly.
        // first
        {
            if (((int&)Sing_805DC680()) == 0) {
                if (s_sharedFileListInitialized == 0) {
                    SharedFilePointerVector_InitReserved1024(s_sharedFileListStorage);
                    __register_global_object(s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy,
                                             (regbase + 0x3c));
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
                    __register_global_object(s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy,
                                             (regbase + 0x3c));
                    s_sharedFileListInitialized = 1;
                }
                ((int&)Sing_805DC680()) = (int)s_sharedFileListStorage;
            }
            SharedFileHandleList_RemoveByDataName((void*)((int&)Sing_805DC680()),
                                                  roundDolPrizeStrings.galleryImageTable);
        }
        // third
        {
            if (((int&)Sing_805DC680()) == 0) {
                if (s_sharedFileListInitialized == 0) {
                    SharedFilePointerVector_InitReserved1024(s_sharedFileListStorage);
                    __register_global_object(s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy,
                                             (regbase + 0x3c));
                    s_sharedFileListInitialized = 1;
                }
                ((int&)Sing_805DC680()) = (int)s_sharedFileListStorage;
            }
            SharedFileHandleList_RemoveByDataName((void*)((int&)Sing_805DC680()),
                                                  roundDolPrizeStrings.prizeGalleryTable);
        }
        if (((int&)Sing_805DC680()) == 0) {
            if (s_sharedFileListInitialized == 0) {
                SharedFilePointerVector_InitReserved1024(s_sharedFileListStorage);
                __register_global_object(s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy,
                                         (regbase + 0x3c));
                s_sharedFileListInitialized = 1;
            }
            ((int&)Sing_805DC680()) = (int)s_sharedFileListStorage;
        }
        // Materialize the message-key name before removing its shared file handle.
        {
            int* sharedFileList = (int*)((int&)Sing_805DC680());
            char* messageKeyName;
            {
                StrLocal str;
                str.flag = 0;
                str.size = 0;
                str.ptr = 0;
                NString_AssignCString(&str, roundDolPrizeStrings.messageKeyFile);
                unsigned int v = str.flag;
                char* dest = (regbase + 0x60);
                char* src;
                if ((v >> 31) == 0)
                    src = (char*)&str + 1; // r1+0x9
                else
                    src = str.ptr;
                strcpy(dest, src);
                messageKeyName = (regbase + 0x60);
            }
            SharedFileHandleList_RemoveByDataName(sharedFileList, messageKeyName);
            ((CProjectG*)g_project)->SetExtensionProperty(0);
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
            ((CBpCtrlHandle*)(((int&)Sing_805DC5F8())))->ReleaseEntry();
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
LOCAL_FIELD(DolfiniSessionRecordView, profile, 0x58);
LOCAL_FIELD(DolfiniSessionRecordView, attributes, 0x51C);
LOCAL_FIELD(DolfiniSessionRecordView, opaqueProfileData4ca, 0x522);
LOCAL_FIELD(DolfiniSessionInput, courseRecords, 0x25C);
LOCAL_FIELD(DolfiniSessionInput, characterEquipment, 0x2CC);
LOCAL_FIELD(DolfiniSessionInput, caddieRecordsFirstEight, 0x46C);
LOCAL_FIELD(DolfiniSessionInput, caddieRecordsLastThree, 0x4AC);
LOCAL_FIELD(DolfiniSessionInput, a, 0x4D2);
LOCAL_FIELD(DolfiniSessionInput, attributes, 0x4C4);
LOCAL_FIELD(DolfiniResourceStringPool, messageKeyFile, 0x2D);
#undef LOCAL_FIELD

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804625BC", _ctors_u8030D804_0, BallWind2d_InitFullTurnConstant);
#pragma pop
