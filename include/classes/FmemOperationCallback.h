// FmemOperationCallback: layout view; gtl_file_io_fmem_create_directory.cpp/file_io_device_fmem_delete.cpp/gtl_fileio_fmem_get_file_size.cpp initialize 0xC-byte callback objects with vtable and two arguments
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_FMEMOPERATIONCALLBACK_H
#define CLASSES_FMEMOPERATIONCALLBACK_H

struct FmemOperationCallback {
    void* vtable; // 0x0
    int argument; // 0x4
    int callbackPayload; // 0x8
};

#define OFF(f, o) typedef char FmemOperationCallback_off_##f[((int)&((FmemOperationCallback*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x0);
OFF(argument, 0x4);
OFF(callbackPayload, 0x8);
#undef OFF
#define AsFmemOperationCallback(p) ((FmemOperationCallback*)(p))

#endif
