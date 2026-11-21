// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CMODELOPERATIONCADDIE_H
#define CLASSES_CMODELOPERATIONCADDIE_H

#include "game/types.h"

class CModelOperationCaddie {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    void* exhibition;                       // 0x10: 5 uses in ModelOperationBase_Update, CModelOperationCaddie_CreatePetPreview
    void* saveRecord;                       // 0x14: 1 use in CModelOperationCaddie_SelectCaddie
    void* asyncPet;                       // 0x18: 5 uses in CModelOperationCaddie_SelectCaddie, CModelOperationCaddie_Ctor, CModelOperationCaddie_dtor...
    int inventoryIndex;                         // 0x1c: 2 uses in CModelOperationCaddie_Ctor, CModelOperationCaddie_SelectCaddie
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CMODELOPERATIONCADDIE_OFF(f, o) typedef char CModelOperationCaddie_off_##f[((int)&((CModelOperationCaddie*)0)->f == (o)) ? 1 : -1]
CMODELOPERATIONCADDIE_OFF(exhibition, 0x10);
CMODELOPERATIONCADDIE_OFF(saveRecord, 0x14);
CMODELOPERATIONCADDIE_OFF(asyncPet, 0x18);
CMODELOPERATIONCADDIE_OFF(inventoryIndex, 0x1c);
#undef CMODELOPERATIONCADDIE_OFF

#endif  // CLASSES_CMODELOPERATIONCADDIE_H
