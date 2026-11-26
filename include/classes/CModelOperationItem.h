#ifndef CLASSES_CMODELOPERATIONITEM_H
#define CLASSES_CMODELOPERATIONITEM_H

#include "game/types.h"

class CModelOperationItem {
public:
    void* vtbl;              // 0x0: original vtable pointer
    char pad_004[0xc];       // 0x4
    void* updateObject;      // 0x10
    char pad_014[0x4];       // 0x14
    int exhibitionPet;       // 0x18
    unsigned char isClubSet; // 0x1c

    int OpenResource();
    int CreateItemPreviewPet();
    int SelectPreviewItem(void* arg);
    int UpdateRotationAndExhibition(float frameDelta);

    CModelOperationItem(int isClubSet);
    ~CModelOperationItem();
};

#define CMODELOPERATIONITEM_OFF(f, o) \
    typedef char CModelOperationItem_off_##f[((int)&((CModelOperationItem*)0)->f == (o)) ? 1 : -1]
CMODELOPERATIONITEM_OFF(updateObject, 0x10);
CMODELOPERATIONITEM_OFF(exhibitionPet, 0x18);
CMODELOPERATIONITEM_OFF(isClubSet, 0x1c);
#undef CMODELOPERATIONITEM_OFF

#endif // CLASSES_CMODELOPERATIONITEM_H
