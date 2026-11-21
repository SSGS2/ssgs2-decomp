// Translation unit 0x80420C40-0x80420CB0 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/PadMemDevice.h"
#include "sdk_protos.h"

extern "C" {
extern void FileIO_Free(void* p);

// Destroy the owned name string and optionally free the PADMEM object; return its old address.
// flags > 0 frees self; otherwise only heap name storage is freed. Inline names
// need no release. Null self is accepted. Returned storage may already be freed,
// and string fields are not reset, so this does not leave an object safe to reuse.
void* gtl_CFileIoDevicePADMEM_dtor(void* self, int flags) {
    void* device = self;
    int deleteFlag = flags;
    // Preserve the nested object/member-address guards in the matched destruction.
    if (device != 0) {
        if (device != 0) {
            unsigned int nameStorageAddress = (unsigned int)device + 4;
            if (nameStorageAddress != 0) {
                if ((AsPadMemDevice(device)->nameStorageWord) >> 31) {
                    FileIO_Free(AsPadMemDevice(device)->nameData);
                }
            }
        }
        if (deleteFlag > 0) {
            FileIO_Free(device);
        }
    }
    return device;
}

} // extern "C"
