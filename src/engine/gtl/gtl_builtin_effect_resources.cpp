// Translation unit 0x80433528-0x80434998 (9 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>

extern "C" {

extern char s_builtinType6ResourceCache[];
extern char s_textureProjectionState[];
struct S44 { unsigned int w[17]; };
struct V6 { int x, y, width, height; float nearDepth, farDepth; };
struct RadialCaptureState {
    void* vertexBuffer;
    void* texture;
    S44 material;
    float projection[16];
    V6 viewport;
};
extern RadialCaptureState s_radialCaptureRenderState;
enum BuiltinRenderConstants {
    kBuiltinEffectNone = 0, kBuiltinEffectRadialCapture = 1,
    kBuiltinEffectDefaultSetup = 4, kBuiltinEffectProjectionTexture = 5,
    kCaptureTextureSide = 256, kRadialGridVertices = 0x6000,
    kRadialGridSide = 64, kRadialCaptureTextureFormat = 0x80000006,
    kProjectionTextureFormat = 0x80000003,
    kBuiltinCacheSlots = 7, kBuiltinCacheNotFound = 8,
    kTriangleStripPrimitive = 0x98, kEffectVertexFormat = 0x401,
    kEffectVertexStride = 0x18, kRadialRowShift = 6, kCornersPerAxis = 2,
    kFirstStripCorner = 1, kLastStripEndpoint = 5, kLastCellCorner = 3,
    kProjectionPixelBytes = 2, kProjectionRowShift = 9, kProjectionCoordinateBias = 3,
    kCaptureTextureOffset = (int)&((RadialCaptureState*)0)->texture
};
extern const S44 kRadialCaptureDefaultMaterialWords;
extern const V6 kRadialCaptureDefaultViewport;
extern float kFloat_neg1_EngineRender, kFloat_1_EngineRender, kRadialCaptureNearPlane, kRadialCaptureFarPlane, kFloat_2_EngineRender, kFloat_0_015625_EngineRenderGrid, kFloat_neg5_RadialCaptureGrid, kFloat_1_25_EngineRender;
extern const float kFloat_0_Framebuffer;
extern void GraphicsResource_CreateType6(int*, void*);
extern void MtxOrthoOffCenter(void*, float, float, float, float, float, float);
extern void Texture_Create2D(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, void*, unsigned int);
extern void RenderMaterial_Copy(void*, void*);
extern void GtlRecordDeque_InitEmpty(void*);
extern void WordBlock_CopySixWords(void*, void*);
extern void GtlRecordDeque_CopyConstructElement(void*, void*, void*);
extern unsigned int TextureFormat_GetRGBA8OrIA8BytesPerTexel(unsigned int);
extern unsigned int ByteBuffer_CalculateGrowthCapacity(void*, unsigned int);
extern void std_ThrowLengthError(void);
extern void* gtl_Graphics_AllocateWithCallback(unsigned int, void*);
extern void VertexBuffer_Create(unsigned int, unsigned int, unsigned int, unsigned int, void*, unsigned int);
extern void Resource_GetOffsetPtr(void*, int, int, void*, int);
extern void GraphicsBuffer_Flush(void*);
extern void Texture_LockLevel(void*, int, void*, int, int);
extern void Image_CopyToPlatformLayoutFormat7(void*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
extern void Texture_UnlockLevel(void*, int);
extern void GtlRecordDeque_Dtor(void*, short);
extern void ByteBuffer_DeletingDtor(void*, short);
extern void dtor_8042AE7C(void*, short);
extern void dtor_804359C4(void*, short);
extern void* GtlRecordDeque_CalculateGrowthCapacity(void*, unsigned int);
extern void GtlRecordDeque_Initialize(void*, void*);
extern void GtlRecordDeque_AllocateStorage(void*, unsigned int, void*);
extern void GtlRecordDeque_PrependRange(void*, void*, void*);
extern void Pointer_Swap(void*, void*);
extern void Swap_Int32(void*, void*);
extern float Math_SquareRootApprox(float);
extern void* memcpy(void* dst, const void* src, unsigned int n);
extern void* memset(void* dst, int c, unsigned int n);

// x/y/z are capture-plane positions; u/v/w are the projected coordinates described below.
struct RadialVertex { float x, y, z, u, v, w; };
struct RadialStripCell { RadialVertex v[6]; };
// Projected x/y are scaled grid positions; height is the upper unit hemisphere.
struct RadialProjectedCoordinates { float x, y, hemisphereHeight; };
struct Capacity {
    unsigned int value;
    // Expose the capacity word by reference to the original allocator helpers.
    operator const unsigned int&() const { return value; }
    // Assign the stored capacity without changing ownership.
    Capacity& operator=(unsigned int v) { value = v; return *this; }
};
struct VecE { RadialVertex* begin; unsigned int size; Capacity cap;
    // Release temporary vertex storage after its contents reach the GPU buffer.
    ~VecE() { GtlRecordDeque_Dtor(this, 0); VecE* destroyed = this; }
};
// Keep VecE's emitted destructor symbol; use its role alias in effect code.
typedef VecE RadialVertexBuffer;
struct VertexReplacementBuffer { RadialVertex* begin; unsigned int size; unsigned int cap; void* alloc; unsigned int pos; };
struct ProjectionByteBuffer { unsigned char* begin; unsigned int size; Capacity cap; };
// Replacement byte storage owns data until the swap installs it in ProjectionByteBuffer.
// constructedBytes tracks live bytes; liveBeginOffsetBytes moves backward while prepending.
// After the swap this guard owns the displaced allocation and releases only that storage.
struct ByteReplacementBuffer { unsigned char* data; unsigned int constructedBytes; unsigned int capacityBytes; void* alloc; unsigned int liveBeginOffsetBytes; };
// referenceCount is the retained reference count; the low half is not interpreted in this unit.
struct ResourceReferenceHeader { CW_BITS2( unsigned int referenceCount : 16, unsigned int lo : 16) };
struct Vec3Words { unsigned int w[3]; };

#pragma optimization_level 0
#pragma optimize_for_size on
// Lazily build a built-in render effect, including radial-grid capture and projection textures.
// index is an unchecked cache index (seven slots); out must be writable. Zero
// returns a null resource. A nonzero result is retained once for the caller via
// the resource header's high 16-bit reference count; the cache keeps its pointer.
// Cases 1 and 5 create extra resources below; other indices use factory defaults.
// Visual controls: kRadialGridSide sets tessellation; kFloat_0_015625_EngineRenderGrid
// is 1/64 of the grid, kFloat_neg5_RadialCaptureGrid sets plane depth, and
// kFloat_1_25_EngineRender scales projected x/y. Keep floating operation order.
int GraphicsResource_AcquireBuiltinEffect(int index, void** out) {
    if (index == kBuiltinEffectNone) { *out = 0; return 1; }
    __alloca(0);
    void** slot = (void**)s_builtinType6ResourceCache + index;
    if (*slot == 0) {
        // Cache miss: create the type-6 resource before effect-specific resources.
        GraphicsResource_CreateType6(&index, slot);
        switch (index) {
        case kBuiltinEffectRadialCapture: {
                // Capture target/material/viewport use an orthographic [-1,1] grid.
                MtxOrthoOffCenter(s_radialCaptureRenderState.projection, kFloat_neg1_EngineRender, kFloat_1_EngineRender, kFloat_1_EngineRender, kFloat_neg1_EngineRender, kRadialCaptureNearPlane, kRadialCaptureFarPlane);
                Texture_Create2D(kCaptureTextureSide, kCaptureTextureSide, 1, 0, kRadialCaptureTextureFormat, 0, &s_radialCaptureRenderState.texture, 0);
                S44 s44 = kRadialCaptureDefaultMaterialWords;
                RenderMaterial_Copy(&s_radialCaptureRenderState.material, &s44);
                RadialVertexBuffer vec;
                V6 v6 = kRadialCaptureDefaultViewport;
                s_radialCaptureRenderState.viewport.x = v6.x;
                s_radialCaptureRenderState.viewport.y = v6.y;
                s_radialCaptureRenderState.viewport.width = v6.width;
                s_radialCaptureRenderState.viewport.height = v6.height;
                s_radialCaptureRenderState.viewport.nearDepth = v6.nearDepth;
                s_radialCaptureRenderState.viewport.farDepth = v6.farDepth;
                GtlRecordDeque_InitEmpty(&vec);
                // Storage expansion stays at this scope; visual controls continue immediately below.
#include "game/u80433528_radial_storage.inc"
                RadialVertex* data = vec.begin;
                RadialStripCell* cells = (RadialStripCell*)data;
                // Four corners per cell plus repeated endpoints join the cells
                // in a strip. Positions span [-1,1] at z=-5; texture coordinates
                // are (1.25*x, 1.25*y, sqrt(max(1-x*x-y*y, 0))). The third
                // coordinate is the upper unit-hemisphere height inside the unit disk;
                // it falls to zero outside. Grid positions stay on the z=-5 plane.
                for (int y = 0; y < kRadialGridSide; y++) {
                    for (int x = 0; x < kRadialGridSide; x++) {
                        RadialVertex* corners = &(cells + (x + (y << kRadialRowShift)))->v[kFirstStripCorner];
                        for (int a = 0; a < kCornersPerAxis; a++) {
                            for (int b = 0; b < kCornersPerAxis; b++) {
                                RadialVertex* e = corners + b + a * kCornersPerAxis;
                                RadialProjectedCoordinates* projected = (RadialProjectedCoordinates*)&e->u;
                                e->x = kFloat_2_EngineRender * (float)(x + b) * kFloat_0_015625_EngineRenderGrid - kFloat_1_EngineRender;
                                e->y = kFloat_2_EngineRender * (float)(y + a) * kFloat_0_015625_EngineRenderGrid - kFloat_1_EngineRender;
                                e->z = kFloat_neg5_RadialCaptureGrid;
                                RadialVertex* position = e;
                                projected->x = kFloat_1_25_EngineRender * e->x;
                                projected->y = kFloat_1_25_EngineRender * e->y;
                                float radiusSquared = e->x * e->x + e->y * e->y;
                                float hemisphereHeight;
                                if (radiusSquared < kFloat_1_EngineRender)
                                    hemisphereHeight = Math_SquareRootApprox(kFloat_1_EngineRender - radiusSquared);
                                else
                                    hemisphereHeight = kFloat_0_Framebuffer;
                                projected->hemisphereHeight = hemisphereHeight;
                            }
                        }
                        *(Vec3Words*)&cells[x + (y << kRadialRowShift)].v[0].x = *(Vec3Words*)&corners->x;
                        *(Vec3Words*)&cells[x + (y << kRadialRowShift)].v[0].u = *(Vec3Words*)&corners->u;
                        RadialVertex* last = &cells[x + (y << kRadialRowShift)].v[kLastStripEndpoint];
                        RadialStripCell* destination = &cells[x + (y << kRadialRowShift)];
                        WordBlock_CopySixWords(last, corners + kLastCellCorner);
                    }
                }
                // Upload a packed 24-byte vertex stream; vec remains CPU-owned.
                unsigned int bytes = kRadialGridVertices * kEffectVertexStride;
                VertexBuffer_Create(bytes, 0, kEffectVertexFormat, 0, &s_radialCaptureRenderState.vertexBuffer, 0);
                void* locked;
                Resource_GetOffsetPtr(s_radialCaptureRenderState.vertexBuffer, 0, 0, &locked, 0);
                Memory_Copy(locked, cells, bytes);
                GraphicsBuffer_Flush(s_radialCaptureRenderState.vertexBuffer);
                break;
            }
        case kBuiltinEffectDefaultSetup:
            break;
        case kBuiltinEffectProjectionTexture: {
                // Create a 256-square two-byte coordinate texture, first in CPU
                // row order, then converted into the platform texture layout.
                unsigned int fmt = kProjectionTextureFormat;
                Texture_Create2D(kCaptureTextureSide, kCaptureTextureSide, 1, 0, fmt, 0, s_textureProjectionState, 0);
                unsigned int n = TextureFormat_GetRGBA8OrIA8BytesPerTexel(fmt);
                ProjectionByteBuffer vb;
                // Storage expansion stays at this scope; visual controls continue immediately below.
#include "game/u80433528_projection_storage.inc"
                unsigned char* raw = vb.begin;
                unsigned char* data = raw;
                unsigned char* base = data;
                // Store x/y byte coordinates with the existing three-unit bias;
                // the first three coordinates on either axis retain their value.
                for (unsigned int y = 0; y < kCaptureTextureSide; y++) {
                    for (unsigned int x = 0; x < kCaptureTextureSide; x++) {
                        unsigned char* pixel = base + x * kProjectionPixelBytes + (y << kProjectionRowShift);
                        unsigned char* p = pixel;
                        p[0] = x >= kProjectionCoordinateBias ? x - kProjectionCoordinateBias : x;
                        p[1] = y >= kProjectionCoordinateBias ? y - kProjectionCoordinateBias : y;
                    }
                }
                unsigned int pair[2];
                Texture_LockLevel(*(void**)s_textureProjectionState, 0, pair, 0, 0);
                Image_CopyToPlatformLayoutFormat7(base, n << 8, 0, pair[1], kCaptureTextureSide, kCaptureTextureSide, 0, fmt);
                Texture_UnlockLevel(*(void**)s_textureProjectionState, 0);
                ByteBuffer_DeletingDtor(&vb, -1);
                break;
            }
        }
    }
    // Both cache hits and successful creation return one retained resource.
    volatile ResourceReferenceHeader* o = (volatile ResourceReferenceHeader*)(*out = *slot);
    o->referenceCount = o->referenceCount + 1;
    unsigned int referenceCount = o->referenceCount;
    (void)referenceCount;
    return 1;
}
#pragma optimization_level reset
#pragma optimize_for_size reset

// Copy float slots from the encoded start index through start plus count; pointers encode indices here.
extern int FloatArray_CopyIndexedRange(void* a, void* b, void* c, unsigned int d);
extern char s_builtinType7ResourceCache[];
extern char s_builtinEffectProjectionSettings[];
extern char s_textureProjectionSettings[];
struct Blk18 {
    unsigned long long w[3];
};

extern const Blk18 kIndirectBumpXYZMatrix;

extern void GraphicsResource_CreateType7(void* a, void** b);
extern void Engine_SaveRenderState(void);
extern void Engine_ClearFramebuffer(int, int, int, int, int, float);
extern void Engine_SetCachedRenderState(int, int);
extern void Engine_SetMaterial(void*);
extern void Engine_SetTextureStageResource(int, void*);
extern void Engine_SetTexturePalette(int, void*);
extern void Engine_SetFlexibleVertexFormat(int);
extern void Engine_SetType6Resource(int);
extern void Engine_SetType7Resource(int);
extern void Engine_GetTransform(int, void*);
extern void Mtx44_InvertLegacyCofactors(void*, void*, void*);
extern void SetTransform(int, void*);
extern void Engine_SetViewport(void*);
extern void Engine_SetVertexStream(int, void*, int, int);
extern void* GX_GetPrimitiveCount(int, int);
extern void DrawPrimitiveFromBoundBuffers(int, int, void*);
// 0x80434630..0x80434654: the original passes 11 integer arguments (r3-r10 plus three stack words at 0x8/0xc/0x10, all 0);
// the callee (gx_renderer_state_and_efb_copy.cpp) stores its last two into engine state at +0x14CC/+0x14D0.
extern void Engine_CopyFramebufferToTexture(int, int, void*, int, int, int, int, int, int, int, int);
extern void Engine_RestoreRenderState(void);
extern void Engine_SetTextureStageParameterList(int, void*, int);
extern void PSMTX44Concat(void*, void*, void*);
extern void PSMTX44Transpose(void*, void*);
extern void Mtx44_SetTranslation(void*, float, float, float);
extern void Mtx44_SetScale(void*, float, float, float);
extern const float kFloat_0_5_Framebuffer;
extern const float kFloat_0_4_Framebuffer;
extern void GXSetIndTexMtx(int a, void* b, signed char c);
extern void GXSetNumIndStages(int a);
extern void GXSetIndTexOrder(int a, int b, int c);
extern void GXSetIndTexCoordScale(int a, int b, int c);
extern void GXSetTevDirect(int a);
extern void GXSetTevOp(int a, int b);
extern void GXTev_SetIndirectBumpXYZ(int a, int b, int c);

struct Obj342D0 {
    CW_BITS2(
        unsigned int referenceCount : 16,
        unsigned int lo : 16)
};

// Find or create a cached type-7 resource and increment its packed reference count.
int GraphicsResource_AcquireCachedType7(unsigned int a, void** out) {
    volatile unsigned int idx = a;
    if (a == 0) {
        *out = 0;
        return 1;
    }
    void** table = (void**)s_builtinType7ResourceCache;
    void** slot = &table[idx];
    if (*slot == 0)
        GraphicsResource_CreateType7((void*)&idx, slot);
    Obj342D0* o = (Obj342D0*)*slot;
    *out = (void*)o;
    volatile Obj342D0* vo = (volatile Obj342D0*)o;
    unsigned int referenceCount = (vo->referenceCount = vo->referenceCount + 1);
    return referenceCount ? 1 : 1;
}

#pragma optimize_for_size on
// Configure four TEV stages for XYZ indirect bump mapping.
// Stage 0 uses GX_REPLACE; stages 1/2 use GX_PASSCLR while the bump helper
// configures indirect coordinates. The explicit stage-3 combiner computes
// previous RGB * texture RGB and preserves previous alpha. Matrix coefficients
// remain the shared defaults; color routing is selected by the TevOrder calls.
void Render_SetupIndirectBumpXYZTev(void) {
    Blk18 b;
    struct Pair { unsigned int a, c; };
    struct Copy { unsigned int pad; Pair data; };
    Copy* src = (Copy*)((char*)&kIndirectBumpXYZMatrix - 4);
    Copy* dst = (Copy*)((char*)&b - 4);
    for (int i = 0; i < 3; ++i) {
        dst->data = src->data;
        src = (Copy*)((char*)src + 8);
        dst = (Copy*)((char*)dst + 8);
    }
    GXSetIndTexMtx(1, &b, (signed char)0);
    GXSetNumTevStages(4);
    GXSetNumIndStages(1);
    GXSetIndTexOrder(0, 0, 1);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetTevDirect(0);
    GXSetTevOrder(0, 0, 0, 0xff);
    GXSetTevOp(0, GX_REPLACE);
    GXTev_SetIndirectBumpXYZ(1, 0, 1);
    GXSetTevOrder(1, 1, 0x102, 0xff);
    GXSetTevOp(1, GX_PASSCLR);
    GXSetTevOrder(2, 2, 0x102, 0xff);
    GXSetTevOp(2, GX_PASSCLR);
    GXSetTevOrder(3, 3, 2, 0xff);
    GXSetTevColorIn(3, 0xf, 0, 8, 0xf);
    GXSetTevAlphaIn(3, 7, 7, 7, 0);
    GXSetTevColorOp(3, 0, 0, 0, (GXBool)1, 0);
    GXSetTevAlphaOp(3, 0, 0, 0, (GXBool)1, 0);
}
#pragma optimize_for_size reset

// Find a type-6 resource in the seven-slot cache, returning eight if absent.
unsigned int GraphicsResource_FindCachedType6Index(void* p) {
    if (p == 0) return 0;
    volatile unsigned int i;
    for (i = 0; i < kBuiltinCacheSlots; i++) {
        if (((volatile void**)s_builtinType6ResourceCache)[i] == p) return i;
    }
    return kBuiltinCacheNotFound;
}

// Find a type-7 resource in the seven-slot cache, returning eight if absent.
unsigned int GraphicsResource_FindCachedType7Index(void* p) {
    if (p == 0) return 0;
    volatile unsigned int i;
    for (i = 0; i < kBuiltinCacheSlots; i++) {
        if (((volatile void**)s_builtinType7ResourceCache)[i] == p) return i;
    }
    return kBuiltinCacheNotFound;
}

// Copy 4*vectorCount floats into built-in projection settings, starting at startIndex.
// startIndex is encoded in a pointer-sized argument, never dereferenced; source
// and destination use the SAME float index. Counts here are four-float groups.
void BuiltinEffect_CopyProjectionFloatRange(void* startIndex, void* source, unsigned int vectorCount) {
    volatile unsigned int count = vectorCount;
    FloatArray_CopyIndexedRange(startIndex, source, s_builtinEffectProjectionSettings + 0x80, count * 4);
}

#pragma inline off
// Copy floatCount individual floats at [startIndex, startIndex+floatCount); return 1.
// startIndex is a 32-bit integer encoded as void*, not an address. Both borrowed
// arrays must contain that range, and the unsigned end index must not overflow.
// Copy proceeds forward (overlapping ranges are not given memmove semantics).
int FloatArray_CopyIndexedRange(void* startIndex, void* source, void* destination, unsigned int floatCount) {
    volatile unsigned int count = floatCount;
    unsigned int i = (unsigned int)startIndex;
    volatile float* src = (volatile float*)source;
    volatile float* dst = (volatile float*)destination;
    while (i < (unsigned int)startIndex + count) {
        dst[i] = src[i];
        i++;
    }
    return 1;
}

#pragma inline off
#pragma optimize_for_size on
// Copy vectorCount four-float groups to texture projection settings at a float index.
// Uses the same encoded-index, borrowed-array and range rules as the built-in wrapper.
void TextureProjection_CopySettingsFloatRange(void* startIndex, void* source, unsigned int vectorCount) {
    volatile unsigned int count = vectorCount;
    FloatArray_CopyIndexedRange(startIndex, source, s_textureProjectionSettings + 0x80, count * 4);
}
#pragma optimize_for_size reset

// Draw the radial capture into its texture and rebuild projected texture transforms.
void Render_UpdateRadialCapture(void) {
    // Save drawing state, render the grid into the capture target, and restore it.
    Engine_SaveRenderState();
    Engine_ClearFramebuffer(0, 0, 7, 0, 0, kFloat_1_EngineRender);
    Engine_SetCachedRenderState(8, 0);
    Engine_SetCachedRenderState(3, 3);
    Engine_SetCachedRenderState(6, 1);
    Engine_SetMaterial(&s_radialCaptureRenderState.material);
    for (unsigned int i = 0; i < 8; i++) {
        Engine_SetTextureStageResource(i, 0);
        Engine_SetTexturePalette(i, 0);
    }
    Engine_SetFlexibleVertexFormat(kEffectVertexFormat);
    Engine_SetType6Resource(0);
    Engine_SetType7Resource(0);
    unsigned char m[0x40];
    Engine_GetTransform(0, m);
    Mtx44_InvertLegacyCofactors(m, 0, m);
    SetTransform(0xb, m);
    SetTransform(1, s_radialCaptureRenderState.projection);
    Engine_SetViewport(&s_radialCaptureRenderState.viewport);
    Engine_SetVertexStream(0, s_radialCaptureRenderState.vertexBuffer, 0, kEffectVertexStride);
    void* buf = GX_GetPrimitiveCount(kRadialGridVertices, kTriangleStripPrimitive);
    DrawPrimitiveFromBoundBuffers(kTriangleStripPrimitive, 0, buf);
// The callee reads eleven integer arguments; the original call also loads an ignored f1.
    ((void (*)(int, int, void*, float, int, int, int, int, int, int, int, int))Engine_CopyFramebufferToTexture)(0, 0, s_radialCaptureRenderState.texture, kFloat_0_Framebuffer, 0, 0, 0, 0, 0, 0, 0, 0);
    Engine_RestoreRenderState();
    // Bind the result and derive texture transforms from saved transform 0xb
    // and current transform 0. Keep concatenation order: inverse-transpose for
    // the normal-dependent path, scale 0.4, then translate texture coordinates
    // by (0.5,0.5). Slots 3/4 share g; slot 5 receives the normal-dependent h.
    Engine_SetTextureStageResource(2, s_radialCaptureRenderState.texture);
    // 0x804346B0..0x804346D8: copied from the stage defaults {0, 0, 1, 0}: two (state, value) pairs for stage 2
    // (Engine_SetTextureStageParameterList walks count pairs), i.e. state 0 = 0 and state 1 = 0.
    unsigned int v[4] = {0, 0, 1, 0};
    Engine_SetTextureStageParameterList(2, v, 2);
    unsigned char a[0x40], b[0x40], e[0x40], f[0x40];
    float c[16], d[16], g[16], h[16];
    Engine_GetTransform(0xb, a);
    Engine_GetTransform(0, b);
    PSMTX44Concat(c, a, b);
    Mtx44_InvertLegacyCofactors(d, 0, c);
    PSMTX44Transpose(d, d);
    d[3] = kFloat_0_Framebuffer;
    d[7] = kFloat_0_Framebuffer;
    d[11] = kFloat_0_Framebuffer;
    Mtx44_SetTranslation(e, kFloat_0_5_Framebuffer, kFloat_0_5_Framebuffer, kFloat_0_Framebuffer);
    Mtx44_SetScale(f, kFloat_0_4_Framebuffer, kFloat_0_4_Framebuffer, kFloat_0_4_Framebuffer);
    PSMTX44Concat(h, d, f);
    PSMTX44Concat(h, h, e);
    for (int k = 0; k < 16; ++k) g[k] = c[k];
    g[12] = kFloat_0_Framebuffer;
    g[13] = kFloat_0_Framebuffer;
    g[14] = kFloat_0_Framebuffer;
    PSMTX44Concat(g, g, f);
    SetTransform(3, g);
    SetTransform(4, g);
    SetTransform(5, h);
}

} // extern "C"
