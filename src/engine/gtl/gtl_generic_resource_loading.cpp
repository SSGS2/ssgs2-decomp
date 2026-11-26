// GTL model/environment/animation resources: copy input files into refcounted
// buffers and extract render descriptions. No gameplay tuning knobs here.
// API/mod lookup:
// - CreateFromMemory: allocation, input-copy and retained-buffer policy below.
// - Material factor/flags: GenericMaterial_GetDescription -> ApplyMaterial in
//   gtl_generic_renderer_material.cpp -> Engine_SetMaterial. MaterialDescriptionSource maps used bytes.
// - Light intensity/range/direction: GenericLight_GetDescription -> environment
//   setup in gtl_generic_environment.cpp -> Engine_SetLight. LightDescriptionSource maps records.
// - Fog/ambient packed colors: GetColor accessors, no color-space conversion.
// - GenericResource_Initialize installs allocate/free callbacks s_genericResourceAllocatorFunctions;
//   resource wrappers must be released through the same allocator family.
// Default model descriptor: s_defaultModelDescriptor; constants
// kMaterialFactorChannelCount/94/98 are shared 3.0f/1.0f/0.0f, not private settings. 2098 is
// also zero time in the model/material-animation wrappers gtl_generic_modeling_resource_eval.cpp/gtl_material_animation_eval.cpp.

#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>
#include "classes/GtlIGenericModelingResource.h"
#include "classes/GtlIGenericEnvironmentResource.h"

struct IGenericResourceFileBase {
    char pad[4];
};

class IGenericResourceFileV : public IGenericResourceFileBase {
public:
    virtual void Release(int flags);
};

extern "C" void* s_genericResourceAllocatorFunctions[2];
extern "C" int Object_GetField8(void* o);

namespace gtl {

struct RefBase {
    int rc;
    // New wrapper owns one reference; buffer ownership is tracked separately.
    RefBase() {
        rc = 1;
    }
};

class IGenericResourceFile : public RefBase {
public:
    // Retain the required backing buffer; final wrapper teardown drops this ref.
    IGenericResourceFile(char* buffer) {
        buf = buffer;
        ((int*)buffer)[0] = ((int*)buffer)[0] + 1;
    }
    virtual ~IGenericResourceFile();
    // Release the retained buffer, restore the base vtable and optionally free the wrapper.
    void* Destroy(int flags);
    // Number of elements stored under a chunk tag in the retained buffer.
    int GetChunkElementCount(int chunkTag);
    char* buf;
};

class IGenericAnimationResource : public IGenericResourceFile {
public:
    // Install the retained animation backing file; its data accessor is invoked
    // for the original constructor side effect even though its value is unused.
    IGenericAnimationResource(char* buffer) : IGenericResourceFile(buffer) {
        Object_GetField8(buffer);
    }
    virtual ~IGenericAnimationResource();
    // Copy an animation file and construct its wrapper; the new reference is returned through out.
    static int CreateFromMemory(void* data, int size, void** out);
    // Use the allocator installed by GenericResource_Initialize, paired below.
    static void* operator new(size_t n) {
        return ((void* (*)(unsigned long))s_genericResourceAllocatorFunctions[0])(n);
    }
    // Matching release family, including constructor-unwind deletion.
    static void operator delete(void* p) {
        ((void (*)(void*))s_genericResourceAllocatorFunctions[1])(p);
    }
};

}

// Allocate an animation wrapper and retain the required buffer in its base.
static inline gtl::IGenericAnimationResource* CreateAnimationResource(char* buffer) {
    return new gtl::IGenericAnimationResource(buffer);
}

extern "C" {

extern void* s_genericResourceAllocatorFunctions[2];
extern void* s_genericEnvironmentLightSlotCount;
extern float kMaterialFactorChannelCount;
extern float kFloat_1_GtlResource;
extern float kFloat_0_GtlResource;
extern void* __vt__Q23gtl20IGenericResourceFile[];
extern void* __vt__Q23gtl27IGenericEnvironmentResource[];
extern void* __vt__Q23gtl25IGenericAnimationResource[];
extern char s_defaultModelDescriptor[];

void gtl_PixelCopy_Copy4Bytes_1(void* dst, void* src);
int Renderer_InitializeDefaults(void* self);
void gtl_GenericRenderer_InitializeContext(void* self);
int Object_GetFieldC(void* o);
int Object_GetField8(void* o);
int gtl_ResourceFile_GetChunkElementCount(int f, int a, int b);
void __dl__FPv(void* p);
void gtl_PixelCopy_Copy16Bytes(void* dst, const void* src);
void Vec4_Scale(void* dst, void* src, float s);
void Vec3_Subtract(void* dst, const void* a, const void* b);
void Vec3_Normalize(void* dst, void* src);
extern "C" void Create__Q23gtl11IG3DXBufferFPvPPv(int size, void** out, int flags);

typedef struct {
    CW_BITS5(unsigned int a : 25,
             unsigned int bypassSpecularAlphaFactor : 1, // numeric mask 0x40: bypass factor adjustment
             unsigned int c : 4,
             unsigned int useFactorDirectly : 1, // numeric mask 0x02: use factor directly; clear = 1-factor
             unsigned int useScalarFactor : 1)   // numeric mask 0x01: scalar +5C; clear = mean of +50/54/58
} MatFlagBits;

// Native 32-bit Wii records. Prefix bytes are copied verbatim to the engine
// description; reserved fields are not interpreted by this adapter.
// Color order verified by CachedRenderMaterial / Engine_SetMaterial engine_render_cache_and_texture_load.cpp.
struct MaterialEngineDescription {
    float diffuse[4], ambient[4], specular[4], emissive[4];
    float power;
};
struct MaterialDescriptionSource {
    MaterialEngineDescription description;
    char reserved44[0x0C];
    float factorChannels[3]; // +50..58: dimensionless factor; sum [0], then [1], then [2]
    float scalarFactor;      // +5C: alternate dimensionless factor
    MatFlagBits flags;       // +60: bypass/source/complement policy above
};
struct LightDescriptionSource {
    int type;         // +00: 1 point / 2 spot / 3 directional (GX consumer)
    float diffuse[4]; // +04: RGBA, all four components scaled by intensity
    char reserved14[0x20];
    float position[3]; // +34: world XYZ, same distance units as target/range
    char reserved40[0x28];
    float intensity; // +68: dimensionless multiplier, no clamping
    char reserved6C[4];
    float range;     // +70: distance units of authored position
    float coneStart; // +74: radians, passed unchanged to description +60
    float coneWidth; // +78: radians, added to start for end angle at +64
    float target[3]; // +7C: world target; spot direction = normalize(target-position)
};
typedef char MaterialDescriptionSource_flags_offset[(int)&((MaterialDescriptionSource*)0)->flags == 0x60 ? 1 : -1];
typedef char LightDescriptionSource_size[sizeof(LightDescriptionSource) == 0x88 ? 1 : -1];

#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
// Release the allocation at self+0 after zeroing word +4 by self-subtraction;
// optionally delete self for flags>0. Null self succeeds. The duplicated guards
// and subtraction preserve recovered destructor shape, not a reference release.
void* dtor_803D7FE8(char* self, int flags) {
    void* p;

    if (self == 0) {
        goto end;
    }
    if (self == 0) {
        goto flags_check;
    }
    if (self == 0) {
        goto flags_check;
    }
    p = *(void**)self;
    if (p == 0) {
        goto flags_check;
    }
    int* a = (int*)(self + 4);
    int* b = (int*)(self + 4);
    *a = *a - *b;
    __dl__FPv(p);
flags_check:
    if (flags > 0) {
        __dl__FPv(self);
    }
end:
    return self;
}
#pragma pop

// Tail-forward the incoming allocation ABI to the installed callback. This
// recovered no-argument declaration intentionally leaves argument registers live.
void gtl_GenericResource_AllocateModel(void) {
    ((void (*)(void))s_genericResourceAllocatorFunctions[0])();
}

// Free a wrapper allocation with the matching installed resource callback.
void gtl_GenericResource_FreeWrapper(void* self) {
    void** vt = s_genericResourceAllocatorFunctions;
    ((void (*)(void*))vt[1])(self);
}
}

// Shared CreateFromMemory contract (all three factories): required data spans
// size>=0 readable bytes; required out receives a NEW wrapper reference or null.
// Bytes are copied before returning; input is borrowed only during the call.
// Buffer creation must succeed (its output is dereferenced unchecked); errors
// from allocation/copy/constructors propagate, with no general rollback guard.
// Returns 1 even when wrapper allocation yields null: inspect *out, not status.
// Success: wrapper retains the copied buffer; the factory drops its local ref.
// Release via the wrapper's refcount interface; final buffer release calls its
// virtual Release(1). No synchronization; install callbacks before these calls.
// Model-specific: def may be null for s_defaultModelDescriptor defaults; nonnull descriptor
// is borrowed during construction (0x54-byte wrapper, constructor gtl_generic_model_resource.cpp).
int gtl::IGenericModelingResource::CreateFromMemory(void* data, int size, void** out, void* def) {
    char* copiedBuffer;
    char* retainedBuffer;
    char* resource;
    char* localBufferReference;
    int remainingBufferRefs;

    Create__Q23gtl11IG3DXBufferFPvPPv(size, (void**)&copiedBuffer, 0);
    Memory_Copy((void*)Object_GetField8(copiedBuffer), data, size);
    if (def == 0) {
        def = s_defaultModelDescriptor;
    }
    retainedBuffer = copiedBuffer;
    if (def == 0) {
        def = s_defaultModelDescriptor;
    }
    resource = (char*)((void* (*)(int))s_genericResourceAllocatorFunctions[0])(0x54);
    if (resource != 0) {
        ((gtl::IGenericModelingResource*)resource)->Init(retainedBuffer, (const ModelParams*)def);
    }
    *out = resource;
    localBufferReference = copiedBuffer;
    remainingBufferRefs = *(int*)localBufferReference - 1;
    *(int*)localBufferReference = remainingBufferRefs;
    if (remainingBufferRefs == 0 && localBufferReference != 0) {
        ((IGenericResourceFileV*)localBufferReference)->Release(1);
    }
    return 1;
}

extern "C" {} // extern "C"

// Copy an animation file and construct its wrapper; shared factory contract above.
int gtl::IGenericAnimationResource::CreateFromMemory(void* data, int size, void** out) {
    char* copiedBuffer;
    char* resource;
    char* localBufferReference;
    int remainingBufferRefs;

    Create__Q23gtl11IG3DXBufferFPvPPv(size, (void**)&copiedBuffer, 0);
    Memory_Copy((void*)Object_GetField8(copiedBuffer), data, size);
    resource = (char*)CreateAnimationResource(copiedBuffer);
    *out = resource;
    localBufferReference = copiedBuffer;
    remainingBufferRefs = *(int*)localBufferReference - 1;
    *(int*)localBufferReference = remainingBufferRefs;
    if (remainingBufferRefs == 0 && localBufferReference != 0) {
        ((IGenericResourceFileV*)localBufferReference)->Release(1);
    }
    return 1;
}

extern "C" {

// Required output self: >=0x44 writable bytes; mat: >=0x64 readable bytes,
// nonoverlapping. Copy the engine prefix, then multiply SPECULAR ALPHA (+2C)
// by a selected factor unless bit 6 bypasses it. Engine_SetMaterial converts
// this float4 to RGBA8 and dirties all lights; diffuse alpha is not modified.
// Input factors are dimensionless and unchecked (no clamp). Preserve the
// left-associated three-channel sum, division, optional complement, multiply.
// Returns 1; no retained pointers. Other description bytes stay copied verbatim.
int gtl_GenericMaterial_GetDescription(char* self, char* mat) {
    float factor;
    float specularAlpha;

    Memory_Copy(self, mat, 0x44);
    if (((MaterialDescriptionSource*)mat)->flags.bypassSpecularAlphaFactor == 0) {
        specularAlpha = ((MaterialEngineDescription*)self)->specular[3];
        if (((MaterialDescriptionSource*)mat)->flags.useScalarFactor) {
            factor = ((MaterialDescriptionSource*)mat)->scalarFactor;
        } else {
            factor = (((MaterialDescriptionSource*)mat)->factorChannels[0] +
                      ((MaterialDescriptionSource*)mat)->factorChannels[1] +
                      ((MaterialDescriptionSource*)mat)->factorChannels[2]) /
                     kMaterialFactorChannelCount;
        }
        if (((MaterialDescriptionSource*)mat)->flags.useFactorDirectly == 0) {
            factor = kFloat_1_GtlResource - factor;
        }
        ((MaterialEngineDescription*)self)->specular[3] = specularAlpha *= factor;
    }
    return 1;
}

// Required nonoverlapping self[0x68]/light[0x88]. Copy prefix, scale vector +4
// by intensity, publish range +4C, and attenuation {0,1/range,0} at +54/58/5C.
// range must be nonzero; zero divides without an error result. Type 2 also
// normalizes target-position into +40 and publishes cone {start,start+width}
// at +60/+64. Target must differ from position for a defined unit direction.
// Engine_SetLight (engine_render_cache_and_texture_load.cpp) caches this 0x68-byte description. The GX submission
// in gx_renderer_state_and_resources.cpp transforms world position/direction by the view matrix and uses
// cutoff=RadiansToDegrees*((start+end)*0.5). Thus authored cone values are radians;
// increasing width increases cutoff by half that increase. No unit conversion
// belongs here. Range is world distance; increasing it lowers linear attenuation
// 1/range, or increases the 1%-brightness distance in the alternate GX path.
// Returns 1, retains nothing, does not validate or clamp authored quantities.
int gtl_GenericLight_GetDescription(char* self, const char* light) {
    char tmp[16];
    float linearAttenuation;
    // Phase 1 writes zero attenuation endpoints; type 2 then reuses this
    // scratch for the cone start. No value is carried between those phases.
    float zeroThenConeStart;
    float range;

    Memory_Copy(self, light, 0x68);
    gtl_PixelCopy_Copy16Bytes(tmp, light + 4);
    Vec4_Scale(tmp, tmp, ((const LightDescriptionSource*)light)->intensity);
    gtl_PixelCopy_Copy16Bytes(self + 4, tmp);
    range = ((const LightDescriptionSource*)light)->range;
    linearAttenuation = kFloat_1_GtlResource / range;
    zeroThenConeStart = kFloat_0_GtlResource;
    *(float*)(self + 0x4c) = range;
    *(float*)(self + 0x54) = zeroThenConeStart;
    *(float*)(self + 0x5c) = zeroThenConeStart;
    *(float*)(self + 0x58) = linearAttenuation;
    if (*(int*)self == 2) {
        Vec3_Subtract(self + 0x40, ((const LightDescriptionSource*)light)->target,
                      ((const LightDescriptionSource*)light)->position);
        Vec3_Normalize(self + 0x40, self + 0x40);
        zeroThenConeStart = ((const LightDescriptionSource*)light)->coneStart;
        *(float*)(self + 0x60) = zeroThenConeStart;
        *(float*)(self + 0x64) = zeroThenConeStart + ((const LightDescriptionSource*)light)->coneWidth;
    }
    return 1;
}

// Copy four packed color bytes from required fog record +C to required output.
// Returns 1; no interpretation, retention or color-space conversion.
int gtl_GenericFog_GetColor(void* dst, char* src) {
    gtl_PixelCopy_Copy4Bytes_1(dst, src + 0xc);
    return 1;
}

// Copy required ambient record's first four color bytes to output; return 1.
int gtl_GenericAmbient_GetColor(void* dst, void* src) {
    gtl_PixelCopy_Copy4Bytes_1(dst, src);
    return 1;
}
}

// Required self/out: publish the same model pointer and add one wrapper ref.
// Caller must release it through the model interface; returns 1, makes no copy.
int gtl::IGenericModelingResource::AddRef(void** out) {
    *out = this;
    refCount = refCount + 1;
    return 1;
}

extern "C" {} // extern "C"

// Release the retained buffer, clear it, restore base vtable and optionally
// free the wrapper (flags>0) using the installed callback. Null self is allowed;
// returned address may already be freed. Buffer virtual Release(1) runs at zero.
void* gtl::IGenericResourceFile::Destroy(int flags) {
    if (this == 0) {
        goto end;
    }
    {
        char* p = buf;
        *(void**)((char*)this + 4) = __vt__Q23gtl20IGenericResourceFile;
        if (p != 0) {
            *(int*)p = *(int*)p - 1;
            if (*(int*)p == 0 && p != 0) {
                ((IGenericResourceFileV*)p)->Release(1);
            }
            buf = 0;
        }
    }
    if (flags > 0) {
        void** vt = s_genericResourceAllocatorFunctions;
        ((void (*)(void*))vt[1])(this);
    }
end:
    return this;
}

extern "C" {

#pragma push
#pragma opt_propagation off
// Install required allocate/free callback words self[0..1] globally, initialize
// renderer defaults and context. Call once before factories; callbacks must
// remain valid until every resource is released. Returns 1, no failure rollback.
int gtl_GenericResource_Initialize(const int* self) {
    char renderer[0x138];
    int ctx[3];
    void** vt;
    void* second;
    void* first;
    first = (void*)self[0];
    vt = s_genericResourceAllocatorFunctions;
    second = (void*)self[1];

    s_genericResourceAllocatorFunctions[0] = first;
    vt[1] = second;
    Renderer_InitializeDefaults(renderer);
    s_genericEnvironmentLightSlotCount = *(void**)(renderer + 0xa0);
    ctx[0] = 0;
    ctx[1] = 0;
    ctx[2] = 0;
    gtl_GenericRenderer_InitializeContext(ctx);
    return 1;
}
#pragma pop

} // extern "C"

// Query a chunk TAG (not an element index)
// from the required wrapper's retained buffer. Return count from resource API;
// the retained buffer and its file/chunk directory must be valid. No allocation or retention.
int gtl::IGenericResourceFile::GetChunkElementCount(int chunkTag) {
    int a = Object_GetFieldC(buf);
    int b = Object_GetField8(buf);
    return gtl_ResourceFile_GetChunkElementCount(b, a, chunkTag);
}

// Copy an environment file into a 0x0C-byte wrapper; shared factory contract above.
int gtl::IGenericEnvironmentResource::CreateFromMemory(void* data, int size, void** out) {
    char* copiedBuffer;
    char* retainedBuffer;
    char* resource;
    char* localBufferReference;
    int remainingBufferRefs;

    Create__Q23gtl11IG3DXBufferFPvPPv(size, (void**)&copiedBuffer, 0);
    Memory_Copy((void*)Object_GetField8(copiedBuffer), data, size);
    retainedBuffer = copiedBuffer;
    resource = (char*)((void* (*)(int))s_genericResourceAllocatorFunctions[0])(0xc);
    if (resource != 0) {
        *(int*)resource = 1;
        *(void**)(resource + 4) = __vt__Q23gtl20IGenericResourceFile;
        *(char**)(resource + 8) = retainedBuffer;
        ((int*)retainedBuffer)[0] = ((int*)retainedBuffer)[0] + 1;
        *(void**)(resource + 4) = __vt__Q23gtl27IGenericEnvironmentResource;
    }
    *out = resource;
    localBufferReference = copiedBuffer;
    remainingBufferRefs = *(int*)localBufferReference - 1;
    *(int*)localBufferReference = remainingBufferRefs;
    if (remainingBufferRefs == 0 && localBufferReference != 0) {
        ((IGenericResourceFileV*)localBufferReference)->Release(1);
    }
    return 1;
}
