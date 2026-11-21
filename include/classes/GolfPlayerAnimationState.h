// GolfPlayerAnimationState: non-virtual layout view; methods/accesses in golf_player_script_animation.cpp.
// No inheritance or virtual declarations: the original vtables remain external symbols.
#ifndef CLASSES_GOLFPLAYERANIMATIONSTATE_H
#define CLASSES_GOLFPLAYERANIMATIONSTATE_H

class GolfPlayerAnimationState {
public:
    char pad_000[0x4];
    void* animationObject; // 0x4: CGolfPlayer_ParseInterpolationCommand: checks object name at +0x18
    char pad_008[0x31];
    char pendingAnimationName; // 0x39: CGolfPlayer_PlayAnimationIfPartPresent: cleared after selecting animation
    char pad_03A[0x22];
    unsigned int flags; // 0x5C: CGolfPlayer_LockClubAttachment ORs 0x100
    int animationMode; // 0x60: CGolfPlayer_UpdatePowerShotPreparation: toggles between 0 and 3
    char pad_064[0x48];
    float interpolationStart; // 0xAC: CGolfPlayer_ParseInterpolationCommand: parsed first value b
    float interpolationRatePerSecond; // 0xB0: CGolfPlayer_ParseInterpolationCommand: (c-b)/(a/kAnimationFramesPerSecond_GolfPlayer), or zero
};

#define OFF(f, o) typedef char GolfPlayerAnimationState_off_##f[((int)&((GolfPlayerAnimationState*)0)->f == (o)) ? 1 : -1]
OFF(animationObject, 0x4);
OFF(pendingAnimationName, 0x39);
OFF(flags, 0x5C);
OFF(animationMode, 0x60);
OFF(interpolationStart, 0xAC);
OFF(interpolationRatePerSecond, 0xB0);
#undef OFF

#endif
