// The base of the preview operations: the shared resource handle, the position
// table, the exhibition and the save record they all work with.
#ifndef CLASSES_CMODELOPERATION_H
#define CLASSES_CMODELOPERATION_H

#include "game/types.h"

class CModelOperation {
public:
    // Open the shared resource at path; returns whether it was created.
    int OpenResource(const char* path);

    void* vtbl;                           // 0x0: original vtable pointer
    void* sharedFile;                     // 0x4
    void* positionTable;                  // 0x8
    unsigned char pointerRotationEnabled; // 0xc
    char pad_00D[0x3];                    // 0xd
    void* asyncExhibition;                // 0x10
    void* saveData;                       // 0x14
};

#define CMODELOPERATION_OFF(f, o) typedef char CModelOperation_off_##f[((int)&((CModelOperation*)0)->f == (o)) ? 1 : -1]
CMODELOPERATION_OFF(sharedFile, 0x4);
CMODELOPERATION_OFF(positionTable, 0x8);
CMODELOPERATION_OFF(pointerRotationEnabled, 0xc);
CMODELOPERATION_OFF(asyncExhibition, 0x10);
CMODELOPERATION_OFF(saveData, 0x14);
#undef CMODELOPERATION_OFF
typedef char CModelOperation_size[(sizeof(CModelOperation) == 0x18) ? 1 : -1];

#endif // CLASSES_CMODELOPERATION_H
