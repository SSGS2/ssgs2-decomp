// GTL FMEM device file-handle release: frees the handle's buffer, its heap string if it
// has one, and the handle allocation itself.

#include "types.h"
#include "sdk_protos.h"
#include "classes/FmemFileHandle.h"
#include "classes/GtlFileIoDeviceFMEM.h"

extern "C" {
extern void File_SetLastError(int x);
extern void FileIO_Free(void* p);

enum FmemReleaseConstants {
    kInvalidHandleError = 6,
    kFileSuccess = 0,
    kBufferStorageOffset = 0x10,
    kStringStorageOffset = 0x04,
    kHeapStringFlagShift = 31
};

// Release buffer storage through the five original destructor layers.
// Integer subobject-address tests are ABI scaffolding, not allocation validation.
// The subtraction deliberately emits size-size before free; assigning literal
// zero changes instructions. There is no byte-range computation to tune here.
static inline void ReleaseOwnedFmemBuffer(void* handle) {
    int bufferGuard1 = (int)handle + kBufferStorageOffset;
    if (bufferGuard1) {
        int bufferGuard2 = (int)handle + kBufferStorageOffset;
        if (bufferGuard2) {
            int bufferGuard3 = (int)handle + kBufferStorageOffset;
            if (bufferGuard3) {
                int bufferGuard4 = (int)handle + kBufferStorageOffset;
                if (bufferGuard4) {
                    int bufferGuard5 = (int)handle + kBufferStorageOffset;
                    if (bufferGuard5) {
                        if (AsFmemFileHandle(handle)->buffer != 0) {
                            void* ownedBuffer = AsFmemFileHandle(handle)->buffer;
                            // Matched form of resetting size to zero before disposal.
                            AsFmemFileHandle(handle)->bufferSize -=
                                ((char*)handle + AsFmemFileHandle(handle)->bufferSize) - (char*)handle;
                            FileIO_Free(ownedBuffer);
                        }
                    }
                }
            }
        }
    }
}
} // extern "C"

// Release an FMEM handle's owned buffer, optional heap string, and handle allocation.
// Return 0 only for sentinel -1; null is a successful no-op. All other handles
// must be valid owned allocations. No pointer is cleared for the caller after release.
int gtl::CFileIoDeviceFMEM::ReleaseFileHandle(void* handle) {
    if (handle == (void*)-1) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    if (handle != 0) {
        // Phase 1: release the owned buffer through its matched cleanup helper.
        do {
            if (handle == 0)
                break;
            ReleaseOwnedFmemBuffer(handle);
            // Phase 2: free string storage only when bit 31 marks heap ownership.
            // Inline text shares the handle allocation and needs no separate release.
            int stringGuard = (int)handle + kStringStorageOffset;
            if (stringGuard) {
                if (AsFmemFileHandle(handle)->stringStorage.flags >> kHeapStringFlagShift) {
                    FileIO_Free(AsFmemFileHandle(handle)->stringStorage.data);
                }
            }
            // Phase 3: child allocations are gone; finally release the handle.
            FileIO_Free(handle);
        } while (0);
    }
    File_SetLastError(kFileSuccess);
    return 1;
}
