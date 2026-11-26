// The item panel HUD (CItemWindow): the panel that shows the player's eight equipped item slots,
// plays the item-use animation, and consumes an item when one is selected.
//
// Also holds the storage helpers and the item-message handling for the window.

#include <decomp/static_init.h>
// In-round item panel / HUD consumer (0x80190474): CItemWindow loads the BP
// animation/layout, shows eight equipped consumable slots, consumes an item,
// and animates its icon toward the current player's projected head bone.
// Mod lookup (all float definitions are in the data section):
// - Widescreen placement: CItemWindow::OnLoad uses COption::hudBaselineY - 25,
//   overlay X=74 (kItemOverlayLeft74), hidden panel X=-384 (kItemPanelHiddenX).
//   OnProcess adds panelOffsetX; OnInit caches layout position minus ORIGIN.
//   OnDisplay writes screen/layout-pixel rectangles from slotOrigins[count-1].
//   Keep BP/layout coordinates, pointer hit tests and world->view->screen head
//   projection in the same viewport mapping. This file sets no aspect/scissor.
// - Icon pitch/size: kItemIconSpacing49=49 pixels; normal/hover widths are
//   0.9/1.05 of layout width. kItemAnimationIconSize is 44.099998 pixels square.
//   kItemIconUvExtent=0.65625 is a normalized atlas extent, not a pixel width.
//   Resizing/replacing textures must preserve the atlas region or change UVs too.
// - Motion/frame rate: kItemAnimationFramesPerSecond=10, homing delay=0.8 seconds,
//   gravity coefficient=6, initial Y step=-3, homing gain=0.5. Integrate elapsed
//   seconds but add velocity to pixels ONCE PER UPDATE; a 60 Hz mod must treat
//   this separately from display presentation. Frame-byte equality at 8 ends
//   the alternate animation; preserve the existing skipped-frame behavior.
// - Gameplay: +33 / cap99 changes the Pangya Combo Gauge (entry->powerGauge);
//   these are separate from icon scale and the 0..100 failure-percent roll.
// - Shared 200: Sound_PlayByName's SECOND float is attenuationRange in the
//   position vector's distance units (SoundManager_CreateSoundInstance,
//   sound_manager.cpp). All four local calls use null position/type 0, so attenuation
//   is bypassed: editing 200 does not change these nonspatial sounds' volume.
//   ResetSelection also writes 200 to +70/+74 (the eight bytes at +70 in itemwindow.h), but
//   no source reader of either word is established. They are initialization
//   storage, not the drawn animationX/Y (+7C/+80) or an opacity/resolution knob.
//   Reset units/effect remain unknown; isolate roles before a future tuning mod.
// Owned resources: overlay tree payloads/window overlay via ResourceManager,
// controllerId via CBpCtrl, 11 CBpTex elements via __destroy_new_array, layout
// via Layout_Destroy. OnDestroy releases them; borrowed inventory/player/head
// pointers remain owned by their services. Required self for all actor methods.
// Golf document/current player and dispatch targets must exist once loaded.
// OnLoad blocks polling until all resources report ready; it does not yield or
// recover allocation failures. Later processing/drawing return early until ready.
// Tests for a later mod: tutorial before/after stage6, empty/full eight-slot stock,
// open/close transitions, hover edges, failed item use, and head-target crossing.
extern "C" {
extern unsigned char s_itemIconNamesAndQuads[]; // Item icon names and quad templates.
}
#include "classes/COption.h"
#include "classes/CMouseCursor.h"
#include "iff/IffStruct.h"
#include "classes/ItemOverlayTreeNode.h"
#include "classes/InputPointerState.h"
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#include "classes/WView.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "game/itemwindow.h"
#include "sdk_protos.h"
#include "game/types.h"
#include "game/singletons.h"
#include "game/wvector.h"
#include "classes/CBpCtrl.h"

enum ItemPanelState {
    kPanelOpen = 0,
    kPanelOpening = 1,
    kPanelClosing = 2,
    kPanelClosed = 3
};
// These constituent bits mirror GolfRuleState, not ItemPanelState (0..3).
enum ItemPanelVisibleGolfState {
    kPanelDuringAim = 0x8,
    kPanelDuringCameraView = 0x100,
    kPanelDuringCameraTransition = 0x200,
    kPanelDuringBeginPlayerTurn = 0x4000,
    kPanelDuringCameraReturn = 0x40000
};
enum ItemWindowConstants {
    kItemSlotCount = 8,
    kItemTextureCount = 11,
    kIffConsumableKind = 6,
    kIffKindShift = 26,
    kGreenGroundType = 2,
    kPercentRange = 100,
    kFailedItemReplacementIndex = 14,
    kFirstItemTutorialStage = 6,
    kItemAnimationState = 8,
    kAlternateItemEndFrame = 8,
    kItemLayoutAllocationBytes = 0x5C,
    kItemLayoutMode = 5,
    kItemTextureArrayBytes = 0x1C8,
    kItemTextureObjectBytes = 0x28,
    kItemSlotFlagSelector = 5,
    kItemQuadVertexCount = 4,
    kItemQuadPrimitive = 0x98,
    kItemQuadVertexFormat = 0x11F005,
    kItemQuadStride = 0x18,
    kPanelVisibilityStateMask = kPanelDuringAim | kPanelDuringCameraView | kPanelDuringCameraTransition |
        kPanelDuringBeginPlayerTurn | kPanelDuringCameraReturn
};
// Item actor message IDs. Query messages write through caller-owned int pointers;
// activation/use messages carry immediate slot/owner/ID/random words instead.
enum ItemWindowMessage {
    kItemMsgRefreshVisibility = 0,
    kItemMsgBeginOpen = 1,
    kItemMsgBeginClose = 2,
    kItemMsgQueryVisible = 3,
    kItemMsgRequestDraw = 4,
    kItemMsgQueryPanelState = 5,
    kItemMsgEnableProcess = 6,
    kItemMsgDisableProcess = 7,
    kItemMsgNoOp = 8,
    kItemMsgActivateSlot = 9,
    kItemMsgUseForOwner = 10,
    kItemMsgResetSelection = 11,
    kItemMsgQueryNoActiveSelection = 12,
    kItemMsgQueryPanelTransitionState = 13
};
enum ItemWindowScreenQuery {
    kScreenQueryHudPhase = 0x30,
    kScreenQueryItemPanelReady = 0x43
};
// UI map: layout elements 0..7 are the eight slot controls. Selector 5 is used
// both to publish slot availability and to collect hovered slots. Named count
// groups use 1..8 in kItemCountGroupFormat; exactly the current count is enabled.
enum ItemWindowStringOffset {
    kItemWindowScript = 0x0E,
    kItemWindowLayout = 0x21,
    kPlayerActor = 0x34,
    kHeadBone = 0x3B,
    kScreenActor = 0x46,
    kItemCountGroupFormat = 0x4D,
    kPanelInScript = 0x59,
    kPanelOpenSound = 0x65,
    kPanelOutScript = 0x74,
    kPanelCloseSound = 0x81,
    kVectorIndexError = 0x91,
    kVectorIndexErrorType = 0xAF,
    kGolfRuleActor = 0xE6,
    kGuideLineActor = 0xEF,
    kItemFailureSound = 0xF9,
    kItemUseSound = 0x108
};
// Each view begins at the shared-document base plus a player stride; it is not a CGolfPlayer actor.
struct SharedPlayerItemSlotsView {
    char padding00[0x8C];
    unsigned int itemIds[kItemSlotCount];
    char paddingAC[0x128];
};
enum ItemLayoutOffset {
    kSharedItemIdsOffset = (int)&((SharedPlayerItemSlotsView*)0)->itemIds,
    kItemOriginTableOffset = (int)&((CItemWindow*)0)->drawRequested
};
typedef char SharedPlayerItemSlots_size[sizeof(SharedPlayerItemSlotsView) == 0x1D4 ? 1 : -1];

// The actor's head-bone result contains an affine world matrix at +0x78.
struct ItemTargetBoneView {
    char boneState[0x78];
    WVector worldAxes[3];
    WVector worldPosition;
};
typedef char ItemTargetBonePosition_offset[IFF_OFFSET(ItemTargetBoneView, worldPosition) == 0x9C ? 1 : -1];

extern "C" {

extern const float kFloat_6_2831855;
extern float s_itemWindowFullTurnAngle;

void appFreeUni__FPv(void*);
void* __nw__FUl(unsigned int);
void __dl__FPv(void*);

struct Obj_80190474 {
    void* ptr;
    void* clearedStorageWord;
};

// Release a raw container allocation after clearing its used-storage word.
inline void ReleaseStorage(Obj_80190474* storage) {
    if (storage && storage->ptr) {
        storage->clearedStorageWord = 0;
        appFreeUni__FPv(storage->ptr);
    }
}

extern const float kFloat_0_itemWindow;
extern char __vt__11CItemWindow[];
extern void __ct__18NativeBase80201D18Fv(void*);
struct RoundPlayerItemState {
    char padding00[8];
    int state;
    char padding0C[0x8C];
    void* owner;
    unsigned char itemCount;
    char padding9D[3];
    unsigned int activeItemId;
    char paddingA4[4];
    float powerGauge;
};

extern CSharedDoc* g_sharedDoc;
RoundPlayerItemState* GolfPlayer_GetEntryByIndex(void*, unsigned char);
void Layout_SetElementPlayerFlag(void*, int, int, int);
int NoticeQueue_ShowItemNotice(void*, unsigned int);
extern void* g_resourceManager;
extern void* g_option;
extern signed char s_bpCtrlInitialized;
extern int s_itemWindowState;
extern const float kItemPanelBaselineInset25, kItemOverlayLeft74, kItemPanelHiddenX;
extern const float kFloat_200_ItemWindowShared;
extern const char itemWindowStrings[];
extern const char kItemWindowIconNames[];
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_80190474[];
int Layout_PollLoadComplete(void*);
void Layout_GetElementPosition(void*, int, float*);
void Layout_GetElementOrigin(void*, int, float*); // Copy the selected layout element's sprite-origin offset.
void* WOverlay_Create(void*, const char*, int);
int WOverlay_GetWidth(void*);
int WOverlay_GetHeight(void*);
void Sprite_NoOpHook(void*, float*);
void CBpCtrl_dtor();
void* Layout_Init(void*, int);
void* Layout_LoadExtended(void*, int, const char*);
void* __nwa__FUl(unsigned int);
extern "C" void __ct__6CBpTexFv();
extern "C" void __dt__6CBpTexFv();
void Layout_Update(void*);
void* IdMap_Lookup(void*, void*);
int BpObject_PollLoad(void*);
void Object_DeleteVirtual(void*, void*);
void OverlayMapTree_FreeSubtree(void*, void*);
void Layout_Destroy(void*, int);
void __destroy_new_array(void*, void*);

extern void* g_view;
extern void* g_inputPointer;
extern void* g_helpMsg;
extern const float kItemAnimationFramesPerSecond, kItemAnimationHomingDelay, kItemAnimationGravity6,
    kItemAnimationHeadOffsetY;
extern const float kFloat_0_5_ItemWindow;
extern void Layout_SetElementPlayerFlagsAndState(void*, int, int);
extern void IActor_DispatchMessage(void*, void*, const char*, int, void*, const char*, int, int);
extern void* Matrix_TransformPoint(void*, void*, void*);
extern void WView_ProjectViewPoint(void*, void*, void*, int);

extern unsigned char s_itemWindowTypeDescriptor;
extern void* g_soundManager;

extern void* BpObject_GetOrCreateNamedGroup(void*, const char*);
extern void UIGroup_SetEnabled(void*, int);
extern void BpObject_StartScriptByName(void*, const char*, int);
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern void* ItemInventory_FindItemData(void*, void*);
extern unsigned int ItemInventory_GetItemIdAt(void*, int);
extern void* InstanceTable_Get();
extern int BpObject_IsCurrentScriptComplete(void*);
extern int GolfTask_NoOpStatus(void*);
extern unsigned int ItemInventory_FindItemIndex(void*, unsigned int);
extern unsigned int* GolfBall__Fv();
extern unsigned int CQuadTree_GetInstance();
extern unsigned int GetGroundType__9CQuadTreeF7WVector(void*);
extern void* NoticeQueue_ShowById(void*, int, int);
extern void* g_mouseCursor;
extern const float kItemIconUvExtent, kFloat_1_ItemWindow, kItemIconSpacing49, kItemIconNormalScale,
    kItemIconHoveredScale, kItemAnimationIconSize, kItemAnimationInitialVelocityY, kItemPowerGaugeIncrease33,
    kItemPowerGaugeMaximum99;
extern char itemWindowDrawData[];
extern char s_gameInputManager[];
extern void BpObject_TryInvokeCurrentCallback(void*, int);
extern void BpObject_Draw(void*);
extern void Layout_DrawWithOpacity(void*, float);
extern void Layout_GetElementSize(void*, int, float*);
extern void Layout_SetElementPosition(void*, int, float*);
extern void Layout_WriteElementRect(void*, int, int*);
extern void InputManager_Init(void*);
extern void InputManager_Dtor();
extern void InputManager_GetPointerPosition(void*, float*, int);
extern void Engine_SetFlexibleVertexFormat(int);
extern void* GX_GetPrimitiveCount(int, int);
extern void DrawPrimitiveUP(int, void*, void*, int);
extern void* memcpy(void*, const void*, unsigned int);

// Borrowed message payload ABI: type 0 refreshes visibility; 1/2 open/close;
// 3 writes visible(int*) through value; 4 requests drawing; 5/13 write panel state;
// 6/7 enable/disable processing; 8 does nothing; 9 uses value as slot index;
// 10 uses value as owner pointer plus integer itemId/randomValue carried as pointers;
// 11 resets selection; 12 writes whether no item is active or selected.
// Output pointers and slot indices are caller-validated, never retained here.
// Payload use by message (all addresses/tokens are 32-bit Gekko words):
// 3/5/12/13: value=int* output; 9: value=zero-based shared inventory slot;
// 10: value=borrowed owner, itemId=immediate IFF ID, randomValue=immediate RNG token.
// Other messages ignore all three words. Query-visible includes opening/closing;
// state queries return ItemPanelState (open/opening/closing/closed = 0/1/2/3).
// No payload pointer is owned by this message or retained by HandleMsg.
struct Message {
    char padding00[4];
    unsigned int type;
    unsigned int value;
    void* itemId;
    void* randomValue;
};
typedef char ItemMessage_value_offset[IFF_OFFSET(Message, value) == 8 ? 1 : -1];
typedef char ItemMessage_random_offset[IFF_OFFSET(Message, randomValue) == 0x10 ? 1 : -1];
extern char
    s_inputManagerDtorRegistration_ItemWindow[]; // Destructor-registration record for this unit's input manager.
extern signed char s_inputManagerInitialized;
struct ItemNames {
    void* name[11];
};
struct ItemVertex {
    float x, y, z;
    unsigned int color;
    float u, v;
};
struct ItemQuad {
    ItemVertex v[kItemQuadVertexCount];
};
typedef char ItemVertex_stride[sizeof(ItemVertex) == kItemQuadStride ? 1 : -1];
struct ItemTexture {
    // CBpTex ABI view; only the called slots promise these signatures.
    // CBpTex slots: +08 uncalled destructor, +0C borrowed C-string address word, +10 uncalled by this unit, +14 bind
    // the loaded icon for quad drawing.
    virtual void a();                        // +08: uncalled destructor slot, do not invoke as void()
    virtual void setTexturePathAddress(int); // +0C: borrowed C-string address word
    virtual void c();                        // +10: uncalled by this unit
    virtual void bind();                     // +14: bind loaded icon for subsequent quad drawing
    virtual int ready();
    // +18: nonzero when texture loading has completed
    char pad[0x24];
};

struct Arr3u {
    unsigned int v[3];
};
struct IntVec {
    int* data;
    unsigned int size;
    int capacity;
    // Free the owned layout-ID array after clearing its logical size.
    ~IntVec() {
        if (data) {
            size -= size;
            appFreeUni__FPv(data);
        }
    }
};
extern int Layout_CollectHovered(void*, IntVec*, int);
extern int Layout_CollectActivated(void*, IntVec*, void*);
extern void __dla__FPv(void*);
extern void __dt__Q23std12out_of_rangeFv();
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12out_of_range[];

struct ErrRef {
    void* vt;
    int c1;
    int c2;
    char* s;
};
struct ErrHolder {
    char* p;
    char* flag;
    // Guard raw exception-message bytes until their deleter is installed.
    ErrHolder(char* b, char* f) {
        p = b;
        flag = f;
    }
    // Delete message bytes if construction failed before ownership transfer.
    ~ErrHolder() {
        if (p)
            __dla__FPv(p);
    }
};
struct ErrBase {
    void* vt;
    char* buf;
    ErrRef* ref;
    // Copy exception text and attach a reference-counted deleter.
    ErrBase(const char* msg0) {
        const char* msg = msg0;
        vt = __vt__Q23std11logic_error;
        char done __attribute__((aligned(4))) = 0;
        unsigned int n = strlen(msg) + 1;
        char* buffer = (char*)__nwa__FUl(n * 2 - n);
        buf = buffer;
        {
            ErrHolder h(buffer, &done);
            ErrRef* r = (ErrRef*)__nw__FUl(16);
            if (r) {
                r->c1 = 1;
                r->c2 = 1;
                r->vt = s_sharedCharDeleterVtable;
                r->s = buffer;
            }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, msg);
    }
    // No additional local cleanup; thrown-object cleanup uses the external destructor.
    ~ErrBase() {}
};
struct OutOfRange : ErrBase {
    // Construct and throw the vector-index exception using the original ABI.
    OutOfRange(const char* msg) : ErrBase(msg) {
        vt = __vt__Q23std12out_of_range;
        __throw(itemWindowStrings + kVectorIndexErrorType, this, (void*)__dt__Q23std12out_of_rangeFv);
    }
    // No derived-owned storage requires additional cleanup.
    ~OutOfRange() {}
};

// Functions in original address order.

// Clear and release this raw storage specialization, then optionally delete it.
Obj_80190474* dtor_80190474(Obj_80190474* self, int flag) {
    if (self != 0) {
        self->clearedStorageWord = 0;
        ReleaseStorage(self);
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Clear and release this raw storage specialization, then optionally delete it.
Obj_80190474* dtor_801904E8(Obj_80190474* self, int flag) {
    if (self != 0) {
        self->clearedStorageWord = 0;
        ReleaseStorage(self);
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Clear and release this raw storage specialization, then optionally delete it.
Obj_80190474* dtor_8019055C(Obj_80190474* self, int flag) {
    if (self != 0) {
        self->clearedStorageWord = 0;
        ReleaseStorage(self);
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Clear and release this raw storage specialization, then optionally delete it.
Obj_80190474* dtor_801905D0(Obj_80190474* self, int flag) {
    if (self != 0) {
        self->clearedStorageWord = 0;
        ReleaseStorage(self);
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Clear and release this raw storage specialization, then optionally delete it.
Obj_80190474* dtor_80190644(Obj_80190474* self, int flag) {
    if (self != 0) {
        self->clearedStorageWord = 0;
        ReleaseStorage(self);
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Clear and release this raw storage specialization, then optionally delete it.
Obj_80190474* dtor_801906B8(Obj_80190474* self, int flag) {
    if (self != 0) {
        self->clearedStorageWord = 0;
        ReleaseStorage(self);
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Clear and release this raw storage specialization, then optionally delete it.
Obj_80190474* dtor_8019072C(Obj_80190474* self, int flag) {
    if (self != 0) {
        self->clearedStorageWord = 0;
        ReleaseStorage(self);
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Clear and release this raw storage specialization, then optionally delete it.
Obj_80190474* dtor_801907A0(Obj_80190474* self, int flag) {
    if (self != 0) {
        self->clearedStorageWord = 0;
        ReleaseStorage(self);
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Initialize the item window's cached full-turn angle.
void InitItemWindowFullTurnAngle() {
    s_itemWindowFullTurnAngle = kFloat_6_2831855;
}

// Allocate and construct an item-window actor.
// Registered as the ItemWindow factory by CItemWindow_RegisterType
// (sun_rays_lens_flare_effects.cpp); the caller/actor registry owns the returned allocation, not this
// factory. Construction creates empty handles; OnLoad acquires resources and
// OnInit establishes selection state. Call CItemWindow::OnDestroy before the
// actor's deleting destructor CItemWindow_dtor(self,1) (sun_rays_lens_flare_effects.cpp): that destructor
// frees remaining overlay-tree nodes, delegates to the base and frees self;
// it does not release the load-time handles. OnDestroy releases overlay/tree, BP entry,
// texture array and layout. Do not free the raw pointer or its handles separately.
// Uses ordinary new; allocation/constructor failure follows that API, with no
// factory status, fallback actor or resource-load readiness guarantee.
CItemWindow* CItemWindow_Create() {
    return new CItemWindow;
}

// Initialize the item-window actor, overlay tree, layout handle, and slot origins.
CItemWindow::CItemWindow() {
    __ct__18NativeBase80201D18Fv(this);
    float zero = kFloat_0_itemWindow;
    void* z = 0;

    vtable = (void*)__vt__11CItemWindow;
    *(void**)pad_18 = z;
    overlayRoot = z;
    overlayFirst = &overlayRoot;
    overlay = z;
    active = 1;
    displayPriority = 4;
    itemTextures = z;
    itemLayout = z;
    controllerId = -1;
    slotOrigins[0][1] = zero;
    slotOrigins[0][0] = zero;
    slotOrigins[1][1] = zero;
    slotOrigins[1][0] = zero;
    slotOrigins[2][1] = zero;
    slotOrigins[2][0] = zero;
    slotOrigins[3][1] = zero;
    slotOrigins[3][0] = zero;
    slotOrigins[4][1] = zero;
    slotOrigins[4][0] = zero;
    slotOrigins[5][1] = zero;
    slotOrigins[5][0] = zero;
    slotOrigins[6][1] = zero;
    slotOrigins[6][0] = zero;
    slotOrigins[7][1] = zero;
    slotOrigins[7][0] = zero;
}

// Free deeper overlay nodes before this node; overlay payloads were released in phase 1.
static inline void ReleaseItemOverlayLeaf(CItemWindow* self, void* node) {
    if (node) {
        if (*(void**)node)
            OverlayMapTree_FreeSubtree(&self->pad_18, *(void**)node);
        if (((ItemOverlayTreeNode*)node)->right)
            OverlayMapTree_FreeSubtree(&self->pad_18, ((ItemOverlayTreeNode*)node)->right);
        appFreeUni__FPv(node);
    }
}

// Release every item overlay, tree node, script entry, texture, and layout.
// Release order (construction order reversed): per-node overlays -> overlay tree nodes ->
// the window's own overlay -> controller script entry (controllerId) -> icon texture array
// (itemTextures) -> layout (itemLayout). Each owner pointer is zeroed after release, so the
// function is safe to call when some parts were never loaded.
void CItemWindow::OnDestroy() {
    void* treeSentinel;
    void* node;
    void* ownedOverlay;
    void* successor;
    void* packedParent;
    void* parentNode;
    void* parentLeftChild;
    void* depth3Node;
    void* depth2Node;
    void* depth1Node;
    void* rootNode;
    // Unused declarators retain CodeWarrior's anonymous constant/EH symbol
    // numbering. Removing them leaves code bytes equal but changes relocation
    // symbol spellings; keep the original eight slots for a strict object check.
    void* unusedTreeScratch0;
    void* unusedTreeScratch1;
    void* unusedTreeScratch2;
    void* unusedTreeScratch3;
    void* unusedTreeScratch4;
    void* unusedTreeScratch5;
    void* unusedTreeScratch6;
    void* unusedTreeScratch7;
    void* ownedTextures;
    void* ownedLayout;
    void* emptySentinel;

    // Phase 1: visit every overlay payload and release through the resource manager.
    // Tree nodes remain allocated during this traversal; parent low bit is color.
    treeSentinel = &overlayRoot;
    node = overlayFirst;
    while (node != treeSentinel) {
        if (g_resourceManager) {
            ownedOverlay = ((ItemOverlayTreeNode*)node)->overlay;
            if (ownedOverlay) {
                Object_DeleteVirtual(g_resourceManager, ownedOverlay);
                ((ItemOverlayTreeNode*)node)->overlay = 0;
            }
        }
        successor = ((ItemOverlayTreeNode*)node)->right;
        if (successor) {
            while (*(void**)successor)
                successor = *(void**)successor;
            node = successor;
            continue;
        }
        while (1) {
            packedParent = (*(void**)&((ItemOverlayTreeNode*)node)->parentAndColor);
            parentNode = (void*)((unsigned long)packedParent & ~(unsigned long)1);
            parentLeftChild = *(void**)parentNode;
            if (node == parentLeftChild) {
                node = parentNode;
                break;
            }
            node = parentNode;
        }
    }
    // Phase 2: free tree nodes, then restore the empty-tree sentinel at overlayRoot.
    // The nested if-blocks below are an unrolled post-order free of a binary tree (left successor
    // at +0, right successor at +4 of each node) to a fixed depth of four levels from `rootNode`;
    // anything deeper is handed to OverlayMapTree_FreeSubtree. It is equivalent to one call
    // OverlayMapTree_FreeSubtree(&pad_18, overlayRoot) followed by freeing the root.
    rootNode = overlayRoot;
    if (rootNode) {
        depth1Node = *(void**)rootNode;
        if (depth1Node) {
            depth2Node = *(void**)depth1Node;
            if (depth2Node) {
                depth3Node = *(void**)depth2Node;
                ReleaseItemOverlayLeaf(this, depth3Node);
                depth3Node = ((ItemOverlayTreeNode*)depth2Node)->right;
                ReleaseItemOverlayLeaf(this, depth3Node);
                appFreeUni__FPv(depth2Node);
            }
            depth2Node = ((ItemOverlayTreeNode*)depth1Node)->right;
            if (depth2Node) {
                depth3Node = *(void**)depth2Node;
                ReleaseItemOverlayLeaf(this, depth3Node);
                depth3Node = ((ItemOverlayTreeNode*)depth2Node)->right;
                ReleaseItemOverlayLeaf(this, depth3Node);
                appFreeUni__FPv(depth2Node);
            }
            appFreeUni__FPv(depth1Node);
        }
        depth1Node = ((ItemOverlayTreeNode*)rootNode)->right;
        if (depth1Node) {
            depth2Node = *(void**)depth1Node;
            if (depth2Node) {
                depth3Node = *(void**)depth2Node;
                ReleaseItemOverlayLeaf(this, depth3Node);
                depth3Node = ((ItemOverlayTreeNode*)depth2Node)->right;
                ReleaseItemOverlayLeaf(this, depth3Node);
                appFreeUni__FPv(depth2Node);
            }
            depth2Node = ((ItemOverlayTreeNode*)depth1Node)->right;
            if (depth2Node) {
                depth3Node = *(void**)depth2Node;
                ReleaseItemOverlayLeaf(this, depth3Node);
                depth3Node = ((ItemOverlayTreeNode*)depth2Node)->right;
                ReleaseItemOverlayLeaf(this, depth3Node);
                appFreeUni__FPv(depth2Node);
            }
            appFreeUni__FPv(depth1Node);
        }
        appFreeUni__FPv(rootNode);
        (*(void**)&pad_18) = 0;
        overlayRoot = 0;
        emptySentinel = &overlayRoot;
        overlayFirst = emptySentinel;
    }
    if (g_resourceManager) {
        ownedOverlay = overlay;
        if (ownedOverlay) {
            Object_DeleteVirtual(g_resourceManager, ownedOverlay);
            overlay = 0;
        }
    }
    // Phase 3: delete the script entry, constructed texture array, and owned layout.
    if (controllerId != -1) {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80190474);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(controllerId);
        controllerId = -1;
    }
    ownedTextures = itemTextures;
    if (ownedTextures) {
        __destroy_new_array(ownedTextures, (void*)__dt__6CBpTexFv);
        itemTextures = 0;
    }
    ownedLayout = itemLayout;
    if (ownedLayout) {
        Layout_Destroy(ownedLayout, 1);
        itemLayout = 0;
    }
}

// Load the item panel and its icon textures once the tutorial enables items.
// Controls HUD placement (pixels from top-left, Y downward); preserve baseline,
// hidden-X/script offset and slot-origin relationships when changing aspect ratio.
void CItemWindow::OnLoad() {
    if (AsCGolfDoc((*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc))->tutorialStage < kFirstItemTutorialStage)
        return;
    overlay = WOverlay_Create(g_resourceManager, itemWindowStrings, 0);
    float hudBaselineY = AsCOption(g_option)->hudBaselineY;
    if (overlay) {
        float overlayY = kItemPanelBaselineInset25;
        float overlayX = kItemOverlayLeft74;
        overlayY = hudBaselineY - overlayY;
        overlayBounds[0] = overlayX;
        overlayBounds[1] = overlayY;
        overlayBounds[2] = (float)WOverlay_GetWidth(overlay);
        overlayBounds[3] = (float)WOverlay_GetHeight(overlay);
        Sprite_NoOpHook(overlay, overlayBounds);
    }
    bounds[0] = kItemPanelHiddenX;
    bounds[1] = hudBaselineY - kItemPanelBaselineInset25;
    bounds[2] = overlayBounds[2];
    bounds[3] = overlayBounds[3];
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80190474);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())->CreateEntry(itemWindowStrings + kItemWindowScript, &controllerId, 0);
    void* obj = __nw__FUl(kItemLayoutAllocationBytes);
    if (obj)
        Layout_Init(obj, kItemLayoutMode);
    itemLayout = obj;
    Layout_LoadExtended(obj, 0, itemWindowStrings + kItemWindowLayout);
    int* garr = (int*)kItemWindowIconNames;
    int arr[11] = {garr[0], garr[1], garr[2], garr[3], garr[4], garr[5], garr[6], garr[7], garr[8], garr[9], garr[10]};
    void* mem = __nwa__FUl(kItemTextureArrayBytes);
    if (mem)
        mem = __construct_new_array(mem, (void*)__ct__6CBpTexFv, (void*)__dt__6CBpTexFv, kItemTextureObjectBytes,
                                    kItemTextureCount);
    itemTextures = mem;
    int* sp = arr;
    for (int i = 0; i < kItemTextureCount; ++i, ++sp) {
        ((ItemTexture*)itemTextures)[i].setTexturePathAddress(*sp);
    }
    for (;;) {
        Layout_Update(itemLayout);
        int ok = 1;
        for (int i = 0; i < kItemTextureCount; ++i) {
            ok &= ((ItemTexture*)itemTextures)[i].ready();
        }
        void* layout = itemLayout;
        ok &= Layout_PollLoadComplete(layout);
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80190474);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        void* object = IdMap_Lookup(Sing_805DC5F0(), *(void**)(&controllerId));
        ok &= BpObject_PollLoad(object);
        if (ok)
            break;
    }
}

// Cache slot origins and reset panel selection and item-use animation.
// Each of eight origins is layout position minus layout origin, in layout pixels;
// a nonzero cached pair is retained. Preserve count 1..8 -> slotOrigins[count-1].
void CItemWindow::OnInit() {
    void* scene = (*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc);
    if (AsCGolfDoc(scene)->tutorialStage < kFirstItemTutorialStage)
        return;
    void* layout = itemLayout;
    if (Layout_PollLoadComplete(layout)) {
        float zero = kFloat_0_itemWindow;
        void* position = this;
        float* offset = (float*)(&slotOrigins[0][0]);
        for (int i = 0; i < kItemSlotCount; ++i) {
            float value = ((CItemWindow*)position)->slotOrigins[0][0];
            if (zero == value) {
                value = ((CItemWindow*)position)->slotOrigins[0][1];
                if (zero == value) {
                    float originOffset[2];
                    layout = itemLayout;
                    Layout_GetElementPosition(layout, i, offset);
                    layout = itemLayout;
                    Layout_GetElementOrigin(layout, i, originOffset);
                    float coordinate = ((CItemWindow*)position)->slotOrigins[0][0];
                    value = originOffset[0];
                    value = coordinate - value;
                    ((CItemWindow*)position)->slotOrigins[0][0] = value;
                    coordinate = ((CItemWindow*)position)->slotOrigins[0][1];
                    value = originOffset[1];
                    value = coordinate - value;
                    ((CItemWindow*)position)->slotOrigins[0][1] = value;
                }
            }
            position = (char*)position + 8;
            offset += 2;
        }
    }
    float full = kFloat_200_ItemWindowShared;
    itemCount = 0;
    panelOpen = 0;
    layoutInitialized = 0;
    float zero = kFloat_0_itemWindow;
    ResetSelection(zero, full);
}

// Update panel loading, item-use animation, slot availability, and selection input.
// dt is elapsed seconds for animationTime, frame conversion and acceleration.
// Position integrates stored velocity once per call (without another dt factor).
// This mixed update rule is existing behavior; changing the call frequency changes motion.
void CItemWindow::OnProcess(float dt) {
    if (AsCGolfDoc((*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc))->tutorialStage < kFirstItemTutorialStage)
        return;
    if (!itemLayout)
        return;
    if (controllerId == -1)
        return;
    // Phase 1: tick layout loading first; script readiness then gates every later update.
    Layout_Update(itemLayout);
    if (!Layout_PollLoadComplete(itemLayout))
        return;
    if (0 == Sing_805DC5F0()) {
        if (0 == s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80190474);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* object = IdMap_Lookup(Sing_805DC5F0(), (*(void**)&controllerId));
    if (!BpObject_PollLoad(object))
        return;
    // Phase 2: initialize slot states once both resources are ready.
    if (layoutInitialized == 0) {
        for (int i = 0; i < kItemSlotCount; ++i) {
            Layout_SetElementPlayerFlagsAndState(itemLayout, i, 0);
        }
        layoutInitialized = 1;
    }
    // Phase 3: advance an animation that was active on entry to this block. Clearing
    // animationActive at the game-state check still permits this call's time/motion update.
    if (animationActive) {
        if (!(AsCGolfDoc((*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc))->gameStateFlags & kItemAnimationState)) {
            animationActive = 0;
        }

        float elapsedSeconds = (animationTime) + dt;
        animationTime = elapsedSeconds;
        if ((useAlternateItem)) {
            float elapsedFrames = kItemAnimationFramesPerSecond * elapsedSeconds;
            int truncatedFrame = (int)elapsedFrames;
            alternateIconFrame = (unsigned char)truncatedFrame;
            // Completion is equality on the truncated byte frame. A large dt may skip it;
            // do not change to >= without intentionally changing animation behavior.
            if ((unsigned char)truncatedFrame == kAlternateItemEndFrame) {
                alternateIconFrame = 0;
                animationActive = 0;
            }
        } else {
            // Arc: vY += gravity*dt, x += vX, y += vY. Positions advance once per
            // call, so velocity is a stored per-call displacement in this integrator.
            if (elapsedSeconds < kItemAnimationHomingDelay) {
                float nextVelocityY = kItemAnimationGravity6 * dt + animationVelocityY;

                animationX = (animationX + animationVelocityX);

                animationVelocityY = nextVelocityY;
                animationY = (animationY + nextVelocityY);
            } else {
                // Homing: borrow the actor's head bone, offset its world position,
                // then project world -> view -> screen before updating velocity.
                void* headBone;
                float projectedHead[14];

                float headOffset[3];

                IActor_DispatchMessage(this, this, itemWindowStrings + kPlayerActor, 7, &headBone,
                                       itemWindowStrings + kHeadBone, 0, 0);
                headOffset[0] = kFloat_0_itemWindow;
                headOffset[1] = kItemAnimationHeadOffsetY;

                headOffset[2] = kFloat_0_itemWindow;
                WVector worldHeadPosition = ((ItemTargetBoneView*)headBone)->worldPosition + *(WVector*)headOffset;
                float viewHeadPosition[3];
                Matrix_TransformPoint(viewHeadPosition, &worldHeadPosition, ((WView*)g_view)->inputTransform);

                WView_ProjectViewPoint(g_view, projectedHead, viewHeadPosition, 0);
                float targetHeadX = projectedHead[0];

                float previousY = animationY;
                float homingGain = kFloat_0_5_ItemWindow;
                float previousVelocityX = animationVelocityX;

                float previousX = animationX;
                float nextVelocityX = (dt * (targetHeadX - previousX)) * homingGain + previousVelocityX;

                animationVelocityX = nextVelocityX;

                float nextX = (previousX + nextVelocityX);
                float verticalErrorStep = dt * ((projectedHead[1]) - previousY);
                float nextVelocityY = homingGain * verticalErrorStep + (animationVelocityY);
                animationVelocityY = nextVelocityY;
                float targetX = projectedHead[0];
                float nextY = previousY + nextVelocityY;

                // Only horizontal target crossing completes this animation.
                // The current call still publishes both final coordinates.
                if (((targetX - nextX) * (targetX - previousX)) < 0.0f) {
                    animationActive = 0;
                }
                animationX = nextX;
                animationY = nextY;
            }
        }
    }
    // Phase 4: refresh usable slots, copy pointer input, update visibility, then handle
    // selection only if the resulting panel state is fully open.
    if (active) {
        if (s_itemWindowState == kPanelOpen) {
            for (int i = 0; kItemSlotCount > i; ++i) {
                void* p = GolfPlayer_GetEntryByIndex(
                    (*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc),
                    AsCGolfDoc((*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc))->currentPlayer);

                if (i < (((RoundPlayerItemState*)p)->itemCount)) {
                    Layout_SetElementPlayerFlag(itemLayout, i, 1, kItemSlotFlagSelector);
                } else {
                    Layout_SetElementPlayerFlag(itemLayout, i, 0, kItemSlotFlagSelector);
                }
            }
        }
        pointerX = ((InputPointerState*)g_inputPointer)->position[0];
        pointerY = ((InputPointerState*)g_inputPointer)->position[1];
        UpdatePanelVisibility();
        if (kPanelClosed == s_itemWindowState) {
            if (g_helpMsg) {
                NoticeQueue_ShowItemNotice(g_helpMsg, 0);
            }
        } else if (s_itemWindowState == kPanelOpen) {
            HandleItemSelection();
        }
        if (kPanelClosed != s_itemWindowState) {
            bounds[0] = kItemPanelHiddenX + panelOffsetX;
        }
    }
}

// Draw the panel, hover-scaled item icons, and an item traveling toward the player.
// Controls screen rectangles/UVs and publishes hit-test rectangles as a draw side
// effect. Required stock invariant: itemCount equals the number of NONZERO IDs
// in the current player's eight shared-doc slots. The loop visits all eight,
// skips zeros and packs visible indices; it does not stop at itemCount. Counts
// 1..8 choose the matching centering origin; count 0 is safe only with no IDs
// (the computed origin then is never read). UseItem preserves this relation by
// decrementing the count, compacting IDs and zeroing the vacated tail. Selection
// additionally expects a compact occupied prefix; a stock mod must update both.
// Every nonzero ID must resolve in ItemInventory_FindItemData; the +0x28 address
// expression below is not a null-record check. When animationActive is set and
// useAlternateItem is clear, usedItemCode must ALSO resolve, even with empty
// equipped stock: UseItem removes the ID before retaining it for the traveling
// icon. Keep that IFF/icon-name record alive through the animation's last draw.
// Texture lookup defaults to index0 when no icon name matches; no resource ownership
// is transferred. Render state changes are left to the surrounding draw context.
void CItemWindow::OnDisplay() {
    char* data = itemWindowDrawData;
    if (AsCGolfDoc((void*)g_sharedDoc->golfDoc)->tutorialStage < kFirstItemTutorialStage || !(*(int*)&itemLayout) ||
        controllerId == -1 || !layoutInitialized)
        return;
    if (!drawRequested)
        return;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80190474);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* doc = IdMap_Lookup(Sing_805DC5F0(), (*(void**)&controllerId));
    BpObject_TryInvokeCurrentCallback(doc, 0);
    BpObject_Draw(doc);
    // Phase 1: draw the BP background. Open-panel icons use screen-space XY,
    // template Z/color, and normalized UVs in XYZ/color/UV 0x18-byte vertices.
    const ItemNames quads = *(const ItemNames*)(((char*)(data + 0x18c)));
    float rect[4];
    rect[0] = rect[1] = kFloat_0_itemWindow;
    rect[2] = rect[3] = kItemIconUvExtent;
    if (!s_itemWindowState) {
        unsigned char player = AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer;
        int* equippedItemIds = (int*)((char*)g_sharedDoc + kSharedItemIdsOffset + player * 0x1d4);
        int itemCount = GolfPlayer_GetEntryByIndex(AsCGolfDoc((void*)g_sharedDoc->golfDoc), player)->itemCount;
        int visibleItemCount = 0;
        Layout_DrawWithOpacity(itemLayout, kFloat_1_ItemWindow);
        float size[2];
        Layout_GetElementSize(itemLayout, 0, size);
        // Count-to-origin map: itemCount 1 uses slotOrigins[0], count 2 uses [1],
        // through count 8 -> [7]. Index 0 overlaps drawRequested and the following
        // word; retain that ABI behavior rather than treating it as a ninth origin.
        // Screen coordinates and icon pitch are pixels; UVs stay normalized.
        float* countLayoutOrigin = (float*)((char*)this + itemCount * 8 + kItemOriginTableOffset);
        const ItemQuad* templateQuad = (const ItemQuad*)(((char*)(data + 0x1b8)));
        for (int inventorySlot = 0; inventorySlot < kItemSlotCount; ++inventorySlot) {
            int itemId = equippedItemIds[inventorySlot];
            if (!itemId)
                continue;
            if (!((char*)ItemInventory_FindItemData(((CSharedDoc*)g_sharedDoc)->itemInventory, (void*)itemId) + 0x28))
                continue;
            int textureIndex = 0;
            ++visibleItemCount;
            for (int candidateTexture = 0; candidateTexture < kItemTextureCount; ++candidateTexture) {
                IFF_STRUCT::sItem* candidate = (IFF_STRUCT::sItem*)ItemInventory_FindItemData(
                    ((CSharedDoc*)g_sharedDoc)->itemInventory, (void*)equippedItemIds[inventorySlot]);
                if (!strcmp((const char*)quads.name[candidateTexture], candidate->common.icon)) {
                    textureIndex = candidateTexture;
                    break;
                }
            }
            int visibleSlot = visibleItemCount - 1;
            int pixelRect[4];
            float pixelPosition[2];
            pixelRect[0] = (int)(countLayoutOrigin[0] + kItemIconSpacing49 * visibleSlot);
            pixelRect[2] = (int)(pixelRect[0] + size[0]);
            pixelRect[1] = (int)countLayoutOrigin[1];
            pixelRect[3] = (int)(pixelRect[1] + size[1]);
            pixelPosition[0] = pixelRect[0];
            pixelPosition[1] = pixelRect[1];
            Layout_SetElementPosition(itemLayout, visibleSlot, pixelPosition);
            Layout_WriteElementRect(itemLayout, visibleSlot, pixelRect);
            if (!GetInputManagerSlot()) {
                if (!s_inputManagerInitialized) {
                    InputManager_Init(s_gameInputManager);
                    __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                             s_inputManagerDtorRegistration_ItemWindow);
                    s_inputManagerInitialized = 1;
                }
                GetInputManagerSlot() = s_gameInputManager;
            }
            float mouse[2];
            InputManager_GetPointerPosition(GetInputManagerSlot(), mouse, -1);
            float width;
            if (pixelRect[0] < mouse[0] && pixelRect[1] < mouse[1] && pixelRect[2] > mouse[0] &&
                pixelRect[3] > mouse[1])
                width = kItemIconHoveredScale * size[0];
            else
                width = kItemIconNormalScale * size[0];
            float bounds[4];
            float delta = (width - size[0]) * kFloat_0_5_ItemWindow;
            bounds[0] = pixelRect[0] - delta;
            bounds[2] = bounds[0] + width;
            bounds[1] = pixelRect[1] - delta;
            bounds[3] = bounds[1] + width;
            ItemQuad verts = *templateQuad;
            verts.v[0].x = bounds[0];
            verts.v[0].y = bounds[1];
            verts.v[0].u = rect[0];
            verts.v[0].v = rect[1];
            verts.v[1].x = bounds[2];
            verts.v[1].y = bounds[1];
            verts.v[1].u = rect[2];
            verts.v[1].v = rect[1];
            verts.v[2].x = bounds[0];
            verts.v[2].y = bounds[3];
            verts.v[2].u = rect[0];
            verts.v[2].v = rect[3];
            verts.v[3].x = bounds[2];
            verts.v[3].y = bounds[3];
            verts.v[3].u = rect[2];
            verts.v[3].v = rect[3];
            Engine_SetFlexibleVertexFormat(kItemQuadVertexFormat);
            ((ItemTexture*)itemTextures)[textureIndex].bind();
            DrawPrimitiveUP(kItemQuadPrimitive, GX_GetPrimitiveCount(kItemQuadVertexCount, kItemQuadPrimitive), &verts,
                            kItemQuadStride);
        }
    }
    // Phase 2: draw the non-alternate traveling icon at its current screen position.
    if (animationActive && !useAlternateItem) {
        float bounds[4];
        bounds[0] = (int)animationX;
        bounds[1] = (int)animationY;
        bounds[3] = bounds[2] = kItemAnimationIconSize;
        ItemQuad verts = *(ItemQuad*)(((char*)(data + 0x218)));
        verts.v[0].x = bounds[0];
        verts.v[0].y = bounds[1];
        verts.v[0].u = rect[0];
        verts.v[0].v = rect[1];
        verts.v[1].x = bounds[0] + bounds[2];
        verts.v[1].y = bounds[1];
        verts.v[1].u = rect[2];
        verts.v[1].v = rect[1];
        verts.v[2].x = bounds[0];
        verts.v[2].y = bounds[1] + bounds[3];
        verts.v[2].u = rect[0];
        verts.v[2].v = rect[3];
        verts.v[3].x = bounds[0] + bounds[2];
        verts.v[3].y = bounds[1] + bounds[3];
        verts.v[3].u = rect[2];
        verts.v[3].v = rect[3];
        Engine_SetFlexibleVertexFormat(kItemQuadVertexFormat);
        int textureIndex = 0;
        for (int candidateTexture = 0; candidateTexture < kItemTextureCount; ++candidateTexture) {
            IFF_STRUCT::sItem* candidate = (IFF_STRUCT::sItem*)ItemInventory_FindItemData(
                ((CSharedDoc*)g_sharedDoc)->itemInventory, (*(void**)&usedItemCode));
            if (!strcmp((const char*)quads.name[candidateTexture], candidate->common.icon)) {
                textureIndex = candidateTexture;
                break;
            }
        }
        ((ItemTexture*)itemTextures)[textureIndex].bind();
        DrawPrimitiveUP(kItemQuadPrimitive, GX_GetPrimitiveCount(kItemQuadVertexCount, kItemQuadPrimitive), &verts,
                        kItemQuadStride);
    }
    drawRequested = 0;
}

#pragma push
#pragma use_lmw_stmw on
// Open or close the panel from screen permissions and available items; finish script transitions.
void CItemWindow::UpdatePanelVisibility() {
    // Screen 0x30 writes HUD phase, which this routine does not read. Screen
    // 0x43 writes itemPanelReady; only that result gates entrance/exit below.
    int unusedHudPhase;
    int itemPanelReady;
    char name[32];
    void* doc;
    unsigned int threshold;
    unsigned int i;
    void* doc2;
    int selByte;
    int j;
    void* g0;
    RoundPlayerItemState* item;
    if ((AsCGolfDoc((void*)g_sharedDoc->golfDoc)->gameStateFlags & kPanelVisibilityStateMask) != 0) {
        IActor_DispatchMessage(this, this, itemWindowStrings + kScreenActor, kScreenQueryHudPhase, &unusedHudPhase, 0,
                               0, 0);
        IActor_DispatchMessage(this, this, itemWindowStrings + kScreenActor, kScreenQueryItemPanelReady,
                               &itemPanelReady, 0, 0, 0);
        if (Sing_805DC5F0() == 0) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80190474);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        doc = (void*)IdMap_Lookup(Sing_805DC5F0(), (*(void**)&controllerId));
        item = GolfPlayer_GetEntryByIndex(AsCGolfDoc((void*)g_sharedDoc->golfDoc),
                                          AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer);
        // Phase 1: start entrance only when item-panel readiness, stock and request agree.
        // panelOpen is the script target; s_itemWindowState also records in-flight motion.
        if (item->itemCount && itemPanelReady) {
            if (s_itemWindowState == kPanelClosed || s_itemWindowState == kPanelOpening) {
                if (!panelOpen) {
                    item = GolfPlayer_GetEntryByIndex(AsCGolfDoc((void*)g_sharedDoc->golfDoc),
                                                      AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer);
                    g0 = Sing_805DC5F0();
                    selByte = item->itemCount;
                    itemCount = selByte;
                    if (g0 == 0) {
                        if (!s_bpCtrlInitialized) {
                            ((CBpCtrl*)s_bpCtrl)->__ct();
                            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80190474);
                            s_bpCtrlInitialized = 1;
                        }
                        Sing_805DC5F0() = s_bpCtrl;
                    }
                    doc2 = (void*)IdMap_Lookup(Sing_805DC5F0(), (*(void**)&controllerId));
                    threshold = selByte - 1;
                    for (i = 0; i < kItemSlotCount; ++i) {
                        sprintf(name, itemWindowStrings + kItemCountGroupFormat, i + 1);
                        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(doc2, name), i == threshold);
                    }
                    BpObject_StartScriptByName(doc, itemWindowStrings + kPanelInScript, 1);
                    panelOpen = 1;
                    Sound_PlayByName(g_soundManager, itemWindowStrings + kPanelOpenSound, 0, 0, 0, 0,
                                     kFloat_0_5_ItemWindow, kFloat_200_ItemWindowShared, kFloat_1_ItemWindow);
                    *(unsigned char*)((char*)InstanceTable_Get() + 0x64) = 1;
                    s_itemWindowState = kPanelOpening;
                }
            }
        }
        // Phase 2: revoked item-panel readiness starts exit and disables all slot input.
        if (!itemPanelReady && (s_itemWindowState == kPanelOpen || s_itemWindowState == kPanelClosing) && panelOpen) {
            BpObject_StartScriptByName(doc, itemWindowStrings + kPanelOutScript, 1);
            panelOpen = 0;
            Sound_PlayByName(g_soundManager, itemWindowStrings + kPanelCloseSound, 0, 0, 0, 0, kFloat_0_5_ItemWindow,
                             kFloat_200_ItemWindowShared, kFloat_1_ItemWindow);
            for (j = 0; j < 8; ++j)
                Layout_SetElementPlayerFlag(itemLayout, j, 0, kItemSlotFlagSelector);
            s_itemWindowState = kPanelClosing;
        }
        // Phase 3: only script completion promotes opening/closing to a stable state.
        if (BpObject_IsCurrentScriptComplete(doc)) {
            switch (s_itemWindowState) {
            case kPanelClosing:
                if (!panelOpen)
                    s_itemWindowState = kPanelClosed;
                break;
            case kPanelOpening:
                if (panelOpen)
                    s_itemWindowState = kPanelOpen;
                break;
            }
        }
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Inventory indexes, not full IFF IDs. Labels state only the verified use policy.
enum ItemUsePolicyIndex {
    kOffGreenItem0 = 0,
    kOffGreenItem1 = 1,
    kOffGreenItem2 = 2,
    kGreenOnlyItem3 = 3,
    kOffGreenModeRestrictedItem4 = 4,
    kUnrestrictedItem5 = 5,
    kRecoveryFlagRequiredItem6 = 6,
    kOffGreenModeRestrictedItem7 = 7,
    kOffGreenItem8 = 8,
    kOffGreenItem9 = 9,
    kOffGreenModeRestrictedItem10 = 10
};

// Show hovered-item help and activate usable items according to ground and ball state.
// Admission contract: OnProcess is the normal caller. It has passed tutorial
// stage>=6, live layout/controller checks, Layout and BP readiness, initialized
// element flags, active!=0 and UpdatePanelVisibility's resulting state==Open.
// Opening/Closing/Closed never call this helper; selectionEnabled alone does not
// admit them. Direct callers must enforce the same conditions and current-player
// inventory 0..8. Layout results are trusted slot indices [0,itemCount), not IFF
// IDs; this function checks item type/policy but does not bounds-check those slots.
// selectedSlot>=0 or !selectionEnabled leaves even the old help/activation words
// untouched. Otherwise last hovered slot supplies help (zero clears it), then
// every activation is visited. Invalid IDs only notify. A valid activation latches
// selectedSlot, resets cursor mode and sets Closing before the task-status test;
// the status can defer consumption, leaving that selection latched. The loop
// does not break/retest admission after a success, so preserve multiple-activation
// order, synchronous rule dispatch, one rand() per dispatched use, and RAII vectors.
void CItemWindow::HandleItemSelection() {
    unsigned int i;
    if (selectedSlot < 0 && selectionEnabled) {
        CSharedDoc* mgr = g_sharedDoc;
        SharedPlayerItemSlotsView* row =
            (SharedPlayerItemSlotsView*)((char*)mgr + AsCGolfDoc((void*)mgr->golfDoc)->currentPlayer * 0x1D4);
        int* slots = (int*)row->itemIds;
        activatedSlot = -1;
        (*(int*)&usedItemCode) = 0;
        // Phase 1: collect hovered controls for help, then collect activated controls.
        IntVec hovered = {0, 0, 0};
        unsigned int picked = 0;
        if (Layout_CollectHovered(itemLayout, &hovered, kItemSlotFlagSelector))
            picked = slots[hovered.data[hovered.size - 1]];
        if (g_helpMsg)
            NoticeQueue_ShowItemNotice(g_helpMsg, picked);
        IntVec activated = {0, 0, 0};
        if (Layout_CollectActivated(itemLayout, &activated, 0)) {
            for (i = 0; i < activated.size; ++i) {
                if (i >= activated.size)
                    OutOfRange e(itemWindowStrings + kVectorIndexError);
                unsigned int index = activated.data[i];
                activatedSlot = index;
                unsigned int code =
                    ((SharedPlayerItemSlotsView*)g_sharedDoc)[AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer]
                        .itemIds[index];
                int valid;
                if (code == 0 || (code >> kIffKindShift) != kIffConsumableKind) {
                    valid = 0;
                } else {
                    unsigned int kind = ItemInventory_FindItemIndex(((CSharedDoc*)g_sharedDoc)->itemInventory, code);
                    switch (kind) {
                    // Inventory indices in this group are usable only off the green.
                    case kOffGreenItem0:
                    case kOffGreenItem1:
                    case kOffGreenItem2:
                    case kOffGreenItem8:
                    case kOffGreenItem9: {
                        Arr3u val = *(Arr3u*)GolfBall__Fv();
                        unsigned int result = ((unsigned int (*)(void*, void*))GetGroundType__9CQuadTreeF7WVector)(
                                                  (void*)CQuadTree_GetInstance(), &val) &
                                              0xFF;
                        valid = result != kGreenGroundType;
                        break;
                    }
                    // These indices also require the current context mode to differ from 3.
                    case kOffGreenModeRestrictedItem4:
                    case kOffGreenModeRestrictedItem7:
                    case kOffGreenModeRestrictedItem10: {
                        valid = 0;
                        Arr3u val = *(Arr3u*)GolfBall__Fv();
                        unsigned int result = ((unsigned int (*)(void*, void*))GetGroundType__9CQuadTreeF7WVector)(
                                                  (void*)CQuadTree_GetInstance(), &val) &
                                              0xFF;
                        if (result != kGreenGroundType &&
                            *((unsigned char*)*(void**)((char*)InstanceTable_Get() + 4) + 0xA) != 3)
                            valid = 1;
                        break;
                    }
                    // Inventory index 3 is usable only on the green.
                    case kGreenOnlyItem3: {
                        Arr3u val = *(Arr3u*)GolfBall__Fv();
                        valid = (((unsigned int (*)(void*, void*))GetGroundType__9CQuadTreeF7WVector)(
                                     (void*)CQuadTree_GetInstance(), &val) &
                                 0xFF) == kGreenGroundType;
                        break;
                    }
                    case kUnrestrictedItem5:
                        valid = 1;
                        break;
                    case kRecoveryFlagRequiredItem6:
                        valid = *((unsigned char*)GolfBall__Fv() + 0x58);
                        break;
                    default:
                        valid = 0;
                        break;
                    }
                }
                // Phase 2: invalid items only notify; valid selection closes the panel
                // and synchronously dispatches consumption if the task permits it.
                if (!valid) {
                    if (g_helpMsg)
                        NoticeQueue_ShowById(g_helpMsg, 6, 0);
                } else {
                    void* d4 = g_mouseCursor;
                    ((CMouseCursor*)d4)->alternateHudMode = 0;
                    selectedSlot = index;
                    s_itemWindowState = kPanelClosing;
                    if (!GolfTask_NoOpStatus(d4)) {
                        IActor_DispatchMessage(this, this, itemWindowStrings + kGolfRuleActor, 0x15, 0, 0, 0, 0);
                        CGolfDoc* sub = AsCGolfDoc((void*)g_sharedDoc->golfDoc);
                        void* roll = (void*)rand();
                        UseItem(sub->currentPlayer, (void*)code, 1, roll);
                    }
                }
            }
        }
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Consume a player's selected item, roll its failure substitution, and apply gauge/guide effects.
// Side-effect table (order of execution):
//   1. roll: useAlternateItem = (random % 100) < item failure percent (100 if item unknown)
//   2. non-current player -> return 0 (nothing else happens)
//   3. remove the item from the player's list (selected slot, or first match) -> 0 if absent
//   4. state 8 only: start the use animation from the removed slot's screen position
//   5. effective item = requested item, or the failure replacement item when the roll failed
//   6. effects by the effective item's inventory index: 7 or 10 -> power gauge +33 (clamped
//      0..99) and a rule message with the change; 3 -> guide-line message; others -> none
//   7. play the failure or use sound; if the requested item is the "cleared after use" item
//      (inventory index 6) send a rule message and reset activeItemId
// itemIdArgument and randomValue carry unsigned 32-bit integers, never pointees.
// Roll = unsigned(randomValue)%100; signed failureChancePercent is compared with <.
// Only the current player can succeed. A nonzero consume flag requires selectedSlot
// to name the matching item; zero searches the first matching item instead. Caller
// must provide 1..8 items and, for selected consumption, 0 <= selectedSlot <
// itemCount. Neither the selected-slot upper bound nor nonempty count is checked.
// Return 1 after consumption/effects, 0 if player/selection validation fails.
// A 0 return can still change useAlternateItem because the roll precedes validation.
int CItemWindow::UseItem(unsigned char playerIndex, void* itemIdArgument, int slotOrConsumeSelection,
                         void* randomValue) {
    // On entry this argument chooses selected-slot consumption; afterward it holds the removed slot index.
    if (playerIndex >= AsCGolfDoc((void*)g_sharedDoc->golfDoc)->playerCount)
        return 0;
    RoundPlayerItemState* entry = GolfPlayer_GetEntryByIndex(AsCGolfDoc((void*)g_sharedDoc->golfDoc), playerIndex);
    // Player rows are 0x1D4 bytes; itemIds begins at shared-doc+0x8C for row zero
    // (not at CSharedDoc's round-player actor pointer array).
    int playerRecordOffset = (int)playerIndex * 0x1D4;
    CSharedDoc* sharedDocument = g_sharedDoc;
    char* playerRecord = (char*)sharedDocument + playerRecordOffset;
    char* itemSlots = (char*)((SharedPlayerItemSlotsView*)playerRecord)->itemIds;
    void* itemRecord = ((void* (*)(void*, void*, void*))ItemInventory_FindItemData)(
        ((CSharedDoc*)sharedDocument)->itemInventory, itemIdArgument, playerRecord);
    // Phase 1: compute failure before validating/removing the selected inventory slot.
    short percentRoll = (unsigned int)randomValue % kPercentRange;
    int failureChancePercent;
    if (itemRecord != 0)
        failureChancePercent = *(short*)&((IFF_STRUCT::sItem*)itemRecord)->failureChancePercent;
    else
        failureChancePercent = kPercentRange;
    // Missing item data forces the failure replacement; the record's halfword is read signed.
    useAlternateItem = percentRoll < failureChancePercent;
    // Phase 2: current-player inventory compaction precedes animation/effect dispatch.
    if (playerIndex == AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer) {
        unsigned char* remainingItemCount = &entry->itemCount;
        int* selectedSlot = (int*)(&this->selectedSlot);
        if (slotOrConsumeSelection != 0) {
            if (*selectedSlot < 0 || (unsigned int)itemIdArgument != *(unsigned int*)(itemSlots + *selectedSlot * 4))
                return 0;
            *remainingItemCount = *remainingItemCount - 1;
            int k = *selectedSlot;
            if (k < (int)*remainingItemCount) {
                memcpy(itemSlots + k * 4, itemSlots + k * 4 + 4, (*remainingItemCount - k) * 4);
            }
            *(unsigned int*)(itemSlots + *remainingItemCount * 4) = 0;
            slotOrConsumeSelection = *selectedSlot;
            *selectedSlot = -1;
        } else {
            int n = *(volatile unsigned char*)remainingItemCount;
            for (slotOrConsumeSelection = 0; slotOrConsumeSelection < n; slotOrConsumeSelection++) {
                if ((unsigned int)itemIdArgument == ((unsigned int*)itemSlots)[slotOrConsumeSelection])
                    break;
            }
            if (slotOrConsumeSelection >= n)
                return 0;
            *remainingItemCount = *remainingItemCount - 1;
            if (slotOrConsumeSelection < (int)*remainingItemCount) {
                memcpy(itemSlots + slotOrConsumeSelection * 4, itemSlots + slotOrConsumeSelection * 4 + 4,
                       (*remainingItemCount - slotOrConsumeSelection) * 4);
            }
            *(unsigned int*)(itemSlots + *remainingItemCount * 4) = 0;
        }

        // Inventory compaction is complete. The reused argument now names the removed
        // slot, whose old screen position starts the animation only in exact state 8.
        if ((*(unsigned int*)&AsCGolfDoc((void*)g_sharedDoc->golfDoc)->gameStateFlags) == kItemAnimationState) {
            float tmp[2];
            float zero = kFloat_0_itemWindow;
            (*(volatile unsigned char*)&alternateIconFrame) = 0;
            animationTime = zero;
            void* p94 = itemLayout;
            Layout_GetElementPosition(p94, slotOrConsumeSelection, tmp);
            float x = tmp[0];
            float zero2 = kFloat_0_itemWindow;
            animationX = x;
            float speed = kItemAnimationInitialVelocityY;
            float y = tmp[1];
            animationY = y;
            animationVelocityX = zero2;
            animationVelocityY = speed;
            animationActive = 1;
        }

        // Keep usedItemCode as the requested ID; activeItemId receives the effective
        // ID after failure substitution. Effects and sound use that failure decision.
        unsigned char useFailureItem = useAlternateItem;
        usedItemCode = (unsigned int)itemIdArgument;
        if (useFailureItem != 0) {
            itemIdArgument = (void*)ItemInventory_GetItemIdAt(((CSharedDoc*)g_sharedDoc)->itemInventory,
                                                              kFailedItemReplacementIndex);
        }
        int itemIndex =
            (int)ItemInventory_FindItemIndex(((CSharedDoc*)g_sharedDoc)->itemInventory, (unsigned int)itemIdArgument);
        switch (itemIndex) {
        // These two inventory indices add 33 percentage points, capped at 99.
        case 7:
        case 10: {
            float gaugeIncrease = entry->powerGauge;
            float requestedGauge = kItemPowerGaugeIncrease33 + entry->powerGauge;
            float clampedGauge = kFloat_0_itemWindow;
            if (requestedGauge < clampedGauge) {
                clampedGauge = kFloat_0_itemWindow;
            } else if (requestedGauge > kItemPowerGaugeMaximum99) {
                clampedGauge = kItemPowerGaugeMaximum99;
            } else {
                clampedGauge = requestedGauge;
            }
            entry->powerGauge = clampedGauge;
            gaugeIncrease = entry->powerGauge - gaugeIncrease;
            IActor_DispatchMessage(this, this, itemWindowStrings + kGolfRuleActor, 0x2E, &gaugeIncrease, 0, 0, 0);
            break;
        }
        case 3:
            IActor_DispatchMessage(this, this, itemWindowStrings + kGuideLineActor, 1, (void*)1, 0, 0, 0);
            break;
        case -100:
            break;
        }
        entry->activeItemId = (unsigned int)itemIdArgument;
    } else {
        return 0;
    }

    // Phase 3: inventory/effects are committed; play result sound and clear transient items.
    if (useAlternateItem != 0) {
        Sound_PlayByName(g_soundManager, itemWindowStrings + kItemFailureSound, 0, 0, 0, 0, kFloat_0_5_ItemWindow,
                         kFloat_200_ItemWindowShared, kFloat_1_ItemWindow);
    } else {
        Sound_PlayByName(g_soundManager, itemWindowStrings + kItemUseSound, 0, 0, 0, 0, kFloat_0_5_ItemWindow,
                         kFloat_200_ItemWindowShared, kFloat_1_ItemWindow);
    }

    if (playerIndex == AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer) {
        unsigned int clearedAfterUseItemId = ItemInventory_GetItemIdAt(((CSharedDoc*)g_sharedDoc)->itemInventory, 6);
        if (usedItemCode == clearedAfterUseItemId) {
            IActor_DispatchMessage(this, this, itemWindowStrings + kGolfRuleActor, 0x19, 0, 0, 0, 0);
            entry->activeItemId = 0;
        }
    }

    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma dont_inline on
// Route item consumption to the matching player owner or the current local player.
// owner is borrowed; itemIdArgument/randomValue retain the integer-in-pointer ABI
// of UseItem. Finding an owner does not guarantee success for a non-current player.
void CItemWindow::UseItemForOwner(void* owner, void* itemIdArgument, void* randomValue) {
    if (GolfTask_NoOpStatus(this)) {
        for (unsigned char i = 0; i < AsCGolfDoc((void*)g_sharedDoc->golfDoc)->playerCount; i++) {
            RoundPlayerItemState* playerState = GolfPlayer_GetEntryByIndex(AsCGolfDoc((void*)g_sharedDoc->golfDoc), i);
            if (playerState->state != 3) {
                RoundPlayerItemState* candidate =
                    GolfPlayer_GetEntryByIndex(AsCGolfDoc((void*)g_sharedDoc->golfDoc), (unsigned char)i);
                if (owner == candidate->owner) {
                    if (UseItem(i, itemIdArgument, owner == 0, randomValue))
                        return;
                    return;
                }
            }
        }
    } else {
        UseItem(AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer, itemIdArgument, 1, randomValue);
    }
}
#pragma pop

// Dispatch panel visibility, selection, item-use, drawing, and state-query messages.
// msg is required. Query value words are writable int*; ActivateSlot carries
// index 0..7 as an immediate word (must also fit the current stock). UseForOwner
// carries borrowed owner plus integer item ID/random words. Unknown types do
// nothing; queries preserve all outputs other than their specified int result.
void CItemWindow::HandleMsg(Message* msg) {
    switch (msg->type) {
    case kItemMsgRefreshVisibility:
        UpdatePanelVisibility();
        break;
    case kItemMsgBeginOpen:
        s_itemWindowState = kPanelOpening;
        selectionEnabled = 1;
        break;
    case kItemMsgBeginClose:
        s_itemWindowState = kPanelClosing;
        selectionEnabled = 0;
        break;
    case kItemMsgQueryPanelState:
        *(int*)msg->value = s_itemWindowState;
        break;
    case kItemMsgQueryVisible:
        if (s_itemWindowState != kPanelClosed)
            *(int*)msg->value = 1;
        else
            *(int*)msg->value = 0;
        break;
    case kItemMsgRequestDraw:
        drawRequested = 1;
        break;
    case kItemMsgEnableProcess:
        active = 1;
        break;
    case kItemMsgDisableProcess:
        active = 0;
        break;
    case kItemMsgActivateSlot: {
        unsigned int index = msg->value;
        unsigned int code =
            *(unsigned int*)((char*)g_sharedDoc + kSharedItemIdsOffset +
                             AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer * 0x1D4 + index * 4);
        int valid;
        if (code == 0 || (code >> kIffKindShift) != kIffConsumableKind) {
            valid = 0;
        } else {
            unsigned int kind = ItemInventory_FindItemIndex(((CSharedDoc*)g_sharedDoc)->itemInventory, code);
            switch (kind) {
                // Inventory indices in this group are usable only off the green.
            case kOffGreenItem0:
            case kOffGreenItem1:
            case kOffGreenItem2:
            case kOffGreenItem8:
            case kOffGreenItem9: {
                Arr3u val = *(Arr3u*)GolfBall__Fv();
                unsigned int result = ((unsigned int (*)(void*, void*))GetGroundType__9CQuadTreeF7WVector)(
                                          (void*)CQuadTree_GetInstance(), &val) &
                                      0xFF;
                valid = result != kGreenGroundType;
                break;
            }
                // These indices also require the current context mode to differ from 3.
            case kOffGreenModeRestrictedItem4:
            case kOffGreenModeRestrictedItem7:
            case kOffGreenModeRestrictedItem10: {
                valid = 0;
                Arr3u val = *(Arr3u*)GolfBall__Fv();
                unsigned int result = ((unsigned int (*)(void*, void*))GetGroundType__9CQuadTreeF7WVector)(
                                          (void*)CQuadTree_GetInstance(), &val) &
                                      0xFF;
                if (result != kGreenGroundType &&
                    *((unsigned char*)*(void**)((char*)InstanceTable_Get() + 4) + 0xA) != 3)
                    valid = 1;
                break;
            }
                // Inventory index 3 is usable only on the green.
            case kGreenOnlyItem3: {
                Arr3u val = *(Arr3u*)GolfBall__Fv();
                valid = (((unsigned int (*)(void*, void*))GetGroundType__9CQuadTreeF7WVector)(
                             (void*)CQuadTree_GetInstance(), &val) &
                         0xFF) == kGreenGroundType;
                break;
            }
            case kUnrestrictedItem5:
                valid = 1;
                break;
            case kRecoveryFlagRequiredItem6:
                valid = *((unsigned char*)GolfBall__Fv() + 0x58);
                break;
            default:
                valid = 0;
                break;
            }
        }
        if (!valid) {
            if (g_helpMsg)
                NoticeQueue_ShowById(g_helpMsg, 6, 0);
            break;
        }
        void* d4 = g_mouseCursor;
        ((CMouseCursor*)d4)->alternateHudMode = 0;
        (*(unsigned int*)&selectedSlot) = index;
        s_itemWindowState = kPanelClosing;
        if (GolfTask_NoOpStatus(d4))
            break;
        IActor_DispatchMessage(this, this, itemWindowStrings + kGolfRuleActor, 0x15, 0, 0, 0, 0);
        CGolfDoc* sub = AsCGolfDoc((void*)g_sharedDoc->golfDoc);
        void* randomValueToken = (void*)rand();
        UseItem(sub->currentPlayer, (void*)code, 1, randomValueToken);
        break;
    }
    case kItemMsgUseForOwner:
        UseItemForOwner((void*)msg->value, msg->itemId, msg->randomValue);
        break;
    case kItemMsgResetSelection: {
        float full = kFloat_200_ItemWindowShared;
        float zero = kFloat_0_itemWindow;
        ResetSelection(zero, full);
        break;
    }
    case kItemMsgQueryNoActiveSelection: {
        CGolfDoc* sub = AsCGolfDoc((void*)g_sharedDoc->golfDoc);
        RoundPlayerItemState* item = GolfPlayer_GetEntryByIndex(sub, sub->currentPlayer);
        if (!(int)item->activeItemId && selectedSlot == -1)
            *(int*)msg->value = 1;
        else
            *(int*)msg->value = 0;
        break;
    }
    case kItemMsgQueryPanelTransitionState:
        *(int*)msg->value = s_itemWindowState;
        break;
    case kItemMsgNoOp:
        break;
    }
}

// Report whether the item panel is fully closed.
int CItemWindow::IsClosed() {
    return s_itemWindowState == kPanelClosed;
}

// Return the item-window runtime type descriptor.
void* CItemWindow::GetRTTI() {
    return &s_itemWindowTypeDescriptor;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462154", _ctors_u80190474_0, InitItemWindowFullTurnAngle);
#pragma pop
