// gtl::CGSPSoundBufferResource: a retained source archive, its parsed 32-byte chunk descriptors and the
// owned source and property reference lists. Accessors return borrowed objects; Destroy releases the lists.
// Field names of SoundResourceListsView are inferred from how the functions use them.
#ifndef CLASSES_SOUNDRESOURCE8043_H
#define CLASSES_SOUNDRESOURCE8043_H

// Data block at +8 of the resource.
struct SoundResourceListsView {
    void* retainedSourceOwner;      // 0x00: retained source archive
    unsigned int* chunkDescriptors; // 0x04: owned 32-byte descriptors
    unsigned int chunkCount;        // 0x08
    unsigned int chunkCapacity;     // 0x0C
    void* sourceStorage;            // 0x10: retained source storage
    unsigned int* sourceSlots;      // 0x14: owned source references
    unsigned int sourceCount;       // 0x18
    unsigned int sourceCapacity;    // 0x1C
    unsigned int* propertySlots;    // 0x20: owned decoded-property references
    unsigned int propertyCount;     // 0x24
};

namespace gtl {

class CGSPSoundBufferResource {
public:
    // Accessors. Each returns a borrowed, resource-owned object or a count.
    unsigned int GetChunkCount();
    unsigned int* GetSourceAt(unsigned int index);
    void* GetPropertySlot(unsigned int index);
    void* GetChunkName(unsigned int index);

    // Attach the resource and its selected property to a sound buffer.
    int ApplyToBuffer(void* arg, void* dst);
    int CopyBaseParams(void* forwardedArg, void* dst);

    // Release the references and lists; deletes the object when deleting is positive.
    void* Destroy(int deleting);

    char pad_00[4];
    void* vtbl;                   // 0x04
    SoundResourceListsView* data; // 0x08
};

}

#define OFF(f, o) \
    typedef char CGSPSoundBufferResource_off_##f[((int)&((gtl::CGSPSoundBufferResource*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x4);
OFF(data, 0x8);
#undef OFF

#endif
