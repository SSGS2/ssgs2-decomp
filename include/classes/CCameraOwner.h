// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCAMERAOWNER_H
#define CLASSES_CCAMERAOWNER_H

#include "game/types.h"

class CCameraOwner {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x18];                 // 0x4
    void* modelPreview;                       // 0x1c: 4 uses in CCameraOwner_Dtor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCAMERAOWNER_OFF(f, o) typedef char CCameraOwner_off_##f[((int)&((CCameraOwner*)0)->f == (o)) ? 1 : -1]
CCAMERAOWNER_OFF(modelPreview, 0x1c);
#undef CCAMERAOWNER_OFF

#endif  // CLASSES_CCAMERAOWNER_H
