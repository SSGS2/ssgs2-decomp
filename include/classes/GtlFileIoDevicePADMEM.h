// gtl::CFileIoDevicePADMEM: the file device for a PAD memory card channel.
//
// The device holds the path prefix and the card channel index. Its virtual slots are the
// entries of the original vtable (taken from the game data), so no member is declared
// virtual and `vtbl` is spelled out as the first member.
#ifndef CLASSES_GTLFILEIODEVICEPADMEM_H
#define CLASSES_GTLFILEIODEVICEPADMEM_H

#include "types.h"
#include "classes/MWStringStorage.h"

namespace gtl {

class CFileIoDevicePADMEM {
public:
    // Allocate raw device storage through the installed callback; construction is separate.
    static void* AllocateStorage(u32 size);
    // Set up the path prefix and the card channel on storage the caller owns; returns this.
    void* Init(int channel);
    // Free the owned path string, and the device itself when flags > 0; returns this.
    void* Destroy(int flags);

    // Open a handle for the part of path after the first slash; returns the handle or null.
    void* OpenHandle(const char* path);
    // Open a path through the device's open slot, then run the completion callback synchronously.
    void* OpenWithCallback(const char* path, int flags, int r6, int mode, int r8, int r9, int callbackAddress,
                           void* data);
    // Release a PAD memory handle through its deleting destructor.
    int CloseHandle(void* handle);

    // Read synchronously into buffer; the byte count is capped at one transfer block.
    int ReadBytes(void* handle, void* buffer, int requestedBytes, void* bytesRead, int deviceOffset);
    // Transfer at most one block synchronously; the caller loops for the rest.
    int TransferOneBlock(unsigned int fileHandle, void* buffer, unsigned int requestedBytes, int* transferredBytes);
    // Submit an asynchronous read; the completion callback runs when the transfer finishes.
    int ReadAsync(void* handle, void* buffer, unsigned int byteCount, unsigned int deviceOffset, int unused,
                  int callbackAddress, void* callbackData);
    // Submit an asynchronous transfer of one block; the completion callback runs when it finishes.
    int BeginAsyncTransfer(void* fileHandle, void* buffer, unsigned int size, int unused, int completionCallback,
                           void* callbackData);

    // Fill the last-write fields of a file-info record from the card's stored metadata.
    int QueryHandleFileInfo(int handle, void* output);
    // Fill a file-info record with the stored title and the metadata words.
    int QueryStoredFileInfo(int unused, void* output);
    // Check a handle word against the invalid sentinel.
    int IsHandleValid(unsigned int handleWord);
    // Return the accepted transfer byte count of a handle.
    int GetTransferBytes(void* handle);

    // Virtual slots with fixed results or a single check. The slot numbers are kept in the
    // names; the purpose of each is not identified.
    int vfunc_9();
    int vfunc_20();
    int vfunc_24();
    int vfunc_30(void* handle, int* hasNonzeroStorageWord);

    void* vtbl;            // 0x00
    MWStringStorage path;  // 0x04
    void* controllerIndex; // 0x10
};

} // namespace gtl

#define GTL_CFILEIODEVICEPADMEM_OFF(f, o) \
    typedef char gtl_CFileIoDevicePADMEM_off_##f[((int)&((gtl::CFileIoDevicePADMEM*)0)->f == (o)) ? 1 : -1]
GTL_CFILEIODEVICEPADMEM_OFF(vtbl, 0x0);
GTL_CFILEIODEVICEPADMEM_OFF(path, 0x4);
GTL_CFILEIODEVICEPADMEM_OFF(controllerIndex, 0x10);
#undef GTL_CFILEIODEVICEPADMEM_OFF

#endif
