#ifndef CLASSES_CSHAREDFILEHANDLEOWNER_H
#define CLASSES_CSHAREDFILEHANDLEOWNER_H

#include "game/types.h"

class CSharedFileHandleOwner {
public:
    void* vtbl;          // 0x0: original vtable pointer
    void* sharedFilePtr; // 0x4

    int IsLoaded();

    CSharedFileHandleOwner(void* value);
    ~CSharedFileHandleOwner();
};

#define CSHAREDFILEHANDLEOWNER_OFF(f, o) \
    typedef char CSharedFileHandleOwner_off_##f[((int)&((CSharedFileHandleOwner*)0)->f == (o)) ? 1 : -1]
CSHAREDFILEHANDLEOWNER_OFF(sharedFilePtr, 0x4);
#undef CSHAREDFILEHANDLEOWNER_OFF

#endif // CLASSES_CSHAREDFILEHANDLEOWNER_H
