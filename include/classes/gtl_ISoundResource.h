// gtl::ISoundResource: a reference-counted sound resource. Init allocates its audio data from the resource
// argument; Destroy frees that data and optionally the object. The vtable comes from the original data, so no
// virtual functions are declared here.
#ifndef CLASSES_GTL_ISOUNDRESOURCE_H
#define CLASSES_GTL_ISOUNDRESOURCE_H

namespace gtl {

class ISoundResource {
public:
    // Take the resource argument, start the reference count at one, and allocate the audio data. Returns this.
    void* Init(void* resourceArg);
    // Free the audio data, clear the resource, and free this object when freeMemory is positive. Returns this.
    void* Destroy(int freeMemory);

    unsigned int referenceCount; // 0x0
    void* vtbl;                  // 0x4
    void* audioData;             // 0x8
    void* resource;              // 0xC
};

#define GTL_ISOUNDRESOURCE_OFF(f, o) \
    typedef char gtl_ISoundResource_off_##f[((int)&((gtl::ISoundResource*)0)->f == (o)) ? 1 : -1]
GTL_ISOUNDRESOURCE_OFF(referenceCount, 0x0);
GTL_ISOUNDRESOURCE_OFF(vtbl, 0x4);
GTL_ISOUNDRESOURCE_OFF(audioData, 0x8);
GTL_ISOUNDRESOURCE_OFF(resource, 0xC);
#undef GTL_ISOUNDRESOURCE_OFF

} // namespace gtl

#endif // CLASSES_GTL_ISOUNDRESOURCE_H
