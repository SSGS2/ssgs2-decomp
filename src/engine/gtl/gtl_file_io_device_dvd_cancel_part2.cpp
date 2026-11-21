// Translation unit 0x80413924-0x8041398C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void File_SetLastError(int error);
int DVDCancelAsync(const char* fileHandle, int callbackAddress);

enum { kInvalidDvdHandle = 0xFFFFFFFFu, kInvalidHandleError = 6, kNoCompletionCallback = 0 };

// Request asynchronous cancellation with no completion callback; return boolean submission status.
// fileHandle is an opaque borrowed DVD handle despite its char* type; it is not a path.
// Caller keeps handle storage valid for the pending SDK operation. self is unused.
// Handle -1 reports error 6; both accepted and rejected SDK requests clear last-error.
int gtl_CFileIoDeviceDVD_CancelAsyncRequest(void* self, const char* fileHandle)
{
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

} // extern "C"
