// Translation unit 0x80412AE8-0x80412B64 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void File_SetLastError(int error);
int fn_8034A774(unsigned int file, unsigned int offset, int mode);

enum { kInvalidDvdFile = 0xFFFFFFFFu, kDvdPositionMode2 = 2,
    kInvalidHandleError = 6, kSeekError = 25, kNoError = 0 };

// Forward a DVD position request with mode 2, clear the optional output and map errors.
// self is unused; file is a borrowed handle word. The callee is not defined in
// current source, so mode 2's "from end" label remains unverified. offset is passed
// unchanged. clearedOutput receives only zero, not bytes read or a new position;
// it is untouched for an invalid handle, but cleared before a callee failure.
// Returns 0 on success, -1 on failure; sets the file layer's last-error either way.
int gtl_CFileIoDeviceDVD_Seek(void* self, unsigned int file, unsigned int offset, int* clearedOutput) {
    if (file == kInvalidDvdFile) {
        File_SetLastError(kInvalidHandleError);
        return -1;
    }

    if (clearedOutput != 0) {
        *clearedOutput = 0;
    }

    int result = fn_8034A774(file, offset, kDvdPositionMode2);
    if (result < 0) {
        File_SetLastError(kSeekError);
        return -1;
    }

    File_SetLastError(kNoError);
    return 0;
}

} // extern "C"
