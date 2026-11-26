// Four-player character entry: build avatar previews/stat rows and open the
// selected player's Pangya shop. Shared layout: classes/CCharaEntry.h; resource
// strings: characterEntryStrings (external pool); preview records: the
// legacy CEnemyRenderRecord/Table headers, aliased below to their general role.
// Mod lookup: preview rectangles/positions -> EntryPreviewGeometry and
// characterEntryRenderPositions; visible rows -> EntryLayoutElement + player;
// stat animations -> UpdateStatBars (computer_player_random_and_stroke.cpp); shop admission/ownership ->
// OpenPlayerShop below. Four save records, vectors and previews share one index.
#include "classes/CEnemyRenderRecord.h"
// Header/ABI spellings are shared; these records hold human and COM previews.
typedef CEnemyRenderRecord PlayerPreviewRecord;
#define AsPlayerPreviewRecord(p) ((PlayerPreviewRecord*)(p))
#include "classes/CEnemyRenderTable.h"
typedef CEnemyRenderTable PlayerPreviewTable;
#define AsPlayerPreviewTable(p) ((PlayerPreviewTable*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/vector.h"
#include "classes/SavePlayerRecord.h"
extern "C" void __construct_array(void*, void*, void*, unsigned int, unsigned int);

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CCharaEntry.h"
#include "classes/CPangya2Shop.h"
#include "classes/CCharacterPreviewTable.h"
#include "classes/CAsyncExhibition.h"
#include "classes/CBpCtrlHandle.h"
#include "classes/CBpCtrl.h"
extern "C" char characterEntryRenderPositions[];

#define AsCCharaEntry(p) ((CCharaEntry*)(p))

// Address-only view; native construction still runs through the original helpers.
struct EntryRenderContextView {
    SavePlayerRecord saves[4];
    IntVector vectors[4];
    PlayerPreviewRecord records[4];
    int count;
};
// Four per-player transition records. Reset current/previous to zero and remainingSteps
// to five before the stats script begins; this function does not advance those steps.
// The int-word stores below address this exact triplet, not an unrelated flat array.
struct EntryStatBarState {
    int currentValue, previousValue, remainingSteps;
};
static const int kStatBarWords = sizeof(EntryStatBarState) / sizeof(int);
static const int kStatBarPreviousWord = (int)&((EntryStatBarState*)0)->previousValue / sizeof(int);
static const int kStatBarStepsWord = (int)&((EntryStatBarState*)0)->remainingSteps / sizeof(int);
struct CharacterEntryStringPool {
    char archive[0x10], speed[0xA], entryLayout[0xA], noEntryLayout[0x15];
    char statsLayout[0x16], selectSound[0xB], partyTask[0xA], fightTask[0xA];
    char okClick[0x13], decideAnimation[0x14], statsScript[0xE], colorScript[0xE];
    char threeEmpty[0xB], twoEmpty[0xB], oneEmpty[0xB], zeroEmpty[0xB];
    char clubhouseMusic[0xE];
};
static const int kEntryStatsScript = (int)&((CharacterEntryStringPool*)0)->statsScript;
static const int kEntryColorScript = (int)&((CharacterEntryStringPool*)0)->colorScript;
static const int kEntryThreeEmptyScript = (int)&((CharacterEntryStringPool*)0)->threeEmpty;
static const int kEntryTwoEmptyScript = (int)&((CharacterEntryStringPool*)0)->twoEmpty;
static const int kEntryOneEmptyScript = (int)&((CharacterEntryStringPool*)0)->oneEmpty;
static const int kEntryZeroEmptyScript = (int)&((CharacterEntryStringPool*)0)->zeroEmpty;
static const int kEntryDecideAnimation = (int)&((CharacterEntryStringPool*)0)->decideAnimation;
static const int kEntryClubhouseMusic = (int)&((CharacterEntryStringPool*)0)->clubhouseMusic;
static const int kEntryPartyTask = (int)&((CharacterEntryStringPool*)0)->partyTask;
static const int kEntryFightTask = (int)&((CharacterEntryStringPool*)0)->fightTask;
static const int kPartyGameMode = 7;
static const int kPlayerColumns = 4;
enum EntryLayoutElement {
    kOccupiedColumn = 0,
    kEntryRow = 4,
    kEntryRowState = 8, // Visibility/style and row-state reset.
    kHumanRow = 0x0c,
    kEntryPresentation00 = 0x10,
    kEntryPresentation01 = 0x14,
    kEntryPresentation10 = 0x18, // Cycled when the record is human.
    kEntryResetVisual30 = 0x30,
    kEntryPresentation01Aux = 0x34,
    kEntryPresentation10Aux = 0x38,
    kEntryPresentation00Aux = 0x3c,
    kEntryResetVisual40 = 0x40,
    kEntryResetVisual44 = 0x44,
    kEntryResetVisual48 = 0x48,
    kEntryResetVisual4C = 0x4c,
    kEntryResetVisual50 = 0x50,
    kEnemyPresentation = 0x54,
    kHumanPresentation = 0x58,
    kChoiceMarkerBase = 0x5c
};
enum EntryScriptColumnGroup {
    kEntryColumn1Group = 1004,
    kEntryColumn2Group = 1005,
    kEntryColumn3Group = 1006,
    kStatsColumn1Group = 1041,
    kStatsColumn2Group = 1042,
    kStatsColumn3Group = 1043
};
enum EntryPreviewGeometry {
    kPreviewColumnWidth = 150,
    kPreviewTop = 120,
    kPreviewBottom = 390,
    kPreviewPositionPairBytes = 8,
    kStatTransitionSteps = 5
};
// UI ID map: every base above has one element per player, ID = base+player.
// The layout assets are required to resolve artwork names; code alone establishes
// column occupancy and the bit-15 presentation, not which individual stat is drawn.
// Only choice markers use ID = kChoiceMarkerBase+4*player+choice.
// Asset navigation: CCharaEntry::CCharaEntry (chara_entry_ctor_dtor.cpp) opens "chara_entry.zip";
// characterEntryStrings holds "entry.spd" at +0x10, "entry.b2d" at +0x1A,
// "entry_c_no_entry.b2d" at +0x24 and "entry_chr_sts_prm.b2d" at +0x39.
// Resolve layout artwork against entry.spd and the BP group against the matching
// .b2d script. They are separate ID spaces; do not assign POWER/CONTROL/SPIN
// labels to layout IDs just from the stat-script group names below.
// Cross-check: CCharaEntry::RefreshPlayerRow (chara_entry_screen_part1.cpp) identifies 04 as the
// styled row and 08 as its reset state; 0C is human-only. CyclePlayerEntryState
// (chara_entry_render_context.cpp) selects presentation 00/01/10 at 10/14/18, with matching auxiliary
// visuals 3C/34/38. Their artwork and the reset visuals 30/40..50 remain unresolved.
// UpdateStatBars (computer_player_random_and_stroke.cpp) identifies script groups 0x400+5*p / 0x3FF+5*p /
// 0x3FE+5*p as POWER/CONTROL/SPIN; those are BP group IDs, not these layout IDs.
// Element addressing: add idx (player column) to each base.
// +0 is enabled for occupied columns; +0x54 is shown only for record flag bit 15;
// +0x0c/+0x10/+0x14/+0x18 and +0x58 are shown for the other presentation.
// Choice markers: 0x5c + 4*idx + choiceIndex, choiceIndex in [0,4).
// +0x30..+0x50 step 4 are reset with stat bars; their individual artwork is unresolved.
// BP groups 1004/1005/1006 and 1041/1042/1043 suppress columns 1/2/3
// in entry/stats scripts. Empty-column scripts are selected by occupied count.

extern "C" {
extern char s_bpCtrlDtorRegistration_80506F04[];
extern char s_bpCtrlHandleDtorRegistration_CharacterEntry[];
extern char s_inputManagerDtorRegistration_CharacterEntry[];
extern char s_renderContextDtorRegistration_80506F1C[];

void RenderContext_ResetPlayerRecords(void*);
void* CharacterPreviewTable_dtor(void*, int);
void appFreeUni__FPv(void*);

struct RenderContextConstructionCleanup_u8024D468 {
    char* base;
    bool active;
    // Arm cleanup for a render context whose arrays are already constructed.
    explicit RenderContextConstructionCleanup_u8024D468(void* object) : base((char*)object), active(true) {}
    // On failure release the player table, then the four per-player vector allocations.
    ~RenderContextConstructionCleanup_u8024D468() {
        if (active) {
            CharacterPreviewTable_dtor(base + 0x20D0, -1);
            for (int index = 3; index >= 0; --index) {
                unsigned int* vector = (unsigned int*)(base + 0x20A0 + index * 12);
                if (vector[0]) {
                    vector[1] = 0;
                    appFreeUni__FPv((void*)vector[0]);
                }
            }
        }
    }
};

// Reset newly constructed records; the local owner unwinds arrays if reset throws.
static void InitializeNewRenderContext(void* base) {
    RenderContextConstructionCleanup_u8024D468 cleanup(base);
    RenderContext_ResetPlayerRecords(base);
    cleanup.active = false;
}

extern signed char s_renderContextInitialized, s_inputManagerInitialized, s_bpCtrlInitialized, s_fadeOverlayInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern void* g_soundManager;
extern void* g_sharedDoc;
extern void* g_taskManager;
extern void SaveData_InitPlayerRecord(void*);
extern void CBpCtrlHandle_Dtor(void*);
extern void* RenderContext_GetOrCreate();
extern void Layout_SetElementEnabled(void*, int, int);
extern void Layout_SetElementPlayerFlagsAndState(void*, int, int);
extern void BpObject_StartScriptByName(void*, const char*, int);
extern void* UIGroup_GetOrCreateById(void*, int);
extern void UIGroup_SetEnabled(void*, int);
extern void IFadeMenuBase_StartMenuFadeOut(void*);

extern void PartyItemRecord_InitDefaults(void*);
extern void PlayerRenderRecord_dtor(void*);
extern void RenderContext_ResetPlayerRecords(void*);
extern void __dt__13RenderContextFv(void*);
extern void MPBase1_ResetExecutionState(void*);
extern void SoundManager_RequestBgmChange(void*, const char*, int, int);
extern void InputManager_Init(void*);
extern void InputManager_Dtor(void*);
extern void InputManager_SetSelectedPlayer(void*, int);
extern void Layout_SetActive(void*, int);
extern void CBpCtrl_dtor(void*);
extern void* IdMap_Lookup(void*, void*);
extern void BpObject_SetEnabled(void*, int);
extern void TaskCommandQueue_AppendNamedRequest(void*, int, const char*, int, int, int, int, int);
extern void FadeOverlay_Init(void*);
extern void ResourceState_Dtor(void*);
extern void FadeState_StartFadeIn(void*, int);
extern char characterEntryStrings[];
char s_inputManagerDtorRegistration_CharacterEntry[12];
char s_pangyaEnemyDataDtorRegistration_CharacterEntry[12];
char s_fadeOverlayDtorRegistration_CharacterEntry[12];
char s_bpCtrlDtorRegistration_80506F04[12];
char s_bpCtrlHandleDtorRegistration_CharacterEntry[12];
char s_renderContextDtorRegistration_80506F1C[12];
extern char s_gameInputManager[];
extern char s_bpCtrl[];
extern char s_fadeOverlay[];
// The ABI name is retained, but its old unused offset-suffixed fields were inconsistent.
// All accesses below use the checked CCharaEntry layout from classes/CCharaEntry.h.
struct InitObj;

extern char s_renderContext[];
extern int s_bpCtrlHandle[2];
extern float kFloat_15_EnemyRender;

// Forced-inline construction retains the caller's exception/unwind scope and
// registrations. The loop still performs the lazy check on its final count test.
// Context: four 0x828 save records, four IntVectors at +0x20a0, player table at +0x20d0.
#pragma push
#pragma always_inline on
// Construct/register the render context once and publish its borrowed singleton pointer.
static inline void EnsureEntryRenderContext(void* registration) {
    if (!Sing_805DC648()) {
        if (!s_renderContextInitialized) {
            char* base = s_renderContext;
            char* member = base + 0x20A0;
            char* p = base;
            do {
                SaveData_InitPlayerRecord(p);
                p += 0x828;
            } while (p < member);
            ((IntVectorArray*)(member))->__ct();
            member = base + 0x20D0;
            __construct_array(member, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4);
            AsPlayerPreviewTable(member)->count = 0;
            InitializeNewRenderContext(base);
            __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv, registration);
            s_renderContextInitialized = 1;
        }
        Sing_805DC648() = s_renderContext;
    }
}
#pragma pop
// Ensure the borrowed BP-control singleton exists using local registrations.
static inline void EnsureEntryBpController(void* registration) {
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, registration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}
// Enable one element in a player column; layout IDs advance by one per player.
static inline void SetEntryElementEnabled(void* self, int player, int base, int enabled) {
    Layout_SetElementEnabled(AsCCharaEntry(self)->layout, player + base, enabled);
}
// Clear the player/state flags of one element in a player column.
static inline void HideEntryElement(void* self, int player, int base) {
    Layout_SetElementPlayerFlagsAndState(AsCCharaEntry(self)->layout, player + base, 0);
}
// Borrow the player table embedded in a render context; never cache across singleton access.
static inline PlayerPreviewTable* EntryPlayerTable(void* context) {
    return (PlayerPreviewTable*)((char*)context + 0x20D0);
}
// Record +0x18 also supplies row style in CCharaEntry::RefreshPlayerRow.
// Here it selects one of four markers; its controller meaning is unproven.
struct EntryRowSelectionView {
    char prefix[0x18];
    int selection;
};

// Borrow a record using the running byte offset maintained by the preview loop.
static inline PlayerPreviewRecord* EntryPlayerRecord(void* context, int recordByteOffset) {
    return (PlayerPreviewRecord*)((char*)context + recordByteOffset + 0x20D0);
}
#pragma push
#pragma always_inline on
// Disable a script group after ensuring the BP controller; scriptId is borrowed, never retained.
static inline void DisableEntryScriptGroup(void* self, void* bpRegistration, void*& scriptId, int id) {
    EnsureEntryBpController(bpRegistration);
    UIGroup_SetEnabled(UIGroup_GetOrCreateById(IdMap_Lookup(Sing_805DC5F0(), scriptId), id), 0);
}
// Start a named entry script after BP construction; pool offset selects an existing resource string.
static inline void StartEntryScript(void* self, void* bpRegistration, void*& scriptId, int offset) {
    EnsureEntryBpController(bpRegistration);
    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), scriptId), characterEntryStrings + offset, 1);
}
#pragma pop

// Build avatar previews, refresh player rows and reset stat-bar transitions for entry.

// Build player previews, row presentations, and initial stat transitions.
#pragma push
#pragma always_inline on
// Use this entry screen's destructor-registration storage when ensuring the shared context.
static inline void EnsureCharacterEntryContext(char* registrations) {
    EnsureEntryRenderContext((registrations + 0x3c));
}
#pragma pop

// Build avatar previews and player rows, then reset stats and start entry scripts.
void CCharaEntry::SetupEntryScreen() {
    char* registrations = s_inputManagerDtorRegistration_CharacterEntry;
    if (!Sing_805DC5F8()) {
        if (!static_cast<signed char>(s_bpCtrlHandleInitialized)) {
            s_bpCtrlHandle[1] = -1;
            s_bpCtrlHandle[0] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (registrations + 0x30));
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(14);
    // Phase 1: populate occupied player previews and select their control presentation.
    // playerIndex = player index, recordByteOffset = playerIndex*sizeof(PlayerPreviewRecord) = playerIndex*0xa4,
    // previewLeft = preview column X, previewXY = borrowed 8-byte XY pair, choiceMarkerBase = marker base.
    // Ensure the singleton exists before reading the record count.
    int rect[4];
    char* volatile previewXY = (char*)((char*)&characterEntryRenderPositions);
    char* entryStrings = characterEntryStrings;
    float avatarPreviewScale = kFloat_15_EnemyRender;
    int playerIndex = 0;
    int recordByteOffset = 0;
    volatile int previewLeft = 0;
    int choiceMarkerBase = kChoiceMarkerBase;
    // Preview rectangles are pixels in the entry layout; avatarPreviewScale is
    // forwarded to avatar setup, not used as elapsed time.
    // The comma condition ensures the singleton before each count test, including
    // the final one. Registration stays in the caller's inlined unwind scope.
    while ((EnsureCharacterEntryContext(registrations), playerIndex < EntryPlayerTable(Sing_805DC648())->count)) {
        rect[0] = previewLeft;
        rect[1] = kPreviewTop;
        rect[2] = previewLeft + kPreviewColumnWidth;
        rect[3] = kPreviewBottom;
        CCharacterPreviewTable::SetupRecordAvatar(EntryPlayerTable(RenderContext_GetOrCreate()), playerIndex, rect, 0,
                                                  avatarPreviewScale);
        if (EntryPlayerRecord(RenderContext_GetOrCreate(), recordByteOffset)->renderObject)
            ((CAsyncExhibition*)EntryPlayerRecord(RenderContext_GetOrCreate(), recordByteOffset)->renderObject)
                ->SetLoadingIndicatorPosition((const float*)previewXY);
        EnsureCharacterEntryContext(registrations);
        if ((EntryPlayerRecord(Sing_805DC648(), recordByteOffset)->flags >> 15) & 1) {
            SetEntryElementEnabled(this, playerIndex, kOccupiedColumn, 1);
            SetEntryElementEnabled(this, playerIndex, kEnemyPresentation, 1);
            HideEntryElement(this, playerIndex, kHumanRow);
            HideEntryElement(this, playerIndex, kEntryPresentation00);
            HideEntryElement(this, playerIndex, kEntryPresentation01);
            HideEntryElement(this, playerIndex, kEntryPresentation10);
            SetEntryElementEnabled(this, playerIndex, kHumanPresentation, 0);
            for (int k = 0; k < 4; k++)
                Layout_SetElementEnabled(*((&layout)), k + choiceMarkerBase, 0);
            EnsureCharacterEntryContext(registrations);
            this->SetEnemyLevelSelection(
                playerIndex, *(signed char*)&EntryPlayerRecord(Sing_805DC648(), recordByteOffset)->levelIndex);
            // +0x1c: signed view
        } else {
            SetEntryElementEnabled(this, playerIndex, kOccupiedColumn, 1);
            SetEntryElementEnabled(this, playerIndex, kEnemyPresentation, 0);
            SetEntryElementEnabled(this, playerIndex, kHumanRow, 1);
            SetEntryElementEnabled(this, playerIndex, kEntryPresentation00, 1);
            SetEntryElementEnabled(this, playerIndex, kEntryPresentation01, 1);
            SetEntryElementEnabled(this, playerIndex, kEntryPresentation10, 1);
            SetEntryElementEnabled(this, playerIndex, kHumanPresentation, 1);
            for (int k = 0; k < 4; k++) {
                EnsureCharacterEntryContext(registrations);
                if (k ==
                    ((EntryRowSelectionView*)(recordByteOffset + (unsigned int)Sing_805DC648() + 0x20D0))->selection)
                    Layout_SetElementEnabled(layout, k + choiceMarkerBase, 1);
                else
                    Layout_SetElementEnabled(layout, k + choiceMarkerBase, 0);
            }
            CyclePlayerEntryState(playerIndex);
            CyclePlayerEntryState(playerIndex);
            CyclePlayerEntryState(playerIndex);
            SetEnemyLevelSelection(playerIndex, -1);
        }
        RefreshPlayerRow(playerIndex);
        UpdateStatBars();
        EnsureCharacterEntryContext(registrations);
        ((CCharacterPreviewTable*)EntryPlayerTable(Sing_805DC648()))
            ->SetAnimationName(playerIndex, entryStrings + kEntryDecideAnimation);
        recordByteOffset += 0xa4;
        choiceMarkerBase += 4;
        ++playerIndex;
        previewLeft += kPreviewColumnWidth;
        previewXY += kPreviewPositionPairBytes;
    }

    // Phase 2: hide unoccupied player columns and clear their level selection.
    for (; playerIndex < 4; ++playerIndex) {
        HideEntryElement(this, playerIndex, kOccupiedColumn);
        HideEntryElement(this, playerIndex, kEnemyPresentation);
        HideEntryElement(this, playerIndex, kHumanRow);
        HideEntryElement(this, playerIndex, kEntryPresentation00);
        HideEntryElement(this, playerIndex, kEntryPresentation01);
        HideEntryElement(this, playerIndex, kEntryPresentation10);
        HideEntryElement(this, playerIndex, kEntryRowState);
        HideEntryElement(this, playerIndex, kEntryRow);
        HideEntryElement(this, playerIndex, kHumanPresentation);
        for (int k = 0; k < 4; ++k)
            Layout_SetElementPlayerFlagsAndState(layout, k + playerIndex * kPlayerColumns + kChoiceMarkerBase, 0);
        SetEnemyLevelSelection(playerIndex, -1);
    }
    // Phase 3: hide stat elements and reset (previous,current,remainingSteps) to (0,0,5).
    // These are transition steps, not elapsed seconds or a five-player count.
    for (playerIndex = 0; playerIndex < 4; ++playerIndex) {
        HideEntryElement(this, playerIndex, kEntryResetVisual30);
        HideEntryElement(this, playerIndex, kEntryPresentation01Aux);
        HideEntryElement(this, playerIndex, kEntryPresentation10Aux);
        HideEntryElement(this, playerIndex, kEntryPresentation00Aux);
        HideEntryElement(this, playerIndex, kEntryResetVisual40);
        HideEntryElement(this, playerIndex, kEntryResetVisual44);
        HideEntryElement(this, playerIndex, kEntryResetVisual48);
        HideEntryElement(this, playerIndex, kEntryResetVisual4C);
        HideEntryElement(this, playerIndex, kEntryResetVisual50);
        ((int*)statBarTransitions)[playerIndex * kStatBarWords + kStatBarPreviousWord] = 0;
        ((int*)statBarTransitions)[playerIndex * kStatBarWords] = 0;
        ((int*)statBarTransitions)[playerIndex * kStatBarWords + kStatBarStepsWord] = kStatTransitionSteps;
    }
    // Phase 4: start entry/stats scripts, mask missing columns, then enter the fade-menu base.
    StartEntryScript(this, (registrations + 0x24), characterStatsScriptId, kEntryStatsScript);
    StartEntryScript(this, (registrations + 0x24), entryScriptId, kEntryColorScript);
    EnsureCharacterEntryContext(registrations);
    switch (EntryPlayerTable(Sing_805DC648())->count) {
    case 1:
        DisableEntryScriptGroup(this, (registrations + 0x24), entryScriptId, kEntryColumn1Group);
        DisableEntryScriptGroup(this, (registrations + 0x24), entryScriptId, kEntryColumn2Group);
        DisableEntryScriptGroup(this, (registrations + 0x24), entryScriptId, kEntryColumn3Group);
        DisableEntryScriptGroup(this, (registrations + 0x24), characterStatsScriptId, kStatsColumn1Group);
        DisableEntryScriptGroup(this, (registrations + 0x24), characterStatsScriptId, kStatsColumn2Group);
        DisableEntryScriptGroup(this, (registrations + 0x24), characterStatsScriptId, kStatsColumn3Group);
        StartEntryScript(this, (registrations + 0x24), noEntryScriptId, kEntryThreeEmptyScript);
        break;
    case 2:
        DisableEntryScriptGroup(this, (registrations + 0x24), entryScriptId, kEntryColumn2Group);
        DisableEntryScriptGroup(this, (registrations + 0x24), entryScriptId, kEntryColumn3Group);
        DisableEntryScriptGroup(this, (registrations + 0x24), characterStatsScriptId, kStatsColumn2Group);
        DisableEntryScriptGroup(this, (registrations + 0x24), characterStatsScriptId, kStatsColumn3Group);
        StartEntryScript(this, (registrations + 0x24), noEntryScriptId, kEntryTwoEmptyScript);
        break;
    case 3:
        DisableEntryScriptGroup(this, (registrations + 0x24), entryScriptId, kEntryColumn3Group);
        DisableEntryScriptGroup(this, (registrations + 0x24), characterStatsScriptId, kStatsColumn3Group);
        StartEntryScript(this, (registrations + 0x24), noEntryScriptId, kEntryOneEmptyScript);
        break;
    case 4:
        StartEntryScript(this, (registrations + 0x24), noEntryScriptId, kEntryZeroEmptyScript);
        break;
    }
    IFadeMenuBase_StartMenuFadeOut(this);
}

// Construct the four-player render context and register its global destructor.
inline void InitStateMembers(char* reg) {
    char* base = s_renderContext;
    char* member = base + 0x20A0;
    char* p = base;
    do {
        SaveData_InitPlayerRecord(p);
        p += 0x828;
    } while (p < member);
    ((IntVectorArray*)(member))->__ct();
    member = base + 0x20D0;
    __construct_array(member, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4);
    AsPlayerPreviewTable(member)->count = 0;
    InitializeNewRenderContext(base);
    __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv, reg + 0x3c);
}

// Return the borrowed singleton, constructing it on first use.
inline char* GetStateForNew(char* reg) {
    if (!Sing_805DC648()) {
        if (!s_renderContextInitialized) {
            InitStateMembers(reg);
            s_renderContextInitialized = 1;
        }
        Sing_805DC648() = s_renderContext;
    }
    return (char*)Sing_805DC648();
}

struct ShopObject {
    char data[0x8E0];
    // Construct an owned shop using the borrowed player snapshot and initialization flag.
    ShopObject(void* record, int flag) {
        ((CPangya2Shop*)this)->Init(record, flag);
    }
};

#pragma use_lmw_stmw on
// Open or refresh the selected player's shop, hide entry controls, and start the menu fade.
// playerIndex selects one of four existing save/render records. inputPlayerIndexWord
// carries the controller channel as an integer in a pointer-shaped ABI word;
// the menu owns shop and keeps it between visits. Repeated opens while shopOpen
// is set return immediately; this function does not release or replace an existing shop.
// Admission is the caller's responsibility: require 0<=playerIndex<4, an occupied
// initialized preview/save slot, valid layout/script IDs, and live task/sound
// managers. There is NO index validation here; adding one is a behavior change.
// inputPlayerIndexWord should encode channel 0..3. InputManager_SetSelectedPlayer
// (game_input_manager_cursor_rumble.cpp) rejects other values and keeps the previous channel; this void
// declaration ignores its status. This argument is not dereferenced or retained
// as a pointer, and it need not equal the selected save-record playerIndex.
// Partial-failure map (there is no rollback/status return):
//  1. Before constructing/refreshing shop, copy avatar id into the save snapshot.
//     Construction failure can leave that snapshot changed, with shopOpen still 0.
//  2. Publish/reuse menu-owned shop, then set shopOpen=1 BEFORE BGM/focus work.
//     A null-returning allocation is not handled: shopOpen still becomes 1;
//     later shop consumers require a nonnull object. Throws stop at their site.
//  3. Save selectedPlayerIndex, hand off input, hide layout and three BP scripts.
//  4. Enqueue Party/Fight request, then initialize/start fade.
// Exceptions in phases 2..4 leave earlier publications visible; reopening while
// shopOpen is set returns immediately. The entry teardown owns the cached shop;
// this routine does not transfer it to the queued task or destroy it on failure.
// CCharaEntry_Process (party_character_entry_menu.cpp) commits the shop snapshot and restores input,
// scripts and entry fade on completion; CCharaEntry_dtor (chara_entry_ctor_dtor.cpp) owns release.
void CCharaEntry::OpenPlayerShop(int playerIndex, int inputPlayerIndexWord) {
    struct StateAccess {
        // Construct singleton members and register their global teardown.
        static void Init() {
            char* base = s_renderContext;
            char* member = base + 0x20A0;
            char* newShop = base;
            do {
                SaveData_InitPlayerRecord(newShop);
                newShop += 0x828;
            } while (newShop < member);
            ((IntVectorArray*)member)->__ct();
            member = base + 0x20D0;
            __construct_array(member, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4);
            AsPlayerPreviewTable(member)->count = 0;
            InitializeNewRenderContext(base);
            __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv,
                                     s_renderContextDtorRegistration_80506F1C);
        }
        // Return the borrowed singleton, initializing it once when first accessed.
        static char* Get() {
            if (!Sing_805DC648()) {
                if (!s_renderContextInitialized) {
                    Init();
                    s_renderContextInitialized = 1;
                }
                Sing_805DC648() = s_renderContext;
            }
            return (char*)Sing_805DC648();
        }
    };
    // Phase 1: ensure context exists and copy the selected avatar id into its save snapshot.
    SavePlayerRecord* playerSaveRecords;
    if (shopOpen)
        return;
    if (!Sing_805DC648()) {
        if (!s_renderContextInitialized) {
            StateAccess::Init();
            s_renderContextInitialized = 1;
        }
        Sing_805DC648() = s_renderContext;
    }
    playerSaveRecords = (SavePlayerRecord*)Sing_805DC648();
    if (!playerSaveRecords) {
        if (!s_renderContextInitialized) {
            StateAccess::Init();
            s_renderContextInitialized = 1;
        }
        Sing_805DC648() = s_renderContext;
    }
    EntryRenderContextView* renderContext = (EntryRenderContextView*)Sing_805DC648();
    playerSaveRecords[playerIndex].characterIndex = renderContext->records[playerIndex].characterInventoryIndex;
    // Phase 2: menu owns the shop object; create it once or refresh its snapshot/selection.
    if (!shop) {
        void* newShop = new ShopObject(&((SavePlayerRecord*)StateAccess::Get())[playerIndex], 1);
        shop = newShop;
    } else {
        if (!renderContext) {
            if (!s_renderContextInitialized) {
                StateAccess::Init();
                s_renderContextInitialized = 1;
            }
            Sing_805DC648() = s_renderContext;
        }
        CPangya2Shop::CopyRecordIntoSnapshot(shop, &((SavePlayerRecord*)Sing_805DC648())[playerIndex]);
        ((CPangya2Shop*)shop)->InitializeProductSelection();
        MPBase1_ResetExecutionState(shop);
    }
    // Phase 3a: publish shopOpen before handing off input focus; selectedPlayerIndex
    // remains the selected save-record player index.
    shopOpen = 1;
    SoundManager_RequestBgmChange(g_soundManager, characterEntryStrings + kEntryClubhouseMusic, 0, 1);
    void* inputManager = GetInputManagerSlot();
    selectedPlayerIndex = playerIndex;

    if (!inputManager) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     s_inputManagerDtorRegistration_CharacterEntry);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    InputManager_SetSelectedPlayer(GetInputManagerSlot(), inputPlayerIndexWord);
    // Phase 3b: hide the layout and each of its three script objects in order.
    Layout_SetActive(layout, 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80506F04);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), entryScriptId), 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80506F04);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), noEntryScriptId), 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80506F04);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), characterStatsScriptId), 0);
    // Phase 3c: enqueue the mode's task, then start the overlay fade after registration.
    if (*((&AsCSharedDoc(g_sharedDoc)->gameMode)) == kPartyGameMode)
        TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, characterEntryStrings + kEntryPartyTask, 2, 0, 0, 0, 0);
    else
        TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, characterEntryStrings + kEntryFightTask, 2, 0, 0, 0, 0);
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                     s_fadeOverlayDtorRegistration_CharacterEntry);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    FadeState_StartFadeIn(Sing_805DC608(), 0);
}
#pragma use_lmw_stmw reset

} // extern "C"
