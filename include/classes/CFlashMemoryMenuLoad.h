#ifndef CLASSES_CFLASHMEMORYMENULOAD_H
#define CLASSES_CFLASHMEMORYMENULOAD_H

#include "game/types.h"

class CFlashMemoryMenuLoad {
public:
    explicit CFlashMemoryMenuLoad(int skipAccountSelection);
    ~CFlashMemoryMenuLoad();
    static void* Create(int skipAccountSelection);
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x44];                 // 0x4
    int state;                          // 0x48: CFlashMemoryMenuLoad::Create zeroes
    int createAccountMenu;              // 0x4c: CFlashMemoryMenuLoad::Create zeroes
    int nameEntryMenu;                  // 0x50: CFlashMemoryMenuLoad::Create zeroes
    unsigned char aborted;              // 0x54
    unsigned char createdNewSave;       // 0x55: CFlashMemoryMenuLoad::Create zeroes
    unsigned char skipAccountSelection; // 0x56: CFlashMemoryMenuLoad::Create copies its argument byte
    unsigned char deletingAccount;      // 0x57: CFlashMemoryMenuLoad::Create zeroes
    unsigned char storageCheckFailed;   // 0x58: CFlashMemoryMenuLoad::Create zeroes
    unsigned char invalidSaveData;      // 0x59: CFlashMemoryMenuLoad::Create zeroes
    char pad_05A[0x2];                  // 0x5a
    void* entries;                      // 0x5c
    unsigned int entryCount;            // 0x60
    unsigned int entryCapacity;         // 0x64: vfunc_8 V8Vec::cap grows the entries vector
    int loadDataMenu;                   // 0x68: CFlashMemoryMenuLoad::Create zeroes

    unsigned char IsAborted();
    void StartFadeIn(int flag);
    static int InitializeMenuAndEntries(void* param);

    int IsComplete();
    void DrawDialogAndSubmenus();
    int ProcessSelectionDialog(float dt);
    int UpdateLoadStateMachine(float dt);
};

#define CFLASHMEMORYMENULOAD_OFF(f, o) \
    typedef char CFlashMemoryMenuLoad_off_##f[((int)&((CFlashMemoryMenuLoad*)0)->f == (o)) ? 1 : -1]
CFLASHMEMORYMENULOAD_OFF(aborted, 0x54);
CFLASHMEMORYMENULOAD_OFF(state, 0x48);
CFLASHMEMORYMENULOAD_OFF(createAccountMenu, 0x4c);
CFLASHMEMORYMENULOAD_OFF(nameEntryMenu, 0x50);
CFLASHMEMORYMENULOAD_OFF(createdNewSave, 0x55);
CFLASHMEMORYMENULOAD_OFF(skipAccountSelection, 0x56);
CFLASHMEMORYMENULOAD_OFF(deletingAccount, 0x57);
CFLASHMEMORYMENULOAD_OFF(storageCheckFailed, 0x58);
CFLASHMEMORYMENULOAD_OFF(invalidSaveData, 0x59);
CFLASHMEMORYMENULOAD_OFF(entries, 0x5c);
CFLASHMEMORYMENULOAD_OFF(entryCount, 0x60);
CFLASHMEMORYMENULOAD_OFF(entryCapacity, 0x64);
CFLASHMEMORYMENULOAD_OFF(loadDataMenu, 0x68);
#undef CFLASHMEMORYMENULOAD_OFF

#endif // CLASSES_CFLASHMEMORYMENULOAD_H
