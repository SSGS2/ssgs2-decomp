// Translation unit 0x80413C2C-0x80413D24 (31 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Base-device fallbacks never touch buffers, handles, callbacks, or last-error.
// A failed read/write leaves the byte-count output UNCHANGED; a failed query also
// leaves its output unchanged. Callers must check the returned word before use.
// No fallback closes/frees a borrowed handle or takes ownership of a path/buffer.
// Slot numbers are virtual indices. Contracts below are cross-checked against
// concrete DVD/HIO/FMEM implementations; unresolved slots retain raw return names.
// For unresolved signatures, only the implicit device word is named and trailing
// ABI words are omitted because their contracts remain unverified. These leaf
// definitions are not a complete interface header; do not use them as prototypes.
// Availability and alignment return 1 for different reasons. Open/size/seek use
// all-bits-set failure; operations return zero. Keep these distinct result domains.
enum BaseFileDeviceResults {
    kBaseOperationUnavailable = 0,
    kBaseInvalidHandleWord = -1,
    kBaseQueryFailedWord = -1,
    kBaseDeviceAvailable = 1,
    kBaseTransferAlignmentBytes = 1,
    kUnresolvedBaseZero = 0
};

// Return the unresolved base result for virtual slot 27; ignore all incoming ABI words.
int gtl_CFileIoDevice_vfunc_27(void* /*device*/) {
	return kUnresolvedBaseZero;
}

// Return the unresolved base result for virtual slot 23; ignore all incoming ABI words.
int gtl_CFileIoDevice_RemoveDirectory(void* /*device*/) {
	return kUnresolvedBaseZero;
}

// Return the unresolved base result for virtual slot 22; ignore all incoming ABI words.
int gtl_CFileIoDevice_CreateDirectory(void* /*device*/) {
	return kUnresolvedBaseZero;
}

// Return the unresolved base result for virtual slot 18; ignore all incoming ABI words.
int gtl_CFileIoDevice_QueryDiskSpace(void* /*device*/) {
	return kUnresolvedBaseZero;
}

// Reject the move request; retain neither borrowed path nor completion argument.
int gtl_CFileIoDevice_MoveFileToDirectoryAsync(void* /*device*/, const char* /*sourcePath*/, const char* /*destinationPath*/, int /*completionArgument*/) {
	return kBaseOperationUnavailable;
}

// Return the unresolved base result for virtual slot 16; ignore all incoming ABI words.
int gtl_CFileIoDevice_vfunc_16(void* /*device*/) {
	return kUnresolvedBaseZero;
}

// Return the unresolved base result for virtual slot 15; ignore all incoming ABI words.
int gtl_CFileIoDevice_MoveToDirectory(void* /*device*/) {
	return kUnresolvedBaseZero;
}

// Return the unresolved base result for virtual slot 14; ignore all incoming ABI words.
int gtl_CFileIoDevice_vfunc_14(void* /*device*/) {
	return kUnresolvedBaseZero;
}

// Return the unresolved base result for virtual slot 13; ignore all incoming ABI words.
int gtl_CFileIoDevice_vfunc_13(void* /*device*/) {
	return kUnresolvedBaseZero;
}

// Return the unresolved base result for virtual slot 12; ignore all incoming ABI words.
int gtl_CFileIoDevice_DeleteFile(void* /*device*/) {
	return kUnresolvedBaseZero;
}

// Return the unresolved base result for virtual slot 11; ignore all incoming ABI words.
int gtl_CFileIoDevice_vfunc_11(void* /*device*/) {
	return kUnresolvedBaseZero;
}

// Return the unresolved base result for virtual slot 10; ignore all incoming ABI words.
int gtl_CFileIoDevice_FlushStub(void* /*device*/) {
	return kUnresolvedBaseZero;
}

// Reject a write; leave the optional bytes-written output untouched.
int gtl_CFileIoDevice_Write(void* /*device*/, void* /*handle*/, const void* /*source*/, unsigned int /*byteCount*/, unsigned int* /*bytesWritten*/) {
	return kBaseOperationUnavailable;
}

// Return the unresolved base result for virtual slot 31; ignore all incoming ABI words.
int gtl_CFileIoDevice_vfunc_31(void* /*device*/) {
	return kUnresolvedBaseZero;
}

// Return the unresolved base result for virtual slot 30; ignore all incoming ABI words.
int gtl_CFileIoDevice_vfunc_30(void* /*device*/) {
	return kUnresolvedBaseZero;
}

// Return the unresolved base result for virtual slot 29; ignore all incoming ABI words.
int gtl_CFileIoDevice_vfunc_29(void* /*device*/) {
	return kUnresolvedBaseZero;
}

// Return the unresolved base result for virtual slot 28; ignore all incoming ABI words.
int gtl_CFileIoDevice_vfunc_28(void* /*device*/) {
	return kUnresolvedBaseZero;
}

// Reject asynchronous read submission; leave the request/buffer unchanged and invoke no callback.
int gtl_CFileIoDevice_ReadAsync(void* /*device*/, void* /*request*/, void* /*destination*/, int /*length*/, int /*offset*/, unsigned int /*priority*/, int /*callbackWord*/, int /*userWord*/) {
	return kBaseOperationUnavailable;
}

// Reject callback-based open/create with the invalid-handle word; invoke no callback.
int gtl_CFileIoDevice_OpenWithCompletion(void* /*device*/, const char* /*path*/) {
	return kBaseInvalidHandleWord;
}

// Report byte alignment of one: the base device imposes no extra transfer alignment.
int gtl_CFileIoDevice_GetTransferAlignment(void* /*device*/) {
	return kBaseTransferAlignmentBytes;
}

// Reject the secondary handle-close operation without freeing the handle.
int gtl_CFileIoDevice_vfunc_21(void* /*device*/, void* /*handle*/) {
	return kBaseOperationUnavailable;
}

// Reject advancing a directory search; leave its cursor and output entry untouched.
int gtl_CFileIoDevice_FindNextFile(void* /*device*/, void* /*searchHandle*/, void* /*outEntry*/) {
	return kBaseOperationUnavailable;
}

// Reject directory search creation; leave the result and completion arguments untouched.
int gtl_CFileIoDevice_FindFirstFile(void* /*device*/, const char* /*path*/, void* /*result*/, void* /*callback*/, void* /*context*/) {
	return kBaseInvalidHandleWord;
}

// Reject cancellation without changing or releasing the borrowed handle.
int gtl_CFileIoDevice_Cancel(void* /*device*/, void* /*handle*/) {
	return kBaseOperationUnavailable;
}

// Reject file-info lookup without writing the caller's result structure.
int gtl_CFileIoDevice_GetFileInfo(void* /*device*/, void* /*handle*/, void* /*fileInfo*/) {
	return kBaseOperationUnavailable;
}

// Reject a file-size query with all bits set; leave the high-size output untouched.
int gtl_CFileIoDevice_GetFileSize(void* /*device*/, void* /*handle*/, unsigned int* /*sizeHigh*/) {
	return kBaseQueryFailedWord;
}

// Reject a seek; leave the optional high-offset output untouched.
int gtl_CFileIoDevice_Seek(void* /*device*/, void* /*handle*/, unsigned int /*offsetLow*/, unsigned int* /*offsetHigh*/) {
	return kBaseQueryFailedWord;
}

// Reject a read; leave destination and the optional byte-count output untouched.
int gtl_CFileIoDevice_Read(void* /*device*/, void* /*handle*/, void* /*destination*/, unsigned int /*byteCount*/, unsigned int* /*bytesRead*/) {
	return kBaseOperationUnavailable;
}

// Reject file close without releasing or changing the borrowed handle.
int gtl_CFileIoDevice_Close(void* /*device*/, void* /*handle*/) {
	return kBaseOperationUnavailable;
}

// Reject file open with the invalid-handle word; path and mode words are borrowed.
int gtl_CFileIoDevice_Open(void* /*device*/, const char* /*path*/) {
	return kBaseInvalidHandleWord;
}

// Report base-device availability without inspecting any media.
int gtl_CFileIoDevice_IsAvailable(void* /*device*/) {
	return kBaseDeviceAvailable;
}

} // extern "C"
