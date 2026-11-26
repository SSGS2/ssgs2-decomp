// Spray updates and drawing: per-frame motion and emission, orbit and parent tracks, and child sprays.
// Drawing covers particle sprites, position trails and ribbons, and the distance-faded colors they use.

#include <decomp/static_init.h>
#include "classes/CFxSpray.h"
#include "classes/CGallery.h"
#define AsCFxSpray(p) ((CFxSpray*)(p))
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
// CFxSpray::Update: spray scalar offsets use CFxSpray; g_view is the WView.
#include "sdk_protos.h"
#include "game/wvector.h"
#include "game/types.h"
#include "classes/CameraTransform800D.h"
#include "classes/CFxSequence.h"
// Supplemental vector/matrix views cover holes in the shared scalar layouts.
// Emission parameter contract (the same six stored floats have mode-dependent roles):
// mode                       +1A8 XYZ             +1B4 X/Y/Z
// sphericalEmission == 0     box minimum          box maximum
// spherical && circular      unused for sampling  ring radius / unused / axial length
// spherical && !circular     sphere center        inner radius / outer radius / angular span
// Ring axis is local X; Y/Z use sin/cos of emissionAngle in degrees. The spherical
// cap is centered on local +Y; angular span is passed directly to cos(span/2).
// Lengths use emitter-local world units; the cap trigonometric input is radians.
// Boundary placement later reuses these fields under its own mode branches.
#define FX_MEMBER_OFFSET(type, member) ((int)&((type*)0)->member)
// bytes (the original's lwz); on the Wii they expand to the original expressions.
#define FX_COLOR_WORD_FIELD(field) (field)
#define FX_COLOR_WORD_BYTES(bytes) (*(unsigned*)(bytes))
#define FX_COLOR_PTR_FIELD(field) (*(void**)&(field))
enum FxAxisMask {
    kFxAxisX = 1,
    kFxAxisY = 2,
    kFxAxisZ = 4,
    kFxAllAxes = 7
};
enum FxBlendState {
    kFxSourceBlendState = 13,
    kFxDestinationBlendState = 14
};
enum FxBlendFactor {
    kFxBlendZero = 0,
    kFxBlendOne = 1,
    kFxBlendSourceColor = 2,
    kFxBlendInverseSourceColor = 3,
    kFxBlendSourceAlpha = 4,
    kFxBlendInverseSourceAlpha = 5
};
enum FxUpstreamBoxFace {
    kFxMinZFace = 1,
    kFxMaxZFace = 2,
    kFxMinYFace = 4,
    kFxMaxYFace = 8,
    kFxMinXFace = 16,
    kFxMaxXFace = 32
};
enum FxChildTrigger {
    kFxPeriodicChild = 0,
    kFxAgeChild = 1,
    kFxDeathChild = 2,
    kFxGroundChild = 3
};
enum {
    kFxAbiWordBytes = 4,
    kFxRandomSampleCount = 20001,
    kFxRandomSampleMidpoint = 10000,
    kFxParentTransformCount = 4,
    kFxColorChannelCount = 4,
    kFxHistoryHeaderBytes = 0x10,
    kFxPositionSampleBytes = 0x10,
    kFxRibbonSampleBytes = 0x1C,
    kFxBoxPlaneCount = 6,
    kFxRandomPhaseSampleCount = 1001,
    kFxDebugBoundsColor = -65536,
    kFxDebugGravityColor = -256,
    kFxDebugCircleSegments = 10,
    kFxDebugSphereSegments = 64,
    kFxTriangleStrip = 0x98,
    kFxLineStrip = 0xb0,
    kFxTrailVertexFormat = 0xf001,
    kFxSpriteDrawFlags = 0x20000000,
    kFxBillboardDrawFlags = 0x20400000,
    kFxBlendAlpha = 0,
    kFxBlendAdditive = 0x800000,
    kFxBlendMultiply = 0x1000000,
    kFxBlendAlternate = 0x1800000
};
struct SpraySpatialView {
    char padding_000[0xC];
    float screenCenterX;
    float screenCenterY;
    void* sourceTransforms[4];
    char padding_024[0x70];
    Vec3f shakeOffset;
    Vec3f shakeCenter;
    char padding_0AC[0x4];
    int spriteDrawCount;
    int trailPrimitiveCount;
    Vec3f position;
    char padding_0C4[0xC];
    Vec3f velocity;
    char padding_0DC[0x4];
    float parentRotation[12];
    float modelRotation[12];
    float velocityRotation[12];
    char padding_170[0x38];
    Vec3f volumeMinimumOrCenter;
    Vec3f volumeMaximumOrRadii;
    char padding_1C0[0x24];
    Vec3f acceleration;
    char padding_1F0[0x3C];
    Vec3f minimumVelocity;
    Vec3f maximumVelocity;
    char padding_244[0x8];
    Vec3f windVector;
};
enum {
    kFxSourceTransformsOffset = FX_MEMBER_OFFSET(SpraySpatialView, sourceTransforms),
    kFxParentSourceTransformsOffset = 0x2C,
    kFxParentSecondAxis = FX_MEMBER_OFFSET(SpraySpatialView, parentRotation) + 3 * sizeof(float),
    kFxParentThirdAxis = FX_MEMBER_OFFSET(SpraySpatialView, parentRotation) + 6 * sizeof(float)
};
typedef char SpraySpatialView_offset_screenCenterX[(FX_MEMBER_OFFSET(SpraySpatialView, screenCenterX) == 0xC) ? 1 : -1];
typedef char
    SpraySpatialView_offset_screenCenterY[(FX_MEMBER_OFFSET(SpraySpatialView, screenCenterY) == 0x10) ? 1 : -1];
typedef char
    SpraySpatialView_offset_sourceTransforms[(FX_MEMBER_OFFSET(SpraySpatialView, sourceTransforms) == 0x14) ? 1 : -1];
typedef char SpraySpatialView_offset_shakeOffset[(FX_MEMBER_OFFSET(SpraySpatialView, shakeOffset) == 0x94) ? 1 : -1];
typedef char SpraySpatialView_offset_shakeCenter[(FX_MEMBER_OFFSET(SpraySpatialView, shakeCenter) == 0xA0) ? 1 : -1];
typedef char
    SpraySpatialView_offset_spriteDrawCount[(FX_MEMBER_OFFSET(SpraySpatialView, spriteDrawCount) == 0xB0) ? 1 : -1];
typedef char
    SpraySpatialView_offset_trailPrimitiveCount[(FX_MEMBER_OFFSET(SpraySpatialView, trailPrimitiveCount) == 0xB4) ? 1
                                                                                                                  : -1];
typedef char SpraySpatialView_offset_position[(FX_MEMBER_OFFSET(SpraySpatialView, position) == 0xB8) ? 1 : -1];
typedef char SpraySpatialView_offset_velocity[(FX_MEMBER_OFFSET(SpraySpatialView, velocity) == 0xD0) ? 1 : -1];
typedef char
    SpraySpatialView_offset_parentRotation[(FX_MEMBER_OFFSET(SpraySpatialView, parentRotation) == 0xE0) ? 1 : -1];
typedef char
    SpraySpatialView_offset_modelRotation[(FX_MEMBER_OFFSET(SpraySpatialView, modelRotation) == 0x110) ? 1 : -1];
typedef char
    SpraySpatialView_offset_velocityRotation[(FX_MEMBER_OFFSET(SpraySpatialView, velocityRotation) == 0x140) ? 1 : -1];
typedef char SpraySpatialView_offset_volumeMinimumOrCenter
    [(FX_MEMBER_OFFSET(SpraySpatialView, volumeMinimumOrCenter) == 0x1A8) ? 1 : -1];
typedef char SpraySpatialView_offset_volumeMaximumOrRadii
    [(FX_MEMBER_OFFSET(SpraySpatialView, volumeMaximumOrRadii) == 0x1B4) ? 1 : -1];
typedef char SpraySpatialView_offset_acceleration[(FX_MEMBER_OFFSET(SpraySpatialView, acceleration) == 0x1E4) ? 1 : -1];
typedef char
    SpraySpatialView_offset_minimumVelocity[(FX_MEMBER_OFFSET(SpraySpatialView, minimumVelocity) == 0x22C) ? 1 : -1];
typedef char
    SpraySpatialView_offset_maximumVelocity[(FX_MEMBER_OFFSET(SpraySpatialView, maximumVelocity) == 0x238) ? 1 : -1];
typedef char SpraySpatialView_offset_windVector[(FX_MEMBER_OFFSET(SpraySpatialView, windVector) == 0x24C) ? 1 : -1];
struct ParticleSpatialView {
    char padding_000[0x4];
    unsigned char motionStopped;
    unsigned char onGround;
    char padding_006[0x2];
    Vec3f localPosition;
    Vec3f initialVector;
    Vec3f emitterPosition;
    float emitterRotation[12];
    Vec3f oscillationOffset;
    char padding_068[0xC];
    Vec3f velocity;
    char padding_080[0x4];
    Vec3f angularVelocity;
    Vec3f rotation;
    int spriteFrame;
    float lastFrameChangeTime;
    float initialLifetime;
    float remainingLifetime;
    float initialSize;
    float size;
    float aspectRatio;
    unsigned int color;
    char padding_0BC[0x28];
    float lastTrailSampleTime;
    char padding_0E8[0x4];
    int childSpawnCount;
    float lastChildSpawnTime;
};
typedef char
    ParticleSpatialView_offset_motionStopped[(FX_MEMBER_OFFSET(ParticleSpatialView, motionStopped) == 0x4) ? 1 : -1];
typedef char ParticleSpatialView_offset_onGround[(FX_MEMBER_OFFSET(ParticleSpatialView, onGround) == 0x5) ? 1 : -1];
typedef char
    ParticleSpatialView_offset_localPosition[(FX_MEMBER_OFFSET(ParticleSpatialView, localPosition) == 0x8) ? 1 : -1];
typedef char
    ParticleSpatialView_offset_initialVector[(FX_MEMBER_OFFSET(ParticleSpatialView, initialVector) == 0x14) ? 1 : -1];
typedef char ParticleSpatialView_offset_emitterPosition[(FX_MEMBER_OFFSET(ParticleSpatialView, emitterPosition) == 0x20)
                                                            ? 1
                                                            : -1];
typedef char ParticleSpatialView_offset_emitterRotation[(FX_MEMBER_OFFSET(ParticleSpatialView, emitterRotation) == 0x2C)
                                                            ? 1
                                                            : -1];
typedef char ParticleSpatialView_offset_oscillationOffset
    [(FX_MEMBER_OFFSET(ParticleSpatialView, oscillationOffset) == 0x5C) ? 1 : -1];
typedef char ParticleSpatialView_offset_velocity[(FX_MEMBER_OFFSET(ParticleSpatialView, velocity) == 0x74) ? 1 : -1];
typedef char ParticleSpatialView_offset_angularVelocity[(FX_MEMBER_OFFSET(ParticleSpatialView, angularVelocity) == 0x84)
                                                            ? 1
                                                            : -1];
typedef char ParticleSpatialView_offset_rotation[(FX_MEMBER_OFFSET(ParticleSpatialView, rotation) == 0x90) ? 1 : -1];
typedef char
    ParticleSpatialView_offset_spriteFrame[(FX_MEMBER_OFFSET(ParticleSpatialView, spriteFrame) == 0x9C) ? 1 : -1];
typedef char ParticleSpatialView_offset_lastFrameChangeTime
    [(FX_MEMBER_OFFSET(ParticleSpatialView, lastFrameChangeTime) == 0xA0) ? 1 : -1];
typedef char ParticleSpatialView_offset_initialLifetime[(FX_MEMBER_OFFSET(ParticleSpatialView, initialLifetime) == 0xA4)
                                                            ? 1
                                                            : -1];
typedef char ParticleSpatialView_offset_remainingLifetime
    [(FX_MEMBER_OFFSET(ParticleSpatialView, remainingLifetime) == 0xA8) ? 1 : -1];
typedef char
    ParticleSpatialView_offset_initialSize[(FX_MEMBER_OFFSET(ParticleSpatialView, initialSize) == 0xAC) ? 1 : -1];
typedef char ParticleSpatialView_offset_size[(FX_MEMBER_OFFSET(ParticleSpatialView, size) == 0xB0) ? 1 : -1];
typedef char
    ParticleSpatialView_offset_aspectRatio[(FX_MEMBER_OFFSET(ParticleSpatialView, aspectRatio) == 0xB4) ? 1 : -1];
typedef char ParticleSpatialView_offset_color[(FX_MEMBER_OFFSET(ParticleSpatialView, color) == 0xB8) ? 1 : -1];
typedef char ParticleSpatialView_offset_lastTrailSampleTime
    [(FX_MEMBER_OFFSET(ParticleSpatialView, lastTrailSampleTime) == 0xE4) ? 1 : -1];
typedef char ParticleSpatialView_offset_childSpawnCount[(FX_MEMBER_OFFSET(ParticleSpatialView, childSpawnCount) == 0xEC)
                                                            ? 1
                                                            : -1];
typedef char ParticleSpatialView_offset_lastChildSpawnTime
    [(FX_MEMBER_OFFSET(ParticleSpatialView, lastChildSpawnTime) == 0xF0) ? 1 : -1];

struct SprayTrackLists {
    char padding_000[0x260];
    void* initialSpeedPercentTrackPrevious;
    void* initialSpeedPercentTrackNext;
    char padding_268[0x70];
    void* spawnTrackTail;
    void* spawnTrackHead;
    char padding_2E0[0x10];
    void* colorTrackTail;
    void* colorTrackHead;
};
typedef char SprayTrackLists_offset_speedTrackTail
    [(FX_MEMBER_OFFSET(SprayTrackLists, initialSpeedPercentTrackPrevious) == 0x260) ? 1 : -1];
typedef char SprayTrackLists_offset_speedTrackHead
    [(FX_MEMBER_OFFSET(SprayTrackLists, initialSpeedPercentTrackNext) == 0x264) ? 1 : -1];
typedef char
    SprayTrackLists_offset_spawnTrackTail[(FX_MEMBER_OFFSET(SprayTrackLists, spawnTrackTail) == 0x2D8) ? 1 : -1];
typedef char
    SprayTrackLists_offset_spawnTrackHead[(FX_MEMBER_OFFSET(SprayTrackLists, spawnTrackHead) == 0x2DC) ? 1 : -1];
typedef char
    SprayTrackLists_offset_colorTrackTail[(FX_MEMBER_OFFSET(SprayTrackLists, colorTrackTail) == 0x2F0) ? 1 : -1];
typedef char
    SprayTrackLists_offset_colorTrackHead[(FX_MEMBER_OFFSET(SprayTrackLists, colorTrackHead) == 0x2F4) ? 1 : -1];

struct EffectRenderStateView {
    char header[0x20];
    float view[16];
    float projection[16];
};
struct EffectOwnerRenderView {
    char prefix[0x114];
    int renderFlags;
    char padding118[0xc];
    unsigned char debugBoundsEnabled;
};

#include "classes/CFxBox.h"
#include "classes/WPuppet.h"
#include "classes/CPetBody.h"
struct PuppetAnimationRangesView {
    void** records;
    int count;
};
struct PuppetAnimationNameView {
    char prefix[0x18];
    void* name;
};
struct TrailFrameSample {
    Vec3f position;
    int spriteFrame;
};
// All track nodes begin with borrowed intrusive links; scheduled count is at +0x10.
struct FxTrackLink {
    void* previous;
    void* next;
};
struct SpawnScheduleNode {
    FxTrackLink links;
    char unknown08[8];
    int particleCount;
};
// Intrusive track-list headers embed a sentinel; its address is never a payload node.
struct FxUpdateListHeader {
    unsigned int count;
    FxTrackLink sentinel;
};
struct SprayUpdateLists {
    char prefix[0x7C];
    FxUpdateListHeader particles;
    char beforeAngularTracks[0x210];
    FxUpdateListHeader angularSpeed, velocityRotation, particleOrbit, emitterOrbit, parentOrbit;
    FxUpdateListHeader spawn, gravityPoint, color;
    float minimumSize, maximumSize, aspectRatio;
    FxUpdateListHeader size, spriteFrame, shake, flash;
    float minimumDrawDistance, maximumDrawDistance;
    FxUpdateListHeader oscillation, childSprays, sounds;
};
// Orbit rates are sampled in emitter milliseconds; scalar curves use particle lifetime percent.
struct FxAxisRateTrack {
    FxTrackLink links;
    Vec3f axis;
    int startMilliseconds;
    float startRate;
    int endMilliseconds;
    float endRate;
};
struct FxLifetimeScalarTrack {
    FxTrackLink links;
    float startPercent, startValue, endPercent, endValue;
};
typedef char
    FxUpdate_lists_particleOrbit[(FX_MEMBER_OFFSET(SprayUpdateLists, particleOrbit.sentinel) == 0x2B4) ? 1 : -1];
typedef char FxUpdate_lists_sounds[(FX_MEMBER_OFFSET(SprayUpdateLists, sounds.sentinel) == 0x358) ? 1 : -1];
typedef char FxAxisRateTrack_end[(FX_MEMBER_OFFSET(FxAxisRateTrack, endRate) == 0x20) ? 1 : -1];
typedef char FxLifetimeScalarTrack_end[(FX_MEMBER_OFFSET(FxLifetimeScalarTrack, endValue) == 0x14) ? 1 : -1];

// Initialization-only ranges: angles and angular rates are transformed by velocityRotation.
struct SprayInitialRotationRanges {
    char prefix[0x268];
    Vec3f minimumRotation, maximumRotation;
    Vec3f minimumAngularVelocity, maximumAngularVelocity;
};
typedef char SprayInitialRotationRanges_end[(sizeof(SprayInitialRotationRanges) == 0x298) ? 1 : -1];
struct ParticleColorTrack {
    void* previous;
    void* next;
    float startPercent;
    char startColor[4];
    float endPercent;
    char endColor[4];
};
struct ChildSprayCommand {
    int trigger;
    char script[0x104];
    unsigned char followSource;
    char padding109[3];
    float intervalOrAge;
    int maximumCount;
};
struct ChildSprayRegistryValue {
    unsigned int key;
    unsigned int emitter;
};
struct ChildSprayRegistryNode {
    void* left;
    void* right;
    unsigned int color;
    ChildSprayRegistryValue value;
};

#define AsSpraySpatial(p) ((SpraySpatialView*)(p))
#define AsParticleSpatial(p) ((ParticleSpatialView*)(p))

#include "classes/CFxParticle.h"
#include "classes/CFxRegistryTree.h"
#define AsCFxParticle(p) ((CFxParticle*)(p))

// ---- merged from fx_spray_particles.cpp ----
namespace ns0 {
// Particle update, translation unit 0x80119230.

struct V3 {
    unsigned int words[3];
};
struct Mat {
    V3 rows[4];
};
struct Link {
    Link* prev;
    Link* next;
};
// Typed-width byte views used by the intentionally overlapping workspace.
// Offset names below document live roles; these do not allocate/convert/own data.
// Keep address expressions at the access site: an inline function changes alias
// analysis and register allocation in CFxSpray::Update under these pragmas.
#define FxAddress(base, offset) ((char*)(base) + (offset))
// Typed lvalues preserve CodeWarrior alias/lifetime behavior in the shared arena.
#define FxFloat(base, offset) (*(float*)FxAddress(base, offset))
#define FxInt(base, offset) (*(int*)FxAddress(base, offset))
#define FxWord(base, offset) (*(unsigned int*)FxAddress(base, offset))
#define FxByte(base, offset) (*(unsigned char*)FxAddress(base, offset))
#define FxPointer(base, offset) (*(void**)FxAddress(base, offset))
#define FxLink(base, offset) (*(Link**)FxAddress(base, offset))
#define FxVectorBits(base, offset) (*(V3*)FxAddress(base, offset))
#define FxMatrixBits(base, offset) (*(Mat*)FxAddress(base, offset))

extern "C" {
extern float kFloat_0_FxSpray;
extern float kFloat_1_FxSpray;
extern float kFloat_neg_1_805DE648;
extern float kFxMotionDeltaTimeScale;
extern float kFloat_0_00001_FxSpray;
extern float kFloat_20000_RandomRange;
extern float kFloat_0_9_FxCullFacingScale;
extern float kFloat_0_75_FxCullAspectScale;
extern float kFloat_360_FxSpray;
extern float kFloat_100_fxSpray;
extern float kFloat_1_5_FxDrawDistanceScale;
extern float kFloat_0_5_FxSpray;
extern float kFloat_0_01_FxSpray;
extern float kFloat_0_03_FxGroundSettleDistance;
extern float kFloat_0_0001_ParticleEmitter;
extern float kFloat_1000_FxAnimationTimeScale;
extern float kFloat_0_006283186_FxRandomPhaseStep;
extern float kFloat_3_1415927_FxSpray;
extern float kFloat_0_002_FxSpray;
extern float kFloat_200_FxSpray;
void __ml__FfRC7WVector(void*, const void*, float);
void __apl__7WVectorFRC7WVector(void*, const void*);
void Matrix_SetIdentityAffine(void*);
void Matrix_ExtractUniformScaleNormalizedRotation(void*, void*);
void Matrix_ConcatAffine(void*, const void*, const void*);
void Matrix_CopyAffine(void*, const void*);
void __as__7WVectorFRC7WVector(void*, const void*);
void Vec3_Zero(void*);
void* Matrix_TransformPoint(void*, const void*, const void*);
float Magnitude__7WVectorCFv(const void*);
float __ml__FRC7WVectorRC7WVector(const void*, const void*);
float Vec3_DotProduct(const void*, const void*);
double acos(double);
void Matrix_FromEulerAngles(void*, const void*);
void Matrix_InvertAffine(void*, const void*);
void Matrix_SetNegativeAxisAngleRotation(void*, const void*, float); /* (matrix out, axis, angle) */
void Matrix_ConcatAffineAssign(void*, const void*);
void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
void __ml__FRC7WVectorf(void*, const void*, float);
void Vec3_DivideScalar(void*, float, const void*);
void Matrix_ExpandAffineTo4x4(const void*, void*);
void Vec3_CopyToOutput(const void*, void*);
void Vec3_MultiplyComponents(void*, const void*, const void*);
void Vec3_Add(void*, const void*, const void*);
void Vec3_Subtract(void*, const void*, const void*);
void Vec3_Scale(void*, const void*, float);
void Vec3_Normalize(void*, const void*);
float Vec3_Dot(const void*, const void*);
float Vec3_LengthSquared(const void*);
void Vec3_Cross(void*, const void*, const void*);
float Math_CosApprox(float);
void FxSpray_ComputeScaledBounds(int, void*, void*, float, float);
float FxSpray_ComputeSpatialFalloff(void*, const void*, const void*, float, float, int);
void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
void Sound_PlayByName(void*, void*, void*, int, int, int, float, float, float);
extern void* g_view;
extern void* g_soundManager;
extern CameraTransform800D s_cameraTransform;
extern V3 kUnitYAxis;
}

struct NodeOwner {
    Link* node;
    // Own an unlinked phase node until insertion explicitly disarms this guard.
    NodeOwner(Link* p) : node(p) {}
    // Free node storage if allocation/copy unwinds before linking completes.
    ~NodeOwner() {
        if (node)
            appFreeUni__FPv(node);
    }
};

// Transform a vector by a column-oriented 4x4 matrix, including translation.
static inline void transform(void* out, const void* m, const V3& v) {
    // Column-oriented affine point transform, implicit input W=1. Retain the
    // original sum/product grouping because fused operations affect code bytes.
    ((float*)out)[0] = ((const float*)m)[12] +
                       (((const float*)&v)[2] * ((const float*)m)[8] +
                        (((const float*)&v)[0] * ((const float*)m)[0] + ((const float*)&v)[1] * ((const float*)m)[4]));
    ((float*)out)[1] = ((const float*)m)[13] +
                       (((const float*)&v)[2] * ((const float*)m)[9] +
                        (((const float*)&v)[0] * ((const float*)m)[1] + ((const float*)&v)[1] * ((const float*)m)[5]));
    ((float*)out)[2] = ((const float*)m)[14] +
                       (((const float*)&v)[2] * ((const float*)m)[10] +
                        (((const float*)&v)[0] * ((const float*)m)[2] + ((const float*)&v)[1] * ((const float*)m)[6]));
}
// Test magnitude against the emitter epsilon while retaining the original sign-bit handling.
static inline int nearlyZero(const float* x) {
    float a;
    if (*(const unsigned int*)x & 0x80000000)
        a = -*x;
    else
        a = *x;
    return a < kFloat_0_00001_FxSpray;
}
// Copy four three-word matrix rows without floating-point conversions.
static inline void copyRows(void* out, const void* in) {
    FxVectorBits(out, 0) = FxVectorBits(in, 0);
    FxVectorBits(out, 0xc) = FxVectorBits(in, 0xc);
    FxVectorBits(out, 0x18) = FxVectorBits(in, 0x18);
    FxVectorBits(out, 0x24) = FxVectorBits(in, 0x24);
}

// Stable slots in the update workspace; these offsets are not object fields.
// Workspace lifetime map: 0x2C8..2D3 is reused successively for volume-test
// position, random oscillation phase and spatial sound position. The particle link
// at 0x1C overlaps an earlier angle magnitude. Do not turn these into simultaneous
// struct members or move a consumer across phases without checking those lifetimes.
// Alias/lifetime rules (half-open byte intervals):
// - [0x190,0x19C): emitter displacement, then X-axis transform; consumed before reuse.
// - [0x1C,0x20): Y reference magnitude, then the live particle-list cursor.
// - [0x2C8,0x2D4): volume position -> new oscillation phase -> sound position.
// - [0x418,0x448) overlaps parent source [0x430,0x460): parent composition ends
//   before velocity-track rotation starts.
// - [0x768,0x798) overlaps parent frame [0x780,0x7B0): volume testing must stay
//   after parent-frame setup. Never hoist its writes into that earlier phase.
// - [0x37C,0x388) is the SECOND vector of core bounds.
// - [0x72C,0x738) is particle-frame translation, not another matrix.
// Each name below describes a live value or subview; +4/+8 select Y/Z components.
// Affine matrices are 12 floats; expanded matrices are 16. WorldStep is motion
// displacement for this update; LocalFrictionStep is subtracted from local velocity.
enum FxUpdateScratch {
    kFxWorldPosition = 0x2E0,
    kFxPreviousWorldPosition = 0x2D4,
    kFxCurrentEmitterOrigin = 0x2BC,
    kFxParticleEmitterOrigin = 0x2B0,
    kFxParticleFrameAffine = 0x708,
    kFxParticleFrame44 = 0x6C8,
    kFxCoreBounds = 0x370,
    kFxParentFrameAffine = 0x780,
    kFxVelocityFrame44 = 0x688,
    kFxWorldVelocity = 0x238,
    kFxWorldStep = 0x244,
    kFxLocalFrictionStep = 0x22C,
    kFxOrbitFrameAffine = 0x508,
    kFxOrbitFrame44 = 0x648,
    kFxInverseParticleFrameAffine = 0x4A8,
    kFxInverseParticleFrame44 = 0x5C8,
    kFxShakeWeights = 0x304,
    kFxZAxisReferenceLength = 0x14,
    kFxZAxisProjectedLength = 0x18,
    kFxYAxisLengthOrParticleLink = 0x1c,
    kFxYAxisProjectedLength = 0x20,
    kFxXAxisReferenceLength = 0x24,
    kFxXAxisProjectedLength = 0x28,
    kFxGroundSurfaceHeight = 0x30,
    kFxCapturedOriginCopy = 0x40,
    kFxCapturedRotatedOrigin = 0x4c,
    kFxCurrentRotatedOrigin = 0x58,
    kFxSoundRelativePosition = 0x64,
    kFxSphereRelativePosition = 0x70,
    kFxMaximumEpsilon = 0x7c,
    kFxExpandedMaximumCopy = 0x88,
    kFxMinimumEpsilon = 0x94,
    kFxExpandedMinimumCopy = 0xa0,
    kFxTrackedAngularStep = 0xac,
    kFxAngularStepTimesPercent = 0xb8,
    kFxAngularStepScaled = 0xc4,
    kFxAngularStep = 0xd0,
    kFxAccelerationStep = 0xdc,
    kFxVelocityPlusAcceleration = 0xe8,
    kFxTrailWorldPosition = 0xf4,
    kFxParticleToCamera = 0x100,
    kFxSelectedOriginCopy = 0x10c,
    kFxCurrentOriginCopy = 0x118,
    kFxShakeInterpolatedAmplitude = 0x124,
    kFxShakeInterpolatedDelta = 0x130,
    kFxShakeAmplitudeTimesAge = 0x13c,
    kFxShakeAmplitudeDelta = 0x148,
    kFxParentEulerInput = 0x154,
    kFxZAxisTransformed = 0x160,
    kFxZAxisReference = 0x16c,
    kFxYAxisTransformed = 0x178,
    kFxYAxisReference = 0x184,
    kFxEmitterStepOrXAxisTransform = 0x190,
    kFxXAxisReference = 0x19c,
    kFxCommitEmitterOrigin = 0x1b4,
    kFxCommitRelativePosition = 0x1c0,
    kFxOscillationSide = 0x1cc,
    kFxOscillationUp = 0x1d8,
    kFxOscillationForwardAndSum = 0x1e4,
    kFxOscillationAmplitudes = 0x1f0,
    kFxVolumeRelativePosition = 0x1fc,
    kFxOrbitRelativePosition = 0x208,
    kFxGravityDirectionAndStep = 0x214,
    kFxGravityEmitterOrigin = 0x220,
    kFxScaledWind = 0x250,
    kFxLocalVelocityCopy = 0x25c,
    kFxFacingDirection = 0x268,
    kFxCameraToParticle = 0x274,
    kFxRotatedLocalPosition = 0x280,
    kFxResolvedWorldPosition = 0x28c,
    kFxEmitterOriginCopy = 0x298,
    kFxLocalPositionCopy = 0x2a4,
    kFxVolumePositionOrPhaseOrSound = 0x2c8,
    kFxWeightedShakeCenter = 0x2ec,
    kFxShakeAmplitude = 0x2f8,
    kFxMaskedParentAngles = 0x310,
    kFxAxisProjection = 0x31c,
    kFxAxisReference = 0x328,
    kFxParentAxisSwap = 0x334,
    kFxExpandedVolumeMinimum = 0x358,
    kFxExpandedVolumeMaximum = 0x364,
    kFxCoreBoundsSecondVector = 0x37c,
    kFxVolumeSourceRotation = 0x388,
    kFxVolumeProductTemporary = 0x3b8,
    kFxOrbitTrackRotation = 0x3e8,
    kFxVelocityTrackRotation = 0x418,
    kFxParentSourceRotation = 0x430,
    kFxParentProduct = 0x460,
    kFxInverseVolumeRotation = 0x4d8,
    kFxSelectedRotationCopy = 0x538,
    kFxMaskedParentRotation = 0x568,
    kFxInverseMaskedParentRotation = 0x598,
    kFxInverseVolumeFrame44 = 0x608,
    kFxVolumeNormalizedRotation = 0x738,
    kFxVolumeParentProduct = 0x768,
    kFxParticleFrameTranslation = kFxParticleFrameAffine + 0x24
};

// Axis-rate segment offsets are from the link-node base, not payload +8.
enum FxAxisRateTrackOffset {
    kFxTrackAxis = 8,
    kFxTrackStartMs = 0x14,
    kFxTrackStartRate = 0x18,
    kFxTrackEndMs = 0x1C,
    kFxTrackEndRate = 0x20
};

// Scalar/color lifetime curves share node-relative offsets; values are floats
// for angular/size curves, four packed bytes for color. Frame tracks differ.
enum FxLifetimeCurveOffset {
    kFxCurveStartPercent = 8,
    kFxCurveStartValue = 0x0C,
    kFxCurveEndPercent = 0x10,
    kFxCurveEndValue = 0x14
};

// Track payload map (node has intrusive links at +0/+4; payload begins +8):
// - Axis rate: axis +08, int start/end milliseconds +14/+1C, float rates +18/+20.
// - Color/angular/size: float lifetime percent +08/+10, endpoint values +0C/+14.
//   Equal angular endpoints use angularVelocity directly; equal size endpoints
//   restore initialSize. Only unequal endpoints apply an interpolated /100 scale.
// - Sprite frame: float percent +08/+0C; int first/last frame +10/+14; int tick
//   interval milliseconds +18. first==-1 chooses rand()%last (last is exclusive);
//   otherwise sequential frames wrap after last (inclusive).
// - Shake, relative to PAYLOAD: float center[3] +00; int startMs +0C; float
//   amplitudes[3] +10; int endMs +1C; float final amplitudes[3] +20;
//   int sampleIntervalMs +2C and mutable float lastSampleMs +30.
// - Gravity, relative to PAYLOAD: float position[3] +00, inner strength +0C,
//   radius +10, outer strength +14. Inside radius uses inverse squared distance
//   floored at 0.5; outside radius uses the fixed outer strength.
// - Oscillation: node +08 is amplitude[3], +14 is frequency[3]; each particle
//   owns a parallel list of three random phases. Keep track/list ordering aligned.

// Parent-composition phase view of the 0x7B0-byte update workspace. This is an
// overlay, not additional storage: later particle phases reuse these bytes.
struct FxWorkspaceAffine {
    float words[12];
};
struct FxParentWorkspace {
    char unused000[0x14];
    float zReferenceLength, zProjectedLength, yReferenceLength, yProjectedLength;
    float xReferenceLength, xProjectedLength;
    char unused02C[0x128];
    Vec3f eulerInput, zTransformed, zReference, yTransformed, yReference;
    Vec3f emitterStepOrXTransformed, xReference;
    char unused1A8[0x168];
    Vec3f maskedAngles, axisProjection, axisReference, savedParentAxis;
    char unused340[0xF0];
    FxWorkspaceAffine parentSource, parentProduct;
    char unused490[0xD8];
    FxWorkspaceAffine maskedRotation, inverseMaskedRotation;
    char unused5C8[0x1B8];
    FxWorkspaceAffine parentFrame;
};
typedef char FxParentWorkspace_size[(sizeof(FxParentWorkspace) == 0x7B0) ? 1 : -1];
#define FX_PARENT_SLOT(member, slot) \
    typedef char FxParentWorkspace_##member[(FX_MEMBER_OFFSET(FxParentWorkspace, member) == slot) ? 1 : -1]
FX_PARENT_SLOT(xReferenceLength, kFxXAxisReferenceLength);
FX_PARENT_SLOT(eulerInput, kFxParentEulerInput);
FX_PARENT_SLOT(emitterStepOrXTransformed, kFxEmitterStepOrXAxisTransform);
FX_PARENT_SLOT(maskedAngles, kFxMaskedParentAngles);
FX_PARENT_SLOT(parentSource, kFxParentSourceRotation);
FX_PARENT_SLOT(maskedRotation, kFxMaskedParentRotation);
FX_PARENT_SLOT(parentFrame, kFxParentFrameAffine);
#undef FX_PARENT_SLOT

// Particle-motion view of the same workspace. These ranges are live during each
// particle iteration, after parent setup; they are not simultaneous parent locals.
struct FxParticleMotionWorkspace {
    char unused000[0x214];
    Vec3f gravityDirectionAndStep, gravityEmitterOrigin;
    Vec3f localFrictionStep, worldVelocity, worldStep, scaledWind;
    Vec3f localVelocityCopy, facingDirection, cameraToParticle;
    Vec3f rotatedLocalPosition, resolvedWorldPosition, emitterOriginCopy, localPositionCopy;
    Vec3f particleEmitterOrigin, currentEmitterOrigin;
    Vec3f volumePositionOrPhaseOrSound;
    Vec3f previousWorldPosition, worldPosition;
    char unused2EC[0x7B0 - 0x2EC];
};
typedef char FxParticleMotion_size[(sizeof(FxParticleMotionWorkspace) == 0x7B0) ? 1 : -1];
typedef char FxParticleMotion_worldPosition
    [(FX_MEMBER_OFFSET(FxParticleMotionWorkspace, worldPosition) == kFxWorldPosition) ? 1 : -1];
typedef char
    FxParticleMotion_worldStep[(FX_MEMBER_OFFSET(FxParticleMotionWorkspace, worldStep) == kFxWorldStep) ? 1 : -1];
// Measure the unsigned reference/projection angle in radians, returning zero for
// either near-zero length and clamping the cosine before acos. The caller supplies
// phase-owned length slots so the overlapping update workspace keeps its layout.
static inline float projectedParentAxisAngle(FxParentWorkspace& workspace, float& referenceLength,
                                             float& projectedLength) {
    float angle;
    if (nearlyZero(&referenceLength))
        angle = kFloat_0_FxSpray;
    else {
        projectedLength = (*(WVector*)&workspace.axisProjection).Magnitude();
        if (nearlyZero(&projectedLength))
            angle = kFloat_0_FxSpray;
        else {
            float cosineRatio = (*(WVector*)&workspace.axisReference * *(WVector*)&workspace.axisProjection) /
                                (referenceLength * projectedLength);
            float clampedCosine;
            if (cosineRatio < kFloat_neg_1_805DE648)
                clampedCosine = kFloat_neg_1_805DE648;
            else if (cosineRatio > kFloat_1_FxSpray)
                clampedCosine = kFloat_1_FxSpray;
            else
                clampedCosine = cosineRatio;
            angle = (float)acos(clampedCosine);
        }
    }
    return angle;
}

} // namespace ns0

#pragma opt_common_subs off
#pragma opt_loop_invariants off
// Advance emitter tracks and particles, applying motion, collision, color/size/frame tracks, trails, sound and child effects.
// deltaTime and elapsed/lifetime/track times use milliseconds (model tick divides
// by 1000). Motion uses the historical 0.06*deltaTime simulation scale, not seconds.
// Returns true while particles remain OR scheduled emission reports more work.
// The emitter owns particle list nodes and deleting particle objects; tracks/parent
// transforms are borrowed. Particle localPosition is committed from world space
// at the end of each iteration; culling exits intentionally skip the later phases.
int CFxSpray::Update(float deltaTime) {
    using namespace ns0;
    float scaledDeltaTime;
    char* parentTransformCursor;
    int i;
    // Scratch vectors and matrices overlap intentionally to retain the matched stack frame.
    // Each FxAddress(scratch, ...) below names a slot in this private workspace, not a game-object field.
    // Long-lived slots have names in FxUpdateScratch. Other slots are phase-local;
    // The particle view names motion ranges. Slot 0x2C8 still uses the byte view
    // because volume tests, oscillation phases and sounds have distinct lifetimes.
    // No pointer into either view escapes CFxSpray::Update.
    union {
        double align;
        char bytes[0x7b0];
        FxParentWorkspace parent;
        FxParticleMotionWorkspace particle;
    } storage;
    char* scratch = storage.bytes;
    // Phase 1: integrate emitter position and compose up to four parent transforms.
    scaledDeltaTime = kFxMotionDeltaTimeScale * deltaTime;
    __ml__FfRC7WVector(&storage.parent.emitterStepOrXTransformed,
                       ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, velocity.x)), scaledDeltaTime);
    *(WVector*)(((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, position.x))) +=
        *(WVector*)(&storage.parent.emitterStepOrXTransformed);
    if (AsCFxSpray(this)->attachedToParent && (*(void**)&AsCFxSpray(this)->parentSequence)) {
        parentTransformCursor = (char*)(*(void**)&AsCFxSpray(this)->parentSequence);
        Matrix_SetIdentityAffine(&storage.parent.parentFrame);
        __alloca(0);
        i = 0;
        while (i < kFxParentTransformCount) {
            if (FxPointer(parentTransformCursor, kFxParentSourceTransformsOffset)) {
                Matrix_ExtractUniformScaleNormalizedRotation(
                    FxPointer(parentTransformCursor, kFxParentSourceTransformsOffset), &storage.parent.parentSource);
                Matrix_ConcatAffine(&storage.parent.parentProduct, &storage.parent.parentFrame,
                                    &storage.parent.parentSource);
                Matrix_CopyAffine(&storage.parent.parentFrame, &storage.parent.parentProduct);
                parentTransformCursor += kFxAbiWordBytes;
                ++i;
            } else
                break;
        }
        Matrix_ExtractUniformScaleNormalizedRotation(
            &storage.parent.parentFrame, ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, parentRotation)));
        Matrix_CopyAffine(((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, modelRotation)),
                          ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, parentRotation)));
        // Swap the stored second/third basis vectors after composing the parent frame.
        // This is an axis-convention adjustment, not a position transform.
        (*(V3*)&storage.parent.savedParentAxis) = FxVectorBits(this, kFxParentSecondAxis);
        *(WVector*)(FxAddress(this, kFxParentSecondAxis)) = *(WVector*)(FxAddress(this, kFxParentThirdAxis));
        *(WVector*)(FxAddress(this, kFxParentThirdAxis)) = *(WVector*)(&storage.parent.savedParentAxis);
        if ((*(int*)&AsCFxSpray(this)->parentRotationAxisMask) &&
            (*(int*)&AsCFxSpray(this)->parentRotationAxisMask) != kFxAllAxes) {
            Vec3_Zero(&storage.parent.maskedAngles);
            if (AsCFxSpray(this)->parentRotationAxisMask & kFxAxisX) {
                storage.parent.xReference.x = kFloat_0_FxSpray;
                storage.parent.xReference.y = kFloat_1_FxSpray;
                storage.parent.xReference.z = kFloat_0_FxSpray;
                *(WVector*)(&storage.parent.axisReference) = *(WVector*)(&storage.parent.xReference);
                Matrix_TransformPoint(&storage.parent.emitterStepOrXTransformed, &storage.parent.axisReference,
                                      ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, parentRotation)));
                *(WVector*)(&storage.parent.axisProjection) = *(WVector*)(&storage.parent.emitterStepOrXTransformed);
                storage.parent.axisProjection.x = kFloat_0_FxSpray;
                storage.parent.xReferenceLength = (*(WVector*)(&storage.parent.axisReference)).Magnitude();
                float angle = projectedParentAxisAngle(storage.parent, storage.parent.xReferenceLength,
                                                       storage.parent.xProjectedLength);
                storage.parent.maskedAngles.x = angle;
                if (!(nearlyZero((float*)FxAddress(scratch, kFxAxisProjection + 8))))
                    storage.parent.maskedAngles.x *=
                        storage.parent.axisProjection.z < kFloat_0_FxSpray ? kFloat_neg_1_805DE648 : kFloat_1_FxSpray;
            }
            if (AsCFxSpray(this)->parentRotationAxisMask & kFxAxisY) {
                storage.parent.yReference.x = kFloat_0_FxSpray;
                storage.parent.yReference.y = kFloat_0_FxSpray;
                storage.parent.yReference.z = kFloat_1_FxSpray;
                *(WVector*)(&storage.parent.axisReference) = *(WVector*)(&storage.parent.yReference);
                Matrix_TransformPoint(&storage.parent.yTransformed, &storage.parent.axisReference,
                                      ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, parentRotation)));
                *(WVector*)(&storage.parent.axisProjection) = *(WVector*)(&storage.parent.yTransformed);
                storage.parent.axisProjection.y = kFloat_0_FxSpray;
                storage.parent.yReferenceLength = (*(WVector*)(&storage.parent.axisReference)).Magnitude();
                float angle = projectedParentAxisAngle(storage.parent, storage.parent.yReferenceLength,
                                                       storage.parent.yProjectedLength);
                storage.parent.maskedAngles.y = angle;
                if (!(nearlyZero((float*)&storage.parent.axisProjection)))
                    storage.parent.maskedAngles.y *=
                        storage.parent.axisProjection.x < kFloat_0_FxSpray ? kFloat_neg_1_805DE648 : kFloat_1_FxSpray;
            }
            if (AsCFxSpray(this)->parentRotationAxisMask & kFxAxisZ) {
                storage.parent.zReference.y = kFloat_0_FxSpray;
                storage.parent.zReference.x = kFloat_1_FxSpray;
                storage.parent.zReference.z = kFloat_0_FxSpray;
                *(WVector*)(&storage.parent.axisReference) = *(WVector*)(&storage.parent.zReference);
                Matrix_TransformPoint(&storage.parent.zTransformed, &storage.parent.axisReference,
                                      ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, parentRotation)));
                *(WVector*)(&storage.parent.axisProjection) = *(WVector*)(&storage.parent.zTransformed);
                storage.parent.axisProjection.z = kFloat_0_FxSpray;
                storage.parent.zReferenceLength = (*(WVector*)(&storage.parent.axisReference)).Magnitude();
                float angle = projectedParentAxisAngle(storage.parent, storage.parent.zReferenceLength,
                                                       storage.parent.zProjectedLength);
                storage.parent.maskedAngles.z = angle;
                if (!(nearlyZero((float*)FxAddress(scratch, kFxAxisProjection + 4))))
                    storage.parent.maskedAngles.z *=
                        storage.parent.axisProjection.y > kFloat_0_FxSpray ? kFloat_neg_1_805DE648 : kFloat_1_FxSpray;
            }
            (*(V3*)&storage.parent.eulerInput) = (*(V3*)&storage.parent.maskedAngles);
            Matrix_FromEulerAngles(&storage.parent.maskedRotation, &storage.parent.eulerInput);
            Matrix_InvertAffine(&storage.parent.inverseMaskedRotation, &storage.parent.maskedRotation);
            Matrix_CopyAffine(((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, parentRotation)),
                              &storage.parent.inverseMaskedRotation);
        }
    }
    // Axis-angle track node contract: links +00/+04; axis float[3] +08;
    // startMs/value +14/+18; endMs/value +1C/+20. Inclusive endpoints;
    // interpolate angular rate then multiply by scaledDeltaTime for this step.
    // Phase 2: evaluate emitter rotation/orbit, weighted shake, then saturated flash.
    // Parent axis masks select Euler components; extracted angles are radians.
    float time = AsCFxSpray(this)->elapsedTime;
    for (Link* n = (*(Link**)&AsCFxSpray(this)->velocityRotationTrackHead);
         (Link*)((char*)&((SprayUpdateLists*)this)->velocityRotation.sentinel) != n; n = n->next) {
        int start = ((FxAxisRateTrack*)n)->startMilliseconds, end = ((FxAxisRateTrack*)n)->endMilliseconds;
        if (time >= (float)start && time <= (float)end) {
            float v;
            if (time - (float)start > kFloat_0_FxSpray)
                v = ((FxAxisRateTrack*)n)->startRate +
                    (((FxAxisRateTrack*)n)->endRate - ((FxAxisRateTrack*)n)->startRate) * (time - (float)start) /
                        (float)(end - start);
            else
                v = ((FxAxisRateTrack*)n)->startRate;
            Matrix_SetNegativeAxisAngleRotation(FxAddress(scratch, kFxVelocityTrackRotation),
                                                FxAddress(n, kFxTrackAxis),
                                                v * scaledDeltaTime); /* orig 0x80119844: r4 = node+8 is the axis */
            Matrix_ConcatAffineAssign(((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, velocityRotation)),
                                      FxAddress(scratch, kFxVelocityTrackRotation));
            break;
        }
    }
    AsCFxSpray(this)->ApplyEmitterOrbitTrack(AsCFxSpray(this)->elapsedTime, scaledDeltaTime);
    AsCFxSpray(this)->ApplyParentOrbitTrack(AsCFxSpray(this)->elapsedTime, scaledDeltaTime);
    FxFloat(scratch, kFxShakeWeights) = kFloat_0_FxSpray;
    FxFloat(scratch, kFxShakeWeights + 4) = kFloat_0_FxSpray;
    FxFloat(scratch, kFxShakeWeights + 8) = kFloat_0_FxSpray;
    Vec3_Zero(((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, shakeCenter.x)));
    int active = 0;
    for (Link* n = (*(Link**)&((SprayUpdateLists*)this)->shake.sentinel.next);
         n != (Link*)((char*)&((SprayUpdateLists*)this)->shake.sentinel); n = n->next) {
        void* p = FxAddress(n, 8);
        float t = AsCFxSpray(this)->elapsedTime;
        if (t < (float)FxInt(n, 0x14)) {
            FxFloat(p, 0x30) = t;
            continue;
        }
        if (!(t <= (float)FxInt(p, 0x1c)))
            continue;
        active = 1;
        if (!(t - FxFloat(p, 0x30) >= (float)FxInt(p, 0x2c)))
            continue;
        FxFloat(p, 0x30) = t;
        int start = FxInt(p, 0xc), duration = FxInt(p, 0x1c) - start;
        const void* value;
        if (duration > 0) {
            __mi__FRC7WVectorRC7WVector(FxAddress(scratch, kFxShakeAmplitudeDelta), FxAddress(p, 0x20),
                                        FxAddress(p, 0x10));
            __ml__FRC7WVectorf(FxAddress(scratch, kFxShakeAmplitudeTimesAge),
                               FxAddress(scratch, kFxShakeAmplitudeDelta),
                               AsCFxSpray(this)->elapsedTime - (float)start);
            ((void (*)(void*, const void*, float))Vec3_DivideScalar)(FxAddress(scratch, kFxShakeInterpolatedDelta),
                                                                     FxAddress(scratch, kFxShakeAmplitudeTimesAge),
                                                                     (float)duration);
            __pl__FRC7WVectorRC7WVector(FxAddress(scratch, kFxShakeInterpolatedAmplitude),
                                        FxAddress(scratch, kFxShakeInterpolatedDelta), FxAddress(p, 0x10));
            value = FxAddress(scratch, kFxShakeInterpolatedAmplitude);
        } else
            value = FxAddress(p, 0x10);
        *(WVector*)(FxAddress(scratch, kFxShakeAmplitude)) = *(WVector*)value;
        Vec3_Zero(((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, shakeOffset.x)));
        int random = rand();
        AsSpraySpatial(this)->shakeOffset.x += FxFloat(scratch, kFxShakeAmplitude) *
                                               (float)(random % kFxRandomSampleCount - kFxRandomSampleMidpoint) /
                                               kFloat_20000_RandomRange;
        random = rand();
        AsSpraySpatial(this)->shakeOffset.y += FxFloat(scratch, kFxShakeAmplitude + 4) *
                                               (float)(random % kFxRandomSampleCount - kFxRandomSampleMidpoint) /
                                               kFloat_20000_RandomRange;
        random = rand();
        AsSpraySpatial(this)->shakeOffset.z += FxFloat(scratch, kFxShakeAmplitude + 8) *
                                               (float)(random % kFxRandomSampleCount - kFxRandomSampleMidpoint) /
                                               kFloat_20000_RandomRange;
        Vec3_MultiplyComponents(FxAddress(scratch, kFxWeightedShakeCenter), p, FxAddress(scratch, kFxShakeAmplitude));
        Vec3_Add(((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, shakeCenter.x)),
                 ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, shakeCenter.x)),
                 FxAddress(scratch, kFxWeightedShakeCenter));
        Vec3_Add(FxAddress(scratch, kFxShakeWeights), FxAddress(scratch, kFxShakeWeights),
                 FxAddress(scratch, kFxShakeAmplitude));
    }
    if (!active)
        Vec3_Zero(((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, shakeOffset.x)));
    else {
        if (!(nearlyZero((float*)FxAddress(scratch, kFxShakeWeights))))
            AsSpraySpatial(this)->shakeCenter.x /= FxFloat(scratch, kFxShakeWeights);
        if (!(nearlyZero((float*)FxAddress(scratch, kFxShakeWeights + 4))))
            AsSpraySpatial(this)->shakeCenter.y /= FxFloat(scratch, kFxShakeWeights + 4);
        if (!(nearlyZero((float*)FxAddress(scratch, kFxShakeWeights + 8))))
            AsSpraySpatial(this)->shakeCenter.z /= FxFloat(scratch, kFxShakeWeights + 8);
    }
    AsCFxSpray(this)->flashOpacity = 0;
    for (Link* n = (*(Link**)&((SprayUpdateLists*)this)->flash.sentinel.next);
         n != (Link*)((char*)&((SprayUpdateLists*)this)->flash.sentinel); n = n->next) {
        int start = FxInt(n, 8), peak = FxInt(n, 0xc);
        float t = AsCFxSpray(this)->elapsedTime;
        if (t >= (float)start) {
            int v;
            if (t <= (float)peak) {
                int d = peak - start;
                v = d > 0 ? 255 * (int)(t - (float)start) / d : 0;
                int sum = v + AsCFxSpray(this)->flashOpacity;
                AsCFxSpray(this)->flashOpacity = sum > 255 ? 255 : sum;
            } else if (t <= (float)FxInt(n, 0x10)) {
                int d = FxInt(n, 0x10) - peak;
                v = d > 0 ? 255 - 255 * (int)(t - (float)peak) / d : 0;
                int sum = v + AsCFxSpray(this)->flashOpacity;
                AsCFxSpray(this)->flashOpacity = sum > 255 ? 255 : sum;
            }
        }
    }
    // Phase 3: emit scheduled particles, derive volume core bounds and culling cone.
    int alive = AsCFxSpray(this)->EmitScheduledParticles(AsCFxSpray(this)->elapsedTime);
    Vec3_Zero(FxAddress(scratch, kFxCoreBounds));
    Vec3_Zero(FxAddress(scratch, kFxCoreBoundsSecondVector));
    FxSpray_ComputeScaledBounds(AsCFxSpray(this)->sphericalEmission, FxAddress(scratch, kFxCoreBounds),
                                ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, volumeMinimumOrCenter.x)),
                                AsCFxSpray(this)->volumeCoreScaleX, AsCFxSpray(this)->volumeCoreScaleY);
    int regeneratedThisPass = 0;
    float threshold =
        kFloat_0_9_FxCullFacingScale *
        (kFloat_1_FxSpray -
         (AsWView(g_view)->fieldOfView /
          (kFloat_0_75_FxCullAspectScale * (AsWView(g_view)->viewportWidth / AsWView(g_view)->viewportHeight))) /
             kFloat_360_FxSpray);
    Link*& current = FxLink(scratch, kFxYAxisLengthOrParticleLink);
    current = (*(Link**)&AsCFxSpray(this)->particleListHead);
    while (current != (Link*)((char*)&((SprayUpdateLists*)this)->particles.sentinel)) {
        // Particle phase A: choose current or captured emitter origin/rotation,
        // then reconstruct world position = transformed localPosition + origin.
        void* particle = FxPointer(current, 8);
        const void* source;
        if ((*(int*)&AsCFxSpray(this)->positionIsWorldSpace)) {
            FxVectorBits(scratch, kFxCurrentOriginCopy) = FxVectorBits(this, 0xb8);
            source = FxAddress(scratch, kFxCurrentOriginCopy);
        } else {
            void* parentSequence = (*(void**)&AsCFxSpray(this)->parentSequence);
            if (parentSequence) {
                if (AsCFxSpray(this)->attachedToParent) {
                    Matrix_TransformPoint(FxAddress(scratch, kFxCurrentRotatedOrigin),
                                          ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, position.x)),
                                          ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, parentRotation)));
                    __pl__FRC7WVectorRC7WVector(FxAddress(scratch, kFxCurrentOriginCopy),
                                                FxAddress(scratch, kFxCurrentRotatedOrigin),
                                                FxAddress(parentSequence, 0x84));
                } else
                    __pl__FRC7WVectorRC7WVector(FxAddress(scratch, kFxCurrentOriginCopy),
                                                ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, position.x)),
                                                FxAddress(parentSequence, 0x84));
            } else
                FxVectorBits(scratch, kFxCurrentOriginCopy) = FxVectorBits(this, 0xb8);
            source = FxAddress(scratch, kFxCurrentOriginCopy);
        }
        *(WVector*)(&storage.particle.currentEmitterOrigin) = *(WVector*)source;
        if (AsCFxSpray(this)->particlesFollowEmitter) {
            if ((*(int*)&AsCFxSpray(this)->positionIsWorldSpace))
                FxVectorBits(scratch, kFxCapturedOriginCopy) = FxVectorBits(this, 0xb8);
            else {
                void* parentSequence = (*(void**)&AsCFxSpray(this)->parentSequence);
                if (parentSequence) {
                    if (AsCFxSpray(this)->attachedToParent) {
                        Matrix_TransformPoint(FxAddress(scratch, kFxCapturedRotatedOrigin),
                                              ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, position.x)),
                                              ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, parentRotation)));
                        __pl__FRC7WVectorRC7WVector(FxAddress(scratch, kFxCapturedOriginCopy),
                                                    FxAddress(scratch, kFxCapturedRotatedOrigin),
                                                    FxAddress(parentSequence, 0x84));
                    } else
                        __pl__FRC7WVectorRC7WVector(FxAddress(scratch, kFxCapturedOriginCopy),
                                                    ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, position.x)),
                                                    FxAddress(parentSequence, 0x84));
                } else
                    FxVectorBits(scratch, kFxCapturedOriginCopy) = FxVectorBits(this, 0xb8);
            }
            source = FxAddress(scratch, kFxCapturedOriginCopy);
        } else
            source = ((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, emitterPosition.x));
        FxVectorBits(scratch, kFxSelectedOriginCopy) = *(const V3*)source;
        *(WVector*)(&storage.particle.particleEmitterOrigin) = *(WVector*)(FxAddress(scratch, kFxSelectedOriginCopy));
        source = AsCFxSpray(this)->particlesFollowEmitter
                     ? ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, parentRotation))
                     : ((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, emitterRotation));
        copyRows(FxAddress(scratch, kFxSelectedRotationCopy), source);
        copyRows(FxAddress(scratch, kFxSelectedRotationCopy), source);
        FxMatrixBits(scratch, kFxSelectedRotationCopy) = *(const Mat*)source;
        copyRows(FxAddress(scratch, kFxSelectedRotationCopy), source);
        copyRows(FxAddress(scratch, kFxSelectedRotationCopy), source);
        FxMatrixBits(scratch, kFxSelectedRotationCopy) = *(const Mat*)source;
        Matrix_CopyAffine(FxAddress(scratch, kFxParticleFrameAffine), FxAddress(scratch, kFxSelectedRotationCopy));
        Matrix_ExpandAffineTo4x4(FxAddress(scratch, kFxParticleFrameAffine), FxAddress(scratch, kFxParticleFrame44));
        Vec3_CopyToOutput(((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, localPosition.x)),
                          &storage.particle.localPositionCopy);
        Vec3_CopyToOutput(&storage.particle.particleEmitterOrigin, &storage.particle.emitterOriginCopy);
        transform(&storage.particle.rotatedLocalPosition, FxAddress(scratch, kFxParticleFrame44),
                  FxVectorBits(scratch, kFxLocalPositionCopy));
        Vec3_Add(&storage.particle.resolvedWorldPosition, &storage.particle.rotatedLocalPosition,
                 &storage.particle.emitterOriginCopy);
        FxVectorBits(scratch, kFxWorldPosition) = FxVectorBits(scratch, kFxResolvedWorldPosition);
        // Particle phase B: expire/delete or regenerate BEFORE view/distance culling.
        // Regeneration continues at the same node once, suppressing a second age decrement.
        if (kFloat_0_FxSpray < AsParticleSpatial(particle)->remainingLifetime && !regeneratedThisPass) {
            AsParticleSpatial(particle)->remainingLifetime -= deltaTime;
            if (AsParticleSpatial(particle)->remainingLifetime <= kFloat_0_FxSpray) {
                for (Link* n = (*(Link**)&((SprayUpdateLists*)this)->childSprays.sentinel.next);
                     n != (Link*)((char*)&((SprayUpdateLists*)this)->childSprays.sentinel); n = n->next)
                    if (FxInt(n, 8) == kFxDeathChild)
                        SpawnChildSpray(FxAddress(n, 8), particle);
                if (AsCFxSpray(this)->regenerateAtSamePosition) {
                    regeneratedThisPass = 1;
                    InitializeParticle(AsCFxSpray(this)->regenerateAtSamePosition, particle,
                                       FxAddress(scratch, kFxCoreBounds), 1, kFloat_100_fxSpray, kFloat_0_FxSpray);
                    continue;
                }
                Link* next = current->next;
                current->prev->next = next;
                current->next->prev = current->prev;
                appFreeUni__FPv(current);
                --(*(int*)&AsCFxSpray(this)->particleCount);
                current = next;
                if (particle)
                    ((void (*)(void*, short))FxPointer(AsCFxParticle(particle)->vtbl, 8))(particle, 1);
                continue;
            }
        }
        if (AsCFxSpray(this)->viewCullingEnabled) {
            Vec3_Subtract(&storage.particle.cameraToParticle, &storage.particle.worldPosition,
                          s_cameraTransform.position);
            Vec3_Normalize(&storage.particle.facingDirection, &storage.particle.cameraToParticle);
            if (Vec3_Dot(&storage.particle.facingDirection, &s_cameraTransform.basis[6]) <= threshold) {
                current = current->next;
                continue;
            }
        }
        __mi__FRC7WVectorRC7WVector(FxAddress(scratch, kFxParticleToCamera), s_cameraTransform.position,
                                    &storage.particle.worldPosition);
        float distance = (*(WVector*)(FxAddress(scratch, kFxParticleToCamera))).Magnitude();
        if (AsCFxSpray(this)->maxDrawDistance > kFloat_1_FxSpray &&
            distance > AsCFxSpray(this)->maxDrawDistance * kFloat_1_5_FxDrawDistanceScale) {
            current = current->next;
            continue;
        }
        float lifetimePercent = AsParticleSpatial(particle)->initialLifetime > kFloat_0_FxSpray
                                    ? (kFloat_100_fxSpray * (AsParticleSpatial(particle)->initialLifetime -
                                                             AsParticleSpatial(particle)->remainingLifetime)) /
                                          AsParticleSpatial(particle)->initialLifetime
                                    : kFloat_100_fxSpray;
        __pl__FRC7WVectorRC7WVector(FxAddress(scratch, kFxTrailWorldPosition), &storage.particle.worldPosition,
                                    ((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, oscillationOffset.x)));
        this->AppendTrailSample((ns3::Stream*)particle, (const ns3::Mat*)FxAddress(scratch, kFxTrailWorldPosition),
                                AsCFxSpray(this)->elapsedTime);
        // Particle phase C: integrate world motion, friction, forces and ground bounce.
        // Trail sampling above records the pre-step position plus oscillation offset.
        if (!AsParticleSpatial(particle)->motionStopped) {
            *(WVector*)(&storage.particle.previousWorldPosition) = *(WVector*)(&storage.particle.worldPosition);
            if (AsParticleSpatial(particle)->velocity.x != kFloat_0_FxSpray ||
                (*(float*)&AsCFxParticle(particle)->velocityY) != kFloat_0_FxSpray ||
                (*(float*)&AsCFxParticle(particle)->velocityZ) != kFloat_0_FxSpray) {
                Matrix_ExpandAffineTo4x4(FxAddress(scratch, kFxParticleFrameAffine),
                                         FxAddress(scratch, kFxVelocityFrame44));
                Vec3_CopyToOutput(((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x)),
                                  &storage.particle.localVelocityCopy);
                transform(&storage.particle.worldVelocity, FxAddress(scratch, kFxVelocityFrame44),
                          FxVectorBits(scratch, kFxLocalVelocityCopy));
            } else
                FxVectorBits(scratch, kFxWorldVelocity) = FxVectorBits(scratch, kFxParticleFrameTranslation);
            if (AsCFxParticle(particle)->forceScale != kFloat_0_FxSpray) {
                Vec3_CopyToOutput(((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, windVector.x)),
                                  &storage.particle.scaledWind);
                Vec3_Scale(&storage.particle.scaledWind, &storage.particle.scaledWind,
                           AsCFxParticle(particle)->forceScale);
                Vec3_Add(&storage.particle.worldStep, &storage.particle.worldVelocity, &storage.particle.scaledWind);
                Vec3_Scale(&storage.particle.worldStep, &storage.particle.worldStep, scaledDeltaTime);
            } else
                Vec3_Scale(&storage.particle.worldStep, &storage.particle.worldVelocity, scaledDeltaTime);
            storage.particle.worldPosition.x += storage.particle.worldStep.x;
            storage.particle.worldPosition.y += storage.particle.worldStep.y;
            storage.particle.worldPosition.z += storage.particle.worldStep.z;
            Vec3_CopyToOutput(((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x)),
                              &storage.particle.localFrictionStep);
            if (storage.particle.localFrictionStep.x != kFloat_0_FxSpray ||
                storage.particle.localFrictionStep.y != kFloat_0_FxSpray ||
                storage.particle.localFrictionStep.z != kFloat_0_FxSpray)
                Vec3_Normalize(&storage.particle.localFrictionStep, &storage.particle.localFrictionStep);
            Vec3_Scale(&storage.particle.localFrictionStep, &storage.particle.localFrictionStep, scaledDeltaTime);
            Vec3_Scale(&storage.particle.localFrictionStep, &storage.particle.localFrictionStep,
                       AsParticleSpatial(particle)->onGround ? AsCFxSpray(this)->groundFriction
                                                             : AsCFxSpray(this)->friction);
            if ((AsParticleSpatial(particle)->velocity.x > kFloat_0_FxSpray &&
                 AsParticleSpatial(particle)->velocity.x < storage.particle.localFrictionStep.x) ||
                (AsParticleSpatial(particle)->velocity.x < kFloat_0_FxSpray &&
                 AsParticleSpatial(particle)->velocity.x > storage.particle.localFrictionStep.x))
                AsParticleSpatial(particle)->velocity.x = kFloat_0_FxSpray;
            else
                AsParticleSpatial(particle)->velocity.x -= storage.particle.localFrictionStep.x;
            if (((*(float*)&AsCFxParticle(particle)->velocityY) > kFloat_0_FxSpray &&
                 (*(float*)&AsCFxParticle(particle)->velocityY) < storage.particle.localFrictionStep.y) ||
                ((*(float*)&AsCFxParticle(particle)->velocityY) < kFloat_0_FxSpray &&
                 (*(float*)&AsCFxParticle(particle)->velocityY) > storage.particle.localFrictionStep.y))
                (*(float*)&AsCFxParticle(particle)->velocityY) = kFloat_0_FxSpray;
            else
                (*(float*)&AsCFxParticle(particle)->velocityY) -= storage.particle.localFrictionStep.y;
            if (((*(float*)&AsCFxParticle(particle)->velocityZ) > kFloat_0_FxSpray &&
                 (*(float*)&AsCFxParticle(particle)->velocityZ) < storage.particle.localFrictionStep.z) ||
                ((*(float*)&AsCFxParticle(particle)->velocityZ) < kFloat_0_FxSpray &&
                 (*(float*)&AsCFxParticle(particle)->velocityZ) > storage.particle.localFrictionStep.z))
                (*(float*)&AsCFxParticle(particle)->velocityZ) = kFloat_0_FxSpray;
            else
                (*(float*)&AsCFxParticle(particle)->velocityZ) -= storage.particle.localFrictionStep.z;
            if (!AsParticleSpatial(particle)->onGround) {
                int force = 0;
                for (Link* n = (*(Link**)&AsCFxSpray(this)->gravityPointTrackHead);
                     n != (Link*)((char*)&((SprayUpdateLists*)this)->gravityPoint.sentinel); n = n->next) {
                    void* p = FxAddress(n, 8);
                    FxVectorBits(scratch, kFxGravityEmitterOrigin) = FxVectorBits(scratch, kFxCurrentEmitterOrigin);
                    Vec3_Add(&storage.particle.gravityDirectionAndStep, p, &storage.particle.gravityEmitterOrigin);
                    Vec3_Subtract(&storage.particle.gravityDirectionAndStep, &storage.particle.gravityDirectionAndStep,
                                  &storage.particle.worldPosition);
                    float d = Vec3_LengthSquared(&storage.particle.gravityDirectionAndStep);
                    Vec3_Normalize(&storage.particle.gravityDirectionAndStep,
                                   &storage.particle.gravityDirectionAndStep);
                    if (d < FxFloat(p, 0x10) * FxFloat(p, 0x10)) {
                        if (d < kFloat_0_5_FxSpray)
                            d = kFloat_0_5_FxSpray;
                        Vec3_Scale(&storage.particle.gravityDirectionAndStep, &storage.particle.gravityDirectionAndStep,
                                   (scaledDeltaTime * FxFloat(p, 0xc)) / d);
                    } else
                        Vec3_Scale(&storage.particle.gravityDirectionAndStep, &storage.particle.gravityDirectionAndStep,
                                   scaledDeltaTime * FxFloat(p, 0x14));
                    Vec3_Add(((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x)),
                             ((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x)),
                             &storage.particle.gravityDirectionAndStep);
                    force = 1;
                }
                FxFloat(scratch, kFxGroundSurfaceHeight) =
                    AsCFxSpray(this)->groundHeightOffset + storage.particle.particleEmitterOrigin.y;
                void* world = AsCFxSpray(this)->effectOwner;
                // Slot 0x10 is CPvsFxBox_TestGroundCrossing(this, float verticalStep, prev, cur, groundHeight): the float
                // parameter comes second.
                if (force && ((int (*)(void*, void*, void*, void*, float))FxPointer(FxPointer(world, 0), 0x10))(
                                 world, &storage.particle.previousWorldPosition, &storage.particle.worldPosition,
                                 FxAddress(scratch, kFxGroundSurfaceHeight), AsCFxSpray(this)->groundCollisionRadius)) {
                    float y = storage.particle.worldPosition.y - AsCFxSpray(this)->groundCollisionRadius;
                    float gap = FxFloat(scratch, kFxGroundSurfaceHeight) - y;
                    gap = (gap > kFloat_0_FxSpray) ? gap : y - FxFloat(scratch, kFxGroundSurfaceHeight);
                    if (storage.particle.previousWorldPosition.y < storage.particle.worldPosition.y)
                        storage.particle.worldPosition.y =
                            FxFloat(scratch, kFxGroundSurfaceHeight) - AsCFxSpray(this)->groundCollisionRadius;
                    else
                        storage.particle.worldPosition.y =
                            FxFloat(scratch, kFxGroundSurfaceHeight) + AsCFxSpray(this)->groundCollisionRadius;
                    (*(float*)&AsCFxParticle(particle)->velocityY) =
                        kFloat_0_01_FxSpray *
                        (-(*(float*)&AsCFxParticle(particle)->velocityY) * AsCFxSpray(this)->groundRestitution);
                    float speed = (*(float*)&AsCFxParticle(particle)->velocityY) > kFloat_0_FxSpray
                                      ? (*(float*)&AsCFxParticle(particle)->velocityY)
                                      : -(*(float*)&AsCFxParticle(particle)->velocityY);
                    volatile bool slow = speed < kFloat_0_01_FxSpray;
                    int stop = (gap < kFloat_0_03_FxGroundSettleDistance && speed < gap) || slow;
                    if (!stop) {
                        __pl__FRC7WVectorRC7WVector(
                            FxAddress(scratch, kFxVelocityPlusAcceleration),
                            ((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x)),
                            ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, acceleration.x)));
                        stop = Vec3_DotProduct(((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x)),
                                               FxAddress(scratch, kFxVelocityPlusAcceleration)) < kFloat_0_FxSpray;
                    }
                    if (stop) {
                        (*(float*)&AsCFxParticle(particle)->velocityY) = kFloat_0_FxSpray;
                        AsParticleSpatial(particle)->onGround = 1;
                    }
                    for (Link* n = (*(Link**)&((SprayUpdateLists*)this)->childSprays.sentinel.next);
                         n != (Link*)((char*)&((SprayUpdateLists*)this)->childSprays.sentinel); n = n->next)
                        if (FxInt(n, 8) == kFxGroundChild)
                            SpawnChildSpray(FxAddress(n, 8), particle);
                } else {
                    __ml__FfRC7WVector(FxAddress(scratch, kFxAccelerationStep),
                                       ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, acceleration.x)),
                                       scaledDeltaTime);
                    *(WVector*)(((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x))) +=
                        *(WVector*)(FxAddress(scratch, kFxAccelerationStep));
                }
                if ((*(int*)&AsCFxSpray(this)->particleOrbitTrackCount)) {
                    float t = AsCFxSpray(this)->elapsedTime;
                    Matrix_SetIdentityAffine(FxAddress(scratch, kFxOrbitFrameAffine));
                    for (Link* n = (*(Link**)&AsCFxSpray(this)->particleOrbitTrackHead);
                         n != (Link*)((char*)&((SprayUpdateLists*)this)->particleOrbit.sentinel); n = n->next) {
                        int start = ((FxAxisRateTrack*)n)->startMilliseconds,
                            end = ((FxAxisRateTrack*)n)->endMilliseconds;
                        if (t >= (float)start && t <= (float)end) {
                            float v;
                            if (t - (float)start > kFloat_0_FxSpray)
                                v = ((FxAxisRateTrack*)n)->startRate +
                                    (((FxAxisRateTrack*)n)->endRate - ((FxAxisRateTrack*)n)->startRate) *
                                        (t - (float)start) / (float)(end - start);
                            else
                                v = ((FxAxisRateTrack*)n)->startRate;
                            Matrix_SetNegativeAxisAngleRotation(
                                FxAddress(scratch, kFxOrbitTrackRotation), FxAddress(n, kFxTrackAxis),
                                v * scaledDeltaTime); /* orig 0x8011AA4C: r4 = node+8 is the axis */
                            Matrix_CopyAffine(FxAddress(scratch, kFxOrbitFrameAffine),
                                              FxAddress(scratch, kFxOrbitTrackRotation));
                        }
                    }
                    Matrix_ExpandAffineTo4x4(FxAddress(scratch, kFxOrbitFrameAffine),
                                             FxAddress(scratch, kFxOrbitFrame44));
                    Vec3_Subtract(FxAddress(scratch, kFxOrbitRelativePosition), &storage.particle.worldPosition,
                                  &storage.particle.currentEmitterOrigin);
                    V3 v;
                    transform(&v, FxAddress(scratch, kFxOrbitFrame44), FxVectorBits(scratch, kFxOrbitRelativePosition));
                    storage.particle.worldPosition.x = storage.particle.currentEmitterOrigin.x + FxFloat(&v, 0);
                    storage.particle.worldPosition.y = storage.particle.currentEmitterOrigin.y + FxFloat(&v, 4);
                    storage.particle.worldPosition.z = storage.particle.currentEmitterOrigin.z + FxFloat(&v, 8);
                }
            }
        }
        // Particle phase D: color/angular-speed/size/frame tracks use lifetimePercent
        // in [0,100] rather than emitter milliseconds. First matching segment wins.
        for (Link* n = (*(Link**)&AsCFxSpray(this)->colorTrackHead);
             n != (Link*)((char*)&((SprayUpdateLists*)this)->color.sentinel); n = n->next) {
            float start = ((FxLifetimeScalarTrack*)n)->startPercent, end = ((FxLifetimeScalarTrack*)n)->endPercent;
            if (lifetimePercent >= start && lifetimePercent <= end) {
                float d = end - start;
                if (d > kFloat_0_FxSpray) {
                    if (FxWord(n, kFxCurveStartValue) == FxWord(n, kFxCurveEndValue))
                        AsParticleSpatial(particle)->color = FxWord(n, kFxCurveStartValue);
                    else {
                        float f = (lifetimePercent - start) / d;
                        for (int i = 0; i < kFxColorChannelCount; ++i) {
                            unsigned char a = FxByte(n, kFxCurveStartValue + i), b = FxByte(n, kFxCurveEndValue + i);
                            if (b == a)
                                FxByte(particle, 0xb8 + i) = a;
                            else
                                FxByte(particle, 0xb8 + i) = (int)(f * (float)((int)b - (int)a) + (float)a);
                        }
                    }
                } else {
                    (*(unsigned char*)&AsParticleSpatial(particle)->color) = FxByte(n, kFxCurveStartValue);
                    FxByte(particle, 0xb9) = FxByte(n, kFxCurveStartValue + 1);
                    FxByte(particle, 0xba) = FxByte(n, kFxCurveStartValue + 2);
                    FxByte(particle, 0xbb) = FxByte(n, kFxCurveStartValue + 3);
                }
                break;
            }
        }
        if ((*(void**)&AsCFxParticle(particle)->spriteResource) || AsCFxParticle(particle)->petBody) {
            for (Link* n = (*(Link**)&AsCFxSpray(this)->angularSpeedTrackHead);
                 n != (Link*)((char*)&((SprayUpdateLists*)this)->angularSpeed.sentinel); n = n->next) {
                float start = ((FxLifetimeScalarTrack*)n)->startPercent, end = ((FxLifetimeScalarTrack*)n)->endPercent;
                if (lifetimePercent >= start && lifetimePercent <= end) {
                    float value = ((FxLifetimeScalarTrack*)n)->startValue,
                          final = ((FxLifetimeScalarTrack*)n)->endValue;
                    if (value == final) {
                        __ml__FfRC7WVector(FxAddress(scratch, kFxAngularStep),
                                           ((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, angularVelocity.x)),
                                           scaledDeltaTime);
                        *(WVector*)(((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, rotation.x))) +=
                            *(WVector*)(FxAddress(scratch, kFxAngularStep));
                    } else {
                        float d = end - start;
                        if (d > kFloat_0_FxSpray)
                            value += (final - value) * (lifetimePercent - start) / d;
                        __ml__FfRC7WVector(FxAddress(scratch, kFxTrackedAngularStep),
                                           ((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, angularVelocity.x)),
                                           scaledDeltaTime);
                        __ml__FRC7WVectorf(FxAddress(scratch, kFxAngularStepTimesPercent),
                                           FxAddress(scratch, kFxTrackedAngularStep), value);
                        ((void (*)(void*, const void*, float))Vec3_DivideScalar)(
                            FxAddress(scratch, kFxAngularStepScaled), FxAddress(scratch, kFxAngularStepTimesPercent),
                            kFloat_100_fxSpray);
                        *(WVector*)(((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, rotation.x))) +=
                            *(WVector*)(FxAddress(scratch, kFxAngularStepScaled));
                    }
                    break;
                }
            }
            for (Link* n = (*(Link**)&AsCFxSpray(this)->sizeTrackHead);
                 n != (Link*)((char*)&((SprayUpdateLists*)this)->size.sentinel); n = n->next) {
                float start = ((FxLifetimeScalarTrack*)n)->startPercent, end = ((FxLifetimeScalarTrack*)n)->endPercent;
                if (lifetimePercent >= start && lifetimePercent <= end) {
                    float value = ((FxLifetimeScalarTrack*)n)->startValue,
                          final = ((FxLifetimeScalarTrack*)n)->endValue;
                    if (value == final)
                        AsParticleSpatial(particle)->size = AsParticleSpatial(particle)->initialSize;
                    else {
                        float d = end - start;
                        if (d > kFloat_0_FxSpray)
                            value += (final - value) * (lifetimePercent - start) / d;
                        AsParticleSpatial(particle)->size =
                            (AsParticleSpatial(particle)->initialSize * value) / kFloat_100_fxSpray;
                    }
                    break;
                }
            }
            if (!AsParticleSpatial(particle)->onGround) {
                for (Link* n = (*(Link**)&AsCFxSpray(this)->spriteFrameTrackHead);
                     n != (Link*)((char*)&((SprayUpdateLists*)this)->spriteFrame.sentinel); n = n->next) {
                    if (lifetimePercent >= FxFloat(n, 8) && lifetimePercent <= FxFloat(n, 0xc) &&
                        AsCFxSpray(this)->elapsedTime - AsParticleSpatial(particle)->lastFrameChangeTime >=
                            (float)FxInt(n, 0x18)) {
                        AsParticleSpatial(particle)->lastFrameChangeTime = AsCFxSpray(this)->elapsedTime;
                        if (FxInt(n, 0x10) == -1)
                            AsParticleSpatial(particle)->spriteFrame = rand() % FxInt(n, 0x14);
                        else {
                            ++AsParticleSpatial(particle)->spriteFrame;
                            if (AsParticleSpatial(particle)->spriteFrame > FxInt(n, 0x14))
                                AsParticleSpatial(particle)->spriteFrame = FxInt(n, 0x10);
                        }
                        break;
                    }
                }
            }
            if (!(*(int*)&AsCFxSpray(this)->minLifetime) && kFloat_0_FxSpray != deltaTime) {
                Matrix_SetIdentityAffine(FxAddress(scratch, kFxVolumeParentProduct));
                void* p = this;
                for (int i = 0; i < kFxParentTransformCount; ++i, p = FxAddress(p, 4)) {
                    if (!FxPointer(p, 0x14))
                        break;
                    Matrix_ExtractUniformScaleNormalizedRotation(FxPointer(p, 0x14),
                                                                 FxAddress(scratch, kFxVolumeSourceRotation));
                    Matrix_ConcatAffine(FxAddress(scratch, kFxVolumeProductTemporary),
                                        FxAddress(scratch, kFxVolumeParentProduct),
                                        FxAddress(scratch, kFxVolumeSourceRotation));
                    Matrix_CopyAffine(FxAddress(scratch, kFxVolumeParentProduct),
                                      FxAddress(scratch, kFxVolumeProductTemporary));
                }
                Matrix_ExtractUniformScaleNormalizedRotation(FxAddress(scratch, kFxVolumeParentProduct),
                                                             FxAddress(scratch, kFxVolumeNormalizedRotation));
                Matrix_InvertAffine(FxAddress(scratch, kFxInverseVolumeRotation),
                                    FxAddress(scratch, kFxVolumeNormalizedRotation));
                Matrix_ExpandAffineTo4x4(FxAddress(scratch, kFxInverseVolumeRotation),
                                         FxAddress(scratch, kFxInverseVolumeFrame44));
                Vec3_Subtract(FxAddress(scratch, kFxVolumeRelativePosition), &storage.particle.worldPosition,
                              &storage.particle.particleEmitterOrigin);
                transform(FxAddress(scratch, kFxVolumePositionOrPhaseOrSound),
                          FxAddress(scratch, kFxInverseVolumeFrame44),
                          FxVectorBits(scratch, kFxVolumeRelativePosition));
                int inside;
                if (!AsCFxSpray(this)->sphericalEmission) {
                    Vec3_Zero(FxAddress(scratch, kFxExpandedVolumeMinimum));
                    Vec3_Zero(FxAddress(scratch, kFxExpandedVolumeMaximum));
                    FxFloat(scratch, kFxMinimumEpsilon) = kFloat_0_00001_FxSpray;
                    FxFloat(scratch, kFxMinimumEpsilon + 4) = kFloat_0_00001_FxSpray;
                    FxFloat(scratch, kFxMinimumEpsilon + 8) = kFloat_0_00001_FxSpray;
                    __mi__FRC7WVectorRC7WVector(
                        FxAddress(scratch, kFxExpandedMinimumCopy),
                        ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, volumeMinimumOrCenter.x)),
                        FxAddress(scratch, kFxMinimumEpsilon));
                    *(WVector*)(FxAddress(scratch, kFxExpandedVolumeMinimum)) =
                        *(WVector*)(FxAddress(scratch, kFxExpandedMinimumCopy));
                    FxFloat(scratch, kFxMaximumEpsilon) = kFloat_0_00001_FxSpray;
                    FxFloat(scratch, kFxMaximumEpsilon + 4) = kFloat_0_00001_FxSpray;
                    FxFloat(scratch, kFxMaximumEpsilon + 8) = kFloat_0_00001_FxSpray;
                    __pl__FRC7WVectorRC7WVector(
                        FxAddress(scratch, kFxExpandedMaximumCopy),
                        ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, volumeMaximumOrRadii.x)),
                        FxAddress(scratch, kFxMaximumEpsilon));
                    *(WVector*)(FxAddress(scratch, kFxExpandedVolumeMaximum)) =
                        *(WVector*)(FxAddress(scratch, kFxExpandedMaximumCopy));
                    inside = FxFloat(scratch, kFxExpandedVolumeMinimum) <=
                                 FxFloat(scratch, kFxVolumePositionOrPhaseOrSound) &&
                             FxFloat(scratch, kFxVolumePositionOrPhaseOrSound) <=
                                 FxFloat(scratch, kFxExpandedVolumeMaximum) &&
                             FxFloat(scratch, kFxExpandedVolumeMinimum + 4) <=
                                 FxFloat(scratch, kFxVolumePositionOrPhaseOrSound + 4) &&
                             FxFloat(scratch, kFxVolumePositionOrPhaseOrSound + 4) <=
                                 FxFloat(scratch, kFxExpandedVolumeMaximum + 4) &&
                             FxFloat(scratch, kFxExpandedVolumeMinimum + 8) <=
                                 FxFloat(scratch, kFxVolumePositionOrPhaseOrSound + 8) &&
                             FxFloat(scratch, kFxVolumePositionOrPhaseOrSound + 8) <=
                                 FxFloat(scratch, kFxExpandedVolumeMaximum + 8);
                } else {
                    __mi__FRC7WVectorRC7WVector(
                        FxAddress(scratch, kFxSphereRelativePosition),
                        FxAddress(scratch, kFxVolumePositionOrPhaseOrSound),
                        ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, volumeMinimumOrCenter.x)));
                    inside = (*(WVector*)(FxAddress(scratch, kFxSphereRelativePosition))).Magnitude() <
                             kFloat_0_0001_ParticleEmitter + AsSpraySpatial(this)->volumeMaximumOrRadii.y;
                }
                if (inside)
                    AsCFxParticle(particle)->opacityScale =
                        ((float (*)(void*, const void*, const void*, int, float, float))FxSpray_ComputeSpatialFalloff)(
                            FxAddress(scratch, kFxVolumePositionOrPhaseOrSound),
                            ((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, volumeMinimumOrCenter.x)),
                            FxAddress(scratch, kFxCoreBounds), AsCFxSpray(this)->sphericalEmission,
                            AsCFxSpray(this)->volumeCoreScaleX, AsCFxSpray(this)->volumeCoreScaleY);
                else {
                    InitializeParticle(AsCFxSpray(this)->regenerateAtSamePosition, particle,
                                       FxAddress(scratch, kFxCoreBounds), 1, kFloat_100_fxSpray, kFloat_0_FxSpray);
                    regeneratedThisPass = 0;
                    current = current->next;
                    continue;
                }
            }
            // Publish the updated transform/opacity to the optional owned model body.
            if (AsCFxParticle(particle)->petBody) {
                ((CPetBody*)(AsCFxParticle(particle)->petBody))
                    ->SetOpacity((unsigned char)(int)(AsCFxSpray(this)->opacityMultiplier *
                                                      ((float)(*(unsigned char*)&AsParticleSpatial(particle)->color) *
                                                       AsCFxParticle(particle)->opacityScale)));
                ((CPetBody*)(AsCFxParticle(particle)->petBody))->SetupLighting();
                ((CPetBody*)(AsCFxParticle(particle)->petBody))->SetPosition(&storage.particle.worldPosition);
                ((CPetBody*)(AsCFxParticle(particle)->petBody))
                    ->SetRotation(((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, rotation.x)));
                ((CPetBody*)(AsCFxParticle(particle)->petBody))
                    ->ApplyRotationMatrix(((char*)this + FX_MEMBER_OFFSET(SpraySpatialView, modelRotation)));
                ((CPetBody*)(AsCFxParticle(particle)->petBody))->SetUniformScale(AsParticleSpatial(particle)->size);
                void* model = AsCFxParticle(particle)->petBody;
                ((void (*)(void*, float))FxPointer(FxPointer(model, 0), 0x74))(
                    model, deltaTime / kFloat_1000_FxAnimationTimeScale);
            }
        }
        // Particle phase E: trigger age/periodic children, oscillation and queued sounds.
        // Death/ground children were handled at their respective transitions above.
        for (Link* n = (*(Link**)&((SprayUpdateLists*)this)->childSprays.sentinel.next);
             n != (Link*)((char*)&((SprayUpdateLists*)this)->childSprays.sentinel); n = n->next) {
            void* p = FxAddress(n, 8);
            if (FxInt(p, 0) == kFxAgeChild && !AsParticleSpatial(particle)->childSpawnCount &&
                FxFloat(p, 0x10c) < kFloat_1_FxSpray + lifetimePercent)
                SpawnChildSpray(p, particle);
            if (!FxInt(p, 0) &&
                AsCFxSpray(this)->elapsedTime - AsParticleSpatial(particle)->lastChildSpawnTime > FxFloat(p, 0x10c) &&
                FxInt(p, 0x110) > AsParticleSpatial(particle)->childSpawnCount) {
                AsParticleSpatial(particle)->lastChildSpawnTime = AsCFxSpray(this)->elapsedTime;
                SpawnChildSpray(p, particle);
            }
        }
        if (FxInt(this, 0x33c)) {
            FxFloat(scratch, kFxOscillationAmplitudes) = kFloat_0_FxSpray;
            FxFloat(scratch, kFxOscillationAmplitudes + 4) = kFloat_0_FxSpray;
            FxFloat(scratch, kFxOscillationAmplitudes + 8) = kFloat_0_FxSpray;
            Link* phase = FxLink(particle, 0x70);
            for (Link* n = (*(Link**)&((SprayUpdateLists*)this)->oscillation.sentinel.next);
                 n != (Link*)((char*)&((SprayUpdateLists*)this)->oscillation.sentinel); n = n->next) {
                if (phase == (Link*)((char*)particle + FX_MEMBER_OFFSET(CFxParticle, listTail))) {
                    float radians = kFloat_0_006283186_FxRandomPhaseStep;
                    int random = rand();
                    FxFloat(scratch, kFxVolumePositionOrPhaseOrSound) =
                        radians * (float)(random % kFxRandomPhaseSampleCount);
                    random = rand();
                    FxFloat(scratch, kFxVolumePositionOrPhaseOrSound + 4) =
                        radians * (float)(random % kFxRandomPhaseSampleCount);
                    random = rand();
                    FxFloat(scratch, kFxVolumePositionOrPhaseOrSound + 8) =
                        radians * (float)(random % kFxRandomPhaseSampleCount);
                    Link* head = (Link*)((char*)particle + FX_MEMBER_OFFSET(CFxParticle, listTail));
                    Link* added = (Link*)appMallocDummy__FUl(0x14);
                    NodeOwner owner(added);
                    if (FxAddress(added, 8))
                        FxVectorBits(added, 8) = FxVectorBits(scratch, kFxVolumePositionOrPhaseOrSound);
                    owner.node = 0;
                    head->prev->next = added;
                    added->prev = head->prev;
                    head->prev = added;
                    added->next = head;
                    ++(*(int*)&AsCFxParticle(particle)->listNodeCount);
                    phase = phase->prev;
                }
                Link* p = phase;
                phase = phase->next;
                if (FxFloat(n, 8) != kFloat_0_FxSpray)
                    FxFloat(scratch, kFxOscillationAmplitudes) +=
                        FxFloat(n, 8) *
                        Math_CosApprox(FxFloat(n, 0x14) * (kFloat_3_1415927_FxSpray *
                                                           (kFloat_0_002_FxSpray * AsCFxSpray(this)->elapsedTime)) +
                                       FxFloat(p, 8));
                if (FxFloat(n, 0xc) != kFloat_0_FxSpray)
                    FxFloat(scratch, kFxOscillationAmplitudes + 4) +=
                        FxFloat(n, 0xc) *
                        Math_CosApprox(FxFloat(n, 0x18) * (kFloat_3_1415927_FxSpray *
                                                           (kFloat_0_002_FxSpray * AsCFxSpray(this)->elapsedTime)) +
                                       FxFloat(p, 0xc));
                if (FxFloat(n, 0x10) != kFloat_0_FxSpray)
                    FxFloat(scratch, kFxOscillationAmplitudes + 8) +=
                        FxFloat(n, 0x10) *
                        Math_CosApprox(FxFloat(n, 0x1c) * (kFloat_3_1415927_FxSpray *
                                                           (kFloat_0_002_FxSpray * AsCFxSpray(this)->elapsedTime)) +
                                       FxFloat(p, 0x10));
            }
            Vec3_CopyToOutput(((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x)),
                              FxAddress(scratch, kFxOscillationForwardAndSum));
            if (AsParticleSpatial(particle)->velocity.x != kFloat_0_FxSpray ||
                (*(float*)&AsCFxParticle(particle)->velocityZ) != kFloat_0_FxSpray)
                Vec3_Cross(FxAddress(scratch, kFxOscillationSide), &kUnitYAxis,
                           FxAddress(scratch, kFxOscillationForwardAndSum));
            else {
                FxFloat(scratch, kFxOscillationSide + 4) = kFloat_0_FxSpray;
                FxFloat(scratch, kFxOscillationSide) = kFloat_0_FxSpray;
                FxFloat(scratch, kFxOscillationSide + 8) = kFloat_1_FxSpray;
            }
            Vec3_Cross(FxAddress(scratch, kFxOscillationUp), FxAddress(scratch, kFxOscillationForwardAndSum),
                       FxAddress(scratch, kFxOscillationSide));
            Vec3_Normalize(FxAddress(scratch, kFxOscillationForwardAndSum),
                           FxAddress(scratch, kFxOscillationForwardAndSum));
            Vec3_Normalize(FxAddress(scratch, kFxOscillationUp), FxAddress(scratch, kFxOscillationUp));
            Vec3_Normalize(FxAddress(scratch, kFxOscillationSide), FxAddress(scratch, kFxOscillationSide));
            Vec3_Scale(FxAddress(scratch, kFxOscillationForwardAndSum), FxAddress(scratch, kFxOscillationForwardAndSum),
                       FxFloat(scratch, kFxOscillationAmplitudes));
            Vec3_Scale(FxAddress(scratch, kFxOscillationUp), FxAddress(scratch, kFxOscillationUp),
                       FxFloat(scratch, kFxOscillationAmplitudes + 4));
            Vec3_Scale(FxAddress(scratch, kFxOscillationSide), FxAddress(scratch, kFxOscillationSide),
                       FxFloat(scratch, kFxOscillationAmplitudes + 8));
            Vec3_Add(FxAddress(scratch, kFxOscillationForwardAndSum), FxAddress(scratch, kFxOscillationForwardAndSum),
                     FxAddress(scratch, kFxOscillationUp));
            Vec3_Add(FxAddress(scratch, kFxOscillationForwardAndSum), FxAddress(scratch, kFxOscillationForwardAndSum),
                     FxAddress(scratch, kFxOscillationSide));
            Vec3_Scale(FxAddress(scratch, kFxOscillationForwardAndSum), FxAddress(scratch, kFxOscillationForwardAndSum),
                       scaledDeltaTime);
            AsParticleSpatial(particle)->oscillationOffset.x += FxFloat(scratch, kFxOscillationForwardAndSum);
            AsParticleSpatial(particle)->oscillationOffset.y += FxFloat(scratch, kFxOscillationForwardAndSum + 4);
            AsParticleSpatial(particle)->oscillationOffset.z += FxFloat(scratch, kFxOscillationForwardAndSum + 8);
        }
        if (FxInt(this, 0x354)) {
            for (Link* n = (*(Link**)&((SprayUpdateLists*)this)->sounds.sentinel.next);
                 n != (Link*)((char*)&((SprayUpdateLists*)this)->sounds.sentinel); n = n->next) {
                void* p = FxAddress(n, 8);
                if (kFloat_0_FxSpray > FxFloat(n, 0x10c))
                    Sound_PlayByName(g_soundManager, p, 0, 0, 0, 0, kFloat_0_5_FxSpray, kFloat_200_FxSpray,
                                     kFloat_1_FxSpray);
                else {
                    __mi__FRC7WVectorRC7WVector(
                        FxAddress(scratch, kFxSoundRelativePosition),
                        ((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, localPosition.x)),
                        FxAddress(FxPointer(AsCFxSpray(this)->effectOwner, 0x16c), 0x24));
                    *(WVector*)(FxAddress(scratch, kFxVolumePositionOrPhaseOrSound)) =
                        *(WVector*)(FxAddress(scratch, kFxSoundRelativePosition));
                    Sound_PlayByName(g_soundManager, p, FxAddress(scratch, kFxVolumePositionOrPhaseOrSound), 5, 0, 0,
                                     kFloat_0_FxSpray, FxFloat(p, 0x104), kFloat_1_FxSpray);
                }
            }
            Link* n = (*(Link**)&((SprayUpdateLists*)this)->sounds.sentinel.next);
            Link* head = (Link*)((char*)&((SprayUpdateLists*)this)->sounds.sentinel);
            if (n != head) {
                Link* tail = head->prev;
                n->prev->next = tail->next;
                tail->next->prev = n->prev;
                while (n != head) {
                    Link* old = n;
                    n = n->next;
                    appFreeUni__FPv(old);
                    --FxInt(this, 0x354);
                }
            }
        }
        // Particle phase F: remove emitter origin and inverse-transform world position
        // into localPosition. Oscillation remains a separate offset, not baked in here.
        Matrix_InvertAffine(FxAddress(scratch, kFxInverseParticleFrameAffine),
                            FxAddress(scratch, kFxParticleFrameAffine));
        Matrix_ExpandAffineTo4x4(FxAddress(scratch, kFxInverseParticleFrameAffine),
                                 FxAddress(scratch, kFxInverseParticleFrame44));
        Vec3_CopyToOutput(&storage.particle.worldPosition, FxAddress(scratch, kFxCommitRelativePosition));
        Vec3_CopyToOutput(&storage.particle.particleEmitterOrigin, FxAddress(scratch, kFxCommitEmitterOrigin));
        Vec3_Subtract(FxAddress(scratch, kFxCommitRelativePosition), FxAddress(scratch, kFxCommitRelativePosition),
                      FxAddress(scratch, kFxCommitEmitterOrigin));
        regeneratedThisPass = 0;
        transform(((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, localPosition.x)),
                  FxAddress(scratch, kFxInverseParticleFrame44), FxVectorBits(scratch, kFxCommitRelativePosition));
        current = current->next;
    }
    AsCFxSpray(this)->elapsedTime += deltaTime;
    return (*(int*)&AsCFxSpray(this)->particleCount) != 0 || alive != 0;
}

// ---- merged from fx_spray_particles.cpp ----
namespace ns1 {

extern "C" {

typedef Vec3f Transform3;

struct OrbitTrack {
    Transform3 pivot;
    Transform3 axis;
    int startFrame;
    float startAngularSpeed;
    int endFrame;
    float endAngularSpeed;
};
struct KeyframeNode {
    KeyframeNode* prev;
    KeyframeNode* next;
    OrbitTrack track;
};

struct ChildNode {
    ChildNode* prev;
    ChildNode* next;
    Transform3 transform;
    Transform3 direction;
};

// Timeline owner: position/velocity plus intrusive velocity, orbit, and parent-orbit track lists.
struct AnimationObject {
    char padding_00[0xB8];
    Transform3 transform;
    char padding_C4[0xC];
    Transform3 result;
    char padding_DC[0x2a8 - 0xdc];
    ChildNode* rotationTrackTail;
    ChildNode* rotationTrackHead;
    unsigned int velocityTrackCount;
    ChildNode* velocityTrackTail;
    ChildNode* children;
    unsigned int orbitTrackCount;
    KeyframeNode* orbitTrackTail;
    KeyframeNode* keys;
    unsigned int parentOrbitTrackCount;
    KeyframeNode* parentOrbitTrackTail;
    KeyframeNode* parentOrbitTrackHead;
};

extern float kFloat_neg0_5_FxSpray, kFloat_0_5_FxSpray, kFloat_2_FxSpray;
extern float kFloat_100_fxSpray, kFloat_0_0001_ParticleEmitter;
void WView_SetScreenCenter(void*, float, float);
void WView_DrawBox(void*, const void*, int, int);
void WView_DrawFacingCircle(void*, const void*, float, int, int, int);
void WView_DrawFacingCircleVec4(void*, const void*, int, int, int);
void Matrix_SetNegativeAxisAngleRotation(Transform3*, const float*, float);
void __mi__FRC7WVectorRC7WVector(Transform3*, const Transform3*, const void*);
void* Matrix_TransformPoint(Transform3*, const Transform3*, const Transform3*);
void __pl__FRC7WVectorRC7WVector(Transform3*, const Transform3*, const void*);
void __as__7WVectorFRC7WVector(Transform3*, const Transform3*);
void __apl__7WVectorFRC7WVector(Transform3*, const Transform3*);
void __ami__7WVectorFRC7WVector(Transform3*, const Transform3*);
void AABB_CopyTranslated(Transform3*, const Transform3*, const Transform3*);
void Vec3_DivideScalar(Transform3*, float, const Transform3*);
void Vec3_Zero(Transform3*);
void AABB_Copy(Transform3*, const Transform3*);
void FxSpray_ComputeScaledBounds(int, void*, void*, float, float);
void* __nw__FUl(unsigned int);
void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
int FxParticle_DrawAnimatedSprite(void*, void*, void*, const void*);

struct Matrix3 {
    unsigned int words[12];
};
struct MatrixRows {
    Transform3 a, b, c, d;
};
struct DrawNode {
    DrawNode* prev;
    DrawNode* next;
    char data[0x80];
};
struct DrawWords {
    unsigned int x, y, z;
};
struct DrawCopy {
    Transform3 value;
};

// Resolve emitter position in world space, applying its parent rotation and translation when attached.
inline Transform3& drawPosition(unsigned char* s, Transform3& result) {
    if (AsCFxSpray(s)->positionIsWorldSpace) {
        *(DrawCopy*)&result = *(DrawCopy*)((char*)s + FX_MEMBER_OFFSET(SpraySpatialView, position));
        return result;
    } else {
        unsigned char* parent = (unsigned char*)AsCFxSpray(s)->parentSequence;
        if (parent) {
            if (AsCFxSpray(s)->attachedToParent) {
                Transform3 tmp;
                Matrix_TransformPoint(&tmp, (Transform3*)((char*)s + FX_MEMBER_OFFSET(SpraySpatialView, position)),
                                      (Transform3*)((char*)s + FX_MEMBER_OFFSET(SpraySpatialView, parentRotation)));
                __pl__FRC7WVectorRC7WVector(&result, &tmp,
                                            (char*)parent + FX_MEMBER_OFFSET(CFxSequence, worldPosition));
                return result;
            } else {
                __pl__FRC7WVectorRC7WVector(&result,
                                            (Transform3*)((char*)s + FX_MEMBER_OFFSET(SpraySpatialView, position)),
                                            (char*)parent + FX_MEMBER_OFFSET(CFxSequence, worldPosition));
                return result;
            }
        }
        *(DrawCopy*)&result = *(DrawCopy*)((char*)s + FX_MEMBER_OFFSET(SpraySpatialView, position));
        return result;
    }
}

struct DrawSphere {
    Transform3 center;
    float radius;
};

} // extern "C"
} // namespace ns1

// Draw emitter debug bounds and all live particle sprites, trails and ribbons using the requested screen center.
void CFxSpray::DrawParticlesAndBounds(void* renderer) {
    using namespace ns1;
    // Phase 1: select the requested screen center and reset diagnostic draw counts.
    unsigned char* s = (unsigned char*)this;
    unsigned char* r = (unsigned char*)renderer;
    if (AsCFxSpray(s)->drawEnabled) {
        DrawSphere final;
        Transform3 offset;
        if (AsSpraySpatial(s)->screenCenterX > kFloat_neg0_5_FxSpray &&
            AsSpraySpatial(s)->screenCenterY > kFloat_neg0_5_FxSpray)
            WView_SetScreenCenter(renderer, AsSpraySpatial(s)->screenCenterX, AsSpraySpatial(s)->screenCenterY);
        else {
            float height = AsWView(r)->viewportHeight; // original +0x18C
            float width = AsWView(r)->viewportWidth;   // original +0x188
            float savedWidth = width;
            float savedHeight = height;
            WView_SetScreenCenter(renderer, kFloat_0_5_FxSpray * savedWidth, kFloat_0_5_FxSpray * savedHeight);
        }
        AsSpraySpatial(s)->spriteDrawCount = 0;
        AsSpraySpatial(s)->trailPrimitiveCount = 0;
        // Phase 2: optional debug box/spheres and gravity-point markers.
        if (((EffectOwnerRenderView*)AsCFxSpray(s)->effectOwner)->debugBoundsEnabled) {
            if (AsCFxSpray(s)->sphericalEmission == 0) {
                DrawSphere combined;

                struct {
                    Transform3 b;
                    Transform3 a;
                } matrix;
                Transform3 pos;
                AABB_CopyTranslated((Transform3*)&matrix,
                                    (Transform3*)((char*)s + FX_MEMBER_OFFSET(SpraySpatialView, volumeMinimumOrCenter)),
                                    &drawPosition(s, pos));

                WView_DrawBox(renderer, &matrix, kFxDebugBoundsColor, 0);
                Transform3 eye;
                Transform3* eyePtr = &drawPosition(s, eye);
                Transform3 middle;
                __pl__FRC7WVectorRC7WVector(
                    &middle, (Transform3*)((char*)s + FX_MEMBER_OFFSET(SpraySpatialView, volumeMinimumOrCenter)),
                    (Transform3*)((char*)s + FX_MEMBER_OFFSET(SpraySpatialView, volumeMaximumOrRadii)));
                Transform3 scaled;
                Vec3_DivideScalar(&scaled, 2.0f, &middle);

                __pl__FRC7WVectorRC7WVector(&combined.center, &scaled, eyePtr);
                WView_DrawFacingCircle(renderer, &combined, 0.5f, kFxDebugBoundsColor, kFxDebugCircleSegments, 0);
            } else {
                DrawSphere second;
                DrawSphere m;

                float radius = AsSpraySpatial(s)->volumeMaximumOrRadii.x;
                float savedRadius = radius;
                *(WVector*)(&m.center) =
                    *(WVector*)((Transform3*)((char*)s + FX_MEMBER_OFFSET(SpraySpatialView, volumeMinimumOrCenter)));
                m.radius = savedRadius;
                WView_DrawFacingCircleVec4(renderer, &m, kFxDebugBoundsColor, kFxDebugSphereSegments, 0);
                radius = AsSpraySpatial(s)->volumeMaximumOrRadii.y;

                *(WVector*)(&second.center) =
                    *(WVector*)((Transform3*)((char*)s + FX_MEMBER_OFFSET(SpraySpatialView, volumeMinimumOrCenter)));
                float savedRadius2 = radius;
                second.radius = savedRadius2;
                *(WVector*)(&m.center) = *(WVector*)(&second.center);
                float savedRadius3 = second.radius;
                m.radius = savedRadius3;
                WView_DrawFacingCircleVec4(renderer, &m, kFxDebugBoundsColor, kFxDebugSphereSegments, 0);
            }
            for (DrawNode *n = (DrawNode*)AsCFxSpray(s)->gravityPointTrackHead,
                          *end = (DrawNode*)(s + FX_MEMBER_OFFSET(CFxSpray, gravityPointTrackHead) - sizeof(void*));
                 end != n; n = n->next) {
                Transform3 pos;

                Transform3* nodePos = (Transform3*)n->data;
                Transform3 t;
                Transform3* posPtr = &drawPosition(s, pos);
                __pl__FRC7WVectorRC7WVector(&t, nodePos, posPtr);
                WView_DrawFacingCircle(renderer, &t, 0.3f, kFxDebugGravityColor, kFxDebugCircleSegments, 0);
            }
        }

        // Phase 3: reconstruct each particle's world position and draw sprite/trail/ribbon.
        // Follow-emitter mode uses today's origin/rotation; otherwise use captured ones.
        Transform3 combined;
        Transform3 result;
        for (DrawNode *n = (DrawNode*)AsCFxSpray(s)->particleListHead,
                      *end = (DrawNode*)(s + FX_MEMBER_OFFSET(CFxSpray, particleListHead) - sizeof(void*));
             end != n; n = n->next) {
            Transform3 local;
            Matrix3 m;
            Transform3 position;

            unsigned char* child = *(unsigned char**)(n->data);
            Transform3* localPtr =
                AsCFxSpray(s)->particlesFollowEmitter
                    ? &drawPosition(s, position)
                    : (Transform3*)((char*)child + FX_MEMBER_OFFSET(ParticleSpatialView, emitterPosition));
            *(DrawCopy*)&local = *(DrawCopy*)localPtr;
            Matrix3* source = (Matrix3*)(AsCFxSpray(s)->particlesFollowEmitter
                                             ? (char*)s + FX_MEMBER_OFFSET(SpraySpatialView, parentRotation)
                                             : (char*)child + FX_MEMBER_OFFSET(ParticleSpatialView, emitterRotation));

            *(MatrixRows*)&m = *(MatrixRows*)source;
            *(MatrixRows*)&m = *(MatrixRows*)source;
            m = *source;
            *(MatrixRows*)&m = *(MatrixRows*)source;
            *(MatrixRows*)&m = *(MatrixRows*)source;
            m = *source;

            unsigned char* savedChild = child + FX_MEMBER_OFFSET(ParticleSpatialView, localPosition);
            __pl__FRC7WVectorRC7WVector(
                &offset, (Transform3*)(savedChild),
                (Transform3*)((char*)child + FX_MEMBER_OFFSET(ParticleSpatialView, oscillationOffset)));
            Matrix_TransformPoint(&combined, &offset, (Transform3*)&m);

            __pl__FRC7WVectorRC7WVector(&result, &combined, &local);

            *(WVector*)((Transform3*)&final) = *(WVector*)&result;
            if (FxParticle_DrawAnimatedSprite(this, renderer, child, &final)) {
                this->DrawPositionTrail(renderer, (ns3::Stream*)child, &final);
                this->DrawRibbonTrail(renderer, (ns4::RibbonHistory*)child, (Arr3*)&final);
            }
        }
        // Phase 4: restore the default viewport midpoint after an explicit screen-center override.
        if (AsSpraySpatial(s)->screenCenterX > -0.5f) {
            if (AsSpraySpatial(s)->screenCenterY > -0.5f) {
                float height = AsWView(r)->viewportHeight;

                float half = 0.5f;
                float savedHalf = half;
                float savedHeight2 = height;
                float savedValue = savedHalf * savedHeight2;
                float savedHalf2 = half;
                WView_SetScreenCenter(renderer, (AsWView(r)->viewportWidth) * savedHalf2, savedValue);
            }
        }
    }
}

// Sample an orbital rotation track and rotate emitter position, velocity and child axes about its pivot.
void CFxSpray::ApplyEmitterOrbitTrack(float frame, float scale) {
    using namespace ns1;
    AnimationObject* self = (AnimationObject*)this;
    KeyframeNode* end;
    int lastFrame;
    KeyframeNode* node;
    int firstFrame;
    node = *(KeyframeNode**)((char*)self + FX_MEMBER_OFFSET(AnimationObject, keys));
    end = (KeyframeNode*)((char*)self + FX_MEMBER_OFFSET(AnimationObject, orbitTrackTail));
    while (node != end) {
        OrbitTrack* key = (OrbitTrack*)((char*)node + FX_MEMBER_OFFSET(KeyframeNode, track));
        firstFrame = node->track.startFrame;
        if (frame >= (float)firstFrame) {
            lastFrame = key->endFrame;
            if (frame <= (float)lastFrame) {
                int delta = lastFrame - firstFrame;
                float value;
                if (delta > 0) {
                    value = key->startAngularSpeed + (key->endAngularSpeed - key->startAngularSpeed) *
                                                         (frame - (float)firstFrame) / (float)delta;
                } else {
                    value = key->startAngularSpeed;
                }
                Matrix3 sampled;
                Matrix_SetNegativeAxisAngleRotation(
                    (Transform3*)&sampled, (float*)((char*)key + FX_MEMBER_OFFSET(OrbitTrack, axis)), value * scale);
                Transform3 original = self->transform;
                Transform3 result;
                Transform3 composed;
                Transform3 local;
                __mi__FRC7WVectorRC7WVector(&local, &self->transform, key);
                Matrix_TransformPoint(&composed, &local, (Transform3*)&sampled);
                __pl__FRC7WVectorRC7WVector(&result, &composed, key);
                *(WVector*)(&self->transform) = *(WVector*)&result;
                Transform3 result2;
                Transform3 composed2;
                Transform3 local2;
                *(WVector*)(&self->result) += *(WVector*)&original;
                __mi__FRC7WVectorRC7WVector(&local2, &self->result, key);
                Matrix_TransformPoint(&composed2, &local2, (Transform3*)&sampled);
                __pl__FRC7WVectorRC7WVector(&result2, &composed2, key);
                *(WVector*)(&self->result) = *(WVector*)&result2;
                *(WVector*)(&self->result) -= *(WVector*)(&self->transform);
                Transform3* childTransform;
                ChildNode* childEnd = (ChildNode*)((char*)self + FX_MEMBER_OFFSET(AnimationObject, velocityTrackTail));
                ChildNode* child = *(ChildNode**)((char*)self + FX_MEMBER_OFFSET(AnimationObject, children));
                for (; child != childEnd; child = child->next) {
                    childTransform = (Transform3*)((char*)child + FX_MEMBER_OFFSET(ChildNode, transform));
                    Transform3 childResult;
                    Transform3 childComposed;
                    Transform3 childLocal;
                    *(WVector*)childTransform += *(WVector*)&original;
                    __mi__FRC7WVectorRC7WVector(&childLocal, childTransform, key);
                    Matrix_TransformPoint(&childComposed, &childLocal, (Transform3*)&sampled);
                    __pl__FRC7WVectorRC7WVector(&childResult, &childComposed, key);
                    *(WVector*)childTransform = *(WVector*)&childResult;
                    *(WVector*)childTransform -= *(WVector*)(&self->transform);
                }
                break;
            }
        }
        node = node->next;
    }
}

namespace ns1 {
extern "C" {} // extern "C"
} // namespace ns1

// Sample a parent orbital track and rotate the emitter plus its dependent position and rotation tracks.
void CFxSpray::ApplyParentOrbitTrack(float frame, float scale) {
    using namespace ns1;
    AnimationObject* self = (AnimationObject*)this;
    KeyframeNode* node = *(KeyframeNode**)((char*)self + FX_MEMBER_OFFSET(AnimationObject, parentOrbitTrackHead));
    KeyframeNode* end = (KeyframeNode*)((char*)self + FX_MEMBER_OFFSET(AnimationObject, parentOrbitTrackTail));
    while (node != end) {
        OrbitTrack* key = (OrbitTrack*)((char*)node + FX_MEMBER_OFFSET(KeyframeNode, track));
        float startFrame = (float)node->track.startFrame;
        if (frame >= startFrame) {
            float endFrame = (float)key->endFrame;
            if (frame <= endFrame) {
                int delta = key->endFrame - node->track.startFrame;
                float value;
                if (delta > 0) {
                    /* orig 0x8011C2E8-0x8011C314 (r30 = node+8): key+0x1C + (key+0x24 - key+0x1C) * ... */
                    value = key->startAngularSpeed +
                            (key->endAngularSpeed - key->startAngularSpeed) * (frame - startFrame) / (float)delta;
                } else {
                    value = key->startAngularSpeed; /* orig 0x8011C31C: lfs 0x1c(r30) */
                }
                Matrix3 sampled; // 3x4 matrix written by Matrix_SetNegativeAxisAngleRotation (0x30 bytes)
                Matrix_SetNegativeAxisAngleRotation(
                    (Transform3*)&sampled, (float*)((char*)key + FX_MEMBER_OFFSET(OrbitTrack, axis)), value * scale);

                Transform3 original = self->transform;
                Transform3 local;
                Transform3 composed;
                Transform3 result;
                __mi__FRC7WVectorRC7WVector(&local, &self->transform, key);
                Matrix_TransformPoint(&composed, &local, (Transform3*)&sampled);
                __pl__FRC7WVectorRC7WVector(&result, &composed, key);
                *(WVector*)(&self->transform) = *(WVector*)&result;

                Transform3* second = (Transform3*)((char*)self + FX_MEMBER_OFFSET(AnimationObject, result));
                /* orig 0x8011C330-0x8011C350 saves self+0xB8 to sp+0xBC before the update and
                   0x8011C384-0x8011C38C adds it to self+0xD0, as CFxSpray_ApplyEmitterOrbitTrack does */
                *(WVector*)second += *(WVector*)&original;
                Transform3 secondLocal;
                Transform3 secondComposed;
                Transform3 secondResult;
                __mi__FRC7WVectorRC7WVector(&secondLocal, second, key);
                Matrix_TransformPoint(&secondComposed, &secondLocal, (Transform3*)&sampled);
                __pl__FRC7WVectorRC7WVector(&secondResult, &secondComposed, key);
                *(WVector*)second = *(WVector*)&secondResult;
                *(WVector*)second -= *(WVector*)(&self->transform);

                ChildNode* childEnd = (ChildNode*)((char*)self + FX_MEMBER_OFFSET(AnimationObject, rotationTrackTail));
                ChildNode* child = *(ChildNode**)((char*)self + FX_MEMBER_OFFSET(AnimationObject, rotationTrackHead));
                for (; child != childEnd; child = child->next) {
                    Transform3* childTransform = (Transform3*)((char*)child + FX_MEMBER_OFFSET(ChildNode, transform));
                    *(WVector*)childTransform += *(WVector*)&original; /* orig: += sp+0xBC (saved self+0xB8) */
                    Transform3 childLocal;
                    Transform3 childComposed;
                    Transform3 childResult;
                    __mi__FRC7WVectorRC7WVector(&childLocal, childTransform, key);
                    Matrix_TransformPoint(&childComposed, &childLocal, (Transform3*)&sampled);
                    __pl__FRC7WVectorRC7WVector(&childResult, &childComposed, key);
                    *(WVector*)childTransform = *(WVector*)&childResult;
                    *(WVector*)childTransform -= *(WVector*)(&self->transform);
                }

                ChildNode* listEnd = (ChildNode*)((char*)self + FX_MEMBER_OFFSET(AnimationObject, orbitTrackTail));
                ChildNode* item = *(ChildNode**)((char*)self + FX_MEMBER_OFFSET(AnimationObject, keys));
                for (; listEnd != item; item = item->next) {
                    Transform3* itemTransform = (Transform3*)((char*)item + FX_MEMBER_OFFSET(ChildNode, transform));
                    Transform3* itemSecond = (Transform3*)((char*)item + FX_MEMBER_OFFSET(ChildNode, direction));
                    *(WVector*)itemSecond += *(WVector*)itemTransform;
                    Transform3 itemLocal;
                    Transform3 itemComposed;
                    Transform3 itemResult;
                    __mi__FRC7WVectorRC7WVector(&itemLocal, itemSecond, key);
                    Matrix_TransformPoint(&itemComposed, &itemLocal, (Transform3*)&sampled);
                    __pl__FRC7WVectorRC7WVector(&itemResult, &itemComposed, key);
                    *(WVector*)itemSecond = *(WVector*)&itemResult;
                    Transform3 itemLocal2;
                    Transform3 itemComposed2;
                    Transform3 itemResult2;
                    __mi__FRC7WVectorRC7WVector(&itemLocal2, itemTransform, key);
                    Matrix_TransformPoint(&itemComposed2, &itemLocal2, (Transform3*)&sampled);
                    __pl__FRC7WVectorRC7WVector(&itemResult2, &itemComposed2, key);
                    *(WVector*)itemTransform = *(WVector*)&itemResult2;
                    *(WVector*)itemSecond -= *(WVector*)itemTransform;
                }
                break;
            }
        }
        node = node->next;
    }
}

namespace ns1 {
extern "C" {

// Inclusive time segments interpolate speed percentages (DecodeVelocityKey in fx_spray_script_parser.cpp).
struct TrackNode {
    TrackNode* prev;
    TrackNode* next;
    int startMilliseconds;
    float startSpeedPercent;
    int endMilliseconds;
    float endSpeedPercent;
};
// Schedules count emitted particles and pass spreadRadians to particle initialization
// (DecodeGenerationKey in fx_spray_script_parser.cpp); start/stop use emitter milliseconds.
struct SpawnNode {
    SpawnNode* prev;
    SpawnNode* next;
    int start;
    int stop;
    int scheduledParticleCount;
    float spreadRadians;
    int emittedCount;
    float lastScheduleMilliseconds;
};
struct ObjectNode {
    ObjectNode* prev;
    ObjectNode* next;
    void* object;
};
} // extern "C"
} // namespace ns1

// Emit the remaining particles due on each active schedule, subject to lifetime and capacity limits.
int CFxSpray::EmitScheduledParticles(float emitterMilliseconds) {
    using namespace ns1;
    unsigned char* s = (unsigned char*)this;
    // Phase 1: sample the first inclusive speed segment (total is its ending time in milliseconds).
    float sampledSpeedPercent = kFloat_100_fxSpray;
    TrackNode* n = *(TrackNode**)(s + FX_MEMBER_OFFSET(SprayTrackLists, initialSpeedPercentTrackNext));
    TrackNode* sentinel = (TrackNode*)(s + FX_MEMBER_OFFSET(SprayTrackLists, initialSpeedPercentTrackPrevious));
    for (; n != sentinel; n = n->next) {
        if (emitterMilliseconds >= (float)n->startMilliseconds) {
            if (emitterMilliseconds <= (float)n->endMilliseconds) {
                int segmentDurationMilliseconds = n->endMilliseconds - n->startMilliseconds;
                if (segmentDurationMilliseconds > 0)
                    sampledSpeedPercent =
                        n->startSpeedPercent + (n->endSpeedPercent - n->startSpeedPercent) *
                                                   (emitterMilliseconds - (float)n->startMilliseconds) /
                                                   (float)segmentDurationMilliseconds;
                else
                    sampledSpeedPercent = n->startSpeedPercent;
                break;
            }
        }
    }
    int scheduleHasWork = 0;
    // Both records are 24-byte boxes (min + max, six floats); AABB_Copy / FxSpray_ComputeScaledBounds write six
    // floats.
    Transform3 boundsScratch, emissionBounds;
    Vec3_Zero(&emissionBounds);
    Vec3_Zero(&boundsScratch);
    if (*(int*)(s + 0x1c8) == 0)
        AABB_Copy(&emissionBounds, (Transform3*)(s + 0x1a8));
    else
        FxSpray_ComputeScaledBounds(s[0x1a5], &emissionBounds, (Transform3*)(s + 0x1a8), *(float*)(s + 0x1d4),
                                    *(float*)(s + 0x1d8));
    // Phase 2: only the first unfinished schedule emits; cap against total - made.
    SpawnNode* end = (SpawnNode*)(s + FX_MEMBER_OFFSET(SprayTrackLists, spawnTrackTail));
    for (SpawnNode* n = *(SpawnNode**)(s + FX_MEMBER_OFFSET(SprayTrackLists, spawnTrackHead)); n != end; n = n->next) {
        if (emitterMilliseconds <= (float)n->stop || n->emittedCount < n->scheduledParticleCount) {
            scheduleHasWork = 1;
            if (emitterMilliseconds < (float)n->start) {
                n->lastScheduleMilliseconds = emitterMilliseconds;
                break;
            }
            // particlesDue is the cumulative scheduled count minus emitted count, not a rate.
            int particlesDue;
            if (emitterMilliseconds < (float)n->stop) {
                int scheduleDurationMilliseconds = n->stop - n->start;
                if (scheduleDurationMilliseconds > 0)
                    particlesDue = (int)(kFloat_0_0001_ParticleEmitter + (float)n->scheduledParticleCount *
                                                                             (emitterMilliseconds - (float)n->start) /
                                                                             (float)scheduleDurationMilliseconds) -
                                   n->emittedCount + 1;
                else
                    particlesDue = n->scheduledParticleCount - n->emittedCount;
            } else {
                particlesDue = n->scheduledParticleCount - n->emittedCount;
            }
            if (particlesDue > 0)
                n->lastScheduleMilliseconds = emitterMilliseconds;
            if (n->scheduledParticleCount < n->emittedCount + particlesDue)
                particlesDue = n->scheduledParticleCount - n->emittedCount;
            int i = 0;
            if (particlesDue > 0) {
                ObjectNode* list = (ObjectNode*)(s + 0x80);
                for (; i < particlesDue; ++i) {
                    // Phase 3: allocate and initialize each particle, then link its owned list node.
                    void* obj = __nw__FUl(0xf4);
                    if (obj)
                        ((CFxParticle*)obj)->__ct();
                    InitializeParticle(0, obj, &emissionBounds, 0, sampledSpeedPercent, n->spreadRadians);
                    ObjectNode* node = (ObjectNode*)appMallocDummy__FUl(12);
                    void** slot = (void**)((char*)node + 8);
                    if (slot)
                        *slot = obj;
                    list->prev->next = node;
                    node->prev = list->prev;
                    list->prev = node;
                    node->next = list;
                    ++*(int*)(s + 0x7c);
                    ++n->emittedCount;
                }
            }
            break;
        }
    }
    return scheduleHasWork;
}

// ---- merged from fx_spray_particles.cpp ----
namespace ns2 {

extern "C" {

struct U8011C878Obj {
    char pad[0x3A0];
};

struct U8011C878Out {
    char pad[0x100];
};

void* __nw__FUl(unsigned int);
void __dla__FPv(void*);
void* __nwa__FUl(unsigned int);
void FxHistoryElement_ConstructNoOp(void*);
void FxHistoryElement_DestroyNoOp(void*);
void* __construct_new_array(void*, void (*)(void*), void (*)(void*), unsigned int, int);
void __ct__8CPetBodyFPCci(void*, signed char*, int);
struct SpawnModel {
    char data[0x1b4];
    // Construct the particle-owned model body for the borrowed resource name.
    SpawnModel(signed char* name, int flag) {
        __ct__8CPetBodyFPCci(this, name, flag);
    }
};
void WPuppet_UpdateBoundsFromResourceBox(void*, int, int);
int rand();
void __as__7WVectorFRC7WVector(void*, void*);
void Vec3_Zero(void*);
void Matrix_CopyAffine(void*, void*);
void* Matrix_TransformPoint(void*, void*, void*);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void __ml__FRC7WVectorf(void*, void*, float);
void __amu__7WVectorFf(void*, float);
void Vector_CrossProduct(void*, void*, void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void Matrix_SetNegativeAxisAngleRotation(void*, void*, float);
void Matrix_InvertAffine(void*, void*);
void Matrix_ConcatAffine(void*, void*, void*);
void Matrix_FromEulerAngles(void*, void*);
void Normalize__7WVectorFv(void*);
float Vec3_DotProduct(void*, void*);
void Vec3_DivideScalar(void*, float, void*);
float Magnitude__7WVectorCFv(void*);
float __ml__FRC7WVectorRC7WVector(void*, void*);
double sin(double);
double cos(double);
double acos(double);
float Math_CosApprox(float);
float Math_SquareRootApprox(float);
float Vec2_CalculateLength(void*);
unsigned long long CursorVec2_Scale(void*, float);
void Vec2_Copy(void*, void*);
void Matrix_SetNegativeAxisAngleRotation(void*, void*, float);
void Matrix_SetIdentityAffine(void*);
void Matrix_ExtractUniformScaleNormalizedRotation(void*, void*);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
float Vec3_DotProduct(void*, void*);
float FxSpray_ComputeSpatialFalloff(void*, void*, void*, float, float, int);

extern float kFloat_0_FxSpray, kFloat_20000_RandomRange, kFloat_0_017453292_FxSpray, kFloat_0_5_FxSpray;
extern float kFloat_360_FxSpray, kFloat_2_FxSpray, kFloat_neg_1_805DE648, kFloat_1_FxSpray;
extern float kFloat_0_00001_FxSpray, kFloat_100_fxSpray, kFloat_6_2831855_FxVelocityRotation,
    kFloat_neg0_999_FxEmissionInset;
extern float kFloat_0_0001_ParticleEmitter, kFloat_neg0_0001_ParticleEmitter;

// Sample the inclusive 20001-point grid lo+(hi-lo)*n/20000; rand()%20001
// retains the original modulo bias. Units come from the supplied property range.
// Call sites pass (low, high) explicitly, with one rand() sample per invocation.
inline float randomRangeImpl(float lo, float hi) {
    return lo + ((hi - lo) * (rand() % kFxRandomSampleCount)) / kFloat_20000_RandomRange;
}
// Return absolute magnitude using the original sign-bit test.
inline float absFloat(float v) {
    return (*(unsigned int*)&v & 0x80000000) ? -v : v;
}
// Return the angle between vectors, handling near-zero lengths and clamping the cosine.
inline float vectorAngle(void* a, void* b) {
    float x = (*(WVector*)a).Magnitude();
    if (absFloat(x) < kFloat_0_00001_FxSpray)
        return kFloat_0_FxSpray;
    float y = (*(WVector*)b).Magnitude();
    if (absFloat(y) < kFloat_0_00001_FxSpray)
        return kFloat_0_FxSpray;
    float d = (*(WVector*)a * *(WVector*)b) / (x * y);
    float c;
    if (d < kFloat_neg_1_805DE648)
        c = kFloat_neg_1_805DE648;
    else if (d > kFloat_1_FxSpray)
        c = kFloat_1_FxSpray;
    else
        c = d;
    return (float)acos(c);
}

// Allocate the optional particle-owned pet body and start its first animation.
// The emitter name/resource manager and animation ranges are borrowed.
static inline void initializeParticleModel(U8011C878Obj* self, U8011C878Out* out) {
    // Phase 0: allocate an optional particle-owned model and select its first animation.
    // Resource manager/name and puppet animation records are borrowed; petBody owns the result.
    if ((*(signed char*)&AsCFxSpray(self)->petResourceName) != 0) {
        void* effectOwner = AsCFxSpray(self)->effectOwner;
        if (((CFxBox*)effectOwner)->resourceManager != 0) {
            void* modelBody =
                new SpawnModel((signed char*)((char*)self + FX_MEMBER_OFFSET(CFxSpray, petResourceName)), 1);
            AsCFxParticle(out)->petBody = modelBody;
            if (modelBody != 0) {
                void* puppet = ((CPetBody*)modelBody)->GetPuppet();
                if (puppet != 0) {
                    puppet = ((CPetBody*)(AsCFxParticle(out)->petBody))->GetPuppet();
                    void* animationRanges = ((WPuppet*)puppet)->animationRanges;
                    if (animationRanges != 0 && ((PuppetAnimationRangesView*)animationRanges)->count != 0) {
                        void* rangeRecords = ((PuppetAnimationRangesView*)animationRanges)->records;
                        void* firstRange = *(void**)rangeRecords;
                        if (firstRange != 0)
                            ((CPetBody*)(AsCFxParticle(out)->petBody))
                                ->PlayAnimationByName(kFloat_0_FxSpray,
                                                      (void*)(((PuppetAnimationNameView*)firstRange)->name), 0, 0);
                    }
                }
                (*(char*)&((CPetBody*)AsCFxParticle(out)->petBody)->defaultRenderParametersEnabled) =
                    (*(char*)&AsCFxSpray(self)->petLightingEnabled);
                modelBody = ((CPetBody*)(AsCFxParticle(out)->petBody))->GetPuppet();
                if (modelBody != 0)
                    WPuppet_UpdateBoundsFromResourceBox(modelBody, 0, 1);
            }
        }
    }
}

} // extern "C"
} // namespace ns2

// Initialize or regenerate position, velocity, orientation, lifetime, sprite/model and trail history from emitter ranges.
// out is an existing emitter-owned particle; nonzero reusePosition preserves its
// localPosition. coreBounds is a borrowed min/max or center/radii pair for falloff.
// initializing is a boolean ABI word controlling boundary placement; speedPercent
// is divided by 100, spreadAngle is radians. Returns 1 after initialization.
// Lifetime ranges are milliseconds; initial-frame ranges are indices truncated to int.
// Box coordinates sample each axis independently; spherical radius samples linearly
// between inner/outer radii (not uniform volume). Circular emission advances a degree
// angle by 360 / scheduled particle count, so that count must be nonzero.
int CFxSpray::InitializeParticle(int reusePosition, void* outPointer, void* coreBounds, int initializing,
                                 float speedPercent, float spreadAngle) {
    using namespace ns2;
    using ns2::rand;
    using ns2::sin;
    using ns2::cos;
    using ns2::acos;
    U8011C878Obj* self = (U8011C878Obj*)this;
    U8011C878Out* out = (U8011C878Out*)outPointer;
    // Phase 0: create the optional model before any random property sampling.
    initializeParticleModel(self, out);
    (*(char*)&AsParticleSpatial(out)->motionStopped) = 0;
    (*(void**)&AsCFxParticle(out)->spriteResource) = (*(void**)&AsCFxSpray(self)->spriteResource);
    if (reusePosition == 0) {
        // Phase 1: choose local emission position; regeneration can retain the old one.
        if (AsCFxSpray(self)->sphericalEmission == 0) {
            float maximumZ = AsSpraySpatial(self)->volumeMaximumOrRadii.z;
            float minimumZ = AsSpraySpatial(self)->volumeMinimumOrCenter.z;
            int randomSample = rand();
            int gridIndex = randomSample - (randomSample / kFxRandomSampleCount) * kFxRandomSampleCount;
            float sampledZ = minimumZ + ((maximumZ - minimumZ) * gridIndex) / kFloat_20000_RandomRange;
            float maximumY = AsSpraySpatial(self)->volumeMaximumOrRadii.y;
            float minimumY = AsSpraySpatial(self)->volumeMinimumOrCenter.y;
            randomSample = rand();
            gridIndex = randomSample - (randomSample / kFxRandomSampleCount) * kFxRandomSampleCount;
            float sampledY = minimumY + ((maximumY - minimumY) * gridIndex) / kFloat_20000_RandomRange;
            float maximumX = AsSpraySpatial(self)->volumeMaximumOrRadii.x;
            float minimumX = AsSpraySpatial(self)->volumeMinimumOrCenter.x;
            randomSample = rand();
            gridIndex = randomSample - (randomSample / kFxRandomSampleCount) * kFxRandomSampleCount;
            float sampledX = minimumX + ((maximumX - minimumX) * gridIndex) / kFloat_20000_RandomRange;
            float emissionPosition[3] = {sampledX, sampledY, sampledZ};
            __as__7WVectorFRC7WVector((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, localPosition.x),
                                      emissionPosition);
        } else if (AsCFxSpray(self)->circularEmission) {
            float angle = AsCFxSpray(self)->emissionAngle;
            float radius = AsSpraySpatial(self)->volumeMaximumOrRadii.x;
            int count = 0;
            void* end = (char*)self + FX_MEMBER_OFFSET(SprayTrackLists, spawnTrackTail);
            for (void* n = (*(void**)&AsCFxSpray(self)->spawnTrackHead); n != end; n = ((FxTrackLink*)n)->next)
                count += ((SpawnScheduleNode*)n)->particleCount;
            AsParticleSpatial(out)->localPosition.x =
                randomRangeImpl(kFloat_0_FxSpray, AsSpraySpatial(self)->volumeMaximumOrRadii.z) -
                kFloat_0_5_FxSpray * AsSpraySpatial(self)->volumeMaximumOrRadii.z;
            AsCFxParticle(out)->positionY = radius * (float)sin(kFloat_0_017453292_FxSpray * angle);
            AsCFxParticle(out)->positionZ = radius * (float)cos(kFloat_0_017453292_FxSpray * angle);
            AsCFxSpray(self)->emissionAngle += kFloat_360_FxSpray / count;
        } else {
            float angularSpanRadians = AsSpraySpatial(self)->volumeMaximumOrRadii.z;
            float outerRadius = AsSpraySpatial(self)->volumeMaximumOrRadii.y;
            float innerRadius = AsSpraySpatial(self)->volumeMaximumOrRadii.x;
            float xy[2], z, length;
            do {
                xy[0] = randomRangeImpl(kFloat_neg_1_805DE648, kFloat_1_FxSpray);
                xy[1] = randomRangeImpl(kFloat_neg_1_805DE648, kFloat_1_FxSpray);
                float minimumDirectionY = Math_CosApprox(kFloat_0_5_FxSpray * angularSpanRadians);
                z = randomRangeImpl(minimumDirectionY, kFloat_1_FxSpray);
                length = Vec2_CalculateLength(xy);
            } while (kFloat_0_00001_FxSpray > absFloat(length));
            unsigned long long scaled = CursorVec2_Scale(xy, Math_SquareRootApprox(kFloat_1_FxSpray - z * z) / length);
            Vec2_Copy(xy, &scaled);
            float direction[3] = {xy[0], z, xy[1]}, offset[3], result[3];
            __ml__FRC7WVectorf(offset, direction, randomRangeImpl(innerRadius, outerRadius));
            __pl__FRC7WVectorRC7WVector(
                result, (char*)self + FX_MEMBER_OFFSET(SpraySpatialView, volumeMinimumOrCenter.x), offset);
            __as__7WVectorFRC7WVector((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, localPosition.x), result);
        }
    }
    // Phase 2: snapshot emitter origin/rotation and sample velocity before orientation masks.
    Vec3_Zero((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, oscillationOffset.x));
    unsigned int position[3];
    if ((*(int*)&AsCFxSpray(self)->positionIsWorldSpace)) {
        position[0] = (unsigned int)(*(int*)&AsSpraySpatial(self)->position.x);
        position[1] = (unsigned int)(*(int*)&AsSpraySpatial(self)->position.y);
        position[2] = (unsigned int)(*(int*)&AsSpraySpatial(self)->position.z);
    } else {
        void* parent = (*(void**)&AsCFxSpray(self)->parentSequence);
        if (parent) {
            if (AsCFxSpray(self)->attachedToParent) {
                float temp[3];
                Matrix_TransformPoint(temp, (char*)self + FX_MEMBER_OFFSET(SpraySpatialView, position.x),
                                      (char*)self + FX_MEMBER_OFFSET(SpraySpatialView, parentRotation));
                __pl__FRC7WVectorRC7WVector(position, temp,
                                            (char*)parent + FX_MEMBER_OFFSET(CFxSequence, worldPosition));
            } else
                __pl__FRC7WVectorRC7WVector(position, (char*)self + FX_MEMBER_OFFSET(SpraySpatialView, position.x),
                                            (char*)parent + FX_MEMBER_OFFSET(CFxSequence, worldPosition));
        } else {
            position[0] = (unsigned int)(*(int*)&AsSpraySpatial(self)->position.x);
            position[1] = (unsigned int)(*(int*)&AsSpraySpatial(self)->position.y);
            position[2] = (unsigned int)(*(int*)&AsSpraySpatial(self)->position.z);
        }
    }
    __as__7WVectorFRC7WVector((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, emitterPosition.x), position);
    Matrix_CopyAffine((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, emitterRotation),
                      (char*)self + FX_MEMBER_OFFSET(SpraySpatialView, parentRotation));
    (*(char*)&AsParticleSpatial(out)->onGround) = 0;
    float velocity[3];
    float vz = randomRangeImpl(AsSpraySpatial(self)->minimumVelocity.z, AsSpraySpatial(self)->maximumVelocity.z);
    float vy = randomRangeImpl(AsSpraySpatial(self)->minimumVelocity.y, AsSpraySpatial(self)->maximumVelocity.y);
    float vx = randomRangeImpl(AsSpraySpatial(self)->minimumVelocity.x, AsSpraySpatial(self)->maximumVelocity.x);
    velocity[0] = vx;
    velocity[1] = vy;
    velocity[2] = vz;
    __amu__7WVectorFf(velocity, speedPercent / kFloat_100_fxSpray);
    __as__7WVectorFRC7WVector((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x), velocity);
    // Phase 2b: velocity -> spread rotation -> random spin -> emitter velocityRotation.
    // Sampling order is Z, Y, X, then spread, then spin; changing order changes RNG behavior.
    float axis[3];
    if (kFloat_0_FxSpray == velocity[0] && kFloat_0_FxSpray == velocity[2]) {
        axis[0] = kFloat_0_FxSpray;
        axis[1] = kFloat_0_FxSpray;
        axis[2] = kFloat_1_FxSpray;
    } else {
        float up[3] = {kFloat_0_FxSpray, kFloat_1_FxSpray, kFloat_0_FxSpray};
        Vector_CrossProduct(axis, velocity, up);
    }
    float spread = randomRangeImpl(kFloat_0_FxSpray, spreadAngle);
    float delta[3], matrix[12], rotated[3];
    __mi__FRC7WVectorRC7WVector(delta, axis, (char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, localPosition.x));
    Matrix_SetNegativeAxisAngleRotation(matrix, delta, spread);
    Matrix_TransformPoint(rotated, (char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x), matrix);
    __as__7WVectorFRC7WVector((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x), rotated);
    Matrix_SetNegativeAxisAngleRotation(matrix, velocity,
                                        randomRangeImpl(kFloat_0_FxSpray, kFloat_6_2831855_FxVelocityRotation));
    Matrix_TransformPoint(rotated, (char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x), matrix);
    __as__7WVectorFRC7WVector((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x), rotated);
    Matrix_TransformPoint(rotated, (char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x),
                          (char*)self + FX_MEMBER_OFFSET(SpraySpatialView, velocityRotation));
    __as__7WVectorFRC7WVector((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x), rotated);
    AsCFxParticle(out)->forceScale = randomRangeImpl(AsCFxSpray(self)->minWindScale, AsCFxSpray(self)->maxWindScale);
    float zero[3] = {kFloat_0_FxSpray, kFloat_0_FxSpray, kFloat_0_FxSpray};
    __as__7WVectorFRC7WVector((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, initialVector.x), zero);
    // Phase 2c: compose source rotations, then apply the external-orientation mask.
    // sourceRotation is the normalized SOURCE frame here; inversion happens inside branches.
    // boundaryDirection below actually holds a velocity-plus-wind direction in the frame
    // required by boundary placement. It does not include the emitter acceleration.
    float sourceProduct[12], sourceRotation[12];
    Matrix_SetIdentityAffine(sourceProduct);
    for (int j = 0; j < kFxParentTransformCount; ++j) {
        void* transform = *(void**)((char*)self + (kFxSourceTransformsOffset + j * kFxAbiWordBytes));
        if (!transform)
            break;
        float m[12], product[12];
        Matrix_ExtractUniformScaleNormalizedRotation(transform, m);
        Matrix_ConcatAffine(product, sourceProduct, m);
        Matrix_CopyAffine(sourceProduct, product);
    }
    Matrix_ExtractUniformScaleNormalizedRotation(sourceProduct, sourceRotation);
    float boundaryDirection[3];
    if ((*(int*)&AsCFxSpray(self)->externalOrientationAxisMask)) {
        if ((*(int*)&AsCFxSpray(self)->externalOrientationAxisMask) == kFxAllAxes) {
            float inv[12], force[3], transformed[3], sum[3], v[3];
            Matrix_InvertAffine(inv, sourceRotation);
            __ml__FRC7WVectorf(force, (char*)self + FX_MEMBER_OFFSET(SpraySpatialView, windVector.x),
                               AsCFxParticle(out)->forceScale);
            Matrix_TransformPoint(transformed, force, inv);
            __pl__FRC7WVectorRC7WVector(sum, (char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x),
                                        transformed);
            __as__7WVectorFRC7WVector(boundaryDirection, sum);
            Matrix_TransformPoint(v, (char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x), sourceRotation);
            __as__7WVectorFRC7WVector((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x), v);
        } else {
            float angles[3], basis[3], projected[3];
            Vec3_Zero(angles);
            if ((*(int*)&AsCFxSpray(self)->externalOrientationAxisMask) & kFxAxisX) {
                float up[3] = {kFloat_0_FxSpray, kFloat_1_FxSpray, kFloat_0_FxSpray}, v[3];
                __as__7WVectorFRC7WVector(basis, up);
                Matrix_TransformPoint(v, basis, sourceRotation);
                __as__7WVectorFRC7WVector(projected, v);
                projected[0] = kFloat_0_FxSpray;
                angles[0] = vectorAngle(basis, projected);
                if (!(absFloat(projected[2]) < kFloat_0_00001_FxSpray))
                    angles[0] *= projected[2] < kFloat_0_FxSpray ? kFloat_neg_1_805DE648 : kFloat_1_FxSpray;
            }
            if ((*(int*)&AsCFxSpray(self)->externalOrientationAxisMask) & kFxAxisY) {
                float forward[3] = {kFloat_0_FxSpray, kFloat_0_FxSpray, kFloat_1_FxSpray}, v[3];
                __as__7WVectorFRC7WVector(basis, forward);
                Matrix_TransformPoint(v, basis, sourceRotation);
                __as__7WVectorFRC7WVector(projected, v);
                projected[1] = kFloat_0_FxSpray;
                angles[1] = vectorAngle(basis, projected);
                if (!(absFloat(projected[0]) < kFloat_0_00001_FxSpray))
                    angles[1] *= projected[0] < kFloat_0_FxSpray ? kFloat_neg_1_805DE648 : kFloat_1_FxSpray;
            }
            if ((*(int*)&AsCFxSpray(self)->externalOrientationAxisMask) & kFxAxisZ) {
                float right[3] = {kFloat_1_FxSpray, kFloat_0_FxSpray, kFloat_0_FxSpray}, v[3];
                __as__7WVectorFRC7WVector(basis, right);
                Matrix_TransformPoint(v, basis, sourceRotation);
                __as__7WVectorFRC7WVector(projected, v);
                projected[2] = kFloat_0_FxSpray;
                angles[2] = vectorAngle(basis, projected);
                if (!(absFloat(projected[1]) < kFloat_0_00001_FxSpray))
                    angles[2] *= projected[1] > kFloat_0_FxSpray ? kFloat_neg_1_805DE648 : kFloat_1_FxSpray;
            }
            float rotation[12], invrot[12], partial[12], invworld[12];
            float euler[3] = {angles[0], angles[1], angles[2]};
            Matrix_FromEulerAngles(rotation, euler);
            Matrix_InvertAffine(invrot, rotation);
            Matrix_CopyAffine(partial, invrot);
            Matrix_InvertAffine(invworld, sourceRotation);
            float force[3], v[3], sum[3], result[3], newvel[3];
            __ml__FRC7WVectorf(force, (char*)self + FX_MEMBER_OFFSET(SpraySpatialView, windVector.x),
                               AsCFxParticle(out)->forceScale);
            Matrix_TransformPoint(v, (char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x), partial);
            __pl__FRC7WVectorRC7WVector(sum, v, force);
            Matrix_TransformPoint(result, sum, invworld);
            __as__7WVectorFRC7WVector(boundaryDirection, result);
            Matrix_TransformPoint(newvel, (char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x), partial);
            __as__7WVectorFRC7WVector((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x), newvel);
        }
    } else {
        float inv[12], force[3], sum[3], result[3];
        Matrix_InvertAffine(inv, sourceRotation);
        __ml__FRC7WVectorf(force, (char*)self + FX_MEMBER_OFFSET(SpraySpatialView, windVector.x),
                           AsCFxParticle(out)->forceScale);
        __pl__FRC7WVectorRC7WVector(sum, (char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, velocity.x), force);
        Matrix_TransformPoint(result, sum, inv);
        __as__7WVectorFRC7WVector(boundaryDirection, result);
    }
    // Phase 2d: move an initializing particle to the upstream volume boundary.
    // Depends on sampled position, adjusted velocity/wind, source frame and emitter
    // acceleration. Reuse can retain the sampled position but this placement still runs.
    if (initializing && !(absFloat(boundaryDirection[0]) < kFloat_0_00001_FxSpray &&
                          absFloat(boundaryDirection[1]) < kFloat_0_00001_FxSpray &&
                          absFloat(boundaryDirection[2]) < kFloat_0_00001_FxSpray &&
                          absFloat(AsSpraySpatial(self)->acceleration.x) < kFloat_0_00001_FxSpray &&
                          absFloat(AsSpraySpatial(self)->acceleration.y) < kFloat_0_00001_FxSpray &&
                          absFloat(AsSpraySpatial(self)->acceleration.z) < kFloat_0_00001_FxSpray)) {
        if (AsCFxSpray(self)->sphericalEmission) {
            float dir[3], a[3], b[3], c[3];
            __mi__FRC7WVectorRC7WVector(dir, (char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, localPosition.x),
                                        (char*)self + FX_MEMBER_OFFSET(SpraySpatialView, volumeMinimumOrCenter.x));
            Normalize__7WVectorFv(dir);
            __ml__FRC7WVectorf(a, dir, AsSpraySpatial(self)->volumeMaximumOrRadii.y);
            __ml__FRC7WVectorf(b, a, kFloat_neg0_999_FxEmissionInset);
            __pl__FRC7WVectorRC7WVector(c, (char*)self + FX_MEMBER_OFFSET(SpraySpatialView, volumeMinimumOrCenter.x),
                                        b);
            __as__7WVectorFRC7WVector((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, localPosition.x), c);
        } else {
            float dir[3];
            if (!(absFloat(boundaryDirection[0]) < kFloat_0_00001_FxSpray &&
                  absFloat(boundaryDirection[1]) < kFloat_0_00001_FxSpray &&
                  absFloat(boundaryDirection[2]) < kFloat_0_00001_FxSpray))
                __as__7WVectorFRC7WVector(dir, boundaryDirection);
            else
                __as__7WVectorFRC7WVector(dir, (char*)self + FX_MEMBER_OFFSET(SpraySpatialView, acceleration.x));
            float planes[kFxBoxPlaneCount][4] = {
                {kFloat_0_FxSpray, kFloat_0_FxSpray, kFloat_1_FxSpray, -AsSpraySpatial(self)->volumeMinimumOrCenter.z},
                {kFloat_0_FxSpray, kFloat_0_FxSpray, kFloat_neg_1_805DE648,
                 AsSpraySpatial(self)->volumeMaximumOrRadii.z},
                {kFloat_0_FxSpray, kFloat_1_FxSpray, kFloat_0_FxSpray, -AsSpraySpatial(self)->volumeMinimumOrCenter.y},
                {kFloat_0_FxSpray, kFloat_neg_1_805DE648, kFloat_0_FxSpray,
                 AsSpraySpatial(self)->volumeMaximumOrRadii.y},
                {kFloat_1_FxSpray, kFloat_0_FxSpray, kFloat_0_FxSpray, -AsSpraySpatial(self)->volumeMinimumOrCenter.x},
                {kFloat_neg_1_805DE648, kFloat_0_FxSpray, kFloat_0_FxSpray,
                 AsSpraySpatial(self)->volumeMaximumOrRadii.x}};
            float points[kFxBoxPlaneCount][3] = {
                {AsSpraySpatial(self)->volumeMinimumOrCenter.x, AsSpraySpatial(self)->volumeMinimumOrCenter.y,
                 AsSpraySpatial(self)->volumeMinimumOrCenter.z},
                {AsSpraySpatial(self)->volumeMaximumOrRadii.x, AsSpraySpatial(self)->volumeMinimumOrCenter.y,
                 AsSpraySpatial(self)->volumeMaximumOrRadii.z},
                {AsSpraySpatial(self)->volumeMaximumOrRadii.x, AsSpraySpatial(self)->volumeMinimumOrCenter.y,
                 AsSpraySpatial(self)->volumeMaximumOrRadii.z},
                {AsSpraySpatial(self)->volumeMinimumOrCenter.x, AsSpraySpatial(self)->volumeMaximumOrRadii.y,
                 AsSpraySpatial(self)->volumeMinimumOrCenter.z},
                {AsSpraySpatial(self)->volumeMinimumOrCenter.x, AsSpraySpatial(self)->volumeMinimumOrCenter.y,
                 AsSpraySpatial(self)->volumeMaximumOrRadii.z},
                {AsSpraySpatial(self)->volumeMaximumOrRadii.x, AsSpraySpatial(self)->volumeMinimumOrCenter.y,
                 AsSpraySpatial(self)->volumeMinimumOrCenter.z}};
            int mask = 0;
            int first = 1;
            if (dir[0] > kFloat_0_0001_ParticleEmitter)
                mask |= kFxMinXFace;
            else if (dir[0] < kFloat_neg0_0001_ParticleEmitter)
                mask |= kFxMaxXFace;
            if (dir[2] > kFloat_0_0001_ParticleEmitter)
                mask |= kFxMinZFace;
            else if (dir[2] < kFloat_neg0_0001_ParticleEmitter)
                mask |= kFxMaxZFace;
            if (dir[1] > kFloat_0_0001_ParticleEmitter)
                mask |= kFxMinYFace;
            else if (dir[1] < kFloat_neg0_0001_ParticleEmitter)
                mask |= kFxMaxYFace;
            float* plane = planes[0];
            float* point = points[0];
            for (int j = 0; j < kFxBoxPlaneCount; ++j, point += 3, plane += 4) {
                if (!(mask & (1 << j)))
                    continue;
                if (!first) {
                    float d[3];
                    __mi__FRC7WVectorRC7WVector(d, (char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, localPosition.x),
                                                point);
                    if (Vec3_DotProduct(d, plane) > kFloat_0_FxSpray)
                        continue;
                }
                float distance =
                    Vec3_DotProduct((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, localPosition.x), plane) +
                    plane[3];
                float denom = Vec3_DotProduct(dir, plane);
                float a[3], b[3], c[3], d[3];
                __ml__FRC7WVectorf(a, dir, distance);
                ((void (*)(void*, void*, float))Vec3_DivideScalar)(b, a, denom);
                __ml__FRC7WVectorf(c, b, kFloat_neg_1_805DE648);
                __pl__FRC7WVectorRC7WVector(d, (char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, localPosition.x), c);
                __as__7WVectorFRC7WVector((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, localPosition.x), d);
                first = 0;
            }
        }
    }
    // Phase 2e: sample angular velocity and initial rotation independently (Z/Y/X),
    // then transform both through velocityRotation before visual-property sampling.
    float angularVelocityZ = randomRangeImpl(((SprayInitialRotationRanges*)self)->minimumAngularVelocity.z,
                                             ((SprayInitialRotationRanges*)self)->maximumAngularVelocity.z);
    float angularVelocityY = randomRangeImpl(((SprayInitialRotationRanges*)self)->minimumAngularVelocity.y,
                                             ((SprayInitialRotationRanges*)self)->maximumAngularVelocity.y);
    float angularVelocityX = randomRangeImpl(((SprayInitialRotationRanges*)self)->minimumAngularVelocity.x,
                                             ((SprayInitialRotationRanges*)self)->maximumAngularVelocity.x);
    float sampledAngularVelocity[3] = {angularVelocityX, angularVelocityY, angularVelocityZ},
          transformedAngularVelocity[3];
    Matrix_TransformPoint(transformedAngularVelocity, sampledAngularVelocity,
                          (char*)self + FX_MEMBER_OFFSET(SpraySpatialView, velocityRotation));
    __as__7WVectorFRC7WVector((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, angularVelocity.x),
                              transformedAngularVelocity);
    float initialRotationZ = randomRangeImpl(((SprayInitialRotationRanges*)self)->minimumRotation.z,
                                             ((SprayInitialRotationRanges*)self)->maximumRotation.z);
    float initialRotationY = randomRangeImpl(((SprayInitialRotationRanges*)self)->minimumRotation.y,
                                             ((SprayInitialRotationRanges*)self)->maximumRotation.y);
    float initialRotationX = randomRangeImpl(((SprayInitialRotationRanges*)self)->minimumRotation.x,
                                             ((SprayInitialRotationRanges*)self)->maximumRotation.x);
    float sampledRotation[3] = {initialRotationX, initialRotationY, initialRotationZ}, transformedRotation[3];
    Matrix_TransformPoint(transformedRotation, sampledRotation,
                          (char*)self + FX_MEMBER_OFFSET(SpraySpatialView, velocityRotation));
    __as__7WVectorFRC7WVector((char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, rotation.x), transformedRotation);
    // Phase 3: initialize visual properties, age and frame from independent range samples.
    AsParticleSpatial(out)->size = AsParticleSpatial(out)->initialSize =
        randomRangeImpl(AsCFxSpray(self)->minInitialSize, AsCFxSpray(self)->maxInitialSize);
    AsParticleSpatial(out)->aspectRatio = AsCFxSpray(self)->spriteAspectRatio;
    (*(int*)&AsParticleSpatial(out)->color) = -1;
    void* end = (char*)self + FX_MEMBER_OFFSET(SprayTrackLists, colorTrackTail);
    for (void* n = (*(void**)&AsCFxSpray(self)->colorTrackHead); n != end; n = ((FxTrackLink*)n)->next) {
        if (kFloat_0_FxSpray == ((ParticleColorTrack*)n)->startPercent) {
            (*(char*)&AsParticleSpatial(out)->color) = ((ParticleColorTrack*)n)->startColor[0];
            ((char*)&AsParticleSpatial(out)->color)[1] = ((ParticleColorTrack*)n)->startColor[1];
            ((char*)&AsParticleSpatial(out)->color)[2] = ((ParticleColorTrack*)n)->startColor[2];
            ((char*)&AsParticleSpatial(out)->color)[3] = ((ParticleColorTrack*)n)->startColor[3];
            break;
        }
    }
    AsCFxParticle(out)->opacityScale = FxSpray_ComputeSpatialFalloff(
        (char*)out + FX_MEMBER_OFFSET(ParticleSpatialView, localPosition.x),
        (char*)self + FX_MEMBER_OFFSET(SpraySpatialView, volumeMinimumOrCenter.x), coreBounds,
        AsCFxSpray(self)->volumeCoreScaleX, AsCFxSpray(self)->volumeCoreScaleY, AsCFxSpray(self)->sphericalEmission);
    float scale =
        randomRangeImpl((float)(*(int*)&AsCFxSpray(self)->minLifetime), (float)(*(int*)&AsCFxSpray(self)->maxLifetime));
    AsParticleSpatial(out)->initialLifetime = scale;
    AsParticleSpatial(out)->remainingLifetime = scale;
    float life = randomRangeImpl((float)(*(int*)&AsCFxSpray(self)->minInitialFrame),
                                 (float)(*(int*)&AsCFxSpray(self)->maxInitialFrame));
    AsParticleSpatial(out)->lastFrameChangeTime = kFloat_0_FxSpray;
    AsParticleSpatial(out)->lastTrailSampleTime = kFloat_0_FxSpray;
    AsParticleSpatial(out)->spriteFrame = (int)life;
    // Phase 4: replace owned history arrays, reset ring indices and child scheduling.
    // Array data follows a 0x10-byte allocator header; deletion subtracts that header.
    if ((*(int*)&AsCFxSpray(self)->trailSegmentCount) > 0) {
        int count = (*(int*)&AsCFxSpray(self)->trailSegmentCount) + 1;
        if (count >= 2) {
            void* old = (*(void**)&AsCFxParticle(out)->positionHistory);
            if (old) {
                if (old)
                    __dla__FPv((char*)old - kFxHistoryHeaderBytes);
            }
            void* array = __nwa__FUl(count * kFxPositionSampleBytes + kFxHistoryHeaderBytes);
            if (array)
                array = __construct_new_array(array, FxHistoryElement_ConstructNoOp, 0, kFxPositionSampleBytes, count);
            (*(void**)&AsCFxParticle(out)->positionHistory) = array;
            (*(int*)&AsCFxParticle(out)->positionHistoryCapacity) = count;
            (*(int*)&AsCFxParticle(out)->positionHistoryWriteIndex) = 0;
            (*(int*)&AsCFxParticle(out)->positionHistoryReadIndex) = 0;
        }
        count = (*(int*)&AsCFxSpray(self)->trailSegmentCount) + 1;
        if (count >= 2) {
            void* old = (*(void**)&AsCFxParticle(out)->ribbonHistory);
            if (old) {
                if (old)
                    __dla__FPv((char*)old - kFxHistoryHeaderBytes);
            }
            void* array = __nwa__FUl(count * kFxRibbonSampleBytes + kFxHistoryHeaderBytes);
            if (array)
                array = __construct_new_array(array, FxHistoryElement_DestroyNoOp, 0, kFxRibbonSampleBytes, count);
            (*(void**)&AsCFxParticle(out)->ribbonHistory) = array;
            (*(int*)&AsCFxParticle(out)->ribbonHistoryCapacity) = count;
            (*(int*)&AsCFxParticle(out)->ribbonHistoryWriteIndex) = 0;
            (*(int*)&AsCFxParticle(out)->ribbonHistoryReadIndex) = 0;
        }
    }
    AsParticleSpatial(out)->childSpawnCount = 0;
    AsParticleSpatial(out)->lastChildSpawnTime = AsCFxSpray(self)->elapsedTime;
    return 1;
}

// ---- merged from fx_spray_particles.cpp ----
namespace ns3 {

extern "C" {

// No-op element constructor used by particle-history array allocation.
void FxHistoryElement_ConstructNoOp() {}

// No-op element destructor used by particle-history array cleanup.
void FxHistoryElement_DestroyNoOp() {}

typedef Vec3f Vec3;
struct Entry {
    Vec3 a;
    Vec3 b;
    float value;
};
// The first ring stores 16-byte position/frame samples; the second stores 28-byte ribbon edges.
struct Stream {
    char pad0[0x74];
    Vec3 velocity;
    char pad80[0x44];
    Entry* lineSamples;
    unsigned int lineReadIndex;
    int lineWriteIndex;
    unsigned int lineCapacity;
    Entry* ribbonSamples;
    unsigned int ribbonReadIndex;
    int ribbonWriteIndex;
    unsigned int ribbonCapacity;
    float lastSampleTime;
};
struct Owner {
    char pad0[0x74];
    Vec3 position;
    char pad80[0x214 - 0x80];
    int trailSampleLimit;
    float sampleInterval;
    char pad21C[4];
    int ribbonTexture;
    float trailWidthScale;
    char pad228;
    unsigned char cameraSideRibbon;
};
struct Node {
    Vec3 pos;
    Vec3 dir;
    float speed;
};
struct Mat {
    char data[12];
};
extern float kFloat_0_0001_ParticleEmitter, kFloat_2_FxSpray, kFloat_0_FxSpray, kFloat_1_FxSpray, kFloat_200_FxSpray;
extern float Magnitude__7WVectorCFv(void*);
extern void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern void Vec3_DivideScalar(void*, float, const void*);
extern void __as__7WVectorFRC7WVector(void*, const void*);
extern void Vector_CrossProduct(void*, const void*, const void*);
extern void* Normalize__7WVectorFv(void*);
extern void __ml__FRC7WVectorf(void*, const void*, float);
extern void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern float Vec3_DotProduct(void*, const void*);
extern int ComputeDistanceFadedEffectColor(void*, void*, void*, void*);
extern void Vec3_Subtract(void*, const void*, const void*);
extern void Vec3_Normalize(void*, const void*);
extern float Vec3_Dot(const void*, const void*);
extern float Vec3_LengthSquared(const void*);
extern void Vec3_Scale(void*, const void*, float);
extern void Vec3_Add(void*, const void*, const void*);
extern void W3dAniSpr_SetPosition(void*, const void*);
extern void W3dAniSpr_SetColor(void*, int);
extern void W3dAniSpr_SetSizeAndPivot(void*, float, float, float, float);
extern void W3dAniSpr_DrawFrame(void*, void*, float, int, int);
extern void Matrix_FromEulerAngles(void*, const void*);
extern void Matrix_ConcatAffine(void*, const void*, const void*);
extern void W3dAniSpr_DrawFrameWithMatrix(void*, void*, const void*, int, int);
extern void Engine_SetCachedRenderState(int, int);
extern float kFloat_0_6_FxSpriteFacingThreshold, kFloat_0_01_FxSpray, kFloat_0_5_FxSpray;
extern CameraTransform800D s_cameraTransform;
struct MatrixRow {
    unsigned int data[3];
};
struct Matrix34 {
    MatrixRow a, b, c, d;
};
struct MatrixWords {
    unsigned int data[12];
};
struct RenderObject {
    virtual void v00();
    virtual void v04();
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2C();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3C();
    virtual void v40();
    virtual void v44();
    virtual void v48();
    virtual void v4C();
    virtual void v50();
    virtual void v54();
    virtual void v58();
    virtual void v5C();
    virtual void v60();
    virtual void v64();
    virtual void render(int, int);
};

// Submitted vertices use RGBA bytes; frame samples are reinterpreted separately before submission.
struct Vertex {
    Vec3 pos;
    unsigned char r, g, b, a;
};
// The union's 32-bit word holds .r in the most significant byte (memory byte 0 of the big-endian word); .word is
// the word ComputeDistanceFadedEffectColor takes and returns.
union ColorWord {
    struct {
        unsigned char r, g, b, a;
    } c;
    unsigned int word;
};
struct RenderStates {
    unsigned int words[30];
};
struct TextureStates {
    unsigned int words[12];
};
struct SamplerStates {
    unsigned int words[10];
};
struct StateTemplates {
    RenderStates render;
    TextureStates texture;
    SamplerStates sampler;
};
extern void Vector_CopyComponents(void*, int);
extern void Engine_SetRenderStateList(void*, int);
extern void Engine_SetTextureStageStateList(int, const void*, int);
extern void Engine_SetTextureStageParameterList(int, const void*, int);
extern void* g_view;
extern int Camera_GetRenderState(void*);
extern void SetTransform(int, const void*);
extern void Engine_SetFlexibleVertexFormat(int);
extern void Engine_SetType7Resource(int);
extern void DrawPrimitiveUP(int, int, const void*, int);
extern StateTemplates kEffectTrailRenderStateTemplates;
extern unsigned char kIdentityMatrix4x4[];

} // extern "C"
} // namespace ns3

// Build a camera-transformed line trail with interpolated endpoint colors and distance-faded alpha.
void CFxSpray::DrawPositionTrail(void* arg, ns3::Stream* stream, const void* position) {
    using namespace ns3;
    Vertex vertices[5];
    int available;
    // Phase 1: measure live ring samples and find the last written world-space sample.

    StateTemplates* states = &kEffectTrailRenderStateTemplates;
    unsigned int savedCount = stream->lineReadIndex;
    int count = savedCount;
    int savedHead = stream->lineWriteIndex;
    int head = savedHead;
    if (head < count)
        available = head + (stream->lineCapacity - count);
    else
        available = head - count;
    if (1 > available)
        return;
    ++AsSpraySpatial(this)->trailPrimitiveCount;
    count = stream->lineReadIndex;
    int savedHead2 = stream->lineWriteIndex;
    int rows;
    head = savedHead2;
    if (head < count)
        rows = head + (stream->lineCapacity - count);
    else
        rows = head - count;
    Vertex* finalEntry;
    if (head - 1 < 0) {
        Entry* savedEntries = stream->lineSamples;
        unsigned int savedCapacity = stream->lineCapacity;
        unsigned int index = savedCapacity;
        unsigned int savedIndex = index;
        index = head + savedIndex;
        finalEntry = (Vertex*)savedEntries + (index - 1);
    } else
        finalEntry = (Vertex*)stream->lineSamples + (head - 1);
    // Phase 2a: a sprite resource draws one color-interpolated sprite per history sample.
    if ((*(int*)&AsCFxParticle(stream)->spriteResource)) {

        unsigned char* src = (unsigned char*)stream + FX_MEMBER_OFFSET(ParticleSpatialView, color);
        unsigned char* dst = (unsigned char*)this + FX_MEMBER_OFFSET(CFxSpray, trailEndAlpha);
        int n = rows - 1;
        ColorWord c;
        c.c.r = dst[0] / rows + (src[0] * n) / rows;
        c.c.g = dst[1] / rows + (src[1] * n) / rows;
        int savedNode = n;
        c.c.b = dst[2] / rows + (src[2] * savedNode) / rows;
        int savedNode2 = n;
        c.c.a = dst[3] / rows + (src[3] * savedNode2) / rows;
        int col = ((int (*)(void*, void*, void*, void*, unsigned int))ComputeDistanceFadedEffectColor)(
            this, arg, finalEntry, stream, c.word);
        if (col & 0xFF000000) {
            W3dAniSpr_SetColor((*(void**)&AsCFxParticle(stream)->spriteResource), col);
            W3dAniSpr_SetPosition((*(void**)&AsCFxParticle(stream)->spriteResource), finalEntry);

            ((void (*)(void*, void*, float, int, int))W3dAniSpr_DrawFrame)(
                (*(void**)&AsCFxParticle(stream)->spriteResource), arg, AsParticleSpatial(stream)->rotation.x,
                ((EffectOwnerRenderView*)AsCFxSpray(this)->effectOwner)->renderFlags |
                    ((*(int*)&AsCFxSpray(this)->trailBlendMode) | 0x00400000),
                ((TrailFrameSample*)finalEntry)->spriteFrame);
        }
        for (int i = rows - 2; i >= 0; i--) {
            ++AsSpraySpatial(this)->trailPrimitiveCount;
            unsigned int savedI = stream->lineReadIndex + i;

            ColorWord blend;
            Vertex* e = (Vertex*)stream->lineSamples + ((savedI) % stream->lineCapacity);

            int weight = rows - i;
            blend.c.r = src[0] * i / rows + dst[0] * weight / rows;
            int savedI2 = i;
            int savedValue2 = src[1] * savedI2;
            int savedI3 = i;
            blend.c.g = savedValue2 / rows + dst[1] * weight / rows;
            int savedWeight = weight;
            blend.c.b = (src[2] * savedI3) / rows + dst[2] * savedWeight / rows;
            blend.c.a = (src[3] * i) / rows + dst[3] * weight / rows;
            col = ((int (*)(void*, void*, void*, void*, unsigned int))ComputeDistanceFadedEffectColor)(
                this, arg, e, stream, blend.word);
            if (col & 0xFF000000) {
                int savedCol = col;
                W3dAniSpr_SetColor((*(void**)&AsCFxParticle(stream)->spriteResource), savedCol);
                W3dAniSpr_SetPosition((*(void**)&AsCFxParticle(stream)->spriteResource), e);

                ((void (*)(void*, void*, float, int, int))W3dAniSpr_DrawFrame)(
                    (*(void**)&AsCFxParticle(stream)->spriteResource), arg, AsParticleSpatial(stream)->rotation.x,
                    ((EffectOwnerRenderView*)AsCFxSpray(this)->effectOwner)->renderFlags |
                        ((*(int*)&AsCFxSpray(this)->trailBlendMode) | 0x00400000),
                    ((TrailFrameSample*)e)->spriteFrame);
            }
        }
        return;
    }
    // Phase 2b: without a sprite, emit current position plus up to four history points.
    if (rows > 4)
        rows = 4;
    if (rows < 1)
        rows = 1;

    Vector_CopyComponents(vertices, (int)position);
    unsigned int col = ((int (*)(void*, void*, void*, void*, unsigned int))ComputeDistanceFadedEffectColor)(
        this, arg, finalEntry, stream, FX_COLOR_WORD_FIELD(AsParticleSpatial(stream)->color));
    vertices[0].a = col >> 24;
    vertices[0].r = col >> 16;
    vertices[0].g = col >> 8;
    vertices[0].b = col;
    Vertex* out = (((vertices + 1)));
    int i = 1;
    while (rows >= i) {
        int weight = rows - i;
        ColorWord blend;

        unsigned int savedCapacity2 = stream->lineCapacity;
        Vertex* e = (Vertex*)stream->lineSamples + ((stream->lineReadIndex + weight) % savedCapacity2);
        unsigned char* src = (unsigned char*)stream + FX_MEMBER_OFFSET(ParticleSpatialView, color);
        unsigned char* dst = (unsigned char*)this + FX_MEMBER_OFFSET(CFxSpray, trailEndAlpha);

        int savedWeight2 = src[0] * weight;
        blend.c.r = savedWeight2 / rows + dst[0] * i / rows;
        int savedRows = rows;
        blend.c.g = src[1] * weight / savedRows + dst[1] * i / savedRows;
        int savedWeight3 = src[2] * weight;
        blend.c.b = savedWeight3 / rows + dst[2] * i / rows;
        int savedRows2 = rows;
        blend.c.a = (src[3] * weight) / savedRows2 + dst[3] * i / savedRows2;
        Vector_CopyComponents(out, (int)e);
        unsigned int col = ((int (*)(void*, void*, void*, void*, unsigned int))ComputeDistanceFadedEffectColor)(
            this, arg, e, stream, blend.word);
        out->a = col >> 24;
        out->r = col >> 16;
        out->g = col >> 8;
        i = i + 1;
        out->b = col;
        out += 1;
    }
    AsSpraySpatial(this)->trailPrimitiveCount = (rows + AsSpraySpatial(this)->trailPrimitiveCount) - 1;
    // Phase 3: submit world-space vertices through camera view/projection, identity model.
    RenderStates render = states->render;
    SamplerStates sampler;
    TextureStates texture;
    texture = states->texture;
    sampler = states->sampler;
    Engine_SetRenderStateList(&render, 15);
    Engine_SetTextureStageStateList(0, &texture, 6);
    Engine_SetTextureStageParameterList(0, &sampler, 5);
    char* device = (char*)Camera_GetRenderState(g_view);
    SetTransform(0, device + 0x20);
    SetTransform(1, device + 0x60);
    Engine_SetFlexibleVertexFormat(kFxTrailVertexFormat);
    Engine_SetType7Resource(0);
    SetTransform(11, kIdentityMatrix4x4);
    Engine_SetCachedRenderState(kFxSourceBlendState, kFxBlendSourceAlpha);
    Engine_SetCachedRenderState(kFxDestinationBlendState, kFxBlendInverseSourceAlpha);
    Engine_SetCachedRenderState(5, 0);
    DrawPrimitiveUP(kFxLineStrip, rows, vertices, sizeof(Vertex));
}

namespace ns3 {
extern "C" {

// Cull transparent/back-facing sprites, apply camera offset and blend mode, then draw the sprite or pet model.
int FxParticle_DrawAnimatedSprite(Owner* self, void* view, Stream* particle, const void* worldPosition) {
    if ((*(int*)&AsCFxParticle(particle)->spriteResource)) {
        int fadedColor = ((int (*)(void*, void*, const void*, void*, void*))ComputeDistanceFadedEffectColor)(
            self, view, worldPosition, particle, FX_COLOR_PTR_FIELD(AsParticleSpatial(particle)->color));
        if (!(fadedColor & 0xFF000000))
            return 0;
        Vec3 xform, rotation, temp;
        Matrix34 output, basis, matrix;
        Vec3_Subtract(&xform, worldPosition, s_cameraTransform.position);
        Vec3_Normalize(&rotation, &xform);
        float facingCosine = Vec3_Dot(&rotation, &s_cameraTransform.basis[6]);
        if (facingCosine <= kFloat_0_6_FxSpriteFacingThreshold)
            return 0;
        if (kFloat_0_FxSpray != AsCFxSpray(self)->cameraOffset) {
            float square = Vec3_LengthSquared(&xform);
            float offset = AsCFxSpray(self)->cameraOffset;
            float nearClipRayDistance = AsWView(view)->nearClip / facingCosine;
            if (square < (offset + nearClipRayDistance) * (offset + nearClipRayDistance)) {
                Vec3_Scale(&temp, &rotation, kFloat_0_01_FxSpray + nearClipRayDistance);
                Vec3_Add(&temp, &temp, s_cameraTransform.position);
            } else {
                Vec3_Scale(&temp, &rotation, offset);
                Vec3_Subtract(&temp, worldPosition, &temp);
            }
            Vec3 vector;
            float x, y, z;
            z = temp.z;
            y = temp.y;
            x = temp.x;
            vector.y = y;
            vector.x = x;
            vector.z = z;
            W3dAniSpr_SetPosition((*(void**)&AsCFxParticle(particle)->spriteResource), &vector);
        } else
            W3dAniSpr_SetPosition((*(void**)&AsCFxParticle(particle)->spriteResource), worldPosition);
        W3dAniSpr_SetColor((*(void**)&AsCFxParticle(particle)->spriteResource), fadedColor);
        float scale = AsParticleSpatial(particle)->size, width = AsParticleSpatial(particle)->aspectRatio;
        float product = scale * width;
        W3dAniSpr_SetSizeAndPivot((*(void**)&AsCFxParticle(particle)->spriteResource), product, scale,
                                  product * kFloat_0_5_FxSpray, scale * kFloat_0_5_FxSpray);
        if (AsCFxSpray(self)->billboardEnabled) {
            ((void (*)(void*, void*, float, int, int))W3dAniSpr_DrawFrame)(
                (*(void**)&AsCFxParticle(particle)->spriteResource), view, AsParticleSpatial(particle)->rotation.x,
                ((EffectOwnerRenderView*)AsCFxSpray(self)->effectOwner)->renderFlags |
                    ((*(int*)&AsCFxSpray(self)->spriteBlendMode) | kFxBillboardDrawFlags),
                AsParticleSpatial(particle)->spriteFrame);
        } else {
            Matrix34* data =
                AsCFxSpray(self)->particlesFollowEmitter
                    ? (Matrix34*)((char*)self + FX_MEMBER_OFFSET(SpraySpatialView, parentRotation))
                    : (Matrix34*)((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, emitterRotation));
            matrix.a = data->a;
            matrix.b = data->b;
            matrix.c = data->c;
            matrix.d = data->d;
            matrix.a = data->a;
            matrix.b = data->b;
            matrix.c = data->c;
            matrix.d = data->d;
            *(MatrixWords*)&matrix = *(MatrixWords*)data;
            matrix.a = data->a;
            matrix.b = data->b;
            matrix.c = data->c;
            matrix.d = data->d;
            matrix.a = data->a;
            matrix.b = data->b;
            matrix.c = data->c;
            matrix.d = data->d;
            *(MatrixWords*)&matrix = *(MatrixWords*)data;
            Vec3 vector = *(Vec3*)((char*)particle + FX_MEMBER_OFFSET(ParticleSpatialView, rotation.x));
            Matrix_FromEulerAngles(&basis, &vector);
            Matrix_ConcatAffine(&output, &basis, &matrix);
            W3dAniSpr_DrawFrameWithMatrix((*(void**)&AsCFxParticle(particle)->spriteResource), view, &output,
                                          ((EffectOwnerRenderView*)AsCFxSpray(self)->effectOwner)->renderFlags |
                                              ((*(int*)&AsCFxSpray(self)->spriteBlendMode) | kFxSpriteDrawFlags),
                                          AsParticleSpatial(particle)->spriteFrame);
        }
    } else if ((*(int*)&AsCFxParticle(particle)->petBody)) {
        int mode = (*(int*)&AsCFxSpray(self)->spriteBlendMode);
        switch (mode) {
        case kFxBlendAlpha:
            Engine_SetCachedRenderState(kFxSourceBlendState, kFxBlendSourceAlpha);
            Engine_SetCachedRenderState(kFxDestinationBlendState, kFxBlendInverseSourceAlpha);
            break;
        case kFxBlendAdditive:
            Engine_SetCachedRenderState(kFxSourceBlendState, kFxBlendSourceAlpha);
            Engine_SetCachedRenderState(kFxDestinationBlendState, kFxBlendOne);
            break;
        case kFxBlendMultiply:
            Engine_SetCachedRenderState(kFxSourceBlendState, kFxBlendZero);
            Engine_SetCachedRenderState(kFxDestinationBlendState, kFxBlendSourceColor);
            break;
        case kFxBlendAlternate:
            Engine_SetCachedRenderState(kFxSourceBlendState, kFxBlendZero);
            Engine_SetCachedRenderState(kFxDestinationBlendState, kFxBlendInverseSourceColor);
            break;
        }
        (*(RenderObject**)((char*)particle + FX_MEMBER_OFFSET(CFxParticle, petBody)))->render(0, 1);
    }
    return 1;
}

} // extern "C"
} // namespace ns3

// Append timed position or ribbon-edge samples to circular histories, deriving ribbon width from motion.
void CFxSpray::AppendTrailSample(ns3::Stream* particle, const ns3::Mat* worldPosition, float sampleTime) {
    using namespace ns3;
    // Histories borrow worldPosition for this call and copy its data. Times are emitter
    // milliseconds. Capacities are allocated as trailSegmentCount+1; one slot separates
    // read/write cursors, and overflow advances the oldest sample rather than reallocating.
    if (this->trailSegmentCount) {
        if (sampleTime - particle->lastSampleTime < this->trailSampleInterval)
            return;
        particle->lastSampleTime = sampleTime;
        if (this->resourceId) {
            // Ribbon phase 1: choose a width vector perpendicular to velocity and either
            // camera forward or world up. Near-zero motion reuses the previous edge span.
            Vec3 halfWidth;
            MatrixRow savedFirstEdge;
            Vec3 previousHalfSpan, previousSpan, cameraPerpendicular, upPerpendicular, worldUp, scaledHalfWidth,
                widthScaledPerpendicular, sizeScaledPerpendicular, firstEdge, secondEdge, currentSpan, previousEdgeSpan,
                centerDisplacement, previousCenter, previousEdgeSum;
            Entry* current;
            Entry* previous;
            if (*(volatile int*)&particle->ribbonWriteIndex < 0)
                current = particle->ribbonSamples + (particle->ribbonCapacity + particle->ribbonWriteIndex);
            else
                current = particle->ribbonSamples + particle->ribbonWriteIndex;
            if (particle->ribbonWriteIndex - 1 < 0) {
                int index = particle->ribbonWriteIndex;
                index = particle->ribbonCapacity + index;
                previous = particle->ribbonSamples + --index;
            } else
                previous = particle->ribbonSamples + (particle->ribbonWriteIndex - 1);
            if (Magnitude__7WVectorCFv(&particle->velocity) < kFloat_0_0001_ParticleEmitter &&
                particle->ribbonSamples + (particle->ribbonReadIndex % particle->ribbonCapacity) != current) {
                __mi__FRC7WVectorRC7WVector(&previousSpan, previous, &previous->b);
                Vec3_DivideScalar(&previousHalfSpan, kFloat_2_FxSpray, &previousSpan);
                __as__7WVectorFRC7WVector(&halfWidth, &previousHalfSpan);
            } else {
                void* effect = *(void**)((char*)this->effectOwner + 0x16c);
                if (effect && this->trailCameraSideEnabled) {
                    Vector_CrossProduct(&cameraPerpendicular,
                                        (char*)effect + FX_MEMBER_OFFSET(CameraTransform800D, basis[6]),
                                        &particle->velocity);
                    __as__7WVectorFRC7WVector(&halfWidth, &cameraPerpendicular);
                } else {
                    worldUp.x = kFloat_0_FxSpray;
                    worldUp.y = kFloat_1_FxSpray;
                    worldUp.z = kFloat_0_FxSpray;
                    Vector_CrossProduct(&upPerpendicular, &worldUp, &particle->velocity);
                    __as__7WVectorFRC7WVector(&halfWidth, &upPerpendicular);
                }
                __ml__FRC7WVectorf(&sizeScaledPerpendicular, Normalize__7WVectorFv(&halfWidth),
                                   AsParticleSpatial(particle)->size);
                __ml__FRC7WVectorf(&widthScaledPerpendicular, &sizeScaledPerpendicular, this->trailWidthScale);
                Vec3_DivideScalar(&scaledHalfWidth, kFloat_200_FxSpray, &widthScaledPerpendicular);
                __as__7WVectorFRC7WVector(&halfWidth, &scaledHalfWidth);
            }
            // Ribbon phase 2: store worldPosition +/- half-width. Swap edges if their
            // span reverses relative to the previous sample, preventing strip twists.
            // value stores distance between sample centers for later texture-U spacing.
            __pl__FRC7WVectorRC7WVector(&firstEdge, worldPosition, &halfWidth);
            __as__7WVectorFRC7WVector(current, &firstEdge);
            __mi__FRC7WVectorRC7WVector(&secondEdge, worldPosition, &halfWidth);
            __as__7WVectorFRC7WVector(&current->b, &secondEdge);
            if (particle->ribbonSamples + (particle->ribbonReadIndex % particle->ribbonCapacity) == current)
                current->value = kFloat_0_FxSpray;
            else {
                __mi__FRC7WVectorRC7WVector(&previousEdgeSpan, previous, &previous->b);
                __mi__FRC7WVectorRC7WVector(&currentSpan, current, &current->b);
                if (Vec3_DotProduct(&currentSpan, &previousEdgeSpan) < kFloat_0_FxSpray) {
                    savedFirstEdge = *(MatrixRow*)current;
                    __as__7WVectorFRC7WVector(current, &current->b);
                    __as__7WVectorFRC7WVector(&current->b, &savedFirstEdge);
                }
                __pl__FRC7WVectorRC7WVector(&previousEdgeSum, previous, &previous->b);
                Vec3_DivideScalar(&previousCenter, kFloat_2_FxSpray, &previousEdgeSum);
                __mi__FRC7WVectorRC7WVector(&centerDisplacement, worldPosition, &previousCenter);
                current->value = Magnitude__7WVectorCFv(&centerDisplacement);
            }
            {
                // Ribbon phase 3: make room by advancing read, then advance write if allowed.
                int writeIndex = (*(int*)&AsCFxParticle(particle)->ribbonHistoryWriteIndex);
                int readIndex = (*(int*)&AsCFxParticle(particle)->ribbonHistoryReadIndex);
                int ringFull;
                do {
                    if (readIndex > writeIndex) {
                        int d = readIndex - writeIndex;
                        if (d > 1) {
                            ringFull = 0;
                            break;
                        }
                    } else {
                        int d = writeIndex - readIndex;
                        if ((int)particle->ribbonCapacity > d + 1) {
                            ringFull = 0;
                            break;
                        }
                    }
                    ringFull = 1;
                } while (0);
                if (ringFull) {
                    int storedDistance;
                    if (writeIndex < readIndex)
                        storedDistance = writeIndex + (particle->ribbonCapacity - readIndex);
                    else
                        storedDistance = writeIndex - readIndex;
                    if (storedDistance >= 1) {
                        particle->ribbonReadIndex = (particle->ribbonReadIndex + 1) % particle->ribbonCapacity;
                    }
                }
            }
            {
                int writeIndex = (*(int*)&AsCFxParticle(particle)->ribbonHistoryWriteIndex);
                int readIndex = (*(int*)&AsCFxParticle(particle)->ribbonHistoryReadIndex);
                int ringFull;
                do {
                    if (readIndex > writeIndex) {
                        int d = readIndex - writeIndex;
                        if (d > 1) {
                            ringFull = 0;
                            break;
                        }
                    } else {
                        int d = writeIndex - readIndex;
                        if ((int)particle->ribbonCapacity > d + 1) {
                            ringFull = 0;
                            break;
                        }
                    }
                    ringFull = 1;
                } while (0);
                if (!ringFull) {
                    particle->ribbonWriteIndex = (particle->ribbonWriteIndex + 1) % particle->ribbonCapacity;
                }
            }
        } else {
            // Position-only phase: store world position/frame, then apply the same ring policy.
            char* n;
            if (*(volatile int*)&particle->lineWriteIndex < 0)
                n = (char*)particle->lineSamples + (particle->lineCapacity + particle->lineWriteIndex) * 16;
            else
                n = (char*)particle->lineSamples + particle->lineWriteIndex * 16;
            __as__7WVectorFRC7WVector(n, worldPosition);
            ((TrailFrameSample*)n)->spriteFrame = AsParticleSpatial(particle)->spriteFrame;
            {
                int writeIndex = (*(int*)&AsCFxParticle(particle)->positionHistoryWriteIndex);
                int readIndex = (*(int*)&AsCFxParticle(particle)->positionHistoryReadIndex);
                int ringFull;
                do {
                    if (readIndex > writeIndex) {
                        if (readIndex - writeIndex > 1) {
                            ringFull = 0;
                            break;
                        }
                    } else {
                        if ((int)particle->lineCapacity > writeIndex - readIndex + 1) {
                            ringFull = 0;
                            break;
                        }
                    }
                    ringFull = 1;
                } while (0);
                if (ringFull) {
                    int distance;
                    if (writeIndex < readIndex)
                        distance = writeIndex + (particle->lineCapacity - readIndex);
                    else
                        distance = writeIndex - readIndex;
                    if (distance >= 1)
                        particle->lineReadIndex = (particle->lineReadIndex + 1) % particle->lineCapacity;
                }
            }
            {
                int writeIndex = (*(int*)&AsCFxParticle(particle)->positionHistoryWriteIndex);
                int readIndex = (*(int*)&AsCFxParticle(particle)->positionHistoryReadIndex);
                int ringFull;
                do {
                    if (readIndex > writeIndex) {
                        if (readIndex - writeIndex > 1) {
                            ringFull = 0;
                            break;
                        }
                    } else {
                        if ((int)particle->lineCapacity > writeIndex - readIndex + 1) {
                            ringFull = 0;
                            break;
                        }
                    }
                    ringFull = 1;
                } while (0);
                if (!ringFull)
                    particle->lineWriteIndex = (particle->lineWriteIndex + 1) % particle->lineCapacity;
            }
        }
    }
}

// ---- merged from fx_spray_particles.cpp ----
namespace ns4 {

extern "C" {

struct RibbonRenderStates {
    unsigned data[30];
};
struct RibbonTextureStates {
    unsigned data[12];
};
struct RibbonSamplerStates {
    unsigned data[10];
};
struct RibbonStateTemplates {
    RibbonRenderStates a;
    RibbonTextureStates b;
    RibbonSamplerStates c;
};

extern float kFloat_6_2831855;
extern float s_fxGalleryTwoPi;
extern float kFloat_0_FxSpray;
extern float kFloat_0_Gallery;
extern char __vt__8CGallery[];
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void* ObjectList_FindById(void*, void*);
extern void __ct__18NativeBase80201D18Fv(void*);
extern void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern void Vec3_DivideScalar(void*, float, void*);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern float Magnitude__7WVectorCFv(void*);
extern void __ml__FRC7WVectorf(void*, void*, float);
extern void Engine_SetRenderStateList(void*, int);
extern void Engine_SetTextureStageStateList(int, void*, int);
extern void Engine_SetTextureStageParameterList(int, void*, int);
extern void* Camera_GetRenderState(void*);
extern void SetTransform(int, void*);
extern void Engine_SetType7Resource(int);
extern void Engine_SetTextureStageResource(int, void*);
extern void Engine_SetTexturePalette(int, void*);
extern void Engine_SetTextureStageState(int, int, int);
extern void Engine_SetTextureStageParameter(int, int, int);
extern void Engine_SetCachedRenderState(int, int);
extern unsigned GX_GetPrimitiveCount(int, int);
extern unsigned ComputeDistanceFadedEffectColor(void*, void*, void*, void*, unsigned);
extern RibbonStateTemplates kEffectTrailRenderStateTemplates;
extern unsigned kIdentityMatrix4x4[];
extern void* g_view;
extern float kFloat_0_5_FxSpray;
extern float kFloat_1_FxSpray;
extern float kFloat_2_FxSpray;

// No-op lifecycle hook retained by the effect unit.
void FxSpray_NoOpLifecycleHook() {}

struct RibbonRenderer {
    unsigned char count;
    // PowerPC passes a 12-byte struct by reference.
    virtual void dummy();
    virtual void reset();
    virtual unsigned char resize(unsigned char);
    virtual void color(unsigned char, unsigned char, unsigned char, unsigned char, unsigned char);
    virtual void position(unsigned char, Arr3);
    virtual void uv(unsigned char, float, float);
    virtual void draw(int, unsigned);
    // Return the current number of writable ribbon vertices.
    unsigned getCount() {
        return count;
    }
    // Resize the renderer only when its cached vertex count differs.
    void size(unsigned char n) {
        if (count != n)
            count = resize(n);
    }
    // Write ARGB bytes to vertex i only when it is in range.
    void setColor(unsigned char i, unsigned c) {
        if (getCount() > i)
            color(i, c >> 24, c >> 16, c >> 8, c);
    }
    // Write the two texture coordinates for an existing vertex.
    void setUV(unsigned char i, float u, const float& v) {
        if (getCount() > i)
            uv(i, u, v);
    }
    // Write a three-component position for an existing ribbon vertex.
    void setPosition(unsigned char i, Arr3 p) {
        if (getCount() > i)
            position(i, p);
    }
    // Submit the current nonempty vertex array as a triangle strip.
    void submit() {
        if (count) {
            unsigned primitiveCount = GX_GetPrimitiveCount(count, kFxTriangleStrip);
            draw(kFxTriangleStrip, primitiveCount);
        }
    }
};
struct RibbonSample {
    Arr3 left, right;
    float distanceFromPreviousCenter;
};
// Read distance from the previous sample center, in world units (the +0x18 scalar).
static inline float ribbonSegmentDistance(const RibbonSample* sample) {
    return sample->distanceFromPreviousCenter;
}
// Borrowed view of CFxParticle +D4/+D8/+DC/+E0: sample buffer, read cursor,
// write cursor (writeIndex), capacity. Empty means read==write; one slot stays
// unused so occupied=(write-read+capacity)%capacity. Samples are copied world-space
// edge pairs; renderer reads newest at (write-1+capacity)%capacity.
struct RibbonHistory {
    char padding_00[0xB8];
    unsigned char colors[4];
    char padding_BC[0x18];
    RibbonSample* vertices;
    unsigned start, writeIndex, capacity; // writeIndex is the next-write cursor, not occupancy.
    // Return ring distance from start to writeIndex.
    int size() {
        return (int)writeIndex < (int)start ? (int)writeIndex + ((int)capacity - (int)start)
                                            : (int)writeIndex - (int)start;
    }
    // Borrow the logical ith sample, wrapping through the history ring.
    RibbonSample* at(unsigned i) {
        return vertices + ((start + i) % capacity);
    }
};
struct RibbonEmitter {
    char padding_00[0xB4];
    unsigned calls;
    char padding_B8[0x148];
    unsigned flags;
    char padding_204[0x18];
    unsigned char color[4];
    unsigned texture;
    char padding_224[4];
    unsigned char textured;
    char padding_229[0x143];
    RibbonRenderer render;
};
struct RibbonColor {
    unsigned char c[4];
};

// Return a reference to the smaller integer color value; consume it before temporaries expire.
inline const int& minimumRibbonCount(const int& b, const int& a) {
    return a < b ? a : b;
}

// Blend the current ribbon cap toward history color using integer channel rules.
// Alpha/blue/green divide each full weighted sum once. Red is handled separately
// by the caller because its two divisions and capped result follow a different rule.
static inline void blendRibbonCapColor(RibbonColor& rgb, RibbonEmitter* self, RibbonHistory* history, int sampleCount) {
    rgb.c[3] = (self->color[3] + history->colors[3] * (sampleCount - 1)) / sampleCount;
    int savedCount2 = sampleCount;
    rgb.c[2] = (self->color[2] + history->colors[2] * (savedCount2 - 1)) / savedCount2;
    rgb.c[1] = (self->color[1] + history->colors[1] * (sampleCount - 1)) / sampleCount;
}

} // extern "C"
} // namespace ns4

// Draw a ribbon strip from circular left/right edge history, optionally assigning texture coordinates.
void CFxSpray::DrawRibbonTrail(void* arg1, ns4::RibbonHistory* history, Arr3* currentWorldPosition) {
    using namespace ns4;
    // Phase 1: measure accumulated center-to-center distance and build the current cap
    // at currentWorldPosition +/- the last sample's half-span. All edge positions are world space.
    RibbonStateTemplates* tables = &kEffectTrailRenderStateTemplates;
    if (history->size() < 1)
        return;
    float totalCenterDistance = kFloat_0_FxSpray;
    int sampleCount = history->size();
    for (int i = 1; i < sampleCount; ++i) {
        RibbonSample* v = history->vertices + ((*(volatile unsigned*)&history->start + i) % history->capacity);
        if (v)
            totalCenterDistance += ribbonSegmentDistance(v);
    }
    ++((RibbonEmitter*)this)->calls;
    RibbonSample* last;
    last = ((int)history->writeIndex - 1 < 0)
               ? history->vertices + ((int)history->capacity + (int)history->writeIndex - 1)
               : history->vertices + ((int)history->writeIndex - 1);
    Arr3 right;
    Arr3 left;
    Arr3 currentCenterDelta;
    Arr3 lastCenter;
    Arr3 lastEdgeSum;
    Arr3 currentLeftEdge;
    Arr3 leftHalfSpan;
    Arr3 leftSpan;
    Arr3 currentRightEdge;
    Arr3 rightHalfSpan;
    Arr3 rightSpan;
    __pl__FRC7WVectorRC7WVector(&lastEdgeSum, &last->left, &last->right);
    Vec3_DivideScalar(&lastCenter, kFloat_2_FxSpray, &lastEdgeSum);
    __mi__FRC7WVectorRC7WVector(&currentCenterDelta, currentWorldPosition, &lastCenter);
    float distanceFromCurrent = Magnitude__7WVectorCFv(&currentCenterDelta);
    float savedLength = distanceFromCurrent;
    totalCenterDistance += savedLength;
    __mi__FRC7WVectorRC7WVector(&leftSpan, &last->left, &last->right);
    __ml__FRC7WVectorf(&leftHalfSpan, &leftSpan, kFloat_0_5_FxSpray);
    __pl__FRC7WVectorRC7WVector(&currentLeftEdge, currentWorldPosition, &leftHalfSpan);
    ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(&left, &currentLeftEdge);
    __mi__FRC7WVectorRC7WVector(&rightSpan, &last->left, &last->right);
    __ml__FRC7WVectorf(&rightHalfSpan, &rightSpan, kFloat_0_5_FxSpray);
    __mi__FRC7WVectorRC7WVector(&currentRightEdge, currentWorldPosition, &rightHalfSpan);
    ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(&right, &currentRightEdge);
    RibbonColor rgb;
    blendRibbonCapColor(rgb, ((RibbonEmitter*)this), history, sampleCount);
    // Red truncates each weighted term before addition, then cannot exceed history red.
    rgb.c[0] = minimumRibbonCount(history->colors[0], ((RibbonEmitter*)this)->color[0] / sampleCount +
                                                          history->colors[0] * (sampleCount - 1) / sampleCount);
    // Phase 2: configure view/projection, texture and alpha/additive/multiply blending.
    RibbonRenderStates saved = tables->a;
    RibbonSamplerStates saved5;
    RibbonTextureStates saved6;
    RibbonTextureStates* table6 = &tables->b;
    RibbonSamplerStates* table5 = &tables->c;
    saved6 = *table6;
    saved5 = *table5;
    Engine_SetRenderStateList(&saved, 15);
    Engine_SetTextureStageStateList(0, &saved6, 6);
    Engine_SetTextureStageParameterList(0, &saved5, 5);
    void* renderer = Camera_GetRenderState(g_view);
    SetTransform(0, (char*)renderer + FX_MEMBER_OFFSET(EffectRenderStateView, view));
    SetTransform(1, (char*)renderer + FX_MEMBER_OFFSET(EffectRenderStateView, projection));
    Engine_SetType7Resource(0);
    ((RibbonEmitter*)this)->render.reset();
    void* device = AsWView(g_view)->renderTarget;
    unsigned texture = ((RibbonEmitter*)this)->texture;
    Engine_SetTextureStageResource(0, ((void* (*)(void*, unsigned))(*(void***)device)[28])(device, texture));
    device = AsWView(g_view)->renderTarget;
    Engine_SetTexturePalette(0, ((void* (*)(void*, unsigned))(*(void***)device)[29])(device, texture));
    SetTransform(2, kIdentityMatrix4x4);
    SetTransform(11, kIdentityMatrix4x4);
    Engine_SetTextureStageState(0, 0, 0);
    Engine_SetTextureStageState(0, 3, 0);
    Engine_SetTextureStageParameter(0, 0, 1);
    Engine_SetTextureStageParameter(0, 1, 1);
    switch ((int)((RibbonEmitter*)this)->flags) {
    case kFxBlendAlpha:
        Engine_SetCachedRenderState(kFxSourceBlendState, kFxBlendSourceAlpha);
        Engine_SetCachedRenderState(kFxDestinationBlendState, kFxBlendInverseSourceAlpha);
        break;
    case kFxBlendAdditive:
        Engine_SetCachedRenderState(kFxSourceBlendState, kFxBlendSourceAlpha);
        Engine_SetCachedRenderState(kFxDestinationBlendState, kFxBlendOne);
        break;
    case kFxBlendMultiply:
        Engine_SetCachedRenderState(kFxSourceBlendState, kFxBlendZero);
        Engine_SetCachedRenderState(kFxDestinationBlendState, kFxBlendSourceColor);
        break;
    case kFxBlendAlternate:
        Engine_SetCachedRenderState(kFxSourceBlendState, kFxBlendZero);
        Engine_SetCachedRenderState(kFxDestinationBlendState, kFxBlendInverseSourceColor);
        break;
    }
    Engine_SetCachedRenderState(5, 0);
    // Phase 3: draw a four-vertex cap joining the current position to the newest sample.
    // Degenerate total distance: cap U falls back to 1; history U falls back to 0.
    // This preserves a collapsed strip without dividing by zero.
    ((RibbonEmitter*)this)->render.size(4);
    float u = kFloat_0_FxSpray;
    unsigned color1 = ComputeDistanceFadedEffectColor(((RibbonEmitter*)this), arg1, &right, history,
                                                      FX_COLOR_WORD_BYTES(history->colors));
    unsigned color2 = ComputeDistanceFadedEffectColor(((RibbonEmitter*)this), arg1, &left, history,
                                                      FX_COLOR_WORD_BYTES(history->colors));
    ((RibbonEmitter*)this)->render.setColor(0, color1);
    ((RibbonEmitter*)this)->render.setUV(0, u, kFloat_0_FxSpray);
    ((RibbonEmitter*)this)->render.setPosition(0, *(Arr3*)(&right));
    ((RibbonEmitter*)this)->render.setColor(1, color2);
    ((RibbonEmitter*)this)->render.setUV(1, u, kFloat_1_FxSpray);
    ((RibbonEmitter*)this)->render.setPosition(1, *(Arr3*)(&left));
    u = ((RibbonEmitter*)this)->textured && totalCenterDistance != kFloat_0_FxSpray
            ? distanceFromCurrent / totalCenterDistance
            : kFloat_1_FxSpray;
    color1 =
        ComputeDistanceFadedEffectColor(((RibbonEmitter*)this), arg1, &last->right, history, FX_COLOR_WORD_BYTES(&rgb));
    color2 = ComputeDistanceFadedEffectColor(((RibbonEmitter*)this), arg1, last, history, FX_COLOR_WORD_BYTES(&rgb));
    ((RibbonEmitter*)this)->render.setColor(2, color1);
    ((RibbonEmitter*)this)->render.setUV(2, u, kFloat_0_FxSpray);
    ((RibbonEmitter*)this)->render.setPosition(2, *(Arr3*)(&last->right));
    ((RibbonEmitter*)this)->render.setColor(3, color2);
    float savedU = u;
    ((RibbonEmitter*)this)->render.setUV(3, savedU, kFloat_1_FxSpray);
    ((RibbonEmitter*)this)->render.setPosition(3, *(Arr3*)(last));
    ((RibbonEmitter*)this)->render.submit();
    // Phase 4: walk history newest to oldest, two vertices per edge pair. Textured U
    // is accumulated distance / totalCenterDistance distance; V selects opposite ribbon sides 0/1.
    ((RibbonEmitter*)this)->render.size(2 * sampleCount);
    int end = sampleCount - 1;
    float zero = kFloat_0_FxSpray;
    for (int i = end; 0 <= i; --i) {
        RibbonSample* vertex = history->at(i);
        float savedTotal = totalCenterDistance;
        float ratio = ((RibbonEmitter*)this)->textured && totalCenterDistance != zero ? distanceFromCurrent / savedTotal
                                                                                      : kFloat_0_FxSpray;
        RibbonColor color;
        unsigned a1 = ((RibbonEmitter*)this)->color[3];
        unsigned a0 = history->colors[3];
        int savedCount3 = sampleCount;
        unsigned savedNode3 = a0;
        color.c[3] = savedNode3 == a1 ? savedNode3 : ((int)savedNode3 * i + (int)a1 * (savedCount3 - i)) / savedCount3;
        a0 = history->colors[2];
        a1 = ((RibbonEmitter*)this)->color[2];
        int savedCount4 = sampleCount;
        color.c[2] = a1 == a0 ? a0 : ((int)a0 * i + (int)a1 * (savedCount4 - i)) / savedCount4;
        a1 = (((RibbonEmitter*)this)->color[1]);
        a0 = history->colors[1];
        bool savedNode4 = a0 == a1;
        color.c[1] = savedNode4 ? a0 : ((int)a0 * i + (int)a1 * (sampleCount - i)) / sampleCount;
        color.c[0] = minimumRibbonCount(
            history->colors[0],
            ((history->colors[0] * i) + ((RibbonEmitter*)this)->color[0] * (sampleCount - i)) / sampleCount);
        unsigned colorA = ComputeDistanceFadedEffectColor(((RibbonEmitter*)this), arg1, &vertex->right, history,
                                                          FX_COLOR_WORD_BYTES(&color));
        unsigned colorB =
            ComputeDistanceFadedEffectColor(((RibbonEmitter*)this), arg1, vertex, history, FX_COLOR_WORD_BYTES(&color));
        int index = (end - i) * 2;
        ((RibbonEmitter*)this)->render.setColor(index, colorA);
        ((RibbonEmitter*)this)->render.setUV(index, ratio, kFloat_0_FxSpray);
        ((RibbonEmitter*)this)->render.setPosition(index, *(Arr3*)(&vertex->right));
        ((RibbonEmitter*)this)->render.setColor(index + 1, colorB);
        ((RibbonEmitter*)this)->render.setUV(index + 1, ratio, kFloat_1_FxSpray);
        ((RibbonEmitter*)this)->render.setPosition(1 + index, *(Arr3*)(vertex));
        distanceFromCurrent = distanceFromCurrent + ribbonSegmentDistance(vertex);
    }
    ((RibbonEmitter*)this)->render.submit();
}

namespace ns4 {
extern "C" {

// Initialize the gallery/effect unit's shared scalar at startup.
void InitializeFxGalleryScalars() {
    s_fxGalleryTwoPi = kFloat_6_2831855;
}

} // extern "C"

} // namespace ns4

// Decrease the gallery countdown only while its countdown flag is enabled.
void CGallery::OnProcess(float delta) {
    if (countdownEnabled != 0) {
        remainingTime -= delta;
    }
}

extern "C" float kFloat_0_FxSpray;

// Copy the wind vector only when at least one configured wind scale is nonzero.
void CFxSpray::SetWindVector(const void* source) {
    if (kFloat_0_FxSpray != minWindScale || kFloat_0_FxSpray != maxWindScale) {
        windVector = *(const WVector*)source;
    }
}

namespace ns4 {

extern "C" {

struct DistanceFadeEmitter {
    char padding_00[0x1D0];
    float opacityMultiplier;
    char padding_1D4[0x30];
    unsigned blendEnabled;
    unsigned blendColor;
    char padding_20C[0x128];
    float lower;
    float upper;
};
struct DistanceFadeParticle {
    char padding_00[0xE8];
    float alpha;
};
extern float kFloat_1_FxSpray;
extern void* FxBoxTree_InsertNode(void*, void*, unsigned, unsigned, void*);
extern void* g_fxChildSprayMapDefaultValue;

// Modulate RGB when enabled and combine particle, emitter and distance opacity into ARGB alpha.
unsigned ComputeDistanceFadedEffectColor(void* selfArg, void* view, void* point, void* effectArg, unsigned color) {
    DistanceFadeEmitter* self = (DistanceFadeEmitter*)selfArg;
    DistanceFadeParticle* effect = (DistanceFadeParticle*)effectArg;
    if (effect == 0)
        return (unsigned)-1;
    float alpha;
    float lower;
    float fade;
    if (self->upper > kFloat_0_FxSpray) {
        Vec3f vec;
        ((void (*)(void*, const void*, const void*))__mi__FRC7WVectorRC7WVector)(
            &vec, (char*)view + FX_MEMBER_OFFSET(WView, viewMatrix[9]), point);
        float value = Magnitude__7WVectorCFv(&vec);
        lower = self->lower;
        if (value < lower) {
            fade = kFloat_1_FxSpray;
        } else if (value > self->upper) {
            return 0;
        } else {
            float t = (value - lower) / (self->upper - lower);
            if (t < kFloat_0_FxSpray)
                fade = kFloat_0_FxSpray;
            else if (t > kFloat_1_FxSpray)
                fade = kFloat_1_FxSpray;
            else
                fade = t;
            fade = kFloat_1_FxSpray - fade;
        }
    } else
        fade = kFloat_1_FxSpray;
    if (self->blendEnabled != 0) {
        unsigned c = self->blendColor;
        unsigned cb = color & 255;
        unsigned cr = (color >> 16) & 255;
        unsigned cg = (color >> 8) & 255;
        unsigned blue = cb * (c & 255);
        unsigned red = cr * ((c >> 16) & 255);
        unsigned green = cg * ((c >> 8) & 255);
        color = (color & 0xFF000000) | ((((red << 8) & 0xFFFF0000) + (green & 0xFFFFFF00)) + (blue >> 8));
    }
    alpha = fade * (effect->alpha * self->opacityMultiplier);
    int byte = (int)((float)(color >> 24) * alpha);
    return (color & 0x00FFFFFF) | (unsigned(byte) << 24);
}

struct ChildSprayOwnerView {
    char padding_00[0x0C];
    float x, y;
    char padding_14[0x20];
    unsigned char enabled;
    char padding_35[0x43];
    void* manager;
    char padding_7C[0x0C];
    void* parent;
    unsigned char flag;
};
struct ChildSprayParticleView {
    char padding_00[0xEC];
    unsigned calls;
};
struct SpawnedSprayView {
    char padding_00[0x0C];
    float a, b;
    char padding_14[0x20];
    unsigned char viewCullingEnabled;
    char padding_35[0x53];
    void* parentCopy;
    unsigned char drawEnabled;
    char padding_8D[0x2B];
    char padding_B8[0x24];
    void* sourceSpray;
    char padding_E0[0x91];
    unsigned char enabled;
    char padding_172[0xD2];
    float f244, f248;
};

} // extern "C"
} // namespace ns4

// Create a scripted child emitter, inherit parent/display state, register it and copy particle position and velocity.
void CFxSpray::SpawnChildSpray(void* source, void* entryPointer) {
    using namespace ns4;
    using ns4::kFloat_0_FxSpray;
    ChildSprayOwnerView* self = (ChildSprayOwnerView*)this;
    ChildSprayParticleView* entry = (ChildSprayParticleView*)entryPointer;
    ++entry->calls;
    SpawnedSprayView* result = (SpawnedSprayView*)CFxSpray::CreateFromScript(
        (CFxBox*)(self->manager), (const char*)((char*)source + FX_MEMBER_OFFSET(ChildSprayCommand, script)), 0, 0);
    if (result == 0)
        return;
    result->enabled = ((ChildSprayCommand*)source)->followSource;
    if (result->enabled)
        result->sourceSpray = self;
    else
        result->sourceSpray = 0;
    result->parentCopy = self->parent;
    result->a = self->x;
    result->b = self->y;
    result->viewCullingEnabled = self->enabled;
    if (self->parent != 0) {
        if (kFloat_0_FxSpray != result->f244 || kFloat_0_FxSpray != result->f248) {
            void* tmp = (char*)result + FX_MEMBER_OFFSET(SpraySpatialView, windVector);
            ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(tmp, (char*)self->parent + 8);
        }
        void* parent20 = (char*)self->parent + FX_MEMBER_OFFSET(CFxSequence, sprayCount);
        unsigned char stack8;
        unsigned char stack9;
        void* stackC;
        void* ret = ((CFxRegistryTree*)parent20)
                        ->FindInsertionPosition((unsigned int*)&result, (FxRegistryNode**)&stackC, &stack9, &stack8);
        void* r5;
        if (stackC != 0 && (unsigned)result <= ((ChildSprayRegistryNode*)stackC)->value.key) {
            r5 = (char*)stackC + FX_MEMBER_OFFSET(ChildSprayRegistryNode, value);
        } else {
            unsigned stackBuf[2];
            stackBuf[0] = (unsigned)result;
            stackBuf[1] = (unsigned)g_fxChildSprayMapDefaultValue;
            void* inserted = FxBoxTree_InsertNode(parent20, ret, stack9, stack8, stackBuf);
            r5 = (char*)inserted + FX_MEMBER_OFFSET(ChildSprayRegistryNode, value);
        }
        ((ChildSprayRegistryValue*)r5)->emitter = (unsigned)result;
    }
    {
        void* r29 = (char*)result + FX_MEMBER_OFFSET(SpraySpatialView, position);
        float buf[3];
        __pl__FRC7WVectorRC7WVector(buf, (char*)entry + FX_MEMBER_OFFSET(ParticleSpatialView, localPosition),
                                    (char*)entry + FX_MEMBER_OFFSET(ParticleSpatialView, emitterPosition));
        ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(r29, buf);
    }
    ((void (*)(void*, void*))__as__7WVectorFRC7WVector)((char*)result + FX_MEMBER_OFFSET(SpraySpatialView, velocity),
                                                        (char*)entry + FX_MEMBER_OFFSET(ParticleSpatialView, velocity));
    result->drawEnabled = self->flag;
    ((CFxBox*)(self->manager))->RegisterSpray(((CFxSpray*)(result)));
}

// Allocate a gallery object with a stopped, zero-valued countdown.
CGallery* CGallery::Create() {
    return new CGallery;
}

namespace ns4 {
extern "C" {

struct ReferencedObjectView {
    char padding_00[0x10];
    int referenceCount;
};
struct FxResourceManagerView {
    char padding_00[0x140];
    void* resourceManager;
};
struct FxResourceOwnerView {
    char padding_00[0x78];
    FxResourceManagerView* effectOwner;
};
struct ReferencedSprayView {
    char padding_00[0x78];
    FxResourceManagerView* effectOwner;
    char padding_7C[0x220 - 0x7C];
    void* resourceId;
};

// Find the emitter's referenced resource and increment its reference count; fail when it cannot be resolved.
int FxSpray_AcquireReferencedObject(void* unused, ReferencedSprayView* self) {
    if (self->resourceId != 0) {
        FxResourceManagerView* manager = self->effectOwner;
        if (manager->resourceManager == 0)
            return 0;
        ReferencedObjectView* result =
            (ReferencedObjectView*)ObjectList_FindById(manager->resourceManager, self->resourceId);
        if (result == 0)
            return 0;
        result->referenceCount++;
    }
    return 1;
}

} // extern "C"

} // namespace ns4

extern "C" {
void InitializeFxGalleryScalars();
}
/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462114", _ctors_u80119230_0, InitializeFxGalleryScalars);
#pragma pop
