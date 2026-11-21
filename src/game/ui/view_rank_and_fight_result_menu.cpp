#include <decomp/static_init.h>
// Ranking and fight-result menus: script startup, timed reveal, score/Pang text,
// confirmation/cancellation and resource destruction. Mod lookup: change ranking
// row placement via ViewRank::rankRowByCharacter; refresh values/tints in
// ViewRank_RefreshPlayerScores; fight completion delay is the shared
// kFloat_5_FightResultAdvanceDelay consumed by CResultFightMenu_UpdateAndTryAdvance.
// Data: rankingViewStrings/fightResultStrings, ranking group/number-name tables;
// layouts: ViewRank.h, CResultFightMenu.h. Reveal state machine: result_menu_and_fight_result.cpp.
#include "classes/CSharedDoc.h"
// Borrow the corresponding typed view of existing object storage.
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
// Borrow the corresponding typed view of existing object storage.
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/ViewRank.h"
#include "classes/CMsgDraw.h"
#include "classes/UITextTable.h"
#include "classes/CRival.h"
#include "classes/GolfDocSnapshot8013.h"
// Borrow the corresponding typed view of existing object storage.
#define AsViewRank(p) ((ViewRank*)(p))

// Translation unit 0x803297B8-0x8032AE10 (18 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "game/wstring.h"
#include "classes/CResultFightMenu.h"

// Borrow the corresponding typed view of existing object storage.
#define AsCResultFightMenu(p) ((CResultFightMenu*)(p))

// Resource access map: Sing_805DC5F0 is the shared BP entry registry,
// Sing_805DC830 the numeric-text registry, Sing_805DC5C8 the input manager.
// Repeated ensure blocks intentionally re-read the singleton after external calls.
// IDs/enabled words use pointer-shaped legacy ABI slots, never owned pointers.
// Packed views of the existing resource pools; array lengths include each terminating NUL.
struct RankingViewStringPool {
    char archive[12], layout[15], script[15], blackSpriteScript[22], messages[12], numberFont[20];
    char confirmSound[19], blackSpriteAnimation[13], rankingAnimation[8], firstPlaceGroup[4], lastPlaceGroup[4], middlePlacesGroup[6];
};
struct FightResultStringPool {
    char layout[23], script[23], numberFont[18], introAnimation[22], confirmSound[14];
};
struct RankingSharedSelection { char prefix[0x120]; void* characterTypeId; };
// These counters occupy part of the snapshot header's aggregate result/stat storage.
struct RankingPlayerResult {
    char prefix[0xC4];
    unsigned char rankFlags; // high nibble: one-based placement
    unsigned char totalStrokes;
    char padding[2];
    int scoreToPar;
    int totalPang;
};
struct RankingHoleRecord { char prefix[0x18]; unsigned char par; char tail[7]; };
// The caller supplies a player index, score (or party-mode value), and base-plus-bonus Pang.
struct FightResultEntry { unsigned int playerIndex, scoreValue, totalPang; };
struct FightTextureSlot { char storage[0x28]; };
struct FightResultStorage {
    char prefix[0x3C];
    FightResultEntry entries[4];
    FightTextureSlot textures[4];
};
#define RESULT_OFF(type, field, offset) typedef char type##_##field##_offset[((int)&((type*)0)->field == offset) ? 1 : -1]
RESULT_OFF(RankingViewStringPool, confirmSound, 0x60);
RESULT_OFF(RankingViewStringPool, rankingAnimation, 0x80);
RESULT_OFF(FightResultStringPool, introAnimation, 0x40);
RESULT_OFF(FightResultStringPool, confirmSound, 0x56);
RESULT_OFF(RankingPlayerResult, scoreToPar, 0xC8);
RESULT_OFF(FightResultStorage, textures, 0x6C);
#undef RESULT_OFF
// Ranking maps character index [0,7] -> zero-based rank row; rankRowByCharacter is not
// an array of character indices sorted by place. Each character's score/name
// uses that same row mapping. Player placement is read from rankFlags' high nibble.
// Result rows are {playerIndex, score/party value, totalPang}; caller count <= 4.
// The four textures belong to the menu; script/number objects live in registries.
// Fight update: disabled -> 0; start intro once; await script; reveal rows; then
// wait five seconds after reveal or accept advance input; return 1.
// Ranking input: confirmation sets selectionResult=1, then activationDelay counts
// down four UPDATE CALLS before returning completion; input is skipped meanwhile.
// Cancel returns completion immediately with selectionResult=2. No delta is used.
enum RankingConstants {
    kCharacterCount = 8, kTextureCount = 4, kRankingActivationFrames = 4,
    kCancelAction = 3, kAlternateCancelAction = 2, kAnyInputPlayer = -1,
    kPlacementShift = 4, kPlacementMask = 15, kFirstPlace = 1, kLastPlace = 8, kRankRowSpacing = 28,
    kPlayerNameColor = 0xD0003EFF, kOtherNameColor = 0x000000FF,
    kPlayerNumberTint = 0xFF3E00D0, kOtherNumberTint = 0xFF000000,
    kFightLayoutElementCount = 27, kFightHiddenGroupCount = 26,
    kFightNumericEntryCount = 16, kRankingConfirmElement = 0,
    kRankingConfirmPlayerFlags = 5, kRankingConfirmVisualState = 2
};

extern "C" {

extern void Layout_Update(void*);
extern void ResultMenu_DrawSharedQuad();
extern void BpObject_TryInvokeCurrentCallback(void*, int);
extern void BpObject_Draw(void*);
extern int BpObject_IsCurrentScriptComplete(void*);
extern void Layout_DrawWithOpacity(void*, float);
extern void UITextTable_Draw(void*);
extern void CResultFightMenu_DrawLayoutTextureQuad(void*, unsigned int);
extern int Layout_PollLoadComplete(void*);
extern void CBpCtrl_dtor(void*);
extern int CBpCtrl_PollAllEntriesLoaded(void*);
extern void* IdMap_Lookup(void*, void*);
extern void BpObject_SetEnabled(void*, void*);
extern void __ct__7CBpCtrlFv(void*);
extern void __register_global_object(void*, void*, void*);
extern float kFloat_6_2831855;
extern float kFloat_1_ResultFightMenu;
extern float s_rankingViewTwoPi;
extern float s_fightResultTwoPi0;
extern float s_fightResultTwoPi1;
extern void UITextTable_ClearEntries(void*);
extern void ViewRank_DrawEntryText(void*);
extern void PointerArray_DeleteValue(void*, void*);
extern void PointerVector_InitReserved(void*);
extern void PointerVector_DestroyStorage(void*);
extern void CBpCtrl_DeleteEntryById(void*, void*);
extern void Layout_Destroy(void*, int);
extern void __destroy_arr(void*, void*, int, int);
extern void __dl__FPv(void*);
extern void InputManager_Init(void*);
extern void InputManager_Dtor(void*);
extern void InputManager_SetSelectedPlayer(void*, int);
extern void CFontNumDraw_RegisterEntries(void*, void*, int, void*, void*);
extern void MPBase1_Ctor(void*);
extern void* __nw__FUl(unsigned int);
extern void Layout_Init(void*, int);
extern void Layout_LoadExtended(void*, int, const char*);
extern void __construct_array(void*, void*, void*, int, int);
extern void CBpCtrl_CreateEntry(void*, const char*, void*, int);
extern void CBpTex_dtor(void*);
extern void CBpTex_Init(void*);
extern void __dt__16CWarningLogoBaseFv(void*);
extern void* CFontNumDraw_CreateInVector(void*);
extern void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
extern int SharedDoc_IsPlayerFlag0Set(int);
extern signed char s_pointerVectorInitialized;
extern void* g_sharedDoc;
extern void* g_rival;
extern void* g_soundManager;
extern signed char s_inputManagerInitialized;
extern char s_fontNumDrawRegistry[];
extern char s_fontNumDrawDtorRegistration_FightResults[];
extern char s_inputManagerDtorRegistration_FightResults[];
extern char s_gameInputManager[];
extern FightResultStringPool fightResultStrings;
extern void* s_fightResultInitiallyHiddenGroupNames[];
extern char s_fightResultNumberDescriptors[];
extern char __vt__16CResultFightMenu[];
extern float kFloat_1_Ranking;
extern float kFloat_5_FightResultAdvanceDelay;
extern float kFloat_0_5_FightResultConfirmSound;
extern float kFloat_200_FightResultConfirmSound;
extern const float kFloat_0_FightResultReveal;
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_inputManagerDtorRegistration_Ranking[];
extern char s_bpCtrlDtorRecord_Menu[];
extern char s_bpCtrlDtorRegistration_ResultFightMenu[];
extern int ItemInventory_FindCharacterIndex(void*, void*);
extern int RivalStrokeUse_GetStrokesThroughHole(void*, int, int);
extern int RivalStrokeUse_GetPangThroughHole(void*, int, int);
extern void Layout_SetElementEnabled(void*, int, int);
extern void BpObject_StartScriptByName(void*, void*, int);
extern void* BpObject_GetOrCreateNamedGroup(void*, void*);
extern void UIGroup_SetEnabled(void*, int);
extern void Text_SetPosition(void*, void*);
extern void CMsgDraw_ClearFormattingAndRevealAll(void*);
extern void CMsgDraw_SetColor(void*, int);
extern void CMsgDraw_UpdateReveal(void*);
extern void Text_DrawGlyphs(void*);
extern void Text_SetContent(void*, void*);
extern void WString_ReplaceRange(void*, int, int, int, int, unsigned char*);
extern int MessageTable_FindTextByName(void*, void*);
extern void* SaveData_AcquireInitializedInstance();
extern void* SaveData_GetCurrentRecord(void*);
extern int wcslen(void*);
extern int UIGroup_SetPosition(void*, void*);
extern int UIGroup_GetPosition(void*, void*);
extern int CMsgDraw_ResetTextState(void*);
extern void* s_rankingCharacterNameMessageKeys[];
extern void* s_rankingPositionGroupNames[];
extern int CResultFightMenu_UpdateRevealSequence(void*, float);
extern int InputManager_IsAnyMenuAdvanceActionActive(void*);
extern void Sound_PlayByName(void*, void*, int, int, int, int, float, float, float);

extern int Layout_CollectActivated(void*, void*, int);
extern void Layout_SetElementPlayerFlag(void*, int, int, int);
extern void Layout_SetElementValueState(void*, int, int);
extern void appFreeUni__FPv(void*);
extern int Input_IsActionPressed(void*, int, int);
extern float kFloat_0_5_RankingConfirmSound;
extern float kFloat_200_RankingConfirmSound;
extern RankingViewStringPool rankingViewStrings;
extern void UITextTable_SetInteger(void*, void*, int);
extern void UIGroup_SetTintColor(void*, int);
extern void* s_rankingScoreGroupNames[];
extern void* s_rankingPangGroupNames[];
extern void* s_rankingScoreNumberEntryNames[];
extern void* s_rankingPangNumberEntryNames[];

// Publish the shared BP registry, registering its owner only on first construction.
// registration is the caller module's static destructor record, never owned here.
static inline void EnsureResultBpRegistry(void* registration) {
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(&s_bpCtrl);
            __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor, registration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
}

// Draw both ranking scripts, the layout and character names; enable the numeric score table.
void ViewRank_Draw(void* arg) {
    EnsureResultBpRegistry(&s_bpCtrlDtorRecord_Menu);
    void* obj = IdMap_Lookup(Sing_805DC5F0(), (*(void**)&AsViewRank(arg)->secondaryEntryId));
    BpObject_TryInvokeCurrentCallback(obj, 0);
    BpObject_Draw(obj);
    EnsureResultBpRegistry(&s_bpCtrlDtorRecord_Menu);
    obj = IdMap_Lookup(Sing_805DC5F0(), (*(void**)&AsViewRank(arg)->entryId));
    BpObject_TryInvokeCurrentCallback(obj, 0);
    BpObject_Draw(obj);
    Layout_DrawWithOpacity(AsViewRank(arg)->layout, kFloat_1_Ranking);
    ViewRank_DrawEntryText(arg);
    AsUITextTable(AsViewRank(arg)->numberDraw)->drawEnabled = 1;
}

// Release numeric text, script entry, layout and four texture slots, then optionally delete this menu.
void* CResultFightMenu_dtor(void* arg, int flags) {
    if (arg) {
        AsCResultFightMenu(arg)->vtbl = &__vt__16CResultFightMenu;
        UITextTable_ClearEntries(AsCResultFightMenu(arg)->numberDraw);
        if ((*(int*)&AsCResultFightMenu(arg)->numberDraw)) {
            if (!Sing_805DC830()) {
                if (!s_pointerVectorInitialized) {
                    PointerVector_InitReserved(&s_fontNumDrawRegistry);
                    __register_global_object(&s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, &s_fontNumDrawDtorRegistration_FightResults);
                    s_pointerVectorInitialized = 1;
                }
                Sing_805DC830() = &s_fontNumDrawRegistry;
            }
            PointerArray_DeleteValue(Sing_805DC830(), AsCResultFightMenu(arg)->numberDraw);
        }
        if (AsCResultFightMenu(arg)->scriptEntryId != -1) {
            EnsureResultBpRegistry(&s_bpCtrlDtorRegistration_ResultFightMenu);
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), (*(void**)&AsCResultFightMenu(arg)->scriptEntryId));
            AsCResultFightMenu(arg)->scriptEntryId = -1;
        }
        void* p = AsCResultFightMenu(arg)->layout;
        if (p) {
            Layout_Destroy(p, 1);
            AsCResultFightMenu(arg)->layout = 0;
        }
        __destroy_arr(((FightResultStorage*)arg)->textures, (void*)CBpTex_dtor, 0x28, 4);
        if (flags > 0) __dl__FPv(arg);
    }
    return arg;
}

struct WStr32 {
    unsigned int flags;
    unsigned int size;
    void* data;
    // Release the temporary wide string heap buffer if present.
    ~WStr32() { if (flags >> 31) appFreeUni__FPv(data); }
    // Return borrowed inline/heap UTF-16 text without transferring ownership.
    void* chars() {
        if ((flags >> 31) == 0) return (char*)this + 2;
        return data;
    }
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_loop_invariants off
// Place each character name at its rank slot; use the save name and highlight color for the player.
void ViewRank_DrawEntryText(void* arg) {
    ItemInventory_FindCharacterIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, ((RankingSharedSelection*)g_sharedDoc)->characterTypeId);
    // Two raw 32-bit words hold screen-space float X/Y returned by the UI;
    // integer storage preserves the original word copies (no numeric conversion).
    int pos[2] = {0, 0};
    CMsgDraw_ResetTextState(AsViewRank(arg)->messageDraw);
    void* text;
    void* item = arg; // biased by 4 per character: only rankRowByCharacter[0] is read
    void** names = s_rankingCharacterNameMessageKeys;
    int index = 0;
    // Character-index iteration selects a rank-position group for each name.
    do {
        EnsureResultBpRegistry(&s_bpCtrlDtorRecord_Menu);
        UIGroup_GetPosition(BpObject_GetOrCreateNamedGroup(IdMap_Lookup(Sing_805DC5F0(), (*(void**)&AsViewRank(arg)->entryId)), s_rankingPositionGroupNames[AsViewRank(item)->rankRowByCharacter[0]]), pos);
        Text_SetPosition(AsViewRank(arg)->messageDraw, pos);
        CMsgDraw_ClearFormattingAndRevealAll(AsViewRank(arg)->messageDraw);
        {
            WString name;
            int len;
            if (index == ItemInventory_FindCharacterIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, ((RankingSharedSelection*)g_sharedDoc)->characterTypeId)) {
                void* extraReceiver5 = (void*)SaveData_AcquireInitializedInstance();
                text = SaveData_GetCurrentRecord(extraReceiver5);
                len = ((name.flags >> 31) == 0) ? (*(unsigned char*)&name.flags & 0x7F) : name.length;
                // Retain the allocator-token byte read emitted by the original
                // empty allocator ABI. It has no established game-state meaning;
                // initializing or removing it would change the matched code.
                volatile int c14;
                unsigned int c10;
                *(unsigned char*)&c10 = *(volatile unsigned char*)&c14;
                int n = wcslen(text);
                WString_ReplaceRange(&name, 0, len, (int)text, (int)((char*)text + n * 2), (unsigned char*)&c10);
                CMsgDraw_SetColor(AsViewRank(arg)->messageDraw, kPlayerNameColor);
            } else {
                text = (void*)MessageTable_FindTextByName(AsViewRank(arg)->taggedText, *names);
                len = ((name.flags >> 31) == 0) ? (*(unsigned char*)&name.flags & 0x7F) : name.length;
                // Same allocator-token ABI as the save-name path above.
                volatile int c0c;
                unsigned int c08;
                *(unsigned char*)&c08 = *(volatile unsigned char*)&c0c;
                int n = wcslen(text);
                WString_ReplaceRange(&name, 0, len, (int)text, (int)((char*)text + n * 2), (unsigned char*)&c08);
                CMsgDraw_SetColor(AsViewRank(arg)->messageDraw, 0xFF);
            }
            Text_SetContent(AsViewRank(arg)->messageDraw, ((name.flags >> 31) == 0) ? (void*)((char*)&name + 2) : name.data);
        }
        ++index;
        item = (char*)item + 4;
        ++names;
    } while (index < 8);
    CMsgDraw_UpdateReveal(AsViewRank(arg)->messageDraw);
    ((CMsgDraw*)AsViewRank(arg)->messageDraw)->drawSuppressed = 0; // original +0x94, followed by Text_DrawGlyphs
    Text_DrawGlyphs(AsViewRank(arg)->messageDraw);
}
#pragma pop

// Start ranking scripts and select the first-place, last-place or intermediate rank marker.
void ViewRank_StartRankingAnimation(void* arg) {
    int local[6];
    local[0] = 0;
    local[1] = 0;
    int* names;
    void* obj;
    void* state;
    int mode;
    EnsureResultBpRegistry(&s_bpCtrlDtorRecord_Menu);
    obj = IdMap_Lookup(Sing_805DC5F0(), (*(void**)&AsViewRank(arg)->secondaryEntryId));
    // blackSpriteAnimation is the secondary object's animation script name.
    BpObject_StartScriptByName(obj, rankingViewStrings.blackSpriteAnimation, 1);
    EnsureResultBpRegistry(&s_bpCtrlDtorRecord_Menu);
    obj = IdMap_Lookup(Sing_805DC5F0(), (*(void**)&AsViewRank(arg)->entryId));
    BpObject_StartScriptByName(obj, rankingViewStrings.rankingAnimation, 1);
    extern int s_rankingPlacementMarkerGroupNames[];
    names = &local[2];
    {
        int* p = (int*)s_rankingPlacementMarkerGroupNames;
        names[0] = *p;
        names[1] = p[1];
        names[2] = p[2];
    }
    for (unsigned int i = 0; i < 3; ++i) {
        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(obj, (void*)names[0]), 0);
        ++names;
    }
    state = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), 0);
    mode = (((RankingPlayerResult*)state)->rankFlags >> kPlacementShift) & kPlacementMask;
    switch (mode) {
        case 8:
            UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(obj, rankingViewStrings.lastPlaceGroup), 1);
            break;
        case 1:
            UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(obj, rankingViewStrings.firstPlaceGroup), 1);
            break;
        default:
            obj = BpObject_GetOrCreateNamedGroup(obj, rankingViewStrings.middlePlacesGroup);
            UIGroup_SetEnabled(obj, 1);
            state = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), 0);
            {
                unsigned char m = (((RankingPlayerResult*)state)->rankFlags >> kPlacementShift) & kPlacementMask;
                *(float*)&local[1] = (float)((m - 2) * 0x1C);
            }
            UIGroup_SetPosition(obj, local);
            break;
    }
}

struct FightTex { 
virtual void _0(); virtual void init(int);
};
struct FightLayout {
    char storage[0x5C];
    // Initialize the owned fight-result layout with the requested mode.
    FightLayout(int n) { Layout_Init(this, n); }
};
#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_strength_reduction off
// Create the multiplayer results layout, script, number table and four result textures.
void* CResultFightMenu_Ctor(void* arg, void* resultMode) {
    MPBase1_Ctor(arg);
    AsCResultFightMenu(arg)->vtbl = &__vt__16CResultFightMenu;
    AsCResultFightMenu(arg)->resultMode = resultMode;
    int zero = 0;
    (*(int*)&AsCResultFightMenu(arg)->layout) = zero;
    AsCResultFightMenu(arg)->scriptEntryId = -1;
    (*(int*)&AsCResultFightMenu(arg)->scriptObject) = zero;
    (*(int*)&AsCResultFightMenu(arg)->numberDraw) = zero;
    AsCResultFightMenu(arg)->displayStarted = zero;
    AsCResultFightMenu(arg)->introScriptComplete = zero;
    AsCResultFightMenu(arg)->elapsedTime = kFloat_0_FightResultReveal;
    (*(int*)&AsCResultFightMenu(arg)->entryCount) = zero;
    // Initialize embedded textures and reveal counters before acquiring UI resources.
    __construct_array(((FightResultStorage*)arg)->textures, (void*)CBpTex_Init, (void*)CBpTex_dtor, 0x28, 4);
    AsCResultFightMenu(arg)->revealState = zero;
    AsCResultFightMenu(arg)->revealRowIndex = zero;
    AsCResultFightMenu(arg)->scoreRevealStep = zero;
    AsCResultFightMenu(arg)->revealElapsedTime = kFloat_0_FightResultReveal;
    // Create the layout and register its script; the menu stores registry IDs/pointers.
    zero = (int)new FightLayout(5);
    AsCResultFightMenu(arg)->layout = (void*)zero;
    Layout_LoadExtended((void*)zero, 0, fightResultStrings.layout);
    EnsureResultBpRegistry(&s_bpCtrlDtorRegistration_ResultFightMenu);
    CBpCtrl_CreateEntry(Sing_805DC5F0(), fightResultStrings.script, &AsCResultFightMenu(arg)->scriptEntryId, 0);
    EnsureResultBpRegistry(&s_bpCtrlDtorRegistration_ResultFightMenu);
    AsCResultFightMenu(arg)->scriptObject = IdMap_Lookup(Sing_805DC5F0(), (void*)AsCResultFightMenu(arg)->scriptEntryId);
    if (!Sing_805DC830()) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(&s_fontNumDrawRegistry);
            __register_global_object(&s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, &s_fontNumDrawDtorRegistration_FightResults);
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = &s_fontNumDrawRegistry;
    }
    AsCResultFightMenu(arg)->numberDraw = CFontNumDraw_CreateInVector(Sing_805DC830());
    // Load one texture per fixed row using the existing resource-ID table.
    extern int s_fightResultTextureIds[];
    struct Arr4 { int v[4]; };
    Arr4 params;
    *(Arr4*)&params = *(Arr4*)s_fightResultTextureIds;
    int* pp = params.v;
    int offset;
    unsigned int i;
    i = 0;
    offset = 0;
    do {
        ((FightTex*)(((char*)((FightResultStorage*)arg)->textures + offset)))->init(*pp);
        ++i;
        ++pp;
        offset += 0x28;
    } while (i < 4);
    return arg;
}
#pragma pop

// Select the smallest eligible input selector, falling back to all players, and register score numbers.
void CResultFightMenu_InitPlayerAndScoreNumbers(void* arg) {
    int i = 0;
    // Prefer eligible players with the smallest first-word input selector (< 3).
    // If none qualify, repeat without the eligibility test. -1 only tracks discovery.
    int threshold = 3;
    int best = -1;
    while (i < AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount) {
        void* entry = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i);
        if (threshold > *(int*)entry && !SharedDoc_IsPlayerFlag0Set(i)) {
            threshold = *(int*)GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i);
            best = i;
        }
        ++i;
    }
    if (best < 0) {
        i = 0;
        while (i < AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount) {
            void* entry = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i);
            if (threshold > *(int*)entry) threshold = *(int*)GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i);
            ++i;
        }
    }
    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration_FightResults);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = &s_gameInputManager;
    }
    InputManager_SetSelectedPlayer(Sing_805DC5C8(), threshold);
    CFontNumDraw_RegisterEntries(AsCResultFightMenu(arg)->numberDraw, &s_fightResultNumberDescriptors, kFightNumericEntryCount, AsCResultFightMenu(arg)->scriptObject, fightResultStrings.numberFont);
    AsUITextTable(AsCResultFightMenu(arg)->numberDraw)->drawEnabled = 1;
}

// Initialize this result animation angle constant to one full turn.
void FightResult_InitFullTurnAngle1() {
    s_fightResultTwoPi1 = kFloat_6_2831855;
}

// entries is borrowed for the copy; count must not exceed the four embedded rows.
// This routine does not sort rows or clamp count.
// Copy the caller's player-index/score/Pang rows into the fixed result array; the caller supplies a valid count.
void CResultFightMenu_SetEntries(void* arg, void* entries, unsigned int count) {
    unsigned int v1;
    unsigned int v0;
    unsigned int v2;
    AsCResultFightMenu(arg)->entryCount = count;
    char* dst = (char*)arg;
    unsigned int* src = (unsigned int*)entries;
    unsigned int i = 0;
    while (i < AsCResultFightMenu(arg)->entryCount) {
        v0 = src[0];
        ++i;
        v1 = src[1];
        ((FightResultStorage*)dst)->entries[0].playerIndex = v0;
        v2 = src[2];
        src = (unsigned int*)((char*)src + sizeof(FightResultEntry));
        ((FightResultStorage*)dst)->entries[0].scoreValue = v1;
        ((FightResultStorage*)dst)->entries[0].totalPang = v2;
        dst += sizeof(FightResultEntry);
    }
}

struct VObj8C4 { 
virtual void _0(); virtual void _1(); virtual void _2(); virtual void _3(); virtual int _4();
};
// 0x803297B8 unit: lazily prepare the script controller before polling it.
static inline int CResultFightMenu_PollScriptEntries() {
    EnsureResultBpRegistry(&s_bpCtrlDtorRegistration_ResultFightMenu);
    return CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0());
}

// Report ready only after the layout, scripts and all four textures have loaded.
int CResultFightMenu_PollLoadComplete(void* arg) {
    if (!Layout_PollLoadComplete(AsCResultFightMenu(arg)->layout) ||
        !CResultFightMenu_PollScriptEntries()) return 0;
    FightTextureSlot* obj = ((FightResultStorage*)arg)->textures;
    for (unsigned int i = 0; i < 4; ++i) {
        if (!((VObj8C4*)obj)->_4()) return 0;
        ++obj;
    }
    return 1;
}

// Set the numeric table and both ranking scripts to the requested enabled state.
void ViewRank_SetEnabled(void* arg, void* value) {
    void* sub = AsViewRank(arg)->numberDraw;
    (*(void**)&AsUITextTable(sub)->drawEnabled) = value;
    EnsureResultBpRegistry(&s_bpCtrlDtorRecord_Menu);
    BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), (*(void**)&AsViewRank(arg)->entryId)), value);
    EnsureResultBpRegistry(&s_bpCtrlDtorRecord_Menu);
    BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), (*(void**)&AsViewRank(arg)->secondaryEntryId)), value);
}

// Draw active result scripts, layout, numeric table and each populated result texture.
void CResultFightMenu_Draw(void* arg) {
    if (AsCResultFightMenu(arg)->displayStarted != 0) {
        ResultMenu_DrawSharedQuad();
        BpObject_TryInvokeCurrentCallback(AsCResultFightMenu(arg)->scriptObject, 0);
        BpObject_Draw(AsCResultFightMenu(arg)->scriptObject);
        if (BpObject_IsCurrentScriptComplete(AsCResultFightMenu(arg)->scriptObject)) {
            Layout_DrawWithOpacity(AsCResultFightMenu(arg)->layout, kFloat_1_ResultFightMenu);
            UITextTable_Draw(AsCResultFightMenu(arg)->numberDraw);
            for (unsigned int i = 0; i < AsCResultFightMenu(arg)->entryCount; ++i) {
                CResultFightMenu_DrawLayoutTextureQuad(arg, i);
            }
        }
    }
}

// Advance the result layout.
void CResultFightMenu_UpdateLayout(void* arg) {
    Layout_Update(AsCResultFightMenu(arg)->layout);
}

#pragma push
#pragma use_lmw_stmw on
// Rank lower stroke totals first, then higher Pang; exact ties prefer the player, then lower character index.
void ViewRank_ComputeCharacterRanks(void* object) {
    void* list = ((CRival*)g_rival)->strokeUse;
    int playerCharacterIndex = ItemInventory_FindCharacterIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, ((RankingSharedSelection*)g_sharedDoc)->characterTypeId);
    int characterIndex = 0;
    int* rankDestination = (int*)object;
    do {
        ((ViewRank*)rankDestination)->rankRowByCharacter[0] = 0;
        // Player uses the game-provided placement; rival rows count better competitors.
        if (characterIndex == playerCharacterIndex) {
            void* info = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), 0);
            ((ViewRank*)rankDestination)->rankRowByCharacter[0] = ((((RankingPlayerResult*)info)->rankFlags >> kPlacementShift) & kPlacementMask) - 1;
        } else {
            int subjectStrokes = RivalStrokeUse_GetStrokesThroughHole(list, characterIndex, AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex + 1);
            int subjectPang = RivalStrokeUse_GetPangThroughHole(list, characterIndex, AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex + 1);
            int otherCharacterIndex = 0;
            while (otherCharacterIndex < kCharacterCount) {
                if (characterIndex != otherCharacterIndex) {
                    int otherStrokes, otherPang;
                    if (otherCharacterIndex == playerCharacterIndex) {
                        void* item = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), 0);
                        otherStrokes = ((RankingPlayerResult*)item)->totalStrokes;
                        item = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), 0);
                        otherPang = ((RankingPlayerResult*)item)->totalPang;
                    } else {
                        otherStrokes = RivalStrokeUse_GetStrokesThroughHole(list, otherCharacterIndex, AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex + 1);
                        otherPang = RivalStrokeUse_GetPangThroughHole(list, otherCharacterIndex, AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex + 1);
                    }
                    // Increment once if the other character precedes this rival:
                    // 1. lower stroke total; 2. equal strokes, higher Pang;
                    // 3. equal totals, other is the player; 4. equal totals,
                    // lower character index. The else-if chain counts at most once.
                    // The player's own row bypasses this comparison entirely and
                    // uses rankFlags placement, so it need not mirror rival ranks.
                    if (subjectStrokes > otherStrokes) ++((ViewRank*)rankDestination)->rankRowByCharacter[0];
                    else if (subjectStrokes == otherStrokes && subjectPang < otherPang) ++((ViewRank*)rankDestination)->rankRowByCharacter[0];
                    else if (subjectStrokes == otherStrokes && subjectPang == otherPang && otherCharacterIndex == playerCharacterIndex) ++((ViewRank*)rankDestination)->rankRowByCharacter[0];
                    else if (subjectStrokes == otherStrokes && subjectPang == otherPang && characterIndex > otherCharacterIndex) ++((ViewRank*)rankDestination)->rankRowByCharacter[0];
                }
                ++otherCharacterIndex;
            }
        }
        ++characterIndex;
        ++rankDestination;
    } while (characterIndex < kCharacterCount);
}
#pragma pop

// Start the results intro, reveal scores, and finish after the delay or an advance action.
// self must own a loaded layout/script and initialized reveal state. delta is
// elapsed seconds, the same clock passed to UpdateRevealSequence (result_menu_and_fight_result.cpp);
// supply finite nonnegative values. elapsedTime counts only calls whose reveal
// helper returns complete, so the baseline threshold is five POST-REVEAL seconds,
// not five seconds since menu activation. Larger delay prolongs this final wait;
// input can bypass it. Layout_Update still runs when updateEnabled==0.
// Returns 0 while disabled/waiting; 1 plays confirmSound and asks the owner to
// leave the menu. It does not latch completion: further calls can play it again.
// Layout/script/number resources remain menu-owned until the destructor.
int CResultFightMenu_UpdateAndTryAdvance(void* self, float delta) {
    Layout_Update(AsCResultFightMenu(self)->layout);
    if (!AsCResultFightMenu(self)->updateEnabled) return 0;
    // Phase 1: start the intro once and hide layout elements/script score groups.
    if (!AsCResultFightMenu(self)->displayStarted) {
        AsCResultFightMenu(self)->displayStarted = 1;
        unsigned int i = 0;
        for (int j = 0; j < kFightLayoutElementCount; ++j) Layout_SetElementEnabled(AsCResultFightMenu(self)->layout, j, 0);
        BpObject_StartScriptByName(AsCResultFightMenu(self)->scriptObject, fightResultStrings.introAnimation, 1);
        struct NameList { void* p[26]; };
        NameList names = *(NameList*)s_fightResultInitiallyHiddenGroupNames;
        i = 0;
        do {
            UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCResultFightMenu(self)->scriptObject, names.p[(unsigned char)i]), 0);
            ++i;
        } while (i < kFightHiddenGroupCount);
    }
    // Phase 2: the intro script gates score reveal; delta is elapsed seconds.
    if (!BpObject_IsCurrentScriptComplete(AsCResultFightMenu(self)->scriptObject)) return 0;
    if (!AsCResultFightMenu(self)->introScriptComplete) AsCResultFightMenu(self)->introScriptComplete = 1;
    // 0x8032AB2C: Join the pending return while animation or input is incomplete.
    do {
        // Phase 3: only accumulate the advance delay after score reveal completes.
        if (CResultFightMenu_UpdateRevealSequence(self, delta)) {
            float lim = kFloat_5_FightResultAdvanceDelay;
            float t = AsCResultFightMenu(self)->elapsedTime;
            t += delta;
            AsCResultFightMenu(self)->elapsedTime = t;
            if (!(t >= lim)) {
                if (!Sing_805DC5C8()) {
                    if (!s_inputManagerInitialized) {
                        InputManager_Init(&s_gameInputManager);
                        __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration_FightResults);
                        s_inputManagerInitialized = 1;
                    }
                    Sing_805DC5C8() = &s_gameInputManager;
                }
                if (!InputManager_IsAnyMenuAdvanceActionActive(Sing_805DC5C8())) break;
            }
            Sound_PlayByName(g_soundManager, fightResultStrings.confirmSound, 0, 0, 0, 0, kFloat_0_5_FightResultConfirmSound, kFloat_200_FightResultConfirmSound, kFloat_1_ResultFightMenu);
            return 1;
        }
    } while (0);

    return 0;
}

}
namespace app { template <class T> struct app_allocator {}; }
namespace std {
template <class T, class A> struct LocalVector_u803297B8 {
    void* p;
    unsigned int a;
    int b;
    // Release the activation-ID vector allocation; integer elements need no cleanup.
    ~LocalVector_u803297B8() { if (p) { a -= ((char*)p + a) - (char*)p; appFreeUni__FPv(p); } }
};
}
extern "C" {

#pragma push
// 0x803297B8 unit: poll the alternate cancel action with lazy input setup.
static inline int ViewRank_PollAlternateCancel() {
    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration_Ranking);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = &s_gameInputManager;
    }
    return Input_IsActionPressed(Sing_805DC5C8(), kAlternateCancelAction, kAnyInputPlayer);
}

// Poll confirmation/cancel; return completion, with selectionResult 1=confirm, 2=cancel.
// Confirm waits four update calls; cancellation completes in the current call.
int ViewRank_UpdateConfirmOrCancel(void* arg) {
    Layout_Update(AsViewRank(arg)->layout);
    // After confirmation, finish when the four-call countdown reaches zero.
    int old = AsViewRank(arg)->activationDelay;
    int cnt = old - 1;
    AsViewRank(arg)->activationDelay = cnt;
    if (old > 0) {
        return (cnt == 0) ? 1 : 0;
    }
std::LocalVector_u803297B8<int, app::app_allocator<int> > locals = {0, 0, 0};
    // Element 0 is confirmation; flags 5 and visual state 2 record its pressed state.
    int activated = Layout_CollectActivated(AsViewRank(arg)->layout, (void*)&locals, 0);
    if (activated) {
        Layout_SetElementPlayerFlag(AsViewRank(arg)->layout, kRankingConfirmElement, 0, kRankingConfirmPlayerFlags);
        Layout_SetElementValueState(AsViewRank(arg)->layout, kRankingConfirmElement, kRankingConfirmVisualState);
        Sound_PlayByName(g_soundManager, (char*)&rankingViewStrings.confirmSound, 0, 0, 0, 0, kFloat_0_5_RankingConfirmSound, kFloat_200_RankingConfirmSound, kFloat_1_Ranking);
        AsViewRank(arg)->activationDelay = kRankingActivationFrames;
        AsViewRank(arg)->selectionResult = 1;
        return 0;
    }
    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration_Ranking);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = &s_gameInputManager;
    }
    int pressed = Input_IsActionPressed(Sing_805DC5C8(), kCancelAction, kAnyInputPlayer);
    if (pressed || ViewRank_PollAlternateCancel()) {
        AsViewRank(arg)->selectionResult = 2;
        return 1;
    }
    return 0;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Fill each ranked row with score relative to par and Pang through the current hole, highlighting the player.
// Required initialized arg, sharedDoc/itemInventory/golfDoc, rival/strokeUse,
// and loaded ranking BP entry/numberDraw. No pointer/lookup failure is checked.
// rankRowByCharacter has eight entries indexed by character [0,7]; every value
// must be a valid [0,7] row in ALL four score/Pang group/number-name tables.
// Normally it is a permutation: duplicates cause later characters to overwrite
// the same row. The selected character must resolve to [0,7]; otherwise no row
// receives the player branch/tint. GolfPlayer entry 0 supplies the player's own
// cumulative scoreToPar and totalPang; all other characters use rival history.
// currentHoleIndex is zero-based; holeRecords must have at least index+1 records
// of 0x20 bytes with unsigned par at +0x18. Rival history must accept that same
// inclusive hole count (index+1) for all eight characters. This routine imposes
// no additional course-length bound and does not clamp an invalid hole index.
// Writes UI score integers (strokes minus cumulative par), Pang totals (currency
// units) and tint, and clears numberEntriesDirty. No score/history is modified.
// References are borrowed for this call; UI and document owners must remain live.
void ViewRank_RefreshPlayerScores(void* arg) {
    int playerCharacterIndex;
    void* rankCursor;
    void* rankingScript;
    void* rivalScores;
    int parThroughCurrentHole;
    int characterIndex;
    int scoreToPar;
    int pang;
    void* scoreGroup;
    void* pangGroup;
    EnsureResultBpRegistry(&s_bpCtrlDtorRecord_Menu);
    rankingScript = IdMap_Lookup(Sing_805DC5F0(), (*(void**)&AsViewRank(arg)->entryId));
    playerCharacterIndex = ItemInventory_FindCharacterIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, ((RankingSharedSelection*)g_sharedDoc)->characterTypeId);
    rivalScores = ((CRival*)g_rival)->strokeUse;
    void* golfDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    parThroughCurrentHole = 0;
    int holeIndex = 0;
    // Phase 1: accumulate par through the current hole (inclusive).
    while (holeIndex <= (*(volatile unsigned char*)&AsCGolfDoc(golfDoc)->currentHoleIndex)) {
        void* hole = AsCGolfDoc(golfDoc)->holeRecords;
        hole = (char*)hole + ((holeIndex & 0xFF) << 5);
        parThroughCurrentHole += ((RankingHoleRecord*)hole)->par;
        ++holeIndex;
    }
    if (AsViewRank(arg)->numberEntriesDirty != 0) {
        AsViewRank(arg)->numberEntriesDirty = 0;
    }
    rankCursor = arg;
    characterIndex = 0;
    // Phase 2: map each character to its row, set colors, then write score/Pang numbers.
    while (characterIndex < kCharacterCount) {
        int idx = AsViewRank(rankCursor)->rankRowByCharacter[0];
        scoreGroup = BpObject_GetOrCreateNamedGroup(rankingScript, ((void**)s_rankingScoreGroupNames)[idx]);
        idx = AsViewRank(rankCursor)->rankRowByCharacter[0];
        pangGroup = BpObject_GetOrCreateNamedGroup(rankingScript, ((void**)s_rankingPangGroupNames)[idx]);
        if (characterIndex == playerCharacterIndex) {
            void* info = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), 0);
            scoreToPar = ((RankingPlayerResult*)info)->scoreToPar;
            info = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), 0);
            pang = ((RankingPlayerResult*)info)->totalPang;
            UIGroup_SetTintColor(scoreGroup, kPlayerNumberTint);
            UIGroup_SetTintColor(pangGroup, kPlayerNumberTint);
        } else {
            scoreToPar = RivalStrokeUse_GetStrokesThroughHole(rivalScores, characterIndex, AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex + 1) - parThroughCurrentHole;
            pang = RivalStrokeUse_GetPangThroughHole(rivalScores, characterIndex, AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex + 1);
            UIGroup_SetTintColor(scoreGroup, kOtherNumberTint);
            UIGroup_SetTintColor(pangGroup, kOtherNumberTint);
        }
        idx = AsViewRank(rankCursor)->rankRowByCharacter[0];
        UITextTable_SetInteger(AsViewRank(arg)->numberDraw, ((void**)s_rankingScoreNumberEntryNames)[idx], scoreToPar);
        idx = AsViewRank(rankCursor)->rankRowByCharacter[0];
        UITextTable_SetInteger(AsViewRank(arg)->numberDraw, ((void**)s_rankingPangNumberEntryNames)[idx], pang);
        ++characterIndex;
        rankCursor = (char*)rankCursor + 4;
    }
}
#pragma pop

// Initialize this ranking animation angle constant to one full turn.
void ViewRank_InitFullTurnAngle() {
    s_rankingViewTwoPi = kFloat_6_2831855;
}

// Initialize this result animation angle constant to one full turn.
void FightResult_InitFullTurnAngle0() {
    s_fightResultTwoPi0 = kFloat_6_2831855;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046262C", _ctors_u803297B8_0, ViewRank_InitFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU8046262C", _ctors_u803297B8_1, FightResult_InitFullTurnAngle0);
STATIC_INIT_ENTRY(".CRT$XCU8046262C", _ctors_u803297B8_2, FightResult_InitFullTurnAngle1);
#pragma pop
