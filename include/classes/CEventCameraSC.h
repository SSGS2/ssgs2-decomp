#ifndef CLASSES_CEVENTCAMERASC_H
#define CLASSES_CEVENTCAMERASC_H

#include "game/types.h"

class CEventCameraSC {
public:
    void* vtbl;            // 0x0: original vtable pointer
    char pad_004[0xc];     // 0x4
    void* cameraAnimation; // 0x10

    int CAM_SET_UpdateCameraAnimation();
};

#define CEVENTCAMERASC_OFF(f, o) typedef char CEventCameraSC_off_##f[((int)&((CEventCameraSC*)0)->f == (o)) ? 1 : -1]
CEVENTCAMERASC_OFF(cameraAnimation, 0x10);
#undef CEVENTCAMERASC_OFF

#endif // CLASSES_CEVENTCAMERASC_H
