#ifndef CLASSES_GTL_CCALLBACKFILELIST_H
#define CLASSES_GTL_CCALLBACKFILELIST_H

#include "types.h"

namespace gtl {
class CFindHandleFMEMi;
}

namespace gtl {

// Completion callback for a directory listing. The matched name is stored inline at +9
// or on the heap, selected by the top bit of stringFlags.
class CCallBackFileList {
public:
    // Release the heap-backed string, then optionally free the object.
    // deleting > 0 frees the object. Returns the original address, which may be dangling.
    void* Destroy(int deleting);

    // Test directory entries against the find pattern until one matches (or none is
    // left), then report the match to the completion callback and dispose of the object.
    // Consumes this object and context.
    void FindNextMatchAndComplete(void* operationResult, void* context);

    void* vtbl;                        // 0x00
    gtl::CFindHandleFMEMi* findHandle; // 0x04: directory search state
    unsigned int stringFlags;          // 0x08; top bit selects heap storage
    unsigned int stringLength;         // 0x0C
    void* heapText;                    // 0x10
    void* completionCallback;          // 0x14
    void* userData;                    // 0x18
};

} // namespace gtl

#define GTL_CCALLBACKFILELIST_OFF(f, o) \
    typedef char gtl_CCallBackFileList_off_##f[((int)&((gtl::CCallBackFileList*)0)->f == (o)) ? 1 : -1]
GTL_CCALLBACKFILELIST_OFF(vtbl, 0x0);
GTL_CCALLBACKFILELIST_OFF(findHandle, 0x4);
GTL_CCALLBACKFILELIST_OFF(stringFlags, 0x8);
GTL_CCALLBACKFILELIST_OFF(stringLength, 0xC);
GTL_CCALLBACKFILELIST_OFF(heapText, 0x10);
GTL_CCALLBACKFILELIST_OFF(completionCallback, 0x14);
GTL_CCALLBACKFILELIST_OFF(userData, 0x18);
#undef GTL_CCALLBACKFILELIST_OFF

#endif // CLASSES_GTL_CCALLBACKFILELIST_H
