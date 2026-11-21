#include "types.h"
#include "sdk_protos.h"

extern "C" {

// Return the HIO device's constant-zero fallback for virtual slot 9.
// self is ignored; no handle or ownership changes occur. The DVD implementation
// uses this slot for cancellation; this stub does no work and leaves errors alone.
int gtl_CFileIoDeviceHIO_CancelHandle(void *self) {
    return 0;
}

} // extern "C"
