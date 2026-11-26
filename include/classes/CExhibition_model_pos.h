#ifndef CLASSES_CEXHIBITION_MODEL_POS_H
#define CLASSES_CEXHIBITION_MODEL_POS_H

#include "game/types.h"

class CExhibition_model_pos {
public:
    ~CExhibition_model_pos();

    void* vtbl;        // 0x0: original vtable pointer
    char pad_004[0x4]; // 0x4
    void* unk_8;       // 0x8
};

#define CEXHIBITION_MODEL_POS_OFF(f, o) \
    typedef char CExhibition_model_pos_off_##f[((int)&((CExhibition_model_pos*)0)->f == (o)) ? 1 : -1]
CEXHIBITION_MODEL_POS_OFF(unk_8, 0x8);
#undef CEXHIBITION_MODEL_POS_OFF

#endif // CLASSES_CEXHIBITION_MODEL_POS_H
