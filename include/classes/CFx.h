// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFX_H
#define CLASSES_CFX_H

#include "game/types.h"

class CFx {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* secondaryVtable;                        // 0x4: 2 uses in CPvsFxBox_ctor, CFx_Dtor
    char pad_008[0xc];                  // 0x8
    float pendingDeltaTime;                       // 0x14: 1 use in CPvsFxBox_ctor
    char pad_018[0x4];                  // 0x18
    float effectNominalStepMilliseconds;                       // 0x1c: 1 use in CPvsFxBox_ctor
    char pad_020[0x378];                // 0x20
    void* unk_398;                      // 0x398: 1 use in CFx_Dtor
    char pad_39C[0x8];                  // 0x39c
    void* unk_3A4;                      // 0x3a4: 1 use in CFx_Dtor
    char pad_3A8[0x4];                  // 0x3a8
    unsigned int controlCount;               // 0x3ac: 2 uses in CPvsFxBox_ctor, CFx_Dtor
    unsigned int controlRoot;               // 0x3b0: 2 uses in CPvsFxBox_ctor, CFx_Dtor
    void* controlFirst;                      // 0x3b4: 1 use in CPvsFxBox_ctor
    char updateEnabled;                       // 0x3b8: 1 use in CPvsFxBox_ctor
    char paused;                       // 0x3b9: 1 use in CPvsFxBox_ctor
    char attachedEffectsEnabled;                       // 0x3ba: 1 use in CPvsFxBox_ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x18, 0x394, 0x3a0, 0x3ac, 0x3b0
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFX_OFF(f, o) typedef char CFx_off_##f[((int)&((CFx*)0)->f == (o)) ? 1 : -1]
CFX_OFF(secondaryVtable, 0x4);
CFX_OFF(pendingDeltaTime, 0x14);
CFX_OFF(effectNominalStepMilliseconds, 0x1c);
CFX_OFF(unk_398, 0x398);
CFX_OFF(unk_3A4, 0x3a4);
CFX_OFF(controlCount, 0x3ac);
CFX_OFF(controlRoot, 0x3b0);
CFX_OFF(controlFirst, 0x3b4);
CFX_OFF(updateEnabled, 0x3b8);
CFX_OFF(paused, 0x3b9);
CFX_OFF(attachedEffectsEnabled, 0x3ba);
#undef CFX_OFF

#endif  // CLASSES_CFX_H
