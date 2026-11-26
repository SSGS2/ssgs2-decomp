// HIO file I/O device: the CancelHandle stub for virtual slot 9 of CFileIoDeviceHIO.
// It does no work and returns zero.

#include "types.h"
#include "sdk_protos.h"
#include "classes/GtlFileIoDeviceHIO.h"

extern "C" {} // extern "C"

// Return the HIO device's constant-zero fallback for virtual slot 9.
// The device is not used; no handle or ownership changes occur. The DVD implementation
// uses this slot for cancellation; this stub does no work and leaves errors alone.
int gtl::CFileIoDeviceHIO::CancelHandle() {
    return 0;
}
