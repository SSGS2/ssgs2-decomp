#ifndef CLASSES_CNODE_H
#define CLASSES_CNODE_H

#include "game/types.h"

// The data layout of CNode has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CNode {
public:
    void* GetNext();
};

#endif // CLASSES_CNODE_H
