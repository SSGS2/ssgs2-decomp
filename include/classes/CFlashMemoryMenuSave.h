#ifndef CLASSES_CFLASHMEMORYMENUSAVE_H
#define CLASSES_CFLASHMEMORYMENUSAVE_H

#include "game/types.h"

struct FlashSaveStateView;
class CFlashMemoryMenuSave {
public:
    CFlashMemoryMenuSave();        // Defined inline in flash_memory_menu_save.cpp.
    void* vtbl;                    // 0x0: original vtable pointer
    char pad_004[0x44];            // 0x4
    unsigned int saveState;        // 0x48
    unsigned int retryCount;       // 0x4c
    unsigned int accessRetryCount; // 0x50

    void vfunc_3();
    static int IsSaveComplete(FlashSaveStateView* arg);
    static void Draw(void* arg);
    static void* Create();
    int UpdateSaveStateMachine(float delta);
    static void StartFadeIn(void* unused, int enabled);
    int EnableAlternateFadeDraw();

    ~CFlashMemoryMenuSave();
};

#define CFLASHMEMORYMENUSAVE_OFF(f, o) \
    typedef char CFlashMemoryMenuSave_off_##f[((int)&((CFlashMemoryMenuSave*)0)->f == (o)) ? 1 : -1]
CFLASHMEMORYMENUSAVE_OFF(saveState, 0x48);
CFLASHMEMORYMENUSAVE_OFF(retryCount, 0x4c);
CFLASHMEMORYMENUSAVE_OFF(accessRetryCount, 0x50);
#undef CFLASHMEMORYMENUSAVE_OFF

#endif // CLASSES_CFLASHMEMORYMENUSAVE_H
