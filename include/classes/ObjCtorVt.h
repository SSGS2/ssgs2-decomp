#ifndef CLASSES_OBJCTORVT_H
#define CLASSES_OBJCTORVT_H

// Vtable view used by the shop category bases. The init hook is the sixth slot (0x14).
struct ObjCtorVt {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void init(void* arg);
};

#endif // CLASSES_OBJCTORVT_H
