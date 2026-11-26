// Tour main task: the tour mode's main task, which runs the menus, the ending movie and the save overlay.
// Completing a course node commits its save and rewards. Also the wind HUD (CBallWind2d) and the edge
// codes that link the tour map's grid nodes.

#include <decomp/static_init.h>
extern "C" char s_tourProgressStateStorage[];
#include "classes/CSharedDoc.h"

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CTourMain.h"
#include "classes/MenuGroup802FB4D0.h"
#include "classes/CBallWind2d.h"
#include "classes/CFlashMemoryMenuSave.h"
#include "classes/CTask.h"

// Borrow the wind HUD view; no storage is created or transferred.
static inline CBallWind2d* AsCBallWind2d(void* p) {
    return (CBallWind2d*)p;
}

// Borrow the tour task using its checked shared layout.
static inline CTourMain* AsCTourMain(void* p) {
    return (CTourMain*)p;
}

// State values are shared by OnProcess and OnDisplay; pendingState commits on the next update.
enum TourPhase {
    kTourIdle = 0,
    kTourInitialize = 1,
    kTourChooseMenu = 2,
    kTourCreateAreaSelection = 3,
    kTourSelectArea = 4,
    kTourCreateItemMenu = 5,
    kTourItemMenu = 6,
    kTourLoadEndingMovie = 7,
    kTourWaitForEndingMovie = 8,
    kTourPlayEndingMovie = 9,
    kTourCloseEndingMovie = 10,
    kTourStartSave = 11,
    kTourSave = 12,
    kTourFinished = 13
};
enum TourMenuResult128 {
    kTourMenuBack = -1,
    kTourMenuStartGame = 1,
    kTourMenuRebuild = 2,
    kTourMenuPending = 3
};
enum TourMovieConstants128 {
    kTourMovieBufferBytes = 0x100000,
    kTourMovieLoadMode = 4,
    kTourMovieExitActionA = 0x11,
    kTourMovieExitActionB = 0x12,
    kTourMovieClosePlaybackState = 2,
    kTourMovieFadeDuration = 8,
    kTourTaskRequestKind = 8
};
enum TourExit {
    kTourNoExit = 0,
    kTourReturnToMenu = 1,
    kTourStartGame = 2
};
enum {
    kTourMapColumns = 5,
    kTourMapRows = 7,
    kHorizontalEdgeCodeBase = 100,
    kTourModeSaveBytes = 0x33C
};
enum TourNodeBits {
    kNodeRewardCounted = 0x10,
    kEdgeDown = 0x20,
    kEdgeUp = 0x40,
    kEdgeRight = 0x80,
    kEdgeLeft = 0x100,
    kNodeRewardsApplied = 0x200,
    kNodePending = 0x400,
    kNodeCompleted = 0x8000
};
enum {
    kFirstWindElement = 6,
    kLastWindElement = 21,
    kWindFeedbackElement = 2
};
// Wind HUD IDs used here: 6..21 are the selectable wind elements (all disabled
// on initialization); element 2 has feedback reset to state 0. Constructor
// visibility flags cover 0, 22..29 and a separately drawn element 3. Their
// artwork/control identities are not established by this initialization unit.
// Tour display map: phase 4=area menu, 6=map/item menu, 9/10=ending movie,
// 12=save menu; other phases emit no draw calls from this task.
// Selection results: area >0=selected, <0=save/exit, 0=pending;
// map/item -1=back, 1=start game through save, 2=recreate menu, 3=no transition.
enum {
    kTourCoursesPerAreaReward = 2,
    kTourOtherRewardsPerArea = 7,
    kTourAreaUnlocked = 0x80,
    kTourAllAreasCleared = 0x20
};
struct TourAreaSave {
    unsigned char flags;
    char padding01;
    short currentNode;
    unsigned short nodes[35]; // seven rows of five nodes
};
struct TourModeSave {
    TourAreaSave areas[11];
    signed char areaOrder[8];
    signed char currentArea;
    char padding337;
    unsigned short rewardCount;
    unsigned char flags;
    char padding33B;
};
// currentArea indexes areaOrder (eight progression slots); the resulting value
// indexes the eleven physical area records. currentNode indexes a 7x5 node grid.
// Node bits: 0x10 prevents duplicate numeric rewards, 0x200 records reward
// application, 0x400 is pending, and 0x8000 is completed. Edge bits are reciprocal.
// TourProgressContext borrows the task's player-record copy until task destruction.
// Packed string pool; empty title transition name is a separate one-byte entry.
struct TourReturnStrings {
    char soundBank[8];    // SE_tour
    char endingMovie[10]; // staffroll
    char titleTask[11];   // CTitleTask
    char emptyTransition[1];
    char titleMain[11];   // TitleMain2
    char golfTask[10];    // CGolfTask
    char golfDocument[9]; // CGolfDoc
};
enum {
    kEndingMovieNameOffset = (int)&((TourReturnStrings*)0)->endingMovie,
    kTitleTaskNameOffset = (int)&((TourReturnStrings*)0)->titleTask,
    kEmptyTransitionNameOffset = (int)&((TourReturnStrings*)0)->emptyTransition,
    kTitleMainNameOffset = (int)&((TourReturnStrings*)0)->titleMain,
    kGolfTaskNameOffset = (int)&((TourReturnStrings*)0)->golfTask,
    kGolfDocumentNameOffset = (int)&((TourReturnStrings*)0)->golfDocument
};
struct TourPlayerRecordView {
    char prefix[0x4EC];
    TourModeSave modes;
};
struct TourPlayerRewardView {
    char prefix[0xD0];
    int pendingReward;
};
struct TourProgressContext {
    void* playerRecord;
    unsigned char edgeRewardState;
    char padding005[3];
    int edgeIndex;
    char padding00C[0x18];
    int selectionResult;
    int initialized;
    int endingMoviePending;
    int areaCleared;
    char padding034[4];
};
#define TOUR_OFFSET(type, field) ((int)&((type*)0)->field)

extern "C" {

extern float kFloat_6_2831855;
extern float s_tourTaskFullTurn;
extern float s_ballWindHudFullTurn;
extern float kFloat_0_BallWindHud;
extern char __vt__9CTourMain[];
extern char __vt__11CBallWind2d[];
extern signed char s_itemRequestStateInitialized;
extern char s_itemRequestState[];
extern char s_itemRequestStateDtorRegistration_tourMain[];
extern char s_tourProgressStateStorage[];
extern void* g_soundManager;
extern char tourReturnTaskStrings[];
extern char s_fadeOverlay[];
extern signed char s_movieControllerInitialized;
extern char s_movieController[];
extern char s_tourTaskMovieDtorRegistration[];
extern volatile signed char s_saveDataInitialized;
extern unsigned char s_saveData[];
extern CSharedDoc* g_sharedDoc;
void SoundManager_AcquireSoundBank(void*, void*);
void* memset(void*, int, unsigned);
extern short s_tourAreaCourseRewards[];
extern short s_tourAreaOtherRewardIndices[];
void __ct__18NativeBase80201D18Fv(void*);
void SaveData_InitPlayerRecord(void*);
void Layout_Init(void*, int);
void __dt__18NativeBase80201D18Fv(void*, int);
void ItemRequestState_Ctor(void*);
extern char ItemRequestState_Dtor[];
void TourItemRequest_DispatchDisplay(void*);
void ItemRequestState_UpdateCompletion(void*);
void ItemRequestState_UpdateCompletion(void*);
void ItemRequestState_ClearEntries(void*);

void SoundManager_ReleaseSoundBank(void*, void*);
void TourMap_DestroyScreenAndPreviewTree(void*, int);
void TourAreaSelect_Destroy(void*, int);
void TourMenuGroup_DrawActiveEntries(void*);
void MovieController_Ctor(void*);
extern char MovieController_Dtor[];
void MoviePlayer_DrawCurrentSamples(void*);
void MPBase1_DrawIfInitialized(void*);
void SaveData_Ctor(void*);
void* SaveData_GetCurrentRecord(void*);
void SaveData_SetModeRecords(void*, void*);
void ItemRequestState_RequestCourseByInventoryIndex(void*, int, int);
void SaveRewards_SetOtherNotificationBit(void*, int);
void ItemRequestState_ApplyPendingRewards(void*);
void ItemRequestState_MarkTextNotifications(void*);
void* GolfPlayer_GetEntryByIndex(void*, int);
int SharedStringState_GetWordValue();
int SharedStringState_GetShortValue();
int EventScript_Player0UnbeatenWinsAtLeastOneThird(int, int);
#pragma push
#pragma always_inline on
// Construct/register the lazy owner once, then publish its singleton address.
inline void EnsureTourItemRequests(void* registration) {
    if (((int&)Sing_805DC958()) == 0) {
        if (s_itemRequestStateInitialized == 0) {
            ItemRequestState_Ctor(s_itemRequestState);
            __register_global_object(s_itemRequestState, ItemRequestState_Dtor, registration);
            s_itemRequestStateInitialized = 1;
        }
        ((int&)Sing_805DC958()) = (int)s_itemRequestState;
    }
}
// Construct/register the lazy owner once, then publish its singleton address.
inline void EnsureTourMovieController(void* registration) {
    if (((int&)Sing_805DC950()) == 0) {
        if (s_movieControllerInitialized == 0) {
            MovieController_Ctor(s_movieController);
            __register_global_object(s_movieController, MovieController_Dtor, registration);
            s_movieControllerInitialized = 1;
        }
        ((int&)Sing_805DC950()) = (int)s_movieController;
    }
}
#pragma pop

// Construct the tour task with its player-record copy and begin at initialization.
CTourMain::CTourMain() {
    __ct__18NativeBase80201D18Fv(this);
    vtbl = __vt__9CTourMain;
    (*(char**)&secondaryVtable) = __vt__9CTourMain + 0x3C;
    SaveData_InitPlayerRecord(playerRecordCopy);
    state = kTourInitialize;
    pendingState = kTourIdle;
    exitDestination = kTourNoExit;
    messageLatch = 0;
    (*(int*)&tourSelect) = 0;
    (*(int*)&tourConfirm) = 0;
    (*(int*)&saveMenu) = 0;
}

// Initialize the item-request singleton if needed and clear its pending entries.
void ItemRequestState_ClearGlobalEntries() {
    EnsureTourItemRequests(s_itemRequestStateDtorRegistration_tourMain);
    ItemRequestState_ClearEntries((void*)((int&)Sing_805DC958()));
}

// Initialize the wind HUD layout, selection, counters, and invalid resource index.
CBallWind2d::CBallWind2d() {
    Layout_Init(this, 5);
    float f0 = kFloat_0_BallWindHud;
    int zero = 0;
    int initialSelectableElement = kFirstWindElement;
    int one = 1;
    int neg = -1;
    (*(char**)&interfaceVtable) = __vt__11CBallWind2d;
    selectedElementIndex = initialSelectableElement;
    selectionActive = zero;
    selectionState = zero;
    ballControlsVisible = zero;
    element0Visible = zero;
    elements26And27Visible = zero;
    element22Visible = zero;
    element25Visible = zero;
    elements25And26Bright = zero;
    element28Visible = zero;
    element29Visible = zero;
    element23Visible = zero;
    element24Visible = zero;
    element3DrawnSeparately = one;
    indicatorCenterX = f0;
    indicatorColor = neg;
    layoutReady = zero;
}

// Initialize the tour task unit's full-turn constant.
void TourTask_InitFullTurn() {
    s_tourTaskFullTurn = kFloat_6_2831855;
}

// This tour task has no work during the pre-load hook.
void CTourMain::OnPreLoadInit() {}

// This tour task has no work during the load hook.
void CTourMain::OnLoad() {}

// This tour task has no work during the init hook.
void CTourMain::OnInit() {}

// Resource cleanup is handled by the destructor rather than this lifecycle hook.
void CTourMain::OnDestroy() {}

extern char s_tourMainTypeInfo;

// Return the tour task's runtime type descriptor.
void* CTourMain::GetRTTI() {
    return &s_tourMainTypeInfo;
}

void* __dt__9CTourMainFv(void*, int);

// The secondary interface's destructor entry: its this pointer is 0x18 bytes
// past the object's start, so step back before destroying.
void CTourMain::DtorThunk18(int deleting) {
    __dt__9CTourMainFv((char*)this - 0x18, deleting);
}

// Initialize the wind HUD unit's full-turn constant.
void BallWindHud_InitFullTurn() {
    s_ballWindHudFullTurn = kFloat_6_2831855;
}

// Record message value 0/1 as the task's message latch; ignore other values.
// state is a borrowed message packet; the value is word 1, not word 0.
void CTourMain::HandleMsg(int* state) {
    int v = state[1];
    if (v != 1) {
        if (v >= 1)
            return;
        if (v < 0)
            return;
        messageLatch = 0;
        return;
    }
    messageLatch = 1;
}

void Layout_Destroy(void*, int);
void __dl__FPv(void*);

// Destroy the wind layout and optionally release its object storage.
CBallWind2d::~CBallWind2d() {
    Layout_Destroy(this, 0);
}

void Layout_SetElementEnabled(void*, int, int);
void Layout_SetElementFeedbackState(void*, int, int);

// Hide selectable layout elements (artwork identity unresolved), clear feedback, and mark the HUD initialized.
int CBallWind2d::InitLayout() {
    for (int i = kFirstWindElement; i <= kLastWindElement; ++i)
        Layout_SetElementEnabled(this, i, 0);
    Layout_SetElementFeedbackState(this, kWindFeedbackElement, 0);
    layoutReady = 1;
    return 1;
}

void Layout_LoadExtended(void*, void*, const char*);
extern char golfBallWindLayoutStrings[];

// Load the wind HUD's named layout resource.
int CBallWind2d::LoadLayout(void* arg0, void* arg1) {
    Layout_LoadExtended(arg0, arg1, golfBallWindLayoutStrings);
    return 1;
}

// Destroy both tour menus, release the sound bank, and optionally delete the tour task.
CTourMain::~CTourMain() {
    void* p = tourConfirm;
    vtbl = __vt__9CTourMain;
    (*(char**)&secondaryVtable) = __vt__9CTourMain + 0x3C;
    if (p != 0) {
        TourMap_DestroyScreenAndPreviewTree(p, 1);
        tourConfirm = 0;
    }
    void* q = tourSelect;
    if (q != 0) {
        TourAreaSelect_Destroy(q, 1);
        tourSelect = 0;
    }
    ((TourProgressContext*)s_tourProgressStateStorage)->playerRecord = 0;
    SoundManager_ReleaseSoundBank(g_soundManager, tourReturnTaskStrings);
    if (this != 0) {
        __dt__18NativeBase80201D18Fv(this, 0);
    }
}

#pragma push
#pragma opt_strength_reduction off
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Mark both ends of a map edge: codes below 100 connect rows; other codes connect adjacent columns.
// self is TourModeSave, not a menu object. Requires a valid areaOrder entry and
// edge node: codes 0..99 encode node->node+5; 100..199 encode node->node+1.
// Last-row/last-column edges are ignored; other out-of-range codes are unchecked.
// Decode signed /100 and %100 using the original multiply-high/truncate-toward-zero sequence.
static inline void DecodeTourEdgeCode128(int edgeCode, int& edgeAxis, int& edgeNode, int& axisMultiple) {
    edgeAxis = __mulhw(0x51EB851F, edgeCode);
    edgeNode = edgeAxis >> 5;
    axisMultiple = (unsigned int)edgeNode >> 31;
    edgeAxis = edgeAxis >> 5;
    axisMultiple = edgeNode + axisMultiple;
    edgeNode = (unsigned int)edgeAxis >> 31;
    axisMultiple = axisMultiple * kHorizontalEdgeCodeBase;
    edgeAxis = edgeAxis + edgeNode;
    edgeNode = edgeCode - axisMultiple;
}

// Set reciprocal direction flags on the two grid nodes encoded by edgeCode.
void TourMap_MarkConnectingEdge(void* self, int edgeCode) {
    if (self == 0)
        return;
    int edgeAxis;
    int edgeNode;
    int axisMultiple;
    char* areaOrderCursor;
    signed char areaOrderIndex;
    signed char areaIndex;
    areaOrderIndex = ((TourModeSave*)self)->currentArea;
    if (areaOrderIndex < 0)
        return;
    areaOrderCursor = (char*)self + areaOrderIndex;
    areaIndex = (signed char)areaOrderCursor[TOUR_OFFSET(TourModeSave, areaOrder)];
    // edgeAxis = edgeCode / 100, edgeNode = edgeCode % 100 (signed,
    // truncating toward zero). Keep this expanded multiply-high sequence:
    // strength reduction is disabled for this already matched function.
    DecodeTourEdgeCode128(edgeCode, edgeAxis, edgeNode, axisMultiple);
    if (edgeAxis != 0) {
        // column = edgeNode % 5. Reject column 4; connect node -> node+1
        // with reciprocal right/left bits in the selected physical area.
        int nodeQuotient = __mulhw(0x66666667, edgeNode);
        nodeQuotient = nodeQuotient >> 1;
        int negativeCorrection = (nodeQuotient < 0) ? 1 : 0;
        nodeQuotient = nodeQuotient + negativeCorrection;
        nodeQuotient = nodeQuotient * kTourMapColumns;
        int column = edgeNode - nodeQuotient;
        if (column >= kTourMapColumns - 1)
            return;
        char* nodeFlags = (char*)self + areaIndex * sizeof(TourAreaSave) + TOUR_OFFSET(TourAreaSave, nodes);
        unsigned int firstNodeByteOffset = (unsigned int)edgeNode << 1;
        unsigned int secondNodeByteOffset = ((unsigned int)edgeNode + 1) << 1;
        unsigned short firstNodeFlags = *(unsigned short*)(nodeFlags + firstNodeByteOffset);
        firstNodeFlags |= kEdgeRight;
        *(unsigned short*)(nodeFlags + firstNodeByteOffset) = firstNodeFlags;
        unsigned short secondNodeFlags = *(unsigned short*)(nodeFlags + secondNodeByteOffset);
        secondNodeFlags |= kEdgeLeft;
        *(unsigned short*)(nodeFlags + secondNodeByteOffset) = secondNodeFlags;
        return;
    } else {
        // row = edgeNode / 5. Reject row 6; connect node -> node+5
        // with reciprocal down/up bits. Node flags are 16-bit grid cells.
        int nodeQuotient = __mulhw(0x66666667, edgeNode);
        nodeQuotient = nodeQuotient >> 1;
        int negativeCorrection = (nodeQuotient < 0) ? 1 : 0;
        nodeQuotient = nodeQuotient + negativeCorrection;
        if (nodeQuotient >= kTourMapRows - 1)
            return;
        char* nodeFlags = (char*)self + areaIndex * sizeof(TourAreaSave) + TOUR_OFFSET(TourAreaSave, nodes);
        unsigned int firstNodeByteOffset = (unsigned int)edgeNode << 1;
        unsigned int secondNodeByteOffset = ((unsigned int)edgeNode + kTourMapColumns) << 1;
        unsigned short firstNodeFlags = *(unsigned short*)(nodeFlags + firstNodeByteOffset);
        firstNodeFlags |= kEdgeDown;
        *(unsigned short*)(nodeFlags + firstNodeByteOffset) = firstNodeFlags;
        unsigned short secondNodeFlags = *(unsigned short*)(nodeFlags + secondNodeByteOffset);
        secondNodeFlags |= kEdgeUp;
        *(unsigned short*)(nodeFlags + secondNodeByteOffset) = secondNodeFlags;
        return;
    }
}

#pragma pop
// Draw the menu, ending movie, or save overlay selected by the current tour phase.
void CTourMain::OnDisplay() {
    unsigned int v = (*(unsigned int*)&state);
    switch (v) {
    case kTourSelectArea: {
        void* p = tourSelect;
        if (p != 0) {
            TourMenuGroup_DrawActiveEntries(p);
            EnsureTourItemRequests(s_itemRequestStateDtorRegistration_tourMain);
            TourItemRequest_DispatchDisplay((void*)((int&)Sing_805DC958()));
        }
        break;
    }
    case kTourItemMenu: {
        void* p = tourConfirm;
        if (p != 0) {
            ((CTourMain*)p)->DrawItemMenu();
            EnsureTourItemRequests(s_itemRequestStateDtorRegistration_tourMain);
            TourItemRequest_DispatchDisplay((void*)((int&)Sing_805DC958()));
        }
        break;
    }
    case kTourPlayEndingMovie:
    case kTourCloseEndingMovie: {
        {
            EnsureTourMovieController(s_tourTaskMovieDtorRegistration);
            MoviePlayer_DrawCurrentSamples((void*)((int&)Sing_805DC950()));
        }
        break;
    }
    case kTourSave: {
        void* p = saveMenu;
        if (p != 0)
            MPBase1_DrawIfInitialized(p);
        break;
    }
    default:
        break;
    }
}
#pragma push
#pragma use_lmw_stmw on
// Table copies retain their original halfword widths; no native struct assignment may change that copy schedule.
// Commit the current node's completion/rewards and unlock progression in saved tour modes.
// Requires a valid current area/node and live shared player state. Works on a
// local save snapshot, then publishes it once; reward singleton calls have side effects.
void TourMap_CommitNodeCompletionAndRewards() {
    // Phase 1: snapshot the saved tour modes before applying any rewards.
    struct TourCourseRewards128 {
        unsigned short perProgressionSlot[8][kTourCoursesPerAreaReward];
    } courseRewardTable;
    struct TourOtherRewards128 {
        unsigned short perProgressionSlot[8][kTourOtherRewardsPerArea];
    } otherRewardTable;
    typedef TourAreaSave Record;
    TourModeSave data;
    char* nodeCursor;
    int loop;
    char* courseRewardCursor;
    int areaByteOffset;
    int nodeIndex;
    // Copy all 0x33C tour bytes before applying edge, area and node changes.
    if (((volatile unsigned int&)GetSaveDataSlot()) == 0) {
        if (s_saveDataInitialized == 0) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((volatile unsigned int&)GetSaveDataSlot()) = (unsigned int)s_saveData;
    }
    void* p0 = SaveData_GetCurrentRecord((void*)((volatile unsigned int&)GetSaveDataSlot()));
    struct TourModeHalfwordSnapshot128 {
        unsigned short words[kTourModeSaveBytes / sizeof(unsigned short)];
    };
    *(TourModeHalfwordSnapshot128*)&data = *(TourModeHalfwordSnapshot128*)&((TourPlayerRecordView*)p0)->modes;
    // Phase 2: resolve progression to the physical area/node, then apply edge progress.
    signed char progressionIndex = data.currentArea;
    signed char physicalAreaIndex = data.areaOrder[progressionIndex];
    signed char areaOrEdgeIndex = physicalAreaIndex;
    areaByteOffset = areaOrEdgeIndex * sizeof(TourAreaSave);
    nodeIndex = *(short*)((char*)&data + 2 + areaByteOffset);
    char* progressStorage = s_tourProgressStateStorage;
    if (((TourProgressContext*)progressStorage)->edgeRewardState == 1) {
        void* mgr = (void*)g_sharedDoc;
        if (((CSharedDoc*)mgr)->golfRuleType == 1) {
            if (EventScript_Player0UnbeatenWinsAtLeastOneThird(0, 0) != 0) {
                ((TourProgressContext*)progressStorage)->edgeRewardState = 2;
                int areaOrEdgeIndex = ((TourProgressContext*)progressStorage)->edgeIndex;
                TourMap_MarkConnectingEdge(&data, areaOrEdgeIndex);
            }
        }
    }
    // Area completion unlocks the next progression slot (using c+1 directly),
    // or requests the ending movie, then queues its two course/seven other rewards.
    // Phase 3: unlock progression and queue the area course/notification rewards.
    if (((TourProgressContext*)s_tourProgressStateStorage)->areaCleared != 0) {
        signed char c = data.currentArea;
        if (c >= 7) {
            ((TourProgressContext*)s_tourProgressStateStorage)->endingMoviePending = 1;
            data.flags |= kTourAllAreasCleared;
        } else {
            data.areas[c + 1].flags |= kTourAreaUnlocked;
        }
        courseRewardTable = *(TourCourseRewards128*)s_tourAreaCourseRewards;
        courseRewardCursor = (char*)&courseRewardTable + ((unsigned char)data.currentArea << 2);
        int courseRewardsQueued = 0;
        for (loop = 0; loop < kTourCoursesPerAreaReward; loop++) {
            // Lazily publish the item-request singleton before consuming its entries.
            EnsureTourItemRequests(s_itemRequestStateDtorRegistration_tourMain);
            void* mgr2 = (void*)((int&)Sing_805DC958());
            short val = *(short*)courseRewardCursor;
            ItemRequestState_RequestCourseByInventoryIndex(mgr2, 0, val);
            courseRewardsQueued++;
            courseRewardCursor += 2;
        }
        otherRewardTable = *(TourOtherRewards128*)s_tourAreaOtherRewardIndices;

        int otherRewardIndex = 0;
        char* otherRewardCursor = (char*)&otherRewardTable +
                                  (unsigned char)data.currentArea * (kTourOtherRewardsPerArea * sizeof(unsigned short));
        for (; otherRewardIndex < kTourOtherRewardsPerArea; otherRewardIndex++, otherRewardCursor += 2) {
            if (*(short*)otherRewardCursor == -1)
                continue;
            EnsureTourItemRequests(s_itemRequestStateDtorRegistration_tourMain);
            void* mgr3 = (void*)((int&)Sing_805DC958());
            unsigned int low = (unsigned char)*(short*)otherRewardCursor;
            SaveRewards_SetOtherNotificationBit(mgr3, (int)low);
        }

        int nodeByteOffset = nodeIndex << 1;
        char* areaNodeCursor = (char*)((unsigned short*)((char*)&data + areaByteOffset) + nodeIndex);
        unsigned short areaNodeFlags = *(unsigned short*)(areaNodeCursor + 4);
        if (((areaNodeFlags >> 4) & 1) == 0) {
            data.currentArea = -1;
        }
    }
    // Numeric rewards are counted once per node, independently of area completion.
    int nodeByteOffset = nodeIndex << 1;
    nodeCursor = (char*)((unsigned short*)((char*)&data + areaByteOffset) + nodeIndex);
    unsigned short nodeFlags = *(unsigned short*)(nodeCursor + 4);
    if (((nodeFlags >> 4) & 1) == 0) {
        void* gm = (void*)g_sharedDoc;
        void* q = GolfPlayer_GetEntryByIndex((*(void**)&((CSharedDoc*)gm)->golfDoc), 0);
        int numericPlayerReward = SharedStringState_GetWordValue();
        ((TourPlayerRewardView*)q)->pendingReward += numericPlayerReward;
        int numericTourReward = SharedStringState_GetShortValue();
        data.rewardCount += numericTourReward;
        *(unsigned short*)(nodeCursor + 4) = (unsigned short)(*(unsigned short*)(nodeCursor + 4) | kNodeRewardCounted);
    }

    // Mark completion before applying queued inventory/text rewards and publishing.
    char* completedNodeCursor = (char*)&data + areaByteOffset + nodeByteOffset;
    *(unsigned short*)(completedNodeCursor + 4) =
        (unsigned short)((*(unsigned short*)(completedNodeCursor + 4) | kNodeCompleted) & ~kNodePending);
    EnsureTourItemRequests(s_itemRequestStateDtorRegistration_tourMain);
    ItemRequestState_ApplyPendingRewards((void*)((int&)Sing_805DC958()));

    EnsureTourItemRequests(s_itemRequestStateDtorRegistration_tourMain);
    ItemRequestState_MarkTextNotifications((void*)((int&)Sing_805DC958()));

    *(unsigned short*)(nodeCursor + 4) = (unsigned short)(*(unsigned short*)(nodeCursor + 4) | kNodeRewardsApplied);

    if (((volatile unsigned int&)GetSaveDataSlot()) == 0) {
        if (s_saveDataInitialized == 0) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((volatile unsigned int&)GetSaveDataSlot()) = (unsigned int)s_saveData;
    }
    // Phase 4: publish the completed local tour-mode snapshot once.
    SaveData_SetModeRecords((void*)((volatile unsigned int&)GetSaveDataSlot()), &data);
}

#pragma pop
extern char s_tourTaskRegistrationAndProgressStorage[];
extern void* g_taskManager;
extern signed char s_videoDimmingStateInitialized;
extern unsigned char s_screenDimmingState;
extern CSharedDoc* g_sharedDoc;
void SoundManager_AcquireSoundBank(void*, void*);
void* memset(void*, int, unsigned);

void* __nw__FUl(unsigned long);
void TourAreaSelect_Ctor(void*, void*);
void ModeRecordUiContext_Initialize(void*, void*);
void SoundManager_RequestBgmStop(void*, int);
void MovieController_Ctor(void*);
void MovieController_LoadNamedMovie(void*, void*, int, int);
int MovieController_PollReadyLatch(void*);
void MovieController_StartPlayback(void*, int);
void MovieController_ReleaseAudioAndResetLatches(void*);
int Movie_GetPlaybackState(void*);
void MovieController_InvokeNoOpHook(void*);
void FadeOverlay_Init(void*);
extern char ResourceState_Dtor[];
void FadeState_StartFadeOut(void*, int);
void FadeState_RequestFramebufferCapture(void*);
void FadeState_StartFadeIn(void*, int);
void VIEnableDimming(int);
int MPBase1_Process(void*, float);
void TaskCommandQueue_AppendNamedRequest(void*, int, void*, int, int, int, int, int);
void SaveData_Ctor(void*);
void* SaveData_GetCurrentRecord(void*);
void SaveData_SetCurrentRecord(void*, void*);
void InputManager_Init(void*);
extern char InputManager_Dtor[];
int Input_IsActionPressed(void*, int, int);
extern signed char s_fadeOverlayInitialized;
extern signed char s_inputManagerInitialized;
extern signed char s_movieControllerInitialized;
extern volatile signed char s_saveDataInitialized;
extern void* g_taskManager;
extern signed char s_videoDimmingStateInitialized;
extern unsigned char s_screenDimmingState;
extern CSharedDoc* g_sharedDoc;
extern signed char s_itemRequestStateInitialized;
extern char s_itemRequestState[];
extern char s_movieController[];
extern char s_gameInputManager[];
extern unsigned char s_saveData[];
extern char tourReturnTaskStrings[];
extern char s_fadeOverlay[];
extern char s_tourTaskRegistrationAndProgressStorage[];
extern char s_itemRequestStateDtorRegistration_tourMain[];
}
class TourSelect {
    char data[0x38];

public:
    // Construct an area menu borrowing the task's shared progress context.
    TourSelect(void* state) {
        TourAreaSelect_Ctor(this, state);
    }
};
class TourConfirm {
    char data[0x9C];

public:
    // Construct the map/item menu borrowing the same progress context.
    TourConfirm(void* state) {
        ModeRecordUiContext_Initialize(this, state);
    }
};
class TourTransition {
public:
    virtual ~TourTransition();
};
extern "C" {

// Transition policy (next update unless marked immediate):
// initialize -> choose; choose -> ending/area/item; area accept -> item;
// area cancel -> save; item back -> area, rebuild -> item, start -> save;
// ending load -> wait [immediate] -> play -> close -> area/item;
// save-start -> save [immediate] -> requested task -> finished.
// Advance tour setup, area/item selection, ending playback, saving, and the requested task transition.
// deltaSeconds is forwarded unchanged to active menus. pendingState=0 means
// no deferred transition; nonzero states commit at the next update's entry.
// Movie-load and save-start cases instead assign state and fall through in this call.
void CTourMain::OnProcess(float deltaSeconds) {
    float frameDelta = deltaSeconds;
    void* tourTask = this;
    char* registrationStorage = s_tourTaskRegistrationAndProgressStorage;
    void* contextOrCompletion = 0;

    // Commit the previous update's deferred transition before dispatching this phase.
    int pendingPhaseWord = AsCTourMain(tourTask)->pendingState;
    if (pendingPhaseWord != 0) {
        AsCTourMain(tourTask)->state = pendingPhaseWord;
        AsCTourMain(tourTask)->pendingState = 0;
    }
    unsigned int phase = (*(unsigned int*)&AsCTourMain(tourTask)->state);
    switch (phase) {
    case kTourIdle:
    case kTourFinished:
        break;
    case kTourInitialize: {
        // Snapshot the current player record; progress borrows this task-local copy.
        contextOrCompletion = (registrationStorage + 0x30);
        if (((TourProgressContext*)contextOrCompletion)->initialized == 0) {
            memset(contextOrCompletion, 0, 0x38);
            ((TourProgressContext*)contextOrCompletion)->initialized = 1;
        }
        if (((volatile unsigned int&)GetSaveDataSlot()) == 0) {
            if (s_saveDataInitialized == 0) {
                SaveData_Ctor(s_saveData);
                s_saveDataInitialized = 1;
            }
            ((volatile unsigned int&)GetSaveDataSlot()) = (unsigned int)s_saveData;
        }
        void* savedPlayerRecord = SaveData_GetCurrentRecord((void*)((volatile unsigned int&)GetSaveDataSlot()));
        // Copy-only 0x828-byte player-record schema: groups encode transfer
        // widths, not independent save objects. +4EC begins TourModeSave.
        struct PlayerRecordTransfer {
            unsigned int words000[151];
            unsigned int words25C[28];
            unsigned short halfwords2CC[208];
            unsigned int words46C[16];
            unsigned char bytes4AC[24];
            unsigned char byte4C4, byte4C5, byte4C6, byte4C7, byte4C8, byte4C9;
            unsigned char bytes4CA[8];
            unsigned char byte4D2, byte4D3, byte4D4;
            unsigned int words4D8[5];
            unsigned short tourHalfwords4EC[414];
        };
        struct TourData {
            char pad[0x34];
            PlayerRecordTransfer playerRecord;
        };
        ((TourData*)tourTask)->playerRecord = *(PlayerRecordTransfer*)savedPlayerRecord;
        *(void**)((registrationStorage + 0x30)) = (void*)((char*)tourTask + TOUR_OFFSET(CTourMain, playerRecordCopy));
        SoundManager_AcquireSoundBank(g_soundManager, tourReturnTaskStrings);
        AsCTourMain(tourTask)->pendingState = kTourChooseMenu;
        break;
    }
    case kTourChooseMenu: {
        // A pending ending takes precedence over an existing area selection.
        char* progressBytes = (registrationStorage + 0x30);
        if (((TourProgressContext*)progressBytes)->endingMoviePending != 0) {
            ((TourProgressContext*)progressBytes)->endingMoviePending = 0;
            AsCTourMain(tourTask)->pendingState = kTourLoadEndingMovie;
        } else {
            void* areaMenuOrPlayer = *(void**)((registrationStorage + 0x30));
            signed char currentArea = ((TourPlayerRecordView*)areaMenuOrPlayer)->modes.currentArea;
            if (currentArea < 0)
                AsCTourMain(tourTask)->pendingState = kTourCreateAreaSelection;
            else
                AsCTourMain(tourTask)->pendingState = kTourCreateItemMenu;
        }
        break;
    }
    case kTourCreateAreaSelection: {
        void* selectionContext = (registrationStorage + 0x30);
        ((TourProgressContext*)selectionContext)->selectionResult = 0;
        void* newMenu = new TourSelect(selectionContext);
        AsCTourMain(tourTask)->tourSelect = newMenu;
        AsCTourMain(tourTask)->pendingState = kTourSelectArea;
        break;
    }
    case kTourSelectArea: {
        // Process queued reward UI first, then accept/cancel the area menu result.
        EnsureTourItemRequests((registrationStorage + 0xc));
        ItemRequestState_UpdateCompletion((void*)((int&)Sing_805DC958()));
        float menuDelta = frameDelta;
        void* activeMenu = AsCTourMain(tourTask)->tourSelect;
        int menuResult = ((MenuGroup802FB4D0*)activeMenu)->ProcessAreaSelection(menuDelta);
        if (menuResult > 0) {
            void* areaMenuOrPlayer = AsCTourMain(tourTask)->tourSelect;
            void* playerRecord = *(void**)((registrationStorage + 0x30));
            int selectionOrDestination = *(int*)areaMenuOrPlayer;
            ((TourPlayerRecordView*)playerRecord)->modes.currentArea = (signed char)selectionOrDestination;
            void* areaMenuToDestroy = AsCTourMain(tourTask)->tourSelect;
            if (areaMenuToDestroy != 0) {
                TourAreaSelect_Destroy(areaMenuToDestroy, 1);
                AsCTourMain(tourTask)->tourSelect = 0;
            }
            AsCTourMain(tourTask)->pendingState = kTourCreateItemMenu;
        } else if (menuResult < 0) {
            AsCTourMain(tourTask)->exitDestination = kTourReturnToMenu;
            AsCTourMain(tourTask)->pendingState = kTourStartSave;
        }
        break;
    }
    case kTourCreateItemMenu: {
        void* newMenu = new TourConfirm((registrationStorage + 0x30));
        contextOrCompletion = newMenu;
        AsCTourMain(tourTask)->tourConfirm = contextOrCompletion;
        AsCTourMain(tourTask)->pendingState = kTourItemMenu;
        break;
    }
    case kTourItemMenu: {
        // The map menu can rebuild itself, start a saved game, or return to areas.
        EnsureTourItemRequests((registrationStorage + 0xc));
        ItemRequestState_UpdateCompletion((void*)((int&)Sing_805DC958()));
        int menuResult = ((CTourMain*)(AsCTourMain(tourTask)->tourConfirm))->ProcessItemMenu(frameDelta);
        switch (menuResult) {
        case kTourMenuRebuild: {
            void* activeMenu = AsCTourMain(tourTask)->tourConfirm;
            if (activeMenu != 0) {
                TourMap_DestroyScreenAndPreviewTree(activeMenu, 1);
                AsCTourMain(tourTask)->tourConfirm = 0;
            }
            AsCTourMain(tourTask)->pendingState = kTourCreateItemMenu;
            break;
        }
        case kTourMenuStartGame: {
            void* activeMenu = AsCTourMain(tourTask)->tourConfirm;
            if (activeMenu != 0) {
                TourMap_DestroyScreenAndPreviewTree(activeMenu, 1);
                AsCTourMain(tourTask)->tourConfirm = 0;
            }
            AsCTourMain(tourTask)->exitDestination = kTourStartGame;
            AsCTourMain(tourTask)->pendingState = kTourStartSave;
            SoundManager_RequestBgmStop(g_soundManager, 1);
            break;
        }
        case kTourMenuBack: {
            void* activeMenu = AsCTourMain(tourTask)->tourConfirm;
            if (activeMenu != 0) {
                TourMap_DestroyScreenAndPreviewTree(activeMenu, 1);
                AsCTourMain(tourTask)->tourConfirm = 0;
            }
            AsCTourMain(tourTask)->pendingState = kTourCreateAreaSelection;
            break;
        }
        case kTourMenuPending:
            break;
        }
        break;
    }
    case kTourLoadEndingMovie: {
        EnsureTourMovieController(((void*)(registrationStorage + 0x24)));
        MovieController_LoadNamedMovie((void*)((int&)Sing_805DC950()),
                                       (void*)(tourReturnTaskStrings + kEndingMovieNameOffset), kTourMovieBufferBytes,
                                       kTourMovieLoadMode);
        if (((int&)Sing_805DC608()) == 0) {
            if (s_fadeOverlayInitialized == 0) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, ResourceState_Dtor, ((void*)(registrationStorage + 0x18)));
                s_fadeOverlayInitialized = 1;
            }
            ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
        }
        FadeState_StartFadeOut((void*)((int&)Sing_805DC608()), kTourMovieFadeDuration);
        AsCTourMain(tourTask)->state = kTourWaitForEndingMovie;
    }
    // Intentional fallthrough: poll readiness immediately after requesting the movie.
    case kTourWaitForEndingMovie: {
        EnsureTourMovieController(((void*)(registrationStorage + 0x24)));
        if (MovieController_PollReadyLatch((void*)((int&)Sing_805DC950())) != 0) {
            if (Sing_805DC960() == 0) {
                if (s_videoDimmingStateInitialized == 0) {
                    contextOrCompletion = (void*)1;
                    s_screenDimmingState = 1;
                    VIEnableDimming(1);
                    s_videoDimmingStateInitialized = 1;
                }
                Sing_805DC960() = &s_screenDimmingState;
            }
            void* dimmingFlag = Sing_805DC960();
            if (*(unsigned char*)dimmingFlag != 0) {
                *(unsigned char*)dimmingFlag = 0;
                VIEnableDimming(0);
            }
            EnsureTourMovieController(((void*)(registrationStorage + 0x24)));
            MovieController_StartPlayback((void*)((int&)Sing_805DC950()), 0);
            AsCTourMain(tourTask)->pendingState = kTourPlayEndingMovie;
        }
        break;
    }
    case kTourPlayEndingMovie: {
        EnsureTourMovieController(((void*)(registrationStorage + 0x24)));
        MovieController_InvokeNoOpHook((void*)((int&)Sing_805DC950()));
        if (((int&)GetInputManagerSlot()) == 0) {
            if (s_inputManagerInitialized == 0) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, InputManager_Dtor, (void*)registrationStorage);
                s_inputManagerInitialized = 1;
            }
            ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
        }
        // Either input action (0x11/0x12) or playback state 2 closes the ending movie.
        if (Input_IsActionPressed((void*)((int&)GetInputManagerSlot()), kTourMovieExitActionA, -1) == 0) {
            if (((int&)GetInputManagerSlot()) == 0) {
                if (s_inputManagerInitialized == 0) {
                    InputManager_Init(s_gameInputManager);
                    __register_global_object(s_gameInputManager, InputManager_Dtor, (void*)(registrationStorage + 0));
                    s_inputManagerInitialized = 1;
                }
                ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
            }
            if (Input_IsActionPressed((void*)((int&)GetInputManagerSlot()), kTourMovieExitActionB, -1) == 0) {
                EnsureTourMovieController(((void*)(registrationStorage + 0x24)));
                if (Movie_GetPlaybackState((void*)((int&)Sing_805DC950())) != kTourMovieClosePlaybackState)
                    break;
            }
        }
        {
            if (((int&)Sing_805DC608()) == 0) {
                if (s_fadeOverlayInitialized == 0) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, ResourceState_Dtor, ((void*)(registrationStorage + 0x18)));
                    s_fadeOverlayInitialized = 1;
                }
                ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
            }
            FadeState_RequestFramebufferCapture((void*)((int&)Sing_805DC608()));
            if (((int&)Sing_805DC608()) == 0) {
                if (s_fadeOverlayInitialized == 0) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, ResourceState_Dtor, ((void*)(registrationStorage + 0x18)));
                    s_fadeOverlayInitialized = 1;
                }
                ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
            }
            FadeState_StartFadeIn((void*)((int&)Sing_805DC608()), 0);
            AsCTourMain(tourTask)->pendingState = kTourCloseEndingMovie;
        }
        break;
    }
    case kTourCloseEndingMovie: {
        // Release movie audio, restore dimming, then resume the appropriate menu.
        {
            EnsureTourMovieController(((void*)(registrationStorage + 0x24)));
            MovieController_ReleaseAudioAndResetLatches((void*)((int&)Sing_805DC950()));
        }
        if (Sing_805DC960() == 0) {
            if (s_videoDimmingStateInitialized == 0) {
                contextOrCompletion = (void*)1;
                s_screenDimmingState = 1;
                VIEnableDimming(1);
                s_videoDimmingStateInitialized = 1;
            }
            Sing_805DC960() = &s_screenDimmingState;
        }
        void* dimmingFlag = Sing_805DC960();
        if (*(unsigned char*)dimmingFlag != 1) {
            *(unsigned char*)dimmingFlag = 1;
            VIEnableDimming(1);
        }
        void* playerRecord = *(void**)((registrationStorage + 0x30));
        signed char currentArea = ((TourPlayerRecordView*)playerRecord)->modes.currentArea;
        if (currentArea < 0)
            AsCTourMain(tourTask)->pendingState = kTourCreateAreaSelection;
        else
            AsCTourMain(tourTask)->pendingState = kTourCreateItemMenu;
        break;
    }
    case kTourStartSave: {
        if (((volatile unsigned int&)GetSaveDataSlot()) == 0) {
            if (s_saveDataInitialized == 0) {
                SaveData_Ctor(s_saveData);
                s_saveDataInitialized = 1;
            }
            ((volatile unsigned int&)GetSaveDataSlot()) = (unsigned int)s_saveData;
        }
        SaveData_SetCurrentRecord((void*)((volatile unsigned int&)GetSaveDataSlot()),
                                  (void*)((char*)tourTask + TOUR_OFFSET(CTourMain, playerRecordCopy)));
        if ((*(int*)&AsCTourMain(tourTask)->saveMenu) == 0) {
            void* saveMenu = CFlashMemoryMenuSave::Create();
            AsCTourMain(tourTask)->saveMenu = saveMenu;
            SoundManager_RequestBgmStop(g_soundManager, 1);
        }
        AsCTourMain(tourTask)->state = kTourSave;
    }
    // Intentional fallthrough: a newly created save menu receives its first update now.
    case kTourSave: {
        void* saveMenu = AsCTourMain(tourTask)->saveMenu;
        contextOrCompletion = 0;
        if (saveMenu == 0) {
            contextOrCompletion = 0;
        } else {
            float menuDelta = frameDelta;
            if (MPBase1_Process(saveMenu, menuDelta) != 0) {
                void* saveMenuToDelete = AsCTourMain(tourTask)->saveMenu;
                if (saveMenuToDelete != 0) {
                    delete (TourTransition*)saveMenuToDelete;
                    AsCTourMain(tourTask)->saveMenu = 0;
                }
                contextOrCompletion = (void*)1;
            }
        }
        // Only a completed live save menu permits the selected task transition.
        // A null save-menu pointer leaves this phase pending.
        if (contextOrCompletion != 0) {
            int selectionOrDestination = AsCTourMain(tourTask)->exitDestination;
            switch (selectionOrDestination) {
            case kTourNoExit:
                break;
            case kTourReturnToMenu: {
                char* returnTaskNames = tourReturnTaskStrings;
                void* titleTaskManager = g_taskManager;
                int titleTransitionFlags = 0;
                void* titleTaskName = returnTaskNames + kTitleTaskNameOffset;
                void* emptyTransitionName = returnTaskNames + kEmptyTransitionNameOffset;
                ((CTask*)titleTaskManager)
                    ->RequestTransitionWithFade((const char*)titleTaskName, (const char*)emptyTransitionName,
                                                titleTransitionFlags);
                void* titleCommandManager = g_taskManager;
                void* titleMainName = returnTaskNames + kTitleMainNameOffset;
                int titleRequestSlot = 0;
                int titleRequestKind = kTourTaskRequestKind;
                int titleRequestArg0 = 0, titleRequestArg1 = 0, titleRequestArg2 = 0, titleRequestArg3 = 0;
                TaskCommandQueue_AppendNamedRequest(titleCommandManager, titleRequestSlot, titleMainName,
                                                    titleRequestKind, titleRequestArg0, titleRequestArg1,
                                                    titleRequestArg2, titleRequestArg3);
                break;
            }
            case kTourStartGame: {
                char* gameTaskNames = tourReturnTaskStrings;
                void* gameTaskManager = g_taskManager;
                int gameTransitionFlags = 0;
                void* golfTaskName = gameTaskNames + kGolfTaskNameOffset;
                void* golfDocumentName = gameTaskNames + kGolfDocumentNameOffset;
                ((CTask*)gameTaskManager)
                    ->RequestTransitionWithFade((const char*)golfTaskName, (const char*)golfDocumentName,
                                                gameTransitionFlags);
                break;
            }
            }
            AsCTourMain(tourTask)->pendingState = kTourFinished;
        }
        break;
    }
    default:
        break;
    }
    return;
}
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804625B4", _ctors_u8030C0E8_0, TourTask_InitFullTurn);
STATIC_INIT_ENTRY(".CRT$XCU804625B4", _ctors_u8030C0E8_1, BallWindHud_InitFullTurn);
#pragma pop
