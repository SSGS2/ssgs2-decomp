// Generic renderer vertex-declaration query; no gameplay constants.
// GetIndexedAttributeCount tells input setup how many declaration entries remain
// after excluding blend weights and the type-5 blend-index form. It counts
// ATTRIBUTES, not vertices, indices in an index buffer, or byte stride.
// VertexDeclaration_GetElements (gtl_texture_vertex_formats.cpp) copies 12-byte native records INCLUDING
// the terminator; GetVertexBlendMode (gtl_generic_environment.cpp) consumes the same usage/type tags.
enum IndexedAttributeTags {
    kBlendWeightUsage = 1,
    kBlendIndicesUsage = 2,
    kExcludedBlendIndexStorageType = 5
};
// Type 5 is deliberately neutral: other declaration builders disagree about its
// extent (gtl_texture_stage_tev_setup.cpp). This query only recognizes the stored code, not a byte4 schema.

typedef unsigned char u8;
typedef unsigned int u32;

struct GenericAttrElement {
    u32 streamAndOffset; // +0: two u16 fields; not examined by this query
    u32 storageType;     // +4: only its low byte is tested below
    u8 method;           // +8: not examined
    u8 usage;            // +9: semantic usage, NOT enable flags
    u8 usageIndex;       // +A: semantic instance, not examined
    u8 reserved;         // +B
};

extern "C" {

void VertexDeclaration_GetElements(void* decl, void* elems, u32* count);

// decl is a required borrowed, terminated declaration of 0..64 real elements.
// The copier must return 1..65 records including the terminator. It treats count
// as an OUTPUT, not capacity; a longer declaration would overrun elems. Zero
// returned count underflows count-1. Neither condition is validated here.
// Return the number of real elements except usage 1 and usage 2/type-low-byte 5.
// No output arguments, resource retention, declaration mutation, or failure code.
unsigned int gtl_GenericRenderer_GetIndexedAttributeCount(void* decl) {
    u32 count = 0;
    GenericAttrElement elems[65];
    VertexDeclaration_GetElements(decl, elems, &count);
    unsigned int elementCount = count - 1; // drop the copied terminal record
    unsigned int result = elementCount;
    GenericAttrElement* element = elems;
    for (u32 i = 0; i < elementCount; i++) {
        u8 usage = element->usage;
        u8 storageTypeLowByte = element->storageType;
        if (usage == kBlendWeightUsage ||
            (usage == kBlendIndicesUsage && storageTypeLowByte == kExcludedBlendIndexStorageType)) {
            result--;
        }
        element++;
    }
    return result;
}

} // extern "C"
