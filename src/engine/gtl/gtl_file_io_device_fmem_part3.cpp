// GTL FMEM device virtual function in slot 24: returns the fixed value 32 and does nothing else.

#include "classes/GtlFileIoDeviceFMEM.h"

// Return the FMEM device's fixed vtable-query value; the property is not identified.
// No arguments or ownership transfer; the raw result is always 32.
int gtl::CFileIoDeviceFMEM::vfunc_24() {
    enum {
        kFmemQueryResult = 0x20
    };
    return kFmemQueryResult;
}
