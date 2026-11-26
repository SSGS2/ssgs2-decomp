// FMEM file I/O device: a placeholder stub for virtual slot 29 of CFileIoDeviceFMEM.
// It does no work and returns zero.

#include "classes/GtlFileIoDeviceFMEM.h"

// Return the FMEM device's constant-zero response for virtual slot 29.
// No side effects. The vtable reference establishes the slot, but its operation and
// the zero is not read as success, failure or unsupported.
int gtl::CFileIoDeviceFMEM::vfunc_29() {
    return 0;
}
