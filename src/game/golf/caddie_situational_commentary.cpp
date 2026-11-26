// Caddie situational commentary: chooses which comment the caddie requests from
// the tutorial aim, terrain, wind, item, score and record state. The unit also
// holds the small pet-body and effects accessors used by the caddie process.
#include "game/wvector.h"
#include "game/golfball.h"
#include "classes/CPetBody.h"
#include "classes/CSharedDoc.h"
#include "classes/CGolfCaddie.h"
#include "classes/CFx.h"
#include "classes/GolfDocSnapshot8013.h"
#include "classes/CWind.h"
#include "classes/CQuadTree.h"
#include "classes/CGolfDoc.h"

extern "C" {

enum CaddieTiming {
    kCommentDelayMs = 1500,
    kDeadlineWarningMs = 10000,
    kSpecialCaddieDeadlineMs = 30000,
    kWindDeadlineMs = 35000,
    kDeadlineMarginMs = 5000
};
// Ground code 3 is preserved from the condition below. The collision unit labels
// 3 rough and 4 bunker; comment ID 14 alone does not establish terrain semantics.
// Keep the code neutral until that discrepancy is resolved; do not change to 4.
enum CaddieRules {
    kStrokePlay = 0,
    kMatchPlay = 1,
    kGreen = 2,
    kCommentGroundCode3 = 3,
    kDisabledPlayer = 3,
    kPutterClub = 3,
    kParFive = 5,
    kFullRoundHoles = 18,
    kItemActionBlocked = 8,
    kNoCourseRecord = 0x7fff,
    kBaseCaddieTypeId = 0x1c000000,
    kLastAimTutorialStage = 3,
    kLongRangeTutorialStage = 2,
    kCaddieYawAxis = 1,
    kRangeCloseTestFlag = 1,
    kPostHolePlayerState = 2
};
enum CaddieComment {
    kPositiveReaction = 1,
    kNegativeReaction = 2,
    kTutorialAimPositive = 3,
    kTutorialAimNegative = 4,
    kTutorialRangeWarning = 5,
    kTutorialRangeClose = 6,
    kNeedMoreRange = 7,
    kNeedLessRange = 8,
    kDeadlineWarning = 9,
    kCaddieInShotDirection = 10,
    kNoGroundAhead = 11,
    kTimedStrongWind = 12,
    kAimAwayFromHole = 13,
    kGroundCode3Comment = 14,
    kTimedBaseCaddie = 15,
    kFiveHoleResultStreak = 16,
    kParFiveReachable = 17,
    kStrokeLead = 19,
    kTimedScoreOpportunity = 20,
    kCourseRecordOpportunity = 21,
    kElevatedGreenDeadline = 22,
    kWindStrength = 23,
    kReactionModeOne = 25,
    kItemStateZero = 26,
    kItemSelected = 27,
    kDriveRecordMatched = 28,
    kPreviousHolePlayerResult = 29,
    kNpcMessageResult = 30,
    kMatchLeadEqualsRemaining = 31,
    kMatchLeadExceedsRemaining = 32,
    kMatchGreenStrokeLead = 33
};
struct CaddieCommentStrings {
    char preserved0[0xf3];
    char tutorialPointFormat[11];
    char driverClubName[3];
    char itemSlotAction[9];
    char npcMessage[4];
};
enum CaddieCommentStringOffsets {
    kTutorialPointFormat = (int)&((CaddieCommentStrings*)0)->tutorialPointFormat,
    kDriverClubName = (int)&((CaddieCommentStrings*)0)->driverClubName,
    kItemSlotAction = (int)&((CaddieCommentStrings*)0)->itemSlotAction,
    kNpcMessage = (int)&((CaddieCommentStrings*)0)->npcMessage
};

extern float kFloat_neg1_GolfPlayerDirection;
extern void* g_fx;
extern CSharedDoc* g_sharedDoc;

struct EnabledCaddieObject {
    char preserved0[4];
    int enabled;
    void* holder;
};

struct CaddieObjectHolder {
    void* object;
};

struct CaddieEnableDispatch {
    virtual void v00();
    virtual void v01();
    virtual void v02();
    virtual void v03();
    virtual void v04();
    virtual void v05();
    virtual void v06();
    virtual void v07();
    virtual void v08();
    virtual void v09();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void v16();
    virtual void v17();
    virtual void v18();
    virtual void v19();
    virtual void v20();
    virtual void v21();
    virtual void v22();
    virtual void v23();
    virtual void v24();
    virtual void v25();
    virtual void setEnabled(int, int);
};

// Enable the held caddie object when this entry is active and has an object holder.
void CaddieObject_ApplyEnabledState(EnabledCaddieObject* arg) {
    if (arg->enabled != 0) {
        CaddieObjectHolder* holder = (CaddieObjectHolder*)arg->holder;
        if (holder == 0)
            return;
        ((CaddieEnableDispatch*)holder->object)->setEnabled(1, 0);
    }
}

// Test |value| against a strict limit, using the float sign bit to preserve signed-zero behavior.
int Float_IsAbsBelow(float* arg, float limit) {
    float value;
    if (((unsigned int*)arg)[0] & 0x80000000) {
        value = -*arg;
    } else {
        value = *arg;
    }
    return value < limit;
}

// Return the shared turn-time limit through the original pointer-sized ABI.
void* SharedDoc_GetTurnTimeLimitMs() {
    return (void*)g_sharedDoc->turnTimeLimitMs;
}

float* CPetBody::GetMovementDirectionPtr() {
    return movementDirection;
}

CFx* CFx::GetInstance() {
    return (CFx*)g_fx;
}

// Integer words preserve the original vector-copy instructions; arithmetic uses WVector views.
struct VectorCopyWords {
    int x, y, z;
};

struct CaddieHoleRecord {
    char pad0[0xC];
    VectorCopyWords pos;
    unsigned char par;
    char pad19;
    unsigned char resultByPlayer[6]; // Four player results plus the original trailing padding.
};

struct CaddieGolfDocView {
    char pad0[8];
    void* courseData;
    char pad0c[0x8C];
    int tutorialStage;
    char pad9c[0x21D];
    unsigned char playerCount;
    char pad2ba[2];
    CaddieHoleRecord* holeRecords;
    char pad2c0[0x7C];
    VectorCopyWords tutorialAimDirection;
    char pad348[0x6E];
    unsigned char currentHoleIndex;
    char pad3b7;
    unsigned int remainingTimeMs;
    char pad3bc[5];
    unsigned char currentPlayer;
};

// Signed timer view preserves this routine's signed tests before explicit unsigned deadline comparisons.
struct CaddieSharedDocView {
    char pad0[0x8C0];
    unsigned char golfRuleType;
    char pad8c1[0x104];
    unsigned char roundHoleCount;
    char pad9c6[2];
    int turnTimeLimitMs;
    char pad9cc[0x1C4];
    CaddieGolfDocView* golfDoc;
};

// Borrowed shot inputs: +4 selects a club record whose +A byte is compared
// with putter code 3. +48 is only known to be tested below 4, and +4C bit 0
// enables a tutorial range-close test; other modes/bits remain unresolved.
struct CaddieShotParameters {
    char pad0[4];
    struct {
        char pad0[0xA];
        unsigned char clubCode; /* club code at record +A */
    }* clubRecord;              // borrowed club record
    char pad8[0x40];
    unsigned char b48; // shot-state code: score prompt only when < 4
    char pad49[3];
    unsigned int shotFlags4C; // shot flags: bit 0 enables tutorial close-range feedback
};

struct CaddiePlayerFieldsView {
    char preserved0[0x9c];
    unsigned char itemState;
    char preserved9d[0x1f];
    unsigned char roundState;
    char preservedbd[0xb];
    int roundResult;
};
// Borrow the few player fields needed by commentary without implying a full class layout.
inline CaddiePlayerFieldsView* CaddiePlayerFields(void* player) {
    return (CaddiePlayerFieldsView*)player;
}
struct CaddieInventoryItem {
    int inventoryIndex;
    unsigned int typeId;
};
struct CaddieEntryInstance {
    char preserved0[0x14];
    CaddieInventoryItem* inventoryItem;
};
struct CaddieActionRestrictions {
    char preserved0[0x48];
    int flags;
};
struct CaddieCourseBest {
    short strokeCount;
    char preserved2[6];
};
// Strided projection of shared player records; preserve the 0x1D4-byte stride without assuming a course count.
struct CaddiePlayerRecords {
    char preserved0[0x70];
    float longestDrive;
    char preserved74[0x40];
    CaddieCourseBest firstCourseBest;
    char preservedBC[0x118];
};

struct CaddieCourseMarker {
    char preserved0[0x24];
    VectorCopyWords position;
};
struct CaddieCameraTransform {
    VectorCopyWords basis[3];
    VectorCopyWords position;
};
enum CaddieRecordOffsets {
    kCourseMarkerPosition = (int)&((CaddieCourseMarker*)0)->position,
    kCameraPosition = (int)&((CaddieCameraTransform*)0)->position,
    kRoundResultOffset = (int)&((CaddiePlayerFieldsView*)0)->roundResult,
    kHoleStrokesOffset = (int)&((GolfDocSnapshot8013*)0)->holeStrokeCounts,
    kHoleResultsOffset = (int)&((GolfDocSnapshot8013*)0)->holeResultValues,
    kHolePlayerResultOffset = (int)&((CaddieHoleRecord*)0)->resultByPlayer,
    kPlayerRecordStride = sizeof(CaddiePlayerRecords),
    kLongestDriveOffset = (int)&((CaddiePlayerRecords*)0)->longestDrive,
    kCourseBestOffset = (int)&((CaddiePlayerRecords*)0)->firstCourseBest,
    kCourseBestStride = sizeof(CaddieCourseBest)
};
// Read the current player's best drive from a base already biased by player stride.
#define PlayerLongestDrive(record) (*(float*)((char*)(record) + (kLongestDriveOffset)))
// Read the accumulated round result; interpretation depends on the golf rule.
#define PlayerRoundResult(player) (*(int*)((char*)(player) + kRoundResultOffset))
// Read an unsigned stroke count using a zero-based round-hole index.
// Preserve base+(offset+index) and expression evaluation: by-value inline helpers
// changed instructions/relocations when indices came from singleton-backed state.
#define PlayerHoleStrokes(player, hole) (*(unsigned char*)((char*)(player) + (kHoleStrokesOffset + (hole))))
// Read a per-hole result code; this unit does not establish all code meanings.
#define PlayerHoleResult(player, hole) (*(unsigned char*)((char*)(player) + (kHoleResultsOffset + (hole))))
// Read a result code from a hole record using a zero-based player index.
#define HolePlayerResult(hole, player) (*(unsigned char*)((char*)(hole) + (kHolePlayerResultOffset + (player))))
typedef char CaddiePlayerRecords_size[(sizeof(CaddiePlayerRecords) == 0x1d4) ? 1 : -1];

void __ml__FfRC7WVector(void*, void*, float);
struct AxisRotationResult {
    char b[0x18];
    VectorCopyWords v;
    char tail[0xC];
};
// The real signature: the struct return puts the hidden result pointer in r3 and the axis in r4.
AxisRotationResult Matrix_SetAxisRotation(int, float);
char* String_FormatTemporary(const char*, ...);
void* CourseData_FindNamedPoint(void*, void*);
CGolfBall* GolfBall__Fv();
void* __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float Vector_SignedAngleXZ(void*, void*);
float WVectorLen__FRC7WVector(void*);
CaddieShotParameters* InstanceTable_Get();
float GolfParameter_ResolveFloat(void*, void*);
void* GolfPlayer_GetEntryByIndex(void*, int);
void* __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void* CQuadTree_GetInstance();
extern "C" void GetWindVector__5CWindFv(void* result, const void* self);
float __ml__FRC7WVectorRC7WVector(void*, void*);
float Magnitude__7WVectorCFv(void*);
extern "C" void GetWindVectorAtPoint__5CWindFRC7WVectorf(void* result, const void* self, const void* point,
                                                         float maxBaseMagnitude);
int SharedDoc_GetCourseId();
int Pointer_GetRisingActionBits(void*, void*, int);
void IActor_DispatchMessage(void*, void*, void*, int, int, int*, int, int);
extern void* g_inputPointer;
extern char s_shotActionRestrictionState[];
extern char golfCaddieStrings[];
extern char s_cameraTransform[];
extern float kFloat_3_2_GolfCaddie, kFloat_neg0_5235988_GolfCaddie, kFloat_0_5235988_GolfCaddie, kFloat_0_GolfCaddie,
    kFloat_0_17453292_GolfCaddie;
extern float kFloat_89_6_GolfCaddie, kFloat_44_8_GolfCaddie, kFloat_48_GolfCaddie,
    kCaddieInShotDirectionAngleToleranceRadians, kFloat_1_0471976_GolfCaddie;
extern float kFloat_1_6_GolfCaddie, kFloat_4_GolfCaddie, kFloat_33_GolfCaddie, kFloat_10_GolfCaddie,
    kFloat_2_GolfPlayer;
extern float kFloat_32_GolfCaddie, kFloat_99999_GolfCaddie, kFloat_11_GolfCaddie, kFloat_9_GolfCaddie,
    kFloat_7_GolfCaddie;
extern float kFloat_5_Caddie, kFloat_3_GolfCaddie, kFloat_960_GolfCaddie, kFloat_0_00001_Caddie;
int g_currentTimeMilliseconds;

// Choose caddie comments from tutorial aim, terrain, wind, item, score and record conditions.
// delta is seconds (wind timer); absolute/deadline timers are milliseconds.
// Vector positions/distances are world units, Y is height, XZ is the ground plane;
// aimYaw and signed angle thresholds are radians. Hole/player indices are zero-based.
// Calls are priority-ordered checks, not an exclusive choice: several comments
// may be requested in one update. TriggerCommentary arbitrates them elsewhere.
// Borrowed shared state, ball, course markers and current player must exist;
// this routine relies on that game-state contract and adds no validation.
void CGolfCaddie::UpdateSituationalCommentary(float delta) {
    VectorCopyWords aimDirection;
    float targetDistance;
    int result;

    VectorCopyWords shotRelative;
    VectorCopyWords recordRelative;
    VectorCopyWords tutorialDistanceVector;
    VectorCopyWords tutorialAngleVector;
    VectorCopyWords ballToCaddie;
    VectorCopyWords projectedShotEnd;
    VectorCopyWords scaledShotDirection;
    VectorCopyWords ballToHoleForAim;
    VectorCopyWords holePos1;
    VectorCopyWords holePos2;
    VectorCopyWords holePos3;
    VectorCopyWords wind;
    VectorCopyWords projectedDriveEnd;
    VectorCopyWords scaledHoleDirection;
    VectorCopyWords windAtCamera;

    float recordDifference;
    CaddieSharedDocView* game;
    // This scratch keeps delta in normal play and holds the tutorial aim angle in the early-return path.
    float deltaOrAimAngle = delta;

    unsigned int elapsedSinceComment = g_currentTimeMilliseconds - commentaryTimestampMs;
    int commentDelayElapsed = elapsedSinceComment > kCommentDelayMs;

    // Rule navigation (checks can emit more than one request):
    // tutorial -> RETURN; normal deadline/aim -> terrain -> streak/par-five ->
    // timed score -> wind -> item/drive -> match (RETURN) or stroke -> record/NPC.
    // Only tutorial/aim tests share the 1500 ms gate. Wind, terrain and score
    // checks below have their own gates; moving them under that gate changes play.
    // Phase 1: derive horizontal aim and handle tutorial-only angle/range prompts.
    aimDirection = Matrix_SetAxisRotation(kCaddieYawAxis, aimYaw).v;
    game = (CaddieSharedDocView*)g_sharedDoc;
    if (kLastAimTutorialStage >= game->golfDoc->tutorialStage) {
        int reactionSign;
        int a;
        reactionSign = (a = reactionValue) >> 31;
        if (((reactionSign ^ a) - reactionSign) > 1) {
            if (a > 0)
                TriggerCommentary(kPositiveReaction, 0, 0);
            else
                TriggerCommentary(kNegativeReaction, 0, 0);
        }
        {
            CaddieGolfDocView* m = ((CaddieSharedDocView*)g_sharedDoc)->golfDoc;
            char* markerName =
                String_FormatTemporary(golfCaddieStrings + kTutorialPointFormat, ((1 + m->tutorialStage)));
            char* marker = (char*)CourseData_FindNamedPoint(m->courseData, markerName);
            CaddieGolfDocView* m2 = ((CaddieSharedDocView*)g_sharedDoc)->golfDoc;
            __mi__FRC7WVectorRC7WVector(&tutorialAngleVector, marker + kCourseMarkerPosition, GolfBall__Fv());
            deltaOrAimAngle = Vector_SignedAngleXZ(&m2->tutorialAimDirection, &tutorialAngleVector);
            __mi__FRC7WVectorRC7WVector(&tutorialDistanceVector, marker + kCourseMarkerPosition, GolfBall__Fv());
            targetDistance = WVectorLen(*(WVector*)&tutorialDistanceVector);
            targetDistance =
                targetDistance - kFloat_3_2_GolfCaddie * GolfParameter_ResolveFloat(InstanceTable_Get(), 0);
        }
        // Tutorial-only exits never reach normal-play wind, score or item rules.
        if (commentDelayElapsed == 0)
            return;
        if (aimCommentIssued)
            return;
        if (deltaOrAimAngle < kFloat_neg0_5235988_GolfCaddie) {
            aimCommentIssued = 1;
            TriggerCommentary(kTutorialAimNegative, 0, 0);
        } else if (deltaOrAimAngle > kFloat_0_5235988_GolfCaddie) {
            aimCommentIssued = 1;
            TriggerCommentary(kTutorialAimPositive, 0, 0);
        }
        deltaOrAimAngle = deltaOrAimAngle > kFloat_0_GolfCaddie ? deltaOrAimAngle : -deltaOrAimAngle;
        // Range advice is meaningful only within the 10-degree aim window.
        if (!(deltaOrAimAngle < kFloat_0_17453292_GolfCaddie))
            return;
        if (actionFlags[3] || actionFlags[4]) {
            float rangeThreshold =
                ((CaddieSharedDocView*)g_sharedDoc)->golfDoc->tutorialStage == kLongRangeTutorialStage
                    ? kFloat_89_6_GolfCaddie
                    : kFloat_44_8_GolfCaddie;

            if ((targetDistance > kFloat_0_GolfCaddie ? targetDistance : -targetDistance) > rangeThreshold)
                TriggerCommentary(kTutorialRangeWarning, 0, 0);
            else
                TriggerCommentary(kTutorialRangeClose, 0, 0);
            actionFlags[3] = 0;
            actionFlags[4] = 0;
        } else if (InstanceTable_Get()->shotFlags4C & kRangeCloseTestFlag) {
            float absoluteTargetDistance = targetDistance > kFloat_0_GolfCaddie ? targetDistance : -targetDistance;
            if (absoluteTargetDistance < kFloat_48_GolfCaddie)
                TriggerCommentary(kTutorialRangeClose, 0, 0);
        }
        {
            float rangeThreshold =
                ((CaddieSharedDocView*)g_sharedDoc)->golfDoc->tutorialStage == kLongRangeTutorialStage
                    ? kFloat_89_6_GolfCaddie
                    : kFloat_44_8_GolfCaddie;
            float absoluteTargetDistance = targetDistance > kFloat_0_GolfCaddie ? targetDistance : -targetDistance;
            if (absoluteTargetDistance > rangeThreshold) {
                if (targetDistance > kFloat_0_GolfCaddie)
                    TriggerCommentary(kNeedMoreRange, 0, 0);
                else
                    TriggerCommentary(kNeedLessRange, 0, 0);
            }
        }
        return;
    }
    // Phase 2: normal-play deadline and aim hazards. The aim latch suppresses
    // repeated directional prompts, but it does not gate all later comment types.
    if (game->turnTimeLimitMs != 0 &&
        ((CaddieSharedDocView*)g_sharedDoc)->golfDoc->remainingTimeMs < kDeadlineWarningMs) {
        TriggerCommentary(kDeadlineWarning, 0, 0);
    }
    if (commentDelayElapsed && !aimCommentIssued) {
        if (InstanceTable_Get()->clubRecord->clubCode != kPutterClub) {
            float a;
            __mi__FRC7WVectorRC7WVector(&ballToCaddie, &position, GolfBall__Fv());
            a = Vector_SignedAngleXZ(&aimDirection, &ballToCaddie);
            a = a > kFloat_0_GolfCaddie ? a : -a;
            if (a < kCaddieInShotDirectionAngleToleranceRadians) {
                aimCommentIssued = 1;
                TriggerCommentary(kCaddieInShotDirection, 0, 0);
            }
        }
        {
            __ml__FfRC7WVector(&scaledShotDirection, &aimDirection,
                               kFloat_3_2_GolfCaddie * GolfParameter_ResolveFloat(InstanceTable_Get(), 0));
            int r;
            __pl__FRC7WVectorRC7WVector(&projectedShotEnd, GolfBall__Fv(), &scaledShotDirection);
            r = ((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(&projectedShotEnd, 1);
            if (InstanceTable_Get()->clubRecord->clubCode != kPutterClub && r < 0) {
                aimCommentIssued = 1;
                TriggerCommentary(kNoGroundAhead, 0, 0);
            }
        }
        if (reactionModeOneEnabled) {
            aimCommentIssued = 1;
            TriggerCommentary(kReactionModeOne, 0, 0);
        }
    }
    // The remainder tests the current hole, player records, and game mode.
// Preserve repeated singleton reloads in this path.
#define GAME ((CaddieSharedDocView*)g_sharedDoc)
#define MGR (GAME->golfDoc)

    // Phase 3: green/ground-code-3 and timed wind checks use current-hole world geometry.
    if (GolfBall__Fv()->m_groundType == kGreen) {
        if (commentDelayElapsed && aimCommentIssued == 0) {
            holePos1 = MGR->holeRecords[MGR->currentHoleIndex].pos;
            __mi__FRC7WVectorRC7WVector(&ballToHoleForAim, &holePos1, GolfBall__Fv());
            float a = Vector_SignedAngleXZ(&aimDirection, &ballToHoleForAim);
            a = a > kFloat_0_GolfCaddie ? a : -a;
            if (a > kFloat_1_0471976_GolfCaddie) {
                aimCommentIssued = 1;
                TriggerCommentary(kAimAwayFromHole, 0, 0);
            }
        }
        if (GAME->turnTimeLimitMs) {
            if (kSpecialCaddieDeadlineMs > MGR->remainingTimeMs) {
                if (((CaddieEntryInstance*)activeCaddie)->inventoryItem->typeId == kBaseCaddieTypeId) {
                    TriggerCommentary(kTimedBaseCaddie, 0, 0);
                }
            }
        }
        // The distance scratch is reused for the ball-radius correction in this height test.
        targetDistance = GolfBall__Fv()->m_radius;
        holePos2 = MGR->holeRecords[MGR->currentHoleIndex].pos;
        if (((WVector*)&holePos2)->y > kFloat_1_6_GolfCaddie + (GolfBall__Fv()->m_pos.y - targetDistance)) {
            // 0x8012B454.
            void* player = GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer);
            if (PlayerHoleResult(player, MGR->currentHoleIndex) == 0 && GAME->turnTimeLimitMs &&
                MGR->remainingTimeMs < (unsigned int)(GAME->turnTimeLimitMs - kDeadlineMarginMs)) {
                TriggerCommentary(kElevatedGreenDeadline, 0, 0);
            }
        }
    } else {
        if (GAME->turnTimeLimitMs && MGR->remainingTimeMs < kWindDeadlineMs &&
            CWind::GetInstance()->GetWindSpeed() > kFloat_4_GolfCaddie) {
            TriggerCommentary(kTimedStrongWind, 0, 0);
        }
        if (GolfBall__Fv()->m_groundType == kCommentGroundCode3)
            TriggerCommentary(kGroundCode3Comment, 0, 0);
    }
    // Phase 4: recent results and par-five reach. Result code 1 is tested as-is;
    // its outcome meaning is not established here. The streak scans five prior holes.
    if (!GAME->golfRuleType && MGR->currentHoleIndex + 1 > 5 &&
        HolePlayerResult(&MGR->holeRecords[(unsigned char)(MGR->currentHoleIndex - 1)], MGR->currentPlayer) == 1) {
        int i;
        for (i = 0; i < MGR->playerCount; ++i) {
            if (((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(MGR, (unsigned char)i))->playerState !=
                    kDisabledPlayer &&
                i != MGR->currentPlayer &&
                HolePlayerResult(&MGR->holeRecords[(unsigned char)(MGR->currentHoleIndex - 1)], i) == 1)
                break;
        }
        CaddieGolfDocView*& m = MGR;
        int hole = m->currentHoleIndex;
        int j;
        for (j = hole - 1; j > m->currentHoleIndex - 6; --j) {
            if (HolePlayerResult(&m->holeRecords[(unsigned char)j], m->currentPlayer) != 1)
                break;
            if (HolePlayerResult(&m->holeRecords[(unsigned char)j], i) != 1)
                break;
        }
        if (j == hole - 6)
            TriggerCommentary(kFiveHoleResultStreak, 0, 0);
    }
    if (kParFive == MGR->holeRecords[MGR->currentHoleIndex].par &&
        PlayerHoleStrokes(GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer), MGR->currentHoleIndex) == 1 &&
        ((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer))->powerShotMeter >=
            kFloat_33_GolfCaddie) {
        holePos3 = MGR->holeRecords[MGR->currentHoleIndex].pos;
        __mi__FRC7WVectorRC7WVector(&shotRelative, &holePos3, GolfBall__Fv());
        ((WVector*)&shotRelative)->y = kFloat_0_GolfCaddie;
        GetWindVector__5CWindFv(&wind, CWind::GetInstance());
        if ((*(WVector*)&wind * *(WVector*)&shotRelative) > kFloat_0_GolfCaddie) {
            // shotRelative is horizontal ball-to-hole displacement, not normalized
            // here. Preserve that vector scaling and the 3.2 world-unit factor.
            __ml__FfRC7WVector(&scaledHoleDirection, &shotRelative,
                               kFloat_3_2_GolfCaddie * (kFloat_10_GolfCaddie +
                                                        GolfParameter_ResolveFloat(
                                                            InstanceTable_Get(), golfCaddieStrings + kDriverClubName)));
            __pl__FRC7WVectorRC7WVector(&projectedDriveEnd, GolfBall__Fv(), &scaledHoleDirection);
            if (((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(&projectedDriveEnd, 1) > 0) {
                void* player = GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer);
                targetDistance = (*(WVector*)&shotRelative).Magnitude();
                float power = GolfParameter_ResolveFloat(InstanceTable_Get(), golfCaddieStrings + kDriverClubName);
                // Reach bound: twice driver range in world units, plus the
                // 32-unit power-meter allowance scaled by meter/33.
                if (kFloat_2_GolfPlayer * (kFloat_3_2_GolfCaddie * power) +
                        kFloat_32_GolfCaddie * (((GolfDocSnapshot8013*)player)->powerShotMeter / kFloat_33_GolfCaddie) >
                    targetDistance)
                    TriggerCommentary(kParFiveReachable, 0, 0);
            }
        }
    }
    // Independent timed-score rule: it does not depend on the par-five test.
    if (GAME->turnTimeLimitMs && MGR->remainingTimeMs < (unsigned int)(GAME->turnTimeLimitMs - kDeadlineMarginMs) &&
        GolfBall__Fv()->m_groundType != kGreen && InstanceTable_Get()->b48 < 4) {
        void* player = GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer);
        if (PlayerHoleStrokes(player, MGR->currentHoleIndex) <= MGR->holeRecords[MGR->currentHoleIndex].par)
            TriggerCommentary(kTimedScoreOpportunity, 0, 0);
    }
    // Phase 5: accumulate seconds inside the camera's wind region, then bucket
    // sampled wind magnitude into comment arguments 1..5. Leaving resets the timer.
    if (InstanceTable_Get()->clubRecord->clubCode != kPutterClub &&
        CWind::GetInstance()->ContainsPoint(*(const WVector*)(s_cameraTransform + kCameraPosition))) {
        windCommentTimer = (((windCommentTimer + deltaOrAimAngle)));
        if (windCommentTimer > kFloat_2_GolfPlayer) {
            GetWindVectorAtPoint__5CWindFRC7WVectorf(&windAtCamera, CWind::GetInstance(),
                                                     s_cameraTransform + kCameraPosition, kFloat_99999_GolfCaddie);
            float windMagnitude = (*(WVector*)&windAtCamera).Magnitude();
            if (windMagnitude > kFloat_11_GolfCaddie)
                TriggerCommentary(kWindStrength, 0, 5);
            else if (windMagnitude > kFloat_9_GolfCaddie)
                TriggerCommentary(kWindStrength, 0, 4);
            else if (windMagnitude > kFloat_7_GolfCaddie)
                TriggerCommentary(kWindStrength, 0, 3);
            else if (windMagnitude > kFloat_5_Caddie)
                TriggerCommentary(kWindStrength, 0, 2);
            else if (windMagnitude > kFloat_3_GolfCaddie)
                TriggerCommentary(kWindStrength, 0, 1);
        }
    } else
        windCommentTimer = kFloat_0_GolfCaddie;
    // Phase 6: item-action feedback and horizontal shot displacement versus best drive.
    if (Pointer_GetRisingActionBits(g_inputPointer, golfCaddieStrings + kItemSlotAction, 1)) {
        if (!(((CaddieActionRestrictions*)s_shotActionRestrictionState)->flags & kItemActionBlocked)) {
            if (CaddiePlayerFields(GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer))->itemState == 0)
                TriggerCommentary(kItemStateZero, 0, 0);
            else if (((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer))->itemId)
                TriggerCommentary(kItemSelected, 0, 0);
        }
    }
    {
        void* player = GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer);
        void* player2 = GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer);
        // Shot-end minus shot-start position, flattened below for the longest-drive comparison.
        __mi__FRC7WVectorRC7WVector(&recordRelative, ((GolfDocSnapshot8013*)player2)->ballPosition,
                                    ((GolfDocSnapshot8013*)player)->previousBallPosition);
        ((WVector*)&recordRelative)->y = kFloat_0_GolfCaddie;
        char* record = (char*)GAME + MGR->currentPlayer * kPlayerRecordStride;
        int matchesDriveRecord = 0;
        if (PlayerLongestDrive(record) > kFloat_960_GolfCaddie) {
            recordDifference = (*(WVector*)&recordRelative).Magnitude() - PlayerLongestDrive(record);
            if (Float_IsAbsBelow(&recordDifference, kFloat_0_00001_Caddie))
                matchesDriveRecord = 1;
        }
        if (matchesDriveRecord)
            TriggerCommentary(kDriveRecordMatched, 0, 0);
    }
    // Phase 7: rule-specific score opportunities. Match-play checks can return
    // early, bypassing the stroke/course-record and final NPC-message checks.
    if (GAME->golfRuleType == kMatchPlay) {
        int count = GAME->roundHoleCount;
        int hole = MGR->currentHoleIndex;
        if (hole + 1 >= count)
            return;
        __typeof__(MGR->currentPlayer) savedPlayerIndex = MGR->currentPlayer;
        __typeof__(MGR->currentHoleIndex) savedHoleIndex = MGR->currentHoleIndex;
        void* other = GolfPlayer_GetEntryByIndex(MGR, (unsigned char)(1 - savedPlayerIndex));
        void* player = GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer);
        if (PlayerRoundResult(player) < PlayerRoundResult(other) + (count - (hole + 1)))
            return;
        __typeof__(MGR->currentHoleIndex) copiedHoleIndex = savedHoleIndex;
        count = GAME->roundHoleCount;
        hole = copiedHoleIndex;
        other = GolfPlayer_GetEntryByIndex(MGR, (unsigned char)(1 - MGR->currentPlayer));
        player = GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer);
        unsigned int event = kMatchLeadExceedsRemaining;
        event = (PlayerRoundResult(player) == PlayerRoundResult(other) + (count - (1 + hole)))
                    ? kMatchLeadEqualsRemaining
                    : event;
        unsigned int savedEvent = event;
        TriggerCommentary(savedEvent, 0, 0);
        if (GolfBall__Fv()->m_groundType != kGreen)
            return;
        other = GolfPlayer_GetEntryByIndex(MGR, (unsigned char)(1 - MGR->currentPlayer));
        hole = MGR->currentHoleIndex;
        player = GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer);
        if (PlayerHoleStrokes(player, MGR->currentHoleIndex) < PlayerHoleStrokes(other, hole))
            TriggerCommentary(kMatchGreenStrokeLead, 0, 0);
        return;
    } else if (GAME->golfRuleType == kStrokePlay) {
        void* player = GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer);
        if (PlayerHoleStrokes(player, MGR->currentHoleIndex) == 1) {
            if (1 < MGR->playerCount) {
                int i;
                for (i = 0; i < MGR->playerCount; ++i) {
                    if (((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(MGR, (unsigned char)i))->playerState !=
                        kDisabledPlayer) {
                        if (i != MGR->currentPlayer) {
                            void* other = GolfPlayer_GetEntryByIndex(MGR, (unsigned char)i);
                            player = GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer);
                            if (PlayerRoundResult(player) + 2 > PlayerRoundResult(other))
                                break;
                        }
                    }
                }
                if (i == MGR->playerCount && (int)MGR->currentHoleIndex % 4 == 3)
                    TriggerCommentary(kStrokeLead, 0, 0);
            }
        }
        if (CaddiePlayerFields(GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer))->roundState ==
            kPostHolePlayerState) {
            CaddieGolfDocView* m = MGR;
            if (m->currentHoleIndex) {
                CaddieHoleRecord* previous = &m->holeRecords[(unsigned char)(m->currentHoleIndex - 1)];
                unsigned char result = ((CGolfDoc*)m)->GetActivePlayerCount();
                if (result == HolePlayerResult(previous, m->currentPlayer))
                    TriggerCommentary(kPreviousHolePlayerResult, 0, 0);
            }
        }
    }
    // Phase 8: final-hole course record, then post-hole actor response.
    if (GAME->roundHoleCount == kFullRoundHoles && MGR->currentHoleIndex == (unsigned char)(GAME->roundHoleCount - 1) &&
        GolfBall__Fv()->m_groundType == kGreen) {
        CaddieSharedDocView* g = GAME;
        CaddieGolfDocView* m = g->golfDoc;
        unsigned int index = (unsigned char)SharedDoc_GetCourseId();
        // Record address = shared base + player*0x1D4 + course*8 + 0xB4.
        // First test the signed short against the absent-record sentinel; the
        // later comparison deliberately truncates it to an unsigned byte.
        if (*(short*)((char*)g + m->currentPlayer * kPlayerRecordStride + index * kCourseBestStride +
                      kCourseBestOffset) < kNoCourseRecord) {
            g = GAME;
            m = g->golfDoc;
            void* player = GolfPlayer_GetEntryByIndex(m, m->currentPlayer);
            int hole = MGR->currentHoleIndex;
            void* player2 = GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer);
            index = (unsigned char)SharedDoc_GetCourseId();
            // Compare stored best with completed strokes plus this hole's strokes.
            if ((unsigned char)*(short*)((char*)g + m->currentPlayer * kPlayerRecordStride + index * kCourseBestStride +
                                         kCourseBestOffset) >
                ((GolfDocSnapshot8013*)player2)->totalStrokeCount + PlayerHoleStrokes(player, hole))
                TriggerCommentary(kCourseRecordOpportunity, 0, 0);
        }
    }
    if (CaddiePlayerFields(GolfPlayer_GetEntryByIndex(MGR, MGR->currentPlayer))->roundState == kPostHolePlayerState) {
        result = 0;
        IActor_DispatchMessage(this, this, golfCaddieStrings + kNpcMessage, 6, 4, &result, 0, 0);
        if (result)
            TriggerCommentary(kNpcMessageResult, 0, 0);
    }
#undef GAME
#undef MGR
}

// The forward direction of the body's base transform, negated.
WVector CPetBody::GetNegatedBaseForward() {
    return kFloat_neg1_GolfPlayerDirection * *(WVector*)&baseMatrixBasis[6];
}

} // extern "C"
