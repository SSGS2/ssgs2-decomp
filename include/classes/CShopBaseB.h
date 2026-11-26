#ifndef CLASSES_CSHOPBASEB_H
#define CLASSES_CSHOPBASEB_H

#include "game/types.h"
#include "classes/ObjCtorVt.h"

struct CShopBaseB {
    CShopBaseB(void*, void*);
    ~CShopBaseB();
    void** vtable;
    void v1C(void* p) {
        ((ObjCtorVt*)this)->init(p);
    }
};

#endif // CLASSES_CSHOPBASEB_H
