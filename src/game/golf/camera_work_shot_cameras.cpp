// Shot cameras for CCameraWork: the pose for each camera mode (launch, hole, power shot,
// trajectory event) and the per-frame updates that track the ball and move the camera.
// Mode selection and the manual input offsets are here too.

#include <decomp/local_class.h>
#include <decomp/ct_member.h>
// Shot camera policy, pose setup, curve evaluation and manual controller offsets.
// Mod lookup:
// - Change which shot gets a camera: SelectShotCameraMode, ShotCameraSelectionCodes
//   and the producer map below. Keep random draw count/priority and installed guard.
// - Change a camera's placement/motion: ConfigureMode -> mode definition index
//   beside CameraCallbackSlot -> Configure* helper and its named update callback.
//   Literal geometry is defined IN those helpers; scalar pools are in
//   the data section. CameraSetupFrameSlot identifies scratch, not tuning data.
// - Change orbit/curve timing: BuildModeCurve and the indexed evaluator; keep
//   trajectory sample coordinates distinct from animationTime gameplay seconds.
// - Controller sensitivity / return speed / zoom limits: UpdateInputOffsets has
//   the units/baselines/consumer map at entry. Shared float words affect several
//   modes, so introduce private storage in a future mod before decoupling them.
// - Shared data: CCameraWork.h, CGolfDoc.h, golfball.h; callback definitions and
// CGolfRule drives this subsystem (golf_rule_round_flow_and_shot_sim.cpp); this unit publishes
// s_cameraTransform and Camera_SetFov to the view. No retrace policy lives here.
extern "C" int s_cameraModeWeightsStorage[];
// Camera contracts used throughout this merged unit:
// - Positions/targets/trajectory samples use world coordinates, +Y up, in game units.
//   Affine camera transforms store right/up/forward/position vectors in that order.
// - FOV, heading and roll use radians; explicit degree constants pass through kDegreesToRadians.
// - elapsedTime is a trajectory sample/frame coordinate, while animationTime accumulates the
//   first callback argument. The second callback argument is the fractional sample blend.
//   Two-argument evaluators call this frameFraction; a one-argument input update still uses time delta dt.
// - Receivers, singleton objects, PreData samples and bone pointers are borrowed. Only history
//   growth and explicitly constructed exception payloads allocate ownership in this unit.
// - Mode setup supplies valid sample/event intervals and nonzero curve/duration denominators.
//   Evaluators generally do not clamp their interpolation parameter or validate those intervals.
// Shot records retain their original integer/byte load widths at these camera-specific fields.
struct CameraTrajectorySample {
    float position[3], velocity[3];
    char reserved18[0x48];
    unsigned char cameraCutState; // value 2 selects the cut frame
    char reserved61[0xF];
    int groundEffectState; // value 3 enables the near-ground FOV adjustment
};
typedef char CameraTrajectorySample_size[(sizeof(CameraTrajectorySample) == 0x74) ? 1 : -1];
struct CameraActorResponse {
    char reserved00[0x45];
    unsigned char cameraAllowed;
};
struct BonePositionView {
    char reserved00[0x9C];
    float worldPosition[3];
};

// Policy gate producer map (do not substitute ball motion state for club category):
// - InstanceTable_Get()->selected+0xA is clubGroup: RegisterClubParameters in
//   club_selection_and_shot_params.cpp writes 0 wood/1 iron/2 wedge/3 putter. This byte is not a
//   camera state, despite the older local variable name "state".
// - CGolfDoc::impactGrade=1 is miss (0 unset,2 poor,3 good,4 Pangya). Producers:
//   CGolfRule::CommitShotGauge in golf_rule_round_flow_and_shot_sim.cpp and CRival_PrepareShotGauge in
//   com_shot_cpu_rival_ai.cpp; miss is selected outside the good/poor impact windows.
// - Event[11] records the first sample after powerFlags bit0x10's live m_flags
//   bit0x10 clears. Event[14] similarly records bit0x40 clearing, but only after
//   event[13]>0. Both writes occur in shot simulation in golf_rule_round_flow_and_shot_sim.cpp (search
//   kBallFlag10EndSample/kBallFlag40EndSample); ResetShotCollisionAndSamples
//   initializes them to -1. The camera compares their SAMPLE INDEX to event[1],
//   not a Boolean "effect finished" flag. Quadtree contact paths in course_collision_and_ball_physics.cpp
//   publish event[1] once on contact. Preserve equality, including absent -1.
//   The bit transitions are established; a player-facing effect name is not
//   needed to edit the selection probabilities and is deliberately not invented.
enum ShotCameraSelectionCodes {
    kShotCameraPutterCategory = 3,
    kShotCameraMissImpact = 1,
    kShotCameraGreenGround = 2,
    kShotCameraPowerMask = 0x70,
    kShotCameraPowerBit10 = 0x10,
    kShotCameraPowerBit20 = 0x20,
    kShotCameraPowerBit40 = 0x40,
    kShotCameraSecondEventSlot = 1,
    kShotCameraEventSlot11 = 11,
    kShotCameraEventSlot14 = 14,
    kShotCameraCaddieQuery = 0x11,
    kShotCameraCutSampleState = 2,
    kShotCameraNormalChance = 1,
    kShotCameraBoostedChance = 5
};

enum CameraPowerShotVariant {
    kPowerShotNoVariant = 0,
    kPowerShotVariant1 = 1,
    kPowerShotVariant2 = 2
};
enum CameraActionFlags {
    kCameraFollowCurrentBall = 1,
    kCameraCutAtEvent = 0x20000000,
    kCameraShakeActive = 0x80000000u
};
enum CameraAction {
    kCameraAction0 = 0,
    kCameraAction1 = 1,
    kCameraEventSideView = 3,
    kCameraSecondEventView = 4,
    kCameraAction5 = 5,
    kCameraAction7 = 7,
    kCameraAction8 = 8,
    kCameraWalkView9 = 9,
    kCameraAction10 = 10,
    kCameraSecondEventAlternateView = 11,
    kCameraWalkView12 = 12,
    kCameraTrajectorySideMidpointView = 13,
    kCameraWalkView18 = 18,
    kCameraAction19 = 19,
    kCameraAction23 = 23,
    kCameraPowerLaunchView = 24,
    kCameraPowerTrajectoryView = 25,
    kCameraPowerStartView = 26,
    kCameraWalkView27 = 27,
    kCameraAction105 = 105,
    kCameraAction200 = 200,
    kCameraAction201 = 201,
    kCameraAction202 = 202,
    kCameraAction2 = 2, // callback/action table index; fuller gameplay role unresolved
    kCameraCutoffBisectorView = 14,
    kCameraPlayerHeadView = 20,
    kCameraLaunchQuarterView = 21,
    kCameraLaunchRandomWideView = 22,
    kCameraTrackBallPitch = 100,
    kCameraRaisedLaunchSideView = 101,
    kCameraRaisedLaunchRearView = 102,
    kCameraElevatedHoleSideView = 103,
    kCameraTrajectoryMidpointView = 104,
    kCameraLaunchFrontView = 106,
    kCameraShotAndHoleView = 107,
    kCameraHoleReverseView = 108,
    kCameraHoleSideView = 109,
    kCameraHoleVelocityView = 110,
    kCameraHoleNearView = 111,
    kCameraHoleTelephotoView = 112,
    kCameraBelowHoleView = 150,
    kCameraOrbitToBall = 203,
    kCameraBlendToGolfView = 204,
    kCameraNarrowGolfView = 205,
    kCameraPowerShotStaticView = 206,
    kCameraPowerShotBlendView = 207,
    kCameraRaisedHoleOrbit = 208,
};

// Address-preserving view of the existing resource string pool.
struct CameraWorkStrings {
    char m_def[6];                // 0x0
    char ken[4];                  // 0x6
    char f_def[6];                // 0xa
    char eri[4];                  // 0x10
    char a_def[6];                // 0x14
    char dai[4];                  // 0x1a
    char c_def[6];                // 0x1e
    char ces[4];                  // 0x24
    char d_def[6];                // 0x28
    char max[4];                  // 0x2e
    char e_def[6];                // 0x32
    char koo[4];                  // 0x38
    char g_def[6];                // 0x3c
    char ari[4];                  // 0x42
    char h_def[6];                // 0x46
    char kaz[4];                  // 0x4c
    char albatrossPose[23];       // 0x50
    char Albatross[10];           // 0x67
    char holeInOnePose[25];       // 0x71
    char HoleInOne[10];           // 0x8a
    char birdiePose[25];          // 0x94
    char Birdie[7];               // 0xad
    char parPose[21];             // 0xb4
    char Par[4];                  // 0xc9
    char bogeyPose[19];           // 0xcd
    char Bogey[6];                // 0xe0
    char doublePower04Pose[25];   // 0xe6
    char DoublePower04[14];       // 0xff
    char doublePower03Pose[25];   // 0x10d
    char DoublePower03[14];       // 0x126
    char doublePower02Pose[25];   // 0x134
    char DoublePower02[14];       // 0x14d
    char doublePower01Pose[25];   // 0x15b
    char DoublePower01[14];       // 0x174
    char stroke[7];               // 0x182
    char match[6];                // 0x189
    char singleAwardCeremony[12]; // 0x18f
    char Single[7];               // 0x19b
    char vector_length_error[20]; // 0x1a2
    char lengthErrorTypeName[55]; // 0x1b6
    char PLAYER_d[10];            // 0x1ed
    char Bip01_HeadNub[14];       // 0x1f7
    char Bip01_Head[11];          // 0x205
    char Player[7];               // 0x210
    char Caddie[7];               // 0x217
    char ballWindSoundAlias[10];  // 0x21e
    char pass_RL[8];              // 0x228
    char pass_LR[8];              // 0x230
    char caddieLabel[11];         // 0x238
    char Head[5];                 // 0x243
    char reserved248[1];
    char camera_double_power_camera[28];        // 0x249
    char camera_hole_end_camera[24];            // 0x265
    char vcg[5];                                // 0x27d
    char nameSeparator[2];                      // 0x282
    char rank[6];                               // 0x284
    char tree_insert_length_error[26];          // 0x28a
    char basic_string_reserve_length_error[35]; // 0x2a4
    char basic_string_out_of_range[27];         // 0x2c7
    char outOfRangeTypeName[55];                // 0x2e2
    char basic_string_length_error[27];         // 0x319
};

#include "game/golfball.h"
struct CameraTransformView {
    float right[3], up[3], forward[3], position[3];
};
#include "classes/CSharedDoc.h"
#include "classes/WPuppet.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CCameraWork.h"
#define AsCCameraWork(p) ((CCameraWork*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const CameraWorkStrings cameraWorkStrings;
}
}

#include "game/singletons.h"
#include "game/types.h"
#include "sdk_protos.h"
#include "classes/CVCGPlayer.h"
#include "classes/CQuadTree.h"
// Construct an exception payload in caller-provided storage without allocating it again.
inline void* operator new(size_t, void* p) {
    return p;
}

// ---- merged from camera_work_shot_cameras.cpp ----
namespace ns0 {
// The mode setup scratch slots and bit-copy views preserve CodeWarrior scheduling.

extern "C" {

extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
extern const CameraWorkStrings cameraWorkStrings;
// This symbol is also the base of the callback triplets addressed below. Other units
// genuinely index its initial words as selection weights; do not rename it for this view alone.
extern int s_extendedCameraModeWeights[];
extern const int zeroInitializationWords[];
extern CSharedDoc* g_sharedDoc;
extern char* g_sceneManager;
extern size_t strlen(const char*);
extern char* strcpy(char*, const char*);
extern void* memcpy(void*, const void*, unsigned long);
extern void* memset(void*, int, unsigned long);
extern void* __nw__FUl(unsigned long);
extern void __dla__FPv(void*);
extern void __throw(void*, void*, void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern void* __nwa__FUl(int);
extern void* appMallocDummy__FUl(unsigned long);
extern void appFreeUni__FPv(void*);

extern char* GolfBall__Fv();
extern char* PreData__9CGolfBallFi(void*, int);
extern void __as__7WVectorFRC7WVector(void*, const void*);
extern void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern void __ml__FRC7WVectorf(void*, const void*, float);
extern void __ml__FfRC7WVector(void*, const void*, float);
extern void Vec2_Zero(void*);
extern void Matrix_BuildCameraTransform(void*, const void*, const void*);
extern void Vector_CrossProduct(void*, const void*, const void*);
extern void* Normalize__7WVectorFv(void*);
extern void Matrix_ConcatAffine(void*, const void*, const void*);
extern void Matrix_TransformDirection(void*, const void*, const void*);
extern void Matrix_CopyAffine(void*, const void*);
extern void Matrix_SetAxisRotation(void*, float, int);
extern float Random_RangeFloat(int, float, float);
extern void* String_FormatTemporary(const char*, ...);
extern void* NamedEffectList_FindByName(void*, void*);
extern char* WPuppet_FindOrCacheBone(void*, const char*);
extern void Matrix3x3_ToQuat(void*, const void*);
extern void* Matrix_TransformPoint(void*, const void*, const void*);
extern float __ml__FRC7WVectorRC7WVector(const void*, const void*);
struct Sink {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void send(void*);
};
extern Sink* TaskActorTable_FindByName(void*, const void*);
extern void* Object_GetChild(void*);
extern void* g_taskManager;

struct LayoutVectorAllocation {
    char* pointer;
    char* allocator;
    // Guard a partially constructed exception message until ownership transfers.
    LayoutVectorAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Free an exception message whose ownership was not transferred.
    ~LayoutVectorAllocation() {
        if (pointer)
            __dla__FPv(pointer);
    }
};
struct LayoutVectorControlBase {
    void* table;
    unsigned strong, weak;
    // Initialize the shared message reference counts and base deleter table.
    LayoutVectorControlBase() {
        strong = 1;
        weak = 1;
        table = s_sharedCharDeleterVtable;
    }
};
struct LayoutVectorControl : LayoutVectorControlBase {
    char* pointer;
    // Attach the allocated message to its reference-counted control block.
    LayoutVectorControl(char* p) {
        pointer = p;
    }
};
struct LayoutVectorErrorBase {
    void* table;
    // Install the standard logic-error base table before constructing its payload.
    LayoutVectorErrorBase() {
        table = __vt__Q23std11logic_error;
    }
    // Run the standard exception base destructor without freeing this storage.
    ~LayoutVectorErrorBase() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
namespace std {
struct length_error : LayoutVectorErrorBase {
    char* text;
    void* control;
    // Allocate and copy the camera vector length-error message with exception-safe ownership.
    length_error() {
        const char* message = cameraWorkStrings.vector_length_error;
        struct Allocator {
            char value;
            char pad[3];
        } allocator;
        allocator.value = 0;
        unsigned n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            LayoutVectorAllocation allocation(p, &allocator.value);
            control = new LayoutVectorControl(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}
} // extern C

extern "C" {

struct LayoutVectorBuffer {
    int* data;
    volatile unsigned size;
    unsigned capacity;
    unsigned* owner;
    volatile unsigned offset;
    // Track a pending vector allocation and its original capacity owner.
    LayoutVectorBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Free the pending allocation on failure, or the replaced old allocation after commit.
    ~LayoutVectorBuffer() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
// Select a growth count by reference to preserve the caller's temporary lifetimes.
inline const unsigned& LayoutVectorMax(const unsigned& a, const unsigned& b) {
    return a < b ? b : a;
}

// Tests whether a scalar is within the camera zero tolerance.
static inline bool CameraIsNearZero(const float& v) {
    return ((*(const unsigned*)&v & 0x80000000u) ? -v : v) < 0.00001f;
}
// Absolute value without changing the compiler's sign-bit test.
static inline float CameraAbs(float x) {
    return (*(unsigned*)&x & 0x80000000u) ? -x : x;
}

// Original frame coordinates for mode setup. Names describe the value produced/consumed,
// not a new stack allocation. CameraScratchAddress converts from this coordinate system.
// Work slots can be reused after their vector calls; preserve each caller's existing lifetime.
// These are coordinates in the existing reconstruction, including slots below the byte
// array origin (0x78); this code does not claim to repair that original stack representation.
enum CameraSetupFrameSlot {
    kSetupRaisedOrbitCup = 0x64,
    kSetupPowerBlend2EndLookOffset = 0x70,
    kSetupPowerBlend2EndTarget = 0x7c,
    kSetupPowerBlend2EndLocalPosition = 0x88,
    kSetupPowerBlend2EndWorldPosition = 0x94,
    kSetupPowerBlend2StartLookOffset = 0xa0,
    kSetupPowerBlend2StartTarget = 0xac,
    kSetupPowerBlend2StartLocalPosition = 0xb8,
    kSetupPowerBlend2StartWorldPosition = 0xc4,
    kSetupPowerBlend1EndLookOffset = 0xd0,
    kSetupPowerBlend1EndTarget = 0xdc,
    kSetupPowerBlend1EndLocalPosition = 0xe8,
    kSetupPowerBlend1EndWorldPosition = 0xf4,
    kSetupPowerBlend1StartLookOffset = 0x100,
    kSetupPowerBlend1StartTarget = 0x10c,
    kSetupPowerBlend1StartLocalPosition = 0x118,
    kSetupPowerBlend1StartWorldPosition = 0x124,
    kSetupPowerStatic2TargetRise = 0x130,
    kSetupPowerStatic2Target = 0x13c,
    kSetupPowerStatic2BasisSum = 0x148,
    kSetupPowerStatic2Offset = 0x154,
    kSetupPowerStatic2RotatedOffset = 0x160,
    kSetupPowerStatic2Position = 0x16c,
    kSetupPowerStatic1BasisSum = 0x178,
    kSetupPowerStatic1Offset = 0x184,
    kSetupPowerStatic1RotatedOffset = 0x190,
    kSetupPowerStatic1Position = 0x19c,
    kSetupGolfViewTarget = 0x1a8,
    kSetupBelowHoleLookOffset = 0x1b4,
    kSetupBelowHoleTarget = 0x1c0,
    kSetupBelowHoleOffset = 0x1cc,
    kSetupBelowHoleCup = 0x1d8,
    kSetupBelowHolePosition = 0x1e4,
    kSetupTelephotoHoleOffset = 0x1f0,
    kSetupTelephotoHoleRotatedOffset = 0x1fc,
    kSetupTelephotoHoleOrigin = 0x208,
    kSetupTelephotoHolePosition = 0x214,
    kSetupTelephotoHoleCup = 0x220,
    kSetupTelephotoHoleDirection = 0x22c,
    kSetupNearHoleOffset = 0x238,
    kSetupNearHoleRotatedOffset = 0x244,
    kSetupNearHoleOrigin = 0x250,
    kSetupNearHolePosition = 0x25c,
    kSetupNearHoleCup = 0x268,
    kSetupNearHoleDirection = 0x274,
    kSetupVelocityHoleOffset = 0x280,
    kSetupVelocityHoleCup = 0x28c,
    kSetupVelocityHolePosition = 0x298,
    kSetupSideHoleOffset = 0x2a4,
    kSetupSideHoleRotatedOffset = 0x2b0,
    kSetupSideHoleOrigin = 0x2bc,
    kSetupSideHolePosition = 0x2c8,
    kSetupSideHoleCup = 0x2d4,
    kSetupSideHoleDirection = 0x2e0,
    kSetupReverseHoleTargetCup = 0x2ec,
    kSetupReverseHoleEndpointSum = 0x2f8,
    kSetupReverseHoleMidpointTarget = 0x304,
    kSetupReverseHoleOffset = 0x310,
    kSetupReverseHoleRotatedOffset = 0x31c,
    kSetupReverseHoleOrigin = 0x328,
    kSetupReverseHolePosition = 0x334,
    kSetupReverseHoleCup = 0x340,
    kSetupReverseHoleDirection = 0x34c,
    kSetupShotHoleTargetCup = 0x358,
    kSetupShotHoleEndpointSum = 0x364,
    kSetupShotHoleMidpointTarget = 0x370,
    kSetupShotHoleUnrotatedOffset = 0x37c,
    kSetupShotHoleRotatedOffset = 0x388,
    kSetupShotHolePosition = 0x394,
    kSetupShotHoleCup = 0x3a0,
    kSetupShotHoleDirection = 0x3ac,
    kSetupFrontOffset = 0x3b8,
    kSetupFrontPosition = 0x3c4,
    kSetupFrontTargetRise = 0x3d0,
    kSetupFrontTarget = 0x3dc,
    kSetupMidpointReverseUp = 0x3e8,
    kSetupMidpointReverseCross = 0x3f4,
    kSetupMidpointReverseOffset = 0x400,
    kSetupMidpointReversePosition = 0x40c,
    kSetupMidpointCameraToTarget = 0x418,
    kSetupMidpointUp = 0x424,
    kSetupMidpointSideCross = 0x430,
    kSetupMidpointSideOffset = 0x43c,
    kSetupMidpointPosition = 0x448,
    kSetupMidpointActorToLaunch = 0x454,
    kSetupMidpointLastToLaunch = 0x460,
    kSetupElevatedHoleSignedUp = 0x46c,
    kSetupElevatedHoleSideCross = 0x478,
    kSetupElevatedHoleCombinedDirection = 0x484,
    kSetupElevatedHoleOffset = 0x490,
    kSetupElevatedHoleRise = 0x49c,
    kSetupElevatedHoleOrigin = 0x4a8,
    kSetupElevatedHoleRaisedOrigin = 0x4b4,
    kSetupElevatedHolePosition = 0x4c0,
    kSetupElevatedHoleCup = 0x4cc,
    kSetupElevatedHoleCupToLaunch = 0x4d8,
    kSetupRaisedRearOffset = 0x4e4,
    kSetupRaisedRearRise = 0x4f0,
    kSetupRaisedRearBase = 0x4fc,
    kSetupRaisedRearPosition = 0x508,
    kSetupRaisedSideRise = 0x514,
    kSetupRaisedSideLateralOffset = 0x520,
    kSetupRaisedSideBase = 0x52c,
    kSetupRaisedSidePosition = 0x538,
    kSetupRaisedSideUp = 0x544,
    kSetupRaisedSideCross = 0x550,
    kSetupWalkAction27LongitudinalOffset = 0x55c,
    kSetupWalkAction27RaisedLaunch = 0x568,
    kSetupWalkAction27Position = 0x574,
    kSetupWalkAction27LateralOffset = 0x580,
    kSetupWalkAction27Up = 0x58c,
    kSetupWalkAction27Cross = 0x598,
    kSetupShotFrameTarget = 0x5a4,
    kSetupLaunchTargetLocalOffset = 0x5b0,
    kSetupLaunchTargetWorldOffset = 0x5bc,
    kSetupLaunchTargetPosition = 0x5c8,
    kSetupLaunchCameraLocalOffset = 0x5d4,
    kSetupLaunchCameraWorldOffset = 0x5e0,
    kSetupLaunchCameraPosition = 0x5ec,
    kSetupWideCameraRise = 0x5f8,
    kSetupWideCameraOffset = 0x604,
    kSetupWideCameraBase = 0x610,
    kSetupWideCameraPosition = 0x61c,
    kSetupWideTargetRise = 0x628,
    kSetupWideTarget = 0x634,
    kSetupQuarterCameraRise = 0x640,
    kSetupQuarterCameraOffset = 0x64c,
    kSetupQuarterCameraBase = 0x658,
    kSetupQuarterCameraPosition = 0x664,
    kSetupQuarterTargetRise = 0x670,
    kSetupQuarterTarget = 0x67c,
    kSetupHeadCameraOffset = 0x688,
    kSetupHeadCameraPosition = 0x694,
    kSetupWeightedHeadNub = 0x6a0,
    kSetupWeightedHead = 0x6ac,
    kSetupHeadTarget = 0x6b8,
    kSetupCutoffBisectorPosition = 0x6c4,
    kSetupCutoffBisectorOffset = 0x6d0,
    kSetupCutoffToNextSample = 0x6dc,
    kSetupCutoffToPreviousSample = 0x6e8,
    kSetupCutoffDirectionSum = 0x6f4,
    kSetupLaunchToLastSample = 0x700,
    kSetupHalfLaunchToLastSample = 0x70c,
    kSetupTrajectoryMidpoint = 0x718,
    kSetupSideMidpointPosition = 0x724,
    kSetupSideMidpointOffset = 0x730,
    kSetupSideMidpointSignedUp = 0x73c,
    kSetupSideMidpointCross = 0x748,
    kSetupWalkAction12LongitudinalOffset = 0x754,
    kSetupWalkAction12RaisedLaunch = 0x760,
    kSetupWalkAction12Position = 0x76c,
    kSetupWalkAction12LateralOffset = 0x778,
    kSetupWalkAction12Up = 0x784,
    kSetupWalkAction12Cross = 0x790,
    kSetupWalkAction18LongitudinalOffset = 0x79c,
    kSetupWalkAction18RaisedLaunch = 0x7a8,
    kSetupWalkAction18Position = 0x7b4,
    kSetupWalkAction18LateralOffset = 0x7c0,
    kSetupWalkAction18Up = 0x7cc,
    kSetupWalkAction18Cross = 0x7d8,
    kSetupWalkAction9LongitudinalOffset = 0x7e4,
    kSetupWalkAction9RaisedLaunch = 0x7f0,
    kSetupWalkAction9Position = 0x7fc,
    kSetupWalkAction9LateralOffset = 0x808,
    kSetupWalkAction9Up = 0x814,
    kSetupWalkAction9Cross = 0x820,
    kSetupEventNormalRise = 0x82c,
    kSetupEventNormalOffset = 0x838,
    kSetupEventNormalBase = 0x844,
    kSetupEventNormalTransition = 0x850,
    kSetupEventRecoveryRise = 0x85c,
    kSetupEventRecoveryOffset = 0x868,
    kSetupEventRecoveryBase = 0x874,
    kSetupEventRecoveryTransition = 0x880,
    kSetupRandomizedEventDirection = 0x88c,
    kSetupEventFallbackLaunchToLast = 0x898,
    kSetupSecondEventToLast = 0x8a4,
    kSetupEventHeightOffset = 0x8b0,
    kSetupRaisedEventPosition = 0x8bc,
    kSetupEventTransitionPosition = 0x8c8,
    kSetupEventSideOffset = 0x8d4,
    kSetupEventSideUp = 0x8e0,
    kSetupEventSideCross = 0x8ec,
    kSetupCapturedCurrentTarget = 0x8f8,
    kSetupMidpointLookDirection = 0x904,
    kSetupMidpointActorDirection = 0x910,
    kSetupMidpointActorPosition = 0x91c,
    kSetupMidpointLaunch = 0x928,
    kSetupWalkAction27EventVelocity = 0x934,
    kSetupWalkAction12EventVelocity = 0x940,
    kSetupWalkAction18EventVelocity = 0x94c,
    kSetupWalkAction9EventVelocity = 0x958,
    kSetupDirectionWork = 0x964,
    kSetupMidpointActorMessage = 0x988,
    kSetupRollRotationMatrix = 0x9a8,
    kSetupUnrolledPoseMatrix = 0x9d8,
    kSetupPowerBlend2EndPose = 0xa08,
    kSetupPowerBlend2StartPose = 0xa38,
    kSetupPowerBlend1EndPose = 0xa68,
    kSetupPowerBlend1StartPose = 0xa98,
    kSetupPowerStatic2Rotation = 0xac8,
    kSetupPowerStatic1Rotation = 0xaf8,
    kSetupBlendDestinationGolfMatrix = 0xb28,
    kSetupGolfViewMatrix = 0xb58,
    kSetupTelephotoHoleRotation = 0xb88,
    kSetupNearHoleRotation = 0xbb8,
    kSetupSideHoleRotation = 0xbe8,
    kSetupReverseHoleRotation = 0xc18,
    kSetupShotHoleRotation = 0xc48,
    kSetupFrontHeadingMatrix = 0xc78,
    kSetupFrontForwardBasis = 0xc90,
    kSetupWideHeadingMatrix = 0xca8,
    kSetupWideForwardBasis = 0xcc0,
    kSetupQuarterHeadingMatrix = 0xcd8,
    kSetupQuarterForwardBasis = 0xcf0,
    kSetupHeadHeadingMatrix = 0xd08,
    kSetupHeadForwardBasis = 0xd20,
    kSetupRandomEventRotation = 0xd38,
    kSetupCapturedPoseMatrix = 0xd68,
    kSetupMidpointActorMatrix = 0xd98,
    kSetupMidpointActorMatrixPosition = 0xdbc,
    kSetupLaunchPoseHeading = 0xdc8,
};
enum {
    kCameraSetupBufferOrigin = 0x78,
    kCameraSetupBufferBytes = 0xD80
};

#undef F_
#define F_ ((char*)frame)
// Scratch slots keep the matched stack layout; addresses are relative to the original frame.
static inline void* CameraScratchAddress(char* scratch, int stackOffset) {
    return &scratch[stackOffset - kCameraSetupBufferOrigin];
}
// Borrow a world vector/displacement view; these references never own scratch.
// XYZ components are floats, while CopyCurrentHoleToCameraScratch writes their
// representations through CameraPositionWords. Never numerically convert those bits.
struct CameraScratchVector {
    float x, y, z;
};
// Return the three-float view at an existing slot, without extending its lifetime.
static inline CameraScratchVector& CameraVectorAt(char* frame, int slot) {
    return *(CameraScratchVector*)&frame[slot - kCameraSetupBufferOrigin];
}
struct CallbackDescriptorPrefix {
    int a, b;
};
struct CameraCallbackDescriptor {
    CallbackDescriptorPrefix ab;
    int c;
};
// Borrow the descriptor view of the mixed weights/descriptor data anchor; never index it as weights.
static inline CameraCallbackDescriptor* CameraCallbackDescriptors() {
    return (CameraCallbackDescriptor*)s_extendedCameraModeWeights;
}
// Action-to-callback map: indices address existing three-word descriptors, never a new runtime table.
// Neutral ActionN names retain unresolved callback roles; each geometry case below names its setup.
// Complete mode definition index: action -> setup definition / running callback.
// Callback names below omit the common CCameraWork_ prefix except CameraWork_*.
// Descriptor byte offsets are the kCallback* enum immediately below; source-owned
// "callback only" means there are no additional local geometric tuning literals;
// inspect that named evaluator and BuildModeCurve for its parameters.
//   0 callback only / UpdateBezierPositionAndBallTarget
//   1 callback only (curveStart=-1) / UpdateBezierPositionAndBallTracking
//   2 callback only / UpdateBezierBallTracking
//   3 ConfigureEventSideCamera / UpdatePassByTracking
//   4 ConfigureSecondEventCamera / UpdateBezierTarget
//  11 ConfigureSecondEventCamera / UpdateBallToBezierTarget
//   5 callback only / NoopA; 7 curveStart=-1 / NoopB
//   8 callback only / UpdateBezierTrackingWithEventCut
//   9 ConfigureCameraWalkPose(start=-1) / UpdateInitialBallZoom
//  10 callback only / SetCaddieHeadView
//  18 ConfigureCameraWalkPose(start=-1,heading=.2617994) / UpdateBezierOrbit
//  19 time=0,start=-1,heading=0 / UpdateShotSideTargetRotation
//  12 ConfigureCameraWalkPose(start=-1) / UpdateBezierPositionAndRaisedBallTarget
//  13 ConfigureTrajectorySideMidpointCamera / UpdateBallTargetWithEventCut
//  14 ConfigureCutoffBisectorCamera / CameraWork_UpdateBallTargetWithRisingOffset
//  20 ConfigurePlayerHeadCamera / RequestQuarterViewAfterDelay
//  21 ConfigureLaunchQuarterCamera / SetTargetToBall
//  22 ConfigureLaunchWideCamera / CameraWork_InterpolatePreviousAndCurrentBallTarget
//  23 curveStart=0 / UpdateRollingShotApproach
//  24 ConfigurePowerLaunchCamera / UpdateBallPathSideView
//  25 ConfigurePowerTrajectoryCamera / UpdatePreDataBallFollow
//  26 start=-1 if variant/group else 15 / UpdateShortShotRelativeMove
//  27 ConfigureCameraWalkPose(start=15) / UpdateEventDrivenBallOrbit
// 100 callback only / TrackBallPitchAndSelectHoleView
// 101 ConfigureRaisedLaunchSideCamera / AdvanceAlongBallHorizontalVector
// 102 ConfigureRaisedLaunchRearCamera / TrackBallPitchAndSelectHoleView
// 103 ConfigureElevatedHoleSideCamera / TrackBallPitchAndSelectHoleView
// 104 ConfigureTrajectoryMidpointCamera / TrackBallPitchAndSelectHoleView
// 105 callback only / BlendShotSidePosition
// 106 ConfigureLaunchFrontCamera(clearance off) / PositionBehindMovingBall
// 107 ConfigureShotAndHoleCamera / ChangeAngleWhenBallLeavesView
// 108 ConfigureHoleReverseCamera / ChangeAngleWhenBallLeavesView
// 109 ConfigureHoleSideCamera / TrackBallPitchAndSelectHoleView
// 110 ConfigureHoleVelocityCamera / TrackBallPitchAndSelectHoleView
// 111 ConfigureHoleNearCamera / LowerPositionAndTrackBall
// 112 ConfigureHoleTelephotoCamera / RetreatFromHoleAndWidenFov
// 150 ConfigureBelowHoleCamera / SetBelowHoleView
// 200 callback only / NoopC; 201 callback only / NoopD
// 202 curveStart=-1 / InterpolatePositionAndDirection
// 203 ConfigureBallOrbitCamera(clearance off) / UpdateDescendingBallOrbit
// 204 ConfigureGolfPoseBlendCamera / InterpolatePositionAndOrientation
// 205 ConfigureNarrowGolfCamera / RestoreNormalFov
// 206 ConfigureStaticPowerShotCamera / cleared callback (static pose)
// 207 ConfigurePowerShotBlendCamera / InterpolatePositionAndQuaternion
// 208 ConfigureRaisedHoleOrbitCamera(substate=0) / NoopE
// other actions: default descriptor / UpdateBezierPositionAndBallTarget.
// Geometry helper inputs/outputs use world distance, normalized directions and
// radians as stated locally. E.g. WalkPose couples lateral40/rise8/jitter[-5,5]/
// targetRise2; Telephoto couples distance60/yaw-.05235988/FOV.17453292; NarrowGolf
// couples FOV.017453292 with duration1s. Larger offsets move farther along the
// indicated basis; higher FOV widens framing. No tested tuning ranges established.
enum CameraCallbackSlot {
    kCallbackAction0 = 0xa8 / sizeof(CameraCallbackDescriptor),
    kCallbackAction1 = 0xb4 / sizeof(CameraCallbackDescriptor),
    kCallbackAction2 = 0xc0 / sizeof(CameraCallbackDescriptor),
    kCallbackEventSideView = 0xcc / sizeof(CameraCallbackDescriptor),
    kCallbackSecondEventView = 0xd8 / sizeof(CameraCallbackDescriptor),
    kCallbackSecondEventAlternateView = 0xe4 / sizeof(CameraCallbackDescriptor),
    kCallbackAction5 = 0xf0 / sizeof(CameraCallbackDescriptor),
    kCallbackAction7 = 0xfc / sizeof(CameraCallbackDescriptor),
    kCallbackAction8 = 0x108 / sizeof(CameraCallbackDescriptor),
    kCallbackWalkView9 = 0x114 / sizeof(CameraCallbackDescriptor),
    kCallbackAction10 = 0x120 / sizeof(CameraCallbackDescriptor),
    kCallbackWalkView18 = 0x12c / sizeof(CameraCallbackDescriptor),
    kCallbackAction19 = 0x138 / sizeof(CameraCallbackDescriptor),
    kCallbackWalkView12 = 0x144 / sizeof(CameraCallbackDescriptor),
    kCallbackTrajectorySideMidpointView = 0x150 / sizeof(CameraCallbackDescriptor),
    kCallbackCutoffBisectorView = 0x15c / sizeof(CameraCallbackDescriptor),
    kCallbackPlayerHeadView = 0x168 / sizeof(CameraCallbackDescriptor),
    kCallbackLaunchQuarterView = 0x174 / sizeof(CameraCallbackDescriptor),
    kCallbackLaunchRandomWideView = 0x180 / sizeof(CameraCallbackDescriptor),
    kCallbackAction23 = 0x18c / sizeof(CameraCallbackDescriptor),
    kCallbackPowerLaunchView = 0x198 / sizeof(CameraCallbackDescriptor),
    kCallbackPowerTrajectoryView = 0x1a4 / sizeof(CameraCallbackDescriptor),
    kCallbackPowerStartView = 0x1b0 / sizeof(CameraCallbackDescriptor),
    kCallbackWalkView27 = 0x1bc / sizeof(CameraCallbackDescriptor),
    kCallbackTrackBallPitch = 0x1c8 / sizeof(CameraCallbackDescriptor),
    kCallbackRaisedLaunchSideView = 0x1d4 / sizeof(CameraCallbackDescriptor),
    kCallbackRaisedLaunchRearView = 0x1e0 / sizeof(CameraCallbackDescriptor),
    kCallbackElevatedHoleSideView = 0x1ec / sizeof(CameraCallbackDescriptor),
    kCallbackTrajectoryMidpointView = 0x1f8 / sizeof(CameraCallbackDescriptor),
    kCallbackAction105 = 0x204 / sizeof(CameraCallbackDescriptor),
    kCallbackLaunchFrontView = 0x210 / sizeof(CameraCallbackDescriptor),
    kCallbackShotAndHoleView = 0x21c / sizeof(CameraCallbackDescriptor),
    kCallbackHoleReverseView = 0x228 / sizeof(CameraCallbackDescriptor),
    kCallbackHoleSideView = 0x234 / sizeof(CameraCallbackDescriptor),
    kCallbackHoleVelocityView = 0x240 / sizeof(CameraCallbackDescriptor),
    kCallbackHoleNearView = 0x24c / sizeof(CameraCallbackDescriptor),
    kCallbackHoleTelephotoView = 0x258 / sizeof(CameraCallbackDescriptor),
    kCallbackBelowHoleView = 0x264 / sizeof(CameraCallbackDescriptor),
    kCallbackNarrowGolfView = 0x270 / sizeof(CameraCallbackDescriptor),
    kCallbackOrbitToBall = 0x27c / sizeof(CameraCallbackDescriptor),
    kCallbackBlendToGolfView = 0x288 / sizeof(CameraCallbackDescriptor),
    kCallbackAction200 = 0x294 / sizeof(CameraCallbackDescriptor),
    kCallbackAction201 = 0x2a0 / sizeof(CameraCallbackDescriptor),
    kCallbackAction202 = 0x2ac / sizeof(CameraCallbackDescriptor),
    kCallbackPowerShotBlendView = 0x2b8 / sizeof(CameraCallbackDescriptor),
    kCallbackRaisedHoleOrbit = 0x2c4 / sizeof(CameraCallbackDescriptor),
    kCallbackDefault = 0x2d0 / sizeof(CameraCallbackDescriptor),
};

// Install one borrowed ABI descriptor; switch exit is explicit at every call site.
static inline void InstallCameraCallback(CCameraWork* self, const CameraCallbackDescriptor* descriptors,
                                         CameraCallbackSlot slot) {
    *(CameraCallbackDescriptor*)&self->motionCallbackThisAdjustment = descriptors[slot];
}

#undef G90
#define G90 ((char*)AsCSharedDoc(g_sharedDoc)->golfDoc) // original +0xb90
// Hole records have a 32-byte stride and bit-copied XYZ position words at +0x0c.
struct CameraPositionXYWords {
    int x, y;
};
struct CameraPositionWords {
    CameraPositionXYWords xy;
    int z;
};
struct CameraHoleRecordView {
    char reserved00[0xc];
    CameraPositionWords position;
    char reserved18[8];
};
typedef char CameraHoleRecordStride[(sizeof(CameraHoleRecordView) == 32) ? 1 : -1];
// Copy the current cup's world position into caller-owned scratch without float conversions.
static inline void CopyCurrentHoleToCameraScratch(char* frame, int slot) {
    *(CameraPositionWords*)CameraScratchAddress(frame, slot) =
        ((CameraHoleRecordView*)AsCGolfDoc(G90)->holeRecords)[AsCGolfDoc(G90)->currentHoleIndex].position;
}
#undef OBJ
#define OBJ(o) ((void*)&((char*)self)[o])

// Initialize the shared walk pose from launch position and the first event's velocity.
// Output: position = launch + 40*normalize(up x eventVelocity) + 8Y + random[-5,5]*normalize(eventVelocity);
// target = launch+2Y. Each slot argument names caller-owned scratch with the original lifetime.
static inline void ConfigureCameraWalkPose(CCameraWork* self, char* frame, char* ball, int clampedFirstEventIndex,
                                           float startFrame, int upSlot, int crossSlot, int lateralSlot,
                                           int velocitySlot, int longitudinalSlot, int raisedLaunchSlot,
                                           int worldPositionSlot) {
    self->motionStartFrame = startFrame;
    CameraVectorAt(frame, upSlot).x = 0.0f;
    CameraVectorAt(frame, upSlot).y = 1.0f;
    CameraVectorAt(frame, upSlot).z = 0.0f;
    Vector_CrossProduct(CameraScratchAddress(frame, crossSlot), CameraScratchAddress(frame, upSlot),
                        ((CameraTrajectorySample*)PreData__9CGolfBallFi(ball, clampedFirstEventIndex))->velocity);
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork), CameraScratchAddress(frame, crossSlot));
    __ml__FRC7WVectorf(CameraScratchAddress(frame, lateralSlot),
                       Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupDirectionWork)), 40.0f);
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              CameraScratchAddress(frame, lateralSlot));
    CameraVectorAt(frame, kSetupDirectionWork).y += 8.0f;
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, velocitySlot),
                              ((CameraTrajectorySample*)PreData__9CGolfBallFi(ball, clampedFirstEventIndex))->velocity);
    {
        void* normalizedEventVelocity = Normalize__7WVectorFv(CameraScratchAddress(frame, velocitySlot));
        __ml__FRC7WVectorf(CameraScratchAddress(frame, longitudinalSlot), normalizedEventVelocity,
                           (float)(self->randomValue % 11) - 5.0f);
    }
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, raisedLaunchSlot), PreData__9CGolfBallFi(ball, 0),
                                CameraScratchAddress(frame, kSetupDirectionWork));
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, worldPositionSlot),
                                CameraScratchAddress(frame, raisedLaunchSlot),
                                CameraScratchAddress(frame, longitudinalSlot));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, worldPositionSlot));
    __as__7WVectorFRC7WVector(self->target, PreData__9CGolfBallFi(ball, 0));
    self->target[1] += 2.0f;
}

#pragma opt_propagation off
// Place a raised world camera along the bisector of the two cutoff-neighbor directions.
static inline void ConfigureCutoffBisectorCamera(CCameraWork* self, char* frame, char* ball, void*& nextDirection) {
    self->motionStartFrame = -1.0f;
    char* cutoffSample = PreData__9CGolfBallFi(ball, self->cutoffFrame);
    char* neighborSample = PreData__9CGolfBallFi(ball, self->cutoffFrame + 1);
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupCutoffToNextSample), neighborSample, cutoffSample);
    nextDirection = Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupCutoffToNextSample));
    cutoffSample = PreData__9CGolfBallFi(ball, self->cutoffFrame);
    neighborSample = PreData__9CGolfBallFi(ball, self->cutoffFrame - 1);
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupCutoffToPreviousSample), neighborSample,
                                cutoffSample);
    void* previousDirection = Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupCutoffToPreviousSample));
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupCutoffDirectionSum), previousDirection,
                                nextDirection);
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              CameraScratchAddress(frame, kSetupCutoffDirectionSum));
    CameraVectorAt(frame, kSetupDirectionWork).y = 0.0f;
    __ml__FRC7WVectorf(CameraScratchAddress(frame, kSetupCutoffBisectorOffset),
                       Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupDirectionWork)), 160.0f);
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              CameraScratchAddress(frame, kSetupCutoffBisectorOffset));
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupCutoffBisectorPosition),
                                PreData__9CGolfBallFi(ball, self->cutoffFrame),
                                CameraScratchAddress(frame, kSetupDirectionWork));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupCutoffBisectorPosition));
    self->position[1] += 96.0f;
}

// Frame the launch from a raised quarter-turn offset in world space.
static inline void ConfigureLaunchQuarterCamera(CCameraWork* self, char* frame, char* ball) {
    self->heading = 1.5707964f;
    CameraVectorAt(frame, kSetupQuarterTargetRise).x = 0.0f;
    CameraVectorAt(frame, kSetupQuarterTargetRise).y = 3.0f;
    CameraVectorAt(frame, kSetupQuarterTargetRise).z = 0.0f;
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupQuarterTarget), PreData__9CGolfBallFi(ball, 0),
                                CameraScratchAddress(frame, kSetupQuarterTargetRise));
    __as__7WVectorFRC7WVector(self->target, CameraScratchAddress(frame, kSetupQuarterTarget));
    CameraVectorAt(frame, kSetupQuarterCameraRise).x = 0.0f;
    CameraVectorAt(frame, kSetupQuarterCameraRise).y = 3.0f;
    CameraVectorAt(frame, kSetupQuarterCameraRise).z = 0.0f;
    Matrix_SetAxisRotation(CameraScratchAddress(frame, kSetupQuarterHeadingMatrix),
                           self->heading + AsCGolfDoc(G90)->headingRadians, 1);
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupQuarterCameraOffset),
                       CameraScratchAddress(frame, kSetupQuarterForwardBasis), 15.0f);
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupQuarterCameraBase), PreData__9CGolfBallFi(ball, 0),
                                CameraScratchAddress(frame, kSetupQuarterCameraOffset));
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupQuarterCameraPosition),
                                CameraScratchAddress(frame, kSetupQuarterCameraBase),
                                CameraScratchAddress(frame, kSetupQuarterCameraRise));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupQuarterCameraPosition));
}

// Frame the launch from its horizontal velocity cross product.
static inline void ConfigureRaisedLaunchSideCamera(CCameraWork* self, char* frame, char* ball) {
    CameraVectorAt(frame, kSetupRaisedSideUp).x = 0.0f;
    CameraVectorAt(frame, kSetupRaisedSideUp).y = 1.0f;
    CameraVectorAt(frame, kSetupRaisedSideUp).z = 0.0f;
    Vector_CrossProduct(CameraScratchAddress(frame, kSetupRaisedSideCross),
                        CameraScratchAddress(frame, kSetupRaisedSideUp),
                        ((CameraTrajectorySample*)PreData__9CGolfBallFi(ball, 0))->velocity);
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              CameraScratchAddress(frame, kSetupRaisedSideCross));
    Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupDirectionWork));
    CameraVectorAt(frame, kSetupRaisedSideRise).x = 0.0f;
    CameraVectorAt(frame, kSetupRaisedSideRise).y = 32.0f;
    CameraVectorAt(frame, kSetupRaisedSideRise).z = 0.0f;
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupRaisedSideLateralOffset),
                       CameraScratchAddress(frame, kSetupDirectionWork), 16.0f);
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupRaisedSideBase), PreData__9CGolfBallFi(ball, 0),
                                CameraScratchAddress(frame, kSetupRaisedSideLateralOffset));
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupRaisedSidePosition),
                                CameraScratchAddress(frame, kSetupRaisedSideBase),
                                CameraScratchAddress(frame, kSetupRaisedSideRise));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupRaisedSidePosition));
    __as__7WVectorFRC7WVector(self->target, PreData__9CGolfBallFi(ball, 0));
    self->target[1] += 2.5f;
}

// Place a raised camera behind the launch velocity.
static inline void ConfigureRaisedLaunchRearCamera(CCameraWork* self, char* frame, char* ball) {
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              ((CameraTrajectorySample*)PreData__9CGolfBallFi(ball, 0))->velocity);
    CameraVectorAt(frame, kSetupRaisedRearRise).x = 0.0f;
    CameraVectorAt(frame, kSetupRaisedRearRise).y = 25.0f;
    CameraVectorAt(frame, kSetupRaisedRearRise).z = 0.0f;
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupRaisedRearOffset),
                       Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupDirectionWork)), 5.0f);
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupRaisedRearBase), PreData__9CGolfBallFi(ball, 0),
                                CameraScratchAddress(frame, kSetupRaisedRearRise));
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupRaisedRearPosition),
                                CameraScratchAddress(frame, kSetupRaisedRearBase),
                                CameraScratchAddress(frame, kSetupRaisedRearOffset));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupRaisedRearPosition));
    __as__7WVectorFRC7WVector(self->target, PreData__9CGolfBallFi(ball, 0));
}

// Frame the launch using the golf heading and a forward offset.
static inline void ConfigureLaunchFrontCamera(CCameraWork* self, char* frame, char* ball) {
    CameraVectorAt(frame, kSetupFrontTargetRise).x = 0.0f;
    CameraVectorAt(frame, kSetupFrontTargetRise).y = 3.0f;
    CameraVectorAt(frame, kSetupFrontTargetRise).z = 0.0f;
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupFrontTarget), PreData__9CGolfBallFi(ball, 0),
                                CameraScratchAddress(frame, kSetupFrontTargetRise));
    __as__7WVectorFRC7WVector(self->target, CameraScratchAddress(frame, kSetupFrontTarget));
    Matrix_SetAxisRotation(CameraScratchAddress(frame, kSetupFrontHeadingMatrix),
                           1.7453293f + AsCGolfDoc(G90)->headingRadians, 1);
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupFrontOffset),
                       CameraScratchAddress(frame, kSetupFrontForwardBasis), 16.0f);
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupFrontPosition), self->target,
                                CameraScratchAddress(frame, kSetupFrontOffset));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupFrontPosition));
}

// Look at the launch/cup midpoint from a rotated launch offset.
static inline void ConfigureShotAndHoleCamera(CCameraWork* self, char* frame, char* ball) {
    CopyCurrentHoleToCameraScratch(frame, kSetupShotHoleCup);
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupShotHoleDirection),
                                CameraScratchAddress(frame, kSetupShotHoleCup), PreData__9CGolfBallFi(ball, 0));
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              CameraScratchAddress(frame, kSetupShotHoleDirection));
    CameraVectorAt(frame, kSetupDirectionWork).y = 0.0f;
    Matrix_SetAxisRotation(CameraScratchAddress(frame, kSetupShotHoleRotation), -2.6179938f, 1);
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupShotHoleUnrotatedOffset),
                       Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupDirectionWork)), 20.0f);
    Matrix_TransformPoint(CameraScratchAddress(frame, kSetupShotHoleRotatedOffset),
                          CameraScratchAddress(frame, kSetupShotHoleUnrotatedOffset),
                          CameraScratchAddress(frame, kSetupShotHoleRotation));
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupShotHolePosition), PreData__9CGolfBallFi(ball, 0),
                                CameraScratchAddress(frame, kSetupShotHoleRotatedOffset));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupShotHolePosition));
    self->position[1] += 20.0f;
    CopyCurrentHoleToCameraScratch(frame, kSetupShotHoleTargetCup);
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupShotHoleEndpointSum), PreData__9CGolfBallFi(ball, 0),
                                CameraScratchAddress(frame, kSetupShotHoleTargetCup));
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupShotHoleMidpointTarget),
                       CameraScratchAddress(frame, kSetupShotHoleEndpointSum), 0.5f);
    __as__7WVectorFRC7WVector(self->target, CameraScratchAddress(frame, kSetupShotHoleMidpointTarget));
}

// Look at the launch/cup midpoint from a reverse cup offset.
static inline void ConfigureHoleReverseCamera(CCameraWork* self, char* frame, char* ball) {
    CopyCurrentHoleToCameraScratch(frame, kSetupReverseHoleCup);
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupReverseHoleDirection),
                                CameraScratchAddress(frame, kSetupReverseHoleCup), PreData__9CGolfBallFi(ball, 0));
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              CameraScratchAddress(frame, kSetupReverseHoleDirection));
    CameraVectorAt(frame, kSetupDirectionWork).y = 0.0f;
    CopyCurrentHoleToCameraScratch(frame, kSetupReverseHoleOrigin);
    Matrix_SetAxisRotation(CameraScratchAddress(frame, kSetupReverseHoleRotation), -1.0471976f, 1);
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupReverseHoleOffset),
                       Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupDirectionWork)), 20.0f);
    Matrix_TransformPoint(CameraScratchAddress(frame, kSetupReverseHoleRotatedOffset),
                          CameraScratchAddress(frame, kSetupReverseHoleOffset),
                          CameraScratchAddress(frame, kSetupReverseHoleRotation));
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupReverseHolePosition),
                                CameraScratchAddress(frame, kSetupReverseHoleOrigin),
                                CameraScratchAddress(frame, kSetupReverseHoleRotatedOffset));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupReverseHolePosition));
    self->position[1] += 20.0f;
    CopyCurrentHoleToCameraScratch(frame, kSetupReverseHoleTargetCup);
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupReverseHoleEndpointSum),
                                PreData__9CGolfBallFi(ball, 0),
                                CameraScratchAddress(frame, kSetupReverseHoleTargetCup));
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupReverseHoleMidpointTarget),
                       CameraScratchAddress(frame, kSetupReverseHoleEndpointSum), 0.5f);
    __as__7WVectorFRC7WVector(self->target, CameraScratchAddress(frame, kSetupReverseHoleMidpointTarget));
}

// Frame the live ball from a perpendicular cup offset.
static inline void ConfigureHoleSideCamera(CCameraWork* self, char* frame, char* ball) {
    CopyCurrentHoleToCameraScratch(frame, kSetupSideHoleCup);
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupSideHoleDirection),
                                CameraScratchAddress(frame, kSetupSideHoleCup), PreData__9CGolfBallFi(ball, 0));
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              CameraScratchAddress(frame, kSetupSideHoleDirection));
    CameraVectorAt(frame, kSetupDirectionWork).y = 0.0f;
    CopyCurrentHoleToCameraScratch(frame, kSetupSideHoleOrigin);
    Matrix_SetAxisRotation(CameraScratchAddress(frame, kSetupSideHoleRotation), -1.5707964f, 1);
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupSideHoleOffset),
                       Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupDirectionWork)), 10.0f);
    Matrix_TransformPoint(CameraScratchAddress(frame, kSetupSideHoleRotatedOffset),
                          CameraScratchAddress(frame, kSetupSideHoleOffset),
                          CameraScratchAddress(frame, kSetupSideHoleRotation));
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupSideHolePosition),
                                CameraScratchAddress(frame, kSetupSideHoleOrigin),
                                CameraScratchAddress(frame, kSetupSideHoleRotatedOffset));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupSideHolePosition));
    self->position[1] += 20.0f;
    __as__7WVectorFRC7WVector(self->target, ball);
}

// Frame the live ball from an offset opposite its velocity at the cup.
static inline void ConfigureHoleVelocityCamera(CCameraWork* self, char* frame, char* ball) {
    CopyCurrentHoleToCameraScratch(frame, kSetupVelocityHoleCup);
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupVelocityHoleOffset), Normalize__7WVectorFv(ball + 0xc), 10.0f);
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupVelocityHolePosition),
                                CameraScratchAddress(frame, kSetupVelocityHoleCup),
                                CameraScratchAddress(frame, kSetupVelocityHoleOffset));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupVelocityHolePosition));
    self->position[1] += 10.0f;
    __as__7WVectorFRC7WVector(self->target, ball);
}

// Frame the live ball from a short rotated cup offset.
static inline void ConfigureHoleNearCamera(CCameraWork* self, char* frame, char* ball) {
    CopyCurrentHoleToCameraScratch(frame, kSetupNearHoleCup);
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupNearHoleDirection),
                                CameraScratchAddress(frame, kSetupNearHoleCup), ball);
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              CameraScratchAddress(frame, kSetupNearHoleDirection));
    CameraVectorAt(frame, kSetupDirectionWork).y = 0.0f;
    CopyCurrentHoleToCameraScratch(frame, kSetupNearHoleOrigin);
    Matrix_SetAxisRotation(CameraScratchAddress(frame, kSetupNearHoleRotation), -0.34906584f, 1);
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupNearHoleOffset),
                       Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupDirectionWork)), 15.0f);
    Matrix_TransformPoint(CameraScratchAddress(frame, kSetupNearHoleRotatedOffset),
                          CameraScratchAddress(frame, kSetupNearHoleOffset),
                          CameraScratchAddress(frame, kSetupNearHoleRotation));
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupNearHolePosition),
                                CameraScratchAddress(frame, kSetupNearHoleOrigin),
                                CameraScratchAddress(frame, kSetupNearHoleRotatedOffset));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupNearHolePosition));
    self->position[1] += 20.0f;
    __as__7WVectorFRC7WVector(self->target, ball);
}

// Frame the ball from a distant cup offset with a narrow field of view.
static inline void ConfigureHoleTelephotoCamera(CCameraWork* self, char* frame, char* ball) {
    CopyCurrentHoleToCameraScratch(frame, kSetupTelephotoHoleCup);
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupTelephotoHoleDirection),
                                CameraScratchAddress(frame, kSetupTelephotoHoleCup), PreData__9CGolfBallFi(ball, 0));
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              CameraScratchAddress(frame, kSetupTelephotoHoleDirection));
    CameraVectorAt(frame, kSetupDirectionWork).y = 0.0f;
    Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupDirectionWork));
    CopyCurrentHoleToCameraScratch(frame, kSetupTelephotoHoleOrigin);
    Matrix_SetAxisRotation(CameraScratchAddress(frame, kSetupTelephotoHoleRotation), -0.05235988f, 1);
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupTelephotoHoleOffset),
                       CameraScratchAddress(frame, kSetupDirectionWork), 60.0f);
    Matrix_TransformPoint(CameraScratchAddress(frame, kSetupTelephotoHoleRotatedOffset),
                          CameraScratchAddress(frame, kSetupTelephotoHoleOffset),
                          CameraScratchAddress(frame, kSetupTelephotoHoleRotation));
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupTelephotoHolePosition),
                                CameraScratchAddress(frame, kSetupTelephotoHoleOrigin),
                                CameraScratchAddress(frame, kSetupTelephotoHoleRotatedOffset));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupTelephotoHolePosition));
    __as__7WVectorFRC7WVector(self->target, ball);
    self->fov = 0.17453292f;
}

// Place the camera below the cup and look slightly upward.
static inline void ConfigureBelowHoleCamera(CCameraWork* self, char* frame) {
    CameraVectorAt(frame, kSetupBelowHoleOffset).x = 0.0f;
    CameraVectorAt(frame, kSetupBelowHoleOffset).y = -2.0f;
    CameraVectorAt(frame, kSetupBelowHoleOffset).z = 0.0f;
    CopyCurrentHoleToCameraScratch(frame, kSetupBelowHoleCup);
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupBelowHolePosition),
                                CameraScratchAddress(frame, kSetupBelowHoleCup),
                                CameraScratchAddress(frame, kSetupBelowHoleOffset));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupBelowHolePosition));
    CameraVectorAt(frame, kSetupBelowHoleLookOffset).x = 0.0f;
    CameraVectorAt(frame, kSetupBelowHoleLookOffset).y = 1.0f;
    CameraVectorAt(frame, kSetupBelowHoleLookOffset).z = 0.2f;
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupBelowHoleTarget), self->position,
                                CameraScratchAddress(frame, kSetupBelowHoleLookOffset));
    __as__7WVectorFRC7WVector(self->target, CameraScratchAddress(frame, kSetupBelowHoleTarget));
}

// Capture the golf-document pose and initialize its narrow-FOV transition.
static inline void ConfigureNarrowGolfCamera(CCameraWork* self, char* frame) {
    CGolfDoc::BuildCameraMatrix(CameraScratchAddress(frame, kSetupGolfViewMatrix), G90);
    Matrix_CopyAffine(self->previousTransform, CameraScratchAddress(frame, kSetupGolfViewMatrix));
    __as__7WVectorFRC7WVector(self->position, &self->previousTransform[9]);
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupGolfViewTarget), self->position,
                                &self->previousTransform[6]);
    __as__7WVectorFRC7WVector(self->target, CameraScratchAddress(frame, kSetupGolfViewTarget));
    self->fov = 0.017453292f;
    self->duration = 1.0f;
    self->animationTime = 0.0f;
    self->motionStartFrame = -1.0f;
}

// Initialize ball-orbit distance, height and heading before disabling ground clearance.
static inline void ConfigureBallOrbitCamera(CCameraWork* self) {
    self->heading = 2.6179938f;
    self->horizontalBallDistance = 80.0f;
    self->ballHeightOffset = 15.0f;
    self->motionStartFrame = -1.0f;
}

// Initialize quaternion and world-position endpoints for the golf-view blend.
static inline void ConfigureGolfPoseBlendCamera(CCameraWork* self, char* frame) {
    Matrix3x3_ToQuat(self->startQuaternion, self->previousTransform);
    CGolfDoc::BuildCameraMatrix(CameraScratchAddress(frame, kSetupBlendDestinationGolfMatrix), G90);
    Matrix3x3_ToQuat(self->endQuaternion, CameraScratchAddress(frame, kSetupBlendDestinationGolfMatrix));
    __as__7WVectorFRC7WVector(self->startPosition, &self->previousTransform[9]);
    __as__7WVectorFRC7WVector(self->endPosition, G90 + 0x348);
    self->animationTime = 0.0f;
    self->duration = 1.0f;
    self->motionStartFrame = -1.0f;
}

// Initialize the raised cup target and orbit distance.
static inline void ConfigureRaisedHoleOrbitCamera(CCameraWork* self, char* frame) {
    self->motionStartFrame = -1.0f;
    CopyCurrentHoleToCameraScratch(frame, kSetupRaisedOrbitCup);
    __as__7WVectorFRC7WVector(self->target, CameraScratchAddress(frame, kSetupRaisedOrbitCup));
    self->target[1] += 3.0f;
    self->heading = 0.0f;
    self->horizontalBallDistance = 64.0f;
}

// Choose a randomized side/height offset around the first trajectory event.
static inline void ConfigureEventSideCamera(CCameraWork* self, char* frame, char* ball, int clampedFirstEventIndex) {
    float sideSign = (self->randomValue % 2) ? 1.0f : -1.0f;
    CameraVectorAt(frame, kSetupEventSideUp).x = 0.0f;
    CameraVectorAt(frame, kSetupEventSideUp).y = sideSign;
    CameraVectorAt(frame, kSetupEventSideUp).z = 0.0f;
    Vector_CrossProduct(CameraScratchAddress(frame, kSetupEventSideCross),
                        CameraScratchAddress(frame, kSetupEventSideUp),
                        ((CameraTrajectorySample*)PreData__9CGolfBallFi(ball, clampedFirstEventIndex))->velocity);
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              CameraScratchAddress(frame, kSetupEventSideCross));
    __ml__FRC7WVectorf(CameraScratchAddress(frame, kSetupEventSideOffset),
                       Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupDirectionWork)), 10.0f);
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              CameraScratchAddress(frame, kSetupEventSideOffset));
    float heightOffset = (float)(self->randomValue % 11) - 5.0f;
    CameraVectorAt(frame, kSetupEventHeightOffset).x = 0.0f;
    CameraVectorAt(frame, kSetupEventHeightOffset).z = 0.0f;
    CameraVectorAt(frame, kSetupEventHeightOffset).y = heightOffset;
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupRaisedEventPosition),
                                PreData__9CGolfBallFi(ball, clampedFirstEventIndex),
                                CameraScratchAddress(frame, kSetupEventHeightOffset));
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupEventTransitionPosition),
                                CameraScratchAddress(frame, kSetupRaisedEventPosition),
                                CameraScratchAddress(frame, kSetupDirectionWork));
    __as__7WVectorFRC7WVector(self->motionStartPosition, CameraScratchAddress(frame, kSetupEventTransitionPosition));
    self->actionFlags = self->randomValue % 2 + 1;
}

// Choose a normal or recovery transition offset from second-event travel.
static inline void ConfigureSecondEventCamera(CCameraWork* self, char* frame, char* ball, int lastSampleIndex,
                                              int clampedSecondEventIndex) {
    char* eventSample = PreData__9CGolfBallFi(ball, clampedSecondEventIndex);
    char* lastSample = PreData__9CGolfBallFi(ball, lastSampleIndex);
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupSecondEventToLast), lastSample, eventSample);
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              CameraScratchAddress(frame, kSetupSecondEventToLast));
    if (CameraIsNearZero(CameraVectorAt(frame, kSetupDirectionWork).x) &&
        CameraIsNearZero(CameraVectorAt(frame, kSetupDirectionWork).z)) {
        char* launchSample = PreData__9CGolfBallFi(ball, 0);
        char* fallbackLastSample = PreData__9CGolfBallFi(ball, lastSampleIndex);
        __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupEventFallbackLaunchToLast), fallbackLastSample,
                                    launchSample);
        __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                                  CameraScratchAddress(frame, kSetupEventFallbackLaunchToLast));
        CameraVectorAt(frame, kSetupDirectionWork).y = 0.0f;
        Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupDirectionWork));
    } else {
        CameraVectorAt(frame, kSetupDirectionWork).y = 0.0f;
        float headingOffset = Random_RangeFloat(self->randomValue, -2.0943951f, 2.0943951f);
        Matrix_SetAxisRotation(CameraScratchAddress(frame, kSetupRandomEventRotation), headingOffset, 1);
        Matrix_TransformDirection(CameraScratchAddress(frame, kSetupRandomizedEventDirection),
                                  Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupDirectionWork)),
                                  CameraScratchAddress(frame, kSetupRandomEventRotation));
        __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                                  CameraScratchAddress(frame, kSetupRandomizedEventDirection));
    }
    if (((CGolfBall*)ball)->m_recoveryRequired) {
        CameraVectorAt(frame, kSetupEventRecoveryRise).x = 0.0f;
        CameraVectorAt(frame, kSetupEventRecoveryRise).y = 25.0f;
        CameraVectorAt(frame, kSetupEventRecoveryRise).z = 0.0f;
        __ml__FRC7WVectorf(CameraScratchAddress(frame, kSetupEventRecoveryOffset),
                           CameraScratchAddress(frame, kSetupDirectionWork), 50.0f);
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupEventRecoveryBase),
                                    PreData__9CGolfBallFi(ball, lastSampleIndex),
                                    CameraScratchAddress(frame, kSetupEventRecoveryOffset));
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupEventRecoveryTransition),
                                    CameraScratchAddress(frame, kSetupEventRecoveryBase),
                                    CameraScratchAddress(frame, kSetupEventRecoveryRise));
        __as__7WVectorFRC7WVector(self->motionStartPosition,
                                  CameraScratchAddress(frame, kSetupEventRecoveryTransition));
    } else {
        CameraVectorAt(frame, kSetupEventNormalRise).x = 0.0f;
        CameraVectorAt(frame, kSetupEventNormalRise).y = 10.0f;
        CameraVectorAt(frame, kSetupEventNormalRise).z = 0.0f;
        __ml__FRC7WVectorf(CameraScratchAddress(frame, kSetupEventNormalOffset),
                           CameraScratchAddress(frame, kSetupDirectionWork), 20.0f);
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupEventNormalBase),
                                    PreData__9CGolfBallFi(ball, lastSampleIndex),
                                    CameraScratchAddress(frame, kSetupEventNormalOffset));
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupEventNormalTransition),
                                    CameraScratchAddress(frame, kSetupEventNormalBase),
                                    CameraScratchAddress(frame, kSetupEventNormalRise));
        __as__7WVectorFRC7WVector(self->motionStartPosition, CameraScratchAddress(frame, kSetupEventNormalTransition));
    }
}

// Place a raised camera beside the trajectory midpoint.
static inline void ConfigureTrajectorySideMidpointCamera(CCameraWork* self, char* frame, char* ball,
                                                         int lastSampleIndex, int clampedFirstEventIndex) {
    self->motionStartFrame = -1.0f;
    float sideSign = (self->randomValue % 2) ? 1.0f : -1.0f;
    CameraVectorAt(frame, kSetupSideMidpointSignedUp).x = 0.0f;
    CameraVectorAt(frame, kSetupSideMidpointSignedUp).y = sideSign;
    CameraVectorAt(frame, kSetupSideMidpointSignedUp).z = 0.0f;
    Vector_CrossProduct(CameraScratchAddress(frame, kSetupSideMidpointCross),
                        CameraScratchAddress(frame, kSetupSideMidpointSignedUp),
                        ((CameraTrajectorySample*)PreData__9CGolfBallFi(ball, 0))->velocity);
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              CameraScratchAddress(frame, kSetupSideMidpointCross));
    CameraVectorAt(frame, kSetupDirectionWork).y = 0.0f;
    __ml__FRC7WVectorf(CameraScratchAddress(frame, kSetupSideMidpointOffset),
                       Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupDirectionWork)), 30.0f);
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              CameraScratchAddress(frame, kSetupSideMidpointOffset));
    char* launchSample = PreData__9CGolfBallFi(ball, 0);
    char* lastSample = PreData__9CGolfBallFi(ball, lastSampleIndex);
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupLaunchToLastSample), lastSample, launchSample);
    __ml__FRC7WVectorf(CameraScratchAddress(frame, kSetupHalfLaunchToLastSample),
                       CameraScratchAddress(frame, kSetupLaunchToLastSample), 0.5f);
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupTrajectoryMidpoint), PreData__9CGolfBallFi(ball, 0),
                                CameraScratchAddress(frame, kSetupHalfLaunchToLastSample));
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupSideMidpointPosition),
                                CameraScratchAddress(frame, kSetupTrajectoryMidpoint),
                                CameraScratchAddress(frame, kSetupDirectionWork));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupSideMidpointPosition));
    self->position[1] = 2.0f + ((float*)PreData__9CGolfBallFi(ball, clampedFirstEventIndex))[1];
}

// Blend two world-space head-bone positions and frame the player at golf heading.
static inline void ConfigurePlayerHeadCamera(CCameraWork* self, char* frame, const CameraWorkStrings& resources,
                                             void*& playerObject, void*& sceneManager) {
    sceneManager = g_sceneManager;
    playerObject = NamedEffectList_FindByName(
        sceneManager, String_FormatTemporary(resources.PLAYER_d, AsCGolfDoc(G90)->currentPlayer)); // original +0x3c1
    float headNubWeight = 0.3f;
    char* headNubBone = WPuppet_FindOrCacheBone(playerObject, resources.Bip01_HeadNub);
    char* headBone = WPuppet_FindOrCacheBone(playerObject, resources.Bip01_Head);
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupWeightedHeadNub), headNubBone + 0x9c, headNubWeight);
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupWeightedHead), headBone + 0x9c, 1.0f - headNubWeight);
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupHeadTarget),
                                CameraScratchAddress(frame, kSetupWeightedHead),
                                CameraScratchAddress(frame, kSetupWeightedHeadNub));
    __as__7WVectorFRC7WVector(self->target, CameraScratchAddress(frame, kSetupHeadTarget));
    Matrix_SetAxisRotation(CameraScratchAddress(frame, kSetupHeadHeadingMatrix),
                           0.34906584f + AsCGolfDoc(G90)->headingRadians,
                           1); // original +kSetupReverseHoleRotatedOffset
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupHeadCameraOffset),
                       CameraScratchAddress(frame, kSetupHeadForwardBasis), 3.0f);
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupHeadCameraPosition), self->target,
                                CameraScratchAddress(frame, kSetupHeadCameraOffset));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupHeadCameraPosition));
    self->position[1] += 0.5f;
    self->motionStartFrame = -1.0f;
}

// Randomize a wide raised launch view around golf heading.
static inline void ConfigureLaunchWideCamera(CCameraWork* self, char* frame, char* ball) {
    CameraVectorAt(frame, kSetupWideTargetRise).x = 0.0f;
    CameraVectorAt(frame, kSetupWideTargetRise).y = 3.0f;
    CameraVectorAt(frame, kSetupWideTargetRise).z = 0.0f;
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupWideTarget), PreData__9CGolfBallFi(GolfBall__Fv(), 0),
                                CameraScratchAddress(frame, kSetupWideTargetRise));
    __as__7WVectorFRC7WVector(self->target, CameraScratchAddress(frame, kSetupWideTarget));
    CameraVectorAt(frame, kSetupWideCameraRise).x = 0.0f;
    CameraVectorAt(frame, kSetupWideCameraRise).y = 10.0f;
    CameraVectorAt(frame, kSetupWideCameraRise).z = 0.0f;
    char* golfDocument = G90;
    float headingOffset = Random_RangeFloat(self->randomValue, -1.5707964f, 1.5707964f);
    Matrix_SetAxisRotation(CameraScratchAddress(frame, kSetupWideHeadingMatrix),
                           AsCGolfDoc(golfDocument)->headingRadians + headingOffset, 1);
    __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupWideCameraOffset),
                       CameraScratchAddress(frame, kSetupWideForwardBasis), 30.0f);
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupWideCameraBase), PreData__9CGolfBallFi(ball, 0),
                                CameraScratchAddress(frame, kSetupWideCameraOffset));
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupWideCameraPosition),
                                CameraScratchAddress(frame, kSetupWideCameraBase),
                                CameraScratchAddress(frame, kSetupWideCameraRise));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupWideCameraPosition));
}

// Gate the special launch pose by power-shot variant and animation group.
static inline void ConfigurePowerLaunchCamera(CCameraWork* self, char* frame, char* ball) {
    if (self->powerShotVariant || self->animationGroup) {
        self->motionStartFrame = -1.0f;
    } else {
        self->motionStartFrame = 15.0f;
        Matrix_SetAxisRotation(CameraScratchAddress(frame, kSetupLaunchPoseHeading), AsCGolfDoc(G90)->headingRadians,
                               1);
        CameraVectorAt(frame, kSetupLaunchCameraLocalOffset).x = 10.0f;
        CameraVectorAt(frame, kSetupLaunchCameraLocalOffset).y = 0.0f;
        CameraVectorAt(frame, kSetupLaunchCameraLocalOffset).z = -20.0f;
        Matrix_TransformDirection(CameraScratchAddress(frame, kSetupLaunchCameraWorldOffset),
                                  CameraScratchAddress(frame, kSetupLaunchCameraLocalOffset),
                                  CameraScratchAddress(frame, kSetupLaunchPoseHeading));
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupLaunchCameraPosition),
                                    PreData__9CGolfBallFi(ball, 0),
                                    CameraScratchAddress(frame, kSetupLaunchCameraWorldOffset));
        __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupLaunchCameraPosition));
        CameraVectorAt(frame, kSetupLaunchTargetLocalOffset).x = 0.0f;
        CameraVectorAt(frame, kSetupLaunchTargetLocalOffset).y = 2.0f;
        CameraVectorAt(frame, kSetupLaunchTargetLocalOffset).z = -2.0f;
        Matrix_TransformDirection(CameraScratchAddress(frame, kSetupLaunchTargetWorldOffset),
                                  CameraScratchAddress(frame, kSetupLaunchTargetLocalOffset),
                                  CameraScratchAddress(frame, kSetupLaunchPoseHeading));
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupLaunchTargetPosition),
                                    PreData__9CGolfBallFi(ball, 0),
                                    CameraScratchAddress(frame, kSetupLaunchTargetWorldOffset));
        __as__7WVectorFRC7WVector(self->target, CameraScratchAddress(frame, kSetupLaunchTargetPosition));
    }
}

// Choose the golf pose or rolled trajectory sample for the power-shot frame.
static inline void ConfigurePowerTrajectoryCamera(CCameraWork* self, char* frame) {
    if (self->powerShotVariant || self->animationGroup) {
        self->motionStartFrame = -1.0f;
    } else {
        self->motionStartFrame = 15.0f;
        self->frameIndex = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0];
        if (self->frameIndex < ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[10]) {
            __as__7WVectorFRC7WVector(self->position, G90 + 0x348);
            __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupShotFrameTarget), self->position,
                                        G90 + 0x33c);
            __as__7WVectorFRC7WVector(self->target, CameraScratchAddress(frame, kSetupShotFrameTarget));
        } else {
            self->roll = 0.2617994f - 0.0029088822f * (float)(self->frameIndex -
                                                              ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[10]);
            int blendSampleIndex = (self->frameIndex * 4 + ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[10]) / 5;
            __as__7WVectorFRC7WVector(self->position, PreData__9CGolfBallFi(GolfBall__Fv(), blendSampleIndex));
            self->position[1] += 1.0f;
            __as__7WVectorFRC7WVector(self->target, GolfBall__Fv());
        }
    }
}

// Place a randomized elevated cup-side view looking toward the launch.
static inline void ConfigureElevatedHoleSideCamera(CCameraWork* self, char* frame, char* ball) {
    CopyCurrentHoleToCameraScratch(frame, kSetupElevatedHoleCup);
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupElevatedHoleCupToLaunch),
                                PreData__9CGolfBallFi(ball, 0), CameraScratchAddress(frame, kSetupElevatedHoleCup));
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupElevatedHoleCupToLaunch)));
    float sideSign = (self->randomValue % 2) ? 1.0f : -1.0f;
    CameraVectorAt(frame, kSetupElevatedHoleSignedUp).x = 0.0f;
    CameraVectorAt(frame, kSetupElevatedHoleSignedUp).y = sideSign;
    CameraVectorAt(frame, kSetupElevatedHoleSignedUp).z = 0.0f;
    CameraVectorAt(frame, kSetupElevatedHoleRise).x = 0.0f;
    CameraVectorAt(frame, kSetupElevatedHoleRise).y = 25.0f;
    CameraVectorAt(frame, kSetupElevatedHoleRise).z = 0.0f;
    CopyCurrentHoleToCameraScratch(frame, kSetupElevatedHoleOrigin);
    Vector_CrossProduct(CameraScratchAddress(frame, kSetupElevatedHoleSideCross),
                        CameraScratchAddress(frame, kSetupElevatedHoleSignedUp),
                        CameraScratchAddress(frame, kSetupDirectionWork));
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupElevatedHoleCombinedDirection),
                                CameraScratchAddress(frame, kSetupDirectionWork),
                                Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupElevatedHoleSideCross)));
    __ml__FRC7WVectorf(CameraScratchAddress(frame, kSetupElevatedHoleOffset),
                       CameraScratchAddress(frame, kSetupElevatedHoleCombinedDirection), 3.0f);
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupElevatedHoleRaisedOrigin),
                                CameraScratchAddress(frame, kSetupElevatedHoleOrigin),
                                CameraScratchAddress(frame, kSetupElevatedHoleRise));
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupElevatedHolePosition),
                                CameraScratchAddress(frame, kSetupElevatedHoleRaisedOrigin),
                                CameraScratchAddress(frame, kSetupElevatedHoleOffset));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupElevatedHolePosition));
    __as__7WVectorFRC7WVector(self->target, PreData__9CGolfBallFi(ball, 0));
}

// Build the midpoint view and notify the Player actor of its world pose.
static inline void ConfigureTrajectoryMidpointCamera(CCameraWork* self, char* frame, char* ball) {
    *(CameraCallbackDescriptor*)CameraScratchAddress(frame, kSetupMidpointLaunch) =
        *(CameraCallbackDescriptor*)PreData__9CGolfBallFi(ball, 0);
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupMidpointLastToLaunch),
                                CameraScratchAddress(frame, kSetupMidpointLaunch),
                                PreData__9CGolfBallFi(ball, ((CGolfBall*)ball)->m_preDataCount));
    __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupDirectionWork),
                              Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupMidpointLastToLaunch)));
    __as__7WVectorFRC7WVector(self->target, CameraScratchAddress(frame, kSetupMidpointLaunch));
    *(CameraCallbackDescriptor*)CameraScratchAddress(frame, kSetupMidpointActorPosition) =
        *(CameraCallbackDescriptor*)CameraScratchAddress(frame, kSetupMidpointLaunch);
    Sink* sink =
        TaskActorTable_FindByName(g_taskManager ? Object_GetChild(g_taskManager) : 0, cameraWorkStrings.Player);
    if (sink) {
        int* actorMessage = (int*)CameraScratchAddress(frame, kSetupMidpointActorMessage);
        int playerSlot = AsCGolfDoc(G90)->currentPlayer;
        actorMessage[0] = 0;
        actorMessage[1] = 30;
        actorMessage[2] = playerSlot;
        actorMessage[3] = (int)CameraScratchAddress(frame, kSetupMidpointActorMatrix);
        actorMessage[4] = 0;
        actorMessage[5] = 0;
        actorMessage[6] = 0;
        actorMessage[7] = 0;
        if (sink)
            sink->send(actorMessage);
        __as__7WVectorFRC7WVector(CameraScratchAddress(frame, kSetupMidpointActorPosition),
                                  CameraScratchAddress(frame, kSetupMidpointActorMatrixPosition));
        CameraVectorAt(frame, kSetupMidpointActorPosition).y = 0.0f;
    }
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupMidpointActorToLaunch),
                                CameraScratchAddress(frame, kSetupMidpointLaunch),
                                CameraScratchAddress(frame, kSetupMidpointActorPosition));
    *(CameraCallbackDescriptor*)CameraScratchAddress(frame, kSetupMidpointActorDirection) =
        *(CameraCallbackDescriptor*)Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupMidpointActorToLaunch));
    CameraVectorAt(frame, kSetupMidpointUp).x = 0.0f;
    CameraVectorAt(frame, kSetupMidpointUp).y = 1.0f;
    CameraVectorAt(frame, kSetupMidpointUp).z = 0.0f;
    Vector_CrossProduct(CameraScratchAddress(frame, kSetupMidpointSideCross),
                        CameraScratchAddress(frame, kSetupMidpointUp),
                        CameraScratchAddress(frame, kSetupDirectionWork));
    __ml__FRC7WVectorf(CameraScratchAddress(frame, kSetupMidpointSideOffset),
                       Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupMidpointSideCross)), 3.0f);
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupMidpointPosition),
                                PreData__9CGolfBallFi(ball, ((CGolfBall*)ball)->m_preDataCount / 2),
                                CameraScratchAddress(frame, kSetupMidpointSideOffset));
    __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupMidpointPosition));
    __mi__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupMidpointCameraToTarget), self->target,
                                self->position);
    *(CameraCallbackDescriptor*)CameraScratchAddress(frame, kSetupMidpointLookDirection) =
        *(CameraCallbackDescriptor*)Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupMidpointCameraToTarget));
    if (__ml__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupMidpointActorDirection),
                                    CameraScratchAddress(frame, kSetupMidpointLookDirection)) >= 0.0f) {
        CameraVectorAt(frame, kSetupMidpointReverseUp).x = 0.0f;
        CameraVectorAt(frame, kSetupMidpointReverseUp).y = -1.0f;
        CameraVectorAt(frame, kSetupMidpointReverseUp).z = 0.0f;
        Vector_CrossProduct(CameraScratchAddress(frame, kSetupMidpointReverseCross),
                            CameraScratchAddress(frame, kSetupMidpointReverseUp),
                            CameraScratchAddress(frame, kSetupDirectionWork));
        __ml__FRC7WVectorf(CameraScratchAddress(frame, kSetupMidpointReverseOffset),
                           Normalize__7WVectorFv(CameraScratchAddress(frame, kSetupMidpointReverseCross)), 3.0f);
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupMidpointReversePosition),
                                    PreData__9CGolfBallFi(ball, ((CGolfBall*)ball)->m_preDataCount / 2),
                                    CameraScratchAddress(frame, kSetupMidpointReverseOffset));
        __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupMidpointReversePosition));
    }
}

// Initialize the selected power-shot variant quaternion and position endpoints.
static inline void ConfigurePowerShotBlendCamera(CCameraWork* self, char* frame) {
    self->motionStartFrame = -1.0f;
    char* golfDocument = G90;
    switch (self->powerShotVariant) {
    case kPowerShotVariant1: // Configure the first existing variant.
        CameraVectorAt(frame, kSetupPowerBlend1StartLocalPosition).x = -5.92f;
        CameraVectorAt(frame, kSetupPowerBlend1StartLocalPosition).y = -4.61f;
        CameraVectorAt(frame, kSetupPowerBlend1StartLocalPosition).z = 37.5f;
        Matrix_TransformPoint(CameraScratchAddress(frame, kSetupPowerBlend1StartWorldPosition),
                              CameraScratchAddress(frame, kSetupPowerBlend1StartLocalPosition), golfDocument + 0x324);
        __as__7WVectorFRC7WVector(self->startPosition,
                                  CameraScratchAddress(frame, kSetupPowerBlend1StartWorldPosition));
        CameraVectorAt(frame, kSetupPowerBlend1StartLookOffset).x = 0.21f;
        CameraVectorAt(frame, kSetupPowerBlend1StartLookOffset).y = 0.3f;
        CameraVectorAt(frame, kSetupPowerBlend1StartLookOffset).z = -0.93f;
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupPowerBlend1StartTarget), self->startPosition,
                                    CameraScratchAddress(frame, kSetupPowerBlend1StartLookOffset));
        Matrix_BuildCameraTransform(CameraScratchAddress(frame, kSetupPowerBlend1StartPose), self->startPosition,
                                    CameraScratchAddress(frame, kSetupPowerBlend1StartTarget));
        Matrix3x3_ToQuat(self->startQuaternion, CameraScratchAddress(frame, kSetupPowerBlend1StartPose));
        CameraVectorAt(frame, kSetupPowerBlend1EndLocalPosition).x = 0.99f;
        CameraVectorAt(frame, kSetupPowerBlend1EndLocalPosition).y = -5.4f;
        CameraVectorAt(frame, kSetupPowerBlend1EndLocalPosition).z = 37.78f;
        Matrix_TransformPoint(CameraScratchAddress(frame, kSetupPowerBlend1EndWorldPosition),
                              CameraScratchAddress(frame, kSetupPowerBlend1EndLocalPosition), golfDocument + 0x324);
        __as__7WVectorFRC7WVector(self->endPosition, CameraScratchAddress(frame, kSetupPowerBlend1EndWorldPosition));
        CameraVectorAt(frame, kSetupPowerBlend1EndLookOffset).x = -0.3f;
        CameraVectorAt(frame, kSetupPowerBlend1EndLookOffset).y = 0.41f;
        CameraVectorAt(frame, kSetupPowerBlend1EndLookOffset).z = -0.86f;
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupPowerBlend1EndTarget), self->endPosition,
                                    CameraScratchAddress(frame, kSetupPowerBlend1EndLookOffset));
        Matrix_BuildCameraTransform(CameraScratchAddress(frame, kSetupPowerBlend1EndPose), self->endPosition,
                                    CameraScratchAddress(frame, kSetupPowerBlend1EndTarget));
        Matrix3x3_ToQuat(self->endQuaternion, CameraScratchAddress(frame, kSetupPowerBlend1EndPose));
        break;
    case kPowerShotVariant2: // Configure the second existing variant.
        CameraVectorAt(frame, kSetupPowerBlend2StartLocalPosition).x = -10.28f;
        CameraVectorAt(frame, kSetupPowerBlend2StartLocalPosition).y = -0.51f;
        CameraVectorAt(frame, kSetupPowerBlend2StartLocalPosition).z = 17.76f;
        Matrix_TransformPoint(CameraScratchAddress(frame, kSetupPowerBlend2StartWorldPosition),
                              CameraScratchAddress(frame, kSetupPowerBlend2StartLocalPosition), golfDocument + 0x324);
        __as__7WVectorFRC7WVector(self->startPosition,
                                  CameraScratchAddress(frame, kSetupPowerBlend2StartWorldPosition));
        CameraVectorAt(frame, kSetupPowerBlend2StartLookOffset).x = 0.93f;
        CameraVectorAt(frame, kSetupPowerBlend2StartLookOffset).y = 0.09f;
        CameraVectorAt(frame, kSetupPowerBlend2StartLookOffset).z = 0.35f;
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupPowerBlend2StartTarget), self->startPosition,
                                    CameraScratchAddress(frame, kSetupPowerBlend2StartLookOffset));
        Matrix_BuildCameraTransform(CameraScratchAddress(frame, kSetupPowerBlend2StartPose), self->startPosition,
                                    CameraScratchAddress(frame, kSetupPowerBlend2StartTarget));
        Matrix3x3_ToQuat(self->startQuaternion, CameraScratchAddress(frame, kSetupPowerBlend2StartPose));
        CameraVectorAt(frame, kSetupPowerBlend2EndLocalPosition).x = -6.45f;
        CameraVectorAt(frame, kSetupPowerBlend2EndLocalPosition).y = 0.62f;
        CameraVectorAt(frame, kSetupPowerBlend2EndLocalPosition).z = 13.05f;
        Matrix_TransformPoint(CameraScratchAddress(frame, kSetupPowerBlend2EndWorldPosition),
                              CameraScratchAddress(frame, kSetupPowerBlend2EndLocalPosition), golfDocument + 0x324);
        __as__7WVectorFRC7WVector(self->endPosition, CameraScratchAddress(frame, kSetupPowerBlend2EndWorldPosition));
        CameraVectorAt(frame, kSetupPowerBlend2EndLookOffset).x = 0.1f;
        CameraVectorAt(frame, kSetupPowerBlend2EndLookOffset).y = -0.07f;
        CameraVectorAt(frame, kSetupPowerBlend2EndLookOffset).z = 0.99f;
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupPowerBlend2EndTarget), self->endPosition,
                                    CameraScratchAddress(frame, kSetupPowerBlend2EndLookOffset));
        Matrix_BuildCameraTransform(CameraScratchAddress(frame, kSetupPowerBlend2EndPose), self->endPosition,
                                    CameraScratchAddress(frame, kSetupPowerBlend2EndTarget));
        Matrix3x3_ToQuat(self->endQuaternion, CameraScratchAddress(frame, kSetupPowerBlend2EndPose));
        break;
    case kPowerShotNoVariant:
        break; // Retain endpoints when no variant is selected.
    }
    self->duration = 0.5f;
}

// Construct the selected static power-shot pose and clear its motion callback.
static inline void ConfigureStaticPowerShotCamera(CCameraWork* self, char* frame) {
    self->motionStartFrame = -1.0f;
    switch (self->powerShotVariant) {
    case kPowerShotVariant1: { // Configure the first existing variant.
        char* golfDocument = G90;
        Matrix_SetAxisRotation(CameraScratchAddress(frame, kSetupPowerStatic1Rotation), 2.0943952f, 1);
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupPowerStatic1BasisSum), golfDocument + 0x330,
                                    golfDocument + 0x33c);
        __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupPowerStatic1Offset),
                           CameraScratchAddress(frame, kSetupPowerStatic1BasisSum), 32.0f);
        Matrix_TransformPoint(CameraScratchAddress(frame, kSetupPowerStatic1RotatedOffset),
                              CameraScratchAddress(frame, kSetupPowerStatic1Offset),
                              CameraScratchAddress(frame, kSetupPowerStatic1Rotation));
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupPowerStatic1Position), GolfBall__Fv(),
                                    CameraScratchAddress(frame, kSetupPowerStatic1RotatedOffset));
        __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupPowerStatic1Position));
        __as__7WVectorFRC7WVector(self->target, GolfBall__Fv());
        break;
    }
    case kPowerShotVariant2: { // Configure the second existing variant.
        char* golfDocument = G90;
        Matrix_SetAxisRotation(CameraScratchAddress(frame, kSetupPowerStatic2Rotation), -1.9198622f, 1);
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupPowerStatic2BasisSum), golfDocument + 0x330,
                                    golfDocument + 0x33c);
        __ml__FfRC7WVector(CameraScratchAddress(frame, kSetupPowerStatic2Offset),
                           CameraScratchAddress(frame, kSetupPowerStatic2BasisSum), 32.0f);
        Matrix_TransformPoint(CameraScratchAddress(frame, kSetupPowerStatic2RotatedOffset),
                              CameraScratchAddress(frame, kSetupPowerStatic2Offset),
                              CameraScratchAddress(frame, kSetupPowerStatic2Rotation));
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupPowerStatic2Position), GolfBall__Fv(),
                                    CameraScratchAddress(frame, kSetupPowerStatic2RotatedOffset));
        __as__7WVectorFRC7WVector(self->position, CameraScratchAddress(frame, kSetupPowerStatic2Position));
        CameraVectorAt(frame, kSetupPowerStatic2TargetRise).x = 0.0f;
        CameraVectorAt(frame, kSetupPowerStatic2TargetRise).y = 2.0f;
        CameraVectorAt(frame, kSetupPowerStatic2TargetRise).z = 0.0f;
        __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupPowerStatic2Target), GolfBall__Fv(),
                                    CameraScratchAddress(frame, kSetupPowerStatic2TargetRise));
        __as__7WVectorFRC7WVector(self->target, CameraScratchAddress(frame, kSetupPowerStatic2Target));
        break;
    }
    }
    *(CameraCallbackDescriptor*)&self->motionCallbackThisAdjustment =
        *(const CameraCallbackDescriptor*)zeroInitializationWords;
}

// Save the current world pose including roll, then restart trajectory playback.
static inline void CaptureCameraTransitionPose(CCameraWork* self, char* frame, float& capturedRoll) {
    __as__7WVectorFRC7WVector(self->position, &self->transform[9]);
    __pl__FRC7WVectorRC7WVector(CameraScratchAddress(frame, kSetupCapturedCurrentTarget), &self->transform[9],
                                &self->transform[6]);
    __as__7WVectorFRC7WVector(self->target, CameraScratchAddress(frame, kSetupCapturedCurrentTarget));
    capturedRoll = self->roll;
    void* capturedPose;
    if (CameraIsNearZero(capturedRoll)) {
        Matrix_BuildCameraTransform(CameraScratchAddress(frame, kSetupCapturedPoseMatrix), self->position,
                                    self->target);
        capturedPose = CameraScratchAddress(frame, kSetupCapturedPoseMatrix);
    } else {
        Matrix_BuildCameraTransform(CameraScratchAddress(frame, kSetupUnrolledPoseMatrix), self->position,
                                    self->target);
        Matrix_SetAxisRotation(CameraScratchAddress(frame, kSetupRollRotationMatrix), capturedRoll, 2);
        Matrix_ConcatAffine(CameraScratchAddress(frame, kSetupCapturedPoseMatrix),
                            CameraScratchAddress(frame, kSetupRollRotationMatrix),
                            CameraScratchAddress(frame, kSetupUnrolledPoseMatrix));
        capturedPose = CameraScratchAddress(frame, kSetupCapturedPoseMatrix);
    }
    Matrix_CopyAffine(self->previousTransform, capturedPose);
    self->ResetTrajectoryPlayback();
}

// Select a shot-camera action, record/replay its history, and initialize its pose and callback.
// requestedAction is a CameraAction code; captureTransition saves the current pose and restarts playback.
// installed suppresses the entire operation. Valid historyIndex replays; otherwise append.
// Replay the next saved action or append to owned history with transactional growth.
#define SELECT_AND_RECORD_CAMERA_ACTION()                                                                   \
    /* Phase 1: replay the next saved action, or append the requested action to owned history. */           \
    this->action = requestedAction;                                                                         \
    int historyCount = (int)this->actionCount;                                                              \
    if (replayIndex >= 0 && replayIndex < historyCount) {                                                   \
        this->historyIndex = replayIndex + 1;                                                               \
        this->action = this->actionHistory[replayIndex];                                                    \
    } else {                                                                                                \
        unsigned storedActionCount = *(volatile unsigned*)&this->actionCount;                               \
        this->historyIndex = -1;                                                                            \
        if (storedActionCount < this->actionCapacity) {                                                     \
            unsigned newActionCount = storedActionCount + 1;                                                \
            this->actionCount = newActionCount;                                                             \
            this->actionHistory[newActionCount - 1] = this->action;                                         \
        } else {                                                                                            \
            if (0x3FFFFFFFu - this->actionCapacity < 1) {                                                   \
                union {                                                                                     \
                    int alignment;                                                                          \
                    unsigned char bytes[sizeof(std::length_error)];                                         \
                } storage;                                                                                  \
                std::length_error* error = (std::length_error*)storage.bytes;                               \
                error->__ct();                                                                              \
                exception_runtime::__throw(exception_runtime::cameraWorkStrings.lengthErrorTypeName, error, \
                                           (void*)exception_runtime::__dt__Q23std12length_errorFv);         \
            }                                                                                               \
            unsigned additionalCapacity = this->actionCount + 1 - this->actionCapacity;                     \
            LayoutVectorBuffer growthBuffer(&this->actionCapacity);                                         \
            unsigned capacity = this->actionCapacity;                                                       \
            if (additionalCapacity > 0x3FFFFFFFu - capacity) {                                              \
                union {                                                                                     \
                    int alignment;                                                                          \
                    unsigned char bytes[sizeof(std::length_error)];                                         \
                } storage;                                                                                  \
                std::length_error* error = (std::length_error*)storage.bytes;                               \
                error->__ct();                                                                              \
                exception_runtime::__throw(exception_runtime::cameraWorkStrings.lengthErrorTypeName, error, \
                                           (void*)exception_runtime::__dt__Q23std12length_errorFv);         \
            }                                                                                               \
            if (capacity < 0x15555555u) {                                                                   \
                capacity += LayoutVectorMax((3 * (capacity + 1)) / 5, additionalCapacity);                  \
            } else if (capacity < 0x2AAAAAAAu) {                                                            \
                capacity += LayoutVectorMax((capacity + 1) / 2, additionalCapacity);                        \
            } else                                                                                          \
                capacity = 0x3FFFFFFF;                                                                      \
            if (capacity > 0x3FFFFFFFu) {                                                                   \
                union {                                                                                     \
                    int alignment;                                                                          \
                    unsigned char bytes[sizeof(std::length_error)];                                         \
                } storage;                                                                                  \
                std::length_error* error = (std::length_error*)storage.bytes;                               \
                error->__ct();                                                                              \
                exception_runtime::__throw(exception_runtime::cameraWorkStrings.lengthErrorTypeName, error, \
                                           (void*)exception_runtime::__dt__Q23std12length_errorFv);         \
            }                                                                                               \
            int* newActions = (int*)appMallocDummy__FUl(capacity * 4);                                      \
            unsigned oldSize = this->actionCount;                                                           \
            growthBuffer.offset = oldSize;                                                                  \
            (newActions + oldSize)[growthBuffer.size] = this->action;                                       \
            int* oldActions = this->actionHistory;                                                          \
            int copiedActionCount = (this->actionHistory + this->actionCount) - oldActions;                 \
            growthBuffer.data = newActions;                                                                 \
            growthBuffer.capacity = capacity;                                                               \
            growthBuffer.size = growthBuffer.size + 1;                                                      \
            int copyStart = oldSize - copiedActionCount;                                                    \
            growthBuffer.offset = copyStart;                                                                \
            memcpy(newActions + copyStart, oldActions, copiedActionCount * 4);                              \
            memset(oldActions, 0, copiedActionCount * 4);                                                   \
            unsigned newSize = growthBuffer.size + copiedActionCount;                                       \
            oldActions = this->actionHistory;                                                               \
            newActions = growthBuffer.data;                                                                 \
            growthBuffer.data = oldActions;                                                                 \
            this->actionCount = 0;                                                                          \
            this->actionCapacity = growthBuffer.capacity;                                                   \
            growthBuffer.size = 0;                                                                          \
            this->actionHistory = newActions;                                                               \
            this->actionCount = newSize;                                                                    \
            growthBuffer.capacity = capacity;                                                               \
        }                                                                                                   \
    }

} // extern "C"

} // namespace ns0

// Install a requested/replayed action and initialize its geometry/transition.
void CCameraWork::ConfigureMode(int requestedAction, int captureTransition) {
    // The setup helpers and scratch layout are local to this file.
    using namespace ns0;
    char frame[kCameraSetupBufferBytes];
    CameraCallbackDescriptor* callbackDescriptors;
    int lastSampleIndex;
    char* ball;
    int clampedFirstEventIndex;
    int clampedSecondEventIndex;
    callbackDescriptors = CameraCallbackDescriptors();
    if (installed)
        return;
    ball = GolfBall__Fv();
    int firstEventIndex = ((CGolfBall*)ball)->m_eventPreDataIndex[2];
    int secondEventIndex = ((CGolfBall*)ball)->m_eventPreDataIndex[1];
    int replayIndex = historyIndex;
    lastSampleIndex = ((CGolfBall*)ball)->m_preDataCount;
    clampedFirstEventIndex = firstEventIndex > 0 ? firstEventIndex : 0;
    clampedSecondEventIndex = secondEventIndex > 0 ? secondEventIndex : 0;
    SELECT_AND_RECORD_CAMERA_ACTION();

    // Phase 2: reset manual offsets unless random mode retains them, and capture transition pose.
    if (!randomModeRequested) {
        inputMoved = 0;
        inputFovOffset = 0.0f;
        Vec2_Zero(inputRotationOffset);
    }
    if (captureTransition) {
        float capturedRoll;
        CaptureCameraTransitionPose(this, frame, capturedRoll);
    }
    if (action < 100 && captureTransition)
        motionStartFrame = 45.0f;
    else
        motionStartFrame = -1.0f;
    lastPreMotionFrame = -2;
    roll = 0.0f;
    if (action < 100 && captureTransition && ((CGolfBall*)GolfBall__Fv())->m_preDataCount < 30) {
        *(CameraCallbackDescriptor*)&motionCallbackThisAdjustment =
            *(const CameraCallbackDescriptor*)zeroInitializationWords;
    } else {
        // Phase 3: install the action descriptor and any action-specific geometry.
        // Low actions build trajectory views; 100-range actions frame the hole; 200-range
        // actions initialize timed/orbit transitions. Unsupported actions use the default callback.
        // Per-action input/output policy (positions/targets are world XYZ; matrix rotations are radians):
        // 0..2,5..8,10,23,26,100,105: primarily install a callback; callback owns subsequent motion.
        // 3: first-event position/velocity + random side/height -> transitionPosition, actionFlags.
        // 4/11: second-event->last displacement (launch->last fallback) + rise/offset -> transitionPosition.
        // 9/12/18/27: walk helper uses launch/event velocity -> position/target (18 also sets heading).
        // 13/14: trajectory midpoint/cutoff-neighbor bisector -> raised world position.
        // 20: weighted player head bones -> target; rotated forward offset -> position.
        // 21/22: launch + rotated heading basis + vertical rise -> position/target.
        // 24..26: special-shot animation/variant gates control trajectory start and pose capture.
        // 101..104: launch/cup/trajectory/actor inputs -> elevated side/rear/midpoint positions.
        // 106..112/150: named cup/launch/velocity offsets -> world position/target; 112 narrows FOV.
        // 203..208: initialize orbit parameters, captured transforms or quaternion endpoints.
        // Every install exits explicitly; no action falls through to a neighboring setup.
        switch (action) {
        // Install this action's update callback; no geometry is built here.
        case kCameraAction0: {
            InstallCameraCallback(this, callbackDescriptors, kCallbackAction0);
            break;
        }
        // Install this action's update callback; no geometry is built here.
        case kCameraAction1:
            motionStartFrame = -1.0f;
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackAction1);
                break;
            }
        // Install this action's update callback; no geometry is built here.
        case kCameraAction2: {
            InstallCameraCallback(this, callbackDescriptors, kCallbackAction2);
            break;
        }
        // Set up event side camera and install its callback.
        case kCameraEventSideView: {
            ConfigureEventSideCamera(this, frame, ball, clampedFirstEventIndex);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackEventSideView);
                break;
            }
        }
        // Share second-event geometry, then choose the action-specific descriptor.
        case kCameraSecondEventView:
        // Set up second event camera and install its callback.
        case kCameraSecondEventAlternateView: {
            ConfigureSecondEventCamera(this, frame, ball, lastSampleIndex, clampedSecondEventIndex);
            if (action == kCameraSecondEventView) {
                InstallCameraCallback(this, callbackDescriptors, kCallbackSecondEventView);
                break;
            }
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackSecondEventAlternateView);
                break;
            }
        }
        // Install this action's update callback; no geometry is built here.
        case kCameraAction5: {
            InstallCameraCallback(this, callbackDescriptors, kCallbackAction5);
            break;
        }
        // Install this action's update callback; no geometry is built here.
        case kCameraAction7:
            motionStartFrame = -1.0f;
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackAction7);
                break;
            }
        // Install this action's update callback; no geometry is built here.
        case kCameraAction8: {
            InstallCameraCallback(this, callbackDescriptors, kCallbackAction8);
            break;
        }
        // Set up camera walk pose and install its callback.
        case kCameraWalkView9: {
            ConfigureCameraWalkPose(this, frame, ball, clampedFirstEventIndex, -1.0f, kSetupWalkAction9Up,
                                    kSetupWalkAction9Cross, kSetupWalkAction9LateralOffset,
                                    kSetupWalkAction9EventVelocity, kSetupWalkAction9LongitudinalOffset,
                                    kSetupWalkAction9RaisedLaunch, kSetupWalkAction9Position);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackWalkView9);
                break;
            }
        }
        // Install this action's update callback; no geometry is built here.
        case kCameraAction10: {
            InstallCameraCallback(this, callbackDescriptors, kCallbackAction10);
            break;
        }
        // Set up camera walk pose and install its callback.
        case kCameraWalkView18: {
            ConfigureCameraWalkPose(this, frame, ball, clampedFirstEventIndex, -1.0f, kSetupWalkAction18Up,
                                    kSetupWalkAction18Cross, kSetupWalkAction18LateralOffset,
                                    kSetupWalkAction18EventVelocity, kSetupWalkAction18LongitudinalOffset,
                                    kSetupWalkAction18RaisedLaunch, kSetupWalkAction18Position);
            heading = 0.2617994f;
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackWalkView18);
                break;
            }
        }
        // Install the Action19 descriptor.
        case kCameraAction19: {
            animationTime = 0.0f;
            motionStartFrame = -1.0f;
            heading = 0.0f;
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackAction19);
                break;
            }
        }
        // Set up camera walk pose and install its callback.
        case kCameraWalkView12: {
            ConfigureCameraWalkPose(this, frame, ball, clampedFirstEventIndex, -1.0f, kSetupWalkAction12Up,
                                    kSetupWalkAction12Cross, kSetupWalkAction12LateralOffset,
                                    kSetupWalkAction12EventVelocity, kSetupWalkAction12LongitudinalOffset,
                                    kSetupWalkAction12RaisedLaunch, kSetupWalkAction12Position);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackWalkView12);
                break;
            }
        }
        // Set up trajectory side midpoint camera and install its callback.
        case kCameraTrajectorySideMidpointView: {
            ConfigureTrajectorySideMidpointCamera(this, frame, ball, lastSampleIndex, clampedFirstEventIndex);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackTrajectorySideMidpointView);
                break;
            }
        }
        // Set up cutoff bisector camera and install its callback.
        case kCameraCutoffBisectorView: {
            void* nextDirection;
            ConfigureCutoffBisectorCamera(this, frame, ball, nextDirection);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackCutoffBisectorView);
                break;
            }
        }
        // Set up player head camera and install its callback.
        case kCameraPlayerHeadView: {
            const CameraWorkStrings& resources = cameraWorkStrings;
            void* playerObject;
            void* sceneManager;
            ConfigurePlayerHeadCamera(this, frame, resources, playerObject, sceneManager);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackPlayerHeadView);
                break;
            }
        }
        // Set up launch quarter camera and install its callback.
        case kCameraLaunchQuarterView: {
            ConfigureLaunchQuarterCamera(this, frame, ball);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackLaunchQuarterView);
                break;
            }
        }
        // Set up launch wide camera and install its callback.
        case kCameraLaunchRandomWideView: {
            ConfigureLaunchWideCamera(this, frame, ball);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackLaunchRandomWideView);
                break;
            }
        }
        // Install this action's update callback; no geometry is built here.
        case kCameraAction23:
            motionStartFrame = 0.0f;
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackAction23);
                break;
            }
        // Set up power launch camera and install its callback.
        case kCameraPowerLaunchView: {
            ConfigurePowerLaunchCamera(this, frame, ball);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackPowerLaunchView);
                break;
            }
        }
        // Set up power trajectory camera and install its callback.
        case kCameraPowerTrajectoryView: {
            ConfigurePowerTrajectoryCamera(this, frame);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackPowerTrajectoryView);
                break;
            }
        }
        // Install the PowerStartView descriptor.
        case kCameraPowerStartView: {
            if (powerShotVariant || animationGroup)
                motionStartFrame = -1.0f;
            else
                motionStartFrame = 15.0f;
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackPowerStartView);
                break;
            }
        }
        // Set up camera walk pose and install its callback.
        case kCameraWalkView27: {
            ConfigureCameraWalkPose(this, frame, ball, clampedFirstEventIndex, 15.0f, kSetupWalkAction27Up,
                                    kSetupWalkAction27Cross, kSetupWalkAction27LateralOffset,
                                    kSetupWalkAction27EventVelocity, kSetupWalkAction27LongitudinalOffset,
                                    kSetupWalkAction27RaisedLaunch, kSetupWalkAction27Position);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackWalkView27);
                break;
            }
        }
        // Install pitch tracking; this case builds no initial geometry.
        case kCameraTrackBallPitch: {
            InstallCameraCallback(this, callbackDescriptors, kCallbackTrackBallPitch);
            break;
        }
        // Set up raised launch side camera and install its callback.
        case kCameraRaisedLaunchSideView: {
            ConfigureRaisedLaunchSideCamera(this, frame, ball);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackRaisedLaunchSideView);
                break;
            }
        }
        // Set up raised launch rear camera and install its callback.
        case kCameraRaisedLaunchRearView: {
            ConfigureRaisedLaunchRearCamera(this, frame, ball);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackRaisedLaunchRearView);
                break;
            }
        }
        // Set up elevated hole side camera and install its callback.
        case kCameraElevatedHoleSideView: {
            ConfigureElevatedHoleSideCamera(this, frame, ball);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackElevatedHoleSideView);
                break;
            }
        }
        // Set up trajectory midpoint camera and install its callback.
        case kCameraTrajectoryMidpointView: {
            ConfigureTrajectoryMidpointCamera(this, frame, ball);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackTrajectoryMidpointView);
                break;
            }
        }
        // Install this action's update callback; no geometry is built here.
        case kCameraAction105: {
            InstallCameraCallback(this, callbackDescriptors, kCallbackAction105);
            break;
        }
        // Set up launch front camera and install its callback.
        case kCameraLaunchFrontView: {
            ConfigureLaunchFrontCamera(this, frame, ball);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackLaunchFrontView);
                groundClearanceEnabled = 0;
                break;
            }
        }
        // Set up shot and hole camera and install its callback.
        case kCameraShotAndHoleView: {
            ConfigureShotAndHoleCamera(this, frame, ball);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackShotAndHoleView);
                break;
            }
        }
        // Set up hole reverse camera and install its callback.
        case kCameraHoleReverseView: {
            ConfigureHoleReverseCamera(this, frame, ball);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackHoleReverseView);
                break;
            }
        }
        // Set up hole side camera and install its callback.
        case kCameraHoleSideView: {
            ConfigureHoleSideCamera(this, frame, ball);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackHoleSideView);
                break;
            }
        }
        // Set up hole velocity camera and install its callback.
        case kCameraHoleVelocityView: {
            ConfigureHoleVelocityCamera(this, frame, ball);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackHoleVelocityView);
                break;
            }
        }
        // Set up hole near camera and install its callback.
        case kCameraHoleNearView: {
            ConfigureHoleNearCamera(this, frame, ball);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackHoleNearView);
                break;
            }
        }
        // Set up hole telephoto camera and install its callback.
        case kCameraHoleTelephotoView: {
            ConfigureHoleTelephotoCamera(this, frame, ball);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackHoleTelephotoView);
                break;
            }
        }
        // Set up below hole camera and install its callback.
        case kCameraBelowHoleView: {
            ConfigureBelowHoleCamera(this, frame);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackBelowHoleView);
                break;
            }
        }
        // Set up narrow golf camera and install its callback.
        case kCameraNarrowGolfView: {
            ConfigureNarrowGolfCamera(this, frame);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackNarrowGolfView);
                break;
            }
        }
        // Set up ball orbit camera and install its callback.
        case kCameraOrbitToBall: {
            ConfigureBallOrbitCamera(this);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackOrbitToBall);
                groundClearanceEnabled = 0;
                break;
            }
        }
        // Set up golf pose blend camera and install its callback.
        case kCameraBlendToGolfView: {
            ConfigureGolfPoseBlendCamera(this, frame);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackBlendToGolfView);
                break;
            }
        }
        // Install this action's update callback; no geometry is built here.
        case kCameraAction200: {
            InstallCameraCallback(this, callbackDescriptors, kCallbackAction200);
            break;
        }
        // Install this action's update callback; no geometry is built here.
        case kCameraAction201: {
            InstallCameraCallback(this, callbackDescriptors, kCallbackAction201);
            break;
        }
        // Install this action's update callback; no geometry is built here.
        case kCameraAction202:
            motionStartFrame = -1.0f;
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackAction202);
                break;
            }
        // Build the variant pose and clear motion callback.
        case kCameraPowerShotStaticView: {
            ConfigureStaticPowerShotCamera(this, frame);
            break;
        }
        // Set up power shot blend camera and install its callback.
        case kCameraPowerShotBlendView: {
            ConfigurePowerShotBlendCamera(this, frame);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackPowerShotBlendView);
                break;
            }
        }
        // Set up raised hole orbit camera and install its callback.
        case kCameraRaisedHoleOrbit: {
            ConfigureRaisedHoleOrbitCamera(this, frame);
            {
                InstallCameraCallback(this, callbackDescriptors, kCallbackRaisedHoleOrbit);
                unk_23C = 0;
                break;
            }
        }
        default: {
            InstallCameraCallback(this, callbackDescriptors, kCallbackDefault);
            break;
        }
        }
    }
    return;
}
#pragma opt_propagation reset

// ---- merged from camera_work_shot_cameras.cpp ----
namespace ns1 {

extern "C" {

typedef Vec3f Vec3;
struct Data_8012140C {
    char pad[0x70];
};
struct Data_801EA168 {
    char pad[4];
    void* field;
};
struct Data_801EA168_Inner {
    char pad[0xA];
    unsigned char state;
};

void* GolfBall__Fv();
void* PreData__9CGolfBallFi(void*, int);
void* CQuadTree_GetInstance();
void* InstanceTable_Get();
unsigned char GetGroundType__9CQuadTreeF7WVector(void*, Vec3*);
void __as__7WVectorFRC7WVector(void*, void*);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void __ml__FRC7WVectorf(void*, void*, float);
void Vector_CrossProduct(void*, void*, void*);
void* Normalize__7WVectorFv(void*);
void Sound_StopByAlias(void*, const char*);
void __mi__FRC7WVectorRC7WVector(Vec3*, Vec3*, void*);
float Vector_SignedAngleXZ(Vec3*, Vec3*);
float Magnitude__7WVectorCFv(Vec3*);
int rand();
int SharedDoc_IsPlayerFlag0Set(unsigned char);
void* Object_GetChild(void*);
void* TaskActorTable_FindByName(void*, const char*);
extern CSharedDoc* g_sharedDoc;
extern void* g_taskManager;
extern const CameraWorkStrings cameraWorkStrings;
extern const float kFloat_0_7853982_CameraWork;
extern const float kFloat_16_CameraWork;
extern const float kFloat_60_CameraWork;
extern const float kFloat_30_0;
extern const float kFloat_90_CameraWork;
extern const float kFloat_0_CameraWork;
extern const float kFloat_neg1_CameraWork;
extern const float kFloat_5_CameraWork;
extern const float kFloat_0_5_CameraWork;
extern const float kFloat_1_CameraWork;
extern const float kFloat_25_CameraWork;
extern const float kFloat_50_CameraWork;
extern const float kFloat_2_CameraWork;
extern const float kFloat_15_CameraWork;
extern const float kFloat_neg15_CameraWork;
extern const float kFloat_0_8_CameraWork;
extern const float kFloat_1_2_CameraWork;
extern const float kFloat_1_5_GolfCamera;
extern const float kFloat_0_75_CameraWork;
extern const float kFloat_100_GolfCamera;
extern const float kFloat_3_CameraWork;
extern const float kFloat_2_5_CameraWork;
extern const float kFloat_40_cameraWork;
extern const float kCameraShared20;
extern const float kFloat_0_3_CameraWork;
extern const float kFloat_0_1_805DDF94;
extern const float kFloat_10_cameraWork;
extern const float kFloat_neg20_CameraWork;
float Vec3_DotProduct(void*, void*);
void __amu__7WVectorFf(void*, float);
void __apl__7WVectorFRC7WVector(void*, void*);
extern const double kSignedIntToDoubleBias_CameraWork;
extern void* g_soundManager;

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
} // extern "C"
} // namespace ns1

typedef Vec3f Vec3;
extern "C" char* GolfBall__Fv();
extern "C" void* InstanceTable_Get();
extern "C" void* CQuadTree_GetInstance();
extern "C" unsigned char GetGroundType__9CQuadTreeF7WVector(void*, const void*);
extern "C" const float kFloat_0_CameraWork;
extern "C" const float kFloat_neg1_CameraWork;
extern "C" const float kFloat_5_CameraWork;
extern "C" void Sound_StopByAlias(void*, const char*);
extern "C" void* g_soundManager;
extern "C" const CameraWorkStrings cameraWorkStrings;
extern "C" void __as__7WVectorFRC7WVector(void*, const void*);

// Reset camera timing, one-shot sound flags, and terrain handling for a new ball trajectory.
void CCameraWork::ResetTrajectoryPlayback() {
    int r29flag;
    int r28flag;
    int r27flag;
    int v0 = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[2];
    firstEventFrame = (float)v0;
    int v1 = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[1];
    secondEventFrame = (float)v1;

    void* tmp = GolfBall__Fv();
    int n = randomValue;
    int count = ((CGolfBall*)tmp)->m_preDataCount;

    r29flag = 0;
    lastPreMotionFrame = -2;
    trajectoryLastFrame = (float)count;
    (*(unsigned int*)&frameIndex) = 0;
    animationTime = kFloat_0_CameraWork;
    directionalSoundPlayed = 0;
    ballProximitySoundState = 0;
    lastShakeUpdateTime = kFloat_0_CameraWork;
    lastProximitySoundUpdateTime = kFloat_0_CameraWork;
    duration = kFloat_neg1_CameraWork;
    groundClearanceEnabled = 1;
    targetRiseHeight = kFloat_5_CameraWork;
    unk_23C = 2;

    r28flag = 0;
    r27flag = 0;
    bool eligible = n % 6 == 0;
    if (eligible) {
        if (((CQuadTree*)CQuadTree_GetInstance())->GetObjColNum() == 0)
            r29flag = eligible;
    }
    if (r29flag != 0) {
        void* e = InstanceTable_Get();
        unsigned char state = *((unsigned char*)*(void**)((unsigned char*)e + 4) + 0xA);
        if (state != 3)
            r27flag = 1;
    }
    if (r27flag != 0) {
        void* a = GolfBall__Fv();
        void* b = GolfBall__Fv();
        Vec3 vec = *(Vec3*)ns1::PreData__9CGolfBallFi(b, ((CGolfBall*)a)->m_preDataCount);
        if (GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &vec) == 2)
            r28flag = 1;
    }
    if (r28flag != 0)
        forceGroundEffect = 1;
    else
        forceGroundEffect = 0;
    unk_164 = 0;
    holeOutDetected = 0;
    (*(int*)&actionFlags) = 1;
    __as__7WVectorFRC7WVector((unsigned char*)initialBallPosition, GolfBall__Fv());
    Sound_StopByAlias(g_soundManager, cameraWorkStrings.ballWindSoundAlias);
}

namespace ns1 {
extern "C" {

#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset
struct Evt_ {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9(void*);
};
// Ordered shot-camera decision table (earlier returns take precedence):
// Gate/predicate                         Action or continuation
// selection disabled                    return
// state 3                               pitch view (player/green angle gates)
// impact grade 1, no trajectory          randomized wide launch view
// impact grade 1, trajectory             randomValue%3 -> pitch/quarter/wide
// power bits, last sample <16 from cup   chance multiplier = boosted ? 5 : 1:
//   0x10, event[11] == event[1]          5% * multiplier -> power start (26)
//   0x20                                10% * multiplier -> launch/trajectory (24/25)
//   0x40, event[14] == event[1]          5% * multiplier -> walk (27)
// missing second event                  return
// action 10                             query Caddie; may request action 0
// cut sample before second event        short interval -> bisector (14);
//                                       second-event views -> midpoint (13)
// remaining frame intervals             midpoint or event/walk alternatives below
// Power selection does not return: ConfigureMode's installed guard and the
// subsequent interval policy remain part of the contract. Percentages reuse
// the same rand() result; do not replace them with independent random draws.
} // extern "C"
} // namespace ns1

extern "C" {
extern CSharedDoc* g_sharedDoc;
int SharedDoc_IsPlayerFlag0Set(unsigned char);
void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
void __ml__FfRC7WVector(void*, const void*, float);
void __ml__FRC7WVectorf(void*, const void*, float);
void __as__7WVectorFRC7WVector(void*, const void*);
}
extern "C" float Vector_SignedAngleXZ(Vec3*, Vec3*);
extern "C" const float kFloat_0_7853982_CameraWork;
extern "C" const float kFloat_16_CameraWork;
extern "C" void* g_taskManager;
extern "C" void* Object_GetChild(void*);
extern "C" void* TaskActorTable_FindByName(void*, const char*);
struct Evt_ {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9(void*);
};
extern "C" const float kFloat_60_CameraWork;
extern "C" const float kFloat_30_0;
extern "C" const float kFloat_90_CameraWork;
// Choose the next shot-camera action from ball events, ground type, and special-shot flags.
void CCameraWork::SelectShotCameraMode(int boostSelectionChance) {
    // Phase 1: handle view/player restrictions and impact-grade-specific camera selection.
    int v1;
    Vec3 vec;
    if (!shotCameraSelectionEnabled)
        return;
    unsigned char state = *((unsigned char*)*(void**)((unsigned char*)InstanceTable_Get() + 4) + 0xA);
    if (state == kShotCameraPutterCategory) {
        if (SharedDoc_IsPlayerFlag0Set(AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer))
            return; // original +0x3c1
        void* ball = GolfBall__Fv();
        if (((CGolfBall*)ball)->m_groundType == kShotCameraGreenGround) {
            void* m = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            unsigned char* entry =
                (unsigned char*)AsCGolfDoc(m)->holeRecords + *((unsigned char*)&AsCGolfDoc(m)->currentHoleIndex) * 32;
            Vec3 a;
            Vec3 target = *(Vec3*)(entry + 0xc);
            void* ballForVelocity = GolfBall__Fv();
            void* extra = GolfBall__Fv();
            __mi__FRC7WVectorRC7WVector(&a, &target, extra);
            float angle = Vector_SignedAngleXZ(&a, (Vec3*)&((CGolfBall*)ballForVelocity)->m_velocity);
            angle = angle > kFloat_0_CameraWork ? angle : -angle;
            if (!(angle > kFloat_0_7853982_CameraWork))
                return;
        }
        ConfigureMode(kCameraTrackBallPitch, 1);
        return;
    }
    if (AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->impactGrade == kShotCameraMissImpact) {
        void* ball = GolfBall__Fv();
        if (!((CGolfBall*)ball)->m_preDataCount) {
            ConfigureMode(kCameraLaunchRandomWideView, 1);
            return;
        }
        int n = randomValue;
        switch (n % 3) {
        case 0:
            ConfigureMode(kCameraTrackBallPitch, 1);
            break;
        case 1:
            ConfigureMode(kCameraLaunchQuarterView, 1);
            break;
        case 2:
            ConfigureMode(kCameraLaunchRandomWideView, 1);
            break;
        }
        return;
    }
    void* ball = GolfBall__Fv();
    // Phase 2: near-hole power-shot candidates use a 5x probability multiplier when requested.
    int flags = ((CGolfBall*)ball)->m_powerFlags & kShotCameraPowerMask;
    if (flags) {
        void* m = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        unsigned char* entry =
            (unsigned char*)AsCGolfDoc(m)->holeRecords + *((unsigned char*)&AsCGolfDoc(m)->currentHoleIndex) * 32;
        Vec3 target = *(Vec3*)(entry + 0xc);
        void* ballForPreDataCount = GolfBall__Fv();
        void* obj = GolfBall__Fv();
        unsigned int id = ((CGolfBall*)ballForPreDataCount)->m_preDataCount;
        void* sel = ns1::PreData__9CGolfBallFi(obj, id);
        __mi__FRC7WVectorRC7WVector(&vec, &target, sel);
        vec.y = kFloat_0_CameraWork;
        int interval = boostSelectionChance ? kShotCameraBoostedChance : kShotCameraNormalChance;
        float len = ns1::Magnitude__7WVectorCFv(&vec);
        if (len < kFloat_16_CameraWork) {
            int randomValue = rand();
            switch (flags) {
            case kShotCameraPowerBit10: {
                int rem = randomValue % 100;
                if (rem < interval * 5) {
                    void* c1 = GolfBall__Fv();
                    v1 = ((CGolfBall*)c1)->m_eventPreDataIndex[kShotCameraSecondEventSlot];
                    void* c2 = GolfBall__Fv();
                    if (((CGolfBall*)c2)->m_eventPreDataIndex[kShotCameraEventSlot11] == v1)
                        ConfigureMode(kCameraPowerStartView, 1);
                }
                break;
            }
            case kShotCameraPowerBit20: {
                int rem = randomValue % 100;
                if (rem < interval * 10) {
                    if (randomValue % 10 < 5)
                        ConfigureMode(kCameraPowerLaunchView, 1);
                    else
                        ConfigureMode(kCameraPowerTrajectoryView, 1);
                }
                break;
            }
            case kShotCameraPowerBit40: {
                int rem = randomValue % 100;
                if (rem < interval * 5) {
                    void* c1 = GolfBall__Fv();
                    v1 = ((CGolfBall*)c1)->m_eventPreDataIndex[kShotCameraSecondEventSlot];
                    void* c2 = GolfBall__Fv();
                    if (((CGolfBall*)c2)->m_eventPreDataIndex[kShotCameraEventSlot14] == v1)
                        ConfigureMode(kCameraWalkView27, 1);
                }
                break;
            }
            }
        }
    }
    if (secondEventFrame < kFloat_0_CameraWork)
        return;
    if (action == kCameraAction10) {
        if (!unk_1A2) {
            ConfigureMode(kCameraAction0, 1);
            return;
        }
        void* result = g_taskManager ? Object_GetChild(g_taskManager) : 0;
        void* event = TaskActorTable_FindByName(result, cameraWorkStrings.Caddie);
        if (event) {
            void* returned;
            int evt[8];
            evt[0] = 0;
            evt[1] = kShotCameraCaddieQuery;
            evt[2] = (int)&returned;
            evt[3] = 0;
            evt[4] = 0;
            evt[5] = 0;
            evt[6] = 0;
            evt[7] = 0;
            if (event) {
                ((Evt_*)event)->v9(evt);
            }
            if (!returned || !((CameraActorResponse*)returned)->cameraAllowed)
                ConfigureMode(kCameraAction0, 1);
        }
    }
    // Phase 3: locate cut-state 2, then choose a fallback from the available frame intervals.
    for (cutoffFrame = 0; cutoffFrame < (int)trajectoryLastFrame; ++cutoffFrame) {
        int idx = cutoffFrame;
        void* cam = GolfBall__Fv();
        void* sel = ns1::PreData__9CGolfBallFi(cam, idx);
        if (sel && ((CameraTrajectorySample*)sel)->cameraCutState == kShotCameraCutSampleState)
            break;
    }
    int step = cutoffFrame;
    if (step < (int)secondEventFrame) {
        (*(int*)&actionFlags) |= kCameraCutAtEvent;
        if ((float)step - motionStartFrame < kFloat_60_CameraWork) {
            ConfigureMode(kCameraCutoffBisectorView, 1);
            return;
        }
        int mode = action;
        if (mode == kCameraSecondEventView || mode == kCameraSecondEventAlternateView)
            ConfigureMode(kCameraTrajectorySideMidpointView, 1);
        return;
    }
    if (secondEventFrame - motionStartFrame < kFloat_60_CameraWork ||
        firstEventFrame - motionStartFrame < kFloat_30_0) {
        ConfigureMode(kCameraTrajectorySideMidpointView, 1);
        return;
    }
    if (secondEventFrame - motionStartFrame < kFloat_90_CameraWork) {
        if (SharedDoc_IsPlayerFlag0Set(AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer)) {
            ConfigureMode(kCameraTrajectorySideMidpointView, 1);
            return;
        }
        unsigned int u = (*(unsigned int*)&randomValue);
        unsigned int s = u >> 31;
        unsigned int lo = u & 1u;
        unsigned int r = (lo ^ s) - s;
        switch (r) {
        case 0:
            ConfigureMode(kCameraSecondEventView, 1);
            break;
        case 1:
            ConfigureMode(kCameraTrajectorySideMidpointView, 1);
            break;
        }
    }
}

namespace ns1 {
extern "C" {

// Build four cubic Bezier control points for the selected shot-camera action.
// Writes curveControlPoints[0..11] as four world-space XYZ points; unsupported action
// values leave the points unchanged but still recompute targetRiseHeight from point 3.
#pragma push
#pragma always_inline on
// Blend the captured pose through event and side-offset controls toward the final sample.
// All vector scratch and curve controls belong to the caller; positions use world XYZ.
// One caller-owned block preserves the native vector scratch layout.
// Members follow increasing stack offsets; helper names express the trajectory phase.
struct CameraCurveScratch {
    Vec3 finalControlToFinalSampleDelta;
    Vec3 wideCurveFinish;
    Vec3 eventMidpointRise;
    Vec3 raisedEventMidpoint;
    Vec3 wideSecondControl;
    Vec3 wideFirstEventRise;
    Vec3 raisedWideFirstEvent;
    Vec3 wideFirstControl;
    Vec3 wideSideDown;
    Vec3 wideSideCross;
    Vec3 scaledWideSide;
    Vec3 scaledRandomNormalApproach;
    Vec3 randomNormalFinish;
    Vec3 extendedRandomRecoveryApproach;
    Vec3 randomRecoveryFinish;
    Vec3 shortenedRandomSide;
    Vec3 randomTrailingOffset;
    Vec3 randomRaisedTrailingEvent;
    Vec3 randomSecondControl;
    Vec3 randomSideUp;
    Vec3 randomSideCross;
    Vec3 scaledRandomSide;
    Vec3 randomFirstEventOffset;
    Vec3 randomFirstControl;
    Vec3 aheadApproachOffset;
    Vec3 aheadApproachRise;
    Vec3 raisedFinishAheadOfApproach;
    Vec3 finishAheadOfHoleApproach;
    Vec3 behindApproachOffset;
    Vec3 behindApproachRise;
    Vec3 raisedFinishBehindApproach;
    Vec3 finishBehindHoleApproach;
    Vec3 capturedLaunchMinusFinish;
    Vec3 extendedHoleApproach;
    Vec3 capturedRecoveryRise;
    Vec3 raisedCapturedRecoveryFinish;
    Vec3 capturedRecoveryFinish;
    Vec3 scaledCapturedEventVelocity;
    Vec3 capturedTrailingEventLead;
    Vec3 capturedSecondControl;
    Vec3 capturedLaunchLead;
    Vec3 capturedFirstEventRise;
    Vec3 raisedCapturedFirstEvent;
    Vec3 capturedFirstEventLead;
    Vec3 capturedFirstControl;
    Vec3 capturedSideAxis;
    Vec3 capturedSideCross;
    Vec3 scaledCapturedSide;
    Vec3 tailFinishRise;
    Vec3 raisedTailFinish;
    Vec3 normalFinishRise;
    Vec3 raisedNormalFinish;
    Vec3 normalSideFinish;
    Vec3 extendedRecoverySide;
    Vec3 recoveryFinishRise;
    Vec3 raisedRecoveryFinish;
    Vec3 recoverySideFinish;
    Vec3 scaledTrailingEventVelocity;
    Vec3 trailingEventVelocityLead;
    Vec3 sideSecondControl;
    Vec3 sideFirstEventVelocity;
    Vec3 sideFirstEventRise;
    Vec3 sideRaisedFirstEvent;
    Vec3 raisedFirstEventVelocityLead;
    Vec3 sideFirstControl;
    Vec3 sideStartPosition;
    Vec3 trajectorySideAxis;
    Vec3 trajectorySideCross;
    Vec3 scaledTrajectorySide;
    Vec3 finalSampleRise;
    Vec3 raisedFinalSample;
    Vec3 trailingSampleRise;
    Vec3 raisedTrailingSample;
    Vec3 trailingVelocityLead;
    Vec3 scaledFirstEventVelocity;
    Vec3 firstEventRise;
    Vec3 raisedFirstEvent;
    Vec3 raisedFirstEventLead;
    Vec3 scaledStartVelocity;
    Vec3 startVelocityLead;
    Vec3 extendedFinishApproach;
    Vec3 capturedCurveFinish;
    Vec3 negativeFinishApproach;
    Vec3 positiveFinishApproach;
    Vec3 launchMinusFinish;
    Vec3 trailingEventRise;
    Vec3 raisedTrailingEvent;
    Vec3 trailingEventSidePosition;
    Vec3 randomSideAxis;
    Vec3 launchSideCross;
    Vec3 scaledLaunchSide;
    Vec3 firstEventLead;
    Vec3 halfLaunchVelocity;
    Vec3 holeWorldPosition;
    Vec3 finishToHoleDelta;
    Vec3 capturedLaunchMinusFinishXZ;
    Vec3 launchMinusFinishXZ;
    Vec3 finishApproachWork;
    Vec3 lateralOffset;
    Vec3 velocityWork;
};
typedef char CameraCurveScratch_size[(sizeof(CameraCurveScratch) == 1200) ? 1 : -1];

enum CameraCurveEventSlot {
    kCurveFirstEventSlot = 2,
    kCurveSecondEventSlot = 1,
    kCurveTrailingEventSlot = 8
};
// Current-hole record: copy XYZ bit patterns with the original integer loads.
// The first three words are the tee position; the next three supply the hole position.
struct CameraCurveHoleRecord {
    int teePositionBits[3];
    int holePositionBits[3];
    char reserved18[8];
};
typedef char CameraCurveHoleRecord_size[(sizeof(CameraCurveHoleRecord) == 32) ? 1 : -1];

// Blend the captured pose through raised event/side controls toward the final sample.
// Scratch is caller-owned; samples and camera state are borrowed world-space inputs.
static inline void BuildCapturedToEventCurve(unsigned char* cameraBytes, unsigned char* ballBytes, int firstEventSample,
                                             int trailingEventSample, CameraCurveScratch& scratch) {
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[0],
                              (unsigned char*)&AsCCameraWork(cameraBytes)->transform[9]);
    __ml__FRC7WVectorf(&scratch.halfLaunchVelocity,
                       ((CameraTrajectorySample*)PreData__9CGolfBallFi(ballBytes, 0))->velocity, kFloat_0_5_CameraWork);
    __as__7WVectorFRC7WVector(&scratch.velocityWork, &scratch.halfLaunchVelocity);
    __pl__FRC7WVectorRC7WVector(&scratch.firstEventLead, PreData__9CGolfBallFi(ballBytes, firstEventSample),
                                &scratch.velocityWork);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[3],
                              &scratch.firstEventLead);
    scratch.randomSideAxis.y =
        AsCCameraWork(cameraBytes)->randomValue % 2 ? kFloat_1_CameraWork : kFloat_neg1_CameraWork;
    scratch.randomSideAxis.x = kFloat_0_CameraWork;
    scratch.randomSideAxis.z = kFloat_0_CameraWork;
    Vector_CrossProduct(&scratch.launchSideCross, &scratch.randomSideAxis, &scratch.velocityWork);
    __ml__FRC7WVectorf(&scratch.scaledLaunchSide, Normalize__7WVectorFv(&scratch.launchSideCross), kFloat_30_0);
    __as__7WVectorFRC7WVector(&scratch.lateralOffset, &scratch.scaledLaunchSide);
    scratch.trailingEventRise.x = kFloat_0_CameraWork;
    scratch.trailingEventRise.y = kFloat_25_CameraWork;
    scratch.trailingEventRise.z = kFloat_0_CameraWork;
    __pl__FRC7WVectorRC7WVector(&scratch.raisedTrailingEvent, PreData__9CGolfBallFi(ballBytes, trailingEventSample),
                                &scratch.trailingEventRise);
    __pl__FRC7WVectorRC7WVector(&scratch.trailingEventSidePosition, &scratch.raisedTrailingEvent,
                                &scratch.lateralOffset);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[6],
                              &scratch.trailingEventSidePosition);
    scratch.finishApproachWork.y = kFloat_0_CameraWork;
    void* finalSample = PreData__9CGolfBallFi(ballBytes, ((CGolfBall*)ballBytes)->m_preDataCount);
    __mi__FRC7WVectorRC7WVector(&scratch.launchMinusFinish, (Vec3*)PreData__9CGolfBallFi(ballBytes, 0), finalSample);
    __as__7WVectorFRC7WVector(&scratch.launchMinusFinishXZ, &scratch.launchMinusFinish);
    scratch.launchMinusFinishXZ.y = kFloat_0_CameraWork;
    if (Vec3_DotProduct(&scratch.launchMinusFinishXZ, &scratch.finishApproachWork) > kFloat_0_CameraWork) {
        __ml__FRC7WVectorf(&scratch.positiveFinishApproach, Normalize__7WVectorFv(&scratch.finishApproachWork),
                           kFloat_15_CameraWork);
        __as__7WVectorFRC7WVector(&scratch.finishApproachWork, &scratch.positiveFinishApproach);
    } else {
        __ml__FRC7WVectorf(&scratch.negativeFinishApproach, Normalize__7WVectorFv(&scratch.finishApproachWork),
                           kFloat_neg15_CameraWork);
        __as__7WVectorFRC7WVector(&scratch.finishApproachWork, &scratch.negativeFinishApproach);
    }
    scratch.finishApproachWork.y += kFloat_5_CameraWork;
    __ml__FRC7WVectorf(&scratch.extendedFinishApproach, &scratch.finishApproachWork, kFloat_5_CameraWork);
    __pl__FRC7WVectorRC7WVector(&scratch.capturedCurveFinish,
                                PreData__9CGolfBallFi(ballBytes, ((CGolfBall*)ballBytes)->m_preDataCount),
                                &scratch.extendedFinishApproach);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[9],
                              &scratch.capturedCurveFinish);
}

// Lead the start/event positions with their velocities and finish above the final sample.
// All vector scratch and curve controls belong to the caller; positions use world XYZ.
static inline void BuildVelocityLeadCurve(unsigned char* cameraBytes, unsigned char* ballBytes, int firstEventSample,
                                          int trailingEventSample, int startSample, CameraCurveScratch& scratch) {
    __as__7WVectorFRC7WVector(&scratch.velocityWork,
                              ((CameraTrajectorySample*)PreData__9CGolfBallFi(ballBytes, startSample))->velocity);
    __ml__FRC7WVectorf(&scratch.scaledStartVelocity, &scratch.velocityWork, kFloat_0_8_CameraWork);
    __pl__FRC7WVectorRC7WVector(&scratch.startVelocityLead, PreData__9CGolfBallFi(ballBytes, startSample),
                                &scratch.scaledStartVelocity);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[0],
                              &scratch.startVelocityLead);
    __as__7WVectorFRC7WVector(&scratch.velocityWork,
                              ((CameraTrajectorySample*)PreData__9CGolfBallFi(ballBytes, firstEventSample))->velocity);
    scratch.firstEventRise.x = kFloat_0_CameraWork;
    scratch.firstEventRise.y = kFloat_50_CameraWork;
    scratch.firstEventRise.z = kFloat_0_CameraWork;
    __ml__FRC7WVectorf(&scratch.scaledFirstEventVelocity, &scratch.velocityWork, kFloat_2_CameraWork);
    __pl__FRC7WVectorRC7WVector(&scratch.raisedFirstEvent, PreData__9CGolfBallFi(ballBytes, firstEventSample),
                                &scratch.firstEventRise);
    __pl__FRC7WVectorRC7WVector(&scratch.raisedFirstEventLead, &scratch.raisedFirstEvent,
                                &scratch.scaledFirstEventVelocity);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[3],
                              &scratch.raisedFirstEventLead);
    __as__7WVectorFRC7WVector(
        &scratch.velocityWork,
        ((CameraTrajectorySample*)PreData__9CGolfBallFi(ballBytes, trailingEventSample))->velocity);
    scratch.trailingSampleRise.x = kFloat_0_CameraWork;
    scratch.trailingSampleRise.y = kFloat_10_cameraWork;
    scratch.trailingSampleRise.z = kFloat_0_CameraWork;
    __pl__FRC7WVectorRC7WVector(&scratch.raisedTrailingSample, PreData__9CGolfBallFi(ballBytes, trailingEventSample),
                                &scratch.trailingSampleRise);
    __pl__FRC7WVectorRC7WVector(&scratch.trailingVelocityLead, &scratch.raisedTrailingSample, &scratch.velocityWork);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[6],
                              &scratch.trailingVelocityLead);
    scratch.finalSampleRise.x = kFloat_0_CameraWork;
    scratch.finalSampleRise.y = kFloat_10_cameraWork;
    scratch.finalSampleRise.z = kFloat_0_CameraWork;
    __pl__FRC7WVectorRC7WVector(&scratch.raisedFinalSample,
                                PreData__9CGolfBallFi(ballBytes, ((CGolfBall*)ballBytes)->m_preDataCount),
                                &scratch.finalSampleRise);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[9],
                              &scratch.raisedFinalSample);
}

// Place controls to one side of the trajectory, extending the endpoint on recovery.
// All vector scratch and curve controls belong to the caller; positions use world XYZ.
static inline void BuildLateralTrajectoryCurve(unsigned char* cameraBytes, unsigned char* ballBytes,
                                               int firstEventSample, int trailingEventSample, int startSample,
                                               CameraCurveScratch& scratch) {
    __as__7WVectorFRC7WVector(&scratch.velocityWork,
                              ((CameraTrajectorySample*)PreData__9CGolfBallFi(ballBytes, 0))->velocity);
    scratch.trajectorySideAxis.y =
        AsCCameraWork(cameraBytes)->randomValue % 2 ? kFloat_1_CameraWork : kFloat_neg1_CameraWork;
    scratch.trajectorySideAxis.x = kFloat_0_CameraWork;
    scratch.trajectorySideAxis.z = kFloat_0_CameraWork;
    Vector_CrossProduct(&scratch.trajectorySideCross, &scratch.trajectorySideAxis, &scratch.velocityWork);
    __ml__FRC7WVectorf(&scratch.scaledTrajectorySide, Normalize__7WVectorFv(&scratch.trajectorySideCross),
                       kFloat_25_CameraWork);
    __as__7WVectorFRC7WVector(&scratch.lateralOffset, &scratch.scaledTrajectorySide);
    __pl__FRC7WVectorRC7WVector(&scratch.sideStartPosition, PreData__9CGolfBallFi(ballBytes, startSample),
                                &scratch.lateralOffset);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[0],
                              &scratch.sideStartPosition);
    __as__7WVectorFRC7WVector(&scratch.velocityWork,
                              ((CameraTrajectorySample*)PreData__9CGolfBallFi(ballBytes, firstEventSample))->velocity);
    scratch.sideFirstEventRise.x = kFloat_0_CameraWork;
    scratch.sideFirstEventRise.y = kFloat_50_CameraWork;
    scratch.sideFirstEventRise.z = kFloat_0_CameraWork;
    __ml__FRC7WVectorf(&scratch.sideFirstEventVelocity, &scratch.velocityWork, kFloat_1_2_CameraWork);
    __pl__FRC7WVectorRC7WVector(&scratch.sideRaisedFirstEvent, PreData__9CGolfBallFi(ballBytes, firstEventSample),
                                &scratch.sideFirstEventRise);
    __pl__FRC7WVectorRC7WVector(&scratch.raisedFirstEventVelocityLead, &scratch.sideRaisedFirstEvent,
                                &scratch.sideFirstEventVelocity);
    __pl__FRC7WVectorRC7WVector(&scratch.sideFirstControl, &scratch.raisedFirstEventVelocityLead,
                                &scratch.lateralOffset);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[3],
                              &scratch.sideFirstControl);
    __as__7WVectorFRC7WVector(
        &scratch.velocityWork,
        ((CameraTrajectorySample*)PreData__9CGolfBallFi(ballBytes, trailingEventSample))->velocity);
    __ml__FRC7WVectorf(&scratch.scaledTrailingEventVelocity, &scratch.velocityWork, kFloat_1_5_GolfCamera);
    __pl__FRC7WVectorRC7WVector(&scratch.trailingEventVelocityLead,
                                PreData__9CGolfBallFi(ballBytes, trailingEventSample),
                                &scratch.scaledTrailingEventVelocity);
    __pl__FRC7WVectorRC7WVector(&scratch.sideSecondControl, &scratch.trailingEventVelocityLead, &scratch.lateralOffset);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[6],
                              &scratch.sideSecondControl);
    if (((CGolfBall*)ballBytes)->m_recoveryRequired) {
        scratch.recoveryFinishRise.x = kFloat_0_CameraWork;
        scratch.recoveryFinishRise.y = kFloat_30_0;
        scratch.recoveryFinishRise.z = kFloat_0_CameraWork;
        __ml__FRC7WVectorf(&scratch.extendedRecoverySide, &scratch.lateralOffset, kFloat_5_CameraWork);
        __pl__FRC7WVectorRC7WVector(&scratch.raisedRecoveryFinish,
                                    PreData__9CGolfBallFi(ballBytes, ((CGolfBall*)ballBytes)->m_preDataCount),
                                    &scratch.recoveryFinishRise);
        __pl__FRC7WVectorRC7WVector(&scratch.recoverySideFinish, &scratch.raisedRecoveryFinish,
                                    &scratch.extendedRecoverySide);
        __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[9],
                                  &scratch.recoverySideFinish);
    } else {
        scratch.normalFinishRise.x = kFloat_0_CameraWork;
        scratch.normalFinishRise.y = kFloat_10_cameraWork;
        scratch.normalFinishRise.z = kFloat_0_CameraWork;
        __pl__FRC7WVectorRC7WVector(&scratch.raisedNormalFinish,
                                    PreData__9CGolfBallFi(ballBytes, ((CGolfBall*)ballBytes)->m_preDataCount),
                                    &scratch.normalFinishRise);
        __pl__FRC7WVectorRC7WVector(&scratch.normalSideFinish, &scratch.raisedNormalFinish, &scratch.lateralOffset);
        __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[9],
                                  &scratch.normalSideFinish);
    }
}

// Follow the last portion of the trajectory from the second event to its final frame.
// All vector scratch and curve controls belong to the caller; positions use world XYZ.
static inline void BuildSecondEventTailCurve(unsigned char* cameraBytes, unsigned char* ballBytes,
                                             int secondEventSample, CameraCurveScratch& scratch) {
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[0],
                              PreData__9CGolfBallFi(ballBytes, secondEventSample));
    __as__7WVectorFRC7WVector(
        (unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[3],
        PreData__9CGolfBallFi(ballBytes, secondEventSample + (int)(kFloat_0_5_CameraWork *
                                                                   (float)(((CGolfBall*)ballBytes)->m_preDataCount -
                                                                           secondEventSample))));
    __as__7WVectorFRC7WVector(
        (unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[6],
        PreData__9CGolfBallFi(ballBytes, secondEventSample + (int)(kFloat_0_75_CameraWork *
                                                                   (float)(((CGolfBall*)ballBytes)->m_preDataCount -
                                                                           secondEventSample))));
    scratch.tailFinishRise.x = kFloat_0_CameraWork;
    scratch.tailFinishRise.y = kFloat_3_CameraWork;
    scratch.tailFinishRise.z = kFloat_0_CameraWork;
    __pl__FRC7WVectorRC7WVector(&scratch.raisedTailFinish,
                                PreData__9CGolfBallFi(ballBytes, (int)AsCCameraWork(cameraBytes)->trajectoryLastFrame),
                                &scratch.tailFinishRise);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[9],
                              &scratch.raisedTailFinish);
}

// Blend a captured pose through lateral controls; choose the endpoint by recovery and approach direction.
// All vector scratch and curve controls belong to the caller; positions use world XYZ.
static inline void BuildCapturedSideCurve(unsigned char* cameraBytes, unsigned char* ballBytes, int firstEventSample,
                                          int trailingEventSample, CameraCurveScratch& scratch) {
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[0],
                              (unsigned char*)&AsCCameraWork(cameraBytes)->transform[9]);
    __as__7WVectorFRC7WVector(&scratch.velocityWork,
                              ((CameraTrajectorySample*)PreData__9CGolfBallFi(ballBytes, 0))->velocity);
    scratch.capturedSideAxis.y =
        AsCCameraWork(cameraBytes)->randomValue % 2 ? kFloat_1_CameraWork : kFloat_neg1_CameraWork;
    scratch.capturedSideAxis.x = kFloat_0_CameraWork;
    scratch.capturedSideAxis.z = kFloat_0_CameraWork;
    Vector_CrossProduct(&scratch.capturedSideCross, &scratch.capturedSideAxis, &scratch.velocityWork);
    __ml__FRC7WVectorf(&scratch.scaledCapturedSide, Normalize__7WVectorFv(&scratch.capturedSideCross), kFloat_30_0);
    __as__7WVectorFRC7WVector(&scratch.lateralOffset, &scratch.scaledCapturedSide);
    scratch.capturedFirstEventRise.x = kFloat_0_CameraWork;
    scratch.capturedFirstEventRise.y = kFloat_25_CameraWork;
    scratch.capturedFirstEventRise.z = kFloat_0_CameraWork;
    __ml__FRC7WVectorf(&scratch.capturedLaunchLead, &scratch.velocityWork, kFloat_2_5_CameraWork);
    __pl__FRC7WVectorRC7WVector(&scratch.raisedCapturedFirstEvent, PreData__9CGolfBallFi(ballBytes, firstEventSample),
                                &scratch.capturedFirstEventRise);
    __pl__FRC7WVectorRC7WVector(&scratch.capturedFirstEventLead, &scratch.raisedCapturedFirstEvent,
                                &scratch.capturedLaunchLead);
    __pl__FRC7WVectorRC7WVector(&scratch.capturedFirstControl, &scratch.capturedFirstEventLead, &scratch.lateralOffset);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[3],
                              &scratch.capturedFirstControl);
    __as__7WVectorFRC7WVector(&scratch.velocityWork,
                              ((CameraTrajectorySample*)PreData__9CGolfBallFi(ballBytes, firstEventSample))->velocity);
    __ml__FRC7WVectorf(&scratch.scaledCapturedEventVelocity, &scratch.velocityWork, kFloat_1_5_GolfCamera);
    __pl__FRC7WVectorRC7WVector(&scratch.capturedTrailingEventLead,
                                PreData__9CGolfBallFi(ballBytes, trailingEventSample),
                                &scratch.scaledCapturedEventVelocity);
    __pl__FRC7WVectorRC7WVector(&scratch.capturedSecondControl, &scratch.capturedTrailingEventLead,
                                &scratch.lateralOffset);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[6],
                              &scratch.capturedSecondControl);
    if (((CGolfBall*)ballBytes)->m_recoveryRequired) {
        scratch.capturedRecoveryRise.x = kFloat_0_CameraWork;
        scratch.capturedRecoveryRise.y = kFloat_5_CameraWork;
        scratch.capturedRecoveryRise.z = kFloat_0_CameraWork;
        __ml__FRC7WVectorf(&scratch.extendedHoleApproach, &scratch.finishApproachWork, kFloat_40_cameraWork);
        __pl__FRC7WVectorRC7WVector(&scratch.raisedCapturedRecoveryFinish,
                                    PreData__9CGolfBallFi(ballBytes, ((CGolfBall*)ballBytes)->m_preDataCount),
                                    &scratch.capturedRecoveryRise);
        __pl__FRC7WVectorRC7WVector(&scratch.capturedRecoveryFinish, &scratch.raisedCapturedRecoveryFinish,
                                    &scratch.extendedHoleApproach);
        __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[9],
                                  &scratch.capturedRecoveryFinish);
    } else {
        void* finalSample = PreData__9CGolfBallFi(ballBytes, ((CGolfBall*)ballBytes)->m_preDataCount);
        __mi__FRC7WVectorRC7WVector(&scratch.capturedLaunchMinusFinish, (Vec3*)PreData__9CGolfBallFi(ballBytes, 0),
                                    finalSample);
        __as__7WVectorFRC7WVector(&scratch.capturedLaunchMinusFinishXZ, &scratch.capturedLaunchMinusFinish);
        scratch.capturedLaunchMinusFinishXZ.y = kFloat_0_CameraWork;
        if (Vec3_DotProduct(&scratch.capturedLaunchMinusFinishXZ, &scratch.finishApproachWork) > kFloat_0_CameraWork) {
            scratch.behindApproachRise.x = kFloat_0_CameraWork;
            scratch.behindApproachRise.y = kFloat_10_cameraWork;
            scratch.behindApproachRise.z = kFloat_0_CameraWork;
            __ml__FRC7WVectorf(&scratch.behindApproachOffset, &scratch.finishApproachWork, kCameraShared20);
            __pl__FRC7WVectorRC7WVector(&scratch.raisedFinishBehindApproach,
                                        PreData__9CGolfBallFi(ballBytes, ((CGolfBall*)ballBytes)->m_preDataCount),
                                        &scratch.behindApproachRise);
            __mi__FRC7WVectorRC7WVector(&scratch.finishBehindHoleApproach, &scratch.raisedFinishBehindApproach,
                                        &scratch.behindApproachOffset);
            __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[9],
                                      &scratch.finishBehindHoleApproach);
        } else {
            scratch.aheadApproachRise.x = kFloat_0_CameraWork;
            scratch.aheadApproachRise.y = kFloat_10_cameraWork;
            scratch.aheadApproachRise.z = kFloat_0_CameraWork;
            __ml__FRC7WVectorf(&scratch.aheadApproachOffset, &scratch.finishApproachWork, kCameraShared20);
            __pl__FRC7WVectorRC7WVector(&scratch.raisedFinishAheadOfApproach,
                                        PreData__9CGolfBallFi(ballBytes, ((CGolfBall*)ballBytes)->m_preDataCount),
                                        &scratch.aheadApproachRise);
            __pl__FRC7WVectorRC7WVector(&scratch.finishAheadOfHoleApproach, &scratch.raisedFinishAheadOfApproach,
                                        &scratch.aheadApproachOffset);
            __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[9],
                                      &scratch.finishAheadOfHoleApproach);
        }
    }
}

// Use random-side raised event controls and a shortened lateral endpoint offset.
// All vector scratch and curve controls belong to the caller; positions use world XYZ.
static inline void BuildRandomOffsetCurve(unsigned char* cameraBytes, unsigned char* ballBytes, int firstEventSample,
                                          int trailingEventSample, int& randomSideParity, float& randomSideSign,
                                          float& randomSideZ, float& randomSideX, CameraCurveScratch& scratch) {
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[0],
                              (unsigned char*)&AsCCameraWork(cameraBytes)->position[0]);
    randomSideParity = AsCCameraWork(cameraBytes)->randomValue % 2;
    randomSideZ = kFloat_10_cameraWork * (randomSideParity ? kFloat_1_CameraWork : kFloat_neg1_CameraWork);
    randomSideSign = randomSideParity ? kFloat_1_CameraWork : kFloat_neg1_CameraWork;
    randomSideX = kFloat_10_cameraWork * randomSideSign;
    scratch.randomFirstEventOffset.z = randomSideZ;
    scratch.randomFirstEventOffset.y = kFloat_50_CameraWork;
    scratch.randomFirstEventOffset.x = randomSideX;
    __pl__FRC7WVectorRC7WVector(&scratch.randomFirstControl, PreData__9CGolfBallFi(ballBytes, firstEventSample),
                                &scratch.randomFirstEventOffset);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[3],
                              &scratch.randomFirstControl);
    scratch.randomSideUp.y = AsCCameraWork(cameraBytes)->randomValue % 2 ? kFloat_1_CameraWork : kFloat_neg1_CameraWork;
    scratch.randomSideUp.x = kFloat_0_CameraWork;
    scratch.randomSideUp.z = kFloat_0_CameraWork;
    Vector_CrossProduct(&scratch.randomSideCross, &scratch.randomSideUp, &scratch.finishApproachWork);
    __ml__FRC7WVectorf(&scratch.scaledRandomSide, Normalize__7WVectorFv(&scratch.randomSideCross),
                       kFloat_15_CameraWork);
    __as__7WVectorFRC7WVector(&scratch.lateralOffset, &scratch.scaledRandomSide);
    randomSideParity = AsCCameraWork(cameraBytes)->randomValue % 2;
    randomSideZ = kFloat_10_cameraWork * (randomSideParity ? kFloat_1_CameraWork : kFloat_neg1_CameraWork);
    randomSideSign = randomSideParity ? kFloat_1_CameraWork : kFloat_neg1_CameraWork;
    randomSideX = kFloat_10_cameraWork * randomSideSign;
    scratch.randomTrailingOffset.z = randomSideZ;
    scratch.randomTrailingOffset.y = kFloat_25_CameraWork;
    scratch.randomTrailingOffset.x = randomSideX;
    __pl__FRC7WVectorRC7WVector(&scratch.randomRaisedTrailingEvent,
                                PreData__9CGolfBallFi(ballBytes, trailingEventSample), &scratch.randomTrailingOffset);
    __pl__FRC7WVectorRC7WVector(&scratch.randomSecondControl, &scratch.randomRaisedTrailingEvent,
                                &scratch.lateralOffset);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[6],
                              &scratch.randomSecondControl);
    __amu__7WVectorFf(&scratch.finishApproachWork, kFloat_neg15_CameraWork);
    __ml__FRC7WVectorf(&scratch.shortenedRandomSide, &scratch.lateralOffset, kFloat_0_3_CameraWork);
    __apl__7WVectorFRC7WVector(&scratch.finishApproachWork, &scratch.shortenedRandomSide);
    scratch.finishApproachWork.y += kFloat_3_CameraWork;
    if (((CGolfBall*)ballBytes)->m_recoveryRequired) {
        __ml__FRC7WVectorf(&scratch.extendedRandomRecoveryApproach, &scratch.finishApproachWork, kFloat_5_CameraWork);
        __pl__FRC7WVectorRC7WVector(&scratch.randomRecoveryFinish,
                                    PreData__9CGolfBallFi(ballBytes, ((CGolfBall*)ballBytes)->m_preDataCount),
                                    &scratch.extendedRandomRecoveryApproach);
        __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[9],
                                  &scratch.randomRecoveryFinish);
    } else {
        __ml__FRC7WVectorf(&scratch.scaledRandomNormalApproach, &scratch.finishApproachWork, kFloat_3_CameraWork);
        __pl__FRC7WVectorRC7WVector(&scratch.randomNormalFinish,
                                    PreData__9CGolfBallFi(ballBytes, ((CGolfBall*)ballBytes)->m_preDataCount),
                                    &scratch.scaledRandomNormalApproach);
        __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[9],
                                  &scratch.randomNormalFinish);
    }
}

// Cross the event interval with a wide lateral baseline and finish behind the final sample.
// All vector scratch and curve controls belong to the caller; positions use world XYZ.
static inline void BuildWideEventCrossingCurve(unsigned char* cameraBytes, unsigned char* ballBytes,
                                               int firstEventSample, int trailingEventSample,
                                               CameraCurveScratch& scratch) {
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[0],
                              (unsigned char*)&AsCCameraWork(cameraBytes)->position[0]);
    scratch.wideSideDown.x = kFloat_0_CameraWork;
    scratch.wideSideDown.y = kFloat_neg1_CameraWork;
    scratch.wideSideDown.z = kFloat_0_CameraWork;
    Vector_CrossProduct(&scratch.wideSideCross, &scratch.wideSideDown,
                        ((CameraTrajectorySample*)PreData__9CGolfBallFi(ballBytes, 0))->velocity);
    __ml__FRC7WVectorf(&scratch.scaledWideSide, Normalize__7WVectorFv(&scratch.wideSideCross), kFloat_100_GolfCamera);
    __as__7WVectorFRC7WVector(&scratch.lateralOffset, &scratch.scaledWideSide);
    scratch.wideFirstEventRise.x = kFloat_0_CameraWork;
    scratch.wideFirstEventRise.y = kFloat_10_cameraWork;
    scratch.wideFirstEventRise.z = kFloat_0_CameraWork;
    __pl__FRC7WVectorRC7WVector(&scratch.raisedWideFirstEvent, PreData__9CGolfBallFi(ballBytes, firstEventSample),
                                &scratch.wideFirstEventRise);
    __pl__FRC7WVectorRC7WVector(&scratch.wideFirstControl, &scratch.raisedWideFirstEvent, &scratch.lateralOffset);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[3],
                              &scratch.wideFirstControl);
    scratch.eventMidpointRise.x = kFloat_0_CameraWork;
    scratch.eventMidpointRise.y = kFloat_10_cameraWork;
    scratch.eventMidpointRise.z = kFloat_0_CameraWork;
    __pl__FRC7WVectorRC7WVector(&scratch.raisedEventMidpoint,
                                PreData__9CGolfBallFi(ballBytes, (firstEventSample + trailingEventSample) / 2),
                                &scratch.eventMidpointRise);
    __mi__FRC7WVectorRC7WVector(&scratch.wideSecondControl, &scratch.raisedEventMidpoint, &scratch.lateralOffset);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[6],
                              &scratch.wideSecondControl);
    __amu__7WVectorFf(&scratch.finishApproachWork, kFloat_neg20_CameraWork);
    scratch.finishApproachWork.y += kFloat_5_CameraWork;
    __pl__FRC7WVectorRC7WVector(&scratch.wideCurveFinish,
                                PreData__9CGolfBallFi(ballBytes, ((CGolfBall*)ballBytes)->m_preDataCount),
                                &scratch.finishApproachWork);
    __as__7WVectorFRC7WVector((unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[9],
                              &scratch.wideCurveFinish);
}

#pragma pop

} // extern "C"
} // namespace ns1

using ns1::CameraCurveScratch;
using ns1::BuildCapturedToEventCurve;
using ns1::BuildVelocityLeadCurve;
using ns1::BuildLateralTrajectoryCurve;
using ns1::BuildSecondEventTailCurve;
using ns1::BuildCapturedSideCurve;
using ns1::BuildRandomOffsetCurve;
using ns1::BuildWideEventCrossingCurve;
using ns1::kCurveFirstEventSlot;
using ns1::kCurveSecondEventSlot;
using ns1::kCurveTrailingEventSlot;
using ns1::CameraCurveHoleRecord;
using ns1::kFloat_0_1_805DDF94;
using ns1::kFloat_1_CameraWork;
// Build four world-space Bezier controls for action, then clamp target rise to 1..5 game units.
// self owns the output controls; trajectory samples and current-hole records are borrowed.
void CCameraWork::BuildModeCurve(unsigned int action) {
    CameraCurveScratch scratch;
    int firstEventSample, secondEventSample, trailingEventSample, startSample;
    unsigned char* ballBytes;
    unsigned char* cameraBytes = (unsigned char*)this;
    ballBytes = (unsigned char*)GolfBall__Fv();
    startSample = (int)AsCCameraWork(cameraBytes)->motionStartFrame;
    int rawFirstEventSample = ((CGolfBall*)ballBytes)->m_eventPreDataIndex[kCurveFirstEventSlot];
    int rawSecondEventSample = ((CGolfBall*)ballBytes)->m_eventPreDataIndex[kCurveSecondEventSlot];
    int rawTrailingEventSample = ((CGolfBall*)ballBytes)->m_eventPreDataIndex[kCurveTrailingEventSlot];
    int finalSampleIndex = ((CGolfBall*)ballBytes)->m_preDataCount;
    // Branchless max(index, 0); retain the signed mask for the original instruction sequence.
    firstEventSample = rawFirstEventSample & ((-rawFirstEventSample & ~rawFirstEventSample) >> 31);
    secondEventSample = rawSecondEventSample & ((-rawSecondEventSample & ~rawSecondEventSample) >> 31);
    trailingEventSample = rawTrailingEventSample & ((-rawTrailingEventSample & ~rawTrailingEventSample) >> 31);
    if (startSample < 0)
        startSample = 0;
    else if (startSample > finalSampleIndex)
        startSample = finalSampleIndex;
    // Phase 1: clamp event/start indices and form the final-sample-to-hole direction.
    AsCCameraWork(cameraBytes)->targetRiseHeight = kFloat_5_CameraWork;
    void* golfDocument = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    int holeYBits, holeXBits;
    CameraCurveHoleRecord* holeRecord = (CameraCurveHoleRecord*)AsCGolfDoc(golfDocument)->holeRecords +
                                        *((unsigned char*)&AsCGolfDoc(golfDocument)->currentHoleIndex);
    struct Words {
        int x, y, z;
    };
    holeXBits = holeRecord->holePositionBits[0];
    holeYBits = holeRecord->holePositionBits[1];
    ((Words*)&scratch.holeWorldPosition)->x = holeXBits;
    ((Words*)&scratch.holeWorldPosition)->y = holeYBits;
    ((Words*)&scratch.holeWorldPosition)->z = holeRecord->holePositionBits[2];
    __mi__FRC7WVectorRC7WVector(&scratch.finishToHoleDelta, &scratch.holeWorldPosition,
                                ns1::PreData__9CGolfBallFi(ballBytes, ((CGolfBall*)ballBytes)->m_preDataCount));
    __as__7WVectorFRC7WVector(&scratch.finishApproachWork, ns1::Normalize__7WVectorFv(&scratch.finishToHoleDelta));
    // Phase 2: choose endpoints and two intermediate controls for the action's framing.
    switch (action) {
    // Blend a captured pose through raised event/side controls.
    case kCameraAction0: {
        BuildCapturedToEventCurve(cameraBytes, ballBytes, firstEventSample, trailingEventSample, scratch);
        break;
    }
    // Lead sampled positions with velocity.
    case kCameraAction1:
        BuildVelocityLeadCurve(cameraBytes, ballBytes, firstEventSample, trailingEventSample, startSample, scratch);
        break;
    // Frame the sampled trajectory from a lateral offset.
    case kCameraAction2:
        BuildLateralTrajectoryCurve(cameraBytes, ballBytes, firstEventSample, trailingEventSample, startSample,
                                    scratch);
        break;
    // Track the interval after the second event.
    case kCameraSecondEventView:
    case kCameraSecondEventAlternateView:
        BuildSecondEventTailCurve(cameraBytes, ballBytes, secondEventSample, scratch);
        break;
    // Blend a captured pose into a side view.
    case kCameraAction8: {
        BuildCapturedSideCurve(cameraBytes, ballBytes, firstEventSample, trailingEventSample, scratch);
        break;
    }
    // Use random-side offsets around event positions.
    case kCameraWalkView12: {
        int randomSideParity;
        float randomSideSign, randomSideZ, randomSideX;
        BuildRandomOffsetCurve(cameraBytes, ballBytes, firstEventSample, trailingEventSample, randomSideParity,
                               randomSideSign, randomSideZ, randomSideX, scratch);
        break;
    }
    // Cross the event interval from a wide lateral baseline.
    case kCameraWalkView18:
        BuildWideEventCrossingCurve(cameraBytes, ballBytes, firstEventSample, trailingEventSample, scratch);
        break;
    }
    // Phase 3: target rise = clamp(0.1 * distance(final sample, final control), 1, 5).
    __mi__FRC7WVectorRC7WVector(&scratch.finalControlToFinalSampleDelta,
                                (Vec3*)ns1::PreData__9CGolfBallFi(ballBytes, ((CGolfBall*)ballBytes)->m_preDataCount),
                                (unsigned char*)&AsCCameraWork(cameraBytes)->curveControlPoints[9]);
    float unclampedRiseHeight =
        kFloat_0_1_805DDF94 * ns1::Magnitude__7WVectorCFv(&scratch.finalControlToFinalSampleDelta);
    float riseHeight = kFloat_1_CameraWork;
    AsCCameraWork(cameraBytes)->targetRiseHeight = unclampedRiseHeight;
    riseHeight = unclampedRiseHeight < riseHeight            ? riseHeight
                 : unclampedRiseHeight > kFloat_5_CameraWork ? kFloat_5_CameraWork
                                                             : unclampedRiseHeight;
    AsCCameraWork(cameraBytes)->targetRiseHeight = riseHeight;
}

namespace ns1 {
extern "C" {} // extern "C"

} // namespace ns1

// ---- merged from camera_work_shot_cameras.cpp ----
namespace ns2 {

extern "C" {

typedef Vec3f Vec3;
struct Mat34 {
    float m[12];
};
struct Pair {
    float yaw, pitch;
};

struct CameraBezierState {
    // curvePoint0..3 are the same four XYZ controls exposed by CCameraWork at +0xE4.
    char reserved00[0x2C];
    Vec3 position;
    Vec3 target;
    char reserved44[0x20];
    float firstEventFrame;
    float secondEventFrame;
    float trajectoryLastFrame;
    float curveStartFrame;
    float elapsedTime;
    char reserved78[0x6C];
    Vec3 curvePoint0;
    Vec3 curvePoint1;
    Vec3 curvePoint2;
    Vec3 curvePoint3;
    char reserved114[0x30];
    float motionStartPositionX; // +144 is motionStartPosition.x in the shared CCameraWork view; unused by this callback
    char reserved148[8];
    float targetRiseHeight;
    float animationTime;
    char reserved158[8];
    float lastProximitySoundUpdateTime;
    char reserved164[4];
    int state;
};

struct CameraSplineState {
    char reserved00[0x2C];
    Vec3 position;
    Vec3 target;
    char reserved44[0x18];
    int pendingAction;
    int actionFlags;
    char reserved64[4]; // +5C pending action; +60 action flags
    float secondEventFrame;
    float trajectoryLastFrame;
    float curveStartFrame;
    float elapsedTime;
    int cutoffFrame; // +78 cutoff trajectory frame
    char reserved7C[0x68];
    Vec3 curvePoint0;
    Vec3 curvePoint1;
    Vec3 curvePoint2;
    Vec3 curvePoint3;
};

extern float kFloat_1_CameraWork, kFloat_3_CameraWork, kFloat_40_cameraWork, kCameraSecondEventTrackingBoundaryFraction,
    kFloat_10_cameraWork;
extern float kCameraWindSoundBallSpeedThreshold, kFloat_100_GolfCamera;
extern const double kSignedIntToDoubleBias_CameraWork;

extern void* g_soundManager;
extern const CameraWorkStrings cameraWorkStrings;
void Sound_StopByAlias(void*, const char*);
void Sound_PlayByName(void*, const char*, Vec3*, int, const char*, int, float, float, float);
int SoundManager_IsAliasPrefixOrStreamStemActive(void*, const char*);
void SoundManager_SetNamedVolumeScale(void*, const char*, float);
Vec3* GolfBall__Fv();
void __ml__FfRC7WVector(Vec3*, const Vec3*, float);
void __pl__FRC7WVectorRC7WVector(Vec3*, const Vec3*, const Vec3*);
void __mi__FRC7WVectorRC7WVector(Vec3*, const Vec3*, const Vec3*);
void __as__7WVectorFRC7WVector(Vec3*, const Vec3*);
float Magnitude__7WVectorCFv(const Vec3*);
void Vec3_CopySourceToDestination(Vec3*, const Vec3*);
void Matrix_SetIdentityAffine(Vec3*);
void __ml__FRC7WVectorf(Vec3*, const Vec3*, float);
void Vector_CrossProduct(Vec3*, const Vec3*, const Vec3*);
void Normalize__7WVectorFv(Vec3*);
void Matrix_ConcatAffineAssign(Vec3*, const Vec3*);
void Matrix_CopyAffine(Vec3*, const Vec3*);
void __apl__7WVectorFRC7WVector(Vec3*, const Vec3*);
void Matrix_SetAxisRotation(Vec3*, float, int);
void* TaskActorTable_FindByName(void*, const char*, int, int);
void* Object_GetChild(void*);
int rand();
int __ptmf_test(void*);
void* CQuadTree_GetInstance();
float WVectorLen__FRC7WVector(const Vec3*);
void* PreData__9CGolfBallFi(void*, int);
void* InstanceTable_Get();
extern void* g_taskManager;
extern CSharedDoc* g_sharedDoc;
extern float kCameraVectorEpsilon, kFloat_3_1415927_CameraWork, kFloat_0_8_CameraWork, kFloat_0_1_805DDF94;
extern const float kFloat_0_CameraWork;
extern float kFloat_6_CameraWork, kCameraShared20, kFloat_10_cameraWork, kCameraGroundEffectMinimumBallDistance,
    kFloat_0_17453292, kCameraGroundEffectMinimumFovRadians, kFloat_0_95993108, kFloat_5_CameraWork;
extern float kFloat_0_2617994, kFloat_2_CameraWork, kFloat_0_5235988_805DDFCC, kCameraManualMaximumFovRadians,
    kCameraMinimumYawRadians, kCameraMinPitchRadians;
extern const float kFloat_1_0471976_cameraWork;
extern CSharedDoc* g_sharedDoc;
extern signed char s_inputManagerInitialized;
extern void* g_view;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_CameraWork[];
extern char s_cameraTransform[];
void InputManager_Init(void*);
void InputManager_Dtor();
void __register_global_object(void*, void (*)(), void*);
int SharedDoc_IsPlayerFlag0Set(int);
int Input_IsActionButtonDown(void*, int, int);
void Vec2_Zero(Vec3*);
void Camera_SetFov(void*, float);
void* Matrix_TransformPoint(Vec3*, const Vec3*, const Vec3*);
void Matrix_BuildCameraTransform(void*, const void*, const void*);
float Math_CosApprox(float);
float Math_SinApprox(float);

struct CameraInputState {
    char pad00[0x10];
    float x;
    float y;
    float z;
    unsigned char moved;
    char pad1d[0x183];
    unsigned char cameraMode;
    char pad1a1[0xa0];
};

struct CameraTargetState;
typedef void (CameraTargetState::*TargetCallback)(float, float);

struct CameraTargetState {
    char p00[0x20];
    // 0x20: a CodeWarrior member-function pointer {this delta, virtual offset or -1, function} (12 bytes).
    TargetCallback callback; // 0x20 (12 bytes)
    Vec3 position;           // 0x2c
    char p38[4];
    float targetY; // 0x3c
    char p40[0x14];
    int action;        // 0x54
    int mode;          // 0x58
    int pendingAction; // 0x5c
    int actionFlags;   // 0x60
    char p64[8];
    float trajectoryLastFrame; // 0x6c
    float curveStartFrame;     // 0x70
    float elapsedTime;         // 0x74
    char p78[4];
    int previousFrameIndex; // 0x7c
    int frameIndex;         // 0x80
    char p84[0x30];
    CameraTransformView previousPose; // 0xb4..0xe3: basis plus world position
    char pE4[0x60];
    Vec3 vec144; // 0x144
    char p150[4];
    float animationTime;  // 0x154
    float lastShakeTime;  // 0x158
    float shakeStartTime; // 0x15c
    char p160[5];
    unsigned char flag165; // 0x165
    char p166[0x3d];
    unsigned char scriptedAnimationEnabled; // 0x1a3
    Vec3 pivotPosition;                     // 0x1a4
    int animationGroup;                     // 0x1b0
    int playingAnimationGroup;              // 0x1b4
    Vec3 initialBallPosition;               // 0x1b8
    char p1c4[0x44];
    float fov; // 0x208
    char p20c[4];
    unsigned char groundClearanceEnabled; // 0x210
    unsigned char forceGroundEffect;      // 0x211
    char p212[0x32];
    CameraTransformView savedPose; // 0x244..0x273: basis plus world position
    float savedFov;                // 0x274
    char p278[0x80];
    int animationGroupIndex; // 0x2f8
    int groupValueCount;     // 0x2fc
};

struct TargetSink {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void send(void*);
};
struct TargetCmd {
    int a;
    int b;
    void* p;
    int r[5];
};

// Test the camera zero tolerance while preserving the original sign-bit handling.
static inline bool CameraTargetIsNearZero(const float& v) {
    return ((*(const unsigned*)&v & 0x80000000u) ? -v : v) < kCameraVectorEpsilon;
}
} // extern "C"
} // namespace ns2

extern "C" void Sound_StopByAlias(void*, const char*);
extern "C" const CameraWorkStrings cameraWorkStrings;
extern "C" void* g_soundManager;

// Mark the proximity sound stopped and stop its shared audio alias.
void CCameraWork::StopBallSound() {
    ballProximitySoundState = 2;
    Sound_StopByAlias(g_soundManager, cameraWorkStrings.ballWindSoundAlias);
}

namespace ns2 {
extern "C" {} // extern "C"
} // namespace ns2

typedef Vec3f Vec3;
extern "C" {
extern const float kFloat_1_CameraWork;
extern const float kFloat_3_CameraWork;
extern const float kFloat_0_CameraWork;
extern void __ml__FfRC7WVector(void*, const void*, float);
extern void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern void __as__7WVectorFRC7WVector(void*, const void*);
extern char* GolfBall__Fv();
extern void Sound_StopByAlias(void*, const char*);
extern float Magnitude__7WVectorCFv(const void*);
extern void Sound_PlayByName(void*, const char*, void*, int, const char*, int, float, float, float);
extern const float kFloat_0_1_805DDF94;
extern float kCameraWindSoundBallSpeedThreshold;
extern const float kFloat_100_GolfCamera;
int SoundManager_IsAliasPrefixOrStreamStemActive(void*, const char*);
void SoundManager_SetNamedVolumeScale(void*, const char*, float);
}

// Move the camera on a cubic Bezier curve while looking at the ball with a rising target.
// P(t)=(1-t)^3*P0 + 3*(1-t)^2*t*P1 + 3*(1-t)*t^2*P2 + t^3*P3.
// frameFraction supplies the fractional frame offset; the target-rise phase uses the same frame clock.
void CCameraWork::UpdateBezierPositionAndBallTarget(float unused, float frameFraction) {
    float u3, u2, u, t2, t;
    t = (elapsedTime + frameFraction - motionStartFrame) / (trajectoryLastFrame - motionStartFrame);
    t2 = t * t;
    u = kFloat_1_CameraWork - t;
    u2 = u * u;
    u3 = u2 * u;
    Vec3 h, offset, a, b, c, d, e, f, g;
    __ml__FfRC7WVector(&a, &curveControlPoints[9], t * t2);
    float _b = kFloat_3_CameraWork * u;
    __ml__FfRC7WVector(&b, &curveControlPoints[6], _b * t2);
    float _c = kFloat_3_CameraWork * u2;
    __ml__FfRC7WVector(&c, &curveControlPoints[3], _c * t);
    __ml__FfRC7WVector(&d, &curveControlPoints[0], u3);
    __pl__FRC7WVectorRC7WVector(&e, &d, &c);
    __pl__FRC7WVectorRC7WVector(&f, &e, &b);
    __pl__FRC7WVectorRC7WVector(&g, &f, &a);
    __as__7WVectorFRC7WVector(position, &g);
    if (elapsedTime < secondEventFrame) {
        __as__7WVectorFRC7WVector(target, GolfBall__Fv());
    } else if (elapsedTime < trajectoryLastFrame) {
        float mul = targetRiseHeight;
        float num = elapsedTime + frameFraction - secondEventFrame;
        float den = trajectoryLastFrame - secondEventFrame;
        offset.x = kFloat_0_CameraWork;
        offset.z = kFloat_0_CameraWork;
        offset.y = (num / den) * mul;
        __pl__FRC7WVectorRC7WVector(&h, GolfBall__Fv(), &offset);
        __as__7WVectorFRC7WVector(target, &h);
    }
}

// Start, attenuate, or stop the ball wind sound according to camera distance and ball speed.
// State map: 0 not started, 1 nearby/playing, 2 stopped, 3 out of range but restartable.
// Odd states continue speed/distance checks; attenuation is (100-distance)/100 within range.
void CCameraWork::UpdateBallProximitySound() {
    float delta = animationTime - lastProximitySoundUpdateTime;
    float cameraBallDistance;
    Vec3 b, cameraToBall;
    if (delta < kFloat_0_1_805DDF94 && ballProximitySoundState != 1)
        return;
    {
        lastProximitySoundUpdateTime = animationTime;
        __mi__FRC7WVectorRC7WVector(&cameraToBall, GolfBall__Fv(), position);
        __as__7WVectorFRC7WVector(&b, &cameraToBall);
        cameraBallDistance = Magnitude__7WVectorCFv(&b);
        if (ballProximitySoundState == 0) {
            float ballSpeed = Magnitude__7WVectorCFv(GolfBall__Fv() + 1);
            if (ballSpeed > kCameraWindSoundBallSpeedThreshold && cameraBallDistance < kFloat_100_GolfCamera) {
                ballProximitySoundState = 1;
                Sound_PlayByName(g_soundManager, cameraWorkStrings.ballWindSoundAlias, &b, 5,
                                 cameraWorkStrings.ballWindSoundAlias, 0, kFloat_0_CameraWork, kFloat_100_GolfCamera,
                                 kFloat_1_CameraWork);
            }
        } else if (ballProximitySoundState & 1) {
            float ballSpeed = Magnitude__7WVectorCFv(GolfBall__Fv() + 1);
            if (ballSpeed > kCameraWindSoundBallSpeedThreshold) {
                if (cameraBallDistance < kFloat_100_GolfCamera) {
                    if (!SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager,
                                                                      cameraWorkStrings.ballWindSoundAlias))
                        Sound_PlayByName(g_soundManager, cameraWorkStrings.ballWindSoundAlias, &b, 5,
                                         cameraWorkStrings.ballWindSoundAlias, 0, kFloat_0_CameraWork,
                                         kFloat_100_GolfCamera, kFloat_1_CameraWork);
                    SoundManager_SetNamedVolumeScale(g_soundManager, cameraWorkStrings.ballWindSoundAlias,
                                                     (kFloat_100_GolfCamera - cameraBallDistance) /
                                                         kFloat_100_GolfCamera);
                } else if (ballProximitySoundState == 1) {
                    Sound_StopByAlias(g_soundManager, cameraWorkStrings.ballWindSoundAlias);
                    ballProximitySoundState = 3;
                }
            } else {
                Sound_StopByAlias(g_soundManager, cameraWorkStrings.ballWindSoundAlias);
                ballProximitySoundState = 2;
            }
        }
    }
}

namespace ns2 {
extern "C" {} // extern "C"
} // namespace ns2

typedef Vec3f Vec3;
extern "C" {
extern const float kFloat_1_CameraWork;
extern const float kFloat_3_CameraWork;
extern const float kFloat_40_cameraWork;
extern const float kFloat_10_cameraWork;
extern float kCameraSecondEventTrackingBoundaryFraction;
extern void __ml__FfRC7WVector(void*, const void*, float);
extern void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern float Magnitude__7WVectorCFv(const void*);
extern void __as__7WVectorFRC7WVector(void*, const void*);
extern char* GolfBall__Fv();
}

// Evaluate a camera spline and update its ball-following target and pending action.
void CCameraWork::UpdateBezierPositionAndBallTracking(float unused, float frameFraction) {
    float u3;
    float u2;
    float u;
    float t2;
    float t;
    t = (elapsedTime + frameFraction - motionStartFrame) / (trajectoryLastFrame - motionStartFrame);
    t2 = t * t;
    u = kFloat_1_CameraWork - t;
    float uValue = u;
    u2 = uValue * uValue;
    u3 = u2 * u;
    Vec3 h;
    Vec3 a;
    Vec3 b;
    Vec3 c;
    Vec3 d;
    Vec3 e;
    Vec3 f;
    Vec3 g;
    __ml__FfRC7WVector(&a, &curveControlPoints[9], t * t2);
    float product = kFloat_3_CameraWork * u;
    __ml__FfRC7WVector(&b, &curveControlPoints[6], (product)*t2);
    float product2 = kFloat_3_CameraWork * u2;
    __ml__FfRC7WVector(&c, &curveControlPoints[3], (product2)*t);
    __ml__FfRC7WVector(&d, &curveControlPoints[0], u3);
    __pl__FRC7WVectorRC7WVector(&e, &d, &c);
    __pl__FRC7WVectorRC7WVector(&f, &e, &b);
    __pl__FRC7WVectorRC7WVector(&g, &f, &a);
    __as__7WVectorFRC7WVector(position, &g);

    if (actionFlags & kCameraCutAtEvent) {
        if (elapsedTime < (float)cutoffFrame - kFloat_40_cameraWork)
            __as__7WVectorFRC7WVector(target, GolfBall__Fv());
        else
            pendingAction = kCameraCutoffBisectorView;
    } else {
        // 0x800DE3A4: distant camera targets keep tracking the ball.
        do {
            if (elapsedTime < kCameraSecondEventTrackingBoundaryFraction * secondEventFrame) {
                __mi__FRC7WVectorRC7WVector(&h, position, target);
                if (Magnitude__7WVectorCFv(&h) > kFloat_10_cameraWork) {
                    __as__7WVectorFRC7WVector(target, GolfBall__Fv());
                    break;
                }
            }
            pendingAction = kCameraAction2;
        } while (0);
    }
}

namespace ns2 {
extern "C" {} // extern "C"
} // namespace ns2

// Advance scripted or trajectory camera motion, apply shake and ground clearance, then update the view.
// deltaTime advances animationTime; frameBlend is passed to the motion callback as its
// fractional frame coordinate. The integer trajectory frame comes from ball event slot 0.
void CCameraWork::UpdateMotionAndTarget(float deltaTime, float frameBlend) {
    using namespace ns2;
    CameraTargetState* self = (CameraTargetState*)this;
    char* cam = (char*)ns2::GolfBall__Fv();
    int current = ((CGolfBall*)cam)->m_eventPreDataIndex[0];
    float step = (float)current;
    self->frameIndex = current;
    self->animationTime += deltaTime;
    self->elapsedTime = step;

    // Phase 1: sample group animation or run a trajectory callback/pending action.
    if (self->scriptedAnimationEnabled) {
        if (self->playingAnimationGroup == 0)
            return; // 0x800DD7B0
        {
            int info[4];
            ((CVCGPlayer*)(self->playingAnimationGroup))->GetPlaybackState(info);
            if (info[2] == 1) {
                if (self->animationGroupIndex < self->groupValueCount) {
                    int n = self->animationGroupIndex + 1;
                    self->animationGroupIndex = n;
                    ((CVCGPlayer*)(self->playingAnimationGroup))->StartGroupPlayback(n, 0);
                } else {
                    self->playingAnimationGroup = 0;
                }
            }
        }
        if (self->playingAnimationGroup) {
            // Camera 3x4 world matrix built from the motion sample: three axes then translation.
            Vec3 matrix[4];
            struct {
                Vec3 motionPosition, motionTarget, motionUp;
                float fovRadians;
            } transform;
            Vec3 positionCopy, targetCopy, upCopy;
            Vec3 v90;
            Vec3 rawForward, rightCross, upCross;
            Vec3 rollRotation[4]; // 3x4 matrix written by ns2::Matrix_SetAxisRotation (0x30 bytes)
            Vec3 v8;
            // Phase 1a: sample the playing animation (position, target, up, field of view).
            ((CVCGPlayer*)(self->animationGroup))->SampleCurrentMotion(&transform);
            ((CVCGPlayer*)(self->animationGroup))->AdvancePlayback();
            ns2::Vec3_CopySourceToDestination(&transform.motionPosition, &positionCopy);
            ns2::Vec3_CopySourceToDestination(&transform.motionTarget, &targetCopy);
            ns2::Vec3_CopySourceToDestination(&transform.motionUp, &upCopy);
            ns2::Matrix_SetIdentityAffine(&matrix[0]);
            ns2::__mi__FRC7WVectorRC7WVector(&rawForward, &targetCopy, &positionCopy);
            ns2::__as__7WVectorFRC7WVector(&matrix[2], &rawForward);
            ns2::Normalize__7WVectorFv(&matrix[2]);
            ns2::__as__7WVectorFRC7WVector(&matrix[1], &upCopy);
            ns2::Normalize__7WVectorFv(&matrix[1]);
            ns2::Vector_CrossProduct(&rightCross, &matrix[1], &matrix[2]);
            ns2::__as__7WVectorFRC7WVector(&matrix[0], &rightCross);
            ns2::Normalize__7WVectorFv(&matrix[0]);
            ns2::Vector_CrossProduct(&upCross, &matrix[2], &matrix[0]);
            ns2::__as__7WVectorFRC7WVector(&matrix[1], &upCross);
            ns2::Normalize__7WVectorFv(&matrix[1]);
            ns2::__as__7WVectorFRC7WVector(&matrix[3], &positionCopy);
            // Motion supplies position, target, up, then field of view in radians.
            self->fov = transform.fovRadians;

            {
                void* obj = ns2::g_taskManager ? ns2::Object_GetChild(ns2::g_taskManager) : 0;
                TargetCmd cmd;
                cmd.a = 0;
                cmd.b = 3;
                cmd.p = &v8;
                cmd.r[0] = 0;
                cmd.r[1] = 0;
                cmd.r[2] = 0;
                cmd.r[3] = 0;
                cmd.r[4] = 0;
                TargetSink* sink =
                    (TargetSink*)ns2::TaskActorTable_FindByName(obj, ns2::cameraWorkStrings.Player, 3, 0);
                if (sink)
                    sink->send(&cmd);
            }
            {
                void* obj = ns2::g_taskManager ? ns2::Object_GetChild(ns2::g_taskManager) : 0;
                TargetCmd cmd;
                cmd.a = 0;
                cmd.b = 4;
                cmd.p = &v90;
                cmd.r[0] = 0;
                cmd.r[1] = 0;
                cmd.r[2] = 0;
                cmd.r[3] = 0;
                cmd.r[4] = 0;
                TargetSink* sink =
                    (TargetSink*)ns2::TaskActorTable_FindByName(obj, ns2::cameraWorkStrings.Player, 4, 0);
                if (sink)
                    sink->send(&cmd);
            }

            // Phase 1b: query the Player actor (commands 3 and 4); a nonzero v90 means the matrix is rolled
            // by (v8.x - pi) about axis 1 before it becomes the camera transform.
            if (!CameraTargetIsNearZero(v90.x) || !CameraTargetIsNearZero(v90.y) || !CameraTargetIsNearZero(v90.z)) {
                ns2::Matrix_SetAxisRotation(rollRotation, v8.x - ns2::kFloat_3_1415927_CameraWork, 1);
                ns2::Matrix_ConcatAffineAssign(&matrix[0], rollRotation);
            }

            // Script transform translations are pivot-relative: retain both poses in world space.
            ns2::Matrix_CopyAffine((Vec3*)&self->previousPose, &matrix[0]);
            ns2::__apl__7WVectorFRC7WVector((Vec3*)self->previousPose.position, &self->pivotPosition);
            ns2::Matrix_CopyAffine((Vec3*)&self->savedPose, &matrix[0]);
            ns2::__apl__7WVectorFRC7WVector((Vec3*)self->savedPose.position, &self->pivotPosition);
            self->savedFov = transform.fovRadians;
        }
    } else {
        self->playingAnimationGroup = 0;
        if (self->action < 0xca) {
            int enable;
            if (step >= self->trajectoryLastFrame) {
                enable = 1;
            } else {
                char* c2 = (char*)ns2::GolfBall__Fv();
                // m_holeInCrypt is the shared obfuscated hole-in value (value = ~(encoded ^ mask)).
                struct HoleInValueWords {
                    int encoded;
                    unsigned char byteCopy[4];
                    int mask;
                };
                HoleInValueWords* holeIn = (HoleInValueWords*)((CGolfBall*)c2)->m_holeInCrypt;
                int same = ~(holeIn->encoded ^ holeIn->mask);
                if (same == 1) {
                    unsigned char* game = (*(unsigned char**)&AsCSharedDoc(ns2::g_sharedDoc)->golfDoc);
                    unsigned char* tbl =
                        (*(unsigned char**)&AsCGolfDoc(game)->holeRecords) + (AsCGolfDoc(game)->currentHoleIndex << 5);
                    Vec3 v54;
                    *(int*)&v54.x = *(int*)(tbl + 0xc);
                    *(int*)&v54.y = *(int*)(tbl + 0x10);
                    *(int*)&v54.z = *(int*)(tbl + 0x14);
                    Vec3 v48, v3c;
                    ns2::__mi__FRC7WVectorRC7WVector(&v48, (Vec3*)ns2::GolfBall__Fv(), &v54);
                    ns2::__as__7WVectorFRC7WVector(&v3c, &v48);
                    float y = v3c.y;
                    v3c.y = ns2::kFloat_0_CameraWork;
                    if (!self->flag165) {
                        float s = ((CGolfBall*)ns2::GolfBall__Fv())->m_radius;
                        if (ns2::Magnitude__7WVectorCFv(&v3c) < 3.0f * s && y < ns2::kFloat_0_CameraWork)
                            self->flag165 = 1;
                    }
                    if (self->flag165)
                        enable = 1;
                    else
                        enable = 0;
                } else {
                    enable = 0;
                }
            }
            if (enable) {
                ns2::__as__7WVectorFRC7WVector(&self->initialBallPosition, (Vec3*)ns2::GolfBall__Fv());
                AsCCameraWork(self)->UpdateInputOffsets(deltaTime);
                return;
            }
        }

        if (self->pendingAction >= 0) {
            ((CCameraWork*)self)->ConfigureMode(self->pendingAction, 0);
            self->pendingAction = -1;
            AsCCameraWork(self)->BuildModeCurve(self->action);
        }

        if (self->elapsedTime < self->curveStartFrame) {
            self->previousFrameIndex = self->frameIndex;
        } else {
            int st = self->action;
            if (st == 3 || st == 4 || st == 0xb) {
                if (self->previousFrameIndex < (int)self->curveStartFrame) {
                    ns2::__as__7WVectorFRC7WVector(&self->position, &self->vec144);
                }
            }
            if (ns2::__ptmf_test(&self->callback) != 0) {
                (self->*self->callback)(deltaTime, frameBlend);
            }
        }

        // Phase 2: add sampled shake displacement for 0.8 animation-time units.
        if ((*(unsigned*)&AsCCameraWork(self)->actionFlags) & kCameraShakeActive) {
            float f3 = self->animationTime;
            float diff = f3 - self->shakeStartTime;
            if (diff < ns2::kFloat_0_8_CameraWork) {
                if (!((f3 - self->lastShakeTime) < ns2::kFloat_0_1_805DDF94)) {
                    int n = 1;
                    self->lastShakeTime = f3;
                    int m = 6 - (int)(6.0f * diff);
                    if (m != 0)
                        n = m;
                    float base = (float)n / ns2::kCameraShared20;
                    Vec3 v24, v30;
                    v24.x = (float)(ns2::rand() % n) / ns2::kFloat_10_cameraWork - base;
                    v24.y = (float)(ns2::rand() % n) / ns2::kFloat_10_cameraWork - base;
                    v24.z = (float)(ns2::rand() % n) / ns2::kFloat_10_cameraWork - base;
                    ns2::__ml__FRC7WVectorf(&v30, &v24, ns2::kFloat_0_1_805DDF94);
                    ns2::__apl__7WVectorFRC7WVector(&self->position, &v30);
                }
            } else {
                self->actionFlags &= ~kCameraShakeActive;
            }
        }
    }

    // Phase 3: raise the camera to ground clearance; scripted targets rise by the same amount.
    if (self->groundClearanceEnabled) {
        unsigned char* game = (*(unsigned char**)&AsCSharedDoc(ns2::g_sharedDoc)->golfDoc);
        int vx = (*(int*)&AsCCameraWork(self)->position[0]);
        int vy = (*(int*)&AsCCameraWork(self)->position[1]);
        int vz = (*(int*)&AsCCameraWork(self)->position[2]);
        Vec3 a0, a1;
        *(int*)&a0.x = vx;
        *(int*)&a0.y = vy;
        *(int*)&a0.z = vz;
        *(int*)&a1.x = vx;
        *(int*)&a1.y = vy;
        *(int*)&a1.z = vz;
        float y = ((CQuadTree*)ns2::CQuadTree_GetInstance())
                      ->ResolveGroundHeight((Vec3f*)&a1, AsCGolfDoc(game)->cameraGroundClearance);
        float selfy = self->position.y;
        if (!(selfy < y))
            y = selfy;
        if (self->scriptedAnimationEnabled) {
            self->targetY += y - self->position.y;
        }
        if (self->position.y < y)
            self->position.y = y;
    }

    // 0x800DD7B0: skip ground adjustment when the camera is close enough.
    do {
        if (!self->forceGroundEffect) {
            Vec3 v60;
            ns2::__mi__FRC7WVectorRC7WVector(&v60, &self->position, (Vec3*)ns2::GolfBall__Fv());
            if (!(ns2::WVectorLen__FRC7WVector(&v60) > ns2::kCameraGroundEffectMinimumBallDistance))
                break;
        }
        if (self->frameIndex > 0) {
            void* o = ns2::PreData__9CGolfBallFi(ns2::GolfBall__Fv(), self->frameIndex);
            if (((CameraTrajectorySample*)o)->groundEffectState == 3 && self->action != 0x19) {
                void* q = ns2::InstanceTable_Get();
                if (*(unsigned char*)((unsigned char*)(*(void**)((char*)q + 4)) + 0xa) != 3) {
                    float f4 = self->fov - ns2::kFloat_0_17453292 * deltaTime;
                    if (!(ns2::kCameraGroundEffectMinimumFovRadians < f4))
                        f4 = ns2::kCameraGroundEffectMinimumFovRadians;
                    float f3 = f4 / ns2::kFloat_0_95993108;
                    float f2 = ns2::kFloat_1_CameraWork - f3;
                    self->fov = f4;
                    self->targetY = self->targetY - ns2::kFloat_5_CameraWork * f2;
                }
            }
        }

    } while (0);
    // Phase 4: save the ball position for the next update and publish manual-offset view state.
    ns2::__as__7WVectorFRC7WVector(&self->initialBallPosition, (Vec3*)ns2::GolfBall__Fv());
    UpdateInputOffsets(deltaTime);
    return;
}

namespace ns2 {
extern "C" {

// Test the camera zero tolerance before restoring or applying input offsets.
static inline bool CameraInputIsNearZero(const float& v) {
    return ((*(const unsigned*)&v & 0x80000000u) ? -v : v) < 0.00001f;
}

} // extern "C"
} // namespace ns2

struct Mat34 {
    float m[12];
};
struct Pair {
    float yaw, pitch;
};
// Test the camera zero tolerance before restoring or applying input offsets.
static inline bool CameraInputIsNearZero(const float& v) {
    return ((*(const unsigned*)&v & 0x80000000u) ? -v : v) < 0.00001f;
}

extern "C" {
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_CameraWork[];
extern void* g_view;
extern char s_cameraTransform[];
extern CSharedDoc* g_sharedDoc;
extern float kFloat_0_2617994, kFloat_2_CameraWork, kFloat_0_5235988_805DDFCC, kCameraManualMaximumFovRadians;
extern float kCameraMinimumYawRadians, kCameraMinPitchRadians;
extern const float kFloat_1_0471976_cameraWork;
extern const float kFloat_0_17453292;
extern const float kFloat_0_95993108;
extern const float kFloat_0_CameraWork;
void* Matrix_TransformPoint(void*, const void*, const void*);
void Matrix_BuildCameraTransform(void*, const void*, const void*);
void Matrix_CopyAffine(void*, const void*);
void InputManager_Init(void*);
void InputManager_Dtor();
int Input_IsActionButtonDown(void*, int, int);
int SharedDoc_IsPlayerFlag0Set(unsigned char);
void Camera_SetFov(void*, float);
void Vec2_Zero(void*);
float Math_CosApprox(float);
float Math_SinApprox(float);
}

// Apply camera zoom/yaw/pitch input, spring offsets back to zero, and publish the view transform.
// Tuning / unit map: definitions in the data section, except the shared
// kFloat_0_95993108 in phase1_shared_data.cpp and the explicit epsilon literal.
// dt is scaled gameplay seconds from CGolfRule::OnProcess (golf_rule_round_flow_and_shot_sim.cpp), via
// UpdateMotionAndTarget or directly from PrepareNextShot. The clock/task chain
// is CMainFrame::UpdateVirtualHooks -> CProjectG_Update(timeScale) -> task actor
// processing (main_frame_letter_and_mouse_cursor.cpp/project_g_app_and_devices.cpp/task_manager_and_actors.cpp). Some setup callers pass zero dt.
// Rates below are radians/s, not radians/draw call.
// inputRotationOffset[1] pitch: actions0/1 +/-kFloat_0_2617994=.2617994 (15deg/s),
// return rate kFloat_2_CameraWork*rate = .5235988 (30deg/s).
// inputRotationOffset[0] yaw: actions2/3 +/-kFloat_0_5235988_805DDFCC=.5235988
// (30deg/s), return rate 2*rate=1.0471976 (60deg/s). These are constant-speed
// returns with a zero-crossing clamp, not a damped spring integration.
// inputFovOffset returns at kFloat_1_0471976_cameraWork=1.0471976 (60deg/s)
// only when BOTH pitch actions are unheld; this function has no held-key FOV
// increment. Its other writers must be considered when adding zoom input.
// Shared role warning: .5235988 is ALSO the positive pitch clamp; 1.0471976 is
// ALSO the positive yaw clamp. Their negative limits are kCameraMinPitchRadians
// and kCameraMinimumYawRadians. FOV clamps to [.17453292,pi/2] radians with
// default .95993108 (55deg) for the uncaptured random view. Epsilon literal1e-5
// radians controls near-zero tests. kFloat_2_CameraWork is also used in curves.
// Larger rates increase sensitivity/return speed; larger clamp magnitudes allow
// wider excursions. Editing a shared rate also edits its limit/other modes:
// split storage in a future mod when those roles should differ. Safe ranges
// beyond the current explicit clamps are not established. Keep float order.
// Restricted input is gameStateFlags==0x40 AND current player flag0 clear;
// inputMoved=0 clears all offsets. Saved/random/scripted branches share clamps
// but choose different base transforms. Calls publish view FOV/matrix, with no
// retained input pointers or allocation and no error result.
void CCameraWork::UpdateInputOffsets(float dt) {
    // Phase 1: sample logical actions 0/1 (pitch +/-), 2/3 (yaw +/-), and spring back
    // unheld offsets without overshooting zero. Action IDs are not physical button masks.
    Mat34 matF8, matC8, mat98, mat68;
    Vec3 vec5C, vec50, vec44, target, vec2C, vec20;
    Pair angA, angB, angC;
    if (GetInputManagerSlot() == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, s_inputManagerDtorRegistration_CameraWork);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    void* input = GetInputManagerSlot();
    int restricted = 0;
    unsigned char* game = (unsigned char*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if ((*(unsigned int*)&AsCGolfDoc(game)->gameStateFlags) == 0x40 &&
        SharedDoc_IsPlayerFlag0Set(AsCGolfDoc(game)->currentPlayer) == 0)
        restricted = 1;
    if ((Input_IsActionButtonDown(input, 0, -1) || Input_IsActionButtonDown(input, 1, -1) ||
         Input_IsActionButtonDown(input, 2, -1) || Input_IsActionButtonDown(input, 3, -1)) &&
        restricted == 0)
        inputMoved = 1;

    if (Input_IsActionButtonDown(input, 0, -1) == 0) {
        if (Input_IsActionButtonDown(input, 1, -1) == 0) {
            if (!CameraInputIsNearZero(inputFovOffset)) {
                float v = inputFovOffset;
                if (v > kFloat_0_CameraWork) {
                    v = v - kFloat_1_0471976_cameraWork * dt;
                    inputFovOffset = v;
                    if (v < kFloat_0_CameraWork)
                        inputFovOffset = kFloat_0_CameraWork;
                } else {
                    v = v + kFloat_1_0471976_cameraWork * dt;
                    inputFovOffset = v;
                    if (v > kFloat_0_CameraWork)
                        inputFovOffset = kFloat_0_CameraWork;
                }
            }
        }
    }

    float rateZ = kFloat_0_2617994;
    if (Input_IsActionButtonDown(input, 0, -1) && restricted == 0)
        inputRotationOffset[1] += rateZ * dt;
    if (Input_IsActionButtonDown(input, 1, -1) && restricted == 0)
        inputRotationOffset[1] -= rateZ * dt;
    if (Input_IsActionButtonDown(input, 0, -1) == 0 && Input_IsActionButtonDown(input, 1, -1) == 0) {
        if (!CameraInputIsNearZero(inputRotationOffset[1])) {
            float v = inputRotationOffset[1];
            if (v > kFloat_0_CameraWork) {
                float stepZ = kFloat_2_CameraWork * rateZ;
                v = v - stepZ * dt;
                inputRotationOffset[1] = v;
                if (v < kFloat_0_CameraWork)
                    inputRotationOffset[1] = kFloat_0_CameraWork;
            } else {
                float stepZ2 = kFloat_2_CameraWork * rateZ;
                v = v + stepZ2 * dt;
                inputRotationOffset[1] = v;
                if (v > kFloat_0_CameraWork)
                    inputRotationOffset[1] = kFloat_0_CameraWork;
            }
        }
    }

    float rateY = kFloat_0_5235988_805DDFCC;
    if (Input_IsActionButtonDown(input, 2, -1) && restricted == 0)
        inputRotationOffset[0] += rateY * dt;
    if (Input_IsActionButtonDown(input, 3, -1) && restricted == 0)
        inputRotationOffset[0] -= rateY * dt;
    if (Input_IsActionButtonDown(input, 2, -1) == 0 && Input_IsActionButtonDown(input, 3, -1) == 0) {
        if (!CameraInputIsNearZero(inputRotationOffset[0])) {
            float v = inputRotationOffset[0];
            if (v > kFloat_0_CameraWork) {
                float stepY = kFloat_2_CameraWork * rateY;
                v = v - stepY * dt;
                inputRotationOffset[0] = v;
                if (v < kFloat_0_CameraWork)
                    inputRotationOffset[0] = kFloat_0_CameraWork;
            } else {
                float stepY2 = kFloat_2_CameraWork * rateY;
                v = v + stepY2 * dt;
                inputRotationOffset[0] = v;
                if (v > kFloat_0_CameraWork)
                    inputRotationOffset[0] = kFloat_0_CameraWork;
            }
        }
    }

    if (inputMoved == 0) {
        inputFovOffset = kFloat_0_CameraWork;
        Vec2_Zero((Vec3*)((unsigned char*)&inputRotationOffset[0]));
    }

    // Phase 2: choose saved/random/scripted base transform; clamp FOV and angular offsets
    // before rebuilding the world camera. In each branch a local direction becomes a
    // world-space target through that branch's camera transform.
    if (randomModeRequested != 0) {
        if (useSavedCameraTransform != 0) {
            float fov = savedFov + inputFovOffset;
            float clampedFov = fov < kFloat_0_17453292                ? kFloat_0_17453292
                               : fov > kCameraManualMaximumFovRadians ? kCameraManualMaximumFovRadians
                                                                      : fov;
            Camera_SetFov(g_view, clampedFov);
            if (!(CameraInputIsNearZero(inputRotationOffset[0]) && CameraInputIsNearZero(inputRotationOffset[1]))) {
                float yaw = inputRotationOffset[0];
                float clampedYaw = yaw < kCameraMinimumYawRadians      ? kCameraMinimumYawRadians
                                   : yaw > kFloat_1_0471976_cameraWork ? kFloat_1_0471976_cameraWork
                                                                       : yaw;
                angA.yaw = clampedYaw;
                float pitch = inputRotationOffset[1];
                float clampedPitch = pitch < kCameraMinPitchRadians      ? kCameraMinPitchRadians
                                     : pitch > kFloat_0_5235988_805DDFCC ? kFloat_0_5235988_805DDFCC
                                                                         : pitch;
                angA.pitch = clampedPitch;
                float cosYaw = Math_CosApprox(angA.yaw);
                float sinPitch = Math_SinApprox(angA.pitch);
                vec50.x = Math_SinApprox(angA.yaw);
                vec50.y = sinPitch;
                vec50.z = cosYaw;
                Matrix_TransformPoint(&vec5C, &vec50, (Vec3*)((unsigned char*)&transform[0]));
                Matrix_BuildCameraTransform(&matF8, (unsigned char*)&savedCameraPosition[0], &vec5C);
                Matrix_CopyAffine((Vec3*)s_cameraTransform, (Vec3*)&matF8);
            } else {
                Matrix_CopyAffine((Vec3*)s_cameraTransform, (Vec3*)((unsigned char*)&savedCameraRight[0]));
            }
        } else {
            float fov = kFloat_0_95993108 + inputFovOffset;
            float clampedFov = fov < kFloat_0_17453292                ? kFloat_0_17453292
                               : fov > kCameraManualMaximumFovRadians ? kCameraManualMaximumFovRadians
                                                                      : fov;
            Camera_SetFov(g_view, clampedFov);
            if (!(CameraInputIsNearZero(inputRotationOffset[0]) && CameraInputIsNearZero(inputRotationOffset[1]))) {
                float yaw = inputRotationOffset[0];
                float clampedYaw = yaw < kCameraMinimumYawRadians      ? kCameraMinimumYawRadians
                                   : yaw > kFloat_1_0471976_cameraWork ? kFloat_1_0471976_cameraWork
                                                                       : yaw;
                angB.yaw = clampedYaw;
                float pitch = inputRotationOffset[1];
                float clampedPitch = pitch < kCameraMinPitchRadians      ? kCameraMinPitchRadians
                                     : pitch > kFloat_0_5235988_805DDFCC ? kFloat_0_5235988_805DDFCC
                                                                         : pitch;
                angB.pitch = clampedPitch;
                float cosYaw = Math_CosApprox(angB.yaw);
                float sinPitch = Math_SinApprox(angB.pitch);
                target.x = Math_SinApprox(angB.yaw);
                target.y = sinPitch;
                target.z = cosYaw;
                Matrix_TransformPoint(&vec44, &target, (Vec3*)((unsigned char*)&transform[0]));
                Matrix_BuildCameraTransform(&matC8, (unsigned char*)&previousTransform[9], &vec44);
                Matrix_CopyAffine((Vec3*)s_cameraTransform, (Vec3*)&matC8);
            } else {
                Matrix_CopyAffine((Vec3*)s_cameraTransform, (Vec3*)((unsigned char*)&previousTransform[0]));
            }
        }
    } else {
        float fov = this->fov + inputFovOffset;
        float clampedFov = fov < kFloat_0_17453292                ? kFloat_0_17453292
                           : fov > kCameraManualMaximumFovRadians ? kCameraManualMaximumFovRadians
                                                                  : fov;
        Camera_SetFov(g_view, clampedFov);
        if (playingAnimationGroup == 0) {
            Matrix_BuildCameraTransform(&mat98, (unsigned char*)&position[0], (unsigned char*)&this->target[0]);
            Matrix_CopyAffine((Vec3*)((unsigned char*)&previousTransform[0]), (Vec3*)&mat98);
        }
        if (!(CameraInputIsNearZero(inputRotationOffset[0]) && CameraInputIsNearZero(inputRotationOffset[1]))) {
            float yaw = inputRotationOffset[0];
            float clampedYaw = yaw < kCameraMinimumYawRadians      ? kCameraMinimumYawRadians
                               : yaw > kFloat_1_0471976_cameraWork ? kFloat_1_0471976_cameraWork
                                                                   : yaw;
            angC.yaw = clampedYaw;
            float pitch = inputRotationOffset[1];
            float clampedPitch = pitch < kCameraMinPitchRadians      ? kCameraMinPitchRadians
                                 : pitch > kFloat_0_5235988_805DDFCC ? kFloat_0_5235988_805DDFCC
                                                                     : pitch;
            angC.pitch = clampedPitch;
            float cosYaw = Math_CosApprox(angC.yaw);
            float sinPitch = Math_SinApprox(angC.pitch);
            vec20.x = Math_SinApprox(angC.yaw);
            vec20.y = sinPitch;
            vec20.z = cosYaw;
            Matrix_TransformPoint(&vec2C, &vec20, (Vec3*)((unsigned char*)&previousTransform[0]));
            Matrix_BuildCameraTransform(&mat68, (unsigned char*)&previousTransform[9], &vec2C);
            Matrix_CopyAffine((Vec3*)s_cameraTransform, (Vec3*)&mat68);
        } else {
            Matrix_CopyAffine((Vec3*)s_cameraTransform, (Vec3*)((unsigned char*)&previousTransform[0]));
        }
        if (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0] > 0)
            UpdateBallProximitySound();
    }

    // Phase 3: reset the stored base FOV for the next update after publishing this frame.
    if (resetFovPending != 0) {
        float v = kFloat_0_95993108;
        resetFovPending = 0;
        this->fov = v;
    }
}

namespace ns2 {
extern "C" {} // extern "C"

} // namespace ns2

// ---- merged from camera_work_shot_cameras.cpp ----
// Empty camera callback retained for its dispatch-table entry.
void CCameraWork::NoopA() {}

// Empty camera callback retained for its dispatch-table entry.
void CCameraWork::NoopB() {}

namespace ns3 {

extern "C" {

struct CameraAnimationState {
    char reservedBeforePosition[0x2C];
    char positionBytes[0x0C];
    char targetBytes[4];
    float targetY;
    char reservedBeforePendingAction[0x1C];
    int pendingAction;
    int actionFlags;
    float firstEventFrame;
    float secondEventFrame;
    float trajectoryLastFrame;
    float curveStartFrame;
    float elapsedTime;
    int cutoffFrame;
    char reservedBeforeFrameIndex[4];
    int frameIndex;
    char reservedBeforeCurveHeight[0xC8];
    float curveEndZ;
    float targetRiseHeight;
    float animationTime;
    float lastShakeTime;
    float shakeStartTime;
    float reservedScalar160;
    float reservedState164;
    float proximitySoundState;
    float heading;
    char reservedBeforeFov[0x98];
    float fov;
    int resetFovPending;
    char reservedBeforeRoll[0x20];
    float roll;
};

void* GolfBall__Fv();
void* PreData__9CGolfBallFi(void*, int);
void __as__7WVectorFRC7WVector(void*, void*);
void __apl__7WVectorFRC7WVector(void*, void*);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void __ml__FRC7WVectorf(void*, void*, float);
void __ml__FfRC7WVector(void*, void*, float);
void Vector_CrossProduct(void*, void*, void*);
void* Normalize__7WVectorFv(void*);
void* Matrix_TransformPoint(void*, void*, void*);
void Matrix_SetAxisRotation(void*, float, int);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern const float kFloat_0_CameraWork;
extern const float kFloat_10_cameraWork;
extern const float kFloat_5_CameraWork;
extern const float kFloat_35_Camera;
extern const float kFloat_0_7_CameraWork;
extern const float kFloat_200_CameraWork;
extern const float kDegreesToRadians;
extern const float kFloat_0_95993108;
extern const float kFloat_1_CameraWork;
extern float kFloat_0_2617994;
extern float kFloat_2_CameraWork;
extern const float kFloat_3_CameraWork;
extern const float kFloat_40_cameraWork;
extern const float kFloat_15_CameraWork;
extern const float kFloat_250_CameraWork;
extern const float kFloat_neg30_CameraWork;
extern const float kFloat_60_CameraWork;
extern const float kFloat_0_5_CameraWork;
extern const float kFloat_neg1_CameraWork;
extern const float kFloat_50_CameraWork;
extern double kSignedIntToDoubleBias_CameraWork;
extern const float kFloat_neg0_6981317_BezierPathCamera;
extern float kFloat_0_2617994;
extern const float kFloat_neg1_3089969_CameraWork;
extern const float kFloat_1_0471976_cameraWork;
extern const float kFloat_25_CameraWork;
extern const float kFloat_30_0;
extern const float kFloat_3_1415927_CameraWork;
extern double kSignedIntToDoubleBias_CameraWork;

typedef Vec3f Vec3;
void* InstanceTable_Get();
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern void* g_soundManager;
extern const CameraWorkStrings cameraWorkStrings;

} // extern "C"
} // namespace ns3

typedef Vec3f Vec3;
extern "C" char* GolfBall__Fv();
extern "C" void __as__7WVectorFRC7WVector(void*, const void*);
extern "C" {
extern const float kFloat_40_cameraWork;
extern const float kFloat_0_7_CameraWork;
extern const float kFloat_1_CameraWork;
extern const float kFloat_0_95993108;
extern const float kDegreesToRadians;
extern const float kFloat_25_CameraWork;
extern const float kFloat_0_5_CameraWork;
extern const float kFloat_3_1415927_CameraWork;
extern const float kFloat_15_CameraWork;
extern const float kFloat_0_CameraWork;
extern const float kFloat_200_CameraWork;
extern char* PreData__9CGolfBallFi(void*, int);
extern void __ml__FRC7WVectorf(void*, const void*, float);
extern void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern void Vector_CrossProduct(void*, const void*, const void*);
extern void Sound_PlayByName(void*, const char*, void*, int, const char*, int, float, float, float);
}
void* InstanceTable_Get();

// Track the shot with a pulsing FOV and one pass-by sound, then request the next camera action.
void CCameraWork::UpdatePassByTracking(float unused, float delta) {
    Vec3 current;
    Vec3 position;
    Vec3 direction;
    Vec3 result;
    Vec3 v1;
    Vec3 v0;
    Vec3 blend;
    Vec3 offset;
    Vec3 axis;
    int valid = 1;
    int frame;
    int index;

    if (actionFlags & kCameraCutAtEvent) {
        if (elapsedTime > (float)(int)cutoffFrame - kFloat_40_cameraWork)
            valid = 0;
    } else if (elapsedTime > kFloat_0_7_CameraWork * secondEventFrame) {
        valid = 0;
    }
    if (valid) {
        if (actionFlags & kCameraFollowCurrentBall) {
            __as__7WVectorFRC7WVector(target, GolfBall__Fv());
            index = 0;
        } else {
            index = -10;
            __ml__FRC7WVectorf(&v0, PreData__9CGolfBallFi(GolfBall__Fv(), frameIndex - 9), delta);
            __ml__FRC7WVectorf(&v1, PreData__9CGolfBallFi(GolfBall__Fv(), frameIndex - 10),
                               kFloat_1_CameraWork - delta);
            __pl__FRC7WVectorRC7WVector(&result, &v1, &v0);
            __as__7WVectorFRC7WVector(target, &result);
        }
        fov =
            kFloat_0_95993108 -
            kDegreesToRadians *
                (kFloat_25_CameraWork *
                 (float)sin(kFloat_0_5_CameraWork * (kFloat_3_1415927_CameraWork *
                                                     ((elapsedTime + delta - motionStartFrame) /
                                                      (kFloat_0_7_CameraWork * secondEventFrame - motionStartFrame)))));
        if (*(unsigned char*)((char*)*(void**)((char*)InstanceTable_Get() + 4) + 10) == 0 &&
            directionalSoundPlayed == 0 && firstEventFrame - elapsedTime < kFloat_15_CameraWork - (float)index) {
            frame = (int)elapsedTime;
            __as__7WVectorFRC7WVector(&current, PreData__9CGolfBallFi(GolfBall__Fv(), frame));
            frame = (int)elapsedTime;
            __mi__FRC7WVectorRC7WVector(&blend, PreData__9CGolfBallFi(GolfBall__Fv(), frame + 1), &current);
            __as__7WVectorFRC7WVector(&position, &blend);
            position.y = kFloat_0_CameraWork;
            __mi__FRC7WVectorRC7WVector(&offset, (char*)&this->position[0], &current);
            __as__7WVectorFRC7WVector(&direction, &offset);
            direction.y = kFloat_0_CameraWork;
            Vector_CrossProduct(&axis, &position, &direction);
            if (axis.y > kFloat_0_CameraWork) {
                Sound_PlayByName(g_soundManager, cameraWorkStrings.pass_RL, 0, 0, 0, 0, kFloat_0_5_CameraWork,
                                 kFloat_200_CameraWork, kFloat_1_CameraWork);
            } else {
                Sound_PlayByName(g_soundManager, cameraWorkStrings.pass_LR, 0, 0, 0, 0, kFloat_0_5_CameraWork,
                                 kFloat_200_CameraWork, kFloat_1_CameraWork);
            }
            directionalSoundPlayed = 1;
        }
    } else {
        fov = kFloat_0_95993108;
        if (actionFlags & kCameraCutAtEvent) {
            pendingAction = 0xE;
        } else {
            pendingAction = 0;
        }
    }
}

// Point the camera target directly at the current ball position.
void CCameraWork::SetTargetToBall() {
    __as__7WVectorFRC7WVector(target, GolfBall__Fv());
}

extern "C" {
extern void* Normalize__7WVectorFv(void*);
extern void __apl__7WVectorFRC7WVector(void*, void*);
extern const float kFloat_35_Camera;
extern const float kFloat_5_CameraWork;
extern const float kFloat_0_CameraWork;
}

// Place a camera beside the caddie head, with actor/model fallbacks, and look at the ball.
void CCameraWork::SetCaddieHeadView() {
    // Resolve the caddie model and actor reply; neither lookup transfers ownership.
    extern void* g_sceneManager;
    extern void* g_taskManager;
    void* NamedEffectList_FindByName(void*, const char*);
    void* Object_GetChild(void*);
    void* TaskActorTable_FindByName(void*, const char*);
    void* WPuppet_FindBoneByName(void*, const char*);
    struct Params {
        int a, type;
        void* out;
        int d, e, f, g, h;
    };
    struct Object {
        virtual void slot0() DECOMP_LOCAL_VIRTUAL;
        virtual void slot1() DECOMP_LOCAL_VIRTUAL;
        virtual void slot2() DECOMP_LOCAL_VIRTUAL;
        virtual void slot3() DECOMP_LOCAL_VIRTUAL;
        virtual void slot4() DECOMP_LOCAL_VIRTUAL;
        virtual void slot5() DECOMP_LOCAL_VIRTUAL;
        virtual void slot6() DECOMP_LOCAL_VIRTUAL;
        virtual void slot7() DECOMP_LOCAL_VIRTUAL;
        virtual void slot8() DECOMP_LOCAL_VIRTUAL;
        virtual void callback(void*) DECOMP_LOCAL_VIRTUAL;
    };
    Params params;
    Params params2;
    Vec3 transform;
    Vec3 source;
    Vec3 buf;
    Vec3 difference;
    Vec3 fallback;
    Vec3 scale;
    Vec3 offset;
    Vec3 scale2;
    Vec3 offset2;
    Vec3 scale3;
    Vec3 offset3;
    Vec3 sum;
    Vec3 c;
    Vec3 b;
    Vec3 up;
    Vec3 a;
    void* caddiePuppet = NamedEffectList_FindByName(g_sceneManager, cameraWorkStrings.caddieLabel);
    if (caddiePuppet == 0)
        return;
    void* state = g_taskManager != 0 ? Object_GetChild(g_taskManager) : 0;
    Object* object = (Object*)TaskActorTable_FindByName(state, cameraWorkStrings.Caddie);
    if (object != 0) {
        params.a = 0;
        params.type = 6;
        params.out = &source;
        params.d = 0;
        params.e = 0;
        params.f = 0;
        params.g = 0;
        params.h = 0;
        if (object)
            object->callback(&params);
        __mi__FRC7WVectorRC7WVector(&difference, &source, GolfBall__Fv());
        __as__7WVectorFRC7WVector(&transform, &difference);
    } else {
        __mi__FRC7WVectorRC7WVector(&fallback, &((WPuppet*)caddiePuppet)->worldMatrix[9], GolfBall__Fv());
        __as__7WVectorFRC7WVector(&transform, &fallback);
    }
    float y = kFloat_10_cameraWork + transform.y;
    transform.y = kFloat_0_CameraWork > y ? y : kFloat_0_CameraWork;
    Normalize__7WVectorFv(&transform);
    void* headBone = WPuppet_FindBoneByName(caddiePuppet, cameraWorkStrings.Head);
    if (headBone != 0) {
        offset.x = kFloat_0_CameraWork;
        offset.y = kFloat_5_CameraWork;
        offset.z = kFloat_0_CameraWork;
        __pl__FRC7WVectorRC7WVector(&scale, ((BonePositionView*)headBone)->worldPosition, &offset);
        __as__7WVectorFRC7WVector(position, &scale);
    } else if (object) {
        params2.a = 0;
        params2.type = 6;
        params2.out = &buf;
        params2.d = 0;
        params2.e = 0;
        params2.f = 0;
        params2.g = 0;
        params2.h = 0;
        if (object)
            object->callback(&params2);
        offset2.x = kFloat_0_CameraWork;
        offset2.y = kFloat_10_cameraWork;
        offset2.z = kFloat_0_CameraWork;
        __pl__FRC7WVectorRC7WVector(&scale2, &buf, &offset2);
        __as__7WVectorFRC7WVector(position, &scale2);
    } else {
        offset3.x = kFloat_0_CameraWork;
        offset3.y = kFloat_10_cameraWork;
        offset3.z = kFloat_0_CameraWork;
        __pl__FRC7WVectorRC7WVector(&scale3, &((WPuppet*)caddiePuppet)->worldMatrix[9], &offset3);
        __as__7WVectorFRC7WVector(position, &scale3);
    }
    up.x = kFloat_0_CameraWork;
    up.y = kFloat_1_CameraWork;
    up.z = kFloat_0_CameraWork;
    __ml__FfRC7WVector(&a, &transform, kFloat_35_Camera);
    Vector_CrossProduct(&b, &up, &transform);
    __ml__FfRC7WVector(&c, &b, kFloat_10_cameraWork);
    __pl__FRC7WVectorRC7WVector(&sum, &c, &a);
    __apl__7WVectorFRC7WVector(position, &sum);
    __as__7WVectorFRC7WVector(target, GolfBall__Fv());
}

namespace ns3 {
extern "C" {

// Track the ball until the second event, then raise the target over the remaining trajectory.
void CameraWork_UpdateBallTargetWithRisingOffset(CameraAnimationState* self, float unused, float delta) {
    if (self->elapsedTime < self->secondEventFrame) {
        __as__7WVectorFRC7WVector(&self->targetBytes, GolfBall__Fv());
    } else {
        char transform[12];
        float v[3];
        float t =
            (self->elapsedTime + delta - self->secondEventFrame) / (self->trajectoryLastFrame - self->secondEventFrame);
        v[0] = kFloat_0_CameraWork;
        v[2] = kFloat_0_CameraWork;
        v[1] = kFloat_5_CameraWork * t;
        void* a = GolfBall__Fv();
        __pl__FRC7WVectorRC7WVector(transform, a, v);
        __as__7WVectorFRC7WVector(&self->targetBytes, transform);
    }
}

} // extern "C"
} // namespace ns3

extern "C" {
extern const float kFloat_35_Camera;
extern float kFloat_2_CameraWork;
extern void Matrix_SetAxisRotation(void*, float, int);
}

extern "C" {
extern const float kFloat_0_CameraWork;
extern const float kFloat_10_cameraWork;
extern const float kFloat_5_CameraWork;
extern const float kFloat_35_Camera;
extern const float kFloat_0_7_CameraWork;
extern const float kFloat_200_CameraWork;
extern const float kDegreesToRadians;
extern const float kFloat_0_95993108;
extern const float kFloat_1_CameraWork;
extern float kFloat_0_2617994;
extern float kFloat_2_CameraWork;
extern const float kFloat_3_CameraWork;
extern const float kFloat_40_cameraWork;
extern const float kFloat_15_CameraWork;
extern const float kFloat_250_CameraWork;
extern const float kFloat_neg30_CameraWork;
extern const float kFloat_60_CameraWork;
extern const float kFloat_0_5_CameraWork;
extern const float kFloat_neg1_CameraWork;
extern const float kFloat_50_CameraWork;
extern double kSignedIntToDoubleBias_CameraWork;
extern const float kFloat_neg0_6981317_BezierPathCamera;
extern const float kFloat_neg1_3089969_CameraWork;
extern const float kFloat_1_0471976_cameraWork;
extern const float kFloat_25_CameraWork;
extern const float kFloat_30_0;
extern const float kFloat_3_1415927_CameraWork;
}

// Zoom toward the ball during the first event interval, then request a follow-up action.
void CCameraWork::UpdateInitialBallZoom(float unused, float delta) {
    if (elapsedTime < firstEventFrame) {
        float t = (elapsedTime + delta) / firstEventFrame;
        fov = kFloat_0_95993108 - kDegreesToRadians * (kFloat_35_Camera * t);
        __as__7WVectorFRC7WVector(target, GolfBall__Fv());
        target[1] += kFloat_2_CameraWork * (kFloat_1_CameraWork - t);
    } else {
        resetFovPending = 1;
        if (actionFlags & kCameraCutAtEvent) {
            pendingAction = 0xE;
        } else {
            pendingAction = 0xB;
        }
    }
}

namespace ns3 {
extern "C" {} // extern "C"
} // namespace ns3

// Follow a cubic camera path with a sinusoidal FOV and a target that rises after the second event.
void CCameraWork::UpdateBezierBallTracking(float unused, float delta) {
    Vec3 output;
    Vec3 movement;
    Vec3 curve;
    Vec3 p3;
    Vec3 p2;
    Vec3 p1;
    Vec3 p0;
    Vec3 sum1;
    Vec3 sum2;
    Vec3 sum3;
    float u3;
    float u2;
    float u;
    float t2;
    float t = (elapsedTime + delta - motionStartFrame) / (trajectoryLastFrame - motionStartFrame);
    u = kFloat_1_CameraWork - t;
    t2 = t * t;
    u2 = u * u;
    u3 = u2 * u;
    __ml__FfRC7WVector(&curve, (char*)&curveControlPoints[9], t * t2);
    float control2Weight = kFloat_3_CameraWork * u;
    float control2Amount = control2Weight * t2;
    __ml__FfRC7WVector(&p3, (char*)&curveControlPoints[6], control2Amount);
    float control1Weight = kFloat_3_CameraWork * u2;
    float control1Amount = control1Weight * t;
    __ml__FfRC7WVector(&p2, (char*)&curveControlPoints[3], control1Amount);
    __ml__FfRC7WVector(&p1, (char*)&curveControlPoints[0], u3);
    __pl__FRC7WVectorRC7WVector(&sum1, &p1, &p2);
    __pl__FRC7WVectorRC7WVector(&sum2, &sum1, &p3);
    __pl__FRC7WVectorRC7WVector(&sum3, &sum2, &curve);
    __as__7WVectorFRC7WVector(position, &sum3);

    fov = kFloat_0_95993108 -
          kDegreesToRadians * (kFloat_25_CameraWork * (float)sin((double)(kFloat_3_1415927_CameraWork * t)));
    if (elapsedTime < secondEventFrame) {
        __as__7WVectorFRC7WVector(target, GolfBall__Fv());
    } else if (elapsedTime < trajectoryLastFrame) {

        movement.y =
            targetRiseHeight * ((elapsedTime + delta - secondEventFrame) / (trajectoryLastFrame - secondEventFrame));
        movement.x = kFloat_0_CameraWork;
        movement.z = kFloat_0_CameraWork;
        __pl__FRC7WVectorRC7WVector(&output, GolfBall__Fv(), &movement);
        __as__7WVectorFRC7WVector(target, &output);
    }
}

namespace ns3 {
extern "C" {} // extern "C"
} // namespace ns3

// Follow a cubic camera path and rising ball target, cutting early when the event-cut flag is set.
void CCameraWork::UpdateBezierTrackingWithEventCut(float unused, float delta) {
    char weightedStart[12];
    __typeof__(motionStartFrame) curveStartValue = motionStartFrame;

    unsigned char curvePosition[12];
    __typeof__(curveStartValue) curveStartValueCopy = motionStartFrame;
    __typeof__(curveStartValueCopy) curveStartUnusedCopy = motionStartFrame;
    float curveStartForNumerator = motionStartFrame;
    float curveStartForDenominator = motionStartFrame;
    float t = ((elapsedTime + delta) - curveStartForNumerator) / (trajectoryLastFrame - curveStartForDenominator);
    __typeof__(t * t) tSquaredUnused = t * t;
    float u = kFloat_1_CameraWork - t;

    float u2 = (u * u);
    float tSquared = t * t;

    __typeof__(u2 * u) uCubed = u * u2;
    __typeof__(u) uCopy = u;
    __typeof__(u2) uSquaredCopy = u2;
    __typeof__(uCubed) startWeight = u * uSquaredCopy;

    float t2 = tSquared;
    char weightedEnd[12];
    __ml__FfRC7WVector(weightedEnd, (char*)&curveControlPoints[9], t * t2);
    __typeof__(u) uForControl2 = u;
    __typeof__(kFloat_3_CameraWork * uForControl2) control2Weight = kFloat_3_CameraWork * uForControl2;
    unsigned char weightedControl2[12];
    __ml__FfRC7WVector(weightedControl2, (char*)&curveControlPoints[6], control2Weight * t2);
    __typeof__(kFloat_3_CameraWork * u2) control1Weight = kFloat_3_CameraWork * u2;

    float control1WeightCopy = kFloat_3_CameraWork * u2;
    char weightedControl1[12];
    __ml__FfRC7WVector(weightedControl1, (char*)&curveControlPoints[3], control1WeightCopy * t);
    char curvePartial[12];
    __ml__FfRC7WVector(weightedStart, (char*)&curveControlPoints[0], (startWeight));
    unsigned char startAndControl1[12];
    __pl__FRC7WVectorRC7WVector(startAndControl1, weightedStart, weightedControl1);
    __pl__FRC7WVectorRC7WVector(curvePartial, startAndControl1, weightedControl2);
    __pl__FRC7WVectorRC7WVector(curvePosition, curvePartial, weightedEnd);
    __as__7WVectorFRC7WVector(position, curvePosition);

    fov = kFloat_0_95993108 -
          kDegreesToRadians * (kFloat_25_CameraWork * ((float)sin((kFloat_3_1415927_CameraWork * t))));
    if (actionFlags & kCameraCutAtEvent) {

        if (elapsedTime < ((float)(int)cutoffFrame - kFloat_30_0)) {
            __as__7WVectorFRC7WVector(target, GolfBall__Fv());
        } else {
            pendingAction = 0xE;
        }
    } else if (elapsedTime < secondEventFrame) {
        __as__7WVectorFRC7WVector(target, GolfBall__Fv());
    } else if (elapsedTime < trajectoryLastFrame) {
        unsigned char result[12];

        float v[3];
        v[1] = targetRiseHeight * ((elapsedTime + delta - secondEventFrame) / (trajectoryLastFrame - secondEventFrame));
        v[0] = kFloat_0_CameraWork;
        v[2] = kFloat_0_CameraWork;
        __pl__FRC7WVectorRC7WVector(result, GolfBall__Fv(), v);
        __as__7WVectorFRC7WVector(target, result);
    }
}

namespace ns3 {
extern "C" {

extern float kFloat_2_CameraWork;

} // extern "C"
} // namespace ns3

// Request the quarter-view action once the animation timer exceeds two units.
void CCameraWork::RequestQuarterViewAfterDelay() {
    if (animationTime > kFloat_2_CameraWork) {
        ConfigureMode(0x15, 1);
    }
}

namespace ns3 {
extern "C" {} // extern "C"
} // namespace ns3

// Track the ball until the second event, then move the target along a cubic Bezier curve.
void CCameraWork::UpdateBezierTarget(float unused, float delta) {
    if (elapsedTime < secondEventFrame) {
        __as__7WVectorFRC7WVector(target, GolfBall__Fv());
    } else if (elapsedTime < trajectoryLastFrame) {
        Vec3 result;
        Vec3 p3;
        Vec3 p2;
        Vec3 p1;
        Vec3 p0;
        Vec3 sum1;
        Vec3 sum2;
        Vec3 sum3;
        float u3;
        float u2;
        float u;
        float t2;
        float t = (elapsedTime + delta - secondEventFrame) / (trajectoryLastFrame - secondEventFrame);
        u = kFloat_1_CameraWork - t;
        t2 = t * t;
        u2 = u * u;
        u3 = u2 * u;
        __ml__FfRC7WVector(&result, (char*)&curveControlPoints[9], t * t2);
        float control2Weight = kFloat_3_CameraWork * u;
        float control2Amount = control2Weight * t2;
        __ml__FfRC7WVector(&p3, (char*)&curveControlPoints[6], control2Amount);
        float control1Weight = kFloat_3_CameraWork * u2;
        float control1Amount = control1Weight * t;
        __ml__FfRC7WVector(&p2, (char*)&curveControlPoints[3], control1Amount);
        __ml__FfRC7WVector(&p1, (char*)&curveControlPoints[0], u3);
        __pl__FRC7WVectorRC7WVector(&sum1, &p1, &p2);
        __pl__FRC7WVectorRC7WVector(&sum2, &sum1, &p3);
        __pl__FRC7WVectorRC7WVector(&sum3, &sum2, &result);
        __as__7WVectorFRC7WVector(target, &sum3);
    }
}

namespace ns3 {
extern "C" {} // extern "C"
} // namespace ns3

extern "C" {
extern const float kFloat_neg0_6981317_BezierPathCamera;
}

// Narrow the FOV while tracking the ball, then move the target onto its Bezier path.
void CCameraWork::UpdateBallToBezierTarget(float unused, float delta) {
    if (elapsedTime < secondEventFrame) {

        fov = kFloat_0_2617994 - (kFloat_neg0_6981317_BezierPathCamera *
                                  ((elapsedTime + delta - motionStartFrame) / (secondEventFrame - motionStartFrame)));
        __as__7WVectorFRC7WVector(target, GolfBall__Fv());
    } else if (elapsedTime < trajectoryLastFrame) {
        float u3;
        float u2;
        float u;
        float t2;
        float t = ((elapsedTime + delta) - secondEventFrame) / (trajectoryLastFrame - secondEventFrame);
        u = kFloat_1_CameraWork - t;
        float uValue2 = u;
        u2 = uValue2 * uValue2;
        t2 = t * t;
        unsigned char a[12], b[12], c[12], d[12];
        float u2Value = u2;
        u3 = u2Value * u;

        __ml__FfRC7WVector(a, (char*)&curveControlPoints[9], t * t2);
        __typeof__(u) uForControl2 = u;
        __typeof__(kFloat_3_CameraWork * uForControl2) control2Weight = kFloat_3_CameraWork * uForControl2;
        __ml__FfRC7WVector(b, (char*)&curveControlPoints[6], (control2Weight)*t2);
        __typeof__(u2) uSquaredCopy = u2;
        __typeof__(uSquaredCopy * kFloat_3_CameraWork) control1Weight = kFloat_3_CameraWork * uSquaredCopy;
        __ml__FfRC7WVector(c, (char*)&curveControlPoints[3], (control1Weight)*t);
        __ml__FfRC7WVector(d, (char*)&curveControlPoints[0], u3);
        char e[12];
        char f[12];
        char g[12];
        __pl__FRC7WVectorRC7WVector(e, d, c);
        __pl__FRC7WVectorRC7WVector(f, e, b);
        __pl__FRC7WVectorRC7WVector(g, f, a);
        __as__7WVectorFRC7WVector(target, g);
    }
}

namespace ns3 {
extern "C" {} // extern "C"
} // namespace ns3

// Move along a Bezier path while lifting the ball target before and after the second event.
void CCameraWork::UpdateBezierPositionAndRaisedBallTarget(float unused, float delta) {
    unsigned char g[12];

    float curveStartForDenominator = motionStartFrame;
    char d[12];
    float curveStartForNumerator = motionStartFrame;
    unsigned char a[12];
    float t = ((elapsedTime + delta) - curveStartForNumerator) / (trajectoryLastFrame - curveStartForDenominator);
    float tSquared = t * t;
    float tSquaredUnused = t * t;
    char b[12];
    float u = 1.0f - t;
    float u2 = u * u;

    __typeof__(u) uCopy = u;
    float t2 = tSquared;
    __typeof__(u2) uSquaredCopy = u2;
    __typeof__(uCopy * uSquaredCopy) startWeight = uSquaredCopy * uCopy;
    __typeof__(u) uForControl2 = u;
    __ml__FfRC7WVector(a, (char*)&curveControlPoints[9], (t2 * t));
    __typeof__(uForControl2 * 3.0f) control2Weight = 3.0f * uForControl2;
    __ml__FfRC7WVector(b, (char*)&curveControlPoints[6], control2Weight * t2);
    char c[12];

    __ml__FfRC7WVector(c, (char*)&curveControlPoints[3], (u2 * 3.0f) * t);
    __ml__FfRC7WVector(d, (char*)&curveControlPoints[0], (startWeight));

    unsigned char f[12];
    char e[12];
    __pl__FRC7WVectorRC7WVector(e, d, c);
    __pl__FRC7WVectorRC7WVector(f, e, b);
    __pl__FRC7WVectorRC7WVector(g, f, a);
    __as__7WVectorFRC7WVector(position, g);
    if (elapsedTime < secondEventFrame) {
        __as__7WVectorFRC7WVector(target, GolfBall__Fv());
        target[1] += 2.0f * (1.0f - t);
    } else if (elapsedTime < trajectoryLastFrame) {
        float v[3];
        char result[12];
        v[1] = targetRiseHeight * ((elapsedTime + delta - secondEventFrame) / (trajectoryLastFrame - secondEventFrame));
        v[2] = 0.0f;
        v[0] = 0.0f;

        __pl__FRC7WVectorRC7WVector(result, GolfBall__Fv(), v);
        __as__7WVectorFRC7WVector(target, result);
    }
}

namespace ns3 {
extern "C" {} // extern "C"
} // namespace ns3

// Move on a Bezier path and rotate the ball-facing direction by the current heading.
void CCameraWork::UpdateBezierOrbit(float delta, float frameOffset) {
    char rotation[0x30], combined[12], result[12],
        rotated[12]; // rotation is a 3x4 matrix (Matrix_SetAxisRotation writes 0x30)
    float t = (elapsedTime + frameOffset - motionStartFrame) / (trajectoryLastFrame - motionStartFrame);
    float t2 = t * t;
    float u = 1.0f - t;
    float u2 = u * u;
    float u3 = u2 * u;
    char a[12], b[12], c[12], d[12];
    __ml__FfRC7WVector(a, (char*)&curveControlPoints[9], t * t2);
    __ml__FfRC7WVector(b, (char*)&curveControlPoints[6], 3.0f * u * t2);
    __ml__FfRC7WVector(c, (char*)&curveControlPoints[3], 3.0f * u2 * t);
    __ml__FfRC7WVector(d, (char*)&curveControlPoints[0], u3);
    char e[12], f[12], g[12];
    __pl__FRC7WVectorRC7WVector(e, d, c);
    __pl__FRC7WVectorRC7WVector(f, e, b);
    __pl__FRC7WVectorRC7WVector(g, f, a);
    __as__7WVectorFRC7WVector(position, g);
    if (elapsedTime < secondEventFrame)
        heading -= 0.03490658476948738f * delta;
    Matrix_SetAxisRotation(rotation, heading, 0);
    void* source = GolfBall__Fv();
    __mi__FRC7WVectorRC7WVector(rotated, source, position);
    Matrix_TransformPoint(result, rotated, rotation);
    __pl__FRC7WVectorRC7WVector(combined, position, result);
    __as__7WVectorFRC7WVector(target, combined);
}

namespace ns3 {
extern "C" {} // extern "C"
} // namespace ns3

// Frame the ball from its shot-side direction, rotate the target, and finish after two animation units.
void CCameraWork::UpdateShotSideTargetRotation(float deltaTime) {
    unsigned char rot[0x38];
    struct Vec3i {
        int x, y, z;
    };
    Vec3i shotDirection;
    Vec3i sideDirectionCopy;
    Vec3i sideDirection;
    float up[3];
    char d[12];
    char c[12];
    char b[12];
    unsigned char a[12];
    unsigned char e[12];
    char g[12];
    unsigned char f[12];
    char pad[0x30];

    void* source = GolfBall__Fv();
    *(long long*)&shotDirection = *(long long*)((char*)&((CGolfBall*)source)->m_velocity);
    shotDirection.z = *(int*)&((CGolfBall*)source)->m_velocity.z;
    *(float*)&shotDirection.y = kFloat_0_CameraWork;
    Normalize__7WVectorFv(&shotDirection);
    up[0] = kFloat_0_CameraWork;
    up[1] = kFloat_1_CameraWork;
    up[2] = kFloat_0_CameraWork;
    Vector_CrossProduct(&sideDirection, up, &shotDirection);
    Vec3i* normalizedSide = (Vec3i*)Normalize__7WVectorFv(&sideDirection);
    *(long long*)&sideDirectionCopy = *(long long*)normalizedSide;
    sideDirectionCopy.z = normalizedSide->z;
    if (animationTime < kFloat_2_CameraWork) {
        __ml__FfRC7WVector(a, &sideDirectionCopy, kFloat_3_CameraWork);
        __ml__FfRC7WVector(b, &shotDirection,
                           kFloat_2_CameraWork * (kFloat_1_CameraWork - kFloat_3_CameraWork * animationTime));
        __pl__FRC7WVectorRC7WVector(c, GolfBall__Fv(), b);
        __pl__FRC7WVectorRC7WVector(d, c, a);
        __as__7WVectorFRC7WVector(position, d);
        if (animationTime < kFloat_0_5_CameraWork) {
            __mi__FRC7WVectorRC7WVector(e, position, &sideDirectionCopy);
            __as__7WVectorFRC7WVector(target, e);
        } else {
            if (heading > kFloat_neg1_3089969_CameraWork)
                heading -= kFloat_1_0471976_cameraWork * deltaTime;
            float angle = heading;
            Matrix_SetAxisRotation(rot, angle, 1);
            Matrix_TransformPoint(f, &sideDirectionCopy, rot);
            __mi__FRC7WVectorRC7WVector(g, position, f);
            __as__7WVectorFRC7WVector(target, g);
        }
    } else
        pendingAction = 4;
}

namespace ns3 {
extern "C" {

// Blend the initial and current ball positions and add a fixed target height.
void CameraWork_InterpolatePreviousAndCurrentBallTarget(CameraAnimationState* self, float unused, float delta) {
    char target[12];
    char position[12];
    char weightedInitialBall[12];
    char weightedCurrentBall[12];
    float a[3];
    float t = (self->elapsedTime + delta - self->curveStartFrame) / (self->trajectoryLastFrame - self->curveStartFrame);
    a[0] = kFloat_0_CameraWork;
    a[1] = kFloat_3_CameraWork;
    a[2] = kFloat_0_CameraWork;
    __ml__FfRC7WVector(weightedCurrentBall, GolfBall__Fv(), t);
    __ml__FfRC7WVector(weightedInitialBall, PreData__9CGolfBallFi(GolfBall__Fv(), 0), kFloat_1_CameraWork - t);
    __pl__FRC7WVectorRC7WVector(position, weightedInitialBall, weightedCurrentBall);
    __pl__FRC7WVectorRC7WVector(target, position, a);
    __as__7WVectorFRC7WVector(&self->targetBytes, target);
}

} // extern "C"
} // namespace ns3

// Track or raise the ball target and request a new action at the event cutoff.
void CCameraWork::UpdateBallTargetWithEventCut(float unused, float delta) {
    if (actionFlags & kCameraCutAtEvent) {
        if (elapsedTime < (float)cutoffFrame - kFloat_40_cameraWork) {
            __as__7WVectorFRC7WVector(target, GolfBall__Fv());
        } else {
            pendingAction = 0xE;
        }
    } else if (elapsedTime < secondEventFrame) {
        __as__7WVectorFRC7WVector(target, GolfBall__Fv());
    } else if (secondEventFrame > kFloat_250_CameraWork) {
        pendingAction = 4;
    } else {
        char result[12];
        float v[3];
        v[1] = 5.0f * ((elapsedTime + delta - secondEventFrame) / (1.0f + (trajectoryLastFrame - secondEventFrame)));
        v[0] = 0.0f;
        v[2] = 0.0f;
        __pl__FRC7WVectorRC7WVector(result, GolfBall__Fv(), v);
        __as__7WVectorFRC7WVector(target, result);
    }
}

namespace ns3 {
extern "C" {} // extern "C"
} // namespace ns3

// Roll and approach along the shot direction until the first trajectory event.
void CCameraWork::UpdateRollingShotApproach(float unused, float delta) {
    unsigned char shotDirection[12];
    unsigned char sideDirection[12];
    char targetScratch[12]; // not read; keeps the frame layout
    float up[3];

    char ballPosition[12];

    float t = ((elapsedTime + delta) - motionStartFrame) / (firstEventFrame - motionStartFrame);
    roll = kDegreesToRadians * (60.0f * t - 30.0f);
    void* base = GolfBall__Fv();

    *(int*)shotDirection = *(int*)&((CGolfBall*)base)->m_velocity.x;
    *(int*)((shotDirection + 4)) = *(int*)&((CGolfBall*)base)->m_velocity.y;
    *(int*)((shotDirection + 8)) = *(int*)&((CGolfBall*)base)->m_velocity.z;
    Normalize__7WVectorFv(shotDirection);
    up[0] = kFloat_0_CameraWork;
    up[1] = kFloat_1_CameraWork;
    up[2] = kFloat_0_CameraWork;
    Vector_CrossProduct(sideDirection, up, shotDirection);
    Normalize__7WVectorFv(sideDirection);
    unsigned char lateralOffset[12];
    __ml__FfRC7WVector(lateralOffset, sideDirection, kFloat_0_5_CameraWork * t + kFloat_neg1_CameraWork);
    __pl__FRC7WVectorRC7WVector(ballPosition, GolfBall__Fv(), lateralOffset);
    __as__7WVectorFRC7WVector(target, ballPosition);
    unsigned char cameraPosition[12];
    unsigned char rearOffset[12];
    __ml__FfRC7WVector(rearOffset, shotDirection, kFloat_50_CameraWork * (kFloat_1_CameraWork - t));
    __pl__FRC7WVectorRC7WVector(cameraPosition, target, rearOffset);
    __as__7WVectorFRC7WVector(position, cameraPosition);
    if (elapsedTime > firstEventFrame)
        pendingAction = 0;
}

namespace ns3 {
extern "C" {} // extern "C"

} // namespace ns3

// ---- merged from camera_work_shot_cameras.cpp ----
namespace ns4 {

extern "C" {

extern const float kFloat_0_95993108;
extern const float kFloat_45_CameraWork;
extern const float kFloat_0_61086524_CameraWork;
extern const float kFloat_1_0471976_cameraWork;
extern const float kFloat_0_033333335_CameraWork;
extern const float kFloat_2_0943952_CameraWork;
extern const float kFloat_0_75_CameraWork;
extern const float kFloat_0_93_GolfCamera;
extern const float kFloat_60_CameraWork;
extern const float kFloat_neg30_CameraWork;
extern const float kDegreesToRadians;
extern const float kFloat_50_CameraWork;
extern const float kFloat_0_9_CameraWork;
extern const float kFloat_0_1_805DDF94;
extern const float kFloat_0_5_CameraWork;
extern const float kCameraNearHoleViewMinimumFrames;
extern float kFloat_neg1_CameraWork;
extern const float kCameraLoweringSpeed;
extern const float kFloat_0_CameraWork;
extern const float kFloat_0_8_CameraWork;
extern const float kFloat_2_5_CameraWork;
extern void* g_view;
extern CSharedDoc* g_sharedDoc;
extern const float kFloat_neg2_CameraWork;
extern const float kFloat_1_CameraWork;
extern const float kFloat_0_2_CameraWork;
extern const float kFloat_0_5235988_805DDFCC;
extern const float kCameraOrbitDistanceDecay;
extern const float kFloat_2_CameraWork;
extern const float kFloat_3_CameraWork;
extern const float kFloat_1_2_CameraWork;
extern const float kCameraBehindBallOffset;
extern float s_cameraWorkInterpolationLimit;
extern const float kFloat_16_CameraWork;
extern const float kCameraShared20;
extern const float kFloat_0_17453292;
extern const float kFloat_neg3_CameraWork;
extern const float kFloat_neg0_7_CameraWork;
extern const float kFloat_neg0_3_CameraWork;
extern unsigned int zeroInitializationWords[];
extern char s_cameraTransform[];
extern const float kFloat_5_CameraWork;
extern const float kFloat_0_3_CameraWork;
extern const float kFloat_0_4_CameraWork;
extern const float kFloat_0_2617994;
extern float kFloat_neg1_CameraWork;
extern const float kCameraBallFollowRollDecay;
extern const float kFloat_6_CameraWork;
extern const float kFloat_10_cameraWork;
extern const double kSignedIntToDoubleBias_CameraWork;

void* GolfBall__Fv();
void* PreData__9CGolfBallFi(void*, int);
void* CQuadTree_GetInstance();
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
void __as__7WVectorFRC7WVector(void*, void*);
float WVectorLen__FRC7WVector(void*);
void* Normalize__7WVectorFv(void*);
void __amu__7WVectorFf(void*, float);
void* Matrix_TransformPoint(void*, void*, void*);
void Vector_CrossProduct(void*, void*, void*);
void* Matrix_BuildCameraTransform(void*, void*, void*);
void __apl__7WVectorFRC7WVector(void*, void*);
void __ml__FRC7WVectorf(void*, void*, float);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void Quat_Slerp(void*, void*, void*, float);
void Quat_ToMatrix3x3(void*, void*);
void __ml__FfRC7WVector(void*, ...);
float Magnitude__7WVectorCFv(void*);
void Matrix_SetAxisRotation(void*, float);
int WView_DefaultAcceptVisibility(void*, void*);
double sqrt(double);
float __ml__FRC7WVectorRC7WVector(void*, void*);

struct CameraInterpolationState {
    char reserved00[0x2C];
    float pos[3];
    float targetX;
    float targetY;
    char reserved40[0x14];
    int action;
    char reserved58[4];
    unsigned int pendingAction;
    char reserved60[0x0C];
    float trajectoryLastFrame;
    float curveStartFrame;
    float elapsedTime;
    char reserved78[8];
    unsigned int frameIndex;
    char reserved84[0x48];
    float valueCC[3];
    char reservedD8[0x7C];
    float animationTime;
    char reserved158[0x14];
    float heading;
    char reserved170[0x54];
    float startQuaternion[4];
    float endQuaternion[4];
    float startPosition[3];
    float endPosition[3];
    float horizontalBallDistance;
    float ballHeightOffset;
    float duration;
    float fov;
    char reserved20C[0x1C];
    unsigned int randomValue;
    float value22C;
    float roll;
};

struct VecBits {
    unsigned int x, y, z;
};
struct VecPair {
    unsigned int x, y;
};
struct VecBitsP {
    VecPair xy;
    unsigned int z;
};
struct VecEntry {
    char reserved00[0x0C];
    VecBitsP vec;
    char reserved18[8];
};
typedef char VecEntry_size[(sizeof(VecEntry) == 0x20) ? 1 : -1];

} // extern "C"
} // namespace ns4

extern "C" {
extern const float kCameraNearHoleViewMinimumFrames;
extern const float kFloat_0_1_805DDF94;
float __ml__FRC7WVectorRC7WVector(void*, void*);
float WVectorLen__FRC7WVector(void*);
double sqrt(double);
extern void* g_view;
int WView_DefaultAcceptVisibility(void*, void*);
}

// Constrain ball-tracking pitch and choose a closer hole view when the trajectory approaches the hole.
void CCameraWork::TrackBallPitchAndSelectHoleView(float amount) {
    if (action == kCameraTrackBallPitch) {
        ns4::VecBits cur = (*(ns4::VecBits*)&previousTransform[6]);
        ns4::VecBitsP sourceBits;
        ns4::VecBitsP targetBits;
        unsigned int posTemp[3];
        __mi__FRC7WVectorRC7WVector(posTemp, GolfBall__Fv(), position);
        unsigned int targetTemp[3];
        sourceBits = *(ns4::VecBitsP*)Normalize__7WVectorFv(posTemp);
        __mi__FRC7WVectorRC7WVector(targetTemp, (char*)&initialBallPosition[0], (char*)&previousTransform[9]);
        targetBits = *(ns4::VecBitsP*)Normalize__7WVectorFv(targetTemp);
        if (*(float*)&sourceBits.xy.y > *(float*)&cur.y || *(float*)&sourceBits.xy.y < *(float*)&targetBits.xy.y) {
            float out[3];
            *(float*)&sourceBits.xy.y =
                *(float*)&cur.y + kFloat_0_1_805DDF94 * (amount * (*(float*)&sourceBits.xy.y - *(float*)&cur.y));
            __pl__FRC7WVectorRC7WVector(out, position, &sourceBits);
            __as__7WVectorFRC7WVector((char*)&target[0], out);
        } else {
            float s = (kFloat_1_CameraWork - *(float*)&cur.y * *(float*)&cur.y) /
                      (kFloat_1_CameraWork - *(float*)&sourceBits.xy.y * *(float*)&sourceBits.xy.y);
            float scale = sqrt(s);
            *(float*)&sourceBits.xy.x *= scale;
            *(float*)&sourceBits.z *= scale;
            *(float*)&sourceBits.xy.y = *(float*)&cur.y;
            float out[3];
            __pl__FRC7WVectorRC7WVector(out, position, &sourceBits);
            __as__7WVectorFRC7WVector((char*)&target[0], out);
        }
    } else {
        __as__7WVectorFRC7WVector((char*)&target[0], GolfBall__Fv());
    }
    if (trajectoryLastFrame > kCameraNearHoleViewMinimumFrames) {
        if (elapsedTime > kFloat_0_5_CameraWork * trajectoryLastFrame) {
            float b[3];
            ns4::VecBitsP vec2;
            float a[3];
            ns4::VecBitsP vec;
            float magnitude[3];
            ns4::VecBitsP mag;
            {
                void* m = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
                ns4::VecEntry* e = &((ns4::VecEntry*)AsCGolfDoc(m)->holeRecords)[AsCGolfDoc(m)->currentHoleIndex];
                vec = e->vec;
            }
            {
                void* m = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
                ns4::VecEntry* e = &((ns4::VecEntry*)AsCGolfDoc(m)->holeRecords)[AsCGolfDoc(m)->currentHoleIndex];
                vec2 = e->vec;
            }
            __mi__FRC7WVectorRC7WVector(a, PreData__9CGolfBallFi(GolfBall__Fv(), 0), &vec);
            void* trans = GolfBall__Fv();
            void* child = GolfBall__Fv();
            __mi__FRC7WVectorRC7WVector(b, PreData__9CGolfBallFi(child, ((CGolfBall*)trans)->m_preDataCount), &vec2);

            if ((__ml__FRC7WVectorRC7WVector(b, a)) > kFloat_0_CameraWork) {
                {
                    void* m = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
                    ns4::VecEntry* e = &((ns4::VecEntry*)AsCGolfDoc(m)->holeRecords)[AsCGolfDoc(m)->currentHoleIndex];
                    mag = e->vec;
                }
                void* t = GolfBall__Fv();
                void* c = GolfBall__Fv();
                __mi__FRC7WVectorRC7WVector(magnitude, PreData__9CGolfBallFi(c, ((CGolfBall*)t)->m_preDataCount), &mag);
                float len = WVectorLen__FRC7WVector(magnitude);
                if (len < kFloat_16_CameraWork) {
                    unsigned int flags = randomValue;

                    ConfigureMode(((int)(((flags & 1)) ^ (flags >> 31)) - (int)(flags >> 31) + 0x6F), 1);
                }
            }
        }
    }
}

namespace ns4 {
extern "C" {

struct E0E64Manager {
    char reserved00[0x2BC];
    void* entries;
};
struct E0E64Entry {
    char reserved00[0x0C];
    float vec[3];
};

} // extern "C"
} // namespace ns4

// Test the ball sphere against the view and choose another hole angle when it leaves the view.
void CCameraWork::ChangeAngleWhenBallLeavesView() {
    float value = ((CGolfBall*)GolfBall__Fv())->m_radius;
    float key[4];
    __as__7WVectorFRC7WVector(key, GolfBall__Fv());
    key[3] = value;
    if (WView_DefaultAcceptVisibility(g_view, key) == 0) {
        unsigned int flags = randomValue;
        int idx = (int)((flags & 1) ^ (flags >> 31)) - (int)(flags >> 31) + 0x6D;
        ConfigureMode(idx, 1);
    }
}

// Move the camera along the horizontal shot direction at a fixed rate.
void CCameraWork::AdvanceAlongBallHorizontalVector(float amount) {
    struct Source {
        unsigned int x;
        float y;
        unsigned int z;
    };
    Source* current = (Source*)(&((CGolfBall*)GolfBall__Fv())->m_velocity);
    Source source = *current;
    source.y = ns4::kFloat_0_CameraWork;
    float a[3], b[3];
    __ml__FRC7WVectorf(b, &source, amount);
    __ml__FRC7WVectorf(a, b, ns4::kFloat_0_8_CameraWork);
    __apl__7WVectorFRC7WVector(position, a);
    __as__7WVectorFRC7WVector((char*)&target[0], GolfBall__Fv());
    target[1] += ns4::kFloat_2_5_CameraWork;
}

// Interpolate the saved camera positions and rotate the target direction over the transition.
void CCameraWork::InterpolatePositionAndOrientation() {
    // Unlike the duration-normalized variant below, animationTime itself is the blend
    // factor here; duration only gates updates. Do not silently divide by duration.
    if (!(animationTime > duration)) {
        float c[3];
        float b[3];
        float pad[12];
        float a[3];

        __mi__FRC7WVectorRC7WVector(a, endPosition, startPosition);

        __ml__FRC7WVectorf(b, a, animationTime);
        __pl__FRC7WVectorRC7WVector(c, startPosition, b);

        __as__7WVectorFRC7WVector(position, c);
        float r[4];
        float q[4];
        ns4::Quat_Slerp(q, startQuaternion, endQuaternion, animationTime);

        ns4::Quat_ToMatrix3x3(pad, q);
        float d[3];
        __pl__FRC7WVectorRC7WVector(d, (char*)&position[0], (pad + 6));
        __as__7WVectorFRC7WVector((char*)&target[0], d);
    }
}

// Increase the field of view toward the normal camera FOV without overshooting.
void CCameraWork::RestoreNormalFov(float delta) {
    float limit = ns4::kFloat_0_95993108;
    float value = fov + limit * delta;
    fov = value;
    if (value > limit)
        fov = limit;
}

// Start camera shake and reset its sample timer using the current animation time.
void CCameraWork::Quake() {
    float minusOne = ns4::kFloat_neg1_CameraWork;
    float value = animationTime;
    actionFlags |= kCameraShakeActive;
    shakeStartTime = value;
    lastShakeUpdateTime = minusOne;
}

// Lower the camera and keep its target at the current ball position.
void CCameraWork::LowerPositionAndTrackBall(float delta) {
    position[1] -= ns4::kCameraLoweringSpeed * delta;
    void* transform = GolfBall__Fv();
    __as__7WVectorFRC7WVector((char*)&target[0], transform);
}

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Place the camera behind a moving ball using a ground-projected target and ball-radius clearance.
void CCameraWork::PositionBehindMovingBall() {
    float buf[3];
    void* manager = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    void* entries = *((void**)((char*)&AsCGolfDoc(manager)->holeRecords));
    unsigned char idx = AsCGolfDoc(manager)->currentHoleIndex;
    volatile char* entryBase = (volatile char*)entries + idx * 0x20;
    unsigned int point[3];
    point[0] = *((unsigned int*)(entryBase + 0x0C));
    point[1] = *((unsigned int*)(entryBase + 0x10));
    point[2] = *((unsigned int*)(entryBase + 0x14));
    __mi__FRC7WVectorRC7WVector(buf, (char*)GolfBall__Fv(), point);
    buf[1] = ns4::kFloat_0_CameraWork;
    float length = Magnitude__7WVectorCFv(buf);
    __as__7WVectorFRC7WVector(buf, &((CGolfBall*)GolfBall__Fv())->m_velocity);
    buf[1] = ns4::kFloat_0_CameraWork;
    if (Magnitude__7WVectorCFv(buf) > ns4::kFloat_1_CameraWork) {
        if (length > ((CGolfBall*)GolfBall__Fv())->m_radius * ns4::kFloat_3_CameraWork) {
            float result[3];
            void* t = GolfBall__Fv();
            char* tp = (char*)t;
            unsigned int saved[3];
            unsigned int s0 = *((unsigned int*)tp);
            unsigned int s1 = *((unsigned int*)(tp + 4));
            saved[0] = s0;
            saved[1] = s1;
            saved[2] = *((unsigned int*)(tp + 8));
            void* cb = CQuadTree_GetInstance();
            ns4::GetGroundPoint__9CQuadTreeF7WVectorbPib(result, cb, saved, 0, 0, 0);
            __as__7WVectorFRC7WVector((char*)&target[0], result);
            float speed = ((CGolfBall*)GolfBall__Fv())->m_radius;
            target[1] += speed;
            void* r = Normalize__7WVectorFv(buf);
            float buf14[3];
            float out8[3];
            __ml__FfRC7WVector(out8, r, ns4::kCameraBehindBallOffset);
            __mi__FRC7WVectorRC7WVector(buf14, (char*)&target[0], out8);
            __as__7WVectorFRC7WVector((char*)&position[0], buf14);
            position[1] += ns4::kFloat_1_CameraWork;
        }
    }
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

// Blend the camera position while framing the ball with shot-direction and lateral offsets.
void CCameraWork::BlendShotSidePosition(float unused, float delta) {
    float a0[3], d0[3], r0[3], axis[3];
    float s1[3], m1[3], s2[3], m2[3];
    float o2[3], o1[3], o6[3], o5[3], o4[3], o3[3];
    __as__7WVectorFRC7WVector(a0, &((CGolfBall*)GolfBall__Fv())->m_velocity);
    axis[0] = ns4::kFloat_0_CameraWork;
    axis[1] = ns4::kFloat_1_CameraWork;
    axis[2] = ns4::kFloat_0_CameraWork;
    Vector_CrossProduct(r0, axis, a0);
    __as__7WVectorFRC7WVector(d0, r0);
    float sp1 = ((CGolfBall*)GolfBall__Fv())->m_radius;
    __ml__FRC7WVectorf(m1, Normalize__7WVectorFv(a0), sp1);
    __ml__FRC7WVectorf(s1, m1, ns4::kFloat_2_CameraWork);
    __as__7WVectorFRC7WVector(a0, s1);
    float sp2 = ((CGolfBall*)GolfBall__Fv())->m_radius;
    __ml__FRC7WVectorf(m2, Normalize__7WVectorFv(d0), sp2);
    __ml__FRC7WVectorf(s2, m2, ns4::kFloat_1_2_CameraWork);
    __as__7WVectorFRC7WVector(d0, s2);
    float t = (elapsedTime + delta) / trajectoryLastFrame;
    const float& cl = (ns4::s_cameraWorkInterpolationLimit < t) ? ns4::s_cameraWorkInterpolationLimit : t;
    float f = cl;
    __pl__FRC7WVectorRC7WVector(o1, GolfBall__Fv(), d0);
    __mi__FRC7WVectorRC7WVector(o2, o1, a0);
    __as__7WVectorFRC7WVector((char*)&target[0], o2);
    __mi__FRC7WVectorRC7WVector(o3, target, &previousTransform[9]);
    __ml__FRC7WVectorf(o4, o3, f);
    __pl__FRC7WVectorRC7WVector(o5, &previousTransform[9], o4);
    __mi__FRC7WVectorRC7WVector(o6, o5, a0);
    __as__7WVectorFRC7WVector(position, o6);
}

// Empty camera callback retained for its dispatch-table entry.
void CCameraWork::NoopC() {}
// Empty camera callback retained for its dispatch-table entry.
void CCameraWork::NoopD() {}
// Empty camera callback retained for its dispatch-table entry.
void CCameraWork::NoopE() {}

// Retreat along the hole-to-shot direction and widen FOV, disabling motion when close to the hole.
void CCameraWork::RetreatFromHoleAndWidenFov(float amount) {
    float direction[3];
    float point[3];
    ns4::VecBitsP source;
    ns4::VecBitsP targetVec;
    ns4::E0E64Manager* manager = (ns4::E0E64Manager*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned char idx = AsCGolfDoc(manager)->currentHoleIndex;
    ns4::VecEntry* entry = &((ns4::VecEntry*)manager->entries)[idx];
    source = entry->vec;
    __mi__FRC7WVectorRC7WVector(point, ((CameraTransformView*)ns4::s_cameraTransform)->position, &source);
    if (WVectorLen__FRC7WVector(point) < ns4::kFloat_16_CameraWork) {
        *(ns4::VecBitsP*)&motionCallbackThisAdjustment = *(ns4::VecBitsP*)ns4::zeroInitializationWords;
    } else {
        manager = (ns4::E0E64Manager*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        idx = AsCGolfDoc(manager)->currentHoleIndex;
        entry = &((ns4::VecEntry*)manager->entries)[idx];
        targetVec = entry->vec;
        void* trans = GolfBall__Fv();
        void* rot = PreData__9CGolfBallFi(trans, 0);
        __mi__FRC7WVectorRC7WVector(direction, rot, &targetVec);
        direction[1] = ns4::kFloat_0_CameraWork;
        Normalize__7WVectorFv(direction);
        ns4::__amu__7WVectorFf(direction, ns4::kCameraShared20 * amount);
        __apl__7WVectorFRC7WVector(position, direction);
        __as__7WVectorFRC7WVector((char*)&target[0], GolfBall__Fv());
        fov += ns4::kFloat_0_17453292 * amount;
    }
}

// Blend saved positions while rotating the look direction through the stored heading.
void CCameraWork::InterpolatePositionAndDirection() {
    float rotationMatrix[12];
    float c[3];
    float b[3];
    float a[3];
    float base[3];
    float u[3], w[3], v[3];
    float q[3];
    float one = ns4::kFloat_1_CameraWork;
    float t = ns4::kFloat_0_CameraWork;
    float raw = one - animationTime / ns4::kFloat_3_CameraWork;
    t = raw < t ? t : (raw > one ? one : raw);
    base[0] = ns4::kFloat_0_CameraWork;
    base[1] = ns4::kFloat_0_CameraWork;
    base[2] = ns4::kFloat_1_CameraWork;
    ((void (*)(void*, float, int))ns4::Matrix_SetAxisRotation)(rotationMatrix, t * heading, 1);
    ns4::Matrix_TransformPoint(a, base, (char*)&rotation[0]);
    ns4::Matrix_TransformPoint(b, a, rotationMatrix);
    __as__7WVectorFRC7WVector(c, b);
    float alpha = ns4::kFloat_0_CameraWork;
    if (t < alpha)
        alpha = ns4::kFloat_0_CameraWork;
    else if (t > ns4::kFloat_1_CameraWork)
        alpha = ns4::kFloat_1_CameraWork;
    else
        alpha = t;
    __mi__FRC7WVectorRC7WVector(v, startPosition, endPosition);
    __ml__FRC7WVectorf(w, v, alpha);
    __pl__FRC7WVectorRC7WVector(u, endPosition, w);
    __as__7WVectorFRC7WVector(position, u);
    __pl__FRC7WVectorRC7WVector(q, position, c);
    __as__7WVectorFRC7WVector((char*)&target[0], q);
}

// Run a short shot-relative camera move, then select the next action from the first event time.
void CCameraWork::UpdateShortShotRelativeMove() {
    if (animationTime < ns4::kFloat_2_CameraWork) {
        void* ball = GolfBall__Fv();
        ns4::VecBitsP source;
        source = *(ns4::VecBitsP*)((char*)&((CGolfBall*)ball)->m_velocity);
        ns4::Normalize__7WVectorFv(&source);
        float cross[3];
        float axis[3] = {ns4::kFloat_0_CameraWork, ns4::kFloat_1_CameraWork, ns4::kFloat_0_CameraWork};
        ns4::Vector_CrossProduct(cross, axis, &source);
        ns4::Normalize__7WVectorFv(cross);
        void* manager = GolfBall__Fv();
        void* child = GolfBall__Fv();
        float a[3];
        __mi__FRC7WVectorRC7WVector(a, child, (char*)&((CGolfBall*)manager)->m_velocity);
        float b[12];
        ns4::Matrix_BuildCameraTransform(b, GolfBall__Fv(), a);
        float result[3];
        float z = ns4::kFloat_neg3_CameraWork + animationTime;
        float offset[3];
        offset[0] = ns4::kFloat_neg0_7_CameraWork;
        offset[1] = ns4::kFloat_neg0_3_CameraWork;
        offset[2] = z;
        ns4::Matrix_TransformPoint(result, offset, b);
        __as__7WVectorFRC7WVector(position, result);
        int idx = (int)frameIndex - 0xF;
        idx = idx > 0 ? idx : 0;
        __as__7WVectorFRC7WVector((char*)&target[0], PreData__9CGolfBallFi(GolfBall__Fv(), idx));
    } else {
        if (elapsedTime > firstEventFrame)
            pendingAction = 4;
        else
            pendingAction = 0x13;
    }
}

// Place the camera two world units below the hole and look upward with a +Z component.
void CCameraWork::SetBelowHoleView() {
    float c[3];
    unsigned int b[3];
    float a[3] = {ns4::kFloat_0_CameraWork, ns4::kFloat_neg2_CameraWork, ns4::kFloat_0_CameraWork};
    unsigned char idx = AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->currentHoleIndex; // original +0x3b6
    ns4::E0E64Manager* manager = (ns4::E0E64Manager*)(*(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    void* entries = manager->entries;
    ns4::E0E64Entry* entry = (ns4::E0E64Entry*)((char*)entries + idx * 0x20);
    b[0] = ((unsigned int*)entry)[3];
    b[1] = ((unsigned int*)entry)[4];
    b[2] = ((unsigned int*)entry)[5];
    __pl__FRC7WVectorRC7WVector(c, b, a);
    __as__7WVectorFRC7WVector(position, c);
    float e[3];
    float d[3] = {ns4::kFloat_0_CameraWork, ns4::kFloat_1_CameraWork, ns4::kFloat_0_2_CameraWork};
    __pl__FRC7WVectorRC7WVector(e, position, d);
    __as__7WVectorFRC7WVector(target, e);
}

// Interpolate saved camera positions and quaternion orientations for the configured duration.
void CCameraWork::InterpolatePositionAndQuaternion() {
    if (!(animationTime > duration)) {
        float amount = animationTime / duration;
        float r[3], pad[12], c[3], b[3], a[3];
        float q[4], d[3];
        __mi__FRC7WVectorRC7WVector(a, endPosition, startPosition);
        __ml__FRC7WVectorf(b, a, amount);
        __pl__FRC7WVectorRC7WVector(c, startPosition, b);
        __as__7WVectorFRC7WVector(position, c);
        ns4::Quat_Slerp(q, startQuaternion, endQuaternion, amount);
        ns4::Quat_ToMatrix3x3((void*)(r + 1), q);
        __pl__FRC7WVectorRC7WVector(d, position, pad + 4);
        __as__7WVectorFRC7WVector((char*)&target[0], d);
    }
}

extern "C" {
extern const float kFloat_0_033333335_CameraWork;
extern const float kFloat_0_4_CameraWork;
extern const float kFloat_0_61086524_CameraWork;
extern const float kFloat_0_93_GolfCamera;
extern const float kFloat_0_9_CameraWork;
extern const float kFloat_2_0943952_CameraWork;
extern const float kFloat_45_CameraWork;
extern const float kCameraOrbitDistanceDecay;
extern const float kCameraBallFollowRollDecay;
extern const float kFloat_1_2_CameraWork;
extern const float kFloat_0_3_CameraWork;
extern const float kFloat_0_75_CameraWork;
}

// Run an event-driven approach, roll, orbit, and trailing phase around the moving ball.
void CCameraWork::UpdateEventDrivenBallOrbit(float delta, float frameOffset) {
    // Four phases: narrow FOV until firstEvent-45, roll/approach until firstEvent,
    // orbit until event slot 13, then move position/target with 0.93*ball velocity.
    float t = delta;

    struct Bits {
        int y;
        int x;
        int z;
    } source;
    float value74Value = elapsedTime;
    float direction[3];
    float diff[3];
    float value = value74Value;
    float length = firstEventFrame;
    if (value < length - (kFloat_45_CameraWork)) {
        t = ((value + frameOffset)) / length;
        fov = kFloat_0_95993108 - kFloat_0_61086524_CameraWork * t;
        __as__7WVectorFRC7WVector(target, GolfBall__Fv());
        target[1] += kFloat_2_CameraWork * (kFloat_1_CameraWork - t);
    } else if (value < length) {

        float base = kFloat_0_95993108;
        t = (value + frameOffset) / length;
        float rotation = kDegreesToRadians * (kFloat_60_CameraWork * t + kFloat_neg30_CameraWork);
        fov = base;
        roll = rotation;
        Bits* src = (Bits*)(&((CGolfBall*)GolfBall__Fv())->m_velocity);
        int y = src->y;
        unsigned int x = src->x;
        source.y = y;
        source.x = x;
        source.z = src->z;
        Normalize__7WVectorFv(&source);
        float axis[3] = {kFloat_0_CameraWork, kFloat_1_CameraWork, kFloat_0_CameraWork};
        Vector_CrossProduct(direction, axis, &source);
        Normalize__7WVectorFv(direction);
        float out[3];
        float scale[3];
        ((void (*)(void*, void*, float))__ml__FfRC7WVector)(scale, direction,
                                                            kFloat_0_5_CameraWork * t + kFloat_neg1_CameraWork);
        __pl__FRC7WVectorRC7WVector(out, GolfBall__Fv(), scale);
        __as__7WVectorFRC7WVector(target, out);
        float out2[3];
        float scale2[3];
        ((void (*)(void*, void*, float))__ml__FfRC7WVector)(
            scale2, &source, kFloat_50_CameraWork * (kFloat_1_CameraWork - kFloat_0_9_CameraWork * t));
        __pl__FRC7WVectorRC7WVector(out2, target, scale2);
        __as__7WVectorFRC7WVector(position, out2);
        float out3[3];
        __mi__FRC7WVectorRC7WVector(out3, GolfBall__Fv(), target);
        __as__7WVectorFRC7WVector(startPosition, out3);
    } else if (!(!((int)frameIndex < ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[13]))) {
        float zero;
        float speed = roll;
        zero = kFloat_0_CameraWork;
        if (speed > zero) {
            float newSpeed = speed - kFloat_1_0471976_cameraWork * t;
            roll = newSpeed;
            if (newSpeed < zero)
                roll = zero;
        }
        __mi__FRC7WVectorRC7WVector(diff, GolfBall__Fv(), position);
        diff[1] = kFloat_0_CameraWork;
        horizontalBallDistance = Magnitude__7WVectorCFv(diff);
        Normalize__7WVectorFv(diff);
        float cross[3], axis[3] = {kFloat_0_CameraWork, kFloat_1_CameraWork, kFloat_0_CameraWork};
        Vector_CrossProduct(cross, diff, axis);
        __as__7WVectorFRC7WVector(diff, cross);
        float f = kFloat_0_033333335_CameraWork * (elapsedTime - firstEventFrame);
        if (f < kFloat_1_CameraWork) {
            float step[3];
            float scaled[3];
            float step2[3];
            float factor = kFloat_1_2_CameraWork * f + kFloat_1_CameraWork;
            float value1FCValue = kFloat_2_0943952_CameraWork * horizontalBallDistance;
            float value1FCValue2 = kFloat_2_0943952_CameraWork * horizontalBallDistance;

            ((void (*)(void*, void*, float))__ml__FfRC7WVector)(scaled, diff, ((value1FCValue)*factor));
            __ml__FRC7WVectorf(step, scaled, t);
            __apl__7WVectorFRC7WVector(position, step);
            float py = (position[1] + t);
            float ay = target[1] + kFloat_0_75_CameraWork * t;
            position[1] = py;
            target[1] = ay;

            __ml__FRC7WVectorf(step2, startPosition, t);
            __apl__7WVectorFRC7WVector(target, step2);
        }
    } else {
        float step[3];
        float scaled[3];
        ((void (*)(void*, void*, float))__ml__FfRC7WVector)(scaled, &((CGolfBall*)GolfBall__Fv())->m_velocity,
                                                            kFloat_0_93_GolfCamera);
        __ml__FRC7WVectorf(step, scaled, t);
        __apl__7WVectorFRC7WVector(position, step);
        float step2[3];
        float scaled2[3];
        ((void (*)(void*, void*, float))__ml__FfRC7WVector)(scaled2, &((CGolfBall*)GolfBall__Fv())->m_velocity,
                                                            kFloat_0_93_GolfCamera);
        __ml__FRC7WVectorf(step2, scaled2, t);
        __apl__7WVectorFRC7WVector(target, step2);
        if (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[7] == -1 ||
            (int)frameIndex < ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[7])
            target[1] -= kFloat_0_5_CameraWork * t;
    }
}

// Reduce heading, distance, and height while orbiting toward the ball, then request the next action.
void CCameraWork::UpdateDescendingBallOrbit(float amount) {
    float x = heading - ns4::kFloat_0_5235988_805DDFCC * amount;
    float y = horizontalBallDistance - kCameraOrbitDistanceDecay * amount;
    float z = ballHeightOffset - kFloat_2_CameraWork * amount;
    ballHeightOffset = z;
    horizontalBallDistance = y;
    heading = x;
    float pad[6], a[6], c[3], b[3], d[3];
    ((void (*)(void*, float, int))Matrix_SetAxisRotation)(
        a, AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->headingRadians - heading, 1);
    ((void (*)(void*, void*, float))__ml__FfRC7WVector)(b, pad, horizontalBallDistance);
    __mi__FRC7WVectorRC7WVector(c, GolfBall__Fv(), b);
    __as__7WVectorFRC7WVector(position, c);
    position[1] += ballHeightOffset;
    __pl__FRC7WVectorRC7WVector(d, position, pad);
    __as__7WVectorFRC7WVector(target, d);
    if (heading < kFloat_0_CameraWork)
        pendingAction = 0xCC;
}

// Move along a side-offset view of the recorded trajectory, then switch to ball tracking.
void CCameraWork::UpdateBallPathSideView(float unused, float delta) {
    void* mgr = GolfBall__Fv();
    if (!(!(((CGolfBall*)mgr)->m_eventPreDataIndex[10] > (int)frameIndex))) {
        float tmp[3];
        float scaled[3];
        float value74Value2 = elapsedTime;
        float buf[3];
        float value74Value3 = elapsedTime;
        mgr = GolfBall__Fv();
        float value74Value4 = elapsedTime;
        int cnt = ((CGolfBall*)mgr)->m_eventPreDataIndex[10];

        float v0[3];
        v0[0] = kFloat_0_CameraWork;
        v0[2] = kFloat_0_CameraWork;
        v0[1] = kFloat_1_CameraWork;

        // f31 = travel progress: (elapsed frame + delta) / frame index of ball event 10.
        // Camera position = trajectory start + (start->event-10 vector) * (0.3 * progress + 0.4)
        //   + 5 * normalized(up x that vector); target = ball position + (0, 1 - progress, 0).
        // (The 0.3 / 0.4 / 5 constants are shared floats kFloat_0_3/0_4/5_CameraWork.)
        float f31 = ((value74Value4) + delta) / ((float)cnt);
        void* childA = PreData__9CGolfBallFi(GolfBall__Fv(), 0);
        void* m2 = GolfBall__Fv();
        void* childB = PreData__9CGolfBallFi(GolfBall__Fv(), ((CGolfBall*)m2)->m_eventPreDataIndex[10]);
        __mi__FRC7WVectorRC7WVector(buf, childB, childA);

        Vector_CrossProduct(tmp, v0, buf);
        Normalize__7WVectorFv(tmp);
        ns4::__amu__7WVectorFf(tmp, kFloat_5_CameraWork);

        float buf2[3];
        void* childC = PreData__9CGolfBallFi(GolfBall__Fv(), 0);
        void* m3 = GolfBall__Fv();
        void* childD = PreData__9CGolfBallFi(GolfBall__Fv(), ((CGolfBall*)m3)->m_eventPreDataIndex[10]);
        __mi__FRC7WVectorRC7WVector(buf2, childD, childC);

        __ml__FfRC7WVector(scaled, buf2, kFloat_0_3_CameraWork * f31 + kFloat_0_4_CameraWork);
        float tmp2[3];
        void* childE = PreData__9CGolfBallFi(GolfBall__Fv(), 0);
        __pl__FRC7WVectorRC7WVector(tmp2, childE, scaled);
        float tmp3[3];
        __pl__FRC7WVectorRC7WVector(tmp3, tmp2, tmp);
        __as__7WVectorFRC7WVector(position, tmp3);

        float v1[3];
        v1[0] = kFloat_0_CameraWork;
        v1[1] = kFloat_1_CameraWork - f31;
        v1[2] = kFloat_0_CameraWork;
        void* mgr2 = GolfBall__Fv();
        float tmp4[3];
        __pl__FRC7WVectorRC7WVector(tmp4, mgr2, v1);
        __as__7WVectorFRC7WVector(target, tmp4);
    } else {
        pendingAction = 4;
    }
}

#pragma opt_propagation off
// Follow interpolated trajectory samples and adjust roll and height at recorded shot events.
void CCameraWork::UpdatePreDataBallFollow(float deltaTime, float frameBlend) {
    // Before event 10, interpolate samples at (4*currentFrame + event10)/5: this looks
    // ahead toward that event. Afterward deltaTime drives roll/height, not sample blending.
    float f31;
    float f30;
    float numerator;
    float cur;
    f30 = frameBlend;
    f31 = deltaTime;
    void* mgr = GolfBall__Fv();
    int limit = ((CGolfBall*)mgr)->m_eventPreDataIndex[10];
    if ((int)frameIndex < limit) {
        float tmp2[3];
        float tmp1[3];
        float tmp0[3];
        mgr = GolfBall__Fv();
        int cnt = ((CGolfBall*)mgr)->m_eventPreDataIndex[10];
        float denominator = (float)cnt;
        numerator = elapsedTime + f30;
        cur = motionStartFrame;
        numerator -= cur;
        roll = kFloat_0_2617994;
        f31 = numerator / (denominator - cur);

        mgr = GolfBall__Fv();
        int currentSampleIndex;
        int nextSampleIndex;
        nextSampleIndex = (((int)frameIndex + 1) * 4 + ((CGolfBall*)mgr)->m_eventPreDataIndex[10]) / 5;
        mgr = GolfBall__Fv();
        void* childA = PreData__9CGolfBallFi(mgr, nextSampleIndex);
        ((void (*)(void*, void*, float))__ml__FfRC7WVector)(tmp0, childA, f30);

        mgr = GolfBall__Fv();
        currentSampleIndex = ((int)frameIndex * 4 + ((CGolfBall*)mgr)->m_eventPreDataIndex[10]) / 5;
        mgr = GolfBall__Fv();
        void* childB = PreData__9CGolfBallFi(mgr, currentSampleIndex);
        ((void (*)(void*, void*, float))__ml__FfRC7WVector)(tmp1, childB, kFloat_1_CameraWork - f30);

        __pl__FRC7WVectorRC7WVector(tmp2, tmp1, tmp0);
        __as__7WVectorFRC7WVector(position, tmp2);

        position[1] += kFloat_2_CameraWork * f31 + kFloat_neg1_CameraWork;

        mgr = GolfBall__Fv();
        __as__7WVectorFRC7WVector(target, mgr);
    } else {
        mgr = GolfBall__Fv();
        // Slot [2] is the earlier event and [1] the later one (the nested test below is only meaningful when
        // [2] < [1]); the same convention names them elsewhere in this file.
        int secondEventIndex = ((CGolfBall*)mgr)->m_eventPreDataIndex[1];
        if ((int)frameIndex < secondEventIndex) {
            mgr = GolfBall__Fv();
            target[0] = ((CGolfBall*)mgr)->m_pos.x;
            mgr = GolfBall__Fv();
            target[2] = ((CGolfBall*)mgr)->m_pos.z;
            mgr = GolfBall__Fv();
            int firstEventIndex = ((CGolfBall*)mgr)->m_eventPreDataIndex[2];
            if ((int)frameIndex < firstEventIndex) {
                roll -= kCameraBallFollowRollDecay * f31;
                mgr = GolfBall__Fv();
                int heightPhaseEndIndex = ((CGolfBall*)mgr)->m_eventPreDataIndex[10] + 0x1E;
                if ((int)frameIndex < heightPhaseEndIndex) {
                    position[1] += ns4::kFloat_6_CameraWork * f31;
                } else {
                    target[1] += kFloat_10_cameraWork * f31;
                }
            } else {
                mgr = GolfBall__Fv();
                float v = ((CGolfBall*)mgr)->m_pos.y;
                if (target[1] > v) {
                    target[1] -= ns4::kCameraShared20 * f31;
                } else {
                    target[1] -= kFloat_5_CameraWork * f31;
                }
            }
        }
    }
}

#pragma opt_propagation reset
