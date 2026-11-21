// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPADMEMORYMENULOAD_H
#define CLASSES_CPADMEMORYMENULOAD_H

#include "game/types.h"

class CPadMemoryMenuLoad {
public:
    explicit CPadMemoryMenuLoad(void* controller);
    void* vtable;                        // 0x0: 1 use in CPadMemoryMenuLoad_dtor
    char pad_004[0x28];                 // 0x4
    int bpEntryId;                         // 0x2c: 1 use in CFileIoMenu_StartSavingScript
    void* bpObject;                       // 0x30: 1 use in CFileIoMenu_StartSavingScript
    char pad_034[0x10];                 // 0x34
    int loadState;                         // 0x44: 22 uses in CPadMemoryMenuLoad_AdvanceLoadState, CPadMemoryMenuLoad_vfunc_20, CPadMemoryMenuLoad_ProcessAndReportComplete
    void* saveData;                       // 0x48: 5 uses in CPadMemoryMenuLoad_AdvanceLoadState, CPadMemoryMenuLoad_dtor
    int loadResult;                         // 0x4c: 3 uses in CPadMemoryMenuLoad_AdvanceLoadState
    unsigned int loadRetryCount;                // 0x50: 3 uses in CPadMemoryMenuLoad_AdvanceLoadState
    unsigned int recoveryRetryCount;                // 0x54: 2 uses in CPadMemoryMenuLoad_AdvanceLoadState
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPADMEMORYMENULOAD_OFF(f, o) typedef char CPadMemoryMenuLoad_off_##f[((int)&((CPadMemoryMenuLoad*)0)->f == (o)) ? 1 : -1]
CPADMEMORYMENULOAD_OFF(bpEntryId, 0x2c);
CPADMEMORYMENULOAD_OFF(bpObject, 0x30);
CPADMEMORYMENULOAD_OFF(loadState, 0x44);
CPADMEMORYMENULOAD_OFF(saveData, 0x48);
CPADMEMORYMENULOAD_OFF(loadResult, 0x4c);
CPADMEMORYMENULOAD_OFF(loadRetryCount, 0x50);
CPADMEMORYMENULOAD_OFF(recoveryRetryCount, 0x54);
#undef CPADMEMORYMENULOAD_OFF

#endif  // CLASSES_CPADMEMORYMENULOAD_H
