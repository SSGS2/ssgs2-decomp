// Completion callbacks for two NAND requests: removing a directory and checking a save area.
// Each one turns the NAND result into a file error code, notifies its client, then releases
// its own storage.

#include "types.h"
#include "classes/gtl_CCallBackRemoveDirectory.h"
#include "classes/gtl_CCallBackRVLCheckSaveArea.h"

extern "C" {
void FileIO_Free(void* p);
void FileIO_InvokeAllocatorCallback3(void* p);
int File_SetLastError(int x);
}

// Optionally free this callback through FileIO; return the original address.
// shouldDelete > 0 releases storage; a returned freed address is not usable.
void* gtl::CCallBackRemoveDirectory::Destroy(int shouldDelete) {
    if (this != 0) {
        if (shouldDelete > 0) {
            FileIO_Free(this);
        }
    }
    return this;
}

extern "C" {

// Views the object's destructor virtual (vtable slot 2) for the teardown call.
struct CallBackRVLCheckSaveAreaVirtuals {
    virtual void teardown(int);
};
enum {
    kFileNandErrorBase = 0xC350,
    kFileNandError64 = 0xC360,
    kFileNandError128 = 0xC361,
    kCallbackTeardownArgument = -1
};

} // extern "C"

// Translate a NAND result, notify the client, then dispose payload and callback.
// The object is consumed. payload is consumed if non-null and MUST be non-null when
// the client callback exists: its first int is the callback's second argument.
// The third argument is the opaque word at +8. Notification is synchronous;
// neither payload nor the object may be retained by the client. No result is returned.
void gtl::CCallBackRVLCheckSaveArea::Complete(int nandResult, void* payload) {
    // Phase 1: translate only the explicitly listed results. -7 is deliberately
    // absent and, like unknown values, maps to the generic 0xC360 error.
    // 0xC351..0xC35F preserve the magnitude of -1..-15; -64/-128 map to
    // 0xC360/0xC361. These are FileIO errors, not UI message identifiers.
    int fileError = 0;
    switch (nandResult) {
    case 0:
        fileError = 0;
        break;
    case -1:
        fileError = kFileNandErrorBase + 1;
        break;
    case -2:
        fileError = kFileNandErrorBase + 2;
        break;
    case -3:
        fileError = kFileNandErrorBase + 3;
        break;
    case -4:
        fileError = kFileNandErrorBase + 4;
        break;
    case -5:
        fileError = kFileNandErrorBase + 5;
        break;
    case -6:
        fileError = kFileNandErrorBase + 6;
        break;
    case -8:
        fileError = kFileNandErrorBase + 8;
        break;
    case -9:
        fileError = kFileNandErrorBase + 9;
        break;
    case -10:
        fileError = kFileNandErrorBase + 10;
        break;
    case -11:
        fileError = kFileNandErrorBase + 11;
        break;
    case -12:
        fileError = kFileNandErrorBase + 12;
        break;
    case -13:
        fileError = kFileNandErrorBase + 13;
        break;
    case -14:
        fileError = kFileNandErrorBase + 14;
        break;
    case -15:
        fileError = kFileNandErrorBase + 15;
        break;
    case -64:
        fileError = kFileNandError64;
        break;
    case -128:
        fileError = kFileNandError128;
        break;
    default:
        fileError = kFileNandError64;
        break;
    }
    // Phase 2: publish the translated error before invoking the optional client.
    File_SetLastError(fileError);
    {
        void* clientCallback = cb;
        if (clientCallback) {
            typedef void (*CbFn)(int, int, int);
            ((CbFn)clientCallback)(fileError, *(int*)payload, field8);
        }
    }
    // Phase 3: release payload, invoke virtual teardown, then release self.
    // Slot 3 is the allocator callback used for disposal; its implementation
    // is externally configured, so do not substitute FileIO_Free.
    if (payload)
        FileIO_InvokeAllocatorCallback3(payload);
    ((CallBackRVLCheckSaveAreaVirtuals*)this)->teardown(kCallbackTeardownArgument);
    FileIO_InvokeAllocatorCallback3(this);
}
