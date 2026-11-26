// GTL FMEM device virtual function in slot 16. It returns zero and does nothing else.

#include "classes/GtlFileIoDeviceFMEM.h"

// Return constant zero from FMEM device virtual slot 16, without accessing the device.
// The operation is not identified; do not infer success or failure from the zero.
int gtl::CFileIoDeviceFMEM::vfunc_16() {
    return 0;
}
