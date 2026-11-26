#ifndef CLASSES_CSHOPBASEE_H
#define CLASSES_CSHOPBASEE_H

#include "game/types.h"

struct CShopBaseE {
    CShopBaseE(void*);
    ~CShopBaseE();
    void** vtable;
    void v1C(void* p) {
        ((ObjCtorVt*)this)->init(p);
    }
};

#endif // CLASSES_CSHOPBASEE_H
