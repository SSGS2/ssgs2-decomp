// Translation unit 0x8041A13C-0x8041A1F4 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/FmemFileStat.h"
#include "types.h"

typedef int (*FmemSizeSlot)(void*, int, int, int, int);

struct CFileIoDeviceFMEM {
    virtual int v0();
    virtual int v1();
    virtual int v2();
    virtual int v3();
    virtual int v4();
    virtual int v5();
    virtual int v6();
    virtual int v7(int, int, int, int);
};

extern "C" {
void File_SetLastError(int x);
void* memset(void*, int, unsigned int);

enum { kInvalidFileHandle = -1, kInvalidHandleError = 6, kFileStatBufferBytes = 0x34 };

// Zero a stat result, query the file size, and set the device's fixed stat word for a valid handle.
// outputStat borrows >=0x34 writable bytes. Null output returns 0 without updating
// last error; handle -1 returns 0 with error 6. Otherwise return 1 and clear error,
// even if v7 returned a negative file-size/error value (stored unchanged at +0x24).
// No file ownership transfers; all stat bytes not explicitly assigned remain zero.
int gtl_CFileIoDeviceFMEM_GetFileStat(CFileIoDeviceFMEM* self, unsigned int fileHandle, void* outputStat) {
    if (outputStat == 0) {
        return 0;
    }
    if (fileHandle == (unsigned int)kInvalidFileHandle) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    memset(outputStat, 0, kFileStatBufferBytes);
    int fileSize = (self->v7(fileHandle, 0, 0, 0));
    AsFmemFileStat(outputStat)->size = fileSize;
    AsFmemFileStat(outputStat)->unk_28 = 1; // Fixed device-stat word; its interpretation requires a consumer.
    File_SetLastError(0);
    return 1;
}
} // extern "C"
