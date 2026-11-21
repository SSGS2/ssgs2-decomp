// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CEVENTCAMERASC_H
#define CLASSES_CEVENTCAMERASC_H

#include "game/types.h"

class CEventCameraSC {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    void* cameraAnimation;              // 0x10: 6 uses in CEventCameraSC_CAM_SET_UpdateCameraAnimation

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CEVENTCAMERASC_OFF(f, o) typedef char CEventCameraSC_off_##f[((int)&((CEventCameraSC*)0)->f == (o)) ? 1 : -1]
CEVENTCAMERASC_OFF(cameraAnimation, 0x10);
#undef CEVENTCAMERASC_OFF

#endif  // CLASSES_CEVENTCAMERASC_H
