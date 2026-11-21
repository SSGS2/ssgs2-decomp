#include <decomp/offset_of.h>
#include "MSL/list.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
typedef std::list<void*, app::app_allocator<void*> > MSLPointerList;
#include "classes/CCreature.h"
// Keep this cast as an expression: an inline call changes CodeWarrior operand selection.
#define AsCCreature(p) ((CCreature*)(p))
#include "classes/CDisposable.h"
// View the existing storage as CDisposable without transferring ownership.
static inline CDisposable* AsCDisposable(const volatile void* p) { return (CDisposable*)p; }
#include "game/wvector.h"
// Translation unit 0x801A51D0-0x801A6854 (14 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// Sparse view of the existing byte pool; resource bytes and storage are unchanged.
struct NpcResourcePool {
    char beforeDefaultAnimation[0x11E];
    char defaultAnimation[12];
    char opacityDirective[8];
    char effectDirective[4];
    char effectScanFormat[9];
    char sprayExtension[4];
    char sequenceExtension[4];
    char beforeWalkAnimation[0x32];
    char walkAnimation[9];
    char beforeShadowBitmap[0x81];
    char shadowBitmap[12];
    char beforeNpcClassName[0x79];
    char npcClassName[5];
};
// Animation range endpoints and the creature's +0x58 cursor use the same time unit.
// Updates add dt directly; root-motion calls multiply by kCreatureAnimationFramesPerSecond.
// The shared header already calls +0x58 animationTime; older local views use animationFrame.
struct CreatureAnimationRange {
    float startTime, endTime;
    char reserved[0x10];
    void* name;
};
struct PenguinShadowView { char prefix[0x11C]; void* bitmap; };
struct CreatureEffectModel {
    char prefix[0x74];
    unsigned int flags;
    char attachmentTransform[1];
};
struct CreatureEffectResource {
    char prefix[0x18];
    char attachmentContext[0x10];
    void* text;
};
struct CreatureEffectEvent { CreatureEffectModel* model; CreatureEffectResource* resource; };
struct CreatureEffectAttachment {
    char prefix[8]; void* transform;
    char reserved[0xC]; void* context;
};
struct CreatureSplineStorage {
    void* points; float* parameters; float* segmentLengths;
    float totalLength; int pointCount;
};
enum CreatureVtableLayout { kSecondaryCreatureVtable = 3 * sizeof(void*) };
enum CreatureFlags { kCreatureDisposable = 1, kCreatureYAxis = 1, kEffectModelDisabled = 0x40000 };
enum CreatureDrawing { kDebugCircleGreen = 0xFF00FF00, kDebugHeadingYellow = 0xFFFFFF00,
    kDebugTargetRed = 0xFFFF0000, kDebugCircleSegments = 20, kOpacityOpaque = 255 };

// Keep byte-pointer address expressions where typed array addresses extend register lifetimes.
// Every address is derived from the checked shared layout instead of a numeric offset.
enum CreatureMemberAddresses {
    kCreatureBoundsMinimum = (int)DECOMP_OFFSETOF(CCreature, movementBoundsOrigin),
    kCreatureBoundsMaximum = (int)DECOMP_OFFSETOF(CCreature, movementBoundsMax),
    kCreatureTransform = (int)DECOMP_OFFSETOF(CCreature, transform),
    kCreatureUpAxis = (int)DECOMP_OFFSETOF(CCreature, transform[3]),
    kCreatureForwardAxis = (int)DECOMP_OFFSETOF(CCreature, transform[6]),
    kCreatureTranslation = (int)DECOMP_OFFSETOF(CCreature, transform[9]),
    kCreatureTargetPosition = (int)DECOMP_OFFSETOF(CCreature, targetPosition),
    kSequenceHandleCount = (int)DECOMP_OFFSETOF(CDisposable, sequenceHandleCount),
    kSprayHandleCount = (int)DECOMP_OFFSETOF(CDisposable, sprayHandleCount),
};

extern "C" {

// Report that disposable creatures do not provide the optional capability.
int CDisposable_vfunc_35() {
    return 0;
}

void CCreature_dtor(void*, int);
void __dl__FPv(void*);
void __dla__FPv(void*);
void appFreeUni__FPv(void*);
void CFx_RemoveSequence(void*, void*, int);
void CFx_RemoveSpray(void*, void*);
extern void* g_fx;
extern const float kFloat_0_Creature;
extern const float kFloat_2_805DF2B0;
extern const float kFloat_1_Creature;
extern const float kFloat_neg1_CreatureDirection;
extern const float kFloat_5_creature;
extern const float kFloat_0_8_805DF2D8;
extern const float kFloat_neg3_1415927_Creature;
extern const float kFloat_3_1415927_805DF2E4;
extern const float kFloat_48_Creature;
extern char __vt__9CCreature[];
extern char __vt__11CDisposable[];
extern char __vt__18ntNpc_PolaPenguin2[];
extern const char npcResourceStrings[];
extern void* g_resourceManager;

void Vec3_Zero(void*);
void WPuppet_SetRendererOption(void*, int);
void appFreeUni__FPv(void*);
char* strstr(const char*, const char*);
int sscanf(const char*, const char*, ...);
char* strchr(const char*, int);
unsigned int strlen(const char*);
int String_CompareCString(const char*, const char*);
void FxAttachment_Init(void*);
void* CFx_CreateAttachedEffect(void*, void*, void*, int, int, int, void*);
void* appMallocDummy__FUl(unsigned int);
void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);
void* WPuppet_FindAnimationRangeByName(void*, const char*);
void* CString_Compare(void*, const char*);
float Random_RangeFloat(int, float, float);
void Matrix_SetAxisRotation(void*, float, int);
void Matrix_CopyAffine(void*, void*);
void __ml__FRC7WVectorf(void*, void*, float);
void __ml__FfRC7WVector(void*, void*, float);
void* GolfBall__Fv();
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void* CQuadTree_GetInstance();
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
void __as__7WVectorFRC7WVector(void*, void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float Vector_SignedAngleXZ(void*, void*);
float Magnitude__7WVectorCFv(void*);
float WVectorLen__FRC7WVector(void*);
void WPuppet_GetInverseRootMotionOffsetForRange(void*, void*, float, void*);
void WPuppet_GetRootMotionDelta(void*, float, void*, float, void*);
void Matrix_TransformDirection(void*, void*, void*);
void __apl__7WVectorFRC7WVector(void*, void*);
void* AnimationEventParser_ProcessFrameRange(void*, void*, float, float, void*, int, int);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
extern const float kFloat_0_017453292_Creature;
extern const float kCreatureAnimationFramesPerSecond;
extern const float kFloat_3_2_CourseGeometry;
void* Normalize__7WVectorFv(void*);
void Matrix_ConcatAffine(void*, void*, void*);
void Vector_CrossProduct(void*, void*, void*);

struct ConstructorObj {
    void* vtable;
    void* vtable2;
    int flags;
    char pad0C[0x0C];
    char sub18[0x0C];
    char pad24[0x34];
    float field58;
};

struct ConstructorVirtual {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24(); virtual void v25(); virtual void v26(); virtual void v27();
    virtual void v28(); virtual void v29(); virtual void v30(); virtual void v31();
    virtual void v32(); virtual void v33(); virtual void v34(); virtual void v35();
    virtual void v36(void*);
};

// Initialize a walking polar penguin, its shadow resource, and initial action range.
void* ntNpc_PolaPenguin2_Ctor(void* self, void* definition) {
    char* p = (char*)self;
    int zero = 0;
    *(void* volatile*)(((char*)&AsCCreature(p)->vtable)) = (char*)__vt__9CCreature;
    *(void* volatile*)(((char*)&AsCCreature(p)->secondaryVtable)) = (char*)__vt__9CCreature + kSecondaryCreatureVtable;
    (*(volatile int*)&AsCCreature(p)->flags) = zero;
    Vec3_Zero((p + kCreatureBoundsMinimum));
    Vec3_Zero((p + kCreatureBoundsMaximum));
    AsCCreature(p)->puppet = zero;
    AsCCreature(p)->animationTime = kFloat_0_Creature;
    AsCCreature(p)->animationRange = zero;
    AsCCreature(p)->heading = kFloat_0_Creature;
    AsCCreature(p)->history = zero;
    AsCCreature(p)->ballSpawnOffsetDistance = kFloat_0_Creature;
    AsCCreature(p)->opacityKeyframeCount = zero;
    AsCCreature(p)->opacityKeyframePrevious = ((char*)&AsCCreature(p)->opacityKeyframeNext);
    AsCCreature(p)->opacityKeyframeNext = ((char*)&AsCCreature(p)->opacityKeyframeNext);
    AsCCreature(p)->targetPoints = zero;
    AsCCreature(p)->targetPointCount = zero;
    AsCCreature(p)->targetPointCapacity = zero;
    AsCCreature(p)->pathPoints = zero;
    AsCCreature(p)->pathPointCount = zero;
    AsCCreature(p)->pathPointCapacity = zero;
    AsCCreature(p)->vtable = (char*)__vt__18ntNpc_PolaPenguin2;
    AsCCreature(p)->secondaryVtable = (char*)__vt__18ntNpc_PolaPenguin2 + kSecondaryCreatureVtable;
    AsCCreature(p)->movementSpeed = kFloat_2_805DF2B0;
    try {
    ((ConstructorVirtual*)self)->v36(definition);
    (*(char*)&AsCCreature(p)->movementEnabled) = 1;
    AsCCreature(p)->cellSize = kFloat_48_Creature;
    ((PenguinShadowView*)p)->bitmap = ResourceManager_LoadBitmap(g_resourceManager, ((const NpcResourcePool*)(char*)npcResourceStrings)->shadowBitmap, 0, 0, 0);
    void* puppet = (*(void**)&AsCCreature(p)->puppet);
    const char* name = ((const NpcResourcePool*)(char*)npcResourceStrings)->walkAnimation;
    if (puppet != 0) {
        void* currentRange = (*(void**)&AsCCreature(p)->animationRange);
        if (currentRange == 0 || CString_Compare(((CreatureAnimationRange*)currentRange)->name, name) != 0) {
            void* requestedRange = WPuppet_FindAnimationRangeByName(puppet, name);
            if (requestedRange != 0) {
                (*(void**)&AsCCreature(p)->animationRange) = requestedRange;
                float timeOffset = kFloat_0_Creature;
                float rangeStartTime = ((CreatureAnimationRange*)requestedRange)->startTime;
                float initialTime = rangeStartTime + timeOffset;
                float rangeEndTime = ((CreatureAnimationRange*)requestedRange)->endTime;
                AsCCreature(p)->animationTime = initialTime > rangeEndTime ? rangeEndTime : initialTime;
            } else {
                AsCCreature(p)->animationTime = kFloat_0_Creature;
            }
        }
    }
    return self;
    } catch (...) {
        CCreature_dtor(self, -1);
        throw;
    }
}

// Spawn a disposable creature near the ball, orient toward its path, and snap it to ground.
// Requires at least two targetPoints. The ball-relative candidate is computed first,
// but the final translation is the terrain projection of targetPoints[0].
// transform holds right/up/forward axes followed by world translation (floats 9..11).
void CDisposable_InitAtPathStart(void* self) {
    union RotationBuffer { double alignment; float matrix[12]; } rotation;
    char forwardDirection[0x0C];
    char pathDifference[0x0C];
    char worldUp[0x0C];
    char rightAxis[0x0C];
    char pathGroundPoint[0x0C];
    char pathStart[0x0C];
    char reverseForward[0x0C];
    char ballOffset[0x0C];
    char spawnCandidate[0x0C];
    char* p = (char*)self;
    char spawnCoordinates[0x0C];
    char spawnGroundPoint[0x0C];
    float zeroAnimationTime = kFloat_0_Creature;
    float minimumHeading = kFloat_neg3_1415927_Creature;
    float maximumHeading = kFloat_3_1415927_805DF2E4;
    unsigned char animationEnabled = 1;
    int fullOpacity = kOpacityOpaque;
    AsCCreature(p)->animationActive = animationEnabled;
    AsCCreature(p)->animationTime = zeroAnimationTime;
    AsCCreature(p)->opacity = (unsigned char)fullOpacity;
    float angle = Random_RangeFloat(-1, minimumHeading, maximumHeading);
    Matrix_SetAxisRotation(rotation.matrix, angle, kCreatureYAxis);
    Matrix_CopyAffine((p + kCreatureTransform), rotation.matrix);
    __ml__FRC7WVectorf(reverseForward, (p + kCreatureForwardAxis), kFloat_neg1_CreatureDirection);
    __ml__FfRC7WVector(ballOffset, reverseForward, AsCCreature(p)->ballSpawnOffsetDistance);
    void* ballPosition = GolfBall__Fv();
    __pl__FRC7WVectorRC7WVector(spawnCandidate, ballPosition, ballOffset);
    *(void**)spawnCoordinates = *(void**)spawnCandidate;
    *((void**)spawnCoordinates + 1) = *((void**)spawnCandidate + 1);
    *((void**)spawnCoordinates + 2) = *((void**)spawnCandidate + 2);
    void* terrain = CQuadTree_GetInstance();
    GetGroundPoint__9CQuadTreeF7WVectorbPib(spawnGroundPoint, terrain, spawnCoordinates, 0, 0, 1);
    __as__7WVectorFRC7WVector((p + kCreatureTranslation), spawnGroundPoint);
    // Rebuild orientation from the first path segment in world XZ, then place at its start.
    __mi__FRC7WVectorRC7WVector(pathDifference, (*(void**)&AsCCreature(p)->targetPoints), ((WVector*)AsCCreature(p)->targetPoints) + 1);
    __ml__FfRC7WVector(forwardDirection, pathDifference, kFloat_neg1_CreatureDirection);
    __as__7WVectorFRC7WVector((p + kCreatureForwardAxis), forwardDirection);
    AsCCreature(p)->transform[7] = kFloat_0_Creature;
    Normalize__7WVectorFv((p + kCreatureForwardAxis));
    float zeroComponent = kFloat_0_Creature;

    *(float*)worldUp = zeroComponent;
    *((float*)worldUp + 1) = (kFloat_1_Creature);
    *((float*)worldUp + 2) = zeroComponent;
    __as__7WVectorFRC7WVector((p + kCreatureUpAxis), worldUp);
    Vector_CrossProduct(rightAxis, (p + kCreatureUpAxis), (p + kCreatureForwardAxis));
    __as__7WVectorFRC7WVector((p + kCreatureTransform), rightAxis);
    void* pathPoints = (*(void**)&AsCCreature(p)->targetPoints);
    int pathX = *(int*)pathPoints;
    int pathY = *((int*)pathPoints + 1);
    *(int*)pathStart = pathX;
    *((int*)pathStart + 1) = pathY;
    *((int*)pathStart + 2) = *((int*)pathPoints + 2);
    terrain = CQuadTree_GetInstance();
    GetGroundPoint__9CQuadTreeF7WVectorbPib(pathGroundPoint, terrain, pathStart, 1, 0, 1);
    __as__7WVectorFRC7WVector((p + kCreatureTranslation), pathGroundPoint);
}

// Handle lists have an embedded sentinel after count; real nodes add a handle at +8.
// Empty next/prev point to the sentinel. Nodes are owned here; effect lifetimes belong
// to g_fx and require CFx_RemoveSequence/CFx_RemoveSpray before node-only destruction.
struct ListNode { ListNode* next; ListNode* prev; };
struct EffectHandleNode { ListNode links; void* handle; };
struct EffectHandleList { int count; ListNode sentinel; };

struct UpdateVirtual {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24(); virtual void v25(); virtual void v26(); virtual void v27();
    virtual void v28(); virtual void v29(); virtual void v30(); virtual void v31();
    virtual void v32(); virtual void v33(); virtual void update(float);
};

// Advance animation events, choose the default/walk range, and apply completed root motion.
void ntNpc_PolaPenguin2_UpdateAnimationAndMovement(void* self, float dt) {
    // 12-byte vector scratch carries XYZ float bits through the WVector ABI.
    // Separate buffers preserve each local/world transform result across calls.
    char motionDelta[12];
    char rangeMotion[12];
    char worldMotion[12];
    char groundPoint[12];
    char currentPosition[12];
    char defaultRangeMotion[12];
    char targetDelta[12];
    char targetDirection[12];
    char defaultRangeStep[12];
    char* p = (char*)self;
    // Transition table at range end:
    // disposable -> deactivate, reset time to start;
    // walking + aligned + default exists + target far enough -> default, stop walking;
    // otherwise -> commit completed root motion, select walk, enable walking.
    // Range selection retains an already-selected range/time; a missing range only zeros time.
    // Phase 1: advance the range cursor and dispatch events over the elapsed interval.
    void* puppet = (*(void**)&AsCCreature(p)->puppet);
    if (puppet == 0 || AsCCreature(p)->animationActive == 0) return;
    float time = AsCCreature(p)->animationTime + dt;
    AsCCreature(p)->animationTime = time;
    if (time < kFloat_0_Creature) return;
    {
        AnimationEventParser_ProcessFrameRange(((char*)&AsCCreature(p)->secondaryVtable), puppet, time - dt, time, (*(void**)&AsCCreature(p)->animationRange), -1, 0);
        void* animation = (*(void**)&AsCCreature(p)->animationRange);
        float end = ((CreatureAnimationRange*)animation)->endTime;
        float current = AsCCreature(p)->animationTime;
        // Phase 2: stop one-shot creatures or wrap once and choose the next range.
        if (current > end) {
            if (AsCCreature(p)->flags & kCreatureDisposable) {
                AsCCreature(p)->animationActive = 0;
                AsCCreature(p)->animationTime = ((CreatureAnimationRange*)animation)->startTime;
            } else {
                // Switching to the default range skips the completed walk-motion branch below.
                do {
                    AsCCreature(p)->animationTime = current - (end - ((CreatureAnimationRange*)animation)->startTime);
                    if (AsCCreature(p)->movementEnabled == 1) {
                        __mi__FRC7WVectorRC7WVector(targetDirection, (p + kCreatureTargetPosition), (p + kCreatureTranslation));
                        float angle = Vector_SignedAngleXZ(targetDirection, (p + kCreatureForwardAxis));
                        angle = angle > kFloat_0_Creature ? angle : -angle;
                        unsigned int canUseDefaultRange;
                        if (angle > kFloat_0_017453292_Creature) {
                            canUseDefaultRange = 0;
                        } else {
                            const char* base = npcResourceStrings;
                            void* defaultRange = WPuppet_FindAnimationRangeByName((*(void**)&AsCCreature(p)->puppet), ((const NpcResourcePool*)base)->defaultAnimation);
                            if (defaultRange == 0) {
                                canUseDefaultRange = 0;
                            } else {
                                __mi__FRC7WVectorRC7WVector(targetDelta, (p + kCreatureTargetPosition), (p + kCreatureTranslation));
                                float distance = WVectorLen__FRC7WVector(targetDelta);
                                WPuppet_GetInverseRootMotionOffsetForRange(defaultRangeMotion, (*(void**)&AsCCreature(p)->puppet), ((CreatureAnimationRange*)defaultRange)->endTime, (void*)(((const NpcResourcePool*)base)->defaultAnimation));
                                __as__7WVectorFRC7WVector(defaultRangeStep, defaultRangeMotion);
                                canUseDefaultRange = !(distance < Magnitude__7WVectorCFv(defaultRangeStep));
                            }
                        }
                        if (canUseDefaultRange == 1) {
                            void* puppetForRange = (*(void**)&AsCCreature(p)->puppet);
                            const char* name = ((const NpcResourcePool*)npcResourceStrings)->defaultAnimation;
                            if (puppetForRange != 0) {
                                void* currentRange = (*(void**)&AsCCreature(p)->animationRange);
                                if (currentRange == 0 || CString_Compare(((CreatureAnimationRange*)currentRange)->name, name) != 0) {
                                    void* nextRange = WPuppet_FindAnimationRangeByName(puppetForRange, name);
                                    if (nextRange != 0) {
                                        (*(void**)&AsCCreature(p)->animationRange) = nextRange;
                                        float timeOffset = kFloat_0_Creature;
                                        float rangeStart = ((CreatureAnimationRange*)nextRange)->startTime;
                                        float rangeEnd = ((CreatureAnimationRange*)nextRange)->endTime;
                                        float initialTime = rangeStart + timeOffset;
                                        AsCCreature(p)->animationTime = initialTime > rangeEnd ? rangeEnd : initialTime;
                                    } else {
                                        AsCCreature(p)->animationTime = kFloat_0_Creature;
                                    }
                                }
                            }
                            AsCCreature(p)->movementEnabled = 0;
                            break;
                        }
                    }
                    {
                        void* completedRange = (*(void**)&AsCCreature(p)->animationRange);
                        // Convert completed local root motion to world space before terrain projection.
                        WPuppet_GetRootMotionDelta(rangeMotion, kCreatureAnimationFramesPerSecond * ((CreatureAnimationRange*)completedRange)->endTime, (*(void**)&AsCCreature(p)->puppet), kCreatureAnimationFramesPerSecond * ((CreatureAnimationRange*)completedRange)->startTime, ((CreatureAnimationRange*)completedRange)->name);
                        __as__7WVectorFRC7WVector(motionDelta, rangeMotion);
                        Matrix_TransformDirection(worldMotion, motionDelta, (p + kCreatureTransform));
                        __as__7WVectorFRC7WVector(motionDelta, worldMotion);
                        __apl__7WVectorFRC7WVector((p + kCreatureTranslation), motionDelta);
                        *(Arr3*)currentPosition = (*(Arr3*)(p + kCreatureTranslation));
                        void* terrain = CQuadTree_GetInstance();
                        GetGroundPoint__9CQuadTreeF7WVectorbPib(groundPoint, terrain, currentPosition, 1, 0, 0);
                        __as__7WVectorFRC7WVector((p + kCreatureTranslation), groundPoint);
                        void* puppetForRange = (*(void**)&AsCCreature(p)->puppet);
                        const char* name = ((const NpcResourcePool*)npcResourceStrings)->walkAnimation;
                        if (puppetForRange != 0) {
                            void* currentRange = (*(void**)&AsCCreature(p)->animationRange);
                            if (currentRange == 0 || CString_Compare(((CreatureAnimationRange*)currentRange)->name, name) != 0) {
                                void* nextRange = WPuppet_FindAnimationRangeByName(puppetForRange, name);
                                if (nextRange != 0) {
                                    (*(void**)&AsCCreature(p)->animationRange) = nextRange;
                                    float timeOffset = kFloat_0_Creature;
                                    float rangeStart = ((CreatureAnimationRange*)nextRange)->startTime;
                                    float rangeEnd = ((CreatureAnimationRange*)nextRange)->endTime;
                                    float initialTime = rangeStart + timeOffset;
                                    AsCCreature(p)->animationTime = initialTime > rangeEnd ? rangeEnd : initialTime;
                                } else {
                                    AsCCreature(p)->animationTime = kFloat_0_Creature;
                                }
                            }
                        }
                        AsCCreature(p)->movementEnabled = 1;
                    }
                } while (0);
            }
        }
        // Phase 3: dispatch motion after event handling and range selection.
        ((UpdateVirtual*)self)->update(dt);
    }
}

struct CreatureVirtual {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14();
    virtual void v15(); virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24();
    virtual void v25(); virtual void v26(); virtual void v27(); virtual void v28(); virtual void v29();
    virtual void v30(); virtual void v31(); virtual void v32(); virtual void v33(); virtual void v34();
    virtual void v35(); virtual void v36(void*); virtual void v37();
};

#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Initialize a one-shot creature and its owned spray/sequence handle lists, then parse its definition.
void* CDisposable_Ctor(void* self, void* definition) {
    unsigned long head;
    void* tail;
    ListNode* end;
    ListNode* node;
    char* p = (char*)self;
    int zero = 0;
    AsCCreature(p)->flags = zero;
    *(void**)p = __vt__9CCreature;
    AsCDisposable(p)->secondaryVtable = (char*)__vt__9CCreature + kSecondaryCreatureVtable;
    Vec3_Zero(p + 0xc);
    Vec3_Zero((p + kCreatureBoundsMaximum));
    float zf = kFloat_0_Creature;
    node = (ListNode*)(((char*)&AsCDisposable(p)->sequenceHandleNext));
    head = (unsigned long)node;
    tail = ((char*)&AsCDisposable(p)->sprayHandleNext);
    AsCCreature(p)->puppet = zero;
    AsCDisposable(p)->animationTime = zf;
    AsCDisposable(p)->animationRange = zero;
    AsCDisposable(p)->heading = zf;
    AsCDisposable(p)->history = zero;
    AsCDisposable(p)->ballSpawnOffsetDistance = zf;
    AsCDisposable(p)->opacityKeyframeCount = zero;
    AsCDisposable(p)->opacityKeyframePrevious = ((char*)&AsCDisposable(p)->opacityKeyframeNext);
    AsCDisposable(p)->opacityKeyframeNext = ((char*)&AsCDisposable(p)->opacityKeyframeNext);
    AsCDisposable(p)->targetPoints = zero;
    AsCDisposable(p)->targetPointCount = zero;
    AsCDisposable(p)->targetPointCapacity = zero;
    AsCDisposable(p)->pathPoints = zero;
    AsCDisposable(p)->pathPointCount = zero;
    AsCDisposable(p)->pathPointCapacity = zero;
    *(void**)p = __vt__11CDisposable;
    AsCDisposable(p)->secondaryVtable = (char*)__vt__11CDisposable + kSecondaryCreatureVtable;
    AsCDisposable(p)->sequenceHandleCount = zero;
    (*(unsigned long*)&AsCDisposable(p)->sequenceHandlePrevious) = head;
    AsCDisposable(p)->sequenceHandleNext = head;
    AsCDisposable(p)->sprayHandleCount = zero;
    AsCDisposable(p)->sprayHandlePrevious = tail;
    AsCDisposable(p)->sprayHandleNext = tail;
    end = (ListNode*)(((char*)&AsCDisposable(p)->sequenceHandleNext));
    if (node != end) {
        ((volatile ListNode*)node)->next->prev = ((volatile ListNode*)node)->prev;
        ((volatile ListNode*)node)->prev->next = ((volatile ListNode*)node)->next;
        while (node != end) {
            ListNode* old = node;
            node = node->prev;
            appFreeUni__FPv(old);
            --AsCDisposable(p)->sequenceHandleCount;
        }
    }
    end = (ListNode*)AsCDisposable(p)->sprayHandlePrevious;
    node = (ListNode*)(((char*)&AsCDisposable(p)->sprayHandleNext));
    if (end != node) {
        ListNode* first = *(ListNode**)node;
        ListNode* nodeNext = end->next;
        nodeNext->prev = first->prev;
        first->prev->next = end->next;
        while (end != node) {
            ListNode* old = end;
            end = end->prev;
            appFreeUni__FPv(old);
            --AsCDisposable(p)->sprayHandleCount;
        }
    }
    AsCCreature(p)->flags |= kCreatureDisposable;
    ((CreatureVirtual*)self)->v36(definition);
    WPuppet_SetRendererOption(AsCDisposable(p)->puppet, 0);
    return self;
}
#pragma use_lmw_stmw reset
#pragma opt_common_subs reset

struct GuardFlag {
    char flag;
    // Initialize the effect-handle construction flag to its uncommitted state.
    GuardFlag(): flag(0) {}
};
struct GuardAlloc {
    ListNode* allocation; ListNode* copy;
    // Preserve the empty cleanup hook used by this expanded allocation path.
    ~GuardAlloc() {}
};
struct GuardTail {
    ListNode* head; ListNode* n;
    // Initialize an uncommitted node-allocation guard.
    GuardTail() : n(0) {}
    // Free a node allocation that was not committed to its list.
    ~GuardTail() { if (n) appFreeUni__FPv(n); }
};

// Store an effect handle in its node, releasing the allocation if initialization throws.
static inline void storeHandle(ListNode* node, void* handle) {
    void** p = &((EffectHandleNode*)node)->handle;
    if (p) {
        try { *p = handle; } catch (...) { appFreeUni__FPv(node); throw; }
    }
}

#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Parse an animation effect event, create its spray or sequence, and retain the effect handle.
// A non-null event must supply resource/text with the expected directive/scan fields.
// Parsing is unchecked: the existing fixed buffers and sscanf format are a caller contract.
void CDisposable_CreateAndStoreAttachedEffect(void* self, void* event) {
    char* extension;
    char* resourceStrings;
    int defaultArgument;
    void* eventText;
    void* model;
    void* handle;
    struct {
        char extra[0x20];
        char filename[0x20];
        char local[0x30];
    } scratch;
    char effectToken[8];
    if (event == 0) return;
    eventText = ((CreatureEffectEvent*)event)->resource->text;
    if (eventText == 0) return;
    resourceStrings = (char*)npcResourceStrings;
    defaultArgument = 0;
    sscanf(strstr((char*)eventText, ((const NpcResourcePool*)resourceStrings)->effectDirective), ((const NpcResourcePool*)resourceStrings)->effectScanFormat, effectToken, scratch.filename, scratch.extra);
    model = *(void**)event;
    if (model == 0) return;
    if (((CreatureEffectModel*)model)->flags & kEffectModelDisabled) return;
    extension = strchr(scratch.filename, '.');
    if (extension == 0) return;
    if (strlen(extension) < 4) return;
    if (String_CompareCString(extension + 1, ((const NpcResourcePool*)resourceStrings)->sprayExtension) != defaultArgument && String_CompareCString(extension + 1, ((const NpcResourcePool*)resourceStrings)->sequenceExtension) != defaultArgument) return;
    // The attachment borrows model transform/resource context; retain only the returned handle.
    FxAttachment_Init(scratch.local);
    ((CreatureEffectAttachment*)scratch.local)->transform = ((CreatureEffectEvent*)event)->model->attachmentTransform;
    ((CreatureEffectAttachment*)scratch.local)->context = ((CreatureEffectEvent*)event)->resource->attachmentContext;
    handle = CFx_CreateAttachedEffect(g_fx, scratch.local, scratch.filename, defaultArgument, 0, defaultArgument, ((CreatureEffectEvent*)event)->resource);
    if (String_CompareCString(extension + 1, ((const NpcResourcePool*)(char*)npcResourceStrings)->sprayExtension) == defaultArgument) {
        GuardFlag gf; GuardTail gt; GuardAlloc ga;
        ListNode* sentinel = (ListNode*)(((char*)&AsCDisposable(self)->sprayHandleNext));
        ListNode* node = (ListNode*)appMallocDummy__FUl(0xc);
        ga.allocation = node; ga.copy = node; gt.head = sentinel; gt.n = node;
        storeHandle(node, handle);
        ListNode* first = sentinel->next;
        first->prev = node;
        node->next = sentinel->next;
        sentinel->next = node;
        node->prev = sentinel;
        ++AsCDisposable(self)->sprayHandleCount;
        gt.n = 0;
        return;
    }
    if (String_CompareCString(extension + 1, ((const NpcResourcePool*)(char*)npcResourceStrings)->sequenceExtension) == defaultArgument) {
        GuardFlag gf; GuardTail gt; GuardAlloc ga;
        ListNode* sentinel = (ListNode*)(((char*)&AsCDisposable(self)->sequenceHandleNext));
        ListNode* node = (ListNode*)appMallocDummy__FUl(0xc);
        ga.allocation = node; ga.copy = node; gt.head = sentinel; gt.n = node;
        storeHandle(node, handle);
        gt.n = 0;
        ListNode* first = sentinel->next;
        first->prev = node;
        node->next = sentinel->next;
        sentinel->next = node;
        node->prev = sentinel;
        ++AsCDisposable(self)->sequenceHandleCount;
    }
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_common_subs off
// Free all nodes in the first list of effect handles and optionally delete the list.
void* dtor_801A5EF4(MSLPointerList* self, int deleting) {
    return ::std::__list_deleting_dtor(self, deleting);
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset

#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_common_subs off
// Free all nodes in the second list of effect handles and optionally delete the list.
void* dtor_801A5FB0(MSLPointerList* self, int deleting) {
    return ::std::__list_deleting_dtor(self, deleting);
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset

#pragma opt_common_subs off
// Set animation enablement; disabling removes and frees attached sequences and sprays.
// Only the low byte of enable is stored/tested. Removing effects precedes freeing list nodes.
void CDisposable_SetAnimationEnabled(void* ptr, int enable) {
    ListNode* end;
    ListNode* node;
    char* p = (char*)ptr;
    AsCDisposable(p)->animationEnabled = (unsigned char)enable;
    if ((enable & 0xFF) == 0) {
        node = ((ListNode*)AsCDisposable(p)->sequenceHandlePrevious);
        end = (ListNode*)(((char*)&AsCDisposable(p)->sequenceHandleNext));
        while (node != end) {
            CFx_RemoveSequence(g_fx, ((EffectHandleNode*)node)->handle, 0);
            node = node->prev;
        }
        node = ((ListNode*)AsCDisposable(p)->sequenceHandlePrevious);
        end = (ListNode*)(((char*)&AsCDisposable(p)->sequenceHandleNext));
        if (node != end) {
            ListNode* first = *(ListNode**)end;
            node->next->prev = first->prev;
            first->prev->next = node->next;
            while (node != end) {
                ListNode* old = node;
                node = node->prev;
                appFreeUni__FPv(old);
                --AsCDisposable(p)->sequenceHandleCount;
            }
        }
        // Sprays use their own FX removal call; list nodes never own the FX allocation.
        node = ((ListNode*)AsCDisposable(p)->sprayHandlePrevious);
        end = (ListNode*)(((char*)&AsCDisposable(p)->sprayHandleNext));
        while (node != end) {
            CFx_RemoveSpray(g_fx, ((EffectHandleNode*)node)->handle);
            node = node->prev;
        }
        node = ((ListNode*)AsCDisposable(p)->sprayHandlePrevious);
        end = (ListNode*)(((char*)&AsCDisposable(p)->sprayHandleNext));
        if (node != end) {
            ListNode* first = *(ListNode**)end;
            node->next->prev = first->prev;
            first->prev->next = node->next;
            while (node != end) {
                ListNode* old = node;
                node = node->prev;
                appFreeUni__FPv(old);
                --AsCDisposable(p)->sprayHandleCount;
            }
        }
    }
}
#pragma opt_common_subs reset

#pragma opt_common_subs off
// Remove owned effects, free both handle lists, destroy the creature base, and optionally delete.
void* CDisposable_Dtor(void* ptr, int deleting) {
    // Phase 1: remove live FX handles before releasing their owned list nodes.
    ListNode* end;
    ListNode* node;
    if (ptr) {
        char* p = (char*)ptr;
        node = ((ListNode*)AsCDisposable(p)->sequenceHandlePrevious);
        end = (ListNode*)(((char*)&AsCDisposable(p)->sequenceHandleNext));
        *(void**)p = __vt__11CDisposable;
        AsCDisposable(p)->secondaryVtable = (char*)__vt__11CDisposable + kSecondaryCreatureVtable;
        while (node != end) {
            CFx_RemoveSequence(g_fx, ((EffectHandleNode*)node)->handle, 0);
            node = node->prev;
        }
        node = ((ListNode*)AsCDisposable(p)->sequenceHandlePrevious);
        end = (ListNode*)(((char*)&AsCDisposable(p)->sequenceHandleNext));
        if (node != end) {
            ListNode* first = *(ListNode**)end;
            node->next->prev = first->prev;
            first->prev->next = node->next;
            while (node != end) {
                ListNode* old = node;
                node = node->prev;
                appFreeUni__FPv(old);
                --AsCDisposable(p)->sequenceHandleCount;
            }
        }
        end = (ListNode*)(((char*)&AsCDisposable(p)->sprayHandleNext));
        node = ((ListNode*)AsCDisposable(p)->sprayHandlePrevious);
        while (node != end) {
            CFx_RemoveSpray(g_fx, ((EffectHandleNode*)node)->handle);
            node = node->prev;
        }
        node = ((ListNode*)AsCDisposable(p)->sprayHandlePrevious);
        end = (ListNode*)(((char*)&AsCDisposable(p)->sprayHandleNext));
        if (node != end) {
            ListNode* first = *(ListNode**)end;
            node->next->prev = first->prev;
            first->prev->next = node->next;
            while (node != end) {
                ListNode* old = node;
                node = node->prev;
                appFreeUni__FPv(old);
                --AsCDisposable(p)->sprayHandleCount;
            }
        }
        // Phase 2: member cleanup frees nodes only. Address tests are ABI scaffolding,
        // not ownership/count tests; retain their evaluation order for identical code.
        if (((((char*)&AsCDisposable(p)->sprayHandleCount)) &&
             (((char*)&AsCDisposable(p)->sprayHandleCount))) &&
            ((((char*)&AsCDisposable(p)->sprayHandleCount)) &&
             (((char*)&AsCDisposable(p)->sprayHandleCount)))) {
            end = (ListNode*)(((char*)&AsCDisposable(p)->sprayHandleNext));
            node = ((ListNode*)AsCDisposable(p)->sprayHandlePrevious);
            if (node != end) {
                ListNode* first = *(ListNode**)end;
                node->next->prev = first->prev;
                first->prev->next = node->next;
                while (node != end) {
                    ListNode* old = node;
                    node = node->prev;
                    appFreeUni__FPv(old);
                    --*(int*)(p + kSprayHandleCount);
                }
            }
        }

        if (((((char*)&AsCDisposable(p)->sequenceHandleCount)) &&
             (((char*)&AsCDisposable(p)->sequenceHandleCount))) &&
            ((((char*)&AsCDisposable(p)->sequenceHandleCount)) &&
             (((char*)&AsCDisposable(p)->sequenceHandleCount)))) {
            end = (ListNode*)(((char*)&AsCDisposable(p)->sequenceHandleNext));
            node = ((ListNode*)AsCDisposable(p)->sequenceHandlePrevious);
            if (node != end) {
                ListNode* first = *(ListNode**)end;
                node->next->prev = first->prev;
                first->prev->next = node->next;
                while (node != end) {
                    ListNode* old = node;
                    node = node->prev;
                    appFreeUni__FPv(old);
                    --*(int*)(p + kSequenceHandleCount);
                }
            }
        }

        CCreature_dtor(ptr, 0);
        if (deleting > 0) __dl__FPv(ptr);
    }
    return ptr;
}
#pragma opt_common_subs reset

void WPuppet_SetOpacityRecursive(void*, unsigned char); void WView_DrawFacingCircleVec4(void*,void*,int,int,int);
void WView_DrawLine(void*,void*,int,void*,int,int); void WPuppet_GetInverseRootMotionOffsetForRange(void*,void*,float,void*);
void Matrix_TransformDirection(void*,void*,void*); void __ami__7WVectorFRC7WVector(void*,void*); void WPuppet_SetAnimationTimeFromRanges(void*,int,float,int,int);
void Model_SetMatrixRecursive(void*,void*,void*); void WPuppet_UpdateBoneWorldMatrices(void*,void*,int,float); void WPuppet_UpdateBoundsFromBone(void*);
void __apl__7WVectorFRC7WVector(void*,void*);
extern char s_sceneLighting[];
// ABI view: declarations start at byte +0x08 after RTTI/null words.
// Legacy labels v5/v6 are slots +0x14/+0x18, while v40 is +0xA0.
// They are used on puppet/creature objects respectively; not one semantic class.
struct PolaVirtual {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual void v5(const char*, int, int); // Likely role: lightingHookSlot14 (base WPuppet hook is empty).
    virtual void v6(void*, int, int, int, float, int); // Likely role: renderWithCamera; byte slot +0x18.
    virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16();
    virtual void v17(); virtual void v18(); virtual void v19(); virtual void v20(); virtual void v21();
    virtual void v22(); virtual void v23(); virtual void v24(); virtual void v25(); virtual void v26();
    virtual void v27(); virtual void v28(); virtual void v29(); virtual void v30(); virtual void v31();
    virtual void v32(); virtual void v33(); virtual void v34(); virtual void v35(); virtual void v36();
    virtual void v37(); virtual void v38(); virtual void v39();
    virtual int v40(void*, float);
    // Likely role: isOutsideCameraFacingThreshold; byte slot +0xA0.
};
// Draw an active polar penguin with root-motion compensation and optional debug geometry.
void ntNpc_PolaPenguin2_Draw(void* self) {
    WVector rootMotion, worldRootMotion;
    char* p = (char*)self;
    if ((*(void**)&AsCCreature(p)->puppet) == 0) return;
    if (!AsCCreature(p)->animationActive) return;
    if (AsCCreature(p)->animationTime < kFloat_0_Creature) return;
    if ((*(unsigned int*)&AsCCreature(p)->flags) & 1) WPuppet_SetOpacityRecursive((*(void**)&AsCCreature(p)->puppet), AsCCreature(p)->opacity);
    extern int g_drawSceneDebugGeometry; extern void* g_view;
    if (g_drawSceneDebugGeometry) {
        WView_DrawFacingCircleVec4(g_view, (char*)(*(void**)&AsCCreature(p)->puppet)+0x124, kDebugCircleGreen, kDebugCircleSegments, 0);
        if (!((*(unsigned int*)&AsCCreature(p)->flags) & 1)) {
            WVector headingLine = kFloat_5_creature * *(WVector*)((p + kCreatureForwardAxis));
            WVector headingEnd = *(WVector*)((p + kCreatureTranslation)) - headingLine;
            WView_DrawLine(g_view, (p + kCreatureTranslation), kDebugHeadingYellow, &headingEnd, kDebugHeadingYellow, 0);
            WView_DrawLine(g_view, (p + kCreatureTranslation), kDebugTargetRed, (p + kCreatureTargetPosition), kDebugTargetRed, 0);
        }
    }
    // Compensate the model's local root motion in world space while updating its bones.
    ((void (*)(void*, void*, void*, float))WPuppet_GetInverseRootMotionOffsetForRange)(&rootMotion, (*(void**)&AsCCreature(p)->puppet), ((CreatureAnimationRange*)AsCCreature(p)->animationRange)->name, AsCCreature(p)->animationTime);
    Matrix_TransformDirection(&worldRootMotion, &rootMotion, (p + kCreatureTransform));
    rootMotion = worldRootMotion;
    *(WVector*)((p + kCreatureTranslation)) -= rootMotion;
    WPuppet_SetAnimationTimeFromRanges((*(void**)&AsCCreature(p)->puppet), 0, AsCCreature(p)->animationTime, 0, 0);
    int culled = ((PolaVirtual*)self)->v40((p + kCreatureTranslation), kFloat_0_8_805DF2D8);
    // Existing early-return behavior leaves the compensated translation in place when culled.
    if (culled != 0) return;
    if (AsCCreature(p)->transform[0] > kFloat_1_Creature) AsCCreature(p)->transform[0] = kFloat_1_Creature;
    if (AsCCreature(p)->transform[4] > kFloat_1_Creature) AsCCreature(p)->transform[4] = kFloat_1_Creature;
    if (AsCCreature(p)->transform[8] > kFloat_1_Creature) AsCCreature(p)->transform[8] = kFloat_1_Creature;
    Model_SetMatrixRecursive((*(void**)&AsCCreature(p)->puppet), (*(void**)&AsCCreature(p)->puppet), (p + kCreatureTransform));
    WPuppet_UpdateBoneWorldMatrices((*(void**)&AsCCreature(p)->puppet), g_view, 0, kFloat_1_Creature);
    WPuppet_UpdateBoundsFromBone((*(void**)&AsCCreature(p)->puppet));
    *(WVector*)((p + kCreatureTranslation)) += rootMotion;
    {
        void* model = (*(void**)&AsCCreature(p)->puppet);
        ((PolaVirtual*)model)->v5(s_sceneLighting, 0, 0);
    }
    {
        void* model = (*(void**)&AsCCreature(p)->puppet);
        ((PolaVirtual*)model)->v6(g_view, 1, 1, 1, kFloat_1_Creature, 0);
    }
}

// Destroy the polar penguin base and optionally release its object storage.
void* ntNpc_PolaPenguin2_Dtor(void* ptr, int deleting) {
    if (ptr != 0) {
        CCreature_dtor(ptr, 0);
        if (deleting > 0) {
            __dl__FPv(ptr);
        }
    }
    return ptr;
}

// Release spline points, parameter and length arrays, reset totals, and optionally delete storage.
void* CreatureSplineStorage_dtor(void* ptr, int deleting) {
    if (ptr != 0) {
        void** fields = (void**)ptr;
        if (fields[0] != 0) {
            if (fields[0] != 0) {
                __dla__FPv((char*)fields[0] - 0x10);
            }
            fields[0] = 0;
        }
        if (fields[1] != 0) {
            __dla__FPv(fields[1]);
            fields[1] = 0;
        }
        if (fields[2] != 0) {
            __dla__FPv(fields[2]);
            fields[2] = 0;
        }
        float zero = kFloat_0_Creature;
        int shouldDelete = deleting > 0;
        ((CreatureSplineStorage*)ptr)->pointCount = 0;
        ((CreatureSplineStorage*)ptr)->totalLength = zero;
        if (shouldDelete) {
            __dl__FPv(ptr);
        }
    }
    return ptr;
}

// Test whether world-space target XYZ lies within the inclusive movement box.
// movementBoundsOrigin is the legacy shared name for the minimum corner here.
// Original 0x801A5990: preserve short-circuit comparison order (including NaN behavior).
static inline int PenguinTargetInsideBounds(char* p) {
    int withinX, aboveMinimumY, withinY, aboveMinimumZ, insideBounds;
    insideBounds = 0;
    aboveMinimumZ = 0;
    withinY = 0;
    aboveMinimumY = 0;
    withinX = 0;
    if (AsCCreature(p)->movementBoundsOrigin[0] <= AsCCreature(p)->targetPosition[0]) {
        if (AsCCreature(p)->targetPosition[0] <= AsCCreature(p)->movementBoundsMax[0]) withinX = 1;
    }
    if (withinX) {
        if (AsCCreature(p)->movementBoundsOrigin[1] <= AsCCreature(p)->targetPosition[1]) aboveMinimumY = 1;
    }
    if (aboveMinimumY) {
        if (AsCCreature(p)->targetPosition[1] <= AsCCreature(p)->movementBoundsMax[1]) withinY = 1;
    }
    if (withinY) {
        if (AsCCreature(p)->movementBoundsOrigin[2] <= AsCCreature(p)->targetPosition[2]) aboveMinimumZ = 1;
    }
    if (aboveMinimumZ) {
        if (AsCCreature(p)->targetPosition[2] <= AsCCreature(p)->movementBoundsMax[2]) insideBounds = 1;
    }
    return insideBounds;
}

// Sample a target, push it outside the bounds, then steer and project motion onto terrain.
void ntNpc_PolaPenguin2_UpdateWanderMovement(void* self, float dt) {
    // Phase 1: choose/project a new target when within one world unit of the old target.
    char* p = (char*)self;
    // Transform translation is +0x4C; the world-space movement target is +0x64.
    // Every 12-byte scratch below is XYZ float storage; matrices hold 12 floats.
    // dt shares animation time units; turnRate * dt is radians, speed * dt is world distance.
    float combinedRotation[12];
    float turnRotation[12];
    char rootMotion[12];
    char targetDistanceVector[12];
    char targetDirection[12];
    char frameStep[12];
    char forwardVelocity[12];
    char groundPosition[12];
    char currentPosition[12];
    char localRootMotion[12];
    char worldRootMotion[12];
    char projectedGroundPoint[12];
    char projectedPosition[12];
    char newTargetPosition[12];
    char newTargetGround[12];
    char outwardDirection[12];
    char outwardStep[12];
    float one;
    float distanceToTarget;
    __mi__FRC7WVectorRC7WVector(targetDistanceVector, (p + kCreatureTranslation), (p + kCreatureTargetPosition));
    distanceToTarget = WVectorLen__FRC7WVector(targetDistanceVector);
    one = kFloat_1_Creature;
    if (distanceToTarget < one) {
        int i;
        char* axisCursor = (char*)self;
        for (i = 0; i < 3; ++i) {
            float minimumWeight = Random_RangeFloat(-1, kFloat_0_Creature, kFloat_1_Creature);
            float maximumWeight = one - minimumWeight;
            *(float*)(axisCursor + 0x64) = minimumWeight * *(float*)(axisCursor + 0x0C) + maximumWeight * *(float*)(axisCursor + 0x18);
            axisCursor += 4;
        }
        // The loop condition is intentional: step outward until any axis leaves the box.
        // The normalized direction requires target and translation to differ.
        while (PenguinTargetInsideBounds(p)) {
            __mi__FRC7WVectorRC7WVector(outwardDirection, (p + kCreatureTargetPosition), (p + kCreatureTranslation));
            __ml__FRC7WVectorf(outwardStep, Normalize__7WVectorFv(outwardDirection), kFloat_3_2_CourseGeometry);
            __apl__7WVectorFRC7WVector((p + kCreatureTargetPosition), outwardStep);
        }
        *(Arr3*)newTargetPosition = (*(Arr3*)(p + kCreatureTargetPosition));
        GetGroundPoint__9CQuadTreeF7WVectorbPib(newTargetGround, CQuadTree_GetInstance(), newTargetPosition, 1, 0, 0);
        __as__7WVectorFRC7WVector((p + kCreatureTargetPosition), newTargetGround);
    }
    // Phase 2: walking uses speed * dt in world space; turning clamps to +/- turnRate * dt.
    if (AsCCreature(p)->movementEnabled) {
        float delta;
        float magnitude;
        __mi__FRC7WVectorRC7WVector(targetDirection, (p + kCreatureTargetPosition), (p + kCreatureTranslation));
        delta = Vector_SignedAngleXZ(targetDirection, (p + kCreatureForwardAxis));
        magnitude = delta > kFloat_0_Creature ? delta : -delta;
        if (magnitude > kFloat_0_017453292_Creature) {
            float turnRate = AsCCreature(p)->turnRate;
            float minimumTurn = -turnRate * dt;
            float maximumTurn = turnRate * dt;
            float turnAngle;
            if (delta < minimumTurn) {
                turnAngle = minimumTurn;
            } else {
                float clampedTurn;
                if (delta > maximumTurn) clampedTurn = maximumTurn;
                else clampedTurn = delta;
                turnAngle = clampedTurn;
            }
            // Gekko passes the axis in a GPR and angle in an FPR; the two source
            // declaration orders describe the same register ABI. Retain this call view.
            ((void (*)(void*, float, int))Matrix_SetAxisRotation)(turnRotation, turnAngle, kCreatureYAxis);
            Matrix_ConcatAffine(combinedRotation, turnRotation, (p + kCreatureTransform));
            Matrix_CopyAffine((p + kCreatureTransform), combinedRotation);
        }
        __ml__FfRC7WVector(forwardVelocity, (p + kCreatureForwardAxis), AsCCreature(p)->movementSpeed);
        __ml__FRC7WVectorf(frameStep, forwardVelocity, dt);
        __apl__7WVectorFRC7WVector((p + kCreatureTranslation), frameStep);
        *(Arr3*)currentPosition = (*(Arr3*)(p + kCreatureTranslation));
        GetGroundPoint__9CQuadTreeF7WVectorbPib(groundPosition, CQuadTree_GetInstance(), currentPosition, 1, 0, 0);
        __as__7WVectorFRC7WVector((p + kCreatureTranslation), groundPosition);
        return;
    }
    // Phase 3: default-range motion is provisional; use projected Y without committing X/Z.
    {
        void* animationRange = (*(void**)&AsCCreature(p)->animationRange);
        WPuppet_GetRootMotionDelta(localRootMotion, kCreatureAnimationFramesPerSecond * AsCCreature(p)->animationTime, (*(void**)&AsCCreature(p)->puppet),
                    kCreatureAnimationFramesPerSecond * ((CreatureAnimationRange*)animationRange)->startTime, ((CreatureAnimationRange*)animationRange)->name);
        __as__7WVectorFRC7WVector(rootMotion, localRootMotion);
        Matrix_TransformDirection(worldRootMotion, rootMotion, (p + kCreatureTransform));
        __as__7WVectorFRC7WVector(rootMotion, worldRootMotion);
        __apl__7WVectorFRC7WVector(rootMotion, (p + kCreatureTranslation));
        *(unsigned int*)projectedPosition = *(unsigned int*)rootMotion;
        *((unsigned int*)projectedPosition + 1) = *((unsigned int*)rootMotion + 1);
        *((unsigned int*)projectedPosition + 2) = *((unsigned int*)rootMotion + 2);
        GetGroundPoint__9CQuadTreeF7WVectorbPib(projectedGroundPoint, CQuadTree_GetInstance(), projectedPosition, 1, 0, 0);
        AsCCreature(p)->transform[10] = *(float*)(projectedGroundPoint + 4);
    }
}

} // extern "C"
