// FmemFileState: layout view; gtl_file_io_device_fmem_open.cpp initializes +0x8C/+0x94; gtl_file_io_device_fmem_part4.cpp returns +0x90; meanings not established
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_FMEMFILESTATE_H
#define CLASSES_FMEMFILESTATE_H

struct FmemFileState {
    char pad_000[0x8C];
    int operationState; // 0x8C
    int operationResult; // 0x90
    int callbackArgument; // 0x94
};

#define OFF(f, o) typedef char FmemFileState_off_##f[((int)&((FmemFileState*)0)->f == (o)) ? 1 : -1]
OFF(operationState, 0x8C);
OFF(operationResult, 0x90);
OFF(callbackArgument, 0x94);
#undef OFF
#define AsFmemFileState(p) ((FmemFileState*)(p))

#endif
