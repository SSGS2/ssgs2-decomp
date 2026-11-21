#include "types.h"
#include "sdk_protos.h"

extern "C" {

// The write-mode string starts three bytes after the read-mode string.
struct HostFileModes { char readBinary[3]; char writeBinary[3]; };
extern char s_hostFileOpenModeStrings[];
enum HostFileModeOffsets { kReadBinaryMode = 0, kWriteBinaryMode = sizeof(((HostFileModes*)0)->readBinary) };
struct HostFileHandle { void* stream; void* reserved; };
enum HostFileOpenFlags { kOpenRead = 0x80000000u, kOpenWrite = 0x40000000u };
enum HostFileErrors { kFileNotFound = 2, kHostOpenFailed = 0x6e };
enum { kHostPathBytes = 0x104 };
void FileIO_Free(void *p);
void *FileIO_Alloc(u32 size);
int fopen(const char *a, const char *b);
void File_SetLastError(int x);

// Borrow path; it must terminate within 0x104 bytes and contain a separator.
// Caller supplies at least one mode bit and successful handle allocation is assumed.
// Write wins when both bits are present. Success transfers a stream-owning wrapper
// to the caller (closed by HIO vfunc_3); failure frees the wrapper and returns (void*)-1.
void *CFileIoDeviceHIO_OpenFile(void *self, const char *path, int flags) {
    char hostPath[kHostPathBytes];
    int openFlags = flags;
    char *mode;
    char *cursor;
    char *deviceSeparator;
    // Work on a bounded local copy; strncpy does not add a terminator if it fills the buffer.
    strncpy(hostPath, path, sizeof(hostPath));
    cursor = hostPath;
    while (*cursor != 0) {
        if (*cursor == '/') {
            *cursor = '\\';
        }
        cursor++;
    }
    // Remove the device prefix through its first normalized separator.
    deviceSeparator = strchr(hostPath, '\\');
    mode = 0;
    if (openFlags & kOpenRead) {
        mode = s_hostFileOpenModeStrings + kReadBinaryMode;
    }
    if (openFlags & kOpenWrite) {
        mode = s_hostFileOpenModeStrings + kWriteBinaryMode;
    }
    // Allocate the owning wrapper before opening; fopen receives the suffix and mode.
    HostFileHandle *handle = (HostFileHandle*)FileIO_Alloc(sizeof(HostFileHandle));
    if (handle != 0) {
        handle->stream = 0;
        handle->reserved = 0;
    }
    void *stream = (void *)fopen(deviceSeparator + 1, mode);
    handle->stream = stream;
    if (stream == 0) {
        // Preserve both error updates and their order; the second is the final last-error value.
        File_SetLastError(kFileNotFound);
        File_SetLastError(kHostOpenFailed);
        if (handle != 0) {
            FileIO_Free(handle);
        }
        return (void *)-1;
    }
    return handle;
}

} // extern "C"
