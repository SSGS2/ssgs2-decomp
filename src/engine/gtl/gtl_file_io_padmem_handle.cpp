// PAD-memory device: closes a handle through its deleting destructor, and frees the handle storage.

#include <decomp/local_class.h>
#include "classes/GtlFileIoDevicePADMEM.h"
#include "classes/gtl_CHandlePADMEM.h"
#include "sdk_protos.h"

extern "C" void FileIO_Free(void* p);
extern "C" void File_SetLastError(int x);

enum PadMemHandleStatus {
    kPadMemNoError = 0,
    kPadMemInvalidHandle = 6
};
enum {
    kDeleteHandleStorage = 1
};

// Consume a non-null PADMEM handle through its deleting destructor; null succeeds.
// The device is not used. The -1 sentinel fails (0/error 6); all other paths return 1/error 0.
// A real handle must have a valid vtable; this function does not contact the driver.
int gtl::CFileIoDevicePADMEM::CloseHandle(void* handle) {
    if (handle == (void*)-1) {
        File_SetLastError(kPadMemInvalidHandle);
        return 0;
    }
    if (handle != 0) {
        if (handle != 0) {
            struct H {
                virtual void c(int) DECOMP_LOCAL_VIRTUAL;
            }; // First slot is the deleting destructor.
            ((H*)handle)->c(kDeleteHandleStorage);
        }
    }
    File_SetLastError(kPadMemNoError);
    return 1;
}

// Free handle storage only for a positive deleting flag; no driver close occurs here.
// Null is accepted. Return the original address, which may now be dangling.
void* gtl::CHandlePADMEM::Destroy(int deleting) {
    if (this != 0 && deleting > 0) {
        FileIO_Free(this);
    }
    return this;
}
