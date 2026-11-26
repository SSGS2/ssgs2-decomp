// ArchiveEntryInfo: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: ArchiveContext_GetEntryInfo (0x802D4A1C) writes the selected index, copies 0x104 path bytes, and builds attribute bits at 0x108.
#ifndef CLASSES_ARCHIVEENTRYINFO_H
#define CLASSES_ARCHIVEENTRYINFO_H

struct ArchiveEntryInfo {
    unsigned int index;                // 0x0
    char path[0x104];                  // 0x4
    unsigned int attributes;           // 0x108
    unsigned int accessTimeLow;        // 0x10C
    unsigned int accessTimeHigh;       // 0x110
    unsigned int creationTimeLow;      // 0x114
    unsigned int creationTimeHigh;     // 0x118
    unsigned int modificationTimeLow;  // 0x11C
    unsigned int modificationTimeHigh; // 0x120
    unsigned int compressedSize;       // 0x124
    unsigned int uncompressedSize;     // 0x128
};

#define OFF(f, o) typedef char ArchiveEntryInfo_off_##f[((int)&((ArchiveEntryInfo*)0)->f == (o)) ? 1 : -1]
OFF(index, 0x0);
OFF(path, 0x4);
OFF(attributes, 0x108);
OFF(accessTimeLow, 0x10C);
OFF(accessTimeHigh, 0x110);
OFF(creationTimeLow, 0x114);
OFF(creationTimeHigh, 0x118);
OFF(modificationTimeLow, 0x11C);
OFF(modificationTimeHigh, 0x120);
OFF(compressedSize, 0x124);
OFF(uncompressedSize, 0x128);
#undef OFF
typedef char ArchiveEntryInfo_size[(sizeof(ArchiveEntryInfo) == 0x12C) ? 1 : -1];
#define AsArchiveEntryInfo(p) ((ArchiveEntryInfo*)(p))

#endif // CLASSES_ARCHIVEENTRYINFO_H
