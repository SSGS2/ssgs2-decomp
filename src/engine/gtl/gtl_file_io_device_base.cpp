// Base implementation of the GTL file I/O device. Every operation here rejects its request with a
// fixed failure value and leaves the buffers and handles alone, except IsAvailable, which reports
// the device as present.
//
// Devices that support an operation replace the matching base method.

#include "classes/gtl_CFileIoDevice.h"

// Results of the base implementations. Open, size and seek report failure with all bits set; the operations
// report failure with zero.
enum BaseFileDeviceResults {
    kBaseOperationUnavailable = 0,
    kBaseInvalidHandleWord = -1,
    kBaseQueryFailedWord = -1,
    kBaseDeviceAvailable = 1,
    kBaseTransferAlignmentBytes = 1,
    kUnresolvedBaseZero = 0
};

// Base implementations never touch buffers, handles, callbacks or the last error. A failed read, write or query
// leaves its output unchanged, and no implementation closes or frees a handle.

// Base slot: returns zero.
int gtl::CFileIoDevice::vfunc_27() {
    return kUnresolvedBaseZero;
}

// Base slot: returns zero.
int gtl::CFileIoDevice::RemoveDirectory() {
    return kUnresolvedBaseZero;
}

// Base slot: returns zero.
int gtl::CFileIoDevice::CreateDirectory() {
    return kUnresolvedBaseZero;
}

// Base slot: returns zero.
int gtl::CFileIoDevice::QueryDiskSpace() {
    return kUnresolvedBaseZero;
}

// Reject the move; the paths and completion argument are not kept.
int gtl::CFileIoDevice::MoveFileToDirectoryAsync(const char* sourcePath, const char* destinationPath,
                                                 int completionArgument) {
    return kBaseOperationUnavailable;
}

// Base slot: returns zero.
int gtl::CFileIoDevice::vfunc_16() {
    return kUnresolvedBaseZero;
}

// Base slot: returns zero.
int gtl::CFileIoDevice::MoveToDirectory() {
    return kUnresolvedBaseZero;
}

// Base slot: returns zero.
int gtl::CFileIoDevice::vfunc_14() {
    return kUnresolvedBaseZero;
}

// Base slot: returns zero.
int gtl::CFileIoDevice::vfunc_13() {
    return kUnresolvedBaseZero;
}

// Base slot: returns zero.
int gtl::CFileIoDevice::DeleteFile() {
    return kUnresolvedBaseZero;
}

// Base slot: returns zero.
int gtl::CFileIoDevice::vfunc_11() {
    return kUnresolvedBaseZero;
}

// Base slot: returns zero.
int gtl::CFileIoDevice::FlushStub() {
    return kUnresolvedBaseZero;
}

// Reject the write; bytesWritten is left unchanged.
int gtl::CFileIoDevice::Write(void* handle, const void* source, unsigned int byteCount, unsigned int* bytesWritten) {
    return kBaseOperationUnavailable;
}

// Base slot: returns zero.
int gtl::CFileIoDevice::vfunc_31() {
    return kUnresolvedBaseZero;
}

// Base slot: returns zero.
int gtl::CFileIoDevice::vfunc_30() {
    return kUnresolvedBaseZero;
}

// Base slot: returns zero.
int gtl::CFileIoDevice::vfunc_29() {
    return kUnresolvedBaseZero;
}

// Base slot: returns zero.
int gtl::CFileIoDevice::vfunc_28() {
    return kUnresolvedBaseZero;
}

// Reject the asynchronous read; the request and buffer are left unchanged and no callback is invoked.
int gtl::CFileIoDevice::ReadAsync(void* request, void* destination, int length, int offset, unsigned int priority,
                                  int callbackWord, int userWord) {
    return kBaseOperationUnavailable;
}

// Reject the open with the invalid-handle result; no callback is invoked.
int gtl::CFileIoDevice::OpenWithCompletion(const char* path) {
    return kBaseInvalidHandleWord;
}

// Byte alignment of one: the base device adds no transfer alignment.
int gtl::CFileIoDevice::GetTransferAlignment() {
    return kBaseTransferAlignmentBytes;
}

// Reject the handle release; the handle is not freed.
int gtl::CFileIoDevice::vfunc_21(void* handle) {
    return kBaseOperationUnavailable;
}

// Reject the directory search step; the cursor and outEntry are left unchanged.
int gtl::CFileIoDevice::FindNextFile(void* searchHandle, void* outEntry) {
    return kBaseOperationUnavailable;
}

// Reject the directory search with the invalid-handle result; result and callback are not used.
int gtl::CFileIoDevice::FindFirstFile(const char* path, void* result, void* callback, void* context) {
    return kBaseInvalidHandleWord;
}

// Reject the cancel; the handle is left unchanged.
int gtl::CFileIoDevice::Cancel(void* handle) {
    return kBaseOperationUnavailable;
}

// Reject the file info query; fileInfo is left unchanged.
int gtl::CFileIoDevice::GetFileInfo(void* handle, void* fileInfo) {
    return kBaseOperationUnavailable;
}

// Reject the size query with all bits set; sizeHigh is left unchanged.
int gtl::CFileIoDevice::GetFileSize(void* handle, unsigned int* sizeHigh) {
    return kBaseQueryFailedWord;
}

// Reject the seek with all bits set; offsetHigh is left unchanged.
int gtl::CFileIoDevice::Seek(void* handle, unsigned int offsetLow, unsigned int* offsetHigh) {
    return kBaseQueryFailedWord;
}

// Reject the read; destination and bytesRead are left unchanged.
int gtl::CFileIoDevice::Read(void* handle, void* destination, unsigned int byteCount, unsigned int* bytesRead) {
    return kBaseOperationUnavailable;
}

// Reject the close; the handle is not released.
int gtl::CFileIoDevice::Close(void* handle) {
    return kBaseOperationUnavailable;
}

// Reject the open with the invalid-handle result.
int gtl::CFileIoDevice::Open(const char* path) {
    return kBaseInvalidHandleWord;
}

// The base device is always available.
int gtl::CFileIoDevice::IsAvailable() {
    return kBaseDeviceAvailable;
}
