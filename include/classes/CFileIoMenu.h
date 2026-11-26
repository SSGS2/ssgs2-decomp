#ifndef CLASSES_CFILEIOMENU_H
#define CLASSES_CFILEIOMENU_H

#include "game/types.h"

struct CFileIoMenuView;
struct FileIoMenuCalls;
class CFileIoMenu {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* fileAccess;                   // 0x18
    char pad_01C[0x4];                  // 0x1c
    void* commonDialog;                 // 0x20: owned CPangyaCommonDialog
    unsigned char commonDialogComplete; // 0x24
    char pad_025[0x3];
    void* saveForm;                   // 0x28
    int bpEntryId;                    // 0x2C
    void* bpObject;                   // 0x30: borrowed from bpEntryId; drawn and scripted by the menu
    unsigned int messageStringFlags;  // 0x34: high bit set when messageStringData is owned
    unsigned int messageStringLength; // 0x38
    void* messageStringData;          // 0x3C

    // Per-frame work and drawing.
    int Update();
    void Draw();
    int vfunc_8();

    // Common dialogs.
    int ShowAcknowledgeDialog(const void* value);
    int ShowOkCancelDialog(const void* value);
    int CreateButtonlessCommonDialog(void* value);
    int DestroyCommonDialog();
    int IsCommonDialogComplete();
    int IsCommonDialogAccepted();

    // Access object and save form queries.
    int QueryFileAccessStatus();
    int QueryAccessFailure();
    int QueryResourcePredicate11();
    int HasValidSaveRecord();
    void* FindValidSaveRecord();

    // BP entry for the menu's own now_saving object.
    int RecreateBpEntry();
    int SetScriptEnabled(int enabled);

    static int RestoreSystemUiAndCallbacks();
    static int IsSaveSlotOccupied(CFileIoMenuView* p, int index);
    static void InitBlankSaveRecordSet();
    static void* BorrowPlayerRecords();
    int BuildCurrentPlayerProfile();
    static int StartSavingScript(void* p);
    static void InitBlankPlayerRecord(void* p, void* value);
    static void SelectAccountSlot(void* p, void* value);

    static int ProcessWhenReady(FileIoMenuCalls* p);

    CFileIoMenu();
    void* Destroy(int flag);
};

#define CFILEIOMENU_OFF(f, o) typedef char CFileIoMenu_off_##f[((int)&((CFileIoMenu*)0)->f == (o)) ? 1 : -1]
CFILEIOMENU_OFF(fileAccess, 0x18);
CFILEIOMENU_OFF(commonDialogComplete, 0x24);
CFILEIOMENU_OFF(commonDialog, 0x20);
CFILEIOMENU_OFF(saveForm, 0x28);
CFILEIOMENU_OFF(bpEntryId, 0x2C);
CFILEIOMENU_OFF(bpObject, 0x30);
CFILEIOMENU_OFF(messageStringFlags, 0x34);
CFILEIOMENU_OFF(messageStringData, 0x3C);
#undef CFILEIOMENU_OFF

#endif // CLASSES_CFILEIOMENU_H
