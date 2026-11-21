// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPETFORMATEXTERNALFILE_H
#define CLASSES_CPETFORMATEXTERNALFILE_H

#include "game/types.h"

class CPetFormatExternalFile {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int chunkData;                          // 0x4: 1 use in CPetFormatExternalFile_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPETFORMATEXTERNALFILE_OFF(f, o) typedef char CPetFormatExternalFile_off_##f[((int)&((CPetFormatExternalFile*)0)->f == (o)) ? 1 : -1]
CPETFORMATEXTERNALFILE_OFF(chunkData, 0x4);
#undef CPETFORMATEXTERNALFILE_OFF

#endif  // CLASSES_CPETFORMATEXTERNALFILE_H
