// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_GTL_CSPLITFILTER_H
#define CLASSES_GTL_CSPLITFILTER_H

#include "game/types.h"

class gtl_CSplitFilter {
public:
    unsigned int unk_0;                 // 0x0: gtl_CSplitFilter_Ctor / gtl_CSplitFilter_Process / gtl_CSplitFilter_SetInputSample (split filter constructor/process/inputSample setter)
    void* vtable;                       // 0x4: gtl_CSplitFilter_Ctor / gtl_CSplitFilter_Process / gtl_CSplitFilter_SetInputSample (split filter constructor/process/inputSample setter)
    char pad_008[0x8];                  // 0x8
    void* inputSample;                        // 0x10: gtl_CSplitFilter_Ctor / gtl_CSplitFilter_Process / gtl_CSplitFilter_SetInputSample (split filter constructor/process/inputSample setter)
    void* primaryOutput;                        // 0x14: gtl_CSplitFilter_Ctor / gtl_CSplitFilter_Process / gtl_CSplitFilter_SetInputSample (split filter constructor/process/inputSample setter)
    void* secondaryOutput;                       // 0x18: gtl_CSplitFilter_Ctor / gtl_CSplitFilter_Process / gtl_CSplitFilter_SetInputSample (split filter constructor/process/inputSample setter)
    unsigned char formatsChecked;                 // 0x1c: gtl_CSplitFilter_Ctor / gtl_CSplitFilter_Process / gtl_CSplitFilter_SetInputSample (split filter constructor/process/inputSample setter)
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define GTL_CSPLITFILTER_OFF(f, o) typedef char gtl_CSplitFilter_off_##f[((int)&((gtl_CSplitFilter*)0)->f == (o)) ? 1 : -1]
GTL_CSPLITFILTER_OFF(vtable, 0x4);
GTL_CSPLITFILTER_OFF(inputSample, 0x10);
GTL_CSPLITFILTER_OFF(primaryOutput, 0x14);
GTL_CSPLITFILTER_OFF(secondaryOutput, 0x18);
GTL_CSPLITFILTER_OFF(formatsChecked, 0x1c);
GTL_CSPLITFILTER_OFF(unk_0, 0x0);
#undef GTL_CSPLITFILTER_OFF

#endif  // CLASSES_GTL_CSPLITFILTER_H
