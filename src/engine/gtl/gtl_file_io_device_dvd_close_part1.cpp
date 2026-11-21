// Translation unit 0x80412938-0x804129C8 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
// DVD close adapter (infrastructure; no gameplay tuning). API map:
// gtl_CFileIoDeviceDVD_CloseFile consumes FileIO's allocated DVD handle;
// DVDClose closes the SDK command/file block, FileIO_Free frees its wrapper,
// File_SetLastError publishes the adapter error. Related operations are in
// file_io_device_dvd_read.cpp (read), gtl_file_io_device_dvd_size.cpp (size) and gtl_file_io_device_dvd_poll.cpp (completion poll).

extern "C" {

int File_SetLastError(int);
int DVDClose(unsigned int);
void FileIO_Free(unsigned int);

static const unsigned int kInvalidHandle = 0xFFFFFFFFU;
enum { kInvalidHandleError = 6, kNoError = 0 };

// Close a DVD file and consume its allocated handle, even when DVDClose fails.
// Return the boolean DVDClose result. Sentinel -1 sets last-error=6 and returns
// 0 without close/free. BOTH SDK close outcomes set last-error=0, including
// failure: a false return here does not promise a nonzero last-error code.
// self is unused. A nonzero valid handle must belong to FileIO's allocator and
// must not be reused after this call. Zero is still passed to DVDClose.
// The handle/block must have no outstanding operation using its storage. No
// retry is possible after failure because the wrapper is consumed regardless.
int gtl_CFileIoDeviceDVD_CloseFile(void* self, unsigned int fileHandle) {
    if (fileHandle == kInvalidHandle) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }

    if (DVDClose(fileHandle) == 0) {
        if (fileHandle != 0) {
            FileIO_Free(fileHandle);
        }
        File_SetLastError(kNoError);
        return 0;
    }

    if (fileHandle != 0) {
        FileIO_Free(fileHandle);
    }
    File_SetLastError(kNoError);
    return 1;
}

} // extern "C"
