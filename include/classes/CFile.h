#ifndef CLASSES_CFILE_H
#define CLASSES_CFILE_H

#include "types.h"

namespace gtl {

// A file opened through the engine's file I/O layer: the device handle, then the vtable pointer at 0x04.
// Methods are plain member functions; the vtable is used only through a separate virtual view in the .cpp.
class CFile {
public:
    void Init();
    CFile* Destroy(s32 flags);
    s32 Open(const char* path, u32 flags);
    s32 Read(void* buffer, u32 length);
    void WriteBuffer(s32 bufferAddress, s32 bytesToWrite);
    s64 Seek64(s32 unused, s32 highOffset, s32 lowOffset, s32 origin);
    void vfunc_5();
    void Close();
    s32 GetFileSize() const;

    s32 handle;     // 0x00
    void* vtbl;     // 0x04
    s32 opened;     // 0x08
    u32 pathFlags;  // 0x0C: SSO flags/capacity, or first byte with heap bit and inline length
    u32 pathLength; // 0x10: heap string length (inline characters overlap this word)
    u32 pathData;   // 0x14: heap pointer bits (inline characters overlap this word)
};

} // namespace gtl

#endif
