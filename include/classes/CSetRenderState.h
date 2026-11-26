#ifndef CLASSES_CSETRENDERSTATE_H
#define CLASSES_CSETRENDERSTATE_H

#include "game/types.h"

// A renderer state saved when a guard is created; Destroy writes the value back.
class CSetRenderState {
public:
    void* Destroy(int flag);

    void* vtable;      // 0x0
    int renderStateId; // 0x4
    int savedValue;    // 0x8
};

#define CSETRENDERSTATE_OFF(f, o) typedef char CSetRenderState_off_##f[((int)&((CSetRenderState*)0)->f == (o)) ? 1 : -1]
CSETRENDERSTATE_OFF(renderStateId, 0x4);
CSETRENDERSTATE_OFF(savedValue, 0x8);
#undef CSETRENDERSTATE_OFF

#endif // CLASSES_CSETRENDERSTATE_H
