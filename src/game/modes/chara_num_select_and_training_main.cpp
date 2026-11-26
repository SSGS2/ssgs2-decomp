// Player-count selection and training: the 2, 3 and 4 player menu shown over the course
// preview (CCharaNum), and the training course selector (CTrainingMain).
//
// It also has the view helpers for the enemy character slot and shared document, and the
// fade and input singletons these menus use.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include "classes/EnemyCharacterSlotView.h"
#include "classes/MPBase1Layout.h"
#include "classes/CPangyaEnemyData.h"
#include "iff/IffStruct.h"
// View the existing storage as EnemyCharacterSlotView without transferring ownership.
static inline EnemyCharacterSlotView* AsCharaSlot8024745C(const volatile void* p) {
    return (EnemyCharacterSlotView*)p;
}
#include "classes/CSharedDoc.h"
// View the existing storage as CSharedDoc without transferring ownership.
static inline CSharedDoc* AsCSharedDoc(const volatile void* p) {
    return (CSharedDoc*)p;
}

#pragma use_lmw_stmw on

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CCharaNum.h"
#include "classes/CTrainingMain.h"
#include "classes/CPackTextureResource.h"
#include "classes/CCourseSelect.h"
#include "classes/CBpCtrl.h"
#include "classes/CBpCtrlHandle.h"
#include "classes/CBpTex.h"

// View the existing storage as CTrainingMain without transferring ownership.
static inline CTrainingMain* AsCTrainingMain(const volatile void* p) {
    return (CTrainingMain*)p;
}

// View the existing storage as CCharaNum without transferring ownership.
static inline CCharaNum* AsCCharaNum(const volatile void* p) {
    return (CCharaNum*)p;
}

// Enemy table payload, distinct from IFF records: outfit entries store inventory
// indices; ItemInventory_Get*IdAt converts them to IFF type IDs.
struct EnemyOutfitRecord {
    char header[2];
    char levelIndex;             // 0x02
    char hairStyleIndex;         // 0x03 (sign-extended when copied)
    int characterInventoryIndex; // 0x04
    int partInventoryIndices[8]; // 0x08
    int clubSetInventoryIndex;   // 0x28
    int ballInventoryIndex;      // 0x2C
    int auxiliaryValues[3];      // 0x30: copied; individual roles unresolved
    int characterIndex;          // 0x3C
    int caddieInventoryIndex;    // 0x40
    unsigned char rankValue;     // 0x44: converted to the slot rank byte
    char packedAttributes[14];   // 0x45: copied without interpretation
    char reserved53;
    char characterAttributes[5]; // 0x54: five copied bytes; stat interpretation unconfirmed
};
typedef char EnemyOutfitRecord_size[(sizeof(EnemyOutfitRecord) == 0x5C) ? 1 : -1];

enum CharaSlotFlags {
    kSlotSourceFlag = 0x2000,
    kSlotClearedSourceFlag = 0x1000,
    kSlotHasOutfit = 0x8000
};
// State map: 0 waits for fade state != 3, 1 advances one update, 2 reads layout/input.
// UI IDs 0/1/2 choose 2/3/4 players; value state 2 highlights, state 0 clears.
// A choice completes after four update calls (not elapsed seconds). Cancel returns
// finished too, but leaves selectionComplete clear; use GetSelectedPlayerCount to distinguish.
// Singleton access below retains the legacy pointer-as-int reference ABI:
// 805DC608 = fade overlay; 805DC5C8 = input; 805DC5F0 = BP registry;
// 805DC620 = texture resource table; 805DC5F8 = active-script handle.
// Registry objects are borrowed; this menu owns layout/packTextureResource/bpTexture.
enum CharaCountState {
    kCountWaitForFade,
    kCountPrepare,
    kCountReadInput
};
enum CharaCountMenu {
    kTwoPlayers,
    kThreePlayers,
    kFourPlayers
};
enum CharaCountTiming {
    kSelectionFeedbackFrames = 4,
    kFadeBusy = 3
};
enum CharaPreviewDrawing {
    kPreviewVertexCount = 4,
    kPreviewTriangleStrip = 0x98,
    kPreviewVertexFormat = 0x11F005,
    kPreviewVertexBytes = 0x18
};
enum CharaCountInput {
    kActionCancel = 6
};
enum CharaCountScript {
    kCharaCountScriptIndex = 14
};
// Search keys delimit the half-open interval returned by SearchObject_FindNext.
enum EnemyRecordRange {
    kRandomEnemyStartSearchKey = 0x33,
    kRandomEnemyEndSearchKey = 0xFB
};
enum CharaCountStrings {
    kCharaCountLayout = 0x0C,
    kCharaCountEntry = 0x18,
    kCharaCountDrawScript = 0x24
};
enum TrainingStrings {
    kTrainingCaptionEntry = 0x11,
    kTrainingBgm = 0x26
};

extern "C" {

char s_inputManagerDtorRegistration_CharacterCount[12];
char s_fadeOverlayDtorRegistration_CharacterCount[12];
char s_bpCtrlDtorRegistration_CharacterCount[12];
char s_bpCtrlHandleDtorRegistration_CharacterCount[12];
char lbl_80506A88[12];
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[];
extern char s_fadeOverlayDtorRegistration_CharacterCount[];
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_CharacterCount[];
void FadeOverlay_Init(void*);
void ResourceState_Dtor();
void* Table_GetEntry(void*, int);
void* ItemInventory_GetCharacterIdAt(void*, int);
void ItemInventory_GetDefaultPartIds(void*, void*, void*);
void* ItemInventory_GetPartIdAt(void*, int);
short ItemInventory_FindPartIndex(void*, int);
void ItemSelection_Init(void*);
void ItemSelection_BuildCharacterOutfit(void*, void*, int, int, void*, int);
void ResolvedItemSelection_Clear(void*);
void ItemSelection_ResolvePartData(void*, void*);
void ItemSelection_CopyValidatedChanges(void*, void*, void*);
void ResolvedItemSelection_Destroy(void*, int);
unsigned char PlayerRank_FindThresholdBand(unsigned char);
void* __nw__FUl(unsigned int);
void __dl__FPv(void*);
extern CSharedDoc* g_sharedDoc;
int FadeState_GetState(void*);
int Layout_CollectActivated(void*, void*, int);
void Layout_Update(void*);
void appFreeUni__FPv(void*);
void Layout_SetElementValueState(void*, int, int);
void Layout_SetElementPlayerFlag(void*, int, int, int);
int Input_IsActionPressed(void*, int, int);
void InputManager_Init(const char*);
void InputManager_Dtor();

} // extern "C"
namespace app {
template <class T> struct app_allocator {};
}
#pragma opt_strength_reduction off
namespace std {
template <class T, class Allocator> struct LocalVector_u8024745C {
    T* data;
    unsigned int size;
    unsigned int capacity;
    // Release the owned activation array.
    ~LocalVector_u8024745C() {
        if (data) {
            char* begin = (char*)data;
            char* end = begin + size;
            size -= end - begin;
            appFreeUni__FPv(data);
        }
    }
};
}
typedef std::LocalVector_u8024745C<int, app::app_allocator<int> > ActivatedList;
#pragma opt_strength_reduction reset

extern "C" {

// Construct/publish the borrowed fade singleton once; retain the legacy int-slot ABI.
static inline void EnsureCharaCountFade() {
    if (!((int&)Sing_805DC608())) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                     s_fadeOverlayDtorRegistration_CharacterCount);
            s_fadeOverlayInitialized = 1;
        }
        ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
    }
}

// Construct/publish the borrowed input singleton once; retain the legacy int-slot ABI.
static inline void EnsureCharaCountInput() {
    if (!((int&)GetInputManagerSlot())) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     s_inputManagerDtorRegistration_CharacterCount);
            s_inputManagerInitialized = 1;
        }
        ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
    }
}

// Build an enemy character slot from inventory indices, resolving defaults and eight outfit overrides.
// source/recordIndex must identify an existing enemy record; destinationAddress/contextAddress
// carry 32-bit addresses through the legacy ABI. Context is borrowed by the destination.
// Inventory indices and IFF type IDs are distinct; only converted IDs enter outfit resolution.
void EnemyData_BuildCharacterSlot(void* source, int destinationAddress, int recordIndex, int contextAddress) {
    EnemyOutfitRecord* record =
        (EnemyOutfitRecord*)Table_GetEntry(((CPangyaEnemyData*)source)->dataBuffer, recordIndex);
    void* slot = (void*)destinationAddress;
    unsigned short sourceFlags = AsCharaSlot8024745C(slot)->flags;
    AsCharaSlot8024745C(slot)->context = (void*)contextAddress;
    sourceFlags = (sourceFlags | kSlotSourceFlag) & ~kSlotClearedSourceFlag;
    AsCharaSlot8024745C(slot)->flags = sourceFlags;
    int characterInventoryIndex = record->characterInventoryIndex;
    AsCharaSlot8024745C(slot)->characterInventoryIndex =
        characterInventoryIndex; // +0 holds the character inventory index
    AsCharaSlot8024745C(slot)->hairStyle = (signed char)record->hairStyleIndex;
    AsCharaSlot8024745C(slot)->baseVariant = 0;
    AsCharaSlot8024745C(slot)->levelIndex = record->levelIndex;
    AsCharaSlot8024745C(slot)->ballInventoryIndex = record->ballInventoryIndex;
    AsCharaSlot8024745C(slot)->clubSetInventoryIndex = record->clubSetInventoryIndex;
    // Phase 1: resolve a default outfit, then apply the eight nonzero inventory overrides.
    void* characterTypeId =
        ItemInventory_GetCharacterIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), characterInventoryIndex);
    char defaultPartIds[0x60];
    ItemInventory_GetDefaultPartIds(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), characterTypeId,
                                    defaultPartIds);
    void* outfitSelection = __nw__FUl(0x68);
    if (outfitSelection)
        ItemSelection_Init(outfitSelection);
    ItemSelection_BuildCharacterOutfit(outfitSelection, characterTypeId,
                                       (unsigned char)AsCharaSlot8024745C(slot)->hairStyle,
                                       (unsigned char)AsCharaSlot8024745C(slot)->baseVariant, defaultPartIds, 0);
    void* resolvedParts = __nw__FUl(0xc8);
    if (resolvedParts)
        ResolvedItemSelection_Clear(resolvedParts);
    ItemSelection_ResolvePartData(resolvedParts, outfitSelection);
    void* partTypeId;
    partTypeId =
        ItemInventory_GetPartIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), record->partInventoryIndices[0]);
    if (partTypeId)
        ItemSelection_CopyValidatedChanges(resolvedParts, outfitSelection, partTypeId);
    partTypeId =
        ItemInventory_GetPartIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), record->partInventoryIndices[1]);
    if (partTypeId)
        ItemSelection_CopyValidatedChanges(resolvedParts, outfitSelection, partTypeId);
    partTypeId =
        ItemInventory_GetPartIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), record->partInventoryIndices[2]);
    if (partTypeId)
        ItemSelection_CopyValidatedChanges(resolvedParts, outfitSelection, partTypeId);
    partTypeId =
        ItemInventory_GetPartIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), record->partInventoryIndices[3]);
    if (partTypeId)
        ItemSelection_CopyValidatedChanges(resolvedParts, outfitSelection, partTypeId);
    partTypeId =
        ItemInventory_GetPartIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), record->partInventoryIndices[4]);
    if (partTypeId)
        ItemSelection_CopyValidatedChanges(resolvedParts, outfitSelection, partTypeId);
    partTypeId =
        ItemInventory_GetPartIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), record->partInventoryIndices[5]);
    if (partTypeId)
        ItemSelection_CopyValidatedChanges(resolvedParts, outfitSelection, partTypeId);
    partTypeId =
        ItemInventory_GetPartIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), record->partInventoryIndices[6]);
    if (partTypeId)
        ItemSelection_CopyValidatedChanges(resolvedParts, outfitSelection, partTypeId);
    partTypeId =
        ItemInventory_GetPartIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), record->partInventoryIndices[7]);
    if (partTypeId)
        ItemSelection_CopyValidatedChanges(resolvedParts, outfitSelection, partTypeId);
    // Phase 2: flatten 24 resolved keys back to inventory indices in the destination slot.
    // These are resolver slot ordinals, not the eight input override positions.
    // The resolver key is at selection+8+4*i; the inventory result at slot+0x20+2*i.
    // No body-region mapping for all 24 slots is established here; preserve order
    // and consult the resolver before adding or moving outfit overrides.
    // The temporary selection/resolution objects are then destroyed, not transferred.
    for (unsigned char i = 0; i < 0x18; ++i) {
        int* in = (int*)((char*)outfitSelection + i * 4);
        AsCharaSlot8024745C(slot)->partIndices[i] =
            ItemInventory_FindPartIndex(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), in[2]); // original +0x20
    }
    if (outfitSelection)
        __dl__FPv(outfitSelection);
    if (resolvedParts)
        ResolvedItemSelection_Destroy(resolvedParts, 1);
    // Phase 3: publish outfit flags and copy remaining rank/attribute payloads.
    // Copy map (source -> slot): +54..58 -> +9B..9F (five raw bytes),
    // +30/+34/+38 -> +68/+6C/+70 (other five words at +74..84 cleared),
    // +45..52 -> +8D..9A (14 raw bytes). Individual stat/bit meanings are
    // unresolved: these are parallel payloads, not inventory IDs or part slots.
    unsigned short outfitFlags = AsCharaSlot8024745C(slot)->flags;
    AsCharaSlot8024745C(slot)->characterIndex = record->characterIndex;
    outfitFlags |= kSlotHasOutfit;
    AsCharaSlot8024745C(slot)->flags = outfitFlags;
    for (int i = 0; i < 5; ++i)
        AsCharaSlot8024745C(slot)->characterAttributes[i] = record->characterAttributes[i]; // original +0x9B
    AsCharaSlot8024745C(slot)->caddieInventoryIndex = record->caddieInventoryIndex;
    (*(char*)&AsCharaSlot8024745C(slot)->rankTier) = PlayerRank_FindThresholdBand(record->rankValue);
    for (int i = 0; i < 8; ++i) {
        AsCharaSlot8024745C(slot)->auxiliaryValues[i] = 0; // original +0x68
        if (i < 3)
            AsCharaSlot8024745C(slot)->auxiliaryValues[i] = record->auxiliaryValues[i];
    }
    for (int i = 0; i < 0xe; ++i)
        AsCharaSlot8024745C(slot)->packedAttributes[i] = record->packedAttributes[i]; // original +0x8D
}

#pragma opt_strength_reduction off
// Wait for the fade, accept a 2/3/4-player choice, and finish after four feedback frames or cancel.
int CCharaNum::UpdatePlayerCountSelection() {
    ActivatedList list;
    list.data = 0;
    list.size = 0;
    list.capacity = 0;
    int state = this->state;
    switch (state) {
    case kCountWaitForFade:
        EnsureCharaCountFade();
        if (FadeState_GetState((void*)((int&)Sing_805DC608())) == kFadeBusy)
            return 0;
        this->state = kCountPrepare;
        break;
    case kCountPrepare:
        this->state = kCountReadInput;
        break;
    case kCountReadInput: {
        Layout_Update(layout);
        if (Layout_CollectActivated(layout, &list, 0)) {
            unsigned int i = 0;
            while (i < list.size) {
                int* item = list.data + i;
                switch (*item) {
                case kTwoPlayers:
                    Layout_SetElementValueState(layout, 0, 2);
                    Layout_SetElementPlayerFlag(layout, 0, 0, 0);
                    Layout_SetElementValueState(layout, 1, 0);
                    Layout_SetElementValueState(layout, 2, 0);
                    selectedPlayerCount = 2;
                    selectionMade = 1;
                    break;
                case kThreePlayers:
                    Layout_SetElementValueState(layout, 0, 0);
                    Layout_SetElementValueState(layout, 1, 2);
                    Layout_SetElementPlayerFlag(layout, 1, 0, 0);
                    Layout_SetElementValueState(layout, 2, 0);
                    selectedPlayerCount = 3;
                    selectionMade = 1;
                    break;
                case kFourPlayers:
                    Layout_SetElementValueState(layout, 0, 0);
                    Layout_SetElementValueState(layout, 1, 0);
                    Layout_SetElementValueState(layout, 2, 2);
                    Layout_SetElementPlayerFlag(layout, 2, 0, 0);
                    selectedPlayerCount = 4;
                    selectionMade = 1;
                    break;
                }
                ++i;
            }
        }
        if (selectedPlayerCount >= 0 && selectionFrames == kSelectionFeedbackFrames) {
            selectionComplete = 1;
            return 1;
        }
        EnsureCharaCountInput();
        if (Input_IsActionPressed((void*)((int&)GetInputManagerSlot()), kActionCancel, -1)) {
            if (!((int&)Sing_805DC608())) {
                if (!s_fadeOverlayInitialized) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                             s_fadeOverlayDtorRegistration_CharacterCount);
                    s_fadeOverlayInitialized = 1;
                }
                ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
            }
            if (FadeState_GetState((void*)((int&)Sing_805DC608())) != kFadeBusy)
                return 1;
        }
        if (selectionMade)
            ++selectionFrames;
        break;
    }
    }
    return 0;
}
#pragma opt_strength_reduction reset

extern float kCharacterCountLayoutOpacity;
struct CharaVertices {
    float v[24];
};
extern CharaVertices s_characterCountPreviewQuad;
extern CharaVertices s_characterCountOverlayQuad;
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_CharacterCount[];
extern char charaCountMenuStrings[];
void __ct__7CBpCtrlFv(const char*);
void CBpCtrl_dtor();

void* BpSprite_GetTextureDescription(void*);
void* GX_GetPrimitiveCount(int, int);
void Engine_SetFlexibleVertexFormat(int);
void DrawPrimitiveUP(int, void*, void*, int);
void Engine_SetTextureStageResource(int, int);
void Engine_SetTexturePalette(int, int);
void* IdMap_Lookup(void*, int);
void BpObject_StartScriptByName(void*, const char*, int);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);

// Draw the course preview, its overlay and the player-count menu.
void CCharaNum::Draw() {
    void* size = BpSprite_GetTextureDescription(bpTexture);
    // Slot 3 prepares the texture for this draw; its exact lower-level binding
    // contract is unresolved. Keep the dispatch index when changing texture code.
    struct Texture {
        virtual void a() DECOMP_LOCAL_VIRTUAL;
        virtual void b() DECOMP_LOCAL_VIRTUAL;
        virtual void c() DECOMP_LOCAL_VIRTUAL;
        virtual void d() DECOMP_LOCAL_VIRTUAL;
    };
    ((Texture*)bpTexture)->d();
    // Four vertices, six words each: XY/Z, packed color, UV. Word 6 is
    // vertex 1 X; 13 is vertex 2 Y; 18/19 are vertex 3 X/Y. The texture
    // description's width/height set pixel-space right/bottom edges, not UVs.
    // Template words retain Z, color and UV; overlay uses the same screen extent.
    CharaVertices vertices1 = s_characterCountPreviewQuad;
    vertices1.v[6] = (float)((unsigned int*)size)[0];
    vertices1.v[13] = (float)((unsigned int*)size)[1];
    vertices1.v[18] = (float)((unsigned int*)size)[0];
    vertices1.v[19] = (float)((unsigned int*)size)[1];
    // GX_GetPrimitiveCount returns 2 for this four-vertex strip. Its legacy declaration
    // carries that integer in a pointer slot; no vertex buffer is returned or owned.
    void* primitiveCount = GX_GetPrimitiveCount(kPreviewVertexCount, kPreviewTriangleStrip);
    Engine_SetFlexibleVertexFormat(kPreviewVertexFormat);
    DrawPrimitiveUP(kPreviewTriangleStrip, primitiveCount, &vertices1, kPreviewVertexBytes);
    Engine_SetTextureStageResource(0, 0);
    Engine_SetTexturePalette(0, 0);
    CharaVertices vertices2 = s_characterCountOverlayQuad;
    vertices2.v[6] = (float)((unsigned int*)size)[0];
    vertices2.v[13] = (float)((unsigned int*)size)[1];
    vertices2.v[18] = (float)((unsigned int*)size)[0];
    vertices2.v[19] = (float)((unsigned int*)size)[1];
    primitiveCount = GX_GetPrimitiveCount(kPreviewVertexCount, kPreviewTriangleStrip);
    Engine_SetFlexibleVertexFormat(kPreviewVertexFormat);
    DrawPrimitiveUP(kPreviewTriangleStrip, primitiveCount, &vertices2, kPreviewVertexBytes);
    if (((int&)Sing_805DC5F0()) == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_CharacterCount);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    void* item = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), bpEntryId);
    BpObject_StartScriptByName(item, charaCountMenuStrings + kCharaCountDrawScript, 1);
    BpObject_TryInvokeCurrentCallback(item, 0);
    BpObject_Draw(item);
    Layout_DrawWithOpacity(layout, kCharacterCountLayoutOpacity);
}

extern float kFloat_6_2831855;
extern float s_characterCountFullTurnAngle;
extern float s_trainingFullTurnAngle;
extern int g_soundManager;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern signed char s_bpCtrlInitialized;
extern char trainingMainStrings[];
extern char s_bpCtrl[];
extern char s_bpCtrlHandleDtorRegistration_CharacterCount[];
extern char s_bpCtrlDtorRegistration_CharacterCount[];
extern float kTrainingWarmupZeroDelta;

void SoundManager_RequestBgmStop(int, int);
void SoundManager_RequestBgmChange(int, const char*, int, int);
void MPBase1_Process(void*, float);
void IFadeMenuBase_StartMenuFadeOut(void*);
void __ct__7CBpCtrlFv(const char*);
void CBpCtrl_dtor();
int Layout_PollLoadComplete(void*);

void CBpCtrlHandle_Dtor();
void Layout_Update(void*);
short* SearchObject_FindNext(void*, int, int);
void* Table_GetEntry(void*, int);
int rand();
void __ct__18NativeBase80201D18Fv(void*);
void* __nw__FUl(unsigned int);
void __dt__18NativeBase80201D18Fv(void*, int);
void __dl__FPv(void*);
void destroy__9CTaskMainFi(void*);
extern char __vt__13CTrainingMain[];
extern char s_bpCtrlDestructorRegistration4745C[];
extern char trainingMainStrings[];
extern char s_inputManagerDtorRegistration_CharacterCount[];
extern char charaCountMenuStrings[];
extern char __vt__9CCharaNum[];
extern char coursePreviewTextureNames[];
extern char s_gameInputManager[];
extern signed char s_resourceTableInitialized;
extern int s_resourceTableStorage;
extern signed char s_inputManagerInitialized;
extern char s_bpCtrl[];
extern signed char s_bpCtrlInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern CSharedDoc* g_sharedDoc;
void MPBase1_Ctor(void*);
int ResourceTable_FindEntryByName(void*, const char*, int*);
void Layout_Init(void*, int);
void InputManager_Init(const char*);
void InputManager_Dtor();
void InputManager_SetSelectedPlayer(void*, int);
void Layout_LoadExtended(void*, int, const char*);
void Layout_Destroy(void*, int);
void Allocation_Dtor(void*);

// Initialize the character-count module angle constant to one full turn.
void CharaCount_InitFullTurnConstant() {
    s_characterCountFullTurnAngle = kFloat_6_2831855;
}
// Initialize a BP entry handle to the invalid sentinel.
void CharaCount_InitInvalidBpHandle(int* p) {
    *p = -1;
}
// Initialize the training module angle constant to one full turn.
void Training_InitFullTurnConstant() {
    s_trainingFullTurnAngle = kFloat_6_2831855;
}

// Stop training BGM when leaving the training task.
void CTrainingMain::OnDestroy() {
    SoundManager_RequestBgmStop(g_soundManager, 1);
}

struct CharaResource {
    char data[0x20];
    // Construct a texture-pack request in the allocation wrapper.
    CharaResource(const char* name, int temporary, int count) {
        ((CPackTextureResource*)this)->Init((const char*)name, (void**)temporary, (unsigned int)count);
    }
};
struct CharaLayout {
    char data[0x5c];
    // Construct a layout with the requested group count.
    CharaLayout(int n) {
        Layout_Init(this, n);
    }
};
struct CharaTexture {
    char data[0x28];
    // Initialize the BP preview texture in the allocation wrapper.
    CharaTexture() {
        ((CBpTex*)this)->__ct();
    }
};
struct CharaTextureVirtual {
    virtual void a();
    virtual void b(const char*);
};
#pragma opt_common_subs off
#pragma opt_propagation off
// Create the player-count layout, BP entry and selected-course preview texture.
CCharaNum::CCharaNum() {
    // Phase 1: initialize menu state and request its texture pack/layout.
    extern char lbl_80506A88[];
    int invalid;
    void* vtable;
    MPBase1_Ctor(this);
    vtable = __vt__9CCharaNum;
    invalid = -1;
    selectedPlayerCount = invalid;
    *(void**)this = vtable;
    (*(void**)&selectionComplete) = 0;
    layout = 0;
    bpEntryId = invalid;
    packTextureResource = 0;
    bpTexture = 0;
    int temporary = 0;
    if (((int&)Sing_805DC620()) == 0) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor,
                                     lbl_80506A88); // resource-table destructor registration node
            s_resourceTableInitialized = 1;
        }
        ((int&)Sing_805DC620()) = (int)&s_resourceTableStorage;
    }
    int count = ResourceTable_FindEntryByName((void*)((int&)Sing_805DC620()), charaCountMenuStrings, &temporary);
    void* q = new CharaResource(charaCountMenuStrings, temporary, count);
    packTextureResource = q;
    q = new CharaLayout(5);
    layout = q;
    // Phase 2: bind input to player zero, load the layout, and create its BP entry.
    EnsureCharaCountInput();
    InputManager_SetSelectedPlayer((void*)((int&)GetInputManagerSlot()), 0);
    Layout_LoadExtended(layout, 0, charaCountMenuStrings + kCharaCountLayout);
    if (((int&)Sing_805DC5F0()) == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_CharacterCount);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    ((CBpCtrl*)(((int&)Sing_805DC5F0())))->CreateEntry(charaCountMenuStrings + kCharaCountEntry, &bpEntryId, 0);
    (*(void**)&selectionMade) = 0;
    (*(void**)&selectionFrames) = 0;
    (*(void**)&state) = 0;
    q = new CharaTexture();
    bpTexture = q;
    // Phase 3: start the course-indexed preview request; polling happens after construction.
    int idx = (*(int*)&AsCSharedDoc(g_sharedDoc)->menuTextureIndex);
    ((CharaTextureVirtual*)q)->b(*(const char**)(coursePreviewTextureNames + idx * 4));
}
#pragma opt_common_subs reset
#pragma opt_propagation reset

// Release the player-count layout, BP entry and preview resources; optionally free the object.
CCharaNum::~CCharaNum() {
    *(void**)this = __vt__9CCharaNum;
    void* q = layout;
    if (q) {
        Layout_Destroy(q, 1);
        layout = 0;
    }
    if (bpEntryId != -1) {
        if (((int&)Sing_805DC5F0()) == 0) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_CharacterCount);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(bpEntryId);
        bpEntryId = -1;
    }
    if (((int&)Sing_805DC5F8()) == 0) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                     s_bpCtrlHandleDtorRegistration_CharacterCount);
            s_bpCtrlHandleInitialized = 1;
        }
        ((int&)Sing_805DC5F8()) = (int)s_bpCtrlHandle;
    }
    ((CBpCtrlHandle*)(((int&)Sing_805DC5F8())))->ReleaseEntry();
    q = packTextureResource;
    if (q) {
        if (q) {
            struct Dtor {
                virtual void a(int) DECOMP_LOCAL_VIRTUAL;
                virtual void b(int) DECOMP_LOCAL_VIRTUAL;
                virtual void c(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((Dtor*)q)->a(1);
        }
        packTextureResource = 0;
    }
    q = bpTexture;
    if (q) {
        if (q) {
            struct Dtor {
                virtual void a(int) DECOMP_LOCAL_VIRTUAL;
                virtual void b(int) DECOMP_LOCAL_VIRTUAL;
                virtual void c(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((Dtor*)q)->a(1);
        }
        bpTexture = 0;
    }
}

// Copy the selected player count only after the selection has completed.
// Returns 1 and writes *out for a completed choice; returns 0 without touching *out otherwise.
int CCharaNum::GetSelectedPlayerCount(int* out) {
    int* p = (int*)this;
    if (AsCCharaNum(p)->selectionComplete != 0) {
        *out = AsCCharaNum(p)->selectedPlayerCount;
        return 1;
    }
    return 0;
}

// Advance the player-count layout and poll its texture pack.
void CCharaNum::UpdateWhileWaiting() {
    Layout_Update(layout);
    ((CPackTextureResource*)packTextureResource)->PollLoad();
}

// Start training BGM and process course selection until its base task is initialized.
void CTrainingMain::OnLoad() {
    SoundManager_RequestBgmChange(g_soundManager, trainingMainStrings + kTrainingBgm, 0, 1);
    void* task;
    while (((MPBase1Layout*)(task = courseSelect))->initialized == 0) // original +8
        MPBase1_Process(task, kTrainingWarmupZeroDelta);
}

// Choose a matching enemy via rand() % matchCount, then build its destination slot.
// Requires at least one match: division/remainder happen before the record-zero fallback.
// Search keys 0x33/0xFB resolve a half-open record-index interval; they are not indices.
void EnemyData_SelectRandomMatchingRecord(void* enemyData, int destinationAddress, int levelIndex,
                                          int characterInventoryIndex, int contextAddress) {
    short firstRecordIndex = *SearchObject_FindNext(enemyData, kRandomEnemyStartSearchKey, 0);
    int matchCount = 0;
    for (int i = firstRecordIndex; i < *SearchObject_FindNext(enemyData, kRandomEnemyEndSearchKey, 0); ++i) {
        EnemyOutfitRecord* record = (EnemyOutfitRecord*)Table_GetEntry(((CPangyaEnemyData*)enemyData)->dataBuffer, i);
        if ((signed char)record->levelIndex == levelIndex && characterInventoryIndex == record->characterInventoryIndex)
            ++matchCount;
    }
    int randomValue = rand();
    int quotient = randomValue / matchCount;
    const int chosenMatch = randomValue % matchCount;
    int seen = 0;
    short scanStartIndex = *SearchObject_FindNext(enemyData, kRandomEnemyStartSearchKey, 0);
    for (int i = scanStartIndex; i < *SearchObject_FindNext(enemyData, kRandomEnemyEndSearchKey, 0); ++i) {
        EnemyOutfitRecord* record = (EnemyOutfitRecord*)Table_GetEntry(((CPangyaEnemyData*)enemyData)->dataBuffer, i);
        if ((signed char)record->levelIndex == levelIndex &&
            characterInventoryIndex == record->characterInventoryIndex) {
            if (seen == chosenMatch) {
                EnemyData_BuildCharacterSlot(enemyData, destinationAddress, i, contextAddress);
                return;
            }
            ++seen;
        }
    }
    EnemyData_BuildCharacterSlot(enemyData, destinationAddress, 0, 0);
}

struct QObj {
    char data[0x80];
    QObj();
};
// Construct the course selector in the allocation wrapper.
QObj::QObj() {
    ((CCourseSelect*)this)->__ct();
}
// Create the training course selector and its background/caption BP entries.
CTrainingMain::CTrainingMain() {
    CTrainingMain* self = this;
    __ct__18NativeBase80201D18Fv(this);
    self->vtbl = __vt__13CTrainingMain;
    self->taskMainVtable = (void*)(__vt__13CTrainingMain + 0x3C);
    self->courseSelect = 0;
    self->backgroundEntryId = (void*)-1;
    self->captionEntryId = (void*)-1;
    self->scriptsStarted = 0;
    void* q = new QObj();
    self->courseSelect = q;
    ((CCourseSelect*)q)->SetWeatherSelectionEnabled(1);
    if (((int&)Sing_805DC5F0()) == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDestructorRegistration4745C);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    ((CBpCtrl*)(((int&)Sing_805DC5F0())))->CreateEntry(trainingMainStrings, (int*)&self->backgroundEntryId, 0);
    if (((int&)Sing_805DC5F0()) == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDestructorRegistration4745C);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    ((CBpCtrl*)(((int&)Sing_805DC5F0())))
        ->CreateEntry(trainingMainStrings + kTrainingCaptionEntry, (int*)&self->captionEntryId, 0);
}

// Destroy the training course selector and both BP entries; optionally free the task.
CTrainingMain::~CTrainingMain() {
    void* child = courseSelect;
    *(void**)this = __vt__13CTrainingMain;
    taskMainVtable = __vt__13CTrainingMain + 0x3C;
    if (child) {
        if (child) {
            struct Dtor {
                virtual void a(int) DECOMP_LOCAL_VIRTUAL;
                virtual void b() DECOMP_LOCAL_VIRTUAL;
                virtual void c() DECOMP_LOCAL_VIRTUAL;
            };
            ((Dtor*)child)->a(1);
        }
        courseSelect = 0;
    }
    int* entryIdPtr = (int*)(((char*)&backgroundEntryId));
    if (*entryIdPtr != -1) {
        if (((int&)Sing_805DC5F0()) == 0) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDestructorRegistration4745C);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(*entryIdPtr);
        *entryIdPtr = -1;
    }
    entryIdPtr = (int*)(((char*)&captionEntryId));
    if (*entryIdPtr != -1) {
        if (((int&)Sing_805DC5F0()) == 0) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDestructorRegistration4745C);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(*entryIdPtr);
        *entryIdPtr = -1;
    }
    if (this)
        __dt__18NativeBase80201D18Fv(this, 0);
}

// Select the player-count BP script and release the temporary texture-pack loader.
void CCharaNum::StartCharaCountMenu() {
    if (((int&)Sing_805DC5F8()) == 0) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                     s_bpCtrlHandleDtorRegistration_CharacterCount);
            s_bpCtrlHandleInitialized = 1;
        }
        ((int&)Sing_805DC5F8()) = (int)s_bpCtrlHandle;
    }
    ((CBpCtrlHandle*)(((int&)Sing_805DC5F8())))->SetScriptIndex(kCharaCountScriptIndex);
    void* q = packTextureResource;
    if (q) {
        if (q) {
            struct Dtor {
                virtual void a(int) DECOMP_LOCAL_VIRTUAL;
                virtual void b() DECOMP_LOCAL_VIRTUAL;
                virtual void c() DECOMP_LOCAL_VIRTUAL;
            };
            ((Dtor*)q)->a(1);
        }
        packTextureResource = 0;
    }
    IFadeMenuBase_StartMenuFadeOut(this);
}

// Report ready only when the texture pack, BP entries, layout and preview texture are loaded.
int CCharaNum::PollLoadComplete() {
    int result = ((CPackTextureResource*)packTextureResource)->IsLoadFinished() & 1;
    if (((int&)Sing_805DC5F0()) == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_CharacterCount);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    result &= ((CBpCtrl*)(((int&)Sing_805DC5F0())))->PollAllEntriesLoaded();
    result &= Layout_PollLoadComplete(layout);
    void* q = bpTexture;
    struct Dtor6 {
        virtual void a() DECOMP_LOCAL_VIRTUAL;
        virtual void b() DECOMP_LOCAL_VIRTUAL;
        virtual void c() DECOMP_LOCAL_VIRTUAL;
        virtual void d() DECOMP_LOCAL_VIRTUAL;
        virtual int e() DECOMP_LOCAL_VIRTUAL;
        virtual int f() DECOMP_LOCAL_VIRTUAL;
        virtual int g() DECOMP_LOCAL_VIRTUAL;
    };
    result &= ((Dtor6*)q)->e();
    return result;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804622BC", _ctors_u8024745C_0, CharaCount_InitFullTurnConstant);
STATIC_INIT_ENTRY(".CRT$XCU804622BC", _ctors_u8024745C_1, Training_InitFullTurnConstant);
#pragma pop
