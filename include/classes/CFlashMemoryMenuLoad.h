// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFLASHMEMORYMENULOAD_H
#define CLASSES_CFLASHMEMORYMENULOAD_H

#include "game/types.h"

class CFlashMemoryMenuLoad {
public:
    explicit CFlashMemoryMenuLoad(int skipAccountSelection);
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x44];                 // 0x4
    int state;                        // 0x48: CFlashMemoryMenuLoad_Create zeroes
    int createAccountMenu;                        // 0x4c: CFlashMemoryMenuLoad_Create zeroes
    int nameEntryMenu;                        // 0x50: CFlashMemoryMenuLoad_Create zeroes
    unsigned char aborted;               // 0x54: 2 uses in CFlashMemoryMenuLoad_vfunc_1, CFlashMemoryMenuLoad_UpdateLoadStateMachine
    unsigned char createdNewSave;              // 0x55: CFlashMemoryMenuLoad_Create zeroes
    unsigned char skipAccountSelection;              // 0x56: CFlashMemoryMenuLoad_Create copies its argument byte
    unsigned char deletingAccount;              // 0x57: CFlashMemoryMenuLoad_Create zeroes
    unsigned char storageCheckFailed;              // 0x58: CFlashMemoryMenuLoad_Create zeroes
    unsigned char invalidSaveData;              // 0x59: CFlashMemoryMenuLoad_Create zeroes
    char pad_05A[0x2];                 // 0x5a
    void* entries;                       // 0x5c: 11 uses in CFlashMemoryMenuLoad_UpdateLoadStateMachine, CFlashMemoryMenuLoad_dtor
    unsigned int entryCount;                // 0x60: 7 uses in CFlashMemoryMenuLoad_dtor, CFlashMemoryMenuLoad_UpdateLoadStateMachine
    unsigned int entryCapacity;        // 0x64: vfunc_8 V8Vec::cap grows the entries vector
    int loadDataMenu;                        // 0x68: CFlashMemoryMenuLoad_Create zeroes

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4c, 0x50, 0x5c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFLASHMEMORYMENULOAD_OFF(f, o) typedef char CFlashMemoryMenuLoad_off_##f[((int)&((CFlashMemoryMenuLoad*)0)->f == (o)) ? 1 : -1]
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

#endif  // CLASSES_CFLASHMEMORYMENULOAD_H
