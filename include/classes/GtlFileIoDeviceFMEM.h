// gtl::CFileIoDeviceFMEM: the file device whose operations are forwarded to the NAND file API.
//
// The device holds the path prefix and a cached copy of the NAND current directory. Its
// virtual slots are the entries of the original vtable (taken from the game data), so no
// member is declared virtual and `vtbl` is spelled out as the first member.
#ifndef CLASSES_GTLFILEIODEVICEFMEM_H
#define CLASSES_GTLFILEIODEVICEFMEM_H

#include "types.h"
#include "classes/MWStringStorage.h"

struct NANDFileInfo;
struct FmemFindData;
// Completion for a directory search started by CFileIoDeviceFMEM::FindFirstFile.
typedef void* (*FmemFindCompletion)(void* status, FmemFindData*, void* handle, void* context);

namespace gtl {

class CFileIoDeviceFMEM {
public:
    // Vtable slots that are fixed results or forward to one NAND call. The slot numbers
    // are kept in the names; the operation of each stub is not identified.

    // Fixed zero results for slots 9, 10, 11, 13, 14, 16, 29 and a fixed 32 for slot 24.
    int vfunc_9();
    int vfunc_10();
    int vfunc_11();
    int vfunc_13();
    int vfunc_14();
    int vfunc_16();
    static int vfunc_24();
    int vfunc_29();

    // Report the device as available.
    int IsAvailable();

    // NAND completion callback: forwards the SDK status and the stored operation words to the
    // callback object held in the command block's user data.
    static void DispatchNandCallback(void* nandResult, void* commandBlock);

    // Release a file handle (buffer, optional heap text, then the handle itself).
    int ReleaseFileHandle(void* handle);
    // Change the NAND current directory and return the NAND status.
    int ChangeDirectory(void* directoryPath);
    // Copy the NAND current directory into a buffer of at least 64 bytes.
    int GetCurrentDirectory(void* directoryBuffer, u32 bufferBytes);
    // Return the file handle's stored operation result.
    int GetOperationResult(void* handle);
    // Report whether the file handle's operation state is nonzero.
    int QueryHasOperationState(void* fileHandle, int* outState);

    // Open a NAND file under the device path prefix; returns the owned handle or -1.
    int OpenFile(const char* devicePath, int flags, int unusedArgument, int createModeOrHandle);
    // Seek an open NAND file; returns the new position, or -1 on failure.
    int Seek(NANDFileInfo* fileInfo, int byteOffset, unsigned int* positionHigh, unsigned int origin);
    // Zero a stat result and fill in the file size for a valid handle.
    int GetFileStat(unsigned int fileHandle, void* outputStat);
    // Queue an asynchronous NAND read, write or seek. Each completes through DispatchNandCallback.
    int ReadAsync(void* device, u32 destinationAddress, u32 size, int p4, int p5, void* cbArg);
    int WriteAsync(void* device, void* srcBuf, u32 size, int unusedArgument, int completionCallback);
    int SeekAsync(void* fileHandle, u32 byteOffset, u32 origin, int unused, int callbackAddress);
    // Delete a NAND file now, or queue the deletion when a completion callback is given.
    int DeleteFile(char* path, int completionCallback, int callbackContext);
    // Move a NAND file into a destination directory, now or queued.
    int MoveToDirectory(const char* sourcePath, const char* destinationPath);
    int MoveFileToDirectoryAsync(const char* sourcePath, const char* destinationPath, int callbackArgument);
    // Create a directory on NAND, synchronously or as a queued operation.
    int CreateDirectory(const char* path, int completionAddress, int userArgument);
    // Remove a directory on NAND, synchronously or as a queued operation.
    int RemoveDirectory(const char* path, int completionCallback, void* callbackContext);
    // Query the length of an open NAND file, synchronously or as a queued operation.
    int GetFileSize(unsigned int fileHandle, int* outSyncWord, int completionAddress);
    // Open or create a NAND file asynchronously.
    int CreateOrOpenFileAsync(const char* devicePath, int flags, int unusedA, int mode, int unusedB, int unusedC,
                              int callbackArgument);
    // Report free space in the cached directory and the device's fixed capacity, optionally
    // through a queued NAND query.
    int QueryDiskSpace(void* unusedArgument, void* directoryFree, void* deviceCapacity, void* nandStat, int callback,
                       int userData);
    // Advance a directory search handle to its next matching name and fill the entry.
    int ReadNextDirectoryEntry(void* directoryHandle, void* outEntry);
    // Start a directory search: synchronously into result, or as a queued NAND operation.
    void* FindFirstFile(const char* path, FmemFindData* result, FmemFindCompletion callback, void* context);
    // Initialize path strings and the cached NAND directory in caller-owned storage; returns this.
    void* Init();
    // Release the owned string buffers, then free the device when flags > 0; returns this.
    void* Destroy(int flags);
    // Allocate raw device storage through the installed callback; construction is separate.
    static void* AllocateStorage(u32 size);
    // Close a NAND file handle and release its allocation.
    int CloseFile(void* handle);
    // Read synchronously into a borrowed destination; returns the NAND byte count in bytesRead.
    int Read(void* fileHandle, void* destination, void* byteCountWord, int* bytesRead);
    // Write synchronously from a borrowed buffer.
    int Write(void* handle, void* buffer, void* byteCount, int* bytesWrittenOut);

    void* vtbl;                             // 0x00
    MWStringStorage pathString;             // 0x04
    MWStringStorage cachedCurrentDirectory; // 0x10
};

} // namespace gtl

#define GTL_CFILEIODEVICEFMEM_OFF(f, o) \
    typedef char gtl_CFileIoDeviceFMEM_off_##f[((int)&((gtl::CFileIoDeviceFMEM*)0)->f == (o)) ? 1 : -1]
GTL_CFILEIODEVICEFMEM_OFF(vtbl, 0x0);
GTL_CFILEIODEVICEFMEM_OFF(pathString, 0x4);
GTL_CFILEIODEVICEFMEM_OFF(cachedCurrentDirectory, 0x10);
#undef GTL_CFILEIODEVICEFMEM_OFF

#endif
