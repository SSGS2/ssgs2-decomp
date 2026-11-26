// GtlFileIoDeviceDVD read and seek completion handlers. The read handler adds each finished chunk,
// submits the next one, and calls the caller back when all bytes are read or on error. The seek handler
// passes the seek result to the caller. This file also reports the transfer alignment.

#include "classes/GtlFileIoDeviceDVD.h"

extern "C" {

enum DvdIoConstants {
    kDvdTransferAlignment = 0x20,
    kDvdReadChunkBytes = 0xF000,
    kDvdResultFatal = -1,
    kDvdResultStatus10 = 10,
    kFileErrorDvdReadFailed = 0x571,
    kFileErrorDvdStatus10 = 0x4C7,
    kFileErrorSubmissionFailed = 0x1E
};

struct DVDContext {
    char operationPrefix[0x40];
    int bytesTransferred;
    int bytesRemaining;
    int destinationAddress; // Next unsubmitted destination address, not the last chunk's.
    int fileOffset;         // Next unsubmitted file offset in bytes.
    int priority;
    void* callback; // (errorCode, completedBytesOrSeekResult, context, userData).
    void* userData; // Borrowed opaque value forwarded unchanged to callback.
};

struct DVDRequest {
    char commandBlockPrefix[0x2C];
    DVDContext* context;
};

void File_SetLastError(int);
int DVDReadAsyncPrio(int, void*, int, int, void*, int);

} // extern "C"

// Report the device's required transfer alignment.
int gtl::CFileIoDeviceDVD::GetTransferAlignment() {
    return kDvdTransferAlignment;
}

// Accumulate a completed read, submit its next chunk, or notify the caller of completion/error.
// result is the SDK byte count or negative status. request/context/destination must
// outlive every resubmitted read; this callback never frees them. Completion is
// notified only on a terminal result or submission failure (and only if non-null).
void gtl::CFileIoDeviceDVD::OnReadChunkComplete(int result, DVDRequest* request) {
    DVDRequest* originalRequest = request;
    DVDContext* context = request->context;
    if (result < 0) {
        // Phase 1a: map a failed chunk without adding it to transferred bytes.
        // Unrecognized negatives deliberately map to zero; status +10 cannot
        // satisfy this branch. These are existing behaviors, not validation.
        int code = 0;
        if (result == kDvdResultFatal)
            code = kFileErrorDvdReadFailed;
        // Retain the original status-10 check inside the negative-result branch.
        if (result == kDvdResultStatus10)
            code = kFileErrorDvdStatus10;
        File_SetLastError(code);
        if (context->callback)
            ((void (*)(int, int, DVDContext*, void*))context->callback)(code, context->bytesTransferred, context,
                                                                        context->userData);
    } else {
        // Phase 1b: account for the chunk which has just completed.
        context->bytesTransferred += result;
        context->bytesRemaining -= result;
        if (context->bytesRemaining > 0) {
            // Phase 2: submit at most 60 KiB from the saved next-chunk cursor.
            int amount = context->bytesRemaining < kDvdReadChunkBytes ? context->bytesRemaining : kDvdReadChunkBytes;
            if (!DVDReadAsyncPrio((int)originalRequest, (void*)context->destinationAddress, amount, context->fileOffset,
                                  (void*)&gtl::CFileIoDeviceDVD::OnReadChunkComplete, context->priority)) {
                File_SetLastError(kFileErrorSubmissionFailed);
                if (context->callback)
                    ((void (*)(int, int, DVDContext*, void*))context->callback)(
                        kFileErrorSubmissionFailed, context->bytesTransferred, context, context->userData);
            } else {
                // Advance by the submitted size only after the SDK accepted it.
                // bytesTransferred changes on completion, not submission.
                context->destinationAddress += amount;
                context->fileOffset += amount;
                File_SetLastError(0);
            }
        } else {
            // Phase 3: all requested bytes completed; report the aggregate count.
            File_SetLastError(0);
            if (context->callback)
                ((void (*)(int, int, DVDContext*, void*))context->callback)(0, context->bytesTransferred, context,
                                                                            context->userData);
        }
    }
}

// Translate the seek result and invoke the operation's completion callback.
// Borrowed request/context; the second callback argument is zero on an error,
// the SDK result on success. As above, unknown negative results map to code zero.
void gtl::CFileIoDeviceDVD::OnSeekComplete(int result, DVDRequest* request) {
    DVDContext* context = request->context;
    if (result < 0) {
        int code = 0;
        if (result == kDvdResultFatal)
            code = kFileErrorDvdReadFailed;
        // Retain the original status-10 check inside the negative-result branch.
        if (result == kDvdResultStatus10)
            code = kFileErrorDvdStatus10;
        File_SetLastError(code);
        if (context->callback)
            ((void (*)(int, int, DVDContext*, void*))context->callback)(code, 0, context, context->userData);
    } else {
        File_SetLastError(0);
        if (context->callback)
            ((void (*)(int, int, DVDContext*, void*))context->callback)(0, result, context, context->userData);
    }
}
