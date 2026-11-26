// CPadSwingCtrlMotion: the motion-controlled (Wii Remote) swing's smoothed position and the club's swing
// animation. It moves a position toward a target with damping, picks the swing animation from the club and
// the swing power, and decides when the animation has played long enough.
//
// The class is declared without virtual functions on purpose: its vtable is still taken from the original
// game data, so the pointer to it is spelled out as a member. The methods below are called through that
// vtable.
#ifndef CLASSES_CPADSWINGCTRLMOTION_H
#define CLASSES_CPADSWINGCTRLMOTION_H

#include "game/types.h"

// The player's animation interface. Defined in the files that call it.
struct SwingAnimationTarget;
// The object interface that binds the selected motion. Defined in the file that binds it.
struct Object0;

class CPadSwingCtrlMotion {
public:
    // Initialize the motion state, timing, interface, and owned channel pointers.
    void Init();
    // Delete the owned channel arrays and optionally release the controller.
    void* Destroy(int deletingFlag);
    // Return the current motion-state value.
    unsigned int vfunc_1();
    // Advance the motion phase, clearing the timing and position data needed by its next phase.
    int AdvanceMotionPhase();
    // Return the motion controller to idle and clear its timing, position, and completion data.
    int ResetMotionToIdle();
    // Bind the motion entry selected by the virtual motion-index query to the supplied object.
    float BindSelectedMotion(Object0* object);

    // Smoothed position.
    float DampedMoveTowardTarget(SwingAnimationTarget* target, float targetFraction, int dampWhenMoving,
                                 int brakeAtTarget, int skipAnimationUpdate);
    float UpdateMotionToThreshold(SwingAnimationTarget* target);
    int UpdateSwingPlayback(SwingAnimationTarget* target, float swingValue, float deltaSeconds, int skipPlayback);

    // Animation timing.
    int IsSwingAnimationComplete(SwingAnimationTarget* target, float value, float delta);
    int StartPowerShotAnimation(SwingAnimationTarget* target, float delta);
    int PlayPrepareAnimation(SwingAnimationTarget* target);
    int SetManualPrepareAnimation(SwingAnimationTarget* target);

    // Swing selection.
    int vfunc_13();
    int IsLargeWoodSwing();
    int GetSwingSizeClass();
    int vfunc_16();
    int GetCharacterTimingFrames();
    int vfunc_18();
    float GetSelectedMotionThreshold();
    int SelectSwingMotion(float powerFraction);
    int GetThresholdIndex();
    int GetTakebackMotionID();
    int GetTakebackBurnMotion();

    // Hand the first owned motion channel to the animation target.
    void AttachFirstChannel(SwingAnimationTarget* target);
    void AttachFirstChannelAlternate(SwingAnimationTarget* target);

    int motionPhase;                 // 0x0: phase 0..4
    float motionPosition;            // 0x4: position in animation units, 0..motionLimit
    float motionLimit;               // 0x8: full range of the position
    float motionVelocity;            // 0xc: position change per call
    float motionAcceleration;        // 0x10
    float motionStep;                // 0x14
    void* firstOwnedMotionChannel;   // 0x18: owned, released by the destructor
    void* ownedChannel1;             // 0x1c
    void* ownedChannel2;             // 0x20
    void* ownedChannel3;             // 0x24
    void* ownedChannel4;             // 0x28
    int motionSettled;               // 0x2c: set once position and velocity have settled
    float settlingElapsed;           // 0x30
    int animationStarted;            // 0x34
    float animationElapsed;          // 0x38
    float completionElapsed;         // 0x3c
    float animationStartWaitElapsed; // 0x40
    void* motionVtable;              // 0x44: original vtable pointer

    int CancelForwardMotion();
};

#define CPADSWINGCTRLMOTION_OFF(f, o) \
    typedef char CPadSwingCtrlMotion_off_##f[((int)&((CPadSwingCtrlMotion*)0)->f == (o)) ? 1 : -1]
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
typedef char CPadSwingCtrlMotion_size_check[(sizeof(CPadSwingCtrlMotion) == 0x48) ? 1 : -1];

#endif // CLASSES_CPADSWINGCTRLMOTION_H
