#ifndef CLASSES_CSHAREDFILE_H
#define CLASSES_CSHAREDFILE_H

#include "game/types.h"

// Base of the shared file objects: a vtable, the loaded buffer (also the sync
// file's buffer), a reference count and the path string.
class CSharedFile {
public:
    void Init();
    void* Destroy(int flag);

    void AddRef();
    int ReleaseRefIsZero();
    void* GetPath();
    int AllocateBuffer(void* encodedByteCount);
    int SetPath(const char* str);
    static int FreeBuffer(void* p);
    static void* GetLoadedBuffer(void* p);

    void* vtable;           // 0x0
    void* buffer;           // 0x4
    int referenceCount;     // 0x8
    unsigned int pathFlags; // 0xc
    int pathLength;         // 0x10
    void* pathData;         // 0x14
};

#define CSHAREDFILE_OFF(f, o) typedef char CSharedFile_off_##f[((int)&((CSharedFile*)0)->f == (o)) ? 1 : -1]
CSHAREDFILE_OFF(buffer, 0x4);
CSHAREDFILE_OFF(referenceCount, 0x8);
CSHAREDFILE_OFF(pathFlags, 0xc);
CSHAREDFILE_OFF(pathLength, 0x10);
CSHAREDFILE_OFF(pathData, 0x14);
#undef CSHAREDFILE_OFF

#endif // CLASSES_CSHAREDFILE_H
