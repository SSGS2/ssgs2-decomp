// GTL environment application: tagged resource -> lights/fog/ambient and
// view/projection state. Also queries vertex blending and frees small buffers.
// Edit index: resource/override selection -> GenericEnvironment_Apply;
// camera FOV/aspect -> final camera branch (projection math: gtl_vector_matrix_math.cpp);
// skinning declaration -> GetVertexBlendMode (element schema below).
// No framebuffer size or presentation/frame-rate policy here. Render state is
// global and must be applied on the renderer's thread; calls do not restore it.
// Shared literal pools: s_fogRenderStateTemplate = five state/value pairs (fog template),
// s_ambientColourRenderState = the {8, 0} (ambient colour render state, default) pair; s_fovAngleScale / s_fovTangentNumerator = float32 2 / 1 of fovY = 2*atan(1/(2*raw30)).

#include "sdk_protos.h"

typedef unsigned int u32;

struct GenericEnvOverride {
    void* lightDescriptions;  // +00 borrowed light-record base, stride 0x88
    void* viewMatrix;         // +04 optional view matrix
    void* projectionMatrix;   // +08 optional projection matrix
    void* fogDescription;     // +0C optional fog record; ALSO gates ambient override
    void* ambientDescription; // +10 ambient override, selected only when fogDescription is nonnull
};

struct GenericEnvEntry {
    int tag;
    u32 elementCount;       // count (lights) / nonzero admission (camera)
    u32 unk8;               // unused here
    u32 resourceByteOffset; // byte offset relative to Object_GetField8(obj)'s resource base
};

struct GenericEnvLightRef {
    int unk0;
    int unk4;
    int lightIndex; // index into selected light base, NOT a byte offset
};

struct GenericEnvCamera {
    float eye[3];
    float target[3];
    float up[3];
    char unk24[4];
    float viewHeightHalf;             // coefficient used as aspect denominator after *2
    float unused2C;                   // unused by this projection builder
    float aspectNumeratorCoefficient; // coefficient used as aspect numerator AND cotangent input
    float nearClip;                   // near clip distance, view units
    float farClip;                    // far clip distance, same units
};

// One GX render-state assignment. The fog template is five pairs: (1,1) fog on, (21,2) linear fog, then the values of
// state 23 (start), 24 (end) and 22 (colour), which gtl_GenericEnvironment_Apply overwrites.
struct RenderStatePair {
    u32 stateId;
    u32 value;
};

struct GenericEnvStateTable {
    RenderStatePair pairs[5];
};

struct DtorObj {
    void* ownedPtr;
    u32 count;
};

struct DtorDispatch {
    int unk0;
    void (*releaseCallback)(void*);
};

// 12-byte native declaration element (VertexElement_Copy, gx_renderer_state_and_efb_copy.cpp):
// +0 u16 stream, +2 u16 byte offset; +4 u32 type; +8 method; +9 usage;
// +A usage index; +B trailing byte. The packed first word is never decoded here.
struct VtxDeclElem {
    u32 packedStreamAndByteOffset;
    int storageType; // storage type: 0/1 distinguish one/two weight components here
    u8 method;
    u8 usage; // usage: 1 blend weights, 2 blend indices
    char reservedA[2];
};
enum EnvironmentChunkTag {
    kLightReferencesTag = 0x67696C,
    kFogDescriptionTag = 0x64676F66,
    kAmbientDescriptionTag = 0x64626D61,
    kLightDescriptionsTag = 0x646C,
    kCameraDescriptionTag = 0x646D6163
};
// (little-endian character packing: "lig", "fogd", "ambd", "ld", "camd").
struct EnvironmentDirectoryView {
    char header[0x14];
    u32 entryCount;
    char reserved18[8];
    GenericEnvEntry entries[1];
};
enum VertexBlendDomain {
    kBlendWeightsUsage = 1,
    kBlendIndicesUsage = 2,
    kSpecialBlendIndicesType = 0xFE,
    kSpecialVertexBlendMode = 0x100
};

extern "C" {

GenericEnvStateTable s_fogRenderStateTemplate;
RenderStatePair s_ambientColourRenderState;
float s_fovAngleScale;
float s_fovTangentNumerator;
DtorDispatch s_genericResourceAllocatorFunctions;
void __dl__FPv(void*);

void* Object_GetField8(void* o);
void Render_SetSystemOption(int mode, int value);
void Engine_SetLightEnabled(int index, int enabled);
void Engine_SetLight(int index, void* light);
int gtl_GenericLight_GetDescription(void* out, void* light);
int gtl_GenericFog_GetColor(void* out, void* fog);
int gtl_GenericAmbient_GetColor(void* out, void* ambient);
void Engine_SetRenderStateList(void* states, int count);
void SetTransform(int state, void* matrix);
void gtl_GenericRenderer_SetViewMatrix(void* matrix);
void gtl_GenericRenderer_SetProjectionMatrix(void* matrix);
void MTXLookAt(void* matrix, const void* eye, const void* target, const void* up);
void MtxPerspectiveFovRH(void* matrix, float fovY, float aspect, float zn, float zf);
void VertexDeclaration_GetElements(void* decl, void* elems, u32* count);

// obj/resource required; overrides optional; maxLights points to an input count
// of renderer light slots to touch (not an output). All resources are borrowed,
// native-endian and already loaded: directory at +0x20 has count@+0x14 entries
// of 16 bytes. Offsets/indices/lengths are trusted, with no malformed-file checks.
// Last matching tag wins; unrecognized tags are skipped. References supply one
// index@+8 per 12-byte light-ref record into 0x88-byte light descriptions.
// Enable first min(maxLights,lightCount) slots; disable remaining requested slots;
// slots >=maxLights untouched. Fog/ambient/view/projection absent -> leave old
// state. Overrides for view/projection take priority; projection override exits
// after publication. Missing chunks are not a failure result (function is void).
// Legacy ambient override gate deliberately tests FOG pointer +0xC, not +0x10.
// If fog override exists but ambient override is null, ambient is skipped.
void gtl_GenericEnvironment_Apply(void* obj, GenericEnvOverride* overrides, u32* maxLights) {
    char* base = (char*)Object_GetField8(obj);
    GenericEnvLightRef* lightRef = 0;
    char* lightBase = 0;
    GenericEnvCamera* camera = 0;
    u32* fog = 0;
    void* ambient = 0;
    u32 lightCount = 0;
    GenericEnvEntry* entry;
    u32 n;
    u32 i;
    u32 j;
    u32 fogColor;
    u32 ambColor;
    RenderStatePair ambStates;
    GenericEnvStateTable fogStates;
    float viewMtx[4][4];
    float projMtx[4][4];
    char lightDesc[0x68];

    Render_SetSystemOption(2, 1);
    n = ((EnvironmentDirectoryView*)base)->entryCount;
    entry = ((EnvironmentDirectoryView*)base)->entries;
    for (j = 0; j < n; j++) {
        switch (entry->tag) {
        case kLightReferencesTag:
            lightCount = entry->elementCount;
            lightRef = (GenericEnvLightRef*)(base + entry->resourceByteOffset);
            break;
        case kFogDescriptionTag:
            if (overrides != 0 && overrides->fogDescription != 0) {
                fog = (u32*)overrides->fogDescription;
            } else {
                fog = (u32*)(base + entry->resourceByteOffset);
            }
            break;
        case kAmbientDescriptionTag:
            if (overrides != 0 && overrides->fogDescription != 0) {
                ambient = overrides->ambientDescription;
            } else {
                ambient = base + entry->resourceByteOffset;
            }
            break;
        case kLightDescriptionsTag:
            if (overrides != 0 && overrides->lightDescriptions != 0) {
                lightBase = (char*)overrides->lightDescriptions;
            } else {
                lightBase = base + entry->resourceByteOffset;
            }
            break;
        case kCameraDescriptionTag:
            if (entry->elementCount != 0) {
                camera = (GenericEnvCamera*)(base + entry->resourceByteOffset);
            }
            break;
        }
        entry++;
    }
    for (i = 0; i < *maxLights; i++) {
        if (i >= lightCount) {
            Engine_SetLightEnabled(i, 0);
        } else {
            Engine_SetLightEnabled(i, 1);
            gtl_GenericLight_GetDescription(lightDesc, lightBase + lightRef->lightIndex * 0x88);
            Engine_SetLight(i, lightDesc);
        }
        lightRef++;
    }
    if (fog != 0) {
        gtl_GenericFog_GetColor(&fogColor, fog);
        fogStates = s_fogRenderStateTemplate;
        fogStates.pairs[2].value = fog[0];
        fogStates.pairs[3].value = fog[1];
        fogStates.pairs[4].value = fogColor;
        Engine_SetRenderStateList(&fogStates, 5);
    }
    if (ambient != 0) {
        gtl_GenericAmbient_GetColor(&ambColor, ambient);
        ambStates = s_ambientColourRenderState;
        ambStates.value = ambColor;
        Engine_SetRenderStateList(&ambStates, 1);
    }
    if (overrides != 0 && overrides->viewMatrix != 0) {
        SetTransform(0, overrides->viewMatrix);
        gtl_GenericRenderer_SetViewMatrix(overrides->viewMatrix);
    } else if (camera != 0) {
        MTXLookAt(viewMtx, camera->eye, camera->target, camera->up);
        SetTransform(0, viewMtx);
        gtl_GenericRenderer_SetViewMatrix(viewMtx);
    }
    if (overrides != 0 && overrides->projectionMatrix != 0) {
        SetTransform(1, overrides->projectionMatrix);
        gtl_GenericRenderer_SetProjectionMatrix(overrides->projectionMatrix);
        return;
    }
    if (camera != 0) {
        // RH projection (-Z forward), row-vector float[16] convention from
        // MtxPerspectiveFovRH. eye/target share world units; up is a direction.
        // EXACT resource conversion: fovy=2*atan(1/(2*raw30)), aspect=raw30/raw28.
        // fovy passed to the builder is vertical radians; do not silently swap
        // the two coefficients to make their apparent width/height roles agree.
        // Authored raw28/raw30 convention is unresolved; not documented angles.
        // Requires nonzero coefficients, 0<near<far and initialized projection
        // depth convention (gtl_vector_matrix_math.cpp). No clamping/error path. Preserve float
        // rounding around atan(double), then divide the two rounded products.
        float doubledAspectNumerator = s_fovAngleScale * camera->aspectNumeratorCoefficient;
        float doubledAspectDenominator = s_fovAngleScale * camera->viewHeightHalf;
        float fovY = s_fovAngleScale * (float)atan(s_fovTangentNumerator / doubledAspectNumerator);
        MtxPerspectiveFovRH(projMtx, fovY, doubledAspectNumerator / doubledAspectDenominator, camera->nearClip,
                            camera->farClip);
        SetTransform(1, projMtx);
        gtl_GenericRenderer_SetProjectionMatrix(projMtx);
    }
}

#pragma opt_propagation off
// Release the buffer at +0, optionally free wrapper when flags>0; accepts null.
// The +4 count reset uses two loads/subtraction intentionally under propagation
// suppression. Return the old address (possibly freed); do not dereference it.
DtorObj* dtor_803DBC90(DtorObj* p, int flags) {
    if (p != 0) {
        void* inner = p->ownedPtr;
        if (inner != 0) {
            u32 a = p->count;
            u32 b = a;
            p->count = a - b;
            __dl__FPv(inner);
        }
        if (flags > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}
#pragma opt_propagation reset

// Reset +4, free buffer then optionally wrapper; null is accepted, return may dangle.
DtorObj* dtor_803DBD7C(DtorObj* p, int flags) {
    if (p != 0) {
        p->count = 0;
        if (p != 0 && p->ownedPtr != 0) {
            p->count = 0;
            __dl__FPv(p->ownedPtr);
        }
        if (flags > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}

#pragma opt_propagation off
// As dtor_803DBC90, using the allocator's release callback at s_genericResourceAllocatorFunctions+4.
DtorObj* dtor_803DBD00(DtorObj* p, int flags) {
    if (p != 0) {
        void* inner = p->ownedPtr;
        if (inner != 0) {
            u32 a = p->count;
            u32 b = a;
            p->count = a - b;
            s_genericResourceAllocatorFunctions.releaseCallback(inner);
        }
        if (flags > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}

// As dtor_803DBD7C, with callback release instead of operator delete for the buffer.
DtorObj* dtor_803DBDF0(DtorObj* p, int flags) {
    if (p != 0) {
        p->count = 0;
        if (p != 0 && p->ownedPtr != 0) {
            p->count = 0;
            s_genericResourceAllocatorFunctions.releaseCallback(p->ownedPtr);
        }
        if (flags > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}
#pragma opt_propagation reset

// Query a required borrowed declaration; writes no renderer state. Capacity is
// 65 elements INCLUDING terminal entry; GetElements must return count in [1,65].
// count=0 would underflow count-1, so invalid declarations are not accepted.
// First usage2/typeFE -> special mode0x100. Otherwise first usage1/type0 -> 1,
// type1 -> 2; no supported weight element -> 0. Returned values are renderer
// blend modes, not byte strides or character indices. Final conversion retains
// legacy cases for internal modes1/4 even though this scan never produces them.
int gtl_GenericRenderer_GetVertexBlendMode(void* decl) {
    VtxDeclElem elems[0x41];
    u32 count;
    u32 i;
    VtxDeclElem* e;
    int mode = 0;
    int ret;

    count = 0x41;
    VertexDeclaration_GetElements(decl, elems, &count);
    if (elems[0].usage == kBlendIndicesUsage && (u32)elems[0].storageType == kSpecialBlendIndicesType) {
        return kSpecialVertexBlendMode;
    }
    e = elems;
    for (i = 0; i < count - 1; i++) {
        if (e->usage == kBlendWeightsUsage) {
            switch (e->storageType) {
            case 0:
                mode = 2;
                break;
            case 1:
                mode = 3;
                break;
            }
            break;
        }
        e++;
    }
    ret = 1;
    switch (mode) {
    case 0:
        ret = 0;
        break;
    case 1:
        ret = 0x100;
        break;
    case 2:
        ret = 1;
        break;
    case 3:
        ret = 2;
        break;
    case 4:
        ret = 3;
        break;
    }
    return ret;
}

} // extern "C"
