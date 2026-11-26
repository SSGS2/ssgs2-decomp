#ifndef CLASSES_GTL_CCALLBACKFINDFIRSTFILE_H
#define CLASSES_GTL_CCALLBACKFINDFIRSTFILE_H

#include "types.h"

namespace gtl {

// Completion callback for a directory search. The directory path is stored inline at
// +9 or on the heap, selected by the top bit of pathFlags.
class CCallBackFindFirstFile {
public:
    // Release the heap-backed path, then optionally free the object. deleting > 0 frees it.
    // Returns the original address, which may be dangling.
    void* Destroy(int deleting);

    // Deliver a failed directory query, or start the listing: allocate an aligned name
    // buffer, hand a copy of the path to a new file-list callback and queue the read.
    // Consumes operation and this object.
    void ContinueListing(void* error, void* operation, int resultBytes);

    void* vtbl;                                      // 0x00
    void* directory;                                 // 0x04
    unsigned int pathFlags;                          // 0x08; top bit selects heap storage
    int pathLength;                                  // 0x0C
    void* heapPath;                                  // 0x10
    void* (*completion)(void*, void*, void*, void*); // 0x14
    void* userArgument;                              // 0x18
};

} // namespace gtl

#define GTL_CCALLBACKFINDFIRSTFILE_OFF(f, o) \
    typedef char gtl_CCallBackFindFirstFile_off_##f[((int)&((gtl::CCallBackFindFirstFile*)0)->f == (o)) ? 1 : -1]
GTL_CCALLBACKFINDFIRSTFILE_OFF(vtbl, 0x0);
GTL_CCALLBACKFINDFIRSTFILE_OFF(directory, 0x4);
GTL_CCALLBACKFINDFIRSTFILE_OFF(pathFlags, 0x8);
GTL_CCALLBACKFINDFIRSTFILE_OFF(heapPath, 0x10);
GTL_CCALLBACKFINDFIRSTFILE_OFF(completion, 0x14);
GTL_CCALLBACKFINDFIRSTFILE_OFF(userArgument, 0x18);
#undef GTL_CCALLBACKFINDFIRSTFILE_OFF

#endif // CLASSES_GTL_CCALLBACKFINDFIRSTFILE_H
