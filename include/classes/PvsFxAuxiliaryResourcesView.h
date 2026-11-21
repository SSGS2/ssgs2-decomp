// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_OWNEDRESOURCES80192FFC_H
#define CLASSES_OWNEDRESOURCES80192FFC_H

#include "game/types.h"

class PvsFxAuxiliaryResourcesView {
public:
    char pad_000[4]; // 0x0: unmodelled bytes
    char pad_004[0x10];                 // 0x4
    void* firstResource;                // 0x14: PvsFxAuxiliaryResources_Dtor original object access
    char pad_018[0x4];                  // 0x18
    void* secondResource;               // 0x1c: PvsFxAuxiliaryResources_Dtor original object access
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define OWNEDRESOURCES80192FFC_OFF(f, o) typedef char OwnedResources80192FFC_off_##f[((int)&((PvsFxAuxiliaryResourcesView*)0)->f == (o)) ? 1 : -1]
OWNEDRESOURCES80192FFC_OFF(firstResource, 0x14);
OWNEDRESOURCES80192FFC_OFF(secondResource, 0x1c);
#undef OWNEDRESOURCES80192FFC_OFF

#endif  // CLASSES_OWNEDRESOURCES80192FFC_H
