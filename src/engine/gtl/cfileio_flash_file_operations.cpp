// File operations on CFileIo and CFileIoFlash: deleting a path, querying free space, reading into an
// aligned buffer, writing a zero-padded buffer, checking the save area and moving a path.
// The owned byte buffer's destructor and the small string helper used for device paths are here too.

#include "classes/CFileIo.h"
#include "classes/CFileIoFlash.h"

// CFileIoFlash::CheckSaveArea / CFileIoFlash::MoveQualifiedPath: the buffer passed to FileIO_GetDefaultDevicePath is the
// existing Buf1::buf member at +4, also used by its RAII destructor.

extern "C" {

extern char __vt__20CSetCurrentDirectory[];
extern char s_ownedByteBufferVtable[];
void FileIO_SetCurrentDirectory(void*); // Select current device/directory; also used to restore it.
void __dl__FPv(void*);
void __dla__FPv(void*);
int FileIO_DeleteFile(void*, int, int);

enum {
    kDevicePathBufferBytes = 0x100
};
static const unsigned int kInvalidFileHandle = 0xFFFFFFFFu;

struct D1 {
    void* vt;
};
struct D2 {
    void* vt;
    char pad[4];
    void* ownedBytes;
};

} // extern "C"

// Submit deletion of the object's path and clear busy if submission fails.
int CFileIo::DeletePath(int completionCallback) {
    if (completionCallback)
        busy = 1;
    int submitted;
    if (FileIO_DeleteFile((pathFlags >> 31) == 0 ? (void*)((char*)&pathFlags + 1) : (void*)pathData, completionCallback,
                          0) == 0) {
        busy = 0;
        submitted = 0;
    } else {
        submitted = 1;
    }
    return submitted;
}

extern "C" {

int FileIO_QueryDiskFreeSpace(void*, void*, void*, void*, int, int);

} // extern "C"

// Query available, total and total-free byte counts for the object's device prefix.
// The three outputs are the 64-bit fields at +0x48, +0x38 and +0x40 of deviceInfo.
int CFileIo::QueryDiskFreeSpace(int completionCallback) {
    if (completionCallback)
        busy = 1;
    int submitted;
    if (FileIO_QueryDiskFreeSpace((devicePrefixFlags >> 31) == 0 ? (void*)((char*)&devicePrefixFlags + 1)
                                                                 : (void*)(char*)devicePrefixData,
                                  &deviceInfo[4], &deviceInfo[0], &deviceInfo[2], completionCallback, 0) == 0) {
        busy = 0;
        submitted = 0;
    } else {
        submitted = 1;
    }
    return submitted;
}

extern "C" {

unsigned FileIO_GetDeviceTransferAlignment(void*); // Returns transfer alignment in bytes.
int FileIO_ReadFile(unsigned, int, int, void*, int, int, int, int);
int FileIO_WriteFile(unsigned, int, int, void*, int, int, int);
void* memset(void*, int, unsigned long);
void Memory_Clear(void*, unsigned);
void Memory_Copy(void*, void*, unsigned);
void FileIO_GetDefaultDevicePath(int, void*);
int gtl_FileIO_CheckFmemSaveArea(void*, void*, void*, int, int);

struct Buf1 {
    void* vt;
    char buf[kDevicePathBufferBytes];
    Buf1() : vt(__vt__20CSetCurrentDirectory), buf() {}
    ~Buf1() {
        vt = __vt__20CSetCurrentDirectory;
        FileIO_SetCurrentDirectory(buf);
    }
};

// Read the file into its owned buffer, rounding the request to the device block size.
// completionCallback is zero for synchronous I/O; nonzero values are callback addresses.
int FileIo_ReadAlignedBuffer(CFileIo* self, int completionCallback) {
    unsigned blockBytes;
    int alignedBytes;
    if (self->fileHandle == kInvalidFileHandle)
        return 0;
    if (self->ioBuffer == 0)
        return 0;
    blockBytes =
        FileIO_GetDeviceTransferAlignment((self->devicePrefixFlags >> 31) == 0 ? (char*)&self->devicePrefixFlags + 1
                                                                               : (char*)(char*)self->devicePrefixData);
    if (blockBytes == 0)
        alignedBytes = 0;
    else
        alignedBytes = blockBytes * ((self->requestedBufferSize + blockBytes - 1) / blockBytes);
    if (completionCallback)
        self->busy = 1;
    int submitted;
    if (FileIO_ReadFile(self->fileHandle, (int)self->ioBuffer, alignedBytes, &self->transferredBytes, 0, -1,
                        completionCallback, 0) == 0) {
        self->busy = 0;
        submitted = 0;
    } else {
        submitted = 1;
    }
    return submitted;
}

} // extern "C"

// Zero-pad the owned buffer to the device transfer alignment before submitting the write.
int CFileIo::WritePaddedBuffer(void* source, int completionCallback) {
    unsigned blockBytes;
    int alignedBytes;
    int bufferPrepared;
    char* devicePrefix;
    if (fileHandle == kInvalidFileHandle)
        return 0;
    if (ioBuffer == 0)
        return 0;
    if (source == 0) {
        bufferPrepared = 0;
    } else if (ioBuffer == 0) {
        bufferPrepared = 0;
    } else {
        devicePrefix = (devicePrefixFlags >> 31) == 0 ? (char*)&devicePrefixFlags + 1 : (char*)devicePrefixData;
        blockBytes = FileIO_GetDeviceTransferAlignment(devicePrefix);
        if (blockBytes == 0)
            alignedBytes = 0;
        else
            alignedBytes = blockBytes * ((requestedBufferSize + blockBytes - 1) / blockBytes);
        Memory_Clear((void*)ioBuffer, alignedBytes);
        Memory_Copy((void*)ioBuffer, source, requestedBufferSize);
        bufferPrepared = 1;
    }
    if (bufferPrepared == 0)
        return 0;
    devicePrefix = (devicePrefixFlags >> 31) == 0 ? (char*)&devicePrefixFlags + 1 : (char*)devicePrefixData;
    blockBytes = FileIO_GetDeviceTransferAlignment(devicePrefix);
    if (blockBytes == 0)
        alignedBytes = 0;
    else
        alignedBytes = blockBytes * ((requestedBufferSize + blockBytes - 1) / blockBytes);
    if (completionCallback)
        busy = 1;
    int submitted;
    if (FileIO_WriteFile(fileHandle, (int)ioBuffer, alignedBytes, &transferredBytes, -1, completionCallback, 0) == 0) {
        busy = 0;
        submitted = 0;
    } else {
        submitted = 1;
    }
    return submitted;
}

#pragma push
#pragma use_lmw_stmw on
// Temporarily select the device prefix and query its save-area status.
int CFileIoFlash::CheckSaveArea(void* query, void* queryResult, int completionCallback) {
    CFileIo* self = (CFileIo*)this;
    char* devicePrefix;
    if (!query || !queryResult)
        return 0;
    devicePrefix =
        (self->devicePrefixFlags >> 31) == 0 ? (char*)&self->devicePrefixFlags + 1 : (char*)self->devicePrefixData;
    Buf1 local;
    FileIO_GetDefaultDevicePath(kDevicePathBufferBytes, local.buf);
    FileIO_SetCurrentDirectory(devicePrefix);
    if (completionCallback)
        self->busy = 1;
    if (gtl_FileIO_CheckFmemSaveArea(query, queryResult, &self->saveAreaStatusFlags, completionCallback, 0) != 0) {
        self->busy = 0;
        return 0;
    }
    return 1;
}
#pragma pop

extern "C" {

unsigned long strlen(const char*);
void String_ReserveCapacity(void*, unsigned);
struct Alloc {};
void String_ReplaceRange(void*, unsigned, unsigned, const char*, const char*, Alloc);
void appFreeUni__FPv(void*);
int FileIO_MoveFileWithCallback(const char*, void*, int, int, int);
int FileIO_MoveFile(const char*, void*);

struct SmallString {
    unsigned flags;
    unsigned count;
    char* pointer;
    SmallString() : flags(0), count(0), pointer(0) {}
    SmallString(const char* p, const Alloc& allocator = Alloc()) : flags(0), count(0), pointer(0) {
        unsigned n = strlen(p);
        String_ReserveCapacity(this, n);
        String_ReplaceRange(this, 0, 0, p, p + n, allocator);
    }
    ~SmallString() {
        if (flags >> 31)
            appFreeUni__FPv(pointer);
    }
    const char* data() const {
        return (flags >> 31) == 0 ? (const char*)this + 1 : pointer;
    }
    unsigned size() const {
        return (flags >> 31) == 0 ? (*(const unsigned char*)this & 127) : count;
    }
    unsigned find(char c) const {
        const char* p;
        unsigned n;
        if ((flags >> 31) == 0) {
            n = *(const unsigned char*)this & 127;
            p = (const char*)this + 1;
        } else {
            p = pointer;
            n = count;
        }
        if (n) {
            const char* end = p + n;
            for (const char* q = p; q < end; ++q)
                if (*q == c)
                    return q - p;
        }
        return -1;
    }
    void prefix(const SmallString& sourceString, unsigned n) {
        unsigned old = size();
        const char* p;
        unsigned len;
        if ((sourceString.flags >> 31) == 0) {
            len = *(const unsigned char*)&sourceString & 127;
            p = (const char*)&sourceString + 1;
        } else {
            p = sourceString.pointer;
            len = sourceString.count;
        }
        const unsigned& chosen = len < n ? len : n;
        n = chosen;
        Alloc allocator;
        String_ReplaceRange(this, 0, old, p, p + n, allocator);
    }
};

} // extern "C"

#pragma push
#pragma use_lmw_stmw on
// Select the source device prefix and move sourcePath to destinationPath, then restore the directory.
int CFileIoFlash::MoveQualifiedPath(void* destinationPath, const char* sourcePath, int completionCallback) {
    CFileIo* self = (CFileIo*)this;
    SmallString qualifiedPath(sourcePath);
    SmallString prefix;
    unsigned separatorIndex = qualifiedPath.find(':');
    if (separatorIndex == (unsigned)-1)
        return 0;
    prefix.prefix(qualifiedPath, separatorIndex + 1);
    const char* devicePrefix = prefix.data();
    Buf1 local;
    FileIO_GetDefaultDevicePath(kDevicePathBufferBytes, local.buf);
    FileIO_SetCurrentDirectory((void*)devicePrefix);
    int result;
    if (completionCallback) {
        self->busy = 1;
        result = FileIO_MoveFileWithCallback(sourcePath, destinationPath, completionCallback, 0, 0);
    } else {
        result = FileIO_MoveFile(sourcePath, destinationPath);
    }
    if (result == 0) {
        self->busy = 0;
        return 0;
    }
    return 1;
}
#pragma pop

extern "C" {

// Release the owned byte array, then optionally delete its wrapper.
D2* OwnedByteBuffer_dtor(D2* o, int d) {
    if (o) {
        void* p = o->ownedBytes;
        o->vt = s_ownedByteBufferVtable;
        if (p)
            __dla__FPv(p);
        o->ownedBytes = 0;
        if (d > 0)
            __dl__FPv(o);
    }
    return o;
}

} // extern "C"

// Store the operation status and report success.
int CFileIo::SetStatusCode(int status) {
    statusCode = status;
    return 1;
}

// Clear the outstanding asynchronous-operation flag.
int CFileIoFlash::ClearBusy() {
    CFileIo* o = (CFileIo*)this;
    o->busy = 0;
    return 1;
}

// Store the completed transfer byte count.
int CFileIo::SetTransferredBytes(int transferred) {
    transferredBytes = transferred;
    return 1;
}
