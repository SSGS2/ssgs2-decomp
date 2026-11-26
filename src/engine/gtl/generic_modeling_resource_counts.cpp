// Buffer and texture handle counts of a model resource. WPuppet's resource-buffer query
// (wpuppet_model_bone_animation.cpp) forwards its selector here. This reports handle counts, not bytes.
// Category 0 counts vertex buffers, 1 index buffers, 3 textures; category 2 and other selectors return zero.
#include "classes/GtlIGenericModelingResource.h"

// Element count of a live borrowed vector header. This is not capacity or byte size; no element storage is read.
struct GenericResourceVectorCount {
    unsigned int data;
    int count;
};
typedef char GenericResourceVectorCount_count_offset[(((int)&((GenericResourceVectorCount*)0)->count) == 0x4) ? 1 : -1];

int gtl::IGenericModelingResource::GetBufferCount(int category) {
    // Preserve the comparison and return order of the original; a switch changes the generated code.
    if (category == 2)
        goto unsupportedCategory;
    if (category >= 2)
        goto categoryAboveTwo;
    if (category == 0)
        goto categoryZero;
    if (category >= 0)
        goto categoryOne;
    goto unsupportedCategory;
categoryAboveTwo:
    if (category >= 4)
        goto unsupportedCategory;
    return (int)textures.size;
categoryZero:
    return (int)vertexBuffers.size;
categoryOne:
    return (int)indexBuffers.size;
unsupportedCategory:
    return 0;
}

extern "C" {

// Required live self for selectors 0, 1 and 3. Category 2, negative and >=4 selectors return zero.
int gtl_GenericResourceVector_GetSize(void* self) {
    return ((GenericResourceVectorCount*)self)->count;
}
}
