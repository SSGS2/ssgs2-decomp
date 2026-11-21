// gtl::nw4r::ut::DvdFileStream: non-virtual view of the existing object.
// Methods in gtl_dvd_file_stream_read_async_part1.cpp/gtl_dvd_file_stream_read_part2.cpp/gtl_dvd_file_stream_cancel.cpp/gtl_dvd_file_stream_and_device.cpp establish these offsets.
#ifndef CLASSES_GTLDVDFILESTREAM_H
#define CLASSES_GTLDVDFILESTREAM_H

class GtlDvdFileStream {
public:
    char pad_000[0x0C];
    int readCallback;              // 0x80412448: stores incoming arg6 before DVDReadAsyncPrio
    int readUserData;             // 0x80412448: stores incoming arg7 before DVDReadAsyncPrio
    unsigned int length;    // 0x8041253C: rounded read limit, loaded from +0x14
    unsigned int position;  // 0x8041253C: DVDReadPrio offset, loaded from +0x18
    void* cancelCallback;           // 0x80412608: stores incoming arg1
    void* cancelUserData;           // 0x80412608: stores incoming arg2
    unsigned char cancelPending;   // 0x80412608: set on successful DVDCancelAsync call
    char pad_025[3];
    char fileInfo[0x40];    // 0x8041253C/0x80412448: first DVDReadPrio[Async] argument
    int priority;           // 0x8041256C: loaded into DVDReadPrio's final argument
    unsigned char busy;    // 0x80412448: set before async read, cleared on failure
};

#define GTLDVDFILESTREAM_OFF(f, o) typedef char GtlDvdFileStream_off_##f[((int)&((GtlDvdFileStream*)0)->f == (o)) ? 1 : -1]
GTLDVDFILESTREAM_OFF(readCallback, 0x0C);
GTLDVDFILESTREAM_OFF(readUserData, 0x10);
GTLDVDFILESTREAM_OFF(length, 0x14);
GTLDVDFILESTREAM_OFF(position, 0x18);
GTLDVDFILESTREAM_OFF(cancelCallback, 0x1C);
GTLDVDFILESTREAM_OFF(cancelUserData, 0x20);
GTLDVDFILESTREAM_OFF(cancelPending, 0x24);
GTLDVDFILESTREAM_OFF(fileInfo, 0x28);
GTLDVDFILESTREAM_OFF(priority, 0x68);
GTLDVDFILESTREAM_OFF(busy, 0x6C);
#undef GTLDVDFILESTREAM_OFF

#endif
