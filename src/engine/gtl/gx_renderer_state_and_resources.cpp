// GX renderer resources: vertex array and index header updates, render surface copies, the
// framebuffer presentation, and the setters for texture stages, viewports and render state.
//
// Draw calls flush the cached state first. Engine_Set* and Engine_Get* functions read and write
// that cache and the GX resources it refers to.

#include <decomp/record_sizes.h>
#include "classes/EngineState.h"
#include "sdk_protos.h"
#include "game/types.h"
#include <decomp/cw_bitfields.h>

extern "C" {

extern unsigned char* g_engineState;
void* g_drawUsesIndexDisplayList;
void* g_drawPrimitiveType;
void* g_drawUsesBoundVertexBuffer;
void* g_useIndexedVertexInput;
int s_defaultTevStateDirty;
char s_fullscreenDepthAuxTexture[32];
extern Mtx kIdentityMatrix4x4;
extern unsigned int s_fullscreenSurfaceVertices[20];
void* memcpy(void*, const void*, unsigned long);
extern const float kFloat_0_GXRenderer;
extern unsigned int s_textureMinMipFilterTable[18];
extern const float kDepth24Maximum_GXRenderer;
extern const float kDepthAxisFlip_GXRenderer;

// Borrowed vertex-format descriptor; copying its eight words transfers no ownership.
// entries[i].object is a GX attribute selector, offsets[i] its byte offset within one
// output vertex. count sizes both tables; stride is the converted vertex size.
// convert(dst, src, vertexCount, objectMatrix, transposedMatrices) writes packed
// vertices, excluding the four-byte draw header. Source stride comes from the active
// declaration/FVF; conversion is requested only when that differs from this stride.
// Engine_UpdateVertexArrays uses a 32-byte-aligned second region of the existing buffer
// for converted output. Buffer creation must provide that space; no allocation happens here.
enum PreDrawStateIndex {
    kLightingEnabledState = 2,
    kViewMatrixOnlyState = 9,
    kSpecularColorWithoutMaterialState = 17,
    kLightColorWithoutMaterialState = 18,
    kPairedSpecularLightsState = 31
};
enum DrawTransformSlot {
    kViewTransform = 0,
    kProjectionTransform = 1,
    kFirstTextureTransform = 2,
    kObjectTransform = 11
};
enum MaterialVertexInput {
    // Engine FVF color-input bits. A zero value selects constant white when
    // lighting is off; a nonzero value lets the vertex color supply modulation.
    kVertexColorInputMask = 0xF000
};
enum MaterialGxChannel {
    kGxColor0Alpha0 = 4, // GX_COLOR0A0: diffuse/material color and alpha together
    kGxColor1Alpha1 = 5  // GX_COLOR1A1: second channel for paired specular lights
};
// Cache IDs are engine ABI, verified against gx_renderer_state_and_efb_copy.cpp setters and gx_render_state_and_texture_upload.cpp consumers.
enum RendererDefaultStateId {
    kDefaultCullMode = 0,
    kDefaultDepthCompare = 3,
    kDefaultAlphaCompare = 4,
    kDefaultDepthWrite = 5,
    kDefaultDepthTest = 6,
    kDefaultAlphaTest = 7,
    kDefaultBlendEnabled = 10,
    kDefaultAlphaReference = 11,
    kDefaultBlendOperation = 12,
    kDefaultSourceBlend = 13,
    kDefaultDestinationBlend = 14,
    kDefaultLogicOperation = 15,
    kDefaultColorWriteMask = 30,
    kDefaultDepthScale = 32,
    kDefaultDepthOffset = 33
};
enum RendererDefaultTextureState {
    kDefaultColorOperation = 0,
    kDefaultColorArgument1 = 1,
    kDefaultColorArgument2 = 2,
    kDefaultAlphaOperation = 3,
    kDefaultAlphaArgument1 = 4,
    kDefaultAlphaArgument2 = 5,
    kDefaultTexCoordSource = 10,
    kDefaultWrapS = 18,
    kDefaultWrapT = 19
};
enum RendererDefaultValue {
    kCombineMultiply = 0,
    kCombineDisabled = 16,
    kCombineSelectFirst = 17,
    kArgumentTexture = 2,
    kArgumentPrevious = 1,
    kWrapRepeat = 1,
    kBlendAdd = 1,
    kBlendOne = 1,
    kBlendZero = 0,
    kNoLogicOperation = 0x80000000u,
    kNoGeneratedTexCoord = 0x80000000u,
    kWriteRgba = 0x01010101,
    kDepthLessEqual = 3,
    kAlphaAlways = 7,
    kCullBack = 2,
    kFloatOneBits = 0x3F800000,
    kFloatZeroBits = 0,
    kDisplaySurfaceFormat44 = 0x8000002Cu, // surface format ID; do not infer GX texture format.
    kOffscreenColorFormat6 = 0x80000006u,
    kOffscreenDepthFormat22 = 0x80000016u
};

enum GraphicsResourceLimits {
    kMinimumTextureExtentPixels = 4
};
// GX ABI enums used by the fullscreen copy path; format words also carry an engine high bit.
enum FullscreenGxCode {
    kGxTextureClamp = 0,
    kGxNearestFilter = 0,
    kGxOrthographicProjection = 1,
    kGxTexGenMatrix2x4 = 1,
    kGxTexCoord0 = 4,
    kGxIdentityTextureMatrix = 0x3C,
    kGxIdentityPostMatrix = 0x7D,
    kGxCompareAlways = 7,
    kGxDepth24Texture = 0x16,
    kGxDepthTextureDisabled = 0,
    kGxDepthTextureReplace = 2,
    kGxNullColorChannel = 0xFF,
    kGxTevColorZero = 0xF,
    kGxTevTextureColor = 8,
    kGxTevAlphaZero = 7,
    kGxTevTextureAlpha = 4,
    kGxDrawQuads = 0x80,
    kFullscreenQuadVertexCount = 4,
    kFullscreenVertexFormat = 0x110005
};
struct Table_805CB298 {
    struct Entry {
        void* object;
        unsigned int pad[3];
    }* entries;
    unsigned int* offsets;
    unsigned int pad08[2];
    void (*convert)(void*, void*, unsigned int, void*, void*);
    unsigned int count;
    unsigned int stride;
    unsigned int unknown1C; // Preserved by the original eight-word descriptor copy.
};
Table_805CB298 s_vertexFormatDescriptor;

void SixFaceRenderer_RenderToTexture(void*, void*);
void* GraphicsResource_GetStorage(void*);
void CubeResourceStorage_dtor(void*, short);
void RenderSurface_GetDescription(void*, void*);
void RenderSurface_GetDataAndPitch(void*, void*, int, int);
void GraphicsResource_NoOpSuccess(void*);
unsigned short TexturePalette_GetEntryCountForType(void*);
void* TexturePalette_CountRequiredSlots(void*, int);
void* TextureStage_CountTextureMapSlotsBeforeIndex(void*, int);
void* TextureStage_ClassifyTextureSlot(void*, int);
int TextureFormat_IsPaletted(void*);
void Texture_GetLevelDescription(void*, int, void*);
void GXInitTexObjCI(void*, void*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int,
                    unsigned int);
void* TexturePalette_ClassifySlot(void*, int);
void GXSetTexCopySrc(int, int, unsigned short, unsigned short);
void GXSetTexCopyDst(unsigned short, unsigned short, int, int);
void GXCopyTex(void*, int);
void GXInitTlutObj(void*, void*, unsigned int, unsigned short);
void GXLoadTlut(void*, int);
void GXSetDrawSync(unsigned short);
void GXPixModeSync(void);
void Engine_ApplySelectedTextureStageState(void);
void GXSetArray(void*, void*, unsigned char);
void* VertexDeclaration_GetElements(void*, void*, void*);
void* VertexDeclaration_GetStride(void*, int);
void* FlexibleVertexFormat_GetStride(void*);
void GXInitTexObjLOD(void*, int, int, float, float, float, GXBool, GXBool, int);
void MtxOrthoOffCenter(void*, float, float, float, float, float, float);
void PSMTX44Transpose(void*, void*);
void VertexInput_FindAndCopyFormatDescriptor(void*, unsigned int);
void GXSetNumIndStages(int);
void GXSetTevDirect(int);
void GXSetZTexture(int, int, int);
void GXSetZCompLoc(unsigned char);
void GXSetColorUpdate(int);
void GXSetAlphaUpdate(int);
void GXClearVtxDesc(void);
void GXBegin(int, int, unsigned short);
unsigned int VIGetNextField(void);
void GXSetViewportJitter(float, float, float, float, float, float, unsigned int);

struct Bits_80425930 {
    CW_BITS2(unsigned int high : 16, unsigned int low : 16)
};

struct TexInfo_804251B8 {
    unsigned int fmt;
    char reserved04[0x18];
    unsigned int width;
    unsigned int height;
};

// RenderSurface_GetDataAndPitch returns two ABI words: pitch first, borrowed image second.
struct ImgInfo_804251B8 {
    unsigned int pitchBytes;
    void* image;
};

struct VertexAttributeDesc {
    unsigned int id;
    unsigned int componentCount;
    unsigned int componentType;
    unsigned char fractionalBits;
    char pad0D[3];
};

struct Scene_804251B8 {
    VertexAttributeDesc* list;
    unsigned int reserved04;
    void (*callback)(void*, int, int);
    char reserved0C[8];
    unsigned int count;
    char tail[8];
};

struct FullscreenQuadVertices {
    unsigned int words[20];
};

struct EnginePresentParams {
    unsigned int width;
    unsigned int height;
    unsigned int reserved08;
    unsigned int reserved0C;
    int type;
    unsigned int reserved14[6];
    unsigned int flags;
    unsigned int tvFormat;
    unsigned int swapInterval; // 0x34: Engine_PresentFrame retrace policy
};

struct EngineViRect {
    unsigned int x, y, width, height;
    int format;
};

extern void Render_UpdateRadialCapture();
extern void Engine_UpdateTextureProjectionMatrices();
extern void TextureProjection_ApplyTwoStageTransforms();
extern void Render_ApplyVertexInputState();
extern void Render_ApplyTevState();
extern unsigned int VertexInput_CountTextureCoordinateGenerators();

struct DrawColor {
    unsigned char r, g, b, a;
    // Leave color bytes uninitialized, as required by disabled-lighting paths.
    DrawColor() {}
};
typedef Vec3f DrawVec;
struct DrawLight {
    int type;
    char pad04[0x30];
    DrawVec position, direction;
    float spot, reserved50, a0, a1, a2, start, end;
};
struct DrawViewport {
    unsigned int x, y, w, h;
    float nearz, farz;
};
enum DrawViewportOffset {
    kDrawViewport_x = (int)&((DrawViewport*)0)->x,
    kDrawViewport_y = (int)&((DrawViewport*)0)->y,
    kDrawViewport_w = (int)&((DrawViewport*)0)->w,
    kDrawViewport_h = (int)&((DrawViewport*)0)->h,
    kDrawViewport_nearz = (int)&((DrawViewport*)0)->nearz,
    kDrawViewport_farz = (int)&((DrawViewport*)0)->farz
};
enum RendererViCode {
    kViInterlaced = 0,
    kViDoubleStrike = 1,
    kViProgressive = 2,
    kViNtsc = 0,
    kViPal = 1,
    kViMpal = 2,
    kViEuRgb60 = 5,
    kViMaximumWidthPixels = 720
};
enum RendererOutputFormat {
    kOutputNtsc = 2,
    kOutputPal = 3,
    kOutputEuRgb60 = 4,
    kOutputMpal = 5
};
// Supplement the shared EngineState view without changing any shared header.
// These fields are selected by the GX calls below; padding preserves unexamined cache regions.
struct PackedRendererColor {
    unsigned char r, g, b, a;
};
struct RendererStateView {
    char prefix[0x14];
    EnginePresentParams presentation;
    EngineViRect outputRect;
    unsigned int offscreenRenderingEnabled;
    void* stageResources[8];
    void* paletteResources[8];
    void* vertexBuffer;
    unsigned int reservedA8;
    unsigned int vertexStride;
    unsigned int reservedB0;
    void* indexBuffer;
    unsigned int reservedB8;
    unsigned int vertexFormat;
    void* displayRenderTarget;
    void* nextDisplayRenderTarget;
    void* offscreenRenderTarget;
    void* currentRenderSurface;
    void* vertexDeclaration;
    unsigned int type6Resource;
    unsigned int type7Resource;
    unsigned int renderStates[42];
    unsigned int textureStageStates[8][25];
    unsigned int renderStateRows[8][9];
    unsigned int immediateStateUpdates;
    unsigned int reserved5C8;
    unsigned int useDistanceAttenuation;
    unsigned int reserved5D0;
    unsigned int useFifoBreakpoints;
    unsigned int reserved5D8;
    unsigned int useDefaultAmbient;
    char reserved5E0[8];
    DrawViewport viewport;
    char transforms[44][0x40]; // Initialization includes one matrix beyond the shared header's 43 entries.
    DrawLight lights[8];
    unsigned int lightEnabled[8];
    char materialCache[0x40];
    float specularPower;
    char callbackCache[0x30];
    unsigned int stateDirty[10];
    unsigned int textureStageComponentDirty[8][2];
    unsigned int vertexArraysDirty;
    unsigned int indexBufferHeaderDirty;
    unsigned int drawStateDirty;
    unsigned int textureStateDirty;
    unsigned int materialColorDirty;
    unsigned int lightStateDirty[8];
    unsigned int viewportDirty;
    unsigned int textureStageDirty[8];
    unsigned int paletteDirty[8];
    unsigned int transformDirty[43];
    unsigned int reserved1660;
    unsigned int renderTargetRequiresCopy;
    unsigned int usesVertexDeclaration;
    unsigned int type6Active[7];
    unsigned int type7Active[7];
    GXRenderModeObj videoMode;
    GXTexObj textureObjects[8];
    char lightObjects[8][64];
    unsigned int lastRetraceCount;
    void* fifoBuffer;
    unsigned int fifoBufferSize;
    unsigned int fifoDivisor;
    float fogNearPlane, fogFarPlane;
    char transposedTransforms[32][0x40];
    PackedRendererColor materialColors[3]; // Diffuse, ambient and specular colors used by GX lighting.
    char reserved2204[4];
    PackedRendererColor lightColors[8][3];
    void* scratchVertexDeclaration;
};
#define RENDERER_OFFSET(f, o) typedef char RendererState_offset_##f[((int)&((RendererStateView*)0)->f == (o)) ? 1 : -1]
RENDERER_OFFSET(stageResources, 0x64);
RENDERER_OFFSET(vertexStride, 0xAC);
RENDERER_OFFSET(textureStageStates, 0x184);
RENDERER_OFFSET(viewport, 0x5E8);
RENDERER_OFFSET(lights, 0x1100);
RENDERER_OFFSET(lightEnabled, 0x1440);
RENDERER_OFFSET(specularPower, 0x14A0);
RENDERER_OFFSET(stateDirty, 0x14D4);
RENDERER_OFFSET(vertexArraysDirty, 0x153C);
RENDERER_OFFSET(viewportDirty, 0x1570);
RENDERER_OFFSET(type6Active, 0x166C);
RENDERER_OFFSET(videoMode, 0x16A4);
RENDERER_OFFSET(textureObjects, 0x16E0);
RENDERER_OFFSET(lastRetraceCount, 0x19E0);
RENDERER_OFFSET(materialColors, 0x21F8);
RENDERER_OFFSET(scratchVertexDeclaration, 0x2268);
#undef RENDERER_OFFSET
// Type-6/7 active masks select custom renderer resource paths; they are not cleared after a draw.
// Field-derived byte offsets retain the original evaluation order in the unoptimized draw-state upload.
#define RENDERER_FIELD_OFFSET(f) ((int)&((RendererStateView*)0)->f)
struct TexturePaletteView {
    char header[0xC];
    void* paletteType;
    unsigned int paletteSize;
    unsigned int format;
};
// Binding-only resource view: eight 16-byte level records precede the encoded format.
// All pointers refer to existing storage; this view neither allocates nor owns it.
struct TextureBindingResourceView {
    unsigned int flags;
    char reserved04[8];
    unsigned int levelCount;
    unsigned int reserved10;
    struct Level {
        unsigned int width, height, imageOffset, imageSize;
    } levels[8];
    unsigned int encodedFormat;
};
struct TextureBindingWrapperView {
    char prefix[0x34];
    unsigned char* resource;
};
enum TextureBindingEncoding {
    kDirectTextureResource = 3,
    kWrappedTextureResource = 4,
    kTextureFormatWithoutResourceFlag = 0x7FFFFFFF,
    kTextureFormatWithoutSplitFlags = 0x3FFFFFFF,
    kSplitFormatComparisonBias = 0x40000000u,
    kSplitCmprI4Tag = 0x30,
    kSplitCmprI8Tag = 0x32,
    kFlaggedRgba8Tag = 6,
    kGxCompressedPlaneFormat = 14,
    kGxIntensity4PlaneFormat = 0,
    kGxIntensity8PlaneFormat = 1,
    kMinMipFilterRowCount = 6,
    kGxTrilinearMinFilter = 5,
    kAnisotropyEligibleMagFilter = 2,
    kAnisotropyRequest1 = 1,
    kAnisotropyRequest2 = 2,
    kAnisotropyRequest3 = 3,
    kAnisotropyRequest4 = 4,
    kGxAnisotropy2 = 1,
    kGxAnisotropy4 = 2
};
// Encoded format decision table (plane count comes from ClassifyTextureSlot):
// paletted:        format & 0x7fffffff -> CI object(s), consecutive TLUT slots.
// 0xc0000030:      plane 0 = CMPR (14); plane 1 = I4 (0).
// 0xc0000032:      plane 0 = CMPR (14); plane 1 = I8 (1).
//                 plane 1 advances by (lastLevel.imageOffset + imageSize) / 2.
// 0xc0000006:      clear both resource flags -> RGBA8 (6).
// all other plain formats: clear bit 31 and retain the remaining format bits.
// The unsigned +0x40000000 comparisons deliberately preserve wraparound encoding.
// Minification is a six-row (minFilter, mipFilter) lookup. A one-level texture
// forces mipFilter=0; only GX filter 5 plus raw magnification tag 2 enables the
// anisotropy mapping: request 1/3 -> none, 2 -> GX 2x, 4 -> GX 4x.
// The table must cover each input pair: the native code has no fallback row.
struct TextureStageBindingView {
    unsigned int reserved00[10];
    unsigned int coordinateSource;
    unsigned int reserved2C[2];
    unsigned int transformDimensions;
    unsigned int reserved38[4];
    unsigned int wrapS, wrapT, minFilter, magFilter, mipFilter;
    float lodBias;
    unsigned int anisotropy;
};
typedef char
    TextureBindingResource_formatOffset[((int)&((TextureBindingResourceView*)0)->encodedFormat == 0x94) ? 1 : -1];
typedef char TextureStageBinding_size[(sizeof(TextureStageBindingView) == 0x64) ? 1 : -1];
// Shift the cache base before selecting a typed field to preserve -O0 address formation.
#define TEXTURE_STAGE_FIELD(index, field)                                          \
    (((TextureStageBindingView*)(MGR + (index) * sizeof(TextureStageBindingView) + \
                                 RENDERER_FIELD_OFFSET(textureStageStates)))       \
         ->field)
#define DRAW_TEXTURE_STAGE_FIELD(index, field)                                                                  \
    (*(unsigned int*)((char*)g_engineState +                                                                    \
                      (RENDERER_FIELD_OFFSET(textureStageStates) + (int)&((TextureStageBindingView*)0)->field + \
                       (index) * sizeof(TextureStageBindingView))))
// Cached matrix slots require offset + stride*index before adding the engine base.
// Keep that addition order: -O0 otherwise emits a different load/address sequence.
struct DrawTransformDirtyView {
    unsigned int dirty;
};
struct DrawTransformMatrixView {
    char matrix[0x40];
};
enum DrawCachedMode {
    kPointLight = 1,
    kSpotLight = 2,
    kDirectionalLight = 3,
    kTexCoordsFromPosition = 0x80020000u,
    kTexCoordsFromNormal = 0x80010000u,
    kTexCoordsFromNormalWithPostMatrix = 0x80030000u,
    kVertexPositionFormatMask = 15,
    kClipSpacePositionFormat = 13,
    kScreenSpacePositionFormat = 5,
    kIndexedObjectTransforms = 0x800000,
    kRadialCaptureTexture = 1,
    kProjectedTexture = 4,
    kPairedProjectionTexture = 5
};
#define DRAW_TRANSFORM_DIRTY(slot)                                                                                  \
    (((DrawTransformDirtyView*)((char*)g_engineState +                                                              \
                                (RENDERER_FIELD_OFFSET(transformDirty) + (slot) * sizeof(DrawTransformDirtyView)))) \
         ->dirty)
#define DRAW_TRANSFORM_MATRIX(slot)                                                                               \
    (((DrawTransformMatrixView*)((char*)g_engineState +                                                           \
                                 (RENDERER_FIELD_OFFSET(transforms) + (slot) * sizeof(DrawTransformMatrixView)))) \
         ->matrix)
struct GraphicsBufferSizeView {
    char header[0xC];
    unsigned int byteSize;
};
struct RenderSurfaceWrapperView {
    char header[0x1C];
    void* sourceFaces[6];
    void* texture;
};
struct RendererDefaultsView {
    char prefix[0xA0];
    unsigned int textureStageCount;
};
enum {
    GpuSync_FrameBoundary = 0xBEEF,
    GpuSync_TextureCopy = 0xDEAD,
    GpuSync_FifoBreakpoint = 0xB00B,
    Clear_Color = 1,
    Clear_Depth = 2,
    Presentation_Interlaced = 0x20,
    Presentation_Progressive = 0x40,
    Presentation_FieldRendering = 0x80
};

// The Wii ABI passes these 4-byte colour structs by reference.
extern void GXSetChanAmbColor(int, DrawColor);
extern void GXSetChanMatColor(int, DrawColor);
extern void GXInitLightColor(void*, DrawColor);
extern void GXLoadLightObjImm(void*, unsigned int);
extern void GXInitLightPos(void*, float, float, float);
extern void GXInitLightDir(void*, float, float, float);
extern void GXInitSpecularDir(void*, float, float, float);
extern void GXInitLightSpot(void*, float, int);
extern void GXInitLightDistAttn(void*, float, float, int);
extern void GXInitLightAttnK(void*, float, float, float);
extern void GXInitLightAttn(void*, float, float, float, float, float, float);
extern void Color_MultiplyRGBA8(DrawColor*, void*, void*);
extern void Vec3_Normalize(DrawVec*, const DrawVec*);
extern void Vec3_Scale(DrawVec*, const DrawVec*, float);
extern void Mtx44_TransformPointPerspective(DrawVec*, const DrawVec*, void*);
extern void Mtx44_TransformDirection(DrawVec*, const DrawVec*, void*);
extern void Vector_CopyComponents(DrawVec*, const DrawVec*);
extern void PSMTX44Concat(void*, const void*, const void*);
extern void Mtx44_InvertLegacyCofactors(void*, int, const void*);
extern void Mtx44_SetScale(void*, float, float, float);
extern void Mtx44_SetTranslation(void*, float, float, float);
extern void Mtx44_TransposeToMtx34(void*, const void*);
extern void PSMTXInvXpose(const Mtx, Mtx);
extern unsigned int VertexDeclaration_UsesSpecialTexCoordMapping(unsigned int);
extern DrawColor kDefaultAmbientColor;
extern const float kFloat_1_Renderer, kFloat_0_5_Light;
extern const float kFloat_neg_100000_GXRenderer, kFloat_90_GXRenderer, kFloat_100000_GXRenderer, kFloat_0_99_GXRenderer;
extern const float kRadiansToDegrees_GXRenderer, kFloat_0_01_GXRenderer, kFloatMax_GXRenderer,
    kSpecularAttenuationScale_GXRenderer;
extern const float kFloat_neg_0_5_GXRenderer, kFloat_2_Renderer;
struct DrawTable11 {
    unsigned int v[11];
};
struct DrawTable4 {
    unsigned int v[4];
};
extern DrawTable11 s_texCoordMatrixIds;
extern DrawTable4 s_specialTexCoordSources, s_specialTexCoordMatrixIds;
extern void (*s_dirtyRenderStateCallbacks[10])();
extern int s_normalMatrixOverrideActive;

struct DrawProjection {
    float fov, aspect, nearz, farz;
};

extern void Render_BuildVerticalFilterWeights(void*, float);

struct CallbackManager {
    char pad[0x14BC];
    void (*callback)(void*);
    void* argument;
    char pad14C4[8];
    void (*pendingCallback)(void*);
    void* pendingArgument;
};

// The 24-byte viewport record: u32 x, y, width, height; float nearZ, farZ (Viewport_Copy stores all six).
struct Result804280EC {
    unsigned int words[4];
    float values[2];
};
typedef char ViewportRecordSizeCheck[sizeof(Result804280EC) == kViewportBytes ? 1 : -1];

struct DtorVtable80427B58 {
    void* reserved0;
    void (*destroy)(void*);
};

extern DtorVtable80427B58* g_graphicsMemoryCallbacks;

extern void* memset(void*, int, unsigned int);
void GraphicsResource_InitType6(void* self, unsigned int value);
void Texture_Ctor2D(void*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);

struct StageResourceRef {
    unsigned int refs;
};

struct Entry80428658 {
    unsigned char pad[0x64];
    StageResourceRef* ref;
};

struct GraphicsAllocator {
    void* (*allocate)(unsigned int);
};

void VertexDeclaration_CopyConstruct(void*, void*);
void GraphicsResource_InitType7(void*, void*);
void VertexBuffer_Ctor(void*, void*, void*, void*, void*);
void RenderSurface_Ctor(void*, void*, void*, void*, int);
void TexturePalette_Ctor(void*, void*, void*, void*, void*);
void IndexBuffer_Ctor(void*, void*, void*, void*, void*, void*);
int Engine_SetTextureStageState(void*, unsigned int, unsigned int);
int GXGetCPUFifo(void*);
void GXGetFifoPtrs(void*, void*, void*);
void GXEnableBreakPt(unsigned int);

void GXSetPixelFmt(int, int);
void GXSetDispCopyYScale(float);
void GXSetTevKColorSel(int, int);
void GXDisableBreakPt();
void GXSetDrawDone();
void* GXInit(void*, unsigned int);
void GXSetCPUFifo(void*);
void GXSetGPFifo(void*);

extern int s_gxRendererInitialized;
void GXInvalidateVtxCache();
void GXInvalidateTexAll();
void GXSetCopyClear(void*, unsigned int);
void GXSetDispCopySrc(unsigned int, unsigned int, unsigned int, unsigned int);
void GXSetDispCopyDst(unsigned int, unsigned int);
void GXCopyDisp(void*, int);
void VIFlush();
unsigned int Video_GetRetraceCount(); // Read the video-interface retrace counter.
void VISetNextFrameBuffer(void*);
void Render_WaitForDrawCompletion();

struct Header804272F4 {
    unsigned int a, b, c, d, e;
    unsigned int present[14];
    unsigned int rect[6];
};
struct Def804272F4 {
    unsigned int w[3];
};
struct Vt804272F4 {
    void* reserved0;
    void* reserved4;
    void* reserved8;
};
struct RefB804272F4 {
    CW_BITS2(unsigned int hi : 16, unsigned int lo : 16)
};

extern Def804272F4 s_scratchVertexDeclarationElements;
void VIInit();
void VISetBlack(unsigned int);
void Texture_LockLevel(void*, unsigned int, void*, unsigned int, unsigned int, unsigned int);
void Texture_UnlockLevel(void*, unsigned int);
void PSMTX44Identity(void*);

struct RenderSurfaceRef {
    unsigned int refs;
};
struct RenderStateRow {
    unsigned int fields[9];
};

// Return XFB/EFB height ratio for presentation scaling.
inline float RenderAspect(const GXRenderModeObj* mode) {
    float xfbHeight;
    float efbHeight;
    xfbHeight = (float)mode->xfbHeight;
    efbHeight = (float)mode->efbHeight;
    return xfbHeight / efbHeight;
}

void Engine_UpdateVertexArrays(void);
int Render_AlignSizeToMultiple(int n, int m);
void Engine_UpdateIndexBufferHeader(void);
void Color_CopyRGBA8(void* destination, const void* source);
void Render_LoadTexturePalette(int index);
void Render_ApplyTextureStage(int index);
void Render_ApplyLightingChannelCount(void);
void Render_UpdateSixFaceTextureStage(int index);
void RenderSurface_CopyFramebufferToTexture(void* arg);
void RenderSurface_DrawFullscreen(void* arg);
void Engine_UpdateOffscreenRenderTarget(void);
int GraphicsResource_Release(void* object);
void Engine_ApplyStateBeforeDraw();
void Engine_BuildRenderMode(GXRenderModeObj* out, volatile EnginePresentParams* params, EngineViRect* rect);
void Presentation_ApplyViScanModeFlags(unsigned int* a, const unsigned int* b);
void Render_OnDrawSyncToken(short value);
void Render_OnDrawDone();
void Engine_InitializeGXPixelAndTevDefaults();
void Render_InitializeAndBindGXFifo(void* unused, int useGX, int initFifo);
int Engine_InitializeVideoAndRenderTargets(void* source);
void Renderer_EmptyHook();
int Renderer_InitializeDefaults(void* self);
int Engine_SetCurrentRenderSurface(void* value);
int Engine_AcquireCurrentRenderSurface(RenderSurfaceRef** out);
int Texture_Create2D(unsigned int widthPixels, unsigned int heightPixels, unsigned int mipLevels,
                     unsigned int layoutFlags, unsigned int format, unsigned int allocationMode, void** out);
void GraphicsResource_Free(void* self);
int TexturePalette_Create(unsigned int paletteType, unsigned int unused, unsigned int format,
                          unsigned int allocationMode, void** out);
int VertexBuffer_Create(unsigned int byteCount, unsigned int usage, unsigned int vertexFormat,
                        unsigned int allocationMode, void** out);
int IndexBuffer_Create(unsigned int indexCount, unsigned int usage, unsigned int format, unsigned int allocationMode,
                       void** out, void* unused, unsigned int indexStrideBytes);
int GraphicsResource_CreateType6(void* source, void** out);
int VertexDeclaration_CreateCopy(void* source, void** out);
int GraphicsResource_CreateType7(void* source, void** out);
int RenderSurface_Create(unsigned int widthPixels, unsigned int heightPixels, unsigned int format, void** out);
int RenderSurface_CreateAdapter(unsigned int widthPixels, unsigned int heightPixels, unsigned int format, void* unused3,
                                void* unused4, void* unused5, void** out);
int Engine_ClearFramebuffer(void* unused, void* src, unsigned int flags, unsigned int color);
void RenderRect_Copy(volatile unsigned int* dst, const volatile unsigned int* src);
int Engine_SetViewport(const void* value);
void Viewport_Copy(volatile Result804280EC* dst, const volatile Result804280EC* src);
int Engine_GetViewport(void* self);
int Engine_PresentFrame();
int Engine_SwapDisplayBuffers();
int Engine_ChangeDisplayMode(void* modeArg);
int Engine_SetTextureStageResource(unsigned int index, void* value);
int Engine_GetTextureStageResource(int index, StageResourceRef** out);
int Engine_SetTextureStageParameter(void* a, unsigned int type, unsigned int c);
int Engine_GetRenderStateRowField(int index, int field, unsigned int* out);
int Engine_SetTextureStageParameterList(void* a, unsigned int* entries, unsigned int count);

// These routines deliberately reload the manager; other routines use ordinary reads.
#define MGR (*(unsigned char* volatile*)&g_engineState)

#define UPLOAD s_vertexFormatDescriptor

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma optimize_for_size on
#pragma dont_inline on
// Convert and flush bound vertex data, then bind GX arrays for indexed input.
void Engine_UpdateVertexArrays(void) {
    unsigned int storage[0x30];
    unsigned int stride;
    int convert;
    void* buffer;
    unsigned int offset;
    unsigned char* source;
    unsigned int count;
    unsigned int flushSize;
    void* drawBuffer;
    unsigned int drawStride;
    unsigned int drawOffset;
    unsigned int i;
    void* object;
    unsigned char* data;
    void* format;
    unsigned int formatCount;
    unsigned char* base;
    unsigned char* sourceBase;
    unsigned char* drawBase;
    unsigned int dataOffset;
    unsigned int strideResult;
    unsigned int strideCopy;
    unsigned char* headerCopy;
    unsigned char* volatile headerCopy2;
    unsigned char* sourceCopy;
    unsigned char* sourceCopy2;
    unsigned char* dataCopy;
    unsigned char* dataCopy2;
    unsigned char drawType;

    stride = 0;
    if (((EngineState*)MGR)->usesVertexDeclaration != 0) {
        format = (*(void**)&((EngineState*)MGR)->vertexDeclaration);
        VertexDeclaration_GetElements(format, storage, &formatCount);
        strideResult = (unsigned int)VertexDeclaration_GetStride(storage, 0);
        strideCopy = strideResult;
        stride = strideCopy;
    } else {
        stride = (unsigned int)FlexibleVertexFormat_GetStride((*(void**)&((EngineState*)MGR)->vertexFormat));
    }
    convert = (UPLOAD.stride - stride) != 0;
    if (((EngineState*)MGR)->vertexBuffer != 0 && g_drawUsesBoundVertexBuffer != 0) {
        buffer = ((EngineState*)MGR)->vertexBuffer;
        offset = 0;
        if (convert != 0) {
            offset = Render_AlignSizeToMultiple(((GraphicsBufferSizeView*)buffer)->byteSize + 4, 32);
        }
        base = (unsigned char*)GraphicsResource_GetStorage(buffer);
        register unsigned char* header = base + offset;
        headerCopy = header;
        headerCopy2 = header;
        sourceBase = (unsigned char*)GraphicsResource_GetStorage(buffer);
        count = ((GraphicsBufferSizeView*)buffer)->byteSize / stride;
        header[0] = 0;
        sourceCopy = sourceBase;
        header[1] = *(unsigned int*)&g_drawPrimitiveType;
        sourceCopy2 = sourceCopy;
        source = sourceCopy2;
        *(unsigned short*)(header + 2) = count;
        flushSize = 4;
        if (convert != 0) {
            flushSize = Render_AlignSizeToMultiple(UPLOAD.stride * count + 4, 32);
            UPLOAD.convert(header + 4, source + 4, count, (unsigned char*)AsEngineState(g_engineState)->transforms[11],
                           (unsigned char*)AsEngineState(g_engineState)->transposedTransforms);
        }
        DCFlushRange(header, flushSize);
    }
    if (g_useIndexedVertexInput != 0) {
        drawBuffer = ((EngineState*)MGR)->vertexBuffer;
        drawStride = ((RendererStateView*)MGR)->vertexStride;
        drawOffset = 0;
        if (convert != 0) {
            drawStride = UPLOAD.stride;
            drawOffset = Render_AlignSizeToMultiple(((GraphicsBufferSizeView*)drawBuffer)->byteSize + 4, 32);
        }
        for (i = 0; i < UPLOAD.count; ++i) {
            object = UPLOAD.entries[i].object;
            dataOffset = drawOffset + UPLOAD.offsets[i] + 4;
            drawBase = (unsigned char*)GraphicsResource_GetStorage(drawBuffer);
            dataCopy = drawBase + dataOffset;
            dataCopy2 = dataCopy;
            data = dataCopy2;
            drawType = drawStride;
            if (object != 0) {
                GXSetArray(object, data, drawType);
            }
        }
    }
}
#pragma pop
#pragma optimize_for_size off
#pragma dont_inline off

#pragma push
#pragma optimization_level 0
// Round a size upward to the requested multiple; callers supply a nonzero multiple.
int Render_AlignSizeToMultiple(int n, int m) {
    register int flag;
    flag = n % m == 0;
    if (flag == 0) {
        n = n + (m - n % m);
    }
    return n;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Update and flush the bound index display-list header.
void Engine_UpdateIndexBufferHeader(void) {
    register unsigned char* dst;
    register void* active;
    register unsigned int offset;
    register unsigned int count;
    register unsigned int value;
    register unsigned int base;
    register unsigned int start;
    register void* object;
    if (((EngineState*)MGR)->indexBuffer == 0 || g_drawUsesIndexDisplayList == 0)
        return;
    object = ((EngineState*)MGR)->indexBuffer;
    active = object;
    offset = 0;
    start = (unsigned int)GraphicsResource_GetStorage(active);
    base = start;
    value = ((GraphicsBufferSizeView*)active)->byteSize;
    count = value >> 1;
    ((unsigned char*)base)[offset] = 0;
    dst = (unsigned char*)(base + offset);
    dst[1] = *(unsigned int*)&g_drawPrimitiveType;
    *(unsigned short*)(dst + 2) = (unsigned short)count;
    DCFlushRange(dst, 4);
}
#pragma pop

// Copy the four RGBA bytes in order, retaining volatile byte accesses.
void Color_CopyRGBA8(void* destination, const void* source) {
    volatile char* dst = (volatile char*)destination;
    const volatile char* src = (const volatile char*)source;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst++ = *src++;
    *dst = *src;
}

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma optimize_for_size on
#pragma peephole on
// Load every TLUT slot required by the selected palette resource.
void Render_LoadTexturePalette(int index) {
    __alloca(0);
    void* buf;
    unsigned int v14;
    unsigned int base2;
    unsigned int count2;
    unsigned int off;
    unsigned int s9;
    unsigned int dst;
    unsigned int q;
    unsigned int base;
    unsigned int count;
    unsigned int s4;
    unsigned int s3;
    struct {
        unsigned short hw;
        unsigned short pad;
    } s;
    char buffer[0x14];
    register void* obj;
    register int i;

    obj = (void*)AsEngineState(MGR)->palettes[index];
    if (obj != 0) {
        v14 = ((TexturePaletteView*)obj)->format;
        buf = buffer;
        s.hw = TexturePalette_GetEntryCountForType(((TexturePaletteView*)obj)->paletteType);
        base = (unsigned int)TexturePalette_CountRequiredSlots((char*)((RendererStateView*)MGR)->stageResources, index);
        base2 = base;
        count = (unsigned int)TexturePalette_ClassifySlot(
            (char*)(unsigned char*)((RendererStateView*)MGR)->stageResources, index);
        count2 = count;
        for (i = 0; i < count2; ++i) {
            off = 0;
            if (i == 1)
                off = (unsigned int)s.hw * 2;
            q = (unsigned int)GraphicsResource_GetStorage(obj);
            s4 = q + off;
            s9 = s4;
            s3 = s4;
            GXInitTlutObj(buf, (void*)s9, v14, s.hw);
            dst = base2 + i;
            GXLoadTlut(buf, (int)dst);
        }
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma optimize_for_size on
#pragma use_lmw_stmw on
#pragma peephole on
#pragma global_optimizer off
// Bind a texture stage, including split formats, palettes, wrapping and LOD filters.
// Binding phases share the caller's explicitly named scratch; no resource is owned here.
// Resolve the direct/wrapped resource, storage, extent, wrapping and GX slot count.
#define RESOLVE_TEXTURE_BINDING_RESOURCE()                                                                           \
    resourceKind = (*(unsigned int*)stageResourceBytes >> 13) & 7;                                                   \
    switch ((int)resourceKind) {                                                                                     \
    case kDirectTextureResource:                                                                                     \
        textureResourceBytes = stageResourceBytes;                                                                   \
        break;                                                                                                       \
    case kWrappedTextureResource:                                                                                    \
        textureResourceBytes = ((TextureBindingWrapperView*)stageResourceBytes)->resource;                           \
        break;                                                                                                       \
    }                                                                                                                \
    baseLevelWords = (unsigned int*)(textureResourceBytes + 0x14);                                                   \
    firstGxMapSlotReply = (unsigned int)TextureStage_CountTextureMapSlotsBeforeIndex(MGR + 0x64, textureStageIndex); \
    firstGxMapSlot = firstGxMapSlotReply;                                                                            \
    levelZeroImageOffset = baseLevelWords[2];                                                                        \
    resourceStorage = (unsigned char*)GraphicsResource_GetStorage(textureResourceBytes);                             \
    baseImageAddress = resourceStorage + levelZeroImageOffset;                                                       \
    imageAddressCopy = baseImageAddress;                                                                             \
    imageAddress = imageAddressCopy;                                                                                 \
    width = (unsigned short)baseLevelWords[0];                                                                       \
    height = (unsigned short)baseLevelWords[1];                                                                      \
    wrapS = TEXTURE_STAGE_FIELD(textureStageIndex, wrapS);                                                           \
    wrapT = TEXTURE_STAGE_FIELD(textureStageIndex, wrapT);                                                           \
    if (((TextureBindingResourceView*)textureResourceBytes)->levelCount == 1)                                        \
        hasMipmapsReply = 0;                                                                                         \
    else                                                                                                             \
        hasMipmapsReply = 1;                                                                                         \
    mipmap = hasMipmapsReply;                                                                                        \
    mapCountReply = (unsigned int)TextureStage_ClassifyTextureSlot(MGR + 0x64, textureStageIndex);                   \
    gxMapCount = mapCountReply;

// Initialize paletted or plain GX texture objects, including split image planes.
#define INITIALIZE_TEXTURE_BINDING_PLANES()                                                                          \
    if (TextureFormat_IsPaletted((void*)((TextureBindingResourceView*)textureResourceBytes)->encodedFormat)) {       \
        paletteGxFormat =                                                                                            \
            ((TextureBindingResourceView*)textureResourceBytes)->encodedFormat & kTextureFormatWithoutResourceFlag;  \
        paletteSlotReply = (unsigned int)TexturePalette_CountRequiredSlots(MGR + 0x64, textureStageIndex);           \
        firstGxPaletteSlot = paletteSlotReply;                                                                       \
        for (palettePlane = 0; palettePlane < gxMapCount; ++palettePlane) {                                          \
            paletteTextureObject = (GXTexObj*)(MGR + ((firstGxMapSlot + palettePlane) << 5) + 0x16E0);               \
            gxPaletteSlot = firstGxPaletteSlot + palettePlane;                                                       \
            GXInitTexObjCI(paletteTextureObject, imageAddress, width, height, paletteGxFormat, wrapS, wrapT, mipmap, \
                           gxPaletteSlot);                                                                           \
        }                                                                                                            \
    } else {                                                                                                         \
        for (unpalettedPlane = 0; unpalettedPlane < gxMapCount; ++unpalettedPlane) {                                 \
            plainTextureObject = (GXTexObj*)(MGR + ((firstGxMapSlot + unpalettedPlane) << 5) + 0x16E0);              \
            gxTextureFormat = ((TextureBindingResourceView*)textureResourceBytes)->encodedFormat &                   \
                              kTextureFormatWithoutResourceFlag;                                                     \
            if (((TextureBindingResourceView*)textureResourceBytes)->encodedFormat + kSplitFormatComparisonBias ==   \
                    kSplitCmprI4Tag ||                                                                               \
                ((TextureBindingResourceView*)textureResourceBytes)->encodedFormat + kSplitFormatComparisonBias ==   \
                    kSplitCmprI8Tag) {                                                                               \
                if ((int)unpalettedPlane == 0)                                                                       \
                    gxTextureFormat = kGxCompressedPlaneFormat;                                                      \
                else if ((int)unpalettedPlane == 1) {                                                                \
                    if (((TextureBindingResourceView*)textureResourceBytes)->encodedFormat +                         \
                            kSplitFormatComparisonBias ==                                                            \
                        kSplitCmprI4Tag)                                                                             \
                        splitPlaneGxFormat = kGxIntensity4PlaneFormat;                                               \
                    else                                                                                             \
                        splitPlaneGxFormat = kGxIntensity8PlaneFormat;                                               \
                    gxTextureFormat = splitPlaneGxFormat;                                                            \
                    lastLevelWords =                                                                                 \
                        (unsigned int*)(textureResourceBytes +                                                       \
                                        (((TextureBindingResourceView*)textureResourceBytes)->levelCount - 1) * 16 + \
                                        0x14);                                                                       \
                    secondPlaneByteOffset = (*(int*)(lastLevelWords + 2) + *(int*)(lastLevelWords + 3)) / 2;         \
                    secondPlaneImageAddress = imageAddress + secondPlaneByteOffset;                                  \
                    imageAddress = secondPlaneImageAddress;                                                          \
                }                                                                                                    \
            } else if (((TextureBindingResourceView*)textureResourceBytes)->encodedFormat +                          \
                           kSplitFormatComparisonBias ==                                                             \
                       kFlaggedRgba8Tag) {                                                                           \
                gxTextureFormat &= kTextureFormatWithoutSplitFlags;                                                  \
            }                                                                                                        \
            GXInitTexObj(plainTextureObject, imageAddress, width, height, (GXTexFmt)gxTextureFormat,                 \
                         (GXTexWrapMode)wrapS, (GXTexWrapMode)wrapT, mipmap);                                        \
        }                                                                                                            \
    }

// Resolve the min/mip-filter pair and configure LOD and anisotropy for every plane.
#define CONFIGURE_TEXTURE_BINDING_LOD()                                                                           \
    Texture_GetLevelDescription(textureResourceBytes, 0, baseLevelDescription);                                   \
    for (lodPlane = 0; lodPlane < gxMapCount; ++lodPlane) {                                                       \
        maxLod = (float)(((TextureBindingResourceView*)textureResourceBytes)->levelCount - 1);                    \
        rawMagFilter = TEXTURE_STAGE_FIELD(textureStageIndex, magFilter);                                         \
        biasClamp = edgeLod = gxAnisotropy = 0;                                                                   \
        lodTextureObject = (GXTexObj*)(MGR + ((firstGxMapSlot + lodPlane) << 5) + 0x16E0);                        \
        minLod = kFloat_0_GXRenderer;                                                                             \
        lodBias = TEXTURE_STAGE_FIELD(textureStageIndex, lodBias);                                                \
        minMipLookup = *(const FilterTable*)s_textureMinMipFilterTable;                                           \
        rawMinFilter = TEXTURE_STAGE_FIELD(textureStageIndex, minFilter);                                         \
        rawMipFilter = TEXTURE_STAGE_FIELD(textureStageIndex, mipFilter);                                         \
        if (mipmap == 0)                                                                                          \
            rawMipFilter = 0;                                                                                     \
        for (filterRowIndex = 0; filterRowIndex < kMinMipFilterRowCount; ++filterRowIndex) {                      \
            filterRow = &minMipLookup.rows[filterRowIndex];                                                       \
            if (filterRow->filter == rawMinFilter && filterRow->mipFilter == rawMipFilter) {                      \
                gxMinFilter = filterRow->value;                                                                   \
                break;                                                                                            \
            }                                                                                                     \
        }                                                                                                         \
        if (gxMinFilter == kGxTrilinearMinFilter &&                                                               \
            TEXTURE_STAGE_FIELD(textureStageIndex, magFilter) == kAnisotropyEligibleMagFilter) {                  \
            switch (TEXTURE_STAGE_FIELD(textureStageIndex, anisotropy)) {                                         \
            case kAnisotropyRequest1:                                                                             \
            case kAnisotropyRequest3:                                                                             \
                break;                                                                                            \
            case kAnisotropyRequest2:                                                                             \
                gxAnisotropy = kGxAnisotropy2;                                                                    \
                break;                                                                                            \
            case kAnisotropyRequest4:                                                                             \
                gxAnisotropy = kGxAnisotropy4;                                                                    \
                break;                                                                                            \
            }                                                                                                     \
        }                                                                                                         \
        GXInitTexObjLOD(lodTextureObject, gxMinFilter, rawMagFilter, minLod, maxLod, lodBias, biasClamp, edgeLod, \
                        gxAnisotropy);                                                                            \
    }

// Load the initialized plane objects into consecutive GX map slots.
#define LOAD_TEXTURE_BINDING_PLANES()                                                        \
    for (loadPlane = 0; loadPlane < gxMapCount; ++loadPlane) {                               \
        loadTextureObject = (GXTexObj*)(MGR + ((firstGxMapSlot + loadPlane) << 5) + 0x16E0); \
        gxMapSlot = firstGxMapSlot + loadPlane;                                              \
        GXLoadTexObj(loadTextureObject, (GXTexMapID)gxMapSlot);                              \
    }

// Bind one engine texture stage; resources are borrowed and GX object cache storage is reused.
// The four phases preserve the native -O0 scratch slots and allocation-free binding order.
void Render_ApplyTextureStage(register int textureStageIndex) {
    register unsigned char* textureResourceBytes;
    __alloca(0);
    unsigned char* stageResourceBytes;
    const unsigned int* baseLevelWords;
    unsigned int firstGxMapSlot;
    unsigned char* imageAddress;
    unsigned int wrapS, wrapT, gxMapCount;
    unsigned int paletteGxFormat, firstGxPaletteSlot, palettePlane;
    GXTexObj* paletteTextureObject;
    unsigned int gxPaletteSlot;
    unsigned int unpalettedPlane;
    GXTexObj* plainTextureObject;
    unsigned int gxTextureFormat;
    unsigned int* lastLevelWords;
    int secondPlaneByteOffset;
    unsigned int lodPlane;
    GXTexObj* lodTextureObject;
    int gxMinFilter;
    unsigned int rawMagFilter;
    unsigned int gxAnisotropy;
    unsigned int rawMinFilter, rawMipFilter, filterRowIndex;
    struct FilterRow {
        unsigned int filter, mipFilter;
        int value;
    };
    FilterRow* filterRow;
    unsigned int loadPlane;
    GXTexObj* loadTextureObject;
    unsigned int gxMapSlot;
    unsigned int baseLevelDescription[9];
    struct FilterTable {
        FilterRow rows[6];
    };
    FilterTable minMipLookup;
    unsigned short width, height;
    unsigned char mipmap, biasClamp, edgeLod;
    float minLod, maxLod, lodBias;
    unsigned char* resourceStorage;
    unsigned int levelZeroImageOffset;
    unsigned int resourceKind, firstGxMapSlotReply;
    unsigned char* baseImageAddress;
    unsigned char* imageAddressCopy;
    unsigned int mapCountReply, paletteSlotReply;
    unsigned char hasMipmapsReply;
    unsigned int splitPlaneGxFormat;
    unsigned char* secondPlaneImageAddress;

    stageResourceBytes = *((unsigned char**)&((RendererStateView*)MGR)->stageResources[textureStageIndex]);
    if (stageResourceBytes != 0) {
        // A resource kind other than direct/wrapped leaves textureResourceBytes undefined: the original dereferences a
        // stale register.
        RESOLVE_TEXTURE_BINDING_RESOURCE();
        INITIALIZE_TEXTURE_BINDING_PLANES();
        CONFIGURE_TEXTURE_BINDING_LOD();
        LOAD_TEXTURE_BINDING_PLANES();
    }
}
#undef RESOLVE_TEXTURE_BINDING_RESOURCE
#undef INITIALIZE_TEXTURE_BINDING_PLANES
#undef CONFIGURE_TEXTURE_BINDING_LOD
#undef LOAD_TEXTURE_BINDING_PLANES

#pragma pop

#pragma push
#pragma optimization_level 0
// Enable the second GX color channel when the render-state cache requests it.
void Render_ApplyLightingChannelCount(void) {
    register int r31 = 1;
    if (((EngineState*)MGR)->renderStates[31] != 0) {
        r31 = r31 + 1;
    }
    GXSetNumChans((unsigned char)r31);
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma optimize_for_size on
// Render the six source faces of a wrapped texture-stage resource.
void Render_UpdateSixFaceTextureStage(int index) {
    if (0)
        __alloca(0);
    register void* obj = *((void**)&((RendererStateView*)MGR)->stageResources[index]);
    if (obj != 0) {
        register int kind = (*(unsigned int*)obj >> 13) & 7;
        if (kind == 4) {
            register void* saved = obj;
            SixFaceRenderer_RenderToTexture(((RenderSurfaceWrapperView*)saved)->texture,
                                            ((RenderSurfaceWrapperView*)saved)->sourceFaces);
        }
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma optimize_for_size on
#pragma dont_inline on
#pragma opt_lifetimes off
// Copy the current framebuffer into a render surface and queue the texture-copy sync token.
void RenderSurface_CopyFramebufferToTexture(register void* surface) {
    // GetDataAndPitch returns byte pitch and a borrowed image pointer (32-bit Wii).
    ImgInfo_804251B8 surfaceStorage;
    TexInfo_804251B8 description;
    RenderSurface_GetDescription(surface, &description);
    RenderSurface_GetDataAndPitch(surface, &surfaceStorage, 0, 0);
    register unsigned int* viewportWordsThenZero = (unsigned int*)((char*)&((RendererStateView*)MGR)->viewport);
    register unsigned int gxTextureFormat = description.fmt & 0x7FFFFFFFu;
    register unsigned int copyWidth = (unsigned short)((DrawViewport*)viewportWordsThenZero)->w;
    register unsigned int copyHeight = (unsigned short)((DrawViewport*)viewportWordsThenZero)->h;
    // The paired width/height assignments and later zero reuse are retained even
    // though GX copy dimensions below come from the full surface description.
    if (description.width < ((DrawViewport*)viewportWordsThenZero)->w ||
        description.height < ((DrawViewport*)viewportWordsThenZero)->h) {
        copyWidth = (unsigned short)description.width;
        copyHeight = (unsigned short)description.height;
        *((int*)&((RendererStateView*)MGR)->viewportDirty) = 1;
    }
    GXSetTexCopySrc(0, 0, (unsigned short)description.width, (unsigned short)description.height);
    viewportWordsThenZero = 0;
    GXSetTexCopyDst((unsigned short)description.width, (unsigned short)description.height, gxTextureFormat,
                    (int)viewportWordsThenZero);
    GXCopyTex(surfaceStorage.image, 0);
    GraphicsResource_NoOpSuccess(surface);
    GXSetDrawSync(GpuSync_TextureCopy);
    GXPixModeSync();
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Draw a render surface as a full-screen color or depth texture.
void RenderSurface_DrawFullscreen(void* surface) {
    if (0)
        __alloca(0);
    struct {
        TexInfo_804251B8 texinfo;
        FullscreenQuadVertices desc;
    } surfaceAndQuad;
    Mtx44 screenProjection;
    GXTexObj textureObject;
    Scene_804251B8 scene;
    ImgInfo_804251B8 surfaceData;
    unsigned int index;
    unsigned char tevStageCount;
    unsigned int i;
    VertexAttributeDesc* entry;
    char* viewportBytes;
    int gxTextureFormat;

    RenderSurface_GetDescription(surface, &surfaceAndQuad.texinfo);
    RenderSurface_GetDataAndPitch(surface, &surfaceData, 0, 0);
    gxTextureFormat = surfaceAndQuad.texinfo.fmt & 0x7FFFFFFF;
    GXInitTexObj(&textureObject, surfaceData.image, (u16)surfaceAndQuad.texinfo.width,
                 (u16)surfaceAndQuad.texinfo.height, (GXTexFmt)gxTextureFormat, kGxTextureClamp, kGxTextureClamp, 0);
    GXInitTexObjLOD(&textureObject, kGxNearestFilter, kGxNearestFilter, kFloat_0_GXRenderer, kFloat_0_GXRenderer,
                    kFloat_0_GXRenderer, 0, 0, 0);
    GXLoadTexObj(&textureObject, 0);
    GraphicsResource_NoOpSuccess(surface);
    ((EngineState*)MGR)->textureStageDirty[0] = 1;
    GXSetCurrentMtx(0);
    viewportBytes = (char*)&((RendererStateView*)MGR)->viewport;
    {
        float zmax = kDepth24Maximum_GXRenderer;
        MtxOrthoOffCenter((float*)screenProjection, (float)*(unsigned int*)(viewportBytes + kDrawViewport_x),
                          (float)*(unsigned int*)(viewportBytes + kDrawViewport_w),
                          (float)*(unsigned int*)(viewportBytes + kDrawViewport_h),
                          (float)*(unsigned int*)(viewportBytes + kDrawViewport_y),
                          *(float*)(viewportBytes + kDrawViewport_nearz),
                          *(float*)(viewportBytes + kDrawViewport_farz));
    }
    screenProjection[2][2] *= kDepthAxisFlip_GXRenderer;
    PSMTX44Transpose(screenProjection, screenProjection);
    GXSetProjection(screenProjection, kGxOrthographicProjection);
    AsEngineState(MGR)->transformDirty[1] = 1;
    GXLoadPosMtxImm(kIdentityMatrix4x4, 0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(0, kGxTexGenMatrix2x4, kGxTexCoord0, kGxIdentityTextureMatrix, 0, kGxIdentityPostMatrix);
    AsEngineState(MGR)->transformDirty[2] = 1;
    GXSetNumIndStages(0);
    for (int i = 0; i < 16; i++)
        GXSetTevDirect(i);
    tevStageCount = 1;
    if (gxTextureFormat == kGxDepth24Texture) {
        GXSetZMode(1, kGxCompareAlways, 1);
        GXSetZTexture(kGxDepthTextureReplace, kGxDepth24Texture, 0);
        GXSetColorUpdate(0);
        GXSetAlphaUpdate(0);
        ((EngineState*)MGR)->alphaWriteStateDirty = ((EngineState*)MGR)->colorWriteStateDirty = 1;
        GXSetZCompLoc(0);
        GXSetAlphaCompare(kGxCompareAlways, 0, 0, kGxCompareAlways, 0);
        ((EngineState*)MGR)->alphaCompareStateDirty = 1;
        GXLoadTexObj((GXTexObj*)s_fullscreenDepthAuxTexture, 1);
        ((EngineState*)MGR)->textureStageDirty[1] = 1;
        tevStageCount = 2;
        GXSetTevOrder(0, 0, 1, kGxNullColorChannel);
        GXSetTevColorIn(0, kGxTevColorZero, kGxTevColorZero, kGxTevColorZero, kGxTevTextureColor);
        GXSetTevColorOp(0, 0, 0, 0, 1, 0);
        GXSetTevAlphaIn(0, kGxTevAlphaZero, kGxTevAlphaZero, kGxTevAlphaZero, kGxTevTextureAlpha);
        GXSetTevAlphaOp(0, 0, 0, 0, 1, 0);
        GXSetTevOrder(1, 0, 0, kGxNullColorChannel);
        GXSetTevColorIn(1, kGxTevColorZero, kGxTevColorZero, kGxTevColorZero, 0);
        GXSetTevColorOp(1, 0, 0, 0, 1, 0);
        GXSetTevAlphaIn(1, kGxTevAlphaZero, kGxTevAlphaZero, kGxTevAlphaZero, 0);
        GXSetTevAlphaOp(1, 0, 0, 0, 1, 0);
    } else {
        GXSetZMode(1, kGxCompareAlways, 0);
        GXSetTevOrder(0, 0, 0, kGxNullColorChannel);
        GXSetTevColorIn(0, kGxTevColorZero, kGxTevColorZero, kGxTevColorZero, kGxTevTextureColor);
        GXSetTevColorOp(0, 0, 0, 0, 1, 0);
        GXSetTevAlphaIn(0, kGxTevAlphaZero, kGxTevAlphaZero, kGxTevAlphaZero, kGxTevTextureAlpha);
        GXSetTevAlphaOp(0, 0, 0, 0, 1, 0);
    }
    GXSetNumTevStages(tevStageCount);
    if (((EngineState*)MGR)->type7Resource == 0)
        s_defaultTevStateDirty = 1;
    GXSetNumChans(0);
    ((EngineState*)MGR)->lightingChannelStateDirty = 1;
    if (((RendererStateView*)MGR)->videoMode.field_rendering) {
        {
            GXSetViewportJitter(kFloat_0_GXRenderer, kFloat_0_GXRenderer, (float)surfaceAndQuad.texinfo.width,
                                (float)surfaceAndQuad.texinfo.height, *(float*)(viewportBytes + kDrawViewport_nearz),
                                *(float*)(viewportBytes + kDrawViewport_farz), VIGetNextField());
        }
    } else {
        {
            GXSetViewport(kFloat_0_GXRenderer, kFloat_0_GXRenderer, (float)surfaceAndQuad.texinfo.width,
                          (float)surfaceAndQuad.texinfo.height, *(float*)(viewportBytes + kDrawViewport_nearz),
                          *(float*)(viewportBytes + kDrawViewport_farz));
        }
    }
    *((int*)&((RendererStateView*)MGR)->viewportDirty) = 1;
    surfaceAndQuad.desc = *(FullscreenQuadVertices*)s_fullscreenSurfaceVertices;
    *(float*)((char*)&surfaceAndQuad.desc + 0x14) = (float)surfaceAndQuad.texinfo.width;
    *(float*)((char*)&surfaceAndQuad.desc + 0x28) = (float)surfaceAndQuad.texinfo.width;
    *(float*)((char*)&surfaceAndQuad.desc + 0x2c) = (float)surfaceAndQuad.texinfo.height;
    *(float*)((char*)&surfaceAndQuad.desc + 0x40) = (float)surfaceAndQuad.texinfo.height;
    VertexInput_FindAndCopyFormatDescriptor(&scene, kFullscreenVertexFormat);
    GXClearVtxDesc();
    for (i = 0; i < scene.count; i++) {
        index = scene.list[i].id;
        GXSetVtxDesc(index, 1);
        entry = &scene.list[i];
        GXSetVtxAttrFmt(0, entry->id, entry->componentCount, entry->componentType,
                        (unsigned char)entry->fractionalBits);
    }
    GXBegin(kGxDrawQuads, 0, kFullscreenQuadVertexCount);
    scene.callback(&surfaceAndQuad.desc, 4, 0);
    ((EngineState*)MGR)->depthStateDirty = 1;
    if (gxTextureFormat == kGxDepth24Texture) {
        GXSetZTexture(kGxDepthTextureDisabled, kGxDepth24Texture, 0);
        GXSetZCompLoc(1);
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
// Flush pending offscreen color/depth copies and texture projection state.
void Engine_UpdateOffscreenRenderTarget(void) {
    if (0)
        __alloca(0);
    int i;
    if (((EngineState*)MGR)->offscreenRenderingEnabled != 0) {
        if (((EngineState*)MGR)->renderTargetRequiresCopy != 0)
            RenderSurface_CopyFramebufferToTexture(((EngineState*)MGR)->offscreenRenderTarget);
        if (((EngineState*)MGR)->activeType6ResourceTypes[4] != 0) {
            void* offscreenTarget = ((EngineState*)MGR)->offscreenRenderTarget;
            unsigned int n;
            void* currentSurface;
            currentSurface = *((void**)&((RendererStateView*)MGR)->currentRenderSurface);
            unsigned int r[2];
            TexInfo_804251B8 q;
            TexInfo_804251B8 p;
            RenderSurface_GetDescription(offscreenTarget, &p);
            RenderSurface_GetDescription(currentSurface, &q);
            n = 19;
            RenderSurface_GetDataAndPitch(offscreenTarget, r, 0, 0);
            GXSetTexCopySrc(0, 0, (unsigned short)q.width, (unsigned short)q.height);
            i = 0;
            GXSetTexCopyDst((unsigned short)p.width, (unsigned short)p.height, n, i);
            GXCopyTex((void*)r[1], 0);
            GraphicsResource_NoOpSuccess(offscreenTarget);
            GXPixModeSync();
        }
        for (i = 0; i < 7; i++) {
            if (AsEngineState(MGR)->activeType6ResourceTypes[i] == 0)
                continue;
            switch (i) {
            case 5:
                Engine_ApplySelectedTextureStageState();
                break;
            }
        }
    }
}
#pragma pop
#pragma optimization_level 4

#pragma push
#pragma optimization_level 0
#pragma optimize_for_size on
// Decrement the packed 16-bit reference count and destroy the resource at zero.
int GraphicsResource_Release(register void* object) {
    --((Bits_80425930*)object)->high;
    if (((Bits_80425930*)object)->high == 0) {
        CubeResourceStorage_dtor(object, 1);
        return 0;
    }
    return ((Bits_80425930*)object)->high;
}
#pragma pop

#pragma optimization_level 0
#pragma optimize_for_size on
// Borrow a cache-array element with its declared type and stride. Shift the base
// before selecting the member: MWCC -O0 emits a different address sequence for
// ordinary field[index]. index is evaluated once; no object is constructed here.
#define RENDERER_SLOT(field, index) \
    (((RendererStateView*)((char*)g_engineState + (index) * sizeof(((RendererStateView*)0)->field[0])))->field[0])

#pragma push
#pragma always_inline on
// Consume vertex descriptor, vertex-array and index-header dirty flags in that order.
static inline void applyDirtyDrawInputs() {
    if (((RendererStateView*)g_engineState)->drawStateDirty) {
        Render_ApplyVertexInputState();
        ((RendererStateView*)g_engineState)->drawStateDirty = 0;
    }
    if (((RendererStateView*)g_engineState)->vertexArraysDirty) {
        Engine_UpdateVertexArrays();
        ((RendererStateView*)g_engineState)->vertexArraysDirty = 0;
    }
    if (((RendererStateView*)g_engineState)->indexBufferHeaderDirty) {
        Engine_UpdateIndexBufferHeader();
        ((RendererStateView*)g_engineState)->indexBufferHeaderDirty = 0;
    }
}
#pragma pop

// Apply dirty vertex, lighting, texture, matrix and viewport caches before submitting a draw.
// Ordering contract: special texture producers -> vertex input/buffers -> material and
// lights -> ten render-state callbacks -> palettes/textures -> TEV -> texgen/matrices
// -> projection/model-view/normal matrices -> viewport. Earlier calls can invalidate
// later domains; do not reorder by dirty-flag address. Each consumer clears only its
// own flag after upload. Disabled lighting leaves lightStateDirty pending. The early
// six-face pass observes textureStageDirty without clearing it; binding happens later.
// The field-derived byte addresses below alias EngineState arrays; 4/12/64/0x68 are
// strides of dirty words, light colors, GX light objects and source DrawLight records.
// Consume dirty caches in dependency order: producers, inputs, lights, bindings, matrices, viewport.
// Scratch is declared once in upload order; phase comments below identify each consumer.
// Earlier uploads can dirty later caches. Each flag is cleared only after its GX upload.
void Engine_ApplyStateBeforeDraw() {
    char secondLight[64];
    DrawTable11 textureMatrixIds;
    Mtx objectViewNormalMatrix;
    Mtx44 objectViewPositionMatrix, objectViewCombined;
    Mtx viewNormalMatrix;
    Mtx44 viewPositionMatrix, viewAffine, objectAffine;
    Mtx objectNormal, objectPosition;
    Mtx44 projectionMatrix, orthoMatrix, coordinatePostMatrix, coordinateBiasMatrix, coordinateScaleMatrix,
        textureMatrix;
    unsigned int targetDescription[9];
    DrawVec secondDirection, pointPosition, spotPosition, spotDirection, sunDirection, sunPosition;
    DrawProjection projectionParameters;
    DrawTable4 specialTextureMatrixIds, specialTexCoordSources;
    /* Phase 1: refresh texture producers before consuming their dirty stage bindings. */
    /* Explicit function-address casts preserve calls that MWCC otherwise auto-inlines. */
    for (int i = 0; i < 7; ++i) {
        if (RENDERER_SLOT(type6Active, i)) {
            switch (i) {
            case kRadialCaptureTexture:
                Render_UpdateRadialCapture();
                break;
            case kProjectedTexture:
                Engine_UpdateTextureProjectionMatrices();
                break;
            case kPairedProjectionTexture:
                TextureProjection_ApplyTwoStageTransforms();
                break;
            }
        }
    }
    for (unsigned int i = 0; i < 8; ++i)
        if (RENDERER_SLOT(textureStageDirty, i))
            ((void (*)(int))Render_UpdateSixFaceTextureStage)(i);
    /* Phase 2: select the vertex descriptor, then refresh vertex and index storage. */
    applyDirtyDrawInputs();
    /* Phase 3: upload material channel colors, then transform world-space lights by */
    /* the view matrix. Paired specular mode consumes two GX light slots per source. */
    if (((RendererStateView*)g_engineState)->materialColorDirty) {
        if (((RendererStateView*)g_engineState)->useDefaultAmbient) {
            DrawColor channelColor = kDefaultAmbientColor;
            GXSetChanAmbColor(kGxColor0Alpha0, channelColor);
        } else
            GXSetChanAmbColor(kGxColor0Alpha0, *(DrawColor*)&((RendererStateView*)g_engineState)->materialColors[1]);
        DrawColor channelColor;
        if (!(*(unsigned int*)&((EngineState*)g_engineState)->renderStates[kLightingEnabledState])) {
            channelColor.r = 0;
            channelColor.g = 0;
            channelColor.b = 0;
            if (!((*(unsigned int*)&((EngineState*)g_engineState)->vertexFormat) & kVertexColorInputMask)) {
                channelColor.r = 255;
                channelColor.g = 255;
                channelColor.b = 255;
            }
        } else {
            channelColor.r = 255;
            channelColor.g = 255;
            channelColor.b = 255;
        }
        if (!(*(unsigned int*)&((EngineState*)g_engineState)->renderStates[kLightingEnabledState]) &&
            !((*(unsigned int*)&((EngineState*)g_engineState)->vertexFormat) & kVertexColorInputMask))
            channelColor.a = 255;
        else
            channelColor.a = ((RendererStateView*)g_engineState)->materialColors[2].a;
        GXSetChanMatColor(kGxColor0Alpha0, channelColor);
        if ((*(unsigned int*)&((EngineState*)g_engineState)->renderStates[kPairedSpecularLightsState])) {
            DrawColor blackAmbient;
            blackAmbient.r = 0;
            blackAmbient.g = 0;
            blackAmbient.b = 0;
            blackAmbient.a = 0;
            GXSetChanAmbColor(kGxColor1Alpha1, blackAmbient);
            DrawColor specularMaterialColor;
            /* The original leaves this color untouched when lighting is disabled. */
            if ((*(unsigned int*)&((EngineState*)g_engineState)->renderStates[kLightingEnabledState])) {
                specularMaterialColor.r = 255;
                specularMaterialColor.g = 255;
                specularMaterialColor.b = 255;
                specularMaterialColor.a = 255;
            }
            GXSetChanMatColor(kGxColor1Alpha1, specularMaterialColor);
        }
        ((RendererStateView*)g_engineState)->materialColorDirty = 0;
    }
    for (unsigned int lightIndex = 0; lightIndex < 8; ++lightIndex) {
        if (!(*(unsigned int*)&((EngineState*)g_engineState)->renderStates[kLightingEnabledState]))
            break;
        if (RENDERER_SLOT(lightStateDirty, lightIndex)) {
            void* gxLightObject = RENDERER_SLOT(lightObjects, lightIndex);
            PackedRendererColor* materialColorSet = &((RendererStateView*)g_engineState)->materialColors[0];
            PackedRendererColor* lightColorSet = RENDERER_SLOT(lightColors, lightIndex);
            DrawLight* sourceLight = &RENDERER_SLOT(lights, lightIndex);
            if (RENDERER_SLOT(lightEnabled, lightIndex)) {
                DrawColor channelColor;
                if (!(*(unsigned int*)&((EngineState*)g_engineState)->renderStates[kLightColorWithoutMaterialState]))
                    Color_MultiplyRGBA8(&channelColor, lightColorSet, materialColorSet);
                else {
                    ((void (*)(void*, const void*))Color_CopyRGBA8)(&channelColor, lightColorSet);
                }
                channelColor.a = 255;
                GXInitLightColor(gxLightObject, channelColor);
                if (sourceLight->type == kDirectionalLight) {
                    Vec3_Normalize(&sunDirection, &sourceLight->direction);
                    Vec3_Scale(&sunPosition, &sunDirection, kFloat_neg_100000_GXRenderer);
                    Mtx44_TransformPointPerspective(&sunPosition, &sunPosition,
                                                    AsEngineState(g_engineState)->transforms[kViewTransform]);
                    Mtx44_TransformDirection(&sunDirection, &sunDirection,
                                             AsEngineState(g_engineState)->transforms[kViewTransform]);
                    GXInitLightPos(gxLightObject, sunPosition.x, sunPosition.y, sunPosition.z);
                    GXInitLightSpot(gxLightObject, kFloat_90_GXRenderer, 0);
                    GXInitLightDistAttn(gxLightObject, kFloat_100000_GXRenderer, kFloat_0_99_GXRenderer, 0);
                } else if (sourceLight->type == kSpotLight) {
                    Vec3_Normalize(&spotDirection, &sourceLight->direction);
                    Mtx44_TransformPointPerspective(&spotPosition, &sourceLight->position,
                                                    AsEngineState(g_engineState)->transforms[kViewTransform]);
                    Mtx44_TransformDirection(&spotDirection, &spotDirection,
                                             AsEngineState(g_engineState)->transforms[kViewTransform]);
                    GXInitLightPos(gxLightObject, spotPosition.x, spotPosition.y, spotPosition.z);
                    GXInitLightDir(gxLightObject, spotDirection.x, spotDirection.y, spotDirection.z);
                    float spotAngle =
                        kRadiansToDegrees_GXRenderer * ((sourceLight->end + sourceLight->start) * kFloat_0_5_Light);
                    GXInitLightSpot(gxLightObject, spotAngle, 2);
                    if (((RendererStateView*)g_engineState)->useDistanceAttenuation)
                        GXInitLightDistAttn(gxLightObject, sourceLight->spot, kFloat_0_01_GXRenderer, 0);
                    else
                        GXInitLightAttnK(gxLightObject, sourceLight->a0, sourceLight->a1, sourceLight->a2);
                } else if (sourceLight->type == kPointLight) {
                    Mtx44_TransformPointPerspective(&pointPosition, &sourceLight->position,
                                                    AsEngineState(g_engineState)->transforms[kViewTransform]);
                    GXInitLightPos(gxLightObject, pointPosition.x, pointPosition.y, pointPosition.z);
                    GXInitLightSpot(gxLightObject, kFloat_90_GXRenderer, 0);
                    if (((RendererStateView*)g_engineState)->useDistanceAttenuation)
                        GXInitLightDistAttn(gxLightObject, sourceLight->spot, kFloat_0_01_GXRenderer, 0);
                    else
                        GXInitLightAttnK(gxLightObject, sourceLight->a0, sourceLight->a1, sourceLight->a2);
                }
                unsigned int gxSlotsPerLight = 1;
                if ((*(unsigned int*)&((EngineState*)g_engineState)->renderStates[kPairedSpecularLightsState]))
                    gxSlotsPerLight = 2;
                unsigned int gxLightIndex = gxSlotsPerLight * lightIndex;
                unsigned int firstGxLightSlot = gxLightIndex;
                unsigned int diffuseLightMask = 1 << firstGxLightSlot;
                GXLoadLightObjImm(gxLightObject, diffuseLightMask);
                if ((*(unsigned int*)&((EngineState*)g_engineState)->renderStates[kPairedSpecularLightsState])) {
                    DrawColor channelColor;
                    if (!(*(unsigned int*)&((EngineState*)g_engineState)
                               ->renderStates[kSpecularColorWithoutMaterialState]))
                        Color_MultiplyRGBA8(&channelColor, &lightColorSet[2], &materialColorSet[2]);
                    else {
                        ((void (*)(void*, const void*))Color_CopyRGBA8)(&channelColor, &lightColorSet[2]);
                    }
                    GXInitLightColor(secondLight, channelColor);
                    Vector_CopyComponents(&secondDirection, &sourceLight->direction);
                    Vec3_Normalize(&secondDirection, &secondDirection);
                    Mtx44_TransformDirection(&secondDirection, &secondDirection,
                                             AsEngineState(g_engineState)->transforms[kViewTransform]);
                    GXInitSpecularDir(secondLight, secondDirection.x, secondDirection.y, secondDirection.z);
                    float specularAttenuation;
                    if (kFloat_0_GXRenderer == ((RendererStateView*)g_engineState)->specularPower)
                        specularAttenuation = kFloatMax_GXRenderer;
                    else
                        specularAttenuation =
                            kSpecularAttenuationScale_GXRenderer / (((RendererStateView*)g_engineState)->specularPower *
                                                                    ((RendererStateView*)g_engineState)->specularPower);
                    GXInitLightAttn(secondLight, kFloat_0_GXRenderer, kFloat_0_GXRenderer, kFloat_1_Renderer,
                                    specularAttenuation * kFloat_0_5_Light, kFloat_0_GXRenderer,
                                    kFloat_1_Renderer - specularAttenuation * kFloat_0_5_Light);
                    unsigned int specularLightMask = 1 << (firstGxLightSlot + 1);
                    GXLoadLightObjImm(secondLight, specularLightMask);
                }
            }
            RENDERER_SLOT(lightStateDirty, lightIndex) = 0;
        }
    }
    /* Phase 4: consume render-state groups, palettes, texture stages and TEV in order. */
    for (int i = 0; i < 10; ++i) {
        if (RENDERER_SLOT(stateDirty, i)) {
            s_dirtyRenderStateCallbacks[i]();
            RENDERER_SLOT(stateDirty, i) = 0;
        }
    }
    for (unsigned int i = 0; i < 8; ++i) {
        if (RENDERER_SLOT(paletteDirty, i)) {
            Render_LoadTexturePalette(i);
            RENDERER_SLOT(paletteDirty, i) = 0;
        }
    }
    for (unsigned int i = 0; i < 8; ++i) {
        if (RENDERER_SLOT(textureStageDirty, i)) {
            Render_ApplyTextureStage(i);
            RENDERER_SLOT(textureStageDirty, i) = 0;
        }
    }
    Render_ApplyTevState();
    /* Phase 5: map texgen sources, then upload only dirty texture-transform slots. */
    /* Modes 0x80020000/0x80010000/0x80030000 derive coordinates from position/normal */
    /* inputs. Keep the written concat/transpose/inverse order: it bridges the engine */
    /* matrix convention to GX; inverse-transpose is required for normal-based inputs. */
    unsigned int texCoordCount = VertexInput_CountTextureCoordinateGenerators();
    GXSetNumTexGens((unsigned char)texCoordCount);
    unsigned int usesSpecialTexCoordMapping = 0;
    if ((*(unsigned int*)&((EngineState*)g_engineState)->usesVertexDeclaration))
        usesSpecialTexCoordMapping = VertexDeclaration_UsesSpecialTexCoordMapping(
            (*(unsigned int*)&((EngineState*)g_engineState)->vertexDeclaration));
    for (unsigned int textureStageIndex = 0; textureStageIndex < texCoordCount; ++textureStageIndex) {
        unsigned int gxTexCoordId = textureStageIndex;
        int gxTexGenType = 1;
        unsigned int gxTexCoordSource = textureStageIndex + 4;
        textureMatrixIds = s_texCoordMatrixIds;
        unsigned int gxTextureMatrixId = textureMatrixIds.v[textureStageIndex];
        int gxTextureMatrixType = 1;
        if ((unsigned char)DRAW_TEXTURE_STAGE_FIELD(textureStageIndex, transformDimensions) == 3) {
            gxTexGenType = 0;
            gxTextureMatrixType = 0;
        }
        unsigned int coordinateSourceMode = DRAW_TEXTURE_STAGE_FIELD(textureStageIndex, coordinateSource);
        if (coordinateSourceMode == kTexCoordsFromPosition)
            gxTexCoordSource = 0;
        else if (coordinateSourceMode == kTexCoordsFromNormal ||
                 coordinateSourceMode == kTexCoordsFromNormalWithPostMatrix)
            gxTexCoordSource = 1;
        else {
            unsigned int sourceTexCoordIndex = coordinateSourceMode & 15;
            if (coordinateSourceMode != kNoGeneratedTexCoord && sourceTexCoordIndex != textureStageIndex)
                gxTexCoordSource = sourceTexCoordIndex + 4;
        }
        if (usesSpecialTexCoordMapping) {
            specialTexCoordSources = s_specialTexCoordSources;
            specialTextureMatrixIds = s_specialTexCoordMatrixIds;
            gxTexCoordSource = specialTexCoordSources.v[textureStageIndex];
            gxTextureMatrixId = specialTextureMatrixIds.v[textureStageIndex];
        }
        GXSetTexCoordGen2(gxTexCoordId, gxTexGenType, gxTexCoordSource, gxTextureMatrixId, 0, 125);
        unsigned int transformSlot = textureStageIndex + 2;
        if (DRAW_TRANSFORM_DIRTY(transformSlot)) {
            if (coordinateSourceMode == kTexCoordsFromPosition) {
                PSMTX44Concat(textureMatrix, AsEngineState(g_engineState)->transforms[kObjectTransform],
                              AsEngineState(g_engineState)->transforms[kViewTransform]);
                PSMTX44Concat(textureMatrix, textureMatrix, DRAW_TRANSFORM_MATRIX(transformSlot));
                PSMTX44Transpose(textureMatrix, textureMatrix);
            } else if (coordinateSourceMode == kTexCoordsFromNormal) {
                PSMTX44Concat(textureMatrix, AsEngineState(g_engineState)->transforms[kViewTransform],
                              AsEngineState(g_engineState)->transforms[kObjectTransform]);
                PSMTX44Concat(textureMatrix, textureMatrix, DRAW_TRANSFORM_MATRIX(transformSlot));
                PSMTX44Transpose(textureMatrix, textureMatrix);
                Mtx44_InvertLegacyCofactors(textureMatrix, 0, textureMatrix);
                PSMTX44Transpose(textureMatrix, textureMatrix);
            } else if (coordinateSourceMode == kTexCoordsFromNormalWithPostMatrix) {
                PSMTX44Concat(textureMatrix, AsEngineState(g_engineState)->transforms[kObjectTransform],
                              AsEngineState(g_engineState)->transforms[kViewTransform]);
                PSMTX44Transpose(textureMatrix, textureMatrix);
                Mtx44_InvertLegacyCofactors(textureMatrix, 0, textureMatrix);
                PSMTX44Transpose(textureMatrix, textureMatrix);
                Mtx44_SetScale(coordinateScaleMatrix, kFloat_0_5_Light, kFloat_neg_0_5_GXRenderer, kFloat_0_GXRenderer);
                Mtx44_SetTranslation(coordinateBiasMatrix, kFloat_0_5_Light, kFloat_0_5_Light, kFloat_1_Renderer);
                PSMTX44Concat(coordinatePostMatrix, coordinateScaleMatrix, coordinateBiasMatrix);
                PSMTX44Transpose(coordinatePostMatrix, coordinatePostMatrix);
                GXLoadTexMtxImm(coordinatePostMatrix, 64, 0);
                GXSetTexCoordGen2(0, 0, 1, 30, 1, 64);
            } else
                PSMTX44Transpose(textureMatrix, DRAW_TRANSFORM_MATRIX(transformSlot));
            GXLoadTexMtxImm(textureMatrix, gxTextureMatrixId, gxTextureMatrixType);
            DRAW_TRANSFORM_DIRTY(transformSlot) = 0;
        }
    }
    /* Phase 6: format 13 uses identity clip-space projection; format 5 uses screen */
    /* orthographic projection. Other formats use projection plus view/object matrices. */
    /* Normal matrices are inverse-transposes of the matching position transform. */
    if (((*(unsigned int*)&((EngineState*)g_engineState)->vertexFormat) & kVertexPositionFormatMask) ==
        kClipSpacePositionFormat) {
        GXSetCurrentMtx(0);
        GXSetProjection(kIdentityMatrix4x4, 1);
        GXLoadPosMtxImm(kIdentityMatrix4x4, 0);
    } else if (((*(unsigned int*)&((EngineState*)g_engineState)->vertexFormat) & kVertexPositionFormatMask) ==
               kScreenSpacePositionFormat) {
        GXSetCurrentMtx(0);
        DrawViewport* viewport = &((RendererStateView*)g_engineState)->viewport;
        float unusedDepthMaximum = kDepth24Maximum_GXRenderer; /* Retained load in the original orthographic path. */
        MtxOrthoOffCenter(orthoMatrix, (float)viewport->x, (float)viewport->w, (float)viewport->h, (float)viewport->y,
                          viewport->nearz, viewport->farz);
        orthoMatrix[2][2] *= kDepthAxisFlip_GXRenderer;
        PSMTX44Transpose(orthoMatrix, orthoMatrix);
        GXSetProjection(orthoMatrix, 1);
        GXLoadPosMtxImm(kIdentityMatrix4x4, 0);
        (*(unsigned int*)&AsEngineState(g_engineState)->transformDirty[11]) = 1;
    } else {
        if ((*(unsigned int*)&AsEngineState(g_engineState)->transformDirty[1])) {
            PSMTX44Transpose(projectionMatrix, AsEngineState(g_engineState)->transforms[kProjectionTransform]);
            int gxProjectionType;
            if (kFloat_0_GXRenderer == projectionMatrix[3][3]) {
                gxProjectionType = 0;
                float* projectionElements = (float*)AsEngineState(g_engineState)->transforms[kProjectionTransform];
                float verticalScaleReciprocal = kFloat_1_Renderer / projectionElements[5];
                float halfFovAtanRadians = (float)atan(verticalScaleReciprocal);
                float halfFovRadians = halfFovAtanRadians;
                projectionParameters.fov = kFloat_2_Renderer * halfFovRadians;
                projectionParameters.aspect = projectionElements[5] / projectionElements[0];
                float farToNearRatio;
                if (projectionElements[10] < kFloat_0_GXRenderer) {
                    farToNearRatio =
                        (projectionElements[10] - kFloat_1_Renderer) / (kFloat_1_Renderer + projectionElements[10]);
                    projectionParameters.nearz =
                        (projectionElements[14] - projectionElements[14] * farToNearRatio) / farToNearRatio;
                } else {
                    farToNearRatio =
                        (kFloat_1_Renderer + projectionElements[10]) / (projectionElements[10] - kFloat_1_Renderer);
                    projectionParameters.nearz =
                        (projectionElements[14] - projectionElements[14] * farToNearRatio) / farToNearRatio;
                }
                projectionParameters.farz = projectionParameters.nearz * farToNearRatio;
                projectionMatrix[2][2] =
                    projectionParameters.nearz / (projectionParameters.nearz - projectionParameters.farz);
            } else
                gxProjectionType = 1;
            GXSetProjection(projectionMatrix, gxProjectionType);
        }
        (*(unsigned int*)&AsEngineState(g_engineState)->transformDirty[1]) = 0;
        if ((*(unsigned int*)&((EngineState*)g_engineState)->renderStates[kViewMatrixOnlyState]) &&
            ((*(unsigned int*)&((EngineState*)g_engineState)->vertexFormat) & kIndexedObjectTransforms)) {
            Mtx44_TransposeToMtx34(viewAffine, AsEngineState(g_engineState)->transforms[kViewTransform]);
            for (unsigned int objectMatrixIndex = 0; objectMatrixIndex < 10; ++objectMatrixIndex) {
                if (DRAW_TRANSFORM_DIRTY(objectMatrixIndex + 11)) {
                    Mtx44_TransposeToMtx34(objectAffine, DRAW_TRANSFORM_MATRIX(objectMatrixIndex + 11));
                    PSMTXConcat(viewAffine, objectAffine, objectPosition);
                    PSMTXInvXpose(objectPosition, objectNormal);
                    GXLoadPosMtxImm(objectPosition, (int)objectMatrixIndex * 3);
                    GXLoadNrmMtxImm(objectNormal, (int)objectMatrixIndex * 3);
                    DRAW_TRANSFORM_DIRTY(objectMatrixIndex + 11) = 0;
                }
            }
        } else if ((*(unsigned int*)&((EngineState*)g_engineState)->renderStates[kViewMatrixOnlyState])) {
            GXSetCurrentMtx(0);
            PSMTX44Transpose(viewPositionMatrix, AsEngineState(g_engineState)->transforms[kViewTransform]);
            GXLoadPosMtxImm(viewPositionMatrix, 0);
            PSMTXInvXpose(viewPositionMatrix, viewNormalMatrix);
            GXLoadNrmMtxImm(viewNormalMatrix, 0);
        } else {
            GXSetCurrentMtx(0);
            PSMTX44Concat(objectViewCombined, AsEngineState(g_engineState)->transforms[kObjectTransform],
                          AsEngineState(g_engineState)->transforms[kViewTransform]);
            PSMTX44Transpose(objectViewPositionMatrix, objectViewCombined);
            GXLoadPosMtxImm(objectViewPositionMatrix, 0);
            PSMTXInvXpose(objectViewPositionMatrix, objectViewNormalMatrix);
            if (!s_normalMatrixOverrideActive)
                GXLoadNrmMtxImm(objectViewNormalMatrix, 0);
        }
    }
    /* Phase 7: clamp the viewport to an offscreen target and apply field jitter last. */
    if (((RendererStateView*)g_engineState)->viewportDirty) {
        DrawViewport* viewport = &((RendererStateView*)g_engineState)->viewport;
        unsigned int viewportWidth = viewport->w, viewportHeight = viewport->h;
        if ((*(unsigned int*)&((EngineState*)g_engineState)->renderTargetRequiresCopy)) {
            RenderSurface_GetDescription((void*)(*(unsigned int*)&((EngineState*)g_engineState)->offscreenRenderTarget),
                                         targetDescription);
            if (targetDescription[7] < viewport->w || targetDescription[8] < viewport->h) {
                viewportWidth = targetDescription[7];
                viewportHeight = targetDescription[8];
            }
        }
        if (((RendererStateView*)g_engineState)->videoMode.field_rendering) {
            GXSetViewportJitter((float)viewport->x, (float)viewport->y, (float)viewportWidth, (float)viewportHeight,
                                viewport->nearz, viewport->farz, VIGetNextField());
        } else
            GXSetViewport((float)viewport->x, (float)viewport->y, (float)viewportWidth, (float)viewportHeight,
                          viewport->nearz, viewport->farz);
        ((RendererStateView*)g_engineState)->viewportDirty = 0;
    }
}

#pragma optimization_level 4
#pragma optimize_for_size off

#pragma optimization_level 0
// Build the GX/VI render mode from presentation flags and the output rectangle.
// Contract: out/params/rect must be nonnull and rect->format must be 2..5.
// The legacy null-rect branch selects a TV code but still dereferences rect for
// dimensions; it does not provide a usable null-rect API. Unsupported formats
// leave tv uninitialized. All supported paths initialize dimensions, scan mode,
// field_rendering, aa and vfilter. sample_pattern is written only when type==0xB.
// Rectangle extents/origins are pixels; presentation width is centered within 720.
// No new defaults or validation are added: callers must satisfy this contract.
void Engine_BuildRenderMode(GXRenderModeObj* out, volatile EnginePresentParams* params, EngineViRect* rect) {
    int mode;
    if (params->flags & Presentation_Interlaced)
        mode = 0;
    else if (params->flags & Presentation_Progressive)
        mode = 2;
    else
        mode = 1;

    int tv;
    if (rect != 0) {
        switch (rect->format) {
        case 2:
            tv = 0;
            break;
        case 3:
            tv = 1;
            break;
        case 4:
            tv = 5;
            break;
        case 5:
            tv = 2;
            break;
        default:
            break;
        }
    } else if (params->tvFormat == 0) {
        tv = VIGetTvFormat();
    } else if (params->tvFormat == 60) {
        tv = 0;
    } else if (params->tvFormat == 50) {
        tv = 1;
    }

    out->tvInfo = (tv << 2) + mode;
    out->fbWidth = params->width;
    out->efbHeight = params->height;
    out->xfbHeight = params->height;
    out->viXOrigin = rect->x + ((0x2D0 - params->width) >> 1);
    out->viYOrigin = rect->y;
    out->viWidth = rect->width;
    out->viHeight = rect->height;

    switch (mode) {
    case 0:
        out->xfbMode = 1;
        out->viHeight = params->height;
        break;
    case 2:
        out->xfbMode = 0;
        out->viHeight = params->height;
        break;
    case 1:
        out->xfbMode = 0;
        out->viHeight = params->height * 2;
        break;
    }

    if (mode != 2) {
        out->field_rendering = params->flags & Presentation_FieldRendering;
        if (out->field_rendering) {
            out->xfbMode = 0;
            out->viHeight = out->xfbHeight * 2;
        }
    } else
        out->field_rendering = 0;

    if (params->type == 0xB) {
        out->aa = 1;
        for (unsigned int row = 0; row < 2; ++row)
            for (unsigned int col = 0; col < 12; ++col)
                out->sample_pattern[col][row] = 6;
    } else
        out->aa = 0;
    Render_BuildVerticalFilterWeights(out->vfilter, kFloat_0_GXRenderer);
}
#pragma optimization_level 4

// Reflect the chosen VI scan mode into the presentation flags.
void Presentation_ApplyViScanModeFlags(unsigned int* a, const unsigned int* b) {
    int state = *b & 3;
    switch (state) {
    case 0:
        ((EnginePresentParams*)a)->flags |= Presentation_Interlaced;
        break;
    case 2:
        ((EnginePresentParams*)a)->flags |= Presentation_Progressive;
        break;
    }
}

#pragma push
#pragma optimization_level 0
// Dispatch frame-boundary or one-shot texture-copy callbacks for known GPU tokens.
void Render_OnDrawSyncToken(short value) {
    if ((unsigned short)value == GpuSync_FrameBoundary) {
        (*(CallbackManager* volatile*)&g_engineState)
            ->callback((*(CallbackManager* volatile*)&g_engineState)->argument);
    } else if ((unsigned short)value == GpuSync_TextureCopy &&
               (*(CallbackManager* volatile*)&g_engineState)->pendingCallback != 0) {
        (*(CallbackManager* volatile*)&g_engineState)
            ->pendingCallback((*(CallbackManager* volatile*)&g_engineState)->pendingArgument);
        (*(CallbackManager* volatile*)&g_engineState)->pendingCallback = 0;
    }
}
#pragma optimization_level 4
#pragma optimize_for_size on
#pragma peephole on
// Invoke the registered draw-completion callback using the original manager reloads.
void Render_OnDrawDone() {
    CallbackManager* p = *(CallbackManager* volatile*)&g_engineState;
    CallbackManager* q = *(CallbackManager* volatile*)&g_engineState;
    volatile int result = ((int (*)(void*))p->callback)(q->argument);
}
#pragma pop
#pragma optimize_for_size off
#pragma optimization_level 4

#pragma push
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma optimization_level 0
#pragma push
#pragma always_inline on
// Seed TEV stage zero with KONST/ONE/raster color inputs and raster alpha.
static inline void initializeDefaultTevStage() {
    GXSetTevSwapModeTable((GXTevSwapSel)1, (GXTevColorChan)0, (GXTevColorChan)3, (GXTevColorChan)2, (GXTevColorChan)3);
    GXSetTevOrder((GXTevStageID)0, (GXTexCoordID)0xFF, (GXTexMapID)0xFF, (GXChannelID)4);
    GXSetTevKColorSel(0, 0xC);
    GXSetTevColorIn((GXTevStageID)0, (GXTevColorArg)0xF, (GXTevColorArg)0xE, (GXTevColorArg)0xC, (GXTevColorArg)0xA);
    GXSetTevColorOp((GXTevStageID)0, (GXTevOp)0, (GXTevBias)0, (GXTevScale)0, (GXBool)1, (GXTevRegID)0);
    GXSetTevAlphaIn((GXTevStageID)0, (GXTevAlphaArg)7, (GXTevAlphaArg)7, (GXTevAlphaArg)7, (GXTevAlphaArg)5);
    GXSetTevAlphaOp((GXTevStageID)0, (GXTevOp)0, (GXTevBias)0, (GXTevScale)0, (GXBool)1, (GXTevRegID)0);
}
#pragma pop

// Initialize pixel format, display-copy scaling, TEV color/alpha and texture matrices.
void Engine_InitializeGXPixelAndTevDefaults() {
    GXSetPixelFmt(0, 0);
    GXRenderModeObj* mode = (GXRenderModeObj*)((unsigned char*)&((RendererStateView*)g_engineState)->videoMode);
    GXSetDispCopyYScale((float)mode->xfbHeight / (float)mode->efbHeight);
    initializeDefaultTevStage();
    GXLoadTexMtxImm(kIdentityMatrix4x4, 0x1E, (GXTexMtxType)1);
    GXLoadTexMtxImm(kIdentityMatrix4x4, 0x21, (GXTexMtxType)1);
    GXLoadTexMtxImm(kIdentityMatrix4x4, 0x24, (GXTexMtxType)1);
    GXLoadTexMtxImm(kIdentityMatrix4x4, 0x27, (GXTexMtxType)1);
}
#pragma pop
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma optimization_level 4

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma peephole on
#pragma dont_inline on
// Initialize and bind the GX FIFO, installing optional frame-breakpoint tokens.
void Render_InitializeAndBindGXFifo(void* unused, int useGX, int initFifo) {
    __alloca(0);
    unsigned int mid;
    unsigned int lo;
    unsigned char buf[0x80];
    if (useGX != 0) {
        *(volatile unsigned int*)(g_engineState + 0x19EC) = 1;
    } else {
        *(volatile unsigned int*)(g_engineState + 0x19EC) = 1;
    }
    unsigned int size = *((unsigned int*)&((RendererStateView*)g_engineState)->fifoBufferSize) /
                        *((unsigned int*)&((RendererStateView*)g_engineState)->fifoDivisor);
    void* fifo = 0;
    if (initFifo != 0) {
        memset(*((void**)&((RendererStateView*)g_engineState)->fifoBuffer), 0,
               *((unsigned int*)&((RendererStateView*)g_engineState)->fifoBufferSize));
        fifo = GXInit(*((void**)&((RendererStateView*)g_engineState)->fifoBuffer), size);
    }
    if (useGX != 0) {
        GXSetDrawDone();
        if (initFifo != 0) {
            GXSetDrawSync(GpuSync_FrameBoundary);
            GXSetCPUFifo(fifo);
            GXSetGPFifo(fifo);
        }
        GXSetDrawSync(GpuSync_FrameBoundary);
        GXGetCPUFifo(buf);
        GXGetFifoPtrs(buf, &mid, &lo);
        GXEnableBreakPt(lo);
        GXSetDrawSync(GpuSync_FifoBreakpoint);
    } else {
        GXDisableBreakPt();
    }
}
#pragma pop
}

struct Texture804272F4 {
    char data[0x98];
    // Allocate wrapper storage using the registered graphics allocator.
    static void* operator new(size_t size) {
        return ((GraphicsAllocator*)g_graphicsMemoryCallbacks)->allocate(size);
    }
    // Release resource-wrapper storage through the graphics memory callback.
    static void operator delete(void* p) {
        GraphicsResource_Free(p);
    }
    // Construct the 2D texture resource with the forwarded format/dimension arguments.
    Texture804272F4(unsigned int widthPixels, unsigned int heightPixels, unsigned int mipLevels,
                    unsigned int layoutFlags, unsigned int format, unsigned int allocationMode) {
        Texture_Ctor2D(this, widthPixels, heightPixels, mipLevels, layoutFlags, format, allocationMode);
    }
};

#pragma push
#pragma optimization_level 0
#pragma peephole on
struct Texture80427CEC {
    char data[0x10];
    // Allocate wrapper storage using the registered graphics allocator.
    static void* operator new(size_t size) {
        return ((GraphicsAllocator*)g_graphicsMemoryCallbacks)->allocate(size);
    }
    // Release resource-wrapper storage through the graphics memory callback.
    static void operator delete(void* p) {
        GraphicsResource_Free(p);
    }
    // Initialize a type-6 graphics resource wrapper.
    Texture80427CEC(unsigned int value) {
        GraphicsResource_InitType6(this, value);
    }
};

struct Texture80427DA4 {
    char data[0x10];
    // Allocate wrapper storage using the registered graphics allocator.
    static void* operator new(size_t size) {
        return ((GraphicsAllocator*)g_graphicsMemoryCallbacks)->allocate(size);
    }
    // Release resource-wrapper storage through the graphics memory callback.
    static void operator delete(void* p) {
        GraphicsResource_Free(p);
    }
    // Initialize a type-7 graphics resource wrapper.
    Texture80427DA4(void* value) {
        GraphicsResource_InitType7(this, value);
    }
};
struct Texture80427D48 {
    char data[0xCC];
    // Allocate wrapper storage using the registered graphics allocator.
    static void* operator new(size_t size) {
        return ((GraphicsAllocator*)g_graphicsMemoryCallbacks)->allocate(size);
    }
    // Release resource-wrapper storage through the graphics memory callback.
    static void operator delete(void* p) {
        GraphicsResource_Free(p);
    }
    // Copy the supplied vertex declaration into its resource wrapper.
    Texture80427D48(void* value) {
        VertexDeclaration_CopyConstruct(this, value);
    }
};
struct Texture80427C70 {
    char data[0x18];
    // Allocate wrapper storage using the registered graphics allocator.
    static void* operator new(size_t size) {
        return ((GraphicsAllocator*)g_graphicsMemoryCallbacks)->allocate(size);
    }
    // Release resource-wrapper storage through the graphics memory callback.
    static void operator delete(void* p) {
        GraphicsResource_Free(p);
    }
    // Construct an index-buffer resource from the forwarded arguments.
    Texture80427C70(void* indexCountWord, void* usageWord, void* formatWord, void* allocationModeWord,
                    void* indexStrideBytesWord) {
        IndexBuffer_Ctor(this, indexCountWord, usageWord, formatWord, allocationModeWord, indexStrideBytesWord);
    }
};
struct Texture80427B88 {
    char data[0x1C];
    // Allocate wrapper storage using the registered graphics allocator.
    static void* operator new(size_t size) {
        return ((GraphicsAllocator*)g_graphicsMemoryCallbacks)->allocate(size);
    }
    // Release resource-wrapper storage through the graphics memory callback.
    static void operator delete(void* p) {
        GraphicsResource_Free(p);
    }
    // Construct a texture-palette resource from the forwarded arguments.
    Texture80427B88(void* paletteTypeWord, void* unusedWord, void* formatWord, void* allocationModeWord) {
        TexturePalette_Ctor(this, paletteTypeWord, unusedWord, formatWord, allocationModeWord);
    }
};
struct Texture80427BFC {
    char data[0x14];
    // Allocate wrapper storage using the registered graphics allocator.
    static void* operator new(size_t size) {
        return ((GraphicsAllocator*)g_graphicsMemoryCallbacks)->allocate(size);
    }
    // Release resource-wrapper storage through the graphics memory callback.
    static void operator delete(void* p) {
        GraphicsResource_Free(p);
    }
    // Construct a vertex-buffer resource from the forwarded arguments.
    Texture80427BFC(void* byteCountWord, void* usageWord, void* vertexFormatWord, void* allocationModeWord) {
        VertexBuffer_Ctor(this, byteCountWord, usageWord, vertexFormatWord, allocationModeWord);
    }
};
struct Texture80427E00 {
    char data[0x20];
    // Allocate wrapper storage using the registered graphics allocator.
    static void* operator new(size_t size) {
        return ((GraphicsAllocator*)g_graphicsMemoryCallbacks)->allocate(size);
    }
    // Release resource-wrapper storage through the graphics memory callback.
    static void operator delete(void* p) {
        GraphicsResource_Free(p);
    }
    // Construct an owning render-surface resource.
    Texture80427E00(void* widthPixelsWord, void* heightPixelsWord, void* formatWord) {
        RenderSurface_Ctor(this, widthPixelsWord, heightPixelsWord, formatWord, 1);
    }
};

// Keep compiler-facing type identities/signatures for emitted constructor/cleanup
// symbols. The pointer-shaped Word arguments above carry numeric 32-bit values;
// the public factories below expose units and cast at this ABI boundary only.
// The create paths below name the actual resource, rather than calling every wrapper a texture.
typedef Texture80427CEC GraphicsType6Storage;
typedef Texture80427DA4 GraphicsType7Storage;
typedef Texture80427D48 VertexDeclarationStorage;
typedef Texture80427C70 IndexBufferStorage;
typedef Texture80427BFC VertexBufferStorage;
typedef Texture80427E00 RenderSurfaceStorage;
#pragma pop

extern "C" {
#pragma push
#pragma optimization_level 0
#pragma optimize_for_size on
#pragma dont_inline off
#undef MGR
#define MGR g_engineState
// Configure VI, allocate display/offscreen buffers, initialize cached states and create a scratch declaration.
int Engine_InitializeVideoAndRenderTargets(register void* configuration) {
#define configurationBytes ((unsigned char*)configuration)
    // Phase 1: cache caller configuration, choose VI mode and acquire display/FIFO buffers.
    *(Header804272F4*)MGR = *(Header804272F4*)configuration;
    g_graphicsMemoryCallbacks = (DtorVtable80427B58*)MGR;
    VIInit();
    register unsigned char* renderModeBytes = (unsigned char*)&((RendererStateView*)MGR)->videoMode;
    Engine_BuildRenderMode((GXRenderModeObj*)renderModeBytes, (EnginePresentParams*)(configurationBytes + 0x14),
                           (EngineViRect*)(configurationBytes + 0x4C));
    VIConfigure((const GXRenderModeObj*)renderModeBytes);
    ((void (*)(unsigned int*, const unsigned int*))Presentation_ApplyViScanModeFlags)(
        (unsigned int*)(configurationBytes + 0x14), (unsigned int*)renderModeBytes);
    ((int (*)(unsigned int, unsigned int, unsigned int, void**))RenderSurface_Create)(
        *(unsigned short*)(renderModeBytes + 4), *(unsigned short*)(renderModeBytes + 8), kDisplaySurfaceFormat44,
        (void**)(MGR + 0xC4));
    ((int (*)(unsigned int, unsigned int, unsigned int, void**))RenderSurface_Create)(
        *(unsigned short*)(renderModeBytes + 4), *(unsigned short*)(renderModeBytes + 8), kDisplaySurfaceFormat44,
        (void**)(MGR + 0xC0));
    unsigned int fifoSizeBytes = *((unsigned int*)&((RendererStateView*)MGR)->fifoBufferSize);
    void* allocatedFifoBuffer = ((void* (*)(unsigned int, void*))((Vt804272F4*)g_graphicsMemoryCallbacks)->reserved8)(
        fifoSizeBytes, (void*)g_graphicsMemoryCallbacks);
    *((void**)&((RendererStateView*)MGR)->fifoBuffer) = allocatedFifoBuffer;
    void* fifoBufferAlias = allocatedFifoBuffer;
    Render_InitializeAndBindGXFifo(0, *((unsigned int*)&((RendererStateView*)MGR)->useFifoBreakpoints) != 0, 1);
    // Phase 2: bind GX defaults, copy the first display frame and synchronize VI.
    Engine_InitializeGXPixelAndTevDefaults();
    void* displaySurface = (*(void**)&((EngineState*)MGR)->displayRenderTarget);
    unsigned int surfaceDescription[9];
    RenderSurface_GetDescription(displaySurface, surfaceDescription);
    GXSetDispCopySrc(0, 0, surfaceDescription[7] & 0xFFFF, surfaceDescription[8] & 0xFFFF);
    GXSetDispCopyDst(surfaceDescription[7] & 0xFFFF, surfaceDescription[8] & 0xFFFF);
    GXCopyDisp(GraphicsResource_GetStorage(displaySurface), 1);
    unsigned char* renderModeAlias = renderModeBytes;
    VISetNextFrameBuffer(GraphicsResource_GetStorage((*(void**)&((EngineState*)MGR)->displayRenderTarget)));
    VIFlush();
    VIWaitForRetrace();
    VIWaitForRetrace();
    unsigned int requiresExtraRetrace = *(unsigned int*)renderModeAlias & 1;
    if (requiresExtraRetrace)
        VIWaitForRetrace();
    *((unsigned int*)&((RendererStateView*)MGR)->lastRetraceCount) = Video_GetRetraceCount();
    VISetBlack(0);
    // Phase 3: seed transform, texture-combiner and render-state caches.
    int transformIndex;
    unsigned int colorOperation;
    unsigned int alphaOperation;
    for (transformIndex = 0; transformIndex < 0x2C; ++transformIndex)
        PSMTX44Identity(RENDERER_SLOT(transforms, transformIndex));
    for (register int i = 0; i < 8; ++i) {
        colorOperation = kCombineDisabled;
        alphaOperation = kCombineDisabled;
        if (i == 0) {
            colorOperation = kCombineMultiply;
            alphaOperation = kCombineSelectFirst;
        }
        RENDERER_SLOT(textureStageStates, i)[kDefaultColorOperation] = colorOperation;
        RENDERER_SLOT(textureStageStates, i)[kDefaultColorArgument1] = kArgumentTexture;
        RENDERER_SLOT(textureStageStates, i)[kDefaultColorArgument2] = kArgumentPrevious;
        RENDERER_SLOT(textureStageStates, i)[kDefaultAlphaOperation] = alphaOperation;
        RENDERER_SLOT(textureStageStates, i)[kDefaultAlphaArgument1] = kArgumentTexture;
        RENDERER_SLOT(textureStageStates, i)[kDefaultAlphaArgument2] = kArgumentPrevious;
        RENDERER_SLOT(textureStageStates, i)[kDefaultWrapS] = kWrapRepeat;
        RENDERER_SLOT(textureStageStates, i)[kDefaultWrapT] = kWrapRepeat;
        RENDERER_SLOT(textureStageStates, i)[kDefaultTexCoordSource] = kNoGeneratedTexCoord;
    }
    // Blend ONE/ZERO add; enable RGBA/depth writes, LEQUAL depth, no alpha test, back cull.
    (*(unsigned int*)&((EngineState*)MGR)->renderStates[kDefaultBlendEnabled]) = 1;
    (*(unsigned int*)&((EngineState*)MGR)->renderStates[kDefaultBlendOperation]) = kBlendAdd;
    (*(unsigned int*)&((EngineState*)MGR)->renderStates[kDefaultLogicOperation]) = kNoLogicOperation;
    (*(unsigned int*)&((EngineState*)MGR)->renderStates[kDefaultSourceBlend]) = kBlendOne;
    (*(unsigned int*)&((EngineState*)MGR)->renderStates[kDefaultDestinationBlend]) = kBlendZero;
    (*(unsigned int*)&((EngineState*)MGR)->renderStates[kDefaultColorWriteMask]) = kWriteRgba;
    (*(unsigned int*)&((EngineState*)MGR)->renderStates[kDefaultDepthWrite]) = 1;
    (*(unsigned int*)&((EngineState*)MGR)->renderStates[kDefaultDepthTest]) = 1;
    (*(unsigned int*)&((EngineState*)MGR)->renderStates[kDefaultDepthCompare]) = kDepthLessEqual;
    (*(unsigned int*)&((EngineState*)MGR)->renderStates[kDefaultDepthScale]) = kFloatOneBits;
    (*(unsigned int*)&((EngineState*)MGR)->renderStates[kDefaultDepthOffset]) = kFloatZeroBits;
    (*(unsigned int*)&((EngineState*)MGR)->renderStates[kDefaultAlphaTest]) = 0;
    (*(unsigned int*)&((EngineState*)MGR)->renderStates[kDefaultAlphaCompare]) = kAlphaAlways;
    (*(unsigned int*)&((EngineState*)MGR)->renderStates[kDefaultAlphaReference]) = 0;
    (*(unsigned int*)&((EngineState*)MGR)->renderStates[kDefaultCullMode]) = kCullBack;
    *((unsigned int*)&((RendererStateView*)MGR)->useDefaultAmbient) = 1;
    (*(unsigned int*)&((EngineState*)MGR)->lightingChannelStateDirty) = 1;
    int lightIndex;
    for (lightIndex = 0; lightIndex < 8; ++lightIndex)
        RENDERER_SLOT(lightEnabled, lightIndex) = 0;
    int type6Index;
    type6Index = 0;
    (*(unsigned int*)&((EngineState*)MGR)->type6Resource) = 0;
    (*(unsigned int*)&((EngineState*)MGR)->type7Resource) = 0;
    (*(unsigned int*)&((EngineState*)MGR)->renderTargetRequiresCopy) = 0;
    for (; type6Index < 7; ++type6Index)
        *(unsigned int*)&AsEngineState(MGR)->activeType6ResourceTypes[type6Index] = 0;
    int type7Index;
    for (type7Index = 0; type7Index < 7; ++type7Index)
        *(unsigned int*)&AsEngineState(MGR)->activeType7ResourceTypes[type7Index] = 0;

    // Phase 4: acquire optional offscreen color/depth surfaces and depth helper texture.
    if ((*(unsigned int*)&((EngineState*)MGR)->offscreenRenderingEnabled) != 0) {
        ((int (*)(unsigned int, unsigned int, unsigned int, void**))RenderSurface_Create)(
            *(unsigned short*)(renderModeBytes + 4), *(unsigned short*)(renderModeBytes + 8), kOffscreenColorFormat6,
            (void**)(MGR + 0xC8));
        *(unsigned int*)((*(unsigned char**)&((EngineState*)MGR)->offscreenRenderTarget) + 0x1C) = 1;
        ((int (*)(unsigned int, unsigned int, unsigned int, void**))RenderSurface_Create)(
            *(unsigned short*)(renderModeBytes + 4), *(unsigned short*)(renderModeBytes + 8), kOffscreenDepthFormat22,
            (void**)(MGR + 0xCC));
        int constructed;
        void* object = new Texture804272F4(8, 8, 1, 0, 0x80000000, 0);
        constructed = 1;
        unsigned int params[2];
        ((void (*)(void*, unsigned int, void*, unsigned int, unsigned int))Texture_LockLevel)(object, 0, params, 0, 0);
        Memory_Clear((void*)params[1], 0x20);
        GXInitTexObj((GXTexObj*)s_fullscreenDepthAuxTexture, (void*)params[1], 8, 8, (GXTexFmt)0, (GXTexWrapMode)0,
                     (GXTexWrapMode)0, (GXBool)0);
        GXInitTexObjLOD((GXTexObj*)s_fullscreenDepthAuxTexture, 0, 0, kFloat_0_GXRenderer, kFloat_0_GXRenderer,
                        kFloat_0_GXRenderer, 0, 0, 0);
        Texture_UnlockLevel(object, 0);
        ((int (*)(void*))GraphicsResource_Release)(object);
    } else {
        (*(unsigned int*)&((EngineState*)MGR)->offscreenRenderTarget) = 0;
        *((unsigned int*)&((RendererStateView*)MGR)->currentRenderSurface) = 0;
    }
    unsigned int def[3];
    *(Def804272F4*)def = s_scratchVertexDeclarationElements;
    VertexDeclaration_CreateCopy(def, (void**)(MGR + 0x2268));
    RefB804272F4* created = (*(RefB804272F4**)&((EngineState*)MGR)->scratchVertexDeclaration);
    created->hi = created->hi + 1;
    unsigned int savedRefs = created->hi;
    (void)savedRefs;
    (*(void**)&((EngineState*)MGR)->vertexDeclaration) = ((EngineState*)MGR)->scratchVertexDeclaration;
    s_gxRendererInitialized = 1;
    return 1;
}
#undef configurationBytes
#pragma pop
#undef MGR
#define MGR (*(unsigned char* volatile*)&g_engineState)

// Empty renderer hook; no observable work.
void Renderer_EmptyHook() {}

// Clear the 0x130-byte renderer defaults and set the texture-stage count to eight.
int Renderer_InitializeDefaults(void* self) {
    memset(self, 0, 0x130);
    ((RendererDefaultsView*)self)->textureStageCount = 8;
    return 1;
}

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma optimize_for_size on
#pragma peephole on
struct RefCountedResource {
    CW_BITS2(unsigned int high : 16, unsigned int low : 16)
    // Retain this resource by incrementing its packed reference count.
    unsigned int addRef() {
        return ++high;
    }
};
// Acquire the new render surface and release the previous one, restoring offscreen contents when needed.
int Engine_SetCurrentRenderSurface(void* value) {
    if (((EngineState*)g_engineState)->offscreenRenderingEnabled != 0)
        ((int (*)(void*))RenderSurface_DrawFullscreen)(value);
    if (value == *((void**)&((RendererStateView*)g_engineState)->currentRenderSurface))
        return 1;
    if (value) {
        ((RefCountedResource*)value)->addRef();
    }
    if (*((void**)&((RendererStateView*)g_engineState)->currentRenderSurface)) {
        ((void (*)(void*))GraphicsResource_Release)(
            *((void**)&((RendererStateView*)g_engineState)->currentRenderSurface));
    }
    *((void**)&((RendererStateView*)g_engineState)->currentRenderSurface) = value;
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma optimize_for_size on
#pragma peephole on
// Flush offscreen rendering and return an acquired reference to the current surface.
int Engine_AcquireCurrentRenderSurface(register RenderSurfaceRef** out) {
    register RenderSurfaceRef* ref = *((RenderSurfaceRef**)&((RendererStateView*)g_engineState)->currentRenderSurface);
    if (((EngineState*)g_engineState)->offscreenRenderingEnabled != 0)
        RenderSurface_CopyFramebufferToTexture(ref);
    *out = ref;
    if (ref) {
        ((RefCountedResource*)*out)->addRef();
    }
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma peephole on
#pragma opt_lifetimes off
#pragma optimize_for_size on
#pragma optimization_level 1
// Create a 2D texture when both dimensions are at least four pixels.
// On dimension rejection return 0 without writing out; otherwise return 1 and
// publish the initial owned reference. Release it with GraphicsResource_Release.
// format/layoutFlags are engine encodings, not GX enums; mipLevels=0 selects
// the constructor's default. allocationMode is passed to the graphics allocator.
int Texture_Create2D(register unsigned int widthPixels, register unsigned int heightPixels,
                     volatile unsigned int mipLevels, volatile unsigned int layoutFlags, volatile unsigned int format,
                     volatile unsigned int allocationMode, register void** out) {
    if (widthPixels < kMinimumTextureExtentPixels || heightPixels < kMinimumTextureExtentPixels) {
        return 0;
    }
    *out = new Texture804272F4(widthPixels, heightPixels, mipLevels, layoutFlags, format, allocationMode);
    return 1;
}
#pragma pop

// Free graphics storage through the registered destruction callback.
void GraphicsResource_Free(void* self) {
    void* volatile saved = self;
    g_graphicsMemoryCallbacks->destroy(self);
}

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma peephole on
#pragma optimize_for_size on
// Allocate and construct a texture palette in the graphics allocator.
// paletteType selects entry count; unused is ignored by TexturePalette_Ctor.
// These factories return 1 after construction, publishing an initial reference
// owned by the caller; release through GraphicsResource_Release, not delete.
int TexturePalette_Create(unsigned int paletteType, unsigned int unused, unsigned int format,
                          unsigned int allocationMode, register void** out) {
    *out = new Texture80427B88((void*)paletteType, (void*)unused, (void*)format, (void*)allocationMode);
    return 1;
}

// Allocate and construct a vertex buffer in the graphics allocator.
// byteCount sizes the source stream; a nonzero vertexFormat may also allocate
// a converted stream. usage and allocationMode preserve engine flag domains.
int VertexBuffer_Create(unsigned int byteCount, unsigned int usage, unsigned int vertexFormat,
                        unsigned int allocationMode, register void** out) {
    *out = new VertexBufferStorage((void*)byteCount, (void*)usage, (void*)vertexFormat, (void*)allocationMode);
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma peephole on
#pragma optimize_for_size on
// Allocate and construct an index buffer in the graphics allocator.
// indexCount counts indices; indexStrideBytes is forwarded to the descriptor.
// unused is a compatibility slot. The returned reference owns the backing buffer.
int IndexBuffer_Create(unsigned int indexCount, unsigned int usage, unsigned int format, unsigned int allocationMode,
                       register void** out, void* unused, unsigned int indexStrideBytes) {
    *out = new IndexBufferStorage((void*)indexCount, (void*)usage, (void*)format, (void*)allocationMode,
                                  (void*)indexStrideBytes);
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma peephole on
#pragma optimize_for_size on
#pragma optimization_level 1
#pragma opt_lifetimes off
// Copy a borrowed payload-pointer word into a type-6 resource; no retain/copy
// of the pointed-to object occurs (GraphicsResource_InitType6). out owns only
// the newly allocated resource reference; the payload's semantic type is unknown.
int GraphicsResource_CreateType6(register void* payloadPointerAddress, register void** out) {
    void* savedSource = payloadPointerAddress;
    *out = new GraphicsType6Storage(*(unsigned int*)savedSource);
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma peephole on
#pragma optimize_for_size on
#pragma optimization_level 1
#pragma opt_lifetimes off
// Allocate a copy of the supplied vertex declaration.
int VertexDeclaration_CreateCopy(void* volatile source, register void** out) {
    *out = new VertexDeclarationStorage(source);
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma peephole on
#pragma optimize_for_size on
#pragma optimization_level 1
#pragma opt_lifetimes off
// Copy the first integer payload word into a type-7 resource (InitType7).
// The pointer-shaped load below preserves the legacy constructor ABI; the word
// is not dereferenced as a pointer. out owns the new resource reference.
int GraphicsResource_CreateType7(register void* payloadValueAddress, register void** out) {
    void* savedSource = payloadValueAddress;
    *out = new GraphicsType7Storage(*(void**)savedSource);
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma peephole on
#pragma optimize_for_size on
#pragma opt_lifetimes off
// Allocate and construct a render surface from width, height and format.
// Dimensions are pixels; format is the engine format word. out receives an owned
// initial reference; the surface constructor uses allocationMode=1.
int RenderSurface_Create(volatile unsigned int widthPixels, volatile unsigned int heightPixels,
                         volatile unsigned int format, register void** out) {
    *out = new RenderSurfaceStorage((void*)widthPixels, (void*)heightPixels, (void*)format);
    return 1;
}

// Render-surface creation adapter; only the first three parameters are forwarded.
int RenderSurface_CreateAdapter(volatile unsigned int widthPixels, volatile unsigned int heightPixels,
                                volatile unsigned int format, void* unused3, void* unused4, void* unused5,
                                register void** out) {
    *out = new Texture80427E00((void*)widthPixels, (void*)heightPixels, (void*)format);
    return 1;
}
#pragma pop

// Forward the clear color/depth to GX without changing their representation.
inline void CopyClear80427EE0(const DrawColor& color, unsigned int depth) {
    DrawColor rgba(color);
    GXSetCopyClear(&rgba, depth);
}
#pragma push
#pragma optimization_level 0
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Clear selected color/depth channels in the requested display-buffer rectangle.
// ignored. The callers' prototypes put the (dead) float argument at different positions among the integers; on PowerPC
// the float goes to f1 whatever its position, so all of them compile the same call (r3..r7 integers + f1 float). A
// canonical prototype must keep that float and the five integers: dropping the float removes the `lfs f1` from the Wii
// call sites.
int Engine_ClearFramebuffer(void* unused, void* src, unsigned int flags, unsigned int color) {
    unsigned int buf[9];
    volatile unsigned int local[4];
    void* p;
    if (s_gxRendererInitialized == 0)
        return 0;
    GXInvalidateVtxCache();
    GXInvalidateTexAll();
    if (flags & Clear_Depth) {
        GXSetZMode(1, 7, 1);
    } else {
        GXSetZMode(0, 0, 0);
    }
    (*(unsigned int*)&((EngineState*)g_engineState)->depthStateDirty) = 1;
    if (flags & Clear_Color) {
        GXSetColorUpdate(1);
        GXSetAlphaUpdate(1);
    } else {
        GXSetColorUpdate(0);
        GXSetAlphaUpdate(0);
    }
    (*(unsigned int*)&((EngineState*)g_engineState)->colorWriteStateDirty) = 1;
    (*(unsigned int*)&((EngineState*)g_engineState)->alphaWriteStateDirty) = 1;
    // The packed word is 0xRRGGBBAA; on the Wii its bytes in memory are r,g,b,a.
    CopyClear80427EE0(*(DrawColor*)&color, 0xFFFFFF);
    p = (*(void**)&((EngineState*)g_engineState)->displayRenderTarget);
    RenderSurface_GetDescription(p, buf);
    if (src != 0) {
        RenderRect_Copy(local, (const volatile unsigned int*)src);
    } else {
        local[1] = local[0] = 0;
        local[2] = buf[7];
        local[3] = buf[8];
    }
    GXSetDispCopySrc(local[0] & 0xFFFF, local[1] & 0xFFFF, local[2] & 0xFFFF, local[3] & 0xFFFF);
    GXSetDispCopyDst(0, 0);
    GXCopyDisp(GraphicsResource_GetStorage(p), 1);
    return 1;
}
#pragma pop
#pragma optimization_level 4

// Copy rectangle coordinates and dimensions using ordered volatile word accesses.
void RenderRect_Copy(volatile unsigned int* dst, const volatile unsigned int* src) {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
}

// Update the viewport cache only when its six words differ.
int Engine_SetViewport(const void* value) {
    if (memcmp(value, (unsigned char*)&((RendererStateView*)g_engineState)->viewport, 0x18) == 0)
        return 1;
    ((void (*)(volatile Result804280EC*, const volatile Result804280EC*))Viewport_Copy)(
        (Result804280EC*)((unsigned char*)&((RendererStateView*)g_engineState)->viewport),
        (const Result804280EC*)value);
    *((unsigned int*)&((RendererStateView*)g_engineState)->viewportDirty) = 1;
    return 1;
}

// Copy viewport coordinates, dimensions and depth range in order.
void Viewport_Copy(volatile Result804280EC* dst, const volatile Result804280EC* src) {
    dst->words[0] = src->words[0];
    dst->words[1] = src->words[1];
    dst->words[2] = src->words[2];
    dst->words[3] = src->words[3];
    dst->values[0] = src->values[0];
    dst->values[1] = src->values[1];
}

// Copy the currently cached viewport into caller storage.
int Engine_GetViewport(void* self) {
    void* volatile saved = self;
    ((void (*)(volatile Result804280EC*, const volatile Result804280EC*))Viewport_Copy)(
        (Result804280EC*)self, (Result804280EC*)((unsigned char*)&((RendererStateView*)g_engineState)->viewport));
    return 1;
}

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma peephole on
// Copy EFB to the display buffer, honor the retrace interval and swap buffers.
int Engine_PresentFrame() {
    __alloca(0);
    unsigned int buf[9];
    unsigned int count;
    unsigned int extra;
    void* p = (*(void**)&((EngineState*)g_engineState)->displayRenderTarget);
    RenderSurface_GetDescription(p, buf);
    GXSetDispCopySrc(0, 0, buf[7] & 0xFFFF, buf[8] & 0xFFFF);
    GXSetDispCopyDst(buf[7] & 0xFFFF, buf[8] & 0xFFFF);
    GXCopyDisp(GraphicsResource_GetStorage(p), 0);
    Render_WaitForDrawCompletion();
    if (*((int*)&((RendererStateView*)g_engineState)->useFifoBreakpoints) != 0)
        VISetNextFrameBuffer(GraphicsResource_GetStorage(*(void**)(*(unsigned char* volatile*)&g_engineState + 0xC4)));
    else
        VISetNextFrameBuffer(GraphicsResource_GetStorage(p));
    count = 0;
    extra = 0;
    switch (*(int*)&((RendererStateView*)g_engineState)->presentation.swapInterval) {
    case 0:
    case 1:
        count = 0;
        break;
    case 2:
        count = 1;
        break;
    case 3:
    case 5:
    case 6:
    case 7:
        break;
    case 4:
        count = 2;
        break;
    case 8:
        count = 3;
        break;
    case 0x80000000:
        extra = 1;
        break;
    default:
        break;
    }
    BOOL level = OSDisableInterrupts();
    while (Video_GetRetraceCount() < *((unsigned int*)&((RendererStateView*)g_engineState)->lastRetraceCount) + count)
        VIWaitForRetrace();
    OSRestoreInterrupts(level);
    VIFlush();
    if (extra == 0)
        VIWaitForRetrace();
    *((unsigned int*)&((RendererStateView*)g_engineState)->lastRetraceCount) = Video_GetRetraceCount();
    ((int (*)(int))Engine_SwapDisplayBuffers)(0);
    return 1;
}
#pragma pop
#pragma optimization_level 4

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma peephole on
#pragma opt_propagation off
#pragma opt_common_subs off
// Exchange two cached 32-bit display-buffer words.
inline void SwapWords(unsigned int& a, unsigned int& b) {
    unsigned int temp = a;
    a = b;
    b = temp;
}

// Swap display buffers and reinstall the optional FIFO breakpoint.
int Engine_SwapDisplayBuffers() {
    if (0)
        __alloca(0);
    unsigned char buf[0x84];
    unsigned int local;
    unsigned int p1;
    unsigned int p2;
    SwapWords((*(unsigned int*)&((EngineState*)MGR)->displayRenderTarget),
              *((unsigned int*)&((RendererStateView*)MGR)->nextDisplayRenderTarget));
    if (*((int*)&((RendererStateView*)MGR)->useFifoBreakpoints) != 0) {
        int result = GXGetCPUFifo(buf);
        GXGetFifoPtrs(buf, &p1, &p2);
        GXEnableBreakPt(p2);
        GXSetDrawSync(GpuSync_FifoBreakpoint);
        (void)result;
        (void)local;
    }
    return 0;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma optimize_for_size on
// Reconfigure VI and resize display surfaces when the presentation dimensions change.
int Engine_ChangeDisplayMode(void* modeArg) {
    unsigned int buf[9];
    void* volatile saved;
    unsigned char* rm = (unsigned char*)&((RendererStateView*)MGR)->videoMode;
    Engine_BuildRenderMode((GXRenderModeObj*)rm, (EnginePresentParams*)modeArg, (EngineViRect*)(MGR + 0x4C));
    VIConfigure((const GXRenderModeObj*)rm);
    ((void (*)(unsigned int*, const unsigned int*))Presentation_ApplyViScanModeFlags)((unsigned int*)modeArg,
                                                                                      (unsigned int*)rm);
    RenderSurface_GetDescription(*((void**)&((RendererStateView*)MGR)->nextDisplayRenderTarget), buf);
    if (buf[7] != ((EnginePresentParams*)modeArg)->width ||
        buf[8] != *(unsigned int*)&((EnginePresentParams*)modeArg)->height) {
        ((int (*)(void*))GraphicsResource_Release)(*((void**)&((RendererStateView*)MGR)->nextDisplayRenderTarget));
        ((int (*)(void*))GraphicsResource_Release)((*(void**)&((EngineState*)MGR)->displayRenderTarget));
        ((int (*)(unsigned int, unsigned int, unsigned int, void**))RenderSurface_Create)(
            *(unsigned short*)(rm + 4), *(unsigned short*)(rm + 8), 0x8000002C, (void**)(MGR + 0xC4));
        ((int (*)(unsigned int, unsigned int, unsigned int, void**))RenderSurface_Create)(
            *(unsigned short*)(rm + 4), *(unsigned short*)(rm + 8), 0x8000002C, (void**)(MGR + 0xC0));
        GXSetDispCopySrc(0, 0, *(unsigned short*)(rm + 4), *(unsigned short*)(rm + 6));
        GXSetDispCopyDst(*(unsigned short*)(rm + 4), *(unsigned short*)(rm + 8));
        GXSetDispCopyYScale((float)*(unsigned short*)(rm + 8) / (float)*(unsigned short*)(rm + 6));
    }
    unsigned char* dev = MGR;
    ((RendererStateView*)dev)->presentation.width = ((EnginePresentParams*)modeArg)->width;
    saved = (void*)(dev + 0x14);
    *(unsigned int*)&((RendererStateView*)dev)->presentation.height =
        *(unsigned int*)&((EnginePresentParams*)modeArg)->height;
    *(unsigned int*)&((RendererStateView*)dev)->presentation.reserved08 =
        *(unsigned int*)&((EnginePresentParams*)modeArg)->reserved08;
    *(unsigned int*)&((RendererStateView*)dev)->presentation.reserved0C =
        *(unsigned int*)&((EnginePresentParams*)modeArg)->reserved0C;
    *(unsigned int*)&((RendererStateView*)dev)->presentation.type =
        *(unsigned int*)&((EnginePresentParams*)modeArg)->type;
    *(unsigned int*)&((RendererStateView*)dev)->presentation.reserved14[0] =
        *(unsigned int*)&((EnginePresentParams*)modeArg)->reserved14[0];
    *(unsigned int*)&((RendererStateView*)dev)->presentation.reserved14[1] =
        *(unsigned int*)&((EnginePresentParams*)modeArg)->reserved14[1];
    *(unsigned int*)&((RendererStateView*)dev)->presentation.reserved14[2] =
        *(unsigned int*)&((EnginePresentParams*)modeArg)->reserved14[2];
    *(unsigned int*)&((RendererStateView*)dev)->presentation.reserved14[3] =
        *(unsigned int*)&((EnginePresentParams*)modeArg)->reserved14[3];
    *(unsigned int*)&((RendererStateView*)dev)->presentation.reserved14[4] =
        *(unsigned int*)&((EnginePresentParams*)modeArg)->reserved14[4];
    *(unsigned int*)&((RendererStateView*)dev)->presentation.reserved14[5] =
        *(unsigned int*)&((EnginePresentParams*)modeArg)->reserved14[5];
    *(unsigned int*)&((RendererStateView*)dev)->presentation.flags =
        *(unsigned int*)&((EnginePresentParams*)modeArg)->flags;
    *(unsigned int*)&((RendererStateView*)dev)->presentation.tvFormat =
        *(unsigned int*)&((EnginePresentParams*)modeArg)->tvFormat;
    *(unsigned int*)&((RendererStateView*)dev)->presentation.swapInterval =
        *(unsigned int*)&((EnginePresentParams*)modeArg)->swapInterval;
    VIFlush();
    VIWaitForRetrace();
    VIWaitForRetrace();
    return 1;
}
#pragma pop
#pragma optimization_level 4

#pragma push
#pragma optimization_level 0
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Acquire and bind a stage resource, release the old one and dirty TEV component caches.
int Engine_SetTextureStageResource(unsigned int index, void* value) {
    if (!value && *((void**)&((RendererStateView*)MGR)->stageResources[index]) == 0)
        return 1;
    *(unsigned int*)&AsEngineState(MGR)->textureStageDirty[index] = 1;
    if (value) {
        unsigned int count = ++((volatile Bits_80425930*)value)->high;
        (void)count;
    }
    if (*((void**)&((RendererStateView*)MGR)->stageResources[index]))
        GraphicsResource_Release(*((void**)&((RendererStateView*)MGR)->stageResources[index]));
    *((void**)&((RendererStateView*)MGR)->stageResources[index]) = value;
    for (int i = 0; i < 8; ++i) {
        int dirty = 1;
        *(unsigned int*)(MGR + i * 8 + 0x14FC) = dirty;
        *(unsigned int*)(MGR + i * 8 + 0x1500) = dirty;
        (*(unsigned int*)&((EngineState*)MGR)->textureStateDirty) = dirty;
    }
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma peephole on
#pragma optimize_for_size on
// Return an acquired reference to a bound texture-stage resource.
int Engine_GetTextureStageResource(int index, StageResourceRef** out) {
    *out = ((Entry80428658*)(g_engineState + index * 4))->ref;
    if (*out) {
        ((RefCountedResource*)*out)->addRef();
    }
    return 1;
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Translate sampler parameter indices into texture-stage state indices.
int Engine_SetTextureStageParameter(void* a, unsigned int type, unsigned int c) {
    register unsigned int value;
    switch (type) {
    case 4:
        value = 0x14;
        break;
    case 3:
        value = 0x15;
        break;
    case 5:
        value = 0x16;
        break;
    case 0:
        value = 0x12;
        break;
    case 1:
        value = 0x13;
        break;
    case 6:
        value = 0x17;
        break;
    case 2:
    case 7:
    case 8:

        // Engine_SetTextureStageState stores through engine + stage*100 + index*4; ids 2/7/8 (and above 8) map to no state
        // index, and the original passes a stale register.
        break;
    }
    return Engine_SetTextureStageState(a, value, c);
}
#pragma pop

// Read one field from the cached render-state row.
int Engine_GetRenderStateRowField(int index, int field, unsigned int* out) {
    *out = ((RenderStateRow*)(g_engineState + 0x4A4))[index].fields[field];
    return 1;
}

#pragma push
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma dont_inline on
#pragma peephole on
#pragma optimize_for_size on
// Apply each parameter/value pair to the requested stage.
int Engine_SetTextureStageParameterList(void* a, unsigned int* entries, unsigned int count) {
    if (0)
        __alloca(0);
    for (unsigned int i = 0; i < count; ++i) {
        unsigned int* entry = entries + i * 2;
        ((int (*)(void*, unsigned int, unsigned int))Engine_SetTextureStageParameter)(a, entry[0], entry[1]);
    }
    return 1;
}
#pragma pop

#undef DW
#undef DP
#undef DF
#undef DWI
#undef MGR

} // extern "C"
