// DVD device: GetFileInfo, which fills a FileInfo record with the file size and a fixed status word.
// The size comes from the device's own size query.

#include "classes/GtlFileIoDeviceDVD.h"

extern "C" {

#include "sdk_protos.h"
void* memset(void*, int, unsigned int);

void File_SetLastError(int);

// Virtual-call view of the device, used only to make the size query's call through the vtable.
// The unnamed slots are placeholders kept so the call dispatches at the original vtable entry.
struct DvdDeviceVirtualView {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual int v8(unsigned int, int, int, int);
};

struct FileInfo {
    // Caller supplies 0x34 writable bytes; this path clears all of them, then
    // writes size in bytes at +0x24 and a fixed 1 at +0x28. The first 0x24 bytes
    // and the meaning of +0x28 are unresolved; existing member names are legacy.
    char unresolvedMetadata00[0x24];
    int size;
    int statusWord28;
    char reservedTail[8];
};

enum {
    kInvalidHandleError = 6,
    kFileSuccess = 0,
    kHandleBias = 0x10000,
    kBiasedInvalidHandle = 0xFFFF,
    kStatWord28Value = 1
};
typedef char FileInfo_size_check[(sizeof(FileInfo) == 0x34) ? 1 : -1];

} // extern "C"

// Clear the DVD status record, query size in bytes and write the fixed status word.
// Return 0 for handle 0xFFFFFFFF (output untouched, last error 6), otherwise 1.
// info is borrowed and must be nonnull for any other handle; size-query errors
// are stored verbatim and do not change the success return. Success clears last error.
// The size comes from the device's own size query, called through the vtable (entry at 0x24).
int gtl::CFileIoDeviceDVD::GetFileInfo(unsigned int fileHandle, FileInfo* info) {
    // The unsigned biased comparison is exactly the invalid-handle sentinel test.
    if ((fileHandle + kHandleBias) == kBiasedInvalidHandle) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }

    memset(info, 0, sizeof(FileInfo));
    info->size = ((DvdDeviceVirtualView*)this)->v8(fileHandle, 0, 0, 0);
    info->statusWord28 = kStatWord28Value; // Legacy name; no reader here proves a validity bit.
    File_SetLastError(kFileSuccess);
    return 1;
}
