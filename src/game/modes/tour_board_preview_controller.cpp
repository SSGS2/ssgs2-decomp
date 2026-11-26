// Tour board preview: the camera controller (CCameraController) that moves the preview model
// across the map, with its heading, travel and animations, the boss preview, and the placement
// of the model on the board plane by unprojecting screen points.

#include <decomp/static_init.h>
// Tour board model placement, widescreen unprojection, and frame rate controls.
// Lookup (scalars: the data section; model/animation methods: tour_task_item_map_and_model_preview.cpp;
// layouts: CCameraController.h and TourBoardPreviewState.h):
// - Travel speed: UpdatePosition, kFloat_4_000001_CameraController is exactly
//   4.000000953674316 board units/CALL. Larger steps reach nodes sooner; the
//   arrival call still takes the full step. Keep arrival/animation events together.
// - Turning: UpdateHeading, smoothing divisor 3 (larger = slower), snap divisor
//   360 (threshold pi/360 radians; larger = smaller snap region).
// - Marker spin: UpdatePlacement, kTourPreviewHeadingStep = pi/60 radians/CALL,
//   wrap 2*pi; modes 1/5/6 do not auto-spin.
// - Widescreen: UnprojectToHeightPlane and UpdatePosition fetch the CURRENT
//   viewport/projection/view. Keep board origin, projection, viewport, and side
//   offsets (+/-32,-16) in one space; there is no aspect setting in this unit.
// - Constraint: dead zone 10 board units; radius 2*16; line (0,0) to (1,1).
//   The 16 scalar is SHARED with side-preview Y offset. A future mod needing
//   separate controls should separate the consumers instead of editing the pool.
// - Models: tourControllerPreviewModelNames[8], tourBossPreviewModelsByIndex[8],
//   tourPreviewModelsByMode[5]; animations: tourPreviewAnimationNames[0..4].
// Safe tuning ranges beyond the documented table domains are not established.
// Frame rate edits must handle per-call motion/spin, animation delta, and sound
// crossings separately. This unit has no particle/fade update policy.
#define AsTourBoardPreviewState(p) ((TourBoardPreviewState*)(p))
#include "classes/TourBoardPreviewState.h"
#include "classes/CCameraController.h"
#include "classes/CCameraOwner.h"
#include "classes/TourModelPreviewView.h"

#define AsCCameraController(p) ((CCameraController*)(p))

// Coordinate contract: controller positions and mapOrigin are 2D board/layout
// coordinates. Subtract mapOrigin to obtain viewport XY, then unproject onto a
// world-space horizontal plane for ModelPreview_SetPosition. Headings are radians.
// Item/node pointers are borrowed; each owner destroys its allocated model preview.
// Movement and automatic heading increments are per update call; only animation
// advancement uses deltaTime in SECONDS. ModelPreview_UpdateAnimation adds it to
// range time; WPuppet_SetAnimationTimeFromRanges (wpuppet_model_bone_animation.cpp) subtracts the
// selected range start then multiplies by 30 before CPetAnimation::SetModeAndAmount.
// Thus footstep thresholds are seconds on the same 30-animation-frames/s clock.
// Preserve byte cursors where typed member addresses lengthen register lifetimes.
enum {
    kControllerPositionOffset = 8,
    kControllerPreviousPositionOffset = 0x10
};
enum TourPreviewMode {
    kNoTourPreview = 0,
    kTourCharacterPreview = 1,
    kTourLeftPreview = 5,
    kTourRightPreview = 6
};
enum TourAnimationIndex {
    kIdleAnimation,
    kMoveAnimation,
    kActionAnimationA,
    kActionAnimationB,
    kTourActionAnimation
};
enum {
    kPreviewVisibleBit = 10,
    kPreviewExcludedFlag13Bit = 13,
    kPreviewExcludedFlag14Bit = 14,
    kProjectionTransform = 1,
    kViewTransform = 0
};
struct TourPreviewNode {
    char reserved00[0xd];
    signed char previewMode;
    short modelIndex;
};

extern "C" {

void TourModelPreview_UpdatePlacement(void*, float, void*, void*);
int ModelPreview_ReturnSuccess(void*);
void ModelPreview_Render(void*);
void gtl_Vec2_Copy(void*, void*);
int ModelPreview_IsAnimationNamed(void*, void*);
void ModelPreview_SetAnimationByName(void*, void*);
void ModelPreview_LoadModel(void*, void*);
void ModelPreview_Ctor(void*, int);
int OpenItemEntry_GetFlagBit15(void*);
int OpenItemEntry_GetFlagBit13(void*);
int OpenItemEntry_GetFlagBit14(void*);
void* __nw__FUl(unsigned long);
void OpenItemEntry_GetPosition(void*, void*);
void ModelPreview_SetHeading(void*, float);
void ModelPreview_ReleaseModel(void*);
int Vec2_Equals(void*, void*);
void Sound_PlayByName(void*, void*, void*, int, void*, int, float, float, float);
double atan2(float, float);
void ModelPreview_GetAnimationName(void*);
void ModelPreview_UpdateAnimation(void*, float);
void ModelPreview_SetupRenderParams(void*);
void ModelPreview_UpdateCamera(void*);
void ModelPreview_Dtor(void*, int);
void __dl__FPv(void*);
extern const float kFloat_6_2831855;
extern float s_cameraControllerRotationPeriod;
extern float s_tourBossPreviewRotationPeriod;
extern float kFloat_0_CameraController;
extern const float kFloat_3_1415927_CameraController_805E11D8;
extern const float kFloat_32_TourPointer;
extern const float kFloat_16_TourMap;
extern const float kTourPreviewDisplacementDeadZone;
extern const float kTourPreviewConstraintRadiusScale;
extern const float kTourPreviewHeadingStep;
extern const float kTourPreviewHeadingWrapPeriod;
extern const float kTourPreviewConstraintLineEndX;
extern const float kTourPreviewConstraintLineEndY;
extern const float kFloat_3_1415927_CameraController_805E119C;
extern const float kFloat_3_1415927;
extern void* g_soundManager;
extern const float kFloat_0_805E1198;
extern const float kFloat_2_CameraController;
extern float kCameraControllerHeadingSmoothingDivisor;
extern const float kCameraControllerHeadingSnapDivisor;
extern const float kFloat_0_5_TourPreviewFootstepSound;
extern const float kFloat_200_CameraControllerSound;
extern const float kFloat_1_CameraControllerSound;
extern char tourPreviewFootstepSoundNames[];
extern char tourBossPreviewAnimationName[];
extern int tourPreviewModelsByMode[];
extern int tourBossPreviewModelsByIndex[];
extern unsigned int tourPreviewFootstepAnimationTimes[];
struct TourPreviewAnimationTable {
    void* animations[4];
    void* tourAction;
};
extern TourPreviewAnimationTable tourPreviewAnimationNames;
extern void* tourControllerPreviewModelNames[];
extern char s_cameraRayTemplatePoolAnchor[];
extern unsigned int s_cameraRayDefaultTemplates[14];
// Read-only templates following the resource-name anchor: up normal (0,1,0),
// first ray point (0,0,-1), second ray point (0,0,-0.10000000149011612).
// These Z values are viewport-unprojection inputs, not the camera near/far clip
// distances. Both ScreenPoint_UnprojectToHeightPlane and UpdatePosition use them;
// Word copies preserve float bits and load/store order. viewport[6] scratch below
// is a 24-byte MIXED ABI record: uint32 x,y,width,height; float depthNear,depthFar,
// despite the float array spelling. Matrices are row-major 16-float row-vector
// transforms; identity world makes unprojection return world coordinates.
struct CameraRayTemplatePool {
    char resourceNamesAndMatrices[0xD8];
    unsigned int planeNormal[3];
    unsigned int nearScreen[3];
    unsigned int farScreen[3];
};
enum CameraRayTemplateOffset {
    kPlaneNormalTemplate = (int)&((CameraRayTemplatePool*)0)->planeNormal,
    kNearScreenTemplate = (int)&((CameraRayTemplatePool*)0)->nearScreen,
    kFarScreenTemplate = (int)&((CameraRayTemplatePool*)0)->farScreen
};
extern const float kFloat_4_000001_CameraController;
extern const float kTourPreviewMapPlaneHeight;
double pow(double, double);
double sqrt(double);
void Engine_GetViewport(void*);
void Engine_GetTransform(int, void*);
void Mtx44_SetTranslation(void*, float, float, float);
void Mtx44_SetRotationY(void*, float);
void PSMTX44Concat(void*, void*, void*);
// ABI order: outWorld, viewportPoint, viewport, projection, view, world.
void Vec3_UnprojectFromViewport(void*, void*, void*, void*, void*, void*);
float Vec3_Dot(void*, void*);
void gtl_Vec2_Subtract(void*, void*, void*);
float Vec2_Length(void*);
void Vec2_Scale(void*, void*, float);
void gtl_Vec2_Add(void*, void*, void*);
void ModelPreview_SetPosition(void*, void*);

// Set the desired heading used by the smoothed preview rotation.
// self required; heading radians should remain within one turn of [-pi,pi],
// since UpdateHeading corrects by only one turn, not a general modulo reduction.
void CCameraController::SetTargetHeading(float heading) {
    targetHeading = heading;
}

// Return the board item currently occupied by the preview.
// self required; borrowed pointer may be null before an item is selected.
void* CCameraController::GetItemEntry() {
    return (*(void**)&itemEntry);
}

// Initialize the controller rotation-period value.
void CameraController_InitRotationPeriod(void) {
    s_cameraControllerRotationPeriod = kFloat_6_2831855;
}

// Forward the tour preview placement update.
void TourModelPreview_UpdatePlacementForward(void* self, float deltaTime, void* mapOrigin, void* referencePosition) {
    TourModelPreview_UpdatePlacement(self, deltaTime, mapOrigin, referencePosition);
}

// 0x802F464C tail-branches to ModelPreview_ReturnSuccess (0x802F4650); callers test the result (0x802EC668, 0x802EE940).
// Return the model preview readiness/success result.
// Requires self/modelPreview. The callee returns success unconditionally; this
// does not prove asynchronous model-resource readiness.
int CameraController_PollPreview(void* self) {
    return ModelPreview_ReturnSuccess(AsCCameraController(self)->modelPreview);
}

// Render the controller-owned model preview.
void CameraController_RenderPreview(void* self) {
    ModelPreview_Render(AsCCameraController(self)->modelPreview);
}

// Initialize the tour preview rotation-period value.
void TourBossPreview_InitRotationPeriod(void) {
    s_tourBossPreviewRotationPeriod = kFloat_6_2831855;
}

// Select an animation from the shared tour preview name table.
// animationIndex is [0,5); no bounds check. self/preview required; table strings
// are borrowed. A missing animation keeps the old range/time (tour_task_item_map_and_model_preview.cpp).
void CCameraController::SetAnimationByIndex(unsigned int animationIndex) {
    ModelPreview_SetAnimationByName(modelPreview, ((void**)&tourPreviewAnimationNames)[animationIndex]);
}

// Remember the model index and load its tour preview model.
// modelIndex is [0,8), also used for the 16-float footstep table in Update.
// Replaces/releases the prior model through ModelPreview_LoadModel; no status
// is returned, so a failed load can leave the preview without a model.
void CCameraController::SetModelIndex(unsigned int modelIndex) {
    this->modelIndex = modelIndex;
    ModelPreview_LoadModel(modelPreview, tourControllerPreviewModelNames[modelIndex]);
}

// Initialize tour-preview flag storage, displacement and heading.
// self required; nodeFlagsAddress is a borrowed halfword address encoded in a
// target-width integer, retained until destruction. previewNode/position are
// initialized separately by the caller; this is not a complete constructor.
void TourBossPreview_InitState(void* self, unsigned int nodeFlagsAddress) {
    float f0 = kFloat_0_CameraController;
    float f1 = kFloat_3_1415927_CameraController_805E11D8;
    (*(unsigned int*)&AsTourBoardPreviewState(self)->flags) = nodeFlagsAddress;
    AsTourBoardPreviewState(self)->unk_4 = 0;
    (*(unsigned int*)&AsTourBoardPreviewState(self)->modelPreview) = 0;
    AsTourBoardPreviewState(self)->heading = f1;
    AsTourBoardPreviewState(self)->displacement[0] = f0;
    AsTourBoardPreviewState(self)->displacement[1] = f0;
}

// Copy the current board-space position.
// self and outPosition (float[2]) required; copies XY without transferring ownership.
void CCameraController::GetPosition(void* outPosition) {
    gtl_Vec2_Copy(outPosition, &positionX); // 0x802F4320 unit: position at +8
}

// Set both current and previous positions from the occupied board item.
// Requires a live itemEntry; resetting both samples prevents a false travel heading.
void CCameraController::ResetPositionToItem() {
    char* base = (char*)this;
    OpenItemEntry_GetPosition((*(void**)&AsCCameraController(base)->itemEntry), base + kControllerPositionOffset);
    gtl_Vec2_Copy(base + kControllerPreviousPositionOffset, base + kControllerPositionOffset);
}

// Select the occupied board item and reset both position samples.
// itemEntry required and borrowed through subsequent updates; does not clear a
// pending target or moving state. Use the travel/reset operations for those.
void CCameraController::SetItemEntry(void* itemEntry) {
    char* base = (char*)this;
    (*(void**)&AsCCameraController(base)->itemEntry) = itemEntry;
    OpenItemEntry_GetPosition(itemEntry, base + kControllerPositionOffset);
    gtl_Vec2_Copy(base + kControllerPreviousPositionOffset, base + kControllerPositionOffset);
}

// Render the tour preview only when flag 10 is set and flags 13/14 are clear.
// self required; absent modelPreview returns before reading the borrowed flags.
// Unknown meanings of bits 13/14 are kept numeric; both suppress this draw.
void TourBossPreview_RenderIfVisible(void* self) {
    void* modelPreview = AsTourBoardPreviewState(self)->modelPreview;
    if (modelPreview == 0)
        return;
    unsigned short flags = *(*(unsigned short**)&AsTourBoardPreviewState(self)->flags);
    if (((flags >> kPreviewVisibleBit) & 1) == 0)
        return;
    if (((flags >> kPreviewExcludedFlag14Bit) & 1) != 0)
        return;
    if (((flags >> kPreviewExcludedFlag13Bit) & 1) != 0)
        return;
    ModelPreview_Render(modelPreview);
}

// 0x802F4E04 tail-branches to ModelPreview_IsAnimationNamed (0x802F4E14); CTourMain_ProcessItemMenu tests the result (0x802ED830).
// Test whether the preview is playing the tour-action animation.
int CCameraController::IsTourActionAnimation() {
    return ModelPreview_IsAnimationNamed(modelPreview, tourPreviewAnimationNames.tourAction);
}

// Stop travel and restore the idle animation and default heading.
void CCameraController::ResetPreview() {
    char* base = (char*)this;
    float pi = kFloat_3_1415927_CameraController_805E119C;
    AsCCameraController(base)->moving = 0;
    AsCCameraController(base)->targetHeading = pi;
    AsCCameraController(base)->currentHeading = pi;
    ModelPreview_SetAnimationByName(AsCCameraController(base)->modelPreview,
                                    ((void**)&tourPreviewAnimationNames)[kIdleAnimation]);
    ModelPreview_SetHeading(AsCCameraController(base)->modelPreview, AsCCameraController(base)->currentHeading);
}

// Report whether either of the two action animations is active.
int CCameraController::IsEitherActionAnimation() {
    void* object = this;
    void** table = (void**)&tourPreviewAnimationNames;
    int result = ModelPreview_IsAnimationNamed(AsCCameraController(object)->modelPreview, table[3]);
    int result2 = ModelPreview_IsAnimationNamed(AsCCameraController(object)->modelPreview, table[2]);
    return result2 | result;
}

// Release and destroy the owned model preview. ModelPreview_ReleaseModel runs
// first so the preview restores its saved camera state before it is destroyed.
CCameraOwner::~CCameraOwner() {
    ModelPreview_ReleaseModel(modelPreview);
    if (modelPreview != 0) {
        ModelPreview_Dtor(modelPreview, 1);
        modelPreview = 0;
    }
}

// Release and destroy the tour preview camera, then optionally delete its owner.
// Null self/preview accepted; borrowed flags/node are never freed. deleting > 0
// also deletes self, leaving a dangling return pointer.
void* TourBossPreview_Destroy(void* self, int deleting) {
    void* object = self;
    int freeFlag = deleting;
    if (object != 0) {
        void* p = AsTourBoardPreviewState(object)->modelPreview;
        if (p != 0) {
            ModelPreview_ReleaseModel(p);
            p = AsTourBoardPreviewState(object)->modelPreview;
            if (p != 0) {
                ModelPreview_Dtor(p, 1);
                AsTourBoardPreviewState(object)->modelPreview = 0;
            }
        }
        if (freeFlag > 0)
            __dl__FPv(object);
    }
    return object;
}

// Initialize movement and animation state and load the selected preview model.
// modelIndex is [0,8). The controller owns the allocated 0x98-byte model preview,
// released by the owner's destructor. No item is selected yet; a failed load
// leaves no model in the preview.
CCameraController::CCameraController(int modelIndex) {
    itemEntry = 0;
    targetItemEntry = 0;
    moving = 0;
    struct AllocatedCamera {
        char storage[0x98];
        // Construct the controller-owned model preview in allocated storage.
        AllocatedCamera() {
            ModelPreview_Ctor(this, 0);
        }
    };
    AllocatedCamera* nw = new AllocatedCamera;
    float f0 = kFloat_0_805E1198;
    float f1 = kFloat_3_1415927_CameraController_805E119C;
    modelPreview = nw;
    targetHeading = f1;
    currentHeading = f1;
    previousAnimationTime = f0;
    previousPositionX = f0;
    previousPositionY = f0;
    positionX = f0;
    positionY = f0;
    this->modelIndex = modelIndex;
    ModelPreview_LoadModel(modelPreview, tourControllerPreviewModelNames[modelIndex]);
}

// Returns 1 only when travel starts. Requires a current item and valid preview.
// Either endpoint must have bit 15, and destination bits 13/14 must be clear.
// Borrow destinationItem through arrival; calling while moving returns 0.
// Validate a destination item and start travel with the moving animation.
int CCameraController::TryMoveToItem(void* destinationItem) {
    void* object = this;
    void* param = destinationItem;
    if (AsCCameraController(object)->moving != 0)
        return 0;
    if (param == 0)
        return 0;
    int ok = OpenItemEntry_GetFlagBit15((void*)AsCCameraController(object)->itemEntry); // original +0
    // 0x802F4D24: Join item validation before starting camera movement.
    do {
        if (ok == 0) {
            ok = OpenItemEntry_GetFlagBit15(param);
            if (ok == 0) {
                param = 0;
                break;
            }
        }
        if (OpenItemEntry_GetFlagBit13(param) != 0)
            param = 0;
        else if (OpenItemEntry_GetFlagBit14(param) != 0)
            param = 0;
    } while (0);

    if (param != 0) {
        ModelPreview_SetAnimationByName(AsCCameraController(object)->modelPreview,
                                        ((void**)&tourPreviewAnimationNames)[kMoveAnimation]);
        (*(void**)&AsCCameraController(object)->targetItemEntry) = param;
        AsCCameraController(object)->moving = 1;
        return 1;
    }
    return 0;
}

#pragma opt_lifetimes off
// Turn toward the movement direction using the shortest wrapped angular difference.
// self/preview required. Movement angle uses atan2(deltaX,deltaY) in board XY,
// not atan2(deltaY,deltaX); target is pi minus that angle. No delta-time scaling.
// When moving but both position samples match, returns without submitting a
// heading. Volatile round trips and opt_lifetimes preserve the stored float
// and register lifetime sequence; keep one-period wrapping and snap order.
void CCameraController::UpdateHeading() {
    char* controllerBytes = (char*)this;
    if (AsCCameraController(controllerBytes)->moving != 0) {
        float previousX, currentX, previousY, currentY;
        if (Vec2_Equals(controllerBytes + kControllerPositionOffset,
                        controllerBytes + kControllerPreviousPositionOffset) != 0)
            return;
        currentY = AsCCameraController(controllerBytes)->positionY;
        previousY = AsCCameraController(controllerBytes)->previousPositionY;
        currentX = AsCCameraController(controllerBytes)->positionX;
        previousX = AsCCameraController(controllerBytes)->previousPositionX;
        float movementAngle = (float)atan2(currentX - previousX, currentY - previousY);
        AsCCameraController(controllerBytes)->targetHeading =
            kFloat_3_1415927_CameraController_805E119C - movementAngle;
    }
    // Wrap target to [-pi, pi], then shift current across the seam for shortest travel.
    float headingWork, targetOrDifference, seamAdjustedCurrent, smoothingDivisor;
    headingWork = AsCCameraController(controllerBytes)->targetHeading;
    if (headingWork > kFloat_3_1415927) {
        headingWork = headingWork - kFloat_2_CameraController * kFloat_3_1415927;
    } else {
        float negativePi = -kFloat_3_1415927;
        if (headingWork < negativePi) {
            headingWork = headingWork + kFloat_2_CameraController * kFloat_3_1415927;
        }
    }
    seamAdjustedCurrent = (*(volatile float*)&AsCCameraController(controllerBytes)->currentHeading);
    double targetAsDouble = headingWork;
    targetOrDifference = (float)targetAsDouble;
    (*(volatile float*)&AsCCameraController(controllerBytes)->targetHeading) = headingWork;
    headingWork = targetOrDifference - seamAdjustedCurrent;
    smoothingDivisor = kCameraControllerHeadingSmoothingDivisor;
    if (headingWork > kFloat_3_1415927) {
        if (seamAdjustedCurrent < kFloat_0_805E1198) {
            seamAdjustedCurrent = seamAdjustedCurrent + kFloat_2_CameraController * kFloat_3_1415927;
        }
    } else {
        float neg = -kFloat_3_1415927;
        if (headingWork < neg) {
            if (seamAdjustedCurrent > kFloat_0_805E1198) {
                seamAdjustedCurrent = seamAdjustedCurrent - kFloat_2_CameraController * kFloat_3_1415927;
            }
        }
    }
    // Interpolate by 1/smoothingDivisor per call; snap below pi/snapDivisor.
    targetOrDifference = targetOrDifference - seamAdjustedCurrent;
    headingWork = targetOrDifference / smoothingDivisor;
    float absoluteDifference = (float)__fabs((double)targetOrDifference);
    float snapThreshold = kFloat_3_1415927 / kCameraControllerHeadingSnapDivisor;
    headingWork = seamAdjustedCurrent + headingWork;
    if (absoluteDifference < snapThreshold) {
        AsCCameraController(controllerBytes)->currentHeading = AsCCameraController(controllerBytes)->targetHeading;
    } else {
        if (headingWork > kFloat_3_1415927) {
            headingWork = headingWork - kFloat_2_CameraController * kFloat_3_1415927;
        } else {
            float neg2 = -kFloat_3_1415927;
            if (headingWork < neg2) {
                headingWork = headingWork + kFloat_2_CameraController * kFloat_3_1415927;
            }
        }
        AsCCameraController(controllerBytes)->currentHeading = headingWork;
    }
    ModelPreview_SetHeading(AsCCameraController(controllerBytes)->modelPreview,
                            AsCCameraController(controllerBytes)->currentHeading);
}
#pragma opt_lifetimes reset

struct Tbl5 {
    int v[5];
};
struct Tbl8 {
    int v[8];
};
struct Tbl16 {
    unsigned int v[16];
};

struct CameraRig {
    char storage[0x98];
    // Construct the tour-node model preview with the default mode.
    CameraRig() {
        ModelPreview_Ctor(this, 0);
    }
};

// Choose the tour preview model from node mode and model index.
// self/previewNode required. Mode 0 does nothing; 1/5/6 require modelIndex [0,8),
// while 2/3/4 index the five-entry mode table directly. Other modes are invalid.
// Call only with modelPreview empty: nonzero modes allocate and overwrite it,
// with ownership ending at TourBossPreview_Destroy. No load-success result.
void TourBossPreview_LoadModel(void* self) {
    void* obj = self;
    signed char previewMode = ((TourPreviewNode*)AsTourBoardPreviewState(obj)->previewNode)->previewMode;
    Tbl5 previewModelsByMode = *(Tbl5*)tourPreviewModelsByMode;
    Tbl8 bossPreviewModelsByIndex = *(Tbl8*)tourBossPreviewModelsByIndex;
    if (previewMode != 0) {
        CameraRig* nw = new CameraRig();
        AsTourBoardPreviewState(obj)->modelPreview = nw;
        signed char c = ((TourPreviewNode*)AsTourBoardPreviewState(obj)->previewNode)->previewMode;
        if (c == 1 || (unsigned char)(c - 5) <= 1) {
            short modelIndex = ((TourPreviewNode*)AsTourBoardPreviewState(obj)->previewNode)->modelIndex;
            ModelPreview_LoadModel(AsTourBoardPreviewState(obj)->modelPreview,
                                   (void*)bossPreviewModelsByIndex.v[modelIndex]);
            ModelPreview_SetAnimationByName(AsTourBoardPreviewState(obj)->modelPreview, tourBossPreviewAnimationName);
        } else {
            ModelPreview_LoadModel(nw, (void*)previewModelsByMode.v[c]);
        }
        ModelPreview_SetHeading(AsTourBoardPreviewState(obj)->modelPreview, kFloat_3_1415927_CameraController_805E11D8);
    }
}

// Footstep thresholds are two animation times per model. Strict crossing tests
// omit equality and do not explicitly handle loop wrap; preserve event order.
// Advance travel and animation, trigger footstep sounds, and refresh camera and heading.
// self/preview/mapOrigin(float[2]) required; modelIndex [0,8). deltaTime seconds,
// not board units. No stored pointer to mapOrigin; previousAnimationTime is seconds.
// At most one footstep fires per update, even if both thresholds are crossed.
void CCameraController::Update(float deltaTime, void* mapOrigin) {
    float firstFootstepTime, f1b;
    float secondFootstepTime, f1;
    void* obj = this;
    float f31 = deltaTime;
    ((CCameraController*)obj)->UpdatePosition(mapOrigin);
    Tbl16 footstepAnimationTimes = *(Tbl16*)tourPreviewFootstepAnimationTimes;
    ModelPreview_GetAnimationName(AsCCameraController(obj)->modelPreview);
    ModelPreview_UpdateAnimation(AsCCameraController(obj)->modelPreview, f31);
    int isMoveAnimation = ModelPreview_IsAnimationNamed(AsCCameraController(obj)->modelPreview,
                                                        ((void**)&tourPreviewAnimationNames)[kMoveAnimation]);
    if (isMoveAnimation != 0) {
        unsigned int footstepOffset = AsCCameraController(obj)->modelIndex * 8;
        f1 = AsCCameraController(obj)->previousAnimationTime;
        void* sub = AsCCameraController(obj)->modelPreview;
        secondFootstepTime = *(float*)((char*)&footstepAnimationTimes.v[1] + footstepOffset);
        float animationTime = AsModelPreview802F5898(sub)->animationTime; // 0x802F46AC: compares animation time
        if (f1 < secondFootstepTime && animationTime > secondFootstepTime) {
            Sound_PlayByName(g_soundManager, tourPreviewFootstepSoundNames, 0, 0, 0, 0,
                             kFloat_0_5_TourPreviewFootstepSound, kFloat_200_CameraControllerSound,
                             kFloat_1_CameraControllerSound);
        } else {
            f1b = AsCCameraController(obj)->previousAnimationTime;
            firstFootstepTime = *(float*)((char*)&footstepAnimationTimes.v[0] + footstepOffset);
            if (f1b < firstFootstepTime && animationTime > firstFootstepTime) {
                Sound_PlayByName(g_soundManager, tourPreviewFootstepSoundNames + 0x10, 0, 0, 0, 0,
                                 kFloat_0_5_TourPreviewFootstepSound, kFloat_200_CameraControllerSound,
                                 kFloat_1_CameraControllerSound);
            }
        }
        AsCCameraController(obj)->previousAnimationTime = animationTime;
    }
    ModelPreview_UpdateCamera(AsCCameraController(obj)->modelPreview);
    ModelPreview_SetupRenderParams(AsCCameraController(obj)->modelPreview);
    ((CCameraController*)obj)->UpdateHeading();
}

#pragma opt_lifetimes off
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_strength_reduction off
// screenPoint supplies two viewport coordinates; out has three world coordinates.
// Template Z values select near/far depths. Unprojection uses the current viewport
// and the two engine transforms with an identity object transform. A ray parallel
// to the plane leaves out untouched; callers must supply a nonparallel view.
// Intersect the unprojected near/far screen ray with the requested world-height plane.
void ScreenPoint_UnprojectToHeightPlane(float* out, float height, const float* screenPoint) {
    float y;
    float x;
    char* base;
    int zero;
    float planeDistance;
    float nearDistance;
    float farDistance;
    unsigned int farZBits;
    unsigned int nearXBits;
    unsigned int nearZBits;
    unsigned int farYBits;
    unsigned int farXBits;
    unsigned int nearYBits;
    float rayParameter;

    float nearScreen[3];
    float nearWorld[3];
    float farScreen[3];
    float farWorld[3];
    float planeNormal[3];
    float planePoint[3];
    float viewport[6];
    float projectionMatrix[16];
    float viewMatrix[16];
    float rotationMatrix[16];
    float translationMatrix[16];
    float worldMatrix[16];
    struct Words {
        unsigned int x, y, z;
    };
    register unsigned int nearTemplateAddress;
    register unsigned int farTemplateAddress;
    register unsigned int normalTemplateAddress;

    // Phase 1: copy near/far depth templates; replace only their viewport XY coordinates.
    x = screenPoint[0];
    y = screenPoint[1];
    base = (char*)(int)s_cameraRayTemplatePoolAnchor;
    nearTemplateAddress = (unsigned int)base + kNearScreenTemplate;
    farTemplateAddress = (unsigned int)base + kFarScreenTemplate;
    {
        nearYBits = (*(unsigned int*)((char*)&(::s_cameraRayTemplatePoolAnchor) + (0xE8)));
        nearXBits = (*(unsigned int*)((char*)&(::s_cameraRayTemplatePoolAnchor) + (0xE4)));
        nearZBits = (*(unsigned int*)((char*)&(::s_cameraRayTemplatePoolAnchor) + (0xEC)));
        farYBits = (*(unsigned int*)((char*)&(::s_cameraRayTemplatePoolAnchor) + (0xF4)));
        farXBits = (*(unsigned int*)((char*)&(::s_cameraRayTemplatePoolAnchor) + (0xF0)));
        farZBits = (*(unsigned int*)((char*)&(::s_cameraRayTemplatePoolAnchor) + (0xF8)));
        *(unsigned int*)&nearScreen[0] = nearXBits;
        *(unsigned int*)&nearScreen[1] = nearYBits;
        *(unsigned int*)&nearScreen[2] = nearZBits;
        *(unsigned int*)&farScreen[0] = farXBits;
        *(unsigned int*)&farScreen[1] = farYBits;
        *(unsigned int*)&farScreen[2] = farZBits;
    }
    nearScreen[0] = x;
    nearScreen[1] = y;
    farScreen[0] = x;
    farScreen[1] = y;
    // Phase 2: fetch projection then view (engine states 1,0) and unproject both depths.
    Engine_GetViewport(viewport);
    Engine_GetTransform(kProjectionTransform, projectionMatrix);
    Engine_GetTransform(kViewTransform, viewMatrix);
    Mtx44_SetTranslation(translationMatrix, kFloat_0_805E1198, kFloat_0_805E1198, kFloat_0_805E1198);
    Mtx44_SetRotationY(rotationMatrix, kFloat_0_805E1198);
    PSMTX44Concat(worldMatrix, rotationMatrix, translationMatrix);
    Vec3_UnprojectFromViewport(nearWorld, nearScreen, viewport, projectionMatrix, viewMatrix, worldMatrix);
    Vec3_UnprojectFromViewport(farWorld, farScreen, viewport, projectionMatrix, viewMatrix, worldMatrix);
    // Phase 3: intersect the world ray with Y=height; leave output untouched if parallel.
    zero = 0;
    *(unsigned int*)&planePoint[1] = zero;
    *(unsigned int*)&planePoint[0] = zero;
    *(unsigned int*)&planePoint[2] = zero;
    planePoint[1] = height;
    normalTemplateAddress = (unsigned int)base + kPlaneNormalTemplate;
    *(unsigned int*)&planeNormal[0] = (*(unsigned int*)((char*)&(::s_cameraRayTemplatePoolAnchor) + (0xD8)));
    *(unsigned int*)&planeNormal[1] = (*(unsigned int*)((char*)&(::s_cameraRayTemplatePoolAnchor) + (0xDC)));
    *(unsigned int*)&planeNormal[2] = (*(unsigned int*)((char*)&(::s_cameraRayTemplatePoolAnchor) + (0xE0)));
    // Plane equation n dot p = n dot (0,height,0). Interpolate from far to near:
    // t = (planeDistance - farDistance) / (nearDistance - farDistance).
    planeDistance = Vec3_Dot(planePoint, planeNormal);
    nearDistance = Vec3_Dot(nearWorld, planeNormal);
    farDistance = Vec3_Dot(farWorld, planeNormal);
    if (nearDistance != farDistance) {
        rayParameter = (planeDistance - farDistance) / (nearDistance - farDistance);
        out[0] = rayParameter * (nearWorld[0] - farWorld[0]) + farWorld[0];
        out[1] = rayParameter * (nearWorld[1] - farWorld[1]) + farWorld[1];
        out[2] = rayParameter * (nearWorld[2] - farWorld[2]) + farWorld[2];
    }
}
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_strength_reduction reset

// Fixed step is 4.000000953674316 board units per call, including the arrival call. The
// code clears the target within one step but still takes the full step, so it can
// overshoot. The ray intersects world Y=0, then output Y is overwritten with the
// configured map-plane height. Requires a nonparallel ray (XZ otherwise undefined).
// Step toward the destination item and unproject the preview position onto the map plane.
// self/preview/mapOrigin(float[2]) required and borrowed. No allocation/status;
// mutates item/target/moving and both position samples before model placement.
// XY are viewport coordinates after origin subtraction; world XZ come from the
// ray, and world Y becomes 0.1 (kTourPreviewMapPlaneHeight), not a screen offset.
void CCameraController::UpdatePosition(void* mapOrigin) {
    struct Words {
        unsigned int x, y, z;
    };
    union Triple {
        Words w;
        float f[3];
    };
    struct Locals {
        float viewportPoint[2], travelStep[2], toDestination[2], destinationPosition[2];
        Triple nearScreen;
        float nearWorld[3];
        Triple farScreen;
        float farWorld[3];
        Triple planeNormal, planePoint;
        float worldPosition[3];
        float pad;
        float viewport[6], projectionMatrix[16], viewMatrix[16], rotationMatrix[16], translationMatrix[16],
            worldMatrix[16];
    } frame;
    float* mapOriginXY = (float*)mapOrigin;
    char* templatePool = s_cameraRayTemplatePoolAnchor;
    CCameraController* controller = this;
    float distanceToDestination;
    // Phase 1: advance board-space travel; clear an arrived target before applying the step.
    if (controller->targetItemEntry != 0) {
        OpenItemEntry_GetPosition((void*)controller->targetItemEntry, frame.destinationPosition);
        gtl_Vec2_Subtract(frame.toDestination, frame.destinationPosition, &controller->positionX);
        distanceToDestination = Vec2_Length(frame.toDestination);
        if (distanceToDestination <= kFloat_4_000001_CameraController) {
            controller->moving = 0;
            controller->itemEntry = controller->targetItemEntry;
            controller->targetItemEntry = 0;
        }
        gtl_Vec2_Copy(&controller->previousPositionX, &controller->positionX);
        if (kFloat_0_805E1198 != distanceToDestination) {
            Vec2_Scale(frame.travelStep, frame.toDestination, kFloat_4_000001_CameraController / distanceToDestination);
            gtl_Vec2_Add(&controller->positionX, frame.travelStep, &controller->positionX);
        }
    }
    // Travel is complete for this call; convert board position to a world-space preview.
    gtl_Vec2_Subtract(frame.viewportPoint, &controller->positionX, mapOriginXY);
    frame.nearScreen.w = *(Words*)(((char*)(templatePool + kNearScreenTemplate)));
    frame.nearScreen.f[0] = frame.viewportPoint[0];
    frame.nearScreen.f[1] = frame.viewportPoint[1];
    frame.farScreen.w = *(Words*)(((char*)(templatePool + kFarScreenTemplate)));
    frame.farScreen.f[0] = frame.viewportPoint[0];
    frame.farScreen.f[1] = frame.viewportPoint[1];
    // Phase 2: unproject the near/far depth templates with projection, view, identity world.
    Engine_GetViewport(frame.viewport);
    Engine_GetTransform(kProjectionTransform, frame.projectionMatrix);
    Engine_GetTransform(kViewTransform, frame.viewMatrix);
    Mtx44_SetTranslation(frame.translationMatrix, kFloat_0_805E1198, kFloat_0_805E1198, kFloat_0_805E1198);
    Mtx44_SetRotationY(frame.rotationMatrix, kFloat_0_805E1198);
    PSMTX44Concat(frame.worldMatrix, frame.rotationMatrix, frame.translationMatrix);
    Vec3_UnprojectFromViewport(frame.nearWorld, frame.nearScreen.f, frame.viewport, frame.projectionMatrix,
                               frame.viewMatrix, frame.worldMatrix);
    Vec3_UnprojectFromViewport(frame.farWorld, frame.farScreen.f, frame.viewport, frame.projectionMatrix,
                               frame.viewMatrix, frame.worldMatrix);
    // Phase 3: intersect at world Y=0; then use the configured model height.
    frame.planePoint.w.y = 0;
    frame.planePoint.w.x = 0;
    frame.planePoint.w.z = 0;
    frame.planePoint.f[1] = kFloat_0_805E1198;
    frame.planeNormal.w = *(Words*)(((char*)(templatePool + kPlaneNormalTemplate)));
    float planeDistance = Vec3_Dot(frame.planePoint.f, frame.planeNormal.f);
    float nearDistance = Vec3_Dot(frame.nearWorld, frame.planeNormal.f);
    float farDistance = Vec3_Dot(frame.farWorld, frame.planeNormal.f);
    if (nearDistance != farDistance) {
        float rayParameter = (planeDistance - farDistance) / (nearDistance - farDistance);
        frame.worldPosition[0] = rayParameter * (frame.nearWorld[0] - frame.farWorld[0]) + frame.farWorld[0];
        frame.worldPosition[1] = rayParameter * (frame.nearWorld[1] - frame.farWorld[1]) + frame.farWorld[1];
        frame.worldPosition[2] = rayParameter * (frame.nearWorld[2] - frame.farWorld[2]) + frame.farWorld[2];
    }
    frame.worldPosition[1] = kTourPreviewMapPlaneHeight;
    ModelPreview_SetPosition((void*)controller->modelPreview, frame.worldPosition);
}

extern const double kCameraLineIntersectionEpsilon;
extern const double kDouble_2_CameraController;
extern const double kDouble_1_CameraController;

#pragma opt_lifetimes off
// Output coefficients satisfy normalX*x + normalY*y + lineConstant == 0.
// Returns 0 on success or -1 when squared endpoint distance is below the threshold.
// Return normalized line coefficients, rejecting coincident endpoints.
inline int NormalizeLine(double& lineConstant, double& normalX, double& normalY, const float* lineStart,
                         const float* lineEnd, double minimumLengthSquared) {
    float x, y;
    double dx, dy;
    y = lineEnd[1];
    dy = y - lineStart[1];
    x = lineEnd[0];
    dx = x - lineStart[0];
    double deltaYSquared = pow(dy, kDouble_2_CameraController);
    double deltaXSquared = pow(dx, kDouble_2_CameraController);
    double lengthSquared = deltaXSquared + deltaYSquared;
    if (lengthSquared < minimumLengthSquared)
        return -1;
    double inverseLength = kDouble_1_CameraController / sqrt(lengthSquared);
    normalX = -dy * inverseLength;
    normalY = dx * inverseLength;
    lineConstant = inverseLength * (lineStart[0] * lineEnd[1] - lineEnd[0] * lineStart[1]);
    return 0;
}

#pragma opt_lifetimes on
// All point arguments are borrowed float[2] in the same 2D space; radius uses its
// distance units. This intersects an infinite line, without segment clipping.
// Returns -1 for near-coincident endpoints/no hit, 3 for a degenerate direction (out
// unchanged), 1 for tangent or 2 for two intersections (out written). Equal
// reference distances choose the second intersection.
// Intersect a line with a circle; choose the intersection nearest the reference point.
// All pointers required; outputs may overlap input points because writes follow
// all reads. Radius is squared as FLOAT before the DOUBLE discriminant. Keep
// pow/sqrt and intermediate precisions; epsilon 1e-15 is used for squared line
// length, direction length and discriminant, not a screen-distance tolerance.
int Math2D_IntersectLineCircleNearestReference(float requestedRadius, void* lineStartStorage, void* lineEndStorage,
                                               void* circleCenterStorage, void* referenceStorage, void* outputStorage) {
    float* lineStart = (float*)lineStartStorage;
    float* lineEnd = (float*)lineEndStorage;
    float* circleCenter = (float*)circleCenterStorage;
    float* reference = (float*)referenceStorage;
    float* out = (float*)outputStorage;

    double one, inverseNormalLengthSquared, scaledLineConstant;
    double epsilon;
    float radius;
    double lineBaseX, lineBaseY, directionX, directionY, directionLengthSquared, normalX, normalY;
    double x1, y1;
    epsilon = kCameraLineIntersectionEpsilon;
    radius = requestedRadius;
    if (NormalizeLine(scaledLineConstant, normalX, normalY, lineStart, lineEnd, epsilon) != 0)
        return -1;

    // Find a line base point and unit direction, then solve the circle quadratic.
    inverseNormalLengthSquared = kDouble_1_CameraController / (normalX * normalX + normalY * normalY);
    scaledLineConstant = -scaledLineConstant * inverseNormalLengthSquared;
    lineBaseX = normalX * scaledLineConstant;
    lineBaseY = normalY * scaledLineConstant;
    double inverseNormalLength = sqrt(inverseNormalLengthSquared);
    double negativeNormalX = -normalX;
    directionX = normalY * inverseNormalLength;
    directionY = negativeNormalX * inverseNormalLength;
    double directionXSquared = directionX * directionX;
    double directionYSquared = directionY * directionY;
    directionLengthSquared = directionXSquared + directionYSquared;
    if (directionLengthSquared < epsilon)
        return 3;

    float rawY, t2, rawX;
    double centerOffsetX, centerOffsetY;
    rawX = circleCenter[0];
    rawY = circleCenter[1];
    centerOffsetX = (double)rawX - lineBaseX;
    centerOffsetY = (double)rawY - lineBaseY;
    double cross = directionX * centerOffsetY - directionY * centerOffsetX;
    t2 = radius * radius;
    double discriminant = directionLengthSquared * t2 - cross * cross;
    if (discriminant < -epsilon)
        return -1;

    double centerProjection = directionX * centerOffsetX + directionY * centerOffsetY;
    if (discriminant < epsilon) {
        double invDot = centerProjection / directionLengthSquared;
        double outx = lineBaseX + directionX * invDot;
        double outy = lineBaseY + directionY * invDot;
        out[0] = (float)outx;
        out[1] = (float)outy;
        return 1;
    }

    double sqrtVal = sqrt(discriminant);
    double centerMinusRoot = centerProjection - sqrtVal;
    double centerPlusRoot = centerProjection + sqrtVal;
    double scale = kDouble_1_CameraController / directionLengthSquared;
    double firstParameter = scale * centerMinusRoot;
    double secondParameter = scale * centerPlusRoot;
    y1 = directionY * firstParameter + lineBaseY;
    x1 = directionX * firstParameter + lineBaseX;
    double x2 = directionX * secondParameter + lineBaseX;
    double y2 = directionY * secondParameter + lineBaseY;

    // Both candidates exist; select by squared distance in the same coordinate space.
    double d1y = pow((double)reference[1] - y1, kDouble_2_CameraController);
    double next = pow((double)reference[0] - x1, kDouble_2_CameraController);
    double firstDistanceSquared = next + d1y;
    double d2y = pow((double)reference[1] - y2, kDouble_2_CameraController);
    next = pow((double)reference[0] - x2, kDouble_2_CameraController);
    double secondDistanceSquared = next + d2y;

    if (firstDistanceSquared < secondDistanceSquared) {
        out[0] = (float)x1;
        out[1] = (float)y1;
    } else {
        out[0] = (float)x2;
        out[1] = (float)y2;
    }
    return 2;
}
#pragma opt_lifetimes reset

#pragma opt_common_subs off
#pragma opt_propagation off
// Modes 5/6 offset by (-/+32,-16) board units. Other modes shrink the previous
// displacement by the dead zone, then constrain it using a line/circle intersection.
// referencePosition and mapOrigin are borrowed float[2]. The intersection result
// code is ignored here, so valid constraint geometry is a caller/data precondition.
// Place side previews at fixed offsets or constrain displacement before updating the model.
// self/node/mapOrigin/referencePosition required, but an absent modelPreview
// returns before touching other pointers. deltaTime seconds; all 2D positions
// are board XY until origin subtraction, then viewport XY. Side previews are
// projected onto world Y=0, unlike the controller's final model Y=0.1.
// Preserve displacement history, ignored intersection status and spin-before-
// animation/camera/render-parameter order. Neither map pointer is retained.
void TourModelPreview_UpdatePlacement(void* self, float deltaTime, void* mapOrigin, void* referencePosition) {
    struct Angle {
        // Subtract one period at the positive wrap threshold; caller supplies bounded steps.
        static float wrap(float f1) {
            if (f1 < kFloat_3_1415927_CameraController_805E11D8)
                return f1;
            return f1 - kTourPreviewHeadingWrapPeriod;
        }
    };
    char* preview = (char*)self;
    float frameStep, displacementLength, deadZone, constraintBaseRadius;
    frameStep = deltaTime;
    void* reference = referencePosition;
    void* origin = mapOrigin;
    if ((*(int*)&AsTourBoardPreviewState(preview)->modelPreview) == 0)
        return;
    signed char type = ((TourPreviewNode*)AsTourBoardPreviewState(preview)->previewNode)->previewMode;
    float worldPosition[3];
    float screenPosition[2];
    float retainedDisplacement[2];
    float targetDisplacement[2];
    float limitedDisplacement[2];
    float constraintOrigin[2];
    float constraintLineEnd[2];
    // 0x802F5570: each camera mode computes the shared output position.
    if (type == kTourLeftPreview) {
        float f0, f1, f2, f3;
        gtl_Vec2_Subtract(screenPosition, AsTourBoardPreviewState(preview)->position, origin);
        f3 = screenPosition[0];
        f1 = kFloat_32_TourPointer;
        f2 = screenPosition[1];
        f3 = f3 - f1;
        f0 = kFloat_16_TourMap;
        f1 = kFloat_0_CameraController;
        f0 = f2 - f0;
        screenPosition[0] = f3;
        screenPosition[1] = f0;
        ScreenPoint_UnprojectToHeightPlane(worldPosition, f1, screenPosition);
    } else if (type == kTourRightPreview) {
        float f0, f1, f2, f3;
        gtl_Vec2_Subtract(screenPosition, AsTourBoardPreviewState(preview)->position, origin);
        f3 = screenPosition[0];
        f1 = kFloat_32_TourPointer;
        f2 = screenPosition[1];
        f3 = f3 + f1;
        f0 = kFloat_16_TourMap;
        f1 = kFloat_0_CameraController;
        f0 = f2 - f0;
        screenPosition[0] = f3;
        screenPosition[1] = f0;
        ScreenPoint_UnprojectToHeightPlane(worldPosition, f1, screenPosition);
    } else {
        // Retain motion outside the dead zone; solve the allowed displacement if close.
        displacementLength = Vec2_Length(AsTourBoardPreviewState(preview)->displacement);
        deadZone = kTourPreviewDisplacementDeadZone;
        constraintBaseRadius = kFloat_16_TourMap;
        gtl_Vec2_Subtract(targetDisplacement, reference, AsTourBoardPreviewState(preview)->position);
        if (displacementLength < deadZone) {
            retainedDisplacement[0] = kFloat_0_CameraController;
            retainedDisplacement[1] = kFloat_0_CameraController;
        } else {
            float f0 = displacementLength - deadZone;
            Vec2_Scale(retainedDisplacement, AsTourBoardPreviewState(preview)->displacement, f0 / displacementLength);
        }
        gtl_Vec2_Subtract(limitedDisplacement, targetDisplacement, retainedDisplacement);
        float limitedLength = Vec2_Length(limitedDisplacement);
        float f0v = kTourPreviewConstraintRadiusScale * constraintBaseRadius;
        if (limitedLength >= f0v) {
            gtl_Vec2_Copy(AsTourBoardPreviewState(preview)->displacement, retainedDisplacement);
        } else {
            *(unsigned int*)&constraintOrigin[0] = 0;
            *(unsigned int*)&constraintOrigin[1] = 0;
            *(unsigned int*)&constraintLineEnd[0] = *(unsigned int*)&kTourPreviewConstraintLineEndX;
            *(unsigned int*)&constraintLineEnd[1] = *(unsigned int*)&kTourPreviewConstraintLineEndY;
            float constraintRadius = f0v;
            Math2D_IntersectLineCircleNearestReference(
                constraintRadius, constraintOrigin, constraintLineEnd, targetDisplacement,
                AsTourBoardPreviewState(preview)->displacement, limitedDisplacement);
            gtl_Vec2_Copy(AsTourBoardPreviewState(preview)->displacement, limitedDisplacement);
        }
        gtl_Vec2_Add(limitedDisplacement, AsTourBoardPreviewState(preview)->position,
                     AsTourBoardPreviewState(preview)->displacement);
        gtl_Vec2_Subtract(screenPosition, limitedDisplacement, origin);
        float f1z = kFloat_0_CameraController;
        ScreenPoint_UnprojectToHeightPlane(worldPosition, f1z, screenPosition);
    }
    // Position is now in world space; update per-call spin and the preview animation.
    {
        signed char type2 = ((TourPreviewNode*)AsTourBoardPreviewState(preview)->previewNode)->previewMode;
        if (type2 != kTourCharacterPreview && type2 != kTourLeftPreview && type2 != kTourRightPreview) {
            float f0, f1, f2;
            f2 = kTourPreviewHeadingStep;
            f1 = AsTourBoardPreviewState(preview)->heading;
            f0 = kFloat_3_1415927_CameraController_805E11D8;
            f1 = f2 + f1;
            f1 = Angle::wrap(f1);
            AsTourBoardPreviewState(preview)->heading = f1;
        }
        ModelPreview_SetPosition(AsTourBoardPreviewState(preview)->modelPreview, worldPosition);
        ModelPreview_SetHeading(AsTourBoardPreviewState(preview)->modelPreview,
                                AsTourBoardPreviewState(preview)->heading);
        ModelPreview_UpdateAnimation(AsTourBoardPreviewState(preview)->modelPreview, frameStep);
        ModelPreview_UpdateCamera(AsTourBoardPreviewState(preview)->modelPreview);
        ModelPreview_SetupRenderParams(AsTourBoardPreviewState(preview)->modelPreview);
    }
}
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs reset

// Guard the local views used by this unit against accidental layout changes.
#define POLISH_OFFSET(T, F, O) typedef char polish_##T##_##F[((int)&((T*)0)->F == (O)) ? 1 : -1]
POLISH_OFFSET(TourPreviewNode, previewMode, 0xd);
POLISH_OFFSET(TourPreviewNode, modelIndex, 0xe);
POLISH_OFFSET(TourPreviewAnimationTable, tourAction, 0x10);
#undef POLISH_OFFSET

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462588", _ctors_u802F4320_0, CameraController_InitRotationPeriod);
STATIC_INIT_ENTRY(".CRT$XCU80462588", _ctors_u802F4320_1, TourBossPreview_InitRotationPeriod);
#pragma pop
