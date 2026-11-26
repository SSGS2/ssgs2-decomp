// CCamera: the in-game camera actor.
//
// It follows the ball over the course, swings around the reference view, and
// blends a timed transition between the reference view and the forward ground
// view. It also turns controller input into camera rotation, height, movement
// and a ground target for the drop shot.
//
// The class is declared without virtual functions on purpose: its vtable is
// still taken from the original game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CCAMERA_H
#define CLASSES_CCAMERA_H

#include "game/types.h"

struct CameraControlEvent;

class CCamera {
public:
    ~CCamera();

    // Ground view.
    void SetupForwardGroundView(int snap);
    void UpdatePitchTransition(float deltaSeconds);

    // Input.
    void UpdateInputDirections(float deltaSeconds);
    void UpdatePointerGroundTarget(float deltaSeconds);
    void HandleMsg(const CameraControlEvent* event);
    void OnProcess(float deltaSeconds);

    void* vtbl;                                 // 0x00: original vtable pointer
    char pad_004[0x4];                          // 0x04
    int displayPriority;                        // 0x08: draw priority of the actor base
    char pad_00C[0xC];                          // 0x0c
    int rotationDirection;                      // 0x18: -1, 0 or 1 while a rotate action is held
    float totalRotationAngle;                   // 0x1c
    float remainingRotationAngle;               // 0x20: part of a requested turn still to apply
    int heightDirection;                        // 0x24
    int movementDirection;                      // 0x28
    float rotationInputRamp;                    // 0x2c
    float heightInputRamp;                      // 0x30
    float movementInputRamp;                    // 0x34
    float position[3];                          // 0x38
    float targetX;                              // 0x44: look target; its three components are contiguous
    float targetY;                              // 0x48
    float targetZ;                              // 0x4c
    float heightRatio;                          // 0x50
    float heightOffsetRatio;                    // 0x54
    float maxHeight;                            // 0x58
    float minHeight;                            // 0x5c
    unsigned char useLinearHeightInterpolation; // 0x60
    char pad_061[0x3];                          // 0x61
    float referenceHeightRatio;                 // 0x64
    float groundHeightRatio;                    // 0x68
    unsigned char processingEnabled;            // 0x6c
    char pad_06D[0x3];                          // 0x6d
    float inputDelay;                           // 0x70: input is ignored while this is positive
    unsigned char radialMenuEnabled;            // 0x74
    char pad_075[0x3];                          // 0x75
    float horizontalOffset;                     // 0x78: target pan along the camera right axis
    float verticalOffset;                       // 0x7c: target pan along the camera up axis
    float headingUpdateTimer;                   // 0x80
    unsigned char headingChanged;               // 0x84
    char pad_085[0x7];                          // 0x85
    float unk_8C;                               // 0x8c: only stored by the scalar control message
    float transitionStart[3];                   // 0x90
    float transitionStartPitch;                 // 0x9c
    float transitionEnd[3];                     // 0xa0
    float transitionEndPitch;                   // 0xac
    float transitionVelocity[3];                // 0xb0
    float transitionPitch;                      // 0xbc
    float transitionTime;                       // 0xc0
    bool transitionActive;                      // 0xc4
    char pad_0C5[0x1B];                         // 0xc5
    int modeChanged;                            // 0xe0
    int mode;                                   // 0xe4
    unsigned int currentInput;                  // 0xe8: action code for this frame
    int previousInput;                          // 0xec
    unsigned int zoomInputBlocked;              // 0xf0
    unsigned int timedControlMode;              // 0xf4
    float timedControlDuration;                 // 0xf8
    unsigned int directionalInputEnabled;       // 0xfc

    void UpdateRotation(float dt);
    void UpdateForwardMovement(float dt);
    void UpdateVerticalMovement(float dt);
    void HandleMovementInput(float dt);
    void AddHeadingAngle(float headingDelta);
    static void* GetRTTI();

    void OnInit();
};

#define CCAMERA_OFF(f, o) typedef char CCamera_off_##f[((int)&((CCamera*)0)->f == (o)) ? 1 : -1]
CCAMERA_OFF(displayPriority, 0x08);
CCAMERA_OFF(rotationDirection, 0x18);
CCAMERA_OFF(rotationInputRamp, 0x2c);
CCAMERA_OFF(position, 0x38);
CCAMERA_OFF(targetX, 0x44);
CCAMERA_OFF(heightRatio, 0x50);
CCAMERA_OFF(processingEnabled, 0x6c);
CCAMERA_OFF(headingUpdateTimer, 0x80);
CCAMERA_OFF(unk_8C, 0x8c);
CCAMERA_OFF(transitionStart, 0x90);
CCAMERA_OFF(transitionEnd, 0xa0);
CCAMERA_OFF(transitionVelocity, 0xb0);
CCAMERA_OFF(transitionTime, 0xc0);
CCAMERA_OFF(transitionActive, 0xc4);
CCAMERA_OFF(mode, 0xe4);
CCAMERA_OFF(currentInput, 0xe8);
CCAMERA_OFF(directionalInputEnabled, 0xfc);
#undef CCAMERA_OFF
typedef char CCamera_size[(sizeof(CCamera) == 0x100) ? 1 : -1];

#endif // CLASSES_CCAMERA_H
