// GTL FMEM device file stat: zeroes a stat buffer, stores the file size from the device's
// size query, and sets the fixed stat word.

#include "classes/FmemFileStat.h"
#include "types.h"
#include "classes/GtlFileIoDeviceFMEM.h"

// Virtual view of the device used only for the file-size call. The declared
// destructor takes two vtable entries, so v7 is vtable entry 9 (byte offset 0x24).
struct FmemDeviceVirtualView {
    virtual ~FmemDeviceVirtualView();
    virtual void* v1();
    virtual void* v2();
    virtual void* v3();
    virtual void* v4();
    virtual void* v5();
    virtual void* v6();
    virtual int v7(int, int, int, int);
};

extern "C" {
void File_SetLastError(int x);
void* memset(void*, int, unsigned int);

enum {
    kInvalidFileHandle = -1,
    kInvalidHandleError = 6,
    kFileStatBufferBytes = 0x34
};
}

// Zero a stat result, query the file size, and set the device's fixed stat word for a valid handle.
// outputStat borrows >=0x34 writable bytes. Null output returns 0 without updating
// last error; handle -1 returns 0 with error 6. Otherwise return 1 and clear error,
// even if v7 returned a negative file-size/error value (stored unchanged at +0x24).
// No file ownership transfers; all stat bytes not explicitly assigned remain zero.
int gtl::CFileIoDeviceFMEM::GetFileStat(unsigned int fileHandle, void* outputStat) {
    if (outputStat == 0) {
        return 0;
    }
    if (fileHandle == (unsigned int)kInvalidFileHandle) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    memset(outputStat, 0, kFileStatBufferBytes);
    int fileSize = ((FmemDeviceVirtualView*)this)->v7(fileHandle, 0, 0, 0);
    AsFmemFileStat(outputStat)->size = fileSize;
    AsFmemFileStat(outputStat)->unk_28 = 1; // Fixed device-stat word; its interpretation requires a consumer.
    File_SetLastError(0);
    return 1;
}
