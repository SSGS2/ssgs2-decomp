// Secondary shot-prediction ribbon: fixed-step preview, world-space strip width,
// independent widescreen projection, and render-state setup. Main entry points:
// CDrawDriveLine_UpdateShotPrediction -> BallPhysics_IntegrateWindSpinAndDrag;
// CDrawDriveLine_DrawRibbon -> MtxPerspectiveFovLH (gtl_vector_matrix_math.cpp).
// Mod lookup (all scalar definitions in u8027775C_data.cpp):
// - Aspect ratio: kDriveLineRibbonAspectRatio = float bits 0x3faaaaab (4/3),
//   secondary vertical FOV kDriveLineRibbonFieldOfView = pi/4 radians, far clip
//   kDriveLineRibbonFarClip = 100000 world units. Near clip uses kFloat_3_DriveLine
//   = 3, ALSO a lift-force multiplier: do not treat it as a private clip setting.
//   Coordinate the secondary projection with main WView inputTransform and the
//   renderer's projection cache; this draw installs both and does not restore them.
// - Width: kDriveLineRibbonRadiusScale = 1.2 times live ball radius on each side;
//   total width is 2.4 radii in world XZ, not pixels. Camera FOV also affects size.
// - Tail: last kRibbonFadeSegments (5) segments, alpha scale 255 / divisor 6.
// - Sampling: kPredictionSteps=150, interval=3, fixed dt=1/30; seed + 50 samples,
//   renderer limit=50. This is recalculated preview time, not per-render cadence.
// - Force/roll controls: kDriveLineSpinForceScale, WindForceScale, GravityScale,
//   DragScale and RollSpeedVariant0/1/2; live shot parameters feed them below.
//   Safe tuning ranges and physical conversion to yards are not established.
// Force/roll parameter map (binary32 definitions: u8027775C_data.cpp):
//   IntegrateWindSpinAndDrag: force is accumulated, divided by live ball mass,
//   then integrated using dt=1/30 simulation second, velocity before position.
//   SpinForceScale=.75 multiplies p.spin*params.spin along normalize(-vz,0,vx);
//   WindForceScale=.02 multiplies CWind_GetWindVectorAtPoint's result;
//   GravityScale=34.2952957 is world units/s^2 (mass cancels in force/mass);
//   shared kFloat_3_DriveLine=3 multiplies p.lift*params.lift AND near clip;
//   DragScale=.00008 multiplies |v|*v and is then divided by mass. The dimensions
//   of the spin/wind/lift inputs remain engine-defined; these are conversion
//   coefficients into force, not separately established SI accelerations.
//   ForceEpsilon=1e-5 suppresses small spin/lift, not motion/position directly.
//   Positive coefficient increases strengthen that term; drag subtracts along v.
//   UpdateShotPrediction: GroundClearanceRadiusScale=2 is a radius multiplier;
//   RollDistanceScale=3.2 converts (sqrt(scale)*club distance - bias100) to world
//   distance. Greater bias triggers earlier; greater scale triggers later only
//   for a positive parenthesis. RollSpeedVariant0/1/2=74/76/80 world units/s,
//   multiplied by sqrt(scale) once at the transition; other variants normalize
//   without a variant multiplier. RollTransitionLift=2.5 feeds the SAME lift
//   force term, then decays by 1/30 per later step in the special-kind-4 branch.
//   Before rollStartStep, ground-roll suppresses wind/gravity/drag, not spin.
//   After transition these forces return, including lift; terrain clamps only
//   the documented pre-transition branch. Coordinate speed, transition distance,
//   clearance, lift decay, mass and force coefficients when modding roll.
// These values are stored as uint32 FLOAT BITS in the data companion; do not
// change them as integer values or merge the force and projection consumers.
// Storage/ownership: draw_drive_line_match.h; sample ABI below; renderer callbacks
// borrow vertices/texture, owner destructors release the resource and sample buffer.
// Actor messages borrow stack output addresses synchronously; Screen's optional
// ball preview determines whether the adjustment output is written (see below).
// Byte layout of the existing string pool; encoded lengths include the NUL.
struct DriveLineStrings {
    char padding000[0x11];
    char screenActor[7]; // Screen
    char spinActor[5]; // Spin
    char playerActor[7]; // Player
};
enum {
    kDriveLineStrings_screenActor = (int)&((DriveLineStrings*)0)->screenActor,
    kDriveLineStrings_spinActor = (int)&((DriveLineStrings*)0)->spinActor,
    kDriveLineStrings_playerActor = (int)&((DriveLineStrings*)0)->playerActor,
};

#include "game/golfball.h"
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
#include "game/draw_drive_line_match.h"
#include "game/wvector.h"

// Translation unit 0x8027775C-0x802789A8 (13 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Actor HandleMsg request IDs; phase1_class_tables' CScreen/CSpin/CGolfPlayer
// tables install these handlers in slot 9. Payload addresses live through the
// synchronous dispatch only; a missing actor leaves the supplied output alone.
enum {
    kRequestShotDirectionResultAddress = 0x33,   // supplied address: &prediction.shotFlagsBits
    kRequestCurveSpinAddresses = 1,      // supplied addresses: &m_initCurve, &m_shotSpinSetting
    kRequestPreviewBasisRowAddress = 0x23,  // supplied address: &adjustment (x component is used)
    kRequestLaunchYawOffsetAddress = 3          // supplied address: &offset (added to the launch yaw)
};
enum {
    kShotPowerBoost = 0x10, kShotGroundRoll = 0x20,
    kPredictionSteps = 150, kSampleStepInterval = 3, kMaxRibbonSamples = 50,
    kPredictionBalloonPop = 3, kPutterCategory = 3, kTrajectoryKind4WithLiftDecay = 4,
    kPredictionStateAim = 0x8, kPredictionStateCameraView = 0x100,
    kPredictionStateUnresolved400 = 0x400,
    kPredictionGameStateMask = kPredictionStateAim | kPredictionStateCameraView | kPredictionStateUnresolved400,
    kDrawTriangleStrip = 0x98
};

// Gate: Balloon Pop party mode 3 (+0xAD8), any of aim/camera-view/state 0x400,
// a non-putter club, and predictionEnabled. Source chain: title multiplayer
// result 3 -> PartyMain message {4,3} (title_main_and_top_menu.cpp) -> partyGameMode=3
// (CPartyMain_HandleMsg, party_main_task_and_phf_helpers.cpp). CScreen_OnLoad installs CBalloonInfo for 3
// (ingame_hud_screen.cpp); CGolfRule_OnProcess queries BalloonSystem scores/items only for
// 3 (golf_rule_round_flow_and_shot_sim.cpp). Public mode name: Balloon Pop.
// 0x8 runs aim input; 0x100 runs camera-view input/dismissal in GolfRule's update.
// 0x400 has no per-state work there; EnterState publishes it/resets the clock
// without an entry action. No named transition into it or player-facing label
// is established in the recovered source. Keep this admission bit, but do not
// present it as a selectable camera mode or invent a transition to reach it.
// This mask tests ANY bit, not equality; it does not make compound rule states
// valid for GolfRule's separate equality-based state dispatch.
// Trajectory kind 4 adds power-based lift, decays it per step, and enables the
// rolling transition below. Its player-facing shot name is unproved.

struct TargetState {
    unsigned int unused[2];
    unsigned char value;
};

#pragma dont_inline off
struct TargetOwner {
    unsigned int unused;
    TargetState* state;
};

#pragma dont_inline on
// Read the selected shot-state variant used to choose rolling speed.
unsigned char TargetOwner_GetStateValue(const TargetOwner* self) {
    return self->state->value;
}

#pragma dont_inline on
// Test whether either power-boost or ground-roll shot behavior is set.
bool DriveLinePrediction_HasPowerBoostOrGroundRoll(const unsigned int* self) {
    return (self[2] & kShotPowerBoost) || (self[2] & kShotGroundRoll);
}

#pragma dont_inline off
struct FloatState {
    char unused[0xc0];
    float radius; // Same radius member as CGolfBall, with this accessor's original ABI view.
};

FloatState* GolfBall__Fv();

#pragma dont_inline on
// Read the live ball radius used for prediction and ribbon width.
float CDrawDriveLine_GetBallRadius(void* = 0) {
    return GolfBall__Fv()->radius;
}

#pragma dont_inline on
// Prediction-record initialization hook; the original implementation does nothing.
void DriveLinePrediction_DefaultConstructNoOp(void* = 0) {}

#pragma dont_inline on
// Read the trajectory kind byte from the instance table.
unsigned char InstanceTable_GetTrajectoryKind(const unsigned char* self) {
    return self[0x48];
}

#pragma dont_inline on
// Return the power-boost shot bit as zero or one.
unsigned int DriveLinePrediction_HasPowerBoost(const unsigned int* self) {
    return (self[2] >> 4) & 1;
}

#pragma dont_inline off
struct VirtualTarget {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void handleMessage(void*);
};

#pragma dont_inline on
// Dispatch a borrowed message to slot 9 (HandleMsg) and return the actor/null.
// Spell out the second ABI register: the old no-argument virtual declaration
// happened to leave r4 intact under this exact compiler recipe. Explicit
// forwarding must preserve the compiled function, including this ABI behavior.
VirtualTarget* DriveLineActor_DispatchMessageAndReturn(VirtualTarget* self, void* message = 0) {
    if (!self) return 0;
    self->handleMessage(message);
    return self;
}

#pragma dont_inline off
extern void* g_drawDriveLine;
void* BaseObject_dtor(void*, int);
void __dl__FPv(void*);
void appFreeUni__FPv(void*);

} // extern "C"

// Clear the singleton before destroying its base object.
WSingleton<CDrawDriveLine>::~WSingleton() {
    g_drawDriveLine = 0;
    BaseObject_dtor(this, 0);
}

// Release owned trajectory sample storage after resetting its logical size.
DriveLineStorage::~DriveLineStorage() {
    if (this) {
        if (this) {
            if (this) {
                if (data) {
                    count -= (unsigned int)count;
                    appFreeUni__FPv(data);
                }
            }
        }
    }
}

// Release the ribbon resource and its owned drawing child.
CDrawDriveLine::~CDrawDriveLine() {
    vtable = __vt__14CDrawDriveLine;
    ResourceManager_ReleaseId(g_resourceManager, resource);
    if (child) child->destroy(1);
}

extern "C" {

struct BufferTarget {
    void* data;
    unsigned int count;
};

struct Child {
    unsigned int unused;
    virtual void destroy(int);
};
struct Owner {
    void* vtable;
    char pad[12];
    BufferTarget buffer;
    char pad18[8];
    Child* child;
    void* handle;
};
extern char __vt__14CDrawDriveLine[];
extern void* g_resourceManager;
void ResourceManager_ReleaseId(void*, void*);
extern const float kFloat_0_AutoSetup, kDriveLineForceEpsilon, kDriveLineSpinForceScale, kFloat_99999_DriveLineWindQuery, kDriveLineWindForceScale, kDriveLineGravityScale, kFloat_3_DriveLine, kDriveLineDragScale, kFloat_OneThirtieth_DriveLine;
struct Vec {
    float x,y,z;
    // Leave coordinates uninitialized for an immediately following engine write.
    Vec() {}
    // Initialize XYZ while retaining the original store order.
    Vec(float a,float b,float c) { z=c; x=a; y=b; }
};
struct Physics {
    // World-space position (course distance units), velocity (units/simulation-time unit).
    // +Y is up; X/Z form the ground plane. SI conversion is not established.
    // rollStartStep is -1 before transition, then a zero-based integration index.
    int rollStartStep;
    unsigned int shotFlags;
    unsigned int shotFlagsBits;
    float spin, lift;
    // curveSetting/spinSetting start as the ball's m_initCurve/m_shotSpinSetting;
    // curveSetting is then adjusted and drives the launch-speed scale and sidespin.
    float curveSetting, spinSetting;
    Vec velocity, position;
};
struct Params { char pad[16]; float spin, lift; };
struct ParamsOwner { void* unused; Params* params; };
ParamsOwner* InstanceTable_Get();
void* CWind_GetInstance();
Vec* Normalize__7WVectorFv(Vec*);
void __ml__FfRC7WVector(Vec*, const Vec*, float);
void Vec3_DivideScalar(Vec*, float, const Vec*);
void __ml__FRC7WVectorf(Vec*, const Vec*, float);
Vec* __apl__7WVectorFRC7WVector(Vec*, const Vec*);
void __ami__7WVectorFRC7WVector(Vec*, const Vec*);
float Magnitude__7WVectorCFv(const Vec*);
void CWind_GetWindVectorAtPoint(Vec*, void*, const Vec*, float);
// Remove a negative sign using the original float bit test and conditional negate.
inline float absoluteFloat(const float& x) { return (*(unsigned int*)&x & 0x80000000) ? -x : x; }
// Test the physics force epsilon without changing the original float comparison.
inline int isNegligible(const float& x) { return absoluteFloat(x) < kDriveLineForceEpsilon; }
// Advance prediction velocity and position by one 1/30-second physics step.
// First ABI receiver unused; p required and mutated in place. No allocation,
// collision or timing accumulator. The fixed step is a simulation-time convention;
// changing presentation cadence alone does not change this bounded prediction loop.
void BallPhysics_IntegrateWindSpinAndDrag(void*, Physics* p) {
    // Semi-implicit Euler: acceleration = force/mass; v += a/30; x += NEW v/30.
    // Spin axis {-vz,0,vx} lies in the ground plane; lift/gravity act along Y.
    // Ground-roll before rollStartStep suppresses wind/gravity/drag, but may
    // still apply spin. No collision/terrain resolution occurs in this helper.
    int suppressWindAndGravity = 0;
    Vec force;
    force.x=kFloat_0_AutoSetup; force.y=kFloat_0_AutoSetup; force.z=kFloat_0_AutoSetup;
    if (p->rollStartStep < 0) {
        if (!isNegligible(p->spin)) {
            Vec temp;
            Vec axis(-p->velocity.z, kFloat_0_AutoSetup, p->velocity.x);
            float scale = InstanceTable_Get()->params->spin;
            __ml__FfRC7WVector(&temp, Normalize__7WVectorFv(&axis), p->spin * (kDriveLineSpinForceScale * scale));
            *(WVector*)&force += *(WVector*)&temp;
        }
        if (p->shotFlags & kShotGroundRoll) suppressWindAndGravity = 1;
    }
    if (!suppressWindAndGravity) {
        Vec temp, wind;
        CWind_GetWindVectorAtPoint(&wind, CWind_GetInstance(), &p->position, kFloat_99999_DriveLineWindQuery);
        __ml__FfRC7WVector(&temp, &wind, kDriveLineWindForceScale);
        *(WVector*)&force += *(WVector*)&temp;
        force.y -= kDriveLineGravityScale * ((CGolfBall*)GolfBall__Fv())->m_mass;
        if (!isNegligible(p->lift)) {
            force.y += p->lift * (kFloat_3_DriveLine * InstanceTable_Get()->params->lift);
        }
        Vec drag;
        __ml__FfRC7WVector(&drag, &p->velocity, kDriveLineDragScale * Magnitude__7WVectorCFv(&p->velocity));
        *(WVector*)&force -= *(WVector*)&drag;
    }
    Vec b, a, c;
    float mass = ((CGolfBall*)GolfBall__Fv())->m_mass;
    Vec3_DivideScalar(&a, mass, &force);
    __ml__FRC7WVectorf(&b, &a, kFloat_OneThirtieth_DriveLine);
    *(WVector*)(&p->velocity) += *(WVector*)&b;
    __ml__FRC7WVectorf(&c, &p->velocity, kFloat_OneThirtieth_DriveLine);
    *(WVector*)(&p->position) += *(WVector*)&c;
}
} // extern "C"

extern "C" {
// Local ABI view: mode aliases CSharedDoc::partyGameMode at +0xAD8.
// CSharedDoc::gameMode lives elsewhere (+0x808).
struct GameState { char pad[0xad8]; int mode; };
struct PlayerState { char pad[0x3c1]; unsigned char index; };
struct PlayerStats { char pad[0xd8]; int power; };
struct Message { char data[32]; };
struct Matrix { char data[24]; Vec axis; char tail[12]; };
struct Sample { Vec position, velocity; };
extern const float kFloat_1_DriveLine, kDriveLinePowerBoostSpeedScale, kFloat_0_5_ShotParameters, kFloat_neg1_ShotParameters, kDriveLineShotCurveYawScale, kFloat_0_1_ShotParameters;
extern const float kDriveLineSpecialShotBaseLift, kDriveLineSpecialShotPowerDivisor, kDriveLineGroundClearanceRadiusScale, kDriveLineRollDistanceScale, kDriveLineRollDistanceBias, kDriveLineRollSpeedVariant0, kDriveLineRollSpeedVariant1, kDriveLineRollSpeedVariant2, kDriveLineRollTransitionLift;
extern char swingTaleStrings[];
GameState* SharedDoc_GetCurrent();
PlayerState* GolfDoc_GetCurrent();
unsigned int* CGolfDoc_GetGameStateFlagsPtr(PlayerState*);
unsigned char InstanceTable_GetClubCategory(ParamsOwner*);
int InstanceTable_GetPowerShotModeWord(ParamsOwner*);
void __as__7WVectorFRC7WVector(Vec*, const Vec*);
void* DriveLineRecord_Init(Message*, int, int, void*, void*, void*, void*, void*);
void* TaskManager_GetCurrentChild();
VirtualTarget* TaskActorTable_FindByName(void*, const char*);
float ShotParameters_GetBaseLaunchSpeed(ParamsOwner*);
float Math_SquareRootApprox(float);
void* CQuadTree_GetInstance();
float CQuadTree_GetMaxPowerOnGround(void*);
void WVector_DefaultConstructNoOp(float*);
void Matrix_SetAxisRotation(Matrix*, float, int);
float ClubSelection_GetLaunchAngle(ParamsOwner*, int);
float Math_AbsFloatConditional(float);
void Matrix_TransformPoint(Vec*, const Vec*, const Matrix*);
void* PartyItemSlots_GetInstance();
struct Bonus { char pad[20]; float value; };
Bonus* CAccState_GetEquipmentBonuses(void*); // Accessory-state block containing the equipment stat bonuses.
float CSharedDoc_ComputeAuxPartModifier(GameState*, unsigned char, int);
PlayerStats* GolfPlayer_GetCurrentEntryByIndex(unsigned char);
float Math_MaxFloat(float,float);
void RecordVector_ClearSize(BufferTarget*);
void SixWordRecord_NoOpCleanup(Sample*);
void SixWordRecordVector_PushBack(BufferTarget*, const Sample*);
float CQuadTree_ResolveGroundHeight(void*, Vec*, float);
float GolfParameter_ResolveFloat(ParamsOwner*, int);
void __amu__7WVectorFf(Vec*, float);

// Build a sampled shot trajectory from ball, club, actor and power-shot settings.
// self required. Ineligible mode/state/club/disabled prediction leaves existing
// buffer and dirty flag untouched. Eligible work marks dirty then replaces samples;
// no success/failure return. The append operation owns any reallocated storage.
void CDrawDriveLine_UpdateShotPrediction(Owner* self) {
    Physics prediction;
    Sample trajectorySample;
    // Output buffer is owned by self; each 24-byte sample is {world position,
    // world velocity}. All initial settings are borrowed from the live game.
    // Integrate 150 steps at 1/30 s = 5 s. Seed t=0, then store AFTER steps
    // i=0,3,...147: times 1/30,4/30,...148/30 (51 samples including the seed).
    // The renderer consumes at most 50, so it may omit that final sample.
    // Samples precede this iteration's terrain clamp/ground-roll corrections;
    // moving the append below them would change the visible trajectory.
    float groundClearanceRadiusScale;
    if (SharedDoc_GetCurrent()->mode != kPredictionBalloonPop) return;
    if (!(*CGolfDoc_GetGameStateFlagsPtr(GolfDoc_GetCurrent()) & kPredictionGameStateMask)) return;
    if (InstanceTable_GetClubCategory(InstanceTable_Get()) == kPutterCategory) return;
    if (!((CDrawDriveLine*)self)->predictionEnabled) return;
    ((CDrawDriveLine*)self)->predictionDirty = 1;
    FloatState* source = GolfBall__Fv();

    // Phase 1: synchronous HandleMsg outputs. DriveLineRecord_Init
    // (pet_preload_async.cpp) packages {scheduledTimeMs=0, code, five payload words, 0}.
    // Screen 0x33 writes shotDirectionResult to shotFlagsBits (zero fallback).
    // Spin 1 (tail_effect_spin_and_bubble.cpp) writes normalized hitX/hitY into the BALL's curve/spin;
    // the prediction copies precede that query, so do not move them below it.
    // Player 3 (golf_player_actor.cpp) writes heading radians into the zero-seeded yaw offset.
    // Screen 0x23 writes oriented preview matrix row m[3..5] into adjustment
    // only when ballPreview exists. Thus the adjustment branch REQUIRES a live
    // Screen actor WITH ballPreview; mode/club admission alone does not prove
    // that invariant. Absent actor/preview leaves adjustment.x indeterminate,
    // which then changes launch speed/spin. Existing caller CScreen_OnProcess
    // invokes prediction in UpdateHudLoadedPanelsAndInput BEFORE its layout-ready
    // test. OnLoad attempts ScreenBallPreview_Ctor and publishes it, but allows
    // a null allocation; admission here does not test that failure. A new caller
    // must establish preview readiness; there is no error return/zero fallback.
    // Retain this initialization/call order and the unchanged-output failure.
    DriveLinePrediction_DefaultConstructNoOp(&prediction);
    *(WVector*)(&prediction.position) = *(WVector*)((Vec*)source);
    *(int*)&prediction.shotFlagsBits = 0;

    Message m0;
    void* msg = DriveLineRecord_Init(&m0,0,kRequestShotDirectionResultAddress,&prediction.shotFlagsBits,0,0,0,0);
    DriveLineActor_DispatchMessageAndReturn(TaskActorTable_FindByName(TaskManager_GetCurrentChild(),swingTaleStrings+kDriveLineStrings_screenActor),msg);
    prediction.curveSetting = ((CGolfBall*)source)->m_initCurve;
    prediction.spinSetting = ((CGolfBall*)source)->m_initSpin;
    FloatState* a = GolfBall__Fv();
    FloatState* b = GolfBall__Fv();
    Message m1;
    msg = DriveLineRecord_Init(&m1,0,kRequestCurveSpinAddresses,&((CGolfBall*)b)->m_initCurve,&((CGolfBall*)a)->m_initSpin,0,0,0);
    DriveLineActor_DispatchMessageAndReturn(TaskActorTable_FindByName(TaskManager_GetCurrentChild(),swingTaleStrings+kDriveLineStrings_spinActor),msg);
    float scale; scale = kFloat_1_DriveLine;

    float speed; speed = (Math_SquareRootApprox(scale)) * (ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get()));
    if (DriveLinePrediction_HasPowerBoost((unsigned int*)&prediction)) speed *= kDriveLinePowerBoostSpeedScale;
    speed *= Math_SquareRootApprox(CQuadTree_GetMaxPowerOnGround(CQuadTree_GetInstance()));
    if (!InstanceTable_GetPowerShotModeWord(InstanceTable_Get()) || DriveLinePrediction_HasPowerBoostOrGroundRoll((unsigned int*)&prediction)) {
        // Screen query requires its preview; no local initialization/fallback.
        Vec adjustment;
        WVector_DefaultConstructNoOp(&adjustment.x);
        Message m2;
        msg = DriveLineRecord_Init(&m2,0,kRequestPreviewBasisRowAddress,&adjustment,0,0,0,0);
        DriveLineActor_DispatchMessageAndReturn(TaskActorTable_FindByName(TaskManager_GetCurrentChild(),swingTaleStrings+kDriveLineStrings_screenActor),msg);
        prediction.curveSetting -= kFloat_0_5_ShotParameters * adjustment.x;
    }
    prediction.curveSetting *= kFloat_neg1_ShotParameters;
    float angle = -(kDriveLineShotCurveYawScale * ((CGolfBall*)source)->m_initCurve);
    float offset = kFloat_0_AutoSetup;
    angle *= kFloat_neg1_ShotParameters;
    Message m3;
    msg = DriveLineRecord_Init(&m3,0,kRequestLaunchYawOffsetAddress,&offset,0,0,0,0);
    DriveLineActor_DispatchMessageAndReturn(TaskActorTable_FindByName(TaskManager_GetCurrentChild(),swingTaleStrings+kDriveLineStrings_playerActor),msg);

    // Phase 2: form a launch vector with X pitch, then rotate around world Y.
    // Both angles are radians: CClub_AddClub converts
    // launchDegrees with kGolfDegreesToRadians (club_selection_and_shot_params.cpp); GetLaunchAngle
    // adds .06981317 radians per spin-setting unit outside mode 4. Matrix_SetAxisRotation
    // (math_matrix_quat_vec_utils.cpp) forwards radians to Math_*Approx (gtl_vector_matrix_math.cpp).
    // ShotCurveYawScale=.34906584 radians per curve-setting unit (~20 degrees);
    // offset is Player heading radians, with zero fallback if the actor is absent.
    // Preserve both existing sign operations, multiplication order and axes.
    Matrix direction; Matrix rotation; Matrix_SetAxisRotation(&rotation,(offset+angle),1);
    Matrix_SetAxisRotation(&direction,ClubSelection_GetLaunchAngle(InstanceTable_Get(),0),0);
    Vec horizontalDisplacement; Vec rotated; Vec scaled;
    __ml__FfRC7WVector(&scaled,&direction.axis,speed * (kFloat_0_1_ShotParameters * Math_AbsFloatConditional(prediction.curveSetting) + kFloat_1_DriveLine));
    Matrix_TransformPoint(&rotated,&scaled,&rotation);
    *(WVector*)(&prediction.velocity) = *(WVector*)&rotated;
    Bonus* extra = CAccState_GetEquipmentBonuses(PartyItemSlots_GetInstance());
    prediction.velocity.y *= (kFloat_1_DriveLine + extra->value);
    if (InstanceTable_GetTrajectoryKind((unsigned char*)InstanceTable_Get()) == kTrajectoryKind4WithLiftDecay) {
        PlayerState* player = GolfDoc_GetCurrent();
        float bonus = kFloat_0_5_ShotParameters * CSharedDoc_ComputeAuxPartModifier(SharedDoc_GetCurrent(),player->index,0);
        float power = (float)GolfPlayer_GetCurrentEntryByIndex(GolfDoc_GetCurrent()->index)->power;
        float ratio = (power + bonus) / kDriveLineSpecialShotPowerDivisor;
        prediction.lift = scale * (scale * (kDriveLineSpecialShotBaseLift + Math_MaxFloat(kFloat_0_AutoSetup,ratio)));
    } else prediction.lift = kFloat_0_AutoSetup;
    prediction.rollStartStep = -1;
    prediction.shotFlags = *(unsigned int*)&prediction.shotFlagsBits;
    prediction.spin = prediction.curveSetting * scale;
    // Phase 3: seed, integrate and sample; special-shot rules then modify state
    // for the following integration step. Clearance argument = radius * 2 (one
    // diameter); the shared scale is measured relative to the ball radius.
    RecordVector_ClearSize(&self->buffer);
    SixWordRecord_NoOpCleanup(&trajectorySample);
    *(WVector*)(&trajectorySample.position) = *(WVector*)(&prediction.position);
    *(WVector*)(&trajectorySample.velocity) = *(WVector*)(&prediction.velocity);
    SixWordRecordVector_PushBack(&self->buffer,&trajectorySample);
    groundClearanceRadiusScale = kDriveLineGroundClearanceRadiusScale;
    for (int i=0;i<kPredictionSteps;++i) {
        BallPhysics_IntegrateWindSpinAndDrag(self,&prediction);
        if (i%kSampleStepInterval==0) {
            *(WVector*)(&trajectorySample.position) = *(WVector*)(&prediction.position);
            *(WVector*)(&trajectorySample.velocity) = *(WVector*)(&prediction.velocity);
            SixWordRecordVector_PushBack(&self->buffer,&trajectorySample);
        }
        if (InstanceTable_GetTrajectoryKind((unsigned char*)InstanceTable_Get()) == kTrajectoryKind4WithLiftDecay) {
            if (prediction.shotFlags & kShotGroundRoll) {
                if (0 > prediction.rollStartStep) {
                    Vec point;
                    *(unsigned int*)&point.x = *(unsigned int*)&prediction.position.x;
                    *(unsigned int*)&point.y = *(unsigned int*)&prediction.position.y;
                    *(unsigned int*)&point.z = *(unsigned int*)&prediction.position.z;

                    prediction.position.y = Math_MaxFloat(prediction.position.y,(CQuadTree_ResolveGroundHeight(CQuadTree_GetInstance(),&point,(groundClearanceRadiusScale * CDrawDriveLine_GetBallRadius(&prediction)))));
                } else prediction.lift -= kFloat_OneThirtieth_DriveLine;
            } else prediction.lift -= kFloat_OneThirtieth_DriveLine * (kFloat_0_5_ShotParameters - kFloat_0_1_ShotParameters * prediction.spinSetting);
        }
        // Ground roll flattens velocity at i==1 (after the second integration).
        // A horizontal X/Z distance test selects the one-time roll transition;
        // speed is then chosen by variant, while later steps still integrate.
        if (prediction.shotFlags & kShotGroundRoll) {
            if (i==1) {
                prediction.velocity.y = kFloat_0_AutoSetup;
                Vec v;
                __ml__FfRC7WVector(&v,Normalize__7WVectorFv(&prediction.velocity),speed);
                *(WVector*)(&prediction.velocity) = *(WVector*)&v;
            }

            extern void __mi__FRC7WVectorRC7WVector(Vec*,const Vec*,const Vec*);
            __mi__FRC7WVectorRC7WVector(&horizontalDisplacement,&prediction.position,(Vec*)source);
            horizontalDisplacement.y = kFloat_0_AutoSetup;
            if (0 > prediction.rollStartStep) {
                float distance = GolfParameter_ResolveFloat(InstanceTable_Get(),0);
                float factor = Math_SquareRootApprox(scale);
                float limit = kDriveLineRollDistanceScale * (factor*distance-kDriveLineRollDistanceBias);
                if ((*(WVector*)&horizontalDisplacement).Magnitude()>limit) {
                    prediction.rollStartStep=i;
                (*(WVector*)(&prediction.velocity)).Normalize();

                    switch (TargetOwner_GetStateValue((TargetOwner*)InstanceTable_Get())) {
                    case 0:
                        *(WVector*)(&prediction.velocity) *= kDriveLineRollSpeedVariant0;
                        break;
                    case 1:
                        *(WVector*)(&prediction.velocity) *= kDriveLineRollSpeedVariant1;
                        break;
                    case 2:
                        *(WVector*)(&prediction.velocity) *= kDriveLineRollSpeedVariant2;
                        break;
                    default:
                        break;
                    }
                    *(WVector*)(&prediction.velocity) *= Math_SquareRootApprox(scale);
                    prediction.lift=kDriveLineRollTransitionLift;
                }
            }
        }
    }
}
}

extern "C" {
// Engine lists are pairs of 32-bit KEY/VALUE words, never pointer payloads.
enum RibbonStateConstants {
    kRibbonInitialStateCount = 15, kRibbonCombinerStateCount = 6,
    kRibbonSamplerStateCount = 5, kRibbonFinalStateCount = 11,
    kRibbonSourceBlendState = 13, kRibbonDestinationBlendState = 14,
    kRibbonBlendSourceAlpha = 4, kRibbonBlendOne = 1,
    kRibbonViewTransform = 0, kRibbonProjectionTransform = 1,
    kRibbonObjectTransform = 11, kRibbonTextureStage = 0,
    kRibbonVerticesPerSegment = 4, kRibbonBaseAlpha = 128,
    kRibbonOpaqueChannel = 255, kRibbonFadeSegments = 5,
    kRibbonSampleBytes = 24
};
struct RibbonStatePair { unsigned int key, value; };
struct RenderCommands { RibbonStatePair entries[kRibbonInitialStateCount]; };
struct RenderColors { RibbonStatePair entries[kRibbonCombinerStateCount]; };
struct RenderTex { RibbonStatePair entries[kRibbonSamplerStateCount]; };
struct RenderEnd { RibbonStatePair entries[kRibbonFinalStateCount]; };
// Defaults at 0x80479298, applied in this order (pairs are copied, not edited here):
// commands: lighting off, ambient/material source 1, lighting-key20=1,
// view-matrix-only off, no culling, blend+alpha-test on, alpha GREATER 0,
// src-alpha/inverse-src-alpha blending, depth test/write on with LEQUAL.
// colors: RGB and alpha multiply texture by previous (operation 0, args 2/0).
// tex parameters: min/mag/mip=1, wrap S/T=0 (backend maps keys 4/3/5/0/1).
// end: reapplies depth/blend/alpha/cull/view-only defaults; it is NOT a saved
// caller state. Keys 9/20 retain backend numeric identity where needed.
// The explicit source/destination settings below then select additive alpha
// (src-alpha,ONE). Change those two named values to change ribbon blending.
struct RenderDefaults { RenderCommands commands; RenderColors colors; RenderTex tex; RenderEnd end; };
extern const RenderDefaults kDriveLineRibbonRenderDefaults;
extern const float kDriveLineRibbonFieldOfView, kDriveLineRibbonAspectRatio, kDriveLineRibbonFarClip, kDriveLineRibbonRadiusScale, kDriveLineRibbonAlphaScale, kFloat_6_TrailAlpha;
struct RenderMatrix { float m[16]; };
extern RenderMatrix kIdentityMatrix4x4;
void Engine_SetRenderStateList(void*,int);
void Engine_SetTextureStageStateList(int,void*,int);
void Engine_SetTextureStageParameterList(int,void*,int);
void Matrix_ExpandAffineTo4x4(void*,RenderMatrix*);
void SetTransform(int,const RenderMatrix*);
void MtxPerspectiveFovLH(RenderMatrix*,float,float,float,float);
void gtl_GenericRenderer_SetProjectionMatrix(RenderMatrix*);
void Engine_SetCachedRenderState(int,int);
void Engine_SetType7Resource(int);
void Engine_SetTextureStageResource(int,void*);
void Engine_SetTexturePalette(int,void*);
void* GX_GetPrimitiveCount(int,int);
void Vector_CrossProduct(Vec*,const Vec*,const Vec*);
void __pl__FRC7WVectorRC7WVector(Vec*,const Vec*,const Vec*);
void __mi__FRC7WVectorRC7WVector(Vec*,const Vec*,const Vec*);
struct RenderResource {
    virtual void m0(); virtual void m1(); virtual void m2(); virtual void m3();
    virtual void m4(); virtual void m5(); virtual void m6(); virtual void m7();
    virtual void m8(); virtual void m9(); virtual void m10(); virtual void m11();
    virtual void m12(); virtual void m13(); virtual void m14(); virtual void m15();
    virtual void m16(); virtual void m17(); virtual void m18(); virtual void m19();
    virtual void m20(); virtual void m21(); virtual void m22(); virtual void m23();
    virtual void m24(); virtual void m25();
    virtual void* textureResourceForSlot(int); virtual void* paletteForSlot(int);
};
// WView+0xA4 is WGeneric3DR. Slots 26/27 are
// WGeneric3DR_GetTextureResourceHandle/GetTextureHandle (wgeneric3dr_immediate_renderer.cpp): argument
// is a texture-slot INDEX [0,2048) in a pointer-shaped ABI word, not a resource
// pointer. First returns the GX texture resource, second the palette object;
// both borrowed, no AddRef. Upper-bound miss yields null, negative is unchecked.
// CDrawDriveLine owns the ResourceManager_LoadBitmap ID and releases it in its
// destructor; keep it and the renderer slot alive until draw submission finishes.
// paletteForSlot supplies a palette, never a vertex/pixel format.
// No image bytes are copied by these getters.
struct RibbonVertex {
    float x, y, z;                     // +00, world-space position
    unsigned char red, green, blue, alpha; // +0C, RGBA8 physical byte order
    float u, v;                        // +10/+14, normalized UV
};
typedef char RibbonVertex_size[sizeof(RibbonVertex) == 0x18 ? 1 : -1];
// Vertex owner is CGeneTex3D (club_trail_and_ring_effects.cpp), installed by CDrawDriveLine's ctor
// (draw_drive_line.cpp): +00 count byte, +04 vptr, +08 owned CPU Vertex3 array. Requesting
// state/count 4 invokes AllocVertexBuffer(4); allocation clears four 0x18-byte
// records. Null allocation is not recoverable before memset. Slot map below:
// 1 selects FVF 0x11F001; 2 replaces/deletes the old array; 3 writes color with
// ARGB argument order into RGBA bytes; 4 copies INPUT XYZ to the vertex through
// Vec3_CopyToOutput(src,dst), despite that provider's misleading comment;
// 5 writes UV; 6 DrawPrimitiveUP borrows the array at stride 0x18. This caller's
// Vec by-value position passes a temporary XYZ address in the Wii ABI, consumed
// before return. No vertex/texture pointer ownership transfers during drawing.
// CGeneTex3D_Dtor releases CPU storage with __dla__; parent dtor destroys child.
struct RenderSystem { char pad[0xa4]; RenderResource* resource; };
extern RenderSystem* g_view;
struct RenderVertices {
    unsigned char count;
    virtual void destroy(int); virtual void selectVertexFormat();
    virtual int allocateVertices(int);
    virtual void writeColorARGB(unsigned char,unsigned char,unsigned char,unsigned char,unsigned char);
    virtual void writePosition(unsigned char,Vec);
    virtual void writeUv(unsigned char,float,float);
    virtual void drawPrimitive(int,void*);
    // Set one vertex's ARGB bytes only if the vertex resource contains it.
    void setcolor(unsigned char i,unsigned char a,unsigned char r,unsigned char g,unsigned char b) {
        if (count>i) writeColorARGB(i,a,r,g,b);
    }
    // Set one world-space vertex position when its index exists.
    void setposition(unsigned char i,Vec p) { if(count>i) writePosition(i,p); }
    // Set one vertex's normalized texture coordinates when its index exists.
    void setuv(unsigned char i,float u,float v) { if(count>i) writeUv(i,u,v); }
    // Draw the current vertex resource with the primitive count for this kind.
    void render(int kind) { if(count) drawPrimitive(kind,GX_GetPrimitiveCount(count,kind)); }
};
struct TrajectoryRender {
    char pad[0x10]; Sample* samples; int count; int unused; int ready;
    RenderVertices* vertices; void* texture; int dirty;
};
// Snapshot the initial render-state list before applying it.
inline void copycommands(RenderCommands& a,const RenderCommands& b) { a=b; }
// Snapshot the texture-stage state list before applying it.
inline void copycolors(RenderColors& a,const RenderColors& b) { a=b; }
// Snapshot the texture-stage parameter list before applying it.
inline void copytex(RenderTex& a,const RenderTex& b) { a=b; }
// Snapshot the final render-state list before applying it.
inline void copyend(RenderEnd& a,const RenderEnd& b) { a=b; }
#pragma opt_lifetimes off
// Draw prediction quads with width along world-up cross velocity, fading the final segments.
// self required; when ready, samples contains count valid 24-byte records, vertices
// supplies at least four writable vertices, and texture/view resource must exist.
// count 0/1 emits no strip but still installs render state. Clears dirty BEFORE
// testing ready, so a not-ready call consumes the pending draw. Zero/up-parallel
// velocity follows WVector::Normalize's fallback, not a camera-facing width rule.
void CDrawDriveLine_DrawRibbon(TrajectoryRender* self) { int count=kMaxRibbonSamples;
    // Consume dirty once even if not ready. Samples/vertices/texture are borrowed.
    // Phase 1: install the documented key/value tables, world->view and
    // left-handed perspective, with
    // identity world transform. Vertex writes follow RibbonVertex's schema above.
    RenderEnd end; RenderCommands commands; RenderMatrix view; RenderMatrix projection; Vec edges[2][2]; RenderTex tex;
    const RenderDefaults* defaults=&kDriveLineRibbonRenderDefaults;
    if (self->dirty == 0) return;
    self->dirty=0;
    if (!self->ready) return;
    copycommands(commands,defaults->commands);
    RenderColors colors; copycolors(colors,defaults->colors);
    copytex(tex,defaults->tex);
    Engine_SetRenderStateList(&commands,kRibbonInitialStateCount);
    Engine_SetTextureStageStateList(kRibbonTextureStage,&colors,kRibbonCombinerStateCount);
    Engine_SetTextureStageParameterList(kRibbonTextureStage,&tex,kRibbonSamplerStateCount);
    copyend(end,defaults->end);
    Engine_SetRenderStateList(&end,kRibbonFinalStateCount);

    Matrix_ExpandAffineTo4x4(((char*)&AsWView(g_view)->inputTransform),&view);
    SetTransform(kRibbonViewTransform,&view);
    MtxPerspectiveFovLH(&projection,kDriveLineRibbonFieldOfView,kDriveLineRibbonAspectRatio,kFloat_3_DriveLine,kDriveLineRibbonFarClip);
    SetTransform(kRibbonProjectionTransform,&projection);
    gtl_GenericRenderer_SetProjectionMatrix(&projection);
    SetTransform(kRibbonObjectTransform,&kIdentityMatrix4x4);
    Engine_SetCachedRenderState(kRibbonSourceBlendState,kRibbonBlendSourceAlpha);
    Engine_SetCachedRenderState(kRibbonDestinationBlendState,kRibbonBlendOne);
    RenderVertices* vertices=self->vertices;
    Engine_SetType7Resource(0);
    vertices->selectVertexFormat();
    void* texture=self->texture;
    Engine_SetTextureStageResource(0,g_view->resource->textureResourceForSlot((int)texture));
    Engine_SetTexturePalette(0,g_view->resource->paletteForSlot((int)texture));
    float ballRadius=GolfBall__Fv()->radius;

    if (self->count<=kMaxRibbonSamples) count=self->count;
    // Phase 2: four vertices are previous +/- side then current +/- side.
    // UVs are (0,1),(0,0),(1,1),(1,0). Base white alpha is 128. The tail
    // overrides alpha using a 0..255 scale; retain this existing discontinuity.
    for(int i=0;kRibbonVerticesPerSegment>i;++i) {
        self->vertices->setcolor(i,kRibbonBaseAlpha,kRibbonOpaqueChannel,kRibbonOpaqueChannel,kRibbonOpaqueChannel);
        self->vertices->setuv(i,(2&i)?kFloat_1_DriveLine:kFloat_0_AutoSetup,(i&1)?kFloat_0_AutoSetup:kFloat_1_DriveLine);
    }

    int offset=0;
    for (int i=0;i<count;++i,offset+=kRibbonSampleBytes) { Vec side;
        // Phase 3: horizontal half-width = normalize(up x velocity) * radius
        // * ribbon scale. Width is in WORLD space, not camera-facing pixels.
        // edges[i&1] alternates two cross-sections; each emits a four-vertex
        // strip (0x98) joining the previous sample to the current one.
        Sample* sample=(Sample*)((char*)self->samples+offset);
         Vec up;
        up.x=kFloat_0_AutoSetup; up.y=kFloat_1_DriveLine; up.z=kFloat_0_AutoSetup;
        Vector_CrossProduct(&side,&up,&sample->velocity);
        (*(WVector*)&side).Normalize();
        *(WVector*)&side *= kDriveLineRibbonRadiusScale*ballRadius;
        int slot=i&1;
        Vec plus;
        Vec& edge=edges[slot][0];
        __pl__FRC7WVectorRC7WVector(&plus,&sample->position,&side);
        *(WVector*)&edge = *(WVector*)&plus;
        Vec& other=edges[slot][1];
        Vec minus;
        __mi__FRC7WVectorRC7WVector(&minus,&sample->position,&side);
        *(WVector*)&other = *(WVector*)&minus;
        if(i) {
            if(kRibbonFadeSegments+i>=count) {
                // Final five segments: old edge alpha=255*(count-i)/6, new
                // edge alpha=255*(count-i-1)/6; the last new edge is transparent.
                int remaining=count-i;
                int alpha=(int)(kDriveLineRibbonAlphaScale*(float)remaining/kFloat_6_TrailAlpha);
                self->vertices->setcolor(0,alpha,255,255,255);
                self->vertices->setcolor(1,alpha,255,255,255);
                int next=remaining-1;
                if(next<=0) alpha=0;
                else alpha=(int)(kDriveLineRibbonAlphaScale*(float)next/kFloat_6_TrailAlpha);
                self->vertices->setcolor(2,alpha,255,255,255);
                self->vertices->setcolor(3,alpha,255,255,255);
            }
            Vec* previous=edges[(1^slot)];
            self->vertices->setposition(0,previous[0]);
            self->vertices->setposition(1,previous[1]);
            self->vertices->setposition(2,edge);
            self->vertices->setposition(3,other);
            self->vertices->render(kDrawTriangleStrip);
        }
    }
}
#pragma opt_lifetimes reset
}

