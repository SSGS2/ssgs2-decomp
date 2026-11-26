// GTL FMEM device file close: closes the NAND driver state for a file, records the
// translated driver error, and frees the file's handle allocation.

#include "types.h"
#include "classes/GtlFileIoDeviceFMEM.h"

extern "C" {

void File_SetLastError(int x);
void FileIO_Free(void* p);
int NANDClose(void* p);

// Numeric names preserve the sparse map, including the absent -7 case.
enum NandFileError {
    kFileSuccess = 0,
    kInvalidHandleError = 6,
    kNandErrorBase = 0xC350,
    kNandErrorMinus64 = 0xC360,
    kNandErrorMinus128 = 0xC361
};

} // extern "C"

// Close NAND state and release its handle allocation even when NANDClose fails.
// Consumes any nonnull handle other than -1; returns 1 only for driver result 0.
// Null is passed to NANDClose but is not freed; it is not an early success case.
int gtl::CFileIoDeviceFMEM::CloseFile(void* handle) {
    if (handle == (void*)-1) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    // Phase 1: close the driver state and record its translated error.
    int closeResult = NANDClose(handle);
    int fileError;
    switch (closeResult) {
    case 0:
        fileError = kFileSuccess;
        break;
    case -1:
        fileError = kNandErrorBase + 1;
        break;
    case -2:
        fileError = kNandErrorBase + 2;
        break;
    case -3:
        fileError = kNandErrorBase + 3;
        break;
    case -4:
        fileError = kNandErrorBase + 4;
        break;
    case -5:
        fileError = kNandErrorBase + 5;
        break;
    case -6:
        fileError = kNandErrorBase + 6;
        break;
    case -8:
        fileError = kNandErrorBase + 8;
        break;
    case -9:
        fileError = kNandErrorBase + 9;
        break;
    case -10:
        fileError = kNandErrorBase + 10;
        break;
    case -11:
        fileError = kNandErrorBase + 11;
        break;
    case -12:
        fileError = kNandErrorBase + 12;
        break;
    case -13:
        fileError = kNandErrorBase + 13;
        break;
    case -14:
        fileError = kNandErrorBase + 14;
        break;
    case -64:
        fileError = kNandErrorMinus64;
        break;
    case -15:
        fileError = kNandErrorBase + 15;
        break;
    case -128:
        fileError = kNandErrorMinus128;
        break;
    default:
        fileError = kNandErrorMinus64;
        break;
    }
    File_SetLastError(fileError);
    // Phase 2: dispose of the handle regardless of the recorded close result.
    if (handle != 0) {
        FileIO_Free(handle);
    }
    return (closeResult == 0) ? 1 : 0;
}
