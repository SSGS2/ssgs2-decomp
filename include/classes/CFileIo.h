// CFileIo: the common file object behind the pad and flash save devices.
//
// It owns at most one open file handle, a status word, a path and a device
// prefix, and the buffer that a read or write transfers through. The vtable
// still comes from the original data, so it is spelled out as a member.
#ifndef CLASSES_CFILEIO_H
#define CLASSES_CFILEIO_H

#include "game/types.h"

class CFileIo {
public:
    // Lifetime and shared state.
    ~CFileIo();
    void InitMembers();
    int AddRef();
    int ReleaseRef();

    // Handle and status.
    int CloseHandle();
    int IsBusy();
    int GetBuffer();
    int GetStatusCode();
    int SetStatusCode(int status);
    int SetSaveAreaStatusFlags(int saveAreaFlags);
    int GetSaveAreaStatusFlags();
    int GetTransferredBytes();
    int SetTransferredBytes(int transferred);

    // Path and transfer buffer.
    int SetPathAndAllocateBuffer(const char* fullPath, int requestedBufferSize);
    int Open(int completionCallback, void* openDisposition);
    int DeletePath(int completionCallback);
    int QueryDiskFreeSpace(int completionCallback);
    int WritePaddedBuffer(void* source, int completionCallback);

    void* vtable;                    // 0x00
    void* ioBuffer;                  // 0x04
    int statusCode;                  // 0x08
    int fileHandle;                  // 0x0c
    int referenceCount;              // 0x10
    unsigned char busy;              // 0x14
    char pad_015[0x3];               // 0x15
    unsigned int pathFlags;          // 0x18  small-string flags and length
    unsigned int pathLength;         // 0x1c  heap length
    void* pathData;                  // 0x20  heap text
    unsigned int devicePrefixFlags;  // 0x24
    unsigned int devicePrefixLength; // 0x28
    void* devicePrefixData;          // 0x2c
    int requestedBufferSize;         // 0x30
    char pad_034[0x4];               // 0x34
    unsigned int deviceInfo[6];      // 0x38  cleared when a path is installed
    int saveAreaStatusFlags;         // 0x50
    int transferredBytes;            // 0x54
};

#define CFILEIO_OFF(f, o) typedef char CFileIo_off_##f[((int)&((CFileIo*)0)->f == (o)) ? 1 : -1]
CFILEIO_OFF(ioBuffer, 0x4);
CFILEIO_OFF(statusCode, 0x8);
CFILEIO_OFF(fileHandle, 0xc);
CFILEIO_OFF(referenceCount, 0x10);
CFILEIO_OFF(busy, 0x14);
CFILEIO_OFF(pathFlags, 0x18);
CFILEIO_OFF(pathLength, 0x1c);
CFILEIO_OFF(pathData, 0x20);
CFILEIO_OFF(devicePrefixFlags, 0x24);
CFILEIO_OFF(devicePrefixLength, 0x28);
CFILEIO_OFF(devicePrefixData, 0x2c);
CFILEIO_OFF(requestedBufferSize, 0x30);
CFILEIO_OFF(deviceInfo, 0x38);
CFILEIO_OFF(saveAreaStatusFlags, 0x50);
CFILEIO_OFF(transferredBytes, 0x54);
#undef CFILEIO_OFF
typedef char CFileIo_size[(sizeof(CFileIo) == 0x58) ? 1 : -1];

#endif // CLASSES_CFILEIO_H
