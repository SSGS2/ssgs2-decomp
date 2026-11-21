// CFileIoAccessWritePad: owner stored by CFileIoAccessWritePad_Create (0x80276BFC).
// Layout view only; the CFileIoAccessWrite prefix remains in its existing header.
#ifndef CLASSES_CFILEIOACCESSWRITEPAD_H
#define CLASSES_CFILEIOACCESSWRITEPAD_H
struct CFileIoAccessWritePad {
    explicit CFileIoAccessWritePad(void* owner);
    char pad_0[0x14];
    void* owner; // 0x14: constructor stores the factory's owner argument.
};
#define OFF(f, o) typedef char CFileIoAccessWritePad_off_##f[((int)&((CFileIoAccessWritePad*)0)->f == (o)) ? 1 : -1]
OFF(owner, 0x14);
#undef OFF
#define AsCFileIoAccessWritePad(p) ((CFileIoAccessWritePad*)(p))
#endif
