#include <decomp/static_init.h>
// Local ABI views shared by the motion-swing and computer-shot callbacks.
// Task interface tables are byte-indexed; these constants name their ABI starts.
enum FightTaskVtableOffsets { kFightTaskSecondaryVtable = 0x10, kFightTaskTertiaryVtable = 0x1C };
// Selected club +0xA identifies wood/iron/wedge/putter group, independent of input mode.
struct SwingModeView { char reserved[0xA]; unsigned char clubGroup; };
struct SwingInstanceView {
    void* primary;
    SwingModeView* selectedClub;
    char parameterStorage[0x3C];
    int powerShotMode;
};
struct SwingCancelView { char reserved[0x60]; int cancelRequested; };
struct SwingAngleLimitsView { char reserved[0x20]; float maximumAngle; };
struct ComSpinStatView { char reserved[0xE4]; int spin; };
struct ComShotCountView { char reserved[0x28]; unsigned char shotCount; };
struct ComConditionArgumentView { void* callback; int command; int threshold; };
struct ComBallPositionView { char reserved[0x1C]; float position[3]; };
struct ComHoleCupView { char reserved[0xC]; float cupPosition[3]; };
// The per-hole byte chooses the level-record offset added to each shared player block.
struct ComLevelLookupView { char prefix[0x83D]; unsigned char recordOffsets[18]; };
enum { kSharedPlayerBlockBytes = 0x1D4 };
struct ComLastOBView { char reserved[0xBC]; unsigned char lastOB; };
struct ComCachedChecksView { char prefix[0xF8]; int boost; int reserved; int carry; int areaStop; };
enum ComConditionResult { kComConditionPass = 1, kComConditionFail = 2, kComConditionPending = 3 };
enum ComShotFlags {
    kComCurveLeft = 4, kComCurveRight = 8, kComTomahawk = 0x10,
    kComCobra = 0x20, kComSpike = 0x40
};
// InstanceTable +4 selects a registered club; its +A byte is wood/iron/wedge/putter.
// Verified in CClub_Init/RegisterClubParameters (club_selection_and_shot_params.cpp).
enum SwingClubGroup { kWoodClubGroup = 0, kIronClubGroup = 1, kWedgeClubGroup = 2, kPutterClubGroup = 3 };
// Callback results below describe reported phase outcomes. The two gate words
// at +48/+4C are separate, unresolved storage; do not infer their writer here.
enum SwingPhaseResult { kSwingIdle = 0, kSwingReady = 1, kSwingPrimaryActive = 2,
    kSwingAlternateActive = 3, kSwingPrimaryComplete = 4, kSwingCancelled = 5,
    kSwingAlternateComplete = 6, kSwingFinishing = 7, kSwingFinished = 8 };
// Callback                 trigger                        reported result
// UpdateIdlePhase          cancel / otherwise             Cancelled / Idle
// vfunc_22                 primary / alternate / neither  PrimaryActive / AlternateActive / Ready
// UpdateForwardPhase       primary latch / otherwise      PrimaryActive / AlternateActive
// UpdateSettlingPhase      completed primary / alternate  PrimaryComplete / AlternateComplete
//                          still updating                 corresponding Active result
// UpdateFinishPhase        finish delay reached / waiting Finished / Finishing
enum SwingConstants { kSwingCancelAction = 6, kSwingPowerBandCount = 7,
    kLastSwingPowerBand = kSwingPowerBandCount - 1,
    kSwingGateBlockedValue = 6, kSwingStateRangeMask = 4, kWordSignShift = 31 };

#include "classes/ThinkRuleNode.h"
#define AsThinkRuleNode(p) ((ThinkRuleNode*)(p))
#include "classes/ThinkRuleTree.h"
#define AsThinkRuleTree(p) ((ThinkRuleTree*)(p))

#include "classes/CEnemyRenderTable.h"
#define AsCEnemyRenderTable(p) ((CEnemyRenderTable*)(p))
#include "classes/CGolfDoc.h"
#include "classes/CSharedDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

// Normal destruction remains explicit. This owner handles exceptional exits.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Arm exceptional-exit cleanup for an already constructed subobject.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Disarm the guard after explicit cleanup or ownership transfer.
    void release() { active = false; }
    // Destroy the guarded subobject without deleting its enclosing storage.
    ~EH5Cleanup() { if (active) destroy(object, -1); }
};

#include "game/vector.h"
#include "game/wvector.h"
extern "C" void __construct_array(void*, void*, void*, unsigned int, unsigned int);
#include "sdk_protos.h"
#include "game/types.h"
#include "game/singletons.h"
#include "classes/CPadSwingCtrl.h"
#include "classes/CComThinkCtrl.h"
#include "classes/CFightMain.h"

#define AsCFightMain(p) ((CFightMain*)(p))

#define AsCComThinkCtrl(p) ((CComThinkCtrl*)(p))

#define AsCPadSwingCtrl(p) ((CPadSwingCtrl*)(p))

// Shared layouts used by the controller, fight tasks, and computer player.
struct SwingGaugeResultView {
    void* manager;
    char reserved04[4];
    float scale;
    char reserved0C[0x3C];
    int index;
    int result;
    int lockA;
    int lockB;
};

struct SwingMgrBase { char pad[0x3924]; };
struct SwingMgr : SwingMgrBase {
    virtual void s0();
    virtual void s1();
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual void s8();
    virtual void s9();
    virtual void s10();
    virtual void s11();
    virtual float width();
    virtual float height();
    virtual void s14();
    virtual void s15();
    virtual void s16();
    virtual void sample(float*, float*, float*, int*);
    virtual void slot18(float*, float*);
};

struct EventTarget {
    virtual void release(int);
};

struct SwingGameBase {
    char pad[0x3924];
};

struct SwingGame : SwingGameBase {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void slot8();
    virtual void slot9();
    virtual void slot10(int);
    virtual int slot11(int, float);
    virtual void slot12();
    virtual void slot13();
    virtual float time();
    virtual void slot15();
    virtual void sample(int*, float*, float);
    virtual void slot17(float*, float*, float*, int*);
    virtual void slot18(float*, float*);
};

struct SwingObjectBase { char pad[0x44]; };
struct SwingObject : SwingObjectBase {
    virtual void slot0();
    virtual void slot1();
    virtual void finish();
    virtual void slot3();
    virtual void confirm(int);
    virtual void slot5();
    virtual void slot6();
    virtual float slot7(int, float);
    virtual int update(int, float, float, int);
    virtual void slot9();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
    virtual void slot13();
    virtual int result(int, float);
    virtual int state(int, float);
};

struct ComPlayerIndexView {
	unsigned char reserved00[0x3C1];
	unsigned char index;
};

struct ComSharedStateView {
	unsigned char reserved00[0xB90];
	ComPlayerIndexView* manager;
};

struct ComCurveStatView {
	unsigned char reserved00[0xE8];
	unsigned int curve;
};

struct ComComparisonArgument {
	unsigned char reserved00[8];
	int threshold;
};

struct ComPowerStatView {
	unsigned char reserved00[0xD8];
	int power;
};

struct ComHolePlayerView {
	unsigned char reserved00[0x3B6];
	unsigned char holeIndex;
	unsigned char reserved3B7[0x0A];
	unsigned char index;
};

struct ComThresholdArgument {
	unsigned char reserved00[8];
	int threshold;
};

struct ComLevelRecordView {
	unsigned char reserved00[0x1A4];
	signed char level;
};

struct ComGaugeView {
	unsigned char reserved00[0xA8];
	float gauge;
};

struct ComClubArgument {
	unsigned char reserved00[24];
	unsigned int clubIndex;
};

struct ComClubSelection {
	unsigned char reserved00[16];
	unsigned int clubIndex;
};

struct ComRuleTreeHeader {
    void* first;
    void* second;
    char reserved08[4];
    void* current;
};

struct TargetSwingObject : SwingObjectBase {
    virtual void slot0();
    virtual void slot1();
    virtual void reset();
    virtual void slot3();
    virtual void confirm(int);
    virtual float value(int);
    virtual void slot6();
    virtual void slot7();
    virtual void slot8();
    virtual int update(int, float, float);
    virtual void begin(int, float);
    virtual void finish(int);
    virtual void step(int);
    virtual void slot13();
    virtual void slot14();
    virtual void slot15();
    virtual void slot16();
    virtual void slot17();
    virtual void start(int);
};
struct TargetSwingGame : SwingGameBase {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void slot8();
    virtual int ready();
};
struct TargetSwingCtrlBase { char pad[0x58]; };
struct TargetSwingCtrl : TargetSwingCtrlBase {
    virtual void slot0();
    virtual void finish();
};
struct ActiveSwingObject : SwingObjectBase {
    virtual void slot0();
    virtual void slot1();
    virtual void reset();
    virtual void slot3();
    virtual void confirm(int);
    virtual void slot5();
    virtual float update(int, float, int, int, int);
    virtual void slot7();
    virtual void slot8();
    virtual void slot9();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
    virtual void slot13();
    virtual void slot14();
    virtual void slot15();
    virtual int base();
    virtual int offset(int, float);
};
struct ActiveSwingGame : SwingGameBase {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void prepare();
    virtual void slot9();
    virtual void apply(int);
};
struct ActiveSwingCtrl : TargetSwingCtrlBase {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void slot8();
    virtual void slot9();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
    virtual void slot13();
    virtual void slot14();
    virtual void slot15();
    virtual void slot16();
    virtual void slot17();
    virtual void slot18();
    virtual void slot19();
    virtual void slot20();
    virtual void slot21();
    virtual void slot22();
    virtual void slot23();
    virtual void slot24();
    virtual void slot25();
    virtual void prepare(int);
    virtual void finish();
    virtual void slot28();
    virtual void step();
};

extern "C" {

void RenderContext_ResetPlayerRecords(void*);
void* CharacterPreviewTable_dtor(void*, int);
void appFreeUni__FPv(void*);

struct RenderContextConstructionCleanup_u802283AC {
    char* base;
    bool active;
    // Guard initialized render-context members until the first reset succeeds.
    explicit RenderContextConstructionCleanup_u802283AC(void* object)
        : base((char*)object), active(true) {}
    // On failure, destroy enemy records and release all four vector buffers.
    ~RenderContextConstructionCleanup_u802283AC() {
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

// Reset a newly constructed render context while retaining exceptional-exit cleanup.
static void InitializeNewRenderContext(void* base) {
    RenderContextConstructionCleanup_u802283AC cleanup(base);
    RenderContext_ResetPlayerRecords(base);
    cleanup.active = false;
}

void appFreeUni__FPv(void*);

// Narrow and wide strings share the heap flag and pointer words.
struct StringUnwindCleanup_u802283AC {
    const unsigned int* words;
    bool active;
    // Arm heap-string cleanup for a fully constructed temporary.
    explicit StringUnwindCleanup_u802283AC(const void* string)
        : words((const unsigned int*)string), active(true) {}
    // Disarm the guard after explicit cleanup or ownership transfer.
    void release() { active = false; }
    // Free heap-backed temporary text when normal cleanup has not disarmed the guard.
    ~StringUnwindCleanup_u802283AC() {
        if (active && (words[0] & 0x80000000u)) {
            appFreeUni__FPv((void*)words[2]);
        }
    }
};


extern const float kFloat_0_PadSwing;
extern const float kPadSwingFinishDelaySeconds;
void* InstanceTable_Get(void);
int EventTiming_IsDurationScalingEnabled(void);
void Vec3_SetAll(void*, float);
extern float kFloat_0_8333333_PadSwing;
extern float kPadSwingSecondarySampleLimitScale;
extern float kFloat_30_PadSwing_805E00D8;
extern const float kFloat_neg1_PadSwing;
extern float kFloat_4_SwingControl;
extern float kFloat_1_swingControl;
extern float kPadSwingTimingFalloffSpan;
extern float kFloat_10_PadSwing;
extern float kFloat_50_PadSwing;
extern float kFloat_5_PadSwing;
extern float kFloat_25_PadSwing;
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[0x100];
extern char s_gameInputManagerDtorRegistration_802283AC[0x100];
void InputManager_Init(void*);
void InputManager_Dtor(void);
void* PartyItemSlots_GetInstance(void*);
int Input_IsActionButtonDown(void*, int, int);
int Input_IsActionPressed(void*, int, int);
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[];
extern char s_fadeOverlayDtorRegistration_802283AC[];
void FadeOverlay_Init(void*);
void ResourceState_Dtor(void);
void FadeState_RequestFramebufferCapture(void*);
void FadeState_StartFadeIn(void*, int);
void* FadeState_GetInstance();
void FadeState_RequestReleaseOnFadeCompletion(void*);
void FadeState_StartFadeOut(void*, int);
extern float g_totalShotGaugeValue;
extern const float kFloat_1000000_ShotGauge;
int __fpclassifyf(float value);
float ShotParameters_GetGoodImpactHalfWidth(void* value);
extern float kFloat_360_PadSwing;
extern float kFloat_3_4906578_PadSwingLimits;
extern float kFloat_4_712388_PadSwingLimits;
extern float kFloat_5_2359867_PadSwingParameters;
extern float kPadSwingWedgeFullPowerAngle;
extern float kPadSwingPutterRampStartAngle;
extern float kPadSwingPutterFullPowerAngle;
extern float kPadSwingPutterFalloffStartAngle;
extern float kPadSwingPutterFalloffEndAngle;
extern float kFloat_149_PadSwing;
extern float kFloat_0_1;
extern float kPadSwingNegativePowerStepLimit;
extern float kFloat_0_005_PadSwing;
extern float kFloat_neg0_005_PadSwing;
extern float kPadSwingSampleFactorOrigin;
extern float kPadSwingSampleFactorSpan;
extern float kPadSwingSampleFactorScale;
extern float kFloat_3_PadSwing;
extern const float kFloat_0_5_PadSwing;
extern float kFloat_0_3_PadSwing;
extern float kFloat_2_5_SwingGauge;
extern float kPadSwingWedgeWidthBase;
extern float kPadSwingWoodIronFirstHeightBoost;
extern float kPadSwingWedgeFirstHeightBoost;
extern float kFloat_0_2_PadSwing;
extern float kFloat_0_07_PadSwing;
extern float kFloat_0_14_PadSwing;
extern float kFloat_0_050000001_PadSwing;
extern float kPadSwingMode2SideAMiddleThreshold;
extern float s_padSwingPowerFactors[];
void CTask_InitActors(void*);
void CTask_PreLoadInitActors(void*);
void CTask_ProcessActors(void*, float);
void CTask_DisplayActorsByPriority(void*);
void CTask_NotifyActorsDestroy(void*);
void CTask_Ctor(void*);
void CTask_dtor(void*, int);
void CVSMain_dtor(void*, int);
void __dl__FPv(void*);
extern unsigned char s_fightMainTypeDescriptor;
extern unsigned char s_fightTaskTypeDescriptor;
extern float kFloat_6_2831855;
extern float s_padSwingTwoPi;
extern float s_fightMainTwoPi;
extern float s_fightTaskTwoPi;
extern float kFloat_neg1_PadSwing_805E01A0;
extern float kFloat_1_EffectParameters;
extern float kComPowerCurveRight;
extern float kComPowerCurveLeft;
void CVSMain_Ctor(void*);
extern unsigned char __vt__10CFightMain[];
extern unsigned char __vt__10CFightTask[];
extern unsigned int g_defaultActorSkipMask;
extern unsigned int g_actorSkipMask;
extern void* g_sharedDoc;
extern float kComThinkSpinCurveStatDivisor;
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, const char*, const char*, const unsigned char*);
void* TaskActorTable_CreateNamedActor(void*, const char*, const char*, int, void*);
void CTask_SetMainActor(void*, void*);
void appFreeUni__FPv(void*);
extern char s_fightMainActorName[];
extern signed char s_renderContextInitialized;
extern char s_renderContext[];
extern char s_renderContextDtorRegistration_802283AC[];
void CVSMain_HandleMsg(void*, void*);
void SaveData_InitPlayerRecord(void*);


void PartyItemRecord_InitDefaults(void*);
void PlayerRenderRecord_dtor(void*);
void RenderContext_ResetPlayerRecords(void*);
void __dt__13RenderContextFv(void*);
void* __nw__FUl(unsigned int);
void CCharaNum_Ctor(void*);
void CCharaSelect_Ctor(void*);
extern float s_comThinkActionTwoPi;
extern float kComThinkGaugeThresholdScale;
extern char comThinkDebugStrings[];

// Packed external string pool; member extents come from the global catalog.
struct ComDebugStringPool {
    char gauge[14];
    char level[14];
    char randomValue[13];
    char shotCount[16];
    char lastOB[20];
    char power[14];
    char passed[3];
    char failed[3];
    char afterBoost[27];
    char afterCarry[27];
    char afterAreaStop[28];
    char padding[5];
};
#define ComDebug ((ComDebugStringPool*)comThinkDebugStrings)

extern float s_comThinkConditionTwoPi;
int CComThinkCtrl_GetSpecialLandingResult(void*); // Read the AI boost-check setting.
int CComThinkCtrl_GetFirstLandingWithinTargetResult(void*); // Read the AI carry-check setting.
int CComThinkCtrl_GetFinalWithinTargetResult(void*); // Read the AI area-stop-check setting.
void* CComThinkCtrl_GetCommandState(void*); // Return the AI command-state block that holds cached results.
extern char __vt__13CComThinkCtrl[];
void ComConditionMap_Clear(void*);
void CComThinkCtrl_ClearStringTree(void*);
void CSharedFileHandle_Destroy(void*, int);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float WVectorLen__FRC7WVector(void*);
extern float kFloat_0_ComThinkCupDistance;
extern float kComThinkCupDistanceDivisor;
extern char comThinkCommandStrings[];

// Packed external string pool; member extents come from the global catalog.
struct ComCommandStringPool {
    char IS_GAUGE_OVER[14];
    char IS_GAUGE_UNDER[15];
    char IS_GAUGE_SAME[14];
    char IS_COM_LEVEL_OVER[18];
    char IS_COM_LEVEL_UNDER[19];
    char IS_COM_LEVEL_SAME[18];
    char IS_COM_WIN[11];
    char IS_COM_LOSE[12];
    char IS_RANDOM[10];
    char IS_ALWAYS[10];
    char IS_NOTHING[11];
    char IS_TEESHOT[11];
    char IS_OB[6];
    char IS_BOOST[9];
    char IS_PAR_POWER[13];
    char IS_CARRY[9];
    char IS_AREA_STOP[13];
    char IS_SHOT_NUM_SAME[17];
    char IS_SHOT_NUM_OVER[17];
    char IS_SHOT_NUM_UNDER[18];
    char IS_CUP_LEN_OVER[16];
    char IS_CUP_LEN_UNDER[17];
    char IS_UNDER_POWER[15];
    char DO_NEXT_STEP[13];
    char DO_POWER_CURVE_RIGHT[21];
    char DO_POWER_CURVE_LEFT[20];
    char DO_POWER_TOP_SPIN[18];
    char DO_POWER_BACK_SPIN[19];
    char DO_POWER_SHOT[14];
    char DO_TOMAHAWK[12];
    char DO_COBRA[9];
    char DO_NORMAL[10];
    char DO_UP[6];
    char DO_DOWN[8];
    char DO_RIGHT[9];
    char DO_LEFT[8];
    char DO_UP_RIGHT[12];
    char DO_UP_LEFT[11];
    char DO_DOWN_RIGHT[14];
    char DO_DOWN_LEFT[13];
    char DO_AREA_OVER[13];
    char DO_SPIKE[9];
    char DO_DOUBLE_POWER[16];
    char IGNORE_DIFFICULTY[18];
    char DO_SET_CLUB[12];
    char s_Start_CheckPoint[48];
    char s[8];
    char d_d_d[20];
    char NG[9];
    char OK[9];
    char Action_s[15];
    char Action_d_d_d[27];
    char actionOK[8];
    char actionNG[8];
    char tree_insert_length_error[26];
    char std_exception_std_logic_error_std_length_error[55];
    char basic_string_reserve_length_error[35];
    char basic_string_out_of_range[27];
    char std_exception_std_logic_error_std_out_of_range[55];
    char basic_string_length_error[27];
    char padding[5];
};
#define ComCommands ((ComCommandStringPool*)comThinkCommandStrings)

void CComThinkCtrl_RegisterCondition(void*, int, void*, const char*);
void CComThinkCtrl_RegisterAction(void*, int, void*, const char*);
void ComConditionMap_FreeSubtree(void*, void*);
void Tree_FreeSubtree(void*, void*);

void ComConditionTree_Init(ComRuleTreeHeader* arg);
void ComActionTree_Init(ComRuleTreeHeader* arg);

// Return the stored swing result word.
int CPadSwingCtrl_vfunc_8(void* self) { return AsCPadSwingCtrl(self)->largeWoodSwingSelected; }

// Return the stored swing-size class selected by the motion controller.
int CPadSwingCtrl_vfunc_9(void* self) { return AsCPadSwingCtrl(self)->swingSizeClass; }

// Return whether the stored power threshold has been reached.
int CPadSwingCtrl_vfunc_10(void* self) { return AsCPadSwingCtrl(self)->powerThresholdReached; }

#pragma opt_lifetimes off
// Derive the clamped integer swing tolerance from sampled motion and timing.
int CPadSwingCtrl_GetTimingScaledSampleScore(void* self) {
    int count = 0;
    int zero = 0;
    void* root = InstanceTable_Get();
    int mode = ((SwingInstanceView*)root)->selectedClub->clubGroup;
    switch (mode) {
        case 0: count = 7; break;
        case 1: count = 6; break;
        case 2: count = 4; break;
        case 3: count = 0; break;
        default: break;
    }
    float scale;
    if (EventTiming_IsDurationScalingEnabled()) scale = kFloat_0_8333333_PadSwing * (float)count;
    else scale = (float)count;
    float vec[3];
    Vec3_SetAll(vec, kFloat_0_PadSwing);
    (*(SwingGame**)self)->sample(&zero, vec, scale);
    float limit = kPadSwingSecondarySampleLimitScale * vec[1];
    scale = vec[0];
    scale = (scale < limit) ? scale : limit;
    float time = (*(SwingGame**)self)->time();
    time = kFloat_30_PadSwing_805E00D8 * time;
    if (time < kFloat_0_PadSwing) time = time * kFloat_neg1_PadSwing;
    float factor;
    if (time < kFloat_4_SwingControl) factor = kFloat_1_swingControl;
    else factor = kFloat_1_swingControl - ((time - kFloat_4_SwingControl) / kPadSwingTimingFalloffSpan);
    scale *= factor;
    float answer = kFloat_5_PadSwing + (kFloat_25_PadSwing * ((scale - kFloat_10_PadSwing) / kFloat_50_PadSwing));
    int result = (int)answer;
    if (result > 30) result = 30;
    else if (result < 5) result = 5;
    return result;
}
#pragma opt_lifetimes reset

// Convert the sampled motion total to a bounded finite swing factor.
float CPadSwingCtrl_GetSampledMotionFactor(void* self) {
    int count = 0;
    int zero = count;
    void* root = InstanceTable_Get();
    int clubGroup = ((SwingInstanceView*)root)->selectedClub->clubGroup;
    switch (clubGroup) {
        case kWoodClubGroup: count = 6; break;
        case kIronClubGroup: count = 6; break;
        case kWedgeClubGroup: count = 6; break;
        case kPutterClubGroup: count = 3; break;
        default: break;
    }
    float value;
    if (EventTiming_IsDurationScalingEnabled()) value = kFloat_0_8333333_PadSwing * (float)count;
    else value = (float)count;
    float vec[3];
    Vec3_SetAll(vec, kFloat_0_PadSwing);
    (*(SwingGame**)self)->sample(&zero, vec, value);
    float total = vec[0] + vec[1] + vec[2];
    float factor = kPadSwingSampleFactorScale * ((total - kPadSwingSampleFactorOrigin) / kPadSwingSampleFactorSpan) + kFloat_0_1;
    if (factor > kFloat_3_PadSwing) factor = kFloat_3_PadSwing;
    else if (factor < kFloat_0_1) factor = kFloat_0_1;
    if (__fpclassifyf(factor) == 1 || __fpclassifyf(factor) == 2) factor = kFloat_0_1;
    return factor;
}

// Request a swing start on the next controller update.
int CPadSwingCtrl_vfunc_13(void* self) { AsCPadSwingCtrl(self)->swingStartRequested = 1; return 1; }

// Configure a normalized power ramp and reset its frame cursor.
int CPadSwingCtrl_SetPowerRamp(void* self, float value, int count) {
    if (value < kFloat_0_PadSwing || value > kFloat_1_swingControl) return 0;
    if (count < 0) return 0;
    AsCPadSwingCtrl(self)->rampTarget = value;
    AsCPadSwingCtrl(self)->rampFrames = count;
    AsCPadSwingCtrl(self)->rampFrame = 0;
    return 1;
}

// Cancel the configured power ramp and reset its target/cursor.
int CPadSwingCtrl_CancelPowerRamp(void* self) {
    float f = kFloat_0_PadSwing;
    AsCPadSwingCtrl(self)->rampFrames = -1;
    AsCPadSwingCtrl(self)->rampTarget = f;
    AsCPadSwingCtrl(self)->rampFrame = 0;
    return 1;
}

// Return the cached power-ramp running flag.
int CPadSwingCtrl_vfunc_16(void* self) { return AsCPadSwingCtrl(self)->rampRunning; }

// Set and return the pending primary swing request.
int CPadSwingCtrl_vfunc_17(void* self, int value) { AsCPadSwingCtrl(self)->primarySwingRequested = value; return value; }

// Set and return the pending alternate swing request.
int CPadSwingCtrl_vfunc_18(void* self, int value) { AsCPadSwingCtrl(self)->alternateSwingRequested = value; return value; }

// Latch forced swinging when either programmatic swing request is present.
int CPadSwingCtrl_SetForcedSwingIfRequested(void* self) {
    if (AsCPadSwingCtrl(self)->primarySwingRequested || AsCPadSwingCtrl(self)->alternateSwingRequested) {
        AsCPadSwingCtrl(self)->forcedSwingActive = 1;
        return 1;
    }
    return 0;
}

// Accept state48 values below 4 unless state4C is 6; state labels remain unknown.
// For normal nonnegative state codes this admits 0,1,2,3 and rejects 4 upward.
// Arithmetic right shift also admits negative state48 values; retain that ABI behavior.
int CPadSwingCtrl_vfunc_20(void* self) {
    if (AsCPadSwingCtrl(self)->readinessInhibitCode == kSwingGateBlockedValue) return 0;
    int value = AsCPadSwingCtrl(self)->readinessRangeCode ^ kSwingStateRangeMask;
    unsigned shifted = value >> 1;
    shifted -= value & kSwingStateRangeMask;
    return shifted >> kWordSignShift;
}

// Accumulate the shot gauge, cap its maximum and clear non-finite results.
int ShotGauge_AddClampedTotal(float delta) {
    float value = g_totalShotGaugeValue + delta;
    g_totalShotGaugeValue = value;
    if (value > kFloat_1000000_ShotGauge) {
        value = kFloat_1000000_ShotGauge;
        g_totalShotGaugeValue = value;
    }
    if (__fpclassifyf((double)value) == 1 || __fpclassifyf(g_totalShotGaugeValue) == 2)
        g_totalShotGaugeValue = kFloat_0_PadSwing;
    return 1;
}

// Reset the shared accumulated shot-gauge value to zero and return success.
int ShotGauge_ResetTotal(void) { g_totalShotGaugeValue = kFloat_0_PadSwing; return 1; }

// Return the shared accumulated shot-gauge value.
float ShotGauge_GetTotal(void) { return g_totalShotGaugeValue; }

// Handle cancellation or prepare the target swing object when swinging becomes ready.
int CPadSwingCtrl_UpdateIdlePhase(void* self, int arg, int mode) {
    void* state = PartyItemSlots_GetInstance(self);
    if (((int&)Sing_805DC5C8()) == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_gameInputManagerDtorRegistration_802283AC);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    if ((Input_IsActionPressed((void*)((int&)Sing_805DC5C8()), 6, -1) && mode != 3) ||
        ((SwingCancelView*)state)->cancelRequested != 0) {
        ((*(TargetSwingObject**)&AsCPadSwingCtrl(self)->motionController))->confirm(arg);
        AsCPadSwingCtrl(self)->swingPower = kFloat_0_PadSwing;
        return kSwingCancelled;
    }
    AsCPadSwingCtrl(self)->swingPower = kFloat_0_PadSwing;
    AsCPadSwingCtrl(self)->smoothedMotionFraction = ((*(TargetSwingObject**)&AsCPadSwingCtrl(self)->motionController))->value(arg);
    if ((*(TargetSwingGame**)self)->ready() || AsCPadSwingCtrl(self)->swingStartRequested) {
        AsCPadSwingCtrl(self)->primarySwingButtonArmed = 0;
        AsCPadSwingCtrl(self)->alternateSwingButtonArmed = 0;
        ((*(TargetSwingObject**)&AsCPadSwingCtrl(self)->motionController))->reset();
        ((*(TargetSwingObject**)&AsCPadSwingCtrl(self)->motionController))->start(arg);
    }
    return kSwingIdle;
}

// Query a held swing action through the lazily initialized game input manager.
inline int ReadSwingButton(int button) {
    if (!((int&)Sing_805DC5C8())) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_gameInputManagerDtorRegistration_802283AC);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    return Input_IsActionButtonDown((void*)((int&)Sing_805DC5C8()), button, -1);
}

#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
// Arm swing buttons, start the selected motion path and report its controller state.
int CPadSwingCtrl_UpdateReadyPhase(void* self, int ownArg, int arg, int mode) {
    int result = kSwingReady;
    void* timer = PartyItemSlots_GetInstance(self);
    if (!((int&)Sing_805DC5C8())) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_gameInputManagerDtorRegistration_802283AC);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    if ((Input_IsActionPressed((void*)((int&)Sing_805DC5C8()), 6, -1) && mode != 3) ||
        ((SwingCancelView*)timer)->cancelRequested) {
        ((*(ActiveSwingObject**)&AsCPadSwingCtrl(self)->motionController))->confirm(arg);
        AsCPadSwingCtrl(self)->swingPower = kFloat_0_PadSwing;
        return kSwingCancelled;
    }
    (*(ActiveSwingGame**)self)->prepare();
    ((ActiveSwingCtrl*)self)->prepare(ownArg);
    ((ActiveSwingCtrl*)self)->step();
    if (!((SwingInstanceView*)InstanceTable_Get())->powerShotMode && mode != 3) {
        AsCPadSwingCtrl(self)->smoothedMotionFraction = ((*(ActiveSwingObject**)&AsCPadSwingCtrl(self)->motionController))->update(arg, AsCPadSwingCtrl(self)->swingPower, 1, 0, 0);
    }
    if (!((int&)Sing_805DC5C8())) {
        if (!s_inputManagerInitialized) { InputManager_Init(&s_gameInputManager); __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_gameInputManagerDtorRegistration_802283AC); s_inputManagerInitialized = 1; }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    if (!Input_IsActionButtonDown((void*)((int&)Sing_805DC5C8()), 11, -1)) AsCPadSwingCtrl(self)->primarySwingButtonArmed = 1;
    if (!((int&)Sing_805DC5C8())) {
        if (!s_inputManagerInitialized) { InputManager_Init(&s_gameInputManager); __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_gameInputManagerDtorRegistration_802283AC); s_inputManagerInitialized = 1; }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    if (!Input_IsActionButtonDown((void*)((int&)Sing_805DC5C8()), 13, -1)) AsCPadSwingCtrl(self)->alternateSwingButtonArmed = 1;
    if ((AsCPadSwingCtrl(self)->primarySwingButtonArmed && ReadSwingButton(11)) ||
        AsCPadSwingCtrl(self)->primarySwingRequested) {
        ((*(ActiveSwingObject**)&AsCPadSwingCtrl(self)->motionController))->reset();
        AsCPadSwingCtrl(self)->primarySwingButtonArmed = 1;
        AsCPadSwingCtrl(self)->alternateSwingButtonArmed = 0;
        result = kSwingPrimaryActive;
        {
            int base = ((*(ActiveSwingObject**)&AsCPadSwingCtrl(self)->motionController))->base();
            int offset = ((*(ActiveSwingObject**)&AsCPadSwingCtrl(self)->motionController))->offset(arg, AsCPadSwingCtrl(self)->swingPower);
            (*(ActiveSwingGame**)self)->apply(base + offset);
        }
        ((ActiveSwingCtrl*)self)->finish();
    } else if ((AsCPadSwingCtrl(self)->alternateSwingButtonArmed && ReadSwingButton(13)) ||
               AsCPadSwingCtrl(self)->alternateSwingRequested) {
        void* root = InstanceTable_Get();
        void* info = ((SwingInstanceView*)root)->selectedClub;
        if (((SwingModeView*)info)->clubGroup != 3) {
            ((*(ActiveSwingObject**)&AsCPadSwingCtrl(self)->motionController))->reset();
            AsCPadSwingCtrl(self)->primarySwingButtonArmed = 0;
            AsCPadSwingCtrl(self)->alternateSwingButtonArmed = 1;
            result = kSwingAlternateActive;
            {
                int base = ((*(ActiveSwingObject**)&AsCPadSwingCtrl(self)->motionController))->base();
                int offset = ((*(ActiveSwingObject**)&AsCPadSwingCtrl(self)->motionController))->offset(arg, AsCPadSwingCtrl(self)->swingPower);
                (*(ActiveSwingGame**)self)->apply(base + offset);
            }
            ((ActiveSwingCtrl*)self)->finish();
        }
    }
    return result;
}
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs reset

// Advance the primary swing while tracking its button and motion completion status.
int CPadSwingCtrl_UpdateForwardPhase(void* self, float delta, int arg, int mode) {
    int result = (AsCPadSwingCtrl(self)->primarySwingButtonArmed != 0) ? (int)kSwingPrimaryActive : (int)kSwingAlternateActive;
    void* root = InstanceTable_Get();
    if (((SwingInstanceView*)root)->powerShotMode == 0 && mode != 3) {
        AsCPadSwingCtrl(self)->smoothedMotionFraction = ((*(SwingObject**)&AsCPadSwingCtrl(self)->motionController))->slot7(arg, AsCPadSwingCtrl(self)->swingPower);
    }
    if ((AsCPadSwingCtrl(self)->primarySwingButtonArmed &&
         (ReadSwingButton(11) || AsCPadSwingCtrl(self)->primarySwingRequested)) ||
        (AsCPadSwingCtrl(self)->alternateSwingButtonArmed &&
         (ReadSwingButton(13) || AsCPadSwingCtrl(self)->alternateSwingRequested)))
    {
        void* root2 = InstanceTable_Get();
        int status = (*(SwingGame**)self)->slot11(((SwingInstanceView*)root2)->selectedClub->clubGroup == 3, delta);
        if (status == 2 || AsCPadSwingCtrl(self)->forcedSwingActive) {
            ((*(SwingObject**)&AsCPadSwingCtrl(self)->motionController))->finish();
        }
    } else
    {
        ((*(SwingObject**)&AsCPadSwingCtrl(self)->motionController))->slot3();
        (*(SwingGame**)self)->slot10(0);
    }
    return result;
}

#pragma opt_common_subs off
// Advance the alternate swing and save its completion result and elapsed time.
int CPadSwingCtrl_UpdateSettlingPhase(void* self, float delta, int arg, int mode) {
    float saved = delta;
    int flags = AsCPadSwingCtrl(self)->primarySwingButtonArmed;
    int result = (flags != 0) ? (int)kSwingPrimaryActive : (int)kSwingAlternateActive;
    void* root = InstanceTable_Get();
    if (((SwingInstanceView*)root)->powerShotMode == 0 && mode != 3) {
        mode = ((*(SwingObject**)&AsCPadSwingCtrl(self)->motionController))->update(arg, AsCPadSwingCtrl(self)->swingPower, saved, 0);
        AsCPadSwingCtrl(self)->largeWoodSwingSelected = ((*(SwingObject**)&AsCPadSwingCtrl(self)->motionController))->result(arg, AsCPadSwingCtrl(self)->swingPower);
        AsCPadSwingCtrl(self)->swingSizeClass = ((*(SwingObject**)&AsCPadSwingCtrl(self)->motionController))->state(arg, AsCPadSwingCtrl(self)->swingPower);
    } else { mode = 1; AsCPadSwingCtrl(self)->swingSizeClass = 2; }
    root = InstanceTable_Get();
    int status = (*(SwingGame**)self)->slot11(((SwingInstanceView*)root)->selectedClub->clubGroup == 3, saved);
    if ((status == 3 || AsCPadSwingCtrl(self)->forcedSwingActive) && mode == 1) {
        if (result == kSwingPrimaryActive) {
            result = kSwingPrimaryComplete;
            ((*(SwingObject**)&AsCPadSwingCtrl(self)->motionController))->confirm(arg);
        } else {
            result = kSwingAlternateComplete;
            ((*(SwingObject**)&AsCPadSwingCtrl(self)->motionController))->finish();
            AsCPadSwingCtrl(self)->finishElapsed = kFloat_neg1_PadSwing;
        }
    }
    return result;
}
#pragma opt_common_subs reset

// Finish the swing, advance its post-swing state and update the controller power.
int CPadSwingCtrl_UpdateFinishPhase(void* self, float delta, int arg, int mode) {
    int result = kSwingFinishing;
    if (kFloat_neg1_PadSwing == AsCPadSwingCtrl(self)->finishElapsed) {
        void* root = InstanceTable_Get();
        if (((SwingInstanceView*)root)->powerShotMode != 0 || mode == 3) {
            ((*(TargetSwingObject**)&AsCPadSwingCtrl(self)->motionController))->begin(arg, delta);
        }
        int ok = ((*(TargetSwingObject**)&AsCPadSwingCtrl(self)->motionController))->update(arg, AsCPadSwingCtrl(self)->swingPower, delta);
        if (ok) {
            if (!((int&)Sing_805DC608())) {
                if (!s_fadeOverlayInitialized) { FadeOverlay_Init(&s_fadeOverlay); __register_global_object(&s_fadeOverlay, (void*)ResourceState_Dtor, &s_fadeOverlayDtorRegistration_802283AC); s_fadeOverlayInitialized = 1; }
                ((int&)Sing_805DC608()) = (int)&s_fadeOverlay;
            }
            FadeState_RequestFramebufferCapture((void*)((int&)Sing_805DC608()));
            AsCPadSwingCtrl(self)->finishElapsed = kFloat_0_PadSwing;
        }
    } else {
    if (kFloat_0_PadSwing == AsCPadSwingCtrl(self)->finishElapsed) {
        if (!((int&)Sing_805DC608())) {
            if (!s_fadeOverlayInitialized) { FadeOverlay_Init(&s_fadeOverlay); __register_global_object(&s_fadeOverlay, (void*)ResourceState_Dtor, &s_fadeOverlayDtorRegistration_802283AC); s_fadeOverlayInitialized = 1; }
            ((int&)Sing_805DC608()) = (int)&s_fadeOverlay;
        }
        FadeState_StartFadeIn((void*)((int&)Sing_805DC608()), 0);
        void* receiver783_fn_8023E120 = (void*)FadeState_GetInstance(); FadeState_RequestReleaseOnFadeCompletion(receiver783_fn_8023E120);
        if (EventTiming_IsDurationScalingEnabled()) {
            if (!((int&)Sing_805DC608())) {
                if (!s_fadeOverlayInitialized) { FadeOverlay_Init(&s_fadeOverlay); __register_global_object(&s_fadeOverlay, (void*)ResourceState_Dtor, &s_fadeOverlayDtorRegistration_802283AC); s_fadeOverlayInitialized = 1; }
                ((int&)Sing_805DC608()) = (int)&s_fadeOverlay;
            }
            FadeState_StartFadeOut((void*)((int&)Sing_805DC608()), 0x19);
        } else {
            if (!((int&)Sing_805DC608())) {
                if (!s_fadeOverlayInitialized) { FadeOverlay_Init(&s_fadeOverlay); __register_global_object(&s_fadeOverlay, (void*)ResourceState_Dtor, &s_fadeOverlayDtorRegistration_802283AC); s_fadeOverlayInitialized = 1; }
                ((int&)Sing_805DC608()) = (int)&s_fadeOverlay;
            }
            FadeState_StartFadeOut((void*)((int&)Sing_805DC608()), 0x1E);
        }
        ((*(TargetSwingObject**)&AsCPadSwingCtrl(self)->motionController))->finish(arg);
        AsCPadSwingCtrl(self)->swingPower = kFloat_0_PadSwing;
    } else if (AsCPadSwingCtrl(self)->finishElapsed >= kPadSwingFinishDelaySeconds) {
        void* root = InstanceTable_Get();
        if (((SwingInstanceView*)root)->powerShotMode != 0 || mode == 3) {
            ((*(TargetSwingObject**)&AsCPadSwingCtrl(self)->motionController))->finish(arg);
        }
        ((*(TargetSwingObject**)&AsCPadSwingCtrl(self)->motionController))->confirm(arg);
        ((TargetSwingCtrl*)self)->finish();
        result = kSwingFinished;
    } else {
        void* root = InstanceTable_Get();
        if (((SwingInstanceView*)root)->powerShotMode != 0 || mode == 3) {
            ((*(TargetSwingObject**)&AsCPadSwingCtrl(self)->motionController))->step(arg);
        }
    }
    AsCPadSwingCtrl(self)->finishElapsed += delta;
    }
    return result;
}

// Convert the swing angle into bounded power using mode-specific limits.
int CPadSwingCtrl_UpdateSwingPowerFromAngle(void* self, float angle, void* limits) {
    float rampStartAngle = kFloat_0_PadSwing;
    float fullPowerAngle = rampStartAngle;
    float falloffStartAngle = rampStartAngle;
    float falloffEndAngle = rampStartAngle;
    float powerError, currentPower, targetPower, powerStep;
    void* root = InstanceTable_Get();
    int clubGroup = ((SwingInstanceView*)root)->selectedClub->clubGroup;
    switch (clubGroup) {
    case kWoodClubGroup:
        rampStartAngle = kFloat_0_PadSwing; fullPowerAngle = kFloat_3_4906578_PadSwingLimits; falloffStartAngle = kFloat_4_712388_PadSwingLimits; falloffEndAngle = kFloat_5_2359867_PadSwingParameters; break;
    case kIronClubGroup:
        rampStartAngle = kFloat_0_PadSwing; fullPowerAngle = kFloat_3_4906578_PadSwingLimits; falloffStartAngle = kFloat_4_712388_PadSwingLimits; falloffEndAngle = kFloat_5_2359867_PadSwingParameters; break;
    case kWedgeClubGroup:
        rampStartAngle = kFloat_0_PadSwing; fullPowerAngle = kPadSwingWedgeFullPowerAngle; falloffStartAngle = kFloat_4_712388_PadSwingLimits; falloffEndAngle = kFloat_5_2359867_PadSwingParameters; break;
    case kPutterClubGroup:
        rampStartAngle = kPadSwingPutterRampStartAngle; fullPowerAngle = kPadSwingPutterFullPowerAngle; falloffStartAngle = kPadSwingPutterFalloffStartAngle; falloffEndAngle = kPadSwingPutterFalloffEndAngle; break;
    }
    if (angle < rampStartAngle) targetPower = kFloat_0_PadSwing;
    else if (angle >= rampStartAngle && angle <= fullPowerAngle) targetPower = (angle - rampStartAngle) / (fullPowerAngle - rampStartAngle);
    else if (angle <= falloffStartAngle) targetPower = kFloat_1_swingControl;
    else if (angle <= falloffEndAngle) targetPower = (falloffEndAngle - angle) / (falloffEndAngle - falloffStartAngle);
    else targetPower = kFloat_0_PadSwing;
    if (kFloat_149_PadSwing != ((SwingAngleLimitsView*)limits)->maximumAngle) {
        float maximum = (((SwingAngleLimitsView*)limits)->maximumAngle - kFloat_149_PadSwing) / kFloat_360_PadSwing;
        if (targetPower > maximum) targetPower = maximum;
    }
    currentPower = AsCPadSwingCtrl(self)->swingPower;
    powerError = targetPower - currentPower;
    powerStep = powerError / kFloat_5_PadSwing;
    if (powerStep > kFloat_0_1) AsCPadSwingCtrl(self)->swingPower = currentPower + kFloat_0_1;
    else if (powerStep < kPadSwingNegativePowerStepLimit) AsCPadSwingCtrl(self)->swingPower = currentPower - kFloat_0_1;
    else if (powerStep < kFloat_0_005_PadSwing && powerStep > kFloat_0_PadSwing) {
        if (powerError < kFloat_0_005_PadSwing) AsCPadSwingCtrl(self)->swingPower = targetPower;
        else AsCPadSwingCtrl(self)->swingPower = currentPower + kFloat_0_005_PadSwing;
    } else if (powerStep > kFloat_neg0_005_PadSwing && powerStep <= kFloat_0_PadSwing) {
        float negative = targetPower - AsCPadSwingCtrl(self)->swingPower;
        if (negative > kFloat_neg0_005_PadSwing) AsCPadSwingCtrl(self)->swingPower = targetPower;
        else AsCPadSwingCtrl(self)->swingPower = AsCPadSwingCtrl(self)->swingPower - kFloat_0_005_PadSwing;
    } else AsCPadSwingCtrl(self)->swingPower += powerStep;
    return 1;
}

// Enforce the minimum impact width and reset the two gauge adjustment words.
int CPadSwingCtrl_ApplyMinimumSwingPower(void* self) {
    void* root = InstanceTable_Get();
    void* object = ((SwingInstanceView*)root)->selectedClub;
    float value;
    if (((SwingModeView*)object)->clubGroup == 3)
        value = kFloat_10_PadSwing;
    else
        value = ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get());
    if (AsCPadSwingCtrl(self)->swingPower < value / kFloat_360_PadSwing)
        AsCPadSwingCtrl(self)->swingPower = value / kFloat_360_PadSwing;
    AsCPadSwingCtrl(self)->unk_50 = 0;
    AsCPadSwingCtrl(self)->unk_54 = 0;
    return 1;
}

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Choose a power band from width/height and reduce it for the measured swing imbalance.
int CPadSwingCtrl_EvaluatePowerBand(SwingGaugeResultView* self, float* outValue, int* outIndex) {
    // Outputs: table factor and band 0..6. Return 0 selects the fixed full-power
    // fallback (scale <= 0.5 or putter); return 1 means measured bands were used.
    // Width/height remain in the manager's measurement units, thresholds scaled by
    // self->scale. No physical distance/angle unit is established for these samples.
    if (self->scale <= kFloat_0_5_PadSwing) {
        *outValue = kFloat_1_swingControl;
        *outIndex = kLastSwingPowerBand;
        return 0;
    }

    if (((SwingInstanceView*)InstanceTable_Get())->selectedClub->clubGroup == kPutterClubGroup) {
        *outValue = kFloat_1_swingControl;
        *outIndex = kLastSwingPowerBand;
        return 0;
    }

    // Phase 1: first width threshold greater than width selects band i; overflow is 6.
    int index = 0;
    float value = kFloat_1_swingControl;
    float width = ((SwingMgr*)self->manager)->width();
    float height = ((SwingMgr*)self->manager)->height();

    float widthBandStep = kFloat_0_PadSwing;
    float widthBandBase = kFloat_0_PadSwing;
    int clubGroup = (int)((SwingInstanceView*)InstanceTable_Get())->selectedClub->clubGroup;
    switch (clubGroup) {
    case kWoodClubGroup:
        widthBandStep = kFloat_0_3_PadSwing;
        widthBandBase = kFloat_2_5_SwingGauge;
        break;
    case kIronClubGroup:
        widthBandStep = kFloat_0_3_PadSwing;
        widthBandBase = kFloat_2_5_SwingGauge;
        break;
    case kWedgeClubGroup:
        widthBandStep = kFloat_0_3_PadSwing;
        widthBandBase = kPadSwingWedgeWidthBase;
        break;
    }
    widthBandStep *= self->scale;
    widthBandBase *= self->scale;
    int i;
    for (i = 0; i < kLastSwingPowerBand; ++i) {
        float threshold = widthBandStep * (float)i + widthBandBase;
        if (width < threshold) {
            index = i;
            value = s_padSwingPowerFactors[i];
            break;
        }
    }
    if (i == kLastSwingPowerBand) { index = i; value = s_padSwingPowerFactors[i]; }
    self->index = index;

    // Phase 2: height adds one or two bands, clamped to the final table entry.
    // self->index above deliberately retains the pre-height width result.
    float heightBoostOne = kFloat_0_PadSwing;
    float heightBoostTwo = kFloat_0_PadSwing;
    clubGroup = (int)((SwingInstanceView*)InstanceTable_Get())->selectedClub->clubGroup;
    switch (clubGroup) {
    case kWoodClubGroup:
        heightBoostOne = kPadSwingWoodIronFirstHeightBoost * self->scale;
        heightBoostTwo = kFloat_50_PadSwing * self->scale;
        break;
    case kIronClubGroup:
        heightBoostOne = kPadSwingWoodIronFirstHeightBoost * self->scale;
        heightBoostTwo = kFloat_50_PadSwing * self->scale;
        break;
    case kWedgeClubGroup:
        heightBoostOne = kPadSwingWedgeFirstHeightBoost * self->scale;
        heightBoostTwo = kFloat_25_PadSwing * self->scale;
        break;
    }
    if (height >= heightBoostTwo) {
        index += 2;
        if (index >= kSwingPowerBandCount) index = kLastSwingPowerBand;
        value = s_padSwingPowerFactors[index];
    } else if (height >= heightBoostOne) {
        index += 1;
        if (index >= kSwingPowerBandCount) index = kLastSwingPowerBand;
        value = s_padSwingPowerFactors[index];
    }

    // Phase 3: compare mean sample A against negative, side-scaled cutoffs.
    // sideA/B[2] are comparison magnitudes from slot18; their physical axes are
    // unresolved. Do not label them left/right or backswing/downswing without evidence.
    // sample() writes three totals and a count. Only total A affects this result;
    // B/C are required outputs and are not combined into an invented physical measure.
    float sampleTotalA = kFloat_0_PadSwing, sampleTotalB = kFloat_0_PadSwing, sampleTotalC = kFloat_0_PadSwing;
    int count = 0;
    ((SwingMgr*)self->manager)->sample(&sampleTotalA, &sampleTotalB, &sampleTotalC, &count);
    if (!count) count = 1;
    float meanSampleA = sampleTotalA / (float)count;
    float sideAThresholdOne = kFloat_0_PadSwing, sideAThresholdTwo = kFloat_0_PadSwing, sideAThresholdThree = kFloat_0_PadSwing, sideBThresholdOne = kFloat_0_PadSwing, sideBThresholdTwo = kFloat_0_PadSwing, sideBThresholdThree = kFloat_0_PadSwing;
    clubGroup = (int)((SwingInstanceView*)InstanceTable_Get())->selectedClub->clubGroup;
    switch (clubGroup) {
    case kWoodClubGroup:
        sideAThresholdOne = kFloat_0_3_PadSwing; sideAThresholdTwo = kFloat_0_2_PadSwing; sideAThresholdThree = kFloat_0_07_PadSwing;
        sideBThresholdOne = kFloat_0_14_PadSwing; sideBThresholdTwo = kFloat_0_1; sideBThresholdThree = kFloat_0_050000001_PadSwing;
        break;
    case kIronClubGroup:
        sideAThresholdOne = kFloat_0_3_PadSwing; sideAThresholdTwo = kFloat_0_2_PadSwing; sideAThresholdThree = kFloat_0_07_PadSwing;
        sideBThresholdOne = kFloat_0_14_PadSwing; sideBThresholdTwo = kFloat_0_1; sideBThresholdThree = kFloat_0_050000001_PadSwing;
        break;
    case kWedgeClubGroup:
        sideAThresholdOne = kFloat_0_2_PadSwing; sideAThresholdTwo = kPadSwingMode2SideAMiddleThreshold; sideAThresholdThree = kFloat_0_07_PadSwing;
        sideBThresholdOne = kFloat_0_1; sideBThresholdTwo = kFloat_0_07_PadSwing; sideBThresholdThree = kFloat_0_050000001_PadSwing;
        break;
    }
    float sideA[3];
    float sideB[3];
    ((SwingMgr*)self->manager)->slot18(sideA, sideB);
    sideAThresholdOne *= kFloat_neg1_PadSwing * sideA[2];
    sideAThresholdTwo *= kFloat_neg1_PadSwing * sideA[2];
    sideAThresholdThree *= kFloat_neg1_PadSwing * sideA[2];
    sideAThresholdOne *= self->scale;
    sideAThresholdTwo *= self->scale;
    sideAThresholdThree *= self->scale;
    // Each side can subtract up to three bands. A has priority: lockA suppresses B.
    // Locks are persistent fields, not local flags initialized by this query.
    if (sideB[2] > kFloat_0_5_PadSwing * sideA[2]) {
        if (meanSampleA < sideAThresholdThree) { index -= 3; self->lockA = 1; }
        else if (meanSampleA < sideAThresholdTwo) { index -= 2; self->lockA = 1; }
        else if (meanSampleA < sideAThresholdOne) { index -= 1; self->lockA = 1; }
        if (index < 0) index = 0;
        value = s_padSwingPowerFactors[index];
    }
    sideBThresholdOne *= kFloat_neg1_PadSwing * sideB[2];
    sideBThresholdTwo *= kFloat_neg1_PadSwing * sideB[2];
    sideBThresholdThree *= kFloat_neg1_PadSwing * sideB[2];
    sideBThresholdOne *= self->scale;
    sideBThresholdTwo *= self->scale;
    sideBThresholdThree *= self->scale;
    if (sideA[2] > kFloat_0_5_PadSwing * sideB[2] && !self->lockA) {
        if (meanSampleA < sideBThresholdThree) { index -= 3; self->lockB = 1; }
        else if (meanSampleA < sideBThresholdTwo) { index -= 2; self->lockB = 1; }
        else if (meanSampleA < sideBThresholdOne) { index -= 1; self->lockB = 1; }
        if (index < 0) index = 0;
        value = s_padSwingPowerFactors[index];
    }
    self->result = index;
    *outValue = value;
    *outIndex = index;
    return 1;
}
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset

// Advance the frame-based power ramp and clear its running flag at the target.
int CPadSwingCtrl_AdvancePowerRamp(char* p) {
    int frame;
    int duration;
    float t;
    duration = AsCPadSwingCtrl(p)->rampFrames;
    if (duration < 0) {
        AsCPadSwingCtrl(p)->rampRunning = 0;
        return 0;
    }
    frame = AsCPadSwingCtrl(p)->rampFrame;
    t = (float)frame * AsCPadSwingCtrl(p)->rampTarget / (float)duration;
    AsCPadSwingCtrl(p)->rampFrame = frame + 1;
    AsCPadSwingCtrl(p)->swingPower = t;
    if (frame + 1 > duration) {
        AsCPadSwingCtrl(p)->rampFrame = duration;
        AsCPadSwingCtrl(p)->rampRunning = 0;
    } else if (frame + 1 == duration) {
        AsCPadSwingCtrl(p)->rampRunning = 1;
    } else {
        AsCPadSwingCtrl(p)->rampRunning = 0;
    }
    return 1;
}

// Initialize this module's cached full-turn angle.
void PadSwing_InitTwoPiCache() { s_padSwingTwoPi = kFloat_6_2831855; }

// Construct the versus-mode main object and install its primary and task-main interfaces.
void* CFightMain_Ctor(void* p) { CVSMain_Ctor(p); void* self = p; AsCFightMain(self)->vtbl = __vt__10CFightMain; AsCFightMain(self)->secondaryVtable = __vt__10CFightMain + 0x3C; return self; }

// Restore versus interfaces, destroy the selection menu/base and optionally free the object.
void* CFightMain_dtor(void* p, int deletingFlag) {
    if (p) {
        CVSMain_dtor(p, 0);
        if (deletingFlag > 0)
            __dl__FPv(p);
    }
    return p;
}

// Only the second word is inspected here. Event storage is borrowed for the call;
// this handler neither retains nor frees it. Codes describe observed transitions,
// not inferred input buttons: 0/1 discard selection then forward; 2/3 forward;
// 4 builds stroke player-count selection; 5 builds match character selection.
enum FightSetupEvent {
    kFightDiscardSelection0 = 0, kFightDiscardSelection1 = 1,
    kFightForwardSetup2 = 2, kFightForwardSetup3 = 3,
    kFightSelectStrokePlayers = 4, kFightSelectMatchCharacters = 5
};
struct FightEvent { int unused; int type; };
#pragma opt_propagation off
// Handle versus-mode setup messages, replace selection menus and initialize shared player records.
void CFightMain_HandleMsg(void* self, const FightEvent* event) {
    // All events establish versus gameMode=1. Menu replacement deletes the prior
    // owned menu before allocation; the new menu is stored in self->selectionMenu.
    // Repeated context setup constructs four 0x828-byte player records, four vectors,
    // and four 0xA4 enemy records only on first use. Each reset keeps that storage.
    *(int*)&AsCSharedDoc(g_sharedDoc)->gameMode = 1;
    switch (event->type) {
    case kFightDiscardSelection0:
    case kFightDiscardSelection1: {
        void* old = AsCFightMain(self)->selectionMenu;
        if (old) {
            if (old) ((EventTarget*)old)->release(1);
            AsCFightMain(self)->selectionMenu = 0;
        }
    }
    // fall through
    case kFightForwardSetup2:
    case kFightForwardSetup3:
        CVSMain_HandleMsg(self, (void*)event);
        break;
    case kFightSelectStrokePlayers: {
        void* old = AsCFightMain(self)->selectionMenu;
        if (old) {
            if (old) ((EventTarget*)old)->release(1);
            AsCFightMain(self)->selectionMenu = 0;
        }
        if (!((unsigned int&)Sing_805DC648())) {
            if (!s_renderContextInitialized) {
                char* end;
                char* cursor;
                char* base;
                char* members;
                base = s_renderContext;
                cursor = base;
                end = base + 0x20A0;
                do { SaveData_InitPlayerRecord(cursor); cursor += 0x828; } while (cursor < end);
                ((IntVectorArray*)(end))->__ct();
                members = base + 0x20D0;
                __construct_array(members, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4);
                AsCEnemyRenderTable(members)->count = 0;
                InitializeNewRenderContext(base);
                __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv, s_renderContextDtorRegistration_802283AC);
                s_renderContextInitialized = 1;
            }
            ((unsigned int&)Sing_805DC648()) = (unsigned int)s_renderContext;
        }
        RenderContext_ResetPlayerRecords((void*)((unsigned int&)Sing_805DC648()));
        {
            // Construct the owned stroke player-count menu.
            struct CharaNum { char storage[0x3C]; CharaNum() { CCharaNum_Ctor(this); } };
            void* p = new CharaNum;
            AsCFightMain(self)->selectionMenu = p;
            AsCFightMain(self)->selectionState = 1;
            AsCSharedDoc(g_sharedDoc)->golfRuleType = 0;
            strcpy(AsCSharedDoc(g_sharedDoc)->pad_8C1, *(char**)&AsCSharedDoc(g_sharedDoc)->strokePlayName);
        }
        break;
    }
    case kFightSelectMatchCharacters: {
        void* old = AsCFightMain(self)->selectionMenu;
        if (old) {
            if (old) ((EventTarget*)old)->release(1);
            AsCFightMain(self)->selectionMenu = 0;
        }
        if (!((unsigned int&)Sing_805DC648())) {
            if (!s_renderContextInitialized) {
                char* cursor;
                char* base;
                char* end;
                char* members;
                base = s_renderContext;
                cursor = base;
                end = base + 0x20A0;
                do { SaveData_InitPlayerRecord(cursor); cursor += 0x828; } while (cursor < end);
                ((IntVectorArray*)(end))->__ct();
                members = base + 0x20D0;
                __construct_array(members, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4);
                AsCEnemyRenderTable(members)->count = 0;
                InitializeNewRenderContext(base);
                __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv, s_renderContextDtorRegistration_802283AC);
                s_renderContextInitialized = 1;
            }
            ((unsigned int&)Sing_805DC648()) = (unsigned int)s_renderContext;
        }
        RenderContext_ResetPlayerRecords((void*)((unsigned int&)Sing_805DC648()));
        if (!((unsigned int&)Sing_805DC648())) {
            if (!s_renderContextInitialized) {
                char* cursor;
                char* base;
                char* end;
                char* members;
                base = s_renderContext;
                cursor = base;
                end = base + 0x20A0;
                do { SaveData_InitPlayerRecord(cursor); cursor += 0x828; } while (cursor < end);
                ((IntVectorArray*)(end))->__ct();
                members = base + 0x20D0;
                __construct_array(members, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4);
                AsCEnemyRenderTable(members)->count = 0;
                InitializeNewRenderContext(base);
                __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv, s_renderContextDtorRegistration_802283AC);
                s_renderContextInitialized = 1;
            }
            ((unsigned int&)Sing_805DC648()) = (unsigned int)s_renderContext;
        }
        *(int*)((char*)(unsigned int)((unsigned int&)Sing_805DC648()) + 0x2360) = 2;
        {
            // Construct the owned match character-selection menu.
            struct CharaSelect { char storage[0x60]; CharaSelect() { CCharaSelect_Ctor(this); } };
            void* p = new CharaSelect;
            AsCFightMain(self)->selectionMenu = p;
            AsCFightMain(self)->selectionState = 2;
            AsCSharedDoc(g_sharedDoc)->golfRuleType = 1;
            strcpy(AsCSharedDoc(g_sharedDoc)->pad_8C1, *(char**)&AsCSharedDoc(g_sharedDoc)->matchPlayName);
        }
        break;
    }
    }
}
#pragma opt_lifetimes reset
#pragma opt_propagation reset

// Return the borrowed versus-main type descriptor.
void* CFightMain_GetRTTI() { return &s_fightMainTypeDescriptor; }

// Initialize this module's cached full-turn angle.
void FightMain_InitTwoPiCache() { s_fightMainTwoPi = kFloat_6_2831855; }

// Adjust the task-main interface pointer back to the versus main owner for destruction.
void* CFightMain_dtor_ThunkMinus24(void* p, int deleting) { return ((void* (*)(void*, int))(void*)CFightMain_dtor)((char*)p - 0x18, deleting); }

// Construct the versus task interfaces and reset the actor skip masks.
void* CFightTask_Ctor(void* p) {
    CTask_Ctor(p);
    void** v = (void**)p;
    void* t = __vt__10CFightTask;
    v[0] = t;
    v[1] = (char*)t + kFightTaskSecondaryVtable;
    v[2] = (char*)t + kFightTaskTertiaryVtable;
    g_defaultActorSkipMask = 0;
    g_actorSkipMask = 0;
    return p;
}

// Destroy the versus task base and optionally free its allocation.
void* CFightTask_dtor(void* p, int deletingFlag) {
    if (p) {
        CTask_dtor(p, 0);
        if (deletingFlag > 0)
            __dl__FPv(p);
    }
    return p;
}

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Create the named versus main actor and store it on the task.
void CFightTask_CreateMainActor(void* self) {
    unsigned char flag;
    unsigned int pad;
    unsigned int object[3] = {};
    const char* str = s_fightMainActorName;
    unsigned int length = strlen(str);
    String_ReserveCapacity(object, length);
    flag = *(unsigned char*)&pad;
    String_ReplaceRange(object, 0, 0, str, str + length, &flag);
    StringUnwindCleanup_u802283AC stringCleanup0(object);
    void* result = TaskActorTable_CreateNamedActor(self, str + 1, str + 12, 0, object);
    CTask_SetMainActor(self, result);
    if (object[0] >> 31)
        { stringCleanup0.release(); appFreeUni__FPv((void*)object[2]); }
}
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset

// Forward actor initialization to the base task.
void CFightTask_InitActors(void* forwarded0) { CTask_InitActors(forwarded0); }

// Forward actor preload initialization to the base task.
void CFightTask_PreLoadInitActors(void* forwarded0) { CTask_PreLoadInitActors(forwarded0); }

// Forward actor processing and its unchanged time argument to the base task.
void CFightTask_ProcessActors(void* forwarded0, float forwarded1) { CTask_ProcessActors(forwarded0, forwarded1); }

// Forward drawing in actor-priority order to the base task.
void CFightTask_DisplayActorsByPriority(void* forwarded0) { CTask_DisplayActorsByPriority(forwarded0); }

// Forward actor destruction notifications to the base task.
void CFightTask_NotifyActorsDestroy(void* forwarded0) { CTask_NotifyActorsDestroy(forwarded0); }

// Return the borrowed versus-task type descriptor.
void* CFightTask_GetRTTI() { return &s_fightTaskTypeDescriptor; }

// Initialize this module's cached full-turn angle.
void FightTask_InitTwoPiCache() { s_fightTaskTwoPi = kFloat_6_2831855; }

// Adjust the secondary task interface back to its owner for destruction.
void* CFightTask_vfunc_1_ThunkMinus8(void* p, int deleting) { return CFightTask_dtor((char*)p - 8, deleting); }

// Advance to the next AI action step by reporting success.
int ComThinkAction_NextStep() { return 1; }

// Apply the AI power curve right command to the shot request.
int ComThinkAction_PowerCurveRight(void*, unsigned int* p) { ((float*)p)[1] = kComPowerCurveRight; *p |= kComCurveRight; return 1; }

// Apply the AI power curve left command to the shot request.
int ComThinkAction_PowerCurveLeft(void*, unsigned int* p) { ((float*)p)[1] = kComPowerCurveLeft; *p |= kComCurveLeft; return 1; }

// Apply the AI power top spin command to the shot request.
int ComThinkAction_PowerTopSpin(void*, unsigned int* p) {
    *p |= 1;
    void* mgr = (void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    unsigned char index = AsCGolfDoc(mgr)->currentPlayer;
    void* item = GolfPlayer_GetEntryByIndex(mgr, index);
    int value = ((ComSpinStatView*)item)->spin;
    ((float*)p)[2] = -((float)value) / kComThinkSpinCurveStatDivisor;
    return 1;
}

// Apply the AI power back spin command to the shot request.
int ComThinkAction_PowerBackSpin(void*, unsigned int* p) {
    *p |= 2;
    void* mgr = (void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    unsigned char index = AsCGolfDoc(mgr)->currentPlayer;
    void* item = GolfPlayer_GetEntryByIndex(mgr, index);
    int value = ((ComSpinStatView*)item)->spin;
    ((float*)p)[2] = ((float)value) / kComThinkSpinCurveStatDivisor;
    return 1;
}

// Apply the AI tomahawk command to the shot request.
int ComThinkAction_Tomahawk(void*, unsigned int* p) { *p |= kComTomahawk; ((unsigned char*)p)[0xC] = 1; return 1; }

// Apply the AI cobra command to the shot request.
int ComThinkAction_Cobra(void*, unsigned int* p) { *p |= kComCobra; ((unsigned char*)p)[0xC] = 1; return 1; }

// Default AI normal callback; return the fixed script result.
int ComThinkAction_NormalStub() { return 1; }

// Apply the AI up command to the shot request.
int ComThinkAction_Up(void*, float* p) { p[2] = kFloat_neg1_PadSwing_805E01A0; return 1; }

// Apply the AI down command to the shot request.
int ComThinkAction_Down(void*, float* p) { p[2] = kFloat_1_EffectParameters; return 1; }

// Set positive horizontal curve from the current player's curve stat.
int ComThinkAction_Right(void* action, float* shotRequest) {
	ComSharedStateView* state = (ComSharedStateView*)g_sharedDoc;
	ComCurveStatView* data = (ComCurveStatView*)GolfPlayer_GetEntryByIndex(state->manager, state->manager->index);
	float curveStat = (float)(int)data->curve;
	float value = curveStat / kComThinkSpinCurveStatDivisor;
	shotRequest[1] = value;
	return 1;
}

// Set negative horizontal curve from the current player's curve stat.
int ComThinkAction_Left(void* action, float* shotRequest) {
	ComSharedStateView* state = (ComSharedStateView*)g_sharedDoc;
	ComCurveStatView* data = (ComCurveStatView*)GolfPlayer_GetEntryByIndex(state->manager, state->manager->index);
	float curveStat = (float)(int)data->curve;
	float value = -curveStat / kComThinkSpinCurveStatDivisor;
	shotRequest[1] = value;
	return 1;
}

// Set the AI shot direction to up-right.
int ComThinkAction_UpRight(void* action, float* shotRequest) {
	float horizontal;
	float vertical;
	vertical = kFloat_neg1_PadSwing_805E01A0;
	horizontal = kFloat_1_EffectParameters;
	shotRequest[2] = vertical;
	shotRequest[1] = horizontal;
	return 1;
}

// Apply the AI up left command to the shot request.
int ComThinkAction_UpLeft(void* action, float* shotRequest) {
	float value = kFloat_neg1_PadSwing_805E01A0;
	shotRequest[2] = value;
	shotRequest[1] = value;
	return 1;
}

// Apply the AI down right command to the shot request.
int ComThinkAction_DownRight(void* action, float* shotRequest) {
	float value = kFloat_1_EffectParameters;
	shotRequest[2] = value;
	shotRequest[1] = value;
	return 1;
}

// Set the AI shot direction to down-left.
int ComThinkAction_DownLeft(void* action, float* shotRequest) {
	float horizontal;
	float vertical;
	vertical = kFloat_1_EffectParameters;
	horizontal = kFloat_neg1_PadSwing_805E01A0;
	shotRequest[2] = vertical;
	shotRequest[1] = horizontal;
	return 1;
}

// Default AI area over callback; return the fixed script result.
int ComThinkAction_AreaOverStub() {
	return 1;
}

// Apply the AI power shot command to the shot request.
int ComThinkAction_PowerShot(void* action, unsigned char* shotRequest) {
	shotRequest[12] = 1;
	return 1;
}

// Apply the AI spike command to the shot request.
int ComThinkAction_Spike(void* action, unsigned int* shotRequest) {
	unsigned int val = shotRequest[0];
	((unsigned char*)shotRequest)[12] = 1;
	shotRequest[0] = val | kComSpike;
	return 1;
}

// Apply the AI double power command to the shot request.
int ComThinkAction_DoublePower(void* action, unsigned char* shotRequest) {
	shotRequest[12] = 2;
	return 1;
}

// Apply the AI ignore difficulty command to the shot request.
int ComThinkAction_IgnoreDifficulty(void* action, unsigned char* shotRequest) {
	shotRequest[13] = 0;
	return 1;
}

// Apply the AI set club command to the shot request.
int ComThinkAction_SetClub(ComClubArgument* action, ComClubSelection* shotRequest) {
	shotRequest->clubIndex = action->clubIndex;
	return 1;
}

// Initialize this module's cached full-turn angle.
void ComThinkAction_InitTwoPiCache() {
	s_comThinkActionTwoPi = kFloat_6_2831855;
}

// Optional AI diagnostic hook; this build discards its arguments.
void ComThink_DebugPrintStub(const char* format, ...) {
}

// Compare the truncated integer gauge at least the scaled threshold; return 1 to pass or 2 to fail.
int ComThink_CheckGaugeAtLeastScaledThreshold(ComThresholdArgument* arg0) {
	ComSharedStateView* state = (ComSharedStateView*)g_sharedDoc;
	ComPlayerIndexView* manager = state->manager;
	ComGaugeView* rating = (ComGaugeView*)GolfPlayer_GetEntryByIndex(manager, manager->index);
	ComThink_DebugPrintStub(comThinkDebugStrings, (int)rating->gauge);
	state = (ComSharedStateView*)g_sharedDoc;
	manager = state->manager;
	rating = (ComGaugeView*)GolfPlayer_GetEntryByIndex(manager, manager->index);
	double ratingDouble = (float)(int)rating->gauge;
	
	return ratingDouble >= (kComThinkGaugeThresholdScale * (float)arg0->threshold) ? 1 : 2;
}

// Compare the truncated integer gauge equal the scaled threshold; return 1 to pass or 2 to fail.
int ComThink_CheckGaugeEqualScaledThreshold(ComThresholdArgument* arg0) {
	ComSharedStateView* state = (ComSharedStateView*)g_sharedDoc;
	ComPlayerIndexView* manager = state->manager;
	ComGaugeView* rating = (ComGaugeView*)GolfPlayer_GetEntryByIndex(manager, manager->index);
	ComThink_DebugPrintStub(comThinkDebugStrings, (int)rating->gauge);
	state = (ComSharedStateView*)g_sharedDoc;
	manager = state->manager;
	rating = (ComGaugeView*)GolfPlayer_GetEntryByIndex(manager, manager->index);
	float scaled = kComThinkGaugeThresholdScale * (float)arg0->threshold;
	float ratingFloat = (float)(int)rating->gauge;
	return scaled == ratingFloat ? 1 : 2;
}

// Compare the truncated integer gauge at most the scaled threshold; return 1 to pass or 2 to fail.
int ComThink_CheckGaugeAtMostScaledThreshold(ComThresholdArgument* arg0) {
	ComSharedStateView* state = (ComSharedStateView*)g_sharedDoc;
	ComPlayerIndexView* manager = state->manager;
	ComGaugeView* rating = (ComGaugeView*)GolfPlayer_GetEntryByIndex(manager, manager->index);
	ComThink_DebugPrintStub(comThinkDebugStrings, (int)rating->gauge);
	state = (ComSharedStateView*)g_sharedDoc;
	manager = state->manager;
	rating = (ComGaugeView*)GolfPlayer_GetEntryByIndex(manager, manager->index);
	double ratingDouble = (float)(int)rating->gauge;
	
	return ratingDouble <= (kComThinkGaugeThresholdScale * (float)arg0->threshold) ? 1 : 2;
}

#pragma opt_common_subs off
// Evaluate the AI check level at least condition; 1 passes, 2 fails and 3 remains pending.
int ComThink_CheckLevelAtLeast(ComThresholdArgument* arg0) {
	{
		ComHolePlayerView* manager;
		ComSharedStateView* state;
		int first;
		state = (ComSharedStateView*)g_sharedDoc;
		manager = (ComHolePlayerView*)state->manager;
		int index = manager->index;
		first = ((ComLevelLookupView*)state)->recordOffsets[manager->holeIndex];
		index *= kSharedPlayerBlockBytes;
		first += (unsigned int)state;
		ComLevelRecordView* record = (ComLevelRecordView*)(first + index);
		signed char value = record->level;
		ComThink_DebugPrintStub(ComDebug->level, value);
	}
	ComHolePlayerView* manager;
	ComSharedStateView* state;
	int first;
	int index;
	ComLevelRecordView* record;
	state = (ComSharedStateView*)g_sharedDoc;
	manager = (ComHolePlayerView*)state->manager;
	index = manager->index;
	first = ((ComLevelLookupView*)state)->recordOffsets[manager->holeIndex];
	index *= kSharedPlayerBlockBytes;
	first += (unsigned int)state;
	record = (ComLevelRecordView*)(first + index);
	signed char value = record->level;
	int result = kComConditionFail;
	if (value >= arg0->threshold) result = kComConditionPass;
	return result;
}

// Evaluate the AI check level at most condition; 1 passes, 2 fails and 3 remains pending.
int ComThink_CheckLevelAtMost(ComThresholdArgument* arg0) {
	{
	ComHolePlayerView* manager;
	ComSharedStateView* state;
	int first;
	state = (ComSharedStateView*)g_sharedDoc;
	manager = (ComHolePlayerView*)state->manager;
	int index = manager->index;
	first = ((ComLevelLookupView*)state)->recordOffsets[manager->holeIndex];
	index *= kSharedPlayerBlockBytes;
	first += (unsigned int)state;
	ComLevelRecordView* record = (ComLevelRecordView*)(first + index);
	signed char value = record->level;
	ComThink_DebugPrintStub(ComDebug->level, value);
	}
	ComHolePlayerView* manager;
	ComSharedStateView* state;
	int first;
	int index;
	ComLevelRecordView* record;
	state = (ComSharedStateView*)g_sharedDoc;
	manager = (ComHolePlayerView*)state->manager;
	index = manager->index;
	first = ((ComLevelLookupView*)state)->recordOffsets[manager->holeIndex];
	index *= kSharedPlayerBlockBytes;
	first += (unsigned int)state;
	record = (ComLevelRecordView*)(first + index);
	signed char value2 = record->level;
	int result = kComConditionFail;
	if (value2 <= arg0->threshold) result = kComConditionPass;
	return result;
}

#pragma opt_common_subs reset
// Evaluate the AI check level equal condition; 1 passes, 2 fails and 3 remains pending.
int ComThink_CheckLevelEqual(ComThresholdArgument* arg0) { int first2; 
	{
		ComHolePlayerView* manager;
		ComSharedStateView* state;
		int first;
		state = (ComSharedStateView*)g_sharedDoc;
		manager = (ComHolePlayerView*)state->manager;
		int index = manager->index;
		first = ((ComLevelLookupView*)state)->recordOffsets[manager->holeIndex];
		index *= kSharedPlayerBlockBytes;
		first += (unsigned int)state;
		ComLevelRecordView* record = (ComLevelRecordView*)(first + index);
		signed char value = record->level;
		ComThink_DebugPrintStub(ComDebug->level, value);
	}
	int result = kComConditionFail;
	int target;
	ComHolePlayerView* manager2;
	ComSharedStateView* state2;
	target = arg0->threshold;
	
	state2 = (ComSharedStateView*)g_sharedDoc;
	manager2 = (ComHolePlayerView*)state2->manager;
	int index2 = manager2->index;
	 first2 = ((ComLevelLookupView*)state2)->recordOffsets[manager2->holeIndex];
	index2 *= 0x1D4;
	first2 += (unsigned int)state2;
	ComLevelRecordView* record2 = (ComLevelRecordView*)(first2 + index2);
    if (target == record2->level) result = kComConditionPass;
	return result;
}

// Default AI check com win callback; return the fixed script result.
int ComThink_CheckComWinStub() {
	return kComConditionFail;
}

// Default AI check com lose callback; return the fixed script result.
int ComThink_CheckComLoseStub() {
	return kComConditionPass;
}

// Evaluate the AI roll gauge threshold condition; 1 passes, 2 fails and 3 remains pending.
int ComThink_RollGaugeThreshold(ComComparisonArgument* arg0) {
	unsigned int value = rand();
	ComThink_DebugPrintStub(ComDebug->randomValue, value);
	unsigned int remainder = value - (value / 100) * 100;
	int result = kComConditionFail;
	if (remainder > (unsigned int)arg0->threshold) result = kComConditionPass;
	return result;
}

// Evaluate the AI check always condition; 1 passes, 2 fails and 3 remains pending.
int ComThink_CheckAlways() {
	return kComConditionPass;
}

// Evaluate the AI check tee shot condition; 1 passes, 2 fails and 3 remains pending.
int ComThink_CheckTeeShot() {
	ComSharedStateView* state = (ComSharedStateView*)g_sharedDoc;
	unsigned char* data = (unsigned char*)GolfPlayer_GetEntryByIndex(state->manager, state->manager->index);
	ComHolePlayerView* manager = (ComHolePlayerView*)((ComSharedStateView*)g_sharedDoc)->manager;
	unsigned char index = manager->holeIndex;
	unsigned char* ptr = data + index;
	int value = ptr[0x28];
	ComThink_DebugPrintStub(ComDebug->shotCount, value);
	int result = kComConditionFail;
	if (value == 1) result = kComConditionPass;
	return result;
}

// Default AI check nothing callback; return the fixed script result.
int ComThink_CheckNothingStub() {
	return kComConditionPending;
}

#pragma opt_lifetimes off
#pragma opt_strength_reduction off
#pragma opt_propagation off
#pragma opt_common_subs off
// Evaluate the AI check last o b presence condition; 1 passes, 2 fails and 3 remains pending.
int ComThink_CheckLastOBPresence(ComThresholdArgument* arg0) {
	ComSharedStateView* state = (ComSharedStateView*)g_sharedDoc;
	ComPlayerIndexView* manager = state->manager;
	int value = ((ComLastOBView*)GolfPlayer_GetEntryByIndex(manager, manager->index))->lastOB;
	ComThink_DebugPrintStub(ComDebug->lastOB, value);
	int mode = arg0->threshold;
	int result;
	switch (mode) {
		case 1:
			result = kComConditionFail;
			if (value >= 1) result = kComConditionPass;
			break;
		case 0:
			result = kComConditionFail;
			if (value == 0) result = kComConditionPass;
			break;
		default:
			result = kComConditionFail;
			break;
	}
	return result;
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_strength_reduction reset
#pragma opt_lifetimes reset

// Default AI check boost callback; return the fixed script result.
int ComThink_CheckBoostStub() {
	return kComConditionPending;
}

// Evaluate the AI check power at least condition; 1 passes, 2 fails and 3 remains pending.
int ComThink_CheckPowerAtLeast(ComComparisonArgument* arg0) {
	ComSharedStateView* state = (ComSharedStateView*)g_sharedDoc;
	ComPowerStatView* data = (ComPowerStatView*)GolfPlayer_GetEntryByIndex(state->manager, state->manager->index);
	ComThink_DebugPrintStub(ComDebug->power, data->power);
	state = (ComSharedStateView*)g_sharedDoc;
	data = (ComPowerStatView*)GolfPlayer_GetEntryByIndex(state->manager, state->manager->index);
	int result = kComConditionFail;
	if (data->power >= arg0->threshold) result = kComConditionPass;
	return result;
}

// Evaluate the AI check power at most condition; 1 passes, 2 fails and 3 remains pending.
int ComThink_CheckPowerAtMost(ComComparisonArgument* arg0) {
	ComSharedStateView* state = (ComSharedStateView*)g_sharedDoc;
	ComPowerStatView* data = (ComPowerStatView*)GolfPlayer_GetEntryByIndex(state->manager, state->manager->index);
	ComThink_DebugPrintStub(ComDebug->power, data->power);
	state = (ComSharedStateView*)g_sharedDoc;
	data = (ComPowerStatView*)GolfPlayer_GetEntryByIndex(state->manager, state->manager->index);
	int result = kComConditionFail;
	if (data->power <= arg0->threshold) result = kComConditionPass;
	return result;
}

#pragma use_lmw_stmw on

// Default AI check carry callback; return the fixed script result.
int ComThink_CheckCarryStub() {
    return kComConditionPending;
}

// Default AI check area stop callback; return the fixed script result.
int ComThink_CheckAreaStopStub() {
    return kComConditionPending;
}

// Evaluate the AI check shot count equal condition; 1 passes, 2 fails and 3 remains pending.
int ComThink_CheckShotCountEqual(void* arg) {
    void* shared = g_sharedDoc;
    void* golfDoc = *(void**)&AsCSharedDoc(shared)->golfDoc;
    unsigned char playerIndex = AsCGolfDoc(golfDoc)->currentPlayer;
    void* playerEntry = GolfPlayer_GetEntryByIndex(golfDoc, playerIndex);
    void* sharedAgain = g_sharedDoc;
    void* golfDocAgain = *(void**)&AsCSharedDoc(sharedAgain)->golfDoc;
    unsigned char holeIndex = AsCGolfDoc(golfDocAgain)->currentHoleIndex;
    void* holeCounter = (char*)playerEntry + holeIndex;
    int shotCount = ((ComShotCountView*)holeCounter)->shotCount;
    ComThink_DebugPrintStub(ComDebug->shotCount, shotCount);
    int expectedCount = (unsigned char)((ComConditionArgumentView*)arg)->threshold;
    int result = kComConditionFail;
    if (shotCount == expectedCount) result = kComConditionPass;
    return result;
}

// Evaluate the AI check shot count at most condition; 1 passes, 2 fails and 3 remains pending.
int ComThink_CheckShotCountAtMost(void* arg) {
    void* shared = g_sharedDoc;
    void* golfDoc = *(void**)&AsCSharedDoc(shared)->golfDoc;
    unsigned char playerIndex = AsCGolfDoc(golfDoc)->currentPlayer;
    void* playerEntry = GolfPlayer_GetEntryByIndex(golfDoc, playerIndex);
    void* sharedAgain = g_sharedDoc;
    void* golfDocAgain = *(void**)&AsCSharedDoc(sharedAgain)->golfDoc;
    unsigned char holeIndex = AsCGolfDoc(golfDocAgain)->currentHoleIndex;
    void* holeCounter = (char*)playerEntry + holeIndex;
    int shotCount = ((ComShotCountView*)holeCounter)->shotCount;
    ComThink_DebugPrintStub(ComDebug->shotCount, shotCount);
    int expectedCount = (unsigned char)((ComConditionArgumentView*)arg)->threshold;
    int result = kComConditionFail;
    if (shotCount <= expectedCount) result = kComConditionPass;
    return result;
}

// Evaluate the AI check shot count at least condition; 1 passes, 2 fails and 3 remains pending.
int ComThink_CheckShotCountAtLeast(void* arg) {
    void* shared = g_sharedDoc;
    void* golfDoc = *(void**)&AsCSharedDoc(shared)->golfDoc;
    unsigned char playerIndex = AsCGolfDoc(golfDoc)->currentPlayer;
    void* playerEntry = GolfPlayer_GetEntryByIndex(golfDoc, playerIndex);
    void* sharedAgain = g_sharedDoc;
    void* golfDocAgain = *(void**)&AsCSharedDoc(sharedAgain)->golfDoc;
    unsigned char holeIndex = AsCGolfDoc(golfDocAgain)->currentHoleIndex;
    void* holeCounter = (char*)playerEntry + holeIndex;
    int shotCount = ((ComShotCountView*)holeCounter)->shotCount;
    ComThink_DebugPrintStub(ComDebug->shotCount, shotCount);
    int expectedCount = (unsigned char)((ComConditionArgumentView*)arg)->threshold;
    int result = kComConditionFail;
    if (shotCount >= expectedCount) result = kComConditionPass;
    return result;
}

// Evaluate the AI check cup distance at least condition; 1 passes, 2 fails and 3 remains pending.
int ComThink_CheckCupDistanceAtLeast(void* arg) {
    void* g = g_sharedDoc;
    void* p = *(void**)&AsCSharedDoc(g)->golfDoc;
    unsigned char b = AsCGolfDoc(p)->currentPlayer;
    void* r = GolfPlayer_GetEntryByIndex(p, b);
    void* g2 = g_sharedDoc;
    void* p2 = *(void**)&AsCSharedDoc(g2)->golfDoc;
    unsigned char c = AsCGolfDoc(p2)->currentHoleIndex;
    void* base = AsCGolfDoc(p2)->holeRecords;
    void* r7 = (char*)base + ((unsigned int)c << 5);
    float arr[6];
    struct Arr3 { float v[3]; };
    *(Arr3*)&arr[0] = *(Arr3*)((ComHoleCupView*)r7)->cupPosition;
    __mi__FRC7WVectorRC7WVector(&arr[3], &arr[0], ((ComBallPositionView*)r)->position);
    arr[4] = kFloat_0_ComThinkCupDistance;
    float len = WVectorLen(*(WVector*)(&arr[3]));
    float ratio = len / kComThinkCupDistanceDivisor;
    int threshold = ((ComConditionArgumentView*)arg)->threshold;
    float d = threshold;
    if (ratio >= d) return kComConditionPass;
    return kComConditionFail;
}

// Evaluate the AI check cup distance at most condition; 1 passes, 2 fails and 3 remains pending.
int ComThink_CheckCupDistanceAtMost(void* arg) {
    void* g = g_sharedDoc;
    void* p = *(void**)&AsCSharedDoc(g)->golfDoc;
    unsigned char b = AsCGolfDoc(p)->currentPlayer;
    void* r = GolfPlayer_GetEntryByIndex(p, b);
    void* g2 = g_sharedDoc;
    void* p2 = *(void**)&AsCSharedDoc(g2)->golfDoc;
    unsigned char c = AsCGolfDoc(p2)->currentHoleIndex;
    void* base = AsCGolfDoc(p2)->holeRecords;
    void* r7 = (char*)base + ((unsigned int)c << 5);
    float arr[6];
    struct Arr3 { float v[3]; };
    *(Arr3*)&arr[0] = *(Arr3*)((ComHoleCupView*)r7)->cupPosition;
    __mi__FRC7WVectorRC7WVector(&arr[3], &arr[0], ((ComBallPositionView*)r)->position);
    arr[4] = kFloat_0_ComThinkCupDistance;
    float len = WVectorLen(*(WVector*)(&arr[3]));
    float ratio = len / kComThinkCupDistanceDivisor;
    int threshold = ((ComConditionArgumentView*)arg)->threshold;
    float d = threshold;
    if (ratio <= d) return kComConditionPass;
    return kComConditionFail;
}

// Resolve and cache the pending AI boost check, retaining an already resolved result.
int ComThink_CheckAndCacheBoost(void* arg) {
    int diagnosticPassed = CComThinkCtrl_GetSpecialLandingResult(arg);
    char* diagnosticText =ComDebug->failed;
    if (diagnosticPassed != 0) diagnosticText =ComDebug->passed;
    ComThink_DebugPrintStub(ComDebug->afterBoost, diagnosticText);
    int conditionPassed = CComThinkCtrl_GetSpecialLandingResult(arg);
    void* commandState = CComThinkCtrl_GetCommandState(arg);
    int* cachedResult = &((ComCachedChecksView*)commandState)->boost;
    if (*cachedResult == kComConditionPending) {
        int resolvedResult = kComConditionFail;
        if (conditionPassed != 0) resolvedResult = kComConditionPass;
        *cachedResult = resolvedResult;
    }
    return *cachedResult;
}

// Resolve and cache the pending AI carry check, retaining an already resolved result.
int ComThink_CheckAndCacheCarry(void* arg) {
    int diagnosticPassed = CComThinkCtrl_GetFirstLandingWithinTargetResult(arg);
    char* diagnosticText =ComDebug->failed;
    if (diagnosticPassed != 0) diagnosticText =ComDebug->passed;
    ComThink_DebugPrintStub(ComDebug->afterCarry, diagnosticText);
    int conditionPassed = CComThinkCtrl_GetFirstLandingWithinTargetResult(arg);
    void* commandState = CComThinkCtrl_GetCommandState(arg);
    int* cachedResult = &((ComCachedChecksView*)commandState)->carry;
    if (*cachedResult == kComConditionPending) {
        int resolvedResult = kComConditionFail;
        if (conditionPassed != 0) resolvedResult = kComConditionPass;
        *cachedResult = resolvedResult;
    }
    return *cachedResult;
}

// Resolve and cache the pending AI area-stop check, retaining an already resolved result.
int ComThink_CheckAndCacheAreaStop(void* arg) {
    int diagnosticPassed = CComThinkCtrl_GetFinalWithinTargetResult(arg);
    char* diagnosticText =ComDebug->failed;
    if (diagnosticPassed != 0) diagnosticText =ComDebug->passed;
    ComThink_DebugPrintStub(ComDebug->afterAreaStop, diagnosticText);
    int conditionPassed = CComThinkCtrl_GetFinalWithinTargetResult(arg);
    void* commandState = CComThinkCtrl_GetCommandState(arg);
    int* cachedResult = &((ComCachedChecksView*)commandState)->areaStop;
    if (*cachedResult == kComConditionPending) {
        int resolvedResult = kComConditionFail;
        if (conditionPassed != 0) resolvedResult = kComConditionPass;
        *cachedResult = resolvedResult;
    }
    return *cachedResult;
}

// Initialize this module's cached full-turn angle.
void ComThinkCondition_InitTwoPiCache() {
    s_comThinkConditionTwoPi = kFloat_6_2831855;
}

// Optional AI command diagnostic hook; this build discards its arguments.
void ComThink_CommandDebugPrintStub(int arg, ...) {
}

void* ComConditionTree_dtor(ThinkRuleTree*, int);
void* ThinkActionTree_dtor(ThinkRuleTree*, int);
// Initialize the condition/action trees and register every named AI command callback.
void* CComThinkCtrl_Ctor(void* arg) {
    char* self = (char*)arg;
    AsCComThinkCtrl(self)->vtbl = __vt__13CComThinkCtrl;
    void (*p1)(ComRuleTreeHeader*) = ComConditionTree_Init;
    p1((ComRuleTreeHeader*)(self + 0x4));
    EH5Cleanup eh5Cleanup17(self + 0x4, (void (*)(void*, int))ComConditionTree_dtor);
    void (*p2)(ComRuleTreeHeader*) = ComActionTree_Init;
    p2((ComRuleTreeHeader*)(self + 0x14));
    EH5Cleanup eh5Cleanup18(self + 0x14, (void (*)(void*, int))ThinkActionTree_dtor);
    AsCComThinkCtrl(self)->unk_24 = 0; // Command file is not ready yet.
    AsCComThinkCtrl(self)->unk_28 = 0; // No loaded command buffer yet.
    AsCComThinkCtrl(self)->sharedFile = 0;
    char* commandNames = comThinkCommandStrings;
    CComThinkCtrl_RegisterCondition(self, 1, (void*)ComThink_CheckGaugeAtLeastScaledThreshold, (commandNames + (int)&((ComCommandStringPool*)0)->IS_GAUGE_OVER));
    CComThinkCtrl_RegisterCondition(self, 2, (void*)ComThink_CheckGaugeEqualScaledThreshold, (commandNames + (int)&((ComCommandStringPool*)0)->IS_GAUGE_UNDER));
    CComThinkCtrl_RegisterCondition(self, 3, (void*)ComThink_CheckGaugeAtMostScaledThreshold, (commandNames + (int)&((ComCommandStringPool*)0)->IS_GAUGE_SAME));
    CComThinkCtrl_RegisterCondition(self, 4, (void*)ComThink_CheckLevelAtLeast, (commandNames + (int)&((ComCommandStringPool*)0)->IS_COM_LEVEL_OVER));
    CComThinkCtrl_RegisterCondition(self, 5, (void*)ComThink_CheckLevelAtMost, (commandNames + (int)&((ComCommandStringPool*)0)->IS_COM_LEVEL_UNDER));
    CComThinkCtrl_RegisterCondition(self, 6, (void*)ComThink_CheckLevelEqual, (commandNames + (int)&((ComCommandStringPool*)0)->IS_COM_LEVEL_SAME));
    CComThinkCtrl_RegisterCondition(self, 7, (void*)ComThink_CheckComWinStub, (commandNames + (int)&((ComCommandStringPool*)0)->IS_COM_WIN));
    CComThinkCtrl_RegisterCondition(self, 8, (void*)ComThink_CheckComLoseStub, (commandNames + (int)&((ComCommandStringPool*)0)->IS_COM_LOSE));
    CComThinkCtrl_RegisterCondition(self, 9, (void*)ComThink_RollGaugeThreshold, (commandNames + (int)&((ComCommandStringPool*)0)->IS_RANDOM));
    CComThinkCtrl_RegisterCondition(self, 10, (void*)ComThink_CheckAlways, (commandNames + (int)&((ComCommandStringPool*)0)->IS_ALWAYS));
    CComThinkCtrl_RegisterCondition(self, 11, (void*)ComThink_CheckNothingStub, (commandNames + (int)&((ComCommandStringPool*)0)->IS_NOTHING));
    CComThinkCtrl_RegisterCondition(self, 12, (void*)ComThink_CheckTeeShot, (commandNames + (int)&((ComCommandStringPool*)0)->IS_TEESHOT));
    CComThinkCtrl_RegisterCondition(self, 13, (void*)ComThink_CheckLastOBPresence, (commandNames + (int)&((ComCommandStringPool*)0)->IS_OB));
    CComThinkCtrl_RegisterCondition(self, 14, (void*)ComThink_CheckBoostStub, (commandNames + (int)&((ComCommandStringPool*)0)->IS_BOOST));
    CComThinkCtrl_RegisterCondition(self, 15, (void*)ComThink_CheckPowerAtLeast, (commandNames + (int)&((ComCommandStringPool*)0)->IS_PAR_POWER));
    CComThinkCtrl_RegisterCondition(self, 16, (void*)ComThink_CheckCarryStub, (commandNames + (int)&((ComCommandStringPool*)0)->IS_CARRY));
    CComThinkCtrl_RegisterCondition(self, 17, (void*)ComThink_CheckAreaStopStub, (commandNames + (int)&((ComCommandStringPool*)0)->IS_AREA_STOP));
    CComThinkCtrl_RegisterCondition(self, 18, (void*)ComThink_CheckShotCountEqual, (commandNames + (int)&((ComCommandStringPool*)0)->IS_SHOT_NUM_SAME));
    CComThinkCtrl_RegisterCondition(self, 19, (void*)ComThink_CheckShotCountAtLeast, (commandNames + (int)&((ComCommandStringPool*)0)->IS_SHOT_NUM_OVER));
    CComThinkCtrl_RegisterCondition(self, 20, (void*)ComThink_CheckShotCountAtMost, (commandNames + (int)&((ComCommandStringPool*)0)->IS_SHOT_NUM_UNDER));
    CComThinkCtrl_RegisterCondition(self, 21, (void*)ComThink_CheckCupDistanceAtLeast, (commandNames + (int)&((ComCommandStringPool*)0)->IS_CUP_LEN_OVER));
    CComThinkCtrl_RegisterCondition(self, 22, (void*)ComThink_CheckCupDistanceAtMost, (commandNames + (int)&((ComCommandStringPool*)0)->IS_CUP_LEN_UNDER));
    CComThinkCtrl_RegisterCondition(self, 23, (void*)ComThink_CheckPowerAtMost, (commandNames + (int)&((ComCommandStringPool*)0)->IS_UNDER_POWER));

    CComThinkCtrl_RegisterAction(self, 1, (void*)ComThinkAction_NextStep, (commandNames + (int)&((ComCommandStringPool*)0)->DO_NEXT_STEP));
    CComThinkCtrl_RegisterAction(self, 2, (void*)ComThinkAction_PowerCurveRight, (commandNames + (int)&((ComCommandStringPool*)0)->DO_POWER_CURVE_RIGHT));
    CComThinkCtrl_RegisterAction(self, 3, (void*)ComThinkAction_PowerCurveLeft, (commandNames + (int)&((ComCommandStringPool*)0)->DO_POWER_CURVE_LEFT));
    CComThinkCtrl_RegisterAction(self, 4, (void*)ComThinkAction_PowerTopSpin, (commandNames + (int)&((ComCommandStringPool*)0)->DO_POWER_TOP_SPIN));
    CComThinkCtrl_RegisterAction(self, 5, (void*)ComThinkAction_PowerBackSpin, (commandNames + (int)&((ComCommandStringPool*)0)->DO_POWER_BACK_SPIN));
    CComThinkCtrl_RegisterAction(self, 6, (void*)ComThinkAction_PowerShot, (commandNames + (int)&((ComCommandStringPool*)0)->DO_POWER_SHOT));
    CComThinkCtrl_RegisterAction(self, 7, (void*)ComThinkAction_Tomahawk, (commandNames + (int)&((ComCommandStringPool*)0)->DO_TOMAHAWK));
    CComThinkCtrl_RegisterAction(self, 8, (void*)ComThinkAction_Cobra, (commandNames + (int)&((ComCommandStringPool*)0)->DO_COBRA));
    CComThinkCtrl_RegisterAction(self, 9, (void*)ComThinkAction_NormalStub, (commandNames + (int)&((ComCommandStringPool*)0)->DO_NORMAL));
    CComThinkCtrl_RegisterAction(self, 10, (void*)ComThinkAction_Up, (commandNames + (int)&((ComCommandStringPool*)0)->DO_UP));
    CComThinkCtrl_RegisterAction(self, 11, (void*)ComThinkAction_Down, (commandNames + (int)&((ComCommandStringPool*)0)->DO_DOWN));
    CComThinkCtrl_RegisterAction(self, 12, (void*)ComThinkAction_Right, (commandNames + (int)&((ComCommandStringPool*)0)->DO_RIGHT));
    CComThinkCtrl_RegisterAction(self, 13, (void*)ComThinkAction_Left, (commandNames + (int)&((ComCommandStringPool*)0)->DO_LEFT));
    CComThinkCtrl_RegisterAction(self, 14, (void*)ComThinkAction_UpRight, (commandNames + (int)&((ComCommandStringPool*)0)->DO_UP_RIGHT));
    CComThinkCtrl_RegisterAction(self, 15, (void*)ComThinkAction_UpLeft, (commandNames + (int)&((ComCommandStringPool*)0)->DO_UP_LEFT));
    CComThinkCtrl_RegisterAction(self, 16, (void*)ComThinkAction_DownRight, (commandNames + (int)&((ComCommandStringPool*)0)->DO_DOWN_RIGHT));
    CComThinkCtrl_RegisterAction(self, 17, (void*)ComThinkAction_DownLeft, (commandNames + (int)&((ComCommandStringPool*)0)->DO_DOWN_LEFT));
    CComThinkCtrl_RegisterAction(self, 18, (void*)ComThinkAction_AreaOverStub, (commandNames + (int)&((ComCommandStringPool*)0)->DO_AREA_OVER));
    CComThinkCtrl_RegisterAction(self, 19, (void*)ComThinkAction_Spike, (commandNames + (int)&((ComCommandStringPool*)0)->DO_SPIKE));
    CComThinkCtrl_RegisterAction(self, 20, (void*)ComThinkAction_DoublePower, (commandNames + (int)&((ComCommandStringPool*)0)->DO_DOUBLE_POWER));
    CComThinkCtrl_RegisterAction(self, 21, (void*)ComThinkAction_IgnoreDifficulty, (commandNames + (int)&((ComCommandStringPool*)0)->IGNORE_DIFFICULTY));
    CComThinkCtrl_RegisterAction(self, 22, (void*)ComThinkAction_SetClub, (commandNames + (int)&((ComCommandStringPool*)0)->DO_SET_CLUB));
    eh5Cleanup17.release(); eh5Cleanup18.release(); return (void*)self;
}

// Initialize an empty AI condition tree and its sentinel iterator.
void ComConditionTree_Init(ComRuleTreeHeader* arg) {
    arg->first = 0;
    arg->second = 0;
    arg->current = &arg->second;
}

// Release one rule node after both children are gone; only a heap-backed key is owned.
// Preserve expanded subobject guards and the key-before-node destruction order.
static inline void ReleaseOwnedRuleNode(char* node) {
    if ((int)node + 0xc && (int)node + 0x10 && (int)node + 0x10 && ((AsThinkRuleNode(node)->stringFlags) >> 31))
        appFreeUni__FPv(AsThinkRuleNode(node)->heapString);
    appFreeUni__FPv(node);
}

#pragma opt_lifetimes off
// Free the condition tree nodes and optionally delete the tree header.
// Nodes own heap-backed key strings (high bit of stringFlags), not the callback
// targets encoded in their values. Traversal is postorder: left, right, key, node.
// Four explicit levels reproduce compiler inlining; ComConditionMap_FreeSubtree
// handles deeper children. Repeated integer-address guards are expanded subobject
// cleanup, not extra allocations. Positive flags also delete the borrowed header.
void* ComConditionTree_dtor(ThinkRuleTree* self, int flags) {
    char* rootNode;
    char* levelOneNode;
    char* treeBytes = (char*)self;
    if (treeBytes != 0) {
      if (treeBytes != 0 && treeBytes != 0 && treeBytes != 0 && treeBytes != 0 && treeBytes != 0) {
        rootNode = AsThinkRuleTree(treeBytes)->root;
        if (rootNode != 0) {
            levelOneNode = *(char**)&AsThinkRuleNode(rootNode)->left;
            if (levelOneNode != 0) {
                char* levelTwoNode = *(char**)&AsThinkRuleNode(levelOneNode)->left;
                if (levelTwoNode != 0) {
                    char* levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->left;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->right;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    ReleaseOwnedRuleNode(levelTwoNode);
                }
                {
                char* levelTwoNode = *(char**)&AsThinkRuleNode(levelOneNode)->right;
                if (levelTwoNode != 0) {
                    char* levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->left;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->right;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    ReleaseOwnedRuleNode(levelTwoNode);
                }
                }
                ReleaseOwnedRuleNode(levelOneNode);
            }
            {
            char* levelOneNode = *(char**)&AsThinkRuleNode(rootNode)->right;
            if (levelOneNode != 0) {
                char* levelTwoNode = *(char**)&AsThinkRuleNode(levelOneNode)->left;
                if (levelTwoNode != 0) {
                    char* levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->left;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->right;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    ReleaseOwnedRuleNode(levelTwoNode);
                }
                {
                char* levelTwoNode = *(char**)&AsThinkRuleNode(levelOneNode)->right;
                if (levelTwoNode != 0) {
                    char* levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->left;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->right;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) ComConditionMap_FreeSubtree((void*)self, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    ReleaseOwnedRuleNode(levelTwoNode);
                }
                }
                ReleaseOwnedRuleNode(levelOneNode);
            }
            }
            ReleaseOwnedRuleNode(rootNode);
        }
      }
        if (flags > 0) __dl__FPv((void*)self);
    }
    return (void*)self;
}
#pragma opt_lifetimes reset

// Initialize an empty AI action tree and its sentinel iterator.
void ComActionTree_Init(ComRuleTreeHeader* arg) {
    arg->first = 0;
    arg->second = 0;
    arg->current = &arg->second;
}

#pragma opt_lifetimes off
// Free the action tree nodes and optionally delete the tree header.
// Same ownership/postorder contract as ComConditionTree_dtor; Tree_FreeSubtree is the
// action specialization's deep fallback. Each string and node is freed once.
void* ThinkActionTree_dtor(ThinkRuleTree* self, int flags) {
    char* treeBytes = (char*)self;
    if (treeBytes != 0) {
      if (treeBytes != 0 && treeBytes != 0 && treeBytes != 0 && treeBytes != 0 && treeBytes != 0) {
        char* rootNode = AsThinkRuleTree(treeBytes)->root;
        if (rootNode != 0) {
            char* leftBranchNode = *(char**)&AsThinkRuleNode(rootNode)->left;
            if (leftBranchNode != 0) {
                char* levelTwoNode = *(char**)&AsThinkRuleNode(leftBranchNode)->left;
                if (levelTwoNode != 0) {
                    char* levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->left;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->right;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    ReleaseOwnedRuleNode(levelTwoNode);
                }
                {
                char* levelTwoNode = *(char**)&AsThinkRuleNode(leftBranchNode)->right;
                if (levelTwoNode != 0) {
                    char* levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->left;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->right;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    ReleaseOwnedRuleNode(levelTwoNode);
                }
                }
                ReleaseOwnedRuleNode(leftBranchNode);
            }
            char* rightBranchNode = *(char**)&AsThinkRuleNode(rootNode)->right;
            if (rightBranchNode != 0) {
                char* levelTwoNode = *(char**)&AsThinkRuleNode(rightBranchNode)->left;
                if (levelTwoNode != 0) {
                    char* levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->left;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->right;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    ReleaseOwnedRuleNode(levelTwoNode);
                }
                {
                char* levelTwoNode = *(char**)&AsThinkRuleNode(rightBranchNode)->right;
                if (levelTwoNode != 0) {
                    char* levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->left;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    levelThreeNode = *(char**)&AsThinkRuleNode(levelTwoNode)->right;
                    if (levelThreeNode != 0) {
                        if (AsThinkRuleNode(levelThreeNode)->left != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->left);
                        if (AsThinkRuleNode(levelThreeNode)->right != 0) Tree_FreeSubtree(treeBytes, AsThinkRuleNode(levelThreeNode)->right);
                        ReleaseOwnedRuleNode(levelThreeNode);
                    }
                    ReleaseOwnedRuleNode(levelTwoNode);
                }
                }
                ReleaseOwnedRuleNode(rightBranchNode);
            }
            ReleaseOwnedRuleNode(rootNode);
        }
      }
        if (flags > 0) __dl__FPv(treeBytes);
    }
    return self;
}

#pragma opt_lifetimes reset
// Clear both command trees, release the shared file and optionally delete the controller.
void* CComThinkCtrl_dtor(void* self, int flags) {
    char* p = (char*)self;
    if (p != 0) {
        AsCComThinkCtrl(p)->vtbl = __vt__13CComThinkCtrl;
        ComConditionMap_Clear(p + 0x4);
        CComThinkCtrl_ClearStringTree(p + 0x14);
        if (AsCComThinkCtrl(p)->sharedFile != 0) {
            CSharedFileHandle_Destroy(AsCComThinkCtrl(p)->sharedFile, 1);
            AsCComThinkCtrl(p)->sharedFile = 0;
        }
        ThinkActionTree_dtor((ThinkRuleTree*)(p + 0x14), -1);
        ComConditionTree_dtor((ThinkRuleTree*)(p + 0x4), -1);
        if (flags > 0) {
            __dl__FPv(p);
        }
    }
    return self;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046222C", _ctors_u802283AC_0, PadSwing_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU8046222C", _ctors_u802283AC_1, FightMain_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU8046222C", _ctors_u802283AC_2, FightTask_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU8046222C", _ctors_u802283AC_3, ComThinkAction_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU8046222C", _ctors_u802283AC_4, ComThinkCondition_InitTwoPiCache);
#pragma pop
