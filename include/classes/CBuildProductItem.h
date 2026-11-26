#ifndef CLASSES_CBUILDPRODUCTITEM_H
#define CLASSES_CBUILDPRODUCTITEM_H

#include "game/types.h"

// The data layout of CBuildProductItem has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CBuildProductItem {
public:
    CBuildProductItem(int count, unsigned char flag);
    ~CBuildProductItem();
};

#endif // CLASSES_CBUILDPRODUCTITEM_H
