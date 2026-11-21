// GTL DVD device file-size query. API: device virtual slot 7 returns bytes from
// an already opened DVDFileInfo-compatible handle; no gameplay tuning knobs.
// Handle contract: SDK DVDFileInfo (revolution/DVD/dvd.h) length is at +0x34;
// open/close own the handle's lifetime. This query borrows it synchronously and
// does not issue disc I/O, advance the file position, or acquire a reference.
// Translation unit 0x80412B64-0x80412BC4 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void File_SetLastError(int error);

struct DVDFileInfoLengthView {
    char dvdFileInfoPrefix[0x34]; // DVDFileInfo fields preceding its file length.
    int fileLength;
};

enum { kFileNoError = 0, kInvalidFileHandleError = 6 };

// Return the DVD file length and clear the optional high word; reject the invalid handle.
// file is a borrowed DVDFileInfo-compatible handle, not the device object self.
// Only -1 is rejected: other values must be valid. sizeHigh is untouched on error.
// Returns the low 32-bit byte length in an int, or -1 with last-error 6.
// self is an unused ABI receiver (never dereferenced); a null file is not an
// invalid-handle sentinel and would fault. sizeHigh may be null; otherwise it
// must designate a writable word. Length is read before that word is cleared,
// even if they alias; the returned length is the previously loaded value.
int gtl_CFileIoDeviceDVD_GetFileSize(void* self, DVDFileInfoLengthView* file, unsigned int* sizeHigh) {
    if ((unsigned int)file == 0xFFFFFFFF) {
        File_SetLastError(kInvalidFileHandleError);
        return -1;
    }

    int result = file->fileLength;
    if (sizeHigh != 0) {
        *sizeHigh = 0;
    }
    File_SetLastError(kFileNoError);
    return result;
}

} // extern "C"
