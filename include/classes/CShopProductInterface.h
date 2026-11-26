#ifndef CLASSES_CSHOPPRODUCTINTERFACE_H
#define CLASSES_CSHOPPRODUCTINTERFACE_H

#include "game/types.h"

// The data layout of CShopProductInterface has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CShopProductInterface {
public:
    void* Destroy(int shouldDelete);
    int vfunc_1();

    void* vtbl; // 0x00
};

#endif // CLASSES_CSHOPPRODUCTINTERFACE_H
