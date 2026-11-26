// DVD device: CloseDirectorySearch, which closes an open DVD directory search and frees its state,
// including a heap-allocated name when one is held. The handle is released on success or failure.

#include "classes/DvdDeviceFileState.h"
#include "classes/GtlFileIoDeviceDVD.h"
extern "C" void FileIO_Free(void* p);
extern "C" void File_SetLastError(int x);
extern "C" int fn_8034A90C(void* p);

extern "C" {

enum DvdCloseError {
    kDvdCloseNoError = 0,
    kDvdCloseInvalidHandle = 6,
    kHeapNameFlagShift = 31,
    kInvalidDvdHandle = -1
};

} // extern "C"

// Close the DVD file, release its owned name and handle, and report the close result.
// Return 0 for handle -1 or a failed low-level close, otherwise 1. Both low-level
// outcomes consume the handle and clear last-error; a failed close cannot be retried.
// Requires a live handle other than -1; null is passed to the close routine too.
// The duplicated null guards and volatile flag reads preserve existing codegen.
int gtl::CFileIoDeviceDVD::CloseDirectorySearch(void* fileHandle) {
    if (fileHandle == (void*)kInvalidDvdHandle) {
        File_SetLastError(kDvdCloseInvalidHandle);
        return 0;
    }

    // Phase 1: close the underlying DVD state before releasing any handle memory.
    if (fn_8034A90C(fileHandle) == 0) {
        // Failed close still frees a heap-backed name (top flag bit) and the handle.
        if (fileHandle != 0) {
            if (fileHandle != 0) {
                if ((unsigned int)fileHandle + 0xC != 0) {
                    if ((AsDvdDeviceFileState(fileHandle)->nameFlags >> kHeapNameFlagShift) != 0) {
                        FileIO_Free(AsDvdDeviceFileState(fileHandle)->nameData);
                    }
                }
                FileIO_Free(fileHandle);
            }
        }
        File_SetLastError(kDvdCloseNoError);
        return 0;
    }

    // Phase 2: successful close uses the same ownership cleanup before reporting 1.
    if (fileHandle != 0) {
        if (fileHandle != 0) {
            if ((unsigned int)fileHandle + 0xC != 0) {
                if ((AsDvdDeviceFileState(fileHandle)->nameFlags >> kHeapNameFlagShift) != 0) {
                    FileIO_Free(AsDvdDeviceFileState(fileHandle)->nameData);
                }
            }
            FileIO_Free(fileHandle);
        }
    }
    File_SetLastError(kDvdCloseNoError);
    return 1;
}
