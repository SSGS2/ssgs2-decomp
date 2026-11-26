// DVD device: CancelAsyncRequest, which submits an asynchronous cancel for a DVD handle with no
// completion callback. An invalid handle is rejected before the request is sent.

#include "classes/GtlFileIoDeviceDVD.h"

extern "C" {

void File_SetLastError(int error);
int DVDCancelAsync(const char* fileHandle, int callbackAddress);

enum {
    kInvalidDvdHandle = 0xFFFFFFFFu,
    kInvalidHandleError = 6,
    kNoCompletionCallback = 0
};

} // extern "C"

// Request asynchronous cancellation with no completion callback; return boolean submission status.
// fileHandle is an opaque borrowed DVD handle despite its char* type; it is not a path.
// Caller keeps handle storage valid for the pending SDK operation.
// Handle -1 reports error 6; both accepted and rejected SDK requests clear last-error.
int gtl::CFileIoDeviceDVD::CancelAsyncRequest(const char* fileHandle) {
    if ((unsigned int)fileHandle == kInvalidDvdHandle) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    if (!DVDCancelAsync(fileHandle, kNoCompletionCallback)) {
        File_SetLastError(0);
        return 0;
    }
    File_SetLastError(0);
    return 1;
}
