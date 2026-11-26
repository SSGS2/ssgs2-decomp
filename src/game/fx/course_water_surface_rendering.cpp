// Course water surface: the spring-grid ground surface with its ripples, the lit surface
// drawing, and the generated point geometry for its texture batches.

typedef unsigned long new_size_t;
// Course water rendering: spring-grid ripples, lit surface and caustic layers.
// Mod lookup (related settings must be considered together):
// - random ripples: GridSurface_UpdateRipples, kRippleIntervalMs=1000 (strictly
//   greater than), kFloat_2_GridSurface=2 velocity impulse to four cells. Timer is
//   unsigned elapsed milliseconds; two rand() draws occur only on injection.
// - surfaceFlags: bit 0 disables injection only; bit 1 skips spring/normal updates
//   and selects the flat quad at draw time; bit 2 skips region registration in
//   GroundGrid_ConfigureSurfaceBounds. Flags can be combined.
// - surfaceType/flags/texture rotation: GroundGrid_ConfigureSurfaceBounds parses
//   resource *type/*rot/*noripple tokens (waterCausticStrings); this unit consumes
//   the resulting flags: type 0 leaves flags unchanged, type 1 ORs bits 0|1,
//   type 2 ORs bits 0|2|3; *noripple forces type 1. Bit 3's consumer is unresolved.
//   Reconfiguration does not clear old flags. Keep bounds, row/column counts and
//   both row arrays paired. *rot is degrees, converted to radians for texture use.
// - lit visibility: GroundGrid_DrawLitSurface skips course IDs 8/11/12/13 and
//   camera Y below boundsMax.y; caustics draw before those tests. Numeric IDs here
//   must be resolved through the course table, not inferred from display names.
// - lit alpha: course 10 uses 40/255, others 80/255; sceneLighting supplies RGB.
//   Specular texture comes from waterCausticStrings.waterSpecular; UV transform
//   is textureTransform. Both surface paths use the documented half-vector shade.
// - caustic blending: course 13, renderMode 2, first two layers use src-alpha /
//   inverse-src-alpha; others use src-alpha / one (additive). Per-layer colors,
//   textures and UVs live in DrawItem, positions in GeneratedPointGeometryStorage.
// - shared render defaults: geneTexRenderStateStorage = 15 render, 6 stage and
//   5 sampler pairs; both draw entry points copy them then override depth/blend,
//   transforms and texture stage 0. They mutate global render state and do not
//   restore it. Schedule on the render thread; callers manage subsequent state.
// Data companions: waterCausticStrings, geneTexRenderStateStorage, scene/camera
// globals; SpringGridVertex and project_base_and_grid_surface.cpp supply spring integration/creation.
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
#include "game/types.h"
#include "classes/SpringGridVertex.h"
#include "classes/CameraTransform800D.h"
#include "classes/CGeneTexPointGeometry.h"

// The generated-texture header merges offsets from several owners; keep the grid and point-geometry views separate.
struct DrawItem;
struct GeneratedPointGeometryStorage {
    Vec3f boundsMin, boundsMax, center;
    char reserved24[4];
    unsigned int* positions;
    int pointCount;
    DrawItem* batches;
    int batchCount;
    char reserved38[0x38];
    int renderMode;
    unsigned char vertexCount;
    char reserved75[3];
    void* vertexBufferVtable;
    void* vertexStorage;
    void** graphicsBuffers;
    unsigned int bufferCount, bufferCapacity;
    unsigned int textureTransform[16];
};
struct GridCameraRenderState {
    char reserved[0x20];
    unsigned int viewMatrix[16], projectionMatrix[16];
};
struct GridSceneLighting {
    int mode;
    float direction[3];
    unsigned int color;
};
#define GEOMETRY_OFFSET(field) ((int)&((GeneratedPointGeometryStorage*)0)->field)
#define GRID_RENDER_OFFSET(field) ((int)&((GridCameraRenderState*)0)->field)
struct GroundGridEdge {
    char reserved[0x10];
    int color;
    char tail[0x24];
};
struct GroundGridStorage {
    void* pointGeometry;
    int columnCount, rowCount;
    void** springRows;
    void** localPositionRows;
    void* specularTexture;
    unsigned int lastRippleTimeMs;
    Vec3f boundsMin, boundsMax, center;
    void* edge0;
    void* edge1;
    void* edge3;
    void* edge2;
    int reservedBoundaryWord; // Cleared with the boundary records; no read or behavioral meaning is established here.
    GroundGridEdge boundaryEdges[4];
    void* cachedSurface;
    char reserved138[4];
    int surfaceType;
    int surfaceFlags;
    unsigned char surfaceVertexCount;
    char reserved145[3];
    void* surfaceVertexBufferVtable;
    void* surfaceVertices;
    unsigned int textureTransform[16];
};
struct WaterCausticStrings {
    char pola[5], icee[5], causticVariantFormat[20], causticFormat[18];
    char vectorLengthError[20], lengthErrorType[55], waterSpecular[19];
    char typeToken[6], integerFormat[3], rotationToken[5], floatFormat[3], noRippleToken[10];
    char padding[3];
};
enum GroundSurfaceFlags {
    kDisableRandomRipples = 1,
    kFlatSurface = 2,
    kSkipGroundRegion = 4
};
enum GroundSurfaceType {
    kRippleSurface = 0,
    kStillSurface = 1,
    kExcludedSurface = 2
};
enum {
    kRippleIntervalMs = 1000,
    kGroundRegionType = 3,
    kInitialGridColor = 0xFFFFFF,
    kVec3VectorMaxElements = 0x15555555,
    kTriangleStripPrimitive = 0x98,
    kTriangleFanPrimitive = 160
};
#define AsGroundGrid(p) ((GroundGridStorage*)(p))
#define GRID_OFFSET(field) ((int)&((GroundGridStorage*)0)->field)
typedef char GroundGrid_textureTransform_offset[(GRID_OFFSET(textureTransform) == 0x150) ? 1 : -1];
typedef char GroundGrid_edge_size[(sizeof(GroundGridEdge) == 0x38) ? 1 : -1];

extern "C" {

void __as__7WVectorFRC7WVector(void*, void*);
void __dl__FPv(void*);
void __dla__FPv(void*);
void* __nw__FUl(unsigned int);
void Vec3_Zero(void*);
void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);
void PSMTX44Identity(void*);
unsigned char SharedDoc_GetCourseId(void);
void __ct__10CGeneTex3DFv(void*);
int WView_DefaultCapabilityTrueC(void*, void*);
unsigned int rand(void);
void Grid_RecomputeNormalsAndNormalTexcoords(void*);
void GridSurface_StepSpringHeights(void*, float);
void ResourceManager_ReleaseId(void*, void*);
void GeneTexPointGeometry_Dtor(void*, int);
void __dt__13GeneTexBufferFv(void*);
extern void* g_resourceManager;
extern void* g_view;
extern unsigned int g_currentTimeMilliseconds;
extern const char waterCausticStrings[];
extern char __vt__10CGeneTex3D[];
extern const float kFloat_2_GridSurface;
extern const float kFloat_0_5_Collision;
extern const float kFloat_0_805DF6F4;
extern const float kFloat_0_017453292_GroundGrid;
void AABB_Copy(void*, const void*);
void BorrowedNameRef_Set(void*, void*);
void String_ScanTokensAfterSubstring(const char*, const char*, const char*, ...);
int strstr(void*, const char*);
void* CQuadTree_GetInstance();
void GroundRegionList_AppendBounds(void*, void*, int);
void GroundGrid_InitializeTwoByTwo(void*);
void Mtx44_SetEulerRotationZXY(void*, float, float, float);
void Mtx44_SetTranslation(void*, float, float, float);
void PSMTX44Concat(void*, void*, void*);
void* Vec3_Set(void*, float, float, float);
void Vec3Vector_AppendCopy(void*, void*);
void dtor_801247CC(void*, int);
void __dt__Q33std6detail16char_array_guardFv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*);
void dtor_801263D8(void*);
unsigned int appMallocDummy__FUl(unsigned int);
unsigned int __nwa__FUl(unsigned int);
unsigned int strlen(const char*);
char* strcpy(char*, const char*);
extern const char __vt__Q23std11logic_error[];
extern const char s_sharedCharDeleterVtable[];
extern const char __vt__Q23std12length_error[];
extern const char waterCausticStrings[];
extern const char waterCausticStrings[];
void __throw(void*, void*, void*);
#pragma does_not_return __throw
void __dt__Q23std12length_errorFv(void*);
void Engine_SetRenderStateList(void*, int);
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetTextureStageParameterList(int, void*, int);
void SetTransform(int, const void*);
void Engine_SetType7Resource(int);
void Engine_SetTextureStageState(int, int, int);
void Engine_SetTextureStageParameter(int, int, int);
void Engine_SetCachedRenderState(int, int);
void Engine_SetTextureStageResource(int, int);
void Engine_SetTexturePalette(int, int);
void Resource_GetOffsetPtr(void*, int, int, void**, int);
void Vector_CopyComponents(void*, void*);
// Upload each caustic layer's positions, color, and UVs, then draw its triangle fan.
void GeneTexPointGeometry_DrawTextureBatches(void*);
void GraphicsBuffer_Flush(void*);
void Engine_SetVertexStream(int, void*, int, int);
void DrawPrimitiveFromBoundBuffers(int, int, int);
void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
void* Normalize__7WVectorFv(void*);
void __ml__FfRC7WVector(void*, const void*, float);
void __pl__FRC7WVectorRC7WVector(void*, const void*, void*);
float __ml__FRC7WVectorRC7WVector(void*, const void*);
int GX_GetPrimitiveCount(int, int);
void GraphicsBuffer_Flush(void*);
extern const float kFloat_1_GeneTex3D;
extern char s_cameraTransform[], s_sceneLighting[];
struct RenderStatePairs15 {
    unsigned int v[30];
};
struct TextureStagePairs6 {
    unsigned int v[12];
};
struct SamplerPairs5 {
    unsigned int v[10];
};
// Word packets contain {engine state ID, value} pairs, not matrices or vertices.
struct WaterRenderStateDefaults {
    RenderStatePairs15 renderStates;
    TextureStagePairs6 textureStages;
    SamplerPairs5 textureParameters;
};
extern WaterRenderStateDefaults geneTexRenderStateStorage;
extern char kIdentityMatrix4x4[];
void* Camera_GetRenderState(void*);

// Initialize ripple-grid storage, water texture, edge records, and generated texture geometry.
// Required writable unconstructed GroundGridStorage; returns self. Resource manager
// is borrowed; the specular handle must be released with ResourceManager_ReleaseId,
// generated geometry with GeneTexPointGeometry_Dtor(...,1). Ctor leaves row counts
// for later grid initialization. ConfigureSurfaceBounds must precede update/draw.
// Both course branches construct the same object: retain the duplicate shape and
// per-allocation catch (free object storage then rethrow). No whole-ctor rollback
// or null-self validation is supplied; missing texture/geometry are stored as null.
void* GroundGrid_Ctor(void* self) {

    char* p = (char*)self;
    unsigned int max = kInitialGridColor;
    (*(int*)&AsGroundGrid(p)->springRows) = 0;
    (*(int*)&AsGroundGrid(p)->localPositionRows) = 0;
    (*(int*)&AsGroundGrid(p)->specularTexture) = 0;
    Vec3_Zero(p + GRID_OFFSET(boundsMin));
    Vec3_Zero(p + GRID_OFFSET(boundsMax));
    (*(int*)&AsGroundGrid(p)->cachedSurface) = 0;
    AsGroundGrid(p)->surfaceFlags = 0;
    AsGroundGrid(p)->surfaceVertexCount = 0;
    AsGroundGrid(p)->surfaceVertexBufferVtable = __vt__10CGeneTex3D;
    (*(int*)&AsGroundGrid(p)->surfaceVertices) = 0;
    AsGroundGrid(p)->edge0 = p + GRID_OFFSET(boundaryEdges[0]);
    AsGroundGrid(p)->edge1 = p + GRID_OFFSET(boundaryEdges[1]);
    AsGroundGrid(p)->edge3 = p + GRID_OFFSET(boundaryEdges[3]);
    AsGroundGrid(p)->edge2 = p + GRID_OFFSET(boundaryEdges[2]);
    AsGroundGrid(p)->reservedBoundaryWord = 0;
    AsGroundGrid(p)->boundaryEdges[0].color = max;
    AsGroundGrid(p)->boundaryEdges[1].color = max;
    AsGroundGrid(p)->boundaryEdges[2].color = max;
    AsGroundGrid(p)->boundaryEdges[3].color = max;
    AsGroundGrid(p)->specularTexture = ResourceManager_LoadBitmap(
        g_resourceManager, ((const WaterCausticStrings*)waterCausticStrings)->waterSpecular, 0, 0, 0);
    PSMTX44Identity(p + GRID_OFFSET(textureTransform));
    int mode = SharedDoc_GetCourseId();
    if (!(mode == 9 || mode < 8 || 14 <= mode)) {
        void* obj = __nw__FUl(0xCC);
        if (obj != 0) {
            try {
                __ct__10CGeneTex3DFv(obj);
            } catch (...) {
                __dl__FPv(obj);
                throw;
            }
        }
        AsGroundGrid(p)->pointGeometry = obj;
    } else {
        void* obj = __nw__FUl(0xCC);
        if (0 != obj) {
            try {
                __ct__10CGeneTex3DFv(obj);
            } catch (...) {
                __dl__FPv(obj);
                throw;
            }
        }
        AsGroundGrid(p)->pointGeometry = obj;
    }
    return self;
}

#pragma push
#pragma use_lmw_stmw on
// Release spring/local-position rows, cached surface, texture handle, and generated geometry.
// Null self is accepted; destroy>0 also frees self. Row arrays must be paired and
// match rowCount. Each row's allocation cookie starts 0x10 before its element
// pointer; freeing the visible element pointer would violate the allocator ABI.
// Resource manager must remain live to release a non-null specular handle.
void* GroundGrid_Dtor(void* self, int destroy) {
    char* p = (char*)self;
    if (self != 0) {
        if (AsGroundGrid(p)->springRows != 0) {
            for (int i = 0; i < AsGroundGrid(p)->rowCount; ++i) {
                if (((void**)AsGroundGrid(p)->springRows)[i] != 0) {
                    if (((void**)AsGroundGrid(p)->springRows)[i] != 0)
                        __dla__FPv((char*)(((void**)AsGroundGrid(p)->springRows)[i]) - 0x10);
                    ((void**)AsGroundGrid(p)->springRows)[i] = 0;
                }
                if (((void**)AsGroundGrid(p)->localPositionRows)[i] != 0) {
                    if (((void**)AsGroundGrid(p)->localPositionRows)[i] != 0)
                        __dla__FPv((char*)(((void**)AsGroundGrid(p)->localPositionRows)[i]) - 0x10);
                    ((void**)AsGroundGrid(p)->localPositionRows)[i] = 0;
                }
            }
            if (AsGroundGrid(p)->springRows != 0) {
                __dla__FPv(AsGroundGrid(p)->springRows);
                AsGroundGrid(p)->springRows = 0;
            }
            if (AsGroundGrid(p)->localPositionRows != 0) {
                __dla__FPv(AsGroundGrid(p)->localPositionRows);
                AsGroundGrid(p)->localPositionRows = 0;
            }
        }
        if (AsGroundGrid(p)->cachedSurface != 0) {
            __dl__FPv(AsGroundGrid(p)->cachedSurface);
            AsGroundGrid(p)->cachedSurface = 0;
        }
        if (g_resourceManager != 0 && AsGroundGrid(p)->specularTexture != 0) {
            ResourceManager_ReleaseId(g_resourceManager, AsGroundGrid(p)->specularTexture);
            AsGroundGrid(p)->specularTexture = 0;
        }
        if (AsGroundGrid(p)->pointGeometry != 0) {
            GeneTexPointGeometry_Dtor(AsGroundGrid(p)->pointGeometry, 1);
            AsGroundGrid(p)->pointGeometry = 0;
        }
        if (p + GRID_OFFSET(surfaceVertexCount) != 0) {
            AsGroundGrid(p)->surfaceVertexBufferVtable = __vt__10CGeneTex3D;
            if (AsGroundGrid(p)->surfaceVertices != 0) {
                __dla__FPv(AsGroundGrid(p)->surfaceVertices);
                AsGroundGrid(p)->surfaceVertices = 0;
            }
        }
        if (destroy > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

// Inject periodic random ripples, step springs, and advance caustic texture coordinates.
// The named capability gate returns 1 unconditionally (wview_camera_projection.cpp); it performs
// no bounds test; it is not a visibility or frustum test.
// Required: live grid, columnCount>1 and rowCount>1 for the modulo denominators,
// rowCount valid springRows of columnCount cells and corresponding position rows.
// The injection touches (row,column), (row+1,column), (row,column+1), (row+1,column+1).
// deltaTime is forwarded unchanged to spring/UV updates; use the caller's existing
// time scale (no seconds-to-frames conversion here). No range clamp or failure
// result exists. Flat mode still permits random impulses unless bit 0 is set.
void GridSurface_UpdateRipples(void* self, float deltaTime) {
    if (WView_DefaultCapabilityTrueC(g_view, (char*)self + GRID_OFFSET(boundsMin))) {
        if ((((*(unsigned int*)&AsGroundGrid(self)->surfaceFlags)) & 1) == 0 &&
            (unsigned int)(g_currentTimeMilliseconds - (*(unsigned int*)&AsGroundGrid(self)->lastRippleTimeMs)) >
                kRippleIntervalMs) {
            (*(unsigned int*)&AsGroundGrid(self)->lastRippleTimeMs) = g_currentTimeMilliseconds;
            int column = (int)rand() % (AsGroundGrid(self)->columnCount - 1);
            int row = (int)rand() % (AsGroundGrid(self)->rowCount - 1);
            void** springRows = AsGroundGrid(self)->springRows;
            SpringGridVertex* cell = (SpringGridVertex*)((char*)springRows[row] + column * sizeof(SpringGridVertex));
            cell->velocity += kFloat_2_GridSurface;
            cell = (SpringGridVertex*)((char*)springRows[row + 1] + (column + 1) * sizeof(SpringGridVertex));
            cell->velocity += kFloat_2_GridSurface;
            cell = (SpringGridVertex*)((char*)springRows[row + 1] + column * sizeof(SpringGridVertex));
            cell->velocity += kFloat_2_GridSurface;
            cell = (SpringGridVertex*)((char*)springRows[row] + (column + 1) * sizeof(SpringGridVertex));
            cell->velocity += kFloat_2_GridSurface;
        }
        if ((((*(unsigned int*)&AsGroundGrid(self)->surfaceFlags)) & 2) == 0) {
            Grid_RecomputeNormalsAndNormalTexcoords(self);
            GridSurface_StepSpringHeights(self, deltaTime);
        }
        if (AsGroundGrid(self)->pointGeometry != 0)
            ((CGeneTex3D*)(AsGroundGrid(self)->pointGeometry))->AdvanceLayerTexcoords(deltaTime);
    }
}

// Copy the generated geometry's center from the supplied world-space position.
void GeneTexPointGeometry_SetCenter(void* self, void* source) {
    __as__7WVectorFRC7WVector((char*)self + GEOMETRY_OFFSET(center), source);
}

// Enable the requested bits without changing any other flags.
void BitFlags_OrInPlace(unsigned int* value, unsigned int flags) {
    *value |= flags;
}

// Initialize an empty vector of three-word positions.
void Vec3Vector_Init(unsigned int* values) {
    values[0] = 0;
    values[1] = 0;
    values[2] = 0;
}

// Report whether any requested flag is set.
int BitFlags_TestAny(const unsigned int* value, unsigned int mask) {
    return (*value & mask) != 0;
}

typedef Vec3f V3;
struct M22 {
    unsigned int v[22];
};
struct PositionWords3 {
    unsigned int v[3];
};

// Return whether any bits in the requested mask are present.
inline int Bit(int v, int m) {
    return (v & m) != 0;
}

// Embedded surface upload interface starts at surfaceVertexCount, not at the grid base.
// Begin opens a batch; Reserve returns the usable vertex count (byte-sized).
// Pos receives center-relative XYZ, Color receives A,R,G,B, and Shade writes two equal
// texture/shading coordinates. End submits the primitive count, not a vertex count.
// Every write is guarded by count so a short reservation is respected.
struct VB {
    unsigned char count;
    virtual void v0();
    virtual void Begin();
    virtual unsigned char Reserve(int);
    virtual void Color(unsigned char, int, int, int, int);
    virtual void Pos(unsigned char, V3);
    virtual void Shade(unsigned char, float, float);
    virtual void End(int, int);
};

// Course ids here are numeric resource ids; display-name mapping is not established.
// Caustic batches draw first even when the lit surface is suppressed. Surface policy:
// ids 8/11/12/13 skip it; the remaining ids skip it below the upper water plane.
// Alpha is 40/255 on id 10 and 80/255 otherwise (the id-8 alpha branch is unreachable).
enum WaterDrawPolicy {
    kCourseId8 = 8,
    kCourseId10 = 10,
    kCourseId11 = 11,
    kCourseId12 = 12,
    kCourseId13 = 13,
    kWaterLowAlpha = 0x28,
    kWaterNormalAlpha = 0x50
};
// Engine IDs and GX factor values are distinct domains (gx_renderer_state_and_efb_copy.cpp/gx_render_state_and_texture_upload.cpp).
enum WaterRenderStateId {
    kWaterDepthCompareState = 3,
    kWaterDepthWriteState = 5,
    kWaterSourceBlendState = 13,
    kWaterDestinationBlendState = 14
};
enum WaterRenderValue {
    kWaterBlendOne = 1,
    kWaterSourceAlpha = 4,
    kWaterInverseSourceAlpha = 5,
    kWaterCompareLessEqual = 3,
    kAlphaBlendedCausticLayers = 2,
    kAlphaBlendedCausticMode = 2
};
enum WaterTransformId {
    kWaterViewTransform = 0,
    kWaterProjectionTransform = 1,
    kWaterWorldTransform = 2,
    kWaterTextureTransform = 11
};

// Draw a flat quad or lit spring grid when the camera is at or above surface height.
// Required configured self, live camera/view/lighting/resource services and a live
// render context; spring path needs both row arrays and dimensions described at
// UpdateRipples. No ownership transfers. Short vertex reservations are respected;
// there is no status result or render-state restoration. See opening tuning map.
void GroundGrid_DrawLitSurface(void* self) {
    // 0x801C895C: the flat and spring paths share the same GroundGridStorage view.
    char* gridBytes = (char*)self;
    unsigned int* defaultStateWords = (unsigned int*)&geneTexRenderStateStorage;
    if (AsGroundGrid(gridBytes)->pointGeometry)
        GeneTexPointGeometry_DrawTextureBatches(AsGroundGrid(gridBytes)->pointGeometry);
    switch ((unsigned char)SharedDoc_GetCourseId()) {
    case kCourseId8:
    case kCourseId11:
    case kCourseId12:
    case kCourseId13:
        return;
    }
    if (*(float*)(s_cameraTransform + ((int)&((CameraTransform800D*)0)->position[1])) <
        AsGroundGrid(gridBytes)->boundsMax.y)
        return;
    if (WView_DefaultCapabilityTrueC(g_view, gridBytes + GRID_OFFSET(boundsMin))) {
        int vertexAlpha;
        if (SharedDoc_GetCourseId() == kCourseId8 || SharedDoc_GetCourseId() == kCourseId10)
            vertexAlpha = kWaterLowAlpha;
        else
            vertexAlpha = kWaterNormalAlpha;
        unsigned int packedLightColor = *(unsigned int*)(s_sceneLighting + ((int)&((GridSceneLighting*)0)->color));
        int lightRed = (packedLightColor >> 16) & 255, lightGreen = (packedLightColor >> 8) & 255,
            lightBlue = packedLightColor & 255;
        // Copy 15 (state,value) pairs plus 6 stage-state and 5 stage-parameter pairs.
        // These arrays are render commands; view/projection matrices come from the camera below.
        unsigned int renderStatePairs[30];
        *(struct RenderStatePairs15*)renderStatePairs = *(struct RenderStatePairs15*)defaultStateWords;
        unsigned int textureStateWords[22];
        struct TextureStagePairs6* textureStageDefaults = (struct TextureStagePairs6*)(defaultStateWords + 30);
        struct SamplerPairs5* textureParameterDefaults = (struct SamplerPairs5*)(defaultStateWords + 42);
        *(struct TextureStagePairs6*)textureStateWords = *textureStageDefaults;
        *(struct SamplerPairs5*)(textureStateWords + 12) = *textureParameterDefaults;
        Engine_SetRenderStateList(renderStatePairs, 15);
        Engine_SetTextureStageStateList(0, textureStateWords, 6);
        Engine_SetTextureStageParameterList(0, textureStateWords + 12, 5);
        void* renderer = Camera_GetRenderState(g_view);
        SetTransform(kWaterViewTransform, (char*)renderer + GRID_RENDER_OFFSET(viewMatrix));
        SetTransform(kWaterProjectionTransform, (char*)renderer + GRID_RENDER_OFFSET(projectionMatrix));
        Engine_SetType7Resource(0);
        ((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))->Begin();
        unsigned int specularTextureId = (*(unsigned int*)&AsGroundGrid(gridBytes)->specularTexture);
        void* textureManager = AsWView(g_view)->renderTarget;
        Engine_SetTextureStageResource(0, (int)((void* (*)(void*, unsigned int))(*(void***)textureManager)[0x70 / 4])(
                                              textureManager, specularTextureId));
        textureManager = AsWView(g_view)->renderTarget;
        Engine_SetTexturePalette(0, (int)((void* (*)(void*, unsigned int))(*(void***)textureManager)[0x74 / 4])(
                                        textureManager, specularTextureId));
        SetTransform(kWaterWorldTransform, kIdentityMatrix4x4);
        SetTransform(kWaterTextureTransform, gridBytes + GRID_OFFSET(textureTransform));
        Engine_SetTextureStageState(0, 0, 0);
        Engine_SetTextureStageState(0, 3, 0);
        Engine_SetTextureStageParameter(0, 0, 1);
        Engine_SetTextureStageParameter(0, 1, 1);
        Engine_SetCachedRenderState(kWaterSourceBlendState, kWaterSourceAlpha);
        Engine_SetCachedRenderState(kWaterDestinationBlendState, kWaterBlendOne);
        Engine_SetCachedRenderState(kWaterDepthWriteState, 0);
        Engine_SetCachedRenderState(kWaterDepthCompareState, kWaterCompareLessEqual);
        if (((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))->count != 4)
            ((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))->count =
                ((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))->Reserve(4);
        // Both paths use world-space V = normalize(cameraPosition - vertexPosition),
        // L = -sceneLighting.direction, H = normalize(L + V), and d = dot(H, normal).
        // Shade = d < 0 ? 1 : d*d. The negative case is deliberately full intensity;
        // its artistic rationale is unknown. This is not a Lambert diffuse clamp.
        // Flat normals are world +Y; spring normals are the cell's normalX/Y/Z.
        if ((*(unsigned int*)&AsGroundGrid(gridBytes)->surfaceFlags) & kFlatSurface) {
            float worldPosition[3];
            worldPosition[1] = AsGroundGrid(gridBytes)->boundsMax.y;
            for (int i = 0; i < 4; i++) {
                V3 localPosition;
                V3 relativePosition;
                V3 halfVector;
                V3 toLight;
                V3 toCamera;
                V3 surfaceNormal;
                worldPosition[0] =
                    (i & 2) ? AsGroundGrid(gridBytes)->boundsMax.x : AsGroundGrid(gridBytes)->boundsMin.x;
                worldPosition[2] =
                    (i & 1) ? AsGroundGrid(gridBytes)->boundsMax.z : AsGroundGrid(gridBytes)->boundsMin.z;
                // Upload center-relative geometry; use the world position worldPosition for lighting.
                __mi__FRC7WVectorRC7WVector(&relativePosition, worldPosition,
                                            (float*)(gridBytes + GRID_OFFSET(center)));
                *(PositionWords3*)&localPosition = *(PositionWords3*)&relativePosition;
                if (((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))->count > (unsigned char)i)
                    ((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))->Pos(i, localPosition);
                surfaceNormal.x = 0.0f;
                surfaceNormal.y = 1.0f;
                surfaceNormal.z = 0.0f;
                __mi__FRC7WVectorRC7WVector(&toCamera,
                                            (float*)(s_cameraTransform + ((int)&((CameraTransform800D*)0)->position)),
                                            worldPosition);
                V3* unitDirection = (V3*)Normalize__7WVectorFv(&toCamera);
                __ml__FfRC7WVector(&toLight, (float*)(s_sceneLighting + ((int)&((GridSceneLighting*)0)->direction)),
                                   -1.0f);
                __pl__FRC7WVectorRC7WVector(&halfVector, &toLight, unitDirection);
                unitDirection = (V3*)Normalize__7WVectorFv(&halfVector);
                float halfVectorShade = __ml__FRC7WVectorRC7WVector(unitDirection, &surfaceNormal);
                if (halfVectorShade < 0.0f)
                    halfVectorShade = 1.0f;
                else
                    halfVectorShade *= halfVectorShade;
                if (((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))->count > (unsigned char)i)
                    ((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))->Shade(i, halfVectorShade, halfVectorShade);
                if (((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))->count > (unsigned char)i)
                    ((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))
                        ->Color(i, vertexAlpha, lightRed, lightGreen, lightBlue);
            }
            int vertexCount = ((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))->count;
            if (vertexCount != 0)
                ((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))
                    ->End(kTriangleStripPrimitive, GX_GetPrimitiveCount(vertexCount, kTriangleStripPrimitive));
        } else {
            for (int row = 0; row < AsGroundGrid(gridBytes)->rowCount - 1; row++)
                for (int column = 0; column < AsGroundGrid(gridBytes)->columnCount - 1; column++) {
                    for (int cornerIndex = 0; cornerIndex < 4; cornerIndex++) {
                        V3 localPosition;
                        V3 halfVector;
                        V3 toLight;
                        V3 toCamera;
                        int cornerRowOffset = (cornerIndex & 1) ? 1 : 0;
                        // Strip corners: (row,column), (row+1,column),
                        // (row,column+1), (row+1,column+1). Geometry is center-relative;
                        // the matching spring cell carries world position and world normal.
                        *(PositionWords3*)&localPosition = *(PositionWords3*)&(
                            (PositionWords3*)AsGroundGrid(gridBytes)->localPositionRows[row + cornerRowOffset] +
                            column)[(cornerIndex & 2) ? 1 : 0];
                        SpringGridVertex* worldCell =
                            (SpringGridVertex*)((char*)AsGroundGrid(gridBytes)->springRows[row + cornerRowOffset] +
                                                (column + ((cornerIndex >> 1) & 1)) * sizeof(SpringGridVertex));
                        if (((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))->count > (unsigned char)cornerIndex)
                            ((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))->Pos(cornerIndex, localPosition);
                        __mi__FRC7WVectorRC7WVector(
                            &toCamera, (float*)(s_cameraTransform + ((int)&((CameraTransform800D*)0)->position)),
                            worldCell);
                        V3* unitDirection = (V3*)Normalize__7WVectorFv(&toCamera);
                        __ml__FfRC7WVector(
                            &toLight, (float*)(s_sceneLighting + ((int)&((GridSceneLighting*)0)->direction)), -1.0f);
                        __pl__FRC7WVectorRC7WVector(&halfVector, &toLight, unitDirection);
                        unitDirection = (V3*)Normalize__7WVectorFv(&halfVector);
                        float halfVectorShade = __ml__FRC7WVectorRC7WVector(unitDirection, &worldCell->normalX);
                        if (halfVectorShade < 0.0f)
                            halfVectorShade = 1.0f;
                        else
                            halfVectorShade *= halfVectorShade;
                        if (((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))->count > (unsigned char)cornerIndex)
                            ((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))
                                ->Shade(cornerIndex, halfVectorShade, halfVectorShade);
                        if (((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))->count > (unsigned char)cornerIndex)
                            ((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))
                                ->Color(cornerIndex, vertexAlpha, lightRed, lightGreen, lightBlue);
                    }
                    int vertexCount = ((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))->count;
                    if (vertexCount != 0)
                        ((VB*)(gridBytes + GRID_OFFSET(surfaceVertexCount)))
                            ->End(kTriangleStripPrimitive, GX_GetPrimitiveCount(vertexCount, kTriangleStripPrimitive));
                }
        }
    }
}

struct UV {
    float u, v;
};
// Layer color is packed AARRGGBB; DrawVtx stores four separate R,G,B,A bytes.
struct DrawItem {
    int pad0, pad4;
    unsigned int color;
    UV* uv;
    int pad10, pad14;
    unsigned int id;
};
struct DrawVtx {
    V3 pos;
    unsigned char r, g, b, a;
    UV uv;
};

// Upload each caustic layer's positions, color, and UVs, then draw its triangle fan.
// Required configured geometry: pointCount>=3, positions[pointCount], and parallel
// batches/graphicsBuffers arrays with batchCount entries. Each batch needs
// uv[pointCount], a valid texture ID and CPU-writable buffer for pointCount DrawVtx.
// Buffer pointers/UVs are borrowed for this call; flush publishes CPU writes to
// graphics. Graphics state changes survive return (see opening render-state map).
void GeneTexPointGeometry_DrawTextureBatches(void* self) {
    WaterRenderStateDefaults* renderStates;
    char* geometryBytes;
    PositionWords3 positionWords;
    RenderStatePairs15 renderStatePairs;
    SamplerPairs5 samplerPairs;
    TextureStagePairs6 textureStagePairs;
    void* cameraState;
    DrawItem* layer;
    unsigned int textureId;
    void* resourceManager;
    DrawVtx* buffer;
    int layerIndex;
    int pointIndex;
    renderStates = &geneTexRenderStateStorage;
    geometryBytes = (char*)self;
    if (!WView_DefaultCapabilityTrueC(g_view, self))
        return;
    // Phase 1: copy state/value packets; the camera supplies the actual matrices.
    renderStatePairs = renderStates->renderStates;
    textureStagePairs = renderStates->textureStages;
    samplerPairs = renderStates->textureParameters;
    Engine_SetRenderStateList(&renderStatePairs, 15);
    Engine_SetTextureStageStateList(0, &textureStagePairs, 6);
    Engine_SetTextureStageParameterList(0, &samplerPairs, 5);
    cameraState = Camera_GetRenderState(g_view);
    SetTransform(kWaterViewTransform, (char*)cameraState + GRID_RENDER_OFFSET(viewMatrix));
    SetTransform(kWaterProjectionTransform, (char*)cameraState + GRID_RENDER_OFFSET(projectionMatrix));
    Engine_SetType7Resource(0);
    // Virtual slot 3 begins the generated-geometry vertex buffer; receiver starts at count.
    ((void (*)(void*))(*(void***)(geometryBytes + GEOMETRY_OFFSET(vertexBufferVtable)))[3])(
        geometryBytes + GEOMETRY_OFFSET(vertexCount));
    SetTransform(kWaterWorldTransform, kIdentityMatrix4x4);
    SetTransform(kWaterTextureTransform, geometryBytes + GEOMETRY_OFFSET(textureTransform));
    Engine_SetTextureStageState(0, 0, 0);
    Engine_SetTextureStageState(0, 3, 0);
    Engine_SetTextureStageParameter(0, 0, 1);
    Engine_SetTextureStageParameter(0, 1, 1);
    Engine_SetCachedRenderState(kWaterDepthWriteState, 0);
    Engine_SetCachedRenderState(kWaterDepthCompareState, kWaterCompareLessEqual);
    // Per-layer storage is borrowed: lock/access CPU vertices, write XYZ/RGBA/UV,
    // flush, then bind the 24-byte stream and submit pointCount-2 fan triangles.
    for (layerIndex = 0; layerIndex < *(int*)(geometryBytes + GEOMETRY_OFFSET(batchCount)); ++layerIndex) {
        layer = *(DrawItem**)(geometryBytes + GEOMETRY_OFFSET(batches)) + layerIndex;
        textureId = layer->id;
        resourceManager = AsWView(g_view)->renderTarget;
        Engine_SetTextureStageResource(0, (int)((void* (*)(void*, unsigned int))(*(void***)resourceManager)[0x70 / 4])(
                                              resourceManager, textureId));
        resourceManager = AsWView(g_view)->renderTarget;
        Engine_SetTexturePalette(0, (int)((void* (*)(void*, unsigned int))(*(void***)resourceManager)[0x74 / 4])(
                                        resourceManager, textureId));
        Resource_GetOffsetPtr(((void**)*(void***)(geometryBytes + GEOMETRY_OFFSET(graphicsBuffers)))[layerIndex], 0, 0,
                              (void**)&buffer, 0);
        for (pointIndex = 0; pointIndex < *(int*)(geometryBytes + GEOMETRY_OFFSET(pointCount)); ++pointIndex) {
            positionWords = ((PositionWords3*)*(void***)(geometryBytes + GEOMETRY_OFFSET(positions)))[pointIndex];
            Vector_CopyComponents(&buffer[pointIndex], &positionWords);
            buffer[pointIndex].uv.u = layer->uv[pointIndex].u;
            buffer[pointIndex].uv.v = layer->uv[pointIndex].v;
            buffer[pointIndex].a = layer->color >> 24;
            buffer[pointIndex].r = layer->color >> 16;
            buffer[pointIndex].g = layer->color >> 8;
            buffer[pointIndex].b = layer->color;
        }
        GraphicsBuffer_Flush(((void**)*(void***)(geometryBytes + GEOMETRY_OFFSET(graphicsBuffers)))[layerIndex]);
        if (layerIndex < kAlphaBlendedCausticLayers && SharedDoc_GetCourseId() == kCourseId13 &&
            *(int*)(geometryBytes + GEOMETRY_OFFSET(renderMode)) == kAlphaBlendedCausticMode) {
            Engine_SetCachedRenderState(kWaterSourceBlendState, kWaterSourceAlpha);
            Engine_SetCachedRenderState(kWaterDestinationBlendState, kWaterInverseSourceAlpha);
        } else {
            Engine_SetCachedRenderState(kWaterSourceBlendState, kWaterSourceAlpha);
            Engine_SetCachedRenderState(kWaterDestinationBlendState, kWaterBlendOne);
        }
        Engine_SetVertexStream(0, ((void**)*(void***)(geometryBytes + GEOMETRY_OFFSET(graphicsBuffers)))[layerIndex], 0,
                               sizeof(DrawVtx));
        DrawPrimitiveFromBoundBuffers(kTriangleFanPrimitive, 0,
                                      *(int*)(geometryBytes + GEOMETRY_OFFSET(pointCount)) - 2);
    }
}

// Release and clear the ground grid's cached surface allocation.
void GroundGrid_ClearCachedSurface(void* self, int unusedArg) {
    void** field = (void**)((char*)self + GRID_OFFSET(cachedSurface));
    if (*field != 0) {
        __dl__FPv(*field);
        *field = 0;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Copy a 4x4 texture transform into generated point geometry, preserving word-copy order.
// Required destination and readable source[16]. Exact source==destination transform
// is permitted; other overlapping ranges are not a buffered-copy contract.
void GeneTexPointGeometry_SetTextureTransformWords(unsigned int* self, const unsigned int* source) {

    // Source is a borrowed array of 16 IEEE-754 words; retain word-copy ordering.
    enum {
        kTextureTransformWord = GEOMETRY_OFFSET(textureTransform) / sizeof(unsigned int)
    };
    self[kTextureTransformWord + 0] = source[0];
    self[kTextureTransformWord + 1] = source[1];
    self[kTextureTransformWord + 2] = source[2];
    self[kTextureTransformWord + 3] = source[3];
    self[kTextureTransformWord + 4] = source[4];
    self[kTextureTransformWord + 5] = source[5];
    self[kTextureTransformWord + 6] = source[6];
    unsigned int savedWord7 = source[7];
    self[kTextureTransformWord + 7] = savedWord7;
    self[kTextureTransformWord + 8] = source[8];
    self[kTextureTransformWord + 9] = source[9];
    self[kTextureTransformWord + 10] = source[10];
    self[kTextureTransformWord + 11] = source[11];
    self[kTextureTransformWord + 12] = source[12];
    self[kTextureTransformWord + 13] = source[13];
    self[kTextureTransformWord + 14] = source[14];
    self[kTextureTransformWord + 15] = source[15];
}
#pragma pop

} // extern "C"
struct CopyWord3 {
    unsigned int a, b, c;
    // Return borrowed placement storage for copying one three-word position.
    static void* operator new(new_size_t, void* p) {
        return p;
    }
    // Copy XYZ storage words without changing the original load/store order.
    CopyWord3(const CopyWord3& x) {
        unsigned int first = x.a;
        unsigned int second = x.b;
        a = first;
        b = second;
        c = x.c;
    }
};
struct CopyRef {
    const void* vt;
    int refs, owners;
    char* data;
    // Attach error text to a shared control block initialized with one owner.
    CopyRef(char* p) {
        refs = 1;
        owners = 1;
        vt = s_sharedCharDeleterVtable;
        data = p;
    }
};
struct CopyGuard_u801C8124 {
    char* p;
    void* allocator;
    // Free uncommitted error-text storage during exception construction.
    ~CopyGuard_u801C8124() {
        if (p)
            __dla__FPv(p);
    }
};
struct CopyErrorBase {
    const void* vt;
    char* text;
    CopyRef* ref;
    // Destroy the standard exception base of this local error wrapper.
    ~CopyErrorBase() {
        __dt__Q23std9exceptionFv(this);
    }
};
struct CopyError : CopyErrorBase {
    // Copy a borrowed diagnostic into shared storage and install length_error.
    CopyError(const char* s) {
        vt = __vt__Q23std11logic_error;
        char allocator = 0;
        unsigned int n = strlen(s) + 1;
        char* buffer = (char*)__nwa__FUl(n * 2 - n);
        text = buffer;
        {
            CopyGuard_u801C8124 guard = {buffer, &allocator};
            ref = new CopyRef(buffer);
            guard.p = 0;
        }
        strcpy(text, s);
        vt = __vt__Q23std12length_error;
    }
};

extern "C" {
// Deep-copy a vector of three-word positions, checking allocation overflow.
// self is unconstructed {data,size,capacity} storage disjoint from input and its
// readable count*12-byte buffer. Return self; source is borrowed, fresh data is
// owned and released by dtor_801247CC. Allocation must succeed or throw; null is
// not checked. On exception the destination may be partially initialized.
void* Vec3Vector_CopyCtor(void* self, const void* input) {
    unsigned int* out = (unsigned int*)self;
    const unsigned int* src = *(const unsigned int* const*)input;
    unsigned int count = *((const unsigned int*)input + 1);
    const unsigned int* cur = src;
    const unsigned int* end = (const unsigned int*)((const char*)src + count * 12);
    out[0] = 0;
    out[1] = 0;
    out[2] = 0;
    unsigned int n = (const CopyWord3*)end - (const CopyWord3*)cur;
    if (n) {
        if (n > kVec3VectorMaxElements) {
            CopyError error(waterCausticStrings + ((int)&((WaterCausticStrings*)0)->vectorLengthError));
            __throw((void*)(waterCausticStrings + ((int)&((WaterCausticStrings*)0)->lengthErrorType)), &error,
                    (void*)__dt__Q23std12length_errorFv);
        }
        unsigned int* fresh = (unsigned int*)appMallocDummy__FUl(n * 12);
        out[0] = (unsigned int)fresh;
        out[2] = n;
        unsigned int* dst = (unsigned int*)((char*)fresh + out[1] * 12);
        while (cur != end) {
            new (dst) CopyWord3(*(const CopyWord3*)cur);
            cur += 3;
            dst += 3;
            ++out[1];
        }
    }
    return self;
}
}

struct Dtor47CC {
    unsigned int a, b, c;
    // Release the temporary position vector without deleting this stack wrapper.
    ~Dtor47CC() {
        dtor_801247CC(this, -1);
    }
};
struct M6 {
    unsigned int v[6];
};
struct Mtx16 {
    float m[16];
};

extern "C" {
#pragma dont_inline on
// Parse surface type/rotation, register ground bounds, and build the centered four-corner geometry.
// Required constructed self, readable world-space min/max bounds and NUL-terminated
// property text. No pointer retained to caller bounds/text. Flags are ORed, not
// reset; *noripple overrides *type. Missing tokens preserve defaults (type 0,
// rotation zero); validation/range rejection is not provided at this boundary.
// Stores bounds/center/type/flags/texture transform, appends a terrain region unless
// skipped, initializes springs unless flat, and invalidates cachedSurface. Geometry
// receives copies of corner positions. Temporary vectors release storage on unwind;
// earlier member/region mutations are not rolled back on allocation failure.
void GroundGrid_ConfigureSurfaceBounds(void* self, void* bounds, void* properties) {
    GroundGridStorage* grid = (GroundGridStorage*)self;
    // Phase 1: copy world bounds/center, parse surface policy and rotation (degrees).
    AABB_Copy(&grid->boundsMin, bounds);
    grid->surfaceType = kRippleSurface;
    grid->center.x = (grid->boundsMax.x + grid->boundsMin.x) * kFloat_0_5_Collision;
    grid->center.y = (grid->boundsMax.y + grid->boundsMin.y) * kFloat_0_5_Collision;
    grid->center.z = (grid->boundsMax.z + grid->boundsMin.z) * kFloat_0_5_Collision;
    char propertyString[4];
    BorrowedNameRef_Set(propertyString, properties);
    String_ScanTokensAfterSubstring(propertyString, ((const WaterCausticStrings*)waterCausticStrings)->typeToken,
                                    ((const WaterCausticStrings*)waterCausticStrings)->integerFormat,
                                    &grid->surfaceType);
    float rotationDegrees = kFloat_0_805DF6F4;
    String_ScanTokensAfterSubstring(propertyString, ((const WaterCausticStrings*)waterCausticStrings)->rotationToken,
                                    ((const WaterCausticStrings*)waterCausticStrings)->floatFormat, &rotationDegrees);
    if (strstr(properties, ((const WaterCausticStrings*)waterCausticStrings)->noRippleToken))
        grid->surfaceType = kStillSurface;
    switch (grid->surfaceType) {
    case kStillSurface:
        BitFlags_OrInPlace((unsigned int*)&grid->surfaceFlags, kDisableRandomRipples | kFlatSurface);
        break;
    case kExcludedSurface:
        // The extra type-2 bit is retained; no consumer in this unit explains it.
        BitFlags_OrInPlace((unsigned int*)&grid->surfaceFlags, kDisableRandomRipples | kSkipGroundRegion | 8);
        break;
    case kRippleSurface:
    default:
        break;
    }
    // Phase 2: register terrain bounds when requested and initialize any spring surface.
    if (!BitFlags_TestAny((unsigned int*)&grid->surfaceFlags, kSkipGroundRegion)) {
        M6 regionBounds = *(M6*)&grid->boundsMin;
        GroundRegionList_AppendBounds(CQuadTree_GetInstance(), &regionBounds, kGroundRegionType);
    }
    if (!BitFlags_TestAny((unsigned int*)&grid->surfaceFlags, kFlatSurface))
        GroundGrid_InitializeTwoByTwo(self);
    GroundGrid_ClearCachedSurface(self, 1);
    PSMTX44Identity(grid->textureTransform);
    // Phase 3: convert degrees to radians for the texture transform; build world-space
    // corners on the upper bounds plane. Generated geometry receives copies, not this vector.
    Mtx16 rotationMatrix, translationMatrix;
    Mtx44_SetEulerRotationZXY(&rotationMatrix, kFloat_0_017453292_GroundGrid * rotationDegrees, kFloat_0_805DF6F4,
                              kFloat_0_805DF6F4);
    Mtx44_SetTranslation(&translationMatrix, grid->center.x, grid->center.y, grid->center.z);
    PSMTX44Concat(grid->textureTransform, &rotationMatrix, &translationMatrix);
    Dtor47CC corners;
    Vec3Vector_Init(&corners.a);
    char t1[12], t2[12], t3[12], t4[12];
    Vec3Vector_AppendCopy(&corners, Vec3_Set(t1, grid->boundsMax.x, grid->boundsMax.y, grid->boundsMax.z));
    Vec3Vector_AppendCopy(&corners, Vec3_Set(t2, grid->boundsMax.x, grid->boundsMax.y, grid->boundsMin.z));
    Vec3Vector_AppendCopy(&corners, Vec3_Set(t3, grid->boundsMin.x, grid->boundsMax.y, grid->boundsMin.z));
    Vec3Vector_AppendCopy(&corners, Vec3_Set(t4, grid->boundsMin.x, grid->boundsMax.y, grid->boundsMax.z));
    if (grid->pointGeometry) {
        GeneTexPointGeometry_SetTextureTransformWords((unsigned int*)grid->pointGeometry, grid->textureTransform);
        GeneTexPointGeometry_SetCenter(grid->pointGeometry, &grid->center);
        Dtor47CC copiedCorners;
        Vec3Vector_CopyCtor(&copiedCorners, &corners);
        ((CGeneTex3D*)(grid->pointGeometry))->InitializePointGeometry(&copiedCorners, grid->surfaceType);
    }
}
#pragma dont_inline off

} // extern "C"
