// gtl::CFileIoDeviceHIO: the host file device. Its file operations forward to the host stdio
// layer and most of its virtual slots are fixed results.
//
// The device holds only its path prefix string. Its virtual slots are the entries of the
// original vtable (taken from the game data), so no member is declared virtual and `vtbl`
// is spelled out as the first member.
#ifndef CLASSES_GTLFILEIODEVICEHIO_H
#define CLASSES_GTLFILEIODEVICEHIO_H

#include "types.h"
#include "classes/MWStringStorage.h"

struct HostFileHandle;

namespace gtl {

class CFileIoDeviceHIO {
public:
    // Set the default path separator in caller-owned storage and bind the vtable; returns this.
    void* Init();
    // Release the owned path text, then free the device when flags > 0; returns this.
    void* Destroy(int flags);
    // Allocate raw device storage through the installed callback; construction is separate.
    static void* AllocateStorage(u32 size);
    // Report that the host device is available.
    int IsAvailable();

    // Open a host file for the path after its device prefix; returns the owned handle or -1.
    void* OpenFile(const char* path, int flags);
    // Close a host file handle and free it.
    int CloseFile(void* handle);
    // Read from a host file; bytesRead is optional and written only on success.
    int ReadBytes(void** file, void* buffer, unsigned int byteCount, unsigned int* bytesRead);
    // Write to a host file; bytesWrittenOut is optional and written only on success.
    int Write(void** handle, void* buffer, unsigned int byteCount, unsigned int* bytesWrittenOut);
    // Seek a host file; highWord is optional and cleared on success.
    int SeekFile(void** file, void* byteOffset, unsigned int* highWord, unsigned int origin);
    // Return the low 32 bits of the file length, or all bits set for an invalid handle.
    void* GetFileSize(HostFileHandle* file, int* sizeHigh);
    // Fill a 0x34-byte stat record with the file size.
    int GetFileStat(void* handle, void* fileStat);
    // Cancellation is not implemented for this device.
    int CancelHandle();
    // Flush the stream of a host file handle.
    int Flush(void* fileHandle);

    // Fixed results for virtual slots without a host operation. The slot numbers are kept in
    // the names; the operation of each stub is not identified.
    int vfunc_11();
    int DeleteFile();
    int vfunc_13();
    int vfunc_14();
    int vfunc_15();
    int vfunc_16();
    int MoveFileToDirectoryAsync();
    int vfunc_18();
    int vfunc_19();
    int vfunc_20();
    int vfunc_21();
    int vfunc_22();
    int vfunc_23();
    int vfunc_24();
    int vfunc_25();
    int vfunc_26();
    int vfunc_27();
    int vfunc_28();
    int vfunc_29();
    int vfunc_30();
    int vfunc_31();

    void* vtbl;                 // 0x00
    MWStringStorage pathString; // 0x04
};

} // namespace gtl

#define GTL_CFILEIODEVICEHIO_OFF(f, o) \
    typedef char gtl_CFileIoDeviceHIO_off_##f[((int)&((gtl::CFileIoDeviceHIO*)0)->f == (o)) ? 1 : -1]
GTL_CFILEIODEVICEHIO_OFF(vtbl, 0x0);
GTL_CFILEIODEVICEHIO_OFF(pathString, 0x4);
#undef GTL_CFILEIODEVICEHIO_OFF

#endif
