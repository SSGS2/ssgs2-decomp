// File I/O device root path: copies the root path into a caller buffer and replaces it with new text.
// The path is stored as a string that is either short (inline) or kept on the heap.

#include "sdk_protos.h"
#include "classes/gtl_CFileIoDevice.h"

extern "C" void* memcpy(void*, const void*, unsigned int);

extern "C" void FileIOString_ReplaceRangeWithAllocator(void*, unsigned int, unsigned int, const char*, const char*,
                                                       void*);

// The root path string is a MWStringStorage: a high flag bit selects heap storage; otherwise the low 7 bits of
// the first byte are the length and the characters start one byte after the flags.
enum DeviceStringConstants {
    kHeapStringFlagShift = 31,
    kInlineStringLengthMask = 0x7F
};

// Copy the root path, including its terminator; return 0 without writing if dest is too small.
int gtl::CFileIoDevice::CopyRootPath(char* dest, unsigned int capacityBytes) {
    unsigned int length;
    unsigned int textLength;
    int isSmall;
    unsigned int flags = rootPath.flags;
    isSmall = !(flags >> kHeapStringFlagShift);
    if (isSmall)
        textLength = *(unsigned char*)&rootPath.flags & kInlineStringLengthMask;
    else
        textLength = rootPath.length;
    if (capacityBytes <= textLength)
        return 0;
    if (textLength == 0) {
        dest[0] = 0;
    } else {
        const char* source;
        if (isSmall)
            source = (const char*)&rootPath + 1;
        else
            source = rootPath.data;
        memcpy(dest, source, textLength);
        dest[textLength] = 0;
    }
    return 1;
}

// Replace the root path with a copy of text.
void gtl::CFileIoDevice::SetRootPath(const char* text) {
    const char* input = text;
    unsigned int flags = rootPath.flags;
    unsigned int oldTextLength;
    unsigned int length;
    char allocatorToken;
    if (!(flags >> kHeapStringFlagShift))
        oldTextLength = *(unsigned char*)&rootPath.flags & kInlineStringLengthMask;
    else
        oldTextLength = rootPath.length;
    // The allocator argument is a scratch byte read one past its own slot. The compiled code does this read and
    // the callee does not use the value, so it is kept to match the original.
    allocatorToken = *((char*)&allocatorToken + 4);
    length = strlen(input);
    FileIOString_ReplaceRangeWithAllocator(&rootPath, 0, oldTextLength, input, input + length, &allocatorToken);
}
