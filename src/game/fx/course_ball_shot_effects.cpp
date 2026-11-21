#include <decomp/static_init.h>
// Course/ball visual effects and resolved character equipment/model storage.
// Mod lookup: frame rate, hole marker, particle texture/model ownership.
// - CLightColumn_UpdateWidthScrollAndFade: kColumnAlphaStep=20 alpha units per
//   UPDATE CALL, saturating 0..255 (13 calls for a full fade). UV scroll instead
//   subtracts kFloat_2_0*delta; width=max(.01*horizontalCameraDistance,10*ballScale).
//   Its constants are shared by other effects in u801AC5B0_data.cpp: introduce
//   separate storage in a future isolated mod instead of changing a shared factor.
// - CLightColumn_LoadBitmaps: five marker frames, scrolling texture and height
//   kFloat_1600_ShotEffects=1600 world units. Draw uses frames 0..3, split at .6H;
//   only the scrolling strip is distance-culled (320 world units, squared 102400).
// - CPowerShot_GrowRingFadeAlpha: lifetime=.8000000119 seconds;
//   radius +=20 world units/second * delta, alpha=255*(1-elapsed/lifetime).
//   No final alpha clamp; elapsed==lifetime stays active until the next step.
// - CShotParticle_Update: lifetime uses delta, but late-life velocity scaling and
//   trailLength*=.9 occur each call. CBunkerShotParticle_Draw also damps scale
//   by .95 PER DRAW; changing draw cadence changes subsequent geometry.
// - CStarDust_Update/CStarDust2_UpdateGlowAndParticles: emission interval/catch-up and per-call
//   ring-buffer spawning differ. CEnergyCenter_UpdateEmitAndParticles emits ceil(35*dt) up to
//   capacity. Review RNG consumption, crossings and rounding when changing cadence.
// - Model/equipment: ItemSelection_ResolvePartData borrows IFF records;
//   ResolvedItemSelection_EquipPart repairs dependent parts without rollback.
//   PlayerModel_ClearState is initialization only, not a release operation;
//   PlayerModel_Dtor calls PlayerModel_ReleaseResourcesByMask before freeing text.
// Related schemas: classes/C*.h and effect_layout below. Data companion contains
// binary32 scalars/raw words/eight-byte leading-float slots; keep widths and bits.
// There is no display retrace/resolution/aspect/viewport/scissor control here.
// Effect clock: the task's deltaSeconds (CTask_UpdateCurrentTaskWithFadeGates,
// ctask_transition_command_queue.cpp; CTask_ProcessActors, task_manager_and_actors.cpp) reaches CParticleEffect_OnProcess
// (particle_effect_pool.cpp), which forwards it unchanged to ParticleEffectPool_UpdateAndReap
// (particle_effect_emitters.cpp), then each effect's update(float). Thus delta/lifetimes/delays here
// are seconds, angles radians, and angular rates radians/second. Frame-count
// alpha/damping operations noted above remain per call. This pool path does
// not use CFxBox's separate seconds-to-milliseconds conversion (cfx_shot_effects.cpp).
// Resource/render methods require the normal game sequence, initialized globals
// and valid document indices. No additional synchronization is provided.
// Preserve virtual ABI, random-call order, float operation order, delayed expiry,
// refcounts and array-cookie release paths. All positions are world-space unless
// a local comment specifies sprite UVs or a different representation.
#include "MSL/vector.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
typedef std::vector<void*, app::app_allocator<void*> > MSLPointerVector;
#include "iff/IffStruct.h"
#include "classes/WView.h"
// WView layout: 0x80065F84; document lookup: 0x801AD618; elapsed effect time: 0x801ACFEC.
#include "classes/CSharedDoc.h"
// View existing storage without changing ownership or representation.
static inline CSharedDoc* AsCSharedDoc(const volatile void* p) { return (CSharedDoc*)p; }
#include "classes/EnergyCenterParticle.h"
#include "classes/StarDust2Particle.h"
#include "classes/StarDustParticle.h"
#include "classes/CGolfDoc.h"
// View existing storage without changing ownership or representation.
static inline CGolfDoc* AsCGolfDoc(const volatile void* p) { return (CGolfDoc*)p; }
#include "game/singletons.h"
#include "game/wvector.h"
#include "game/types.h"
#include "sdk_protos.h"
#include "classes/CEnergyCenter.h"
#include "classes/CStarDust2.h"
#include "classes/CCoin.h"
#include "classes/CBunkerShotParticle.h"
#include "classes/CStarDust.h"
#include "classes/CLightColumn.h"
#include "classes/CRipple.h"
#include "classes/CDust.h"
#include "classes/CShotParticle.h"
#include "classes/CPowerShot.h"
#include "classes/CGrassParticle.h"
#include "classes/CFragment.h"
#include "classes/CBunkerParticle.h"
#include "classes/CDroplet.h"
#include "classes/W3dSpr.h"
#include "classes/W3dAniSpr.h"
#include "classes/CameraTransform800D.h"
#include "classes/ItemSelection801B7688.h"
#include <decomp/cw_bitfields.h>

// These POD views describe effect records absent from the shared class headers.
// No constructors or virtual methods: address calculations must remain unchanged.
// This merged unit has two independent responsibilities: particle effects (ns0..ns3/ns5)
// and equipment-selection/player-model storage (ns4). Effect positions/velocities and
// orbit origins are world-space; sprite sizes use engine world units. Update deltas share
// seconds with lifetime/emission intervals; byte alpha is in [0,255].
// Resource handles are released through their manager/refcount, sprite objects through
// virtual destruction, and particle arrays through their array/cookie release paths.
namespace effect_layout {
// Immediate ring/streak vertex: position, RGBA bytes, then texture coordinates.
struct TexturedVertex {
    Vec3f position;
    unsigned char red, green, blue, alpha;
    float u, v;
};
// Address a textured vertex by byte offset into the borrowed vertex buffer.
static inline TexturedVertex* TexturedVertexAt(void* buffer, int byteOffset) {
    return (TexturedVertex*)((char*)buffer + byteOffset);
}
struct MotionRecord { Vec3f position, velocity; };
// Address a position/velocity record by byte offset into the borrowed particle array.
static inline MotionRecord* MotionAt(void* records, int byteOffset) {
    return (MotionRecord*)((char*)records + byteOffset);
}
struct DustRecord { Vec3f position; float angle; };
// Address a dust position/angle record by byte offset into the borrowed array.
static inline DustRecord* DustAt(void* records, int byteOffset) {
    return (DustRecord*)((char*)records + byteOffset);
}
struct ResolvedPart { unsigned int key; void* record; };
// Unresolved selections store 24 packed keys after an 8-byte header; resolved selections
// keep the same header and 24 {key, borrowed IFF record pointer} pairs (0xC8 total).
struct ResolvedSelection {
    unsigned int character;
    unsigned char primaryCount, totalCount, textureVariant, bitmapVariant;
    ResolvedPart parts[24];
};
struct OwnedUnicodeBuffer { void* data; unsigned int size, capacity; };
// Storage regions established by the model reset/release paths. Unused words retain their grouping.
struct PlayerModelStorage {
    unsigned int resourceWords[7];
    OwnedUnicodeBuffer name;
    unsigned int modelWords[3];
    unsigned int defaultEnabled;
    char textureState[0x60];
    char partResourceState[0x120];
    unsigned char enabled, status1, status2, reserved1BB;
    ResolvedSelection selection;
    char animationState[0x14];
    float animationPosition[2];
};
typedef char CheckTexturedVertex[sizeof(TexturedVertex) == 0x18 ? 1 : -1];
typedef char CheckResolvedSelection[sizeof(ResolvedSelection) == 0xC8 ? 1 : -1];
typedef char CheckPlayerModelStorage[sizeof(PlayerModelStorage) == 0x2A0 ? 1 : -1];
struct CoinShadowOrigin { char reserved00[0x4C]; Vec3f position; };
struct Base {
    void* vtable;
    int liveCount;
    Vec3f position;
    int bitmapId;
    unsigned char active;
    char reserved19[3];
    void* animatedSprite;
    void* staticSprite;
    float elapsed;
    float interval;
    int reserved2C;
    void* vertexPointers[4];
    int vertexListTerminator;
    char vertices[4][0x38];
};
struct Ball {
    Vec3f position;
    Vec3f velocity;
    char reserved18[0xC0 - 0x18];
    float effectScale; // Ball +0xC0 scales effect size; not a measured velocity.
};
struct ShotStreak {
    unsigned char active;
    char reserved01[3];
    Vec3f position, velocity;
    float remainingLife, trailLength, slowdownThreshold;
};
// Sand streak ABI is distinct from ShotStreak: +1C is unused by the current
// update/draw path; life is +20, fade threshold +24. Position is world XYZ,
// velocity world units/second, both time fields seconds. UpdateParticles
// damps X/Z per call, applies Y gravity and expires active when life < 0.
struct BunkerStreak {
    unsigned char active; char reserved01[3];
    Vec3f position, velocity;
    float unused1C, remainingLife, slowdownThreshold;
};
typedef char CheckBunkerStreakSize[sizeof(BunkerStreak) == 0x28 ? 1 : -1];
typedef char CheckBunkerStreakLife[(int)&((BunkerStreak*)0)->remainingLife == 0x20 ? 1 : -1];
struct Grass {
    int frame;
    float opacity, angle, angularVelocity;
    Vec3f position, velocity;
    unsigned char active;
    char reserved29[3];
    float lifetime;
};
// Orbit equation: position = origin + radialScale *
// (cos(angle) * cosineAxis + sin(angle) * sineAxis), with angle in radians.
// The axes include random jitter and need not be unit length or perpendicular.
struct Orbit {
    unsigned char active;
    char reserved01[3];
    float radialScale;
    Vec3f position, cosineAxis, sineAxis;
    char reserved2C[0xC];
    Vec3f origin;
    float angle, angularVelocity, fade;
};
struct Energy {
    unsigned char active;
    char reserved01[3];
    Vec3f position, direction;
    float opacity;
};
struct Ripple {
    float diameter, opacity, initialOpacity, fadeRate, expansionRate;
    unsigned char active;
    char reserved15[3];
    float startDelay, delayRemaining;
};
// Address a ripple record by byte offset into the borrowed array.
static inline Ripple* RippleAt(void* records, int byteOffset) {
    return (Ripple*)((char*)records + byteOffset);
}
struct CoinSprite {
    unsigned char active;
    char reserved01[3];
    Vec3f position, velocity;
    float opacity, remainingLife, fadeThreshold;
};
// Coin model record, exactly 0x18 bytes, four-byte aligned, target-native floats.
// +00 owned CPetBody*, +04 Y rotation (radians), +08 velocity xyz, +14 retained
// bounce Y velocity. Velocity is launch-space data: Update multiplies it by
// 15 before delta, so world velocity is 15*velocity, not the stored vector alone.
// CCoin owns records/models; vfunc_6 reuses models and resets velocity/angle,
// vfunc_1 integrates/bounces, DrawCoinsAndSprites consumes body transforms,
// and CCoin_dtor releases models then the array (including its array cookie).
struct CoinModel {
    void* object;
    float angle;
    Vec3f velocity;
    float bounceVelocity;
};
// CoinSprite above is the OTHER array: exactly 0x28 bytes. +00 active byte,
// +04 world xyz, +10 velocity xyz (world speed is 7*stored velocity),
// +1C opacity in 0..255 float units, +20 remaining seconds, +24 fade threshold
// seconds (initial remainingLife*0.8). vfunc_6 initializes it; vfunc_1 changes
// active/position/velocity/opacity/time; DrawCoinsAndSprites borrows it. It owns
// no per-record allocation; CCoin owns the array and shared animated sprite.
typedef char CheckCoinModelSize[sizeof(CoinModel) == 0x18 ? 1 : -1];
typedef char CheckCoinVelocityOffset[IFF_OFFSET(CoinModel, velocity) == 8 ? 1 : -1];
typedef char CheckCoinBounceOffset[IFF_OFFSET(CoinModel, bounceVelocity) == 0x14 ? 1 : -1];
typedef char CheckCoinSparkTimeOffset[IFF_OFFSET(CoinSprite, remainingLife) == 0x20 ? 1 : -1];
struct CameraMatrices {
    char reserved00[0x20];
    float transform0[16], transform1[16];
};
struct CenterPosition {
    char reserved00[0x134];
    Vec3f center;
};
struct FragmentAxis {
    char reserved00[0x138];
    Vec3f launchAxis;
};
struct CourseEffectStringPool {
    char holeEffectPattern[0x16];
    char holeScript[0xF];
    char bunkerSandSound[0x11];
    char waterDropTexture[0xB];
    char coinPetCommand[0x18];
    char coinShadowGroup[5];
    char sunBone[4];
    char coinDropSound[0xF];
    char golfBackgroundActor[7];
    char ballObject[7];
    char reserved7F[5];
};
// Preserve the signedness of the existing loop counters and packed bit operations.
enum {
    kPowerShotSegments = 16,
    kRingVertexBytes = 0x18, kRingQuadBytes = 0x60, kRingBufferBytes = 0x600,
    kTextureHandleIdMask = 0x7FF,
    kStarDustCapacity = 400, kStarDust2Capacity = 60,
    kOpaqueAlpha = 255,
    kWhiteRgb = 0x00FFFFFF,
    kFloatSignBit = 0x80000000u,
    kEffectSuppressedGameStates = 0x070A3000,
    kLightColumnHiddenFlag = 0x200,
    kColumnFadeIdle = 0, kColumnFadeIn = 1, kColumnFadeOut = 2,
    kColumnAlphaStep = 20, // byte-alpha units per eligible update, NOT per second
    kColumnFrameBitmapCount = 5, kColumnFirstResourceFrame = 10,
    kRippleStorageBytes = 0xA0, kDropletChildParticleCount = 100,
    kPartyItemMode = 3, kOpaqueBallPartyItem = 3,
    kPositionColorVertexFormat = 0xF001,
    kPositionColorUvVertexFormat = 0x11F001,
    kPartKindShift = 9, kPartKindMask = 3,
    kPartSlotShift = 13, kPartSlotMask = 31,
    kPartVariantBit = 0x200,
    kDefaultPartKeyBits = 0x08000400,
    kFullDefaultPartKeyBits = 0x08000600,
    kCharacterKeyMask = 0x03FFFFFF,
    kCharacterKeyShift = 18,
    kUnspecifiedSlot = 0xFF
};
typedef char CheckBaseSize[sizeof(Base) == 0x124 ? 1 : -1];
typedef char CheckShotSize[sizeof(ShotStreak) == 0x28 ? 1 : -1];
typedef char CheckGrassSize[sizeof(Grass) == 0x30 ? 1 : -1];
typedef char CheckOrbitSize[sizeof(Orbit) == 0x50 ? 1 : -1];
// Orbit records are stored back to back in the dust effects' particle arrays (sizeof(Orbit) == 0x50).
enum { kOrbitRecordBytes = 0x50 };
// Arrays allocated with new[] here carry a 0x10-byte header before the first element;
// FreeArrayBlock releases such a block given the pointer to its first element.
enum { kArrayHeaderBytes = 0x10 };
#define FreeArrayBlock(firstElement) __dla__FPv((char*)(firstElement) - kArrayHeaderBytes)
#define ARRAY_NEW(n) __nwa__FUl(n)
// Return the index-th orbit record of a borrowed particle array.
// (A macro, not an inline function: as an inline function it changed the compiled code.)
#define OrbitAt(particles, index) ((Orbit*)((char*)(particles) + (index) * kOrbitRecordBytes))
typedef char CheckEnergySize[sizeof(Energy) == 0x20 ? 1 : -1];
typedef char CheckRippleSize[sizeof(Ripple) == 0x20 ? 1 : -1];
typedef char CheckCoinSpriteSize[sizeof(CoinSprite) == 0x28 ? 1 : -1];
typedef char CheckStringPoolSize[sizeof(CourseEffectStringPool) == 0x84 ? 1 : -1];
}
using namespace effect_layout;


// View existing storage without changing ownership or representation.
static inline CDroplet* AsCDroplet(const volatile void* p) { return (CDroplet*)p; }

// View existing storage without changing ownership or representation.
static inline CBunkerParticle* AsCBunkerParticle(const volatile void* p) { return (CBunkerParticle*)p; }

// View existing storage without changing ownership or representation.
static inline CFragment* AsCFragment(const volatile void* p) { return (CFragment*)p; }

// View existing storage without changing ownership or representation.
static inline CGrassParticle* AsCGrassParticle(const volatile void* p) { return (CGrassParticle*)p; }

// Keep this cast as an expression: an inline call changes CodeWarrior operand selection.
#define AsCPowerShot(p) ((CPowerShot*)(p))

// View existing storage without changing ownership or representation.
static inline CShotParticle* AsCShotParticle(const volatile void* p) { return (CShotParticle*)p; }

// Keep this cast as an expression: an inline call changes CodeWarrior operand selection.
#define AsCDust(p) ((CDust*)(p))

// View existing storage without changing ownership or representation.
static inline CRipple* AsCRipple(const volatile void* p) { return (CRipple*)p; }

// View existing storage without changing ownership or representation.
static inline CLightColumn* AsCLightColumn(const volatile void* p) { return (CLightColumn*)p; }

// Keep this cast as an expression: an inline call changes CodeWarrior operand selection.
#define AsCStarDust(p) ((CStarDust*)(p))

// View existing storage without changing ownership or representation.
static inline CBunkerShotParticle* AsCBunkerShotParticle(const volatile void* p) { return (CBunkerShotParticle*)p; }

// View existing storage without changing ownership or representation.
static inline CCoin* AsCCoin(const volatile void* p) { return (CCoin*)p; }

// View existing storage without changing ownership or representation.
static inline CStarDust2* AsCStarDust2(const volatile void* p) { return (CStarDust2*)p; }

// Keep this cast as an expression: an inline call changes CodeWarrior operand selection.
#define AsCEnergyCenter(p) ((CEnergyCenter*)(p))

// ---- merged from course_ball_shot_effects.cpp ----
namespace ns0 {
// Translation unit 0x801AC5B0-0x801ADCA0 (14 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern char __vt__12CLightColumn[64];
extern char __vt__13CShotParticle[64];
extern char s_cameraTransform[64];
void *GolfBall__Fv();
void *PreData__9CGolfBallFi(void *, int);
void __as__7WVectorFRC7WVector(void *, int);
void __ml__FRC7WVectorf(void *, void *, float);
extern const float kFloat_0_Particles;
extern const float kFloat_1_6_ShotEffects; // 1.6: power-shot ring height above the ball
extern const float kFloat_0_39269909_ShotEffects; // pi/8: angular step for sixteen ring segments
extern const float kFloat_1600_ShotEffects; // 1600: default column height
extern char courseEffectStrings[64];
extern char __vt__10CPowerShot[64];extern char s_particleVtable[64];
void *g_resourceManager;
char *String_FormatTemporary(const char *, ...);
void *ResourceManager_LoadBitmap(void *, const char *, int, int, int);
void ResourceManager_ReleaseId(void *, void *);
void CubeResourceStorage_dtor(void *, int);
void __dl__FPv(void *);
void __dla__FPv(void *);
void Vec3_Scale(void *, void *, float);
void Vec3_Add(void *, void *, void *);
void __mi__FRC7WVectorRC7WVector(void *, void *, void *);
float Magnitude__7WVectorCFv(void *);
extern const float kFloat_0_89999998_ShotEffects; // 0.9: shot-streak tail decay per update
extern const float kFloat_10_CourseEffects;
extern const float kFloat_2_0;
extern const float kFloat_1_2_ShotEffects; // 1.2: inner ring radius multiplier
extern const float kFloat_0_0099999998_ShotEffects; // 0.01: dust spawn interval / distance-to-scale factor
extern void *g_sharedDoc;
extern const char s_shotAndRippleRenderStateLists[64];
extern char kIdentityMatrix4x4[64];
extern int g_view;
void VertexBuffer_Create(int, int, unsigned int, int, void **, int);
void Resource_GetOffsetPtr(void *, int, int, void *, int);
void GraphicsBuffer_Flush(void *);
void Vec3_Normalize(void *, void *);
void Vector_CopyComponents(void *, void *);
int rand();
void Engine_SetRenderStateList(void *, int);
float Math_CosApprox(float);
float Math_SinApprox(float);
void Engine_SetTextureStageStateList(int, void *, int);
void Engine_SetTextureStageParameterList(int, void *, int);
void DrawPrimitiveFromBoundBuffers(int, int, int);
void *Camera_GetRenderState(int);
void SetTransform(int, void *);
void Engine_SetType7Resource(int);
void Engine_SetFlexibleVertexFormat(int);
void Engine_SetVertexStream(int, void *, int, int);
void Engine_SetIndexBuffer(void *, int);
void Engine_SetTextureStageResource(int, int);
void Engine_SetTexturePalette(int, int);
void Engine_DrawIndexedPrimitive(int, int, int, int, int, int);
float Vec3_CalculateLengthSquared(void *);
void *Normalize__7WVectorFv(void *);
void *GolfPlayer_GetEntryByIndex(void *, int);
void *memcpy(void *, const void *, unsigned int);
struct Blk24 { int v[24]; };
struct Blk12 { int v[12]; };
struct Blk10 { int v[10]; };
extern const float kFloat_neg0_5_ShotEffects; // -0.5: hole frame left extent
extern const float kFloat_102400_ShotEffects; // 102400: squared distance limit for the scrolling column
extern const float kFloat_neg0_25_ShotEffects; // -0.25: scrolling-column left extent
extern const float kFloat_0_25_805DF468;
extern const float kFloat_200_shotParticles;
extern const float kFloat_0_5_Particle;
extern const float kFloat_0_6_ShotEffects;
struct PShot {
    char pad0[8]; Vec3f position; char pad1[0x18 - 0x14]; unsigned char active;
    char pad2[0x124 - 0x19]; int alpha; float elapsedTime; float radius;
};
// Start the expanding ring at the ball position and clear its lifetime and radius.
void CPowerShot_ResetRingAtBall(PShot *self) {
    self->radius = kFloat_0_Particles;
    self->elapsedTime = kFloat_0_Particles;
    *(WVector*)(&self->position) = *(WVector*)((int)GolfBall__Fv());
    self->active = 1;
    self->position.y = self->position.y + kFloat_1_6_ShotEffects;
}

// Build and draw the sixteen textured quads of the expanding power-shot ring.
void CPowerShot_BuildRingAndDraw(void* self) {
    unsigned int quadByteOffset = 0;
    char* p = (char*)self;
    char* g = (char*)s_shotAndRippleRenderStateLists;
    if (!AsCPowerShot(p)->active) return;
    float angleStep;
    float innerRadiusScale;
    float segmentAngle = kFloat_0_39269909_ShotEffects; // 2*pi/16 radians
    void* data;
    Resource_GetOffsetPtr(AsCPowerShot(p)->vertexBuffer, 0, 0, &data, 0);
    float outerRadiusScale = kFloat_2_0;
    int segmentIndex = 0;
    innerRadiusScale = kFloat_1_2_ShotEffects;
    angleStep = kFloat_0_39269909_ShotEffects;
    // This cursor is two vertices before the current quad. Indices 4/5 below
    // are its inner/outer end; indices 0/1 belong to the preceding quad.
    int quadByteOffsetMinusTwoVertices = -0x30;

    // World XZ annulus, raised origin at self+8. Vertex order is outer start,
    // inner start, inner end, outer end. Radius scales are 2.0 and 1.2: increasing
    // both grows the ring; their difference determines its width. Segment count,
    // 2*pi/16 angle step, four vertices/quad and buffer capacity must agree.
    for (; segmentIndex < kPowerShotSegments; ) {
        int alpha = AsCPowerShot(p)->alpha;
        TexturedVertexAt(data, quadByteOffset)[3].alpha = (unsigned char)alpha;
        TexturedVertexAt(data, quadByteOffset)[2].alpha = (unsigned char)alpha;
        TexturedVertexAt(data, quadByteOffset)[1].alpha = (unsigned char)alpha;
        TexturedVertexAt(data, quadByteOffset)->alpha = (unsigned char)alpha;

        if (segmentIndex == 0) {
            float outerStartOffset[3] = { 0.0f, 0.0f, 0.0f };
            outerStartOffset[0] = outerRadiusScale * AsCPowerShot(p)->radius;
            float innerStartOffset[3] = { 0.0f, 0.0f, 0.0f };
            innerStartOffset[0] = innerRadiusScale * AsCPowerShot(p)->radius;
            Vec3_Add(data, p + 8, outerStartOffset);
            Vec3_Add(&((TexturedVertex*)data)[1].position, p + 8, innerStartOffset);
        } else {
            // Reuse the preceding edge, preserving seam coordinates bit for bit.
            Vector_CopyComponents((char*)data + quadByteOffset,
                &TexturedVertexAt(data, quadByteOffsetMinusTwoVertices)[1].position);
            Vector_CopyComponents(&TexturedVertexAt(data, quadByteOffsetMinusTwoVertices)[3].position,
                (char*)data + quadByteOffsetMinusTwoVertices);
        }

        // Keep cos-before-sin, radius multiplication, scale, then translation.
        // Algebraic reassociation or a different trig API can change float bits.
        float radialOffset[3] = { 0.0f, 0.0f, 0.0f };
        radialOffset[0] = AsCPowerShot(p)->radius * Math_CosApprox(segmentAngle);
        radialOffset[2] = AsCPowerShot(p)->radius * Math_SinApprox(segmentAngle);
        Vec3_Scale(&TexturedVertexAt(data, quadByteOffsetMinusTwoVertices)[5].position, radialOffset, kFloat_2_0);
        Vec3_Scale(&TexturedVertexAt(data, quadByteOffsetMinusTwoVertices)[4].position, radialOffset, kFloat_1_2_ShotEffects);
        Vec3_Add(&TexturedVertexAt(data, quadByteOffsetMinusTwoVertices)[4].position,
            &TexturedVertexAt(data, quadByteOffsetMinusTwoVertices)[4].position, p + 8);
        Vec3_Add(&TexturedVertexAt(data, quadByteOffsetMinusTwoVertices)[5].position,
            &TexturedVertexAt(data, quadByteOffsetMinusTwoVertices)[5].position, p + 8);
        segmentIndex = segmentIndex + 1;
        segmentAngle = segmentAngle + angleStep;
        quadByteOffset += kRingQuadBytes;
        quadByteOffsetMinusTwoVertices += kRingQuadBytes;
    }

    // Phase 2: flush geometry, copy the render-state lists, then bind camera/texture state.
    GraphicsBuffer_Flush(AsCPowerShot(p)->vertexBuffer);

    char * savedG = g;
    struct Blk24 copied = *(struct Blk24 *)(savedG + 0x28);

    int *sa = (int *)(g + 0x88);
    int a1 = sa[1]; int a0 = sa[0]; int a3 = sa[3]; int a2 = sa[2];
    int a4 = sa[4]; int a6 = sa[6]; int a5 = sa[5]; int a7 = sa[7];

    int a8 = sa[8]; int a9 = sa[9]; int a10 = sa[10]; int a11 = sa[11];
    int *sb = (int *)(g + 0xb8); int b1 = sb[1]; int b0 = sb[0]; unsigned int b2 = sb[2]; int b4 = sb[4]; int b6 = sb[6];
    int b3 = sb[3]; unsigned int b5 = sb[5]; int b7 = sb[7]; unsigned int b8 = sb[8]; int b9 = sb[9];

    int s58[12];
    s58[0] = a0;
    s58[1] = a1;
    int savedNode5 = a2;
    s58[2] = savedNode5;
    s58[3] = a3;
    s58[4] = a4; s58[5] = a5; s58[6] = a6; s58[7] = a7;
    s58[8] = a8; s58[9] = a9; s58[11] = a11; s58[10] = a10;

    int s30[10];
    s30[0] = b0; s30[1] = b1; s30[2] = b2; s30[3] = b3; s30[4] = b4;
    s30[5] = b5;
    int savedNode6 = b6;
    s30[6] = savedNode6;
    s30[7] = b7;
    s30[8] = b8;
    s30[9] = b9;

    Engine_SetRenderStateList(copied.v, 12);
    Engine_SetTextureStageStateList(0, s58, 6);
    Engine_SetTextureStageParameterList(0, s30, 5);

    void *obj16 = ((WView*)g_view)->renderTarget;
    if (0 != obj16) {
        ((void (*)(void *, int))(*(void ***)obj16)[0x64 / 4])(obj16, 0x20200000);
        ((void (*)(void *, int))(*(void ***)obj16)[0x68 / 4])(obj16, 0x00800000);
    }

    void *camera = Camera_GetRenderState(*(int *)&g_view);
    char *cam = (char *)camera;
    SetTransform(1, cam + 0x60); // Camera projection matrix; unrelated to the quad byte size.
    SetTransform(0, cam + 0x20);
    Engine_SetType7Resource(0);
    // Positions already include world translation, so the model transform is identity.
    SetTransform(2, kIdentityMatrix4x4);
    SetTransform(11, kIdentityMatrix4x4);
    Engine_SetFlexibleVertexFormat(kPositionColorUvVertexFormat);
    Engine_SetVertexStream(0, AsCPowerShot(p)->vertexBuffer, 0, kRingVertexBytes);

    void *obj = ((WView*)g_view)->renderTarget;
    void *r = ((void *(*)(void *, int))(*(void ***)obj)[0x70/4])(obj, AsCPowerShot(p)->textureId);
    Engine_SetTextureStageResource(0, (int)r);

    obj = ((WView*)g_view)->renderTarget;
    r = ((void *(*)(void *, int))(*(void ***)obj)[0x74/4])(obj, AsCPowerShot(p)->textureId);
    Engine_SetTexturePalette(0, (int)r);

    DrawPrimitiveFromBoundBuffers(0x80, 0, 0x10);
}

// Initialize the hole-marker column and its bitmap slots.
void CLightColumn_Ctor(void *self) {
    char *p = (char *)self;
    AsCLightColumn(p)->unk_14 = 0;
    AsCLightColumn(p)->initialized = 0;
    (*(int*)&AsCLightColumn(p)->ownedObject1) = 0;
    (*(int*)&AsCLightColumn(p)->ownedObject2) = 0;
    AsCLightColumn(p)->vertex0 = ((Base*)p)->vertices[0];
    AsCLightColumn(p)->vertex1 = ((Base*)p)->vertices[1];
    AsCLightColumn(p)->vertex2 = ((Base*)p)->vertices[3];
    AsCLightColumn(p)->vertex3 = ((Base*)p)->vertices[2];
    AsCLightColumn(p)->vertexListTerminator = 0;
    *(void **)p = &__vt__12CLightColumn;
    (*(int*)&AsCLightColumn(p)->scrollingBitmap) = 0;
    (*(int*)&AsCLightColumn(p)->frameBitmap0) = 0;
    AsCLightColumn(p)->frameBitmap1 = 0;
    AsCLightColumn(p)->frameBitmap2 = 0;
    AsCLightColumn(p)->frameBitmap3 = 0;
    AsCLightColumn(p)->frameBitmap4 = 0;
}

// Load marker frames numbered 10..14 and the scrolling texture; start fully visible.
// Required constructed self receives manager-retained bitmap handles; matching
// releases are in CLightColumn_dtor. No failure checks before setting initialized.
// Height=1600 world units, V=0, mode=idle and alpha=255. The shifted receiver in
// the frame loop advances one HANDLE WORD, not a full CLightColumn object.
// scriptPathCursor's unused final addition preserves the old source lifetime.
void CLightColumn_LoadBitmaps(void *self) {
    char *slot = (char *)self;
    int i = 0;
    do {
        int resourceFrameNumber = i + kColumnFirstResourceFrame;
        AsCLightColumn(slot)->frameBitmap0 = ResourceManager_LoadBitmap(g_resourceManager, String_FormatTemporary(((CourseEffectStringPool*)courseEffectStrings)->holeEffectPattern, resourceFrameNumber), 0, 0, 0);
        slot += 4;
        i += 1;
    } while (i < kColumnFrameBitmapCount);
    char *scriptPathCursor =((CourseEffectStringPool*)courseEffectStrings)->holeScript;
    char * scriptPath = scriptPathCursor;
    void *scrollingBitmap = ResourceManager_LoadBitmap(g_resourceManager, scriptPath, 0, 0, 0);
    scriptPathCursor = scriptPathCursor + 0x16;
    float initialScrollV = kFloat_0_Particles;
    int opaqueAlphaWord = kOpaqueAlpha;
    unsigned int idleFadeMode = 0;
    float columnHeight = kFloat_1600_ShotEffects;

    AsCLightColumn(self)->scrollingBitmap = scrollingBitmap;
    AsCLightColumn(self)->columnHeight = columnHeight;
    AsCLightColumn(self)->textureScrollV = initialScrollV;
    AsCLightColumn(self)->fadeMode = idleFadeMode;
    AsCLightColumn(self)->frameColumnAlpha = (unsigned char)opaqueAlphaWord;
    int scrollingOpaqueAlphaWord = opaqueAlphaWord;
    int loadedFlag = 1;
    AsCLightColumn(self)->scrollingColumnAlpha = (unsigned char)scrollingOpaqueAlphaWord;
    AsCLightColumn(self)->initialized = (unsigned char)loadedFlag;
}

struct ResourceRefBits { CW_BITS2( unsigned int refs : 16, unsigned int other : 16) };
struct ShotOwnedObject { 
virtual void destroy(int);
};
// Release the retained ring buffer and owned sprite objects; flags > 0 also frees self.
void *CPowerShot_dtor(void *self, int flags) {
    char *p = (char *)self;
    if (p != 0) {
        void *resource = AsCPowerShot(p)->vertexBuffer;
        *(void **)p = &__vt__10CPowerShot;
        if (resource != 0) {
            ResourceRefBits* refs = (ResourceRefBits*)resource;
            --refs->refs;
            if (refs->refs == 0)
                CubeResourceStorage_dtor(resource, 1);
            AsCPowerShot(p)->vertexBuffer = 0;
        }
    if (p != 0) {
        *(void **)p = &s_particleVtable;
        void *obj = AsCPowerShot(p)->ownedObject2;
        if (obj != 0) {
            if (obj != 0) {
                ((ShotOwnedObject*)obj)->destroy(1);
            }
            AsCPowerShot(p)->ownedObject2 = 0;
        }
        obj = AsCPowerShot(p)->ownedObject1;
        if (obj != 0) {
            if (obj != 0) {
                ((ShotOwnedObject*)obj)->destroy(1);
            }
            AsCPowerShot(p)->ownedObject1 = 0;
        }
    }
        if (flags > 0)
            __dl__FPv(p);
    }
    return self;
}

#pragma use_lmw_stmw on
// Release five frame bitmaps, scrolling bitmap, and owned sprites; optionally free self.
void *CLightColumn_dtor(void *self, int flags) {
    char *p = (char *)self;
    if (p != 0) {
        *(void **)p = &__vt__12CLightColumn;
        char *q = p;
        for (int i = 0; i < 5; ++i) {
            if (g_resourceManager != 0 && AsCLightColumn(q)->frameBitmap0 != 0) {
                ResourceManager_ReleaseId(g_resourceManager, AsCLightColumn(q)->frameBitmap0);
                AsCLightColumn(q)->frameBitmap0 = 0;
            }
            q += 4;
        }
        if (g_resourceManager != 0 && AsCLightColumn(p)->scrollingBitmap != 0) {
            ResourceManager_ReleaseId(g_resourceManager, AsCLightColumn(p)->scrollingBitmap);
            AsCLightColumn(p)->scrollingBitmap = 0;
        }
    if (p != 0) {
        *(void **)p = &s_particleVtable;
        void *obj = AsCLightColumn(p)->ownedObject2;
        if (obj != 0) {
            if (obj != 0) {
                ((ShotOwnedObject*)obj)->destroy(1);
            }
            AsCLightColumn(p)->ownedObject2 = 0;
        }
        obj = AsCLightColumn(p)->ownedObject1;
        if (obj != 0) {
            if (obj != 0) {
                ((ShotOwnedObject*)obj)->destroy(1);
            }
            AsCLightColumn(p)->ownedObject1 = 0;
        }
    }
        if (flags > 0)
            __dl__FPv(p);
    }
    return self;
}
#pragma use_lmw_stmw reset

extern const float kFloat_0_80000001_ShotEffects; // 0.8: ring lifetime / spark fade multiplier
extern const float kFloat_1_805DF43C;
extern float kFloat_255_SpriteAlpha;
extern float kFloat_20_ShotEffects;
extern const float kFloat_0_69999999_ShotEffects; // 0.7: initial streak length / sand-cloud opacity
extern float kFloat_neg1_ShotEffects; // -1: reverse-vector scale / vertical-velocity threshold

extern const double kDouble_SignedIntConversionBias_Particles;
extern const float kFloat_0_1_Particle;

enum { kStreakVelocityRandomValues = 41, kStreakLifeRandomValues = 10, kStreakRecordBytes = 0x28 };
// Seed shot streaks from ball history sample 3, with random velocity and lifetime.
void CShotParticle_SpawnStreaks(void *self) {
    char * p;
    int particleByteOffset;
    void * ball;
    char * historySample;
    float velocityJitterHalfRange;
    float historyVelocityScale;
    float slowdownLifeFraction;
    float lifeRandomDivisor;
    float minimumLife;
    float initialTrailLength;
    int particleIndex;
    int randomValue;
    p = (char *)self;
    ball = GolfBall__Fv();
    historySample = (char *)PreData__9CGolfBallFi(ball, 3);
    if (historySample == 0)
        return;

    historyVelocityScale = kFloat_0_1_Particle;
    velocityJitterHalfRange = kFloat_20_ShotEffects;
    initialTrailLength = kFloat_0_69999999_ShotEffects;
    minimumLife = kFloat_0_5_Particle;
    lifeRandomDivisor = kFloat_10_CourseEffects;
    slowdownLifeFraction = kFloat_0_6_ShotEffects;
    // Seed world position/velocity, then lifetime and its slowdown threshold.
    particleIndex = 0;
    particleByteOffset = 0;
    for (; particleIndex < *(int *)(p + IFF_OFFSET(CShotParticle, particleCapacity)); ++particleIndex, particleByteOffset += kStreakRecordBytes) {
        ((ShotStreak*)((char*)*(char **)(p + IFF_OFFSET(CShotParticle, particles)) + particleByteOffset))->active = 1;
        Vector_CopyComponents(&((ShotStreak*)((char*)*(char **)(p + IFF_OFFSET(CShotParticle, particles)) + particleByteOffset))->position, ball);
        randomValue = rand();
        ((ShotStreak*)((char*)*(char **)(p + IFF_OFFSET(CShotParticle, particles)) + particleByteOffset))->velocity.x = (float)(randomValue % kStreakVelocityRandomValues) + historyVelocityScale * ((Ball*)historySample)->velocity.x - velocityJitterHalfRange;
        randomValue = rand();
        ((ShotStreak*)((char*)*(char **)(p + IFF_OFFSET(CShotParticle, particles)) + particleByteOffset))->velocity.y = (float)(randomValue % kStreakVelocityRandomValues) + historyVelocityScale * ((Ball*)historySample)->velocity.y - velocityJitterHalfRange;
        randomValue = rand();
        ((ShotStreak*)((char*)*(char **)(p + IFF_OFFSET(CShotParticle, particles)) + particleByteOffset))->velocity.z = (float)(randomValue % kStreakVelocityRandomValues) + historyVelocityScale * ((Ball*)historySample)->velocity.z - velocityJitterHalfRange;
        ((ShotStreak*)((char*)*(char **)(p + IFF_OFFSET(CShotParticle, particles)) + particleByteOffset))->trailLength = initialTrailLength;
        randomValue = rand();
        ((ShotStreak*)((char*)*(char **)(p + IFF_OFFSET(CShotParticle, particles)) + particleByteOffset))->remainingLife = minimumLife + minimumLife * (float)(randomValue % kStreakLifeRandomValues) / lifeRandomDivisor;
        ((ShotStreak*)((char*)*(char **)(p + IFF_OFFSET(CShotParticle, particles)) + particleByteOffset))->slowdownThreshold = slowdownLifeFraction * ((ShotStreak*)((char*)*(char **)(p + IFF_OFFSET(CShotParticle, particles)) + particleByteOffset))->remainingLife;
    }
    AsCShotParticle(p)->activeParticleCount = *(int *)(p + IFF_OFFSET(CShotParticle, particleCapacity));
    AsCShotParticle(p)->active = 1;
}

// Allocate sixteen Power Shot ring quads; initialize fixed UVs/white RGB and flush.
// Fresh self returns self, owning a retained vertex buffer. Creation/map failures
// are unchecked. Positions/alpha are supplied by draw, not initialized here.
// Quad UVs are (1,0),(1,1),(0,1),(0,0); packed vertices are 0x18 bytes. Expanded
// byte stores retain the original order. Embedded vertex pointer order is 0,1,3,2
// (not 0,1,2,3); keep it when editing the renderer-facing pointer list.
void *CPowerShot_Ctor(void *self) {
    char *vertexStorage0 = ((Base*)self)->vertices[0];
    char *vertexStorage1 = ((Base*)self)->vertices[1];
    char *vertexStorage3 = ((Base*)self)->vertices[3];
    char *vertexStorage2 = ((Base*)self)->vertices[2];
    int colorQuadByteOffset = 0;
    char *ring = (char *)self;
    AsCPowerShot(ring)->textureId = 0;
    AsCPowerShot(ring)->active = 0;
    (*(int*)&AsCPowerShot(ring)->ownedObject1) = 0;
    (*(int*)&AsCPowerShot(ring)->ownedObject2) = 0;
    AsCPowerShot(ring)->vertex0 = (void *)vertexStorage0;
    AsCPowerShot(ring)->vertex1 = (void *)vertexStorage1;
    AsCPowerShot(ring)->vertex2 = (void *)vertexStorage3;
    AsCPowerShot(ring)->vertex3 = (void *)vertexStorage2;
    AsCPowerShot(ring)->vertexListTerminator = 0;
    *(void **)ring = &__vt__10CPowerShot;
    VertexBuffer_Create(kRingBufferBytes, 0, kPositionColorUvVertexFormat, 0, (void **)(ring + IFF_OFFSET(CPowerShot, vertexBuffer)), 0);
    void * data;
    Resource_GetOffsetPtr(AsCPowerShot(ring)->vertexBuffer, 0, 0, &data, 0);
    float uvZero = kFloat_0_Particles;
    float one = kFloat_1_805DF43C;
    unsigned char whiteChannel = 0xff;
    int uvQuadByteOffset = 0;
    for (int ctr = kPowerShotSegments; ctr != 0; --ctr) {
        char *uvQuad = (char *)data + uvQuadByteOffset;
        *(float *)(uvQuad + 0x10) = one;
        uvQuad = (char *)data + uvQuadByteOffset;
        *(float *)(uvQuad + 0x14) = uvZero;
        uvQuad = (char *)data + uvQuadByteOffset;
        *(float *)(uvQuad + 0x28) = one;
        uvQuad = (char *)data + uvQuadByteOffset;
        *(float *)(uvQuad + 0x2c) = one;
        uvQuad = (char *)data + uvQuadByteOffset;
        *(float *)(uvQuad + 0x40) = uvZero;
        uvQuad = (char *)data + uvQuadByteOffset;
        *(float *)(uvQuad + 0x44) = one;
        uvQuad = (char *)data + uvQuadByteOffset;
        *(float *)(uvQuad + 0x58) = uvZero;
        char *lastUvQuad = (char *)data + uvQuadByteOffset;
        uvQuadByteOffset = uvQuadByteOffset + kRingQuadBytes;
        *(float *)(lastUvQuad + 0x5c) = uvZero;
        char *colorQuad = (char *)data + colorQuadByteOffset;
        *(unsigned char *)(colorQuad + 0xe) = whiteChannel;
        colorQuad = (char *)data + colorQuadByteOffset;
        *(unsigned char *)(colorQuad + 0xd) = whiteChannel;
        colorQuad = (char *)data + colorQuadByteOffset;
        *(unsigned char *)(colorQuad + 0xc) = whiteChannel;
        colorQuad = (char *)data + colorQuadByteOffset;
        *(unsigned char *)(colorQuad + 0x26) = whiteChannel;
        colorQuad = (char *)data + colorQuadByteOffset;
        *(unsigned char *)(colorQuad + 0x25) = whiteChannel;
        colorQuad = (char *)data + colorQuadByteOffset;
        *(unsigned char *)(colorQuad + 0x24) = whiteChannel;
        colorQuad = (char *)data + colorQuadByteOffset;
        *(unsigned char *)(colorQuad + 0x3e) = whiteChannel;
        colorQuad = (char *)data + colorQuadByteOffset;
        *(unsigned char *)(colorQuad + 0x3d) = whiteChannel;
        colorQuad = (char *)data + colorQuadByteOffset;
        *(unsigned char *)(colorQuad + 0x3c) = whiteChannel;
        colorQuad = (char *)data + colorQuadByteOffset;
        *(unsigned char *)(colorQuad + 0x56) = whiteChannel;
        colorQuad = (char *)data + colorQuadByteOffset;
        *(unsigned char *)(colorQuad + 0x55) = whiteChannel;
        char *lastColorQuad = (char *)data + colorQuadByteOffset;
        colorQuadByteOffset = colorQuadByteOffset + kRingQuadBytes;
        *(unsigned char *)(lastColorQuad + 0x54) = whiteChannel;
    }
    GraphicsBuffer_Flush(AsCPowerShot(ring)->vertexBuffer);
    return self;
}

// Build line segments for live shot streaks and draw the compacted vertex buffer.
void CShotParticle_DrawStreaks(void *self) {
    char * p;
    void * resource;
    int liveStreakCount;
    void * data;
    int particleByteOffset;
    int headVertexByteOffset;
    int tailVertexByteOffset;
    int particleIndex;
    p = (char *)self;
    if (!AsCShotParticle(p)->active) return;
    resource = AsCShotParticle(p)->vertexBuffer;
    liveStreakCount = 0;
    Resource_GetOffsetPtr(resource, 0, 0, &data, 0);
    particleIndex = 0;
    particleByteOffset = 0;
    tailVertexByteOffset = 0;
    headVertexByteOffset = 0x10;
    for (; particleIndex < AsCShotParticle(p)->particleCapacity; ) {
        ShotStreak* particle = (ShotStreak*)((*(char **)&((CShotParticle*)p)->particles) + particleByteOffset);
        if (particle->active) {
            float reverseDirection = kFloat_neg1_ShotEffects;
            float tailPosition[3];
            Vec3_Scale(tailPosition, &particle->velocity, reverseDirection);
            Vec3_Normalize(tailPosition, tailPosition);
            Vec3_Scale(tailPosition, tailPosition, *(float *)((*(char **)&((CShotParticle*)p)->particles) + particleByteOffset + 0x20));
            Vec3_Add(tailPosition, tailPosition, &((ShotStreak*)((char*)((CShotParticle*)p)->particles + particleByteOffset))->position);
            Vector_CopyComponents((char *)data + tailVertexByteOffset, tailPosition);
            Vector_CopyComponents((char *)data + headVertexByteOffset, &((ShotStreak*)((char*)((CShotParticle*)p)->particles + particleByteOffset))->position);
            tailVertexByteOffset += 0x20;
            headVertexByteOffset += 0x20;
            ++liveStreakCount;
        }
        particleByteOffset += 0x28;
        ++particleIndex;
    }
    GraphicsBuffer_Flush(AsCShotParticle(p)->vertexBuffer);
    particleByteOffset = (int)s_shotAndRippleRenderStateLists;
    int first = *(int *)particleByteOffset;
    int renderStates[10];
    renderStates[0] = first;
    renderStates[1] = ((int *)particleByteOffset)[1];
    renderStates[2] = ((int *)particleByteOffset)[2];
    renderStates[3] = ((int *)particleByteOffset)[3];
    renderStates[4] = ((int *)particleByteOffset)[4];
    renderStates[5] = ((int *)particleByteOffset)[5];
    renderStates[6] = ((int *)particleByteOffset)[6];
    renderStates[7] = ((int *)particleByteOffset)[7];
    renderStates[8] = ((int *)particleByteOffset)[8];
    renderStates[9] = ((int *)particleByteOffset)[9];
    Engine_SetRenderStateList(renderStates, 5);
    void *camera = Camera_GetRenderState(g_view);
    SetTransform(1, ((CameraMatrices*)camera)->transform1);
    SetTransform(0, ((CameraMatrices*)camera)->transform0);
    Engine_SetType7Resource(0);
    SetTransform(2, kIdentityMatrix4x4);
    SetTransform(11, kIdentityMatrix4x4);
    Engine_SetFlexibleVertexFormat(kPositionColorVertexFormat);
    Engine_SetVertexStream(0, AsCShotParticle(p)->vertexBuffer, 0, 16);
    Engine_SetIndexBuffer(AsCShotParticle(p)->indexBuffer, 0);
    Engine_SetTextureStageResource(0, 0);
    Engine_DrawIndexedPrimitive(0xa8, 0, 0, liveStreakCount * 2, 0, liveStreakCount);
}

// Expand the active Power Shot ring and fade its alpha against elapsed lifetime.
// delta is seconds on the particle-effect task clock; inactive self is untouched. Writes
// radius/elapsedTime/alpha, then clears active only for elapsed>lifetime (strict).
// Large deltas can store negative alpha before deactivation; keep that behavior.
// Render consumes alpha as a byte. No resources are acquired or released.
void CPowerShot_GrowRingFadeAlpha(void *self, float delta) {
    char * p;
    float lifeFraction;
    float lifetime;
    float elapsed;
    float previousRadius;
    float maxAlpha;
    float remainingFraction;
    p = (char *)self;
    if (AsCPowerShot(p)->active) {
        maxAlpha = kFloat_255_SpriteAlpha;
        lifetime = kFloat_0_80000001_ShotEffects;
        elapsed = AsCPowerShot(p)->elapsedTime + delta;
        previousRadius = AsCPowerShot(p)->radius;
        lifeFraction = elapsed / lifetime;
        remainingFraction = kFloat_1_805DF43C - lifeFraction;
        AsCPowerShot(p)->radius = kFloat_20_ShotEffects * delta + previousRadius;
        AsCPowerShot(p)->elapsedTime = elapsed;
        AsCPowerShot(p)->alpha = (int)(maxAlpha * remainingFraction);
        if (elapsed > lifetime)
            AsCPowerShot(p)->active = 0;
    }
}

// Integrate shot streaks, shrink late-life velocity/tails and expire live particles.
// Required constructed self owns particleCapacity contiguous 0x28-byte records;
// activeParticleCount tracks active flags. Inactive effect/records are untouched.
// delta advances positions/lifetimes; remainingLife<slowdownThreshold triggers
// velocity*=remaining/threshold and trailLength*=.9 once per call before movement.
// remainingLife<0 clears the flag/count; count==0 deactivates the effect.
// Preserve scaling-before-integration and strict tests; these are frame rate knobs.
void CShotParticle_Update(void *self, float delta) {
    char *p = (char *)self;
    if (AsCShotParticle(p)->active) {
        float zero, trailDecay;
        int particleByteOffset, particleIndex, inactive = 0;
        trailDecay = kFloat_0_89999998_ShotEffects;
        zero = kFloat_0_Particles;
        particleIndex = 0;
        particleByteOffset = 0;
        while (particleIndex < AsCShotParticle(p)->particleCapacity) {
            ShotStreak* particle = (ShotStreak*)((*(char**)&AsCShotParticle(p)->particles) + particleByteOffset);
            if (particle->active) {
                float slowdownThreshold, remainingLife;
                remainingLife = particle->remainingLife;
                slowdownThreshold = particle->slowdownThreshold;
                if (remainingLife < slowdownThreshold) {
                    Vec3_Scale(&particle->velocity, &particle->velocity, remainingLife / slowdownThreshold);
                    particle->trailLength = particle->trailLength * trailDecay;
                }
                float temp[3];
                Vec3_Scale(temp, &particle->velocity, delta);
                Vec3_Add(&particle->position, &particle->position, temp);
                float remaining = particle->remainingLife - delta;
                particle->remainingLife = remaining;
                if (remaining < zero) {
                    particle->active = inactive;
                    --AsCShotParticle(p)->activeParticleCount;
                }
            }
            particleByteOffset += 0x28;
            ++particleIndex;
        }
        if (AsCShotParticle(p)->activeParticleCount == 0)
            AsCShotParticle(p)->active = 0;
    }
}

// Release the streak array, retained vertex buffer, and owned sprites; optionally free self.
void *CShotParticle_dtor(void *self, int flags) {
    char *p = (char *)self;
    if (p != 0) {
        void *array = AsCShotParticle(p)->particles;
        *(void **)p = &__vt__13CShotParticle;
        if (array != 0) {
            __dla__FPv(array);
            AsCShotParticle(p)->particles = 0;
        }
        void *resource = AsCShotParticle(p)->vertexBuffer;
        if (resource != 0) {
            ResourceRefBits* refs = (ResourceRefBits*)resource;
            --refs->refs;
            if (refs->refs == 0) CubeResourceStorage_dtor(resource, 1);
            AsCShotParticle(p)->vertexBuffer = 0;
        }
        resource = AsCShotParticle(p)->indexBuffer;
        if (resource != 0) {
            ResourceRefBits* refs = (ResourceRefBits*)resource;
            --refs->refs;
            if (refs->refs == 0) CubeResourceStorage_dtor(resource, 1);
            AsCShotParticle(p)->indexBuffer = 0;
        }
    if (p != 0) {
        *(void **)p = &s_particleVtable;
        void *obj = AsCShotParticle(p)->ownedResource2;
        if (obj != 0) {
            if (obj != 0) {
                ((ShotOwnedObject*)obj)->destroy(1);
            }
            AsCShotParticle(p)->ownedResource2 = 0;
        }
        obj = AsCShotParticle(p)->ownedResource1;
        if (obj != 0) {
            if (obj != 0) {
                ((ShotOwnedObject*)obj)->destroy(1);
            }
            AsCShotParticle(p)->ownedResource1 = 0;
        }
    }
    if (flags > 0)
        __dl__FPv(p);
    }
    return self;
}

struct V3i { int x, y, z; };
struct LRec { char pad0[0xc]; V3i pos; unsigned char par; char pad1[7]; };

// Set world-space hole-marker width and advance UV scroll plus call-based alpha fades.
// Required self must have loaded bitmaps; initialized==0 or suppressed golf flags
// return before ANY width/UV/alpha update. Camera/hole/ball pointers are borrowed.
// delta shares the effect clock with scrolling (2 V units per delta unit); alpha
// ignores delta and steps by kColumnAlphaStep even at delta==0. Fade modes 1/2
// saturate both alpha bytes then return to idle only when both reach the endpoint.
// No allocation, bitmap ownership change or draw submission occurs here.
// A frame rate mod must treat alpha and UV scroll separately; safe width/UV ranges
// beyond the current resource setup have not been established.
void CLightColumn_UpdateWidthScrollAndFade(void *self, float delta) {
    char *column = (char *)self;
    if (!AsCLightColumn(column)->initialized)
        return;
    void *sharedDocument = *(void **)(&g_sharedDoc);
    char *golfDocument = (char*)((CSharedDoc*)sharedDocument)->golfDoc;
    if ((AsCGolfDoc(golfDocument)->gameStateFlags & kEffectSuppressedGameStates) != 0)
        return;
    unsigned int holeIndex = AsCGolfDoc(golfDocument)->currentHoleIndex;
    LRec *holeRecord = (LRec *)((char*)AsCGolfDoc(golfDocument)->holeRecords) + holeIndex;
    float horizontalCameraOffset[3];
    {
        V3i src = holeRecord->pos;
        __mi__FRC7WVectorRC7WVector(horizontalCameraOffset, AsCameraTransform800D(s_cameraTransform)->position, &src);
    }
    horizontalCameraOffset[1] = kFloat_0_Particles;
    float widthScale = kFloat_0_0099999998_ShotEffects * (*(WVector*)horizontalCameraOffset).Magnitude();
    void *ball = GolfBall__Fv();
    float ballScale = ((Ball*)ball)->effectScale;
    float minimumWidthScale = kFloat_10_CourseEffects * ballScale;
    widthScale = minimumWidthScale < widthScale ? widthScale : minimumWidthScale;
    AsCLightColumn(column)->columnWidthScale = widthScale;
    // Scrolling is time-scaled, but alpha advances by 20 per update call in fade modes 1/2.
    AsCLightColumn(column)->textureScrollV -= kFloat_2_0 * delta;
    int mode = AsCLightColumn(column)->fadeMode;
    if (mode == kColumnFadeIn) {
        int scrollingFadeInAlpha = kOpaqueAlpha;
        int scrollingAlpha = AsCLightColumn(column)->scrollingColumnAlpha + kColumnAlphaStep;
        if (scrollingAlpha < kOpaqueAlpha) scrollingFadeInAlpha = scrollingAlpha;
        AsCLightColumn(column)->scrollingColumnAlpha = (unsigned char)scrollingFadeInAlpha;
        int frameAlpha = AsCLightColumn(column)->frameColumnAlpha + kColumnAlphaStep;
        int frameFadeInAlpha = frameAlpha < kOpaqueAlpha ? frameAlpha : kOpaqueAlpha;
        AsCLightColumn(column)->frameColumnAlpha = (unsigned char)frameFadeInAlpha;
        if (AsCLightColumn(column)->scrollingColumnAlpha == kOpaqueAlpha && (unsigned char)frameFadeInAlpha == kOpaqueAlpha)
            AsCLightColumn(column)->fadeMode = kColumnFadeIdle;
    } else if (mode == kColumnFadeOut) {
        int scrollingAlpha = (int)AsCLightColumn(column)->scrollingColumnAlpha - kColumnAlphaStep;
        int frameAlpha = (int)AsCLightColumn(column)->frameColumnAlpha - kColumnAlphaStep;
        scrollingAlpha = scrollingAlpha > 0 ? scrollingAlpha : 0;
        frameAlpha = frameAlpha > 0 ? frameAlpha : 0;
        AsCLightColumn(column)->scrollingColumnAlpha = (unsigned char)scrollingAlpha;
        AsCLightColumn(column)->frameColumnAlpha = (unsigned char)frameAlpha;
        if ((unsigned char)scrollingAlpha == 0 && (unsigned char)frameAlpha == 0)
            AsCLightColumn(column)->fadeMode = kColumnFadeIdle;
    }
}

typedef Vec3f V3;
struct LPlr {
    char pad0[0x2bc]; LRec *holeRecords;
    char pad1[0x358 - 0x2c0]; unsigned int flags;
    char pad2[0x3b6 - 0x35c]; unsigned char currentHoleIndex;
    char pad3[0x3c1 - 0x3b7]; unsigned char currentPlayer;
};
struct LMgr {
    char pad0[0x8c0]; unsigned char golfRuleType;
    char pad1[0xad8 - 0x8c1]; int partyGameMode;
    char pad2[0xb90 - 0xadc]; LPlr *golfDoc;
};
struct LEff {
    virtual void fn00(); virtual void fn04(); virtual void fn08(); virtual void fn0C();
    virtual void fn10(); virtual void fn14(); virtual void fn18(); virtual void fn1C();
    virtual void fn20();
    virtual void fn2C(void *, int, int, int);
};
struct LCol {
    char pad0[0x18]; unsigned char active;
    char pad1[0x30 - 0x19];
    char vertexPointerList[0x44 - 0x30];
    float minU, minV, maxU, maxV;
    int vertexArgb, vertexFrame;
    float left, bottom, right, top;
    V3 position;
    unsigned char vertexEnabled;
    char pad2[0x128 - 0x79];
    float columnHeight, columnWidthScale;
    unsigned char scrollingColumnAlpha, frameColumnAlpha;
    char pad3[0x134 - 0x132];
    int frameBitmapIds[5];
    int scrollingBitmapId;
    float textureScrollV;
};

// Clamp the relative hole score to the four available marker frames.
static inline int clamp03(int v) {
    if (v < 0) return 0;
    if (v > 3) return 3;
    return v;
}

// Hide the hole column while the document's explicit suppression flag is set.
static inline bool lightColumnBlocked(unsigned int flags) {
    if (flags & kLightColumnHiddenFlag) return true;
    return false;
}

// Draw the score-selected hole-marker frame and optional nearby scrolling strip.
// Requires loaded bitmap IDs and live view/document; does not allocate/retain them.
// Skips inactive, suppression-bit and party-mode-1 cases; the marker draws before
// the scrolling strip's 320-unit distance test. Uses world-space billboards and
// normalized/repeating UVs, not screen-pixel placement. No aspect or viewport here.
// Both golfRuleType branches currently select the same frame; retain the duplicate
// calls and integer-word vector copies (float bits, not numeric conversions).
void CLightColumn_Draw(void *self) {
    LCol *p = (LCol *)self;
    V3 out, dir;
    if (p->active != 0) {
    LMgr *m = (LMgr *)g_sharedDoc;
    LPlr *pl = m->golfDoc;
    unsigned int flags = pl->flags;
    switch (flags & kLightColumnHiddenFlag) {
    case 0: break;
    default: return;
    }
    {
    if (flags & kEffectSuppressedGameStates) return;
    if (m->partyGameMode == 1) return;
    LRec *rec = pl->holeRecords + pl->currentHoleIndex;
    {
        V3i src = rec->pos;
        __mi__FRC7WVectorRC7WVector(&out, AsCameraTransform800D(s_cameraTransform)->position, &src);
    }
    out.y = kFloat_0_Particles;

    // Phase 1: clamp strokes - par + 1 to frames 0..3 (the fifth loaded frame is unused here).
    int slot;
    if (((LMgr *)g_sharedDoc)->golfRuleType == 1) {
        LPlr *a = ((LMgr *)g_sharedDoc)->golfDoc;
        unsigned char *q = (unsigned char *)GolfPlayer_GetEntryByIndex(a, a->currentPlayer);
        LPlr *b = ((LMgr *)g_sharedDoc)->golfDoc;
        rec = b->holeRecords + b->currentHoleIndex;
        q += b->currentHoleIndex;
        slot = clamp03(q[0x28] - rec->par + 1);
    } else {
        LPlr *a = ((LMgr *)g_sharedDoc)->golfDoc;
        unsigned char *q = (unsigned char *)GolfPlayer_GetEntryByIndex(a, a->currentPlayer);
        LPlr *b = ((LMgr *)g_sharedDoc)->golfDoc;
        rec = b->holeRecords + b->currentHoleIndex;
        q += b->currentHoleIndex;
        slot = clamp03(q[0x28] - rec->par + 1);
    }

    {
        V3i src2 = rec->pos;
        __as__7WVectorFRC7WVector(&dir, (int)&src2);
    }
    // Phase 2: draw the marker in vertical slices [0,0.6H] and [0.6H,H].
    p->position = dir;
    p->left = kFloat_neg0_5_ShotEffects * p->columnWidthScale;
    p->bottom = kFloat_0_Particles;
    p->right = kFloat_0_5_Particle * p->columnWidthScale;
    p->top = kFloat_0_6_ShotEffects * p->columnHeight;
    p->vertexArgb = (p->frameColumnAlpha << 24) | kWhiteRgb;
    p->vertexEnabled = 1;
    p->vertexFrame = 0;
    p->minU = kFloat_0_Particles;
    p->minV = kFloat_0_Particles;
    p->maxU = kFloat_1_805DF43C;
    p->maxV = kFloat_1_805DF43C;
    ((LEff *)g_view)->fn2C(p->vertexPointerList, 0x20000000, -1, p->frameBitmapIds[slot] & kTextureHandleIdMask);

    p->top = p->columnHeight;
    float sliceFraction = kFloat_0_6_ShotEffects;
    p->bottom = sliceFraction * p->columnHeight;
    ((LEff *)g_view)->fn2C(p->vertexPointerList, 0x20000000, -1, p->frameBitmapIds[slot] & kTextureHandleIdMask);

    // Phase 3: scrolling strip is limited to horizontal distance 320 (squared: 102400).
    // Move it half a world unit away from the camera along the horizontal camera-to-hole axis.
    if (Vec3_CalculateLengthSquared(&out) > kFloat_102400_ShotEffects) return;

    {
        V3 half;
        __ml__FRC7WVectorf(&half, Normalize__7WVectorFv(&out), kFloat_0_5_Particle);
        __as__7WVectorFRC7WVector(&out, (int)&half);
    }

    LPlr *c = ((LMgr *)g_sharedDoc)->golfDoc;
    LRec *rec3 = c->holeRecords + c->currentHoleIndex;
    {
        V3 out2;
        V3i src3 = rec3->pos;
        __mi__FRC7WVectorRC7WVector(&out2, &src3, &out);
        __as__7WVectorFRC7WVector(&dir, (int)&out2);
    }
    p->position = dir;
    p->left = kFloat_neg0_25_ShotEffects * p->columnWidthScale;
    p->bottom = kFloat_0_Particles;
    p->right = kFloat_0_25_805DF468 * p->columnWidthScale;
    p->top = kFloat_0_6_ShotEffects * p->columnHeight;
    p->vertexArgb = (p->scrollingColumnAlpha << 24) | kWhiteRgb;
    p->minU = kFloat_0_Particles;
    p->minV = p->textureScrollV;
    p->maxU = kFloat_1_805DF43C;
    p->maxV = kFloat_200_shotParticles + p->textureScrollV;
    ((LEff *)g_view)->fn2C(p->vertexPointerList, 0x20800000, -1, p->scrollingBitmapId & kTextureHandleIdMask);

    float f128 = p->columnHeight;
    p->top = f128;
    p->bottom = kFloat_0_6_ShotEffects * f128;
    ((LEff *)g_view)->fn2C(p->vertexPointerList, 0x20800000, -1, p->scrollingBitmapId & kTextureHandleIdMask);
    }
    }
}

} // extern "C"

} // namespace ns0

// ---- merged from course_ball_shot_effects.cpp ----
namespace ns1 {
// Translation unit 0x801ADCA0-0x801AF2EC (18 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

struct EffectEntry {
    unsigned int id;
    float opacity;
    float value;
    char reserved0C[4];
    unsigned int arg;
    char reserved14[0x14];
    unsigned char active;
};

struct SmallEntry {
    SmallEntry();
    static void* operator new[](size_t size);
    char data[0x10];
};
struct LargeEntry {
    LargeEntry();
    static void* operator new[](size_t size);
    char data[0x30];
};

struct EffectObject {
    char reserved00[0x18];
    unsigned char active;
    char reserved19[3];
    void* manager;
    char reserved20[0x104];
    EffectEntry* entries;
    int entryCount;
    float divisor;
    float countdown;
};

struct EffectBase {
    // Provide the empty sprite base destructor for virtual deletion.
    virtual ~EffectBase() {}
};

struct EffectState {
    char data[0x12c];
};

extern const float kFloat_0_Particles;
extern const float kFloat_1_805DF43C;
extern unsigned int g_view;
void W3dAniSpr_SetPosition(void*, unsigned int*);
void W3dAniSpr_SetColor(void*, unsigned int);
void W3dAniSpr_DrawFrame(void*, unsigned int, float, unsigned int, unsigned int);
void* __nwa__FUl(unsigned int);
void* __as__7WVectorFRC7WVector(void*, void* = 0);
void W3dSpr_ctor(void*);
void W3dSpr_AttachTexture(void*, unsigned int);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern unsigned int g_resourceManager;
extern unsigned int g_soundManager;
extern const char __vt__5CDust[];
extern const char __vt__14CGrassParticle[];
extern const char courseEffectStrings[];
extern const char s_particleVtable[];
void __dla__FPv(void*);
void __dl__FPv(void*);
void W3dSpr_SetPos(void*, int);
void W3dSpr_SetRect(void*, float, float, float, float);
void W3dSpr_SetColor(void*, unsigned int);
void Particle_DrawRotatedBillboard(void*, unsigned int, float, unsigned int);
extern const float kFloat_128_ShotEffects; // 128: half-opacity scale
extern const float kFloat_0_5_Particle;
extern const float kFloat_255_SpriteAlpha;
extern const float kFloat_34_295296_ShotEffects; // 34.2952957: grass/droplet downward acceleration
extern const double kDouble_SignedIntConversionBias_Particles;
extern const float kFloat_2_0;
extern float kFloat_200_shotParticles;
extern const float kFloat_0_25_805DF468;
extern const float kFloat_3_1415927_Particles;
extern const float kFloat_0_0078125_ShotEffects; // 1/128: dust rotation random scale
extern const char __vt__19CBunkerShotParticle[];
void __ml__FRC7WVectorf(void*, void*, float = 0.5f);
void __apl__7WVectorFRC7WVector(void*, void*);
void* CQuadTree_GetInstance();
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
void CubeResourceStorage_dtor(void*, int);
void* GolfBall__Fv();
float Magnitude__7WVectorCFv(void*);
int rand();
void* PreData__9CGolfBallFi(void*, int);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void __ml__FfRC7WVector(void*, void*, float);
extern const float kFloat_5_ShotEffects; // 5: sand velocity jitter half-range
extern const float kFloat_2_5_ShotEffects; // 2.5: sand random-velocity scale
extern const float kFloat_20_ShotEffects;
extern const float kFloat_10_CourseEffects;
extern const float kFloat_0_6_ShotEffects;
extern const float kFloat_0_3_805DF4A4;

#pragma opt_loop_invariants off
// Seed sand sprites and streaks from ball history with independent random velocities.
// History sample 3 supplies velocities; both particle groups start at the current ball position.
void CBunkerShotParticle_SeedSpritesAndStreaks(void* object) {
    void* manager = GolfBall__Fv();
    char* self = (char*)object;
    void* state = PreData__9CGolfBallFi(manager, 3);
    if (state) {
        int i = 0;
        int offset = 0;
        for (; i < AsCBunkerShotParticle(self)->spriteParticleCount; ++i) {
            float result[3];
            float base[3];
            float scaled[3];
            float randomPosition[3];
            char* entry = (char*)AsCBunkerShotParticle(self)->spriteParticles + offset;
            __as__7WVectorFRC7WVector(entry, manager);

            float y;
            float z = (float)(rand() % 11) - kFloat_5_ShotEffects;

            y = (float)(rand() % 11) - kFloat_5_ShotEffects;

            float x = (float)(rand() % 11) - kFloat_5_ShotEffects;
            randomPosition[0] = x;
            randomPosition[1] = y;
            randomPosition[2] = z;

            __ml__FfRC7WVector(scaled, randomPosition, kFloat_2_5_ShotEffects);
            __ml__FRC7WVectorf(base, &((Ball*)state)->velocity, kFloat_0_5_Particle);

            __pl__FRC7WVectorRC7WVector(result, base, scaled);
            __as__7WVectorFRC7WVector(&((MotionRecord*)entry)->velocity, result);
            offset += 0x18;
        }
        // Sand sprites are seeded; initialize separate streak flags, velocities and fade thresholds.
        offset = 0;
        i = 0;
        for (; offset < AsCBunkerShotParticle(self)->streakParticleCount; ++offset) {
            float result[3];
            float base[3];
            float randomPosition[3];
            char* entry = (char*)AsCBunkerShotParticle(self)->streakParticles + i;
            entry[0] = 1;
            __as__7WVectorFRC7WVector(entry + 4, manager);
            float y;
            float z = (float)(rand() % 41) - kFloat_20_ShotEffects;
            y = (float)(rand() % 41) - kFloat_20_ShotEffects;
            float x = (float)(rand() % 41) - kFloat_20_ShotEffects;
            randomPosition[0] = x;
            randomPosition[1] = y;
            randomPosition[2] = z;

            __ml__FRC7WVectorf(base, &((Ball*)state)->velocity, kFloat_0_6_ShotEffects);

            __pl__FRC7WVectorRC7WVector(result, base, randomPosition);
            __as__7WVectorFRC7WVector(entry + 0x10, result);
            float t = kFloat_1_805DF43C + (kFloat_0_5_Particle * (float)(rand() % 10)) / kFloat_10_CourseEffects;
            *(float*)(entry + 0x20) = t;
            *(float*)(entry + 0x24) = kFloat_0_5_Particle * t;
            i += 0x28;
        }
        AsCBunkerShotParticle(self)->streakLength = kFloat_0_3_805DF4A4;
    }
}
#pragma opt_loop_invariants on

// Empty particle-record constructor or virtual hook; performs no work.
void DustParticle_CtorNoInit(SmallEntry*) {}
// Invoke the no-op dust-record constructor without initializing particle data.
SmallEntry::SmallEntry() { DustParticle_CtorNoInit(this); }
/* Allocate wrapper storage through the existing engine allocator. */
inline void* SmallEntry::operator new[](size_t size) { return ARRAY_NEW(size); }
// Empty particle-record constructor or virtual hook; performs no work.
void GrassParticleRecord_CtorNoInit(LargeEntry*) {}
// Invoke the no-op grass-record constructor without initializing particle data.
LargeEntry::LargeEntry() { GrassParticleRecord_CtorNoInit(this); }
/* Allocate wrapper storage through the existing engine allocator. */
inline void* LargeEntry::operator new[](size_t size) { return ARRAY_NEW(size); }
// Release owned particles, sprites, and buffers; optionally delete the effect object.
void* CDust_dtor(void* object, int deleting) {
    if (object) {
        char* self = (char*)object;
        void* entries = AsCDust(self)->particles;
        *(void**)self = (void*)__vt__5CDust;
        if (entries) {
            if (entries) {
                FreeArrayBlock(entries);
            }
            AsCDust(self)->particles = 0;
        }
        void* child = AsCDust(self)->sprite;
        if (child) {
            delete (EffectBase*)child;
            AsCDust(self)->sprite = 0;
        }
        if (self) {
            child = AsCDust(self)->sprite;
            *(void**)self = (void*)s_particleVtable;
            if (child) {
                delete (EffectBase*)child;
                AsCDust(self)->sprite = 0;
            }
            child = AsCDust(self)->ownedAuxiliaryObject;
            if (child) {
                delete (EffectBase*)child;
                AsCDust(self)->ownedAuxiliaryObject = 0;
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return object;
}
// Release owned particles, sprites, and buffers; optionally delete the effect object.
void* CGrassParticle_dtor(void* object, int deleting) {
    if (object) {
        char* self = (char*)object;
        void* entries = AsCGrassParticle(self)->particles;
        *(void**)self = (void*)__vt__14CGrassParticle;
        if (entries) {
            if (entries) {
                FreeArrayBlock(entries);
            }
            AsCGrassParticle(self)->particles = 0;
        }
        entries = 0;
        void* child = AsCGrassParticle(self)->sprite;
        if (child) {
            delete (EffectBase*)child;
            AsCGrassParticle(self)->sprite = 0;
        }
        if (self) {
            child = AsCGrassParticle(self)->ownedAuxiliaryObject;
            *(void**)self = (void*)s_particleVtable;
            if (child) {
                delete (EffectBase*)child;
                AsCGrassParticle(self)->ownedAuxiliaryObject = 0;
            }
            child = AsCGrassParticle(self)->sprite;
            if (child) {
                delete (EffectBase*)child;
                AsCGrassParticle(self)->sprite = 0;
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return object;
}
// Empty particle-record constructor or virtual hook; performs no work.
void BunkerSpriteParticle_CtorNoInit(void*) {}
// Empty particle-record constructor or virtual hook; performs no work.
void BunkerStreakParticle_CtorNoInit(void*) {}

void VertexBuffer_Create(int, int, int, int, void**, int);
void Resource_GetOffsetPtr(void*, int, int, void*, int);
void GraphicsBuffer_Flush(void*);
void IndexBuffer_Create(int, int, int, int, void**, int, int);
void IndexBuffer_GetOffsetPtr(void*, int, int, void*, int);
void IndexBuffer_Flush(void*);

// Recreate the dust sprite and seed positions, rotations, sizes, and fade lifetime.
void CDust_BeginEffect(void* object) {
    char* self = (char*)object;
    void* child = AsCDust(self)->sprite;
    if (child) {
        delete (EffectBase*)child;
        AsCDust(self)->sprite = 0;
    }
    EffectState* state = new EffectState;
    if (state)
        {
            try {
                W3dSpr_ctor(state);
            } catch (...) {
                __dl__FPv(state);
                throw;
            }
        }
    AsCDust(self)->sprite = state;
    (*(unsigned int*)&((W3dSpr*)state)->resourceManager) = g_resourceManager;
    W3dSpr_AttachTexture(AsCDust(self)->sprite, AsCDust(self)->bitmapId);
    float half;
    register float one = kFloat_1_805DF43C;
    float scale = (*(WVector*)(&((Ball*)GolfBall__Fv())->velocity)).Magnitude();
    half = kFloat_0_5_Particle;
    register float speedScaledSize = half * scale;
    float one2 = kFloat_1_805DF43C;
    register double magic = kDouble_SignedIntConversionBias_Particles;
    int index = 0;
    int offset = 0;
    while (index < AsCDust(self)->particleCount) {
        *(WVector*)((char*)AsCDust(self)->particles + offset) = *(WVector*)(self + 8);
        int value = rand() % 128;
        index = index + 1;
        int curOffset = offset;
        offset = offset + 0x10;
        DustAt(AsCDust(self)->particles, curOffset)->angle = (((float)value * kFloat_0_0078125_ShotEffects - half) * kFloat_2_0) * kFloat_3_1415927_Particles * kFloat_0_25_805DF468;
    }
    (void)magic;
    float low = one > speedScaledSize ? speedScaledSize : one;
    AsCDust(self)->minParticleSize = low;
    speedScaledSize = (one < speedScaledSize) ? speedScaledSize : one;
    AsCDust(self)->remainingLife = kFloat_1_805DF43C;
    AsCDust(self)->maxParticleSize = speedScaledSize;
    AsCDust(self)->fadeDuration = one2;
    AsCDust(self)->active = 1;
}

typedef Vec3f Vec3Copy;

// Query terrain height beneath a grass blade and test whether the blade has fallen below it.
inline int GrassParticle_IsBelowGround(Grass* entry) {
    float transformed[3];
    Vec3Copy matrix = entry->position;
    GetGroundPoint__9CQuadTreeF7WVectorbPib(transformed, CQuadTree_GetInstance(), &matrix, 0, 0, 0);
    float y = transformed[1];
    return entry->position.y < y;
}

// Integrate grass motion and spin; deactivate blades below the ground or with no opacity.
void CGrassParticle_UpdateBlades(void* object, float delta) {
    char* self = (char*)object;
    if (AsCGrassParticle(self)->active) {
        int bladeByteOffset;
        float alphaRate = kFloat_255_SpriteAlpha;
        int anyActive = 0;
        int bladeIndex = 0;
        bladeByteOffset = 0;
        while (bladeIndex < AsCGrassParticle(self)->particleCount) {
            int savedOffset = bladeByteOffset;
            Grass* blade = (Grass*)((char*)AsCGrassParticle(self)->particles + savedOffset);
            if (blade->active) {

                blade->opacity -= alphaRate * (delta / blade->lifetime);
                blade->velocity.y -= kFloat_34_295296_ShotEffects * delta;

                WVector displacement = *(WVector*)(&blade->velocity) * delta;
                *(WVector*)(&blade->position) += displacement;
                blade->angle += blade->angularVelocity * delta;
                int expired = blade->opacity < kFloat_0_Particles || GrassParticle_IsBelowGround(blade);
                if (expired)
                    blade->active = 0;
                else
                    anyActive = 1;
            }
            bladeByteOffset += 0x30;
            bladeIndex++;
        }
        AsCGrassParticle(self)->active = anyActive;
    }
}

struct RefCounted {
    CW_BITS2(
        unsigned int count : 16,
        unsigned int pad : 16)
};

// Release owned particles, sprites, and buffers; optionally delete the effect object.
void* CBunkerShotParticle_dtor(void* object, int deleting) {
    if (object) {
        char* self = (char*)object;
        void* ptr = AsCBunkerShotParticle(self)->spriteParticles;
        *(void**)self = (void*)__vt__19CBunkerShotParticle;
        if (ptr) {
            if (ptr) {
                FreeArrayBlock(ptr);
            }
            AsCBunkerShotParticle(self)->spriteParticles = 0;
        }
        ptr = AsCBunkerShotParticle(self)->streakParticles;
        if (ptr) {
            if (ptr) {
                FreeArrayBlock(ptr);
            }
            AsCBunkerShotParticle(self)->streakParticles = 0;
        }
        ptr = AsCBunkerShotParticle(self)->sprite;
        if (ptr) {
            delete (EffectBase*)ptr;
            AsCBunkerShotParticle(self)->sprite = 0;
        }
        ptr = AsCBunkerShotParticle(self)->vertexBuffer;
        if (ptr) {
            if (--((RefCounted*)ptr)->count == 0)
                CubeResourceStorage_dtor(ptr, 1);
            AsCBunkerShotParticle(self)->vertexBuffer = 0;
        }
        ptr = AsCBunkerShotParticle(self)->indexBuffer;
        if (ptr) {
            if (--((RefCounted*)ptr)->count == 0)
                CubeResourceStorage_dtor(ptr, 1);
            AsCBunkerShotParticle(self)->indexBuffer = 0;
        }
        (*(unsigned int*)&AsCBunkerShotParticle(self)->spriteParticleCount) = 0;
        (*(unsigned int*)&AsCBunkerShotParticle(self)->streakParticleCount) = 0;
        if (self) {
            ptr = AsCBunkerShotParticle(self)->sprite;
            *(void**)self = (void*)s_particleVtable;
            if (ptr) {
                delete (EffectBase*)ptr;
                AsCBunkerShotParticle(self)->sprite = 0;
            }
            ptr = AsCBunkerShotParticle(self)->ownedAuxiliaryObject;
            if (ptr) {
                delete (EffectBase*)ptr;
                AsCBunkerShotParticle(self)->ownedAuxiliaryObject = 0;
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return object;
}

// Initialize effect state and allocate its particle records.
EffectObject* CDust_Ctor(EffectObject* self, int count) {
    int zero = 0;
    int savedCount = count;
    EffectObject* savedSelf = self;
    AsCDust(savedSelf)->bitmapId = zero;
    self->active = 0;
    (*(unsigned int*)&AsCDust(self)->ownedAuxiliaryObject) = 0;
    (*(unsigned int*)&AsCDust(self)->sprite) = 0;
    AsCDust(self)->vertex0 = ((Base*)self)->vertices[0];
    AsCDust(self)->vertex1 = ((Base*)self)->vertices[1];
    AsCDust(self)->vertex2 = ((Base*)self)->vertices[3];
    AsCDust(self)->vertex3 = ((Base*)self)->vertices[2];
    AsCDust(self)->vertexListTerminator = 0;
    *(void**)self = (void*)__vt__5CDust;
    AsCDust(self)->particles = 0;
    AsCDust(savedSelf)->particleCount = savedCount;
    void* entries = new SmallEntry[savedCount];
    AsCDust(savedSelf)->particles = entries;
    return savedSelf;
}

// Initialize effect state and allocate its particle records.
// initialVelocity borrows a world-space three-float vector, copied before particle allocation.
EffectObject* CGrassParticle_Ctor(EffectObject* self, int count, void* initialVelocity) {
    EffectObject* savedSelf = self;
    int savedCount = count;
    void* block;
    void* entries;
    AsCGrassParticle(self)->bitmapId = 0;
    self->active = 0;
    (*(unsigned int*)&AsCGrassParticle(self)->sprite) = 0;
    (*(unsigned int*)&AsCGrassParticle(self)->ownedAuxiliaryObject) = 0;
    AsCGrassParticle(self)->vertex0 = ((Base*)self)->vertices[0];
    AsCGrassParticle(self)->vertex1 = ((Base*)self)->vertices[1];
    AsCGrassParticle(self)->vertex2 = ((Base*)self)->vertices[3];
    AsCGrassParticle(self)->vertex3 = ((Base*)self)->vertices[2];
    AsCGrassParticle(self)->vertexListTerminator = 0;
    *(void**)self = (void*)__vt__14CGrassParticle;
    AsCGrassParticle(savedSelf)->particleCount = savedCount;
    *(WVector*)AsCGrassParticle(savedSelf)->initialVelocity = *(WVector*)initialVelocity; // original CGrassParticle_Ctor +0x12C
    savedCount = AsCGrassParticle(savedSelf)->particleCount;
    entries = new LargeEntry[savedCount];
    AsCGrassParticle(savedSelf)->particles = entries;
    return savedSelf;
}

// Recreate the sand sprite, seed particles through the virtual hook, and play the bunker sound.
void CBunkerShotParticle_Begin(EffectObject* self) {
    char* p = (char*)self;
    void* child = AsCBunkerShotParticle(p)->sprite;
    if (child) {
        if (child) {
            ((void (*)(void*, int))(*(void***)child)[2])(child, 1);
        }
        AsCBunkerShotParticle(p)->sprite = 0;
    }
    EffectState* state = new EffectState;
    if (state) {
        try {
            W3dSpr_ctor(state);
        } catch (...) {
            __dl__FPv(state);
            throw;
        }
    }
    AsCBunkerShotParticle(p)->sprite = state;
    (*(unsigned int*)&((W3dSpr*)state)->resourceManager) = g_resourceManager;
    W3dSpr_AttachTexture(AsCBunkerShotParticle(p)->sprite, AsCBunkerShotParticle(p)->bitmapId);
    typedef void (*Update)(EffectObject*);
    ((Update)(*(void**)((char*)*(void**)p + 0x20)))(self);
    float three = kFloat_1_805DF43C;
    AsCBunkerShotParticle(p)->fadeRemaining = three;
    AsCBunkerShotParticle(p)->fadeDuration = three;
    Sound_PlayByName((void*)g_soundManager,((CourseEffectStringPool*)courseEffectStrings)->bunkerSandSound,
                     0, 0, 0, 0, kFloat_0_5_Particle, kFloat_200_shotParticles, three);
    AsCBunkerShotParticle(p)->active = 1;
}

// Count down the dust fade and stop the effect when its normalized lifetime expires.
void CDust_UpdateFadeCountdown(EffectObject* self, float amount) {
    if (!self->active)
        return;
    float divisor = self->divisor;
    float countdown = self->countdown;
    countdown = countdown - amount / divisor;
    self->countdown = countdown;
    if (countdown < kFloat_0_Particles) {
        self->active = 0;
        self->countdown = kFloat_1_805DF43C;
    }
}

// Draw each dust billboard with lifetime-dependent size, opacity, and rotation.
void CDust_DrawParticles(EffectObject* self) {
    if (self->active) {
        float scale = kFloat_128_ShotEffects;
        int index = 0;
        float one = kFloat_1_805DF43C;
        int offset = 0;
        float half = kFloat_0_5_Particle;
        while (self->entryCount > index) {
            float progress = AsCDust(self)->remainingLife;
            float savedScale = scale;

            float start = AsCDust(self)->minParticleSize;
            float savedOne = one;
            float value = (savedOne - progress) * ((AsCDust(self)->maxParticleSize) - start) + start;
            int alpha = (int)(savedScale * progress);
                W3dSpr_SetPos(AsCDust(self)->sprite, (int)((char*)AsCDust(self)->particles + offset));
                float saved_p = (half * value); W3dSpr_SetRect(AsCDust(self)->sprite, value, value, saved_p,
                        saved_p);
            W3dSpr_SetColor(AsCDust(self)->sprite, (alpha << 24) | kWhiteRgb);
            int savedOffset2 = offset; Particle_DrawRotatedBillboard(AsCDust(self)->sprite, g_view, DustAt(AsCDust(self)->particles, savedOffset2)->angle * AsCDust(self)->remainingLife,
                        0x20400000);
            offset += 0x10;
            index = index + 1;
        }
    }
}

// Draw the active grass sprites using their frame, position, opacity, and rotation.
void CGrassParticle_DrawSprites(EffectObject* self) {
    if (self->active) {
        int alpha;
        int offset;
        int i = 0;
        offset = 0;
        while (i < self->entryCount) {
            EffectEntry* entry = (EffectEntry*)((char*)self->entries + offset);
            if (entry->active) {
                alpha = (int)entry->opacity;
                W3dAniSpr_SetPosition(self->manager, &entry->arg);
                W3dAniSpr_SetColor(self->manager, (alpha << 24) | kWhiteRgb);
                W3dAniSpr_DrawFrame(self->manager, g_view, entry->value, 0x20400000, entry->id);
            }
            offset += 0x30;
            ++i;
        }
    }
}

extern const float kFloat_64_ShotEffects; // 64: grass bitmap grid-cell size
extern const float kFloat_0_0009765625_805DF488;
extern const float kFloat_2_0;
extern float kFloat_200_shotParticles;
extern const float kFloat_3_1415927_Particles;
extern const float kFloat_0_2_ShotEffects;
extern const float kFloat_3_ShotEffects; // 3: velocity jitter bias / coin gravity
extern const float kFloat_0_001953125_ShotEffects; // 1/512: signed random-axis scale
extern const float kFloat_1_5_ShotEffects; // 1.5: droplet lifetime / initial coin bounce speed
extern const double kDouble_SignedIntConversionBias_Particles;
extern unsigned int g_resourceManager;
void* __nw__FUl(unsigned int);
void W3dAniSpr_Ctor(void*);
void W3dAniSpr_LoadTexture(void*, void*, float, float);
void W3dAniSpr_SetSizeAndPivot(void*, float, float, float, float);

// Recreate the grass sprite grid and initialize randomized blade motion, spin, and opacity.
void CGrassParticle_Begin(EffectObject* self) {
    if (self->manager) {
        delete (EffectBase*)self->manager;
        self->manager = 0;
    }
    EffectState* state = (EffectState*)__nw__FUl(0x19c);
    if (state)
        {
            try {
                W3dAniSpr_Ctor(state);
            } catch (...) {
                __dl__FPv(state);
                throw;
            }
        }
    self->manager = state;
    (*(unsigned int*)&((W3dSpr*)state)->resourceManager) = g_resourceManager;
    W3dAniSpr_LoadTexture(self->manager, (*(void**)&AsCGrassParticle(self)->bitmapId), kFloat_64_ShotEffects, kFloat_64_ShotEffects);
    W3dAniSpr_SetSizeAndPivot(self->manager, kFloat_0_5_Particle, kFloat_0_5_Particle, kFloat_0_5_Particle * kFloat_0_5_Particle,
                kFloat_0_5_Particle * kFloat_0_5_Particle);
    /* 0x801AE79C: b 0x801AE94C - bladeIndex < entryCount is tested before the first pass (count 0: no blade
       written; ours wrote entries[0]) */
    for (int bladeIndex = 0; bladeIndex < self->entryCount; ++bladeIndex) {
        Grass* blade = (Grass*)((char*)self->entries + bladeIndex * 0x30);
        *(WVector*)(&blade->position) = *(WVector*)(GolfBall__Fv());
        blade->position.y -= ((Ball*)GolfBall__Fv())->effectScale;
        blade->active = 1;
        int randomValue = rand() % 0x3ff;
        blade->angle = (float)randomValue * kFloat_0_0009765625_805DF488 * kFloat_2_0 * kFloat_3_1415927_Particles;
        randomValue = rand() % 0x3ff;
        blade->angularVelocity = ((float)randomValue * kFloat_0_0009765625_805DF488 - kFloat_0_5_Particle) * kFloat_2_0 * kFloat_2_0 * kFloat_3_1415927_Particles;
        float* initialVelocity = AsCGrassParticle(self)->initialVelocity; // original CGrassParticle_Begin +0x12C
        randomValue = rand() % 0x401;
        blade->velocity.x = (float)(randomValue - 0x200) * (kFloat_0_2_ShotEffects * initialVelocity[0] + kFloat_3_ShotEffects) * kFloat_0_001953125_ShotEffects + initialVelocity[0];
        randomValue = rand() % 0x401;
        blade->velocity.y = (float)(randomValue - 0x200) * (kFloat_0_2_ShotEffects * initialVelocity[1] + kFloat_3_ShotEffects) * kFloat_0_001953125_ShotEffects + initialVelocity[1];
        randomValue = rand() % 0x401;
        blade->velocity.z = (float)(randomValue - 0x200) * (kFloat_0_2_ShotEffects * initialVelocity[2] + kFloat_3_ShotEffects) * kFloat_0_001953125_ShotEffects + initialVelocity[2];
        blade->frame = rand() % 4;
        blade->lifetime = kFloat_1_5_ShotEffects;
        blade->opacity = kFloat_255_SpriteAlpha;
    }
    self->active = 1;
}

struct EffectColor {
    unsigned char r, g, b, a;
};
struct EffectVtx {
    char pad0[0xc];
    EffectColor c0;
    char pad10[0xc];
    EffectColor c1;
};

struct BunkerEntry18 {
    BunkerEntry18();
    static void* operator new[](size_t size);
    char data[0x18];
};
struct BunkerEntry28 {
    BunkerEntry28();
    static void* operator new[](size_t size);
    char data[0x28];
};
// Invoke the no-op sand-sprite record constructor.
BunkerEntry18::BunkerEntry18() { BunkerSpriteParticle_CtorNoInit(this); }
/* Allocate wrapper storage through the existing engine allocator. */
inline void* BunkerEntry18::operator new[](size_t size) { return ARRAY_NEW(size); }
// Invoke the no-op sand-streak record constructor.
BunkerEntry28::BunkerEntry28() { BunkerStreakParticle_CtorNoInit(this); }
/* Allocate wrapper storage through the existing engine allocator. */
inline void* BunkerEntry28::operator new[](size_t size) { return ARRAY_NEW(size); }

#pragma use_lmw_stmw on
// Initialize effect state and allocate its particle records.
// count1 = sand sprite count, count2 = streak count. The arrays have 0x18/0x28-byte records.
// Buffers are allocated only for positive streak count; each streak uses two RGBA vertices.
void* CBunkerShotParticle_Ctor(char* p, int count1, int count2) {
    (*(int*)&AsCBunkerShotParticle(p)->bitmapId) = 0;
    AsCBunkerShotParticle(p)->active = 0;
    (*(int*)&AsCBunkerShotParticle(p)->ownedAuxiliaryObject) = 0;
    (*(int*)&AsCBunkerShotParticle(p)->sprite) = 0;
    AsCBunkerShotParticle(p)->vertex0 = ((Base*)p)->vertices[0];
    AsCBunkerShotParticle(p)->vertex1 = ((Base*)p)->vertices[1];
    AsCBunkerShotParticle(p)->vertex2 = ((Base*)p)->vertices[3];
    AsCBunkerShotParticle(p)->vertex3 = ((Base*)p)->vertices[2];
    AsCBunkerShotParticle(p)->vertexListTerminator = 0;
    *(void**)p = (void*)__vt__19CBunkerShotParticle;
    AsCBunkerShotParticle(p)->spriteParticles = 0;
    (*(int*)&AsCBunkerShotParticle(p)->streakParticles) = 0;
    AsCBunkerShotParticle(p)->spriteParticleCount = count1;
    AsCBunkerShotParticle(p)->streakParticleCount = count2;
    AsCBunkerShotParticle(p)->particleRgb = 0x00F8E9AB;
    if (count1 > 0) {
        AsCBunkerShotParticle(p)->spriteParticles = new BunkerEntry18[count1];
    }
    // Initialize fixed streak colors/endpoint alpha, then sequential 16-bit line indices.
    if (count2 > 0) {
        AsCBunkerShotParticle(p)->streakParticles = new BunkerEntry28[count2];
        VertexBuffer_Create(count2 << 5, 0, kPositionColorVertexFormat, 0, (void**)(p + IFF_OFFSET(CBunkerShotParticle, vertexBuffer)), 0);
        unsigned char* colors;
        Resource_GetOffsetPtr(AsCBunkerShotParticle(p)->vertexBuffer, 0, 0, &colors, 0);
        for (int i = 0; i < count2; ++i) {
            for (int j = 0; j < 2; ++j) {
                colors[i * 32 + j * 16 + 15] = j * kOpaqueAlpha;
                colors[i * 32 + j * 16 + 12] = 0xa0;
                colors[i * 32 + j * 16 + 13] = 0x80;
                colors[i * 32 + j * 16 + 14] = 0;
            }
        }
        GraphicsBuffer_Flush(AsCBunkerShotParticle(p)->vertexBuffer);
        IndexBuffer_Create(count2 << 2, 0, 3, 0, (void**)(p + IFF_OFFSET(CBunkerShotParticle, indexBuffer)), 0, 0);
        void* indices;
        IndexBuffer_GetOffsetPtr(AsCBunkerShotParticle(p)->indexBuffer, 0, 0, &indices, 0);
        for (int i = 0; i < count2 * 2; ++i)
            ((unsigned short*)indices)[i] = (unsigned short)i;
        IndexBuffer_Flush(AsCBunkerShotParticle(p)->indexBuffer);
    }
    return p;
}
#pragma use_lmw_stmw reset

} // extern "C"

} // namespace ns1

// ---- merged from course_ball_shot_effects.cpp ----
namespace ns2 {
// Translation unit 0x801AF2EC-0x801B0A54 (19 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern const float kFloat_0_Particles;
extern const float kFloat_1_805DF43C;
extern const float kFloat_32_SpriteGrid;
extern const float kFloat_9_9999997eneg06_ShotEffects; // 0.00001: near-zero threshold
extern const float s_fragmentUprightAxisY;
extern const float kFloat_5_ShotEffects; // 5: sand velocity jitter half-range
extern const float kFloat_0_1_Particle;
extern const float kFloat_20_ShotEffects;
extern const float kFloat_0_5_Particle;
extern const float kFloat_255_SpriteAlpha;
extern const float kFloat_0_0099999998_ShotEffects; // 0.01: dust spawn interval / distance-to-scale factor
extern const float kFloat_0_2_ShotEffects;
extern const float kFloat_15_ShotEffects; // 15: coin motion/spin scale / first ripple growth rate
extern float kFloat_17_ShotEffects; // 17: second ripple growth rate
extern const float kFloat_19_ShotEffects; // 19: third ripple growth rate
extern const float kFloat_128_ShotEffects; // 128: half-opacity scale
extern const float kFloat_200_shotParticles;
extern char __vt__15CBunkerParticle[0x28];
extern char s_particleVtable[0x28];
extern char __vt__8CDroplet[0x28];
extern char __vt__7CRipple[0x28];
extern unsigned int g_view;

void W3dSpr_SetRect(void*, float, float, float, float);
void W3dSpr_SetColor(void*, unsigned int);
void Billboard_DrawOrientedQuad(void*, unsigned int, unsigned int, int);
void W3dAniSpr_SetColor(void*, unsigned int);
void W3dAniSpr_SetPosition(void*, unsigned int);
void W3dAniSpr_DrawFrame(void*, unsigned int, float, unsigned int, int);
void* __nwa__FUl(unsigned int);
void* W3dSpr_Create(void*, const char*, int);
void W3dSpr_SetPos(void*, void*);
void Particle_DrawRotatedBillboard(void*, unsigned int, float, unsigned int);
void W3dSpr_AttachTexture(void*, unsigned int);
void W3dSpr_ctor(void*);
void Sound_PlayByName(unsigned int, const char*, int, int, int, int, float, float, float);
extern unsigned int g_resourceManager;
extern unsigned int g_soundManager;
void __ml__FRC7WVectorf(void*, void*, float);
void __apl__7WVectorFRC7WVector(void*, void*);
void* GolfBall__Fv();
void __as__7WVectorFRC7WVector(void*, void*);
void Matrix_SetIdentityAffine(void*);
void Vector_CrossProduct(void*, void*, void*);
void Normalize__7WVectorFv(void*);
void Vec3_Zero(void*);
void Matrix_TransformPoint(void*, void*, void*);
float Magnitude__7WVectorCFv(void*);
void Matrix_SetAxisRotation(void*, float, signed char);
void __ml__FfRC7WVector(void*, void*, float);
void Matrix_TransformPoint(void*, void*, void*);
void Object_DeleteVirtual(void*, void*);
void __dl__FPv(void*);
void __dla__FPv(void*);

struct AnimObject {
    virtual void v0(int);
    virtual void v1();
    virtual void v2(int);
};

// Release owned particles, sprites, and buffers; optionally delete the effect object.
void* CBunkerParticle_dtor(void* object, int shouldDelete) {
    if (object) {
        void** fields = (void**)object;
        void* anim = fields[8];
        void* vt = &__vt__15CBunkerParticle;
        fields[0] = vt;
        if (anim) {
            if (anim) {
                ((AnimObject*)anim)->v0(1);
            }
            fields[8] = 0;
        }
    if (object) {
        void** fields = (void**)object;
        fields[0] = &s_particleVtable;
        void* anim = fields[8];
        if (anim) {
            if (anim) {
                ((AnimObject*)anim)->v0(1);
            }
            fields[8] = 0;
        }
        anim = fields[7];
        if (anim) {
            if (anim) {
                ((AnimObject*)anim)->v0(1);
            }
            fields[7] = 0;
        }
    }
    if (shouldDelete > 0) __dl__FPv(object);
    }
    return object;
}

// Release owned particles, sprites, and buffers; optionally delete the effect object.
void* CDroplet_dtor(void* object, int shouldDelete) {
    if (object) {
        void** fields = (void**)object;
        void* anim0 = fields[8];
        void* vt = &__vt__8CDroplet;
        fields[0] = vt;
        if (anim0) {
            Object_DeleteVirtual((void*)g_resourceManager, anim0);
            fields[8] = 0;
        }
        void* entries = fields[0x124 / 4];
        if (entries) {
            if (entries) {
                FreeArrayBlock(entries);
            }
            fields[0x124 / 4] = 0;
        }
    if (object) {
        void** fields = (void**)object;
        fields[0] = &s_particleVtable;
        void* anim = fields[8];
        if (anim) {
            if (anim) {
                ((AnimObject*)anim)->v0(1);
            }
            fields[8] = 0;
        }
        anim = fields[7];
        if (anim) {
            if (anim) {
                ((AnimObject*)anim)->v0(1);
            }
            fields[7] = 0;
        }
    }
    if (shouldDelete > 0) __dl__FPv(object);
    }
    return object;
}

struct FadeEntries {
    char reserved00[0x18];
    unsigned char active;
    char reserved19[0x10B];
    void* entries;
    int count;
    char reserved12C[8];
    float value;
};
extern const float kFloat_96_ShotEffects; // 96: fragment opacity fade rate
extern const float kFloat_17_147648_ShotEffects; // 17.147648: fragment downward acceleration
// Integrate fragment motion under gravity and fade the fragment effect.
void CFragment_UpdateMotionAndFade(FadeEntries* self, float delta) {
    if (!self->active) return;
    int offset;
    float downwardAcceleration = kFloat_17_147648_ShotEffects;
    int i = 0;
    self->value = self->value - kFloat_96_ShotEffects * delta;
    offset = 0;
    char temp[0x10];
    for (; i < self->count; ++i) {
        MotionAt(self->entries, offset)->velocity.y = MotionAt(self->entries, offset)->velocity.y - downwardAcceleration * delta;
        __ml__FRC7WVectorf(temp, &MotionAt(self->entries, offset)->velocity, delta);
        *(WVector*)((char*)self->entries + offset) += *(WVector*)temp;
        offset += 0x18;
    }
    if (self->value < kFloat_0_Particles) self->active = 0;
}
extern char __vt__9CFragment[0x28];
extern const char courseEffectStrings[0x100];
extern const float kFloat_34_295296_ShotEffects; // 34.2952957: grass/droplet downward acceleration
extern const float kFloat_120_03354_ShotEffects; // 120.033539: sand-sprite downward acceleration
extern const float kFloat_0_99000001_ShotEffects; // 0.99: horizontal sand-streak damping
extern const float kFloat_102_88589_ShotEffects; // 102.885887: sand-streak downward acceleration
extern const double kDouble_SignedIntConversionBias_Particles;
extern const float kFloat_0_0009765625_805DF488;
extern const float kFloat_2_0;
extern const float kFloat_3_1415927_Particles;
extern const float kFloat_6_ShotEffects; // 6: droplet radial-speed range
extern float kFloat_40_ShotEffects; // 40: sand billboard size / droplet upward-speed bound
extern const float kFloat_10_CourseEffects;
extern const float kFloat_1_5_ShotEffects; // 1.5: droplet lifetime / initial coin bounce speed

struct MainAnim {
    char data[0x12C];
    // Construct a static billboard sprite in its allocation wrapper.
    MainAnim() { W3dSpr_ctor(this); }
};
extern "C" void FragmentParticle_CtorNoInit();

struct BunkerConstructionView {
    void* vtable;
    char reserved04[0x10];
    int bitmapId;
    unsigned char active;
    char reserved19[3];
    int animatedSprite;
    void* staticSprite;
    char reserved24[0xC];
    void* vertex0;
    void* vertex1;
    void* vertex2;
    void* vertex3;
    int vertexListTerminator;
    char reserved44[0xBC];
};

// Initialize a single expanding sand-cloud sprite.
void CBunkerParticle_Ctor(BunkerConstructionView* self) {
    self->bitmapId = 0;
    self->active = 0;
    self->animatedSprite = 0;
    self->staticSprite = 0;
    self->vertex0 = ((Base*)self)->vertices[0];
    self->vertex1 = ((Base*)self)->vertices[1];
    self->vertex2 = ((Base*)self)->vertices[3];
    self->vertex3 = ((Base*)self)->vertices[2];
    self->vertexListTerminator = 0;
    self->vtable = &__vt__15CBunkerParticle;
}

struct BunkerDrawView {
    char reserved00[0x18];
    unsigned char active;
    char reserved19[7];
    void* staticSprite;
    char reserved24[0x100];
    int opacityByte;
    float remainingLifeFraction;
    float reserved12C;
    float billboardSize;
};

// Draw the active sand cloud with its current scale and opacity.
void CBunkerParticle_DrawSandCloud(BunkerDrawView* self) {
    if (!self->active) return;

    float value = (kFloat_20_ShotEffects) * (kFloat_1_805DF43C - self->remainingLifeFraction);
    float savedValue4 = value;
    self->billboardSize = savedValue4;
    float savedValue5 = value;
    void* savedUnk = self->staticSprite;
    W3dSpr_SetRect(savedUnk, savedValue5, savedValue5, (kFloat_0_5_Particle) * savedValue5, (kFloat_0_Particles));

    float scaledOpacity = kFloat_255_SpriteAlpha * self->remainingLifeFraction;
    unsigned int alpha = (int)(scaledOpacity);
    void* savedUnk2 = self->staticSprite;
    self->opacityByte = alpha;
    W3dSpr_SetColor(savedUnk2, (((unsigned int)alpha << 24) | kWhiteRgb));
    void* savedUnk3 = self->staticSprite;
    Billboard_DrawOrientedQuad(savedUnk3, g_view, 0x20400000, 1);
}

struct FragmentDrawView {
    char reserved00[0x18];
    unsigned char active;
    char reserved19[3];
    void* animatedSprite;
    char reserved20[0x104];
    int entryArrayAddress;
    int entryCount;
    float reserved12C;
    float reserved130;
    float value;
};

// Draw all fragments at their stored positions using the effect opacity.
void CFragment_DrawFragments(FragmentDrawView* self) {
    if (!self->active) return;
    int alpha = (int)self->value;
    unsigned int color = ((unsigned int)alpha << 24) | kWhiteRgb;
    W3dAniSpr_SetColor(self->animatedSprite, color);
    for (int i = 0; i < self->entryCount; i++) {
        W3dAniSpr_SetPosition(self->animatedSprite, self->entryArrayAddress + i * 0x18);
        W3dAniSpr_DrawFrame(self->animatedSprite, g_view, kFloat_0_Particles, 0x00C00000, 7);
    }
}

struct AnimOwner {
    char reserved00[0x20];
    void* anim;
    char reserved24[0x104];
    float scale;
    float alpha;
    float reserved130;
    float opacity;
    char rest[0x8];
};

extern const float kFloat_0_69999999_ShotEffects; // 0.7: initial streak length / sand-cloud opacity
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Recreate the sand-cloud sprite, reset expansion/fade state, and play the bunker sound.
void CBunkerParticle_RestartEffect(AnimOwner* self) {
    if (self->anim) {
        if (self->anim) {
            ((AnimObject*)self->anim)->v0(1);
        }
        self->anim = 0;
    }
    MainAnim* p = new MainAnim;
    self->anim = p;
    (*(unsigned int*)&((W3dSpr*)self->anim)->resourceManager) = g_resourceManager;
    W3dSpr_AttachTexture(self->anim, AsCBunkerParticle(self)->bitmapId);
    char* arg = (char*)self + 8;
    self->reserved130 = kFloat_0_Particles;
    self->scale = kFloat_1_805DF43C;
    self->opacity = kFloat_0_69999999_ShotEffects;
    W3dSpr_SetPos(self->anim, arg);
    Sound_PlayByName(g_soundManager, ((CourseEffectStringPool*)courseEffectStrings)->bunkerSandSound,
                0, 0, 0, 0, kFloat_0_5_Particle, kFloat_200_shotParticles, kFloat_1_805DF43C);
    AsCBunkerParticle(self)->active = 1;
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset

struct ArrayOwner {
    char reserved00[0x14]; int bitmapId; unsigned char active; char pad19[3];
    int animatedSprite; void* anim; char pad24[0x10];
    void* entries; int count; float reserved12C; float value;
    char pad134[0xF0];
};

struct CFragmentArr {
    char data[0x18];
    // Invoke the no-op fragment-record constructor.
    CFragmentArr() { FragmentParticle_CtorNoInit(); }
    /* Allocate wrapper storage through the existing engine allocator. */
    static void* operator new[](size_t n) { return ARRAY_NEW(n); }
    /* Release wrapper storage through the matching engine allocator. */
    static void operator delete[](void* p) { __dla__FPv(p); }
};

// Initialize effect state and allocate its particle records.
void* CFragment_Ctor(void* self_) {
    char* self = (char*)self_;
    AsCFragment(self)->bitmapId = 0;
    AsCFragment(self)->active = 0;
    AsCFragment(self)->sprite = 0;
    AsCFragment(self)->unk_20 = 0;
    AsCFragment(self)->vertex0 = self + 0x44;
    AsCFragment(self)->vertex1 = self + 0x7C;
    AsCFragment(self)->vertex2 = self + 0xEC;
    AsCFragment(self)->vertex3 = self + 0xB4;
    AsCFragment(self)->vertexListTerminator = 0;
    *(void**)self = &__vt__9CFragment;
    AsCFragment(self)->fragments = 0;
    AsCFragment(self)->fragmentCount = 30;
    AsCFragment(self)->fragments = new CFragmentArr[30];
    return self;
}

struct RenderOwner {
    char pad00[0x18]; unsigned char active; char pad19[3]; void* old;
    void* anim; char pad24[0x100]; void* entries; int count;
    float reserved12C; float value; float alpha; char pad138[4]; float angle; char pad140[0x24];
};

void W3dAniSpr_Ctor(void*);
struct LargeAnim {
    char pad[0x10]; unsigned int resourceManager; char data[0x188];
    // Construct an animated billboard sprite in its allocation wrapper.
    LargeAnim() { W3dAniSpr_Ctor(this); }
};
void W3dAniSpr_Ctor(void*);
void W3dAniSpr_LoadTexture(void*, unsigned int, float, float);
void W3dAniSpr_SetSizeAndPivot(void*, float, float, float, float);

// Take the magnitude by testing the stored float sign bit.
static inline float absf(float x) {
    if (*(unsigned int*)&x & 0x80000000) return -x;
    return x;
}

struct Frag {
    char pad00[8];
    float pos[3];
    unsigned int bitmapId;
    unsigned char active;
    char pad19[3];
    AnimObject* old;
    char pad20[0x104];
    char* entries;
    int count;
    char pad12C[8];
    float alpha;
    float axis[3];
};

// Create the fragment sprite grid and launch fragments around the stored source axis.
// Build a launch basis around axis, seed local radial velocities, then rotate them to world.
// The final clamped speed value controls billboard size, not opacity.
// Required self owns count nonnegative 0x18-byte {world position,velocity} records;
// axis and pos are copied world-space vectors. Replaces/deletes the owned sprite,
// binds 32x32 texture frames, consumes three RNG samples per fragment and returns
// active at alpha 255. Allocation/texture load failures are unchecked.
// Launch basis is a 4x3 row-vector affine matrix: rows X/Y/Z then translation.
// If |axis.y-1|<1e-5, identity is retained (X/Z are NOT examined). Otherwise Y=axis,
// X=normalize(axis x worldUp), Z=normalize(X x axis), translation=0. The incoming
// axis is not normalized here. Normalize preserves exact zero (wvector_affine_matrix_math.cpp), so a
// parallel/downward/zero axis can produce zero X/Z rows; no fallback is invented.
// Local radial azimuth uses radians; local up speed interpolates 2..5, then all
// velocity is scaled by max(2,.1*ballSpeed) before the launch basis transform.
// Keep RNG order, Y-only identity test, matrix/temporary alignment, vector-copy
// calls and the final sprite-size clamp .2.. .5; none of these is a frame count.
void CFragment_LaunchFragments(Frag* self) {
    if (self->old) {
        if (self->old) {
            self->old->v0(1);
        }
        self->old = 0;
    }
    LargeAnim* anim = new LargeAnim;
    self->old = (AnimObject*)anim;
    float size = kFloat_32_SpriteGrid;
    anim->resourceManager = g_resourceManager;
    W3dAniSpr_LoadTexture(self->old, self->bitmapId, size, size);

    union {
        double align;
        struct { float azimuthMatrix[12], launchBasisX[3], launchBasisY[3], launchBasisZ[3], launchTranslation[3]; } v;
    } buffers;
    float axisCrossUp[3];
    float worldUp[3];
    float basisXCrossAxis[3];
    float rotatedRadialDirection[3];
    float scaledRadialDirection[3];
    float localForward[3];
    float worldVelocity[3];
    float speedScaledLocalVelocity[3];
    // Phase 1: build the launch frame at launchBasisX..launchTranslation; azimuthMatrix is separate azimuth scratch.
    Matrix_SetIdentityAffine(buffers.v.launchBasisX);
    float axisYMinusUp = self->axis[1] - s_fragmentUprightAxisY;
    int keepIdentityBasis = absf(axisYMinusUp) < kFloat_9_9999997eneg06_ShotEffects;
    if (!keepIdentityBasis) {
        __as__7WVectorFRC7WVector(buffers.v.launchBasisY, self->axis);
        worldUp[0] = kFloat_0_Particles;
        worldUp[1] = kFloat_1_805DF43C;
        worldUp[2] = kFloat_0_Particles;
        Vector_CrossProduct(axisCrossUp, buffers.v.launchBasisY, worldUp);
        __as__7WVectorFRC7WVector(buffers.v.launchBasisX, axisCrossUp);
        Normalize__7WVectorFv(buffers.v.launchBasisX);
        Vector_CrossProduct(basisXCrossAxis, buffers.v.launchBasisX, buffers.v.launchBasisY);
        __as__7WVectorFRC7WVector(buffers.v.launchBasisZ, basisXCrossAxis);
        Normalize__7WVectorFv(buffers.v.launchBasisZ);
        Vec3_Zero(buffers.v.launchTranslation);
    }
    float tenth, five, pi, two, one, zero, randomNormalization;
    int offset, i;
    i = 0;
    two = kFloat_2_0;
    offset = 0;
    // Phase 2: randomize azimuth/speed, apply a 2..5 upward component, then the launch basis.
    for (; i < self->count; ++i) {
        int randomSample = rand() % 1023;
        float random = (float)randomSample;
        randomNormalization = kFloat_0_0009765625_805DF488;
        zero = kFloat_0_Particles;
        one = kFloat_1_805DF43C;
        pi = kFloat_3_1415927_Particles;
        five = kFloat_5_ShotEffects;
        tenth = kFloat_0_1_Particle;
        float verticalBlend = random * randomNormalization;
        __as__7WVectorFRC7WVector(self->entries + offset, self->pos);
        localForward[0] = zero;
        localForward[1] = zero;
        localForward[2] = one;
        randomSample = rand() % 1023;
        Matrix_SetAxisRotation(buffers.v.azimuthMatrix, pi * (two * ((float)randomSample * randomNormalization)), 1);
        randomSample = rand() % 1023;
        __ml__FfRC7WVector(scaledRadialDirection, localForward, (float)randomSample * randomNormalization);
        Matrix_TransformPoint(rotatedRadialDirection, scaledRadialDirection, buffers.v.azimuthMatrix);
        __as__7WVectorFRC7WVector(self->entries + offset + 0xC, rotatedRadialDirection);
        *(float*)(self->entries + offset + 0x10) = two * (one - verticalBlend) + five * verticalBlend;
        float launchSpeedScale = tenth * Magnitude__7WVectorCFv(&((Ball*)GolfBall__Fv())->velocity);
        launchSpeedScale = (two < launchSpeedScale) ? launchSpeedScale : two;
        __ml__FfRC7WVector(speedScaledLocalVelocity, self->entries + offset + 0xC, launchSpeedScale);
        Matrix_TransformPoint(worldVelocity, speedScaledLocalVelocity, buffers.v.launchBasisX);
        __as__7WVectorFRC7WVector(self->entries + offset + 0xC, worldVelocity);
        offset += 0x18;
    }
    // Phase 3: activate at full alpha with speed-dependent sprite size/pivot.
    float full = kFloat_255_SpriteAlpha;
    self->active = 1;
    self->alpha = full;
    float speedScaledSize = kFloat_0_0099999998_ShotEffects * Magnitude__7WVectorCFv(&((Ball*)GolfBall__Fv())->velocity);
    float spriteSize = (speedScaledSize < kFloat_0_2_ShotEffects) ? kFloat_0_2_ShotEffects : ((speedScaledSize > kFloat_0_5_Particle) ? kFloat_0_5_Particle : speedScaledSize);
    float pivotOffset = kFloat_0_5_Particle * spriteSize;
    W3dAniSpr_SetSizeAndPivot(self->old, spriteSize, spriteSize, pivotOffset, pivotOffset);
}

// Draw each water droplet billboard with the effect's current opacity.
void CDroplet_DrawParticles(RenderOwner* self) {
    if (!self->active) return;
    int offset, color, i;
    i = 0;
    offset = 0;
    int alpha = (int)self->value;
    color = ((unsigned int)alpha << 24) | kWhiteRgb;
    (void)color;
    for (; i < AsCDroplet(self)->particleCount; ++i) {
        W3dSpr_SetPos(self->anim, (char*)self->entries + offset);
        W3dSpr_SetColor(self->anim, color);
        Particle_DrawRotatedBillboard(self->anim, g_view, kFloat_0_Particles, 0x20400000);
        offset += 0x1C;
    }
}

// Empty particle-record constructor or virtual hook; performs no work.
void FragmentParticle_CtorNoInit() {}
// Empty particle-record constructor or virtual hook; performs no work.
void DropletParticle_CtorNoInit() {}

struct DEntry {
    char d[0x1C];
    // Invoke the no-op droplet-record constructor.
    DEntry() { DropletParticle_CtorNoInit(); }
    /* Allocate wrapper storage through the existing engine allocator. */
    static void* operator new[](size_t n) { return ARRAY_NEW(n); }
    /* Release wrapper storage through the matching engine allocator. */
    static void operator delete[](void* p) { __dla__FPv(p); }
};

// Ripple-record aliases, exactly the same 0x20 bytes as effect_layout::Ripple:
// diameter, opacity, initialOpacity, fadeRate, expansionRate, active,
// startDelay and delayRemaining follow the shared ripple offsets.
// Units use the update delta clock; the droplet child keeps its separate schema.
struct FadeEntry {
    float diameter;
    float opacity;
    float initialOpacity;
    float fadeRate;
    float expansionRate;
    unsigned char active;
    char pad15[3];
    float startDelay;
    float delayRemaining;
};
// Shared PREFIX view, not one particle-record type. A parent CRipple owns
// table as 0x20-byte Ripple records and child as a CDroplet. The constructor
// below builds that DROPLET child, whose table is actually 0x1C-byte DEntry
// motion records; never index that child through FadeEntry* arithmetic.
struct FadeObject {
    void* vtable;
    int remainingRippleCount;
    char pad08[0x0C];
    int bitmapId;
    unsigned char active;
    char pad19[3];
    int animatedSprite;
    void* anim;
    char pad24[0x0C];
    void* vertex0;
    void* vertex1;
    void* vertex2;
    void* vertex3;
    int vertexListTerminator;
    char pad44[0xE0];
    FadeEntry* table;
    int count;
    void* child;
    char pad130[4];
    // Construct the owned water-droplet child with 100 no-init motion records and
    // a waterDropTexture sprite. Uses the shared effect prefix, CDroplet vtable,
    // 32-bit float color/geometry defaults and the normal new[] cookie lifetime.
    FadeObject() {
        bitmapId = 0; active = 0; animatedSprite = 0; anim = 0;
        vertex0 = ((Base*)this)->vertices[0];
        vertex1 = ((Base*)this)->vertices[1];
        vertex2 = ((Base*)this)->vertices[3];
        vertex3 = ((Base*)this)->vertices[2];
        vertexListTerminator = 0;
        vtable = __vt__8CDroplet;
        count = kDropletChildParticleCount;
        table = (FadeEntry*)new DEntry[count];
        anim = W3dSpr_Create((void*)g_resourceManager, ((CourseEffectStringPool*)courseEffectStrings)->waterDropTexture, 0);
        float f = kFloat_0_5_Particle * kFloat_0_2_ShotEffects;
        W3dSpr_SetRect(anim, kFloat_0_2_ShotEffects, kFloat_0_2_ShotEffects, f, f);
    }
};

// Initialize three delayed expanding ripple records and an owned water-droplet child.
// Fresh self returns self; allocates 0xA0 bytes (five 0x20-byte record slots) but
// initializes/uses only THREE. Expansion rates=15/17/19 diameter units per delta;
// initial delays=.01/.2/.5 delta units; initial opacity/fade rates=255/255/128.
// Begin resets delayRemaining from startDelay; constructor leaves it zero until
// then. Parent table has a plain allocation; child DEntry[] has its own cookie.
// CRipple_dtor releases both using their distinct paths. Preserve this distinction.
FadeObject* CRipple_Ctor(FadeObject* self) {
    float thirdInitialOpacityAndFadeRate, thirdStartDelay, thirdExpansionRate, secondStartDelay, secondExpansionRate, fullInitialOpacityAndFadeRate, firstStartDelay, firstExpansionRate;
    self->bitmapId = 0; self->active = 0; self->animatedSprite = 0; self->anim = 0;
    self->vertex0 = ((Base*)self)->vertices[0];
    self->vertex1 = ((Base*)self)->vertices[1];
    self->vertex2 = ((Base*)self)->vertices[3];
    self->vertex3 = ((Base*)self)->vertices[2];
    self->vertexListTerminator = 0;
    self->vtable = __vt__7CRipple;
    self->table = 0;
    self->count = 0;
    void* mem = __nwa__FUl(kRippleStorageBytes);
    firstExpansionRate = kFloat_15_ShotEffects;
    ((FadeEntry*)(self->table = (FadeEntry*)mem))[self->count].expansionRate = firstExpansionRate;
    firstStartDelay = kFloat_0_0099999998_ShotEffects;
    fullInitialOpacityAndFadeRate = kFloat_255_SpriteAlpha;
    secondExpansionRate = kFloat_17_ShotEffects; secondStartDelay = kFloat_0_2_ShotEffects;
    self->table[self->count].startDelay = firstStartDelay;
    thirdExpansionRate = kFloat_19_ShotEffects; thirdStartDelay = kFloat_0_5_Particle; thirdInitialOpacityAndFadeRate = kFloat_128_ShotEffects;
    self->table[self->count].initialOpacity = fullInitialOpacityAndFadeRate;
    self->table[self->count].fadeRate = fullInitialOpacityAndFadeRate;
    int c = self->count + 1;
    self->count = c;
    self->table[c].expansionRate = secondExpansionRate;
    self->table[self->count].startDelay = secondStartDelay;
    self->table[self->count].initialOpacity = fullInitialOpacityAndFadeRate;
    self->table[self->count].fadeRate = fullInitialOpacityAndFadeRate;
    c = self->count + 1;
    self->count = c;
    self->table[c].expansionRate = thirdExpansionRate;
    self->table[self->count].startDelay = thirdStartDelay;
    self->table[self->count].initialOpacity = thirdInitialOpacityAndFadeRate;
    self->table[self->count].fadeRate = thirdInitialOpacityAndFadeRate;
    self->count = self->count + 1;
    FadeObject* child = new FadeObject();
    self->child = child;
    int i = 0; int offset = 0;
    for (; i < self->count; ++i) {
        ((Ripple*)((char*)self->table + offset))->active = 0;
        ((Ripple*)((char*)self->table + offset))->diameter = kFloat_0_Particles;
        ((Ripple*)((char*)self->table + offset))->opacity = ((Ripple*)((char*)self->table + offset))->initialOpacity;
        ((Ripple*)((char*)self->table + offset))->delayRemaining = kFloat_0_Particles;
        offset = offset + 0x20;
    }
    self->remainingRippleCount = self->count;
    return self;
}

struct DropletResetView {
    char reserved00[0x18]; unsigned char active; char pad19[0x10B];
    void* entries; int count; float value; float alpha;
};
#pragma opt_lifetimes off
// Launch water droplets from the effect position with randomized radial velocity.
void CDroplet_InitRandomVelocities(DropletResetView* self) {
    float unit;
    double magic;
    float one, zero, ten, forty, six, pi, two;
    unsigned int offset;
    int i;
    zero = kFloat_0_Particles;
    one = kFloat_1_805DF43C;
    magic = kDouble_SignedIntConversionBias_Particles;
    unit = kFloat_0_0009765625_805DF488;
    i = 0;
    two = kFloat_2_0;
    offset = 0;
    pi = kFloat_3_1415927_Particles;
    six = kFloat_6_ShotEffects;
    forty = kFloat_40_ShotEffects;
    ten = kFloat_10_CourseEffects;
    for (; self->count > i; ++i) {
        float v[12];
        float combined[4];
        float p[3];
        float a[3];
        void* randObject = (void*)GolfBall__Fv();
        void* savedEntries = self->entries;
        __as__7WVectorFRC7WVector((char*)savedEntries + offset, randObject);
        a[0] = zero; a[1] = zero; a[2] = one;
        int random = rand();
        int rem = random % 0x3FF;
        float converted1 = (float)rem;
        float angleUnit = converted1 * unit;
        Matrix_SetAxisRotation(v, pi * (two * angleUnit), 1);
        random = rand();
        rem = random % 0x3FF;
        float converted2 = (float)rem;
        float scaled = six * converted2;
        float scale = scaled * unit;
        __ml__FfRC7WVector(p, a, scale);
        Matrix_TransformPoint(combined, p, v);
        __as__7WVectorFRC7WVector(&MotionAt(self->entries, offset)->velocity, combined);
        random = rand();
        int savedRandom = random;
        rem = savedRandom % 0x3FF;
        void* savedEntries2 = self->entries;
        float converted3 = (float)rem;
        float t = converted3 * unit;
        MotionAt(savedEntries2, offset)->velocity.y =
            ten * (one - t) + forty * t;
        offset += 0x1C;
    }
    self->alpha = kFloat_255_SpriteAlpha;
    self->value = kFloat_1_5_ShotEffects;
    self->active = 1;
}
#pragma opt_lifetimes reset

struct BunkerUpdateView {
    char reserved00[0x18];
    unsigned char active;
    char reserved19[0x10F];
    float value;
    char reserved12C[8];
    float duration;
};

// Count down the sand-cloud lifetime and deactivate the effect on expiry.
void CBunkerParticle_DecrementLifeFraction(BunkerUpdateView* self, float delta) {
    if (0 == self->active) {
        return;
    }
    float savedDuration2 = self->duration;
    register float duration = savedDuration2;
    register float step = delta;
    step = step / duration;
    register float value = self->value;

    value = value - step;
    self->value = value;
    if ((value < kFloat_0_Particles)) {
        self->active = 0;
    }
}

struct DropletUpdateView {
    char reserved00[0x18]; unsigned char active; char pad19[0x10B];
    void* entries; int count; float rate; float value;
};
#pragma opt_lifetimes off
// Integrate droplet positions under gravity and fade the whole droplet effect.
void CDroplet_UpdateAndFade(DropletUpdateView* self, float delta) {
    if (self->active) {
        register float value;
        register float product;
        register float rate;
        rate = self->rate;
        product = kFloat_255_SpriteAlpha * delta;
        value = self->value;
        product = product / rate;
        value = value - product;
        self->value = value;
        if (value < kFloat_0_Particles) {
            self->active = 0;
        } else if (rate < kFloat_0_Particles) {
            self->active = 0;
        } else {
            float speed = kFloat_34_295296_ShotEffects;
            unsigned int offset;
            int i = 0;
            offset = 0;
            for (; i < self->count; ++i) {
                void* savedEntries3 = self->entries;
                float savedSpeed = speed;
                void* savedEntries4 = self->entries;
                MotionAt(savedEntries3, offset)->velocity.y = MotionAt(savedEntries4, offset)->velocity.y - savedSpeed * delta;
                unsigned char temp[0x10];
                void* savedEntries5 = self->entries;
                __ml__FRC7WVectorf(temp, &MotionAt(savedEntries5, offset)->velocity, delta);
                *(WVector*)((char*)self->entries + offset) += *(WVector*)temp;
                offset += 0x1C;
            }
        }
    }
}
#pragma opt_lifetimes reset

// Release owned particles, sprites, and buffers; optionally delete the effect object.
void* CFragment_dtor(void* object, int shouldDelete) {
    if (object) {
        void** fields = (void**)object;
        void* entries = AsCFragment(object)->fragments;
        fields[0] = &__vt__9CFragment;
        if (entries) {
            if (entries) {
                FreeArrayBlock(entries);
            }
            AsCFragment(object)->fragments = 0;
        }
        void* anim = fields[7];
        if (anim) {
            if (anim) {
                ((AnimObject*)anim)->v0(1);
            }
            fields[7] = 0;
        }
    if (object) {
        void** fields = (void**)object;
        fields[0] = &s_particleVtable;
        void* anim = fields[8];
        if (anim) {
            if (anim) {
                ((AnimObject*)anim)->v0(1);
            }
            fields[8] = 0;
        }
        anim = fields[7];
        if (anim) {
            if (anim) {
                ((AnimObject*)anim)->v0(1);
            }
            fields[7] = 0;
        }
    }
    if (shouldDelete > 0) __dl__FPv(object);
    }
    return object;
}

struct BunkerShotUpdateView {
    char reserved00[0x18]; unsigned char active; char pad19[0x10B];
    void* first; void* second; int firstCount; int liveCount;
    float rate; float value;
};
// Integrate sand sprites and streaks, apply gravity, and fade the effect.
void CBunkerShotParticle_UpdateParticles(BunkerShotUpdateView* self, float delta) {
    if (!self->active) return;
    register float rate = self->rate;
    register float step = delta;
    register float value = self->value;
    step = step / rate;
    value = value - step;
    if (value < kFloat_0_Particles) {
        self->value = value;
        self->active = 0;
        return;
    }
    self->value = value;
    int i = 0;
    int offset;
    float speed = kFloat_120_03354_ShotEffects;
    offset = 0;
    for (; i < self->firstCount; ++i) {
        char* entry = (char*)self->first + offset;

        *(float*)(entry + 0x10) = *(float*)(entry + 0x10) - speed * delta;
        WVector temp = *(WVector*)&MotionAt(self->first, offset)->velocity * delta;
        *(WVector*)entry += temp;
        offset += 0x18;
    }
    float zero;
    float drop;
    float decay;
    decay = kFloat_0_99000001_ShotEffects;
    drop = kFloat_102_88589_ShotEffects;
    zero = kFloat_0_Particles;
    offset = 0;
    i = 0;
    for (; i < self->liveCount; ++i) {
        char* entry = (char*)self->second + offset;
        if (*(unsigned char*)entry) {

            *(float*)(entry + 0x10) = *(float*)(entry + 0x10) * decay;
            *(float*)(entry + 0x14) = *(float*)(entry + 0x14) - drop * delta;
            *(float*)(entry + 0x18) = *(float*)(entry + 0x18) * decay;
            WVector temp = *(WVector*)(entry + 0x10) * delta;
            *(WVector*)(entry + 4) += temp;
            float t = *(float*)(entry + 0x20) - delta;
            *(float*)(entry + 0x20) = t;
            if (t < zero) {
                *(unsigned char*)entry = 0;
                --self->liveCount;
            }
        }
        offset += 0x28;
    }
}

struct BunkerShotDrawView {
    char reserved00[0x18]; unsigned char active; char pad19[7]; void* anim;
    char pad24[0x100]; void* entries; unsigned char* streakParticles; int tableCount; int liveCount;
    char pad134[4]; float fadeRemaining; unsigned int color; float scale; void* drawBuffer; void* indexBuffer;
};
extern float kFloat_40_ShotEffects; // 40: sand billboard size / droplet upward-speed bound
extern const float kFloat_neg1_ShotEffects; // -1: reverse-vector scale / vertical-velocity threshold
extern const float kFloat_0_1_Particle;
extern const float kFloat_0_94999999_ShotEffects; // 0.95: sand-streak length decay per draw
extern unsigned int g_view;
struct BunkerRenderState { int words[10]; };
extern const BunkerRenderState s_bunkerStreakRenderStates;
extern const char kIdentityMatrix4x4[0x30];
void Resource_GetOffsetPtr(void*, int, int, void*, int);
void GraphicsBuffer_Flush(void*);
void Vec3_Scale(void*, const void*, float);
void Vec3_Normalize(void*, const void*);
void Vec3_Add(void*, const void*, const void*);
void Vector_CopyComponents(void*, const void*);
void Engine_SetRenderStateList(void*, int);
void* Camera_GetRenderState(unsigned int);
void SetTransform(int, const void*);
void Engine_SetType7Resource(int);
void Engine_SetFlexibleVertexFormat(unsigned int);
void Engine_SetVertexStream(int, void*, int, int);
void Engine_SetIndexBuffer(void*, int);
void Engine_SetTextureStageResource(int, int);
void Engine_DrawIndexedPrimitive(int, int, int, int, int, int);

// Draw the sand sprites and rebuild/draw live streak geometry.
// Sprite size grows as fadeRemaining falls; streak length shrinks near each particle's expiry.
// Draw also damps self->scale once per call; invoking it changes later streak geometry.
void CBunkerShotParticle_Draw(BunkerShotDrawView* self) {
    // Three cursor words are reused across phases to retain the original register lifetimes:
    // r27 replacement: sprite offset -> line-end offset; r28: sprite index -> streak offset;
    // r29: sprite position address -> line-start offset. All offsets below are bytes.
    int drawnLineCount;
    int spriteOffsetOrLineEndOffset;
    int spriteIndexOrStreakByteOffset;
    int spriteAddressOrLineStartOffset;
    int streakIndex;
    effect_layout::BunkerStreak* entry;
    float alphaScale;
    float pivotFraction;
    float spriteGrowthScale;
    float fullFade;
    void* buffer;
    float lineEndPosition[3];
    float fadeValue;
    int unusedEntryAddress;
    int packedColor;
    int alphaByte;
    float slowdownThreshold;
    float remainingLife;
    BunkerRenderState state;
    void* device;
    if (self->active == 0) return;
    fullFade = kFloat_1_805DF43C;
    spriteIndexOrStreakByteOffset = 0;
    spriteGrowthScale = kFloat_40_ShotEffects;
    spriteOffsetOrLineEndOffset = 0;
    pivotFraction = kFloat_0_5_Particle;
    alphaScale = kFloat_255_SpriteAlpha;
    // Phase 1: draw expanding sand sprites in world space.
    for (; spriteIndexOrStreakByteOffset < self->tableCount; ) {
        fadeValue = self->fadeRemaining;
        unusedEntryAddress = (int)self->entries;
        fadeValue = fullFade - fadeValue;
        spriteAddressOrLineStartOffset = (int)self->entries + spriteOffsetOrLineEndOffset;
        W3dSpr_SetRect(self->anim, spriteGrowthScale * fadeValue, spriteGrowthScale * fadeValue, pivotFraction * (spriteGrowthScale * fadeValue), kFloat_0_Particles);
        fadeValue = self->fadeRemaining;
        packedColor = self->color;
        fadeValue = alphaScale * fadeValue;
        alphaByte = (int)fadeValue;
        packedColor = packedColor | (alphaByte << 24);
        W3dSpr_SetColor(self->anim, (unsigned int)packedColor);
        W3dSpr_SetPos(self->anim, (void*)spriteAddressOrLineStartOffset);
        Billboard_DrawOrientedQuad(self->anim, g_view, 0x20400000, 1);
        spriteOffsetOrLineEndOffset += 0x18;
        spriteIndexOrStreakByteOffset += 1;
    }
    // Phase 2: compact active streaks into two world-space vertices per line.
    drawnLineCount = 0;
    Resource_GetOffsetPtr(self->drawBuffer, 0, 0, &buffer, 0);
    streakIndex = 0;
    spriteIndexOrStreakByteOffset = 0;
    spriteAddressOrLineStartOffset = 0;
    spriteOffsetOrLineEndOffset = 0x10;
    for (; streakIndex < self->liveCount; spriteIndexOrStreakByteOffset += 0x28, ++streakIndex) {
        entry = (effect_layout::BunkerStreak*)((char*)self->streakParticles + spriteIndexOrStreakByteOffset);
        if (entry->active) {
            Vec3_Scale(lineEndPosition, &entry->velocity, kFloat_neg1_ShotEffects);
            Vec3_Normalize(lineEndPosition, lineEndPosition);
            Vec3_Scale(lineEndPosition, lineEndPosition, self->scale);
            remainingLife = entry->remainingLife;
            slowdownThreshold = entry->slowdownThreshold;
            if (remainingLife < slowdownThreshold) Vec3_Scale(lineEndPosition, lineEndPosition, remainingLife / slowdownThreshold);
            Vec3_Add(lineEndPosition, lineEndPosition, &entry->position);
            Vector_CopyComponents((char*)buffer + spriteAddressOrLineStartOffset, lineEndPosition);
            Vector_CopyComponents((char*)buffer + spriteOffsetOrLineEndOffset, &((effect_layout::BunkerStreak*)((char*)self->streakParticles + spriteIndexOrStreakByteOffset))->position);
            spriteAddressOrLineStartOffset += 0x20;
            spriteOffsetOrLineEndOffset += 0x20;
            drawnLineCount = drawnLineCount + 1;
        }
    }
    GraphicsBuffer_Flush(self->drawBuffer);
    if (self->scale > kFloat_0_1_Particle) self->scale *= kFloat_0_94999999_ShotEffects;
    // Phase 3: bind camera matrices/identity model lineEndPosition, then draw only compacted lines.
    state = s_bunkerStreakRenderStates;
    Engine_SetRenderStateList(&state, 5);
    device = Camera_GetRenderState(g_view);
    SetTransform(1, ((CameraMatrices*)device)->transform1);
    SetTransform(0, ((CameraMatrices*)device)->transform0);
    Engine_SetType7Resource(0);
    SetTransform(2, kIdentityMatrix4x4);
    SetTransform(0xB, kIdentityMatrix4x4);
    Engine_SetFlexibleVertexFormat(kPositionColorVertexFormat);
    Engine_SetVertexStream(0, self->drawBuffer, 0, 0x10);
    Engine_SetIndexBuffer(self->indexBuffer, 0);
    Engine_SetTextureStageResource(0, 0);
    Engine_DrawIndexedPrimitive(0xA8, 0, 0, 2 * drawnLineCount, 0, drawnLineCount);
}

} // extern "C"

} // namespace ns2

// ---- merged from course_ball_shot_effects.cpp ----
namespace ns3 {
// Translation unit 0x801B0A54-0x801B215C (13 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct VObj {
    virtual ~VObj();
};

struct VCall5 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void f();
};

extern "C" {

extern void Engine_SetRenderStateList(void*, int);
extern void Engine_SetTextureStageStateList(int, void*, int);
extern void Engine_SetTextureStageParameterList(int, void*, int);
extern void Engine_SetType7Resource(int);
extern void Engine_SetFlexibleVertexFormat(int);
extern void Engine_SetTextureStageResource(int, int);
extern void Engine_SetTexturePalette(int, int);
extern void DrawPrimitiveUP(int, int, void*, int);
extern void SetTransform(int, void*);
extern void* Camera_GetRenderState(void*);
extern const int s_shotAndRippleRenderStateLists[];
extern const char kIdentityMatrix4x4[];
extern const float kFloat_0_5_Particle;
extern const double kDouble_SignedIntConversionBias_Particles;

extern void __as__7WVectorFRC7WVector(void*, void*);
void CoinModelEntry_Init(int*);
void CoinSparkParticle_CtorNoInit();
extern void* __nwa__FUl(unsigned int);
extern void* __construct_new_array(void*, void*, int, int, int);
extern void W3dAniSpr_SetColor(void*, unsigned int);
extern void W3dAniSpr_SetPosition(void*, void*);
extern void W3dAniSpr_DrawFrame(void*, int, float, unsigned int, int);
extern void __dla__FPv(void*);
extern void __dl__FPv(void*);
extern void CParticleBase_dtor(void*, int);
extern char __vt__5CCoin[];
extern char __vt__7CRipple[];
extern char s_particleVtable[];
extern int g_view;
extern const float kFloat_0_Particles;
extern float kFloat_0_15000001_ShotEffects; // 0.15: coin spark pivot / shadow size
extern signed char s_renderPacketInitialized;
extern int g_shadowManager;
extern char s_renderPacket[];
extern char s_renderPacketDestructorRegistration[];
extern char courseEffectStrings[];
extern void RenderPacket_Init(void*);
extern void RenderPacket_Destroy();
extern void __register_global_object(void*, void*, void*);
extern void* CShadowManager_GetOrCreateEntry(int, char*, float, int, char*, void*);
extern void GfxObjList_RemoveGroupByName(int, char*);
extern void TextureItemList_ClearAndRelease(void*);
extern void* CPetBody_GetPuppet(void*);
extern void TextureItemQueue_PushFront(void*, void*);

#pragma use_lmw_stmw on
// Initialize owned coin-model and spark arrays around a copied world-space origin.
// count must be nonnegative and fit count*0x18+0x10 in the target allocation word;
// arg2 borrows three float words. Returns fresh self, inactive; counts/arrays are
// installed, but model slots and 50 spark records still need the reset hook.
// Coin and spark arrays have distinct 0x18/0x28 strides plus runtime cookies.
// Allocation failure is not rolled back here; exception path destroys the base.
// CCoin_dtor releases models, sprites, retained bitmap and both array allocations.
void* CCoin_Ctor(char* self, int count, void* arg2) {
    AsCCoin(self)->bitmapId = 0;
    (*(char*)&AsCCoin(self)->enabled) = 0;
    AsCCoin(self)->sprite = 0;
    AsCCoin(self)->sparkSprite = 0;
    AsCCoin(self)->vertex0 = self + 0x44;
    AsCCoin(self)->vertex1 = self + 0x7C;
    AsCCoin(self)->vertex2 = self + 0xEC;
    AsCCoin(self)->vertex3 = self + 0xB4;
    AsCCoin(self)->vertexListTerminator = 0;
    *(void**)self = __vt__5CCoin;
    *(WVector*)(self + 0x134) = *(WVector*)arg2;
    AsCCoin(self)->coinCount = count;
    try {
    void* a = __nwa__FUl(count * 0x18 + 0x10);
    int flag = 0;
    if (a) { a = __construct_new_array(a, (void*)CoinModelEntry_Init, 0, 0x18, count); flag = 1; }
    AsCCoin(self)->coinEntries = a;
    AsCCoin(self)->spriteCount = 50;
    a = __nwa__FUl(50 * 0x28 + 0x10);
    flag = 0;
    if (a) { a = __construct_new_array(a, (void*)CoinSparkParticle_CtorNoInit, 0, 0x28, 50); flag = 1; }
    AsCCoin(self)->spriteEntries = a;
    (void)flag;
    } catch (...) {
        CParticleBase_dtor(self, -1);
        throw;
    }
    return self;
}
#pragma use_lmw_stmw reset

struct CoinObj_ { 
virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19(); virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24(); virtual void v25(); virtual void v26(int, int);
};
// Draw enabled coin models and their live spark sprites.
void CCoin_DrawCoinsAndSprites(unsigned char* self) {
    if (AsCCoin(self)->enabled) { int offset;
        int i;
        i = 0;
        offset = 0;
        while (i < AsCCoin(self)->coinCount) {
            int savedOffset3 = offset;
            void* obj = *(void**)((*(char**)&AsCCoin(self)->coinEntries) + savedOffset3);
            ((CoinObj_*)obj)->v26(1, 1);
            offset += 0x18; i = 1 + i;
        }
        int spriteIndex = 0;
        int spriteOffset = 0;
        while (spriteIndex < AsCCoin(self)->spriteCount) {
            int savedOffset4 = spriteOffset;
            CoinSprite* item = (CoinSprite*)((*(char**)&AsCCoin(self)->spriteEntries) + savedOffset4);
            W3dAniSpr_SetColor(AsCCoin(self)->sprite, ((unsigned int)((int)item->opacity) << 24) | kWhiteRgb);
            Vec3f* savedItem = &item->position;
            W3dAniSpr_SetPosition(AsCCoin(self)->sprite, savedItem);
            W3dAniSpr_DrawFrame(AsCCoin(self)->sprite, g_view, kFloat_0_Particles, 0x20C00000, 5);
            spriteOffset += 0x28; spriteIndex++;
        }
    }
}

struct RippleOwnedObject { 
virtual void destroy(int);
};
// Release owned particles, sprites, and buffers; optionally delete the effect object.
void* CRipple_dtor(char* self, int deleting) {
    if (self) {
        *(void**)self = __vt__7CRipple;
        if (AsCRipple(self)->ripples) {
            __dla__FPv(AsCRipple(self)->ripples); AsCRipple(self)->ripples = 0;
        }
        void* o = AsCRipple(self)->dropletEffect;
        if (o) {
            if (o) { ((RippleOwnedObject*)o)->destroy(1); }
            AsCRipple(self)->dropletEffect = 0;
        }
        if (self) {
        *(void**)self = s_particleVtable;
        o = AsCRipple(self)->ownedObject2;
        if (o) {
            if (o) { ((RippleOwnedObject*)o)->destroy(1); }
            AsCRipple(self)->ownedObject2 = 0;
        }
        o = AsCRipple(self)->ownedObject1;
        if (o) {
            if (o) { ((RippleOwnedObject*)o)->destroy(1); }
            AsCRipple(self)->ownedObject1 = 0;
        }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Empty particle-record constructor or virtual hook; performs no work.
void CoinSparkParticle_CtorNoInit() {}

// Set coin-effect visibility and configure or remove each coin model's shadow.
void CCoin_SetEnabled(char* self, unsigned char enabled) {
    (*(char*)&AsCCoin(self)->enabled) = enabled;
    if (enabled) {
        // Lazy singleton: construct the global render packet on first use (Sing_805DC7B0 slot).
        if (!((int&)Sing_805DC7B0())) {
            if (!s_renderPacketInitialized) {
                RenderPacket_Init(s_renderPacket);
                __register_global_object(s_renderPacket, (void*)RenderPacket_Destroy, s_renderPacketDestructorRegistration);
                s_renderPacketInitialized = 1;
            }
            ((int&)Sing_805DC7B0()) = (int)s_renderPacket;
        }
        int offset;
        void* layout;
        int i;
        void* tmp = CShadowManager_GetOrCreateEntry(g_shadowManager,((CourseEffectStringPool*)courseEffectStrings)->coinShadowGroup, kFloat_0_15000001_ShotEffects,0x110, ((CourseEffectStringPool*)courseEffectStrings)->sunBone, &((CoinShadowOrigin*)((int&)Sing_805DC7B0()))->position);
        layout = tmp;
        if (AsCCoin(self)->coinCount) {
            TextureItemList_ClearAndRelease(layout);
            i = 0;
            offset = 0;
            while (i < AsCCoin(self)->coinCount) {
                void* item = *(void**)((*(char**)&AsCCoin(self)->coinEntries) + offset);
                void* converted = CPetBody_GetPuppet(item);
                TextureItemQueue_PushFront(layout, converted);
                offset += 0x18;
                ++i;
            }
        }
    } else {
        GfxObjList_RemoveGroupByName(g_shadowManager,((CourseEffectStringPool*)courseEffectStrings)->coinShadowGroup);
    }
}

#pragma use_lmw_stmw on
// Release owned particles, sprites, and buffers; optionally delete the effect object.
void* CCoin_dtor(char* self, int deleting) {
    if (self) {
        *(void**)self = __vt__5CCoin;
        int offset;
        int i;
        i = 0;
        offset = 0;
        while (i < AsCCoin(self)->coinCount) {
            void* item = *(void**)((*(char**)&AsCCoin(self)->coinEntries) + offset);
            if (item) {
                delete (VObj*)item;
                *(void**)((*(char**)&AsCCoin(self)->coinEntries) + offset) = 0;
            }
            offset += 0x18; ++i;
        }
        void* ptr = AsCCoin(self)->coinEntries;
        if (ptr) {
            if (ptr) FreeArrayBlock(ptr);
            AsCCoin(self)->coinEntries = 0;
        }
        ptr = AsCCoin(self)->spriteEntries;
        if (ptr) {
            if (ptr) FreeArrayBlock(ptr);
            AsCCoin(self)->spriteEntries = 0;
        }
        void* object = AsCCoin(self)->sprite;
        if (object) {
            delete (VObj*)object;
            AsCCoin(self)->sprite = 0;
        }
        AsCCoin(self)->coinCount = 0;
        AsCCoin(self)->spriteCount = 0;
        if (self) {
            void* object20 = AsCCoin(self)->sparkSprite;
            *(void**)self = s_particleVtable;
            if (object20) {
                delete (VObj*)object20;
                AsCCoin(self)->sparkSprite = 0;
            }
            object = AsCCoin(self)->sprite;
            if (object) {
                delete (VObj*)object;
                AsCCoin(self)->sprite = 0;
            }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Initialize one stored coin-model pointer to null.
void CoinModelEntry_Init(int* value) {
    *value = 0;
}

struct Vtx {
    float x, y, z;
    unsigned char r, g, b, a;
    float u, v;
};
struct Blk24 { unsigned int w[24]; };
struct Blk12 { unsigned int w[12]; };
struct Blk10 { unsigned int w[10]; };

// Draw active expanding ripple quads, then draw the child water-droplet effect.
void CRipple_Draw(char* self) {
    char* base = (char*)s_shotAndRippleRenderStateLists;
    if (!AsCRipple(self)->active) return;
    Blk24 temp = *(Blk24*)(((char*)(base + 0x108)));
    Blk12 data = *(Blk12*)(((char*)(base + 0x168)));
    Blk10 data2 = *(Blk10*)(((char*)(base + 0x198)));
    Engine_SetRenderStateList(&temp, 12);
    Engine_SetTextureStageStateList(0, &data, 6);
    Engine_SetTextureStageParameterList(0, &data2, 5);
    void* device = ((WView*)g_view)->renderTarget;
    if (device) {
        ((void (*)(void*, int))(*(void***)device)[25])(device, 0x20200000);
        ((void (*)(void*, int))(*(void***)device)[26])(device, 0x00800000);
    }
    void* global = (void*)g_view;
    void* camera = Camera_GetRenderState(global);
    SetTransform(1, ((CameraMatrices*)camera)->transform1);
    SetTransform(0, ((CameraMatrices*)camera)->transform0);
    Engine_SetType7Resource(0);
    SetTransform(2, (char*)kIdentityMatrix4x4);
    SetTransform(11, (char*)kIdentityMatrix4x4);
    Engine_SetFlexibleVertexFormat(kPositionColorUvVertexFormat);
    void* dev = ((WView*)g_view)->renderTarget;
    int arg = ((int (*)(void*, int))(*(void***)dev)[28])(dev, AsCRipple(self)->textureId);
    Engine_SetTextureStageResource(0, arg);
    dev = ((WView*)g_view)->renderTarget;
    arg = ((int (*)(void*, int))(*(void***)dev)[29])(dev, AsCRipple(self)->textureId);
    Engine_SetTexturePalette(0, arg);

    float half = kFloat_0_5_Particle;
    Vtx verts[4];
    for (int j = 0; j < 4; ++j) {
        verts[j].u = (float)(j & 1);
        verts[j].v = (float)(j >> 1);
        verts[j].r = verts[j].g = verts[j].b = kOpaqueAlpha;
    }
    int offset = 0;
    int index = 0;
    while (index < AsCRipple(self)->rippleCount) {
        if (RippleAt(AsCRipple(self)->ripples, offset)->active) {
            Vtx* p = verts;
            for (int k = 0; k < 4; ++k) {
                p->x = (half - (float)(k & 1)) * RippleAt(AsCRipple(self)->ripples, offset)->diameter + AsCRipple(self)->positionX;
                p->y = AsCRipple(self)->positionY;
                p->z = (half - (float)(k >> 1)) * RippleAt(AsCRipple(self)->ripples, offset)->diameter + AsCRipple(self)->positionZ;
                p->a = (unsigned char)RippleAt(AsCRipple(self)->ripples, offset)->opacity;
                ++p;
            }
            DrawPrimitiveUP(0x98, 2, verts, 0x18);
        }
        offset += 0x20;
        ++index;
    }
    void* obj = AsCRipple(self)->dropletEffect;
    ((void (*)(void*))(*(void***)obj)[4])(obj);
}

extern void __as__7WVectorFRC7WVector(void*, void*);
extern const float kFloat_0_Particles;
extern const float kFloat_1_805DF43C;
extern const float kFloat_3_ShotEffects; // 3: velocity jitter bias / coin gravity
extern const float kFloat_32_SpriteGrid;
extern const float kFloat_0_3_805DF4A4;
extern float kFloat_0_15000001_ShotEffects; // 0.15: coin spark pivot / shadow size
extern int g_resourceManager;

// Reset ripple delays, opacity, and live count, then restart the child droplet effect.
void CRipple_Begin(char* self) {
    int i = 0;
    int offset = 0;
    while (i < AsCRipple(self)->rippleCount) {
        char* item = (*(char**)&AsCRipple(self)->ripples) + offset;
        ((Ripple*)item)->active = 0;
        RippleAt(AsCRipple(self)->ripples, offset)->diameter = kFloat_0_Particles;
        item = (*(char**)&AsCRipple(self)->ripples) + offset;
        ((Ripple*)item)->opacity = ((Ripple*)item)->initialOpacity;
        item = (*(char**)&AsCRipple(self)->ripples) + offset;
        ((Ripple*)item)->delayRemaining = kFloat_0_Particles;
        ++i;
        offset += 0x20;
    }
    AsCRipple(self)->remainingRippleCount = AsCRipple(self)->rippleCount;
    for (i = 0; i < AsCRipple(self)->rippleCount; ++i) {
        ((Ripple*)AsCRipple(self)->ripples)[i].delayRemaining =
            ((Ripple*)AsCRipple(self)->ripples)[i].startDelay;
    }
    (*(char*)&AsCRipple(self)->active) = 1;
    char* object = (*(char**)&AsCRipple(self)->dropletEffect);
    *(WVector*)((char*)object + 8) = (*(WVector*)&((CRipple*)self)->positionX);
    object = (*(char**)&AsCRipple(self)->dropletEffect);
    ((VCall5*)object)->f();
}

#pragma use_lmw_stmw on
#pragma opt_lifetimes off
// Start delayed ripples, expand and fade them, then update the child droplet effect.
void CRipple_Update(volatile char* self, float delta) {
    if (AsCRipple(self)->active == 0)
        return;
    float zero = kFloat_0_Particles;
    int offset = 0;
    int i = 0;
    while (i < AsCRipple(self)->rippleCount) {
        float t = RippleAt(AsCRipple(self)->ripples, offset)->delayRemaining;
        if (t > zero) {
            t -= delta;
            RippleAt(AsCRipple(self)->ripples, offset)->delayRemaining = t;
            float t2 = RippleAt(AsCRipple(self)->ripples, offset)->delayRemaining;
            if (t2 < zero)
                RippleAt(AsCRipple(self)->ripples, offset)->active = 1;
        }
        if (RippleAt(AsCRipple(self)->ripples, offset)->active) {
            RippleAt(AsCRipple(self)->ripples, offset)->diameter += delta * RippleAt(AsCRipple(self)->ripples, offset)->expansionRate;
            RippleAt(AsCRipple(self)->ripples, offset)->opacity -= delta * RippleAt(AsCRipple(self)->ripples, offset)->fadeRate;
            if (RippleAt(AsCRipple(self)->ripples, offset)->opacity < zero) {
                RippleAt(AsCRipple(self)->ripples, offset)->active = 0;
                int n = --AsCRipple(self)->remainingRippleCount;
                if (!n)
                    self[0x18] = 0;
            }
        }
        offset += 0x20;
        ++i;
    }
    (*(void (***)(void*))(AsCRipple(self)->dropletEffect))[3](AsCRipple(self)->dropletEffect);
}
#pragma opt_lifetimes reset

extern void* __nw__FUl(unsigned int);
extern void W3dAniSpr_Ctor(void*);
extern void W3dAniSpr_LoadTexture(void*, void*, float, float);
extern void W3dAniSpr_SetSizeAndPivot(void*, float, float, float, float);
extern void CPetBody_SetOpacity(void*, unsigned char);
extern void CPetBody_SetupLighting(void*);
extern void __ml__FRC7WVectorf(void*, void*, float);
extern void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern void CPetBody_SetPosition(void*, void*);
extern void CPetBody_SetRotation(void*, void*);
extern void* CQuadTree_GetInstance();
extern void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
extern void Sound_PlayByName(void*, char*, int, int, int, int, float, float, float);
extern void __amu__7WVectorFf(void*, float);
extern void __apl__7WVectorFRC7WVector(void*, void*);
extern void* g_soundManager;
extern const float kFloat_0_1_Particle, kFloat_0_5_Particle, kFloat_255_SpriteAlpha;
extern const float kFloat_1_805DF43C;
extern const float kFloat_200_shotParticles, kFloat_0_2_ShotEffects;
extern const float kFloat_0_80000001_ShotEffects; // 0.8: ring lifetime / spark fade multiplier
extern const float kFloat_15_ShotEffects, kFloat_7_ShotEffects; // 15: coin motion/spin scale / first ripple growth rate; 7: coin spark velocity scale
extern int rand();
extern void WPuppet_UpdateBound(void*, int);
extern const float kFloat_6_2831855_ShotEffects, kFloat_0_03125_ShotEffects, kFloat_1_5_ShotEffects; // 2*pi: full-turn angle; 1/32: coin angle random scale; 1.5: droplet lifetime / initial coin bounce speed
extern const float kFloat_1_3_ShotEffects, kFloat_0_0049999999_ShotEffects, kFloat_0_25_805DF468, kFloat_2_0; // 1.3: secondary coin launch-height bias; 0.005: fade-to-emission scale / coin velocity jitter
extern const float kFloat_0_050000001_ShotEffects, kFloat_0_02_ShotEffects, kFloat_10_CourseEffects; // 0.05: spawn velocity jitter scale; 0.02: coin spark radial jitter scale
extern void CPetBody_Ctor(void*, char*, int);
extern void CPetBody_SetPosition(void*, void*);
extern void Matrix_SetAxisRotation(void*, float, signed char);
extern void Matrix_TransformPoint(void*, void*, void*);
extern void __as__7WVectorFRC7WVector(void*, void*);

struct EffectObject {
    char bytes[0x1B4];
    // Construct the coin pet-body wrapper from its resource name and flags.
    EffectObject(char* name, int flag) { CPetBody_Ctor(this, name, flag); }
};
// Reset and update share the exact 0x18-byte schema above, not a larger partial view.
typedef CoinModel Entry;

// Create missing coin models and launch the coins and spark sprites around the effect origin.
// Coin models persist across resets; velocities/angles are reinitialized. spriteCount must
// be zero or greater than one because spark azimuth divides by spriteCount - 1.
// Keep this reset's address-based copies: replacing them with typed member
// addresses changed CCoin_InitializeLaunchAndSparks's exception entry under the exact build recipe.
// Their schema is CoinModel/CoinSprite above: coin +8=velocity; spark +4=position,
// +10=velocity, +1C=opacity, +20=remainingLife, +24=fadeThreshold, +0=active.
// The typed update below uses the same storage. No float/RNG order may change.
void CCoin_InitializeLaunchAndSparks(char* self) {
    if (AsCCoin(self)->coinCount > 0) { WVector out;  float launchRotation[12];
        Entry* entry = (Entry*)AsCCoin(self)->coinEntries;
        if (entry->object == 0) {
            EffectObject* p = new EffectObject((courseEffectStrings + IFF_OFFSET(CourseEffectStringPool, coinPetCommand)), 0);
            entry->object = p;
            void* rv = CPetBody_GetPuppet(p);
            if (rv) WPuppet_UpdateBound(rv, 1);
        }
        void* savedObject = entry->object;
        CPetBody_SetPosition(savedObject, self + 0x134);
        int n = rand();
        int angleRandom = n;
        entry->angle = kFloat_6_2831855_ShotEffects * (float)(angleRandom % 32) * kFloat_0_03125_ShotEffects;
        n = rand();
        int bounceSpeedRandom = n;
        float firstBounceVelocity = kFloat_1_5_ShotEffects + kFloat_0_1_Particle * (float)(bounceSpeedRandom % 5 - 2);
        float savedBounceVelocity = firstBounceVelocity;
        entry->bounceVelocity = savedBounceVelocity;

        float pos[3] = {kFloat_0_3_805DF4A4, firstBounceVelocity, kFloat_0_Particles};
          Matrix_SetAxisRotation(launchRotation, kFloat_6_2831855_ShotEffects / (float)(rand() % 32 + 1), 1);
        Matrix_TransformPoint(&out, pos, launchRotation);
        *(WVector*)((char*)entry + 8) = out;
    }
    // Phase 2: secondary coins spread around a full turn; coin zero has its own random launch.
    if (AsCCoin(self)->coinCount > 1) {
        for (int i = 1; i < AsCCoin(self)->coinCount; ++i) {
            Entry* e = (Entry*)((char*)AsCCoin(self)->coinEntries + i * 0x18);
            if (0 == e->object) {
                EffectObject* p = new EffectObject((courseEffectStrings + IFF_OFFSET(CourseEffectStringPool, coinPetCommand)), 0);
                e->object = p;
                void* rv = CPetBody_GetPuppet(p);
                if (rv) WPuppet_UpdateBound(rv, 1);
            }
                void* savedObject2 = e->object;
                CPetBody_SetPosition(savedObject2, self + 0x134);
            int n = rand();
            e->angle = kFloat_6_2831855_ShotEffects * (float)(n % 32) * kFloat_0_03125_ShotEffects;
            n = rand();
            int bounceSpeedRandom = n;
            float bounceVelocity = kFloat_0_1_Particle * (float)(bounceSpeedRandom % 5 - 2) + kFloat_1_3_ShotEffects;
            float savedBounceVelocity = bounceVelocity;
            e->bounceVelocity = savedBounceVelocity;
            WVector c;
            n = rand();
            float a[3] = {kFloat_0_25_805DF468 + kFloat_0_0049999999_ShotEffects * (float)(n % 51 - 25), bounceVelocity, kFloat_0_Particles};
            float b[12];
            Matrix_SetAxisRotation(b, kFloat_6_2831855_ShotEffects * (float)i / (float)(AsCCoin(self)->coinCount - 1), 1);
            Matrix_TransformPoint(&c, a, b); *(WVector*)((char*)e + 8) = c;
        }
    }
    // Phase 3: initialize the independent spark ring with world-space positions/velocities.
    // Complete CoinSprite schema, stride 0x28 (same type used by vfunc_1/Draw):
    // +00 u8 active; +01..03 padding (untouched); +04 float position[3] world XYZ;
    // +10 float velocity[3] launch coefficients (Update scales by 7 before dt);
    // +1C float opacity 0..255; +20 float remainingLife seconds (.5.. .95);
    // +24 float fadeThreshold seconds (= .8*initial remainingLife).
    // Required spriteEntries has spriteCount records; spriteCount>=2 here since
    // angular spacing divides by count-1. Records borrow the shared sprite/bitmap,
    // own no resources. Preserve RNG order: height, horizontal speed, lifetime.
    // Raw vector stores retain the original WVector copy ABI; scalar stores below
    // use CoinSprite so their domain is visible beside the initialization.
    for (int i = 0; i < AsCCoin(self)->spriteCount; ++i) { float q[12];  WVector r;
        char* item = (char*)AsCCoin(self)->spriteEntries + i * 0x28;
                        char* savedItem2 = item + 4;
                        *(WVector*)savedItem2 = *(WVector*)(self + 0x134);
        int n = rand();
        float h = kFloat_2_0 + kFloat_0_050000001_ShotEffects * (float)(n % 21 - 10);

        n = rand();
        float v[3] = {kFloat_1_3_ShotEffects + kFloat_0_02_ShotEffects * (float)(n % 5 - 2), h, kFloat_0_Particles};
          Matrix_SetAxisRotation(q, kFloat_6_2831855_ShotEffects * (float)i / (float)(AsCCoin(self)->spriteCount-1), 1);
        Matrix_TransformPoint(&r, v, q); *(WVector*)(item + 0x10) = r;
        ((CoinSprite*)item)->opacity=kFloat_255_SpriteAlpha;
        ((CoinSprite*)item)->remainingLife=kFloat_0_5_Particle + (float)(rand()%10)*kFloat_0_5_Particle/kFloat_10_CourseEffects;
        ((CoinSprite*)item)->fadeThreshold=kFloat_0_80000001_ShotEffects * ((CoinSprite*)item)->remainingLife;
        ((CoinSprite*)item)->active=1;
    }
}

struct Fx1C68 {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(int);
    virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8();
    virtual void v9(); virtual void v10(); virtual void v11(); virtual void v12();
    virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16();
    virtual void v17(); virtual void v18(); virtual void v19(); virtual void v20();
    virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24();
    virtual void v25(); virtual void v26(); virtual void v27(float);
};
struct IV1C68 { int a, b, c; };

// Integrate and bounce coins, limit bounce sounds, fade sparks, and stop on expiry.
// self/arrays and every coin model are required when enabled; counts bound the
// arrays. delta is task seconds, fadeDuration must be nonzero, active spark
// fadeThreshold is positive after vfunc_6. No validation/failure return exists.
// Position uses old velocity, then rotation advances by 15 radians/second,
// horizontal drag approaches zero by 0.1 stored-velocity units/second and gravity
// subtracts 3 stored-velocity units/second. Ground contact halves retained bounce
// velocity, snaps world Y and replaces vertical velocity; at most four contacts
// play sound across the whole effect, reset by RestartCoinEffect. Spark late-life
// damping is per update (including delta==0); expiry tests after subtracting delta.
void CCoin_UpdateCoinsAndSprites(char* self, float delta) {
    if (AsCCoin(self)->enabled) {
        AsCCoin(self)->opacity -= delta / AsCCoin(self)->fadeDuration;
        if (AsCCoin(self)->opacity < kFloat_0_Particles) {
            AsCCoin(self)->opacity = kFloat_1_805DF43C;
            ((Fx1C68*)self)->v4(0);
        }
        char* modelPosition;
        int off, j;
        // Fade still permits this update's model/spark work after disabling the effect.
        // Integration order is velocity*15, then *delta, then position addition.
        float horizontalDragStep = kFloat_0_1_Particle * delta;
        int coinByteOffset, coinIndex;
        coinIndex = 0;
        coinByteOffset = 0;
        while (coinIndex < AsCCoin(self)->coinCount) { IV1C68 coinPosition;
            CoinModel* item = (CoinModel*)((char*)AsCCoin(self)->coinEntries + coinByteOffset);

            float nextPosition[3];
            float displacement[3];
            float worldVelocity[3];
            float rotation[3];
            float groundPoint[3];
            int coinAlpha = (int)(kFloat_255_SpriteAlpha * AsCCoin(self)->opacity / kFloat_0_2_ShotEffects);
            IV1C68 groundQueryPosition;

            CPetBody_SetOpacity(item->object, (kOpaqueAlpha <= coinAlpha ? kOpaqueAlpha : coinAlpha));
            CPetBody_SetupLighting(item->object);
            modelPosition = (char*)item->object + 0x80; // CPetBody world position, not record storage.
            __ml__FRC7WVectorf(worldVelocity, &item->velocity, kFloat_15_ShotEffects);
            __ml__FRC7WVectorf(displacement, worldVelocity, delta);
            __pl__FRC7WVectorRC7WVector(nextPosition, modelPosition, displacement);
            CPetBody_SetPosition(item->object, nextPosition);
            float angle = item->angle;
            rotation[0] = kFloat_0_Particles; rotation[1] = angle; rotation[2] = kFloat_0_Particles;
            CPetBody_SetRotation(item->object, rotation);
            float* angleToAdvance = &item->angle;
            *angleToAdvance += kFloat_15_ShotEffects * delta;
            float componentVelocity = item->velocity.x;
            if (componentVelocity > horizontalDragStep) item->velocity.x = componentVelocity - horizontalDragStep;
            else if (componentVelocity < -horizontalDragStep) item->velocity.x = componentVelocity + horizontalDragStep;
            componentVelocity = item->velocity.z;
            if (componentVelocity > horizontalDragStep) item->velocity.z = componentVelocity - horizontalDragStep;
            else if (componentVelocity < -horizontalDragStep) item->velocity.z = componentVelocity + horizontalDragStep;
            item->velocity.y -= kFloat_3_ShotEffects * delta;
            const IV1C68* source;
            source = (const IV1C68*)((char*)item->object + 0x80);
            *(unsigned long long*)&coinPosition = *(const unsigned long long*)source;
            coinPosition.c = source->c;
            groundQueryPosition = coinPosition;
            void* groundTree = CQuadTree_GetInstance();
            GetGroundPoint__9CQuadTreeF7WVectorbPib(groundPoint, groundTree, &groundQueryPosition, 1, 0, 1);
            float groundY = groundPoint[1];
            if (*(float*)&coinPosition.b < groundY) {
                if (AsCCoin(self)->bounceSoundCount < 4) {
                    Sound_PlayByName(g_soundManager,(courseEffectStrings + IFF_OFFSET(CourseEffectStringPool, coinDropSound)), 0, 0, 0, 0, kFloat_0_5_Particle, kFloat_200_shotParticles, kFloat_1_805DF43C);
                    ++AsCCoin(self)->bounceSoundCount;
                }
                item->bounceVelocity *= kFloat_0_5_Particle;
                *(float*)&coinPosition.b = groundY;
                item->velocity.y = item->bounceVelocity;
                CPetBody_SetPosition(item->object, &coinPosition);
            }
            ((Fx1C68*)item->object)->v27(delta);
            coinByteOffset += sizeof(CoinModel); ++coinIndex;
        }
        // Coin models are updated; age/integrate sparks with additional end-of-life damping.
        j = 0; off = 0;
        while (j < AsCCoin(self)->spriteCount) {
            CoinSprite* item = (CoinSprite*)((char*)AsCCoin(self)->spriteEntries + off);
            if (item->active) {
                if (item->remainingLife < item->fadeThreshold) {
                    __amu__7WVectorFf(&item->velocity, kFloat_0_80000001_ShotEffects * item->remainingLife / item->fadeThreshold);
                    item->opacity *= item->remainingLife / item->fadeThreshold;
                }
                float displacement[3], worldVelocity[3];
                __ml__FRC7WVectorf(worldVelocity, &item->velocity, kFloat_7_ShotEffects);
                __ml__FRC7WVectorf(displacement, worldVelocity, delta);
                __apl__7WVectorFRC7WVector(&item->position, displacement);
                item->remainingLife -= delta;
                if (item->remainingLife < kFloat_0_Particles) item->active = 0;
            }
            off += sizeof(CoinSprite); ++j;
        }
    }
}

struct Obj19C {
    char bytes[0x19C];
    // Construct the coin spark's animated billboard in its allocation wrapper.
    Obj19C() { W3dAniSpr_Ctor(this); }
};

struct DelObj { 
virtual ~DelObj();
};
struct Self1490 {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(int); virtual void v5(); virtual void v6();
    char pad[0x10]; void* q; int pad1; DelObj* p; char pad2[0x120]; float f140; float f144; int i148;
};

#pragma use_lmw_stmw on
// Recreate the coin spark grid, reset fade/sound state, and launch and enable the effect.
void CCoin_RestartCoinEffect(Self1490* self) {
    self->f144 = kFloat_1_805DF43C;
    self->f140 = kFloat_3_ShotEffects;
    if (self->p) {
        delete self->p;
        self->p = 0;
    }
    Obj19C* obj = new Obj19C;
    self->p = (DelObj*)obj;
    float c = kFloat_32_SpriteGrid;
    (*(void**)&((W3dAniSpr*)obj)->resourceManager) = (void*)g_resourceManager;
    W3dAniSpr_LoadTexture(self->p, self->q, c, c);
    W3dAniSpr_SetSizeAndPivot(self->p, kFloat_0_3_805DF4A4, kFloat_0_3_805DF4A4, kFloat_0_15000001_ShotEffects, kFloat_0_15000001_ShotEffects);
    self->i148 = 0;
    self->v6();
    self->v4(1);
}

} // extern "C"

} // namespace ns3

// ---- merged from course_ball_shot_effects.cpp ----
namespace ns4 {
// Translation unit 0x801B3830-0x801B4F20 (18 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void* memset(void*, int, unsigned int);

extern const float kFloat_6_2831855;
extern float s_particleEffectsFullTurnAngle;
void __dl__FPv(void*);
void appFreeUni__FPv(void*);
void PlayerModel_ReleaseResourcesByMask(void*, int);
int ItemSelection_Validate(void*);
void* ItemInventory_FindPartData(void*, unsigned int);
void W3dAniSpr_SetColor(void*, unsigned int);
void* GolfBall__Fv();
void W3dAniSpr_SetPosition(void*, void*);
void W3dAniSpr_SetSizeAndPivot(void*, float, float, float, float);
void W3dAniSpr_DrawFrame(void*, unsigned int, float, unsigned int, unsigned int);
void Vec2_Zero(void*);
int ResolvedItemSelection_EquipPart(void*, unsigned int);
// 0x801B4904: mask scans return at the first conflicting equipped entry.
inline int SelectionHasNoMaskConflict(unsigned int*& cursor, int& index, unsigned int& equippedEntry, int count, int excludedIndex, unsigned int conflictMask) {
    for (; index < count; cursor += 2, index++) {
        equippedEntry = cursor[3];
        if (equippedEntry && excludedIndex != index && (conflictMask & ((IFF_STRUCT::sPart*)equippedEntry)->hideMask)) return 0;
    }
    return 1;
}
// Return whether no equipped part covers any slot in the proposed hide mask.
inline int SelectionAllowsMask(unsigned int*& cursor, int& index, unsigned int& equippedEntry, unsigned char* base, unsigned int conflictMask) {
    for (; index < ((ItemSelection801B7688*)base)->totalCount; cursor += 2, index++) {
        equippedEntry = cursor[3];
        if (equippedEntry && index != kUnspecifiedSlot && (conflictMask & ((IFF_STRUCT::sPart*)equippedEntry)->hideMask)) return 0;
    }
    return 1;
}

int ResolvedItemSelection_ReconcileSubparts(void*);
extern unsigned int g_view;
extern CSharedDoc* g_sharedDoc;
extern float kFloat_2_0;
extern const float kFloat_1_805DF43C;
extern const float kFloat_0_5_Particle;
extern const float kFloat_0_25_805DF468;
extern const float kFloat_0_Particles;
extern const float kFloat_negativeZero_ShotEffects; // negative zero: second dust vertical-velocity threshold
extern const float kFloat_0_0049999999_ShotEffects; // 0.005: fade-to-emission scale / coin velocity jitter
extern const float kFloat_150_ShotEffects; // 150: second dust opacity fade rate
extern const float kFloat_9_9999997eneg06_ShotEffects; // 0.00001: near-zero threshold
extern const float kFloat_18_849556_ShotEffects; // 6*pi: dust sprite-frame rate
extern float kFloat_100_ShotEffects; // 100: second dust particle opacity fade rate
extern CSharedDoc* g_sharedDoc;
extern unsigned int g_sceneManager;
extern char courseEffectStrings[];
void CStarDust2_UpdateGlowAndParticles(void*, float);
void __ml__FfRC7WVector(void*, void*, float);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void __as__7WVectorFRC7WVector(void*, void*);
void Vector_CrossProduct(void*, void*, void*);
void* Normalize__7WVectorFv(void*);
void Vec3_DivideScalar(void*, float, const void*);
float Magnitude__7WVectorCFv(void*);
void __ml__FRC7WVectorf(void*, void*, float);
void __ml__FfRC7WVector(void*, void*, float);
int rand();
extern const float kFloat_4_ShotEffects; // 4: second dust radial-scale multiplier
extern const double kDouble_SignedIntConversionBias_Particles;
extern const float kFloat_1_5707964_ShotEffects; // pi/2: orbit spin base / center frame rate
extern const float kFloat_0_0009765625_805DF488;
extern const float kFloat_0_001953125_ShotEffects; // 1/512: signed random-axis scale
extern const float kFloat_6_2831855_ShotEffects; // 2*pi: full-turn angle
extern const float kFloat_255_SpriteAlpha;
extern char s_cameraTransform[];
float Math_SinApprox(float);
float Math_CosApprox(float);
int PartyItemSlots_GetInstance();
int PartyItemSlots_GetItemType(int);
void NamedEffectList_FindByName(void*, void*);
void WPuppet_SetOpacityRecursive(int);

// Validate a packed selection and resolve each part key to its inventory record.
// self is 0xC8 bytes; src is a validated packed-key selection. Returns 0 without
// writing when validation fails; record pointers are borrowed from the item inventory.
// src is a required packed-key selection, validated by ItemSelection_Validate;
// input keys advance by 4 bytes, resolved output entries by 8. Buffers must not
// overlap because the output is cleared before copying source fields/keys.
// Validation success does not promise every inventory lookup succeeds: a nonzero
// stored key can accompany a null record. Mutation APIs require resolved records.
int ItemSelection_ResolvePartData(void* self, void* src) {
	if (!ItemSelection_Validate(src))
		return 0;
	memset((char*)self + 8, 0, 0xc0);
	unsigned int _w0 = *(volatile unsigned int*)src;
	unsigned char _b4 = *(volatile unsigned char*)((char*)src + 4);
	unsigned char _b5 = *(volatile unsigned char*)((char*)src + 5);
	unsigned char _b6 = *(volatile unsigned char*)((char*)src + 6);
	unsigned char _b7 = *(volatile unsigned char*)((char*)src + 7);
	*(unsigned int*)self = _w0;
	((ItemSelection801B7688*)self)->primaryCount = _b4;
	((ItemSelection801B7688*)self)->totalCount = _b5;
	((unsigned char*)self)[6] = _b6;
	((unsigned char*)self)[7] = _b7;
	char* entry = (char*)self;
	int i = 0;
	for (; i < ((ItemSelection801B7688*)self)->totalCount; i++) {
		if (*(unsigned int*)((char*)src + 8)) {
			void* value = ItemInventory_FindPartData(g_sharedDoc->itemInventory, *(unsigned int*)((char*)src + 8)); // original +0xA04
			*(unsigned int*)(entry + 8) = *(unsigned int*)((char*)src + 8);
			*(void**)(entry + 0xc) = value;
		}
		src = (char*)src + 4;
		entry = entry + 8;
	}
	return 1;
}

// Draw ball glow and orbit dust; particle/effect packed color words are combined by bitwise AND.
void CStarDust2_DrawGlowAndParticles(void* self) {
	if (!AsCStarDust2(self)->active)
		return;
	int color = (int)AsCStarDust2(self)->opacity;
	unsigned int alpha = (color << 24) | kWhiteRgb;
	W3dAniSpr_SetColor(AsCStarDust2(self)->sprite, alpha);
	W3dAniSpr_SetPosition(AsCStarDust2(self)->sprite, GolfBall__Fv());
	W3dAniSpr_SetSizeAndPivot(AsCStarDust2(self)->sprite, kFloat_2_0, kFloat_2_0, kFloat_1_805DF43C, kFloat_1_805DF43C);
	W3dAniSpr_DrawFrame(AsCStarDust2(self)->sprite, g_view, AsCStarDust2(self)->spriteFrame, 0x20f00000, 5);
	for (int i = 0; i < AsCStarDust2(self)->particleCount; i++) {
		Orbit* entry = OrbitAt(AsCStarDust2(self)->particles, i);
		if (entry->active) {
			W3dAniSpr_SetPosition(AsCStarDust2(self)->sprite, &entry->position);
			W3dAniSpr_SetSizeAndPivot(AsCStarDust2(self)->sprite, kFloat_0_5_Particle, kFloat_0_5_Particle, kFloat_0_25_805DF468, kFloat_0_25_805DF468);
			int ec = (int)((StarDust2Particle*)entry)->opacity;
			W3dAniSpr_SetColor(AsCStarDust2(self)->sprite, alpha & ((ec << 24) | kWhiteRgb));
			W3dAniSpr_DrawFrame(AsCStarDust2(self)->sprite, g_view, kFloat_0_Particles, 0x00f00000, 6);
		}
	}
}

#pragma opt_common_subs off
#pragma optimize_for_size on
// Clear a resolved equipment selection's header and all 24 {key,record} pairs.
// self is required 0xC8-byte storage; record pointers are borrowed IFF data and
// require no release. Header/first pair are explicit stores, then 23 pairs use
// guarded 8-pair blocks and a tail. Pointer words/overflow tests remain unsigned
// 32-bit arithmetic with the original signed-count checks; do not replace the
// expanded form with memset or change pointer widths in a matching pass.
void ResolvedItemSelection_Clear(void* self) {
	// The header is 8 bytes; each resolved entry is a key and record pointer.
	// Keep the expanded fill guards and store order to preserve the compiled code.
	unsigned int pairsEndAddress, unrolledEndAddress, pairCursorAddress, pairsStartAddress, pairRangeBytes, roundedPairRangeBytes, pairLimitPlusOne;
	int pairIndexLimit;
	pairCursorAddress = (unsigned int)self + 0x10;
	pairsEndAddress = (unsigned int)self + 0xc8;
	*(int*)self = 0;
	((char*)self)[4] = 0;
	((char*)self)[5] = 0;
	((char*)self)[6] = 0;
	((char*)self)[7] = 0;
	(*(int*)&((ResolvedSelection*)self)->parts[0].key) = 0;
	(*(int*)&((ResolvedSelection*)self)->parts[0].record) = 0;
	if (pairCursorAddress < pairsEndAddress) {
		pairsStartAddress = (unsigned int)self + 0x10;
		unrolledEndAddress = (unsigned int)self + 0x88;
		pairRangeBytes = pairsEndAddress - pairsStartAddress;
		roundedPairRangeBytes = pairRangeBytes + 7;
		pairIndexLimit = (int)roundedPairRangeBytes / 8;
		pairLimitPlusOne = (unsigned int)pairIndexLimit + 1;
		if ((int)pairLimitPlusOne > 8) {
			unsigned int canUseUnrolledStores = 0, byteRangeHasNoOverflow = 0;
			if (pairsStartAddress <= pairsEndAddress) {
				unsigned int compatibleByteRangeSigns = 1;
				if ((pairRangeBytes & 0x80000000) == 0) {
					if ((roundedPairRangeBytes & 0x80000000) != 0) compatibleByteRangeSigns = 0;
				}
				if (compatibleByteRangeSigns) byteRangeHasNoOverflow = 1;
			}
			if (byteRangeHasNoOverflow) {
				int pairCountSign = pairIndexLimit & 0x80000000;
				unsigned int compatiblePairCountSigns = 1;
				if (pairCountSign == 0) {
					if (pairCountSign != (int)(pairLimitPlusOne & 0x80000000)) compatiblePairCountSigns = 0;
				}
				if (compatiblePairCountSigns) canUseUnrolledStores = 1;
			}
			if (canUseUnrolledStores) {
				unsigned int zero = 0;
				for (; pairCursorAddress < unrolledEndAddress; pairCursorAddress += 0x40) {
					*(unsigned int*)(pairCursorAddress + 0x00) = zero;
					*(unsigned int*)(pairCursorAddress + 0x04) = zero;
					*(unsigned int*)(pairCursorAddress + 0x08) = zero;
					*(unsigned int*)(pairCursorAddress + 0x0c) = zero;
					*(unsigned int*)(pairCursorAddress + 0x10) = zero;
					*(unsigned int*)(pairCursorAddress + 0x14) = zero;
					*(unsigned int*)(pairCursorAddress + 0x18) = zero;
					*(unsigned int*)(pairCursorAddress + 0x1c) = zero;
					*(unsigned int*)(pairCursorAddress + 0x20) = zero;
					*(unsigned int*)(pairCursorAddress + 0x24) = zero;
					*(unsigned int*)(pairCursorAddress + 0x28) = zero;
					*(unsigned int*)(pairCursorAddress + 0x2c) = zero;
					*(unsigned int*)(pairCursorAddress + 0x30) = zero;
					*(unsigned int*)(pairCursorAddress + 0x34) = zero;
					*(unsigned int*)(pairCursorAddress + 0x38) = zero;
					*(unsigned int*)(pairCursorAddress + 0x3c) = zero;
				}
			}
		}
		unsigned int tailEndAddress = (unsigned int)self + 0xc8;
		unsigned int zero = 0;
		for (; pairCursorAddress < tailEndAddress; pairCursorAddress += 8) {
			*(unsigned int*)(pairCursorAddress + 0) = zero;
			*(unsigned int*)(pairCursorAddress + 4) = zero;
		}
	}
}
#pragma optimize_for_size reset
#pragma opt_common_subs reset

#pragma opt_strength_reduction off
#pragma opt_lifetimes off
// Emit four randomized orbiting dust particles into the circular particle buffer.
// Requires positive particleCount and a nonzero velocity/camera-basis cross product.
// Four consecutive slots are overwritten with new world-space origins/axes.
void CStarDust2_EmitOrbitingParticles(void* self) {
	void* state = GolfBall__Fv();
	char* layout = s_cameraTransform;
	float orbitBase[3];
	Vector_CrossProduct(orbitBase, &((Ball*)state)->velocity, &AsCameraTransform800D(layout)->basis[6]);
	float ballScale = ((Ball*)GolfBall__Fv())->effectScale;
	void* unitOrbitBase = Normalize__7WVectorFv(orbitBase);
	float factor = kFloat_2_0;
	float doubledOrbitBase[3];
	__ml__FRC7WVectorf(doubledOrbitBase, unitOrbitBase, factor);
	float scaledOrbitBase[3];
	__ml__FRC7WVectorf(scaledOrbitBase, doubledOrbitBase, ballScale);
	((void (*)(void*, void*))__as__7WVectorFRC7WVector)(orbitBase, scaledOrbitBase);
	int unusedConstantWord = 0x80200803;
	float ballScaleMultiplier = kFloat_4_ShotEffects;
	double integerConversionBias = kDouble_SignedIntConversionBias_Particles;
	float spinBase = kFloat_1_5707964_ShotEffects;
	float randomUnitScale = kFloat_0_0009765625_805DF488;
	float axisRandomScale = kFloat_0_001953125_ShotEffects;
	float fullTurnAngle = kFloat_6_2831855_ShotEffects;
	float initialOpacity = kFloat_255_SpriteAlpha;
	int unusedZeroWord = 0;
	int unusedEnabledWord = 1;
	for (int i = 0; i < 4; i++) {
		int index = AsCStarDust2(self)->nextParticleIndex;
		int count = AsCStarDust2(self)->particleCount;
		AsCStarDust2(self)->nextParticleIndex = index + 1 - ((index + 1) / count) * count;
		Orbit* particle = OrbitAt(AsCStarDust2(self)->particles, index);
		particle->active = 1;
		((StarDust2Particle*)particle)->radialScale = ((Ball*)GolfBall__Fv())->effectScale * ballScaleMultiplier;
		int randomSample = rand();
		((StarDust2Particle*)particle)->angularVelocity = spinBase * (float)(randomSample % 1023) * randomUnitScale + spinBase;
		randomSample = rand();
		float cosineJitterZ = (float)((randomSample % 1024) - 0x200) * axisRandomScale;
		randomSample = rand();
		float cosineJitterY = (float)((randomSample % 1024) - 0x200) * axisRandomScale;
		randomSample = rand();
		float cosineJitterX = (float)((randomSample % 1024) - 0x200) * axisRandomScale;
		float cosineRandomVector[3] = { cosineJitterX, cosineJitterY, cosineJitterZ };
		float cosineJitterOffset[3]; // WVector result (12 bytes), was cosineJitterZ 4-byte void*
		((void (*)(void*, void*, float))__ml__FfRC7WVector)(cosineJitterOffset, cosineRandomVector, factor);
		float cosineAxis[3]; // WVector result (12 bytes), was cosineJitterZ 4-byte void*
		__pl__FRC7WVectorRC7WVector(cosineAxis, &AsCameraTransform800D(layout)->basis[6], cosineJitterOffset);
		((void (*)(void*, void*))__as__7WVectorFRC7WVector)(&particle->cosineAxis, cosineAxis);
		randomSample = rand();
		float sineJitterZ = (float)((randomSample % 1024) - 0x200) * axisRandomScale;
		randomSample = rand();
		float sineJitterY = (float)((randomSample % 1024) - 0x200) * axisRandomScale;
		randomSample = rand();
		float sineJitterX = (float)((randomSample % 1024) - 0x200) * axisRandomScale;
		float sineRandomVector[3] = { sineJitterX, sineJitterY, sineJitterZ };
		float sineJitterOffset[3]; // WVector result (12 bytes), was cosineJitterZ 4-byte void*
		((void (*)(void*, void*, float))__ml__FfRC7WVector)(sineJitterOffset, sineRandomVector, factor);
		float magnitude = Magnitude__7WVectorCFv(orbitBase);
		float unitOrbitDirection[3]; // WVector result (12 bytes), was cosineJitterZ 4-byte void*
		Vec3_DivideScalar(unitOrbitDirection, magnitude, orbitBase);
		float sineAxis[3]; // WVector result (12 bytes), was cosineJitterZ 4-byte void*
		__pl__FRC7WVectorRC7WVector(sineAxis, unitOrbitDirection, sineJitterOffset);
		((void (*)(void*, void*))__as__7WVectorFRC7WVector)(&particle->sineAxis, sineAxis);
		((void (*)(void*, void*))__as__7WVectorFRC7WVector)(&particle->origin, GolfBall__Fv());
		randomSample = rand();
		((StarDust2Particle*)particle)->angle = fullTurnAngle * (float)(randomSample % 1023) * randomUnitScale;
		((StarDust2Particle*)particle)->opacity = initialOpacity;
	}
}
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

#pragma opt_common_subs off
struct StarDust2UpdateVirtual {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6();
};
// Fade the ball glow as it descends, emit dust, and integrate/fade every live orbit particle.
void CStarDust2_UpdateGlowAndParticles(void* self, float delta) {
	float ballScaleFraction, zeroOpacity, particleFadeRate, radialAcceleration;
	if (AsCStarDust2(self)->active) {

		if (((Ball*)GolfBall__Fv())->velocity.y < kFloat_negativeZero_ShotEffects) {
			float factor = kFloat_0_0049999999_ShotEffects * AsCStarDust2(self)->opacity;
			float step = factor * delta;
			step = (delta > step) ? step : delta;
			AsCStarDust2(self)->emissionElapsed += step;
			float remain = AsCStarDust2(self)->opacity - kFloat_150_ShotEffects * delta;
			remain = (kFloat_0_Particles < remain) ? remain : kFloat_0_Particles;
			AsCStarDust2(self)->opacity = remain;
			void* state = (void*)(unsigned long)g_sharedDoc;
 			if ((*(int*)&((CSharedDoc*)state)->partyGameMode) == kPartyItemMode && (((int (*)(int, int))PartyItemSlots_GetItemType)(PartyItemSlots_GetInstance(), AsCGolfDoc((void*)((CSharedDoc*)state)->golfDoc)->currentPlayer) == kOpaqueBallPartyItem)) {
				((void (*)(void*, unsigned char))WPuppet_SetOpacityRecursive)(((void* (*)(void*, void*))NamedEffectList_FindByName)((void*)(unsigned long)g_sceneManager,((CourseEffectStringPool*)courseEffectStrings)->ballObject), kOpaqueAlpha);
			} else {
				((void (*)(void*, unsigned char))WPuppet_SetOpacityRecursive)(((void* (*)(void*, void*))NamedEffectList_FindByName)((void*)(unsigned long)g_sceneManager,((CourseEffectStringPool*)courseEffectStrings)->ballObject), kOpaqueAlpha - (int)AsCStarDust2(self)->opacity);
			}
			float intensity;
			if ((*(unsigned int*)&AsCStarDust2(self)->opacity) & kFloatSignBit) intensity = -AsCStarDust2(self)->opacity;
			else intensity = AsCStarDust2(self)->opacity;
			int below = intensity < kFloat_9_9999997eneg06_ShotEffects;
			if (below) {
				AsCStarDust2(self)->active = 0;
				AsCStarDust2(self)->opacity = kFloat_0_Particles;
				((void (*)(void*, unsigned char))WPuppet_SetOpacityRecursive)(((void* (*)(void*, void*))NamedEffectList_FindByName)((void*)(unsigned long)g_sceneManager,((CourseEffectStringPool*)courseEffectStrings)->ballObject), kOpaqueAlpha);
			}
		} else {
			AsCStarDust2(self)->emissionElapsed += delta;
		}
		// Emit at most one four-particle batch here, even after a long delta (unlike CStarDust).
		if (AsCStarDust2(self)->emissionElapsed > AsCStarDust2(self)->emissionInterval) {
			AsCStarDust2(self)->emissionElapsed -= AsCStarDust2(self)->emissionInterval;
			((StarDust2UpdateVirtual*)self)->v6();
		}
		float nextFrame = AsCStarDust2(self)->spriteFrame - kFloat_18_849556_ShotEffects * delta;
		radialAcceleration = kFloat_2_0;
		particleFadeRate = kFloat_100_ShotEffects;
		zeroOpacity = kFloat_0_Particles;
		ballScaleFraction = kFloat_0_5_Particle;
		AsCStarDust2(self)->spriteFrame = nextFrame;
		int inactive = 0;
		for (int i = 0; i < AsCStarDust2(self)->particleCount; i++) {
			Orbit* particle = OrbitAt(AsCStarDust2(self)->particles, i);
			if (!particle->active) continue;
			((StarDust2Particle*)particle)->angularVelocity -= radialAcceleration * delta;
			((StarDust2Particle*)particle)->angle -= ((StarDust2Particle*)particle)->angularVelocity * delta;
			float position[3], scaledOffset[3], orbitOffset[3], cosineOffset[3], sineOffset[3];
			float sine = Math_SinApprox(((StarDust2Particle*)particle)->angle);
			__ml__FfRC7WVector(sineOffset, &particle->sineAxis, sine);
			float cosine = Math_CosApprox(((StarDust2Particle*)particle)->angle);
			__ml__FfRC7WVector(cosineOffset, &particle->cosineAxis, cosine);
__pl__FRC7WVectorRC7WVector(orbitOffset, cosineOffset, sineOffset);
((void (*)(void*, void*, float))__ml__FfRC7WVector)(scaledOffset, orbitOffset, ((StarDust2Particle*)particle)->radialScale);
__pl__FRC7WVectorRC7WVector(position, &particle->origin, scaledOffset);
			((void (*)(void*, void*))__as__7WVectorFRC7WVector)(&particle->position, position);
			((StarDust2Particle*)particle)->radialScale += radialAcceleration * delta;
			float opacity = ((StarDust2Particle*)particle)->opacity - particleFadeRate * delta;
			if (opacity > zeroOpacity) ((StarDust2Particle*)particle)->opacity = opacity;
			else ((StarDust2Particle*)particle)->opacity = zeroOpacity;
			float ballScale = ((Ball*)GolfBall__Fv())->effectScale;
			float radius = ((StarDust2Particle*)particle)->radialScale;
			float limit = ballScaleFraction * ballScale;
			if (radius < limit) particle->active = (unsigned char)inactive;
		}
	}
}
#pragma opt_common_subs reset

// Initialize the shared full-turn angle from its constant at startup.
void ParticleEffects_InitFullTurnAngle() {
	s_particleEffectsFullTurnAngle = kFloat_6_2831855;
}

// Set the same selection-state byte on two selections only when their character keys agree.
int ItemSelection_SetStateIfSameCharacter(unsigned int* self, unsigned int* other, unsigned char flag) {
	if (*self != *other)
		return 0;
	((unsigned char*)other)[6] = flag;
	((unsigned char*)self)[6] = flag;
	return 1;
}

// 0x801B42E4: compare keys without leaving the caller's validation flow.
inline int SelectionKeysEqual(unsigned int* source, unsigned int* destination, unsigned char count) {
    for (int i = 0; i < count; i++) {
        if (destination[2] != source[2]) return 0;
        source = (unsigned int*)((char*)source + 4);
        destination = (unsigned int*)((char*)destination + 8);
    }
    return 1;
}

// Validate and apply a part change, then copy changed keys back and return their bitmask.
// self is resolved (8-byte entries), src packed (4-byte keys). Return bits identify changed
// slots; 0 means either failure or no changed keys. Failure after EquipPart may have already
// mutated self: this operation is not transactional and does not roll back earlier changes.
int ItemSelection_CopyValidatedChanges(void* self, void* src, unsigned int value) {
    unsigned int characterIndex = (value >> kCharacterKeyShift) & 0xff;
    unsigned int destinationCharacterKey = *(unsigned int*)self;
    unsigned int sourceCharacterKey;
    if (characterIndex != (destinationCharacterKey & kCharacterKeyMask) || (sourceCharacterKey = *(unsigned int*)src, characterIndex != (sourceCharacterKey & kCharacterKeyMask)))
        return 0;
    int sameSelection;
    if (destinationCharacterKey != sourceCharacterKey) {
        sameSelection = 0;
    } else {
        unsigned char destinationBasePartCount = ((ItemSelection801B7688*)self)->primaryCount;
        unsigned char sourceBasePartCount = ((ItemSelection801B7688*)src)->primaryCount;
        unsigned char destinationPartCount;
        if (destinationBasePartCount != sourceBasePartCount || (destinationPartCount = ((ItemSelection801B7688*)self)->totalCount, destinationPartCount != ((ItemSelection801B7688*)src)->totalCount)) {
            sameSelection = 0;
        } else {
            sameSelection = SelectionKeysEqual((unsigned int*)src, (unsigned int*)self, destinationPartCount);
        }
    }
    if (!sameSelection || !ItemSelection_Validate(src))
        return 0;
    if (!ResolvedItemSelection_EquipPart(self, value))
        return 0;
    if (!ResolvedItemSelection_ReconcileSubparts(self))
        return 0;
    unsigned int changedSlots = 0;
    unsigned int* pSelf = (unsigned int*)self;
    unsigned int* pSrc = (unsigned int*)src;
    unsigned int key;
    int i = 0;
    unsigned int one = 1;
    // 0x801B42E4: update each selection and accumulate its changed bit.
    for (; i < ((ItemSelection801B7688*)self)->totalCount; i++) {
        key = pSelf[2];
        unsigned int previousKey = pSrc[2];
        if (previousKey != key) {
            pSrc[2] = key;
            changedSlots |= one << i;
        }
        pSelf = (unsigned int*)((char*)pSelf + 8);
        pSrc = (unsigned int*)((char*)pSrc + 4);
    }
    if (ItemSelection_Validate(src))
        return changedSlots;
    return 0;
}

// Resolve one {key, borrowed IFF record} entry from a cursor relative to the 8-byte selection header.
// Advancing that cursor by two words advances one logical equipment slot.
static inline effect_layout::ResolvedPart* EquipmentEntry(void* headerCursor) {
    return (effect_layout::ResolvedPart*)((char*)headerCursor + 8);
}
// Access the borrowed record word directly when two mutation branches join before storing it.
static inline void*& EquipmentRecord(void* headerCursor) {
    return *(void**)((char*)headerCursor + 12);
}

enum { kVariantRepairKind00 = 0, kVariantRepairKind02 = 2 };

// Clear covered {key, borrowed-record} slots, advancing the caller's existing scan cursors.
// Stops once no mask bits remain at/above the current slot; it does not release IFF records.
static inline void ClearEquipmentCoverage(unsigned char* base, unsigned int coverage,
        unsigned int*& slotCursor, int& slotIndex) {
    for (slotIndex = 0; ((ItemSelection801B7688*)base)->totalCount > slotIndex; slotIndex++) {
        if (!(coverage & (unsigned int)(-1 << slotIndex))) break;
        if (coverage & (1u << slotIndex)) {
            EquipmentEntry(slotCursor)->key = 0;
            EquipmentEntry(slotCursor)->record = 0;
        }
        slotCursor += 2;
    }
}

// Equip a part, resolve overlapping hide masks, and substitute compatible part variants.
// Requires a well-formed resolved selection with totalCount <= 24 and a valid encoded slot.
// Equipping the same key toggles a primary slot to default or removes an extra slot.
// Returns 1 after repair, 0 on unresolved/conflicting parts; earlier writes are retained.
// Required self is a 0xC8-byte ResolvedSelection with
// 0<=primaryCount<=totalCount<=24; every nonzero key in inspected slots requires
// a valid borrowed sPart record. key's slot bits ((key>>13)&31) must be <24
// BEFORE the initial key lookup (not validated here). hideMask bits describe
// occupied logical slots, not bytes; provider records must agree with the counts.
// Record lookups borrow inventory storage; this operation never deletes records.
// A returned 0 may follow partial overlap clearing/default replacement: callers
// must not assume rollback. Allocation ownership and model loading are unchanged.
int ResolvedItemSelection_EquipPart(void* self, unsigned int key) {
    int requestedKey = key;
    int slotIndex;
    unsigned char* selectionBytes = (unsigned char*)self;
    unsigned char partSlot = (unsigned char)((key >> kPartSlotShift) & kPartSlotMask);
    unsigned int requestedSlotByteOffset = 8 * partSlot;

    int toggleOffExtra = 0;
    if (key == *(unsigned int*)(selectionBytes + requestedSlotByteOffset + 8)) {
        if (partSlot < ((ItemSelection801B7688*)selectionBytes)->primaryCount) requestedKey = (*(unsigned int*)selectionBytes << kCharacterKeyShift) | (partSlot << kPartSlotShift) | kDefaultPartKeyBits;
        if (partSlot >= ((ItemSelection801B7688*)selectionBytes)->primaryCount) toggleOffExtra = 1;
    }
    int lookupKey = requestedKey;
    IFF_STRUCT::sPart* newPart = (IFF_STRUCT::sPart*)ItemInventory_FindPartData(g_sharedDoc->itemInventory, lookupKey); // original +0xA04
    if (0 == newPart) return 0;
    int repairSlotIndex;

    // Decision table, evaluated against the selection as it is being mutated:
    // same key / primary slot -> request that slot's default instead
    // same key / extra slot -> clear overlaps, skip requested-entry reinsertion
    // no overlap              -> retain equipped entry
    // entire old coverage hit -> clear every logical slot in the overlap
    // partial hit, kind 0/2   -> try alternate key; if absent, repair surviving old slots with defaults
    // partial hit, other kind -> no alternate repair; clear overlap only
    // default conflicts with new coverage -> return 0 immediately (earlier changes stay)
    // Then optionally insert requested part, try its full default, fill uncovered primary slots.
    // Coverage bits identify logical slots; each entry is {u32 key, borrowed sPart*}.
    // Phase 1: remove overlapping entries, or substitute their alternate/default variants.
    // hideMask is occupied body-slot coverage here, not an allow-list.
    unsigned int unprocessedNewCoverage = newPart->hideMask;
    unsigned int* equippedCursor = (unsigned int*)selectionBytes;
    for (slotIndex = 0; ((ItemSelection801B7688*)selectionBytes)->totalCount > slotIndex; slotIndex++) {
        unsigned int equippedKey = EquipmentEntry(equippedCursor)->key;
        if (equippedKey) {
            unsigned int overlap = newPart->hideMask & ((IFF_STRUCT::sPart*)EquipmentEntry(equippedCursor)->record)->hideMask;
            if (overlap) {
                unsigned int savedOverlap = overlap;
                unprocessedNewCoverage &= ~savedOverlap;
                if (overlap == ((IFF_STRUCT::sPart*)EquipmentEntry(equippedCursor)->record)->hideMask) {
                    unsigned int* slotCursor = (unsigned int*)selectionBytes;
                    ClearEquipmentCoverage(selectionBytes, overlap, slotCursor, repairSlotIndex);
                } else {
                    switch ((equippedKey >> kPartKindShift) & kPartKindMask) {
                    case kVariantRepairKind00:
                    case kVariantRepairKind02: {
                            unsigned int candidateKey = equippedKey | kPartVariantBit;
                            IFF_STRUCT::sPart* candidatePart = (IFF_STRUCT::sPart*)ItemInventory_FindPartData(g_sharedDoc->itemInventory, candidateKey); // original +0xA04
                            if (candidatePart) {
                                unsigned int* slotCursor = (unsigned int*)selectionBytes;
                                unsigned int variantCoverage = candidatePart->hideMask;
                                ClearEquipmentCoverage(selectionBytes, variantCoverage, slotCursor, repairSlotIndex);
                                for (repairSlotIndex = 0; ((ItemSelection801B7688*)selectionBytes)->totalCount > repairSlotIndex; repairSlotIndex++) {
                                    if (variantCoverage & (1u << repairSlotIndex)) {
                                        unsigned char* entryBytes = selectionBytes + 8;
                                        unsigned int* variantEntry = (unsigned int*)(entryBytes + repairSlotIndex * 8);
                                        unsigned int variantKeyForStore = candidateKey;
                                        ((effect_layout::ResolvedPart*)variantEntry)->key = variantKeyForStore;
                                        ((effect_layout::ResolvedPart*)variantEntry)->record = candidatePart;
                                        break;
                                    }
                                }
                            } else {
                                unsigned int* slotCursor = (unsigned int*)selectionBytes;
                                unsigned int uncoveredOldSlots = ((IFF_STRUCT::sPart*)EquipmentEntry(equippedCursor)->record)->hideMask & (newPart->hideMask ^ ((IFF_STRUCT::sPart*)EquipmentEntry(equippedCursor)->record)->hideMask);
                                for (repairSlotIndex = 0; ((ItemSelection801B7688*)selectionBytes)->totalCount > repairSlotIndex; repairSlotIndex++) {
                                    if (!(uncoveredOldSlots & (unsigned int)(-1 << repairSlotIndex))) break;
                                    if (uncoveredOldSlots & (1u << repairSlotIndex)) {
                                        int defaultSlotIndex;
                                        int repairIndexForKey = repairSlotIndex;
                                        unsigned int defaultKey = (*(unsigned int*)selectionBytes << kCharacterKeyShift) | ((unsigned int)repairIndexForKey << kPartSlotShift) | kDefaultPartKeyBits;
                                        IFF_STRUCT::sPart* added = (IFF_STRUCT::sPart*)ItemInventory_FindPartData(g_sharedDoc->itemInventory, defaultKey); // original +0xA04
                                        if (added) {
                                            unsigned int defaultCoverage = added->hideMask;
                                            if (defaultCoverage & newPart->hideMask) return 0;
                                            unsigned int* defaultSlotCursor = (unsigned int*)selectionBytes;
                                            ClearEquipmentCoverage(selectionBytes, defaultCoverage, defaultSlotCursor, defaultSlotIndex);
                                            EquipmentEntry(slotCursor)->key = defaultKey;
                                        } else {
                                            EquipmentEntry(slotCursor)->key = 0;
                                        }
                                        EquipmentRecord(slotCursor) = added;
                                    }
                                    slotCursor += 2;
                                }
                            }
                            break;
                        }
                    default:
                        break;
                    }
                    {
                        unsigned int* slotCursor = (unsigned int*)selectionBytes;
                        ClearEquipmentCoverage(selectionBytes, overlap, slotCursor, repairSlotIndex);
                    }
                }
            }
        }
        if (!unprocessedNewCoverage) break;
        equippedCursor += 2;
    }
    // Phase 2: install the requested entry (unless toggled off) and collect occupied slots.
    if (0 == toggleOffExtra) {
        unsigned int keySlotByteOffset = requestedSlotByteOffset;
        *(unsigned int*)(keySlotByteOffset + selectionBytes + 8) = requestedKey;
        unsigned int recordSlotByteOffset = requestedSlotByteOffset;
        ((void**)((char*)self + recordSlotByteOffset))[3] = newPart;
    }
    unsigned int* slotPtr = (unsigned int*)selectionBytes;
    int occupiedSlots = 0;
    for (slotIndex = 0; slotIndex < ((ItemSelection801B7688*)selectionBytes)->totalCount; slotIndex++) {
        void* value = (void*)EquipmentEntry(slotPtr)->record;
        if (value) occupiedSlots |= ((IFF_STRUCT::sPart*)value)->hideMask;
        slotPtr = slotPtr + 2;
    }
    if (!toggleOffExtra) {
        unsigned char savedType = partSlot;
        unsigned int newKey = (*(unsigned int*)selectionBytes << kCharacterKeyShift) | (savedType << kPartSlotShift) | kFullDefaultPartKeyBits;
        IFF_STRUCT::sPart* added = (IFF_STRUCT::sPart*)ItemInventory_FindPartData(g_sharedDoc->itemInventory, newKey); // original +0xA04
        if (added) { if (!(added->hideMask & occupiedSlots)) {

                for (repairSlotIndex = 0; repairSlotIndex < ((ItemSelection801B7688*)selectionBytes)->primaryCount; repairSlotIndex++) {
                    if ((added->hideMask) & (1u << repairSlotIndex)) break;
                }
                if (!*(unsigned int*)(8 + selectionBytes + 8 * repairSlotIndex)) {
                    unsigned char* primaryKeyBytes = selectionBytes + 8;
                    int primaryIndexForStore = repairSlotIndex;
                    *(unsigned int*)(primaryKeyBytes + 8 * repairSlotIndex) = newKey;
                    unsigned char* primaryRecordBytes = 12 + selectionBytes;
                    *(void**)(primaryRecordBytes + primaryIndexForStore * 8) = added;
                }
            } }
    }
    // Phase 3: fill uncovered primary slots with nonconflicting defaults.
    slotPtr = (unsigned int*)selectionBytes;
    for (slotIndex = 0; slotIndex < ((ItemSelection801B7688*)selectionBytes)->primaryCount; slotIndex++) {
        if (!(occupiedSlots & (1u << slotIndex))) {
            int primaryIndexForKey = slotIndex;
            unsigned int newKey = (*(unsigned int*)selectionBytes << kCharacterKeyShift) | ((unsigned int)primaryIndexForKey << kPartSlotShift) | kDefaultPartKeyBits;
            unsigned int savedNewKey = newKey;
            IFF_STRUCT::sPart* added = (IFF_STRUCT::sPart*)ItemInventory_FindPartData(g_sharedDoc->itemInventory, savedNewKey); // original +0xA04
            if (added) { if (!(added->hideMask & occupiedSlots)) {
                    EquipmentEntry(slotPtr)->key = newKey;
                    EquipmentEntry(slotPtr)->record = added;
                } }
        }
        slotPtr = slotPtr + 2;
    }
    return 1;
}

struct Pair8 { unsigned int a; unsigned int b; };
// Fill a range of two-word records without changing the compiler's expanded store order.
static inline void Pair8_fill(Pair8* words, Pair8* end, unsigned int v) {
	for (; words < end; words++) { words->a = v; words->b = v; }
}

#pragma opt_common_subs off
#pragma optimize_for_size on
// Clear the player-model state, including resource pointers, vectors, and selection storage.
// Initialization only: clears ownership pointers without releasing them. Destroy/release a
// live model before reuse. Returns self; expanded pair fills preserve the 0x2A0 ABI layout.
void* PlayerModel_ClearState(void* self) {
	// Several cleared regions have no confirmed field layout yet.
	// Preserve the expanded integer-pointer overflow guards and store order.
	// Clear starts at selection.parts[1] (+0x1CC), ends at animationState (+0x284).
	// clearValueOrUnrolledLimit first carries zero, then self+0x244 for the block
	// loop. This reuse retains its compiler lifetime; no address is stored as data.
	unsigned int pairsEndAddress, clearValueOrUnrolledLimit, pairCursorAddress, pairsStartAddress, pairRangeBytes, roundedPairRangeBytes, pairLimitPlusOne;
	int pairIndexLimit;
	unsigned int enabledWord;
	// Phase 1: zero ownership/header words and selection header; enable default state.
	pairCursorAddress = (unsigned int)self + 0x1cc;
	pairsEndAddress = (unsigned int)self + 0x284;
	clearValueOrUnrolledLimit = 0;
	enabledWord = 1;
	(*(unsigned int*)&((PlayerModelStorage*)self)->resourceWords[0]) = clearValueOrUnrolledLimit;
	(*(unsigned int*)&((PlayerModelStorage*)self)->resourceWords[1]) = clearValueOrUnrolledLimit;
	(*(unsigned int*)&((PlayerModelStorage*)self)->resourceWords[2]) = clearValueOrUnrolledLimit;
	(*(unsigned int*)&((PlayerModelStorage*)self)->resourceWords[3]) = clearValueOrUnrolledLimit;
	(*(unsigned int*)&((PlayerModelStorage*)self)->resourceWords[4]) = clearValueOrUnrolledLimit;
	(*(unsigned int*)&((PlayerModelStorage*)self)->resourceWords[5]) = clearValueOrUnrolledLimit;
	(*(unsigned int*)&((PlayerModelStorage*)self)->resourceWords[6]) = clearValueOrUnrolledLimit;
	*(unsigned int*)((char*)&((PlayerModelStorage*)self)->name) = clearValueOrUnrolledLimit;
	((PlayerModelStorage*)self)->name.size = clearValueOrUnrolledLimit;
	(*(unsigned int*)&((PlayerModelStorage*)self)->name.capacity) = clearValueOrUnrolledLimit;
	(*(unsigned int*)&((PlayerModelStorage*)self)->modelWords[0]) = clearValueOrUnrolledLimit;
	(*(unsigned int*)&((PlayerModelStorage*)self)->modelWords[1]) = clearValueOrUnrolledLimit;
	(*(unsigned int*)&((PlayerModelStorage*)self)->modelWords[2]) = clearValueOrUnrolledLimit;
	(*(unsigned int*)&((PlayerModelStorage*)self)->defaultEnabled) = enabledWord;
	((unsigned char*)self)[0x1b8] = (unsigned char)enabledWord;
	((unsigned char*)self)[0x1b9] = (unsigned char)clearValueOrUnrolledLimit;
	((unsigned char*)self)[0x1ba] = (unsigned char)clearValueOrUnrolledLimit;
	(*(unsigned int*)&((PlayerModelStorage*)self)->selection.character) = clearValueOrUnrolledLimit;
	((unsigned char*)self)[0x1c0] = (unsigned char)clearValueOrUnrolledLimit;
	((unsigned char*)self)[0x1c1] = (unsigned char)clearValueOrUnrolledLimit;
	((unsigned char*)self)[0x1c2] = (unsigned char)clearValueOrUnrolledLimit;
	((unsigned char*)self)[0x1c3] = (unsigned char)clearValueOrUnrolledLimit;
	(*(unsigned int*)&((PlayerModelStorage*)self)->selection.parts[0].key) = clearValueOrUnrolledLimit;
	(*(unsigned int*)&((PlayerModelStorage*)self)->selection.parts[0].record) = clearValueOrUnrolledLimit;
	// Phase 2: clear remaining key/record pairs using unrolled eight-pair stores and a tail.
	if (pairCursorAddress < pairsEndAddress) {
		pairsStartAddress = (unsigned int)self + 0x1cc;
		clearValueOrUnrolledLimit = (unsigned int)self + 0x244;
		pairRangeBytes = pairsEndAddress - pairsStartAddress;
		roundedPairRangeBytes = pairRangeBytes + 7;
		pairIndexLimit = (int)roundedPairRangeBytes / 8;
		pairLimitPlusOne = (unsigned int)pairIndexLimit + 1;
		if ((int)pairLimitPlusOne > 8) {
			unsigned int canUseUnrolledStores = 0;
			unsigned int byteRangeHasNoOverflow = 0;
			if (pairsStartAddress <= pairsEndAddress) {
				unsigned int compatibleByteRangeSigns = 1;
				if ((pairRangeBytes & 0x80000000) == 0) {
					if ((roundedPairRangeBytes & 0x80000000) != 0) compatibleByteRangeSigns = 0;
				}
				if (compatibleByteRangeSigns) byteRangeHasNoOverflow = 1;
			}
			if (byteRangeHasNoOverflow) {
				int pairCountSign = pairIndexLimit & 0x80000000;
				unsigned int compatiblePairCountSigns = 1;
				if (pairCountSign == 0) {
					if (pairCountSign != (int)(pairLimitPlusOne & 0x80000000)) compatiblePairCountSigns = 0;
				}
				if (compatiblePairCountSigns) canUseUnrolledStores = 1;
			}
			if (canUseUnrolledStores) {
				unsigned int zero = 0;
				{
					for (; pairCursorAddress < clearValueOrUnrolledLimit; pairCursorAddress += 0x40) {
						*(unsigned int*)(pairCursorAddress + 0x00) = zero;
						*(unsigned int*)(pairCursorAddress + 0x04) = zero;
						*(unsigned int*)(pairCursorAddress + 0x08) = zero;
						*(unsigned int*)(pairCursorAddress + 0x0c) = zero;
						*(unsigned int*)(pairCursorAddress + 0x10) = zero;
						*(unsigned int*)(pairCursorAddress + 0x14) = zero;
						*(unsigned int*)(pairCursorAddress + 0x18) = zero;
						*(unsigned int*)(pairCursorAddress + 0x1c) = zero;
						*(unsigned int*)(pairCursorAddress + 0x20) = zero;
						*(unsigned int*)(pairCursorAddress + 0x24) = zero;
						*(unsigned int*)(pairCursorAddress + 0x28) = zero;
						*(unsigned int*)(pairCursorAddress + 0x2c) = zero;
						*(unsigned int*)(pairCursorAddress + 0x30) = zero;
						*(unsigned int*)(pairCursorAddress + 0x34) = zero;
						*(unsigned int*)(pairCursorAddress + 0x38) = zero;
						*(unsigned int*)(pairCursorAddress + 0x3c) = zero;
					}
				}
			}
		}
		unsigned int tailEndAddress = (unsigned int)self + 0x284;
		unsigned int zeroWord = 0;
		{
			for (; pairCursorAddress < tailEndAddress; pairCursorAddress += 8) {
				*(unsigned int*)(pairCursorAddress + 0) = zeroWord;
				*(unsigned int*)(pairCursorAddress + 4) = zeroWord;
			}
		}
	}
	// Phase 3: reset texture, part-resource, and animation state regions.
	Vec2_Zero(((PlayerModelStorage*)self)->animationPosition);
	memset(((PlayerModelStorage*)self)->textureState, 0, sizeof(((PlayerModelStorage*)self)->textureState));
	memset(((PlayerModelStorage*)self)->partResourceState, 0, sizeof(((PlayerModelStorage*)self)->partResourceState));
	memset(((PlayerModelStorage*)self)->animationState, 0, sizeof(((PlayerModelStorage*)self)->animationState));
	return self;
}
#pragma opt_common_subs reset
#pragma optimize_for_size reset

// Return whether the resolved equipment selection contains the requested part key.
int ResolvedItemSelection_ContainsPart(void* self, unsigned int value) {
	unsigned char count = ((ItemSelection801B7688*)self)->totalCount;
	unsigned char i;
	for (i = 0; i < count; i++) {
		unsigned int* entry;
		if (i >= count) {
			entry = 0;
		} else {
			entry = (unsigned int*)((char*)self + (unsigned int)i * 8);
			entry = (unsigned int*)((char*)entry + 8);
		}
		if (value == *entry)
			return 1;
	}
	return 0;
}

// Free the owned Unicode buffer and optionally delete its wrapper.
// The MSLPointerVector type models scalar cleanup only: this storage is not an
// array of retained object pointers. Positive shouldDelete frees the wrapper;
// return may be dangling. It must not destroy individual Unicode code units.
void* dtor_801B4E14(MSLPointerVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

// Release all player-model resources and its owned Unicode buffer; optionally delete the object.
void* PlayerModel_Dtor(void* self, int shouldDelete) {
	if (self != 0) {
		PlayerModel_ReleaseResourcesByMask(self, 7);
		if ((unsigned int)self + 0x1c) {
			if ((unsigned int)self + 0x1c) {
				if ((unsigned int)self + 0x1c) {
					if ((unsigned int)self + 0x1c) {
						if ((unsigned int)self + 0x1c) {
							void* member = ((PlayerModelStorage*)self)->name.data;
							if (member != 0) {
								((PlayerModelStorage*)self)->name.size -= ((char*)&((PlayerModelStorage*)self)->name + ((PlayerModelStorage*)self)->name.size) - ((char*)&((PlayerModelStorage*)self)->name);
								appFreeUni__FPv(member);
							}
						}
					}
				}
			}
		}
		if (shouldDelete > 0)
			__dl__FPv(self);
	}
	return self;
}

#pragma use_lmw_stmw on
#pragma opt_propagation off
// Reconcile dependent subparts with equipped parts and their overlapping body-slot masks.
// Two passes: remove orphan/conflicting dependent parts, then ensure parent-requested subparts.
// Returns 0 on a failed repair with prior mutations preserved; IFF pointers remain borrowed.
// Entry: self is a resolved 0xC8-byte selection, NOT the 0x68-byte ID-only
// ItemSelection. Header +00 u32 character ID; +04 u8 primaryCount, +05 u8
// totalCount, +06/+07 variants; +08 contains 24 {u32 key,sPart* record} pairs.
// Require 0<=primaryCount<=totalCount<=24 and all used key slot bits
// ((key>>13)&31)<totalCount, agreeing with their pair index. A nonzero key must
// have a live matching borrowed IFF sPart; null record denotes an empty pair.
// Every dependency ID must encode a slot <24, because EquipPart indexes BEFORE
// looking up the record. Inventory/default records must outlive the whole call.
// Caller ItemSelection_CopyValidatedChanges equips then invokes this repair (above).
// Mutation is IN PLACE: EquipPart can clear/replace several other pairs and fill
// defaults. Pass 1 reloads totalCount, pass 2 reexamines current parent records
// after each repair; neither pass snapshots the selection or owns IFF records.
// Despite dependency traversal this is not recursive: EquipPart does not call
// ReconcileSubparts. Two slots per parent are visited in array order, with no
// general dependency-graph traversal/cycle detection or fixed-point guarantee.
// Return 1 means both passes ran; missing optional dependency lookup is skipped.
// Return 0 stops on an EquipPart conflict/missing required record, retaining all
// earlier mutations. Callers needing rollback must snapshot BEFORE equipping.
int ResolvedItemSelection_ReconcileSubparts(void* self) {
    unsigned char* base;
    int index;
    int cleared;
    unsigned int* equippedCursor;
    unsigned int* dependencyCursor;
    unsigned int key;
    unsigned int dependencyPart;
    int j;
    int count;
    int subPartIndex;
    int excludedSlot;
    unsigned int equippedHideMask;
    int j2;
    unsigned int* overlapCursor;
    unsigned int overlapPart;
    unsigned int candidateHideMask;
    int j3;
    unsigned int* candidateCursor;
    unsigned int candidatePart;
    base = (unsigned char*)self;
    index = 0;
    cleared = 0;
    equippedCursor = (unsigned int*)self;
    for (; index < (count = ((ItemSelection801B7688*)base)->totalCount); equippedCursor += 2, index++) {
        key = equippedCursor[2];
        if (!key || ((key >> kPartKindShift) & kPartKindMask) != 1) continue;
        j = 0;
        dependencyCursor = (unsigned int*)self;
        for (; j < count; dependencyCursor += 2, j++) {
            dependencyPart = dependencyCursor[3];
            if (!dependencyPart || index == j) continue;
            subPartIndex = 0;
            if (key != ((IFF_STRUCT::sPart*)dependencyPart)->subPartIds[0]) {
                subPartIndex = 1;
                if (key != ((IFF_STRUCT::sPart*)dependencyPart)->subPartIds[1]) subPartIndex = 2;
            }
            if (subPartIndex < 2) break;
        }
        if (j >= count) {
            if (!ResolvedItemSelection_EquipPart(self, key)) return 0;
            continue;
        }
        if (index < ((ItemSelection801B7688*)base)->primaryCount || !equippedCursor[3]) continue;
        equippedHideMask = ((IFF_STRUCT::sPart*)equippedCursor[3])->hideMask;
        excludedSlot = (unsigned char)index;
        j2 = 0;
        overlapCursor = (unsigned int*)self;
        int none = SelectionHasNoMaskConflict(overlapCursor, j2, overlapPart, count, excludedSlot, equippedHideMask);
        if (!none) { equippedCursor[2] = cleared; equippedCursor[3] = cleared; }
    }
    // Pass 2: each surviving parent may request up to two subordinate part IDs.
    unsigned int* parentCursor = (unsigned int*)self;
    int subPartByteOffset;
    for (int n = 0; n < ((ItemSelection801B7688*)base)->totalCount; parentCursor += 2, n++) {
        if (!parentCursor[3]) continue;
        int k = 0;
        subPartByteOffset = 0;
        for (; k < 2; k++, subPartByteOffset += 4) {
            unsigned int value = *(unsigned int*)((char*)parentCursor[3] + IFF_OFFSET(IFF_STRUCT::sPart, subPartIds) + subPartByteOffset);
            if (!value) continue;
            unsigned int type = (value >> kPartSlotShift) & kPartSlotMask;
            if (type < ((ItemSelection801B7688*)base)->primaryCount) {
                unsigned int* ref = (unsigned int*)(base + (((unsigned char)type) << 3));
                if (((ref[2] >> kPartKindShift) & kPartKindMask) == 2 && !ResolvedItemSelection_EquipPart(self, value)) return 0;
            } else {
                IFF_STRUCT::sPart* object = (IFF_STRUCT::sPart*)ItemInventory_FindPartData(g_sharedDoc->itemInventory, value); // original +0xA04
                if (!object) continue;
                candidateHideMask = object->hideMask;
                candidateCursor = (unsigned int*)self;
                j3 = 0;
                int valid = SelectionAllowsMask(candidateCursor, j3, candidatePart, base, candidateHideMask);
                if (valid && !ResolvedItemSelection_EquipPart(self, *(unsigned int*)((char*)parentCursor[3] + IFF_OFFSET(IFF_STRUCT::sPart, subPartIds) + subPartByteOffset))) return 0;
            }
        }
    }
    return 1;
}
#pragma opt_propagation reset
#pragma use_lmw_stmw reset

// Optionally free an otherwise empty equipment-selection object.
void* ResolvedItemSelection_Destroy(void* self, int shouldDelete) {
	if (self != 0 && shouldDelete > 0)
		__dl__FPv(self);
	return self;
}

// Empty particle-record constructor or virtual hook; performs no work.
void CShotParticle_vfunc_7() {}

// Empty particle-record constructor or virtual hook; performs no work.
void CShotParticle_vfunc_6() {}

// Copy the launch axis used by subsequent fragment resets.
void CFragment_SetNormal(void* self, void* source) {
	__as__7WVectorFRC7WVector(&((FragmentAxis*)self)->launchAxis, source);
}

} // extern "C"

} // namespace ns4

// ---- merged from course_ball_shot_effects.cpp ----
namespace ns5 {
// Translation unit 0x801B215C-0x801B3830 (18 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Empty particle-record constructor or virtual hook; performs no work.
void EnergyCenterParticle_CtorNoInit() {}
// Empty particle-record constructor or virtual hook; performs no work.
void ShotEffects_NoOpHook_1B2A98(void*) {}
// Empty particle-record constructor or virtual hook; performs no work.
void ShotEffects_NoOpHook_1B362C(void*) {}

struct StateObject {
    unsigned char reserved00[0x14];
    void* bitmapResource;
    unsigned char active;
    unsigned char reserved19[3];
    void* sprite;
    void* auxiliarySprite;
    float emissionElapsed;
    unsigned char reserved28[0x04];
    void* reservedPointer2C;
    void* reservedPointer30;
    void* reservedPointer34;
    void* reservedPointer38;
    void* reservedPointer3C;
    unsigned char reserved44[0xE4];
    void* particles;
    unsigned int particleCapacity;
    unsigned int particleIndex;
    float spriteFrame;
    float opacity;
};

extern "C" void* __nw__FUl(unsigned int);
extern "C" void* __nwa__FUl(unsigned int);
extern "C" void W3dAniSpr_Ctor(void*);
extern "C" void W3dAniSpr_LoadTexture(void*, void*, float, float);
extern "C" void* __construct_new_array(void*, void (*)(void*), void*, unsigned int, unsigned int);
extern "C" unsigned int g_resourceManager;
extern "C" char __vt__9CStarDust[];
extern "C" char __vt__10CStarDust2[];
extern "C" char __vt__13CEnergyCenter[];
extern "C" const char courseEffectStrings[];
extern "C" char s_particleVtable[];
extern "C" char s_cameraTransform[];
extern "C" const float kFloat_32_SpriteGrid;
extern "C" const float kFloat_0_0099999998_ShotEffects; // 0.01: dust spawn interval / distance-to-scale factor
extern "C" const float kFloat_0_1_Particle;
extern "C" const float kFloat_0_Particles;
extern "C" const float kFloat_neg1_ShotEffects; // -1: reverse-vector scale / vertical-velocity threshold
extern "C" const float kFloat_0_0049999999_ShotEffects; // 0.005: fade-to-emission scale / coin velocity jitter
extern "C" const float kFloat_9_9999997eneg06_ShotEffects; // 0.00001: near-zero threshold
extern "C" const float kFloat_127_ShotEffects; // 127: first dust effect opacity fade rate
extern "C" const float kFloat_18_849556_ShotEffects; // 6*pi: dust sprite-frame rate
extern "C" const double kDouble_SignedIntConversionBias_Particles;
extern "C" const float kFloat_1_5707964_ShotEffects; // pi/2: orbit spin base / center frame rate
extern "C" const float kFloat_0_0009765625_805DF488;
extern "C" const float kFloat_0_001953125_ShotEffects; // 1/512: signed random-axis scale
extern "C" const float kFloat_6_2831855_ShotEffects; // 2*pi: full-turn angle
extern "C" const float kFloat_255_SpriteAlpha;
extern "C" const float kFloat_2_0;
extern "C" const float kFloat_0_3_805DF4A4;
extern "C" const float kFloat_1_805DF43C;
extern "C" const float kFloat_0_5_Particle;
extern "C" const float kFloat_0_25_805DF468;
extern "C" const float kFloat_0_6_ShotEffects;
extern "C" unsigned int g_taskManager;
extern "C" unsigned int g_view;
extern "C" void* Object_GetChild();
extern "C" void* TaskActorTable_FindByName(void*, const char*);
extern "C" void W3dAniSpr_SetColor(void*, unsigned int);
extern "C" void W3dAniSpr_SetPosition(void*, const void*);
extern "C" void W3dAniSpr_SetSizeAndPivot(void*, float, float, float, float);
extern "C" void W3dAniSpr_DrawFrame(void*, unsigned int, float, unsigned int, int);
extern "C" void* GolfBall__Fv();
extern "C" void* PartyItemSlots_GetInstance();
extern "C" int PartyItemSlots_GetItemType(void*, int);
extern "C" void* NamedEffectList_FindByName(void*, const char*);
extern "C" void WPuppet_SetOpacityRecursive(void*, unsigned char);
extern "C" CSharedDoc* g_sharedDoc;
extern "C" void* g_sceneManager;
extern "C" float Math_SinApprox(float);
extern "C" float Math_CosApprox(float);
extern "C" void __ml__FfRC7WVector(void*, const void*, float);
extern "C" void __ml__FRC7WVectorf(void*, const void*, float);
extern "C" void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern "C" void __as__7WVectorFRC7WVector(void*, const void*);
extern "C" void Vector_CrossProduct(void*, const void*, const void*);
extern "C" void Matrix_TransformPoint(void*, const void*, const void*);
extern "C" void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern "C" void __apl__7WVectorFRC7WVector(void*, const void*);
extern "C" void Matrix_SetAxisRotation(void*, float, signed char);
extern "C" void* Normalize__7WVectorFv(void*);
extern "C" void* Normalize__7WVectorFv(void*);
extern "C" float Magnitude__7WVectorCFv(void*);
extern "C" void Vec3_DivideScalar(void*, float, const void*);
extern "C" unsigned int rand();
extern "C" const float kFloat_35_ShotEffects, kFloat_0_050000001_ShotEffects, kFloat_8_ShotEffects, kFloat_1000_ShotEffects, kFloat_20_ShotEffects, kFloat_400_ShotEffects, kFloat_250_ShotEffects, kFloat_160_ShotEffects; // 35: energy-particle emission rate; 0.05: spawn velocity jitter scale; 8: energy-particle spawn offset bias; 1000: inward-particle opacity rise rate; 400: center opacity fade rate after all particles arrive; 250: center auxiliary fade rate; 160: center auxiliary fade floor
extern "C" void __dla__FPv(void*);
extern "C" void __dl__FPv(void*);

typedef void (*StateDtor)(void*, int);
}
struct AllocatedState {
    unsigned char bytes[0x19C];
    // Construct an animated sprite in its allocation wrapper.
    AllocatedState() { W3dAniSpr_Ctor(this); }
};
struct VDel { 
virtual void Del(int);
};
extern "C" {

typedef Vec3f Vec3;

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
// Emit inward-moving particles, accumulate center opacity as they arrive, then fade the center.
// Positive particle capacity is required for spawn-angle modulo and per-arrival alpha.
// Emission loops until index >= 35 * dt: fractional positive limits still emit one particle.
void CEnergyCenter_UpdateEmitAndParticles(StateObject* self, float dt) {
    if (!AsCEnergyCenter(self)->active) return;
    float spawnLimit = kFloat_35_ShotEffects * dt;
    int particleIndex = 0;
    while (AsCEnergyCenter(self)->spawnedParticleCount < AsCEnergyCenter(self)->particleCapacity && (float)particleIndex < spawnLimit) {
        int spawnIndex = AsCEnergyCenter(self)->spawnedParticleCount;
        Energy* particle = (Energy*)((char*)AsCEnergyCenter(self)->particles + spawnIndex * 0x20);
        int r = (int)rand();
        float y = kFloat_0_050000001_ShotEffects * (float)(r % 31 - 15) + kFloat_8_ShotEffects;
        r = (int)rand();
        float x = kFloat_0_050000001_ShotEffects * (float)(r % 31 - 15) + kFloat_8_ShotEffects;
        Vec3 spawnOffset, rot, tmp, mat;
        float spawnRotation[12];
        spawnOffset.x = x; spawnOffset.y = y; spawnOffset.z = 0.0f;
        int capacity = AsCEnergyCenter(self)->particleCapacity;
        r = (int)rand();
        float spawnAngle = kFloat_6_2831855_ShotEffects * (float)capacity / (float)(r % capacity + 1);
        Matrix_SetAxisRotation(spawnRotation, spawnAngle, 1);
        Matrix_TransformPoint(&tmp, &spawnOffset, spawnRotation);
        __pl__FRC7WVectorRC7WVector(&mat, &((CenterPosition*)self)->center, &tmp);
        __as__7WVectorFRC7WVector(&particle->position, &mat);
        __mi__FRC7WVectorRC7WVector(&tmp, &((CenterPosition*)self)->center, &particle->position);
        __as__7WVectorFRC7WVector(&particle->direction, &tmp);
        __ml__FfRC7WVector(&rot, Normalize__7WVectorFv(&particle->direction), kFloat_1_805DF43C);
        __as__7WVectorFRC7WVector(&particle->direction, &rot);
        particle->opacity = 0.0f;
        particle->active = 1;
        ++particleIndex;
        ++AsCEnergyCenter(self)->spawnedParticleCount;
    }
    // Phase 2: move emitted particles toward center, rotate their relative offsets, and
    // count a crossing of the center Y plane as an arrival. Direction is normalized again.
    int allParticlesArrived = 1;
    for (particleIndex = 0; particleIndex < AsCEnergyCenter(self)->spawnedParticleCount; ++particleIndex) {
        Energy* particle = (Energy*)((unsigned char*)AsCEnergyCenter(self)->particles + particleIndex * 0x20);
        if (!particle->active) continue;
        float opacity = kFloat_1000_ShotEffects * dt + particle->opacity;
        if (opacity < kFloat_255_SpriteAlpha) particle->opacity = opacity;
        else particle->opacity = kFloat_255_SpriteAlpha;
        // velocity = 20*direction; position += velocity*dt; then rotate (position-center) about Y.
        // Scratch vectors are reused only after their previous value has been consumed.
        Vec3 velocityOrWorldPosition,stepOrDirectionToCenter,unusedVector,offsetFromCenter,rotatedOffset;
        float orbitRotation[12];
        __ml__FRC7WVectorf(&velocityOrWorldPosition, &particle->direction, kFloat_20_ShotEffects);
        __ml__FRC7WVectorf(&stepOrDirectionToCenter, &velocityOrWorldPosition, dt);
        __apl__7WVectorFRC7WVector(&particle->position, &stepOrDirectionToCenter);
        Matrix_SetAxisRotation(orbitRotation, AsCEnergyCenter(self)->rotationAngle, 1);
        __mi__FRC7WVectorRC7WVector(&offsetFromCenter, &particle->position, &((CenterPosition*)self)->center);
        Matrix_TransformPoint(&rotatedOffset, &offsetFromCenter, orbitRotation);
        __pl__FRC7WVectorRC7WVector(&velocityOrWorldPosition, &((CenterPosition*)self)->center, &rotatedOffset);
        __as__7WVectorFRC7WVector(&particle->position, &velocityOrWorldPosition);
        if (particle->position.y * particle->direction.y >= AsCEnergyCenter(self)->centerY * particle->direction.y) {
            AsCEnergyCenter(self)->centerOpacity += kFloat_255_SpriteAlpha / (float)AsCEnergyCenter(self)->particleCapacity;
            particle->active = 0;
        } else allParticlesArrived = 0;
        __mi__FRC7WVectorRC7WVector(&stepOrDirectionToCenter, &((CenterPosition*)self)->center, &particle->position);
        __as__7WVectorFRC7WVector(&particle->direction, &stepOrDirectionToCenter);
        __as__7WVectorFRC7WVector(&particle->direction, Normalize__7WVectorFv(&particle->direction));
    }
    // Phase 3: animate the center; fade it only when no emitted particle remains en route.
    AsCEnergyCenter(self)->centerFrame -= kFloat_1_5707964_ShotEffects * dt;
    if (allParticlesArrived) {
        float v = AsCEnergyCenter(self)->centerOpacity - kFloat_400_ShotEffects * dt;
        AsCEnergyCenter(self)->centerOpacity = v;
        if (v < 0.0f) AsCEnergyCenter(self)->active = 0;
    } else {
        float v = AsCEnergyCenter(self)->unk_148 - kFloat_250_ShotEffects * dt;
        if (v > kFloat_160_ShotEffects) AsCEnergyCenter(self)->unk_148 = v;
        else AsCEnergyCenter(self)->unk_148 = kFloat_160_ShotEffects;
    }
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset

// Initialize the next orbiting dust particle around the ball and camera basis.
// Cross world ball velocity with camera basis[6], then jitter both orbit axes.
// The cross product must be nonzero for normalization; no fallback axis is supplied.
void CStarDust_ResetParticle(StateObject* self) {
    typedef void (*ScaleFn)(void*, const void*, float);
    Vec3 orbitBase, scaledOrbitBase, doubledOrbitBase, cosineAxis, randomCosineOffset, cosineJitter, sineAxis, normalizedOrbitBase, randomSineOffset, sineJitter;
    void* ball = GolfBall__Fv();
    Vector_CrossProduct(&orbitBase, &((Ball*)ball)->velocity, (s_cameraTransform + IFF_OFFSET(CameraTransform800D, basis[6])));
    float ballScale = ((Ball*)GolfBall__Fv())->effectScale;
    ((ScaleFn)__ml__FRC7WVectorf)(&doubledOrbitBase, Normalize__7WVectorFv(&orbitBase), kFloat_2_0);
    ((ScaleFn)__ml__FRC7WVectorf)(&scaledOrbitBase, &doubledOrbitBase, ballScale);
    __as__7WVectorFRC7WVector(&orbitBase, &scaledOrbitBase);
    int particleIndex = AsCStarDust(self)->particleIndex;
    Orbit* particle = OrbitAt(AsCStarDust(self)->particles, particleIndex);
    particle->active = 1;
    ((StarDustParticle*)particle)->radialScale = ((Ball*)GolfBall__Fv())->effectScale;
    int randomValue = (int)rand();
    ((StarDustParticle*)particle)->angularVelocity = kFloat_1_5707964_ShotEffects * (float)(randomValue % 1023) / 1024.0f + kFloat_1_5707964_ShotEffects;
    float a = (float)((int)rand() % 1024 - 512) / 512.0f;
    float b = (float)((int)rand() % 1024 - 512) / 512.0f;
    float c = (float)((int)rand() % 1024 - 512) / 512.0f;
    cosineJitter.y = b; cosineJitter.z = a; cosineJitter.x = c;
    ((ScaleFn)__ml__FfRC7WVector)(&randomCosineOffset, &cosineJitter, kFloat_2_0);
    __pl__FRC7WVectorRC7WVector(&cosineAxis, (s_cameraTransform + IFF_OFFSET(CameraTransform800D, basis[6])), &randomCosineOffset);
    __as__7WVectorFRC7WVector(&particle->cosineAxis, &cosineAxis);
    a = (float)((int)rand() % 1024 - 512) / 512.0f;
    b = (float)((int)rand() % 1024 - 512) / 512.0f;
    c = (float)((int)rand() % 1024 - 512) / 512.0f;
    sineJitter.y = b; sineJitter.z = a; sineJitter.x = c;
    ((ScaleFn)__ml__FfRC7WVector)(&randomSineOffset, &sineJitter, kFloat_2_0);
    float magnitude = Magnitude__7WVectorCFv(&orbitBase);
    Vec3_DivideScalar(&normalizedOrbitBase, magnitude, &orbitBase);
    __pl__FRC7WVectorRC7WVector(&sineAxis, &normalizedOrbitBase, &randomSineOffset);
    __as__7WVectorFRC7WVector(&particle->sineAxis, &sineAxis);
    __as__7WVectorFRC7WVector(&particle->origin, GolfBall__Fv());
    randomValue = (int)rand();
    ((StarDustParticle*)particle)->angle = kFloat_6_2831855_ShotEffects * (float)(randomValue % 1023) / 1024.0f;
    ((StarDustParticle*)particle)->sizeFactor = kFloat_1_805DF43C;
}

// animation/update state
// Fade dust on descent, restore ball opacity, and catch up particle emissions after long frames.
extern "C" void CStarDust_Update(StateObject* self, float dt) {
    if (!self->active) return;
    void* ball = GolfBall__Fv();
    if (((Ball*)ball)->velocity.y < kFloat_neg1_ShotEffects) {
        float emissionScale = kFloat_0_0049999999_ShotEffects * self->opacity;
        float emissionStep = emissionScale * dt;
        emissionStep = (dt > emissionStep) ? emissionStep : dt;
        self->emissionElapsed = self->emissionElapsed + emissionStep;
        float remainingOpacity = self->opacity - kFloat_127_ShotEffects * dt;
        remainingOpacity = (kFloat_0_Particles < remainingOpacity) ? remainingOpacity : kFloat_0_Particles;
        self->opacity = remainingOpacity;
        void* manager = g_sharedDoc;
        // 0x801B3108: the party item keeps its full opacity.
        if ((*(int*)&((CSharedDoc*)manager)->partyGameMode) == kPartyItemMode &&
        PartyItemSlots_GetItemType(PartyItemSlots_GetInstance(), AsCGolfDoc((*(void**)&((CSharedDoc*)manager)->golfDoc))->currentPlayer) == kOpaqueBallPartyItem) {
            WPuppet_SetOpacityRecursive(NamedEffectList_FindByName(g_sceneManager,(courseEffectStrings + IFF_OFFSET(CourseEffectStringPool, ballObject))), kOpaqueAlpha);
        } else {
            WPuppet_SetOpacityRecursive(NamedEffectList_FindByName(g_sceneManager,(courseEffectStrings + IFF_OFFSET(CourseEffectStringPool, ballObject))), kOpaqueAlpha - (int)self->opacity);
        }
        {
            float opacityMagnitude;
            if (((*(unsigned int*)&((CStarDust*)self)->alpha)) & kFloatSignBit) opacityMagnitude = -self->opacity;
            else opacityMagnitude = self->opacity;
            unsigned int belowThreshold = opacityMagnitude < kFloat_9_9999997eneg06_ShotEffects;
            if (belowThreshold) {
                self->opacity = kFloat_0_Particles;
                self->active = 0;
                WPuppet_SetOpacityRecursive(NamedEffectList_FindByName(g_sceneManager,(courseEffectStrings + IFF_OFFSET(CourseEffectStringPool, ballObject))), kOpaqueAlpha);
            }
        }
    } else {
        self->emissionElapsed = self->emissionElapsed + dt;
    }
    typedef void (*Update)(void*, int, float);
    for (int i = 0; i < (*(int*)&((CStarDust*)self)->particleCount); ++i) {
        ((Update)(*(void***)self)[9])(self, i, dt);
    }
    // Emit pairs from the ring, replaying missed intervals and aging each new pair to now.
    if (self->emissionElapsed > AsCStarDust(self)->spawnInterval) {
        int cycles = dt > AsCStarDust(self)->spawnInterval ? (int)(self->emissionElapsed / AsCStarDust(self)->spawnInterval) : 1;
        self->emissionElapsed = self->emissionElapsed - AsCStarDust(self)->spawnInterval * cycles;
        for (; 0 < cycles; --cycles) {
            for (int j = 0; j < 2; ++j) {
                typedef void (*Reset)(void*);
                ((Reset)(*(void***)self)[8])(self);
                if (dt > AsCStarDust(self)->spawnInterval) {
                    float catchUpDelta = AsCStarDust(self)->spawnInterval * (float)cycles + self->emissionElapsed;
                    ((Update)(*(void***)self)[9])(self, self->particleIndex, catchUpDelta);
                }
                self->particleIndex = ((int)self->particleIndex + 1) % ((*(int*)&((CStarDust*)self)->particleCount));
            }
        }
    }
    self->spriteFrame = self->spriteFrame - kFloat_18_849556_ShotEffects * dt;
}

struct Elem20 {
    unsigned char d[0x20];
    // Invoke the no-op energy-particle record constructor.
    Elem20() { EnergyCenterParticle_CtorNoInit(); }
    /* Allocate wrapper storage through the existing engine allocator. */
    static void* operator new[](size_t n) { return ARRAY_NEW(n); }
    /* Release wrapper storage through the matching engine allocator. */
    static void operator delete[](void* p) { __dla__FPv(p); }
};
// Allocate a fixed-capacity inward-particle effect at the supplied center and rotation angle.
// self/src required; src is three world-position floats copied now, value is
// rotation about world Y in radians. Source-backed admission: the sole factory,
// CParticleEffect_HandleMsg case kSpawnEnergyColumn (particle_effect_emitters.cpp), passes count=30.
// Thus the observed admitted domain is {30}; no wider gameplay range is proven.
// A new caller must use positive count with 32*count plus the array cookie
// representable in 32-bit allocation size, and successful allocation; no tested
// gameplay capacity range beyond 30 is established. Zero/negative/overflowing
// counts are not rejected here and are not supported inputs. Update divides by
// capacity on particle arrival; do not interpret count as a frame duration.
// Elem20 construction is a no-op. Only spawnedParticleCount entries may be read:
// ResetEffect clears that count before activation, and vfunc_1 initializes each
// entry before incrementing it. The pool binds an already registered bitmap
// before ResetEffect; its named bitmap lookup must succeed.
// Array allocation has no local recovery/status. If it throws, self is incomplete
// (particles has not been published); do not call the complete-object destructor
// or draw. If a null result is published, later emission dereferences it: catching
// a failure does not make this effect drawable. Completed teardown uses
// CEnergyCenter_dtor/FreeArrayBlock, which understands the array cookie.
StateObject* CEnergyCenter_Ctor(StateObject* self, int count, void* src, float value) {
    StateObject* base = self;
    int n;
    void* mem;
    self->bitmapResource = 0;
    self->active = 0;
    self->sprite = 0;
    self->auxiliarySprite = 0;
    AsCEnergyCenter(self)->vertex0 = ((Base*)self)->vertices[0];
    AsCEnergyCenter(self)->vertex1 = ((Base*)self)->vertices[1];
    AsCEnergyCenter(self)->vertex2 = ((Base*)self)->vertices[3];
    AsCEnergyCenter(self)->vertex3 = ((Base*)self)->vertices[2];
    AsCEnergyCenter(self)->vertexListTerminator = 0;
    *(char**)self = __vt__13CEnergyCenter;
    AsCEnergyCenter(self)->rotationAngle = value;
    AsCEnergyCenter(self)->particleCapacity = count;
    *(WVector*)(&((CenterPosition*)self)->center) = *(WVector*)src;
    AsCEnergyCenter(base)->centerOpacity = kFloat_1_805DF43C;
    AsCEnergyCenter(base)->centerFrame = kFloat_0_Particles;
    AsCEnergyCenter(base)->unk_148 = kFloat_255_SpriteAlpha;
    AsCEnergyCenter(base)->unk_14C = kFloat_255_SpriteAlpha;
    unsigned int a = g_taskManager ? (unsigned int)((void* (*)(unsigned int))Object_GetChild)(g_taskManager) : 0;
    AsCEnergyCenter(base)->golfBackgroundActor = TaskActorTable_FindByName((void*)a, ((CourseEffectStringPool*)courseEffectStrings)->golfBackgroundActor);
    n = AsCEnergyCenter(base)->particleCapacity;
    mem = new Elem20[n];
    AsCEnergyCenter(base)->particles = mem;
    return base;
}

// Draw the animated center sprite and every live inward-moving particle.
void CEnergyCenter_DrawCenterAndParticles(StateObject* self) {
    if (!self->active) return;
    float alpha = AsCEnergyCenter(self)->centerOpacity;
    W3dAniSpr_SetColor(self->sprite, ((int)alpha << 24) | kWhiteRgb);
    W3dAniSpr_SetPosition(self->sprite, &((CenterPosition*)self)->center);
    W3dAniSpr_SetSizeAndPivot(self->sprite, kFloat_1_805DF43C, kFloat_1_805DF43C, kFloat_0_5_Particle, kFloat_0_5_Particle);
    W3dAniSpr_DrawFrame(self->sprite, g_view, AsCEnergyCenter(self)->centerFrame, 0x00F00000, 4);
    for (int i = 0; i < AsCEnergyCenter(self)->spawnedParticleCount; ++i) {
        Energy* p = (Energy*)((char*)AsCEnergyCenter(self)->particles + i * 0x20);
        if (p->active) {
            W3dAniSpr_SetColor(self->sprite, ((int)((EnergyCenterParticle*)p)->opacity << 24) | kWhiteRgb);
            W3dAniSpr_SetPosition(self->sprite, &p->position);
            W3dAniSpr_SetSizeAndPivot(self->sprite, kFloat_0_5_Particle, kFloat_0_5_Particle, kFloat_0_25_805DF468, kFloat_0_25_805DF468);
            W3dAniSpr_DrawFrame(self->sprite, g_view, kFloat_0_Particles, 0x20C00000, 6);
        }
    }
}

// Draw the ball glow and active dust sprites using the effect opacity and per-particle scale.
void CStarDust_DrawBallGlowAndParticles(StateObject* self) {
    if (!self->active) return;
    unsigned int color = ((int)AsCStarDust(self)->alpha << 24) | kWhiteRgb;
    W3dAniSpr_SetSizeAndPivot(self->sprite, kFloat_1_805DF43C, kFloat_1_805DF43C, kFloat_0_5_Particle, kFloat_0_5_Particle);
    W3dAniSpr_SetColor(self->sprite, color);
    W3dAniSpr_SetPosition(self->sprite, GolfBall__Fv());
    W3dAniSpr_DrawFrame(self->sprite, g_view, self->spriteFrame, 0x20C00000, 4);
    for (int i = 0; i < (*(int*)&AsCStarDust(self)->particleCount); ++i) {
        Orbit* p = OrbitAt(AsCStarDust(self)->particles, i);
        if (p->active) {
            float scale = kFloat_0_6_ShotEffects * ((StarDustParticle*)p)->sizeFactor;
            W3dAniSpr_SetPosition(self->sprite, &p->position);
            W3dAniSpr_SetSizeAndPivot(self->sprite, scale, scale, kFloat_0_5_Particle * scale, kFloat_0_5_Particle * scale);
            W3dAniSpr_SetColor(self->sprite, color);
            W3dAniSpr_DrawFrame(self->sprite, g_view, kFloat_0_Particles, 0x20C00000, 5);
        }
    }
}

// Release owned particles, sprites, and buffers; optionally delete the effect object.
StateObject* CEnergyCenter_dtor(StateObject* self, int flags) {
    if (self) {
        void* array = AsCEnergyCenter(self)->particles;
        *(char**)self = __vt__13CEnergyCenter;
        if (array) {
            if (array) FreeArrayBlock(array);
            AsCEnergyCenter(self)->particles = 0;
        }
        if (self->sprite) {
            if (self->sprite) {
                ((VDel*)self->sprite)->Del(1);
            }
            self->sprite = 0;
        }
        AsCEnergyCenter(self)->particleCapacity = 0;
        if (self) {
            void* obj2 = self->auxiliarySprite;
            *(char**)self = s_particleVtable;
            if (obj2) {
                if (obj2) {
                    ((VDel*)obj2)->Del(1);
                }
                self->auxiliarySprite = 0;
            }
            void* obj = self->sprite;
            if (obj) {
                if (obj) {
                    ((VDel*)obj)->Del(1);
                }
                self->sprite = 0;
            }
        }
        if (flags > 0) __dl__FPv(self);
    }
    return self;
}

// Release owned particles, sprites, and buffers; optionally delete the effect object.
StateObject* CStarDust_dtor(StateObject* self, int flags) {
    if (self) {
        void* array = AsCStarDust(self)->particles;
        *(char**)self = __vt__9CStarDust;
        if (array) {
            if (array) FreeArrayBlock(array);
            AsCStarDust(self)->particles = 0;
        }
        if (self->sprite) {
            if (self->sprite) {
                ((VDel*)self->sprite)->Del(1);
            }
            self->sprite = 0;
        }
        (*(void**)&AsCStarDust(self)->particleCount) = 0;
        if (self) {
            void* obj2 = self->auxiliarySprite;
            *(char**)self = s_particleVtable;
            if (obj2) {
                if (obj2) {
                    ((VDel*)obj2)->Del(1);
                }
                self->auxiliarySprite = 0;
            }
            void* obj = self->sprite;
            if (obj) {
                if (obj) {
                    ((VDel*)obj)->Del(1);
                }
                self->sprite = 0;
            }
        }
        if (flags > 0) __dl__FPv(self);
    }
    return self;
}

// Release owned particles, sprites, and buffers; optionally delete the effect object.
StateObject* CStarDust2_dtor(StateObject* self, int flags) {
    if (self) {
        void* array = AsCStarDust2(self)->particles;
        *(char**)self = __vt__10CStarDust2;
        if (array) {
            if (array) FreeArrayBlock(array);
            AsCStarDust2(self)->particles = 0;
        }
        if (self->sprite) {
            if (self->sprite) {
                ((VDel*)self->sprite)->Del(1);
            }
            self->sprite = 0;
        }
        (*(void**)&AsCStarDust2(self)->particleCount) = 0;
        if (self) {
            void* obj2 = self->auxiliarySprite;
            *(char**)self = s_particleVtable;
            if (obj2) {
                if (obj2) {
                    ((VDel*)obj2)->Del(1);
                }
                self->auxiliarySprite = 0;
            }
            void* obj = self->sprite;
            if (obj) {
                if (obj) {
                    ((VDel*)obj)->Del(1);
                }
                self->sprite = 0;
            }
        }
        if (flags > 0) __dl__FPv(self);
    }
    return self;
}

// Recreate the dust sprite grid and reset the emission cursor, opacity, and live flags.
void CStarDust_StartAndResetParticles(StateObject* self) {
    if (self->sprite) {
        void* old = self->sprite;
        if (old) ((VDel*)old)->Del(1);
        self->sprite = 0;
    }
    void* obj = new AllocatedState;
    self->sprite = obj;
    (*(void**)&((W3dAniSpr*)obj)->resourceManager) = (void*)g_resourceManager;
    W3dAniSpr_LoadTexture(self->sprite, self->bitmapResource, kFloat_32_SpriteGrid, kFloat_32_SpriteGrid);
    self->particleIndex = 0;
    self->opacity = kFloat_255_SpriteAlpha;
    AsCStarDust(self)->spawnElapsed = kFloat_0_Particles;
    self->spriteFrame = kFloat_0_Particles;
    self->active = 1;
    for (int i = 0; i < (*(int*)&AsCStarDust(self)->particleCount); ++i)
        *((unsigned char*)AsCStarDust(self)->particles + i * kOrbitRecordBytes) = 0; // first byte of an orbit record is its active flag
}

// Recreate the second dust sprite grid and reset its emission cursor and live flags.
void CStarDust2_BeginEffect(StateObject* self) {
    if (self->sprite) {
        void* old = self->sprite;
        if (old) ((VDel*)old)->Del(1);
        self->sprite = 0;
    }
    void* obj = new AllocatedState;
    self->sprite = obj;
    (*(void**)&((W3dAniSpr*)obj)->resourceManager) = (void*)g_resourceManager;
    W3dAniSpr_LoadTexture(self->sprite, self->bitmapResource, kFloat_32_SpriteGrid, kFloat_32_SpriteGrid);
    self->particleIndex = 0;
    self->opacity = kFloat_255_SpriteAlpha;
    AsCStarDust2(self)->emissionElapsed = kFloat_0_Particles;
    self->spriteFrame = kFloat_0_Particles;
    self->active = 1;
    for (int i = 0; i < AsCStarDust2(self)->particleCount; ++i)
        *((unsigned char*)AsCStarDust2(self)->particles + i * kOrbitRecordBytes) = 0; // first byte of an orbit record is its active flag
}

// Advance one dust particle's orbit, radius, spin, and size until it expires.
// index must be within particleCount. Angular velocity changes before angle; radius grows
// while the separate fade/size factor falls. The stored origin does not follow the current ball.
void CStarDust_UpdateParticle(StateObject* self, int index, float dt) {
    float position[3];
    float scaledOffset[3];
    float orbitOffset[3];
    Orbit* particle = OrbitAt(AsCStarDust(self)->particles, index);
    float cosineOffset[3];
    if (!particle->active) return;
    particle->angularVelocity = particle->angularVelocity - kFloat_2_0 * dt;
    particle->angle -= particle->angularVelocity * dt;

    float sineOffset[3];
    float sine = Math_SinApprox(particle->angle);
    __ml__FfRC7WVector(sineOffset, &particle->sineAxis, sine);
    float cosine = Math_CosApprox(particle->angle);
    __ml__FfRC7WVector(cosineOffset, &particle->cosineAxis, cosine);
     __pl__FRC7WVectorRC7WVector(orbitOffset, cosineOffset, sineOffset);
    __ml__FfRC7WVector(scaledOffset, (const void*)orbitOffset, particle->radialScale);
     __pl__FRC7WVectorRC7WVector(position, (const void*)(&particle->origin), scaledOffset);
    __as__7WVectorFRC7WVector((void*)(&particle->position), position);
    particle->radialScale = particle->radialScale + kFloat_2_0 * dt;
    particle->fade = particle->fade - kFloat_0_3_805DF4A4 * dt;
    if (particle->fade < kFloat_0_Particles) particle->active = 0;
}

struct PartCtor {
    unsigned char d[0x50];
    // Leave orbit-particle bytes uninitialized; the reset/emission paths fill them.
    PartCtor() {}
};
extern void CParticleBase_dtor(void*, int);
// Allocate four hundred orbiting dust records and set the emission interval.
StateObject* CStarDust_Ctor(StateObject* self) {
    int count = kStarDustCapacity;
    self->bitmapResource = 0;
    self->active = 0;
    self->sprite = 0;
    self->auxiliarySprite = 0;
    AsCStarDust(self)->vertex0 = ((Base*)self)->vertices[0];
    AsCStarDust(self)->vertex1 = ((Base*)self)->vertices[1];
    AsCStarDust(self)->vertex2 = ((Base*)self)->vertices[3];
    AsCStarDust(self)->vertex3 = ((Base*)self)->vertices[2];
    AsCStarDust(self)->vertexListTerminator = 0;
    *(char**)self = __vt__9CStarDust;
    AsCStarDust(self)->spawnInterval = kFloat_0_0099999998_ShotEffects;
    AsCStarDust(self)->particleCount = count;
    AsCStarDust(self)->particles = new PartCtor[AsCStarDust(self)->particleCount];
    return self;
}

// Allocate sixty orbiting dust records, preserving base cleanup if allocation throws.
StateObject* CStarDust2_Ctor(StateObject* self) {
    int count = kStarDust2Capacity;
    int flag = 0;
    self->bitmapResource = 0;
    self->active = 0;
    self->sprite = 0;
    self->auxiliarySprite = 0;
    AsCStarDust2(self)->vertex0 = ((Base*)self)->vertices[0];
    AsCStarDust2(self)->vertex1 = ((Base*)self)->vertices[1];
    AsCStarDust2(self)->vertex2 = ((Base*)self)->vertices[3];
    AsCStarDust2(self)->vertex3 = ((Base*)self)->vertices[2];
    AsCStarDust2(self)->vertexListTerminator = 0;
    *(char**)self = __vt__10CStarDust2;
    AsCStarDust2(self)->emissionInterval = kFloat_0_1_Particle;
    (*(unsigned int*)&AsCStarDust2(self)->particleCount) = count;
    try {
        void* mem = new PartCtor[count];
        AsCStarDust2(self)->particles = mem;
    } catch (...) {
        CParticleBase_dtor(self, -1);
        throw;
    }
    return self;
}

// Activate the center effect, reset its spawn count, and recreate its sprite grid.
// Requires constructed self, initialized resource manager and a valid loaded
// bitmapResource with nonzero dimensions. Grid cells are 32x32 texels; fractions
// are truncated by W3dAniSpr_LoadTexture (engine_startup_and_ani_sprite.cpp). It borrows the
// bitmap handle: the effect/resource owner must keep it alive through the last
// draw and until sprite destruction. AddBitmapGridFrames does not retain it.
// Replacement phases: publish particleIndex=0/active=1; destroy old sprite via
// deleting virtual destructor(1); clear pointer; construct new 0x19C sprite;
// publish pointer; bind manager; append frames. There is NO transaction/rollback.
// If allocation/construction throws, old sprite is already gone, pointer stays
// null and active stays 1. A null allocation result is not handled before the
// manager write. If frame allocation throws, self retains the partially filled
// sprite for later teardown. There is no failure status and drawing must not
// resume after an intercepted failure without restoring valid sprite state.
// CEnergyCenter_dtor deletes particles then sprite, without releasing this
// borrowed bitmap. Reset does not clear existing particle records or replace
// auxiliarySprite. Renderer submission uses manager texture/palette resources;
// teardown must follow the normal update/draw sequence, not race queued draws.
void CEnergyCenter_ResetEffect(StateObject* self) {
    self->particleIndex = 0;
    self->active = 1;
    if (self->sprite != 0) {
        void* old = self->sprite;
        if (old != 0) {
            ((VDel*)old)->Del(1);
        }
        self->sprite = 0;
    }
    void* obj = new AllocatedState;
    self->sprite = obj;
    (*(void**)&((W3dAniSpr*)obj)->resourceManager) = (void*)g_resourceManager;
    W3dAniSpr_LoadTexture(self->sprite, self->bitmapResource, kFloat_32_SpriteGrid, kFloat_32_SpriteGrid);
}

} // extern "C"

} // namespace ns5

extern "C" { void ParticleEffects_InitFullTurnAngle(); }
/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462178", _ctors_u801AC5B0_0, ParticleEffects_InitFullTurnAngle);
#pragma pop
