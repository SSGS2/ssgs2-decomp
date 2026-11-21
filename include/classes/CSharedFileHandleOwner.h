// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHAREDFILEHANDLEOWNER_H
#define CLASSES_CSHAREDFILEHANDLEOWNER_H

#include "game/types.h"

class CSharedFileHandleOwner {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* sharedFilePtr;                        // 0x4: 2 uses in CSharedFileHandleOwner_Dtor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHAREDFILEHANDLEOWNER_OFF(f, o) typedef char CSharedFileHandleOwner_off_##f[((int)&((CSharedFileHandleOwner*)0)->f == (o)) ? 1 : -1]
CSHAREDFILEHANDLEOWNER_OFF(sharedFilePtr, 0x4);
#undef CSHAREDFILEHANDLEOWNER_OFF

#endif  // CLASSES_CSHAREDFILEHANDLEOWNER_H
