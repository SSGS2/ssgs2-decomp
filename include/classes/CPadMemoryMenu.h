#ifndef CLASSES_CPADMEMORYMENU_H
#define CLASSES_CPADMEMORYMENU_H

#include "game/types.h"

class CPadMemoryMenu {
public:
    void* vtbl;            // 0x0: original vtable pointer
    char pad_004[0x14];    // 0x4
    void* fileIoAccess;    // 0x18
    char pad_01C[0xc];     // 0x1c
    void* saveForm;        // 0x28
    char pad_02C[0x14];    // 0x2c
    void* controllerIndex; // 0x40

    int HasScreenPointer();
    int IsDeviceDisconnected();
    int CopyValidSaveData(void* destination);
    int HasFileErrorC73D();
    int CreateReadFileIo();
    int CreateWriteFileIo();
    int VerifyWrittenRecord();

    static int SetCurrentSharedContext(void* arg);
    int HasStoredTitle();
    int StoredTitleMatchesGame();

    void* Init(void* value);
    ~CPadMemoryMenu();
};

#define CPADMEMORYMENU_OFF(f, o) typedef char CPadMemoryMenu_off_##f[((int)&((CPadMemoryMenu*)0)->f == (o)) ? 1 : -1]
CPADMEMORYMENU_OFF(fileIoAccess, 0x18);
CPADMEMORYMENU_OFF(saveForm, 0x28);
CPADMEMORYMENU_OFF(controllerIndex, 0x40);
#undef CPADMEMORYMENU_OFF

#endif // CLASSES_CPADMEMORYMENU_H
