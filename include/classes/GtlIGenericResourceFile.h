// gtl::IGenericResourceFile: a resource wrapper holding a reference count, the vtable word and the retained
// backing file buffer. gtl::IGenericAnimationResource adds only behaviour. The vtable word is written by the
// constructors and destructors from original data; no virtual functions are declared here.
#ifndef CLASSES_GTLIGENERICRESOURCEFILE_H
#define CLASSES_GTLIGENERICRESOURCEFILE_H

namespace gtl {

class IGenericResourceFile {
public:
    // Drop the retained buffer, restore the base vtable and free the wrapper when flags > 0. Returns this.
    void* Destroy(int flags);
    // Number of elements stored under a chunk tag in the retained file.
    int GetChunkElementCount(int chunkTag);
    // Borrowed pointer to an element of the chunk with the given tag, or null.
    void* GetChunkElement(int chunkTag, int elementIndex);

    int refCount;         // 0x0
    void* vtbl;           // 0x4
    void* resourceBuffer; // 0x8: retained backing buffer
};

class IGenericAnimationResource : public IGenericResourceFile {
public:
    // Copy an animation file and construct its wrapper; the new reference is returned through out.
    static int CreateFromMemory(void* data, int size, void** out);
    // Drop the retained buffer and free the wrapper when flags > 0. Returns this.
    void* Destroy(int flags);
    // Evaluate local scale/rotation/translation channels into poses at animationFrame.
    int EvaluateTransforms(void* poses, void* trackLimit, float animationFrame) const;
    // Evaluate texture transform matrices at time dt.
    void EvaluateMaterials(void* matrices, float dt) const;
};

typedef char IGenericResourceFile_size[(sizeof(IGenericResourceFile) == 0xC) ? 1 : -1];

} // namespace gtl

#endif // CLASSES_GTLIGENERICRESOURCEFILE_H
