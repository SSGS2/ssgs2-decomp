// Character selection: load gating, selection-screen reset, controller-to-player
// assignment, avatar cleanup, and unlocked-character input availability.
// Edit index: reset/player panels -> CCharaSelect::InitializeSelectionScreen and CharaSelectLayout;
// character/save buttons -> UpdateSlotInputAvailability; async admission ->
// PollLoadComplete; resource pumping -> UpdateLayoutAndTexture.
// Related schemas: CCharaSelect.h, SavePlayerRecord.h, CEnemyRenderRecord.h;
// scripts/panel layout -> charaSelectStrings (the data section).
// Controllers 0..3, player slots 0..3, and character indices 0..7 are distinct.
// There is no frame-rate setting here; availability is recomputed from save
// equipment flags, playerStates, controller lists, gameMode and partyGameMode.
#include "classes/CCharaSelect.h"
#include "classes/CEnemyRenderTable.h"
#include "classes/CEnemyRenderRecord.h"
#define AsCEnemyRenderTable(p) ((CEnemyRenderTable*)(p))
#include "classes/CSharedDoc.h"
#include "classes/SavePlayerRecord.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/vector.h"

#pragma use_lmw_stmw on

#include "game/singletons.h"
#include <decomp/cw_bitfields.h>
#include "classes/CPackTextureResource.h"
#include "classes/CBpCtrlHandle.h"
#include "classes/CCharaSelect.h"
#include "classes/CCharaSelect.h"
#include "classes/CBpCtrl.h"
extern "C" {

void RenderContext_ResetPlayerRecords(void*);
void* CharacterPreviewTable_dtor(void*, int);
void appFreeUni__FPv(void*);

struct CharaSelectionVector {
    unsigned int* data;
    unsigned int count, capacity;
};
// Four controller vectors hold player-slot indices, not character IDs. The render
// table is indexed by player slot; a record supplies avatar id and flags bit15.
// The shared context owns saved records, vectors and avatars. Reset marks assigned
// slots first, then destroys avatar objects and reloads save data for unassigned slots.
struct CharaRenderContext {
    SavePlayerRecord playerRecords[4];
    CharaSelectionVector selections[4];
    CEnemyRenderTable renderTable;
};
typedef char CharaRenderContext_tableOffset[((int)&((CharaRenderContext*)0)->renderTable == 0x20D0) ? 1 : -1];
static const int kControllerSelectionsOffset = (int)&((CharaRenderContext*)0)->selections;
static const int kRenderRecordsOffset = (int)&((CharaRenderContext*)0)->renderTable;

struct RenderContextConstructionCleanup_u80244B74 {
    char* base;
    bool active;
    // Guard the newly constructed context members during player-record initialization.
    explicit RenderContextConstructionCleanup_u80244B74(void* object) : base((char*)object), active(true) {}
    // Unwind constructed render records and selection vectors unless disarmed.
    ~RenderContextConstructionCleanup_u80244B74() {
        if (active) {
            CharacterPreviewTable_dtor(base + kRenderRecordsOffset, -1);
            for (int index = 3; index >= 0; --index) {
                unsigned int* vector =
                    (unsigned int*)(base + kControllerSelectionsOffset + index * (int)sizeof(CharaSelectionVector));
                if (vector[0]) {
                    vector[1] = 0;
                    appFreeUni__FPv((void*)vector[0]);
                }
            }
        }
    }
};

// Reset player records with cleanup of partially constructed render-context members on failure.
static void InitializeNewRenderContext(void* base) {
    RenderContextConstructionCleanup_u80244B74 cleanup(base);
    RenderContext_ResetPlayerRecords(base);
    cleanup.active = false;
}

extern void Layout_Update(void*);
extern int Layout_PollLoadComplete(void*);
extern void CBpCtrl_dtor(void*);
extern void* __register_global_object(void*, void*, void*);

extern char* RenderContext_GetInstance();
extern void Layout_SetElementPlayerFlagsAndState(void*, int, int);
extern void Layout_ReadElementRect(void*, int, void*);
extern void Layout_WriteElementRect(void*, int, void*);
extern void IFadeMenuBase_StartMenuFadeOut(void*);
extern void* CBpCtrl_GetInstance();
extern void* IdMap_Lookup(void*, void*);
extern void* UIGroup_GetOrCreateById(void*, int);
extern void UIGroup_SetEnabled(void*, int);
extern void BpObject_StartScriptByName(void*, char*, int);
extern unsigned int Vector_GetCount(void*);
extern unsigned int* WordVector_GetElementAddress(void*, unsigned int);
extern void* RenderContext_GetOrCreate();
extern void SaveData_CopyCurrentRecordToArray(void*, int);
extern void LayoutIntVector_PushBack(void*, const int*);
extern void* SharedDoc_GetCurrent();
extern void Layout_SetElementSecondaryState(void*, int, int);
extern void Layout_SetElementPlayerFlag(void*, int, int, int);
extern void Layout_SetElementEnabled(void*, int, int);
extern void SaveData_InitPlayerRecord(void*);
extern void __construct_array(void*, void*, void*, int, int);

extern void PartyItemRecord_InitDefaults(void*);
extern void PlayerRenderRecord_dtor(void*);
extern void __dt__13RenderContextFv(void*);
extern void RenderContext_ResetPlayerRecords(void*);
extern signed char s_renderContextInitialized;
extern CSharedDoc* g_sharedDoc;
extern char s_renderContext[];
extern char s_renderContextRegistrations_805041E8[];

extern char charaSelectStrings[];
// Initial integer selection appended when the random-character slot is installed.
extern int kCharacterRandomSelectionInitialValue;
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_805041C4[];

// The texture handle occupies the existing unexposed +0x30 slot in CCharaSelect.
struct CharaSelectTextureHandle {
    char prefix[0x30];
    void* texture;
};
struct CharaPlayerTransition {
    int state, elapsed;
    int duration;
};
enum CharaSelectLayout {
    kFirstPlayerChoice = 0x2A,
    kFirstPlayerOption = 0x2E,
    kFirstCharacterIcon = 0x21,
    kFirstCharacterHighlight = 0x18,
    kFirstLockedCharacterIcon = 0x0F,
    kLoadSaveDataIcon = 0x29,
    kLoadSaveDataHighlight = 0x20,
    kAllPlayers = 5,
    kPlayerCount = 4,
    kSelectableCharacterCount = 8,
    kLayoutElementCount = 0x52,
    kCharaSelectScriptIndex = 0x0E,
    kRandomSelectionMode = 6,
    kModeWithoutSecondaryChoice = 7,
    kOneSelectionPerController = 3
};
// Layout relationship map: icons 0x21..0x28 -> highlights 0x18..0x1F (icon-9);
// save icon 0x29 -> highlight 0x20. Player choices 0x2A..0x2D and secondary
// choices 0x2E..0x31 are indexed by player slot. IDs 0x32..0x39 are reset here,
// but their visible role is unresolved. Groups 0x410/0x411/0x412 are hidden for
// the extra player panels at 1/2/3-player counts; selection scripts handle the rest.
// The script literals are colorsprite and C_NO_ENT_3/2/1/0 in the shared string pool.
struct CharaSelectScriptPool {
    char earlierStrings[0xD1];
    char startSelection[12];
    char onePlayer[11], twoPlayers[11], threePlayers[11], fourPlayers[11];
    char remainingStrings[0x1A0 - 0x109];
};
typedef char CharaSelectScriptPool_size[(sizeof(CharaSelectScriptPool) == 0x1A0) ? 1 : -1];
// Derive offsets from the pool layout; retaining byte addition preserves script-pointer lifetimes.
static const int kStartSelectionScript = (int)&((CharaSelectScriptPool*)0)->startSelection;
static const int kOnePlayerScript = (int)&((CharaSelectScriptPool*)0)->onePlayer;
static const int kTwoPlayerScript = (int)&((CharaSelectScriptPool*)0)->twoPlayers;
static const int kThreePlayerScript = (int)&((CharaSelectScriptPool*)0)->threePlayers;
static const int kFourPlayerScript = (int)&((CharaSelectScriptPool*)0)->fourPlayers;

struct CharaCleanupObject {
    virtual ~CharaCleanupObject();
};

#pragma opt_propagation off
// Align each character/save highlight with its icon in layout pixel coordinates.
// The caller owns the four-integer rectangle scratch; layout calls borrow it only.
static inline void AlignCharacterSelectionHighlightRects(CCharaSelect* self, int (&elementRect)[4]) {
    Layout_ReadElementRect(self->layout, kFirstCharacterIcon, elementRect);
    Layout_WriteElementRect(self->layout, kFirstCharacterHighlight, elementRect);
    Layout_ReadElementRect(self->layout, kFirstCharacterIcon + 1, elementRect);
    Layout_WriteElementRect(self->layout, kFirstCharacterHighlight + 1, elementRect);
    Layout_ReadElementRect(self->layout, kFirstCharacterIcon + 2, elementRect);
    Layout_WriteElementRect(self->layout, kFirstCharacterHighlight + 2, elementRect);
    Layout_ReadElementRect(self->layout, kFirstCharacterIcon + 3, elementRect);
    Layout_WriteElementRect(self->layout, kFirstCharacterHighlight + 3, elementRect);
    Layout_ReadElementRect(self->layout, kFirstCharacterIcon + 4, elementRect);
    Layout_WriteElementRect(self->layout, kFirstCharacterHighlight + 4, elementRect);
    Layout_ReadElementRect(self->layout, kFirstCharacterIcon + 5, elementRect);
    Layout_WriteElementRect(self->layout, kFirstCharacterHighlight + 5, elementRect);
    Layout_ReadElementRect(self->layout, kFirstCharacterIcon + 6, elementRect);
    Layout_WriteElementRect(self->layout, kFirstCharacterHighlight + 6, elementRect);
    Layout_ReadElementRect(self->layout, kFirstCharacterIcon + 7, elementRect);
    Layout_WriteElementRect(self->layout, kFirstCharacterHighlight + 7, elementRect);
    Layout_ReadElementRect(self->layout, kLoadSaveDataIcon, elementRect);
    Layout_WriteElementRect(self->layout, kLoadSaveDataHighlight, elementRect);
}

// Reset the selection screen, rebuild controller assignments and avatars, then refresh available choices.
// Call only after PollLoadComplete succeeds and BP/layout entries exist. The
// shared render count is 0..4 (normal screens use 1..4); list values must be
// player indices 0..3. Assigned records supply avatar IDs, not controller IDs.
// No bounds/loading checks here. Existing assigned avatars stay owned by the
// context; only unassigned avatars are deleted. All layout/script state is reset.
void CCharaSelect::InitializeSelectionScreen() {
    int elementRect[4];
    int assignedSlots[4];
    char* scriptStrings;
    int* assignedSlot;
    // Reused cursor: player-slot count in phase 1; render-record byte offset in phase 4.
    int playerSlotCursor;
    int contextByteOffset;
    int outerChoiceIndex;
    unsigned int innerChoiceIndex;
    int playerSlot;
    int avatarId;
    int cleanupIndex;
    char* renderContext;
    // Phase 1: reset player slots and hide layout controls outside the active count.
    ((CBpCtrlHandle*)(CBpCtrlHandle::GetInstance()))->SetScriptIndex(kCharaSelectScriptIndex);
    playerSlotCursor = 0;
    for (; playerSlotCursor < kPlayerCount; playerSlotCursor++) {
        ResetPlayerSlot(playerSlotCursor);
    }
    playerSlotCursor = (&((CharaRenderContext*)RenderContext_GetInstance())->renderTable)->count;
    for (; playerSlotCursor < kPlayerCount; playerSlotCursor++) {
        Layout_SetElementPlayerFlagsAndState(layout, playerSlotCursor + kFirstPlayerChoice, 0);
        Layout_SetElementPlayerFlagsAndState(layout, playerSlotCursor + kFirstPlayerOption, 0);
        Layout_SetElementPlayerFlagsAndState(layout, playerSlotCursor, 0);
    }
    Layout_SetElementPlayerFlagsAndState(layout, 0x32, 0);
    Layout_SetElementPlayerFlagsAndState(layout, 0x33, 0);
    Layout_SetElementPlayerFlagsAndState(layout, 0x34, 0);
    Layout_SetElementPlayerFlagsAndState(layout, 0x35, 0);
    Layout_SetElementPlayerFlagsAndState(layout, 0x36, 0);
    Layout_SetElementPlayerFlagsAndState(layout, 0x37, 0);
    Layout_SetElementPlayerFlagsAndState(layout, 0x38, 0);
    Layout_SetElementPlayerFlagsAndState(layout, 0x39, 0);
    // Phase 2: align each highlight rect with its icon; these are paired element IDs.
    AlignCharacterSelectionHighlightRects(this, elementRect);
    ((CharaPlayerTransition*)playerTransitions)[0].elapsed = 0;
    ((CharaPlayerTransition*)playerTransitions)[0].state = 0;
    ((CharaPlayerTransition*)playerTransitions)[1].elapsed = 0;
    ((CharaPlayerTransition*)playerTransitions)[1].state = 0;
    ((CharaPlayerTransition*)playerTransitions)[2].elapsed = 0;
    ((CharaPlayerTransition*)playerTransitions)[2].state = 0;
    ((CharaPlayerTransition*)playerTransitions)[3].elapsed = 0;
    ((CharaPlayerTransition*)playerTransitions)[3].state = 0;
    IFadeMenuBase_StartMenuFadeOut(this);
    void* bpController = CBpCtrl_GetInstance();
    void* primaryBpEntry = IdMap_Lookup(bpController, primaryBpEntryId);
    scriptStrings = charaSelectStrings;
    BpObject_StartScriptByName(primaryBpEntry, scriptStrings + kStartSelectionScript, 1);

    // Phase 3: select the player-count script and hide unused player panels.
    int activePlayerCount = (&((CharaRenderContext*)RenderContext_GetInstance())->renderTable)->count;
    switch (activePlayerCount) {
    case 1: {
        bpController = CBpCtrl_GetInstance();
        void* c1 = IdMap_Lookup(bpController, primaryBpEntryId);
        void* d1 = UIGroup_GetOrCreateById(c1, 0x410);
        UIGroup_SetEnabled(d1, 0);
        bpController = CBpCtrl_GetInstance();
        void* c2 = IdMap_Lookup(bpController, primaryBpEntryId);
        void* d2 = UIGroup_GetOrCreateById(c2, 0x411);
        UIGroup_SetEnabled(d2, 0);
        bpController = CBpCtrl_GetInstance();
        void* c3 = IdMap_Lookup(bpController, primaryBpEntryId);
        void* d3 = UIGroup_GetOrCreateById(c3, 0x412);
        UIGroup_SetEnabled(d3, 0);
        bpController = CBpCtrl_GetInstance();
        void* c4 = IdMap_Lookup(bpController, secondaryBpEntryId);
        BpObject_StartScriptByName(c4, scriptStrings + kOnePlayerScript, 1);
        Layout_SetElementPlayerFlagsAndState(layout, 0x2b, 0);
        Layout_SetElementPlayerFlagsAndState(layout, 0x2f, 0);
        Layout_SetElementPlayerFlagsAndState(layout, 0x2c, 0);
        Layout_SetElementPlayerFlagsAndState(layout, 0x30, 0);
        Layout_SetElementPlayerFlagsAndState(layout, 0x2d, 0);
        Layout_SetElementPlayerFlagsAndState(layout, 0x31, 0);
    } break;
    case 2: {
        bpController = CBpCtrl_GetInstance();
        void* c1 = IdMap_Lookup(bpController, primaryBpEntryId);
        void* d1 = UIGroup_GetOrCreateById(c1, 0x411);
        UIGroup_SetEnabled(d1, 0);
        bpController = CBpCtrl_GetInstance();
        void* c2 = IdMap_Lookup(bpController, primaryBpEntryId);
        void* d2 = UIGroup_GetOrCreateById(c2, 0x412);
        UIGroup_SetEnabled(d2, 0);
        bpController = CBpCtrl_GetInstance();
        void* c3 = IdMap_Lookup(bpController, secondaryBpEntryId);
        BpObject_StartScriptByName(c3, scriptStrings + kTwoPlayerScript, 1);
        Layout_SetElementPlayerFlagsAndState(layout, 0x2c, 0);
        Layout_SetElementPlayerFlagsAndState(layout, 0x30, 0);
        Layout_SetElementPlayerFlagsAndState(layout, 0x2d, 0);
        Layout_SetElementPlayerFlagsAndState(layout, 0x31, 0);
    } break;
    case 3: {
        bpController = CBpCtrl_GetInstance();
        void* c1 = IdMap_Lookup(bpController, primaryBpEntryId);
        void* d1 = UIGroup_GetOrCreateById(c1, 0x412);
        UIGroup_SetEnabled(d1, 0);
        bpController = CBpCtrl_GetInstance();
        void* c2 = IdMap_Lookup(bpController, secondaryBpEntryId);
        BpObject_StartScriptByName(c2, scriptStrings + kThreePlayerScript, 1);
        Layout_SetElementPlayerFlagsAndState(layout, 0x2d, 0);
        Layout_SetElementPlayerFlagsAndState(layout, 0x31, 0);
    } break;
    case 4: {
        bpController = CBpCtrl_GetInstance();
        void* c1 = IdMap_Lookup(bpController, secondaryBpEntryId);
        BpObject_StartScriptByName(c1, scriptStrings + kFourPlayerScript, 1);
    } break;
    default:
        break;
    }

    // playerSlotCursor changes to a byte offset below; contextByteOffset walks vector headers.
    renderContext = RenderContext_GetInstance();
    int renderRecordCount = (&((CharaRenderContext*)renderContext)->renderTable)->count; // original +0x2360
    if (renderRecordCount != 0) {
        assignedSlots[0] = 0;
        assignedSlots[1] = 0;
        assignedSlots[2] = 0;
        assignedSlots[3] = 0;
        int* assignedSlotsBegin = assignedSlots;
        outerChoiceIndex = 0;
        contextByteOffset = 0;
        int assignedValue = 1;
        // Phase 4: each controller's vector maps to player-slot records; one
        // controller may own several slots. contextByteOffset advances by vector
        // size here, while playerSlotCursor indexes render-record storage.
        for (; outerChoiceIndex < kPlayerCount; outerChoiceIndex++) {
            innerChoiceIndex = 0;
            while (innerChoiceIndex <
                   Vector_GetCount((char*)((CharaRenderContext*)RenderContext_GetInstance())->selections +
                                   contextByteOffset)) {
                renderContext = RenderContext_GetInstance();
                unsigned int* selectedSlot = WordVector_GetElementAddress(
                    (char*)((CharaRenderContext*)renderContext)->selections + contextByteOffset, innerChoiceIndex);
                playerSlot = *selectedSlot;
                playerSlotCursor = playerSlot * (int)sizeof(CEnemyRenderRecord);
                renderContext = RenderContext_GetInstance();
                void* renderRecord = ((CharaRenderContext*)renderContext)->renderTable.recordStorage + playerSlotCursor;
                avatarId = ((CEnemyRenderRecord*)renderRecord)->characterInventoryIndex;
                renderContext = RenderContext_GetInstance();
                unsigned short recordFlags =
                    ((CEnemyRenderRecord*)(((CharaRenderContext*)renderContext)->renderTable.recordStorage +
                                           playerSlotCursor))
                        ->flags; // original +0x20EE
                int computerPlayer = (recordFlags >> 15) & 1;
                SetRandomSlot(playerSlot, outerChoiceIndex, computerPlayer, 0);
                SetupSlotAvatar(playerSlot, outerChoiceIndex, avatarId);
                Layout_SetElementPlayerFlagsAndState(layout, playerSlot + kFirstPlayerChoice, 0);
                Layout_SetElementPlayerFlagsAndState(layout, playerSlot + kFirstPlayerOption, 0);
                assignedSlots[playerSlot] = 1;
                innerChoiceIndex++;
            }
            contextByteOffset += (int)sizeof(CharaSelectionVector);
        }
        // Phase 5: assignedSlots is now complete. Sweep all four player slots;
        // contextByteOffset now advances by render-record size, not vector size.
        // Delete only unassigned avatars, clear their pointers, reload save records.
        assignedSlot = assignedSlots;
        cleanupIndex = 0;
        contextByteOffset = 0;
        int clearValue = 0;
        for (; cleanupIndex < kPlayerCount; cleanupIndex++) {
            if (*assignedSlot == 0) {
                renderContext = RenderContext_GetInstance();
                int avatarAddress =
                    (int)((CEnemyRenderRecord*)(((CharaRenderContext*)renderContext)->renderTable.recordStorage +
                                                contextByteOffset))
                        ->renderObject; // original +0x2170
                if (avatarAddress != 0) {
                    renderContext = RenderContext_GetInstance();
                    delete (CharaCleanupObject*)((CEnemyRenderRecord*)(((CharaRenderContext*)renderContext)
                                                                           ->renderTable.recordStorage +
                                                                       contextByteOffset))
                        ->renderObject;
                    renderContext = RenderContext_GetInstance();
                    ((CEnemyRenderRecord*)(((CharaRenderContext*)renderContext)->renderTable.recordStorage +
                                           contextByteOffset))
                        ->renderObject = 0;
                }
                void* currentSharedObject = RenderContext_GetOrCreate();
                SaveData_CopyCurrentRecordToArray(currentSharedObject, cleanupIndex);
            }
            contextByteOffset += (int)sizeof(CEnemyRenderRecord);
            assignedSlot++;
        }
    }

    // Phase 6: refresh controller masks, install random mode if requested, clear secondary states.
    UpdateSlotInputAvailability();
    void* currentSharedObject = SharedDoc_GetCurrent();
    unsigned int gameMode = AsCSharedDoc(currentSharedObject)->gameMode; // SharedDoc_GetInstance, original +0x808
    if (gameMode == kRandomSelectionMode) {
        Layout_SetElementPlayerFlagsAndState(layout, kFirstPlayerChoice, 0);
        renderContext = RenderContext_GetInstance();
        LayoutIntVector_PushBack(((CharaRenderContext*)renderContext)->selections,
                                 &kCharacterRandomSelectionInitialValue);
        SetRandomSlot(0, 0, 0, 1);
    }
    for (cleanupIndex = 0; cleanupIndex < kLayoutElementCount; cleanupIndex++) {
        // Clear the secondary state word for every layout element.
        Layout_SetElementSecondaryState(layout, cleanupIndex, 0);
    }
}

#pragma opt_propagation reset

// Update the selection layout and poll the character-selection texture resource.
void CCharaSelect::UpdateLayoutAndTexture() {
    Layout_Update(layout);
    ((CPackTextureResource*)(((CharaSelectTextureHandle*)this)->texture))->PollLoad();
}

struct CRecBits {
    CW_BITS3(unsigned char reservedHighBit : 1, signed char characterSelectable : 1, unsigned char remainingFlags : 6)
};
struct SavedOutfitSelection {
    CRecBits flags;
    char equipment[0x33];
};
struct CharaSaveRecordView {
    char inventory[0x2CC];
    SavedOutfitSelection characterEquipment[8];
    char remainingRecords[sizeof(SavePlayerRecord) - 0x46C];
};
static const int kSavedCharacterEquipmentOffset = (int)&((CharaSaveRecordView*)0)->characterEquipment;
typedef char CharaSaveRecordView_size[(sizeof(CharaSaveRecordView) == sizeof(SavePlayerRecord)) ? 1 : -1];

struct ControllerSelectionList {
    int* items;
    unsigned int count;
    int capacity;
};

struct CharaRenderContextView {
    SavePlayerRecord playerRecords[4];
    ControllerSelectionList lists[4];
    CEnemyRenderRecord entries[4];
    int count;
};

// Lazily construct the shared render context and register its destructor.
// Returns a borrowed CharaRenderContext; the records at its front are save copies.
// First use constructs four save records/vectors/render entries and registers
// global destruction; callers must not free the returned shared context.
static inline void* GetSharedCharaRenderContext() {
    if (Sing_805DC648() == 0) {
        if (s_renderContextInitialized == 0) {
            char* base = s_renderContext;
            char* playerRecord = base;
            char* playerRecordsEnd = base + kControllerSelectionsOffset;
            char* renderTable;
            do {
                SaveData_InitPlayerRecord(playerRecord);
                playerRecord += (int)sizeof(SavePlayerRecord);
            } while (playerRecord < playerRecordsEnd);
            ((IntVectorArray*)(playerRecordsEnd))->__ct();
            renderTable = base + kRenderRecordsOffset;
            __construct_array(renderTable, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor,
                              (int)sizeof(CEnemyRenderRecord), kPlayerCount);
            AsCEnemyRenderTable(renderTable)->count = 0;
            InitializeNewRenderContext(base);
            __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv,
                                     s_renderContextRegistrations_805041E8);
            s_renderContextInitialized = 1;
        }
        Sing_805DC648() = s_renderContext;
    }
    return Sing_805DC648();
}

#pragma opt_lifetimes off
// Set per-controller input flags and show owned or locked outfit choices for the current selections.
// Requires loaded layout, valid shared doc/context; render count 0..4; every
// controller list has count<=capacity and readable player-slot values 0..3.
// Each player has eight 0x34-byte equipment records starting at save+0x2CC.
// CRecBits.characterSelectable is byte bit 6 (signed one-bit value, tested only for !=0).
// It means this character can be selected from that player's saved equipment;
// it does not describe which character is currently selected or an outfit ID.
// Policy: playerStates!=0 disables that player's controls; last list entry with
// state==1 is the controller's active chooser. partyGameMode==3 locks further
// assignment once a controller owns any slot. An icon shows unlocked if ANY
// active player's bit6 is set; input permission still tests the active chooser's
// own bit. Empty lists grant no character input. COM (render flags bit15) blocks
// loading save data; gameMode 6 also suppresses enabling the save-data button.
// No state or ownership changes outside layout flags; no validation/failure code.
void CCharaSelect::UpdateSlotInputAvailability() {
    int playerSlot;
    int outerChoiceIndex;
    int innerChoiceIndex;
    int selectedSlot;
    int characterEquipmentByteOffset;
    int characterAvailable;

    // Phase 1: expose only active player-slot controls.
    for (playerSlot = 0; playerSlot < ((CharaRenderContextView*)GetSharedCharaRenderContext())->count; playerSlot++) {
        Layout_SetElementPlayerFlag(layout, playerSlot + kFirstPlayerChoice, 1, kAllPlayers);
        if (AsCSharedDoc(g_sharedDoc)->gameMode == kModeWithoutSecondaryChoice) {
            Layout_SetElementPlayerFlag(layout, playerSlot + kFirstPlayerOption, 0, kAllPlayers);
        } else {
            Layout_SetElementPlayerFlag(layout, playerSlot + kFirstPlayerOption, 1, kAllPlayers);
        }
    }

    for (playerSlot = ((CharaRenderContextView*)GetSharedCharaRenderContext())->count; playerSlot < kPlayerCount;
         playerSlot++) {
        Layout_SetElementPlayerFlag(layout, playerSlot + kFirstPlayerChoice, 0, kAllPlayers);
        Layout_SetElementPlayerFlag(layout, playerSlot + kFirstPlayerOption, 0, kAllPlayers);
    }

    // outerChoiceIndex = controller 0..3; innerChoiceIndex = index in its selection list.
    // Phase 2: disable assigned slots and lock each controller to its active selection.
    for (outerChoiceIndex = 0; outerChoiceIndex < kPlayerCount; outerChoiceIndex++) {
        for (innerChoiceIndex = 0;
             innerChoiceIndex < ((CharaRenderContextView*)GetSharedCharaRenderContext())->lists[outerChoiceIndex].count;
             innerChoiceIndex++) {
            selectedSlot = ((CharaRenderContextView*)GetSharedCharaRenderContext())
                               ->lists[outerChoiceIndex]
                               .items[innerChoiceIndex];
            if (playerStates[selectedSlot] != 0) {
                Layout_SetElementPlayerFlag(layout, selectedSlot + kFirstPlayerChoice, 0, kAllPlayers);
                Layout_SetElementPlayerFlag(layout, selectedSlot + kFirstPlayerOption, 0, kAllPlayers);
            }
        }
        if (((CharaRenderContextView*)GetSharedCharaRenderContext())->lists[outerChoiceIndex].count != 0) {
            CharaRenderContextView* renderContext = (CharaRenderContextView*)GetSharedCharaRenderContext();
            int lastSelectionIndex = renderContext->lists[outerChoiceIndex].count - 1;
            selectedSlot = renderContext->lists[outerChoiceIndex].items[lastSelectionIndex];
            if (playerStates[selectedSlot] == 1) {
                for (innerChoiceIndex = 0; innerChoiceIndex < kPlayerCount; innerChoiceIndex++) {
                    Layout_SetElementPlayerFlag(layout, innerChoiceIndex + kFirstPlayerChoice, 0, outerChoiceIndex);
                    Layout_SetElementPlayerFlag(layout, innerChoiceIndex + kFirstPlayerOption, 0, outerChoiceIndex);
                }
            }
        }
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kOneSelectionPerController) {
            if (((CharaRenderContextView*)GetSharedCharaRenderContext())->lists[outerChoiceIndex].count != 0) {
                for (innerChoiceIndex = 0; innerChoiceIndex < kPlayerCount; innerChoiceIndex++) {
                    Layout_SetElementPlayerFlag(layout, innerChoiceIndex + kFirstPlayerChoice, 0, outerChoiceIndex);
                    Layout_SetElementPlayerFlag(layout, innerChoiceIndex + kFirstPlayerOption, 0, outerChoiceIndex);
                }
            }
        }
    }

    Layout_SetElementPlayerFlag(layout, kLoadSaveDataIcon, 0, kAllPlayers);
    Layout_SetElementPlayerFlag(layout, kLoadSaveDataHighlight, 0, kAllPlayers);

    // outerChoiceIndex = character 0..7; innerChoiceIndex = controller 0..3.
    // Phase 3: enumerate character equipment and each controller's active player slot.
    characterEquipmentByteOffset = 0;
    for (outerChoiceIndex = 0; outerChoiceIndex < kSelectableCharacterCount; outerChoiceIndex++) {
        characterAvailable = 0;
        for (playerSlot = 0; playerSlot < ((CharaRenderContextView*)GetSharedCharaRenderContext())->count;
             playerSlot++) {
            if (((CRecBits*)((char*)GetSharedCharaRenderContext() + playerSlot * (int)sizeof(SavePlayerRecord) +
                             characterEquipmentByteOffset + kSavedCharacterEquipmentOffset))
                    ->characterSelectable != 0) {
                characterAvailable = 1;
            }
        }
        if (characterAvailable != 0) {
            Layout_SetElementEnabled(layout, outerChoiceIndex + kFirstCharacterIcon, 1);
            Layout_SetElementEnabled(layout, outerChoiceIndex + kFirstLockedCharacterIcon, 0);
        } else {
            Layout_SetElementEnabled(layout, outerChoiceIndex + kFirstCharacterIcon, 0);
            Layout_SetElementEnabled(layout, outerChoiceIndex + kFirstLockedCharacterIcon, 1);
        }
        for (innerChoiceIndex = 0; innerChoiceIndex < kPlayerCount; innerChoiceIndex++) {
            if (((CharaRenderContextView*)GetSharedCharaRenderContext())->lists[innerChoiceIndex].count != 0) {
                CharaRenderContextView* renderContext = (CharaRenderContextView*)GetSharedCharaRenderContext();
                int lastSelectionIndex = renderContext->lists[innerChoiceIndex].count - 1;
                selectedSlot = renderContext->lists[innerChoiceIndex].items[lastSelectionIndex];
            } else {
                selectedSlot = -1;
            }
            if (selectedSlot >= 0 && playerStates[selectedSlot] == 1 &&
                ((CRecBits*)((char*)GetSharedCharaRenderContext() + selectedSlot * (int)sizeof(SavePlayerRecord) +
                             characterEquipmentByteOffset + kSavedCharacterEquipmentOffset))
                        ->characterSelectable != 0) {
                Layout_SetElementPlayerFlag(layout, outerChoiceIndex + kFirstCharacterIcon, 1, innerChoiceIndex);
                Layout_SetElementPlayerFlag(layout, outerChoiceIndex + kFirstCharacterHighlight, 1, innerChoiceIndex);
                if ((((CharaRenderContextView*)GetSharedCharaRenderContext())->entries[selectedSlot].flags >> 15) & 1) {
                    if (AsCSharedDoc(g_sharedDoc)->gameMode != kRandomSelectionMode) {
                        Layout_SetElementPlayerFlag(layout, kLoadSaveDataIcon, 0, innerChoiceIndex);
                    }
                    Layout_SetElementPlayerFlag(layout, kLoadSaveDataHighlight, 0, innerChoiceIndex);
                } else {
                    if (AsCSharedDoc(g_sharedDoc)->gameMode != kRandomSelectionMode) {
                        Layout_SetElementPlayerFlag(layout, kLoadSaveDataIcon, 1, innerChoiceIndex);
                        Layout_SetElementPlayerFlag(layout, kLoadSaveDataHighlight, 1, innerChoiceIndex);
                    }
                }
            } else {
                Layout_SetElementPlayerFlag(layout, outerChoiceIndex + kFirstCharacterIcon, 0, innerChoiceIndex);
                Layout_SetElementPlayerFlag(layout, outerChoiceIndex + kFirstCharacterHighlight, 0, innerChoiceIndex);
            }
        }
        characterEquipmentByteOffset += (int)sizeof(SavedOutfitSelection);
    }
}
#pragma opt_lifetimes reset

// Wait for texture, Bp entries and layout loading to finish before activating selection.
int CCharaSelect::PollLoadComplete() {
    if (((CPackTextureResource*)(((CharaSelectTextureHandle*)this)->texture))->IsLoadFinished() == 0) {
        return 0;
    }
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_805041C4);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    if (((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded() != 0) {
        return Layout_PollLoadComplete(layout);
    }
    return 0;
}

} // extern "C"
