// ArchiveContext: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: ArchiveContext_ReadEntry compares/updates openIndex; ArchiveContext_GetEntryInfo copies the 0x12c result and updates cachedIndex; Archive_Close deletes the two buffers.
#ifndef CLASSES_ARCHIVECONTEXT_H
#define CLASSES_ARCHIVECONTEXT_H

struct ArchiveContext {
    void* reader; // 0x0
    int openIndex; // 0x4
    char cachedInfo[0x12c]; // 0x8
    int cachedIndex; // 0x134
    void* password; // 0x138
    void* ownedAuxiliaryBuffer; // 0x13C
};

#define OFF(f, o) typedef char ArchiveContext_off_##f[((int)&((ArchiveContext*)0)->f == (o)) ? 1 : -1]
OFF(reader, 0x0);
OFF(openIndex, 0x4);
OFF(cachedInfo, 0x8);
OFF(cachedIndex, 0x134);
OFF(password, 0x138);
OFF(ownedAuxiliaryBuffer, 0x13C);
#undef OFF
typedef char ArchiveContext_size[(sizeof(ArchiveContext) == 0x140) ? 1 : -1];
#define AsArchiveContext(p) ((ArchiveContext*)(p))

#endif // CLASSES_ARCHIVECONTEXT_H
