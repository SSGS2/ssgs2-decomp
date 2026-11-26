// The camera actor. It follows the ball over the course, turns the view with the
// course heading, and blends a timed transition between the reference view and
// the forward ground view. It also turns controller and pointer input into
// camera rotation, height, movement and a ground target for a drop shot.
//
// Camera control messages and the actor queries below are synchronous engine
// interfaces: a query writes its answer into a stack word supplied by the caller.
#include "classes/CameraTransform800D.h"
#include "classes/CMouseCursor.h"
#include "iff/IffStruct.h"
#include "classes/InputPointerState.h"
#include "classes/CSharedDoc.h"
#include "classes/CQuadTree.h"
#include "classes/CGolfDoc.h"
#include "game/itemwindow.h"
#include "game/singletons.h"
#include "game/wvector.h"
#include "game/types.h"
#include "sdk_protos.h"
#include "classes/CCamera.h"

// The game state the camera reacts to. Some callers test one bit, others
// require the whole state word to be equal.
enum CameraState {
    kCameraAtReference = 8,
    kCameraForward = 0x100,
    kCameraFreeHeight = 0x200,
    kCameraNamedView = 0x8000,
    kCameraNeedsGroundTransition = 0x40000
};
enum CameraViewMode {
    kCameraFallbackMode = 1,
    kCameraForwardGroundMode = 3,
    kCameraMode4 = 4
};
// Replies to the actor queries used by this file. The HUD phase is 0 inactive,
// 1 shot, 2 aim, 3 items, 4 spin, 5 shot transition. Camera movement requires
// the aim phase and no transition dialog.
enum CameraQueryContract {
    kQueryHudPhase = 0x30,
    kQueryRuleTransitionDialog = 0x2A,
    kHudPhaseAim = 2,
    kQueryCameraOrBallControl = 0x42,
    kQueryDropMode = 0x56
};
// Actions are named for their effect here; the physical button bindings belong
// to the input manager.
enum CameraInputAction {
    kRaiseOrAdvanceAction = 0,
    kLowerOrRetreatAction = 1,
    kRotatePositiveAction = 2,
    kRotateNegativeAction = 3,
    kChooseDropTargetAction = 4,
    kAdvanceOrLowerAction = 9,
    kRetreatOrRaiseAction = 10,
    kCameraAction19 = 0x19
};

enum CameraControlMessage {
    kCameraSetDocumentView = 1,
    kCameraSetHeightRange = 2,
    kCameraClearLatch = 3,
    kCameraEnable = 4,
    kCameraDisable = 5,
    kCameraCenterHeight = 6,
    kCameraSetScalar = 7,
    kCameraGetInput = 8,
    kCameraBlockZoom = 9,
    kCameraSetTimedControl = 10,
    kCameraEnableDirections = 11,
    kCameraSnapGroundView = 12,
    kCameraSetHeading = 0x2717
};

// CGolfDoc keeps its camera matrix in a gap of the shared layout. The matrix is
// three axes followed by the camera position.
struct GolfDocCameraFields {
    char beforeMatrix[0x324];
    float right[3], up[3], forward[3], position[3];
};
// The instance table's word at 0x4c enables the forward ground view when its
// low bit is set.
struct CameraInstanceSettings {
    char beforeGroundViewFlags[0x4C];
    unsigned int groundViewFlags;
};
// One player record in the shared document; the stride is fixed.
struct SharedPlayerCameraFields {
    char beforeFlags[0x38];
    unsigned int flags;
    char rest[0x198];
};

union CameraMessageArgument {
    unsigned int word;
    int signedWord;
    void* pointer;
};
// A camera control message. The first argument carries a pointer for the
// setters that take one; the delivery time defers the message otherwise.
struct CameraControlEvent {
    unsigned int header;
    int type;
    CameraMessageArgument first, second;
    char reserved[0xC];
    unsigned int deliveryTime;
};
typedef char CameraControlEvent_argument_offset[(IFF_OFFSET(CameraControlEvent, first) == 8) ? 1 : -1];
typedef char CameraControlEvent_time_offset[(IFF_OFFSET(CameraControlEvent, deliveryTime) == 0x1C) ? 1 : -1];

// The camera is driven as an actor: a message that is not due yet is handed to
// the tenth virtual slot of the receiver.
struct CameraEventReceiver {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void slot8();
    virtual void slot9();
    virtual void slot10();
};

extern "C" {

extern void* g_mouseCursor;
extern int s_previousCameraRotationDirection, s_previousCameraHeightDirection, s_previousCameraMovementDirection;
extern char cameraActorStrings[];
extern float kCameraHeadingUpdatePeriod, kFloat_4_CameraRotation;
extern const float kCameraRotationRate;
extern int Pointer_GetButtonState(void*, int);
extern int GolfTask_NoOpStatus();

typedef Vec3f Vec3;
struct Vec3i {
    int x, y, z;
};

extern CSharedDoc* g_sharedDoc;
extern unsigned int g_currentTimeMilliseconds;
extern float kFloat_3_2_CameraPosition, kFloat_30_CameraPosition, kFloat_neg60_CameraPosition;
extern const float kFloat_0_camera;
extern float kCameraRotationEpsilon;
extern float kFloat_neg0_2617994_CameraTransition, kFloat_1_2_CameraTransition;
extern const float kFloat_0_5_Camera_805DDE20, kFloat_320_Camera, kFloat_10_Camera;
extern const float kFloat_1_BallPhysics;
extern float kFloat_25_6_CameraGroundTarget, kFloat_200_CameraActor;
extern const float kFloat_neg1_Camera;
extern signed char s_inputManagerInitialized;
extern void* g_inputPointer;
extern void* g_view;
extern void* g_soundManager;
extern void* g_helpMsg;
extern char s_cameraTransform[];
extern void* InstanceTable_Get();
extern float GolfParameter_ResolveFloat(void*, int);
extern void* GolfBall__Fv();
extern void __ml__FfRC7WVector(void*, void*, float);
extern void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern void* CQuadTree_GetInstance();
extern void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void Matrix_TransformDirection(void*, void*, void*);
extern void __apl__7WVectorFRC7WVector(void*, void*);
extern void Matrix_BuildCameraTransform(void*, void*, void*);
extern void Matrix_CopyAffine(void*, void*);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern void Vec3_DivideScalar(void*, float, void*);
extern float Math_SquareRootApprox(float);
extern void IActor_DispatchMessage(void*, void*, const char*, int, int, int, int, int);
extern void InputManager_Init(void*);
extern void InputManager_Dtor();
extern char s_gameInputManager[];
extern char s_gameInputManagerDtorRegistration_GolfInput[];
extern int InputManager_GetSelectedPlayer(void*);
extern int Input_IsActionButtonDown(void*, int, int);
extern int InputManager_RaiseCursorAppearance(void*, int, int);
extern int Input_IsActionPressed(void*, int, int);
extern void InputManager_GetPointerPosition(void*, float*, int);
extern void Camera_ScreenToRay(void*, void*, float, float, float);
// GetRayIntersection returns its Vec3f through a caller-supplied result buffer.
extern void GetRayIntersection__9CQuadTreeFPvPv(void* result, void* self, void* point, void* ray);
// The original passes these by value, which CodeWarrior turns into a pointer to a copy.
extern unsigned char GetGroundType__9CQuadTreeF7WVector(void*, Vec3);
extern int CQuadTree_OverlappedWithObject(void*, Vec3, float);
extern int CourseData_IsPointOutsidePlayableRegion(void*, void*);
extern float Magnitude__7WVectorCFv(void*);
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern void NoticeQueue_ShowById(void*, int, int);

} // extern "C"

// Borrow the current golf document.
static inline CGolfDoc* CameraGolfDocument() {
    return (CGolfDoc*)g_sharedDoc->golfDoc;
}

// The cursor region the pointer is over.
static inline int CursorHitRegion(CMouseCursor* cursor) {
    return cursor->hitRegion;
}

// Process one frame: gate on the active player, synchronize the camera mode and
// pointer UI, advance the ground-view transition, then either read input or
// finish the rotation that is still in progress.
void CCamera::OnProcess(float dt) {
    if (processingEnabled == 0)
        return;
    void* world = g_sharedDoc;
    void* view = CameraGolfDocument();

    void* player = (char*)world + ((CGolfDoc*)view)->currentPlayer * sizeof(SharedPlayerCameraFields);
    do {
        // A player record whose low flag bit is set skips this pass.
        switch (*(unsigned int*)((char*)player + IFF_OFFSET(SharedPlayerCameraFields, flags)) & 1) {
        case 0:
            break;
        default:
            continue;
        }
        {
            int mode;
            int old = this->mode;
            mode = ((CGolfDoc*)view)->cameraMode;
            this->mode = mode;
            if (mode != old)
                modeChanged = 1;
            if (Pointer_GetButtonState(g_inputPointer, 2) == 1) {
                if (CItemWindow::IsClosed()) {
                    int toggle = !radialMenuEnabled;
                    radialMenuEnabled = toggle;
                    ((CMouseCursor*)g_mouseCursor)->SetRadialMenuEnabled(toggle);
                    if (radialMenuEnabled)
                        IActor_DispatchMessage(this, this, cameraActorStrings, 0x12, 0, 0, 0, 0);
                }
            }
            CMouseCursor* cursor = (CMouseCursor*)g_mouseCursor;
            int state = CursorHitRegion(cursor);
            if (state == 0x16 && state == cursor->buttonDownRegions[0] &&
                Pointer_GetButtonState(g_inputPointer, 0) == 2)
                IActor_DispatchMessage(this, this, cameraActorStrings + 7, 7, 8, 0, 0, 0);
            if (GolfTask_NoOpStatus()) {
                float limit = kCameraHeadingUpdatePeriod;
                float t = headingUpdateTimer;
                t += dt;
                if ((headingUpdateTimer = t) > limit) {
                    headingUpdateTimer = t - limit;
                    if (headingChanged)
                        headingChanged = 0;
                }
            }
            // Advance the transition and the movement, then read input or finish the
            // rotation sweep that is still outstanding.
            UpdatePitchTransition(dt);
            HandleMovementInput(dt);
            view = CameraGolfDocument();
            if ((*(unsigned int*)&((CGolfDoc*)view)->gameStateFlags) & kCameraNamedView)
                return;
            float value = inputDelay;
            if (value > kFloat_0_camera) {
                inputDelay = value - dt;
                return;
            }
            float x;
            if ((*(unsigned int*)&remainingRotationAngle) & 0x80000000)
                x = -remainingRotationAngle;
            else
                x = remainingRotationAngle;
            // The comparison goes through temporaries; the aliases keep its order.
            __typeof__(x) _p479 = x;
            __typeof__(_p479 < kCameraRotationEpsilon) _p284 = _p479 < kCameraRotationEpsilon;
            __typeof__(_p284) _p288 = _p479 < kCameraRotationEpsilon;
            if ((_p288))
                UpdateInputDirections(dt);
            else {
                float yy;
                UpdateRotation((kFloat_4_CameraRotation * (kCameraRotationRate * dt)) *
                               (rotationDirection == 1 ? kFloat_1_BallPhysics : kFloat_neg1_Camera));
                if ((*(unsigned int*)&totalRotationAngle) & 0x80000000)
                    yy = -totalRotationAngle;
                else
                    yy = totalRotationAngle;
                __typeof__(yy) _p738 = yy;
                __typeof__(_p738 < kCameraRotationEpsilon) _p283 = _p738 < kCameraRotationEpsilon;
                __typeof__(yy) _p712 = yy;
                __typeof__(_p283) _p331 = _p712 < kCameraRotationEpsilon;
                bool capturedValue2 = yy < kCameraRotationEpsilon;
                __typeof__(yy < kCameraRotationEpsilon) _p978 = yy < kCameraRotationEpsilon;
                __typeof__(_p978) _p781 = yy < kCameraRotationEpsilon;
                __typeof__(yy) _p381 = yy;
                __typeof__(_p781) _p144 = _p381 < kCameraRotationEpsilon;
                float ratio = (_p144) ? kFloat_1_BallPhysics
                                      : kFloat_1_BallPhysics - (remainingRotationAngle / totalRotationAngle);
                IActor_DispatchMessage(this, this, cameraActorStrings + 0x10, 7, (int)&ratio, 0, 0, 0);
            }
            // Ramp each continuously held direction independently, clamped to [0, 1].
            int a = rotationDirection;
            if (s_previousCameraRotationDirection == a && a != 0) {
                float v = rotationInputRamp + dt;
                rotationInputRamp = v < kFloat_0_camera        ? kFloat_0_camera
                                    : v > kFloat_1_BallPhysics ? kFloat_1_BallPhysics
                                                               : v;
            } else
                rotationInputRamp = kFloat_0_camera;
            int b = heightDirection;
            s_previousCameraRotationDirection = rotationDirection;
            if (s_previousCameraHeightDirection == b && 0 != b) {
                float v = heightInputRamp + dt;
                heightInputRamp = v < kFloat_0_camera        ? kFloat_0_camera
                                  : v > kFloat_1_BallPhysics ? kFloat_1_BallPhysics
                                                             : v;
            } else
                heightInputRamp = kFloat_0_camera;
            int c = movementDirection;
            s_previousCameraHeightDirection = heightDirection;
            if (s_previousCameraMovementDirection == c && c != 0) {
                float v = movementInputRamp + dt;
                movementInputRamp = v < kFloat_0_camera        ? kFloat_0_camera
                                    : v > kFloat_1_BallPhysics ? kFloat_1_BallPhysics
                                                               : v;
            } else
                movementInputRamp = kFloat_0_camera;
            s_previousCameraMovementDirection = movementDirection;
        }
    } while (0);
}

// Follow the golf state through the forward ground-view transition. The
// transition starts when the state asks for it, advances by the frame time and
// keeps the camera above the terrain until its look target is rebuilt.
void CCamera::UpdatePitchTransition(float dt) {
    float z;
    void* view = CameraGolfDocument();
    if (((CGolfDoc*)view)->tutorialStage >= 1) {
        if (CItemWindow::IsClosed()) {
            float angle;
            if ((*(unsigned int*)&remainingRotationAngle) & 0x80000000u)
                angle = -remainingRotationAngle;
            else
                angle = remainingRotationAngle;
            bool capturedValue = angle < kCameraRotationEpsilon;
            if ((capturedValue)) {
                view = CameraGolfDocument();

                // Dispatch the state change and choose a smooth or immediate ground view.
                if (((CGolfDoc*)view)->cameraMode == kCameraForwardGroundMode) {
                    if ((*(unsigned int*)&((CGolfDoc*)view)->gameStateFlags) != kCameraNeedsGroundTransition) {
                        float a = GolfParameter_ResolveFloat(InstanceTable_Get(), 0) * kFloat_3_2_CameraPosition;
                        Vec3 q;
                        Vec3 v;
                        __ml__FfRC7WVector(&v, (char*)view + IFF_OFFSET(GolfDocCameraFields, forward), a);
                        __pl__FRC7WVectorRC7WVector(&q, GolfBall__Fv(), &v);
                        void* mgr = CQuadTree_GetInstance();

                        if (((CQuadTree*)mgr)->GetMeshIndex(&q, 0) == -1) {
                            CameraGolfDocument()->cameraMode = kCameraFallbackMode;
                            IActor_DispatchMessage(this, this, (const char*)cameraActorStrings + 7, 7, 8, 0, 0, 0);
                            return;
                        }
                    }
                    view = CameraGolfDocument();

                    switch (((CGolfDoc*)view)->gameStateFlags) {
                    case kCameraNeedsGroundTransition:
                        IActor_DispatchMessage(this, this, (const char*)cameraActorStrings + 7, 7, 8, 0, 0, 0);
                        break;
                    case kCameraForward:
                    case kCameraAtReference:
                        IActor_DispatchMessage(this, this, (const char*)cameraActorStrings + 7, 7,
                                               kCameraNeedsGroundTransition, 0, 0, 0);
                        SetupForwardGroundView(0);
                        break;
                    case kCameraFreeHeight:
                        ((CGolfDoc*)view)->cameraMode = kCameraMode4;
                        if (CameraGolfDocument()->dropRequest)
                            break;
                    case kCameraNamedView:
                        IActor_DispatchMessage(this, this, (const char*)cameraActorStrings + 7, 7,
                                               kCameraNeedsGroundTransition, 0, 0, 0);
                        SetupForwardGroundView(1);
                        break;
                    }
                }
                view = CameraGolfDocument();
                if ((*(unsigned int*)&((CGolfDoc*)view)->gameStateFlags) != kCameraNeedsGroundTransition)
                    return;
                view = InstanceTable_Get();
                if (*(unsigned int*)((char*)view + IFF_OFFSET(CameraInstanceSettings, groundViewFlags)) & 1)
                    SetupForwardGroundView(1);
                if (transitionActive == 0)
                    return;
                // Advance the active transition and clamp its elapsed time to the duration.
                float maxv = kFloat_1_2_CameraTransition;
                float t = transitionTime;
                t += dt;
                transitionTime = t;
                if (t > maxv) {
                    transitionActive = 0;
                    transitionTime = maxv;
                }

                Vec3 tmp8;
                transitionPitch = (transitionTime * (transitionEndPitch - transitionStartPitch) + transitionStartPitch);
                Vec3 a;
                __ml__FfRC7WVector(&a, transitionVelocity, transitionTime);
                __pl__FRC7WVectorRC7WVector(&tmp8, transitionStart, &a);
                __as__7WVectorFRC7WVector(position, &tmp8);
                view = CameraGolfDocument();
                // Keep the camera above the ground, rebuild the look target and publish
                // the camera transform.
                Vec3 tmp20;
                z = position[1];
                Vec3 xyz = (*(Vec3*)position);
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&tmp20, CQuadTree_GetInstance(), &xyz, 0, 0, 1);
                float f = tmp20.y + ((CGolfDoc*)view)->cameraGroundClearance;
                z = f < z ? z : f;
                Vec3 tmp8b;
                position[1] = z;
                __pl__FRC7WVectorRC7WVector(&tmp8b, position,
                                            (char*)CameraGolfDocument() + IFF_OFFSET(GolfDocCameraFields, forward));
                __as__7WVectorFRC7WVector((char*)&targetX, &tmp8b);
                targetY += (float)tan(transitionPitch);

                unsigned char tmp60[0x38];
                Matrix_BuildCameraTransform((Vec3*)tmp60, position, (char*)&targetX);
                Matrix_CopyAffine(s_cameraTransform, (Vec3*)tmp60);
            }
        }
    }
}

// Project the document's forward direction from the ball onto the ground and
// derive the forward ground-view endpoint. A nonzero snap writes the camera
// immediately; zero starts a timed transition.
void CCamera::SetupForwardGroundView(int snap) {
    int index;
    char t4[0x2C];
    Vec3 matrix;
    Vec3 q2;
    Vec3 q;
    Vec3 v;
    Vec3 out;
    Vec3 pos;
    Vec3 t3;
    Vec3 t2b;
    Vec3 t1;
    volatile char pad[0x20]; // the original reserves these stack bytes
    transitionActive = (snap == 0);
    float a = GolfParameter_ResolveFloat(InstanceTable_Get(), 0) * kFloat_3_2_CameraPosition;
    __ml__FfRC7WVector(&v, ((GolfDocCameraFields*)CameraGolfDocument())->forward, a);
    void* x = GolfBall__Fv();
    __pl__FRC7WVectorRC7WVector(&q, x, &v);
    *(Vec3i*)&q2 = *(Vec3i*)&q;
    void* m = CQuadTree_GetInstance();
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&matrix, m, &q2, 1, (int)&index, 1);
    __as__7WVectorFRC7WVector(transitionEnd, &matrix);
    if (index == -1) {
        if (snap) {
            CameraGolfDocument()->cameraMode = kCameraFallbackMode;
            IActor_DispatchMessage(this, this, (const char*)cameraActorStrings + 7, 7, 8, 0, 0, 0);
        }
        return;
    }
    // Raise and pull back the endpoint, and record its pitch.
    void* view = CameraGolfDocument();
    pos.x = kFloat_0_camera;
    pos.y = kFloat_30_CameraPosition;
    pos.z = kFloat_neg60_CameraPosition;
    Matrix_TransformDirection(&out, &pos, (char*)view + IFF_OFFSET(GolfDocCameraFields, right));
    __apl__7WVectorFRC7WVector(transitionEnd, &out);
    transitionEndPitch = kFloat_neg0_2617994_CameraTransition;
    if (snap) {
        __as__7WVectorFRC7WVector(position, transitionEnd);
        __pl__FRC7WVectorRC7WVector(&t3, transitionEnd, ((GolfDocCameraFields*)CameraGolfDocument())->forward);
        __as__7WVectorFRC7WVector((char*)&targetX, &t3);
        float t2 = (float)tan((double)transitionEndPitch);
        targetY = targetY + t2;
        Matrix_BuildCameraTransform((Vec3*)t4, position, (char*)&targetX);
        Matrix_CopyAffine(s_cameraTransform, (Vec3*)t4);
    } else {
        __as__7WVectorFRC7WVector(transitionStart, AsCameraTransform800D(s_cameraTransform)->position);
        transitionStartPitch = (float)atan(
            (double)(*(float*)(&AsCameraTransform800D(s_cameraTransform)->basis[7]) /
                     Math_SquareRootApprox(kFloat_1_BallPhysics -
                                           *(float*)(&AsCameraTransform800D(s_cameraTransform)->basis[7]) *
                                               *(float*)(&AsCameraTransform800D(s_cameraTransform)->basis[7]))));
        __mi__FRC7WVectorRC7WVector(&t1, transitionEnd, transitionStart);
        Vec3_DivideScalar(&t2b, kFloat_1_2_CameraTransition, &t1);
        __as__7WVectorFRC7WVector(transitionVelocity, &t2b);
        transitionTime = kFloat_0_camera;
    }
}

// Translate the permitted controller actions into camera rotation, height,
// movement and drop targeting. The actor queries borrow stack outputs
// synchronously: the screen reports its HUD phase and the golf rule reports
// whether a transition dialog exists. No input runs while that dialog exists.
void CCamera::UpdateInputDirections(float dt) {
    int rotateNegativeHeld, rotatePositiveHeld;
    void* inputManager;
    int action19Released;
    int movementModeEnabled;
    int hudPhase, transitionDialogActive;
    double pointerPositionStorage; // two float pointer coordinates in the aligned slot
    unsigned char actorDirectionBlock;
    int cameraOrBallControlActive;
    int dropModeActive;

    // Clear this frame's directions, then gate input on the transition and item state.
    rotationDirection = 0;
    heightDirection = 0;
    movementDirection = 0;
    do {
        if ((*(unsigned int*)&CameraGolfDocument()->gameStateFlags) & kCameraNeedsGroundTransition) {
            if (transitionActive != 0)
                break;
        }
        if (CItemWindow::IsClosed() == 0)
            break;
        if (((int&)GetInputManagerSlot()) == 0) {
            if (s_inputManagerInitialized == 0) {
                InputManager_Init(&s_gameInputManager);
                __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor,
                                         &s_gameInputManagerDtorRegistration_GolfInput);
                s_inputManagerInitialized = 1;
            }
            ((int&)GetInputManagerSlot()) = (int)&s_gameInputManager;
        }
        inputManager = (void*)((int&)GetInputManagerSlot());
        ((CMouseCursor*)g_mouseCursor)->GetInputPosition((float*)&pointerPositionStorage);
        previousInput = (int)currentInput;
        currentInput = 0;
        action19Released = (Input_IsActionButtonDown(inputManager, kCameraAction19, -1) == 0);
        rotateNegativeHeld = Input_IsActionButtonDown(inputManager, kRotateNegativeAction, -1);
        rotatePositiveHeld = Input_IsActionButtonDown(inputManager, kRotatePositiveAction, -1);
        movementModeEnabled = 0;
        if (mode != 0 && mode != kCameraMode4)
            movementModeEnabled = 1;
        IActor_DispatchMessage(this, this, cameraActorStrings, kQueryHudPhase, (int)&hudPhase, 0, 0, 0);
        IActor_DispatchMessage(this, this, cameraActorStrings + 7, kQueryRuleTransitionDialog,
                               (int)&transitionDialogActive, 0, 0, 0);
        if (transitionDialogActive != 0)
            break;
        // Map held actions to movement, height and rotation according to the camera state.
        if (hudPhase == kHudPhaseAim) {
            if (action19Released) {
                if (CameraGolfDocument()->cameraMode != 0) {
                    int n = InputManager_GetSelectedPlayer(inputManager);
                    InputManager_RaiseCursorAppearance(inputManager, 4, n);
                    currentInput = 1;
                }
            }
            if (movementModeEnabled && zoomInputBlocked == 0) {
                if (Input_IsActionButtonDown(inputManager, kAdvanceOrLowerAction, -1) != 0) {
                    currentInput = 4;
                    if ((*(unsigned int*)&CameraGolfDocument()->gameStateFlags) == kCameraFreeHeight) {
                        if (heightRatio > kFloat_0_camera) {
                            float mv = kFloat_neg1_Camera * dt;
                            heightDirection = -1;
                            mv *= kFloat_1_BallPhysics;
                            UpdateVerticalMovement(mv);
                        }
                    } else {
                        movementDirection = 1;
                        UpdateForwardMovement(dt * kFloat_1_BallPhysics);
                    }
                } else if (Input_IsActionButtonDown(inputManager, kRetreatOrRaiseAction, -1) != 0) {
                    currentInput = 5;
                    if ((*(unsigned int*)&CameraGolfDocument()->gameStateFlags) == kCameraFreeHeight) {
                        if (heightRatio < kFloat_1_BallPhysics) {
                            heightDirection = 1;
                            UpdateVerticalMovement(dt * kFloat_1_BallPhysics);
                        }
                    } else {
                        float mv2 = kFloat_neg1_Camera * dt;
                        movementDirection = -1;
                        mv2 *= kFloat_1_BallPhysics;
                        UpdateForwardMovement(mv2);
                    }
                }
            }
            if (directionalInputEnabled != 0 || CameraGolfDocument()->tutorialStage == 6) {
                if (action19Released || rotateNegativeHeld || rotatePositiveHeld) {
                    if (rotatePositiveHeld) {
                        rotationDirection = 1;
                        currentInput = 7;
                        UpdateRotation(kCameraRotationRate * -dt);
                    } else if (rotateNegativeHeld) {
                        rotationDirection = -1;
                        currentInput = 8;
                        UpdateRotation(kCameraRotationRate * dt);
                    }
                }
                if (action19Released && movementModeEnabled) {
                    if (action19Released && Input_IsActionButtonDown(inputManager, kRaiseOrAdvanceAction, -1) != 0) {
                        currentInput = 2;
                        if ((*(unsigned int*)&CameraGolfDocument()->gameStateFlags) == kCameraFreeHeight) {
                            movementDirection = 1;
                            UpdateForwardMovement(dt);
                        } else if (heightRatio < kFloat_1_BallPhysics) {
                            heightDirection = 1;
                            UpdateVerticalMovement(dt);
                        }
                    }
                    if (action19Released && Input_IsActionButtonDown(inputManager, kLowerOrRetreatAction, -1) != 0) {
                        currentInput = 3;
                        if ((*(unsigned int*)&CameraGolfDocument()->gameStateFlags) == kCameraFreeHeight) {
                            movementDirection = -1;
                            UpdateForwardMovement(kFloat_neg1_Camera * dt);
                        } else if (heightRatio > kFloat_0_camera) {
                            heightDirection = -1;
                            UpdateVerticalMovement(kFloat_neg1_Camera * dt);
                        }
                    }
                }
            }
        }
        // Drop targeting needs free-height state, idle movement and every actor permission.
        IActor_DispatchMessage(this, this, (const char*)cameraActorStrings + 0x15, 0x0D, (int)&actorDirectionBlock, 0,
                               0, 0);
        int canChooseDropTarget = (int)(((*(unsigned int*)&CameraGolfDocument()->gameStateFlags) >> 9) & 1);
        canChooseDropTarget &= (heightDirection == 0);
        canChooseDropTarget &= (movementDirection == 0);
        canChooseDropTarget &= (hudPhase == kHudPhaseAim);
        IActor_DispatchMessage(this, this, cameraActorStrings, kQueryCameraOrBallControl,
                               (int)&cameraOrBallControlActive, 0, 0, 0);
        dropModeActive = 0;
        IActor_DispatchMessage(this, this, cameraActorStrings, kQueryDropMode, (int)&dropModeActive, 0, 0, 0);
        canChooseDropTarget &= (cameraOrBallControlActive == 0);
        canChooseDropTarget &= (dropModeActive != 0);
        canChooseDropTarget &= (Input_IsActionPressed(inputManager, kChooseDropTargetAction, -1) != 0);
        canChooseDropTarget &= (CameraGolfDocument()->tutorialStage >= 1);
        canChooseDropTarget &= (actorDirectionBlock == 0);
        if (canChooseDropTarget)
            UpdatePointerGroundTarget(dt);
    } while (0);
}

// Raycast the pointer onto the course and accept a legal drop target, otherwise
// play the rejection cue. Ground code 0 skips the region and distance tests;
// code 1 must be in the playable region and farther from the hole than the
// ball's horizontal distance minus 25.6. Other ground codes and object hits
// reject the drop. The ball's height offset at +0xC0 is added to the hit point.
void CCamera::UpdatePointerGroundTarget(float dt) {
    Vec3 targetFromHoleXZ, ballFromHoleXZ;
    struct {
        float x, y, z, w;
    } v;
    Vec3 ray, dropTargetWorld, rayDir, groundIntersection, originCopy, rayCopy, ballFromHole, holePosition,
        targetFromHole, holePositionAgain;
    int initialized = ((int&)GetInputManagerSlot());
    *(Arr3*)&v = *(Arr3*)((InputPointerState*)g_inputPointer)->position;
    if (!initialized) {
        if (!s_inputManagerInitialized) {
            InputManager_Init((void*)s_gameInputManager);
            __register_global_object((void*)s_gameInputManager, (void*)InputManager_Dtor,
                                     (void*)s_gameInputManagerDtorRegistration_GolfInput);
            s_inputManagerInitialized = 1;
        }
        ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
    }
    void* cursor = (void*)((int&)GetInputManagerSlot());
    float xy[2];
    int n = InputManager_GetSelectedPlayer(cursor);
    InputManager_GetPointerPosition(cursor, xy, n);
    v.x = xy[0];
    v.y = xy[1];
    // Turn the screen coordinates into a world ray that starts at the camera.
    Camera_ScreenToRay(&rayDir, g_view, xy[0], xy[1], kFloat_neg1_Camera);
    __as__7WVectorFRC7WVector(&ray, &rayDir);
    *(Arr3*)&rayCopy = *(Arr3*)&ray;
    *(Arr3*)&originCopy = *(Arr3*)(AsCameraTransform800D(s_cameraTransform)->position);
    GetRayIntersection__9CQuadTreeFPvPv(&groundIntersection, CQuadTree_GetInstance(), &originCopy, &rayCopy);
    __as__7WVectorFRC7WVector(&dropTargetWorld, &groundIntersection);
    float* ballState = (float*)GolfBall__Fv();
    float ballTargetHeightOffset = ballState[0xC0 / 4];
    dropTargetWorld.y = dropTargetWorld.y + ballTargetHeightOffset;
    // Compare the horizontal hole-relative distances, then accept or reject the drop.
    if (CameraGolfDocument()->dropRequest) {
        *(Arr3*)&holePosition =
            *(Arr3*)((char*)CameraGolfDocument()->holeRecords + CameraGolfDocument()->currentHoleIndex * 32 + 0xC);
        __mi__FRC7WVectorRC7WVector(&ballFromHole, GolfBall__Fv(), &holePosition);
        __as__7WVectorFRC7WVector(&ballFromHoleXZ, &ballFromHole);
        ballFromHoleXZ.y = kFloat_0_camera;
        *(Arr3*)&holePositionAgain =
            *(Arr3*)((char*)CameraGolfDocument()->holeRecords + CameraGolfDocument()->currentHoleIndex * 32 + 0xC);
        __mi__FRC7WVectorRC7WVector(&targetFromHole, &dropTargetWorld, &holePositionAgain);
        __as__7WVectorFRC7WVector(&targetFromHoleXZ, &targetFromHole);
        targetFromHoleXZ.y = kFloat_0_camera;
        unsigned char groundType = GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), dropTargetWorld);
        do {
            if (!CQuadTree_OverlappedWithObject(CQuadTree_GetInstance(), dropTargetWorld, kFloat_0_camera)) {
                if (groundType) {
                    if (groundType != 1)
                        break;
                    void* mgr = CameraGolfDocument()->courseData;
                    if (CourseData_IsPointOutsidePlayableRegion(mgr, &dropTargetWorld) != 0)
                        break;
                    float d1 = Magnitude__7WVectorCFv(&ballFromHoleXZ) - kFloat_25_6_CameraGroundTarget;
                    float d2 = Magnitude__7WVectorCFv(&targetFromHoleXZ);
                    if (!(d2 > d1))
                        break;
                }
                CameraGolfDocument()->dropRequest = 0;
                IActor_DispatchMessage(this, this, (const char*)cameraActorStrings + 7, 0x11, (int)&dropTargetWorld, 0,
                                       0, 0);
                return;
            }
        } while (0);
        Sound_PlayByName(g_soundManager, (const char*)cameraActorStrings + 0x1B, 0, 0, 0, 0, kFloat_0_5_Camera_805DDE20,
                         kFloat_200_CameraActor, kFloat_1_BallPhysics);
        if (g_helpMsg)
            NoticeQueue_ShowById(g_helpMsg, 0, 0);
    }
}

#pragma opt_lifetimes off
// Apply camera control messages whose delivery time has passed; defer the rest
// through the receiver's tenth virtual slot.
void CCamera::HandleMsg(const CameraControlEvent* event) {
    unsigned int deliveryTime = event->deliveryTime;
    if (deliveryTime > g_currentTimeMilliseconds) {
        ((CameraEventReceiver*)this)->slot10();
        return;
    }
    switch (event->type) {
    case kCameraCenterHeight:
        heightOffsetRatio = kFloat_0_camera;
        heightRatio = kFloat_0_5_Camera_805DDE20;
        break;
    case kCameraSetHeading: {
        headingUpdateTimer = kFloat_0_camera;
        void* view = CameraGolfDocument();
        ((CGolfDoc*)view)->headingRadians = *(float*)event->first.pointer;
        break;
    }
    case kCameraSetDocumentView: {
        (*(WVector*)position) = *(WVector*)(((GolfDocCameraFields*)CameraGolfDocument())->position);

        // The look point is the camera position plus its forward direction; the
        // pitch then adds tan(radians) on Y.
        WVector v = *(WVector*)(((GolfDocCameraFields*)CameraGolfDocument())->position) +
                    *(WVector*)(((GolfDocCameraFields*)CameraGolfDocument())->forward);
        (*(WVector*)&targetX) = v;
        targetY += (float)tan((double)CameraGolfDocument()->cameraPitchRadians);
        heightOffsetRatio = kFloat_0_camera;
        groundHeightRatio =
            (((GolfDocCameraFields*)CameraGolfDocument())->position[1] - minHeight) / (maxHeight - minHeight);
        heightRatio = groundHeightRatio;
        referenceHeightRatio =
            (((GolfDocCameraFields*)CameraGolfDocument())->position[1] - minHeight) / (maxHeight - minHeight);
        break;
    }
    case kCameraSetHeightRange: {
        float v = ((CQuadTree*)CQuadTree_GetInstance())->minimumGroundHeight;
        minHeight = v;
        maxHeight = kFloat_320_Camera + v;
        break;
    }
    case kCameraClearLatch:
        useLinearHeightInterpolation = 0;
        break;
    case kCameraEnable:
        processingEnabled = 1;
        break;
    case kCameraDisable:
        processingEnabled = 0;
        break;
    case kCameraSetScalar:
        unk_8C = *(float*)event->first.pointer;
        break;
    case kCameraGetInput: {
        void* p = event->first.pointer;
        *(unsigned int*)p = currentInput;
        break;
    }
    case kCameraBlockZoom:
        zoomInputBlocked = event->first.word;
        break;
    case kCameraSetTimedControl: {
        // Store the control mode and convert its duration from tenths to whole time units.
        timedControlMode = event->first.word;
        timedControlDuration = (float)event->second.signedWord / kFloat_10_Camera;
        break;
    }
    case kCameraEnableDirections:
        directionalInputEnabled = event->first.word;
        break;
    case kCameraSnapGroundView: {
        void* view = CameraGolfDocument();
        if ((*(unsigned int*)&((CGolfDoc*)view)->gameStateFlags) == kCameraNeedsGroundTransition)
            SetupForwardGroundView(1);
        break;
    }
    default:
        break;
    }
}
#pragma opt_lifetimes reset
