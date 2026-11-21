// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCAMERAWORK_H
#define CLASSES_CCAMERAWORK_H

#include "game/types.h"

class CCameraWork {
public:
    void* vtbl; // 0x0: action methods 0x800D830C-0x800E1318
    char pad_4[0xc];
    float inputFovOffset; // 0x10: action methods 0x800D830C-0x800E1318
    float inputRotationOffset[2]; // 0x14: action methods 0x800D830C-0x800E1318
    unsigned char inputMoved; // 0x1c: action methods 0x800D830C-0x800E1318
    char pad_1D[0x3];
    int motionCallbackThisAdjustment; // 0x20: action methods 0x800D830C-0x800E1318
    int motionCallbackVtableOffset; // 0x24: action methods 0x800D830C-0x800E1318
    int motionCallbackFunction; // 0x28: action methods 0x800D830C-0x800E1318
    float position[3]; // 0x2c: action methods 0x800D830C-0x800E1318
    float target[3]; // 0x38: action methods 0x800D830C-0x800E1318
    int modeCount; // 0x44: action methods 0x800D830C-0x800E1318
    int extendedModeCount; // 0x48: action methods 0x800D830C-0x800E1318
    int modeWeightTotal; // 0x4c: action methods 0x800D830C-0x800E1318
    int extendedModeWeightTotal; // 0x50: action methods 0x800D830C-0x800E1318
    int action; // 0x54: action methods 0x800D830C-0x800E1318
    int mode; // 0x58: action methods 0x800D830C-0x800E1318
    int pendingAction; // 0x5c: action methods 0x800D830C-0x800E1318
    unsigned int actionFlags; // 0x60: action methods 0x800D830C-0x800E1318
    float firstEventFrame; // 0x64: action methods 0x800D830C-0x800E1318
    float secondEventFrame; // 0x68: action methods 0x800D830C-0x800E1318
    float trajectoryLastFrame; // 0x6c: action methods 0x800D830C-0x800E1318
    float motionStartFrame; // 0x70: action methods 0x800D830C-0x800E1318
    float elapsedTime; // 0x74: action methods 0x800D830C-0x800E1318
    int cutoffFrame; // 0x78: action methods 0x800D830C-0x800E1318
    int lastPreMotionFrame; // 0x7c: action methods 0x800D830C-0x800E1318
    int frameIndex; // 0x80: action methods 0x800D830C-0x800E1318
    float transform[12]; // 0x84: action methods 0x800D830C-0x800E1318
    float previousTransform[12]; // 0xb4: action methods 0x800D830C-0x800E1318
    float curveControlPoints[24]; // 0xe4: action methods 0x800D830C-0x800E1318
    float motionStartPosition[3]; // 0x144: action methods 0x800D830C-0x800E1318
    float targetRiseHeight; // 0x150: action methods 0x800D830C-0x800E1318
    float animationTime; // 0x154: action methods 0x800D830C-0x800E1318
    float lastShakeUpdateTime; // 0x158: action methods 0x800D830C-0x800E1318
    float shakeStartTime; // 0x15c: action methods 0x800D830C-0x800E1318
    float lastProximitySoundUpdateTime; // 0x160: action methods 0x800D830C-0x800E1318
    unsigned char unk_164; // 0x164: action methods 0x800D830C-0x800E1318
    unsigned char holeOutDetected; // 0x165: action methods 0x800D830C-0x800E1318
    unsigned char directionalSoundPlayed; // 0x166: action methods 0x800D830C-0x800E1318
    char pad_167[0x1];
    int ballProximitySoundState; // 0x168: action methods 0x800D830C-0x800E1318
    float heading; // 0x16c: action methods 0x800D830C-0x800E1318
    float rotation[12]; // 0x170: action methods 0x800D830C-0x800E1318
    unsigned char randomModeRequested; // 0x1a0: action methods 0x800D830C-0x800E1318
    unsigned char shotCameraSelectionEnabled; // 0x1a1: action methods 0x800D830C-0x800E1318
    char pad_1A2[0x1];
    unsigned char groupAnimationActive; // 0x1a3: action methods 0x800D830C-0x800E1318
    float pivotPosition[3]; // 0x1a4: action methods 0x800D830C-0x800E1318
    int animationGroup; // 0x1b0: action methods 0x800D830C-0x800E1318
    int playingAnimationGroup; // 0x1b4: action methods 0x800D830C-0x800E1318
    float initialBallPosition[3]; // 0x1b8: action methods 0x800D830C-0x800E1318
    float startQuaternion[4]; // 0x1c4: action methods 0x800D830C-0x800E1318
    float endQuaternion[4]; // 0x1d4: action methods 0x800D830C-0x800E1318
    float startPosition[3]; // 0x1e4: action methods 0x800D830C-0x800E1318
    float endPosition[3]; // 0x1f0: action methods 0x800D830C-0x800E1318
    float horizontalBallDistance; // 0x1fc: action methods 0x800D830C-0x800E1318
    float ballHeightOffset; // 0x200: action methods 0x800D830C-0x800E1318
    float duration; // 0x204: action methods 0x800D830C-0x800E1318
    float fov; // 0x208: action methods 0x800D830C-0x800E1318
    int resetFovPending; // 0x20c: action methods 0x800D830C-0x800E1318
    unsigned char groundClearanceEnabled; // 0x210: action methods 0x800D830C-0x800E1318
    unsigned char forceGroundEffect; // 0x211: action methods 0x800D830C-0x800E1318
    char pad_212[0x6];
    void* actionHistory; // 0x218: action methods 0x800D830C-0x800E1318
    int actionCount; // 0x21c: action methods 0x800D830C-0x800E1318
    int actionCapacity; // 0x220: action methods 0x800D830C-0x800E1318
    int historyIndex; // 0x224: action methods 0x800D830C-0x800E1318
    int randomValue; // 0x228: action methods 0x800D830C-0x800E1318
    unsigned char installed; // 0x22c: action methods 0x800D830C-0x800E1318
    char pad_22D[0x3];
    float roll; // 0x230: action methods 0x800D830C-0x800E1318
    int powerShotVariant; // 0x234: action methods 0x800D830C-0x800E1318
    int pivot; // 0x238: action methods 0x800D830C-0x800E1318
    int unk_23C; // 0x23c: action methods 0x800D830C-0x800E1318
    unsigned char unk_240; // 0x240: action methods 0x800D830C-0x800E1318
    unsigned char useSavedCameraTransform; // 0x241: action methods 0x800D830C-0x800E1318
    char pad_242[0x2];
    float savedCameraRight[3]; // 0x244: action methods 0x800D830C-0x800E1318
    float savedCameraUp[3]; // 0x250: action methods 0x800D830C-0x800E1318
    float savedCameraForward[3]; // 0x25c: action methods 0x800D830C-0x800E1318
    float savedCameraPosition[3]; // 0x268: action methods 0x800D830C-0x800E1318
    float savedFov; // 0x274: action methods 0x800D830C-0x800E1318
    char characterModelMappings[16]; // 0x278: action methods 0x800D830C-0x800E1318
    char indexedCameraGroups[16]; // 0x288: action methods 0x800D830C-0x800E1318
    char powerShotMotionAliases[16]; // 0x298: action methods 0x800D830C-0x800E1318
    char resourceGroupNames[16]; // 0x2a8: action methods 0x800D830C-0x800E1318
    char awardMotionAliases[16]; // 0x2b8: action methods 0x800D830C-0x800E1318
    char loadedCharacterResourceGroups[16]; // 0x2c8: action methods 0x800D830C-0x800E1318
    char nestedCameraMap[16]; // 0x2d8: action methods 0x800D830C-0x800E1318
    char cameraWorkTree[16]; // 0x2e8: action methods 0x800D830C-0x800E1318
    int animationSegmentIndex; // 0x2f8: action methods 0x800D830C-0x800E1318
    int groupValueCount; // 0x2fc: action methods 0x800D830C-0x800E1318
};

#define CCAMERAWORK_OFF(f, o) typedef char CCameraWork_off_##f[((int)&((CCameraWork*)0)->f == (o)) ? 1 : -1]
CCAMERAWORK_OFF(vtbl, 0x0);
CCAMERAWORK_OFF(inputFovOffset, 0x10);
CCAMERAWORK_OFF(inputRotationOffset, 0x14);
CCAMERAWORK_OFF(inputMoved, 0x1c);
CCAMERAWORK_OFF(motionCallbackThisAdjustment, 0x20);
CCAMERAWORK_OFF(motionCallbackVtableOffset, 0x24);
CCAMERAWORK_OFF(motionCallbackFunction, 0x28);
CCAMERAWORK_OFF(position, 0x2c);
CCAMERAWORK_OFF(target, 0x38);
CCAMERAWORK_OFF(modeCount, 0x44);
CCAMERAWORK_OFF(extendedModeCount, 0x48);
CCAMERAWORK_OFF(modeWeightTotal, 0x4c);
CCAMERAWORK_OFF(extendedModeWeightTotal, 0x50);
CCAMERAWORK_OFF(action, 0x54);
CCAMERAWORK_OFF(mode, 0x58);
CCAMERAWORK_OFF(pendingAction, 0x5c);
CCAMERAWORK_OFF(actionFlags, 0x60);
CCAMERAWORK_OFF(firstEventFrame, 0x64);
CCAMERAWORK_OFF(secondEventFrame, 0x68);
CCAMERAWORK_OFF(trajectoryLastFrame, 0x6c);
CCAMERAWORK_OFF(motionStartFrame, 0x70);
CCAMERAWORK_OFF(elapsedTime, 0x74);
CCAMERAWORK_OFF(cutoffFrame, 0x78);
CCAMERAWORK_OFF(lastPreMotionFrame, 0x7c);
CCAMERAWORK_OFF(frameIndex, 0x80);
CCAMERAWORK_OFF(transform, 0x84);
CCAMERAWORK_OFF(previousTransform, 0xb4);
CCAMERAWORK_OFF(curveControlPoints, 0xe4);
CCAMERAWORK_OFF(motionStartPosition, 0x144);
CCAMERAWORK_OFF(targetRiseHeight, 0x150);
CCAMERAWORK_OFF(animationTime, 0x154);
CCAMERAWORK_OFF(lastShakeUpdateTime, 0x158);
CCAMERAWORK_OFF(shakeStartTime, 0x15c);
CCAMERAWORK_OFF(lastProximitySoundUpdateTime, 0x160);
CCAMERAWORK_OFF(unk_164, 0x164);
CCAMERAWORK_OFF(holeOutDetected, 0x165);
CCAMERAWORK_OFF(directionalSoundPlayed, 0x166);
CCAMERAWORK_OFF(ballProximitySoundState, 0x168);
CCAMERAWORK_OFF(heading, 0x16c);
CCAMERAWORK_OFF(rotation, 0x170);
CCAMERAWORK_OFF(randomModeRequested, 0x1a0);
CCAMERAWORK_OFF(shotCameraSelectionEnabled, 0x1a1);
CCAMERAWORK_OFF(groupAnimationActive, 0x1a3);
CCAMERAWORK_OFF(pivotPosition, 0x1a4);
CCAMERAWORK_OFF(animationGroup, 0x1b0);
CCAMERAWORK_OFF(playingAnimationGroup, 0x1b4);
CCAMERAWORK_OFF(initialBallPosition, 0x1b8);
CCAMERAWORK_OFF(startQuaternion, 0x1c4);
CCAMERAWORK_OFF(endQuaternion, 0x1d4);
CCAMERAWORK_OFF(startPosition, 0x1e4);
CCAMERAWORK_OFF(endPosition, 0x1f0);
CCAMERAWORK_OFF(horizontalBallDistance, 0x1fc);
CCAMERAWORK_OFF(ballHeightOffset, 0x200);
CCAMERAWORK_OFF(duration, 0x204);
CCAMERAWORK_OFF(fov, 0x208);
CCAMERAWORK_OFF(resetFovPending, 0x20c);
CCAMERAWORK_OFF(groundClearanceEnabled, 0x210);
CCAMERAWORK_OFF(forceGroundEffect, 0x211);
CCAMERAWORK_OFF(actionHistory, 0x218);
CCAMERAWORK_OFF(actionCount, 0x21c);
CCAMERAWORK_OFF(actionCapacity, 0x220);
CCAMERAWORK_OFF(historyIndex, 0x224);
CCAMERAWORK_OFF(randomValue, 0x228);
CCAMERAWORK_OFF(installed, 0x22c);
CCAMERAWORK_OFF(roll, 0x230);
CCAMERAWORK_OFF(powerShotVariant, 0x234);
CCAMERAWORK_OFF(pivot, 0x238);
CCAMERAWORK_OFF(unk_23C, 0x23c);
CCAMERAWORK_OFF(unk_240, 0x240);
CCAMERAWORK_OFF(useSavedCameraTransform, 0x241);
CCAMERAWORK_OFF(savedCameraRight, 0x244);
CCAMERAWORK_OFF(savedCameraUp, 0x250);
CCAMERAWORK_OFF(savedCameraForward, 0x25c);
CCAMERAWORK_OFF(savedCameraPosition, 0x268);
CCAMERAWORK_OFF(savedFov, 0x274);
CCAMERAWORK_OFF(characterModelMappings, 0x278);
CCAMERAWORK_OFF(indexedCameraGroups, 0x288);
CCAMERAWORK_OFF(powerShotMotionAliases, 0x298);
CCAMERAWORK_OFF(resourceGroupNames, 0x2a8);
CCAMERAWORK_OFF(awardMotionAliases, 0x2b8);
CCAMERAWORK_OFF(loadedCharacterResourceGroups, 0x2c8);
CCAMERAWORK_OFF(nestedCameraMap, 0x2d8);
CCAMERAWORK_OFF(cameraWorkTree, 0x2e8);
CCAMERAWORK_OFF(animationSegmentIndex, 0x2f8);
CCAMERAWORK_OFF(groupValueCount, 0x2fc);
#undef CCAMERAWORK_OFF
#endif
