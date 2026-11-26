// gtl::CGSPSoundStreamResource: a sound stream resource. It holds a pointer to its parsed data block, which lists
// the source slots, the decoded property slots and the 32-byte chunk descriptors. Methods are in
// src/engine/gtl/gtl_sound_buffer_stream_resources.cpp. No virtual functions are declared here; the vtable comes
// from the original data.
#ifndef CLASSES_GTL_CGSPSOUNDSTREAMRESOURCE_H
#define CLASSES_GTL_CGSPSOUNDSTREAMRESOURCE_H

struct Data_8043E118;

namespace gtl {

class CGSPSoundStreamResource {
public:
    // Release the sources, properties, chunks and the source owner. Frees this object when deleting is positive.
    // Returns this.
    void* Destroy(int deleting);
    // Return the number of stream chunks, as a pointer-width value.
    void* GetChunkCount();
    // Return the name stored in the chunk at index, or null when the index is out of range.
    void* GetChunkName(unsigned int index);
    // Copy the base parameter block of the property selected by index to dst. Returns 1 when it was copied.
    int CopyBaseParams(void* index, void* dst);
    // Return the source at index, or null when the index is out of range.
    void* GetSourceAt(unsigned int index);
    // Return the property at index, or null when the index is out of range.
    void* GetProperty(unsigned int index);
    // Set the source and apply the selected property to the stream dst. Returns 0 when dst is null, else 1.
    int ApplyToStream(void* arg, void* dst);

    char pad_00[4];      // 0x0, not used by these methods
    void* vtbl;          // 0x4
    Data_8043E118* data; // 0x8
};

} // namespace gtl

#define GTL_CGSPSOUNDSTREAMRESOURCE_OFF(f, o) \
    typedef char gtl_CGSPSoundStreamResource_off_##f[((int)&((gtl::CGSPSoundStreamResource*)0)->f == (o)) ? 1 : -1]
GTL_CGSPSOUNDSTREAMRESOURCE_OFF(vtbl, 0x4);
GTL_CGSPSOUNDSTREAMRESOURCE_OFF(data, 0x8);

#endif // CLASSES_GTL_CGSPSOUNDSTREAMRESOURCE_H
