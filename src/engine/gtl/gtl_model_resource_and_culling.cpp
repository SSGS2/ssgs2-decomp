#include <decomp/cw_bitfields.h>
// GTL model resource access and renderer/culling ABI forwarders.
// Mod lookup: GetResource selects cached graphics handles loaded by
// gtl_GenericModel_LoadGraphicsResources; gtl_generic_model_resource.cpp constructs the containing
// resource. IPetMesh_ScaleResource (pet_mesh_and_fixed_font.cpp) requests type 0 vertex and type 1
// index buffers. Type 3 selects textures: the constructor passes self+0x0C to
// the loader, which fills its first vector with Texture_Create2D handles.
// The Set* wrappers publish global culling state in gtl_model_culling_billboard_curve.cpp; they do not use
// a renderer receiver. Their typed argument is the original single r3 argument.
// Translation unit 0x803DB888-0x803DB94C (4 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct GtlRefHeader {
    // Native big-endian word: refCount occupies bits 31..16, not a full u32.
    CW_BITS2(
        unsigned int refCount : 16,
        unsigned int unk : 16)
};

// Read-only partial view of the 0x54-byte model wrapper. Arrays are owned by the
// wrapper; elements are refcounted. 0x18-byte spacing is preserved rather than
// assuming that the unexamined words are vector sizes/capacities.
struct ModelResourceArraysView {
    char reserved00[0x0C];
    GtlRefHeader** textures;
    char reserved10[0x14];
    GtlRefHeader** vertexBuffers;
    char reserved28[0x14];
    GtlRefHeader** indexBuffers;
};
typedef char ModelResourceArraysView_vertex_offset[
    (int)&((ModelResourceArraysView*)0)->vertexBuffers == 0x24 ? 1 : -1];
typedef char ModelResourceArraysView_index_offset[
    (int)&((ModelResourceArraysView*)0)->indexBuffers == 0x3C ? 1 : -1];

enum ModelResourceType { kVertexBufferResource = 0, kIndexBufferResource = 1, kTextureResource = 3 };

extern "C" {

void gtl_Culling_SetEnabled(int enabled);
void gtl_Culling_SetProjectionMatrix(const float* projectionMatrix);
void gtl_Culling_SetViewMatrix(const void* viewMatrix);

// Required live self/out. For types 0/1/3, index must be nonnegative and less
// than GetBufferCount(self,type) (generic_modeling_resource_counts.cpp): +28 vertex / +40 index / +10
// texture counts. Neither bound nor array pointer
// is checked here. On a nonnull entry, writes *out and adds one 16-bit reference;
// caller must release through the returned resource's matching interface.
// A null entry or unsupported type leaves *out UNCHANGED. Always returns 1:
// initialize *out before calling if absence must be distinguishable. No copy.
int gtl_IGenericModelingResource_GetResource(char* self, int type, int index, GtlRefHeader** out) {
    switch (type) {
    case kTextureResource: {
        GtlRefHeader* p = ((ModelResourceArraysView*)self)->textures[index];
        if (p != 0) {
            *out = p;
            p->refCount++;
        }
        break;
    }
    case kVertexBufferResource: {
        GtlRefHeader* p = ((ModelResourceArraysView*)self)->vertexBuffers[index];
        if (p != 0) {
            *out = p;
            p->refCount++;
        }
        break;
    }
    case kIndexBufferResource: {
        GtlRefHeader* p = ((ModelResourceArraysView*)self)->indexBuffers[index];
        if (p != 0) {
            *out = p;
            p->refCount++;
        }
        break;
    }
    }
    return 1;
}

// Set global culling context +0x180 to the caller's integer mode. This is the
// entire option word, not a bitwise update; accepted mode policy belongs to
// the culling consumers in gtl_model_culling_billboard_curve.cpp. No receiver, allocation or return status.
void gtl_GenericRenderer_SetCullingMode(int cullingMode) {
    gtl_Culling_SetEnabled(cullingMode);
}

// Forward the required borrowed 16-float projection matrix. Culling derives six planes
// (p[0],p[5],p[10],p[14]) and updates world planes using the stored view transpose;
// supply a valid perspective matrix. No pointer is retained. See gtl_model_culling_billboard_curve.cpp.
void gtl_GenericRenderer_SetProjectionMatrix(const float* projectionMatrix) {
    gtl_Culling_SetProjectionMatrix(projectionMatrix);
}

// Forward the required borrowed 16-float invertible view matrix (world -> view).
// Culling copies it, computes inverse/transpose, and refreshes world planes in
// the same global context. Projection and view updates therefore share state.
void gtl_GenericRenderer_SetViewMatrix(const void* viewMatrix) {
    gtl_Culling_SetViewMatrix(viewMatrix);
}

} // extern "C"
