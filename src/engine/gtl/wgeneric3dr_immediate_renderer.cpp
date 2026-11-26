// WGeneric3DR immediate-mode drawing for menu and HUD use: packed texture slots and stage
// bindings, blend and render-flag state, and the command and property dispatch.

#include <decomp/static_init.h>
// WGeneric3DR immediate rendering for menu/HUD consumers: packed texture slots,
// vertex formats, blend/depth/fog state, and renderer command/property dispatch.
// Widescreen lookup: DrawImmediatePrimitive is a CONTEXT/return-address site in
// already prepared by callers; it sets no projection, viewport, scissor or video
// resolution. Width/height getters expose renderer cached dimensions. Camera
// projection/target updates live in wview_camera_projection.cpp; UI geometry belongs to layouts.
// Texture/model modding lookup:
// - WGeneric3DR_LoadTextureSlot reads asset width/height/format from the shared
//   resource description (words 0/1/3); this unit does not choose a texture format.
//   Preserve description layout and the distinct stage-resource/palette handles.
// - kTextureSlotCount=2048; packed draw mode uses 11 bits for stage 0 but only
//   7 for stage 1. Increasing pool capacity alone cannot expand those encodings.
// - ImmediateVertexFormat values are layout KEYS, not GX texture formats. Keep
//   key, VertexFormat_GetStride result and caller buffer layout consistent.
// - RendererBlendMode/DrawTextureOperation select equations; preserve engine/GX
//   factor domains, stage existence checks, argument selection and clamping.
//   Sampler/depth/fog flags are wrapper bits translated to engine state IDs.
// API contract: required object is a live WGeneric3DR (shared header schema).
// Vertices are borrowed for immediate submission; the caller supplies enough
// complete stride-sized records. Texture slots/cache and engine state mutate;
// serialize draw/loading/teardown. No routine here restores arbitrary caller
// render state. Opaque resource handles are not CPU pixel-buffer pointers.
// Related default tables: s_rendererDefaultMaterial, s_rendererDefaultTextureStageStates,
// s_textureStageStateIds; their encoded data remains shared and unchanged.
// Preserve all state-cache checks, duplicate resource calls, integerized ABI
// words and primitive count adjustments, including the unusual strip mapping.
#include "classes/WGeneric3DR.h"
#include <decomp/cw_bitfields.h>
#include "classes/CGsrResource.h"
#define AsWGeneric3DR(p) ((WGeneric3DR*)(p))

extern "C" {

// Each texture slot owns a shared resource and caches the two handles used by GX binding.
struct RendererTextureSlot {
    void* sharedTexture;
    void* resourceHandle;
    void* paletteObject; // +8: borrowed palette cache; this record view is local to this unit.
    int bindingState;
    char reserved[12];
    int width, height, format, uploadState;
};
struct RendererTextureTable {
    char prefix[0x49a4];
    RendererTextureSlot slots[0x800];
};
typedef char RendererTextureStrideCheck[sizeof(RendererTextureSlot) == 0x2c ? 1 : -1];
// Flags are wrapper bitfields, not engine state IDs. In particular bit 27 writes
// engine state 1, which invalidates FOG, not depth (Engine_SetCachedRenderState).
enum RendererFlags {
    kTextureSlotCount = 0x800,
    kTexture0IndexMask = 0x7ff,
    kTexture1IndexMask = 0x7f,
    kTexture1IndexShift = 11,
    kPackedTextureIndicesMask = 0x3ffff,
    kRenderStateMask = 0xaa380000,
    kDepthCompareMask = 0x00300000,
    kTextureAddressMode0 = 0x02000000,
    kTextureAddressMode1 = 0x00080000,
    kFogEnableFlag = 0x08000000,
    kDisableDepthWrite = 0x20000000,
    kEnableAlphaTest = 0x80000000
};

extern float kFloat_6_2831855;
extern float s_twoPi_80088DD4;

extern void* Engine_SetCachedRenderState(int, ...);
extern void* ISharedResource_GetTextureDescription(void*);
extern void* ISharedResource_GetLocked(void*);
extern void* ISharedResource_GetPaletteObject(void*);
extern void Engine_SetTextureStageState(int, int, int);
extern void UIRender_SetDefaultState();
extern void Engine_SetMaterial(void*);
extern void Engine_SetFlexibleVertexFormat(int);
extern void* Engine_SetTextureStageResource(int, int);
extern void* Engine_SetTexturePalette(int, int);
extern void Engine_SetTextureStageStateList(int, void*, int);
extern void SetTransform(int, void*);
extern void Engine_SetType7Resource(int);
extern void* DrawPrimitiveUP(int, int, void*, int);
extern int s_rendererDefaultMaterial[];
extern int s_rendererDefaultTextureStageStates[];
extern int kIdentityMatrix4x4[];
extern int s_textureStageStateIds[];
extern int jumptable_804ABDE8[];
extern int jumptable_804ABE6C[];

enum GenericEngineStateId {
    kFogEnableState = 1,
    kLightingState = 2,
    kDepthCompareState = 3,
    kDepthWriteState = 5,
    kViewMatrixOnlyState = 9,
    kAlphaReferenceState = 11,
    kSourceBlendState = 13,
    kDestinationBlendState = 14
};
// Query ABI: unresolved properties have only initialization/query uses in the
// current source. Keep their IDs explicit rather than inventing capabilities.
enum RendererCommand {
    kRendererNoOp5 = 5,
    kRendererNoOp6 = 6,
    kRendererNoOp7 = 7,
    kRendererNoOp8 = 8,
    kRendererSetPresentationTarget = 9,
    kRendererNoOp10 = 10,
    kRendererForwardTargetValue = 11,
    kRendererNoOp12 = 12,
    kRendererNoOp13 = 13,
    kRendererReleaseSharedResource = 14,
    kRendererNoOp15 = 15,
    kRendererNoOp16 = 16,
    kRendererNoOp17 = 17,
    kRendererNoOp18 = 18,
    kRendererNoOp19 = 19,
    kRendererNoOp20 = 20,
    kRendererSetClampedSetting21 = 21,
    kRendererSetBooleanSetting22 = 22,
    kRendererSetNonnegativeSetting23 = 23,
    kRendererSetBoundedSetting24 = 24,
    kRendererSetSamplerSetting25 = 25,
    kRendererQueryWord4844 = 26,
    kRendererNoOp27 = 27,
    kRendererQueryLoadedTextureCount = 28,
    kRendererNoOp29 = 29,
    kRendererNoOp30 = 30,
    kRendererQueryProperty = 31,
    kRendererApplyInitializedSetting32 = 32,
    kRendererNoOp33 = 33
};
enum ImmediatePrimitive {
    kImmediateTriangles = 0x90,
    kImmediateTriangleStrip = 0x98,
    kImmediateTriangleFan = 0xA0,
    kImmediateLines = 0xA8,
    kImmediateLineStrip = 0xB0
};
enum RendererPropertyId {
    kPropertyNoOutput = 0,
    kPropertyFlags4888BothSet = 1,
    kPropertyUnresolvedWord4850 = 2,
    kPropertyWindowFormatBit29 = 3,
    kPropertyDefaultTextureDimensions = 4,
    kPropertyUnresolvedWord4900 = 5,
    kPropertyUnresolvedWord4904 = 6,
    kPropertyUnresolvedWord4914 = 7,
    kPropertyUnresolvedWord4918 = 8,
    kPropertyUnresolvedFloat491C = 9,
    kPropertyUnresolvedWord4930 = 10,
    kPropertyUnresolvedWord4938 = 11
};
// These are GX factors, passed unchanged by Render_ApplyBlendState.
enum RendererBlendFactor {
    kBlendZero = 0,
    kBlendOne = 1,
    kBlendSourceColor = 2,
    kBlendInverseSourceColor = 3,
    kBlendSourceAlpha = 4,
    kBlendInverseSourceAlpha = 5
};
enum RendererBlendMode {
    kBlendAlphaOver = 0,
    kBlendReplace = 0x40000000,
    kBlendAlphaAdd = 0x00800000,
    kBlendAdd = 0x40800000,
    kBlendMultiply = 0x01000000,
    kBlendMultiplyAlternate = 0x41000000,
    kBlendInverseMultiply = 0x01800000,
    kBlendInverseMultiplyAlternate = 0x41800000
};
// TextureStage_ConfigureTevColorCombiner (gtl_texture_stage_tev_setup.cpp) decodes color operations:
// 0 = first*second, 2 = first+second, 16 = keep previous, 17/18 = first/second argument.
// The alpha translator (gx_fifo_vertex_and_tev_helpers.cpp) uses 0=multiply and 17/18=selectedArg*K,
// where K is the active konst selector. Do not assume selection ignores K.
// Every final equation clamps to [0,1]. This unit chooses operations, not arguments.
enum DrawTextureOperation {
    kTextureMultiply = 0,
    kTextureAdd = 2,
    kTextureKeepPrevious = 0x10,
    kTextureSelectFirst = 0x11,
    kTextureSelectSecond = 0x12
};
// Established layouts share the names/evidence of VertexFormat_GetStride;
// opaque encodings retain their hex key instead of guessing storage fields.
enum ImmediateVertexFormat {
    kVertexXYZUv = 0x110001,
    kVertexXYZUvAlt = 0x110005,
    kVertexXYZColor = 0xF001,
    kVertexXYZColorAlt = 0xF005,
    kVertexXYZColorUv = 0x11F001,
    kVertexXYZColorUvAlt = 0x11F005,
    kVertexXYZNormalUv = 0x110401,
    kVertexXYZNormalColorUv = 0x11F401,
    kVertexLayout401 = 0x401,
    kVertexLayoutF401 = 0xF401,
    kVertexLayout51F541 = 0x51F541,
    kVertexLayout510541 = 0x510541,
    kVertexLayout510521 = 0x510521,
    kVertexLayout510511 = 0x510511,
    kVertexLayout510531 = 0x510531,
    kVertexLayout51F521 = 0x51F521,
    kVertexLayout51F511 = 0x51F511,
    kVertexLayout51F531 = 0x51F531,
    kVertexLayout120005 = 0x120005,
    kVertexLayout40120005 = 0x40120005,
    kVertexLayout120401 = 0x120401,
    kVertexLayout12F401 = 0x12F401,
    kVertexLayout12F005 = 0x12F005,
    kVertexLayout12F001 = 0x12F001,
    kVertexLayout40120401 = 0x40120401,
    kVertexLayout4012F401 = 0x4012F401,
    kVertexLayout4012F005 = 0x4012F005,
    kVertexLayout4000F005 = 0x4000F005,
    kVertexLayout4000F001 = 0x4000F001
};
enum ImmediateDrawBits {
    kDefaultDrawFlags = 0x1004,
    kDrawSettingShift = 4,
    kDrawSettingMask = 0xF,
    kDrawModeAddSecondTextureColor = 0x40000,
    kDrawModeTextureOperationShift = 18,
    kCachedAddSecondTextureColor = 0x40000000,
    kCachedVertexFormatMask = 0x7FFFFFFF,
    kSubmittedVertexFormatMask = 0x3FFFFFFF,
    kPackedBlendModeMask = 0x41800000,
    kTextureTransformSlot0 = 0xB,
    kImmediateWorldTransform = 2,
    kAlphaTestReference = 0x80
};
// Local vtable ABI view: +0x58 returns a BYTE STRIDE (VertexFormat_GetStride).
// The shared +0x49A0 field caches this byte stride.
// +0x64/+0x68 dispatch render flags and blend mode respectively.
struct RendererDrawDispatch {
    virtual void p0();
    virtual void p1();
    virtual void p2();
    virtual void p3();
    virtual void p4();
    virtual void p5();
    virtual void p6();
    virtual void p7();
    virtual void p8();
    virtual void p9();
    virtual void p10();
    virtual void p11();
    virtual void p12();
    virtual void p13();
    virtual void p14();
    virtual void p15();
    virtual void p16();
    virtual void p17();
    virtual void p18();
    virtual void p19();
    virtual unsigned int GetVertexStride(unsigned int);
    virtual void p21();
    virtual void p22();
    virtual void ApplyRenderFlags(unsigned int);
    virtual void SetBlendMode(unsigned int);
};
// Material is copied as 17 words; individual component semantics are not needed here.
struct MaterialWords {
    int words[17];
};
struct TextureStatePair {
    int stateId, value;
};
struct TextureStageStatePairs {
    TextureStatePair entries[6];
};
typedef char MaterialWordCopySize[sizeof(MaterialWords) == 68 ? 1 : -1];
typedef char TextureStatePairCopySize[sizeof(TextureStageStatePairs) == 48 ? 1 : -1];

// Inline the three state-cache checks at each supported draw-format branch.
// Apply each changed texture operation only when its stage exists.
static inline void SetCachedTextureOperations(char* self, int stage0ColorOperationValue, int stage1ColorOperationValue,
                                              int stage0AlphaOperationValue) {
    if (AsWGeneric3DR(self)->stage0ColorOperation != (stage0ColorOperationValue) &&
        *(int*)&AsWGeneric3DR(self)->textureStageCount > 0) {
        Engine_SetTextureStageState(0, s_textureStageStateIds[2], (stage0ColorOperationValue));
        AsWGeneric3DR(self)->stage0ColorOperation = (stage0ColorOperationValue);
    }
    if (AsWGeneric3DR(self)->stage1ColorOperation != (stage1ColorOperationValue) &&
        *(int*)&AsWGeneric3DR(self)->textureStageCount > 1) {
        Engine_SetTextureStageState(1, s_textureStageStateIds[2], (stage1ColorOperationValue));
        AsWGeneric3DR(self)->stage1ColorOperation = (stage1ColorOperationValue);
    }
    if (AsWGeneric3DR(self)->stage0AlphaOperation != (stage0AlphaOperationValue) &&
        *(int*)&AsWGeneric3DR(self)->textureStageCount > 0) {
        Engine_SetTextureStageState(0, s_textureStageStateIds[5], (stage0AlphaOperationValue));
        AsWGeneric3DR(self)->stage0AlphaOperation = (stage0AlphaOperationValue);
    }
}

// Read the cached resource handle for a texture slot.
// Retain signed byte stepping and word loads; pointer-array loads change register allocation.
static inline int TextureResourceHandle(char* renderer, int slot) {
    return *(int*)((char*)&((RendererTextureTable*)renderer)->slots[0].resourceHandle +
                   slot * (int)sizeof(RendererTextureSlot));
}
// Read the borrowed cached palette object for a valid texture slot [0,2048).
// The int return preserves a Wii pointer word; Engine_SetTexturePalette consumes it.
static inline int TexturePaletteHandle(char* renderer, int slot) {
    return *(int*)&((RendererTextureTable*)(renderer + slot * (int)sizeof(RendererTextureSlot)))
                ->slots[0]
                .paletteObject;
}

// Decode one packed texture index and bind it only when that stage changes.
static inline void BindChangedTextureStage(char* self, int stage, unsigned int idx, unsigned int prev) {
    int sh = stage ? kTexture1IndexShift : 0;
    unsigned int m = stage ? kTexture1IndexMask : kTexture0IndexMask;
    int n = (idx >> sh) & m;
    if (n != ((prev >> sh) & m)) {
        Engine_SetTextureStageResource(stage, TextureResourceHandle(self, n));
        int palette = TexturePaletteHandle(self, n);
        if (palette)
            Engine_SetTexturePalette(stage, palette);
        else
            Engine_SetTexturePalette(stage, 0);
    }
}

// Bind the borrowed override texture and invalidate the packed-index cache.
static inline void BindOverrideDrawTexture(char* self) {
    if (ISharedResource_GetLocked(AsWGeneric3DR(self)->overrideTexture)) {
        Engine_SetTextureStageResource(0, (int)ISharedResource_GetLocked(AsWGeneric3DR(self)->overrideTexture));
        Engine_SetTexturePalette(0, (int)ISharedResource_GetPaletteObject(AsWGeneric3DR(self)->overrideTexture));
        *(int*)&AsWGeneric3DR(self)->boundTextureIndices = 0;
    }
}

// Select the cached texture-operation tuple for the encoded vertex format.
// Unlisted formats preserve the previous operations; 0x110001 also tests +1A9D8.
static inline void ApplyDrawFormatTextureOperations(char* self) {
    switch (AsWGeneric3DR(self)->cachedVertexFormat & kCachedVertexFormatMask) {
    case kVertexXYZUv: // Select/multiply stage 0 color via +1A9D8; multiply alpha; keep stage 1.
        if (AsWGeneric3DR(self)->stage0ColorMultiplyEnabled) {
            if (AsWGeneric3DR(self)->stage0ColorOperation != kTextureMultiply &&
                AsWGeneric3DR(self)->textureStageCount > 0) {
                Engine_SetTextureStageState(0, s_textureStageStateIds[2], kTextureMultiply);
                AsWGeneric3DR(self)->stage0ColorOperation = kTextureMultiply;
            }
        } else {
            if (AsWGeneric3DR(self)->stage0ColorOperation != kTextureSelectFirst &&
                AsWGeneric3DR(self)->textureStageCount > 0) {
                Engine_SetTextureStageState(0, s_textureStageStateIds[2], kTextureSelectFirst);
                AsWGeneric3DR(self)->stage0ColorOperation = kTextureSelectFirst;
            }
        }
        if (AsWGeneric3DR(self)->stage1ColorOperation != kTextureKeepPrevious &&
            AsWGeneric3DR(self)->textureStageCount > 1) {
            Engine_SetTextureStageState(1, s_textureStageStateIds[2], kTextureKeepPrevious);
            AsWGeneric3DR(self)->stage1ColorOperation = kTextureKeepPrevious;
        }
        if (AsWGeneric3DR(self)->stage0AlphaOperation != kTextureMultiply &&
            AsWGeneric3DR(self)->textureStageCount > 0) {
            Engine_SetTextureStageState(0, s_textureStageStateIds[5], kTextureMultiply);
            AsWGeneric3DR(self)->stage0AlphaOperation = kTextureMultiply;
        }
        break;
    case kVertexXYZUvAlt: // Select first arguments on stage 0; keep stage 1.
        SetCachedTextureOperations(self, kTextureSelectFirst, kTextureKeepPrevious, kTextureSelectFirst);
        break;
    // Select second color/alpha arguments on stage 0; keep stage 1.
    case kVertexLayout401:
    case kVertexXYZColor:
    case kVertexLayoutF401:
    case kVertexXYZColorAlt:
        SetCachedTextureOperations(self, kTextureSelectSecond, kTextureKeepPrevious, kTextureSelectSecond);
        break;
    // Multiply color/alpha arguments on stage 0; keep stage 1.
    case kVertexXYZColorUv:
    case kVertexXYZNormalUv:
    case kVertexXYZNormalColorUv:
    case kVertexXYZColorUvAlt:
    case kVertexLayout51F541:
    case kVertexLayout510541:
    case kVertexLayout510521:
    case kVertexLayout510511:
    case kVertexLayout510531:
    case kVertexLayout51F521:
    case kVertexLayout51F511:
    case kVertexLayout51F531:
        SetCachedTextureOperations(self, kTextureMultiply, kTextureKeepPrevious, kTextureMultiply);
        break;
    case kVertexLayout120005: // Select stage 0 first arguments; multiply stage 1 color.
        SetCachedTextureOperations(self, kTextureSelectFirst, kTextureMultiply, kTextureSelectFirst);
        break;
    case kVertexLayout40120005: // Select stage 0 first arguments; add stage 1 color.
        SetCachedTextureOperations(self, kTextureSelectFirst, kTextureAdd, kTextureSelectFirst);
        break;
    // Multiply both color stages and stage 0 alpha.
    case kVertexLayout120401:
    case kVertexLayout12F401:
    case kVertexLayout12F005:
    case kVertexLayout12F001:
        SetCachedTextureOperations(self, kTextureMultiply, kTextureMultiply, kTextureMultiply);
        break;
    // Multiply stage 0 color/alpha; add stage 1 color.
    case kVertexLayout40120401:
    case kVertexLayout4012F401:
    case kVertexLayout4012F005:
        SetCachedTextureOperations(self, kTextureMultiply, kTextureAdd, kTextureMultiply);
        break;
    // Select stage 0 second arguments; add stage 1 color.
    case kVertexLayout4000F005:
    case kVertexLayout4000F001:
        SetCachedTextureOperations(self, kTextureSelectSecond, kTextureAdd, kTextureSelectSecond);
        break;
    }
}

// Convert vertex count to the immediate draw ABI's primitive count.
static inline void AdjustImmediatePrimitiveCount(int primitive, int& count) {
    // Input count is vertices; output count follows DrawPrimitiveUP conventions:
    // 0x90 -> /3; 0x98 -> -1; 0xA0 -> -2; 0xA8 -> /2; 0xB0 -> -1.
    // Preserve these exact adjustments (including strip behavior); no minimum-count
    // checks occur here. Caller supplies a supported topology and sufficient vertices.
    switch (primitive - kImmediateTriangles) {
    case kImmediateTriangleFan - kImmediateTriangles:
        count -= 2;
        break;
    case kImmediateTriangleStrip - kImmediateTriangles:
        count -= 1;
        break;
    case kImmediateTriangles - kImmediateTriangles:
        count /= 3;
        break;
    case kImmediateLineStrip - kImmediateTriangles:
        count -= 1;
        break;
    case kImmediateLines - kImmediateTriangles:
        count /= 2;
        break;
    }
}

} // extern "C"

#pragma push
#pragma use_lmw_stmw on
// Configure material, vertex format and texture stages, then draw the supplied immediate vertices.
// mode packs texture indices/render flags; mask is the encoded vertex layout;
// count is INPUT VERTICES (converted to primitives below), flags select setup.
// primitive must be one of ImmediatePrimitive with a sufficient count. vertices
// must match the resulting stride. No buffer is copied/retained and no status is
// returned. Unknown formats keep previous texture equations: caller must pair
// supported format/mode/cache state, not rely on this function to validate them.
void WGeneric3DR::DrawImmediatePrimitive(unsigned int mode, int count, unsigned int mask, void* vertices, int primitive,
                                         unsigned int flags) {
    RendererDrawDispatch* dispatch = (RendererDrawDispatch*)this;
    // Phase 1: establish the default material and choose the cached/full setup path.
    UIRender_SetDefaultState();
    MaterialWords defaultMaterial = *(MaterialWords*)s_rendererDefaultMaterial;
    Engine_SetMaterial(&defaultMaterial);

    unsigned int drawSettings = (flags | kDefaultDrawFlags) >> kDrawSettingShift & kDrawSettingMask;
    if (drawSettings) {
        if (this->cachedDrawSettings != drawSettings)
            this->cachedDrawSettings = drawSettings;
        unsigned int vertexFormatKey = mask & kCachedVertexFormatMask;
        if (this->cachedVertexFormat != vertexFormatKey) {
            this->cachedVertexFormat = vertexFormatKey;
            Engine_SetFlexibleVertexFormat(vertexFormatKey);
            this->vertexStrideBytes = dispatch->GetVertexStride(this->cachedVertexFormat);
        }
        if (this->overrideTexture) {
            BindOverrideDrawTexture((char*)this);
        } else {
            unsigned int textureIndices = mode & kPackedTextureIndicesMask;
            unsigned int previousTextureIndices = this->boundTextureIndices;
            if (previousTextureIndices != textureIndices) {
                BindChangedTextureStage((char*)this, 0, textureIndices, previousTextureIndices);
                BindChangedTextureStage((char*)this, 1, textureIndices, this->boundTextureIndices);
                this->boundTextureIndices = textureIndices;
            }
        }
    } else {
        // Phase 2: full setup resets fixed-function state before selecting texture equations.
        this->cachedDrawSettings = drawSettings;
        this->drawMode = mode;
        dispatch->ApplyRenderFlags(mode);
        dispatch->SetBlendMode(mode & kPackedBlendModeMask);
        if (this->overrideTexture) {
            BindOverrideDrawTexture((char*)this);
        } else {
            unsigned int textureIndices = mode & kPackedTextureIndicesMask;
            unsigned int previousTextureIndices = this->boundTextureIndices;
            if (previousTextureIndices != textureIndices) {
                BindChangedTextureStage((char*)this, 0, textureIndices, previousTextureIndices);
                BindChangedTextureStage((char*)this, 1, textureIndices, this->boundTextureIndices);
                this->boundTextureIndices = textureIndices;
            }
        }
        ((void (*)(int, int))Engine_SetCachedRenderState)(kLightingState, 0);
        ((void (*)(int, int))Engine_SetCachedRenderState)(kViewMatrixOnlyState, 0);
        TextureStageStatePairs defaultStageStates = *(TextureStageStatePairs*)s_rendererDefaultTextureStageStates;
        Engine_SetTextureStageStateList(0, &defaultStageStates, 6);
        SetTransform(kTextureTransformSlot0, kIdentityMatrix4x4);
        SetTransform(kImmediateWorldTransform, kIdentityMatrix4x4);
        Engine_SetType7Resource(0);
        // Mode bit 18 sets cached bit 30: for the two-texture keys below it
        // selects ADD rather than MULTIPLY for stage 1 COLOR. Alpha stays the
        // stage-0 operation. This is combiner policy, not an extra vertex lane:
        // both GX format submission and stride lookup remove bit 30.
        // Accepted pairs (base -> additive key): 120005->40120005,
        // 120401->40120401, 12F401->4012F401, 12F005->4012F005,
        // F005->4000F005, F001->4000F001. Base 12F001 has no additive case.
        // Any other bit-30 combination misses ApplyDrawFormatTextureOperations
        // and retains OLD combiner state. Supply both stage resources and a
        // matching layout; this API does not validate those preconditions.
        // On the cached path above bit 30 is caller-supplied in mask and the
        // combiner is not reapplied; use it only after compatible full setup.
        unsigned int formatMetadataBit = (mode >> kDrawModeTextureOperationShift) & 1;
        unsigned int formatMetadataMask = mode & kDrawModeAddSecondTextureColor;
        this->cachedVertexFormat =
            (mask | (-formatMetadataBit & kCachedAddSecondTextureColor)) & kCachedVertexFormatMask;
        Engine_SetFlexibleVertexFormat(
            (mask | ((int)((0u - formatMetadataMask) | formatMetadataMask) >> 31 & kCachedAddSecondTextureColor)) &
            kSubmittedVertexFormatMask);
        this->vertexStrideBytes = dispatch->GetVertexStride(this->cachedVertexFormat & ~kCachedAddSecondTextureColor);
        ApplyDrawFormatTextureOperations((char*)this);
    }

    // Phase 3: convert the count and submit caller-owned immediate vertices.
    AdjustImmediatePrimitiveCount(primitive, count);
    DrawPrimitiveUP(primitive, count, vertices, this->vertexStrideBytes);
}
#pragma pop
extern void CubeResourceStorage_dtor(void*, int);

// Query renderer properties; property four returns a pair and property nine writes a float.
// output is borrowed writable storage. IDs 1..8,10,11 write an int, except 4
// writes two ints (texture width,height) and 9 writes a float through the ABI slot.
// 1 tests both flag bits 0x80000/0x10000; 3 returns windowFormatFlags bit 29.
// Producer/consumer inventory (wgeneric3dr_renderer_core.cpp::InstallRendererPropertyDefaults and
// WGeneric3DR::Initialize -> this query, including command 31 below):
// ID    offset   stored type/default       known quantity domain
// 2     4850     int / 0                   legacy integer query word
// 5     4900     int / 4                   legacy integer query word
// 6     4904     int / 0                   legacy integer query word
// 7     4914     int / 0                   legacy integer query word
// 8     4918     int / 0                   legacy integer query word
// 9     491C     float / 0.0                legacy scalar query word
// 10    4930     int / 0                   legacy integer query word
// 11    4938     int / 0                   legacy integer query word
// No downstream interpretation of these eight query results is established in
// src/ or include/. Their units, supported nondefault values and visible effects
// remain unknown; neither a dimension nor a hardware capability is proved by
// the default. Do not tune them based on the adjacent texture-dimension pair.
// Unknown IDs, including 0, leave output untouched.
void WGeneric3DR::GetProperty(unsigned int index, int* output) {
    switch (index) {
    case kPropertyFlags4888BothSet: {
        int bits = this->unk_4888;
        if ((bits & 0x00080000) && (bits & 0x00010000)) {
            *output = 1;
        } else {
            *output = 0;
        }
        break;
    }
    case kPropertyUnresolvedWord4850:
        *output = this->unk_4850;
        break;
    case kPropertyWindowFormatBit29:
        *output = ((unsigned int)this->windowFormatFlags >> 29) & 1;
        break;
    case kPropertyDefaultTextureDimensions:
        *output = this->defaultTextureWidth;
        ((int*)output)[1] = this->defaultTextureHeight;
        break;
    case kPropertyUnresolvedWord4900:
        *output = this->unk_4900;
        break;
    case kPropertyUnresolvedWord4904:
        *output = this->unk_4904;
        break;
    case kPropertyUnresolvedWord4914:
        *output = this->unk_4914;
        break;
    case kPropertyUnresolvedWord4918:
        *output = this->unk_4918;
        break;
    case kPropertyUnresolvedFloat491C:
        *(float*)output = this->unk_491C;
        break;
    case kPropertyUnresolvedWord4930:
        *output = this->unk_4930;
        break;
    case kPropertyUnresolvedWord4938:
        *output = this->unk_4938;
        break;
    case kPropertyNoOutput:
        break;
    }
}

#define W6C_CALL(o, a, b) ((W6C*)(o))->v6C((a), (b))
class W6C {
public:
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
    virtual void v68();
    virtual void v6C(int, int);
};
// Decode the blend-mode bits into source and destination blend factors.
// For additive blending, Cout = Cs*Fs + Cd*Fd (componentwise RGB).
// This selects factors only; blend enable/operation remain separate engine state.
// Unlisted value is still cached but leaves engine factors unchanged. Preserve
// that partial update; the input is a packed RendererBlendMode, not an index.
void WGeneric3DR::SetBlendMode(int value) {
    cachedBlendMode = value;
    switch (value) {
    case kBlendAlphaOver: // Cs*As + Cd*(1-As)
        W6C_CALL(this, kBlendSourceAlpha, kBlendInverseSourceAlpha);
        break;
    case kBlendReplace: // Cs
        W6C_CALL(this, kBlendOne, kBlendZero);
        break;
    case kBlendAlphaAdd: // Cs*As + Cd
        W6C_CALL(this, kBlendSourceAlpha, kBlendOne);
        break;
    case kBlendAdd: // Cs + Cd
        W6C_CALL(this, kBlendOne, kBlendOne);
        break;
    case kBlendMultiply:
    case kBlendMultiplyAlternate: // Cd*Cs
        W6C_CALL(this, kBlendZero, kBlendSourceColor);
        break;
    case kBlendInverseMultiply:
    case kBlendInverseMultiplyAlternate: // Cd*(1-Cs)
        W6C_CALL(this, kBlendZero, kBlendInverseSourceColor);
        break;
    default:
        return;
    }
}

// Return old XOR new; expanded copies preserve MWCC's register assignment.
static inline unsigned int ChangedRenderFlagBits(unsigned int old, unsigned int masked) {
    unsigned int previousCopy = old;
    unsigned int xorFirst = previousCopy ^ masked;
    unsigned int xorFirstCopy = xorFirst;
    unsigned int xorFirstHeld = xorFirstCopy;
    unsigned int currentCopy = masked;
    unsigned int currentHeld = currentCopy;
    unsigned int xorSecond = old ^ currentHeld;
    unsigned int previousHeld = old;
    unsigned int xorSecondHeld = xorSecond;
    unsigned int xorFinal = xorSecondHeld;
    unsigned int currentFinal = masked;
    unsigned int xorThird = old ^ currentFinal;
    unsigned int xorFinalHeld = xorFinal;
    return xorFinalHeld;
}

// Apply changed depth, fog, alpha-reference and texture-sampler flags to the engine cache.
// value is wrapper packed flags. Writes cachedRenderFlags after changes; sends
// depth comparison even when no cached bits differ. Alpha threshold is 128/255
// when enabled (not opacity), otherwise zero. Preserve bit masking and stage IDs.
void WGeneric3DR::ApplyRenderFlags(unsigned int value) {
    unsigned int masked = value & kRenderStateMask;
    // Phase 1: depth comparison is sent on every call, even if flags are unchanged.
    unsigned int maskedValue = masked;
    unsigned int mode = maskedValue & kDepthCompareMask;
    switch (mode) {
    case 0x00200000:
    case 0:
        ((void (*)(int, int))Engine_SetCachedRenderState)(kDepthCompareState, 3);
        break;
    case 0x00100000:
        ((void (*)(int, int))Engine_SetCachedRenderState)(kDepthCompareState, 2);
        break;
    case 0x00300000:
        ((void (*)(int, int))Engine_SetCachedRenderState)(kDepthCompareState, 7);
        break;
    }
    unsigned int old = cachedRenderFlags;
    if (old == masked)
        return;
    // Phase 2: only changed bits update samplers, fog, depth write and alpha test.
    unsigned int diff = ChangedRenderFlagBits(old, masked);
    if (diff & kTextureAddressMode0) {
        if (masked & kTextureAddressMode0) {
            Engine_SetTextureStageState(0, 0x12, 0);
            Engine_SetTextureStageState(0, 0x13, 0);
        } else {
            Engine_SetTextureStageState(0, 0x12, 1);
            Engine_SetTextureStageState(0, 0x13, 1);
        }
    }
    if (diff & kFogEnableFlag)
        ((void (*)(int, int))Engine_SetCachedRenderState)(kFogEnableState, (masked >> 27) & 1);
    if (diff & kTextureAddressMode1) {
        if (masked & kTextureAddressMode1) {
            Engine_SetTextureStageState(0, 0x14, 0);
            Engine_SetTextureStageState(0, 0x15, 0);
        } else {
            Engine_SetTextureStageState(0, 0x14, 1);
            Engine_SetTextureStageState(0, 0x15, 1);
        }
    }
    if (diff & kDisableDepthWrite) {

        ((void (*)(int, int))Engine_SetCachedRenderState)(kDepthWriteState, !(masked & kDisableDepthWrite));
    }
    if (diff & kEnableAlphaTest)
        ((void (*)(int, int))Engine_SetCachedRenderState)(kAlphaReferenceState,
                                                          (masked >> 31) ? kAlphaTestReference : 0);
    cachedRenderFlags = masked;
}

// Return the viewport-width word through the existing pointer-shaped ABI.
// Initialize stores kDefaultDisplayWidth at +0x14; this is a scalar, with no
// allocation ownership. Do not dereference it (shared field type needs correction).
void* WGeneric3DR::vfunc_13() {
    return viewportWidth;
}

extern "C" {

// Initialize this unit's stored full-turn angle constant.
void Renderer_InitTwoPi() {
    s_twoPi_80088DD4 = kFloat_6_2831855;
}

} // extern "C"

// Return the resource handle for an in-range texture slot; negative indices retain the original contract.
// Caller must supply 0<=index<2048; only upper bound is checked. Returns a
// borrowed handle or null, with no reference increment. Preserve index arithmetic.
void* WGeneric3DR::GetTextureResourceHandle(int index) {
    if (index >= kTextureSlotCount) {
        return 0;
    }
    return ((RendererTextureTable*)this)->slots[index].resourceHandle;
}

// Default virtual hook with no work.
void WGeneric3DR::vfunc_5() {}

// Return the cached palette object for an in-range texture slot.
// Same required nonnegative index/lifetime as GetTextureResourceHandle; an
// unloaded slot may yield null. No resource allocation or ownership transfer.
void* WGeneric3DR::GetTexturePaletteObject(int index) {
    if (index >= kTextureSlotCount) {
        return 0;
    }
    return ((RendererTextureTable*)this)->slots[index].paletteObject;
}

// Set scalar GX source/destination factors through the engine dispatcher; no ownership transfers.
void WGeneric3DR::SetBlendState(int sourceFactor, int destinationFactor) {
    ((void (*)(int, int))Engine_SetCachedRenderState)(kSourceBlendState, sourceFactor);
    ((void (*)(int, int))Engine_SetCachedRenderState)(kDestinationBlendState, destinationFactor);
}

// Load one shared texture and cache its dimensions, format, and binding handles.
// index must be 0..2047 (no range check); arg is the borrowed loader input.
// Writes sharedTexture first, then other fields only if load/description succeed.
// A failed load clears sharedTexture but leaves old cached metadata/handles;
// missing description leaves them too. No status or rollback is supplied.
// No old-resource release occurs here: slot replacement/lifetime must be managed
// by its owning renderer/resource system. The loader returns an acquired shared
// handle (gsr_resource_bitmap_and_heap.cpp); its release API is ISharedResource_Release (shared_texture_resource.cpp).
// Do not release the cached stage-resource or palette handle independently. Preserve this
// partial-write ordering and the loader's blocking readiness poll (no timeout).
void WGeneric3DR::LoadTextureSlot(void* arg, int index) {
    int offset = index * (int)sizeof(RendererTextureSlot);
    if (void* resource = (((RendererTextureTable*)((char*)this + offset))->slots[0].sharedTexture =
                              ((CGsrResource*)arg)->LoadSharedTexture())) {
        void* info = ISharedResource_GetTextureDescription(resource);
        if (info) {
            *(int*)((char*)&((RendererTextureTable*)this)->slots[0].width + offset) = *(int*)info;
            ((RendererTextureTable*)((char*)this + offset))->slots[0].height = *((int*)info + 1);
            ((RendererTextureTable*)((char*)this + offset))->slots[0].format = *((int*)info + 3);
            ((RendererTextureTable*)((char*)this + offset))->slots[0].resourceHandle =
                ISharedResource_GetLocked(((RendererTextureTable*)((char*)this + offset))->slots[0].sharedTexture);
            ((RendererTextureTable*)((char*)this + offset))->slots[0].paletteObject = ISharedResource_GetPaletteObject(
                ((RendererTextureTable*)((char*)this + offset))->slots[0].sharedTexture);
            ((RendererTextureTable*)((char*)this + offset))->slots[0].bindingState = 0;
            ((RendererTextureTable*)((char*)this + offset))->slots[0].uploadState = 0;
        }
    }
}

struct Caller36 {
    virtual void m0();
    virtual void m1();
    virtual void m2();
    virtual void m3();
    virtual void m4();
    virtual void m5();
    virtual void m6();
    virtual void m7();
    virtual void m8();
    virtual void m9();
    virtual void m10();
    virtual void m11();
    virtual void m12();
    virtual void m13();
    virtual void m14();
    virtual void m15();
    virtual void m16();
    virtual void m17();
    virtual void m18();
    virtual void m19();
    virtual void m20();
    virtual void m21();
    virtual void m22();
    virtual void m23();
    virtual void m24();
    virtual void m25();
    virtual void m26();
    virtual void m27();
    virtual void m28();
    virtual void m29();
    virtual void m30();
    virtual void m31();
    virtual void m32();
    virtual void target(int);
};
struct RefCount7 {
    CW_BITS2(unsigned int hi : 16, unsigned int lo : 16)
};
// Dispatch renderer commands for resource release, bounded settings, counts, and property queries.
// Command map (all paths return null; this is not a failure indicator):
// 9 sets presentationTarget; 11 forwards value to virtual target; 14 releases the
// retained resource unless value==1; 21 clamps an unresolved setting to [0,5];
// 22 stores boolean; 23 stores max(value,0); 24 accepts only [1,5]; 25 accepts
// [0,5] and uploads sampler state 0x16. 26 and 28 treat value AS int* and write
// cached field 0x4844 / loaded-texture count. 31 treats value as a property ID and
// uses output (see GetProperty for buffer types). 32,value=0 uploads state 0x10=0
// only when initialized. Other IDs are no-ops. Pointer-as-int slots use the Wii
// 32-bit ABI; output/value pointers remain caller-owned and must be writable.
// Compatibility-storage contract for commands 21..24:
// command  offset   constructor default   accepted input -> stored value
// 21       1AA38    0                     any int -> clamp [0,5]
// 22       1AA3C    0                     any int -> byte(value != 0)
// 23       1AA40    1                     any int -> max(value,0)
// 24       1AA44    2                     [1,5] -> value; otherwise unchanged
// The named-field source references are constructor writes and these writes;
// no reader/upload of these four settings is established. Unlike command 25,
// they do not send Engine_SetTextureStageState here. Changing them has no
// source-demonstrated visible effect; feature names/units remain unproved.
void* WGeneric3DR::DispatchCommand(int command, int value, int* output) {
    switch (command) {
    case kRendererSetPresentationTarget: // Cache the caller's presentation-target value.
        this->presentationTarget = value;
        break;
    case kRendererNoOp5:
        return 0;
    case kRendererForwardTargetValue: { // Forward the value to the target virtual interface.
        ((Caller36*)this)->target(value);
        break;
    }
    case kRendererReleaseSharedResource: { // Release the retained resource unless value requests retention.
        if (value == 1)
            return 0;
        RefCount7* ref = ((RefCount7*)this->releasableSharedResource);
        if (ref) {
            --ref->hi;
            if (ref->hi == 0)
                CubeResourceStorage_dtor(ref, 1);
            this->releasableSharedResource = 0;
        }
        break;
    }
    case kRendererNoOp17:
        return 0;
    case kRendererNoOp18:
        return 0;
    case kRendererNoOp19:
        return 0;
    case kRendererSetClampedSetting21: // Clamp the unresolved setting to [0,5].
        this->unk_1AA38 = value <= 0 ? 0 : value >= 5 ? 5 : value;
        break;
    case kRendererSetBooleanSetting22: // Normalize the unresolved setting to zero/one.
        this->unk_1AA3C = value != 0;
        break;
    case kRendererSetNonnegativeSetting23: { // Clamp the unresolved setting to a nonnegative value.
        int t = -value;
        t = value | ~t;
        t >>= 31;
        this->unk_1AA40 = value & ~t;
        break;
    }
    case kRendererSetBoundedSetting24: // Accept only values [1,5] for the unresolved setting.
        if (value >= 1 && value <= 5)
            this->unk_1AA44 = value;
        break;
    case kRendererSetSamplerSetting25: // Cache [0,5] and upload texture-stage state 0x16.
        if (value >= 0 && value <= 5) {
            this->unk_1AA48 = value;
            Engine_SetTextureStageState(0, 0x16, value);
        }
        break;
    case kRendererQueryWord4844: // Write one word through the pointer-shaped value argument.
        *(int*)value = this->submissionState;
        break;
    case kRendererNoOp27:
        return 0;
    case kRendererQueryLoadedTextureCount: { // Count slots with nonnull cached resourceHandle.
        // A failed replacement can clear sharedTexture while leaving resourceHandle
        // nonnull; such a slot still counts (see WGeneric3DR_LoadTextureSlot).
        int count = 0;
        char* entry = (char*)this;
        int idx = 0;
        for (; idx < kTextureSlotCount; ++idx) {
            if (AsWGeneric3DR(entry)->textureResource)
                count++;
            entry += 0x2c;
        }
        (void)idx;
        *(int*)value = count;
        break;
    }
    case kRendererQueryProperty: // Write the selected property to caller-owned output.
        GetProperty(value, output);
        break;
    case kRendererApplyInitializedSetting32: // Upload subcommand zero only after initialization.
        if (this->initialized) {
            switch (value) {
            case 0:
                ((void (*)(int, int))Engine_SetCachedRenderState)(0x10, 0);
                break;
            }
        }
        break;
    case kRendererNoOp6:
    case kRendererNoOp7:
    case kRendererNoOp8:
    case kRendererNoOp10:
    case kRendererNoOp12:
    case kRendererNoOp13:
    case kRendererNoOp15:
    case kRendererNoOp16:
    case kRendererNoOp20:
    case kRendererNoOp29:
    case kRendererNoOp30:
    case kRendererNoOp33:
        break;
    }
    return 0;
}

// Return the renderer value also exposed by command twenty-six.
int WGeneric3DR::vfunc_19() {
    return submissionState;
}

// Return the renderer integer exposed by virtual property slot fourteen.
// This is cached height in pixels, with no allocation or state mutation.
int WGeneric3DR::vfunc_14() {
    return viewportHeight;
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046203C", _ctors_u800879D0_0, Renderer_InitTwoPi);
#pragma pop
