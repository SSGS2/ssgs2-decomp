#ifndef CLASSES_GTL_CCALLBACKWRITEFILE_H
#define CLASSES_GTL_CCALLBACKWRITEFILE_H

#include "types.h"

namespace gtl {

// Completion callback for a file write. It publishes the byte count in the borrowed
// file state, invokes the optional completion function, and owns its own storage.
class CCallBackWriteFile {
public:
    // Destruct the object. flags > 0 also frees its storage. Returns the original address.
    void* Destroy(int flags);

    // Publish the write result, notify the completion function, then release the
    // completion storage and this object through allocator callback 3.
    void* OnWriteComplete(int result, void* completionStorage);

    void* vtbl;               // 0x00
    void* fileState;          // 0x04: borrowed FMEM file state, must survive completion
    void* completionCallback; // 0x08
};

} // namespace gtl

#define GTL_CCALLBACKWRITEFILE_OFF(f, o) \
    typedef char gtl_CCallBackWriteFile_off_##f[((int)&((gtl::CCallBackWriteFile*)0)->f == (o)) ? 1 : -1]
GTL_CCALLBACKWRITEFILE_OFF(vtbl, 0x0);
GTL_CCALLBACKWRITEFILE_OFF(fileState, 0x4);
GTL_CCALLBACKWRITEFILE_OFF(completionCallback, 0x8);
#undef GTL_CCALLBACKWRITEFILE_OFF

#endif // CLASSES_GTL_CCALLBACKWRITEFILE_H
