#include "classes/GtlDvdFileStream.h"
// Borrow the stream layout without constructing or retaining it.
static inline GtlDvdFileStream* AsGtlDvdFileStream(void* object) { return (GtlDvdFileStream*)object; }
// Translation unit 0x80412608-0x80412664 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

int DVDCancelAsync(void*, void*);
void gtl_DvdFileStream_OnCancelComplete();

// Store cancellation notification context and submit an asynchronous DVD cancellation.
// Returns true if accepted, not if completed. self/userData must survive completion.
// Completion clears cancelPending then invokes callback(result, self, userData),
// if non-null. Failed submission retains both new context values and the old pending flag.
bool gtl_nw4r_ut_DvdFileStream_CancelAsync(void* self, void* callback, void* userData) {
    AsGtlDvdFileStream(self)->cancelCallback = callback;
    AsGtlDvdFileStream(self)->cancelUserData = userData;
    int result = DVDCancelAsync(AsGtlDvdFileStream(self)->fileInfo, (void*)gtl_DvdFileStream_OnCancelComplete);
    if (result)
        AsGtlDvdFileStream(self)->cancelPending = 1;
    return result != 0;
}

} // extern "C"
