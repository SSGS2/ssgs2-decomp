// FileHandle: explicit layout view; no inheritance, virtual methods, or RTTI.
// File handle operations live in file_io_facade.cpp; comments cite original function addresses.
#ifndef CLASSES_FILEHANDLE_H
#define CLASSES_FILEHANDLE_H

#include "game/types.h"

class FileHandle {
public:
    void* handle; // 0x0: FileIO_DispatchDeviceHandleOperation10: argument passed to the file device
    void* device; // 0x4: FileIO_DispatchDeviceHandleOperation10: receiver for virtual file operations
    char pad_008[0xc];
    unsigned int flags; // 0x14: FileIO_ReadFile: 0x40000000 async, 0x20000000 direct buffer
    void*
        transferBuffer; // 0x18: FileIO_ReadFile/FileIO_WriteFile/FileIO_PollAsyncTransferResult: device transfer buffer
    unsigned int allocatedBytes; // 0x1C: FileIO_ReadFile: aligned allocation size; nonzero means free buffer
    void* readCopyDestination;   // 0x20: FileIO_PollAsyncTransferResult: memcpy destination after buffered read
    unsigned int requestedBytes; // 0x24: FileIO_PollAsyncTransferResult: memcpy byte count
};

#define OFF(f, o) typedef char FileHandle_off_##f[((int)&((FileHandle*)0)->f == (o)) ? 1 : -1]
OFF(handle, 0x0);
OFF(device, 0x4);
OFF(flags, 0x14);
OFF(transferBuffer, 0x18);
OFF(allocatedBytes, 0x1C);
OFF(readCopyDestination, 0x20);
OFF(requestedBytes, 0x24);
#undef OFF

#endif
