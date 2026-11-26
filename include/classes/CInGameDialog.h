#ifndef CLASSES_CINGAMEDIALOG_H
#define CLASSES_CINGAMEDIALOG_H

#include "game/types.h"

struct InGameDialogView;
struct DialogOpenStateView;
struct DialogInactiveStateView;
class CInGameDialog {
public:
    void* vtbl;                       // 0x0: original vtable pointer
    void* callbackVtable;             // 0x4
    char pad_008[0x10];               // 0x8
    unsigned int state;               // 0x18: DialogTransitionState
    char pad_01C[0x1C];               // 0x1c
    int blockRegistryClose;           // 0x38: nonzero blocks the registry close sweep
    char pad_03C[0x4];                // 0x3c
    unsigned char stringStorageFlags; // 0x40
    char pad_041[0x3];                // 0x41
    int stringLength;                 // 0x44

    static int Init(InGameDialogView* p);
    static int RequestOpen(InGameDialogView* p, int animate);
    static int RequestClose(InGameDialogView* p, int animate);
    static int UpdateTransition(InGameDialogView* p, float dt);
    int ResetDialog();
    static int CompleteClose(DialogInactiveStateView* arg);
    static void CompleteOpen(DialogOpenStateView* arg);
    void StartOpening();
    void StartClosing();
    static int CanOpen();
    static int CanClose();
    static void OnFrame();
    static int IsReadyForUpdate();

    // Dialog state queries.
    bool IsInactive();
    bool IsOpenOrTransitioning();
};

#define CINGAMEDIALOG_OFF(f, o) typedef char CInGameDialog_off_##f[((int)&((CInGameDialog*)0)->f == (o)) ? 1 : -1]
CINGAMEDIALOG_OFF(callbackVtable, 0x4);
CINGAMEDIALOG_OFF(state, 0x18);
CINGAMEDIALOG_OFF(blockRegistryClose, 0x38);
CINGAMEDIALOG_OFF(stringStorageFlags, 0x40);
CINGAMEDIALOG_OFF(stringLength, 0x44);
#undef CINGAMEDIALOG_OFF

#endif // CLASSES_CINGAMEDIALOG_H
