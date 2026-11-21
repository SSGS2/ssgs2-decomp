// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_GTL_CTRANSFORMFILTER_H
#define CLASSES_GTL_CTRANSFORMFILTER_H

#include "game/types.h"

class gtl_CTransformFilter {
public:
    unsigned int unk_0;                 // 0x0: gtl_CTransformFilter_Ctor / gtl_CTransformFilter_Process (transform filter constructor/process)
    void* vtable;                       // 0x4: gtl_CTransformFilter_Ctor / gtl_CTransformFilter_Process (transform filter constructor/process)
    char pad_008[0x8];                  // 0x8
    void* inputSample;                        // 0x10: gtl_CTransformFilter_Ctor / gtl_CTransformFilter_Process (transform filter constructor/process)
    void* outputSample;                        // 0x14: gtl_CTransformFilter_Ctor / gtl_CTransformFilter_Process (transform filter constructor/process)
    unsigned char formatsChecked;                 // 0x18: gtl_CTransformFilter_Ctor / gtl_CTransformFilter_Process (transform filter constructor/process)
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define GTL_CTRANSFORMFILTER_OFF(f, o) typedef char gtl_CTransformFilter_off_##f[((int)&((gtl_CTransformFilter*)0)->f == (o)) ? 1 : -1]
GTL_CTRANSFORMFILTER_OFF(vtable, 0x4);
GTL_CTRANSFORMFILTER_OFF(inputSample, 0x10);
GTL_CTRANSFORMFILTER_OFF(outputSample, 0x14);
GTL_CTRANSFORMFILTER_OFF(formatsChecked, 0x18);
GTL_CTRANSFORMFILTER_OFF(unk_0, 0x0);
#undef GTL_CTRANSFORMFILTER_OFF

#endif  // CLASSES_GTL_CTRANSFORMFILTER_H
