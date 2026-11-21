#include <decomp/static_init.h>
#include <decomp/record_sizes.h>
#include <decomp/offset_of.h>
#include "classes/CameraTransform800D.h"
#include "game/golfball.h"
#include "classes/CMouseCursor.h"
#define AsCMouseCursor(p) ((CMouseCursor*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "game/itemwindow.h"
#include "game/types.h"
#include "game/wvector.h"
#include "classes/CCamera.h"

#define AsCCamera(p) ((CCamera*)(p))
// Translation unit 0x800D3DB4-0x800D5208 (8 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// Camera-only fields currently covered by padding in CGolfDoc's shared layout.
struct GolfDocCameraView {
    char pad_000[0x304];
    float orbitDistance;
    float orbitHeight;
    float groundClearance;
    float orbitCenter[3];
    float headingRadians;
    float unresolvedHeadingValue;
    CameraTransform800D headingTransform;
};
struct CameraNameRecord {
    char name[0x20];
    CameraTransform800D transform;
    float fieldOfViewDegrees;
};
struct CameraInstanceTableView { char pad_000[0x44]; unsigned int count; };
struct CameraActorStrings {
    char screen[7], golfRule[9], spot[5], score[6], unplayableSound[16];
    char padKeyFormat[7], numberKeyFormat[7], cameraNameFormat[9];
    char increaseHeading[6], leftControl[9], rightControl[9], leftShift[7], rightShift[7];
    char decreaseHeading[5], player[7], grid[5], caddie[7], guideLine[10], cameraType[8];
};
#define AsGolfDocCamera(p) ((GolfDocCameraView*)(p))
enum CameraState {
    kCameraAtReference = 8, kCameraForward = 0x100, kCameraFreeHeight = 0x200,
    kCameraNamedView = 0x8000, kCameraNeedsGroundTransition = 0x40000
};
enum CameraRegion {
    kRaiseTarget = 8, kLowerTarget = 9, kPanLeft = 0x17, kPanRight = 0x18,
    kRaiseTargetButton = 0x19, kLowerTargetButton = 0x1A
};
enum { kFloatSignMask = 0x80000000u, kYAxis = 1, kFirstCameraHotkey = 3, kCameraHotkeyEnd = 10 };
// Byte forms retain relocation and address scheduling where direct global members differ.
enum CameraTransformOffset {
    kCameraPositionOffset = (int)DECOMP_OFFSETOF(CameraTransform800D, position),
    kCameraHeightOffset = (int)DECOMP_OFFSETOF(CameraTransform800D, position[1]),
    kCameraPositionZOffset = (int)DECOMP_OFFSETOF(CameraTransform800D, position[2]),
    kCameraForwardOffset = (int)DECOMP_OFFSETOF(CameraTransform800D, basis[6]),
    kCameraUpOffset = (int)DECOMP_OFFSETOF(CameraTransform800D, basis[3])
};
enum CameraStringOffset {
    kCameraScreen = (int)DECOMP_OFFSETOF(CameraActorStrings, screen),
    kCameraSpot = (int)DECOMP_OFFSETOF(CameraActorStrings, spot),
    kPadKeyFormat = (int)DECOMP_OFFSETOF(CameraActorStrings, padKeyFormat),
    kNumberKeyFormat = (int)DECOMP_OFFSETOF(CameraActorStrings, numberKeyFormat),
    kCameraNameFormat = (int)DECOMP_OFFSETOF(CameraActorStrings, cameraNameFormat),
    kIncreaseHeading = (int)DECOMP_OFFSETOF(CameraActorStrings, increaseHeading),
    kLeftControl = (int)DECOMP_OFFSETOF(CameraActorStrings, leftControl),
    kRightControl = (int)DECOMP_OFFSETOF(CameraActorStrings, rightControl),
    kLeftShift = (int)DECOMP_OFFSETOF(CameraActorStrings, leftShift),
    kRightShift = (int)DECOMP_OFFSETOF(CameraActorStrings, rightShift),
    kDecreaseHeading = (int)DECOMP_OFFSETOF(CameraActorStrings, decreaseHeading),
    kCameraPlayer = (int)DECOMP_OFFSETOF(CameraActorStrings, player),
    kCameraGrid = (int)DECOMP_OFFSETOF(CameraActorStrings, grid),
    kCameraCaddie = (int)DECOMP_OFFSETOF(CameraActorStrings, caddie),
    kCameraGuideLine = (int)DECOMP_OFFSETOF(CameraActorStrings, guideLine),
    kCameraType = (int)DECOMP_OFFSETOF(CameraActorStrings, cameraType)
};
typedef char CameraNameRecord_transformOffset[((int)DECOMP_OFFSETOF(CameraNameRecord, transform) == 0x20) ? 1 : -1];
typedef char GolfDocCamera_transformOffset[((int)DECOMP_OFFSETOF(GolfDocCameraView, headingTransform) == 0x324) ? 1 : -1];

extern "C" {

extern unsigned char s_cameraTypeInfo;
extern unsigned char s_actorTypeInfo;
extern float s_cameraUnitTwoPiCache;
extern const float kFloat_6_2831855;
extern char cameraFactoryTypeName;
extern char cameraTaskFactoryName;
extern const char cameraActorStrings[];
extern const float kFloat_0_camera;
extern float kCameraRotationEpsilon;
extern const char s_cameraTransform[];
extern const float kCameraMinimumGroundClearance;
extern const float kFloat_300_Camera;
extern const float kFloat_0_2_Camera;
extern const float kFloat_400_CameraMove;
extern const float kFloat_0_01_CameraMove;
extern const float kFloat_0_1_CameraMove;
extern const float kFloat_1_BallPhysics;
extern const float kFloat_3_2_CameraPosition;
extern const float kFloat_0_25_GolfCamera;
extern const float kFloat_2598_4001_CameraMove;
extern const float kFloat_192_Camera;
extern const float kFloat_320_Camera;
extern const float kFloat_0_65_Camera;
extern const float kFloat_10_Camera;
extern const float kDegreesToRadians_CameraFov;
extern const float kFloat_5_Camera;
extern const float kFloat_neg10_Camera;
extern const float kFloat_neg1_Camera;
extern const float kCameraRotationRate;
extern const float kFloat_neg1_0471976_CameraRotation;
extern const float kFloat_neg5_CameraVerticalOffset;
extern const float kFloat_6_2831855_CameraAngleWrap;
extern const float kFloat_3_1415927_Camera;
extern const float kFloat_neg3_1415927_CameraAngleWrap;
extern CMouseCursor* g_mouseCursor;
extern void* g_inputPointer;
extern int s_previousCameraRotationDirection;
extern int s_previousCameraMovementDirection;
extern void* g_view;
void* String_FormatTemporary(const char*, ...);
int Pointer_GetRisingActionBits(void*, void*, int);
int Pointer_GetButtonState(void*, int);
int Pointer_IsActionActive(void*, const char*, int);
void* CourseData_FindNameRecord(void*, void*);
void Camera_SetFov(void*, float);
void __ami__7WVectorFRC7WVector(void*, void*);

struct AffineScratch48 { unsigned char data[0x30]; };
struct VectorScratch16 { unsigned char data[0x10]; };
struct BasisScratch36 { unsigned char data[0x24]; };
void Matrix_SetAxisRotation(void*, float, int);
void Matrix_ConcatAffine(void*, const char*, void*);
void Matrix_CopyAffine(void*, void*);
void Vec3_Zero(void*);
void Matrix_ConcatAffineAssign(void*, void*);
void CCamera_AddHeadingAngle(void*, float);
void __as__7WVectorFRC7WVector(void*, void*);
void __apl__7WVectorFRC7WVector(void*, void*);
float __ml__FRC7WVectorRC7WVector(void*, void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void Matrix_TransformPoint(void*, void*, void*);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void* CQuadTree_GetInstance();
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
void* GolfBall__Fv();
void Matrix_BuildCameraTransform(void*, void*, void*);
double atan(double);
double tan(double);
void CQuadTree_AdjustPointToRegionAndGroundClearance(void*, void*, void*);
void CCamera_SetupForwardGroundView(void*, int);
extern CSharedDoc* g_sharedDoc;
double floor(double);
void Matrix_SetAxisRotation(void*, float, int);
void __ml__FfRC7WVector(void*, void*, float);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void CQuadTree_AdjustPointToRegionAndGroundClearance(void*, void*, void*);
void IActor_DispatchMessage(void*, void*, const char*, int, void*, int, int, int);
void* InstanceTable_Get();

typedef Vec3f Vec3;
struct Word3 { unsigned x; unsigned y; unsigned z; };
// Clamp from below, retaining the original operand order for floating-point comparisons.
inline float MaxCameraValue(float v, float lo) { return lo < v ? v : lo; }
// Clamp from above with the original comparison/NaN behavior.
inline float MinCameraValue(float v, float hi) { return hi > v ? v : hi; }
// Bound a float to the supplied interval without rearranging comparisons.
inline float ClampCameraValue(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
// Bound a double to the supplied interval.
inline double ClampCameraDouble(double v, double lo, double hi) { return v < lo ? lo : (v > hi ? hi : v); }
// Select the original sign branch relative to the shared zero constant.
inline float AbsCameraValue(float v) { return v > kFloat_0_camera ? v : -v; }

// Move along the course heading, constrain height to terrain, and detect crossing the reference position.
// Positions are world-space XYZ; Y is height. headingTransform.basis[6..8] is
// the world-space forward direction. dt is the signed movement step in the
// caller's time scale; ramp values are dimensionless. 400 is travel per dt unit.
// Word3/unsigned arrays copy FLOAT BITS for vector ABI calls, not numeric casts.
// transformBytes and its typed reference aliases always address the same global
// affine transform; changing those views can change CodeWarrior scheduling.
void CCamera_UpdateForwardMovement(CCamera* self, float dt) {
    float savedDt = dt;
    float cameraMatrix[12];
    Vec3 movement;
    Vec3 groundAdjustedPosition;
    Vec3 sampledGroundPoint;
    unsigned cameraPositionWords[3];
    Vec3 heightScaledMovement;
    Vec3 adjustedFreePosition;
    Vec3 requestedPosition;
    Vec3 groundPoint;
    Word3 candidatePositionWords;
    Vec3 positionChange;
    Vec3 headingTarget;
    Vec3 oldReferenceOffset;
    Vec3 referenceOffset;
    Vec3 newReferenceOffset;
    Vec3 referenceTarget;
    char* transformBytes;
    if ((*(unsigned*)&AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags) == kCameraFreeHeight) {
        // Free-height mode: sample terrain and scale forward speed by clearance.
        transformBytes = (char*)s_cameraTransform;
        cameraPositionWords[0] = *(unsigned*)&AsCameraTransform800D(transformBytes)->position[0];
        cameraPositionWords[1] = *(unsigned*)&AsCameraTransform800D(transformBytes)->position[1];
        cameraPositionWords[2] = *(unsigned*)&AsCameraTransform800D(transformBytes)->position[2];
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&sampledGroundPoint, CQuadTree_GetInstance(), cameraPositionWords, 1, 0, 1);
        float projected = sampledGroundPoint.y;
        float factor = ClampCameraValue((*(float*)&AsCameraTransform800D(transformBytes)->position[1] - projected) * kFloat_0_01_CameraMove, kFloat_0_1_CameraMove, kFloat_1_BallPhysics);
        __ml__FfRC7WVector(&heightScaledMovement, &AsGolfDocCamera((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->headingTransform.basis[6], savedDt * (kFloat_400_CameraMove * (factor * self->movementInputRamp)));
        transformBytes = (char*)s_cameraTransform;
        __apl__7WVectorFRC7WVector(AsCameraTransform800D(transformBytes)->position, &heightScaledMovement);
        CQuadTree_AdjustPointToRegionAndGroundClearance(&adjustedFreePosition, CQuadTree_GetInstance(), AsCameraTransform800D(transformBytes)->position);
        __as__7WVectorFRC7WVector(AsCameraTransform800D(transformBytes)->position, &adjustedFreePosition);
    } else {
        // Ground/reference modes: constrain a candidate, then apply actual displacement.
        __ml__FfRC7WVector(&movement, &AsGolfDocCamera((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->headingTransform.basis[6], savedDt * (kFloat_400_CameraMove * self->movementInputRamp));
        transformBytes = (char*)s_cameraTransform;
        __pl__FRC7WVectorRC7WVector(&requestedPosition, AsCameraTransform800D(transformBytes)->position, &movement);
        CQuadTree_AdjustPointToRegionAndGroundClearance(&groundAdjustedPosition, CQuadTree_GetInstance(), &requestedPosition);
        float groundHeight = self->minHeight;
        candidatePositionWords = *(Word3*)&groundAdjustedPosition;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPoint, CQuadTree_GetInstance(), &candidatePositionWords, 1, 0, 1);
        groundHeight = groundPoint.y < groundHeight ? groundHeight : groundPoint.y;
        float ceiling = groundHeight + AsCGolfDoc(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->cameraGroundClearance;
        groundAdjustedPosition.y = ClampCameraValue(groundAdjustedPosition.y, ceiling, self->maxHeight);
        { float*& transformVectorAlias = (float*&)transformBytes;
        transformVectorAlias = (float*)s_cameraTransform;
        __mi__FRC7WVectorRC7WVector(&positionChange, &groundAdjustedPosition, AsCameraTransform800D(transformVectorAlias)->position); }
        __as__7WVectorFRC7WVector(&movement, &positionChange);
        __apl__7WVectorFRC7WVector(self->position, &movement);
        __pl__FRC7WVectorRC7WVector(&headingTarget, self->position, &AsGolfDocCamera((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->headingTransform.basis[6]);
        __as__7WVectorFRC7WVector(&self->targetX, &headingTarget);
        // Rebuild target/pitch and normalized height after the terrain constraint.
        float heightRatioDelta;
        float heightRatio = (self->position[1] - self->minHeight) / kFloat_320_Camera;
        heightRatioDelta = heightRatio - self->groundHeightRatio;
        self->heightRatio = heightRatio;
        self->heightOffsetRatio = heightRatioDelta;
        float terrainPitch = atan(kFloat_0_65_Camera * heightRatioDelta);
        float pitch = AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->cameraPitchRadians - terrainPitch;
        pitch = MinCameraValue(pitch, kFloat_0_camera);
        self->targetY = self->targetY + (float)tan(pitch);
        Matrix_BuildCameraTransform(cameraMatrix, self->position, &self->targetX);
        Matrix_CopyAffine((void*)s_cameraTransform, cameraMatrix);
        transformBytes = (char*)s_cameraTransform;
        self->useLinearHeightInterpolation = 1;
        float limit = groundHeight + AsCGolfDoc(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->cameraGroundClearance;
        self->groundHeightRatio = (limit - self->minHeight) / (self->maxHeight - self->minHeight);

        // Compare signed heading distances before/after movement. A crossing
        // within 3.2 world height units snaps back to the reference camera.
        void* golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        __mi__FRC7WVectorRC7WVector(&referenceOffset, AsCameraTransform800D(transformBytes)->position, AsGolfDocCamera(golfDoc)->headingTransform.position);
        __mi__FRC7WVectorRC7WVector(&oldReferenceOffset, &referenceOffset, &movement);
        float previousHeadingDistance = __ml__FRC7WVectorRC7WVector(&AsGolfDocCamera(golfDoc)->headingTransform.basis[6], &oldReferenceOffset);
        golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        __mi__FRC7WVectorRC7WVector(&newReferenceOffset, AsCameraTransform800D(transformBytes)->position, AsGolfDocCamera(golfDoc)->headingTransform.position);
        float currentHeadingDistance = __ml__FRC7WVectorRC7WVector(&AsGolfDocCamera(golfDoc)->headingTransform.basis[6], &newReferenceOffset);
        unsigned state = (*(unsigned*)&AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags);
        if (state == kCameraForward && previousHeadingDistance * currentHeadingDistance < kFloat_0_camera &&
            AbsCameraValue(*(float*)&AsCameraTransform800D(transformBytes)->position[1] - AsGolfDocCamera((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->headingTransform.position[1]) < kFloat_3_2_CameraPosition) {
                    __as__7WVectorFRC7WVector(self->position, AsGolfDocCamera((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->headingTransform.position);
                    __pl__FRC7WVectorRC7WVector(&referenceTarget, AsGolfDocCamera((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->headingTransform.position, &AsGolfDocCamera((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->headingTransform.basis[6]);
                    __as__7WVectorFRC7WVector(&self->targetX, &referenceTarget);
                    float turn = (float)tan(AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->cameraPitchRadians);
                    float raised = self->targetY + turn;
                    self->heightOffsetRatio = kFloat_0_camera;
                    self->heightRatio = self->groundHeightRatio;
                    self->targetY = raised;
                    { Vec3*& transformVectorAlias = (Vec3*&)transformBytes;
                    transformVectorAlias = (Vec3*)s_cameraTransform;
                    __as__7WVectorFRC7WVector(AsCameraTransform800D(transformVectorAlias)->position, AsGolfDocCamera((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->headingTransform.position); }
                    AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->unk_2C0 = 0xFF;
                    (*(unsigned*)&AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags) = kCameraAtReference;
                    self->inputDelay = kFloat_0_25_GolfCamera;
        } else if (state == kCameraAtReference) {
            AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->unk_2C0 = 0x80;
            (*(unsigned*)&AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags) = kCameraForward;
        }
    }
}

// Consume the rotation ramp, rotate around the active pivot and rebuild the camera transform.
// angleStep/remainingRotationAngle are radians; sign-bit reads test float signs
// without converting their values. Rotation is about world Y, around either
// the reference position or orbitCenter, followed by terrain adjustment.
void CCamera_UpdateRotation(void* self, float dt) { VectorScratch16 pivotResult;  float remainingAngle = AsCCamera(self)->remainingRotationAngle;
    struct { BasisScratch36 basis; Vec3f translation; } matrix;
    Vec3f& translation = matrix.translation;
    BasisScratch36& basis = matrix.basis;
    float angleStep = AsCCamera(self)->rotationInputRamp * dt;

    if (remainingAngle < kFloat_0_camera || kFloat_0_camera < remainingAngle) {
        float remainingMagnitude = AsCCamera(self)->remainingRotationAngle;
        int finishRotation = 1;
        float stepMagnitude = angleStep;
        stepMagnitude = (*(unsigned*)(&(angleStep)) & kFloatSignMask) ? -stepMagnitude : stepMagnitude;

        if ((*(unsigned*)&AsCCamera(self)->remainingRotationAngle) & kFloatSignMask) remainingMagnitude = -remainingMagnitude;
        if (!(remainingMagnitude < stepMagnitude)) {
            float residualAngle = AsCCamera(self)->remainingRotationAngle - angleStep;
            if (*(unsigned*)(&residualAngle) & kFloatSignMask) residualAngle = -residualAngle;
            if (!(residualAngle < kCameraRotationEpsilon))
                finishRotation = 0;
        }
        if (finishRotation) {
            AsCCamera(self)->totalRotationAngle = kFloat_0_camera;
            AsCCamera(self)->remainingRotationAngle = kFloat_0_camera;
            angleStep = remainingAngle;
        } else {
            AsCCamera(self)->remainingRotationAngle = AsCCamera(self)->remainingRotationAngle - angleStep;
        }
    }
    // Consume at most the remaining requested turn before rotating the basis.
    AffineScratch48 turnMatrix; AffineScratch48 yawMatrix; AffineScratch48 pivotRotation;

    VectorScratch16 rotatedOffset; VectorScratch16 pivotOffset;
    Matrix_SetAxisRotation(&turnMatrix, angleStep, kYAxis);

    Matrix_ConcatAffine(&basis, s_cameraTransform, &turnMatrix); Matrix_CopyAffine(&basis, (void*)s_cameraTransform);
    Vec3_Zero(&translation);
    Matrix_SetAxisRotation(&yawMatrix, angleStep, kYAxis);
    Matrix_ConcatAffineAssign(&basis, &yawMatrix);
    CCamera_AddHeadingAngle(self, angleStep);
    void* golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);

    if (((*(unsigned*)&AsCGolfDoc(golfDoc)->gameStateFlags)) & kCameraAtReference) {
        *(WVector*)&translation = *(WVector*)(AsGolfDocCamera(golfDoc)->headingTransform.position);
    } else {
        Matrix_SetAxisRotation(&pivotRotation, angleStep, kYAxis);
        __mi__FRC7WVectorRC7WVector(&pivotOffset, (void*)((s_cameraTransform + kCameraPositionOffset)), AsGolfDocCamera(golfDoc)->orbitCenter);
        Matrix_TransformPoint(&rotatedOffset, &pivotOffset, &pivotRotation);
        __pl__FRC7WVectorRC7WVector(&pivotResult, AsGolfDocCamera(golfDoc)->orbitCenter, &rotatedOffset);
        *(WVector*)&translation = *(WVector*)&pivotResult;
        CQuadTree_AdjustPointToRegionAndGroundClearance(&turnMatrix, CQuadTree_GetInstance(), &translation);
        *(WVector*)&translation = *(WVector*)&turnMatrix;
        golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        if ((*(unsigned*)&AsCGolfDoc(golfDoc)->gameStateFlags) & kCameraFreeHeight)
            translation.y = *(const float*)((s_cameraTransform + kCameraHeightOffset));
    }
    Matrix_CopyAffine((void*)s_cameraTransform, &basis);
    *(WVector*)AsCCamera(self)->position = *(WVector*)((void*)((s_cameraTransform + kCameraPositionOffset)));
    __pl__FRC7WVectorRC7WVector(&turnMatrix, (void*)((s_cameraTransform + kCameraPositionOffset)), (void*)((s_cameraTransform + kCameraForwardOffset)));
    *(WVector*)((char*)&AsCCamera(self)->targetX) = *(WVector*)&turnMatrix;
    golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (((*(unsigned*)&AsCGolfDoc(golfDoc)->gameStateFlags) & kCameraNeedsGroundTransition) && !AsCCamera(self)->transitionActive)
        CCamera_SetupForwardGroundView(self, 1);
}


// Adjust terrain-relative height, using linear or curved interpolation around the reference view.
// World-distance rules in free-height mode: floor = terrain Y + 16; speed
// reaches 400 per dt unit at 300 above that floor, with a 0.2 minimum factor;
// ceiling = minHeight + 2598.4001. dt may be signed; no seconds conversion occurs.
// Other modes use dimensionless height ratios: 320 converts ratio to world Y;
// 192 scales backward heading displacement by offsetRatio*(1-groundHeightRatio).
void CCamera_UpdateVerticalMovement(void* self, float dt) {
    float savedDt = dt;
    float cameraMatrix[11];
    float groundPoint[3];
    unsigned cameraPositionWords[3];
    float referenceOffset[3], headingOffset[3];
    float groundAdjustedPosition[3];
    float headingTarget[3];
    void* golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned state = (*(unsigned*)&AsCGolfDoc(golfDoc)->gameStateFlags);
    if (state == kCameraFreeHeight) {
        // Sample floor, apply clearance-dependent vertical speed, then clamp.
        cameraPositionWords[0] = *(unsigned*)((s_cameraTransform + kCameraPositionOffset));
        cameraPositionWords[1] = *(unsigned*)((s_cameraTransform + kCameraHeightOffset));
        cameraPositionWords[2] = *(unsigned*)((s_cameraTransform + kCameraPositionZOffset));
        GetGroundPoint__9CQuadTreeF7WVectorbPib(groundPoint, CQuadTree_GetInstance(), cameraPositionWords, 1, 0, 1);
        float groundHeight = groundPoint[1];
        float minimumHeight = kCameraMinimumGroundClearance + groundHeight;
        float cameraHeight = *(float*)((s_cameraTransform + kCameraHeightOffset));
        float heightAboveFloor = cameraHeight - minimumHeight;
        if (heightAboveFloor < kFloat_300_Camera) {
            float movementScale = heightAboveFloor / kFloat_300_Camera;
            movementScale = MaxCameraValue(movementScale, kFloat_0_2_Camera);
            *(float*)((s_cameraTransform + kCameraHeightOffset)) += savedDt * (kFloat_400_CameraMove * movementScale);
        } else {
            *(float*)((s_cameraTransform + kCameraHeightOffset)) = cameraHeight + kFloat_400_CameraMove * savedDt;
        }
        *(float*)((s_cameraTransform + kCameraHeightOffset)) = ClampCameraValue(*(float*)((s_cameraTransform + kCameraHeightOffset)), minimumHeight,
                                                 kFloat_2598_4001_CameraMove + AsCCamera(self)->minHeight);
        return;
    }
    if (state == kCameraAtReference && dt > kFloat_0_camera) {
        (*(unsigned*)&AsCGolfDoc(golfDoc)->gameStateFlags) = kCameraForward;
        golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        *((unsigned char*)&AsCGolfDoc(golfDoc)->unk_2C0) = 0x80;
    }
    // Update normalized height, optionally moving back along the reference heading.
    CCamera* camera = AsCCamera(self);
    camera->heightOffsetRatio = ClampCameraValue(camera->heightInputRamp * dt + camera->heightOffsetRatio, kFloat_0_camera, kFloat_1_BallPhysics - camera->groundHeightRatio);
    if (camera->useLinearHeightInterpolation) {
        camera->heightRatio = camera->groundHeightRatio + camera->heightOffsetRatio;
    } else {
        if (((CGolfBall*)GolfBall__Fv())->m_groundType == 2)
            camera->heightRatio = camera->groundHeightRatio + camera->heightOffsetRatio;
        else
            camera->heightRatio = camera->heightOffsetRatio * camera->heightOffsetRatio + camera->groundHeightRatio;
        golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        float angle = kFloat_192_Camera * (camera->heightOffsetRatio * (kFloat_1_BallPhysics - camera->groundHeightRatio));
        __ml__FfRC7WVector(headingOffset, &AsGolfDocCamera(golfDoc)->headingTransform.basis[6], angle);
        __mi__FRC7WVectorRC7WVector(referenceOffset, AsGolfDocCamera(golfDoc)->headingTransform.position, headingOffset);
        __as__7WVectorFRC7WVector((char*)s_cameraTransform + kCameraPositionOffset, referenceOffset);
    }
    // Publish world position, ground-clear it, and rebuild target/pitch/transform.
    char* transformBytes = (char*)s_cameraTransform;
    *(float*)((transformBytes + kCameraHeightOffset)) = kFloat_320_Camera * camera->heightRatio + camera->minHeight;
    CQuadTree_AdjustPointToRegionAndGroundClearance(groundAdjustedPosition, CQuadTree_GetInstance(), (transformBytes + kCameraPositionOffset));
    __as__7WVectorFRC7WVector((transformBytes + kCameraPositionOffset), groundAdjustedPosition);
    __as__7WVectorFRC7WVector(camera->position, (transformBytes + kCameraPositionOffset));
    golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    __pl__FRC7WVectorRC7WVector(headingTarget, (transformBytes + kCameraPositionOffset), &AsGolfDocCamera(golfDoc)->headingTransform.basis[6]);
    __as__7WVectorFRC7WVector(&camera->targetX, headingTarget);
    float facing = atan(kFloat_0_65_Camera * camera->heightOffsetRatio);
    golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    float pitch = AsCGolfDoc(golfDoc)->cameraPitchRadians - facing;
    camera->targetY += (float)tan(MinCameraValue(pitch, kFloat_0_camera));
    Matrix_BuildCameraTransform(cameraMatrix, camera->position, &camera->targetX);
    Matrix_CopyAffine((void*)s_cameraTransform, cameraMatrix);
    if (!camera->useLinearHeightInterpolation && camera->heightRatio == camera->referenceHeightRatio) {
        *(unsigned*)&AsCGolfDoc((*(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->gameStateFlags = kCameraAtReference;
        AsCGolfDoc((*(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->unk_2C0 = 0xFF;
    }
}

#pragma push
#pragma opt_common_subs off
// Select named camera views, pan their target, and ramp held rotation/height input.
// dt is elapsed time in the caller's movement scale; positions use world XYZ.
// Named-view panning keeps the eye fixed and moves only the target along the
// current right/up basis. Clamp-only paths do not rebuild the transform.
void CCamera_HandleMovementInput(void* self, float dt) {
    float savedDt = dt;
    // UI/drop gates apply to all camera keys and pointer regions below.
    if (!CItemWindow::IsClosed()) return;
    // One ABI scratch word has two phases. Name the active union member at every
    // use: golfDoc for document gates/lookups, keyStrings for action-name offsets.
    // Each transition writes that member before reading it; no pointer type punning.
    union LookupScratch { void* golfDoc; const char* keyStrings; } lookup;
    lookup.golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (*((unsigned char*)&AsCGolfDoc(lookup.golfDoc)->dropRequest) != 0) return;
    // Named-view hotkeys copy a world transform and convert stored FOV degrees
    // to radians. A handled key returns immediately, before held-input ramps.
    {
        const char*& names = lookup.keyStrings;
        names = cameraActorStrings;
        for (int i = kFirstCameraHotkey; kCameraHotkeyEnd > i; ++i) {
            void* cameraName = String_FormatTemporary(names + kPadKeyFormat, i);
            float targetPosition[3];
            if (!Pointer_GetRisingActionBits(g_inputPointer, cameraName, 1)) {
                cameraName = String_FormatTemporary(names + kNumberKeyFormat, i);
                if (!Pointer_GetRisingActionBits(g_inputPointer, cameraName, 1)) continue;
            }
            lookup.golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
            cameraName = String_FormatTemporary(cameraActorStrings + kCameraNameFormat, i);
            void* cameraRecord = CourseData_FindNameRecord(AsCGolfDoc(lookup.golfDoc)->courseData, cameraName);
            if (!cameraRecord) return;
            AsCCamera(self)->horizontalOffset = kFloat_0_camera;
            AsCCamera(self)->verticalOffset = kFloat_0_camera;
            float unusedVectorScratch[3];
            float forwardOffset[3];
            __as__7WVectorFRC7WVector((char*)AsCCamera(self)->position, ((CameraNameRecord*)cameraRecord)->transform.position);
            __ml__FfRC7WVector(forwardOffset, &((CameraNameRecord*)cameraRecord)->transform.basis[6], kFloat_10_Camera);
            __pl__FRC7WVectorRC7WVector(targetPosition, (char*)AsCCamera(self)->position, forwardOffset);
            __as__7WVectorFRC7WVector((char*)&AsCCamera(self)->targetX, targetPosition);
            Matrix_CopyAffine((void*)s_cameraTransform, &((CameraNameRecord*)cameraRecord)->transform);
            Camera_SetFov((void*)g_view, kDegreesToRadians_CameraFov * ((CameraNameRecord*)cameraRecord)->fieldOfViewDegrees);
            void* resetDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
            (*(unsigned*)&AsCGolfDoc(resetDoc)->gameStateFlags) = kCameraNamedView;
            return;
        }
    }
    lookup.golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    if ((*(unsigned*)&AsCGolfDoc(lookup.golfDoc)->gameStateFlags) != kCameraNamedView) return;
    // Named-view panning: change target along camera right/up, keep eye fixed,
    // and rebuild the world transform only while offsets stay inside their limits.
    AsCCamera(self)->rotationDirection = 0;
    int hitRegion = AsCMouseCursor(g_mouseCursor)->hitRegion;
    if (hitRegion == kPanLeft) {
        if (Pointer_GetButtonState(g_inputPointer, 0) > 0) {
            float speed = kFloat_5_Camera * AsCCamera(self)->rotationInputRamp;
            AsCCamera(self)->rotationDirection = 1;
            float value = AsCCamera(self)->horizontalOffset - speed * savedDt;
            AsCCamera(self)->horizontalOffset = value;
            if (value > kFloat_10_Camera) AsCCamera(self)->horizontalOffset = kFloat_10_Camera;
            else if (value < kFloat_neg10_Camera) AsCCamera(self)->horizontalOffset = kFloat_neg10_Camera;
            else {
                float vec[3];
                __ml__FfRC7WVector(vec, (void*)s_cameraTransform, (speed * savedDt));
                __ami__7WVectorFRC7WVector((char*)&AsCCamera(self)->targetX, vec);
                float rot[12];
                Matrix_BuildCameraTransform(rot, (char*)AsCCamera(self)->position, (char*)&AsCCamera(self)->targetX);
                Matrix_CopyAffine((void*)s_cameraTransform, rot);
            }
        }
    }
    hitRegion = AsCMouseCursor(g_mouseCursor)->hitRegion;
    if (hitRegion == kPanRight) {
        if (Pointer_GetButtonState(g_inputPointer, 0) > 0) {
            float value;
            AsCCamera(self)->rotationDirection = -1;
            register float reverseDt;
            float speed;
            reverseDt = kFloat_neg1_Camera * savedDt;
            float limit;
            limit = kFloat_5_Camera;
            speed = AsCCamera(self)->rotationInputRamp;
            float product = limit * speed;
            speed = product;
            value = AsCCamera(self)->horizontalOffset - speed * reverseDt;
            AsCCamera(self)->horizontalOffset = value;
            if (value > kFloat_10_Camera) AsCCamera(self)->horizontalOffset = kFloat_10_Camera;
            else if (value < kFloat_neg10_Camera) AsCCamera(self)->horizontalOffset = kFloat_neg10_Camera;
            else {
                float vec[3];
                float rot[12];
                __ml__FfRC7WVector(vec, (void*)s_cameraTransform, (speed * reverseDt));
                __ami__7WVectorFRC7WVector((char*)&AsCCamera(self)->targetX, vec);
                Matrix_BuildCameraTransform(rot, (char*)AsCCamera(self)->position, (char*)&AsCCamera(self)->targetX);
                Matrix_CopyAffine((void*)s_cameraTransform, rot);
            }
        }
    }
    // Direct heading keys require no Control/Shift modifier; angles are radians.
    // Keep the short-circuit order: each action query observes current input state.
    {
        const char*& names = lookup.keyStrings;
        names = cameraActorStrings;
        if (Pointer_IsActionActive(g_inputPointer, names + kIncreaseHeading, 1) &&
            !Pointer_IsActionActive(g_inputPointer, names + kLeftControl, 1) &&
            !Pointer_IsActionActive(g_inputPointer, names + kRightControl, 1) &&
            !Pointer_IsActionActive(g_inputPointer, names + kLeftShift, 1) &&
            !Pointer_IsActionActive(g_inputPointer, names + kRightShift, 1))
            CCamera_AddHeadingAngle(self, kCameraRotationRate * savedDt);
    }
    {
        const char*& names = lookup.keyStrings;
        names = cameraActorStrings;
        if (Pointer_IsActionActive(g_inputPointer, names + kDecreaseHeading, 1) &&
            !Pointer_IsActionActive(g_inputPointer, names + kLeftControl, 1) &&
            !Pointer_IsActionActive(g_inputPointer, names + kRightControl, 1) &&
            !Pointer_IsActionActive(g_inputPointer, names + kLeftShift, 1) &&
            !Pointer_IsActionActive(g_inputPointer, names + kRightShift, 1))
            CCamera_AddHeadingAngle(self, kFloat_neg1_0471976_CameraRotation * savedDt);
    }
    // Continued direction ramps toward 1; releasing/reversing resets it to zero.
    int direction = AsCCamera(self)->rotationDirection;
    if (s_previousCameraRotationDirection == direction && direction) {
        float v = AsCCamera(self)->rotationInputRamp + savedDt;
        float result;
        if (v < kFloat_0_camera) result = kFloat_0_camera; else if (v > kFloat_1_BallPhysics) result = kFloat_1_BallPhysics; else result = v;
        AsCCamera(self)->rotationInputRamp = result;
    } else AsCCamera(self)->rotationInputRamp = kFloat_0_camera;

    // Vertical target panning uses its own direction and acceleration ramp.
    AsCCamera(self)->movementDirection = 0;
    void* cursorState = g_mouseCursor;
    s_previousCameraRotationDirection = (AsCCamera(self)->rotationDirection);
    hitRegion = AsCMouseCursor(cursorState)->hitRegion;
    if ((hitRegion == kRaiseTarget || (hitRegion == kRaiseTargetButton && Pointer_GetButtonState(g_inputPointer, 0) > 0))) {

        float value;
        float speed;
        float limit = kFloat_5_Camera;
        speed = limit * AsCCamera(self)->movementInputRamp;
        AsCCamera(self)->movementDirection = 1;
        value = AsCCamera(self)->verticalOffset + speed * savedDt;
        AsCCamera(self)->verticalOffset = value;
        if (value > limit) AsCCamera(self)->verticalOffset = limit;
        else if (value < kFloat_neg5_CameraVerticalOffset) AsCCamera(self)->verticalOffset = kFloat_neg5_CameraVerticalOffset;
        else {

            float vec[3];
            __ml__FfRC7WVector(vec, (char*)s_cameraTransform + kCameraUpOffset, (speed * savedDt));
            float rot[12];
            __apl__7WVectorFRC7WVector((char*)&AsCCamera(self)->targetX, vec);
            Matrix_BuildCameraTransform(rot, (char*)AsCCamera(self)->position, (char*)&AsCCamera(self)->targetX);
            Matrix_CopyAffine((void*)s_cameraTransform, rot);
        }
    }
    hitRegion = AsCMouseCursor(g_mouseCursor)->hitRegion;
    if ((hitRegion == kLowerTarget || (hitRegion == kLowerTargetButton && Pointer_GetButtonState(g_inputPointer, 0) > 0))) {
        float value;
        float reverseDt = (kFloat_neg1_Camera * savedDt);
        float speed;
        float limit = kFloat_5_Camera;
        speed = limit * AsCCamera(self)->movementInputRamp;
        AsCCamera(self)->movementDirection = -1;

        value = AsCCamera(self)->verticalOffset + speed * reverseDt;
        AsCCamera(self)->verticalOffset = value;
        if (value > limit) AsCCamera(self)->verticalOffset = limit;
        else if (value < kFloat_neg5_CameraVerticalOffset) AsCCamera(self)->verticalOffset = kFloat_neg5_CameraVerticalOffset;
        else {
            float vec[3];
            float rot[12];
            __ml__FfRC7WVector(vec, (char*)s_cameraTransform + kCameraUpOffset, (speed * reverseDt));
            __apl__7WVectorFRC7WVector((char*)&AsCCamera(self)->targetX, vec);
            Matrix_BuildCameraTransform(rot, (char*)AsCCamera(self)->position, (char*)&AsCCamera(self)->targetX);
            Matrix_CopyAffine((void*)s_cameraTransform, rot);
        }
    }
    int verticalPanDirection = AsCCamera(self)->movementDirection;
    if (s_previousCameraMovementDirection == verticalPanDirection && verticalPanDirection) {
        float v = AsCCamera(self)->movementInputRamp + savedDt;
        float result;
        if (v < kFloat_0_camera) result = kFloat_0_camera; else if (v > kFloat_1_BallPhysics) result = kFloat_1_BallPhysics; else result = v;
        AsCCamera(self)->movementInputRamp = result;
    } else AsCCamera(self)->movementInputRamp = kFloat_0_camera;
    s_previousCameraMovementDirection = AsCCamera(self)->movementDirection;
}

#pragma pop
// Wrap the heading, recompute its terrain-clamped reference position and notify dependent actors.
void CCamera_AddHeadingAngle(void* self, float headingDelta) {
    // Normalize radians into [-pi,pi] with the original endpoint comparisons.
    float angle = headingDelta + AsCGolfDoc(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->headingRadians;
    float absAngle = angle > kFloat_0_camera ? angle : -angle;
    if (absAngle > kFloat_6_2831855_CameraAngleWrap) {
        float turns = (float)floor(angle / kFloat_6_2831855_CameraAngleWrap);
        angle = angle - kFloat_6_2831855_CameraAngleWrap * turns;
    }
    if (angle > kFloat_3_1415927_Camera)
        angle -= kFloat_6_2831855_CameraAngleWrap;
    else if (angle < kFloat_neg3_1415927_CameraAngleWrap)
        angle += kFloat_6_2831855_CameraAngleWrap;

    // Rebuild the world-space reference from orbit center/distance/height.
    void* golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    AsCGolfDoc(golfDoc)->headingRadians = angle;
    unsigned char headingMatrix[0x30], orbitOffset[0x10], groundAdjustedPosition[0x0C], requestedPosition[0x0C];
    golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    Matrix_SetAxisRotation(headingMatrix, AsCGolfDoc(golfDoc)->headingRadians + kFloat_0_camera, kYAxis);
    Matrix_CopyAffine(&AsGolfDocCamera(golfDoc)->headingTransform, headingMatrix);
    golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    __ml__FfRC7WVector(orbitOffset, &AsGolfDocCamera(golfDoc)->headingTransform.basis[6], kFloat_neg1_Camera * AsGolfDocCamera(golfDoc)->orbitDistance);
    *(float*)(orbitOffset + 4) = AsGolfDocCamera((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->orbitHeight;
    golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    __pl__FRC7WVectorRC7WVector(requestedPosition, AsGolfDocCamera(golfDoc)->orbitCenter, orbitOffset);
    CQuadTree_AdjustPointToRegionAndGroundClearance(groundAdjustedPosition, CQuadTree_GetInstance(), requestedPosition);
    *(WVector*)(AsGolfDocCamera(golfDoc)->headingTransform.position) = *(WVector*)groundAdjustedPosition;
    golfDoc = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    float minimumHeight = AsCCamera(self)->minHeight;
    float maximumHeight = AsCCamera(self)->maxHeight;
    float referenceHeight = AsGolfDocCamera(golfDoc)->headingTransform.position[1];
    AsCCamera(self)->headingChanged = 1;
    AsCCamera(self)->referenceHeightRatio = (referenceHeight - minimumHeight) / (maximumHeight - minimumHeight);
    // Notify dependent actors with a borrowed pointer to this call's angle delta.
    const char* names = cameraActorStrings;
    IActor_DispatchMessage(self, self, names + kCameraPlayer, 2, &headingDelta, 0, 0, 0);
    IActor_DispatchMessage(self, self, names + kCameraSpot, 0, &headingDelta, 0, 0, 0);
    IActor_DispatchMessage(self, self, names, 0, &headingDelta, 0, 0, 0);
    IActor_DispatchMessage(self, self, names + kCameraGrid, 0, &headingDelta, 0, 0, 0);
    IActor_DispatchMessage(self, self, names + kCameraCaddie, 0x12, &headingDelta, 0, 0, 0);
    IActor_DispatchMessage(self, self, names + kCameraGuideLine, 0, 0, 0, 0, 0);
    float hasInstances = kFloat_0_camera;
    unsigned count = ((CameraInstanceTableView*)InstanceTable_Get())->count;
    hasInstances = (float)(unsigned)(count != 0);
    IActor_DispatchMessage(self, self, names, 9, &hasInstances, 0, 0, 0);
}

// Return the camera type descriptor.
void* CCamera_GetRTTI() {
    return &s_cameraTypeInfo;
}

void __dt__18NativeBase80201D18Fv(void*, int);
void __dl__FPv(void*);
// Reset input gates, destroy the base actor and optionally free the camera.
void* CCamera_dtor(void* self, int shouldFree) {
    if (self != 0) {
        (*(int*)&AsCCamera(self)->zoomInputBlocked) = 0;
        (*(int*)&AsCCamera(self)->directionalInputEnabled) = 1;
        __dt__18NativeBase80201D18Fv(self, 0);
        if (shouldFree > 0)
            __dl__FPv(self);
    }
    return self;
}

void IntPair_Set(void*, const char*, void*);
void* ActorFactoryRegistry_GetInstance();
void StringMap_SetByName(void*, void*, const char*);
void CCamera_Create();
void* TaskRegistry_GetInstance();
void TaskRegistry_AppendFactory(void*, const char*);
// Initialize the angle cache and register camera actor/task factories.
void CCamera_RegisterType() {
    s_cameraUnitTwoPiCache = kFloat_6_2831855;
    IntPair_Set(&s_cameraTypeInfo, (const char*)&cameraActorStrings + kCameraType, &s_actorTypeInfo);
    StringMap_SetByName(ActorFactoryRegistry_GetInstance(), (void*)CCamera_Create, &cameraFactoryTypeName);
    TaskRegistry_AppendFactory(TaskRegistry_GetInstance(), &cameraTaskFactoryName);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804620E4", _ctors_u800D3DB4_0, CCamera_RegisterType);
#pragma pop

