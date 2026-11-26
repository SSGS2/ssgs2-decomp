#ifndef CLASSES_GTL_CCALLBACKMOVEFILEWITHPROGRESS_H
#define CLASSES_GTL_CCALLBACKMOVEFILEWITHPROGRESS_H

#include "types.h"

namespace gtl {

// Completion callback for a file move that reports progress. It holds the client's
// callback and owns its own storage.
class CCallBackMoveFileWithProgress {
public:
    // Optionally free the object. deleting > 0 frees it. Returns the original address,
    // which may already be freed.
    void* Destroy(int deleting);

    // Translate the NAND status, notify the client with four zero progress pairs, then
    // tear the object down without freeing it and pass its storage to allocator callback 3.
    void* OnNandResult(int status);

    void* vtbl;     // 0x00
    void* callback; // 0x04; client callback
};

} // namespace gtl

#define GTL_CCALLBACKMOVEFILEWITHPROGRESS_OFF(f, o) \
    typedef char                                    \
        gtl_CCallBackMoveFileWithProgress_off_##f[((int)&((gtl::CCallBackMoveFileWithProgress*)0)->f == (o)) ? 1 : -1]
GTL_CCALLBACKMOVEFILEWITHPROGRESS_OFF(vtbl, 0x0);
GTL_CCALLBACKMOVEFILEWITHPROGRESS_OFF(callback, 0x4);
#undef GTL_CCALLBACKMOVEFILEWITHPROGRESS_OFF

#endif // CLASSES_GTL_CCALLBACKMOVEFILEWITHPROGRESS_H
