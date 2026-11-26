#ifndef CLASSES_CNODERECORDVECTOR_H
#define CLASSES_CNODERECORDVECTOR_H

#include "game/types.h"

// The data layout of CNodeRecordVector has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CNodeRecordVector {
public:
    static void GrowAndAppend(void* object, void* value);
};

#endif // CLASSES_CNODERECORDVECTOR_H
