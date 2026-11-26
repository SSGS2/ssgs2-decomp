// Model resource handle access, and renderer culling forwarders. GetResource selects graphics handles loaded by
// gtl_GenericModel_LoadGraphicsResources. Type 0 is the vertex buffers and type 1 the index buffers (requested by
// IPetMesh_ScaleResource in pet_mesh_and_fixed_font.cpp); type 3 selects textures. The Set* wrappers publish global
// culling state in gtl_model_culling_billboard_curve.cpp; they take no receiver.
#include "classes/GtlIGenericModelingResource.h"

enum ModelResourceType {
    kVertexBufferResource = 0,
    kIndexBufferResource = 1,
    kTextureResource = 3
};

extern "C" {

void gtl_Culling_SetEnabled(int enabled);
void gtl_Culling_SetProjectionMatrix(const float* projectionMatrix);
void gtl_Culling_SetViewMatrix(const void* viewMatrix);
}

// Required live object. For types 0, 1 and 3, index must be nonnegative and less than GetBufferCount(type)
// (generic_modeling_resource_counts.cpp). Neither bound nor array pointer is checked here. On a nonnull entry,
// writes *out and adds one 16-bit reference; the caller must release through the returned resource's matching
// interface. A null entry or unsupported type leaves *out unchanged. Always returns 1, so initialize *out before
// calling if absence must be distinguishable. No copy is made.
int gtl::IGenericModelingResource::GetResource(int type, int index, GtlRefHeader** out) {
    switch (type) {
    case kTextureResource: {
        GtlRefHeader* p = ((GtlRefHeader**)textures.data)[index];
        if (p != 0) {
            *out = p;
            p->refCount++;
        }
        break;
    }
    case kVertexBufferResource: {
        GtlRefHeader* p = ((GtlRefHeader**)vertexBuffers.data)[index];
        if (p != 0) {
            *out = p;
            p->refCount++;
        }
        break;
    }
    case kIndexBufferResource: {
        GtlRefHeader* p = ((GtlRefHeader**)indexBuffers.data)[index];
        if (p != 0) {
            *out = p;
            p->refCount++;
        }
        break;
    }
    }
    return 1;
}

extern "C" {

// Set global culling context +0x180 to the caller's integer mode. This is the entire option word, not a bitwise
// update; accepted mode policy belongs to the culling consumers in gtl_model_culling_billboard_curve.cpp.
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
