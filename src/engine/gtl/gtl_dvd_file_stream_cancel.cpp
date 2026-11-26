// DvdFileStream::CancelAsync: stores the cancel callback and its user data, then submits
// an asynchronous DVD cancel for the stream's file.

#include "classes/GtlDvdFileStream.h"

extern "C" {

int DVDCancelAsync(void*, void*);

} // extern "C"

// Store cancellation notification context and submit an asynchronous DVD cancellation.
// Returns true if accepted, not if completed. self/userData must survive completion.
// Completion clears cancelPending then invokes callback(result, self, userData),
// if non-null. Failed submission retains both new context values and the old pending flag.
bool gtl::nw4r::ut::DvdFileStream::CancelAsync(void* callback, void* userData) {
    cancelCallback = callback;
    cancelUserData = userData;
    int result = DVDCancelAsync(fileInfo, (void*)OnCancelComplete);
    if (result)
        cancelPending = 1;
    return result != 0;
}
