// Spinning Dolfini scene (CRoundDol3D): the dolphin and capsule models and their bone poses,
// the scripted camera and its animation ranges, the projected shadows, and the effect box.
// Scene resource loading and the spray-attachment tree cleanup are also in this unit.

#include "classes/GtlIGenericEnvironmentResource.h"
#include <decomp/static_init.h>
// Spinning Dolfini model scene: models, capsule, scripted camera, shadows and effects.
// Internal asset/ABI spellings use dolphin/CRoundDol3D; the US game calls the
// lottery Spinning Dolfini. Shared layouts: CRoundDol3D/CPvsFxBox/WVector.
// Modder lookup:
// - Model/course loading: LoadSceneResources loads the four model paths selected
//   by RoundSceneString from roundDol3DStrings (dolphin, stand, background,
//   capsule); this is the lottery scene, not the golf course loader. All retained
//   model resources are released in the scene destructor through g_resourceManager.
// - Frame rate/model timeline: kRoundAnimationFrameStep in the data section
//   is float bits 0x3d08882f (~0.033333 seconds) added per UpdateAnimationAndCamera.
//   WPuppet_SetAnimationTimeFromRanges (wpuppet_model_bone_animation.cpp) multiplies range-local
// - Camera timeline: CVCGPlayer::AdvancePlayback (vcg_motion_player_and_fade.cpp) separately adds
//   ONE camera-script tick per call; sample occurs before that advance. A model
//   step change alone does not rescale the scripted camera or effect updates.
// - Projection: sampled pose.fov is passed to Camera_SetFov; near 0.1 scene units
//   and kRoundCameraFarClip=20000 come from the scalar pools. Main-camera aspect
//   still comes from kViewPerspectiveAspectRatio in the data section; no resolution is set here.
// - Shadow targets: CShadowManager::CShadowManager in shadow_manager.cpp sets its queue to 256x256
//   pixels. The local 0x100 passed to GetOrCreateEntry is kRoundShadowEntryFlags,
//   an ENTRY FLAG, not texture size. Pair target-size edits with capture projection
//   and queue allocation; do not change the entry flag to resize the capture.
// - Effects: CFxBox::UpdateEffects receives kRoundEffectStepMilliseconds=34 milliseconds.
//   Constructor writes nominalStepMilliseconds=33.333332 at absolute scene+0x280
//   (effectBox+4). The provider (fx_box_interpreter_resources.cpp) optionally clamps delta to 1.5 times
//   this nominal step and multiplies by its own timeScale before ticking effects.
//   The preceding virtual hook updates attachment positions (golf_hud_hole_info_and_fonts.cpp);
//   its retained float ABI argument is ignored. Do not tick effects twice.
// Preserve sample/advance/pose/target order (bone target is used next call), global
// shadow ownership, sound alias lifetime, light reset and attachment-tree cleanup.
#include "game/wvector.h"
#include "classes/CPvsFxBox.h"
#include "classes/CRoundDol3D.h"
#include "classes/CFxBox.h"
#include "classes/CVCGPlayer.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CSharedFilePtr.h"
#include "classes/CShadowManager.h"

// View borrowed scene storage using the checked shared layout.
inline CRoundDol3D* AsCRoundDol3D(void* p) {
    return (CRoundDol3D*)p;
}

// Borrow the embedded effect-box layout; scene-base storage uses separate private views.
inline CPvsFxBox* AsCPvsFxBox(void* p) {
    return (CPvsFxBox*)p;
}

class CRoundDol3DObj {
public:
    virtual void vdtor(int);
};

class CPvsFxBoxObj {
public:
    virtual void v0();
    // Slot 1 prepares attachment positions; its implementation ignores this float.
    // Keep the historical caller signature and the original preceding FP load.
    virtual void v1(float);
};

struct CVec3 {
    float x, y, z;
};
// Affine storage order is right, up, forward, then position (three floats each).
struct CMtx34 {
    float m[12];
};
enum CameraAffineSlot {
    kCameraRight = 0,
    kCameraUp = 3,
    kCameraForward = 6,
    kCameraEye = 9
};
struct RoundCameraBone {
    char hierarchyAndMatrix[0x9C];
    WVector worldPosition;
};
struct RoundAnimationRange {
    float firstSeconds, lastSeconds, handoffSeconds;
    char eventMetadata[0x10];
    void* nextRangeName;
};
typedef char RoundAnimation_next_check[((int)&((RoundAnimationRange*)0)->nextRangeName == 0x1C) ? 1 : -1];
typedef char RoundCameraBone_position_check[((int)&((RoundCameraBone*)0)->worldPosition == 0x9C) ? 1 : -1];
struct CCamPose {
    CVec3 eye, target, up;
    float fov;
};
struct CCamState {
    // CVCGPlayer::GetPlaybackState writes {int group,float time,int stopped,int loop}.
    // Time is not read here: retain the word view instead of pretending it is an index.
    int groupIndex, playbackTimeBits, stopped, loopMode;
};

// Native Wii light-record schema of s_dolphinShadowSetup/s_capsuleShadowSetup
// all components binary32 except type. Table baseline: directional type=3,
// diffuse/specular=(255,255,255,255), ambient=(128,128,128,255), position=0,
// direction=(1,1,1) / (0,1,-.08), all remaining fields zero.
struct RoundShadowLight {
    unsigned int type;                         // +00: Engine_SetLight type 3=directional
    float diffuse[4], specular[4], ambient[4]; // +04/+14/+24, float color lanes
    float worldPosition[3];                    // +34
    float worldDirection[3];                   // +40, negated by GetOrCreateEntry
    float range, reserved50;                   // +4C/+50
    float attenuation[3];                      // +54: constant/linear/quadratic
    float spotStartRadians, spotEndRadians;    // +60/+64
};
// Keep the original two-word integer copy loop: a float-typed assignment changes
// MWCC load/store selection. This union makes the editable field schema visible
// while preserving the 26 bit words copied to CShadowManager entry+0x8C.
union RoundShadowSnapshot {
    RoundShadowLight light;
    unsigned int words[26];
};
typedef char RoundShadowSnapshot_size[sizeof(RoundShadowSnapshot) == 0x68 ? 1 : -1];
typedef char RoundShadowDirection_offset[(int)&((RoundShadowLight*)0)->worldDirection == 0x40 ? 1 : -1];
// The schema agrees with Engine_SetLight/ApplyStateBeforeDraw's light record
// (engine_render_cache_and_texture_load.cpp/gx_renderer_state_and_resources.cpp); that alone does not make every light field a shadow
// tuning input. The actual shadow capture overlay
// (ShadowNamedGroup, shadow_manager_capture.cpp) aliases snapshot+00 as anchor[3] and +40 as light
// direction. Although the capture call supplies snapshot+00 as an extra ABI
// argument, PuppetList_GetRepeatedFirstAnchorPosition (texture_capture_queue_and_mesh.cpp) takes only
// result/list and ignores it: the anchor comes from the first puppet's bone or
// matrix translation, repeatedly accumulated for the list count. Neither the
// type/color prefix nor worldPosition+34 sets that anchor. Capture
// eye=average-8*negatedDirection; width/height=256*entry.projectionScale, whose
// .05/.01 values are separate arguments below, not fields in these snapshots.
// Standard light fields expose the table representation; their effect beyond
// the observed direction consumer is unestablished here. To edit capture use direction,
// projectionScale and queue dimensions together, preserving this overlay ABI.

// Offsets into the packed roundDol3DStrings pool (verified by globals round 4).
// Byte-address form preserves CodeWarrior common-subexpression and loop allocation.
enum RoundSceneLayout {
    kSprayAttachmentTreeOffset = (int)&((CRoundDol3D*)0)->effectBox + (int)&((CPvsFxBox*)0)->sprayAttachmentTreeRoot -
        4,
    kSequenceAttachmentTreeOffset = (int)&((CRoundDol3D*)0)->effectBox +
        (int)&((CPvsFxBox*)0)->sequenceAttachmentTreeRoot - 4,
    kSceneEffectsOffset = (int)&((CRoundDol3D*)0)->effectBox,
    kMotionSamplesOffset = (int)&((CRoundDol3D*)0)->motionSamples
};

enum RoundAnimationIndex {
    kEntrance = 0,
    kDolphinIdle = 1,
    kRotateLow = 2,
    kRotateMiddle = 3,
    kRotateHigh = 4,
    kRotateSuper = 5,
    kRotateOver = 6,
    kCapsuleLow = 7,
    kCapsuleMiddle = 8,
    kCapsuleHigh = 9,
    kCapsuleSuper = 10,
};
enum RoundSceneLimits {
    kSceneLightCount = 8,
    kMotionSampleCount = 32,
    kPlaybackStopped = 1,
    kRoundShadowEntryFlags = 0x100, // creation flags; capture dimensions belong to the manager
};
enum RoundSceneString {
    kDolphinModelPath = 0x14,
    kDolphinStandModelPath = 0x21,
    kBackgroundModelPath = 0x34,
    kCapsuleModelPath = 0x3f,
    kCameraScriptPath = 0x54,
    kLightingPath = 0x67,
    kTurnSoundName = 0x78,
    kDolphinShadowName = 0x89,
    kCeilingLightName = 0x91,
    kCapsuleShadowName = 0x9b,
    kDolphinCameraBone = 0xa3,
    kCapsuleCameraBone = 0xb4,
};

// Scene-base initialization inventory: six binary32 words +1C..+30 are written
// {0,0,10,0,0,0} by the constructor. CRoundDol3D::UpdateAnimationAndCamera uses the separate
// cameraTarget at +9C, rotationAngle at +18 and animationTime at +268; companion
// round_dol_gurudol_minigame.cpp's draw/rotation/target handlers do not name these six words either.
// No downstream interpretation is established in src/ or include/. In particular
// 10 is not proved to be zoom, distance, FOV or animation speed. Keep these words
// opaque until a reader is found; there is no source-backed visible tuning knob.
// This storage precedes effectBox and must never be interpreted as CPvsFxBox.
struct RoundSceneInitialScalars128 {
    char prefix[0x1C];
    float unk1C, unk20, unk24, unk28, unk2C, unk30;
};
// Absolute +0x280 is effectBox+4, the CFxBox nominal update-step clamp input.
// This is a private scene-relative view, not CPvsFxBox's conflicting wrapper layout.
struct RoundSceneScalar280128 {
    char prefix[0x280];
    float nominalStepMilliseconds;
};
struct RoundAttachmentHeader128 {
    int count;
    void* root;
    void* first;
};
struct RoundEffectTreeHeaders128 {
    char prefix[0x37C];
    RoundAttachmentHeader128 spray;
    RoundAttachmentHeader128 sequence;
};
typedef char RoundSceneInitialScalars128_end[(sizeof(RoundSceneInitialScalars128) == 0x34) ? 1 : -1];
typedef char RoundEffectTreeHeaders128_sequence[((int)&((RoundEffectTreeHeaders128*)0)->sequence == 0x388) ? 1 : -1];

extern "C" {

// kFloat_1_805E13D8 is the 1.0f bone-update step in every call below; other global uses are not inferred here.
// kRoundCameraFarClip and kRoundAnimationFrameStep are the camera far plane and per-update animation step.
extern float kFloat_0_1_RoundScene, kRoundCameraFarClip, kRoundAnimationFrameStep, kFloat_1_805E13D8,
    kRoundEffectStepMilliseconds;
extern void* g_view;
void Vec3_CopySourceToDestination(CVec3*, CVec3*);
void Matrix_SetIdentityAffine(void*);
void __mi__FRC7WVectorRC7WVector(CVec3*, CVec3*, CVec3*);
void __as__7WVectorFRC7WVector(void*, void*);
void Normalize__7WVectorFv(void*);
void Vector_CrossProduct(CVec3*, CVec3*, CVec3*);
void WView_SetViewMatrix(void*, void*);
void Camera_SetClip(void*, float, float, int);
void Camera_SetFov(void*, float);
void* WPuppet_FindAnimationRangeByName(void*, void*);
void AnimationEventParser_ProcessFrameRange(void*, void*, float, float, void*, int, int);
void Matrix_SetAxisRotation(void*, float, int);
void WPuppet_SetAnimationTimeFromRanges(void*, int, float, int, int);
void Model_SetMatrixRecursive(void*, void*, void*);
void WPuppet_UpdateBoneWorldMatrices(void*, int, int, float);
int WPuppet_FindOrCacheBone(void*, const char*);
void Matrix_CopyAffine(void*, void*);

extern float kFloat_6_2831855;
extern float s_roundSceneTwoPiStorage;
extern const float kFloat_0_RoundScene;
extern float kFloat_0_5_RoundSound;
extern float kFloat_200_RoundSound;
extern const float kFloat_10_RoundScene;
extern const float kRoundEffectNominalStepMilliseconds;
extern float kFloat_0_05_RoundShadow;
extern float kFloat_0_01_RoundShadow;
extern void* g_shadowManager;
extern void* g_resourceManager;
extern void* g_soundManager;
extern const unsigned char s_dolphinShadowSetup[];
extern const unsigned char s_capsuleShadowSetup[];
extern const char roundDol3DStrings[];
extern const char __vt__11CRoundDol3D[];
extern const char __vt__9CPvsFxBox[];
extern const char s_cameraTransform[];

void Vec3_SetAll(void*, float);
void* __nw__FUl(unsigned int);
void __dt__6CFxBoxFv(void*, int);
extern "C" void* GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(void*, const char*, float, int, const char*,
                                                                         char*);
void* TextureItemQueue_PushFront(void*, void*);
void CShadowManager_SetSelectionValue(void*, float, const char*, void*);

void* ResourceManager_LoadByExtension(void*, const char*, int, int, int);
void* __nw__FUl(unsigned int);
void CameraMotionGroupVector_Clear(void*);
int CameraScript_LoadFileByPath(void*, const char*);
void* SharedFile_Create(const char*, int);
void* CSharedFilePtr_GetLoadedBuffer(void*);
void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);
void ResourceManager_ReleaseObject(void*, void*, int);
void Sound_StopByAlias(void*, const char*);
void Engine_SetLight(int, void*);
void Engine_SetLightEnabled(int, int);
void FxBoxResourceTree_FreeSubtreeNodes(void*, void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);

// Initialize the round-scene angle constant at startup.
void CRoundDol3D_InitTwoPiConstant() {
    s_roundSceneTwoPiStorage = kFloat_6_2831855;
}

// Pose the dolphin model and publish its world bones; scene/model storage is borrowed.
static inline void PoseRoundDolphin128(char* o, CMtx34* matrix) {
    WPuppet_SetAnimationTimeFromRanges(AsCRoundDol3D(o)->dolphinModel, 0, AsCRoundDol3D(o)->animationTime, 0, 0);
    Model_SetMatrixRecursive(AsCRoundDol3D(o)->dolphinModel, AsCRoundDol3D(o)->dolphinModel, matrix);
    WPuppet_UpdateBoneWorldMatrices(AsCRoundDol3D(o)->dolphinModel, 0, 0, kFloat_1_805E13D8);
}

// Pose the capsule model and publish its world bones; scene/model storage is borrowed.
static inline void PoseRoundCapsule128(char* o, CMtx34* matrix) {
    WPuppet_SetAnimationTimeFromRanges(AsCRoundDol3D(o)->capsuleModel, 0, AsCRoundDol3D(o)->animationTime, 0, 0);
    Model_SetMatrixRecursive(AsCRoundDol3D(o)->capsuleModel, AsCRoundDol3D(o)->capsuleModel, matrix);
    WPuppet_UpdateBoneWorldMatrices(AsCRoundDol3D(o)->capsuleModel, 0, 0, kFloat_1_805E13D8);
}

// Pose dolphin/active models in their interleaved order; return a borrowed bone ADDRESS in an int ABI word.
static inline int PoseRoundRotationModels128(char* o, CMtx34* matrix) {
    WPuppet_SetAnimationTimeFromRanges(AsCRoundDol3D(o)->dolphinModel, 0, AsCRoundDol3D(o)->animationTime, 0, 0);
    Model_SetMatrixRecursive(AsCRoundDol3D(o)->dolphinModel, AsCRoundDol3D(o)->dolphinModel, matrix);
    WPuppet_SetAnimationTimeFromRanges(((void*)AsCRoundDol3D(o)->activeModel), 0, AsCRoundDol3D(o)->animationTime, 0,
                                       0);
    Model_SetMatrixRecursive(((void*)AsCRoundDol3D(o)->activeModel), ((void*)AsCRoundDol3D(o)->activeModel), matrix);
    WPuppet_UpdateBoneWorldMatrices(((void*)AsCRoundDol3D(o)->activeModel), 0, 0, kFloat_1_805E13D8);
    WPuppet_UpdateBoneWorldMatrices(AsCRoundDol3D(o)->dolphinModel, 0, 0, kFloat_1_805E13D8);
    return WPuppet_FindOrCacheBone(((void*)AsCRoundDol3D(o)->activeModel), roundDol3DStrings + kDolphinCameraBone);
}

// Pose the active event model and publish world bones before the caller samples its camera target.
static inline void PoseRoundActiveModel128(char* o, CMtx34* matrix) {
    WPuppet_SetAnimationTimeFromRanges(((void*)AsCRoundDol3D(o)->activeModel), 0, AsCRoundDol3D(o)->animationTime, 0,
                                       0);
    Model_SetMatrixRecursive(((void*)AsCRoundDol3D(o)->activeModel), ((void*)AsCRoundDol3D(o)->activeModel), matrix);
    WPuppet_UpdateBoneWorldMatrices(((void*)AsCRoundDol3D(o)->activeModel), 0, 0, kFloat_1_805E13D8);
}

// Advance the model/camera scene and publish the next bone target; preserve all five phase boundaries.
// self is required, borrowed, with loaded models/player and animationIndex in
// [0,10]. Advancing requires a valid animation range AND named successor; lookup
// failure is unchecked before dereference. State zero pauses model time except
// rotation indices [2,6]; the camera player advances independently when not stopped.
// Returns 1, mutates scene time/poses and global g_view/s_cameraTransform/effects.
// Model times are seconds, camera time is script ticks, rotation is radians and
// eye/target/bones are world-space scene distances. No framerate delta is supplied.
int CRoundDol3D::UpdateAnimationAndCamera() {
    char* o = (char*)this;
    CCamPose pose;
    CMtx34 rotationMatrix;
    CMtx34 capsuleLowRotation, capsuleMiddleRotation, capsuleHighRotation, capsuleSuperRotation;
    CVec3 eyePosition, targetPosition;
    CCamState state;
    CVec3 forwardAxis, rightAxis, upAxis;
    CMtx34* cameraMatrix;
    int cameraBoneAddress = 0;

    // Phase 1: sample this tick's world-space eye/target/up before advancing playback.
    ((CVCGPlayer*)(AsCRoundDol3D(o)->cameraMotionPlayer))->SampleCurrentMotion(&pose);
    if (AsCRoundDol3D(o)->cameraTargetOverrideEnabled) {
        pose.target.x = AsCRoundDol3D(o)->cameraTarget.x;
        pose.target.y = AsCRoundDol3D(o)->cameraTarget.y;
        pose.target.z = AsCRoundDol3D(o)->cameraTarget.z;
    }
    Vec3_CopySourceToDestination(&pose.eye, &eyePosition);
    Vec3_CopySourceToDestination(&pose.target, &targetPosition);
    // Phase 2: build camera-to-world axes: F=normalize(target-eye),
    // R=normalize(up x F), U=normalize(F x R), translation=eye.
    // The WVector aliases address contiguous triples, each exactly 12 bytes.
    Matrix_SetIdentityAffine((void*)s_cameraTransform);
    __mi__FRC7WVectorRC7WVector(&forwardAxis, &targetPosition, &eyePosition);
    cameraMatrix = (CMtx34*)s_cameraTransform;
    *(WVector*)(&cameraMatrix->m[kCameraForward]) = *(WVector*)&forwardAxis;
    (*(WVector*)(&cameraMatrix->m[kCameraForward])).Normalize();
    Vec3_CopySourceToDestination(&pose.up, (CVec3*)&cameraMatrix->m[kCameraUp]);
    Vector_CrossProduct(&rightAxis, (CVec3*)&cameraMatrix->m[kCameraUp], (CVec3*)&cameraMatrix->m[kCameraForward]);
    *(WVector*)(&cameraMatrix->m[kCameraRight]) = *(WVector*)&rightAxis;
    (*(WVector*)(&cameraMatrix->m[kCameraRight])).Normalize();
    Vector_CrossProduct(&upAxis, (CVec3*)&cameraMatrix->m[kCameraForward], (CVec3*)&cameraMatrix->m[kCameraRight]);
    *(WVector*)(&cameraMatrix->m[kCameraUp]) = *(WVector*)&upAxis;
    (*(WVector*)(&cameraMatrix->m[kCameraUp])).Normalize();
    *(WVector*)(&cameraMatrix->m[kCameraEye]) = *(WVector*)&eyePosition;
    Camera_SetClip(g_view, kFloat_0_1_RoundScene, kRoundCameraFarClip, 0);
    WView_SetViewMatrix(g_view, cameraMatrix);
    Camera_SetFov(g_view, pose.fov);
    // Phase 3: retain the pre-advance completion state, then advance model time.
    // animationTime is the current time, converted to frames by the puppet backend. Range changes reset previous/current together so event
    // processing below cannot span the gap between unrelated ranges.
    ((CVCGPlayer*)(AsCRoundDol3D(o)->cameraMotionPlayer))->GetPlaybackState(&state);
    ((CVCGPlayer*)(AsCRoundDol3D(o)->cameraMotionPlayer))->AdvancePlayback();
    if (!(AsCRoundDol3D(o)->state == 0 &&
          (unsigned)(AsCRoundDol3D(o)->animationIndex - kRotateLow) > kRotateOver - kRotateLow)) {
        float currentAnimationSeconds = AsCRoundDol3D(o)->animationTime;
        AsCRoundDol3D(o)->previousAnimationTime = currentAnimationSeconds;
        if (((RoundAnimationRange*)AsCRoundDol3D(o)->animation)->lastSeconds !=
            ((RoundAnimationRange*)AsCRoundDol3D(o)->animation)->firstSeconds) {
            AsCRoundDol3D(o)->animationTime = currentAnimationSeconds + kRoundAnimationFrameStep;
        }
        if (AsCRoundDol3D(o)->animationTime >= AsCRoundDol3D(o)->animationEnd) {
            AsCRoundDol3D(o)->animation = (unsigned int)WPuppet_FindAnimationRangeByName(
                ((void*)AsCRoundDol3D(o)->activeModel),
                ((RoundAnimationRange*)AsCRoundDol3D(o)->animation)->nextRangeName);
            RoundAnimationRange* animationRange = (RoundAnimationRange*)AsCRoundDol3D(o)->animation;
            AsCRoundDol3D(o)->animationTime = animationRange->firstSeconds;
            AsCRoundDol3D(o)->previousAnimationTime = animationRange->firstSeconds;
            AsCRoundDol3D(o)->animationEnd = animationRange->handoffSeconds;
        }
        if (state.stopped == kPlaybackStopped)
            AsCRoundDol3D(o)->state = 0;
        void* anim = ((void*)AsCRoundDol3D(o)->activeModel);
        float currentSeconds = AsCRoundDol3D(o)->animationTime;
        float previousSeconds = AsCRoundDol3D(o)->previousAnimationTime;
        void* animationRangeForEvents = ((void*)AsCRoundDol3D(o)->animation);
        AnimationEventParser_ProcessFrameRange(o, anim ? anim : 0, previousSeconds, currentSeconds,
                                               animationRangeForEvents, -1, 1);
    }
    // Phase 4: apply model-specific animation/matrices before querying world bones.
    // Rotation variants pose dolphin + active model; idle also poses the stand.
    // Capsule variants use zero rotation. Helpers retain the original interleaving
    // of time/matrix/bone calls and reload borrowed scene fields across callbacks.
    Matrix_SetAxisRotation(&rotationMatrix, AsCRoundDol3D(o)->rotationAngle, 0);
    switch (AsCRoundDol3D(o)->animationIndex) {
    case kEntrance:
        PoseRoundDolphin128(o, &rotationMatrix);
        break;
    case kDolphinIdle:
        WPuppet_SetAnimationTimeFromRanges(AsCRoundDol3D(o)->dolphinModel, 0, AsCRoundDol3D(o)->animationTime, 0, 0);
        WPuppet_SetAnimationTimeFromRanges(AsCRoundDol3D(o)->dolphinStandModel, 0, AsCRoundDol3D(o)->animationTime, 0,
                                           0);
        Model_SetMatrixRecursive(AsCRoundDol3D(o)->dolphinModel, AsCRoundDol3D(o)->dolphinModel, &rotationMatrix);
        Model_SetMatrixRecursive(AsCRoundDol3D(o)->dolphinStandModel, AsCRoundDol3D(o)->dolphinStandModel,
                                 &rotationMatrix);
        PoseRoundActiveModel128(o, &rotationMatrix);
        WPuppet_UpdateBoneWorldMatrices(AsCRoundDol3D(o)->dolphinModel, 0, 0, kFloat_1_805E13D8);
        WPuppet_UpdateBoneWorldMatrices(AsCRoundDol3D(o)->dolphinStandModel, 0, 0, kFloat_1_805E13D8);
        cameraBoneAddress =
            WPuppet_FindOrCacheBone(((void*)AsCRoundDol3D(o)->activeModel), roundDol3DStrings + kDolphinCameraBone);
        break;
    case kRotateLow:
        cameraBoneAddress = PoseRoundRotationModels128(o, &rotationMatrix);
        break;
    case kRotateMiddle:
        cameraBoneAddress = PoseRoundRotationModels128(o, &rotationMatrix);
        break;
    case kRotateHigh:
        cameraBoneAddress = PoseRoundRotationModels128(o, &rotationMatrix);
        break;
    case kRotateSuper:
        cameraBoneAddress = PoseRoundRotationModels128(o, &rotationMatrix);
        break;
    case kRotateOver:
        cameraBoneAddress = PoseRoundRotationModels128(o, &rotationMatrix);
        break;
    case kCapsuleLow:
        Matrix_SetAxisRotation(&capsuleLowRotation, kFloat_0_RoundScene, 0);
        Matrix_CopyAffine(&rotationMatrix, &capsuleLowRotation);
        PoseRoundCapsule128(o, &rotationMatrix);
        PoseRoundActiveModel128(o, &rotationMatrix);
        cameraBoneAddress =
            WPuppet_FindOrCacheBone(((void*)AsCRoundDol3D(o)->activeModel), roundDol3DStrings + kCapsuleCameraBone);
        break;
    case kCapsuleMiddle:
        Matrix_SetAxisRotation(&capsuleMiddleRotation, kFloat_0_RoundScene, 0);
        Matrix_CopyAffine(&rotationMatrix, &capsuleMiddleRotation);
        PoseRoundCapsule128(o, &rotationMatrix);
        PoseRoundActiveModel128(o, &rotationMatrix);
        cameraBoneAddress =
            WPuppet_FindOrCacheBone(((void*)AsCRoundDol3D(o)->activeModel), roundDol3DStrings + kCapsuleCameraBone);
        break;
    case kCapsuleHigh:
        Matrix_SetAxisRotation(&capsuleHighRotation, kFloat_0_RoundScene, 0);
        Matrix_CopyAffine(&rotationMatrix, &capsuleHighRotation);
        PoseRoundCapsule128(o, &rotationMatrix);
        // Retain this first lookup before posing: the original cache/call order is observable.
        WPuppet_FindOrCacheBone(((void*)AsCRoundDol3D(o)->activeModel), roundDol3DStrings + kCapsuleCameraBone);
        PoseRoundActiveModel128(o, &rotationMatrix);
        cameraBoneAddress =
            WPuppet_FindOrCacheBone(((void*)AsCRoundDol3D(o)->activeModel), roundDol3DStrings + kCapsuleCameraBone);
        break;
    case kCapsuleSuper:
        Matrix_SetAxisRotation(&capsuleSuperRotation, kFloat_0_RoundScene, 0);
        Matrix_CopyAffine(&rotationMatrix, &capsuleSuperRotation);
        PoseRoundCapsule128(o, &rotationMatrix);
        PoseRoundActiveModel128(o, &rotationMatrix);
        cameraBoneAddress =
            WPuppet_FindOrCacheBone(((void*)AsCRoundDol3D(o)->activeModel), roundDol3DStrings + kCapsuleCameraBone);
        break;
    }
    {
        ((CPvsFxBoxObj*)((o + kSceneEffectsOffset)))->v1(kRoundEffectStepMilliseconds);
    }
    // Phase 5: advance attached effects, then save the selected bone's world target.
    ((CFxBox*)((o + kSceneEffectsOffset)))->UpdateEffects(kRoundEffectStepMilliseconds);
    if (((void*)AsCRoundDol3D(o)->activeModel) && cameraBoneAddress)
        AsCRoundDol3D(o)->cameraTarget = ((RoundCameraBone*)cameraBoneAddress)->worldPosition;
    return 1;
}

#pragma opt_unroll_loops off
// Create/select dolphin/capsule shadow entries and attach models; preserve copied settings and manager ownership.
// arg and models are required, borrowed. Settings are two 0x68-byte snapshots;
// the provider copies them and reverses light direction. Its ABI receives the
// trailing float in f1 as entry selection value, 0x100 in an integer flag word,
// then the secondary-key pointer and settings copy. The local call declaration
// retains that historical source order. Entries/textures remain manager-owned;
// no null failure is handled before appending models. Capture size is not set here.
// Admission: call only after LoadSceneResources has successfully produced live
// dolphin, stand, capsule AND background puppets, with initialized global shadow
// manager/resource cache. The loaded resources are borrowed, not transferred:
// PushFront constructs a binding retaining the puppet pointer and cached root/
// child shadow-model pointers (ShadowModel_BindPuppetAndChildren, shadow_manager_capture.cpp).
// These referents must survive registration and all shadow draws; entries/binding
// vectors are manager-owned. Scene dtor releases its resource references and then
// destroys the manager; do not start a shadow draw during that teardown interval.
// GetOrCreateEntry copies the two stack settings records and light key, reuses
// existing entries, or links a new entry BEFORE allocating its capture texture
// (shadow_manager.cpp). Failed texture creation leaves that entry linked; this function
// still appends bindings. Null model input is dereferenced by the binding provider;
// it is not an empty shadow. Allocation/cache failures are neither reported nor
// rolled back here. Cached shadow-model lookup may yield null entries (the draw
// provider skips null models), but that does not make a null source puppet safe.
// ReplaceSelection first frees the old background selection; its null allocation
// result is published and then dereferenced by SetSelectionValue. There is no
// all-or-nothing registration status; callers must not treat return as success.
void CRoundDol3D::SetupModelShadows() {
    RoundShadowSnapshot dolphinSettings;
    RoundShadowSnapshot capsuleSettings;
    {
        unsigned int* d = dolphinSettings.words - 1;
        const unsigned int* s = (const unsigned int*)s_dolphinShadowSetup - 1;
        for (int i = 0; i < 13; ++i) {
            d[1] = s[1];
            *(d += 2) = *(s += 2);
        }
    }
    {
        const unsigned int* s = (const unsigned int*)s_capsuleShadowSetup - 1;
        unsigned int* d = capsuleSettings.words - 1;
        for (int i = 0; i < 13; ++i) {
            d[1] = s[1];
            *(d += 2) = *(s += 2);
        }
    }
    void* obj = ((void* (*)(void*, const char*, int, const char*, char*,
                            float))GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF)(
        g_shadowManager, roundDol3DStrings + kDolphinShadowName, kRoundShadowEntryFlags,
        roundDol3DStrings + kCeilingLightName, (char*)dolphinSettings.words, kFloat_0_05_RoundShadow);
    TextureItemQueue_PushFront(obj, dolphinModel);
    TextureItemQueue_PushFront(obj, dolphinStandModel);
    obj = ((void* (*)(void*, const char*, int, const char*, char*,
                      float))GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF)(
        g_shadowManager, roundDol3DStrings + kCapsuleShadowName, kRoundShadowEntryFlags,
        roundDol3DStrings + kCeilingLightName, (char*)capsuleSettings.words, kFloat_0_01_RoundShadow);
    TextureItemQueue_PushFront(obj, capsuleModel);
    ((CShadowManager*)g_shadowManager)->ReplaceSelection(roundDol3DStrings + 0xA0);
    ((void (*)(void*, const char*, void*, float))CShadowManager_SetSelectionValue)(
        g_shadowManager, roundDol3DStrings + 0xA0, backgroundModel, kFloat_0_RoundScene);
}
#pragma opt_unroll_loops reset

class CShadowMgrObj {
public:
    // Construct shadow storage using the borrowed resource manager and original mode.
    CShadowMgrObj(void* resourceManager, int b) {
        ((CShadowManager*)this)->__ct(resourceManager, b);
    }
    char unk[0x30];
};

// Initialize scene models, camera samples, rotation state, and the embedded effect box.
CRoundDol3D::CRoundDol3D() {
    char* member = effectBox;
    char* obj = (char*)this;
    vtbl = (void*)__vt__11CRoundDol3D;
    dolphinModel = 0;
    rotationAngle = kFloat_0_RoundScene;
    ((CFxBox*)member)->__ct();
    *(const char**)member = __vt__9CPvsFxBox;
    // Each empty attachment tree caches &root as its first-node sentinel.
    ((RoundEffectTreeHeaders128*)member)->spray.count = 0;
    ((RoundEffectTreeHeaders128*)member)->spray.root = 0;
    ((RoundEffectTreeHeaders128*)member)->spray.first = member + (int)&((RoundEffectTreeHeaders128*)0)->spray.root;
    ((RoundEffectTreeHeaders128*)member)->sequence.count = 0;
    ((RoundEffectTreeHeaders128*)member)->sequence.root = 0;
    ((RoundEffectTreeHeaders128*)member)->sequence.first =
        member + (int)&((RoundEffectTreeHeaders128*)0)->sequence.root;
    sharedFile = 0;
    // The six floats at +0x1C..+0x30 start as {0,0,10,0,0,0}. They belong to the
    // scene itself, not to the effect box, and nothing in the source reads them yet.
    ((RoundSceneInitialScalars128*)obj)->unk1C = kFloat_0_RoundScene;
    ((RoundSceneInitialScalars128*)obj)->unk20 = kFloat_0_RoundScene;
    ((RoundSceneInitialScalars128*)obj)->unk24 = kFloat_10_RoundScene;
    ((RoundSceneInitialScalars128*)obj)->unk28 = kFloat_0_RoundScene;
    ((RoundSceneInitialScalars128*)obj)->unk2C = kFloat_0_RoundScene;
    ((RoundSceneInitialScalars128*)obj)->unk30 = kFloat_0_RoundScene;
    *(float*)pad_0AC = kFloat_0_RoundScene;
    unk_B4 = 0;
    rotationSpeed = kFloat_0_RoundScene;
    speedLevel = 0;
    cameraMotionPlayer = 0;
    motionSampleCount = 0;
    Vec3_SetAll(previousFirstMotionSample, kFloat_0_RoundScene);
    int off;
    int n;
    n = 0;
    off = 0;
    do {
        Vec3_SetAll(motionSamples + off, kFloat_0_RoundScene);
        n += 1;
        off += 12;
    } while (n < kMotionSampleCount);
    state = 0;
    animationTime = kFloat_0_RoundScene;
    previousAnimationTime = kFloat_0_RoundScene;
    animationIndex = 0;
    capsuleModel = 0;
    dolphinModel = 0;
    dolphinStandModel = 0;
    backgroundModel = 0;
    activeModel = 0;
    cameraTargetOverrideEnabled = 0;
    animation = 0;
    new CShadowMgrObj(g_resourceManager, 1);
    ((CFxBox*)effectBox)->SetResourceManager(g_resourceManager);
    ((CFxBox*)effectBox)->SetCameraTransform(s_cameraTransform);
    ((RoundSceneScalar280128*)obj)->nominalStepMilliseconds = kRoundEffectNominalStepMilliseconds;
}

class CRefCountBase {
public:
    int refs;
};

class CRefObj : public CRefCountBase {
public:
    virtual void release(int);
    virtual void v0();
    virtual void v1();
};

class CVCGPlayerObj {
public:
    // Construct the owned camera-motion player with its original initializer.
    CVCGPlayerObj() {
        ((CVCGPlayer*)this)->__ct();
    }
    char unk[0x30];
};

// Retain four scene models, load camera/lighting and start the turn sound; preserve load/apply/release order.
// self is constructed and borrows global resource/sound managers. Model/player
// pointers persist to dtor; lighting handle and temporary environment reference
// are released after application. Requires successful creation/loading: the
// lighting-buffer poll busy-waits without timeout, status failures are unchecked.
// Publication/failure sequence (this is initialization, not a reload transaction):
// 1. Each model load result is assigned directly to its scene member, in path
//    order. A later null/throw leaves earlier results published. Old models are
//    not released before replacement, so do not repeatedly call this to reload.
// 2. Camera-player allocation is published next, before clear/script-load/start.
//    Script status is ignored; there is no "scene ready" flag committed at end.
// 3. An old sharedFile is destroyed/cleared before its replacement is stored.
//    A missing loaded buffer spins forever; a null handle is no checked fallback.
// 4. Environment creation status is ignored; its output is applied/decremented
//    unconditionally. On that failure there is no safe guaranteed return. No
//    local guard releases the environment or file if an intervening call throws.
// 5. Only the ordinary success path plays the turn sound and clears sharedFile.
// Dtor releases all four model member references, stops the alias, deletes the
// published camera player and global shadow manager, disables lights, releases
// a surviving sharedFile, then destroys attachment trees/effect box. That cleanup
// requires a constructed scene and the providers' normal pointer contracts;
// this function does not promise recovery from arbitrary partial provider failure.
// The owner in dolfini_round_main.cpp must finish loading before update/draw/shadow registration.
void CRoundDol3D::LoadSceneResources() {
    char* obj = (char*)this;
    void* r31base = (void*)roundDol3DStrings;
    AsCRoundDol3D(obj)->dolphinModel =
        ResourceManager_LoadByExtension(g_resourceManager, (const char*)r31base + kDolphinModelPath, 0, 1, 0);
    AsCRoundDol3D(obj)->dolphinStandModel =
        ResourceManager_LoadByExtension(g_resourceManager, (const char*)r31base + kDolphinStandModelPath, 0, 1, 0);
    AsCRoundDol3D(obj)->backgroundModel =
        ResourceManager_LoadByExtension(g_resourceManager, (const char*)r31base + kBackgroundModelPath, 0, 1, 0);
    AsCRoundDol3D(obj)->capsuleModel =
        ResourceManager_LoadByExtension(g_resourceManager, (const char*)r31base + kCapsuleModelPath, 0, 1, 0);
    void* r31 = new CVCGPlayerObj;
    AsCRoundDol3D(obj)->cameraMotionPlayer = r31;
    CameraMotionGroupVector_Clear(r31);
    CameraScript_LoadFileByPath(AsCRoundDol3D(obj)->cameraMotionPlayer,
                                (const char*)roundDol3DStrings + kCameraScriptPath);
    ((CVCGPlayer*)(AsCRoundDol3D(obj)->cameraMotionPlayer))->StartGroupPlayback(0, 0);
    void** lightingFile = &AsCRoundDol3D(obj)->sharedFile;
    if (*lightingFile) {
        CSharedFileHandle::Destroy(*lightingFile, 1);
        *lightingFile = 0;
    }
    *lightingFile = SharedFile_Create((const char*)roundDol3DStrings + kLightingPath, 0);
    while (CSharedFilePtr_GetLoadedBuffer(*lightingFile) == 0) {
    }
    int lightingBytes = CSharedFilePtr::GetSize(*lightingFile);
    void* lightingData = (void*)CSharedFilePtr_GetLoadedBuffer(*lightingFile);
    void* environmentResource;
    gtl::IGenericEnvironmentResource::CreateFromMemory(lightingData, lightingBytes, &environmentResource);
    ((gtl::IGenericEnvironmentResource*)environmentResource)->Apply(0);
    {
        void* q = environmentResource;
        if (--*(int*)q == 0 && q != 0) {
            ((CRefObj*)q)->release(1);
        }
    }
    Sound_PlayByName(g_soundManager, (const char*)roundDol3DStrings + kTurnSoundName, 0, 1,
                     (const char*)roundDol3DStrings, 0, kFloat_0_5_RoundSound, kFloat_200_RoundSound,
                     kFloat_0_RoundScene);
    if (*lightingFile) {
        CSharedFileHandle::Destroy(*lightingFile, 1);
        *lightingFile = 0;
    }
}

// Release 1 unrolled tree levels before delegating deeper subtrees to the shared free routine.
static void tree_free_1(void* owner, void* node) {
    if (!node)
        return;
    void* firstChild = *(void**)node;
    if (firstChild)
        FxBoxResourceTree_FreeSubtreeNodes(owner, firstChild);
    void* secondChild = *((void**)node + 1);
    if (secondChild)
        FxBoxResourceTree_FreeSubtreeNodes(owner, secondChild);
    appFreeUni__FPv(node);
}

// Release 2 unrolled tree levels before delegating deeper subtrees to the shared free routine.
static void tree_free_2(void* owner, void* node) {
    if (!node)
        return;
    void* left;
    left = *(void**)node;
    tree_free_1(owner, left);
    tree_free_1(owner, *((void**)node + 1));
    appFreeUni__FPv(node);
}

// Release 3 unrolled tree levels before delegating deeper subtrees to the shared free routine.
static void tree_free_3(void* owner, void* node) {
    if (!node)
        return;
    void* left;
    left = *(void**)node;
    tree_free_2(owner, left);
    tree_free_2(owner, *((void**)node + 1));
    appFreeUni__FPv(node);
}

// Release 4 unrolled tree levels before delegating deeper subtrees to the shared free routine.
static void tree_free_4(void* owner, void* node) {
    if (!node)
        return;
    void* left;
    left = *(void**)node;
    tree_free_3(owner, left);
    tree_free_3(owner, *((void**)node + 1));
    appFreeUni__FPv(node);
}

#pragma use_lmw_stmw on
// Free one tree level after its children; owner is borrowed, n is consumed.
inline void FREE_LEAF(void* owner, void* n) {
    if (n) {
        if (*(void**)n)
            FxBoxResourceTree_FreeSubtreeNodes(owner, *(void**)n);
        if (*((void**)n + 1))
            FxBoxResourceTree_FreeSubtreeNodes(owner, *((void**)n + 1));
        appFreeUni__FPv(n);
    }
}
// Free two unrolled levels using the caller's scratch pointer for the child.
inline void FREE_PAIR(void* owner, void* n, void*& child) {
    if (n) {
        child = *(void**)n;
        FREE_LEAF(owner, child);
        child = *((void**)n + 1);
        FREE_LEAF(owner, child);
        appFreeUni__FPv(n);
    }
}
// Free three unrolled levels, preserving caller scratch lifetimes and postorder.
inline void FREE_QUAD(void* owner, void* n, void*& child, void*& leaf) {
    if (n) {
        child = *(void**)n;
        FREE_PAIR(owner, child, leaf);
        child = *((void**)n + 1);
        FREE_PAIR(owner, child, leaf);
        appFreeUni__FPv(n);
    }
}
// Release scene resources, camera playback, shadows, lights, and effect attachment trees.
// The repeated 'if (obj + kXxxAttachmentTreeOffset)' tests are always true (pointer
// arithmetic is never null); they and the FREE_PAIR/FREE_QUAD sequences reproduce the
// original unrolled code. In plain terms they free every node of the sequence-attachment
// tree, then every node of the spray-attachment tree, then destroy the CFxBox.
CRoundDol3D::~CRoundDol3D() {
    // Phase 1: release model resources, playback, shadow ownership and lights.
    void* n26;
    void* n27;
    void* n28;
    void* n29;
    char* obj = (char*)this;
    vtbl = (void*)__vt__11CRoundDol3D;
    ResourceManager_ReleaseObject(g_resourceManager, dolphinModel, 0);
    ResourceManager_ReleaseObject(g_resourceManager, dolphinStandModel, 0);
    ResourceManager_ReleaseObject(g_resourceManager, backgroundModel, 0);
    ResourceManager_ReleaseObject(g_resourceManager, capsuleModel, 0);
    Sound_StopByAlias(g_soundManager, roundDol3DStrings);
    {
        void* p = cameraMotionPlayer;
        if (p) {
            if (p)
                ((CRoundDol3DObj*)p)->vdtor(1);
            cameraMotionPlayer = 0;
        }
    }
    {
        void* p = g_shadowManager;
        if (p) {
            if (p)
                ((CRoundDol3DObj*)p)->vdtor(1);
        }
    }
    for (int i = 0; i < kSceneLightCount; ++i) {
        Engine_SetLight(i, obj + 0x34);
        Engine_SetLightEnabled(i, 0);
    }
    {
        void* p = sharedFile;
        if (p) {
            CSharedFileHandle::Destroy(p, 1);
            sharedFile = 0;
        }
    }
    // Phase 2: retire sequence/spray attachment nodes before destroying the effect box.
    if ((obj + kSceneEffectsOffset)) {
        if (obj + kSequenceAttachmentTreeOffset) {
            if (obj + kSequenceAttachmentTreeOffset) {
                if (obj + kSequenceAttachmentTreeOffset) {
                    if (obj + kSequenceAttachmentTreeOffset) {
                        if (obj + kSequenceAttachmentTreeOffset) {
                            if (obj + kSequenceAttachmentTreeOffset) {
                                n29 = AsCPvsFxBox(obj + kSceneEffectsOffset)->sequenceAttachmentTreeRoot;
                                if (n29) {
                                    n28 = *(void**)n29;
                                    if (n28) {
                                        n27 = *(void**)n28;
                                        FREE_PAIR(obj + kSequenceAttachmentTreeOffset, n27, n26);
                                        n26 = *((void**)n28 + 1);
                                        FREE_PAIR(obj + kSequenceAttachmentTreeOffset, n26, n27);
                                        appFreeUni__FPv(n28);
                                    }
                                    n26 = *((void**)n29 + 1);
                                    FREE_QUAD(obj + kSequenceAttachmentTreeOffset, n26, n27, n28);
                                    appFreeUni__FPv(n29);
                                }
                            }
                        }
                    }
                }
            }
        }
        if (obj + kSprayAttachmentTreeOffset) {
            if (obj + kSprayAttachmentTreeOffset) {
                if (obj + kSprayAttachmentTreeOffset) {
                    if (obj + kSprayAttachmentTreeOffset) {
                        if (obj + kSprayAttachmentTreeOffset) {
                            if (obj + kSprayAttachmentTreeOffset) {
                                n26 = AsCPvsFxBox(obj + kSceneEffectsOffset)->sprayAttachmentTreeRoot;
                                if (n26) {
                                    n27 = *(void**)n26;
                                    FREE_QUAD(obj + kSprayAttachmentTreeOffset, n27, n28, n29);
                                    n27 = *((void**)n26 + 1);
                                    FREE_QUAD(obj + kSprayAttachmentTreeOffset, n27, n28, n29);
                                    appFreeUni__FPv(n26);
                                }
                            }
                        }
                    }
                }
            }
        }
        __dt__6CFxBoxFv((obj + kSceneEffectsOffset), 0);
    }
}
#pragma use_lmw_stmw off

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804625C0", _ctors_u80314290_0, CRoundDol3D_InitTwoPiConstant);
#pragma pop
