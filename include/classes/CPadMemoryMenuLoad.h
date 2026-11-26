#ifndef CLASSES_CPADMEMORYMENULOAD_H
#define CLASSES_CPADMEMORYMENULOAD_H

#include "game/types.h"

class CPadMemoryMenuLoad {
public:
    explicit CPadMemoryMenuLoad(void* controller);
    void* vtable;                    // 0x0
    char pad_004[0x28];              // 0x4
    int bpEntryId;                   // 0x2c
    void* bpObject;                  // 0x30
    char pad_034[0x10];              // 0x34
    int loadState;                   // 0x44
    void* saveData;                  // 0x48
    int loadResult;                  // 0x4c
    unsigned int loadRetryCount;     // 0x50
    unsigned int recoveryRetryCount; // 0x54

    static int vfunc_8(void* arg);
    static int Draw(void* arg);
    int IsComplete();
    int CopySaveData(void* destination);
    int ProcessAndReportComplete();
    static void* Create(void* controller);
    static int AdvanceLoadState(void* arg);

    ~CPadMemoryMenuLoad();
};

#define CPADMEMORYMENULOAD_OFF(f, o) \
    typedef char CPadMemoryMenuLoad_off_##f[((int)&((CPadMemoryMenuLoad*)0)->f == (o)) ? 1 : -1]
CPADMEMORYMENULOAD_OFF(bpEntryId, 0x2c);
CPADMEMORYMENULOAD_OFF(bpObject, 0x30);
CPADMEMORYMENULOAD_OFF(loadState, 0x44);
CPADMEMORYMENULOAD_OFF(saveData, 0x48);
CPADMEMORYMENULOAD_OFF(loadResult, 0x4c);
CPADMEMORYMENULOAD_OFF(loadRetryCount, 0x50);
CPADMEMORYMENULOAD_OFF(recoveryRetryCount, 0x54);
#undef CPADMEMORYMENULOAD_OFF

#endif // CLASSES_CPADMEMORYMENULOAD_H
