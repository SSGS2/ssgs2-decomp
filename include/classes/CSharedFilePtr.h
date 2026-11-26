#ifndef CLASSES_CSHAREDFILEPTR_H
#define CLASSES_CSHAREDFILEPTR_H

#include "game/types.h"

// The data layout of CSharedFilePtr has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CSharedFilePtr {
public:
    static int GetSize(void* a);
};

#endif // CLASSES_CSHAREDFILEPTR_H
