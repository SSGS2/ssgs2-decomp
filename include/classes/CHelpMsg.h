#ifndef CLASSES_CHELPMSG_H
#define CLASSES_CHELPMSG_H

#include "game/types.h"

// The data layout of CHelpMsg has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CHelpMsg {
public:
    CHelpMsg();
    ~CHelpMsg();
};

#endif // CLASSES_CHELPMSG_H
