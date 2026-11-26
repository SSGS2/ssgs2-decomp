// DVD request view, owned by file_io_device_dvd_async_read.cpp. gtl::CFileIoDeviceDVD::StartChunkedAsyncRead saves total length, next buffer/offset, DVDReadAsyncPrio priority and completion arguments. No constructors or virtual methods.
#ifndef CLASSES_DVDREADREQUEST80413768_H
#define CLASSES_DVDREADREQUEST80413768_H

struct ChunkedDvdReadRequest {
    char dvdCommand[0x2c];
    void* readContext;
    char pad_30[0x10];
    int bytesRead;
    int remainingBytes;
    int nextBuffer;
    int nextOffset;
    int priority;
    int completionCallback;
    int completionUserData;
};

#define OFF(f, o) typedef char DvdReadRequest80413768_off_##f[((int)&((ChunkedDvdReadRequest*)0)->f == (o)) ? 1 : -1]
OFF(dvdCommand, 0x0);
OFF(readContext, 0x2c);
OFF(bytesRead, 0x40);
OFF(remainingBytes, 0x44);
OFF(nextBuffer, 0x48);
OFF(nextOffset, 0x4c);
OFF(priority, 0x50);
OFF(completionCallback, 0x54);
OFF(completionUserData, 0x58);
#undef OFF

#endif
