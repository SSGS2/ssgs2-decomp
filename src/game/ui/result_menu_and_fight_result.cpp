#include <decomp/static_init.h>
// Stroke / Match result screens: scripted reveal, Pang totals, rank-progress
// animation, optional placing texture, and dismissal input. Separate Fight/Match
// views below describe their own objects, not aliases of the common result menu.
// Mod lookup (scalar storage: u8032AE10_data.cpp; ABI layout: CResultMenu.h):
// - Frame rate: CResultMenu_UpdateUntilDismissed counts kAutomaticDismissUpdateCalls = 150
//   AFTER UpdateProgressAndTotals reports complete. Baseline 30 calls/s would
//   give 5 seconds; doubling calls halves that delay unless handled separately.
// - Reward animation: UpdateProgressAndTotals adds ONE progress unit per eligible
//   CALL after 6 seconds, pausing for the rank-up BP script. Its shared 1.0 scalar
//   also means full opacity, a reveal second, and a sound argument: editing the
//   data word is not an isolated progress-speed mod.
// - Timeline seconds: earned Pang 1, bonus 2, double 3, labels 4, total 5,
//   progress starts 6, final total after 7 AND progressComplete. Absent double
//   Pang/mode-2 progress skip their respective seconds. Long deltas can skip
//   earlier reveal branches: this ladder does not queue all crossed events.
// - Match/Fight reveals: CResultFightMenu_AdvanceRowRevealStages has FOUR equal 1-second
//   thresholds (not delays 1/2/3/4); UpdateRevealSequence resets each score clock.
// - Rank endpoints: constructor's 11 cumulative kRankProgressEnd* values, in
//   progress units; preserve increasing nonzero interval lengths and element IDs
//   33..43. Mask X = 10*(1-fraction), Y=1, not viewport pixel coordinates.
// - Texture/layout: DrawLayoutTextureQuad keeps a one-texel inset and scales UVs
//   by layout size / descriptor dimensions. No framebuffer/aspect/format setting
//   here; coordinate a widescreen layout edit with the UI projection elsewhere.
// Constructor owns layout, BP entries, result texture and text; textFactory and
// singletons are borrowed. dtor unregisters before destroying handles. Required
// resources must pass readiness before setup/draw/update. Preserve event order,
// overlays, string cleanup, and the extra completion-observation call.
#include "classes/CMsgDraw.h"
#include "classes/UITextTable.h"
#include "classes/CGolfDoc.h"
#include "classes/CSharedDoc.h"
#include "classes/CResultMenu.h"
#define AsCResultMenu(p) ((class_layout::CResultMenu*)(p))
#include "sdk_protos.h"
#include "game/types.h"
#include "game/singletons.h"

enum ResultMenuConstants {
    kStrokeResultMode = 0,
    kMatchResultMode = 1,
    kSkipProgressResultMode = 2,
    kResultRevealInitial = 0,
    kResultRevealStages = 1,
    kResultRevealScore = 2,
    kMatchRevealStageCount = 4,
    kMatchRevealThresholdSeconds = 1,
    kResultLayoutElementCount = 0x38,
    kAutomaticDismissUpdateCalls = 150,
    kLastProgressSegment = 10,
    kProgressSegmentElementBase = 0x21,
    kTexturePreviewElement = 0x2C,
    kResultMessageElement = 0x2D,
    kTotalProgressTextEntry = 0x32,
    kTotalPangTextEntry = 0x33,
    kBonusPangTextEntry = 0x34,
    kEarnedPangTextEntry = 0x35,
    kResultVertexFormat = 0x11F005,
    kTriangleStrip = 0x98,
    kResultVertexBytes = 0x18,
    kLoseResultElement = 0x0F, kWinResultElement = 0x10,
    kMatchMessageIndexBase = 0x20
};
// Cumulative progress units, not seconds, Pang, or verified player-facing ranks.
enum ResultRankProgressEndpoints {
    kRankProgressEnd0 = 200, kRankProgressEnd1 = 550,
    kRankProgressEnd2 = 1050, kRankProgressEnd3 = 1800,
    kRankProgressEnd4 = 2800, kRankProgressEnd5 = 4050,
    kRankProgressEnd6 = 5650, kRankProgressEnd7 = 7650,
    kRankProgressEnd8 = 10150, kRankProgressEnd9 = 13150,
    kRankProgressEnd10 = 16650
};
enum ResultTotalElements {
    kResultLabelElement0 = 0, kResultLabelElement1 = 1, kResultLabelElement2 = 2,
    kNormalProgressElementA = 3, kCommonProgressElement = 4,
    kDoublePangElement = 5, kNormalProgressElementB = 6,
    kTotalPangElement = 7, kBonusPangElement = 8, kEarnedPangElement = 9,
    kTotalPangPartnerElement = 11, kBonusPangPartnerElement = 12, kEarnedPangPartnerElement = 13,
    kSpecialProgressElementA = 0x33, kSpecialProgressElementB = 0x34
};
// Main result layout map (elements and numeric-text entries are separate namespaces):
// 0/1/2 labels; 3/6 normal progress, 4 shared progress; 5 double-Pang stage;
// 7/11 total Pang, 8/12 bonus Pang, 9/13 earned Pang; 15 lose, 16 win;
// 33..43 rank segments; 44 texture preview; 45 message bounds; 51/52 special progress.
// Text entries 50/51/52/53 carry total progress/total Pang/bonus Pang/earned Pang.
// Timeline seconds: 1 earned, 2 bonus, 3 double, 4 labels, 5 total, 6 progress, 7 final.
// Progress itself advances one unit per update (not deltaSeconds), pausing for rank-up scripts.

// Result-menu implementation, 0x8032AE10-0x8032D0FC.
extern "C" {
void appFreeUni__FPv(void*);

// Narrow and wide strings share the heap flag and pointer words.
struct StringUnwindCleanup_u8032AE10 {
    const unsigned int* words;
    bool active;
    // Guard a string's heap allocation until explicit normal-path release.
    explicit StringUnwindCleanup_u8032AE10(const void* string)
        : words((const unsigned int*)string), active(true) {}
    // Disarm cleanup when the caller frees or transfers the buffer itself.
    void release() { active = false; }
    // Release the heap-backed string during unwinding, unless disarmed.
    ~StringUnwindCleanup_u8032AE10() {
        if (active && (words[0] & 0x80000000u)) {
            appFreeUni__FPv((void*)words[2]);
        }
    }
};


struct ResultMenuConfigView;
void InputManager_Init(void*);
void InputManager_Dtor(void*);
int InputManager_IsAnyMenuAdvanceActionActive(void*);
void SoundManager_SetStreamVolume(void*, float, float);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
void CResultMenu_SelectStateScript(ResultMenuConfigView*);
void CResultMenu_UpdateTaggedResultText(void*);
int CResultMenu_UpdateProgressAndTotals(void*, float);
extern const float kFloat_0_ResultMenu; // 0.0: result-menu zero
extern const float kFloat_0_6_ResultMenu;
extern const float kFloat_0_5_ResultScreen;
extern const float kFloat_200_ResultMenu;
extern const float kFloat_1_Menu8032AE10;
struct ResultScreenStringPool {
    char layoutFile[14];
    char resultBpFile[14];
    char rankUpBpFile[18];
    char numberFont[18];
    char defaultScript[8];
    char trophySound[22];
    char clickSound[14];
    char winScript[11];
    char loseScript[12];
    char messageNumberFormat[5];
    char captionGroup[13];
    char rankUpGroup[17];
    char experienceSound[14];
    char levelUpSound[9];
    char rankUpScript[14];
    char progressMaskGroup[14];
};
extern char resultScreenStrings[];
extern char s_gameInputManager[];
extern int s_resultPlacingDecorationElementIds[];
extern int s_resultPlacingLabelElementIds[];
extern void* g_soundManager;
extern signed char s_inputManagerInitialized;
void Layout_Update(void*);
int Layout_PollLoadComplete(void*);
void CBpCtrl_dtor(void*);
void PairVector_InitReserved(void*);
void MessageDrawPairList_Dtor(void*);
void PointerPairList_RemoveByKey(void*, void*);
void UITextTable_ClearEntries(void*);
void PointerVector_InitReserved(void*);
void PointerVector_DestroyStorage(void*);
void PointerArray_DeleteValue(void*, void*);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_DeleteEntryById(void*, int);
void Layout_Destroy(void*, int);
void __dl__FPv(void*);
extern signed char s_pointerPairListInitialized;
extern signed char s_pointerVectorInitialized;
extern signed char s_bpCtrlInitialized;
extern char s_messageDrawPairList[];
extern char s_fontNumDrawRegistry[];
extern char __vt__11CResultMenu[];
extern char s_inputManagerDtorRegistration_ResultMenu[];
int CBpCtrl_PollAllEntriesLoaded(void*);
void MPBase1_Ctor(void*);
void* __nw__FUl(unsigned int);
void Layout_Init(void*, int);
void Layout_LoadExtended(void*, int, const char*);
void* IdMap_Lookup(void*, int);
void CBpCtrl_CreateEntry(void*, const char*, void*, int);
void* CFontNumDraw_CreateInVector(void*);
void WFixedFont_CreateMsgEcbResource(void*, void*, int, int);
void WFixedFont_CreateAndRegisterMsgDraw(void*, void*, int);
struct ChatMessageFactoryView { char managerStorage[0x10]; void* textFactory; };
extern ChatMessageFactoryView* g_chatMsg;
extern char s_fontNumRegistryDtorRegistration_ResultMenu[];
extern float kFloat_6_2831855;
extern float s_resultFightMenuTau;
extern char s_bpCtrl[0x100];
extern char s_bpCtrlDtorRegistration_ResultMenu[0x100];
extern float kFloat_0_FightResultReveal; // 0.0: fight-result timer reset
extern float kFloat_1_ResultFightMenu;
extern double kDouble_SignedConversionBias_ResultMenu;
extern double kDouble_UnsignedConversionBias_ResultFight;
extern double kDouble_SignedConversionBias_ResultFight;
extern CSharedDoc* g_sharedDoc;
extern char s_resultNumberEntryDescriptors[];
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
int SharedDoc_IsPlayerFlag0Set(int);
void InputManager_SetSelectedPlayer(void*, int);
void CFontNumDraw_RegisterEntries(void*, const char*, int, void*, void*);
void BpObject_SetEnabled(void*, int);
void BpObject_StartScriptByName(void*, const char*, int);
float SoundManager_GetVolumeByMode(void*, int);
int CGolfDoc_SelectPlayerBySnapshotWords(void*);
extern int s_fightResultTextureElementIds[4];
extern int s_fightResultTextureQuadTemplate[24];
extern int s_resultPreviewQuadTemplate[];
extern int s_fightResultScoreElementIds[];
void ResultMenu_DrawSharedQuad(void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
int BpObject_IsCurrentScriptComplete(void*);
void Layout_DrawWithOpacity(void*, float);
void Layout_ReadElementRect(void*, int, void*);
void Engine_SetFlexibleVertexFormat(int);
void DrawPrimitiveUP(int, int, void*, int);
void UITextTable_Draw(void*);
void Layout_SetElementEnabled(void*, int, int);
void Layout_SetElementColor(void*, int, int);
void UITextTable_SetInteger(void*, int, int);
void* Layout_GetElementEnabled(void*, int, int*);
void Layout_GetElementSize(void*, int, void*);
int BpSprite_GetTextureDescriptor(void*, void*);
extern float s_resultMenuTau;
extern unsigned int s_skipProgressResultScriptNames[4];
extern unsigned int s_strokeResultPlacingScriptNames[4];
struct ResultMessageTagPool {
    unsigned short centeredAlignment[13];
    unsigned short messagePrefix[17];
    unsigned short strokePrefix[12];
    unsigned short matchPrefix[11];
    unsigned short tagEnd[5];
};
extern char s_resultMessageTagFragments[];
extern const unsigned int s_specialResultHiddenGroupName0;
extern const unsigned int s_specialResultHiddenGroupName1;
extern const unsigned int kFloatBits_0_ResultProgressScale;
extern const unsigned int kFloatBits_1_ResultProgressScale;
extern const float kFloat_7_ResultMenu;
extern const float kFloat_6_ResultProgressStartSeconds; // 6.0: start progress animation
extern const float kFloat_3_ResultMenu;
extern const float kFloat_10_ResultProgressMaskScale; // 10.0: progress-mask scale range
extern const float kFloat_5_ResultTotalPangRevealSeconds; // 5.0: reveal total Pang
extern const float kFloat_4_ResultLabelRevealSeconds; // 4.0: reveal result labels
extern const float kFloat_2_ResultBonusPangRevealSeconds; // 2.0: reveal bonus Pang
extern unsigned int s_resultModePlacingTextureNames[6];
void WString_ReserveCapacity(void*, int);
void WString_ReplaceRange(void*, int, int, const char*, const char*, const char*);
void Text_ExpandTaggedText(void*, void*, void*, int);
void Layout_GetElementPosition(void*, int, void*);
void Text_SetPosition(void*, void*);
void CMsgDraw_SetLayoutSize(void*, void*);
unsigned int UIGroup_GetTintColor(void*);
int UIGroup_GetCombinedOpacity(void*);
void Text_SetContent(void*, const void*);
void appFreeUni__FPv(void*);
void* BpObject_GetOrCreateNamedGroup(void*, unsigned int);
void UIGroup_SetEnabled(void*, int);
void CBpTex_Init(void*);
void UIGroup_SetOpacity(void*, int);
void Sound_StopByAlias(void*, const char*);
void UIGroup_SetScale(void*, void*);

struct ResultLayoutView { char padding_00[0x1C]; void* layout; };

/* original 0x8032AEAC-0x8032AEF0: the calls go through vtable slots 0x18 (int) and 0x14 (ours used 0x20 / 0x1C) */
struct RenderCallback { 
virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual int v4();
};

struct DtorCallback { 
virtual void f0(int);
};

// Layout contracts (all byte offsets; views borrow the same allocation):
// Common result menu: +18 mode, +1C layout, +20/+24 main BP id/object,
// +28/+2C progress BP id/object, +30 result texture, +34 numeric text table,
// +38 text factory, +3C tagged text, +40 expanded text. +44..+6C hold
// eleven progress boundaries; +70 current units, +74 initial units, +78 length,
// +7C segment, +80 initialized, +84 header shown, +88 elapsed seconds,
// +8C progress complete, +90 totals shown, +94 dismissal update calls, +98 saved volume.
// ResultMenuProcessView::resultTextureWord is the +30 texture-presence word.
// It gates audio ducking/trophy sound.
// MatchResultRevealView and FightResultRevealView are two partial views of the
// SAME CResultFightMenu (see its constructor/SetEntries in view_rank_and_fight_result_menu.cpp); neither
// overlays the common CResultMenu. Preserve their different ABI load types.
struct MatchResultRevealView {
    char padding_00[0x18];
    int resultMode; // CResultFightMenu_Ctor: 0 stroke, 1 match, 2 party
    void* layout;
    char padding_20[8];
    void* uiTextTable;
    char padding_2c[0xE4];
    int resultIndex;
    int revealedStages;
    float revealSeconds;
};

struct ResultMenuOwnedResources {
    void* vtable;
    char padding_04[0x18];
    void* layout;
    int mainBpEntryId;
    void* mainBpObject;
    int progressBpEntryId;
    char padding_2c[4];
    void* resultTexture;
    void* uiTextTable;
    char padding_38[4];
    void* taggedText;
    void* text;
};

struct Table24 { int v[24]; };

// The expanded-text pointer precedes eleven rank-interval end thresholds.
struct ResultMenuTextAndThresholds { void* text; int segmentThresholds[11]; };
struct ResultMenuProgressSetup {
    char padding_00[0x24];
    void* mainBpObject;
    char padding_28[4];
    void* progressBpObject;
    char padding_30[4];
    void* uiTextTable;
    char padding_38[8];
    union {
        ResultMenuTextAndThresholds textAndThresholds;
        int textAndThresholdWords[12]; // Raw indexing preserves the original window addressing.
    };
    char padding_70[4];
    float initialSegmentProgress;
    int segmentLength;
    int segmentIndex;
    char padding_80[0x18];
    float savedStreamVolume;
    char padding_9c[0x18];
    int remainingProgress;
};

// Used after adding segmentIndex * 4 to the menu address: both words are thresholds.
struct ResultMenuShiftedProgressWindow {
    char prefix[0x40];
    unsigned int segmentBegin, segmentEnd;
};

struct Callback { 
virtual void f0(); virtual void f1(); virtual void f2(); virtual void f3(); virtual int invoke();
};

struct CallbackOwner { char padding_00[0x30]; Callback* resultTexture; };

struct ResultMenuConstructionView {
    void* vtable;
    char baseStorage[0x14];
    int resultMode;
    void* layout;
    int mainBpEntryId;
    void* mainBpObject;
    int progressBpEntryId;
    void* progressBpObject;
    void* resultTexture;
    void* uiTextTable;
    void* textFactory;
    void* taggedText;
    void* text;
    int segmentThresholds[11];
    float currentProgress;
    float initialSegmentProgress;
    int segmentLength;
    int segmentIndex;
    int initialized;
    int headerDisplayed;
    float displayElapsedTime;
    int progressComplete;
    int totalsDisplayed;
    int completionFrames;
    float savedStreamVolume;
};

struct ResultMenuDrawView {
    char padding_00[0x1C];
    void* layout;
    char padding_20[0x4];
    void* mainBpObject;
    char padding_28[0x4];
    void* progressBpObject;
    void* resultTexture;
    void* uiTextTable;
    char padding_38[0x48];
    int initialized;
};

struct Vtx24 { float x, y; int rest[4]; };

struct Quad24 { Vtx24 v[4]; };

struct VObjQuad { 
virtual void p0(); virtual void p1(); virtual void p2(); virtual void v5(); virtual int v6();
};

struct Table15 { int v[15]; };

struct FightResultRevealView {
    char padding_00[0x18];
    void* resultMode; // same +18 word as MatchResultRevealView, scalar ABI
    void* layout;
    char padding_20[0x18];
    unsigned int resultCount;
    char padding_3c[0xD0];
    int revealState;
    unsigned int nextResult;
    int revealedStages;
    float revealSeconds;
};

struct ResultMenuProcessView {
    char padding_00[8]; int active; char padding_0c[0x0c]; int mode;
    void* layout; char padding_20[4]; void* menu; char padding_28[8];
    int resultTextureWord; // +30: resultTexture address word, not a general enable flag
    char padding_34[0x4c]; int init; int headerDisplayed; char padding_88[0x0c];
    int counter; char padding_98[4]; int selection; char padding_a0[0x2c]; int special;
};

struct ResultMenuData {
    // Producer: CGolfRule_CreateResultMenu (golf_rule_round_flow_and_shot_sim.cpp). It takes the last eligible
    // non-CPU player (or player 0 for special mode), not necessarily currentPlayer.
    // +00 placement = player+C4 high nibble; party overrides from shared+AF0.
    // +08 score = signed score-to-par player+C8, or party+1C4 (mode 2 float is
    // truncated to int). unsigned storage preserves negative score bits.
    // +0C/+10 are player+CC/+D0 earned/bonus Pang, already computed by reward code;
    // +14 is player+16E double-Pang byte, treated as Boolean, not another multiplier.
    // +18 player+164 remaining progress, +1C sign-extended short player+16C total
    // progress displayed at completion. Rank setup separately uses shared player
    // cumulative progress (+44 in each 0x1D4 record) minus remaining as its start.
    // +20/+24 round holes/current zero-based hole; +28 shared+B80 message suffix.
    // +2C special total comes from SharedStringState_GetShortValue on a special
    // win; +30 specialMode is (gameMode==6), distinct from resultMode==2 (party).
    // Score/party value, progress and Pang are independent units; do not copy a
    // Pang multiplier into score/progress. UI formats score bits as a signed int.
    // Scalar values are copied; no payload ownership transfers. placing is normally 1-based,
    // but mode 2 indexes its four scripts directly. textureIndex selects one of three images.
    unsigned int placing;
    unsigned int textureIndex;
    unsigned int score;
    unsigned int earnedPang;
    unsigned int bonusPang;
    unsigned int hasDoublePang;
    unsigned int remainingProgress;
    unsigned int totalProgress;
    unsigned int roundHoleCount;
    unsigned int currentHoleIndex;
    unsigned int messageIndex;
    unsigned short specialTotal;
    char pad2E[2];
    int specialMode; // +30 in payload: specialMode, copied to menu +CC
};

struct ResultMenuConfigView {
    char pad00[0x18];
    int resultMode;
    char pad1c[8];
    void* resource;
    char pad28[8];
    void* resultTexture;
    char pad34[0x68];
    int placing;
    int textureIndex;
    char padA4[0x28];
    int specialMode;
};

struct ResultTextureStorage {
    void* vtable;
    char pad04[0x24];
    // Initialize the result texture wrapper before a placing-specific image is selected.
    ResultTextureStorage() { CBpTex_Init(this); }
};

struct ChildInterface {
    virtual ~ChildInterface();
    virtual void initialize(const char*);
};

struct WideStringStorage {
    unsigned int flags;
    unsigned int size;
    char* data;
};
// flags bit 31 selects heap data; otherwise wide characters begin at +2. Sizes count
// 16-bit characters, while range endpoints passed to WString_ReplaceRange are byte addresses.

struct AllocatorByteLow {
    char val;
    char pad1[3];
    volatile char junk;
    char pad2[3];
};

struct AllocatorByteHigh {
    volatile char junk;
    char pad1[3];
    char val;
    char pad2[3];
};

// Draw a loaded result sprite into its enabled layout rectangle with texture-relative UVs.
// self/layout required, index [0,4). Disabled element, unloaded child or failed
// descriptor query returns without drawing. Width/height must be nonzero.
// Layout rectangle is LTRB in UI coordinates (+Y down); UVs are normalized texels.
// No ownership transfer; binding and vertex-format changes remain global.
// self is a CResultFightMenu: row[index] at +3C+12*index contains player index
// [0,4), which selects the embedded texture at +6C+0x28*playerIndex. Producer
// SetEntries copies up to four placing-ordered rows; populated indices must be
// <entryCount. Descriptor width/height are positive pixel counts. To keep UVs
// within [0,1], layout width<=textureWidth-1 and height<=textureHeight-1; no
// clamping is done. Rectangle L<=R,T<=B and dimensions must describe that same
// layout element; this is a caller/resource invariant, not a validated fallback.
void CResultFightMenu_DrawLayoutTextureQuad(ResultLayoutView* self, int index) {
    // index must be 0..3. Texture descriptor words 0/1 supply pixel width/height.
    unsigned int rect[4];
    int* dp = s_fightResultTextureElementIds;
    int dims[4];
    dims[0] = *dp++;
    int state = 0;
    dims[1] = *dp++;
    dims[2] = *dp++;
    dims[3] = *dp++;
    int viewport[4];
    float scale[4];
    float one[4];
    int id = dims[index];
    Layout_GetElementEnabled(self->layout, id, &state);
    if (state == 0) return;

    char* item = (char*)self + index * 0x0C;
    int childIndex = *(int*)(item + 0x3C);
    RenderCallback* cb = (RenderCallback*)((char*)self + childIndex * 0x28 + 0x6C);
    if (cb == 0 || cb->v4() == 0) return;
    if (BpSprite_GetTextureDescriptor(cb, rect) == 0) return;
    cb->v3();

    Layout_ReadElementRect(self->layout, id, viewport);
    one[1] = kFloat_1_ResultFightMenu;
    one[0] = kFloat_1_ResultFightMenu;
    Layout_GetElementSize(self->layout, id, scale);

    // UVs begin one texel inside the image; the sampled span is the layout size / texture size.
    float x0 = one[0] / (float)rect[0];
    float y0 = one[1] / (float)rect[1];
    float sx = scale[0] / (float)rect[0];
    float sy = scale[1] / (float)rect[1];

    float verts[24];
    *(Table24*)verts = *(Table24*)s_fightResultTextureQuadTemplate;
    float x1 = x0 + sx;
    float y1 = y0 + sy;
    verts[0] = (float)viewport[0];
    verts[1] = (float)viewport[1];
    verts[4] = x0;
    verts[5] = y0;
    verts[6] = (float)viewport[2];
    verts[7] = (float)viewport[1];
    verts[10] = x1;
    verts[11] = y0;
    verts[12] = (float)viewport[0];
    verts[13] = (float)viewport[3];
    verts[16] = x0;
    verts[17] = y1;
    verts[18] = (float)viewport[2];
    verts[19] = (float)viewport[3];
    verts[22] = x1;
    verts[23] = y1;
    Engine_SetFlexibleVertexFormat(kResultVertexFormat);
    DrawPrimitiveUP(kTriangleStrip, 2, verts, kResultVertexBytes);
}

#pragma opt_common_subs off
// CResultFightMenu_SetEntries (view_rank_and_fight_result_menu.cpp) copies rows {playerIndex,score,totalPang}
// at +3C; CGolfRule_CreateFightResultOrRetryMenu (golf_rule_round_flow_and_shot_sim.cpp) orders them by place.
// This shifted view starts +34 so one-based resultIndex reads score at +40 and
// Pang at +44 for row 0. Index 0 is decoration only, never a numeric record.
// roundScore is signed player+C8 score-to-par, replaced by party+1C4 for party
// modes 1/3, or truncated float +1C4 for mode 2. totalPang = player+CC + player+D0
// (base+bonus), with no additional multiplier here. The third shifted word is
// the NEXT row's player index (or texture prefix after row 3); never read it here.
struct MatchRevealValueRecord { int roundScore, totalPang, unusedNextWord; };
struct MatchRevealPayloadView { char prefix[0x34]; MatchRevealValueRecord values[5]; };

// Reveal each newly reached match-result stage and publish its numeric values.
// arg/layout/textTable required; resultIndex [0,5), resultMode [0,3),
// revealedStages [0,4]. Mutates enabled elements/numbers and revealedStages only;
// clock advancement belongs to UpdateRevealSequence. No allocation or ownership.
int CResultFightMenu_AdvanceRowRevealStages(void* arg) {
    // All four thresholds are one second; a delayed call may reveal several stages together.
    // Reaching the final stage returns 0 on this call, then 1 on the next call.
    unsigned int revealedStageCount = ((MatchResultRevealView*)arg)->revealedStages;
    MatchResultRevealView* self = (MatchResultRevealView*)arg;
    float thresholds[4] = {kMatchRevealThresholdSeconds, kMatchRevealThresholdSeconds,
                           kMatchRevealThresholdSeconds, kMatchRevealThresholdSeconds};
    if (revealedStageCount == kMatchRevealStageCount) return 1;
    unsigned int revealableStageCount;
    float revealSeconds = self->revealSeconds;
    for (revealableStageCount = 0; revealableStageCount < kMatchRevealStageCount; ++revealableStageCount) {
        if (revealSeconds < thresholds[revealableStageCount]) break;
    }
    if (revealedStageCount == revealableStageCount) return 0;
    for (unsigned int i = revealedStageCount; i < revealableStageCount; ++i) {
        switch ((int)i) {
        // Decoration first, then its label; resultIndex zero uses dedicated elements 1..3.
        case 0: {
            int resultIndex = self->resultIndex;
            if (resultIndex == 0) Layout_SetElementEnabled(self->layout, 1, 1);
            else {
                int decorationElementByResult[4] = {4, 5, 6, 7};
                Layout_SetElementEnabled(self->layout, decorationElementByResult[resultIndex - 1], 1);
            }
            break;
        }
        case 1: {
            int resultIndex = self->resultIndex;
            if (resultIndex != 0) {
                int labelElementByResult[4] = {8, 9, 10, 11};
                Layout_SetElementEnabled(self->layout, labelElementByResult[resultIndex - 1], 1);
                Layout_SetElementColor(self->layout, labelElementByResult[self->resultIndex - 1], 0);
            }
            break;
        }
        // Score text style is MODE-dependent: stroke, match, party groups of four.
        case 2: {
            int resultIndex = self->resultIndex;
            if (resultIndex == 0) Layout_SetElementEnabled(self->layout, 2, 1);
            else {
                int scoreTextByModeAndResult[12] = {30, 31, 32, 33, 38, 39, 40, 41, 26, 27, 28, 29};
                UITextTable_SetInteger(self->uiTextTable, (scoreTextByModeAndResult + self->resultMode * 4)[resultIndex - 1],
                            ((MatchRevealPayloadView*)self)->values[resultIndex].roundScore);
            }
            break;
        }
        // Total Pang uses one shared group of four text-table entries.
        case 3: {
            int resultIndex = self->resultIndex;
            if (resultIndex == 0) Layout_SetElementEnabled(self->layout, 3, 1);
            else {
                int pangTextByResult[4] = {34, 35, 36, 37};
                UITextTable_SetInteger(self->uiTextTable, pangTextByResult[resultIndex - 1],
                            ((MatchRevealPayloadView*)self)->values[resultIndex].totalPang);
            }
            break;
        }
        }
    }
    self->revealedStages = revealableStageCount;
    return 0;
}
#pragma opt_common_subs reset

// Reveal the first fight-result element once its one-second delay expires.
inline int ResultMenu_RevealFirst(FightResultRevealView* self) {
    if (self->revealSeconds < kFloat_1_ResultFightMenu) return 0;
    Layout_SetElementEnabled(self->layout, 0, 1);
    return 1;
}

// Enable the three layout elements for the next completed fight result.
// nextResult [0,5), required self/layout; the 15-int table is five triples.
// Negative timer leaves state unchanged; success increments nextResult once.
inline int ResultMenu_RevealScore(FightResultRevealView* self) {
    if (self->revealSeconds < kFloat_0_FightResultReveal) return 0;
    int* p = s_fightResultScoreElementIds;
    Table15 st = *(Table15*)p;
    int off;
    unsigned int i;
    i = 0;
    off = 0;
    for (; i < 3; i++) {
        unsigned int idx = self->nextResult;
        Layout_SetElementEnabled(self->layout, *(int*)((unsigned int)&st + (off + (int)(idx * 12))), 1);
        off += 4;
    }
    self->nextResult = self->nextResult + 1;
    return 1;
}

// Advance the initial delay, staged reveal, and score reveal; return true after the last result.
// self required; dt seconds, nonnegative by convention. resultCount <= 4 for
// the five triples; nextResult/resultIndex follow the domains above. Completion
// uses resultCount < nextResult, so equality still processes that final score.
// Return 1 only on the later call observing completion. State transitions can
// fall through within a call; do not replace them with independent frame delays.
int CResultFightMenu_UpdateRevealSequence(FightResultRevealView* self, float dt) {
    if (self->resultCount < self->nextResult) return 1;
    switch (self->revealState) {
    case kResultRevealInitial:
        if (ResultMenu_RevealFirst(self) == 0) break;
        {
            float zero = kFloat_0_FightResultReveal;
            self->revealState = kResultRevealStages;
            self->revealSeconds = zero;
        }
        // Fall through to advance the first stage immediately.
    case kResultRevealStages:
        if (CResultFightMenu_AdvanceRowRevealStages(self) == 0) break;
        {
            float zero = kFloat_0_FightResultReveal;
            self->revealState = kResultRevealScore;
            self->revealSeconds = zero;
        }
        // Fall through to reveal the completed score.
    case kResultRevealScore:
        if (ResultMenu_RevealScore(self) == 0) break;
        {
            float zero = kFloat_0_FightResultReveal;
            self->revealState = kResultRevealStages;
            self->revealSeconds = zero;
            self->revealedStages = 0;
        }
        break;
    }
    self->revealSeconds += dt;
    return 0;
}

// Initialize this module's stored two-pi constant from the shared scalar pool.
void ResultFightMenu_InitTauConstant() { s_resultFightMenuTau = kFloat_6_2831855; }

} // C linkage
struct CWarningLogoBase {
    char storage[0x18];
    // Initialize the shared menu base storage.
    CWarningLogoBase() { MPBase1_Ctor(this); }
    ~CWarningLogoBase();
};
struct CResultMenu : CWarningLogoBase {
    char rest[0x84];
    CResultMenu(void*);
};
// Play a named result-screen sound, borrowing name for this synchronous request.
// Sound_PlayByName -> SoundManager_CreateSoundInstance (sound_manager.cpp) takes three
// floats in FPRs independently of its four GPR optional arguments. Here the
// latter are {position=null,type=0,image/alias=null,distanceScale=null}:
// .5 = minimumGain (dimensionless attenuation floor), 200 = attenuationRange
// (same distance units as the position vector), 1 = volumeScale (unity gain).
// The floor/range are used only for type==5; with this type 0/null position
// they have NO attenuation effect. volumeScale still multiplies base and manager
// gains, so raising/lowering it changes output level, not pitch. No conversion
// to physical meters or safe gain range is established here. Preserve float/GPR
// argument ordering in this legacy declaration; changing its signature needs
// coordinated ABI work. The manager owns/reuses the resulting sound instance.
static inline void PlayResultSound(const char* name) {
    Sound_PlayByName((void*)g_soundManager, name, 0, 0, 0, 0, kFloat_0_5_ResultScreen, kFloat_200_ResultMenu, kFloat_1_Menu8032AE10);
}

// Lazy singleton: construct the BP controller (CBpCtrl) on first use, register its destructor and publish it
// through Sing_805DC5F0.
static inline void EnsureBpCtrl() {
    if (((int&)Sing_805DC5F0()) == 0) {
        if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl); __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_ResultMenu); s_bpCtrlInitialized=1; }
        ((int&)Sing_805DC5F0())=(int)s_bpCtrl;
    }
}
// Create owning layout/BP/text resources; arg carries the result-mode integer through void*.
CResultMenu::CResultMenu(void* arg) {
    // Phase 1: reset ownership handles and animation state before creating resources.
    ResultMenuConstructionView* self = (ResultMenuConstructionView*)this;
    float zero = kFloat_0_ResultMenu;
    self->resultMode = (int)arg;
    self->vtable = __vt__11CResultMenu;
    // Owned resource handles start empty (-1 = no BP entry yet).
    self->layout = 0;
    self->mainBpEntryId = -1;
    self->mainBpObject = 0;
    self->progressBpEntryId = -1;
    self->progressBpObject = 0;
    self->resultTexture = 0;
    self->uiTextTable = 0;
    self->textFactory = 0;
    self->taggedText = 0;
    self->text = 0;
    // Progress / display state starts at zero.
    self->currentProgress = zero;
    self->initialSegmentProgress = zero;
    self->segmentLength = 0;
    self->segmentIndex = 0;
    self->initialized = 0;
    self->headerDisplayed = 0;
    self->displayElapsedTime = zero;
    self->progressComplete = 0;
    self->totalsDisplayed = 0;
    self->completionFrames = 0;
    self->savedStreamVolume = zero;
    struct MenuLayout {
        char storage[0x5c];
        // Initialize the owning layout object with the requested layout parameter.
        MenuLayout(int count) { Layout_Init(this, count); }
    };
    MenuLayout* p = new MenuLayout(5);
    self->layout = p;
    Layout_LoadExtended(p, 0, ((ResultScreenStringPool*)resultScreenStrings)->layoutFile);
    EnsureBpCtrl();
    CBpCtrl_CreateEntry((void*)((int&)Sing_805DC5F0()), ((ResultScreenStringPool*)resultScreenStrings)->resultBpFile, &self->mainBpEntryId, 0);
    EnsureBpCtrl();
    self->mainBpObject = IdMap_Lookup((void*)((int&)Sing_805DC5F0()),self->mainBpEntryId);
    EnsureBpCtrl();
    CBpCtrl_CreateEntry((void*)((int&)Sing_805DC5F0()),((ResultScreenStringPool*)resultScreenStrings)->rankUpBpFile,&self->progressBpEntryId,0);
    EnsureBpCtrl();
    self->progressBpObject = IdMap_Lookup((void*)((int&)Sing_805DC5F0()),self->progressBpEntryId);
    // Lazy singleton: construct the font-number draw registry on first use and publish it through Sing_805DC830.
    if (!((int&)Sing_805DC830())) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,s_fontNumRegistryDtorRegistration_ResultMenu);
            s_pointerVectorInitialized=1;
        }
        ((int&)Sing_805DC830())=(int)s_fontNumDrawRegistry;
    }
    // Phase 2: register numeric/text renderers in shared registries; retain their handles.
    self->uiTextTable=CFontNumDraw_CreateInVector((void*)((int&)Sing_805DC830()));
    self->textFactory=g_chatMsg->textFactory;
    WFixedFont_CreateMsgEcbResource(self->textFactory,&self->taggedText,0,0);
    WFixedFont_CreateAndRegisterMsgDraw(self->textFactory,&self->text,1);
    // Phase 3: cumulative rank endpoints: 200,550,1050,1800,2800,4050,5650,7650,10150,13150,16650.
    // Cumulative endpoints of the 11 rank-progress segments.
    self->segmentThresholds[0] = kRankProgressEnd0;
    self->segmentThresholds[1] = kRankProgressEnd1;
    self->segmentThresholds[2] = kRankProgressEnd2;
    self->segmentThresholds[3] = kRankProgressEnd3;
    self->segmentThresholds[4] = kRankProgressEnd4;
    self->segmentThresholds[5] = kRankProgressEnd5;
    self->segmentThresholds[6] = kRankProgressEnd6;
    self->segmentThresholds[7] = kRankProgressEnd7;
    self->segmentThresholds[8] = kRankProgressEnd8;
    self->segmentThresholds[9] = kRankProgressEnd9;
    self->segmentThresholds[10] = kRankProgressEnd10;
}
extern "C" {

// Release owned textures/text, unregister numeric entries, and remove both BP resources.
// Null accepted; shouldDelete > 0 also frees self, so return is then dangling.
// Unregister text/numeric entries before deleting referenced objects; BP object
// handles are released via entry IDs, not independently through their pointers.
ResultMenuOwnedResources* CResultMenu_dtor(ResultMenuOwnedResources* self, int shouldDelete) {
    char* g = s_inputManagerDtorRegistration_ResultMenu;
    if (self != 0) {
        self->vtable = __vt__11CResultMenu;
        if (self->resultTexture != 0) {
            if (self->resultTexture != 0) {
                ((DtorCallback*)self->resultTexture)->f0(1);
            }
            self->resultTexture = 0;
        }
        if (((int&)Sing_805DC650()) == 0) {
            if (s_pointerPairListInitialized == 0) {
                PairVector_InitReserved(s_messageDrawPairList);
                __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor, (g + 0x24));
                s_pointerPairListInitialized = 1;
            }
            ((int&)Sing_805DC650()) = (int)s_messageDrawPairList;
        }
        PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), self->text);
        if (self->text != 0) {
            if (self->text != 0) {
                ((DtorCallback*)self->text)->f0(1);
            }
            self->text = 0;
        }
        if (self->taggedText != 0) {
            if (self->taggedText != 0) {
                ((DtorCallback*)self->taggedText)->f0(1);
            }
            self->taggedText = 0;
        }
        UITextTable_ClearEntries(self->uiTextTable);
        if (self->uiTextTable != 0) {
            if (((int&)Sing_805DC830()) == 0) {
                if (s_pointerVectorInitialized == 0) {
                    PointerVector_InitReserved(s_fontNumDrawRegistry);
                    __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, (g + 0x18));
                    s_pointerVectorInitialized = 1;
                }
                ((int&)Sing_805DC830()) = (int)s_fontNumDrawRegistry;
            }
            PointerArray_DeleteValue((void*)((int&)Sing_805DC830()), self->uiTextTable);
        }
        if (self->progressBpEntryId != -1) {
            if (((int&)Sing_805DC5F0()) == 0) {
                if (s_bpCtrlInitialized == 0) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (g + 0x0C));
                    s_bpCtrlInitialized = 1;
                }
                ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), self->progressBpEntryId);
            self->progressBpEntryId = -1;
        }
        if (self->mainBpEntryId != -1) {
            if (((int&)Sing_805DC5F0()) == 0) {
                if (s_bpCtrlInitialized == 0) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (g + 0x0C));
                    s_bpCtrlInitialized = 1;
                }
                ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), self->mainBpEntryId);
            self->mainBpEntryId = -1;
        }
        if (self->layout != 0) { Layout_Destroy((void*)self->layout, 1); self->layout = 0; }
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}

// Update the result layout while resources are loading.
void CResultMenu_UpdateLayout(ResultLayoutView* self) { Layout_Update(self->layout); }

// Wait for the layout, all BP resources, and the optional result texture to become ready.
int CResultMenu_PollLoadComplete(CallbackOwner* self) {
    // Original result-menu readiness path: success follows the shared failure return.
    do {
        if (Layout_PollLoadComplete(AsCResultMenu(self)->layout) != 0) {
        EnsureBpCtrl();
        if (CBpCtrl_PollAllEntriesLoaded((void*)((int&)Sing_805DC5F0())) != 0) {
            if (self->resultTexture == 0 || self->resultTexture->invoke() != 0) break;
        }
        }
        return 0;
    } while (0);
    return 1;
}

// Choose the input owner and initialize progress relative to the selected player's previous total.
// Requires initialized payload and loaded layout/BP resources. Selects global
// input player, registers five numeric entries, saves stream volume, and picks
// interval 0..10. Award must not exceed the stored total; intervals must be
// increasing/nonzero. Snapshot player selection is clamped to [0,3].
void CResultMenu_InitRankProgress(ResultMenuProgressSetup* self) {
    // Phase 1: choose the lowest input-player value, preferring players without flag 0.
    int i;
    int selectedInputPlayer = 3;
    int candidateIndex = -1;
    for (i = 0; i < ((CGolfDoc*)((CSharedDoc*)g_sharedDoc)->golfDoc)->playerCount; i++) {
        int* entry = (int*)GolfPlayer_GetEntryByIndex((void*)((CSharedDoc*)g_sharedDoc)->golfDoc, (unsigned char)i);
        if (selectedInputPlayer > *entry && SharedDoc_IsPlayerFlag0Set(i) == 0) {
            selectedInputPlayer = *(int*)GolfPlayer_GetEntryByIndex((void*)((CSharedDoc*)g_sharedDoc)->golfDoc, (unsigned char)i);
            candidateIndex = i;
        }
    }
    if (candidateIndex < 0) {
        for (i = 0; i < ((CGolfDoc*)((CSharedDoc*)g_sharedDoc)->golfDoc)->playerCount; i++) {
            int* entry = (int*)GolfPlayer_GetEntryByIndex((void*)((CSharedDoc*)g_sharedDoc)->golfDoc, (unsigned char)i);
            if (selectedInputPlayer > *entry) selectedInputPlayer = *(int*)GolfPlayer_GetEntryByIndex((void*)((CSharedDoc*)g_sharedDoc)->golfDoc, (unsigned char)i);
        }
    }
    if (((int&)Sing_805DC5C8()) == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_ResultMenu);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)s_gameInputManager;
    }
    InputManager_SetSelectedPlayer((void*)((int&)Sing_805DC5C8()), selectedInputPlayer);
    CFontNumDraw_RegisterEntries(self->uiTextTable, s_resultNumberEntryDescriptors, 5, self->mainBpObject, ((ResultScreenStringPool*)resultScreenStrings)->numberFont);
    AsUITextTable(self->uiTextTable)->drawEnabled = 1;
    BpObject_SetEnabled(self->progressBpObject, 0);
    BpObject_StartScriptByName(self->progressBpObject, ((ResultScreenStringPool*)resultScreenStrings)->defaultScript, 1);
    self->savedStreamVolume = SoundManager_GetVolumeByMode((void*)g_soundManager, 0);
    // Phase 2: select the rank interval containing the player's total before this reward.
    int progressPlayer = CGolfDoc_SelectPlayerBySnapshotWords((void*)((CSharedDoc*)g_sharedDoc)->golfDoc);
    int clampedPlayerIndex = progressPlayer < 0 ? 0 : (progressPlayer > 3 ? 3 : progressPlayer);
    int remainingProgress = self->remainingProgress;
    int initialTotalProgress = *(int*)((char*)g_sharedDoc + clampedPlayerIndex * 0x1D4 + 0x44) - remainingProgress;
    for (int j = 0; j < 11; j++) {
        if (initialTotalProgress >= self->textAndThresholdWords[j+1] && j != 10) continue;
        if (j == 0) {
            int* q = &self->textAndThresholdWords[j];
            self->segmentLength = q[1];
            self->initialSegmentProgress = (float)initialTotalProgress;
        } else {
            int* q = &self->textAndThresholdWords[j];
            self->segmentLength = q[1] - q[0];
            self->initialSegmentProgress = (float)(initialTotalProgress - q[0]);
        }
        self->segmentIndex = j;
        break;
    }
}

#pragma use_lmw_stmw on
// Start the result scripts, reveal totals/progress, then accept advance input or the dismissal timeout.
// self required, dt seconds. Inactive/script-busy paths still update layout and
// return 0. Return 1 asks the caller to leave; click sound fires on that call.
// Counter advances only after totals were complete at entry to their updater;
// it is update calls, not elapsed dt. Normal placing must be [1,8] for the two
// eight-entry decoration tables; do not remove lower-bound caller validation.
int CResultMenu_UpdateUntilDismissed(ResultMenuProcessView* self, float dt) {
    // Phase 1: initialize visuals once, then wait for the introductory BP script to finish.
    Layout_Update(self->layout);
    if (!self->active) return 0;
    if (!self->init) {
        self->init = 1;
        for (int i = 0; i < kResultLayoutElementCount; i++) Layout_SetElementEnabled(self->layout, i, 0);
        CResultMenu_SelectStateScript((ResultMenuConfigView*)self);
        CResultMenu_UpdateTaggedResultText(self);
        if (self->resultTextureWord) SoundManager_SetStreamVolume((void*)g_soundManager, kFloat_0_ResultMenu, kFloat_0_6_ResultMenu);
    }
    if (!BpObject_IsCurrentScriptComplete(self->menu)) return 0;
    if (!self->headerDisplayed) {
        self->headerDisplayed = 1;
        if (self->mode != 2) {
            if (self->mode == 1 || self->special) {
                if (self->selection == 1) {
                    Layout_SetElementEnabled(self->layout, kWinResultElement, 1);
                    if (self->resultTextureWord)
                        PlayResultSound(((ResultScreenStringPool*)resultScreenStrings)->trophySound);
                } else Layout_SetElementEnabled(self->layout, kLoseResultElement, 1);
            } else {
                struct Eight { int v[8]; };
                Eight a = *(Eight*)s_resultPlacingDecorationElementIds;
                Eight b = *(Eight*)s_resultPlacingLabelElementIds;
                int n = self->selection - 1;
                int play = 1;
                if (n < 3) {
                    if (self->resultTextureWord) {
                        PlayResultSound(((ResultScreenStringPool*)resultScreenStrings)->trophySound);
                    }
                    play = 0;
                }
                if (play) Layout_SetElementEnabled(self->layout, a.v[n], 1);
                Layout_SetElementEnabled(self->layout, b.v[n], 1);
            }
        }
    }
    // Phase 2: after totals finish, accept advance input or dismiss after 150 update calls.
    do {
        if (CResultMenu_UpdateProgressAndTotals(self, dt)) {
            if (++self->counter < kAutomaticDismissUpdateCalls) {
                if (!((int&)Sing_805DC5C8())) {
                    if (!s_inputManagerInitialized) {
                        InputManager_Init(s_gameInputManager);
                        __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_ResultMenu);
                        s_inputManagerInitialized = 1;
                    }
                    ((int&)Sing_805DC5C8()) = (int)s_gameInputManager;
                }
                if (InputManager_IsAnyMenuAdvanceActionActive((void*)((int&)Sing_805DC5C8())) == 0) break;
            }
            PlayResultSound(((ResultScreenStringPool*)resultScreenStrings)->clickSound);
            return 1;
        }
    } while (0);
    return 0;
}
#pragma use_lmw_stmw reset

// Draw the result background, optional texture preview, numeric text, and progress animation.
// self required. Uninitialized returns without rendering. Loaded callbacks may
// alter global render state; no caller-state restore. Rectangles are UI LTRB,
// vertex ABI is XYZ/color/UV (24 bytes), triangle strip emits two primitives.
void CResultMenu_Draw(ResultMenuDrawView* self) {
    if (self->initialized == 0) return;
    ResultMenu_DrawSharedQuad(self);
    BpObject_TryInvokeCurrentCallback(self->mainBpObject, 0);
    BpObject_Draw(self->mainBpObject);
    if (BpObject_IsCurrentScriptComplete(self->mainBpObject) != 0) {
        Layout_DrawWithOpacity(self->layout, kFloat_1_Menu8032AE10);
        if (self->resultTexture != 0 && ((VObjQuad*)self->resultTexture)->v6() != 0) {
            int rect[4];
            Layout_ReadElementRect(self->layout, kTexturePreviewElement, rect);
            Quad24 q = *(Quad24*)s_resultPreviewQuadTemplate;
            q.v[0].x = (float)rect[0]; q.v[0].y = (float)rect[1];
            q.v[1].x = (float)rect[2]; q.v[1].y = (float)rect[1];
            q.v[2].x = (float)rect[0]; q.v[2].y = (float)rect[3];
            q.v[3].x = (float)rect[2]; q.v[3].y = (float)rect[3];
            Engine_SetFlexibleVertexFormat(kResultVertexFormat);
            ((VObjQuad*)self->resultTexture)->v5();
            DrawPrimitiveUP(kTriangleStrip, 2, q.v, kResultVertexBytes);
        }
        UITextTable_Draw(self->uiTextTable);
    }
    BpObject_TryInvokeCurrentCallback(self->progressBpObject, 0);
    BpObject_Draw(self->progressBpObject);
}

// Copy the result payload and create its placing-specific texture when normal-mode rules require it.
// self/config required; config remains caller-owned, scalar fields are copied.
// Texture index [0,3), resultMode [0,3); normal placing uses SelectStateScript's
// domain below. The prior owned texture is deleted before testing new mode/load.
// Full downstream domain: normal stroke placing 1..8; match placing==1 wins,
// every other admitted value loses; mode 2 placing 0..3 indexes scripts directly.
// textureIndex 0..2, remainingProgress in 0..2^24, nonnegative Pang totals/sums that fit
// signed int for UITextTable. score may be negative via its uint32 bit pattern.
// roundHoleCount 1..18, currentHoleIndex<roundHoleCount; messageIndex must name
// an existing result message (no numeric maximum is established by this lookup).
// hasDoublePang/specialMode are Boolean, not amount factors. Normal/special total
// progress are display values; remainingProgress alone controls reveal count.
// Progress precision contract (also applies to the shared-doc producer read in
// initialization): integer prior total = sharedDoc player word +44 minus reward
// must be nonnegative without signed overflow. Select increasing integer rank
// thresholds with positive interval lengths <=2^24 and an admitted initial
// interval offset 0<=initialSegmentProgress<segmentLength. Require
// initialSegmentProgress+remainingProgress<=2^24 (16777216). These sufficient
// bounds keep counted unit increments, tested offset sums and count conversions
// exact in UpdateProgressAndTotals, including remaining-minus-consumed at a rank
// crossing. The mask expression (including its sum after a terminal clamp) still
// uses binary32 rounding as written. Larger counts are not
// checked: above 2^24, adding 1.0f can round back to the same value and stall
// completion. SetResultData copies rather than enforces this producer contract.
// These are caller invariants, not checks performed here. No reward is awarded
// or recomputed by copying the payload. A texture load failure leaves the new
// owned object installed; readiness polling decides whether drawing can begin.
void CResultMenu_SetResultData(ResultMenuConfigView* self, ResultMenuData* config) {
    int specialMode;
    unsigned short specialTotal;
    unsigned int messageIndex, currentHoleIndex, roundHoleCount, totalProgress, remainingProgress, hasDoublePang, bonusPang, earnedPang, score;
    int textureIndex;
    void* child = self->resultTexture;
    if (child) {
        delete (ChildInterface*)child;
        self->resultTexture = 0;
    }
    specialMode = config->specialMode; // payload +30 selects special-mode result rules
    unsigned int placing = config->placing;
    textureIndex = config->textureIndex;
    score = config->score;
    earnedPang = config->earnedPang;
    bonusPang = config->bonusPang;
    hasDoublePang = config->hasDoublePang;
    remainingProgress = config->remainingProgress;
    totalProgress = config->totalProgress;
    roundHoleCount = config->roundHoleCount;
    currentHoleIndex = config->currentHoleIndex;
    messageIndex = config->messageIndex;
    specialTotal = config->specialTotal;
    self->placing = placing;
    self->textureIndex = textureIndex;
    AsCResultMenu(self)->score = score;
    AsCResultMenu(self)->earnedPang = earnedPang;
    AsCResultMenu(self)->earnedBonusPang = bonusPang;
    AsCResultMenu(self)->doublePangEnabled = hasDoublePang;
    AsCResultMenu(self)->remainingProgress = remainingProgress;
    AsCResultMenu(self)->normalTotalProgress = totalProgress; // total progress displayed for normal results
    AsCResultMenu(self)->roundHoleCount = roundHoleCount;
    AsCResultMenu(self)->currentHoleIndex = currentHoleIndex;
    AsCResultMenu(self)->resultMessageIndex = messageIndex; // suffix of MSG_SINGLE_STROKE_CMN / MATCH_CMN
    AsCResultMenu(self)->specialTotalProgress = specialTotal; // total displayed in special mode
    self->specialMode = specialMode;
    if (!specialMode && (int)placing - 1 < 3 && textureIndex < 3) {
        const char* names[2][3];
        unsigned int* pp = s_resultModePlacingTextureNames;
        names[0][0] = (const char*)*pp++;
        names[0][1] = (const char*)*pp++;
        names[0][2] = (const char*)*pp++;
        names[1][0] = (const char*)*pp++;
        names[1][1] = (const char*)*pp++;
        names[1][2] = (const char*)*pp;
        placing = (self->resultMode == 0);
        ResultTextureStorage* obj = new ResultTextureStorage;
        self->resultTexture = obj;
        ((ChildInterface*)obj)->initialize(names[placing][textureIndex]);
    }
}

// Choose the result script by mode and placing; suppress two groups for special results.
// Mode 2 requires placing 0..3. Normal stroke requires placing >=1 and caps only the upper
// index at 3; match/special use placing==1 as win. No lower-bound validation is added here.
void CResultMenu_SelectStateScript(ResultMenuConfigView* self) {
    if (self->resultMode == kSkipProgressResultMode) {
        unsigned int* p = s_skipProgressResultScriptNames;
        const char* names[4];
        names[0] = (const char*)*p++;
        names[1] = (const char*)*p++;
        names[2] = (const char*)*p++;
        names[3] = (const char*)*p;
        BpObject_StartScriptByName(self->resource, names[self->placing], 1);
    } else if (self->resultMode == kMatchResultMode || self->specialMode) {
        if (self->placing == 1)
            BpObject_StartScriptByName(self->resource, ((ResultScreenStringPool*)resultScreenStrings)->winScript, 1);
        else
            BpObject_StartScriptByName(self->resource, ((ResultScreenStringPool*)resultScreenStrings)->loseScript, 1);
    } else {
        int index = self->placing - 1;
        if (index > 3) index = 3;
        unsigned int* p2 = s_strokeResultPlacingScriptNames;
        const char* names[4];
        names[0] = (const char*)*p2++;
        names[1] = (const char*)*p2++;
        names[2] = (const char*)*p2++;
        names[3] = (const char*)*p2;
        BpObject_StartScriptByName(self->resource, names[index], 1);
    }
    if (self->specialMode) {
        unsigned int ids[2];
        ids[0] = s_specialResultHiddenGroupName0;
        ids[1] = s_specialResultHiddenGroupName1;
        for (unsigned char i = 0; i < 2; ++i) {
            void* obj = BpObject_GetOrCreateNamedGroup(self->resource, ids[i]);
            UIGroup_SetEnabled(obj, 0);
        }
    }
}

#pragma use_lmw_stmw on
// Build the stroke/match message tag, expand it, then apply layout bounds and caption-group color.
// self and constructed text resources required; temporary wide strings own their
// heap bytes and unwind guards balance them. Text receives expanded content while
// buffers are alive. Layout element 45 supplies position/size, caption group tint
// supplies color/opacity; preserve expansion before clearing/freeing temporaries.
void CResultMenu_UpdateTaggedResultText(void* self) {
    // Phase 1: start with centered alignment; normal results append and expand a message tag.
    WideStringStorage expandedText;
    WideStringStorage messageTag;
    short wide[4];
    char messagePosition[8];
    char messageSize[8];
    char messageNumber[8];
    AllocatorByteLow s38;
    AllocatorByteLow s30;
    AllocatorByteLow s28;
    AllocatorByteLow s20;
    AllocatorByteLow s18;
    AllocatorByteLow s10;
    AllocatorByteHigh s8;
    int len;
    unsigned int sz2;
    unsigned int sz;
    unsigned int sz3;
    const char* base = s_resultMessageTagFragments;
    expandedText.flags = 0;
    expandedText.size = 0;
    expandedText.data = 0;
    len = wcslen((const wchar_t*)base);
    WString_ReserveCapacity(&expandedText, len);
    s38.val = s38.junk;
    WString_ReplaceRange(&expandedText, 0, 0, base, base + len * 2, &s38.val);
    StringUnwindCleanup_u8032AE10 stringCleanup0(&expandedText);
    if (AsCResultMenu(self)->specialMode == 0) {
        int v = (*(int*)&AsCResultMenu(self)->resultMessageIndex);
        const char* p1a = base + (int)&((ResultMessageTagPool*)0)->messagePrefix;
        messageTag.flags = 0;
        messageTag.size = 0;
        messageTag.data = 0;
        int len2 = wcslen((const wchar_t*)p1a);
        WString_ReserveCapacity(&messageTag, len2);
        s30.val = s30.junk;
        WString_ReplaceRange(&messageTag, 0, 0, p1a, p1a + len2 * 2, &s30.val);
        StringUnwindCleanup_u8032AE10 stringCleanup1(&messageTag);
        if (AsCResultMenu(self)->resultMode == kStrokeResultMode) {
            const char* p3c = base + (int)&((ResultMessageTagPool*)0)->strokePrefix;
            sz = (messageTag.flags >> 31) == 0 ? (*(unsigned char*)&messageTag & 0x7F) : messageTag.size;
            s28.val = s28.junk;
            int len3 = wcslen((const wchar_t*)p3c);
            WString_ReplaceRange(&messageTag, sz, 0, p3c, p3c + len3 * 2, &s28.val);
        } else {
            const char* p54 = base + (int)&((ResultMessageTagPool*)0)->matchPrefix;
            sz = (messageTag.flags >> 31) == 0 ? (*(unsigned char*)&messageTag & 0x7F) : messageTag.size;
            s20.val = s20.junk;
            int len3 = wcslen((const wchar_t*)p54);
            WString_ReplaceRange(&messageTag, sz, 0, p54, p54 + len3 * 2, &s20.val);
            v -= kMatchMessageIndexBase;
        }
        sprintf(messageNumber, ((ResultScreenStringPool*)resultScreenStrings)->messageNumberFormat, v);
        char* ps = messageNumber;
        short* pw = wide;
        int n = 0;
        while ((unsigned int)n < strlen(messageNumber)) {
            *pw = *ps;
            ps += 1;
            pw += 1;
            n += 1;
        }
        wide[n] = 0;
        sz2 = (messageTag.flags >> 31) == 0 ? (*(unsigned char*)&messageTag & 0x7F) : messageTag.size;
        s18.val = s18.junk;
        int len4 = wcslen((const wchar_t*)wide);
        WString_ReplaceRange(&messageTag, sz2, 0, (const char*)wide, (const char*)wide + len4 * 2, &s18.val);
        const char* p10 = s_resultMessageTagFragments + (int)&((ResultMessageTagPool*)0)->tagEnd;
        sz3 = (messageTag.flags >> 31) == 0 ? (*(unsigned char*)&messageTag & 0x7F) : messageTag.size;
        s10.val = s10.junk;
        int len5 = wcslen((const wchar_t*)p10);
        WString_ReplaceRange(&messageTag, sz3, 0, p10, p10 + len5 * 2, &s10.val);
        Text_ExpandTaggedText(AsCResultMenu(self)->taggedText, &expandedText, (messageTag.flags >> 31) == 0 ? (char*)&messageTag + 2 : messageTag.data, 0);
        if (messageTag.flags >> 31) { stringCleanup1.release(); appFreeUni__FPv(messageTag.data); }
    }
    // Phase 2: apply screen-space bounds and the animated caption group's color/opacity.
    Layout_GetElementPosition(AsCResultMenu(self)->layout, kResultMessageElement, messagePosition);
    Text_SetPosition(AsCResultMenu(self)->text, messagePosition);
    Layout_GetElementSize(AsCResultMenu(self)->layout, kResultMessageElement, messageSize);
    CMsgDraw_SetLayoutSize(AsCResultMenu(self)->text, messageSize);
    void* captionGroup = BpObject_GetOrCreateNamedGroup(AsCResultMenu(self)->resource, (unsigned int)(((ResultScreenStringPool*)resultScreenStrings)->captionGroup));
    unsigned int groupTint = UIGroup_GetTintColor(captionGroup);
    int low;
    int groupOpacity = UIGroup_GetCombinedOpacity(captionGroup);
    // Alpha = 0 for zero opacity, otherwise (2*opacity-1)&255; RGB bytes are reordered
    // from the group's packed tint. Preserve this integer mapping, including its wrapping.
    low = ((-groupOpacity | groupOpacity) >> 31) & (groupOpacity * 2 - 1);
    unsigned int packed = (groupTint << 24) | ((groupTint << 8) & 0xFF0000) | ((groupTint >> 8) & 0xFF00) | (low & 0xFF);
    (*(unsigned int*)&((CMsgDraw*)AsCResultMenu(self)->text)->colorMultiplier) = packed;
    Text_SetContent(AsCResultMenu(self)->text, (expandedText.flags >> 31) == 0 ? (char*)&expandedText + 2 : expandedText.data);
    s8.val = s8.junk;
    WString_ReplaceRange(&expandedText, 0, -1, 0, 0, &s8.val);
    if (expandedText.flags >> 31) { stringCleanup0.release(); appFreeUni__FPv(expandedText.data); }
}
#pragma use_lmw_stmw reset

// Reveal Pang totals over time and animate progress through rank thresholds; report completion.
// deltaSeconds advances only displayElapsedTime. Return 1 if already completed at entry;
// the call that first sets totalsDisplayed still returns 0. segmentLength must be nonzero.
// self/layout/main and progress BP/textTable required; segmentIndex [0,10],
// increasing thresholds and nonnegative reward/current units. This modifies
// DISPLAY state only, not the player's stored Pang/progress. Sounds, stream
// restoration and rank-script restart are observable effects at boundaries.
// No ownership transfer. kFloat_1_Menu8032AE10 is shared with opacity, sound and
// timeline skips; a future progress-rate setting must be separated at this use.
// The precision admission at SetResultData is required here too, including when
// state is installed by another caller: while progressComplete==0, require
// integral float current/initial offsets, 0<=current<=remaining<=2^24,
// initial+remaining<=2^24, and positive integral
// segment lengths<=2^24. Initialization derives the initial offset from the
// integer shared-doc total minus reward and preceding threshold; rank crossing
// resets both float counters and converts (remaining-current) back to int.
// Keep those steps and the +1.0f operation; using a wider accumulator would
// silently change the admitted rounding behavior rather than document it.
int CResultMenu_UpdateProgressAndTotals(void* self, float deltaSeconds) {
    if (AsCResultMenu(self)->totalsDisplayed != 0) {
        return 1;
    }
    // Seconds drive reveal stages; progress units advance once per eligible update call.
    float displaySeconds = AsCResultMenu(self)->displayElapsedTime;
    // Phase 1: reveal the final progress total once both timeline and rank animation finish.
    if (displaySeconds >= kFloat_7_ResultMenu && AsCResultMenu(self)->progressComplete != 0) {
        if (AsCResultMenu(self)->specialMode != 0) {
            Layout_SetElementEnabled(AsCResultMenu(self)->layout, kSpecialProgressElementA, 1);
            Layout_SetElementEnabled(AsCResultMenu(self)->layout, kSpecialProgressElementB, 1);
            Layout_SetElementEnabled(AsCResultMenu(self)->layout, kCommonProgressElement, 1);
            UITextTable_SetInteger(AsCResultMenu(self)->uiTextTable, kTotalProgressTextEntry, AsCResultMenu(self)->specialTotalProgress);
        } else {
            Layout_SetElementEnabled(AsCResultMenu(self)->layout, kNormalProgressElementB, 1);
            Layout_SetElementEnabled(AsCResultMenu(self)->layout, kNormalProgressElementA, 1);
            Layout_SetElementEnabled(AsCResultMenu(self)->layout, kCommonProgressElement, 1);
            UITextTable_SetInteger(AsCResultMenu(self)->uiTextTable, kTotalProgressTextEntry, (*(int*)&AsCResultMenu(self)->normalTotalProgress));
        }
        AsCResultMenu(self)->totalsDisplayed = 1;
    } else {
        // Phase 2: advance one progress unit if the rank script is idle; otherwise reveal totals.
        displaySeconds = AsCResultMenu(self)->displayElapsedTime;
        if (displaySeconds >= kFloat_6_ResultProgressStartSeconds && AsCResultMenu(self)->progressComplete == 0 && BpObject_IsCurrentScriptComplete(AsCResultMenu(self)->progressBpObject) != 0) {
            Layout_SetElementEnabled(AsCResultMenu(self)->layout, AsCResultMenu(self)->segmentIndex + kProgressSegmentElementBase, 1);
            BpObject_SetEnabled(AsCResultMenu(self)->progressBpObject, 1);
            void* obj = BpObject_GetOrCreateNamedGroup(AsCResultMenu(self)->progressBpObject, (unsigned int)(((ResultScreenStringPool*)resultScreenStrings)->rankUpGroup));
            UIGroup_SetOpacity(obj, 0);
            int remaining = (*(int*)&AsCResultMenu(self)->remainingProgress);
            float current = AsCResultMenu(self)->currentProgress;
            float limit = (float)remaining;
            float increment = kFloat_1_Menu8032AE10;
            float advancedProgressUnits = current + increment;
            AsCResultMenu(self)->currentProgress = advancedProgressUnits;
            if (advancedProgressUnits >= limit) {
                // Award exhausted: clamp units, stop the loop and restore stream volume.
                AsCResultMenu(self)->currentProgress = (float)remaining;
                Sound_StopByAlias(g_soundManager, ((ResultScreenStringPool*)resultScreenStrings)->experienceSound);
                AsCResultMenu(self)->progressComplete = 1;
                if (AsCResultMenu(self)->resultTexture != 0) {
                    float f2;
                    if (AsCResultMenu(self)->resultMode == kStrokeResultMode) f2 = kFloat_0_6_ResultMenu; else f2 = kFloat_3_ResultMenu;
                    SoundManager_SetStreamVolume(g_soundManager, AsCResultMenu(self)->savedStreamVolume, f2);
                }
            } else {
                int segmentLength = AsCResultMenu(self)->segmentLength;
                float previous = AsCResultMenu(self)->initialSegmentProgress;
                float segmentLimit = (float)segmentLength;
                // Include units earned before this result in the current segment.
                advancedProgressUnits = advancedProgressUnits + previous;
                if (advancedProgressUnits >= segmentLimit) {
                    if (AsCResultMenu(self)->segmentIndex == kLastProgressSegment) {
                        Sound_StopByAlias(g_soundManager, ((ResultScreenStringPool*)resultScreenStrings)->experienceSound);
                        int length = AsCResultMenu(self)->segmentLength;
                        float completed = (float)length;
                        AsCResultMenu(self)->progressComplete = 1;
                        AsCResultMenu(self)->currentProgress = completed;
                        if (AsCResultMenu(self)->resultTexture != 0) {
                            float f2;
                            if (AsCResultMenu(self)->resultMode == kStrokeResultMode) f2 = kFloat_0_6_ResultMenu; else f2 = kFloat_3_ResultMenu;
                            SoundManager_SetStreamVolume(g_soundManager, AsCResultMenu(self)->savedStreamVolume, f2);
                        }
                    } else {
                        PlayResultSound(((ResultScreenStringPool*)resultScreenStrings)->levelUpSound);
                        UIGroup_SetOpacity(obj, 0xff);
                        BpObject_StartScriptByName(AsCResultMenu(self)->progressBpObject, ((ResultScreenStringPool*)resultScreenStrings)->rankUpScript, 1);
                        int remainingBefore = (*(int*)&AsCResultMenu(self)->remainingProgress);
                        float currentBefore = AsCResultMenu(self)->currentProgress;
                        AsCResultMenu(self)->currentProgress = kFloat_0_ResultMenu;
                        AsCResultMenu(self)->initialSegmentProgress = kFloat_0_ResultMenu;
                        float difference = (float)remainingBefore;
                        difference = difference - currentBefore;
                        int remainingAfter = (int)difference;
                        AsCResultMenu(self)->remainingProgress = remainingAfter;
                        Layout_SetElementEnabled(AsCResultMenu(self)->layout, (*(unsigned int*)&AsCResultMenu(self)->segmentIndex) + kProgressSegmentElementBase, 0);
                        unsigned int nextSegment = (*(unsigned int*)&AsCResultMenu(self)->segmentIndex) + 1;
                        (*(unsigned int*)&AsCResultMenu(self)->segmentIndex) = nextSegment;
                        int off = nextSegment << 2;
                        char* p = (char*)self + off;
                        unsigned int segmentBegin = ((ResultMenuShiftedProgressWindow*)p)->segmentBegin;
                        unsigned int segmentEnd = ((ResultMenuShiftedProgressWindow*)p)->segmentEnd;
                        (*(unsigned int*)&AsCResultMenu(self)->segmentLength) = segmentEnd - segmentBegin;
                        Layout_SetElementEnabled(AsCResultMenu(self)->layout, nextSegment + kProgressSegmentElementBase, 1);
                    }
                } else {
                    PlayResultSound(((ResultScreenStringPool*)resultScreenStrings)->experienceSound);
                }
            }

            // Mask X scale = 10*(1 - (current + initial)/segmentLength); Y stays at one.
            unsigned int progress[2] = {kFloatBits_0_ResultProgressScale, kFloatBits_1_ResultProgressScale};
            int length = AsCResultMenu(self)->segmentLength;
            float currentProgress = AsCResultMenu(self)->currentProgress;
            float previousProgress = AsCResultMenu(self)->initialSegmentProgress;
            float progressLength = (float)length;
            float denominator = (float)progressLength;
            float numerator = currentProgress + previousProgress;
            float fraction = numerator / denominator;
            float scale = kFloat_10_ResultProgressMaskScale;
            float progressValue = scale * fraction;
            progressValue = scale - progressValue;
            *(float*)progress = progressValue;
            void* o = BpObject_GetOrCreateNamedGroup(AsCResultMenu(self)->progressBpObject, (unsigned int)(((ResultScreenStringPool*)resultScreenStrings)->progressMaskGroup));
            UIGroup_SetScale(o, progress);
        } else {
            displaySeconds = AsCResultMenu(self)->displayElapsedTime;
            // Reveal the latest applicable stage; this ladder does not replay skipped stages.
            if (displaySeconds >= kFloat_5_ResultTotalPangRevealSeconds) {
                Layout_SetElementEnabled(AsCResultMenu(self)->layout, kTotalPangElement, 1);
                Layout_SetElementEnabled(AsCResultMenu(self)->layout, kTotalPangPartnerElement, 1);
                unsigned int a = AsCResultMenu(self)->doublePangEnabled;
                unsigned int b = AsCResultMenu(self)->earnedPang;
                unsigned int c = AsCResultMenu(self)->earnedBonusPang;
                unsigned int sum = b + c;
                if (a != 0) sum = sum << 1;
                UITextTable_SetInteger(AsCResultMenu(self)->uiTextTable, kTotalPangTextEntry, sum);
            } else if (displaySeconds >= kFloat_4_ResultLabelRevealSeconds) {
                Layout_SetElementEnabled(AsCResultMenu(self)->layout, kResultLabelElement0, 1);
                Layout_SetElementEnabled(AsCResultMenu(self)->layout, kResultLabelElement1, 1);
                Layout_SetElementEnabled(AsCResultMenu(self)->layout, kResultLabelElement2, 1);
            } else if (displaySeconds >= kFloat_3_ResultMenu) {
                Layout_SetElementEnabled(AsCResultMenu(self)->layout, kDoublePangElement, 1);
            } else if (displaySeconds >= kFloat_2_ResultBonusPangRevealSeconds) {
                Layout_SetElementEnabled(AsCResultMenu(self)->layout, kBonusPangElement, 1);
                Layout_SetElementEnabled(AsCResultMenu(self)->layout, kBonusPangPartnerElement, 1);
                UITextTable_SetInteger(AsCResultMenu(self)->uiTextTable, kBonusPangTextEntry, AsCResultMenu(self)->earnedBonusPang);
            } else if (displaySeconds >= kFloat_1_Menu8032AE10) {
                Layout_SetElementEnabled(AsCResultMenu(self)->layout, kEarnedPangElement, 1);
                Layout_SetElementEnabled(AsCResultMenu(self)->layout, kEarnedPangPartnerElement, 1);
                UITextTable_SetInteger(AsCResultMenu(self)->uiTextTable, kEarnedPangTextEntry, AsCResultMenu(self)->earnedPang);
            }
        }
    }
    // Phase 3: advance the reveal clock and skip stages absent in this result mode.
    displaySeconds = AsCResultMenu(self)->displayElapsedTime;
    if (displaySeconds < kFloat_7_ResultMenu) {
        displaySeconds = displaySeconds + deltaSeconds;
        int hasDoublePang = (*(int*)&AsCResultMenu(self)->doublePangEnabled);
        AsCResultMenu(self)->displayElapsedTime = displaySeconds;
        if (hasDoublePang == 0) {
            if (displaySeconds >= kFloat_3_ResultMenu) {
                if (displaySeconds < kFloat_4_ResultLabelRevealSeconds) {
                    displaySeconds = displaySeconds + kFloat_1_Menu8032AE10;
                    AsCResultMenu(self)->displayElapsedTime = displaySeconds;
                }
            }
        }
        if (AsCResultMenu(self)->resultMode == kSkipProgressResultMode) {
            displaySeconds = AsCResultMenu(self)->displayElapsedTime;
            if (displaySeconds >= kFloat_6_ResultProgressStartSeconds) {
                if (displaySeconds < kFloat_7_ResultMenu) {
                    AsCResultMenu(self)->progressComplete = 1;
                    displaySeconds = displaySeconds + kFloat_1_Menu8032AE10;
                    AsCResultMenu(self)->displayElapsedTime = displaySeconds;
                }
            }
        }
    }
    return 0;
}

// Initialize this module's stored two-pi constant from the shared scalar pool.
void ResultMenu_InitTauConstant() {
    s_resultMenuTau = kFloat_6_2831855;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462638", _ctors_u8032AE10_0, ResultFightMenu_InitTauConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462638", _ctors_u8032AE10_1, ResultMenu_InitTauConstant);
#pragma pop
