// Translation unit 0x80413AFC-0x80413C2C (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "classes/MWStringStorage.h"

extern "C" void *memcpy(void *, const void *, unsigned int);

extern "C" {
void FileIOString_ReplaceRangeWithAllocator(void *, unsigned int, unsigned int, const char *, const char *, void *);

// The device root path is initialized to "/" by DVD/PADMEM constructors
// (gtl_dvd_file_stream_and_device.cpp/gtl_file_io_devices_and_registry.cpp); these base slots copy/set that same +4 string.
// Embedded string at object+4: high flag bit selects heap storage; otherwise the
// first byte's low 7 bits are length and inline characters begin at object+5.
struct DeviceRootPathView {
    void* vtable;
    MWStringStorage rootPath;
};

// Borrow the embedded path descriptor; this view never acquires string ownership.
static inline MWStringStorage* DeviceRootPath(void* object) {
    return &((DeviceRootPathView*)object)->rootPath;
}

enum DeviceStringConstants { kHeapStringFlagShift = 31, kInlineStringLengthMask = 0x7F };

// Copy the device root path, including NUL; return 0 without writing if too small.
// dest is borrowed writable storage of capacityBytes bytes; success returns 1.
int gtl_CFileIoDevice_CopyRootPath(void *object, char *dest, unsigned int capacityBytes) {
    unsigned int length;
    unsigned int textLength;
    int isSmall;
    unsigned int flags = DeviceRootPath(object)->flags; // 0x80413AFC: embedded string
    isSmall = !(flags >> kHeapStringFlagShift);
    if (isSmall)
        textLength = *(unsigned char *)&DeviceRootPath(object)->flags & kInlineStringLengthMask;
    else
        textLength = DeviceRootPath(object)->length;
    if (capacityBytes <= textLength)
        return 0;
    if (textLength == 0) {
        dest[0] = 0;
    } else {
        const char *source;
        if (isSmall)
            source = (const char *)DeviceRootPath(object) + 1;
        else
            source = DeviceRootPath(object)->data;
        memcpy(dest, source, textLength);
        dest[textLength] = 0;
    }
    return 1;
}

// Replace the entire device root path with a copy of borrowed NUL-terminated text.
// Allocation/exception behavior is delegated to ReplaceRangeWithAllocator.
void gtl_CFileIoDevice_SetRootPath(void *object, const char *text) {
    const char *input = text;
    unsigned int flags = DeviceRootPath(object)->flags; // 0x80413BA4
    unsigned int oldTextLength;
    unsigned int length;
    char allocatorToken;
    if (!(flags >> kHeapStringFlagShift))
        oldTextLength = *(unsigned char *)&DeviceRootPath(object)->flags & kInlineStringLengthMask;
    else
        oldTextLength = DeviceRootPath(object)->length;
    // Empty-allocator ABI scratch, NOT path data: the original 0x80413FB8
    // replacement body never reads incoming r8 (allocator pointer); it uses
    // FileIO_Alloc/FileIO_Free directly. Even its overlap-recursion path creates
    // its own scratch tag. Thus this copied byte cannot affect the path result.
    // Retain the +4 stack read/store for matching; root-path edits belong in
    allocatorToken = *((char *)&allocatorToken + 4);
    length = strlen(input);
    FileIOString_ReplaceRangeWithAllocator(DeviceRootPath(object), 0, oldTextLength, input, input + length, &allocatorToken);
}
} // extern "C"

