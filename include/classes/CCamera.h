// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCAMERA_H
#define CLASSES_CCAMERA_H

#include "game/types.h"

class CCamera {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    int rotationDirection;                         // 0x18: 3 uses in CCamera_OnProcess
    float totalRotationAngle;                       // 0x1c: 5 uses in CCamera_OnProcess, CCamera_OnInit
    float remainingRotationAngle;                       // 0x20: 5 uses in CCamera_OnProcess, CCamera_OnInit
    int heightDirection;                         // 0x24: 2 uses in CCamera_OnProcess
    int movementDirection;                         // 0x28: 2 uses in CCamera_OnProcess
    float rotationInputRamp;                       // 0x2c: 4 uses in CCamera_OnProcess, CCamera_OnInit
    float heightInputRamp;                       // 0x30: 4 uses in CCamera_OnProcess, CCamera_OnInit
    float movementInputRamp;                       // 0x34: 4 uses in CCamera_OnProcess, CCamera_OnInit
    float position[3];                 // 0x38: CCamera_HandleMovementInput assigns the camera position
    float targetX;                      // 0x44
    float targetY;                      // 0x48: 1 use in CCamera_HandleMsg
    float targetZ;                      // 0x4c: third component of the target at 0x44
    float heightRatio;                       // 0x50: 3 uses in CCamera_HandleMsg, CCamera_OnInit
    float heightOffsetRatio;                       // 0x54: 3 uses in CCamera_HandleMsg, CCamera_OnInit
    float maxHeight;                    // 0x58: 3 uses in CCamera_HandleMsg
    float minHeight;                    // 0x5c: 5 uses in CCamera_HandleMsg
    unsigned char useLinearHeightInterpolation;               // 0x60: 2 uses in CCamera_OnInit, CCamera_HandleMsg
    char pad_061[0x3];                  // 0x61
    float referenceHeightRatio;                       // 0x64: 1 use in CCamera_HandleMsg
    float groundHeightRatio;                       // 0x68: 2 uses in CCamera_HandleMsg
    unsigned char processingEnabled;    // 0x6c: 4 uses in CCamera_HandleMsg, CCamera_OnInit, CCamera_OnProcess
    char pad_06D[0x3];                  // 0x6d
    float inputDelay;                   // 0x70: 3 uses in CCamera_OnProcess, CCamera_OnInit
    unsigned char radialMenuEnabled;               // 0x74: 4 uses in CCamera_OnProcess, CCamera_OnInit
    char pad_075[0x3];
    float horizontalOffset;             // 0x78: CCamera_HandleMovementInput clamps horizontal movement
    float verticalOffset;               // 0x7c: CCamera_HandleMovementInput clamps vertical movement
    float headingUpdateTimer;                       // 0x80: 5 uses in CCamera_OnProcess, CCamera_OnInit, CCamera_HandleMsg
    unsigned char headingChanged;               // 0x84: 3 uses in CCamera_OnProcess, CCamera_OnInit
    char pad_085[0x7];                  // 0x85
    float unk_8C;                       // 0x8c: 2 uses in CCamera_OnInit, CCamera_HandleMsg
    float transitionStart[3];          // 0x90: CCamera_SetupForwardGroundView copies camera translation; CCamera_UpdatePitchTransition interpolates
    float transitionStartPitch;        // 0x9c: CCamera_SetupForwardGroundView derives atan from camera direction
    float transitionEnd[3];            // 0xa0: CCamera_SetupForwardGroundView ground target plus transformed offset
    float transitionEndPitch;          // 0xac: CCamera_SetupForwardGroundView sets kFloat_neg0_2617994_CameraTransition
    float transitionVelocity[3];       // 0xb0: CCamera_SetupForwardGroundView scales end minus start
    float transitionPitch;             // 0xbc: CCamera_UpdatePitchTransition interpolates start/end pitch
    float transitionTime;              // 0xc0: CCamera_UpdatePitchTransition adds dt, clamps to kFloat_1_2_CameraTransition
    unsigned char transitionActive;    // 0xc4: CCamera_UpdatePitchTransition gates and ends transition
    char pad_0C5[0x1b];
    int modeChanged;                         // 0xe0: 2 uses in CCamera_OnInit, CCamera_OnProcess
    int mode;                         // 0xe4: 3 uses in CCamera_OnProcess, CCamera_OnInit
    unsigned int currentInput;          // 0xe8: 2 uses in CCamera_OnInit, CCamera_HandleMsg
    int previousInput;                 // 0xec: CCamera_UpdateInputDirections copies event code at +0xe8
    unsigned int zoomInputBlocked;                // 0xf0: 3 uses in CCamera_OnInit, CCamera_HandleMsg, CCamera_dtor
    unsigned int timedControlMode;                // 0xf4: 2 uses in CCamera_OnInit, CCamera_HandleMsg
    float timedControlDuration;                       // 0xf8: 2 uses in CCamera_OnInit, CCamera_HandleMsg
    unsigned int directionalInputEnabled;                // 0xfc: 3 uses in CCamera_OnInit, CCamera_HandleMsg, CCamera_dtor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCAMERA_OFF(f, o) typedef char CCamera_off_##f[((int)&((CCamera*)0)->f == (o)) ? 1 : -1]
CCAMERA_OFF(vtbl, 0x0);
CCAMERA_OFF(position, 0x38);
CCAMERA_OFF(targetX, 0x44);
CCAMERA_OFF(targetZ, 0x4c);
CCAMERA_OFF(horizontalOffset, 0x78);
CCAMERA_OFF(verticalOffset, 0x7c);
CCAMERA_OFF(rotationDirection, 0x18);
CCAMERA_OFF(totalRotationAngle, 0x1c);
CCAMERA_OFF(remainingRotationAngle, 0x20);
CCAMERA_OFF(heightDirection, 0x24);
CCAMERA_OFF(movementDirection, 0x28);
CCAMERA_OFF(rotationInputRamp, 0x2c);
CCAMERA_OFF(heightInputRamp, 0x30);
CCAMERA_OFF(movementInputRamp, 0x34);
CCAMERA_OFF(targetY, 0x48);
CCAMERA_OFF(heightRatio, 0x50);
CCAMERA_OFF(heightOffsetRatio, 0x54);
CCAMERA_OFF(maxHeight, 0x58);
CCAMERA_OFF(minHeight, 0x5c);
CCAMERA_OFF(useLinearHeightInterpolation, 0x60);
CCAMERA_OFF(referenceHeightRatio, 0x64);
CCAMERA_OFF(groundHeightRatio, 0x68);
CCAMERA_OFF(processingEnabled, 0x6c);
CCAMERA_OFF(inputDelay, 0x70);
CCAMERA_OFF(radialMenuEnabled, 0x74);
CCAMERA_OFF(headingUpdateTimer, 0x80);
CCAMERA_OFF(headingChanged, 0x84);
CCAMERA_OFF(unk_8C, 0x8c);
CCAMERA_OFF(modeChanged, 0xe0);
CCAMERA_OFF(mode, 0xe4);
CCAMERA_OFF(currentInput, 0xe8);
CCAMERA_OFF(zoomInputBlocked, 0xf0);
CCAMERA_OFF(timedControlMode, 0xf4);
CCAMERA_OFF(timedControlDuration, 0xf8);
CCAMERA_OFF(directionalInputEnabled, 0xfc);
CCAMERA_OFF(transitionStart, 0x90);
CCAMERA_OFF(transitionStartPitch, 0x9C);
CCAMERA_OFF(transitionEnd, 0xA0);
CCAMERA_OFF(transitionEndPitch, 0xAC);
CCAMERA_OFF(transitionVelocity, 0xB0);
CCAMERA_OFF(transitionPitch, 0xBC);
CCAMERA_OFF(transitionTime, 0xC0);
CCAMERA_OFF(transitionActive, 0xC4);
CCAMERA_OFF(previousInput, 0xEC);
#undef CCAMERA_OFF

#endif  // CLASSES_CCAMERA_H

