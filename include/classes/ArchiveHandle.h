// ArchiveHandle: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: Archive_Open (0x802D51F0) allocates eight bytes and writes tag 1 plus the context pointer; Archive_ReadEntry validates the tag.
#ifndef CLASSES_ARCHIVEHANDLE_H
#define CLASSES_ARCHIVEHANDLE_H

struct ArchiveHandle {
    unsigned int tag; // 0x0
    void* context; // 0x4
};

#define OFF(f, o) typedef char ArchiveHandle_off_##f[((int)&((ArchiveHandle*)0)->f == (o)) ? 1 : -1]
OFF(tag, 0x0);
OFF(context, 0x4);
#undef OFF
typedef char ArchiveHandle_size[(sizeof(ArchiveHandle) == 0x8) ? 1 : -1];
#define AsArchiveHandle(p) ((ArchiveHandle*)(p))

#endif // CLASSES_ARCHIVEHANDLE_H
