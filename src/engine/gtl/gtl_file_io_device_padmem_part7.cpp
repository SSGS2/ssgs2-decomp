// PADMEM file device destructor: frees the heap name string and, when asked, the device object.

#include "classes/GtlFileIoDevicePADMEM.h"

extern "C" void FileIO_Free(void* p);

// Destroy the owned name string and optionally free the PADMEM object; return its old address.
// flags > 0 frees the object; otherwise only heap name storage is freed. Inline names
// need no release. Null this is accepted. Returned storage may already be freed,
// and string fields are not reset, so this does not leave an object safe to reuse.
// The nested this and member-address guards are kept: the original compiles them this way.
void* gtl::CFileIoDevicePADMEM::Destroy(int flags) {
    if (this != 0) {
        if (this != 0) {
            if ((unsigned int)this + 4 != 0) {
                if (path.flags >> 31) {
                    FileIO_Free(path.data);
                }
            }
        }
        if (flags > 0) {
            FileIO_Free(this);
        }
    }
    return this;
}
