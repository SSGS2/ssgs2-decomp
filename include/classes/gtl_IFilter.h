#ifndef CLASSES_GTL_IFILTER_H
#define CLASSES_GTL_IFILTER_H

#include "game/types.h"

namespace gtl {

// Base of the media filters: reference count, vtable pointer, filter type and state.
class IFilter {
public:
    // Setup and teardown.
    // Set the reference count to 1, the base vtable and the filter type; state starts at 0.
    void Init(unsigned int filterType);
    // Free this storage when deleting is positive; returns this.
    void* Destroy(int deleting);

    // Type and state.
    // Write the filter type through the output pointer.
    int GetType(unsigned int* value);
    // Write the filter state through the output pointer.
    int GetState(int* value);
    // Store a filter state without initiating a manager transition.
    void SetState(unsigned int value);

    unsigned int referenceCount; // 0x0
    void* vtable;                // 0x4
    unsigned int type;           // 0x8
    unsigned int state;          // 0xC
};

} // namespace gtl

#define GTL_IFILTER_OFF(f, o) typedef char gtl_IFilter_off_##f[((int)&((gtl::IFilter*)0)->f == (o)) ? 1 : -1]
GTL_IFILTER_OFF(referenceCount, 0x0);
GTL_IFILTER_OFF(vtable, 0x4);
GTL_IFILTER_OFF(type, 0x8);
GTL_IFILTER_OFF(state, 0xC);
#undef GTL_IFILTER_OFF

#endif // CLASSES_GTL_IFILTER_H
