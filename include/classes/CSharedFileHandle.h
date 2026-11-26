#ifndef CLASSES_CSHAREDFILEHANDLE_H
#define CLASSES_CSHAREDFILEHANDLE_H

#include "game/types.h"

// The data layout of CSharedFileHandle has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CSharedFileHandle {
public:
    static void* Destroy(void* p, int flag);
};

#endif // CLASSES_CSHAREDFILEHANDLE_H
