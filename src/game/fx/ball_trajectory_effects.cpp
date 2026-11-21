#include <decomp/static_init.h>
#include <decomp/local_class.h>
// Ball trajectory presentation: comet/ribbon/skid/wind/ring/putt effects and ground marker.
// This is visual history consumption, not golf physics or the image-present clock.
// Frame rate lookup: CComet_UpdateTrail removes .5 stored edges PER CALL while
// sampleElapsed uses elapsed seconds. Commit at most one edge per call after a
// STRICT sampleInterval crossing; it does not catch up with a loop. At rest of
// flight, CBallTrack uses 1/30 seconds per HISTORY SAMPLE, not per rendered frame.
// Keep those clocks distinct when changing presentation rate or history cadence.
// Modder index (definitions in u80201D04_data.cpp unless noted):
// - comet length/sampling: kTrajectorySharedHalf (.5 edge/call),
//   kFloat_0_1_GeneTex3D (.1 s/edge), kCometInitialTrailCapacity (20 edges).
//   Capacity is also the INLINE 20*24-byte ring at +118..+2F8 in the 0x324-byte
//   CComet allocation: raising just the integer overwrites later state.
// - comet fade: kCometBrightnessDecayRate=150 alpha/s, initial kFullOpacity255,
//   kCometTimeStepBrightnessScale=.005/alpha, stop epsilon ~1e-5 alpha units.
// - ball ribbon: kBallTrackSecondsPerHistorySample=1/30 s, fade clamp [.5,2] s,
//   kHistorySampleSpacing=5 samples, near-position cutoff .32000002265 world units,
//   kBallRibbonRadiusMultiplier=1.2; boundaryMarker forces otherwise skipped samples.
// - ring effect: OnLoad count=10, spawn interval shares .1 s with comet sampling;
//   shrink=.8 size/s, fade=255 alpha/s, at most one spawn/call with remainder kept.
// - wind streaks: 256 slots; camera-motion gate=1 world unit/call, cull=160 world
//   units from BALL, opacity distance=200 from CAMERA, max alpha=128, spawn jitter
//   +/-75, mean length/forward offset=100, length jitter +/-30 world units.
// - putt ribbon: radius scale=.45, age ramp=90..15 HISTORY samples behind playback,
//   ordinary alpha=64; course/live-ground override stores 256 (not a byte alpha).
//   CPuttLine_Process maps aim-state admission, history motion tags and ground
//   material codes separately; sample+0x70 is NOT a terrain classification.
// - resources: BallTrackStringPool paths select bitmaps; OnLoad owns the seven
//   effects/marker. activeEffect[0..4] borrow those owners, never own new instances.
// - model visibility: HandleMsg restores the ball via WPuppet opacity; the
//   ground marker uses 15-world-unit X/Z basis vectors, not viewport pixels.
// Shared .5/.1/2/255 constants have several roles: introduce an independent future
// setting for one role instead of retuning all their consumers accidentally.
// World vectors use Y up; camera right/up/forward/position is an affine basis.
// No aspect ratio, resolution or scissor setting lives here. Draw routines install
// GX state and borrowed camera/bitmap bindings without restoring every prior state.
// Required initialized self/ball/camera/history unless a function says otherwise;
// history indices must name allocated samples. Start/reset follows load, and
// OnDestroy releases owners after callbacks stop. Positive destructor flags free
// storage; returned self may be freed. See class headers and local ABI views below.
// No locks are taken; calls assume serialized actor/render lifecycle access.
// Retuning ranges are unestablished except the storage/index bounds stated here;
// increasing spatial scales widens/lengthens effects, increasing decay shortens
// persistence, and increasing sampling intervals reduces emission at fixed dt.
#include "game/golfball.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CSharedDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CGolfDoc.h"
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
#include "sdk_protos.h"
#include "game/wvector.h"
#include "game/types.h"
#include "classes/CComet.h"
#include "classes/CBallTrack.h"
#include "classes/CTrajectory.h"
#include "classes/CBallWindTrack.h"
#include "classes/CDoughnut.h"
#include "classes/CPuttLine.h"
#include "classes/CSkidMark.h"
#include "classes/CBallTrace.h"

#define AsCBallTrace(p) ((CBallTrace*)(p))

#define AsCSkidMark(p) ((CSkidMark*)(p))

#define AsCPuttLine(p) ((CPuttLine*)(p))

#define AsCDoughnut(p) ((CDoughnut*)(p))

#define AsCBallWindTrack(p) ((CBallWindTrack*)(p))

#define AsCTrajectory(p) ((CTrajectory*)(p))

#define AsCBallTrack(p) ((CBallTrack*)(p))

#define AsCComet(p) ((CComet*)(p))

// Packed Shift-JIS/ASCII pool; array extents include each terminator.
struct BallTrackStringPool {
    char trajectoryActor[5], circleBitmap[19], ribbonBitmap[17], skidBitmap[16];
    char cobraBitmap[15], groundBitmap[11], ballActor[7], cometBitmap[19];
};
enum BallTrackStringOffsets {
    kCircleBitmap = (int)&((BallTrackStringPool*)0)->circleBitmap,
    kRibbonBitmap = (int)&((BallTrackStringPool*)0)->ribbonBitmap,
    kSkidBitmap = (int)&((BallTrackStringPool*)0)->skidBitmap,
    kCobraBitmap = (int)&((BallTrackStringPool*)0)->cobraBitmap,
    kGroundBitmap = (int)&((BallTrackStringPool*)0)->groundBitmap,
    kBallActorName = (int)&((BallTrackStringPool*)0)->ballActor,
    kCometBitmap = (int)&((BallTrackStringPool*)0)->cometBitmap
};

// Physical sample prefix used by ribbon width and putt visibility calculations.
struct BallHistorySample {
    WVector position; WVector velocity; char reserved18[0x48];
    unsigned char boundaryMarker; char reserved61[0x0f]; int motionState;
};
struct GroundMarkerStorage {
    unsigned char active; char reserved01[3]; void* bitmap; WVector position;
};
struct TrajectoryMessage { unsigned recipient; int effectType; };
// Only Reset bypasses free-slot selection. RestoreBall/GroundMarker also need a free slot,
// even though neither stores one; both are ignored when all five effect slots are occupied.
enum TrajectoryEffectRequest {
    kRequestComet = 0, kRequestBallRibbon = 1, kRequestSkid = 2, kRequestWind = 3,
    kRequestRings = 4, kRequestPuttRibbon = 5, kRequestReset = 6,
    kRequestRestoreBall = 7, kRequestCobraSkid = 8, kRequestGroundMarker = 9
};
// Observed roles in m_eventPreDataIndex, not inferred physical event names: 0 = playback
// sample, 1 = ring-stop/fade gate, 1/2 bracket the wind midpoint, 7 = skid origin, 10 = cobra end.
// All are history sample indices, not seconds. Slot 1 is a first-contact/flight
// transition marker, NOT a universal landing time. ResetShotCollisionAndSamples
// (golf_rule_round_flow_and_shot_sim.cpp) sets it to -1. CQuadTree simulation (course_collision_and_ball_physics.cpp) latches preDataCount
// only while it is negative: on raised-region top contact, on ground-following
// motion becoming airborne, or on terrain contact before bounce classification.
// The latter also clears curve/special-flight flags. It therefore identifies
// whichever of these events FIRST occurred; later contacts do not replace it.
// CBallTrack_InitTrailPoints uses it even when -1 to derive fade/lead time;
// CDoughnut_UpdateParticles disables emission when slot1>0; wind disables after
// playback>(slot1+slot2)/2 with integer truncation, also only when slot1>0.
// Slot2 is first airborne descending sample (state==1, velocity.y<0, golf_rule_round_flow_and_shot_sim.cpp).
// Event-at-sample-zero does not satisfy those >0 gates. Keep reset sentinel,
// latch order and each consumer's different test; none of these indices is dt.
enum TrajectoryHistoryIndex {
    kHistoryPlaybackSample = 0, kHistoryEffectGate1 = 1, kHistoryWindMidpointEnd2 = 2,
    kHistorySkidOrigin7 = 7, kHistoryCobraEnd10 = 10
};
enum TrajectoryOrderingKey { kGroundEffectOrder = 1, kAirEffectOrder = 6 };
// Cached state IDs/value domains verified by Engine_SetCachedRenderState in
// gx_renderer_state_and_efb_copy.cpp and Render_ApplyBlendState/DepthState in gx_render_state_and_texture_upload.cpp. These are engine
// IDs paired with GX factor/compare values, not byte offsets into an effect.
enum TrailRenderState {
    kTrailDepthCompareState = 3, kTrailDepthWriteState = 5,
    kTrailSourceBlendState = 13, kTrailDestinationBlendState = 14,
    kTrailBlendZero = 0, kTrailBlendOne = 1,
    kTrailBlendInverseSourceColor = 3, kTrailBlendSourceAlpha = 4,
    kTrailCompareLessEqual = 3, kTrailCompareAlways = 7,
    kTrailPresetRenderPairs = 15, kTrailPresetStagePairs = 6, kTrailPresetSamplerPairs = 5
};
enum { kSkidHistoryWindow = 45 }; // History samples, not 45 render calls.
struct EffectCameraMatrices { char reserved00[0x20]; float view[16], projection[16]; };
struct EffectCameraTransform { WVector right, up, forward, position; };
struct BallRibbonEdge { WVector left, right; };
struct WindTrailParticle {
    WVector start, end; int opacity; float length; unsigned char active; char reserved21[3];
};
struct DoughnutParticle {
    float size, opacity; unsigned char active; char reserved09[3];
    WVector right, up, forward, position;
};
struct SkidFrameRangeView { char reserved00[0x124]; int startFrame, endFrame; };
struct WindTrackOwnedStorage { char reserved00[0x230]; void* embeddedQuadRenderer; void* embeddedTrailPoints; };
// Wind ownership: outer particles +0x118 are WindTrailParticle[256]; embedded CBallTrack
// starts +0x11C and owns a renderer +0x230 and ribbon-edge array +0x234. Outer renderer
// +0x114 belongs to CBallTrace. Array-new pointers have a 16-byte cookie before element 0;
// raw renderer objects instead use their virtual deleting destructor.
// +0 is either alpha0..255 or selector256; display consumes it as a tagged
// color/opacity word, never as an unconditional byte alpha (skidmark_ball.cpp).
struct PuttRibbonSegment { int colorOrOpacity; WVector left, right; };
#define TRAIL_LAYOUT_OFF(T, f, o) typedef char T##_off_##f[((int)&((T*)0)->f == (o)) ? 1 : -1]
TRAIL_LAYOUT_OFF(BallHistorySample, velocity, 0xc);
TRAIL_LAYOUT_OFF(BallHistorySample, boundaryMarker, 0x60);
TRAIL_LAYOUT_OFF(BallHistorySample, motionState, 0x70);
TRAIL_LAYOUT_OFF(WindTrailParticle, opacity, 0x18);
TRAIL_LAYOUT_OFF(WindTrailParticle, active, 0x20);
TRAIL_LAYOUT_OFF(DoughnutParticle, right, 0xc);
TRAIL_LAYOUT_OFF(PuttRibbonSegment, left, 4);
TRAIL_LAYOUT_OFF(PuttRibbonSegment, right, 0x10);
typedef char WindParticle_size[(sizeof(WindTrailParticle) == 0x24) ? 1 : -1];
typedef char DoughnutParticle_size[(sizeof(DoughnutParticle) == 0x3c) ? 1 : -1];
typedef char PuttRibbonSegment_size[(sizeof(PuttRibbonSegment) == 0x1c) ? 1 : -1];
#undef TRAIL_LAYOUT_OFF

enum TrajectoryConstants {
    kPreviousCameraOffset = (int)&((CBallWindTrack*)0)->previousCameraPosition,
    kCameraMotionVectorOffset = (int)&((CBallWindTrack*)0)->cameraMotionVector,
    kActorMessageInterfaceOffset = 0x18,
    kSkidStartFrameOffset = (int)&((SkidFrameRangeView*)0)->startFrame,
    kSkidEndFrameOffset = (int)&((CSkidMark*)0)->endFrame,
    kCameraForwardOffset = (int)&((EffectCameraTransform*)0)->forward,
    kCameraPositionOffset = (int)&((EffectCameraTransform*)0)->position,
    kEmbeddedBallRibbonOffset = (int)&((CBallWindTrack*)0)->ballTrackVtable,
    kWindParticleActiveOffset = (int)&((WindTrailParticle*)0)->active,
    kWindParticleOpacityOffset = (int)&((WindTrailParticle*)0)->opacity,
    kWindParticleLengthOffset = (int)&((WindTrailParticle*)0)->length,
    kWindParticleEndOffset = (int)&((WindTrailParticle*)0)->end,
    kDoughnutSizeOffset = (int)&((DoughnutParticle*)0)->size,
    kDoughnutOpacityOffset = (int)&((DoughnutParticle*)0)->opacity,
    kDoughnutActiveOffset = (int)&((DoughnutParticle*)0)->active,
    kDoughnutRightOffset = (int)&((DoughnutParticle*)0)->right,
    kDoughnutUpOffset = (int)&((DoughnutParticle*)0)->up,
    kDoughnutPositionOffset = (int)&((DoughnutParticle*)0)->position,
    kActiveEffectSlots = 5, kQuadVertexCount = 4,
    kWindParticleCount = 256, kHistorySampleSpacing = 5,
    // IActor_Skipped tests global mask & actor.skipMask. Golf-task setup points
    // that mask at CGolfDoc::gameStateFlags (golf_task_loading_and_pause.cpp); it is not a render flag.
    // Components: 0x2|0x40|0x100|0x200|0x4000|0x8000|0x80000. Established
    // task names are button-shot animation (0x40) and begin-player-turn (0x4000);
    // remaining rule states retain numeric labels rather than invented meanings.
    kTrajectorySkipFlags = 0x8c342,
    kEffectViewTransform = 0, kEffectProjectionTransform = 1, kEffectTexture0Transform = 2, kEffectWorldTransform = 11,
    kQuadStripPrimitive = 0x98,
    kPuttFadeHistoryStart = 90, kPuttFadeHistoryEnd = 15,
    kPuttMaxOpacity = 0x40, kPuttSpecialSurfaceColorSelector = 0x100,
    kPuttSpecialCourse = 10,
    kPuttGroundType10 = 10, // Separate ground-type domain; gameplay surface name is unresolved.
    kCometInitialTrailCapacity = 20, // Must fit inline storage, 20*24 bytes; not a free-standing size knob.
    kRingParticleCount = 10, kArrayCookieBytes = 16,
    kRingParticleBytes = 0x3C, kRibbonEdgeBytes = 0x18,
    kPuttSegmentBytes = 0x1C, kOpaqueAlphaByte = 255,
    kOpaqueWhiteRgb = 0x00FFFFFF,
    kEffectSlotOffset = (int)&((CTrajectory*)0)->activeEffect0
};

struct GenePrefixReal {
    unsigned char count;
    char pad[3];
    // Initialize the generated renderer vertex-count byte to zero.
    GenePrefixReal() { count = 0; }
};
struct CGeneTex3D : GenePrefixReal {
    virtual void v08(); virtual void v0C(); virtual int resize(int);
    virtual void color(unsigned char, int, int, int, int);
    int state;
    // Initialize the generated textured renderer state word.
    CGeneTex3D() {
    state = 0; }
};

struct NativeBase80201D18 {
    char data[0x18];
    NativeBase80201D18();
    ~NativeBase80201D18();
};

struct NativeMember80201D18 {
    char data[0x14];
    // Preserve the nonthrowing empty secondary-interface member construction.
    NativeMember80201D18() __attribute__((nothrow)) {}
    ~NativeMember80201D18();
};

struct NativeOwner80201D18 : NativeBase80201D18, NativeMember80201D18 {
    char tail[0x24];
    NativeOwner80201D18();
};

// ---- merged from ball_trajectory_effects.cpp ----
namespace ns0 {
// Translation unit 0x80201D04-0x80203204 (14 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Virtual function map (symbol names kept as-is):
// VObj4.v2(deltaSeconds) updates one effect; VObj5.v3() draws one effect.
// VObjTop.a12() suppresses trajectory drawing when nonzero. Earlier slots are
// layout placeholders; their operation contracts are not established here.
struct VObj4 { 
virtual void v0(); virtual void v1(); virtual void v2(float);
};
struct VObjTop { 
virtual void a0(); virtual void a1(); virtual void a2(); virtual void a3(); virtual void a4(); virtual void a5(); virtual void a6(); virtual void a7(); virtual void a8(); virtual void a9(); virtual void a10(); virtual void a11(); virtual int a12();
};
struct VObj5 { 
virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
};
// Fourth vtable word initializes an existing effect synchronously; no retain occurs.
struct TrajectoryEffectVtable {
    void* precedingSlots[3];
    void (*initialize)(void*);
};
extern const float kFloat_6_2831855;
extern float s_twoPi_Trajectory;
extern const float kFloat_1_BallTrack;
extern const float kFloat_0_0_BallTrack;
extern void *g_sceneManager;
extern char ballTrackEffectStrings[];
extern char s_ballTraceBaseVtable[];
extern char __vt__11CTrajectory[];
extern char __vt__10CBallTrack[];
extern char kIdentityMatrix4x4[];
extern void *g_resourceManager;
extern const float kFloat_15_TrajectoryMarker;
extern const float kFloat_0_1_GeneTex3D;
extern char __vt__9CDoughnut[], __vt__9CPuttLine[], __vt__16ntSkidMark_Cobra[], __vt__14CBallWindTrack[], __vt__11CGenePoly3D[], __vt__9CSkidMark[];
extern char __vt__10CGeneTex3D[];

void CTitleTask_dtor(void*, int);
void __ct__18NativeBase80201D18Fv(void *);
void IActor_AddSkipFlags(void *, int);
void ResourceManager_ReleaseId(void *, int);
void __dla__FPv(void *);
void *CQuadTree_GetInstance();
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void *, void *, void *, int, int, int);
void CQuadTree_DrawProjectedTexturedPatch(void *, float, float, void *, void *, void *, void *, int, int);
void SetTransform(int, void *);
void ResourcePreloadQueue_AddRequest(void *, int, int, float, void *, float, char *, int, int, int);
void *GolfBall__Fv();
void __dl__FPv(void *);
void *PreData__9CGolfBallFi(void *, int);
void *__nwa__FUl(unsigned int);
void Vector_CrossProduct(void *, void *, void *);
void Normalize__7WVectorFv(void *);
void __amu__7WVectorFf(void *, float);
void __pl__FRC7WVectorRC7WVector(void *, void *, void *);
void __mi__FRC7WVectorRC7WVector(void *, void *, void *);
void __as__7WVectorFRC7WVector(void *, void *);
// CBallTrack_DefaultHook: public symbol spelling is retained for coordinator renaming.
void CBallTrack_DefaultHook();
extern float kBallTrackSecondsPerHistorySample, kTrajectorySharedHalf, kFloat_2_TrajectoryShared, kFullOpacity255, kBallRibbonRadiusMultiplier;
extern const double kDouble_SignedIntConversionBias_InventoryRows;
extern const float kFloat_0_32000002_BallTrack;

float WVectorLen__FRC7WVector(void *);
void Vec3_Zero(void *);
// NamedEffectList_GetRenderKind: public symbol spelling is retained for coordinator renaming.
int NamedEffectList_GetRenderKind(void *, char *);
// NamedEffectList_SetRenderKind: public symbol spelling is retained for coordinator renaming.
void NamedEffectList_SetRenderKind(void *, char *, int);
int NamedEffectList_FindByName(void *, char *);
void WPuppet_SetOpacityRecursive(int, int);
void *CComet_Ctor(void *);
void *ResourceManager_LoadBitmap(void *, char *, int, int, int);
// DoughnutParticle_DefaultConstruct: public symbol spelling is retained for coordinator renaming.
void DoughnutParticle_DefaultConstruct(void *);
// WindTrailParticle_DefaultConstruct: public symbol spelling is retained for coordinator renaming.
void WindTrailParticle_DefaultConstruct(void *);
void *__nw__FUl(unsigned int);

// Constructor adapter for the full comet object, including its base storage.
struct BuildFirst {
    char bytes[0x324];
    // Construct the owned complete comet object in this allocation.
    BuildFirst() { CComet_Ctor(this); }
};
// Quad-renderer adapters keep the count byte before the secondary virtual interface.
struct BuildControllerPad { unsigned char state; char pad[3]; };
struct BuildController : BuildControllerPad {
    virtual void c0(); virtual void c1();
    virtual unsigned char setState(int);
};
// Shared trace owner: bitmap, activation state and an owned quad renderer.
struct BuildBase {
    void *table;
    int field4;
    void *resource;
    int count;
    char pad10[8];
    float value;
    unsigned char active;
    char pad1d[0xf7];
    void *controller;
    // Clear common trace ownership pointers and activity without installing a derived vtable.
    BuildBase() __attribute__((nothrow)) {
        field4=0; resource=0; active=0; controller=0;
    }
    // Clear common trace fields and install the supplied derived vtable.
    BuildBase(char *t) __attribute__((nothrow)) {
        field4=0; resource=0; active=0; controller=0; table=t;
    }
    // Destroy the common trace renderer during normal teardown or constructor unwind.
    ~BuildBase() {
        table = s_ballTraceBaseVtable;
        if (controller) {
            void* p = controller;
            ((void (*)(void*, int))(*(void***)((char*)p + 4))[2])(p, 1);
            controller = 0;
        }
    }
    // Load one bitmap entry from the trajectory resource pool.
    void load(int offset) {
        resource=ResourceManager_LoadBitmap(g_resourceManager,ballTrackEffectStrings+offset,0,0,0);
    }
    // Allocate a quad renderer and request its four-vertex state.
    void control(char *table) {
        void *m=__nw__FUl(12);
        if(m) { *(char *)m=0; *(void **)((char *)m+4)=table; *(int *)((char *)m+8)=0; }
        controller=m;
        if(*(unsigned char *)m!=4) *(unsigned char *)m=((BuildController *)m)->setState(4);
    }
};
// Array element adapter for one shrinking ring particle.
struct BuildElement {
    // Allocate element arrays through the game allocator; CodeWarrior adds its array cookie.
    static void *operator new[](size_t size) { return __nwa__FUl(size); }
    char data[0x3c];
    // Preserve the ring-particle element construction hook; it writes no payload.
    BuildElement() { DoughnutParticle_DefaultConstruct(this); }
};
// Array element adapter for one wind streak.
struct BuildElement24 {
    // Allocate element arrays through the game allocator; CodeWarrior adds its array cookie.
    static void *operator new[](size_t size) { return __nwa__FUl(size); }
    char data[0x24];
    // Preserve the wind-particle element construction hook; it writes no payload.
    BuildElement24() { WindTrailParticle_DefaultConstruct(this); }
};
// Construct a ring effect with the requested particle count and circle bitmap.
struct BuildArray : BuildBase {
    void *array;
    // Allocate owned ring particles and acquire their bitmap and quad renderer.
    BuildArray(int size) {
        table=__vt__9CDoughnut; count=size;
        array=new BuildElement[count];
        load(kCircleBitmap); control(__vt__10CGeneTex3D); value=kFloat_0_1_GeneTex3D;
    }
};
// Construct and destroy the ball ribbon, including bitmap and history-array ownership.
struct Build134 : BuildBase {
    void *array;
    char tail[0x18];
    // Initialize an owned ball ribbon with no edge array yet.
    Build134() {
        table=__vt__10CBallTrack; array=0;
        load(kRibbonBitmap); control(__vt__10CGeneTex3D);
    }
    // Release the ribbon bitmap and array before the common renderer base is destroyed.
    ~Build134() {
        table = __vt__10CBallTrack;
        if (resource) { ResourceManager_ReleaseId(g_resourceManager, (int)resource); resource = 0; }
        if (array) { __dla__FPv((char*)array - 16); array = 0; }
    }
};
// Construct a skid ribbon with unset frame limits and its skid bitmap.
struct Build130 : BuildBase {
    void *array;
    // +11c capacity, +120 populated rows, +124/+128 inclusive history limits,
    // +12c last processed sample. Limits -1 request the later initializer's default.
    int pointCapacity, pointCount, startFrame, endFrame, lastProcessedFrame;
    // Initialize a skid effect with unset frame limits and its renderer/bitmap.
    Build130() {
        table=__vt__9CSkidMark; array=0;
        startFrame=-1; endFrame=-1; pointCapacity=0; pointCount=0;
        load(kSkidBitmap); control(__vt__10CGeneTex3D);
    }
};
struct BuildVector {
    char data[12];
    // Zero a world-space vector stored inside the wind effect.
    BuildVector() { Vec3_Zero(this); }
};
// Construct wind particles plus the embedded ball ribbon and camera vectors.
struct Build26c : BuildBase {
    void *array;
    Build134 inner; // Owned embedded ribbon; constructed before the outer particles.
    char pad250[4];
    // Same storage as CBallWindTrack.h: +254 is last camera world position;
    // +260 is current camera displacement, normalized only for steps >=1 unit.
    // UpdateCameraStreaks produces both; DrawRibbonAndStreaks reads +260 for
    // nonzero-length admission. Neither represents CWind's physical wind force.
    // Zero construction seeds the first displacement relative to world origin;
    // StartAndResetParticles does not replace these vectors with camera position.
    BuildVector previousCameraPosition, cameraMotionVector;
    // Allocate 256 wind particles after constructing the embedded ball ribbon.
    Build26c() : BuildBase(__vt__14CBallWindTrack), array(0) {
        array=new BuildElement24[kWindParticleCount];
    }
};
// Construct the putt ribbon with an owned polygon renderer.
struct Build11c : BuildBase {
    void *array;
    // Initialize an empty putt ribbon with its owned polygon renderer.
    Build11c() {
        table=__vt__9CPuttLine; array=0; control(__vt__11CGenePoly3D);
    }
};
// Construct the cobra skid ribbon with its dedicated bitmap.
struct BuildLast130 : BuildBase {
    void *array;
    // Same skid-row descriptor as Build130; limits are history sample indices.
    int pointCapacity,pointCount,startFrame,endFrame,lastProcessedFrame;
    // Initialize the cobra skid effect with its distinct bitmap and unset frame limits.
    BuildLast130() {
        table=__vt__16ntSkidMark_Cobra; array=0; control(__vt__10CGeneTex3D);
        startFrame=-1; endFrame=-1; pointCapacity=0; pointCount=0; load(kCobraBitmap);
    }
};
// Construct the initially hidden projected ground marker and load its bitmap.
struct BuildLast {
    unsigned char flag; char pad[3]; void *resource; char rest[12];
    // Create an initially hidden ground marker and acquire its bitmap.
    BuildLast() {
        flag=0; resource=ResourceManager_LoadBitmap(g_resourceManager,ballTrackEffectStrings + kGroundBitmap,0,0,0);
    }
};
// Constructor adapters retain their ABI type names so exception cleanup symbols stay
// unchanged. Use these role aliases when adding/reordering effect construction.
// BuildBase: resource=bitmap ID, controller=owned quad renderer, active=enabled byte;
// value at +0x18 is the ring spawn interval here; CComet uses the same offset for its
// sample interval. The ball ribbon does not initialize this common-layout word here.
// Build130/BuildLast130: pointCapacity/pointCount describe allocated/populated rows;
// startFrame/endFrame bound history samples; lastProcessedFrame tracks updates.
// All array pointers own cookie-backed arrays.
typedef BuildFirst CometConstruction;
typedef BuildArray RingEffectConstruction;
typedef Build134 BallRibbonConstruction;
typedef Build130 SkidConstruction;
typedef Build26c WindTrailConstruction;
typedef Build11c PuttRibbonConstruction;
typedef BuildLast130 CobraSkidConstruction;
typedef BuildLast GroundMarkerConstruction;

// Allocate each trajectory effect and the projected ground marker.
void CTrajectory_OnLoad(void *self) {
    AsCTrajectory(self)->comet=new CometConstruction;
    AsCTrajectory(self)->doughnut=new RingEffectConstruction(kRingParticleCount);
    AsCTrajectory(self)->ballTrack=new BallRibbonConstruction;
    AsCTrajectory(self)->skidMark=new SkidConstruction;
    AsCTrajectory(self)->ballWindTrack=new WindTrailConstruction;
    AsCTrajectory(self)->puttLine=new PuttRibbonConstruction;
    AsCTrajectory(self)->cobraSkidMark=new CobraSkidConstruction;
    AsCTrajectory(self)->groundMarker=new GroundMarkerConstruction;
}

// Release a ball ribbon's bitmap, sample array and quad renderer; optionally free the ribbon.
void *CBallTrack_Destroy(void *self, int flags) {
    void *bitmap;
    void *array;
    if (self != 0) {
        *(void **)self = __vt__10CBallTrack;
        bitmap = AsCBallTrack(self)->bitmapResourceId;
        if (bitmap != 0) {
            ResourceManager_ReleaseId(g_resourceManager, (*(int*)&AsCBallTrack(self)->bitmapResourceId));
            (*(int*)&AsCBallTrack(self)->bitmapResourceId) = 0;
        }
        array = AsCBallTrack(self)->trailPoints;
        if (array != 0) {
            if (array != 0) {
                __dla__FPv((char *)array - 0x10);
            }
            AsCBallTrack(self)->trailPoints = 0;
        }
        if (self != 0) {
            struct DBase { int pad; };
            struct DObj : DBase { 
            virtual void v0(int) DECOMP_LOCAL_VIRTUAL;
            };
            DObj *member = (*(DObj**)&AsCBallTrack(self)->quadRenderer);
            *(void **)self = s_ballTraceBaseVtable;
            if (member != 0) {
                if (member != 0) {
                    member->v0(1);
                }
                AsCBallTrack(self)->quadRenderer = 0;
            }
        }
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}

// Draw active trajectory effects and project the enabled ground marker onto the course.
// Requires loaded marker even when inactive; virtual a12 suppresses the entire
// draw. Unavailable playback history draws nothing. Active slots borrow owners;
// marker projection queries current terrain Y without changing the stored position.
void CTrajectory_Draw(void *self) {
    if (((struct VObjTop *)self)->a12() != 0)
        return;
    if (((CGolfBall*)((void *(*)())GolfBall__Fv)())->m_eventPreDataIndex[kHistoryPlaybackSample] < 0) {
    } else {
        SetTransform(kEffectWorldTransform, kIdentityMatrix4x4);
        // gx_renderer_state_and_resources.cpp maps texture stage N to transform slot N+2. Reset texture
        // stage 0 independently of world/view/projection before these effect draws.
        SetTransform(kEffectTexture0Transform, kIdentityMatrix4x4);
        void *cursor = self;
        for (int i = 0; i < kActiveEffectSlots; ++i) {
            void *obj = AsCTrajectory(cursor)->activeEffect0;
            if (obj != 0) {
                ((struct VObj5 *)obj)->v3(); // Draw the borrowed active effect.
            }
            cursor = (char *)cursor + 4;
        }
        cursor = AsCTrajectory(self)->groundMarker;
        if (*(unsigned char *)cursor != 0) {
            float first[3] = {kFloat_0_0_BallTrack, kFloat_0_0_BallTrack, kFloat_15_TrajectoryMarker};
            float second[3] = {kFloat_15_TrajectoryMarker, kFloat_0_0_BallTrack, kFloat_0_0_BallTrack};
            unsigned int third[3];
            unsigned int t0 = (*(unsigned int*)&((GroundMarkerStorage*)cursor)->position.x);
            unsigned int t1 = (*(unsigned int*)&((GroundMarkerStorage*)cursor)->position.y);
            third[0] = t0;
            third[1] = t1;
            third[2] = (*(unsigned int*)&((GroundMarkerStorage*)cursor)->position.z);
            float output[3];
            void *transform = CQuadTree_GetInstance();
            GetGroundPoint__9CQuadTreeF7WVectorbPib(output, transform, third, 0, 0, 0);
            transform = CQuadTree_GetInstance();
            CQuadTree_DrawProjectedTexturedPatch(transform, kFloat_1_BallTrack, kFloat_1_BallTrack, ((GroundMarkerStorage*)cursor)->bitmap,
                        output, second, first, 0, 0);
        }
    }
}

// Queue trajectory resources through the actor's secondary message interface.
// arg may be null; otherwise the queued callback receives arg+0x18. The actor
// storage must survive the asynchronous preload request; the queue does not own it.
void CTrajectory_OnPreLoadInit(void *arg) {
    void *adjusted = arg;
    if (adjusted != 0)
        adjusted = (char *)adjusted + kActorMessageInterfaceOffset;
    ResourcePreloadQueue_AddRequest(g_sceneManager, 6, 0, kFloat_1_BallTrack, adjusted,
                kFloat_0_0_BallTrack, ballTrackEffectStrings, 0, 0, 0);
}

// Advance the five active effects once simulated ball history is available.
// delta is elapsed seconds, forwarded unchanged. Negative playback history skips
// every child. Slots remain borrowed and each nonnull slot is updated once in order.
void CTrajectory_OnProcess(void *self, float delta) {
    if (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryPlaybackSample] < 0)
        return;
    for (int i = 0; i < kActiveEffectSlots; ++i) {
        void *obj = AsCTrajectory(self)->activeEffect0;
        if (obj != 0) {
            ((struct VObj4 *)obj)->v2(delta); // Advance by delta seconds.
        }
        self = (char *)self + 4;
    }
}

// Destroy the trace base's owned quad renderer and optionally free the trace.
void *CBallTrace_Destroy(void *self, int flags) {
    if (self != 0) {
        struct DBase { int pad; };
        struct DObj : DBase { 
        virtual ~DObj() DECOMP_LOCAL_VIRTUAL;
        };
        DObj *member = (*(DObj**)&AsCBallTrace(self)->quadRenderer);
        *(void **)self = s_ballTraceBaseVtable;
        if (member != 0) {
            delete member;
            (*(int*)&AsCBallTrace(self)->quadRenderer) = 0;
        }
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy the seven trajectory effects and release the ground-marker bitmap.
// Does not clear active slots: caller must stop process/display before teardown.
// Destructors clear owner pointers; the resource manager must outlive all releases.
void CTrajectory_OnDestroy(void *self) {
    struct Obj { 
    virtual ~Obj() DECOMP_LOCAL_VIRTUAL;
    };
    void *obj = AsCTrajectory(self)->comet;
    if (obj != 0) { delete (Obj *)obj; AsCTrajectory(self)->comet = 0; }
    obj = AsCTrajectory(self)->doughnut;
    if (obj != 0) { delete (Obj *)obj; AsCTrajectory(self)->doughnut = 0; }
    obj = AsCTrajectory(self)->ballTrack;
    if (obj != 0) { delete (Obj *)obj; AsCTrajectory(self)->ballTrack = 0; }
    obj = AsCTrajectory(self)->skidMark;
    if (obj != 0) { delete (Obj *)obj; AsCTrajectory(self)->skidMark = 0; }
    obj = AsCTrajectory(self)->ballWindTrack;
    if (obj != 0) { delete (Obj *)obj; AsCTrajectory(self)->ballWindTrack = 0; }
    obj = AsCTrajectory(self)->puttLine;
    if (obj != 0) { delete (Obj *)obj; AsCTrajectory(self)->puttLine = 0; }
    obj = AsCTrajectory(self)->cobraSkidMark;
    if (obj != 0) { delete (Obj *)obj; AsCTrajectory(self)->cobraSkidMark = 0; }
    void *p = AsCTrajectory(self)->groundMarker;
    if (p != 0) {
        if (p != 0) {
            if (((GroundMarkerStorage*)p)->bitmap != 0) {
                ResourceManager_ReleaseId(g_resourceManager, (*(int*)&((GroundMarkerStorage*)p)->bitmap));
                ((GroundMarkerStorage*)p)->bitmap = 0;
            }
            __dl__FPv(p);
        }
        AsCTrajectory(self)->groundMarker = 0;
    }
}

struct TrackPoint {
    char data[0x18];
    // Leave ribbon-edge storage uninitialized until sample generation writes it.
    TrackPoint() {}
    // Allocate element arrays through the game allocator; CodeWarrior adds its array cookie.
    static void *operator new[](size_t size) { return __nwa__FUl(size); }
};

struct TrackCtl : BuildControllerPad {
    virtual void c0(); virtual void c1(); virtual void c2(); virtual void c3();
    virtual void c4();
    virtual void c5(unsigned char, float, float);
};

// Allocate a ribbon across ball-history samples and initialize its lifetime, opacity and UVs.
// History-to-time conversion is 1/30 second per sample. World-space half-width is
// normalize(worldUp cross sampleVelocity) * 1.2 * ballRadius; edges are center +/- half-width.
// Allocates preDataCount+1 pairs, replacing the old owned array; require sample 0
// and history gate indices within the allocation. Fade seconds clamp to [.5,2].
// remainingLifetime = gateSample/30 + fadeDuration - 1, so the shared one also
// supplies a one-second lead. Keep fade setup and per-update alpha integration aligned.
void CBallTrack_InitTrailPoints(void *self) {
    WVector mtx;
    float up[3];

    struct TrackPoint *old = (*(struct TrackPoint**)&AsCBallTrack(self)->trailPoints);
    if (old != 0) {
        delete[] old;
        AsCBallTrack(self)->trailPoints = 0;
    }
    int count = ((CGolfBall*)((void *(*)())GolfBall__Fv)())->m_preDataCount + 1;
    struct TrackPoint *array = new struct TrackPoint[count];
    AsCBallTrack(self)->trailPoints = array;
    AsCBallTrack(self)->active = 1;
    int high = ((CGolfBall*)((void *(*)())GolfBall__Fv)())->m_preDataCount;
    int low = ((CGolfBall*)((void *(*)())GolfBall__Fv)())->m_eventPreDataIndex[kHistoryEffectGate1];
    float f = kBallTrackSecondsPerHistorySample * (float)(high - low);
    float clamped = f < kTrajectorySharedHalf ? kTrajectorySharedHalf : (f > kFloat_2_TrajectoryShared ? kFloat_2_TrajectoryShared : f);
    float full = kFullOpacity255;
    AsCBallTrack(self)->fadeDuration = clamped;
    AsCBallTrack(self)->initialOpacity = full;
    AsCBallTrack(self)->opacity = full;
    low = ((CGolfBall*)((void *(*)())GolfBall__Fv)())->m_eventPreDataIndex[kHistoryEffectGate1];
    float value = kBallTrackSecondsPerHistorySample * (float)low + AsCBallTrack(self)->fadeDuration - kFloat_1_BallTrack;
    AsCBallTrack(self)->historyIndex = 0;
    AsCBallTrack(self)->trailPointCount = 1;
    AsCBallTrack(self)->remainingLifetime = value;
    up[0] = kFloat_0_0_BallTrack; up[1] = kFloat_1_BallTrack; up[2] = kFloat_0_0_BallTrack;
    Vector_CrossProduct(&mtx, up, &((BallHistorySample*)PreData__9CGolfBallFi(((void *(*)())GolfBall__Fv)(), 0))->velocity);
    mtx.Normalize();
    mtx *= kBallRibbonRadiusMultiplier * ((CGolfBall*)((void *(*)())GolfBall__Fv)())->m_radius;
    WVector first = *(WVector*)(PreData__9CGolfBallFi(((void *(*)())GolfBall__Fv)(), 0)) + mtx;
    *(WVector*)(AsCBallTrack(self)->trailPoints) = first;
    WVector second = *(WVector*)(PreData__9CGolfBallFi(((void *(*)())GolfBall__Fv)(), 0)) - mtx;
    *(WVector*)((char *)AsCBallTrack(self)->trailPoints + 0xc) = second;
    for (int i = 0; i < kQuadVertexCount; ++i) {
        void *controller = AsCBallTrack(self)->quadRenderer;
        if (*(unsigned char *)controller > (unsigned char)i) {
            float x = (i & 1) ? kFloat_0_0_BallTrack : kFloat_1_BallTrack;
            float y = (i & 2) ? kFloat_1_BallTrack : kFloat_0_0_BallTrack;
            ((struct TrackCtl *)controller)->c5((unsigned char)i, y, x);
        }
    }
}

// Reserved ball-ribbon hook; it performs no work.
void CBallTrack_DefaultHook() {}

// Dispatch effect requests into the active slots; reset requests clear every slot and the marker.
// Request map: comet/rings/ball-ribbon/wind -> air order 6;
// skid/putt-ribbon/cobra-skid -> ground order 1. These seven store one borrowed
// pointer in a free slot BEFORE initialization, THEN adjust scene order.
// GroundMarker -> order 1, position/enable only; RestoreBall -> opacity only.
// Both still require a free slot but leave it empty. Reset bypasses the gate,
// restores ball opacity, clears slots and disables the marker without deletion.
// arg borrows TrajectoryMessage. Slots alias owned effects, not new allocations; vtable[3]
// initializes synchronously. Repeated requests can alias one effect in several slots.
// Skid bounds are clamped to playback/history limits; callers supply initialized effect objects.
void CTrajectory_HandleMsg(void *self, const void *arg) {
    // Phase 1: preserve the full-slot return for requests that do not store a slot too.
    // The unsigned range check rejects negative codes after the slot search.
    int type = ((const TrajectoryMessage*)arg)->effectType;
    void **slot = 0;
    if (type != kRequestReset) {
        char *cursor = (char *)self;
        for (int i = 0; i < kActiveEffectSlots; ++i) {
            if (AsCTrajectory(cursor)->activeEffect0 == 0) {
                slot = (void **)((char *)self + i * sizeof(void*) + kEffectSlotOffset);
                break;
            }
            cursor += 4;
        }
        if (slot == 0)
            return;
    }
    if ((unsigned int)type > kRequestGroundMarker)
        return;
    // Phase 2: initialize the requested effect and select its existing scene ordering key.
    void *obj;
    switch (type) {
    case kRequestComet:
        obj = AsCTrajectory(self)->comet;
        if (obj == 0) return;
        *slot = obj;
        ((TrajectoryEffectVtable*)*(void**)obj)->initialize(obj);
        if (NamedEffectList_GetRenderKind(g_sceneManager, ballTrackEffectStrings) != kAirEffectOrder)
            NamedEffectList_SetRenderKind(g_sceneManager, ballTrackEffectStrings, kAirEffectOrder);
        return;
    case kRequestRings:
        obj = AsCTrajectory(self)->doughnut;
        if (obj == 0) return;
        *slot = obj;
        ((TrajectoryEffectVtable*)*(void**)obj)->initialize(obj);
        if (NamedEffectList_GetRenderKind(g_sceneManager, ballTrackEffectStrings) != kAirEffectOrder)
            NamedEffectList_SetRenderKind(g_sceneManager, ballTrackEffectStrings, kAirEffectOrder);
        return;
    case kRequestBallRibbon:
        obj = AsCTrajectory(self)->ballTrack;
        if (obj == 0) return;
        *slot = obj;
        ((TrajectoryEffectVtable*)*(void**)obj)->initialize(obj);
        if (NamedEffectList_GetRenderKind(g_sceneManager, ballTrackEffectStrings) != kAirEffectOrder)
            NamedEffectList_SetRenderKind(g_sceneManager, ballTrackEffectStrings, kAirEffectOrder);
        return;
    case kRequestSkid:
        if (AsCTrajectory(self)->skidMark == 0) return;
        {
            int endSample = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySkidOrigin7] + kSkidHistoryWindow;
            int startSample = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySkidOrigin7];
            void *target = AsCTrajectory(self)->skidMark;
            int limit = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryPlaybackSample];
            if (startSample < limit) startSample = limit;
            ((SkidFrameRangeView*)target)->startFrame = startSample;
            limit = ((CGolfBall*)GolfBall__Fv())->m_preDataCount;
            if (endSample > limit) endSample = limit;
            AsCSkidMark(target)->endFrame = endSample;
            *slot = target;
            ((TrajectoryEffectVtable*)*(void**)target)->initialize(target);
            if (NamedEffectList_GetRenderKind(g_sceneManager, ballTrackEffectStrings) != kGroundEffectOrder)
                NamedEffectList_SetRenderKind(g_sceneManager, ballTrackEffectStrings, kGroundEffectOrder);
        }
        return;
    case kRequestWind:
        obj = AsCTrajectory(self)->ballWindTrack;
        if (obj == 0) return;
        *slot = obj;
        ((TrajectoryEffectVtable*)*(void**)obj)->initialize(obj);
        if (NamedEffectList_GetRenderKind(g_sceneManager, ballTrackEffectStrings) != kAirEffectOrder)
            NamedEffectList_SetRenderKind(g_sceneManager, ballTrackEffectStrings, kAirEffectOrder);
        return;
    case kRequestPuttRibbon:
        obj = AsCTrajectory(self)->puttLine;
        if (obj == 0) return;
        *slot = obj;
        ((TrajectoryEffectVtable*)*(void**)obj)->initialize(obj);
        if (NamedEffectList_GetRenderKind(g_sceneManager, ballTrackEffectStrings) != kGroundEffectOrder)
            NamedEffectList_SetRenderKind(g_sceneManager, ballTrackEffectStrings, kGroundEffectOrder);
        return;
    case kRequestCobraSkid:
        if (AsCTrajectory(self)->cobraSkidMark == 0) return;
        {
            void *target = AsCTrajectory(self)->cobraSkidMark;
            int endSample = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryCobraEnd10];
            int playbackSample = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryPlaybackSample];
            ((SkidFrameRangeView*)target)->startFrame = playbackSample > 0 ? playbackSample : 0;
            int historyEnd = ((CGolfBall*)GolfBall__Fv())->m_preDataCount;
            if (endSample > historyEnd) endSample = historyEnd;
            AsCSkidMark(target)->endFrame = endSample;
            *slot = target;
            ((TrajectoryEffectVtable*)*(void**)target)->initialize(target);
            if (NamedEffectList_GetRenderKind(g_sceneManager, ballTrackEffectStrings) != kGroundEffectOrder)
                NamedEffectList_SetRenderKind(g_sceneManager, ballTrackEffectStrings, kGroundEffectOrder);
        }
        return;
    case kRequestGroundMarker:
        if (NamedEffectList_GetRenderKind(g_sceneManager, ballTrackEffectStrings) != kGroundEffectOrder)
            NamedEffectList_SetRenderKind(g_sceneManager, ballTrackEffectStrings, kGroundEffectOrder);
        {
            void *p = AsCTrajectory(self)->groundMarker;
            *(unsigned char *)p = 1;
            ((GroundMarkerStorage*)p)->position = *(WVector*)(GolfBall__Fv());
        }
        return;
    case kRequestReset:
        if (NamedEffectList_FindByName(g_sceneManager, ballTrackEffectStrings + kBallActorName))
            WPuppet_SetOpacityRecursive(NamedEffectList_FindByName(g_sceneManager, ballTrackEffectStrings + kBallActorName), 0xff);
        (*(int*)&AsCTrajectory(self)->activeEffect0) = 0;
        AsCTrajectory(self)->activeEffect1 = 0;
        AsCTrajectory(self)->activeEffect2 = 0;
        AsCTrajectory(self)->activeEffect3 = 0;
        AsCTrajectory(self)->activeEffect4 = 0;
        if (AsCTrajectory(self)->groundMarker != 0)
            *(unsigned char *)AsCTrajectory(self)->groundMarker = 0;
        return;
    case kRequestRestoreBall:
        if (NamedEffectList_FindByName(g_sceneManager, ballTrackEffectStrings + kBallActorName))
            WPuppet_SetOpacityRecursive(NamedEffectList_FindByName(g_sceneManager, ballTrackEffectStrings + kBallActorName), 0xff);
        return;
    }
}

// Append spaced ball-history ribbon edges, keep a live endpoint and fade the ribbon over time.
// delta is elapsed seconds. Preserve boundary samples even before five samples pass;
// otherwise skip a nearby sample only while its position is within 0.32 world units.
// Requires allocated capacity for every history edge plus the live endpoint;
// no bounds check is performed here. Opacity/lifetime can overshoot below zero
// before activity is cleared; preserve the existing order and float operations.
void CBallTrack_UpdateTrailAndFade(void *self, float delta) {
    if (AsCBallTrack(self)->active != 0) {
        int last = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryPlaybackSample];
        int current = AsCBallTrack(self)->historyIndex;
        if (current != last) {
            float fZero = kFloat_0_0_BallTrack;
            float fOne = kFloat_1_BallTrack;
            float f12 = kBallRibbonRadiusMultiplier;
            float f032 = kFloat_0_32000002_BallTrack;
            int i = current + 1;
            // Original unit 0x80201D04: skip nearby history samples.
            for (; i <= last; ++i) {
                if (i - AsCBallTrack(self)->historyIndex < kHistorySampleSpacing) {
                    void *candidate = PreData__9CGolfBallFi(GolfBall__Fv(), i);
                    if (((BallHistorySample*)candidate)->boundaryMarker == 0) {
                        void *old = PreData__9CGolfBallFi(GolfBall__Fv(), AsCBallTrack(self)->historyIndex);
                        candidate = PreData__9CGolfBallFi(GolfBall__Fv(), i);
                        float __tmp[3];
                        __mi__FRC7WVectorRC7WVector(__tmp, candidate, old);
                        if (WVectorLen__FRC7WVector(__tmp) < f032)
                            continue;
                    }
                }
                {
                    float up[3] = {fZero, fOne, fZero};
                    float vec[3];
                    float trans[3];
                    void *obj = PreData__9CGolfBallFi(GolfBall__Fv(), i);
                    Vector_CrossProduct(vec, up, &((BallHistorySample*)obj)->velocity);
                    __as__7WVectorFRC7WVector(trans, vec);
                    Normalize__7WVectorFv(trans);
                    __amu__7WVectorFf(trans, f12 * ((CGolfBall*)GolfBall__Fv())->m_radius);
                    obj = PreData__9CGolfBallFi(GolfBall__Fv(), i);
                    __pl__FRC7WVectorRC7WVector(vec, obj, trans);
                    __as__7WVectorFRC7WVector((char *)AsCBallTrack(self)->trailPoints + AsCBallTrack(self)->trailPointCount * 0x18, vec);
                    obj = PreData__9CGolfBallFi(GolfBall__Fv(), i);
                    __mi__FRC7WVectorRC7WVector(vec, obj, trans);
                    __as__7WVectorFRC7WVector((char *)AsCBallTrack(self)->trailPoints + AsCBallTrack(self)->trailPointCount * 0x18 + 0xc, vec);
                    AsCBallTrack(self)->historyIndex = i;
                    ++AsCBallTrack(self)->trailPointCount;
                }
            }
        }
        // Keep a live, uncounted endpoint after committed edges until history playback finishes.
        if (AsCBallTrack(self)->historyIndex < ((CGolfBall*)GolfBall__Fv())->m_preDataCount) {
            float fZero = kFloat_0_0_BallTrack;
            float fOne = kFloat_1_BallTrack;
            float up[3] = {fZero, fOne, fZero};
            float vec[3];
            float trans[3];
            void *obj = PreData__9CGolfBallFi(GolfBall__Fv(), last);
            Vector_CrossProduct(vec, up, &((BallHistorySample*)obj)->velocity);
            __as__7WVectorFRC7WVector(trans, vec);
            Normalize__7WVectorFv(trans);
            __amu__7WVectorFf(trans, kBallRibbonRadiusMultiplier * ((CGolfBall*)GolfBall__Fv())->m_radius);
            __pl__FRC7WVectorRC7WVector(vec, GolfBall__Fv(), trans);
            __as__7WVectorFRC7WVector((char *)AsCBallTrack(self)->trailPoints + AsCBallTrack(self)->trailPointCount * 0x18, vec);
            __mi__FRC7WVectorRC7WVector(vec, GolfBall__Fv(), trans);
            __as__7WVectorFRC7WVector((char *)AsCBallTrack(self)->trailPoints + AsCBallTrack(self)->trailPointCount * 0x18 + 0xc, vec);
        }
        // Fade phase: lifetime/duration are seconds; float opacity uses the 0..255 scale.
        float time = AsCBallTrack(self)->remainingLifetime - delta;
        float duration = AsCBallTrack(self)->fadeDuration;
        AsCBallTrack(self)->remainingLifetime = time;
        if (time < duration) {
            AsCBallTrack(self)->opacity -= delta * (kFloat_1_BallTrack / duration * AsCBallTrack(self)->initialOpacity);
        } else {
            AsCBallTrack(self)->opacity = AsCBallTrack(self)->initialOpacity;
        }
        if (AsCBallTrack(self)->opacity < kFloat_0_0_BallTrack)
            AsCBallTrack(self)->active = 0;
    }
}

// Adjust a secondary-interface pointer and forward destruction to the title task.
void CTitleTask_vfunc_1_ThunkMinus8(void *self, int forwarded1) {
    CTitleTask_dtor((char *)self - 8, forwarded1);
}

// Initialize this trajectory unit's cached full-turn angle.
void CTrajectory_InitializeUnitConstants() {
    s_twoPi_Trajectory = kFloat_6_2831855;
}

} // extern "C"

} // namespace ns0

// ---- merged from ball_trajectory_effects.cpp ----
namespace ns1 {
// Translation unit 0x80203204-0x80204958 (12 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern float WVectorLen__FRC7WVector(void*);
extern void Normalize__7WVectorFv(void*);
extern float __ml__FRC7WVectorRC7WVector(void*, void*);
extern void __ml__FRC7WVectorf(void*, void*, float);
extern void __ami__7WVectorFRC7WVector(void*, void*);
extern void __ml__FfRC7WVector(void*, void*, float);
extern void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern float Random_RangeFloat(int, float, float);
extern char s_cameraTransform[];
extern float kFloat_160_BallTrack, kFloat_200_BallTrack, kFloat_128_BallTrack, kFloat_30_BallWindTrack, kWindTrackMeanStreakLength;
extern float kTrajectorySharedHalf, kNegativeUnit, kFloat_75_BallWindTrack;
extern double kDouble_SignedIntConversionBias_InventoryRows;
extern const double kSkidMarkNearbyAlphaMultiplier;
extern const float kBallRibbonRadiusMultiplier;
extern const float kSkidMarkNearSampleDistance, kSkidMarkBaseOpacity;
extern void* PreData__9CGolfBallFi(void*, int);
extern void Vector_CrossProduct(void*, void*, void*);
extern void __amu__7WVectorFf(void*, float);
extern float Magnitude__7WVectorCFv(void*);
extern void* CQuadTree_GetInstance();
extern void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);

extern unsigned int cometRenderStatePreset[];
extern void* GolfBall__Fv();
extern void Engine_SetRenderStateList();
extern void Engine_SetTextureStageStateList(int, void*, int);
extern void Engine_SetTextureStageParameterList(int, void*, int);
extern void Engine_SetCachedRenderState(int, int);
extern void* Camera_GetRenderState(void*);
extern void SetTransform(int, void*);
extern void Engine_SetType7Resource(int);
extern void Engine_SetTextureStageResource(int, void*);
extern void Engine_SetTexturePalette(int, void*);
extern void* GX_GetPrimitiveCount(int, int);
extern void Engine_SetRenderStateList();
extern void Engine_SetTextureStageStateList(int, void*, int);
extern void Engine_SetTextureStageParameterList(int, void*, int);
extern void Engine_SetCachedRenderState(int, int);
extern void Engine_SetType7Resource(int);
extern void Engine_SetTextureStageResource(int, void*);
extern void Engine_SetTexturePalette(int, void*);

extern void* g_view;
struct DrawWords30 { unsigned int words[30]; };
struct DrawWords12 { unsigned int words[12]; };
struct DrawWords10 { unsigned int words[10]; };
struct DrawVec {
    unsigned int x, y, z;
    // Copy raw XYZ float bits into the quad-position ABI argument without conversion.
    static DrawVec from(const unsigned int* p) { return *(const DrawVec*)p; }
};
struct DrawTexture {
    unsigned char count;
    virtual void unused0();
    virtual void begin();
    virtual void unused10();
    virtual void color(unsigned char, unsigned char, unsigned char, unsigned char, unsigned char);
    virtual void position(unsigned char, DrawVec);
    virtual void unused1c();
    virtual void draw(int, void*);
    // Set one valid quad color, forwarding the original fixed and caller-supplied byte order.
    void setColor(unsigned char i, unsigned char a) {
        if (count > i) color(i, 255, a, a, a);
    }
    // Set a world-space corner only if the quad renderer contains that vertex.
    void setPosition(unsigned char i, DrawVec v) {
        if (count > i) position(i, v);
    }
};
struct DrawRenderer {
    virtual void v00(); virtual void v01(); virtual void v02(); virtual void v03();
    virtual void v04(); virtual void v05(); virtual void v06(); virtual void v07();
    virtual void v08(); virtual void v09(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24(); virtual void v25();
    virtual void* texture(void*);
    virtual void* mode(void*);
};

#pragma use_lmw_stmw on
// Draw terrain-projected skid quads with each row's stored opacity.
void CSkidMark_Draw(void* self) {
    // Phase 1: bound visible rows, including the live endpoint only while the frame range advances.
    unsigned int* base = cometRenderStatePreset;
    char* p = (char*)self;
    if (!AsCSkidMark(p)->enabled)
        return;
    int end = (AsCSkidMark(p)->lastProcessedFrame < AsCSkidMark(p)->endFrame) ? AsCSkidMark(p)->pointCount + 1 : AsCSkidMark(p)->pointCount;
    int frame = AsCSkidMark(p)->pointCapacity;
    if (frame > end)
        frame = end;

    // Phase 2: install preset render/texture state and camera matrices; bind the skid bitmap.
    DrawWords30 states = *(DrawWords30*)base;
    DrawWords12 pipeline = *(DrawWords12*)(base + 30);
    DrawWords10 other = *(DrawWords10*)(base + 42);
    DrawVec positionBits;
    ((void (*)(void*, int))Engine_SetRenderStateList)(&states, 15);
    Engine_SetTextureStageStateList(0, &pipeline, kTrailPresetStagePairs);
    Engine_SetTextureStageParameterList(0, &other, kTrailPresetSamplerPairs);
    Engine_SetCachedRenderState(kTrailSourceBlendState, kTrailBlendZero);
    Engine_SetCachedRenderState(kTrailDestinationBlendState, kTrailBlendInverseSourceColor);

    void* transform = Camera_GetRenderState(g_view);
    SetTransform(kEffectViewTransform, ((EffectCameraMatrices*)transform)->view);
    SetTransform(kEffectProjectionTransform, ((EffectCameraMatrices*)transform)->projection);
    DrawTexture* texture = (*(DrawTexture**)&AsCSkidMark(p)->quadRenderer);
    Engine_SetType7Resource(0);
    texture->begin();
    DrawRenderer* renderer = (*(DrawRenderer**)&AsWView(g_view)->renderTarget);
    void* object = AsCSkidMark(p)->textureResource;
    Engine_SetTextureStageResource(0, renderer->texture(object));
    renderer = (*(DrawRenderer**)&AsWView(g_view)->renderTarget);
    Engine_SetTexturePalette(0, renderer->mode(object));
    Engine_SetCachedRenderState(kTrailDepthWriteState, 1);
    Engine_SetCachedRenderState(kTrailDepthCompareState, kTrailCompareAlways);
    // Phase 3: each quad joins two adjacent WORLD-space left/right edge rows.
    // Corner bit 1 chooses current vs previous row; bit 0 chooses right vs left edge.
    int rowOffset;
    int row = 1;
    rowOffset = 0x1c;
    while (row < frame) {
        for (int channel = 0; channel < 4; ++channel) {
            char* entry = (char*)(rowOffset + (*(int*)&AsCSkidMark(p)->trailPoints));
            unsigned char alpha = *(unsigned char*)(entry + 0x18);
            ((*(DrawTexture**)&AsCSkidMark(p)->quadRenderer))->setColor((unsigned char)channel, alpha);
            int previous = row - 1;
            if (channel & 2)
                previous = row;
            if (channel & 1)
                positionBits = *(DrawVec*)((*(char**)&AsCSkidMark(p)->trailPoints) + previous * 0x1c + 0xc);
            else
                positionBits = *(DrawVec*)((*(char**)&AsCSkidMark(p)->trailPoints) + previous * 0x1c);
            ((*(DrawTexture**)&AsCSkidMark(p)->quadRenderer))->setPosition((unsigned char)channel, DrawVec::from(&positionBits.x));
        }
        texture = (*(DrawTexture**)&AsCSkidMark(p)->quadRenderer);
        unsigned char count = texture->count;
        if (count != 0) {
            void* data = GX_GetPrimitiveCount(count, 0x98);
            texture->draw(0x98, data);
        }
        rowOffset += 0x1c;
        ++row;
    }
}
#pragma use_lmw_stmw reset

struct BallTrackPositionBits { unsigned int v[3]; };
struct BallTrackTex {
    unsigned char count;
    char pad01[3];
    void** vt;
};

// Draw the ball ribbon, including its unsaved live endpoint when history is still advancing.
void CBallTrack_Draw(void* self) {
    // Phase 1: decide whether the uncommitted live endpoint participates in this draw.
    char* p = (char*)self;
    unsigned int* base = cometRenderStatePreset;
    if (!AsCBallTrack(p)->active)
        return;

    void* state = GolfBall__Fv();
    unsigned int stateWord = (*(unsigned int*)&((CGolfBall*)state)->m_preDataCount);
    unsigned int mask = (*(unsigned int*)&AsCBallTrack(p)->historyIndex) ^ stateWord;
    int changed = (unsigned int)(((int)mask >> 1) - (int)(mask & stateWord)) >> 31;

    // Phase 2: copy preset state, install camera transforms and bind the ribbon bitmap/quad.
    DrawWords30 constants = *(DrawWords30*)base;
    char* pipelineSrc = (char*)base + sizeof(DrawWords30);
    char* otherSrc = (char*)base + sizeof(DrawWords30) + sizeof(DrawWords12);
    DrawWords10 other;
    DrawWords12 pipeline;
    pipeline = *(DrawWords12*)pipelineSrc;
    other = *(DrawWords10*)otherSrc;
    ((void (*)(void*, int))Engine_SetRenderStateList)(&constants, 15);
    Engine_SetTextureStageStateList(0, &pipeline, kTrailPresetStagePairs);
    Engine_SetTextureStageParameterList(0, &other, kTrailPresetSamplerPairs);
    Engine_SetCachedRenderState(kTrailSourceBlendState, kTrailBlendSourceAlpha);
    Engine_SetCachedRenderState(kTrailDestinationBlendState, kTrailBlendOne);

    void* transform = Camera_GetRenderState(g_view);
    SetTransform(kEffectViewTransform, ((EffectCameraMatrices*)transform)->view);
    SetTransform(kEffectProjectionTransform, ((EffectCameraMatrices*)transform)->projection);

    BallTrackTex* texture = (*(BallTrackTex**)&AsCBallTrack(p)->quadRenderer);
    Engine_SetType7Resource(0);
    void (*begin)(void*) = (void (*)(void*))texture->vt[3];
    begin(texture);

    void* object = AsCBallTrack(p)->bitmapResourceId;
    void* renderer = AsWView(g_view)->renderTarget;
    void* a = ((void* (*)(void*, void*))(*(void***)renderer)[0x70 / 4])(renderer, object);
    Engine_SetTextureStageResource(0, a);
    renderer = AsWView(g_view)->renderTarget;
    a = ((void* (*)(void*, void*))(*(void***)renderer)[0x74 / 4])(renderer, object);
    Engine_SetTexturePalette(0, a);

    for (int i = 0; i < kQuadVertexCount; ++i) {
        unsigned char limit = (unsigned char)(int)AsCBallTrack(p)->opacity;
        BallTrackTex* t = (*(BallTrackTex**)&AsCBallTrack(p)->quadRenderer);
        if (t->count > (unsigned char)i)
            ((void (*)(void*, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char))t->vt[5])(t, (unsigned char)i, limit, 255, 255, 255);
    }

    // Phase 3: join consecutive WORLD-space edge pairs. The quad ABI stores a count at +0,
    // vtable at +4: slots 3/5/6/8 begin, set color, set position, and draw respectively.
    int frame = 1;
    while (frame < AsCBallTrack(p)->trailPointCount + changed) {
        for (int channel = 0; channel < 4; ++channel) {
            int source = (channel & 2) ? frame : frame - 1;
            BallTrackPositionBits positionBits;
            if (channel & 1)
                positionBits = *(BallTrackPositionBits*)((*(char**)&AsCBallTrack(p)->trailPoints) + source * 0x18 + 0xc);
            else
                positionBits = *(BallTrackPositionBits*)((*(char**)&AsCBallTrack(p)->trailPoints) + source * 0x18);
            BallTrackTex* t = (*(BallTrackTex**)&AsCBallTrack(p)->quadRenderer);
            if (t->count > (unsigned char)channel)
// A 12-byte struct argument is passed by reference on the Wii; the callee reads it through a pointer.
                ((void (*)(void*, unsigned char, BallTrackPositionBits))t->vt[6])(t, (unsigned char)channel, positionBits);
        }
        BallTrackTex* t = (*(BallTrackTex**)&AsCBallTrack(p)->quadRenderer);
        unsigned char count = t->count;
        if (count != 0) {
            void* data = GX_GetPrimitiveCount(count, 0x98);
            ((void (*)(void*, int, void*))t->vt[8])(t, 0x98, data);
        }
        ++frame;
    }
}

struct TraceOwnedStorage {
    void* vtable;
    char reserved04[4];
    void* resource;
    char traceBaseStorage[0x108];
    void* ownedObject;
    void* allocatedArray;
    void* rowCountStorage;
    void* opacityStorage;
};

// Embedded CBallTrack virtual prefix. Raw vtable byte offsets include the two
// CodeWarrior metadata words: +0x0C starts/rebuilds the trail, +0x14 draws it.
// Concrete targets: CBallTrack_InitTrailPoints / CBallTrack_Draw,
// paired by __vt__10CBallTrack in phase1_class_tables.cpp. No separate allocation.
struct WindRibbonComponent {
    virtual void unused0();
    virtual void start();
};
// Concrete WIND record, 0x26C bytes, 4-byte alignment. The common CBallTrace
// prefix ends at +0x118. Embedded CBallTrack occupies [0x11C,0x250); its owned
// bitmap is outer+0x124, renderer +0x230, edges +0x234, count +0x238. Its
// lifetime is in-place, bounded by the outer destructor; particles are a separate
// owned WindTrailParticle[256] allocation. See CBallWindTrack.h for live fields.
struct WindTrackSubmissionView {
    void* vtable; char traceState[0x110]; void* outerQuadRenderer;
    WindTrailParticle* particles;
    WindRibbonComponent ribbon;
    char ribbonState[0x130];
    float opacityScale;
    WVector previousCameraPosition, cameraMotionVector;
};
// Concrete COMET record, 0x324 bytes, 4-byte alignment: its inline edges are
// NOT wind particles or an embedded ribbon. +4/+8 are constructor-cleared
// inherited words; no consumer semantics are established for those seed words.
// StartTrail owns live index/clock/length/brightness initialization. material
// and bitmap are owned until CComet_dtor; the edge ring is inline, never freed.
struct CometConstructionView {
    void* vtable; int clearedBaseWord04, clearedBaseWord08;
    int trailCapacity, trailIndex; float sampleElapsed, sampleInterval;
    unsigned char active; char traceState[0xF7]; void* material;
    BallRibbonEdge edges[20]; float trailLength; char reserved2FC[4];
    float brightness; void* bitmap; int hasPendingTrailEdge; BallRibbonEdge pendingEdge;
};
typedef char WindSubmission_size[sizeof(WindTrackSubmissionView) == 0x26C ? 1 : -1];
typedef char WindSubmission_ribbon[(int)&((WindTrackSubmissionView*)0)->ribbon == 0x11C ? 1 : -1];
typedef char CometConstruction_size[sizeof(CometConstructionView) == 0x324 ? 1 : -1];
typedef char CometConstruction_edges[(int)&((CometConstructionView*)0)->edges == 0x118 ? 1 : -1];

extern void* g_resourceManager;
extern void ResourceManager_ReleaseId(void*);
extern void __dla__FPv(void*);
extern void __dl__FPv(void*);
extern void* g_view;
extern const float kFloat_0_0_BallTrack;
extern const float kFloat_1_BallTrack;
extern const float kFloat_0_1_GeneTex3D;
extern double kDouble_SignedIntConversionBias_InventoryRows;
extern char __vt__6CComet[];
extern char ballTrackEffectStrings[];
extern char __vt__10CGeneTex3D[];
extern void* ResourceManager_LoadBitmap(void*, void*, int, int, int, int);
extern float WVectorLen__FRC7WVector(void*);
extern void WView_DrawLine(void*, void*, int, unsigned int, unsigned int, unsigned int);
extern void ResourceManager_ReleaseId(void*);
extern void* __nw__FUl(unsigned int);

extern char __vt__9CSkidMark[];
extern char s_ballTraceBaseVtable[];
extern char __vt__14CBallWindTrack[];
extern char __vt__10CBallTrack[];
extern void* GolfBall__Fv();
extern void* __nwa__FUl(unsigned int);
extern void* __construct_new_array(void*, void*, void*, unsigned int, unsigned int);
// SkidMarkRow_DefaultConstruct: public symbol spelling is retained for coordinator renaming.
void SkidMarkRow_DefaultConstruct();

struct VP4Pre_ { int pad; };
struct VP4Obj_ : VP4Pre_ { 
virtual void destroy(int);
};
// Destroy wind particles, the embedded ball ribbon and the outer quad renderer.
// Positive deleteFlag also frees self; return the original address. The embedded object is
// destroyed in place, never deleted separately. See WindTrackOwnedStorage for the ownership map.
void* CBallWindTrack_dtor(void* self, int deleteFlag) {
    if (self != 0) {
        char* p = (char*)self;
        *(void**)p = __vt__14CBallWindTrack;
        // Phase 1: destroy the OUTER wind-particle array, not a ribbon trailPoints array.
        void* a = AsCBallWindTrack(p)->particles;
        if (a != 0) {
            if (a != 0) {
                __dla__FPv((char*)a - 0x10);
            }
            AsCBallWindTrack(p)->particles = 0;
        }
        if (p + 0x11c != 0) {
            // +0x11C begins an embedded CBallTrack with its own vtable and bitmap.
            // Phase 2: release its bitmap, edge array and renderer in place.
            AsCBallTrack(p + 0x11c)->vtbl = __vt__10CBallTrack;
            void* b = AsCBallTrack(p + 0x11c)->bitmapResourceId;
            if (b != 0) {
                ((void(*)(void*,int))ResourceManager_ReleaseId)(g_resourceManager, (int)b);
                AsCBallTrack(p + 0x11c)->bitmapResourceId = 0;
            }
            a = ((WindTrackOwnedStorage*)p)->embeddedTrailPoints;
            if (a != 0) {
                if (a != 0) {
                    __dla__FPv((char*)a - 0x10);
                }
                ((WindTrackOwnedStorage*)p)->embeddedTrailPoints = 0;
            }
        if (p + 0x11c != 0) {
            *(void**)(p + 0x11c) = s_ballTraceBaseVtable;
            a = ((WindTrackOwnedStorage*)p)->embeddedQuadRenderer;
            if (a != 0) {
                if (a != 0) {
                    ((VP4Obj_*)a)->destroy(1);
                }
                ((WindTrackOwnedStorage*)p)->embeddedQuadRenderer = 0;
            }
        }
        }
        if (p != 0) {
            *(void**)p = s_ballTraceBaseVtable;
            // Phase 3: release the OUTER common trace renderer, then optionally the whole object.
            a = AsCBallTrace(p)->quadRenderer;
            if (a != 0) {
                if (a != 0) {
                    ((VP4Obj_*)a)->destroy(1);
                }
                AsCBallTrace(p)->quadRenderer = 0;
            }
        }
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

struct Fn8020366CRow {
    char d[0x1c];
    // Leave skid-row storage uninitialized until the terrain projection writes it.
    Fn8020366CRow() {}
    // Allocate skid-row arrays through the game allocator; the compiler adds the array cookie.
    static void* operator new[](size_t size) { return __nwa__FUl(size); }
    // Release the allocation passed by the compiler's array-delete path.
    static void operator delete[](void* p) { __dla__FPv(p); }
};

struct SkidQuadVtable {
    void* v[7];
    void (*f1c)(void*, unsigned int, float, float);
};

struct SkidQuadTexture {
    unsigned char count;
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void f1c(unsigned int, float, float);
};

struct SkidMarkInitStorage {
    char pad00[0x1c];
    unsigned char ready;
    char pad1d[0xf7];
    SkidQuadTexture* tex;
    Fn8020366CRow* rows;
    int rowCount;
    int field120;
    int first;
    int last;
    int field12c;
};

// Clamp the skid history range, allocate rows and initialize quad texture coordinates.
extern "C" void CSkidMark_AllocateFrameRange(SkidMarkInitStorage* self) {
    if (self->first < ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryPlaybackSample])
        self->first = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryPlaybackSample];
    if (self->last < 0 || self->last > ((CGolfBall*)GolfBall__Fv())->m_preDataCount)
        self->last = ((CGolfBall*)GolfBall__Fv())->m_preDataCount;
    self->rowCount = self->last - self->first + 1;
    if (self->rows != 0) {
        delete[] self->rows;
        self->rows = 0;
    }
    int n = self->rowCount;
    if (n > 0)
        self->rows = new Fn8020366CRow[n];
    self->field120 = 0;
    self->field12c = self->first - 1;
    int i = 0;
    do {
        SkidQuadTexture* obj = self->tex;
        if (obj->count > (unsigned char)i) {
            float y = (i & 1) ? kFloat_0_0_BallTrack : kFloat_1_BallTrack;
            float x = (i & 2) ? kFloat_1_BallTrack : kFloat_0_0_BallTrack;
            obj->f1c((unsigned char)i, x, y);
        }
        ++i;
    } while (i < kQuadVertexCount);
    self->ready = 1;
}

// Empty constructor hook for skid-mark array elements.
void SkidMarkRow_DefaultConstruct() {}

typedef Vec3f SkV3;
typedef SkV3 SkXf;
struct SkRow {
    SkV3 left;
    SkV3 right;
    unsigned char alpha;
    char pad19[3];
};

struct SkidMark {
    char pad00[0x1c];
    unsigned char active;
    char pad1d[0xf7];
    void* tex;
    SkRow* rows;
    int maxRows;
    int curRow;
    int minFrame;
    int endFrame;
    int lastFrame;

    float distance(int idx);
    void addPoint(SkV3* key, int idx, SkXf* xf);
};

// Measure the distance between two consecutive ball-history positions.
// idx>=1 and both idx/idx-1 must name allocated history samples of the live ball.
// Returns world distance; does not change history, rows or counters. No bounds check.
inline float SkidMark::distance(int idx) {
    SkV3 c = *(SkV3*)PreData__9CGolfBallFi(GolfBall__Fv(), idx);
    SkV3 d = *(SkV3*)PreData__9CGolfBallFi(GolfBall__Fv(), idx - 1);
    SkV3 delta;
    __mi__FRC7WVectorRC7WVector(&delta, &c, &d);
    return (*(WVector*)&delta).Magnitude();
}

// Project a ribbon edge onto terrain and derive opacity from spacing and row position.
// Requires writable rows[maxRows], maxRows>0 and 0<=curRow<maxRows. key and xf
// borrow readable world float3s (center and half-width) for this call; copy key
// before writing rows. Ground tree must be live. idx>=0; idx>=1 with valid idx
// and idx-1 history is required only when curRow>0 takes the distance branch.
// curRow>0 also requires initialized rows[curRow-1].alpha. Writes exactly the
// current row's edges/alpha, leaving curRow unchanged (caller advances it).
// maxRows is a positive divisor in the fallback opacity ramp, not just capacity.
inline void SkidMark::addPoint(SkV3* key, int idx, SkXf* xf) {
    SkV3 a = *key;
    SkV3 b = a;
    SkV3 blend;
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&blend, CQuadTree_GetInstance(), &b, 1, 0, 1);
    a.y = blend.y;
    SkV3 first;
    __pl__FRC7WVectorRC7WVector(&first, &a, xf);
    (*(WVector*)&rows[curRow].left) = *(WVector*)&first;
    SkV3 second;
    __mi__FRC7WVectorRC7WVector(&second, &a, xf);
    (*(WVector*)&rows[curRow].right) = *(WVector*)&second;
    int row = curRow;
    unsigned char alpha;
    if (row > 0 && idx > 0 && distance(idx) < kSkidMarkNearSampleDistance) {
        int v = (int)(kSkidMarkNearbyAlphaMultiplier * (double)rows[row - 1].alpha);
        int clamped = 255;
        if (v < 255) clamped = v;
        alpha = (unsigned char)clamped;
    } else {
        alpha = (unsigned char)(int)(kSkidMarkBaseOpacity * ((float)(maxRows - row) / (float)maxRows));
    }
    rows[curRow].alpha = alpha;
}


// Skin/skid mark row emission.
// Emit terrain-projected skid rows from new history samples and maintain the live endpoint.
// Active objects require the same row-storage/divisor invariant as addPoint:
// maxRows>0, allocated rows[maxRows], 0<=curRow<=maxRows. Populated rows before
// curRow are initialized; minFrame/endFrame/lastFrame and the playback cursor
// name the loaded history (lastFrame==-1 permits first sample zero). Every
// traversed i is nonnegative; i-1 is read only after the explicit i>0 test.
// Capacity equality suppresses writes. No allocation or range repair occurs here.
void CSkidMark_UpdateFromBallHistory(SkidMark* self) {
    // Phase 1: compute a world-space half-width from up cross current history velocity.
    // One half-width is reused across this update's rows; Y is replaced by terrain height.
    struct Copy { unsigned int words[3]; };
    SkXf xf;
    SkV3 a, a2, pos, blend, b, first, second;
    SkV3 blend2, b2, first2, second2, delta, d, c, delta2, d2, c2;
    int v;
    if (!self->active) return;
    int frame = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryPlaybackSample];
    if (frame < self->minFrame) return;
    pos.x = kFloat_0_0_BallTrack;
    pos.y = kFloat_1_BallTrack;
    pos.z = kFloat_0_0_BallTrack;
    Vector_CrossProduct(&xf, &pos, &((BallHistorySample*)PreData__9CGolfBallFi(GolfBall__Fv(), frame))->velocity);
    (*(WVector*)&xf).Normalize();
    *(WVector*)&xf *= kBallRibbonRadiusMultiplier * ((CGolfBall*)GolfBall__Fv())->m_radius;
    // Phase 2: emit all newly reached history rows within capacity, with spacing-based opacity.
    for (int i = self->lastFrame + 1; i <= frame; ++i) {
        if (self->curRow < self->maxRows) {
            SkV3* key = (SkV3*)PreData__9CGolfBallFi(GolfBall__Fv(), i);
            *(Copy*)&a = *(Copy*)key;
            *(Copy*)&b = *(Copy*)&a;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(&blend, CQuadTree_GetInstance(), &b, 1, 0, 1);
            a.y = blend.y;
            __pl__FRC7WVectorRC7WVector(&first, &a, &xf);
            (*(WVector*)&self->rows[self->curRow].left) = *(WVector*)&first;
            __mi__FRC7WVectorRC7WVector(&second, &a, &xf);
            (*(WVector*)&self->rows[self->curRow].right) = *(WVector*)&second;
            int row = self->curRow;
            unsigned char alpha;
            // Nearby rows use min(255, previous alpha * 1.1), preserving the original truncation.
            do {
                if (row > 0 && i > 0) {
                    *(Copy*)&c = *(Copy*)PreData__9CGolfBallFi(GolfBall__Fv(), i);
                    *(Copy*)&d = *(Copy*)PreData__9CGolfBallFi(GolfBall__Fv(), i - 1);
                    __mi__FRC7WVectorRC7WVector(&delta, &c, &d);
                    if ((*(WVector*)&delta).Magnitude() < kSkidMarkNearSampleDistance) {
                        v = (int)(kSkidMarkNearbyAlphaMultiplier * (double)self->rows[row - 1].alpha);
                        alpha = (unsigned char)(v < 255 ? v : 255);
                        break;
                    }
                }
                alpha = (unsigned char)(int)(kSkidMarkBaseOpacity * ((float)(self->maxRows - row) / (float)self->maxRows));
            } while (0);
            self->rows[self->curRow].alpha = alpha;
            self->curRow++;
        }
    }
    // Phase 3: save the playback position and maintain an uncounted live endpoint if room remains.
    self->lastFrame = frame;
    if (frame < self->endFrame && self->curRow < self->maxRows) {
        SkV3* key = (SkV3*)GolfBall__Fv();
        *(Copy*)&a2 = *(Copy*)key;
        *(Copy*)&b2 = *(Copy*)&a2;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&blend2, CQuadTree_GetInstance(), &b2, 1, 0, 1);
        a2.y = blend2.y;
        __pl__FRC7WVectorRC7WVector(&first2, &a2, &xf);
        (*(WVector*)&self->rows[self->curRow].left) = *(WVector*)&first2;
        __mi__FRC7WVectorRC7WVector(&second2, &a2, &xf);
        (*(WVector*)&self->rows[self->curRow].right) = *(WVector*)&second2;
        int row = self->curRow;
        int i = self->lastFrame;
        unsigned char alpha;
        do {
            if (row > 0 && i > 0) {
                *(Copy*)&c2 = *(Copy*)PreData__9CGolfBallFi(GolfBall__Fv(), i);
                *(Copy*)&d2 = *(Copy*)PreData__9CGolfBallFi(GolfBall__Fv(), i - 1);
                __mi__FRC7WVectorRC7WVector(&delta2, &c2, &d2);
                if ((*(WVector*)&delta2).Magnitude() < kSkidMarkNearSampleDistance) {
                    v = (int)(kSkidMarkNearbyAlphaMultiplier * (double)self->rows[row - 1].alpha);
                    alpha = (unsigned char)(v < 255 ? v : 255);
                    break;
                }
            }
            alpha = (unsigned char)(int)(kSkidMarkBaseOpacity * ((float)(self->maxRows - row) / (float)self->maxRows));
        } while (0);
        self->rows[self->curRow].alpha = alpha;
    }
}
// Empty constructor hook for wind-particle array elements.
void WindTrailParticle_DefaultConstruct() {}

typedef Vec3f WindVector;
struct WindVectorBits { int words[3]; };
struct EmbeddedBallTrackHooks {
    virtual void v0();
    virtual void v1();
    virtual void v2();
};

// Move and respawn wind streaks around the camera until the ball's event midpoint.
// This update uses world-space CAMERA displacement, not elapsed time or physical wind.
// cameraMotionVector stores displacement, normalized only at length >= 1 world unit; smaller motion defers particles.
// Borrows the ball/camera; owns neither. The 256-slot array is wind-owned. Each
// respawn consumes RNG in Z,Y,X,length order; changing iteration changes randomness.
// Camera displacements below one unit are discarded by updating previousCameraPosition,
// not accumulated for a later step. Preserve this behavior when changing cadence.
void CBallWindTrack_UpdateCameraStreaks(void* self) {
    char* p = (char*)self;
    ((EmbeddedBallTrackHooks*)(p + kEmbeddedBallRibbonOffset))->v2();
    if (!AsCBallWindTrack(p)->active)
        return;
    if (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryEffectGate1] > 0) {
        if (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryPlaybackSample] > (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryEffectGate1] + ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryWindMidpointEnd2]) / 2) {
            AsCBallWindTrack(p)->active = 0;
            return;
        }
    }

    WindVectorBits ballVelocityBits;
    WindVector delta, cameraToParticle, ballToParticle, cameraDistanceVector, halfCameraStep, fullCameraStep, streakEnd, streakVector, spawnPosition, spawnCenter, forwardSpawnOffset, spawnOffset, randomUnitOffset;
    float randomY, randomZ, meanStreakLength, streakLengthJitter, alphaMax, one, opacityDistance, cullDistance, zero, cameraStepLength;
    int offset, index;
    // Phase 1: after embedded-ribbon/history gates, measure camera displacement.
    __mi__FRC7WVectorRC7WVector(&delta, s_cameraTransform + kCameraPositionOffset, (p + kPreviousCameraOffset));
    *(WVector*)((p + kCameraMotionVectorOffset)) = *(WVector*)&delta;
    cameraStepLength = WVectorLen(*(WVector*)((p + kCameraMotionVectorOffset)));
    if (cameraStepLength < kFloat_1_BallTrack) {
        (*(WVector*)(p + kPreviousCameraOffset)) = *(WVector*)(s_cameraTransform + kCameraPositionOffset);
        return;
    }
    (*(WVector*)((p + kCameraMotionVectorOffset))).Normalize();
    zero = kFloat_0_0_BallTrack;
    index = 0;
    cullDistance = kFloat_160_BallTrack;
    offset = 0;
    opacityDistance = kFloat_200_BallTrack;
    one = kFloat_1_BallTrack;
    alphaMax = kFloat_128_BallTrack;
    streakLengthJitter = kFloat_30_BallWindTrack;
    meanStreakLength = kWindTrackMeanStreakLength;
    // Phase 2: move active streaks by half the camera displacement; respawn inactive slots
    // ahead of the camera with random +/-75 offsets. Distant particles deactivate for this pass.
    for (; index < kWindParticleCount; ++index, offset += sizeof(WindTrailParticle)) {
        if (*(unsigned char*)((*(char**)&AsCBallWindTrack(p)->particles) + offset + kWindParticleActiveOffset)) {
            __mi__FRC7WVectorRC7WVector(&cameraToParticle, (*(char**)&AsCBallWindTrack(p)->particles) + offset, s_cameraTransform + kCameraPositionOffset);
            if (!((*(WVector*)&cameraToParticle * *(WVector*)(s_cameraTransform + kCameraForwardOffset)) < zero)) {
                __mi__FRC7WVectorRC7WVector(&ballToParticle, (*(char**)&AsCBallWindTrack(p)->particles) + offset, GolfBall__Fv());
                if (WVectorLen(*(WVector*)&ballToParticle) > cullDistance) {
                    *(unsigned char*)((*(char**)&AsCBallWindTrack(p)->particles) + offset + kWindParticleActiveOffset) = 0;
                    continue;
                }
            }
            __mi__FRC7WVectorRC7WVector(&cameraDistanceVector, (*(char**)&AsCBallWindTrack(p)->particles) + offset, s_cameraTransform + kCameraPositionOffset);
            float amount = WVectorLen(*(WVector*)&cameraDistanceVector) / opacityDistance;
            if (amount > one)
                amount = one;
            *(int*)((*(char**)&AsCBallWindTrack(p)->particles) + offset + kWindParticleOpacityOffset) = (int)(alphaMax * amount);
            __ml__FRC7WVectorf(&fullCameraStep, (p + kCameraMotionVectorOffset), cameraStepLength);
            __ml__FRC7WVectorf(&halfCameraStep, &fullCameraStep, kTrajectorySharedHalf);
            *(WVector*)((*(char**)&AsCBallWindTrack(p)->particles) + offset) -= *(WVector*)&halfCameraStep;
            __ml__FRC7WVectorf(&streakVector, (p + kCameraMotionVectorOffset), *(float*)((*(char**)&AsCBallWindTrack(p)->particles) + offset + kWindParticleLengthOffset));
            __mi__FRC7WVectorRC7WVector(&streakEnd, (*(char**)&AsCBallWindTrack(p)->particles) + offset, &streakVector);
            *(WVector*)((*(char**)&AsCBallWindTrack(p)->particles) + offset + kWindParticleEndOffset) = *(WVector*)&streakEnd;
        } else {
            randomZ = Random_RangeFloat(-1, kNegativeUnit, kFloat_1_BallTrack);
            randomY = Random_RangeFloat(-1, kNegativeUnit, kFloat_1_BallTrack);
            float x = Random_RangeFloat(-1, kNegativeUnit, kFloat_1_BallTrack);
            randomUnitOffset.x = x;
            randomUnitOffset.y = randomY;
            randomUnitOffset.z = randomZ;
            __ml__FfRC7WVector(&spawnOffset, &randomUnitOffset, kFloat_75_BallWindTrack);
            __ml__FfRC7WVector(&forwardSpawnOffset, (p + kCameraMotionVectorOffset), kWindTrackMeanStreakLength);
            __pl__FRC7WVectorRC7WVector(&spawnCenter, s_cameraTransform + kCameraPositionOffset, &forwardSpawnOffset);
            __pl__FRC7WVectorRC7WVector(&spawnPosition, &spawnCenter, &spawnOffset);
            *(WVector*)((*(char**)&AsCBallWindTrack(p)->particles) + offset) = *(WVector*)&spawnPosition;
            *(float*)((*(char**)&AsCBallWindTrack(p)->particles) + offset + kWindParticleLengthOffset) =
                streakLengthJitter * Random_RangeFloat(-1, kNegativeUnit, kFloat_1_BallTrack) + meanStreakLength;
            *(unsigned char*)((*(char**)&AsCBallWindTrack(p)->particles) + offset + kWindParticleActiveOffset) = 1;
            *(int*)((*(char**)&AsCBallWindTrack(p)->particles) + offset + kWindParticleOpacityOffset) = 0;
        }
    }
    // Phase 3: derive opacity from normalized ball-velocity Y, then save camera position.
    // A negative opacityScale is possible; drawing gates on opacityScale > 0 instead of clamping it here.
    WindVectorBits* ballVelocity = (WindVectorBits*)(&((CGolfBall*)GolfBall__Fv())->m_velocity);
    ballVelocityBits = *ballVelocity;
    (*(WVector*)&ballVelocityBits).Normalize();
    float normalizedVelocityY = *(float*)&ballVelocityBits.words[1];
    float opacityScale;
    if (normalizedVelocityY > kFloat_0_0_BallTrack)
        opacityScale = kFloat_1_BallTrack;
    else
        opacityScale = kTrajectorySharedHalf + normalizedVelocityY;
    AsCBallWindTrack(p)->opacityScale = opacityScale;
    (*(WVector*)(p + kPreviousCameraOffset)) = *(WVector*)(s_cameraTransform + kCameraPositionOffset);
}

// Start the embedded ribbon and clear every wind-particle active byte.
// Reset bounds: 32 batches * 8 entries = 256 records, each 0x24 bytes; active is at +0x20.
void CBallWindTrack_StartAndResetParticles(WindTrackSubmissionView* self) {
    // Starting the embedded ribbon precedes resetting outer particles.
    self->ribbon.start();
    AsCBallWindTrack(self)->active = 1;
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 8; ++j)
            *((unsigned char*)AsCBallWindTrack(self)->particles + i * 0x120 + j * sizeof(WindTrailParticle) + kWindParticleActiveOffset) = 0;
    }
}

struct V3Call_ { 
virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
};
// Draw active wind streaks using their stored opacity and the track's global opacity scale.
void CBallWindTrack_DrawRibbonAndStreaks(WindTrackSubmissionView* self) {
    // Draw the embedded ball ribbon even when the outer streak opacity gate is closed.
    ((V3Call_*)&self->ribbon)->s3();
    if (AsCBallWindTrack(self)->active && AsCBallWindTrack(self)->opacityScale > kFloat_0_0_BallTrack && kFloat_0_0_BallTrack != WVectorLen((*(WVector*)AsCBallWindTrack(self)->cameraMotionVector))) {
        for (int i = 0; i < kWindParticleCount; ++i) {
            char* entry = (char*)AsCBallWindTrack(self)->particles + i * (int)sizeof(WindTrailParticle);
            if (((WindTrailParticle*)entry)->active) {
                int value = ((WindTrailParticle*)entry)->opacity;

                unsigned int color = ((unsigned int)((int)(AsCBallWindTrack(self)->opacityScale * (float)value)) << 24) | kOpaqueWhiteRgb;
                WView_DrawLine(g_view, entry, (int)color, (unsigned int)(entry + kWindParticleEndOffset), 0u, 0x20800000u);
            }
        }
    }
}

// Create the comet quad renderer and bitmap, then initialize trail capacity and sample interval.
// Required fresh 0x324-byte storage. StartTrail must initialize live counters/
// edges before Update/Draw; construction alone does not make a running trail.
// This owns bitmap/material until CComet_dtor. No allocation failure recovery
// is implemented before renderer dereferences; preserve lifecycle/call order.
void* CComet_Ctor(CometConstructionView* self) {
    void* p;
    float sampleIntervalSeconds;
    int i;
    unsigned char* quadRenderer;
    self->clearedBaseWord04 = 0;
    self->clearedBaseWord08 = 0;
    AsCComet(self)->active = 0;
    *((&AsCComet(self)->material)) = 0;
    *(void**)self = __vt__6CComet;
    AsCComet(self)->bitmap = ResourceManager_LoadBitmap(g_resourceManager, ballTrackEffectStrings + kCometBitmap, 0, 0, 0, (int)__vt__6CComet);
    p = new CGeneTex3D;
    *((&AsCComet(self)->material)) = p;
    if (((unsigned char*)p)[0] != 4) {
        ((unsigned char*)p)[0] = (unsigned char)((CGeneTex3D*)p)->resize(4);
    }
    sampleIntervalSeconds = kFloat_0_1_GeneTex3D;
    AsCComet(self)->trailCapacity = kCometInitialTrailCapacity;
    i = 0;
    AsCComet(self)->sampleInterval = sampleIntervalSeconds;
    while (i < kQuadVertexCount) {
        quadRenderer = (unsigned char*)*((&AsCComet(self)->material));
        if (quadRenderer[0] > (unsigned char)i) {
            ((CGeneTex3D*)quadRenderer)->color((unsigned char)i, 255, 255, 255, 255);
        }
        ++i;
    }
    return self;
}

struct Dv4B { char pad[4]; };
class Dv4 : public Dv4B { public: 
virtual void v08(int);
};
// Release the skid bitmap, rows and quad renderer; optionally free the skid object.
void* CSkidMark_dtor(void* self, int deleteFlag) {
    if (self != 0) {
        char* p = (char*)self;
        void* res = AsCSkidMark(p)->textureResource;
        *(void**)p = __vt__9CSkidMark;
        if (res != 0) {
            ((void (*)(void*, int))ResourceManager_ReleaseId)(g_resourceManager, (int)res);
            AsCSkidMark(p)->textureResource = 0;
        }
        void* arr = AsCSkidMark(p)->trailPoints;
        if (arr != 0) {
            if (arr != 0) {
                __dla__FPv((char*)arr - 0x10);
            }
            AsCSkidMark(p)->trailPoints = 0;
        }
        (*(void**)&AsCSkidMark(p)->pointCapacity) = 0;
        (*(void**)&AsCSkidMark(p)->pointCount) = 0;
        if (p != 0) {
            *(void**)p = s_ballTraceBaseVtable;
            void* owned = AsCSkidMark(p)->quadRenderer;
            if (owned != 0) {
                if (owned != 0) {
                    ((Dv4*)owned)->v08(1);
                }
                AsCSkidMark(p)->quadRenderer = 0;
            }
        }
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

} // extern "C"

} // namespace ns1

// ---- merged from ball_trajectory_effects.cpp ----
namespace ns2 {

// Translation unit 0x80204958-0x80205FCC (14 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern char __vt__9CPuttLine[];
extern char __vt__9CDoughnut[];
extern char __vt__6CComet[];
extern char s_ballTraceBaseVtable[];
extern void* g_resourceManager;
extern void* g_sharedDoc;
void ResourceManager_ReleaseId(void*);
void __dl__FPv(void*);
void __dla__FPv(void*);
extern float kFloat_1_BallTrack;
extern const double kDouble_SignedIntConversionBias_InventoryRows;
extern const float kFloat_0_0_BallTrack;
extern const float kFloat_0_45_PuttLine;
extern const float kFullOpacity255;
extern const float kFloat_2_TrajectoryShared;
extern const float kFloat_0_8_Doughnut;
extern const float kFloat_neg0_Comet;
extern const float kTrajectorySharedHalf;
extern const float kCometTimeStepBrightnessScale;
extern const float kCometBrightnessDecayRate;
extern const float kFloat_0_00001_Comet;
extern char s_cameraTransform[];
void* GolfBall__Fv();
void* __nwa__FUl(unsigned int);
void Vector_CrossProduct(void*, void*, void*);
void* Normalize__7WVectorFv(void*);
void __ml__FRC7WVectorf(void*, void*, float);
void __ml__FfRC7WVector(void*, void*, float);
void __apl__7WVectorFRC7WVector(void*, void*);
void __as__7WVectorFRC7WVector(void*, void*);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void Matrix_BuildCameraTransform(void*, void*, void*);
void Matrix_CopyAffine(void*, void*);
void __amu__7WVectorFf(void*, float);
void* PreData__9CGolfBallFi(void*, int);
unsigned int SharedDoc_GetCourseId();
unsigned int GetGroundType__9CQuadTreeF7WVector(void*, void*);
void* CQuadTree_GetInstance();
void Engine_SetRenderStateList(void*, int);
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetTextureStageParameterList(int, void*, int);
void Engine_SetCachedRenderState(int, int);
void* Camera_GetRenderState(void*);
void Engine_SetType7Resource(int);
void Engine_SetTextureStageResource(int, void*);
void Engine_SetTexturePalette(int, void*);
int GX_GetPrimitiveCount(int, int);
void SetTransform(int, void*);
extern void* g_view;
extern const float kNegativeUnit;
struct CometGlobals;
extern CometGlobals cometRenderStatePreset;

// Start the comet trail with one camera-facing ribbon edge at the ball.
// Required constructed comet/ball/camera; no allocation occurs. Resets brightness
// to 255, stored edge count to one and timer/index to zero; sampleInterval persists.
void CComet_StartTrail(void* self) {
    // World half-width = normalize(ball velocity cross camera forward) * 2 * ballRadius.
    AsCComet(self)->trailLength = kFloat_1_BallTrack;
    AsCComet(self)->hasPendingTrailEdge = 0;
    AsCComet(self)->trailIndex = 0;
    AsCComet(self)->brightness = kFullOpacity255;
    AsCComet(self)->sampleElapsed = kFloat_0_0_BallTrack;
    (*(char*)&AsCComet(self)->active) = 1;
    void* ball = GolfBall__Fv();
    WVector halfWidth;
    Vector_CrossProduct(&halfWidth, &((CGolfBall*)ball)->m_velocity, (char*)&s_cameraTransform + kCameraForwardOffset);
    float ballRadius = ((CGolfBall*)GolfBall__Fv())->m_radius;
    void* normalizedSide = Normalize__7WVectorFv(&halfWidth);
    halfWidth = (*(WVector*)normalizedSide * kFloat_2_TrajectoryShared) * ballRadius;
    (*(WVector*)AsCComet(self)->edgeA) = *(WVector*)GolfBall__Fv() + halfWidth;
    (*(WVector*)AsCComet(self)->edgeB) = *(WVector*)GolfBall__Fv() - halfWidth;
}

// Write a ball-facing edge into the circular trail and advance its bounded length.
// trailCapacity must be 1..20 for the existing inline allocation; commits exactly
// one edge, wraps index modulo capacity and clears hasPendingTrailEdge. Position
// uses ball center +/- normalized(velocity cross cameraForward)*2*ballRadius.
void CComet_CommitTrailEdge(void* self) {
    // Side scratch reserves 24 bytes to preserve the original temporary layout;
    // vector APIs consume only XYZ. Keep normalize -> scale by 2 -> radius order.
    char halfWidth[0x18], radiusScaledSide[0xC], doubledUnitSide[0xC], edgeA[0xC], edgeB[0xC];
    void* ball = GolfBall__Fv();
    Vector_CrossProduct(halfWidth, &((CGolfBall*)ball)->m_velocity, (char*)&s_cameraTransform + kCameraForwardOffset);
    float ballRadius = ((CGolfBall*)GolfBall__Fv())->m_radius;
    void* normalizedSide = Normalize__7WVectorFv(halfWidth);
    __ml__FRC7WVectorf(doubledUnitSide, normalizedSide, kFloat_2_TrajectoryShared);
    __ml__FRC7WVectorf(radiusScaledSide, doubledUnitSide, ballRadius);
    __as__7WVectorFRC7WVector(halfWidth, radiusScaledSide);
    int nextIndex = AsCComet(self)->trailIndex + 1;
    int capacity = AsCComet(self)->trailCapacity;
    AsCComet(self)->trailIndex = nextIndex % capacity;
    __pl__FRC7WVectorRC7WVector(edgeA, GolfBall__Fv(), halfWidth);
    __as__7WVectorFRC7WVector((char*)self + (int)&((CComet*)0)->edgeA + AsCComet(self)->trailIndex * kRibbonEdgeBytes, edgeA);
    __mi__FRC7WVectorRC7WVector(edgeB, GolfBall__Fv(), halfWidth);
    __as__7WVectorFRC7WVector((char*)self + (int)&((CComet*)0)->edgeA + sizeof(WVector) + AsCComet(self)->trailIndex * kRibbonEdgeBytes, edgeB);
    if ((int)AsCComet(self)->trailLength < AsCComet(self)->trailCapacity) AsCComet(self)->trailLength = AsCComet(self)->trailLength + kFloat_1_BallTrack;
    AsCComet(self)->hasPendingTrailEdge = 0;
}

// Shrink/fade ring particles and spawn timed rings in a frame built from ball position/velocity.
// Required constructed owner with particleCount>0. dt is elapsed seconds;
// inactive owner returns immediately. A positive history gate disables the
// owner BEFORE existing particle decay/spawn. Particle size is not clamped;
// opacity<0 disables the slot. The strict spawn test retains timer remainder.
void CDoughnut_UpdateParticles(void* self, float dt) {
    // Phase 1: check the history gate, then fade opacity by 255*dt and shrink size by 0.8*dt.
    // dt is elapsed seconds; each particle stores its own world-space basis/position.
    if (!AsCDoughnut(self)->active) return;
    if (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryEffectGate1] > 0) {
        AsCDoughnut(self)->active = 0;
        return;
    }
    float alphaDecayPerSecond = kFullOpacity255;
    float sizeDecayPerSecond = kFloat_0_8_Doughnut;
    float zero = kFloat_0_0_BallTrack;
    int particleIndex = 0;
    int particleByteOffset = 0;
    while (particleIndex < AsCDoughnut(self)->particleCount) {
        unsigned char* item = (unsigned char*)AsCDoughnut(self)->particles + particleByteOffset;
        if (((DoughnutParticle*)item)->active) {
            ((DoughnutParticle*)item)->opacity -= alphaDecayPerSecond * dt;
            ((DoughnutParticle*)((char*)AsCDoughnut(self)->particles + particleByteOffset))->size -= sizeDecayPerSecond * dt;
            if ((*(volatile float*)&((DoughnutParticle*)item)->opacity) < zero) ((DoughnutParticle*)item)->active = 0;
        }
        particleByteOffset += kRingParticleBytes;
        ++particleIndex;
    }
    // Phase 2: spawn at most one ring per update; keep the timer remainder and wrap its slot.
    float t = AsCDoughnut(self)->spawnTimer + dt;
    AsCDoughnut(self)->spawnTimer = t;
    if (t > AsCDoughnut(self)->spawnInterval) {
        AsCDoughnut(self)->spawnTimer = t - AsCDoughnut(self)->spawnInterval;
        int idx = AsCDoughnut(self)->nextParticleIndex;
        int stride = idx * 0x3C;
        void* base = AsCDoughnut(self)->particles;
        unsigned char active = 1;
        float level = kFullOpacity255;
        float value = kFloat_1_BallTrack;
        *(unsigned char*)((char*)base + stride + 8) = active;
        *(float*)((char*)AsCDoughnut(self)->particles + stride + 4) = level;
        *(float*)((char*)AsCDoughnut(self)->particles + stride) = value;
        void* r30 = GolfBall__Fv();
        char a[0x30]; // Matrix_BuildCameraTransform builds a 3x4 matrix here
        char b[0x10];
        __pl__FRC7WVectorRC7WVector(b, GolfBall__Fv(), &((CGolfBall*)r30)->m_velocity);
        Matrix_BuildCameraTransform(a, GolfBall__Fv(), b);
        Matrix_CopyAffine((char*)AsCDoughnut(self)->particles + AsCDoughnut(self)->nextParticleIndex * 0x3C + 0xC, a);
        int n = AsCDoughnut(self)->nextParticleIndex + 1;
        int c = AsCDoughnut(self)->particleCount;
        AsCDoughnut(self)->nextParticleIndex = n - (n / c) * c;
    }
}

struct CometVector { int x, y, z; };
struct CometMaterial {
    unsigned char count;
    virtual void unused8();
    virtual void begin();
    virtual void unused10();
    virtual void color(unsigned char, unsigned char, unsigned char, unsigned char, unsigned char);
    virtual void position(unsigned char, CometVector);
    virtual void texcoord(unsigned char, float, float);
    virtual void draw(int, int);
};
struct CometState120 { int values[30]; };
struct CometState48 { int values[12]; };
struct CometState40 { int values[10]; };
struct CometGlobals { CometState120 state; CometState48 textureStageStates; CometState40 texture; };
struct CometDevice {
    virtual void unused08(); virtual void unused0C();
    virtual void unused10(); virtual void unused14(); virtual void unused18(); virtual void unused1C();
    virtual void unused20(); virtual void unused24(); virtual void unused28(); virtual void unused2C();
    virtual void unused30(); virtual void unused34(); virtual void unused38(); virtual void unused3C();
    virtual void unused40(); virtual void unused44(); virtual void unused48(); virtual void unused4C();
    virtual void unused50(); virtual void unused54(); virtual void unused58(); virtual void unused5C();
    virtual void unused60(); virtual void unused64(); virtual void unused68(); virtual void unused6C();
    virtual void* color(void*);
    virtual void* alpha(void*);
};
// Copy 15 render-state pairs through the original aggregate-copy boundary.
inline void CometCopy120(CometState120& dst, const CometState120& src) { dst = src; }
// Copy six texture-stage state pairs without changing their layout.
inline void CometCopy48(CometState48& dst, const CometState48& src) { dst = src; }
// Copy five texture-stage parameter pairs without changing their layout.
inline void CometCopy40(CometState40& dst, const CometState40& src) { dst = src; }

// Draw the comet's circular trail and optional unsaved head quad.
// Ring: +0x118 is trailCapacity entries of {world edgeA XYZ, world edgeB XYZ}, stride 24.
// trailIndex is the newest committed edge; trailLength is a float count truncated for iteration.
// trailEdgeA/B +0x30C/+0x318 are the live uncommitted head. material is comet-owned; the render
// device and bitmap are borrowed for drawing. Requires positive capacity and a valid material.
void CComet_DrawTrail(void* self) {
    // Phase 1: install preset state, world-to-view/projection matrices and bitmap resources.
    CometGlobals* globals = &cometRenderStatePreset;
    if (!AsCComet(self)->active) return;
    CometState120 state;
    CometCopy120(state, globals->state);
    CometState40 texture;
    CometState48 textureStageStates;
    CometCopy48(textureStageStates, globals->textureStageStates);
    CometCopy40(texture, globals->texture);
    Engine_SetRenderStateList(&state, kTrailPresetRenderPairs);
    Engine_SetTextureStageStateList(0, &textureStageStates, kTrailPresetStagePairs);
    Engine_SetTextureStageParameterList(0, &texture, kTrailPresetSamplerPairs);
    Engine_SetCachedRenderState(kTrailSourceBlendState, kTrailBlendSourceAlpha);
    Engine_SetCachedRenderState(kTrailDestinationBlendState, kTrailBlendOne);
    void* renderer = Camera_GetRenderState(g_view);
    SetTransform(kEffectViewTransform, ((EffectCameraMatrices*)renderer)->view);
    SetTransform(kEffectProjectionTransform, ((EffectCameraMatrices*)renderer)->projection);
    CometMaterial* material = (*(CometMaterial**)&AsCComet(self)->material);
    Engine_SetType7Resource(0);
    material->begin();
    void* bitmap = AsCComet(self)->bitmap;
    CometDevice* device = (*(CometDevice**)&AsWView(g_view)->renderTarget);
    void* textureStageResource = device->color(bitmap);
    Engine_SetTextureStageResource(0, textureStageResource);
    device = (*(CometDevice**)&AsWView(g_view)->renderTarget);
    void* textureObject = device->alpha(bitmap);
    Engine_SetTexturePalette(0, textureObject);
    Engine_SetCachedRenderState(kTrailDepthWriteState, 0);
    Engine_SetCachedRenderState(kTrailDepthCompareState, kTrailCompareLessEqual);
    // Phase 2: emit committed quads newest-to-oldest. Corner bit 1 chooses the older edge;
    // bit 0 chooses its side. U increases with edge age, V is 1 on edgeA and 0 on edgeB.
    // Current brightness supplies the same alpha to all four corners of every segment.
    int olderEdgeFlag, headEdgeSide, edgeSide, headStoredEdgeFlag, segmentAge, headCorner, corner;
    segmentAge = 0;
    while (segmentAge < (int)AsCComet(self)->trailLength - 1) {
        for (corner = 0; corner < 4; ++corner) {
            unsigned char bright = (unsigned char)(int)AsCComet(self)->brightness;
            material = (*(CometMaterial**)&AsCComet(self)->material);
            if (material->count > (unsigned char)corner) {
                material->color((unsigned char)corner, bright, 0xFF, 0xFF, 0xFF);
            }
            olderEdgeFlag = corner & 2;
            int ringIndex;
            if (olderEdgeFlag) {
                int newestEdgeIndex = AsCComet(self)->trailIndex;
                int capacity = AsCComet(self)->trailCapacity;
                int newestMinusAge = newestEdgeIndex - segmentAge;
                int wrappedIndex = capacity + newestMinusAge - 1;
                ringIndex = wrappedIndex % capacity;
            } else {
                int newestEdgeIndex = AsCComet(self)->trailIndex;
                int capacity = AsCComet(self)->trailCapacity;
                int newestMinusAge = newestEdgeIndex - segmentAge;
                int wrappedIndex = capacity + newestMinusAge;
                ringIndex = wrappedIndex % capacity;
            }
            edgeSide = corner & 1;
            char* base = (char*)self + ringIndex * 0x18 + edgeSide * 0x0C;
            CometVector positionBits = *(CometVector*)(base + 0x118);
            material = (*(CometMaterial**)&AsCComet(self)->material);
            if (material->count > (unsigned char)corner) {
                material->position((unsigned char)corner, positionBits);
            }
            float textureU;
            if (olderEdgeFlag) {
                int storedEdgeCount = (int)AsCComet(self)->trailLength;
                int pendingEdgeCount = AsCComet(self)->hasPendingTrailEdge;
                textureU = (float)(segmentAge + pendingEdgeCount + 1) / (float)(pendingEdgeCount + storedEdgeCount);
            } else {
                int storedEdgeCount = (int)AsCComet(self)->trailLength;
                int pendingEdgeCount = AsCComet(self)->hasPendingTrailEdge;
                textureU = (float)(segmentAge + pendingEdgeCount) / (float)(pendingEdgeCount + storedEdgeCount);
            }
            material = (*(CometMaterial**)&AsCComet(self)->material);
            if (material->count > (unsigned char)corner) {
                float textureV;
                if (edgeSide) textureV = kFloat_0_0_BallTrack; else textureV = kFloat_1_BallTrack;
                material->texcoord((unsigned char)corner, textureU, textureV);
            }
        }
        material = (*(CometMaterial**)&AsCComet(self)->material);
        unsigned char vertexCount = material->count;
        if (vertexCount) {
            int primitiveCount = GX_GetPrimitiveCount(vertexCount, 0x98);
            material->draw(0x98, primitiveCount);
        }
        ++segmentAge;
    }
    // Phase 3: join the live head to the newest committed edge. U spans 0..1/(1+trailLength),
    // continuing the committed quads' pending-edge-adjusted U coordinates.
    if (AsCComet(self)->hasPendingTrailEdge == 0) return;
    float oneEdge = kFloat_1_BallTrack;
    float storedEdgeLength = AsCComet(self)->trailLength;
    if (!(storedEdgeLength >= oneEdge)) return;
    headCorner = 0;
    while (headCorner < 4) {
        headStoredEdgeFlag = headCorner & 2;
        if (headStoredEdgeFlag) {
            headEdgeSide = headCorner & 1;
            int newestEdgeIndex = AsCComet(self)->trailIndex;
            char* base = (char*)self + newestEdgeIndex * 0x18 + headEdgeSide * 0x0C;
            CometVector positionBits = *(CometVector*)(base + 0x118);
            material = (*(CometMaterial**)&AsCComet(self)->material);
            if (material->count > (unsigned char)headCorner) {
                material->position((unsigned char)headCorner, positionBits);
            }
        } else {
            headEdgeSide = headCorner & 1;
            char* base = (char*)self + headEdgeSide * 0x0C;
            CometVector positionBits = *(CometVector*)(base + 0x30C);
            material = (*(CometMaterial**)&AsCComet(self)->material);
            if (material->count > (unsigned char)headCorner) {
                material->position((unsigned char)headCorner, positionBits);
            }
        }
        float headTextureU;
        if (headStoredEdgeFlag) {
            float trailEdgeLength = AsCComet(self)->trailLength;
            headTextureU = oneEdge / (oneEdge + trailEdgeLength);
        } else {
            headTextureU = kFloat_0_0_BallTrack;
        }
        material = (*(CometMaterial**)&AsCComet(self)->material);
        if (material->count > (unsigned char)headCorner) {
            float headTextureV;
            if (headEdgeSide) headTextureV = kFloat_0_0_BallTrack; else headTextureV = kFloat_1_BallTrack;
            material->texcoord((unsigned char)headCorner, headTextureU, headTextureV);
        }
        ++headCorner;
    }
    material = (*(CometMaterial**)&AsCComet(self)->material);
    unsigned char vertexCount = material->count;
    if (vertexCount) {
        int primitiveCount = GX_GetPrimitiveCount(vertexCount, 0x98);
        material->draw(0x98, primitiveCount);
    }
}

// Empty constructor hook for ring-particle array elements.
void DoughnutParticle_DefaultConstruct() {}

// Sample the comet trail, shrink and fade it while descending, and maintain its live head edge.
// Required started comet with capacity 1..20; dt is elapsed seconds. The .5
// shrink aliases a shared scalar used by other effects (see file index), not
// an independent frame-rate constant. Negative-zero velocity boundary is kept.
// A sample crossing invokes the +18 commit callback and returns BEFORE updating
// the live head. Large dt still commits once and leaves excess in the timer.
void CComet_UpdateTrail(void* self, float dt) {
    // Descending phase: shrink trailLength by 0.5 PER UPDATE (not multiplied by dt), while
    // brightness and the sample timer use elapsed seconds. Preserve that original mixed scaling.
    if (AsCComet(self)->active) {
        if (((CGolfBall*)GolfBall__Fv())->m_velocity.y < kFloat_neg0_Comet) {
            float remainingEdges = AsCComet(self)->trailLength;
            float edgesRemovedPerCall = kTrajectorySharedHalf;
            float zero = kFloat_0_0_BallTrack;
            remainingEdges = remainingEdges - edgesRemovedPerCall;
            remainingEdges = (remainingEdges > zero) ? remainingEdges : zero;
            AsCComet(self)->trailLength = remainingEdges;
            float rate = kCometTimeStepBrightnessScale * AsCComet(self)->brightness;
            float step = rate * dt;
            step = (dt > step) ? step : dt;
            float value = AsCComet(self)->brightness - kCometBrightnessDecayRate * dt;
            float time = AsCComet(self)->sampleElapsed + step;
            AsCComet(self)->sampleElapsed = time;
            value = (zero < value) ? value : zero;
            AsCComet(self)->brightness = value;
            float absval = value;
            absval = (absval >= 0.0f) ? absval : -absval;
            if (absval < kFloat_0_00001_Comet) {
                AsCComet(self)->active = 0;
                AsCComet(self)->brightness = zero;
            }
        } else {
            AsCComet(self)->sampleElapsed += dt;
        }
        // Commit at most one timed edge per update; otherwise maintain the live head edge.
        float t = AsCComet(self)->sampleElapsed;
        if (t > AsCComet(self)->sampleInterval) {
            AsCComet(self)->sampleElapsed = t - AsCComet(self)->sampleInterval;
            (*(void (**)(void*))(*(char**)self + 0x18))(self);
            return;
        }
        char a[0x18];
        Vector_CrossProduct(a, &((CGolfBall*)GolfBall__Fv())->m_velocity, s_cameraTransform + 0x18);
        (*(WVector*)a).Normalize();
        *(WVector*)a *= kFloat_2_TrajectoryShared * ((CGolfBall*)GolfBall__Fv())->m_radius;

        WVector b = *(WVector*)(GolfBall__Fv()) + *(WVector*)a;
        (*(WVector*)AsCComet(self)->trailEdgeA) = b;
        WVector c = *(WVector*)(GolfBall__Fv()) - *(WVector*)a;
        (*(WVector*)AsCComet(self)->trailEdgeB) = c;
        AsCComet(self)->hasPendingTrailEdge = 1;
    }
}

// Empty constructor hook for putt-ribbon array elements.
void PuttRibbonSegment_DefaultConstruct() {}

// Quad renderer: count at +0; secondary vtable at +4. These helpers borrow the
// renderer and do not own vertices. Position takes a pointer to XYZ (unlike the
// comet renderer's by-value CometVector adapter). Slots are offsets / sizeof(void*).
// Begin submitting a textured particle quad through secondary-interface slot 3.
static inline void BeginParticleQuad(void* material) {
    (*(void (**)(void*))((char*)*(void**)((char*)material + 4) + 0xC))(material);
}
// Set one vertex to white with the stored particle brightness as its alpha byte.
static inline void SetParticleQuadOpacity(void* material, unsigned char corner, unsigned char brightness) {
    void* vtable = *(void**)((char*)material + 4);
    (*(void (**)(void*, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char))((char*)vtable + 5 * sizeof(void*)))(material, corner, brightness, 0xFF, 0xFF, 0xFF);
}
// Copy a world-space XYZ corner into the renderer through slot 6.
static inline void SetParticleQuadPosition(void* material, int corner, void* vertex) {
    (*(void (**)(void*, int, void*))((char*)*(void**)((char*)material + 4) + 0x18))(material, corner, vertex);
}
// Submit the prepared quad; the primitive-count word is carried in the pointer ABI slot.
static inline void DrawParticleQuad(void* material, int primitive, void* primitiveCount) {
    (*(void (**)(void*, int, void*))((char*)*(void**)((char*)material + 4) + 0x20))(material, primitive, primitiveCount);
}

// Draw each active ring particle as a textured quad using its stored basis and position.
void CDoughnut_DrawParticleQuads(void* self) {
    // Phase 1: install preset/camera state and bind the ring bitmap to the owned quad renderer.
    if (!AsCDoughnut(self)->active) return;
    int renderStates[0x1E];
    for (int i = 0; i < 0x1E; ++i) {
        renderStates[i] = *(int*)((char*)&cometRenderStatePreset + i * 4);
    }
    int stageStates[12];
    int samplerStates[10];
    for (int i = 0; i < 12; ++i) stageStates[i] = *(int*)((char*)&cometRenderStatePreset + 0x78 + i * 4);
    for (int i = 0; i < 10; ++i) samplerStates[i] = *(int*)((char*)&cometRenderStatePreset + 0xA8 + i * 4);
    Engine_SetRenderStateList(renderStates, kTrailPresetRenderPairs);
    Engine_SetTextureStageStateList(0, stageStates, kTrailPresetStagePairs);
    Engine_SetTextureStageParameterList(0, samplerStates, kTrailPresetSamplerPairs);
    Engine_SetCachedRenderState(kTrailSourceBlendState, kTrailBlendSourceAlpha);
    Engine_SetCachedRenderState(kTrailDestinationBlendState, kTrailBlendOne);
    void* cameraMatrices = Camera_GetRenderState(g_view);
    SetTransform(kEffectViewTransform, ((EffectCameraMatrices*)cameraMatrices)->view);
    SetTransform(kEffectProjectionTransform, ((EffectCameraMatrices*)cameraMatrices)->projection);
    void* material = AsCDoughnut(self)->material;
    Engine_SetType7Resource(0);
    BeginParticleQuad(material);
    void* bitmap = AsCDoughnut(self)->bitmapResource;
    void* device = AsWView(g_view)->renderTarget;
    void* textureStageResource = (*(void* (**)(void*, void*))((char*)*(void**)device + 0x70))(device, bitmap);
    Engine_SetTextureStageResource(0, textureStageResource);
    void* textureObject = (*(void* (**)(void*, void*))((char*)*(void**)device + 0x74))(device, bitmap);
    Engine_SetTexturePalette(0, textureObject);
    // Phase 2: for each active ring, build world corners = position + size/2 *
    // (signX * storedUp + signY * storedRight). UVs were initialized when the effect started.
    // The basis was captured at spawn, so it does not continuously turn toward the camera.
    float half = kTrajectorySharedHalf;
    int offset = 0;
    for (int index = 0; index < AsCDoughnut(self)->particleCount; ++index, offset += sizeof(DoughnutParticle)) {
        char* particle = (char*)AsCDoughnut(self)->particles + offset;
        if (!particle[kDoughnutActiveOffset]) continue;
        for (int corner = 0; corner < kQuadVertexCount; ++corner) {
            unsigned char count = *(*(unsigned char**)&AsCDoughnut(self)->material);
            int brightness = (int)*(float*)(particle + kDoughnutOpacityOffset);
            if (count > (unsigned char)corner) {
                SetParticleQuadOpacity(material, (unsigned char)corner, (unsigned char)brightness);
            }
            WVector vertex;
            vertex = *(WVector*)(particle + kDoughnutPositionOffset);
            float x = (corner & 1) ? kFloat_1_BallTrack : kNegativeUnit;
            float y = (corner & 2) ? kFloat_1_BallTrack : kNegativeUnit;
            WVector upContribution = x * *(WVector*)(particle + kDoughnutUpOffset);
            WVector rightContribution = y * *(WVector*)(particle + kDoughnutRightOffset);
            WVector cornerDirection = rightContribution + upContribution;
            WVector worldCornerOffset = (half * *(float*)(particle + kDoughnutSizeOffset)) * cornerDirection;
            vertex += worldCornerOffset;
            unsigned char n = *(*(unsigned char**)&AsCDoughnut(self)->material);
            if ((unsigned char)corner < n) {
                SetParticleQuadPosition(material, corner, &vertex);
            }
        }
        unsigned char n = *(*(unsigned char**)&AsCDoughnut(self)->material);
        if (n) {
            int primitiveCount = GX_GetPrimitiveCount(n, kQuadStripPrimitive);
            DrawParticleQuad(material, kQuadStripPrimitive, (void*)(long)primitiveCount);
        }
    }
}

// Allocate one putt-ribbon segment per history sample and offset its two edges by ball radius.
// Required constructed owner; replaces its segment array. History must contain
// preDataCount+1 accessible samples, including both endpoints. No texture dimensions
// or viewport conversion is involved; all positions remain world XYZ, Y up.
void CPuttLine_Init(void* self) { float scale = kFloat_0_45_PuttLine;
    // Phase 1: replace the owned segment array, including CodeWarrior's 16-byte array cookie.
    void* old = AsCPuttLine(self)->segments;
    if (old) {
        __dla__FPv((char*)old - 0x10);
        AsCPuttLine(self)->segments = 0;
    }
    int count = ((CGolfBall*)GolfBall__Fv())->m_preDataCount + 1;
    void* data = __nwa__FUl(count * 0x1C + 0x10);
    if (data) {
        data = __construct_new_array(data, (void*)PuttRibbonSegment_DefaultConstruct, (void*)0, (size_t)0x1C, (size_t)count);
    }
    AsCPuttLine(self)->active = 1;
    AsCPuttLine(self)->segments = data;
    float one = kFloat_1_BallTrack;
    float zero = kFloat_0_0_BallTrack;
    int i = 0;
    unsigned int offset = 0;
    // Phase 2: world half-width = normalize(up cross velocity) * 0.45 * ballRadius.
    // Lower both edges by one ball radius in world Y; one segment is reserved per sample.
    while (i <= ((CGolfBall*)GolfBall__Fv())->m_preDataCount) {
        WVector transformed;
        float pos[3] = {zero, one, zero};

         void* item = PreData__9CGolfBallFi(GolfBall__Fv(), i);
        Vector_CrossProduct(&transformed, pos, &((BallHistorySample*)item)->velocity);
        transformed.Normalize();
        transformed *= scale * ((CGolfBall*)GolfBall__Fv())->m_radius;
        WVector a = *(WVector*)(PreData__9CGolfBallFi(GolfBall__Fv(), i)) + transformed;
        ((PuttRibbonSegment*)((char*)AsCPuttLine(self)->segments + offset))->left = a;
        WVector b = *(WVector*)(PreData__9CGolfBallFi(GolfBall__Fv(), i)) - transformed;
        ((PuttRibbonSegment*)((char*)AsCPuttLine(self)->segments + offset))->right = b;
        ((PuttRibbonSegment*)((char*)AsCPuttLine(self)->segments + offset))->left.y -= ((CGolfBall*)GolfBall__Fv())->m_radius;
        ((PuttRibbonSegment*)((char*)AsCPuttLine(self)->segments + offset))->right.y -= ((CGolfBall*)GolfBall__Fv())->m_radius;
        i++;
        ((PuttRibbonSegment*)((char*)AsCPuttLine(self)->segments + offset))->colorOrOpacity = 0;
        offset += 0x1C;
    }
}

// Visibility domains, with source-grounded producers:
// - CGolfDoc+0x358 == 8 is kStateAim (golf_rule_round_flow_and_shot_sim.cpp); gauge cancel/return paths
//   restore shotPhase=idle, stop the gauge, send camera message 4, then write 8.
//   Exact equality here latches active=0; a combined bitmask containing 8 does
//   not. Init must reactivate the line; leaving aim alone does not reactivate it.
// - History+0x70 is copied from ball.m_state by simulation/sample writers
//   (com_shot_cpu_rival_ai.cpp ball record writer, guide_line_trajectory.cpp guide sampler), matching BallCollisionHistory in
//   course_collision_and_ball_physics.cpp: 0 stopped,1 airborne,2 spin-delay,3 rolling,4 in cup. It is NOT
//   ground type. In-cup history is hidden on every course; course ID 10 further
//   restricts to rolling history before querying the sample position's ground.
// - Live ground byte comes from GetGroundType (course_collision_and_ball_physics.cpp): triangle material,
//   with region overrides. 3=rough,4=bunker; 10 is also tested by high-speed
//   impact policy there, but its material name is unproven. Keep numeric 10
//   searchable rather than labeling it ice/water. Course 10's display name is
//   unnecessary to select it: the predicate uses SharedDoc_GetCourseId, not map text.
enum PuttVisibilityCodes {
    kPuttAimState = 8, // Equality test, not a bitmask test.
    kPuttHistoryInCup = 4,
    kPuttHistoryRolling = 3,
    kPuttGroundRough = 3, kPuttGroundBunker = 4
};
// Set putt opacity from age, history motion state and the live-ground exception.
// Ordinary opacity ramps 0..64 over playback samples [current-90,current-15]. Special value
// 0x100 selects the course/surface color in CPuttLine_Display (skidmark_ball.cpp):
// RGBA=(230,200,130,128), instead of white RGB with the low-byte alpha. It is
// a selector, not opacity256; truncating it to a byte would make it transparent.
// Display bridges adjacent left/right world edges, ignores depth comparison and
// never writes depth. Changing this selector's RGB/A belongs in Display; changing
// which surfaces receive it belongs in the predicate here.
// Mod lookup: change ground membership in the final groundType predicate, course
// membership in kPuttSpecialCourse, age/opacity in the named constants. Keep the
// in-cup and rolling tests distinct from ground IDs. Test aim reentry, combined
// state flags, stopped live ball (early return leaves previous segment values),
// hole-in samples, course 10 rolling on ground 3/4/10 versus other ground, ordinary
// courses and the collapsed ramp. This operation rewrites through playback index
// inclusive only; later segments retain their old values. It takes no delta time.
void CPuttLine_Process(void* self) {
    void* scene = g_sharedDoc;
    void* golfDoc = (*(void**)&AsCSharedDoc(scene)->golfDoc);
    if ((*(unsigned int*)&AsCGolfDoc(golfDoc)->gameStateFlags) == kPuttAimState) AsCPuttLine(self)->active = 0;
    if (!AsCPuttLine(self)->active) return;
    if (!AsCPuttLine(self)->segments) return;
    if (!((CGolfBall*)GolfBall__Fv())->m_state) return;
    // Phase 1: derive the age ramp in sample indices, with both ends clamped at zero.
    int fadeStartSample = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryPlaybackSample] - kPuttFadeHistoryStart;
    fadeStartSample = (fadeStartSample > 0) ? fadeStartSample : 0;
    int fadeEndSample = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryPlaybackSample] - kPuttFadeHistoryEnd;
    fadeEndSample = (fadeEndSample > 0) ? fadeEndSample : 0;
    int fadeSampleSpan = fadeEndSample - fadeStartSample;
    for (int i = 0; i <= ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistoryPlaybackSample]; ++i) {
        void* item = PreData__9CGolfBallFi(GolfBall__Fv(), i);
        // Phase 2: history motion admission precedes live-ground override.
        if (((BallHistorySample*)item)->motionState == kPuttHistoryInCup) { ((PuttRibbonSegment*)AsCPuttLine(self)->segments)[i].colorOrOpacity = 0; continue; }
        if ((unsigned char)SharedDoc_GetCourseId() == kPuttSpecialCourse) {
            item = PreData__9CGolfBallFi(GolfBall__Fv(), i);
            if (((BallHistorySample*)item)->motionState != kPuttHistoryRolling) { ((PuttRibbonSegment*)AsCPuttLine(self)->segments)[i].colorOrOpacity = 0; continue; }
            int worldPositionBits[3];
            item = PreData__9CGolfBallFi(GolfBall__Fv(), i);
            worldPositionBits[0] = *(int*)item; worldPositionBits[1] = *((int*)item + 1); worldPositionBits[2] = *((int*)item + 2);
            unsigned char groundType = GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), worldPositionBits);
            if (groundType == kPuttGroundRough || groundType == kPuttGroundBunker || groundType == kPuttGroundType10) { ((PuttRibbonSegment*)AsCPuttLine(self)->segments)[i].colorOrOpacity = kPuttSpecialSurfaceColorSelector; continue; }
        }
        // Phase 3: ordinary samples fade 0..64; precedence keeps a collapsed ramp
        // from dividing by zero (the <= start case wins when both ends are zero).
        if (i <= fadeStartSample) ((PuttRibbonSegment*)AsCPuttLine(self)->segments)[i].colorOrOpacity = 0;
        else if (i >= fadeEndSample) ((PuttRibbonSegment*)AsCPuttLine(self)->segments)[i].colorOrOpacity = kPuttMaxOpacity;
        else if (fadeEndSample == fadeStartSample) ((PuttRibbonSegment*)AsCPuttLine(self)->segments)[i].colorOrOpacity = 0;
        else ((PuttRibbonSegment*)AsCPuttLine(self)->segments)[i].colorOrOpacity = ((i - fadeStartSample) * kPuttMaxOpacity) / fadeSampleSpan;
    }
}

struct VP4Pre_ { int pad; };
struct VP4Obj_ : VP4Pre_ { 
virtual void destroy(int);
};
// Release putt segments and the polygon renderer; optionally free the putt-line object.
void* CPuttLine_dtor(void* self, int deleting) {
    if (self) {
        void* p = AsCPuttLine(self)->segments;
        *(void**)self = &__vt__9CPuttLine;
        if (p) {
            if (p) {
                __dla__FPv((char*)p - 0x10);
            }
            AsCPuttLine(self)->segments = 0;
        }
        if (self) {
            p = AsCPuttLine(self)->polygon;
            *(void**)self = &s_ballTraceBaseVtable;
            if (p) {
                if (p) {
                    ((VP4Obj_*)p)->destroy(1);
                }
                AsCPuttLine(self)->polygon = 0;
            }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Release ring particles, the bitmap and quad renderer; optionally free the ring effect.
void* CDoughnut_dtor(void* self, int deleting) {
    if (self) {
        void* p = AsCDoughnut(self)->particles;
        *(void**)self = &__vt__9CDoughnut;
        if (p) {
            if (p) {
                __dla__FPv((char*)p - 0x10);
            }
            AsCDoughnut(self)->particles = 0;
        }
        p = AsCDoughnut(self)->bitmapResource;
        if (p) {
            ((void(*)(void*,int))ResourceManager_ReleaseId)(g_resourceManager, (int)p);
            AsCDoughnut(self)->bitmapResource = 0;
        }
        if (self) {
            p = AsCDoughnut(self)->material;
            *(void**)self = &s_ballTraceBaseVtable;
            if (p) {
                if (p) {
                    ((VP4Obj_*)p)->destroy(1);
                }
                AsCDoughnut(self)->material = 0;
            }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Clear all ring particles, restart the spawn timer and set quad texture coordinates.
void CDoughnut_StartAndResetRingParticles(void* self) {
    class Channel {
    public:
        unsigned char count;
        virtual void slot0() DECOMP_LOCAL_VIRTUAL;
        virtual void slot1() DECOMP_LOCAL_VIRTUAL;
        virtual void slot2() DECOMP_LOCAL_VIRTUAL;
        virtual void slot3() DECOMP_LOCAL_VIRTUAL;
        virtual void slot4() DECOMP_LOCAL_VIRTUAL;
        virtual void set(unsigned char, float, float) DECOMP_LOCAL_VIRTUAL;
    };
    int i;
    for (i = 0; i < AsCDoughnut(self)->particleCount; ++i)
        *((char*)AsCDoughnut(self)->particles + i * 0x3C + 8) = 0;
    float zero = kFloat_0_0_BallTrack;
    int j;
    AsCDoughnut(self)->nextParticleIndex = j = 0;
    AsCDoughnut(self)->spawnTimer = zero;
    (*(char*)&AsCDoughnut(self)->active) = 1;
    for (; j < 4; ++j) {
        Channel* p = (*(Channel**)&AsCDoughnut(self)->material);
        if (p->count > (unsigned char)j) {
            float a = (j & 1) ? kFloat_0_0_BallTrack : kFloat_1_BallTrack;
            float b = (j & 2) ? kFloat_1_BallTrack : kFloat_0_0_BallTrack;
            p->set((unsigned char)j, b, a);
        }
    }
}

struct Dv4cB { char pad[4]; };
class Dv4c : public Dv4cB { public: 
virtual void v08(int);
};
// Release the comet bitmap and quad renderer; optionally free the comet object.
void* CComet_dtor(void* self, int deleting) {
    if (self) {
        void* p = AsCComet(self)->bitmap;
        *(void**)self = &__vt__6CComet;
        if (p) {
            ((void (*)(void*, int))ResourceManager_ReleaseId)(g_resourceManager, (int)p);
            AsCComet(self)->bitmap = 0;
        }
        if (self) {
            p = AsCComet(self)->material;
            *(void**)self = &s_ballTraceBaseVtable;
            if (p) {
                if (p) {
                    ((Dv4c*)p)->v08(1);
                }
                AsCComet(self)->material = 0;
            }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

} // extern "C"

} // namespace ns2

// Construct the trajectory actor with empty effect slots and its secondary message interface.
NativeOwner80201D18::NativeOwner80201D18() {
    *(void **)this = ns0::__vt__11CTrajectory;
    AsCTrajectory(this)->secondaryVtable = ns0::__vt__11CTrajectory + 0x3c;
    (*(int*)&AsCTrajectory(this)->comet) = 0;
    (*(int*)&AsCTrajectory(this)->doughnut) = 0;
    (*(int*)&AsCTrajectory(this)->puttLine) = 0;
    (*(int*)&AsCTrajectory(this)->ballTrack) = 0;
    (*(int*)&AsCTrajectory(this)->skidMark) = 0;
    (*(int*)&AsCTrajectory(this)->ballWindTrack) = 0;
    (*(int*)&AsCTrajectory(this)->activeEffect0) = 0;
    AsCTrajectory(this)->activeEffect1 = 0;
    AsCTrajectory(this)->activeEffect2 = 0;
    AsCTrajectory(this)->activeEffect3 = 0;
    AsCTrajectory(this)->activeEffect4 = 0;
    (*(int*)&AsCTrajectory(this)->cobraSkidMark) = 0;
    (*(int*)&AsCTrajectory(this)->groundMarker) = 0;
    ns0::IActor_AddSkipFlags(this, kTrajectorySkipFlags);
    AsCTrajectory(this)->unk_8 = 1;
}

extern "C" { void CTrajectory_InitializeUnitConstants(); }
/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621E0", _ctors_u80201D04_0, CTrajectory_InitializeUnitConstants);
#pragma pop
