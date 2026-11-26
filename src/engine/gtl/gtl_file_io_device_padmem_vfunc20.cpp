// PADMEM file device: a virtual slot that always returns zero and reads no device state.

#include "classes/GtlFileIoDevicePADMEM.h"

// Return the slot's fixed zero result without reading or modifying device state.
// No ownership transfer; do not infer "success" or "unsupported" from zero alone.
int gtl::CFileIoDevicePADMEM::vfunc_20() {
    return 0;
}
