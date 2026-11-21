// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CEXHIBITION_MODEL_POS_H
#define CLASSES_CEXHIBITION_MODEL_POS_H

#include "game/types.h"

class CExhibition_model_pos {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    void* unk_8;                        // 0x8: 1 use in ModelOperationBase_CreatePositionTable
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CEXHIBITION_MODEL_POS_OFF(f, o) typedef char CExhibition_model_pos_off_##f[((int)&((CExhibition_model_pos*)0)->f == (o)) ? 1 : -1]
CEXHIBITION_MODEL_POS_OFF(unk_8, 0x8);
#undef CEXHIBITION_MODEL_POS_OFF

#endif  // CLASSES_CEXHIBITION_MODEL_POS_H
