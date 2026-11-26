#ifndef CLASSES_CPROJECTBASE_H
#define CLASSES_CPROJECTBASE_H

#include "game/types.h"

// The data layout of CProjectBase has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CProjectBase {
public:
    ~CProjectBase();
};

#endif // CLASSES_CPROJECTBASE_H
