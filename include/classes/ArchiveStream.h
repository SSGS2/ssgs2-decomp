// ArchiveStream: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: ArchiveContext_GetEntryInfo (0x802D4A1C) clamps reads to bufferSize-position, copies from buffer+position, and advances position.
#ifndef CLASSES_ARCHIVESTREAM_H
#define CLASSES_ARCHIVESTREAM_H

struct ArchiveStream {
    char pad_000[0x4];
    void* fileHandle; // 0x4
    char pad_008[0x8];
    unsigned char closeFileOnDestroy; // 0x10
    char pad_011[0x3];
    void* buffer; // 0x14
    unsigned int bufferSize; // 0x18
    unsigned int position; // 0x1C
};

#define OFF(f, o) typedef char ArchiveStream_off_##f[((int)&((ArchiveStream*)0)->f == (o)) ? 1 : -1]
OFF(fileHandle, 0x4);
OFF(closeFileOnDestroy, 0x10);
OFF(buffer, 0x14);
OFF(bufferSize, 0x18);
OFF(position, 0x1C);
#undef OFF
typedef char ArchiveStream_size[(sizeof(ArchiveStream) == 0x20) ? 1 : -1];
#define AsArchiveStream(p) ((ArchiveStream*)(p))

#endif // CLASSES_ARCHIVESTREAM_H
