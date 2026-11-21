// DvdStreamFields: layout view; gtl_dvd_file_stream.cpp: read/cancel completion callbacks and ctor file-length copy; gtl_dvd_file_stream_read_async_part2.cpp saves read callback arguments
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_DVDSTREAMFIELDS_H
#define CLASSES_DVDSTREAMFIELDS_H

struct DvdStreamFields {
    char pad_000[0x8];
    void* readResult; // 0x8
    void* readCallback; // 0xC
    void* readUserData; // 0x10
    char pad_014[0x8];
    void* cancelCallback; // 0x1C
    void* cancelUserData; // 0x20
    char pad_024[0x38];
    void* fileLength; // 0x5C
};

#define OFF(f, o) typedef char DvdStreamFields_off_##f[((int)&((DvdStreamFields*)0)->f == (o)) ? 1 : -1]
OFF(readResult, 0x8);
OFF(readCallback, 0xC);
OFF(readUserData, 0x10);
OFF(cancelCallback, 0x1C);
OFF(cancelUserData, 0x20);
OFF(fileLength, 0x5C);
#undef OFF
#define AsDvdStreamFields(p) ((DvdStreamFields*)(p))

#endif
