// gtl::CFileIoDeviceDVD: the DVD file device. It opens, reads, seeks, cancels and searches
// DVD entries through the SDK and publishes each result as the file layer's last error.
//
// The device holds only its root path string. Its virtual slots are the entries of the
// original vtable (taken from the game data), so no member is declared virtual and `vtbl`
// is spelled out as the first member. File handles passed to these methods are SDK-side
// objects, not instances of this class.
#ifndef CLASSES_GTLFILEIODEVICEDVD_H
#define CLASSES_GTLFILEIODEVICEDVD_H

#include "types.h"
#include "classes/MWStringStorage.h"

struct DVDFile;
struct DVDFileInfoLengthView;
struct DVDRequest;
struct FileInfo;
struct FindData;

namespace gtl {

class CFileIoDeviceDVD {
public:
    // Set the root path to "/" and bind the vtable, then initialize the DVD subsystem; returns this.
    void* Init();
    // Release the root path text, then free the device when deleteStorage > 0; returns this.
    void* Destroy(int deleteStorage);
    // Allocate raw device storage through the installed callback; construction is separate.
    static void* AllocateStorage(u32 size);
    // Report that the DVD device is available. No media check is made.
    int IsAvailable();

    // Open a DVD entry for the path after its device prefix; returns the owned handle or -1.
    DVDFile* OpenFile(const char* path);
    // Close a DVD file handle and free it. Returns 1 when the SDK close failed.
    int CloseFile(unsigned int fileHandle);
    // Read synchronously at a byte offset; bytesRead is written only when it is not null.
    int ReadFile(unsigned int fileHandle, void* dst, int length, int* bytesRead, int offset);
    // Seek a DVD file; clearedOutput is cleared before the seek is issued.
    int Seek(unsigned int file, unsigned int offset, int* clearedOutput);
    // Return the low 32 bits of the file length, or -1 for an invalid handle.
    int GetFileSize(DVDFileInfoLengthView* file, unsigned int* sizeHigh);
    // Fill a 0x34-byte file info record with the file size, through the size slot of the vtable.
    int GetFileInfo(unsigned int fileHandle, FileInfo* info);
    // Cancel a DVD command synchronously. The handle stays open.
    int CancelHandle(unsigned int handle);
    // Open a directory search and return its first match, or -1 with the last error set.
    int FindFirst(const char* path, FindData* out);
    // Advance a search cursor until its wildcard pattern matches an entry.
    int FindNextFile(DVDFile* search, FindData* outEntry);
    // Close a directory search handle, release its name storage and free it.
    int CloseDirectorySearch(void* fileHandle);

    // Report the required transfer alignment in bytes.
    int GetTransferAlignment();
    // Chunked read completion. The SDK calls this static with (result, request).
    static void OnReadChunkComplete(int result, DVDRequest* request);
    // Seek completion. The SDK calls this static with (result, request).
    static void OnSeekComplete(int result, DVDRequest* request);
    // Open a DVD entry and invoke the callback synchronously on success, with (0, 0, file, context).
    DVDFile* OpenWithSynchronousCallback(const char* path, int unused3, int unused4, int unused5, int unused6,
                                         int unused7, void (*callback)(int, int, DVDFile*, void*), void* context);
    // Start an asynchronous read in chunks of at most 60 KiB; returns 1 when accepted.
    int StartChunkedAsyncRead(void* request, void* destination, int length, int offset, unsigned int priorityCode,
                              int completionArg, int completionArg2);
    // Submit a seek whose completion callback is recorded after successful submission; returns 1 when accepted.
    int SubmitSeekWithCallback(unsigned int file, int offset, int unused, unsigned int priorityArgument, int callback,
                               int userData);
    // Request asynchronous cancellation with no completion callback.
    int CancelAsyncRequest(const char* fileHandle);
    // Poll a DVD command block. outComplete is set to 1 only when the command has ended.
    int PollCommandCompletion(unsigned int handle, int* outComplete);
    // Return the 32-bit query word stored in a DVD file handle, or 0 for an invalid handle.
    unsigned int vfunc_31(void* file);

    void* vtbl;               // 0x00
    MWStringStorage rootPath; // 0x04: device root path, "/" after Init
};

} // namespace gtl

#define GTL_CFILEIODEVICEDVD_OFF(f, o) \
    typedef char gtl_CFileIoDeviceDVD_off_##f[((int)&((gtl::CFileIoDeviceDVD*)0)->f == (o)) ? 1 : -1]
GTL_CFILEIODEVICEDVD_OFF(vtbl, 0x0);
GTL_CFILEIODEVICEDVD_OFF(rootPath, 0x4);
#undef GTL_CFILEIODEVICEDVD_OFF
typedef char gtl_CFileIoDeviceDVD_size[(sizeof(gtl::CFileIoDeviceDVD) == 0x10) ? 1 : -1];

#endif
