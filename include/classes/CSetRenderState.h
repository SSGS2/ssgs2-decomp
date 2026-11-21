// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSETRENDERSTATE_H
#define CLASSES_CSETRENDERSTATE_H

#include "game/types.h"

class CSetRenderState {
public:
    void* vtable;                        // 0x0: 3 uses in FadeState_Draw, CSetRenderState_dtor
    float renderStateId;                        // 0x4: 6 uses in FadeState_Draw, CSetRenderState_dtor
    int savedValue;                          // 0x8: 1 use in CSetRenderState_dtor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSETRENDERSTATE_OFF(f, o) typedef char CSetRenderState_off_##f[((int)&((CSetRenderState*)0)->f == (o)) ? 1 : -1]
CSETRENDERSTATE_OFF(renderStateId, 0x4);
CSETRENDERSTATE_OFF(savedValue, 0x8);
#undef CSETRENDERSTATE_OFF

#endif  // CLASSES_CSETRENDERSTATE_H
