#include "classes/CEnemyRenderRecord.h"
#include "classes/EnemyPanel8024.h"
#include "classes/CEnemyRenderTable.h"
#define AsCEnemyRenderTable(p) ((CEnemyRenderTable*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/vector.h"
// Translation unit 0x8024BD08-0x8024D468 (5 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CCharaEntry.h"
#include <decomp/cw_bitfields.h>

#define AsCCharaEntry(p) ((CCharaEntry*)(p))
enum {
    kPlayerCount = 4, kEnemyLevelCount = 5, kLevelElementStride = 4, kFirstLevelElement = 28,
    kLayoutHiddenStyle = 5, kEnemyModeWithoutCaddie = 7,
    kAvatarWidth = 150, kAvatarTop = 120, kAvatarBottom = 390,
    kEnemyNameChanged = 0x4000, kFadeComplete = 2,
    kPlayerSaveBytes = 0x828, kPlayerSavesBytes = kPlayerCount * kPlayerSaveBytes,
    kEnemyTableOffset = (int)&((RenderContext*)0)->players
};

// UI IDs are column-major: add player index 0..3 to each base below.
//   4: main row visibility/style; 8: row flags/state reset;
//  12: visible only when record flag 0x8000 is clear;
//  16/20/24: always styled from record +0x18 (artwork meaning unresolved);
//  28/32/36/40/44: one indicator for enemy levels 0/1/2/3/4 respectively.
// Style 5 is used on the hidden calls; enabled is passed separately, not encoded
// in that style. All record/table/singleton storage here is borrowed.
enum CharacterEntryElementBase {
    kRowVisibilityElement = 4, kRowStateResetElement = 8,
    kNonEnemyRowElement = 12, kStyledRowElement16 = 16,
    kStyledRowElement20 = 20, kStyledRowElement24 = 24
};

extern "C" {

void RenderContext_ResetPlayerRecords(void*);
void* CharacterPreviewTable_dtor(void*, int);
void appFreeUni__FPv(void*);

struct RenderContextConstructionCleanup_u8024BD08 {
    char* base;
    bool active;
    // Arm rollback for constructed records/vectors until record reset succeeds.
    explicit RenderContextConstructionCleanup_u8024BD08(void* object)
        : base((char*)object), active(true) {}
    // Release records and vector allocations only if construction unwinds.
    ~RenderContextConstructionCleanup_u8024BD08() {
        if (active) {
            CharacterPreviewTable_dtor(base + kEnemyTableOffset, -1);
            for (int index = kPlayerCount - 1; index >= 0; --index) {
                IntVector* vector = ((RenderContext*)base)->vectors + index;
                if (vector->items) {
                    vector->size = 0;
                    appFreeUni__FPv(vector->items);
                }
            }
        }
    }
};

// Reset the new context's records, disarming rollback only after successful reset.
static void InitializeNewRenderContext(void* base) {
    RenderContextConstructionCleanup_u8024BD08 cleanup(base);
    RenderContext_ResetPlayerRecords(base);
    cleanup.active = false;
}


struct CharacterEntryConfirmation {
    char baseState[0x18];
    int rowTransitions[kPlayerCount];
    int selectionConfirmed;
};
// CodeWarrior's big-endian bitfield order makes flag the high bit (0x8000):
// the enemy/randomized-record branch, not the low-bit enabled flag of a UI object.
struct UiFlagShort { CW_BITS2( unsigned short flag:1, unsigned short rest:15) };
// Keep the byte stride explicit: callers also pass base + row stride to enemy selection.
#pragma push
#pragma always_inline on
// Borrow a record at a BYTE offset (playerIndex * sizeof(record)), not an index.
static inline CEnemyRenderRecord* EnemyRow(void* context, int recordByteOffset) {
    return (CEnemyRenderRecord*)(((RenderContext*)context)->players + recordByteOffset);
}
// Read record +0x18, forwarded both as layout player/style argument and selection context.
static inline int EnemyRowLayoutValue(void* context, int recordByteOffset) {
    return *(int*)EnemyRow(context, recordByteOffset)->pad_18;
}
// Borrow the record flag word through its high-bit view; no flags are changed here.
static inline UiFlagShort* EnemyRowFlags(void* context, int recordByteOffset) {
    return (UiFlagShort*)&EnemyRow(context, recordByteOffset)->flags;
}
#pragma pop
extern float kFloat_1_CharaEntry;

extern signed char s_fadeOverlayInitialized;
extern signed char s_bpCtrlInitialized;
extern char s_fadeOverlay[];
extern char s_bpCtrl[];
extern char s_fadeOverlayDtorRegistration_CharacterEntry[];
extern char s_bpCtrlDtorRegistration_80506F04[];
void FadeOverlay_Init(void*);
void ResourceState_Dtor(void*);
void __register_global_object(void*, void (*)(void*), void*);
void FadeState_StartFadeIn(void*, int);
int FadeState_GetState(void*);
void FadeState_RequestFramebufferCapture(void*);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void*);
void* IdMap_Lookup(void*, void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void* RenderContext_GetOrCreate();
void CCharacterPreviewTable_DrawAvatars(void*);
void Layout_DrawWithOpacity(void*, float);
void MPBase1_DrawIfInitialized(void*);
extern CSharedDoc* g_sharedDoc;
extern signed char s_renderContextInitialized, s_pangyaEnemyDataInitialized;
extern char s_renderContext[], s_pangyaEnemyData[], s_renderContextDtorRegistration_80506F1C[], s_pangyaEnemyDataDtorRegistration_CharacterEntry[], characterEntryRenderPositions[];
extern float kFloat_15_EnemyRender;
void SaveData_InitPlayerRecord(void*);


void PartyItemRecord_InitDefaults(void*);
void __dt__13RenderContextFv(void*);
void PlayerRenderRecord_dtor(void*);
void __construct_array(void*, void*, void*, int, int);
void RenderContext_ResetPlayerRecords(void*);
void CPangyaEnemyData_Init(void*);
void CPangyaEnemyData_dtor(void*);
void EnemyData_SelectRandomMatchingRecord(void*, void*, int, int, int, void*);
void CCharacterPreviewTable_SetupRecordAvatar(void*, int, int*, int, float);
void CAsyncExhibition_SetLoadingIndicatorPosition(void*, void*);
void Layout_SetElementEnabled(void*, int, int);
void Layout_SetElementPlayerFlag(void*, int, int, int);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
void* RenderContext_GetOrCreate();
extern signed char s_renderContextInitialized;
extern char s_renderContext[];

// Construct and register the shared render context, including player records, vectors, and avatars.
static void init_global_ui() {
    if (!((int&)Sing_805DC648())) {
        if (!s_renderContextInitialized) {
            // Phase 1: initialize four 0x828-byte save records, then their vectors.
            char* base = s_renderContext;
            char* p = base;
            char* end = p + kPlayerSavesBytes;
            do {
                SaveData_InitPlayerRecord(p);
                p += kPlayerSaveBytes;
            } while (p < end);
            ((IntVectorArray*)(end))->__ct();
            // Phase 2: construct four avatar records; ResetPlayerRecords is guarded
            // by rollback cleanup before publishing/registering the singleton.
            char* avatarRecords = base + kEnemyTableOffset;
            __construct_array(avatarRecords, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, sizeof(CEnemyRenderRecord), kPlayerCount);
            AsCEnemyRenderTable(avatarRecords)->count = 0;
            InitializeNewRenderContext(base);
            __register_global_object(s_renderContext, __dt__13RenderContextFv, s_renderContextDtorRegistration_80506F1C);
            s_renderContextInitialized = 1;
        }
        ((int&)Sing_805DC648()) = (int)s_renderContext;
    }
}

// Ensure the shared render context exists, registering its destruction once.
// These expansions retain three declaration orders; inline-function versions
// (even always_inline) change generated instructions. Each expansion initializes
// saves -> vectors -> avatar records -> guarded record reset, then publishes.
#define INIT_GLOBAL_UI() do { \
    if (!((int&)Sing_805DC648())) { \
        if (!s_renderContextInitialized) { \
            char* p; \
            char* base = s_renderContext; \
            p = base; \
            char* end = base + kPlayerSavesBytes; \
            do { \
                SaveData_InitPlayerRecord(p); \
                p += kPlayerSaveBytes; \
            } while (p < end); \
            ((IntVectorArray*)(end))->__ct(); \
            char* avatarRecords = base + kEnemyTableOffset; \
            __construct_array(avatarRecords, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, sizeof(CEnemyRenderRecord), kPlayerCount); \
            AsCEnemyRenderTable(avatarRecords)->count = 0; \
            InitializeNewRenderContext(base); \
            __register_global_object(s_renderContext, __dt__13RenderContextFv, s_renderContextDtorRegistration_80506F1C); \
            s_renderContextInitialized = 1; \
        } \
        ((int&)Sing_805DC648()) = (int)s_renderContext; \
    } \
} while (0)

// Same initialization, with cursor/end declarations preceding the base pointer.
#define INIT_GLOBAL_UI_FIRST() do { \
    if (!((int&)Sing_805DC648())) { \
        if (!s_renderContextInitialized) { \
            char* p; \
            char* end; \
            char* base = s_renderContext; \
            p = base; \
            end = base + kPlayerSavesBytes; \
            do { \
                SaveData_InitPlayerRecord(p); \
                p += kPlayerSaveBytes; \
            } while (p < end); \
            ((IntVectorArray*)(end))->__ct(); \
            char* avatarRecords = base + kEnemyTableOffset; \
            __construct_array(avatarRecords, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, sizeof(CEnemyRenderRecord), kPlayerCount); \
            AsCEnemyRenderTable(avatarRecords)->count = 0; \
            InitializeNewRenderContext(base); \
            __register_global_object(s_renderContext, __dt__13RenderContextFv, s_renderContextDtorRegistration_80506F1C); \
            s_renderContextInitialized = 1; \
        } \
        ((int&)Sing_805DC648()) = (int)s_renderContext; \
    } \
} while (0)

// Same initialization, reusing end for the avatar-table origin after vectors.
#define INIT_GLOBAL_UI2() do { \
    if (!((int&)Sing_805DC648())) { \
        if (!s_renderContextInitialized) { \
            char* end; \
            char* p; \
            char* base = s_renderContext; \
            p = base; \
            end = base + kPlayerSavesBytes; \
            do { \
                SaveData_InitPlayerRecord(p); \
                p += kPlayerSaveBytes; \
            } while (p < end); \
            ((IntVectorArray*)(end))->__ct(); \
            end = base + kEnemyTableOffset; \
            __construct_array(end, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, sizeof(CEnemyRenderRecord), kPlayerCount); \
            AsCEnemyRenderTable(end)->count = 0; \
            RenderContext_ResetPlayerRecords(base); \
            __register_global_object(s_renderContext, __dt__13RenderContextFv, s_renderContextDtorRegistration_80506F1C); \
            s_renderContextInitialized = 1; \
        } \
        ((int&)Sing_805DC648()) = (int)s_renderContext; \
    } \
} while (0)

// Layout object of the character-entry panel (borrowed; self is an EnemyPanel8024 here).
static inline void* PanelLayout(void* self) { return ((EnemyPanel8024*)self)->layout; }
// UI element id of the indicator for enemy level 0..4 (add the player index 0..3).
static inline int LevelElement(int level) { return kFirstLevelElement + level * kLevelElementStride; }

#pragma use_lmw_stmw on
// Refresh the player row and show only the selected enemy level indicators.
// index must be 0..3; record/layout must already be valid. No bounds checks here.
void CCharaEntry_RefreshPlayerRow(void* self, int index) {
    // Phase 1: clear row transition/state and hide the conditional row elements.
    ((CharacterEntryConfirmation*)self)->rowTransitions[index] = 0;
    Layout_SetElementEnabled(PanelLayout(self), index + kRowVisibilityElement, 1);
    Layout_SetElementPlayerFlagsAndState(PanelLayout(self), index + kRowStateResetElement, 0);
    Layout_SetElementPlayerFlag(PanelLayout(self), index + kNonEnemyRowElement, 0, kLayoutHiddenStyle);
    Layout_SetElementPlayerFlag(PanelLayout(self), index + kStyledRowElement16, 0, kLayoutHiddenStyle);
    INIT_GLOBAL_UI_FIRST();
    int recordByteOffset = index * sizeof(CEnemyRenderRecord);
    // Phase 2: apply the record's player/style value to the common elements.
    if (!EnemyRowFlags(Sing_805DC648(), recordByteOffset)->flag) {
        INIT_GLOBAL_UI_FIRST();
        Layout_SetElementPlayerFlag(PanelLayout(self), index + kNonEnemyRowElement, 1,
                    EnemyRowLayoutValue(Sing_805DC648(), recordByteOffset));
    }
    INIT_GLOBAL_UI();
    Layout_SetElementPlayerFlag(PanelLayout(self), index + kStyledRowElement16, 1,
                EnemyRowLayoutValue(Sing_805DC648(), recordByteOffset));
    INIT_GLOBAL_UI();
    Layout_SetElementPlayerFlag(PanelLayout(self), index + kStyledRowElement20, 1,
                EnemyRowLayoutValue(Sing_805DC648(), recordByteOffset));
    INIT_GLOBAL_UI();
    Layout_SetElementPlayerFlag(PanelLayout(self), index + kStyledRowElement24, 1,
                EnemyRowLayoutValue(Sing_805DC648(), recordByteOffset));
    INIT_GLOBAL_UI();
    if (EnemyRowFlags(Sing_805DC648(), recordByteOffset)->flag) {
        // Phase 3: enemy records hide all five level indicators before revealing
        // the selected one. Out-of-range stored levels leave all five hidden.
        Layout_SetElementPlayerFlag(PanelLayout(self), index + LevelElement(0), 0, kLayoutHiddenStyle);
        Layout_SetElementPlayerFlag(PanelLayout(self), index + LevelElement(1), 0, kLayoutHiddenStyle);
        Layout_SetElementPlayerFlag(PanelLayout(self), index + LevelElement(2), 0, kLayoutHiddenStyle);
        Layout_SetElementPlayerFlag(PanelLayout(self), index + LevelElement(3), 0, kLayoutHiddenStyle);
        Layout_SetElementPlayerFlag(PanelLayout(self), index + LevelElement(4), 0, kLayoutHiddenStyle);
        INIT_GLOBAL_UI();
        switch (*(signed char*)&EnemyRow(Sing_805DC648(), recordByteOffset)->levelIndex) {
        case 0: INIT_GLOBAL_UI(); Layout_SetElementPlayerFlag(PanelLayout(self), index + LevelElement(0), 1, EnemyRowLayoutValue(Sing_805DC648(), recordByteOffset)); break;
        case 1: INIT_GLOBAL_UI(); Layout_SetElementPlayerFlag(PanelLayout(self), index + LevelElement(1), 1, EnemyRowLayoutValue(Sing_805DC648(), recordByteOffset)); break;
        case 2: INIT_GLOBAL_UI(); Layout_SetElementPlayerFlag(PanelLayout(self), index + LevelElement(2), 1, EnemyRowLayoutValue(Sing_805DC648(), recordByteOffset)); break;
        case 3: INIT_GLOBAL_UI(); Layout_SetElementPlayerFlag(PanelLayout(self), index + LevelElement(3), 1, EnemyRowLayoutValue(Sing_805DC648(), recordByteOffset)); break;
        case 4: INIT_GLOBAL_UI(); Layout_SetElementPlayerFlag(PanelLayout(self), index + LevelElement(4), 1, EnemyRowLayoutValue(Sing_805DC648(), recordByteOffset)); break;
        }
    }
    // Phase 4: force the main row through hidden before applying its visible style.
    Layout_SetElementPlayerFlag(PanelLayout(self), index + kRowVisibilityElement, 0, kLayoutHiddenStyle);
    INIT_GLOBAL_UI();
    Layout_SetElementPlayerFlag(PanelLayout(self), index + kRowVisibilityElement, 1,
                EnemyRowLayoutValue(Sing_805DC648(), recordByteOffset));
}
#pragma use_lmw_stmw off

#pragma use_lmw_stmw on
// Replace an enemy record for the selected level, rebuild its avatar, and refresh the level controls.
// index is 0..3; selectedLevel is normally 0..4. A negative value skips record
// replacement and clears all five controls. A nonnegative unchanged level skips
// replacement/avatar setup but still refreshes controls. No input validation added.
void CCharaEntry_SetEnemyLevelSelection(void* self, int index, int selectedLevel) {
    int idContext;
    int selectionContext;
    int recordByteOffset;
    int enemyData;
    if (selectedLevel >= 0) {
        INIT_GLOBAL_UI2();
        recordByteOffset = index * sizeof(CEnemyRenderRecord);
        if (selectedLevel != *(signed char*)&EnemyRow(Sing_805DC648(), recordByteOffset)->levelIndex) {
            // Phase 1: obtain enemy data and replace the selected render record.
            // Repeated singleton expressions preserve the original call/lifetime order.
            if (!((int&)Sing_805DC678())) {
                if (!s_pangyaEnemyDataInitialized) {
                    CPangyaEnemyData_Init(s_pangyaEnemyData);
                    __register_global_object(s_pangyaEnemyData, CPangyaEnemyData_dtor, s_pangyaEnemyDataDtorRegistration_CharacterEntry);
                    s_pangyaEnemyDataInitialized = 1;
                }
                ((int&)Sing_805DC678()) = (int)s_pangyaEnemyData;
            }
            enemyData = ((int&)Sing_805DC678());
            INIT_GLOBAL_UI2();
            selectionContext = ((int&)Sing_805DC648());
            INIT_GLOBAL_UI2();
            idContext = ((int&)Sing_805DC648());
            INIT_GLOBAL_UI2();
            char* layoutValueContext = (char*)((int&)Sing_805DC648());
            EnemyData_SelectRandomMatchingRecord((void*)enemyData, EnemyRow((void*)selectionContext, recordByteOffset), selectedLevel,
                        EnemyRow((void*)idContext, recordByteOffset)->characterInventoryIndex, EnemyRowLayoutValue(layoutValueContext, recordByteOffset),
                        (void*)(selectionContext + recordByteOffset));
            if (g_sharedDoc->gameMode == kEnemyModeWithoutCaddie) {
                INIT_GLOBAL_UI2();
                EnemyRow(Sing_805DC648(), recordByteOffset)->caddieInventoryIndex = -1;
            }
            // Phase 2: rebuild the preview. bounds = {left, top, right, bottom}
            // in screen pixels; previewScale is dimensionless character framing.
            int bounds[4];
            int avatarLeft = index * kAvatarWidth;
            bounds[0] = avatarLeft;
            bounds[1] = kAvatarTop;
            bounds[2] = avatarLeft + kAvatarWidth;
            bounds[3] = kAvatarBottom;
            float previewScale = kFloat_15_EnemyRender;
            CCharacterPreviewTable_SetupRecordAvatar(((RenderContext*)RenderContext_GetOrCreate())->players, index, bounds, 0, previewScale);
            if (EnemyRow(RenderContext_GetOrCreate(), recordByteOffset)->renderObject)
                CAsyncExhibition_SetLoadingIndicatorPosition(EnemyRow(RenderContext_GetOrCreate(), recordByteOffset)->renderObject, characterEntryRenderPositions + index * 8);
            INIT_GLOBAL_UI2();
            EnemyRow(Sing_805DC648(), recordByteOffset)->flags |= kEnemyNameChanged;
        }
    }
    // Phase 3: select one of five level controls, or clear all when none matches.
    int element = index + kFirstLevelElement;
    for (int i = 0; i < kEnemyLevelCount; ++i, element += kLevelElementStride) {
        if (i == selectedLevel) {
            Layout_SetElementEnabled(PanelLayout(self), element, 1);
            INIT_GLOBAL_UI2();
            Layout_SetElementPlayerFlag(PanelLayout(self), element, 1,
                        EnemyRowLayoutValue(Sing_805DC648(), index * sizeof(CEnemyRenderRecord)));
        } else {
            Layout_SetElementPlayerFlagsAndState(PanelLayout(self), element, 0);
        }
    }
}

#pragma use_lmw_stmw reset
// Report whether the character-entry selection has been confirmed.
int CCharaEntry_IsSelectionConfirmed(CharacterEntryConfirmation* self) {
    return self->selectionConfirmed != 0;
}

// Start the entry fade or request the pending transition when leaving.
// enabled != 0 starts fade-in; zero requests transition only if fade state !=2.
// The receiver argument is unused; all operations target the shared fade object.
void CCharaEntry_StartFadeInOrRequestTransition(void*, int enabled) {
    if (enabled) {
        if (!((int&)Sing_805DC608())) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayDtorRegistration_CharacterEntry);
                s_fadeOverlayInitialized = 1;
            }
            ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
        }
        FadeState_StartFadeIn((void*)((int&)Sing_805DC608()), 0);
    } else {
        if (!((int&)Sing_805DC608())) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayDtorRegistration_CharacterEntry);
                s_fadeOverlayInitialized = 1;
            }
            ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
        }
        if (FadeState_GetState((void*)((int&)Sing_805DC608())) != kFadeComplete) {
            if (!((int&)Sing_805DC608())) {
                if (!s_fadeOverlayInitialized) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayDtorRegistration_CharacterEntry);
                    s_fadeOverlayInitialized = 1;
                }
                ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
            }
            FadeState_RequestFramebufferCapture((void*)((int&)Sing_805DC608()));
        }
    }
}

// Run and draw the entry scripts, then draw either the open shop or the character-entry screen.
void CCharaEntry_DrawEntryScreen(CCharaEntry* obj) {
    // Phase 1: invoke both entry/no-entry scripts, then draw them in that order.
    // Each lookup result is borrowed; lazy BP initialization is repeated at uses.
    if (!((int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80506F04);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsCCharaEntry(obj)->noEntryScriptId), 0);
    if (!((int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80506F04);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsCCharaEntry(obj)->entryScriptId), 0);
    if (!((int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80506F04);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    BpObject_Draw(IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsCCharaEntry(obj)->noEntryScriptId));
    if (!((int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80506F04);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    BpObject_Draw(IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsCCharaEntry(obj)->entryScriptId));
    // Phase 2: a nonnegative selected player draws the shop and ends this pass.
    if (obj->selectedPlayerIndex >= 0) {
        MPBase1_DrawIfInitialized(AsCCharaEntry(obj)->shop);
        return;
    }
    // Phase 3: otherwise draw avatar previews, layout, then the stats script.
    CCharacterPreviewTable_DrawAvatars(((RenderContext*)RenderContext_GetOrCreate())->players);
    Layout_DrawWithOpacity(AsCCharaEntry(obj)->layout, kFloat_1_CharaEntry);
    if (!((int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80506F04);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsCCharaEntry(obj)->characterStatsScriptId), 0);
    if (!((int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80506F04);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    BpObject_Draw(IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsCCharaEntry(obj)->characterStatsScriptId));
}

} // extern "C"
