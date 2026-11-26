// GX render cache and encoded texture loading (GSR image-resource consumer).
// Modding lookup:
// - Texture formats/dimensions/mips come from gtl_Image_GetDescription into
//   DecodedTextureDescription, not a hard-coded resolution in this file.
// - Texture_LoadResourceWithOptionalPalette maps palette format to platform 7
//   (Wii), allocates handles and flushes palette bytes before GX can read them.
//   Palette count/type mapping: gtl_texture_vertex_formats.cpp; copy/deinterleave: gtl_graphics_texture_buffer_resources.cpp.
// - ImageResource_LoadFileAndDecode reads a zero-padded, 32-byte-rounded buffer;
//   this padding is not texture scaling or a guarantee of pointer alignment.
// - Engine_SetScissorRect takes GX framebuffer pixel {x,y,width,height};
//   Engine_SetOutputRectangle copies VI output {x,y,width,height,format} at +4C.
//   The output rectangle, draw viewport (gx_renderer_state_and_resources.cpp) and camera aspect ratio are
//   distinct settings: a widescreen mod must keep their consumers consistent.
// - Render_SetSystemOption controls the vertical copy filter and FIFO policy.
// Shared storage is g_engineState / EngineState.h. All cache entry points
// require an initialized engine and external render-thread serialization.
// Non-null input/output pointers and valid indices are caller preconditions
// unless a function explicitly tests them; these APIs do not validate bounds.
// Keep volatile accesses, float bit reinterpretations, pragmas and partial-error
// ownership behavior: the code keeps it as-is; it is not a loader/error-path fix.
#pragma ipa file
#pragma auto_inline off
#include "classes/EngineState.h"

#include "sdk_protos.h"

// Partial views fill the lighting/material gaps in EngineState without changing the shared header.
// Light record is 0x68 bytes, 4-byte lanes in native Wii big-endian storage.
// Consumer schema: Engine_ApplyStateBeforeDraw in gx_renderer_state_and_resources.cpp, DrawLight view.
// World-space position/direction are transformed by the cached VIEW matrix;
// position uses point transform, direction uses direction transform/normalization.
// type 1=point, 2=spot, 3=directional (see that consumer's light-type enum).
// Spot cutoff is (start+end)/2 converted from radians to GX degrees. The
// distance branch uses range at 1% brightness, or polynomial K=(a0,a1,a2);
// coefficients consequently use 1, inverse-distance, inverse-distance-squared.
// +50 is copied but not read by that submission path; leave it reserved.
struct CachedRenderLight {
    int type;
    float diffuse[4], specular[4], ambient[4];
    float worldPosition[3];      // +34, world distance
    float worldDirection[3];     // +40, normalized at submission
    float attenuationRange;      // +4C, world distance
    float reserved50;            // +50, no established GX consumer
    float constantAttenuation;   // +54, a0
    float linearAttenuation;     // +58, a1
    float quadraticAttenuation;  // +5C, a2
    float spotStartAngleRadians; // +60, DrawLight::start
    float spotEndAngleRadians;   // +64, DrawLight::end
};
struct CachedRenderMaterial {
    float diffuse[4], ambient[4], specular[4], emissive[4];
    float power;
};
struct PackedLightColors {
    unsigned char diffuse[4], ambient[4], specular[4];
};
struct EngineLightingView {
    char prefix[0x1100];
    CachedRenderLight lights[8];
    int lightEnabled[8];
    CachedRenderMaterial material;
    char padding[8];
    unsigned int scissor[4];
    char paddingToIndexDirty[0x84];
    int indexBufferDirty;
    char paddingToMaterialDirty[8];
    int materialDirty;
    char paddingToColors[0xca8];
    unsigned char materialDiffuse[4], materialAmbient[4], materialSpecular[4], materialEmissive[4];
    PackedLightColors lightColors[8];
};
struct EngineCopyFilterView {
    char prefix[0x16be];
    unsigned char samplePattern[24];
    unsigned char verticalFilter[7];
};
struct EngineSystemOptionsView {
    char prefix[0x5c4];
    int options[9];
};
// The upload receiver (gx_render_state_and_texture_upload.cpp) consumes format, texel width/height and mip count.
// +10 is the SOURCE platform layout, not mip bytes or a texture pointer.
// gtl_ImageGSR_GetDescription in gtl_image_decode_and_tiling.cpp maps GSR fileType 1/2/3/4/5 to
// layout 5/4/6/3/7 respectively; unknown types pass -1. GIM converts formats
// using layout 8 but does NOT initialize this fifth word; do not read it as 8.
// Keep the decoder's source representation distinct from kImagePlatformWii=7,
// the destination layout used for palette/upload conversion. This wrapper
// forwards the whole description; it does not override the fifth word. The
// upload receiver reads only the first four words, so GIM's unwritten word is
// not a local upload input (gx_render_state_and_texture_upload.cpp::Texture_CreateAndUploadMipChain).
// Format ABI words carry encoded integers, never owned or dereferenced pointers.
typedef void* ImageFormatAbiWord;
struct DecodedTextureDescription {
    ImageFormatAbiWord format;
    unsigned int widthTexels, heightTexels, mipLevelCount;
    unsigned int sourcePlatformLayout;
};
struct DecodedPaletteDescription {
    int swizzledFormat, entryCount;
};
enum ImageDecodeConstants {
    kNoImagePalette = -1,
    kImagePlatformWii = 7
};
typedef char DecodedTextureDescriptionSize[sizeof(DecodedTextureDescription) == 20 ? 1 : -1];
typedef char DecodedPaletteDescriptionSize[sizeof(DecodedPaletteDescription) == 8 ? 1 : -1];

struct TextureCreationOptions {
    char prefix[0x10];
    int paletteOptions;
};
enum SystemOption {
    kSystemReservedOption = 2,
    kSystemVerticalFilter = 3,
    kSystemAsyncFifo = 4,
    kSystemResetGX = 7,
    kSystemMiscToken1 = 8
};
// Shared getter/setter schema: nine native 32-bit words at engine+0x5C4.
// Engine_SaveRenderState (gtl_render_state_and_primitive.cpp) reads all indices 0..8 into its snapshot;
// Engine_RestoreRenderState does NOT replay these nine saved words; do not
// assume save/restore undoes a system-option change.
// 0 is immediateStateUpdates: Engine_SetCachedRenderState (gx_renderer_state_and_efb_copy.cpp) applies
// the cache immediately only when this word equals 1. 1/5/6 have no named local
// side effect: preserve their raw values rather than assigning a guessed label.
// Option domain (caller supplies a valid options[] index; no range check here):
// 2: stored only, no side effect established; 3: IEEE float bits in the int value,
// passed to vertical-filter weight generation; 4: zero/nonzero FIFO mode;
// 7: nonzero requests GX defaults immediately; 8: zero/nonzero selects GX misc 0/8.
// Other indices are retained verbatim. Boolean side effects do not normalize storage.
enum {
    kDrawCompletionToken = 0xbeef,
    kLightCount = 8,
    kImageFileAlignment = 0x20,
    kScissorWordCount = 4,
    kScissorBytes = 0x10,
    kOutputRectangleCacheOffset = 0x4C,
    kFileOpenDisposition3 = 3,
    kFileOpenAttribute80 = 0x80
};
// FileIO numeric disposition/attribute values retain their encoded ABI meaning;
static const unsigned int kFileReadAccess = 0x80000000u;
typedef char RenderLightSizeCheck[sizeof(CachedRenderLight) == 0x68 ? 1 : -1];
typedef char RenderLightPositionCheck[(int)&((CachedRenderLight*)0)->worldPosition == 0x34 ? 1 : -1];
typedef char RenderLightAttenuationCheck[(int)&((CachedRenderLight*)0)->constantAttenuation == 0x54 ? 1 : -1];
typedef char RenderLightSpotCheck[(int)&((CachedRenderLight*)0)->spotEndAngleRadians == 0x64 ? 1 : -1];
typedef char RenderMaterialSizeCheck[sizeof(CachedRenderMaterial) == 0x44 ? 1 : -1];
typedef char LightingScissorOffsetCheck[(int)&((EngineLightingView*)0)->scissor == 0x14ac ? 1 : -1];
typedef char LightingColorOffsetCheck[(int)&((EngineLightingView*)0)->materialDiffuse == 0x21f8 ? 1 : -1];

extern "C" {

extern void* g_engineState;
extern void* g_graphicsMemoryCallbacks;
extern int s_gxRendererInitialized;
extern void Render_BuildVerticalFilterWeights(void*, float);
extern void GXSetCopyFilter(int, void*, int, void*);
extern void Render_WaitForDrawCompletion();
extern void Render_InitializeAndBindGXFifo(int, int, int);
extern void Render_OnDrawSyncToken();
extern void Render_OnDrawDone();
extern void GXDraw_SetSyncCallback(void (*)());
extern void GXDraw_SetDoneCallback(void (*)());
extern void Engine_InitializeGXPixelAndTevDefaults();
extern void GXSetMisc(int, int);
extern void RenderLight_Copy(void*, void*);
extern void gtl_PixelConvert_Float4ToRGBA8(void*, const void*);
extern void Engine_ApplyStateBeforeDraw();
extern void RenderMaterial_Copy(void*, const void*);
extern void Engine_SetVertexStream(void*, void*, int, int);
extern void GXSetDrawSync(unsigned short);
extern unsigned short GXReadDrawSync();
extern void GXDrawDone();
extern void* ByteBuffer_ReleaseAllocation(void*, short);
extern void CallbackByteBuffer_Destroy(void*, short);
extern void __dl__FPv(void*);
extern int gtl_Image_GetDescription(void*, void*, void*, void*);
extern int Texture_CreateAndUploadMipChain(void*, void*, void*, void*);
extern void ScissorRect_Copy(unsigned int*, const unsigned int*);
extern void* ByteBuffer_DestroyBase(void*, short);
// FileIO_OpenFile takes 10 arguments: it loads the 9th/10th from the caller's 0x8/0xC(r1) (0x8041582C, 0x8041583C).
extern int FileIO_OpenFile(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int,
                           unsigned int, unsigned int, unsigned int, unsigned int);
extern int FileIO_GetFileSize(int, int*, int, int);
extern void FileIO_CloseFile(int);
extern int FileIO_ReadFile(void*, void*, int, int, int, int, int, int);
extern int Math_RoundUpToMultiple(int, int);
struct ByteBufferStorage {
    char* data;
    unsigned int size;
    unsigned int cap;
};
extern void ByteBuffer_ResizeZeroFilled(ByteBufferStorage*, unsigned int);
extern void std_ThrowLengthError();
extern void* dtor_8042AE7C(void*, short);
extern void Swap_Int32(void*, void*);
extern int TexturePalette_GetTypeForEntryCount(int);
extern void TexturePalette_GetData(void*, void*, int);
extern void TexturePalette_Flush(void*);
extern void TexturePalette_CopyOrDeinterleaveEntries(unsigned int, unsigned int, void*, void*);
extern void* gtl_ImageFormat_FromSwizzled(int);
extern void* gtl_ImageFormat_ToPlatformFormat(void*, int);
extern int gtl_Image_GetPaletteData(void*, void*, void*);
extern void TexturePalette_Create(void*, void*, void*, void*, void*, int);
// Update GX scissoring only when the framebuffer-pixel {x,y,width,height} changes.
// rect requires four readable u32s; copies them into the cache and returns 1
// even when unchanged. No clipping/range validation: callers must supply a
// rectangle supported by GX. Preserve cache update before hardware submission.
int Engine_SetScissorRect(const unsigned int* rect) {
    if (memcmp(rect, ((EngineLightingView*)g_engineState)->scissor, kScissorBytes) == 0)
        return 1;
    ScissorRect_Copy((unsigned int*)(((EngineLightingView*)g_engineState)->scissor), rect);
    GXSetScissor(rect[0], rect[1], rect[2], rect[3]);
    return 1;
}

// Destroy the byte buffer through its base cleanup and optionally free the object.
void* ByteBuffer_DeletingDtor(void* p, short flags) {
    volatile short savedFlags = flags;
    short mode = 0;
    if (p != 0) {
        if (p != 0)
            ByteBuffer_DestroyBase(p, mode);
        short f = savedFlags;
        void* volatile saved = p;
        if (f > 0)
            __dl__FPv(p);
    }
    return p;
}

// Decode an image description and upload its mip chain, clearing output on failure.
// imageSource borrows encoded bytes; imageByteCount carries a byte count in a
// pointer-shaped ABI word. textureHandleOut is required writable handle storage.
// Description failure writes zero/returns 0; otherwise forwards the upload
// status and resource-ownership contract without palette creation.
int Texture_CreateFromImageDescription(void* imageSource, void* imageByteCount, void* textureHandleOut) {
    char result[32];
    char local[8];
    int found = gtl_Image_GetDescription(result, local, imageSource, imageByteCount);
    if (found == 0) {
        *(int*)textureHandleOut = 0;
        return 0;
    }
    return Texture_CreateAndUploadMipChain(imageSource, imageByteCount, textureHandleOut, result);
}

#pragma optimization_level 0
// Cache one light, convert its three colors to RGBA8, and mark that light dirty.
// index is 0..7; data borrows a 0x68-byte CachedRenderLight. No validation;
// returns 1 on unchanged or copied state. Preserve color conversion and dirtying.
int Engine_SetLight(int index, const void* data) {
    if (memcmp(data, &((EngineLightingView*)g_engineState)->lights[index], 0x68) == 0)
        return 1;
    AsEngineState(g_engineState)->lightStateDirty[index] = 1;
    RenderLight_Copy(&((EngineLightingView*)g_engineState)->lights[index], (void*)data);
    gtl_PixelConvert_Float4ToRGBA8(((EngineLightingView*)g_engineState)->lightColors[index].diffuse,
                                   ((const CachedRenderLight*)data)->diffuse);
    gtl_PixelConvert_Float4ToRGBA8(((EngineLightingView*)g_engineState)->lightColors[index].specular,
                                   ((const CachedRenderLight*)data)->specular);
    gtl_PixelConvert_Float4ToRGBA8(((EngineLightingView*)g_engineState)->lightColors[index].ambient,
                                   ((const CachedRenderLight*)data)->ambient);
    return 1;
}
#pragma optimization_level reset

// Copy all 104 light bytes, retaining the original volatile word/float access order.
void RenderLight_Copy(void* dst, void* src) {
    volatile unsigned int* d = (volatile unsigned int*)dst;
    const volatile unsigned int* s = (const volatile unsigned int*)src;
    d[0] = s[0];
    // Keep each two-read/two-write pair explicit; an out-of-line copy helper
    // changes this unit's code under its file-wide compiler settings.
    {
        unsigned int first = s[1];
        unsigned int second = s[2];
        d[1] = first;
        d[2] = second;
    }
    {
        unsigned int first = s[3];
        unsigned int second = s[4];
        d[3] = first;
        d[4] = second;
    }
    {
        unsigned int first = s[5];
        unsigned int second = s[6];
        d[5] = first;
        d[6] = second;
    }
    {
        unsigned int first = s[7];
        unsigned int second = s[8];
        d[7] = first;
        d[8] = second;
    }
    {
        unsigned int first = s[9];
        unsigned int second = s[10];
        d[9] = first;
        d[10] = second;
    }
    {
        unsigned int first = s[11];
        unsigned int second = s[12];
        d[11] = first;
        d[12] = second;
    }
    {
        unsigned int first = s[13];
        unsigned int second = s[14];
        d[13] = first;
        d[14] = second;
    }
    d[15] = s[15];
    {
        unsigned int first = s[16];
        unsigned int second = s[17];
        d[16] = first;
        d[17] = second;
    }

    d[18] = s[18];
    for (int i = 19; i < 26; i++)
        ((volatile float*)dst)[i] = ((const volatile float*)src)[i];
}

// Cache material colors and shininess, convert colors, and invalidate all eight lights.
// data borrows 0x44 bytes in CachedRenderMaterial order; returns 1 even if equal.
// Colors are float4 inputs converted to RGBA8, power remains a float. No ownership.
int Engine_SetMaterial(const void* data) {
    if (memcmp(data, &((EngineLightingView*)g_engineState)->material, 0x44) == 0)
        return 1;
    ((EngineLightingView*)g_engineState)->materialDirty = 1;
    RenderMaterial_Copy(&((EngineLightingView*)g_engineState)->material, data);
    gtl_PixelConvert_Float4ToRGBA8(((EngineLightingView*)g_engineState)->materialDiffuse, data);
    gtl_PixelConvert_Float4ToRGBA8(((EngineLightingView*)g_engineState)->materialSpecular,
                                   ((const CachedRenderMaterial*)data)->specular);
    gtl_PixelConvert_Float4ToRGBA8(((EngineLightingView*)g_engineState)->materialAmbient,
                                   ((const CachedRenderMaterial*)data)->ambient);
    gtl_PixelConvert_Float4ToRGBA8(((EngineLightingView*)g_engineState)->materialEmissive,
                                   ((const CachedRenderMaterial*)data)->emissive);
    for (volatile int i = 0; i < 8; ++i)
        AsEngineState(g_engineState)->lightStateDirty[i] = 1;
    return 1;
}

struct QueueState {
    char prefix[0xb4];
    volatile int buffer;
    volatile int baseVertex;
};

// Copy one cached light into the caller buffer.
// index must be 0..7, lightOut must hold 0x68 writable bytes; always returns 1.
int Engine_GetLight(int index, void* lightOut) {
    int idx = index;
    void* a = lightOut;
    char* base = (char*)g_engineState;
    RenderLight_Copy(a, &((EngineLightingView*)base)->lights[idx]);
    return 1;
}

#pragma optimization_level 0
// Borrow an index-buffer resource (pointer encoded in the 32-bit ABI word) and
// cache its base-vertex word. No AddRef/release or allocation: keep the resource
// alive through draws and any Engine_SaveRenderState/RestoreRenderState use.
// Engine_DrawIndexedPrimitive (gx_renderer_state_and_efb_copy.cpp) obtains storage with a four-byte
// header, followed by 16-bit indices/display-list data; this is not a raw index
// array address. Geometry callers (grid, marker, particle) pass baseVertex=0;
// snapshots preserve other words verbatim. The Wii draw/header paths do not
// consume baseVertex, so a supported nonzero offset/range is NOT established.
// Null unbinds. If already unbound, return 1 without changing baseVertex or the
// dirty flag; otherwise write both cache words and mark the header dirty.
int Engine_SetIndexBuffer(int indexBufferAddress, int baseVertex) {
    if (indexBufferAddress == 0 && *(volatile int*)&AsEngineState(g_engineState)->indexBuffer == 0)
        return 1;
    *(volatile int*)&AsEngineState(g_engineState)->indexBuffer = indexBufferAddress;
    ((volatile QueueState*)g_engineState)->baseVertex = baseVertex;
    ((volatile EngineLightingView*)g_engineState)->indexBufferDirty = 1;
    return 1;
}
#pragma optimization_level reset

// Read the cached enable word for one light.
// index must be 0..7 and out is required; returns status 1, not the enable value.
int Engine_GetLightEnabled(int index, int* out) {
    volatile char* p = (volatile char*)g_engineState;
    p += index * 4;
    *out = *(volatile int*)(p + 0x1440);
    return 1;
}

// Read one raw cached option; index must be 0..8 and out must hold one writable
// int. Returns status 1, with *out replaced even when the option value is zero.
// No validation/failure path or GX side effect. See SystemOption above for the
// shared offset/meaning map: option 3 returns FLOAT BITS, not converted intensity.
// The shifted EngineState view below reaches options[index] at +0x5C4+4*index.
int Engine_GetSystemOption(int index, int* out) {
    volatile char* p = (volatile char*)g_engineState;
    p += index * 4;
    *out = (*(volatile int*)&((EngineState*)p)->immediateStateUpdates);
    return 1;
}

#pragma optimize_for_size on
#pragma optimization_level 1
// Apply copy-filter, FIFO, GX-reset or GX misc-token settings, then cache the raw word.
// mode must index options[0..8]; value for kSystemVerticalFilter carries float
// bits, not a numeric integer conversion. Option 8 selects GXSetMisc(1,8/0);
// its old "texture cache" label was unsupported by any use in this unit.
int Render_SetSystemOption(register int mode, int value) {
    volatile int stored = value;
    switch (mode) {
    case kSystemReservedOption:
        break;
    case kSystemVerticalFilter:
        Render_BuildVerticalFilterWeights(((EngineCopyFilterView*)g_engineState)->verticalFilter,
                                          *(volatile float*)&stored);
        GXSetCopyFilter(0, ((EngineCopyFilterView*)*(void* volatile*)&g_engineState)->samplePattern, 1,
                        ((EngineCopyFilterView*)g_engineState)->verticalFilter);
        break;
    case kSystemAsyncFifo:
        if (s_gxRendererInitialized != 0) {
            if ((stored != 0) ^ (((EngineSystemOptionsView*)g_engineState)->options[kSystemAsyncFifo] != 0)) {
                Render_WaitForDrawCompletion();
                Render_InitializeAndBindGXFifo(
                    (bool)*(unsigned int*)&((EngineSystemOptionsView*)g_engineState)->options[kSystemAsyncFifo],
                    (bool)stored, 0);
                GXDraw_SetSyncCallback(Render_OnDrawSyncToken);
                GXDraw_SetDoneCallback(Render_OnDrawDone);
            }
        }
        break;
    case kSystemResetGX:
        if (stored != 0)
            Engine_InitializeGXPixelAndTevDefaults();
        break;
    case kSystemMiscToken1:
        if (stored != 0) {
            GXSetMisc(1, 8);
        } else {
            GXSetMisc(1, 0);
        }
        break;
    }
    ((EngineSystemOptionsView*)g_engineState)->options[mode] = stored;
    return 1;
}
#pragma optimize_for_size reset
#pragma optimization_level reset

// Return the cached index-buffer address and base vertex.
// Both outputs are required; the buffer address is borrowed. Always returns 1.
int Engine_GetIndexBuffer(int* out0, int* out1) {
    *out0 = ((volatile QueueState*)g_engineState)->buffer;
    *out1 = ((volatile QueueState*)g_engineState)->baseVertex;
    return 1;
}

// Change one light enable word and invalidate lighting.
// index is 0..7; retains value verbatim (does not normalize to Boolean). Returns 1.
int Engine_SetLightEnabled(int index, int value) {
    (*(volatile int*)&AsEngineState(g_engineState)->lightingStateDirty) = 1;
    volatile char* p = (volatile char*)g_engineState;
    p += index * 4;
    *(volatile int*)(p + 0x1440) = value;
    return 1;
}

// Apply cached state changes before the next draw.
int Engine_FlushRenderState() {
    Engine_ApplyStateBeforeDraw();
    return 1;
}

// Copy the cached 68-byte material into the caller buffer.
int Engine_GetMaterial(void* materialOut) {
    volatile void* saved = materialOut;
    RenderMaterial_Copy((void*)saved, &((EngineLightingView*)g_engineState)->material);
    (void)saved;
    return 1;
}

// Copy four scissor words in their original volatile access order; buffers need 16 bytes.
void ScissorRect_Copy(unsigned int* dst, const unsigned int* src) {
    volatile unsigned int* d = (volatile unsigned int*)dst;
    const volatile unsigned int* s = (const volatile unsigned int*)src;
    d[0] = s[0];
    d[1] = s[1];
    d[2] = s[2];
    d[3] = s[3];
}

#pragma peephole off
// Wait for the GX sync token in asynchronous mode, or block with GXDrawDone.
void Render_WaitForDrawCompletion() {
    if (((EngineSystemOptionsView*)g_engineState)->options[kSystemAsyncFifo]) {
        GXSetDrawSync(kDrawCompletionToken);
        while (GXReadDrawSync() != kDrawCompletionToken) {
        }
    } else {
        GXDrawDone();
    }
}
#pragma peephole reset

// Initialize an empty byte buffer; volatile address temporaries preserve compiler ordering.
void ByteBuffer_InitEmpty(void* p) {
    volatile void* a = p;
    volatile void* b = (void*)a;
    volatile void* c = (void*)b;
    volatile void* d = (void*)c;
    volatile void* e = &((ByteBufferStorage*)d)->cap;
    volatile void* f = &((ByteBufferStorage*)d)->cap;
    volatile void* g = &((ByteBufferStorage*)d)->cap;
    volatile void* slots[5];
    slots[0] = a;
    slots[1] = b;
    slots[2] = c;
    slots[3] = d;
    slots[4] = e;
    *(int*)p = 0;
    *((int*)p + 1) = 0;
    *((int*)p + 2) = 0;
    *(volatile void**)&slots[0] = f;
    *(volatile void**)&slots[1] = g;
}

// Run byte-buffer cleanup and optionally free its storage object.
void* ByteBuffer_DestroyBase(void* p, short deletingFlag) {
    volatile void* saved = p;
    if (saved != 0) {
        if (saved != 0)
            ByteBuffer_ReleaseAllocation((void*)saved, 0);
        if (deletingFlag > 0)
            __dl__FPv((void*)saved);
    }
    return p;
}

#pragma optimization_level 0
// Copy the cached scissor rectangle into the caller buffer.
int Engine_GetScissorRect(void* scissorRectOut) {
    ScissorRect_Copy((unsigned int*)scissorRectOut,
                     (const unsigned int*)(((EngineLightingView*)g_engineState)->scissor));
    return 1;
}
#pragma optimization_level reset

#pragma optimization_level 0
// Bind a vertex stream with a byte offset of zero.
void Engine_SetVertexStreamZeroOffset(void* stream, void* vertexBuffer, int stride) {
    Engine_SetVertexStream(stream, vertexBuffer, 0, stride);
}
#pragma optimization_level reset

// Cache the five-word VI output rectangle {x,y,width,height,format} at engine +4C.
// src requires 20 readable bytes. Coordinates/dimensions are output pixels;
// format is an encoded mode word, not a fifth coordinate. Copies all words,
// returns no status and does not submit GXSetViewport or GXSetScissor.
// Engine_ChangeDisplayMode / Engine_PresentFrame consume this separate cache
// (gx_renderer_state_and_resources.cpp); see EngineViRect there. Preserve word order and format when sizing.
void Engine_SetOutputRectangle(const int* src) {
    int* base = (int*)g_engineState;
    int* dst = base + (kOutputRectangleCacheOffset / 4);
    base[kOutputRectangleCacheOffset / 4] = src[0];
    base[kOutputRectangleCacheOffset / 4 + 1] = src[1];
    base[kOutputRectangleCacheOffset / 4 + 2] = src[2];
    base[kOutputRectangleCacheOffset / 4 + 3] = src[3];
    base[kOutputRectangleCacheOffset / 4 + 4] = src[4];
    (void)dst;
}

#pragma use_lmw_stmw on
// Upload a decoded texture and, when present, create and populate its palette.
// imageSource is a borrowed encoded-image buffer; imageByteCount is its byte length
// carried in a pointer-shaped ABI word (see ImageResource_LoadFileAndDecode).
// Outputs are texture/palette handles, not pixel buffers.
// Both output pointers are required and independent. The caller receives owned
// graphics-resource handles (release via GraphicsResource_Release); zero means
// no handle. creationOptions is required only when a palette is present and
// must expose paletteOptions at +10. The fifth ABI argument is unused.
// An absent palette returns success with *paletteHandleOut == 0. Palette-data
// failure returns 0 after texture creation; this routine does not roll back that
// texture or release the palette allocated before the failed data lookup.
// The mip-upload return is ignored. Thus 1 reports this control path completed,
// not independent validation of both graphics allocations. Preserve that behavior.
int Texture_LoadResourceWithOptionalPalette(void* imageSource, void* imageByteCount, unsigned int* textureHandleOut,
                                            unsigned int* paletteHandleOut, void* /*unused*/, void* creationOptions) {
    volatile char pad[0x1c];
    int paletteDestinationAddress, paletteSourceAddress;
    DecodedTextureDescription textureDescription;
    DecodedPaletteDescription paletteDescription;
    (void)pad;
    // Decode descriptions before publishing either resource handle.
    int decoded = gtl_Image_GetDescription(&textureDescription, &paletteDescription, imageSource, imageByteCount);
    if (decoded == 0) {
        *textureHandleOut = 0;
        *paletteHandleOut = 0;
        return 0;
    }
    Texture_CreateAndUploadMipChain(imageSource, imageByteCount, textureHandleOut, &textureDescription);
    if (paletteDescription.swizzledFormat == kNoImagePalette) {
        *paletteHandleOut = 0;
        return 1;
    }
    // Allocate a platform palette, copy the decoded bytes, then flush for GX access.
    // Supported entry counts map to 16/256-entry kinds (gtl_texture_vertex_formats.cpp); other counts
    // return -1 and are still passed on here. Platform RGBA8 deinterleaves two
    // halfword planes; packed RGBA8 copies entries (gtl_graphics_texture_buffer_resources.cpp). A format edit
    // must keep encoded bytes, mip description and palette representation together.
    ImageFormatAbiWord decodedPaletteFormat = gtl_ImageFormat_FromSwizzled(paletteDescription.swizzledFormat);
    ImageFormatAbiWord platformPaletteFormat =
        gtl_ImageFormat_ToPlatformFormat(decodedPaletteFormat, kImagePlatformWii);
    int paletteType = TexturePalette_GetTypeForEntryCount(paletteDescription.entryCount);
    int paletteOptions = ((TextureCreationOptions*)creationOptions)->paletteOptions;
    TexturePalette_Create((void*)paletteType, (void*)0, platformPaletteFormat, (void*)paletteOptions,
                          (void*)paletteHandleOut, 0);
    decoded = gtl_Image_GetPaletteData(&paletteSourceAddress, imageSource, imageByteCount);
    if (decoded == 0) {
        *paletteHandleOut = 0;
        return 0;
    }
    unsigned int paletteHandle = *paletteHandleOut;
    TexturePalette_GetData((void*)paletteHandle, &paletteDestinationAddress, 0);
    unsigned int destinationAddress = paletteDestinationAddress;
    unsigned int sourceAddress = paletteSourceAddress;
    TexturePalette_CopyOrDeinterleaveEntries(destinationAddress, sourceAddress, (void*)paletteType,
                                             platformPaletteFormat);
    TexturePalette_Flush((void*)paletteHandle);
    return 1;
}

int ByteBuffer_CalculateGrowthCapacity(void* obj, int amount);

extern char s_byteBufferLengthErrorStrings[];
extern void* memset(void*, int, unsigned int);
extern void* memcpy(void*, const void*, unsigned int);

struct TemporaryByteBufferStorage {
    char* volatile buf;
    volatile unsigned int count;
    volatile unsigned int cap;
    void* volatile alloc;
    volatile unsigned int off;
};

#pragma push
#pragma auto_inline on
// ResizeTmp keeps its linker-visible destructor name; it owns the temporary allocation.
struct ResizeTmp : TemporaryByteBufferStorage {
    // Release whichever allocation remains owned by the temporary after the swap.
    ~ResizeTmp() {
        dtor_8042AE7C(this, -1);
    }
};
#pragma pop

#pragma push
#pragma optimization_level 2
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma optimize_for_size on
#pragma use_lmw_stmw on
#pragma auto_inline on
// Refresh capacity before size and publish the allocator address after both reads.
// Keep this ordering: the buffer view is volatile and the second bounds check
// intentionally reloads fields after the first (delta > capacity) check.
static inline unsigned int ReloadSizeForCapacityCheck(volatile ByteBufferStorage* buffer, unsigned int& capacity,
                                                      void* volatile& allocatorAddress, ByteBufferStorage* owner) {
    capacity = buffer->cap;
    unsigned int size = buffer->size;
    allocatorAddress = &owner->cap;
    return size;
}
// References to caller-frame commit scratch; this view owns no allocations.
struct ByteBufferCommitAbi {
    char* volatile& oldAllocationForSwap;
    void* volatile& scratchCommitTempCapacity;
    void* volatile& scratchCommitOwnerCapacity;
    volatile char& emptySwapTag;
    void* volatile& scratchSwapOwner;
    void* volatile& scratchSwapTemporary;
};

// Publish replacement capacity/data/size; the temporary adopts the retired allocation.
// Logical inputs are owner/live/storage; output is the published owner and retired
// storage.buf. All remaining references are caller-frame ABI scratch, not resources.
// Publish replacement storage; the temporary adopts the retired allocation.
static inline void CommitByteBufferGrowth(ByteBufferStorage* owner, volatile ByteBufferStorage* live,
                                          ResizeTmp& storage, ResizeTmp* temporaryOwner,
                                          const ByteBufferCommitAbi& abi) {
    abi.scratchCommitTempCapacity = (void*)&storage.cap;
    abi.scratchCommitOwnerCapacity = &owner->cap;
    abi.emptySwapTag = 0;
    // Growth phase 3: commit data/capacity/size; storage takes the old allocation.
    live->size = 0;
    Swap_Int32(&owner->cap, (void*)&storage.cap);
    char* newData;
    char* oldData = live->data;
    newData = storage.buf;
    abi.oldAllocationForSwap = oldData;
    live->data = newData; // Container store: publish the replacement allocation.
    char* retiredAllocation = abi.oldAllocationForSwap;
    // Write-only compiler scratch: these two stores transfer no ownership.
    abi.scratchSwapOwner = owner;
    abi.scratchSwapTemporary = (void*)temporaryOwner;
    storage.buf = retiredAllocation; // Guard now frees the retired allocation.
    Swap_Int32(&owner->size, (void*)&storage.count);
}

// Copy the preserved prefix into replacement storage and zero the moved-from bytes.
// The caller owns storage and ABI scratch; this phase transfers no allocation ownership.
// Prefix-copy scratch and moved-byte outputs; the live owner is still unchanged.
struct ByteBufferPrefixAbi {
    volatile char& emptyAppendTag;
    volatile char& emptyMoveTag;
    volatile char& emptyCommitCountTag;
    char* volatile& scratchPrefixEnd;
    char* volatile& scratchPrefixBegin;
    char* volatile& scratchOldEnd;
    char* volatile& movedSource;
    volatile unsigned int& movedByteCount;
};

// Copy and zero the old prefix, updating temporary counts through the borrowed ABI view.
static inline void MoveByteBufferPrefix(TemporaryByteBufferStorage& storage, volatile ByteBufferStorage* live,
                                        unsigned int addedBytes, const ByteBufferPrefixAbi& abi) {
    unsigned int copiedPrefixBytes;
    unsigned int initializedBytes;
    char* prefixDestination;
    unsigned int newPrefixOffset;
    unsigned int prefixBytes;
    char* freshBuffer;
    char* oldEnd;
    unsigned int oldSizeReload;
    char* oldBaseReload;
    char* oldBegin;
    unsigned int prefixInsertionOffset;
    unsigned int newCount;
    newCount = storage.count + addedBytes;
    prefixInsertionOffset = storage.off;
    abi.emptyAppendTag = 0;
    storage.count = newCount;
    oldBegin = live->data;
    oldBaseReload = live->data;
    oldSizeReload = live->size;
    abi.emptyMoveTag = 0;
    oldEnd = oldBaseReload + oldSizeReload;
    freshBuffer = *(char* volatile*)&storage.buf;
    prefixBytes = oldEnd - oldBegin;
    abi.scratchPrefixEnd = oldEnd;
    newPrefixOffset = prefixInsertionOffset - prefixBytes;
    storage.off = newPrefixOffset;
    prefixDestination = (char*)((unsigned int)freshBuffer + newPrefixOffset);
    abi.scratchPrefixBegin = oldBegin;
    abi.scratchOldEnd = oldEnd;
    abi.movedSource = oldBegin;
    abi.movedByteCount = prefixBytes;
    memcpy(prefixDestination, oldBegin, prefixBytes);
    memset(abi.movedSource, 0, abi.movedByteCount);
    initializedBytes = storage.count;
    copiedPrefixBytes = abi.movedByteCount;
    abi.emptyCommitCountTag = 0;
    storage.count = initializedBytes + copiedPrefixBytes;
}

// Zero newly exposed bytes in replacement storage before the preserved prefix is inserted.
// The caller owns storage and ABI scratch; this phase transfers no allocation ownership.
// Suffix-initialization scratch; no allocation ownership is transferred.
struct ByteBufferSuffixAbi {
    volatile char& emptySuffixTag1;
    volatile char& emptySuffixTag2;
    char* volatile& scratchFreshBase1;
    char* volatile& scratchFreshBase2;
    char* volatile& scratchFreshBase3;
    void* volatile& scratchSuffixCapacityAddress;
};

// Zero the newly exposed suffix before copying the old prefix into its reserved gap.
static inline void ZeroByteBufferGrowthSuffix(TemporaryByteBufferStorage& storage, volatile ByteBufferStorage* live,
                                              char* allocated, unsigned int addedBytes,
                                              const ByteBufferSuffixAbi& abi) {
    unsigned int initializedSuffixBytes;
    char* freshBase;
    char* suffixStart;
    unsigned int oldSize;
    oldSize = live->size;
    abi.emptySuffixTag1 = 0;
    abi.emptySuffixTag2 = 0;
    suffixStart = *(char* volatile*)&storage.buf + oldSize;
    freshBase = allocated;
    abi.scratchFreshBase1 = freshBase;
    abi.scratchFreshBase2 = freshBase;
    abi.scratchFreshBase3 = freshBase;
    initializedSuffixBytes = storage.count;
    abi.scratchSuffixCapacityAddress = (void*)&storage.cap;
    storage.off = oldSize;
    // Growth phase 2: zero the suffix, copy the prefix, then clear the moved-from bytes.
    memset((void*)(initializedSuffixBytes + (unsigned int)suffixStart), 0, addedBytes);
}

// Allocate replacement capacity under the caller-owned cleanup guard; return the fresh byte base.
// Logical result is a fresh byte base owned by storage; addedBytes is a byte delta.
// abi borrows caller-frame scratch outputs; it does not choose allocator policy.
// Allocation leaves the live buffer untouched; later MoveByteBufferPrefix clears
// its old bytes BEFORE commit. An allocation exception leaves live ownership intact.
// Guarded allocation scratch; references preserve caller-frame ABI stores.
struct ByteBufferAllocationAbi {
    void* volatile& allocatorContextAddress;
    void* volatile& scratchOwnerCapacityAddress;
    unsigned int* volatile& ownerCapacityAddress;
    void* volatile& scratchTempCapacityAddress1;
    void* volatile& scratchTempCapacityAddress2;
    void* volatile& scratchTemporaryBase;
    void* volatile& scratchTemporaryOwner;
    volatile char& emptyAllocationTag;
    volatile unsigned int& allocationLimit;
    volatile char& emptyLimitTag;
    volatile unsigned int& requestedAllocationBytes;
    volatile char& emptyAllocatorTag;
    void* volatile& scratchAllocatorContext;
    volatile unsigned int& allocationLimitCopy;
    volatile unsigned int& scratchLimitCopy1;
    volatile unsigned int& scratchLimitCopy2;
    void* volatile& scratchAllocationContext;
};

// Allocate replacement bytes under the temporary guard; return their base address.
static inline char* AllocateByteBufferGrowth(ByteBufferStorage* owner, volatile ByteBufferStorage* live,
                                             ResizeTmp& storage, ResizeTmp*& temporaryOwner, unsigned int addedBytes,
                                             const ByteBufferAllocationAbi& abi) {
    unsigned int allocationSize;
    char* allocated;
    unsigned int maximumAllocationBytes;
    void* alloc;
    unsigned int growthCapacity;
    unsigned int amount;
    temporaryOwner = &storage;
    abi.allocatorContextAddress = &owner->cap;
    abi.scratchOwnerCapacityAddress = &owner->cap;
    abi.ownerCapacityAddress = (unsigned int*)(&owner->cap);
    storage.alloc = abi.allocatorContextAddress;
    storage.buf = 0;
    storage.count = 0;
    storage.cap = 0;
    storage.off = 0;
    amount = (live->size + addedBytes) - *abi.ownerCapacityAddress;
    abi.scratchTempCapacityAddress1 = (void*)&storage.cap;
    abi.scratchTempCapacityAddress2 = (void*)&storage.cap;
    abi.scratchTemporaryBase = (void*)temporaryOwner;
    abi.scratchTemporaryOwner = (void*)temporaryOwner;
    growthCapacity = ByteBuffer_CalculateGrowthCapacity(owner, amount);
    alloc = storage.alloc;
    abi.emptyAllocationTag = 0;
    abi.allocationLimit = 0xFFFFFFFFu;
    abi.emptyLimitTag = 0;
    abi.requestedAllocationBytes = growthCapacity;
    abi.emptyAllocatorTag = 0;
    abi.scratchAllocatorContext = alloc;
    abi.allocationLimitCopy = abi.allocationLimit;
    maximumAllocationBytes = abi.allocationLimitCopy;
    abi.scratchLimitCopy1 = maximumAllocationBytes;
    abi.scratchLimitCopy2 = maximumAllocationBytes;
    if (abi.requestedAllocationBytes > maximumAllocationBytes)
        std_ThrowLengthError();
    abi.scratchAllocationContext = storage.alloc;
    allocated = (char*)((void* (*)(unsigned int))(*(void**)g_graphicsMemoryCallbacks))(abi.requestedAllocationBytes);
    allocationSize = abi.requestedAllocationBytes;
    storage.buf = allocated;
    storage.cap = allocationSize;
    return allocated;
}

// Resize the byte buffer, zero new bytes, and transfer ownership through a growth temporary.
// buffer is required initialized storage; requestedBytes is an unsigned byte
// count, independent of texture dimensions. Existing data pointers are invalidated
// only by allocation growth. Overflow throws length_error; allocation failures
// follow the graphics allocator. Preserve RAII retirement and volatile reloads.
// Invariant: size <= cap, data owns cap bytes. Growth preserves [0, old size),
// zeros [old size, requestedBytes), then swaps ownership; the guard frees the old buffer.
// Shrink changes size only (retains capacity and truncated bytes); equality is a no-op.
// Resize outcomes (requestedBytes compared with current size):
//   equal: no stores; smaller: change size only;
//   larger within capacity: zero suffix, then grow size;
//   larger beyond capacity: allocate -> zero suffix -> move prefix -> commit -> retire.
// The first growth calculation is intentionally repeated by the allocator path.
// empty*Tag and scratch* locals preserve expanded allocator/cleanup stores;
// they are compiler scaffolding, not fields in either container. Phase-specific
// reference views borrow those exact locals without generating extra instructions. The remaining
// volatile locals carry actual allocation limits, moved ranges and swap values.
// Lifetime ledger for reading the expanded allocator wiring (N=old size,
// D=requestedBytes-N, C=new capacity). ByteBufferStorage={data,size,cap} is the
// public 12-byte owner; ResizeTmp is the only cleanup guard, never a second owner
// of live->data before commit. Temporary field meanings are:
//   buf = owned allocation; cap = allocation bytes; count = initialized bytes;
//   alloc = borrowed &owner->cap (allocator context); off = prefix insertion end.
// phase/helper                   live owner                   temporary
// AllocateByteBufferGrowth       old data,N,old cap           fresh buf,0,C,off=0
// ZeroByteBufferGrowthSuffix     same                        zero [N,N+D),off=N
// MoveByteBufferPrefix           old bytes now ZEROED        copy [0,N),count=N+D,off=0
// CommitByteBufferGrowth         fresh buf,N+D,C             old buf,0,old cap
// ~ResizeTmp                     unchanged                   frees old allocation
// Move's destination is fresh+(off-N), so off=N reserves exactly the old prefix.
// Commit clears live size, swaps capacities, publishes data, transfers old data
// into the guard, then swaps sizes; preserve this order and volatile reloads.
// Scratch views allocationAbi/suffixAbi/prefixAbi/commitAbi borrow caller-frame
// lvalues, not buffers. Repeated scratch copies and empty tags write stack lanes;
// only owner/live/storage fields carry ownership. Removing those volatile writes
// is a code change requiring equivalence proof, not a lifetime optimization.
// Exception boundary: growth/length/allocation can fail before prefix mutation;
// allocation-null is not checked before memset. No concurrent reader may observe
// the cleared old prefix or intermediate commit. A future allocator refactor must
// preserve the guard's destructor and this destructive-move publication contract.
void ByteBuffer_ResizeZeroFilled(ByteBufferStorage* buffer, register unsigned int requestedBytes) {
    register ByteBufferStorage* owner = buffer;
    register ResizeTmp* temporaryOwner;
    // ABI tags: write-only allocator and cleanup markers; no size/ownership policy.
    volatile char emptyShortfallTag, emptyGrowthTag, emptySwapTag, emptyCommitCountTag, emptyMoveTag, emptyAppendTag,
        emptySuffixTag2, emptySuffixTag1, emptyLimitTag, emptyAllocationTag, emptyAllocatorTag, emptyResizeTag,
        emptyCapacityTag, emptyShrinkTag1, emptyShrinkTag2;
    // Bounds/swap state: the volatile reload order is part of this reconstruction.
    volatile unsigned int scratchCapacityShortfall, scratchGrowthCapacity;
    char* volatile oldAllocationForSwap;
    volatile unsigned int movedByteCount;
    char* volatile scratchOldEnd;
    char* volatile movedSource;
    volatile unsigned int allocationLimit, requestedAllocationBytes;
    void* volatile allocatorContextAddress;
    void* volatile scratchTemporaryOwner;
    volatile unsigned int scratchTruncatedBytes;
    void* volatile scratchInitialCapacityAddress;
    void* volatile scratchReloadCapacityAddress;
    void* volatile scratchGrowthCapacityAddress;
    char* volatile scratchSuffixStart;
    void* volatile scratchOwnerCapacityAddress;
    void* volatile scratchTempCapacityAddress1;
    void* volatile scratchTempCapacityAddress2;
    void* volatile scratchTemporaryBase;
    unsigned int* volatile ownerCapacityAddress;
    void* volatile scratchAllocatorContext;
    volatile unsigned int allocationLimitCopy, scratchLimitCopy1, scratchLimitCopy2;
    void* volatile scratchAllocationContext;
    // Suffix/prefix/commit scratch destinations, in original frame declaration order.
    char* volatile scratchFreshBase1;
    char* volatile scratchFreshBase2;
    char* volatile scratchFreshBase3;
    void* volatile scratchSuffixCapacityAddress;
    char* volatile scratchPrefixEnd;
    char* volatile scratchPrefixBegin;
    void* volatile scratchCommitTempCapacity;
    void* volatile scratchCommitOwnerCapacity;
    void* volatile scratchSwapOwner;
    void* volatile scratchSwapTemporary;
    register volatile ByteBufferStorage* live = owner;
    if (requestedBytes > live->size) {
        unsigned int oldByteCount = live->size;
        emptyResizeTag = 0;
        register unsigned int addedBytes = requestedBytes - oldByteCount;
        unsigned int capacity = live->cap;
        scratchInitialCapacityAddress = &owner->cap;
        emptyCapacityTag = 0;
        // 0x8042A648: growth skips the in-place fill before buffer setup.
        // Refresh capacity/size only after the first bound check. Keep these sequenced reads:
        // a boolean temporary changes branch generation in this compiler.
        if (addedBytes > capacity ||
            ReloadSizeForCapacityCheck(live, capacity, scratchReloadCapacityAddress, owner) > capacity - addedBytes) {
            scratchGrowthCapacityAddress = &owner->cap;
            unsigned int requested = live->size + addedBytes;
            unsigned int oldCap = live->cap;
            unsigned int capacityShortfall = requested - oldCap;
            emptyShortfallTag = 0;
            scratchCapacityShortfall = capacityShortfall;
            emptyGrowthTag = 0;
            scratchGrowthCapacity = ByteBuffer_CalculateGrowthCapacity(owner, capacityShortfall);
        } else {
            // Capacity suffices: only the newly exposed suffix needs initialization.
            char* suffixBegin = live->data + live->size;
            scratchSuffixStart = suffixBegin;
            memset(suffixBegin, 0, addedBytes);
            live->size += addedBytes;
            return;
        }
        // Growth phase 1: the temporary owns fresh storage; owner still owns its old data.
        ResizeTmp storage;
        ByteBufferAllocationAbi allocationAbi = {allocatorContextAddress,
                                                 scratchOwnerCapacityAddress,
                                                 ownerCapacityAddress,
                                                 scratchTempCapacityAddress1,
                                                 scratchTempCapacityAddress2,
                                                 scratchTemporaryBase,
                                                 scratchTemporaryOwner,
                                                 emptyAllocationTag,
                                                 allocationLimit,
                                                 emptyLimitTag,
                                                 requestedAllocationBytes,
                                                 emptyAllocatorTag,
                                                 scratchAllocatorContext,
                                                 allocationLimitCopy,
                                                 scratchLimitCopy1,
                                                 scratchLimitCopy2,
                                                 scratchAllocationContext};
        char* allocated = AllocateByteBufferGrowth(owner, live, storage, temporaryOwner, addedBytes, allocationAbi);
        ByteBufferSuffixAbi suffixAbi = {emptySuffixTag1,   emptySuffixTag2,   scratchFreshBase1,
                                         scratchFreshBase2, scratchFreshBase3, scratchSuffixCapacityAddress};
        ZeroByteBufferGrowthSuffix(storage, live, allocated, addedBytes, suffixAbi);
        ByteBufferPrefixAbi prefixAbi = {emptyAppendTag,     emptyMoveTag,  emptyCommitCountTag, scratchPrefixEnd,
                                         scratchPrefixBegin, scratchOldEnd, movedSource,         movedByteCount};
        MoveByteBufferPrefix(storage, live, addedBytes, prefixAbi);
        ByteBufferCommitAbi commitAbi = {oldAllocationForSwap, scratchCommitTempCapacity, scratchCommitOwnerCapacity,
                                         emptySwapTag,         scratchSwapOwner,          scratchSwapTemporary};
        CommitByteBufferGrowth(owner, live, storage, temporaryOwner, commitAbi);
        return;
    }
    if (requestedBytes < live->size) {
        unsigned int truncatedBytes = live->size - requestedBytes;
        emptyShrinkTag1 = 0;
        unsigned int newSize = live->size - truncatedBytes;
        scratchTruncatedBytes = truncatedBytes;
        emptyShrinkTag2 = 0;
        live->size = newSize;
    }
}
#pragma pop

extern char __vt__Q23std12length_error[];
extern char __vt__Q23std11logic_error[];
extern char __vt__Q43std3tr16detail25shared_ptr_deleter_common[];
extern char __vt__Q23std9exception[];
extern char s_sharedCharDeleterVtable[];
extern void* __nwa__FUl(unsigned int);
extern void* __nw__FUl(unsigned int);
extern void __dt__Q33std6detail16char_array_guardFv(void*, short);

extern void __throw(const char*, void*, void*);
extern void __dt__Q23std12length_errorFv();

struct ExceptionMessageControlBlock {
    volatile void* vptr;
    volatile int refs;
    volatile int flags;
    volatile char* data;
};

struct LengthErrorObj {
    void* volatile vptr;
    volatile char* buffer;
    volatile ExceptionMessageControlBlock* rep;
};

struct MsgTmp {
    volatile char* ptr;
    volatile char* end;
};

#pragma push
#pragma optimization_level 1
#pragma auto_inline on
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma use_lmw_stmw on
#pragma optimize_for_size on
// Stack reconstruction state for the shared exception message; scratch fields are
// volatile ABI stores, while tmp guards the only not-yet-adopted allocation.
struct LengthErrorBuildSlots {
    volatile unsigned char copy, zero, ca, cb;
    void* volatile scratchMessageData;
    void* volatile scratchGuardForCleanup;
    MsgTmp* volatile guardForRelease;
    void* volatile scratchControlDataAddress;
    void* volatile scratchAdoptedDataAddress;
    void* volatile scratchControlBase;
    void* volatile scratchGuardAddress1;
    void* volatile scratchGuardAddress2;
    void* volatile scratchExceptionBase;
    void* volatile scratchThrowBase;
    void* volatile scratchBufferMember;
    char* volatile messageAllocation;
    void* volatile scratchReleasedPointer1;
    void* volatile scratchReleasedPointer2;
    void* volatile scratchAdoptedControlBlock;
    ExceptionMessageControlBlock* volatile allocatedControlBlock;
    void* volatile scratchThrowBufferMember;
    const char* volatile messageText;
    LengthErrorObj* volatile exceptionObject;
    void* volatile scratchThrowObject;
    MsgTmp tmp;
    LengthErrorObj exceptionToThrow;
};
#pragma push
#pragma always_inline on
// Adopt message bytes into an initialized shared deleter, retaining ABI tag stores.
static inline void InitializeLengthErrorMessageOwner(LengthErrorBuildSlots& state,
                                                     ExceptionMessageControlBlock* controlBlockUnderConstruction,
                                                     char* adoptedMessage) {
    unsigned char allocatorTag = state.copy;
    state.scratchControlBase = controlBlockUnderConstruction;
    state.ca = allocatorTag;
    controlBlockUnderConstruction->data = adoptedMessage;
    state.cb = allocatorTag;
    state.scratchAdoptedDataAddress = &controlBlockUnderConstruction->data;
    state.scratchControlDataAddress = &controlBlockUnderConstruction->data;
    state.scratchAdoptedControlBlock = controlBlockUnderConstruction;
}
#pragma pop

// Build and throw a standard length_error while guarding the allocated message string.
void std_ThrowLengthError() {
    // Ownership phases below mirror the standard exception's expanded constructor:
    // allocate message under a char-array guard -> attach shared deleter control block
    // -> disarm guard -> copy text/install derived vtable -> transfer to __throw.
    LengthErrorBuildSlots state;
    register LengthErrorObj* object;
    register char** messageMember;
    register ExceptionMessageControlBlock* controlBlockUnderConstruction;
    int destroyFlag;
    state.zero = 0;
    unsigned char z = state.zero;
    state.messageText = s_byteBufferLengthErrorStrings;
    object = &state.exceptionToThrow;
    messageMember = (char**)&object->buffer;
    object->vptr = __vt__Q23std9exception;
    object->vptr = __vt__Q23std11logic_error;
    const char* msg = state.messageText;
    state.exceptionObject = object;
    state.scratchExceptionBase = object;
    state.copy = z;
    // Message allocation is guard-owned until a control block adopts it.
    char* freshMessage = (char*)__nwa__FUl(strlen(msg) + 1);
    *(char* volatile*)messageMember = freshMessage;
    state.messageAllocation = freshMessage;
    *(char* volatile*)&state.tmp.ptr = freshMessage;
    *(volatile char* volatile*)&state.tmp.end = (volatile char*)&state.copy;
    state.scratchGuardAddress2 = (void*)&state.tmp;
    state.scratchGuardAddress1 = (void*)&state.tmp;
    ExceptionMessageControlBlock* freshControlBlock;
    try {
        freshControlBlock = (ExceptionMessageControlBlock*)__nw__FUl(0x10);
    } catch (...) {
        __dt__Q33std6detail16char_array_guardFv((void*)&state.tmp, -1);
        throw;
    }
    state.allocatedControlBlock = freshControlBlock;
    if (freshControlBlock != 0) {
        controlBlockUnderConstruction = state.allocatedControlBlock;
        *(void* volatile*)&controlBlockUnderConstruction->vptr = __vt__Q43std3tr16detail25shared_ptr_deleter_common;
        int one = 1;
        controlBlockUnderConstruction->refs = one;
        char* adoptedMessage = state.messageAllocation;
        controlBlockUnderConstruction->flags = one;
        *(void* volatile*)&controlBlockUnderConstruction->vptr = s_sharedCharDeleterVtable;
        InitializeLengthErrorMessageOwner(state, controlBlockUnderConstruction, adoptedMessage);
    }
    // Adopt into the exception and disarm temporary ownership before copying the text.
    ExceptionMessageControlBlock* control = state.allocatedControlBlock;
    state.guardForRelease = (MsgTmp*)&state.tmp;
    ((ExceptionMessageControlBlock* volatile*)messageMember)[1] = control;
    MsgTmp* release = state.guardForRelease;
    state.scratchGuardForCleanup = (void*)&state.tmp;
    unsigned int saved = (unsigned int)release->ptr;
    state.scratchReleasedPointer1 = (void*)saved;
    state.scratchReleasedPointer2 = (void*)saved;
    *(char* volatile*)&state.tmp.ptr = 0;
    destroyFlag = -1;
    __dt__Q33std6detail16char_array_guardFv((void*)&state.tmp, destroyFlag);
    char* data = *messageMember;
    state.scratchBufferMember = (void*)messageMember;
    const char* copyMessage = state.messageText;
    state.scratchMessageData = data;
    strcpy(data, copyMessage);
    LengthErrorObj* exceptionToThrow = state.exceptionObject;
    LengthErrorObj* exceptionVtableTarget = state.exceptionObject;
    void* lengthVt = __vt__Q23std12length_error;
    state.scratchThrowObject = exceptionToThrow;
    state.scratchThrowBufferMember = (void*)messageMember;
    state.scratchThrowBase = object;
    exceptionVtableTarget->vptr = lengthVt;
    // Throw transfers exception cleanup to the runtime and supplies its destructor.
    __throw(s_byteBufferLengthErrorStrings + 0x14, exceptionToThrow, (void*)__dt__Q23std12length_errorFv);
}
#pragma pop

#pragma push
#pragma optimization_level 1
#pragma optimize_for_size on
#pragma auto_inline on
#pragma opt_lifetimes off
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
struct LoadBuf : ByteBufferStorage {
    // Begin with no owned file-buffer allocation.
    LoadBuf() {
        ByteBuffer_InitEmpty(this);
    }
    // Release the temporary encoded-file bytes on normal return or unwinding.
    ~LoadBuf() {
        ByteBuffer_DeletingDtor(this, -1);
    }
};

#pragma opt_strength_reduction off
#pragma optimize_for_size on
// Read an image file into a buffer whose SIZE is rounded up to 32 bytes, then decode handles.
// filenameWord is borrowed; creationOptionsWord is a pointer-shaped options ABI word.
// The temporary owns the encoded bytes. On successful read the file is closed before decode;
// the existing read-failure path returns without a close, and is preserved here.
// Open failure clears both required outputs; read failure leaves them untouched.
// Decode receives the rounded buffer SIZE, not bytesRead or fileByteCount; tail
// bytes remain zero. Preserve that distinction for replacement GSR assets.
int ImageResource_LoadFileAndDecode(register void* filenameWord, register unsigned int* textureHandleOut,
                                    register unsigned int* paletteHandleOut, register int unusedDecodeOptionWord,
                                    register int creationOptionsWord) {
    unsigned int initialZero;
    int fileSizeAuxiliaryWord;
    int bytesRead;
    volatile int savedCreationOptionsWord;
    volatile int savedUnusedDecodeOptionWord;
    volatile int savedFilenameWord;
    volatile unsigned int stackArgumentZeroC;
    volatile unsigned int stackArgumentZero8;
    register unsigned int fileHandle;
    register int fileByteCount;
    register unsigned int* paletteOutput;
    register unsigned int* textureOutput;
    register int failure;
    register int result;
    unsigned int invalidFileHandle;
    initialZero = 0;
    textureOutput = textureHandleOut;
    paletteOutput = paletteHandleOut;
    savedFilenameWord = (int)filenameWord;
    savedUnusedDecodeOptionWord = unusedDecodeOptionWord;
    savedCreationOptionsWord = creationOptionsWord;
    stackArgumentZero8 = initialZero;
    stackArgumentZeroC = initialZero;
    // The final two zero arguments are explicit ABI stack words for FileIO_OpenFile.
    fileHandle = FileIO_OpenFile((unsigned int)savedFilenameWord, kFileReadAccess, 0, 0, kFileOpenDisposition3,
                                 kFileOpenAttribute80, 0, -1, initialZero, initialZero);
    invalidFileHandle = -1;
    if (fileHandle == invalidFileHandle) {
        *textureOutput = 0;
        *paletteOutput = 0;
        return 0;
    }
    fileByteCount = FileIO_GetFileSize((int)fileHandle, &fileSizeAuxiliaryWord, 0, 0);
    LoadBuf encodedFile;
    ByteBuffer_ResizeZeroFilled(&encodedFile, Math_RoundUpToMultiple(fileByteCount, kImageFileAlignment));
    if (FileIO_ReadFile((void*)fileHandle, encodedFile.data, fileByteCount, (int)&bytesRead, 0, -1, 0, 0) == 0) {
        failure = 0;
        return failure;
    }
    FileIO_CloseFile((int)fileHandle);
    void* encodedBytes = encodedFile.data;
    void* encodedByteCountWord = (void*)encodedFile.size;
    result =
        Texture_LoadResourceWithOptionalPalette(encodedBytes, encodedByteCountWord, textureOutput, paletteOutput,
                                                (void*)savedUnusedDecodeOptionWord, (void*)savedCreationOptionsWord);
    return result;
}
#pragma optimization_level reset
#pragma pop
} // extern "C"

// Copy sixteen material color words and the shininess float in the original order.
extern "C" void RenderMaterial_Copy(void* dst, const void* src) {
    volatile unsigned int* d = (volatile unsigned int*)dst;
    const volatile unsigned int* s = (const volatile unsigned int*)src;
    // Explicit pairs preserve the two-read/two-write schedule without helper macros.
    {
        unsigned int second;
        unsigned int first;
        first = s[0];
        second = s[1];
        d[0] = first;
        d[1] = second;
    }
    {
        unsigned int second;
        unsigned int first;
        first = s[2];
        second = s[3];
        d[2] = first;
        d[3] = second;
    }
    {
        unsigned int second;
        unsigned int first;
        first = s[4];
        second = s[5];
        d[4] = first;
        d[5] = second;
    }
    {
        unsigned int second;
        unsigned int first;
        first = s[6];
        second = s[7];
        d[6] = first;
        d[7] = second;
    }
    {
        unsigned int second;
        unsigned int first;
        first = s[8];
        second = s[9];
        d[8] = first;
        d[9] = second;
    }
    {
        unsigned int second;
        unsigned int first;
        first = s[10];
        second = s[11];
        d[10] = first;
        d[11] = second;
    }
    {
        unsigned int second;
        unsigned int first;
        first = s[12];
        second = s[13];
        d[12] = first;
        d[13] = second;
    }
    {
        unsigned int second;
        unsigned int first;
        first = s[14];
        second = s[15];
        d[14] = first;
        d[15] = second;
    }

    ((volatile CachedRenderMaterial*)dst)->power = ((const volatile CachedRenderMaterial*)src)->power;
}

// Discard the temporary buffer range, release its allocation, and optionally delete the object.
extern "C" void* dtor_8042AE7C(void* p, short flags) {
    volatile short savedFlags = flags;
    if (p != 0) {
        volatile unsigned char* base = (volatile unsigned char*)p;
        volatile unsigned char* end =
            base + *(volatile unsigned int*)(base + 0x10) + *(volatile unsigned int*)(base + 4);
        volatile unsigned char* begin = base + *(volatile unsigned int*)base + *(volatile unsigned int*)(base + 0x10);
        while (end > begin) {
            --end;
            (void)*(volatile void**)(base + 0x0c);
        }
        *(unsigned int*)(base + 4) = 0;
        CallbackByteBuffer_Destroy(p, 0);
        (void)base;
        (void)end;
        (void)begin;
        if (savedFlags > 0)
            __dl__FPv(p);
    }
    return p;
}

// Release an allocated byte buffer through the graphics allocator callback.
// p may be null; non-null storage is ByteBufferStorage and callback table must
// be initialized if data != 0. Frees even with size==0, sets size=0 but leaves
// data/cap stale; do not release twice or reuse without reinitialization. Positive
// flags also delete the header; returned address then has no live lifetime.
extern "C" void* ByteBuffer_ReleaseAllocation(void* p, short flags) {
    volatile short savedFlags = flags;
    void* self = p;
    if (self != 0) {
        unsigned int* fields = (unsigned int*)p;
        if (fields[0] != 0) {
            int oldCount = fields[1];
            unsigned int* p8a = (unsigned int*)(&((ByteBufferStorage*)p)->cap);
            unsigned char zero = 0;
            unsigned int* p8b = (unsigned int*)(&((ByteBufferStorage*)p)->cap);
            (void)p8a;
            (void)p8b;
            fields[1] = fields[1] - oldCount;
            volatile unsigned char z = zero;
            (void)z;
            void** dispatch = (void**)g_graphicsMemoryCallbacks;
            typedef void (*Callback)(void*, void*);
            Callback callback = *(Callback*)&dispatch[1];
            callback((void*)fields[0], (void*)dispatch);
        }
        if (savedFlags > 0)
            __dl__FPv(p);
    }
    return p;
}

#pragma optimize_for_size on
#pragma use_lmw_stmw on
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma optimization_level 2
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
#pragma scheduling reset
// Choose incremental or saturated byte-buffer capacity and reject overflow.
// buffer is required; minimumExtraBytes is interpreted as unsigned additional
// capacity (not final size), even though the ABI parameter is signed. Result is
// a u32 byte capacity carried in int: small max(extra,3*(cap+1)/5), medium
// max(extra,(cap+1)/2), large UINT_MAX, each added to cap except saturation.
// Throws length_error when extra exceeds UINT_MAX-cap; preserve unsigned math.
extern "C" int ByteBuffer_CalculateGrowthCapacity(void* buffer, int minimumExtraBytes) {
    void* owner;
    unsigned int limit;
    unsigned int capacity;
    owner = buffer;
    struct Locals {
        unsigned int* volatile scratchSelectedMedium2;
        unsigned int* volatile scratchSelectedMedium1;
        unsigned int* volatile selectedMediumIncrement;
        unsigned int* volatile scratchSelectedSmall2;
        unsigned int* volatile scratchSelectedSmall1;
        unsigned int* volatile selectedSmallIncrement;
        void* volatile scratchCapacityAddressReload;
        volatile unsigned int scratchLimit2;
        volatile unsigned int scratchLimit1;
        volatile unsigned int limitWord;
        void* volatile scratchCapacityAddress;
        volatile int allBitsSetSigned;
        unsigned int mediumIncrement;
        unsigned int smallIncrement;
        volatile unsigned int oneThirdLimit;
    } s;
    int initial = -1;
    s.allBitsSetSigned = initial;
    s.scratchCapacityAddress = &((ByteBufferStorage*)owner)->cap;
    if (initial < 0)
        s.limitWord = s.allBitsSetSigned;
    else
        s.limitWord = initial;
    limit = s.limitWord;
    capacity = *(unsigned int*)(&((ByteBufferStorage*)owner)->cap);
    s.scratchCapacityAddressReload = &((ByteBufferStorage*)owner)->cap;
    s.scratchLimit1 = limit;
    s.scratchLimit2 = limit;
    if ((unsigned int)minimumExtraBytes > limit - capacity)
        std_ThrowLengthError();
    unsigned int oneThirdLimit = limit / 3u;
    s.oneThirdLimit = oneThirdLimit;
    if (capacity < oneThirdLimit) {
        unsigned int smallIncrement = ((capacity + 1) * 3u) / 5u;
        s.smallIncrement = smallIncrement;
        if (smallIncrement < (unsigned int)minimumExtraBytes)
            s.selectedSmallIncrement = (unsigned int*)&minimumExtraBytes;
        else
            s.selectedSmallIncrement = &s.smallIncrement;
        unsigned int* ptr = s.selectedSmallIncrement;
        unsigned int value = *ptr;
        s.scratchSelectedSmall1 = ptr;
        s.scratchSelectedSmall2 = ptr;
        return capacity + value;
    }
    if (capacity < s.oneThirdLimit * 2u) {
        unsigned int mediumIncrement = (capacity + 1) >> 1;
        s.mediumIncrement = mediumIncrement;
        if (mediumIncrement < (unsigned int)minimumExtraBytes)
            s.selectedMediumIncrement = (unsigned int*)&minimumExtraBytes;
        else
            s.selectedMediumIncrement = &s.mediumIncrement;
        unsigned int* ptr = s.selectedMediumIncrement;
        unsigned int value = *ptr;
        s.scratchSelectedMedium1 = ptr;
        s.scratchSelectedMedium2 = ptr;
        return capacity + value;
    }
    return limit;
}
