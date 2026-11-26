// CFileIoAccessWritePad: pad-file write access. The first 0x14 bytes are the
// CFileIoAccessWrite layout (classes/CFileIoAccessWrite.h); the owner is stored at 0x14.
#ifndef CLASSES_CFILEIOACCESSWRITEPAD_H
#define CLASSES_CFILEIOACCESSWRITEPAD_H

struct CFileIoAccessWritePad {
    explicit CFileIoAccessWritePad(void* owner);

    // Destroy the write-access base; deletes this object when flag > 0. Returns this.
    void* Destroy(int flag);
    // Create a pad-file instance through the owner.
    void* CreatePad(int openArgument1, int openArgument2);

    static void* Create(void* owner, void* openOptions, void* byteCount, void* sourceData);
    static void InitAngleCache();

    char pad_0[0x14];
    void* owner; // 0x14: constructor stores the factory's owner argument.
};

typedef char CFileIoAccessWritePad_off_owner[((int)&((CFileIoAccessWritePad*)0)->owner == 0x14) ? 1 : -1];

#endif
