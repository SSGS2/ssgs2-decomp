#ifndef CLASSES_CSHOPBASEA_H
#define CLASSES_CSHOPBASEA_H

#include "game/types.h"

struct CShopBaseA {
    CShopBaseA(void*, void*);
    ~CShopBaseA();
    void** vtable;
    void v1C(void* p) {
        ((ObjCtorVt*)this)->init(p);
    }
};

#endif // CLASSES_CSHOPBASEA_H
