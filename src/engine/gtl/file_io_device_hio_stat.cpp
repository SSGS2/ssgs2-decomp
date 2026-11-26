// Host file device, GetFileStat: fills a 0x34-byte stat record with the file's size in bytes. The
// record is cleared first, and a fixed word at +0x28 is set to 1. The size comes from a virtual call
// on the device. A null output or an invalid handle returns 0; the invalid handle sets the last error.

#include "types.h"
#include "sdk_protos.h"
#include "classes/GtlFileIoDeviceHIO.h"
#include "classes/FmemFileStat.h"

struct V9 {
    // Minimal ABI view: device slot m7 is queried for the file size in bytes.
    // Other slots only preserve dispatch offsets; their signatures are unresolved.
    virtual int m0(void*, int, int, int);
    virtual int m1(void*, int, int, int);
    virtual int m2(void*, int, int, int);
    virtual int m3(void*, int, int, int);
    virtual int m4(void*, int, int, int);
    virtual int m5(void*, int, int, int);
    virtual int m6(void*, int, int, int);
    virtual int m7(void*, int, int, int);
    virtual int m8(void*, int, int, int);
    virtual int m9(void*, int, int, int);
};

extern "C" {

void File_SetLastError(int x);
void* memset(void*, int, unsigned long);

enum HioStatConstants {
    kInvalidHandleError = 6,
    kFileStatBytes = 0x34,
    kStatWord28Value = 1
};

} // extern "C"

// Clear the HIO status record, query size in bytes and write the fixed +0x28 word.
// Return 0 for null output (last error untouched) or handle -1 (last error 6).
// Otherwise return 1 even for a negative size result; this path does not explicitly
// clear last error. fileStat is borrowed writable storage of at least 0x34 bytes,
// larger than the known FmemFileStat view; neither handle nor output is retained.
int gtl::CFileIoDeviceHIO::GetFileStat(void* handle, void* fileStat) {
    if (fileStat == 0) {
        return 0;
    }
    if (handle == (void*)-1) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    memset(fileStat, 0, kFileStatBytes);
    int fileSize = ((V9*)this)->m7(handle, 0, 0, 0);
    AsFmemFileStat(fileStat)->size = fileSize;           // 0x80418FEC: file size from device slot m7
    AsFmemFileStat(fileStat)->unk_28 = kStatWord28Value; // Word meaning remains unresolved.
    return 1;
}
