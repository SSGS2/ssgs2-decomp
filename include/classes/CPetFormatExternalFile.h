#ifndef CLASSES_CPETFORMATEXTERNALFILE_H
#define CLASSES_CPETFORMATEXTERNALFILE_H

#include "game/types.h"

class CPetFormatExternalFile {
public:
    void* vtbl;    // 0x0: original vtable pointer
    int chunkData; // 0x4

    void* GetPayload();
    void* GetChunkData();

    CPetFormatExternalFile(int value);
    ~CPetFormatExternalFile();
};

#define CPETFORMATEXTERNALFILE_OFF(f, o) \
    typedef char CPetFormatExternalFile_off_##f[((int)&((CPetFormatExternalFile*)0)->f == (o)) ? 1 : -1]
CPETFORMATEXTERNALFILE_OFF(chunkData, 0x4);
#undef CPETFORMATEXTERNALFILE_OFF

#endif // CLASSES_CPETFORMATEXTERNALFILE_H
