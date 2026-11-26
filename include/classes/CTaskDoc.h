#ifndef CLASSES_CTASKDOC_H
#define CLASSES_CTASKDOC_H

#include "game/types.h"

// The data layout of CTaskDoc has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CTaskDoc {
public:
    static void* GetRTTI();

    ~CTaskDoc();
    CTaskDoc();
};

#endif // CLASSES_CTASKDOC_H
