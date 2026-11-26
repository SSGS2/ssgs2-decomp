// gtl::CCallBackCreateFile: callback for an asynchronous file create in the FMEM device.
// After a successful create it submits the NAND open and releases itself.
// Plain fields only: no generated vtable or RTTI.
#ifndef CLASSES_GTLCREATEFILECALLBACK_H
#define CLASSES_GTLCREATEFILECALLBACK_H

namespace gtl {

class CCallBackCreateFile {
public:
    void* vtbl;               // 0x00: OnCreateResult dispatches deleting virtual slot 2
    void* file;               // 0x04: file state passed to NANDOpenAsync
    void* path;               // 0x08: path passed to NANDOpenAsync; freed after use
    unsigned char accessMode; // 0x0C: access mode byte passed to NANDOpenAsync
    char pad_0D[3];
    void* completionCallback; // 0x10: invoked with (result, 0, file, file user data)

    void* Destroy(int deleting);
    void OnCreateResult(int createResult, void* context);
};

#define GTLCREATEFILECALLBACK_OFF(f, o) \
    typedef char CCallBackCreateFile_off_##f[((int)&((CCallBackCreateFile*)0)->f == (o)) ? 1 : -1]
GTLCREATEFILECALLBACK_OFF(vtbl, 0x0);
GTLCREATEFILECALLBACK_OFF(file, 0x4);
GTLCREATEFILECALLBACK_OFF(path, 0x8);
GTLCREATEFILECALLBACK_OFF(accessMode, 0xC);
GTLCREATEFILECALLBACK_OFF(completionCallback, 0x10);
#undef GTLCREATEFILECALLBACK_OFF

}

#endif
