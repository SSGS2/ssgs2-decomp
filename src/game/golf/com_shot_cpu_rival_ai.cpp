#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include "classes/CGolfDoc.h"
#include "fp_fused.h"
// Layout evidence: 0x801DDF64 candidate tree; 0x801DECF8 CRival state; 0x801E2960 current hole.
#include "classes/CSharedDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/types.h"
#include "game/wvector.h"
#include "game/golfball.h"

// The pool keeps the original Shift-JIS bytes; members describe string boundaries.
struct ComShotStringPool {
    char playerActor[7]; // +0x0
    char driverParameter[3]; // +0x7
    char positionLabel[5]; // +0xA
    char vectorFormat[32]; // +0xF
    char candidateRadiusFormat[48]; // +0x2F
    char playerBannerFormat[49]; // +0x5F
    char windLabel[7]; // +0x90
    char checkStart[33]; // +0x97
    char priorityFormat[17]; // +0xB8
    char checkpointRejected[46]; // +0xC9
    char noMoreCandidates[52]; // +0xF7
    char unplayableFallback[56]; // +0x12B
    char screenActor[7]; // +0x163
    char selectedClubFormat[24]; // +0x16A
    char powerFormat[23]; // +0x182
    char curveFormat[23]; // +0x199
    char outputLabel[6]; // +0x1B0
    char alignmentPadding[2];
};

// CPU shot pipeline: build checkpoint candidates -> run each correction chain ->
// simulate angle/power samples -> validate landing rules -> cache/apply a shot.
// Mod lookup: candidate ranking/club selection and simulation live in this unit;
// kCpuShotHorizontalStopToleranceWorld is the power-search stop tolerance (world distance, below).
// Companion scalar/string definitions are in u801DD078_data.cpp; the live ball
// ABI is game/golfball.h. Simulation calls the terrain/ball providers and sends
// Player message 11 to count usable swing placements at each history position.
// A second, independent subsystem shares this address bundle: CRiver_Ctor near
// the end initializes path/auxiliary vectors, two scrolling texture layers and
// an embedded generated-texture resource. For river width/texture/scroll edits,
// follow RiverConstructionView -> CRiver_BuildTerrainStrips/LoadTextureLayers in
// river_lava_texture_layers.cpp; CPU sampling constants do not tune river geometry or scrolling.
// Task states: 0 idle, 1 start, 2 update, 3 await child/finish. Start and update
// fall through in one call; finish result 1 repeats update, any other value stops.
// Correction axes alternate 0/1 over four directions; sample configurations must
// contain at least TWO samples (the interpolation divides by sampleCount-1).
// Candidate angles are radians, power is a normalized ratio, positions are world
// coordinates (+Y vertical). Horizontal distances explicitly zero Y; /3.2 converts
// world distance to yards (the marker label uses that same 3.2 convention).
// candidate.rejected combines landing-rule, slope, hazard and distance
// failures. Ball recoveryRequired is the distinct live byte at +0x58.
// Tree nodes are owned; checkpoints/configuration/rival links are borrowed.
enum ThinkTaskState { kTaskIdle = 0, kTaskStart = 1, kTaskUpdate = 2, kTaskAwaitChild = 3 };
enum {
    kInvalidClub = -1,
    kImpactUnset = 0, kImpactMiss = 1, kImpactPoor = 2, kImpactGood = 3, kImpactPangya = 4,
    kSpecialShotMask = 0x7F, kPersistingSpecialShotMask = 0x70,
    kTaskResultRepeat = 1,
    kRadialProbeCount = 8,
    kMaxSimulationFrames = 1800,
    kSlowSimulationFrames = 900,
    kCorrectionDirectionCount = 4,
    kNormalPowerRandomIndex = 0, kNormalImpactRandomIndex = 1,
    kPuttPowerRandomIndex = 2, kPuttAngleRandomIndex = 3,
    kNoTargetPowerRandomIndex = 4, kNoTargetImpactRandomIndex = 5,
    kSpecialPowerRandomIndex = 6, kSpecialImpactRandomIndex = 7,
    kThinkOptionSpecialLanding = 0xE, kThinkOptionFirstLanding = 0x10
};

// Narrow views for fields absent from the existing shared class headers.
struct ClubSelectionInfo {
    char pad_00[8];
    unsigned char category, clubIndex, flightMode;
};
struct ShotParameterState {
    char pad_00[4]; ClubSelectionInfo* selectedClub;
    char pad_08[0x3C]; int powerShotLevel;
    unsigned char clubKind;
};
struct RivalAimView { char pad_00[0x10]; Vec3f target; };
struct ShotTargetArea { char pad_00[0xC]; Vec3f position; float radius; int index; };
struct RivalTargetAreaView { char pad_00[0x54]; ShotTargetArea* targetArea; };
struct CourseHoleView { char pad_00[0xC]; Vec3f position; char pad_18[8]; };
struct PlayerCurveStatView { char pad_00[0xE8]; int curve; };
// Simulation/correction payload: the same 0x40-byte result is copied wordwise
// so floating-point payload bits and byte-sized ground flags remain unchanged.
struct ShotCandidateLayout {
    float aimAngle, remainingDistance, power, curve;
    Vec3f position;
    unsigned int rejected;
    unsigned char groundType; char pad_21[3];
    unsigned int nearHazard, firstLandingWithinTarget, finalWithinTarget;
    unsigned int hasSpecialLanding, hasInvalidLanding, holed, simulationFrames;
};
struct ShotProcessorBestView { char pad_00[0x4C]; ShotCandidateLayout bestCandidate; };
typedef char ShotCandidateLayout_size[(sizeof(ShotCandidateLayout) == 0x40) ? 1 : -1];
typedef char ComShotStringPool_size[(sizeof(ComShotStringPool) == 0x1B8) ? 1 : -1];
struct PlayerRoundFlagsView { char pad_00[0x28]; unsigned char strokes[18]; char pad_3A[0x82]; unsigned char flagBC; };
struct PlayerPositionView { char pad_00[0x1C]; Vec3f position; };
struct BallRecordView {
    Vec3f position, velocity; float rotation[2];
    unsigned char event20; char pad_21[0x3F];
    unsigned char event60, event61;
    char pad_62[0xE]; int motionState;
};
struct SharedHoleCharacterView { char pad_00[0x83D]; unsigned char holeSlots[18]; };
struct SharedCpuCharacterRecord { char pad_00[0x1A4]; signed char characters[0x30]; };

#define AsGolfBall(p) ((CGolfBall*)(p))
#define AsShotParameters(p) ((ShotParameterState*)(p))

// Translation unit 0x801DD078-0x801E356C (merged from com_shot_cpu_rival_ai.cpp, com_shot_cpu_rival_ai.cpp, com_shot_cpu_rival_ai.cpp, com_shot_cpu_rival_ai.cpp, com_shot_cpu_rival_ai.cpp).
#include "sdk_protos.h"
#include "classes/CComShotProc.h"
#include "classes/CComCorrectionChain.h"
#include "classes/CRival.h"
#include <decomp/cw_bitfields.h>
extern "C" char __vt__12CComShotProc[];

#define AsCRival(p) ((CRival*)(p))

#define AsCComShotProc(p) ((CComShotProc*)(p))

extern "C" int __cntlzw(unsigned int);

// ---- from com_shot_cpu_rival_ai.cpp ----
namespace ns0 {
extern "C" {
typedef Vec3f V3;

extern "C" int g_taskManager;
extern "C" CSharedDoc* g_sharedDoc;
extern "C" float kFloat_3_2_ComShot;
extern "C" const float kFloat_0_ComShot;
extern "C" float kFloat_0_1_U801DD078;
extern "C" const float kFloat_1_Rival;
extern "C" float kFloat_0_01_CourseObject, kFloat_0_34906584_ComShot, kFloat_16_ComShot, kFloat_1_6_ComShotBounds;
extern "C" char comShotDebugStrings[];
extern "C" void* InstanceTable_Get();
extern "C" void Matrix_SetAxisRotation(void*, float, int);
extern "C" void __as__7WVectorFRC7WVector(void*, void*);
extern "C" void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern "C" void* CQuadTree_GetInstance();
extern "C" void* Object_GetChild(void*);
extern "C" void* TaskActorTable_FindByName(void*, const char*);
extern "C" void __ml__FfRC7WVector(void*, void*, float);
extern "C" void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern "C" float WVectorLen__FRC7WVector(void*);
extern "C" void Normalize__7WVectorFv(void*);
extern "C" void Matrix_TransformPoint(void*, void*, void*);
extern "C" void Vector_CrossProduct(void*, void*, void*);
extern "C" void Vec3_Zero(void*);
extern "C" void CQuadTree_GetObjList(void*, void*);
extern "C" int CQuadTree_OverlappedWithObject(void*, void*, float);
extern "C" float Math_SinApprox(float);

struct Hit { V3 p; union { float d; float distance[1]; }; };
struct Seg { V3 a, b; };
struct Frame { V3 axis, side, up, fourth; };
extern "C" void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int*, int);
struct Six { float v[6]; };

// Terrain probe workspace: world XYZ vectors, signed ground indices, distances and rotation frames.
// Keep word-copy casts at ABI boundaries; the fields themselves expose type and lifetime.
struct TerrainProbeWorkspace {
    int fineGroundIndex;
    int coarseGroundIndex;
    int originGroundIndex;
    V3 fineGroundResult;
    V3 fineQueryPosition;
    V3 fineGroundDelta;
    V3 coarseGroundResult;
    V3 coarseQueryPosition;
    V3 coarseGroundDelta;
    V3 originGroundResult;
    V3 originCopy;
    V3 originGroundDelta;
    V3 clearanceQuery;
    V3 clearanceOffset;
    V3 boundsMaxTemp;
    V3 boundsMinTemp;
    V3 horizontalScale;
    V3 slopeSideTemp;
    V3 slopeDirectionTemp;
    V3 fineAdvance;
    V3 finePosition;
    V3 finePositionCopy;
    V3 coarseAdvance;
    V3 coarsePosition;
    V3 coarsePositionCopy;
    V3 clearancePosition;
    V3 boundsHalfExtent;
    V3 scanDirection;
    V3 playerProbeVector;
    V3 localProbeOffset;
    Six queryBounds;
    struct { V3 min, max; } bounds;
    V3 previousGroundPosition;
    float previousSignedDistance;
    V3 groundPosition;
    float signedDistance;
    Frame headingFrame;
    Frame slopeFrame;
    char reservedTail[4];
};
typedef char TerrainProbeWorkspaceSize[sizeof(TerrainProbeWorkspace)==0x204 ? 1 : -1];

// Probe terrain along a heading, rejecting steep ground or nearby object obstructions.
#pragma push
#pragma always_inline on
#pragma inline_depth(16)
// Query world-space object bounds around the origin, then test the slope-aligned probe endpoint.
static inline int IsTerrainProbeClearOfObjects(TerrainProbeWorkspace& probe, V3* origin) {
    // Reject slopes above the threshold; query nearby bounds in world space.
    Vec3_Zero(&probe.bounds.min);
    Vec3_Zero(&probe.bounds.max);
    probe.horizontalScale.x = kFloat_1_Rival;
    probe.horizontalScale.y = kFloat_0_ComShot;
    probe.horizontalScale.z = kFloat_1_Rival;
    __ml__FfRC7WVector(&probe.boundsHalfExtent, &probe.horizontalScale, kFloat_16_ComShot);
    __mi__FRC7WVectorRC7WVector(&probe.boundsMinTemp, origin, &probe.boundsHalfExtent);
    *(WVector*)(&probe.bounds.min) = *(WVector*)(&probe.boundsMinTemp);
    __pl__FRC7WVectorRC7WVector(&probe.boundsMaxTemp, origin, &probe.boundsHalfExtent);
    *(WVector*)(&probe.bounds.max) = *(WVector*)(&probe.boundsMaxTemp);
    probe.queryBounds = *(Six*)(&probe.bounds.min);
    CQuadTree_GetObjList(CQuadTree_GetInstance(), &probe.queryBounds);
    Matrix_TransformPoint(&probe.clearanceOffset, &probe.localProbeOffset, &probe.slopeFrame);
    __pl__FRC7WVectorRC7WVector(&probe.clearancePosition, origin, &probe.clearanceOffset);
    *(Arr3*)(&probe.clearanceQuery) = *(Arr3*)(&probe.clearancePosition);
    return CQuadTree_OverlappedWithObject(CQuadTree_GetInstance(), &probe.clearanceQuery, kFloat_1_6_ComShotBounds) == 0;
}

// Probe coarse/fine terrain crossings and reject steep slopes or an obstructed endpoint.
int ComShot_IsSlopeAndClearanceAcceptable(V3* origin, float headingRadians) {
    TerrainProbeWorkspace probe;
    float probeRadius = kFloat_3_2_ComShot;
    float fraction;
    float savedHeading = headingRadians;
    float coarseStep, zero, maxFraction, fineStep, zero2, fineEnd, absoluteSlopeY;
    void* world;
    int groundFound;

    probe.localProbeOffset.y = kFloat_0_ComShot;
    probe.localProbeOffset.x = probeRadius;
    probe.localProbeOffset.z = kFloat_0_ComShot;
    void* ptr = g_taskManager ? Object_GetChild((void*)g_taskManager) : 0;
    char* table = (char*)TaskActorTable_FindByName(ptr, comShotDebugStrings);
    if (table) {
        int id = AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer;
        void* state = InstanceTable_Get();
        int type = *((unsigned char*)*(void**)((char*)state + 4) + 0xA);
        char* e = table + (id << 6);
        e += type << 4;
        int y = *(int*)(e + 0x12c);
        int x = *(int*)(e + 0x128);
        *(int*)(&probe.playerProbeVector) = x;
        int z = *(int*)(e + 0x130);
        *(int*)(&probe.playerProbeVector.y) = y;
        probeRadius = probe.playerProbeVector.x;
        *(int*)(&probe.playerProbeVector.z) = z;
    }
    Matrix_SetAxisRotation(&probe.headingFrame, savedHeading, 1);
    __ml__FfRC7WVector(&probe.scanDirection, &probe.headingFrame, probeRadius);
    int o0 = ((int*)origin)[0];
    int o1 = ((int*)origin)[1];
    int o2 = ((int*)origin)[2];
    ((int*)(&probe.originCopy))[0] = o0;
    ((int*)(&probe.originCopy.y))[0] = o1;
    ((int*)(&probe.originCopy.z))[0] = o2;
    world = CQuadTree_GetInstance();
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&probe.originGroundResult, world, &probe.originCopy, 1, &probe.originGroundIndex, 0);
    *(WVector*)(&probe.groundPosition) = *(WVector*)(&probe.originGroundResult);
    if (probe.originGroundIndex >= 0) {
        __mi__FRC7WVectorRC7WVector(&probe.originGroundDelta, &probe.groundPosition, origin);
        probe.signedDistance = WVectorLen(*(WVector*)(&probe.originGroundDelta)) - probeRadius;
    }
    fraction = kFloat_0_ComShot;
    // Coarse search: bracket a zero crossing of (ground distance - probe radius).
    coarseStep = kFloat_0_1_U801DD078;
    zero = fraction;
    maxFraction = kFloat_1_Rival;
    while (fraction < maxFraction) {
        *(WVector*)(&probe.previousGroundPosition) = *(WVector*)(&probe.groundPosition);
        probe.previousSignedDistance = probe.signedDistance;
        __ml__FfRC7WVector(&probe.coarseAdvance, &probe.scanDirection, coarseStep + fraction);
        __pl__FRC7WVectorRC7WVector(&probe.coarsePosition, origin, &probe.coarseAdvance);
        *(Arr3*)(&probe.coarsePositionCopy) = *(Arr3*)(&probe.coarsePosition);
        *(Arr3*)(&probe.coarseQueryPosition) = *(Arr3*)(&probe.coarsePosition);
        world = CQuadTree_GetInstance();
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&probe.coarseGroundResult, world, &probe.coarseQueryPosition, 1, &probe.coarseGroundIndex, 0);
        *(WVector*)(&probe.groundPosition) = *(WVector*)(&probe.coarseGroundResult);
        if (probe.coarseGroundIndex < 0) {
            groundFound = 0;
        } else {
            __mi__FRC7WVectorRC7WVector(&probe.coarseGroundDelta, &probe.groundPosition, origin);
            probe.signedDistance = WVectorLen(*(WVector*)(&probe.coarseGroundDelta)) - probeRadius;
            groundFound = 1;
        }
        if (!groundFound) return 0;
        if (!(probe.signedDistance * probe.previousSignedDistance > zero)) break;
        fraction = fraction + coarseStep;
    }
    *(WVector*)(&probe.groundPosition) = *(WVector*)(&probe.previousGroundPosition);
    probe.signedDistance = probe.previousSignedDistance;
    // Refine that bracket at 0.01 steps, then build a terrain-aligned clearance frame.
    fineEnd = kFloat_0_1_U801DD078 + fraction;
    fineStep = kFloat_0_01_CourseObject;
    zero2 = kFloat_0_ComShot;
    while (fraction < fineEnd) {
        *(WVector*)(&probe.previousGroundPosition) = *(WVector*)(&probe.groundPosition);
        probe.previousSignedDistance = probe.signedDistance;
        __ml__FfRC7WVector(&probe.fineAdvance, &probe.scanDirection, fineStep + fraction);
        __pl__FRC7WVectorRC7WVector(&probe.finePosition, origin, &probe.fineAdvance);
        *(Arr3*)(&probe.finePositionCopy) = *(Arr3*)(&probe.finePosition);
        *(Arr3*)(&probe.fineQueryPosition) = *(Arr3*)(&probe.finePosition);
        world = CQuadTree_GetInstance();
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&probe.fineGroundResult, world, &probe.fineQueryPosition, 1, &probe.fineGroundIndex, 0);
        *(WVector*)(&probe.groundPosition) = *(WVector*)(&probe.fineGroundResult);
        if (probe.fineGroundIndex >= 0) {
            __mi__FRC7WVectorRC7WVector(&probe.fineGroundDelta, &probe.groundPosition, origin);
            probe.signedDistance = WVectorLen(*(WVector*)(&probe.fineGroundDelta)) - probeRadius;
        }
        if (!(probe.signedDistance * probe.previousSignedDistance > zero2)) {
            Matrix_SetAxisRotation(&probe.slopeFrame, savedHeading, 1);
            __mi__FRC7WVectorRC7WVector(&probe.slopeDirectionTemp, &probe.groundPosition, origin);
            *(WVector*)(&probe.slopeFrame) = *(WVector*)(&probe.slopeDirectionTemp);
            (*(WVector*)(&probe.slopeFrame)).Normalize();
            Vector_CrossProduct(&probe.slopeSideTemp, &probe.slopeFrame.up, &probe.slopeFrame);
            *(WVector*)(&probe.slopeFrame.side) = *(WVector*)(&probe.slopeSideTemp);
            absoluteSlopeY = probe.slopeFrame.axis.y;
            absoluteSlopeY = absoluteSlopeY > kFloat_0_ComShot ? absoluteSlopeY : -absoluteSlopeY;
            if (absoluteSlopeY > Math_SinApprox(kFloat_0_34906584_ComShot)) return 0;
            return IsTerrainProbeClearOfObjects(probe, origin);
        }
        fraction = fraction + fineStep;
    }
    return 0;
}
#pragma pop


void __dl__FPv(void*);

// Release correction-helper storage when called as a deleting destructor.
void* dtor_801DD784(void* self, int deleting) {
    if (self != 0 && deleting > 0) {
        __dl__FPv(self);
    }
    return self;
}

// Release angle-correction storage when deletion is requested.
void* CComShotAngleCorrection_dtor(void* self, int deleting) {
    if (self != 0 && deleting > 0) {
        __dl__FPv(self);
    }
    return self;
}

// Release power-correction storage when deletion is requested.
void* CComShotPowerCorrection_dtor(void* self, int deleting) {
    if (self != 0 && deleting > 0) {
        __dl__FPv(self);
    }
    return self;
}

struct FreeNode { FreeNode* first; FreeNode* second; };
extern "C" void RBTree_FreeTrivialSubtree(void*, void*);
extern "C" void appFreeUni__FPv(void*);
extern "C" void* CComShotProc_dtor(void*, int);

struct TreeNode { TreeNode* first; TreeNode* second; };
inline void freeLeaf(void*, FreeNode*);

extern "C" void* GolfBall__Fv();
extern "C" void* CQuadTree_GetInstance();
extern "C" void CQuadTree_SetSimulationMode(void*, int);
extern "C" void* InstanceTable_Get();
extern "C" float GolfParameter_ResolveFloat(void*, const char*);
extern "C" void ComShot_Log(const char*, ...);
extern "C" void CComThinkShotPos_BuildCandidateTree(void*, void*, void*, float);
extern "C" float kFloat_3_2_ComShot;
extern "C" char comShotDebugStrings[];

extern "C" void* CWind_GetInstance();
extern "C" void ShotProgressList_SetAllValuesToOne(void*);
extern "C" void __as__7WVectorFRC7WVector(void*, void*);
extern "C" void CWind_GetWindVector(void*, void*);
extern "C" int CComThinkCtrl_EvaluateCheckpoint(void*, void*, void*);
extern "C" void Matrix_SetAxisRotation(void*, float, int);
extern "C" void Matrix_TransformPoint(void*, void*, void*);
extern "C" void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern "C" unsigned char GetGroundType__9CQuadTreeF7WVector(void*, void*);extern "C" int CourseData_IsPointOutsidePlayableRegion(void*, void*);
extern "C" void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
struct SExtra { char pad_0[0xE4]; int runtimeWordE4; };
extern "C" int CRival_ChooseCurveAroundObstruction(void*, void*);
extern "C" const float kFloat_0_ComShot;
extern "C" const float kFloat_48_ComShot;
extern "C" const float kFloat_0_7853982_ComShot;
extern "C" const double kSignedIntToDoubleBias_ComShot;
extern "C" const float kFloat_30_Rival;
struct ChainNode;
void ComThinkChain_MarkAllNodes(ChainNode*);

typedef Vec3f SVec;
struct SIVec { int x, y, z; };
struct ThinkCheckpointSettings {
    char pad_0[0x1BC]; int shotFlags; float curve; float spin; unsigned char powerShotLevel; unsigned char difficultyEnabled;
    char pad[2]; int clubIndex;
};
struct ThinkCheckpoint { unsigned int nameFlags; char pad_4[4]; char* heapName; SVec targetPosition; char pad_18[4]; int priority; };
struct RivalCheckpointView {
    char pad_0[0x10]; SVec targetPosition; char pad_1C[0xC]; unsigned char powerShotLevel; char pad_29[7]; int applyPhase; char pad_34[4];
    float curve; float spin; char pad_40[0x10]; ThinkCheckpointSettings* thinkController; ThinkCheckpoint* checkpoint; char pad_58[0x4C]; int selectedClubIndex; char pad_A8[4];
    unsigned char difficultyEnabled;
};
struct BallCheckpointView { char pad_0[0x34]; float curve; float spin; char pad_3C[8]; int shotFlags; char pad_48[0x11]; unsigned char groundType; };
struct CheckpointCandidateNode { char pad_0[0x10]; ThinkCheckpoint* checkpoint; };
struct CorrectionChainLink { char pad_0[4]; CorrectionChainLink* next; int mark; };
struct ShotProcessorCheckpointView {
    char pad_0[4]; CorrectionChainLink* currentCorrectionNode; char pad_8[0x20]; int puttCandidateRejected; char pad_2C[0x60]; CorrectionChainLink* puttCorrection; CorrectionChainLink* shotCorrection;
    RivalCheckpointView* rival; char pad_98[0xC]; CheckpointCandidateNode* candidateIterator;
};

// Load checkpoint settings, test terrain, and activate putt or full-shot corrections.
void CComShotProc_ApplyCheckpointAndStartCorrection(ShotProcessorCheckpointView* self) {
    BallCheckpointView* list;
    SVec pos;
    char buf[0x180];
    // Phase 1: evaluate the current checkpoint; a rejection skips correction setup.
    self->currentCorrectionNode = 0;
    CWind_GetWindVector(&pos, CWind_GetInstance());
    ComShot_Log(((ComShotStringPool*)comShotDebugStrings)->playerBannerFormat, AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer);
    ComShot_Log(((ComShotStringPool*)comShotDebugStrings)->vectorFormat, ((ComShotStringPool*)comShotDebugStrings)->windLabel, pos.x, pos.y, pos.z);
    list = (BallCheckpointView*)GolfBall__Fv();
    ComShot_Log(((ComShotStringPool*)comShotDebugStrings)->checkStart);
    self->rival->applyPhase = 0;
    self->rival->checkpoint = self->candidateIterator->checkpoint;
    ComShot_Log(((ComShotStringPool*)comShotDebugStrings)->priorityFormat, self->rival->checkpoint->priority);
    if (!CComThinkCtrl_EvaluateCheckpoint(self->rival->thinkController, (self->rival->checkpoint->nameFlags >> 31) == 0 ? (char*)self->rival->checkpoint + 1 : self->rival->checkpoint->heapName, buf)) {
        ComShot_Log(((ComShotStringPool*)comShotDebugStrings)->checkpointRejected);
        self->puttCandidateRejected = 1;
        return;
    }
    void* receiver402_fn_800EE568 = (void*)InstanceTable_Get();
    ShotProgressList_SetAllValuesToOne(receiver402_fn_800EE568);
    // Phase 2: apply checkpoint shot options to both rival and live ball state.
    ThinkCheckpointSettings* sel = self->rival->thinkController;
    self->rival->curve = sel->curve;
    self->rival->spin = sel->spin;
    self->rival->powerShotLevel = sel->powerShotLevel;
    self->rival->difficultyEnabled = sel->difficultyEnabled;
    self->rival->selectedClubIndex = sel->clubIndex;
    list->curve = self->rival->curve;
    list->spin = self->rival->spin;
    list->shotFlags = sel->shotFlags;
    *(WVector*)(&self->rival->targetPosition) = *(WVector*)(&self->rival->checkpoint->targetPosition);
    if (kFloat_0_ComShot != self->rival->curve || CRival_ChooseCurveAroundObstruction(self->rival, &self->rival->targetPosition)) {
        if (kFloat_0_ComShot == self->rival->spin) {
            SIVec copy;
            SVec off, hit;
            SVec dir;
            float rot[12];
            SVec* origin = &self->rival->targetPosition;
            float step = kFloat_0_7853982_ComShot;
            int found;
            dir.x = kFloat_48_ComShot;
            dir.y = kFloat_0_ComShot;
            dir.z = kFloat_0_ComShot;
            // Original unit 0x801DD078: clear found only after all eight probes.
            for (unsigned i = 0; i < kRadialProbeCount || (found = 0, false); i++) {
                Matrix_SetAxisRotation(rot, step * (float)(int)i, 1);
                Matrix_TransformPoint(&off, &dir, rot);
                __pl__FRC7WVectorRC7WVector(&hit, origin, &off);
                copy = *(SIVec*)&hit;
                if ((unsigned)(GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &copy) - 8) <= 1) {
                    found = 1;
                    break;
                }
                if (CourseData_IsPointOutsidePlayableRegion(AsCGolfDoc((void*)g_sharedDoc->golfDoc)->courseData, &hit)) {
                    found = 1;
                    break;
                }
            }
            if (found) {
                self->rival->spin = -(float)((SExtra*)GolfPlayer_GetEntryByIndex((void*)g_sharedDoc->golfDoc, AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer))->runtimeWordE4 / kFloat_30_Rival;
                ((BallCheckpointView*)GolfBall__Fv())->spin = self->rival->spin;
            }
        }
        // Phase 3: select the putt or full-shot chain and mark its nodes for start.
        if (list->groundType == 2) {
            CorrectionChainLink* n = self->puttCorrection;
            self->currentCorrectionNode = n;
            n->mark = 1;
            n = n->next;
            if (n) {
                n->mark = 1;
                n = n->next;
                if (n) {
                    n->mark = 1;
                    n = n->next;
                    if (n) {
                        n->mark = 1;
                        n = n->next;
                        if (n) {
                            n->mark = 1;
                            n = n->next;
                            if (n) ComThinkChain_MarkAllNodes((ChainNode*)n);
                        }
                    }
                }
            }
        } else {
            CorrectionChainLink* n = self->shotCorrection;
            self->currentCorrectionNode = n;
            n->mark = 1;
            n = n->next;
            if (n) {
                n->mark = 1;
                n = n->next;
                if (n) {
                    n->mark = 1;
                    n = n->next;
                    if (n) {
                        n->mark = 1;
                        n = n->next;
                        if (n) {
                            n->mark = 1;
                            n = n->next;
                            if (n) ComThinkChain_MarkAllNodes((ChainNode*)n);
                        }
                    }
                }
            }
        }
    }
}

// Free a candidate node after releasing both descendant subtrees.
static void freeTreeLeaf(void* owner, TreeNode* n) {
    if (n != 0) {
        if (n->first != 0) RBTree_FreeTrivialSubtree(owner, n->first);
        if (n->second != 0) RBTree_FreeTrivialSubtree(owner, n->second);
        appFreeUni__FPv(n);
    }
}

// Free the next two candidate-tree levels with the original expanded traversal.
static void freeTreeBranch(void* owner, TreeNode* n) {
    if (n != 0) {
        freeTreeLeaf(owner, n->first);
        freeTreeLeaf(owner, n->second);
        appFreeUni__FPv(n);
    }
}

// Release a candidate root and its descendants using the expanded traversal.
static void freeTreeRoot(void* owner, TreeNode* n) {
    if (n != 0) {
        freeTreeBranch(owner, n->first);
        freeTreeBranch(owner, n->second);
        appFreeUni__FPv(n);
    }
}

#pragma use_lmw_stmw on
// Enter simulation mode and rebuild candidate targets around the ball.
// Candidate nodes own their child links at +0/+4; payloads are borrowed.
struct CandidateTreeLinks { void* left; void* right; };
// Enter simulation mode and rebuild the candidate targets around the live ball.
void CComShotProc_RebuildCandidateTree(void* self) {
    void* list = GolfBall__Fv();
    void* manager = CQuadTree_GetInstance();
    CQuadTree_SetSimulationMode(manager, 1);
    void* state = InstanceTable_Get();
    AsShotParameters(state)->powerShotLevel = 2;
    void* state2 = InstanceTable_Get();
    char* base = (char*)comShotDebugStrings;
    float scale = 3.2f * GolfParameter_ResolveFloat(state2, base + (int)&((ComShotStringPool*)0)->driverParameter);
    void* state3 = InstanceTable_Get();
    AsShotParameters(state3)->powerShotLevel = 0;
    ComShot_Log(base + (int)&((ComShotStringPool*)0)->vectorFormat, base + (int)&((ComShotStringPool*)0)->positionLabel,
                *(float*)list, *((float*)list + 1), *((float*)list + 2));
    ComShot_Log(base + (int)&((ComShotStringPool*)0)->candidateRadiusFormat, scale);

    // Discard the owned old tree before building branch fresh candidate set around the ball.
    base = (char*)AsCComShotProc(self)->candidateRoot;
    if (base != 0) {
        void* branch = ((CandidateTreeLinks*)base)->left;
        if (branch != 0) {
            void* subtree = ((CandidateTreeLinks*)branch)->left;
            if (subtree != 0) {
                void* childSubtree = ((CandidateTreeLinks*)subtree)->left;
                if (childSubtree != 0) {
                    void* descendant = ((CandidateTreeLinks*)childSubtree)->left;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    descendant = ((CandidateTreeLinks*)childSubtree)->right;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    appFreeUni__FPv(childSubtree);
                }
                childSubtree = ((CandidateTreeLinks*)subtree)->right;
                if (childSubtree != 0) {
                    void* descendant = ((CandidateTreeLinks*)childSubtree)->left;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    descendant = ((CandidateTreeLinks*)childSubtree)->right;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    appFreeUni__FPv(childSubtree);
                }
                appFreeUni__FPv(subtree);
            }
            subtree = ((CandidateTreeLinks*)branch)->right;
            if (subtree != 0) {
                void* childSubtree = ((CandidateTreeLinks*)subtree)->left;
                if (childSubtree != 0) {
                    void* descendant = ((CandidateTreeLinks*)childSubtree)->left;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    descendant = ((CandidateTreeLinks*)childSubtree)->right;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    appFreeUni__FPv(childSubtree);
                }
                childSubtree = ((CandidateTreeLinks*)subtree)->right;
                if (childSubtree != 0) {
                    void* descendant = ((CandidateTreeLinks*)childSubtree)->left;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    descendant = ((CandidateTreeLinks*)childSubtree)->right;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    appFreeUni__FPv(childSubtree);
                }
                appFreeUni__FPv(subtree);
            }
            appFreeUni__FPv(branch);
        }
        branch = ((CandidateTreeLinks*)base)->right;
        if (branch != 0) {
            void* subtree = ((CandidateTreeLinks*)branch)->left;
            if (subtree != 0) {
                void* childSubtree = ((CandidateTreeLinks*)subtree)->left;
                if (childSubtree != 0) {
                    void* descendant = ((CandidateTreeLinks*)childSubtree)->left;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    descendant = ((CandidateTreeLinks*)childSubtree)->right;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    appFreeUni__FPv(childSubtree);
                }
                childSubtree = ((CandidateTreeLinks*)subtree)->right;
                if (childSubtree != 0) {
                    void* descendant = ((CandidateTreeLinks*)childSubtree)->left;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    descendant = ((CandidateTreeLinks*)childSubtree)->right;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    appFreeUni__FPv(childSubtree);
                }
                appFreeUni__FPv(subtree);
            }
            subtree = ((CandidateTreeLinks*)branch)->right;
            if (subtree != 0) {
                void* childSubtree = ((CandidateTreeLinks*)subtree)->left;
                if (childSubtree != 0) {
                    void* descendant = ((CandidateTreeLinks*)childSubtree)->left;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    descendant = ((CandidateTreeLinks*)childSubtree)->right;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    appFreeUni__FPv(childSubtree);
                }
                childSubtree = ((CandidateTreeLinks*)subtree)->right;
                if (childSubtree != 0) {
                    void* descendant = ((CandidateTreeLinks*)childSubtree)->left;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    descendant = ((CandidateTreeLinks*)childSubtree)->right;
                    if (descendant != 0) RBTree_FreeTrivialSubtree(((char*)&AsCComShotProc(self)->candidateCount), descendant);
                    appFreeUni__FPv(childSubtree);
                }
                appFreeUni__FPv(subtree);
            }
            appFreeUni__FPv(branch);
        }
        appFreeUni__FPv(base);
        AsCComShotProc(self)->candidateCount = 0;
        AsCComShotProc(self)->candidateRoot = 0;
        AsCComShotProc(self)->firstCandidate = (char*)&AsCComShotProc(self)->candidateRoot;
    }
    // The first-candidate iterator refers into the newly built tree, or its sentinel.
    void* controller = AsCComShotProc(self)->owner;
    CComThinkShotPos_BuildCandidateTree(AsCRival(controller)->shotPositionThinker, ((char*)&AsCComShotProc(self)->candidateCount), list, scale);
    AsCComShotProc(self)->shotCandidateRejected = 1;
    AsCComShotProc(self)->candidateIterator = AsCComShotProc(self)->firstCandidate;
    AsCComShotProc(self)->puttCandidateRejected = 1;
    AsCComShotProc(self)->puttRemainingDistance = 1000.0f;
}

struct DVirt { 
virtual ~DVirt();
};
struct DTree { void* head; TreeNode* root; void* tail; };
struct DObj {
    void* vt; char pad0[0x8C - 4]; DVirt* o1; DVirt* o2; char pad1[4]; DTree tree;
};
// Free a candidate leaf and the subtrees owned below it.
inline void dLeaf(DObj* t, FreeNode* n) {
    if (n) {
        if (n->first) RBTree_FreeTrivialSubtree(&t->tree, n->first);
        if (n->second) RBTree_FreeTrivialSubtree(&t->tree, n->second);
        appFreeUni__FPv(n);
    }
}
// Release the middle level of the owned candidate tree.
inline void dMid(DObj* t, FreeNode* n) {
    if (n) {
        dLeaf(t, (FreeNode*)n->first);
        dLeaf(t, (FreeNode*)n->second);
        appFreeUni__FPv(n);
    }
}
// Release the top level of the owned candidate tree.
inline void dTop(DObj* t, FreeNode* n) {
    if (n) {
        dMid(t, (FreeNode*)n->first);
        dMid(t, (FreeNode*)n->second);
        appFreeUni__FPv(n);
    }
}
#pragma use_lmw_stmw on
// Destroy both correction chains and all owned candidate nodes.
void* CComShotProc_dtor(void* self_, int deleting) {
    DObj* self = (DObj*)self_;
    if (self) {

        DVirt* o = self->o1; self->vt = (void*)((char*)&__vt__12CComShotProc);
        delete o;
        o = self->o2;
        delete o;
        // Embedded-tree ABI guards are repeated by the original destructor.
        // Keep each test: CodeWarrior emits them as distinct instructions.
        if (&self->tree)
        if (&self->tree)
        if (&self->tree)
        if (&self->tree)
        if (&self->tree)
        if (&self->tree) {
            TreeNode* root = self->tree.root;
            if (root) {
            __typeof__(root->first) savedFirst = root->first; dTop(self, (FreeNode*)savedFirst);
                dTop(self, (FreeNode*)root->second);
                appFreeUni__FPv(root);
            }
        }
        if (0 < deleting) __dl__FPv(self);
    }
    return self;
}

// Release descendant candidate nodes before freeing their parent.
inline void freeLeaf(void* owner, FreeNode* n) {
    if (!n) return;
    if (n->first) RBTree_FreeTrivialSubtree(owner, n->first);
    if (n->second) RBTree_FreeTrivialSubtree(owner, n->second);
    appFreeUni__FPv(n);
}

#pragma use_lmw_stmw on
// Destroy the candidate-tree subobject and optionally free its containing storage.
void* dtor_801DD7C4(void* self, int deleting) {
    if (!self) return self;
    FreeNode* a = *(FreeNode**)((char*)self + 4);
    FreeNode* b;
    FreeNode* c;
    if (a) {
        b = a->first;
        if (b) {
            c = b->first;
            if (c) { freeLeaf(self, c->first); freeLeaf(self, c->second); appFreeUni__FPv(c); }
            c = (FreeNode*)*((void**)b + 1);
            if (c) { freeLeaf(self, c->first); freeLeaf(self, c->second); appFreeUni__FPv(c); }
            appFreeUni__FPv(b);
        }
        b = (FreeNode*)*((void**)a + 1);
        if (b) {
            c = b->first;
            if (c) { freeLeaf(self, c->first); freeLeaf(self, c->second); appFreeUni__FPv(c); }
            c = (FreeNode*)*((void**)b + 1);
            if (c) { freeLeaf(self, c->first); freeLeaf(self, c->second); appFreeUni__FPv(c); }
            appFreeUni__FPv(b);
        }
        appFreeUni__FPv(a);
    }
    if (deleting > 0) __dl__FPv(self);
    return self;
}

struct ChainNode {
    char pad00[4];
    ChainNode* next;
    int mark;
};

void ComThinkChain_MarkAllNodes(ChainNode*);

// Mark every linked correction task ready to run.
void ComThinkChain_MarkAllNodes(ChainNode* self) {
    self->mark = 1;
    ChainNode* n1 = self->next;
    if (n1 == 0) return;
    n1->mark = 1;
    ChainNode* n2 = n1->next;
    if (n2 == 0) return;
    n2->mark = 1;
    ChainNode* n3 = n2->next;
    if (n3 == 0) return;
    n3->mark = 1;
    ChainNode* n4 = n3->next;
    if (n4 == 0) return;
    n4->mark = 1;
    ChainNode* n5 = n4->next;
    if (n5 == 0) return;
    ComThinkChain_MarkAllNodes(n5);
}

struct Child {
    void** vtable;
};
struct PairChild {
    virtual void _dtor(int);
};

struct PairObject {
    void** vtable;
    char pad04[0x18];
    Child* first;
    Child* second;
};

struct MidNode { void** vtable; MidNode* next; MidNode* child; void* correctionConfig; void* owner; char pad14[0x4C]; };
struct LeafNode { void** vtable; LeafNode* next; LeafNode* child; void* correctionConfig; void* owner; char pad14[0x4C]; int enabled; };

struct RawPair { char pad[0x24]; };
struct RawLeaf { char pad[0x64]; };
struct RawMid { char pad[0x68]; };
extern "C" char __vt__12CComShotProc[];
extern "C" char __vt__15CComPadOnePoint[];
extern "C" char __vt__23CComShotAngleCorrection[];
extern "C" char __vt__23CComShotPowerCorrection[];
extern "C" char __vt__16CComShotOnePoint[];

// Create the putt/full-shot chains, each with angle and power correction children.
void* CComShotProc_Ctor(void* self, void* owner) {
    char* p=(char*)self;
    AsCComShotProc(p)->currentCorrectionNode=0;
    AsCComShotProc(p)->taskState=0;
    AsCComShotProc(p)->vtable=(void*)__vt__12CComShotProc;
    AsCComShotProc(p)->owner=owner;
    AsCComShotProc(p)->candidateCount=0;
    AsCComShotProc(p)->candidateRoot=0;
    AsCComShotProc(p)->firstCandidate=p+0x9C;
    // Build the owned putt task and its two correction children; owner links borrow the rival.
    RawPair* a=new RawPair;
    if(a) {
        void* o0=AsCComShotProc(p)->owner;
        ((CComOnePointState*)a)->next=0;
        ((CComOnePointState*)a)->child=0;
        ((CComOnePointState*)a)->vtable=(void*)__vt__15CComPadOnePoint;
        ((CComOnePointState*)a)->owner=o0;
        RawLeaf* c=new RawLeaf;
        if(c) {
            void* o1=((CComOnePointState*)a)->owner;
            ((CComAngleCorrectionState*)c)->next=0;
            ((CComAngleCorrectionState*)c)->child=0;
            ((CComAngleCorrectionState*)c)->vtable=(void*)__vt__23CComShotAngleCorrection;
            ((CComAngleCorrectionState*)c)->state=0;
            ((CComAngleCorrectionState*)c)->owner=o1;
            ((CComAngleCorrectionState*)c)->enabled=1;
        }
        ((CComOnePointState*)a)->angleCorrection=c;
        RawMid* d=new RawMid;
        if(d) {
            void* o2=((CComOnePointState*)a)->owner;
            ((CComPowerCorrectionState*)d)->next=0;
            ((CComPowerCorrectionState*)d)->child=0;
            ((CComPowerCorrectionState*)d)->vtable=(void*)__vt__23CComShotPowerCorrection;
            ((CComPowerCorrectionState*)d)->state=0;
            ((CComPowerCorrectionState*)d)->owner=o2;
            ((CComPowerCorrectionState*)d)->enabled=1;
        }
        ((CComOnePointState*)a)->powerCorrection=d;
        ((CComOnePointState*)a)->shotProcessorState=p+0xC; // Link the pad correction helper to the shot processor's +0x0C state.
    }
    AsCComShotProc(p)->padOnePoint=a;
    // Build the equivalent full-shot task; both task payloads refer back into self.
    RawPair* b=new RawPair;
    if(b) {
        void* o0=AsCComShotProc(p)->owner;
        ((CComOnePointState*)b)->next=0;
        ((CComOnePointState*)b)->child=0;
        ((CComOnePointState*)b)->vtable=(void*)__vt__16CComShotOnePoint;
        ((CComOnePointState*)b)->owner=o0;
        RawLeaf* c=new RawLeaf;
        if(c) {
            void* o1=((CComOnePointState*)b)->owner;
            ((CComAngleCorrectionState*)c)->next=0;
            ((CComAngleCorrectionState*)c)->child=0;
            ((CComAngleCorrectionState*)c)->vtable=(void*)__vt__23CComShotAngleCorrection;
            ((CComAngleCorrectionState*)c)->state=0;
            ((CComAngleCorrectionState*)c)->owner=o1;
            ((CComAngleCorrectionState*)c)->enabled=1;
        }
        ((CComOnePointState*)b)->angleCorrection=c;
        RawMid* d=new RawMid;
        if(d) {
            void* o2=((CComOnePointState*)b)->owner;
            ((CComPowerCorrectionState*)d)->next=0;
            ((CComPowerCorrectionState*)d)->child=0;
            ((CComPowerCorrectionState*)d)->vtable=(void*)__vt__23CComShotPowerCorrection;
            ((CComPowerCorrectionState*)d)->state=0;
            ((CComPowerCorrectionState*)d)->owner=o2;
            ((CComPowerCorrectionState*)d)->enabled=1;
        }
        ((CComOnePointState*)b)->powerCorrection=d;
        ((CComOnePointState*)b)->unk_C=p+0xC;
    }
    AsCComShotProc(p)->shotOnePoint=b;
    return self;
}

extern "C" char __vt__15CComPadOnePoint[];
extern "C" char __vt__16CComShotOnePoint[];
// Destroy the putt angle/power correction children and optionally delete this helper.
void* CComPadOnePoint_dtor(PairObject* self, int deleting) {
    if (self != 0) {
        self->vtable = (void**)__vt__15CComPadOnePoint;
        if (self->first != 0) ((PairChild*)self->first)->_dtor(1);
        if (self->second != 0) ((PairChild*)self->second)->_dtor(1);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Destroy the full-shot angle/power correction children and optionally delete this helper.
void* CComShotOnePoint_dtor(PairObject* self, int deleting) {
    if (self != 0) {
        self->vtable = (void**)__vt__16CComShotOnePoint;
        if (self->first != 0) ((PairChild*)self->first)->_dtor(1);
        if (self->second != 0) ((PairChild*)self->second)->_dtor(1);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

} // extern "C"
} // namespace ns0

// ---- from com_shot_cpu_rival_ai.cpp ----
namespace ns1 {
extern "C" {

void* CQuadTree_GetInstance();
void BaseObject_ctor(void*);
void CQuadTree_SetSimulationMode(void*, int);
void BaseObject_dtor(void*, int);
void __dl__FPv(void*);
void* __nw__FUl(unsigned int);
void CComThinkShotPos_Ctor(void*);
void CComStrokeUse_Ctor(void*);
void CComRandomUse_Ctor(void*);
void CComUnplayable_Ctor(void*, void*, void*);
void CComShotProc_Ctor(void*, void*);
void CComThinkCtrl_Ctor(void*);
extern void* g_rival;
extern char s_rivalBaseVtable[];
extern char __vt__6CRival[];
extern void* g_taskManager;
extern const float kFloat_0_ComShot;
extern float s_comShotReferencePowerRatio; // Actually the 1.0 normalized-power reference; shared name retained.
extern const float kFloat_0_00001_ComShot;
extern const float kFloat_360_ComShot;
extern const float kFloat_149_ComShot;
extern const float kFloat_2_ComShot;
extern CSharedDoc* g_sharedDoc;
float ShotParameters_GetGoodImpactHalfWidth(void*);
float ShotParameters_GetPoorImpactHalfWidth(void*);
void* InstanceTable_Get();
void ClubSelection_SelectForCpuTarget(void*, void*);
int CComThinkCtrl_GetOption(void*, int);
void ComShot_SimulateTrajectory(void*, void*, void*);
void* GolfBall__Fv();
int GolfTips_IsDialogActive();
void* Object_GetChild(void*);
void* TaskActorTable_FindByName(void*, int, int, int);
void ClubSelection_SetClubIndex(void*, unsigned char);
void CClub_SelectBestClubForHole(void*);
void ShotProgressList_SetAllValuesToOne(void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float WVectorLen__FRC7WVector(void*);
void __ml__FfRC7WVector(void*, void*, float);
void Matrix_TransformPoint(void*, void*, void*);
void Matrix_SetAxisRotation(void*, float, int);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void __as__7WVectorFRC7WVector(void*, void*);
void ComShot_Log(const char*, ...);
void ComShot_EvaluateCandidate(void*, void*);
int GolfBall_IsGroundTypeOutsideZeroToTwo(void*);
void CComUnplayable_ResetToIdle(void*);
int CComUnplayable_BeginDropSequence(void*);
float CComRandomUse_AdjustValue(void*, int, int, float);
void CRival_PrepareShotGauge(void*, float, float);
float CQuadTree_FindObstructionSweepFraction(void*, void*, void*);
void* GolfPlayer_GetEntryByIndex(void*, int);
extern char g_comShotCurveProbePosition[0xC];
extern const float kFloat_192_ComShot;
extern const float kFloat_0_5_ComShot;
extern const float kFloat_100_0_ComShot;
extern const float kFloat_1_Rival;
extern const double kSignedIntToDoubleBias_ComShot;
extern const float kFloat_30_Rival;
extern const float kFloat_0_8_Rival;
extern const float kFloat_25_6_ComShot;
extern const float kFloat_0_001_Rival;
extern const char comShotDebugStrings[];
extern const int courseObjectRankTable[];

struct ObjDE {
    char pad[0x100];
};

// Construction wrappers keep their ABI names because exception tables reference them.
// A/B/C: shot-position, stroke, and random-use helpers; D/E/F: unplayable, processor, and thinker.
struct A_DE { char p[0x20]; A_DE(); };
struct B_DE { char p[0x2e8]; B_DE(); };
struct C_DE { char p[0x14]; C_DE(); };
struct D_DE { char p[0x20]; D_DE(void*, void*); };
struct E_DE { char p[0xa8]; E_DE(void*); };
struct F_DE { char p[0x1d0]; F_DE(); };
struct DtorBaseDE { 
virtual void _dtor(int);
};

// Construct the owned shot-position thinker.
A_DE::A_DE() { CComThinkShotPos_Ctor(this); }
// Construct the owned rival stroke-history helper.
B_DE::B_DE() { CComStrokeUse_Ctor(this); }
// Construct the owned shot-error randomizer.
C_DE::C_DE() { CComRandomUse_Ctor(this); }
// Construct the owned unplayable-drop helper with borrowed owner/context.
D_DE::D_DE(void* owner, void* a) { CComUnplayable_Ctor(this, owner, a); }
// Construct the owned shot processor with its borrowed rival owner.
E_DE::E_DE(void* owner) { CComShotProc_Ctor(this, owner); }
// Construct the owned checkpoint rule evaluator.
F_DE::F_DE() { CComThinkCtrl_Ctor(this); }

struct GameDataDE {
    char pad[0x2C4];
    float shotPowerGaugeX;
    float shotImpactGaugeX;
    float shotGaugeCursorX;
    float roundedShotGaugeCursorX;
    int impactGrade;
    unsigned char pangyaAssistFlag;
    char pad2[3];
    int shotGaugeDirection;
    float pangyaTargetX;
    float calipersGaugeX;
    float pangyaZoneScale;
    int gaugeWord2EC;
    float gaugeFloat2F0;
    int gaugeWord2F4;
    int pangyaAssistState;
    int unassistedImpactWords[2];
    int gaugeWord304;
};

void* dtor_801DEB54(void*, int);

// Create the rival singleton and its thinking, correction, and unplayable-shot helpers.
void* CRival_Ctor(void* self) {
    BaseObject_ctor(self);
    *(void**)self = s_rivalBaseVtable;
    g_rival = self;
    *(void**)self = __vt__6CRival;
    char* p = (char*)self;
    AsCRival(p)->powerShotLevel = 0;
    AsCRival(p)->shotCurve = kFloat_0_ComShot;
    AsCRival(p)->shotSpin = kFloat_0_ComShot;
    AsCRival(p)->savedClubIndex = kInvalidClub;
    AsCRival(p)->selectedClubIndex = kInvalidClub;
    AsCRival(p)->difficultyEnabled = 1;
    try {
    void* a = new A_DE;
    AsCRival(p)->shotPositionThinker = a;
    void* b = new B_DE;
    AsCRival(p)->strokeUse = b;
    void* c = new C_DE;
    AsCRival(p)->randomUse = c;
    void* d = new D_DE(self, AsCRival(p)->shotPositionThinker);
    AsCRival(p)->unplayableHandler = d;
    void* e = new E_DE(self);
    AsCRival(p)->shotProcessor = e;
    void* f = new F_DE;
    AsCRival(p)->thinkController = f;
    return self;
    } catch (...) {
        dtor_801DEB54(self, -1);
        throw;
    }
}

// Release all rival helpers and clear the singleton before optional deletion.
void* CRival_dtor(void* self, int flags) {
    if (self) {
        char* p = (char*)self;
        *(void**)p = __vt__6CRival;
        {
            void* obj = AsCRival(p)->unplayableHandler;
            if (obj) ((DtorBaseDE*)obj)->_dtor(1);
        }
        {
            void* obj = AsCRival(p)->thinkController;
            if (obj) ((DtorBaseDE*)obj)->_dtor(1);
        }
        {
            void* obj = AsCRival(p)->shotProcessor;
            if (obj) ((DtorBaseDE*)obj)->_dtor(1);
        }
        {
            void* obj = AsCRival(p)->shotPositionThinker;
            if (obj) ((DtorBaseDE*)obj)->_dtor(1);
        }
        {
            void* obj = AsCRival(p)->strokeUse;
            if (obj) ((DtorBaseDE*)obj)->_dtor(1);
        }
        {
            void* obj = AsCRival(p)->randomUse;
            if (obj) ((DtorBaseDE*)obj)->_dtor(1);
        }
        if (self) {
            g_rival = 0;
            BaseObject_dtor(self, 0);
        }
        if (flags > 0) __dl__FPv(self);
    }
    return self;
}

// normalizedPower maps to gauge X as 360*power+149; impactOffset is an X offset
// from pangyaTargetX. This also mutates the live round gauge and clears special
// shot flags for non-Pangya impact before caching the prepared gauge in the rival.
// Convert normalized power and impact error into a cached shot-gauge snapshot.
void CRival_PrepareShotGauge(void* self, float normalizedPower, float impactOffset) {
    char* out = (char*)self;
    GameDataDE* g = ((GameDataDE*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    g->pangyaAssistState = 0;
    float diff = normalizedPower - s_comShotReferencePowerRatio;
    {
        float savedDiff = diff;

        if ((*(int*)&(savedDiff)) & 0x80000000) diff = -diff;
    }
        __typeof__(kFloat_0_00001_ComShot > diff) powerAtReference = (diff < kFloat_0_00001_ComShot);
    if (powerAtReference) g->pangyaAssistFlag = 1;
    g->shotPowerGaugeX = FP_FMADDS(kFloat_360_ComShot, normalizedPower, kFloat_149_ComShot);
    g->shotImpactGaugeX = (g->pangyaTargetX + impactOffset);
    g->impactGrade = kImpactMiss;
    float amount = g->shotImpactGaugeX - g->pangyaTargetX;
    amount = amount > kFloat_0_ComShot ? amount : -amount;
    if (amount < kFloat_2_ComShot) {
        g->impactGrade = kImpactPangya;
        g->pangyaAssistState = 2;
    } else {
        if (amount < ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get())) g->impactGrade = kImpactGood;
        else {
            if (amount < ShotParameters_GetPoorImpactHalfWidth(InstanceTable_Get())) g->impactGrade = kImpactPoor;
        }
    }
    if (g->impactGrade != kImpactPangya) {
        void* x = GolfBall__Fv();
        int* p = &AsGolfBall(x)->m_powerFlags;
        *p &= ~kSpecialShotMask;
    }
    AsCRival(out)->shotPowerGaugeX = g->shotPowerGaugeX;
    AsCRival(out)->shotImpactGaugeX = g->shotImpactGaugeX;
    AsCRival(out)->shotGaugeCursorX = g->shotGaugeCursorX;
    AsCRival(out)->roundedShotGaugeCursorX = g->roundedShotGaugeCursorX;
    AsCRival(out)->impactGrade = g->impactGrade;
    AsCRival(out)->preparedPangyaAssistFlag = g->pangyaAssistFlag;
    AsCRival(out)->shotGaugeDirection = g->shotGaugeDirection;
    AsCRival(out)->pangyaTargetX = g->pangyaTargetX;
    AsCRival(out)->calipersGaugeX = g->calipersGaugeX;
    AsCRival(out)->pangyaZoneScale = g->pangyaZoneScale;
    AsCRival(out)->impactTimingSpriteIndex = g->gaugeWord2EC;
    AsCRival(out)->impactFeedbackGaugeX = g->gaugeFloat2F0;
    AsCRival(out)->powerShotBonusPang = g->gaugeWord2F4;
    enum AssistWord { kAssistWordMin = 0, kAssistWordMax = 0xFFFFFFFFu };
    AssistWord tail;
    AssistWord state;
    state = (AssistWord)g->pangyaAssistState;
    tail = (AssistWord)g->unassistedImpactWords[0];
    AsCRival(out)->pangyaAssistState = state;
    AsCRival(out)->preparedUnassistedImpactGaugeBits = tail;
    AsCRival(out)->unassistedImpactGrade = g->unassistedImpactWords[1];
}

// Return the rival rule/checkpoint evaluator.
void* CRival_GetThinkController(void* self) {
    return AsCRival(self)->thinkController;
}

// Wait for task/UI readiness, then apply club, curve, spin, and the cached shot gauge.
int CRival_ApplyPreparedShot(void* self) {
    // Phase 1: wait for thinking/dialogs, then advance the actor-request handshake.
    void* state = GolfBall__Fv();
    if (*(int*)&AsCComShotProc(AsCRival(self)->shotProcessor)->taskState != 0) return 0;
    if (GolfTips_IsDialogActive() != 0) return 0;
    int mode = AsCRival(self)->shotApplyPhase;
    switch (mode) {
    case 0:
        break;
    case 1: {
        void* x = g_taskManager ? Object_GetChild(g_taskManager) : 0;
        int zero = 0;
        int id = 0x37;
        int args[8];
        args[0] = zero;
        args[1] = id;
        args[2] = 1;
        args[3] = zero;
        args[4] = zero;
        args[5] = zero;
        args[6] = zero;
        args[7] = zero;
        void* layout = TaskActorTable_FindByName(x, (int)((char*)((ComShotStringPool*)comShotDebugStrings)->screenActor), id, zero);
        if (layout) ((void (*)(void*, void*))(*(void***)layout)[11])(layout, args);
        AsCRival(self)->shotApplyPhase = 2;
        return 0;
    }
    case 2: {
        void* x = g_taskManager ? Object_GetChild(g_taskManager) : 0;
        int index;
        int zero = 0;
        int id = 0x15;
        int args[8];
        args[0] = zero;
        args[1] = id;
        args[2] = (int)&index;
        args[3] = zero;
        args[4] = zero;
        args[5] = zero;
        args[6] = zero;
        args[7] = zero;
        void* layout = TaskActorTable_FindByName(x, (int)comShotDebugStrings, id, zero);
        if (layout) ((void (*)(void*, void*))(*(void***)layout)[11])(layout, args);
        if (index == 3) return 0;
        break;
    }
    }
    // Phase 2: select the club and publish curve/spin (the ball calls them lie fields).
    int choice = AsCRival(self)->selectedClubIndex;
    if (choice != -1) {
        ClubSelection_SetClubIndex(InstanceTable_Get(), (unsigned char)choice);
    } else {
        CClub_SelectBestClubForHole(InstanceTable_Get());
    }
    AsCRival(self)->shotApplyPhase = 0;
    AsGolfBall(state)->m_initCurve = AsCRival(self)->shotCurve;
    AsGolfBall(state)->m_initSpin = AsCRival(self)->shotSpin;
    if (AsCRival(self)->impactGrade != 4) {
        void* game = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        void* info = GolfPlayer_GetEntryByIndex(game, AsCGolfDoc(game)->currentPlayer);
        float curve = AsGolfBall(state)->m_initCurve;
        float maximumCurve = (float)((PlayerCurveStatView*)info)->curve / kFloat_30_Rival;
        AsGolfBall(state)->m_initCurve = curve < -maximumCurve ? -maximumCurve : (curve > maximumCurve ? maximumCurve : curve);
    }
    if (AsCRival(self)->shotPowerRatio < kFloat_0_8_Rival) {
        int* flags = (int*)((char*)GolfBall__Fv() + 0x44);
        *flags &= ~kSpecialShotMask;
    }
    signed char flag = *(signed char*)&AsCRival(self)->powerShotLevel;
    AsShotParameters(InstanceTable_Get())->powerShotLevel = flag;
    void* receiver1274_fn_800EE568 = (void*)InstanceTable_Get();
    ShotProgressList_SetAllValuesToOne(receiver1274_fn_800EE568);
    char* game = (char*)((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    // Phase 3: restore the cached gauge values into the live round.
    AsCGolfDoc(game)->shotPowerGaugeX = AsCRival(self)->shotPowerGaugeX;
    AsCGolfDoc(game)->shotImpactGaugeX = AsCRival(self)->shotImpactGaugeX;
    AsCGolfDoc(game)->shotGaugeCursorX = AsCRival(self)->shotGaugeCursorX;
    AsCGolfDoc(game)->roundedShotGaugeCursorX = AsCRival(self)->roundedShotGaugeCursorX;
    AsCGolfDoc(game)->impactGrade = AsCRival(self)->impactGrade;
    game[0x2d8] = ((char*)self)[0x6c];
    AsCGolfDoc(game)->shotGaugeDirection = AsCRival(self)->shotGaugeDirection;
    AsCGolfDoc(game)->pangyaTargetX = AsCRival(self)->pangyaTargetX;
    AsCGolfDoc(game)->calipersGaugeX = AsCRival(self)->calipersGaugeX;
    AsCGolfDoc(game)->pangyaZoneScale = AsCRival(self)->pangyaZoneScale;
    AsCGolfDoc(game)->impactTimingSpriteIndex = AsCRival(self)->impactTimingSpriteIndex;
    AsCGolfDoc(game)->impactFeedbackGaugeX = AsCRival(self)->impactFeedbackGaugeX;
    AsCGolfDoc(game)->powerShotBonusPang = AsCRival(self)->powerShotBonusPang;
    AsCGolfDoc(game)->pangyaAssistState = AsCRival(self)->pangyaAssistState;
    *(int*)&AsCGolfDoc(game)->unassistedImpactGaugeX = AsCRival(self)->preparedUnassistedImpactGaugeBits;
    AsCGolfDoc(game)->unassistedImpactGrade = AsCRival(self)->unassistedImpactGrade;
    AsCRival(self)->savedClubIndex = kInvalidClub;
    AsCRival(self)->selectedClubIndex = kInvalidClub;
    return 1;
}

// Advance the candidate iterator and use the best shot or unplayable fallback when exhausted.
int CComShotProc_NextCandidateOrUseFallback(void* self) {
    int low;
    int high;
    int param;
    float upperLimit;
    float diff;
    // Phase 1: find the in-order successor; parent links carry a low-bit tree tag.
    void* node = AsCComShotProc(self)->candidateIterator;
    node = *(void**)((char*)node + 4);
    if (node != 0) {
        while (*(void**)node != 0)
            node = *(void**)node;
        AsCComShotProc(self)->candidateIterator = node;
    } else {
        void* parent;
        while ((node = AsCComShotProc(self)->candidateIterator) != *(void**)(parent = (void*)(*(unsigned int*)((char*)node + 8) & ~1u))) {
            AsCComShotProc(self)->candidateIterator = parent;
        }
        AsCComShotProc(self)->candidateIterator = parent;
    }
    // A valid candidate ends iteration; exhaustion uses the best fallback or a drop.
    if (AsCComShotProc(self)->puttCandidateRejected == 0)
        return 0;
    if (AsCComShotProc(self)->candidateIterator == ((char*)&AsCComShotProc(self)->candidateRoot)) {
    if (AsCComShotProc(self)->shotCandidateRejected == 0) {
        ComShot_Log(((ComShotStringPool*)comShotDebugStrings)->noMoreCandidates);
        ComShot_EvaluateCandidate(AsCComShotProc(self)->owner, &((ShotProcessorBestView*)self)->bestCandidate);
        return 0;
    }
    // Phase 2: with no usable candidate, prepare a half-power fallback or request a drop.
    void* state = AsCComShotProc(self)->owner;
    void* sub = AsCRival(state)->unplayableHandler;
    if (GolfBall_IsGroundTypeOutsideZeroToTwo(sub) == 0) {
        ComShot_Log(((ComShotStringPool*)comShotDebugStrings)->unplayableFallback);
        CComUnplayable_ResetToIdle(AsCRival(AsCComShotProc(self)->owner)->unplayableHandler);
        void* game = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        unsigned char index = AsCGolfDoc(game)->currentHoleIndex;
        void* entries = AsCGolfDoc(game)->holeRecords;
        void* entry = (char*)entries + index * 32;
        struct ProcVec { int x, y, z; };
        ProcVec vec = *(ProcVec*)&((CourseHoleView*)entry)->position;
        *(WVector*)AsCRival(AsCComShotProc(self)->owner)->holePosition = *(WVector*)&vec;
        AsCRival(AsCComShotProc(self)->owner)->shotPowerRatio = kFloat_0_5_ComShot;
        AsCRival(AsCComShotProc(self)->owner)->shotCurve = kFloat_0_ComShot;
        AsCRival(AsCComShotProc(self)->owner)->shotSpin = kFloat_0_ComShot;
        game = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        unsigned char a = AsCGolfDoc(game)->currentHoleIndex;
        unsigned char b = AsCGolfDoc(game)->currentPlayer;
        unsigned int selector = *((unsigned char*)g_sharedDoc + a + 0x83D);
        const char* row = (const char*)(selector + (unsigned int)g_sharedDoc);
        int offset = b * 0x1D4;
        param = *(signed char*)(row + offset + 0x1A4);
        low = kNormalPowerRandomIndex;
        high = kNormalImpactRandomIndex;
        void* modeState = GolfBall__Fv();
        if (AsGolfBall(modeState)->m_powerFlags != 0) {
            low = kSpecialPowerRandomIndex; high = kSpecialImpactRandomIndex;
        } else if (*(int*)(*(char**)((char*)AsCComShotProc(self)->owner + 0x54) + 0x1C) == -1) {
            low = kNoTargetPowerRandomIndex; high = kNoTargetImpactRandomIndex;
        }
        float rate = CComRandomUse_AdjustValue(AsCRival(AsCComShotProc(self)->owner)->randomUse, param, low, AsCRival(AsCComShotProc(self)->owner)->shotPowerRatio);
        AsCRival(AsCComShotProc(self)->owner)->shotPowerRatio = rate;
        float upper = CComRandomUse_AdjustValue(AsCRival(AsCComShotProc(self)->owner)->randomUse, param, high, kFloat_100_0_ComShot);
        diff = upper - kFloat_100_0_ComShot;
        float powerRatio;
        float clampedPower;
        powerRatio = AsCRival(AsCComShotProc(self)->owner)->shotPowerRatio;
        clampedPower = kFloat_1_Rival;
        clampedPower = powerRatio > clampedPower ? clampedPower : powerRatio;
        AsCRival(AsCComShotProc(self)->owner)->shotPowerRatio = clampedPower;
        upperLimit = ShotParameters_GetPoorImpactHalfWidth(InstanceTable_Get()) - kFloat_0_00001_ComShot;
        float lowerLimit = -(ShotParameters_GetPoorImpactHalfWidth(InstanceTable_Get()) - kFloat_0_00001_ComShot);
        float delta;
        if (diff < lowerLimit) delta = lowerLimit;
        else delta = diff > upperLimit ? upperLimit : diff;
        CRival_PrepareShotGauge(AsCComShotProc(self)->owner, AsCRival(AsCComShotProc(self)->owner)->shotPowerRatio, delta);
        return 0;
    }
    int r = CComUnplayable_BeginDropSequence(AsCRival(AsCComShotProc(self)->owner)->unplayableHandler);
    if (r != 0) return 0;
    }
    return 1;
}

// a/b are borrowed 0x40-byte candidate payloads; return aliases one of them.
// compareValidity==0 compares only remaining distance after the optional hole
// exclusion. Otherwise prefer valid -> inside target -> holed/fewer frames ->
// optional terrain rank -> away from hazards -> shorter distance. Exact distance
// ties choose b. The difficulty exclusion can reject a holed candidate first.
// Signed word projection preserves the comparison widths of the original selector;
// the member supplies the offset, so candidate layout changes remain reviewable.
// Borrow a word with the selector's signed interpretation; do not narrow flags to bytes.
static inline int CandidateSignedWordValue(const unsigned int& word) { return *(const int*)&word; }

enum CandidatePairPresence {
    kNeitherCandidate = 0, kOnlyCandidateB = 1,
    kOnlyCandidateA = 2, kBothCandidates = 3
};
// First decisive row wins. Equal fields continue unless the row states a tie result:
// difficulty exclusion: holed B -> A, otherwise holed A -> B (even when both holed)
// rejected:            only A -> B; only B -> A
// finalWithinTarget:   only A -> A; only B -> B
// both inside target:  prefer holed; both holed -> fewer signed frames (tie A)
// terrain rank:        larger rank wins; equal ranks continue
// nearHazard:          only A -> B; only B -> A
// remainingDistance:   smaller wins; exact tie or unordered float comparison -> B
// Compare two simulated shots by validity, target flags, terrain rank, and distance.
void* ComShot_SelectPreferredCorrectionCandidate(void* context, void* a, void* b, int compareTerrainRank, int applyDifficultyHoleExclusion, int compareValidity) {
    ShotCandidateLayout* candidateA = (ShotCandidateLayout*)a;
    ShotCandidateLayout* candidateB = (ShotCandidateLayout*)b;
    // Phase 1: the optional difficulty rule can discard a holed shot before all comparisons.
    if (applyDifficultyHoleExclusion) {
        void* game = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        unsigned char index = AsCGolfDoc(game)->currentPlayer;
        SharedCpuCharacterRecord* entry = (SharedCpuCharacterRecord*)g_sharedDoc + index;
        if (entry->characters[0xE] < 4) {
            if (CandidateSignedWordValue(candidateB->holed) != 0) return a;
            if (CandidateSignedWordValue(candidateA->holed) != 0) return b;
        }
    }
    // Pair masks throughout: bit 1 means A's field is set, bit 0 means B's is set.
    // First prefer an unrejected candidate (+0x1C); distance-only mode returns here.
    if (compareValidity) {
        int otherPreferenceFlag = CandidateSignedWordValue(candidateB->rejected);
        int flags = kNeitherCandidate;
        if (CandidateSignedWordValue(candidateA->rejected)) flags |= kOnlyCandidateA;
        if (otherPreferenceFlag) flags |= kOnlyCandidateB;
        switch (flags) {
        case kOnlyCandidateA:
            return b;
        case kOnlyCandidateB:
            return a;
        case kNeitherCandidate:
        case kBothCandidates:
            break;
        default:
            break;
        }
    } else {
        if (((ShotCandidateLayout*)candidateA)->remainingDistance < ((ShotCandidateLayout*)candidateB)->remainingDistance) return a;
        return b;
    }
    // Phase 2: prefer finalWithinTarget (+0x2C). If both, prefer holed (+0x38),
    // then fewer simulation frames (+0x3C); equal frame counts select A.
    int otherPreferenceFlag = CandidateSignedWordValue(candidateB->finalWithinTarget);
    int flags = kNeitherCandidate;
    if (CandidateSignedWordValue(candidateA->finalWithinTarget)) flags |= kOnlyCandidateA;
    if (otherPreferenceFlag) flags |= kOnlyCandidateB;
    switch (flags) {
    case kNeitherCandidate:
    case kBothCandidates:
        if (flags == kBothCandidates) {
        int otherHoled = CandidateSignedWordValue(candidateB->holed);
        int holedPair = kNeitherCandidate;
        if (CandidateSignedWordValue(candidateA->holed)) holedPair |= kOnlyCandidateA;
        if (otherHoled) holedPair |= kOnlyCandidateB;
        switch (holedPair) {
        case kBothCandidates:
            return CandidateSignedWordValue(candidateA->simulationFrames) > CandidateSignedWordValue(candidateB->simulationFrames) ? b : a;
        case kOnlyCandidateA:
            return a;
        case kNeitherCandidate:
            break;
        case kOnlyCandidateB:
            return b;
        default:
            break;
        }
        }
        break;
    case kOnlyCandidateA:
        return a;
    case kOnlyCandidateB:
        return b;
    default:
        return 0;
    }
    // Phase 3: optionally compare the ground-type (+0x20) table's terrain ranks.
    if (compareTerrainRank) {
        struct RankPair { int id; int rank; };
        int id = *(signed char*)&((ShotCandidateLayout*)candidateA)->groundType;
        int rankA = 0;
        const RankPair* p = (const RankPair*)courseObjectRankTable;
        for (int i = 0; i < 12; ++p, ++i) {
            if (id == p->id) { rankA = ((const RankPair*)courseObjectRankTable)[i].rank; break; }
        }
        id = *(signed char*)&((ShotCandidateLayout*)candidateB)->groundType;
        int rankB = 0;
        const RankPair* q = (const RankPair*)courseObjectRankTable;
        for (int i = 0; i < 12; ++q, ++i) {
            if (id == q->id) { rankB = ((const RankPair*)courseObjectRankTable)[i].rank; break; }
        }
        int diff = rankA - rankB;
        if (diff > 0) return a;
        if (diff < 0) return b;
    }
    // Phase 4: prefer nearHazard==0 (+0x24), then shorter remainingDistance (+4).
    int otherNearHazard = CandidateSignedWordValue(candidateB->nearHazard);
    int hazardPair = kNeitherCandidate;
    if (CandidateSignedWordValue(candidateA->nearHazard)) hazardPair |= kOnlyCandidateA;
    if (otherNearHazard) hazardPair |= kOnlyCandidateB;
    switch (hazardPair) {
    case kOnlyCandidateA:
        return b;
    case kOnlyCandidateB:
        return a;
    case kNeitherCandidate:
    case kBothCandidates:
        break;
    default:
        break;
    }
    if (((ShotCandidateLayout*)candidateA)->remainingDistance < ((ShotCandidateLayout*)candidateB)->remainingDistance) return a;
    return b;
}

// Probe straight and rotated sweeps, possibly changing curve and the debug probe position.
// Every path returns 1. Rotated fraction < 1 selects a sign; this code does not
// promise a clear trajectory, and the return is not a collision-free predicate.
int CRival_ChooseCurveAroundObstruction(void* self, void* target) {
    double leftCurveMatrix[6];
    double rightCurveMatrix[6];
    float ballPosition[3];
    float midpointDirection[3];
    float input[3];
    float first[3];
    float finalVec[3];
    float third[3];
    float tailB[3];
    float tailA[3];

    float maxCurve;
    void* base = GolfBall__Fv();
    int b0 = ((int*)base)[0];
    unsigned int b1 = ((int*)base)[1];
    ((int*)ballPosition)[1] = b1;
    ((int*)ballPosition)[0] = b0;
    ((int*)ballPosition)[2] = ((int*)base)[2];
    __mi__FRC7WVectorRC7WVector(input, target, ballPosition);
    int v1 = ((int*)input)[1];
    int v0 = ((int*)input)[0];

    float v[3]; ((int*)v)[1] = v1;
    ((int*)v)[0] = v0;
    unsigned int v2 = ((int*)input)[2]; ((int*)v)[2] = v2;
    v[1] = 0.0f;

    // Phase 1: only probe long shots; raise the origin and sweep half the target delta.
    if ((WVectorLen__FRC7WVector(v)) < 192.0f)
        return 1;
    ballPosition[1] += 3.0f;
    __mi__FRC7WVectorRC7WVector(first, target, ballPosition);
    __ml__FfRC7WVector(midpointDirection, first, 0.5f);
    midpointDirection[1] = 32.0f;
    float straightHitFraction = CQuadTree_FindObstructionSweepFraction(CQuadTree_GetInstance(), ballPosition, midpointDirection);
    if (straightHitFraction >= 1.0f)
        return 1;
    void* stage = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned char idx = AsCGolfDoc(stage)->currentPlayer;
    void* info = GolfPlayer_GetEntryByIndex(stage, idx);
    int curveStat = ((PlayerCurveStatView*)info)->curve;
    maxCurve = ((float)curveStat) / 30.0f;
    // Phase 2: rotate the sweep by +/- (10 * curve) degrees, converted to radians.
    // A hit fraction < 1 selects the corresponding sign; preserve that original rule.
    __typeof__(maxCurve * 10.0f) curveProbeDegrees = (10.0f * maxCurve); Matrix_SetAxisRotation(leftCurveMatrix, curveProbeDegrees * 0.01745329238474369f, 1);
    Matrix_TransformPoint(finalVec, midpointDirection, leftCurveMatrix);
    float curvedHitFraction = CQuadTree_FindObstructionSweepFraction(CQuadTree_GetInstance(), ballPosition, finalVec);
    if (curvedHitFraction < 1.0f) {
        AsCRival(self)->shotCurve = -(double)maxCurve;
        maxCurve = (float)(-(double)maxCurve);
        AsGolfBall(GolfBall__Fv())->m_initCurve = maxCurve;
    } else {
        Matrix_SetAxisRotation(rightCurveMatrix, -maxCurve * 10.0f * 0.01745329238474369f, 1);
        Matrix_TransformPoint(third, midpointDirection, rightCurveMatrix);
        curvedHitFraction = CQuadTree_FindObstructionSweepFraction(CQuadTree_GetInstance(), ballPosition, third);
        if (curvedHitFraction < 1.0f) {
            AsCRival(self)->shotCurve = maxCurve;
            AsGolfBall(GolfBall__Fv())->m_initCurve = maxCurve;
        } else {
            return 1;
        }
    }
    // Phase 3: expose the original straight-sweep contact position to the debug view.
    __ml__FfRC7WVector(tailA, midpointDirection, straightHitFraction);
    __pl__FRC7WVectorRC7WVector(tailB, ballPosition, tailA);
    __as__7WVectorFRC7WVector(g_comShotCurveProbePosition, tailB);
    return 1;
}

// Leave the course collision system's simulation mode.
void CComShotProc_vfunc_2() {
    CQuadTree_SetSimulationMode(CQuadTree_GetInstance(), 0);
}

// headingRadians seeds candidate angle; out holds the 0x40-byte candidate.
// The chosen club and simulation start frame are stored in the rival as side effects.
// Try club choices, simulate the target, and return a power-scaled candidate.
void CRival_SelectClubAndComputeShot(void* self, void* out, float headingRadians) {
    struct Vec3 { int x, y, z; };
    struct ClubIndexView { char pad[9]; unsigned char clubIndex; };
    struct ClubSelectionView { char pad[4]; ClubIndexView* info; };
    struct LayoutVt {
        virtual void v0() DECOMP_LOCAL_VIRTUAL; virtual void v1() DECOMP_LOCAL_VIRTUAL; virtual void v2() DECOMP_LOCAL_VIRTUAL; virtual void v3() DECOMP_LOCAL_VIRTUAL;
        virtual void v4() DECOMP_LOCAL_VIRTUAL; virtual void v5() DECOMP_LOCAL_VIRTUAL; virtual void v6() DECOMP_LOCAL_VIRTUAL; virtual void v7() DECOMP_LOCAL_VIRTUAL;
        virtual void v8() DECOMP_LOCAL_VIRTUAL;
        virtual void apply(void*) DECOMP_LOCAL_VIRTUAL;
    };
    struct LayoutArgs {
        int a; int size; float* value; int r0, r1, r2, r3, r4;
        // Package a synchronous actor request whose output is the borrowed float pointer.
        LayoutArgs(float* v) {
            a = 0; size = 0x20; value = v;
            r0 = 0; r1 = 0; r2 = 0; r3 = 0; r4 = 0;
        }
    };
    struct ClubShotCandidate {
        float aimAngle, remainingDistance, power, curve;
        Vec3 pos;
        int rejected;
        unsigned char groundType;
        int nearHazard, firstLandingWithinTarget, finalWithinTarget;
        int hasSpecialLanding, hasInvalidLanding, holed, simulationFrames;
    };

    char* s = (char*)self;
    ClubSelection_SelectForCpuTarget(InstanceTable_Get(), s + 0x10);
    float rate = kFloat_0_ComShot;
    void* context = g_taskManager ? ((void* (*)(void*))Object_GetChild)(g_taskManager) : 0;
    LayoutArgs args(&rate);
    void* layout = ((void* (*)(void*, int))TaskActorTable_FindByName)(context, (int)comShotDebugStrings);
    if (layout) ((LayoutVt*)layout)->apply(&args);
    float baseSimulationFrame = kFloat_30_Rival + (float)*(unsigned int*)((char*)CQuadTree_GetInstance() + 0x21AC);
    AsCRival(s)->simulationStartFrame = baseSimulationFrame + (float)Runtime_ConvertDoubleToUnsigned(kFloat_30_Rival * rate);
    Vec3 first, cur, end, firstFlat, curFlat, endFlat;
    __mi__FRC7WVectorRC7WVector(&first, s + 0x10, GolfBall__Fv());
    firstFlat = first;
    *(float*)&firstFlat.y = kFloat_0_ComShot;
    float initialDistance = WVectorLen__FRC7WVector(&firstFlat);
    float curve = AsCRival(s)->shotCurve;
    ClubShotCandidate state;
    // Phase 1: simulate at full power with the initial club selection.
    state.aimAngle = headingRadians;
    state.power = kFloat_1_Rival;
    state.curve = curve;
    state.rejected = 0;
    state.groundType = 0;
    state.nearHazard = 0; state.firstLandingWithinTarget = 0; state.finalWithinTarget = 0; state.hasSpecialLanding = 0;
    state.hasInvalidLanding = 0; state.holed = 0; state.simulationFrames = 0;
    ComShot_SimulateTrajectory(self, &state, s + 0x10);
    // Phase 2: honor a fixed club or try lower club indices while the shot overshoots.
    int index = AsCRival(s)->selectedClubIndex;
    if (index != -1) {
        ClubSelection_SetClubIndex(InstanceTable_Get(), (unsigned char)index);
    } else {
        int mode = CComThinkCtrl_GetOption(AsCRival(s)->thinkController, 0x10);
        int forceFirstClub;
        switch (mode) {
        case 1:
        case 3:
            forceFirstClub = 1;
            break;
        default:
            forceFirstClub = 0;
            break;
        }
        if (forceFirstClub) {
            ClubSelection_SetClubIndex(InstanceTable_Get(), 0);
        } else if (!(*(unsigned int*)&AsGolfBall(GolfBall__Fv())->m_powerFlags & 0x20)) {
            for (;;) {
                if (((ClubSelectionView*)InstanceTable_Get())->info->clubIndex == 0) break;
                __mi__FRC7WVectorRC7WVector(&cur, &state.pos, GolfBall__Fv());
                curFlat = cur;
                *(float*)&curFlat.y = kFloat_0_ComShot;
                if (initialDistance <= WVectorLen__FRC7WVector(&curFlat) - kFloat_25_6_ComShot) break;
                ClubSelectionView* list = (ClubSelectionView*)InstanceTable_Get();
                ClubSelection_SetClubIndex(InstanceTable_Get(), (unsigned char)(list->info->clubIndex - 1));
                float rate2 = kFloat_0_ComShot;
                void* context2 = g_taskManager ? ((void* (*)(void*))Object_GetChild)(g_taskManager) : 0;
                LayoutArgs args2(&rate2);
                void* layout2 = ((void* (*)(void*, int))TaskActorTable_FindByName)(context2, (int)comShotDebugStrings);
                if (layout2) ((LayoutVt*)layout2)->apply(&args2);
                float retryBaseSimulationFrame = kFloat_30_Rival + (float)*(unsigned int*)((char*)CQuadTree_GetInstance() + 0x21AC);
                AsCRival(s)->simulationStartFrame = retryBaseSimulationFrame + (float)Runtime_ConvertDoubleToUnsigned(kFloat_30_Rival * rate2);
                ComShot_SimulateTrajectory(self, &state, s + 0x10);
            }
        }
    }
    ClubIndexView* shownInfo = ((ClubSelectionView*)InstanceTable_Get())->info;
    int shown = shownInfo->clubIndex;
    ComShot_Log(((ComShotStringPool*)comShotDebugStrings)->selectedClubFormat, shown, comShotDebugStrings);
    AsCRival(s)->selectedClubIndex = ((ClubSelectionView*)InstanceTable_Get())->info->clubIndex;
    // Phase 3: scale power by horizontal target/simulated travel, then copy the candidate.
    if (AsGolfBall(GolfBall__Fv())->m_powerFlags == 0) {
        __mi__FRC7WVectorRC7WVector(&end, &state.pos, GolfBall__Fv());
        endFlat = end;
        *(float*)&endFlat.y = kFloat_0_ComShot;
        state.power = initialDistance / WVectorLen__FRC7WVector(&endFlat);
        state.power = state.power < kFloat_0_001_Rival ? kFloat_0_001_Rival : (state.power > kFloat_1_Rival ? kFloat_1_Rival : state.power);
    } else {
        state.power = kFloat_1_Rival;
    }
    // Mark the club-selection seed rejected until correction validates it.
    state.rejected = 1;
    float* dst = (float*)out;
    dst[0] = state.aimAngle; dst[1] = state.remainingDistance; dst[2] = state.power; dst[3] = state.curve;
    __as__7WVectorFRC7WVector(&((ClubShotCandidate*)out)->pos, &state.pos);
    ((ClubShotCandidate*)out)->rejected = state.rejected;
    ((ClubShotCandidate*)out)->groundType = state.groundType;
    ((ClubShotCandidate*)out)->nearHazard = state.nearHazard;
    ((ClubShotCandidate*)out)->firstLandingWithinTarget = state.firstLandingWithinTarget;
    ((ClubShotCandidate*)out)->finalWithinTarget = state.finalWithinTarget;
    ((ClubShotCandidate*)out)->hasSpecialLanding = state.hasSpecialLanding;
    ((ClubShotCandidate*)out)->hasInvalidLanding = state.hasInvalidLanding;
    ((ClubShotCandidate*)out)->holed = state.holed;
    ((ClubShotCandidate*)out)->simulationFrames = state.simulationFrames;
}

// Clear the rival singleton and destroy the base during constructor unwinding.
void* dtor_801DEB54(void* self, int flags) {
    if (self != 0) {
        g_rival = 0;
        BaseObject_dtor(self, 0);
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}

} // extern "C"
} // namespace ns1

// ---- from com_shot_cpu_rival_ai.cpp ----
namespace ns2 {
extern "C" {

struct RivalUnplayableView {
    char pad[0x4C];
    void* unplayableHandler;
};

struct ThinkTaskLinkView { int a; ThinkTaskLinkView* next; int state; };
struct GolfDocIndicesView { char pad[0x3B6]; unsigned char currentHoleIndex; char pad2[0xA]; unsigned char currentPlayer; };
struct GolfDocHoleTableView { char pad[0x2BC]; void* holeRecords; };
struct SharedGolfDocView { char pad[0xB90]; GolfDocIndicesView* golfDoc; };

void* CComUnplayable_UpdateDropSequence(void*);
void* CComUnplayable_IsDropSequenceActive(void*);
void* InstanceTable_Get();
void CClub_SelectBestClubForHole(void*);
void __as__7WVectorFRC7WVector(void*, void*);
int GolfPlayer_GetEntryByIndex(void*, unsigned char);
void* CComUnplayable_ResetToIdle(void*);
void ComThinkChain_MarkAllNodes(void*);
void CComThinkShotPos_ClearPlayerPriorityAdjustments(void*, unsigned char);
extern CSharedDoc* g_sharedDoc;

// Legacy local ABI declaration: most calls use a no-argument singleton view;
// the slope sampler also forwards an ignored output address. Keep those call
// shapes until a separate signature cleanup can preserve their instructions.
Vec3f* GolfBall__Fv(void*);
void ComShot_SimulateTrajectory(void*, void*, void*);
void ComShot_Log(const char*, ...);
void Matrix_SetAxisRotation(void*, float, int);
void __ml__FfRC7WVector(Vec3f*, Vec3f*, float);
void Matrix_TransformPoint(Vec3f*, Vec3f*, void*);
void __pl__FRC7WVectorRC7WVector(Vec3f*, Vec3f*, Vec3f*);
float CComRandomUse_AdjustValue(void*, int, int, float);
float ShotParameters_GetPoorImpactHalfWidth(void*);
void CRival_PrepareShotGauge(void*, float, float);
extern int s_comShotCandidateEvaluationState;
extern Vec3f g_comShotPredictedLandingPosition;
extern char comShotDebugStrings[], comShotPowerLengthAngleFormat[];
extern char blankRivalStatusString[4], ainStatusString[4], shotLogNoObMarker[3], outOfBoundsAbbreviation[3];
extern float kFloat_0_ComShot, kFloat_1_Rival, kFloat_100_0_ComShot, kFloat_0_00001_ComShot, kFloat_neg1_ComShot;

struct EvaluatedShotView {
    float angle, remainingDistance, power, curve;
    Vec3f pos;
    int rejected;
    char pad20[12];
    int finalWithinTarget;
};
struct RivalEvaluationView {
    char pad0[0x10];
    Vec3f targetPosition;
    Vec3f aimPoint;
    char pad28[8];
    int flag;
    float power, curve;
    char pad3c[12];
    void* randomUse;
    void* unplayableHandler;
    char pad50[4];
    void* targetArea;
};

// Cap a candidate power value at the supplied upper bound.
inline float Upper801E(float limit, float power) {
    return power > limit ? limit : power;
}
struct SharedCharacterRecord { char pad[0x1A4]; signed char character[0x30]; };

#pragma opt_propagation off
// Simulate a candidate, add character-dependent error, and prepare its shot gauge.
int ComShot_EvaluateCandidate(RivalEvaluationView* self, EvaluatedShotView* arg) {
    const char* positionFormat;
    const char* positionName;
    int character;
    int first;
    int second;
    float upper;
    float adjustment;
    // Phase 1: simulate the selected candidate and publish its predicted landing.
    self->curve = arg->curve;
    self->power = arg->power;
    s_comShotCandidateEvaluationState = 1;
    ComShot_SimulateTrajectory(self, arg, &self->targetPosition);
    __as__7WVectorFRC7WVector(&g_comShotPredictedLandingPosition, &arg->pos);
    ComShot_Log(((ComShotStringPool*)comShotDebugStrings)->powerFormat, self->power);
    ComShot_Log(((ComShotStringPool*)comShotDebugStrings)->curveFormat, self->curve);
    positionName = comShotDebugStrings + (int)&((ComShotStringPool*)0)->outputLabel;
    positionFormat = ((ComShotStringPool*)comShotDebugStrings)->vectorFormat;
    ComShot_Log(positionFormat, positionName, arg->pos.x, arg->pos.y, arg->pos.z);
    const char* name = comShotDebugStrings + (int)&((ComShotStringPool*)0)->outputLabel;
    const char* finalWithinTarget = blankRivalStatusString;
    if (arg->finalWithinTarget) finalWithinTarget = ainStatusString;
    ComShot_Log(comShotPowerLengthAngleFormat, name, arg->power, arg->remainingDistance, arg->angle, arg->rejected ? outOfBoundsAbbreviation : shotLogNoObMarker, finalWithinTarget);
    float matrix[12];
    Vec3f direction;
    Vec3f scaled;
    Vec3f rotated;
    Vec3f result;
    float angle = arg->angle;
    direction.x = kFloat_0_ComShot;
    direction.y = kFloat_0_ComShot;
    direction.z = kFloat_1_Rival;
    Matrix_SetAxisRotation(matrix, angle, 1);
    __ml__FfRC7WVector(&scaled, &direction, kFloat_100_0_ComShot);
    Matrix_TransformPoint(&rotated, &scaled, matrix);
    __pl__FRC7WVectorRC7WVector(&result, ((Vec3f*(*)())GolfBall__Fv)(), &rotated);
    __as__7WVectorFRC7WVector(&self->aimPoint, &result);
    // Phase 2: select error-distribution lanes for normal/special/no-target shots.
    first = kNormalPowerRandomIndex;
    second = kNormalImpactRandomIndex;
    GolfDocIndicesView* golfDoc;
    char* manager;
    manager = (char*)g_sharedDoc;
    golfDoc = ((SharedGolfDocView*)manager)->golfDoc;
    unsigned int index = (unsigned char)(manager + golfDoc->currentHoleIndex)[0x83D];
    SharedCharacterRecord* shifted = (SharedCharacterRecord*)(index + (unsigned int)manager);
    character = shifted[golfDoc->currentPlayer].character[0];
    if (*(int*)((char*)((Vec3f*(*)())GolfBall__Fv)() + 0x44)) {
        first = kSpecialPowerRandomIndex;
        second = kSpecialImpactRandomIndex;
    } else if (((ShotTargetArea*)self->targetArea)->index == -1) {
        first = kNoTargetPowerRandomIndex;
        second = kNoTargetImpactRandomIndex;
    }
    // Phase 3: perturb power/impact, clamp impact inside the poor-impact window.
    self->power = CComRandomUse_AdjustValue(self->randomUse, character, first, self->power);
    adjustment = CComRandomUse_AdjustValue(self->randomUse, character, second, kFloat_100_0_ComShot) - kFloat_100_0_ComShot;
    self->power = Upper801E(kFloat_1_Rival, self->power);
    upper = ShotParameters_GetPoorImpactHalfWidth(InstanceTable_Get()) - kFloat_0_00001_ComShot;
    float lower = -(ShotParameters_GetPoorImpactHalfWidth(InstanceTable_Get()) - kFloat_0_00001_ComShot);
    float value = adjustment < lower ? lower : adjustment > upper ? upper : adjustment;
    CRival_PrepareShotGauge(self, self->power, value);
    self->flag = 0;
    if (*(int*)((char*)((Vec3f*(*)())GolfBall__Fv)() + 0x44)) {
        if (*(int*)((char*)((Vec3f*(*)())GolfBall__Fv)() + 0x44) & 0x70)
            self->flag = 1;
    } else {
        float clamped;
        float v = self->curve;
        float low = kFloat_neg1_ComShot;
        if (v < low) clamped = low;
        else {
            float high = kFloat_1_Rival;
            if (v > high) clamped = high;
            else clamped = v;
        }
        self->curve = clamped;
    }
    CComUnplayable_ResetToIdle(self->unplayableHandler);
    return 1;
}
#pragma opt_propagation reset

// Save the selected club and restart the rival correction chain for this hole.
void CRival_ResetShotThinkingState(void* self) {
    CClub_SelectBestClubForHole(InstanceTable_Get());
    SharedGolfDocView* mgr = (SharedGolfDocView*)g_sharedDoc;
    GolfDocHoleTableView* game = (GolfDocHoleTableView*)mgr->golfDoc;
    int idx = mgr->golfDoc->currentHoleIndex;
    void* entry = (char*)game->holeRecords + (idx << 5);
    Vec3f xyz = *(Vec3f*)&((CourseHoleView*)entry)->position;
    *(WVector*)AsCRival(self)->holePosition = *(WVector*)&xyz;
    mgr = (SharedGolfDocView*)g_sharedDoc;
    GolfDocIndicesView* golfDoc = mgr->golfDoc;
    unsigned char type = golfDoc->currentPlayer;
    __typeof__(golfDoc->currentHoleIndex) savedIndex = golfDoc->currentHoleIndex; unsigned int off = (int)GolfPlayer_GetEntryByIndex(golfDoc, type);
    void* base = (void*)(savedIndex + off);
    if (((PlayerRoundFlagsView*)base)->strokes[0] >= 1) {
        void* info = InstanceTable_Get();
        unsigned char v = *((unsigned char*)(*(void**)((char*)info + 4)) + 9);
        AsCRival(self)->savedClubIndex = v;
        AsCRival(self)->selectedClubIndex = kInvalidClub;
        CComUnplayable_ResetToIdle(AsCRival(self)->unplayableHandler);
        ThinkTaskLinkView* n = (ThinkTaskLinkView*)AsCRival(self)->shotProcessor;
        n->state = 1;
        n = n->next;
        // Mark the first five links explicitly, then delegate the remaining tail.
        if (n) {
            n->state = 1; n = n->next;
            if (n) {
                n->state = 1; n = n->next;
                if (n) {
                    n->state = 1; n = n->next;
                    if (n) {
                        n->state = 1; n = n->next;
                        if (n) ComThinkChain_MarkAllNodes(n);
                    }
                }
            }
        }
        base = (void*)GolfPlayer_GetEntryByIndex(((SharedGolfDocView*)g_sharedDoc)->golfDoc, ((SharedGolfDocView*)g_sharedDoc)->golfDoc->currentPlayer);
        if (!((PlayerRoundFlagsView*)base)->flagBC)
            CComThinkShotPos_ClearPlayerPriorityAdjustments(AsCRival(self)->shotPositionThinker, ((SharedGolfDocView*)g_sharedDoc)->golfDoc->currentPlayer);
    }
}

struct State801E {
    virtual void v2();
    virtual void start(void*); virtual void stop(void*); virtual void update(void*); virtual int finish(void*);
    State801E* child; int state;
};
int Process_NestedTaskStates(State801E*, void*);
void ClubSelection_SetClubIndex(void*, unsigned char);

// Run nested shot tasks while temporarily selecting and then restoring the rival club.
int CRival_UpdateShotProcessor(void* self) {
    int index = AsCRival(self)->selectedClubIndex;
    if (index != -1) {
        void* info = InstanceTable_Get();
        ClubSelection_SetClubIndex(info, (unsigned char)index);
    }

    State801E* current = (State801E*)AsCRival(self)->shotProcessor;
    switch (current->state) {
    case kTaskIdle: break;
    case kTaskStart:
        current->start(0);
        current->state = kTaskUpdate;
    // Fall through: start, update, and child processing can run in one call.
    case kTaskUpdate:
        current->update(0);
        current->state = kTaskAwaitChild;
    case kTaskAwaitChild: {
            State801E* child = current->child;
            if (child) {
                switch (child->state) {
                case kTaskIdle: break;
                case kTaskStart:
                    child->start(current);
                    child->state = kTaskUpdate;
                // Fall through: start, update, and child processing can run in one call.
    case kTaskUpdate:
                    child->update(current);
                    child->state = kTaskAwaitChild;
                case kTaskAwaitChild:
                    if (child->child) {
                        Process_NestedTaskStates(child->child, child);
                        if (child->child->state != kTaskIdle) break;
                    }
                    {
                        int result = child->finish(current);
                        if (result == kTaskResultRepeat)
                            child->state = kTaskUpdate;
                        else {
                            child->stop(current);
                            child->state = kTaskIdle;
                        }
                    }
                }
                if (current->child->state != kTaskIdle)
                    break;
            }
            {
                int result = current->finish(0);
                if (result == kTaskResultRepeat)
                    current->state = kTaskUpdate;
                else {
                    current->stop(0);
                    current->state = kTaskIdle;
                }
            }
        }
    }

    // Original 0x801E0018: finish the outer update after the state switch.
    int finalIndex = AsCRival(self)->savedClubIndex;
    if (finalIndex != -1) {
        void* info = InstanceTable_Get();
        ClubSelection_SetClubIndex(info, (unsigned char)finalIndex);
    }
    return 1;
}

// arg is the borrowed parent context; return 0 for idle/root completion, 1 otherwise.
// Child completion is tracked by state, so return 1 alone does not mean work remains.
// Run start/update/child/finish phases, repeating update when finish requests it.
int Process_NestedTaskStates(State801E* self, void* arg) {
    switch (self->state) {
    case kTaskIdle: return 0;
    case kTaskStart: self->start(arg); self->state = kTaskUpdate;
    // Fall through: start, update, and child processing can run in one call.
    case kTaskUpdate: self->update(arg); self->state = kTaskAwaitChild;
    case kTaskAwaitChild: {
    State801E* child = self->child;
    if (child) {
        switch (child->state) {
        case kTaskIdle: break;
        case kTaskStart: child->start(self); child->state = kTaskUpdate;
        // Fall through: start, update, and child processing can run in one call.
    case kTaskUpdate: child->update(self); child->state = kTaskAwaitChild;
        case kTaskAwaitChild:
            if (child->child) {
                Process_NestedTaskStates(child->child, child);
                if (child->child->state != kTaskIdle) break;
            }
            {
                int result = child->finish(self);
                if (result == kTaskResultRepeat) child->state = kTaskUpdate;
                else { child->stop(self); child->state = kTaskIdle; }
            }
        }
        if (self->child->state != kTaskIdle) break;
    }
    int result = self->finish(arg);
    if (result == kTaskResultRepeat) self->state = kTaskUpdate;
    else {
        self->stop(arg);
        self->state = kTaskIdle;
        if (!arg) return 0;
    }
    }
    }
    return 1;
}

// Advance the rival's unplayable-ball drop sequence.
void* ComUnplayable_UpdateCurrentDropSequence(RivalUnplayableView* self) {
    return CComUnplayable_UpdateDropSequence(self->unplayableHandler);
}

// Forward the rival's unplayable helper to its sequence accessor.
void* CRival_IsUnplayableDropActive(RivalUnplayableView* self) {
    return CComUnplayable_IsDropSequenceActive(self->unplayableHandler);
}

// Legacy local ABI declaration: most calls use a no-argument singleton view;
// the slope sampler also forwards an ignored output address. Keep those call
// shapes until a separate signature cleanup can preserve their instructions.
Vec3f* GolfBall__Fv(void*);
void __mi__FRC7WVectorRC7WVector(Vec3f*, void*, Vec3f*);
float Magnitude__7WVectorCFv(Vec3f*);
void Normalize__7WVectorFv(Vec3f*);
void Vec3_Zero(void*);
void __ml__FfRC7WVector(Vec3f*, Vec3f*, float);
void __apl__7WVectorFRC7WVector(Vec3f*, Vec3f*);
void* CQuadTree_GetInstance();
void CQuadTree_GetMeshNormalAtPoint(Vec3f*, void*, Vec3f*);

extern float kFloat_0_ComShot;
extern const float kFloat_3_2_ComShot;

// summedSlope is a caller-owned Vec3 output; target is a borrowed world position.
// Samples are spaced 3.2 world units; this is a SUM, not an averaged/normalized slope.
// A zero-length direction is passed to Normalize as-is, preserving existing behavior.
// Sum horizontal terrain normals at evenly spaced points from the ball to the target.
void ComPutt_SumSlopeAlongTargetPath(void* summedSlope, void* unusedContext, Vec3f* target) {
    Vec3f direction;
    int sampleCount;
    Vec3f* ballPosition = GolfBall__Fv(summedSlope);
    __mi__FRC7WVectorRC7WVector(&direction, target, ballPosition);
    direction.y = kFloat_0_ComShot;
    sampleCount = (int)(Magnitude__7WVectorCFv(&direction) / kFloat_3_2_ComShot);
    Normalize__7WVectorFv(&direction);
    Vec3_Zero(summedSlope);

    Vec3f samplePosition = *ballPosition;
    float zero = kFloat_0_ComShot;
    for (int i = 0; i < sampleCount; ++i) {
        Vec3f groundNormal;
        Vec3f step;
        __ml__FfRC7WVector(&step, &direction, kFloat_3_2_ComShot);
        __apl__7WVectorFRC7WVector(&samplePosition, &step);
        Vec3f* world = (Vec3f*)CQuadTree_GetInstance();
        CQuadTree_GetMeshNormalAtPoint(&groundNormal, world, &samplePosition);
        groundNormal.y = zero;
        __apl__7WVectorFRC7WVector((Vec3f*)summedSlope, &groundNormal);
    }
}

} // extern "C"
} // namespace ns2

// ---- from com_shot_cpu_rival_ai.cpp ----
namespace ns3 {
extern "C" {
typedef Vec3f Vec3;
struct Vec3W { Vec3 v;
    // Assign all three world-vector components without changing copy representation.
    inline void Set(float x,float y,float z) { v.x=x; v.y=y; v.z=z; }
};
struct EncodedInt {
    unsigned int value; unsigned char bytes[4]; unsigned int key;
    // Decode the ball-state word using its per-object XOR key and complement.
    inline int Get() const { return ~(value^key); }
};
struct SimulatedBallView {
    Vec3W pos,vel;
    int motionState; char pad1c[0xc]; int groundTriangle;
    float rotationX; char pad30[4];
    float lieSlope,lieAngle,curveImpulse,spinImpulse;
    unsigned int requestedShotFlags,activeShotFlags;
    Vec3W spinImpulseVector;
    unsigned char recoveryRequired,groundType; char pad5a[2];
    int lie,shotStage,frameCount; char pad68[4];
    int firstLandingFrame,descentStartFrame,eventFrame74,eventFrame78,eventFrame7C,eventFrame80,eventFrame84,eventFrame88,eventFrame8C,special20TransitionFrame,special10EndFrame,special40LaunchFrame,special40DiveFrame,special40EndFrame;
    float simulatedSeconds; char pada8[4]; unsigned char bounceCount; char padad[1];
    short firstContactId; int firstContactIndex; short lastContactId; char padb6[2]; int lastContactIndex; char padbc[4]; float radius;
    char padc4[0x1989c-0xc4]; EncodedInt encoded;
    // Update the encoded ball-state word and its address-dependent check bytes.
    inline void SetEncoded(int v) {
        char* act=(char*)this; char* key=act+0x1989C; char* r=key; unsigned char* kb=(unsigned char*)&key; *(int*)(act+0x1989C)=~(v^*(int*)(act+0x198A4));
        for (int j=0;j<sizeof(key);++j) { act[0x198A0+j]=r[j]^kb[j]; }
    }
};
// +0x70 holds a motion-state tag, not a frame number.
struct BallSampleView {
    Vec3W pos,vel; char pad18[0x48]; unsigned char b60; char pad61[15]; int motionState;
};
struct Receiver {
    virtual void slot0(); virtual void slot1(); virtual void slot2(); virtual void slot3();
    virtual void slot4(); virtual void slot5(); virtual void slot6(); virtual void slot7();
    virtual void slot8(); virtual void dispatch(void*);
};
// Integer view for bit-preserving fallback XYZ copies.
struct SimulationVectorWords { unsigned int word0, word1, word2; };
struct ShotSimulationResult {
    float headingRadians, remainingDistance, power; int curveBits;
    Vec3W result;
    int rejected; unsigned char groundType; char pad21[3]; int nearHazard,firstLandingWithinTarget,finalWithinTarget,hasSpecialLanding,hasInvalidLanding,holed,simulationFrames;
};
extern CSharedDoc* g_sharedDoc;
extern void* g_taskManager;
extern const char comShotDebugStrings[];
extern const float kFloat_0_ComShot,kFloat_1_3_ComShot,kFloat_0_34906584_ComShot,kFloat_149_ComShot,kFloat_1_Rival;
extern const float kFloat_0_2_ComShot,kFloat_neg1_ComShot,kFloat_0_17453292_ComShot,kFloat_15_ComShot,kFloat_0_6981317_ComShot;
extern const float kFloat_20_ComShot,kFloat_1_3962634_ComShot,kFloat_50_ComShot,kFloat_5_ComShot,kFloat_0_1_U801DD078;
extern const float kFloat_0_5_ComShot,kFloat_170_ComShot,kFloat_1_5_ComShot,kFloat_273_ComShot,kFloat_20_2_ComShot;
extern const float kFloat_2_ComShot,kFloat_neg10_799999_ComShot,kFloat_0_0033333336_ComShot,kFloat_306_ComShot,kFloat_105_ComShot;
extern const float kFloat_20_5_ComShot,kComShotSimulationStepSeconds,kFloat_neg10_299999_ComShot,kFloat_0_7853982_ComShot,kFloat_7_ComShot;
extern const float kFloat_100_0_ComShot,kFloat_3_2_ComShot,kFloat_2_5_ComShot,kFloat_3_1_ComShot,kFloat_344_ComShot;
extern const float kFloat_112_ComShot,kFloat_21_5_ComShot,kFloat_neg8_ComShot,kFloat_74_ComShot,kFloat_76_ComShot;
extern const float kFloat_80_ComShot,kFloat_72_5_ComShot,kFloat_192_ComShot;
extern float s_comShotDiveSpeedCapClubCategory0,s_comShotDiveSpeedCapClubCategory1,s_comShotDiveSpeedCapClubCategory2;
void* GolfPlayer_GetEntryByIndex(void*,unsigned char);
SimulatedBallView* GolfBall__Fv();
void* CQuadTree_GetInstance();
void* InstanceTable_Get();
float ShotParameters_GetBaseLaunchSpeed(void*);
float Math_SquareRootApprox(float);
BallSampleView* CGolfBall_GetOrAllocatePreDataAt(void*,int);
void __as__7WVectorFRC7WVector(void*,const void*);
void Vec3_Zero(void*);
void CRival_PrepareShotGauge(void*,float,float);
float CQuadTree_GetMaxPowerOnGround(void*);
float CQuadTree_GetMinPowerOnGround(void*);
float ShotParameters_GetGoodImpactHalfWidth(void*);
float ShotParameters_GetPoorImpactHalfWidth(void*);
float ClubSelection_GetLaunchAngle(void*,int);
float GolfParameter_ResolveFloat(void*,int);
void Matrix_SetAxisRotation(void*,float,int);
void __ml__FfRC7WVector(void*,const void*,float);
void Matrix_TransformPoint(void*,const void*,const void*);
void* CGolfBall_NewPreData(void*);
void CGolfBall_ClearBuffer(void*);
void* PreData__9CGolfBallFi(void*,int);
void CQuadTree_ResetPerShotSimulationState(void*);
float CSharedDoc_ComputeAuxPartModifier(void*,int,int);
void CQuadTree_StepBallSimulation(void*,float);
float CQuadTree_ResolveGroundHeight(void*,const void*,float);
int CQuadTree_GetObjColNum(void*);
int CQuadTree_GetMeshIndex(void*,void*,int);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*,void*,const void*,int,int,int);
void Vec2_Copy(void*,const void*);
void __mi__FRC7WVectorRC7WVector(void*,const void*,const void*);
void __pl__FRC7WVectorRC7WVector(void*,const void*,const void*);
float WVectorLen__FRC7WVector(const void*);
float Magnitude__7WVectorCFv(const void*);
void* Normalize__7WVectorFv(void*);
void __amu__7WVectorFf(void*,float);
float CQuadTree_GetGroundSlope(void*,int);
int CComThinkCtrl_GetOption(void*,int);
int CourseData_IsPointOutsidePlayableRegion(void*,const void*);
void* Object_GetChild(void*);
void* TaskActorTable_FindByName(void*,const char*);
int QuadTree_IsWaterAtPoint(void*,const void*);
unsigned char GetGroundType__9CQuadTreeF7WVector(void*,const void*);
void CQuadTree_UpdateAnimatedObjects(void*,float,int);
void* memcpy(void*,const void*,unsigned long);
// ABI-width accessors for the remaining partially placed external records.
// Simulation scratch itself uses typed fields below.
#pragma push
#pragma always_inline on

// Access a signed word at a known byte offset.
inline int& IntAt(void* p, int offset) { return *(int*)((char*)p + offset); }

// Access a byte at a known offset.
inline unsigned char& ByteAt(void* p, int offset) { return *(unsigned char*)((char*)p + offset); }
// Access a signed halfword at a known byte offset.
inline short& ShortAt(void* p, int offset) { return *(short*)((char*)p + offset); }

// Resolve the shared document's active round without caching the singleton.
inline CGolfDoc* CurrentGolfDoc() { return AsCGolfDoc((void*)g_sharedDoc->golfDoc); }
// Resolve the current player entry used for temporary simulation state.
inline void* CurrentPlayerEntry() { return GolfPlayer_GetEntryByIndex(CurrentGolfDoc(), CurrentGolfDoc()->currentPlayer); }
struct SharedPlayerClubRecord {
    char pad_00[0x120]; int clubTypeId; char pad_124[0x1D4 - 0x124];
};
// Read the current player's selected club identifier from its fixed-stride record.
inline int CurrentClubTypeId() {
    return ((SharedPlayerClubRecord*)g_sharedDoc)[CurrentGolfDoc()->currentPlayer].clubTypeId;
}
// Read the club's trajectory mode; this is separate from the task-state enum.
inline unsigned char CurrentFlightMode() { return AsShotParameters(InstanceTable_Get())->selectedClub->flightMode; }
// Treat option results one and three as enabled, preserving the evaluator's encoding.
inline int ThinkOptionEnabled(void* thinker, int option) {
    switch (CComThinkCtrl_GetOption(thinker, option)) {
    case 1: case 3: return 1;
    case 2: default: return 0;
    }
}
// Saved subset within the simulation frame. Both exits use the same restore path:
// source                                 saved member            restore
// player position +1C                    savedPlayerPosition     12-byte copy (outside this struct)
// gauge shotPowerGaugeX..+3F              shotGauge               0x40-byte copy
// ball position / curve / spin           ballPosition/settings   vector copy / float stores
// world frame +21AC                      worldFrame              word copy; elapsedSeconds becomes 0
// ball power flags / ground triangle/type shotFlags/ground*       word / word / byte copies
// player +AE / +A0                       playerByteAE/WordA0      byte / word copies
// ball recoveryRequired +58              savedRecoveryRequired   restored as (saved != 0)
// shot-parameter powerShotLevel          powerShotWasEnabled     SAVED AS (level != 0), restored as 0/1
// Thus this is a subset restore, not a full rollback or an exact power-level snapshot.
// History, collision queries and animation state remain changed by simulation.
struct SavedLiveSimulationState {
    unsigned char shotGauge[0x40];
    Vec3W ballPosition;
    char pad390[4];
    float curveSetting, spinSetting;
    int worldFrame, shotFlags;
    char pad3A4[8];
    int groundTriangle;
    unsigned char playerByteAE; char pad3B1[3];
    int playerWordA0, savedRecoveryRequired, powerShotWasEnabled;
    unsigned char groundType;
};
typedef char SavedSimulationGroundOffset[((int)&((SavedLiveSimulationState*)0)->groundType == 0x7C) ? 1 : -1];
// Actor dispatch borrows this 0x20-byte request and its history sample.
// Player message 11 = kPlayerTestSwingDirections (CGolfPlayer_HandleMsg,
// golf_player_actor.cpp). scheduledTimeMs=0 admits immediate handling; sample begins with
// the world Vec3 read by that case. It probes eight headings around the supplied
// position and writes ball byte +AC with the COUNT of usable placements (0..8).
// This is a side effect on the live ball, not a history-output/position-set message.
// The actor lookup may return null, which skips the count update. The sample is
// borrowed only for synchronous dispatch and must not escape into a queued task.
enum { kSimulationTestSwingDirectionsMessage = 11 };
struct SimulationHistoryRequest {
    int scheduledTimeMs, playerMessage; void* sample;
    int word0C, word10, word14, word18, word1C;
};
// Typed simulation workspace; declaration order retains the original stack frame.
// Scratch vectors are world XYZ unless their names say local/horizontal; rotations are 3x4.
struct SimulationScratch {
    float diveSpeedCategory2, diveSpeedCategory1, diveSpeedCategory0;
    Vec3W finalHazardProbeRadius;
    Vec3W finalHazardProbePosition;
    Vec3W finalHazardRotatedOffset;
    Vec3W finalHazardTerrainQuery;
    Vec3W landingHazardProbeRadius;
    Vec3W landingHazardProbePosition;
    Vec3W landingHazardRotatedOffset;
    Vec3W landingHazardTerrainQuery;
    Vec3W targetDistanceHorizontal;
    Vec3W remainingDistanceHorizontal;
    Vec3W resultToAreaDelta;
    Vec3W resultToAreaHorizontal;
    Vec3W firstLandingToAreaDelta;
    Vec3W firstLandingToAreaHorizontal;
    Vec3W fallbackTargetCopy;
    Vec3W fallbackTargetGround;
    Vec3W fallbackDistanceDelta;
    Vec3W targetDistanceDelta;
    Vec3W resultTerrainQuery;
    Vec3W resultToTargetDelta;
    Vec3W waterQueryPosition;
    Vec3W special40LaunchVelocity;
    Vec3W special20LaunchVelocity;
    Vec3W forcedStopQuery;
    Vec3W forcedStopGround;
    Vec3W special40GroundQuery;
    Vec3W special20GroundQuery;
    Vec3W localLaunchVelocity;
    Vec3W worldLaunchVelocity;
    Vec3W mode2MissVelocity;
    Vec3W mode2ForwardMiss;
    Vec3W mode1MissVelocity;
    Vec3W mode1ForwardMiss;
    Vec3W mode0ForwardMiss;
    Vec3W special40TravelDelta;
    Vec3W special20TravelDelta;
    Vec3W specialLandingPosition;
    Vec3W firstLandingPosition;
    Vec3W savedPlayerPosition;
    char reserved1EC[4];
    SimulationHistoryRequest historyActorRequest;
    struct { Vec3W forward, side, up, fourth; } finalHazardRotation;
    struct { Vec3W forward, side, up, fourth; } landingHazardRotation;
    struct { Vec3W forward, side, up, fourth; } launchHeadingRotation;
    struct { Vec3W forward, side, up, fourth; } launchElevationRotation;
    struct { Vec3W forward, side, up, fourth; } mode2MissRotation;
    struct { Vec3W forward, side, up, fourth; } mode1MissRotation;
    char reserved330[0x14];
    SavedLiveSimulationState saved;
    char reservedTail[4];
};
typedef char SimulationScratchSize[sizeof(SimulationScratch)==0x3BC ? 1 : -1];
typedef char SimulationSavedOffset[(int)&((SimulationScratch*)0)->saved==0x338 ? 1 : -1];
#pragma pop

// Borrowed live views: only named saved/restored fields have established roles.
// Unknown player flags retain offset-based names rather than guessed semantics.
enum SimulationMotionTag { kSimulationStopped = 0, kSimulationFlying = 1, kSimulationMotion3 = 3 };
enum SimulationFlightMode { kFlightMode0 = 0, kFlightMode1 = 1, kFlightMode2 = 2, kFlightMode3 = 3 };
enum SimulationShotFlag { kSimulationShot10 = 0x10, kSimulationShot20 = 0x20, kSimulationShot40 = 0x40 };
enum SimulationClubId {
    kClubTypeIdBase = 0x04000000, kClubIdSuffix1 = 1, kClubIdSuffix5 = 5, kClubIdSuffix6 = 6
};
enum SimulationHistoryEvent { kHistoryFirstLanding = 1, kHistoryInvalidLanding = 2, kHistorySpecialLanding = 5 };
enum SimulationClubCategory { kSimulationCategory0 = 0, kSimulationCategory1 = 1, kSimulationCategory2 = 2 };
enum SimulationContract { kSimulationClubKind4 = 4, kSpecialDiveDelayFrames = 60, kStuckCollisionLimit = 10 };

struct SimulationWorldClock {
    char prefix[0x21A8];
    float elapsedSeconds;
    union { int frame; unsigned int frameBits; };
};
struct SimulationPlayerState {
    char prefix[0x1C]; Vec3f position;
    char pad028[0xA0 - 0x28]; int wordA0;
    char pad0A4[8]; short contactIdAC; unsigned char flagAE;
    char pad0AF[5]; int wordB4;
    char pad0B8[0xD8 - 0xB8]; int spinStat;
};
typedef char SimulationPlayerFlagOffset[((int)&((SimulationPlayerState*)0)->flagAE == 0xAE) ? 1 : -1];
typedef char SimulationPlayerSpinOffset[((int)&((SimulationPlayerState*)0)->spinStat == 0xD8) ? 1 : -1];
typedef char SimulationClockOffset[((int)&((SimulationWorldClock*)0)->frame == 0x21AC) ? 1 : -1];
typedef char SimulationHistoryRequestSize[sizeof(SimulationHistoryRequest) == 0x20 ? 1 : -1];

// Access the sampled motion tag.
static inline int& SampleMotionState(BallSampleView* sample) { return sample->motionState; }

// Save the live subset restored by both simulation exits; history/world time are excluded.
static inline void SaveLiveShotSimulationState(SimulationScratch& scratch,
        void*& player) {
    player=CurrentPlayerEntry();
    scratch.savedPlayerPosition=*(Vec3W*)&((PlayerPositionView*)player)->position;
    void* liveBallForSnapshot=GolfBall__Fv();
    scratch.saved.savedRecoveryRequired=AsGolfBall(liveBallForSnapshot)->m_recoveryRequired;
    scratch.saved.worldFrame=((SimulationWorldClock*)CQuadTree_GetInstance())->frame;
    __as__7WVectorFRC7WVector(&scratch.saved.ballPosition,liveBallForSnapshot);
    memcpy(scratch.saved.shotGauge,&CurrentGolfDoc()->shotPowerGaugeX,0x40);
    scratch.saved.curveSetting=AsGolfBall(liveBallForSnapshot)->m_initCurve;
    scratch.saved.spinSetting=AsGolfBall(liveBallForSnapshot)->m_initSpin;
    scratch.saved.shotFlags=AsGolfBall(liveBallForSnapshot)->m_powerFlags;
    scratch.saved.playerByteAE=((SimulationPlayerState*)CurrentPlayerEntry())->flagAE;
    scratch.saved.playerWordA0=((SimulationPlayerState*)CurrentPlayerEntry())->wordA0;
    scratch.saved.groundTriangle=((SimulatedBallView*)liveBallForSnapshot)->groundTriangle;
    scratch.saved.powerShotWasEnabled=AsShotParameters(InstanceTable_Get())->powerShotLevel!=0;
    scratch.saved.groundType=AsGolfBall(liveBallForSnapshot)->m_groundType;
}

// Restore the documented subset, including the normalized power level; reset world elapsed time.
// Caller variables are references so both exit paths retain their matched lifetimes.
static inline void RestoreLiveShotSimulationState(SimulationScratch& scratch,
        void*& liveBall, void*& player, unsigned int& simulationStartFrame, void*& world) {
    liveBall=GolfBall__Fv();
    AsGolfBall(liveBall)->m_recoveryRequired=scratch.saved.savedRecoveryRequired!=0;
    __as__7WVectorFRC7WVector(liveBall,&scratch.saved.ballPosition);
    player=CurrentPlayerEntry(); ((SimulationPlayerState*)player)->flagAE=scratch.saved.playerByteAE;
    player=CurrentPlayerEntry(); ((SimulationPlayerState*)player)->wordA0=scratch.saved.playerWordA0;
    AsGolfBall(liveBall)->m_initCurve=scratch.saved.curveSetting; AsGolfBall(liveBall)->m_initSpin=scratch.saved.spinSetting;
    AsGolfBall(liveBall)->m_powerFlags=scratch.saved.shotFlags; ((SimulatedBallView*)liveBall)->groundTriangle=scratch.saved.groundTriangle;
    AsGolfBall(liveBall)->m_groundType=scratch.saved.groundType;
    memcpy(&CurrentGolfDoc()->shotPowerGaugeX,scratch.saved.shotGauge,0x40);
    simulationStartFrame=scratch.saved.worldFrame; world=CQuadTree_GetInstance(); ((SimulationWorldClock*)world)->frame=simulationStartFrame; ((SimulationWorldClock*)world)->elapsedSeconds=kFloat_0_ComShot;
    AsShotParameters(InstanceTable_Get())->powerShotLevel=scratch.saved.powerShotWasEnabled;
}

// Keep frame/ABI accessor calls inlined through the additional phase boundary.
#pragma push
#pragma always_inline on
#pragma inline_depth(16)
// Impact/flight policy (local miss velocities are later rotated by the shot heading):
// impact > Miss: ordinary launch construction runs after this helper for EVERY mode.
// mode 3: set motion tag 3 and bypass all impact/lie rules here.
// Unset/Miss for modes 0..2:
// mode | gauge right of target | gauge at/left of target    | stop overrides
// 0    | zero, tag 0           | forward 15, tag 3, +40deg | suffix 1/6 on left; suffix 5 either side
// 1    | forward 20, tag 3     | 80deg rotation * 50, tag 1| suffix 6 or 5, either side
// 2    | forward 5, tag 3      | 80deg rotation * 20, tag 1| none
// Other modes keep the initial tag 1 and prior sample velocity, except suffix 5 stops.
// Mode 3 also leaves the prior velocity for Unset/Miss. Do not add a zeroing default.
// Requested flag 0x10 and live power flag 0x40 each multiply launch speed by 1.3;
// when both are set the factors compound. 0x20 has no initial speed multiplier.
// In the step loop, 0x20 and 0x40 have separate horizontal-launch/travel transitions.
// Club-kind-4 spin handling gives 0x20 precedence over 0x40 when both are active.
// Apply impact/lie corrections or the club-mode-specific miss velocity.
// Mutated live state: current's lie fields and sample velocity/motion.
// Numerical in/out: impactError, launchSpeed and launchHeadingCorrection.
// scratch vectors and good/poor impact references are temporary arithmetic storage;
// flyingState/stoppedMotionTag carry the existing motion values. No history is owned.
static inline void ApplySimulationImpactRules(SimulationScratch& scratch,
        SimulatedBallView* current, BallSampleView* state, int& stoppedMotionTag,
        int& flyingState, float& impactError, float& launchSpeed, float& launchHeadingCorrection,
        float& goodImpactRatio, float& goodImpactMagnitude, float& poorImpactMagnitude, float& poorImpactRatio) {
    if (CurrentFlightMode()==kFlightMode3) SampleMotionState(state)=kSimulationMotion3;
    else {
        flyingState=kSimulationFlying; SampleMotionState(state)=flyingState;
        switch (CurrentGolfDoc()->impactGrade) {
        case kImpactPangya: // Apply upper lie scale and slope heading correction.
            launchSpeed*=Math_SquareRootApprox(CQuadTree_GetMaxPowerOnGround(CQuadTree_GetInstance()));
            launchHeadingCorrection=FP_FNMSUBS(kFloat_0_34906584_ComShot, current->lieSlope, launchHeadingCorrection);
            break;
        case kImpactGood: { // Blend lie scales by impact error, then adjust curve.
            float v=ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get());
            if (impactError>kFloat_0_ComShot) goodImpactRatio=impactError/((kFloat_149_ComShot+v)-CurrentGolfDoc()->pangyaTargetX);
            else goodImpactRatio=impactError/(CurrentGolfDoc()->pangyaTargetX-(kFloat_149_ComShot-v));
            goodImpactMagnitude=goodImpactRatio>kFloat_0_ComShot?goodImpactRatio:-goodImpactRatio;
            impactError=goodImpactRatio>kFloat_0_ComShot?goodImpactRatio:-goodImpactRatio;
            goodImpactMagnitude*=CQuadTree_GetMinPowerOnGround(CQuadTree_GetInstance());
            launchSpeed*=Math_SquareRootApprox(FP_FMADDS((kFloat_1_Rival-impactError), CQuadTree_GetMaxPowerOnGround(CQuadTree_GetInstance()), goodImpactMagnitude));
            float old=current->lieSlope;
            launchHeadingCorrection=FP_FNMSUBS(kFloat_0_34906584_ComShot, old, launchHeadingCorrection);
            current->lieSlope=FP_FNMSUBS(kFloat_0_2_ComShot, goodImpactRatio, old);
            break;
        }
        case kImpactPoor: { // Apply poor-impact speed/heading and reset spin setting.
            poorImpactMagnitude=ShotParameters_GetGoodImpactHalfWidth(InstanceTable_Get());
            impactError=CurrentGolfDoc()->shotImpactGaugeX-kFloat_149_ComShot;
            impactError=impactError>kFloat_0_ComShot?impactError:-impactError;
            float v=ShotParameters_GetPoorImpactHalfWidth(InstanceTable_Get())-poorImpactMagnitude;
            poorImpactRatio=(impactError-poorImpactMagnitude)/v;
            poorImpactMagnitude=poorImpactRatio*CQuadTree_GetMinPowerOnGround(CQuadTree_GetInstance());
            launchSpeed*=Math_SquareRootApprox(FP_FMADDS((kFloat_1_Rival-poorImpactRatio), CQuadTree_GetMinPowerOnGround(CQuadTree_GetInstance()), poorImpactMagnitude));
            current->lieSlope=kFloat_0_2_ComShot; current->lieAngle=kFloat_0_ComShot;
            if (CurrentGolfDoc()->shotImpactGaugeX>kFloat_149_ComShot) {
                float flipped=current->lieSlope*kFloat_neg1_ComShot;
                launchHeadingCorrection=FP_FNMSUBS(kFloat_0_17453292_ComShot, poorImpactRatio, launchHeadingCorrection); current->lieSlope=flipped;
            } else FP_FMADDS_ACCUM(launchHeadingCorrection, kFloat_0_17453292_ComShot, poorImpactRatio);
            break;
        }
        case kImpactUnset: case kImpactMiss: // Select the club's miss/stop behavior.
            current->lieSlope=kFloat_0_ComShot; current->lieAngle=kFloat_0_ComShot;
            switch (CurrentFlightMode()) {
            case kFlightMode0: // Stop or use a low forward miss with heading correction.
                if (CurrentGolfDoc()->shotImpactGaugeX>CurrentGolfDoc()->pangyaTargetX) {
                    Vec3_Zero(&state->vel); SampleMotionState(state)=stoppedMotionTag;
                } else if ((unsigned)(CurrentClubTypeId()-kClubTypeIdBase)==kClubIdSuffix1 || (unsigned)(CurrentClubTypeId()-kClubTypeIdBase)==kClubIdSuffix6) {
                    Vec3_Zero(&state->vel); SampleMotionState(state)=kSimulationStopped;
                } else {
                    scratch.mode0ForwardMiss.v.x=kFloat_0_ComShot; scratch.mode0ForwardMiss.v.y=kFloat_0_ComShot; scratch.mode0ForwardMiss.v.z=kFloat_15_ComShot;
                    __as__7WVectorFRC7WVector(&state->vel,&scratch.mode0ForwardMiss); SampleMotionState(state)=kSimulationMotion3;
                    launchHeadingCorrection = launchHeadingCorrection + kFloat_0_6981317_ComShot;
                }
                break;
            case kFlightMode1: // Use a forward miss or the high-angle 50-speed miss.
                if (CurrentGolfDoc()->shotImpactGaugeX>CurrentGolfDoc()->pangyaTargetX) {
                    scratch.mode1ForwardMiss.v.x=kFloat_0_ComShot; scratch.mode1ForwardMiss.v.y=kFloat_0_ComShot; scratch.mode1ForwardMiss.v.z=kFloat_20_ComShot;
                    __as__7WVectorFRC7WVector(&state->vel,&scratch.mode1ForwardMiss); SampleMotionState(state)=kSimulationMotion3;
                } else {
                    Matrix_SetAxisRotation(&scratch.mode1MissRotation,kFloat_1_3962634_ComShot,0);
                    __ml__FfRC7WVector(&scratch.mode1MissVelocity,&scratch.mode1MissRotation.up,kFloat_50_ComShot);
                    __as__7WVectorFRC7WVector(&state->vel,&scratch.mode1MissVelocity); SampleMotionState(state)=flyingState;
                }
                if ((unsigned)(CurrentClubTypeId()-kClubTypeIdBase)==kClubIdSuffix6) {
                    Vec3_Zero(&state->vel); SampleMotionState(state)=kSimulationStopped;
                }
                break;
            case kFlightMode2: // Use a slow forward miss or high-angle 20-speed miss.
                if (CurrentGolfDoc()->shotImpactGaugeX>CurrentGolfDoc()->pangyaTargetX) {
                    scratch.mode2ForwardMiss.v.x=kFloat_0_ComShot; scratch.mode2ForwardMiss.v.y=kFloat_0_ComShot; scratch.mode2ForwardMiss.v.z=kFloat_5_ComShot;
                    __as__7WVectorFRC7WVector(&state->vel,&scratch.mode2ForwardMiss); SampleMotionState(state)=kSimulationMotion3;
                } else {
                    Matrix_SetAxisRotation(&scratch.mode2MissRotation,kFloat_1_3962634_ComShot,0);
                    __ml__FfRC7WVector(&scratch.mode2MissVelocity,&scratch.mode2MissRotation.up,kFloat_20_ComShot);
                    __as__7WVectorFRC7WVector(&state->vel,&scratch.mode2MissVelocity); SampleMotionState(state)=flyingState;
                }
                break;
            }
            if (CurrentFlightMode()!=kFlightMode2) { if ((unsigned)(CurrentClubTypeId()-kClubTypeIdBase)==kClubIdSuffix5) {
                Vec3_Zero(&state->vel); SampleMotionState(state)=kSimulationStopped;
            } }
            break;
        }
        if (CurrentGolfDoc()->impactGrade>kImpactMiss && !CurrentGolfDoc()->powerShotMeterOverride && CurrentGolfDoc()->impactGrade==kImpactPangya && ByteAt(CurrentGolfDoc(),0x2d8)) InstanceTable_Get();
    }

}

#pragma pop

#pragma push
#pragma always_inline on
#pragma inline_depth(16)
// Set the pre-scaling Y velocity for special-shot bit 0x40 after its descent delay.
// Categories are the stored club-category bytes; their gameplay names remain unverified.
// Each affine curve is baseline - slope*((referenceSpeed-min(cap,sqrt(power)*launchSpeed))/span).
// (referenceSpeed, span, slope, baseline): 0=(344,112,21.5,-8),
// 1=(306,105,20.5,-10.3), 2=(273,100,20.2,-10.8), in existing world/time units.
// The caller subsequently scales the WHOLE velocity by 7*sqrt(power). These empirical
// category curves are separate tuning sets; no physical meaning for their coefficients is recovered.
static inline void ApplyCategorySpecificDiveVelocity(SimulationScratch& scratch,
        SimulatedBallView* current, float& normalizedPower, float& scale) {
    switch (AsShotParameters(InstanceTable_Get())->selectedClub->category) {
    case kSimulationCategory0: {
        scale=ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
        float powerSpeedFactor=Math_SquareRootApprox(normalizedPower); powerSpeedFactor*=scale;
        scratch.diveSpeedCategory0=powerSpeedFactor;
        float* v=s_comShotDiveSpeedCapClubCategory0<scratch.diveSpeedCategory0?&s_comShotDiveSpeedCapClubCategory0:(float*)&scratch.diveSpeedCategory0;
        current->vel.v.y=FP_FNMSUBS(kFloat_21_5_ComShot, ((kFloat_344_ComShot-*v)/kFloat_112_ComShot), kFloat_neg8_ComShot);
        break;
    }
    case kSimulationCategory1: {
        scale=ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
        float powerSpeedFactor=Math_SquareRootApprox(normalizedPower); powerSpeedFactor*=scale; scratch.diveSpeedCategory1=powerSpeedFactor;
        float* v=s_comShotDiveSpeedCapClubCategory1<scratch.diveSpeedCategory1?&s_comShotDiveSpeedCapClubCategory1:(float*)&scratch.diveSpeedCategory1;
        current->vel.v.y=FP_FNMSUBS(kFloat_20_5_ComShot, ((kFloat_306_ComShot-*v)/kFloat_105_ComShot), kFloat_neg10_299999_ComShot);
        break;
    }
    case kSimulationCategory2: {
        scale=ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
        float powerSpeedFactor=Math_SquareRootApprox(normalizedPower); powerSpeedFactor*=scale; scratch.diveSpeedCategory2=powerSpeedFactor;
        float* v=s_comShotDiveSpeedCapClubCategory2<scratch.diveSpeedCategory2?&s_comShotDiveSpeedCapClubCategory2:(float*)&scratch.diveSpeedCategory2;
        current->vel.v.y=FP_FNMSUBS(kFloat_20_2_ComShot, ((kFloat_273_ComShot-*v)/kFloat_100_0_ComShot), kFloat_neg10_799999_ComShot);
        break;
    }
    }
}

// Advance the borrowed live ball until rest; write history and candidate landing flags.
// Persistent outputs: current/candidate contents and firstLandingSeen/invalidLanding.
// Arithmetic/query storage: scratch, scale, record and queryResult. launchSpeed,
// normalizedPower and clearedStateWord are inputs retained by reference for MWCC lifetimes.
static inline void RunCandidateBallSimulation(SimulationScratch& scratch,
        SimulatedBallView* current, ShotSimulationResult* candidate, void* rival,
        float& launchSpeed, float& normalizedPower, float& scale, void*& record,
        int& clearedStateWord, int& firstLandingSeen, int& invalidLanding, int& queryResult) {
    // Phase 3: fixed-step simulation; record contacts, landings and special-shot transitions.
    while (current->motionState) {
        record=CGolfBall_NewPreData(current);
        ((BallRecordView*)record)->event20=clearedStateWord; ((BallRecordView*)record)->event60=(int)clearedStateWord; ((BallRecordView*)record)->event61=(int)clearedStateWord;
        CQuadTree_StepBallSimulation(CQuadTree_GetInstance(),kComShotSimulationStepSeconds);
        if (AsShotParameters(InstanceTable_Get())->clubKind==kSimulationClubKind4) {
            if (current->activeShotFlags&kSimulationShot20) {
                if (0>current->special20TransitionFrame) {
                    scale=current->radius; scratch.special20GroundQuery=*(Vec3W*)current;
                    float h=CQuadTree_ResolveGroundHeight(CQuadTree_GetInstance(),&scratch.special20GroundQuery,kFloat_2_ComShot*scale);
                    current->pos.v.y=current->pos.v.y<h?h:current->pos.v.y;
                } else current->spinImpulse-=kComShotSimulationStepSeconds;
            } else if (current->activeShotFlags&kSimulationShot40) {
                if (current->special40LaunchFrame<0) {
                    scale=current->radius; scratch.special40GroundQuery=*(Vec3W*)current;
                    float h=CQuadTree_ResolveGroundHeight(CQuadTree_GetInstance(),&scratch.special40GroundQuery,kFloat_2_ComShot*scale);
                    current->pos.v.y=current->pos.v.y<h?h:current->pos.v.y;
                } else current->spinImpulse-=kComShotSimulationStepSeconds;
            } else if (current->firstLandingFrame<0 && CQuadTree_GetObjColNum(CQuadTree_GetInstance())==0) {
                current->spinImpulse=FP_FNMSUBS(kComShotSimulationStepSeconds, FP_FNMSUBS(kFloat_0_1_U801DD078, current->lieAngle, kFloat_0_5_ComShot), current->spinImpulse);
            } else current->spinImpulse-=kFloat_0_0033333336_ComShot;
        }
        // Termination guard: force a grounded stop at 1800 frames, or after 900 if stuck.
        if (current->frameCount>=kMaxSimulationFrames || (current->frameCount>=kSlowSimulationFrames &&
            (Magnitude__7WVectorCFv(&current->vel)<kFloat_0_1_U801DD078 || CQuadTree_GetObjColNum(CQuadTree_GetInstance()) >=kStuckCollisionLimit || CQuadTree_GetMeshIndex(CQuadTree_GetInstance(),current,0)==-1))) {
            Vec3_Zero(&current->vel);
            scratch.forcedStopQuery=*(Vec3W*)current;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(&scratch.forcedStopGround,CQuadTree_GetInstance(),&scratch.forcedStopQuery,0,0,0);
            __as__7WVectorFRC7WVector(current,&scratch.forcedStopGround);
            float ballRadius = current->radius; current->pos.v.y+=ballRadius; current->motionState=(int)clearedStateWord;
        }
        __as__7WVectorFRC7WVector(record,current);
        __as__7WVectorFRC7WVector(&((BallRecordView*)record)->velocity,&current->vel);
        Vec2_Copy(((BallRecordView*)record)->rotation,&current->rotationX);
        ((BallRecordView*)record)->motionState=current->motionState;
        // Contact codes: 1 first ordinary landing, 2 invalid landing, 5 special landing.
        switch (((BallRecordView*)record)->event60) {
        case kHistorySpecialLanding: // Save the first special-landing position.
            if (!candidate->hasSpecialLanding) { __as__7WVectorFRC7WVector(&scratch.specialLandingPosition,current); candidate->hasSpecialLanding=1; }
            break;
        case kHistoryFirstLanding: // Save the first landing and validate its target/slope.
            if (!firstLandingSeen) {
                firstLandingSeen=1;
                __mi__FRC7WVectorRC7WVector(&scratch.firstLandingToAreaDelta,&((RivalTargetAreaView*)rival)->targetArea->position,current);
                scratch.firstLandingToAreaHorizontal=scratch.firstLandingToAreaDelta; scratch.firstLandingToAreaHorizontal.v.y=kFloat_0_ComShot;
                candidate->firstLandingWithinTarget=((RivalTargetAreaView*)rival)->targetArea->radius>=WVectorLen__FRC7WVector(&scratch.firstLandingToAreaHorizontal);
                __as__7WVectorFRC7WVector(&scratch.firstLandingPosition,current);
                queryResult=CQuadTree_GetMeshIndex(CQuadTree_GetInstance(),current,0);
                if (*(unsigned char*)&AsCRival(rival)->difficultyEnabled && CQuadTree_GetGroundSlope(CQuadTree_GetInstance(),queryResult)>kFloat_0_7853982_ComShot) invalidLanding=1;
            }
            break;
        case kHistoryInvalidLanding: // Mark this landing unusable and retain its position.
            invalidLanding=1; __as__7WVectorFRC7WVector(&scratch.firstLandingPosition,current); candidate->hasInvalidLanding=1;
            break;
        }
        if (!current->motionState) current->activeShotFlags&=kPersistingSpecialShotMask;
        if (kSimulationFlying==current->motionState && current->vel.v.y<kFloat_0_ComShot && current->descentStartFrame<0) current->descentStartFrame=current->frameCount;
        if (current->special10EndFrame<0 && (current->requestedShotFlags&kSimulationShot10) && !(current->activeShotFlags&kSimulationShot10)) current->special10EndFrame=current->frameCount;
        if (current->activeShotFlags&kSimulationShot20) {
            if (current->frameCount==1) {
                current->vel.v.y=kFloat_0_ComShot;
                void* norm=Normalize__7WVectorFv(&current->vel);
                __ml__FfRC7WVector(&scratch.special20LaunchVelocity,norm,launchSpeed);
                __as__7WVectorFRC7WVector(&current->vel,&scratch.special20LaunchVelocity);
            }
            __mi__FRC7WVectorRC7WVector(&scratch.special20TravelDelta,current,PreData__9CGolfBallFi(current,0));
            scratch.special20TravelDelta.v.y=kFloat_0_ComShot;
            if (0>current->special20TransitionFrame) {
                scale=GolfParameter_ResolveFloat(InstanceTable_Get(),0);
                float powerSpeedFactor=Math_SquareRootApprox(normalizedPower);
                scale=kFloat_3_2_ComShot*FP_FMSUBS(powerSpeedFactor, scale, kFloat_100_0_ComShot);
                if (Magnitude__7WVectorCFv(&scratch.special20TravelDelta)>scale) {
                    current->special20TransitionFrame=current->frameCount;
                    Normalize__7WVectorFv(&current->vel);
                    switch (AsShotParameters(InstanceTable_Get())->selectedClub->category) {
                    case kSimulationCategory0: __amu__7WVectorFf(&current->vel,kFloat_74_ComShot); break;
                    case kSimulationCategory1: __amu__7WVectorFf(&current->vel,kFloat_76_ComShot); break;
                    case kSimulationCategory2: __amu__7WVectorFf(&current->vel,kFloat_80_ComShot); break;
                    }
                    __amu__7WVectorFf(&current->vel,Math_SquareRootApprox(normalizedPower));
                    current->spinImpulse=kFloat_2_5_ComShot;
                }
            }
        }
        if (0>current->special40EndFrame && (current->requestedShotFlags&kSimulationShot40) && !(current->activeShotFlags&kSimulationShot40) && current->special40DiveFrame>0) current->special40EndFrame=current->frameCount;
        if (current->activeShotFlags&kSimulationShot40) {
            if (current->frameCount==1) {
                current->vel.v.y=kFloat_0_ComShot;
                void* norm=Normalize__7WVectorFv(&current->vel);
                __ml__FfRC7WVector(&scratch.special40LaunchVelocity,norm,launchSpeed);
                __as__7WVectorFRC7WVector(&current->vel,&scratch.special40LaunchVelocity);
            }
            __mi__FRC7WVectorRC7WVector(&scratch.special40TravelDelta,current,PreData__9CGolfBallFi(current,0));
            scratch.special40TravelDelta.v.y=kFloat_0_ComShot;
            if (current->special40LaunchFrame<0) { if (Magnitude__7WVectorCFv(&scratch.special40TravelDelta)>kFloat_0_ComShot) {
                current->special40LaunchFrame=current->frameCount;
                Normalize__7WVectorFv(&current->vel);
                __amu__7WVectorFf(&current->vel,kFloat_72_5_ComShot);
                __amu__7WVectorFf(&current->vel,kFloat_2_ComShot*Math_SquareRootApprox(normalizedPower));
                current->spinImpulse=kFloat_3_1_ComShot;
            } }
            if (current->descentStartFrame>=0) { if (current->frameCount>current->descentStartFrame+kSpecialDiveDelayFrames) { if (0>current->special40DiveFrame) {
                current->special40DiveFrame=current->frameCount;
                ApplyCategorySpecificDiveVelocity(scratch, current, normalizedPower, scale);
                __amu__7WVectorFf(&current->vel,kFloat_7_ComShot*Math_SquareRootApprox(normalizedPower));
                current->spinImpulse=current->lieAngle;
            } } }
        }
    }

}
#pragma pop

// rival points to CRival; candidate/target must be nonnull. Inputs are
// headingRadians/power and live shot configuration; other candidate fields are
// outputs. Returns 0 for an accepted simulation, 1 for rejection. This is not pure:
// it rewrites sample history and touches collision/animation state. Both exits
// restore only the explicitly saved live ball/player/gauge/world-frame subset.
// Accepted remainingDistance is horizontal; rejected fallback distance is 3D to
// ground under target. Do not treat those distances as interchangeable metrics.
// Save live shot state, simulate the candidate to rest, record results, and restore live state.
#pragma push
#pragma always_inline on
#pragma inline_depth(16)
// Extract horizontal target metrics, terrain and completion flags from an accepted candidate.
// playerPosition is borrowed; this phase reads live ball state but does not restore it.
static inline void RecordAcceptedCandidateMetrics(SimulationScratch& scratch,
        void* rival, ShotSimulationResult* candidate, Vec3* target,
        SimulatedBallView* current, void* playerPosition) {
    __mi__FRC7WVectorRC7WVector(&scratch.resultToAreaDelta,&((RivalTargetAreaView*)rival)->targetArea->position,&candidate->result);
    scratch.resultToAreaHorizontal=scratch.resultToAreaDelta; scratch.resultToAreaHorizontal.v.y=kFloat_0_ComShot;
    candidate->finalWithinTarget=((RivalTargetAreaView*)rival)->targetArea->radius>=WVectorLen__FRC7WVector(&scratch.resultToAreaHorizontal);
    __mi__FRC7WVectorRC7WVector(&scratch.resultToTargetDelta,target,&candidate->result);
    scratch.remainingDistanceHorizontal=scratch.resultToTargetDelta; scratch.remainingDistanceHorizontal.v.y=kFloat_0_ComShot;
    candidate->remainingDistance=WVectorLen__FRC7WVector(&scratch.remainingDistanceHorizontal);
    candidate->rejected=current->recoveryRequired;
    scratch.resultTerrainQuery=*(Vec3W*)playerPosition;
    candidate->groundType=GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(),&scratch.resultTerrainQuery);
    // Classification and frame count belong to this simulated candidate.
    candidate->holed=current->encoded.Get()==1;
    candidate->simulationFrames=current->frameCount; candidate->nearHazard=0;
}

// Simulate one borrowed candidate, write its metrics, then restore the documented live subset.
int ComShot_SimulateTrajectory(void* rival,ShotSimulationResult* candidate,Vec3* target) {
    // Declare the borrowed landing sample first to retain register allocation without aliasing rival.
    void* landingSample;
    unsigned int unusedEventFlags;
    float headingRadians;
    // Named typed storage retains 12-byte vector copies and the original frame layout.
    SimulationScratch scratch;

    float normalizedPower; float launchSpeed; float impactError; int clearedStateWord;

    float scale;
    SimulatedBallView* current;
    BallSampleView* state;
    unsigned char savedGroundType;
    int queryResult; int firstLandingSeen;
    int invalidLanding, flyingState;
    void* record;
    // Phase 1: snapshot the live fields that both exits must restore.
    void* player;
    SaveLiveShotSimulationState(scratch, player);
    // Phase 2: derive launch velocity from normalized power, impact, lie and club mode.
    CRival_PrepareShotGauge(rival,candidate->power,kFloat_0_ComShot);
    headingRadians=candidate->headingRadians; normalizedPower=candidate->power;
    current=GolfBall__Fv();
    impactError=CurrentGolfDoc()->shotImpactGaugeX-CurrentGolfDoc()->pangyaTargetX;
    scale=ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
    launchSpeed=Math_SquareRootApprox(normalizedPower);
    launchSpeed*=scale;
    float launchHeadingCorrection; launchHeadingCorrection=kFloat_0_ComShot;
    if (current->requestedShotFlags&kSimulationShot10) launchSpeed*=kFloat_1_3_ComShot;
    if (*(unsigned int*)&AsGolfBall(GolfBall__Fv())->m_powerFlags&kSimulationShot40) launchSpeed*=kFloat_1_3_ComShot;
    state=CGolfBall_GetOrAllocatePreDataAt(current,0);
    invalidLanding=0; state->b60=invalidLanding;
    __as__7WVectorFRC7WVector(state,current);
    // Caller-owned impact intermediates preserve MWCC's floating-register assignment.
    float goodImpactRatio, goodImpactMagnitude, poorImpactMagnitude, poorImpactRatio;
    ApplySimulationImpactRules(scratch, current, state, invalidLanding, flyingState,
        impactError, launchSpeed, launchHeadingCorrection,
        goodImpactRatio, goodImpactMagnitude, poorImpactMagnitude, poorImpactRatio);
    float invert=kFloat_neg1_ComShot;
    float curve=current->lieSlope;
    launchHeadingCorrection*=invert; current->lieSlope=curve*invert;
    if (CurrentGolfDoc()->impactGrade>kImpactMiss) {
        scale=current->lieSlope;
        scale=scale>kFloat_0_ComShot?scale:-scale;
        Matrix_SetAxisRotation(&scratch.launchHeadingRotation,headingRadians+launchHeadingCorrection,1);
        float angle=ClubSelection_GetLaunchAngle(InstanceTable_Get(),0);
        Matrix_SetAxisRotation(&scratch.launchElevationRotation,angle,0);
        __ml__FfRC7WVector(&scratch.localLaunchVelocity,&scratch.launchElevationRotation.up,launchSpeed*FP_FMADDS(kFloat_0_1_U801DD078, scale, kFloat_1_Rival));
        Matrix_TransformPoint(&scratch.worldLaunchVelocity,&scratch.localLaunchVelocity,&scratch.launchHeadingRotation);
        __as__7WVectorFRC7WVector(&state->vel,&scratch.worldLaunchVelocity);
    }
    float worldFrameBeforeSimulation; worldFrameBeforeSimulation=(float)((SimulationWorldClock*)CQuadTree_GetInstance())->frameBits;
    unsigned int simulationStartFrame=Runtime_ConvertDoubleToUnsigned(AsCRival(rival)->simulationStartFrame);
    void* world=CQuadTree_GetInstance();
    ((SimulationWorldClock*)world)->frameBits=simulationStartFrame; ((SimulationWorldClock*)world)->elapsedSeconds=kFloat_0_ComShot;
    __as__7WVectorFRC7WVector(&current->vel,&state->vel);
    current->motionState=SampleMotionState(state); current->activeShotFlags=current->requestedShotFlags; current->lie=0;
    clearedStateWord=0;

    current->SetEncoded(current->lie);
    { void* pl=CurrentPlayerEntry(); ((SimulationPlayerState*)pl)->contactIdAC=-1; } ((SimulationPlayerState*)CurrentPlayerEntry())->wordB4=(int)clearedStateWord;
    if (current->motionState) ((SimulationPlayerState*)CurrentPlayerEntry())->flagAE=(int)clearedStateWord;
    CQuadTree_ResetPerShotSimulationState(CQuadTree_GetInstance()); CGolfBall_ClearBuffer(current);
    current->frameCount=0;
    current->firstLandingFrame=-1; current->descentStartFrame=-1; current->eventFrame74=-1;
    current->eventFrame78=-1; current->eventFrame7C=-1; current->eventFrame80=-1;
    current->eventFrame8C=-1; current->eventFrame84=-1;
    current->eventFrame88=(current->motionState==kSimulationMotion3)?0:-1;
    current->shotStage=0; current->bounceCount=0;
    current->firstContactIndex=-1; current->firstContactId=-1; current->lastContactIndex=-1; current->lastContactId=-1;
    current->groundTriangle=-1; current->special20TransitionFrame=-1; current->special10EndFrame=-1;
    Vec3_Zero(&current->spinImpulseVector);
    current->special40LaunchFrame=-1; current->special40DiveFrame=-1; current->special40EndFrame=-1;
    current->recoveryRequired=0;
    savedGroundType=current->groundType;
    if (CurrentGolfDoc()->impactGrade==kImpactMiss) {
        current->spinImpulse=kFloat_0_ComShot; current->curveImpulse=kFloat_0_ComShot;
    } else {
        if (AsShotParameters(InstanceTable_Get())->clubKind==kSimulationClubKind4) {
            scale=kFloat_0_5_ComShot*CSharedDoc_ComputeAuxPartModifier(g_sharedDoc,CurrentGolfDoc()->currentPlayer,0);
            float v=(scale+(float)((SimulationPlayerState*)CurrentPlayerEntry())->spinStat)/kFloat_170_ComShot;
            v=kFloat_0_ComShot<v?v:kFloat_0_ComShot;
            current->spinImpulse=normalizedPower*(normalizedPower*(kFloat_1_5_ComShot+v));
        } else current->spinImpulse=kFloat_0_ComShot;
        current->curveImpulse=current->lieSlope*normalizedPower;
    }
    firstLandingSeen=0; invalidLanding=0; unusedEventFlags=0; flyingState=kSimulationFlying;
    candidate->hasSpecialLanding=0; candidate->hasInvalidLanding=0;
    // Phase 3: fixed-step simulation records landings and special-shot transitions.
    RunCandidateBallSimulation(scratch, current, candidate, rival, launchSpeed,
        normalizedPower, scale, record, clearedStateWord, firstLandingSeen, invalidLanding, queryResult);
    // Phase 4: validate the rest state and select final/first/special result position.
    current->groundType=savedGroundType;
    current->activeShotFlags=current->requestedShotFlags;
    // Original unit 0x801DD078: failed checks share the fallback below.
    do {
        if (current->recoveryRequired || (invalidLanding && *(unsigned char*)&AsCRival(rival)->difficultyEnabled)) break;
        queryResult=ThinkOptionEnabled(AsCRival(rival)->thinkController,kThinkOptionSpecialLanding);
        if (queryResult && candidate->hasSpecialLanding == 0) break;
        if (CQuadTree_GetObjColNum(CQuadTree_GetInstance())>0) current->eventFrame74=-1;
        world=CQuadTree_GetInstance();
        ((SimulationWorldClock*)world)->frameBits=Runtime_ConvertDoubleToUnsigned(worldFrameBeforeSimulation-kFloat_1_Rival);
        ((SimulationWorldClock*)world)->elapsedSeconds=kFloat_0_ComShot;
        CQuadTree_UpdateAnimatedObjects(CQuadTree_GetInstance(),kComShotSimulationStepSeconds,1);
        current->curveImpulse=current->lieSlope; current->spinImpulse=current->lieAngle;
        current->simulatedSeconds=kComShotSimulationStepSeconds*(float)current->frameCount;
        if (current->recoveryRequired || CourseData_IsPointOutsidePlayableRegion(CurrentGolfDoc()->courseData,current)) break;
        {
            void* manager=g_taskManager?Object_GetChild(g_taskManager):0;
            ((SimulationHistoryRequest*)&scratch.historyActorRequest)->sample=PreData__9CGolfBallFi(current,current->frameCount);
            ((SimulationHistoryRequest*)&scratch.historyActorRequest)->scheduledTimeMs=0; ((SimulationHistoryRequest*)&scratch.historyActorRequest)->playerMessage=kSimulationTestSwingDirectionsMessage;
            ((SimulationHistoryRequest*)&scratch.historyActorRequest)->word0C=0; ((SimulationHistoryRequest*)&scratch.historyActorRequest)->word10=0; ((SimulationHistoryRequest*)&scratch.historyActorRequest)->word14=0; ((SimulationHistoryRequest*)&scratch.historyActorRequest)->word18=0; ((SimulationHistoryRequest*)&scratch.historyActorRequest)->word1C=0;
            Receiver* receiver=(Receiver*)TaskActorTable_FindByName(manager,comShotDebugStrings);
            if (receiver) receiver->dispatch(&scratch.historyActorRequest);
        }
        void* playerPosition; playerPosition=(char*)CurrentPlayerEntry()+0x1c;
        if (current->recoveryRequired || (current->encoded.Get()!=1 && !current->bounceCount)) break;
        __as__7WVectorFRC7WVector(playerPosition,PreData__9CGolfBallFi(current,current->frameCount));
        scratch.waterQueryPosition=*(Vec3W*)playerPosition;
        if (QuadTree_IsWaterAtPoint(CQuadTree_GetInstance(),&scratch.waterQueryPosition)) break;
        queryResult=ThinkOptionEnabled(AsCRival(rival)->thinkController,kThinkOptionFirstLanding);
        if (queryResult) __as__7WVectorFRC7WVector(&candidate->result,&scratch.firstLandingPosition);
        else {
            queryResult=ThinkOptionEnabled(AsCRival(rival)->thinkController,kThinkOptionSpecialLanding);
            if (queryResult) __as__7WVectorFRC7WVector(&candidate->result,&scratch.specialLandingPosition);
            else __as__7WVectorFRC7WVector(&candidate->result,current);
        }
        RecordAcceptedCandidateMetrics(scratch, rival, candidate, target, current, playerPosition);
        __mi__FRC7WVectorRC7WVector(&scratch.targetDistanceDelta,target,&scratch.savedPlayerPosition);
        scratch.targetDistanceHorizontal=scratch.targetDistanceDelta; scratch.targetDistanceHorizontal.v.y=kFloat_0_ComShot;
        queryResult=(int)WVectorLen__FRC7WVector(&scratch.targetDistanceHorizontal);
        if (*(unsigned char*)&AsCRival(rival)->difficultyEnabled) { if (((RivalTargetAreaView*)rival)->targetArea->index!=-1 || (float)queryResult>kFloat_192_ComShot) {
                int difficultyLevel; difficultyLevel=*(signed char*)((char*)g_sharedDoc+CurrentGolfDoc()->currentPlayer*0x1d4+0x1b2);
                // Keep the second probe conditional on the first probe's result.
                do {
                    if (current->firstLandingFrame>0) {
                        // Borrow the landing-history position for radial hazard probes.
                        landingSample = PreData__9CGolfBallFi(current,current->firstLandingFrame);
                        scale=kFloat_0_7853982_ComShot;
                        scratch.landingHazardProbeRadius.Set(kFloat_3_2_ComShot*(float)(18-2*difficultyLevel),kFloat_0_ComShot,kFloat_0_ComShot);
                        for (unsigned int n=0;n<kRadialProbeCount || (queryResult=0,false);++n) {
                            Matrix_SetAxisRotation(&scratch.landingHazardRotation,scale*(float)(int)n,1);
                            Matrix_TransformPoint(&scratch.landingHazardRotatedOffset,&scratch.landingHazardProbeRadius,&scratch.landingHazardRotation);
                            __pl__FRC7WVectorRC7WVector(&scratch.landingHazardProbePosition,landingSample,&scratch.landingHazardRotatedOffset); scratch.landingHazardTerrainQuery=scratch.landingHazardProbePosition;

                            if ((unsigned)((GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(),&scratch.landingHazardTerrainQuery))-8)<=1) { queryResult=1; break; }
                            if (CourseData_IsPointOutsidePlayableRegion(CurrentGolfDoc()->courseData,&scratch.landingHazardProbePosition)) { queryResult=1; break; }
                        }
                        if (queryResult) { candidate->nearHazard=1; break; }
                    }
                    {
                        scale=kFloat_0_7853982_ComShot;
                        scratch.finalHazardProbeRadius.Set(kFloat_3_2_ComShot*(float)(12-2*difficultyLevel),kFloat_0_ComShot,kFloat_0_ComShot);
                        for (unsigned int n=0;n<kRadialProbeCount || (queryResult=0,false);++n) {
                            Matrix_SetAxisRotation(&scratch.finalHazardRotation,scale*(float)(int)n,1);
                            Matrix_TransformPoint(&scratch.finalHazardRotatedOffset,&scratch.finalHazardProbeRadius,&scratch.finalHazardRotation);
                            __pl__FRC7WVectorRC7WVector(&scratch.finalHazardProbePosition,playerPosition,&scratch.finalHazardRotatedOffset); scratch.finalHazardTerrainQuery=scratch.finalHazardProbePosition;
                            unsigned char terrain=GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(),&scratch.finalHazardTerrainQuery);
                            if ((unsigned)(terrain-8)<=1) { queryResult=1; break; }
                            if (CourseData_IsPointOutsidePlayableRegion(CurrentGolfDoc()->courseData,&scratch.finalHazardProbePosition)) { queryResult=1; break; }
                        }
                        if (queryResult) candidate->nearHazard=1;
                    }
                } while (0);
        } }
        // Accepted exit: restore saved live fields after extracting candidate results.
        __as__7WVectorFRC7WVector(current,state);
        __as__7WVectorFRC7WVector(&current->vel,&state->vel);
        current->groundTriangle=CQuadTree_GetMeshIndex(CQuadTree_GetInstance(),current,0);
        // Restore the live ball only after extracting the candidate result.
        void* liveBall;
        RestoreLiveShotSimulationState(scratch, liveBall, player, simulationStartFrame, world);
        __as__7WVectorFRC7WVector((char*)CurrentPlayerEntry()+0x1c,&scratch.savedPlayerPosition);
        return candidate->rejected;
    } while (0);
    // Rejected exit: keep a fallback position/distance, restore the same live subset.
    if (candidate->hasInvalidLanding) __as__7WVectorFRC7WVector(&candidate->result,&scratch.firstLandingPosition);
    else __as__7WVectorFRC7WVector(&candidate->result,current);
    {
        enum PositionWord { kPositionWordZero = 0, kPositionWordAllBits = 0xFFFFFFFFu };
        PositionWord vz, vy, vx;
        vx = (PositionWord)((SimulationVectorWords*)target)->word0;
        vy = (PositionWord)((SimulationVectorWords*)target)->word1;
        vz = (PositionWord)((SimulationVectorWords*)target)->word2;
        ((SimulationVectorWords*)&scratch.fallbackTargetCopy)->word0 = vx;
        ((SimulationVectorWords*)&scratch.fallbackTargetCopy)->word1 = vy;
        ((SimulationVectorWords*)&scratch.fallbackTargetCopy)->word2 = vz;
    }
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&scratch.fallbackTargetGround,CQuadTree_GetInstance(),&scratch.fallbackTargetCopy,0,0,0);
    __mi__FRC7WVectorRC7WVector(&scratch.fallbackDistanceDelta,&scratch.fallbackTargetGround,&candidate->result);
    candidate->remainingDistance=WVectorLen__FRC7WVector(&scratch.fallbackDistanceDelta);
    // Restore the live ball only after extracting the candidate result.
    void* liveBall;
    RestoreLiveShotSimulationState(scratch, liveBall, player, simulationStartFrame, world);
    candidate->rejected=1;
    __as__7WVectorFRC7WVector((char*)CurrentPlayerEntry()+0x1c,&scratch.savedPlayerPosition);
    return 1;
}
}
#pragma pop
} // namespace ns3

// ---- from com_shot_cpu_rival_ai.cpp ----
namespace ns4 {
extern "C" {

// Context adapter: +40 shot-position thinker, +54 target-area context;
// nested +30 supplies its priority source. Shared document +B90 is GolfDoc,
// whose +3C1 byte selects the current player. All links are borrowed.
struct RivalShotPositionContextView {
    char pad_00[0x40];
    void* shotPositionThinker;
    char pad_44[0x10];
    struct TargetAreaPriorityView* targetAreaContext;
};

struct TargetAreaPriorityView {
    char pad_00[4];
    void* field04;
    char pad_08[0x28];
    void* prioritySource;
};

struct SharedGolfDocContextView { char pad_00[0xB90]; void* golfDoc; };
struct GolfDocPlayerSelectionView { char pad_00[0x3C1]; unsigned char currentPlayer; };
extern CSharedDoc* g_sharedDoc;
void CComThinkShotPos_AddContextPriorityBonus(void*, void*, unsigned char);

// Configure the shot-position thinker with current target data and the current player.
void CRival_SetShotPositionCandidateContext(RivalShotPositionContextView* self) {
    CComThinkShotPos_AddContextPriorityBonus(self->shotPositionThinker, self->targetAreaContext->prioritySource,
                ((GolfDocPlayerSelectionView*)(((SharedGolfDocContextView*)g_sharedDoc)->golfDoc))->currentPlayer);
}

struct PuttCandidateView { float aimAngle, remainingDistance, power, curve; char positionStorage[0x0C]; int rejected; unsigned char groundType; char pad21[3]; unsigned nearHazard, firstLandingWithinTarget, finalWithinTarget, hasSpecialLanding, hasInvalidLanding, holed, simulationFrames; };
struct PuttRivalTargetView { char pad_00[0x10]; float targetX, targetY, targetZ, holeX; };
struct PuttInitializationView { char pad_00[0x0C]; void* correctionAxis; PuttCandidateView* candidate; void* directionIndex; PuttRivalTargetView* rival; };
struct PowerCandidateStatus { int firstWord; float remainingDistance; char pad_08[0x14]; int rejected; };
struct PowerCorrectionIterationView {
    char pad_00[0x0C];
    struct { char pad_00[0x0C]; int powerSampleCount; }* config;
    unsigned rival;
    unsigned sampleIndex;
    PowerCandidateStatus* bestCandidate;
};
extern const float kCpuShotHorizontalStopToleranceWorld;
// Stop power sampling for a short valid shot or after the configured sample count.
// remainingDistance is |target-result| with Y=0 for accepted simulations
// (RecordAcceptedCandidateMetrics); rejected fallback uses a 3D ground delta,
// but rejected!=0 prevents that distance from satisfying this early stop.
// Baseline 0.25 is WORLD distance, not yards or a power fraction: the marker
// convention in spot_marker_draw_and_messages.cpp uses 3.2 world units/yard, so this is 0.078125 yard
// (~7.14 cm). Strict < accepts a candidate inside that horizontal error radius.
// Raising tolerance stops earlier with coarser power search; lowering spends
// more samples. Equality continues; the configured count still bounds work.
// No tested tuning range is established; retain nonnegative distance semantics.
int CComShotPowerCorrection_NextPowerSample(PowerCorrectionIterationView* self) {
    PowerCandidateStatus* inner = self->bestCandidate;
    if (inner->rejected == 0 && inner->remainingDistance < kCpuShotHorizontalStopToleranceWorld)
        return 0;
    int value = ++self->sampleIndex;
    return value < self->config->powerSampleCount;
}

// Constructor alias map from CRiver_BuildTerrainStrips/LoadTextureLayers in river_lava_texture_layers.cpp:
// +08/+0C/+10 = owned path-node vector data/count/capacity; half-width at +04
// is deliberately not initialized here. Texture layers occupy +14 and +30,
// each 0x1C bytes: V offset/speed, bitmap, V scale, U extent, world-Y offset,
// enabled byte. Defaults are zero/disabled until LoadTextureLayers supplies them.
// +4C/+50/+54 = auxiliary owned vector; +58 flags and +5C vtable belong to
// the embedded resource handle; +60/+64/+68 are its data and buffer words.
// The generated-texture vtable's name alone does not establish every buffer role.
struct fn_801E3390_object {
    void* field00; char pad_04[4]; void* field08; void* field0C; void* field10;
    float field14; float field18; int field1C; float field20; float field24; float field28;
    unsigned char field2C; float field30; float field34; int field38; float field3C; float field40; float field44;
    unsigned char field48; char pad_49[3]; void* field4C; void* field50; void* field54;
    unsigned char field58; char pad_59[3]; void* field5C; void* field60; void* field64; void* field68;
};
// Construction projection of the same storage; the legacy ABI view above is retained.
struct RiverLayerConstructionView {
    float vOffset, vScrollSpeed;
    int bitmapHandleBits;
    float vScale, uExtent, worldHeightOffset;
    unsigned char enabled; char pad[3];
};
struct RiverConstructionView {
    void* vtable; char untouchedHalfWidth[4];
    void* nodes; void* nodeCountBits; void* nodeCapacityBits;
    RiverLayerConstructionView layers[2];
    void* auxiliaryData; void* auxiliarySizeBits; void* auxiliaryCapacityBits;
    unsigned char materialFlags; char pad59[3];
    void* materialVtable; void* materialData;
    void* vertexBuffer; void* secondVertexBuffer;
};
typedef char RiverLayerConstructionSize[(sizeof(RiverLayerConstructionView) == 0x1C) ? 1 : -1];
typedef char RiverConstructionBufferOffset[((int)&((RiverConstructionView*)0)->vertexBuffer == 0x64) ? 1 : -1];
extern char __vt__6CRiver[];
extern char __vt__10CGeneTex3D[];
extern const float kFloat_0_River;
#pragma scheduling off
#pragma opt_propagation off
#pragma global_optimizer off
// Initialize river geometry, texture/resource handles, and embedded generated-texture state.
void CRiver_Ctor(fn_801E3390_object* self) {
    void* vtGene;
    void* vtRiver;
    int zero = 0;
    float f0 = kFloat_0_River;
    vtRiver = (void*)__vt__6CRiver;
    vtGene = (void*)__vt__10CGeneTex3D;
    // Phase 1: bind the river type and empty all owned buffers/material storage.
    ((RiverConstructionView*)self)->vtable = vtRiver;
    ((RiverConstructionView*)self)->nodes = (void*)zero;
    ((RiverConstructionView*)self)->nodeCountBits = (void*)zero;
    ((RiverConstructionView*)self)->nodeCapacityBits = (void*)zero;
    ((RiverConstructionView*)self)->auxiliaryData = (void*)zero;
    ((RiverConstructionView*)self)->auxiliarySizeBits = (void*)zero;
    ((RiverConstructionView*)self)->auxiliaryCapacityBits = (void*)zero;
    ((RiverConstructionView*)self)->materialFlags = (unsigned char)zero;
    ((RiverConstructionView*)self)->materialVtable = vtGene;
    ((RiverConstructionView*)self)->materialData = (void*)zero;
    // Phase 2: leave both texture layers disabled with zero tuning until loading.
    ((RiverConstructionView*)self)->layers[0].vOffset = f0;
    ((RiverConstructionView*)self)->layers[0].vScrollSpeed = f0;
    ((RiverConstructionView*)self)->layers[0].bitmapHandleBits = zero;
    ((RiverConstructionView*)self)->layers[0].vScale = f0;
    ((RiverConstructionView*)self)->layers[0].uExtent = f0;
    ((RiverConstructionView*)self)->layers[0].worldHeightOffset = f0;
    ((RiverConstructionView*)self)->layers[0].enabled = 0;
    ((RiverConstructionView*)self)->vertexBuffer = 0;
    ((RiverConstructionView*)self)->layers[1].vOffset = f0;
    ((RiverConstructionView*)self)->layers[1].vScrollSpeed = f0;
    ((RiverConstructionView*)self)->layers[1].bitmapHandleBits = 0;
    ((RiverConstructionView*)self)->layers[1].vScale = f0;
    ((RiverConstructionView*)self)->layers[1].uExtent = f0;
    ((RiverConstructionView*)self)->layers[1].worldHeightOffset = f0;
    ((RiverConstructionView*)self)->layers[1].enabled = 0;
    ((RiverConstructionView*)self)->secondVertexBuffer = 0;
}
#pragma opt_propagation reset
#pragma global_optimizer reset
#pragma scheduling on
extern const char comShotScheduledDebugLabel[4];
extern const char comShotPlannedPositionLabel[4];
extern const char comShotPlannedLabel[4];
extern const char comShotExpectedPositionLabel[4];
extern const char blankRivalStatusString[4];
extern const char ainStatusString[4];
extern const char shotLogNoObMarker[4];
extern const char outOfBoundsAbbreviation[4];
extern const char comShotPowerLengthAngleFormat[];
void ComShot_Log(const char*, ...);
extern char comShotDebugStrings[];
struct PowerCorrectionLogView { char pad_00[0x18]; struct { float aimAngle, remainingDistance, power, curve, positionX, positionY, positionZ; int rejected; char pad_20[0x0C]; int finalWithinTarget; }* candidate; };
// Log the selected power-corrected candidate and its landing status.
void CComShotPowerCorrection_LogCandidate(PowerCorrectionLogView* self) {
    ComShot_Log(((ComShotStringPool*)comShotDebugStrings)->vectorFormat, comShotPlannedPositionLabel,
                self->candidate->positionX, self->candidate->positionY, self->candidate->positionZ);
    ComShot_Log(comShotPowerLengthAngleFormat, comShotScheduledDebugLabel, self->candidate->power,
                self->candidate->remainingDistance, self->candidate->aimAngle,
                self->candidate->rejected != 0 ? outOfBoundsAbbreviation : shotLogNoObMarker,
                self->candidate->finalWithinTarget != 0 ? ainStatusString : blankRivalStatusString);
}

struct AngleCorrectionCounts { int firstConfigWord; int angleSampleCount; };
struct AngleCorrectionIterationView { char pad_00[0x0C]; AngleCorrectionCounts* config; int rival; int sampleIndex; };

// Advance the angle sample index and report whether more samples remain.
unsigned CComShotAngleCorrection_NextAngleSample(AngleCorrectionIterationView* self) {
    int value = ++self->sampleIndex;
    return value < self->config->angleSampleCount;
}

struct PuttCorrectionIterationView { char pad_00[0x0C]; unsigned correctionAxis; unsigned reservedCounter; unsigned directionIndex; };
// Alternate putt correction axes and stop after four directions.
unsigned CComPadOnePoint_AdvanceCorrectionDirection(PuttCorrectionIterationView* self) {
    if (self->correctionAxis == 0) {
        self->correctionAxis = 1;
    } else {
        self->correctionAxis = 0;
        ++self->directionIndex;
    }
    unsigned value = self->directionIndex ^ kCorrectionDirectionCount;
    return (4u << __cntlzw(value)) >> 31;
}

struct CorrectionEnableView {
    char pad_00[0x60];
    int angleCorrectionEnabled;
    int powerCorrectionEnabled;
};

// Enable or disable candidate validity checks during angle correction.
void CComShotAngleCorrection_vfunc_5(CorrectionEnableView* self, int value) {
    self->angleCorrectionEnabled = value;
}

// Enable or disable candidate validity checks during power correction.
void CComShotPowerCorrection_vfunc_5(CorrectionEnableView* self, int value) {
    self->powerCorrectionEnabled = value;
}

extern const float kFloat_6_2831855;
extern float s_twoPi_River;

// Initialize the river helper's full-turn angle constant.
void River_InitializeAngleConstant() {
    s_twoPi_River = kFloat_6_2831855;
}

struct ShotCandidateStatus { char pad_00[4]; float remainingDistance; char pad_08[0x14]; int rejected; };
struct ShotCorrectionIterationView {
    char pad_00[0x0C]; ShotCandidateStatus* candidate; unsigned directionIndex; int correctionAxis;
};
// Alternate correction axes, stopping on a usable candidate or after four directions.
int CComShotOnePoint_NextCorrectionDirection(ShotCorrectionIterationView* self) {
    int result = 0;
    switch (self->correctionAxis) {
    case 0:
        self->correctionAxis = 1;
        break;
    case 1:
        self->correctionAxis = 0;
        if (self->candidate->rejected != 0 && self->directionIndex != 0)
            result = 1;
        ++self->directionIndex;
        break;
    }
    if (result != 0)
        return 0;
    if (self->candidate->rejected == 0 && self->candidate->remainingDistance < kCpuShotHorizontalStopToleranceWorld)
        return 0;
    if (self->directionIndex >= kCorrectionDirectionCount)
        return 0;
    return 1;
}

struct AimPowerCurve { float x, y, z, w; };
void __as__7WVectorFRC7WVector(void*, const void*);
struct ShotCorrectionInitializationView { char pad_00[0x0C]; void* candidate; unsigned directionIndex; unsigned correctionAxis; void* rival; };
extern void* GolfBall__Fv();
extern void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern float Vector_SignedAngleXZ(const void*, const void*);
extern void CRival_SelectClubAndComputeShot(void*, void*, float);
extern const char kComShotTargetHeadingFormat[];
extern const char kComShotBallPositionLabel[];
extern float kFloat_0_ComShot;
extern float kFloat_1_Rival;
// Compute the target heading, select a club, and initialize a full-shot candidate.
void CComShotOnePoint_SelectClubForTargetHeading(ShotCorrectionInitializationView* self) {
    char* pos = (char*)&((RivalAimView*)self->rival)->target;
    void* a = GolfBall__Fv();
    void* b = GolfBall__Fv();
    float up[3];

    WVector vec = *(WVector*)pos - *(WVector*)b;
    up[0] = kFloat_0_ComShot;
    up[1] = kFloat_0_ComShot;
    up[2] = kFloat_1_Rival;
    float angle = Vector_SignedAngleXZ(&vec, up);
    ComShot_Log(((ComShotStringPool*)comShotDebugStrings)->vectorFormat, kComShotBallPositionLabel, ((float*)a)[0], ((float*)a)[1], ((float*)a)[2]);
    ComShot_Log(kComShotTargetHeadingFormat, angle);
    CRival_SelectClubAndComputeShot(self->rival, self->candidate, angle);
    self->directionIndex = 0;
    self->correctionAxis = 0;
}

struct PowerCorrectionSourceView { float aimAngle, remainingDistance, power, curve; char positionStorage[0x0C]; unsigned rejected; unsigned char groundType; char pad_21[3]; unsigned nearHazard, firstLandingWithinTarget, finalWithinTarget, hasSpecialLanding, hasInvalidLanding, holed, simulationFrames; };
struct PowerCorrectionSnapshotView { char pad_00[0x0C]; void* config; unsigned rival; unsigned sampleIndex; PowerCorrectionSourceView* bestCandidate; float sampleAngle, sampleDistance, samplePower, sampleCurve; unsigned samplePositionX; unsigned char samplePositionY; char pad_31[3]; unsigned samplePositionZ; unsigned sampleRejected; unsigned char sampleGroundType; char pad_3D[3]; unsigned sampleNearHazard, sampleFirstLandingWithinTarget, sampleFinalWithinTarget, sampleHasSpecialLanding, sampleHasInvalidLanding, sampleHoled, sampleFrames; float minimumPower; float maximumPower; };
extern float kFloat_2_ComShot;
// Copy the current candidate and establish the power sampling interval.
void CComShotPowerCorrection_InitSampleState(PowerCorrectionSnapshotView* self) {
    PowerCorrectionSourceView* src = self->bestCandidate;
    self->sampleAngle = src->aimAngle; self->sampleDistance = src->remainingDistance;
    self->samplePower = src->power; self->sampleCurve = src->curve;
    *(WVector*)(&self->samplePositionX) = *(WVector*)(&src->positionStorage);
    self->sampleRejected = src->rejected; self->sampleGroundType = src->groundType;
    self->sampleNearHazard = src->nearHazard; self->sampleFirstLandingWithinTarget = src->firstLandingWithinTarget;
    self->sampleFinalWithinTarget = src->finalWithinTarget; self->sampleHasSpecialLanding = src->hasSpecialLanding;
    self->sampleHasInvalidLanding = src->hasInvalidLanding; self->sampleHoled = src->holed;
    self->sampleFrames = src->simulationFrames;
    float difference = self->bestCandidate->power - *(float*)((char*)self->config + 8);
    self->minimumPower = difference;
    float limit = self->maximumPower - 2.0f * *(float*)((char*)self->config + 8);
    difference = (difference > limit) ? limit : difference;
    self->minimumPower = difference;
    self->sampleIndex = 0;
}

struct PuttRandomizerView { char pad_00[0x48]; void* randomUse; };
struct PuttResultApplyView { char pad_00[0x10]; AimPowerCurve* candidate; char pad_14[4]; PuttRandomizerView* rival; };
struct PuttSharedDocPrefix { char pad_00[0xB90]; void* golfDoc; };
struct PuttGolfDocIndices { char pad_00[0x3B6]; unsigned char holeIndex; char pad_3B7[0xA]; unsigned char playerIndex; };
extern float CComRandomUse_AdjustValue(void*, int, int, float);
extern void Matrix_SetAxisRotation(void*, float, int);
extern void __ml__FfRC7WVector(void*, const void*, float);
extern void Matrix_TransformPoint(void*, const void*, const void*);
extern void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern void __as__7WVectorFRC7WVector(void*, const void*);
extern void CRival_PrepareShotGauge(void*, float, float);
extern const float kFloat_100_0_ComShot;
#pragma opt_common_subs off
// Apply character-dependent putt error and update the rival aim and cached shot gauge.
void CComPadOnePoint_ApplyPuttResultToRival(PuttResultApplyView* self) {
    PuttSharedDocPrefix* global = (PuttSharedDocPrefix*)g_sharedDoc;

    PuttGolfDocIndices* game = (PuttGolfDocIndices*)(void*)AsCSharedDoc(global)->golfDoc;
    // Keep hole-slot lookup before player-stride addition for the original schedule.
    enum { characterOffset = (int)&((SharedCpuCharacterRecord*)0)->characters };
    int index = (signed char)*(unsigned char*)((char*)global + ((SharedHoleCharacterView*)global)->holeSlots[game->holeIndex] + game->playerIndex * sizeof(SharedCpuCharacterRecord) + characterOffset);
    self->candidate->z = CComRandomUse_AdjustValue(AsCRival(self->rival)->randomUse, index, kPuttPowerRandomIndex, self->candidate->z);
    self->candidate->x = CComRandomUse_AdjustValue(AsCRival(self->rival)->randomUse, index, kPuttAngleRandomIndex, self->candidate->x);
    float out[12];
    __typeof__(self->candidate->x) savedX = self->candidate->x; void* dst = AsCRival(self->rival)->holePosition;
    float a[3] = {kFloat_0_ComShot, kFloat_0_ComShot, kFloat_1_Rival};
    float b[3];
    float c[3];
    Matrix_SetAxisRotation(out, savedX, 1);
    __ml__FfRC7WVector(b, a, kFloat_100_0_ComShot);
    float d[3];
    Matrix_TransformPoint(c, b, out);
    __pl__FRC7WVectorRC7WVector(d, GolfBall__Fv(), c);
    __as__7WVectorFRC7WVector(dst, d);
    CRival_PrepareShotGauge(self->rival, self->candidate->z, kFloat_0_ComShot);

    float* end = (float*)self->rival; AimPowerCurve* value = self->candidate;
    end[0x38/4] = value->w;
    end[0x34/4] = value->z;
}
#pragma opt_common_subs reset

struct AngleCorrectionLogCandidate { float aimAngle; float remainingDistance; float power; float curve; float positionX; float positionY; float positionZ; int rejected; char pad_20[0x0C]; int finalWithinTarget; };
struct AngleCorrectionLogView { char pad_00[0x18]; AngleCorrectionLogCandidate* candidate; };
// Log the angle-corrected candidate and its landing status.
void CComShotAngleCorrection_LogCandidate(AngleCorrectionLogView* self) {
    ComShot_Log(((ComShotStringPool*)comShotDebugStrings)->vectorFormat, comShotExpectedPositionLabel,
                self->candidate->positionX, self->candidate->positionY, self->candidate->positionZ);
    ComShot_Log(comShotPowerLengthAngleFormat, comShotPlannedLabel,
                self->candidate->power, self->candidate->remainingDistance,
                self->candidate->aimAngle,
                self->candidate->rejected != 0 ? outOfBoundsAbbreviation : shotLogNoObMarker,
                self->candidate->finalWithinTarget != 0 ? ainStatusString : blankRivalStatusString);
}

struct AngleCorrectionSourceView { float aimAngle; float remainingDistance; float power; float curve; float positionX; float positionY; float positionZ; int rejected; unsigned char groundType; char pad_21[3]; unsigned nearHazard; unsigned firstLandingWithinTarget; unsigned finalWithinTarget; unsigned hasSpecialLanding; unsigned hasInvalidLanding; unsigned holed; unsigned simulationFrames; };
struct AngleCorrectionSnapshotView { char pad_00[0x0C]; void* config; int rival; int sampleIndex; AngleCorrectionSourceView* bestCandidate; float sampleAngle; float sampleDistance; float samplePower; float sampleCurve; float samplePositionX; float samplePositionY; float samplePositionZ; int sampleRejected; unsigned char sampleGroundType; char pad_3D[3]; unsigned sampleNearHazard; unsigned sampleFirstLandingWithinTarget; unsigned sampleFinalWithinTarget; unsigned sampleHasSpecialLanding; unsigned sampleHasInvalidLanding; unsigned sampleHoled; unsigned sampleFrames; float minimumAngle; };
void __as__7WVectorFRC7WVector(void*, const void*);
// Copy the current candidate and establish the angle sampling interval.
void CComShotAngleCorrection_InitSampleFromCandidate(AngleCorrectionSnapshotView* self) {
    AngleCorrectionSourceView* source = self->bestCandidate;
    self->sampleAngle = source->aimAngle;
    self->sampleDistance = source->remainingDistance;
    self->samplePower = source->power;
    self->sampleCurve = source->curve;
    *(WVector*)(&self->samplePositionX) = *(WVector*)(&source->positionX);
    self->sampleRejected = source->rejected;
    self->sampleGroundType = source->groundType;
    self->sampleNearHazard = source->nearHazard;
    self->sampleFirstLandingWithinTarget = source->firstLandingWithinTarget;
    self->sampleFinalWithinTarget = source->finalWithinTarget;
    self->sampleHasSpecialLanding = source->hasSpecialLanding;
    self->sampleHasInvalidLanding = source->hasInvalidLanding;
    self->sampleHoled = source->holed;
    self->sampleFrames = source->simulationFrames;
    self->minimumAngle = *(float*)self->bestCandidate - *(float*)self->config;
    self->sampleIndex = 0;
}

struct CorrectionTaskView {
    virtual void unused00(); virtual void unused04(); virtual void unused08();
    virtual void unused0C(); virtual void unused10(); virtual void activate(int);
    CorrectionTaskView* next; int enabled; void* config; char pad_10[8]; int bestCandidate; char pad_1C[0x44]; float maximumPower;
};
struct ShotCorrectionDispatchView {
    char pad_00[0x04]; CorrectionTaskView* active; int chainState; int candidateAddress; unsigned directionIndex; int correctionAxis;
    char pad_18[4]; CorrectionTaskView* angleCorrection; CorrectionTaskView* powerCorrection;
};
extern const char kComShotCorrectionRangesByDirection[];
extern float kFloat_1_Rival;
void ComThinkChain_MarkAllNodes(CorrectionTaskView*);
// Configure and activate the angle or power correction child for this direction.
void CComShotOnePoint_ConfigureAndActivateCorrection(ShotCorrectionDispatchView* self) {
    void* config = (void*)(kComShotCorrectionRangesByDirection + self->directionIndex * 16);
    switch (self->correctionAxis) {
    case 0: {
        int value = self->candidateAddress;
        CorrectionTaskView* item = self->angleCorrection;
        item->config = config;
        item->bestCandidate = value;
        self->angleCorrection->activate(self->directionIndex != 0);
        item = self->angleCorrection;
        item->enabled = 1;
        CorrectionTaskView* child = item->next;
        for (int i = 0; i < 4; ++i) {
            if (!child) break;
            child->enabled = 1;
            child = child->next;
        }
        if (child) ComThinkChain_MarkAllNodes(child);
        self->active = self->angleCorrection;
        break;
    }
    case 1: {
        int value = self->candidateAddress;
        CorrectionTaskView* item = self->powerCorrection;
        item->config = config;
        item->bestCandidate = value;
        item->maximumPower = kFloat_1_Rival;
        self->powerCorrection->activate(self->directionIndex != 0);
        item = self->powerCorrection;
        item->enabled = 1;
        CorrectionTaskView* child = item->next;
        for (int i = 0; i < 4; ++i) {
            if (!child) break;
            child->enabled = 1;
            child = child->next;
        }
        if (child) ComThinkChain_MarkAllNodes(child);
        self->active = self->powerCorrection;
        break;
    }
    }
}

struct PuttCorrectionDispatchView {
    char pad_00[4]; void* active; int chainState; int correctionAxis; int candidateAddress; unsigned directionIndex;
    char pad_18[4]; CorrectionTaskView* angleCorrection; CorrectionTaskView* powerCorrection;
};
extern const char kComPuttCorrectionRangesByDirection[];
extern const float kFloat_1_5_ComShot;
#pragma opt_lifetimes off
// Configure the current putt correction child and activate its task chain.
void CComPadOnePoint_ActivateCorrectionTask(PuttCorrectionDispatchView* self) {
    void* config = (void*)(kComPuttCorrectionRangesByDirection + self->directionIndex * 16);
    switch (self->correctionAxis) {
    case 0: {
        int _v = self->candidateAddress;
        {
            CorrectionTaskView* it = self->angleCorrection;
            it->config = config;
            it->bestCandidate = _v;
        }
        self->angleCorrection->activate(1);
        self->angleCorrection->enabled = 1;
        CorrectionTaskView* child = self->angleCorrection->next;
        for (int i = 0; i < 4; ++i) {
            if (!child) break;
            child->enabled = 1;
            child = child->next;
        }
        if (child) ComThinkChain_MarkAllNodes(child);
        self->active = self->angleCorrection;
        break;
    }
    case 1: {
        int _v = self->candidateAddress;
        {
            CorrectionTaskView* it = self->powerCorrection;
            it->config = config;
            it->bestCandidate = _v;
            it->maximumPower = kFloat_1_5_ComShot;
        }
        self->powerCorrection->activate(1);
        self->powerCorrection->enabled = 1;
        CorrectionTaskView* child = self->powerCorrection->next;
        for (int i = 0; i < 4; ++i) {
            if (!child) break;
            child->enabled = 1;
            child = child->next;
        }
        if (child) ComThinkChain_MarkAllNodes(child);
        self->active = self->powerCorrection;
        break;
    }
    }
}
#pragma opt_lifetimes reset

struct RiverRef { CW_BITS2( unsigned count : 16, unsigned flags : 16) };
struct RiverVector {
    void* data; int count; int capacity;
    // Reduce the river array count without freeing storage or destroying elements.
    void remove(int amount) { count -= amount; }
};
struct fn_801E3414_object { void* vtable; char pad_04[4]; RiverVector rivers; char pad_14[8]; void* field1C; char pad_20[0x2C]; RiverVector points; int field58; void* field5C; void* field60; RiverRef* field64; };
extern void* g_resourceManager;
extern char __vt__6CRiver[];
extern char __vt__10CGeneTex3D[];
void ResourceManager_ReleaseId(void*, void*);
void CubeResourceStorage_dtor(void*, int);
void __dla__FPv(void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
#pragma opt_lifetimes off
#pragma opt_propagation off
// Release river resources, generated texture data, and geometry arrays before optional deletion.
fn_801E3414_object* CRiver_dtor(fn_801E3414_object* self, int deleting) {
    if (self) {
        self->vtable = (void*)__vt__6CRiver;
        fn_801E3414_object* part = self;
        fn_801E3414_object* ref = self;
        for (int i = 0; i < 2; ++i) {
            if (g_resourceManager && part->field1C) {
                ResourceManager_ReleaseId(g_resourceManager, part->field1C);
                part->field1C = 0;
            }
            RiverRef* resource = ref->field64;
            if (resource) {
                if (--resource->count == 0) CubeResourceStorage_dtor(resource, 1);
                ref->field64 = 0;
            }
            enum { resourceViewStride = 0x1C };
            part = (fn_801E3414_object*)((char*)part + resourceViewStride);
            ref = (fn_801E3414_object*)((char*)ref + 4);
        }
        if (&self->field58) {
            self->field5C = (void*)__vt__10CGeneTex3D;
            if (self->field60) { __dla__FPv(self->field60); self->field60 = 0; }
        }
        if (&self->points) { if (&self->points) { if (&self->points) { if (&self->points) {
            if (self->points.data) {
                self->points.remove(self->points.count);
                appFreeUni__FPv(self->points.data);
            }
        } } } }
        if (&self->rivers) { if (&self->rivers) { if (&self->rivers) { if (&self->rivers) {
            if (self->rivers.data) {
                self->rivers.remove(self->rivers.count);
                appFreeUni__FPv(self->rivers.data);
            }
        } } } }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

#pragma opt_lifetimes reset
#pragma opt_propagation reset
struct AngleCorrectionRange { float angleHalfRange; int sampleCount; };
struct AngleCorrectionSampleView {
    char pad_00[0x0C]; AngleCorrectionRange* config; void* rival; int sampleIndex;
    void* bestCandidate; float sampleAngle, sampleDistance, samplePower; char pad_28[0x10];
    int sampleRejected; char pad_3C[0x0C]; int sampleFinalWithinTarget; char pad_4C[0x10]; float minimumAngle; int enabled;
};
extern const double kSignedIntToDoubleBias_ComShot;
extern float kFloat_2_ComShot;
extern const char comShotAngleCorrectionLogTag[8];
extern void* ComShot_SimulateTrajectory(void*, const void*, const void*);
extern void* ComShot_SelectPreferredCorrectionCandidate(void*, void*, void*, int, int, int);
extern void __as__7WVectorFRC7WVector(void*, const void*);
// config.sampleCount >= 2; sampleAngle and following fields form the candidate scratch payload.
// Simulate one angle sample and retain the preferred candidate.
void CComShotAngleCorrection_EvaluateAngleSample(AngleCorrectionSampleView* self) {

    const AngleCorrectionRange* range = self->config;
    const float t = (float)self->sampleIndex / (float)(range->sampleCount - 1);
    self->sampleAngle = ((2.0f) * range->angleHalfRange) * t + self->minimumAngle;
    void* context = self->rival;
    if (ComShot_SimulateTrajectory(context, &self->sampleAngle, &((RivalAimView*)context)->target) == 0 || self->enabled == 0) {
        const char* status = blankRivalStatusString;
        if (self->sampleFinalWithinTarget) status = ainStatusString;

        const char* fmt = comShotPowerLengthAngleFormat;
        float _f1 = self->samplePower;
        float _f2 = self->sampleDistance;
        float _f3 = self->sampleAngle;
        const char* result = (self->sampleRejected) ? outOfBoundsAbbreviation : shotLogNoObMarker;
        ComShot_Log(fmt, comShotAngleCorrectionLogTag, _f1, _f2, _f3, result, status);
        void* output = ComShot_SelectPreferredCorrectionCandidate(self->rival, (float*)self->bestCandidate, &self->sampleAngle, 0, 0, self->enabled);
        float* src = (float*)output;
        float* dst = (float*)self->bestCandidate;
        dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
        *(WVector*)(dst + 4) = *(WVector*)(4 + src);
        unsigned* dw = (unsigned*)dst; unsigned* sw = (unsigned*)src;
        dw[7] = sw[7]; ((unsigned char*)dst)[0x20] = ((unsigned char*)src)[0x20];
        dw[9] = sw[9]; dw[10] = sw[10]; dw[11] = sw[11]; dw[12] = sw[12]; dw[13] = sw[13]; dw[14] = sw[14]; dw[15] = sw[15];
    }
}

struct PowerCorrectionRange { char pad_00[8]; float powerHalfRange; int sampleCount; };
struct PowerCorrectionSampleView {
    char pad_00[0x0C]; PowerCorrectionRange* config; void* rival; int sampleIndex;
    void* bestCandidate; float sampleAngle, sampleDistance, samplePower; char pad_28[0x10];
    int sampleRejected; char pad_3C[0x0C]; int sampleFinalWithinTarget; char pad_4C[0x10]; float minimumPower, maximumPower; int enabled;
};
extern const float kFloat_0_01_CourseObject;
extern const float kFloat_0_8_Rival;
extern const char comShotPowerSelectionLogName[8];
extern void* GolfBall__Fv();
#pragma opt_propagation off
// config.sampleCount >= 2; sampling mutates the live simulation scratch before choosing the winner.
// Clamp and simulate one power sample and retain the preferred candidate.
void CComShotPowerCorrection_SimulatePowerSample(PowerCorrectionSampleView* self) {
    const PowerCorrectionRange* range = self->config;
    float t = (float)self->sampleIndex / (float)(range->sampleCount - 1);
    float maximumPower = self->maximumPower;
    float value = (2.0f * range->powerHalfRange) * t + self->minimumPower;
    self->samplePower = value;
    float clamped = value < kFloat_0_01_CourseObject ? kFloat_0_01_CourseObject : (value > maximumPower ? maximumPower : value);
    self->samplePower = clamped;
    if (self->samplePower < kFloat_0_8_Rival && AsGolfBall(GolfBall__Fv())->m_powerFlags != 0) return;
    if (ComShot_SimulateTrajectory(self->rival, &self->sampleAngle, (char*)&((RivalAimView*)self->rival)->target) != 0 && self->enabled != 0) return;
    const char* status = self->sampleFinalWithinTarget ? ainStatusString : blankRivalStatusString;
    const char* fmt = comShotPowerLengthAngleFormat;
    float f24 = self->samplePower;
    int field38 = self->sampleRejected;
    float f20 = self->sampleDistance;
    float f1c = self->sampleAngle;
    const char* name = comShotPowerSelectionLogName;
    const char* result = field38 ? outOfBoundsAbbreviation : shotLogNoObMarker;
    ComShot_Log(fmt, name, f24, f20, f1c, result, status);
    void* output = ComShot_SelectPreferredCorrectionCandidate(self->rival, &self->sampleAngle, self->bestCandidate, 0, 0, self->enabled);
    float* src = (float*)output;
    float* dst = (float*)self->bestCandidate;
    dst[0]=src[0]; dst[1]=src[1]; dst[2]=src[2]; dst[3]=src[3]; *(WVector*)(dst+4) = *(WVector*)(src+4);
    unsigned* dw=(unsigned*)dst; unsigned* sw=(unsigned*)src;
    dw[7]=sw[7]; ((unsigned char*)dst)[0x20] = ((unsigned char*)src)[0x20];
    dw[9]=sw[9]; dw[10]=sw[10]; dw[11]=sw[11]; dw[12]=sw[12]; dw[13]=sw[13]; dw[14]=sw[14]; dw[15]=sw[15];
}
#pragma opt_propagation reset

struct ShotCandidateView { float aimAngle, remainingDistance, power, curve; char positionStorage[0x0C]; unsigned rejected; unsigned char groundType; char pad_21[3]; unsigned nearHazard, firstLandingWithinTarget, finalWithinTarget, hasSpecialLanding, hasInvalidLanding, holed, simulationFrames; };
struct ShotCheckpointView { char pad_00[0x1C]; int targetIndex; char pad_20[0x10]; void* targetData; char pad_34[0x14]; void* randomUse; };
struct ShotRivalTargetView { char pad_00[0x50]; void* thinkController; void* targetArea; };
struct ShotResultApplyView { char pad_00[0x0C]; ShotCandidateView* candidate; char pad_10[8]; ShotRivalTargetView* rival; };
extern int CComThinkCtrl_ValidateCommandResults(void*);
extern void CComThinkCtrl_SetSpecialLandingResult(void*, void*); // Store the simulated special-landing result.
extern void CComThinkCtrl_SetFirstLandingWithinTargetResult(void*, void*); // Store whether the first landing is inside the target.
extern void CComThinkCtrl_SetFinalWithinTargetResult(void*, void*); // Store whether the final position is inside the target.
extern void* GolfBall__Fv();
extern int ComShot_IsSlopeAndClearanceAcceptable(void*, float);
extern void ComShot_Log(const char*, ...);
extern void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern float WVectorLen__FRC7WVector(const void*);
extern void __as__7WVectorFRC7WVector(void*, const void*);
extern void ComShot_EvaluateCandidate(void*, ShotCandidateView*);
extern float kFloat_0_ComShot;
extern const float kFloat_80_ComShot;
extern const char kComShotSlopeClearanceRejectedMessage[];
extern const char kComShotDistantCandidateRejectedMessage[];
extern const char kComShotHazardCandidateRejectedMessage[];
// Validate landing constraints, update the fallback candidate, or accept the simulated shot.
void CComShotOnePoint_EvaluateShotResult(ShotResultApplyView* self, void* result) {
    // Phase 1: reject disallowed landing results and unsuitable launch terrain.
    ShotCheckpointView* config = (ShotCheckpointView*)self->rival->thinkController;
    if (self->candidate->rejected == 0) {
        CComThinkCtrl_SetSpecialLandingResult(config, *(void**)&self->candidate->hasSpecialLanding);
        CComThinkCtrl_SetFirstLandingWithinTargetResult(config, *(void**)&self->candidate->firstLandingWithinTarget);
        CComThinkCtrl_SetFinalWithinTargetResult(config, *(void**)&self->candidate->finalWithinTarget);
        if (CComThinkCtrl_ValidateCommandResults(config) == 0) {
            self->candidate->rejected = 1;
            return;
        }
        void* current = GolfBall__Fv();
        if (ComShot_IsSlopeAndClearanceAcceptable(current, self->candidate->aimAngle) == 0) {
            ComShot_Log(kComShotSlopeClearanceRejectedMessage);
            self->candidate->rejected = 1;
            return;
        }
        // Phase 2: compare horizontal distance to the target area, retaining fallbacks.
        unsigned m[3];
        unsigned v[3];
        __mi__FRC7WVectorRC7WVector(m, &((ShotTargetArea*)self->rival->targetArea)->position, self->candidate->positionStorage);
        v[0] = m[0]; v[1] = m[1]; v[2] = m[2]; *(float*)&v[1] = kFloat_0_ComShot;
        float value = WVectorLen(*(WVector*)v);
        ShotCandidateView* state;
        if (value >= kFloat_80_ComShot + ((ShotTargetArea*)self->rival->targetArea)->radius) {
            if (((ShotProcessorBestView*)result)->bestCandidate.rejected != 0 || self->candidate->remainingDistance < ((ShotProcessorBestView*)result)->bestCandidate.remainingDistance) {
                state = self->candidate;
                ((ShotProcessorBestView*)result)->bestCandidate.aimAngle = state->aimAngle;
                ((ShotProcessorBestView*)result)->bestCandidate.remainingDistance = state->remainingDistance;
                ((ShotProcessorBestView*)result)->bestCandidate.power = state->power;
                ((ShotProcessorBestView*)result)->bestCandidate.curve = state->curve;
                *(WVector*)&((ShotProcessorBestView*)result)->bestCandidate.position = *(WVector*)state->positionStorage;
                ((ShotProcessorBestView*)result)->bestCandidate.rejected = state->rejected;
                ((ShotProcessorBestView*)result)->bestCandidate.groundType = state->groundType;
                for (int i = 0; i < 7; ++i) ((unsigned*)&((ShotProcessorBestView*)result)->bestCandidate.nearHazard)[i] = ((&state->nearHazard)[i]);
            }
            ComShot_Log(kComShotDistantCandidateRejectedMessage);
            self->candidate->rejected = 1;
        } else if (state = self->candidate, state->nearHazard != 0) {
            if (((ShotProcessorBestView*)result)->bestCandidate.rejected != 0) {
                ((ShotProcessorBestView*)result)->bestCandidate.aimAngle = state->aimAngle;
                ((ShotProcessorBestView*)result)->bestCandidate.remainingDistance = state->remainingDistance;
                ((ShotProcessorBestView*)result)->bestCandidate.power = state->power;
                ((ShotProcessorBestView*)result)->bestCandidate.curve = state->curve;
                *(WVector*)&((ShotProcessorBestView*)result)->bestCandidate.position = *(WVector*)state->positionStorage;
                ((ShotProcessorBestView*)result)->bestCandidate.rejected = state->rejected;
                ((ShotProcessorBestView*)result)->bestCandidate.groundType = state->groundType;
                for (int i = 0; i < 7; ++i) ((unsigned*)&((ShotProcessorBestView*)result)->bestCandidate.nearHazard)[i] = ((&state->nearHazard)[i]);
            }
            ComShot_Log(kComShotHazardCandidateRejectedMessage);
            self->candidate->rejected = 1;
        } else ComShot_EvaluateCandidate(self->rival, state);
    }
}

extern const char kComPuttBallPositionLabel[];
extern const char kComPuttTargetPositionLabel[];
extern const char kComPuttHolePositionLabel[];
extern const char kComPuttTargetDistanceFormat[];
extern const char kComPuttPowerFormat[];
extern const char kComPuttClubIndexFormat[];
extern const float kFloat_3_2_ComShot;
extern float kFloat_0_ComShot;
extern float kFloat_1_Rival;
extern const float kFloat_5_ComShot;
extern const float kFloat_1_1_ComShot;
extern void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern float Magnitude__7WVectorCFv(void*);
extern void __ml__FfRC7WVector(void*, const void*, float);
extern float Vector_SignedAngleXZ(const void*, const void*);
extern float WVectorLen__FRC7WVector(const void*);
extern void __as__7WVectorFRC7WVector(void*, const void*);
extern void* InstanceTable_Get();
extern void ClubSelection_SelectForCpuTarget(void*, void*);
extern float GolfParameter_ResolveFloat(void*, int);
extern void ComPutt_SumSlopeAlongTargetPath(void*, void*, void*);
extern void* GolfBall__Fv();
extern void ComShot_Log(const char*, ...);
extern CSharedDoc* g_sharedDoc;
extern char comShotDebugStrings[];
struct CPOHole { int a, b, c; };
struct CPOVec { float v[3]; };
struct CPOEntry { char pad[0xC]; CPOHole pos; char pad2[0x8]; };
// Estimate putt heading and power from target distance and sampled terrain slope.
void CComPadOnePoint_InitializePuttCandidate(PuttInitializationView* self) {
    float vector0[3];
    float vector[3];
    float tmp[3];
    CPOHole hole1;
    float result[3];
    CPOHole hole2;
    CPOVec flat;
    float up[3];
    CPOVec holePos;

    // Phase 1: measure the world target distance and select the putter parameters.
    float* target = &self->rival->targetX;
    float* pos = (float*)GolfBall__Fv();
    ComShot_Log(comShotDebugStrings + (int)&((ComShotStringPool*)0)->vectorFormat, kComPuttBallPositionLabel, pos[0], pos[1], pos[2]);
    ComShot_Log(comShotDebugStrings + (int)&((ComShotStringPool*)0)->vectorFormat, kComPuttTargetPositionLabel, target[0], target[1], target[2]);
    CSharedDoc* sg = g_sharedDoc;
    void* b90 = (void*)sg->golfDoc;
    unsigned char idx = AsCGolfDoc(b90)->currentHoleIndex;
    void* holeBase = AsCGolfDoc(b90)->holeRecords;
    void* entry = (char*)holeBase + (idx << 5);
    hole1 = *(CPOHole*)&((CourseHoleView*)entry)->position;
    ComShot_Log(comShotDebugStrings + (int)&((ComShotStringPool*)0)->vectorFormat, kComPuttHolePositionLabel, *(float*)&hole1.a, *(float*)&hole1.b, *(float*)&hole1.c);
    __mi__FRC7WVectorRC7WVector(vector0, GolfBall__Fv(), target);
    float dist = Magnitude__7WVectorCFv(vector0) / kFloat_3_2_ComShot;
    ComShot_Log(kComPuttTargetDistanceFormat, dist);
    ClubSelection_SelectForCpuTarget(InstanceTable_Get(), target);
    float five = kFloat_5_ComShot;
    CSharedDoc* sg2 = g_sharedDoc;
    void* b902 = (void*)sg2->golfDoc;
    unsigned char idx2 = AsCGolfDoc(b902)->currentHoleIndex;
    CPOEntry* holeBase2 = (CPOEntry*)AsCGolfDoc(b902)->holeRecords;
    int h0;
    CPOHole* src2 = &holeBase2[idx2].pos;
    h0 = src2->a;
    hole2.b = src2->b;
    hole2.a = h0;
    hole2.c = src2->c;
    // Phase 2: offset the aim by the summed terrain normals toward the hole.
    ComPutt_SumSlopeAlongTargetPath(result, self->rival, &hole2);
    __ml__FfRC7WVector(vector, result, five);
    __mi__FRC7WVectorRC7WVector(tmp, target, vector);
    __mi__FRC7WVectorRC7WVector(holePos.v, tmp, GolfBall__Fv());
    up[0] = kFloat_0_ComShot; up[1] = kFloat_0_ComShot; up[2] = kFloat_1_Rival;
    float angle = Vector_SignedAngleXZ(holePos.v, up);
    flat = holePos;
    flat.v[1] = kFloat_0_ComShot;
    float power = WVectorLen__FRC7WVector(flat.v);
    power /= kFloat_3_2_ComShot * GolfParameter_ResolveFloat(InstanceTable_Get(), 0);
    power *= kFloat_1_1_ComShot;
    ComShot_Log(kComPuttPowerFormat, power);
    void* e = InstanceTable_Get();
    unsigned char club = *(unsigned char*)(*(char**)((char*)e + 4) + 9);
    ComShot_Log(kComPuttClubIndexFormat, club);
    float cp = (power > kFloat_1_Rival) ? kFloat_1_Rival : power;
    // Phase 3: publish the initial candidate. Position/distance bytes are not
    // initialized here; later simulation must fill them before they are consumed.
    PuttCandidateView s;
    s.aimAngle = angle;
    s.power = cp;
    s.curve = 0.0f;
    s.rejected = 0;
    s.groundType = 0;
    s.nearHazard = 0; s.firstLandingWithinTarget = 0; s.finalWithinTarget = 0; s.hasSpecialLanding = 0; s.hasInvalidLanding = 0; s.holed = 0; s.simulationFrames = 0;
    PuttCandidateView* f10 = self->candidate;
    f10->aimAngle = s.aimAngle; f10->remainingDistance = s.remainingDistance; f10->power = s.power; f10->curve = s.curve;
    __as__7WVectorFRC7WVector(f10->positionStorage, s.positionStorage);
    f10->rejected = s.rejected;
    f10->groundType = s.groundType;
    f10->nearHazard = s.nearHazard; f10->firstLandingWithinTarget = s.firstLandingWithinTarget; f10->finalWithinTarget = s.finalWithinTarget; f10->hasSpecialLanding = s.hasSpecialLanding;
    f10->hasInvalidLanding = s.hasInvalidLanding; f10->holed = s.holed; f10->simulationFrames = s.simulationFrames;
    self->candidate->rejected = 1;
    self->correctionAxis = 0;
    self->directionIndex = 0;
    e = InstanceTable_Get();
    *(unsigned*)&AsCRival(self->rival)->selectedClubIndex = *(unsigned char*)(*(char**)((char*)e + 4) + 9);
}
} // extern "C"
} // namespace ns4

extern "C" { void River_InitializeAngleConstant(); }
/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621A4", _ctors_u801DD078_0, River_InitializeAngleConstant);
#pragma pop
