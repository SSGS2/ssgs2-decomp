// gtl::CFileIoDevicePADMEM: partial layout from its methods in
// gtl_file_io_device_padmem_part2.cpp/gtl_file_io_device_padmem_part3.cpp. This view emits no virtual table or constructors.
#ifndef CLASSES_GTLFILEIODEVICEPADMEM_H
#define CLASSES_GTLFILEIODEVICEPADMEM_H

class GtlFileIoDevicePADMEM {
public:
    char pad_000[0x10];
    void* controllerIndex; // PadMemory_QueryStoredMetadata/PadMemory_QueryStoredTitle first argument (0x80420288/0x80420914)
};

#define GTLFILEIODEVICEPADMEM_OFF(f, o) typedef char GtlFileIoDevicePADMEM_off_##f[((int)&((GtlFileIoDevicePADMEM*)0)->f == (o)) ? 1 : -1]
GTLFILEIODEVICEPADMEM_OFF(controllerIndex, 0x10);
#undef GTLFILEIODEVICEPADMEM_OFF

#endif
