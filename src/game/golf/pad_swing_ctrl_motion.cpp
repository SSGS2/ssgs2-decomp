#include <decomp/static_init.h>
// Wii Remote swing playback / gauge publication, not the course ball integrator.
// Mod lookup (constants in u80226C9C_data.cpp unless another owner is named):
// - CPadSwingCtrl_vfunc_5 fast-forwards motion with skipPlayback=1: at most
//   kMaxSettleSteps (150) callbacks at kFloat_OneThirtieth_PadSwingSettleStep
//   (0.03333333507180214 seconds each). This is a synchronous bounded playback
//   loop, not 150 rendered frames or 150 golf-ball collision steps.
// - vfunc_8/IsSwingAnimationComplete multiply elapsed seconds by
//   kFloat_30_PadSwingMotion (30 animation frames/second); completion also adds
//   kMotionCompletionFrameOffset (15 frames) and the selected range/timing offset.
// - DampedMoveTowardTarget advances once per CALL; 10/5 response/damping calls,
//   velocity limit 2, max change 0.5, duration scale 1 or 1.2. Do not change only
//   the seconds-to-frames scalar when changing update cadence.
// - SelectSwingMotion/GetThresholdIndex: ten s_swingMotionThresholds; names in
//   swingMotionNames, 30 fixed 36-byte entries. Selected-ID timing rows are
//   is the CHARACTER type-ID's low 26-bit index, with no indirection through a
//   display-order list. GetCharacterTimingFrames below contains the full row/
//   column contract; ItemInventory_FindCharacterData in golf_player_actor.cpp uses that
//   same selected ID. Character names/localization are not table indices.
// - vfunc_2 gauge mapping: 149 + 360*power; impact lower bound 114, power upper
//   bound 509. These are cursor coordinates, not screen resolution or world units.
// All self/animationTarget pointers are required and borrowed during playback.
// CPadSwingCtrl_Ctor owns two controllers; dtor releases them. No callback target
// ownership transfers. Preserve phase result codes, update order, float operations,
// repeated singleton/phase queries, manual-animation toggles, and early returns.
// Safe tuning ranges beyond table domains are not established. For golf physics
// use the separate shot/trajectory/collision units.
// Shared headers: CPadSwingCtrl::accelerationController at +0 owns the
// acceleration controller; its interface vptr is +0x58. The integer at
// CPadSwingCtrlMotion::motionPhase (+0) stores phase 0..4, with interface vptr +0x44.
// Constructor, destructor and vfunc_1/AdvanceMotionPhase establish those meanings.
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
// CPadSwingCtrlMotion_GetCharacterTimingFrames (0x80227870): manager +0xB90 is the golf
// document; its +0x3C1 byte selects the player passed to CGolfDoc_GetItemSelection.
// Translation unit 0x80226C9C-0x802283AC (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "classes/CPadSwingCtrl.h"
#include "classes/CPadSwingCtrlMotion.h"

// Borrow the motion-controller storage view without transferring ownership.
static inline CPadSwingCtrlMotion* AsCPadSwingCtrlMotion(void* p) { return (CPadSwingCtrlMotion*)p; }

// Borrow the swing-controller storage view without transferring ownership.
static inline CPadSwingCtrl* AsCPadSwingCtrl(void* p) { return (CPadSwingCtrl*)p; }

// Indices are verified against the 36-byte entries in swingMotionNames.
enum SwingMotionIndex {
    kMotionWoodPrepare = 0,
    kMotionWoodPowerPrepare = 1,
    kMotionWoodDoublePowerPrepare = 2,
    kMotionWoodTakeback = 3,
    kMotionWoodTakebackBurn = 4,
    kMotionWoodSmall = 5,
    kMotionWoodMedium = 6,
    kMotionWoodLarge = 7,
    kMotionWoodPower = 8,
    kMotionWoodDoublePower = 9,
    kMotionIronPrepare = 10,
    kMotionIronPowerPrepare = 11,
    kMotionIronTakeback = 12,
    kMotionIronTakebackBurn = 13,
    kMotionIronSmall = 14,
    kMotionIronMedium = 15,
    kMotionIronLarge = 16,
    kMotionIronPower = 17,
    kMotionWedgePrepare = 18,
    kMotionWedgePowerPrepare = 19,
    kMotionWedgeTakeback = 20,
    kMotionWedgeTakebackBurn = 21,
    kMotionWedgeMedium = 22,
    kMotionWedgeLarge = 23,
    kMotionWedgePower = 24,
    kMotionPutterPrepare = 25,
    kMotionPutterTakeback = 26,
    kMotionPutterTakebackBurn = 27,
    kMotionPutterMedium = 28,
    kMotionPutterLarge = 29,
};
enum ClubAnimationCategory { kWood = 0, kIron = 1, kWedge = 2, kPutter = 3 };
// Motion-state cycle verified in event_fx_sound_and_pad_swing_accele.cpp::CPadSwingCtrlMotion_AdvanceMotionPhase.
// These phase values select callbacks; they are distinct from shot result codes.
enum SwingMotionPhase { kSwingIdle = 0, kSwingReturning = 1, kSwingForward = 2,
                        kSwingSettling = 3, kSwingComplete = 4 };
// Commit results verified in pad_swing_ctrl_and_com_think.cpp::CPadSwingCtrl_UpdateSettlingPhase. Other callback
// results are forwarded unchanged (including 5=cancel and 7=finishing).
enum SwingCommitResult { kSwingPrimaryCommitted = 4, kSwingAlternateCommitted = 6 };
enum { kMotionNameStride = 0x24, kMotionThresholdCount = 10, kMaxSettleSteps = 150 };
// This removes the six high type-kind bits; the result is the selected type ID's
// low 26-bit index, NOT the item-kind/category (which would be typeId >> 26).
enum { kSelectionTypeIndexMask = 0x03FFFFFF };
// Code 6 is both the default classification output and the forced-swing value.
// Its gameplay label is unresolved; do not equate it with motion phase 6.
enum { kDefaultSwingKindCode = 6 };
struct SwingShotInfo { char reserved[0xA]; unsigned char clubCategory; };
struct SwingInstanceState {
    void* vtable;
    SwingShotInfo* shot;
    char reserved[0x3C];
    int powerShotMode;
};
typedef char SwingInstancePowerOffset[((int)&((SwingInstanceState*)0)->powerShotMode == 0x44) ? 1 : -1];
typedef char SwingClubCategoryOffset[((int)&((SwingShotInfo*)0)->clubCategory == 0xA) ? 1 : -1];
// Read the selected shot/context through its local ABI view.
static inline SwingInstanceState* SwingInstance(void* p) { return (SwingInstanceState*)p; }

extern "C" {
extern const float kFloat_6_2831855;
extern float s_padSwingAngleCache;
extern const float kFloat_0_PadSwing;
extern const float kFloat_neg1_PadSwing;
extern const float kFloat_OneThirtieth_PadSwingSettleStep; // Fixed settle-step duration: 1/30 second.
extern const float kFloat_0_805E0078;
extern const float kFloat_30_PadSwingMotion;
extern const float kMotionCompletionFrameOffset; // Additional animation-completion delay, read as a float word (15).
extern const float kFloat_1_PadSwingMotion;
extern const float kFloat_1_2_PadSwingDurationScale; // Duration scaling factor: 1.2.
extern const float kFloat_10_PadSwingResponseFrames; // Target response time before duration scaling: 10 frames.
extern const float kFloat_5_PadSwingDampingFrames; // Damping period before duration scaling: 5 frames.
extern const float kFloat_2_PadSwingVelocityLimit; // Maximum smoothed velocity before duration scaling: 2.
extern const float kFloat_0_5_PadSwing_805E008C;
extern const float kFloat_0_03_PadSwingVelocityDeadband; // Positive velocity damping deadband: 0.03.
extern const float kFloat_neg0_03_PadSwingVelocityDeadband; // Negative velocity damping deadband: -0.03.
extern const float kFloat_0_05_PadSwingPositionDeadband; // Positive position-error deadband: 0.05.
extern const float kFloat_neg0_05_PadSwingPositionDeadband; // Negative position-error deadband: -0.05.
extern const float kFloat_0_06_PadSwingBrakeVelocity; // Velocity threshold for braking at the target: 0.06.
extern const float kFloat_0_2_PadSwingMotion;
extern const float kFloat_neg0_2_PadSwing;
extern const float kFloat_149_PadSwing; // Gauge power origin in cursor units.
extern const float kFloat_360_PadSwing; // Gauge span: power=1 maps 149+360=509.
extern const float kFloat_30_PadSwing_805E00D8;
extern const float kFloat_114_PadSwing; // Minimum impact cursor coordinate.
extern const float kFloat_509_PadSwing; // Maximum published power cursor coordinate.
extern const float kFloat_0_3_PadSwing;
extern float s_swingMotionThresholds[];
extern char swingMotionNames[];
extern CSharedDoc* g_sharedDoc;
extern int s_characterSwingTimingFrames[][kMotionThresholdCount];
void* InstanceTable_Get();
void* CGolfDoc_GetItemSelection(void*, unsigned char);
void CPadSwingAccele_Ctor(void*);
void CPadSwingCtrlMotion_Ctor(void*);
int EventTiming_IsDurationScalingEnabled();
struct MotionAnimationTarget {
    virtual void m0(int); virtual void m1(); virtual void m2(); virtual void m3();
    virtual void m4(); virtual void m5(); virtual void m6(); virtual void m7();
    virtual void m8(); virtual void m9(); virtual void m10(); virtual void m11();
    virtual void m12(); virtual void m13(); virtual void m14(); virtual void m15();
    virtual void m16(); virtual void m17(); virtual void m18(); virtual void m19();
    virtual void m20(); virtual void m21(); virtual void m22(); virtual void m23();
    virtual void m24(); virtual void m25(); virtual void m26(); virtual void m27();
    virtual void m28(); virtual void m29(); virtual void m30(); virtual void m31();
    virtual void m32(); virtual void m33(); virtual void m34(); virtual void m35();
    virtual void m36(); virtual void m37(); virtual void m38(); virtual void m39();
    virtual void m40(); virtual void setManualAnimation(int); virtual void m42();
    virtual void m43(); virtual void setAnimationPosition(const char*, float); virtual void m45();
};

// ABI slot map (dummy mN declarations reserve slots; they are not general APIs):
// 0 deleting destructor(deleteFlag), 1 phase query, 4 reset(animationTarget),
// 8 settle/play(animationTarget, powerFraction, deltaSeconds, skipPlayback),
// 18 phase-reset hook; 20/21 choose motion/threshold. Target/context pointers borrow.
struct SwingMotionInterface {
    char pad[0x44];
    virtual int m0(int);
    virtual int m1();
    virtual int m2();
    virtual int m3();
    virtual void m4(void*);
    virtual int m5();
    virtual void m6(void*);
    virtual int m7();
    virtual int m8(void*, float, float, int);
    virtual int m9();
    virtual int m10();
    virtual int m11();
    virtual int m12();
    virtual int m13();
    virtual int m14();
    virtual int m15();
    virtual int m16();
    virtual int m17();
    virtual int m18();
    virtual int m19();
    virtual int selectSwingMotion();
    virtual int thresholdIndex();
    virtual int m22();
    virtual int m23();
};

// Owned acceleration controller: slot 0 destroys, 1 resets. Other signatures are
// per-call ABI views; do not use an unverified mN slot through this placeholder.
struct AccelerationOwnerInterface {
    char pad[0x3924];
    virtual void m0(int); virtual void m1(); virtual void m2(); virtual void m3();
    virtual void m4(float, int);
    virtual void m5(); virtual void m6(); virtual void m7(); virtual void m8();
    virtual void m9(); virtual void m10(); virtual void m11(); virtual void m12();
    virtual void m13(); virtual void m14(); virtual void m15(); virtual float m16();
};

struct SwingAnimationTarget {
    virtual void m0(); virtual void m1(); virtual void m2(); virtual void m3();
    virtual void m4(); virtual void m5(); virtual void m6(); virtual void m7();
    virtual void m8(); virtual void m9(); virtual void m10(); virtual void m11();
    virtual void m12(); virtual void m13(); virtual void m14(); virtual void m15();
    virtual void m16(); virtual void m17(); virtual void m18(); virtual void m19();
    virtual void m20(); virtual void m21(); virtual void m22(); virtual void m23();
    virtual void m24(); virtual void m25(); virtual void m26(); virtual void m27();
    virtual void m28(); virtual void m29(); virtual void m30(); virtual void m31();
    virtual void m32(); virtual void m33(); virtual void m34(); virtual void m35();
    virtual void m36(); virtual void m37(); virtual void m38(); virtual void m39();
    virtual void m40();
    virtual void setManualAnimation(int);
    virtual void m42(); virtual void m43(); virtual void setAnimationPosition(); virtual void m45();
    virtual void queryAnimationPosition(float*, const char*);
    virtual void playAnimation(const char*, int, float);
    virtual int isAnimationPlaying(const char*);
};
// Test whether the selected normal swing is the large wood animation.
int CPadSwingCtrlMotion_vfunc_14(SwingMotionInterface* self) { return self->selectSwingMotion() == kMotionWoodLarge; }

// Map the selected normal swing animation to its small/medium/large size class.
int CPadSwingCtrlMotion_GetSwingSizeClass(SwingMotionInterface* self) {
    int motionIndex = self->selectSwingMotion();
    switch (motionIndex) {
    case kMotionWoodLarge: case kMotionIronLarge: case kMotionWedgeLarge: return 2;
    case kMotionWoodMedium: case kMotionIronMedium: case kMotionWedgeMedium: return 1;
    case kMotionWoodSmall: case kMotionIronSmall: return 0;
    default: return 0;
    }
}

// Return the threshold for a validated animation-threshold index.
// self required; invalid indices fall back to entry zero, not a clamped endpoint.
float CPadSwingCtrlMotion_GetSelectedMotionThreshold(SwingMotionInterface* self) {
	int thresholdIndex = self->thresholdIndex();
    if (thresholdIndex < 0) thresholdIndex = 0;
    else if (thresholdIndex >= kMotionThresholdCount) thresholdIndex = 0;
    return s_swingMotionThresholds[thresholdIndex];
}

// Report that the motion controller supports this optional swing capability.
int CPadSwingCtrlMotion_vfunc_13() { return 1; }
// Report support for this optional swing capability (always enabled here).
int CPadSwingCtrlMotion_vfunc_16() { return 1; }
// Report support for this optional swing capability (always enabled here).
int CPadSwingCtrlMotion_vfunc_18() { return 1; }

struct PrepareAnimationTarget {
    virtual void m0(); virtual void m1(); virtual void m2(); virtual void m3();
    virtual void m4(); virtual void m5(); virtual void m6(); virtual void m7();
    virtual void m8(); virtual void m9(); virtual void m10(); virtual void m11();
    virtual void m12(); virtual void m13(); virtual void m14(); virtual void m15();
    virtual void m16(); virtual void m17(); virtual void m18(); virtual void m19();
    virtual void m20(); virtual void m21(); virtual void m22(); virtual void m23();
    virtual void m24(); virtual void m25(); virtual void m26(); virtual void m27();
    virtual void m28(); virtual void m29(); virtual void m30(); virtual void m31();
    virtual void m32(); virtual void m33(); virtual void m34(); virtual void m35();
    virtual void m36(); virtual void m37(); virtual void m38(); virtual void m39();
    virtual void m40(); virtual void setManualAnimation(int); virtual void m42();
    virtual void m43(); virtual void setAnimationPosition(const char*, float);
};

// Set the club/power-shot preparation animation to its first frame in manual mode.
// First ABI argument unused; object required. Returns 1 unconditionally; unsupported
// categories keep the default wood-prepare index rather than reporting failure.
int CPadSwingCtrlMotion_SetManualPrepareAnimation(void*, void* object) {
    int index = kMotionWoodPrepare;
    void* state = InstanceTable_Get();
    if (SwingInstance(state)->powerShotMode != 0) {
        state = InstanceTable_Get();
        unsigned char type = SwingInstance(state)->shot->clubCategory;
        switch (type) {
        case kWood:
            state = InstanceTable_Get();
            if (SwingInstance(state)->powerShotMode == 2) index = kMotionWoodDoublePowerPrepare;
            else index = kMotionWoodPowerPrepare;
            break;
        case kIron:
            index = kMotionIronPowerPrepare;
            break;
        case kWedge:
            index = kMotionWedgePowerPrepare;
            break;
        }
    } else {
        state = InstanceTable_Get();
        unsigned char type = SwingInstance(state)->shot->clubCategory;
        switch (type) {
        case kWood:
            index = kMotionWoodPrepare;
            break;
        case kIron:
            index = kMotionIronPrepare;
            break;
        case kWedge:
            index = kMotionWedgePrepare;
            break;
        }
    }
    PrepareAnimationTarget* o = (PrepareAnimationTarget*)object;
    o->setManualAnimation(1);
    o->setAnimationPosition((const char*)swingMotionNames + index * kMotionNameStride, kFloat_0_805E0078);
    return 1;
}

// Wait for the power-shot animation to start; force playback after the timeout.
// self/object required; delta seconds. Strict >1-second timeout, no >= replacement.
// Returns 1 even while waiting; animationStarted is the state, not the return value.
int CPadSwingCtrlMotion_StartPowerShotAnimation(void* self, void* object, float delta) {
    int index = kMotionWoodPrepare;
    if (AsCPadSwingCtrlMotion(self)->animationStarted == 0) {
        void* state = InstanceTable_Get();
        unsigned char type = SwingInstance(state)->shot->clubCategory;
        switch (type) {
        case kWood:
            state = InstanceTable_Get();
            if (SwingInstance(state)->powerShotMode == 2) index = kMotionWoodDoublePower;
            else index = kMotionWoodPower;
            break;
        case kIron:
            index = kMotionIronPower;
            break;
        case kWedge:
            index = kMotionWedgePower;
            break;
        }
        ((SwingAnimationTarget*)object)->setManualAnimation(0);
        const char* entry = swingMotionNames + index * kMotionNameStride;
        if (((SwingAnimationTarget*)object)->isAnimationPlaying(entry))
            AsCPadSwingCtrlMotion(self)->animationStarted = 1;
        if (AsCPadSwingCtrlMotion(self)->animationStarted == 0) {
            AsCPadSwingCtrlMotion(self)->animationStartWaitElapsed += delta;
            if (AsCPadSwingCtrlMotion(self)->animationStartWaitElapsed > kFloat_1_PadSwingMotion) {
                ((SwingAnimationTarget*)object)->playAnimation(entry, 2, kFloat_0_805E0078);
                AsCPadSwingCtrlMotion(self)->animationStarted = 1;
            }
        }
    }
    return 1;
}

// Play the club preparation animation using the normal or power-shot playback mode.
// First ABI argument unused; object required. Modes 0/3 are passed through to the
// animation target (their full semantics belong to that API). Always returns 1.
// Complete selection policy (SwingInstance and its shot must be live):
// powerShotMode  wood(0)                  iron(1)             wedge(2)
// 0              WoodPrepare              IronPrepare         WedgePrepare
// 2              WoodDoublePowerPrepare   IronPowerPrepare    WedgePowerPrepare
// any other !=0  WoodPowerPrepare         IronPowerPrepare    WedgePowerPrepare
// Putter(3) and unknown club bytes fall back to WoodPrepare IN BOTH branches;
// they do not select PutterPrepare here. Established power modes are 0=normal,
// 1=single, 2=double (CGolfPlayer_SetPowerMode path in golf_player_actor.cpp); all other ints
// follow the nonzero fallback. Every path clears manual animation, starts at
// 0.0, and chooses target playback mode 0 for normal / 3 for nonzero power.
// Repeated singleton reads are retained; do not cache/reorder them in a mod.
int CPadSwingCtrlMotion_PlayPrepareAnimation(void*, void* object) {
    int index = kMotionWoodPrepare;
    void* state = InstanceTable_Get();
    if (SwingInstance(state)->powerShotMode != 0) {
        state = InstanceTable_Get();
        unsigned char type = SwingInstance(state)->shot->clubCategory;
        switch (type) {
        case kWood:
            state = InstanceTable_Get();
            if (SwingInstance(state)->powerShotMode == 2) index = kMotionWoodDoublePowerPrepare;
            else index = kMotionWoodPowerPrepare;
            break;
        case kIron:
            index = kMotionIronPowerPrepare;
            break;
        case kWedge:
            index = kMotionWedgePowerPrepare;
            break;
        }
    } else {
        state = InstanceTable_Get();
        unsigned char type = SwingInstance(state)->shot->clubCategory;
        switch (type) {
        case kWood:
            index = kMotionWoodPrepare;
            break;
        case kIron:
            index = kMotionIronPrepare;
            break;
        case kWedge:
            index = kMotionWedgePrepare;
            break;
        }
    }
    ((SwingAnimationTarget*)object)->setManualAnimation(0);
    state = InstanceTable_Get();
    if (SwingInstance(state)->powerShotMode != 0) {
        ((SwingAnimationTarget*)object)->playAnimation(swingMotionNames + index * kMotionNameStride, 3, kFloat_0_805E0078);
    } else {
        ((SwingAnimationTarget*)object)->playAnimation(swingMotionNames + index * kMotionNameStride, 0, kFloat_0_805E0078);
    }
    return 1;
}

// Completion callbacks on the motion receiver. Vtable evidence:
// phase1_class_tables::__vt__19CPadSwingCtrlMotion slots 17 and 20.
// slot17 GetCharacterTimingFrames -> int animation frames to subtract from end;
// slot20 SelectSwingMotion -> SwingMotionIndex from powerFraction. The retained
// target/power args on slot17 are unused by the concrete timing implementation.
// Both calls borrow the target. Neither owns it, advances time, or plays a clip.
struct SwingPlaybackCompletionCallbacks {
    char pad[0x44];
    virtual void d0(); virtual void d1(); virtual void d2(); virtual void d3();
    virtual void d4(); virtual void d5(); virtual void d6(); virtual void d7();
    virtual void d8(); virtual void d9(); virtual void d10(); virtual void d11();
    virtual void d12(); virtual void d13(); virtual void d14(); virtual void d15();
    virtual void d16();
    virtual unsigned int characterTimingFrames(void*, float);
    virtual void d18(); virtual void d19();
    virtual int selectSwingMotion(void*, float);
};

struct AnimationEndQuery {
    virtual void m0(); virtual void m1(); virtual void m2(); virtual void m3();
    virtual void m4(); virtual void m5(); virtual void m6(); virtual void m7();
    virtual void m8(); virtual void m9(); virtual void m10(); virtual void m11();
    virtual void m12(); virtual void m13(); virtual void m14(); virtual void m15();
    virtual void m16(); virtual void m17(); virtual void m18(); virtual void m19();
    virtual void m20(); virtual void m21(); virtual void m22(); virtual void m23();
    virtual void m24(); virtual void m25(); virtual void m26(); virtual void m27();
    virtual void m28(); virtual void m29(); virtual void m30(); virtual void m31();
    virtual void m32(); virtual void m33(); virtual void m34(); virtual void m35();
    virtual void m36(); virtual void m37(); virtual void m38(); virtual void m39();
    virtual void m40(); virtual void setManualAnimation(); virtual void m42(); virtual void m43();
    virtual void setAnimationPosition(); virtual void m45();
    virtual int queryAnimationEnd(float*, const char*);
};

// Report completion after the selected animation end plus its normal/power-shot delay.
// self/object required; delta seconds, value forwarded to motion callbacks.
// The range query writes a frame position; its status is ignored and the local
// defaults to zero. completionElapsed advances even if the query does not write.
int CPadSwingCtrlMotion_IsSwingAnimationComplete(void* self, void* object, float value, float delta) {
    int result = 0;
    int index = kMotionWoodPrepare;
    float offset = kFloat_0_805E0078;
    float position = kFloat_0_805E0078;
    void* state = InstanceTable_Get();
    if (SwingInstance(state)->powerShotMode != 0) {
        state = InstanceTable_Get();
        unsigned char type = SwingInstance(state)->shot->clubCategory;
        switch (type) {
        case kWood:
            state = InstanceTable_Get();
            if (SwingInstance(state)->powerShotMode == 2) index = kMotionWoodDoublePower;
            else index = kMotionWoodPower;
            break;
        case kIron:
            index = kMotionIronPower;
            break;
        case kWedge:
            index = kMotionWedgePower;
            break;
        }
        offset = kFloat_30_PadSwingMotion;
    } else {
        index = ((SwingPlaybackCompletionCallbacks*)self)->selectSwingMotion(object, value);
        int count = ((SwingPlaybackCompletionCallbacks*)self)->characterTimingFrames(object, value);
        offset = offset - (float)count;
    }
    const char* entry = swingMotionNames + index * kMotionNameStride;
    ((AnimationEndQuery*)object)->queryAnimationEnd(&position, entry);
    position += offset;
    AsCPadSwingCtrlMotion(self)->completionElapsed += delta;
    if (kFloat_30_PadSwingMotion * AsCPadSwingCtrlMotion(self)->completionElapsed >= kMotionCompletionFrameOffset + position) result = 1;
    return result;
}

// Select swing size from club category, normalized power and motion thresholds.
// powerFraction convention is [0,1], no clamp; self->motionLimit uses animation
// coordinates. Strict > comparisons assign equality to the smaller animation.
// Unsupported club categories leave the default large-wood motion selection.
int CPadSwingCtrlMotion_SelectSwingMotion(void* self, float powerFraction) {
    int result = kMotionWoodLarge;
    float positionLimit = AsCPadSwingCtrlMotion(self)->motionLimit;
    void* state = InstanceTable_Get();
    unsigned char clubCategory = SwingInstance(state)->shot->clubCategory;
    switch (clubCategory) {
    case kWood:
        if (powerFraction * positionLimit > s_swingMotionThresholds[1]) result = kMotionWoodLarge;
        else if (powerFraction * positionLimit > s_swingMotionThresholds[0]) result = kMotionWoodMedium;
        else result = kMotionWoodSmall;
        break;
    case kIron:
        if (powerFraction * positionLimit > s_swingMotionThresholds[4]) result = kMotionIronLarge;
        else if (powerFraction * positionLimit > s_swingMotionThresholds[3]) result = kMotionIronMedium;
        else result = kMotionIronSmall;
        break;
    case kWedge: result = powerFraction * positionLimit > s_swingMotionThresholds[6] ? kMotionWedgeLarge : kMotionWedgeMedium; break;
    case kPutter: result = powerFraction * positionLimit > s_swingMotionThresholds[8] ? kMotionPutterLarge : kMotionPutterMedium; break;
    }
    return result;
}

// Read the selected character type-ID-index timing row in animation frames.
// The shared ABI name says category, but low 26 bits are an INDEX, not kind bits.
// Required selection record starts with uint32 typeId; its index must be [0,8).
// thresholdIndex is validated/falls back to zero; the selected-ID row is unchecked.
// 10 signed 32-bit frame counts per row; row stride=40 bytes, whole table=320.
// Columns: wood small/medium/large, iron small/medium/large, wedge medium/large,
// putter medium/large, exactly GetThresholdIndex's 0..9 mapping.
// Selected type-ID low index -> baseline row (animation frames, 30 frames/sec):
// 0: 5,7,8,7,8,11,8,11,7,10       1: 6,8,9,7,7,8,6,9,6,9
// 2: 7,12,9,8,7,8,7,10,14,10     3: 8,8,14,7,10,15,7,10,16,9
// 4: 5,7,14,6,6,13,4,9,5,8       5: 6,8,14,7,7,11,6,11,3,6
// 6: 6,6,12,6,7,11,6,9,6,10      7: 6,9,13,7,7,11,7,7,13,8
// No character-name lookup is required to edit an equipped character: inspect
// that selection's typeId & 0x03ffffff, then edit its row. Adding index 8+ requires
// extending the table AND all character-selection consumers; no row clamp exists.
// IsSwingAnimationComplete subtracts this count only in its normal-power branch:
// increasing a cell moves completion earlier; changing this is not motion speed.
int CPadSwingCtrlMotion_GetCharacterTimingFrames(SwingMotionInterface* a) {
    int thresholdIndex = a->thresholdIndex();
    int clampedIndex = thresholdIndex;
    if (thresholdIndex < 0) clampedIndex = 0;
    else if (thresholdIndex >= kMotionThresholdCount) clampedIndex = 0;
    thresholdIndex = clampedIndex;
    void* golfDoc = (void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    unsigned char playerIndex = ((CGolfDoc*)golfDoc)->currentPlayer;
    void* characterSelection = CGolfDoc_GetItemSelection(golfDoc, playerIndex);
    unsigned int characterIndex = *(unsigned int*)characterSelection & kSelectionTypeIndexMask;
    return s_characterSwingTimingFrames[characterIndex][thresholdIndex];
}

struct VPadSwing {
    char pad[0x44];
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5();
    virtual void v8(void*, int, int, int, float);
    virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18();
    virtual int v19();
    virtual void v20();
    virtual float v21();
    virtual int v22(void*, float);
};

// Settle the smoothed motion, start its swing animation, then wait for completion.
// deltaSeconds feeds the elapsed timers; comparisons multiply by 30 to obtain
// animation frames. swingValue is forwarded to the motion-selection callback;
// skipPlayback suppresses starting the animation, but still advances the state.
// Returns 0 while settling/waiting and 1 when the frame threshold has elapsed.
int CPadSwingCtrlMotion_vfunc_8(void* self, void* object, float swingValue, float deltaSeconds, int skipPlayback) {
    int result = 0;
    float scale = AsCPadSwingCtrlMotion(self)->motionLimit;
    if (AsCPadSwingCtrlMotion(self)->motionSettled == 0) {
        // Phase 1: approach the threshold position (motion units, not seconds).
        float targetPosition = ((VPadSwing*)self)->v21();
        ((VPadSwing*)self)->v8(object, 0, 0, skipPlayback, targetPosition / scale);
        if (kFloat_30_PadSwingMotion * AsCPadSwingCtrlMotion(self)->settlingElapsed >= kFloat_1_PadSwingMotion) {
            if (AsCPadSwingCtrlMotion(self)->motionPosition - targetPosition < kFloat_0_2_PadSwingMotion &&
                AsCPadSwingCtrlMotion(self)->motionPosition - targetPosition > kFloat_neg0_2_PadSwing) {
                if (AsCPadSwingCtrlMotion(self)->motionVelocity < kFloat_0_2_PadSwingMotion &&
                    AsCPadSwingCtrlMotion(self)->motionVelocity > kFloat_neg0_2_PadSwing) {
                    AsCPadSwingCtrlMotion(self)->motionSettled = 1;
                    AsCPadSwingCtrlMotion(self)->motionPosition = kFloat_0_805E0078;
                    AsCPadSwingCtrlMotion(self)->motionVelocity = kFloat_0_805E0078;
                } else if (AsCPadSwingCtrlMotion(self)->motionVelocity < kFloat_0_805E0078) {
                    AsCPadSwingCtrlMotion(self)->motionPosition = kFloat_0_805E0078;
                    AsCPadSwingCtrlMotion(self)->motionSettled = 1;
                    AsCPadSwingCtrlMotion(self)->motionVelocity = kFloat_0_805E0078;
                }
            }
        }
        AsCPadSwingCtrlMotion(self)->settlingElapsed += deltaSeconds;
    } else if (AsCPadSwingCtrlMotion(self)->animationStarted == 0) {
        // Phase 2: start once after settling; subsequent calls only advance time.
        if (skipPlayback == 0) {
            ((SwingAnimationTarget*)object)->setManualAnimation(0);
            int index = ((VPadSwing*)self)->v22(object, swingValue);
            ((SwingAnimationTarget*)object)->playAnimation(swingMotionNames + index * kMotionNameStride, 2, kFloat_0_805E0078);
        }
        AsCPadSwingCtrlMotion(self)->animationStarted = 1;
    } else {
        // Phase 3: wait for the selected frame threshold at 30 animation frames/s.
        AsCPadSwingCtrlMotion(self)->animationElapsed += deltaSeconds;
        int completionFrames = ((VPadSwing*)self)->v19();
        float frameThreshold = (float)completionFrames;
        if (kFloat_30_PadSwingMotion * AsCPadSwingCtrlMotion(self)->animationElapsed >= frameThreshold)
            result = 1;
    }
    return result;
}

struct MotionState {
    unsigned int phase; // +0: motion-state code, not the interface vptr at +0x44.
    float position, limit, velocity;
};

// Smooth the target swing fraction with bounded acceleration, damping and optional braking.
// Units: position and limit share the motion's animation units (position runs 0..limit, limit =
// full backswing/follow range); velocity is position units per CALL (one call = one frame step,
// not scaled by elapsed time); targetFraction and the return value are 0..1 fractions of limit.
// Per call, in order:
//   1. acceleration a = (target - position) / responseFrames, clamped to +-maxVelocityChange;
//   2. damping: velocity *= (dampingFrames - 1) / dampingFrames, applied always, or (when
//      dampWhenMoving) only while |velocity| or |position/limit - targetFraction| exceeds its deadband;
//   3. velocity += a, then clamped to +-velocityLimit; position += velocity, clamped to 0..limit
//      (only overshooting a bound zeroes velocity; exact arrival retains it before step 4);
//   4. brakeAtTarget: once position >= target and velocity is above a small threshold, velocity is
//      halved (the same 0.5 constant as the acceleration clamp's base factor);
//   5. unless skipAnimationUpdate, write the position into the selected swing animation.
// The four time constants are divided/multiplied by durationScale (1.2 when duration scaling is
// enabled, else 1): a larger scale SHORTENS response/damping call counts, increases
// velocity/acceleration limits, and decreases the damping retention multiplier.
// self/object required, limit > 0; targetFraction convention [0,1], no validation.
// skipAnimationUpdate suppresses position submission but still enables manual mode
// and changes position/velocity. Return is the bounded position divided by limit.
// Example (scale 1): target fraction 1.0 and position 0 give a = min(limit / 10, 0.5).
// This call damps velocity when required, adds a, then clamps velocity to [-2,2] before moving.
float CPadSwingCtrlMotion_DampedMoveTowardTarget(MotionState* self, MotionAnimationTarget* object, float targetFraction, int dampWhenMoving, int brakeAtTarget, int skipAnimationUpdate) {
    float positionLimit, targetPosition, durationScale, responseFrames, difference, dampingFrames, velocityChange, maxVelocityChange, velocityLimit;
    float zero, velocity;
    object->setManualAnimation(1);
    positionLimit = self->limit;
    targetPosition = targetFraction * positionLimit;
    durationScale = EventTiming_IsDurationScalingEnabled() ? kFloat_1_2_PadSwingDurationScale : kFloat_1_PadSwingMotion;
    responseFrames = kFloat_10_PadSwingResponseFrames / durationScale;
    velocityLimit = kFloat_2_PadSwingVelocityLimit * durationScale;
    maxVelocityChange = kFloat_0_5_PadSwing_805E008C * durationScale;
    dampingFrames = kFloat_5_PadSwingDampingFrames / durationScale;
    velocityChange = (targetPosition - self->position) / responseFrames;
    if (velocityChange >= kFloat_0_805E0078 && velocityChange > maxVelocityChange) velocityChange = maxVelocityChange;
    else if (velocityChange < kFloat_0_805E0078 && velocityChange < -maxVelocityChange) velocityChange = -maxVelocityChange;
    if (dampWhenMoving) {
        float normalized = self->position / positionLimit - targetFraction;
        if (self->velocity > kFloat_0_03_PadSwingVelocityDeadband ||
            self->velocity < kFloat_neg0_03_PadSwingVelocityDeadband ||
            normalized > kFloat_0_05_PadSwingPositionDeadband ||
            normalized < kFloat_neg0_05_PadSwingPositionDeadband) {
            float damping = (dampingFrames - kFloat_1_PadSwingMotion) / dampingFrames;
            self->velocity *= damping;
        }
    } else {
        float damping = (dampingFrames - kFloat_1_PadSwingMotion) / dampingFrames;
        self->velocity *= damping;
    }
    velocity = self->velocity;
    zero = kFloat_0_805E0078;
    velocity += velocityChange;
    self->velocity = velocity;
    if (velocity >= zero && velocity > velocityLimit)
        self->velocity = velocityLimit;
    else if (self->velocity < kFloat_0_805E0078 && self->velocity < -velocityLimit)
        self->velocity = -velocityLimit;
    float position = self->position + self->velocity;
    self->position = position;
    if (position > positionLimit) {
        float zero = kFloat_0_805E0078;
        self->position = positionLimit;
        self->velocity = zero;
    } else if (position < kFloat_0_805E0078) {
        float zero = kFloat_0_805E0078;
        self->position = zero;
        self->velocity = zero;
    }
    if (brakeAtTarget && self->position >= targetPosition && self->velocity > kFloat_0_06_PadSwingBrakeVelocity)
        self->velocity *= kFloat_0_5_PadSwing_805E008C;
    if (!skipAnimationUpdate) {
        int index = ((SwingMotionInterface*)self)->m22() * kMotionNameStride;
        float value = self->position;
        object->setAnimationPosition(swingMotionNames + index, value);
    }
    return self->position / positionLimit;
}

struct V26F34 {
    char pad[0x44];
    virtual void d0(); virtual void d1(); virtual void d2(); virtual void d3();
    virtual void d4(); virtual void d5();
    virtual void d6(void*, int, int, int, float);
    virtual void d7(); virtual void d8(); virtual void d9();
    virtual void d10(); virtual void d11(); virtual void d12(); virtual void d13();
    virtual void d14(); virtual void d15(); virtual void d16(); virtual void d17();
    virtual void d18();
    virtual float d19();
};

// Drive the smoothed motion toward the current threshold and return its normalized position.
float CPadSwingCtrlMotion_UpdateMotionToThreshold(void* self, void* animationTarget) {
    float scale = AsCPadSwingCtrlMotion(self)->motionLimit;
    float thresholdPosition = ((V26F34*)self)->d19();
    float normalized = thresholdPosition / scale;
    ((V26F34*)self)->d6(animationTarget, 1, 1, 0, normalized);
    return AsCPadSwingCtrlMotion(self)->motionPosition / scale;
}

// Published gauge coordinates, not world positions or angles. powerCursor and
// roundedCursor start at floor(149+360*power). A committed non-putter shot changes
// continuous/rounded to impact/ceil(impact), saves the full power coordinate, and
// may reduce/clamp powerCursor. Fields not assigned in a path retain caller state.
struct OutData {
    float powerCursor;
    float impactCursor;
    float continuousCursor;
    float roundedCursor;
    int pad10;
    unsigned char powerClamped;
    char pad15[3];
    int phaseDirection;
    float impactOrigin;
    int pad20;
    int pad24;
    int swingKind;
    float fullPowerCursor;
};

// Same acceleration object as AccelerationOwnerInterface; invoked slots are
// update(seconds, nonPutterFlag) and slot 14 = CPadSwingAccele_vfunc_14, verified
// in phase1_class_tables.cpp. It returns the handedness-adjusted direction angle
// produced by atan in event_fx_sound_and_pad_swing_accele.cpp::CPadSwingAccele_AnalyzeCapturedSamples (radians).
struct AccelerationUpdateInterface {
    char pad[0x3924];
    virtual void m0(); virtual void m1();
    virtual void update(float, int);
    virtual void m3(); virtual void m4(); virtual void m5();
    virtual void m6(); virtual void m7(); virtual void m8();
    virtual void m9(); virtual void m10(); virtual void m11();
    virtual void m12(); virtual void m13();
    virtual float directionAngleRadians();
};

struct SwingPhaseInterface {
    char pad[0x58];
// Slots 21..25: idle, returning (vfunc_22), forward, settling and finish in pad_swing_ctrl_and_com_think.cpp. actor is the borrowed
// Player animation interface; state is a VALUE from Player message 0x15. Only the returning callback also receives
// the caller's gauge block. Slot 28 writes the selected power fraction and classification code.
    virtual void m0(); virtual void m1(); virtual void m2();
    virtual void m3(); virtual void m4(); virtual void m5();
    virtual void m6(); virtual void m7(); virtual void m8();
    virtual void m9(); virtual void m10(); virtual void m11();
    virtual void m12(); virtual void m13(); virtual void m14();
    virtual void m15(); virtual void m16(); virtual void m17();
    virtual void m18(); virtual void m19(); virtual void m20();
    // Slots 21..25: idle, returning (vfunc_22), forward, settling, finish in
    // pad_swing_ctrl_and_com_think.cpp. actor is the borrowed Player animation interface; state is a
    // VALUE from Player message 0x15, not a second pointer/context. Only the
    // returning callback also receives the caller's gauge block.
    virtual int updateIdle(SwingAnimationTarget*, int);
    virtual int updateReturning(OutData*, SwingAnimationTarget*, int);
    virtual int updateForward(float, SwingAnimationTarget*, int);
    virtual int updateSettling(float, SwingAnimationTarget*, int);
    virtual int updateFinish(float, SwingAnimationTarget*, int);
    virtual void m26(); virtual void m27();
    // Slot 28 writes the selected power fraction and classification code.
    virtual void amount(float*, int*);
};

// Advance the current swing phase and publish power/impact cursor values for the shot.
// Phase dispatch (motionController->m1() value -> callback; result is the callback's return):
//   idle -> updateIdle(actor,state), returning -> updateReturning(out,actor,state),
//   forward -> updateForward(delta,..), settling -> updateSettling(delta,..),
//   complete -> updateFinish(delta,..) and return immediately (nothing below runs).
// Output fields (OutData) written after dispatch:
//   always (non-complete): phaseDirection (idle 0, forward/settling -1, other +1), continuousCursor
//     and roundedCursor/powerCursor from swingPower (floored 149 + 360 * swingPower);
//   only when result is a committed code and the club is not a putter: fullPowerCursor, swingKind,
//     impactCursor (>= minimum 114), continuousCursor/roundedCursor re-set to the impact cursor
//     (rounded up), powerCursor clamped to 509 with powerClamped = 1 at/above the maximum.
// Retained inputs: impactOrigin is read, not modified. Early completion skips the gauge refresh and
// the swingStartRequested / powerThresholdReached update.
// Caller contract: CGolfRule_ProcessMotionSwingInput (golf_rule_round_flow_and_shot_sim.cpp) looks up the
// current task's "Player" actor, synchronously queries message 0x15 into an int,
// and passes that actor/state here. animationTarget must supply the Player
// animation slots described by SwingAnimationTarget (41 manual toggle, 44 seek,
// 46 query, 47 play, 48 playing). It stays live through this call AND subsequent
// playback updates; no pointer is retained by these phase methods. Animation
// resources/range names must remain installed throughout the swing. out is the
// caller-owned gauge block beginning at CGolfDoc::shotPowerGaugeX, size 0x30;
// returning phase may pass it to acceleration preparation. No alias with actor
// or controller storage. delta is seconds. animationState is passed unchanged;
// callbacks test ==3 to suppress cancel input and normal manual-motion updates,
// using the power-shot playback path instead. Other int values take the normal
// branch here; obtain it from Player, do not substitute a pointer or a phase ID.
// CGolfPlayer_HandleMsg case kPlayerGetAnimationMode (golf_player_actor.cpp) reads the
// current player's 0xB4-stride animation record at +0x60. Value 3 is
// kAnimationEnteringPowerReady there; this callback does not set that state.
// Return the phase callback's result unchanged. Complete-phase
// dispatch returns early and does not refresh gauge fields or consume start flags.
int CPadSwingCtrl_UpdateSwingPhaseAndGauge(void* self, OutData* out, float delta, SwingAnimationTarget* animationTarget, int animationState) {
    int result = 0;
    // Phase 1: acceleration is updated even if the phase callback returns early.
    int useNonPutterAcceleration = 1;
    void* controller = InstanceTable_Get();
    if (SwingInstance(controller)->shot->clubCategory == kPutter)
        useNonPutterAcceleration = 0;
    ((AccelerationUpdateInterface*)*(void**)self)->update(delta, useNonPutterAcceleration);
    int mode = ((SwingMotionInterface*)AsCPadSwingCtrl(self)->motionController)->m1();
    switch (mode) {
    case kSwingIdle:
        result = ((SwingPhaseInterface*)self)->updateIdle(animationTarget, animationState);
        break;
    case kSwingReturning:
        result = ((SwingPhaseInterface*)self)->updateReturning(out, animationTarget, animationState);
        break;
    case kSwingForward:
        result = ((SwingPhaseInterface*)self)->updateForward(delta, animationTarget, animationState);
        break;
    case kSwingSettling:
        result = ((SwingPhaseInterface*)self)->updateSettling(delta, animationTarget, animationState);
        break;
    case kSwingComplete:
        return ((SwingPhaseInterface*)self)->updateFinish(delta, animationTarget, animationState);
    default:
        break;
    }

    // Phase 2: publish a gauge snapshot for all non-complete callback results.
    // Phase can change during its callback. Query again for gauge direction:
    // idle=0, forward/settling=-1, other active phases=+1.
    int status = ((SwingMotionInterface*)AsCPadSwingCtrl(self)->motionController)->m1();
    if (status == kSwingIdle)
        out->phaseDirection = 0;
    else {
        status = ((SwingMotionInterface*)AsCPadSwingCtrl(self)->motionController)->m1();
        if (status == kSwingForward || ((SwingMotionInterface*)AsCPadSwingCtrl(self)->motionController)->m1() == kSwingSettling)
            out->phaseDirection = -1;
        else
            out->phaseDirection = 1;
    }
    out->continuousCursor = kFloat_149_PadSwing + kFloat_360_PadSwing * AsCPadSwingCtrl(self)->swingPower;
    float flooredPowerCursor = floor(out->continuousCursor);
    out->roundedCursor = flooredPowerCursor;
    out->powerCursor = flooredPowerCursor;
    // Phase 3: only committed NON-PUTTER shots publish impact/classification.
    // Other paths leave impactCursor/fullPowerCursor/swingKind/powerClamped alone.
    // Derive the selected power and impact coordinates. Forced swings
    // preserve full power and use impactOrigin directly, bypassing motion phase.
    if (result == kSwingPrimaryCommitted || result == kSwingAlternateCommitted) {
        controller = InstanceTable_Get();
        if (SwingInstance(controller)->shot->clubCategory != kPutter) {
            if (AsCPadSwingCtrl(self)->forcedSwingActive == 0) {
                int shotKind = kDefaultSwingKindCode;
                float selectedPowerFraction = kFloat_0_PadSwing;
                ((SwingPhaseInterface*)self)->amount(&selectedPowerFraction, &shotKind);
                out->fullPowerCursor = out->powerCursor;
                float powerSpan = kFloat_360_PadSwing * AsCPadSwingCtrl(self)->swingPower;
                out->powerCursor = floor(kFloat_149_PadSwing + selectedPowerFraction * powerSpan);
                out->swingKind = shotKind;
                void* player = *(void**)self;
                float swingDirectionRadians = ((AccelerationUpdateInterface*)player)->directionAngleRadians();
                // Convert the captured direction angle at 30 cursor units/radian;
                // this separate 30.0 word is NOT the animation frames/second scalar.
                float t = kFloat_30_PadSwing_805E00D8 * swingDirectionRadians + out->impactOrigin;
                out->impactCursor = t;
                if (out->impactCursor < kFloat_114_PadSwing) out->impactCursor = kFloat_114_PadSwing;
            } else {
                out->fullPowerCursor = out->powerCursor;
                out->swingKind = kDefaultSwingKindCode;
                out->impactCursor = out->impactOrigin;
            }
            // Impact rounds UP; power above rounded DOWN. Keep those policies distinct.
            float impactCursor = out->impactCursor;
            out->continuousCursor = impactCursor;
            float roundedImpactCursor = ceil(impactCursor);
            out->roundedCursor = roundedImpactCursor;
            if (out->powerCursor >= kFloat_509_PadSwing) {
                out->powerCursor = kFloat_509_PadSwing;
                out->powerClamped = 1;
            } else out->powerClamped = 0;
        }
    }
    // Phase 4: consume the start request, then latch threshold-reached once true.
    AsCPadSwingCtrl(self)->swingStartRequested = 0;
    if (AsCPadSwingCtrl(self)->swingPower >= kFloat_0_3_PadSwing ||
        SwingInstance(InstanceTable_Get())->shot->clubCategory == kPutter)
        AsCPadSwingCtrl(self)->powerThresholdReached = 1;
    return result;
}

// Select the club-specific takeback setup animation.
int CPadSwingCtrlMotion_GetTakebackMotionID() {
    int result = kMotionWoodPrepare;
    unsigned char x = SwingInstance(InstanceTable_Get())->shot->clubCategory;
    switch (x) {
    case kWood: result = kMotionWoodTakeback; break;
    case kIron: result = kMotionIronTakeback; break;
    case kWedge: result = kMotionWedgeTakeback; break;
    case kPutter: result = kMotionPutterTakeback; break;
    }
    return result;
}

// Translate the chosen swing animation into one of ten threshold-table entries.
int CPadSwingCtrlMotion_GetThresholdIndex(SwingMotionInterface* self) {
    int result = 0;
    switch (self->selectSwingMotion()) {
        case kMotionWoodSmall: result = 0; break;
        case kMotionWoodMedium: result = 1; break;
        case kMotionWoodLarge: result = 2; break;
        case kMotionIronSmall: result = 3; break;
        case kMotionIronMedium: result = 4; break;
        case kMotionIronLarge: result = 5; break;
        case kMotionWedgeMedium: result = 6; break;
        case kMotionWedgeLarge: result = 7; break;
        case kMotionPutterMedium: result = 8; break;
        case kMotionPutterLarge: result = 9; break;
        default: break;
        }
    return result;
}

// Select the club-specific burning takeback setup animation.
int CPadSwingCtrlMotion_GetTakebackBurnMotion() {
    int result = kMotionWoodPrepare;
    unsigned char x = SwingInstance(InstanceTable_Get())->shot->clubCategory;
    switch (x) {
    case kWood: result = kMotionWoodTakebackBurn; break;
    case kIron: result = kMotionIronTakebackBurn; break;
    case kWedge: result = kMotionWedgeTakebackBurn; break;
    case kPutter: result = kMotionPutterTakebackBurn; break;
    }
    return result;
}

// Report that this swing-controller lifecycle hook succeeded.
int CPadSwingCtrl_vfunc_6() { return 1; }

extern char __vt__13CPadSwingCtrl[];
extern void __dl__FPv(void*);
// Destroy the owned acceleration and motion controllers, then optionally free this wrapper.
// Null accepted; flag > 0 deletes self, so the return then dangles. Targets
// previously passed for animation are borrowed and are not destroyed here.
void* CPadSwingCtrl_dtor(void* self, int flag) {
    if (self != 0) {
        AccelerationOwnerInterface* x = (AccelerationOwnerInterface*)*(void**)self;
        AsCPadSwingCtrl(self)->interfaceVtable = &__vt__13CPadSwingCtrl;
        if (x != 0) {
            x->m0(1);
        }
        SwingMotionInterface* y = (SwingMotionInterface*)AsCPadSwingCtrl(self)->motionController;
        if (y != 0) {
            y->m0(1);
        }
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

// Per-call view of the same motion vtable: slot 8 has the source argument order
// (target, skipPlayback, powerFraction, deltaSeconds). SwingMotionInterface lists
// the integer after the floats. Both supply r4=target, r5=skipPlayback,
// f1=powerFraction, f2=deltaSeconds in the original ABI; preserve that mapping.
// Slot 14 queries the large-wood selection; slot 4 resets using the borrowed target.
struct V28224 {
    char pad[0x44];
    virtual void m0(); virtual void m1(); virtual void m2(); virtual void m3();
    virtual void m4(void*);
    virtual void m5(); virtual void m6(); virtual void m7();
    virtual int m8(void*, int, float, float);
    virtual void m9(); virtual void m10(); virtual void m11();
    virtual void m12(); virtual void m13();
    virtual int m14(void*, float);
};

// Advance a swing animation and reset its motion object when playback completes.
// self/animationTarget required; value is elapsed seconds forwarded as f2.
// Returns 1 only for callback result exactly 1, otherwise 0. The large-wood
// predicate is refreshed every call, before the completion reset.
int CPadSwingCtrl_vfunc_4(void* self, void* animationTarget, float value) {
    int done = 0;
    int result = ((*(V28224**)&AsCPadSwingCtrl(self)->motionController))->m8(
        animationTarget, 0, AsCPadSwingCtrl(self)->swingPower, value);
    // Cache the large-wood animation predicate; the shared field name is provisional.
    AsCPadSwingCtrl(self)->largeWoodSwingSelected = ((*(V28224**)&AsCPadSwingCtrl(self)->motionController))->m14(
        animationTarget, AsCPadSwingCtrl(self)->swingPower);
    if (result == 1) {
        done = 1;
        ((*(V28224**)&AsCPadSwingCtrl(self)->motionController))->m4(animationTarget);
    }
    return done;
}

// Run up to 150 fixed-time motion steps, stopping when the motion object finishes.
// self/animationTarget required. Returns the ZERO-BASED successful callback index
// (0 means the first call finished), or 150 if exhausted. Any nonzero callback
// result ends the loop, unlike vfunc_4's exact-1 test. Reset runs only on completion.
// Changing the bound changes the work budget; changing the step changes elapsed
// animation time per attempt. Neither is a render-rate control or ball timestep.
int CPadSwingCtrl_vfunc_5(void* self, void* animationTarget) {
    int i = 0;
    do {
        SwingMotionInterface* x = (SwingMotionInterface*)AsCPadSwingCtrl(self)->motionController;
        if (x->m8(animationTarget, AsCPadSwingCtrl(self)->swingPower, kFloat_OneThirtieth_PadSwingSettleStep, 1)) {
            ((SwingMotionInterface*)AsCPadSwingCtrl(self)->motionController)->m4(animationTarget);
            break;
        }
        ++i;
    } while (i < kMaxSettleSteps);
    return i;
}

// Initialize this unit's cached full-circle angle.
void PadSwing_InitializeAngleCache() { s_padSwingAngleCache = kFloat_6_2831855; }

// Construct the owned acceleration controller in its full 0x3928-byte allocation.
struct Alloc3928 { char data[0x3928];
    // Construct the owned Wii Remote acceleration controller in its full ABI size.
    Alloc3928() { CPadSwingAccele_Ctor(this); }
};
// Construct the owned motion controller in its full 0x48-byte allocation.
struct Alloc48 { char data[0x48];
    // Construct the owned swing motion controller in its full ABI size.
    Alloc48() { CPadSwingCtrlMotion_Ctor(this); }
};
// Allocate the acceleration and motion controllers and reset swing/ramp state.
// self points to caller-provided CPadSwingCtrl storage; returns self. No explicit
// allocation failure handling in this constructor. Destroy with CPadSwingCtrl_dtor.
void* CPadSwingCtrl_Ctor(void* self) {
    AsCPadSwingCtrl(self)->interfaceVtable = __vt__13CPadSwingCtrl;
    Alloc3928* first = new Alloc3928;
    *(Alloc3928**)self = first;
    Alloc48* second = new Alloc48;
    float neg = kFloat_neg1_PadSwing;
    float zero = kFloat_0_PadSwing;
    int zeroInt = 0;
    int negone = -1;
    (*(Alloc48**)&AsCPadSwingCtrl(self)->motionController) = second;
    AsCPadSwingCtrl(self)->swingPower = zero;
    AsCPadSwingCtrl(self)->smoothedMotionFraction = zero;
    AsCPadSwingCtrl(self)->primarySwingButtonArmed = zeroInt;
    AsCPadSwingCtrl(self)->alternateSwingButtonArmed = zeroInt;
    AsCPadSwingCtrl(self)->largeWoodSwingSelected = zeroInt;
    AsCPadSwingCtrl(self)->swingSizeClass = zeroInt;
    AsCPadSwingCtrl(self)->swingStartRequested = zeroInt;
    AsCPadSwingCtrl(self)->finishElapsed = neg;
    AsCPadSwingCtrl(self)->rampTarget = zero;
    AsCPadSwingCtrl(self)->rampFrames = negone;
    AsCPadSwingCtrl(self)->rampFrame = zeroInt;
    AsCPadSwingCtrl(self)->primarySwingRequested = zeroInt;
    AsCPadSwingCtrl(self)->alternateSwingRequested = zeroInt;
    AsCPadSwingCtrl(self)->powerThresholdReached = zeroInt;
    AsCPadSwingCtrl(self)->rampRunning = zeroInt;
    // These two adjustment words are also cleared when enforcing the minimum impact width.
    AsCPadSwingCtrl(self)->unk_50 = zeroInt;
    AsCPadSwingCtrl(self)->unk_54 = zeroInt;
    return self;
}

// Reset the owned acceleration controller and clear pending swing/ramp requests.
int CPadSwingCtrl_ResetSwingRequestsAndRampState(void* self) {
    ((AccelerationOwnerInterface*)*(void**)self)->m1();
    float zero = kFloat_0_PadSwing;
    int zeroInt = 0;
    enum { kSwingStateUnset = 6 };
    int unsetState = kSwingStateUnset;
    int negone = -1;
    AsCPadSwingCtrl(self)->powerThresholdReached = zeroInt;
    AsCPadSwingCtrl(self)->swingStartRequested = zeroInt;
    AsCPadSwingCtrl(self)->rampTarget = zero;
    AsCPadSwingCtrl(self)->rampFrames = negone;
    AsCPadSwingCtrl(self)->rampFrame = zeroInt;
    AsCPadSwingCtrl(self)->rampRunning = zeroInt;
    AsCPadSwingCtrl(self)->primarySwingRequested = zeroInt;
    AsCPadSwingCtrl(self)->alternateSwingRequested = zeroInt;
    AsCPadSwingCtrl(self)->forcedSwingActive = zeroInt;
    // Independent readiness gates consumed by vfunc_20 (pad_swing_ctrl_and_com_think.cpp):
    // +4C is an inhibit code: exactly 6 returns not-ready before testing +48.
    // +48 is a range code: for nonnegative codes, 0..3 admits readiness and
    // >=4 rejects it (the bit expression also admits negative values).
    // Resetting BOTH to 6 independently closes both gates. They are not motion
    // phases or swingKind. No other named source writer is established; their
    // gameplay labels/admitted external writer domain remain unknown. A new
    // caller should use reset/playback APIs rather than write guessed states.
    AsCPadSwingCtrl(self)->readinessRangeCode = unsetState;
    AsCPadSwingCtrl(self)->readinessInhibitCode = unsetState;
    return 1;
}

struct V227C44 {
virtual void m0();
virtual void m1();
virtual void m2();
virtual void m3();
virtual void m4();
virtual void m5();
virtual void m6();
virtual void m7();
virtual void m8();
virtual void m9();
virtual void m10();
virtual void m11();
virtual void m12();
virtual void m13();
virtual void m14();
virtual void m15();
virtual void m16();
virtual void m17();
virtual void m18();
virtual void m19();
virtual void m20();
virtual void m21();
virtual void m22();
virtual void m23();
virtual void m24();
virtual void m25();
virtual void m26();
virtual void m27();
virtual void m28();
virtual void m29();
virtual void m30();
virtual void m31();
virtual void m32();
virtual void m33();
virtual void m34();
virtual void m35();
virtual void m36();
virtual void m37();
virtual void m38();
virtual void m39();
virtual void m40();
virtual void setManualAnimation();
virtual void m42(void*);
virtual void m43(void*);
};
// Pass the first owned motion channel to the target's first channel hook.
void CPadSwingCtrlMotion_vfunc_24(void* self, void* animationTarget) {
    reinterpret_cast<V227C44*>(animationTarget)->m42(((char*)&AsCPadSwingCtrlMotion(self)->firstOwnedMotionChannel));
}
// Pass the first owned motion channel to the target's second channel hook.
void CPadSwingCtrlMotion_vfunc_25(void* self, void* animationTarget) {
    reinterpret_cast<V227C44*>(animationTarget)->m43(((char*)&AsCPadSwingCtrlMotion(self)->firstOwnedMotionChannel));
}

// Invoke the motion controller's phase-reset hook.
int CPadSwingCtrl_vfunc_3(void* self) {
    SwingMotionInterface* x = (SwingMotionInterface*)AsCPadSwingCtrl(self)->motionController;
    x->m18();
    return 1;
}

struct V3924 {
    char pad[0x3924];
    virtual void m0(); virtual void m1(); virtual void m2();
    virtual void m3(); virtual void m4(); virtual void m5();
    virtual void m6(); virtual void m7();
};
// Invoke the acceleration controller's reset hook.
int CPadSwingCtrl_vfunc_7(void* self) {
    V3924* x = (V3924*)*(void**)self;
    x->m4();
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462228", _ctors_u80226C9C_0, PadSwing_InitializeAngleCache);
#pragma pop
