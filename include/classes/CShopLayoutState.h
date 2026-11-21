// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHOPLAYOUTSTATE_H
#define CLASSES_CSHOPLAYOUTSTATE_H

#include "game/types.h"

class CShopLayoutState {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* layout;                        // 0x4: 1 use in CShopLayoutState_IsLoaded
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHOPLAYOUTSTATE_OFF(f, o) typedef char CShopLayoutState_off_##f[((int)&((CShopLayoutState*)0)->f == (o)) ? 1 : -1]
CSHOPLAYOUTSTATE_OFF(layout, 0x4);
#undef CSHOPLAYOUTSTATE_OFF

#endif  // CLASSES_CSHOPLAYOUTSTATE_H
