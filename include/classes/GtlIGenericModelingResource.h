// gtl::IGenericModelingResource: a model resource wrapper, 0x54 bytes. It holds a reference count, a vtable
// word, the retained backing resource file and six handle vectors. Each vector is {data, size, capacity}; size
// and capacity count slots, not bytes. The vtable word is written by the constructors from original data.
#ifndef CLASSES_GTLIGENERICMODELINGRESOURCE_H
#define CLASSES_GTLIGENERICMODELINGRESOURCE_H

#include <decomp/cw_bitfields.h>

struct ModelParams;

// Reference header of a handle. The reference count is the high 16 bits of the first word.
struct GtlRefHeader {
    CW_BITS2(unsigned int refCount : 16, unsigned int unk : 16)
};

// One handle vector inside the model wrapper.
struct GenericResourceVector {
    void* data;            // 0x0: slot array
    unsigned int size;     // 0x4: live slot count
    unsigned int capacity; // 0x8: allocated slot count
};

namespace gtl {

class IGenericModelingResource {
public:
    // Factory: copies the input, builds a wrapper and returns a new reference through out.
    static int CreateFromMemory(void* data, int size, void** out, void* def);

    // Construction and destruction.
    IGenericModelingResource* Init(void* archive, const ModelParams* params);
    void InitEmpty();
    void* Destroy(int flags);
    int Clone(unsigned int flags, IGenericModelingResource** out);

    // Reference count.
    int AddRef(void** out);

    // Drawing and animation.
    void QueueDraw(void* b, void* c);
    void EvaluateTransforms(void* out, void* unused, void* root, void* poses, unsigned int poseCount,
                            unsigned int* flagsIn) const;
    int EvaluateTransformAnimation(void* transformsOut, void* trackCountWord) const;
    void EvaluateMaterialAnimation(void* matrices) const;
    int GetDescription(unsigned int* out);
    int GetBufferCount(int category);

    // Handle access.
    int GetResource(int type, int index, GtlRefHeader** out);
    int SetResource(int type, int index, GtlRefHeader* res);

    int refCount;                         // 0x00
    void* vtable;                         // 0x04: __vt__Q23gtl24IGenericModelingResource
    void* backingResource;                // 0x08: retained backing resource file
    GenericResourceVector textures;       // 0x0C
    GenericResourceVector palettes;       // 0x18
    GenericResourceVector vertexBuffers;  // 0x24
    GenericResourceVector vertexDecls;    // 0x30
    GenericResourceVector indexBuffers;   // 0x3C
    GenericResourceVector vertexBuffers2; // 0x48
};

} // namespace gtl

#define GTL_IGENERICMODELINGRESOURCE_OFF(f, o) \
    typedef char gtl_IGenericModelingResource_off_##f[((int)&((gtl::IGenericModelingResource*)0)->f == (o)) ? 1 : -1]
GTL_IGENERICMODELINGRESOURCE_OFF(refCount, 0x0);
GTL_IGENERICMODELINGRESOURCE_OFF(vtable, 0x4);
GTL_IGENERICMODELINGRESOURCE_OFF(backingResource, 0x8);
GTL_IGENERICMODELINGRESOURCE_OFF(textures, 0xC);
GTL_IGENERICMODELINGRESOURCE_OFF(palettes, 0x18);
GTL_IGENERICMODELINGRESOURCE_OFF(vertexBuffers, 0x24);
GTL_IGENERICMODELINGRESOURCE_OFF(vertexDecls, 0x30);
GTL_IGENERICMODELINGRESOURCE_OFF(indexBuffers, 0x3C);
GTL_IGENERICMODELINGRESOURCE_OFF(vertexBuffers2, 0x48);
#undef GTL_IGENERICMODELINGRESOURCE_OFF
typedef char gtl_IGenericModelingResource_size[(sizeof(gtl::IGenericModelingResource) == 0x54) ? 1 : -1];

#endif // CLASSES_GTLIGENERICMODELINGRESOURCE_H
