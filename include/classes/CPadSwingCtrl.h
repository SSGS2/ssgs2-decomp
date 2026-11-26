#ifndef CLASSES_CPADSWINGCTRL_H
#define CLASSES_CPADSWINGCTRL_H

#include "game/types.h"

// The caller-owned swing gauge block and the player's animation interface. Defined in the files that use them.
struct SwingGauge;
struct SwingAnimationTarget;

class CPadSwingCtrl {
public:
    // Lifecycle: owns an acceleration controller and a motion controller.
    CPadSwingCtrl();
    ~CPadSwingCtrl();
    int ResetSwingRequestsAndRampState();

    // Per-frame update: runs the current swing phase and fills in the gauge block.
    int UpdateSwingPhaseAndGauge(SwingGauge* out, float delta, SwingAnimationTarget* target, int animationState);

    // Motion playback.
    int vfunc_3();
    int UpdateMotionPlayback(SwingAnimationTarget* target, float delta);
    int FastForwardMotion(SwingAnimationTarget* target);
    int vfunc_6();
    int vfunc_7();

    void* accelerationController;  // 0x0: owned controller; interface vptr is at +0x58
    void* motionController;        // 0x4
    float swingPower;              // 0x8
    float smoothedMotionFraction;  // 0xc
    int primarySwingButtonArmed;   // 0x10
    int alternateSwingButtonArmed; // 0x14
    int largeWoodSwingSelected;    // 0x18
    int swingSizeClass;            // 0x1c
    float finishElapsed;           // 0x20
    int powerThresholdReached;     // 0x24
    int swingStartRequested;       // 0x28
    float rampTarget;              // 0x2c
    int rampFrames;                // 0x30
    int rampFrame;                 // 0x34
    int rampRunning;               // 0x38
    int primarySwingRequested;     // 0x3c
    int alternateSwingRequested;   // 0x40
    int forcedSwingActive;         // 0x44
    int readinessRangeCode;        // 0x48
    int readinessInhibitCode;      // 0x4c
    int unk_50;                    // 0x50
    int unk_54;                    // 0x54
    void* interfaceVtable;         // 0x58

    int GetLargeWoodSwingSelected();
    int vfunc_9();
    int IsPowerThresholdReached();
    int GetTimingScaledSampleScore();
    int RequestSwingStart();
    int SetPowerRamp(float value, int count);
    int CancelPowerRamp();
    int IsRampRunning();
    int SetPrimarySwingRequested(int value);
    int SetAlternateSwingRequested(int value);
    int SetForcedSwingIfRequested();
    int vfunc_20();
    int UpdateIdlePhase(int arg, int mode);
    int UpdateReadyPhase(int ownArg, int arg, int mode);
    int UpdateForwardPhase(float delta, int arg, int mode);
    int UpdateSettlingPhase(float delta, int arg, int mode);
    int UpdateFinishPhase(float delta, int arg, int mode);
    int UpdateSwingPowerFromAngle(float angle, void* limits);
    int ApplyMinimumSwingPower();
    int EvaluatePowerBand(float* outValue, int* outIndex);
    int AdvancePowerRamp();

    float GetSampledMotionFactor();
};

#define CPADSWINGCTRL_OFF(f, o) typedef char CPadSwingCtrl_off_##f[((int)&((CPadSwingCtrl*)0)->f == (o)) ? 1 : -1]
CPADSWINGCTRL_OFF(motionController, 0x4);
CPADSWINGCTRL_OFF(swingPower, 0x8);
CPADSWINGCTRL_OFF(smoothedMotionFraction, 0xc);
CPADSWINGCTRL_OFF(primarySwingButtonArmed, 0x10);
CPADSWINGCTRL_OFF(alternateSwingButtonArmed, 0x14);
CPADSWINGCTRL_OFF(largeWoodSwingSelected, 0x18);
CPADSWINGCTRL_OFF(swingSizeClass, 0x1c);
CPADSWINGCTRL_OFF(finishElapsed, 0x20);
CPADSWINGCTRL_OFF(powerThresholdReached, 0x24);
CPADSWINGCTRL_OFF(swingStartRequested, 0x28);
CPADSWINGCTRL_OFF(rampTarget, 0x2c);
CPADSWINGCTRL_OFF(rampFrames, 0x30);
CPADSWINGCTRL_OFF(rampFrame, 0x34);
CPADSWINGCTRL_OFF(rampRunning, 0x38);
CPADSWINGCTRL_OFF(primarySwingRequested, 0x3c);
CPADSWINGCTRL_OFF(alternateSwingRequested, 0x40);
CPADSWINGCTRL_OFF(forcedSwingActive, 0x44);
CPADSWINGCTRL_OFF(readinessRangeCode, 0x48);
CPADSWINGCTRL_OFF(readinessInhibitCode, 0x4c);
CPADSWINGCTRL_OFF(unk_50, 0x50);
CPADSWINGCTRL_OFF(unk_54, 0x54);
CPADSWINGCTRL_OFF(interfaceVtable, 0x58);
#undef CPADSWINGCTRL_OFF

#endif // CLASSES_CPADSWINGCTRL_H
