#include <decomp/static_init.h>
// Snow weather: allocation (CSnow_RebuildParticleBuffers), motion (CSnow_UpdateParticles), and
// camera-facing textured quads (CSnow_Draw). CSnow.h/SnowParticle.h define storage;
// CSnow_Ctor/dtor in pvsfxbox_sky_and_wind_particles.cpp own the lifetime. The CSnow vtable in
// phase1_class_tables.cpp selects these operations through the weather interface.
// Mod map: density=requestedCount/2 below; size=quadBasisScaleA/B; fall speed=
// kFloat_34_295296_Snow * fallSpeedMultiplier; sway=kFloat_1_5707964_Snow;
// volume=kFloat_80_Snow/kFloat_160_Snow and Snow_ComputeOrientedVolumeAABB.
// Spin HUD placement shares this unit but uses CSpin and COption::hudBaselineY.
#include "classes/CameraTransform800D.h"
#include "classes/COption.h"
#include "iff/IffStruct.h"
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
#include "game/types.h"
#include "game/spin.h"
// Translation unit 0x801FA2D0-0x801FB14C (9 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
#include "sdk_protos.h"
#include "classes/CSnow.h"
#include "classes/SnowParticle.h"
#include <decomp/cw_bitfields.h>

#define AsCSnow(p) ((CSnow*)(p))
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Constants below are shared symbols named by value; roles (from their uses in this unit):
//   kFloat_6_2831855 / kFloat_6_2831855_Snow = 2*pi; kFloat_67_Spin = spin display centre X; kFloat_4_Spin = offset added to the HUD baseline Y;
//   kFloat_80_Snow / kFloat_neg80_Snow = half-extent of the snow volume (and particle spawn range); kFloat_160_Snow = volume depth and wrap step;
//   kFloat_34_295296_Snow = base fall speed (world units/second); kFloat_1_5707964_Snow (pi/2) = sway radians/second; kFloat_99999_Snow = last argument of
//   CWind_GetWindVectorAtPoint (meaning unresolved); kFloat_0_*/kFloat_1_CourseBox = plain 0.0 / 1.0.
extern float kFloat_6_2831855;
extern float s_twoPi_SpinSnow;
extern float kFloat_4_Spin;
extern float kFloat_67_Spin;
extern const float kFloat_0_spin;
extern float kFloat_neg1_Snow;
extern const float kFloat_80_Snow;
extern float kFloat_160_Snow;
extern float kFloat_99999_Snow;
extern float kFloat_34_295296_Snow;
extern float kFloat_1_5707964_Snow;
extern const float kFloat_0_Snow;
extern const float kFloat_1_CourseBox;
extern int g_soundManager;
extern void* g_view;
extern char kIdentityMatrix4x4[];
extern char __vt__5CSpin[];
extern char strokePointMoveStrings[];
extern char s_cameraTransform[];
extern const float kFloat_6_2831855_Snow;
extern const float kFloat_neg80_Snow;
extern const float kFloat_0_05_ScreenOverlay;

void* __ct__18NativeBase80201D18Fv(void*);
void* IActor_AddSkipFlags(void*, int);
void Sound_StopByAlias(int, char*);
void __dt__18NativeBase80201D18Fv(void*, int);
void Vec3_Scale(float*, char*, float);
void Vec3_Add(float*, float*, float*);
void Vector_CopyComponents(float*, float*);
void Resource_GetOffsetPtr(void*, int, int, void*, int);
void GraphicsBuffer_Flush(void*);
void Engine_SetRenderStateList(void*, int);
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetTextureStageParameterList(int, void*, int);
void Engine_SetCachedRenderState(int, int);
void Camera_Apply(void*, float, float);
void* Camera_GetRenderState(void*);
void SetTransform(int, void*);
void Engine_SetFlexibleVertexFormat(int);
void Engine_SetVertexStream(int, void*, int, int);
void Engine_SetIndexBuffer(void*, int);
void Engine_SetTextureStageResource(int, int);
void Engine_SetTexturePalette(int, int);
void Engine_DrawIndexedPrimitive(int, int, int, int, int, int);
void __dl__FPv(void*);
void __dla__FPv(void*);
void* __nwa__FUl(int);
float Random_RangeFloat(int, float, float);
float Math_SinApprox(float);
float Math_CosApprox(float);
void* VertexBuffer_Create(int, int, int, int, void*, int);
void* IndexBuffer_Create(int, int, int, int, void*, int, int);
void IndexBuffer_GetOffsetPtr(void*, int, int, void*, int);
void IndexBuffer_Flush(void*);
void CubeResourceStorage_dtor(void*, int);

struct Fn801FAAF4Vertex { float v[5]; };
struct Fn801FAAF4Pair { int a, b; };
struct Fn801FAAF4Block15 { Fn801FAAF4Pair p[15]; };
struct Fn801FAAF4Block6 { Fn801FAAF4Pair p[6]; };
struct Fn801FAAF4Block5 { Fn801FAAF4Pair p[5]; };
struct Fn801FAAF4Table { Fn801FAAF4Block15 a; Fn801FAAF4Block6 b; Fn801FAAF4Block5 c; };
extern Fn801FAAF4Table snowRenderStateLists;

extern COption* g_option;

typedef Vec3f Vec3;

// Buffer handles are retained references: decrement the 16-bit count and destroy at zero.
// particles is separately array-owned; mapping a buffer borrows its CPU write pointer.
struct Fn801FA2D0Ref { CW_BITS2( unsigned int count : 16, unsigned int rest : 16) };
// Camera_GetRenderState starts 0x20 bytes before the saved view matrix.
struct SnowCameraTransforms { char header[0x20]; float view[16]; float projection[16]; };
struct SnowQuadVertex { float position[3]; float uv[2]; };
typedef char SnowQuadVertex_stride[(sizeof(SnowQuadVertex) == 0x14) ? 1 : -1];
typedef char SnowCameraTransforms_view_offset[(IFF_OFFSET(SnowCameraTransforms, view) == 0x20) ? 1 : -1];
typedef char SnowCameraTransforms_projection_offset[(IFF_OFFSET(SnowCameraTransforms, projection) == 0x60) ? 1 : -1];
enum { kSnowQuadVertexCount = 4, kSnowQuadIndexBytes = 8,
    kSnowVertexFormat = 0x110001, kSnowOverlayVertexFormat = 0xF005,
    kSnowOverlayAlpha = 0x80, kSnowOverlayGray = 0x3C, kActorSkipSnowSpin = 0x400,
    kSnowOverlayVertexCount = 4, kSnowOverlayVertexBytes = 0x10,
    kSnowRenderStateCount = 15, kSnowStageStateCount = 6, kSnowStageParameterCount = 5,
    kSnowQuadPrimitive = 0x80, kSnowViewTransform = 0, kSnowProjectionTransform = 1,
    kSnowWorldTransform = 2, kSnowTextureTransform = 0xB };
// Borrow a particle at a byte offset; callers advance by sizeof(SnowParticle).
// Keep this single-evaluation cast macro: the tested inline function changes MWCC's
// instruction scheduling and exception tables in CSnow_UpdateParticles (byte identity required).
#define SnowParticleAt(particles, byteOffset) ((SnowParticle*)((char*)(particles) + (byteOffset)))

struct SnowBufferFields {
    char actorBase[0x20];
    char* particles;
    char particleRenderSetup[0x20];
    int count;
    char bufferPadding[4];
    Fn801FA2D0Ref* vertices;
    Fn801FA2D0Ref* indices;
    Fn801FA2D0Ref* overlayVertexBuffer;
};

// Allocate half the requested particle count, seed snow positions, and build quad/index/overlay buffers.
// requestedCount is divided by two with truncation toward zero; callers supply a nonnegative count.
// Index representation imposes count <= 16384: the last quad index is 4*count-1
// and must fit u16 (65535). Thus requestedCount may be 0..32769; odd counts round
// down. This is a representation bound, not a promise that memory/GPU allocation
// succeeds at that density. No runtime clamp exists; negative values are invalid.
// Replaces owned particles and retained buffers. No allocation-failure recovery is performed here.
void CSnow_RebuildParticleBuffers(SnowBufferFields* self, int requestedCount) {
    int particleByteOffset;
    int quadByteOffset;
    int i;
    int j;
    float angle;
    char* mappedVertices;
    char* mappedIndices;
    char* mappedOverlay;
    int zeroUv;
    int oneUv;
    int vertexBytes;
    Fn801FA2D0Ref* ref;
    int indexBytes;
    int indexByteOffset;
    int secondVertex;
    int firstVertex;
    int thirdVertex;
    int k;
    int overlayByteOffset;
    int c;
    float viewportWidth;
    float viewportHeight;

    // Phase 1: replace the particle array; centers are world positions around the camera.
    self->count = ((int)((unsigned int)requestedCount >> 31) + requestedCount) >> 1;
    if (self->particles) {
        __dla__FPv(self->particles);
        self->particles = 0;
    }
    self->particles = (char*)__nwa__FUl(self->count * (int)sizeof(SnowParticle));

    for (i = 0, particleByteOffset = 0; i < self->count; ++i, particleByteOffset += (int)sizeof(SnowParticle)) {
        angle = Random_RangeFloat(-1, kFloat_0_Snow, kFloat_6_2831855_Snow);
        SnowParticleAt(self->particles, particleByteOffset)->directionSin = Math_SinApprox(angle);
        SnowParticleAt(self->particles, particleByteOffset)->directionCos = Math_CosApprox(angle);
        SnowParticleAt(self->particles, particleByteOffset)->swayPhase = Random_RangeFloat(-1, kFloat_0_Snow, kFloat_6_2831855_Snow);
        SnowParticleAt(self->particles, particleByteOffset)->center[0] = *(float*)(s_cameraTransform + IFF_OFFSET(CameraTransform800D, position)) + Random_RangeFloat(-1, kFloat_neg80_Snow, kFloat_80_Snow);
        SnowParticleAt(self->particles, particleByteOffset)->center[1] = *(float*)(s_cameraTransform + IFF_OFFSET(CameraTransform800D, position) + sizeof(float)) + Random_RangeFloat(-1, kFloat_neg80_Snow, kFloat_80_Snow);
        SnowParticleAt(self->particles, particleByteOffset)->center[2] = *(float*)(s_cameraTransform + IFF_OFFSET(CameraTransform800D, position) + 2 * sizeof(float)) + Random_RangeFloat(-1, kFloat_neg80_Snow, kFloat_80_Snow);
        Vector_CopyComponents(SnowParticleAt(self->particles, particleByteOffset)->position, SnowParticleAt(self->particles, particleByteOffset)->center);
        // The trailing particle byte is initialized here but has no established consumer.
        SnowParticleAt(self->particles, particleByteOffset)->unk_24 = 1;
    }

    // Phase 2: allocate four XYZ/UV vertices per particle and seed their fixed UVs.
    // Positions remain unwritten until CSnow_Draw expands the current particles.
    vertexBytes = self->count * (kSnowQuadVertexCount * (int)sizeof(SnowQuadVertex));
    ref = self->vertices;
    if (ref) {
        if (--ref->count == 0) CubeResourceStorage_dtor(ref, 1);
        self->vertices = 0;
    }
    VertexBuffer_Create(vertexBytes, 0, kSnowVertexFormat, 0, &self->vertices, 0);
    Resource_GetOffsetPtr(self->vertices, 0, 0, &mappedVertices, 0);
    zeroUv = 0;
    oneUv = 1;
    for (j = 0, quadByteOffset = 0; j < self->count; ++j, quadByteOffset += kSnowQuadVertexCount * (int)sizeof(SnowQuadVertex)) {
        *(float*)(mappedVertices + quadByteOffset + 0xC) = (float)zeroUv;
        *(float*)(mappedVertices + quadByteOffset + 0x10) = kFloat_1_CourseBox - (float)zeroUv;
        *(float*)(mappedVertices + quadByteOffset + 0x20) = (float)oneUv;
        *(float*)(mappedVertices + quadByteOffset + 0x24) = kFloat_1_CourseBox - (float)zeroUv;
        *(float*)(mappedVertices + quadByteOffset + 0x34) = (float)zeroUv;
        *(float*)(mappedVertices + quadByteOffset + 0x38) = kFloat_1_CourseBox - (float)oneUv;
        *(float*)(mappedVertices + quadByteOffset + 0x48) = (float)oneUv;
        *(float*)(mappedVertices + quadByteOffset + 0x4C) = kFloat_1_CourseBox - (float)oneUv;
    }
    GraphicsBuffer_Flush(self->vertices);

    // Phase 3: each quad uses four 16-bit indices in perimeter order 0, 1, 3, 2.
    ref = self->indices;
    indexBytes = self->count * kSnowQuadIndexBytes;
    if (ref) {
        if (--ref->count == 0) CubeResourceStorage_dtor(ref, 1);
        self->indices = 0;
    }
    IndexBuffer_Create(indexBytes, 0, 3, 0, &self->indices, 0, 0);
    IndexBuffer_GetOffsetPtr(self->indices, 0, 0, &mappedIndices, 0);
    {

        for (k = 0, firstVertex = 0, indexByteOffset = 0; k < self->count; ++k, firstVertex += kSnowQuadVertexCount, indexByteOffset += kSnowQuadIndexBytes) {
            secondVertex = firstVertex + 1;
            thirdVertex = secondVertex + 1;
            *(unsigned short*)(mappedIndices + indexByteOffset) = firstVertex;
            *(unsigned short*)(mappedIndices + indexByteOffset + 2) = secondVertex;
            *(unsigned short*)(mappedIndices + indexByteOffset + 4) = thirdVertex + 1;
            *(unsigned short*)(mappedIndices + indexByteOffset + 6) = thirdVertex;
        }
    }
    IndexBuffer_Flush(self->indices);

    // Phase 4: build a separate viewport-sized XY/depth/RGBA overlay, in pixel coordinates.
    ref = self->overlayVertexBuffer;
    if (ref) {
        if (--ref->count == 0) CubeResourceStorage_dtor(ref, 1);
        self->overlayVertexBuffer = 0;
    }
    VertexBuffer_Create(kSnowOverlayVertexCount * kSnowOverlayVertexBytes, 0, kSnowOverlayVertexFormat, 0, &self->overlayVertexBuffer, 0);
    Resource_GetOffsetPtr(self->overlayVertexBuffer, 0, 0, &mappedOverlay, 0);

    for (c = 0, overlayByteOffset = 0; c < kSnowOverlayVertexCount; ++c, overlayByteOffset += kSnowOverlayVertexBytes) {
        viewportWidth = AsWView(g_view)->viewportWidth;
        *(float*)(mappedOverlay + overlayByteOffset) = (float)(c & 1) * viewportWidth;
        viewportHeight = AsWView(g_view)->viewportHeight;
        *(float*)(mappedOverlay + overlayByteOffset + 4) = (float)(c >> 1) * viewportHeight;
        *(float*)(mappedOverlay + overlayByteOffset + 8) = kFloat_0_05_ScreenOverlay;
        ((unsigned char*)mappedOverlay + overlayByteOffset)[0xF] = kSnowOverlayAlpha;
        ((unsigned char*)mappedOverlay + overlayByteOffset)[0xC] = kSnowOverlayGray;
        ((unsigned char*)mappedOverlay + overlayByteOffset)[0xD] = kSnowOverlayGray;
        ((unsigned char*)mappedOverlay + overlayByteOffset)[0xE] = kSnowOverlayGray;
    }
    GraphicsBuffer_Flush(self->overlayVertexBuffer);
}

struct SnowDrawFields {
    char actorBase[0x20];
    void* particles;
    char particleRenderPadding[4];
    void* state;
    char textureSetup[0x10];
    float quadBasisScaleA;
    float quadBasisScaleB;
    int count;
    char bufferPadding[4];
    void* buffer;
    void* indexBuffer;
};

// View render-target interface: slots 26/27 look up a texture and texture palette;
// v0..v25 are placeholders that put them at the right vtable index.
struct Fn801FAAF4Manager {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24(); virtual void v25();
    virtual int texture(void*);
    virtual int texturePalette(void*);
};

// Expand each snow particle to a camera-facing quad and submit the snow draw.
// Borrows particle positions and maps the retained vertex buffer; initialization must have run first.
void CSnow_Draw(SnowDrawFields* self) {
    char* stateTable = (char*)&snowRenderStateLists;
    void* vertices;
    Resource_GetOffsetPtr(self->buffer, 0, 0, &vertices, 0);
    // Phase 1: use the first two stored view basis vectors as world-space quad edges.
    // The four corners are P, P+A, P+B, P+A+B; P is an anchored corner, not a center.
    Vec3 edgeA, edgeB, oppositeCornerOffset;
    Vec3_Scale(&edgeA.x, ((char*)&AsWView(g_view)->viewMatrix), self->quadBasisScaleA);
    Vec3_Scale(&edgeB.x, (char*)&AsWView(g_view)->viewMatrix[3], self->quadBasisScaleB);
    Vec3_Add(&oppositeCornerOffset.x, &edgeA.x, &edgeB.x);

    for (int i = 0; i < self->count; ++i) {
        Vector_CopyComponents((float*)((char*)vertices + i * (kSnowQuadVertexCount * (int)sizeof(SnowQuadVertex))), (float*)((char*)self->particles + i * (int)sizeof(SnowParticle)));
        Vec3_Add((float*)((Fn801FAAF4Vertex*)vertices + (i * kSnowQuadVertexCount + 1)), (float*)((char*)self->particles + i * (int)sizeof(SnowParticle)), &edgeA.x);
        Vec3_Add((float*)((Fn801FAAF4Vertex*)vertices + (i * kSnowQuadVertexCount + 2)), (float*)((char*)self->particles + i * (int)sizeof(SnowParticle)), &edgeB.x);
        Vec3_Add((float*)((Fn801FAAF4Vertex*)vertices + (i * kSnowQuadVertexCount + 3)), (float*)((char*)self->particles + i * (int)sizeof(SnowParticle)), &oppositeCornerOffset.x);
    }
    GraphicsBuffer_Flush(self->buffer);

    // Phase 2: copy state/value pairs (15 render, 6 stage, 5 stage-parameter entries).
    Fn801FAAF4Block15 renderStates = *(Fn801FAAF4Block15*)((unsigned int)stateTable + 0);
    Fn801FAAF4Block5 stageParameters;
    Fn801FAAF4Block6 stageStates;
    stageStates = *(Fn801FAAF4Block6*)((unsigned int)stateTable + 0x78);
    stageParameters = *(Fn801FAAF4Block5*)((unsigned int)stateTable + 0xA8);
    Engine_SetRenderStateList(&renderStates, kSnowRenderStateCount);
    Engine_SetTextureStageStateList(0, &stageStates, kSnowStageStateCount);
    Engine_SetTextureStageParameterList(0, &stageParameters, kSnowStageParameterCount);
    Engine_SetCachedRenderState(5, 0);
    Camera_Apply(g_view, kFloat_0_Snow, kFloat_0_Snow);
    void* camera = Camera_GetRenderState(g_view);
    SetTransform(kSnowViewTransform, ((SnowCameraTransforms*)camera)->view);
    SetTransform(kSnowProjectionTransform, ((SnowCameraTransforms*)camera)->projection);
    SetTransform(kSnowWorldTransform, kIdentityMatrix4x4);
    SetTransform(kSnowTextureTransform, kIdentityMatrix4x4);
    Engine_SetCachedRenderState(0xD, 4);
    Engine_SetCachedRenderState(0xE, 1);
    Engine_SetCachedRenderState(1, 0);
    Engine_SetFlexibleVertexFormat(kSnowVertexFormat);
    // Phase 3: bind XYZ/UV vertices, indices and the two texture handles, then draw all quads.
    // Slot 27 supplies a texture-palette handle; it is independent of indexBuffer.
    Engine_SetVertexStream(0, self->buffer, 0, sizeof(SnowQuadVertex));
    Engine_SetIndexBuffer(self->indexBuffer, 0);
    int stageResource = ((Fn801FAAF4Manager*)AsWView(g_view)->renderTarget)->texture(self->state);
    Engine_SetTextureStageResource(0, stageResource);
    int texturePalette = ((Fn801FAAF4Manager*)AsWView(g_view)->renderTarget)->texturePalette(self->state);
    Engine_SetTexturePalette(0, texturePalette);
    Engine_DrawIndexedPrimitive(kSnowQuadPrimitive, 0, 0, self->count * kSnowQuadVertexCount, 0, self->count);
}

void* CWind_GetInstance();
void CWind_GetWindVectorAtPoint(Vec3*, void*, Vec3*, float);
// Initialize the spin/snow angle constant to one full turn.
void SpinSnow_InitializeAngleConstant() {
    s_twoPi_SpinSnow = kFloat_6_2831855;
}

// Position the spin display relative to the configured HUD baseline.
void CSpin_OnPreLoadInit(CSpin* self) {
    float baselineOffset = kFloat_4_Spin;
    float fixedCenterX = kFloat_67_Spin;
    float hudBaseline = g_option->hudBaselineY;
    float centerY = baselineOffset + hudBaseline;
    self->centerX = fixedCenterX;
    self->centerY = centerY;
}

// Restart the spin-indicator animation through its shared reset routine.
void CSpin_OnInit(CSpin* self) {
    self->Reset();
}

// Install the spin actor interface, display priority and update-skip bit.
CSpin::CSpin() {
    CSpin* self = this;
    self->displayPriority = 4;
    self->vtable = __vt__5CSpin;
    IActor_AddSkipFlags(self, kActorSkipSnowSpin);
}

// Stop the spin movement sound and release the actor base.
void* CSpin_dtor(CSpin* self, int deleting) {
    if (self) {
        int soundWasPlaying = self->soundPlaying;
        self->vtable = __vt__5CSpin;
        if (soundWasPlaying) {
            Sound_StopByAlias(g_soundManager, strokePointMoveStrings);
            self->soundPlaying = 0;
        }
        __dt__18NativeBase80201D18Fv(self, 0);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Bound the eight corners of the camera-oriented snow volume in world space.
// unused is ignored; min/max are required writable outputs. The volume spans +/-80
// along camera bases 0/1 and 0..160 along basis 2, translated by camera position.
void Snow_ComputeOrientedVolumeAABB(void* unused, Vec3* min, Vec3* max) {
    Vec3 points[8];
    float sign = kFloat_neg1_Snow;
    unsigned int offset = 0;
    Vec3* q = points;

    // Phase 1: form the two four-corner planes in camera-relative world axes.
    for (int i = 0; 4 > i; ++i) {
        float width = kFloat_80_Snow;

        float x = width;
        if (1 & i) x = width * sign;
        if (2 & i) width = width * sign;
        Vec3 a; Vec3_Scale(&a.x, s_cameraTransform, x);
        Vec3 b; Vec3_Scale(&b.x, s_cameraTransform + IFF_OFFSET(CameraTransform800D, basis[3]), width);
        Vec3_Add(&points[i].x, &a.x, &b.x);
        Vec3_Scale(&b.x, s_cameraTransform + IFF_OFFSET(CameraTransform800D, basis[6]), kFloat_160_Snow);
        // points[4 + i]: the original adds 0xC to the offset after this store (801FAEF0-801FAF0C); adding it first wrote
        // points[8], past the array, over saved r24-r26 (the White Wiz snow caller at 0x80170D08).
        Vec3_Add((float*)((char*)points + offset + 0x30), &q->x, &b.x);
        offset += 0x0C;
        ++q;
    }
    // Phase 2: reduce all eight corners to an axis-aligned box, then translate to world space.
    Vector_CopyComponents(&min->x, &points[0].x);
    Vector_CopyComponents(&max->x, &points[0].x);
    Vec3* p = points + 1;
    for (int k = 0; k < 7; ++k, ++p) {
        if (min->x > p->x) min->x = p->x;
        if (min->y > p->y) min->y = p->y;
        if (min->z > p->z) min->z = p->z;
        if (max->x < p->x) max->x = p->x;
        if (max->y < p->y) max->y = p->y;
        if (max->z < p->z) max->z = p->z;
    }
    Vec3_Add(&min->x, &min->x, (float*)(s_cameraTransform + IFF_OFFSET(CameraTransform800D, position)));
    Vec3_Add(&max->x, &max->x, (float*)(s_cameraTransform + IFF_OFFSET(CameraTransform800D, position)));
}

// Integrate wind and falling speed, wrap particles into the volume, and add sinusoidal sway.
// deltaTime is scaled simulation seconds. Source chain: CMainFrame_UpdateVirtualHooks
// (main_frame_letter_and_mouse_cursor.cpp, reciprocal 30/25 updates/s) -> CProjectG_Update (project_g_app_and_devices.cpp, timeScale)
// -> CTask_UpdateCurrentTaskWithFadeGates (ctask_transition_command_queue.cpp) -> CTask_ProcessActors
// (task_manager_and_actors.cpp) -> DrawScene_UpdateRegisteredObjects (draw_scene_named_effect_list.cpp, primaryUpdater +F8)
// -> CSnow's update virtual slot (phase1_class_tables). Each forwards seconds.
// A nonnegative finite delta is expected; pauses may suppress the call entirely.
// Fall distance = 34.295296 world units/s * fallSpeedMultiplier * seconds, -Y.
// Sway adds pi/2 radians/s (four simulated seconds per cycle); increasing this
// rate speeds sway without changing its one-world-unit amplitude. Increasing the
// fall multiplier speeds descent only. Neither coefficient has a proven tuning
// range; preserve single-precision order and the shared wind timebase.
// center is integrated/wrapped world position; position adds horizontal unit-amplitude sway.
void CSnow_UpdateParticles(void* self, float deltaTime) {
    float swayRate;
    Vec3 min, max;
    Vec3 windVelocity;
    // Phase 1: sample wind once at the camera and convert velocity to this update's displacement.
    Snow_ComputeOrientedVolumeAABB(self, &min, &max);
    CWind_GetWindVectorAtPoint(&windVelocity, CWind_GetInstance(), (Vec3*)(s_cameraTransform + IFF_OFFSET(CameraTransform800D, position)), kFloat_99999_Snow);
    Vec3 windDisplacement;
    windDisplacement.x = windVelocity.x;
    windDisplacement.y = windVelocity.y;
    windDisplacement.z = windVelocity.z;
    Vec3_Scale(&windDisplacement.x, (char*)&windDisplacement.x, deltaTime);

    int byteOffset;
    int particleIndex = 0;
    byteOffset = 0;
    float fallDistance;

    float wrapDistance;
    wrapDistance = kFloat_160_Snow;
    swayRate = kFloat_1_5707964_Snow;
    fallDistance = kFloat_34_295296_Snow * AsCSnow(self)->fallSpeedMultiplier * deltaTime;
    // Phase 2: integrate centers and phase independently from the visible sway offset.
    for (; particleIndex < AsCSnow(self)->particleCount; byteOffset += (int)sizeof(SnowParticle), ++particleIndex) {
        Vec3_Add(
            SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center,
            SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center,
            &windDisplacement.x);
        SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[1] -= fallDistance;
        SnowParticleAt(AsCSnow(self)->particles, byteOffset)->swayPhase =
            swayRate * deltaTime + SnowParticleAt(AsCSnow(self)->particles, byteOffset)->swayPhase;

        Vec3 swayOffset = {0, 0, 0};
        swayOffset.x = SnowParticleAt(AsCSnow(self)->particles, byteOffset)->directionCos;
        swayOffset.z = SnowParticleAt(AsCSnow(self)->particles, byteOffset)->directionSin;
        Vec3_Scale(&swayOffset.x, (char*)&swayOffset.x, Math_SinApprox(SnowParticleAt(AsCSnow(self)->particles, byteOffset)->swayPhase));

        // Phase 3: wrap each world axis in 160-unit steps, even when the AABB is wider.
        // Preserve the asymmetric below-min expression (max - center); this is existing behavior.
        if (SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[0] > max.x)
            SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[0] -= wrapDistance * (float)ceil((double)((SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[0] - max.x) / wrapDistance));
        else if (SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[0] < min.x)
            SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[0] += wrapDistance * (float)ceil((double)((max.x - SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[0]) / wrapDistance));
        if (SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[1] > max.y)
            SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[1] -= wrapDistance * (float)ceil((double)((SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[1] - max.y) / wrapDistance));
        else if (SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[1] < min.y)
            SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[1] += wrapDistance * (float)ceil((double)((max.y - SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[1]) / wrapDistance));
        if (SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[2] > max.z)
            SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[2] -= wrapDistance * (float)ceil((double)((SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[2] - max.z) / wrapDistance));
        else if (SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[2] < min.z)
            SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[2] += wrapDistance * (float)ceil((double)((max.z - SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center[2]) / wrapDistance));
        // Phase 4: publish the visible world position after wrapping the unswayed center.
        Vec3_Add(
            SnowParticleAt(AsCSnow(self)->particles, byteOffset)->position,
            SnowParticleAt(AsCSnow(self)->particles, byteOffset)->center,
            &swayOffset.x);
    }
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621CC", _ctors_u801FA2D0_0, SpinSnow_InitializeAngleConstant);
#pragma pop
