#ifndef CLASSES_CPRELOADFILE_H
#define CLASSES_CPRELOADFILE_H

#include "game/types.h"

class CPreloadFile {
public:
    void* vtbl;                 // 0x0: original vtable pointer
    unsigned int filenameFlags; // 0x4
    char pad_008[0x4];          // 0x8
    void* filenameBuffer;       // 0xc
    void* fileStream;           // 0x10

    int QueryStreamSlot7();
    void* GetLoadedData();
    void* QueryStreamSlot6();
    void* ScanFormat(void* fmt, ...);
    void* Read(void* opaqueReadArgument1, void* opaqueReadArgument2);
    int OpenResource(void* arg);

    ~CPreloadFile();
    CPreloadFile();
};

#define CPRELOADFILE_OFF(f, o) typedef char CPreloadFile_off_##f[((int)&((CPreloadFile*)0)->f == (o)) ? 1 : -1]
CPRELOADFILE_OFF(filenameFlags, 0x4);
CPRELOADFILE_OFF(filenameBuffer, 0xc);
CPRELOADFILE_OFF(fileStream, 0x10);
#undef CPRELOADFILE_OFF

#endif // CLASSES_CPRELOADFILE_H
