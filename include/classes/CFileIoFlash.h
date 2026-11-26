#ifndef CLASSES_CFILEIOFLASH_H
#define CLASSES_CFILEIOFLASH_H

#include "game/types.h"

class CFileIoFlash {
public:
    // Shared instance: AcquireInstance returns the existing one, CreateInstance sets up a new one with its arguments.
    static void* AcquireInstance();
    static void* CreateInstance(void* a, void* b);

    // Operations that pick the synchronous or completion-callback form by the debug option.
    int ReadAlignedBuffer();
    void DispatchDeletePath();
    int Open(void* openMode);
    int StartPaddedWrite(void* sourceBuffer);
    void DispatchSaveAreaCheck(void* saveArea, void* checkOptions);
    int DispatchQualifiedPathMove(void* destinationPath, void* sourcePath);
    int SetPathAndAllocateBuffer(const char* path, void* openOptions);

    // Save-area check and qualified-path move, run with an optional completion callback.
    int CheckSaveArea(void* query, void* queryResult, int completionCallback);
    int MoveQualifiedPath(void* destinationPath, const char* sourcePath, int completionCallback);

    // Completion callbacks: publish the result on the shared instance, then clear busy.
    static void CompleteWithStatus(int status);
    static void OnReadWriteComplete(void* status, int transferredBytes);
    static void OnOpenComplete(int status, int completionFlag);
    static void OnSaveAreaCheckComplete(void* status, int saveAreaFlags);
    static int OnQualifiedPathMoveComplete();

    // Busy flag and status queries.
    int ClearBusy();
    int HasNonzeroStatus();
    int QueryDiskFreeSpaceDefault();

    // Virtual slots of the original vtable; their purpose is not established.
    int IsStatusC353();
    int vfunc_9();
    int IsStatusC361();
    int IsStatusC354();
    int vfunc_14();

    // Deleting teardown: clears the shared instance and the file-I/O base, deletes the object when flag > 0.
    void* Destroy(int flag);

    // Static initialiser: the flash file helper's two-pi cache.
    static void InitAngleCache();

    void* vtbl;        // 0x0: original vtable pointer
    char pad_004[0x4]; // 0x4
    int statusCode;    // 0x8
};

#define CFILEIOFLASH_OFF(f, o) typedef char CFileIoFlash_off_##f[((int)&((CFileIoFlash*)0)->f == (o)) ? 1 : -1]
CFILEIOFLASH_OFF(statusCode, 0x8);
#undef CFILEIOFLASH_OFF

#endif // CLASSES_CFILEIOFLASH_H
