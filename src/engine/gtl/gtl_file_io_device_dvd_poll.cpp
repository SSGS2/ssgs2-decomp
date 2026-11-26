// DVD device: PollCommandCompletion, which checks the status of a DVD command block and reports
// whether it has finished. Known failure states set the last error for the caller.

#include "classes/GtlFileIoDeviceDVD.h"

// SDK/file support routines used by this device method.
extern "C" void File_SetLastError(int error);
extern "C" int DVDGetCommandBlockStatus(unsigned int handle);

extern "C" {

enum DvdCommandStatus {
    kDvdFatal = -1,
    kDvdEnd = 0,
    kDvdBusy = 1,
    kDvdWaiting = 2,
    kDvdCoverOpen = 4,
    kDvdNoDisc = 5,
    kDvdCoverClosed = 6,
    kDvdWrongDisc = 7,
    kDvdMotorStopped = 9,
    kDvdCanceled = 10,
    kDvdRetry = 11
};
enum DvdPollError {
    kNoError = 0,
    kInvalidHandleError = 6,
    kCoverOpenError = 0x14,
    kDiscError = 0x15,
    kCanceledError = 0x4c7,
    kRetryError = 0x4d5,
    kDeviceError = 0x571,
    kMotorStoppedError = 0x6f9
};
static const unsigned int kInvalidHandle = 0xFFFFFFFFU;

} // extern "C"

// Poll a borrowed DVD command block and distinguish completion from valid progress.
// outComplete is required and cleared even for invalid handles. Return 1 for
// END/BUSY/WAITING (only END sets *outComplete=1), 0 otherwise. Known failures set
// last-error; unlisted statuses leave the previous error intact.
int gtl::CFileIoDeviceDVD::PollCommandCompletion(unsigned int handle, int* outComplete) {
    int status;

    // Phase 1: initialize the output and reject the invalid-handle sentinel.
    *outComplete = 0;
    if (handle == kInvalidHandle) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }

    // Phase 2: successful polling includes commands that are still in flight.
    status = DVDGetCommandBlockStatus(handle);
    if (status == kDvdEnd) {
        *outComplete = 1;
        File_SetLastError(kNoError);
        return 1;
    }

    if (status == kDvdBusy || status == kDvdWaiting) {
        File_SetLastError(kNoError);
        return 1;
    }
    // Phase 3: translate recognized terminal/device states in the original order.
    if (status == kDvdFatal)
        File_SetLastError(kDeviceError);
    if (status == kDvdCanceled)
        File_SetLastError(kCanceledError);
    if (status == kDvdRetry)
        File_SetLastError(kRetryError);
    if (status == kDvdCoverOpen)
        File_SetLastError(kCoverOpenError);
    if (status == kDvdNoDisc)
        File_SetLastError(kDiscError);
    if (status == kDvdWrongDisc)
        File_SetLastError(kDiscError);
    if (status == kDvdMotorStopped)
        File_SetLastError(kMotorStoppedError);
    if (status == kDvdCoverClosed)
        File_SetLastError(kDeviceError);
    return 0;
}
