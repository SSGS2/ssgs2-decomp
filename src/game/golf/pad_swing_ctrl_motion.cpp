// Wii Remote swing playback and the swing gauge.
//
// CPadSwingCtrlMotion smooths the swing position toward a target, picks the swing animation for the club and
// the swing power, and times when the animation is done. CPadSwingCtrl owns one of those plus an acceleration
// controller; here it runs the current swing phase each frame and publishes the power and impact cursor values.
//
// Animation times are in frames at 30 frames per second. The damped move advances once per call, not per second.
#include <decomp/local_class.h>
#include <decomp/static_init.h>
#include "sdk_protos.h"
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#include "classes/CPadSwingCtrl.h"
#include "classes/CPadSwingCtrlMotion.h"
#include "classes/CPadSwingAccele.h"

// Indices into swingMotionNames, which holds 36-byte animation names.
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
enum ClubAnimationCategory {
    kWood = 0,
    kIron = 1,
    kWedge = 2,
    kPutter = 3
};

// Values of CPadSwingCtrlMotion::motionPhase, which selects the phase callback.
enum SwingMotionPhase {
    kSwingIdle = 0,
    kSwingReturning = 1,
    kSwingForward = 2,
    kSwingSettling = 3,
    kSwingComplete = 4
};

// Phase callback results that commit the shot. Other results are passed through unchanged.
enum SwingCommitResult {
    kSwingPrimaryCommitted = 4,
    kSwingAlternateCommitted = 6
};

enum {
    kMotionNameStride = 0x24,
    kMotionThresholdCount = 10,
    kMaxSettleSteps = 150,
    kSelectionTypeIndexMask = 0x03FFFFFF, // the character type ID without its six kind bits
    kDefaultSwingKindCode = 6,            // the swing kind before classification, and the forced-swing kind
    kSwingStateUnset = 6
};

// Float constants. Their values are still linked from the original data section, so they are referenced by
// symbol and given readable names below.
extern "C" {
extern const float kFloat_6_2831855;
extern float s_padSwingAngleCache;
extern const float kFloat_0_PadSwing;
extern const float kFloat_neg1_PadSwing;
extern const float kFloat_OneThirtieth_PadSwingSettleStep;
extern const float kFloat_0_805E0078;
extern const float kFloat_30_PadSwingMotion;
extern const float kMotionCompletionFrameOffset;
extern const float kFloat_1_PadSwingMotion;
extern const float kFloat_1_2_PadSwingDurationScale;
extern const float kFloat_10_PadSwingResponseFrames;
extern const float kFloat_5_PadSwingDampingFrames;
extern const float kFloat_2_PadSwingVelocityLimit;
extern const float kFloat_0_5_PadSwing_805E008C;
extern const float kFloat_0_03_PadSwingVelocityDeadband;
extern const float kFloat_neg0_03_PadSwingVelocityDeadband;
extern const float kFloat_0_05_PadSwingPositionDeadband;
extern const float kFloat_neg0_05_PadSwingPositionDeadband;
extern const float kFloat_0_06_PadSwingBrakeVelocity;
extern const float kFloat_0_2_PadSwingMotion;
extern const float kFloat_neg0_2_PadSwing;
extern const float kFloat_149_PadSwing;
extern const float kFloat_360_PadSwing;
extern const float kFloat_30_PadSwing_805E00D8;
extern const float kFloat_114_PadSwing;
extern const float kFloat_509_PadSwing;
extern const float kFloat_0_3_PadSwing;
extern float s_swingMotionThresholds[];
extern char swingMotionNames[];
extern CSharedDoc* g_sharedDoc;
extern int s_characterSwingTimingFrames[][kMotionThresholdCount];
extern char __vt__13CPadSwingCtrl[];

void* InstanceTable_Get();
int EventTiming_IsDurationScalingEnabled();
void __dl__FPv(void*);
}

#define kZero kFloat_0_805E0078
#define kOne kFloat_1_PadSwingMotion
#define kHalf kFloat_0_5_PadSwing_805E008C
#define kTwoPi kFloat_6_2831855
#define kFramesPerSecond kFloat_30_PadSwingMotion // animation frames per second
#define kSettleStepSeconds kFloat_OneThirtieth_PadSwingSettleStep
#define kDurationScale kFloat_1_2_PadSwingDurationScale // 1.2 when duration scaling is enabled
#define kResponseFrames kFloat_10_PadSwingResponseFrames
#define kDampingFrames kFloat_5_PadSwingDampingFrames
#define kVelocityLimit kFloat_2_PadSwingVelocityLimit
#define kVelocityDeadband kFloat_0_03_PadSwingVelocityDeadband
#define kNegativeVelocityDeadband kFloat_neg0_03_PadSwingVelocityDeadband
#define kPositionDeadband kFloat_0_05_PadSwingPositionDeadband
#define kNegativePositionDeadband kFloat_neg0_05_PadSwingPositionDeadband
#define kBrakeVelocity kFloat_0_06_PadSwingBrakeVelocity
#define kSettleTolerance kFloat_0_2_PadSwingMotion
#define kNegativeSettleTolerance kFloat_neg0_2_PadSwing
#define kGaugeOrigin kFloat_149_PadSwing // gauge cursor at zero power
#define kGaugeSpan kFloat_360_PadSwing   // cursor units from zero to full power
#define kCursorUnitsPerRadian kFloat_30_PadSwing_805E00D8
#define kMinimumImpactCursor kFloat_114_PadSwing
#define kMaximumPowerCursor kFloat_509_PadSwing
#define kPowerThreshold kFloat_0_3_PadSwing

// The player's animation interface, as called here. Slots with no name are unused.
#define UNUSED_SLOT(n) virtual void Slot##n() DECOMP_LOCAL_VIRTUAL
struct SwingAnimationTarget {
    UNUSED_SLOT(0);
    UNUSED_SLOT(1);
    UNUSED_SLOT(2);
    UNUSED_SLOT(3);
    UNUSED_SLOT(4);
    UNUSED_SLOT(5);
    UNUSED_SLOT(6);
    UNUSED_SLOT(7);
    UNUSED_SLOT(8);
    UNUSED_SLOT(9);
    UNUSED_SLOT(10);
    UNUSED_SLOT(11);
    UNUSED_SLOT(12);
    UNUSED_SLOT(13);
    UNUSED_SLOT(14);
    UNUSED_SLOT(15);
    UNUSED_SLOT(16);
    UNUSED_SLOT(17);
    UNUSED_SLOT(18);
    UNUSED_SLOT(19);
    UNUSED_SLOT(20);
    UNUSED_SLOT(21);
    UNUSED_SLOT(22);
    UNUSED_SLOT(23);
    UNUSED_SLOT(24);
    UNUSED_SLOT(25);
    UNUSED_SLOT(26);
    UNUSED_SLOT(27);
    UNUSED_SLOT(28);
    UNUSED_SLOT(29);
    UNUSED_SLOT(30);
    UNUSED_SLOT(31);
    UNUSED_SLOT(32);
    UNUSED_SLOT(33);
    UNUSED_SLOT(34);
    UNUSED_SLOT(35);
    UNUSED_SLOT(36);
    UNUSED_SLOT(37);
    UNUSED_SLOT(38);
    UNUSED_SLOT(39);
    UNUSED_SLOT(40);
    virtual void SetManualAnimation(int manual) DECOMP_LOCAL_VIRTUAL;                      // slot 41
    virtual void AttachChannel(void* channel) DECOMP_LOCAL_VIRTUAL;                        // slot 42
    virtual void AttachChannelAlternate(void* channel) DECOMP_LOCAL_VIRTUAL;               // slot 43
    virtual void SetAnimationPosition(const char* name, float frame) DECOMP_LOCAL_VIRTUAL; // slot 44
    UNUSED_SLOT(45);
    virtual int QueryAnimationEnd(float* frame, const char* name) DECOMP_LOCAL_VIRTUAL;            // slot 46
    virtual void PlayAnimation(const char* name, int mode, float startFrame) DECOMP_LOCAL_VIRTUAL; // slot 47
    virtual int IsAnimationPlaying(const char* name) DECOMP_LOCAL_VIRTUAL;                         // slot 48
};

// The current shot's data and swing options. Belongs to the instance table's class once that has a header.
struct SwingShotInfo {
    char pad_00[0xA];
    unsigned char clubCategory; // 0xA
};
struct SwingInstanceState {
    void* vtbl;          // 0x0
    SwingShotInfo* shot; // 0x4
    char pad_08[0x3C];
    int powerShotMode; // 0x44: 0 normal, 1 single power shot, 2 double power shot
};
typedef char SwingInstancePowerOffset[((int)&((SwingInstanceState*)0)->powerShotMode == 0x44) ? 1 : -1];
typedef char SwingClubCategoryOffset[((int)&((SwingShotInfo*)0)->clubCategory == 0xA) ? 1 : -1];

// The motion controller's virtual functions, called through the vtable pointer at +0x44. Each slot is
// declared with the arguments the callers pass.
struct MotionSlots {
    char pad_00[0x44];
    virtual void Destroy(int deleteFlag) DECOMP_LOCAL_VIRTUAL; // slot 0
    virtual int GetMotionPhase() DECOMP_LOCAL_VIRTUAL;         // slot 1
    UNUSED_SLOT(2);
    UNUSED_SLOT(3);
    virtual void ResetMotionToIdle(SwingAnimationTarget* target) DECOMP_LOCAL_VIRTUAL; // slot 4
    UNUSED_SLOT(5);
    virtual float DampedMoveTowardTarget(SwingAnimationTarget* target, float targetFraction, int dampWhenMoving,
                                         int brakeAtTarget, int skipAnimationUpdate) DECOMP_LOCAL_VIRTUAL; // slot 6
    UNUSED_SLOT(7);
    virtual int UpdateSwingPlayback(SwingAnimationTarget* target, float swingValue, float deltaSeconds,
                                    int skipPlayback) DECOMP_LOCAL_VIRTUAL; // slot 8
    UNUSED_SLOT(9);
    UNUSED_SLOT(10);
    UNUSED_SLOT(11);
    UNUSED_SLOT(12);
    UNUSED_SLOT(13);
    virtual int IsLargeWoodSwing(SwingAnimationTarget* target, float power) DECOMP_LOCAL_VIRTUAL; // slot 14
    UNUSED_SLOT(15);
    UNUSED_SLOT(16);
    virtual int GetCharacterTimingFrames() DECOMP_LOCAL_VIRTUAL;     // slot 17
    virtual int vfunc_18() DECOMP_LOCAL_VIRTUAL;                     // slot 18
    virtual float GetSelectedMotionThreshold() DECOMP_LOCAL_VIRTUAL; // slot 19
    virtual int SelectSwingMotion() DECOMP_LOCAL_VIRTUAL;            // slot 20
    virtual int GetThresholdIndex() DECOMP_LOCAL_VIRTUAL;            // slot 21
    virtual int GetTakebackMotionID() DECOMP_LOCAL_VIRTUAL;          // slot 22
};

// The acceleration controller's virtual functions, called through the vtable pointer at +0x3924.
struct AccelerationSlots {
    char pad_00[0x3924];
    virtual void Destroy(int deleteFlag) DECOMP_LOCAL_VIRTUAL;              // slot 0
    virtual void Reset() DECOMP_LOCAL_VIRTUAL;                              // slot 1
    virtual void Update(float seconds, int nonPutter) DECOMP_LOCAL_VIRTUAL; // slot 2
    UNUSED_SLOT(3);
    virtual void ResetHook() DECOMP_LOCAL_VIRTUAL; // slot 4
    UNUSED_SLOT(5);
    UNUSED_SLOT(6);
    UNUSED_SLOT(7);
    UNUSED_SLOT(8);
    UNUSED_SLOT(9);
    UNUSED_SLOT(10);
    UNUSED_SLOT(11);
    UNUSED_SLOT(12);
    UNUSED_SLOT(13);
    virtual float GetDirectionAngleRadians() DECOMP_LOCAL_VIRTUAL; // slot 14: handedness-adjusted swing direction
};

// The swing controller's own virtual functions, called through the vtable pointer at +0x58. The phase
// callbacks take the player's animation interface and the animation state read from the player.
struct SwingPhaseSlots {
    char pad_00[0x58];
    UNUSED_SLOT(0);
    UNUSED_SLOT(1);
    UNUSED_SLOT(2);
    UNUSED_SLOT(3);
    UNUSED_SLOT(4);
    UNUSED_SLOT(5);
    UNUSED_SLOT(6);
    UNUSED_SLOT(7);
    UNUSED_SLOT(8);
    UNUSED_SLOT(9);
    UNUSED_SLOT(10);
    UNUSED_SLOT(11);
    UNUSED_SLOT(12);
    UNUSED_SLOT(13);
    UNUSED_SLOT(14);
    UNUSED_SLOT(15);
    UNUSED_SLOT(16);
    UNUSED_SLOT(17);
    UNUSED_SLOT(18);
    UNUSED_SLOT(19);
    UNUSED_SLOT(20);
    virtual int UpdateIdle(SwingAnimationTarget* target, int animationState) DECOMP_LOCAL_VIRTUAL; // slot 21
    virtual int UpdateReturning(SwingGauge* out, SwingAnimationTarget* target,
                                int animationState) DECOMP_LOCAL_VIRTUAL; // slot 22
    virtual int UpdateForward(float delta, SwingAnimationTarget* target,
                              int animationState) DECOMP_LOCAL_VIRTUAL; // slot 23
    virtual int UpdateSettling(float delta, SwingAnimationTarget* target,
                               int animationState) DECOMP_LOCAL_VIRTUAL; // slot 24
    virtual int UpdateFinish(float delta, SwingAnimationTarget* target,
                             int animationState) DECOMP_LOCAL_VIRTUAL; // slot 25
    UNUSED_SLOT(26);
    UNUSED_SLOT(27);
    virtual void GetSelectedPowerAndKind(float* powerFraction, int* kind) DECOMP_LOCAL_VIRTUAL; // slot 28
};

// The gauge block the caller owns, filled in by UpdateSwingPhaseAndGauge. Values are cursor coordinates on the
// shot gauge, not world positions. Fields a path does not assign keep the caller's values.
struct SwingGauge {
    float powerCursor;      // 0x00
    float impactCursor;     // 0x04
    float continuousCursor; // 0x08
    float roundedCursor;    // 0x0C
    int pad_10;
    unsigned char powerClamped; // 0x14
    char pad_15[3];
    int phaseDirection; // 0x18: 0 idle, -1 forward or settling, +1 otherwise
    float impactOrigin; // 0x1C: read, never written here
    int pad_20;
    int pad_24;
    int swingKind;         // 0x28
    float fullPowerCursor; // 0x2C
};

static inline SwingInstanceState* CurrentSwing() {
    return (SwingInstanceState*)InstanceTable_Get();
}
static inline MotionSlots* Slots(CPadSwingCtrlMotion* motion) {
    return (MotionSlots*)motion;
}
static inline MotionSlots* MotionOf(CPadSwingCtrl* ctrl) {
    return (MotionSlots*)ctrl->motionController;
}
static inline AccelerationSlots* AccelerationOf(CPadSwingCtrl* ctrl) {
    return (AccelerationSlots*)ctrl->accelerationController;
}
static inline SwingPhaseSlots* PhaseSlots(CPadSwingCtrl* ctrl) {
    return (SwingPhaseSlots*)ctrl;
}

// The completion check calls two motion slots with the target and swing value as arguments; the
// implementations do not read them.
struct SwingCompletionSlots {
    char pad_00[0x44];
    UNUSED_SLOT(0);
    UNUSED_SLOT(1);
    UNUSED_SLOT(2);
    UNUSED_SLOT(3);
    UNUSED_SLOT(4);
    UNUSED_SLOT(5);
    UNUSED_SLOT(6);
    UNUSED_SLOT(7);
    UNUSED_SLOT(8);
    UNUSED_SLOT(9);
    UNUSED_SLOT(10);
    UNUSED_SLOT(11);
    UNUSED_SLOT(12);
    UNUSED_SLOT(13);
    UNUSED_SLOT(14);
    UNUSED_SLOT(15);
    UNUSED_SLOT(16);
    virtual unsigned int GetCharacterTimingFrames(SwingAnimationTarget* target,
                                                  float value) DECOMP_LOCAL_VIRTUAL; // slot 17
    UNUSED_SLOT(18);
    UNUSED_SLOT(19);
    virtual int SelectSwingMotion(SwingAnimationTarget* target, float value) DECOMP_LOCAL_VIRTUAL; // slot 20
};

// Smooth the target swing fraction with bounded acceleration, damping and optional braking.
//
// The position runs 0..motionLimit; targetFraction and the return value are fractions of the limit. The
// velocity is in position units per call. Per call, in order:
//   1. acceleration = (target - position) / responseFrames, clamped to +-maxVelocityChange;
//   2. damping: velocity *= (dampingFrames - 1) / dampingFrames, always, or (with dampWhenMoving) only while
//      the velocity or the distance to the target exceeds its deadband;
//   3. velocity += acceleration, clamped to +-velocityLimit; position += velocity, clamped to 0..limit
//      (overshooting a bound zeroes the velocity);
//   4. brakeAtTarget: once the position reaches the target with the velocity above a small threshold, the
//      velocity is halved;
//   5. unless skipAnimationUpdate, write the position into the takeback animation.
// The four time constants are divided or multiplied by 1.2 when duration scaling is enabled.
float CPadSwingCtrlMotion::DampedMoveTowardTarget(SwingAnimationTarget* target, float targetFraction,
                                                  int dampWhenMoving, int brakeAtTarget, int skipAnimationUpdate) {
    float positionLimit, targetPosition, durationScale, responseFrames, difference, dampingFrames, velocityChange,
        maxVelocityChange, velocityLimit;
    float zero, velocity;
    target->SetManualAnimation(1);
    positionLimit = motionLimit;
    targetPosition = targetFraction * positionLimit;
    durationScale = EventTiming_IsDurationScalingEnabled() ? kDurationScale : kOne;
    responseFrames = kResponseFrames / durationScale;
    velocityLimit = kVelocityLimit * durationScale;
    maxVelocityChange = kHalf * durationScale;
    dampingFrames = kDampingFrames / durationScale;
    velocityChange = (targetPosition - motionPosition) / responseFrames;
    if (velocityChange >= kZero && velocityChange > maxVelocityChange) {
        velocityChange = maxVelocityChange;
    } else if (velocityChange < kZero && velocityChange < -maxVelocityChange) {
        velocityChange = -maxVelocityChange;
    }
    if (dampWhenMoving) {
        float normalized = motionPosition / positionLimit - targetFraction;
        if (motionVelocity > kVelocityDeadband || motionVelocity < kNegativeVelocityDeadband ||
            normalized > kPositionDeadband || normalized < kNegativePositionDeadband) {
            float damping = (dampingFrames - kOne) / dampingFrames;
            motionVelocity *= damping;
        }
    } else {
        float damping = (dampingFrames - kOne) / dampingFrames;
        motionVelocity *= damping;
    }
    velocity = motionVelocity;
    zero = kZero;
    velocity += velocityChange;
    motionVelocity = velocity;
    if (velocity >= zero && velocity > velocityLimit) {
        motionVelocity = velocityLimit;
    } else if (motionVelocity < kZero && motionVelocity < -velocityLimit) {
        motionVelocity = -velocityLimit;
    }
    float position = motionPosition + motionVelocity;
    motionPosition = position;
    if (position > positionLimit) {
        float zero = kZero;
        motionPosition = positionLimit;
        motionVelocity = zero;
    } else if (position < kZero) {
        float zero = kZero;
        motionPosition = zero;
        motionVelocity = zero;
    }
    if (brakeAtTarget && motionPosition >= targetPosition && motionVelocity > kBrakeVelocity) {
        motionVelocity *= kHalf;
    }
    if (!skipAnimationUpdate) {
        int index = Slots(this)->GetTakebackMotionID() * kMotionNameStride;
        float value = motionPosition;
        target->SetAnimationPosition(swingMotionNames + index, value);
    }
    return motionPosition / positionLimit;
}

// Drive the smoothed position toward the current threshold and return it as a fraction of the limit.
float CPadSwingCtrlMotion::UpdateMotionToThreshold(SwingAnimationTarget* target) {
    float scale = motionLimit;
    float thresholdPosition = Slots(this)->GetSelectedMotionThreshold();
    float normalized = thresholdPosition / scale;
    Slots(this)->DampedMoveTowardTarget(target, normalized, 1, 1, 0);
    return motionPosition / scale;
}

// Settle the smoothed position, start the swing animation, then wait for it to finish.
//
// Returns 0 while settling or waiting and 1 once the selected frame count has elapsed (frames are seconds times
// 30). swingValue is passed to the motion selection. skipPlayback leaves the animation unstarted but still
// advances the state.
int CPadSwingCtrlMotion::UpdateSwingPlayback(SwingAnimationTarget* target, float swingValue, float deltaSeconds,
                                             int skipPlayback) {
    int result = 0;
    float scale = motionLimit;
    if (motionSettled == 0) {
        // Settle: approach the threshold position.
        float targetPosition = Slots(this)->GetSelectedMotionThreshold();
        Slots(this)->DampedMoveTowardTarget(target, targetPosition / scale, 0, 0, skipPlayback);
        if (kFramesPerSecond * settlingElapsed >= kOne) {
            if (motionPosition - targetPosition < kSettleTolerance &&
                motionPosition - targetPosition > kNegativeSettleTolerance) {
                if (motionVelocity < kSettleTolerance && motionVelocity > kNegativeSettleTolerance) {
                    motionSettled = 1;
                    motionPosition = kZero;
                    motionVelocity = kZero;
                } else if (motionVelocity < kZero) {
                    motionPosition = kZero;
                    motionSettled = 1;
                    motionVelocity = kZero;
                }
            }
        }
        settlingElapsed += deltaSeconds;
    } else if (animationStarted == 0) {
        // Start the animation once after settling; later calls only advance time.
        if (skipPlayback == 0) {
            target->SetManualAnimation(0);
            int index = ((SwingCompletionSlots*)this)->SelectSwingMotion(target, swingValue);
            target->PlayAnimation(swingMotionNames + index * kMotionNameStride, 2, kZero);
        }
        animationStarted = 1;
    } else {
        // Wait for the selected frame count.
        animationElapsed += deltaSeconds;
        int completionFrames = Slots(this)->GetCharacterTimingFrames();
        float frameThreshold = (float)completionFrames;
        if (kFramesPerSecond * animationElapsed >= frameThreshold) {
            result = 1;
        }
    }
    return result;
}

// Report that the swing animation has played through its end frame plus its delay.
//
// The end frame comes from the selected animation's range; the delay is 30 frames for power shots, otherwise
// the character's timing frames subtracted from zero. completionElapsed advances on every call.
int CPadSwingCtrlMotion::IsSwingAnimationComplete(SwingAnimationTarget* target, float value, float delta) {
    int result = 0;
    int index = kMotionWoodPrepare;
    float offset = kZero;
    float position = kZero;
    SwingInstanceState* state = CurrentSwing();
    if (state->powerShotMode != 0) {
        state = CurrentSwing();
        unsigned char type = state->shot->clubCategory;
        switch (type) {
        case kWood:
            state = CurrentSwing();
            if (state->powerShotMode == 2) {
                index = kMotionWoodDoublePower;
            } else {
                index = kMotionWoodPower;
            }
            break;
        case kIron:
            index = kMotionIronPower;
            break;
        case kWedge:
            index = kMotionWedgePower;
            break;
        }
        offset = kFramesPerSecond;
    } else {
        index = ((SwingCompletionSlots*)this)->SelectSwingMotion(target, value);
        int count = ((SwingCompletionSlots*)this)->GetCharacterTimingFrames(target, value);
        offset = offset - (float)count;
    }
    const char* entry = swingMotionNames + index * kMotionNameStride;
    target->QueryAnimationEnd(&position, entry);
    position += offset;
    completionElapsed += delta;
    if (kFramesPerSecond * completionElapsed >= kMotionCompletionFrameOffset + position) {
        result = 1;
    }
    return result;
}

// Wait for the power-shot animation to start, forcing it to play after one second.
//
// Returns 1 even while waiting; animationStarted holds the state.
int CPadSwingCtrlMotion::StartPowerShotAnimation(SwingAnimationTarget* target, float delta) {
    int index = kMotionWoodPrepare;
    if (animationStarted == 0) {
        SwingInstanceState* state = CurrentSwing();
        unsigned char type = state->shot->clubCategory;
        switch (type) {
        case kWood:
            state = CurrentSwing();
            if (state->powerShotMode == 2) {
                index = kMotionWoodDoublePower;
            } else {
                index = kMotionWoodPower;
            }
            break;
        case kIron:
            index = kMotionIronPower;
            break;
        case kWedge:
            index = kMotionWedgePower;
            break;
        }
        target->SetManualAnimation(0);
        const char* entry = swingMotionNames + index * kMotionNameStride;
        if (target->IsAnimationPlaying(entry)) {
            animationStarted = 1;
        }
        if (animationStarted == 0) {
            animationStartWaitElapsed += delta;
            if (animationStartWaitElapsed > kOne) {
                target->PlayAnimation(entry, 2, kZero);
                animationStarted = 1;
            }
        }
    }
    return 1;
}

// The preparation animation for the current club and power mode. Putters and unknown clubs keep the wood
// preparation in both modes.
static inline int PrepareAnimationIndex() {
    int index = kMotionWoodPrepare;
    SwingInstanceState* state = CurrentSwing();
    if (state->powerShotMode != 0) {
        state = CurrentSwing();
        unsigned char type = state->shot->clubCategory;
        switch (type) {
        case kWood:
            state = CurrentSwing();
            if (state->powerShotMode == 2) {
                index = kMotionWoodDoublePowerPrepare;
            } else {
                index = kMotionWoodPowerPrepare;
            }
            break;
        case kIron:
            index = kMotionIronPowerPrepare;
            break;
        case kWedge:
            index = kMotionWedgePowerPrepare;
            break;
        }
    } else {
        state = CurrentSwing();
        unsigned char type = state->shot->clubCategory;
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
    return index;
}

// Play the club's preparation animation, in power-shot mode when one is active.
//
// Selection by club (wood, iron, wedge) and power mode (0 normal, 2 double, otherwise single):
//   normal: wood, iron and wedge prepare;
//   double: wood double-power prepare; single: wood power prepare;
//   power shots: iron and wedge power prepare.
// Putters and unknown clubs fall back to the wood preparation in both modes. Always returns 1.
int CPadSwingCtrlMotion::PlayPrepareAnimation(SwingAnimationTarget* target) {
    int index = PrepareAnimationIndex();
    target->SetManualAnimation(0);
    if (CurrentSwing()->powerShotMode != 0) {
        target->PlayAnimation(swingMotionNames + index * kMotionNameStride, 3, kZero);
    } else {
        target->PlayAnimation(swingMotionNames + index * kMotionNameStride, 0, kZero);
    }
    return 1;
}

// Put the club's preparation animation on its first frame in manual mode. Selects the animation the same way
// as PlayPrepareAnimation. Always returns 1.
int CPadSwingCtrlMotion::SetManualPrepareAnimation(SwingAnimationTarget* target) {
    int index = PrepareAnimationIndex();
    target->SetManualAnimation(1);
    target->SetAnimationPosition((const char*)swingMotionNames + index * kMotionNameStride, kZero);
    return 1;
}

// Report support for an optional swing capability; always enabled here.
int CPadSwingCtrlMotion::vfunc_13() {
    return 1;
}

// Test whether the selected normal swing is the large wood animation.
int CPadSwingCtrlMotion::IsLargeWoodSwing() {
    return Slots(this)->SelectSwingMotion() == kMotionWoodLarge;
}

// Map the selected normal swing animation to its size class: 0 small, 1 medium, 2 large.
int CPadSwingCtrlMotion::GetSwingSizeClass() {
    int motionIndex = Slots(this)->SelectSwingMotion();
    switch (motionIndex) {
    case kMotionWoodLarge:
    case kMotionIronLarge:
    case kMotionWedgeLarge:
        return 2;
    case kMotionWoodMedium:
    case kMotionIronMedium:
    case kMotionWedgeMedium:
        return 1;
    case kMotionWoodSmall:
    case kMotionIronSmall:
        return 0;
    default:
        return 0;
    }
}

// Report support for an optional swing capability; always enabled here.
int CPadSwingCtrlMotion::vfunc_16() {
    return 1;
}

// Look up the equipped character's timing frames for the selected swing size.
//
// s_characterSwingTimingFrames has one row per character type index (the type ID's low 26 bits, not its kind
// bits) and one column per threshold index: wood small/medium/large, iron small/medium/large, wedge
// medium/large, putter medium/large. IsSwingAnimationComplete subtracts the result from the completion frame,
// so a larger value ends the swing sooner. The row index is not range checked.
int CPadSwingCtrlMotion::GetCharacterTimingFrames() {
    int thresholdIndex = Slots(this)->GetThresholdIndex();
    int clampedIndex = thresholdIndex;
    if (thresholdIndex < 0) {
        clampedIndex = 0;
    } else if (thresholdIndex >= kMotionThresholdCount) {
        clampedIndex = 0;
    }
    thresholdIndex = clampedIndex;
    CGolfDoc* golfDoc = (CGolfDoc*)g_sharedDoc->golfDoc;
    unsigned char playerIndex = golfDoc->currentPlayer;
    void* characterSelection = ((CGolfDoc*)golfDoc)->GetItemSelection(playerIndex);
    unsigned int characterIndex = *(unsigned int*)characterSelection & kSelectionTypeIndexMask;
    return s_characterSwingTimingFrames[characterIndex][thresholdIndex];
}

// Report support for an optional swing capability; always enabled here.
int CPadSwingCtrlMotion::vfunc_18() {
    return 1;
}

// Return the position threshold for the selected swing size. An index out of range falls back to entry zero.
float CPadSwingCtrlMotion::GetSelectedMotionThreshold() {
    int thresholdIndex = Slots(this)->GetThresholdIndex();
    if (thresholdIndex < 0) {
        thresholdIndex = 0;
    } else if (thresholdIndex >= kMotionThresholdCount) {
        thresholdIndex = 0;
    }
    return s_swingMotionThresholds[thresholdIndex];
}

// Pick the swing animation from the club and the swing power. powerFraction is 0..1 and is scaled by the
// motion limit before comparing against the club's thresholds (ties go to the smaller swing). Clubs that are
// not handled keep the large wood swing.
int CPadSwingCtrlMotion::SelectSwingMotion(float powerFraction) {
    int result = kMotionWoodLarge;
    float positionLimit = motionLimit;
    SwingInstanceState* state = CurrentSwing();
    unsigned char clubCategory = state->shot->clubCategory;
    switch (clubCategory) {
    case kWood:
        if (powerFraction * positionLimit > s_swingMotionThresholds[1]) {
            result = kMotionWoodLarge;
        } else if (powerFraction * positionLimit > s_swingMotionThresholds[0]) {
            result = kMotionWoodMedium;
        } else {
            result = kMotionWoodSmall;
        }
        break;
    case kIron:
        if (powerFraction * positionLimit > s_swingMotionThresholds[4]) {
            result = kMotionIronLarge;
        } else if (powerFraction * positionLimit > s_swingMotionThresholds[3]) {
            result = kMotionIronMedium;
        } else {
            result = kMotionIronSmall;
        }
        break;
    case kWedge:
        result = powerFraction * positionLimit > s_swingMotionThresholds[6] ? kMotionWedgeLarge : kMotionWedgeMedium;
        break;
    case kPutter:
        result = powerFraction * positionLimit > s_swingMotionThresholds[8] ? kMotionPutterLarge : kMotionPutterMedium;
        break;
    }
    return result;
}

// Translate the selected swing animation into one of the ten threshold table entries.
int CPadSwingCtrlMotion::GetThresholdIndex() {
    int result = 0;
    switch (Slots(this)->SelectSwingMotion()) {
    case kMotionWoodSmall:
        result = 0;
        break;
    case kMotionWoodMedium:
        result = 1;
        break;
    case kMotionWoodLarge:
        result = 2;
        break;
    case kMotionIronSmall:
        result = 3;
        break;
    case kMotionIronMedium:
        result = 4;
        break;
    case kMotionIronLarge:
        result = 5;
        break;
    case kMotionWedgeMedium:
        result = 6;
        break;
    case kMotionWedgeLarge:
        result = 7;
        break;
    case kMotionPutterMedium:
        result = 8;
        break;
    case kMotionPutterLarge:
        result = 9;
        break;
    default:
        break;
    }
    return result;
}

// The club's takeback animation.
int CPadSwingCtrlMotion::GetTakebackMotionID() {
    int result = kMotionWoodPrepare;
    unsigned char club = CurrentSwing()->shot->clubCategory;
    switch (club) {
    case kWood:
        result = kMotionWoodTakeback;
        break;
    case kIron:
        result = kMotionIronTakeback;
        break;
    case kWedge:
        result = kMotionWedgeTakeback;
        break;
    case kPutter:
        result = kMotionPutterTakeback;
        break;
    }
    return result;
}

// The club's burning takeback animation.
int CPadSwingCtrlMotion::GetTakebackBurnMotion() {
    int result = kMotionWoodPrepare;
    unsigned char club = CurrentSwing()->shot->clubCategory;
    switch (club) {
    case kWood:
        result = kMotionWoodTakebackBurn;
        break;
    case kIron:
        result = kMotionIronTakebackBurn;
        break;
    case kWedge:
        result = kMotionWedgeTakebackBurn;
        break;
    case kPutter:
        result = kMotionPutterTakebackBurn;
        break;
    }
    return result;
}

// Pass the first owned motion channel to the target's first channel hook.
void CPadSwingCtrlMotion::AttachFirstChannel(SwingAnimationTarget* target) {
    target->AttachChannel(&firstOwnedMotionChannel);
}

// Pass the first owned motion channel to the target's second channel hook.
void CPadSwingCtrlMotion::AttachFirstChannelAlternate(SwingAnimationTarget* target) {
    target->AttachChannelAlternate(&firstOwnedMotionChannel);
}

// Initialize this unit's cached full-circle angle.
extern "C" void PadSwing_InitializeAngleCache() {
    s_padSwingAngleCache = kTwoPi;
}

// The acceleration controller and the motion controller are allocated at their full sizes and constructed in
// place by the other units' constructors.
struct AccelerationAllocation {
    char data[0x3928];
    AccelerationAllocation() {
        ((CPadSwingAccele*)this)->Init();
    }
};
struct MotionAllocation {
    char data[0x48];
    MotionAllocation() {
        ((CPadSwingCtrlMotion*)this)->Init();
    }
};

// Allocate the acceleration and motion controllers and clear the swing and ramp state.
CPadSwingCtrl::CPadSwingCtrl() {
    interfaceVtable = __vt__13CPadSwingCtrl;
    AccelerationAllocation* acceleration = new AccelerationAllocation;
    accelerationController = acceleration;
    MotionAllocation* motion = new MotionAllocation;
    float neg = kFloat_neg1_PadSwing;
    float zero = kFloat_0_PadSwing;
    int zeroInt = 0;
    int negone = -1;
    motionController = motion;
    swingPower = zero;
    smoothedMotionFraction = zero;
    primarySwingButtonArmed = zeroInt;
    alternateSwingButtonArmed = zeroInt;
    largeWoodSwingSelected = zeroInt;
    swingSizeClass = zeroInt;
    swingStartRequested = zeroInt;
    finishElapsed = neg;
    rampTarget = zero;
    rampFrames = negone;
    rampFrame = zeroInt;
    primarySwingRequested = zeroInt;
    alternateSwingRequested = zeroInt;
    powerThresholdReached = zeroInt;
    rampRunning = zeroInt;
    // These two words are also cleared when the minimum impact width is enforced.
    unk_50 = zeroInt;
    unk_54 = zeroInt;
}

// Destroy the owned acceleration and motion controllers.
CPadSwingCtrl::~CPadSwingCtrl() {
    AccelerationSlots* acceleration = AccelerationOf(this);
    interfaceVtable = &__vt__13CPadSwingCtrl;
    if (acceleration != 0) {
        acceleration->Destroy(1);
    }
    MotionSlots* motion = MotionOf(this);
    if (motion != 0) {
        motion->Destroy(1);
    }
}

// Reset the acceleration controller and clear the pending swing and ramp requests.
int CPadSwingCtrl::ResetSwingRequestsAndRampState() {
    AccelerationOf(this)->Reset();
    float zero = kFloat_0_PadSwing;
    int zeroInt = 0;
    int unsetState = kSwingStateUnset;
    int negone = -1;
    powerThresholdReached = zeroInt;
    swingStartRequested = zeroInt;
    rampTarget = zero;
    rampFrames = negone;
    rampFrame = zeroInt;
    rampRunning = zeroInt;
    primarySwingRequested = zeroInt;
    alternateSwingRequested = zeroInt;
    forcedSwingActive = zeroInt;
    // The two readiness codes close both gates when set to 6; they are not motion phases.
    readinessRangeCode = unsetState;
    readinessInhibitCode = unsetState;
    return 1;
}

// Advance the current swing phase and publish the power and impact cursor values.
//
// The acceleration controller is updated first, then the phase callback for the motion's current phase runs and
// its result is returned unchanged. Completion returns at once, before the gauge is touched. For the other
// phases the gauge gets:
//   always: phaseDirection (0 idle, -1 forward or settling, +1 otherwise) and the power cursor,
//     floor(149 + 360 * swingPower), in continuousCursor, roundedCursor and powerCursor;
//   when the result is a committed code and the club is not a putter: fullPowerCursor, swingKind and
//     impactCursor (at least 114), then continuousCursor and roundedCursor become the impact cursor (rounded
//     up) and powerCursor is capped at 509, with powerClamped set when it reaches the cap.
// A forced swing keeps the full power and uses impactOrigin as the impact cursor. animationState comes from
// the player; the callbacks treat 3 as the power-ready state.
int CPadSwingCtrl::UpdateSwingPhaseAndGauge(SwingGauge* out, float delta, SwingAnimationTarget* target,
                                            int animationState) {
    int result = 0;
    int useNonPutterAcceleration = 1;
    if (CurrentSwing()->shot->clubCategory == kPutter) {
        useNonPutterAcceleration = 0;
    }
    AccelerationOf(this)->Update(delta, useNonPutterAcceleration);
    int mode = MotionOf(this)->GetMotionPhase();
    switch (mode) {
    case kSwingIdle:
        result = PhaseSlots(this)->UpdateIdle(target, animationState);
        break;
    case kSwingReturning:
        result = PhaseSlots(this)->UpdateReturning(out, target, animationState);
        break;
    case kSwingForward:
        result = PhaseSlots(this)->UpdateForward(delta, target, animationState);
        break;
    case kSwingSettling:
        result = PhaseSlots(this)->UpdateSettling(delta, target, animationState);
        break;
    case kSwingComplete:
        return PhaseSlots(this)->UpdateFinish(delta, target, animationState);
    default:
        break;
    }

    // The phase can change inside its callback, so query it again for the gauge direction.
    int status = MotionOf(this)->GetMotionPhase();
    if (status == kSwingIdle) {
        out->phaseDirection = 0;
    } else {
        status = MotionOf(this)->GetMotionPhase();
        if (status == kSwingForward || MotionOf(this)->GetMotionPhase() == kSwingSettling) {
            out->phaseDirection = -1;
        } else {
            out->phaseDirection = 1;
        }
    }
    out->continuousCursor = kGaugeOrigin + kGaugeSpan * swingPower;
    float flooredPowerCursor = floor(out->continuousCursor);
    out->roundedCursor = flooredPowerCursor;
    out->powerCursor = flooredPowerCursor;
    // Only committed non-putter shots publish the impact cursor and swing kind.
    if (result == kSwingPrimaryCommitted || result == kSwingAlternateCommitted) {
        if (CurrentSwing()->shot->clubCategory != kPutter) {
            if (forcedSwingActive == 0) {
                int shotKind = kDefaultSwingKindCode;
                float selectedPowerFraction = kFloat_0_PadSwing;
                PhaseSlots(this)->GetSelectedPowerAndKind(&selectedPowerFraction, &shotKind);
                out->fullPowerCursor = out->powerCursor;
                float powerSpan = kGaugeSpan * swingPower;
                out->powerCursor = floor(kGaugeOrigin + selectedPowerFraction * powerSpan);
                out->swingKind = shotKind;
                float swingDirectionRadians = AccelerationOf(this)->GetDirectionAngleRadians();
                // 30 cursor units per radian of swing direction.
                float t = kCursorUnitsPerRadian * swingDirectionRadians + out->impactOrigin;
                out->impactCursor = t;
                if (out->impactCursor < kMinimumImpactCursor) {
                    out->impactCursor = kMinimumImpactCursor;
                }
            } else {
                out->fullPowerCursor = out->powerCursor;
                out->swingKind = kDefaultSwingKindCode;
                out->impactCursor = out->impactOrigin;
            }
            // The impact cursor rounds up; the power cursor above rounded down.
            float impactCursor = out->impactCursor;
            out->continuousCursor = impactCursor;
            float roundedImpactCursor = ceil(impactCursor);
            out->roundedCursor = roundedImpactCursor;
            if (out->powerCursor >= kMaximumPowerCursor) {
                out->powerCursor = kMaximumPowerCursor;
                out->powerClamped = 1;
            } else {
                out->powerClamped = 0;
            }
        }
    }
    // Consume the start request, and latch the power threshold once it is reached.
    swingStartRequested = 0;
    if (swingPower >= kPowerThreshold || CurrentSwing()->shot->clubCategory == kPutter) {
        powerThresholdReached = 1;
    }
    return result;
}

// Ask the motion controller for the capability reported by its slot 18 (always supported).
int CPadSwingCtrl::vfunc_3() {
    MotionSlots* motion = MotionOf(this);
    motion->vfunc_18();
    return 1;
}

// Advance the motion's playback and reset it once it completes.
//
// Returns 1 only when the playback result is exactly 1. The large-wood flag is refreshed on every call, before
// the reset.
int CPadSwingCtrl::UpdateMotionPlayback(SwingAnimationTarget* target, float delta) {
    int done = 0;
    int result = MotionOf(this)->UpdateSwingPlayback(target, swingPower, delta, 0);
    largeWoodSwingSelected = MotionOf(this)->IsLargeWoodSwing(target, swingPower);
    if (result == 1) {
        done = 1;
        MotionOf(this)->ResetMotionToIdle(target);
    }
    return done;
}

// Play the motion at fixed steps of 1/30 second, at most 150 of them, until it finishes.
//
// Returns the index of the step that finished it (0 for the first), or 150 if it never did. Any nonzero result
// ends the loop, and the motion is reset only on completion.
int CPadSwingCtrl::FastForwardMotion(SwingAnimationTarget* target) {
    int i = 0;
    do {
        MotionSlots* motion = MotionOf(this);
        if (motion->UpdateSwingPlayback(target, swingPower, kSettleStepSeconds, 1)) {
            MotionOf(this)->ResetMotionToIdle(target);
            break;
        }
        ++i;
    } while (i < kMaxSettleSteps);
    return i;
}

// Report that this lifecycle hook succeeded.
int CPadSwingCtrl::vfunc_6() {
    return 1;
}

// Call the acceleration controller's slot 4 hook.
int CPadSwingCtrl::vfunc_7() {
    AccelerationSlots* acceleration = AccelerationOf(this);
    acceleration->ResetHook();
    return 1;
}

// Static initialisers run from the .ctors table, as in the original.
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462228", _ctors_u80226C9C_0, PadSwing_InitializeAngleCache);
#pragma pop
