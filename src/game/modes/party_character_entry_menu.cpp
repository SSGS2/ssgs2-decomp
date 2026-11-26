// Four-player party character entry: the per-frame update that advances each
// player's ready state, entry and enemy-level choices, hands a selected player
// off to the Pangya shop, and returns once every player is ready or cancels.
#include "game/vector.h"
#include "classes/CSharedDoc.h"
#include "classes/CEnemyRenderTable.h"
#include "classes/CCharaEntry.h"
#include "classes/CAsyncExhibition.h"

struct CharacterEntryStringView {
    char resourceNames[0x4F];
    char selectionMusic[11];
    char partyTask[10];
    char fightTask[10];
    char confirmSound[19];
};
#define EntryStrings ((CharacterEntryStringView*)characterEntryStrings)
enum EntryMenuConstants {
    kEntryPlayerCount = 4,
    kPartyEntryMode = 7,
    kCancelAction = 6,
    kShopPending = 0,
    kShopWaitForLoad = 1,
    kShopProcess = 2,
    kFadeBusy = 3,
    kShopFeedbackFrames = 5,
    kAllReadyDelayFrames = 3
};
enum EntryRecordContract {
    kFeedbackWordsPerPlayer = 3,
    kFeedbackActiveWord = 0,
    kFeedbackTimerWord = 1,
    kQueuedShopSelectionWord = 2,
    kNoQueuedShop = 5,
    kEntryPreviewNameChanged = 0x4000
};
// Layout groups use base + player (0..3). Event -> paired feedback element:
//   04 ready -> 08 ready/cancel row (no paired timed highlight here);
//   08 refresh row; 0c open shop -> 30; 10 cycle entry -> 34;
//   1c/20/24/28/2c enemy level 1/2/3/4/0 -> 40/44/48/4c/50.
// Bases 38 and 3c are reset alongside the others, but their control meanings
// are unresolved here (no 14/18 event branches in this function).
// All nine feedback bases 30..50 are cleared when a player's timer reaches 5.
enum EntryFeedbackElement {
    kShopFeedbackBase = 0x30,
    kCycleFeedbackBase = 0x34,
    kUnresolvedFeedback38 = 0x38,
    kUnresolvedFeedback3C = 0x3C,
    kLevel1FeedbackBase = 0x40,
    kLevel2FeedbackBase = 0x44,
    kLevel3FeedbackBase = 0x48,
    kLevel4FeedbackBase = 0x4C,
    kLevel0FeedbackBase = 0x50
};
// Each event range contains four player IDs: event - base gives player 0..3.
// These bases index CONTROLS; EntryFeedbackElement indexes their timed visuals.
enum EntryEventBase {
    kReadyEventBase = 0x04,
    kRefreshEventBase = 0x08,
    kShopEventBase = 0x0C,
    kCycleEventBase = 0x10,
    kUnresolvedEvent14 = 0x14,
    kUnresolvedEvent18 = 0x18,
    kLevel1EventBase = 0x1C,
    kLevel2EventBase = 0x20,
    kLevel3EventBase = 0x24,
    kLevel4EventBase = 0x28,
    kLevel0EventBase = 0x2C
};
enum EntryVisualState {
    kFeedbackIdle = 0,
    kFeedbackPressed = 1
};
enum {
    kEntrySharedPlayerChannel = 5,
    kEntryResumeScript = 14,
    kEntryResumeFadeArgument = 30
};
extern "C" void __construct_array(void*, void*, void*, unsigned int, unsigned int);

#include "game/singletons.h"
#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>
#include "classes/CPangya2Shop.h"
#include "classes/CCharacterPreviewTable.h"
#include "classes/CBpCtrlHandle.h"
#include "classes/CBpCtrl.h"

extern "C" {
extern char s_bpCtrlDtorRegistration_80506F04[];
extern char s_bpCtrlHandleDtorRegistration_CharacterEntry[];
extern char s_fadeOverlayDtorRegistration_CharacterEntry[];
extern char s_renderContextDtorRegistration_80506F1C[];

void RenderContext_ResetPlayerRecords(void*);
void* CharacterPreviewTable_dtor(void*, int);
void appFreeUni__FPv(void*);

extern int Layout_SetElementPlayerFlagsAndState(void*, int, int);
extern void* RenderContext_GetOrCreate();
extern int MPBase1_Process(void*, float);
extern int Layout_Update(void*);
extern int Layout_CollectActivated(void*, void*, void*);
extern int Layout_SetElementEnabled(void*, int, int);
extern void Layout_SetElementPlayerFlag(void*, int, int, int);
extern int Input_IsActionPressed(void*, int, int);
extern int FadeState_GetState(void*);
extern void FadeState_StartFadeOut(void*, int);
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern void SoundManager_RequestBgmChange(void*, const char*, int, int);
extern void InputManager_Init(void*);
extern void InputManager_SetSelectedPlayer(void*, int);
extern void Layout_SetActive(void*, int);
extern void* IdMap_Lookup(void*, int);
extern void BpObject_SetEnabled(void*, int);
extern void FadeOverlay_Init(void*);
extern void ResourceState_Dtor(void*, int);
extern void RenderContext_ResetPlayerRecords(void*);
extern void TaskCommandQueue_AppendNamedRequest(void*, int, const char*, int, int, int, int, int);
extern void SaveData_InitPlayerRecord(void*);

extern void PartyItemRecord_InitDefaults(void*);
extern void PlayerRenderRecord_dtor(void*);
extern void __dt__13RenderContextFv(void*);
extern void InputManager_Dtor(void*, int);
extern void CBpCtrlHandle_Dtor(void*, int);
extern void appFreeUni__FPv(void*);
extern signed char s_renderContextInitialized;
extern signed char s_inputManagerInitialized;
extern signed char s_bpCtrlInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern signed char s_fadeOverlayInitialized;
extern int g_sharedDoc;
extern int g_taskManager;
extern int g_soundManager;
extern char s_renderContext[];
extern char s_fadeOverlay[];
extern char s_gameInputManager[];
extern char s_bpCtrl[];
extern char characterEntryRenderPositions[];
extern char characterEntryStrings[];
extern char s_inputManagerDtorRegistration_CharacterEntry[];
extern const float kFloat_15_EnemyRender, kFloat_0_5_CharacterEntrySound, kFloat_200_CharacterEntrySound,
    kFloat_1_CharaEntry;

struct EntryFlags {
    CW_BITS4(unsigned short top : 2, unsigned short alternate : 1, unsigned short enabled : 1,
             unsigned short lower : 12)
};
struct Entry {
    int object;
    char pad04[8];
    int selection;
    char pad10[8];
    int player;
    char pad1C[2];
    union {
        EntryFlags flags;
        unsigned short fullFlags;
    };
    char pad20[0x80];
    void* target;
};
struct Pool {
    char pad[0x20D0];
    Entry entries[4];
};
// statBarTransitions is four flat {active, elapsedUpdates, queuedShopSelection} triples.
// Active increments elapsedUpdates once per call, BEFORE this call's UI events.
// At exactly five it clears active/timer and all feedback, then opens a queued
// shop unless selection==5 (none), replacing that selection with the sentinel.
// Equality is intentional; do not change to >= or reset timers at new sites
// without checking event scheduling. These are update counts, not dt seconds.
struct Record {
    int active, timer, value;
};
struct EntryVectorView {
    void* data;
    int count;
    unsigned int capacity;
};
struct EntryVectorPool {
    char pad[0x20A0];
    EntryVectorView entryVectors[4];
};
struct EntryRenderContextView {
    char playerSaveRecords[0x20A0];
    EntryVectorView entryVectors[kEntryPlayerCount];
    CEnemyRenderTable previewTable;
};
struct EntryShopLoadView {
    char prefix[8];
    int loadComplete;
};
enum EntryPreviewGeometry {
    kPreviewColumnPixels = 150,
    kPreviewTopPixels = 120,
    kPreviewBottomPixels = 390,
    kPlayerSaveRecordBytes = 0x828,
    kPreviewTableOffset = 0x20D0
};

// Borrow a player-vector count using its documented layout and original address grouping.
static inline int& EntryVectorCount(void* context, int playerIndex) {
    return *(int*)((char*)context + (playerIndex * 12 + (int)&((EntryVectorPool*)0)->entryVectors[0].count));
}

// Select feedback by event base while retaining the original combined address expression.
static inline Record& FeedbackForEvent(int* records, int eventId, int eventBase) {
    return *(Record*)((char*)((Record*)records + eventId) - sizeof(Record) * eventBase);
}
#define ENTRY(p, index) (((Pool*)(p))->entries[index])
#define FLAGS(p, index) ENTRY(p, index).flags

// Return the stable render-context destructor-registration block for this build form.
static inline char* EntryRenderContextRegistration(char* regmem) {
    return (regmem + 0x3C);
}

// Ensure singleton storage and registration before each access. regmem is the
// caller-supplied destructor-registration block, not temporary storage. GAME's
// construction block remains a macro: extracting it changes generated .text.
// TODO: model native context member construction to recover its exception
// cleanup entries without adding normal-path cleanup branches.
void CBpCtrl_dtor(void*, int);
#define GAME(registration)                                                                                            \
    do {                                                                                                              \
        if (!((int&)Sing_805DC648())) {                                                                               \
            if (!s_renderContextInitialized) {                                                                        \
                char* end;                                                                                            \
                char* p;                                                                                              \
                char* base;                                                                                           \
                base = s_renderContext;                                                                               \
                p = base;                                                                                             \
                end = base + 0x20A0;                                                                                  \
                do {                                                                                                  \
                    SaveData_InitPlayerRecord(p);                                                                     \
                    p += 0x828;                                                                                       \
                } while (p < end);                                                                                    \
                ((IntVectorArray*)(end))->__ct();                                                                     \
                end = base + 0x20D0;                                                                                  \
                __construct_array(end, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4); \
                ((CEnemyRenderTable*)end)->count = 0;                                                                 \
                RenderContext_ResetPlayerRecords(base);                                                               \
                __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv, registration);              \
                s_renderContextInitialized = 1;                                                                       \
            }                                                                                                         \
            ((int&)Sing_805DC648()) = (int)s_renderContext;                                                           \
        }                                                                                                             \
    } while (0)

// Ensure the input singleton exists and its destructor has stable registration.
static inline void EnsureInputManager(char* regmem) {
    if (!((int&)GetInputManagerSlot())) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, regmem);
            s_inputManagerInitialized = 1;
        }
        ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
    }
}

// Ensure the BP script controller exists before looking up script entry IDs.
static inline void EnsureBpController(char* regmem) {
    if (!((int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (regmem + 0x24));
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
}

// Ensure the fade singleton exists before polling or starting a transition.
static inline void EnsureFadeOverlay(char* regmem) {
    if (!((int&)Sing_805DC608())) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, (regmem + 0x18));
            s_fadeOverlayInitialized = 1;
        }
        ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
    }
}

extern void CBpCtrl_dtor(void*, int);

struct ActivatedIDs {
    int* data;
    unsigned int size;
    unsigned int capacity;
    // Start an empty owned array of activated layout element IDs.
    ActivatedIDs() : data(0), size(0), capacity(0) {}
    // Clear and release the collected ID buffer.
    ~ActivatedIDs() {
        if (data) {
            size -= ((char*)data + size) - (char*)data;
            appFreeUni__FPv(data);
        }
    }
};
struct ActivatedValues {
    int* data;
    unsigned int size;
    unsigned int capacity;
    // Start an empty owned array of values parallel to activated IDs.
    ActivatedValues() : data(0), size(0), capacity(0) {}
    // Clear and release the collected value buffer.
    ~ActivatedValues() {
        if (data) {
            size -= ((char*)data + size) - (char*)data;
            appFreeUni__FPv(data);
        }
    }
};

// Place one player's avatar in its 150-pixel column of the entry screen.
inline void UpdateEntryRect(int index, float previewScale) {
    int rect[4];
    rect[1] = kPreviewTopPixels;
    rect[3] = kPreviewBottomPixels;
    int left = index * kPreviewColumnPixels;
    rect[0] = left;
    rect[2] = left + kPreviewColumnPixels;
    CCharacterPreviewTable::SetupRecordAvatar((char*)RenderContext_GetOrCreate() + 0x20D0, index, rect, 0,
                                              previewScale);
}

// Queue a shop choice for the event's player; feedback expires before the shop opens.
// ids->data[index] is 0C..0F; selection is the parallel activated-value entry.
// The original address form is retained for code generation:
// records + event*12 - 0x90 == records + (event-kShopEventBase)*12;
// -0x88 selects the queued-selection word of that same player record.
static inline void QueueShopFeedback(CCharaEntry* self, ActivatedIDs* ids, int index, int widget, int selection) {
    Layout_SetElementPlayerFlagsAndState(self->layout, widget, kFeedbackPressed);
    FeedbackForEvent(self->statBarTransitions, ids->data[index], kShopEventBase).active = 1;
    FeedbackForEvent(self->statBarTransitions, ids->data[index], kShopEventBase).value = selection;
}

// Show one player's level feedback, apply the level, and refresh all stat bars.
// player is 0..3, selection is enemy level 0..4; no timer reset occurs here.
static inline void SelectEnemyLevelFeedback(CCharaEntry* self, int widget, int player, int selection) {
    Layout_SetElementPlayerFlagsAndState(self->layout, widget, kFeedbackPressed);
    self->statBarTransitions[player * kFeedbackWordsPerPlayer] = 1;
    self->SetEnemyLevelSelection(player, selection);
    self->UpdateStatBars();
}

} // extern "C"

// Advance shop transitions, dispatch player controls, and finish once every player is ready.
int CCharaEntry::Process(float elapsedSeconds) {
    // Returns 0 while active, 1 for either accepted or cancelled completion;
    // result distinguishes accept(1)/cancel(0). The layout, context and shop
    // objects are borrowed; the temporary activation arrays own their buffers.
    // Phase 1: expire prior feedback and launch delayed shop requests.
    char* regmem;
    int recordOffset;
    int i;
    regmem = s_inputManagerDtorRegistration_CharacterEntry;
    for (i = 0, recordOffset = 0; i < 4; ++i, recordOffset += kFeedbackWordsPerPlayer) {
        if (statBarTransitions[recordOffset] != 0) {
            ++statBarTransitions[recordOffset + 1];
            if (statBarTransitions[recordOffset + 1] == kShopFeedbackFrames) {
                statBarTransitions[recordOffset + 1] = 0;
                statBarTransitions[recordOffset] = 0;
                Layout_SetElementPlayerFlagsAndState(layout, i + kShopFeedbackBase, 0);
                Layout_SetElementPlayerFlagsAndState(layout, i + kCycleFeedbackBase, 0);
                Layout_SetElementPlayerFlagsAndState(layout, i + kUnresolvedFeedback38, 0);
                Layout_SetElementPlayerFlagsAndState(layout, i + kUnresolvedFeedback3C, 0);
                Layout_SetElementPlayerFlagsAndState(layout, i + kLevel1FeedbackBase, 0);
                Layout_SetElementPlayerFlagsAndState(layout, i + kLevel2FeedbackBase, 0);
                Layout_SetElementPlayerFlagsAndState(layout, i + kLevel3FeedbackBase, 0);
                Layout_SetElementPlayerFlagsAndState(layout, i + kLevel4FeedbackBase, 0);
                Layout_SetElementPlayerFlagsAndState(layout, i + kLevel0FeedbackBase, 0);
                if (statBarTransitions[recordOffset + 2] != kNoQueuedShop) {
                    CCharacterPreviewTable::SetScriptEnabled((char*)RenderContext_GetOrCreate() + 0x20D0, 0);
                    OpenPlayerShop(i, statBarTransitions[recordOffset + 2]);
                    statBarTransitions[recordOffset + 2] = kNoQueuedShop;
                }
            }
        }
    }
    if (selectedPlayerIndex >= 0) {
        // Phase 2: pending -> load poll -> process may all run in this call.
        // A completed load polls MPBase1_Process again in kShopProcess. Shop
        // activity always returns before entry-screen input handling below.
        switch (shopState) {
        case kShopPending:
            shopState = kShopWaitForLoad; // Fall through to the load poll.
        case kShopWaitForLoad:
            MPBase1_Process(shop, elapsedSeconds);
            if (((EntryShopLoadView*)shop)->loadComplete == 0)
                return 0;
            shopState = kShopProcess; // Fall through to normal shop processing.
        case kShopProcess:
            if (MPBase1_Process(shop, elapsedSeconds)) {
                // Phase 2a: commit the shop snapshot, rebuild the render record,
                // then restore its preexisting enabled/alternate flag values.
                // Persist snapshot FIRST; rebuilding the descriptor would lose
                // changes otherwise. Preserve enabled/alternate across refill.
                GAME(EntryRenderContextRegistration(regmem));
                CPangya2Shop::CopySnapshotToRecord(shop, (char*)((int&)Sing_805DC648()) + selectedPlayerIndex * 0x828);
                GAME(EntryRenderContextRegistration(regmem));
                int enabled = FLAGS(((int&)Sing_805DC648()), selectedPlayerIndex).enabled != 0;
                GAME(EntryRenderContextRegistration(regmem));
                int alternate = FLAGS(((int&)Sing_805DC648()), selectedPlayerIndex).alternate != 0;
                GAME(EntryRenderContextRegistration(regmem));
                char* first = (char*)((int&)Sing_805DC648());
                GAME(EntryRenderContextRegistration(regmem));
                char* second = (char*)((int&)Sing_805DC648());
                GAME(EntryRenderContextRegistration(regmem));
                ((CCharacterPreviewRecord*)&ENTRY(first, selectedPlayerIndex))
                    ->FillFromSaveData(ENTRY(second, selectedPlayerIndex).object,
                                       (char*)((int&)Sing_805DC648()) + selectedPlayerIndex * 0x828);
                if (((CSharedDoc*)g_sharedDoc)->gameMode == kPartyEntryMode) {
                    GAME(EntryRenderContextRegistration(regmem));
                    ENTRY(((int&)Sing_805DC648()), selectedPlayerIndex).selection = -1;
                }
                GAME(EntryRenderContextRegistration(regmem));
                FLAGS(((int&)Sing_805DC648()), selectedPlayerIndex).enabled = enabled;
                GAME(EntryRenderContextRegistration(regmem));
                float previewScale = kFloat_15_EnemyRender;
                FLAGS(((int&)Sing_805DC648()), selectedPlayerIndex).alternate = alternate;
                int previewPlayerIndex = selectedPlayerIndex;
                // Phase 2b: rebuild the preview geometry and loading feedback
                // from the newly installed descriptor before resuming entry UI.
                UpdateEntryRect(previewPlayerIndex, previewScale);
                if (ENTRY(RenderContext_GetOrCreate(), previewPlayerIndex).target) {
                    ((CAsyncExhibition*)ENTRY(RenderContext_GetOrCreate(), previewPlayerIndex).target)
                        ->SetLoadingIndicatorPosition(
                            (const float*)(characterEntryRenderPositions + previewPlayerIndex * 8));
                }
                GAME(EntryRenderContextRegistration(regmem));
                ENTRY(((int&)Sing_805DC648()), selectedPlayerIndex).fullFlags |= kEntryPreviewNameChanged;
                SoundManager_RequestBgmChange(
                    (void*)g_soundManager,
                    (characterEntryStrings + (int)&((CharacterEntryStringView*)0)->selectionMusic), 0, 1);
                selectedPlayerIndex = -1;
                // Phase 2c: restore the entry screen's input/scripts and task,
                // begin its fade, then relinquish active shop processing.
                // Restore input, layout and three BP entries, select the party
                // or fight task, then run script 14 and start fade with argument 30.
                EnsureInputManager(regmem);
                InputManager_SetSelectedPlayer((void*)((int&)GetInputManagerSlot()), 0);
                Layout_SetActive(layout, 1);
                EnsureBpController(regmem);
                BpObject_SetEnabled(IdMap_Lookup((void*)((int&)Sing_805DC5F0()), (int)entryScriptId), 1);
                EnsureBpController(regmem);
                BpObject_SetEnabled(IdMap_Lookup((void*)((int&)Sing_805DC5F0()), (int)noEntryScriptId), 1);
                EnsureBpController(regmem);
                BpObject_SetEnabled(IdMap_Lookup((void*)((int&)Sing_805DC5F0()), (int)characterStatsScriptId), 1);
                if (((CSharedDoc*)g_sharedDoc)->gameMode == kPartyEntryMode) {
                    TaskCommandQueue_AppendNamedRequest(
                        (void*)g_taskManager, 0,
                        (characterEntryStrings + (int)&((CharacterEntryStringView*)0)->partyTask), 3, 0, 0, 0, 0);
                } else {
                    TaskCommandQueue_AppendNamedRequest(
                        (void*)g_taskManager, 0,
                        (characterEntryStrings + (int)&((CharacterEntryStringView*)0)->fightTask), 3, 0, 0, 0, 0);
                }
                if (!((int&)Sing_805DC5F8())) {
                    if (!s_bpCtrlHandleInitialized) {
                        s_bpCtrlHandle[1] = -1;
                        s_bpCtrlHandle[0] = -1;
                        __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (regmem + 0x30));
                        s_bpCtrlHandleInitialized = 1;
                    }
                    ((int&)Sing_805DC5F8()) = (int)s_bpCtrlHandle;
                }
                ((CBpCtrlHandle*)(((int&)Sing_805DC5F8())))->SetScriptIndex(kEntryResumeScript);
                EnsureFadeOverlay(regmem);
                FadeState_StartFadeOut((void*)((int&)Sing_805DC608()), kEntryResumeFadeArgument);
                UpdateStatBars();
                shopState = kShopPending;
                CCharacterPreviewTable::SetScriptEnabled((char*)RenderContext_GetOrCreate() + 0x20D0, 1);
                shopOpen = 0;
                ((CPangya2Shop*)(shop))->SetControlsEnabled(0);
            }
            return 0;
        }
        return 0;
    }

    // Phase 3: wait through fade/startup gating, collect parallel ID/value arrays
    // and dispatch per-player controls. Values are shop selections for 0c..0f.
    EnsureFadeOverlay(regmem);
    if (FadeState_GetState((void*)((int&)Sing_805DC608())) == kFadeBusy)
        return 0;
    if (started == 0) {
        started = 1;
        return 0;
    }
    Layout_Update(layout);
    ActivatedIDs ids;
    ActivatedValues values;
    if (Layout_CollectActivated(layout, &ids, &values)) {
        for (i = 0; (unsigned int)i < ids.size; ++i) {
            int value = values.data[i];
            int id = ids.data[i];
            switch (id) {
            // Ready: lock this player's entry controls and enable its refresh row.
            case kReadyEventBase:
            case kReadyEventBase + 1:
            case kReadyEventBase + 2:
            case kReadyEventBase + 3: {
                Sound_PlayByName(
                    (void*)g_soundManager, (characterEntryStrings + (int)&((CharacterEntryStringView*)0)->confirmSound),
                    0, 0, 0, 0, kFloat_0_5_CharacterEntrySound, kFloat_200_CharacterEntrySound, kFloat_1_CharaEntry);
                int player = ids.data[i] - kReadyEventBase;
                ready[player] = 1;
                Layout_SetElementPlayerFlagsAndState(layout, player + kReadyEventBase, kFeedbackIdle);
                Layout_SetElementEnabled(layout, player + kRefreshEventBase, 1);
                Layout_SetElementPlayerFlag(layout, player + kShopEventBase, 0, kEntrySharedPlayerChannel);
                Layout_SetElementPlayerFlag(layout, player + kCycleEventBase, 0, kEntrySharedPlayerChannel);
                Layout_SetElementPlayerFlag(layout, player + kUnresolvedEvent14, 0, kEntrySharedPlayerChannel);
                Layout_SetElementPlayerFlag(layout, player + kUnresolvedEvent18, 0, kEntrySharedPlayerChannel);
                Layout_SetElementPlayerFlag(layout, player + kLevel1EventBase, 0, kEntrySharedPlayerChannel);
                Layout_SetElementPlayerFlag(layout, player + kLevel2EventBase, 0, kEntrySharedPlayerChannel);
                Layout_SetElementPlayerFlag(layout, player + kLevel3EventBase, 0, kEntrySharedPlayerChannel);
                Layout_SetElementPlayerFlag(layout, player + kLevel4EventBase, 0, kEntrySharedPlayerChannel);
                Layout_SetElementPlayerFlag(layout, player + kLevel0EventBase, 0, kEntrySharedPlayerChannel);
                Layout_SetElementPlayerFlag(layout, player + kRefreshEventBase, 0, kEntrySharedPlayerChannel);
                GAME(EntryRenderContextRegistration(regmem));
                Layout_SetElementPlayerFlag(layout, player + kRefreshEventBase, 1,
                                            ENTRY(((int&)Sing_805DC648()), player).player);
                break;
            }
            case kRefreshEventBase:
            case kRefreshEventBase + 1:
            case kRefreshEventBase + 2:
            case kRefreshEventBase + 3:
                RefreshPlayerRow(id - kRefreshEventBase);
                break;
            // Shop: keep each fixed feedback ID; the selected player is read from
            // ids again AFTER the layout call, matching the existing alias order.
            case kShopEventBase:
                QueueShopFeedback(this, &ids, i, kShopFeedbackBase, value);
                break;
            case kShopEventBase + 1:
                QueueShopFeedback(this, &ids, i, kShopFeedbackBase + 1, value);
                break;
            case kShopEventBase + 2:
                QueueShopFeedback(this, &ids, i, kShopFeedbackBase + 2, value);
                break;
            case kShopEventBase + 3:
                QueueShopFeedback(this, &ids, i, kShopFeedbackBase + 3, value);
                break;
            case kCycleEventBase:
            case kCycleEventBase + 1:
            case kCycleEventBase + 2:
            case kCycleEventBase + 3:
                CyclePlayerEntryState(id - kCycleEventBase);
                // event*12 - 0xC0 addresses active for player (event-0x10).
                FeedbackForEvent(statBarTransitions, ids.data[i], kCycleEventBase).active = 1;
                break;
            // Level: fixed cases preserve player order and selection 1,2,3,4,0.
            case kLevel1EventBase:
                SelectEnemyLevelFeedback(this, kLevel1FeedbackBase, 0, 1);
                break;
            case kLevel1EventBase + 1:
                SelectEnemyLevelFeedback(this, kLevel1FeedbackBase + 1, 1, 1);
                break;
            case kLevel1EventBase + 2:
                SelectEnemyLevelFeedback(this, kLevel1FeedbackBase + 2, 2, 1);
                break;
            case kLevel1EventBase + 3:
                SelectEnemyLevelFeedback(this, kLevel1FeedbackBase + 3, 3, 1);
                break;
            case kLevel2EventBase:
                SelectEnemyLevelFeedback(this, kLevel2FeedbackBase, 0, 2);
                break;
            case kLevel2EventBase + 1:
                SelectEnemyLevelFeedback(this, kLevel2FeedbackBase + 1, 1, 2);
                break;
            case kLevel2EventBase + 2:
                SelectEnemyLevelFeedback(this, kLevel2FeedbackBase + 2, 2, 2);
                break;
            case kLevel2EventBase + 3:
                SelectEnemyLevelFeedback(this, kLevel2FeedbackBase + 3, 3, 2);
                break;
            case kLevel3EventBase:
                SelectEnemyLevelFeedback(this, kLevel3FeedbackBase, 0, 3);
                break;
            case kLevel3EventBase + 1:
                SelectEnemyLevelFeedback(this, kLevel3FeedbackBase + 1, 1, 3);
                break;
            case kLevel3EventBase + 2:
                SelectEnemyLevelFeedback(this, kLevel3FeedbackBase + 2, 2, 3);
                break;
            case kLevel3EventBase + 3:
                SelectEnemyLevelFeedback(this, kLevel3FeedbackBase + 3, 3, 3);
                break;
            case kLevel4EventBase:
                SelectEnemyLevelFeedback(this, kLevel4FeedbackBase, 0, 4);
                break;
            case kLevel4EventBase + 1:
                SelectEnemyLevelFeedback(this, kLevel4FeedbackBase + 1, 1, 4);
                break;
            case kLevel4EventBase + 2:
                SelectEnemyLevelFeedback(this, kLevel4FeedbackBase + 2, 2, 4);
                break;
            case kLevel4EventBase + 3:
                SelectEnemyLevelFeedback(this, kLevel4FeedbackBase + 3, 3, 4);
                break;
            case kLevel0EventBase:
                SelectEnemyLevelFeedback(this, kLevel0FeedbackBase, 0, 0);
                break;
            case kLevel0EventBase + 1:
                SelectEnemyLevelFeedback(this, kLevel0FeedbackBase + 1, 1, 0);
                break;
            case kLevel0EventBase + 2:
                SelectEnemyLevelFeedback(this, kLevel0FeedbackBase + 2, 2, 0);
                break;
            case kLevel0EventBase + 3:
                SelectEnemyLevelFeedback(this, kLevel0FeedbackBase + 3, 3, 0);
                break;
            }
        }
    }
    // Phase 4: require every participating player ready, then a post-increment
    // delay (threshold 3). Cancellation is checked only after readiness and is
    // suppressed during fade state 3. Finish by advancing avatar animations.
    int allReady = 1;
    int player = 0;
    // The shared context must be initialized before the first count check.
    // The supported statement expression keeps initialization in each loop test.
    while (({
        GAME(EntryRenderContextRegistration(regmem));
        player < ((EntryRenderContextView*)((int&)Sing_805DC648()))->previewTable.count;
    })) {
        if (ready[player] != 1)
            allReady = 0;
        ++player;
    }
    if (allReady == 1) {
        if (readyFrames++ < kAllReadyDelayFrames)
            allReady = 0;
    }
    if (allReady) {
        result = 1;
        return 1;
    }
    // Phase 5: poll cancellation only if readiness has not already completed.
    // This reads the context's per-player entry vectors, not statBarTransitions.
    for (int i = 0; i < kEntryPlayerCount; ++i) {
        GAME(EntryRenderContextRegistration(regmem));
        char* record = (char*)((int&)Sing_805DC648()) + i * 12;
        if (((EntryVectorPool*)record)->entryVectors[0].count != 0) {
            EnsureInputManager(regmem);
            if (Input_IsActionPressed((void*)((int&)GetInputManagerSlot()), kCancelAction, i)) {
                EnsureFadeOverlay(regmem);
                if (FadeState_GetState((void*)((int&)Sing_805DC608())) != kFadeBusy) {
                    GAME(EntryRenderContextRegistration(regmem));
                    EntryVectorCount((void*)((int&)Sing_805DC648()), i) -= 1;
                    result = 0;
                    return 1;
                }
            }
        }
    }
    // Phase 6: only the continuing entry-screen path advances avatar animations.
    CCharacterPreviewTable::Update((char*)RenderContext_GetOrCreate() + 0x20D0, elapsedSeconds);
    return 0;
}
