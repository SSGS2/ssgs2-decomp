// gtl::CCallBackCreateFile layout view; method owned by gtl_fmem_create_file_callback.cpp.
// Plain fields only: no generated vtable or RTTI.
#ifndef CLASSES_GTLCREATEFILECALLBACK_H
#define CLASSES_GTLCREATEFILECALLBACK_H

class GtlCreateFileCallback {
public:
    void* vtbl; // 0x00: gtl_CCallBackCreateFile_OnCreateResult dispatches deleting virtual slot 2
    void* file; // 0x04: original 0x8041F514 loads NANDOpenAsync's file argument
    void* path; // 0x08: original 0x8041F510 loads NANDOpenAsync's path argument; freed after use
    unsigned char accessMode; // 0x0C: original 0x8041F51C loads NANDOpenAsync's mode byte
    char pad_0D[3];
    void* completionCallback; // 0x10: invoked with (result, 0, file, file user data)
};

#define GTLCREATEFILECALLBACK_OFF(f, o) typedef char GtlCreateFileCallback_off_##f[((int)&((GtlCreateFileCallback*)0)->f == (o)) ? 1 : -1]
GTLCREATEFILECALLBACK_OFF(vtbl, 0x0);
GTLCREATEFILECALLBACK_OFF(file, 0x4);
GTLCREATEFILECALLBACK_OFF(path, 0x8);
GTLCREATEFILECALLBACK_OFF(accessMode, 0xC);
GTLCREATEFILECALLBACK_OFF(completionCallback, 0x10);
#undef GTLCREATEFILECALLBACK_OFF

#endif
