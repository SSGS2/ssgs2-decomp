// ArchiveCursor: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: ArchiveContext_ReadEntry and ArchiveContext_GetEntryInfo (0x802D4FF0 / 0x802D4A1C) advance index and file offset, resetting to firstEntryOffset when seeking backward.
#ifndef CLASSES_ARCHIVECURSOR_H
#define CLASSES_ARCHIVECURSOR_H

struct ArchiveCursor {
    void* stream;            // 0x0
    unsigned int entryCount; // 0x4
    char pad_008[0x8];
    int entryIndex;           // 0x10
    unsigned int entryOffset; // 0x14
    int entryReady;           // 0x18
    char pad_01C[0x8];
    int firstEntryOffset;   // 0x24
    char entryHeader[0x20]; // 0x28
    int fileNameLength;     // 0x48
    int extraFieldLength;   // 0x4C
    int fileCommentLength;  // 0x50
    char pad_054[0x24];
    char localHeaderOffset[4]; // 0x78
    int entryReadState;        // 0x7C
};

#define OFF(f, o) typedef char ArchiveCursor_off_##f[((int)&((ArchiveCursor*)0)->f == (o)) ? 1 : -1]
OFF(stream, 0x0);
OFF(entryCount, 0x4);
OFF(entryIndex, 0x10);
OFF(entryOffset, 0x14);
OFF(entryReady, 0x18);
OFF(firstEntryOffset, 0x24);
OFF(entryHeader, 0x28);
OFF(fileNameLength, 0x48);
OFF(extraFieldLength, 0x4C);
OFF(fileCommentLength, 0x50);
OFF(localHeaderOffset, 0x78);
OFF(entryReadState, 0x7C);
#undef OFF
typedef char ArchiveCursor_size[(sizeof(ArchiveCursor) == 0x80) ? 1 : -1];
#define AsArchiveCursor(p) ((ArchiveCursor*)(p))

#endif // CLASSES_ARCHIVECURSOR_H
