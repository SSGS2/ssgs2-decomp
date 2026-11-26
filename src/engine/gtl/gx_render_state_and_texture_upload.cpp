// GX render state and texture upload: the texture mip-chain upload and the functions that
// apply cached render state to GX, such as depth, blend, alpha test, fog, vertex input and TEV.
//
// Each Render_Apply* function writes one group of cached state to the GX hardware. The texture
// upload copies consecutive mip levels into a new texture.

#include <decomp/offset_of.h>
// GX renderer: image mip uploads, texture/TEV stage allocation and cached draw state.
// Texture mod lookup: Texture_CreateAndUploadMipChain receives dimensions in
// UploadImageDescription, translates its engine format for platform 7 (Wii),
// then copies consecutive mip bytes. It consumes decoded images; GSR parsing,
// palette upload and resource destruction are in the image/texture APIs, not here.
// Render_ApplyTevState is the companion consumer: C8/RGBA8-palette and split
// compressed formats require extra color/alpha stages. Change format and stage
// policy together; an HD replacement pack need not change this native upload path.
// Resolution/filter lookup: BuildVerticalFilterWeights programs seven vertical
// taps totaling 64; ApplyPointSize converts a normalized value to a GX byte.
// No viewport, scissor, aspect-ratio or presentation interval is set in this unit.
// Scalar pool: the data section plus shared kFloat_1_Renderer/.5_Light.
// RenderState80422F10.h owns the ABI schema; local stage/color views add fields
// at documented offsets in that same state. All Apply calls require initialized
// g_engineState on the render thread, write GX/cache state, return no status
// unless specified, and borrow texture/light/descriptor/callback storage.
// Integerized addresses are 32-bit Wii ABI values; they are not serialized offsets.
// The optimization pragmas, __alloca(0), volatile spills and access order retain
// the original call/register/stack shape; do not remove them as dead work.
#define FIELD_OFFSET(type, field) ((int)DECOMP_OFFSETOF(type, field))
#include "classes/RenderState80422F10.h"

#include "sdk_protos.h"

enum {
    kVertexColorMask = 0xF000,
    kTextureStageCount = 8,
    kLightSlotCount = 8,
    kColorWriteRgbMask = 0x00010101,
    kColorWriteAlphaShift = 24,
    kVertexInputDirect = 1,
    kVertexInputIndex16 = 3,
    kCompareAlways = 7,
    kAlphaByteMask = 255,
    kTevOperationDisabled = 0x10,
    kFilterWeightTotal = 64,
    kVertexTexCoordCountShift = 16,
    kVertexTexCoordCountMask = 0xF,
    kVertexTexCoordCountBits = 0x000F0000,
    kTextureStageBytes = 0x64,
    kTextureDescriptionWords = 11,
    kTextureMipLevelCountForSizeQuery = 1
};
enum RenderBlendOperation {
    kBlendOperationAdd = 1,
    kBlendOperationSubtract = 3
};
enum RenderStageSentinel {
    kNoLogicOperation = 0x80000000u,
    kNoGeneratedTexCoord = 0x80000000u
};
// The engine format encodes a family in the high bits and an ID in the low
// bits. Keep the original unsigned bias tests (addition wraps modulo 2^32).
enum TextureFormatEncoding {
    kPlatformFormatBias = 0x80000000u,
    kSplitCompressedFormatBias = 0x40000000u,
    kIndexed8FormatCode = 9,
    kRgba8PaletteFormatCode = 6,
    kSplitCompressedFormat30 = 0x30,
    kSplitCompressedFormat32 = 0x32,
    kImagePlatformWii = 7
};
enum TevEffectSlot {
    kIndirectBumpEffect = 1,
    kProjectionTintEffect = 4,
    kFourStageEffect = 6
};

// Borrowed four-word image metadata; the format occupies a pointer-sized word
// in the image API, while dimensions are texels and mipCount counts stored levels.
struct UploadImageDescription {
    void* format;
    unsigned int width, height, mipCount;
};
// Texture query writes nine words into an eleven-word ABI scratch area here;
// only the first word (encoded format) is consumed in this unit.
struct TextureFormatQuery {
    unsigned int format;
    unsigned int remainingWords[kTextureDescriptionWords - 1];
};
struct PaletteFormatQuery {
    unsigned int format;
    unsigned int remainingWords[4];
};

// Only fields consumed here are named; all other stage state stays opaque.
struct TextureStageView {
    char reserved00[0xC];
    unsigned int colorOperation;
    char reserved10[8];
    unsigned int alphaOperation;
    char reserved1C[0x18];
    unsigned int texCoordSource;
    char reserved38[0x2C];
};
struct RenderStageOwnerView {
    char reserved000[0x178];
    TextureStageView stages[8];
    char reserved498[0x1064];
    int combinerDirty[8][2];
    char reserved153C[0x14C];
    int specialEffects[7];
};
struct RenderMaterialColorView {
    char reserved000[0xFC];
    unsigned char ambient[4];
    char reserved100[0x20FC];
    unsigned char material[4];
    char reserved2200[4];
    unsigned char emissive[4];
};
// Cast-only ABI overlays borrow the same state; no allocation occurs. Retain
// macros here: inline calls are emitted in the optimization-level-zero regions.
#define AsRenderStages(p) ((RenderStageOwnerView*)(p))
#define AsRenderColors(p) ((RenderMaterialColorView*)(p))

extern "C" {

extern void* g_engineState;
extern const float kFloat_1_Renderer;
extern const float kFloat_0_5_Light;
extern const float kFloat_64_VerticalFilterWeightTotal;
extern const float kFloat_0_25_Renderer;
extern const float kFloat_3_Renderer;
extern const float kFloat_255_RenderPointSize;
void TextureStage_ConfigureTevColorCombiner(void*, void*);
void Engine_ConfigureTextureStageAlphaCombiner(void*, void*);
void GXSetAlphaUpdate(int);
void GXSetColorUpdate(int);
void GXSetZScaleOffset(float, float);
void GXSetPointSize(unsigned int, int);
void GXSetZCompLoc(int);
int VertexDeclaration_UsesSpecialTexCoordMapping(unsigned int);
void VertexDeclaration_GetElements(unsigned int, void*, unsigned int*);
void GXSetFog(void*, unsigned char*, float, float, float, float);
void VertexInput_CopyDescriptorForDeclaration(void*, unsigned int);
void VertexInput_FindAndCopyFormatDescriptor(void*, unsigned int);
void GXClearVtxDesc();
int gtl_Image_GetImageData(void*, void*, int);
unsigned int gtl_ImageFormat_ToPlatformFormat(void*, int);
void Texture_Create2D(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int,
                      unsigned int);
void Texture_LockLevel(unsigned int, unsigned int, void*, int, int);
unsigned int gtl_Image_GetMipChainSize(unsigned int, unsigned int, int, unsigned int, int);
void Texture_SplitCompressedColorAndAlphaMipData(void*, void*, unsigned int, unsigned int, unsigned int, unsigned int);
void Texture_UnlockLevel(unsigned int, unsigned int);
void GXSetBlendMode(GXBlendMode, GXBlendFactor, GXBlendFactor, GXLogicOp);

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Create a texture and upload successive image levels, delegating special tiled formats to conversion.
// arg/imageDescription are required borrowed inputs; unused is an unused ABI
// argument. out is required writable handle storage owned by the caller, to be
// released through the texture resource API. Image lookup failure clears *out
// and returns 0; after lookup this returns 1 even if creation/lock APIs fail
// (their status is not checked here). Supply valid formats/storage/dimensions.
// width/height are texels; mipCount includes level 0. Right shifts have NO
// minimum-one clamp: the caller must provide dimensions valid at every level.
// Preserve source-size calculation in the ORIGINAL image format even though
// destination creation uses platformFormat, and advance source by that byte size.
// Split conversion receives the full mipCount and exits without this path's
// ordinary unlock. Keep that delegation boundary; no cleanup is added on failure.
// Supported input-format words -> Wii platform words, from platform-7 switch in
// gtl_ImageFormat_ToPlatformFormat (gtl_image_format_dispatch.cpp); format is a NUMERIC word stored
// in a pointer-shaped ABI slot, not an address to dereference:
// 13/76 -> 00000006, 77 -> 40000006, 14 -> 80000006,
// 30 -> 80000009 (C8), 80 -> 00000004 (RGB565), 15 -> C0000006,
// 46 -> 8000000E, 98 -> 0000000E (CMPR), 48 -> C0000030,
// 50 -> C0000032. Other words (including -1) map to -1, which is NOT checked.
// High-bit variants carry engine storage policies; do not strip them to GX IDs.
// Extents: Texture_Create2D (gx_renderer_state_and_resources.cpp) rejects base width/height<4 without
// writing *out; this caller ignores that result, so require both>=4 beforehand.
// Texture_BuildMipLayout (gtl_texture_vertex_formats.cpp) has EIGHT level records: require 1<=mipCount<=8.
// It clamps destination dimensions to 1, whereas THIS upload loop merely shifts;
// require width>>(mipCount-1) and height>>(mipCount-1) both >=1. Zero mipCount
// uploads nothing and advertises the constructor default; it is not a full-chain
// request here. The full decoded source must cover the sum of original-format
// GetMipChainSize(w>>i,h>>i,1,format,7) bytes, in consecutive mip order.
// For split formats C0000030/C0000032 the converter (gtl_graphics_texture_buffer_resources.cpp) uses
// (w>>2)*(h>>2) 4x4 blocks per mip, without rounding up: each shifted dimension
// must stay >=4 and divisible by 4 at every requested mip. Its output places
// ALL color mips before the decoded alpha plane. This path never resizes input
// or repairs missing/tiny mips. Upper hardware extent/alignment policies belong
// to GX/resource creation and are not validated by this API; do not infer an
// arbitrary-size replacement guarantee from the integer parameter widths.
// After success the caller owns *out's initial graphics reference and releases
// it with GraphicsResource_Release. Existing *out must not own a prior texture.
int Texture_CreateAndUploadMipChain(void* arg, void* unused, unsigned int* out,
                                    UploadImageDescription* imageDescription) {
    // Phase 1: obtain borrowed image bytes and create the output texture (0 on image lookup failure).
    __alloca(0);
    unsigned int header[3];
    unsigned int i;
    unsigned int platformFormat;
    unsigned int dataPtr;
    unsigned int width;
    unsigned int height;
    unsigned int size;
    if (!gtl_Image_GetImageData(header, arg, 0)) {
        *out = 0;
        return 0;
    }
    platformFormat = gtl_ImageFormat_ToPlatformFormat(imageDescription->format, kImagePlatformWii);
    Texture_Create2D(imageDescription->width, imageDescription->height, imageDescription->mipCount, 0, platformFormat,
                     0, (unsigned int)out, 0);
    dataPtr = header[0];
    width = imageDescription->width;
    height = imageDescription->height;
    // Phase 2: upload each mip level; split compressed formats delegate the remaining chain and exit.
    for (i = 0; i < imageDescription->mipCount; ++i) {
        Texture_LockLevel(*out, i, &header[1], 0, 0);
        size = gtl_Image_GetMipChainSize(width, height, kTextureMipLevelCountForSizeQuery,
                                         (unsigned int)imageDescription->format, kImagePlatformWii);
        if (platformFormat + kSplitCompressedFormatBias == kSplitCompressedFormat30 ||
            platformFormat + kSplitCompressedFormatBias == kSplitCompressedFormat32) {
            Texture_SplitCompressedColorAndAlphaMipData((void*)header[2], (void*)dataPtr, width, height, platformFormat,
                                                        imageDescription->mipCount);
            break;
        }
        Memory_Copy((void*)header[2], (void*)dataPtr, size);
        Texture_UnlockLevel(*out, i);
        dataPtr += size;
        width >>= 1;
        height >>= 1;
    }
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Build diffuse/specular GX light masks and select vertex or register material color sources.
// Eight cached entries do not imply eight usable specular pairs: stride two
// consumes two physical GX light slots per active entry. Caller configuration
// must fit GX's eight light slots; this function does not validate the mask.
// GX domains (GXTypes.h): color source 0=GX_SRC_REG, 1=GX_SRC_VTX;
// diffuse 0=NONE (no normal cosine), 1=SIGN (signed cosine), 2=CLAMP
// (nonnegative cosine). Attenuation 0=SPEC (specular), 1=SPOT (spot/distance),
// 2=NONE. Diffuse configures COLOR0 and ALPHA0 separately with CLAMP/SPOT;
// the optional specular path configures COLOR1 with NONE/SPEC. These are GX
// lighting functions, not game difficulty or brightness levels. Absent vertex
// color forces both color sources to register; disabled lighting with vertex
// color instead selects vertex material. Masks name physical GX slots 0..7:
// specular enabled doubles stride, so only logical entries 0..3 may be active.
// Require specularLightingEnabled=>lightingEnabled for an active-light setup:
// stride is assigned only in the lighting-enabled branch. There is no guard
// against that unsupported combination or against physical-mask overflow.
void Render_ApplyLightingChannels() {
    // Phase 1: choose vertex/register sources and collect active diffuse light slots.
    __alloca(0);
    int materialSource;
    int lightIndex;
    int specularIndex;
    int lightingEnabled;
    int ambientSource;
    int diffuseLightMask;
    int lightSlotStride;
    int specularLightMask;
    lightingEnabled = 1;
    diffuseLightMask = 0;
    ambientSource = AsRenderState80422F10(g_engineState)->ambientColorSource;
    materialSource = AsRenderState80422F10(g_engineState)->materialColorSource;
    if (AsRenderState80422F10(g_engineState)->lightingEnabled == 0) {
        lightingEnabled = 0;
        if (AsRenderState80422F10(g_engineState)->flexibleVertexFormat & kVertexColorMask)
            materialSource = 1;
        else
            materialSource = 0;
    } else {
        lightSlotStride = 1;
        if (AsRenderState80422F10(g_engineState)->specularLightingEnabled != 0)
            lightSlotStride = 2;
        for (lightIndex = 0; lightIndex < kLightSlotCount; ++lightIndex) {
            if ((*(int*)&AsRenderState80422F10(g_engineState)->lights[lightIndex]) != 0) {
                diffuseLightMask |= 1 << (lightIndex * lightSlotStride);
            }
        }
    }
    if (!(AsRenderState80422F10(g_engineState)->flexibleVertexFormat & kVertexColorMask)) {
        ambientSource = 0;
        materialSource = 0;
    }
    GXSetChanCtrl(GX_COLOR0, (GXBool)lightingEnabled, (GXColorSrc)ambientSource, (GXColorSrc)materialSource,
                  (GXLightID)diffuseLightMask, GX_DF_CLAMP, GX_AF_SPOT);
    GXSetChanCtrl(GX_ALPHA0, (GXBool)lightingEnabled, (GXColorSrc)ambientSource, (GXColorSrc)materialSource,
                  (GXLightID)diffuseLightMask, GX_DF_CLAMP, GX_AF_SPOT);
    // Phase 2: specular lights use alternating slots when enabled, beside the diffuse slots.
    if (AsRenderState80422F10(g_engineState)->specularLightingEnabled != 0) {
        specularLightMask = 0;
        for (specularIndex = 0; specularIndex < kLightSlotCount; ++specularIndex) {
            if ((*(int*)&AsRenderState80422F10(g_engineState)->lights[specularIndex]) != 0) {
                specularLightMask |= 2 << (specularIndex * lightSlotStride);
            }
        }
        GXSetChanCtrl(GX_COLOR1, (GXBool)lightingEnabled, (GXColorSrc)ambientSource, (GXColorSrc)materialSource,
                      (GXLightID)specularLightMask, GX_DF_NONE, GX_AF_SPEC);
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Distribute a total weight of 64 among seven taps, preserving integer rounding.
// out requires seven bytes. filterBias is dimensionless: center fraction=(1+b)/2,
// outer=64-trunc(64*centerFraction); increasing b weights the middle three taps.
// [-1,1] keeps the arithmetic's two weight totals nonnegative; no caller-range
// validation or saturation occurs. Preserve truncation, remainder distribution
// and tap order; the resulting bytes feed the EFB-to-display copy filter.
void Render_BuildVerticalFilterWeights(unsigned char* out, float filterBias) {
    struct {
        unsigned char center[3];
        unsigned char outer[4];
    } weights;
    float centerFraction = (kFloat_1_Renderer + filterBias) * kFloat_0_5_Light;
    int centerWeight = (int)(kFloat_64_VerticalFilterWeightTotal * centerFraction);
    int outerWeight = kFilterWeightTotal - centerWeight;
    for (int i = 0; i < 4; ++i) {
        if (i < outerWeight - (outerWeight / 4) * 4)
            weights.outer[i] = (unsigned char)((float)outerWeight * kFloat_0_25_Renderer + kFloat_1_Renderer);
        else
            weights.outer[i] = (unsigned char)((float)outerWeight * kFloat_0_25_Renderer);
    }
    for (int i = 0; i < 3; ++i) {
        register int three = 3;
        if (i < centerWeight - (centerWeight / three) * three)
            weights.center[i] = (unsigned char)(kFloat_1_Renderer + (float)centerWeight / kFloat_3_Renderer);
        else
            weights.center[i] = (unsigned char)((float)centerWeight / kFloat_3_Renderer);
    }
    out[0] = weights.outer[0];
    out[1] = weights.outer[2];
    out[2] = weights.center[0];
    out[3] = weights.center[1];
    out[4] = weights.center[2];
    out[5] = weights.outer[1];
    out[6] = weights.outer[3];
}
#pragma pop
struct VertexDescList {
    unsigned int* recs; // 0x0
    char reserved04[0x10];
    unsigned int count; // 0x14
};
extern VertexDescList s_vertexFormatDescriptor;
extern unsigned int g_useIndexedVertexInput;

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Test for the declaration element triple required by the extra texture-coordinate generators.
// Together with gx_renderer_state_and_resources.cpp's texgen setup, this is the admission test for the
// alternate s_specialTexCoordSources/s_specialTexCoordMatrixIds mapping; it
// also adds three generators in VertexInput_CountTextureCoordinateGenerators.
// Code 3 is a normal element (gx_renderer_state_and_efb_copy.cpp's declaration filter). Codes 6/7 retain
// their numeric identity until their attribute meaning is independently established.
// declaration is a borrowed declaration handle with at most sixteen 12-byte elements;
// returns a Boolean, writes no caller storage. Element byte +9 is a type code.
int VertexDeclaration_UsesSpecialTexCoordMapping(unsigned int declaration) {
    unsigned int count;
    unsigned char desc[16][12];
    VertexDeclaration_GetElements(declaration, desc, (unsigned int*)&count);
    int has3 = 0;
    int has6 = 0;
    int has7 = 0;
    for (unsigned int i = 0; i < count; ++i) {
        switch (desc[i][9]) {
        case 3:
            has3 = 1;
            break;
        case 6:
            has6 = 1;
            break;
        case 7:
            has7 = 1;
            break;
        }
    }
    return has3 && has6 && has7;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Apply fog type, RGBA color, distance range and projection planes from cached render state.
// Editing map: Engine_SetCachedRenderState (gx_renderer_state_and_efb_copy.cpp) IDs 1(enable), 21(type),
// 22(color), 23(start), 24(end) dirty this group. Float states travel as raw
// binary32 bits, not integer distances. fogType's void* is an encoded GX enum,
// never an address. Admitted values (revolution/GX/GXTypes.h): 0 none;
// perspective 2 linear, 4 exp, 5 exp2, 6 reverse-exp, 7 reverse-exp2;
// orthographic 10,12,13,14,15 respectively (bit3 selects orthographic).
// Disabled fog forces type0 but still forwards color/distances/planes. Unlisted
// values are not validated and must not be introduced as custom curve IDs.
// start/end are view-distance quantities in the SAME units as the active
// projection's near/far planes, not normalized clip Z, screen pixels or yards.
// For an ordinary noninverted ramp keep finite start<end and finite near<far;
// the projection determines plane signs (do not impose perspective's positive
// near-distance convention on an orthographic matrix). start/end need not equal
// the clipping planes. This wrapper supplies no clamps or inverted/nonfinite/
// degenerate-range policy; exact SDK behavior outside the ordinary ramp is not
// established by these callers. Keep the projection producer's plane contract.
// Projection
// producer SetTransform(state1) in gx_renderer_state_and_efb_copy.cpp recovers +19F0/+19F4
// from matrix[10,11,14,15]; its recognized perspective/negative-Z orthographic
// forms must be preserved. Keep fog type's projection bit and those planes in
// agreement; this call does not infer type from the matrix or convert spaces.
// RGBA is forwarded as four bytes; no blending/draw-state restoration occurs.
void Render_ApplyFogState() {
    void* renderState = g_engineState;
    int fogEnabled = AsRenderState80422F10(renderState)->fogEnabled;
    void* fogType = fogEnabled ? AsRenderState80422F10(renderState)->fogType : 0;
    unsigned char color[8];
    unsigned char r = AsRenderState80422F10(renderState)->fogRed;
    unsigned char g = AsRenderState80422F10(renderState)->fogGreen;
    unsigned char b = AsRenderState80422F10(renderState)->fogBlue;
    unsigned char a = AsRenderState80422F10(renderState)->fogAlpha;
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = a;
    color[4] = r;
    color[5] = g;
    color[6] = b;
    color[7] = a;
    float startDistance = AsRenderState80422F10(renderState)->fogStart;
    float endDistance = AsRenderState80422F10(renderState)->fogEnd;
    float nearPlane = AsRenderState80422F10(renderState)->fogNearPlane;
    float farPlane = AsRenderState80422F10(renderState)->fogFarPlane;
    GXSetFog(fogType, color, startDistance, endDistance, nearPlane, farPlane);
}
#pragma pop

// Select direct/indexed vertex descriptors and program the corresponding GX attribute formats.
void Render_ApplyVertexInputState() {
    if (AsRenderState80422F10(g_engineState)->useVertexDeclaration)
        VertexInput_CopyDescriptorForDeclaration(&s_vertexFormatDescriptor,
                                                 AsRenderState80422F10(g_engineState)->vertexDeclaration);
    else
        VertexInput_FindAndCopyFormatDescriptor(
            &s_vertexFormatDescriptor, (*(unsigned int*)&AsRenderState80422F10(g_engineState)->flexibleVertexFormat));
    GXClearVtxDesc();
    int mode = g_useIndexedVertexInput ? kVertexInputIndex16 : kVertexInputDirect;
    unsigned int i = 0;
    while (i < s_vertexFormatDescriptor.count) {
        unsigned int* rec = s_vertexFormatDescriptor.recs + i * 4;
        unsigned int attr = rec[0];
        if (attr || mode != kVertexInputIndex16)
            GXSetVtxDesc((GXAttr)attr, (GXAttrType)mode);
        ++i;
    }
    i = 0;
    while (i < s_vertexFormatDescriptor.count) {
        unsigned int* rec = s_vertexFormatDescriptor.recs + i * 4;
        if (rec[0])
            GXSetVtxAttrFmt((GXVtxFmt)0, (GXAttr)rec[0], (GXCompCnt)rec[1], (GXCompType)rec[2],
                            *(unsigned char*)((char*)rec + 12));
        ++i;
    }
}

#pragma push
#pragma optimization_level 0
// Apply cached depth-test, comparison and depth-write settings.
void Render_ApplyDepthState() {
    struct Access {
        // Read whether the cached state enables depth comparison.
        static unsigned int compare() {
            void* p = g_engineState;
            return AsRenderState80422F10(p)->depthTest;
        }
        // Read whether the cached state enables depth writes.
        static unsigned int update() {
            void* p = g_engineState;
            return AsRenderState80422F10(p)->depthWrite;
        }
        // Read the GX depth-comparison function code.
        static int func() {
            void* p = g_engineState;
            return AsRenderState80422F10(p)->depthCompare;
        }
    };
    GXSetZMode((GXBool)Access::compare(), (GXCompare)Access::func(), (GXBool)Access::update());
}
#pragma pop

// Enable RGB writes when any cached RGB channel mask is set.
void Render_ApplyColorWriteState() {
    unsigned int flags = AsRenderState80422F10(g_engineState)->colorWriteMask;
    unsigned int masked = flags & kColorWriteRgbMask;
    int nonzero = (masked != 0);
    volatile int saved = nonzero;
    GXSetColorUpdate(nonzero);
    (void)saved;
}

// Apply depth scale/bias while preserving the original volatile reads and extra float operation.
void Render_ApplyDepthScaleOffset() {
    void* volatile a = g_engineState;
    void* volatile b = g_engineState;
    float first = AsRenderState80422F10(a)->depthScale;
    float second = AsRenderState80422F10(b)->depthOffset;
    GXSetZScaleOffset(first, second);
    volatile float keep = first + second;
    (void)keep;
}

// Configure texture-stage state using the current texture-handle array.
void Render_ConfigureTextureStagesFromBoundTextures(void* p) {
    void* volatile saved = p;
    TextureStage_ConfigureTevColorCombiner(AsRenderState80422F10(g_engineState)->textures,
                                           p); // 0x80423998: texture handle array
}

// Configure the alpha combiner using the current texture-handle array.
void Engine_ApplyTextureStageAlphaCombiner(void* p) {
    void* volatile saved = p;
    Engine_ConfigureTextureStageAlphaCombiner(AsRenderState80422F10(g_engineState)->textures,
                                              p); // 0x804239C8: texture handle array
}

// Enable alpha writes from the high-byte write-mask bit.
void Render_ApplyAlphaWriteState() {
    void* volatile keep = g_engineState;
    unsigned int flags = AsRenderState80422F10(g_engineState)->colorWriteMask;
    GXSetAlphaUpdate((flags >> kColorWriteAlphaShift) & 1);
}

#pragma push
#pragma dont_inline on
#pragma optimization_level 0
// Multiply two RGBA byte colors componentwise, dividing by 255.
// Each pointer requires four bytes. Exact in-place dst==firstColor or dst==secondColor is valid;
// partial overlap changes later inputs and is not a supported copy operation.
void Color_MultiplyRGBA8(unsigned char* dst, const unsigned char* firstColor, const unsigned char* secondColor) {
    int d = 255;
    dst[0] = (unsigned char)(((int)firstColor[0] * (int)secondColor[0]) / d);
    dst[1] = (unsigned char)(((int)firstColor[1] * (int)secondColor[1]) / d);
    dst[2] = (unsigned char)(((int)firstColor[2] * (int)secondColor[2]) / d);
    dst[3] = (unsigned char)(((int)firstColor[3] * (int)secondColor[3]) / d);
}
#pragma pop

// Convert normalized point size to GX's byte encoding (sixths of a pixel); clamp only above 1.
// Writer: Engine_SetCachedRenderState ID35 (gx_renderer_state_and_efb_copy.cpp) stores the raw binary32
// word and dirties this group. Supported mod input is finite and >=0: [0,1]
// truncates 255*x to a byte (pixel size=byte/6), >1 saturates to255 (42.5 pixels).
// Zero is forwarded as zero; no positive minimum is imposed. Negative input has
// no lower clamp and is outside this contract: do not depend on the compiler's
// float-to-byte result. +infinity happens to take the upper branch; NaN/-infinity
// reach an invalid conversion. None is an admitted size. The API returns no
// failure/status and GXSetPointSize uses texture-offset selector0 every time.
void Render_ApplyPointSize() {
    void* p = g_engineState;
    float normalizedPointSize = AsRenderState80422F10(p)->normalizedPointSize;
    unsigned char pointSize;
    if (normalizedPointSize > kFloat_1_Renderer) {
        pointSize = 0xff;
    } else {
        float encodedPointSize = kFloat_255_RenderPointSize * normalizedPointSize;
        pointSize = (unsigned char)encodedPointSize;
    }
    GXSetPointSize(pointSize, 0);
}

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Store the vertex format and dirty transform/draw state; color-presence changes also dirty lighting.
int Render_SetFlexibleVertexFormatState(int vertexFormat) {
    int hasNewVertexColor, hadVertexColor;
    AsRenderState80422F10(g_engineState)->projectionTransformDirty = 1;
    AsRenderState80422F10(g_engineState)->drawStateDirty = 1;
    int old = AsRenderState80422F10(g_engineState)->flexibleVertexFormat;
    unsigned int oldColorBits = old & kVertexColorMask;
    unsigned int newColorBits = vertexFormat & kVertexColorMask;
    hadVertexColor = (oldColorBits != 0);
    hasNewVertexColor = (newColorBits != 0);
    if (hadVertexColor ^ hasNewVertexColor) {
        AsRenderState80422F10(g_engineState)->materialStateDirty = 1;
        AsRenderState80422F10(g_engineState)->lightingStateDirty = 1;
    }
    AsRenderState80422F10(g_engineState)->flexibleVertexFormat = vertexFormat;
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Program alpha comparison or an always-pass test, and retain the original early-depth setting.
void Render_ApplyAlphaTestState() {
    int firstCompare = 7;
    int firstReference = 0;
    int alphaOperation = 0;
    int secondCompare = 7;
    int secondReference = 0;
    if (AsRenderState80422F10(g_engineState)->alphaTestEnabled != 0) {
        firstCompare = AsRenderState80422F10(g_engineState)->alphaCompare;
        firstReference = AsRenderState80422F10(g_engineState)->alphaReference & 0xFF;
    }
    GXSetAlphaCompare((GXCompare)firstCompare, (u8)firstReference, (GXAlphaOp)alphaOperation, (GXCompare)secondCompare,
                      (u8)secondReference);
    GXSetZCompLoc(0);
}
#pragma pop

// Count explicit, generated and declaration-derived texture coordinates, up to eight base stages.
// A declaration carrying types 3/6/7 adds three AFTER the base-stage count, so
// the result can exceed eight. No hardware-limit validation is performed here.
int VertexInput_CountTextureCoordinateGenerators() {
    unsigned int raw = (*(unsigned int*)&AsRenderState80422F10(g_engineState)->flexibleVertexFormat);
    int base = (raw >> kVertexTexCoordCountShift) & kVertexTexCoordCountMask;
    int dead = raw & kVertexTexCoordCountBits;
    (void)dead;
    int count = base;
    int i = base;
    if (i < 8)
        do {
            void* p = g_engineState;
            // Equivalent to AsRenderStages(p)->stages[i].texCoordSource; keep the
            // combined member offset because typed indexing changes registers at -O0.
            unsigned int ptr = *(unsigned int*)((char*)p + i * kTextureStageBytes +
                                                FIELD_OFFSET(RenderStageOwnerView, stages[0].texCoordSource));
            if (ptr == kNoGeneratedTexCoord)
                break;
            ++count;
            ++i;
        } while (i < 8);
    void* p = g_engineState;
    if (AsRenderState80422F10(p)->useVertexDeclaration != 0) {
        int extra = 0;
        unsigned int declaration = AsRenderState80422F10(g_engineState)->vertexDeclaration;
        if (VertexDeclaration_UsesSpecialTexCoordMapping(declaration) != 0)
            extra = 3;
        count += extra;
    }
    return count;
}

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Select disabled, blend/subtract or logic blending from the cached operations and factors.
// logicOperation==0x80000000 means use blendOperation (1 add, 3 subtract), otherwise use logic.
// GX mode codes: NONE=0, BLEND=1, LOGIC=2, SUBTRACT=3. Other blendOperation values leave
// type unassigned in the existing code: caller state must provide one of the supported operations.
void Render_ApplyBlendState() {
    int type;
    int logic = GX_LO_COPY;
    int blend_src = AsRenderState80422F10(g_engineState)->sourceBlend;
    int blend_dst = AsRenderState80422F10(g_engineState)->destinationBlend;
    if (AsRenderState80422F10(g_engineState)->blendEnabled == 0) {
        type = GX_BM_NONE;
    } else if (AsRenderState80422F10(g_engineState)->logicOperation + kNoLogicOperation == 0) {
        switch (AsRenderState80422F10(g_engineState)->blendOperation) {
        case kBlendOperationAdd:
            type = GX_BM_BLEND;
            break;
        case kBlendOperationSubtract:
            type = GX_BM_SUBTRACT;
            break;
        }
    } else {
        type = GX_BM_LOGIC;
        logic = (*(int*)&AsRenderState80422F10(g_engineState)->logicOperation);
    }
    GXSetBlendMode((GXBlendMode)type, (GXBlendFactor)blend_src, (GXBlendFactor)blend_dst, (GXLogicOp)logic);
}
#pragma pop

extern unsigned int s_textureNeedsExtraTevStages[];
extern unsigned int s_defaultTevStateDirty;
extern const unsigned char s_defaultTevConstantRed;
extern const unsigned char s_defaultTevConstantGreen;
extern unsigned char s_defaultTevConstantBlue;
extern unsigned char s_defaultTevConstantAlpha;
extern void (*s_textureStageCombinerApplyCallbacks[])(int);
void Texture_GetLevelDescription(unsigned int, int, void*);
void TexturePalette_GetDescription(unsigned int, void*);
void Render_ConfigureFourStageTevCombiner(void*);
void Render_SetupIndirectBumpXYZTev();
void TextureProjection_SetupTintCombiner();
void GXSetTevKColor(int, void*);
void GXSetTevKColorSel(int, int);
void GXSetNumIndStages(int);
void GXSetTevDirect(int);
void GXSetTevSwapMode(int, int, int);

#pragma push
#pragma optimization_level 0
// Count texture/lighting TEV stages, configure stage orders and apply dirty or special combiners.
// Stage-allocation map:
// base texture span = min(last-bound-slot+1, max(colorOpCount,alphaOpCount), texCoordCount);
// each marked texture adds two TEV stages (extra texture map, then no-map combine);
// ordinary lighting reserves one diffuse stage plus optional specular, or one replacement stage.
// Hardware TEV limit is 16; callers must supply a representable combination (no clamp here).
// Physical TEV stage, texture-map index, texcoord index and logical texture slot advance differently.
// Custom TEV mode skips default lighting setup; effect overrides below may replace configuration.
// Format-to-budget table per visited logical texture (TEV stages / map slots /
// coordinate generators). All three expanded stages share ONE coordinate:
// ordinary texture                         1 / 1 / 1
// 80000009 C8 + palette 80000006 RGBA8       3 / 2 / 1
// C0000030 or C0000032 split color/alpha    3 / 2 / 1
// C8 with any other palette                1 / 1 / 1
// null logical slot                       1 / 0 / 0 (no-map/no-coordinate)
// Expanded recipe: stage S samples map M, S+1 samples map M+1 with the SAME
// coord C, S+2 has no map and combines; next texture starts S+3,M+2,C+1.
// Marked formats are discovered only up to the first disabled color/alpha op or
// coordinate-limit boundary. C8 requires a valid palette handle for the query.
// Budget all consumers together: 16 physical TEV stages, 8 texture maps and
// 8 generated coordinates. Initial lighting takes 1 TEV stage, +1 for specular;
// replacement lighting takes 1, with neither maps nor coordinates. In the
// ordinary bound contiguous case, N textures with E expansions cost
// N+2E+lighting stages, N+E maps and N coordinates. The actual code uses spans
// and can include holes; use its Phase-2 count for a sparse binding. Special
// effect overrides own their separate recipes and can return before dirty
// callbacks. No phase clamps an over-budget configuration to hardware limits.
void Render_ApplyTevState() {
    // Phase 1: inspect active texture formats/palettes and flag split color/alpha representations.
    int i;
    for (i = 0; i < 8; ++i)
        s_textureNeedsExtraTevStages[i] = 0;
    unsigned int availableTexCoords = (unsigned int)VertexInput_CountTextureCoordinateGenerators();
    int j;
    for (j = 0; j < 8; ++j) {
        if (AsRenderState80422F10(g_engineState)->textures[j] == 0)
            continue;
        // Equivalent typed fields: stages[j].colorOperation/alphaOperation.
        // Keep the folded offset in these -O0 reads (direct [] changes registers).
        if (*(unsigned int*)((char*)g_engineState + j * kTextureStageBytes +
                             FIELD_OFFSET(RenderStageOwnerView, stages[0].colorOperation)) == kTevOperationDisabled)
            break;
        if (*(unsigned int*)((char*)g_engineState + j * kTextureStageBytes +
                             FIELD_OFFSET(RenderStageOwnerView, stages[0].alphaOperation)) == kTevOperationDisabled)
            break;
        if ((unsigned int)j == availableTexCoords)
            break;
        unsigned int obj = AsRenderState80422F10(g_engineState)->textures[j];
        TextureFormatQuery textureDescription;
        Texture_GetLevelDescription(obj, 0, &textureDescription);
        // C8 with an RGBA8 palette and split compressed formats need the extra
        // texture-map/combine pair. Other formats use the ordinary single stage.
        if (textureDescription.format + kPlatformFormatBias == kIndexed8FormatCode) {
            PaletteFormatQuery paletteDescription;
            TexturePalette_GetDescription(AsRenderState80422F10(g_engineState)->palettes[j], &paletteDescription);
            if (paletteDescription.format + kPlatformFormatBias == kRgba8PaletteFormatCode)
                s_textureNeedsExtraTevStages[j] = 1;
        } else if (textureDescription.format + kSplitCompressedFormatBias == kSplitCompressedFormat30 ||
                   textureDescription.format + kSplitCompressedFormatBias == kSplitCompressedFormat32) {
            s_textureNeedsExtraTevStages[j] = 1;
        }
    }
    // Phase 2: count independent operation spans and reserve physical texture/lighting stages.
    int colorStageCount = 0;
    int alphaStageCount = 0;
    int colorStage;
    for (colorStage = 0; colorStage < 8; ++colorStage) {
        if (*(unsigned int*)((char*)g_engineState + colorStage * kTextureStageBytes +
                             FIELD_OFFSET(RenderStageOwnerView, stages[0].colorOperation)) == kTevOperationDisabled)
            break;
        ++colorStageCount;
    }
    int alphaStage;
    for (alphaStage = 0; alphaStage < 8; ++alphaStage) {
        if (*(unsigned int*)((char*)g_engineState + alphaStage * kTextureStageBytes +
                             FIELD_OFFSET(RenderStageOwnerView, stages[0].alphaOperation)) == kTevOperationDisabled)
            break;
        ++alphaStageCount;
    }
    int boundTextureCount = 0;
    int boundStage;
    for (boundStage = 0; boundStage < 8; ++boundStage) {
        if (AsRenderState80422F10(g_engineState)->textures[boundStage] != 0)
            boundTextureCount = boundStage + 1;
    }
    unsigned int* maxStagePointer;
    if ((unsigned int)colorStageCount < (unsigned int)alphaStageCount)
        maxStagePointer = (unsigned int*)&alphaStageCount;
    else
        maxStagePointer = (unsigned int*)&colorStageCount;
    unsigned int* savedMaxStagePointer = maxStagePointer;
    unsigned int maxOperationStages = *maxStagePointer;
    unsigned int* minStagePointer;
    if ((unsigned int)boundTextureCount < maxOperationStages)
        minStagePointer = (unsigned int*)&boundTextureCount;
    else
        minStagePointer = (unsigned int*)&maxOperationStages;
    unsigned int* savedMinStagePointer = minStagePointer;
    unsigned int w = *minStagePointer;
    unsigned int textureStageLimit = w;
    unsigned int totalStageCount = w;
    unsigned int texCoordCount = (unsigned int)VertexInput_CountTextureCoordinateGenerators();
    if (totalStageCount > texCoordCount)
        totalStageCount = texCoordCount;
    int extraTextureStages = 0;
    int extraStage;
    for (extraStage = 0; extraStage < 8; ++extraStage) {
        if (s_textureNeedsExtraTevStages[extraStage] != 0)
            extraTextureStages += 2;
    }
    int extraStageCount = extraTextureStages;
    totalStageCount += (unsigned int)extraTextureStages;
    int lightingStageCount;
    int lightingChannels;
    if (AsRenderState80422F10(g_engineState)->replaceLightingTevStage != 0) {
        lightingStageCount = 0;
    } else {
        lightingChannels = 1;
        if (AsRenderState80422F10(g_engineState)->specularLightingEnabled != 0)
            ++lightingChannels;
        lightingStageCount = lightingChannels;
    }
    totalStageCount += (unsigned int)lightingStageCount;
    if (AsRenderState80422F10(g_engineState)->replaceLightingTevStage != 0)
        ++totalStageCount;
    int computedStageCount = (int)totalStageCount;
    int nextTevStage = 0;
    unsigned int tevStageCount = totalStageCount;
    // Phase 3: initialize default swap/direct state and diffuse/specular lighting combiners.
    if (AsRenderState80422F10(g_engineState)->type7Resource == 0) {
        if (s_defaultTevStateDirty != 0) {
            // Swap 1 produces (R,A,B,A); swap 2 replicates G into RGB and keeps A.
            GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_ALPHA, GX_CH_BLUE, GX_CH_ALPHA);
            GXSetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
            GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
            // TEV add is D + (1-C)*A + C*B: RGB = raster0 + K0,
            // alpha = raster0 alpha. Clamp to [0,1] and store in PREV.
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_ONE, GX_CC_RASC);
            GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, (GXBool)1, GX_TEVPREV);
            GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
            GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, (GXBool)1, GX_TEVPREV);
            GXSetNumIndStages(0);
            int directStage;
            for (directStage = 0; directStage < 16; ++directStage) {
                unsigned int t = (unsigned int)directStage;
                GXSetTevDirect((int)t);
                GXSetTevSwapMode((int)t, 0, 0);
            }
            s_defaultTevStateDirty = 0;
        }
        unsigned char color[4];
        color[0] = s_defaultTevConstantRed;
        color[1] = s_defaultTevConstantGreen;
        color[2] = s_defaultTevConstantBlue;
        color[3] = s_defaultTevConstantAlpha;
        unsigned char* ambientColor = AsRenderColors(g_engineState)->ambient;
        unsigned char* materialColor = AsRenderColors(g_engineState)->material;
        unsigned char* emissiveColor = AsRenderColors(g_engineState)->emissive;
        // K0 starts at the shared default; register ambient selects ambient*material.
        // Emissive is then added per byte (wrapping here before GX's later clamp).
        if (AsRenderState80422F10(g_engineState)->ambientColorSource == 0)
            Color_MultiplyRGBA8(color, ambientColor, materialColor);
        if (AsRenderState80422F10(g_engineState)->ambientColorSource == 0 ||
            ((*(unsigned int*)&AsRenderState80422F10(g_engineState)->flexibleVertexFormat) & kVertexColorMask) == 0) {
            color[0] = (unsigned char)(color[0] + emissiveColor[0]);
            color[1] = (unsigned char)(color[1] + emissiveColor[1]);
            color[2] = (unsigned char)(color[2] + emissiveColor[2]);
            color[3] = (unsigned char)(color[3] + emissiveColor[3]);
        }
        unsigned char buf[4];
        unsigned char* bp = buf;
        buf[0] = color[0];
        buf[1] = color[1];
        buf[2] = color[2];
        buf[3] = color[3];
        GXSetTevKColor(0, buf);
        ++nextTevStage;
        if (AsRenderState80422F10(g_engineState)->specularLightingEnabled != 0) {
            // Specular channel adds raster1 RGB to PREV; alpha adds raster1
            // alpha times the currently selected constant alpha, again clamped.
            GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);
            GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_RASC, GX_CC_ONE, GX_CC_CPREV);
            GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, (GXBool)1, GX_TEVPREV);
            GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_RASA, GX_CA_KONST, GX_CA_APREV);
            GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, (GXBool)1, GX_TEVPREV);
            ++nextTevStage;
        }
    }
    // Phase 4: assign each physical TEV stage its texture map, coordinate source and raster channel.
    GXSetNumTevStages((u8)tevStageCount);
    int textureMapIndex = 0;
    int tevStage = nextTevStage;
    int texCoordIndex = 0;
    int textureIndex = 0;
    while (tevStage < tevStageCount) {
        unsigned int stageId = (unsigned int)tevStage;
        unsigned int mapId = (unsigned int)textureMapIndex;
        unsigned int channelId = 4;
        unsigned int coordId = (unsigned int)texCoordIndex;
        if (AsRenderState80422F10(g_engineState)->textures[textureIndex] == 0) {
            GXSetTevOrder((GXTevStageID)stageId, (GXTexCoordID)0xff, (GXTexMapID)0xff, (GXChannelID)channelId);
            ++tevStage;
            ++textureIndex;
            continue;
        }
        GXSetTevOrder((GXTevStageID)stageId, (GXTexCoordID)coordId, (GXTexMapID)mapId, (GXChannelID)channelId);
        if (s_textureNeedsExtraTevStages[textureIndex] != 0) {
            ++tevStage;
            ++textureMapIndex;
            GXSetTevOrder((GXTevStageID)tevStage, (GXTexCoordID)coordId, (GXTexMapID)textureMapIndex,
                          (GXChannelID)channelId);
            ++tevStage;
            GXSetTevOrder((GXTevStageID)tevStage, (GXTexCoordID)coordId, (GXTexMapID)0xff, (GXChannelID)channelId);
        }
        ++tevStage;
        ++textureMapIndex;
        ++texCoordIndex;
        ++textureIndex;
    }
    // Phase 5: first supported active effect wins and returns before ordinary dirty callbacks.
    int effectIndex;
    for (effectIndex = 0; effectIndex < 7; ++effectIndex) {
        // Typed equivalent: specialEffects[effectIndex]. Raw offset keeps the
        // original base-plus-index evaluation order under optimization level 0.
        if (*(int*)((char*)g_engineState + effectIndex * 4 + FIELD_OFFSET(RenderStageOwnerView, specialEffects)) == 0)
            continue;
        switch (effectIndex) {
        case kIndirectBumpEffect:
            Render_SetupIndirectBumpXYZTev();
            return;
        case kProjectionTintEffect:
            TextureProjection_SetupTintCombiner();
            return;
        case kFourStageEffect:
            Render_ConfigureFourStageTevCombiner(AsRenderState80422F10(g_engineState)->lights[0]);
            AsRenderState80422F10(g_engineState)->textureStateDirty = 0;
            return;
        case 2:
            break;
        }
    }
    // Phase 6: apply remaining per-texture color/alpha dirty callbacks, then clear their latches.
    if (AsRenderState80422F10(g_engineState)->textureStateDirty != 0) {
        unsigned int dirtyTexture;
        for (dirtyTexture = 0; dirtyTexture < 8; ++dirtyTexture) {
            int combinerIndex;
            for (combinerIndex = 0; combinerIndex < 2; ++combinerIndex) {
                // combinerDirty[logicalTexture][0=color,1=alpha], same owner;
                // retain address arithmetic because typed indexing changes -O0 code.
                if (*(int*)((char*)g_engineState + dirtyTexture * 8 + combinerIndex * 4 +
                            FIELD_OFFSET(RenderStageOwnerView, combinerDirty)) != 0) {
                    s_textureStageCombinerApplyCallbacks[combinerIndex](dirtyTexture);
                    *(int*)((char*)g_engineState + dirtyTexture * 8 + combinerIndex * 4 +
                            FIELD_OFFSET(RenderStageOwnerView, combinerDirty)) = 0;
                }
            }
        }
        AsRenderState80422F10(g_engineState)->textureStateDirty = 0;
    }
}
#pragma pop

} // extern "C"

// Layout guards keep the byte-address fallbacks tied to named state fields.
typedef char TextureStage_size[(sizeof(TextureStageView) == 0x64) ? 1 : -1];
typedef char RenderStage_color_offset[(FIELD_OFFSET(RenderStageOwnerView, stages[0].colorOperation) == 0x184) ? 1 : -1];
typedef char RenderStage_dirty_offset[(FIELD_OFFSET(RenderStageOwnerView, combinerDirty) == 0x14FC) ? 1 : -1];
typedef char RenderStage_effect_offset[(FIELD_OFFSET(RenderStageOwnerView, specialEffects) == 0x1688) ? 1 : -1];
