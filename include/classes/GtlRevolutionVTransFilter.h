// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_GTLREVOLUTIONVTRANSFILTER_H
#define CLASSES_GTLREVOLUTIONVTRANSFILTER_H

#include "game/types.h"

class GtlRevolutionVTransFilter {
public:
    char pad_000[0x4];                  // 0x0: uninterpreted prefix
    void* vtbl;                         // 0x4: gtl_CRevolutionVTransFilter_Ctor (0x803FA01C), gtl_CRevolutionVTransFilter_dtor (0x803FA0D8)
    char pad_008[0x14];                 // 0x8
    int frameHeaderSize;                         // 0x1c: gtl_CRevolutionVTransFilter_Ctor (0x803FA01C)
    char pad_020[0x4];                  // 0x20
    unsigned short yPlaneWidth;              // 0x24: gtl_CRevolutionVTransFilter_Ctor (0x803FA01C)
    unsigned short yPlaneHeight;              // 0x26: gtl_CRevolutionVTransFilter_Ctor (0x803FA01C)
    char pad_028[0x4];                  // 0x28
    unsigned short uPlaneWidth;              // 0x2c: gtl_CRevolutionVTransFilter_Ctor (0x803FA01C)
    unsigned short uPlaneHeight;              // 0x2e: gtl_CRevolutionVTransFilter_Ctor (0x803FA01C)
    char pad_030[0x4];                  // 0x30
    unsigned short vPlaneWidth;              // 0x34: gtl_CRevolutionVTransFilter_Ctor (0x803FA01C)
    unsigned short vPlaneHeight;              // 0x36: gtl_CRevolutionVTransFilter_Ctor (0x803FA01C)
    char pad_038[0x4];                  // 0x38
    int yPlane;                         // 0x3c: gtl_CRevolutionVTransFilter_Ctor (0x803FA01C)
    int uPlane;                         // 0x40: gtl_CRevolutionVTransFilter_Ctor (0x803FA01C)
    int vPlane;                         // 0x44: gtl_CRevolutionVTransFilter_Ctor (0x803FA01C)
    void* decoderWorkBuffer;                 // 0x48: gtl_CRevolutionVTransFilter_Ctor (0x803FA01C), gtl_CRevolutionVTransFilter_dtor (0x803FA0D8)
    void* frameBuffer;                 // 0x4c: gtl_CRevolutionVTransFilter_Ctor (0x803FA01C), gtl_CRevolutionVTransFilter_dtor (0x803FA0D8)
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define GTLREVOLUTIONVTRANSFILTER_OFF(f, o) typedef char GtlRevolutionVTransFilter_off_##f[((int)&((GtlRevolutionVTransFilter*)0)->f == (o)) ? 1 : -1]
GTLREVOLUTIONVTRANSFILTER_OFF(vtbl, 0x4);
GTLREVOLUTIONVTRANSFILTER_OFF(frameHeaderSize, 0x1c);
GTLREVOLUTIONVTRANSFILTER_OFF(yPlaneWidth, 0x24);
GTLREVOLUTIONVTRANSFILTER_OFF(yPlaneHeight, 0x26);
GTLREVOLUTIONVTRANSFILTER_OFF(uPlaneWidth, 0x2c);
GTLREVOLUTIONVTRANSFILTER_OFF(uPlaneHeight, 0x2e);
GTLREVOLUTIONVTRANSFILTER_OFF(vPlaneWidth, 0x34);
GTLREVOLUTIONVTRANSFILTER_OFF(vPlaneHeight, 0x36);
GTLREVOLUTIONVTRANSFILTER_OFF(yPlane, 0x3c);
GTLREVOLUTIONVTRANSFILTER_OFF(uPlane, 0x40);
GTLREVOLUTIONVTRANSFILTER_OFF(vPlane, 0x44);
GTLREVOLUTIONVTRANSFILTER_OFF(decoderWorkBuffer, 0x48);
GTLREVOLUTIONVTRANSFILTER_OFF(frameBuffer, 0x4c);
#undef GTLREVOLUTIONVTRANSFILTER_OFF

#endif  // CLASSES_GTLREVOLUTIONVTRANSFILTER_H
