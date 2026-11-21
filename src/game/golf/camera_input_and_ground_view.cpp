#include "classes/CameraTransform800D.h"
#include "classes/CMouseCursor.h"
#include "iff/IffStruct.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/InputPointerState.h"
#include "classes/CSharedDoc.h"
#include "classes/CQuadTree.h"
#define AsCQuadTree(p) ((CQuadTree*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "game/itemwindow.h"
#include "game/singletons.h"
#include "game/wvector.h"
#include "game/types.h"
// Translation unit 0x800D2860-0x800D3DB4 (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "classes/CCamera.h"

#define AsCCamera(p) ((CCamera*)(p))

// Same state vocabulary as camera_movement_and_input.cpp; some callers test a bit, others require exact state equality.
enum CameraState {
    kCameraAtReference = 8, kCameraForward = 0x100, kCameraFreeHeight = 0x200,
    kCameraNamedView = 0x8000, kCameraNeedsGroundTransition = 0x40000
};
enum CameraViewMode {
    kCameraFallbackMode = 1, kCameraForwardGroundMode = 3, kCameraMode4 = 4
};
// Synchronous actor-query contracts, verified in CScreen_HandleMsg and
// CGolfRule_HandleMsg: 0x30 writes HudPhase (0 inactive, 1 shot, 2 aim,
// 3 items, 4 spin, 5 shot transition); 0x2A writes whether a transition
// dialog exists. Camera movement requires aim phase and no such dialog.
enum CameraQueryContract { kQueryHudPhase = 0x30, kQueryRuleTransitionDialog = 0x2A,
    kHudPhaseAim = 2, kQueryCameraOrBallControl = 0x42, kQueryDropMode = 0x56 };
// Actions are named for their effect in this unit; physical-button bindings belong to InputManager.
enum CameraInputAction {
    kRaiseOrAdvanceAction = 0, kLowerOrRetreatAction = 1,
    kRotatePositiveAction = 2, kRotateNegativeAction = 3, kChooseDropTargetAction = 4,
    kAdvanceOrLowerAction = 9, kRetreatOrRaiseAction = 10, kCameraAction19 = 0x19
};

extern "C" {

extern void* g_mouseCursor;
extern int s_previousCameraRotationDirection, s_previousCameraHeightDirection, s_previousCameraMovementDirection;
extern char cameraActorStrings[];
extern float kCameraHeadingUpdatePeriod, kFloat_4_CameraRotation;
extern const float kCameraRotationRate;
extern int Pointer_GetButtonState(void*, int);
extern int GolfTask_NoOpStatus();
extern void CMouseCursor_SetRadialMenuEnabled(void*, int);
extern void CCamera_HandleMovementInput(void*, float);
extern void CCamera_UpdateVerticalMovement(void*, float);
extern void CCamera_UpdateForwardMovement(void*, float);
extern void CCamera_UpdatePointerGroundTarget(void*, float);
extern void CCamera_UpdateInputDirections(void*, float);
extern void CCamera_UpdateRotation(void*, float);

typedef Vec3f Vec3;
struct Vec3i { int x, y, z; };
// Preserve Vec3 assignment and the bool store used by the original transition helper.
struct CameraTransitionFields {
    unsigned char pad0[0x38];
    Vec3 position;
    Vec3 target;
    unsigned char pad1[0x40];
    Vec3 transitionStart;
    float transitionStartPitch;
    Vec3 transitionEnd;
    float transitionEndPitch;
    Vec3 transitionVelocity;
    unsigned char pad2[4];
    float transitionTime;
    bool transitionActive;
};
// CGolfDoc stores its affine camera matrix in a header gap at 0x324.
struct GolfDocCameraFields { char beforeMatrix[0x324]; float right[3], up[3], forward[3], position[3]; };
struct CameraInstanceSettings { char beforeGroundViewFlags[0x4C]; unsigned groundViewFlags; };
struct SharedPlayerCameraFields { char beforeFlags[0x38]; unsigned flags; char rest[0x198]; };
typedef char CameraTransitionFields_active_offset[(IFF_OFFSET(CameraTransitionFields, transitionActive) == 0xC4) ? 1 : -1];
typedef char GolfDocCameraFields_forward_offset[(IFF_OFFSET(GolfDocCameraFields, forward) == 0x33C) ? 1 : -1];
typedef char GolfDocCameraFields_position_offset[(IFF_OFFSET(GolfDocCameraFields, position) == 0x348) ? 1 : -1];
typedef char SharedPlayerCameraFields_stride[(sizeof(SharedPlayerCameraFields) == 0x1D4) ? 1 : -1];
extern CSharedDoc* g_sharedDoc;
// Borrow the current golf document; no reference is retained or cached.
static inline CGolfDoc* CameraGolfDocument() { return (CGolfDoc*)g_sharedDoc->golfDoc; }
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
extern int CQuadTree_GetMeshIndex(void*, void*, int);
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
extern void CMouseCursor_GetInputPosition(void*, void*);
extern int Input_IsActionButtonDown(void*, int, int);
extern int InputManager_RaiseCursorAppearance(void*, int, int);
extern int Input_IsActionPressed(void*, int, int);
extern void InputManager_GetPointerPosition(void*, float*, int);
extern void Camera_ScreenToRay(void*, void*, float, float, float);
extern void CQuadTree_GetRayIntersection(void*, void*, Vec3*, Vec3*);
// The original passes these by value, which CodeWarrior turns into a pointer to a copy.
extern unsigned char GetGroundType__9CQuadTreeF7WVector(void*, Vec3);
extern int CQuadTree_OverlappedWithObject(void*, Vec3, float);
#define ARGV(x) (x)
extern int CourseData_IsPointOutsidePlayableRegion(void*, void*);
extern float Magnitude__7WVectorCFv(void*);
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern void NoticeQueue_ShowById(void*, int, int);

// Find the forward ground target and either snap the camera or prepare a timed pitch transition.
// flag!=0 snaps immediately; flag==0 prepares interpolation. Positions/offsets are world units;
// the local (0,30,-60) offset is rotated by the document camera basis into world space.
void CCamera_SetupForwardGroundView(CameraTransitionFields* self, int flag) {
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
    volatile char pad[0x20];
    // Phase 1: project the document's forward direction from the ball onto course ground.
    self->transitionActive = (flag == 0);
    float a = GolfParameter_ResolveFloat(InstanceTable_Get(), 0) * kFloat_3_2_CameraPosition;
    __ml__FfRC7WVector(&v, ((GolfDocCameraFields*)CameraGolfDocument())->forward, a);
    void* x = GolfBall__Fv();
    __pl__FRC7WVectorRC7WVector(&q, x, &v);
    *(Vec3i*)&q2 = *(Vec3i*)&q;
    void* m = CQuadTree_GetInstance();
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&matrix, m, &q2, 1, (int)&index, 1);
    __as__7WVectorFRC7WVector(&self->transitionEnd, &matrix);
    if (index == -1) {
        if (flag) {
            CameraGolfDocument()->cameraMode = kCameraFallbackMode;
            IActor_DispatchMessage(self, self, (const char*)cameraActorStrings + 7, 7, 8, 0, 0, 0);
        }
        return;
    }
    // Phase 2: form the elevated/retreated world-space camera endpoint and its pitch in radians.
    void* view = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    pos.x = kFloat_0_camera;
    pos.y = kFloat_30_CameraPosition;
    pos.z = kFloat_neg60_CameraPosition;
    Matrix_TransformDirection(&out, &pos, (char*)view + IFF_OFFSET(GolfDocCameraFields, right));
    __apl__7WVectorFRC7WVector(&self->transitionEnd, &out);
    self->transitionEndPitch = kFloat_neg0_2617994_CameraTransition;
    if (flag) {
        __as__7WVectorFRC7WVector(&self->position, &self->transitionEnd);
        __pl__FRC7WVectorRC7WVector(&t3, &self->transitionEnd, ((GolfDocCameraFields*)CameraGolfDocument())->forward);
        __as__7WVectorFRC7WVector(&self->target, &t3);
        float t2 = (float)tan((double)self->transitionEndPitch);
        self->target.y = self->target.y + t2;
        Matrix_BuildCameraTransform((Vec3*)t4, &self->position, &self->target);
        Matrix_CopyAffine(s_cameraTransform, (Vec3*)t4);
    } else {
        __as__7WVectorFRC7WVector(&self->transitionStart, AsCameraTransform800D(s_cameraTransform)->position);
        self->transitionStartPitch = (float)atan((double)(*(float*)(&AsCameraTransform800D(s_cameraTransform)->basis[7]) / Math_SquareRootApprox(kFloat_1_BallPhysics - *(float*)(&AsCameraTransform800D(s_cameraTransform)->basis[7]) * *(float*)(&AsCameraTransform800D(s_cameraTransform)->basis[7]))));
        __mi__FRC7WVectorRC7WVector(&t1, &self->transitionEnd, &self->transitionStart);
        Vec3_DivideScalar(&t2b, kFloat_1_2_CameraTransition, &t1);
        __as__7WVectorFRC7WVector(&self->transitionVelocity, &t2b);
        self->transitionTime = kFloat_0_camera;
    }
}

// Coordinate golf-state camera changes and interpolate the active ground-view transition.
// dt uses the engine frame-time unit, as do transitionTime and the existing 1.2 duration constant.
void CCamera_UpdatePitchTransition(void* self, float dt) {
    unsigned char* s = (unsigned char*)self;
    float time = dt;
    float z;
    void* view = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (AsCGolfDoc(view)->tutorialStage >= 1) { if (CItemWindow::IsClosed()) {
    float angle;
    if ((*(unsigned int*)&AsCCamera(s)->remainingRotationAngle) & 0x80000000u) angle = -AsCCamera(s)->remainingRotationAngle;
    else angle = AsCCamera(s)->remainingRotationAngle;
            bool capturedValue = angle < kCameraRotationEpsilon;
            if ((capturedValue)) {
    view = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);

    // Phase 1: dispatch state changes and choose smooth versus immediate ground-view setup.
    if ((AsCGolfDoc(view)->cameraMode) == kCameraForwardGroundMode) {
        if ((*(unsigned int*)&AsCGolfDoc(view)->gameStateFlags) != kCameraNeedsGroundTransition) {
            float a = GolfParameter_ResolveFloat(InstanceTable_Get(), 0) * kFloat_3_2_CameraPosition;
            Vec3 q; Vec3 v;
                        __ml__FfRC7WVector(&v, (char*)view + IFF_OFFSET(GolfDocCameraFields, forward), a);
            __pl__FRC7WVectorRC7WVector(&q, GolfBall__Fv(), &v);
            void* mgr = CQuadTree_GetInstance();

            if ((CQuadTree_GetMeshIndex(mgr, &q, 0)) == -1) {
                CameraGolfDocument()->cameraMode = kCameraFallbackMode;
                IActor_DispatchMessage(self, self, (const char*)cameraActorStrings + 7, 7, 8, 0, 0, 0);
                return;
            }
        }
        view = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);

        switch ((AsCGolfDoc(view)->gameStateFlags)) {
        case kCameraNeedsGroundTransition:
            IActor_DispatchMessage(self, self, (const char*)cameraActorStrings + 7, 7, 8, 0, 0, 0);
            break;
        case kCameraForward:
        case kCameraAtReference:
            IActor_DispatchMessage(self, self, (const char*)cameraActorStrings + 7, 7, kCameraNeedsGroundTransition, 0, 0, 0);
            CCamera_SetupForwardGroundView((CameraTransitionFields*)self, 0);
            break;
        case kCameraFreeHeight:
            AsCGolfDoc(view)->cameraMode = kCameraMode4;
            if (AsCGolfDoc(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)->dropRequest) break; // original +0x3C8
        case kCameraNamedView:
            IActor_DispatchMessage(self, self, (const char*)cameraActorStrings + 7, 7, kCameraNeedsGroundTransition, 0, 0, 0);
            CCamera_SetupForwardGroundView((CameraTransitionFields*)self, 1);
            break;
        }
    }
    view = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if ((*(unsigned int*)&AsCGolfDoc(view)->gameStateFlags) != kCameraNeedsGroundTransition) return;
    view = InstanceTable_Get();
    if (*(unsigned int*)((char*)view + IFF_OFFSET(CameraInstanceSettings, groundViewFlags)) & 1) CCamera_SetupForwardGroundView((CameraTransitionFields*)self, 1);
    if (AsCCamera(s)->transitionActive == 0) return;
    // Phase 2: advance the active transition and clamp its elapsed time to the duration.
    float maxv = kFloat_1_2_CameraTransition;
    float t = AsCCamera(s)->transitionTime;
    t += dt;
                AsCCamera(s)->transitionTime = t;
    if (t > maxv) {
                    AsCCamera(s)->transitionActive = 0;
                    AsCCamera(s)->transitionTime = maxv;
    }

    Vec3 tmp8;
    AsCCamera(s)->transitionPitch = (AsCCamera(s)->transitionTime * (AsCCamera(s)->transitionEndPitch - AsCCamera(s)->transitionStartPitch) + AsCCamera(s)->transitionStartPitch); Vec3 a;
    __ml__FfRC7WVector(&a, s + IFF_OFFSET(CCamera, transitionVelocity), AsCCamera(s)->transitionTime);
    __pl__FRC7WVectorRC7WVector(&tmp8, s + IFF_OFFSET(CCamera, transitionStart), &a);
    __as__7WVectorFRC7WVector(s + IFF_OFFSET(CCamera, position), &tmp8);
    view = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    // Phase 3: enforce ground clearance, reconstruct the look target and publish the camera matrix.
    Vec3 tmp20;
    z = AsCCamera(s)->position[1];
      Vec3 xyz = (*(Vec3*)AsCCamera(s)->position);
      GetGroundPoint__9CQuadTreeF7WVectorbPib(&tmp20, CQuadTree_GetInstance(), &xyz, 0, 0, 1);
    float f = tmp20.y + AsCGolfDoc(view)->cameraGroundClearance;
      z = f < z ? z : f;
    Vec3 tmp8b;
    AsCCamera(s)->position[1] = z;
    __pl__FRC7WVectorRC7WVector(&tmp8b, s + IFF_OFFSET(CCamera, position), (char*)AsCSharedDoc(g_sharedDoc)->golfDoc + IFF_OFFSET(GolfDocCameraFields, forward));
    __as__7WVectorFRC7WVector(s + IFF_OFFSET(CCamera, targetX), &tmp8b);
    AsCCamera(s)->targetY += (float)tan(AsCCamera(s)->transitionPitch);

      unsigned char tmp60[0x38]; Matrix_BuildCameraTransform((Vec3*)tmp60, s + IFF_OFFSET(CCamera, position), s + IFF_OFFSET(CCamera, targetX));
    Matrix_CopyAffine(s_cameraTransform, (Vec3*)tmp60);
    }
    } }
}

// Translate permitted controller actions into camera rotation, height, movement, and drop targeting.
// Actor queries borrow stack outputs synchronously: Screen returns its HUD phase;
// GolfRule reports a live transition dialog. No input runs while that dialog exists.
// dt uses the camera update clock; height ratios are dimensionless, headings radians.
void CCamera_UpdateInputDirections(void* self, float dt) {
    unsigned char* s = (unsigned char*)self;
    int rotateNegativeHeld, rotatePositiveHeld;
    void* inputManager;
    int action19Released;
    int movementModeEnabled;
    int hudPhase, transitionDialogActive;
    double pointerPositionStorage; // Two float pointer coordinates, kept in the original aligned slot.
    unsigned char actorDirectionBlock;
    int cameraOrBallControlActive;
    int dropModeActive;

    // Phase 1: clear this frame's directions, then gate input on transition/item-window state.
    AsCCamera(s)->rotationDirection = 0;
    AsCCamera(s)->heightDirection = 0;
    AsCCamera(s)->movementDirection = 0;
    // Original 0x800D3230: shared exit after resetting camera input directions.
    do {
        if ((*(unsigned int*)&CameraGolfDocument()->gameStateFlags) & kCameraNeedsGroundTransition) {
            if (AsCCamera(s)->transitionActive != 0) break;
        }
        if (CItemWindow::IsClosed() == 0) break;
        if (((int&)Sing_805DC5C8()) == 0) {
            if (s_inputManagerInitialized == 0) {
                InputManager_Init(&s_gameInputManager);
                __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_gameInputManagerDtorRegistration_GolfInput);
                s_inputManagerInitialized = 1;
            }
            ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
        }
        inputManager = (void*)((int&)Sing_805DC5C8());
        CMouseCursor_GetInputPosition(g_mouseCursor, (float*)&pointerPositionStorage);
        AsCCamera(s)->previousInput = (*(int*)&AsCCamera(s)->currentInput);
        (*(int*)&AsCCamera(s)->currentInput) = 0;
        action19Released = (Input_IsActionButtonDown(inputManager, kCameraAction19, -1) == 0);
        rotateNegativeHeld = Input_IsActionButtonDown(inputManager, kRotateNegativeAction, -1);
        rotatePositiveHeld = Input_IsActionButtonDown(inputManager, kRotatePositiveAction, -1);
        movementModeEnabled = 0;
        if (AsCCamera(s)->mode != 0 && AsCCamera(s)->mode != kCameraMode4) movementModeEnabled = 1;
        IActor_DispatchMessage(self, self, cameraActorStrings, kQueryHudPhase, (int)&hudPhase, 0, 0, 0);
        IActor_DispatchMessage(self, self, cameraActorStrings + 7, kQueryRuleTransitionDialog, (int)&transitionDialogActive, 0, 0, 0);
        if (transitionDialogActive != 0) break;
        // Phase 2: map held actions to movement/height/rotation according to camera state.
        if (hudPhase == kHudPhaseAim) {
            if (action19Released) {
                if (CameraGolfDocument()->cameraMode != 0) {
                    int n = InputManager_GetSelectedPlayer(inputManager);
                    InputManager_RaiseCursorAppearance(inputManager, 4, n);
                    (*(int*)&AsCCamera(s)->currentInput) = 1;
                }
            }
            if (movementModeEnabled && (*(int*)&AsCCamera(s)->zoomInputBlocked) == 0) {
                if (Input_IsActionButtonDown(inputManager, kAdvanceOrLowerAction, -1) != 0) {
                    (*(int*)&AsCCamera(s)->currentInput) = 4;
                    if ((*(unsigned int*)&CameraGolfDocument()->gameStateFlags) == kCameraFreeHeight) {
                        if (AsCCamera(s)->heightRatio > kFloat_0_camera) {
                            float mv = kFloat_neg1_Camera * dt;
                            AsCCamera(s)->heightDirection = -1;
                            mv *= kFloat_1_BallPhysics;
                            CCamera_UpdateVerticalMovement(self, mv);
                        }
                    } else {
                        AsCCamera(s)->movementDirection = 1;
                        CCamera_UpdateForwardMovement(self, dt * kFloat_1_BallPhysics);
                    }
                } else if (Input_IsActionButtonDown(inputManager, kRetreatOrRaiseAction, -1) != 0) {
                    (*(int*)&AsCCamera(s)->currentInput) = 5;
                    if ((*(unsigned int*)&CameraGolfDocument()->gameStateFlags) == kCameraFreeHeight) {
                        if (AsCCamera(s)->heightRatio < kFloat_1_BallPhysics) {
                            AsCCamera(s)->heightDirection = 1;
                            CCamera_UpdateVerticalMovement(self, dt * kFloat_1_BallPhysics);
                        }
                    } else {
                        float mv2 = kFloat_neg1_Camera * dt;
                        AsCCamera(s)->movementDirection = -1;
                        mv2 *= kFloat_1_BallPhysics;
                        CCamera_UpdateForwardMovement(self, mv2);
                    }
                }
            }
            if ((*(int*)&AsCCamera(s)->directionalInputEnabled) != 0 || CameraGolfDocument()->tutorialStage == 6) {
                if (action19Released || rotateNegativeHeld || rotatePositiveHeld) {
                    if (rotatePositiveHeld) {
                        AsCCamera(s)->rotationDirection = 1;
                        (*(int*)&AsCCamera(s)->currentInput) = 7;
                        CCamera_UpdateRotation(self, kCameraRotationRate * -dt);
                    } else if (rotateNegativeHeld) {
                        AsCCamera(s)->rotationDirection = -1;
                        (*(int*)&AsCCamera(s)->currentInput) = 8;
                        CCamera_UpdateRotation(self, kCameraRotationRate * dt);
                    }
                }
                if (action19Released && movementModeEnabled) {
                    if (action19Released && Input_IsActionButtonDown(inputManager, kRaiseOrAdvanceAction, -1) != 0) {
                        (*(int*)&AsCCamera(s)->currentInput) = 2;
                        if ((*(unsigned int*)&CameraGolfDocument()->gameStateFlags) == kCameraFreeHeight) {
                            AsCCamera(s)->movementDirection = 1;
                            CCamera_UpdateForwardMovement(self, dt);
                        } else if (AsCCamera(s)->heightRatio < kFloat_1_BallPhysics) {
                            AsCCamera(s)->heightDirection = 1;
                            CCamera_UpdateVerticalMovement(self, dt);
                        }
                    }
                    if (action19Released && Input_IsActionButtonDown(inputManager, kLowerOrRetreatAction, -1) != 0) {
                        (*(int*)&AsCCamera(s)->currentInput) = 3;
                        if ((*(unsigned int*)&CameraGolfDocument()->gameStateFlags) == kCameraFreeHeight) {
                            AsCCamera(s)->movementDirection = -1;
                            CCamera_UpdateForwardMovement(self, kFloat_neg1_Camera * dt);
                        } else if (AsCCamera(s)->heightRatio > kFloat_0_camera) {
                            AsCCamera(s)->heightDirection = -1;
                            CCamera_UpdateVerticalMovement(self, kFloat_neg1_Camera * dt);
                        }
                    }
                }
            }
        }
        // Phase 3: drop targeting needs free-height state, idle movement and all actor permissions.
        IActor_DispatchMessage(self, self, (const char*)cameraActorStrings + 0x15, 0x0D, (int)&actorDirectionBlock, 0, 0, 0);
        int canChooseDropTarget = (int)(((*(unsigned int*)&CameraGolfDocument()->gameStateFlags) >> 9) & 1);
        canChooseDropTarget &= (AsCCamera(s)->heightDirection == 0);
        canChooseDropTarget &= (AsCCamera(s)->movementDirection == 0);
        canChooseDropTarget &= (hudPhase == kHudPhaseAim);
        IActor_DispatchMessage(self, self, cameraActorStrings, kQueryCameraOrBallControl, (int)&cameraOrBallControlActive, 0, 0, 0);
        dropModeActive = 0;
        IActor_DispatchMessage(self, self, cameraActorStrings, kQueryDropMode, (int)&dropModeActive, 0, 0, 0);
        canChooseDropTarget &= (cameraOrBallControlActive == 0);
        canChooseDropTarget &= (dropModeActive != 0);
        canChooseDropTarget &= (Input_IsActionPressed(inputManager, kChooseDropTargetAction, -1) != 0);
        canChooseDropTarget &= (CameraGolfDocument()->tutorialStage >= 1);
        canChooseDropTarget &= (actorDirectionBlock == 0);
        if (canChooseDropTarget) CCamera_UpdatePointerGroundTarget(self, dt);
    } while (0);
    return;
}

// Raycast the pointer onto the course and accept a legal drop target, otherwise play the rejection cue.
// Borrowed world state only; dt is unused. Ground code 0 bypasses the region/distance test;
// code 1 must be in the playable region and farther from the hole than ballDistanceXZ-25.6.
// Other ground codes or object-bound hits reject the drop. Distances are engine world units.
// Ball+0xC0 is added to hit Y; its physical meaning is unresolved (do not assume radius).
void CCamera_UpdatePointerGroundTarget(void* self, float dt) {
    Vec3 targetFromHoleXZ, ballFromHoleXZ;
    struct { float x, y, z, w; } v;
    Vec3 ray, dropTargetWorld, rayDir, groundIntersection, originCopy, rayCopy, ballFromHole, holePosition, targetFromHole, holePositionAgain;
    int initialized = ((int&)Sing_805DC5C8());
    *(Arr3*)&v = *(Arr3*)((InputPointerState*)g_inputPointer)->position; // 0x800D380C: pointer position +0x54
    if (!initialized) {
        if (!s_inputManagerInitialized) {

            InputManager_Init((void*)s_gameInputManager);
            __register_global_object((void*)s_gameInputManager, (void*)InputManager_Dtor, (void*)s_gameInputManagerDtorRegistration_GolfInput);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)s_gameInputManager;
    }
    void* cursor = (void*)((int&)Sing_805DC5C8());
    float xy[2];
    int n = InputManager_GetSelectedPlayer(cursor);
    InputManager_GetPointerPosition(cursor, xy, n);
    v.x = xy[0];
    v.y = xy[1];
    // Phase 1: turn screen coordinates into a world ray starting at the current camera position.
    Camera_ScreenToRay(&rayDir, g_view, xy[0], xy[1], kFloat_neg1_Camera);
    __as__7WVectorFRC7WVector(&ray, &rayDir);
    *(Arr3*)&rayCopy = *(Arr3*)&ray;
    *(Arr3*)&originCopy = *(Arr3*)(AsCameraTransform800D(s_cameraTransform)->position);
    CQuadTree_GetRayIntersection(&groundIntersection, CQuadTree_GetInstance(), &originCopy, &rayCopy);
    __as__7WVectorFRC7WVector(&dropTargetWorld, &groundIntersection);
    float* ballState = (float*)GolfBall__Fv();
    float ballTargetHeightOffset = ballState[0xC0/4];
    dropTargetWorld.y = dropTargetWorld.y + ballTargetHeightOffset;
    // Phase 2: compare horizontal hole-relative distances, then dispatch acceptance or rejection.
    if (CameraGolfDocument()->dropRequest) {
        *(Arr3*)&holePosition = *(Arr3*)((char*)CameraGolfDocument()->holeRecords+CameraGolfDocument()->currentHoleIndex*32+0xC);
        __mi__FRC7WVectorRC7WVector(&ballFromHole, GolfBall__Fv(), &holePosition);
        __as__7WVectorFRC7WVector(&ballFromHoleXZ, &ballFromHole);
        ballFromHoleXZ.y = kFloat_0_camera;
        *(Arr3*)&holePositionAgain = *(Arr3*)((char*)CameraGolfDocument()->holeRecords+CameraGolfDocument()->currentHoleIndex*32+0xC);
        __mi__FRC7WVectorRC7WVector(&targetFromHole, &dropTargetWorld, &holePositionAgain);
        __as__7WVectorFRC7WVector(&targetFromHoleXZ, &targetFromHole);
        targetFromHoleXZ.y = kFloat_0_camera;
        unsigned char groundType = GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), ARGV(dropTargetWorld));
        // Original 0x800D380C: failed drop checks share the notice path.
        do {
            if (!CQuadTree_OverlappedWithObject(CQuadTree_GetInstance(), ARGV(dropTargetWorld), kFloat_0_camera)) {
                if (groundType) {
                    if (groundType != 1) break;
                    void* mgr = CameraGolfDocument()->courseData;
                    if (CourseData_IsPointOutsidePlayableRegion(mgr, &dropTargetWorld) != 0) break;
                    float d1 = Magnitude__7WVectorCFv(&ballFromHoleXZ) - kFloat_25_6_CameraGroundTarget;
                    float d2 = Magnitude__7WVectorCFv(&targetFromHoleXZ);
                    if (!(d2 > d1)) break;
                }
                CameraGolfDocument()->dropRequest = 0;
                IActor_DispatchMessage(self, self, (const char*)cameraActorStrings + 7, 0x11, (int)&dropTargetWorld, 0, 0, 0);
                return;
            }
        } while (0);
        Sound_PlayByName(g_soundManager, (const char*)cameraActorStrings + 0x1B, 0, 0, 0, 0, kFloat_0_5_Camera_805DDE20, kFloat_200_CameraActor, kFloat_1_BallPhysics);
        if (g_helpMsg) NoticeQueue_ShowById(g_helpMsg, 0, 0);
    }
}

enum CameraControlMessage { kCameraSetDocumentView = 1, kCameraSetHeightRange = 2,
    kCameraClearLatch = 3, kCameraEnable = 4, kCameraDisable = 5, kCameraCenterHeight = 6,
    kCameraSetScalar = 7, kCameraGetInput = 8, kCameraBlockZoom = 9, kCameraSetTimedControl = 10,
    kCameraEnableDirections = 11, kCameraSnapGroundView = 12, kCameraSetHeading = 0x2717 };
union CameraMessageArgument { unsigned word; int signedWord; void* pointer; };
struct CameraControlEvent {
    unsigned header;
    int type;
    CameraMessageArgument first, second;
    char reserved[0xC];
    unsigned deliveryTime;
};
typedef char CameraControlEvent_argument_offset[(IFF_OFFSET(CameraControlEvent, first) == 8) ? 1 : -1];
typedef char CameraControlEvent_time_offset[(IFF_OFFSET(CameraControlEvent, deliveryTime) == 0x1C) ? 1 : -1];

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

#pragma opt_lifetimes off
// Apply camera control messages whose timestamp is due; defer future messages through virtual slot 10.
void CCamera_HandleMsg(void* self, const CameraControlEvent* event) {
    unsigned int deliveryTime = event->deliveryTime;
    if (deliveryTime > g_currentTimeMilliseconds) {
        ((CameraEventReceiver*)self)->slot10();
        return;
    }
    switch (event->type) {
    case kCameraCenterHeight:
        AsCCamera(self)->heightOffsetRatio = kFloat_0_camera;
        AsCCamera(self)->heightRatio = kFloat_0_5_Camera_805DDE20;
        break;
    case kCameraSetHeading: {
        AsCCamera(self)->headingUpdateTimer = kFloat_0_camera;
        void* view = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        AsCGolfDoc(view)->headingRadians = *(float*)event->first.pointer;
        break;
    }
    case kCameraSetDocumentView: {
        (*(WVector*)AsCCamera(self)->position) = *(WVector*)(((GolfDocCameraFields*)CameraGolfDocument())->position);

        // World-space look point = camera position + forward; pitch adds tan(radians) on Y.
        WVector v = *(WVector*)(((GolfDocCameraFields*)CameraGolfDocument())->position) + *(WVector*)(((GolfDocCameraFields*)CameraGolfDocument())->forward);
        (*(WVector*)&AsCCamera(self)->targetX) = v;
        AsCCamera(self)->targetY += (float)tan((double)AsCGolfDoc(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)->cameraPitchRadians); // original +0x354
        AsCCamera(self)->heightOffsetRatio = kFloat_0_camera;
        AsCCamera(self)->groundHeightRatio = (((GolfDocCameraFields*)CameraGolfDocument())->position[1] - AsCCamera(self)->minHeight) / (AsCCamera(self)->maxHeight - AsCCamera(self)->minHeight);
        AsCCamera(self)->heightRatio = AsCCamera(self)->groundHeightRatio;
        AsCCamera(self)->referenceHeightRatio = (((GolfDocCameraFields*)CameraGolfDocument())->position[1] - AsCCamera(self)->minHeight) / (AsCCamera(self)->maxHeight - AsCCamera(self)->minHeight);
        break;
    }
    case kCameraSetHeightRange: {
        float v = AsCQuadTree(CQuadTree_GetInstance())->minimumGroundHeight;
        AsCCamera(self)->minHeight = v;
        AsCCamera(self)->maxHeight = kFloat_320_Camera + v;
        break;
    }
    case kCameraClearLatch: AsCCamera(self)->useLinearHeightInterpolation = 0; break;
    case kCameraEnable: AsCCamera(self)->processingEnabled = 1; break;
    case kCameraDisable: AsCCamera(self)->processingEnabled = 0; break;
    case kCameraSetScalar: AsCCamera(self)->unk_8C = *(float*)event->first.pointer; break; // scalar setter; no consumer is established here
    case kCameraGetInput: {
        void* p = event->first.pointer;
        *(unsigned int*)p = AsCCamera(self)->currentInput;
        break;
    }
    case kCameraBlockZoom: AsCCamera(self)->zoomInputBlocked = event->first.word; break;
    case kCameraSetTimedControl: {
        // Store the control mode and convert its duration from tenths to whole time units.
        AsCCamera(self)->timedControlMode = event->first.word;
        AsCCamera(self)->timedControlDuration = (float)event->second.signedWord / kFloat_10_Camera;
        break;
    }
    case kCameraEnableDirections: AsCCamera(self)->directionalInputEnabled = event->first.word; break;
    case kCameraSnapGroundView: {
        void* view = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        if ((*(unsigned int*)&AsCGolfDoc(view)->gameStateFlags) == kCameraNeedsGroundTransition) CCamera_SetupForwardGroundView((CameraTransitionFields*)self, 1);
        break;
    }
    default: break;
    }
}

#pragma opt_lifetimes reset

// Read the cursor hit-region ID through the existing integer ABI.
inline int stateValue(void* obj) { return *(int*)((char*)obj + IFF_OFFSET(CMouseCursor, hitRegion)); }
// Return the next heading timer value without storing it.
inline float timerValue(void* obj, float dt) {
    return AsCCamera(obj)->headingUpdateTimer + dt;
}
// Refresh camera mode, pointer controls, transition motion, and held-input acceleration ramps.
void CCamera_OnProcess(void* self, float dt) {
    // Phase 1: gate on player/processing state and synchronize camera mode and pointer UI.
    unsigned char* s = (unsigned char*)self;
    if (AsCCamera(s)->processingEnabled == 0) return;
    void* world = g_sharedDoc;
    void* view = (*(void**)&AsCSharedDoc(world)->golfDoc);

    void* player = (char*)world + (*((unsigned char*)&AsCGolfDoc(view)->currentPlayer)) * 0x1D4;
    // Original CCamera_OnProcess: skip this pass for an inactive player.
    do {
        switch (*(unsigned int*)((char*)player + IFF_OFFSET(SharedPlayerCameraFields, flags)) & 1) {
        case 0: break;
        default: continue;
        }
        {
        int mode;
        int old = AsCCamera(s)->mode;
        mode = AsCGolfDoc(view)->cameraMode;
        AsCCamera(s)->mode = mode;
        if (mode != old) AsCCamera(s)->modeChanged = 1;
        if (Pointer_GetButtonState(g_inputPointer, 2) == 1) { if (CItemWindow::IsClosed()) {
            int toggle = !AsCCamera(s)->radialMenuEnabled;
            AsCCamera(s)->radialMenuEnabled = toggle;
            CMouseCursor_SetRadialMenuEnabled(g_mouseCursor, toggle);
            if (AsCCamera(s)->radialMenuEnabled) IActor_DispatchMessage(self, self, cameraActorStrings, 0x12, 0, 0, 0, 0);
        } }
        int state;
        void* stateObj = g_mouseCursor;
        state = stateValue(stateObj);
        if (state == 0x16 && state == *(int*)((char*)stateObj + IFF_OFFSET(CMouseCursor, buttonDownRegions)) && Pointer_GetButtonState(g_inputPointer, 0) == 2)
            IActor_DispatchMessage(self, self, cameraActorStrings + 7, 7, 8, 0, 0, 0);
        if (GolfTask_NoOpStatus()) {
            float limit = kCameraHeadingUpdatePeriod;
            float t = AsCCamera(s)->headingUpdateTimer;
            t += dt;
            if ((AsCCamera(s)->headingUpdateTimer = t) > limit) {
                AsCCamera(s)->headingUpdateTimer = t - limit;
                if (AsCCamera(s)->headingChanged) AsCCamera(s)->headingChanged = 0;
            }
        }
        // Phase 2: update transition/motion, then handle input or the outstanding rotation sweep.
        CCamera_UpdatePitchTransition(self, dt);
        CCamera_HandleMovementInput(self, dt);
        view = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        if ((*(unsigned int*)&AsCGolfDoc(view)->gameStateFlags) & kCameraNamedView) return;
        float value = AsCCamera(s)->inputDelay;
        if (value > kFloat_0_camera) { AsCCamera(s)->inputDelay = value - dt; return; }
        float x;
        if ((*(unsigned int*)&AsCCamera(s)->remainingRotationAngle) & 0x80000000) x = -AsCCamera(s)->remainingRotationAngle;
        else x = AsCCamera(s)->remainingRotationAngle;
            __typeof__(x) _p479 = x;
            __typeof__(_p479 < kCameraRotationEpsilon) _p284 = _p479 < kCameraRotationEpsilon;
            __typeof__(_p284) _p288 = _p479 < kCameraRotationEpsilon;
            if ((_p288)) CCamera_UpdateInputDirections(self, dt);
        else {

            float yy;
            CCamera_UpdateRotation(self, (kFloat_4_CameraRotation * (kCameraRotationRate * dt)) * (AsCCamera(s)->rotationDirection == 1 ? kFloat_1_BallPhysics : kFloat_neg1_Camera));
            if ((*(unsigned int*)&AsCCamera(s)->totalRotationAngle) & 0x80000000) yy = -AsCCamera(s)->totalRotationAngle;
            else yy = AsCCamera(s)->totalRotationAngle;
                __typeof__(yy) _p738 = yy;
                __typeof__(_p738 < kCameraRotationEpsilon) _p283 = _p738 < kCameraRotationEpsilon;
                __typeof__(yy) _p712 = yy;
                __typeof__(_p283) _p331 = _p712 < kCameraRotationEpsilon;
                bool capturedValue2 = yy < kCameraRotationEpsilon;
                __typeof__(yy < kCameraRotationEpsilon) _p978 = yy < kCameraRotationEpsilon;
                __typeof__(_p978) _p781 = yy < kCameraRotationEpsilon;
                __typeof__(yy) _p381 = yy;
                __typeof__(_p781) _p144 = _p381 < kCameraRotationEpsilon;
                float ratio = (_p144) ? kFloat_1_BallPhysics : kFloat_1_BallPhysics - (AsCCamera(s)->remainingRotationAngle / AsCCamera(s)->totalRotationAngle);
            IActor_DispatchMessage(self, self, cameraActorStrings + 0x10, 7, (int)&ratio, 0, 0, 0);
        }
        // Phase 3: ramp each continuously held direction independently, clamping to [0,1].
        int a = AsCCamera(s)->rotationDirection;
        if (s_previousCameraRotationDirection == a && a != 0) {
            float v = AsCCamera(s)->rotationInputRamp + dt;
            AsCCamera(s)->rotationInputRamp = v < kFloat_0_camera ? kFloat_0_camera : v > kFloat_1_BallPhysics ? kFloat_1_BallPhysics : v;
        } else AsCCamera(s)->rotationInputRamp = kFloat_0_camera;
        int b = AsCCamera(s)->heightDirection;
        s_previousCameraRotationDirection = AsCCamera(s)->rotationDirection;
        if (s_previousCameraHeightDirection == b && 0 != b) {
            float v = AsCCamera(s)->heightInputRamp + dt;
                AsCCamera(s)->heightInputRamp = v < kFloat_0_camera ? kFloat_0_camera : v > kFloat_1_BallPhysics ? kFloat_1_BallPhysics : v;
        } else AsCCamera(s)->heightInputRamp = kFloat_0_camera;
        int c = AsCCamera(s)->movementDirection;
        s_previousCameraHeightDirection = AsCCamera(s)->heightDirection;
        if (s_previousCameraMovementDirection == c && c != 0) {
            float v = AsCCamera(s)->movementInputRamp + dt;
                AsCCamera(s)->movementInputRamp = v < kFloat_0_camera ? kFloat_0_camera : v > kFloat_1_BallPhysics ? kFloat_1_BallPhysics : v;
        } else AsCCamera(s)->movementInputRamp = kFloat_0_camera;
        s_previousCameraMovementDirection = AsCCamera(s)->movementDirection;
        }
    } while (0);
    return;
}

} // extern "C"
