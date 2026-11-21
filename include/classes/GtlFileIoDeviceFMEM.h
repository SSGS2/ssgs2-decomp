// gtl::CFileIoDeviceFMEM: partial view; vfunc_18 lives in gtl_file_io_device_fmem_disk_space.cpp.
// No inheritance, virtual declarations, constructors, or emitted code.
#ifndef CLASSES_GTLFILEIODEVICEFMEM_H
#define CLASSES_GTLFILEIODEVICEFMEM_H

#include "classes/MWStringStorage.h"

struct GtlFileIoDeviceFMEM {
    char pad_000[0x10];
    MWStringStorage cachedCurrentDirectory; // 0x10: vfunc_18 selects small/heap text for NANDChangeDir and FileIoDevice_AccumulateDirectoryAllocationSize
};

#define OFF(f, o) typedef char GtlFileIoDeviceFMEM_off_##f[((int)&((GtlFileIoDeviceFMEM*)0)->f == (o)) ? 1 : -1]
OFF(cachedCurrentDirectory, 0x10);
#undef OFF
#define AsGtlFileIoDeviceFMEM(p) ((GtlFileIoDeviceFMEM*)(p))

#endif
