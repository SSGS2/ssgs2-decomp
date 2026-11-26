#ifndef CLASSES_GTL_CCALLBACKDISKFREESPACE_H
#define CLASSES_GTL_CCALLBACKDISKFREESPACE_H

#include "types.h"

namespace gtl {

// Completion callback for a disk-free-space query. It reports three fixed result words
// to the optional completion function, then disposes of itself.
class CCallBackDiskFreeSpace {
public:
    // Optionally free the object. flags > 0 frees it. Returns the original address,
    // which may already be freed.
    void* Destroy(int flags);

    // Report the fixed results, end the object's lifetime without freeing it, and pass
    // its storage to allocator callback 3.
    void* CompleteAndDispose();

    void* vtbl;                 // 0x00
    char pad_004[4];            // 0x04
    void* completionCallback;   // 0x08
    int completionUserDataWord; // 0x0C: passed as the completion function's last argument
};

} // namespace gtl

#define GTL_CCALLBACKDISKFREESPACE_OFF(f, o) \
    typedef char gtl_CCallBackDiskFreeSpace_off_##f[((int)&((gtl::CCallBackDiskFreeSpace*)0)->f == (o)) ? 1 : -1]
GTL_CCALLBACKDISKFREESPACE_OFF(vtbl, 0x0);
GTL_CCALLBACKDISKFREESPACE_OFF(completionCallback, 0x8);
GTL_CCALLBACKDISKFREESPACE_OFF(completionUserDataWord, 0xC);
#undef GTL_CCALLBACKDISKFREESPACE_OFF

#endif // CLASSES_GTL_CCALLBACKDISKFREESPACE_H
