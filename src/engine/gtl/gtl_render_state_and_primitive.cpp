#include "classes/RenderStateSnapshot.h"
#define AsRenderStateSnapshot(p) ((RenderStateSnapshot*)(p))
#include "classes/GtlBuffer.h"
#define AsGtlBuffer(p) ((GtlBuffer*)(p))
#include "classes/GtlPrimitive.h"
#define AsGtlPrimitive(p) ((GtlPrimitive*)(p))
#include "classes/GtlVideoConfigView.h"
#define AsVideoConfig804326B0(p) ((GtlVideoConfigView*)(p))
// Translation unit 0x804323D8-0x80433528 (25 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#pragma use_lmw_stmw on

extern "C" {

enum {
    kTextureStageCount = 8, kRenderStateCount = 42, kTransformCount = 44,
    kTextureStateCount = 25, kSamplerStateCount = 9, kFormatPatternCount = 39,
    kSavedSamplerQueryCount = 25, kSnapshotClearBytes = 0x1488,
    kBasisVertexFloatCount = 14
    , kGxWriteGatherPipe = 0xCC008000
};


// The state APIs pass scalar bits in pointer-sized arguments on the Wii ABI.
// Distinguish those bits from retained resources without changing byte staging.
union SnapshotStateWord { unsigned int scalar; void* resource; };

// Offsets come from the shared snapshot layout; byte staging preserves API order.
enum SnapshotArrayLayout {
    kSnapshotRenderStatesOffset = (int)&((RenderStateSnapshot*)0)->renderStates,
    kSnapshotStageResourcesOffset = (int)&((RenderStateSnapshot*)0)->paletteResources,
    kSnapshotTextureStageStatesOffset = (int)&((RenderStateSnapshot*)0)->textureStageStates,
    kSnapshotSamplerStatesOffset = (int)&((RenderStateSnapshot*)0)->samplerStates,
    kSnapshotTransformsOffset = (int)&((RenderStateSnapshot*)0)->transforms,
    kSnapshotLightsOffset = (int)&((RenderStateSnapshot*)0)->lights,
    kSnapshotLightEnabledOffset = (int)&((RenderStateSnapshot*)0)->lightEnabled,
    kSnapshotTextureRowBytes = 25 * 4, kSnapshotSamplerRowBytes = 9 * 4,
    kSnapshotTransformBytes = 0x40, kSnapshotLightBytes = 0x68
};

#include "sdk_protos.h"
extern void* memset(void*, int, unsigned int);
void RenderState_CopyEightWords(unsigned int*, unsigned int*);
void Engine_GetRenderState(unsigned int, void*);
void GraphicsResource_Release(void*);
void Engine_GetTextureStageResource(unsigned int, void*);
void Engine_AcquireTexturePalette(unsigned int, void*);
void Engine_GetTextureStageState(unsigned int, unsigned int, void*);
void Engine_GetRenderStateRowField(unsigned int, unsigned int, void*);
void Engine_GetVertexStream(unsigned int, void*, void*, void*);
void Engine_GetIndexBuffer(void*, void*);
void Render_GetVertexFormat(void*);
void Engine_AcquireCurrentRenderTarget(unsigned int, void*);
void Engine_AcquireCurrentRenderSurface(void*);
void Render_GetVertexDeclaration(void*);
void Engine_AcquireType6Resource(void*);
void Engine_AcquireType7Resource(void*);
void Engine_GetSystemOption(unsigned int, void*);
void Engine_GetViewport(void*);
void Engine_GetTransform(unsigned int, void*);
void Engine_GetLight(unsigned int, void*);
void Engine_GetLightEnabled(unsigned int, void*);
void Engine_GetMaterial(void*);
extern unsigned char s_renderStateSnapshotInitialized;
extern unsigned int s_renderStateSnapshot[];
extern unsigned int g_useIndexedVertexInput, g_drawPrimitiveType, g_drawUsesBoundVertexBuffer, g_drawUsesIndexDisplayList, g_boundBufferDrawArgument;
extern unsigned int s_vertexFormatDescriptor[8];

#pragma dont_inline on
#pragma optimization_level 1
// Save graphics resources, state arrays, transforms and draw globals; release prior snapshot references first.
void Engine_SaveRenderState(void) {
    if (s_renderStateSnapshotInitialized == 0) {
        Memory_Clear(s_renderStateSnapshot, kSnapshotClearBytes);
        s_renderStateSnapshotInitialized = 1;
    }
    for (unsigned int i = 0; i < kRenderStateCount; ++i) {
        Engine_GetRenderState(i, (char*)s_renderStateSnapshot + i * 4 + kSnapshotRenderStatesOffset);
    }

    for (unsigned int i = 0; i < kTextureStageCount; ++i) {
        if (s_renderStateSnapshot[i]) {
            GraphicsResource_Release(*(void**)((char*)s_renderStateSnapshot + i * 4));
            s_renderStateSnapshot[i] = 0;
        }
        if (*(unsigned int*)((char*)s_renderStateSnapshot + i * 4 + kSnapshotStageResourcesOffset)) {
            GraphicsResource_Release(*(void**)((char*)s_renderStateSnapshot + i * 4 + kSnapshotStageResourcesOffset));
            *(unsigned int*)((char*)s_renderStateSnapshot + i * 4 + kSnapshotStageResourcesOffset) = 0;
        }
        Engine_GetTextureStageResource(i, (char*)s_renderStateSnapshot + i * 4);
        Engine_AcquireTexturePalette(i, (char*)s_renderStateSnapshot + i * 4 + kSnapshotStageResourcesOffset);
        for (unsigned int j = 0; j < kTextureStateCount; ++j) {
            Engine_GetTextureStageState(i, j, (char*)s_renderStateSnapshot + i * kSnapshotTextureRowBytes + j * 4 + kSnapshotTextureStageStatesOffset);
        }
        // The original save queries 25 fields at a NINE-word row stride.
        // Queries 9..24 overlap later sampler rows; the last stage also reaches
        // systemOptions, viewport and the start of transforms. Those regions are
        // populated again below. Restore uses only the nine fields of each row.
        // The reason for the extra queries is unresolved; preserve their calls.
        for (unsigned int j = 0; j < kSavedSamplerQueryCount; ++j) {
            Engine_GetRenderStateRowField(i, j, (char*)s_renderStateSnapshot + i * kSnapshotSamplerRowBytes + j * 4 + kSnapshotSamplerStatesOffset);
        }
    }
    if (AsRenderStateSnapshot(s_renderStateSnapshot)->vertexStream) {
        GraphicsResource_Release((*(void**)&AsRenderStateSnapshot(s_renderStateSnapshot)->vertexStream));
        AsRenderStateSnapshot(s_renderStateSnapshot)->vertexStream = 0;
    }
    unsigned int temp;
    Engine_GetVertexStream(0, (char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->vertexStream, &temp, (char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->vertexStride);
    Engine_GetIndexBuffer((char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->indexBuffer, (char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->indexOffset);
    Render_GetVertexFormat((char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->vertexFormat);
    if ((*(unsigned int*)&AsRenderStateSnapshot(s_renderStateSnapshot)->renderTarget)) { GraphicsResource_Release(AsRenderStateSnapshot(s_renderStateSnapshot)->renderTarget); (*(unsigned int*)&AsRenderStateSnapshot(s_renderStateSnapshot)->renderTarget) = 0; }
    if ((*(unsigned int*)&AsRenderStateSnapshot(s_renderStateSnapshot)->depthStencilSurface)) { GraphicsResource_Release(AsRenderStateSnapshot(s_renderStateSnapshot)->depthStencilSurface); (*(unsigned int*)&AsRenderStateSnapshot(s_renderStateSnapshot)->depthStencilSurface) = 0; }
    Engine_AcquireCurrentRenderTarget(0, (char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->renderTarget);
    Engine_AcquireCurrentRenderSurface((char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->depthStencilSurface);
    if ((*(unsigned int*)&AsRenderStateSnapshot(s_renderStateSnapshot)->vertexDeclaration)) { GraphicsResource_Release(AsRenderStateSnapshot(s_renderStateSnapshot)->vertexDeclaration); (*(unsigned int*)&AsRenderStateSnapshot(s_renderStateSnapshot)->vertexDeclaration) = 0; }
    if ((*(unsigned int*)&AsRenderStateSnapshot(s_renderStateSnapshot)->type6Resource)) { GraphicsResource_Release(AsRenderStateSnapshot(s_renderStateSnapshot)->type6Resource); (*(unsigned int*)&AsRenderStateSnapshot(s_renderStateSnapshot)->type6Resource) = 0; }
    if ((*(unsigned int*)&AsRenderStateSnapshot(s_renderStateSnapshot)->type7Resource)) { GraphicsResource_Release(AsRenderStateSnapshot(s_renderStateSnapshot)->type7Resource); (*(unsigned int*)&AsRenderStateSnapshot(s_renderStateSnapshot)->type7Resource) = 0; }
    Render_GetVertexDeclaration((char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->vertexDeclaration);
    Engine_AcquireType6Resource((char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->type6Resource);
    Engine_AcquireType7Resource((char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->type7Resource);
    for (int i = 0; i < 9; ++i)
        Engine_GetSystemOption(i, (char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->systemOptions + i * 4);
    Engine_GetViewport((char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->viewport);
    for (int i = 0; i < kTransformCount; ++i)
        Engine_GetTransform(i, (char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->transforms + i * kSnapshotTransformBytes);
    for (unsigned int i = 0; i < kTextureStageCount; ++i) {
        Engine_GetLight(i, (char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->lights + i * kSnapshotLightBytes);
        Engine_GetLightEnabled(i, (char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->lightEnabled + i * 4);
    }
    Engine_GetMaterial((char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->material);
    AsRenderStateSnapshot(s_renderStateSnapshot)->useIndexedVertexInput = g_useIndexedVertexInput;
    AsRenderStateSnapshot(s_renderStateSnapshot)->drawPrimitiveType = g_drawPrimitiveType;
    AsRenderStateSnapshot(s_renderStateSnapshot)->drawUsesBoundVertexBuffer = g_drawUsesBoundVertexBuffer;
    AsRenderStateSnapshot(s_renderStateSnapshot)->drawUsesIndexDisplayList = g_drawUsesIndexDisplayList;
    AsRenderStateSnapshot(s_renderStateSnapshot)->boundBufferDrawArgument = g_boundBufferDrawArgument;
    RenderState_CopyEightWords((unsigned int*)((char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->formatDescriptor), s_vertexFormatDescriptor);
}
#pragma global_optimizer reset
#pragma dont_inline reset

extern unsigned int SCGetProgressiveMode(void);
extern void Engine_SetCachedRenderState(unsigned int, void*);
extern void Engine_SetTextureStageResource(unsigned int, void*);
extern void Engine_SetTexturePalette(unsigned int, void*);
extern void Engine_SetTextureStageState(unsigned int, unsigned int, void*);
extern void Engine_SetTextureStageParameter(unsigned int, unsigned int, void*);
extern void Engine_SetVertexStream(unsigned int, unsigned int, unsigned int, unsigned int);
extern void Engine_SetFlexibleVertexFormat(unsigned int);
extern void Engine_SetRenderTarget(unsigned int, void*);
extern void Engine_SetType6Resource(void*);
extern void Engine_SetType7Resource(void*);
extern void Engine_SetCurrentRenderSurface(void*);
extern void Engine_SetVertexDeclaration(void*);
extern void Engine_SetIndexBuffer(void*, int);
extern void Engine_SetViewport(void*);
extern void SetTransform(unsigned int, void*);
extern void Engine_SetLight(unsigned int, void*);
extern void Engine_SetLightEnabled(unsigned int, void*);
extern void Engine_SetMaterial(void*);
extern unsigned int s_renderStateSnapshot[0x14A0/4];
extern unsigned int g_useIndexedVertexInput, g_drawPrimitiveType, g_drawUsesBoundVertexBuffer, g_drawUsesIndexDisplayList, g_boundBufferDrawArgument;
extern unsigned int s_vertexFormatDescriptor[8];

extern const float kFloat_60_VideoConfig, kFloat_50_VideoConfig; // PAL refresh rate: 50 Hz.
enum VideoTvFormat { kTvNtsc = 0, kTvPal = 1, kTvMpal = 2, kTvEuRgb60 = 5 };
enum VideoOutputMode { kOutputNtsc = 2, kOutputPal = 3, kOutputEuRgb60 = 4, kOutputMpal = 5 };
enum VideoDimensions { kVideoWidth = 640, kVideoHeight60Hz = 480, kVideoHeightPal = 576 };
#pragma optimization_level 0
// Choose framebuffer dimensions and refresh rate from the console TV standard and progressive setting.
void VideoConfig_InitFromTvFormat(void* p) {
    memset(p, 0, 0x30);
    AsVideoConfig804326B0(p)->interlaced = ((unsigned char)SCGetProgressiveMode()) == 0;
    AsVideoConfig804326B0(p)->unk_10 = 0;
    AsVideoConfig804326B0(p)->unk_C = 0;
    switch (VIGetTvFormat()) {
    case kTvNtsc:
        AsVideoConfig804326B0(p)->tvMode = kOutputNtsc;
        AsVideoConfig804326B0(p)->width = kVideoWidth;
        AsVideoConfig804326B0(p)->height = kVideoHeight60Hz;
        AsVideoConfig804326B0(p)->refreshRateHz = kFloat_60_VideoConfig;
        break;
    case kTvPal:
        AsVideoConfig804326B0(p)->tvMode = kOutputPal;
        AsVideoConfig804326B0(p)->width = kVideoWidth;
        AsVideoConfig804326B0(p)->height = kVideoHeightPal;
        AsVideoConfig804326B0(p)->refreshRateHz = kFloat_50_VideoConfig;
        break;
    case kTvEuRgb60:
        AsVideoConfig804326B0(p)->tvMode = kOutputEuRgb60;
        AsVideoConfig804326B0(p)->width = kVideoWidth;
        AsVideoConfig804326B0(p)->height = kVideoHeight60Hz;
        AsVideoConfig804326B0(p)->refreshRateHz = kFloat_60_VideoConfig;
        break;
    case kTvMpal:
        AsVideoConfig804326B0(p)->tvMode = kOutputMpal;
        AsVideoConfig804326B0(p)->width = kVideoWidth;
        AsVideoConfig804326B0(p)->height = kVideoHeight60Hz;
        AsVideoConfig804326B0(p)->refreshRateHz = kFloat_60_VideoConfig;
        break;
    }
}
#pragma optimization_level reset

// Restore the graphics state and draw globals captured by Engine_SaveRenderState.
void Engine_RestoreRenderState(void) {
    // Phase 1: restore scalar state values and texture references by stage.
    // These APIs encode scalar values in void*; they do not dereference them.
    for (unsigned int i = 0; i < kRenderStateCount; ++i)
        Engine_SetCachedRenderState(i, (void*)((SnapshotStateWord*)((char*)s_renderStateSnapshot + i * 4 + kSnapshotRenderStatesOffset))->scalar);
    for (unsigned int i = 0; i < kTextureStageCount; ++i) {
        Engine_SetTextureStageResource(i, ((SnapshotStateWord*)((char*)s_renderStateSnapshot + i * 4))->resource);
        Engine_SetTexturePalette(i, ((SnapshotStateWord*)((char*)s_renderStateSnapshot + i * 4 + kSnapshotStageResourcesOffset))->resource);
        for (unsigned int j = 0; j < kTextureStateCount; ++j)
            Engine_SetTextureStageState(i, j, (void*)((SnapshotStateWord*)((char*)s_renderStateSnapshot + i * kSnapshotTextureRowBytes + j * 4 + kSnapshotTextureStageStatesOffset))->scalar);
        for (unsigned int j = 0; j < kSamplerStateCount; ++j)
            Engine_SetTextureStageParameter(i, j, (void*)((SnapshotStateWord*)((char*)s_renderStateSnapshot + i * kSnapshotSamplerRowBytes + j * 4 + kSnapshotSamplerStatesOffset))->scalar);
    }
    // Phase 2: rebind streams, targets, declarations and other retained resources.
    Engine_SetVertexStream(0, AsRenderStateSnapshot(s_renderStateSnapshot)->vertexStream, 0, AsRenderStateSnapshot(s_renderStateSnapshot)->vertexStride);
    Engine_SetIndexBuffer(AsRenderStateSnapshot(s_renderStateSnapshot)->indexBuffer, (int)AsRenderStateSnapshot(s_renderStateSnapshot)->indexOffset);
    Engine_SetFlexibleVertexFormat(0);
    Engine_SetFlexibleVertexFormat(AsRenderStateSnapshot(s_renderStateSnapshot)->vertexFormat);
    Engine_SetRenderTarget(0, AsRenderStateSnapshot(s_renderStateSnapshot)->renderTarget);
    Engine_SetCurrentRenderSurface(AsRenderStateSnapshot(s_renderStateSnapshot)->depthStencilSurface);
    Engine_SetVertexDeclaration(AsRenderStateSnapshot(s_renderStateSnapshot)->vertexDeclaration);
    Engine_SetType6Resource(AsRenderStateSnapshot(s_renderStateSnapshot)->type6Resource);
    Engine_SetType7Resource(AsRenderStateSnapshot(s_renderStateSnapshot)->type7Resource);
    // Phase 3: restore camera/light/material data, then the draw-dispatch globals.
    Engine_SetViewport((char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->viewport);
    for (int i = 0; i < kTransformCount; ++i)
        SetTransform(i, (char*)s_renderStateSnapshot + i * kSnapshotTransformBytes + kSnapshotTransformsOffset);
    for (unsigned int i = 0; i < kTextureStageCount; ++i) {
        Engine_SetLight(i, (char*)s_renderStateSnapshot + i * kSnapshotLightBytes + kSnapshotLightsOffset);
        Engine_SetLightEnabled(i, (void*)((SnapshotStateWord*)((char*)s_renderStateSnapshot + i * 4 + kSnapshotLightEnabledOffset))->scalar);
    }
    Engine_SetMaterial((char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->material);
    g_useIndexedVertexInput = AsRenderStateSnapshot(s_renderStateSnapshot)->useIndexedVertexInput;
    g_drawPrimitiveType = AsRenderStateSnapshot(s_renderStateSnapshot)->drawPrimitiveType;
    g_drawUsesBoundVertexBuffer = AsRenderStateSnapshot(s_renderStateSnapshot)->drawUsesBoundVertexBuffer;
    g_drawUsesIndexDisplayList = AsRenderStateSnapshot(s_renderStateSnapshot)->drawUsesIndexDisplayList;
    g_boundBufferDrawArgument = AsRenderStateSnapshot(s_renderStateSnapshot)->boundBufferDrawArgument;
    RenderState_CopyEightWords(s_vertexFormatDescriptor, (unsigned int*)((char*)&AsRenderStateSnapshot(s_renderStateSnapshot)->formatDescriptor));
}

extern char s_fixedVertexFormatDescriptors[0x5A0];
extern char __vt__Q23gtl11IG3DXBuffer[256];

void RenderState_CopyEightWords(unsigned int*, unsigned int*);
void IndexBuffer_Flush(void* p);
void GraphicsBuffer_Flush(void* p);
void IndexBuffer_GetOffsetPtr(void* p, int a, int b, void* c, void* d);
void Resource_GetOffsetPtr(void* p, int a, int b, void* c, void* d);
void Render_WriteFifo3f(float a, float b, float c);
void Render_WriteNormal3f(float a, float b, float c);
void Render_WriteTexCoord2f(float a, float b);
void GraphicsResource_Release(void* p);
void gtl_Graphics_FreeWithCallback(void* p);
void VertexBuffer_GetDescription(void* p, void* out);
void Engine_SetVertexDeclaration(void* p);
void* FlexibleVertexFormat_GetStride(void* p);
void Engine_SetVertexStreamZeroOffset(int a, void* b, void* c);
void DrawPrimitiveFromBoundBuffers(void* a, int b, void* c);
void Engine_SetIndexBuffer(void* a, int b);
void Engine_DrawIndexedPrimitive(void* a, int b, int c, void* d, int e, void* f);

extern void* __alloca(unsigned int);
#pragma optimization_level 0
#pragma optimize_for_size on
// Emit packed vertices to GX as position, normal, basis1, basis2, then UV.
// Input stride is 14 floats: XYZ[0..2], UV[3..4], normal[5..7], basis1[8..10],
// basis2[11..13]. Input is borrowed; count is a vertex count, not a float count.
// All vectors/UVs pass through unchanged: caller supplies the required spaces.
// Each vertex writes 12 vector components and two UV components to the FIFO.
void Render_WriteVerticesPositionNormalBasisTex1(float* input, volatile unsigned int count) {
    if (0) __alloca(0);
    float* p = input;
    for (unsigned int i = 0; i < count; p += kBasisVertexFloatCount, ++i) {
        Render_WriteFifo3f(p[0], p[1], p[2]);
        Render_WriteNormal3f(p[5], p[6], p[7]);
        Render_WriteNormal3f(p[8], p[9], p[10]);
        Render_WriteNormal3f(p[11], p[12], p[13]);
        Render_WriteTexCoord2f(p[3], p[4]);
    }
}
#pragma optimize_for_size reset
#pragma optimization_level reset

#pragma optimization_level 0
#pragma optimize_for_size on
// Find the fixed-format key and copy its eight-word vertex input descriptor; leave destination unchanged on a miss.
void VertexInput_FindAndCopyFormatDescriptor(void* descriptorOut, volatile unsigned int formatFlags) {
    if (0) __alloca(0);
    unsigned int i = 0;
    for (; i < kFormatPatternCount; ++i) {
        unsigned char* entry = (unsigned char*)&s_fixedVertexFormatDescriptors + i * kSnapshotSamplerRowBytes;
        if (*(unsigned int*)entry == formatFlags) {
            RenderState_CopyEightWords((unsigned int*)descriptorOut, (unsigned int*)(entry + 4));
            return;
        }
    }
}
#pragma optimize_for_size reset
#pragma optimization_level reset

// Copy the eight words of a vertex input format descriptor.
void RenderState_CopyEightWords(unsigned int* dst, unsigned int* src) {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    dst[4] = src[4];
    dst[5] = src[5];
    dst[6] = src[6];
    dst[7] = src[7];
}

#pragma optimization_level 0
#pragma opt_propagation off
#pragma opt_common_subs off
// Forward index-buffer data outputs in the buffer API's pointer/size order.
void gtl_IG3DXPrimitive_GetIndexData(void* p, void* a, void* b) {
    void* q = p;
    IndexBuffer_GetOffsetPtr(AsGtlPrimitive(q)->indexBuffer, 0, 0, b, a);
}
#pragma opt_common_subs reset
#pragma opt_propagation reset

// Forward vertex-buffer data outputs in the buffer API's pointer/size order.
void gtl_IG3DXPrimitive_GetVertexData(void* p, void* a, void* b) {
    void* q = p;
    Resource_GetOffsetPtr(AsGtlPrimitive(q)->vertexBuffer, 0, 0, b, a);
}
#pragma optimization_level reset

#pragma optimization_level 0
#pragma use_lmw_stmw on
// Emit each 16-bit index three times to the GX write-gather FIFO.
void Render_WriteTriangleIndices16(unsigned short* p, volatile unsigned int count) {
    unsigned int i;
    for (i = 0; i < count; ++i) {
        unsigned short a = p[0];
        *(volatile unsigned short*)kGxWriteGatherPipe = a;
        unsigned short b = p[0];
        *(volatile unsigned short*)kGxWriteGatherPipe = b;
        unsigned short c = p[0];
        *(volatile unsigned short*)kGxWriteGatherPipe = c;
        ++p;
    }
}
#pragma optimization_level reset
#pragma use_lmw_stmw reset

// Reserved graphics hook with no work in this build.
void Render_ReservedNoOp() {}

extern unsigned int gtl_Graphics_AllocateWithCallback(void*);
extern void gtl_Graphics_FreeWithCallback(void*);
extern char __vt__Q23gtl11IG3DXBuffer[256];
extern char s_fixedVertexFormatDescriptors[0x5A0];
void GtlRecordDeque_DestroyStorage(void*, short);

extern char __vt__Q23gtl14IG3DXPrimitive[];
extern char s_gtlReferenceCountedBaseVtable[];
extern char s_vertexDeclarationPatterns[];
void VertexDeclaration_CreateCopy(void*, void*);
int VertexDeclaration_TryConvertToFormatFlags(void*, void*);
void VertexBuffer_Create(int, int, unsigned int, int, void*, int);
void IndexBuffer_Create(int, int, int, int, void*, int, int);
int Primitive_GetIndexCount(void*, void*);
void VertexDeclaration_GetElements(unsigned int, void*, void*);
void IUnknownCustom_dtor(void*);

struct VertexDeclarationElement {
    unsigned int streamAndOffset[2];
    unsigned char storageType;
    unsigned char id;
    char methodAndUsageIndex[2];
};

struct VertexDeclarationPattern {
    unsigned int* ids;
    int elementCount;
    unsigned int data[8];
};

#pragma dont_inline on

#pragma optimization_level 0
// Resolve a declaration through fixed-format flags or the explicit declaration-pattern table.
void VertexInput_CopyDescriptorForDeclaration(void* descriptorOut, volatile unsigned int declaration) {
    unsigned int elementCount;
    unsigned int formatFlags;
    VertexDeclarationElement elements[17];
    VertexDeclaration_GetElements(declaration, elements, &elementCount);
    int attributeCount = (int)elementCount - 1;
    int hasFixedFormat = VertexDeclaration_TryConvertToFormatFlags(elements, &formatFlags);
    if (hasFixedFormat != 0) {
        VertexInput_FindAndCopyFormatDescriptor(descriptorOut, formatFlags);
        return;
    }
    for (unsigned int i = 0; i < 1; ++i) {
        VertexDeclarationPattern* pattern = (VertexDeclarationPattern*)(s_vertexDeclarationPatterns + i * 0x28);
        if (attributeCount != pattern->elementCount)
            continue;
        int matched = 0;
        for (int j = 0; j < attributeCount; ++j) {
            if ((int)elements[j].id != (int)pattern->ids[j])
                break;
            if (j == attributeCount - 1) {
                RenderState_CopyEightWords((unsigned int*)descriptorOut, pattern->data);
                matched = 1;
            }
        }
        if (matched != 0)
            return;
    }
}
#pragma optimization_level reset

#pragma dont_inline reset

struct PrimitiveConstructorView {
    unsigned int referenceCount;
    void* vtable;
    unsigned int vertexBuffer;
    unsigned int indexBuffer;
    int primitiveCount;
    int vertexCount;
    unsigned int creationFlags;
    unsigned int vertexDeclaration;
    void* primitiveType;
};

#pragma optimization_level 0
#pragma optimize_for_size on
// Initialize the primitive's base vtable and its single owned reference.
inline void InitPrimitiveBase(PrimitiveConstructorView* o) {
    o->vtable = s_gtlReferenceCountedBaseVtable;
    o->referenceCount = 1;
}
// Store a resource-address word through the constructor's packed pointer ABI.
inline void InitPrimitivePointer(unsigned int* p, unsigned int value) {
    *p = value;
}
#pragma optimize_for_size off
// Create a declaration copy and allocate vertex/index buffers for the requested primitive.
void* gtl_IG3DXPrimitive_Ctor(PrimitiveConstructorView* primitive, int primitiveCount, int vertexCount, volatile unsigned int creationFlags, void* declaration, void* primitiveType) {
    PrimitiveConstructorView* base = primitive;
    InitPrimitiveBase(primitive);
    primitive->vtable = (void*)__vt__Q23gtl14IG3DXPrimitive;
    InitPrimitivePointer(&primitive->vertexBuffer, 0);
    InitPrimitivePointer(&primitive->indexBuffer, 0);
    primitive->primitiveCount = primitiveCount;
    primitive->vertexCount = vertexCount;
    primitive->creationFlags = creationFlags;
    primitive->vertexDeclaration = 0;
    primitive->primitiveType = primitiveType;
    VertexDeclaration_CreateCopy(declaration, &primitive->vertexDeclaration);
    unsigned int formatFlags;
    VertexDeclaration_TryConvertToFormatFlags(declaration, &formatFlags);
    int vertexStride = (int)(unsigned int)FlexibleVertexFormat_GetStride((void*)formatFlags);
    VertexBuffer_Create(vertexCount * vertexStride, 0, formatFlags, 0, (void*)&primitive->vertexBuffer, 0);
    int indexCount = Primitive_GetIndexCount((void*)primitiveCount, primitiveType);
    IndexBuffer_Create(indexCount << 1, 0, 3, 0, (void*)&primitive->indexBuffer, 0, 0);
    return primitive;
}
#pragma optimize_for_size reset
#pragma optimization_level reset

#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma optimize_for_size on
// Create a reference-counted graphics buffer using the configured allocation callback.
void* gtl_IG3DXBuffer_Ctor(void* p, void* data) {
    void* base = p;
    (*(unsigned int*)&AsGtlBuffer(p)->vtable) = (unsigned int)s_gtlReferenceCountedBaseVtable;
    AsGtlBuffer(p)->referenceCount = 1;
    (*(unsigned int*)&AsGtlBuffer(p)->vtable) = (unsigned int)__vt__Q23gtl11IG3DXBuffer;
    AsGtlBuffer(p)->buffer = 0;
    (*(unsigned int*)&AsGtlBuffer(p)->sourceData) = (unsigned int)data;
    AsGtlBuffer(p)->buffer = gtl_Graphics_AllocateWithCallback(data);
    return p;
}
#pragma optimization_level reset
#pragma use_lmw_stmw reset
#pragma optimize_for_size reset

#pragma optimization_level 0
// Free the owned buffer, then optionally free the wrapper.
void* gtl_IG3DXBuffer_dtor(void* p, volatile short freeFlag) {
    if (p != 0) {
        AsGtlBuffer(p)->vtable = (void*)&__vt__Q23gtl11IG3DXBuffer;
        if (AsGtlBuffer(p)->buffer != 0) {
            gtl_Graphics_FreeWithCallback((*(void**)&AsGtlBuffer(p)->buffer));
            AsGtlBuffer(p)->buffer = 0;
        }
        if (freeFlag > 0) {
            gtl_Graphics_FreeWithCallback(p);
        }
    }
    return p;
}
#pragma optimization_level reset

#pragma optimization_level 0
#pragma opt_lifetimes off
#pragma opt_common_subs off
// Release vertex, index and declaration resources, then optionally free the primitive.
void* gtl_IG3DXPrimitive_dtor(void* p, volatile short freeFlag) {
    if (p != 0) {
        AsGtlPrimitive(p)->vtable = (void*)__vt__Q23gtl14IG3DXPrimitive;
        if ((*(unsigned int*)&AsGtlPrimitive(p)->vertexBuffer) != 0) {
            GraphicsResource_Release(AsGtlPrimitive(p)->vertexBuffer);
            (*(unsigned int*)&AsGtlPrimitive(p)->vertexBuffer) = 0;
        }
        if ((*(unsigned int*)&AsGtlPrimitive(p)->indexBuffer) != 0) {
            GraphicsResource_Release(AsGtlPrimitive(p)->indexBuffer);
            (*(unsigned int*)&AsGtlPrimitive(p)->indexBuffer) = 0;
        }
        if (AsGtlPrimitive(p)->vertexDeclaration != 0) {
            GraphicsResource_Release((*(void**)&AsGtlPrimitive(p)->vertexDeclaration));
            AsGtlPrimitive(p)->vertexDeclaration = 0;
        }
        if (freeFlag > 0)
            gtl_Graphics_FreeWithCallback(p);
    }
    return p;
}
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma optimization_level reset

extern void* __alloca(unsigned int);
#pragma optimization_level 0
#pragma optimize_for_size on
// Bind the primitive declaration and vertex stream, then issue indexed or unindexed drawing.
int gtl_IG3DXPrimitive_Draw(void* primitive) {
    __alloca(0);
    unsigned int bufferDescription[4];
    void* vertexBuffer = AsGtlPrimitive(primitive)->vertexBuffer;
    VertexBuffer_GetDescription(vertexBuffer, bufferDescription);
    Engine_SetVertexDeclaration((*(void**)&AsGtlPrimitive(primitive)->vertexDeclaration));
    void* vertexStride = FlexibleVertexFormat_GetStride((void*)bufferDescription[3]);
    Engine_SetVertexStreamZeroOffset(0, vertexBuffer, vertexStride);
    if (AsGtlPrimitive(primitive)->indexBuffer == 0) {
        DrawPrimitiveFromBoundBuffers(AsGtlPrimitive(primitive)->primitiveType, 0, AsGtlPrimitive(primitive)->primitiveCount);
    } else {
        Engine_SetIndexBuffer(AsGtlPrimitive(primitive)->indexBuffer, 0);
        Engine_DrawIndexedPrimitive(AsGtlPrimitive(primitive)->primitiveType, 0, 0,
                    AsGtlPrimitive(primitive)->vertexCount, 0, AsGtlPrimitive(primitive)->primitiveCount);
    }
    return 1;
}
#pragma optimization_level reset
#pragma optimize_for_size reset

// Return the primitive's vertex-buffer handle as an ABI word.
unsigned int Object_GetField8(void* p) {
    return (*(unsigned int*)&AsGtlPrimitive(p)->vertexBuffer);
}

// Return the primitive's index-buffer handle as an ABI word.
unsigned int Object_GetFieldC(void* p) {
    return (*(unsigned int*)&AsGtlPrimitive(p)->indexBuffer);
}

#pragma optimization_level 0
// Flush pending index-buffer writes.
void gtl_IG3DXPrimitive_FlushIndexBuffer(void* p) {
    p = (char*)p + 0;
    IndexBuffer_Flush(AsGtlPrimitive(p)->indexBuffer);
}
#pragma optimization_level reset

#pragma optimization_level 0
// Flush pending vertex-buffer writes.
void gtl_IG3DXPrimitive_FlushVertexBuffer(void* p) {
    void* q = p;
    GraphicsBuffer_Flush(AsGtlPrimitive(q)->vertexBuffer);
}
#pragma optimization_level reset

// Copy six words, preserving the original load/store ordering for overlapping storage.
void WordBlock_CopySixWords(unsigned int* dst, unsigned int* src) {
    unsigned int e;
    unsigned int b;
    unsigned int c;
    unsigned int d;
    unsigned int a;
    unsigned int f;
    a = src[0];
    b = src[1];
    dst[0] = a;
    dst[1] = b;
    c = src[2];
    dst[2] = c;
    d = src[3];
    e = src[4];
    dst[3] = d;
    dst[4] = e;
    f = src[5];
    dst[5] = f;
}

int VIGetDTVStatus(void);
void* GtlRecordDeque_Dtor(void* p, short a);
void __dl__FPv(void* p);

// Map the console digital-output status to the engine's video-output code.
int Video_GetDtvOutputCode(void) {
    switch (VIGetDTVStatus()) {
    case 1: return 1;
    case 0: return 6;
    default: return 0;
    }
}

#pragma optimization_level 0
#pragma optimize_for_size on
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
struct DequeStorageWords { unsigned int data, size, capacity; };

// Initialize the deque storage pointer, element count and capacity to zero.
void GtlRecordDeque_InitEmpty(void* volatile p) {
    void* a = p;
    void* b = a;
    void* d;
    void* e;
    void* f;
    void* c = b;
    void* volatile saved = b;
    d = (char*)&((DequeStorageWords*)b)->capacity;
    e = (char*)&((DequeStorageWords*)b)->capacity;
    f = (char*)&((DequeStorageWords*)b)->capacity;
    ((DequeStorageWords*)a)->data = 0;
    ((DequeStorageWords*)a)->size = 0;
    ((DequeStorageWords*)a)->capacity = 0;
}
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma optimize_for_size reset
#pragma optimization_level reset

// Destroy the deque wrapper and optionally release its allocation.
void* dtor_80432D20(void* p, short freeFlag) {
    volatile short flag = freeFlag;
    if (p != 0) {
        GtlRecordDeque_Dtor(p, 0);
        if (flag > 0)
            __dl__FPv(p);
    }
    return p;
}

// Destroy deque storage and optionally release its allocation.
void* GtlRecordDeque_Dtor(void* p, short freeFlag) {
    volatile short flag = freeFlag;
    if (p != 0) {
        if (p != 0) {
            GtlRecordDeque_DestroyStorage(p, 0);
        }
        if (flag > 0)
            __dl__FPv(p);
    }
    return p;
}

} // extern "C"
