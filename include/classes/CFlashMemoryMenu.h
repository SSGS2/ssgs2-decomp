#ifndef CLASSES_CFLASHMEMORYMENU_H
#define CLASSES_CFLASHMEMORYMENU_H

#include "game/types.h"

struct FlashMenuBannerView;
class CFlashMemoryMenu {
public:
    void* vtbl;                  // 0x0: original vtable pointer
    char pad_004[0x14];          // 0x4
    void* fileIo;                // 0x18
    char pad_01C[0x10];          // 0x1c
    int bpEntryId;               // 0x2c
    char pad_030[0x10];          // 0x30
    void* banner;                // 0x40
    unsigned int readRetryCount; // 0x44

    int RecreateBpEntry();
    int RestoreSystemUiAfterLoad();

    int ClearResult();
    int QueryResourcePredicate8();
    int LoadValidSaveRecord();
    int VerifyWrittenSlotBlock();

    int PrepareBannerRead();
    int PrepareSaveDataRead();
    int CreateSaveWriteFileIo();
    int CommitSaveFile();
    int CommitBannerFile();
    void SetSavingIndicatorEnabled(int arg);
    void SetSharedRecordMode(int recordMode, int modeFlags);
    int StartSavingScript();
    static int IsInitializationFinished(FlashMenuBannerView* arg);
    static int RecreateEntryAndBanner(FlashMenuBannerView* arg);
    int CreateBannerWriteFileIo();
    int PrepareStorageCheck();

    // Resource readiness, banner polling and the read-retry and storage checks.
    int IsResourceReady();
    int UpdateAndPollBanner(float delta);
    int TryConsumeSingleIoRetry();
    int IsStorageSpaceFull();
    int IsStorageEntryTableFull();

    ~CFlashMemoryMenu();
    CFlashMemoryMenu();
};

#define CFLASHMEMORYMENU_OFF(f, o) \
    typedef char CFlashMemoryMenu_off_##f[((int)&((CFlashMemoryMenu*)0)->f == (o)) ? 1 : -1]
CFLASHMEMORYMENU_OFF(fileIo, 0x18);
CFLASHMEMORYMENU_OFF(bpEntryId, 0x2C);
CFLASHMEMORYMENU_OFF(banner, 0x40);
CFLASHMEMORYMENU_OFF(readRetryCount, 0x44);
#undef CFLASHMEMORYMENU_OFF

#endif // CLASSES_CFLASHMEMORYMENU_H
