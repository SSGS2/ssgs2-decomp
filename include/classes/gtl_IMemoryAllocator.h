#ifndef CLASSES_GTL_IMEMORYALLOCATOR_H
#define CLASSES_GTL_IMEMORYALLOCATOR_H

namespace gtl {

// Base interface of the engine's memory allocators. Its vtable is shared by
// the allocator classes; Destroy is the deleting destructor (slot 0).
class IMemoryAllocator {
public:
    void* Destroy(int deletingFlag);
};

} // namespace gtl

#endif // CLASSES_GTL_IMEMORYALLOCATOR_H
