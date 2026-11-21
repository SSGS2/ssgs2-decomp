#include "classes/ChunkedDvdReadRequest.h"
// Translation unit 0x80413768-0x8041385C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

int File_SetLastError(int);
int DVDReadAsyncPrio(void*, void*, int, int, void*, int);
void gtl_CFileIoDeviceDVD_OnReadChunkComplete(void*);

enum DvdReadConstants {
    kInvalidRequest = -1, kInvalidHandleError = 6,
    kDefaultPriorityCode = 0xA0, kPriorityCodeShift = 6,
    kMaximumChunkBytes = 0xF000, kSubmissionFailedError = 0x1E
};

#pragma use_lmw_stmw on
// Start a chunked asynchronous DVD read; return 1 when accepted, 0 on immediate error.
// request and destination are borrowed and must survive all chunk completions.
// length/offset/cursors are bytes; DVD alignment and nonnegative sizes are caller
// requirements. priorityCode=-1 chooses 0xA0; the SDK receives priorityCode >> 6.
// completionArg is a callback address and completionArg2 its opaque user word;
// the completion handler calls (error, bytesRead, request, userWord). Neither is
// retained outside the request. Success clears last error, but does not mean I/O finished.
int gtl_CFileIoDeviceDVD_StartChunkedAsyncRead(int unused, void* request, void* destination,
                                  int length, int offset, unsigned int priorityCode,
                                  int completionArg,
                                  int completionArg2) {
    if ((unsigned int)request == (unsigned int)kInvalidRequest) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    if (priorityCode == (unsigned int)kInvalidRequest)
        priorityCode = kDefaultPriorityCode;

    // Phase 1: reset completion state and install the command's back-pointer.
    ((ChunkedDvdReadRequest*)request)->completionCallback = 0;
    ((ChunkedDvdReadRequest*)request)->priority = priorityCode >> kPriorityCodeShift;
    ((ChunkedDvdReadRequest*)request)->completionUserData = 0;
    ((ChunkedDvdReadRequest*)request)->bytesRead = 0;
    ((ChunkedDvdReadRequest*)request)->remainingBytes = length;
    ((ChunkedDvdReadRequest*)request)->readContext = request;

    // Phase 2: submit at most 60 KiB; failed submission leaves the reset state above.
    int firstChunkBytes = length < kMaximumChunkBytes ? length : kMaximumChunkBytes;
    if (!DVDReadAsyncPrio(request, destination, firstChunkBytes, offset, (void*)gtl_CFileIoDeviceDVD_OnReadChunkComplete,
                      ((ChunkedDvdReadRequest*)request)->priority)) {
        File_SetLastError(kSubmissionFailedError);
        return 0;
    }
    // Phase 3: save the unsubmitted cursor and publish caller notification arguments.
    ((ChunkedDvdReadRequest*)request)->nextBuffer = (int)destination + firstChunkBytes;
    ((ChunkedDvdReadRequest*)request)->nextOffset = offset + firstChunkBytes;
    ((ChunkedDvdReadRequest*)request)->completionCallback = completionArg;
    ((ChunkedDvdReadRequest*)request)->completionUserData = completionArg2;
    File_SetLastError(0);
    return 1;
}
#pragma use_lmw_stmw reset

} // extern "C"
