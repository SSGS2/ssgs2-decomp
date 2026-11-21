// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFLASHMEMORYMENUSAVE_H
#define CLASSES_CFLASHMEMORYMENUSAVE_H

#include "game/types.h"

class CFlashMemoryMenuSave {
public:
    CFlashMemoryMenuSave(); // Defined inline in flash_memory_menu_save.cpp.
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x44];                 // 0x4
    unsigned int saveState;                // 0x48: 1 use in CFlashMemoryMenuSave_UpdateSaveStateMachine
    unsigned int retryCount;                // 0x4c: 11 uses in CFlashMemoryMenuSave_UpdateSaveStateMachine
    unsigned int accessRetryCount;                // 0x50: 4 uses in CFlashMemoryMenuSave_UpdateSaveStateMachine
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFLASHMEMORYMENUSAVE_OFF(f, o) typedef char CFlashMemoryMenuSave_off_##f[((int)&((CFlashMemoryMenuSave*)0)->f == (o)) ? 1 : -1]
CFLASHMEMORYMENUSAVE_OFF(saveState, 0x48);
CFLASHMEMORYMENUSAVE_OFF(retryCount, 0x4c);
CFLASHMEMORYMENUSAVE_OFF(accessRetryCount, 0x50);
#undef CFLASHMEMORYMENUSAVE_OFF

#endif  // CLASSES_CFLASHMEMORYMENUSAVE_H
