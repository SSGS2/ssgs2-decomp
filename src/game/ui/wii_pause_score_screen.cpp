#include <decomp/local_class.h>
// Pause / post-hole golf scorecard: score tables, party rankings, graphs,
// training actions, confirmation/options menus and input publication.
// Mod lookup:
// - Winner/tie policy: CWiiPauseScore_RankPartyHoleResults, PartyScoreRule and
//   PausePlayerHoleScore below; keep score representation and save mode together.
// - Scorecard/page/column: CWiiPauseScore_RefreshScorecard, ScoreDisplayMode,
//   ScoreView and the refresh map below. Overall gameMode is a separate protocol.
// - Graph layout: kGraphWidth/kHolesPerPage, PauseGraphPoint and the graph draw
//   hook; colours/layout presets are s_pauseScoreGraphLayoutPresets/s_scorecardPlayerCountTables in the companion.
// - Buttons: ScoreOptionControl and UI protocol below; change BP/layout element
//   IDs together with their pressed/selected indicators and returned actions.
// Caller CGolfTask_BeginLoadingAndBuildPauseMenu (golf_task_loading_and_pause.cpp) constructs/selects this menu and feeds
// hole-major scores; MPBase1 drives its load/process/draw lifecycle. Shared
// schemas: CWiiPauseScore, CGolfDoc, CSharedDoc and UITextTable headers. Numeric
// data and strings: u8032E804_data.cpp and pooled wiiPauseScoreStrings. Layout
// coordinates are pixels; no local video resolution or presentation-rate knob.
extern "C" char s_pauseBpHandleRegistration[];
extern "C" {
extern unsigned char s_pauseScoreGraphLayoutPresets[]; // Score-graph colour tables.
extern unsigned char s_scorecardPlayerCountTables[]; // Player-count layout tables and graph presets.
}
// Party variants are named by the scoring algorithm observed here, rather than unverified UI titles.
enum PartyScoreRule {
    kPartyLowerAdjustedValueWins = 1, kPartyHigherCompetitionValueWins = 2,
    kPartySumStrokeValues = 3
};
enum ScoreInteraction { kScoreInactive = 0, kScoreInteractive = 1, kScoreExiting = 4, kScoreOptionsOpen = 5, kScoreConfirmOpen = 6 };
// s_pauseScoreGraphLayoutPresets (0x430 bytes) = 36 layout presets of 28 bytes (0x3F0), then a 16-word score-graph colour table;
// the second reader starts 8 words (0x20 bytes) into that table.
enum { kScoreGraphColourOffset = 0x3F0, kScoreGraphColourSecondHalfOffset = 0x410 };
enum { kPauseMatchTextOffset = 0x42, kPauseRoundTextOffset = 0x7e, kPauseTrainingTextOffset = 0xba };

enum { kDrawLines = 0xb0, kDrawTriangleStrip = 0x98, kScoreVertexFormat = 0xf005, kPlayerHoleScoreBytes = 12 };

// Byte offsets into the existing string pool; keep its storage and encoding.
enum {
    kWiiPauseScoreStrings_vector_length_error = 0x96, // vector length error
    kWiiPauseScoreStrings_std_exception_std_logic_error_std_length_error = 0xaa, // !std::exception!!std::logic_error!!std::length_error!!
    kWiiPauseScoreStrings_Black_sprite = 0xe1, // Black_sprite
    kWiiPauseScoreStrings_score_training = 0xee, // score_training
    kWiiPauseScoreStrings_score_match = 0xfd, // score_match
    kWiiPauseScoreStrings_score_dstroke = 0x109, // score_%dstroke
    kWiiPauseScoreStrings_score_dparty = 0x118, // score_%dparty
    kWiiPauseScoreStrings_btn_select = 0x126, // btn_select
    kWiiPauseScoreStrings_player_num = 0x131, // player_num
    kWiiPauseScoreStrings_score_num11_20_gsr = 0x13c, // score_num11_20.gsr
    kWiiPauseScoreStrings_R_score_record_d_d = 0x14f, // R_score_record%d_%d
    kWiiPauseScoreStrings_R_NUM_hole_d_K2_LC = 0x163, // R_NUM_hole%d_K2_LC
    kWiiPauseScoreStrings_R_score_subtotal = 0x176, // R_score_subtotal
    kWiiPauseScoreStrings_R_NUM_result_d_K4_LC_FM = 0x187, // R_NUM_result%d_K4_LC_FM
    kWiiPauseScoreStrings_R_NUM_rtotal_d_K5_LC_FM = 0x19f, // R_NUM_rtotal%d_K5_LC_FM
    kWiiPauseScoreStrings_R_NUM_scr_d_K3_LC_FM_FP = 0x1b7, // R_NUM_scr%d_K3_LC_FM_FP
    kWiiPauseScoreStrings_R_NUM_rscore_d_K3_LC_FM = 0x1cf, // R_NUM_rscore%d_K3_LC_FM
    kWiiPauseScoreStrings_R_score_record1_d = 0x1e7, // R_score_record1_%d
    kWiiPauseScoreStrings_R_score_record2_d = 0x1fa, // R_score_record2_%d
};

// Score data are hole-major: scores[hole * playerCount + player].
#define PAUSE_FIELD_OFFSET(field) ((int)&((CWiiPauseScore*)0)->field)
struct PauseHoleInfo { char holeNumber; char padding[3]; int par; };
struct PausePlayerHoleScore { int strokes; int competitionValue; int pang; };
enum { kHolesPerPage = 9, kGraphWidth = 440, kScoreVertexStride = 16 };
enum ScoreDisplayMode {
    kScoreStroke = 0, kScoreStrokeAlternate = 1, kScoreMatch = 2,
    kScoreParty = 3, kScoreTraining = 4, kScoreMatchAlternate = 5,
    kScoreConfirmationOnly = 6
};
enum ScoreCellField { kCellStrokes = 0, kCellCompetition = 4, kCellPang = 8 };
// View numbers are shared by several display modes; the secondary view's column
// is selected by displayMode/partyGameMode, so it must not be called "Pang view".
enum ScoreView { kPrimaryView = 0, kSecondaryView = 1, kCompetitionView = 2, kGraphView = 3 };
enum ScoreControllerScript {
    kPauseControllerScript = 0x10,
    kStrokePostHoleControllerScript = 0x12,
    kPostHoleControllerScript = 0x13
};

// Overall gameMode is independent of ScoreDisplayMode. Values named here are
// source-facing SAVE policies (in_game_dialog_and_snapshot_copy.cpp), not invented user-facing mode titles:
// 0 saves single-player Stroke/Match course progress, 6 saves party progress.
// CSingleMainRev publishes 0 for both selectors; party publication uses 6.
// CGolfTask_BeginLoadingAndBuildPauseMenu maps 0/1 -> stroke-alternate(1) or match(2) by rule;
// 2/3/4 -> confirmation-only(6); 5 -> Free Training(4); 6/7 -> party(3) if
// partyGameMode!=0, else stroke-alternate(1)/match-alternate(5). Default ->0.
// Here mode 0 gates post-hole single-player recording/actions and mode 6 the
// persistent-party controls; the alternate overall values share display layouts
// without automatically gaining those controls. Keep the two enums distinct.
enum { kSinglePlayerRecordMode = 0, kPartyRecordMode = 6 };
enum ScoreOptionControl {
    kOptionStrokeView = 0, kOptionTrainingView = 1, kOptionMatchView = 2,
    kOptionNineHolePage = 3, kOptionPartyStrokeView = 4,
    kOptionPartyAdjustedView = 5, kOptionPartyCompetitionView = 6,
    kOptionFrontNine = 7, kOptionBackNine = 8, kOptionPageIndicatorBase = 7,
    kOptionViewIndicatorBase = 9, kOptionPressedIndicatorOffset = 13,
    kOptionTrainingViewPressed = kOptionTrainingView + kOptionPressedIndicatorOffset,
    kOptionNineHolePagePressed = kOptionNineHolePage + kOptionPressedIndicatorOffset
};

// UI protocol (element IDs are local to each layout):
// options 0 cycles stroke views 0..3; 1 toggles training views 0/1; 2 and 4..6
// toggle mode-specific views 0/1; 3 toggles nine-hole pages. 7/8 are page markers,
// 9+selectedView is the view marker, control+13 is its pressed presentation.
// Regular menu 0 resumes, 1 confirms exit, 2 opens options, 3 returns action 3,
// 4 returns action 6 (or changes a tied match view first); pressed IDs are +5.
// Training menu 0/1/2 have the same roles; 3..9 return actions 3,4,5,7,8,6,9.
// Pressed IDs are +10; post-hole controls 6..9 also update IDs +14 and +18.
// selectedView 0 shows strokes/match winners/party rank; 1 shows training result,
// adjusted-party strokes, or Pang; 2 shows competition values; 3 only totals.
// playerRows maps logical players to visual rows, so cell IDs use that mapping.
// Score storage is owned by self; layouts and dialogs have explicit cleanup in
// dtor/vfunc_5. Callers retain no ownership of buffers passed to the setter APIs.
//
// RefreshScorecard view map (header totals are computed BEFORE the view branch):
// view  display/rule                 cells                    row footer
// 0     stroke                      strokes, score tint      completed strokes
// 0     match                       winner/tie/loss markers   hidden
// 0     party rule 3                party stroke values      hidden
// 0     party rules 1/2             ranking indicators       hidden
// 1     training                    competition values       completed competition
// 1     party rule 1                party stroke values      hidden
// 1     all remaining               Pang                     cumulativePang array
// 2     supported stroke views      competition values       completed competition
// 3     supported stroke views      graph in draw hook       completed strokes
// Header totals: match=hole wins, party 3=sum, other party=sole wins,
// other modes=completed strokes minus completed par. They are not footer totals.
// All score arrays use LOGICAL player order; text/marker positions use
// playerRows[player]. Never apply that permutation to playerHoleScores itself.
enum { kResumeInputAction = 0x11, kAlternateResumeInputAction = 0x12 };
// Input protocol numbers are retained until the physical buttons are verified.
enum { kPostHoleInput2 = 2, kPostHoleInput3 = 3, kPostHoleDismissAction = 10 };
enum ScorePaletteIndex {
    kTripleBogeyOrWorseColour = 0, kHoleInOneColour = 1,
    kAlbatrossOrBetterColour = 2, kParColour = 5
};
enum { kHoleInOneStrokes = 1, kAlbatrossDelta = -3, kTripleBogeyDelta = 3 };

#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/UITextTable.h"

// Party rounds store one course index for each hole; this array is padding in CSharedDoc.
struct PartyRoundCourseView {
    char sharedDocPrefix[0x83d];
    unsigned char courseIndices[18];
};

#define GAME_VECTOR_AGGREGATE

#include "game/vector.h"

// Stack-local storage view; external vector operations retain the original ABI.
struct LocalIntVector_u8032E804 {
    int* items;
    volatile unsigned int size;
    unsigned int capacity;
    // Clear the temporary score count and free its owned integer array.
    ~LocalIntVector_u8032E804() {
        if (items) {
            size = *(unsigned int*)&size - *(int*)&size;
            appFreeUni__FPv(items);
        }
    }
    unsigned int grow(unsigned int);
    void allocate(unsigned int);
    void append(unsigned int);
    void resize(unsigned int);
    int& operator[](int);
};

// Translation unit 0x8032E804-0x80333CE8 (20 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CWiiPauseScore.h"
#include <decomp/cw_bitfields.h>

#define AsCWiiPauseScore(p) ((CWiiPauseScore*)(p))

#define PAUSE_INDEXED_OFFSET(field) ((int)&((PauseIndexedFields*)0)->field)
// Indexed score-screen fields; scalar fields continue to use CWiiPauseScore.h.
struct PauseGraphPoint { int x, y; };
struct PauseIndexedFields {
    char padding000[0x64];
    int holeNumberTextId, parTextId, parTotalTextId, cellTextId;
    int subtotalTextIds[4];
    int roundTotalTextIds[4];
    char padding094[0x10];
    int cumulativePang[4];
    PauseGraphPoint graphOrigins[4];
    char padding0d4[0x1c];
    int menuButtonAvailable[6];
    char padding108[0x18];
    unsigned char playerRows[4];
    PauseGraphPoint playerLabelPositions[4];
};
typedef char PauseIndexedFields_size[(sizeof(PauseIndexedFields) == 0x144) ? 1 : -1];

// Locate a mapped player row in screen pixels; no ownership or bounds check.
static inline PauseGraphPoint* PauseScoreGraphRow(unsigned char* scoreBytes, int playerIndex) {
    return (PauseGraphPoint*)(scoreBytes + (((int)scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + playerIndex]) << 3) + PAUSE_INDEXED_OFFSET(graphOrigins));
}

// Read one hole-major score column without changing the stored signed value.
static inline int PauseScoreCell(void* scores, unsigned char playerCount, int hole, int player, int fieldOffset) {
    return *(int*)((char*)scores + (hole * playerCount + player) * 12 + fieldOffset);
}

extern "C" {

void BpObject_SetEnabled(void* self, int value);
void Layout_Update(void* self);
void appFreeUni__FPv(void* self);
void __dl__FPv(void* self);
void Layout_SetElementPlayerFlag(void* self, int a, int b, int c);
void Layout_SetElementColor(void* self, int a, int b);
int BpObject_PollLoad(void* self);
int Layout_PollLoadComplete(void* self);
void Layout_DrawWithOpacity(void* self, float value);
void BpObject_TryInvokeCurrentCallback(void* self, int value);
void BpObject_Draw(void* self);
void Layout_Destroy(void* self, int deleting);
void __ct__7CBpCtrlFv(void* self);
void CBpCtrl_dtor(void);
void __register_global_object(void* object, void* destructor, void* dso);
void CBpCtrl_DeleteEntryById(void* self, int value);
void UITextTable_ClearEntries(void* self);
void PointerVector_InitReserved(void* self);
void PointerVector_DestroyStorage(void);
void PointerArray_DeleteValue(void* self, int value);
void UITextTable_Draw(void* self);
extern char pauseScorePartTotalGroupName[];
extern float kPauseScoreGraphHoleSpacing;
extern float kPauseScoreGraphHalfHoleSpacing;
void Engine_SetRenderStateList(void* dest, int count);
void Engine_SetFlexibleVertexFormat(int value);
void Engine_SetType7Resource(int value);
void DrawPrimitiveUP(int primitive, int count, void* vertices, int stride);
void Vec3_Add(void* dest, void* source, float* xy);
struct ScoreRenderState { unsigned int a, b; };
struct ScoreRenderStateList { ScoreRenderState p[7]; };
struct ScoreGraphVertex { float x, y, z; unsigned char r, g, b, a; };
struct ScoreGraphLine { ScoreGraphVertex v[2]; };
struct ScoreGraphLineOffsets { float v[6]; };
struct ScoreGraphMarkerOffsets { float v[12]; };
struct ScoreGraphScales { float v[5]; };


// Draw per-player stroke graphs, coloured by strokes relative to each hole par.
void CWiiPauseScore_DrawScoreGraph(void* self) {
    // Phase 1: choose the nine-hole range and install graph render state.
    int showFrontNine;
    int displayedHoles;
    unsigned int* colourTable;
    int lastCompletedHole;
    int threshold;
    float* scales;
    unsigned int cr, cg, cb, ca;
    int holeIndex; int playerIndex; int nextHoleIndex;
    float currentHoleX, nextHoleX, holeSpacing, halfHoleSpacing;
    int end = kHolesPerPage;
    showFrontNine = AsCWiiPauseScore(self)->showFrontNine;
    unsigned char* scoreConfig = (unsigned char*)pauseScorePartTotalGroupName;
    displayedHoles = AsCWiiPauseScore(self)->displayedHoleCount;
    int start = (showFrontNine != 0) ? 0 : kHolesPerPage;
    end = ((int)displayedHoles - start <= kHolesPerPage) ? displayedHoles : end;
    unsigned char* scoreBytes = (unsigned char*)self;

    ScoreGraphScales playerScales = *(ScoreGraphScales*)(((char*)(scoreConfig + 0x2838)));
    ScoreRenderStateList pairs = *(ScoreRenderStateList*)(((char*)(scoreConfig + 0x2850)));
    Engine_SetRenderStateList(&pairs, 7);
    Engine_SetFlexibleVertexFormat(kScoreVertexFormat);
    Engine_SetType7Resource(0);

    ScoreGraphLine line = *(ScoreGraphLine*)(((char*)(scoreConfig + 0x2888)));
    unsigned char* lp = scoreBytes;
    for (int i = 0; i < (int)AsCWiiPauseScore(self)->playerCount; lp += 8, ++i) {
        line.v[0].x = (float)AsCWiiPauseScore(lp)->graphOrigin0X;
        line.v[1].x = (float)(AsCWiiPauseScore(lp)->graphOrigin0X + kGraphWidth);
        line.v[0].y = line.v[1].y = (float)AsCWiiPauseScore(lp)->graphOrigin0Y;
        DrawPrimitiveUP(kDrawLines, 1, &line, kScoreVertexStride);
    }

    ScoreGraphVertex quad[4] = {};
    holeSpacing = kPauseScoreGraphHoleSpacing;
    halfHoleSpacing = kPauseScoreGraphHalfHoleSpacing;
    lastCompletedHole = end - 1;
    colourTable = (unsigned int*)(((char*)(scoreConfig + 0x2678)));
    threshold = kTripleBogeyDelta;
    scales = playerScales.v;
    ScoreGraphLineOffsets lineOffsets = *(ScoreGraphLineOffsets*)(((char*)(scoreConfig + 0x28a8)));
    ScoreGraphMarkerOffsets markerOffsets = *(ScoreGraphMarkerOffsets*)(((char*)(scoreConfig + 0x28c0)));
    float pointA[3] = { 0.0f, 0.0f, 0.0f };
    float pointB[3] = { 0.0f, 0.0f, 0.0f };

    // Phase 2: for each player/hole, screen x = rowOrigin.x + holeSpacing*pageHole
    // + halfSpacing; screen y = rowOrigin.y + playerScale*(strokes-par). The
    // hole positions below are horizontal offsets, before adding each row origin.
    // Endpoint colors encode score relative to par, with hole-in-one overriding it.
    for (holeIndex = start; holeIndex < end; ++holeIndex) {
        nextHoleIndex = (1 + holeIndex);
        currentHoleX = holeSpacing * (float)(holeIndex - start) + halfHoleSpacing;
        nextHoleX = holeSpacing * (float)(nextHoleIndex - start) + halfHoleSpacing;
        for (playerIndex = 0; playerIndex < (int)AsCWiiPauseScore(self)->playerCount; ++playerIndex) {
            pointA[0] = currentHoleX + (float)PauseScoreGraphRow(scoreBytes, playerIndex)->x;
            int par = ((PauseHoleInfo*)AsCWiiPauseScore(self)->holeInfo)[holeIndex].par;
            int strokes = ((PausePlayerHoleScore*)AsCWiiPauseScore(self)->playerHoleScores)[holeIndex * (int)AsCWiiPauseScore(self)->playerCount + playerIndex].strokes;
            pointA[1] = scales[AsCWiiPauseScore(self)->playerCount] * (float)(strokes - par) + (float)PauseScoreGraphRow(scoreBytes, playerIndex)->y;

            // Colour queries retain the original byte-width player/hole projections.
            int strokesForColour = ((PausePlayerHoleScore*)AsCWiiPauseScore(self)->playerHoleScores)[(int)AsCWiiPauseScore(self)->playerCount * (unsigned char)holeIndex + (unsigned char)playerIndex].strokes;
            int colour;
            if (strokesForColour == kHoleInOneStrokes) {
                colour = kHoleInOneColour;
            } else {
                int delta = strokesForColour -
                    ((PauseHoleInfo*)AsCWiiPauseScore(self)->holeInfo)[(unsigned char)holeIndex].par;
                if (delta <= kAlbatrossDelta) {
                    colour = kAlbatrossOrBetterColour;
                } else {
                    colour = delta >= threshold ? kTripleBogeyOrWorseColour : delta + kParColour;
                }
            }
            unsigned int rgba = colourTable[colour];
            cr = (unsigned char)(rgba >> 24);
            cg = (unsigned char)(rgba >> 16);
            cb = (unsigned char)(rgba >> 8);
            ca = (unsigned char)rgba;
            quad[0].r = quad[2].r = cr;
            quad[0].g = quad[2].g = cg;
            quad[0].b = quad[2].b = cb;
            quad[0].a = quad[2].a = ca;

            if (holeIndex < lastCompletedHole) {
                pointB[0] = nextHoleX + (float)PauseScoreGraphRow(scoreBytes, playerIndex)->x;
                int nextPar = ((PauseHoleInfo*)AsCWiiPauseScore(self)->holeInfo)[nextHoleIndex].par;

                pointB[1] = scales[AsCWiiPauseScore(self)->playerCount] * (float)((((PausePlayerHoleScore*)AsCWiiPauseScore(self)->playerHoleScores)[nextHoleIndex * (int)AsCWiiPauseScore(self)->playerCount + playerIndex].strokes) - nextPar) + (float)PauseScoreGraphRow(scoreBytes, playerIndex)->y;
                int nextStrokesForColour = ((PausePlayerHoleScore*)AsCWiiPauseScore(self)->playerHoleScores)[(unsigned char)nextHoleIndex * (int)AsCWiiPauseScore(self)->playerCount + (unsigned char)playerIndex].strokes; int colour2;
                if (nextStrokesForColour == kHoleInOneStrokes) {
                    colour2 = kHoleInOneColour;
                } else {
                    int delta = nextStrokesForColour -
                        ((PauseHoleInfo*)AsCWiiPauseScore(self)->holeInfo)[(unsigned char)nextHoleIndex].par;
                    if (delta <= kAlbatrossDelta) {
                        colour2 = kAlbatrossOrBetterColour;
                    } else {
                        colour2 = threshold <= delta ? kTripleBogeyOrWorseColour : delta + kParColour;
                    }
                }
                unsigned int rgba2 = colourTable[colour2];
                unsigned char cr2 = (unsigned char)(rgba2 >> 24);
                unsigned char cg2 = (unsigned char)(rgba2 >> 16);
                unsigned char cb2 = (unsigned char)(rgba2 >> 8);
                unsigned char ca2 = (unsigned char)rgba2;
                quad[1].r = quad[3].r = cr2;
                quad[1].g = quad[3].g = cg2;
                quad[1].b = quad[3].b = cb2;
                quad[1].a = quad[3].a = ca2;
                Vec3_Add(&quad[0], &lineOffsets.v[0], pointA);
                Vec3_Add(&quad[1], &lineOffsets.v[0], pointB);
                Vec3_Add(&quad[2], &lineOffsets.v[3], pointA);
                Vec3_Add(&quad[3], &lineOffsets.v[3], pointB);
                DrawPrimitiveUP(kDrawTriangleStrip, 2, quad, kScoreVertexStride);
            }
            quad[1].r = quad[3].r = cr;
            quad[1].g = quad[3].g = cg;
            quad[1].b = quad[3].b = cb;
            quad[1].a = quad[3].a = ca;
            Vec3_Add(&quad[0], &markerOffsets.v[0], pointA);
            Vec3_Add(&quad[1], &markerOffsets.v[3], pointA);
            Vec3_Add(&quad[2], &markerOffsets.v[6], pointA);
            Vec3_Add(&quad[3], &markerOffsets.v[kHolesPerPage], pointA);
            DrawPrimitiveUP(kDrawTriangleStrip, 2, quad, kScoreVertexStride);
        }
    }
}
void Engine_SetRenderStateList(void* dest, int count);
void Engine_SetFlexibleVertexFormat(int value);
void Engine_SetType7Resource(int value);
void DrawPrimitiveUP(int primitive, int count, void* vertices, int stride);
void Vec3_Add(void* dest, void* source, float* xy);
void MPBase1_DrawIfInitialized(void* self);
int Layout_CollectActivated(void* layout, void* results, int player);
int Layout_SetElementPlayerFlagsAndState(void* layout, int element, int visible);
void Layout_SetElementEnabled(void* layout, int element, int visible);
void Layout_SetElementPlayerFlag(void* layout, int element, int visible, int colour);
void* COptionDialog_Ctor(void* self, int value);
void CSmallCommonDialog_Create(int index, void* output, int count);
void CBpCtrlHandle_ReleaseEntry(void* self);
void CBpCtrlHandle_Dtor(void* self);
void __register_global_object(void* object, void* destructor, void* dso);
extern void* g_sharedDoc;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern char s_pauseBpHandleRegistration[0x100];
extern char pauseConfirmationText[0x100];
extern char jumptable_804BA3B8[0x100];

void CWiiPauseScore_RefreshScorecard(void* self);
void Layout_SetElementPosition(void* layout, int element, void* value);
void Layout_GetElementPosition(void* layout, int element, void* value);
void Layout_SetElementEnabled(void* layout, int element, int visible);
int SharedDoc_IsPlayerFlag0Set(int index);
void* GolfPlayer_GetEntryByIndex(void* self, unsigned char index);
void BpObject_StartScriptByName(void* self, const char* name, int value);
void UIGroup_SetEnabled(void* self, int value);
void sprintf(char* dest, const char* format, ...);
void* BpObject_GetOrCreateNamedGroup(void* self, const char* name);
void CFontNumDraw_RegisterEntries(void* self, int a, int b, void* layout, const char* name);
extern char wiiPauseScoreStrings[];

#pragma push
#pragma use_lmw_stmw on
// Activate the score screen, configure mode-specific controls and select the initial nine-hole page.
void CWiiPauseScore_InitializeScoreScreen(void* self) {
    // Phase 1: restore available controls and select the mode's post-hole buttons.
    unsigned char* o = (unsigned char*)self;
    unsigned char* cfg = (unsigned char*)pauseScorePartTotalGroupName;
    struct I5 { int v[5]; };
    switch (AsCWiiPauseScore(o)->displayMode) {
    case kScoreStroke:
    case kScoreStrokeAlternate:
    case kScoreMatch:
    case kScoreParty:
    case kScoreMatchAlternate: {
        unsigned char* p = o;
        for (int i = 0; i <= 3; ++i, p += 4) {
            int s = AsCWiiPauseScore(o)->displayMode;
            if (s == 6) continue;
            if (i < 4) {
                if (i >= 0) {} else continue; // Keep the positive test's original branch polarity.
                } else if (!(i < 6 && s == 4)) continue;
            Layout_SetElementPlayerFlag(AsCWiiPauseScore(o)->menuLayout, i, 1, 5);
            Layout_SetElementColor(AsCWiiPauseScore(o)->menuLayout, i, -1);
            AsCWiiPauseScore(p)->menuButtonAvailable0 = 1;
        }
        if (AsCWiiPauseScore(o)->postHoleMode != 0) {
            for (int i = 0; i <= 10; ++i) Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->menuLayout, i, 0);
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->menuLayout, 4, 1);
        } else {
            for (int i = 0; i <= 10; ++i) Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->menuLayout, i, 0);
            for (int i = 0; i <= 3; ++i) Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->menuLayout, i, 1);
        }
        break;
    }
    case kScoreTraining: {
        unsigned char* p = o;
        for (int i = 0; i <= 5; ++i, p += 4) {
            int s = AsCWiiPauseScore(o)->displayMode;
            if (s == 6) continue;
            if (i < 4) {
                if (i >= 0) {} else continue; // Keep the positive test's original branch polarity.
                } else if (!(i < 6 && s == 4)) continue;
            Layout_SetElementPlayerFlag(AsCWiiPauseScore(o)->menuLayout, i, 1, 5);
            Layout_SetElementColor(AsCWiiPauseScore(o)->menuLayout, i, -1);
            AsCWiiPauseScore(p)->menuButtonAvailable0 = 1;
        }
        for (int i = 0; i <= 0x1c; ++i) Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->menuLayout, i, 0);
        if (AsCWiiPauseScore(o)->postHoleMode != 0) {
            if (AsCWiiPauseScore(o)->holeCount == AsCWiiPauseScore(o)->displayedHoleCount) {
                Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->menuLayout, 9, 1);
                Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->menuLayout, 7, 1);
                Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->menuLayout, 6, 1);
                Layout_SetElementEnabled(AsCWiiPauseScore(o)->menuLayout, 0x17, 1);
                Layout_SetElementEnabled(AsCWiiPauseScore(o)->menuLayout, 0x15, 1);
                Layout_SetElementEnabled(AsCWiiPauseScore(o)->menuLayout, 0x14, 1);
            } else {
                Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->menuLayout, 8, 1);
                Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->menuLayout, 9, 1);
                Layout_SetElementEnabled(AsCWiiPauseScore(o)->menuLayout, 0x16, 1);
                Layout_SetElementEnabled(AsCWiiPauseScore(o)->menuLayout, 0x17, 1);
            }
        } else {
            for (int i = 0; i <= 5; ++i) Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->menuLayout, i, 1);
        }
        break;
    }
    case kScoreConfirmationOnly:
        return;
    }

    // Phase 2: reset presentation layers, choose the score script and option controls.
    if (AsCWiiPauseScore(o)->scorecardSuppressed != 0) return;
    BpObject_StartScriptByName(AsCWiiPauseScore(o)->blackSprite, wiiPauseScoreStrings + kWiiPauseScoreStrings_Black_sprite, 1);
    I5 limA = *(I5*)(((char*)(cfg + 0x27ac)));
    I5 limB = *(I5*)(((char*)(cfg + 0x27c0)));
    I5 limC = *(I5*)(((char*)(cfg + 0x27d4)));
    for (int i = 0; i <= 0x13; ++i) Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->optionsLayout, i, 0);
    if (AsCWiiPauseScore(o)->alternatePlayerLabelLayout != 0) Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->alternatePlayerLabelLayout, 0, 0);
    for (int i = 0; i <= limA.v[AsCWiiPauseScore(o)->playerCount]; ++i) Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->playerLabelLayout, i, 0);
    for (int i = 0; i <= limB.v[AsCWiiPauseScore(o)->playerCount]; ++i) Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->tieMarkerLayout, i, 0);
    for (int i = 0; i <= limC.v[AsCWiiPauseScore(o)->playerCount]; ++i) Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->lossMarkerLayout, i, 0);
    AsCWiiPauseScore(o)->selectedView = 0;
    char buf[0x40];
    switch (AsCWiiPauseScore(o)->displayMode) {
    case kScoreTraining:
        BpObject_StartScriptByName(AsCWiiPauseScore(o)->scoreObject, wiiPauseScoreStrings + kWiiPauseScoreStrings_score_training, 1);
        Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->optionsLayout, kOptionTrainingView, 1);
        break;
    case kScoreMatch:
    case kScoreMatchAlternate:
        if (AsCWiiPauseScore(o)->playerCount <= 2) {
            BpObject_StartScriptByName(AsCWiiPauseScore(o)->scoreObject, wiiPauseScoreStrings + kWiiPauseScoreStrings_score_match, 1);
        } else {
            sprintf(buf, wiiPauseScoreStrings + kWiiPauseScoreStrings_score_dstroke, AsCWiiPauseScore(o)->playerCount);
            BpObject_StartScriptByName(AsCWiiPauseScore(o)->scoreObject, buf, 1);
        }
        Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->optionsLayout, kOptionMatchView, 1);
        break;
    case kScoreParty:
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartySumStrokeValues) Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->optionsLayout, kOptionPartyStrokeView, 1);
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartyLowerAdjustedValueWins) Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->optionsLayout, kOptionPartyAdjustedView, 1);
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartyHigherCompetitionValueWins) Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->optionsLayout, kOptionPartyCompetitionView, 1);
        sprintf(buf, wiiPauseScoreStrings + kWiiPauseScoreStrings_score_dparty, AsCWiiPauseScore(o)->playerCount);
        BpObject_StartScriptByName(AsCWiiPauseScore(o)->scoreObject, buf, 1);
        break;
    default:
        sprintf(buf, wiiPauseScoreStrings + kWiiPauseScoreStrings_score_dstroke, AsCWiiPauseScore(o)->playerCount);
        BpObject_StartScriptByName(AsCWiiPauseScore(o)->scoreObject, buf, 1);
        Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->optionsLayout, kOptionStrokeView, 1);
        break;
    }
    Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->optionsLayout, kOptionNineHolePage, 1);
    Layout_SetElementEnabled(AsCWiiPauseScore(o)->optionsLayout, kOptionFrontNine, 1);
    Layout_SetElementEnabled(AsCWiiPauseScore(o)->optionsLayout, 9, 1);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(*(void**)(o + PAUSE_FIELD_OFFSET(scoreObject)), wiiPauseScoreStrings + kWiiPauseScoreStrings_btn_select), 0);
    if (AsCWiiPauseScore(o)->playerCount != 1) {
        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(*(void**)(o + PAUSE_FIELD_OFFSET(scoreObject)), wiiPauseScoreStrings + kWiiPauseScoreStrings_player_num), 0);
    }
    if (AsCWiiPauseScore(o)->interactionState == 0) {
        int* pd = (int*)(((char*)(cfg + 0x27e8)));
        int* pe = (int*)(((char*)(cfg + 0x27fc)));
        I5 limD = *(I5*)pd;
        I5 limE = *(I5*)pe;
        CFontNumDraw_RegisterEntries(AsCWiiPauseScore(o)->textTable, limD.v[AsCWiiPauseScore(o)->playerCount], limE.v[AsCWiiPauseScore(o)->playerCount], *(void**)(o + PAUSE_FIELD_OFFSET(scoreObject)), wiiPauseScoreStrings + kWiiPauseScoreStrings_score_num11_20_gsr);
        if (AsCWiiPauseScore(o)->playerCount > 1) {
            unsigned char* q = o + 0x124;
            for (int i = 0, j = 0; i < AsCWiiPauseScore(o)->playerCount; q += 8, j += 2, ++i) {
                Layout_GetElementPosition(AsCWiiPauseScore(o)->playerLabelLayout, j, q);
            }
        }
    }
    // Phase 3: select a page, hide pending presentation, and arm first-tick activation.
    AsCWiiPauseScore(o)->showFrontNine = 1;
    if ((*(unsigned int*)&AsCWiiPauseScore(o)->holeInfoCount) < 10) {
        Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(o)->optionsLayout, kOptionNineHolePage, 0);
        Layout_SetElementEnabled(AsCWiiPauseScore(o)->optionsLayout, kOptionFrontNine, 0);
    } else if (AsCWiiPauseScore(o)->holeCount > 9 && (int)AsCWiiPauseScore(o)->displayedHoleCount >= AsCWiiPauseScore(o)->postHoleMode + 9) {
        AsCWiiPauseScore(o)->showFrontNine = 0;
        Layout_SetElementEnabled(AsCWiiPauseScore(o)->optionsLayout, kOptionFrontNine, 0);
        Layout_SetElementEnabled(AsCWiiPauseScore(o)->optionsLayout, kOptionBackNine, 1);
    } else {
        Layout_SetElementEnabled(AsCWiiPauseScore(o)->optionsLayout, kOptionFrontNine, 1);
        Layout_SetElementEnabled(AsCWiiPauseScore(o)->optionsLayout, kOptionBackNine, 0);
    }
    BpObject_SetEnabled(*(void**)(o + PAUSE_FIELD_OFFSET(scoreObject)), 0);
    BpObject_SetEnabled(AsCWiiPauseScore(o)->blackSprite, 0);
    if (AsCWiiPauseScore(o)->textTable != 0) *(int*)((char*)AsCWiiPauseScore(o)->textTable + 0x28) = 0;
    AsCWiiPauseScore(o)->interactionState = kScoreInteractive;
    AsCWiiPauseScore(o)->activationDelayFrames = 0;
    AsCWiiPauseScore(o)->actionDelayPending = 0;
    AsCWiiPauseScore(o)->activationPending = 1;
    unsigned int mode = AsCSharedDoc(g_sharedDoc)->gameMode;
    // Phase 4: map logical players to visual rows and position labels before refresh.
    if ((mode != kPartyRecordMode && AsCWiiPauseScore(o)->displayMode == kScoreParty) || (mode != kSinglePlayerRecordMode && AsCWiiPauseScore(o)->displayMode == kScoreStrokeAlternate)) {
        for (int i = 0; i < AsCWiiPauseScore(o)->playerCount; ++i) {
            o[PAUSE_INDEXED_OFFSET(playerRows) + i] = (unsigned char)((*(unsigned char*)((char*)GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i) + 0xc4) >> 4) & 0xf) - 1;
        }
    } else {
        for (int i = 0; i < AsCWiiPauseScore(o)->playerCount; ++i) o[PAUSE_INDEXED_OFFSET(playerRows) + i] = (unsigned char)i;
    }
    if (AsCWiiPauseScore(o)->playerCount > 1) {
        int i = 0;
        for (int j = 0; i < AsCWiiPauseScore(o)->playerCount; j += 2, ++i) {
            int jj = j + 1;
            if (SharedDoc_IsPlayerFlag0Set(i) != 0) {
                unsigned int m = AsCSharedDoc(g_sharedDoc)->gameMode;
                if ((m == 0 && AsCWiiPauseScore(o)->displayMode == kScoreMatch) || (m == 6 && AsCWiiPauseScore(o)->displayMode == kScoreMatchAlternate)) {
                    if (AsCWiiPauseScore(o)->alternatePlayerLabelLayout != 0) Layout_SetElementEnabled(AsCWiiPauseScore(o)->alternatePlayerLabelLayout, 0, 1);
                } else if (m == 6 && AsCWiiPauseScore(o)->displayMode == kScoreStrokeAlternate) {
                    Layout_SetElementPosition(AsCWiiPauseScore(o)->alternatePlayerLabelLayout, 0, o + 0x124 + o[PAUSE_INDEXED_OFFSET(playerRows) + i] * 8);
                    Layout_SetElementEnabled(AsCWiiPauseScore(o)->alternatePlayerLabelLayout, 0, 1);
                } else {
                    Layout_SetElementPosition(AsCWiiPauseScore(o)->playerLabelLayout, jj, o + 0x124 + o[PAUSE_INDEXED_OFFSET(playerRows) + i] * 8);
                    Layout_SetElementEnabled(AsCWiiPauseScore(o)->playerLabelLayout, jj, 1);
                }
            } else {
                Layout_SetElementPosition(AsCWiiPauseScore(o)->playerLabelLayout, j, o + 0x124 + o[PAUSE_INDEXED_OFFSET(playerRows) + i] * 8);
                Layout_SetElementEnabled(AsCWiiPauseScore(o)->playerLabelLayout, j, 1);
            }
        }
    }
    CWiiPauseScore_RefreshScorecard(self);
}
#pragma pop
void InputManager_Init(void* self);
void InputManager_Dtor(void* self);
int Input_IsActionPressed(void* self, int action, int player);
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[0x100];
extern char s_pauseInputManagerRegistration[0x100];

typedef LocalIntVector_u8032E804 ActivationList80330C54;

struct Object80330C54 {
    char data[0x860];
    // Construct the options dialog in its fixed-size allocation.
    Object80330C54() { COptionDialog_Ctor(this, 0); }
};
extern float kPauseScoreLayoutOpacity;
extern char pauseScorePartTotalGroupName[];
extern float kPauseScoreGraphHoleSpacing;
extern float kPauseScoreGraphHalfHoleSpacing;
extern signed char s_bpCtrlInitialized;
extern signed char s_pointerVectorInitialized;
extern char s_bpCtrl[0x100];
extern char s_pauseBpCtrlRegistration[0x100];
extern char s_fontNumDrawRegistry[0x100];
extern char s_pauseFontRegistryRegistration[0x100];
extern char __vt__14CWiiPauseScore[0x100];
void FadeOverlay_Init(void* self);
void ResourceState_Dtor(void* self);
void FadeState_StartFadeOut(void* self, int value);
void CBpCtrlHandle_SetScriptIndex(void* self, int value);
int MPBase1_Process(void* self);
int Layout_SetElementPlayerFlagsAndState(void* layout, int element, int visible);
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[0x100];
int CWiiPauseScore_ProcessOptionActions(void* self);
int CWiiPauseScore_ProcessMenuActions(void* self);
extern char s_gameInputManager[0x100];

struct VObj8032F954 {
    virtual void v1(int);
};

struct VBase8032F954 {
    int refs;
};
struct VObj50_8032F954 : VBase8032F954 {
    virtual void f00(int flag);
    virtual void f01();
    virtual void f02();
    virtual void f03();
    virtual void f04();
    virtual void f05();
    virtual void f06();
    virtual void f07();
    virtual void f08();
    virtual void f09();
    virtual void f10();
    virtual void f11();
    virtual void f12();
    virtual void f13();
    virtual void f14();
    virtual void f15();
    virtual void f16();
    virtual void f17();
    virtual void f18(const char* text);
    virtual int f19(float delta);
    virtual int f20();
};

#pragma push
#pragma use_lmw_stmw on
// Original 0x8032E804 unit: keep lazy manager creation before each input query.
// Lazily initialize and return the shared input manager.
static inline void* PauseScoreInputManager(char* registry) {
    if (Sing_805DC5C8() == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, registry + 0);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = s_gameInputManager;
    }
    return (void*)Sing_805DC5C8();
}

// Refresh the activated score screen and select its initial controller script.
static inline void ActivatePauseScorePresentation(void* self, char* obj, char* reg) {
        AsCWiiPauseScore(obj)->activationPending = 0;
        CWiiPauseScore_RefreshScorecard(self);
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->gameMode) == kSinglePlayerRecordMode && AsCWiiPauseScore(obj)->displayMode == kScoreStrokeAlternate && AsCWiiPauseScore(obj)->postHoleMode != 0) {
            if (Sing_805DC5F8() == 0) {
                if (s_bpCtrlHandleInitialized == 0) {
                    s_bpCtrlHandle[1] = -1;
                    s_bpCtrlHandle[0] = -1;
                    __register_global_object(&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (reg + 0x18));
                    s_bpCtrlHandleInitialized = 1;
                }
                Sing_805DC5F8() = s_bpCtrlHandle;
            }
            CBpCtrlHandle_SetScriptIndex((void*)Sing_805DC5F8(), kStrokePostHoleControllerScript);
        } else if (AsCWiiPauseScore(obj)->scorecardSuppressed == 0) {
            if (!Sing_805DC5F8()) {
                if (s_bpCtrlHandleInitialized == 0) {
                    s_bpCtrlHandle[1] = -1;
                    s_bpCtrlHandle[0] = -1;
                    __register_global_object(&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (reg + 0x18));
                    s_bpCtrlHandleInitialized = 1;
                }
                Sing_805DC5F8() = s_bpCtrlHandle;
            }
            CBpCtrlHandle_SetScriptIndex((void*)Sing_805DC5F8(), AsCWiiPauseScore(obj)->postHoleMode != 0 ? kPostHoleControllerScript : kPauseControllerScript);
        }
}

// Process score-screen activation, dialogs and menu input; return when an exit action completes.
// Return 0 while activation/options/confirmation remains pending. Nonzero is
// consumed by the parent task as completion; selectedAction supplies its reason.
// Tick priority: activation -> options -> confirmation -> confirmation-only
// creation -> ordinary input. Every earlier pending phase returns before input.
// interactionState records presentation state; live dialog pointers decide
// which modal branch owns the tick. Repeated singleton blocks preserve the
// original construction/registration order and all refer to the same storage.
int CWiiPauseScore_UpdateAndPollExit(void* self, float delta) {
    // Phase 1: defer presentation setup to the first tick after activation.
    char* reg = s_pauseInputManagerRegistration;
    char* obj = (char*)self;
    if (AsCWiiPauseScore(obj)->activationPending != 0) {
        ActivatePauseScorePresentation(self, obj, reg);
        return 0;
    }

    // Phase 2: an options dialog owns input until it completes; then restore each
    // remembered button's availability and the normal score presentation.
    void* old = AsCWiiPauseScore(obj)->optionDialog;
    if (old != 0) {
        if (MPBase1_Process(old) == 0) return 0;
        old = AsCWiiPauseScore(obj)->optionDialog;
        if (old != 0) {
            if (old) {
                ((VObj8032F954*)old)->v1(1);
            }
            AsCWiiPauseScore(obj)->optionDialog = 0;
        }
        if (Sing_805DC608() == 0) {
            if (s_fadeOverlayInitialized == 0) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, (reg + 0x24));
                s_fadeOverlayInitialized = 1;
            }
            Sing_805DC608() = s_fadeOverlay;
        }
        FadeState_StartFadeOut((void*)Sing_805DC608(), 0);
        AsCWiiPauseScore(obj)->interactionState = kScoreInteractive;
        switch (AsCWiiPauseScore(obj)->displayMode) {
        case kScoreStroke:
        case kScoreStrokeAlternate:
        case kScoreMatch:
        case kScoreParty:
        case kScoreMatchAlternate: {
            int saved;
            char* src;
            char* dst;
            int i;
            for (int k = 0; 4 > k; ++k) Layout_SetElementPlayerFlag(AsCWiiPauseScore(obj)->menuLayout, k, 1, 5);
            dst = obj;
            src = obj;
            for (i = 0; i <= 3; ++i, dst += 4, src += 4) {
                int state = AsCWiiPauseScore(obj)->displayMode;
                saved = AsCWiiPauseScore(src)->menuButtonAvailable0;
                if (state == kScoreConfirmationOnly) continue;
                switch (i) {
                case 4: case 5:
                    if (state != kScoreTraining) continue;
                case 0: case 1: case 2: case 3:
                    break;
                default: continue;
                }
                if (saved) {
                    Layout_SetElementPlayerFlag(AsCWiiPauseScore(obj)->menuLayout, i, 1, 5);
                    Layout_SetElementColor(AsCWiiPauseScore(obj)->menuLayout, i, -1);
                } else {
                    Layout_SetElementPlayerFlag(AsCWiiPauseScore(obj)->menuLayout, i, 0, 5);
                    Layout_SetElementColor(AsCWiiPauseScore(obj)->menuLayout, i, (int)0xB090B0B0);
                }
                AsCWiiPauseScore(dst)->menuButtonAvailable0 = saved;
            }
            break;
        }
        case kScoreTraining: { int i;
            char* src;

            char* dst;

            for (int k = 0; k < 5; ++k) Layout_SetElementPlayerFlag(AsCWiiPauseScore(obj)->menuLayout, k, 1, 5);
            dst = obj;
            src = obj;
            for (i = 0; 5 >= i; ++i, dst += 4, src += 4) {
                int state = AsCWiiPauseScore(obj)->displayMode;
                unsigned int saved; saved = AsCWiiPauseScore(src)->menuButtonAvailable0;
                if (kScoreConfirmationOnly == state) continue;
                switch (i) {
                case 4: case 5:
                    if (state != kScoreTraining) continue;
                case 0: case 1: case 2: case 3:
                    break;
                default: continue;
                }
                if (0 != saved) {
                    Layout_SetElementPlayerFlag(AsCWiiPauseScore(obj)->menuLayout, i, 1, 5);
                    Layout_SetElementColor(AsCWiiPauseScore(obj)->menuLayout, i, -1);
                } else {
                    Layout_SetElementPlayerFlag(AsCWiiPauseScore(obj)->menuLayout, i, 0, 5);
                    Layout_SetElementColor(AsCWiiPauseScore(obj)->menuLayout, i, (int)0xB090B0B0);
                }
                AsCWiiPauseScore(dst)->menuButtonAvailable0 = saved;
            }
            break;
        }
        default:
            break;
        }
        if (AsCWiiPauseScore(obj)->displayMode != kScoreTraining) {
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->menuLayout, 2, 1);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->menuLayout, 7, 0);
        } else {
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->menuLayout, 2, 1);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->menuLayout, 12, 0);
        }
        if (Sing_805DC5F8() == 0) {
            if (s_bpCtrlHandleInitialized == 0) {
                s_bpCtrlHandle[1] = -1;
                s_bpCtrlHandle[0] = -1;
                __register_global_object(&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (reg + 0x18));
                s_bpCtrlHandleInitialized = 1;
            }
            Sing_805DC5F8() = s_bpCtrlHandle;
        }
        CBpCtrlHandle_SetScriptIndex((void*)Sing_805DC5F8(), AsCWiiPauseScore(obj)->postHoleMode != 0 ? kPostHoleControllerScript : kPauseControllerScript);
        return 0;
    }

    // Phase 3: poll confirmation, translate acceptance to selectedAction, then
    // drop this dialog reference. Confirmation-only screens finish here.
    if (AsCWiiPauseScore(obj)->confirmationDialog != 0) {
        void* p50;
        if (Sing_805DC5F8() == 0) {
            if (s_bpCtrlHandleInitialized == 0) {
                s_bpCtrlHandle[1] = -1;
                s_bpCtrlHandle[0] = -1;
                __register_global_object(&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (reg + 0x18));
                s_bpCtrlHandleInitialized = 1;
            }
            Sing_805DC5F8() = s_bpCtrlHandle;
        }
        CBpCtrlHandle_ReleaseEntry((void*)Sing_805DC5F8());
        p50 = *(void**)(obj + PAUSE_FIELD_OFFSET(confirmationDialog));
        if (((VObj50_8032F954*)p50)->f19(delta) == 0) return 0;
        p50 = *(void**)(obj + PAUSE_FIELD_OFFSET(confirmationDialog));
        int enabled = ((VObj50_8032F954*)p50)->f20() == 1;
        AsCWiiPauseScore(obj)->selectedAction = (enabled != 0);
        p50 = *(void**)(obj + PAUSE_FIELD_OFFSET(confirmationDialog));
        if (p50 != 0) {
            if (--*(int*)p50 == 0) {
                if (p50 != 0) {
                    ((VObj50_8032F954*)p50)->f00(1);
                }
            }
            *(void**)(obj + PAUSE_FIELD_OFFSET(confirmationDialog)) = 0;
        }
        if (AsCWiiPauseScore(obj)->displayMode == kScoreConfirmationOnly) return 1;
        if (AsCWiiPauseScore(obj)->displayMode != kScoreTraining) {
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->menuLayout, 1, 1);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->menuLayout, 6, 0);
        } else {
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->menuLayout, 1, 1);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->menuLayout, 11, 0);
        }
        AsCWiiPauseScore(obj)->interactionState = kScoreInteractive;
        if (!enabled) {
            if (Sing_805DC5F8() == 0) {
                if (s_bpCtrlHandleInitialized == 0) {
                    s_bpCtrlHandle[0] = -1;
                    s_bpCtrlHandle[1] = -1;
                    __register_global_object(&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (reg + 0x18));
                    s_bpCtrlHandleInitialized = 1;
                }
                Sing_805DC5F8() = s_bpCtrlHandle;
            }
            CBpCtrlHandle_SetScriptIndex((void*)Sing_805DC5F8(), AsCWiiPauseScore(obj)->postHoleMode != 0 ? kPostHoleControllerScript : kPauseControllerScript);
        }
        return enabled;
    }

    // Phase 4: confirmation-only screens create their dialog without a scorecard.
    int category = AsCWiiPauseScore(obj)->displayMode;
    if (category == kScoreConfirmationOnly) {
        AsCWiiPauseScore(obj)->interactionState = kScoreConfirmOpen;

        if (6 > (AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->tutorialStage)) {
            CSmallCommonDialog_Create(0, obj + PAUSE_FIELD_OFFSET(confirmationDialog), 0x18);
            void* item = *(void**)(obj + PAUSE_FIELD_OFFSET(confirmationDialog));
            ((VObj50_8032F954*)item)->f18(pauseConfirmationText);
        } else if (category == kScoreMatchAlternate) {
            CSmallCommonDialog_Create(0, obj + PAUSE_FIELD_OFFSET(confirmationDialog), 5);
            void* item = *(void**)(obj + PAUSE_FIELD_OFFSET(confirmationDialog));
            ((VObj50_8032F954*)item)->f18(pauseConfirmationText + kPauseMatchTextOffset);
        } else {
            CSmallCommonDialog_Create(0, obj + PAUSE_FIELD_OFFSET(confirmationDialog), 5);
            void* item = *(void**)(obj + PAUSE_FIELD_OFFSET(confirmationDialog));
            ((VObj50_8032F954*)item)->f18(pauseConfirmationText + kPauseRoundTextOffset);
        }
        if (Sing_805DC5F8() == 0) {
            if (0 == s_bpCtrlHandleInitialized) {
                s_bpCtrlHandle[1] = -1;
                s_bpCtrlHandle[0] = -1;
                __register_global_object(&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (reg + 0x18));
                s_bpCtrlHandleInitialized = 1;
            }
            Sing_805DC5F8() = s_bpCtrlHandle;
        }
        CBpCtrlHandle_ReleaseEntry((void*)Sing_805DC5F8());
        return 0;
    }

    // Phase 5: process normal input/layout actions and latch the exit interaction state.
    void* target = AsCWiiPauseScore(obj)->textTable;
    if (0 != target) AsUITextTable(target)->drawEnabled = 0;
    unsigned int result = 0;
    // Both activation keys share the state transition; otherwise update layouts.
    if (AsCWiiPauseScore(obj)->postHoleMode == 0 && AsCWiiPauseScore(obj)->actionDelayPending == 0 &&
        (Input_IsActionPressed(PauseScoreInputManager(reg), kResumeInputAction, -1) != 0 ||
        Input_IsActionPressed(PauseScoreInputManager(reg), kAlternateResumeInputAction, -1) != 0)) {
        AsCWiiPauseScore(obj)->selectedAction = 0;
        result = 1;
    } else {
        Layout_Update(AsCWiiPauseScore(obj)->menuLayout);
        Layout_Update(AsCWiiPauseScore(obj)->optionsLayout);
        old = AsCWiiPauseScore(obj)->alternatePlayerLabelLayout;
        if (old != 0) Layout_Update(old);
        Layout_Update(AsCWiiPauseScore(obj)->playerLabelLayout);
        Layout_Update(AsCWiiPauseScore(obj)->tieMarkerLayout);
        Layout_Update(AsCWiiPauseScore(obj)->lossMarkerLayout);
        switch (AsCWiiPauseScore(obj)->displayMode) {
        case kScoreStroke:
        case kScoreStrokeAlternate:
        case kScoreMatch:
        case kScoreParty:
        case kScoreMatchAlternate:
            result = CWiiPauseScore_ProcessOptionActions(self);
            if (0 != result && AsCWiiPauseScore(obj)->selectedAction == 10) return 1;
            break;
        case kScoreTraining:
            result = CWiiPauseScore_ProcessMenuActions(self);
            break;
        default:
            break;
        }
    }
    if (result) {
        if (Sing_805DC5F8() == 0) {
            if (0 == s_bpCtrlHandleInitialized) {
                s_bpCtrlHandle[1] = -1;
                s_bpCtrlHandle[0] = -1;
                __register_global_object(&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (reg + 0x18));
                s_bpCtrlHandleInitialized = 1;
            }
            Sing_805DC5F8() = s_bpCtrlHandle;
        }
        CBpCtrlHandle_ReleaseEntry((void*)Sing_805DC5F8());
        AsCWiiPauseScore(obj)->interactionState = kScoreExiting;
    }
    return result;
}

#pragma pop
// Release score layouts, controller entries and owned hole/player score storage.
void* CWiiPauseScore_dtor(void* self, int deleting) {
    // Phase 1: delete each owned layout and clear its pointer.
    if (self != 0) {
        void* value = AsCWiiPauseScore(self)->menuLayout;
        *(void**)self = &__vt__14CWiiPauseScore;
        if (value != 0) {
            Layout_Destroy(value, 1);
            AsCWiiPauseScore(self)->menuLayout = 0;
        }
        value = AsCWiiPauseScore(self)->optionsLayout;
        if (value != 0) {
            Layout_Destroy(value, 1);
            AsCWiiPauseScore(self)->optionsLayout = 0;
        }
        value = AsCWiiPauseScore(self)->alternatePlayerLabelLayout;
        if (value != 0) {
            Layout_Destroy(value, 1);
            AsCWiiPauseScore(self)->alternatePlayerLabelLayout = 0;
        }
        value = AsCWiiPauseScore(self)->playerLabelLayout;
        if (0 != value) {
            Layout_Destroy(value, 1);
            AsCWiiPauseScore(self)->playerLabelLayout = 0;
        }
        value = AsCWiiPauseScore(self)->tieMarkerLayout;
        if (value != 0) {
            Layout_Destroy(value, 1);
            AsCWiiPauseScore(self)->tieMarkerLayout = 0;
        }
        value = AsCWiiPauseScore(self)->lossMarkerLayout;
        if (value != 0) {
            Layout_Destroy(value, 1);
            AsCWiiPauseScore(self)->lossMarkerLayout = 0;
        }
        // Phase 2: remove the two presentation entries through their registry owner.
        if (AsCWiiPauseScore(self)->scoreEntryId != -1) {
            if (((int&)Sing_805DC5F0()) == 0) {
                if (s_bpCtrlInitialized == 0) {
                    __ct__7CBpCtrlFv(&s_bpCtrl);
                    __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor, &s_pauseBpCtrlRegistration);
                    s_bpCtrlInitialized = 1;
                }
                ((int&)Sing_805DC5F0()) = (int)&s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), AsCWiiPauseScore(self)->scoreEntryId);
            AsCWiiPauseScore(self)->scoreEntryId = -1;
        }
        if (AsCWiiPauseScore(self)->blackSpriteEntryId != -1) {
            if (((int&)Sing_805DC5F0()) == 0) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(&s_bpCtrl);
                    __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor, &s_pauseBpCtrlRegistration);
                    s_bpCtrlInitialized = 1;
                }
                ((int&)Sing_805DC5F0()) = (int)&s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), AsCWiiPauseScore(self)->blackSpriteEntryId);
            AsCWiiPauseScore(self)->blackSpriteEntryId = -1;
        }
        // Phase 3: deregister numeric text, release score arrays, then optionally self.
        unsigned int clear = *(volatile int*)&AsCWiiPauseScore(self)->holeInfoCount;
        AsCWiiPauseScore(self)->holeInfoCount = clear - clear;
        AsCWiiPauseScore(self)->playerHoleScoreCount = clear - clear;
        clear = *(volatile int*)&AsCWiiPauseScore(self)->playerHoleScoreCount;
        value = AsCWiiPauseScore(self)->textTable;
        if (value != 0) {
            UITextTable_ClearEntries(value);
            if (((int&)Sing_805DC830()) == 0) {
                if (s_pointerVectorInitialized == 0) {
                    PointerVector_InitReserved(&s_fontNumDrawRegistry);
                    __register_global_object(&s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, &s_pauseFontRegistryRegistration);
                    s_pointerVectorInitialized = 1;
                }
                ((int&)Sing_805DC830()) = (int)&s_fontNumDrawRegistry;
            }
            PointerArray_DeleteValue((void*)((int&)Sing_805DC830()), (*(int*)&AsCWiiPauseScore(self)->textTable));
        }
        if (&AsCWiiPauseScore(self)->playerHoleScores != 0) {
            if (&AsCWiiPauseScore(self)->playerHoleScores != 0) {
                if (&AsCWiiPauseScore(self)->playerHoleScores != 0) {
                    if (&AsCWiiPauseScore(self)->playerHoleScores != 0) {
                        value = AsCWiiPauseScore(self)->playerHoleScores;
                        if (value != 0) {
                            clear = *(volatile int*)&AsCWiiPauseScore(self)->playerHoleScoreCount;
                            AsCWiiPauseScore(self)->playerHoleScoreCount = clear - clear;
                            appFreeUni__FPv(value);
                        }
                    }
                }
            }
        }
        if (&AsCWiiPauseScore(self)->holeInfo != 0) {
            if (&AsCWiiPauseScore(self)->holeInfo != 0) {
                if (&AsCWiiPauseScore(self)->holeInfo != 0) {
                    if (&AsCWiiPauseScore(self)->holeInfo != 0) {
                        value = AsCWiiPauseScore(self)->holeInfo;
                        if (value != 0) {
                            clear = *(volatile int*)&AsCWiiPauseScore(self)->holeInfoCount;
                            AsCWiiPauseScore(self)->holeInfoCount = clear - clear;
                            appFreeUni__FPv(value);
                        }
                    }
                }
            }
        }
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Return the selected score-screen action through the output pointer.
int Object_GetField14C(void* self, int* out) {
    *out = AsCWiiPauseScore(self)->selectedAction;
    return 1;
}

// Store the displayed hole number and par at an eight-bit hole index.
// holeIndex is truncated to 8 bits and must address allocated holeInfo. holeNumber
// is the display label, not the storage index. No resize or range check occurs.
void CWiiPauseScore_SetHoleInfo(void* self, int holeIndex, char holeNumber, int par) {
    ((PauseHoleInfo*)AsCWiiPauseScore(self)->holeInfo)[holeIndex & 0xff].holeNumber = holeNumber;
    ((PauseHoleInfo*)AsCWiiPauseScore(self)->holeInfo)[holeIndex & 0xff].par = par;
}

// Store strokes, the mode-specific competition value and Pang for one player on one hole.
// Indices are zero-based and must already fit the owned hole-major score array.
// competitionValue is mode-dependent (e.g. drive distance times 100 in party rule 2).
void CWiiPauseScore_SetPlayerHoleScore(void* self, int playerIndex, int holeIndex, int strokes, int competitionValue, int pang) {
    unsigned char c = AsCWiiPauseScore(self)->playerCount;
    int off = (playerIndex + holeIndex * (int)c) * sizeof(PausePlayerHoleScore);
    ((PausePlayerHoleScore*)((char*)AsCWiiPauseScore(self)->playerHoleScores + off))->strokes = strokes;
    ((PausePlayerHoleScore*)((char*)AsCWiiPauseScore(self)->playerHoleScores + off))->competitionValue = competitionValue;
    ((PausePlayerHoleScore*)((char*)AsCWiiPauseScore(self)->playerHoleScores + off))->pang = pang;
}

// Hide the score and black-sprite presentation objects when present.
void CWiiPauseScore_HideBpObjects(void* self) {
    void* value = AsCWiiPauseScore(self)->scoreObject;
    if (value != 0) {
        BpObject_SetEnabled(value, 0);
    }
    value = AsCWiiPauseScore(self)->blackSprite;
    if (value != 0) {
        BpObject_SetEnabled(value, 0);
    }
}

// Update a valid mode-specific menu button and remember its availability.
void CWiiPauseScore_SetMenuButtonAvailable(void* self, int index, int value) {
    int state = AsCWiiPauseScore(self)->displayMode;
    if (state == kScoreConfirmationOnly) {
        return;
    }
    if (index < 4) {
        if (index < 0) {
            return;
        }
    } else {
        if (index >= 6) {
            return;
        }
        if (state == kScoreTraining) {
        } else {
            return;
        }
    }
    if (value != 0) {
        Layout_SetElementPlayerFlag(AsCWiiPauseScore(self)->menuLayout, index, 1, 5);
        Layout_SetElementColor(AsCWiiPauseScore(self)->menuLayout, index, -1);
    } else {
        Layout_SetElementPlayerFlag(AsCWiiPauseScore(self)->menuLayout, index, 0, 5);
        Layout_SetElementColor(AsCWiiPauseScore(self)->menuLayout, index, (int)0xB090B0B0);
    }
    ((PauseIndexedFields*)self)->menuButtonAvailable[index] = value;
}

// Poll the score presentation and layouts until all required resources finish loading.
int CWiiPauseScore_PollLoadComplete(void* self) {
    if (!BpObject_PollLoad(AsCWiiPauseScore(self)->scoreObject)) {
        return 0;
    }
    BpObject_SetEnabled(AsCWiiPauseScore(self)->scoreObject, 0);
    if (!BpObject_PollLoad(AsCWiiPauseScore(self)->blackSprite)) {
        return 0;
    }
    BpObject_SetEnabled(AsCWiiPauseScore(self)->blackSprite, 0);
    if (AsCWiiPauseScore(self)->displayMode == kScoreConfirmationOnly) {
        return 1;
    }
    if (!Layout_PollLoadComplete(AsCWiiPauseScore(self)->menuLayout) ||
        !Layout_PollLoadComplete(AsCWiiPauseScore(self)->optionsLayout) ||
        !Layout_PollLoadComplete(AsCWiiPauseScore(self)->playerLabelLayout) ||
        !Layout_PollLoadComplete(AsCWiiPauseScore(self)->tieMarkerLayout) ||
        !Layout_PollLoadComplete(AsCWiiPauseScore(self)->lossMarkerLayout)) {
        return 0;
    }
    void* layout = AsCWiiPauseScore(self)->alternatePlayerLabelLayout;
    if (layout != 0 && !Layout_PollLoadComplete(layout)) {
        return 0;
    }
    return 1;
}

// Draw the visible score presentation, layouts, numeric entries and any active dialog.
void CWiiPauseScore_Draw(void* self) {
    int state = AsCWiiPauseScore(self)->displayMode;
    if (state != kScoreConfirmationOnly) {
        if (state != kScoreStroke) {
            BpObject_SetEnabled(AsCWiiPauseScore(self)->blackSprite, 1);
            BpObject_TryInvokeCurrentCallback(AsCWiiPauseScore(self)->blackSprite, 0);
            BpObject_Draw(AsCWiiPauseScore(self)->blackSprite);
            BpObject_SetEnabled(AsCWiiPauseScore(self)->scoreObject, 1);
            BpObject_TryInvokeCurrentCallback(AsCWiiPauseScore(self)->scoreObject, 0);
            BpObject_Draw(AsCWiiPauseScore(self)->scoreObject);
            *(int*)((*(char**)&AsCWiiPauseScore(self)->textTable) + 0x28) = 1;
            UITextTable_Draw(AsCWiiPauseScore(self)->textTable);
            if ((*(signed char*)&AsCWiiPauseScore(self)->selectedView) == kGraphView) {
                CWiiPauseScore_DrawScoreGraph(self);
            }
        }
        Layout_DrawWithOpacity(AsCWiiPauseScore(self)->menuLayout, kPauseScoreLayoutOpacity);
        Layout_DrawWithOpacity(AsCWiiPauseScore(self)->optionsLayout, kPauseScoreLayoutOpacity);
        void* layout = AsCWiiPauseScore(self)->alternatePlayerLabelLayout;
        if (layout != 0) {
            Layout_DrawWithOpacity(layout, kPauseScoreLayoutOpacity);
        }
        Layout_DrawWithOpacity(AsCWiiPauseScore(self)->playerLabelLayout, kPauseScoreLayoutOpacity);
        Layout_DrawWithOpacity(AsCWiiPauseScore(self)->tieMarkerLayout, kPauseScoreLayoutOpacity);
        Layout_DrawWithOpacity(AsCWiiPauseScore(self)->lossMarkerLayout, kPauseScoreLayoutOpacity);
    }
    void* object = AsCWiiPauseScore(self)->confirmationDialog;
    if (object != 0) {
        struct VirtualObject {
            int field00;
            virtual void slot08() DECOMP_LOCAL_VIRTUAL;
            virtual void slot0C() DECOMP_LOCAL_VIRTUAL;
            virtual void slot10() DECOMP_LOCAL_VIRTUAL;
            virtual void slot14() DECOMP_LOCAL_VIRTUAL;
            virtual void update() DECOMP_LOCAL_VIRTUAL;
        };
        ((VirtualObject*)object)->update();
    } else {
        object = AsCWiiPauseScore(self)->optionDialog;
        if (object != 0) {
            MPBase1_DrawIfInitialized(object);
        }
    }
}

// Hide the score presentation while it is inactive.
void CWiiPauseScore_HideInactiveScore(void* self) {
    if (AsCWiiPauseScore(self)->scorecardSuppressed == 0) {
        BpObject_SetEnabled(AsCWiiPauseScore(self)->scoreObject, 0);
        BpObject_SetEnabled(AsCWiiPauseScore(self)->blackSprite, 0);
        *(int*)((*(char**)&AsCWiiPauseScore(self)->textTable) + 0x28) = 0;
    }
}

// Process score-view/page changes and regular pause-menu actions, including delayed exits.
int CWiiPauseScore_ProcessOptionActions(void* self) {
    // Phase 1: finish the two-frame pressed-button delay before returning an action.
    char* obj = (char*)self;
    if (AsCWiiPauseScore(obj)->actionDelayPending != 0) {
        int frame = AsCWiiPauseScore(obj)->activationDelayFrames;
        if (frame < 2) {
            AsCWiiPauseScore(obj)->activationDelayFrames = frame + 1;
            return 0;
        }
        AsCWiiPauseScore(obj)->activationDelayFrames = 0;
        AsCWiiPauseScore(obj)->actionDelayPending = 0;
        return 1;
    }

    // Phase 2: consume option controls and refresh the score/page presentation.
    ActivationList80330C54 activated = { 0, 0, 0 };
    if (Layout_CollectActivated(AsCWiiPauseScore(obj)->optionsLayout, &activated, 0)) {
        int update = 1;
        switch (*(int*)activated.items) {
        case 2:
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionMatchView, 0);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, 15, 1);
            break;
        case 4:
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionPartyStrokeView, 0);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, 17, 1);
            break;
        case 5:
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionPartyAdjustedView, 0);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, 18, 1);
            break;
        case 6:
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionPartyCompetitionView, 0);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, 19, 1);
            break;
        default:
            update = 0;
            break;
        }
        if (update) {
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, *(signed char*)((char*)&AsCWiiPauseScore(obj)->selectedView) + 9, 0);
            int old = *(signed char*)((char*)&AsCWiiPauseScore(obj)->selectedView);
            int mask = ~((old - 1) | (1 - old));
            *(signed char*)((char*)&AsCWiiPauseScore(obj)->selectedView) = (old + 1) & ~(mask >> 31);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, *(signed char*)((char*)&AsCWiiPauseScore(obj)->selectedView) + 9, 1);
        }

        switch (*(int*)activated.items) {
        case 0:
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionStrokeView, 0);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, 13, 1);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, *(signed char*)((char*)&AsCWiiPauseScore(obj)->selectedView) + 9, 0);
            int old = *(signed char*)((char*)&AsCWiiPauseScore(obj)->selectedView);
            int mask = ~((old - 3) | (3 - old));
            *(signed char*)((char*)&AsCWiiPauseScore(obj)->selectedView) = (old + 1) & ~(mask >> 31);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, *(signed char*)((char*)&AsCWiiPauseScore(obj)->selectedView) + 9, 1);
            break;
        case 3:
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionNineHolePage, 0);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, 16, 1);
            int value = AsCWiiPauseScore(obj)->showFrontNine + 7;
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, value, 1);
            if (AsCWiiPauseScore(obj)->showFrontNine != 0) {
                AsCWiiPauseScore(obj)->showFrontNine = 0;
            } else {
                AsCWiiPauseScore(obj)->showFrontNine = 1;
            }
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, AsCWiiPauseScore(obj)->showFrontNine + 7, 0);
            break;
        default:
            break;
        }
        CWiiPauseScore_RefreshScorecard(self);
    } else {
        if (*(unsigned int*)((char*)&AsCWiiPauseScore(obj)->holeInfoCount) > 9) {
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionNineHolePage, 1);
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, 16, 0);
        }
        if (AsCWiiPauseScore(obj)->displayMode == kScoreTraining) {
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionTrainingView, 1);
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, 14, 0);
        } else if (AsCWiiPauseScore(obj)->displayMode == kScoreMatch || AsCWiiPauseScore(obj)->displayMode == kScoreMatchAlternate) {
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionMatchView, 1);
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, 15, 0);
        } else if (AsCWiiPauseScore(obj)->displayMode == kScoreParty) {
            if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartySumStrokeValues) {
                Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionPartyStrokeView, 1);
                Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, 17, 0);
            }
            if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartyLowerAdjustedValueWins) {
                Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionPartyAdjustedView, 1);
                Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, 18, 0);
            }
            if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartyHigherCompetitionValueWins) {
                Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionPartyCompetitionView, 1);
                Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, 19, 0);
            }
        } else {
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionStrokeView, 1);
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, 13, 0);
        }
    }

    // Phase 3: reuse activation storage for main-menu actions and dialogs.
    activated.size -= activated.size;
    if (Layout_CollectActivated(AsCWiiPauseScore(obj)->menuLayout, &activated, 0)) {
        switch (*(int*)activated.items) {
        case 1:
            AsCWiiPauseScore(obj)->selectedAction = 1;
            AsCWiiPauseScore(obj)->interactionState = kScoreConfirmOpen;
            int state = (*(int*)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            if (AsCGolfDoc(state)->tutorialStage < 6) {
                CSmallCommonDialog_Create(0, obj + PAUSE_FIELD_OFFSET(confirmationDialog), 0x18);
                void* object = AsCWiiPauseScore(obj)->confirmationDialog;
                (*(void (**)(void*, const char*))(*(char**)((char*)object + 4) + 0x50))(object, pauseConfirmationText);
            } else if (AsCWiiPauseScore(obj)->displayMode == kScoreMatchAlternate) {
                CSmallCommonDialog_Create(0, obj + PAUSE_FIELD_OFFSET(confirmationDialog), 5);
                void* object = AsCWiiPauseScore(obj)->confirmationDialog;
                (*(void (**)(void*, const char*))(*(char**)((char*)object + 4) + 0x50))(object, pauseConfirmationText + kPauseMatchTextOffset);
            } else {
                CSmallCommonDialog_Create(0, obj + PAUSE_FIELD_OFFSET(confirmationDialog), 5);
                void* object = AsCWiiPauseScore(obj)->confirmationDialog;
                (*(void (**)(void*, const char*))(*(char**)((char*)object + 4) + 0x50))(object, pauseConfirmationText + kPauseRoundTextOffset);
            }
            if (Sing_805DC5F8() == 0) {
                if (s_bpCtrlHandleInitialized == 0) {
                    s_bpCtrlHandle[0] = -1;
                    s_bpCtrlHandle[1] = -1;
                    __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_pauseBpHandleRegistration);
                    s_bpCtrlHandleInitialized = 1;
                }
                Sing_805DC5F8() = s_bpCtrlHandle;
            }
            CBpCtrlHandle_ReleaseEntry(Sing_805DC5F8());
            break;
        case 2:
            void* old = AsCWiiPauseScore(obj)->optionDialog;
            AsCWiiPauseScore(obj)->interactionState = kScoreOptionsOpen;
            if (old != 0) {
                if (old != 0) {
                    (*(void (**)(void*, int))((char*)*(void**)old + 8))(old, 1);
                    AsCWiiPauseScore(obj)->optionDialog = 0;
                }
            }
            Object80330C54* object = new Object80330C54;
            AsCWiiPauseScore(obj)->optionDialog = object;
            if (Sing_805DC5F8() == 0) {
                if (s_bpCtrlHandleInitialized == 0) {
                    s_bpCtrlHandle[0] = -1;
                    s_bpCtrlHandle[1] = -1;
                    __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_pauseBpHandleRegistration);
                    s_bpCtrlHandleInitialized = 1;
                }
                Sing_805DC5F8() = s_bpCtrlHandle;
            }
            CBpCtrlHandle_ReleaseEntry(Sing_805DC5F8());
            for (int i = 0; i < 4; ++i) {
                Layout_SetElementPlayerFlag(AsCWiiPauseScore(obj)->menuLayout, i, 0, 5);
            }
            break;
        default:
            break;
        }

        int control = 0;
        switch (*(int*)activated.items) {
        case 0:
            AsCWiiPauseScore(obj)->selectedAction = 0;
            control = 1;
            break;
        case 3:
            AsCWiiPauseScore(obj)->selectedAction = 3;
            control = 1;
            break;
        case 4:
            if (AsCWiiPauseScore(obj)->postHoleMode != 0 && AsCSharedDoc(g_sharedDoc)->courseRecordUpdatesEnabled != 0 &&
                *(int*)(obj + 0x94) == *(int*)(obj + 0x98) &&
                (AsCWiiPauseScore(obj)->displayMode == kScoreMatch || AsCWiiPauseScore(obj)->displayMode == kScoreMatchAlternate) &&
                *(signed char*)((char*)&AsCWiiPauseScore(obj)->selectedView) == 0) {
                Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionMatchView, 0);
                Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, 15, 1);
                Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, *(signed char*)((char*)&AsCWiiPauseScore(obj)->selectedView) + 9, 0);
                *(signed char*)((char*)&AsCWiiPauseScore(obj)->selectedView) = 1;
                Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, 10, 1);
                CWiiPauseScore_RefreshScorecard(self);
            } else {
                AsCWiiPauseScore(obj)->selectedAction = 6;
                control = 1;
            }
            break;
        default:
            break;
        }
        if (control || AsCWiiPauseScore(obj)->postHoleMode == 0) {
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->menuLayout, *(int*)activated.items, 0);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->menuLayout, *(int*)activated.items + 5, 1);
        }
        if (control) {
            AsCWiiPauseScore(obj)->activationDelayFrames = 0;
            AsCWiiPauseScore(obj)->actionDelayPending = 1;

            return 0;
        }
    }

    if (AsCWiiPauseScore(obj)->postHoleMode != 0 && AsCWiiPauseScore(obj)->displayMode == kScoreStrokeAlternate &&
        (*(int*)&AsCSharedDoc(g_sharedDoc)->gameMode) == 0) {
        if (Sing_805DC5C8() == 0) {
            if (s_inputManagerInitialized == 0) {
                InputManager_Init(&s_gameInputManager);
                __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, s_pauseInputManagerRegistration);
                s_inputManagerInitialized = 1;
            }
            Sing_805DC5C8() = &s_gameInputManager;
        }
        if (Input_IsActionPressed(Sing_805DC5C8(), kPostHoleInput3, -1) == 0) {
            if (Sing_805DC5C8() == 0) {
                if (s_inputManagerInitialized == 0) {
                    InputManager_Init(&s_gameInputManager);
                    __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, s_pauseInputManagerRegistration);
                    s_inputManagerInitialized = 1;
                }
                Sing_805DC5C8() = &s_gameInputManager;
            }
            if (Input_IsActionPressed(Sing_805DC5C8(), kPostHoleInput2, -1) != 0) {
                AsCWiiPauseScore(obj)->selectedAction = kPostHoleDismissAction;

                return 1;
            }
        } else {
            AsCWiiPauseScore(obj)->selectedAction = kPostHoleDismissAction;

            return 1;
        }
    }

    AsCWiiPauseScore(obj)->actionDelayPending = 0;

    return 0;
}

// Process training menu selections and their confirmation/options dialogs.
int CWiiPauseScore_ProcessMenuActions(void* self) {
    // Phase 1: return an action only after its two-frame pressed-button delay.
    struct TextObject {
        unsigned int field00;
        virtual void slot0C() DECOMP_LOCAL_VIRTUAL; virtual void slot08() DECOMP_LOCAL_VIRTUAL;
        virtual void slot18() DECOMP_LOCAL_VIRTUAL; virtual void slot1C() DECOMP_LOCAL_VIRTUAL; virtual void slot20() DECOMP_LOCAL_VIRTUAL; virtual void slot24() DECOMP_LOCAL_VIRTUAL;
        virtual void slot28() DECOMP_LOCAL_VIRTUAL; virtual void slot2C() DECOMP_LOCAL_VIRTUAL; virtual void slot30() DECOMP_LOCAL_VIRTUAL; virtual void slot34() DECOMP_LOCAL_VIRTUAL;
        virtual void slot38() DECOMP_LOCAL_VIRTUAL; virtual void slot3C() DECOMP_LOCAL_VIRTUAL; virtual void slot40() DECOMP_LOCAL_VIRTUAL; virtual void slot44() DECOMP_LOCAL_VIRTUAL;
        virtual void slot48() DECOMP_LOCAL_VIRTUAL; virtual void slot4C() DECOMP_LOCAL_VIRTUAL; virtual void slot50() DECOMP_LOCAL_VIRTUAL; virtual void slot54() DECOMP_LOCAL_VIRTUAL;
        virtual void setText(const char*) DECOMP_LOCAL_VIRTUAL;
        virtual void slot14() DECOMP_LOCAL_VIRTUAL; virtual void slot10() DECOMP_LOCAL_VIRTUAL;
    };
    char* obj = (char*)self;
    if (AsCWiiPauseScore(obj)->actionDelayPending != 0) {
        int frame = AsCWiiPauseScore(obj)->activationDelayFrames;
        if (2 > frame) {
            AsCWiiPauseScore(obj)->activationDelayFrames = (1 + frame);
            return 0;
        }
        AsCWiiPauseScore(obj)->activationDelayFrames = 0;
        AsCWiiPauseScore(obj)->actionDelayPending = 0;
        return 1;
    }

    // Phase 2: handle view/page options before collecting training-menu actions.
    ActivationList80330C54 activated = { 0, 0, 0 };
    if (Layout_CollectActivated(AsCWiiPauseScore(obj)->optionsLayout, &activated, 0)) {
        switch (*(int*)activated.items) {
        case kOptionTrainingView: {
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionTrainingView, 0);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, kOptionTrainingViewPressed, 1);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, *(signed char*)((char*)&AsCWiiPauseScore(obj)->selectedView) + kOptionViewIndicatorBase, 0);

            *(signed char*)((char*)&AsCWiiPauseScore(obj)->selectedView) = (*(signed char*)((char*)&AsCWiiPauseScore(obj)->selectedView)) == 1 ? 0 : 1;
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, *(signed char*)((char*)&AsCWiiPauseScore(obj)->selectedView) + kOptionViewIndicatorBase, 1);
            break;
        }
        case kOptionNineHolePage: {
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionNineHolePage, 0);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, kOptionNineHolePagePressed, 1);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, AsCWiiPauseScore(obj)->showFrontNine + kOptionPageIndicatorBase, 1);
            AsCWiiPauseScore(obj)->showFrontNine = !AsCWiiPauseScore(obj)->showFrontNine;
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->optionsLayout, AsCWiiPauseScore(obj)->showFrontNine + kOptionPageIndicatorBase, 0);
            break;
        }
        default:
            break;
        }
        CWiiPauseScore_RefreshScorecard(self);
    } else {
        if (*(unsigned int*)((char*)&AsCWiiPauseScore(obj)->holeInfoCount) > 9) {
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionNineHolePage, 1);
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionNineHolePagePressed, 0);
        }
        if (AsCWiiPauseScore(obj)->displayMode == kScoreTraining) {
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionTrainingView, 1);
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionTrainingViewPressed, 0);
        } else {
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, kOptionStrokeView, 1);
            Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->optionsLayout, 13, 0);
        }
    }

    // Phase 3: reuse activation storage for menu/dialog commands and the action delay.
    activated.size = *(unsigned int*)&activated.size - *(int*)&activated.size;
    if (Layout_CollectActivated(AsCWiiPauseScore(obj)->menuLayout, &activated, 0)) {
        Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->menuLayout, *(int*)activated.items, 0);
        Layout_SetElementEnabled(AsCWiiPauseScore(obj)->menuLayout, *(int*)activated.items + 10, 1);
        if (*(int*)activated.items >= 6) {
        Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(obj)->menuLayout, *(int*)activated.items + 14, 0);
            Layout_SetElementEnabled(AsCWiiPauseScore(obj)->menuLayout, *(int*)activated.items + 18, 1);
        }

        switch (*(int*)activated.items) {
        case 1: {
            AsCWiiPauseScore(obj)->selectedAction = 1;
            AsCWiiPauseScore(obj)->interactionState = kScoreConfirmOpen;
            void* state = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            if (AsCGolfDoc(state)->tutorialStage < 6) {
                CSmallCommonDialog_Create(0, obj + PAUSE_FIELD_OFFSET(confirmationDialog), 0x18);
                ((TextObject*)AsCWiiPauseScore(obj)->confirmationDialog)->setText(pauseConfirmationText);
            } else if (AsCWiiPauseScore(obj)->displayMode == kScoreMatchAlternate) {
                CSmallCommonDialog_Create(0, obj + PAUSE_FIELD_OFFSET(confirmationDialog), 5);
                ((TextObject*)AsCWiiPauseScore(obj)->confirmationDialog)->setText(pauseConfirmationText + kPauseMatchTextOffset);
            } else {
                CSmallCommonDialog_Create(0, obj + PAUSE_FIELD_OFFSET(confirmationDialog), 5);
                ((TextObject*)AsCWiiPauseScore(obj)->confirmationDialog)->setText(pauseConfirmationText + kPauseTrainingTextOffset);
            }
            if (0 == Sing_805DC5F8()) {
                if (0 == s_bpCtrlHandleInitialized) {
                    s_bpCtrlHandle[0] = -1;
                    s_bpCtrlHandle[1] = -1;
                    __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_pauseBpHandleRegistration);
                    s_bpCtrlHandleInitialized = 1;
                }
                Sing_805DC5F8() = s_bpCtrlHandle;
            }
            CBpCtrlHandle_ReleaseEntry(Sing_805DC5F8());
            break;
        }
        case 2: {
            void* old = AsCWiiPauseScore(obj)->optionDialog;
            AsCWiiPauseScore(obj)->interactionState = kScoreOptionsOpen;
            if (old) {
                struct Disposable { 
                virtual ~Disposable() DECOMP_LOCAL_VIRTUAL;
                };
                delete (Disposable*)old;
                AsCWiiPauseScore(obj)->optionDialog = 0;
            }
            Object80330C54* item = new Object80330C54;
            AsCWiiPauseScore(obj)->optionDialog = item;
            for (int i = 0; i < 5; ++i) {
                Layout_SetElementPlayerFlag(AsCWiiPauseScore(obj)->menuLayout, i, 0, 5);
            }
            if (!Sing_805DC5F8()) {
                if (!s_bpCtrlHandleInitialized) {
                    s_bpCtrlHandle[0] = -1;
                    s_bpCtrlHandle[1] = -1;
                    __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_pauseBpHandleRegistration);
                    s_bpCtrlHandleInitialized = 1;
                }
                Sing_805DC5F8() = s_bpCtrlHandle;
            }
            CBpCtrlHandle_ReleaseEntry(Sing_805DC5F8());
            break;
        }
        default:
            break;
        }

        unsigned int result = 1;
        switch (*(int*)activated.items) {
        case 0: AsCWiiPauseScore(obj)->selectedAction = 0; break;
        case 3: AsCWiiPauseScore(obj)->selectedAction = 3; break;
        case 4: AsCWiiPauseScore(obj)->selectedAction = 4; break;
        case 5: AsCWiiPauseScore(obj)->selectedAction = 5; break;
        case 6: AsCWiiPauseScore(obj)->selectedAction = 7; break;
        case 7: AsCWiiPauseScore(obj)->selectedAction = 8; break;
        case 8: AsCWiiPauseScore(obj)->selectedAction = 6; break;
        case 9: AsCWiiPauseScore(obj)->selectedAction = 9; break;
        default: result = 0; break;
        }
        if (result) {
            AsCWiiPauseScore(obj)->activationDelayFrames = 0;
            AsCWiiPauseScore(obj)->actionDelayPending = 1;

            return 0;
        }
    }
    AsCWiiPauseScore(obj)->actionDelayPending = 0;

    return 0;
}

// Update all score-screen layouts unless this is a confirmation-only screen.
void CWiiPauseScore_UpdateLayouts(void* self) {
    if (AsCWiiPauseScore(self)->displayMode != kScoreConfirmationOnly) {
        Layout_Update(AsCWiiPauseScore(self)->menuLayout);
        Layout_Update(AsCWiiPauseScore(self)->optionsLayout);
        void* value = AsCWiiPauseScore(self)->alternatePlayerLabelLayout;
        if (value != 0) {
            Layout_Update(value);
        }
        Layout_Update(AsCWiiPauseScore(self)->playerLabelLayout);
        Layout_Update(AsCWiiPauseScore(self)->tieMarkerLayout);
        Layout_Update(AsCWiiPauseScore(self)->lossMarkerLayout);
    }
}

// Release owned vector storage and optionally free the vector object.
void* dtor_80333BF0(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    void* value = *(void**)self;
                    if (value != 0) {
                    int count = *(volatile int*)((char*)self + 4);
                    *(volatile int*)((char*)self + 4) = count - count;
                        appFreeUni__FPv(value);
                    }
                }
            }
        }
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Release owned vector storage and optionally free the vector object.
void* dtor_80333C6C(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    void* value = *(void**)self;
                    if (value != 0) {
                    int count = *(volatile int*)((char*)self + 4);
                    *(volatile int*)((char*)self + 4) = count - count;
                        appFreeUni__FPv(value);
                    }
                }
            }
        }
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Set availability and colour of the fourth regular pause-menu button.
void CWiiPauseScore_SetMenuButton3Available(void* self, int value) {
    if (AsCWiiPauseScore(self)->displayMode == kScoreConfirmationOnly) {
        return;
    }
    if (AsCWiiPauseScore(self)->displayMode == kScoreConfirmationOnly) {
        return;
    }
    if (value != 0) {
        Layout_SetElementPlayerFlag(AsCWiiPauseScore(self)->menuLayout, 3, 1, 5);
        Layout_SetElementColor(AsCWiiPauseScore(self)->menuLayout, 3, -1);
    } else {
        Layout_SetElementPlayerFlag(AsCWiiPauseScore(self)->menuLayout, 3, 0, 5);
        Layout_SetElementColor(AsCWiiPauseScore(self)->menuLayout, 3, (int)0xB090B0B0);
    }
    ((PauseIndexedFields*)self)->menuButtonAvailable[3] = value;
}

void UIGroup_SetTintColor(void* self, int value);
void Layout_SetElementEnabled(void* self, int index, int value);
void* __nwa__FUl(unsigned int size);
void* appMallocDummy__FUl(unsigned int size);
void appFreeUni__FPv(void* self);
void UITextTable_SetInteger(void* self, int value, int index);
extern void* g_sharedDoc;
extern char wiiPauseScoreStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern char pauseScorePartTotalGroupName[];
struct I4 { int v[4]; };
extern I4 s_pauseScoreTieMarkerBases;
extern I4 s_pauseScoreLossMarkerBases;

void CWiiPauseScore_RankPartyHoleResults(void* self, int* result, int* selection);
void NumericTextTable_ClearEntryText(void* self, int value);
int PartyCourseTable_FindCourseHolePosition(int a, int b);
#pragma push
#pragma use_lmw_stmw on
#pragma push
#pragma inline_max_size(10000)
#pragma inline_max_total_size(20000)
// Read the four-bit player type from the borrowed round-player entry.
static inline unsigned int PauseScorePlayerType(int playerIndex) {
    struct PlayerInfo { char pad[0xc4]; CW_BITS2( unsigned char type : 4, unsigned char other : 4) };
    return ((PlayerInfo*)GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)playerIndex))->type;
}

// Format a one-based visual cell name into caller-provided name storage.
static inline void PauseScoreFormatCellName(char* name, char* strings, unsigned char* scoreBytes, int firstHole, int holeIndex, int playerIndex) {
    sprintf(name, strings + kWiiPauseScoreStrings_R_NUM_result_d_K4_LC_FM,
            (holeIndex - firstHole) + scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + playerIndex] * kHolesPerPage + 1);
}

// Publish one score column using descending numeric IDs and mapped visual rows.
static inline void PauseScorePutCell(unsigned char* scoreBytes, int firstHole, int holeIndex, int playerIndex, int fieldOffset) {
    UITextTable_SetInteger(AsCWiiPauseScore(scoreBytes)->textTable,
        (*(int*)(scoreBytes + PAUSE_INDEXED_OFFSET(cellTextId)) - (holeIndex - firstHole)) - scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + playerIndex] * kHolesPerPage,
        PauseScoreCell(AsCWiiPauseScore(scoreBytes)->playerHoleScores, AsCWiiPauseScore(scoreBytes)->playerCount, holeIndex, playerIndex, fieldOffset));
}

#pragma pop

// Populate stroke, match, party, training and Pang views for the selected nine-hole page.
void CWiiPauseScore_RefreshScorecard(void* self) {
    // Phase 1: select the page, clear prior cells/markers, and publish hole/par labels.
    unsigned char* scoreBytes = (unsigned char*)self;
    char* cfg = pauseScorePartTotalGroupName;
    if (AsCWiiPauseScore(scoreBytes)->scorecardSuppressed) return;
    // These are the original absolute loop limits, not counts to add to firstPageHole.
    // The back-nine branch caps at 9 in the original; preserve that legacy bound.
    int firstPageHole = AsCWiiPauseScore(scoreBytes)->showFrontNine ? 0 : kHolesPerPage;
    int completedCellLimit = (int)AsCWiiPauseScore(scoreBytes)->displayedHoleCount - firstPageHole <= kHolesPerPage ? (int)AsCWiiPauseScore(scoreBytes)->displayedHoleCount : kHolesPerPage;
    int labelCellLimit = (int)AsCWiiPauseScore(scoreBytes)->holeCount - firstPageHole <= kHolesPerPage ? (int)AsCWiiPauseScore(scoreBytes)->holeCount : kHolesPerPage;
    int limitsA[5];
    int limitsB[5];
    struct Limits { int v[5]; };
    *(Limits*)limitsA = *(Limits*)(((char*)(cfg + 0x2810)));
    *(Limits*)limitsB = *(Limits*)(((char*)(cfg + 0x2824)));
    int parTotal = 0;
    for (int i = 0; i <= limitsA[AsCWiiPauseScore(scoreBytes)->playerCount]; ++i) Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(scoreBytes)->tieMarkerLayout, i, 0);
    for (int i = 0; i <= limitsB[AsCWiiPauseScore(scoreBytes)->playerCount]; ++i) Layout_SetElementPlayerFlagsAndState(AsCWiiPauseScore(scoreBytes)->lossMarkerLayout, i, 0);
    char name[72];
    char* s = (char*)wiiPauseScoreStrings;
    for (int holeIndex = 1; holeIndex <= kHolesPerPage; ++holeIndex) for (int playerIndex = 1; playerIndex <= AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex) {
        sprintf(name, s + kWiiPauseScoreStrings_R_score_record_d_d, playerIndex, holeIndex);
        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 0);
    }
    for (int i = 0; i < kHolesPerPage; ++i) {
        sprintf(name, s + kWiiPauseScoreStrings_R_NUM_hole_d_K2_LC, i + 1);
        UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), -1);
        NumericTextTable_ClearEntryText(AsCWiiPauseScore(scoreBytes)->textTable, *(int*)(scoreBytes + PAUSE_INDEXED_OFFSET(holeNumberTextId)) - i);
        NumericTextTable_ClearEntryText(AsCWiiPauseScore(scoreBytes)->textTable, *(int*)(scoreBytes + PAUSE_INDEXED_OFFSET(parTextId)) - i);
    }
    if (AsCWiiPauseScore(scoreBytes)->displayMode == kScoreParty && (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartySumStrokeValues) {
        for (int i = 0; i < AsCWiiPauseScore(scoreBytes)->holeCount; ++i) {
            int x = *((unsigned char*)*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(holeInfo)) + i * 8);
            UITextTable_SetInteger(AsCWiiPauseScore(scoreBytes)->textTable, *(int*)(scoreBytes + PAUSE_INDEXED_OFFSET(holeNumberTextId)) - i,
                        PartyCourseTable_FindCourseHolePosition(((PartyRoundCourseView*)g_sharedDoc)->courseIndices[i], x));
        }
    } else for (int i = firstPageHole; i < labelCellLimit; ++i)
        UITextTable_SetInteger(AsCWiiPauseScore(scoreBytes)->textTable, *(int*)(scoreBytes + PAUSE_INDEXED_OFFSET(holeNumberTextId)) - (i - firstPageHole),
                    *((unsigned char*)*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(holeInfo)) + i * 8));
    for (int i = firstPageHole; i < labelCellLimit; ++i) {
        UITextTable_SetInteger(AsCWiiPauseScore(scoreBytes)->textTable, *(int*)(scoreBytes + PAUSE_INDEXED_OFFSET(parTextId)) - (i - firstPageHole),
                    *(int*)((char*)*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(holeInfo)) + i * 8 + 4));
        parTotal += *(int*)((char*)*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(holeInfo)) + i * 8 + 4);
    }
    if (AsCWiiPauseScore(scoreBytes)->displayMode != kScoreMatch && AsCWiiPauseScore(scoreBytes)->displayMode != kScoreMatchAlternate && AsCWiiPauseScore(scoreBytes)->displayMode != kScoreParty)
        UITextTable_SetInteger(AsCWiiPauseScore(scoreBytes)->textTable, *(int*)(scoreBytes + PAUSE_INDEXED_OFFSET(parTotalTextId)), parTotal);
    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), s + kWiiPauseScoreStrings_R_score_subtotal), 1);
    for (int i = 0; i < AsCWiiPauseScore(scoreBytes)->playerCount * kHolesPerPage; ++i) {
        sprintf(name, s + kWiiPauseScoreStrings_R_NUM_result_d_K4_LC_FM, i + 1);
        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 0);
    }
    for (int i = 0; i < AsCWiiPauseScore(scoreBytes)->playerCount; ++i) {
        sprintf(name, s + kWiiPauseScoreStrings_R_NUM_rtotal_d_K5_LC_FM, i + 1);
        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 0);
    }
    // Phase 2: compute completed-hole totals: match wins, party sole wins/sums,
    // or strokes relative to cumulative par. Then publish each player's header.
    // Accumulate in logical player order; apply playerRows only during UI publication.
    int totals[4] = {};
    if (AsCWiiPauseScore(scoreBytes)->displayMode == kScoreMatch || AsCWiiPauseScore(scoreBytes)->displayMode == kScoreMatchAlternate) {
        for (int i = 0; i < AsCWiiPauseScore(scoreBytes)->displayedHoleCount; ++i) {
            int ix = i * AsCWiiPauseScore(scoreBytes)->playerCount;
            int player1Strokes = *(int*)((char*)AsCWiiPauseScore(scoreBytes)->playerHoleScores + (ix + 1) * 12);
            int player0Strokes = *(int*)((char*)AsCWiiPauseScore(scoreBytes)->playerHoleScores + ix * 12);
            if (player0Strokes < player1Strokes) ++totals[0]; else if (player0Strokes != player1Strokes) ++totals[1];
        }
    } else if (AsCWiiPauseScore(scoreBytes)->displayMode == kScoreParty) {
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartySumStrokeValues) {
            int span = completedCellLimit + 1 - firstPageHole;
            for (int holeIndex = 0; holeIndex < AsCWiiPauseScore(scoreBytes)->displayedHoleCount + (AsCWiiPauseScore(scoreBytes)->postHoleMode == 0 && span <= kHolesPerPage); ++holeIndex)
                for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex)
                    totals[playerIndex] += *(int*)((char*)AsCWiiPauseScore(scoreBytes)->playerHoleScores + (holeIndex * AsCWiiPauseScore(scoreBytes)->playerCount + playerIndex) * 12);
        } else CWiiPauseScore_RankPartyHoleResults(self, totals, 0);
    } else {
    for (int holeIndex = 0; holeIndex < AsCWiiPauseScore(scoreBytes)->displayedHoleCount; ++holeIndex) for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex)
        totals[playerIndex] += *(int*)((char*)AsCWiiPauseScore(scoreBytes)->playerHoleScores + (holeIndex * AsCWiiPauseScore(scoreBytes)->playerCount + playerIndex) * 12);
    int total = 0;
    for (int i = 0, off = 0; i < AsCWiiPauseScore(scoreBytes)->displayedHoleCount; ++i, off += 8) total += *(int*)((char*)*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(holeInfo)) + off + 4);
    for (int i = 0; i < AsCWiiPauseScore(scoreBytes)->playerCount; ++i) totals[i] -= total;
    }
    int usesFixedPointScoreLabel = 0;
    if (AsCWiiPauseScore(scoreBytes)->displayMode == kScoreStrokeAlternate || AsCWiiPauseScore(scoreBytes)->displayMode == kScoreTraining) usesFixedPointScoreLabel = 1;
    for (int i = 0; i < AsCWiiPauseScore(scoreBytes)->playerCount; ++i) {
        if (usesFixedPointScoreLabel) sprintf(name, s + kWiiPauseScoreStrings_R_NUM_scr_d_K3_LC_FM_FP, scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + i] + 1);
        else sprintf(name, s + kWiiPauseScoreStrings_R_NUM_rscore_d_K3_LC_FM, scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + i] + 1);
        if (AsCSharedDoc(g_sharedDoc)->courseRecordUpdatesEnabled &&
            PauseScorePlayerType(i) == 1)
            UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 0xfffdce73);
        else UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), -1);
        UITextTable_SetInteger(AsCWiiPauseScore(scoreBytes)->textTable, *(int*)(scoreBytes + 0x84 + scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + i] * 4), totals[i]);
        totals[i] = 0; // Header publication is complete; reuse totals for view footers.
    }
    // Phase 3: fill the selected view. Cell names count upward from 1, while
    // numeric entry IDs count DOWN from cellTextId by page hole and visual row.
    // In-progress-hole inclusion depends on display mode; preserve each loop's bound.
    if ((signed char)AsCWiiPauseScore(scoreBytes)->selectedView == kPrimaryView) {
        if (AsCWiiPauseScore(scoreBytes)->displayMode == kScoreParty) {
            if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartySumStrokeValues) {
                int span = completedCellLimit + 1 - firstPageHole;
                for (int holeIndex = firstPageHole; holeIndex < completedCellLimit + (AsCWiiPauseScore(scoreBytes)->postHoleMode == 0 && span <= kHolesPerPage); ++holeIndex)
                    for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex) {
                        PauseScoreFormatCellName(name, s, scoreBytes, firstPageHole, holeIndex, playerIndex);
                        UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), -1);
                        PauseScorePutCell(scoreBytes, firstPageHole, holeIndex, playerIndex, kCellStrokes);
                    }
            } else CWiiPauseScore_RankPartyHoleResults(self, 0, (int*)1);
            UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), s + kWiiPauseScoreStrings_R_score_subtotal), 0);
            return;
        } else if (AsCWiiPauseScore(scoreBytes)->displayMode == kScoreMatch || AsCWiiPauseScore(scoreBytes)->displayMode == kScoreMatchAlternate) {
            if (AsCWiiPauseScore(scoreBytes)->playerCount != 2) return;
            for (int holeIndex = firstPageHole; holeIndex < completedCellLimit; ++holeIndex) {
                int y = PauseScoreCell(AsCWiiPauseScore(scoreBytes)->playerHoleScores, AsCWiiPauseScore(scoreBytes)->playerCount,holeIndex,1,0);
                int x = PauseScoreCell(AsCWiiPauseScore(scoreBytes)->playerHoleScores, AsCWiiPauseScore(scoreBytes)->playerCount,holeIndex,0,0);
                if (x < y) {
                    sprintf(name, s + kWiiPauseScoreStrings_R_score_record1_d, holeIndex - firstPageHole + 1);
                    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 1);
                    Layout_SetElementEnabled(AsCWiiPauseScore(scoreBytes)->lossMarkerLayout, holeIndex - firstPageHole + kHolesPerPage, 1);
                } else if (x == y) {
                    Layout_SetElementEnabled(AsCWiiPauseScore(scoreBytes)->tieMarkerLayout, holeIndex - firstPageHole, 1);
                    Layout_SetElementEnabled(AsCWiiPauseScore(scoreBytes)->tieMarkerLayout, holeIndex - firstPageHole + kHolesPerPage, 1);
                } else {
                    sprintf(name, s + kWiiPauseScoreStrings_R_score_record2_d, holeIndex - firstPageHole + 1);
                    UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 1);
                    Layout_SetElementEnabled(AsCWiiPauseScore(scoreBytes)->lossMarkerLayout, holeIndex - firstPageHole, 1);
                }
            }
            if (AsCWiiPauseScore(scoreBytes)->postHoleMode == 0 &&
                (AsCWiiPauseScore(scoreBytes)->showFrontNine != 0 || AsCWiiPauseScore(scoreBytes)->displayedHoleCount >= kHolesPerPage) &&
                (AsCWiiPauseScore(scoreBytes)->showFrontNine == 0 || AsCWiiPauseScore(scoreBytes)->displayedHoleCount < kHolesPerPage)) {
                for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex) {
                    sprintf(name, s + kWiiPauseScoreStrings_R_NUM_result_d_K4_LC_FM, AsCWiiPauseScore(scoreBytes)->displayedHoleCount - firstPageHole + scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + playerIndex] * kHolesPerPage + 1);
                    UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), -1);
                    PauseScorePutCell(scoreBytes, firstPageHole, completedCellLimit, playerIndex, 0);
                }
            }
            UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), s + kWiiPauseScoreStrings_R_score_subtotal), 0);
            return;
        } else {
            int* colors = (int*)(((char*)(cfg + 0x2698)));
            int colorMax = kTripleBogeyDelta;
            for (int holeIndex = firstPageHole; holeIndex < completedCellLimit; ++holeIndex) for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex) {
                int score = *(int*)((char*)AsCWiiPauseScore(scoreBytes)->playerHoleScores + ((unsigned char)holeIndex * AsCWiiPauseScore(scoreBytes)->playerCount + (unsigned char)playerIndex) * 12);
                int color;
                if (score == kHoleInOneStrokes) color = kHoleInOneColour;
                else {
                    int delta = score - *(int*)((char*)*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(holeInfo)) + (unsigned char)holeIndex * 8 + 4);
                    if (delta <= kAlbatrossDelta) color = kAlbatrossOrBetterColour;
                    else color = colorMax <= delta ? kTripleBogeyOrWorseColour : delta + kParColour;
                }
                PauseScoreFormatCellName(name, s, scoreBytes, firstPageHole, holeIndex, playerIndex);
                UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), colors[color]);
                PauseScorePutCell(scoreBytes, firstPageHole, holeIndex, playerIndex, kCellStrokes);
            }
            // Accumulate this column across completed holes.
            for (int holeIndex = 0; holeIndex < AsCWiiPauseScore(scoreBytes)->displayedHoleCount; ++holeIndex)
                for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex)
                    totals[playerIndex] += PauseScoreCell(AsCWiiPauseScore(scoreBytes)->playerHoleScores, AsCWiiPauseScore(scoreBytes)->playerCount, holeIndex, playerIndex, kCellStrokes);
            // Publish row totals; only the requested player type receives record tint.
            for (int ii = 0; ii < AsCWiiPauseScore(scoreBytes)->playerCount; ++ii) {
                sprintf(name, s + kWiiPauseScoreStrings_R_NUM_rtotal_d_K5_LC_FM, scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + ii] + 1);
                if ((1) && AsCSharedDoc(g_sharedDoc)->courseRecordUpdatesEnabled && PauseScorePlayerType(ii) == 1)
                    UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 0xfffdce73);
                else UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), -1);
                UITextTable_SetInteger(AsCWiiPauseScore(scoreBytes)->textTable, *(int*)(scoreBytes + 0x74 + scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + ii] * 4), totals[ii]);
            }
            return;
        }
        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), s + kWiiPauseScoreStrings_R_score_subtotal), 0);
    } else if ((signed char)AsCWiiPauseScore(scoreBytes)->selectedView == kSecondaryView) {
        if (AsCWiiPauseScore(scoreBytes)->displayMode == kScoreTraining) {
            for (int holeIndex = firstPageHole; holeIndex < completedCellLimit; ++holeIndex) for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex) {
                PauseScoreFormatCellName(name, s, scoreBytes, firstPageHole, holeIndex, playerIndex);
                UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 0xff55c355);
                PauseScorePutCell(scoreBytes, firstPageHole, holeIndex, playerIndex, kCellCompetition);
            }
            // Accumulate this column across completed holes.
            for (int holeIndex = 0; holeIndex < AsCWiiPauseScore(scoreBytes)->displayedHoleCount; ++holeIndex)
                for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex)
                    totals[playerIndex] += PauseScoreCell(AsCWiiPauseScore(scoreBytes)->playerHoleScores, AsCWiiPauseScore(scoreBytes)->playerCount, holeIndex, playerIndex, kCellCompetition);
            // Publish row totals; only the requested player type receives record tint.
            for (int ii = 0; ii < AsCWiiPauseScore(scoreBytes)->playerCount; ++ii) {
                sprintf(name, s + kWiiPauseScoreStrings_R_NUM_rtotal_d_K5_LC_FM, scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + ii] + 1);
                if ((0) && AsCSharedDoc(g_sharedDoc)->courseRecordUpdatesEnabled && PauseScorePlayerType(ii) == 1)
                    UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 0xfffdce73);
                else UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), -1);
                UITextTable_SetInteger(AsCWiiPauseScore(scoreBytes)->textTable, *(int*)(scoreBytes + 0x74 + scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + ii] * 4), totals[ii]);
            }
            return;
        }
        if (AsCWiiPauseScore(scoreBytes)->displayMode == kScoreParty && (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartyLowerAdjustedValueWins) {
            int span = completedCellLimit + 1 - firstPageHole;
            for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex)
                for (int holeIndex = firstPageHole; holeIndex < completedCellLimit + (AsCWiiPauseScore(scoreBytes)->postHoleMode == 0 && span <= kHolesPerPage); ++holeIndex) {
                    PauseScoreFormatCellName(name, s, scoreBytes, firstPageHole, holeIndex, playerIndex);
                    UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), -1);
                    // Keep the scalar stroke load here; a typed array changes the loop induction code.
                    int score = *(int*)((char*)AsCWiiPauseScore(scoreBytes)->playerHoleScores + (holeIndex * AsCWiiPauseScore(scoreBytes)->playerCount + playerIndex) * kPlayerHoleScoreBytes + kCellStrokes);
                    UITextTable_SetInteger(AsCWiiPauseScore(scoreBytes)->textTable, (*(int*)(scoreBytes + PAUSE_INDEXED_OFFSET(cellTextId)) - (holeIndex-firstPageHole)) - scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + playerIndex]*kHolesPerPage, score);
                    totals[playerIndex] += score;
                }
            UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), s + kWiiPauseScoreStrings_R_score_subtotal), 0);
            return;
        }
        int span = completedCellLimit + 1 - firstPageHole;
        for (int holeIndex = firstPageHole; holeIndex < completedCellLimit + (AsCWiiPauseScore(scoreBytes)->postHoleMode == 0 && span <= kHolesPerPage); ++holeIndex)
            for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex) {
                PauseScoreFormatCellName(name, s, scoreBytes, firstPageHole, holeIndex, playerIndex);
                if (AsCSharedDoc(g_sharedDoc)->courseRecordUpdatesEnabled &&
                    PauseScorePlayerType(playerIndex) == 1)
                    UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 0xfffdce73);
                else UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 0xff4991f3);
                PauseScorePutCell(scoreBytes, firstPageHole, holeIndex, playerIndex, kCellPang);
            }
        unsigned char* from = scoreBytes;
        for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex, from += 4) totals[playerIndex] = *(int*)(from + 0xa4);
        // Publish row totals; only the requested player type receives record tint.
        for (int ii = 0; ii < AsCWiiPauseScore(scoreBytes)->playerCount; ++ii) {
            sprintf(name, s + kWiiPauseScoreStrings_R_NUM_rtotal_d_K5_LC_FM, scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + ii] + 1);
            if ((1) && AsCSharedDoc(g_sharedDoc)->courseRecordUpdatesEnabled && PauseScorePlayerType(ii) == 1)
                UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 0xfffdce73);
            else UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), -1);
            UITextTable_SetInteger(AsCWiiPauseScore(scoreBytes)->textTable, *(int*)(scoreBytes + 0x74 + scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + ii] * 4), totals[ii]);
        }
    } else if ((signed char)AsCWiiPauseScore(scoreBytes)->selectedView == kCompetitionView) {
        for (int holeIndex = firstPageHole; holeIndex < completedCellLimit; ++holeIndex) for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex) {
            PauseScoreFormatCellName(name, s, scoreBytes, firstPageHole, holeIndex, playerIndex);
            UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 0xff55c355);
            PauseScorePutCell(scoreBytes, firstPageHole, holeIndex, playerIndex, kCellCompetition);
        }
        // Accumulate this column across completed holes.
        for (int holeIndex = 0; holeIndex < AsCWiiPauseScore(scoreBytes)->displayedHoleCount; ++holeIndex)
            for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex)
                totals[playerIndex] += PauseScoreCell(AsCWiiPauseScore(scoreBytes)->playerHoleScores, AsCWiiPauseScore(scoreBytes)->playerCount, holeIndex, playerIndex, kCellCompetition);
        // Publish row totals; only the requested player type receives record tint.
        for (int ii = 0; ii < AsCWiiPauseScore(scoreBytes)->playerCount; ++ii) {
            sprintf(name, s + kWiiPauseScoreStrings_R_NUM_rtotal_d_K5_LC_FM, scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + ii] + 1);
            if ((0) && AsCSharedDoc(g_sharedDoc)->courseRecordUpdatesEnabled && PauseScorePlayerType(ii) == 1)
                UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 0xfffdce73);
            else UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), -1);
            UITextTable_SetInteger(AsCWiiPauseScore(scoreBytes)->textTable, *(int*)(scoreBytes + 0x74 + scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + ii] * 4), totals[ii]);
        }
    } else if ((signed char)AsCWiiPauseScore(scoreBytes)->selectedView == kGraphView) {
        // Accumulate this column across completed holes.
        for (int holeIndex = 0; holeIndex < AsCWiiPauseScore(scoreBytes)->displayedHoleCount; ++holeIndex)
            for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex)
                totals[playerIndex] += PauseScoreCell(AsCWiiPauseScore(scoreBytes)->playerHoleScores, AsCWiiPauseScore(scoreBytes)->playerCount, holeIndex, playerIndex, kCellStrokes);
        // Publish row totals; only the requested player type receives record tint.
        for (int ii = 0; ii < AsCWiiPauseScore(scoreBytes)->playerCount; ++ii) {
            sprintf(name, s + kWiiPauseScoreStrings_R_NUM_rtotal_d_K5_LC_FM, scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + ii] + 1);
            if ((0) && AsCSharedDoc(g_sharedDoc)->courseRecordUpdatesEnabled && PauseScorePlayerType(ii) == 1)
                UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 0xfffdce73);
            else UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), -1);
            UITextTable_SetInteger(AsCWiiPauseScore(scoreBytes)->textTable, *(int*)(scoreBytes + 0x74 + scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + ii] * 4), totals[ii]);
        }
    }
}
#pragma pop


void* memset(void*, int, unsigned int);
void* memcpy(void*, const void*, unsigned int);
unsigned int strlen(const char*);
char* strcpy(char*, const char*);
void* __nw__FUl(unsigned int);
void __dla__FPv(void*);
void __throw(const char*, void*, void*);
void __dt__Q23std12length_errorFv(void*, int);

} // extern "C"

// Storage/exception support ends immediately before RankPartyHoleResults below.
// This block contains two ABI views of the same integer-vector operations:
// std::vector supplies the template specialization; LocalIntVector supplies
// the stack view used for activation lists and ranking scratch.
// Both grow -> allocate guarded storage -> append zeroes -> copy prefix ->
// exchange ownership, with a shared MSL length_error path. Neither implements
// ranking policy. Keep duplicated bodies/exception boundaries for exact codegen.
// The resize operation retains size/capacity checks even for an empty vector.
#pragma push
#pragma inline_max_size(10000)
#pragma inline_max_total_size(20000)
namespace std {
namespace detail {
struct CharArrayGuard_u8032E804 {
    char* pointer;
    char* allocator;
    // Own temporary character storage until a shared control block adopts it.
    inline CharArrayGuard_u8032E804(char* p, char* a) : pointer(p), allocator(a) {}
    // Free the temporary character array unless ownership was released.
    inline ~CharArrayGuard_u8032E804() { if (pointer) __dla__FPv(pointer); }
    // Transfer temporary character storage to its shared owner.
    inline char* release() { char* p = pointer; pointer = 0; return p; }
};
}
}
struct ScoreLengthError {
    void* vtable;
    char* text;
    struct SharedText {
        void* vtable;
        unsigned int owners, references;
        char* text;
    } *shared;
    // Copy the length-error text into shared owned character storage.
    inline ScoreLengthError(const char* message) {
        char allocator[4];
        allocator[0] = 0;
        vtable = __vt__Q23std11logic_error;
        text = new char[strlen(message) + 1];
        {
            std::detail::CharArrayGuard_u8032E804 guard(text, allocator);
            char* buffer = text;
            SharedText* control = (SharedText*)__nw__FUl(sizeof(SharedText));
            if (control) {
                control->owners = 1;
                control->references = 1;
                control->vtable = s_sharedCharDeleterVtable;
                control->text = buffer;
            }
            shared = control;
            guard.release();
        }
        strcpy(text, message);
        vtable = __vt__Q23std12length_error;
    }
};

// Throw the runtime vector length exception using the score string pool.
inline void scoreLengthError() {
    ScoreLengthError error(wiiPauseScoreStrings + kWiiPauseScoreStrings_vector_length_error);
    __throw(wiiPauseScoreStrings + kWiiPauseScoreStrings_std_exception_std_logic_error_std_length_error, &error,
            (void*)__dt__Q23std12length_errorFv);
}

// Select the larger capacity increment by reference.
inline const unsigned int& scoreMax(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

// Count integer entries between two vector pointers.
inline int scoreDistance(const int* first, const int* last) {
    return last - first;
}

// Exchange vector storage metadata without changing ownership.
template <class T> inline void scoreSwap(T& first, T& second) {
    T value = first;
    first = second;
    second = value;
}

struct ScoreBuffer {
    int* data;
    unsigned int size, capacity;
    unsigned int* originalCapacity;
    unsigned int before;
    char allocator;
    // Create empty growth storage and borrow the original-capacity reference.
    inline ScoreBuffer(unsigned int& original)
        : data(0), size(0), capacity(0), originalCapacity(&original), before(0) {}
    // Release whichever integer allocation remains owned after vector exchange.
    inline ~ScoreBuffer() {
        if (data) { size = 0; appFreeUni__FPv(data); }
    }
    // Allocate guarded integer storage for the requested element capacity.
    inline void allocate(unsigned int count) {
        if (count > 0x3fffffffu) scoreLengthError();
        data = (int*)appMallocDummy__FUl(count * 4);
        capacity = count;
    }
};

namespace std {
    // Choose the new capacity while enforcing the integer-vector limit.
    inline unsigned int vector<int, app::app_allocator<int> >::grow(unsigned int count) {
        unsigned int current = *(volatile unsigned int*)&capacity;
        if (count > 0x3fffffffu - current) scoreLengthError();
        if (current < 0x15555555u)
            return current + scoreMax((current + 1) * 3 / 5, count);
        if (current < 0x2aaaaaaau)
            return current + scoreMax((current + 1) / 2, count);
        return 0x3fffffffu;
    }
    // Allocate the owned integer array for the requested element capacity.
    inline void vector<int, app::app_allocator<int> >::allocate(unsigned int count) {
        if (count > 0x3fffffffu) scoreLengthError();
        items = (int*)appMallocDummy__FUl(count * 4);
        capacity = count;
    }
    // Append zero-valued integers, growing and exchanging storage if necessary.
    inline void vector<int, app::app_allocator<int> >::append(unsigned int count) {
        // Validate the requested insertion before constructing its temporary
        // buffer; allocation also validates its independently chosen capacity.
        if (count > capacity || (*(unsigned int*)&size) > capacity - count)
            (void)grow(size + count);
        if (count <= capacity - (*(unsigned int*)&size)) {
            memset(items + (*(unsigned int*)&size), 0, count * 4);
            (*(unsigned int*)&size) += count;
        } else {
            ScoreBuffer buffer(capacity);
            buffer.allocate(grow((*(unsigned int*)&size) + count - *buffer.originalCapacity));
            buffer.before = (*(unsigned int*)&size);
            memset(buffer.data + buffer.before + buffer.size, 0, count * 4);
            buffer.size += count;
            int* old = items;
            int copied = scoreDistance(old, items + (*(unsigned int*)&size));
            buffer.before -= copied;
            memcpy(buffer.data + buffer.before, old, copied * 4);
            memset(old, 0, copied * 4);
            unsigned int newSize = buffer.size + copied;
            int* previous = items;
            items = buffer.data;
            scoreSwap(capacity, buffer.capacity);
            buffer.data = previous;
            (*(unsigned int*)&size) = newSize;
            buffer.size = 0;
        }
    }
    // Grow with zero-valued entries or reduce the logical element count.
    inline void vector<int, app::app_allocator<int> >::resize(unsigned int count) {
        if (count > (*(unsigned int*)&size)) vector<int, app::app_allocator<int> >::append(count - (*(unsigned int*)&size));
        else if (count < (*(unsigned int*)&size)) (*(unsigned int*)&size) -= (*(unsigned int*)&size) - count;
    }
    // Borrow one integer entry; the caller supplies an in-range index.
    inline int& vector<int, app::app_allocator<int> >::operator[](int index) { return items[index]; }
}
// Matching operations on the stack-local view.

    // Choose the ranking scratch capacity with the same integer-vector limit.
    inline unsigned int LocalIntVector_u8032E804::grow(unsigned int count) {
        unsigned int current = *(volatile unsigned int*)&capacity;
        if (count > 0x3fffffffu - current) scoreLengthError();
        if (current < 0x15555555u)
            return current + scoreMax((current + 1) * 3 / 5, count);
        if (current < 0x2aaaaaaau)
            return current + scoreMax((current + 1) / 2, count);
        return 0x3fffffffu;
    }
    // Allocate the owned integer array for the requested element capacity.
    inline void LocalIntVector_u8032E804::allocate(unsigned int count) {
        if (count > 0x3fffffffu) scoreLengthError();
        items = (int*)appMallocDummy__FUl(count * 4);
        capacity = count;
    }
    // Append zero-valued integers, growing and exchanging storage if necessary.
    inline void LocalIntVector_u8032E804::append(unsigned int count) {
        // Validate the requested insertion before constructing its temporary
        // buffer; allocation also validates its independently chosen capacity.
        if (count > capacity || (*(unsigned int*)&size) > capacity - count)
            (void)grow(size + count);
        if (count <= capacity - (*(unsigned int*)&size)) {
            memset(items + (*(unsigned int*)&size), 0, count * 4);
            (*(unsigned int*)&size) += count;
        } else {
            ScoreBuffer buffer(capacity);
            buffer.allocate(grow((*(unsigned int*)&size) + count - *buffer.originalCapacity));
            buffer.before = (*(unsigned int*)&size);
            memset(buffer.data + buffer.before + buffer.size, 0, count * 4);
            buffer.size += count;
            int* old = items;
            int copied = scoreDistance(old, items + (*(unsigned int*)&size));
            buffer.before -= copied;
            memcpy(buffer.data + buffer.before, old, copied * 4);
            memset(old, 0, copied * 4);
            unsigned int newSize = buffer.size + copied;
            int* previous = items;
            items = buffer.data;
            scoreSwap(capacity, buffer.capacity);
            buffer.data = previous;
            (*(unsigned int*)&size) = newSize;
            buffer.size = 0;
        }
    }
    // Grow with zero-valued entries or reduce the logical element count.
    inline void LocalIntVector_u8032E804::resize(unsigned int count) {
        if (count > (*(unsigned int*)&size)) LocalIntVector_u8032E804::append(count - (*(unsigned int*)&size));
        else if (count < (*(unsigned int*)&size)) (*(unsigned int*)&size) -= (*(unsigned int*)&size) - count;
    }
    // Borrow one integer entry; the caller supplies an in-range index.
    inline int& LocalIntVector_u8032E804::operator[](int index) { return items[index]; }


extern "C" {
#pragma push
#pragma use_lmw_stmw on
// Rank party-game hole results, optionally accumulate sole wins and update winner/tie indicators.
// Require initialized self; 1..4 players; 1..18 allocated holes, and a valid
// displayed/page interval inside that hole count. scores are native SIGNED
// 32-bit {strokes,competitionValue,pang} per hole/player; Pang is unused here.
// Rules other than 1 compare competitionValue directly: 0..INT_MAX competes,
// any negative value is excluded (not just -1); larger wins. Rule 1 computes
// competitionValue-strokes BEFORE testing negativity; require that subtraction
// to fit [-2147483648,2147483647]. Then 0..INT_MAX competes and smaller wins.
// There is no narrower enforced score cap, saturation or overflow recovery.
// In particular a negative source operand can still yield a valid nonnegative
// difference; do not discard it before subtraction. Rule 3's sum belongs to
// RefreshScorecard, not this comparison path. Ranking counts are at most 3.
// Optional soleWinCounts[players] is an accumulator: reserve headroom for every
// sole win visited (at most 9 per page call). Negative starting counts are not
// validated. Single-player post-hole instead overwrites [0] with 0/1 according
// to player+0xC4's result nibble <3, bypassing all score arithmetic.
// playerRows must map to 0..3; indicator layouts/BP/text resources must be ready
// if updateIndicators!=0. The final unfinished-hole display reads index end, so
// when that path runs require end<holeCount even for an empty rank interval.
// This documents the existing front/back end calculation exactly: end starts
// at 9; only displayedHoleCount-first<=9 replaces it with displayedHoleCount.
// A caller changing pagination must preserve/validate that boundary separately.
void CWiiPauseScore_RankPartyHoleResults(void* self, int* soleWinCounts, int* updateIndicators) {
    // updateIndicators is a boolean ABI flag; callers may pass (int*)1, and it is never dereferenced.
    unsigned char* scoreBytes = (unsigned char*)self;
    I4 tieMarkerBases = s_pauseScoreTieMarkerBases;
    I4 lossMarkerBases = s_pauseScoreLossMarkerBases;
    char name[0x40];

    if (AsCWiiPauseScore(scoreBytes)->postHoleMode != 0 && AsCWiiPauseScore(scoreBytes)->playerCount == 1) {
        unsigned char* player = (unsigned char*)GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), 0);
        if ((unsigned int)((player[0xc4] >> 4) & 0xF) < 3u) {
            if (soleWinCounts) *soleWinCounts = 1;
            if (updateIndicators) {
                sprintf(name, wiiPauseScoreStrings + kWiiPauseScoreStrings_R_score_record_d_d, 1, 1);
                UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 1);
            }
        } else {
            if (soleWinCounts) *soleWinCounts = 0;
            if (updateIndicators) Layout_SetElementEnabled(AsCWiiPauseScore(scoreBytes)->lossMarkerLayout, lossMarkerBases.v[0], 1);
        }
        return;
    }

    // Phase 1: materialize comparable hole values: competition-strokes (lower
    // wins) for rule 1, competition (higher wins) otherwise. Negative values
    // receive rank -1 and do not compete; rank 0 is a best valid value.
    int end = kHolesPerPage;
    int first = AsCWiiPauseScore(scoreBytes)->showFrontNine != 0 ? 0 : kHolesPerPage;
    if ((int)AsCWiiPauseScore(scoreBytes)->displayedHoleCount - first <= kHolesPerPage) end = AsCWiiPauseScore(scoreBytes)->displayedHoleCount;

    LocalIntVector_u8032E804 holeValues = {0, 0, 0};
    unsigned int total = (unsigned int)AsCWiiPauseScore(scoreBytes)->holeCount * AsCWiiPauseScore(scoreBytes)->playerCount;
    holeValues.resize(total);

    int isSpecialPartyRule = (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartyLowerAdjustedValueWins;
    int preferHigherValue = 0;
    if (isSpecialPartyRule) {
        for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex) {
            for (int holeIndex = first; holeIndex < end; ++holeIndex) {
                int* rec = (int*)(*(unsigned char**)(scoreBytes + 0xe4) + (playerIndex + holeIndex * AsCWiiPauseScore(scoreBytes)->playerCount) * 12);
                holeValues[playerIndex + holeIndex * AsCWiiPauseScore(scoreBytes)->playerCount] = rec[1] - rec[0];
            }
        }
    } else {
        preferHigherValue = 1;
        for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex) {
            for (int holeIndex = first; holeIndex < end; ++holeIndex) {
                int* rec = (int*)(*(unsigned char**)(scoreBytes + 0xe4) + (playerIndex + holeIndex * AsCWiiPauseScore(scoreBytes)->playerCount) * 12);
                holeValues[playerIndex + holeIndex * AsCWiiPauseScore(scoreBytes)->playerCount] = rec[1];
            }
        }
    }

    // Phase 2: rank each completed hole, count tied best values, accumulate only
    // sole wins, and optionally publish winner/tie/loss markers for rule 1.
    int ranks[4];
    int winnerCountMinusOne;
    for (int holeIndex = first; holeIndex < end; ++holeIndex) {
        for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex) {
            ranks[playerIndex] = -1;
            int playerValue = holeValues[playerIndex + holeIndex * AsCWiiPauseScore(scoreBytes)->playerCount];
            if (playerValue < 0) continue;
            ++ranks[playerIndex];
            for (int other = 0; other < AsCWiiPauseScore(scoreBytes)->playerCount; ++other) {
                int otherValue = holeValues[other + holeIndex * AsCWiiPauseScore(scoreBytes)->playerCount];
                if (playerIndex != other && otherValue >= 0) {
                    if (preferHigherValue) {
                        if (playerValue < otherValue) ++ranks[playerIndex];
                    } else {
                        if (playerValue > otherValue) ++ranks[playerIndex];
                    }
                }
            }
        }
        winnerCountMinusOne = -1;
        for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex) {
            if (ranks[playerIndex] == 0) ++winnerCountMinusOne;
        }
        if (soleWinCounts) {
            for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex) {
                if (ranks[playerIndex] == 0 && winnerCountMinusOne == 0) ++soleWinCounts[playerIndex];
            }
        }
        if (updateIndicators && isSpecialPartyRule) {
            int rowIndex = holeIndex - first;
            for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex) {
                if (ranks[playerIndex] == 0) {
                    if (winnerCountMinusOne == 0) {
                        sprintf(name, wiiPauseScoreStrings + kWiiPauseScoreStrings_R_score_record_d_d, scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + playerIndex] + 1, rowIndex + 1);
                        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), 1);
                    } else {
                        Layout_SetElementEnabled(AsCWiiPauseScore(scoreBytes)->tieMarkerLayout, rowIndex + tieMarkerBases.v[scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + playerIndex]], 1);
                    }
                } else if (winnerCountMinusOne < 0) {
                    Layout_SetElementEnabled(AsCWiiPauseScore(scoreBytes)->tieMarkerLayout, rowIndex + tieMarkerBases.v[scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + playerIndex]], 1);
                } else {
                    Layout_SetElementEnabled(AsCWiiPauseScore(scoreBytes)->lossMarkerLayout, rowIndex + lossMarkerBases.v[scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + playerIndex]], 1);
                }
            }
        }
    }

    // Phase 3: optionally display the current unfinished hole's stroke values.
    // soleWinCounts is an accumulator (not reset here), except the single-player
    // post-hole special case above which assigns its first word directly.
    if (!updateIndicators) return;
    if (AsCWiiPauseScore(scoreBytes)->postHoleMode != 0 && isSpecialPartyRule) return;
    if (AsCWiiPauseScore(scoreBytes)->postHoleMode != 0) end = first;
    int count = end - first;
            for (int playerIndex = 0; playerIndex < AsCWiiPauseScore(scoreBytes)->playerCount; ++playerIndex) {
                int idx = playerIndex + end * AsCWiiPauseScore(scoreBytes)->playerCount;
                holeValues[idx] = *(int*)(*(unsigned char**)(scoreBytes + 0xe4) + idx * 12);
                sprintf(name, wiiPauseScoreStrings + kWiiPauseScoreStrings_R_NUM_result_d_K4_LC_FM, count + scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + playerIndex] * kHolesPerPage + 1);
                UIGroup_SetTintColor(BpObject_GetOrCreateNamedGroup(*(void**)(scoreBytes + PAUSE_FIELD_OFFSET(scoreObject)), name), -1);
                UITextTable_SetInteger(AsCWiiPauseScore(scoreBytes)->textTable, *(int*)(scoreBytes + PAUSE_INDEXED_OFFSET(cellTextId)) - count - scoreBytes[PAUSE_INDEXED_OFFSET(playerRows) + playerIndex] * kHolesPerPage, holeValues[idx]);
            }
}
#pragma pop
#pragma pop

} // extern "C"
