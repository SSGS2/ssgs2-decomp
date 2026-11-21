// Translation unit 0x80412C6C-0x80412CD0 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void File_SetLastError(int error);
int DVDCancel(unsigned int handle);

enum { kInvalidDvdHandle = 0xFFFFFFFFu, kInvalidHandleError = 6, kDvdCancelFailure = -1 };

// Cancel the borrowed DVD handle synchronously; return 0 for -1 handle or SDK result -1, else 1.
// self is unused. Cancellation does not close/free the handle. Null is not rejected.
// Both SDK success and SDK failure clear last-error; return status is the only distinction.
int gtl_CFileIoDeviceDVD_CancelHandle(void* self, unsigned int handle) {
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

} // extern "C"
