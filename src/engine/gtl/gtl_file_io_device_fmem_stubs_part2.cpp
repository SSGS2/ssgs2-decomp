// FMEM file I/O device: fixed-zero stubs for virtual slots 13 and 14 of CFileIoDeviceFMEM.
// Each does no work and returns zero.

#include "classes/GtlFileIoDeviceFMEM.h"

// Return the FMEM device's fixed zero for slot 13; no state or last error changes.
// The operation is not identified.
int gtl::CFileIoDeviceFMEM::vfunc_13() {
    return 0;
}

// Return the FMEM device's fixed zero for slot 14; no state or last error changes.
// Keep this distinct vtable entry despite the identical stub body.
int gtl::CFileIoDeviceFMEM::vfunc_14() {
    return 0;
}
