// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CMODELOPERATION_H
#define CLASSES_CMODELOPERATION_H

#include "game/types.h"

class CModelOperation {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int sharedFile;                     // 0x4: 1 use in ModelOperationBase_Ctor
    int positionTable;                          // 0x8: 1 use in ModelOperationBase_Ctor
    char pointerRotationEnabled;                         // 0xc: 1 use in ModelOperationBase_Ctor
    char pad_00D[0x3];                  // 0xd
    int asyncExhibition;                         // 0x10: 1 use in ModelOperationBase_Ctor
    int saveData;                         // 0x14: 1 use in ModelOperationBase_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CMODELOPERATION_OFF(f, o) typedef char CModelOperation_off_##f[((int)&((CModelOperation*)0)->f == (o)) ? 1 : -1]
CMODELOPERATION_OFF(sharedFile, 0x4);
CMODELOPERATION_OFF(positionTable, 0x8);
CMODELOPERATION_OFF(pointerRotationEnabled, 0xc);
CMODELOPERATION_OFF(asyncExhibition, 0x10);
CMODELOPERATION_OFF(saveData, 0x14);
#undef CMODELOPERATION_OFF

#endif  // CLASSES_CMODELOPERATION_H
