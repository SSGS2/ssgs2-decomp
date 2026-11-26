#ifndef CLASSES_GTL_CCALLBACKRVLCHECKSAVEAREA_H
#define CLASSES_GTL_CCALLBACKRVLCHECKSAVEAREA_H

#include "types.h"

namespace gtl {

// Completion callback for a save-area capacity check. It holds the caller's callback
// and one opaque word, and owns its own storage.
class CCallBackRVLCheckSaveArea {
public:
    // Optionally free the object. deleteFlag > 0 frees it. Returns the original
    // address, which may already be freed.
    void* Destroy(int deleteFlag);

    // Translate the NAND result, notify the callback with the payload's first word, then
    // dispose of the payload, this object's teardown and its storage. Consumes both.
    void Complete(int nandResult, void* payload);

    void* vtbl; // 0x00
    void* cb;   // 0x04; called as (fileError, payload word, field8)
    int field8; // 0x08
};

} // namespace gtl

#define GTL_CCALLBACKRVLCHECKSAVEAREA_OFF(f, o) \
    typedef char gtl_CCallBackRVLCheckSaveArea_off_##f[((int)&((gtl::CCallBackRVLCheckSaveArea*)0)->f == (o)) ? 1 : -1]
GTL_CCALLBACKRVLCHECKSAVEAREA_OFF(vtbl, 0x0);
GTL_CCALLBACKRVLCHECKSAVEAREA_OFF(cb, 0x4);
GTL_CCALLBACKRVLCHECKSAVEAREA_OFF(field8, 0x8);
#undef GTL_CCALLBACKRVLCHECKSAVEAREA_OFF

#endif // CLASSES_GTL_CCALLBACKRVLCHECKSAVEAREA_H
