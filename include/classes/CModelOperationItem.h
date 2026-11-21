// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CMODELOPERATIONITEM_H
#define CLASSES_CMODELOPERATIONITEM_H

#include "game/types.h"

class CModelOperationItem {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    void* updateObject;                       // 0x10: 3 uses in CModelOperationItem_UpdateRotationAndExhibition, CModelOperationItem_CreateItemPreviewPet
    char pad_014[0x4];                  // 0x14
    int exhibitionPet;                         // 0x18: 5 uses in CModelOperationItem_SelectPreviewItem, CModelOperationItem_Ctor, CModelOperationItem_dtor...
    unsigned char isClubSet;               // 0x1c: 1 use in CModelOperationItem_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CMODELOPERATIONITEM_OFF(f, o) typedef char CModelOperationItem_off_##f[((int)&((CModelOperationItem*)0)->f == (o)) ? 1 : -1]
CMODELOPERATIONITEM_OFF(updateObject, 0x10);
CMODELOPERATIONITEM_OFF(exhibitionPet, 0x18);
CMODELOPERATIONITEM_OFF(isClubSet, 0x1c);
#undef CMODELOPERATIONITEM_OFF

#endif  // CLASSES_CMODELOPERATIONITEM_H
