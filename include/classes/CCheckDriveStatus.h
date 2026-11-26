#ifndef CLASSES_CCHECKDRIVESTATUS_H
#define CLASSES_CCHECKDRIVESTATUS_H

#include "game/types.h"

class CCheckDriveStatus {
public:
    int statusTextMapCount;             // 0x0
    int statusTextMapRoot;              // 0x4
    char pad_008[0x4];                  // 0x8
    void* statusTextMapSentinel;        // 0xc
    void* diskErrorDialog;              // 0x10
    unsigned int statusTextFlags;       // 0x14
    int statusTextLength;               // 0x18
    void* statusTextData;               // 0x1c
    int threadActive;                   // 0x20
    int latched;                        // 0x24
    int errorDialogsEnabled;            // 0x28
    int skipDialogDelay;                // 0x2c
    int dialogCloseRequested;           // 0x30
    int dialogDelayMilliseconds;        // 0x34
    char displayThread[0x318];          // 0x38
    int suspendedThread;                // 0x350
    unsigned char savedHomeMenuBlocked; // 0x354
    char pad_355[0x3];                  // 0x355
    void* vtable;                       // 0x358

    static int UpdateErrorDialog(void* selfv);
    static void SetLatch();
    static void StopThread();
    static void StartThread();
    static void SetErrorDialogsEnabled(int enabled);
    static void SetSkipDialogDelay(int skip);

    CCheckDriveStatus();
};

#define CCHECKDRIVESTATUS_OFF(f, o) \
    typedef char CCheckDriveStatus_off_##f[((int)&((CCheckDriveStatus*)0)->f == (o)) ? 1 : -1]
CCHECKDRIVESTATUS_OFF(statusTextMapRoot, 0x4);
CCHECKDRIVESTATUS_OFF(statusTextMapSentinel, 0xc);
CCHECKDRIVESTATUS_OFF(diskErrorDialog, 0x10);
CCHECKDRIVESTATUS_OFF(statusTextFlags, 0x14);
CCHECKDRIVESTATUS_OFF(statusTextLength, 0x18);
CCHECKDRIVESTATUS_OFF(statusTextData, 0x1c);
CCHECKDRIVESTATUS_OFF(threadActive, 0x20);
CCHECKDRIVESTATUS_OFF(latched, 0x24);
CCHECKDRIVESTATUS_OFF(errorDialogsEnabled, 0x28);
CCHECKDRIVESTATUS_OFF(skipDialogDelay, 0x2c);
CCHECKDRIVESTATUS_OFF(dialogCloseRequested, 0x30);
CCHECKDRIVESTATUS_OFF(dialogDelayMilliseconds, 0x34);
CCHECKDRIVESTATUS_OFF(suspendedThread, 0x350);
CCHECKDRIVESTATUS_OFF(savedHomeMenuBlocked, 0x354);
CCHECKDRIVESTATUS_OFF(vtable, 0x358);
#undef CCHECKDRIVESTATUS_OFF

#endif // CLASSES_CCHECKDRIVESTATUS_H
