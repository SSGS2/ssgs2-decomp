#ifndef CLASSES_CSHOPBASEC_H
#define CLASSES_CSHOPBASEC_H

#include "game/types.h"
#include "classes/ObjCtorVt.h"

struct CShopBaseC {
    CShopBaseC(void*);
    ~CShopBaseC();
    void** vtable;
    void v1C(void* p) {
        ((ObjCtorVt*)this)->init(p);
    }
};

#endif // CLASSES_CSHOPBASEC_H
