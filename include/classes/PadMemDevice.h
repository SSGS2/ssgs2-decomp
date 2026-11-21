// PadMemDevice: layout view; gtl_file_io_device_padmem_part7.cpp frees nameData when nameStorageWord bit 31 is set; gtl_file_io_device_padmem_part1.cpp/gtl_fileio_padmem_transfer.cpp pass device to PADMEM I/O
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_PADMEMDEVICE_H
#define CLASSES_PADMEMDEVICE_H

struct PadMemDevice {
    char pad_000[0x4];
    unsigned int nameStorageWord; // 0x4
    char pad_008[0x4];
    void* nameData; // 0xC
    void* device; // 0x10
};

#define OFF(f, o) typedef char PadMemDevice_off_##f[((int)&((PadMemDevice*)0)->f == (o)) ? 1 : -1]
OFF(nameStorageWord, 0x4);
OFF(nameData, 0xC);
OFF(device, 0x10);
#undef OFF
#define AsPadMemDevice(p) ((PadMemDevice*)(p))

#endif
