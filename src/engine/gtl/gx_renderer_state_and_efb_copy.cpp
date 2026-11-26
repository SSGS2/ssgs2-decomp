// GX renderer bindings, projection and EFB texture capture (0x804287D0).
// Mod lookup:
// - Cup mask / terrain / HUD captures: Engine_CopyFramebufferToTexture copies an
//   EFB pixel rectangle into mip level zero. sourceBounds overrides the cached
//   EngineState::copySourceX/Y/Width/Height (+0x5E8). Destination dimensions/format
//   come from Texture_GetLevelDescription, not from that source rectangle.
//   Keep both extents and texture allocation/tile format consistent in a mod.
// - Widescreen aspect ratio: SetTransform(state=1) receives the projection;
//   Engine_GetTransform returns it. No 4:3/16:9 or 640x480 policy is defined here.
//   Viewport/scissor setup is in gx_renderer_state_and_resources.cpp; changing an EFB copy rectangle
//   alone does not change either viewport or scissor or the HUD's layout space.
// - Texture filtering/wrap/blend: TextureStateId/CachedRenderStateId below are
//   ENGINE IDs. Consumers gx_renderer_state_and_resources.cpp/gx_render_state_and_texture_upload.cpp/gtl_texture_stage_tev_setup.cpp/gx_fifo_vertex_and_tev_helpers.cpp translate them.
//   Engine_SetTexturePalette/GetTexture are legacy names for the PALETTE array at +0x84,
//   not the image array at +0x64 (Engine_Set/GetTextureStageResource, gx_renderer_state_and_resources.cpp).
//   EngineState::palettes/paletteDirty name the retained palettes and dirty words;
//   RendererStateView and RenderState80422F10 expose the verified second view.
//   Paletted formats must keep the image, palette and sampler changes coordinated.
// - Model drawing: vertex/index resources and declarations must agree in stride,
//   element usage, command prefixes and 32-byte GX display-list alignment.
// API contract for all functions below: renderer thread, initialized g_engineState;
// required input/output pointers unless a specific null case is documented.
// Texture stages are 0..7, render-state IDs 0..41, transforms 0..42, vertex streams
// 0..1. No bounds checks. Set/get functions return 1 as completion, never validation.
// Resource getters retain nonnull outputs: caller balances GraphicsResource_Release.
// Resource setters retain new before releasing old; argument arrays/matrices are
// borrowed for the call. Integer-in-pointer ABI slots remain 32-bit Wii words.
// Pragma blocks, volatile spills, duplicate loads and narrowing to GX u16 counts
// preserve CodeWarrior code shape. They do not provide concurrent access safety.
// Constants 0/1/2 used in projection math are pooled in phase1_shared_data.cpp;
// preserve floating-point operation order and do not repurpose shared identities.
#include "classes/EngineState.h"
#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>

// Partial views of renderer resources; only the fields read below are described.
struct RenderBufferFields {
    char padding00[0x0C];
    unsigned int byteSize;
    char padding10[4];
    int displayListCopies;
};
// The display-list allocator reads the same size word as a signed multiplication operand.
struct SignedRenderBufferFields {
    char prefix[0x0C];
    int byteSize;
};
struct RenderSurfaceFields {
    char padding00[0x1C];
    int copyMode;
};
struct VertexStreamFields {
    char padding00[0xA4];
    int buffers[2];
    int strides[2];
};
// +153C is consumed/cleared by Engine_ApplyStateBeforeDraw before updating GX arrays.
struct VertexArrayInvalidationView {
    char prefix[0x153C];
    int vertexArraysDirty;
};
// Cast-only accessors remain macros because some draw paths require inline_depth(0).
#define AsRenderBuffer(resource) ((RenderBufferFields*)(resource))
#define AsRenderSurface(resource) ((RenderSurfaceFields*)(resource))
#define AsVertexStreams(state) ((VertexStreamFields*)(state))
// Palette API view of the SAME EngineState palettes/paletteDirty storage;
// image bindings are separate at +0x64.
struct PaletteBindingsView {
    char prefix[0x84];
    int palettes[8];
    char between[0x1594 - 0xA4];
    int paletteDirty[8];
};
typedef char PaletteBindingOffset[((int)&((PaletteBindingsView*)0)->palettes == 0x84) ? 1 : -1];
typedef char PaletteDirtyOffset[((int)&((PaletteBindingsView*)0)->paletteDirty == 0x1594) ? 1 : -1];
// ABI words encode a void (*)(void*) callback and its borrowed argument.
// Consumer: gx_renderer_state_and_resources.cpp::CallbackManager, Render_OnDrawSyncToken(0xDEAD).
struct TextureCopyCompletionWords {
    char prefix[0x14CC];
    unsigned int callbackWord, argumentWord;
};
enum {
    kTextureStageCount = 8,
    kCachedResourceTypeCount = 7,
    kVertexDeclarationCapacity = 16,
    kVertexElementEnd = 0xFF,
    kVertexElementUnusedType = 0x11,
    kDisplayListAlignment = 32,
    kModelTransformFirst = 0x0B,
    kModelTransformEnd = 0x2B,
    kViewTransform = 0,
    kProjectionTransform = 1,
    kFramebufferCopySyncToken = 0xDEAD, // synchronization marker, not a wait duration
    kBufferCommandPrefixBytes = 4,
    kBaseMipLevel = 0,
    kCopyClearFlagShift = 8,
    kLevelFormatWord = 0,
    kLevelWidthWord = 7,
    kLevelHeightWord = 8,
    kLockedLevelDataWord = 1
};
// Level description's high format bit is stripped before GXSetTexCopyDst.
// The remaining word is the GX texture format: preserve the producer's format
// and corresponding tiled allocation; this routine performs no CPU conversion.
static const unsigned int kGxTextureFormatBits = 0x7FFFFFFFu;

// Engine-local IDs, not GX enum values. Consumers: gx_render_state_and_texture_upload.cpp (render state),
// gx_renderer_state_and_resources.cpp (samplers), gtl_texture_stage_tev_setup.cpp/gx_fifo_vertex_and_tev_helpers.cpp (color/alpha combiners).
// Sampler filter/operation values are translated by those consumers; float values
// such as fog distances and LOD bias travel as raw IEEE-754 words.
enum TextureStateId {
    kTextureColorOperation = 0,
    kTextureColorArgument1 = 1,
    kTextureColorArgument2 = 2,
    kTextureAlphaOperation = 3,
    kTextureAlphaArgument1 = 4,
    kTextureAlphaArgument2 = 5,
    kTextureColorArgument0 = 14,
    kTextureAlphaAddendArgument = 15,
    kTextureWrapS = 18,
    kTextureWrapT = 19,
    kTextureMinFilter = 20,
    kTextureMagFilter = 21,
    kTextureMipFilter = 22,
    kTextureLodBias = 23,
    kTextureAnisotropy = 24
};

// Value domains verified in gtl_texture_stage_tev_setup.cpp/gx_fifo_vertex_and_tev_helpers.cpp; these are engine operations,
// distinct from GXTevOp. Only the listed operations have established equations.
enum TextureCombineValue {
    kCombineMultiply = 0,
    kCombineAdd = 2,
    kCombineDisable = 16,
    kCombineSelectFirst = 17,
    kCombineSelectSecond = 18,
    kCombineBlendConstant = 19,
    kCombineAlphaMultiplyAdd = 20,
    kCombineColorBlendTextureAlpha = 21,
    kCombineColorInterpolate = 22
};
enum TextureArgumentValue {
    kArgumentPrevious0 = 0,
    kArgumentPrevious1 = 1,
    kArgumentTexture = 2,
    kArgumentStageConstant = 3,
    kColorArgumentTextureAlpha = 0x22
};
// Arguments 0/1 both consume the previous TEV result in these translators.
// Argument 3 is operation-dependent: alpha uses it only for multiply-add's
// second argument. Consult the equation in the consumer before mixing values.
// Wrap S/T pass GXTexWrapMode directly: 0 clamp, 1 repeat, 2 mirror.
// Mag filter passes GXTexFilter directly (0 nearest, 1 linear); min+mip form a
// PAIR looked up in s_textureMinMipFilterTable, not a GX min-filter enum.
// The consumer has no defined min-filter fallback for unknown pairs; use a pair
// from that six-row table. This setter does not validate any sampler value.
// Table rows (min,mip -> GX) are (0,0->0), (1,0->1), (0,0->2),
// (1,0->3), (0,1->4), (1,1->5). Search takes the FIRST match: the two duplicate
// pairs do not select GX 2/3. Do not "repair" the table by assuming distinct IDs.
// Anisotropy is engine-valued: 2 -> GX 2x, 4 -> GX 4x, 1/3 leave GX 1x.
// It applies only when resolved minFilter==5 AND cached magFilter==2; ordinary
// GX mag values 0/1 bypass it. Preserve this unusual gate. LOD bias is float bits.
// Blend states: operation 1=add, 3=subtract; logicOperation=0x80000000 disables
// logic override. Source/destination factors are GXBlendFactor: 0 zero, 1 one,
// 2 source-color (destination-color for source factor), 3 its inverse,
// 4 source-alpha, 5 inverse-source-alpha, 6 destination-alpha, 7 its inverse.
// Other blend-operation values leave the consumer's mode undefined; avoid them.
enum CachedRenderStateId {
    kRenderCullMode = 0,
    kRenderFogEnabled = 1,
    kRenderLightingEnabled = 2,
    kRenderDepthCompare = 3,
    kRenderAlphaCompare = 4,
    kRenderDepthWrite = 5,
    kRenderDepthTest = 6,
    kRenderAlphaTestEnabled = 7,
    kRenderAmbientColor =
        8, // the environment ambient colour reaches GX only through this id (gtl_GenericEnvironment_Apply: pair {8, colour})
    kRenderBlendEnabled = 10,
    kRenderAlphaReference = 11,
    kRenderBlendOperation = 12,
    kRenderSourceBlend = 13,
    kRenderDestinationBlend = 14,
    kRenderLogicOperation = 15,
    kRenderSpecularColorSource = 17,
    kRenderMaterialColorSource = 18,
    kRenderAmbientColorSource = 19,
    kRenderLightingProperty20 = 20, // consumer not established
    kRenderFogType = 21,
    kRenderFogColor = 22,
    kRenderFogStart = 23,
    kRenderFogEnd = 24,
    kRenderLightingChannels = 31,
    kRenderNormalizedPointSize = 35, // GX point-size consumer, invalidates pointSizeStateDirty
    kRenderPointGroup36 = 36,
    kRenderPointGroup37 = 37,
    kRenderPointGroup38 = 38,
    kRenderPointGroup39 = 39,
    kRenderPointGroup40 = 40,
    kRenderPointGroup41 = 41,
    kRenderColorAndAlphaWrite = 30,
    kRenderDepthScale = 32,
    kRenderDepthOffset = 33
};
// Unknown lighting ID 20 and point-group IDs 36..41 keep their numeric identity.
// Only ID 35 is consumed by Render_ApplyPointSize, which sets GX point size.
// Changing anisotropy ID 24 alone does not dirty the sampler: change a sampler
// property 18..23 as well when a mod needs the new anisotropy uploaded immediately.
// Unlisted IDs are cached without invalidating a group. All IDs/indices are unchecked.
// Setters return 1 even for an unchanged value; this is not a validation result.

extern "C" {

extern void* g_engineState;
int g_useIndexedVertexInput;
int s_defaultTevStateDirty;
void* g_drawPrimitiveType;
int g_drawUsesBoundVertexBuffer;
int g_drawUsesIndexDisplayList;
void* g_boundBufferDrawArgument;
extern void* s_vertexFormatDescriptor[8];

void Engine_ApplyStateBeforeDraw();
void GXBegin(void*, int, unsigned short);
void Engine_UpdateOffscreenRenderTarget();
void RenderSurface_CopyFramebufferToTexture(void*);
int GraphicsResource_Release(volatile unsigned int*);
int Primitive_GetIndexCount(int, void*);
void Render_SetFlexibleVertexFormatState(void*);
void VertexFormat_BuildDeclaration(void*, void*);
void RenderSurface_DrawFullscreen(void*);
void RenderRect_Copy(void*, void*);
void Texture_GetLevelDescription(void*, int, void*);
void Texture_LockLevel(void*, int, void*, int, int);
void GXSetTexCopySrc(int, int, int, int, int);
void GXSetTexCopyDst(int, int, int, int);
void GXCopyTex(int, int);
void Texture_UnlockLevel(void*, int);
void GXSetDrawSync(int);
void GXPixModeSync();
void GXSetCullMode(void*);
void* GraphicsResource_GetStorage(void*);
void* Render_AlignSizeToMultiple(unsigned int, unsigned int);
void GXCallDisplayList(void*, void*);
void VertexInput_FindAndCopyFormatDescriptor(void*, void*);
unsigned int FlexibleVertexFormat_GetStride(void*);
void VertexDeclaration_GetElements(void*, void*, void*);
unsigned int VertexDeclaration_GetStride(void*, int);
int VertexDeclaration_TryConvertToFormatFlags(void*, void*);

void TextureProjection_CopySettingsFloatRange(void*, int, int);
int PSMTX44Transpose(void*, void*);
void BuiltinEffect_CopyProjectionFloatRange(void*, int, int);
int GraphicsResource_FindCachedType6Index(void*);
int GraphicsResource_FindCachedType7Index(void*);

struct RefCountWord {
    CW_BITS2(unsigned int count : 16, unsigned int rest : 16)
};

// Retain the type-6 resource, release the old one and invalidate its cached binding flags.
// Null clears the binding. Nonnull input must belong to the built-in type6 cache:
// FindCachedType6Index returns 8 on a miss, but this seven-word array is unchecked.
// Index0 also means no active type bit; preserve that sentinel/cache convention.
int Engine_SetType6Resource(void* object) {
    unsigned int count;
    int index;
    void* g = g_engineState;
    if (object == (*(void**)&AsEngineState(g)->type6Resource))
        return 1;
    if (object) {
        count = ++((volatile RefCountWord*)object)->count;
    }
    if ((*(void* volatile*)&AsEngineState(*(void* volatile*)&g_engineState)->type6Resource))
        GraphicsResource_Release((
            volatile unsigned int*)(*(void* volatile*)&AsEngineState(*(void* volatile*)&g_engineState)->type6Resource));
    (*(void**)&AsEngineState(g_engineState)->type6Resource) = object;
    volatile int i = 0;
    for (; i < kCachedResourceTypeCount; ++i)
        AsEngineState(g_engineState)->activeType6ResourceTypes[i] = 0;
    index = GraphicsResource_FindCachedType6Index(object);
    if (index)
        AsEngineState(g_engineState)->activeType6ResourceTypes[index] = 1;
    return 1;
}

// Retain the type-7 resource and update cached bindings or request the default TEV state.
// Null selects default TEV. Nonnull input must belong to the built-in type7 cache;
// a cache miss returns8 and would index beyond the seven-word active array here.
int Engine_SetType7Resource(void* object) {
    unsigned int count;
    int index;
    void* g = g_engineState;
    if (object == (*(void**)&AsEngineState(g)->type7Resource))
        return 1;
    if (object) {
        count = ++((volatile RefCountWord*)object)->count;
    }
    if ((*(void* volatile*)&AsEngineState(*(void* volatile*)&g_engineState)->type7Resource))
        GraphicsResource_Release((
            volatile unsigned int*)(*(void* volatile*)&AsEngineState(*(void* volatile*)&g_engineState)->type7Resource));
    (*(void**)&AsEngineState(g_engineState)->type7Resource) = object;
    volatile int i = 0;
    for (; i < kCachedResourceTypeCount; ++i)
        AsEngineState(g_engineState)->activeType7ResourceTypes[i] = 0;
    index = GraphicsResource_FindCachedType7Index(object);
    if (index == 0)
        s_defaultTevStateDirty = 1;
    else
        AsEngineState(g_engineState)->activeType7ResourceTypes[index] = 1;
    return 1;
}

// Cache a texture-stage value and invalidate the color, alpha or sampler state group.
// stageArgument carries an integer stage index in the pointer-sized ABI slot.
// Entry domains: stage=0..7, stateId=0..24, value is a 32-bit WORD. The complete
// known value/bit domain is beside TextureStateId/TextureCombineValue above:
// integer operations/arguments/wrap/filter/anisotropy versus IEEE-754 LOD bias.
// No numeric float-to-int conversion occurs here. IDs6..13/16..17 are storage
// slots without a dirty-group case in this API; do not invent sampler semantics
// for them. For unsupported combiner values, the consumers' switch defaults may
// omit GX writes; caching a value/returning1 does not establish valid rendering.
// Definitions: min/mip table and sampler translation in gx_renderer_state_and_resources.cpp; TEV color
// in gtl_texture_stage_tev_setup.cpp and alpha in gx_fifo_vertex_and_tev_helpers.cpp. Preserve their operation-dependent
// argument interpretation and the anisotropy dirty exception described above.
int Engine_SetTextureStageState(void* stageArgument, unsigned int stateId, unsigned int value) {
    unsigned int stageIndex = (unsigned int)stageArgument;
    void* engineState = g_engineState;
    // Address = engine + 0x184 + stage*100 + stateId*4: 25 words per stage.
    // The shifted whole-engine cast is address arithmetic, not another EngineState object.
    int* slot = &AsEngineState((char*)engineState + stageIndex * 100 + stateId * 4)
                     ->textureStageStates[0][0]; // original cache +0x184; retain byte-index evaluation order
    if (value == (unsigned int)*slot)
        return 1;
    *slot = value;
    // Color-combine, alpha-combine and sampler changes invalidate separate cache groups.
    switch (stateId) {
    case kTextureColorOperation:
    case kTextureColorArgument1:
    case kTextureColorArgument2:
    case kTextureColorArgument0:
        AsEngineState(engineState)->textureStageComponentDirty[stageIndex][0] = 1; // original +0x14FC
        AsEngineState(engineState)->textureStateDirty = 1;
        break;
    case kTextureAlphaOperation:
    case kTextureAlphaArgument1:
    case kTextureAlphaArgument2:
    case kTextureAlphaAddendArgument:
        AsEngineState(engineState)->textureStageComponentDirty[stageIndex][1] = 1; // original +0x1500
        AsEngineState(engineState)->textureStateDirty = 1;
        break;
    case kTextureWrapS:
    case kTextureWrapT:
    case kTextureMinFilter:
    case kTextureMagFilter:
    case kTextureMipFilter:
    case kTextureLodBias:
        AsEngineState(engineState)->textureStageDirty[stageIndex] = 1;
        break;
    case kTextureAnisotropy:
        break;
    }
    return 1;
}

// Copy texture-projection settings: a is a float index encoded as void*, b is a
// borrowed float-array address encoded as int, c counts FOUR-float groups.
// The callee reads source[a..a+4*c); both arrays must contain that range.
// No range/overlap validation; preserve the ABI spill frame and forward copy order.
void TextureProjection_CopySettingsFloatRangeThunk(void* startIndex, int source, int vectorCount) {
    volatile int savedA = (int)startIndex;
    volatile int savedB = source;
    volatile int savedC = vectorCount;
    volatile int frame[5];
    TextureProjection_CopySettingsFloatRange(startIndex, source, vectorCount);
    (void)frame;
}

#pragma scheduling off
// Copy built-in projection settings with the same encoded index/source/group-count
// contract as the texture thunk; preserve spills and source/destination float indices.
void BuiltinEffect_CopyProjectionFloatRangeThunk(void* startIndex, int source, int vectorCount) {
    volatile int savedC;
    volatile int savedB;
    volatile int savedA;
    savedA = (int)startIndex;
    savedB = source;
    savedC = vectorCount;
    volatile int frame[5];
    BuiltinEffect_CopyProjectionFloatRange(startIndex, source, vectorCount);
    (void)frame;
}
#pragma scheduling reset

// Return the current type-7 resource with an additional retained reference.
int Engine_AcquireType7Resource(int* out) {
    void* g = g_engineState;
    *out = AsEngineState(g)->type7Resource;
    if (*out) {
        unsigned int count = ++((volatile RefCountWord*)*out)->count;
    }
    return 1;
}

// Return the current type-6 resource with an additional retained reference.
int Engine_AcquireType6Resource(int* out) {
    void* g = g_engineState;
    *out = AsEngineState(g)->type6Resource;
    if (*out) {
        void* obj = *(void* volatile*)out;
        unsigned int count = ++((volatile RefCountWord*)obj)->count;
    }
    return 1;
}

void Mtx44_Copy(void*, void*);

// Copy the selected cached 4x4 transform to the caller.
int Engine_GetTransform(int index, void* arg) {
    volatile int savedIndex = index;
    volatile void* savedArg = arg;
    void* base = g_engineState;
    void* p = (char*)base + index * 64;
    Mtx44_Copy(arg, AsEngineState(p)->transforms[0]); // original +0x600
    return 1;
}

struct VertexDeclarationElement {
    unsigned short stream;
    unsigned short byteOffset;
    unsigned int type;
    unsigned char method;
    unsigned char usage;
    unsigned char usageIndex;
    unsigned char trailingByte;
};

// Copy one complete vertex declaration element, including its trailing byte.
void VertexElement_Copy(VertexDeclarationElement* dst, VertexDeclarationElement* src) {
    dst->stream = src->stream;
    dst->byteOffset = src->byteOffset;
    dst->type = src->type;
    dst->method = src->method;
    dst->usage = src->usage;
    dst->usageIndex = src->usageIndex;
    dst->trailingByte = src->trailingByte;
}

// Return the cached flexible vertex-format flags without retaining an object.
int Render_GetVertexFormat(int* out) {
    void* g = g_engineState;
    *out = AsEngineState(g)->vertexFormat;
    return 1;
}

// Return one cached engine render-state value.
int Engine_GetRenderState(int index, int* out) {
    void* g = g_engineState;
    *out = AsEngineState(g)->renderStates[index];
    return 1;
}

#pragma inline_depth(0)
// Build a declaration for format flags and replace the retained declaration resource.
int Engine_SetFlexibleVertexFormat(void* formatFlags) {
    void* g = g_engineState;
    if ((*(void**)&AsEngineState(g)->vertexFormat) == formatFlags)
        return 1;
    Render_SetFlexibleVertexFormatState(formatFlags);
    g = g_engineState;
    AsEngineState(g)->usesVertexDeclaration = 0;
    volatile unsigned char frameSpace[0x80];
    VertexDeclarationElement copies[16];
    volatile unsigned char* fp = frameSpace;
    (void)fp;
    VertexFormat_BuildDeclaration(formatFlags, copies);
    // scratchVertexDeclaration layout: word 0 = header (high 16 bits = reference count, low 16 bits kept),
    // followed by kVertexDeclarationCapacity 12-byte elements starting at byte 12 (hence i*12+12 below).
    // Incrementing the high half retains this resource before releasing the old
    // binding. It does not track a declaration version or detect changed elements.
    for (int i = 0; i < kVertexDeclarationCapacity; ++i) {
        VertexDeclarationElement* dst =
            (VertexDeclarationElement*)((char*)AsEngineState(g_engineState)->scratchVertexDeclaration + i * 12 + 12);
        VertexElement_Copy(dst, &copies[i]);
    }
    unsigned int* resourceHeader = (unsigned int*)AsEngineState(g_engineState)->scratchVertexDeclaration;
    unsigned int oldHeader = *resourceHeader;
    *resourceHeader = (((oldHeader >> 16) + 1) << 16) | (oldHeader & 0xFFFF);
    unsigned int retainedReferenceCount = *resourceHeader >> 16;
    (void)retainedReferenceCount;
    g = g_engineState;
    if ((*(void**)&AsEngineState(g)->vertexDeclaration))
        GraphicsResource_Release((volatile unsigned int*)(*(void**)&AsEngineState(g)->vertexDeclaration));
    g = g_engineState;
    (*(void**)&AsEngineState(g)->vertexDeclaration) = AsEngineState(g)->scratchVertexDeclaration;
    return 1;
}
#pragma inline_depth()

// First word of a reference-counted graphics resource: the high-order 16-bit field (bit-field `count`) is the
// reference count incremented on retain (and, by GraphicsResource_Release, decremented on release); `rest`
// holds the other 16 bits unchanged.
struct RefWord80429820 {
    CW_BITS2(unsigned int count : 16, unsigned int rest : 16)
};

// Replace the active display/offscreen target and update offscreen-copy requirements.
// Both branches follow the same retain-new / release-old / store pattern on a reference-counted surface:
//   offscreenRenderingEnabled == 0 -> slot displayRenderTarget;
//   otherwise -> slot offscreenRenderTarget, then draw it fullscreen and set renderTargetRequiresCopy
//   when the surface's copyMode is 0. Null clears the display slot only when
//   offscreenRenderingEnabled==0; the offscreen branch requires a valid surface
//   because it draws and dereferences it after publication. `unused` is ignored.
// Volatile casts retain the original load order and carry no threading meaning.
int Engine_SetRenderTarget(int unused, void* object) {
    volatile void* savedObject = object;
    if (AsEngineState(g_engineState)->offscreenRenderingEnabled == 0) {
        if (savedObject) {
            ++((volatile RefWord80429820*)savedObject)->count;
        }
        if ((*(void* volatile*)&AsEngineState(g_engineState)->displayRenderTarget))
            GraphicsResource_Release(
                (volatile unsigned int*)(*(void* volatile*)&AsEngineState(*(void* volatile*)&g_engineState)
                                              ->displayRenderTarget));
        (*(void**)&AsEngineState(g_engineState)->displayRenderTarget) = (void*)savedObject;
    } else {
        if (savedObject) {
            ++((volatile RefWord80429820*)savedObject)->count;
        }
        if ((*(void* volatile*)&AsEngineState(g_engineState)->offscreenRenderTarget))
            GraphicsResource_Release(
                (volatile unsigned int*)(*(void* volatile*)&AsEngineState(*(void* volatile*)&g_engineState)
                                              ->offscreenRenderTarget));
        AsEngineState(g_engineState)->offscreenRenderTarget = (void*)savedObject;
        RenderSurface_DrawFullscreen((*(void* volatile*)&AsEngineState(g_engineState)->offscreenRenderTarget));
        AsEngineState(g_engineState)->renderTargetRequiresCopy =
            (AsRenderSurface(*(void* volatile*)&AsEngineState(*(void* volatile*)&g_engineState)->offscreenRenderTarget)
                 ->copyMode == 0);
    }
    return 1;
}

// Retain a palette binding at +0x84 and invalidate all texture-combine state groups.
// Engine_SetTexturePalette (symbol name Engine_SetTexturePalette).
// gx_renderer_state_and_resources.cpp::RendererStateView calls these paletteResources/paletteDirty, and
// Render_ApplyTevState queries them with TexturePalette_GetDescription (C8 images).
// index is 0..7; null clears the palette. Image bindings are a separate API.
int Engine_SetTexturePalette(int index, void* object) {
    volatile int savedIndex = index;
    volatile void* savedObject = object;
    void* g;
    if (!savedObject && (*(void**)&((PaletteBindingsView*)g_engineState)->palettes[savedIndex]) == 0)
        return 1;
    g = g_engineState;
    ((PaletteBindingsView*)g)->paletteDirty[savedIndex] = 1;
    if (savedObject) {
        volatile RefCountWord* ref = (volatile RefCountWord*)savedObject;
        unsigned int high = ref->count;
        high += 1;
        ref->count = high;
        RefCountWord* plain = (RefCountWord*)savedObject;
        unsigned int retainedReferenceCount = plain->count;
        (void)retainedReferenceCount;
    }
    g = g_engineState;
    if ((*(void**)&((PaletteBindingsView*)g)->palettes[savedIndex]))
        GraphicsResource_Release((volatile unsigned int*)(*(void**)&((PaletteBindingsView*)g)->palettes[savedIndex]));
    g = g_engineState;
    (*(void**)&((PaletteBindingsView*)g)->palettes[savedIndex]) = (void*)savedObject;
    volatile int i = 0;
    for (; i < kTextureStageCount; ++i) {
        AsEngineState(g_engineState)->textureStageComponentDirty[i][0] = 1;
        AsEngineState(g_engineState)->textureStageComponentDirty[i][1] = 1;
        AsEngineState(g_engineState)->textureStateDirty = 1;
    }
    return 1;
}

// Return the current vertex declaration with an additional retained reference.
int Render_GetVertexDeclaration(int* out) {
    void* g = g_engineState;
    *out = AsEngineState(g)->vertexDeclaration;
    if (*out) {
        void* obj = *(void* volatile*)out;
        volatile unsigned int retainedReferenceCount = ++((volatile RefCountWord*)obj)->count;
        (void)retainedReferenceCount;
    }
    return 1;
}

// Return the selected palette binding (+0x84), retaining a nonnull output for caller.
// Engine_GetTexturePalette (symbol name Engine_AcquireTexturePalette).
// Release the returned palette with GraphicsResource_Release.
int Engine_AcquireTexturePalette(int index, int* out) {
    void* g = g_engineState;
    int object = ((PaletteBindingsView*)g)->palettes[index];
    *out = object;
    if (object) {
        // The upper 16 bits are the same reference count as RefCountWord::count;
        // preserve the lower resource-header flags while retaining the palette.
        volatile unsigned int* p = (volatile unsigned int*)*out;
        unsigned int first = *p;
        unsigned int high = first >> 16;
        high += 1;
        unsigned int second = *p;
        *p = (second & 0xFFFF) | (high << 16);
        volatile unsigned int retainedReferenceCount = *p >> 16;
        (void)retainedReferenceCount;
    }
    return 1;
}

int Engine_SetCachedRenderState(int, void*);

struct Pair80429918 {
    int a;
    int b;
};

struct RenderStateListFrame {
    void* entries;
    unsigned int count;
};

// Apply an array of render-state ID/value pairs.
int Engine_SetRenderStateList(int* entries, unsigned int count) {
    volatile RenderStateListFrame frame;
    frame.entries = entries;
    frame.count = count;
    for (unsigned int i = 0; i < frame.count; ++i) {
        int* e = (int*)frame.entries;
        Engine_SetCachedRenderState(e[i * 2], (void*)e[i * 2 + 1]);
    }
    return 1;
}

// Read the cached value of a selected stage and texture-state ID.
int Engine_GetTextureStageState(int stageIndex, int stateId, int* out) {
    void* engineState = g_engineState;
    *out = AsEngineState(engineState)->textureStageStates[stageIndex][stateId]; // original cache +0x184
    return 1;
}

struct CachedTransform {
    float m[16];
};
#pragma inline_depth(8)
#pragma optimize_for_size on
#pragma optimization_level 0

extern const float kFloat_1_Renderer;
extern const float kFloat_0_GXRenderer;
extern const float kFloat_2_Renderer;

// Compute float absolute value through the compiler's double fabs intrinsic.
static inline float Abs80429460(float x) {
    double d = __fabs((double)x);
    float f = (float)d;
    return f;
}

// Recognize the perspective marker used by the fog-plane calculation.
static inline int IsPerspective80429460(float absolutePerspectiveTerm, float* matrix) {
    if (kFloat_1_Renderer == absolutePerspectiveTerm && kFloat_0_GXRenderer == matrix[15])
        return 1;
    if (kFloat_0_GXRenderer == matrix[11] && kFloat_1_Renderer == matrix[15])
        return 0;
    return 0;
}

// Cache a matrix, update projection fog planes and invalidate dependent transforms/lights.
// matrix is borrowed, with 16 contiguous floats in the engine's cache order.
// Projection state 1 uses A=m[10], P=m[11], B=m[14], W=m[15]. In four
// consecutive rows these are m22,m23,m32,m33; multiplication direction is not
// established by this setter. Do not transpose merely to match a GX convention.
// Supported perspective marker: |P|=1,W=0. Let R=(A-1)/(1+A) for A<0,
// or (1+A)/(A-1) otherwise: near=-B*(R-1)/R; far=near*R.
// Other forms use near=(1+B)/(2*A), far=(B-1)/(2*A), only when A<0.
// A valid nondegenerate projection is required: unsupported forms can publish
// uninitialized planes, and singular denominators are not checked. Preserve equations
// and evaluation order: algebraic simplification can change floating-point bytes.
int SetTransform(int state, float* matrix) {
    unsigned int i;
    CachedTransform* cache;
    CachedTransform* dst;
    volatile float recoveredPlanes[6]; // slots 4/5 are near/far; 0..3 preserve ABI scratch space
    if (memcmp(matrix, AsEngineState(g_engineState)->transforms[state], 0x40) == 0)
        return 1;
    AsEngineState(g_engineState)->transformDirty[state] = 1; // original +0x15B4
    cache = &((CachedTransform*)AsEngineState(g_engineState)->transforms)[state];
    *cache = *(CachedTransform*)matrix;
    if (state == kProjectionTransform) {
        float perspectiveTerm = matrix[11];
        double d = __fabs((double)perspectiveTerm);
        float f = (float)d;
        float absolutePerspectiveTerm = f;
        int perspectiveProjection;
        if (kFloat_1_Renderer == absolutePerspectiveTerm && kFloat_0_GXRenderer == matrix[15]) {
            perspectiveProjection = 1;
        } else if (kFloat_0_GXRenderer == matrix[11] && kFloat_1_Renderer == matrix[15]) {
            perspectiveProjection = 0;
        } else {
            perspectiveProjection = 0;
        }
        if (perspectiveProjection) {
            float ratio;
            if (matrix[10] < kFloat_0_GXRenderer) {
                ratio = (matrix[10] - kFloat_1_Renderer) / (kFloat_1_Renderer + matrix[10]);
                recoveredPlanes[4] = (-(matrix[14] * ratio - matrix[14])) / ratio;
            } else {
                ratio = (kFloat_1_Renderer + matrix[10]) / (matrix[10] - kFloat_1_Renderer);
                recoveredPlanes[4] = (-(matrix[14] * ratio - matrix[14])) / ratio;
            }
            recoveredPlanes[5] = recoveredPlanes[4] * ratio;
        } else if (matrix[10] < kFloat_0_GXRenderer) {
            recoveredPlanes[4] = (kFloat_1_Renderer + matrix[14]) / (kFloat_2_Renderer * matrix[10]);
            recoveredPlanes[5] = (matrix[14] - kFloat_1_Renderer) / (kFloat_2_Renderer * matrix[10]);
        }
        AsEngineState(g_engineState)->fogFarPlane = recoveredPlanes[5];
        AsEngineState(g_engineState)->fogNearPlane = recoveredPlanes[4];
    }
    if (state == kViewTransform) {
        for (i = 0; i < kTextureStageCount; ++i)
            AsEngineState(g_engineState)->lightStateDirty[i] = 1;
    }
    if (state >= kModelTransformFirst && state < kModelTransformEnd)
        PSMTX44Transpose(AsEngineState(g_engineState)->transposedTransforms[state - kModelTransformFirst],
                         matrix); // original +0x19F8
    return 1;
}
struct Pad80429460 {
    int x;
};
#pragma optimization_level 4
#pragma optimize_for_size reset
#pragma inline_depth()

// Return a retained stream buffer, zero offset and its cached stride.
int Engine_GetVertexStream(int index, int* outBuffer, int* outByteOffset, int* outStride) {
    volatile int savedIndex = index;
    void* g = g_engineState;
    int value = AsVertexStreams(g)->buffers[index];
    *outBuffer = value;
    if (value) {
        unsigned int count;
        count = ++((volatile RefCountWord*)value)->count;
    }
    *outByteOffset = 0;
    g = g_engineState;
    *outStride = AsVertexStreams(g)->strides[index];
    (void)savedIndex;
    return 1;
}

struct TextureStageStateListFrame {
    void* stageArgument;
    unsigned int* entries;
    unsigned int count;
};

// Apply an array of state ID/value pairs to one texture stage.
int Engine_SetTextureStageStateList(void* stageArgument, unsigned int* entries, unsigned int count) {
    volatile TextureStageStateListFrame saved;
    saved.stageArgument = stageArgument;
    saved.entries = entries;
    saved.count = count;
    for (unsigned int i = 0; i < saved.count; ++i) {
        unsigned int* p = saved.entries;
        Engine_SetTextureStageState((void*)saved.stageArgument, p[i * 2], p[i * 2 + 1]);
    }
    return 1;
}

// Copy an offscreen target when needed and return a retained active target.
int Engine_AcquireCurrentRenderTarget(int /*a*/, int* out) {
    void* g = g_engineState;
    if (AsEngineState(g)->offscreenRenderingEnabled == 0) {
        *out = AsEngineState(g)->displayRenderTarget;
        if (*out) {
            unsigned int count = ++((volatile RefCountWord*)*out)->count;
        }
    } else {
        void* d = AsEngineState(g)->offscreenRenderTarget;
        RenderSurface_CopyFramebufferToTexture(d);
        *out = (int)d;
        if (d) {
            unsigned int count = ++((volatile RefCountWord*)d)->count;
        }
    }
    return 1;
}

// Flush caller vertex data, emit an immediate primitive and finish offscreen rendering.
// vertices is borrowed through synchronous format emission; vertexStride is bytes,
// primitiveCount is primitive units converted by Primitive_GetIndexCount. The
// resulting vertex count must fit u16 and the buffer must contain count*stride
// bytes. Flush is a CPU->GX cache operation, not a frame pacing wait.
// Immediate callback schema: descriptor[2] is void writer(vertices,count,matrices).
// The eight descriptor words (after the format key) are defined by Data0VertexFormat
// gtl_render_state_and_primitive.cpp copies a matching row and leaves the old descriptor on a miss.
// descriptor[6] is the writer's fixed byte stride: vertexStride only controls
// cache flush length and MUST match that layout, it is not passed to the writer.
// matrices points at EngineState::transforms[11] (+0x8C0), first world/model 4x4
// in the contiguous palette (SetTransform indices11..42). It is borrowed only
// during emission. Ordinary XYZ/color/UV writers (gx_fifo_vertex_and_tev_helpers.cpp/gx_vertex_submit_and_cpu_skinning.cpp) ignore the
// third ABI argument and emit coordinates unchanged; Skin_Submit* writers use it
// as a matrix palette, blend indexed matrices, then transform positions/normals.
// Palette indices are bytes0..31, 0x20 terminates influences (Mtx44_BlendIndexed2
// in gx_vertex_submit_and_cpu_skinning.cpp; 3/4-influence variants in skin_vertex_transform.cpp). Each matrix is64B.
// Engine matrix storage uses translation in elements12..14: the perspective
// point consumer computes out[j]=x*m[j]+y*m[4+j]+z*m[8+j]+m[12+j], then divides
// XYZ by out[3] (gtl_vector_matrix_math.cpp). This is a storage/multiplication convention,
// not a permission to transpose the palette into GX layout before the callback.
// Thus vertices are in the space required by the SELECTED writer, not universally
// screen space. Bone indices must address that palette; weights/layout must agree
// with its format. NoOpHook descriptor rows intentionally emit no vertices.
// Set a supported format before drawing; unknown format can reuse a stale writer.
int DrawPrimitiveUP(int primitiveType, volatile int primitiveCount, void* vertices, volatile int vertexStride) {
    int vertexCount;
    void* gxPrimitiveType = (void*)primitiveType;
    int* engineState = (int*)g_engineState;
    g_useIndexedVertexInput = 0;
    g_drawPrimitiveType = gxPrimitiveType;
    g_drawUsesBoundVertexBuffer = 0;
    g_drawUsesIndexDisplayList = 0;
    AsEngineState(engineState)->drawStateDirty = 1;
    Engine_ApplyStateBeforeDraw();
    vertexCount = Primitive_GetIndexCount(primitiveCount, gxPrimitiveType);
    DCFlushRange(vertices, vertexStride * vertexCount);
    GXBegin(gxPrimitiveType, 0, (unsigned short)vertexCount);
    typedef void (*ImmediateVertexWriter)(void*, unsigned int, void*);
    ImmediateVertexWriter writeVertices = (ImmediateVertexWriter)s_vertexFormatDescriptor[2];
    writeVertices(vertices, (unsigned int)vertexCount, AsEngineState(g_engineState)->transforms[11]); // original +0x8C0
    Engine_UpdateOffscreenRenderTarget();
    return 1;
}

// Replace a retained vertex buffer/stride binding and invalidate stream state.
int Engine_SetVertexStream(int index, void* buffer, int /*unused*/, int stride) {
    volatile int savedStride = stride;
    void* p = buffer;
    if (p == 0) {
        if (AsVertexStreams(g_engineState)->buffers[index] == 0) {
            return 1;
        }
    }
    if (p != 0) {
        volatile RefCountWord* ref = (volatile RefCountWord*)p;
        unsigned int high = ref->count;
        high += 1;
        ref->count = high;
        RefCountWord* plain = (RefCountWord*)p;
        unsigned int retainedReferenceCount = plain->count;
        (void)retainedReferenceCount;
    }
    if (AsVertexStreams(g_engineState)->buffers[index] != 0) {
        GraphicsResource_Release((volatile unsigned int*)AsVertexStreams(g_engineState)->buffers[index]);
    }
    AsVertexStreams(g_engineState)->buffers[index] = (int)p;
    AsVertexStreams(g_engineState)->strides[index] = savedStride;
    ((VertexArrayInvalidationView*)g_engineState)->vertexArraysDirty = 1;
    return 1;
}

struct TextureCopyFrame {
    unsigned int w10[2];
    unsigned int bounds[4];
    unsigned int info[13];
};

#pragma optimization_level 4
#pragma opt_propagation off
#pragma use_lmw_stmw on
#pragma optimize_for_size on
// Copy selected/default framebuffer bounds into texture level zero and synchronize GX.
// Locals: f.bounds[0..3] = copy source x, y, width, height (from sourceBounds, or the engine's stored copy
// rectangle when null); f.info = destination level description (info[0] low 31 bits = texture format,
// info[7]/info[8] = destination width/height as passed to GXSetTexCopyDst); f.w10[1] = locked level data
// address that GXCopyTex writes to. copyFlags bit 8 selects clear-after-copy.
// Source x/y are EFB pixels from the top-left; width/height are pixel extents,
// not right/bottom endpoints. All four are narrowed to unsigned 16 bits without
// range checks. GXSetTexCopyDst receives destination width/height, format and no
// mipmap generation. Required destinationTexture stays borrowed during the lock.
// unused3..unused8 are unused ABI slots; final words are the copy-completion
// callback and argument, encoded as 32-bit Wii addresses. Zero callback disables
// notification. gx_renderer_state_and_resources.cpp::Render_OnDrawSyncToken receives 0xDEAD, invokes
// callback(argument), then clears ONLY the callback word. Thus callback/context
// must outlive queued GX completion; invocation precedes clearing (reentrant
// registration during callback is overwritten). The state holds one pending
// pair globally: submissions must serialize/replace it intentionally; this is
// not a per-texture callback queue. No ownership/reference count is acquired.
// Other producers of 0xDEAD, e.g. RenderSurface_CopyFramebufferToTexture, use
// the same dispatcher, so interleaving copies can consume the pending pair.
// Known capture callers gtl_builtin_effect_resources.cpp/gtl_texture_projection_input_device.cpp pass zero/zero. Lock failures are
// not checked; early publication of the pair and store order are preserved.
// The legacy five-argument declaration passes copyFlags in an extra integer slot
// to GXSetTexCopySrc; the SDK's operation has four parameters (see gx_renderer_state_and_resources.cpp).
// Do not infer a scaling/clear control from that extra argument. Finishes with a
// draw-sync marker and pixel-mode sync; it does not wait for a CPU-readable image.
// The caller must keep destination texture storage alive until the GPU completes
// the queued copy; unlocking level0 here does not transfer or release ownership.
void Engine_CopyFramebufferToTexture(volatile unsigned int copyFlags, void* sourceBounds,
                                     void* volatile destinationTexture, int unused3, int unused4, int unused5,
                                     int unused6, int unused7, int unused8, unsigned int copyCompletionCallbackWord,
                                     unsigned int copyCompletionArgumentWord) {
    volatile int spill[3];
    TextureCopyFrame f;
    register void* bounds = sourceBounds;
    ((TextureCopyCompletionWords*)g_engineState)->callbackWord = copyCompletionCallbackWord;
    ((TextureCopyCompletionWords*)g_engineState)->argumentWord = copyCompletionArgumentWord;
    (void)spill;
    if (bounds != 0) {
        RenderRect_Copy(f.bounds, bounds);
    } else {
        void* base = g_engineState;
        f.bounds[0] = AsEngineState(base)->copySourceX; // original +0x5E8..+0x5F4
        f.bounds[1] = AsEngineState(base)->copySourceY;
        f.bounds[2] = AsEngineState(base)->copySourceWidth;
        f.bounds[3] = AsEngineState(base)->copySourceHeight;
    }
    register void* texture = destinationTexture;
    Texture_GetLevelDescription(texture, kBaseMipLevel, f.info);
    Texture_LockLevel(texture, kBaseMipLevel, f.w10, 0, 0);
    unsigned int savedCopyFlags = copyFlags;
    unsigned int formatWord = f.info[kLevelFormatWord];
    register int clearAfterCopy = (savedCopyFlags >> kCopyClearFlagShift) & 1;
    register unsigned int textureFormat = formatWord & kGxTextureFormatBits;
    GXSetTexCopySrc((unsigned short)f.bounds[0], (unsigned short)f.bounds[1], (unsigned short)f.bounds[2],
                    (unsigned short)f.bounds[3], savedCopyFlags);
    GXSetTexCopyDst((unsigned short)f.info[kLevelWidthWord], (unsigned short)f.info[kLevelHeightWord], textureFormat,
                    0);
    GXCopyTex(f.w10[kLockedLevelDataWord], (unsigned char)clearAfterCopy);
    Texture_UnlockLevel(texture, kBaseMipLevel);
    unsigned int token = kFramebufferCopySyncToken;
    GXSetDrawSync((unsigned short)token);
    GXPixModeSync();
    __alloca(0);
}
#pragma use_lmw_stmw reset
#pragma optimize_for_size reset
#pragma opt_propagation reset
#pragma optimization_level 4
#pragma optimize_for_size reset
#pragma inline_depth()

#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma optimization_level 0
#pragma use_lmw_stmw on
#pragma optimize_for_size on
// Cache one render-state value and mark its consuming GX state group dirty.
// value is a raw 32-bit state word transported as void*, not an owned pointer.
// Repeated volatile engine loads and shifted views preserve the original access ABI.
// Mode immediateStateUpdates==1 applies state even when the cached word is unchanged.
int Engine_SetCachedRenderState(int indexArg, void* value) {
    if (value == (*(void**)&AsEngineState(*(void* volatile*)&g_engineState)->renderStates[(unsigned int)indexArg]) &&
        AsEngineState(*(void* volatile*)&g_engineState)->immediateStateUpdates != 1u)
        return 1;
    (*(void**)&AsEngineState(*(void* volatile*)&g_engineState)->renderStates[(unsigned int)indexArg]) = value;
    // Each state ID invalidates the GX group that consumes it; culling applies immediately.
    switch ((unsigned int)indexArg) {
    case kRenderCullMode:
        GXSetCullMode(value);
        break;
    case kRenderFogEnabled:
    case kRenderFogType:
    case kRenderFogColor:
    case kRenderFogStart:
    case kRenderFogEnd:
        AsEngineState(*(void* volatile*)&g_engineState)->fogStateDirty = 1;
        break;
    case kRenderLightingEnabled:
    case kRenderSpecularColorSource:
    case kRenderMaterialColorSource:
    case kRenderAmbientColorSource:
    case kRenderLightingProperty20:
        AsEngineState(*(void* volatile*)&g_engineState)->lightingStateDirty = 1;
        break;
    case kRenderDepthCompare:
    case kRenderDepthWrite:
    case kRenderDepthTest:
        AsEngineState(*(void* volatile*)&g_engineState)->depthStateDirty = 1;
        break;
    case kRenderBlendEnabled:
    case kRenderBlendOperation:
    case kRenderSourceBlend:
    case kRenderDestinationBlend:
    case kRenderLogicOperation:
        AsEngineState(*(void* volatile*)&g_engineState)->blendStateDirty = 1;
        break;
    case kRenderAlphaCompare:
    case kRenderAlphaTestEnabled:
    case kRenderAlphaReference:
        AsEngineState(*(void* volatile*)&g_engineState)->alphaCompareStateDirty = 1;
        break;
    case kRenderLightingChannels:
        AsEngineState(*(void* volatile*)&g_engineState)->lightingChannelStateDirty = 1;
        break;
    case kRenderColorAndAlphaWrite:
        AsEngineState(*(void* volatile*)&g_engineState)->alphaWriteStateDirty =
            AsEngineState(*(void* volatile*)&g_engineState)->colorWriteStateDirty = 1;
        break;
    case kRenderDepthScale:
    case kRenderDepthOffset:
        AsEngineState(*(void* volatile*)&g_engineState)->depthScaleOffsetStateDirty = 1;
        break;
    case kRenderNormalizedPointSize:
    case kRenderPointGroup36:
    case kRenderPointGroup37:
    case kRenderPointGroup38:
    case kRenderPointGroup39:
    case kRenderPointGroup40:
    case kRenderPointGroup41:
        AsEngineState(*(void* volatile*)&g_engineState)->pointSizeStateDirty = 1;
        break;
    default:
        break;
    }
    if (AsEngineState(*(void* volatile*)&g_engineState)->immediateStateUpdates == 1u)
        Engine_ApplyStateBeforeDraw();
    return 1;
}
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma optimization_level 4
#pragma use_lmw_stmw reset
#pragma optimize_for_size reset

int Engine_DrawPrimitiveWithCallback(void*, unsigned int, void*);

struct IndexedDrawFrame {
    int displayListAddressCopy;
    int displayListAddress;
    int displayListCopyCount;
    int indexDataAddressCopy;
    int indexDataAddress;
    int displayListStorageBase;
    int indexStorageBase;
    int submittedAlignedBytes;
    int submittedDisplayListAddress;
    int reservedAlignedBytes;
    int displayListPayloadBytes;
    int savedCopyCount;
    int descriptorStride;
    int indexCapacity;
    int displayListOffset;
    int indexCount;
};

#pragma opt_propagation off
#pragma optimize_for_size on
#pragma use_lmw_stmw on
#pragma optimization_level 1
#pragma scheduling on
// Draw indexed data through a callback or the index buffer display list.
// Three unnamed ABI words and arg5 are unused. Requires a bound index buffer and
// matching vertex format/resource; counts must fit its capacity. primitiveType is
// a GX topology encoded as pointer, not a pointee. No draw-range validation occurs.
int Engine_DrawIndexedPrimitive(void* primitiveType, void*, void*, void*, void* arg5, volatile int primitiveCount) {
    // Phase 1: derive index count and select raw-index callback vs cached display list.
    int argVal = primitiveCount;
    // Volatile scratch copies preserve the draw ABI; names distinguish addresses,
    // unaligned payload lengths, and lengths rounded for GX (no allocation here).
    volatile IndexedDrawFrame ctx;
    ctx.indexCount = Primitive_GetIndexCount(argVal, primitiveType);
    void* state = AsEngineState(g_engineState)->indexBuffer;
    if (AsRenderBuffer(state)->displayListCopies == 0) {
        void* base = GraphicsResource_GetStorage(state);
        unsigned int result = ctx.indexCount;
        ctx.indexStorageBase = (int)base;
        base = (char*)base + 4;
        ctx.indexDataAddress = (int)base;
        ctx.indexDataAddressCopy = (int)base;
        return Engine_DrawPrimitiveWithCallback(primitiveType, result, base);
    }
    // Phase 2: publish indexed/bound-buffer draw flags and apply dependent GX state.
    int one = 1;
    void* engine = g_engineState;
    g_useIndexedVertexInput = one;
    g_drawPrimitiveType = primitiveType;
    g_drawUsesBoundVertexBuffer = one;
    g_drawUsesIndexDisplayList = one;
    AsEngineState(engine)->drawStateDirty = one;
    ((void (*)(void*, int))Engine_ApplyStateBeforeDraw)(engine, one);
    // Phase 3: compute the aligned display-list byte span; storage has a 4-byte prefix.
    unsigned int halfSize = AsRenderBuffer(state)->byteSize;
    int format = (int)s_vertexFormatDescriptor[6];
    halfSize >>= 1;
    ctx.indexCapacity = halfSize;
    int zero = 0;
    ctx.displayListOffset = zero;
    ctx.descriptorStride = format;
    if (AsRenderBuffer(state)->displayListCopies == 0) {
        ctx.displayListCopyCount = 1;
    } else {
        ctx.displayListCopyCount = (*(volatile int*)&AsRenderBuffer(state)->displayListCopies);
    }
    int count = ctx.displayListCopyCount;
    ctx.savedCopyCount = count;
    int size = ((SignedRenderBufferFields*)state)->byteSize * count;
    ctx.displayListPayloadBytes = size;
    void* buffer = Render_AlignSizeToMultiple((unsigned int)(size + 4), 0x20);
    ctx.reservedAlignedBytes = (int)buffer;
    int savedSize;
    void* base = GraphicsResource_GetStorage(state);
    int offset = ctx.displayListOffset;
    savedSize = ctx.displayListPayloadBytes;
    void* end = (char*)base + offset;
    ctx.displayListStorageBase = (int)base;
    savedSize += 4;
    ctx.displayListAddress = (int)end;
    ctx.displayListAddressCopy = (int)end;
    ctx.submittedDisplayListAddress = (int)end;
    void* buffer2 = Render_AlignSizeToMultiple(savedSize, 0x20);
    void* source = (void*)ctx.submittedDisplayListAddress;
    ctx.submittedAlignedBytes = (int)buffer2;
    GXCallDisplayList(source, buffer2);
    Engine_UpdateOffscreenRenderTarget();
    return 1;
}
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
#pragma optimize_for_size reset
#pragma optimization_level 4
#pragma scheduling reset

// Begin an indexed primitive and emit indices through the selected format callback.
// callbackArg is borrowed index storage; vertexCount must fit u16. The format
// descriptor's callback at [3] must be initialized for the current bound streams.
int Engine_DrawPrimitiveWithCallback(void* primitiveType, unsigned int vertexCount, void* callbackArg) {
    g_useIndexedVertexInput = 1;
    g_drawPrimitiveType = primitiveType;
    g_drawUsesBoundVertexBuffer = 1;
    g_drawUsesIndexDisplayList = 0;
    AsEngineState(g_engineState)->drawStateDirty = 1;
    Engine_ApplyStateBeforeDraw();
    GXBegin(primitiveType, 0, (unsigned short)vertexCount);
    ((void (*)(void*, unsigned int))s_vertexFormatDescriptor[3])(callbackArg, vertexCount);
    Engine_UpdateOffscreenRenderTarget();
    return 1;
}

#pragma inline_depth(0)
#pragma use_lmw_stmw on
#pragma optimize_for_size on
#pragma opt_propagation off
#pragma optimization_level 1
// Draw bound vertices, falling back to immediate data for a partial buffer.
// primitiveCount is an integer count encoded as void*, firstVertex is a vertex
// index. Requires a nonnull vertex buffer/declaration, nonzero declaration stride
// and sufficient prepared display-list storage. The full-display-list path uses
// its prepared start, whereas only the raw fallback applies firstVertex.
int DrawPrimitiveFromBoundBuffers(int prim, volatile int firstVertex, void* primitiveCount) {
    // Address/byte-count/stride slots are all 32-bit Wii ABI words. They remain
    // separate volatile slots because their access order is part of this build.
    typedef unsigned int Address32;
    typedef unsigned int ByteStride32;
    typedef unsigned int ByteCount32;
    struct {
        volatile Address32 displayListAddressCopy, displayListAddress, firstVertexAddressCopy, firstVertexAddress;
        volatile ByteStride32 declarationStrideCopy, declarationStride;
        volatile Address32 displayListStorageBase, vertexStorageBase;
        unsigned int declarationElementCount;
        volatile Address32 declarationAddress;
        volatile ByteCount32 submittedAlignedBytes;
        volatile Address32 submittedDisplayListAddress;
        volatile ByteStride32 descriptorStride;
        volatile unsigned int requestedVertexCount, bufferVertexCapacity;
        volatile ByteStride32 savedDeclarationStride;
        volatile ByteCount32 convertedRegionOffset;
        volatile unsigned int requiresConversion;
        VertexDeclarationElement records[17];
    } ctx;
    // Storage map (already prepared by the resource writer; no conversion here):
    // base+0: command prefix (4 bytes), then byteSize bytes in the original format.
    // If descriptorStride != FVF stride, base+align32(byteSize+4) starts a second
    // command region holding the expanded format. Both regions include a prefix.
    // Raw-data fallback skips that prefix; GXCallDisplayList includes it.
    // Invariants: byteSize/declarationStride counts vertices; primitiveCount is
    // converted to requested vertices; descriptorStride*vertices+4 counts command
    // bytes before alignment. The unsigned subtraction tests ANY stride mismatch,
    // including underflow, so do not replace it with a greater-than comparison.
    // Phase 1: compare selected input stride with the bound declaration's stride/count.
    VertexInput_FindAndCopyFormatDescriptor(s_vertexFormatDescriptor,
                                            (*(void**)&AsEngineState(g_engineState)->vertexFormat));
    unsigned int stride = FlexibleVertexFormat_GetStride((*(void**)&AsEngineState(g_engineState)->vertexFormat));
    unsigned int formatStride = (unsigned int)s_vertexFormatDescriptor[6];
    void* state = AsEngineState(g_engineState)->vertexBuffer;
    ctx.convertedRegionOffset = 0;
    unsigned int differs = (formatStride - stride) > 0u;
    ctx.requiresConversion = differs;
    if (differs) {
        ctx.convertedRegionOffset = (unsigned int)Render_AlignSizeToMultiple(
            AsRenderBuffer(state)->byteSize + kBufferCommandPrefixBytes, kDisplayListAlignment);
    }
    void* data = (*(void**)&AsEngineState(g_engineState)->vertexDeclaration);
    ctx.declarationAddress = (unsigned int)data;
    VertexDeclaration_GetElements(data, ctx.records, &ctx.declarationElementCount);
    unsigned int stride2 = VertexDeclaration_GetStride(ctx.records, 0);
    unsigned int bytes = AsRenderBuffer(state)->byteSize;
    ctx.declarationStride = stride2;
    unsigned int stateCount = bytes / stride2;
    ctx.declarationStrideCopy = stride2;
    ctx.savedDeclarationStride = stride2;
    ctx.bufferVertexCapacity = stateCount;
    unsigned int result = Primitive_GetIndexCount((int)primitiveCount, (void*)prim);
    unsigned int available = ctx.bufferVertexCapacity;
    ctx.requestedVertexCount = result;
    // Phase 2a: a partial compatible buffer draws caller data at firstVertex (skip 4-byte prefix).
    if (available != result && !ctx.requiresConversion) {
        void* base = GraphicsResource_GetStorage(state);
        unsigned int start = firstVertex;
        ctx.vertexStorageBase = (unsigned int)base;
        void* ptr = (char*)(stride * start) + (unsigned int)base + kBufferCommandPrefixBytes;
        ctx.firstVertexAddress = (unsigned int)ptr;
        ctx.firstVertexAddressCopy = (unsigned int)ptr;
        return DrawPrimitiveUP(prim, (int)primitiveCount, ptr, stride);
    }
    // Phase 2b: use bound-buffer display-list storage and publish draw flags.
    int one = 1;
    int zero = 0;
    unsigned int bufSize = (unsigned int)s_vertexFormatDescriptor[6];
    g_useIndexedVertexInput = zero;
    g_drawPrimitiveType = (void*)prim;
    g_drawUsesBoundVertexBuffer = one;
    g_drawUsesIndexDisplayList = zero;
    g_boundBufferDrawArgument = primitiveCount;
    ctx.descriptorStride = bufSize;
    AsEngineState(g_engineState)->drawStateDirty = one;
    Engine_ApplyStateBeforeDraw();
    void* base = GraphicsResource_GetStorage(state);
    unsigned int offset = ctx.convertedRegionOffset;
    unsigned int n = ctx.requestedVertexCount;
    void* buffer = (char*)base + offset;
    unsigned int size = ctx.descriptorStride;
    ctx.displayListStorageBase = (unsigned int)base;
    ctx.displayListAddress = (unsigned int)buffer;
    ctx.displayListAddressCopy = (unsigned int)buffer;
    ctx.submittedDisplayListAddress = (unsigned int)buffer;
    // GX length is a byte count carried in a pointer ABI slot, not a new buffer.
    void* newBuf = Render_AlignSizeToMultiple(n * size + kBufferCommandPrefixBytes, kDisplayListAlignment);
    void* source = (void*)ctx.submittedDisplayListAddress;
    ctx.submittedAlignedBytes = (unsigned int)newBuf;
    GXCallDisplayList(source, newBuf);
    Engine_UpdateOffscreenRenderTarget();
    return 1;
}
#pragma use_lmw_stmw reset
#pragma optimize_for_size reset
#pragma opt_propagation reset
#pragma optimization_level 4
#pragma inline_depth()

#pragma inline_depth(0)
#pragma optimization_level 0
#pragma scheduling on
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma optimize_for_size on
// Retain a declaration and derive format flags, filtering unsupported usages if needed.
// object must be valid when a declaration is already bound: memcmp dereferences
// its elements before the null guard. A null input with no old binding is allowed.
// The fallback may fail conversion again; status is not checked a second time.
// Preserve supported usages 0/3/5/10, the 16-element capacity and terminator record.
// Declaration binary schema: VertexDeclarationElement is 12 bytes: u16 stream,
// u16 byteOffset, u32 type, u8 method/usage/usageIndex/reserved. At most 15 data
// records plus the full stream0xFF/type0x11 terminator fit the local arrays.
// Conversion in gtl_texture_stage_tev_setup.cpp requires contiguous stream0 storage, walks elements
// by byteOffset, and matches (usage,usageIndex,type) in increasing rule-table order.
// Established tuples in kVertexDeclarationConversionRules (source definition in
//   (2,0,0xFE), (0,0,2), (1,0,0/1/2), (3,0,2), (2,0,5), (10,0,4),
//   (5,0..3,1). These mean indices, XYZ, float1/2/3 weights, normal XYZ,
//   packed indices, packed color, UV sets0..3. Type sizes0/1/2/3 = float1/2/3/4,
//   type4 packed color4B, type5 storage4B, type6/7 short2/4, 0xFE special1B;
//   size support alone does not imply a conversion-table tuple exists.
// method is not examined by TryConvertToFormatFlags; producers BuildDeclaration
// use method0. Neither this function nor conversion validates other methods.
// usageIndex is a semantic set index, not a vertex/bone index. Fallback retains
// usages0/3/5/10 without changing type/index/method OR closing byteOffset gaps;
// it can therefore fail again. The converter zeroes *out before validation and
// only publishes flags at success: on retry failure frame.flags remains0, and
// Render_SetFlexibleVertexFormatState(0) still runs, usesVertexDeclaration=1,
// resource remains retained and return value is1. It is not a conversion status.
// Frame clearing and unchecked second status are existing behavior, not a promise
// of a usable draw format. Callers must provide one of the supported layouts.
int Engine_SetVertexDeclaration(void* object) {
    struct DeclarationResource {
        char resourceHeader[0x0C];
        VertexDeclarationElement elements[kVertexDeclarationCapacity];
    };
    struct {
        volatile unsigned int retainedReferenceCount;
        unsigned int flags;
        unsigned int count;
        VertexDeclarationElement records[16];
        VertexDeclarationElement descriptors[16];
    } frame;
    // The retain result is a reference count, including when stored only as a spill.
    int status;
    unsigned int i;
    unsigned int output;
    // Phase 1: compare declaration elements; otherwise retain new before releasing old.
    if ((*(void* volatile*)&AsEngineState(*(void* volatile*)&g_engineState)->vertexDeclaration) &&
        memcmp(((DeclarationResource*)object)->elements,
               ((DeclarationResource*)(*(void* volatile*)&AsEngineState(*(void* volatile*)&g_engineState)
                                            ->vertexDeclaration))
                   ->elements,
               sizeof(((DeclarationResource*)0)->elements)) == 0)
        return 1;
    if (object) {
        frame.retainedReferenceCount = ++((volatile RefCountWord*)object)->count;
    }
    if ((*(void* volatile*)&AsEngineState(*(void* volatile*)&g_engineState)->vertexDeclaration))
        GraphicsResource_Release(
            (volatile unsigned int*)(*(void* volatile*)&AsEngineState(*(void* volatile*)&g_engineState)
                                          ->vertexDeclaration));
    (*(void**)&AsEngineState(g_engineState)->vertexDeclaration) = object;
    if (!object)
        return 1;
    VertexDeclaration_GetElements(object, frame.descriptors, &frame.count);
    status = VertexDeclaration_TryConvertToFormatFlags(frame.descriptors, &frame.flags);
    // Phase 2: if direct format conversion fails, retain supported usage IDs and retry.
    if (!status) {
        unsigned int* p = &frame.count;
        unsigned int clearCount = 24;
        do {
            p[1] = 0;
            p[2] = 0;
            p += 2;
        } while (--clearCount);
        output = 0;
        for (i = 0; i < frame.count; ++i) {
            if (frame.descriptors[i].stream == kVertexElementEnd) {
                VertexDeclarationElement* dst = &frame.records[output];
                dst->stream = kVertexElementEnd;
                dst->byteOffset = 0;
                dst->type = kVertexElementUnusedType;
                dst->method = 0;
                dst->usage = 0;
                dst->usageIndex = 0;
                break;
            }
            switch (((VertexDeclarationElement*)((char*)frame.descriptors + i * 12))->usage) {
            // Omit these usage IDs from the FVF fallback; retain original records
            // only for the supported cases below. This filters descriptors, not
            // the retained resource or its ownership.
            case 1:
            case 2:
            case 4:
            case 6:
            case 7:
            case 8:
            case 9:
                break;
            // Retain position (0), normal (3), texture coordinates (5), color (10).
            case 0:
            case 3:
            case 5:
            case 10:
                VertexElement_Copy(&frame.records[output], &frame.descriptors[i]);
                ++output;
                break;
            }
        }
        status = VertexDeclaration_TryConvertToFormatFlags(frame.records, &frame.flags);
        AsEngineState(g_engineState)->usesVertexDeclaration = 1;
    }
    Render_SetFlexibleVertexFormatState((void*)frame.flags);
    return 1;
}
#pragma inline_depth()
#pragma scheduling reset
#pragma use_lmw_stmw reset
#pragma optimization_level reset
#pragma opt_propagation reset
#pragma optimize_for_size reset

} // extern "C"
