#ifndef CLASSES_GTL_CFILEIODEVICE_H
#define CLASSES_GTL_CFILEIODEVICE_H

#include "classes/MWStringStorage.h"

namespace gtl {

// Base file device: holds the device root path and the default implementations of the device slots.
class CFileIoDevice {
public:
    // Root path.
    // Copy the root path with its terminator; returns 0 without writing when dest is too small, else 1.
    int CopyRootPath(char* dest, unsigned int capacityBytes);
    // Replace the root path with a copy of text.
    void SetRootPath(const char* text);

    // Default slots. These do nothing and return the base result.
    int vfunc_27();
    int RemoveDirectory();
    int CreateDirectory();
    int QueryDiskSpace();
    int MoveFileToDirectoryAsync(const char* sourcePath, const char* destinationPath, int completionArgument);
    int vfunc_16();
    int MoveToDirectory();
    int vfunc_14();
    int vfunc_13();
    int DeleteFile();
    int vfunc_11();
    int FlushStub();
    int Write(void* handle, const void* source, unsigned int byteCount, unsigned int* bytesWritten);
    int vfunc_31();
    int vfunc_30();
    int vfunc_29();
    int vfunc_28();
    int ReadAsync(void* request, void* destination, int length, int offset, unsigned int priority, int callbackWord,
                  int userWord);
    int OpenWithCompletion(const char* path);
    int GetTransferAlignment();
    int vfunc_21(void* handle);
    int FindNextFile(void* searchHandle, void* outEntry);
    int FindFirstFile(const char* path, void* result, void* callback, void* context);
    int Cancel(void* handle);
    int GetFileInfo(void* handle, void* fileInfo);
    int GetFileSize(void* handle, unsigned int* sizeHigh);
    int Seek(void* handle, unsigned int offsetLow, unsigned int* offsetHigh);
    int Read(void* handle, void* destination, unsigned int byteCount, unsigned int* bytesRead);
    int Close(void* handle);
    int Open(const char* path);
    int IsAvailable();

    void* vtable;             // 0x0
    MWStringStorage rootPath; // 0x4: device root path, "/" after the DVD and PADMEM constructors
};

} // namespace gtl

#define GTL_CFILEIODEVICE_OFF(f, o) \
    typedef char gtl_CFileIoDevice_off_##f[((int)&((gtl::CFileIoDevice*)0)->f == (o)) ? 1 : -1]
GTL_CFILEIODEVICE_OFF(vtable, 0x0);
GTL_CFILEIODEVICE_OFF(rootPath, 0x4);
#undef GTL_CFILEIODEVICE_OFF

#endif // CLASSES_GTL_CFILEIODEVICE_H
