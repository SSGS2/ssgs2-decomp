#ifndef CLASSES_GTL_CCALLBACKREADFILE_H
#define CLASSES_GTL_CCALLBACKREADFILE_H

#include "types.h"

namespace gtl {

// Completion callback for a file read. It publishes the byte count in the borrowed
// file state, invokes the optional completion function, and owns its own storage.
class CCallBackReadFile {
public:
    // Destruct the object. flags > 0 also frees its storage. Returns the original address.
    void* Destroy(int flags);

    // Publish the read result, notify the completion function, then end the object's
    // lifetime and pass its storage to allocator callback 3.
    void* CompleteRead(int result);

    void* vtbl;               // 0x00
    void* fileState;          // 0x04: borrowed FMEM file state, valid through user completion
    void* completionCallback; // 0x08: completion(error, byteCount, fileState, userArgument)
};

} // namespace gtl

#define GTL_CCALLBACKREADFILE_OFF(f, o) \
    typedef char gtl_CCallBackReadFile_off_##f[((int)&((gtl::CCallBackReadFile*)0)->f == (o)) ? 1 : -1]
GTL_CCALLBACKREADFILE_OFF(vtbl, 0x0);
GTL_CCALLBACKREADFILE_OFF(fileState, 0x4);
GTL_CCALLBACKREADFILE_OFF(completionCallback, 0x8);
#undef GTL_CCALLBACKREADFILE_OFF

#endif // CLASSES_GTL_CCALLBACKREADFILE_H
