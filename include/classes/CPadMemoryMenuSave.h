#ifndef CLASSES_CPADMEMORYMENUSAVE_H
#define CLASSES_CPADMEMORYMENUSAVE_H

#include "game/types.h"

struct PadSaveCompletionView;
struct PadSaveStateView;
class CPadMemoryMenuSave {
public:
    explicit CPadMemoryMenuSave(void* controller);
    void* vtbl;          // 0x0: original vtable pointer
    char pad_004[0x3c];  // 0x4
    void* sharedContext; // 0x40
    unsigned int
        saveState; // 0x44: CPadMemoryMenuSave_Create zeroes; vfunc_9 switches over save operation states (pad_memory_menu_save.cpp)
    unsigned int retryCount; // 0x48: vfunc_9 retries writes while count < 3 (pad_memory_menu_save.cpp)
    void* saveData;          // 0x4c

    static int UpdateAndReportSaveComplete(PadSaveCompletionView* arg);
    static void Draw(void* forwarded0);
    static int IsSaveComplete(PadSaveStateView* arg);
    int GetSaveRecord();
    int AdvanceSaveStateMachine();
    void* Create(void* arg);
    int InitSaveRecordAndFade(void* arg);

    ~CPadMemoryMenuSave();
};

#define CPADMEMORYMENUSAVE_OFF(f, o) \
    typedef char CPadMemoryMenuSave_off_##f[((int)&((CPadMemoryMenuSave*)0)->f == (o)) ? 1 : -1]
CPADMEMORYMENUSAVE_OFF(sharedContext, 0x40);
CPADMEMORYMENUSAVE_OFF(saveState, 0x44);
CPADMEMORYMENUSAVE_OFF(retryCount, 0x48);
CPADMEMORYMENUSAVE_OFF(saveData, 0x4c);
#undef CPADMEMORYMENUSAVE_OFF

#endif // CLASSES_CPADMEMORYMENUSAVE_H
