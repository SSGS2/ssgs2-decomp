// The original vtable (__vt__17CFileIoAccessRead) and typeinfo stay data symbols.
#ifndef CLASSES_CFILEIOACCESSREAD_H
#define CLASSES_CFILEIOACCESSREAD_H

#include "classes/CFileIoAccess.h"

class CFileIoAccessRead : public CFileIoAccess {
public:
    CFileIoAccessRead();

    int state; // 0x0C: read state machine step (CFileIoAccessRead_Update), 4 when finished

    // 0x0C (vtable slot 3): deleting destructor; keeps the object alive when flag <= 0
    void* destroy(int flag);

    int vfunc_2();
    static int Update(void* access);
};

#endif // CLASSES_CFILEIOACCESSREAD_H
