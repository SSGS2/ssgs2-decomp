// Handle released by gtl_CFileIoDeviceFMEM_ReleaseFileHandle (0x8041C840).
// This is the open handle, separate from the GtlFileIoDeviceFMEM device layout.
#ifndef CLASSES_FMEMFILEHANDLE_H
#define CLASSES_FMEMFILEHANDLE_H

#include "classes/MWStringStorage.h"

struct FmemFileHandle {
    char pad_000[4];
    MWStringStorage stringStorage; // +0x4: heap flag controls freeing its pointer at +0xC.
    void* buffer;                  // +0x10: freed before the name and handle.
    int bufferSize;                // +0x14: reduced to zero before buffer release.
};

#define OFF(f, o) typedef char FmemFileHandle_off_##f[((int)&((FmemFileHandle*)0)->f == (o)) ? 1 : -1]
OFF(stringStorage, 0x4);
OFF(buffer, 0x10);
OFF(bufferSize, 0x14);
#undef OFF
#define AsFmemFileHandle(p) ((FmemFileHandle*)(p))

#endif
