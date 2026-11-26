#ifndef CLASSES_CBGMCOLLECTIONMAIN_H
#define CLASSES_CBGMCOLLECTIONMAIN_H

#include "game/types.h"

class CBgmCollectionMain {
public:
    void* vtbl;           // 0x0: original vtable pointer
    char pad_004[0x14];   // 0x4
    void* taskMainVtable; // 0x18
    void* collection;     // 0x1c

    static void OnPreLoadInit();
    static void OnInit();
    static char* GetRTTI();
    void OnDestroy();
    void OnLoad();
    void OnProcess();
    void OnDisplay();
    void DtorThunk18(int deleting);

    CBgmCollectionMain();
    ~CBgmCollectionMain();
};

#define CBGMCOLLECTIONMAIN_OFF(f, o) \
    typedef char CBgmCollectionMain_off_##f[((int)&((CBgmCollectionMain*)0)->f == (o)) ? 1 : -1]
CBGMCOLLECTIONMAIN_OFF(taskMainVtable, 0x18);
CBGMCOLLECTIONMAIN_OFF(collection, 0x1c);
#undef CBGMCOLLECTIONMAIN_OFF

#endif // CLASSES_CBGMCOLLECTIONMAIN_H
