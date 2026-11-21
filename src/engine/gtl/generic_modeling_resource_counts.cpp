// Generic modeling-resource buffer counts (0x803D97C0..0x803D980C).
// WPuppet's resource-buffer query (wpuppet_model_bone_animation.cpp) forwards its selector here. This
// reports buffer/texture handles, not allocated bytes or vertices/indices.
// No gameplay tuning: change resource construction, not this count accessor.

// Three vector-like tables in the borrowed resource, each {data,count,capacity}.
// Only counts are consumed here; pointer words retain their Wii 32-bit layout.
// Category 2 has no table in this accessor. Unsupported selectors return zero.
// Clone (gtl_generic_modeling_resource_clone.cpp) establishes the tables: 0=vertex buffers (+24), 1=index
// buffers (+3C), 3=textures (+0C); palettes (+18), declarations (+30) and the
// second vertex-buffer table (+48) are not selected here. CtorEmpty (gtl_model_resource_storage.cpp)
// zeroes each {data,count,capacity} triple. Category 2 is unsupported here.
struct ModelingResourceCounts {
    char reserved00[0x0C];
    unsigned int textureData; int textureCount; unsigned int textureCapacity;
    char reserved18[0x0C];
    unsigned int vertexBufferData; int vertexBufferCount; unsigned int vertexBufferCapacity;
    char reserved30[0x0C];
    unsigned int indexBufferData; int indexBufferCount; unsigned int indexBufferCapacity;
};
struct GenericResourceVectorCount { unsigned int data; int count; };
typedef char TextureCountOffset[((int)&((ModelingResourceCounts*)0)->textureCount == 0x10) ? 1 : -1];
typedef char VertexBufferCountOffset[((int)&((ModelingResourceCounts*)0)->vertexBufferCount == 0x28) ? 1 : -1];
typedef char IndexBufferCountOffset[((int)&((ModelingResourceCounts*)0)->indexBufferCount == 0x40) ? 1 : -1];

extern "C" {

// self must be a live resource for selectors 0,1,3. No ownership or state changes;
// category 2, negative and >=4 selectors return zero without dereferencing self.
int gtl_IGenericModelingResource_GetBufferCount(void* self, int category) {
    // Selection table: 0 -> +28, 1 -> +40, 2 -> zero, 3 -> +10;
    // everything else -> zero. Preserve the comparison/return block order:
    // both a switch and direct early returns lose this original-exact function.
    if (category == 2) goto unsupportedCategory;
    if (category >= 2) goto categoryAboveTwo;
    if (category == 0) goto categoryZero;
    if (category >= 0) goto categoryOne;
    goto unsupportedCategory;
categoryAboveTwo:
    if (category >= 4) goto unsupportedCategory;
    return ((ModelingResourceCounts*)self)->textureCount;
categoryZero:
    return ((ModelingResourceCounts*)self)->vertexBufferCount;
categoryOne:
    return ((ModelingResourceCounts*)self)->indexBufferCount;
unsupportedCategory:
    return 0;
}

// Return the element count of a live borrowed vector header. This is not capacity
// or byte size; no element storage is read and the vector is left unchanged.
int gtl_GenericResourceVector_GetSize(void* self) {
    return ((GenericResourceVectorCount*)self)->count;
}

} // extern "C"
