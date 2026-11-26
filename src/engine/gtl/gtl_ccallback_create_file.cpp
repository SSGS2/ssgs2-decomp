// Destructor for the create-file callback (gtl::CCallBackCreateFile). It releases the owned path
// string through the file-I/O allocator, resets the vtable and frees the object when asked.

#include "types.h"
#include "classes/GtlCreateFileCallback.h"

extern "C" {
void FileIO_Free(void* p);
extern char __vt__Q23gtl19CCallBackCreateFile[];
}

namespace gtl {

// Release the create callback's owned path, then optionally free the callback itself.
// The captured file/completion callback are borrowed and untouched. Null is accepted;
// deleting > 0 frees this. The ABI returns this even when that address has been freed.
void* CCallBackCreateFile::Destroy(int deleting) {
    if (this) {
        void* ownedPath = path;
        vtbl = (void*)__vt__Q23gtl19CCallBackCreateFile;
        if (ownedPath) {
            FileIO_Free(ownedPath);
            path = 0;
        }
        if (deleting > 0) {
            FileIO_Free(this);
        }
    }
    return this;
}

} // namespace gtl
