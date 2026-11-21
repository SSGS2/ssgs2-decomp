// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSINGLETASKREV_H
#define CLASSES_CSINGLETASKREV_H

#include "game/types.h"

class CSingleTaskRev {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* secondaryVtable;                        // 0x4: 1 use in CSingleTaskRev_Ctor
    void* tertiaryVtable;                        // 0x8: 1 use in CSingleTaskRev_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSINGLETASKREV_OFF(f, o) typedef char CSingleTaskRev_off_##f[((int)&((CSingleTaskRev*)0)->f == (o)) ? 1 : -1]
CSINGLETASKREV_OFF(secondaryVtable, 0x4);
CSINGLETASKREV_OFF(tertiaryVtable, 0x8);
#undef CSINGLETASKREV_OFF

#endif  // CLASSES_CSINGLETASKREV_H
