// DvdDeviceFileState: layout view; gtl_file_io_device_dvd_close_part2.cpp tests ownership/free; file_io_device_dvd_seek.cpp stores the file itself as callback context; gtl_file_io_device_dvd_query.cpp returns +0x40
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_DVDDEVICEFILESTATE_H
#define CLASSES_DVDDEVICEFILESTATE_H

struct DvdDeviceFileState {
    char pad_000[0xC];
    unsigned int nameFlags; // 0xC
    char pad_010[0x4];
    void* nameData; // 0x14
    char pad_018[0x14];
    void* callbackContext; // 0x2C
    char pad_030[0x10];
    unsigned int unk_40; // 0x40
};

#define OFF(f, o) typedef char DvdDeviceFileState_off_##f[((int)&((DvdDeviceFileState*)0)->f == (o)) ? 1 : -1]
OFF(nameFlags, 0xC);
OFF(nameData, 0x14);
OFF(callbackContext, 0x2C);
OFF(unk_40, 0x40);
#undef OFF
#define AsDvdDeviceFileState(p) ((DvdDeviceFileState*)(p))

#endif
