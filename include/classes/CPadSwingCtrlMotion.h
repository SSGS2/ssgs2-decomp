// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPADSWINGCTRLMOTION_H
#define CLASSES_CPADSWINGCTRLMOTION_H

#include "game/types.h"

class CPadSwingCtrlMotion {
public:
    int motionPhase;                    // 0x0: phase 0..4; interface vptr is at +0x44
    float motionPosition;                        // 0x4: 5 uses in CPadSwingCtrlMotion_vfunc_8, CPadSwingCtrlMotion_UpdateMotionToThreshold
    float motionLimit;                        // 0x8: 3 uses in CPadSwingCtrlMotion_SelectSwingMotion, CPadSwingCtrlMotion_vfunc_8, CPadSwingCtrlMotion_UpdateMotionToThreshold
    float motionVelocity;                        // 0xc: 5 uses in CPadSwingCtrlMotion_vfunc_8
    char pad_010[0x8];                  // 0x10
    void* firstOwnedMotionChannel;                       // 0x18: 3 uses in CPadSwingCtrlMotion_dtor
    char pad_01C[0x10];                 // 0x1c
    int motionSettled;                         // 0x2c: 3 uses in CPadSwingCtrlMotion_vfunc_8
    float settlingElapsed;                       // 0x30: 2 uses in CPadSwingCtrlMotion_vfunc_8
    int animationStarted;                         // 0x34: 6 uses in CPadSwingCtrlMotion_StartPowerShotAnimation, CPadSwingCtrlMotion_vfunc_8
    float animationElapsed;                       // 0x38: 2 uses in CPadSwingCtrlMotion_vfunc_8
    float completionElapsed;                       // 0x3c: 2 uses in CPadSwingCtrlMotion_IsSwingAnimationComplete
    float animationStartWaitElapsed;                       // 0x40: 2 uses in CPadSwingCtrlMotion_StartPowerShotAnimation
    void* motionVtable;                       // 0x44: 1 use in CPadSwingCtrlMotion_dtor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x8, 0x18
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPADSWINGCTRLMOTION_OFF(f, o) typedef char CPadSwingCtrlMotion_off_##f[((int)&((CPadSwingCtrlMotion*)0)->f == (o)) ? 1 : -1]
CPADSWINGCTRLMOTION_OFF(motionPosition, 0x4);
CPADSWINGCTRLMOTION_OFF(motionLimit, 0x8);
CPADSWINGCTRLMOTION_OFF(motionVelocity, 0xc);
CPADSWINGCTRLMOTION_OFF(firstOwnedMotionChannel, 0x18);
CPADSWINGCTRLMOTION_OFF(motionSettled, 0x2c);
CPADSWINGCTRLMOTION_OFF(settlingElapsed, 0x30);
CPADSWINGCTRLMOTION_OFF(animationStarted, 0x34);
CPADSWINGCTRLMOTION_OFF(animationElapsed, 0x38);
CPADSWINGCTRLMOTION_OFF(completionElapsed, 0x3c);
CPADSWINGCTRLMOTION_OFF(animationStartWaitElapsed, 0x40);
CPADSWINGCTRLMOTION_OFF(motionVtable, 0x44);
#undef CPADSWINGCTRLMOTION_OFF

#endif  // CLASSES_CPADSWINGCTRLMOTION_H
