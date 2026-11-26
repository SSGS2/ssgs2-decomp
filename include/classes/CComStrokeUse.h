#ifndef CLASSES_CCOMSTROKEUSE_H
#define CLASSES_CCOMSTROKEUSE_H

#include "game/types.h"

// The data layout of CComStrokeUse has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CComStrokeUse {
public:
    int RequestFileLoad(void* arg);
    int IsFileLoaded();

    CComStrokeUse();
    ~CComStrokeUse();
};

#endif // CLASSES_CCOMSTROKEUSE_H
