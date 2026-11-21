// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CEVENTEFFECT_H
#define CLASSES_CEVENTEFFECT_H

#include "game/types.h"

class CEventEffect {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* treeRoot;                        // 0x4: 3 uses in CEventEffect_dtor
    void* firstNode;                        // 0x8: 2 uses in CEventEffect_dtor
    void* effectVtable;                        // 0xc: 1 use in CEventEffect_dtor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CEVENTEFFECT_OFF(f, o) typedef char CEventEffect_off_##f[((int)&((CEventEffect*)0)->f == (o)) ? 1 : -1]
CEVENTEFFECT_OFF(treeRoot, 0x4);
CEVENTEFFECT_OFF(firstNode, 0x8);
CEVENTEFFECT_OFF(effectVtable, 0xc);
#undef CEVENTEFFECT_OFF

#endif  // CLASSES_CEVENTEFFECT_H
