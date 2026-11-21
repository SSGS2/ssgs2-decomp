// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPADSWINGCTRL_H
#define CLASSES_CPADSWINGCTRL_H

#include "game/types.h"

class CPadSwingCtrl {
public:
    void* accelerationController;       // 0x0: owned controller; interface vptr is at +0x58
    void* motionController;                        // 0x4: 33 uses in CPadSwingCtrl_UpdateReadyPhase, CPadSwingCtrl_UpdateFinishPhase, CPadSwingCtrl_UpdateSettlingPhase...
    float swingPower;                        // 0x8: 32 uses in CPadSwingCtrl_UpdateSwingPowerFromAngle, CPadSwingCtrl_UpdateReadyPhase, CPadSwingCtrl_UpdateSwingPhaseAndGauge...
    float smoothedMotionFraction;                        // 0xc: 4 uses in CPadSwingCtrl_Ctor, CPadSwingCtrl_UpdateIdlePhase, CPadSwingCtrl_UpdateReadyPhase...
    int primarySwingButtonArmed;                         // 0x10: 9 uses in CPadSwingCtrl_UpdateReadyPhase, CPadSwingCtrl_UpdateForwardPhase, CPadSwingCtrl_Ctor...
    int alternateSwingButtonArmed;                         // 0x14: 7 uses in CPadSwingCtrl_UpdateReadyPhase, CPadSwingCtrl_Ctor, CPadSwingCtrl_UpdateIdlePhase...
    int largeWoodSwingSelected;                         // 0x18: 4 uses in CPadSwingCtrl_vfunc_4, CPadSwingCtrl_Ctor, CPadSwingCtrl_vfunc_8...
    int swingSizeClass;                         // 0x1c: 4 uses in CPadSwingCtrl_UpdateSettlingPhase, CPadSwingCtrl_Ctor, CPadSwingCtrl_vfunc_9
    float finishElapsed;                       // 0x20: 7 uses in CPadSwingCtrl_UpdateFinishPhase, CPadSwingCtrl_Ctor, CPadSwingCtrl_UpdateSettlingPhase
    int powerThresholdReached;                         // 0x24: 4 uses in CPadSwingCtrl_UpdateSwingPhaseAndGauge, CPadSwingCtrl_Ctor, CPadSwingCtrl_ResetSwingRequestsAndRampState...
    int swingStartRequested;                         // 0x28: 5 uses in CPadSwingCtrl_UpdateSwingPhaseAndGauge, CPadSwingCtrl_Ctor, CPadSwingCtrl_ResetSwingRequestsAndRampState...
    float rampTarget;                       // 0x2c: 5 uses in CPadSwingCtrl_Ctor, CPadSwingCtrl_ResetSwingRequestsAndRampState, CPadSwingCtrl_SetPowerRamp...
    int rampFrames;                         // 0x30: 5 uses in CPadSwingCtrl_Ctor, CPadSwingCtrl_ResetSwingRequestsAndRampState, CPadSwingCtrl_SetPowerRamp...
    int rampFrame;                         // 0x34: 7 uses in CPadSwingCtrl_AdvancePowerRamp, CPadSwingCtrl_Ctor, CPadSwingCtrl_ResetSwingRequestsAndRampState...
    int rampRunning;                         // 0x38: 7 uses in CPadSwingCtrl_AdvancePowerRamp, CPadSwingCtrl_Ctor, CPadSwingCtrl_ResetSwingRequestsAndRampState...
    int primarySwingRequested;                         // 0x3c: 6 uses in CPadSwingCtrl_Ctor, CPadSwingCtrl_ResetSwingRequestsAndRampState, CPadSwingCtrl_vfunc_17...
    int alternateSwingRequested;                         // 0x40: 6 uses in CPadSwingCtrl_Ctor, CPadSwingCtrl_ResetSwingRequestsAndRampState, CPadSwingCtrl_vfunc_18...
    int forcedSwingActive;                         // 0x44: 5 uses in CPadSwingCtrl_UpdateSwingPhaseAndGauge, CPadSwingCtrl_ResetSwingRequestsAndRampState, CPadSwingCtrl_SetForcedSwingIfRequested...
    int readinessRangeCode;                         // 0x48: 2 uses in CPadSwingCtrl_ResetSwingRequestsAndRampState, CPadSwingCtrl_vfunc_20
    int readinessInhibitCode;                         // 0x4c: 2 uses in CPadSwingCtrl_ResetSwingRequestsAndRampState, CPadSwingCtrl_vfunc_20
    int unk_50;                         // 0x50: 2 uses in CPadSwingCtrl_Ctor, CPadSwingCtrl_ApplyMinimumSwingPower
    int unk_54;                         // 0x54: 2 uses in CPadSwingCtrl_Ctor, CPadSwingCtrl_ApplyMinimumSwingPower
    void* interfaceVtable;                       // 0x58: 2 uses in CPadSwingCtrl_dtor, CPadSwingCtrl_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
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

#endif  // CLASSES_CPADSWINGCTRL_H
