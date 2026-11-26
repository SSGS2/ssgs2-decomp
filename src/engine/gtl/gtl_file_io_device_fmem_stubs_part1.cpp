// FMEM file I/O device: fixed-zero stubs for virtual slots 9, 10 and 11 of CFileIoDeviceFMEM.
// Each does no work and returns zero.

#include "classes/GtlFileIoDeviceFMEM.h"

// These are distinct FMEM device vtable entries. Their operations are not
// identified, so do not infer success or failure from the zero they return.
// No state, output, last-error value or resource ownership is changed.

// Return the fixed zero result for FMEM device virtual entry 9.
int gtl::CFileIoDeviceFMEM::vfunc_9() {
    return 0;
}

// Return the fixed zero result for FMEM device virtual entry 10.
int gtl::CFileIoDeviceFMEM::vfunc_10() {
    return 0;
}

// Return the fixed zero result for FMEM device virtual entry 11.
int gtl::CFileIoDeviceFMEM::vfunc_11() {
    return 0;
}
