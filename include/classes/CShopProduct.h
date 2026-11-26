#ifndef CLASSES_CSHOPPRODUCT_H
#define CLASSES_CSHOPPRODUCT_H

#include "game/types.h"

// The data layout of CShopProduct has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CShopProduct {
public:
    static int DefaultUnsupportedHookA();
    static int DefaultUnsupportedHookB();
};

#endif // CLASSES_CSHOPPRODUCT_H
