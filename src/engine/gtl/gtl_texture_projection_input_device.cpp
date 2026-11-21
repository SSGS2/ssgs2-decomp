#include <decomp/offset_of.h>
extern "C" {
extern unsigned char kIndirectBumpXYZMatrix[];
}
#include "classes/RenderStateSnapshot.h"
#include "classes/GtlInputDevice.h"
#include "classes/GtlInputDeviceRecords.h"
#define AsGtlInputDevice(p) ((GtlInputDevice*)(p))
#include "classes/GtlRecordDeque.h"
#define AsGtlRecordDeque(p) ((GtlRecordDeque*)(p))

// Translation unit 0x80434998-0x804360D0 (24 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
// Projected texture/six-face rendering plus input-driver/record-deque adapters.
// Mod lookup: face accumulation -> SixFaceRenderer_RenderToTexture and
// s_sixFaceRenderResources/templates; tint -> TextureProjection_SetupTintCombiner;
// depth/UV projection -> Engine_UpdateTextureProjectionMatrices, then
// TextureProjection_ApplyTwoStageTransforms, paired with stage-state restore.
// Input descriptor sampling is separate: RefreshDriverState/ReadState/ReadItemDataByKind.
// Field names are provisional and kept consistent with the shared headers.

#include "sdk_protos.h"

// Layout views for the projection renderer's persistent buffers and templates.
struct SixFaceRenderResources {
    unsigned int reserved;
    float projection[16];
    float view[16];
    float faceTransforms[6][16];
    char pad204[0x48];
    char material[0x44];
    void* texture;
    void* lineVertices;
    void* lineIndices;
    void* stripVertices;
    void* stripIndices;
};
struct SixFaceRenderTemplates {
    char pad000[0x170];
    unsigned int viewport[6];
    unsigned int renderStates[18];
    unsigned int firstStageStates[16];
    unsigned int secondStageStates[16];
    unsigned int stageParameters[10];
    unsigned int material[26];
};
struct TextureProjectionState {
    void* texture;
    float textureMatrix[16];
    float depthMatrix[16];
    unsigned int firstStage;
    unsigned int secondStage;
};
struct ProjectionColorSettings {
    char pad000[0x80];
    float red, green, blue, alpha;
};
struct InputItemValues {
    void* button;
    void* axis;
    void* rangeMinimum;
    void* rangeMaximum;
};
// In/out buffer: +0..0xF is the caller's borrowed query, never overwritten;
// +0x10 receives button/axis/minimum, +0x14 receives maximum only for ranges.
// Values are provider words transported as void*, not newly allocated objects.
struct InputItemOutput {
    GtlInputItemQuery query;
    void* value;
    void* rangeMaximum;
};
typedef char SixFaceResources_facesOffset[((int)DECOMP_OFFSETOF(SixFaceRenderResources, faceTransforms) == 0x84) ? 1 : -1];
typedef char SixFaceResources_materialOffset[((int)DECOMP_OFFSETOF(SixFaceRenderResources, material) == 0x24C) ? 1 : -1];
typedef char SixFaceResources_textureOffset[((int)DECOMP_OFFSETOF(SixFaceRenderResources, texture) == 0x290) ? 1 : -1];
typedef char ProjectionState_stageOffset[((int)DECOMP_OFFSETOF(TextureProjectionState, firstStage) == 0x84) ? 1 : -1];
typedef char InputItemOutput_valueOffset[((int)DECOMP_OFFSETOF(InputItemOutput, value) == 0x10) ? 1 : -1];

enum {
    kRecordBytes = 0x18,
    kMaxRecordCount = 0x0AAAAAAA, // largest count whose 24-byte allocation fits
    kUnavailableInputChannel = 5,
    kInputQueryKey = 1,
    kInputQueryMaskedOrdinal = 2,
    kInputButton = 0,
    kInputRange = 1,
    kInputAxis = 2,
    kInputHandleCount = 256,
    kInputTypeMask = 0xFF,
    kInputOrdinalShift = 8,
    kInputOrdinalMask = 0xFFFF,
    kViewportWidthWord = 2, kViewportHeightWord = 3,
    kTransformView = 0,
    kTransformProjection = 1,
    kTransformFirstTexture = 2,
    kTransformWorld = 11,
    kTextureStateProjectionCount = 13,
    kTextureStateCoordinateSource = 10,
    kGXTriangleFan = 0xA0,
    kGXTriangleStrip = 0x98
};
static const unsigned int kInvalidInputHandle = 0xFFFFFFFF;

// Keep byte-stride expressions in the two state-save/restore functions: typed
// array indexing changes address calculation instructions in this compiler.
enum {
    kSnapshotTransforms = (int)DECOMP_OFFSETOF(RenderStateSnapshot, transforms),
    kSnapshotProjectionCount = (int)DECOMP_OFFSETOF(RenderStateSnapshot, textureStageStates[13]),
    kSnapshotCoordinateSource = (int)DECOMP_OFFSETOF(RenderStateSnapshot, textureStageStates[10]),
    kSnapshotSampler0 = (int)DECOMP_OFFSETOF(RenderStateSnapshot, samplerStates[0]),
    kSnapshotSampler1 = (int)DECOMP_OFFSETOF(RenderStateSnapshot, samplerStates[1]),
    kProjectionFirstStage = (int)DECOMP_OFFSETOF(TextureProjectionState, firstStage),
    kProjectionSecondStage = (int)DECOMP_OFFSETOF(TextureProjectionState, secondStage),
    kProjectionDepthMatrix = (int)DECOMP_OFFSETOF(TextureProjectionState, depthMatrix)
};

// GX combiner operands (color and alpha inputs use different selector spaces).
enum {
    kGXColorPrevious = 0, kGXColorTexture = 8, kGXColorRaster = 10,
    kGXColorOne = 12, kGXColorConstant = 14, kGXColorZero = 15,
    kGXAlphaRaster = 5, kGXAlphaZero = 7,
    kGXTevAdd = 0, kGXTevCompareGR16Greater = 10,
    kGXTevNoBias = 0, kGXTevScaleOne = 0, kGXTevPrevious = 0,
    kGXNoTexture = 255, kGXNoColorChannel = 255, kGXColor0Alpha0 = 4,
    kGXRed = 0, kGXGreen = 1, kGXBlue = 2, kGXAlpha = 3,
    kGXKonstColor0 = 12,
    kSnapshotMatrixBytes = 16 * sizeof(float),
    kSnapshotStageBytes = 25 * sizeof(void*),
    kSnapshotSamplerBytes = 9 * sizeof(void*),
    kDepthByteScale = 256,
    kFaceCount = 6,
    kFaceVertexStride = 0x18
};
static const unsigned int kProjectedPositionCoordinates = 0x80020000;

// Provider vtable slots are ordered to preserve the driver ABI.
struct VObj {
    virtual void* v00();
    virtual void* v01();
    virtual void* v02(void*, int); // retrieve resolved item payload
    virtual void* v03();
    virtual void* v04();
    virtual int v05(unsigned int, void*, int); // read state into caller buffer address
    virtual void* v06(void*); // query buffered sample count for a controller channel
    virtual void* v07();
    virtual void* v08();
    virtual int v09(unsigned char, void*);
    // request motor state for a controller channel
};

extern "C" {

void __dl__Q23gtl25IInputResourceMemOperatorFPv(void*);
void* InputDriver_GetByProviderId(void*);
void InputDriver_BuildDescriptorMapping(void*, void*, int);
void* InputDriver_EnumerateItems(void*, void*, void*);
void std_ThrowLengthError();
void* gtl_Graphics_AllocateWithCallback(unsigned int);
void gtl_Graphics_FreeWithCallback(void*);
void __dl__FPv(void*);
void* GtlRecordDeque_DestroyStorageVolatile(void*, short);
void GtlRecordDeque_CopyConstructElement(void*, unsigned int*, unsigned int*);
int gtl_IInputDevice_ResolveItemHandle(void*, void**, void*);
void SetTransform(unsigned int, void*);
void Engine_SetTextureStageState(unsigned int, unsigned int, void*);
void Engine_SetTextureStageParameter(unsigned int, unsigned int, void*);
void Engine_SetTextureStageResource(unsigned int, void*);
void Engine_GetTransform(unsigned int, void*);
void Engine_GetTextureStageState(unsigned int, unsigned int, void*);
void Engine_GetRenderStateRowField(unsigned int, unsigned int, void*);
void Engine_GetTextureStageResource(unsigned int, void*);
void PSMTX44Concat(void*, void*, void*);
void Mtx44_InvertLegacyCofactors(void*, unsigned int, void*);
void Mtx44_SetScale(void*, float, float, float);
void Mtx44_SetTranslation(void*, float, float, float);
void Mtx44_Concat3(void*, void*, void*, void*);
void GXSetTevKColorSel(unsigned int, unsigned int);
void GXSetTevKColor(unsigned int, void*);
void GXSetTevSwapMode(unsigned int, unsigned int, unsigned int);
extern void* __vt__Q23gtl12IInputDevice[];
extern void* s_defaultInputItemHandle;
extern const float kFloat_255_Framebuffer;
extern ProjectionColorSettings s_textureProjectionSettings;
extern unsigned char s_textureProjectionState[];
extern unsigned char s_renderStateSnapshot[];

extern unsigned int s_normalMatrixOverrideActive;
extern SixFaceRenderResources s_sixFaceRenderResources;
extern SixFaceRenderTemplates s_sixFaceRenderTemplates;
extern unsigned char kIdentityMatrix4x4[];

void Engine_SaveRenderState();
void Engine_ClearFramebuffer(int, int, int, int, int, float);
void Texture_GetLevelDescription(void*, int, void*);
void Engine_SetViewport(void*);
void Engine_SetRenderStateList(void*, int);
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetTextureStageParameterList(int, void*, int);
void Engine_SetLightEnabled(int, int);
void Engine_SetMaterial(void*);
void Engine_SetFlexibleVertexFormat(int);
void Engine_SetVertexDeclaration(int);
void PSMTX44Transpose(void*, void*);
void Engine_SetVertexStream(int, void*, int, int);
void* GX_GetPrimitiveCount(void*, int);
void DrawPrimitiveFromBoundBuffers(int, int, void*);
void Engine_CopyFramebufferToTexture(int, int, void*, int, int, int, int, int, int, int, int);
void Engine_RestoreRenderState();
void GXInitLightColor(void*, void*);
void GXSetChanMatColor(int, void*);
void GXSetChanAmbColor(int, void*);
void GXInitLightAttnA(void*, float, float, float);
void GXInitLightAttnK(void*, float, float, float);
void GXInitLightPos(void*, float, float, float);
void GXInitLightDir(void*, float, float, float);
void GXLoadLightObjImm(void*, int);
extern "C" void* memcpy(void*, const void*, unsigned long);
extern const float kFloat_1_EngineRender;
extern const float kFloat_0_Framebuffer;
extern const float kFloat_2_EngineRender;
extern const float kFloat_neg1_EngineRender;

struct Col {
    unsigned char r, g, b, a;
};

// Copy pairs of words without changing the original load/store grouping.
inline void CopyRenderWords(void* dst, const void* src, unsigned int pairs) {
    unsigned int* d = (unsigned int*)dst - 1;
    const unsigned int* s = (const unsigned int*)src - 1;
    int n = pairs;
    do {
        unsigned int a = s[1];
        unsigned int b = s[2];
        s += 2;
        d[1] = a;
        d[2] = b;
        d += 2;
    } while (--n != 0);
}

struct Blk18 { unsigned int w[18]; };
struct Blk26 { unsigned int w[26]; };

// Level-description fields used by this pass; the preceding 28 bytes are opaque.
struct ProjectionTextureDescription {
    unsigned char header[0x1C];
    unsigned int widthPixels, heightPixels;
};
typedef char ProjectionTextureDescription_size[(sizeof(ProjectionTextureDescription) == 0x24) ? 1 : -1];

// Stack frame of SixFaceRenderer_RenderToTexture, laid out exactly as the original function's frame.
// These arrays are WORKING COPIES of templates, not the saved caller state.
// Engine_SaveRenderState/RestoreRenderState own the external snapshot boundary.
struct SixFaceRenderFrame {
    void* destinationTexture;                // Render-target texture passed to the copy operation.
    void* ambientColorAddress;                  // 0x1c
    void* materialColorAddress;                  // 0x20
    void* lightColorAddress;                  // 0x24
    Col ambientColor;                // 0x28
    Col materialColor;                // 0x2c
    Col lightColor;                // 0x30
    Col colorScratch;                // 0x34
    unsigned int viewport[6];      // 0x38
    unsigned char light[0x40];   // 0x50
    unsigned char normalMatrix[0x40]; // 0x90
    unsigned int textureTransform[16];    // 0xd0
    unsigned char worldToView[0x40];// 0x110
    unsigned char inverseView[0x40];// 0x150
    unsigned int identityWorld[16];   // 0x190
    unsigned int stageParameters[10];   // 0x1d0
    unsigned int secondStageStates[16];   // 0x1f8
    unsigned int firstStageStates[16];   // 0x238
    ProjectionTextureDescription textureDescription;   // 0x278
    unsigned int materialTemplate[26];   // 0x29c
    unsigned int renderStates[18];   // 0x304
    char pad1[0x14];           // tail
};

#pragma push
#pragma global_optimizer off
#pragma use_frame_pointer on
#pragma opt_lifetimes off
// Render six oriented face textures into the destination texture while saving and restoring engine state.
// Snapshot boundary (Engine_Save/RestoreRenderState in gtl_render_state_and_primitive.cpp): 42 render
// states; 8 texture/resource stages, 25 states and 9 sampler parameters per stage;
// stream 0/index buffer, render target/surface, vertex formats/declaration;
// viewport, 44 transforms, 8 lights/enables, material and cached draw globals.
// It uses ONE global snapshot, so a nested call would overwrite the saved state.
// This is an engine-cache boundary; it does not independently snapshot raw GX.
void SixFaceRenderer_RenderToTexture(void* destinationTexture, void** faceTextures) {
    SixFaceRenderFrame f;
    SixFaceRenderTemplates* templates = &s_sixFaceRenderTemplates;

    // faceTextures supplies six borrowed textures in faceTransforms order; all
    // use the first face's dimensions. This pass accumulates six draws in ONE
    // framebuffer and copies it once, rather than producing six output layers.
    // Exact overlap/blend behavior comes from the external render-state templates.
    // Phase 1: snapshot engine state, clear once, install projection/view and
    // viewport (pixel dimensions from face 0), then copy state/value pair lists.
    // Lists hold 9 render, 8 stage-0, 8 stage-1 and 5 per-stage parameter pairs.
    f.destinationTexture = destinationTexture;
    Engine_SaveRenderState();
    Engine_ClearFramebuffer(0, 0, 3, 0, 0, kFloat_1_EngineRender);
    SetTransform(1, s_sixFaceRenderResources.projection);
    SetTransform(0, s_sixFaceRenderResources.view);
    Texture_GetLevelDescription(*(void**)faceTextures, 0, &f.textureDescription);
    CopyRenderWords(f.viewport, (unsigned int*)((char*)(templates->viewport)), 3);
    f.viewport[kViewportWidthWord] = f.textureDescription.widthPixels;
    f.viewport[kViewportHeightWord] = f.textureDescription.heightPixels;
    Engine_SetViewport(f.viewport);
    *(Blk18*)f.renderStates = *(Blk18*)((unsigned int*)((char*)(templates->renderStates)));
    Engine_SetRenderStateList(f.renderStates, 9);
    CopyRenderWords(f.firstStageStates, (unsigned int*)((char*)(templates->firstStageStates)), 8);
    CopyRenderWords(f.secondStageStates, (unsigned int*)((char*)(templates->secondStageStates)), 8);
    Engine_SetTextureStageStateList(0, f.firstStageStates, 8);
    Engine_SetTextureStageStateList(1, f.secondStageStates, 8);
    CopyRenderWords(f.stageParameters, (unsigned int*)((char*)(templates->stageParameters)), 5);
    for (unsigned int i = 0; i < 2; ++i)
        Engine_SetTextureStageParameterList(i, f.stageParameters, 5);
    // Phase 2: establish common world/view/material and auxiliary stage-1 texture.
    // identityWorld/worldToView/inverseView are temporary matrices; the copied
    // materialTemplate is retained for ABI fidelity but is not passed to SetMaterial.
    SetTransform(11, kIdentityMatrix4x4);
    Engine_SetTextureStageResource(1, s_sixFaceRenderResources.texture);
    *(Blk26*)f.materialTemplate = *(Blk26*)((unsigned int*)((char*)(templates->material)));
    Engine_SetLightEnabled(0, 1);
    Engine_SetMaterial(s_sixFaceRenderResources.material);
    CopyRenderWords(f.identityWorld, kIdentityMatrix4x4, 8);
    Mtx44_InvertLegacyCofactors(f.inverseView, 0, s_sixFaceRenderResources.view);
    PSMTX44Concat(f.worldToView, f.identityWorld, f.inverseView);
    Engine_SetFlexibleVertexFormat(0x401);
    Engine_SetVertexDeclaration(0);
    // Phase 3: accumulate every face before the single framebuffer copy.
    // Reorient projected coordinates and normals for each face. The API's concat
    // argument order is retained literally: inverse-transpose(face) is combined
    // with the identity/inverse-view matrix before installation in texture slots.
    for (int i = 0; i < kFaceCount; ++i) {
        Engine_SetTextureStageResource(0, faceTextures[i]);
        CopyRenderWords(f.textureTransform, s_sixFaceRenderResources.faceTransforms[i], 8);
        Mtx44_InvertLegacyCofactors(f.textureTransform, 0, f.textureTransform);
        PSMTX44Transpose(f.textureTransform, f.textureTransform);
        PSMTX44Concat(f.textureTransform, f.worldToView, f.textureTransform);
        SetTransform(2, f.textureTransform);
        SetTransform(3, f.textureTransform);
        PSMTX44Transpose(f.normalMatrix, s_sixFaceRenderResources.faceTransforms[i]);
        GXLoadNrmMtxImm((const float(*)[4])f.normalMatrix, 0);
        s_normalMatrixOverrideActive = 1;
        // White material/light, black ambient and a light along view -Z weight
        // each face through its transformed normals. Geometry is shared by faces.
        GXSetChanCtrl(4, 1, 0, 0, 1, 0, 0);
        f.colorScratch.a = 0xff;
        f.colorScratch.b = 0xff;
        f.colorScratch.g = 0xff;
        f.colorScratch.r = 0xff;
        f.lightColorAddress = &f.lightColor;
        f.lightColor.g = 0xff;
        f.lightColor.r = 0xff;
        f.lightColor.b = 0xff;
        f.lightColor.a = 0xff;
        GXInitLightColor(f.light, &f.lightColor);
        f.materialColorAddress = &f.materialColor;
        f.materialColor.r = f.colorScratch.r;
        f.materialColor.g = f.colorScratch.g;
        f.materialColor.b = f.colorScratch.b;
        f.materialColor.a = f.colorScratch.a;
        GXSetChanMatColor(4, &f.materialColor);
        f.colorScratch.a = 0;
        f.colorScratch.b = 0;
        f.colorScratch.g = 0;
        f.colorScratch.r = 0;
        f.ambientColorAddress = &f.ambientColor;
        f.ambientColor.g = 0;
        f.ambientColor.r = 0;
        f.ambientColor.b = 0;
        f.ambientColor.a = 0;
        GXSetChanAmbColor(4, &f.ambientColor);
        GXInitLightAttnA(f.light, kFloat_0_Framebuffer, kFloat_2_EngineRender, kFloat_0_Framebuffer);
        GXInitLightAttnK(f.light, kFloat_0_Framebuffer, kFloat_1_EngineRender, kFloat_0_Framebuffer);
        GXInitLightPos(f.light, kFloat_0_Framebuffer, kFloat_0_Framebuffer, kFloat_neg1_EngineRender);
        GXInitLightDir(f.light, kFloat_0_Framebuffer, kFloat_0_Framebuffer, kFloat_neg1_EngineRender);
        GXLoadLightObjImm(f.light, 1);
        // Draw the face's triangle fan and strip into the same accumulation target.
        // lineVertices/lineIndices are legacy field names; 0xA0 is GX_TRIANGLEFAN.
        Engine_SetVertexStream(0, s_sixFaceRenderResources.lineVertices, 0, kFaceVertexStride);
        DrawPrimitiveFromBoundBuffers(kGXTriangleFan, 0, GX_GetPrimitiveCount(s_sixFaceRenderResources.lineIndices, kGXTriangleFan));
        Engine_SetVertexStream(0, s_sixFaceRenderResources.stripVertices, 0, kFaceVertexStride);
        DrawPrimitiveFromBoundBuffers(kGXTriangleStrip, 0, GX_GetPrimitiveCount(s_sixFaceRenderResources.stripIndices, kGXTriangleStrip));
    }
    // Phase 4: copy after all six contributions and restore the engine snapshot.
    // normalMatrixOverrideActive is explicitly cleared, not restored to its entry
    // value; direct GX lighting/matrix calls are outside the field layout above.
    // This routine is neither a nested snapshot nor a scoped GX-state guard.
    s_normalMatrixOverrideActive = 0;
// The callee reads eleven integer arguments; the original call also loads an ignored f1.
    ((void (*)(int, float, int, void*, int, int, int, int, int, int, int, int))Engine_CopyFramebufferToTexture)(0, kFloat_0_Framebuffer, 0, f.destinationTexture, 0, 0, 0, 0, 0, 0, 0, 0);
    Engine_RestoreRenderState();
}
#pragma pop

// Configure three TEV stages to combine the two projected textures with the configured RGBA tint.
void TextureProjection_SetupTintCombiner() {
    // Let T1/T0 be the stage-1/stage-0 texture samples before swizzling.
    // Stage 0 writes T1'=(T1.r,T1.a,T1.b); stage 1 compares packed GR16
    // A=(T1.a<<8)|T1.r against B=(T0.g<<8)|T0.a, using T0'=(a,g,b).
    // With C=one and D=zero, compare-GR16-GT yields RGB mask (A>B ? 1 : 0).
    // Stage 2 computes rasterRGB*(1-mask) + konstRGB*mask: tint where A>B,
    // otherwise raster color (the usual TEV A*(1-C)+B*C+D interpolation).
    // Final alpha comes from raster alpha, not the uploaded konst alpha.
    GXSetNumTevStages(3);
    GXSetTevKColorSel(2, kGXKonstColor0);

    unsigned char color[8];
    ProjectionColorSettings* settings = &s_textureProjectionSettings;
    color[4] = color[0] = (unsigned char)(int)(kFloat_255_Framebuffer * settings->red);
    color[5] = color[1] = (unsigned char)(int)(kFloat_255_Framebuffer * settings->green);
    color[6] = color[2] = (unsigned char)(int)(kFloat_255_Framebuffer * settings->blue);
    color[7] = color[3] = (unsigned char)(int)(kFloat_255_Framebuffer * settings->alpha);
    GXSetTevKColor(0, color);
    GXSetTevSwapModeTable(1, kGXRed, kGXAlpha, kGXBlue, kGXAlpha);
    GXSetTevSwapModeTable(2, kGXAlpha, kGXGreen, kGXBlue, kGXAlpha);
    GXSetTevOrder(0, 1, 1, kGXNoColorChannel);
    GXSetTevColorIn(0, kGXColorZero, kGXColorZero, kGXColorZero, kGXColorTexture);
    GXSetTevColorOp(0, kGXTevAdd, kGXTevNoBias, kGXTevScaleOne, 1, kGXTevPrevious);
    GXSetTevSwapMode(0, 0, 1);
    GXSetTevOrder(1, 0, 0, kGXNoColorChannel);
    GXSetTevColorIn(1, kGXColorPrevious, kGXColorTexture, kGXColorOne, kGXColorZero);
    GXSetTevColorOp(1, kGXTevCompareGR16Greater, kGXTevNoBias, kGXTevScaleOne, 1, kGXTevPrevious);
    GXSetTevSwapMode(1, 0, 2);
    GXSetTevOrder(2, kGXNoTexture, kGXNoTexture, kGXColor0Alpha0);
    GXSetTevColorIn(2, kGXColorRaster, kGXColorConstant, kGXColorPrevious, kGXColorZero);
    GXSetTevColorOp(2, kGXTevAdd, kGXTevNoBias, kGXTevScaleOne, 1, kGXTevPrevious);
    GXSetTevSwapMode(2, 0, 0);
    GXSetTevAlphaIn(0, kGXAlphaZero, kGXAlphaZero, kGXAlphaZero, kGXAlphaZero);
    GXSetTevAlphaOp(0, kGXTevAdd, kGXTevNoBias, kGXTevScaleOne, 1, kGXTevPrevious);
    GXSetTevAlphaIn(1, kGXAlphaZero, kGXAlphaZero, kGXAlphaZero, kGXAlphaZero);
    GXSetTevAlphaOp(1, kGXTevAdd, kGXTevNoBias, kGXTevScaleOne, 1, kGXTevPrevious);
    GXSetTevAlphaIn(2, kGXAlphaZero, kGXAlphaZero, kGXAlphaZero, kGXAlphaRaster);
    GXSetTevAlphaOp(2, kGXTevAdd, kGXTevNoBias, kGXTevScaleOne, 1, kGXTevPrevious);
}

// Visit records in reverse, release deque storage, and optionally delete the deque itself.
void* dtor_804359C4(void* p, short volatile deleting) {
    volatile void* self = p;
    if (self) {
        volatile char* base = (char*)*(volatile void**)&AsGtlRecordDeque(self)->data + *(volatile int*)&AsGtlRecordDeque(self)->firstIndex * kRecordBytes;
        volatile char* cur = base + *(volatile int*)&AsGtlRecordDeque(self)->size * kRecordBytes;
        volatile void* unused27 = 0;
        volatile char* unused28 = 0;
        while (cur > base) {
            cur = cur - kRecordBytes;
            unused27 = *(volatile void**)&AsGtlRecordDeque(self)->allocator;
            unused28 = cur;
        }
        (void)unused27;
        (void)unused28;
        *(volatile int*)&AsGtlRecordDeque(self)->size = 0;
        GtlRecordDeque_DestroyStorageVolatile((void*)self, 0);
        if (deleting > 0)
            __dl__FPv((void*)self);
    }
    return (void*)self;
}

// Copy a half-open range backwards into free slots before the deque front.
void GtlRecordDeque_PrependRange(void* deque, char* volatile first, char* last) {
    char* destination = (char*)AsGtlRecordDeque(deque)->data + AsGtlRecordDeque(deque)->firstIndex * kRecordBytes;
    char* source = last;
    while (first < source) {
        source -= kRecordBytes;
        destination -= kRecordBytes;
        GtlRecordDeque_CopyConstructElement(AsGtlRecordDeque(deque)->allocator, (unsigned int*)destination, (unsigned int*)source);
        --AsGtlRecordDeque(deque)->firstIndex;
        ++*(int*)&AsGtlRecordDeque(deque)->size;
    }
}

// Clear the deque count and free its storage, optionally deleting the container.
void* GtlRecordDeque_DestroyStorage(void* deque, short deleting) {
    void* self = deque;
    short del = deleting;
    unsigned int oldSize = 0;
    unsigned char allocatorTag = 0;
    unsigned char copiedAllocatorTag = 0;
    if (deque) {
        if (AsGtlRecordDeque(deque)->data) {
            void* data = AsGtlRecordDeque(deque)->data;
            oldSize = AsGtlRecordDeque(deque)->size;
            allocatorTag = 0;
            AsGtlRecordDeque(deque)->size = oldSize - oldSize;
            copiedAllocatorTag = allocatorTag;
            gtl_Graphics_FreeWithCallback(data);
        }
        if (del > 0)
            __dl__FPv(self);
    }
    (void)copiedAllocatorTag;
    return self;
}

// Release deque storage using the original volatile cleanup sequence.
void* GtlRecordDeque_DestroyStorageVolatile(void* deque, short volatile deleting) {
    void* self = deque;
    if (self) {
        if (AsGtlRecordDeque(self)->data) {
            volatile unsigned int* size = (volatile unsigned int*)(((char*)&AsGtlRecordDeque(self)->size));
            volatile unsigned char allocatorTag = 0;
            *size = *size - *size;
            volatile unsigned char copiedAllocatorTag = allocatorTag;
            (void)copiedAllocatorTag;
            gtl_Graphics_FreeWithCallback(AsGtlRecordDeque(self)->data);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Refresh the controller channel's sample count and reapply its retained motor request.
int gtl_IInputDevice_RefreshDriverState(void* device) {
    VObj* driver = (VObj*)InputDriver_GetByProviderId(AsGtlInputDevice(device)->provider);
    *(void**)&AsGtlInputDevice(device)->activeChannel = AsGtlInputDevice(device)->state;
    AsGtlInputDevice(device)->deviceResult = driver->v06(AsGtlInputDevice(device)->state);
    unsigned char motorRequest = AsGtlInputDevice(device)->motorEnabled;
    driver = (VObj*)InputDriver_GetByProviderId(AsGtlInputDevice(device)->provider);
    if (driver->v09(motorRequest, *(void**)&AsGtlInputDevice(device)->activeChannel))
        AsGtlInputDevice(device)->motorEnabled = motorRequest;
    return 1;
}

// Delegate item enumeration when an input descriptor is installed.
void* gtl_IInputDevice_EnumerateItems(void* device, void* a, void* b) {
    if (*(int*)&AsGtlInputDevice(device)->descriptor == 0)
        return 0;
    return InputDriver_EnumerateItems(InputDriver_GetByProviderId(AsGtlInputDevice(device)->provider), a, b);
}

struct Elem18 {
    unsigned int w[6];
};

#pragma push
#pragma use_frame_pointer on
// Copy one 24-byte record; the allocator argument is unused for this record type.
void GtlRecordDeque_CopyConstructElement(void* deque, unsigned int* destination, unsigned int* source) {
    if (destination)
        *(Elem18*)destination = *(Elem18*)source;
}
#pragma pop

// Allocate storage for a checked number of 24-byte records and store the capacity.
void GtlRecordDeque_AllocateStorage(void* deque, void* unused, unsigned int count) {
    unsigned int max = kMaxRecordCount;
    unsigned int current = *(unsigned int*)&AsGtlRecordDeque(deque)->allocator;
    unsigned int* oldData = (unsigned int*)AsGtlRecordDeque(deque)->data;
    if (count > max)
        std_ThrowLengthError();
    void* data = gtl_Graphics_AllocateWithCallback(count * kRecordBytes);
    AsGtlRecordDeque(deque)->data = data;
    AsGtlRecordDeque(deque)->capacity = count;
    (void)current;
    (void)oldData;
}

// Release the input device only for a positive deleting-destructor flag.
void* gtl_IInputDevice_dtor(void* device, int flag) {
    if (device && flag > 0)
        __dl__Q23gtl25IInputResourceMemOperatorFPv(device);
    return device;
}

// Initialize an empty deque with the supplied allocator.
void GtlRecordDeque_Initialize(GtlRecordDeque* p, void* allocator) {
    p->data = 0;
    p->size = 0;
    p->capacity = 0;
    p->allocator = allocator;
    p->firstIndex = 0;
}

// Install a descriptor and ask the provider to rebuild its per-item handle mapping.
int gtl_IInputDevice_SetDescriptor(void* device, int value) {
    *(int*)&AsGtlInputDevice(device)->descriptor = value;
    void* driverResult = InputDriver_GetByProviderId(AsGtlInputDevice(device)->provider);
    InputDriver_BuildDescriptorMapping(driverResult, ((char*)&AsGtlInputDevice(device)->handles), *(int*)&AsGtlInputDevice(device)->descriptor);
    return 1;
}

// Return success for this no-op input-device operation; its caller-specific role is unconfirmed.
int InputDevice_NoOpSuccess_80435D34() {
    return 1;
}

// Return success for this no-op input-device operation; its caller-specific role is unconfirmed.
int InputDevice_NoOpSuccess_80435FB4() {
    return 1;
}

// Validate and clear the output buffer, then ask the provider to fill it unless unavailable.
int gtl_IInputDevice_ReadState(void* device, unsigned int stateBytes, unsigned int outputAddress) {
    if (!outputAddress)
        return 0;
    void* item = AsGtlInputDevice(device)->descriptor;
    if (!item)
        return 0;
    if (((GtlInputDescriptor*)item)->stateSizeBytes != stateBytes) // gtl_IInputDevice_ReadState: +0xC
        return 0;
    Memory_Clear((void*)outputAddress, stateBytes);
    if (AsGtlInputDevice(device)->activeChannel == kUnavailableInputChannel)
        return 0;
    void* driverResult = InputDriver_GetByProviderId(AsGtlInputDevice(device)->provider);
    return ((VObj*)driverResult)->v05(outputAddress, AsGtlInputDevice(device)->descriptor, AsGtlInputDevice(device)->activeChannel);
}

// Read the query in queryAndValue, resolve its provider handle, then write the requested payload.
// Requires an installed descriptor and room for InputItemOutput (24 bytes).
// Resolve failure returns 0 without changing payload; success returns 1 even
// for an unknown valueKind, which leaves both payload words unchanged.
int gtl_IInputDevice_ReadItemDataByKind(void* device, int valueKind, void* queryAndValue) {
    void* handle = 0;
    if (!gtl_IInputDevice_ResolveItemHandle(device, &handle, queryAndValue))
        return 0;
    void* driverResult = InputDriver_GetByProviderId(AsGtlInputDevice(device)->provider);
    void* data = ((VObj*)driverResult)->v02(handle, AsGtlInputDevice(device)->activeChannel);
    switch (valueKind) {
    case kInputButton:
        ((InputItemOutput*)queryAndValue)->value = ((InputItemValues*)data)->button;
        break;
    case kInputAxis:
        ((InputItemOutput*)queryAndValue)->value = ((InputItemValues*)data)->axis;
        break;
    case kInputRange:
        ((InputItemOutput*)queryAndValue)->value = ((InputItemValues*)data)->rangeMinimum;
        ((InputItemOutput*)queryAndValue)->rangeMaximum = ((InputItemValues*)data)->rangeMaximum;
        break;
    }
    return 1;
}

// Install the provider/channel pair and initialize every descriptor handle to the default sentinel.
void gtl_IInputDevice_Ctor(void* device, const unsigned int* providerAndChannel) {
    *(unsigned long long*)&AsGtlInputDevice(device)->provider = *(unsigned long long*)providerAndChannel;
    AsGtlInputDevice(device)->unk_0 = 1;
    AsGtlInputDevice(device)->vtbl = &__vt__Q23gtl12IInputDevice;
    AsGtlInputDevice(device)->activeChannel = kUnavailableInputChannel;
    *(int*)&AsGtlInputDevice(device)->descriptor = 0;
    *(int*)&AsGtlInputDevice(device)->deviceResult = 0;
    AsGtlInputDevice(device)->motorEnabled = 0;
    void** descriptorItem = (void**)AsGtlInputDevice(device)->handles;
    for (int i = 0; i < kInputHandleCount; ++i)
        descriptorItem[i] = s_defaultInputItemHandle;
}

// Exchange two pointer values.
void Pointer_Swap(void** a, void** b) {
    void* temp = *a;
    *a = *b;
    *b = temp;
}

// Resolve an exact key or the Nth descriptor matching a type mask to its provider handle.
// Query type 1: keyOrMaskIndex is the full 32-bit exact key.
// Query type 2: bits 7..0 are an intersecting type mask, bits 23..8 a ZERO-BASED
// ordinal among matches; bits 31..24 are ignored. Handles follow descriptor order.
// Requires descriptor != null and valid query/output addresses; failure leaves
// *handleOut untouched. The returned handle is borrowed from the provider map.
int gtl_IInputDevice_ResolveItemHandle(void* device, void** handleOut, void* itemQuery) {
    int queryKind = ((GtlInputItemQuery*)itemQuery)->type; // gtl_IInputDevice_ResolveItemHandle: +0xC
    void* descriptor = AsGtlInputDevice(device)->descriptor;
    unsigned int* descriptorItems = ((GtlInputDescriptor*)descriptor)->items; // +0x14
    unsigned int itemCount = ((GtlInputDescriptor*)descriptor)->itemCount; // +0x10
    unsigned int providerHandle = kInvalidInputHandle;
    if (queryKind == kInputQueryMaskedOrdinal) {
        unsigned int packedQuery = ((GtlInputItemQuery*)itemQuery)->keyOrMaskIndex; // +8
        unsigned int mask = packedQuery & kInputTypeMask;
        unsigned int matchOrdinal = (packedQuery >> kInputOrdinalShift) & kInputOrdinalMask;
        GtlInputDescriptorItem* descriptorItem = (GtlInputDescriptorItem*)descriptorItems;
        unsigned int matches = 0;
        for (unsigned int i = 0; i < itemCount; ++i) {
            if (mask & (descriptorItem->typeMask & kInputTypeMask)) { // +8
                if (matchOrdinal == matches) {
                    providerHandle = AsGtlInputDevice(device)->handles[i];
                    break;
                }
                ++matches;
            }
            ++descriptorItem;
        }
    } else if (queryKind == kInputQueryKey) {
        unsigned int itemKey = ((GtlInputItemQuery*)itemQuery)->keyOrMaskIndex; // +8
        GtlInputDescriptorItem* descriptorItem = (GtlInputDescriptorItem*)descriptorItems;
        for (unsigned int i = 0; i < itemCount; ++i) {
            if (itemKey == descriptorItem->key) { // +4
                providerHandle = AsGtlInputDevice(device)->handles[i];
                break;
            }
            ++descriptorItem;
        }
    }
    if (providerHandle == kInvalidInputHandle)
        return 0;
    *handleOut = (void*)providerHandle;
    return 1;
}

// Grow capacity by the larger of the request and a size-dependent increment, with overflow checks.
unsigned int GtlRecordDeque_CalculateGrowthCapacity(void* deque, unsigned int additionalCount) {
    volatile unsigned int max = kMaxRecordCount;
    unsigned int capacity = AsGtlRecordDeque(deque)->capacity;
    if (additionalCount > max - capacity)
        std_ThrowLengthError();
    volatile unsigned int three = 3;
    unsigned int third = max / three;
    if (capacity < third) {
        volatile unsigned int five = 5;
        unsigned int growth = ((capacity + 1) * three) / five;
        unsigned int* chosen = growth < additionalCount ? &additionalCount : &growth;
        return capacity + *chosen;
    } else if (capacity < third * 2) {
        unsigned int growth = (capacity + 1) / 2;
        unsigned int* chosen = growth < additionalCount ? &additionalCount : &growth;
        return capacity + *chosen;
    }
    return max;
}

// Restore transforms, stage states, sampler parameters and texture for the two selected stages.
void Engine_ApplySelectedTextureStageState() {
    unsigned int firstStage = *(unsigned int*)(s_textureProjectionState + kProjectionFirstStage);
    unsigned int secondStage = *(unsigned int*)(s_textureProjectionState + kProjectionSecondStage);
    SetTransform(firstStage + kTransformFirstTexture, s_renderStateSnapshot + (firstStage + kTransformFirstTexture) * kSnapshotMatrixBytes + kSnapshotTransforms);
    SetTransform(secondStage + kTransformFirstTexture, s_renderStateSnapshot + (secondStage + kTransformFirstTexture) * kSnapshotMatrixBytes + kSnapshotTransforms);
    Engine_SetTextureStageState(firstStage, kTextureStateProjectionCount, *(void**)(s_renderStateSnapshot + firstStage * kSnapshotStageBytes + kSnapshotProjectionCount));
    Engine_SetTextureStageState(secondStage, kTextureStateProjectionCount, *(void**)(s_renderStateSnapshot + secondStage * kSnapshotStageBytes + kSnapshotProjectionCount));
    Engine_SetTextureStageState(firstStage, kTextureStateCoordinateSource, *(void**)(s_renderStateSnapshot + firstStage * kSnapshotStageBytes + kSnapshotCoordinateSource));
    Engine_SetTextureStageState(secondStage, kTextureStateCoordinateSource, *(void**)(s_renderStateSnapshot + secondStage * kSnapshotStageBytes + kSnapshotCoordinateSource));
    Engine_SetTextureStageParameter(secondStage, 0, *(void**)(s_renderStateSnapshot + secondStage * kSnapshotSamplerBytes + kSnapshotSampler0));
    Engine_SetTextureStageParameter(secondStage, 1, *(void**)(s_renderStateSnapshot + secondStage * kSnapshotSamplerBytes + kSnapshotSampler1));
    Engine_SetTextureStageResource(secondStage, ((void**)s_renderStateSnapshot)[secondStage]);
}

// Build projected UV and encoded-depth matrices from the current view and projection.
void Engine_UpdateTextureProjectionMatrices() {
    float view[16];
    float projection[16];
    float textureProjection[16];
    float clipScale[16];
    float clipTranslation[16];
    // These engine matrices use flat slots 10/14 for projection Z coefficients;
    // retain the engine's concat convention, not the SDK's usual row notation.
    // UV mapping sends clip x/w [-1,1] to [0,1], and flips y for texture origin.
    // Replacing Z leaves the X/Y projection intact for projective UV generation.
    Engine_GetTransform(kTransformView, view);
    Engine_GetTransform(kTransformProjection, projection);
    for (int i = 0; i < 16; ++i)
        textureProjection[i] = projection[i];
    textureProjection[10]=-1.0f;
    textureProjection[14]=0.0f;
    Mtx44_SetScale(clipScale, 0.5f, -0.5f, 1.0f);
    Mtx44_SetTranslation(clipTranslation, 0.5f, 0.5f, 0.0f);
    PSMTX44Concat(clipScale, clipScale, clipTranslation);
    Mtx44_Concat3(((TextureProjectionState*)s_textureProjectionState)->textureMatrix, view, textureProjection, clipScale);
    // Write A=projection[10], B=projection[14], r=depthScale, d=depthOffset.
    // r=(A-1)/(A+1) for A<0 (reciprocal otherwise), d=B*(1-r)/r.
    // Then s=d*r, span=s-d; coefficients are a=s/span, b=s*d/span.
    // The installed coordinates are proportional to (a*z+b, 256*(a*z+b), z).
    // Projective divide therefore gives (a+b/z, 256*(a+b/z)): two byte scales
    // of the same depth for the GR16 compare. Slot 10 supplies the divisor z.
    // Producer contract: MtxPerspectiveFovRH/LH in gtl_vector_matrix_math.cpp writes A/B at
    // 10/14 and W sign at 11 (-1 for RH/-Z-forward, +1 for LH/+Z-forward).
    // For positive ordered clip distances, A<0 selects RH, otherwise LH here;
    // this adapter reads A, not slot11. Orthographic/arbitrary matrices are
    // outside that contract. Global Projection_SetDepthConvention also changes
    // A in the producer; keep it consistent with encoded-depth consumers.
    // z is view-space position after view transform; A/r are dimensionless,
    // B/d/s/span use the caller's view-distance unit. No conversion to meters,
    // yards or pixels occurs. Scaling all positions and clip distances together
    // preserves the ratios; changing only B changes depth lookup. GR16 byte
    // encoding is separate from world distance and from UV's Y-origin flip.
    // Requires A+1, A-1, r, span nonzero and finite (plus z!=0 at sampling).
    // No guards/clamps/status: unsupported matrices publish nonfinite values.
    float projectionZ=projection[10], depth=projection[14];
    float depthScale,depthOffset;
    if (projectionZ < 0.0f) {
        depthScale=(projectionZ-1.0f)/(1.0f+projectionZ);
        depthOffset=(depth-depth*((projectionZ-1.0f)/(1.0f+projectionZ)))/depthScale;
    }
    else {
        depthScale=(1.0f+projectionZ)/(projectionZ-1.0f);
        depthOffset=(depth-depth*((1.0f+projectionZ)/(projectionZ-1.0f)))/depthScale;
    }
    float scaledDepth=depthOffset*depthScale, depthSpan=scaledDepth-depthOffset;
    float* depthMatrix=(float*)(((TextureProjectionState*)s_textureProjectionState)->depthMatrix);
    Mtx44_SetScale(depthMatrix, 0.0f, 0.0f, 0.0f);
    depthMatrix[8]=scaledDepth/depthSpan;
    depthMatrix[9]=(float)kDepthByteScale*depthMatrix[8];
    depthMatrix[12]=scaledDepth*depthOffset/depthSpan;
    depthMatrix[13]=(float)kDepthByteScale*depthMatrix[12];
    depthMatrix[10]=1.0f;
    PSMTX44Concat(depthMatrix, view, depthMatrix);
}

// Save two texture stages and configure their transforms and resources for projected texture rendering.
void TextureProjection_ApplyTwoStageTransforms() {
    // Save the selected stages first; Engine_ApplySelectedTextureStageState is
    // the matching restore. Generated coordinates use position, three components.
    *(unsigned int*)(s_textureProjectionState+kProjectionFirstStage)=0;
    *(unsigned int*)(s_textureProjectionState+kProjectionSecondStage)=1;
    unsigned int firstStage=*(volatile unsigned int*)(s_textureProjectionState+kProjectionFirstStage), secondStage=*(volatile unsigned int*)(s_textureProjectionState+kProjectionSecondStage);
    unsigned int firstTransform=firstStage+kTransformFirstTexture, secondTransform=secondStage+kTransformFirstTexture;
    Engine_GetTransform(firstTransform, s_renderStateSnapshot+firstTransform*kSnapshotMatrixBytes+kSnapshotTransforms);
    Engine_GetTransform(secondTransform, s_renderStateSnapshot+secondTransform*kSnapshotMatrixBytes+kSnapshotTransforms);
    Engine_GetTextureStageState(firstStage,kTextureStateProjectionCount,s_renderStateSnapshot+firstStage*kSnapshotStageBytes+kSnapshotProjectionCount);
    Engine_GetTextureStageState(secondStage,kTextureStateProjectionCount,s_renderStateSnapshot+secondStage*kSnapshotStageBytes+kSnapshotProjectionCount);
    Engine_GetTextureStageState(firstStage,kTextureStateCoordinateSource,s_renderStateSnapshot+firstStage*kSnapshotStageBytes+kSnapshotCoordinateSource);
    Engine_GetTextureStageState(secondStage,kTextureStateCoordinateSource,s_renderStateSnapshot+secondStage*kSnapshotStageBytes+kSnapshotCoordinateSource);
    Engine_GetRenderStateRowField(secondStage,0,s_renderStateSnapshot+secondStage*kSnapshotSamplerBytes+kSnapshotSampler0);
    Engine_GetRenderStateRowField(secondStage,1,s_renderStateSnapshot+secondStage*kSnapshotSamplerBytes+kSnapshotSampler1);
    Engine_GetTextureStageResource(secondStage,s_renderStateSnapshot+secondStage*4);
    // Convert the previously built view/projection matrices for the current world
    // transform, then cancel current world/view as required by position texgen.
    unsigned char world[64], textureProjection[64], depthProjection[64], inverseWorld[64], inverseView[64], view[64], viewToWorld[64];
    Engine_GetTransform(kTransformWorld,world);
    PSMTX44Concat(textureProjection,world,s_textureProjectionState+4);
    PSMTX44Concat(depthProjection,world,s_textureProjectionState+kProjectionDepthMatrix);
    Engine_GetTransform(kTransformView,view);
    Mtx44_InvertLegacyCofactors(inverseWorld,0,world);
    Mtx44_InvertLegacyCofactors(inverseView,0,view);
    PSMTX44Concat(viewToWorld,inverseView,inverseWorld);
    PSMTX44Concat(textureProjection,viewToWorld,textureProjection);
    PSMTX44Concat(depthProjection,viewToWorld,depthProjection);
    SetTransform(kTransformFirstTexture,textureProjection);
    SetTransform(kTransformFirstTexture + 1,depthProjection);
    Engine_SetTextureStageState(firstStage,kTextureStateProjectionCount,(void*)3);
    Engine_SetTextureStageState(secondStage,kTextureStateProjectionCount,(void*)3);
    Engine_SetTextureStageState(firstStage,kTextureStateCoordinateSource,(void*)kProjectedPositionCoordinates);
    Engine_SetTextureStageState(secondStage,kTextureStateCoordinateSource,(void*)kProjectedPositionCoordinates);
    Engine_SetTextureStageParameter(secondStage,0,(void*)0);
    Engine_SetTextureStageParameter(secondStage,1,(void*)1);
    Engine_SetTextureStageResource(secondStage,*(void**)s_textureProjectionState);
}

} // extern "C"
