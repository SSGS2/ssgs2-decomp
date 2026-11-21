// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFILEIO_H
#define CLASSES_CFILEIO_H

#include "game/types.h"

class CFileIo {
public:
    void* vtable;                        // 0x0: 2 uses in CFileIo_InitMembers, CFileIo_dtor
    void* ioBuffer;                        // 0x4: 6 uses in CFileIo_SetPathAndAllocateBuffer, CFileIo_dtor, CFileIo_InitMembers
    int statusCode;                          // 0x8: 1 use in CFileIo_InitMembers
    int fileHandle;                          // 0xc: 3 uses in CFileIo_dtor, CFileIo_InitMembers
    int referenceCount;                         // 0x10: 1 use in CFileIo_InitMembers
    unsigned char busy;               // 0x14: 1 use in CFileIo_InitMembers
    char pad_015[0x3];                  // 0x15
    unsigned int pathFlags;                // 0x18: 8 uses in CFileIo_SetPathAndAllocateBuffer, CFileIo_InitMembers, CFileIo_dtor (CONFLICT sizes [1, 4])
    unsigned int pathLength;                // 0x1c: 4 uses in CFileIo_SetPathAndAllocateBuffer, CFileIo_InitMembers
    void* pathData;                       // 0x20: 4 uses in CFileIo_SetPathAndAllocateBuffer, CFileIo_InitMembers, CFileIo_dtor
    unsigned int devicePrefixFlags;                // 0x24: 5 uses in CFileIo_SetPathAndAllocateBuffer, CFileIo_InitMembers, CFileIo_dtor (CONFLICT sizes [1, 4])
    unsigned int devicePrefixLength;                // 0x28: 2 uses in CFileIo_InitMembers, CFileIo_SetPathAndAllocateBuffer
    int devicePrefixData;                         // 0x2c: 3 uses in CFileIo_InitMembers, CFileIo_dtor, CFileIo_SetPathAndAllocateBuffer
    int requestedBufferSize;                         // 0x30: 3 uses in CFileIo_SetPathAndAllocateBuffer, CFileIo_InitMembers
    char pad_034[0x1c];                 // 0x34
    int saveAreaStatusFlags;                         // 0x50: 1 use in CFileIo_InitMembers
    int transferredBytes;                         // 0x54: 1 use in CFileIo_InitMembers

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x18, 0x19, 0x24, 0x38, 0x40, 0x48
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
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
CFILEIO_OFF(saveAreaStatusFlags, 0x50);
CFILEIO_OFF(transferredBytes, 0x54);
#undef CFILEIO_OFF

#endif  // CLASSES_CFILEIO_H
