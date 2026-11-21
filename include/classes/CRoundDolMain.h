// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CROUNDDOLMAIN_H
#define CLASSES_CROUNDDOLMAIN_H

#include "game/types.h"

class CRoundDolMain {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    unsigned int taskMainVtable;                // 0x18: 2 uses in CRoundDolMain_Ctor, CRoundDolMain_dtor
    void* dolfiniSession;                       // 0x1c: 2 uses in CRoundDolMain_dtor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CROUNDDOLMAIN_OFF(f, o) typedef char CRoundDolMain_off_##f[((int)&((CRoundDolMain*)0)->f == (o)) ? 1 : -1]
CROUNDDOLMAIN_OFF(taskMainVtable, 0x18);
CROUNDDOLMAIN_OFF(dolfiniSession, 0x1c);
#undef CROUNDDOLMAIN_OFF

#endif  // CLASSES_CROUNDDOLMAIN_H
