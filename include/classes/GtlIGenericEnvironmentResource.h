// gtl::IGenericEnvironmentResource: an environment lighting wrapper, 0x0C bytes. It holds a reference count,
// the vtable word and the retained file buffer that holds the environment file copied by the factory.
#ifndef CLASSES_GTLIGENERICENVIRONMENTRESOURCE_H
#define CLASSES_GTLIGENERICENVIRONMENTRESOURCE_H

namespace gtl {

class IGenericEnvironmentResource {
public:
    // Factory: copies the environment file and returns the new wrapper through out.
    static int CreateFromMemory(void* data, int size, void** out);

    // Releases the retained buffer, and deletes the wrapper when flag > 0. Returns this.
    IGenericEnvironmentResource* Destroy(int flag);
    // Applies the environment to the current renderer; overrides may be null.
    void Apply(void* overrides);
    // Writes the number of light references to count and returns 1.
    int GetLightCount(int* count);

    int refCount;         // 0x00
    void* vtable;         // 0x04: __vt__Q23gtl27IGenericEnvironmentResource
    void* resourceBuffer; // 0x08: retained buffer (IG3DXBuffer)
};

typedef char IGenericEnvironmentResource_size[(sizeof(IGenericEnvironmentResource) == 0xC) ? 1 : -1];

} // namespace gtl

#endif // CLASSES_GTLIGENERICENVIRONMENTRESOURCE_H
