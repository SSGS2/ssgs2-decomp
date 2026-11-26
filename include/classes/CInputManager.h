// CInputManager: explicit layout view; no inheritance, virtual methods, or RTTI.
// Layout view for CInputManager_dtor (0x8018687C) and its lock/list helpers.
#ifndef CLASSES_CINPUTMANAGER_H
#define CLASSES_CINPUTMANAGER_H

#include "game/types.h"

class CInputManager {
public:
    void* vtbl; // 0x0: CInputManager_dtor: __vt__13CInputManager
    char pad_004[0xc];
    void* activeHead;  // 0x10: inputFirst: start of active list
    void* currentNode; // 0x14: inputNext: traversal cursor
    void* pendingHead; // 0x18: inputDestroyList: list drained and freed
    void* freeHead;    // 0x1C: CInputManager_dtor: recycled list nodes
    char pad_020[0x8];
    int slotCount;    // 0x28: CInputManager_dtor: pointer slots to clear
    void** slots;     // 0x2C: CInputManager_dtor: freed pointer array
    char mutex[0x18]; // 0x30: inputFirst/inputNext/inputDestroyList: OSLockMutex/OSUnlockMutex lock

    static int HasAnyMonitoredAction(void* inputManager);

    ~CInputManager();
};

#define OFF(f, o) typedef char CInputManager_off_##f[((int)&((CInputManager*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(activeHead, 0x10);
OFF(currentNode, 0x14);
OFF(pendingHead, 0x18);
OFF(freeHead, 0x1C);
OFF(slotCount, 0x28);
OFF(slots, 0x2C);
OFF(mutex, 0x30);
#undef OFF

#endif
