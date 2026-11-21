// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCHECKDRIVESTATUS_H
#define CLASSES_CCHECKDRIVESTATUS_H

#include "game/types.h"

class CCheckDriveStatus {
public:
    int statusTextMapCount;                          // 0x0: 1 use in CCheckDriveStatus_Ctor
    int statusTextMapRoot;                          // 0x4: 1 use in CCheckDriveStatus_Ctor
    char pad_008[0x4];                  // 0x8
    void* statusTextMapSentinel;                        // 0xc: 1 use in CCheckDriveStatus_Ctor
    void* diskErrorDialog;                       // 0x10: 7 uses in CCheckDriveStatus_dtor, CCheckDriveStatus_Ctor
    unsigned int statusTextFlags;                // 0x14: 7 uses in CCheckDriveStatus_Ctor, CCheckDriveStatus_dtor (CONFLICT sizes [1, 4])
    int statusTextLength;                         // 0x18: 4 uses in CCheckDriveStatus_Ctor, CCheckDriveStatus_dtor
    void* statusTextData;                       // 0x1c: 5 uses in CCheckDriveStatus_dtor, CCheckDriveStatus_Ctor
    int threadActive;                         // 0x20: 3 uses in CCheckDriveStatus_dtor, CCheckDriveStatus_Ctor
    int unk_24;                         // 0x24: 2 uses in CCheckDriveStatus_Ctor, CCheckDriveStatus_dtor
    int errorDialogsEnabled;                         // 0x28: 1 use in CCheckDriveStatus_Ctor
    int skipDialogDelay;                         // 0x2c: 1 use in CCheckDriveStatus_Ctor
    int dialogCloseRequested;                         // 0x30: 1 use in CCheckDriveStatus_Ctor
    int dialogDelayMilliseconds;                         // 0x34: 1 use in CCheckDriveStatus_Ctor
    char pad_038[0x318];                // 0x38
    int suspendedThread;                        // 0x350: 2 uses in CCheckDriveStatus_Ctor, CCheckDriveStatus_dtor
    unsigned char savedHomeMenuBlocked;              // 0x354: 1 use in CCheckDriveStatus_Ctor
    char pad_355[0x3];                  // 0x355
    void* vtable;                      // 0x358: 2 uses in CCheckDriveStatus_Ctor, CCheckDriveStatus_dtor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x14, 0x38
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCHECKDRIVESTATUS_OFF(f, o) typedef char CCheckDriveStatus_off_##f[((int)&((CCheckDriveStatus*)0)->f == (o)) ? 1 : -1]
CCHECKDRIVESTATUS_OFF(statusTextMapRoot, 0x4);
CCHECKDRIVESTATUS_OFF(statusTextMapSentinel, 0xc);
CCHECKDRIVESTATUS_OFF(diskErrorDialog, 0x10);
CCHECKDRIVESTATUS_OFF(statusTextFlags, 0x14);
CCHECKDRIVESTATUS_OFF(statusTextLength, 0x18);
CCHECKDRIVESTATUS_OFF(statusTextData, 0x1c);
CCHECKDRIVESTATUS_OFF(threadActive, 0x20);
CCHECKDRIVESTATUS_OFF(unk_24, 0x24);
CCHECKDRIVESTATUS_OFF(errorDialogsEnabled, 0x28);
CCHECKDRIVESTATUS_OFF(skipDialogDelay, 0x2c);
CCHECKDRIVESTATUS_OFF(dialogCloseRequested, 0x30);
CCHECKDRIVESTATUS_OFF(dialogDelayMilliseconds, 0x34);
CCHECKDRIVESTATUS_OFF(suspendedThread, 0x350);
CCHECKDRIVESTATUS_OFF(savedHomeMenuBlocked, 0x354);
CCHECKDRIVESTATUS_OFF(vtable, 0x358);
#undef CCHECKDRIVESTATUS_OFF

#endif  // CLASSES_CCHECKDRIVESTATUS_H
