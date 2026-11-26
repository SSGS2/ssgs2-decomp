// DVD device: CancelHandle, which cancels a DVD command synchronously and reports whether the
// cancel succeeded. An invalid handle is rejected before the DVD call.

#include "classes/GtlFileIoDeviceDVD.h"

extern "C" {

void File_SetLastError(int error);
int DVDCancel(unsigned int handle);

enum {
    kInvalidDvdHandle = 0xFFFFFFFFu,
    kInvalidHandleError = 6,
    kDvdCancelFailure = -1
};

} // extern "C"

// Cancel the borrowed DVD handle synchronously; return 0 for -1 handle or SDK result -1, else 1.
// Cancellation does not close/free the handle. Null is not rejected.
// Both SDK success and SDK failure clear last-error; return status is the only distinction.
int gtl::CFileIoDeviceDVD::CancelHandle(unsigned int handle) {
    if (handle == kInvalidDvdHandle) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }

    if (DVDCancel(handle) == kDvdCancelFailure) {
        File_SetLastError(0);
        return 0;
    }

    File_SetLastError(0);
    return 1;
}
