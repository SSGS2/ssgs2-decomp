#ifndef CLASSES_CBGMCOLLECTIONTASK_H
#define CLASSES_CBGMCOLLECTIONTASK_H

#include "game/types.h"

class CBgmCollectionTask {
public:
    void* vtbl;                     // 0x0: original vtable pointer
    void* interfaceVtable;          // 0x4: CBgmCollectionTask_Ctor installs vtable + 0x10
    void* secondaryInterfaceVtable; // 0x8: CBgmCollectionTask_Ctor installs vtable + 0x1c

    static char* GetRTTI();

    static void StartActorLoading(void* taskBase);
    static void ProcessActors(void* taskBase, float forwarded1);
    static void DisplayActorsByPriority(void* taskBase);
    CBgmCollectionTask();
    ~CBgmCollectionTask();
    void CreateActors();
    void DtorThunk8(int deleting);
};

#define CBGMCOLLECTIONTASK_OFF(f, o) \
    typedef char CBgmCollectionTask_off_##f[((int)&((CBgmCollectionTask*)0)->f == (o)) ? 1 : -1]
CBGMCOLLECTIONTASK_OFF(interfaceVtable, 0x4);
CBGMCOLLECTIONTASK_OFF(secondaryInterfaceVtable, 0x8);
#undef CBGMCOLLECTIONTASK_OFF

#endif // CLASSES_CBGMCOLLECTIONTASK_H
