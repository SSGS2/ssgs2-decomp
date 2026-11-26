#ifndef CLASSES_CSHOPBASED_H
#define CLASSES_CSHOPBASED_H

#include "game/types.h"
#include "classes/ObjCtorVt.h"

struct CShopBaseD {
    CShopBaseD(void*);
    ~CShopBaseD();
    void** vtable;
    void v1C(void* p) {
        ((ObjCtorVt*)this)->init(p);
    }
};

#endif // CLASSES_CSHOPBASED_H
