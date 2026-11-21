// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_GTL_CSOURCEFILTER_H
#define CLASSES_GTL_CSOURCEFILTER_H

#include "game/types.h"

class gtl_CSourceFilter {
public:
    unsigned int unk_0;                 // 0x0: gtl_CSourceFilter_Ctor / gtl_CSourceFilter_Process (source filter constructor/process)
    void* vtable;                       // 0x4: gtl_CSourceFilter_Ctor / gtl_CSourceFilter_Process (source filter constructor/process)
    char pad_008[0x8];                  // 0x8
    void* outputSample;                        // 0x10: gtl_CSourceFilter_Ctor / gtl_CSourceFilter_Process (source filter constructor/process)
    void* samplePool;                        // 0x14: gtl_CSourceFilter_Ctor / gtl_CSourceFilter_Process (source filter constructor/process)
    unsigned char readPending;                 // 0x18: gtl_CSourceFilter_Ctor / gtl_CSourceFilter_Process (source filter constructor/process)
    char pad_019[0x3];                  // 0x19
    void* callback;                     // 0x1c: gtl_CSourceFilter_Ctor / gtl_CSourceFilter_Process (source filter constructor/process)
    void* source;                       // 0x20: gtl_CSourceFilter_Ctor / gtl_CSourceFilter_Process (source filter constructor/process)
    unsigned int readOffset;                 // 0x24: gtl_CSourceFilter_Ctor / gtl_CSourceFilter_Process (source filter constructor/process)
    unsigned char sourceValidated;                // 0x28: gtl_CSourceFilter_Ctor / gtl_CSourceFilter_Process (source filter constructor/process)
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define GTL_CSOURCEFILTER_OFF(f, o) typedef char gtl_CSourceFilter_off_##f[((int)&((gtl_CSourceFilter*)0)->f == (o)) ? 1 : -1]
GTL_CSOURCEFILTER_OFF(vtable, 0x4);
GTL_CSOURCEFILTER_OFF(outputSample, 0x10);
GTL_CSOURCEFILTER_OFF(samplePool, 0x14);
GTL_CSOURCEFILTER_OFF(readPending, 0x18);
GTL_CSOURCEFILTER_OFF(callback, 0x1c);
GTL_CSOURCEFILTER_OFF(source, 0x20);
GTL_CSOURCEFILTER_OFF(readOffset, 0x24);
GTL_CSOURCEFILTER_OFF(sourceValidated, 0x28);
GTL_CSOURCEFILTER_OFF(unk_0, 0x0);
#undef GTL_CSOURCEFILTER_OFF

#endif  // CLASSES_GTL_CSOURCEFILTER_H
