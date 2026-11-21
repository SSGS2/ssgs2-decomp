#include <decomp/static_init.h>
extern "C" char s_playerInfoBpCtrlDestructorRegistration[];
// The first declaration must retain noreturn for exception cleanup generation.
extern "C" void __throw(const char*, void*, void*) __attribute__((noreturn));
#include "sdk_protos.h"
#include "game/types.h"
#include "game/singletons.h"
#include "game/vector.h"
#include "classes/CPlayerInfo.h"
#include "classes/CMsgDraw.h"

#define AsCPlayerInfo(p) ((CPlayerInfo*)(p))
// Byte positions in the external packed string pool; preserve its original storage and encoding.
enum {
    kPlayerInfo_status_zip = 0, // "status.zip"
    kPlayerInfo_status_outgame_spd = 0xb, // "status_outgame.spd"
    kPlayerInfo_status_outgame_b2d = 0x1e, // "status_outgame.b2d"
    kPlayerInfo_p2_common_bg_b2d = 0x31, // "p2_common_bg.b2d"
    kPlayerInfo_num_status_01_gsr = 0x42, // "num_status_01.gsr"
    kPlayerInfo_num_status_00_gsr = 0x54, // "num_status_00.gsr"
    kPlayerInfo_ui_dialog_open = 0x66, // "ui_dialog_open"
    kPlayerInfo_basic_info = 0x75, // "basic_info"
    kPlayerInfo_R_status_expgraph = 0x80, // "R_status_expgraph"
    kPlayerInfo_R_status_hyphen1 = 0x92, // "R_status_hyphen1"
    kPlayerInfo_R_status_hyphen2 = 0xa3, // "R_status_hyphen2"
    kPlayerInfo_R_status_hyphen3 = 0xb4, // "R_status_hyphen3"
    kPlayerInfo_R_status_hyphen4 = 0xc5, // "R_status_hyphen4"
    kPlayerInfo_R_status_hyphen5 = 0xd6, // "R_status_hyphen5"
    kPlayerInfo_R_status_hyphen6 = 0xe7, // "R_status_hyphen6"
    kPlayerInfo_R_status_hyphen7 = 0xf8, // "R_status_hyphen7"
    kPlayerInfo_R_status_hyphen8 = 0x109, // "R_status_hyphen8"
    kPlayerInfo_R_status_hyphen9 = 0x11a, // "R_status_hyphen9"
    kPlayerInfo_bg = 0x12b, // "bg"
    kPlayerInfo_ui_icon_click = 0x12e, // "ui_icon_click"
    kPlayerInfo_ui_dialog_close = 0x13c, // "ui_dialog_close"
    kPlayerInfo_shot_info = 0x14c, // "shot_info"
    kPlayerInfo_personal_record = 0x156, // "personal_record"
    kPlayerInfo_course_record = 0x166, // "course_record"
    kPlayerInfo_R_status_scroll_upendl = 0x174, // "R_status_scroll_upendl"
    kPlayerInfo_R_status_scroll_upendc = 0x18b, // "R_status_scroll_upendc"
    kPlayerInfo_R_status_scroll_upendr = 0x1a2, // "R_status_scroll_upendr"
    kPlayerInfo_R_status_scroll_dendl = 0x1b9, // "R_status_scroll_dendl"
    kPlayerInfo_R_status_scroll_dendc = 0x1cf, // "R_status_scroll_dendc"
    kPlayerInfo_R_status_scroll_dendr = 0x1e5, // "R_status_scroll_dendr"
    kPlayerInfo_R_status_shadow_up = 0x1fb, // "R_status_shadow_up"
    kPlayerInfo_R_status_scrollb_upl = 0x20e, // "R_status_scrollb_upl"
    kPlayerInfo_R_status_scrollb_upc = 0x223, // "R_status_scrollb_upc"
    kPlayerInfo_R_status_scrollb_upr = 0x238, // "R_status_scrollb_upr"
    kPlayerInfo_R_status_shadow_down = 0x24d, // "R_status_shadow_down"
    kPlayerInfo_R_status_scrollb_downl = 0x262, // "R_status_scrollb_downl"
    kPlayerInfo_R_status_scrollb_downc = 0x279, // "R_status_scrollb_downc"
    kPlayerInfo_R_status_scrollb_downr = 0x290, // "R_status_scrollb_downr"
    kPlayerInfo_R_MSG_playername_LC = 0x347, // "R_MSG_playername_LC"
    kPlayerInfo_vector_at_index_out_of_range = 0x2f2, // "vector::at index out of range"
    kPlayerInfo_std_exception_std_logic_error_std_out_of_range = 0x310, // "!std::exception!!std::logic_error!!std::out_of_range!!"
};

extern "C" {
extern float kFloat_6_2831855;
extern float s_playerInfoFullTurnRadiansA;
extern float s_playerInfoFullTurnRadiansB;
struct OwnedWideString {
    unsigned int flags;
    char padding04[4];
    void* value;
};

struct PlayerInfoLoadState {
    char pad[0x20];
    void* layout;
    char pad24[0x78];
    void* current;
};

void Layout_Update(void*);
void CPackTextureResource_PollLoad(void*);
int CPackTextureResource_IsLoadFinished(void*);
int __ct__7CBpCtrlFv(void*);
int CBpCtrl_PollAllEntriesLoaded(void*);
int Layout_PollLoadComplete(void*);
void CBpCtrl_dtor();
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_playerInfoBpCtrlDestructorRegistration[];
struct PlayerInfoTexture {
    virtual void m0();
    virtual void init(int);
    virtual void m2();
    virtual void apply();
    virtual int valid();
    char data[36];
};

extern signed char s_pointerVectorInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern signed char s_pointerPairListInitialized;
extern char s_fontNumDrawRegistry[];
extern char s_messageDrawPairList[];
extern char s_playerInfoDestructorRegistrations[];
extern char __vt__11CPlayerInfo[];
void Layout_Destroy(void*, int);
void UITextTable_ClearEntries(void*);
void PointerVector_InitReserved(void*);
void PointerArray_DeleteValue(void*, int);
void PointerVector_DestroyStorage();
void CBpCtrl_DeleteEntryById(void*, int);
void PairVector_InitReserved(void*);
void PointerPairList_RemoveByKey(void*, int);
void MessageDrawPairList_Dtor();
void CBpTex_dtor();
void __destroy_new_array(void*, void (*)());
void WString_ReplaceRange(void*, int, int, void*, void*, void*);
void CBpCtrlHandle_Dtor();
void CBpCtrlHandle_SetScriptIndex(void*, int);
void __dl__FPv(void*);
struct SwingControllerInterface {
    char pad[0x58];
    void** vtable;
};

struct PlayerInfoOwnedResources {
    void** vtable;
    char pad04[0x18];
    int bpEntryId;
    int layoutWord;
    int numberDrawWord;
    char pad28[4];
    void* text;
    unsigned int textStateFlags;
    char pad34[4];
    void* textStateStorage;
    char pad3c[0x38];
    void* courseTextures;
    void* unavailableCourseTextures;
    void* characterTextures;
    unsigned int nameStringFlags;
    char pad84[4];
    void* nameStringData;
    char pad8c[8];
    SwingControllerInterface* padSwingCtrl;
    char pad98[4];
    void* texturePack;
    int previousScriptIndex;
    int pageEntryId;
};

struct CourseTextureIds {
    int v[14];
};

struct CharacterTextureIds {
    int v[9];
};

struct PlayerLevelDefinition {
    unsigned int metadata[4]; // Other level-definition words are not read by this unit.
    unsigned int end;
};

extern PlayerLevelDefinition playerLevelDefinitions[];
extern int s_levelCourseTextureIds[];
extern int s_levelUnavailableCourseTextureIds[];
extern int s_levelCharacterTextureIds[];
extern char playerInfoStrings[];
extern signed char s_resourceTableInitialized;
extern int s_resourceTableStorage;
extern char* g_chatMsg;
void MPBase1_Ctor(void*, int);
void Allocation_Dtor();
void* IdMap_Lookup(void*, int);
void* ResourceTable_FindEntryByName(void*, const char*, int*);
void* __nw__FUl(unsigned long);
void CPackTextureResource_Init(void*, const char*, int, void*);
void Layout_Init(void*, int);
int CFontNumDraw_CreateInVector(void*);
void WFixedFont_CreateAndRegisterMsgDraw(void*, void*, int);
void Layout_LoadExtended(void*, int, const char*);
void CBpCtrl_CreateEntry(void*, const char*, int*, int);
void* __nwa__FUl(unsigned long);
void CBpTex_Init();
void CPadSwingCtrl_Ctor(void*);
extern signed char s_saveDataInitialized;
extern void* g_soundManager;
extern char s_saveData[];
extern char s_playerInfoNamePrefix[];
extern int s_playerInfoLevelIconIds[];
extern char s_playerInfoNumericTextDefinitions[];
extern int kPlayerInfoExperienceBarZeroBits;
extern int kPlayerInfoExperienceBarOneBits;
extern float kFloat_0_5_PlayerInfoSound;
extern float kFloat_200_StatisticsSound;
extern float kFloat_1_PlayerInfo;
extern float kFloat_0_Statistics;
extern float kPlayerInfoExperienceBarScaleFactor;
extern float kPlayerInfoExperienceBarPositionFactor;
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[];
void SaveData_Ctor(void*);
char* SaveData_GetCurrentRecord(void*);
void SaveRecord_CopyCtor(void*, void*);
void Layout_GetElementPosition(void*, int, int*);
void Text_SetPosition(void*, int*);
void Layout_GetElementSize(void*, int, int*);
void CMsgDraw_SetLayoutSize(void*, int*);
int PlayerRank_FindTierForValue(int);
void* UIGroup_GetOrCreateById(void*, int);
void UIGroup_SetEnabled(void*, int);
void Layout_SetElementEnabled(void*, int, int);
void CFontNumDraw_RegisterEntries(void*, const char*, int, void*, void*);
void UITextTable_SetEntryResourceName(void*, int, const char*);
void Layout_ReadElementRect(void*, int, int*);
void Layout_WriteElementRect(void*, int, int*);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
void BpObject_StartScriptByName(void*, const char*, int);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);
void UIGroup_GetPosition(void*, void*);
void UIGroup_SetPosition(void*, void*);
void UIGroup_SetScale(void*, void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void FadeOverlay_Init(void*);
void ResourceState_Dtor();
void FadeState_StartFadeOut(void*, int);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
struct LevelIconIds {
    int v[11];
};

struct TexturePackInterface {
    virtual void release(int);
    virtual void v1();
    virtual void v2(int);
};

struct PlayerInfoSaveRecord {
    char pad[0x24];
    int experience;
    char rest[0x800];
};

struct PlayerInfoProcessState {
    char basePadding[0x18];
    void* bpObject;
    char padding1c[0x4];
    void* layout;
    char padding24[0x28];
    int scrollDelayActive;
    int scrollDelayFrames;
    int courseRecordIndex;
    char padding58[0x4];
    int page;
    int requestedPage;
    int basicPageInitialized;
    int shotPageInitialized;
    int personalPageInitialized;
    int coursePageInitialized;
    char padding74[0x18];
    int opened;
    int closing;
};

int BpObject_IsCurrentScriptComplete(void*);
int FadeOverlay_GetInstance();
int FadeState_GetState(void*);
void Layout_CollectActivated(void*, void*, int);
int Vector_GetCount(void*);
void* LayoutManager_GetInstance();
int Input_IsActionPressed(void*, int, int);
void LayoutIntVector_PushBack(void*, const char*);
void FadeState_StartFadeIn(void*, int);
void FadeState_RequestFramebufferCapture(void*);
extern int s_playerInfoBasicNextElement;
extern int s_playerInfoBasicCloseElement;
extern int s_playerInfoShotNextElement;
extern int s_playerInfoShotPreviousElement;
extern int s_playerInfoShotCloseElement;
extern int s_playerInfoPersonalNextElement;
extern int s_playerInfoPersonalPreviousElement;
extern int s_playerInfoPersonalCloseElement;
extern int s_playerInfoCoursePreviousElement;
extern int s_playerInfoCourseCloseElement;
extern void* jumptable_804B5C28;
struct Vertex {
    float x,y;
    unsigned int rest[4];
};

struct Quad {
    Vertex v[4];
};

extern Quad s_playerInfoCharacterIconQuad;
int GX_GetPrimitiveCount(int,int);
void DrawPrimitiveUP(int,int,void*,int);
struct PlayerInfoIconOwner {
    char pad[32];
    void* layout;
    char pad24[88];
    PlayerInfoTexture* items;
};

struct RecordCharacterIndex {
    char pad[6];
    signed char index;
    char last;
};

extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12out_of_range[];
void __dla__FPv(void*);
void __dt__Q23std9exceptionFv(void*,int);
void __dt__Q23std12out_of_rangeFv();
struct SharedBlock {
    void* vt;
    int a,b;
    void* data;
    // Give a newly allocated exception-message buffer its shared ownership block.
    SharedBlock(void* p) {
        a=1;
        b=1;
        vt=s_sharedCharDeleterVtable;
        data=p;
    }
};

struct Guard_u8029736C {
    int allocatorWord;
    void* data;
    char* flag;
    // Free the message allocation when control-block construction fails.
    ~Guard_u8029736C() {
        if (data) __dla__FPv(data);
    }
};

struct ErrorBase_u8029736C {
    void* vt;
    char* str;
    SharedBlock* block;
    // Run exception-base cleanup during local error construction/destruction.
    ~ErrorBase_u8029736C() {
        __dt__Q23std9exceptionFv(this,0);
    }
};

extern Quad s_playerInfoCourseIconQuad;
void BpObject_Draw(void*);
void CMsgDraw_ResetTextState(void*);
void CMsgDraw_ClearFormattingAndRevealAll(void*);
unsigned int UIGroup_GetTintColor(void*);
int UIGroup_GetCombinedOpacity(void*);
void Text_SetContent(void*,void*);
void Layout_DrawWithOpacity(void*,float);
void UITextTable_Draw(void*);
void CMsgDraw_UpdateReveal(void*);
void Text_DrawGlyphs(void*);
struct PlayerInfoDrawState {
    char p0[0x18];
    void* panel;
    void* bpEntryWord;
    void* layout;
    void* numberDraw;
    void* textFactory;
    void* text;
    char p30[0x24];
    int courseRecordIndex;
    int levelIconIndex;
    int page;
    char p60[0x14];
    PlayerInfoTexture* items;
    char characterTexturePadding[8];
    unsigned int stringFlag;
    int nameStringLength;
    void* stringPtr;
    char pageStatePadding[0x18];
    int id;
};

void UITextTable_SetInteger(void*,int,int);
void UITextTable_SetFloat(void*,int,float);
void NumericTextTable_ClearEntryText(void*,int);
extern float kFloat_100_PlayerInfo;
extern float kFloat_0_03125_PlayerInfo;
float ShotGauge_GetTotal();

// Save-record fields used by the numeric statistics pages. Lettered counters retain
// their UI/formula roles because the underlying scoring events are not yet identified.
struct PlayerStatisticsRecord {
    char playerName[0x20];
    int playTimeFrames;
    int experience;
    int pang;
    char paddingToShotStatistics[0x1B0];
    int sampleCountA; // +1DC: denominator of shot rate A and part of combined-rate denominator.
    int sampleCountB; // +1E0: denominator of shot rate B and part of combined-rate denominator.
    int shotCounter;  // +1E4: displayed as an integer at kTextShotCount; event not identified.
    int successCountA; // +1E8: numerator of shot rate A (displayed as percent).
    int personalCountA;
    int personalCountB;
    int sampleCountC; // +1F4: denominator of shot rate C.
    int successCountC; // +1F8: numerator of shot rate C (displayed as percent).
    int successCountB; // +1FC: numerator of shot rate B (displayed as percent).
    int combinedSuccessCount; // +200: numerator divided by sample A + sample B.
    float personalValueA, personalValueB, personalValueC; // +204/+208/+20C: truncate to 2 decimal places.
    int personalCountC;
    float shotValue; // +214: displayed directly at kTextShotValue; unit/event not established.
    // +218/+238: two groups of 32 signed bytes. ZERO contributes to the displayed
    // percentage; neither course completion nor achievement completion is established.
    signed char completionFlags[64];
};
struct PlayerInfoTextFactory { char prefix[0x10]; void* fixedFont; };
enum PlayerInfoConstants {
    kPlayerInfoBasicPage = 0, kPlayerInfoShotPage = 1,
    kPlayerInfoPersonalPage = 2, kPlayerInfoCoursePage = 3,
    kPlayerInfoCourseCount = 14, kPlayerInfoCharacterCount = 8,
    kPlayerInfoRecordWindow = 3, kPlayerInfoLastRecordWindow = kPlayerInfoCourseCount - kPlayerInfoRecordWindow,
    kPlayerInfoFramesPerSecond = 30, kPlayerInfoFramesPerMinute = 1800,
    kPlayerInfoFramesPerHour = 108000,
    kPlayerInfoNameOffset = ((int)&((CPlayerInfo*)0)->nameStringFlags),
    kPlayerInfoNameInlineOffset = ((int)&((CPlayerInfo*)0)->nameStringFlags) + 2
};
// UI contract (layout IDs, independent of page-state values above):
// 0/2 scroll toward lower index, 1/3 toward higher index (paired state players);
// 4/6 lower-index motion visuals, 5/7 higher-index motion visuals;
// 8/10 lower-index end caps, 9/11 higher-index end caps.
// Main row: buttons 12..16 = basic/shot/personal/course/close; highlights 17..21.
// Paired row: buttons 22..26 in the same order; highlights 27..31.
// For either row, current-page button state is 0 and its highlight is enabled;
// other page/close button states are 1. The paired row's rectangles are copied
// from the main row at initialization. Input switches below consume IDs 12..16.
// 0x20 is the player-name text rectangle. 0x21..0x38 are hidden layout anchors
// for custom content (individual labels unresolved). 0x39..0x3B are course-icon
// rectangles, 0x3C..0x3E character-icon rectangles, 0x3F..0x47 three groups of
// three placeholders enabled when that record's character index is invalid.
// Course scrolling shows three consecutive records starting at index 0..11
// out of fourteen; each move gates further input for four update calls.
enum PlayerInfoElement {
    kPlayerInfoScrollLower = 0, kPlayerInfoScrollHigher = 1,
    kPlayerInfoPairedScrollLower = 2, kPlayerInfoPairedScrollHigher = 3,
    kPlayerInfoLowerMotion = 4, kPlayerInfoHigherMotion = 5,
    kPlayerInfoPairedLowerMotion = 6, kPlayerInfoPairedHigherMotion = 7,
    kPlayerInfoLowerEnd = 8, kPlayerInfoHigherEnd = 9,
    kPlayerInfoPairedLowerEnd = 10, kPlayerInfoPairedHigherEnd = 11,
    kPlayerInfoRecordPlaceholderFirst = 63, kPlayerInfoRecordPlaceholderLast = 71,
    kPlayerInfoBasicButton = 12, kPlayerInfoShotButton = 13,
    kPlayerInfoPersonalButton = 14, kPlayerInfoCourseButton = 15,
    kPlayerInfoCloseButton = 16,
    kPlayerInfoBasicHighlight = 17, kPlayerInfoShotHighlight = 18,
    kPlayerInfoPersonalHighlight = 19, kPlayerInfoCourseHighlight = 20,
    kPlayerInfoCloseHighlight = 21,
    kPlayerInfoPairedBasicButton = 22, kPlayerInfoPairedShotButton = 23,
    kPlayerInfoPairedPersonalButton = 24, kPlayerInfoPairedCourseButton = 25,
    kPlayerInfoPairedCloseButton = 26,
    kPlayerInfoPairedBasicHighlight = 27, kPlayerInfoPairedShotHighlight = 28,
    kPlayerInfoPairedPersonalHighlight = 29, kPlayerInfoPairedCourseHighlight = 30,
    kPlayerInfoPairedCloseHighlight = 31, kPlayerInfoNameRect = 0x20
};
// Layout-manager action IDs are separate from element IDs and statistics text slots.
enum PlayerInfoInputAction { kPlayerInfoPreviousAction = 2, kPlayerInfoNextAction = 3, kPlayerInfoCloseAction = 6 };
enum { kPlayerInfoCourseRecordsOffset = 0x25C, kPlayerInfoCompletionBlockSize = 32 };

// Numeric table entries (separate ID namespace from layout elements):
// Basic: AA/A9/A8 hours/minutes/seconds, B7/B8 experience progress/tier span,
// A7 record +28 value, A6 record +132 short, A5/A4 percentages of ZERO flags
// in +218's first/second 32-byte blocks, A3 ShotGauge_GetTotal result.
// Shot: A6/A4/A5 = success A/B/C divided by sample A/B/C; AC uses combined
// successes / (sample A+B), AB shotCounter, A3 shotValue. Zero denominators -> 0.
// Personal: B2 count C, B1/B0/AF values A/B/C truncated toward zero to 2 decimals,
// AE/AD counts A/B. Course: A2/A1/A0 counts and 9F/9E/9D values for three records.
// Event meanings of lettered counters and zero-completion flags remain unresolved.
// Modding lookup: each enum below names the exact page/text slot that consumes a field.
// Rate A uses +1E8/+1DC; B +1FC/+1E0; C +1F8/+1F4; combined +200/(+1DC + +1E0).
// No verified writer/event-to-slot map is available here: do not relabel these as
// fairway/GIR/PangYa/putt rates or the personal values as yardages without that evidence.
// These text slots are reused by page; A/B/C retain unresolved scoring-event identities.
enum PlayerInfoNumericText {
    kTextGaugeTotal = 163,
    kTextCompletionSecondBlock = 164,
    kTextCompletionFirstBlock = 165,
    kTextSavedShort132 = 166,
    kTextPang = 167,
    kTextPlaySeconds = 168,
    kTextPlayMinutes = 169,
    kTextPlayHours = 170,
    kTextExperienceProgress = 183,
    kTextExperienceSpan = 184,
    kTextShotValue = 163,
    kTextShotRateB = 164,
    kTextShotRateC = 165,
    kTextShotRateA = 166,
    kTextShotCount = 171,
    kTextShotCombinedRate = 172,
    kTextPersonalCountB = 173,
    kTextPersonalCountA = 174,
    kTextPersonalValueC = 175,
    kTextPersonalValueB = 176,
    kTextPersonalValueA = 177,
    kTextPersonalCountC = 178,
    kTextCourseValueThird = 157,
    kTextCourseValueSecond = 158,
    kTextCourseValueFirst = 159,
    kTextCourseCountThird = 160,
    kTextCourseCountSecond = 161,
    kTextCourseCountFirst = 162,
    kPlayerInfoScrollDelayCalls = 4
};
// Partial live-save view: only this displayed short is needed; its event is unresolved.
struct PlayerInfoLiveTotalsView { char padToSavedShort[0x132]; short savedShort132; };

typedef char PlayerStatsShotsOffset[((int)&((PlayerStatisticsRecord*)0)->sampleCountA == 0x1DC) ? 1 : -1];
typedef char PlayerStatsCompletionOffset[((int)&((PlayerStatisticsRecord*)0)->completionFlags == 0x218) ? 1 : -1];

struct StatEntry {
    int value;
    short count;
    signed char index;
    char pad;
};

void WString_DestroyMapKey(void*, OwnedWideString* object);
void PlayerInfo_InitFullTurnConstantA();
void PlayerInfo_InitFullTurnConstantB();
void* CPlayerInfo_Ctor(void* self, int arg);
PlayerInfoOwnedResources* CPlayerInfo_dtor(PlayerInfoOwnedResources* self, int freeFlag);
void CPlayerInfo_UpdateWhileWaiting(PlayerInfoLoadState* self);
int CPlayerInfo_PollLoadComplete(PlayerInfoLoadState* self);
void CPlayerInfo_InitializePage(void* self);
int CPlayerInfo_UpdatePageInputAndDispatch(PlayerInfoProcessState* self);
void CPlayerInfo_Draw(PlayerInfoDrawState* self);
int CPlayerInfo_SetModeButtonStates(void* obj, int mode);
int CPlayerInfo_PopulateStatisticsPage(void* obj,int mode);
int CPlayerInfo_DrawRecordIcons(PlayerInfoIconOwner* self,int index);
unsigned int* PlayerInfoWordVector_At(void* obj,unsigned int index);
// Free a heap-backed wide-string map key; inline strings own no separate allocation.
void WString_DestroyMapKey(void*, OwnedWideString* object) {
    if (object == 0) return;
    if (object == 0) return;
    if (object->flags >> 31) {
        appFreeUni__FPv(object->value);
    }
}

// Initialize the first player-info full-turn angle constant.
void PlayerInfo_InitFullTurnConstantA() {
    s_playerInfoFullTurnRadiansA = kFloat_6_2831855;
}

// Initialize the second player-info full-turn angle constant.
void PlayerInfo_InitFullTurnConstantB() {
    s_playerInfoFullTurnRadiansB = kFloat_6_2831855;
}

// Byte offsets of each object's destructor-registration record inside s_playerInfoDestructorRegistrations
// (one 12-byte record per object registered with __register_global_object).
enum { kRegBpCtrl = 0xc, kRegBpCtrlHandle = 0x18, kRegFontNumRegistry = 0x24, kRegPairList = 0x30,
       kRegResourceTable = 0x3c, kRegFadeOverlay = 0x48 };

// ---- Lazy singleton set-up ----
// The original code pasted this block inline at every use: if the global pointer is still null, construct the
// static object once (guard flag + __register_global_object with its destructor record) and publish it.
// Each helper owns one singleton. Sing_805DC5F0 = BpCtrl (guard s_bpCtrlInitialized), Sing_805DC5F8 = BpCtrl
// handle (script index), Sing_805DC650 = message-draw pointer-pair list, Sing_805DC830 = font number-draw
// registry, Sing_805DC608 = fade overlay. `reg` is that object's destructor-registration record.
// Construct/register the BP controller once and publish its borrowed singleton.
static inline void EnsureBpCtrl(void* reg) {
    if (!((int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, reg);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
}
// Initialize the shared script-index handle and register its teardown once.
static inline void EnsureBpCtrlHandle(void* reg) {
    if (!((int&)Sing_805DC5F8())) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, reg);
            s_bpCtrlHandleInitialized = 1;
        }
        ((int&)Sing_805DC5F8()) = (int)s_bpCtrlHandle;
    }
}
// Create the shared message-draw registry and register its owned storage cleanup.
static inline void EnsurePairList(void* reg) {
    if (!((int&)Sing_805DC650())) {
        if (!s_pointerPairListInitialized) {
            PairVector_InitReserved(s_messageDrawPairList);
            __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor, reg);
            s_pointerPairListInitialized = 1;
        }
        ((int&)Sing_805DC650()) = (int)s_messageDrawPairList;
    }
}
// Create the shared numeric-renderer registry and register its storage cleanup.
static inline void EnsureFontNumRegistry(void* reg) {
    if (!((int&)Sing_805DC830())) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, reg);
            s_pointerVectorInitialized = 1;
        }
        ((int&)Sing_805DC830()) = (int)s_fontNumDrawRegistry;
    }
}
// Construct/register the shared fade overlay once and publish its borrowed singleton.
static inline void EnsureFadeOverlay(void* reg) {
    if (!((int&)Sing_805DC608())) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, reg);
            s_fadeOverlayInitialized = 1;
        }
        ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
    }
}

#pragma use_lmw_stmw on
#pragma opt_common_subs off
struct PlayerInfoPackAllocation {
    char storage[0x20];
    // Construct an owned texture pack from the borrowed archive name and script entry.
    PlayerInfoPackAllocation(const char* name, int count, void* entry) {
        CPackTextureResource_Init(this, name, count, entry);
    }
};
struct PlayerInfoLayoutAllocation {
    char storage[0x5c];
    // Initialize the menu-owned layout using the original allocation size.
    PlayerInfoLayoutAllocation(int mode) { Layout_Init(this, mode); }
};
struct PlayerInfoSwingAllocation {
    char storage[0x5c];
    // Construct the menu-owned swing controller in its original allocation.
    PlayerInfoSwingAllocation() { CPadSwingCtrl_Ctor(this); }
};
struct PlayerInfoCourseArrayAllocation {
    char storage[0x240];
    // Allocate array-wrapper storage through the game's array allocator.
    static void* operator new(size_t size) { return __nwa__FUl(size); }
    // Release wrapper storage if array element construction throws.
    static void operator delete(void* block) { __dla__FPv(block); }
    // Construct fourteen owned course textures and return the array payload after its cookie.
    PlayerInfoCourseArrayAllocation(void*& result) {
        result = __construct_new_array(this, (void*)CBpTex_Init, (void*)CBpTex_dtor, 0x28, 14);
    }
};
struct PlayerInfoCharacterArrayAllocation {
    char storage[0x150];
    // Allocate array-wrapper storage through the game's array allocator.
    static void* operator new(size_t size) { return __nwa__FUl(size); }
    // Release wrapper storage if array element construction throws.
    static void operator delete(void* block) { __dla__FPv(block); }
    // Construct eight owned character textures and return the array payload after its cookie.
    PlayerInfoCharacterArrayAllocation(void*& result) {
        result = __construct_new_array(this, (void*)CBpTex_Init, (void*)CBpTex_dtor, 0x28, 8);
    }
};
// Create the statistics layout, text renderers, script entries, course/character textures and swing controller.
void* CPlayerInfo_Ctor(void* self, int recordPointer) {
    // recordPointer is a borrowed save-record address carried as a word. Owned
    // layout/texture arrays/swing controller are deleted by dtor; number/text
    // renderers additionally register in global registries and must be removed there.
    char* destructorRegistrations;
    char* levelData = (char*)playerLevelDefinitions;
    destructorRegistrations = (char*)s_playerInfoDestructorRegistrations;
    MPBase1_Ctor(self, recordPointer);
    AsCPlayerInfo(self)->vtbl = __vt__11CPlayerInfo;
    AsCPlayerInfo(self)->bpEntryId = -1;
    // Empty the text-state string (tag, long-string length, heap pointer).
    AsCPlayerInfo(self)->unk_30 = 0;
    AsCPlayerInfo(self)->unk_34 = 0;
    AsCPlayerInfo(self)->unk_38 = 0;
    *(int*)((char*)self+0x4c) = 0;
    *(int*)((char*)self+0x50) = 0;
    *(int*)((char*)self+0x54) = 0;
    *(int*)((char*)self+0x58) = 0;
    *(int*)((char*)self+0x5c) = 0;
    *(int*)((char*)self+0x60) = 0;
    *(int*)((char*)self+0x64) = 0;
    *(int*)((char*)self+0x68) = 0;
    *(int*)((char*)self+0x6c) = 0;
    *(int*)((char*)self+0x70) = 0;
    *(int*)((char*)self+0x74) = 0;
    *(int*)((char*)self+0x78) = 0;
    *(int*)((char*)self+0x7c) = 0;
    *(int*)((char*)self+0x80) = 0;
    *(int*)((char*)self+0x84) = 0;
    *(int*)((char*)self+0x88) = 0;
    *(int*)((char*)self+0x8c) = 0;
    *(int*)((char*)self+0x90) = 0;
    *(int*)((char*)self+0x94) = 0;
    AsCPlayerInfo(self)->playerRecord = recordPointer;
    (*(int*)&AsCPlayerInfo(self)->texturePack) = 0;
    AsCPlayerInfo(self)->previousScriptIndex = -1;
    AsCPlayerInfo(self)->pageEntryId = -1;
    // Phase 1: request the texture pack and create/register layout and text renderers.
    int count = 0;
    int resourceTable = ((int&)Sing_805DC620());
    if (!resourceTable) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor, (destructorRegistrations + kRegResourceTable));
            s_resourceTableInitialized = 1;
        }
        ((int&)Sing_805DC620()) = (int)&s_resourceTableStorage;
    }
    void* obj;
    int resourceEntry = (int)ResourceTable_FindEntryByName((void*)((int&)Sing_805DC620()), playerInfoStrings + kPlayerInfo_status_zip, &count);
    obj = new PlayerInfoPackAllocation(playerInfoStrings, count, (void*)resourceEntry);
    AsCPlayerInfo(self)->texturePack = obj;
    obj = new PlayerInfoLayoutAllocation(5);
    AsCPlayerInfo(self)->layout = obj;
    EnsureFontNumRegistry((destructorRegistrations + kRegFontNumRegistry));
    (*(int*)&AsCPlayerInfo(self)->numberDraw) = CFontNumDraw_CreateInVector((void*)((int&)Sing_805DC830()));
    void* msgObj = (void*)(*(int*)&((PlayerInfoTextFactory*)g_chatMsg)->fixedFont);
    AsCPlayerInfo(self)->textFactory = (int)msgObj;
    WFixedFont_CreateAndRegisterMsgDraw(msgObj, &AsCPlayerInfo(self)->text, 1);
    Layout_LoadExtended(AsCPlayerInfo(self)->layout, 0, playerInfoStrings + kPlayerInfo_status_outgame_spd);
    // Four additional UI state words start clear; their consumers are not identified here.
    AsCPlayerInfo(self)->unk_3C = 0;
    AsCPlayerInfo(self)->unk_40 = 0;
    AsCPlayerInfo(self)->unk_44 = 0;
    AsCPlayerInfo(self)->unk_48 = 0;
    EnsureBpCtrl((destructorRegistrations + kRegBpCtrl));
    CBpCtrl_CreateEntry((void*)((int&)Sing_805DC5F0()),playerInfoStrings + kPlayerInfo_status_outgame_b2d,&AsCPlayerInfo(self)->bpEntryId,0);
    EnsureBpCtrl((destructorRegistrations + kRegBpCtrl));
    AsCPlayerInfo(self)->bpObject=IdMap_Lookup((void*)((int&)Sing_805DC5F0()),AsCPlayerInfo(self)->bpEntryId);
    // Phase 2: construct owned normal/unavailable course textures and character textures.
    // new[] adds a 0x10-byte header; returned pointers address elements, not headers.
    {
        void* arr = 0;
        new PlayerInfoCourseArrayAllocation(arr);
        AsCPlayerInfo(self)->courseTextures = arr;
    }
    {
        void* arr = 0;
        new PlayerInfoCourseArrayAllocation(arr);
        AsCPlayerInfo(self)->unavailableCourseTextures = arr;
    }
    {
        void* arr = 0;
        new PlayerInfoCharacterArrayAllocation(arr);
        AsCPlayerInfo(self)->characterTextures = arr;
    }
    CourseTextureIds* srcA = (CourseTextureIds*)(levelData+0x598);
    CourseTextureIds* srcB = (CourseTextureIds*)(levelData+0x748);
    CourseTextureIds courseIds = *srcA;
    CourseTextureIds unavailableCourseIds = *srcB;
    int* courseId = courseIds.v;
    int textureOffset = 0;
    int* unavailableCourseId = unavailableCourseIds.v;
    int i = 0;
    do {
        ((PlayerInfoTexture*)((char*)AsCPlayerInfo(self)->courseTextures + textureOffset))->init(*courseId);
        ((PlayerInfoTexture*)((char*)AsCPlayerInfo(self)->unavailableCourseTextures + textureOffset))->init(*unavailableCourseId);
        ++i;
        textureOffset += 0x28;
        ++unavailableCourseId;
        ++courseId;
    } while (i<kPlayerInfoCourseCount);
    CharacterTextureIds* srcC = (CharacterTextureIds*)(levelData+0x81c);
    CharacterTextureIds characterIds = *srcC;
    i = 0;
    do { (((PlayerInfoTexture*)AsCPlayerInfo(self)->characterTextures + i))->init(characterIds.v[i]); } while (++i<kPlayerInfoCharacterCount);
    // Phase 3: allocate the swing controller and register the common background entry.
    obj = new PlayerInfoSwingAllocation;
    AsCPlayerInfo(self)->padSwingCtrl=obj;
    EnsureBpCtrl((destructorRegistrations + kRegBpCtrl));
    CBpCtrl_CreateEntry((void*)((int&)Sing_805DC5F0()),playerInfoStrings + kPlayerInfo_p2_common_bg_b2d,&AsCPlayerInfo(self)->pageEntryId,0);
    return self;
}

#pragma opt_common_subs reset
#pragma use_lmw_stmw reset
// Destroy the layout and owned UI resources, unregister text and restore the prior script index.
PlayerInfoOwnedResources* CPlayerInfo_dtor(PlayerInfoOwnedResources* self, int freeFlag) {
    // Positive freeFlag deletes self; all other values still release owned members.
    // Phase 1: destroy layout, unregister numeric/text draws and remove the primary BP entry.
    char* base = s_playerInfoDestructorRegistrations;
    if (self != 0) {
        self->vtable = (void**)__vt__11CPlayerInfo;
        if (self->layoutWord) {
            Layout_Destroy((void*)self->layoutWord, 1);
            self->layoutWord = 0;
        }
        UITextTable_ClearEntries((void*)self->numberDrawWord);
        if (self->numberDrawWord) {
            EnsureFontNumRegistry((base + kRegFontNumRegistry));
            PointerArray_DeleteValue((void*)((int&)Sing_805DC830()), self->numberDrawWord);
        }
        if (self->bpEntryId != -1) {
            EnsureBpCtrl((base + kRegBpCtrl));
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), self->bpEntryId);
            self->bpEntryId = -1;
        }
        EnsurePairList((base + kRegPairList));
        PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), (int)self->text);
        if (self->text) {
            if (self->text) {
                ((void (*)(void*, int))(*(void***)self->text)[2])((void*)self->text, 1);
            }
            self->text = 0;
        }
        // Phase 2: destroy the three binding arrays and swing controller, then empty name text.
        if (self->courseTextures) {
            __destroy_new_array(self->courseTextures, CBpTex_dtor);
            self->courseTextures = 0;
        }
        if (self->unavailableCourseTextures) {
            __destroy_new_array(self->unavailableCourseTextures, CBpTex_dtor);
            self->unavailableCourseTextures = 0;
        }
        if (self->characterTextures) {
            __destroy_new_array(self->characterTextures, CBpTex_dtor);
            self->characterTextures = 0;
        }
        if (self->padSwingCtrl) {
            if (self->padSwingCtrl) {
                ((void (*)(void*, int))(self->padSwingCtrl->vtable)[2])(self->padSwingCtrl, 1);
            }
            self->padSwingCtrl = 0;
        }
        unsigned char b = 0;
        WString_ReplaceRange((char*)self+kPlayerInfoNameOffset, 0, -1, 0, 0, &b);
        if (self->texturePack) {
            if (self->texturePack) {
                ((void (*)(void*, int))(*(void***)self->texturePack)[2])(self->texturePack, 1);
            }
            self->texturePack = 0;
        }
        // Phase 3: restore the global script index, remove the background entry,
        // and free both owned string buffers before optionally deleting self.
        EnsureBpCtrlHandle((base + kRegBpCtrlHandle));
        CBpCtrlHandle_SetScriptIndex((void*)((int&)Sing_805DC5F8()), self->previousScriptIndex);
        if (self->pageEntryId != -1) {
            EnsureBpCtrl((base + kRegBpCtrl));
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), self->pageEntryId);
            self->pageEntryId = -1;
        }
        if (&self->nameStringFlags && (self->nameStringFlags >> 31)) appFreeUni__FPv(self->nameStringData);
        if (&self->textStateFlags && (self->textStateFlags >> 31)) appFreeUni__FPv(self->textStateStorage);
        if (freeFlag > 0) __dl__FPv(self);
    }
    return self;
}

// Update the layout and poll the outstanding texture pack.
void CPlayerInfo_UpdateWhileWaiting(PlayerInfoLoadState* self) {
    Layout_Update(self->layout);
    if (self->current) CPackTextureResource_PollLoad(self->current);
}

// Wait until every icon texture, script entry and layout resource has loaded.
int CPlayerInfo_PollLoadComplete(PlayerInfoLoadState* self) {
    if (self->current && !CPackTextureResource_IsLoadFinished(self->current)) {
        return 0;
    }
    for (int i = 0; i < kPlayerInfoCourseCount; ++i) {
        PlayerInfoTexture* a = ((PlayerInfoTexture*)AsCPlayerInfo(self)->courseTextures + i);
        if (a->valid() != 0 && (((PlayerInfoTexture*)AsCPlayerInfo(self)->unavailableCourseTextures + i))->valid() != 0) {
        }
        else {
            return 0;
        }
    }
    for (int i = 0; i < kPlayerInfoCharacterCount; ++i) {
        PlayerInfoTexture* a = ((PlayerInfoTexture*)AsCPlayerInfo(self)->characterTextures + i);
        if (!a->valid()) return 0;
    }
    EnsureBpCtrl(&s_playerInfoBpCtrlDestructorRegistration);
    if (!CBpCtrl_PollAllEntriesLoaded((void*)((int&)Sing_805DC5F0()))) return 0;
    return Layout_PollLoadComplete(self->layout) != 0;
}

// Populate player name and rank, start the basic-info/background scripts and initialize the opening fade.
void CPlayerInfo_InitializePage(void* self) {
    // Phase 1: copy the save record, assemble the wide name, and select a rank icon.
    // Experience at +0x24 is converted to a tier for the rank icon.
    char* destructorRegistrations = s_playerInfoDestructorRegistrations;
    char* playerInfo = (char*)self;
    int rect[4];
    int v[4];
    float vec[4];
    char cbuf[16];
    LevelIconIds labels;
    PlayerInfoSaveRecord saveRecord;
    if (!((int&)Sing_805DC5E0())) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)Sing_805DC5E0()) = (int)s_saveData;
    }
    SaveRecord_CopyCtor(&saveRecord, SaveData_GetCurrentRecord((void*)((int&)Sing_805DC5E0())));
    unsigned int count;
    char* namePrefix = s_playerInfoNamePrefix;
    void* experienceBar;
    if ((*(unsigned int*)(playerInfo + kPlayerInfoNameOffset) >> 31) == 0) {
        count = AsCPlayerInfo(playerInfo)->nameStringFlags & 0x7f;
    }
    else {
        count = AsCPlayerInfo(playerInfo)->nameStringLength;
    }
    cbuf[8] = cbuf[12];
    int len = wcslen((const wchar_t*)namePrefix);
    WString_ReplaceRange(playerInfo + kPlayerInfoNameOffset, 0, count, namePrefix, namePrefix + len * 2, &cbuf[8]);
    if ((*(unsigned int*)(playerInfo + kPlayerInfoNameOffset) >> 31) == 0) {
        count = AsCPlayerInfo(playerInfo)->nameStringFlags & 0x7f;
    }
    else {
        count = AsCPlayerInfo(playerInfo)->nameStringLength;
    }
    cbuf[0] = cbuf[4];
    len = wcslen((const wchar_t*)&saveRecord);
    WString_ReplaceRange(playerInfo + kPlayerInfoNameOffset, count, 0, (char*)&saveRecord, (char*)&saveRecord + len * 2, &cbuf[0]);
    v[2] = 0;
    v[3] = 0;
    Layout_GetElementPosition(AsCPlayerInfo(playerInfo)->layout, kPlayerInfoNameRect, v + 2);
    Text_SetPosition(AsCPlayerInfo(playerInfo)->text, v + 2);
    v[0] = 0;
    v[1] = 0;
    Layout_GetElementSize(AsCPlayerInfo(playerInfo)->layout, kPlayerInfoNameRect, v);
    CMsgDraw_SetLayoutSize(AsCPlayerInfo(playerInfo)->text, v);
    int levelTier = PlayerRank_FindTierForValue(saveRecord.experience);
    switch (levelTier) {
    case 0 ... 5: AsCPlayerInfo(playerInfo)->levelIconIndex = 0;
        break;
    case 6 ... 0xa: AsCPlayerInfo(playerInfo)->levelIconIndex = 1;
        break;
    case 0xb ... 0xf: AsCPlayerInfo(playerInfo)->levelIconIndex = 2;
        break;
    case 0x10 ... 0x14: AsCPlayerInfo(playerInfo)->levelIconIndex = 3;
        break;
    case 0x15 ... 0x19: AsCPlayerInfo(playerInfo)->levelIconIndex = 4;
        break;
    case 0x1a ... 0x1e: AsCPlayerInfo(playerInfo)->levelIconIndex = 5;
        break;
    case 0x1f ... 0x23: AsCPlayerInfo(playerInfo)->levelIconIndex = 6;
        break;
    case 0x24 ... 0x28: AsCPlayerInfo(playerInfo)->levelIconIndex = 7;
        break;
    case 0x29 ... 0x2d: AsCPlayerInfo(playerInfo)->levelIconIndex = 8;
        break;
    case 0x2e ... 0x32: AsCPlayerInfo(playerInfo)->levelIconIndex = 9;
        break;
    case 0x33 ... 0x37: AsCPlayerInfo(playerInfo)->levelIconIndex = 10;
        break;
    }
    labels = *(LevelIconIds*)s_playerInfoLevelIconIds;
    for (int i = 0; i <= 10; i++) {
        void* elem = UIGroup_GetOrCreateById(AsCPlayerInfo(playerInfo)->bpObject, labels.v[i]);
        if (AsCPlayerInfo(playerInfo)->levelIconIndex == i) {
            UIGroup_SetEnabled(elem, 1);
        }
        else {
            UIGroup_SetEnabled(elem, 0);
        }
    }
    // Phase 1b: disable layout elements 0x21..0x3e one by one, then 0x3f..0x47 in a loop (the straight-line calls are
    // kept: a loop here changes the generated code). Their individual roles are not identified; all start hidden.
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, kPlayerInfoNameRect, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x21, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x22, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x23, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x24, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x25, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x26, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x27, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x28, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x29, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x2a, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x2b, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x2c, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x2d, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x2e, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x2f, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x30, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x31, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x32, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x33, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x34, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x35, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x36, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x37, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x38, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x39, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x3a, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x3b, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x3c, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x3d, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, 0x3e, 0);
    for (int i = kPlayerInfoRecordPlaceholderFirst; i <= kPlayerInfoRecordPlaceholderLast; i++)
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, i, 0);
    // Phase 2: hide anchor controls, register numeric entries and pair hit rectangles.
    CFontNumDraw_RegisterEntries(AsCPlayerInfo(playerInfo)->numberDraw, s_playerInfoNumericTextDefinitions, 0x18, AsCPlayerInfo(playerInfo)->bpObject, playerInfoStrings + kPlayerInfo_num_status_01_gsr);
    UITextTable_SetEntryResourceName(AsCPlayerInfo(playerInfo)->numberDraw, 0xb7, playerInfoStrings + kPlayerInfo_num_status_00_gsr);
    UITextTable_SetEntryResourceName(AsCPlayerInfo(playerInfo)->numberDraw, 0xb8, playerInfoStrings + kPlayerInfo_num_status_00_gsr);
    Layout_ReadElementRect(AsCPlayerInfo(playerInfo)->layout, 0xc, rect);
    Layout_WriteElementRect(AsCPlayerInfo(playerInfo)->layout, 0x16, rect);
    Layout_ReadElementRect(AsCPlayerInfo(playerInfo)->layout, 0xd, rect);
    Layout_WriteElementRect(AsCPlayerInfo(playerInfo)->layout, 0x17, rect);
    Layout_ReadElementRect(AsCPlayerInfo(playerInfo)->layout, 0xe, rect);
    Layout_WriteElementRect(AsCPlayerInfo(playerInfo)->layout, 0x18, rect);
    Layout_ReadElementRect(AsCPlayerInfo(playerInfo)->layout, 0xf, rect);
    Layout_WriteElementRect(AsCPlayerInfo(playerInfo)->layout, 0x19, rect);
    Layout_ReadElementRect(AsCPlayerInfo(playerInfo)->layout, 0x10, rect);
    Layout_WriteElementRect(AsCPlayerInfo(playerInfo)->layout, 0x1a, rect);
    Layout_ReadElementRect(AsCPlayerInfo(playerInfo)->layout, 0, rect);
    Layout_WriteElementRect(AsCPlayerInfo(playerInfo)->layout, 2, rect);
    Layout_ReadElementRect(AsCPlayerInfo(playerInfo)->layout, 1, rect);
    Layout_WriteElementRect(AsCPlayerInfo(playerInfo)->layout, 3, rect);
    Sound_PlayByName(g_soundManager, playerInfoStrings + kPlayerInfo_ui_dialog_open, 0, 0, 0, 0, kFloat_0_5_PlayerInfoSound, kFloat_200_StatisticsSound, kFloat_1_PlayerInfo);
    BpObject_StartScriptByName(AsCPlayerInfo(playerInfo)->bpObject, playerInfoStrings + kPlayerInfo_basic_info, 1);
    // Phase 3: compute experience within the selected tier and transform the bar.
    // Keep this initialization hook's 0x410A terminal fallback distinct from the basic
    // statistics page's 3500 fallback; these are existing different behaviors.
    int experience = saveRecord.experience;
    float experienceProgress = (float)experience;
    float tierExperience = kFloat_0_Statistics;
    int i;
    for (i = 0; i < 0x38; i++) {
        if ((unsigned int)experience <= playerLevelDefinitions[i].end) {
            if (i == 0) {
                tierExperience = (float)playerLevelDefinitions[i].end;
            }
            else {
                PlayerLevelDefinition* e = &playerLevelDefinitions[i];
                unsigned int prev = *(unsigned int*)((char*)e - 4);
                tierExperience = (float)(e->end - prev);
                experienceProgress = experienceProgress - (float)prev;
            }
            break;
        }
    }
    if (i == 0x38) {
        unsigned int maximumExperience = 0x410a;
        tierExperience = (float)maximumExperience;
        experienceProgress = tierExperience;
    }
    experienceBar = BpObject_GetOrCreateNamedGroup(AsCPlayerInfo(playerInfo)->bpObject, playerInfoStrings + kPlayerInfo_R_status_expgraph);
    *(int*)&vec[2] = kPlayerInfoExperienceBarZeroBits;
    *(int*)&vec[3] = kPlayerInfoExperienceBarOneBits;
    UIGroup_GetPosition(experienceBar, vec);
    float ratio = experienceProgress / tierExperience;
    float barScaleX = kPlayerInfoExperienceBarScaleFactor - kPlayerInfoExperienceBarScaleFactor * ratio;
    float barPositionOffsetX = kPlayerInfoExperienceBarPositionFactor * barScaleX - kPlayerInfoExperienceBarPositionFactor;
    vec[2] = barScaleX;
    vec[0] = vec[0] - barPositionOffsetX;
    UIGroup_SetPosition(experienceBar, vec);
    UIGroup_SetScale(experienceBar, vec + 2);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCPlayerInfo(playerInfo)->bpObject, playerInfoStrings + kPlayerInfo_R_status_hyphen1), 0);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCPlayerInfo(playerInfo)->bpObject, playerInfoStrings + kPlayerInfo_R_status_hyphen2), 0);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCPlayerInfo(playerInfo)->bpObject, playerInfoStrings + kPlayerInfo_R_status_hyphen3), 0);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCPlayerInfo(playerInfo)->bpObject, playerInfoStrings + kPlayerInfo_R_status_hyphen4), 0);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCPlayerInfo(playerInfo)->bpObject, playerInfoStrings + kPlayerInfo_R_status_hyphen5), 0);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCPlayerInfo(playerInfo)->bpObject, playerInfoStrings + kPlayerInfo_R_status_hyphen6), 0);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCPlayerInfo(playerInfo)->bpObject, playerInfoStrings + kPlayerInfo_R_status_hyphen7), 0);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCPlayerInfo(playerInfo)->bpObject, playerInfoStrings + kPlayerInfo_R_status_hyphen8), 0);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(AsCPlayerInfo(playerInfo)->bpObject, playerInfoStrings + kPlayerInfo_R_status_hyphen9), 0);
    EnsureBpCtrlHandle((destructorRegistrations + kRegBpCtrlHandle));
    // Phase 4: save/override the shared script index, start the background and fade,
    // then release the staging texture pack once the individual bindings are ready.
    AsCPlayerInfo(playerInfo)->previousScriptIndex = ((int*)Sing_805DC5F8())[1];
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (destructorRegistrations + kRegBpCtrlHandle));
            s_bpCtrlHandleInitialized = 1;
        }
        ((int&)Sing_805DC5F8()) = (int)s_bpCtrlHandle;
    }
    CBpCtrlHandle_SetScriptIndex((void*)((int&)Sing_805DC5F8()), 0xf);
    EnsureBpCtrl((destructorRegistrations + kRegBpCtrl));
    void* page = (void*)IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsCPlayerInfo(playerInfo)->pageEntryId);
    BpObject_StartScriptByName(page, playerInfoStrings + kPlayerInfo_bg, 1);
    BpObject_TryInvokeCurrentCallback(page, 0);
    if (AsCPlayerInfo(playerInfo)->texturePack) {
        if (AsCPlayerInfo(playerInfo)->texturePack) {
            ((*(TexturePackInterface**)&AsCPlayerInfo(playerInfo)->texturePack))->release(1);
        }
        AsCPlayerInfo(playerInfo)->texturePack = 0;
    }
    AsCPlayerInfo(playerInfo)->pageInitialized = 1;
    Layout_SetElementPlayerFlagsAndState(AsCPlayerInfo(playerInfo)->layout, kPlayerInfoScrollLower, 0);
    Layout_SetElementPlayerFlagsAndState(AsCPlayerInfo(playerInfo)->layout, kPlayerInfoScrollHigher, 0);
    Layout_SetElementPlayerFlagsAndState(AsCPlayerInfo(playerInfo)->layout, kPlayerInfoPairedScrollLower, 0);
    Layout_SetElementPlayerFlagsAndState(AsCPlayerInfo(playerInfo)->layout, kPlayerInfoPairedScrollHigher, 0);
    // Hide layout elements 4..0xb and 0x3f..0x47 (straight-line calls kept; a loop changes the generated code).
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, kPlayerInfoLowerMotion, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, kPlayerInfoHigherMotion, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, kPlayerInfoPairedLowerMotion, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, kPlayerInfoPairedHigherMotion, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, kPlayerInfoLowerEnd, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, kPlayerInfoHigherEnd, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, kPlayerInfoPairedLowerEnd, 0);
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, kPlayerInfoPairedHigherEnd, 0);
    for (int j = kPlayerInfoRecordPlaceholderFirst; j <= kPlayerInfoRecordPlaceholderLast; j++)
    Layout_SetElementEnabled(AsCPlayerInfo(playerInfo)->layout, j, 0);
    CPlayerInfo_SetModeButtonStates(playerInfo, AsCPlayerInfo(playerInfo)->page);
    EnsureFadeOverlay((destructorRegistrations + kRegFadeOverlay));
    FadeState_StartFadeOut((void*)((int&)Sing_805DC608()), 0xf);
}

#pragma opt_common_subs off
#pragma opt_strength_reduction off
// Hide both rows of course-scroll controls when a non-course page opens.
static inline void HidePlayerInfoCourseScrollControls(PlayerInfoProcessState* self) {
    Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoScrollLower, 0);
    Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoScrollHigher, 0);
    Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedScrollLower, 0);
    Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedScrollHigher, 0);
    Layout_SetElementEnabled(self->layout, kPlayerInfoLowerMotion, 0);
    Layout_SetElementEnabled(self->layout, kPlayerInfoHigherMotion, 0);
    Layout_SetElementEnabled(self->layout, kPlayerInfoPairedLowerMotion, 0);
    Layout_SetElementEnabled(self->layout, kPlayerInfoPairedHigherMotion, 0);
    Layout_SetElementEnabled(self->layout, kPlayerInfoLowerEnd, 0);
    Layout_SetElementEnabled(self->layout, kPlayerInfoHigherEnd, 0);
    Layout_SetElementEnabled(self->layout, kPlayerInfoPairedLowerEnd, 0);
    Layout_SetElementEnabled(self->layout, kPlayerInfoPairedHigherEnd, 0);
}

// Process page changes, closing and course-record scrolling after the current script/fade finishes.
// Start the close sound/fade and latch closing; retain the caller's event-vector lifetime.
static inline void ClosePlayerInfo(PlayerInfoProcessState* self) {
    Sound_PlayByName(g_soundManager, playerInfoStrings + kPlayerInfo_ui_dialog_close, 0, 0, 0, 0,kFloat_0_5_PlayerInfoSound,kFloat_200_StatisticsSound,kFloat_1_PlayerInfo);
    void* fadeState = (void*)FadeOverlay_GetInstance();
    FadeState_RequestFramebufferCapture(fadeState);
    FadeState_StartFadeIn((void*)FadeOverlay_GetInstance(), 0);
    self->closing = 1;
}

// Set end-cap triplets and matching shadow triplets in the original layout order.
static inline void SetPlayerInfoScrollDecorations(PlayerInfoProcessState* self, int upperEndEnabled,
        int lowerEndEnabled, int upperShadowEnabled, int lowerShadowEnabled) {
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->bpObject, playerInfoStrings + kPlayerInfo_R_status_scroll_upendl), upperEndEnabled);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->bpObject, playerInfoStrings + kPlayerInfo_R_status_scroll_upendc), upperEndEnabled);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->bpObject, playerInfoStrings + kPlayerInfo_R_status_scroll_upendr), upperEndEnabled);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->bpObject, playerInfoStrings + kPlayerInfo_R_status_scroll_dendl), lowerEndEnabled);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->bpObject, playerInfoStrings + kPlayerInfo_R_status_scroll_dendc), lowerEndEnabled);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->bpObject, playerInfoStrings + kPlayerInfo_R_status_scroll_dendr), lowerEndEnabled);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->bpObject, playerInfoStrings + kPlayerInfo_R_status_shadow_up), upperShadowEnabled);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->bpObject, playerInfoStrings + kPlayerInfo_R_status_scrollb_upl), upperShadowEnabled);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->bpObject, playerInfoStrings + kPlayerInfo_R_status_scrollb_upc), upperShadowEnabled);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->bpObject, playerInfoStrings + kPlayerInfo_R_status_scrollb_upr), upperShadowEnabled);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->bpObject, playerInfoStrings + kPlayerInfo_R_status_shadow_down), lowerShadowEnabled);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->bpObject, playerInfoStrings + kPlayerInfo_R_status_scrollb_downl), lowerShadowEnabled);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->bpObject, playerInfoStrings + kPlayerInfo_R_status_scrollb_downc), lowerShadowEnabled);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->bpObject, playerInfoStrings + kPlayerInfo_R_status_scrollb_downr), lowerShadowEnabled);
}

// Keep helpers inline without changing the original event-vector and scratch-index lifetimes.
#pragma inline_depth(16)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(10000)
// Refresh course-window end caps, shadows, and settled controls after event handling.
static inline void RefreshPlayerInfoCourseScroll(PlayerInfoProcessState* self) {
    // Phase 3 (course page): choose end caps/shadows from the clamped
    // window origin. Motion visuals remain active until the four-call gate ends.
    if (self->courseRecordIndex == 0) {
        if (self->scrollDelayActive == 0) {
                Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoScrollLower, 0);
                Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoScrollHigher, 1);
                Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedScrollLower, 0);
                Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedScrollHigher, 1);
                Layout_SetElementEnabled(self->layout, kPlayerInfoLowerMotion, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoHigherMotion, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoPairedLowerMotion, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoPairedHigherMotion, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoLowerEnd, 1);
                Layout_SetElementEnabled(self->layout, kPlayerInfoHigherEnd, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoPairedLowerEnd, 1);
                Layout_SetElementEnabled(self->layout, kPlayerInfoPairedHigherEnd, 0);
        }
        SetPlayerInfoScrollDecorations(self, 1, 0, 0, 1);
    } else if (self->courseRecordIndex == kPlayerInfoLastRecordWindow) {
        if (self->scrollDelayActive == 0) {
                Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoScrollLower, 1);
                Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoScrollHigher, 0);
                Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedScrollLower, 1);
                Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedScrollHigher, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoLowerMotion, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoHigherMotion, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoPairedLowerMotion, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoPairedHigherMotion, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoLowerEnd, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoHigherEnd, 1);
                Layout_SetElementEnabled(self->layout, kPlayerInfoPairedLowerEnd, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoPairedHigherEnd, 1);
        }
        SetPlayerInfoScrollDecorations(self, 0, 1, 1, 0);
    } else {
        if (self->scrollDelayActive == 0) {
                Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoScrollLower, 1);
                Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoScrollHigher, 1);
                Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedScrollLower, 1);
                Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedScrollHigher, 1);
                Layout_SetElementEnabled(self->layout, kPlayerInfoLowerMotion, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoHigherMotion, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoPairedLowerMotion, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoPairedHigherMotion, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoLowerEnd, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoHigherEnd, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoPairedLowerEnd, 0);
                Layout_SetElementEnabled(self->layout, kPlayerInfoPairedHigherEnd, 0);
        }
        SetPlayerInfoScrollDecorations(self, 0, 0, 1, 1);
    }
}

// Move the three-course window one record lower and start its four-update input gate.
static inline void ScrollPlayerInfoCourseLower(PlayerInfoProcessState* self) {
    if (self->courseRecordIndex > 0) {
        if (self->courseRecordIndex == kPlayerInfoLastRecordWindow) {
        Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoHigherEnd, 0);
        Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedHigherEnd, 0);
        }
        Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoScrollLower, 0);
        Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoScrollHigher, 1);
        Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedScrollLower, 0);
        Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedScrollHigher, 1);
        Layout_SetElementEnabled(self->layout, kPlayerInfoLowerMotion, 1);
        Layout_SetElementEnabled(self->layout, kPlayerInfoHigherMotion, 0);
        Layout_SetElementEnabled(self->layout, kPlayerInfoPairedLowerMotion, 1);
        Layout_SetElementEnabled(self->layout, kPlayerInfoPairedHigherMotion, 0);
        self->scrollDelayActive = 1;
        self->courseRecordIndex--;
        self->scrollDelayFrames = kPlayerInfoScrollDelayCalls;
    }
}

// Move the three-course window one record higher and start its four-update input gate.
static inline void ScrollPlayerInfoCourseHigher(PlayerInfoProcessState* self) {
    if (self->courseRecordIndex < kPlayerInfoLastRecordWindow) {
        if (self->courseRecordIndex == 0) {
            Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoLowerEnd, 0);
            Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedLowerEnd, 0);
        }
        Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoScrollLower, 1);
        Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoScrollHigher, 0);
        Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedScrollLower, 1);
        Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedScrollHigher, 0);
        Layout_SetElementEnabled(self->layout, kPlayerInfoLowerMotion, 0);
        Layout_SetElementEnabled(self->layout, kPlayerInfoHigherMotion, 1);
        Layout_SetElementEnabled(self->layout, kPlayerInfoPairedLowerMotion, 0);
        Layout_SetElementEnabled(self->layout, kPlayerInfoPairedHigherMotion, 1);
        self->scrollDelayActive = 1;
        self->courseRecordIndex++;
        self->scrollDelayFrames = kPlayerInfoScrollDelayCalls;
    }
}

// Apply basic-page button activations in order; borrow the caller-owned event vector.
// activationIndex references caller scratch to retain CodeWarrior register allocation; no ownership transfer.
static inline void DispatchPlayerInfoBasicElements(PlayerInfoProcessState* self, IntVector* activatedElements, int& activationIndex) {
    for (; (unsigned int)activationIndex < (unsigned int)Vector_GetCount(activatedElements); activationIndex++) {
        void* elementWord = PlayerInfoWordVector_At(activatedElements, activationIndex);
        int elementId = *(int*)elementWord;
        switch (elementId) {
        case kPlayerInfoShotButton: {
            self->requestedPage = kPlayerInfoShotPage;
            self->basicPageInitialized = 0;
            break;
        }
        case kPlayerInfoPersonalButton: {
            self->requestedPage = kPlayerInfoPersonalPage;
            self->basicPageInitialized = 0;
            break;
        }
        case kPlayerInfoCourseButton: {
            self->requestedPage = kPlayerInfoCoursePage;
            self->basicPageInitialized = 0;
            break;
        }
        case kPlayerInfoCloseButton: {
            ClosePlayerInfo(self);
        }
        }
    }
}

// Apply shot-page button activations in order; borrow the caller-owned event vector.
static inline void DispatchPlayerInfoShotElements(PlayerInfoProcessState* self, IntVector* activatedElements, int& activationIndex) {
    for (; (unsigned int)activationIndex < (unsigned int)Vector_GetCount(activatedElements); activationIndex++) {
        void* elementWord = PlayerInfoWordVector_At(activatedElements, activationIndex);
        int elementId = *(int*)elementWord;
        switch (elementId) {
        case kPlayerInfoBasicButton: {
            self->requestedPage = kPlayerInfoBasicPage;
            self->shotPageInitialized = 0;
            break;
        }
        case kPlayerInfoPersonalButton: {
            self->requestedPage = kPlayerInfoPersonalPage;
            self->shotPageInitialized = 0;
            break;
        }
        case kPlayerInfoCourseButton: {
            self->requestedPage = kPlayerInfoCoursePage;
            self->shotPageInitialized = 0;
            break;
        }
        case kPlayerInfoCloseButton: {
            ClosePlayerInfo(self);
        }
        }
    }
}

// Apply personal-page button activations in order; borrow the caller-owned event vector.
static inline void DispatchPlayerInfoPersonalElements(PlayerInfoProcessState* self, IntVector* activatedElements, int& activationIndex) {
    for (; (unsigned int)activationIndex < (unsigned int)Vector_GetCount(activatedElements); activationIndex++) {
        void* elementWord = PlayerInfoWordVector_At(activatedElements, activationIndex);
        int elementId = *(int*)elementWord;
        switch (elementId) {
        case kPlayerInfoBasicButton: {
            self->requestedPage = kPlayerInfoBasicPage;
            self->personalPageInitialized = 0;
            break;
        }
        case kPlayerInfoShotButton: {
            self->requestedPage = kPlayerInfoShotPage;
            self->personalPageInitialized = 0;
            break;
        }
        case kPlayerInfoCourseButton: {
            self->requestedPage = kPlayerInfoCoursePage;
            self->personalPageInitialized = 0;
            break;
        }
        case kPlayerInfoCloseButton: {
            ClosePlayerInfo(self);
        }
        }
    }
}

// Apply course-page button activations in order; borrow the caller-owned event vector.
static inline void DispatchPlayerInfoCourseElements(PlayerInfoProcessState* self, IntVector* activatedElements, int& activationIndex) {
    for (; (unsigned int)activationIndex < (unsigned int)Vector_GetCount(activatedElements); activationIndex++) {
        void* elementWord = PlayerInfoWordVector_At(activatedElements, activationIndex);
        switch (*(int*)elementWord) {
        case kPlayerInfoBasicButton:
            self->requestedPage = kPlayerInfoBasicPage;
            self->coursePageInitialized = 0;
            break;
        case kPlayerInfoShotButton:
            self->requestedPage = kPlayerInfoShotPage;
            self->coursePageInitialized = 0;
            break;
        case kPlayerInfoPersonalButton:
            self->requestedPage = kPlayerInfoPersonalPage;
            self->coursePageInitialized = 0;
            break;
        case kPlayerInfoCloseButton:
            ClosePlayerInfo(self);
            break;
        case kPlayerInfoScrollLower:
            ScrollPlayerInfoCourseLower(self);
            break;
        case kPlayerInfoScrollHigher:
            ScrollPlayerInfoCourseHigher(self);
            break;
        }
    }
}

// Process gated input, dispatch page events, and refresh course-scroll presentation.
int CPlayerInfo_UpdatePageInputAndDispatch(PlayerInfoProcessState* self) {
    // Returns 1 only once a closing script is complete; page/scroll updates return 0.
    // Phase 1: gate input on closing, overlay fade, current script and scroll delay.
    int finished;
    Layout_Update(self->layout);
    finished = 0;
    if (self->closing != 0) {
        if (BpObject_IsCurrentScriptComplete(self->bpObject) != 0) finished = 1;
        return finished;
    }
    void* fadeState = (void*)FadeOverlay_GetInstance();
    if (FadeState_GetState(fadeState) != 0) {
        return 0;
    }
    if (BpObject_IsCurrentScriptComplete(self->bpObject) == 0) return 0;
    if (self->scrollDelayActive != 0) {
        if (--self->scrollDelayFrames == 0) self->scrollDelayActive = 0;
        else return 0;
    }
    if (self->page != self->requestedPage) self->page = self->requestedPage;
    // Phase 2: initialize each page once, consume layout events (or synthesize
    // button IDs from actions 2/3/6 = previous/next/close), then request the next
    // page. requestedPage becomes page on the following ungated update.
    int page = self->page;
    switch (page) {
    case kPlayerInfoBasicPage: {
        // Basic page: one-time setup, ordered input synthesis, then event dispatch.
        if (self->opened == 0) self->opened = 1;
        if (self->basicPageInitialized == 0) {
            self->basicPageInitialized = 1;
            BpObject_StartScriptByName(self->bpObject, playerInfoStrings + kPlayerInfo_basic_info, 1);
            HidePlayerInfoCourseScrollControls(self);
            for (int i = kPlayerInfoRecordPlaceholderFirst; i <= kPlayerInfoRecordPlaceholderLast; i++) Layout_SetElementEnabled(self->layout, i, 0);
        }
        if (BpObject_IsCurrentScriptComplete(self->bpObject) != 0) {
            IntVector activatedElements;
            Layout_CollectActivated(self->layout, &activatedElements, 0);
            if (Vector_GetCount(&activatedElements) == 0) {
                if (Input_IsActionPressed(LayoutManager_GetInstance(), kPlayerInfoNextAction, -1) != 0) {
                    LayoutIntVector_PushBack(&activatedElements, (const char*)&s_playerInfoBasicNextElement);
                    Sound_PlayByName(g_soundManager, playerInfoStrings + kPlayerInfo_ui_icon_click, 0, 0, 0, 0,kFloat_0_5_PlayerInfoSound,kFloat_200_StatisticsSound,kFloat_1_PlayerInfo);
                }
                else {
                    if (Input_IsActionPressed(LayoutManager_GetInstance(), kPlayerInfoCloseAction, -1) != 0) {
                        LayoutIntVector_PushBack(&activatedElements, (const char*)&s_playerInfoBasicCloseElement);
                    }
                }
            }
            int activationIndex = 0;
            DispatchPlayerInfoBasicElements(self, &activatedElements, activationIndex);
        }

            break;
    }

    case kPlayerInfoShotPage: {
        // Shot page: one-time setup, ordered input synthesis, then event dispatch.
        if (self->shotPageInitialized == 0) {
            self->shotPageInitialized = 1;
            BpObject_StartScriptByName(self->bpObject, playerInfoStrings + kPlayerInfo_shot_info, 1);
            HidePlayerInfoCourseScrollControls(self);
            for (int i = kPlayerInfoRecordPlaceholderFirst; i <= kPlayerInfoRecordPlaceholderLast; i++) Layout_SetElementEnabled(self->layout, i, 0);
        }
        if (BpObject_IsCurrentScriptComplete(self->bpObject) != 0) {
            IntVector activatedElements;
            Layout_CollectActivated(self->layout, &activatedElements, 0);
            if (Vector_GetCount(&activatedElements) == 0) {
                if (Input_IsActionPressed(LayoutManager_GetInstance(), kPlayerInfoNextAction, -1) != 0) {
                    LayoutIntVector_PushBack(&activatedElements, (const char*)&s_playerInfoShotNextElement);
                    Sound_PlayByName(g_soundManager, playerInfoStrings + kPlayerInfo_ui_icon_click, 0, 0, 0, 0,kFloat_0_5_PlayerInfoSound,kFloat_200_StatisticsSound,kFloat_1_PlayerInfo);
                }
                else {
                    if (Input_IsActionPressed(LayoutManager_GetInstance(), kPlayerInfoPreviousAction, -1) != 0) {
                        LayoutIntVector_PushBack(&activatedElements, (const char*)&s_playerInfoShotPreviousElement);
                        Sound_PlayByName(g_soundManager, playerInfoStrings + kPlayerInfo_ui_icon_click, 0, 0, 0, 0,kFloat_0_5_PlayerInfoSound,kFloat_200_StatisticsSound,kFloat_1_PlayerInfo);
                    }
                    else {
                        if (Input_IsActionPressed(LayoutManager_GetInstance(), kPlayerInfoCloseAction, -1) != 0) {
                            LayoutIntVector_PushBack(&activatedElements, (const char*)&s_playerInfoShotCloseElement);
                        }
                    }
                }
            }
            int activationIndex = 0;
            DispatchPlayerInfoShotElements(self, &activatedElements, activationIndex);
        }

            break;
    }

    case kPlayerInfoPersonalPage: {
        // Personal page: one-time setup, ordered input synthesis, then event dispatch.
        if (self->personalPageInitialized == 0) {
            self->personalPageInitialized = 1;
            BpObject_StartScriptByName(self->bpObject, playerInfoStrings + kPlayerInfo_personal_record, 1);
            HidePlayerInfoCourseScrollControls(self);
            for (int i = kPlayerInfoRecordPlaceholderFirst; i <= kPlayerInfoRecordPlaceholderLast; i++) Layout_SetElementEnabled(self->layout, i, 0);
        }
        if (BpObject_IsCurrentScriptComplete(self->bpObject) != 0) {
            IntVector activatedElements;
            Layout_CollectActivated(self->layout, &activatedElements, 0);
            if (Vector_GetCount(&activatedElements) == 0) {
                if (Input_IsActionPressed(LayoutManager_GetInstance(), kPlayerInfoNextAction, -1) != 0) {
                    LayoutIntVector_PushBack(&activatedElements, (const char*)&s_playerInfoPersonalNextElement);
                    Sound_PlayByName(g_soundManager, playerInfoStrings + kPlayerInfo_ui_icon_click, 0, 0, 0, 0,kFloat_0_5_PlayerInfoSound,kFloat_200_StatisticsSound,kFloat_1_PlayerInfo);
                }
                else {
                    if (Input_IsActionPressed(LayoutManager_GetInstance(), kPlayerInfoPreviousAction, -1) != 0) {
                        LayoutIntVector_PushBack(&activatedElements, (const char*)&s_playerInfoPersonalPreviousElement);
                        Sound_PlayByName(g_soundManager, playerInfoStrings + kPlayerInfo_ui_icon_click, 0, 0, 0, 0,kFloat_0_5_PlayerInfoSound,kFloat_200_StatisticsSound,kFloat_1_PlayerInfo);
                    }
                    else {
                        if (Input_IsActionPressed(LayoutManager_GetInstance(), kPlayerInfoCloseAction, -1) != 0) {
                            LayoutIntVector_PushBack(&activatedElements, (const char*)&s_playerInfoPersonalCloseElement);
                        }
                    }
                }
            }
            int activationIndex = 0;
            DispatchPlayerInfoPersonalElements(self, &activatedElements, activationIndex);
        }

            break;
    }

    case kPlayerInfoCoursePage: {
        // Course page: one-time setup, ordered input synthesis, then event dispatch.
        if (self->coursePageInitialized == 0) {
            self->coursePageInitialized = 1;
            BpObject_StartScriptByName(self->bpObject, playerInfoStrings + kPlayerInfo_course_record, 1);
        }
        if (BpObject_IsCurrentScriptComplete(self->bpObject) != 0) {
            IntVector activatedElements;
            Layout_CollectActivated(self->layout, &activatedElements, 0);
            if (Vector_GetCount(&activatedElements) == 0) {
                if (Input_IsActionPressed(LayoutManager_GetInstance(), kPlayerInfoPreviousAction, -1) != 0) {
                    LayoutIntVector_PushBack(&activatedElements, (const char*)&s_playerInfoCoursePreviousElement);
                    Sound_PlayByName(g_soundManager, playerInfoStrings + kPlayerInfo_ui_icon_click, 0, 0, 0, 0,kFloat_0_5_PlayerInfoSound,kFloat_200_StatisticsSound,kFloat_1_PlayerInfo);
                }
                else {
                    if (Input_IsActionPressed(LayoutManager_GetInstance(), kPlayerInfoCloseAction, -1) != 0) {
                        LayoutIntVector_PushBack(&activatedElements, (const char*)&s_playerInfoCourseCloseElement);
                    }
                }
            }
            int activationIndex = 0;
            DispatchPlayerInfoCourseElements(self, &activatedElements, activationIndex);
            RefreshPlayerInfoCourseScroll(self);
        }
            break;
    }

    }
    int selectedPage = self->page;
    CPlayerInfo_SetModeButtonStates(self, selectedPage);
    return 0;
}

#pragma opt_common_subs reset
#pragma opt_strength_reduction reset
#pragma inline_depth(1)
#pragma auto_inline off
#pragma optimize_for_size off
#pragma use_lmw_stmw on
// Draw the background, statistics text, name and selected course/character record icons.
void CPlayerInfo_Draw(PlayerInfoDrawState* self) {
    // Text color repacks the style's tint bytes and converts 7-bit opacity to
    // alpha using 0 -> 0, n -> 2*n-1. Icon rects are screen-space corners
    // (left,top,right,bottom); the four vertices retain template Z/color/UV words.
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl,(void*)CBpCtrl_dtor,s_playerInfoBpCtrlDestructorRegistration);
            s_bpCtrlInitialized=1;
        }
        Sing_805DC5F0()=s_bpCtrl;
    }
    {
        void* panel=IdMap_Lookup(Sing_805DC5F0(),self->id);
        BpObject_TryInvokeCurrentCallback(panel,0);
        BpObject_Draw(panel);
    }
    BpObject_TryInvokeCurrentCallback(self->panel,0);
    CPlayerInfo_PopulateStatisticsPage(self,self->page);
    CMsgDraw_ResetTextState(self->text);
    CMsgDraw_ClearFormattingAndRevealAll(self->text);
    void* style=BpObject_GetOrCreateNamedGroup(self->panel,playerInfoStrings + kPlayerInfo_R_MSG_playername_LC);
    unsigned int color=UIGroup_GetTintColor(style);
    int flag=UIGroup_GetCombinedOpacity(style);
    int alpha=flag ? flag*2-1 : 0;
    (*(unsigned int*)&((CMsgDraw*)self->text)->colorMultiplier)=alpha | ((color<<24)|(color<<8 & 0xff0000)|(color>>8 & 0xff00));
    Text_SetContent(self->text, !(self->stringFlag>>31) ? (char*)self+kPlayerInfoNameInlineOffset : self->stringPtr);
    BpObject_Draw(self->panel);
    Layout_DrawWithOpacity(self->layout,kFloat_1_PlayerInfo);
    if (self->page==3) {
        int index=self->courseRecordIndex;
        if (!Sing_805DC5E0()) {
            if (!s_saveDataInitialized) {
                SaveData_Ctor(s_saveData);
                s_saveDataInitialized=1;
            }
            Sing_805DC5E0()=s_saveData;
        }
        SaveData_GetCurrentRecord(Sing_805DC5E0());
        for (int i=0;i<kPlayerInfoRecordWindow;++i) {
            if (self->items[index+i].valid()) {
                self->items[index+i].apply();
                int rect[4];
                Layout_ReadElementRect(self->layout,i+0x39,rect);
                Quad quad=s_playerInfoCourseIconQuad;
                quad.v[0].x=rect[0];
                quad.v[0].y=rect[1];
                quad.v[1].x=rect[2];
                quad.v[1].y=rect[1];
                quad.v[2].x=rect[0];
                quad.v[2].y=rect[3];
                quad.v[3].x=rect[2];
                quad.v[3].y=rect[3];
                int count=GX_GetPrimitiveCount(4,0x98);
                DrawPrimitiveUP(0x98,count,&quad,0x18);
            }
        }
        CPlayerInfo_DrawRecordIcons((PlayerInfoIconOwner*)self,self->courseRecordIndex);
    }
    UITextTable_Draw(self->numberDraw);
    CMsgDraw_UpdateReveal(self->text);
    Text_DrawGlyphs(self->text);
}

} // extern "C"

#pragma push
#pragma always_inline on
// Apply one page selection to both rows; close stays idle on every statistics page.
// Template selection is fixed at each switch arm; retain the twenty layout calls in order.
template<int SelectedPage>
static inline void SelectPlayerInfoPageControls(PlayerInfoDrawState* self) {
    Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoBasicButton, SelectedPage != 0);
    Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoShotButton, SelectedPage != 1);
    Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPersonalButton, SelectedPage != 2);
    Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoCourseButton, SelectedPage != 3);
    Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoCloseButton, SelectedPage != 4);
    Layout_SetElementEnabled(self->layout, kPlayerInfoBasicHighlight, SelectedPage == 0);
    Layout_SetElementEnabled(self->layout, kPlayerInfoShotHighlight, SelectedPage == 1);
    Layout_SetElementEnabled(self->layout, kPlayerInfoPersonalHighlight, SelectedPage == 2);
    Layout_SetElementEnabled(self->layout, kPlayerInfoCourseHighlight, SelectedPage == 3);
    Layout_SetElementEnabled(self->layout, kPlayerInfoCloseHighlight, SelectedPage == 4);
    Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedBasicButton, SelectedPage != 0);
    Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedShotButton, SelectedPage != 1);
    Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedPersonalButton, SelectedPage != 2);
    Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedCourseButton, SelectedPage != 3);
    Layout_SetElementPlayerFlagsAndState(self->layout, kPlayerInfoPairedCloseButton, SelectedPage != 4);
    Layout_SetElementEnabled(self->layout, kPlayerInfoPairedBasicHighlight, SelectedPage == 0);
    Layout_SetElementEnabled(self->layout, kPlayerInfoPairedShotHighlight, SelectedPage == 1);
    Layout_SetElementEnabled(self->layout, kPlayerInfoPairedPersonalHighlight, SelectedPage == 2);
    Layout_SetElementEnabled(self->layout, kPlayerInfoPairedCourseHighlight, SelectedPage == 3);
    Layout_SetElementEnabled(self->layout, kPlayerInfoPairedCloseHighlight, SelectedPage == 4);
}
#pragma pop

extern "C" {

// Select the active page button in both button rows and update their highlight elements.
int CPlayerInfo_SetModeButtonStates(void* obj, int pageIndex) {
    // Unknown page indices leave the current visual state untouched and return 1.
    PlayerInfoDrawState* self = (PlayerInfoDrawState*)obj;
    switch (pageIndex) {
    case kPlayerInfoBasicPage:
        SelectPlayerInfoPageControls<kPlayerInfoBasicPage>(self);
        break;
    case kPlayerInfoShotPage:
        SelectPlayerInfoPageControls<kPlayerInfoShotPage>(self);
        break;
    case kPlayerInfoPersonalPage:
        SelectPlayerInfoPageControls<kPlayerInfoPersonalPage>(self);
        break;
    case kPlayerInfoCoursePage:
        SelectPlayerInfoPageControls<kPlayerInfoCoursePage>(self);
        break;
    }
    return 1;
}

#pragma use_lmw_stmw off
#pragma opt_strength_reduction off
#pragma push
#pragma always_inline on
#pragma inline_depth(16)
// Initialize the borrowed save-data singleton while retaining each call site's reloads.
// Do not cache the current record across these calls: copying a record may invoke other code.
static inline void EnsurePlayerInfoSaveData() {
    if (!Sing_805DC5E0()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized=1;
        }
        Sing_805DC5E0()=s_saveData;
    }
}

// Populate the basic page and clear numeric slots owned by other pages.
static inline void PopulatePlayerInfoBasicPage(PlayerInfoDrawState* self, PlayerStatisticsRecord* statistics) {
    int hours=statistics->playTimeFrames/kPlayerInfoFramesPerHour;
    if (hours<=0) hours=0;
    int seconds;
    int minutes=statistics->playTimeFrames/kPlayerInfoFramesPerMinute-hours*60;
    if (minutes<=0) minutes=0;
    seconds=statistics->playTimeFrames/kPlayerInfoFramesPerSecond-hours*3600-minutes*60;
    if (seconds<=0) seconds=0;
    UITextTable_SetInteger(self->numberDraw,kTextPlayHours,hours);
    UITextTable_SetInteger(self->numberDraw,kTextPlayMinutes,minutes);
    UITextTable_SetInteger(self->numberDraw,kTextPlaySeconds,seconds);
    int experience=statistics->experience;
    float progress=(float)experience;
    float total=kFloat_0_Statistics;
    PlayerLevelDefinition* levelDefinition=playerLevelDefinitions;
    int levelIndex;
    for (levelIndex=0; levelIndex<56; ++levelDefinition,++levelIndex) {
        if ((*(volatile unsigned int*)&statistics->experience)<=levelDefinition->end) {
            if (levelIndex==0) total=(float)playerLevelDefinitions[levelIndex].end;
            else {
                unsigned int previous=*(unsigned int*)((char*)&playerLevelDefinitions[levelIndex]-4);
                total=(float)(playerLevelDefinitions[levelIndex].end-previous);
                progress-=(float)previous;
            }
            break;
        }
    }
    if (levelIndex==56) {
        unsigned int maximum=3500;
        total=(float)maximum;
        progress=total;
    }
    UITextTable_SetFloat(self->numberDraw,kTextExperienceProgress,progress);
    UITextTable_SetFloat(self->numberDraw,kTextExperienceSpan,total);
    UITextTable_SetInteger(self->numberDraw,kTextPang,((PlayerStatisticsRecord*)AsCPlayerInfo(self)->playerRecord)->pang);
    char buffer[0x828];
    EnsurePlayerInfoSaveData();
    SaveRecord_CopyCtor(buffer,SaveData_GetCurrentRecord(Sing_805DC5E0()));
    EnsurePlayerInfoSaveData();
    UITextTable_SetInteger(self->numberDraw,kTextSavedShort132,((PlayerInfoLiveTotalsView*)SaveData_GetCurrentRecord(Sing_805DC5E0()))->savedShort132);
    {
        float zeroFlagCount=kFloat_0_Statistics;
        for (int flagIndex=0; flagIndex<kPlayerInfoCompletionBlockSize; ++flagIndex) if (statistics->completionFlags[flagIndex]==0) zeroFlagCount+=kFloat_1_PlayerInfo;
        float percent=kFloat_100_PlayerInfo*(zeroFlagCount*kFloat_0_03125_PlayerInfo);
        if (percent>=kFloat_100_PlayerInfo) percent=kFloat_100_PlayerInfo;
        if (percent<=kFloat_0_Statistics) percent=kFloat_0_Statistics;
        UITextTable_SetFloat(self->numberDraw,kTextCompletionFirstBlock,percent);
    }
    {
        float zeroFlagCount=kFloat_0_Statistics;
        for (int flagIndex=kPlayerInfoCompletionBlockSize; flagIndex<2*kPlayerInfoCompletionBlockSize; ++flagIndex) if (statistics->completionFlags[flagIndex]==0) zeroFlagCount+=kFloat_1_PlayerInfo;
        float percent=kFloat_100_PlayerInfo*(zeroFlagCount*kFloat_0_03125_PlayerInfo);
        if (percent>=kFloat_100_PlayerInfo) percent=kFloat_100_PlayerInfo;
        if (percent<=kFloat_0_Statistics) percent=kFloat_0_Statistics;
        UITextTable_SetFloat(self->numberDraw,kTextCompletionSecondBlock,percent);
    }
    UITextTable_SetFloat(self->numberDraw,kTextGaugeTotal,ShotGauge_GetTotal());
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseValueFirst);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseValueSecond);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseValueThird);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseCountThird);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseCountSecond);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseCountFirst);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextShotCombinedRate);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextShotCount);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalCountA);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalCountB);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalCountC);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalValueA);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalValueB);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalValueC);
}

// Populate the shot page and clear numeric slots owned by other pages.
static inline void PopulatePlayerInfoShotPage(PlayerInfoDrawState* self, PlayerStatisticsRecord* statistics) {
    int rateSampleCount=statistics->sampleCountA;
    if (rateSampleCount==0) UITextTable_SetFloat(self->numberDraw,kTextShotRateA,kFloat_0_Statistics);
    else UITextTable_SetFloat(self->numberDraw,kTextShotRateA,kFloat_100_PlayerInfo*((float)statistics->successCountA/(float)rateSampleCount));
    if ((rateSampleCount=statistics->sampleCountA+statistics->sampleCountB)==0) UITextTable_SetFloat(self->numberDraw,kTextShotCombinedRate,kFloat_0_Statistics);
    else UITextTable_SetFloat(self->numberDraw,kTextShotCombinedRate,kFloat_100_PlayerInfo*((float)statistics->combinedSuccessCount/(float)rateSampleCount));
    if ((rateSampleCount=statistics->sampleCountB)==0) UITextTable_SetFloat(self->numberDraw,kTextShotRateB,kFloat_0_Statistics);
    else UITextTable_SetFloat(self->numberDraw,kTextShotRateB,kFloat_100_PlayerInfo*((float)statistics->successCountB/(float)rateSampleCount));
    if ((rateSampleCount=statistics->sampleCountC)==0) UITextTable_SetFloat(self->numberDraw,kTextShotRateC,kFloat_0_Statistics);
    else UITextTable_SetFloat(self->numberDraw,kTextShotRateC,kFloat_100_PlayerInfo*((float)statistics->successCountC/(float)rateSampleCount));
    UITextTable_SetInteger(self->numberDraw,kTextShotCount,statistics->shotCounter);
    UITextTable_SetFloat(self->numberDraw,kTextShotValue,statistics->shotValue);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPlayHours);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPlayMinutes);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPlaySeconds);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextExperienceProgress);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextExperienceSpan);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPang);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseValueFirst);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseValueSecond);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseValueThird);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseCountFirst);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseCountSecond);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseCountThird);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalCountA);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalCountB);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalCountC);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalValueA);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalValueB);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalValueC);
}

// Populate the personal page and clear numeric slots owned by other pages.
static inline void PopulatePlayerInfoPersonalPage(PlayerInfoDrawState* self, PlayerStatisticsRecord* statistics) {
    UITextTable_SetInteger(self->numberDraw,kTextPersonalCountC,statistics->personalCountC);
    UITextTable_SetFloat(self->numberDraw,kTextPersonalValueA,(float)(int)(kFloat_100_PlayerInfo*statistics->personalValueA)/kFloat_100_PlayerInfo);
    UITextTable_SetFloat(self->numberDraw,kTextPersonalValueB,(float)(int)(kFloat_100_PlayerInfo*statistics->personalValueB)/kFloat_100_PlayerInfo);
    UITextTable_SetFloat(self->numberDraw,kTextPersonalValueC,(float)(int)(kFloat_100_PlayerInfo*statistics->personalValueC)/kFloat_100_PlayerInfo);
    UITextTable_SetInteger(self->numberDraw,kTextPersonalCountA,statistics->personalCountA);
    UITextTable_SetInteger(self->numberDraw,kTextPersonalCountB,statistics->personalCountB);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextSavedShort132);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCompletionFirstBlock);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCompletionSecondBlock);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextShotCombinedRate);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextShotCount);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextGaugeTotal);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPlayHours);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPlayMinutes);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPlaySeconds);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextExperienceProgress);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextExperienceSpan);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPang);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseValueFirst);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseValueSecond);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseValueThird);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseCountFirst);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseCountSecond);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseCountThird);
}

// Populate the course page and clear numeric slots owned by other pages.
static inline void PopulatePlayerInfoCoursePage(PlayerInfoDrawState* self) {
    EnsurePlayerInfoSaveData();
    // Three consecutive 8-byte records in a flat course array; the window advances one record.
    StatEntry* entries=(StatEntry*)(SaveData_GetCurrentRecord(Sing_805DC5E0())+kPlayerInfoCourseRecordsOffset);
    {
        int id=(entries+self->courseRecordIndex)[0].index;
        if ((entries+self->courseRecordIndex)[0].index<0 || id>7) {
            NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseCountFirst);
            NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseValueFirst);
            Layout_SetElementEnabled(self->layout,63,1);
            Layout_SetElementEnabled(self->layout,64,1);
            Layout_SetElementEnabled(self->layout,65,1);
        }
        else {
            UITextTable_SetInteger(self->numberDraw,kTextCourseCountFirst,(entries+self->courseRecordIndex)[0].count);
            UITextTable_SetInteger(self->numberDraw,kTextCourseValueFirst,(entries+self->courseRecordIndex)[0].value);
            Layout_SetElementEnabled(self->layout,63,0);
            Layout_SetElementEnabled(self->layout,64,0);
            Layout_SetElementEnabled(self->layout,65,0);
        }
    }
    {
        int id=(entries+self->courseRecordIndex)[1].index;
        if ((entries+self->courseRecordIndex)[1].index<0 || id>7) {
            NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseCountSecond);
            NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseValueSecond);
            Layout_SetElementEnabled(self->layout,66,1);
            Layout_SetElementEnabled(self->layout,67,1);
            Layout_SetElementEnabled(self->layout,68,1);
        }
        else {
            UITextTable_SetInteger(self->numberDraw,kTextCourseCountSecond,(entries+self->courseRecordIndex)[1].count);
            UITextTable_SetInteger(self->numberDraw,kTextCourseValueSecond,(entries+self->courseRecordIndex)[1].value);
            Layout_SetElementEnabled(self->layout,66,0);
            Layout_SetElementEnabled(self->layout,67,0);
            Layout_SetElementEnabled(self->layout,68,0);
        }
    }
    {
        int id=(entries+self->courseRecordIndex)[2].index;
        if ((entries+self->courseRecordIndex)[2].index<0 || id>7) {
            NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseCountThird);
            NumericTextTable_ClearEntryText(self->numberDraw,kTextCourseValueThird);
            Layout_SetElementEnabled(self->layout,69,1);
            Layout_SetElementEnabled(self->layout,70,1);
            Layout_SetElementEnabled(self->layout,71,1);
        }
        else {
            UITextTable_SetInteger(self->numberDraw,kTextCourseCountThird,(entries+self->courseRecordIndex)[2].count);
            UITextTable_SetInteger(self->numberDraw,kTextCourseValueThird,(entries+self->courseRecordIndex)[2].value);
            Layout_SetElementEnabled(self->layout,69,0);
            Layout_SetElementEnabled(self->layout,70,0);
            Layout_SetElementEnabled(self->layout,71,0);
        }
    }
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalCountC);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalValueA);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalValueB);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalValueC);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalCountA);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPersonalCountB);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextSavedShort132);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCompletionFirstBlock);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextCompletionSecondBlock);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextShotCombinedRate);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextShotCount);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextGaugeTotal);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPlayHours);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPlayMinutes);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPlaySeconds);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextExperienceProgress);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextExperienceSpan);
    NumericTextTable_ClearEntryText(self->numberDraw,kTextPang);
}

#pragma pop

#pragma push
#pragma inline_depth(16)
// Fill the chosen statistics page, clearing numeric entries belonging to the other pages.
int CPlayerInfo_PopulateStatisticsPage(void* obj,int pageIndex) {
    // Reads current global save data, except basic entry A7 uses playerRecord.
    // Each valid page writes its values and clears the other pages' numeric text;
    // an unknown page is a no-op returning 1. No denominator clamp beyond == 0.
    PlayerStatisticsRecord* statistics;
    PlayerInfoDrawState* self=(PlayerInfoDrawState*)obj;
    EnsurePlayerInfoSaveData();
    statistics=(PlayerStatisticsRecord*)SaveData_GetCurrentRecord(Sing_805DC5E0());
    switch (pageIndex) {
    case kPlayerInfoBasicPage: {
            PopulatePlayerInfoBasicPage(self, statistics);
            break;
        }
    case kPlayerInfoShotPage: {
            PopulatePlayerInfoShotPage(self, statistics);
            break;
        }
    case kPlayerInfoPersonalPage: {
            PopulatePlayerInfoPersonalPage(self, statistics);
            break;
        }
    case kPlayerInfoCoursePage: {
            PopulatePlayerInfoCoursePage(self);
            break;
        }
    }
    return 1;
}
#pragma pop

#undef SI
#undef SF
#pragma optimize_for_size on
// Draw up to three valid character icons for the visible course-record entries.
int CPlayerInfo_DrawRecordIcons(PlayerInfoIconOwner* self,int index) {
    if (!Sing_805DC5E0()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized=1;
        }
        Sing_805DC5E0()=s_saveData;
    }
    RecordCharacterIndex* base=(RecordCharacterIndex*)(SaveData_GetCurrentRecord(Sing_805DC5E0())+0x25c);
    RecordCharacterIndex* entry=base+index;
    for (int i=0;i<kPlayerInfoRecordWindow;++i,++entry) {
        int id=entry->index;
        if (entry->index>=0 && id<=7 && self->items[id].valid()) {
            self->items[entry->index].apply();
            int rect[4];
            Layout_ReadElementRect(self->layout,i+0x3c,rect);
            Quad quad=s_playerInfoCharacterIconQuad;
            quad.v[0].x=rect[0];
            quad.v[0].y=rect[1];
            quad.v[1].x=rect[2];
            quad.v[1].y=rect[1];
            quad.v[2].x=rect[0];
            quad.v[2].y=rect[3];
            quad.v[3].x=rect[2];
            quad.v[3].y=rect[3];
            int count=GX_GetPrimitiveCount(4,0x98);
            DrawPrimitiveUP(0x98,count,&quad,0x18);
        }
    }
    return 1;
}

#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Return the checked activated-element word, throwing out_of_range for an invalid index.
unsigned int* PlayerInfoWordVector_At(void* obj,unsigned int index) {
    IntVector* array = (IntVector*)obj;
    if (index>=array->size) {
        const char* text=playerInfoStrings + kPlayerInfo_vector_at_index_out_of_range;
        char flag=0;
        ErrorBase_u8029736C error;
        ErrorBase_u8029736C* e=&error;
        e->vt=__vt__Q23std11logic_error;
        unsigned long n=strlen(text)+1;
        char* data=(char*)__nwa__FUl((n<<1)-n);
        e->str=data;
        {
            Guard_u8029736C guard;
            guard.data=data;
            guard.flag=&flag;
            e->block=new SharedBlock(data);
            guard.data=0;
        }
        strcpy(e->str,text);
        e->vt=__vt__Q23std12out_of_range;
        __throw(playerInfoStrings + kPlayerInfo_std_exception_std_logic_error_std_out_of_range,e,(void*)__dt__Q23std12out_of_rangeFv);
    }
    return (unsigned int*)array->items+index;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462464", _ctors_u8029736C_0, PlayerInfo_InitFullTurnConstantA);
STATIC_INIT_ENTRY(".CRT$XCU80462464", _ctors_u8029736C_1, PlayerInfo_InitFullTurnConstantB);
#pragma pop
