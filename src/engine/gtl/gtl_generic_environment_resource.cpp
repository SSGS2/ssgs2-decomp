// Generic environment resource: the destructor for the environment resource wrapper, which releases its
// retained buffer and frees the wrapper when asked.
//
// The class layout is classes/GtlIGenericEnvironmentResource.h.

#include "classes/GtlIGenericEnvironmentResource.h"

struct GtlGenericResourceFileBase {
    int refCount;
};
struct GtlGenericResourceFile : GtlGenericResourceFileBase {
    virtual void dispose(int);
};

struct GtlSdaFnSlot {
    int unk0;
    void (*fn)(void*);
};

extern "C" {

extern char __vt__Q23gtl20IGenericResourceFile[];
extern GtlSdaFnSlot s_genericResourceAllocatorFunctions;

} // extern "C"

// Release the retained buffer and clear the vtable; frees the wrapper when flag > 0.
gtl::IGenericEnvironmentResource* gtl::IGenericEnvironmentResource::Destroy(int flag) {
    if (this) {
        if (this) {
            GtlGenericResourceFile* res = (GtlGenericResourceFile*)resourceBuffer;
            vtable = __vt__Q23gtl20IGenericResourceFile;
            if (res) {
                if (--res->refCount == 0 && res) {
                    res->dispose(1);
                }
                resourceBuffer = 0;
            }
        }
        if (flag > 0) {
            s_genericResourceAllocatorFunctions.fn(this);
        }
    }
    return this;
}
