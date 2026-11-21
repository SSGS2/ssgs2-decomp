#include <decomp/local_class.h>
#include <decomp/offset_of.h>
// Course creatures: flock placement, authored flight paths, grid roaming and
// spline NPCs. Creation/metadata parsing is shared with npc_creatures_course_objects.cpp/creature_puppet_loading.cpp;
// animation event consumers and waypoint builders also live in creature_path_movement_and_draw.cpp.
// Modding index (world units, seconds for animation/movement dt, radians for turns):
// - Mole/dolphin/crab/gull/butterfly: their *_Ctor below sets speed/cellSize;
//   *_vfunc_31 movement consumes those fields. Do not edit CCreature's layout.
// - Sparrow/duck flocks: CSparrow_Ctor/CDuck_Ctor(count), their instance matrix
//   builders and Draw methods; count controls allocations, not just draw count.
// - Santa/fighter flight: CSanta_Ctor/CFighter_Ctor set initial speeds 120/320,
//   turnRate 2*pi/3 rad/s, bankRate pi/2 rad/s; *_vfunc_31 recalculates leg speeds.
//   Santa SetFlightLegSpeeds' 120 is the authored-leg traversal rate; fighter's
//   resolved XZ distance becomes speed, so changing initial 320 affects first leg.
// - UFO/penguin flight: CUfo_Ctor/ntPenguin_Ctor and *_vfunc_31; kFloat_120_Creature
//   sets target-distance scaling; heading/bank thresholds share the scalar pool.
// - Sailor/penguin ducks: ntPenguin_Sailor_Ctor / ntPenguin_Duck_Ctor and follower
//   methods; movementEnabled differs from animationActive and flock instance count.
// - Black sparrow: ntNpc_BlackSparrow_Ctor speed 5.5, cellSize 48; vfunc_31 consumes
//   grid counts/steps/history, vfunc_27 switches rest/move at animation wraps
//   (rand()%101 <20 rests, else moves). Grid data is initialized upstream, below.
// - Dragon/ship: ntNpc_Dragon_Ctor / ntNpc_PolaShip_Ctor(value) store spawnDelay;
//   spline updates compare animation time to that delay, then advance pathDistance.
// Shared values are defined in u8019B4B0_data.cpp, also consumed by npc_pola_ship_and_penguin.cpp,
// npc_pola_penguin.cpp/npc_creatures_and_spline.cpp; isolate a new species setting in a future mod instead of
// changing a shared 0.1/2/5/48 pool value globally. Safe tuning ranges unestablished.
#include "classes/CCreature.h"
// Species -> suffix view (only the listed storage is interchangeable):
// CDuck: DuckInstanceStorage; CSparrow: SparrowInstanceStorage.
// ntPenguin_Duck: PenguinDuckStorage (counts/list precede matrices/scales).
// CSanta/CFighter: Obj8019 flight paths; CUfo/ntPenguin: Obj_8019E368
//   with FlightTargetCursorView and FlightWaypointStorage during construction.
// ntPenguin_Sailor: SailorMovementView plus CreatureGridPathView for grid history.
// ntNpc_BlackSparrow: CreatureGridPathView for its grid/path suffix.
// ntNpc_Dragon: SplineNpcStorage; ntNpc_PolaShip: ShipStorage (different offsets).
// CreatureEffectMetadataView describes the attachment ARGUMENT, not the creature.
// Constructors and the species update methods below establish this map; a common
// CCreature prefix alone does not authorize a cast to another species suffix.
// Only common creature members belong to CCreature. Derived storage below overlaps
// between flocks, path-following NPCs and spline NPCs and must use separate views.
// CDuck and CSparrow use different meanings for the words following the base.
// Flock suffixes below use logical counts/pointers, not float bit
// containers. They are independent layouts: never use a flock view on a flight
// or spline NPC merely because their suffixes start at the same address.
// Grid/path suffix shared by these movement methods. Views borrow the creature's
// storage; recentCells owns count integers elsewhere, pointsAddressBits owns XYZ
// records elsewhere. Keep address bits unsigned for the original byte arithmetic.
struct CreatureGridPathView {
    char prefix[0xA4];
    int recentCellCount;
    int* recentCells;
    char reservedAC[0x24];
    unsigned int pointsAddressBits, pointCount, pointCapacity;
};
typedef char CreatureGridPath_count_offset[((int)DECOMP_OFFSETOF(CreatureGridPathView, pointCount) == 0xD4) ? 1 : -1];
// Field-derived offsets retain byte-address staging where inline accessors alter code generation.
enum GridPathAccessOffsets { kRecentCellsOffset = (int)DECOMP_OFFSETOF(CreatureGridPathView, recentCells),
    kRecentCellCountOffset = (int)DECOMP_OFFSETOF(CreatureGridPathView, recentCellCount),
    kWaypointCapacityOffset = (int)DECOMP_OFFSETOF(CreatureGridPathView, pointCapacity) };
enum GridMovementPolicy { kGridAxisCount = 3, kNeighborStepChoices = 3,
    kNeighborAttemptLimit = 5, kHistoryShiftBatchWords = 8,
    kWaypointBytes = 12, kMaxWaypointCount = 0x15555555,
    kWaypointThreeFifthsGrowthLimit = 0x071C71C7, kWaypointHalfGrowthLimit = 0x0E38E38E };

struct DuckInstanceStorage {
    char common[0x11C]; int instanceCount; void* matrices;
    int puppetCount; void* listPrevious; void* listNext;
};
#define AsDuckInstances(p) ((DuckInstanceStorage*)(p))
struct SparrowInstanceStorage {
    char common[0x11C]; int maxInstanceCount, instanceCount; void* matrices;
};
#define AsSparrowInstances(p) ((SparrowInstanceStorage*)(p))
struct FlightWaypointStorage { char common[0x11C]; float verticalSpeed; int bankPhase; };
struct FlightTargetCursorView { char common[0xCC]; unsigned char targetCursor; };
struct CreatureEffectMetadataView { char common[0x18]; char attachmentPayload[1]; };
struct SailorMovementView { char common[0x120]; unsigned char movementEnabled; };
struct SharedCharControlView { void* vtable; void* strongCountBits; void* weakCountBits; void* data; };
enum { kCreatureSecondaryVtableOffset = 12 };

struct PenguinDuckStorage {
    char common[0x11C]; int instanceCount; int puppetCount;
    void* listNext; void* listPrevious;
    void* instanceMatrices; void* instanceScales;
};
#define AsPenguinDuck(p) ((PenguinDuckStorage*)(p))
struct SplineNpcStorage {
    char common[0x120]; int delayActiveBits;
    float spawnDelay, pathDistance;
    void* splinePoints; void* splineDistances;
    unsigned int splineCoefficients; unsigned int splineLengthBits; unsigned int splinePointCount;
};
#define AsSplineNpc(p) ((SplineNpcStorage*)(p))
struct ShipStorage {
    char common[0x120]; int delayActiveBits; float spawnDelay, pathDistance;
    void* byteBuffer; void* byteCountBits; unsigned int byteCapacity;
    unsigned char bufferControlByte; char padding139[3];
    unsigned int splinePoints, splineDistances, splineCoefficients;
    float splineLength; unsigned int splinePointCount;
};
#define AsShip(p) ((ShipStorage*)(p))
struct CreatureAnimationRange {
    float startTime, endTime; char padding008[0x10]; const char* name;
};
struct CreatureAffineWords { unsigned int right[3], up[3], forward[3], position[3]; };
struct CreaturePlayerGateView { char padding000[0xAE]; unsigned char creatureSpawnEligibility; };
enum { kCreatureOneShotAnimation = 1, kCreatureOpaque = 255,
       kBankReturnToLevel = 0, kBankIntoTurn = 1, kBankLevel = 2,
       kYawAxis = 1, kPitchAxis = 0, kRollAxis = 2 };
// Preserve address-staging lifetimes in the follower loops using field-derived offsets.
enum { kCreatureTransformOffset = (int)DECOMP_OFFSETOF(CCreature, transform[0]),
       kCreatureTargetOffset = (int)DECOMP_OFFSETOF(CCreature, targetPosition[0]),
       kCreatureSpawnOffset = (int)DECOMP_OFFSETOF(CCreature, movementBoundsOrigin),
       kCreatureBoundsMaxOffset = (int)DECOMP_OFFSETOF(CCreature, movementBoundsMax),
       kCreaturePositionOffset = (int)DECOMP_OFFSETOF(CCreature, transform[9]),
       kCreatureUpOffset = (int)DECOMP_OFFSETOF(CCreature, transform[3]),
       kCreatureForwardOffset = (int)DECOMP_OFFSETOF(CCreature, transform[6]) };
// Byte layout of the existing string pool; encoded lengths include the NUL.
struct CreatureStrings {
    char vectorLengthError[20]; // vector length error
    char lengthErrorType[55]; // !std::exception!!std::logic_error!!std::length_error!!
    char padding04B[0xD3];
    char defaultAnimation[12]; // NPC_Default
    char padding12A[0x8];
    char effectTag[4]; // *fx
    char effectTagFormat[9]; // %s %s %s
    char spriteExtension[4]; // spr
    char sequenceExtension[4]; // seq
    char alternateDefaultAnimation[13]; // NPC_Default1
    char santaSoundAlias[6]; // Santa
    char santaBoostSound[11]; // santaboost
    char padding165[0x14];
    char action0[9]; // NPC_Act0
    char action1[9]; // NPC_Act1
    char dragonFireEffect[15]; // devil_fire.seq
    char action02[10]; // NPC_Act02
    char action01[10]; // NPC_Act01
};
enum {
    kCreatureStrings_vectorLengthError = (int)DECOMP_OFFSETOF(CreatureStrings, vectorLengthError),
    kCreatureStrings_lengthErrorType = (int)DECOMP_OFFSETOF(CreatureStrings, lengthErrorType),
    kCreatureStrings_defaultAnimation = (int)DECOMP_OFFSETOF(CreatureStrings, defaultAnimation),
    kCreatureStrings_effectTag = (int)DECOMP_OFFSETOF(CreatureStrings, effectTag),
    kCreatureStrings_effectTagFormat = (int)DECOMP_OFFSETOF(CreatureStrings, effectTagFormat),
    kCreatureStrings_spriteExtension = (int)DECOMP_OFFSETOF(CreatureStrings, spriteExtension),
    kCreatureStrings_sequenceExtension = (int)DECOMP_OFFSETOF(CreatureStrings, sequenceExtension),
    kCreatureStrings_alternateDefaultAnimation = (int)DECOMP_OFFSETOF(CreatureStrings, alternateDefaultAnimation),
    kCreatureStrings_santaSoundAlias = (int)DECOMP_OFFSETOF(CreatureStrings, santaSoundAlias),
    kCreatureStrings_santaBoostSound = (int)DECOMP_OFFSETOF(CreatureStrings, santaBoostSound),
    kCreatureStrings_action0 = (int)DECOMP_OFFSETOF(CreatureStrings, action0),
    kCreatureStrings_action1 = (int)DECOMP_OFFSETOF(CreatureStrings, action1),
    kCreatureStrings_dragonFireEffect = (int)DECOMP_OFFSETOF(CreatureStrings, dragonFireEffect),
    kCreatureStrings_action02 = (int)DECOMP_OFFSETOF(CreatureStrings, action02),
    kCreatureStrings_action01 = (int)DECOMP_OFFSETOF(CreatureStrings, action01),
};

#include "game/golfball.h"
#include "classes/CGolfDoc.h"
#include "classes/CSharedDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "sdk_protos.h"
#include "game/wvector.h"
#include "game/types.h"
#include "classes/CCreature.h"
#include "classes/WPuppet.h"
#define AsWPuppet(p) ((WPuppet*)(p))
#include "classes/CDuck.h"
#include "classes/CSparrow.h"

struct NativeMember80201D18 {
    void* vt;
    // Leave the native-member placeholder unchanged during construction.
    NativeMember80201D18() {}
    ~NativeMember80201D18();
};

#define AsCSparrow(p) ((CSparrow*)(p))

#define AsCDuck(p) ((CDuck*)(p))

#define AsCCreature(p) ((CCreature*)(p))

// Return caller storage for placement construction.
inline void* operator new(size_t, void* p) { return p; }
extern "C" {
extern void* __nwa__FUl(unsigned int);
}
// Allocate the requested array bytes with the game allocator.
inline void* operator new[](size_t size) { return __nwa__FUl(size); }

// ---- merged from course_creatures_flocks_and_flyers.cpp ----
namespace ns0 {
// Translation unit 0x8019B4B0-0x8019CBFC (21 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
extern char s_cameraTransform[];
extern void* CCreature_dtor(void*, int);
extern void __dl__FPv(void*);
extern void Vec3_Subtract(void*, const void*, const void*);
extern void Vec3_Normalize(void*, const void*);
extern float Vec3_Dot(const void*, const void*);

extern const float kFloat_0_Creature;
extern const float kFloat_neg1_CreatureDirection;
extern const float kFloat_neg3_1415927_Creature;
extern const float kFloat_3_1415927_805DF2E4;
extern const float kFloat_6_4_Creature;
extern const float kFloat_10_CreatureMotion;
extern const float kFloat_48_Creature;
extern const float kFloat_7_Creature;
extern const float kFloat_16_Creature;
extern const float kFloat_0_7853982_Creature;

extern void CCreature_InitRandomGridCell(void*, const void*, unsigned int);
extern void* g_sharedDoc;
extern void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
extern void Vec3_Zero(void*);
extern void Matrix_CopyAffine(void*, const void*);
extern void __ml__FfRC7WVector(void*, const void*, float);
extern void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern void __as__7WVectorFRC7WVector(void*, const void*);
extern void Vector_CrossProduct(void*, const void*, const void*);
extern void Normalize__7WVectorFv(void*);
extern void __ml__FRC7WVectorf(void*, const void*, float);
extern void Matrix_SetAxisRotation(void*, float, int);
extern float Random_RangeFloat(int, float, float);
extern void* GolfBall__Fv();
extern void* PreData__9CGolfBallFi(void*, int);
extern void* CQuadTree_GetInstance();
extern void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, const void*, int, int, int);
// GetGroundPoint has the PowerPC-shaped signature (return slot, self, pointer to the point, ...); the by-value
// calls go through a helper.
#define GROUND_BV_CREATURE ((WVector (*)(void*, WVector, int, int, int))GetGroundPoint__9CQuadTreeF7WVectorbPib)
extern void* CCreature_LoadPuppetResource(void*, void*, int);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void ResourceManager_ReleaseObject(void*, void*, int);
extern void* ResourceManager_LoadByExtension(void*, void*, int, int, int, void*);
extern void* __nwa__FUl(unsigned int);
extern void Model_SetMatrixRecursive(void*, int, const void*);
extern void WPuppet_UpdateBoneWorldMatrices(void*, float, int);
extern void WPuppet_UpdateBoundsFromBone(void*);
extern void Matrix_CopyScaledBasis(void*, const void*, float);
extern void* fn_8004C2BC_ret(const void*);
extern void CCreature_Draw(void*);
extern void* fn_8019A674_ret(void*);
extern void WMatrix_DefaultConstructNoOp(void*);
extern unsigned char GetGroundType__9CQuadTreeF7WVector(void*, void*);
extern void __dla__FPv(void*);
extern void* g_resourceManager;
extern void* g_view;
extern char __vt__5CDuck[];
extern char __vt__8CSparrow[];
extern const float kFloat_1_Creature;
extern const float kFloat_0_8_805DF2D8;
extern const float kFloat_0_5_Creature;
extern float Magnitude__7WVectorCFv(const void*);
extern void* fn_8004C2BC_ret(const void*);
extern const double kDouble_SignedIntConversionBias_Creature;
extern const float kFloat_0_017453292_Creature;
extern const float kFloat_neg180_Creature;
extern const float kFloat_180_Creature;
extern const float kFloat_0_5_Creature;
extern int lbl_8019C508_UNUSED;

// Integer lanes preserve bitwise vector copies; math calls interpret the same bytes as floats.
struct Vec3 {
    int x;
    int y;
    int z;
};

struct Elem {
    char pad00[0x18];
    Vec3 forward;
    Vec3 position;
};

struct UnkObj {
    char pad00[0x28];
    char basisXY[0x18];
    char forward[0xc];
    char position[0xc];
    float animationFrame;
    char pad5c[0x14];
    float turnRate;
    float movementSpeed;
    char pad78[0x34];
    unsigned char opacity;
    unsigned char animationActive;
    char pad_ae[0x2];
    float ballSpawnOffsetDistance;
    char pad_b4[0x68];
    int instanceCount;
    Elem* instanceMatrices;
};

struct ObjLists {
    char pad00[0x11c];
    int instanceCount;
    int instanceMatrices;
    Elem* f124;
    void* f128;
    void* f12c;
};

struct ListNode { ListNode* prev; ListNode* next; void* value; };

struct CtorObj {
    void* vtable;
    void* secondaryVtable;
    int flags;
    char movementBoundsOrigin[0xc];
    char movementBoundsMax[0xc];
    int puppet;
    char pad28[0x30];
    float animationFrame;
    char pad5c[4];
    int animationRange;
    char pad64[0x10];
    float movementSpeed;
    char pad78[4];
    float maxBankAngle;
    char pad80[4];
    float heading;
    float cellSize;
    char pad8c[0x1c];
    int history;
    char padac[4];
    float ballSpawnOffsetDistance;
    int opacityKeyframeCount;
    void* listFirst;
    void* listLast;
    int targetPoints;
    int targetPointCount;
    int targetPointCapacity;
    char padcc[4];
    int pathPoints;
    int pathPointCount;
    int pathPointCapacity;
};

struct C6Obj {
    char pad00[0x28];
    char basisXY[0x18];
    char forward[0xc];
    char position[0xc];
    float animationFrame;
    char pad5c[0x50];
    unsigned char opacity;
    unsigned char animationActive;
    char pad_ae[2];
    float ballSpawnOffsetDistance;
};

extern char __vt__9CCreature[];
extern char __vt__5CGull[];
extern char __vt__10CButterfly[];
extern char __vt__5CMole[];
extern char __vt__8CDolphin[];
extern char __vt__5CCrab[];
extern const float kFloat_neg3_2_Crab;
extern const float kFloat_3_2_CourseGeometry;
extern const float kFloat_0_7_Creature;
extern const float kFloat_neg0_7_Creature;

// Initialize common creature storage, install the derived vtable, and invoke its resource hook.
static CtorObj* init_ctor(CtorObj* self, void* arg, char* vt) {
    self->vtable = __vt__9CCreature;
    self->secondaryVtable = __vt__9CCreature + 0xc;
    self->flags = 0;
    Vec3_Zero(self->movementBoundsOrigin);
    Vec3_Zero(self->movementBoundsMax);
    self->history = 0;
    self->animationFrame = kFloat_0_Creature;
    self->animationRange = 0;
    self->heading = kFloat_0_Creature;
    self->history = 0;
    self->ballSpawnOffsetDistance = kFloat_0_Creature;
    self->opacityKeyframeCount = 0;
    self->listLast = &self->listFirst;
    self->listFirst = &self->listFirst;
    self->targetPoints = 0;
    self->targetPointCount = 0;
    self->targetPointCapacity = 0;
    self->pathPoints = 0;
    self->pathPointCount = 0;
    self->pathPointCapacity = 0;
    self->vtable = vt;
    self->secondaryVtable = vt + 0xc;
    self->flags |= 1;
    ((void (*)(CtorObj*, void*))((void* const*)(vt))[0x98 / 4])(self, arg);
    return self;
}

// Initialize CMole state and owned resources.
CtorObj* CMole_Ctor(CtorObj* self, void* arg) {
    self->secondaryVtable = __vt__9CCreature + 0xc;
    self->vtable = __vt__9CCreature;
    self->flags = 0;
    Vec3_Zero(self->movementBoundsOrigin);
    Vec3_Zero(self->movementBoundsMax);
    self->animationFrame = kFloat_0_Creature;
    self->animationRange = 0;
    self->history = 0;
    self->heading = kFloat_0_Creature;
    self->ballSpawnOffsetDistance = kFloat_0_Creature;
    self->opacityKeyframeCount = 0;
    self->listFirst = &self->listFirst;
    self->listLast = &self->listFirst;
    self->targetPoints = 0;
    self->targetPointCount = 0;
    self->targetPointCapacity = 0;
    self->pathPoints = 0;
    self->puppet = 0;
    self->pathPointCount = 0;
    self->animationFrame = kFloat_0_Creature;
    self->pathPointCapacity = 0;
    self->animationRange = 0;
    self->heading = kFloat_0_Creature;
    self->history = 0;
    self->ballSpawnOffsetDistance = kFloat_0_Creature;
    void* lf = &self->listFirst;
    self->opacityKeyframeCount = 0;
    self->listLast = lf;
    self->listFirst = lf;
    self->targetPoints = 0;
    self->targetPointCount = 0;
    self->targetPointCapacity = 0;
    self->pathPoints = 0;
    self->pathPointCount = 0;
    self->pathPointCapacity = 0;
    self->vtable = __vt__5CMole;
    self->secondaryVtable = __vt__5CMole + 0xc;
    self->flags = self->flags | 1;
    try {
    ((void (*)(CtorObj*, void*))((void* const*)self->vtable)[0x98 / 4])(self, arg);
    self->ballSpawnOffsetDistance = kFloat_6_4_Creature;
    return self;
    } catch (...) {
        CCreature_dtor(self, -1);
        throw;
    }
}

// Initialize CDolphin state and owned resources.
CtorObj* CDolphin_Ctor(CtorObj* self, void* arg) {
    self->secondaryVtable = __vt__9CCreature + 0xc;
    self->vtable = __vt__9CCreature;
    self->flags = 0;
    Vec3_Zero(self->movementBoundsOrigin);
    Vec3_Zero(self->movementBoundsMax);
    self->puppet = 0;
    self->animationFrame = kFloat_0_Creature;
    self->animationRange = 0;
    self->heading = kFloat_0_Creature;
    self->history = 0;
    self->ballSpawnOffsetDistance = kFloat_0_Creature;
    self->opacityKeyframeCount = 0;
    self->listLast = &self->listFirst;
    self->listFirst = &self->listFirst;
    self->targetPoints = 0;
    self->targetPointCount = 0;
    self->targetPointCapacity = 0;
    self->pathPoints = 0;
    self->pathPointCount = 0;
    self->pathPointCapacity = 0;
    self->vtable = __vt__8CDolphin;
    self->secondaryVtable = __vt__8CDolphin + 0xc;
    self->flags = ((1 | self->flags));
    try {
    ((void (*)(CtorObj*, void*))((void* const*)(__vt__8CDolphin))[0x98 / 4])(self, arg);
    self->ballSpawnOffsetDistance = kFloat_0_Creature;
    return self;
    } catch (...) {
        CCreature_dtor(self, -1);
        throw;
    }
}

// Initialize CCrab state and owned resources.
CtorObj* CCrab_Ctor(CtorObj* self, void* arg) {
    self->secondaryVtable = __vt__9CCreature + 0xc;
    self->vtable = __vt__9CCreature;
    self->flags = 0;
    Vec3_Zero(self->movementBoundsOrigin);
    Vec3_Zero(self->movementBoundsMax);
    self->puppet = 0;
    self->animationFrame = kFloat_0_Creature;
    self->animationRange = 0;
    self->heading = kFloat_0_Creature;
    self->history = 0;
    self->ballSpawnOffsetDistance = kFloat_0_Creature;
    self->opacityKeyframeCount = 0;
    self->listLast = &self->listFirst;
    self->listFirst = &self->listFirst;
    self->targetPoints = 0;
    self->targetPointCount = 0;
    self->targetPointCapacity = 0;
    self->pathPoints = 0;
    self->pathPointCount = 0;
    self->pathPointCapacity = 0;
    self->vtable = __vt__5CCrab;
    self->secondaryVtable = __vt__5CCrab + 0xc;
    self->flags |= 1;
    try {
    ((void (*)(CtorObj*, void*))((void* const*)(__vt__5CCrab))[0x98 / 4])(self, arg);
    self->ballSpawnOffsetDistance = kFloat_neg3_2_Crab;
    return self;
    } catch (...) {
        CCreature_dtor(self, -1);
        throw;
    }
}

// Initialize the flock leader transform and place followers at fixed intervals behind it.
void CDuck_InitFlockFormation(UnkObj* self, const void* src, unsigned int flags) {
    CCreature_InitRandomGridCell(self, src, flags);
    if (self->instanceCount != 0) {
        Matrix_CopyAffine(self->instanceMatrices, self->basisXY);
        for (int i = 1; i < self->instanceCount; i++) {
            Vec3 w;
            Vec3 v;
            Matrix_CopyAffine(&self->instanceMatrices[i], &self->instanceMatrices[i - 1]);
            __ml__FfRC7WVector(&v, &self->instanceMatrices[i - 1].forward, kFloat_6_4_Creature);
            __pl__FRC7WVectorRC7WVector(&w, &self->instanceMatrices[i - 1].position, &v);
            __as__7WVectorFRC7WVector(&self->instanceMatrices[i].position, &w);
        }
    }
}

// Randomize heading and spawn the creature on the ground at its configured ball-relative offset.
void CCreature_SpawnAtBallOffset(UnkObj* self) {
    char buf[0x30];
    Vec3 out;
    Vec3 d;
    Vec3 c;
    Vec3 b;
    Vec3 a;

    self->animationActive = 1;
    self->animationFrame = kFloat_0_Creature;
    self->opacity = kCreatureOpaque;
    Matrix_SetAxisRotation(buf, Random_RangeFloat(-1, kFloat_neg3_1415927_Creature, kFloat_3_1415927_805DF2E4), 1);
    Matrix_CopyAffine(self->basisXY, buf);
    __ml__FRC7WVectorf(&a, self->forward, kFloat_neg1_CreatureDirection);
    __ml__FfRC7WVector(&b, &a, self->ballSpawnOffsetDistance);
    __pl__FRC7WVectorRC7WVector(&c, GolfBall__Fv(), &b);
    d = c;
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&out, CQuadTree_GetInstance(), &d, 0, 0, 1);
    __as__7WVectorFRC7WVector(self->position, &out);
}

// Release CGull resources and optionally delete the object.
void* CGull_Dtor(void* self, int doDelete) {
    if (self != 0) {
        CCreature_dtor(self, 0);
        if (doDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Release CButterfly resources and optionally delete the object.
void* CButterfly_Dtor(void* self, int doDelete) {
    if (self != 0) {
        CCreature_dtor(self, 0);
        if (doDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Initialize CGull state and owned resources.
CtorObj* CGull_Ctor(CtorObj* self, void* arg) {
    self->secondaryVtable = __vt__9CCreature + 0xc;
    self->vtable = __vt__9CCreature;
    self->flags = 0;
    Vec3_Zero(self->movementBoundsOrigin);
    Vec3_Zero(self->movementBoundsMax);
    self->puppet = 0;
    self->animationFrame = kFloat_0_Creature;
    self->animationRange = 0;
    self->heading = kFloat_0_Creature;
    self->history = 0;
    self->ballSpawnOffsetDistance = kFloat_0_Creature;
    self->opacityKeyframeCount = 0;
    self->listLast = &self->listFirst;
    self->listFirst = &self->listFirst;
    self->targetPoints = 0;
    self->targetPointCount = 0;
    self->targetPointCapacity = 0;
    self->pathPoints = 0;
    self->pathPointCount = 0;
    self->pathPointCapacity = 0;
    self->vtable = __vt__5CGull;
    void* savedVtable = self->vtable;
    self->secondaryVtable = __vt__5CGull + 0xc; self->movementSpeed = kFloat_10_CreatureMotion;
    try {
    ((void (*)(CtorObj*, void*))((void* const*)(savedVtable))[0x98 / 4])(self, arg);
    self->maxBankAngle = kFloat_0_7853982_Creature;
    self->cellSize = kFloat_48_Creature;
    return self;
    } catch (...) {
        CCreature_dtor(self, -1);
        throw;
    }
}

// Initialize CButterfly state and owned resources.
CtorObj* CButterfly_Ctor(CtorObj* self, void* arg) {
    self->secondaryVtable = __vt__9CCreature + 0xc;
    self->vtable = __vt__9CCreature;
    self->flags = 0;
    Vec3_Zero(self->movementBoundsOrigin);
    Vec3_Zero(self->movementBoundsMax);
    self->puppet = 0;
    self->animationFrame = kFloat_0_Creature;
    self->animationRange = 0;
    self->heading = kFloat_0_Creature;
    self->history = 0;
    self->ballSpawnOffsetDistance = kFloat_0_Creature;
    self->opacityKeyframeCount = 0;
    self->listLast = &self->listFirst;
    self->listFirst = &self->listFirst;
    self->targetPoints = 0;
    self->targetPointCount = 0;
    self->targetPointCapacity = 0;
    self->pathPoints = 0;
    self->pathPointCount = 0;
    self->pathPointCapacity = 0;
    self->vtable = __vt__10CButterfly;
    self->secondaryVtable = __vt__10CButterfly + 0xc;
    self->movementSpeed = kFloat_7_Creature;
    try {
    ((void (*)(CtorObj*, void*))((void* const*)(self->vtable))[0x98 / 4])(self, arg);
    self->maxBankAngle = kFloat_0_Creature;
    self->cellSize = kFloat_16_Creature;
    return self;
    } catch (...) {
        CCreature_dtor(self, -1);
        throw;
    }
}

// Spawn the crab near the ball when the current player eligibility byte is nonzero.
void CCrab_SpawnNearBall(C6Obj* self) {
    void* mgr = *(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc; // original +0xB90
    void* item = GolfPlayer_GetEntryByIndex(mgr, AsCGolfDoc(mgr)->currentPlayer);
    if (((CreaturePlayerGateView*)item)->creatureSpawnEligibility >= 1) {
        char buf[0x30];
        Vec3 t0;
        Vec3 t1;
        Vec3 t2;
        Vec3 t3;
        Vec3 out;
        self->animationActive = 1;
        self->animationFrame = kFloat_0_Creature;
        self->opacity = kCreatureOpaque;
        Matrix_SetAxisRotation(buf, Random_RangeFloat(-1, kFloat_neg3_1415927_Creature, kFloat_3_1415927_805DF2E4), 1);
        Matrix_CopyAffine(self->basisXY, buf);
        __ml__FRC7WVectorf(&t0, self->forward, kFloat_neg1_CreatureDirection);
        __ml__FfRC7WVector(&t1, &t0, self->ballSpawnOffsetDistance);
        __pl__FRC7WVectorRC7WVector(&t2, GolfBall__Fv(), &t1);
        t3 = t2;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&out, CQuadTree_GetInstance(), &t3, 0, 0, 1);
        __as__7WVectorFRC7WVector(self->position, &out);
    }
}

// Spawn the dolphin after the ball has landed in lie category 1.
void CDolphin_TrySpawnNearBall(UnkObj* self) {
    void* shared = GolfBall__Fv();
    if (((CGolfBall*)shared)->m_recoveryRequired && ((CGolfBall*)GolfBall__Fv())->m_lie == 1) {
        char buf[0x30];
        Vec3 a, b, c, d, out;
        self->animationActive = 1;
        self->animationFrame = kFloat_0_Creature;
        self->opacity = kCreatureOpaque;
        Matrix_SetAxisRotation(buf, Random_RangeFloat(-1, kFloat_neg3_1415927_Creature, kFloat_3_1415927_805DF2E4), 1);
        Matrix_CopyAffine(self->basisXY, buf);
        __ml__FRC7WVectorf(&a, self->forward, kFloat_neg1_CreatureDirection);
        __ml__FfRC7WVector(&b, &a, self->ballSpawnOffsetDistance);
        __pl__FRC7WVectorRC7WVector(&c, GolfBall__Fv(), &b);
        d = c;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&out, CQuadTree_GetInstance(), &d, 0, 0, 1);
        __as__7WVectorFRC7WVector(self->position, &out);
    }
}

struct LNode { LNode* prev; LNode* next; void* value; };
struct LGuard {
    char flag; LNode* allocation; LNode* copy; LNode* unused; LNode* head; LNode* n;
    // Initialize a guard with no node allocation to release.
    LGuard(): flag(0), n(0) {}
    // Free the node allocation if it has not been transferred.
    ~LGuard() { if (n) appFreeUni__FPv(n); }
};

// Load one puppet per duck instance into the owned list and select the first puppet.
void CDuck_LoadPuppetInstances(ObjLists* self, void* arg) {
    for (int i = 0; i < self->instanceCount; ++i) {
        LNode* head = (LNode*)&self->f128;
        void* value;
        if (i == 0) value = CCreature_LoadPuppetResource(self, arg, 1);
        else value = CCreature_LoadPuppetResource(self, arg, 0);
        LGuard guard;
        LNode* node = (LNode*)appMallocDummy__FUl(0xc);
        guard.allocation = node; guard.copy = node; guard.head = head; guard.n = node;
        if (&node->value) {
            try { node->value = value; } catch (...) { appFreeUni__FPv(node); throw; }
        }
        LNode* n = guard.n;
        guard.n = 0;
        n->prev = head->prev;
        head->prev->next = n;
        head->prev = n;
        n->next = head;
        ++AsDuckInstances(self)->puppetCount;
    }
    (*(void **)&AsCCreature(self)->puppet) = ((LNode*)self->f12c)->value;
}

// Scale active flock size by ball speed and randomize instance headings around the spawn center.
void CSparrow_InitFlockInstances(ObjLists* self) {
    char* p = (char*)self;
    p[0xad] = 1;
    float initialBallSpeed = Magnitude__7WVectorCFv((char*)PreData__9CGolfBallFi(GolfBall__Fv(), 0) + 0xc);
    float currentBallSpeed = Magnitude__7WVectorCFv(&((CGolfBall*)GolfBall__Fv())->m_velocity);
    float ballSpeedRatio = currentBallSpeed / initialBallSpeed;
    int transformOffset;
    int instanceIndex = 0;
    float radians = kFloat_0_017453292_Creature;
    transformOffset = 0;
    AsSparrowInstances(p)->instanceCount = (int)((float)AsSparrowInstances(p)->maxInstanceCount * ballSpeedRatio);
    while (instanceIndex < AsSparrowInstances(p)->instanceCount) {
        char headingMatrix[0x30];
        Matrix_SetAxisRotation(headingMatrix, radians * Random_RangeFloat(-1, kFloat_neg180_Creature, kFloat_180_Creature), 1);
        Matrix_CopyAffine((char*)AsSparrowInstances(p)->matrices + transformOffset, headingMatrix);
        Vec3 spawnCenter, boundsSum;
        __pl__FRC7WVectorRC7WVector(&boundsSum, (p + kCreatureBoundsMaxOffset), (p + kCreatureSpawnOffset));
        __ml__FfRC7WVector(&spawnCenter, &boundsSum, kFloat_0_5_Creature);
        __as__7WVectorFRC7WVector((char*)AsSparrowInstances(p)->matrices + transformOffset + 0x24, &spawnCenter);
        transformOffset += 0x30;
        ++instanceIndex;
    }
}

// Return whether a world position falls outside the configured camera-facing dot threshold.
int CCreature_IsOutsideCameraFacingThreshold(void* a, void* src, float threshold) {
    char buf[12]; // r1+0x14? actually 12 bytes
    float rot[3]; // r1+0x8
    Vec3_Subtract(buf, src, ((CreatureAffineWords*)s_cameraTransform)->position);
    Vec3_Normalize(rot, buf);
    float d = Vec3_Dot(rot, ((CreatureAffineWords*)s_cameraTransform)->forward);
    return d <= threshold;
}

struct SparrowElement {
    char data[0x30];
    // Preserve the matrix default-construction hook for one flock element.
    SparrowElement() { WMatrix_DefaultConstructNoOp(this); }
};

typedef ::NativeMember80201D18 NativeMember80201D18;
struct SparrowNative4 {
    void* vt;
    // Leave the native four-byte member unchanged during construction.
    SparrowNative4() {}
    // Preserve the empty native-member destruction hook.
    ~SparrowNative4() {}
};
struct SparrowCreatureBase : NativeMember80201D18, SparrowNative4 {
    char data[0xdc - 8];
    // Initialize the common creature base for sparrow construction.
    SparrowCreatureBase() {
    char* p = (char*)this;
    *(volatile int*)&AsCCreature(p)->flags = 0; // original +8
    Vec3_Zero(AsCCreature(p)->movementBoundsOrigin); // original +0xC
    Vec3_Zero(AsCCreature(p)->movementBoundsMax); // original +0x18
    AsCCreature(p)->puppet = 0; // original +0x24
    AsCCreature(p)->animationTime = kFloat_0_Creature; // original +0x58
    AsCCreature(p)->animationRange = 0; // original +0x60
    AsCCreature(p)->heading = kFloat_0_Creature; // original +0x84
    AsCCreature(p)->history = 0; // original +0xA8
    AsCCreature(p)->ballSpawnOffsetDistance = kFloat_0_Creature; // original +0xB0
    AsCCreature(p)->opacityKeyframeCount = 0; // original +0xB4
    AsCCreature(p)->opacityKeyframePrevious = ((char*)&AsCCreature(p)->opacityKeyframeNext); // original +0xBC
    AsCCreature(p)->opacityKeyframeNext = ((char*)&AsCCreature(p)->opacityKeyframeNext); // original +0xB8
    AsCCreature(p)->targetPoints = 0; // original +0xC0
    AsCCreature(p)->targetPointCount = 0; // original +0xC4
    AsCCreature(p)->targetPointCapacity = 0; // original +0xC8
    AsCCreature(p)->pathPoints = 0; // original +0xD0
    AsCCreature(p)->pathPointCount = 0; // original +0xD4
    AsCCreature(p)->pathPointCapacity = 0; // original +0xD8
    }
    // Destroy the common creature base without deleting its storage.
    ~SparrowCreatureBase() { CCreature_dtor(this, 0); }
};
struct SparrowCtor : SparrowCreatureBase {
    char tail[0x128 - 0xdc];
    // Construct the sparrow base and initialize the requested instance count.
    SparrowCtor(void* arg, int count) {
    char* p = (char*)this;
    AsCSparrow(p)->vtbl = __vt__8CSparrow;
    AsCSparrow(p)->secondaryVtable = __vt__8CSparrow + 0xc; // original +4
    AsCSparrow(p)->instanceMatrices = 0; // original +0x124
    void* resource = ResourceManager_LoadByExtension(g_resourceManager, arg, 0, 1, 0, ((char*)&AsCCreature(p)->opacityKeyframeNext));
    AsCSparrow(p)->model = resource; // original +0x24
    AsCCreature(p)->bankAngle = kFloat_0_Creature; // original +0x78
    p[0x119] = 0;
    AsCCreature(p)->opacity = kCreatureOpaque; // original +0xAC
    p[0xad] = 0;
    AsSparrowInstances(p)->maxInstanceCount = count; // original +0x11C
    void* array = new SparrowElement[count];
    AsCSparrow(p)->instanceMatrices = array; // original +0x124
    }
};
// Initialize CSparrow state and owned resources.
void* CSparrow_Ctor(void* self, void* arg, int count) {
    *(void* volatile*)self = __vt__9CCreature;
    *(void* volatile*)((char*)self + 4) = __vt__9CCreature + 0xc;
    new (self) SparrowCtor(arg, count);
    return self;
}

class VA0 { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24(); virtual void v28(); virtual void v2C(); virtual void v30(); virtual void v34(); virtual void v38(); virtual void v3C(); virtual void v40(); virtual void v44(); virtual void v48(); virtual void v4C(); virtual void v50(); virtual void v54(); virtual void v58(); virtual void v5C(); virtual void v60(); virtual void v64(); virtual void v68(); virtual void v6C(); virtual void v70(); virtual void v74(); virtual void v78(); virtual void v7C(); virtual void v80(); virtual void v84(); virtual void v88(); virtual void v8C(); virtual void v90(); virtual void v94(); virtual void v98(); virtual void v9C(); virtual int vA0(const void*,float);
};
class V18 { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18(void*,int,int,int,float,int);
};
// Query virtual slot A0; zero permits instance drawing, with world-space position and caller margin.
#define CreatureInstanceIsCulled(creature, worldPosition, margin) \
    (((VA0*)(creature))->vA0((worldPosition), (margin)))

// Draw a prepared puppet through virtual slot 18 with the instance path flags.
#define DrawCreaturePuppet(puppet, view, scale) \
    (((V18*)(puppet))->v18((view), 1, 1, 1, (scale), 0))

// Cull and draw each duck puppet using its corresponding follower transform.
void CDuck_DrawInstances(ObjLists* self) {
    ListNode* node;
    ListNode* sentinel;
    int offset;
    int index;
    float one;
    void* item;
    void** vt;
    if (self->instanceCount == 0) {
        CCreature_Draw(self);
        return;
    }
    node = (ListNode*)self->f12c;
    sentinel = (ListNode*)(((char*)&AsDuckInstances(self)->listPrevious));
    index = 0;
    offset = 0;
    one = kFloat_1_Creature;
    while (node != sentinel) {
        item = node->value;
        if (item != 0) {
            if (CreatureInstanceIsCulled(self, (char*)self->instanceMatrices + offset + 0x24, kFloat_0_8_805DF2D8) == 0) {
                if (*(float*)((char*)self->instanceMatrices + offset) > one) *(float*)((char*)self->instanceMatrices + offset) = one;
                if (*(float*)((char*)self->instanceMatrices + offset + 0x10) > one) *(float*)((char*)self->instanceMatrices + offset + 0x10) = one;
                if (*(float*)((char*)self->instanceMatrices + offset + 0x20) > one) *(float*)((char*)self->instanceMatrices + offset + 0x20) = one;
                char drawTransform[0x30];
                Matrix_CopyScaledBasis(drawTransform, (char*)self->instanceMatrices + offset, index == 0 ? kFloat_1_Creature : kFloat_0_5_Creature);
                Model_SetMatrixRecursive(item, 0, drawTransform);
                ((void(*)(void*, void*, int, float))WPuppet_UpdateBoneWorldMatrices)(item, g_view, 0, kFloat_1_Creature);
                WPuppet_UpdateBoundsFromBone(item);
                DrawCreaturePuppet(item, g_view, kFloat_1_Creature);
            }
        }
        node = node->next;
        offset += 0x30;
        ++index;
    }
}
#undef CreatureInstanceIsCulled
#undef DrawCreaturePuppet

#pragma push
#pragma use_lmw_stmw on
// Release puppet resources held by the duck list without deleting its nodes.
static inline void releaseBAList(char* p, void* zero) {
    ListNode* sentinel = (ListNode*)(p + 0x128);
    ListNode* n = *(ListNode**)(p + 0x12c);
    while (n != sentinel) {
        void* manager = g_resourceManager;
        if (manager != 0 && n->value != 0) {
            ResourceManager_ReleaseObject(manager, n->value, 0);
            n->value = zero;
        }
        n = n->next;
    }
}
// Release CDuck resources and optionally delete the object.
void* CDuck_Dtor(void* self, int doDelete) {
    if (self != 0) {
        char* p = (char*)self;
        *(void**)p = __vt__5CDuck;
        AsCDuck(p)->secondaryVtable = __vt__5CDuck + 0xc;
        void* array = AsCDuck(p)->instanceMatrices;
        if (array != 0) {
            if (array != 0) __dla__FPv((char*)array - 0x10);
            AsCDuck(p)->instanceMatrices = 0;
        }
        releaseBAList(p, 0);
        {
        ListNode* sentinel = (ListNode*)(p + 0x128);
        ListNode* first = (ListNode*)AsCDuck(p)->listNext;
        if (first != sentinel) {
            ListNode* tail = (ListNode*)sentinel->prev;
            ListNode* before = (ListNode*)first->prev;
            before->next = tail->next;
            tail->next->prev = first->prev;
            while (first != sentinel) {
                ListNode* old = first;
                first = first->next;
                appFreeUni__FPv(old);
                --AsCDuck(p)->linkCount;
            }
        }
        }
        AsCDuck(p)->puppet = 0;
        if (p + 0x124 != 0) {
        if (p + 0x124 != 0) {
        if (p + 0x124 != 0) {
        if (p + 0x124 != 0) {
        ListNode* sentinel2 = (ListNode*)(p + 0x128);
        ListNode* first2 = (ListNode*)AsCDuck(p)->listNext;
        if (first2 != sentinel2) {
            ListNode* tail2 = (ListNode*)sentinel2->prev;
            ListNode* before2 = (ListNode*)first2->prev;
            before2->next = tail2->next;
            tail2->next->prev = first2->prev;
            while (first2 != sentinel2) {
                ListNode* old = first2;
                first2 = first2->next;
                appFreeUni__FPv(old);
                --*(int*)(p + 0x124);
            }
        }
        } } } }
        CCreature_dtor(self, 0);
        if (doDelete > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop
extern void WPuppet_SetAnimationTimeFromRanges(void*, int, float, int, int);
extern char s_sceneLighting[];
extern const float kFloat_2_805DF2B0;
extern const float kFloat_0_5235988_Creature;
extern const float kFloat_32_Creature;

// Draw the shared sparrow puppet once for each active instance transform.
void CSparrow_Draw(void* self) {
    char* p = (char*)self;
    if (!AsCSparrow(p)->model || !AsCSparrow(p)->drawEnabled) return;
    for (int off=0, i=0; i < AsCSparrow(p)->instanceCount; off += 0x30, ++i) {
        WPuppet_SetAnimationTimeFromRanges(AsCSparrow(p)->model, 0, AsCSparrow(p)->animationFrame, 0, 0);
        if (((int(*)(void*, void*, float))(*(void***)self)[0xa0/4])(self, (*(char**)&AsCSparrow(p)->instanceMatrices) + off + 0x24, kFloat_0_8_805DF2D8)) continue;
        if (*(float*)((*(char**)&AsCSparrow(p)->instanceMatrices) + off) > kFloat_1_Creature) *(float*)((*(char**)&AsCSparrow(p)->instanceMatrices) + off) = kFloat_1_Creature;
        if (*(float*)((*(char**)&AsCSparrow(p)->instanceMatrices) + off + 0x10) > kFloat_1_Creature) *(float*)((*(char**)&AsCSparrow(p)->instanceMatrices) + off + 0x10) = kFloat_1_Creature;
        if (*(float*)((*(char**)&AsCSparrow(p)->instanceMatrices) + off + 0x20) > kFloat_1_Creature) *(float*)((*(char**)&AsCSparrow(p)->instanceMatrices) + off + 0x20) = kFloat_1_Creature;
        float* v = (float*)((*(char**)&AsCSparrow(p)->instanceMatrices) + off);
        Model_SetMatrixRecursive(AsCSparrow(p)->model, (int)AsCSparrow(p)->model, v);
        ((void(*)(void*, void*, int, float))WPuppet_UpdateBoneWorldMatrices)(AsCSparrow(p)->model, g_view, 0, kFloat_1_Creature);
        WPuppet_UpdateBoundsFromBone(AsCSparrow(p)->model);
        ((void(*)(void*, const char*, int, int))(*(void***)AsCSparrow(p)->model)[0x14/4])(AsCSparrow(p)->model, s_sceneLighting, 0, 0);
        ((void(*)(void*, void*, int, int, int, float, int))(*(void***)AsCSparrow(p)->model)[0x18/4])(AsCSparrow(p)->model, g_view, 0, 1, 1, kFloat_1_Creature, 0);
    }
}

// Initialize CDuck state and owned resources.
void* CDuck_Ctor(void* self, void* arg, int count) {
    // Slot padding only: no data getters are called through these declarations.
    // The only dispatch here is +0x98 initialize(arg), which loads derived creature
    // resources after base/list initialization. Other hook semantics are unresolved.
    struct VirtualObject {
        virtual void f08() DECOMP_LOCAL_VIRTUAL; virtual void f0C() DECOMP_LOCAL_VIRTUAL; virtual void f10() DECOMP_LOCAL_VIRTUAL; virtual void f14() DECOMP_LOCAL_VIRTUAL;
        virtual void f18() DECOMP_LOCAL_VIRTUAL; virtual void f1C() DECOMP_LOCAL_VIRTUAL; virtual void f20() DECOMP_LOCAL_VIRTUAL; virtual void unusedSlot24() DECOMP_LOCAL_VIRTUAL;
        virtual void unusedSlot28() DECOMP_LOCAL_VIRTUAL; virtual void f2C() DECOMP_LOCAL_VIRTUAL; virtual void f30() DECOMP_LOCAL_VIRTUAL; virtual void f34() DECOMP_LOCAL_VIRTUAL;
        virtual void f38() DECOMP_LOCAL_VIRTUAL; virtual void f3C() DECOMP_LOCAL_VIRTUAL; virtual void unusedSlot40() DECOMP_LOCAL_VIRTUAL; virtual void f44() DECOMP_LOCAL_VIRTUAL;
        virtual void f48() DECOMP_LOCAL_VIRTUAL; virtual void f4C() DECOMP_LOCAL_VIRTUAL; virtual void f50() DECOMP_LOCAL_VIRTUAL; virtual void f54() DECOMP_LOCAL_VIRTUAL;
        virtual void unusedSlot58() DECOMP_LOCAL_VIRTUAL; virtual void f5C() DECOMP_LOCAL_VIRTUAL; virtual void unusedSlot60() DECOMP_LOCAL_VIRTUAL; virtual void f64() DECOMP_LOCAL_VIRTUAL;
        virtual void f68() DECOMP_LOCAL_VIRTUAL; virtual void f6C() DECOMP_LOCAL_VIRTUAL; virtual void unusedSlot70() DECOMP_LOCAL_VIRTUAL; virtual void unusedSlot74() DECOMP_LOCAL_VIRTUAL;
        virtual void f78() DECOMP_LOCAL_VIRTUAL; virtual void f7C() DECOMP_LOCAL_VIRTUAL; virtual void f80() DECOMP_LOCAL_VIRTUAL; virtual void unusedSlot84() DECOMP_LOCAL_VIRTUAL;
        virtual void unusedSlot88() DECOMP_LOCAL_VIRTUAL; virtual void f8C() DECOMP_LOCAL_VIRTUAL; virtual void f90() DECOMP_LOCAL_VIRTUAL; virtual void f94() DECOMP_LOCAL_VIRTUAL;
        virtual void initialize(void*) DECOMP_LOCAL_VIRTUAL;
    };
    char* p = (char*)self;
    *(void* volatile*)p = __vt__9CCreature;
    *(void* volatile*)(p + 4) = __vt__9CCreature + 0xc;
    (*(volatile int*)&AsCCreature(p)->flags) = 0;
    Vec3_Zero(((char*)&AsCCreature(p)->movementBoundsOrigin[0]));
    Vec3_Zero(((char*)&AsCCreature(p)->movementBoundsMax[0]));
    AsCCreature(p)->puppet = 0;
    AsCCreature(p)->animationTime = kFloat_0_Creature;
    AsCCreature(p)->animationRange = 0;
    AsCCreature(p)->heading = kFloat_0_Creature;
    AsCCreature(p)->history = 0;
    AsCCreature(p)->ballSpawnOffsetDistance = kFloat_0_Creature;
    AsCCreature(p)->opacityKeyframeCount = 0;
    AsCCreature(p)->opacityKeyframePrevious = ((char*)&AsCCreature(p)->opacityKeyframeNext);
    AsCCreature(p)->opacityKeyframeNext = ((char*)&AsCCreature(p)->opacityKeyframeNext);
    AsCCreature(p)->targetPoints = 0;
    AsCCreature(p)->targetPointCount = 0;
    AsCCreature(p)->targetPointCapacity = 0;
    AsCCreature(p)->pathPoints = 0;
    AsCCreature(p)->pathPointCount = 0;
    AsCCreature(p)->pathPointCapacity = 0;
    *(void**)p = __vt__5CDuck;
    AsCCreature(p)->secondaryVtable = __vt__5CDuck + 0xc;
    AsDuckInstances(p)->matrices = 0;
    AsDuckInstances(p)->puppetCount = 0;
    AsDuckInstances(p)->listNext = p + 0x128;
    AsDuckInstances(p)->listPrevious = p + 0x128;
    AsCCreature(p)->movementSpeed = kFloat_2_805DF2B0;
    AsCCreature(p)->turnRate = kFloat_0_5235988_Creature;
    AsCCreature(p)->maxBankAngle = kFloat_0_Creature;
    AsCCreature(p)->cellSize = kFloat_32_Creature;
    AsDuckInstances(p)->instanceCount = count;
    if (count > 0) {
        void* a = new SparrowElement[count];
        AsDuckInstances(p)->matrices = a;
    }
    ListNode* sentinel = (ListNode*)(p + 0x128);
    ListNode* first = (ListNode*)AsDuckInstances(p)->listNext;
    if (first != sentinel) {
        ListNode* tail = (ListNode*)sentinel->prev;
        ListNode* before = (ListNode*)first->prev;
        before->next = tail->next;
        tail->next->prev = first->prev;
        while (first != sentinel) {
            ListNode* old = first;
            first = first->next;
            appFreeUni__FPv(old);
            --AsDuckInstances(p)->puppetCount;
        }
    }
    ((VirtualObject*)self)->initialize(arg);
    return self;
}

extern void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern float WVectorLen__FRC7WVector(const void*);
extern float Vector_SignedAngleXZ(void*, const void*);
extern void Matrix_ConcatAffine(void*, const void*, const void*);
extern void __apl__7WVectorFRC7WVector(void*, const void*);

class V90 { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24(); virtual void v28(); virtual void v2C(); virtual void v30(); virtual void v34(); virtual void v38(); virtual void v3C(); virtual void v40(); virtual void v44(); virtual void v48(); virtual void v4C(); virtual void v50(); virtual void v54(); virtual void v58(); virtual void v5C(); virtual void v60(); virtual void v64(); virtual void v68(); virtual void v6C(); virtual void v70(); virtual void v74(); virtual void v78(); virtual void v7C(); virtual void v80(); virtual void v84(); virtual void v88(); virtual void v8C(); virtual void v90();
};
// Move each duck toward its predecessor with a clamped turn rate and minimum separation.
void CDuck_UpdateFlockFormation(UnkObj* self, float deltaTime) {
    float maxTurn;
    Elem* follower;
    float turnAngle;
    float clampedTurn;
    float minTurn;
    ((V90*)self)->v90();
    if (self->instanceCount != 0) {
        Matrix_CopyAffine(self->instanceMatrices, self->basisXY);
        for (int transformOffset = 0x30, instanceIndex = 1; instanceIndex < self->instanceCount; transformOffset += 0x30, ++instanceIndex) {
            unsigned char rotatedTransform[0x30];
            unsigned char yawRotation[0x30];
            Vec3 predecessorOffset; Vec3 predecessorDelta; Vec3 steeringOffset; Vec3 frameDisplacement; Vec3 forwardVelocity;
            __mi__FRC7WVectorRC7WVector(&predecessorDelta, &self->instanceMatrices[instanceIndex - 1].position, &self->instanceMatrices[instanceIndex].position);
            __as__7WVectorFRC7WVector(&predecessorOffset, &predecessorDelta);

            if (!((WVectorLen__FRC7WVector(&predecessorOffset)) < kFloat_3_2_CourseGeometry)) {
                float positiveTurnRate = self->turnRate;
                float positiveYawRate = kFloat_0_7_Creature * positiveTurnRate;
                float positiveYawStep = positiveYawRate * deltaTime;
                maxTurn = positiveYawStep;
                follower = &self->instanceMatrices[instanceIndex];
                __mi__FRC7WVectorRC7WVector(&steeringOffset, &self->instanceMatrices[instanceIndex - 1].position, &follower->position);
                turnAngle = Vector_SignedAngleXZ(&steeringOffset, &follower->forward);
                float negativeTurnRate = self->turnRate;
                float negativeYawRate = kFloat_neg0_7_Creature * negativeTurnRate;
                minTurn = negativeYawRate * deltaTime;
                if (turnAngle < minTurn) {
                    clampedTurn = minTurn;
                } else {
                    maxTurn = (turnAngle > maxTurn) ? maxTurn : turnAngle;
                    float upperClampedYawStep = maxTurn;
                    clampedTurn = upperClampedYawStep;
                }
                Matrix_SetAxisRotation(yawRotation, clampedTurn, kYawAxis);
                Elem* instanceMatrices = self->instanceMatrices;
                Matrix_ConcatAffine(rotatedTransform, yawRotation, (char*)instanceMatrices + transformOffset);
                Matrix_CopyAffine((char*)self->instanceMatrices + transformOffset, rotatedTransform);
                __ml__FfRC7WVector(&forwardVelocity, (char*)self->instanceMatrices + transformOffset + 0x18, self->movementSpeed);
                __ml__FRC7WVectorf(&frameDisplacement, &forwardVelocity, deltaTime);
                __apl__7WVectorFRC7WVector((char*)self->instanceMatrices + transformOffset + 0x24, &frameDisplacement);
            }
        }
    }
}

// Place the mole near the ball, turn toward it on non-category-3 ground, and lower its height.
void CMole_SpawnNearBall(C6Obj* self) { unsigned char rot[0x30];
    void* mgr = *(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc; // original +0xB90
    void* item = GolfPlayer_GetEntryByIndex(mgr, AsCGolfDoc(mgr)->currentPlayer);
    if (((CreaturePlayerGateView*)item)->creatureSpawnEligibility < 1) return;
    self->animationFrame = kFloat_0_Creature;
    self->animationActive = 1;
    self->opacity = kCreatureOpaque;

    Matrix_SetAxisRotation(rot, Random_RangeFloat(-1, kFloat_neg3_1415927_Creature, kFloat_3_1415927_805DF2E4), 1);
    Matrix_CopyAffine(self->basisXY, rot);

    *(WVector*)self->position = GROUND_BV_CREATURE(CQuadTree_GetInstance(),
        *(WVector*)GolfBall__Fv() + self->ballSpawnOffsetDistance * (*(WVector*)self->forward * kFloat_neg1_CreatureDirection), 0, 0, 1);
    Vec3 vec = *(Vec3*)self->position;

    if (3 != (GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &vec))) {
        *(WVector*)self->forward = *(WVector*)GolfBall__Fv() - kFloat_0_5_Creature * ((*(WVector*)&AsCCreature(self)->movementBoundsMax[0]) + (*(WVector*)&AsCCreature(self)->movementBoundsOrigin[0]));
        AsCCreature(self)->transform[7] = kFloat_0_Creature;
        WVector tmp;
        Normalize__7WVectorFv(self->forward);
        Vector_CrossProduct(&tmp, ((char*)&AsCCreature(self)->transform[3]), self->forward);
        __as__7WVectorFRC7WVector(self->basisXY, &tmp);
        Normalize__7WVectorFv(self->basisXY);
        *(WVector*)self->position = GROUND_BV_CREATURE(CQuadTree_GetInstance(),
            *(WVector*)GolfBall__Fv() + self->ballSpawnOffsetDistance * (*(WVector*)self->forward * kFloat_neg1_CreatureDirection), 0, 0, 1);
    }
    AsCCreature(self)->transform[10] -= kFloat_0_7_Creature;
}
} // extern "C"

} // namespace ns0

// ---- merged from course_creatures_flocks_and_flyers.cpp ----
namespace ns1 {
// Translation unit 0x8019CBFC-0x8019E368 (7 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void CCreature_dtor(void*, int);
void __dl__FPv(void*);
void Vec3_Zero(void*);
void AnimationEventParser_ProcessFrameRange(void*, void*, float, float, void*, int, int); /* AnimationEventParser_ProcessFrameRange is (self, manager r4, bounds r5, f1, f2, r6, r7): f2 is the new time (0x802A663C reads it) */
int rand();
int CString_Compare(const char*, const char*);
void* WPuppet_FindAnimationRangeByName(void*, const char*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float Magnitude__7WVectorCFv(void*);
void __as__7WVectorFRC7WVector(void*, void*);
void __ml__FfRC7WVector(void*, void*, float);
float Vector_SignedAngleXZ(void*, void*);
void fn_8004BA28v(void*, void*);
void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
void Matrix_SetAxisRotation(void*, float, signed char);
void Matrix_ConcatAffine(void*, void*, void*);
void Matrix_CopyAffine(void*, void*);
void __ml__FRC7WVectorf(void*, void*, float);
void __apl__7WVectorFRC7WVector(void*, void*);
void* __nwa__FUl(unsigned int);
void __dla__FPv(void*);
extern unsigned char __vt__Q23std11logic_error[];
extern unsigned char s_sharedCharDeleterVtable[];
extern unsigned char __vt__Q23std12length_error[];
void __throw(void*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*);
unsigned int strlen(const char*);
char* strcpy(char*, const char*);

struct Obj8019 {
    unsigned int vptr[2];
    unsigned int flags;
    unsigned char movementBoundsOrigin[0x0C];
    unsigned char movementBoundsMax[0x0C];
    void* puppet;
    unsigned char basisXY[0x18];
    float forwardX;
    float forwardY;
    float forwardZ;
    float positionX;
    float positionY;
    float positionZ;
    float animationFrame;
    unsigned char pad05C[4];
    void* animationRange;
    float targetX;
    float targetY;
    float targetZ;
    float turnRate;
    float movementSpeed;
    float bankAngle;
    float maxBankAngle;
    float bankRate;
    float heading;
    unsigned char pad088[0x20];
    void* history;
    unsigned char pad0AC;
    unsigned char animationActive;
    unsigned char pad0AE[2];
    float ballSpawnOffsetDistance;
    void* opacityKeyframeCount;
    void* opacityKeyframeNext;
    void* opacityKeyframePrevious;
    unsigned int targetPoints;
    unsigned int targetPointCount;
    unsigned int targetPointCapacity;
    unsigned char targetCursor;
    unsigned char pad0CD[3];
    unsigned int pathPoints;
    unsigned int pathPointCount;
    unsigned int pathPointCapacity;
    unsigned char pad0DC[0x40];
    float verticalSpeed;
    int bankPhase;
};

// Release CSanta resources and optionally delete the object.
void* CSanta_Dtor(void* p, int del) {
    if (p) {
        CCreature_dtor(p, 0);
        if (del > 0)
            __dl__FPv(p);
    }
    return p;
}

// Release CFighter resources and optionally delete the object.
void* CFighter_Dtor(void* p, int del) {
    if (p) {
        CCreature_dtor(p, 0);
        if (del > 0)
            __dl__FPv(p);
    }
    return p;
}

extern unsigned char __vt__9CCreature[];
extern unsigned char __vt__6CSanta[];
extern unsigned char __vt__8CFighter[];
extern unsigned char npcResourceStrings[];
extern unsigned char s_cameraTransform[];
extern const float kFloat_0_Creature;
extern void* g_soundManager;
extern float Random_RangeFloat(int, float, float);
extern int SoundManager_IsAliasPrefixOrStreamStemActive(void*, const char*);
extern void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);
extern void SoundManager_SetNamedVolumeScale(void*, const char*, float);
extern void Sound_StopByAlias(void*, const char*);

// Initialize CSanta state and owned resources.
Obj8019* CSanta_Ctor(Obj8019* self, void* arg) {
    self->vptr[1] = (unsigned int)(__vt__9CCreature + 0x0C);
    self->vptr[0] = (unsigned int)__vt__9CCreature;
    self->flags = 0;
    Vec3_Zero(self->movementBoundsOrigin);
    Vec3_Zero(self->movementBoundsMax);
    self->puppet = 0;
    self->animationFrame = 0.0f;
    self->animationRange = 0;
    self->heading = 0.0f;
    self->history = 0;
    self->ballSpawnOffsetDistance = 0.0f;
    self->opacityKeyframeCount = 0;
    self->opacityKeyframeNext = self->opacityKeyframePrevious = ((char*)&AsCCreature(self)->opacityKeyframeNext);
    self->targetPoints = 0;
    self->targetPointCount = 0;
    self->targetPointCapacity = 0;
    self->pathPoints = 0;
    self->pathPointCount = 0;
    self->pathPointCapacity = 0;
    self->vptr[0] = (unsigned int)__vt__6CSanta;
    self->vptr[1] = (unsigned int)(__vt__6CSanta + 0x0C); __typeof__(self->vptr[0]) _p637 = self->vptr[0];
    try {
    ((void (*)(Obj8019*, void*))((unsigned int*)_p637)[0x98 / 4])(self, arg);
    self->targetCursor = 1;
    self->movementSpeed = 120.0f;
    self->maxBankAngle = 0.7853981852531433f;
    self->turnRate = 2.094395160675049f;
    self->verticalSpeed = 0.0f;
    self->bankRate = 1.5707963705062866f;
    self->bankPhase = 2;
    return self;
    } catch (...) {
        CCreature_dtor(self, -1);
        throw;
    }
}

// Initialize CFighter state and owned resources.
Obj8019* CFighter_Ctor(Obj8019* self, void* arg) {
    self->vptr[1] = (unsigned int)(__vt__9CCreature + 0x0C);
    self->vptr[0] = (unsigned int)__vt__9CCreature;
    self->flags = 0;
    Vec3_Zero(self->movementBoundsOrigin);
    Vec3_Zero(self->movementBoundsMax);
    self->puppet = 0;
    self->animationFrame = 0.0f;
    self->animationRange = 0;
    self->heading = 0.0f;
    self->history = 0;
    self->ballSpawnOffsetDistance = 0.0f;
    self->opacityKeyframeCount = 0;
    self->opacityKeyframeNext = self->opacityKeyframePrevious = ((char*)&AsCCreature(self)->opacityKeyframeNext);
    self->targetPoints = 0;
    self->targetPointCount = 0;
    self->targetPointCapacity = 0;
    self->pathPoints = 0;
    self->pathPointCount = 0;
    self->pathPointCapacity = 0;
    self->vptr[0] = (unsigned int)__vt__8CFighter;
    self->vptr[1] = (unsigned int)(__vt__8CFighter + 0x0C); __typeof__(self->vptr[0]) _p803 = self->vptr[0];
    try {
    ((void (*)(Obj8019*, void*))((unsigned int*)_p803)[0x98 / 4])(self, arg);
    self->targetCursor = 1;
    self->movementSpeed = 320.0f;
    self->maxBankAngle = 0.7853981852531433f;
    self->turnRate = 2.094395160675049f;
    self->verticalSpeed = 0.0f;
    self->bankRate = 1.5707963705062866f;
    self->bankPhase = 2;
    return self;
    } catch (...) {
        CCreature_dtor(self, -1);
        throw;
    }
}

struct V90b_ { 
virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19(); virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24(); virtual void v25(); virtual void v26(); virtual void v27(); virtual void v28(); virtual void v29(); virtual void v30(); virtual void v31(); virtual void v32(); virtual void v33(); virtual void v34(float);
};
// Advance Santa animation events, occasionally switch its default clip, then update movement.
void CSanta_UpdateAnimationAndMovement(Obj8019* self, float dt) {
    if (!self->puppet) return;
    // 0x8019CD68: Use the active waypoint path.
    do {
        if (self->animationActive) break;
        return;
    } while (0);

    float t = self->animationFrame + dt;
    self->animationFrame = t;
    if (t < kFloat_0_Creature) return;
    AnimationEventParser_ProcessFrameRange((char*)self + 4, self->puppet, t - dt, t, self->animationRange, -1, 0);
    // 0x8019CD68: Reuse the current animation or select its replacement before updating.
    do {
        if (self->animationFrame > ((CreatureAnimationRange*)self->animationRange)->endTime) {
            self->animationFrame -= ((CreatureAnimationRange*)self->animationRange)->endTime;

            if ((rand()) % 10 == 0 && !CString_Compare(((CreatureAnimationRange*)self->animationRange)->name, (const char*)npcResourceStrings + kCreatureStrings_defaultAnimation)) {
                void* node = self->puppet;
                const char* name = (const char*)npcResourceStrings + kCreatureStrings_alternateDefaultAnimation;
                if (node) {
                    if (self->animationRange && !CString_Compare(((CreatureAnimationRange*)self->animationRange)->name, name)) break;
                    void* found = WPuppet_FindAnimationRangeByName(node, name);
                    if (found) { self->animationRange = found;
                        float zero = kFloat_0_Creature;

                        float start = *(float*)found;
                        float end = ((CreatureAnimationRange*)found)->endTime;
                        self->animationFrame = ((start + zero)) > end ? end : ((zero + start));
                    } else self->animationFrame = kFloat_0_Creature;
                }
            } else {
                void* node = self->puppet;
                const char* name = (const char*)npcResourceStrings + kCreatureStrings_defaultAnimation;
                if (node) {
                    if (self->animationRange && !CString_Compare(((CreatureAnimationRange*)self->animationRange)->name, name)) break;
                    void* found = WPuppet_FindAnimationRangeByName(node, name);
                    if (found) {
                        self->animationRange = found;

                        float zero = kFloat_0_Creature;
                        float start = *(float*)found;
                        float end = ((CreatureAnimationRange*)found)->endTime;
                        self->animationFrame = ((start + zero)) > end ? end : (start + zero);
                    } else self->animationFrame = kFloat_0_Creature;
                }
            }
        }
    } while (0);

    ((V90b_*)self)->v34(dt);
}

typedef Vec3f Vec3;
void Matrix_TransformPoint(void*, void*, void*);
float WVectorLen__FRC7WVector(void*);
void dtor_80126448(void*, int);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
void __dt__Q33std6detail16char_array_guardFv(void*, int);
void* __nw__FUl(unsigned int);

struct Grow8019 {
    Vec3* data;
    unsigned int size, capacity;
    unsigned int* allocator;
    unsigned int remaining;
    // Initialize an empty temporary waypoint buffer tied to the owner capacity.
    Grow8019(unsigned int* a) : data(0), size(0), capacity(0), allocator(a), remaining(0) {}
    // Free the temporary waypoint allocation after transfer or unwind.
    ~Grow8019() { if (data) { size=0; appFreeUni__FPv(data); } }
};
struct CopyVec8019 {
    unsigned int x,y,z;
    // Copy the three XYZ storage words without numeric conversion.
    CopyVec8019(const CopyVec8019& v) : x(v.x), y(v.y), z(v.z) {}
};
// Return the larger unsigned growth request by reference.
inline const unsigned int& max8019(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
// Return magnitude using the original positive-versus-nonpositive branch.
inline float abs8019(float v) { return v > 0.0f ? v : -v; }
// Clamp a steering value to the supplied lower and upper bounds.
inline float clamp8019(float v, float low, float high) {
    return v < low ? low : (v > high ? high : v);
}
struct Guard8019 {
    void* data;
    void* allocator;
    // Guard an allocated character buffer until ownership transfers.
    Guard8019(void* p, void* a):data(p),allocator(a){}
    // Free the guarded character buffer unless ownership transferred.
    ~Guard8019() { if (data) __dla__FPv(data); }
};
struct Error8019 {
    void* vtable;
    char* text;
    void* shared;
    // Build an owned exception message with a shared character control block.
    Error8019(const char* message) {
        vtable = __vt__Q23std11logic_error;
        char alloc = 0;
        text = (char*)__nwa__FUl(strlen(message)+1);
        {
            Guard8019 guard(text, &alloc);
            unsigned int* block = (unsigned int*)__nw__FUl(16);
            if (block) {
                block[1]=1; block[2]=1; block[0]=(unsigned int)s_sharedCharDeleterVtable; block[3]=(unsigned int)text;
            }
            shared = block;
            guard.data=0;
        }
        strcpy(text, message);
    }
    // Run the base exception destructor without freeing wrapper storage.
    ~Error8019() { __dt__Q23std9exceptionFv(this, -1); }
};
// Construct and throw the waypoint length error through the game ABI.
inline void lengthError8019() {
    Error8019 error((const char*)npcResourceStrings);
    error.vtable = __vt__Q23std12length_error;
    __throw(npcResourceStrings + kCreatureStrings_lengthErrorType, &error, (void*)__dt__Q23std12length_errorFv);
}

struct PathAllocator8019 {
    volatile unsigned char value;
    // Preserve allocator state initialization required by the exception ABI.
    PathAllocator8019() : value(0) {}
    // Preserve allocator state initialization required by the exception ABI.
    PathAllocator8019(int) {}
};
// Return the maximum count of twelve-byte path records.
inline unsigned int pathMaxSize8019() {
    PathAllocator8019 allocator;
    return 0x15555555u;
}
struct PathPoint8019 {
    unsigned int x,y,z;
    // Copy one XYZ record while preserving the exception boundary.
    PathPoint8019(const PathPoint8019& p) {
        try {
            unsigned int a, b;
            const volatile PathPoint8019* source=&p;
            a=source->x; b=source->y;
            x=a; y=b; z=source->z;
        } catch (...) { throw; }
    }
};
// Copy one path point while retaining the original exception-cleanup boundary.
inline void pathConstruct8019(const PathPoint8019& source, PathPoint8019* dest) {
    new(dest) PathPoint8019(source);
}
} // extern "C"
} // namespace ns1
namespace std {
namespace detail {
struct CharArrayGuard_u8019B4B0 {
    void* data;
    void* allocator;
    // Guard the exception-message allocation until shared ownership is installed.
    CharArrayGuard_u8019B4B0(void* p,void* a):data(p),allocator(a) {}
    // Free an exception-message array that has not transferred.
    ~CharArrayGuard_u8019B4B0() { if(data) ns1::__dla__FPv(data); }
};
}
struct exception {
    void* vtable;
    char* text;
    void* shared;
    // Preserve base-exception allocator initialization for this overload.
    exception() { ns1::PathAllocator8019 allocator; }
    // Preserve base-exception allocator initialization for this overload.
    exception(volatile unsigned char& allocator) { allocator=0; }
    ~exception();
};
struct logic_error : exception {
    // Construct the logic-error message and shared character ownership.
    logic_error(const char* const& message) {
        vtable=ns1::__vt__Q23std11logic_error;
        ns1::PathAllocator8019 allocator;
        unsigned int length=strlen(message)+1;
        text=(char*)__nwa__FUl((length<<1)-length);
        {
            char* storage=text;
            detail::CharArrayGuard_u8019B4B0 guard(storage,&allocator);
            unsigned int* block=(unsigned int*)ns1::__nw__FUl(16);
            if(block) {
                block[1]=1; block[2]=1;
                block[0]=(unsigned int)ns1::s_sharedCharDeleterVtable;
                block[3]=(unsigned int)storage;
            }
            shared=block;
            guard.data=0;
        }
        strcpy(text,message);
    }
    // Build the owned message while exposing both allocator markers to cleanup.
    logic_error(const char* const& message, volatile unsigned char& baseAllocator,
                volatile unsigned char& allocator) : exception(baseAllocator) {
        vtable=ns1::__vt__Q23std11logic_error;
        allocator=0;
        unsigned int length=strlen(message)+1;
        text=(char*)__nwa__FUl((length<<1)-length);
        {
            char* storage=text;
            detail::CharArrayGuard_u8019B4B0 guard(storage,(void*)&allocator);
            unsigned int* block=(unsigned int*)ns1::__nw__FUl(16);
            if(block) {
                block[1]=1; block[2]=1;
                block[0]=(unsigned int)ns1::s_sharedCharDeleterVtable;
                block[3]=(unsigned int)storage;
            }
            shared=block;
            guard.data=0;
        }
        strcpy(text,message);
    }
    ~logic_error();
};
struct length_error : logic_error {
    // Construct the owned length-error message and select its exception type.
    length_error(const char* const& message):logic_error(message) {
        vtable=ns1::__vt__Q23std12length_error;
    }
    // Construct the length-error subtype with explicit allocator cleanup markers.
    length_error(const char* const& message, volatile unsigned char& baseAllocator,
                 volatile unsigned char& allocator):logic_error(message,baseAllocator,allocator) {
        vtable=ns1::__vt__Q23std12length_error;
    }
    ~length_error();
};
}
namespace ns1 {
extern "C" {
#define PATH_LENGTH_ERROR_8019() { throw std::length_error((const char*)npcResourceStrings); }
#define PATH_LOCAL_LENGTH_ERROR_8019() { \
    PathAllocator8019 baseAllocator(0), textAllocator(0); \
    throw std::length_error((const char*)npcResourceStrings,baseAllocator.value,textAllocator.value); \
}
// Throw the path-vector length error at this helper boundary.
inline void pathThrowLength8019() {
    PATH_LENGTH_ERROR_8019();
}
struct PathGrow8019 {
    PathPoint8019* data;
    unsigned int size, capacity;
    unsigned int* allocator;
    unsigned int remaining;
    // Initialize a temporary path allocation tied to the owner capacity.
    PathGrow8019(unsigned int* a) : data(0),size(0),capacity(0),allocator(a),remaining(0) {}
    // Free whichever path allocation remains owned after swap or unwind.
    ~PathGrow8019() {
        if (data) {
            PathAllocator8019 alloc;
            size-=size;
            appFreeUni__FPv(data);
        }
    }
};

struct StopPath8019 : V90b_ { 
virtual void v35(int);
};
// Reference parameters borrow caller locals to retain CodeWarrior register lifetimes.
// Integrate the shared three-state flight bank policy in radians and dt time units.
static inline void IntegrateFlightBank(Obj8019*& self, float& dt, float& turn) {
    switch (self->bankPhase) {
    case kBankReturnToLevel: { // Ease roll back toward level at bankRate.
        float rollAngle=self->bankAngle;
        if ((rollAngle>0?rollAngle:-rollAngle)>self->bankRate*dt) {
            float rollRate = self->bankRate;
            if (self->bankAngle > 0.0f) rollRate = -rollRate;
            self->bankAngle += dt*rollRate;
        }
        else { self->bankPhase=kBankLevel; self->bankAngle=0; }
        break;
    }
    case kBankIntoTurn: { // Bank away from the turn sign up to maxBankAngle.
        float rollAngle=self->bankAngle;
        if ((rollAngle>0?rollAngle:-rollAngle)<self->maxBankAngle) self->bankAngle += dt*(turn>0.0f ? -self->bankRate : self->bankRate);
        else self->bankPhase=kBankReturnToLevel;
        break;
    }
    case kBankLevel: // Hold the current level roll; no integration.
    default:
        break;
    }
}

// Add a world-space offset with yaw in [-pi,pi] and pitch in [-pi/2,pi/2].
// This samples angles, not a uniform sphere; radius is ballSpawnOffsetDistance.
static inline void JitterSantaTarget(Obj8019*& self, float* randomRotation, float* randomPitch, Vec3& offset, Vec3& adjusted) {
    signed char yawAxis=1;
    float yawAngle=Random_RangeFloat(-1, -3.14159274f, 3.14159274f);
    ((void (*)(void*, float, signed char))Matrix_SetAxisRotation)(randomRotation,yawAngle,yawAxis);
    signed char pitchAxis=0;
    float pitchAngle=Random_RangeFloat(-1, -1.57079637f, 1.57079637f);
    ((void (*)(void*, float, signed char))Matrix_SetAxisRotation)(randomPitch,pitchAngle,pitchAxis);
    __ml__FfRC7WVector(&offset,randomPitch+6,self->ballSpawnOffsetDistance);
    Matrix_TransformPoint(&adjusted,&offset,randomRotation);
    __apl__7WVectorFRC7WVector(&self->targetX,&adjusted);
}

// Derive world-units/time speeds from the resolved XZ distance and authored leg.
// The authored horizontal length / 120 is the nominal travel time; Y follows it.
static inline void SetFlightLegSpeeds(Obj8019*& self, float& horizontalDistance, Vec3& authoredSegment) {
    self->movementSpeed = 120.0f * horizontalDistance / Magnitude__7WVectorCFv(&authoredSegment);
    self->verticalSpeed = (self->movementSpeed * (self->targetY - self->positionY)) / horizontalDistance;
}

// Follow or record waypoints, bank into turns, and scale Santa boost sound by camera distance.
// Both flight updates use the movement-rate time base passed in dt (no conversion).
// World positions are XYZ, steering is in XZ, and angles/rates use radians.
// Precondition: nonempty targetPoints and nonzero horizontal segment lengths;
// recorded pathPoints is an owned vector of 12-byte XYZ bit-copy records.
// pathCursor is a byte; targetCursor selects the authored target sequence.
// Santa moves along NEGATIVE forward; Fighter below moves along POSITIVE forward.
// Preserve that difference when changing movement or replay behavior.
// Admission for BOTH entries: initialization must have supplied targetPoints
// (native float XYZ, 12-byte stride) before movement starts. Constructors clear
// its vector then call initialization slot +98 and set targetCursor=1. For that
// initial cursor require 2 <= targetPointCount <= 256; subsequent cursor must be
// in [0,count). targetCursor at +CC is unsigned byte: counts above 256 cannot be
// traversed. Last authored target stops before modulo/increment. Count zero can
// reach modulo-by-zero; these methods do not validate or synthesize a path.
// Replay pathCursor +DC is also a byte. A finite replay must have <=255 records;
// 256+ lets the increment wrap to zero before exhaustion and repeats the prefix.
// Recording can allocate more records, but that does not widen replay's cursor.
// Owned target/path arrays are released by creature teardown (CCreature_dtor);
// movement borrows them and can grow only the recorded vector. No pointer escapes.
// Every Santa authored XZ segment AND resolved target XZ distance must be nonzero:
// SetFlightLegSpeeds divides by both. Fighter sets speed=resolved XZ distance and
// computes speed*deltaY/distance without cancelling; distance zero produces NaN
// verticalSpeed (0/0), then arrival's strict distance < speed*lookahead can stall.
// No epsilon/skip/vertical-only-leg policy exists; author distinct XZ waypoints.
// Negative/nonfinite dt is unsupported; RNG calls, terrain and audio are side
// effects. Failure while growing may leave the new target/history already set.
void CSanta_UpdateFlightMovement(Obj8019* self, float dt) {
    Vec3 direction, segment, adjusted, offset, next, move, turnForward, step, forward, negForward, distance;
    float randomPitch[12], randomRotation[12], combined[12], rotation[12];
    // Phase 1: decide arrival in XZ with max(dt,0.1) as the look-ahead interval.
    // Select a new authored target when recording or replay is exhausted.
    __mi__FRC7WVectorRC7WVector(&direction, &self->targetX, &self->positionX);
    direction.y = 0.0f;
    float arrivalLookahead = dt < 0.1f ? 0.1f : dt;
    if (Magnitude__7WVectorCFv(&direction) < self->movementSpeed * arrivalLookahead) {
        unsigned int recordedPointCount;
        unsigned char replayPointIndex;
        recordedPointCount = self->pathPointCount;
        replayPointIndex = AsCCreature(self)->pathCursor;
        if (replayPointIndex >= recordedPointCount || AsCCreature(self)->recordPath) {
            unsigned int count = self->targetPointCount;
            unsigned int index = self->targetCursor;
            if (index == count - 1) {
                ((StopPath8019*)self)->v35(0);
                return;
            }
            self->targetCursor = ((unsigned char)index + 1) % count;
            __as__7WVectorFRC7WVector(&self->targetX, (char*)self->targetPoints + self->targetCursor * 12);
            JitterSantaTarget(self, randomRotation, randomPitch, offset, adjusted);
            __mi__FRC7WVectorRC7WVector(&next, &self->targetX, &self->positionX);
            __as__7WVectorFRC7WVector(&direction, &next);
            direction.y = 0.0f;
            __mi__FRC7WVectorRC7WVector(&segment,(char*)self->targetPoints+self->targetCursor*12,(char*)self->targetPoints+(unsigned char)index*12);
            segment.y=0.0f;
            float resolvedHorizontalDistance = Magnitude__7WVectorCFv(&direction);
            // Jitter changes the destination in world XYZ, not the authored path.
            // Let L be the original horizontal segment length and D the distance
            // from the current position to the jittered target. Speed=120*D/L
            // sets nominal straight-line travel time D/speed=L/120 in dt units.
            // Vertical speed uses that same nominal interval. Yaw limits and
            // the arrival look-ahead still affect actual arrival time/height.
            // Raising jitter can increase speed; it does not independently slow
            // travel. Tune 120 for traversal rate, ballSpawnOffsetDistance for spread.
            SetFlightLegSpeeds(self, resolvedHorizontalDistance, segment);
            __ml__FfRC7WVector(&move, &self->forwardX, -1.0f);
            float angle = Vector_SignedAngleXZ(&direction, &move);
            angle = angle > 0.0f ? angle : -angle;
            self->maxBankAngle = angle > 1.57079637f ? 1.57079637f : angle;
            self->bankPhase = kBankIntoTurn;
            // Phase 2: record the resolved target, not the source authored point.
            // Reserve -> construct tail -> reverse-copy prefix -> swap; grow owns
            // new storage until swap, then old storage for automatic cleanup.
            if (AsCCreature(self)->recordPath) {
              unsigned int oldSize=self->pathPointCount;
              unsigned int oldLimit=self->pathPointCapacity;
              PathAllocator8019 appendAllocator;
              if (oldSize < oldLimit) {
                PathPoint8019* dst = (PathPoint8019*)(self->pathPoints + oldSize * 12);
                pathConstruct8019(*(PathPoint8019*)(&self->targetX), dst);
                self->pathPointCount++;
              } else {
                PathAllocator8019 growAllocator;
                if (pathMaxSize8019()-oldLimit < 1) pathThrowLength8019();
                unsigned int neededCount=self->pathPointCount;
                unsigned int neededLimit=self->pathPointCapacity;
                unsigned int needed = neededCount + 1 - neededLimit;
                PathGrow8019 grow(&self->pathPointCapacity);
                unsigned int capacity = self->pathPointCapacity;
                if (needed > kMaxWaypointCount-capacity) PATH_LOCAL_LENGTH_ERROR_8019();
                unsigned int halfIncrement, smallIncrement;
                if (capacity < kWaypointThreeFifthsGrowthLimit) {
                    smallIncrement = ((capacity+1)*3)/5;
                    capacity += max8019(smallIncrement, needed);
                } else if (capacity < kWaypointHalfGrowthLimit) {
                    halfIncrement = (capacity+1)/2;
                    capacity += max8019(halfIncrement, needed);
                } else capacity=kMaxWaypointCount;
                PathAllocator8019 allocateAllocator;
                if (capacity > kMaxWaypointCount) PATH_LOCAL_LENGTH_ERROR_8019();
                PathPoint8019* allocated=(PathPoint8019*)appMallocDummy__FUl(capacity*12);
                unsigned int oldCount=self->pathPointCount;
                grow.data = allocated;
                grow.capacity=capacity;
                PathPoint8019* dst=allocated+oldCount+grow.size;
                grow.remaining=oldCount;
                PathAllocator8019 insertAllocator;
                for (unsigned int n=1; n!=0; --n, ++dst) {
                    new(dst) PathPoint8019(*(PathPoint8019*)(&self->targetX));
                    ++grow.size;
                }
                PathAllocator8019 copyAllocator;
                PathPoint8019* end=(PathPoint8019*)self->pathPoints+self->pathPointCount;
                PathPoint8019* begin=(PathPoint8019*)self->pathPoints;
                dst=grow.data+grow.remaining;
                while (end>begin) {
                    --dst; --end;
                    new(dst) PathPoint8019(*(PathPoint8019*)end);
                    --grow.remaining;
                    ++grow.size;
                }
                self->pathPointCount=0;
                PathAllocator8019 clearAllocator;
                PathPoint8019* old=(PathPoint8019*)self->pathPoints;
                PathPoint8019* replacement=grow.data;
                grow.data=old;
                unsigned int oldCapacity=self->pathPointCapacity;
                self->pathPointCapacity=grow.capacity;
                grow.capacity=oldCapacity;
                self->pathPoints=(unsigned int)replacement;
                self->pathPointCount=grow.size;
                grow.size=0;
              }
            }
        } else {
            if (replayPointIndex < recordedPointCount) {
                // Replay consumes exactly one stored XYZ point and advances the byte cursor.
                __as__7WVectorFRC7WVector(&self->targetX, (char*)self->pathPoints + replayPointIndex*12);
                ++AsCCreature(self)->pathCursor;
            } else self->animationActive = 0;
        }
    }
    // Phase 3: clamp yaw to turnRate*dt, then integrate bank separately.
    // Bank states: IntoTurn -> ReturnToLevel -> Level (no additional roll).
    __ml__FfRC7WVector(&turnForward, &self->forwardX, -1.0f);
    float turn = Vector_SignedAngleXZ(&direction, &turnForward);
    if ((turn > 0.0f ? turn : -turn) > 0.0174532924f) {
        float low=-self->turnRate * dt;
        float high=self->turnRate * dt;
        float clamped = clamp8019(turn, low, high);
        signed char axis = 1;
        ((void (*)(void*, float, signed char))Matrix_SetAxisRotation)(rotation, clamped, axis);
        Matrix_ConcatAffine(combined, rotation, self->basisXY);
        Matrix_CopyAffine(self->basisXY, combined);
    }
    IntegrateFlightBank(self, dt, turn);
    __ml__FfRC7WVector(&negForward, &self->forwardX, -1.0f);
    __ml__FfRC7WVector(&forward, &negForward, self->movementSpeed);
    __ml__FRC7WVectorf(&step, &forward, dt);
    __apl__7WVectorFRC7WVector(&self->positionX, &step);
    // Phase 4: integrate world position; vertical speed reaches target height
    // over the horizontal segment travel time calculated on target selection.
    self->positionY += self->verticalSpeed*dt;
    __mi__FRC7WVectorRC7WVector(&distance, ((CreatureAffineWords*)s_cameraTransform)->position, &self->positionX);
    // Santa-only audio: camera distance in world units; linear gain falls to zero at 640.
    float range = WVectorLen__FRC7WVector(&distance);
    if (range < 640.0f) {
        if (!SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager, (const char*)npcResourceStrings + kCreatureStrings_santaSoundAlias))
            ((void (*)(void*, const char*, int, int, const char*, int, float, float, float))Sound_PlayByName)(g_soundManager, (const char*)npcResourceStrings + kCreatureStrings_santaBoostSound, 0, 1, (const char*)npcResourceStrings + kCreatureStrings_santaSoundAlias, 0, 0.5f, 200.0f, 1.0f);
        SoundManager_SetNamedVolumeScale(g_soundManager, (const char*)npcResourceStrings + kCreatureStrings_santaSoundAlias, (640.0f-range)/640.0f);
    } else Sound_StopByAlias(g_soundManager, (const char*)npcResourceStrings + kCreatureStrings_santaSoundAlias);
}

// Follow or record waypoints, interpolate height, and bank into horizontal turns.
// Shares the flight path/recording and banking invariants described above Santa.
// This variant stops by clearing animationActive and moves along positive forward.
// Required count/cursor limits, owned-array lifetime and zero-XZ/NaN behavior are
// the shared flight admission contract immediately above CSanta_UpdateFlightMovement.
void CFighter_FlyAlongWaypointPath(Obj8019* self, float dt) {
    Vec3 direction, next, move, turnForward, step, forward, distance;
    float combined[12], rotation[12];
    // Phase 1: decide arrival in XZ with max(dt,0.1) as the look-ahead interval.
    // Select a new authored target when recording or replay is exhausted.
    __mi__FRC7WVectorRC7WVector(&direction, &self->targetX, &self->positionX);
    direction.y = 0.0f;
    float arrivalLookahead = dt < 0.1f ? 0.1f : dt;
    if (Magnitude__7WVectorCFv(&direction) < self->movementSpeed * arrivalLookahead) {
        unsigned char replayPointIndex;
        unsigned int recordedPointCount;
        recordedPointCount = *(volatile unsigned int*)&self->pathPointCount;
        replayPointIndex = (*(volatile unsigned char*)&AsCCreature(self)->pathCursor);
        if (replayPointIndex >= recordedPointCount || AsCCreature(self)->recordPath) {
            unsigned int count = self->targetPointCount;
            unsigned int index = self->targetCursor;
            if (index == count - 1) {
                self->animationActive = 0;
                return;
            }
            self->targetCursor = ((unsigned char)index + 1) % count;
            __as__7WVectorFRC7WVector(&self->targetX, (char*)self->targetPoints + self->targetCursor * 12);
            __mi__FRC7WVectorRC7WVector(&next, &self->targetX, &self->positionX);
            __as__7WVectorFRC7WVector(&direction, &next);
            direction.y = 0.0f;
            float resolvedHorizontalDistance = Magnitude__7WVectorCFv(&direction);
            self->movementSpeed = resolvedHorizontalDistance;
            self->verticalSpeed = (self->movementSpeed * (self->targetY - self->positionY)) / resolvedHorizontalDistance;
            __ml__FfRC7WVector(&move, &self->forwardX, -1.0f);
            float angle = Vector_SignedAngleXZ(&direction, &move);
            angle = angle > 0.0f ? angle : -angle;
            self->maxBankAngle = angle > 1.57079637f ? 1.57079637f : angle;
            self->bankPhase = kBankIntoTurn;
            // Phase 2: record the resolved target, not the source authored point.
            // Reserve -> construct tail -> reverse-copy prefix -> swap; grow owns
            // new storage until swap, then old storage for automatic cleanup.
            if (AsCCreature(self)->recordPath) {
              unsigned int oldSize=*(volatile unsigned int*)&self->pathPointCount;
              unsigned int oldLimit=*(volatile unsigned int*)&self->pathPointCapacity;
              PathAllocator8019 appendAllocator;
              if (oldSize < oldLimit) {
                PathPoint8019* dst = (PathPoint8019*)(self->pathPoints + oldSize * 12);
                pathConstruct8019(*(PathPoint8019*)(&self->targetX), dst);
                self->pathPointCount++;
              } else {
                PathAllocator8019 growAllocator;
                if (pathMaxSize8019()-oldLimit < 1) pathThrowLength8019();
                unsigned int neededCount=*(volatile unsigned int*)&self->pathPointCount;
                unsigned int neededLimit=*(volatile unsigned int*)&self->pathPointCapacity;
                unsigned int needed = neededCount + 1 - neededLimit;
                PathGrow8019 grow(&self->pathPointCapacity);
                unsigned int capacity = self->pathPointCapacity;
                if (needed > kMaxWaypointCount-capacity) PATH_LOCAL_LENGTH_ERROR_8019();
                unsigned int halfIncrement, smallIncrement;
                if (capacity < kWaypointThreeFifthsGrowthLimit) {
                    smallIncrement = ((capacity+1)*3)/5;
                    capacity += max8019(smallIncrement, needed);
                } else if (capacity < kWaypointHalfGrowthLimit) {
                    halfIncrement = (capacity+1)/2;
                    capacity += max8019(halfIncrement, needed);
                } else capacity=kMaxWaypointCount;
                PathAllocator8019 allocateAllocator;
                if (capacity > kMaxWaypointCount) PATH_LOCAL_LENGTH_ERROR_8019();
                PathPoint8019* allocated=(PathPoint8019*)appMallocDummy__FUl(capacity*12);
                unsigned int oldCount=self->pathPointCount;
                grow.data = allocated;
                grow.capacity=capacity;
                PathPoint8019* dst=allocated+oldCount+grow.size;
                grow.remaining=oldCount;
                PathAllocator8019 insertAllocator;
                for (unsigned int n=1; n!=0; --n, ++dst) {
                    new(dst) PathPoint8019(*(PathPoint8019*)(&self->targetX));
                    ++grow.size;
                }
                PathAllocator8019 copyAllocator;
                PathPoint8019* end=(PathPoint8019*)self->pathPoints+self->pathPointCount;
                PathPoint8019* begin=(PathPoint8019*)self->pathPoints;
                dst=grow.data+grow.remaining;
                while (end>begin) {
                    --dst; --end;
                    new(dst) PathPoint8019(*(PathPoint8019*)end);
                    --grow.remaining;
                    ++grow.size;
                }
                self->pathPointCount=0;
                PathAllocator8019 clearAllocator;
                PathPoint8019* old=(PathPoint8019*)self->pathPoints;
                PathPoint8019* replacement=grow.data;
                grow.data=old;
                unsigned int oldCapacity=self->pathPointCapacity;
                self->pathPointCapacity=grow.capacity;
                grow.capacity=oldCapacity;
                self->pathPoints=(unsigned int)replacement;
                self->pathPointCount=grow.size;
                grow.size=0;
              }
            }
        } else {
            if (replayPointIndex < recordedPointCount) {
                // Replay consumes exactly one stored XYZ point and advances the byte cursor.
                __as__7WVectorFRC7WVector(&self->targetX, (char*)self->pathPoints + replayPointIndex*12);
                ++AsCCreature(self)->pathCursor;
            } else self->animationActive = 0;
        }
    }
    // Phase 3: clamp yaw to turnRate*dt, then integrate bank separately.
    // Bank states: IntoTurn -> ReturnToLevel -> Level (no additional roll).
    __ml__FfRC7WVector(&turnForward, &self->forwardX, -1.0f);
    float turn = Vector_SignedAngleXZ(&direction, &turnForward);
    if ((turn > 0.0f ? turn : -turn) > 0.0174532924f) {
        float low=-self->turnRate * dt;
        float high=self->turnRate * dt;
        float clamped = clamp8019(turn, low, high);
        signed char axis = 1;
        ((void (*)(void*, float, signed char))Matrix_SetAxisRotation)(rotation, clamped, axis);
        Matrix_ConcatAffine(combined, rotation, self->basisXY);
        Matrix_CopyAffine(self->basisXY, combined);
    }
    IntegrateFlightBank(self, dt, turn);
    __ml__FfRC7WVector(&forward, &self->forwardX, self->movementSpeed);
    __ml__FRC7WVectorf(&step, &forward, dt);
    __apl__7WVectorFRC7WVector(&self->positionX, &step);
    // Phase 4: integrate world position; vertical speed reaches target height
    // over the horizontal segment travel time calculated on target selection.
    self->positionY += self->verticalSpeed*dt;
}

} // extern "C"

} // namespace ns1

// ---- merged from course_creatures_flocks_and_flyers.cpp ----
namespace ns2 {
// Translation unit 0x8019E368-0x8019F9E4 (8 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

typedef Vec3f V3;

extern "C" void appFreeUni__FPv(void*);

struct VecTmp {
    V3* begin;
    unsigned count;
    unsigned cap;
    unsigned* capRef;
    unsigned pos;
    // Release the temporary vector allocation.
    ~VecTmp() {
        if (begin) {
            count = 0;
            appFreeUni__FPv(begin);
        }
    }
};

// Return the larger unsigned capacity-growth request by reference.
static inline const unsigned& mx(const unsigned& a, const unsigned& b) {
    return a < b ? b : a;
}

extern "C" {
void __dl__FPv(void*);
void __dla__FPv(void*);
size_t strlen(const char*);
char* strcpy(char*, const char*);
void* __nwa__FUl(unsigned int);
void __throw(void*, void*, void*);
void __dt__Q23std12length_errorFv(void*);
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[], npcResourceStrings[];
}

struct StrRep {
    void* vt;
    int refs;
    int weak;
    char* buf;
    // Create a shared message-buffer control block with initial reference counts.
    StrRep(char* b) : vt(s_sharedCharDeleterVtable), refs(1), weak(1), buf(b) {}
};

struct StrBuf {
    char* buf;
    char flag;
    // Guard the message buffer until its control block takes ownership.
    StrBuf(char* b) : buf(b), flag(0) {}
    // Free the message buffer if still guarded.
    ~StrBuf() { if (buf) __dla__FPv(buf); }
};

struct LenErr {
    void* vt;
    char* buf;
    StrRep* rep;
    // Set the base logic-error vtable for the temporary exception.
    LenErr() : vt(__vt__Q23std11logic_error) {}
    // Preserve the empty temporary-exception cleanup hook.
    ~LenErr() {}
};

// Construct and throw a vector length error through the game exception ABI.
static inline void throwLengthError() {
    LenErr e;
    unsigned n = strlen(npcResourceStrings) + 1;
    char* buf = (char*)__nwa__FUl(n * 2 - n);
    StrBuf holder(buf);
    e.buf = buf;
    e.rep = new StrRep(buf);
    holder.buf = 0;
    strcpy(e.buf, npcResourceStrings);
    e.vt = __vt__Q23std12length_error;
    __throw(npcResourceStrings + kCreatureStrings_lengthErrorType, &e, (void*)__dt__Q23std12length_errorFv);
}

extern "C" {

struct Obj_8019E368 {
    void* vtable;
    void* secondaryVtable;
    char padding008[4];
    char base0C[0x0C];
    char base18[0x0C];
    int puppet;
    char transform[0x30];
    float animationFrame;
    char padding05C[4];
    int animationRange;
    char targetPosition[0x0C];
    float turnRate;
    float movementSpeed;
    float bankAngle;
    float maxBankAngle;
    float bankRate;
    float heading;
    char padding088[0x20];
    int history;
    char padding0AC[4];
    float ballSpawnOffsetDistance;
    int opacityKeyframeCount;
    void* opacityKeyframeNext;
    void* opacityKeyframePrevious;
    void* targetPoints;
    unsigned targetPointCount;
    int targetPointCapacity;
    unsigned char targetCursor;
    char padding0CD[3];
    int pathPoints;
    unsigned pathPointCount;
    unsigned pathPointCapacity;
    unsigned char pathCursor;
};

extern char __vt__9CCreature[];
extern char __vt__4CUfo[];
extern const float kFloat_0_Creature;
extern const float kFloat_0_1_Creature;
extern const float kFloat_neg3_1415927_Creature;
extern const float kFloat_3_1415927_805DF2E4;
extern const float kFloat_neg1_5707964_Creature;
extern const float kFloat_0_017453292_Creature;
extern const float kFloat_2_857143_Creature;
extern const float kFloat_0_7853982_Creature;
extern const float kFloat_2_0943952_Creature;
extern const float kHalfPi_Creature;
extern const float kFloat_120_Creature;
void Vec3_Zero(void*);
void CCreature_dtor(void*, int);
void __dl__FPv(void*);
extern char __vt__9ntPenguin[];
extern char npcResourceStrings[];
void AnimationEventParser_ProcessFrameRange(void*, int, float, float, int, int, int); /* AnimationEventParser_ProcessFrameRange is (self, manager r4, bounds r5, f1, f2, r6, r7): f2 is the new time (0x802A663C reads it) */
void* CString_Compare(void*, void*);
void* WPuppet_FindAnimationRangeByName(void*, void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float Magnitude__7WVectorCFv(void*);
void __as__7WVectorFRC7WVector(void*, void*);
void Matrix_SetAxisRotation(void*, float, signed char);
float Random_RangeFloat(int, float, float);
void __ml__FfRC7WVector(void*, void*, float);
void Matrix_TransformPoint(void*, void*, void*);
void __apl__7WVectorFRC7WVector(void*, void*);
float Vector_SignedAngleXZ(void*, void*);
void __ml__FRC7WVectorf(void*, void*, float);
void Matrix_ConcatAffine(void*, void*, void*);
void Matrix_CopyAffine(void*, void*);
void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
void* __nwa__FUl(unsigned int);
void __nw__FUl(unsigned int);
void __throw(void*, void*, void*);
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
void __dt__Q23std12length_errorFv(void*);
void* CQuadTree_GetInstance();
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);

// Initialize CUfo state and owned resources.
Obj_8019E368* CUfo_Ctor(Obj_8019E368* self, void* arg) {
    struct ConstructorBody : Obj_8019E368 {
    // Initialize this derived creature and its owned path/effect storage.
    Obj_8019E368* initialize(void* arg) {
    Obj_8019E368* self = this;
    struct VirtualObject {
        virtual void f00() DECOMP_LOCAL_VIRTUAL; virtual void f01() DECOMP_LOCAL_VIRTUAL; virtual void f02() DECOMP_LOCAL_VIRTUAL; virtual void f03() DECOMP_LOCAL_VIRTUAL;
        virtual void f04() DECOMP_LOCAL_VIRTUAL; virtual void f05() DECOMP_LOCAL_VIRTUAL; virtual void f06() DECOMP_LOCAL_VIRTUAL; virtual void f07() DECOMP_LOCAL_VIRTUAL;
        virtual void f08() DECOMP_LOCAL_VIRTUAL; virtual void f09() DECOMP_LOCAL_VIRTUAL; virtual void f10() DECOMP_LOCAL_VIRTUAL; virtual void f11() DECOMP_LOCAL_VIRTUAL;
        virtual void f12() DECOMP_LOCAL_VIRTUAL; virtual void f13() DECOMP_LOCAL_VIRTUAL; virtual void f14() DECOMP_LOCAL_VIRTUAL; virtual void f15() DECOMP_LOCAL_VIRTUAL;
        virtual void f16() DECOMP_LOCAL_VIRTUAL; virtual void f17() DECOMP_LOCAL_VIRTUAL; virtual void f18() DECOMP_LOCAL_VIRTUAL; virtual void f19() DECOMP_LOCAL_VIRTUAL;
        virtual void f20() DECOMP_LOCAL_VIRTUAL; virtual void f21() DECOMP_LOCAL_VIRTUAL; virtual void f22() DECOMP_LOCAL_VIRTUAL; virtual void f23() DECOMP_LOCAL_VIRTUAL;
        virtual void unusedSlot24() DECOMP_LOCAL_VIRTUAL; virtual void f25() DECOMP_LOCAL_VIRTUAL; virtual void f26() DECOMP_LOCAL_VIRTUAL; virtual void f27() DECOMP_LOCAL_VIRTUAL;
        virtual void unusedSlot28() DECOMP_LOCAL_VIRTUAL; virtual void f29() DECOMP_LOCAL_VIRTUAL; virtual void f30() DECOMP_LOCAL_VIRTUAL; virtual void f31() DECOMP_LOCAL_VIRTUAL;
        virtual void f32() DECOMP_LOCAL_VIRTUAL; virtual void f33() DECOMP_LOCAL_VIRTUAL; virtual void f34() DECOMP_LOCAL_VIRTUAL; virtual void f35() DECOMP_LOCAL_VIRTUAL;
        virtual void initialize(void*) DECOMP_LOCAL_VIRTUAL;
    };
    *(void* volatile*)((char*)self) = __vt__9CCreature;
    *(void* volatile*)((char*)self + 4) = __vt__9CCreature + 0x0C;
    (*(volatile int*)&AsCCreature(self)->flags) = 0;
    Vec3_Zero(((char*)&AsCCreature(self)->movementBoundsOrigin[0]));
    Vec3_Zero(((char*)&AsCCreature(self)->movementBoundsMax[0]));
    self->puppet = 0;
    self->animationFrame = kFloat_0_Creature;
    self->animationRange = 0;
    self->heading = kFloat_0_Creature;
    self->history = 0;
    self->ballSpawnOffsetDistance = kFloat_0_Creature;
    self->opacityKeyframeCount = 0;
    self->opacityKeyframePrevious = ((char*)&AsCCreature(self)->opacityKeyframeNext);
    self->opacityKeyframeNext = ((char*)&AsCCreature(self)->opacityKeyframeNext);
    self->targetPoints = 0;
    AsCCreature(self)->targetPointCount = 0;
    AsCCreature(self)->targetPointCapacity = 0;
    AsCCreature(self)->pathPoints = 0;
    AsCCreature(self)->pathPointCount = 0;
    AsCCreature(self)->pathPointCapacity = 0;
    self->vtable = __vt__4CUfo;
    self->secondaryVtable = __vt__4CUfo + 0x0C;
    try {
    ((VirtualObject*)self)->initialize(arg);
    ((FlightTargetCursorView*)self)->targetCursor = 1;
    self->movementSpeed = kFloat_2_857143_Creature;
    self->maxBankAngle = kFloat_0_7853982_Creature;
    self->turnRate = kFloat_2_0943952_Creature;
    ((FlightWaypointStorage*)self)->verticalSpeed = kFloat_0_Creature;
    self->bankRate = kHalfPi_Creature;
    ((FlightWaypointStorage*)self)->bankPhase = 2;
    return self;
    } catch (...) {
        CCreature_dtor(self, -1);
        throw;
    }
    }
    };
    return ((ConstructorBody*)self)->initialize(arg);
}

// Initialize ntPenguin state and owned resources.
Obj_8019E368* ntPenguin_Ctor(Obj_8019E368* self, void* arg) {
    struct ConstructorBody : Obj_8019E368 {
    // Initialize this derived creature and its owned path/effect storage.
    Obj_8019E368* initialize(void* arg) {
    Obj_8019E368* self = this;
    struct VirtualObject {
        virtual void f00() DECOMP_LOCAL_VIRTUAL; virtual void f01() DECOMP_LOCAL_VIRTUAL; virtual void f02() DECOMP_LOCAL_VIRTUAL; virtual void f03() DECOMP_LOCAL_VIRTUAL;
        virtual void f04() DECOMP_LOCAL_VIRTUAL; virtual void f05() DECOMP_LOCAL_VIRTUAL; virtual void f06() DECOMP_LOCAL_VIRTUAL; virtual void f07() DECOMP_LOCAL_VIRTUAL;
        virtual void f08() DECOMP_LOCAL_VIRTUAL; virtual void f09() DECOMP_LOCAL_VIRTUAL; virtual void f10() DECOMP_LOCAL_VIRTUAL; virtual void f11() DECOMP_LOCAL_VIRTUAL;
        virtual void f12() DECOMP_LOCAL_VIRTUAL; virtual void f13() DECOMP_LOCAL_VIRTUAL; virtual void f14() DECOMP_LOCAL_VIRTUAL; virtual void f15() DECOMP_LOCAL_VIRTUAL;
        virtual void f16() DECOMP_LOCAL_VIRTUAL; virtual void f17() DECOMP_LOCAL_VIRTUAL; virtual void f18() DECOMP_LOCAL_VIRTUAL; virtual void f19() DECOMP_LOCAL_VIRTUAL;
        virtual void f20() DECOMP_LOCAL_VIRTUAL; virtual void f21() DECOMP_LOCAL_VIRTUAL; virtual void f22() DECOMP_LOCAL_VIRTUAL; virtual void f23() DECOMP_LOCAL_VIRTUAL;
        virtual void unusedSlot24() DECOMP_LOCAL_VIRTUAL; virtual void f25() DECOMP_LOCAL_VIRTUAL; virtual void f26() DECOMP_LOCAL_VIRTUAL; virtual void f27() DECOMP_LOCAL_VIRTUAL;
        virtual void unusedSlot28() DECOMP_LOCAL_VIRTUAL; virtual void f29() DECOMP_LOCAL_VIRTUAL; virtual void f30() DECOMP_LOCAL_VIRTUAL; virtual void f31() DECOMP_LOCAL_VIRTUAL;
        virtual void f32() DECOMP_LOCAL_VIRTUAL; virtual void f33() DECOMP_LOCAL_VIRTUAL; virtual void f34() DECOMP_LOCAL_VIRTUAL; virtual void f35() DECOMP_LOCAL_VIRTUAL;
        virtual void initialize(void*) DECOMP_LOCAL_VIRTUAL;
    };
    *(void* volatile*)((char*)self) = __vt__9CCreature;
    *(void* volatile*)((char*)self + 4) = __vt__9CCreature + 0x0C;
    (*(volatile int*)&AsCCreature(self)->flags) = 0;
    Vec3_Zero(((char*)&AsCCreature(self)->movementBoundsOrigin[0]));
    Vec3_Zero(((char*)&AsCCreature(self)->movementBoundsMax[0]));
    self->puppet = 0;
    self->animationFrame = kFloat_0_Creature;
    self->animationRange = 0;
    self->heading = kFloat_0_Creature;
    self->history = 0;
    self->ballSpawnOffsetDistance = kFloat_0_Creature;
    self->opacityKeyframeCount = 0;
    self->opacityKeyframePrevious = ((char*)&AsCCreature(self)->opacityKeyframeNext);
    self->opacityKeyframeNext = ((char*)&AsCCreature(self)->opacityKeyframeNext);
    self->targetPoints = 0;
    AsCCreature(self)->targetPointCount = 0;
    AsCCreature(self)->targetPointCapacity = 0;
    AsCCreature(self)->pathPoints = 0;
    AsCCreature(self)->pathPointCount = 0;
    AsCCreature(self)->pathPointCapacity = 0;
    self->vtable = __vt__9ntPenguin;
    self->secondaryVtable = __vt__9ntPenguin + 0x0C;
    try {
    ((VirtualObject*)self)->initialize(arg);
    ((FlightTargetCursorView*)self)->targetCursor = 1;
    self->movementSpeed = kFloat_2_857143_Creature;
    self->maxBankAngle = kFloat_0_7853982_Creature;
    self->turnRate = kFloat_2_0943952_Creature;
    ((FlightWaypointStorage*)self)->verticalSpeed = kFloat_0_Creature;
    self->bankRate = kHalfPi_Creature;
    ((FlightWaypointStorage*)self)->bankPhase = 2;
    return self;
    } catch (...) {
        CCreature_dtor(self, -1);
        throw;
    }
    }
    };
    return ((ConstructorBody*)self)->initialize(arg);
}

// Advance UFO animation events, wrap the clip, and run its movement update.
void CUfo_UpdateAnimationAndMovement(Obj_8019E368* self, float dt) {
    if (self->puppet == 0) return;
    if (AsCCreature(self)->animationActive != 0) {
    self->animationFrame = self->animationFrame + dt;
    if (self->animationFrame < 0.0f)
        return;
    AnimationEventParser_ProcessFrameRange((char*)self + 4, self->puppet, self->animationFrame - dt, self->animationFrame, self->animationRange, -1, 0);
    void* cur = (void*)self->animationRange;
    if (self->animationFrame > ((CreatureAnimationRange*)cur)->endTime) {
        self->animationFrame = self->animationFrame - ((CreatureAnimationRange*)cur)->endTime;
        int p = self->puppet;
        char* name = npcResourceStrings + kCreatureStrings_defaultAnimation;
        if (p != 0 && (cur == 0 || CString_Compare((void*)((CreatureAnimationRange*)cur)->name, name))) {
            void* found = WPuppet_FindAnimationRangeByName((void*)p, name);
            if (found != 0) {
                self->animationRange = (int)found;
                float zero = kFloat_0_Creature;
                float low = *(float*)found;
                float f = low + zero;
                float hi = ((CreatureAnimationRange*)found)->endTime;
                hi = (f > hi) ? hi : f;
                self->animationFrame = hi;
            } else {
                self->animationFrame = kFloat_0_Creature;
            }
        }
    }
    ((void (**)(void*, float))self->vtable)[0x90 / 4](self, dt);
    }
}
// Advance penguin animation events, wrap the clip, and run its movement update.
void ntPenguin_UpdateAnimationAndMovement(Obj_8019E368* self, float dt) {
    struct VirtualObject {
        virtual void f00() DECOMP_LOCAL_VIRTUAL; virtual void f01() DECOMP_LOCAL_VIRTUAL; virtual void f02() DECOMP_LOCAL_VIRTUAL; virtual void f03() DECOMP_LOCAL_VIRTUAL;
        virtual void f04() DECOMP_LOCAL_VIRTUAL; virtual void f05() DECOMP_LOCAL_VIRTUAL; virtual void f06() DECOMP_LOCAL_VIRTUAL; virtual void f07() DECOMP_LOCAL_VIRTUAL;
        virtual void f08() DECOMP_LOCAL_VIRTUAL; virtual void f09() DECOMP_LOCAL_VIRTUAL; virtual void f10() DECOMP_LOCAL_VIRTUAL; virtual void f11() DECOMP_LOCAL_VIRTUAL;
        virtual void f12() DECOMP_LOCAL_VIRTUAL; virtual void f13() DECOMP_LOCAL_VIRTUAL; virtual void f14() DECOMP_LOCAL_VIRTUAL; virtual void f15() DECOMP_LOCAL_VIRTUAL;
        virtual void f16() DECOMP_LOCAL_VIRTUAL; virtual void f17() DECOMP_LOCAL_VIRTUAL; virtual void f18() DECOMP_LOCAL_VIRTUAL; virtual void f19() DECOMP_LOCAL_VIRTUAL;
        virtual void f20() DECOMP_LOCAL_VIRTUAL; virtual void f21() DECOMP_LOCAL_VIRTUAL; virtual void f22() DECOMP_LOCAL_VIRTUAL; virtual void f23() DECOMP_LOCAL_VIRTUAL;
        virtual void unusedSlot24() DECOMP_LOCAL_VIRTUAL; virtual void f25() DECOMP_LOCAL_VIRTUAL; virtual void f26() DECOMP_LOCAL_VIRTUAL; virtual void f27() DECOMP_LOCAL_VIRTUAL;
        virtual void unusedSlot28() DECOMP_LOCAL_VIRTUAL; virtual void f29() DECOMP_LOCAL_VIRTUAL; virtual void f30() DECOMP_LOCAL_VIRTUAL; virtual void f31() DECOMP_LOCAL_VIRTUAL;
        virtual void f32() DECOMP_LOCAL_VIRTUAL; virtual void f33() DECOMP_LOCAL_VIRTUAL;
        virtual void update(float) DECOMP_LOCAL_VIRTUAL;
    };
    int state = self->puppet;
    if (state == 0) return;
    if (AsCCreature(self)->animationActive == 0) {
        return;
    }
    self->animationFrame = self->animationFrame + dt;
    if (self->animationFrame < 0.0f)
        return;
    AnimationEventParser_ProcessFrameRange((char*)self + 4, state, self->animationFrame - dt, self->animationFrame, self->animationRange, -1, 0);
    void* cur = (void*)self->animationRange;
    if (self->animationFrame > ((CreatureAnimationRange*)cur)->endTime) {
        self->animationFrame = self->animationFrame - ((CreatureAnimationRange*)cur)->endTime;
        int p = self->puppet;
        char* name = npcResourceStrings + kCreatureStrings_defaultAnimation;
        if (p != 0 && (cur == 0 || CString_Compare((void*)((CreatureAnimationRange*)cur)->name, name))) {
            void* found = WPuppet_FindAnimationRangeByName((void*)p, name);
            if (found != 0) {
                self->animationRange = (int)found;
                float zero = kFloat_0_Creature;
                float low = *(float*)found;
                float f = low + zero;
                float hi = ((CreatureAnimationRange*)found)->endTime;
                self->animationFrame = f > hi ? hi : f;
            } else {
                self->animationFrame = kFloat_0_Creature;
            }
        }
    }
    ((VirtualObject*)self)->update(dt);
}

// Follow or record target waypoints while turning and banking toward the next target.
void ntPenguin_MoveAlongPath(ns1::Obj8019* self, float dt) {
    using ns1::PathAllocator8019;
    using ns1::PathPoint8019;
    using ns1::PathGrow8019;
    V3 current, target, next, prev, prev2, next2, movement, velocity;
    char angle2[0x30], angle[0x30], pushResult[0x30], pushAngle[0x30];
    // Phase 1: horizontal distance versus speed * max(dt, 0.1 caller-time units).
    __mi__FRC7WVectorRC7WVector(&current, &self->targetX, &self->positionX);
    float step = kFloat_0_1_Creature;
    current.y = kFloat_0_Creature;
    step = dt < step ? step : dt;
    float horizontalDistance = Magnitude__7WVectorCFv(&current);
    // Phase 2: choose/jitter the next target or replay a recorded world-space waypoint.
    if (horizontalDistance < self->movementSpeed * step) {
        unsigned int num = self->pathPointCount;
        unsigned char waypoint = (AsCCreature(self)->pathCursor);
        if (waypoint >= num || AsCCreature(self)->recordPath) {
            if (self->targetCursor == (unsigned)self->targetPointCount - 1) self->targetCursor = 0;
            unsigned int index = self->targetCursor;
            self->targetCursor = ((unsigned char)index + 1) % (unsigned)self->targetPointCount;
            __as__7WVectorFRC7WVector(&self->targetX, (char*)self->targetPoints + self->targetCursor * 12);
            Matrix_SetAxisRotation(angle, Random_RangeFloat(-1, kFloat_neg3_1415927_Creature, kFloat_3_1415927_805DF2E4), 1);
            Matrix_SetAxisRotation(angle2, Random_RangeFloat(-1, kFloat_neg1_5707964_Creature, kHalfPi_Creature), 0);
            __ml__FfRC7WVector(&prev, angle2 + 0x18, self->ballSpawnOffsetDistance);
            Matrix_TransformPoint(&next, &prev, angle);
            __apl__7WVectorFRC7WVector(&self->targetX, &next);
            __mi__FRC7WVectorRC7WVector(&prev2, &self->targetX, &self->positionX);
            __as__7WVectorFRC7WVector(&current, &prev2);
            current.y = kFloat_0_Creature;
            __mi__FRC7WVectorRC7WVector(&target, (char*)self->targetPoints + self->targetCursor * 12,
                        (char*)self->targetPoints + (unsigned char)index * 12);
            target.y = kFloat_0_Creature;
            if (AsCCreature(self)->recordPath) {
                unsigned oldSize = *(volatile unsigned int*)&self->pathPointCount;
                unsigned oldLimit = *(volatile unsigned int*)&self->pathPointCapacity;
                PathAllocator8019 appendAllocator;
                if (oldSize < oldLimit) {
                    PathPoint8019* slot = (PathPoint8019*)(self->pathPoints + oldSize * 12);
                    ns1::pathConstruct8019(*(PathPoint8019*)(&self->targetX), slot);
                    ++self->pathPointCount;
                } else {
                    PathAllocator8019 growAllocator;
                    if (ns1::pathMaxSize8019() - oldLimit < 1) ns1::pathThrowLength8019();
                    unsigned neededCount = self->pathPointCount;
                    unsigned neededLimit = self->pathPointCapacity;
                    PathGrow8019 tmp(&self->pathPointCapacity);
                    unsigned need = neededCount + 1 - neededLimit;
                    unsigned cap = self->pathPointCapacity;
                    if (need > 0x15555555u - cap) PATH_LOCAL_LENGTH_ERROR_8019();
                    unsigned halfIncrement, smallIncrement;
                    if (cap < 0x071C71C7u) {
                        smallIncrement = (cap + 1) * 3 / 5;
                        cap += mx(smallIncrement, need);
                    } else if (cap < 0x0E38E38Eu) {
                        halfIncrement = (cap + 1) >> 1;
                        cap += mx(halfIncrement, need);
                    } else {
                        cap = 0x15555555u;
                    }
                    PathAllocator8019 allocateAllocator;
                    if (cap > 0x15555555u) PATH_LOCAL_LENGTH_ERROR_8019();
                    PathPoint8019* allocated = (PathPoint8019*)appMallocDummy__FUl(cap * 12);
                    unsigned oldCount = self->pathPointCount;
                    tmp.data = allocated;
                    tmp.capacity = cap;
                    PathPoint8019* dst = allocated + oldCount + tmp.size;
                    tmp.remaining = oldCount;
                    PathAllocator8019 insertAllocator;
                    for (unsigned n = 1; n != 0; --n, ++dst) {
                        new(dst) PathPoint8019(*(PathPoint8019*)(&self->targetX));
                        ++tmp.size;
                    }
                    PathAllocator8019 copyAllocator;
                    PathPoint8019* src;
                    PathPoint8019* oldBegin;
                    oldBegin = (PathPoint8019*)self->pathPoints;
                    src = oldBegin + self->pathPointCount;
                    dst = tmp.data + tmp.remaining;
                    while (src > oldBegin) {
                        --dst;
                        --src;
                        new(dst) PathPoint8019(*src);
                        --tmp.remaining;
                        ++tmp.size;
                    }
                    self->pathPointCount = 0;
                    PathAllocator8019 clearAllocator;
                    PathPoint8019* oldBuf = (PathPoint8019*)self->pathPoints;
                    PathPoint8019* newBuf = tmp.data;
                    tmp.data = oldBuf;
                    unsigned oldCap = self->pathPointCapacity;
                    self->pathPointCapacity = tmp.capacity;
                    tmp.capacity = oldCap;
                    self->pathPoints = (int)newBuf;
                    self->pathPointCount = tmp.size;
                    tmp.size = 0;
                }
            }
        } else {
            unsigned char dc = waypoint;
            if (dc < num) {
                __as__7WVectorFRC7WVector(&self->targetX, (char*)self->pathPoints + dc * 12);
                ++(AsCCreature(self)->pathCursor);
            } else {
                (AsCCreature(self)->pathCursor) = 0;
            }
        }
    }
    // Phase 3: steer in XZ, bank if applicable, then integrate world position.
    float ang = Vector_SignedAngleXZ(&current, &self->forwardX);
    float turn = ang > 0.0f ? ang : -ang;
    if (turn > kFloat_0_017453292_Creature) {
        float low = -self->turnRate * dt;
        float high = self->turnRate * dt;
        float clamped = ang < low ? low : (ang > high ? high : ang);
        Matrix_SetAxisRotation(pushAngle, clamped, 1);
        Matrix_ConcatAffine(pushResult, pushAngle, self->basisXY);
        Matrix_CopyAffine(self->basisXY, pushResult);
    }
    __ml__FfRC7WVector(&velocity, &self->forwardX, self->movementSpeed);
    __ml__FRC7WVectorf(&movement, &velocity, dt);
    __apl__7WVectorFRC7WVector(&self->positionX, &movement);
    self->positionY += self->verticalSpeed * dt;
    V3 pos = *(V3*)(&self->positionX);
    V3 res;
    ((void (*)(void*, void*, void*, bool, int*, bool))GetGroundPoint__9CQuadTreeF7WVectorbPib)(&res, CQuadTree_GetInstance(), &pos, true, 0, false);
    __as__7WVectorFRC7WVector(&self->positionX, &res);
}

// Follow or record target waypoints while applying the UFO turn and bank state machine.
#pragma push
#pragma always_inline on
// Advance the UFO roll state in radians using caller time, retaining the original multiply/add sequence.
static inline void IntegrateUfoBankAngle(ns1::Obj8019*& self, float& dt, float& ang) {
    switch (self->bankPhase) {
    case kBankReturnToLevel: {
        float speed = self->bankAngle;
        float mag = speed > 0.0f ? speed : -speed;
        float acc = self->bankRate;
        if (mag > acc * dt) {
            if (self->bankAngle > 0.0f) acc = -acc;
            self->bankAngle = self->bankAngle + dt * acc;
        } else {
            self->bankPhase = kBankLevel;
            self->bankAngle = 0.0f;
        }
        break;
    }
    case kBankIntoTurn: {
        float speed = self->bankAngle;
        float mag = speed > 0.0f ? speed : -speed;
        if (mag < self->maxBankAngle) {
            float delta = ang > 0.0f ? -self->bankRate : self->bankRate;
            self->bankAngle = self->bankAngle + dt * delta;
        } else {
            self->bankPhase = kBankReturnToLevel;
        }
        break;
    }
    case kBankLevel:
    default:
        break;
    }
}
#pragma pop

// Select or replay waypoints, steer/bank the UFO, then integrate its world position.
void CUfo_FlyAlongWaypoints(ns1::Obj8019* self, float dt) {
    using ns1::PathAllocator8019;
    using ns1::PathPoint8019;
    using ns1::PathGrow8019;
    V3 current, target, next, prev, prev2, next2, movement, velocity;
    char angle2[0x30], angle[0x30], pushResult[0x30], pushAngle[0x30];
    // Phase 1: horizontal distance versus speed * max(dt, 0.1 caller-time units).
    __mi__FRC7WVectorRC7WVector(&current, &self->targetX, &self->positionX);
    float step = kFloat_0_1_Creature;
    current.y = kFloat_0_Creature;
    step = dt < step ? step : dt;
    float horizontalDistance = Magnitude__7WVectorCFv(&current);
    // Phase 2: choose/jitter the next target or replay a recorded world-space waypoint.
    if (horizontalDistance < self->movementSpeed * step) {
        unsigned int num = self->pathPointCount;
        unsigned char waypoint = AsCCreature(self)->pathCursor;
        if (waypoint >= num || AsCCreature(self)->recordPath) {
            unsigned int count = self->targetPointCount;
            unsigned int index = self->targetCursor;
            if (index == count - 1) {
                self->animationActive = 0;
                return;
            }
            self->targetCursor = ((unsigned char)index + 1) % count;
            __as__7WVectorFRC7WVector(&self->targetX, (char*)self->targetPoints + self->targetCursor * 12);
            Matrix_SetAxisRotation(angle, Random_RangeFloat(-1, kFloat_neg3_1415927_Creature, kFloat_3_1415927_805DF2E4), 1);
            Matrix_SetAxisRotation(angle2, Random_RangeFloat(-1, kFloat_neg1_5707964_Creature, kHalfPi_Creature), 0);
            __ml__FfRC7WVector(&prev, angle2 + 0x18, self->ballSpawnOffsetDistance);
            Matrix_TransformPoint(&next, &prev, angle);
            __apl__7WVectorFRC7WVector(&self->targetX, &next);
            __mi__FRC7WVectorRC7WVector(&prev2, &self->targetX, &self->positionX);
            __as__7WVectorFRC7WVector(&current, &prev2);
            current.y = kFloat_0_Creature;
            __mi__FRC7WVectorRC7WVector(&target, (char*)self->targetPoints + self->targetCursor * 12,
                        (char*)self->targetPoints + (unsigned char)index * 12);
            target.y = kFloat_0_Creature;
            float lenCur = Magnitude__7WVectorCFv(&current);
            self->movementSpeed = kFloat_120_Creature * lenCur / Magnitude__7WVectorCFv(&target);
            self->verticalSpeed = self->movementSpeed * (self->targetY - self->positionY) / lenCur;
            // Use the selected segment's heading to choose the bank amplitude.
            float ang = Vector_SignedAngleXZ(&current, &self->forwardX);
            float mag = ang > 0.0f ? ang : -ang;
            self->maxBankAngle = mag > kHalfPi_Creature ? kHalfPi_Creature : mag;
            self->bankPhase = kBankIntoTurn;
            if (AsCCreature(self)->recordPath) {
                unsigned oldSize = *(volatile unsigned int*)&self->pathPointCount;
                unsigned oldLimit = *(volatile unsigned int*)&self->pathPointCapacity;
                PathAllocator8019 appendAllocator;
                if (oldSize < oldLimit) {
                    PathPoint8019* slot = (PathPoint8019*)(self->pathPoints + oldSize * 12);
                    ns1::pathConstruct8019(*(PathPoint8019*)&self->targetX, slot);
                    ++self->pathPointCount;
                } else {
                    PathAllocator8019 growAllocator;
                    if (ns1::pathMaxSize8019() - oldLimit < 1) ns1::pathThrowLength8019();
                    unsigned neededCount = self->pathPointCount;
                    unsigned neededLimit = self->pathPointCapacity;
                    PathGrow8019 tmp(&self->pathPointCapacity);
                    unsigned need = neededCount + 1 - neededLimit;
                    unsigned cap = self->pathPointCapacity;
                    if (need > 0x15555555u - cap) PATH_LOCAL_LENGTH_ERROR_8019();
                    unsigned halfIncrement, smallIncrement;
                    if (cap < 0x071C71C7u) {
                        smallIncrement = (cap + 1) * 3 / 5;
                        cap += mx(smallIncrement, need);
                    } else if (cap < 0x0E38E38Eu) {
                        halfIncrement = (cap + 1) >> 1;
                        cap += mx(halfIncrement, need);
                    } else {
                        cap = 0x15555555u;
                    }
                    PathAllocator8019 allocateAllocator;
                    if (cap > 0x15555555u) PATH_LOCAL_LENGTH_ERROR_8019();
                    PathPoint8019* allocated = (PathPoint8019*)appMallocDummy__FUl(cap * 12);
                    unsigned oldCount = self->pathPointCount;
                    tmp.data = allocated;
                    tmp.capacity = cap;
                    PathPoint8019* dst = allocated + oldCount + tmp.size;
                    tmp.remaining = oldCount;
                    PathAllocator8019 insertAllocator;
                    for (unsigned n = 1; n != 0; --n, ++dst) {
                        new(dst) PathPoint8019(*(PathPoint8019*)&self->targetX);
                        ++tmp.size;
                    }
                    PathAllocator8019 copyAllocator;
                    PathPoint8019* src;
                    PathPoint8019* oldBegin;
                    oldBegin = (PathPoint8019*)self->pathPoints;
                    src = oldBegin + self->pathPointCount;
                    dst = tmp.data + tmp.remaining;
                    while (src > oldBegin) {
                        --dst;
                        --src;
                        new(dst) PathPoint8019(*src);
                        --tmp.remaining;
                        ++tmp.size;
                    }
                    self->pathPointCount = 0;
                    PathAllocator8019 clearAllocator;
                    PathPoint8019* oldBuf = (PathPoint8019*)self->pathPoints;
                    PathPoint8019* newBuf = tmp.data;
                    tmp.data = oldBuf;
                    unsigned oldCap = self->pathPointCapacity;
                    self->pathPointCapacity = tmp.capacity;
                    tmp.capacity = oldCap;
                    self->pathPoints = (int)newBuf;
                    self->pathPointCount = tmp.size;
                    tmp.size = 0;
                }
            }
        } else {
            unsigned char dc = waypoint;
            if (dc < num) {
                __as__7WVectorFRC7WVector(&self->targetX, (char*)self->pathPoints + dc * 12);
                ++(AsCCreature(self)->pathCursor);
            } else {
                self->animationActive = 0;
            }
        }
    }
    // Phase 3: steer in XZ, bank if applicable, then integrate world position.
    float ang = Vector_SignedAngleXZ(&current, &self->forwardX);
    float turn = ang > 0.0f ? ang : -ang;
    if (turn > kFloat_0_017453292_Creature) {
        float low = -self->turnRate * dt;
        float high = self->turnRate * dt;
        float clamped = ang < low ? low : (ang > high ? high : ang);
        Matrix_SetAxisRotation(pushAngle, clamped, 1);
        Matrix_ConcatAffine(pushResult, pushAngle, self->basisXY);
        Matrix_CopyAffine(self->basisXY, pushResult);
    }
    IntegrateUfoBankAngle(self, dt, ang);
    __ml__FfRC7WVector(&velocity, &self->forwardX, self->movementSpeed);
    __ml__FRC7WVectorf(&movement, &velocity, dt);
    __apl__7WVectorFRC7WVector(&self->positionX, &movement);
    self->positionY += self->verticalSpeed * dt;
}

// Release CUfo resources and optionally delete the object.
void* CUfo_Dtor(void* ptr, int shouldDelete) {
    if (ptr) {
        CCreature_dtor(ptr, 0);
        if (shouldDelete > 0)
            __dl__FPv(ptr);
    }
    return ptr;
}

// Release ntPenguin resources and optionally delete the object.
void* ntPenguin_Dtor(void* ptr, int shouldDelete) {
    if (ptr) {
        CCreature_dtor(ptr, 0);
        if (shouldDelete > 0)
            __dl__FPv(ptr);
    }
    return ptr;
}

} // extern "C"

} // namespace ns2

// ---- merged from course_creatures_flocks_and_flyers.cpp ----
namespace ns3 {
// Translation unit 0x8019F9E4-0x801A10FC (8 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" void __throw(const char*, void*, void*) __attribute__((noreturn));
typedef Vec3f FramePosition;

extern "C" {

void CCreature_dtor(void*, int);
void __dl__FPv(void*);
void* CCreature_LoadPuppetResource(void*, void*, int);
void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
void Vec3_Zero(void*);
int CString_Compare(void*, const char*);
void* WPuppet_FindAnimationRangeByName(void*, const char*);
float Random_RangeFloat(int, float, float);
void* __nwa__FUl(unsigned int);
void WMatrix_DefaultConstructNoOp(void*);
float WVectorLen__FRC7WVector(void*);
void __as__7WVectorFRC7WVector(void*, void*);
float Vector_SignedAngleXZ(void*, void*);
void* Normalize__7WVectorFv(void*);
void __ml__FRC7WVectorf(void*, void*, float);
void __apl__7WVectorFRC7WVector(void*, void*);
void Matrix_ConcatAffine(void*, void*, void*);
void Matrix_CopyAffine(void*, void*);
void* CQuadTree_GetInstance(void);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
void __dla__FPv(void*);
void ResourceManager_ReleaseObject(void*, void*, int);
void WPuppet_SetOpacityRecursive(void*, int);
void WView_DrawFacingCircleVec4(void*, void*, int, int, int);
void __ml__FfRC7WVector(void*, void*, float);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void WView_DrawLine(void*, void*, int, void*, int, int);
void WPuppet_SetAnimationTimeFromRanges(void*, int, float, int, int);
void Matrix_SetAxisRotation(void*, float, signed char);
void Matrix_ConcatAffine(void*, void*, void*);
void* Matrix_CopyScaledBasis(void*, void*, float);
void Model_SetMatrixRecursive(void*, void*, void*);
void WPuppet_UpdateBoneWorldMatrices(void*, void*, int, float);
void WPuppet_UpdateBoundsFromBone(void*);
void AnimationEventParser_ProcessFrameRange(void*, void*, float, float, void*, int, int); /* AnimationEventParser_ProcessFrameRange is (self, manager r4, bounds r5, f1, f2, r6, r7): f2 is the new time (0x802A663C reads it) */
extern int g_drawSceneDebugGeometry;
extern void* g_view;
extern void* g_resourceManager;
extern const char s_sceneLighting[];
extern const char __vt__9CCreature[];
extern const char __vt__16ntPenguin_Sailor[];
extern char __vt__14ntPenguin_Duck[];
extern const char npcResourceStrings[];
extern const float kFloat_0_Creature;
extern const float kFloat_32_Creature;
extern const float kFloat_0_5235988_Creature;
extern const float kFloat_2_805DF2B0;
extern const float kFloat_0_7853982_Creature;
extern const float kFloat_48_Creature;
extern const float kFloat_0_7_Creature;
extern const float kFloat_1_Creature;
extern const float kFloat_0_6_Creature;
extern const float kFloat_0_8_805DF2D8;
extern const float kFloat_5_creature;
extern const float kFloat_0_017453292_Creature;
void* __nw__FUl(unsigned int);
void __dt__Q23std12length_errorFv(void*, int);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

struct FrameAllocator {
    volatile unsigned char value;
    char padding[3];
    // Initialize the frame allocator marker used by exception cleanup.
    FrameAllocator() : value(0) {}
};
struct LengthErrorBase {
    void* vtable;
    char* text;
    void* shared;
    // Initialize the logic-error base and allocator marker.
    LengthErrorBase() { FrameAllocator allocator; vtable = __vt__Q23std11logic_error; }
    // Invoke the base exception cleanup without deleting wrapper storage.
    ~LengthErrorBase() { __dt__Q23std9exceptionFv(this, 0); }
};
struct ErrorSharedData {
    void* vtable;
    int refs, owners;
    char* text;
};
struct ErrorAllocation {
    char* data;
    FrameAllocator* allocator;
    // Free the exception-message array if ownership has not transferred.
    ~ErrorAllocation() { if (data) __dla__FPv(data); }
};

// Placement-copy one frame record without changing its storage words.
inline void constructFrame(FramePosition* dest, const FramePosition& source) {
    unsigned int x = ((const unsigned int*)&source)[0];
    unsigned int y = ((const unsigned int*)&source)[1];
    ((unsigned int*)dest)[0] = x;
    ((unsigned int*)dest)[1] = y;
    ((unsigned int*)dest)[2] = ((const unsigned int*)&source)[2];
}

struct LengthErrorData : LengthErrorBase {
    // Build the guarded shared message buffer for a vector length error.
    LengthErrorData() {
        FrameAllocator allocator;
        const char* message = npcResourceStrings;
        unsigned int length = strlen(message) + 1;
        text = (char*)__nwa__FUl((length << 1) - length);
        {
            ErrorAllocation allocation = {text, &allocator};
            char* storage = text;
            ErrorSharedData* value;
            value = (ErrorSharedData*)__nw__FUl(16);
            if (value) {
                value->refs = 1; value->owners = 1;
                value->vtable = s_sharedCharDeleterVtable; value->text = storage;
            }
            shared = value;
            allocation.data = 0;
        }
        strcpy(text, message);
        vtable = __vt__Q23std12length_error;
    }
};

// Construct one recorded waypoint with the original checked-copy cleanup path.
inline void constructFrameChecked(FramePosition* dest, const FramePosition& source) {
    if (dest) {
        try { constructFrame(dest, source); }
        catch (...) { throw; }
    }
}

struct FrameBuffer {
    FramePosition* data;
    unsigned int size, capacity;
    unsigned int* capacityRef;
    unsigned int original;
    // Initialize an empty temporary frame buffer tied to the owner capacity.
    FrameBuffer(unsigned int* ref) : data(0), size(0), capacity(0), capacityRef(ref), original(0) {}
    // Release the temporary frame allocation after swap or unwind.
    ~FrameBuffer() {
        if (data) {
            FrameAllocator allocator;
            size -= size;
            appFreeUni__FPv(data);
        }
    }
};

#define THROW_FRAME_LENGTH() \
    { LengthErrorData error; \
      __throw(npcResourceStrings + kCreatureStrings_lengthErrorType, &error, (void*)__dt__Q23std12length_errorFv); }

struct InitObject {
    void* vtable;
    void* vtableEnd;
    int flags;
    char pad00c[0x0c];
    char pad018[0x0c];
    void* puppet;
    char pad028[0x30];
    float animationFrame;
    char pad05c[4];
    void* animationRange;
    char pad064[0x10];
    float movementSpeed;
    float f78;
    float maxBankAngle;
    char pad080[4];
    float heading;
    float cellSize;
    char pad08c[0x1c];
    void* history;
    char pad0ac[4];
    float ballSpawnOffsetDistance;
    void* opacityKeyframeCount;
    void* head;
    void* tail;
    int targetPoints;
    int targetPointCount;
    int targetPointCapacity;
    char pad0cc[4];
    int pathPoints;
    int pathPointCount;
    int pathPointCapacity;
    char pad0dc[0x40];
    float modelScale;
    unsigned char enabled;
};

struct AnimationObject {
#define V(n) virtual void slot##n();
    V(0) V(1) V(2) V(3) V(4) V(5) V(6) V(7) V(8) V(9)
    V(10) V(11) V(12) V(13) V(14) V(15) V(16) V(17) V(18) V(19)
    V(20) V(21) V(22) V(23) V(24) V(25) V(26) V(27) V(28) V(29)
    V(30) V(31) V(32) V(33) 
    virtual void slot34(float);
    V(35) V(36) V(37)
#undef V
    virtual int check(void*, float);
    char pad004[4];
    unsigned int flags;
    char pad00c[0x18];
    void* resource;
    float rightScale;
    char pad02c[0xc];
    float upScale;
    char pad03c[0xc];
    float forwardScale;
    char pad04c[0xc];
    float time;
    char pad05c[0x1c];
    float bankAngle;
    char pad07c[0xa0];
    float modelScale;
    char pad120[0x10];
    unsigned char derivedState;
};

struct KeyFrame { KeyFrame* next; KeyFrame* prev; float value; float time; };

// Advance sailor clips, choose random actions, and update opacity or movement as required.
void ntPenguin_Sailor_UpdateAnimationAndMovement(AnimationObject* self, float delta) {
    void* resource = (*(void **)&AsCCreature(self)->puppet);
    if (!resource || !AsCCreature(self)->animationActive) return;
        float time = (self->time + delta);
        self->time = time;
        if (time < 0.0f) {
        } else {
            ((void (*)(void*, void*, float, float, void*, int, int))AnimationEventParser_ProcessFrameRange)((char*)self + 4, resource, time - delta, time, (*(void **)&AsCCreature(self)->animationRange), -1, 0);
            resource = (*(void **)&AsCCreature(self)->animationRange);
            float end;
            float currentTime;
            currentTime = self->time;
            end = ((CreatureAnimationRange*)resource)->endTime;
            if (currentTime > end) {
                if (self->flags & kCreatureOneShotAnimation) {
                    AsCCreature(self)->animationActive = 0;
                    self->time = *(float*)resource;
                } else {
                    self->time = currentTime - (end - *(float*)resource);
                    int tick = rand();
                    int mod = tick % 10;
                    switch (mod) {
                    case 0: {
                        void* owner = (*(void **)&AsCCreature(self)->puppet);
                        const char* name = npcResourceStrings + kCreatureStrings_action1;
                        if (owner && (!(*(void **)&AsCCreature(self)->animationRange) || CString_Compare(*(void**)((char*)(*(void **)&AsCCreature(self)->animationRange) + 0x18), name))) {
                            void* found = WPuppet_FindAnimationRangeByName(owner, name);
                            if (found) {
                                (*(void **)&AsCCreature(self)->animationRange) = found;
                                float v = *(float*)found + kFloat_0_Creature;
                                float hi = ((CreatureAnimationRange*)found)->endTime;
                                hi = v > hi ? hi : v;
                                self->time = hi;
                            } else self->time = 0.0f;
                        }
                        ((SailorMovementView*)self)->movementEnabled = 0;
                        break;
                    }
                    case 1: {
                        void* owner = (*(void **)&AsCCreature(self)->puppet);
                        const char* name = npcResourceStrings + kCreatureStrings_defaultAnimation;
                        if (owner && (!(*(void **)&AsCCreature(self)->animationRange) || CString_Compare(*(void**)((char*)(*(void **)&AsCCreature(self)->animationRange) + 0x18), name))) {
                            void* found = WPuppet_FindAnimationRangeByName(owner, name);
                            if (found) {
                                (*(void **)&AsCCreature(self)->animationRange) = found;
                                float v = *(float*)found + kFloat_0_Creature;
                                float hi = ((CreatureAnimationRange*)found)->endTime;
                                hi = v > hi ? hi : v;
                                self->time = hi;
                            } else self->time = 0.0f;
                        }
                        ((SailorMovementView*)self)->movementEnabled = 0;
                        break;
                    }
                    default: {
                        void* owner = (*(void **)&AsCCreature(self)->puppet);
                        const char* name = npcResourceStrings + kCreatureStrings_action0;
                        if (owner && (!(*(void **)&AsCCreature(self)->animationRange) || CString_Compare(*(void**)((char*)(*(void **)&AsCCreature(self)->animationRange) + 0x18), name))) {
                            void* found = WPuppet_FindAnimationRangeByName(owner, name);
                            if (found) {
                                (*(void **)&AsCCreature(self)->animationRange) = found;
                                float v = *(float*)found + kFloat_0_Creature;
                                float hi = ((CreatureAnimationRange*)found)->endTime;
                                hi = v > hi ? hi : v;
                                self->time = hi;
                            } else self->time = 0.0f;
                        }
                        ((SailorMovementView*)self)->movementEnabled = 1;
                    }
                    }
                }
            }
            if (self->flags & kCreatureOneShotAnimation) {
                KeyFrame* sentinel = (KeyFrame*)(((char*)&AsCCreature(self)->opacityKeyframeNext));
                KeyFrame* node = (*(KeyFrame **)&AsCCreature(self)->opacityKeyframePrevious);
                while (node != sentinel) {
                    KeyFrame* previous = node;
                    node = node->prev;
                    if (node == sentinel) {
                        AsCCreature(self)->opacity = (unsigned char)(int)previous->value;
                        continue;
                    }
                    if (self->time > previous->time) { if (self->time < node->time) {
                        float t = (self->time - previous->time) / (node->time - previous->time);
                        AsCCreature(self)->opacity = (unsigned char)(int)((1.0f-t)*previous->value + t*node->value);
                        break;
                    } }
                }
            } else if (((SailorMovementView*)self)->movementEnabled) {
                self->slot34(delta);
            }
        }
}

struct SailorResource {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void name(const char*, int, int);
    virtual void draw(void*, int, int, float, int, int);
};

#pragma opt_common_subs off
// Cull and draw the sailor puppet with animation, opacity and banked world transform.
void ntPenguin_Sailor_Draw(AnimationObject* self) {
    void* resource = (*(void **)&AsCCreature(self)->puppet);
    if (!resource || !AsCCreature(self)->animationActive || self->time < kFloat_0_Creature)
        return;
    if (self->flags & kCreatureOneShotAnimation)
        WPuppet_SetOpacityRecursive(resource, AsCCreature(self)->opacity);
    if (g_drawSceneDebugGeometry) {
        WView_DrawFacingCircleVec4(g_view, ((char*)&AsWPuppet(self->resource)->boundsCenter), 0xFF00FF00, 0x14, 0);
        if (!(self->flags & kCreatureOneShotAnimation)) {

            WVector tmp = kFloat_5_creature * (*(WVector*)&AsCCreature(self)->transform[6]);
            WVector tmp2 = (*(WVector*)&AsCCreature(self)->transform[9]) - tmp;
            WView_DrawLine(g_view, ((char*)&AsCCreature(self)->transform[9]), -256, &tmp2, -256, 0);
            WView_DrawLine(g_view, ((char*)&AsCCreature(self)->transform[9]), 0xFFFF0000, ((char*)&AsCCreature(self)->targetPosition[0]), 0xFFFF0000, 0);
        }
    }
    WPuppet_SetAnimationTimeFromRanges(self->resource, 0, self->time, 0, 0);
    if (self->check(((char*)&AsCCreature(self)->transform[9]), kFloat_0_8_805DF2D8))
        return;
    if (self->rightScale > kFloat_1_Creature) self->rightScale = kFloat_1_Creature;
    if (self->upScale > kFloat_1_Creature) self->upScale = kFloat_1_Creature;
    if (self->forwardScale > kFloat_1_Creature) self->forwardScale = kFloat_1_Creature;
    float delta = self->bankAngle > kFloat_0_Creature ? self->bankAngle : -self->bankAngle;
    float rotated[12], transformed[12], result[12], plain[12];
    void* matrix;
    if (delta > kFloat_0_Creature) {
        Matrix_SetAxisRotation(rotated, self->bankAngle, 2);
        Matrix_ConcatAffine(transformed, rotated, ((char*)&AsCCreature(self)->transform[0]));
        Matrix_CopyScaledBasis(result, transformed, self->modelScale);
        matrix = result;
    } else {
        Matrix_CopyScaledBasis(plain, ((char*)&AsCCreature(self)->transform[0]), self->modelScale);
        matrix = plain;
    }
    Model_SetMatrixRecursive(self->resource, self->resource, matrix);
    WPuppet_UpdateBoneWorldMatrices(self->resource, g_view, 0, kFloat_1_Creature);
    WPuppet_UpdateBoundsFromBone(self->resource);
    ((SailorResource*)self->resource)->name(s_sceneLighting, 0, 0);
    ((SailorResource*)self->resource)->draw(g_view, 1, 1, kFloat_1_Creature, 0, 0);
}

#pragma opt_common_subs reset
// Initialize ntPenguin_Sailor state and owned resources.
InitObject* ntPenguin_Sailor_Ctor(InitObject* self, void* arg) {
    const char* name;
    void* resource;
    struct VirtualObject {
#define V(n) virtual void slot##n() DECOMP_LOCAL_VIRTUAL;
    V(0) V(1) V(2) V(3) V(4) V(5) V(6) V(7) V(8) V(9)
    V(10) V(11) V(12) V(13) V(14) V(15) V(16) V(17) V(18) V(19)
    V(20) V(21) V(22) V(23) V(24) V(25) V(26) V(27) V(28) V(29)
    V(30) V(31) V(32) V(33) V(34) V(35)
#undef V
    virtual void initialize(void*) DECOMP_LOCAL_VIRTUAL;
    };
    *(void* volatile*)self = (void*)__vt__9CCreature;
    *(void* volatile*)((char*)self + 4) = (void*)(__vt__9CCreature + 12);
    (*(volatile int*)&AsCCreature(self)->flags) = 0;
    Vec3_Zero(((char*)&AsCCreature(self)->movementBoundsOrigin[0]));
    Vec3_Zero(((char*)&AsCCreature(self)->movementBoundsMax[0]));
    self->puppet = 0;
    self->animationFrame = kFloat_0_Creature;
    self->animationRange = 0;
    self->heading = kFloat_0_Creature;
    self->history = 0;
    self->ballSpawnOffsetDistance = kFloat_0_Creature;
    self->opacityKeyframeCount = 0;
    self->tail = ((char*)&AsCCreature(self)->opacityKeyframeNext);
    self->head = ((char*)&AsCCreature(self)->opacityKeyframeNext);
    self->targetPoints = 0;
    self->targetPointCount = 0;
    self->targetPointCapacity = 0;
    self->pathPoints = 0;
    self->pathPointCount = 0;
    self->pathPointCapacity = 0;
    self->vtable = (void*)__vt__16ntPenguin_Sailor;
    self->vtableEnd = (void*)(__vt__16ntPenguin_Sailor + 12);
    self->movementSpeed = kFloat_2_805DF2B0;
    try {
    ((VirtualObject*)self)->initialize(arg);
    resource = self->puppet;
    self->maxBankAngle = kFloat_0_7853982_Creature;
    self->cellSize = kFloat_48_Creature;
    name = npcResourceStrings + kCreatureStrings_action0;
    if (resource) {
        void* existing = self->animationRange;
        if (existing == 0 || CString_Compare((void*)((CreatureAnimationRange*)existing)->name, name) != 0) {
            void* found = WPuppet_FindAnimationRangeByName(resource, name);
            if (found) {
                self->animationRange = found;
                float v0 = *(float*)found + kFloat_0_Creature;
                float v1 = ((CreatureAnimationRange*)found)->endTime;
                self->animationFrame = v0 > v1 ? v1 : v0;
            } else {
                self->animationFrame = kFloat_0_Creature;
            }
        }
    }
    self->modelScale = Random_RangeFloat(-1, kFloat_0_7_Creature, kFloat_1_Creature);
    self->enabled = 1;
    return self;
    } catch (...) {
        CCreature_dtor(self, -1);
        throw;
    }
}

// Release ntPenguin_Sailor resources and optionally delete the object.
void* ntPenguin_Sailor_Dtor(void* ptr, int flag) {
    if (ptr != 0) {
        CCreature_dtor(ptr, 0);
        if (flag > 0)
            __dl__FPv(ptr);
    }
    return ptr;
}

struct ItemNode { ItemNode* next; ItemNode* prev; void* value; };
struct ItemList { ItemNode* head; };
struct ObjectList {
    char pad0[0x11c];
    int size;
    int count;
    ItemList list;
    char pad12c[0x0c];
    void* finalSource;
};

struct RenderNode { RenderNode* next; RenderNode* prev; void* data; };

struct UpdateObject {
    char pad0[0x0c]; float x, y, z;
    char pad18[0x34];
    float pos[3];
    char pad58[0x0c];
    float targetPos[3];
    char pad70[0x1c];
    int width, height, depth;
    float spacingX, spacingY, spacingZ;
    int countC;
    int* order;
    char padAC[0x24];
    void* frameData;
    unsigned int frames, capacity;
    unsigned char frameIndex;
    char padDD[0x38];
};

#pragma opt_loop_transform off
#pragma opt_strength_reduction off
#pragma opt_common_subs off
#pragma opt_propagation off
// Choose nearby grid cells or replay recorded targets, then move in space or along the ground.
void ntPenguin_Sailor_UpdateGridTargetAndMove(UpdateObject* self, float delta) {
    float rotatedTransform[12], yawRotation[12];
    int neighborStep[3];
    float positionMinusTarget[3];
    float spatialDisplacement[3], spatialVelocity[3], towardSpatialTarget[3];
    float towardGroundTarget[3];
    float groundDisplacement[3], groundVelocity[3];
    float groundPosition[3];
    FramePosition input;
    // Phase 1: within one world unit of target, choose a neighboring grid cell or replay XYZ.
    // Flattening is x + width*(z + depth*y); only five candidates are attempted,
    // so the last candidate may repeat a recent cell. Y-count 1 constrains to ground.
    __mi__FRC7WVectorRC7WVector(positionMinusTarget, ((char*)&AsCCreature(self)->transform[9]), ((char*)&AsCCreature(self)->targetPosition[0]));
    float distance = WVectorLen__FRC7WVector(positionMinusTarget);
    if (distance < kFloat_1_Creature) {
        unsigned int idx = AsCCreature(self)->pathCursor;
        if (idx >= self->frames || AsCCreature(self)->recordPath) {
            unsigned int capacity;
            int y, z, x, target, found, historyCount;
            int tries = 0;
            do {
                ++tries;
                int* coordinate = neighborStep;
                int i=0;
                while (i<kGridAxisCount) {
                    *coordinate = rand() % kNeighborStepChoices - 1;
                    ++coordinate;
                    ++i;
                }
                int* order = ((CreatureGridPathView*)self)->recentCells;
                int depth = self->depth;
                int width = self->width;
                int maxX = width-1;
                int stride = width * depth;
                int index = order[0];
                int cy = index / stride;
                int cx = index % width;
                int cz = index % stride / width;
                x = cx + neighborStep[0];
                x = x < 0 ? 0 : x > maxX ? maxX : x;
                y = cy + neighborStep[1];
                int maxY = self->height-1;
                y = y < 0 ? 0 : y > maxY ? maxY : y;
                int maxZ = depth-1;
                z = cz + neighborStep[2];
                z = z < 0 ? 0 : z > maxZ ? maxZ : z;
                historyCount = ((CreatureGridPathView*)self)->recentCellCount;
                found = 0;
                i=0;
                int offset = 0;
                target = width * (z + y * depth) + x;
                while (i<historyCount) {
                    if (target == *(int*)((char*)order+offset)) { found=1; break; }
                    offset += 4;
                    ++i;
                }
            } while (found && tries < kNeighborAttemptLimit);
            // Phase 2: shift recent cells one slot right, from the end toward index 1,
            // then put the selected cell at index 0. The expanded preflight below
            // guards the eight-store batch against signed count/loop overflow;
            // failed preflight falls through to the scalar backward loop. Volatile
            // count reloads and each store are preserved (not a circular buffer).
            int i=historyCount-1;
            if (i >= 1) {
                if (i > kHistoryShiftBatchWords) {
                    int enabled = 0, safe = 0, positive = 0;
                    int count = *(volatile int*)&((CreatureGridPathView*)self)->recentCellCount - 1;
                    if (count >= 1 && count >= (int)0x80000001) positive = 1;
                    if (positive) {
                        int count = *(volatile int*)&((CreatureGridPathView*)self)->recentCellCount;
                        int sameSign = 1;
                        int sign = (count - 1) & 0x80000000;
                        if ((unsigned int)(sign + 0x80000000) == 0 && sign != (int)((count-2)&0x80000000)) sameSign = 0;
                        if (sameSign) safe = 1;
                    }
                    if (safe) {
                        int count = *(volatile int*)&((CreatureGridPathView*)self)->recentCellCount;
                        int noOverflow = 1;
                        if (!((count-2)&0x80000000) && ((count-1)&0x80000000)) noOverflow = 0;
                        if (noOverflow) enabled = 1;
                    }
                    if (enabled) {
                        int offset = i * 4;
                        while (i >= kHistoryShiftBatchWords + 1) {
                            i -= kHistoryShiftBatchWords;
                            int* row = (int*)((char*)((CreatureGridPathView*)self)->recentCells + offset);
                            row[0] = row[-1];
                            row = (int*)((char*)((CreatureGridPathView*)self)->recentCells + offset); row[-1] = row[-2];
                            row = (int*)((char*)((CreatureGridPathView*)self)->recentCells + offset); row[-2] = row[-3];
                            row = (int*)((char*)((CreatureGridPathView*)self)->recentCells + offset); row[-3] = row[-4];
                            row = (int*)((char*)((CreatureGridPathView*)self)->recentCells + offset); row[-4] = row[-5];
                            row = (int*)((char*)((CreatureGridPathView*)self)->recentCells + offset); row[-5] = row[-6];
                            row = (int*)((char*)((CreatureGridPathView*)self)->recentCells + offset); row[-6] = row[-7];
                            row = (int*)((char*)((CreatureGridPathView*)self)->recentCells + offset);
                            offset -= kHistoryShiftBatchWords * 4;
                            row[-7] = row[-8];
                        }
                    }
                }
                int offset = i * 4;
                while (i >= 1) {
                    int* row = (int*)((char*)((CreatureGridPathView*)self)->recentCells + offset);
                    --i;
                    offset -= 4;
                    row[0] = row[-1];
                }
            }
            ((CreatureGridPathView*)self)->recentCells[0] = target;
            // Phase 3: jitter inside the chosen world-space cell and optionally record XYZ.
            float a = Random_RangeFloat(-1, kFloat_0_Creature, self->spacingX);
            self->targetPos[0] = (self->spacingX * x + self->x) + a;
            if (self->height > 1) self->targetPos[1] = (self->spacingY * y + self->y) + Random_RangeFloat(-1,kFloat_0_Creature,self->spacingY);
            self->targetPos[2] = (self->spacingZ * z + self->z) + Random_RangeFloat(-1,kFloat_0_Creature,self->spacingZ);
            if (AsCCreature(self)->recordPath) {
                FrameAllocator frameAllocator;
                if (self->frames < self->capacity) {
                    FramePosition* row = (FramePosition*)self->frameData + self->frames;
                    constructFrameChecked(row, *(FramePosition*)self->targetPos);
                    ++self->frames;
                } else {
                    FrameAllocator reserveAllocator;
                    FrameAllocator limitAllocator;
                    if (kMaxWaypointCount - self->capacity < 1) THROW_FRAME_LENGTH();
                    unsigned int* capacityRef = &self->capacity;
                    unsigned int needed = self->frames + 1 - self->capacity;
                    FrameBuffer buffer(capacityRef);
                    capacity = self->capacity;
                    if (needed > kMaxWaypointCount - capacity) THROW_FRAME_LENGTH();
                    if (capacity < kWaypointThreeFifthsGrowthLimit) {
                        unsigned int growth = (capacity + 1) * 3 / 5;
                        const unsigned int& increment = growth < needed ? needed : growth;
                        capacity += increment;
                    } else if (capacity < kWaypointHalfGrowthLimit) {
                        unsigned int growth = (capacity + 1) / 2;
                        const unsigned int& increment = growth < needed ? needed : growth;
                        capacity += increment;
                    } else capacity = kMaxWaypointCount;
                    FrameAllocator allocateAllocator;
                    if (capacity > kMaxWaypointCount) THROW_FRAME_LENGTH();
                    FramePosition* allocated = (FramePosition*)appMallocDummy__FUl(capacity * 12);
                    unsigned int frames = self->frames;
                    buffer.data = allocated;
                    buffer.capacity = capacity;
                    buffer.original = frames;
                    FramePosition* row = allocated + frames + buffer.size;
                    unsigned int count=1;
                    FrameAllocator constructAllocator;
                    while (count != 0) {
                        constructFrameChecked(row, *(FramePosition*)self->targetPos);
                        ++buffer.size;
                        --count;
                        ++row;
                    }
                    FramePosition* begin = (FramePosition*)self->frameData;
                    FramePosition* source = begin + self->frames;
                    FramePosition* dest = buffer.data + buffer.original;
                    FrameAllocator copyAllocator;
                    while (source > begin) {
                        --source; --dest;
                        constructFrameChecked(dest, *source);
                        --buffer.original;
                        ++buffer.size;
                    }
                    unsigned int zero = 0;
                    self->frames = zero;
                    FramePosition* old = (FramePosition*)self->frameData;
                    FramePosition* data = buffer.data;
                    unsigned int size = buffer.size;
                    unsigned int oldCapacity = self->capacity;
                    buffer.data = old;
                    FrameAllocator swapAllocator;
                    self->capacity = buffer.capacity;
                    buffer.capacity = oldCapacity;
                    self->frameData = data;
                    self->frames = size;
                    buffer.size = zero;
                }
            }
        } else {
            unsigned int cur = (unsigned char)idx;
            __as__7WVectorFRC7WVector(self->targetPos, (char*)(*(void **)&AsCCreature(self)->pathPoints) + cur*12);
            ++AsCCreature(self)->pathCursor;
        }
    }
    // Phase 4: normalized 3D pursuit, or rate-limited yaw plus ground projection.
    // delta has the same time base as movementSpeed and turnRate; no unit conversion.
    if (self->height > 1) {
        __mi__FRC7WVectorRC7WVector(towardSpatialTarget, self->targetPos, self->pos);
        void* normalized = Normalize__7WVectorFv(towardSpatialTarget);
        __ml__FfRC7WVector(spatialVelocity, normalized, AsCCreature(self)->movementSpeed);
        __ml__FRC7WVectorf(spatialDisplacement, spatialVelocity, delta);
        __apl__7WVectorFRC7WVector(((char*)&AsCCreature(self)->transform[9]), spatialDisplacement);
    } else {
        __mi__FRC7WVectorRC7WVector(&towardGroundTarget, self->targetPos, self->pos);
        float diff = Vector_SignedAngleXZ(&towardGroundTarget, ((char*)&AsCCreature(self)->transform[6]));
        float magnitude = diff > kFloat_0_Creature ? diff : -diff;
        if (magnitude > kFloat_0_017453292_Creature) {
            float rate = AsCCreature(self)->turnRate;
            float step = rate * delta;
            float low = -rate * delta;
            diff = diff < low ? low : diff > step ? step : diff;
            char rotationFlag = 1;
            Matrix_SetAxisRotation(yawRotation, diff, rotationFlag);
            Matrix_ConcatAffine(rotatedTransform, yawRotation, ((char*)&AsCCreature(self)->transform[0]));
            Matrix_CopyAffine(((char*)&AsCCreature(self)->transform[0]), rotatedTransform);
        }
        __ml__FfRC7WVector(groundVelocity, ((char*)&AsCCreature(self)->transform[6]), AsCCreature(self)->movementSpeed);
        __ml__FRC7WVectorf(groundDisplacement, groundVelocity, delta);
        __apl__7WVectorFRC7WVector(((char*)&AsCCreature(self)->transform[9]), groundDisplacement);
        constructFrame(&input, *(FramePosition*)self->pos);
        void* rotation = CQuadTree_GetInstance();
        int flag = 0;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(groundPosition, rotation, &input, 1, flag, flag);
        __as__7WVectorFRC7WVector(((char*)&AsCCreature(self)->transform[9]), groundPosition);
    }
}
#pragma opt_strength_reduction reset
#pragma opt_loop_transform reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
struct RenderObject {
    char pad0[0x11c];
    int size;
    int count;
    RenderNode* sentinelNext;
    RenderNode* sentinelPrev;
    char pad130[0x0c];
};

// Release ntPenguin_Duck resources and optionally delete the object.
void* ntPenguin_Duck_Dtor(RenderObject* self, int flag) {
    if (!self) return self;
        *(void**)self = (void*)__vt__14ntPenguin_Duck;
        *((void**)self + 1) = (void*)((char*)__vt__14ntPenguin_Duck + kCreatureSecondaryVtableOffset);
        // release array-backed fields
        void* array = AsPenguinDuck(self)->instanceMatrices;
        if (array) {
            if (array)
            __dla__FPv((char*)array - 0x10);
            AsPenguinDuck(self)->instanceMatrices = 0;
        }
        void* second = AsPenguinDuck(self)->instanceScales;
        if (second) {
            __dla__FPv(second);
            AsPenguinDuck(self)->instanceScales = 0;
        }
        RenderNode* node = (*(RenderNode **)&AsPenguinDuck(self)->listPrevious);
        RenderNode* end1 = (RenderNode*)(((char*)&AsPenguinDuck(self)->listNext));
        while (end1 != node) {
            void* manager = g_resourceManager;
            if (manager && node->data) { ResourceManager_ReleaseObject(manager, node->data, 0); node->data = 0; }
            node = node->prev;
        }
        RenderNode* cur = (*(RenderNode **)&AsPenguinDuck(self)->listPrevious);
        RenderNode* end2 = (RenderNode*)(((char*)&AsPenguinDuck(self)->listNext));
        if (end2 != cur) {
            RenderNode* first = end2->next;
            cur->next->prev = first->prev;
            first->prev->next = cur->next;
            while (end2 != cur) {
                RenderNode* dead = cur;
                cur = cur->prev;
                appFreeUni__FPv(dead);
                int cnt = AsPenguinDuck(self)->puppetCount;
                AsPenguinDuck(self)->puppetCount = cnt - 1;
            }
        }
        (*(void **)&AsCCreature(self)->puppet) = 0;
        if (((char*)&AsPenguinDuck(self)->puppetCount)) {
            if (((char*)&AsPenguinDuck(self)->puppetCount)) {
                if (((char*)&AsPenguinDuck(self)->puppetCount)) {
                    if (((char*)&AsPenguinDuck(self)->puppetCount)) {
                        cur = (*(RenderNode **)&AsPenguinDuck(self)->listPrevious);
                        RenderNode* end3 = (RenderNode*)(((char*)&AsPenguinDuck(self)->listNext));
                        if (end3 != cur) {
                            RenderNode* first = end3->next;
                            cur->next->prev = first->prev;
                            first->prev->next = cur->next;
                            while (end3 != cur) {
                                RenderNode* dead = cur;
                                cur = cur->prev;
                                appFreeUni__FPv(dead);
                                int cnt = AsPenguinDuck(self)->puppetCount;
                                AsPenguinDuck(self)->puppetCount = cnt - 1;
                            }
                        }
                    }
                }
            }
        }
        CCreature_dtor(self, 0);
        if (flag > 0) __dl__FPv(self);
    return self;
}

#pragma push
#pragma use_lmw_stmw on
struct DuckElement {
    char data[0x30];
    // Preserve the matrix default-construction hook for one duck instance.
    DuckElement() { WMatrix_DefaultConstructNoOp(this); }
};
// Initialize ntPenguin_Duck state and owned resources.
void* ntPenguin_Duck_Ctor(void* self, void* arg, int count) {
    char* obj = (char*)self;
    *(void* volatile*)(obj + 0) = (void*)__vt__9CCreature;
    *(void* volatile*)(obj + 4) = (char*)__vt__9CCreature + kCreatureSecondaryVtableOffset;
    (*(volatile int*)&AsCCreature(obj)->flags) = 0;
    Vec3_Zero(((char*)&AsCCreature(self)->movementBoundsOrigin[0]));
    Vec3_Zero(((char*)&AsCCreature(self)->movementBoundsMax[0]));
    float zero = kFloat_0_Creature;
    (*(void**)&AsCCreature(obj)->puppet) = 0;
    AsCCreature(obj)->animationTime = zero;
    AsCCreature(obj)->animationRange = 0;
    AsCCreature(obj)->heading = zero;
    AsCCreature(obj)->history = 0;
    AsCCreature(obj)->ballSpawnOffsetDistance = zero;
    AsCCreature(obj)->opacityKeyframeCount = 0;
    AsCCreature(obj)->opacityKeyframePrevious = ((char*)&AsCCreature(obj)->opacityKeyframeNext);
    AsCCreature(obj)->opacityKeyframeNext = ((char*)&AsCCreature(obj)->opacityKeyframeNext);
    AsCCreature(obj)->targetPoints = 0; AsCCreature(obj)->targetPointCount = 0; AsCCreature(obj)->targetPointCapacity = 0;
    AsCCreature(obj)->pathPoints = 0; AsCCreature(obj)->pathPointCount = 0; AsCCreature(obj)->pathPointCapacity = 0;
    AsCCreature(obj)->vtable = (void*)__vt__14ntPenguin_Duck;
    AsCCreature(obj)->secondaryVtable = (char*)__vt__14ntPenguin_Duck + kCreatureSecondaryVtableOffset;
    AsPenguinDuck(self)->puppetCount = 0;
    (*(void**)&AsPenguinDuck(self)->listPrevious) = ((char*)&AsPenguinDuck(self)->listNext);
    (*(void**)&AsPenguinDuck(self)->listNext) = ((char*)&AsPenguinDuck(self)->listNext);
    (*(int*)&AsPenguinDuck(self)->instanceMatrices) = 0;
    AsCCreature(obj)->movementSpeed = kFloat_2_805DF2B0;
    AsCCreature(obj)->turnRate = kFloat_0_5235988_Creature;
    AsCCreature(obj)->maxBankAngle = zero;
    AsCCreature(obj)->cellSize = kFloat_32_Creature;
    AsPenguinDuck(self)->instanceCount = count;
    if (count > 0) {
        void* raw = new DuckElement[count];
        AsPenguinDuck(obj)->instanceMatrices = raw;
        float* table = (float*)__nwa__FUl(AsPenguinDuck(obj)->instanceCount * 4);
        (*(float**)&AsPenguinDuck(self)->instanceScales) = table;
        table[0] = kFloat_1_Creature;
        for (int i = 1; i < AsPenguinDuck(obj)->instanceCount; ++i)
            ((*(float**)&AsPenguinDuck(obj)->instanceScales))[i] = Random_RangeFloat(-1, kFloat_0_6_Creature, kFloat_0_8_805DF2D8);
    }
    RenderNode* sentinel = (RenderNode*)(((char*)&AsPenguinDuck(obj)->listNext));
    RenderNode* node = (*(RenderNode**)&AsPenguinDuck(obj)->listPrevious);
    AsCCreature(self)->movementEnabled = 0;
    if (node != sentinel) {
        RenderNode* first = sentinel->next;
        node->next->prev = first->prev;
        first->prev->next = node->next;
        while (node != sentinel) {
            RenderNode* dead = node;
            node = node->prev;
            appFreeUni__FPv(dead);
            --AsPenguinDuck(obj)->puppetCount;
        }
    }
    struct VirtualObject {
        virtual void f00() DECOMP_LOCAL_VIRTUAL; virtual void f01() DECOMP_LOCAL_VIRTUAL; virtual void f02() DECOMP_LOCAL_VIRTUAL; virtual void f03() DECOMP_LOCAL_VIRTUAL;
        virtual void f04() DECOMP_LOCAL_VIRTUAL; virtual void f05() DECOMP_LOCAL_VIRTUAL; virtual void f06() DECOMP_LOCAL_VIRTUAL; virtual void f07() DECOMP_LOCAL_VIRTUAL;
        virtual void f08() DECOMP_LOCAL_VIRTUAL; virtual void f09() DECOMP_LOCAL_VIRTUAL; virtual void f10() DECOMP_LOCAL_VIRTUAL; virtual void f11() DECOMP_LOCAL_VIRTUAL;
        virtual void f12() DECOMP_LOCAL_VIRTUAL; virtual void f13() DECOMP_LOCAL_VIRTUAL; virtual void f14() DECOMP_LOCAL_VIRTUAL; virtual void f15() DECOMP_LOCAL_VIRTUAL;
        virtual void f16() DECOMP_LOCAL_VIRTUAL; virtual void f17() DECOMP_LOCAL_VIRTUAL; virtual void f18() DECOMP_LOCAL_VIRTUAL; virtual void f19() DECOMP_LOCAL_VIRTUAL;
        virtual void f20() DECOMP_LOCAL_VIRTUAL; virtual void f21() DECOMP_LOCAL_VIRTUAL; virtual void f22() DECOMP_LOCAL_VIRTUAL; virtual void f23() DECOMP_LOCAL_VIRTUAL;
        virtual void unusedSlot24() DECOMP_LOCAL_VIRTUAL; virtual void f25() DECOMP_LOCAL_VIRTUAL; virtual void f26() DECOMP_LOCAL_VIRTUAL; virtual void f27() DECOMP_LOCAL_VIRTUAL;
        virtual void unusedSlot28() DECOMP_LOCAL_VIRTUAL; virtual void f29() DECOMP_LOCAL_VIRTUAL; virtual void f30() DECOMP_LOCAL_VIRTUAL; virtual void f31() DECOMP_LOCAL_VIRTUAL;
        virtual void f32() DECOMP_LOCAL_VIRTUAL; virtual void f33() DECOMP_LOCAL_VIRTUAL; virtual void f34() DECOMP_LOCAL_VIRTUAL; virtual void f35() DECOMP_LOCAL_VIRTUAL;
        virtual void initialize(void*) DECOMP_LOCAL_VIRTUAL;
    };
    ((VirtualObject*)self)->initialize(arg);
    return self;
}
#pragma pop

struct ItemAllocation {
    void* allocator;
    ItemNode* volatile position;
    ItemNode* node;
    // Free the node allocation if it has not transferred.
    ~ItemAllocation() { if (node) appFreeUni__FPv(node); }
};
// Store a loaded puppet in its newly allocated list node.
inline void constructItem(void** data, void* const& value) {
    *data = value;
}

#pragma opt_common_subs off
#pragma use_lmw_stmw on
// Load puppet instances into the penguin-duck list and use its first puppet as the leader.
void ntPenguin_Duck_LoadPuppetInstances(void* self, void* arg) {
    ItemNode* sentinel;
    int index = 0;
    if (AsPenguinDuck(self)->instanceCount > 0) {
      sentinel = (ItemNode*)(((char*)&AsPenguinDuck(self)->listNext));
      while (index < AsPenguinDuck(self)->instanceCount) {
        void* value;
        if (index == 0) value = CCreature_LoadPuppetResource(self, arg, 1);
        else value = CCreature_LoadPuppetResource(self, arg, 0);
        {
        volatile unsigned char allocator = 0;
        ItemNode* raw = (ItemNode*)appMallocDummy__FUl(12);
        ItemAllocation allocation;
        ItemNode* volatile memory;
        ItemNode* volatile allocated;
        memory = raw;
        allocated = raw;
        allocation.position = sentinel;
        allocation.node = raw;
        void** data = &raw->value;
        if (data) {
            try { constructItem(data, value); }
            catch (...) { throw; }
        }
        ItemNode* node = allocation.node;
        allocation.node = 0;
        sentinel->next->prev = node;
        node->next = sentinel->next;
        sentinel->next = node;
        node->prev = sentinel;
        int cnt = AsPenguinDuck(self)->puppetCount;
        AsPenguinDuck(self)->puppetCount = cnt + 1;
        }
        ++index;
      }
    }
    void* p = (*(void **)&AsPenguinDuck(self)->listPrevious);
    (*(void **)&AsCCreature(self)->puppet) = *(void**)((char*)p + 8);
}
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

} // extern "C"

#undef THROW_FRAME_LENGTH
#undef V
} // namespace ns3

// ---- merged from course_creatures_flocks_and_flyers.cpp ----
namespace ns4 {
// Translation unit 0x801A10FC-0x801A2788 (10 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
extern const float kFloat_0_Creature;
extern const float kFloat_0_7_Creature;
extern const float kFloat_neg0_7_Creature;
extern const float kFloat_4_16_Creature;
extern const float kFloat_6_4_Creature;
extern const float kFloat_0_1_Creature;
extern const float kFloat_0_00001_CreaturePath;
extern const float kFloat_1_Creature;
extern const float kFloat_0_8_805DF2D8;
extern const float kFloat_100_Creature;
extern unsigned int g_view;
extern void* CCreature_Draw(void*);
extern void Spline_EvaluatePosition(void*, const void*, float);
extern void __as__7WVectorFRC7WVector(void*, const void*);
extern int Vec3_NearlyEqualWithTolerance(void*, const void*, float);
extern void Matrix_BuildCameraTransform(void*, const void*, const void*);
extern void Matrix_CopyAffine(void*, const void*);
extern void CCreature_InitRandomGridCell(void*, const void*, unsigned int);
extern void __ml__FfRC7WVector(void*, const void*, float);
extern void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern void Vec3_Zero(void*);
extern int String_CompareCString(const char*, const char*);
extern void FxAttachment_Init(void*);
extern void* CFx_CreateAttachedEffect(unsigned int, void*, const char*, void*, void*, void*, void*);
extern unsigned int g_fx;
extern const char npcResourceStrings[];
extern const char __vt__9CCreature[];
extern const char __vt__12ntNpc_Dragon[];
extern void __dla__FPv(void*);
extern void __dl__FPv(void*);
extern void* CCreature_dtor(void*, int);
extern int WPuppet_FindAnimationRangeByName(void*, const char*);
extern void WPuppet_SetAnimationTimeFromRanges(void*, int, float, int, int);
extern void Matrix_CopyScaledBasis(void*, const void*, float);
extern void Model_SetMatrixRecursive(void*, void*, const void*);
extern void WPuppet_UpdateBoneWorldMatrices(void*, unsigned int, int, float);
extern void WPuppet_UpdateBound(void*, int);
extern void WPuppet_UpdateBoundsFromBone(void*);
extern void AnimationEventParser_ProcessFrameRange(void*, void*, float, float, void*, int, int); /* AnimationEventParser_ProcessFrameRange is (self, manager r4, bounds r5, f1, f2, r6, r7): f2 is the new time (0x802A663C reads it) */
extern int CString_Compare(const char*, const char*);
extern void* CQuadTree_GetInstance(void*);
extern void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, const void*, int, int, int);
extern void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern float WVectorLen__FRC7WVector(const void*);
extern float Vector_SignedAngleXZ(void*, const void*);
extern void Matrix_SetAxisRotation(void*, float, int);
extern void Matrix_ConcatAffine(void*, const void*, const void*);
extern void __ml__FRC7WVectorf(void*, const void*, float);
extern void __apl__7WVectorFRC7WVector(void*, const void*);
extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void* Normalize__7WVectorFv(void*);
extern void Vector_CrossProduct(void*, const void*, const void*);
extern float Magnitude__7WVectorCFv(void*);
extern void Matrix_CopyAffine(void*, const void*);
extern void Spline_Init(void*, void*, float*, int);
extern void* __nw__FUl(unsigned int);
extern void __throw(const char*, void*, void*) __attribute__((noreturn));
extern void __dt__Q23std12length_errorFv(void*);
extern const char __vt__Q23std11logic_error[];
extern const char s_sharedCharDeleterVtable[];
extern const char __vt__Q23std12length_error[];
extern const char __vt__9CCreature[];
extern const char __vt__12ntNpc_Dragon[];

// Select the requested clip on the first flock puppet and clamp its starting time.
static inline void SelectClip(char* p, int nameOff) {
    const char* name = npcResourceStrings + nameOff;
    ns0::ListNode* leaderLink = (ns0::ListNode*)AsPenguinDuck(p)->listPrevious;
    void* leaderPuppet = leaderLink->value; // Borrowed +8 payload; nonnull link required.
    if (leaderPuppet && (!AsCCreature(p)->animationRange || CString_Compare(((CreatureAnimationRange*)AsCCreature(p)->animationRange)->name, name) != 0)) {
        void* found = (void*)WPuppet_FindAnimationRangeByName(leaderPuppet, name);
        if (found) {
            (*(void **)&AsCCreature(p)->animationRange) = found;
            float lo;
            float zero;
            float hi;
            float v;
            zero = kFloat_0_Creature;
            lo = *(float*)found;
            hi = ((CreatureAnimationRange*)found)->endTime;
            v = lo + zero;
            hi = (v > hi) ? hi : v;
            AsCCreature(p)->animationTime = hi;
        } else {
            AsCCreature(p)->animationTime = kFloat_0_Creature;
        }
    }
}

// Advance leader animation and update the ground-following duck chain when movement is enabled.
void ntPenguin_Duck_UpdateAnimationAndFollowChain(void* self_, float delta) {
    int off;
    int i;
    char* p;
    float lowerScale, upperScale, threshold;
    p = (char*)self_;
    float fv[12];
    float sc[12];
    float t[3];
    float out[3];
    unsigned int m[3];
    float d[3];
    float rot[3];
    float tmp[3];
    float pos[3];
    float o[3];
    unsigned int s[3];
    char* base;
    char* cur;
    unsigned int prev;
    if (AsPenguinDuck(p)->puppetCount == 0) return;
    if (AsCCreature(p)->animationActive != 0) {
        float zero = kFloat_0_Creature;
        float time = AsCCreature(p)->animationTime;
        time += delta;
        AsCCreature(p)->animationTime = time;
        if (time < zero) return;
        AnimationEventParser_ProcessFrameRange(p + 4, (*(void **)&AsCCreature(p)->puppet), time - delta, time, (*(void **)&AsCCreature(p)->animationRange), -1, 0);
        void* current = (*(void **)&AsCCreature(p)->animationRange);
        float end;
        float t2;
        t2 = AsCCreature(p)->animationTime;
        end = ((CreatureAnimationRange*)current)->endTime;
        if (t2 > end) {
            if (AsCCreature(p)->flags & kCreatureOneShotAnimation) {
                AsCCreature(p)->animationActive = 0;
                AsCCreature(p)->animationTime = *(float*)current;
            } else {
                AsCCreature(p)->animationTime = t2 - (end - *(float*)current);
                SelectClip(p, kCreatureStrings_action0);
                switch (rand() % 10) {
                case 0:
                    SelectClip(p, kCreatureStrings_action1);
                    AsCCreature(p)->movementEnabled = 0;
                    break;
                default:
                    SelectClip(p, kCreatureStrings_action0);
                    AsCCreature(p)->movementEnabled = 1;
                    break;
                }
            }
        }
        if (AsCCreature(p)->movementEnabled) {
            ((void (*)(void*, float)) * (void**)((char*)*(void**)p + 0x90))(p, delta);
        }
        unsigned int a = (*(unsigned int*)&AsCCreature(p)->transform[9]);
        unsigned int b = (*(unsigned int*)&AsCCreature(p)->transform[10]);
        m[0] = a;
        m[1] = b;
        m[2] = (*(unsigned int*)&AsCCreature(p)->transform[11]);
        void* ret = CQuadTree_GetInstance((void*)a);
        GetGroundPoint__9CQuadTreeF7WVectorbPib(out, ret, m, 1, 0, 0);
        __as__7WVectorFRC7WVector((p + kCreaturePositionOffset), out);
        if (!AsPenguinDuck(p)->instanceCount) return;
        if (!AsCCreature(p)->movementEnabled) return;
        Matrix_CopyAffine(AsPenguinDuck(p)->instanceMatrices, ((char*)&AsCCreature(p)->transform[0]));
        upperScale = kFloat_0_7_Creature;
        i = 1;
        lowerScale = kFloat_neg0_7_Creature;
        off = 0x30;
        threshold = kFloat_4_16_Creature;
        while (i < AsPenguinDuck(p)->instanceCount) {
            prev = (unsigned int)(i - 1) * 0x30u;
            base = (*(char **)&AsPenguinDuck(p)->instanceMatrices);
            __mi__FRC7WVectorRC7WVector(d, base + prev + 0x24, base + off + 0x24);
            __as__7WVectorFRC7WVector(t, d);
            if (!(WVectorLen__FRC7WVector(t) < threshold)) {
                float clamped;
                float upper;
                float lower;
                base = (*(char **)&AsPenguinDuck(p)->instanceMatrices);
                cur = base + off;
                upper = upperScale * AsCCreature(p)->turnRate * delta;
                __mi__FRC7WVectorRC7WVector(rot, base + prev + 0x24, cur + 0x24);
                float f1 = Vector_SignedAngleXZ(rot, cur + 0x18);
                lower = lowerScale * AsCCreature(p)->turnRate * delta;
                if (f1 < lower) {
                    clamped = lower;
                } else {
                    upper = (f1 > upper) ? upper : f1;
                    clamped = upper;
                }
                Matrix_SetAxisRotation(sc, clamped, 1);
                base = (*(char **)&AsPenguinDuck(p)->instanceMatrices);
                Matrix_ConcatAffine(fv, sc, base + off);
                base = (*(char **)&AsPenguinDuck(p)->instanceMatrices);
                Matrix_CopyAffine(base + off, fv);
                base = (*(char **)&AsPenguinDuck(p)->instanceMatrices);
                __ml__FfRC7WVector(pos, base + off + 0x18, AsCCreature(p)->movementSpeed);
                __ml__FRC7WVectorf(tmp, pos, delta);
                base = (*(char **)&AsPenguinDuck(p)->instanceMatrices);
                __apl__7WVectorFRC7WVector(base + off + 0x24, tmp);
                base = (*(char **)&AsPenguinDuck(p)->instanceMatrices);
                unsigned int sa = *(unsigned int*)(base + off + 0x24);
                unsigned int sb = *(unsigned int*)(base + off + 0x28);
                s[0] = sa;
                s[1] = sb;
                s[2] = *(unsigned int*)(base + off + 0x2c);
                void* r = CQuadTree_GetInstance((void*)sa);
                GetGroundPoint__9CQuadTreeF7WVectorbPib(o, r, s, 1, 0, 0);
                __as__7WVectorFRC7WVector((*(char **)&AsPenguinDuck(p)->instanceMatrices) + off + 0x24, o);
            }
            off += 0x30;
            ++i;
        }
    } else {
        return;
    }
}

// Cull and draw the penguin-duck list using its per-instance transforms.
void ntPenguin_Duck_CullAndDrawInstancePuppets(void* self) {
    char* p = (char*)self;
    if (!AsPenguinDuck(p)->puppetCount) return;
    if (!AsCCreature(p)->animationActive) return;
    if (AsCCreature(p)->animationTime < kFloat_0_Creature) return;
    if (!AsPenguinDuck(p)->instanceCount) { CCreature_Draw(self); return; }
    void* sentinel = p + 0x124;
    unsigned int offset = 0;
    int index = 0;
    void* node = (*(void **)&AsPenguinDuck(p)->listPrevious);
    float f31 = kFloat_1_Creature;
    while (node != sentinel) {
        void* item = *(void**)((char*)node + 8);
        if (item) {
            if (((*(int (**)(void*, void*, float))((char*)*(void**)p + 0xa0))(self, (char*)AsPenguinDuck(p)->instanceMatrices + offset + 0x24, kFloat_0_8_805DF2D8)) == 0) { float matrix[12];
                WPuppet_SetAnimationTimeFromRanges(item, 0, AsCCreature(p)->animationTime, 0, 0);
                if (*(float*)((char*)AsPenguinDuck(p)->instanceMatrices + offset) > f31) *(float*)((char*)AsPenguinDuck(p)->instanceMatrices + offset) = f31;
                if (*(float*)((char*)AsPenguinDuck(p)->instanceMatrices + offset + 0x10) > f31) *(float*)((char*)AsPenguinDuck(p)->instanceMatrices + offset + 0x10) = f31;
                if (*(float*)((char*)AsPenguinDuck(p)->instanceMatrices + offset + 0x20) > f31) *(float*)((char*)AsPenguinDuck(p)->instanceMatrices + offset + 0x20) = f31;

                Matrix_CopyScaledBasis(matrix, (char*)AsPenguinDuck(p)->instanceMatrices + offset, (index == 0) ? kFloat_1_Creature : kFloat_0_8_805DF2D8);
                Model_SetMatrixRecursive(item, *(void**)((char*)node+8), matrix);
                WPuppet_UpdateBoneWorldMatrices(item, g_view, 0, kFloat_1_Creature);
                WPuppet_UpdateBound(item, 0);
                WPuppet_UpdateBoundsFromBone(item);
                (*(void (**)(void*, unsigned int, int, int, float, int, int))((char*)*(void**)item + 0x18))(item, g_view, 1, 1, kFloat_1_Creature, 1, 0);
            }
        }
        node = *(void**)((char*)node + 4);
        offset += 0x30;
        index++;
    }
}

// Wait for the spawn delay, advance spline distance, and select a new dragon clip after completion.
void ntNpc_Dragon_UpdateAnimationAndMovement(void* self, float delta) {
    struct VirtualObject {
        virtual void v08() DECOMP_LOCAL_VIRTUAL; virtual void v0C() DECOMP_LOCAL_VIRTUAL; virtual void v10() DECOMP_LOCAL_VIRTUAL; virtual void v14() DECOMP_LOCAL_VIRTUAL;
        virtual void v18() DECOMP_LOCAL_VIRTUAL; virtual void v1C() DECOMP_LOCAL_VIRTUAL; virtual void v20() DECOMP_LOCAL_VIRTUAL; virtual void v24() DECOMP_LOCAL_VIRTUAL;
        virtual void v28() DECOMP_LOCAL_VIRTUAL; virtual void v2C() DECOMP_LOCAL_VIRTUAL; virtual void v30() DECOMP_LOCAL_VIRTUAL; virtual void v34() DECOMP_LOCAL_VIRTUAL;
        virtual void v38() DECOMP_LOCAL_VIRTUAL; virtual void v3C() DECOMP_LOCAL_VIRTUAL; virtual void v40() DECOMP_LOCAL_VIRTUAL; virtual void v44() DECOMP_LOCAL_VIRTUAL;
        virtual void v48() DECOMP_LOCAL_VIRTUAL; virtual void v4C() DECOMP_LOCAL_VIRTUAL; virtual void v50() DECOMP_LOCAL_VIRTUAL; virtual void v54() DECOMP_LOCAL_VIRTUAL;
        virtual void v58() DECOMP_LOCAL_VIRTUAL; virtual void v5C() DECOMP_LOCAL_VIRTUAL; virtual void v60() DECOMP_LOCAL_VIRTUAL; virtual void v64() DECOMP_LOCAL_VIRTUAL;
        virtual void v68() DECOMP_LOCAL_VIRTUAL; virtual void v6C() DECOMP_LOCAL_VIRTUAL; virtual void v70() DECOMP_LOCAL_VIRTUAL; virtual void v74() DECOMP_LOCAL_VIRTUAL;
        virtual void v78() DECOMP_LOCAL_VIRTUAL; virtual void v7C() DECOMP_LOCAL_VIRTUAL; virtual void v80() DECOMP_LOCAL_VIRTUAL; virtual void v84() DECOMP_LOCAL_VIRTUAL;
        virtual void v88() DECOMP_LOCAL_VIRTUAL; virtual void v8C() DECOMP_LOCAL_VIRTUAL; virtual void v90(float) DECOMP_LOCAL_VIRTUAL;
        virtual void v94() DECOMP_LOCAL_VIRTUAL; virtual void v98() DECOMP_LOCAL_VIRTUAL; virtual void v9C() DECOMP_LOCAL_VIRTUAL;
        virtual int vA0(void*, float) DECOMP_LOCAL_VIRTUAL;
    };
    float t;
    char* p = (char*)self;
    if (!AsCCreature(p)->animationActive) return;
    void* resource = (*(void **)&AsCCreature(p)->puppet);
    if (resource == 0) { return; } else {
    t = AsCCreature(p)->animationTime + delta;
    AsCCreature(p)->animationTime = t;
    if (*(unsigned char*)(((char*)&AsSplineNpc(p)->delayActiveBits))) {
        if (t > AsSplineNpc(p)->spawnDelay) {
        float zero = kFloat_0_Creature;
        *(unsigned char*)(((char*)&AsSplineNpc(p)->delayActiveBits)) = 0;
        AsCCreature(p)->animationTime = zero;
        const char* name = npcResourceStrings + kCreatureStrings_defaultAnimation;
        if (resource) {
            void* current = (*(void **)&AsCCreature(p)->animationRange);
            if (!current || CString_Compare(((CreatureAnimationRange*)current)->name, name)) {
                void* found = (void*)WPuppet_FindAnimationRangeByName(resource, name);
                if (found) {
                    (*(void **)&AsCCreature(p)->animationRange) = found;
                    float zero2 = kFloat_0_Creature;
                    float lo = *(float*)found;
                    float hi = ((CreatureAnimationRange*)found)->endTime;
                    float v = lo + zero2;
                    AsCCreature(p)->animationTime = (v > hi) ? hi : v;
                } else {
                    AsCCreature(p)->animationTime = kFloat_0_Creature;
                }
            }
        }
        } else { return; }
    }
    t = AsCCreature(p)->animationTime;
    AnimationEventParser_ProcessFrameRange(p+4, (*(void **)&AsCCreature(p)->puppet), t - delta, t, (*(void **)&AsCCreature(p)->animationRange), -1, 0); /* 0x801A1CC0-0x801A1CDC: original lfs f2, 0x58(r31) before the call; ours left f2 unset */
    AsSplineNpc(p)->pathDistance += kFloat_100_Creature * delta;
    ((VirtualObject*)self)->v90(delta);
    if (((VirtualObject*)self)->vA0(((char*)&AsCCreature(p)->transform[9]), kFloat_0_8_805DF2D8)) return;
    void* cur = (*(void **)&AsCCreature(p)->animationRange);
    if (!(AsCCreature(p)->animationTime > ((CreatureAnimationRange*)cur)->endTime)) return;
    int n = rand();
    int phase = n % 101;
    char text[32] = {0};
    if (phase < 5) strcpy(text, npcResourceStrings + kCreatureStrings_action02);
    else if (phase < 10) strcpy(text, npcResourceStrings + kCreatureStrings_action01);
    else strcpy(text, npcResourceStrings + kCreatureStrings_defaultAnimation);
    int done;
    void* old = (*(void **)&AsCCreature(p)->puppet);
    if (old == 0) {
        done = 0;
    } else {
        void* cur2 = (*(void **)&AsCCreature(p)->animationRange);
        if (cur2 && !CString_Compare(((CreatureAnimationRange*)cur2)->name, text)) {
            done = 0;
        } else {
            void* found = (void*)WPuppet_FindAnimationRangeByName(old, text);
            if (found) {
                (*(void **)&AsCCreature(p)->animationRange) = found;
                float zero = kFloat_0_Creature;
                float lo = *(float*)found;
                float hi = ((CreatureAnimationRange*)found)->endTime;
                float v = lo + zero;
                AsCCreature(p)->animationTime = (v > hi) ? hi : v;
            } else {
                AsCCreature(p)->animationTime = kFloat_0_Creature;
            }
            done = 1;
        }
    }
    if (!done) {
        void* found = (void*)WPuppet_FindAnimationRangeByName((*(void **)&AsCCreature(p)->puppet), text);
        (*(void **)&AsCCreature(p)->animationRange) = found;
        AsCCreature(p)->animationTime = *(float*)found;
    }
    }
}

// Initialize leader and follower transforms at fixed spacing along their forward axes.
void ntPenguin_Duck_InitInstanceMatrices(void* self, const void* src, unsigned int flags) {
    CCreature_InitRandomGridCell(self, src, flags);
    if (AsPenguinDuck(self)->instanceCount != 0) { Matrix_CopyAffine(AsPenguinDuck(self)->instanceMatrices, ((char*)&AsCCreature(self)->transform[0]));
        int off = 0x30;
        int i = 1;

        // Scalar-copy staging preserves separate index and byte-offset registers.
        while (i < AsPenguinDuck(self)->instanceCount) { float tmp2[3];__typeof__(off) followerOffset = off;
            __typeof__(i) instanceForCopy = i;
            __typeof__(instanceForCopy) copyIndex = instanceForCopy; Matrix_CopyAffine((char*)AsPenguinDuck(self)->instanceMatrices + followerOffset, (char*)AsPenguinDuck(self)->instanceMatrices + (copyIndex - 1) * 0x30);

            __typeof__(i) scaleIndex = i; float tmp[3]; __ml__FfRC7WVector(tmp, (char*)AsPenguinDuck(self)->instanceMatrices + (scaleIndex - 1) * 0x30 + 0x18, kFloat_6_4_Creature);

            __typeof__(off) instanceOffset = off;
            __typeof__(instanceOffset) savedOffset = instanceOffset;
            __typeof__(savedOffset) positionOffset = savedOffset;
            __typeof__(i) positionIndex = i; __pl__FRC7WVectorRC7WVector(tmp2, (char*)AsPenguinDuck(self)->instanceMatrices + (positionIndex - 1) * 0x30 + 0x24, tmp);
            __typeof__(1 + i) nextInstance = 1 + i;
            __as__7WVectorFRC7WVector((char*)AsPenguinDuck(self)->instanceMatrices + positionOffset + 0x24, tmp2);
            off += 0x30;
            __typeof__(i) _p889 = i;
            __typeof__(1 + _p889) _p811 = 1 + _p889;
            __typeof__(_p811) _p141 = i + 1;
            __typeof__(i + 1) _p989 = 1 + i;
            __typeof__(_p141) _p802 = nextInstance;
            __typeof__(i + 1) _p796 = _p989;
            __typeof__(i) _p499 = i;
            __typeof__(_p796) _p243 = _p802;
            __typeof__(_p499) _p935 = _p499;
            __typeof__(_p935 + 1) _p891 = _p243;
            __typeof__(1 + i) _p759 = 1 + i;
            __typeof__(i + 1) _p129 = _p759;
            __typeof__(1 + i) _p426 = 1 + i;
            __typeof__(_p426) _p317 = 1 + i;
            __typeof__(i + 1) _p258 = _p317;
            __typeof__(_p129) _p949 = _p258;
            __typeof__(_p949) _p631 = _p891; i = _p631;
        }
    }
}

// Initialize ntNpc_Dragon state and owned resources.
void* ntNpc_Dragon_Ctor(void* self, void* arg, float value) {
    char* p = (char*)self;
    const char** vt = (const char**)p;
    *vt = __vt__9CCreature;
    vt[1] = __vt__9CCreature + 12;
    AsCCreature(p)->flags = 0;
    Vec3_Zero(p + 12);
    Vec3_Zero(p + 24);
    AsCCreature(p)->puppet = 0;
    AsCCreature(p)->animationTime = kFloat_0_Creature;
    AsCCreature(p)->animationRange = 0;
    AsCCreature(p)->heading = kFloat_0_Creature;
    AsCCreature(p)->history = 0;
    AsCCreature(p)->ballSpawnOffsetDistance = kFloat_0_Creature;
    AsCCreature(p)->opacityKeyframeCount = 0;
    void** b8 = (void**)(((char*)&AsCCreature(p)->opacityKeyframeNext));
    *b8 = b8;
    AsCCreature(p)->opacityKeyframePrevious = b8;
    AsCCreature(p)->targetPoints = 0;
    AsCCreature(p)->targetPointCount = 0;
    AsCCreature(p)->targetPointCapacity = 0;
    AsCCreature(p)->pathPoints = 0;
    AsCCreature(p)->pathPointCount = 0;
    AsCCreature(p)->pathPointCapacity = 0;
    *vt = __vt__12ntNpc_Dragon;
    vt[1] = __vt__12ntNpc_Dragon + 12;
    (*(int*)&AsSplineNpc(p)->splinePoints) = 0;
    (*(int*)&AsSplineNpc(p)->splineDistances) = 0;
    (*(int*)&AsSplineNpc(p)->splineCoefficients) = 0;
    (*(float*)&AsSplineNpc(p)->splineLengthBits) = kFloat_0_Creature;
    (*(int*)&AsSplineNpc(p)->splinePointCount) = 0;
    void (**vtable)(void*, void*) = (void (**)(void*, void*))*((void**)p);
    vtable[0x98 / 4](self, arg);
    *(unsigned char*)(((char*)&AsSplineNpc(p)->delayActiveBits)) = 1;
    AsSplineNpc(p)->pathDistance = kFloat_0_Creature;
    AsSplineNpc(p)->spawnDelay = value;
    return self;
}

// Parse a bone effect tag and attach a supported sprite or sequence resource.
void ntNpc_Dragon_AttachBoneEffect(void* self, void* arg) {
    void* inner;
    if (arg == 0 || (inner = *(void**)((char*)*(void**)((char*)arg + 4) + 0x28)) == 0) return;
        char* base = (char*)npcResourceStrings;
        struct Buffers { char small1[8]; char small2[0x20]; char filename[0x20]; char handle[0x30]; } buffers;
        sscanf(strstr((const char*)inner, base + kCreatureStrings_effectTag), base + kCreatureStrings_effectTagFormat, buffers.small1, buffers.filename, buffers.small2);
        if (*(void**)arg == 0) return;
        if (*(unsigned int*)((char*)*(void**)arg + 0x74) & 0x40000) return;
        char* ext = strchr(buffers.filename, '.');
        if (ext == 0 || strlen(ext) < 4) return;
        if (String_CompareCString(ext + 1, base + kCreatureStrings_spriteExtension) && String_CompareCString(ext + 1, base + kCreatureStrings_sequenceExtension)) return;
        FxAttachment_Init(buffers.handle);
        *(void**)(buffers.handle + 8) = (char*)*(void**)arg + 0x78;
        void* resource = *(void**)((char*)arg + 4);
        *(void**)(buffers.handle + 0x18) = ((CreatureEffectMetadataView*)resource)->attachmentPayload;
        unsigned char* result = (unsigned char*)CFx_CreateAttachedEffect(g_fx, buffers.handle, buffers.filename, 0, 0, 0, resource);
        if (!String_CompareCString(buffers.filename, npcResourceStrings + kCreatureStrings_dragonFireEffect) && result[0x80]) result[0x80] = 0;
}

// Draw the dragon after its initial delay while its animation is active.
void* ntNpc_Dragon_Draw(void* self) {
    if (AsCCreature(self)->animationActive == 0) return self;
    if (*(unsigned char*)&AsSplineNpc(self)->delayActiveBits == 1) return self;
    return CCreature_Draw(self);
}

} // extern "C"

#pragma exceptions on
struct TargetPoint {
    unsigned int x, y, z;
    // Preserve default or copy construction of an XYZ target record.
    TargetPoint() {}
    // Preserve default or copy construction of an XYZ target record.
    TargetPoint(const TargetPoint& p) { try { unsigned int a, b; a = p.x; b = p.y; x = a; y = b; z = p.z; } catch (...) { throw; } }
};
struct TargetAllocator {
    volatile char state;
    // Initialize the target-vector allocator marker.
    TargetAllocator() : state(0) {}
};
struct TargetRange {
    TargetPoint* volatile first;
    TargetPoint* volatile last;
    // Capture the supplied target range for vector construction.
    TargetRange(TargetPoint* a, TargetPoint* b) {
        last = b; first = a;
    }
};
struct TargetVector {
    TargetPoint* data;
    unsigned int size, capacity;
    // Initialize an empty owning target vector.
    TargetVector() : data(0), size(0), capacity(0) {}
    // Release the owning target-vector allocation.
    ~TargetVector() {
        if (data) { clear(); appFreeUni__FPv(data); }
    }
    // Clear the logical target count while retaining allocated storage.
    void clear() { TargetAllocator allocator; size -= size; }
};
struct TargetBufferGuard {
    char* data;
    bool* flag;
    // Free the target exception-message array if still guarded.
    ~TargetBufferGuard() { if (data) __dla__FPv(data); }
};
struct TargetErrorRef {
    const char* table;
    int strong, weak;
    char* data;
    // Create the reference-counted target error-message control block.
    TargetErrorRef(char* p) { strong = 1; weak = 1; table = s_sharedCharDeleterVtable; data = p; }
};
extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct TargetErrorBase {
    const char* table;
    char* text;
    TargetErrorRef* ref;
    // Initialize the target logic-error base and allocator state.
    TargetErrorBase() {
        TargetAllocator allocator;
        table = __vt__Q23std11logic_error;
    }
    // Invoke the base exception cleanup without deleting the wrapper.
    ~TargetErrorBase() { __dt__Q23std9exceptionFv(this, -1); }
};
struct TargetLengthError : TargetErrorBase {
    // Construct a guarded target-vector length-error message.
    TargetLengthError(const char* message) : TargetErrorBase() {
        bool flag = false;
        unsigned int size = strlen(message) + 1;
        text = (char*)__nwa__FUl(size * 2 - size);
        {
            char* buffer = text;
            TargetBufferGuard guard = {buffer, &flag};
            ref = new TargetErrorRef(buffer);
            guard.data = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
    }
};
// Throw the target-vector length error through the game exception ABI.
static inline void TargetThrowLength() {
    const char* message = npcResourceStrings;
    TargetLengthError error(message);
    __throw(npcResourceStrings + kCreatureStrings_lengthErrorType, &error, __dt__Q23std12length_errorFv);
}

// Copy authored world-space XYZ points and accumulate world-distance spline knots.
// nextPosition/nextDistance advance within caller-owned temporary arrays; segmentDelta is borrowed work storage.
// Cursor references retain the caller lifetimes required for the original register allocation.
static inline void BuildDragonSplineKnots(TargetVector* authoredTargets, TargetPoint*& nextPosition, float*& nextDistance, int& pointIndex, float* segmentDelta) {
    while (pointIndex < authoredTargets->size) {
        int pointByteOffset = pointIndex * 12;
        *(WVector*)nextPosition = *(WVector*)((char*)authoredTargets->data+pointByteOffset);
        unsigned int previousPointByteOffset = (unsigned int)(pointIndex-1)*12u;
        __mi__FRC7WVectorRC7WVector(segmentDelta,(char*)authoredTargets->data+pointByteOffset,(char*)authoredTargets->data+previousPointByteOffset);
        *nextDistance=nextDistance[-1]+(*(WVector*)segmentDelta).Magnitude();
        ++pointIndex;
        ++nextPosition;
        ++nextDistance;
    }
}

extern "C" {
// Copy control points, build cumulative path distances, and initialize the dragon spline.
// input borrows a TargetVector with at least two points; self keeps its own copy.
// Caller arrays remain temporary owners and are released after Spline_Init returns.
void ntNpc_Dragon_InitSplinePath(void* self, void* input, float animationTime) {
    char* creatureBytes = (char*)self;
    TargetVector* ownedTargets = (TargetVector*)&AsCCreature(creatureBytes)->targetPoints;
    TargetVector* authoredTargets = (TargetVector*)input;
    unsigned int authoredCount = *(unsigned int*)((char*)input + 4);
    float segmentDelta[7];
    WVector rightAxis;
    float worldUp[3];

    // Phase 1: clear and reserve the owned target vector; guards retain the old allocation until swap.
    ownedTargets->clear();
    if (authoredCount > (*(unsigned int*)&AsCCreature(creatureBytes)->targetPointCapacity)) {
        unsigned int capacityBeforeGrowth = (*(unsigned int*)&AsCCreature(creatureBytes)->targetPointCapacity);
        TargetVector reservedTargets;
        TargetAllocator allocator;
        if (authoredCount > 0x15555555) {
            // The original vector throws length_error when its requested size
            // cannot be represented as a byte count.
            TargetThrowLength();
        }
        reservedTargets.data = (TargetPoint*)appMallocDummy__FUl(authoredCount * 12);
        TargetAllocator allocatorC;
        reservedTargets.capacity = authoredCount;
        TargetPoint* end = ownedTargets->data + ownedTargets->size;
        TargetPoint* copyDestination = reservedTargets.data + reservedTargets.size;
        for (TargetPoint* copySource = ownedTargets->data; copySource < end; ++copySource, ++copyDestination) {
            new (copyDestination) TargetPoint(*copySource);
            ++reservedTargets.size;
        }
        ownedTargets->clear();
        TargetPoint* old = ownedTargets->data;
        unsigned int oldCapacity = ownedTargets->capacity, oldSize = ownedTargets->size;
        ownedTargets->capacity = reservedTargets.capacity;
        reservedTargets.capacity = oldCapacity;
        ownedTargets->data = reservedTargets.data;
        reservedTargets.data = old;
        ownedTargets->size = reservedTargets.size;
        reservedTargets.size = oldSize;
    }
    // Phase 2: assign the borrowed authored range, constructing any new tail elements.
    TargetPoint* authoredBegin = authoredTargets->data;
    TargetPoint* authoredEnd = authoredTargets->data + authoredTargets->size;
    TargetPoint* volatile savedAuthoredBegin = authoredBegin;
    TargetRange authoredRange(authoredBegin, authoredEnd);
    TargetAllocator allocator;
    unsigned int assignmentCount = authoredEnd - authoredBegin;
    unsigned int replacementCapacity;
    // Complete in-place point assignment before the allocation path.
    do {
        if (assignmentCount > ownedTargets->capacity) {
            volatile unsigned int extra = assignmentCount - ownedTargets->capacity;
            TargetAllocator allocatorA;
            unsigned int capacityBeforeGrowth = ownedTargets->capacity;
            if (extra > 0x15555555 - capacityBeforeGrowth) {
                TargetThrowLength();
            }
            if (capacityBeforeGrowth < 0x071c71c7) {
                unsigned int growth = (capacityBeforeGrowth + 1) * 3 / 5;
                const volatile unsigned int& amount = growth < extra ? extra : growth;
                replacementCapacity = capacityBeforeGrowth + amount;
            } else if (capacityBeforeGrowth < 0x0e38e38e) {
                unsigned int growth = (capacityBeforeGrowth + 1) / 2;
                const volatile unsigned int& amount = growth < extra ? extra : growth;
                replacementCapacity = capacityBeforeGrowth + amount;
            } else replacementCapacity = 0x15555555;
        } else {
            const unsigned int& amount = ownedTargets->size < assignmentCount ? ownedTargets->size : assignmentCount;
            TargetPoint* end = authoredBegin + amount;
            TargetPoint* copySource = savedAuthoredBegin;
            TargetPoint* copyDestination = ownedTargets->data;
            for (; copySource < end; ++copySource, ++copyDestination) *(WVector*)copyDestination = *(WVector*)copySource;
            if (assignmentCount < ownedTargets->size) {
                TargetAllocator allocatorP;
                ownedTargets->size -= ownedTargets->size - assignmentCount;
            } else if (ownedTargets->size < assignmentCount) {
                TargetAllocator allocatorO;
                copyDestination = ownedTargets->data + ownedTargets->size;
                TargetRange tail(end, authoredRange.last);
                for (; tail.first != tail.last;) {
                    new (copyDestination) TargetPoint(*tail.first);
                    TargetPoint* next = tail.first + 1;
                    ++copyDestination;
                    unsigned int oldSize = ownedTargets->size;
                    tail.first = next;
                    ownedTargets->size = oldSize + 1;
                }
            }
            break;
        }
        {
            ownedTargets->clear();
            if (ownedTargets->data) {
                appFreeUni__FPv(ownedTargets->data);
                ownedTargets->data = 0;
                ownedTargets->capacity = 0;
            }
            TargetAllocator allocatorJ;
            if (replacementCapacity > 0x15555555) {
                TargetThrowLength();
            }
            ownedTargets->data = (TargetPoint*)appMallocDummy__FUl(replacementCapacity * 12);
            ownedTargets->capacity = replacementCapacity;
            TargetPoint* copyDestination = ownedTargets->data + ownedTargets->size;
            TargetAllocator allocatorN;
            TargetRange copy(savedAuthoredBegin, authoredRange.last);
            for (; copy.first != copy.last;) {
                new (copyDestination) TargetPoint(*copy.first);
                TargetPoint* next = copy.first + 1;
                ++copyDestination;
                unsigned int oldSize = ownedTargets->size;
                copy.first = next;
                ownedTargets->size = oldSize + 1;
            }
        }
    } while (0);

    // Phase 3: orient the creature from the first world-space segment and seed its first target.
    // Requires at least two authored points and a nonzero first segment.
    WVector firstSegment = *(WVector*)(ownedTargets->data + 1) - *(WVector*)ownedTargets->data;
    (*(WVector*)&AsCCreature(creatureBytes)->transform[6]) = firstSegment.Normalize();
    worldUp[0] = kFloat_0_Creature;
    worldUp[1] = kFloat_1_Creature;
    worldUp[2] = kFloat_0_Creature;
    (*(WVector*)&AsCCreature(creatureBytes)->transform[3]) = *(WVector*)worldUp;
    Vector_CrossProduct(&rightAxis,(creatureBytes + kCreatureUpOffset),(creatureBytes + kCreatureForwardOffset));
    (*(WVector*)&AsCCreature(creatureBytes)->transform[0]) = rightAxis;
    (*(WVector*)&AsCCreature(creatureBytes)->transform[9]) = *(WVector*)(*(void**)input);
    (*(WVector*)&AsCCreature(creatureBytes)->targetPosition[0]) = *(WVector*)((*(char **)&AsCCreature(creatureBytes)->targetPoints)+12);
    unsigned int splinePointCount=*(unsigned int*)((char*)input+4);
    AsCCreature(creatureBytes)->animationTime=animationTime;
    // Phase 4: build cumulative world-distance knots in temporary arrays.
    TargetPoint* splinePositions = new TargetPoint[splinePointCount];
    float* cumulativeDistances=(float*)__nwa__FUl(authoredTargets->size*4);
    *(WVector*)splinePositions = *(WVector*)authoredTargets->data;
    cumulativeDistances[0]=kFloat_0_Creature;
    TargetPoint* nextSplinePosition = splinePositions + 1;
    float* nextSplineDistance = cumulativeDistances + 1;
    int pointIndex;
    pointIndex = 1;
    BuildDragonSplineKnots(authoredTargets, nextSplinePosition, nextSplineDistance, pointIndex, segmentDelta);
    // Phase 5: Spline_Init consumes the arrays before these temporary owners are freed.
    Spline_Init(&AsSplineNpc(creatureBytes)->splinePoints,splinePositions,cumulativeDistances,authoredTargets->size);
    if(splinePositions) delete[] splinePositions;
    if(cumulativeDistances) __dla__FPv(cumulativeDistances);
}

#pragma opt_common_subs off
// Evaluate the spline position and tangent, orient the dragon, and wrap at the path endpoint.
void ntNpc_Dragon_vfunc_31(void* self) {
    float tmp20[12];
    float tmp14[3];
    WVector tmp08;
    Spline_EvaluatePosition(tmp14, ((char*)&AsSplineNpc(self)->splinePoints), AsSplineNpc(self)->pathDistance - kFloat_0_1_Creature);
    Spline_EvaluatePosition(&tmp08, ((char*)&AsSplineNpc(self)->splinePoints), AsSplineNpc(self)->pathDistance);
    (*(WVector*)&AsCCreature(self)->transform[9]) = tmp08;
    if (Vec3_NearlyEqualWithTolerance(tmp14, (void*)(AsCCreature(self)->targetPoints + (AsCCreature(self)->targetPointCount - 1) * 0xc), kFloat_0_00001_CreaturePath)) {
        AsSplineNpc(self)->pathDistance = kFloat_0_Creature;
    }
    Matrix_BuildCameraTransform(tmp20, tmp14, ((char*)&AsCCreature(self)->transform[9]));
    Matrix_CopyAffine(((char*)&AsCCreature(self)->transform[0]), tmp20);
}
#pragma opt_common_subs reset

// Release ntNpc_Dragon resources and optionally delete the object.
void* ntNpc_Dragon_Dtor(void* self, int doDelete) {
    if (self == 0) return self;
    if (((char*)&AsSplineNpc(self)->splinePoints)) {
        if (AsSplineNpc(self)->splinePoints != 0) {
            if (AsSplineNpc(self)->splinePoints != 0) {
                __dla__FPv((char*)AsSplineNpc(self)->splinePoints - 0x10);
            }
            AsSplineNpc(self)->splinePoints = 0;
        }
        if (AsSplineNpc(self)->splineDistances != 0) {
            __dla__FPv(AsSplineNpc(self)->splineDistances);
            AsSplineNpc(self)->splineDistances = 0;
        }
        if ((*(void **)&AsSplineNpc(self)->splineCoefficients) != 0) {
            __dla__FPv((*(void **)&AsSplineNpc(self)->splineCoefficients));
            (*(void **)&AsSplineNpc(self)->splineCoefficients) = 0;
        }
        (*(float*)&AsSplineNpc(self)->splineLengthBits) = kFloat_0_Creature;
        (*(int*)&AsSplineNpc(self)->splinePointCount) = 0;
    }
    CCreature_dtor(self, 0);
    if (doDelete > 0) {
        __dl__FPv(self);
    }
    return self;
}
} // extern "C"

} // namespace ns4

// ---- merged from course_creatures_flocks_and_flyers.cpp ----
namespace ns5 {
// Translation unit 0x801A2788-0x801A3B4C (7 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct Vec3i { int x, y, z; };

extern "C" {
extern void *CCreature_dtor(void *, int);
extern void __dl__FPv(void *);
extern void __dla__FPv(void *);
extern void Vec3_Zero(void *);
extern void appFreeUni__FPv(void *);
extern char __vt__9CCreature[];
extern char __vt__14ntNpc_PolaShip[];
extern char __vt__18ntNpc_BlackSparrow[];
extern char npcResourceStrings[];
extern const float kFloat_0_Creature;
extern const float kFloat_5_5_BlackSparrow;
extern const float kFloat_0_7853982_Creature;
extern float kFloat_48_Creature;
extern void *WPuppet_FindAnimationRangeByName(void *, const char *);
extern void *CString_Compare(void *, const char *);
// AnimationEventParser_ProcessFrameRange is (self, manager r4, bounds r5, f1, f2, r6, r7); f2 is the new time
// (read at 0x802A663C).
extern void AnimationEventParser_ProcessFrameRange(void *, void *, float, float, void *, int, int); /* AnimationEventParser_ProcessFrameRange is (self, manager r4, bounds r5, f1, f2, r6, r7): f2 is the new time (0x802A663C reads it) */
extern void WPuppet_SetOpacityRecursive(void *, unsigned char);
extern void WView_DrawFacingCircleVec4(void *, void *, int, int, int);
extern void __ml__FfRC7WVector(void *, void *, float);
extern void __mi__FRC7WVectorRC7WVector(void *, void *, void *);
extern void WView_DrawLine(void *, void *, int, void *, int, int);
extern void WPuppet_SetAnimationTimeFromRanges(void *, int, float, int, int);
extern void Matrix_SetAxisRotation(void *, float, char);
extern void Matrix_ConcatAffine(void *, void *, void *);
extern void Model_SetMatrixRecursive(void *, void *);
extern void WPuppet_UpdateBoneWorldMatrices(void *, void *, int, float);
extern void WPuppet_UpdateBoundsFromBone(void *);
extern char s_sceneLighting[];
extern void *g_view;
extern void *g_drawSceneDebugGeometry;
extern const float kFloat_5_creature;
extern const float kFloat_0_8_805DF2D8;
extern const float kFloat_1_Creature;
extern const float kFloat_0_017453292_Creature;
extern const double kDouble_SignedIntConversionBias_Creature;
extern float WVectorLen__FRC7WVector(void *);
extern void __as__7WVectorFRC7WVector(void *, void *);
extern void __apl__7WVectorFRC7WVector(void *, void *);
extern float Random_RangeFloat(int, float, float);
extern void *__nwa__FUl(unsigned int);
extern void *appMallocDummy__FUl(unsigned int);
extern float Vector_SignedAngleXZ(void *, void *);
extern void __ml__FRC7WVectorf(void *, void *, float);
extern void *Normalize__7WVectorFv(void *);
extern void *CQuadTree_GetInstance(void);
extern void GetGroundPoint__9CQuadTreeF7WVectorbPib(void *, void *, void *, int, int, int);
extern void Matrix_CopyAffine(void *, void *);
extern unsigned int __vt__Q23std11logic_error[];
extern unsigned int s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern void *__nw__FUl(unsigned int);
extern void __dt__Q23std12length_errorFv(void *);

#define THROW_LENGTH_ERROR()                                                  \
    do {                                                                      \
        char so[12];                                                          \
        const char *sname = (const char *)npcResourceStrings;                       \
        *(void **)(so + 0) = (void *)__vt__Q23std11logic_error;                            \
        unsigned int mlen = strlen(sname) + 1;                                \
        *(void **)(so + 4) = __nwa__FUl(mlen * 2 - mlen);                    \
        void *rec = __nw__FUl(0x10);                                          \
        if (rec) {                                                            \
            ((SharedCharControlView*)rec)->strongCountBits = (void *)1;                        \
            ((SharedCharControlView*)rec)->weakCountBits = (void *)1;                        \
            ((SharedCharControlView*)rec)->vtable = (void *)s_sharedCharDeleterVtable;             \
            ((SharedCharControlView*)rec)->data = *(void **)(so + 4);               \
        }                                                                     \
        *(void **)(so + 8) = rec;                                             \
        strcpy((char *)*(void **)(so + 4), sname);                            \
        *(void **)(so + 0) = (void *)__vt__Q23std12length_error;                            \
        __throw((void *)(sname + 0x14), so,                                   \
                (void *)__dt__Q23std12length_errorFv);                        \
    } while (0)

extern "C" void __dt__Q23std9exceptionFv(void*, int);

struct Alloc801A {
    volatile char state;
    char padding[3];
    // Initialize the grid-path allocator marker.
    Alloc801A() : state(0) {}
};
struct Grow801A {
    Vec3i *data;
    unsigned int size, capacity;
    unsigned int *allocator;
    unsigned int remaining;
    // Initialize an empty temporary grid-path vector tied to owner capacity.
    Grow801A(unsigned int *a) : data(0), size(0), capacity(0), allocator(a), remaining(0) {}
    // Free the grid-path allocation retained after swap or unwind.
    ~Grow801A() { if (data) { Alloc801A allocator; size -= size; appFreeUni__FPv(data); } }
};
struct Copy801A {
    unsigned int x, y, z;
    // Copy three XYZ storage words while retaining exception cleanup.
    Copy801A(const Copy801A &v) {
        try {
            unsigned int a, b;
            a = v.x; b = v.y;
            x = a; y = b; z = v.z;
        } catch (...) {
            throw;
        }
    }
};
// Return the larger grid-path growth request by reference.
inline const unsigned int &max801A(const unsigned int &a, const unsigned int &b) {
    return a < b ? b : a;
}
struct Guard801A {
    void *data;
    void *allocator;
    // Guard the message allocation until shared ownership transfers.
    Guard801A(void *p, void *a) : data(p), allocator(a) {}
    // Free an untransferred message allocation.
    ~Guard801A() { if (data) __dla__FPv(data); }
};
struct Error801A {
    void *vtable;
    char *text;
    void *shared;
    // Initialize the grid-path logic-error base and allocator state.
    Error801A() {
        Alloc801A allocator;
        vtable = __vt__Q23std11logic_error;
    }
    // Run the base exception cleanup without deleting wrapper storage.
    ~Error801A() { __dt__Q23std9exceptionFv(this, -1); }
};
struct LengthError801A : Error801A {
    // Construct the owned grid-path length-error message.
    LengthError801A(const char *message) : Error801A() {
        const char *sname = message;
        Alloc801A alloc;
        unsigned int mlen = strlen(sname) + 1;
        char *t = (char *)__nwa__FUl(mlen * 2 - mlen);
        text = t;
        {
            Guard801A guard(t, (void *)&alloc);
            unsigned int *block = (unsigned int *)__nw__FUl(16);
            if (block) {
                block[1] = 1; block[2] = 1; block[0] = (unsigned int)s_sharedCharDeleterVtable; block[3] = (unsigned int)t;
            }
            shared = block;
            guard.data = 0;
        }
        strcpy(text, sname);
    }
};
// Throw the grid-path length error through the game exception ABI.
inline void lengthError801A() {
    unsigned int storage[sizeof(LengthError801A) / sizeof(unsigned int)];
    LengthError801A *error = new (storage) LengthError801A((const char *)npcResourceStrings);
    error->vtable = __vt__Q23std12length_error;
    __throw(npcResourceStrings + kCreatureStrings_lengthErrorType, error, (void *)__dt__Q23std12length_errorFv);
}

#pragma opt_propagation off
// Choose an unvisited nearby grid cell or recorded waypoint, then move toward the target.
// World-space target/position are at +0x64/+0x4C; +0xA4/+0xA8 hold recent cell
// count/pointer. Grid flattening is x + width*(z + depth*y), with X fastest.
// Require positive X/Y/Z grid counts, a nonempty history, and valid path storage.
// Try at most five neighbor cells (each axis step -1/0/+1), clamp each axis, and
// reject recent cells; the fifth may repeat. YCount==1 uses ground-constrained
// forward motion, otherwise direct 3D pursuit. dt uses the movement-rate time base.
// Black-sparrow input/tuning map (CCreature.h is the common prefix):
// +0C/+18 bounds origin/max are world XYZ; CCreature_InitRandomGridCell
// (creature_puppet_loading.cpp) derives counts with ceil(extent/cellSize), steps=extent/count;
// placement flag 4 enables vertical cells, otherwise YCount=1. It allocates
// history length 4 in 3D, 2 on ground, filled with the initial flattened cell.
// Placement state bit 1 skips this initialization, so such callers supply a
// valid existing grid/history. Re-initialization overwrites history without
// freeing the old array; dispose it first in any future grid-rebuild mod.
// The resulting positive counts are
// +8C/+90/+94 and steps +98/+9C/+A0. cellSize +88=48 is set by ctor AFTER its
// initialize callback; changing cell size after grid construction does not rebuild
// counts/steps. Re-run the grid builder as part of a future spacing mod.
// +74 speed=5.5 world units/s (ctor, kFloat_5_5_BlackSparrow); +70 turnRate rad/s
// caps ground-mode yaw. +11A movementEnabled is controlled by animation vfunc_27,
// not tested here: that dispatcher decides whether to invoke this method.
// Arrival radius is kFloat_1_Creature=1 world unit, independent of speed*dt.
// Recent-cell array +A8 contains recentCellCount(+A4) int32 cell IDs, newest first;
// require count>=1, each ID < XCount*YCount*ZCount and product fits signed int.
// Each candidate consumes exactly 3 rand calls; then target jitter consumes X,Z
// floats plus Y only in 3D. Ground mode retains target Y until terrain snap.
// Recorded vector +D0/+D4/+D8 owns native float XYZ records, stride 12. Vec3i and
// Copy801A below COPY FLOAT BITS; no integer coordinate conversion takes place.
// pathCursor +DC byte must address a <=255-point finite replay. Recording selects
// fresh targets regardless of cursor and can grow beyond that replayable limit.
// Growth phases: construct tail -> reverse-copy old prefix -> publish -> guard
// frees retired array. Length error precedes publication; allocation null unchecked.
// Target/history mutation happens BEFORE allocation, so failure is not transactional.
// The field-derived byte addresses below retain the current alias/load schedule;
void ntNpc_BlackSparrow_UpdateGridTargetAndMove(void *self, float dt) {
    char *s = (char *)self;
    unsigned int capacity;
    unsigned int needed, increment2, increment1;
    int neighborStep[3];
    Vec3f positionMinusTarget;

    __mi__FRC7WVectorRC7WVector(&positionMinusTarget, (s + kCreaturePositionOffset), (s + kCreatureTargetOffset));
    if (WVectorLen__FRC7WVector(&positionMinusTarget) < kFloat_1_Creature) {
        if (AsCCreature(s)->pathCursor >= ((CreatureGridPathView*)s)->pointCount || AsCCreature(s)->recordPath != 0) {
            int y, z, x;
            int selectionAttempts;
            int candidateCell, recentlyVisited;
            int historyCount;
            // Phase 1: choose a neighboring grid cell and publish its history entry.
            selectionAttempts = 0;
            do {
                selectionAttempts++;
                int *stepCursor = neighborStep;
                int i = 0;
                while (i < kGridAxisCount) {
                    *stepCursor = rand() % kNeighborStepChoices - 1;
                    stepCursor++;
                    i++;
                }
                int gridWidth = AsCCreature(s)->gridXCount;
                int gridDepth = AsCCreature(s)->gridZCount;
                int maxX = gridWidth - 1;
                int previousCell = (*(int**)(s + kRecentCellsOffset))[0];
                int layerCells = gridWidth * gridDepth;
                int previousY = previousCell / layerCells;
                int previousZ = (previousCell % layerCells) / gridWidth;
                x = neighborStep[0] + previousCell % gridWidth;
                if (x < 0) x = 0; else if (x > maxX) x = maxX;
                y = previousY + neighborStep[1];
                int maxY = AsCCreature(s)->gridYCount - 1;
                if (y < 0) y = 0; else if (y > maxY) y = maxY;
                z = previousZ + neighborStep[2];
                int maxZ = gridDepth - 1;
                if (z < 0) z = 0; else if (z > maxZ) z = maxZ;
                candidateCell = gridWidth * (z + y * gridDepth) + x;
                historyCount = (*(int*)(s + kRecentCellCountOffset));
                recentlyVisited = 0;
                int historyIndex = 0;
                int historyByteOffset = 0;
                while (historyIndex < historyCount) {
                    if (candidateCell == *(int *)((char *)(*(int**)(s + kRecentCellsOffset)) + historyByteOffset)) { recentlyVisited = 1; break; }
                    historyIndex++;
                    historyByteOffset += 4;
                }
            } while (recentlyVisited && selectionAttempts < kNeighborAttemptLimit);
            for (int j = (*(int*)(s + kRecentCellCountOffset)) - 1; j >= 1; j--)
                (*(int**)(s + kRecentCellsOffset))[j] = (*(int**)(s + kRecentCellsOffset))[j - 1];
            (*(int**)(s + kRecentCellsOffset))[0] = candidateCell;

            // Phase 2: jitter the target inside the selected world-space grid cell.
            AsCCreature(s)->targetPosition[0] = AsCCreature(s)->movementBoundsOrigin[0] + AsCCreature(s)->gridStepX * (float)x + Random_RangeFloat(-1, kFloat_0_Creature, AsCCreature(s)->gridStepX);
            if (AsCCreature(s)->gridYCount > 1) {
                AsCCreature(s)->targetPosition[1] = AsCCreature(s)->movementBoundsOrigin[1] + AsCCreature(s)->gridStepY * (float)y + Random_RangeFloat(-1, kFloat_0_Creature, AsCCreature(s)->gridStepY);
            }
            AsCCreature(s)->targetPosition[2] = AsCCreature(s)->movementBoundsOrigin[2] + AsCCreature(s)->gridStepZ * (float)z + Random_RangeFloat(-1, kFloat_0_Creature, AsCCreature(s)->gridStepZ);

            // Phase 3: append the chosen XYZ target or replay one existing target.
            if (AsCCreature(s)->recordPath != 0) {
                Alloc801A allocatorPush;
                unsigned int count = ((CreatureGridPathView*)s)->pointCount;
                unsigned int allocated = (*(unsigned int*)(s + kWaypointCapacityOffset));
                if (count < allocated) {
                    Vec3i *dst = (Vec3i *)(((CreatureGridPathView*)s)->pointsAddressBits + count * 12);
                    new (dst) Copy801A((*(Copy801A*)(s + kCreatureTargetOffset)));
                    ((CreatureGridPathView*)s)->pointCount++;
                } else {
                    Alloc801A allocatorInsert;
                    Alloc801A allocatorCheck;
                    if (kMaxWaypointCount - allocated < 1) lengthError801A();
                    needed = ((CreatureGridPathView*)s)->pointCount + 1 - (*(unsigned int*)(s + kWaypointCapacityOffset));
                    Grow801A grow(&(*(unsigned int*)(s + kWaypointCapacityOffset)));
                    capacity = (*(unsigned int*)(s + kWaypointCapacityOffset));
                    if (needed > kMaxWaypointCount - capacity) lengthError801A();
                    if (capacity < kWaypointThreeFifthsGrowthLimit) {
                        increment1 = ((capacity + 1) * 3) / 5;
                        capacity += max801A(increment1, needed);
                    } else if (capacity < kWaypointHalfGrowthLimit) {
                        increment2 = (capacity + 1) / 2;
                        capacity += max801A(increment2, needed);
                    } else capacity = kMaxWaypointCount;
                    Alloc801A allocatorAlloc;
                    if (capacity > kMaxWaypointCount) lengthError801A();
                    grow.data = (Vec3i *)appMallocDummy__FUl(capacity * 12);
                    unsigned int oldSize = ((CreatureGridPathView*)s)->pointCount;
                    grow.capacity = capacity;
                    grow.remaining = oldSize;
                    Vec3i *dst = grow.data + oldSize + grow.size;
                    Alloc801A allocatorCopy;
                    for (unsigned int n = 1; n; --n, ++dst) {
                        new (dst) Copy801A(*(Copy801A *)((s + kCreatureTargetOffset)));
                        ++grow.size;
                    }
                    Vec3i *begin = (Vec3i *)((CreatureGridPathView*)s)->pointsAddressBits;
                    Vec3i *end = begin + ((CreatureGridPathView*)s)->pointCount;
                    dst = grow.data + grow.remaining;
                    Alloc801A allocatorMove;
                    while (end > begin) {
                        --dst; --end;
                        new (dst) Copy801A(*(Copy801A *)end);
                        --grow.remaining;
                        ++grow.size;
                    }
                    Alloc801A allocatorClear;
                    ((CreatureGridPathView*)s)->pointCount = 0;
                    Vec3i *old = (Vec3i *)((CreatureGridPathView*)s)->pointsAddressBits;
                    unsigned int oldCapacity = (*(unsigned int*)(s + kWaypointCapacityOffset));
                    (*(unsigned int*)(s + kWaypointCapacityOffset)) = grow.capacity;
                    grow.capacity = oldCapacity;
                    ((CreatureGridPathView*)s)->pointsAddressBits = (unsigned int)grow.data;
                    grow.data = old;
                    ((CreatureGridPathView*)s)->pointCount = grow.size;
                    grow.size = 0;
                }
            }
        } else {
            __as__7WVectorFRC7WVector((s + kCreatureTargetOffset), (char *)(void*)((CreatureGridPathView*)s)->pointsAddressBits + AsCCreature(s)->pathCursor * 12);
            AsCCreature(s)->pathCursor = AsCCreature(s)->pathCursor + 1;
        }
    }
    // Phase 4: pursue in 3D or yaw/move forward and snap to the ground surface.
    if (AsCCreature(s)->gridYCount > 1) {
        Vec3f displacement, velocity, directionToTarget;
        __mi__FRC7WVectorRC7WVector(&directionToTarget, (s + kCreatureTargetOffset), (s + kCreaturePositionOffset));
        void *unitDirection = Normalize__7WVectorFv(&directionToTarget);
        __ml__FfRC7WVector(&velocity, unitDirection, AsCCreature(s)->movementSpeed);
        __ml__FRC7WVectorf(&displacement, &velocity, dt);
        __apl__7WVectorFRC7WVector((s + kCreaturePositionOffset), &displacement);
    } else {
        Vec3f d2, t2, t1;
        char comb[48], rot[48];
        __mi__FRC7WVectorRC7WVector(&d2, (s + kCreatureTargetOffset), (s + kCreaturePositionOffset));
        float ang = Vector_SignedAngleXZ(&d2, (s + kCreatureForwardOffset));
        float aa = ang > kFloat_0_Creature ? ang : -ang;
        if (aa > kFloat_0_017453292_Creature) {
            float lim = AsCCreature(s)->turnRate;
            float lo = -lim * dt;
            float hi = lim * dt;
            ang = ang < lo ? lo : (ang > hi ? hi : ang);
            char one = 1;
            Matrix_SetAxisRotation(rot, ang, one);
            Matrix_ConcatAffine(comb, rot, (s + kCreatureTransformOffset));
            Matrix_CopyAffine((s + kCreatureTransformOffset), comb);
        }
        float sc = AsCCreature(s)->movementSpeed;
        __ml__FfRC7WVector(&t1, (s + kCreatureForwardOffset), sc);
        __ml__FRC7WVectorf(&t2, &t1, dt);
        __apl__7WVectorFRC7WVector((s + kCreaturePositionOffset), &t2);
        Vec3f res;
        Vec3i vv = (*(Vec3i*)(s + kCreaturePositionOffset));
        void *rr = CQuadTree_GetInstance();
        int zero = 0;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&res, rr, &vv, 1, zero, zero);
        __as__7WVectorFRC7WVector((s + kCreaturePositionOffset), &res);
    }
}

#pragma opt_propagation reset

// Advance clips, alternate resting and moving actions, and interpolate one-shot opacity keys.
void ntNpc_BlackSparrow_UpdateAnimationAndMovement(void *self, float dt) {
      if ((*(void **)&AsCCreature(self)->puppet) == 0 || AsCCreature(self)->animationActive == 0) return;
      float limit = kFloat_0_Creature;
      float curTime = AsCCreature(self)->animationTime + dt;
    AsCCreature(self)->animationTime = curTime;
      if (limit > curTime) return;
    void *anim = (*(void **)&AsCCreature(self)->animationRange);
    AnimationEventParser_ProcessFrameRange((char *)self + 4, (*(void **)&AsCCreature(self)->puppet), curTime - dt, curTime, anim, -1, 0);
    anim = (*(void **)&AsCCreature(self)->animationRange);
      float end = *((float *)((char *)anim + 4));
      if (end < AsCCreature(self)->animationTime) {
        if (((*(unsigned int*)&AsCCreature(self)->flags) & 1) != 0) {
            AsCCreature(self)->animationActive = 0;
            AsCCreature(self)->animationTime = *(float *)anim;
        } else {
            AsCCreature(self)->animationTime = AsCCreature(self)->animationTime - (end - *(float *)anim);
            int tick = rand();
            int phase = tick - ((tick / 101) * 101);
            if (20 > phase) {
                void *parent = (*(void **)&AsCCreature(self)->puppet);
                const char *name = npcResourceStrings + kCreatureStrings_action01;
                if (parent) {
                    void *cur = (*(void **)&AsCCreature(self)->animationRange);
                    if (!cur || CString_Compare(*((void **)cur + 6), name) != 0) {
                        void *next = WPuppet_FindAnimationRangeByName(parent, name);
                        if (next != 0) {
                            (*(void **)&AsCCreature(self)->animationRange) = next;
                            float zero = kFloat_0_Creature;
                            float v = *(float *)next + zero;
                            float cap = *((float *)((char *)next + 4));
                            AsCCreature(self)->animationTime = v > cap ? cap : v;
                        } else AsCCreature(self)->animationTime = kFloat_0_Creature;
                    }
                }
                AsCCreature(self)->movementEnabled = 0;
            } else {
                void *parent = (*(void **)&AsCCreature(self)->puppet);
                const char *name = npcResourceStrings + kCreatureStrings_defaultAnimation;
                if (parent != 0) {
                    void *cur = (*(void **)&AsCCreature(self)->animationRange);
                    if (0 == cur || CString_Compare(*((void **)cur + 6), name) != 0) {
                        void *next = WPuppet_FindAnimationRangeByName(parent, name);
                        if (next != 0) {
                            (*(void **)&AsCCreature(self)->animationRange) = next;

                            float zero = kFloat_0_Creature;
                            float v = *(float *)next + zero;
                            float cap = *((float *)((char *)next + 4));
                            AsCCreature(self)->animationTime = v > cap ? cap : v;
                        } else AsCCreature(self)->animationTime = kFloat_0_Creature;
                    }
                }
                AsCCreature(self)->movementEnabled = 1;
            }
        }
    }
    if (((*(unsigned int*)&AsCCreature(self)->flags) & 1) != 0) {
        void *node = AsCCreature(self)->opacityKeyframePrevious;
          void *head = ((char*)&AsCCreature(self)->opacityKeyframeNext);
        while (head != node) {
            void *prev = node;
            node = *((void **)node + 1);
            if (head == node) {
                AsCCreature(self)->opacity = (unsigned char)(int)(*((float *)prev + 2));
                break;
            }
            float t = AsCCreature(self)->animationTime;
            float a = *((float *)prev + 3); float b = *((float *)node + 3);
            if (t > a && b > t) {
                float ratio = (t - a) / (b - a);

                AsCCreature(self)->opacity = (unsigned char)(int)((1.0f - ratio) * (*((float *)prev + 2)) + ratio * (*((float *)node + 2)));
                return;
            }
        }
    } else if (AsCCreature(self)->movementEnabled != 0) {
        void **vtable = *(void ***)self;
        ((void (*)(void *, float))vtable[0x90 / 4])(self, dt);
    }
}

// Initialize ntNpc_BlackSparrow state and owned resources.
void *ntNpc_BlackSparrow_Ctor(void *self, void *arg) {
    const char* name;
    void* selected;
    struct VirtualObject {
#define V(n) virtual void slot##n() DECOMP_LOCAL_VIRTUAL;
    V(0) V(1) V(2) V(3) V(4) V(5) V(6) V(7) V(8) V(9)
    V(10) V(11) V(12) V(13) V(14) V(15) V(16) V(17) V(18) V(19)
    V(20) V(21) V(22) V(23) V(24) V(25) V(26) V(27) V(28) V(29)
    V(30) V(31) V(32) V(33) V(34) V(35)
#undef V
    virtual void initialize(void*) DECOMP_LOCAL_VIRTUAL;
    };
    *(void *volatile*)self = __vt__9CCreature;
    *((void *volatile*)self + 1) = __vt__9CCreature + 0x0C;
    *((volatile unsigned int *)self + 2) = 0;
    Vec3_Zero(((char*)&AsCCreature(self)->movementBoundsOrigin[0]));
    Vec3_Zero(((char*)&AsCCreature(self)->movementBoundsMax[0]));
    ns3::InitObject* object = (ns3::InitObject*)self;
    object->puppet = 0;
    object->animationFrame = kFloat_0_Creature;
    object->animationRange = 0;
    object->heading = kFloat_0_Creature;
    object->history = 0;
    object->ballSpawnOffsetDistance = kFloat_0_Creature;
    object->opacityKeyframeCount = 0;
    object->tail = ((char*)&AsCCreature(self)->opacityKeyframeNext);
    object->head = ((char*)&AsCCreature(self)->opacityKeyframeNext);
    object->targetPoints = 0;
    object->targetPointCount = 0;
    object->targetPointCapacity = 0;
    object->pathPoints = 0;
    object->pathPointCount = 0;
    object->pathPointCapacity = 0;
    object->vtable = __vt__18ntNpc_BlackSparrow;
    object->vtableEnd = __vt__18ntNpc_BlackSparrow + 12;
    object->movementSpeed = kFloat_5_5_BlackSparrow;
    try {
    ((VirtualObject*)self)->initialize(arg);
    selected = (*(void **)&AsCCreature(self)->puppet);
    object->maxBankAngle = kFloat_0_7853982_Creature;
    object->cellSize = ns3::kFloat_48_Creature;
    name = npcResourceStrings + kCreatureStrings_defaultAnimation;
    if (selected != 0) {
        void *current = (*(void **)&AsCCreature(self)->animationRange);
        if (current == 0 || CString_Compare(*((void **)current + 6), name) != 0) {
            void *result = WPuppet_FindAnimationRangeByName(selected, name);
            if (result != 0) {
                (*(void **)&AsCCreature(self)->animationRange) = result;
                float f0 = kFloat_0_Creature;
                float f1 = *(float *)result;
                float f2 = *((float *)((char *)result + 4));
                f0 = f1 + f0;
                f2 = (f0 > f2) ? f2 : f0;
                AsCCreature(self)->animationTime = f2;
            } else {
                AsCCreature(self)->animationTime = kFloat_0_Creature;
            }
        }
    }
    AsCCreature(self)->movementEnabled = 1;
    return self;
    } catch (...) {
        CCreature_dtor(self, -1);
        throw;
    }
}

#pragma opt_common_subs off
// Draw the animated sparrow with bank angle, opacity and optional debug bounds and direction.
void ntNpc_BlackSparrow_Draw(ns3::AnimationObject *self) {
    struct Resource {
        virtual void slot0() DECOMP_LOCAL_VIRTUAL;
        virtual void slot1() DECOMP_LOCAL_VIRTUAL;
        virtual void slot2() DECOMP_LOCAL_VIRTUAL;
        virtual void name(const char*, int, int) DECOMP_LOCAL_VIRTUAL;
        virtual void draw(void*, int, int, int, float, int) DECOMP_LOCAL_VIRTUAL;
    };
    if ((*(void **)&AsCCreature(self)->puppet) == 0 || AsCCreature(self)->animationActive == 0 || AsCCreature(self)->animationTime < kFloat_0_Creature) return;
    if (((*(unsigned int*)&AsCCreature(self)->flags) & 1) != 0)
        WPuppet_SetOpacityRecursive((*(void **)&AsCCreature(self)->puppet), AsCCreature(self)->opacity);
    if (g_drawSceneDebugGeometry != 0) {
        WView_DrawFacingCircleVec4(g_view, ((char*)&AsWPuppet(AsCCreature(self)->puppet)->boundsCenter), 0xFF00FF00, 0x14, 0);
        if (((*(unsigned int*)&AsCCreature(self)->flags) & 1) == 0) {
            float scale = 5.0f;
            unsigned char pos[12];

            __ml__FfRC7WVector(pos, ((char*)&AsCCreature(self)->transform[6]), scale);
            WVector transformed = (*(WVector*)&AsCCreature(self)->transform[9]) - *(WVector*)pos;
            WView_DrawLine(g_view, ((char*)&AsCCreature(self)->transform[9]), -256, &transformed, -256, 0);
            WView_DrawLine(g_view, ((char*)&AsCCreature(self)->transform[9]), -65536, ((char*)&AsCCreature(self)->targetPosition[0]), -65536, 0);
        }
    }
    WPuppet_SetAnimationTimeFromRanges((*(void **)&AsCCreature(self)->puppet), 0, AsCCreature(self)->animationTime, 0, 0);
    if (self->check(((char*)&AsCCreature(self)->transform[9]), kFloat_0_8_805DF2D8) != 0) return;
    if (self->rightScale > kFloat_1_Creature) self->rightScale = kFloat_1_Creature;
    if (self->upScale > kFloat_1_Creature) self->upScale = kFloat_1_Creature;
    if (self->forwardScale > kFloat_1_Creature) self->forwardScale = kFloat_1_Creature;
    float angle = AsCCreature(self)->bankAngle;
    angle = (angle > kFloat_0_Creature) ? angle : -angle;
    float temp[12], combined[12];
    void *matrix;
    if (angle > 0.0f) {
        Matrix_SetAxisRotation(temp, AsCCreature(self)->bankAngle, 2);
        Matrix_ConcatAffine(combined, temp, ((char*)&AsCCreature(self)->transform[0]));
        matrix = combined;
    } else matrix = ((char*)&AsCCreature(self)->transform[0]);
    void *object = (*(void **)&AsCCreature(self)->puppet);
    ns3::Model_SetMatrixRecursive(object, object, matrix);
    WPuppet_UpdateBoneWorldMatrices((*(void **)&AsCCreature(self)->puppet), g_view, 0, 1.0f);
    WPuppet_UpdateBoundsFromBone((*(void **)&AsCCreature(self)->puppet));
    ((Resource*)self->resource)->name(s_sceneLighting, 0, 0);
    ((Resource*)self->resource)->draw(g_view, 1, 1, 1, kFloat_1_Creature, 0);
}
#pragma opt_common_subs reset

// Release ntNpc_BlackSparrow resources and optionally delete the object.
void *ntNpc_BlackSparrow_Dtor(void *self, int doDelete) {
    if (self != 0) {
        CCreature_dtor(self, 0);
        if (doDelete > 0) __dl__FPv(self);
    }
    return self;
}

// Initialize ntNpc_PolaShip state and owned resources.
void *ntNpc_PolaShip_Ctor(void *self, int arg, float f1) {
    AsCCreature(self)->vtable = (void *)__vt__9CCreature;
    AsCCreature(self)->secondaryVtable = (void *)(__vt__9CCreature + 0x0C);
    (*(unsigned int*)&AsCCreature(self)->flags) = 0;
    Vec3_Zero(((char*)&AsCCreature(self)->movementBoundsOrigin[0]));
    Vec3_Zero(((char*)&AsCCreature(self)->movementBoundsMax[0]));
    float zero = kFloat_0_Creature;

    (*(unsigned int*)&AsCCreature(self)->puppet) = 0;
    AsCCreature(self)->animationTime = zero;
    (*(unsigned int*)&AsCCreature(self)->animationRange) = 0;
    AsCCreature(self)->heading = zero;
    (*(unsigned int*)&AsCCreature(self)->history) = 0;
    AsCCreature(self)->ballSpawnOffsetDistance = zero;
    (*(unsigned int*)&AsCCreature(self)->opacityKeyframeCount) = 0;
    void *r6 = ((char*)&AsCCreature(self)->opacityKeyframeNext); AsCCreature(self)->opacityKeyframePrevious = r6;
    AsCCreature(self)->opacityKeyframeNext = r6;
    (*(unsigned int*)&AsCCreature(self)->targetPoints) = 0;
    (*(unsigned int*)&AsCCreature(self)->targetPointCount) = 0;
    (*(unsigned int*)&AsCCreature(self)->targetPointCapacity) = 0;
    (*(unsigned int*)&AsCCreature(self)->pathPoints) = 0;
    (*(unsigned int*)&AsCCreature(self)->pathPointCount) = 0;
    (*(unsigned int*)&AsCCreature(self)->pathPointCapacity) = 0;
    AsCCreature(self)->vtable = (void *)__vt__14ntNpc_PolaShip;
    AsCCreature(self)->secondaryVtable = (void *)(__vt__14ntNpc_PolaShip + 0x0C);
    (*(unsigned int*)&AsShip(self)->byteBuffer) = 0;
    AsShip(self)->byteCapacity = 0;
    AsShip(self)->splinePoints = 0;
    AsShip(self)->splineDistances = 0;
    AsShip(self)->splineCoefficients = 0;
    AsShip(self)->splineLength = zero;
    AsShip(self)->splinePointCount = 0;
    (*(unsigned int*)&AsShip(self)->byteCountBits) = 0;
    AsShip(self)->bufferControlByte = 0;
    ((void (*)(void *, int))((*(void ***)self)[0x98 / 4]))(self, arg);
    AsShip(self)->pathDistance = kFloat_0_Creature;
    *(unsigned char *)(((char*)&AsShip(self)->delayActiveBits)) = 1;
    AsShip(self)->spawnDelay = f1;
    return self;
}

#pragma opt_common_subs off
// Release ntNpc_PolaShip resources and optionally delete the object.
void *ntNpc_PolaShip_Dtor(void *self, int doDelete) {
    if (self != 0) {
        if (((char*)&AsShip(self)->splinePoints)) {
        void *p = (*(void **)&AsShip(self)->splinePoints);
        if (p != 0) {
            if (p != 0) __dla__FPv((char *)p - 0x10);
            (*(void **)&AsShip(self)->splinePoints) = 0;
        }
        p = (*(void **)&AsShip(self)->splineDistances);
        if (p != 0) {
            __dla__FPv(p);
            (*(void **)&AsShip(self)->splineDistances) = 0;
        }
        p = (*(void **)&AsShip(self)->splineCoefficients);
        if (p != 0) {
            __dla__FPv(p);
            (*(void **)&AsShip(self)->splineCoefficients) = 0;
        }
        float zero = kFloat_0_Creature;
        AsShip(self)->splinePointCount = 0;
        AsShip(self)->splineLength = zero;
        }
        if (((char*)&AsShip(self)->byteBuffer) != 0) {
          if (((char*)&AsShip(self)->byteBuffer) != 0) {
           if (((char*)&AsShip(self)->byteBuffer) != 0) {
            if (((char*)&AsShip(self)->byteBuffer) != 0) {
             if (((char*)&AsShip(self)->byteBuffer) != 0) {
              void *p = AsShip(self)->byteBuffer;
              if (p != 0) {
                char* end = (char*)p + (*(unsigned int*)&AsShip(self)->byteCountBits);
                (*(unsigned int*)&AsShip(self)->byteCountBits) -= end - (char*)p;
                appFreeUni__FPv(p);
              }
             }
            }
           }
          }
        }
        CCreature_dtor(self, 0);
        if (doDelete > 0) __dl__FPv(self);
    }
    return self;
}
#pragma opt_common_subs reset

} // extern "C"

#undef THROW_LENGTH_ERROR
} // namespace ns5

