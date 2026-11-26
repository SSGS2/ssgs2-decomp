// Tour area-selection menu (MenuGroup802FB4D0): the area reveal queue, the hover list, and the
// layouts that show each area. The item-request queue that dispatches area rewards is here too.

#include <decomp/static_init.h>
#include "ppc_double_words.h"
// Tour area-selection menu, new-area reveal queue and item-menu owner lifetime.
// The area-selection menu advances resources -> reveal -> rewards -> selection;
// TourMenuGroup_DrawActiveEntries draws registered BP scenes/layouts.
// Modding lookup:
// - Widescreen map backdrop: DrawActiveEntries, BP draw return site 0x802FB568
//   this file sets no viewport, scissor, framebuffer resolution or aspect ratio.
// - Level atlas row: RefreshLevelAndConnections, selectedArea * 32 pixels.
// - Connection sprite sizes: s_tourAreaConnectionMarkerIndices selects records
//   in s_tourAreaConnectionMarkers; X/Y factors 98/62 in the data section.
//   Products are passed as texture-region SIZE, not world/board position.
// - Model/course/resource loading: texture/data archive names live in
//   tourAreaSelectStrings; BP scripts/layouts control the map presentation.
//   Area IDs here are 0..7; they are not the eleven character-map indices used
//   by OpenItemMenuState in tour_character_map_board.cpp.
// Resource polling is asynchronous. The owner retains archive/texture wrappers,
// owns layouts, and stores BP registration IDs; getters borrow singleton state.
// Draw/input require initialized loaded objects. No per-frame motion scalar here:
// reveal completion comes from the BP script, rewards from the request processor.
extern "C" char s_fadeOverlayDtorRegistration_TourArea[];
#include "classes/MenuGroup802FB4D0.h"
#include "classes/ItemRequestState80300EC8.h"
// Borrowed layout views preserve the original field layout and byte cursors.
// Borrow the area group's selected ID, BP registrations and two layout handles.
static inline MenuGroup802FB4D0* AsMenuGroup(void* object) {
    return (MenuGroup802FB4D0*)object;
}
#include "classes/MenuOwner802FB7F8.h"
#include "classes/CTourWindow.h" // same 0x28 bytes; names proved by the selection consumers
// Borrow the separate item-owner layout; it is not the area-group layout.
static inline MenuOwner802FB7F8* AsMenuOwner(void* object) {
    return (MenuOwner802FB7F8*)object;
}

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CBpCtrl.h"
#include "classes/CPackTextureResource.h"
#include "classes/CPackedFile.h"

struct AreaRegisteredObject {
    char reserved[0x28];
    int enabled;
};

enum AreaSelectionState {
    kAreaLoadArchives = 1,
    kAreaWaitResources = 2,
    kAreaWaitControllers = 3,
    kAreaWaitFade = 4,
    kAreaChoose = 5,
    kAreaStartReveal = 6,
    kAreaWaitReveal = 7,
    kAreaDispatchRewards = 8,
    kAreaWaitRewards = 9,
    kAreaCancelled = 10,
    kAreaStartConfirmFade = 11,
    kAreaConfirmed = 12
};
// Transition contract: 1 allocates ->2 polls/creates UI ->3 waits BP/layout ->4
// waits fade ->6 if reveals remain, else 8. 6 starts a reveal and falls into 7;
// 7 completion flags/unlinks one node ->6 or 8. 8 dispatches rewards ->9 until
// complete, else ->5. 5 confirms ->11 ->12 (disable BP => +1, no fade poll), or cancels
// ->10 (disable BP => -1). Otherwise return 0. Queue nodes own only themselves;
// their persistent flag pointers are borrowed. Never free a flag with its node.
enum {
    kAreaCount = 8,
    kAreaDetailSlotCount = 3,
    kNoArea = -1,
    kActionConfirm = 4,
    kActionCancel = 6,
    kAreaRevealFinished = 0x40,
    kFadeBusy = 3,
    kAreaHoverMask = 5,
    kAreaDrawEnd = 13,
    kAreaLevelRowPixels = 32
};

extern "C" {
extern "C" float kFloat_6_2831855;
extern "C" float s_tourAreaSelectRotationPeriod;

extern "C" float kFloat_1_MenuGroup;
extern "C" float kTourAreaMarkerScaleX;
extern "C" float kTourAreaMarkerScaleY;
extern "C" double kSignedIntToDoubleBias_TourAreaSelect;

extern "C" void* Layout_Init(void*, int);
struct AreaMenuLayoutStorage {
    char storage[0x5C];
    // Construct an owned layout with the caller's layout-mode selector.
    AreaMenuLayoutStorage(int layoutMode) {
        Layout_Init(this, layoutMode);
    }
};

extern "C" signed char s_bpCtrlInitialized;
extern "C" signed char s_pointerVectorInitialized;
extern "C" char s_bpCtrl[];
extern "C" char s_bpCtrlDtorRegistration_tourWindow[];
extern "C" char s_bpCtrlDtorRegistration_80522860[];
extern "C" char s_fontNumDrawRegistry[];
extern "C" char s_fontNumDrawRegistryDtorRegistration_Tour[];
// Packed view of the existing string storage; member lengths include terminators.
struct TourAreaSelectStringPool {
    char textureArchive[24];
    char dataArchive[26];
    char pointLayout[19];
    char logoLayout[22];
    char selectScript[11];
    char defaultGroup[8];
    char newLabel[11];
    char levelGroup[5];
    char boundsError[30];
    char boundsExceptionType[55];
    char selectSound[17];
    char openAreaSound[15];
    char treeLengthError[26];
    char lengthExceptionType[55];
};
extern "C" char tourAreaSelectStrings[];
// Borrow the packed area archive/script names; retain original offsets and string storage.
static inline TourAreaSelectStringPool& TourAreaSelectStringPoolView() {
    return *(TourAreaSelectStringPool*)tourAreaSelectStrings;
}

// s_tourAreaConnectionMarkers contains 15 twelve-byte {s32 type,f32 x,f32 y}
// records (plus a trailing unused word in its owning MenuPositions aggregate).
// Stock records 0..4: type0, x=0..4,y=0; 5..9: type0,x=0..4,y=1;
// 10..14: type1,x=0..4,y=0. type==1 chooses detail value-state 3; EVERY
// other integer chooses state 2. x/y are dimensionless multipliers, converted
// to texture-region width/height pixels by x*98,y*62, NOT sprite positions.
// s_tourAreaConnectionMarkerIndices is s32 [8][3], indexed by selectedArea
// 0..7 then detail slot 0..2. -1 hides the slot; other values MUST be 0..14
// (no bounds check). Stock rows: {0,-1,-1},{1,-1,-1},{2,-1,-1},{3,7,-1},
// {4,8,-1},{5,13,-1},{10,11,12},{6,14,-1}. A hidden slot remains disabled by
// RefreshLevelAndConnections' initial three-element clear. New records need
// the owning data aggregate expanded as well; changing the extern bound alone
// allocates nothing. Preserve type/atlas value states and both scale consumers.
struct TourAreaConnectionMarker {
    int type;
    float x;
    float y;
};
extern "C" TourAreaConnectionMarker s_tourAreaConnectionMarkers[15];
extern "C" int s_tourAreaConnectionMarkerIndices[];

extern "C" void CBpCtrl_dtor();
extern "C" void* IdMap_Lookup(void*, int);
extern "C" void BpObject_SetEnabled(void*, int);
extern "C" void BpObject_Draw(void*);
extern "C" void* BpObject_GetOrCreateNamedGroup(void*, void*);
extern "C" void UIGroup_SetEnabled(void*, int);
extern "C" int UIGroup_SetSourcePosition(void*, void*);
extern "C" void Sprite_SetTextureRegionSize2D(void*, void*);

extern "C" void Layout_UpdateCursorHover(void*, float);
extern "C" void Layout_SetElementPlayerFlagsAndState(void*, int, int);
extern "C" int Layout_UpdateExtended(void*);
extern "C" void Layout_SetElementValueState(void*, int, int);
extern "C" void* Layout_GetElementSprite(void*, int);
extern "C" void Layout_SetElementEnabled(void*, int, int);
extern "C" void* Layout_Destroy(void*, int);

extern "C" void UITextTable_ClearEntries(void*);
extern "C" void PointerVector_InitReserved(void*);
extern "C" int PointerArray_DeleteValue(void*, void*);
extern "C" void PointerVector_DestroyStorage();
extern "C" void __dl__FPv(void*);

// Initialize the unit rotation-period value.
void TourAreaSelect_InitRotationPeriod() {
    s_tourAreaSelectRotationPeriod = kFloat_6_2831855;
}

// Allocate the item-menu layout and clear its BP registration and state fields.
// Required fresh self covers CTourWindow's 0x28 bytes; context is the borrowed
// 0x828-byte player-record snapshot, retained until window destruction. This is
// the character/CADDIE window inside the tour item menu, not an area-group.
// Full written schema: +00 screenState=0 (inactive; 1 selecting, 2 closing),
// +04 BP entry ID=0 (later replaced by LoadResources), +08 owned Layout(mode5),
// +0C signed selectedCharacter=0, +0D signed selectedCaddie=0, +20 playerRecord,
// +24 numberDraw=null. +0E/+16 availability[8] and +1E padding are untouched.
// Caller CreateTourItemWindowsAndLayout (tour_character_map_board.cpp) immediately calls
// CTourWindow_ResetNumberDraw/LoadResources (tour_change_character_window.cpp). After polling load,
// CTourWindow_InitializeCharacterSelection sets state=1, replaces both indices
// from the record and fills availability from saved flag 40. Selection input
// consumes -1/0..7 indices, updates preview/stats, and commits only on confirm;
// cancellation/confirm enter state2, whose update disables BP and returns 1.
// Thus initializer zero indices are placeholders, not a saved character change.
// The window destructor owns layout teardown and BP deletion; numberDraw belongs
// to the shared registry. Context is never freed here.
// Allocation failure is not returned; exceptions leave the already-written BP
// word=0 and no completed layout publication. No whole-object clear is intended.
void* TourItemMenu_Init(void* self, void* context) {
    AsMenuOwner(self)->bpEntryId = 0;
    AreaMenuLayoutStorage* o = new AreaMenuLayoutStorage(5);
    AsMenuOwner(self)->layout = o;
    ((CTourWindow*)self)->selectedCharacterIndex = 0;
    ((CTourWindow*)self)->selectedCaddieIndex = 0;
    *(int*)&((CTourWindow*)self)->screenState = 0;
    AsMenuOwner(self)->context = context;
    (*(int*)&AsMenuOwner(self)->registeredObject) = 0;
    return self;
}

// Draw each active BP entry and the associated layouts.
// self required, entryCount is 0..3; IDs occupy self+04,+08,+0C, followed
// immediately by entryCount at +10 (MenuGroup802FB4D0::controllerIds[3] is the
// complete bounded overlay). A fourth entry reads the count as an
// ID; extending this array requires relocating the following fields as an ABI
// change, not merely increasing entryCount. Entries/layouts must be loaded.
// States 4..12 draw in ID order; other states have no draw/hover side effects.
// No IDs are removed by drawing. The volatile cursor preserves observable loads
// of singleton and ID words; do not cache them across BP drawing callbacks.
void TourMenuGroup_DrawActiveEntries(void* self) {
    int state = AsMenuGroup(self)->state;
    void* menu = self;
    // 0x802FB4D0: only active menu states draw their entries.
    if (state >= kAreaWaitFade && state < kAreaDrawEnd) {
        char* bpStorageUnused = s_bpCtrl; // lis
        volatile void* entryCursor = self;
        // r28 unused as func ptr
        char* bpStorage = s_bpCtrl;
        char* registration = s_bpCtrlDtorRegistration_80522860;
        int unusedOne = 1;
        for (int entryIndex = 0; entryIndex < AsMenuGroup(menu)->entryCount; entryIndex++) {
            if (((volatile void*&)Sing_805DC5F0()) == 0) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80522860);
                    s_bpCtrlInitialized = 1;
                }
                ((volatile void*&)Sing_805DC5F0()) = (void*)s_bpCtrl;
            }
            int bpEntryId = *(int*)((volatile char*)entryCursor + 0x4);
            void* bpObject = IdMap_Lookup((void*)((volatile void*&)Sing_805DC5F0()), bpEntryId);
            BpObject_Draw(bpObject);
            entryCursor = (volatile void*)((int*)entryCursor + 1);
        }
        Layout_UpdateCursorHover(AsMenuGroup(menu)->firstLayout, kFloat_1_MenuGroup);
        Layout_UpdateCursorHover(AsMenuGroup(menu)->secondLayout, kFloat_1_MenuGroup);
    }
}

// Destroy the window: drop its number renderer, layout and BP entry.
CTourWindow::~CTourWindow() {
    if (numberDraw != 0) {
        ((AreaRegisteredObject*)numberDraw)->enabled = 0;
        UITextTable_ClearEntries(numberDraw);
        if (((volatile void*&)Sing_805DC830()) == 0) {
            if (!s_pointerVectorInitialized) {
                PointerVector_InitReserved(s_fontNumDrawRegistry);
                __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                         s_fontNumDrawRegistryDtorRegistration_Tour);
                s_pointerVectorInitialized = 1;
            }
            ((volatile void*&)Sing_805DC830()) = (void*)s_fontNumDrawRegistry;
        }
        PointerArray_DeleteValue((void*)((volatile void*&)Sing_805DC830()), numberDraw);
    }
    if (layout != 0) {
        Layout_Destroy(layout, 1);
        layout = 0;
    }
    if ((int)bpEntryId != -1) {
        if (((volatile void*&)Sing_805DC5F0()) == 0) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_tourWindow);
                s_bpCtrlInitialized = 1;
            }
            ((volatile void*&)Sing_805DC5F0()) = (void*)s_bpCtrl;
        }
        ((CBpCtrl*)(((volatile void*&)Sing_805DC5F0())))->DeleteEntryById((int)bpEntryId);
        bpEntryId = (void*)-1;
    }
}

// Disable the BP entry and its registered UI object.
void TourItemMenu_DisableRegisteredObject(void* self) {
    if (((volatile void*&)Sing_805DC5F0()) == 0) {
        if (!s_bpCtrlInitialized) {
            char* bpStorage = s_bpCtrl;
            ((CBpCtrl*)bpStorage)->__ct();
            __register_global_object(bpStorage, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_tourWindow);
            s_bpCtrlInitialized = 1;
        }
        ((volatile void*&)Sing_805DC5F0()) = (void*)s_bpCtrl;
    }
    volatile void* menu = self;
    void* bpObject =
        IdMap_Lookup((void*)((volatile void*&)Sing_805DC5F0()), ((volatile MenuOwner802FB7F8*)menu)->bpEntryId);
    BpObject_SetEnabled(bpObject, 0);
    // The original cast reads the pointee without volatile qualification; only
    // the local pointer object above is volatile. Preserve that distinction.
    ((AreaRegisteredObject*)AsMenuOwner((void*)menu)->registeredObject)->enabled = 0;
}

// Refresh the selected area level and up to three connected-area marker sprites.
// selectedArea is -1 (hide details) or 0..7. Each table row has three indices;
// -1 suppresses a slot, other entries must be 0..14 in the 15-record table
// documented beside TourAreaConnectionMarker. Type 1 selects layout value 3;
// all other types select value 2. These are layout variants, not area IDs;
// their artwork meaning comes from pointLayout, not a separate enum here. Level group
// and secondLayout must exist: this code does not recover from missing assets.
// Table dimensions are multiplied by 98/62 then passed to the sprite size API;
// larger factors enlarge the source region, not the marker's world position.
// Preserve the conversion-bias construction and exact shift-before-conversion.
void TourAreaSelect_RefreshLevelAndConnections(void* self) {
    void* menu = self;
    // Phase 1: levelNameThenMarkers is a string; levelGroupThenConnections is
    // BP construction storage and then a UI group. Phase 2 reuses them for the
    // marker and connection-index arrays (see the explicit transition below).
    // Splitting these locals changes register allocation in the matching object.
    void* levelNameThenMarkers = (void*)(TourAreaSelectStringPoolView().levelGroup);
    void* levelGroupThenConnections;
    int slotIndex;
    // global init
    if (((volatile void*&)Sing_805DC5F0()) == 0) {
        if (!s_bpCtrlInitialized) {
            levelGroupThenConnections = (void*)s_bpCtrl;
            ((CBpCtrl*)levelGroupThenConnections)->__ct();
            __register_global_object(levelGroupThenConnections, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80522860);
            s_bpCtrlInitialized = 1;
        }
        ((volatile void*&)Sing_805DC5F0()) = (void*)s_bpCtrl;
    }
    {
        void* areaBpObject = IdMap_Lookup((void*)((volatile void*&)Sing_805DC5F0()), AsMenuGroup(menu)->bpEntryId);
        if (areaBpObject == 0) {
            levelGroupThenConnections = 0;
        } else {
            levelGroupThenConnections = BpObject_GetOrCreateNamedGroup(areaBpObject, levelNameThenMarkers);
        }
    }
    // Clear all three detail slots before showing this area's level/connection data.
    Layout_UpdateExtended(AsMenuGroup(menu)->secondLayout);
    for (slotIndex = 0; slotIndex < kAreaDetailSlotCount; slotIndex++) {
        Layout_SetElementPlayerFlagsAndState(AsMenuGroup(menu)->secondLayout, slotIndex, 0);
    }
    UIGroup_SetEnabled(levelGroupThenConnections, 0);
    if (*(int*)menu != -1) {
        int v = *(int*)menu;
        union {
            double d;
            unsigned int w[2];
        } u;
        u.w[PPC_DW_HI] = 0x43300000u;
        u.w[PPC_DW_LO] = ((unsigned int)v << 5) ^ 0x80000000u;
        float f0 = (float)(u.d - kSignedIntToDoubleBias_TourAreaSelect);
        struct {
            int a;
            float b;
        } st;
        st.a = 0;
        st.b = f0;
        // Select the level strip's source row: selectedArea * 32 pixels, x = 0.
        UIGroup_SetEnabled(levelGroupThenConnections, 1);
        UIGroup_SetSourcePosition(levelGroupThenConnections, &st);
        // Each area owns three table indices; -1 leaves that detail slot hidden.
        // Phase 2: neither local refers to a UI name/group beyond this point.
        levelNameThenMarkers = (void*)s_tourAreaConnectionMarkers;
        levelGroupThenConnections = (void*)s_tourAreaConnectionMarkerIndices;
        float f30 = kTourAreaMarkerScaleX;
        float f31 = kTourAreaMarkerScaleY;
        slotIndex = 0;
        int connectionByteOffset = 0;
        while (slotIndex < kAreaDetailSlotCount) {
            int selectedArea = *(int*)menu;
            int connectionOffset = connectionByteOffset + selectedArea * 12;
            int markerIndex = *(int*)((char*)levelGroupThenConnections + connectionOffset);
            if (markerIndex != -1) {
                TourAreaConnectionMarker* marker = &((TourAreaConnectionMarker*)levelNameThenMarkers)[markerIndex];
                int type = marker->type;
                void* layout = AsMenuGroup(menu)->secondLayout;
                if (type == 1) {
                    Layout_SetElementValueState(layout, slotIndex, 3);
                } else if (type >= 2) {
                    Layout_SetElementValueState(layout, slotIndex, 2);
                } else if (type >= 0) {
                    Layout_SetElementValueState(layout, slotIndex, 2);
                } else {
                    Layout_SetElementValueState(layout, slotIndex, 2);
                }
                void* ret = Layout_GetElementSprite(layout, slotIndex);
                // Table x/y are scaled independently into the sprite's 2D size input.
                // The callee name does not establish these as world coordinates.
                float f1 = f30 * marker->x;
                float f0v = f31 * marker->y;
                struct {
                    float x;
                    float y;
                } vec;
                vec.x = f1;
                vec.y = f0v;
                Sprite_SetTextureRegionSize2D(ret, &vec);
                Layout_SetElementEnabled(layout, slotIndex, 1);
            }
            slotIndex++;
            connectionByteOffset += 4;
        }
    }
}

extern signed char s_resourceTableInitialized;
extern int s_resourceTableStorage;
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[];
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern signed char s_itemRequestStateInitialized;
extern char s_itemRequestState[];
extern void* g_soundManager;
extern float kFloat_0_5_MenuGroup;
extern float kFloat_200_TourAreaSelect;
extern void* tourAreaPointGroupNames[];
extern int s_tourAreaRevealScriptNames[];
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12out_of_range[];
void Allocation_Dtor();
void FadeOverlay_Init(void*);
void ResourceState_Dtor();
int FadeState_GetState(void*);
void FadeState_RequestFramebufferCapture(void*);
void FadeState_StartFadeIn(void*, int);
void InputManager_Init(void*);
void InputManager_Dtor();
int Input_IsActionPressed(void*, int, int);
void ItemRequestState_Ctor(void*);
void ItemRequestState_Dtor();
int ItemRequestState_GetEntryCount(void*);
int ItemRequestState_IsComplete(void*);
int ResourceTable_FindEntryByName(void*, const char*, int*);
int TourAreaSelect_PollResourcesAndCreateUi(void*);
void TourAreaSelect_StartDisplayAndFadeOut(void*);
int BpObject_PollLoad(void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
int Layout_PollLoadComplete(void*);
void Layout_CollectHovered(void*, void*, int);
void Layout_SetElementFeedbackState(void*, int, int);
void BpObject_StartScriptByName(void*, int, int);
int BpObject_IsCurrentScriptComplete(void*);
void RBTree_RebalanceAfterErase(void*, void*);
void appFreeUni__FPv(void*);
void* __nwa__FUl(unsigned long);
void __dla__FPv(void*);
void* __nw__FUl(unsigned long);
void __dt__Q23std12out_of_rangeFv();
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);

struct TextureResource {
    char data[0x20];
    // Initialize a texture archive resource with its resolved size and handle.
    TextureResource(const char* name, int size, int handle) {
        ((CPackTextureResource*)this)->Init((const char*)name, (void**)size, (unsigned int)handle);
    }
};

struct DtorRegistrations {
    char bp[12];
    char input[12];
    char fadeOverlay[12];
    char resource[12];
    char queue[12];
};

// Return the lazy BP controller; the singleton retains ownership.
static inline void* GetBp802FA3EC(DtorRegistrations* regmem) {
    if (((volatile void*&)Sing_805DC5F0()) == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80522860);
            s_bpCtrlInitialized = 1;
        }
        ((volatile void*&)Sing_805DC5F0()) = s_bpCtrl;
    }
    return (void*)((volatile void*&)Sing_805DC5F0());
}

// Return the lazy fade overlay and register its module-lifetime cleanup once.
static inline void* GetFadeOverlay(DtorRegistrations* regmem) {
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, (regmem->fadeOverlay));
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    return Sing_805DC608();
}

// Return the lazy input manager; callers borrow it.
static inline void* GetInput802FA3EC(DtorRegistrations* regmem) {
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, regmem->input);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    return GetInputManagerSlot();
}

// Return the lazy item-request queue used to dispatch area rewards.
static inline void* GetItemRequestState(DtorRegistrations* regmem) {
    if (!Sing_805DC958()) {
        if (!s_itemRequestStateInitialized) {
            ItemRequestState_Ctor(s_itemRequestState);
            __register_global_object(s_itemRequestState, (void*)ItemRequestState_Dtor, (regmem->queue));
            s_itemRequestStateInitialized = 1;
        }
        Sing_805DC958() = s_itemRequestState;
    }
    return Sing_805DC958();
}

struct RefCountBlock {
    void* vtable;
    int strong, weak;
    char* text;
};
struct MessageBufferGuard {
    char* buffer;
    char* allocator;
    // Free an exception message unless its reference-counted control block took it.
    ~MessageBufferGuard() {
        if (buffer)
            __dla__FPv(buffer);
    }
};
struct ThrownException {
    void* vtable;
    char* text;
    RefCountBlock* control;
};
// Throw the hover-vector bounds exception with an owned copy of its message.
static inline void ThrowOutOfRange() {
    const char* message = TourAreaSelectStringPoolView().boundsError;
    int allocator;
    *(char*)&allocator = 0;
    ThrownException error;
    error.vtable = __vt__Q23std11logic_error;
    unsigned long length = strlen(message) + 1;
    char* allocated = (char*)__nwa__FUl(length * 2 - length);
    error.text = allocated;
    {
        MessageBufferGuard guard;
        guard.buffer = allocated;
        guard.allocator = (char*)&allocator;
        RefCountBlock* control = (RefCountBlock*)__nw__FUl(0x10);
        if (control) {
            control->strong = 1;
            control->weak = 1;
            control->vtable = s_sharedCharDeleterVtable;
            control->text = allocated;
        }
        error.control = control;
        guard.buffer = 0;
    }
    strcpy(error.text, message);
    error.vtable = __vt__Q23std12out_of_range;
    __throw(TourAreaSelectStringPoolView().boundsExceptionType, &error, (void*)__dt__Q23std12out_of_rangeFv);
}

struct HoverIdVector {
    int* data;
    volatile unsigned int size;
    unsigned int capacity;
    // Start an empty owned vector of hovered element IDs.
    HoverIdVector() : data(0), size(0), capacity(0) {}
    // Release the temporary hover collection, leaving layout ownership untouched.
    ~HoverIdVector() {
        if (data) {
            unsigned int oldSize = size;
            size = oldSize - oldSize;
            appFreeUni__FPv(data);
        }
    }
    // Return the first hover ID; require a nonempty collection or throw.
    int first() {
        if (!size)
            ThrowOutOfRange();
        return data[0];
    }
};

struct AreaRevealNode {
    AreaRevealNode* left;
    AreaRevealNode* right;
    unsigned int parent;
    int index;
    unsigned char* flags;
};
// Return the owner's cached named group, constructing/caching it from matching
// script entries on a miss (bp_object_sprite_resources.cpp). Returns null for absent bp or no matches;
// the group is borrowed until that BP owner's script/resource reset.
static inline void* GetNamedGroup(void* bp, void* name) {
    void* group;
    if (bp)
        group = BpObject_GetOrCreateNamedGroup(bp, name);
    else
        group = 0;
    return group;
}

// Poll both layouts once the controller loads have all reported ready.
static inline void PollLayouts(MenuGroup802FB4D0* menu, bool& ready) {
    Layout_UpdateExtended(menu->firstLayout);
    if (!Layout_PollLoadComplete(menu->firstLayout)) {
        ready = false;
    } else {
        Layout_UpdateExtended(menu->secondLayout);
        ready = Layout_PollLoadComplete(menu->secondLayout) != 0;
    }
}

struct Iterator802FA3EC {
    AreaRevealNode* node;
    // Wrap a borrowed tree node for identity comparison.
    Iterator802FA3EC(AreaRevealNode* p) : node(p) {}
    // Compare node identity without following tagged parent links.
    bool operator==(const Iterator802FA3EC& other) const {
        return node == other.node;
    }
};
// Walk to the in-order successor; parent bit 0 is a tag, not an address bit.
static inline AreaRevealNode* NextInOrderNode(AreaRevealNode* node) {
    AreaRevealNode* child = node->right;
    if (child) {
        while (child->left)
            child = child->left;
        return child;
    }
    AreaRevealNode* parent;
    while (node != (parent = (AreaRevealNode*)(node->parent & ~1u))->left)
        node = parent;
    return parent;
}

// Advance an affected traversal cursor, unlink the reveal node and free only that node.
static inline void EraseRevealNode(MenuGroup802FB4D0* menu, AreaRevealNode* node) {
    if (Iterator802FA3EC(menu->revealCursor) == Iterator802FA3EC(node))
        menu->revealCursor = NextInOrderNode(node);
    RBTree_RebalanceAfterErase(node, menu->treeRoot);
    appFreeUni__FPv(node);
}

// Advance the menu through resource loading, the new-area reveal queue, reward
// dispatch and area selection.
//
// Returns 0 while pending, -1 when cancelled and +1 once the confirmation fade
// has run. The reveal queue holds area indices and borrowed pointers to their
// persistent flags; a completed node is unlinked and freed, never its flags.
// The frame delta is part of the update interface and is not used here.
int MenuGroup802FB4D0::ProcessAreaSelection(float deltaSeconds) {
    DtorRegistrations* regmem = (DtorRegistrations*)s_bpCtrlDtorRegistration_80522860;
    int result = 0;
    switch (state) {
    case kAreaLoadArchives: {
        int size = 0;
        if (!Sing_805DC620()) {
            if (!s_resourceTableInitialized) {
                s_resourceTableStorage = 0;
                __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor, (regmem->resource));
                s_resourceTableInitialized = 1;
            }
            Sing_805DC620() = &s_resourceTableStorage;
        }
        int handle =
            ResourceTable_FindEntryByName(Sing_805DC620(), TourAreaSelectStringPoolView().textureArchive, &size);
        textureResource = new TextureResource(TourAreaSelectStringPoolView().textureArchive, size, handle);
        packedFile = CPackedFile::Create(TourAreaSelectStringPoolView().dataArchive);
        state = kAreaWaitResources;
        break;
    }
    case kAreaWaitResources:
        if (TourAreaSelect_PollResourcesAndCreateUi(this))
            state = kAreaWaitControllers;
        break;
    case kAreaWaitControllers: {
        int i;
        int* controller = &selectedArea;
        bool ready;
        // Controllers are polled in order; the layouts are polled exactly once
        // after them, including when entryCount is zero.
        for (i = 0; i < entryCount || (PollLayouts(this, ready), false); ++controller, ++i) {
            if (!BpObject_PollLoad(IdMap_Lookup(GetBp802FA3EC(regmem), controller[1]))) {
                ready = false;
                break;
            }
        }
        if (ready) {
            TourAreaSelect_StartDisplayAndFadeOut(this);
            state = kAreaWaitFade;
        }
        break;
    }
    case kAreaWaitFade: {
        int* controller = &selectedArea;
        for (int i = 0; i < entryCount; ++controller, ++i)
            BpObject_TryInvokeCurrentCallback(IdMap_Lookup(GetBp802FA3EC(regmem), controller[1]), 0);
        Layout_UpdateExtended(firstLayout);
        {
            HoverIdVector hovered;
            Layout_CollectHovered(firstLayout, &hovered, kAreaHoverMask);
            if (hovered.size)
                selectedArea = hovered.first();
        }
        for (int i = 0; i < kAreaCount; ++i) {
            Layout_SetElementValueState(firstLayout, i, 0);
            Layout_SetElementFeedbackState(firstLayout, i, 1);
        }
        if (selectedArea != kNoArea)
            Layout_SetElementValueState(firstLayout, selectedArea, 1);
        TourAreaSelect_RefreshLevelAndConnections(this);
        if (FadeState_GetState(GetFadeOverlay(regmem)) != kFadeBusy) {
            if (remaining)
                state = kAreaStartReveal;
            else
                state = kAreaDispatchRewards;
        }
        break;
    }
    case kAreaChoose: {
        Layout_UpdateExtended(firstLayout);
        {
            HoverIdVector hovered;
            Layout_CollectHovered(firstLayout, &hovered, kAreaHoverMask);
            if (hovered.size)
                selectedArea = hovered.first();
        }
        for (int i = 0; i < kAreaCount; ++i) {
            Layout_SetElementValueState(firstLayout, i, 0);
            Layout_SetElementFeedbackState(firstLayout, i, 1);
        }
        if (selectedArea != kNoArea)
            Layout_SetElementValueState(firstLayout, selectedArea, 1);
        TourAreaSelect_RefreshLevelAndConnections(this);
        void* input = GetInput802FA3EC(regmem);
        if (input) {
            if (Input_IsActionPressed(input, kActionConfirm, -1) && selectedArea != kNoArea) {
                Layout_SetElementValueState(firstLayout, selectedArea, 2);
                Layout_SetElementFeedbackState(firstLayout, selectedArea, 2);
                Sound_PlayByName(g_soundManager, TourAreaSelectStringPoolView().selectSound, 0, 0, 0, 0,
                                 kFloat_0_5_MenuGroup, kFloat_200_TourAreaSelect, kFloat_1_MenuGroup);
                state = kAreaStartConfirmFade;
            } else if (Input_IsActionPressed(input, kActionCancel, -1)) {
                FadeState_RequestFramebufferCapture(GetFadeOverlay(regmem));
                FadeState_StartFadeIn(GetFadeOverlay(regmem), 0);
                state = kAreaCancelled;
            }
        }
        int* controller = &selectedArea;
        for (int i = 0; i < entryCount; ++controller, ++i)
            BpObject_TryInvokeCurrentCallback(IdMap_Lookup(GetBp802FA3EC(regmem), controller[1]), 0);
        break;
    }
    case kAreaStartReveal: {
        if (!remaining) {
            int* controller = &selectedArea;
            for (int i = 0; i < entryCount; ++controller, ++i)
                BpObject_TryInvokeCurrentCallback(IdMap_Lookup(GetBp802FA3EC(regmem), controller[1]), 0);
            state = kAreaDispatchRewards;
            break;
        }
        int index = revealCursor->index;
        void* name = tourAreaPointGroupNames[index];
        void* bp = IdMap_Lookup(GetBp802FA3EC(regmem), controllerIds[0]);
        void* group = GetNamedGroup(bp, name);
        UIGroup_SetEnabled(group, 1);
        BpObject_StartScriptByName(IdMap_Lookup(GetBp802FA3EC(regmem), controllerIds[0]),
                                   s_tourAreaRevealScriptNames[index], 1);
        Sound_PlayByName(g_soundManager, TourAreaSelectStringPoolView().openAreaSound, 0, 0, 0, 0, kFloat_0_5_MenuGroup,
                         kFloat_200_TourAreaSelect, kFloat_1_MenuGroup);
        state = kAreaWaitReveal;
        // The first animation update belongs to this state as well.
    }
    case kAreaWaitReveal: {
        int* controller = &selectedArea;
        for (int i = 0; i < entryCount; ++controller, ++i)
            BpObject_TryInvokeCurrentCallback(IdMap_Lookup(GetBp802FA3EC(regmem), controller[1]), 0);
        if (!BpObject_IsCurrentScriptComplete(IdMap_Lookup(GetBp802FA3EC(regmem), controllerIds[0])))
            break;
        *revealCursor->flags |= kAreaRevealFinished;
        Layout_SetElementPlayerFlagsAndState(firstLayout, revealCursor->index, 1);
        EraseRevealNode(this, revealCursor);
        if (--remaining)
            state = kAreaStartReveal;
        else
            state = kAreaDispatchRewards;
        break;
    }
    case kAreaDispatchRewards: {
        int* controller = &selectedArea;
        for (int i = 0; i < entryCount; ++controller, ++i)
            BpObject_TryInvokeCurrentCallback(IdMap_Lookup(GetBp802FA3EC(regmem), controller[1]), 0);
        if (ItemRequestState_GetEntryCount(GetItemRequestState(regmem)) > 0) {
            ((ItemRequestState80300EC8*)GetItemRequestState(regmem))->dispatchEnabled = 1;
            state = kAreaWaitRewards;
        } else
            state = kAreaChoose;
        break;
    }
    case kAreaWaitRewards: {
        int* controller = &selectedArea;
        for (int i = 0; i < entryCount; ++controller, ++i)
            BpObject_TryInvokeCurrentCallback(IdMap_Lookup(GetBp802FA3EC(regmem), controller[1]), 0);
        if (ItemRequestState_IsComplete(GetItemRequestState(regmem))) {
            ((ItemRequestState80300EC8*)GetItemRequestState(regmem))->dispatchEnabled = 0;
            state = kAreaChoose;
        }
        break;
    }
    case kAreaCancelled:
        BpObject_SetEnabled(IdMap_Lookup(GetBp802FA3EC(regmem), controllerIds[0]), 0);
        BpObject_SetEnabled(IdMap_Lookup(GetBp802FA3EC(regmem), controllerIds[1]), 0);
        result = -1;
        break;
    case kAreaStartConfirmFade: {
        int* controller = &selectedArea;
        for (int i = 0; i < entryCount; ++controller, ++i)
            BpObject_TryInvokeCurrentCallback(IdMap_Lookup(GetBp802FA3EC(regmem), controller[1]), 0);
        FadeState_RequestFramebufferCapture(GetFadeOverlay(regmem));
        FadeState_StartFadeIn(GetFadeOverlay(regmem), 0);
        if (selectedArea != kNoArea) {
            Layout_SetElementValueState(firstLayout, selectedArea, 1);
            Layout_SetElementFeedbackState(firstLayout, selectedArea, 1);
        }
        state = kAreaConfirmed;
        break;
    }
    case kAreaConfirmed:
        BpObject_SetEnabled(IdMap_Lookup(GetBp802FA3EC(regmem), controllerIds[0]), 0);
        BpObject_SetEnabled(IdMap_Lookup(GetBp802FA3EC(regmem), controllerIds[1]), 0);
        result = 1;
        break;
    }
    return result;
}

// Guard the local views used by this unit against accidental layout changes.
#define POLISH_OFFSET(T, F, O) typedef char polish_##T##_##F[((int)&((T*)0)->F == (O)) ? 1 : -1]
POLISH_OFFSET(AreaRegisteredObject, enabled, 0x28);
#undef POLISH_OFFSET

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804625A4", _ctors_u802FA3EC_0, TourAreaSelect_InitRotationPeriod);
#pragma pop
