// NAND (flash memory) save menu: CFlashMemoryMenuSave runs the save state machine, and
// CFlashMemoryMenu writes the save record and banner files, checks free space and retries
// failed reads once.

#include <decomp/static_init.h>
// NAND User Data save menu: asynchronous banner/record reads, temporary writes,
// publication by move, readback verification, dialogs, audio and fade ownership.
// Mod lookup (frame rate / save-load pacing): CFlashMemoryMenuSave::UpdateSaveStateMachine ticks
// ONE switch case per successful update; CFlashMemoryMenu::UpdateAndPollBanner polls the base
// menu, then banner initialization. The initial stream fade uses 0.5 seconds
// (kFloat_0_5_FlashMenu in the data section), not a frame countdown. The
// terminal-state query tests saveState==SaveComplete (24); the outer menu
// wrapper also completes on tick failure (0). A tick return of 1 is NOT completion.
// Faster calls reduce case-to-case latency but must never bypass I/O polling,
// menu setup hooks, dialogs, or readback. There is no fixed FPS value to edit here.
// Retry knobs: kSaveWriteRetryLimit = 3 extra attempts, separate write and
// verification budgets; a separate read counter allows ONE retry across the
// initial banner/record reads. ClearResult also zeros that same +0x44 word at
// menu admission and successful read stages; it is not a saved success latch.
// Data-format sizes: record snapshot 0x20a0 bytes, banner image 0xf0a0 bytes;
// these describe serialization, not pacing. Paths are g_*Save*Filename globals.
// Preserve the temporary/final distinction and record verification before banner
// publication. No resolution, aspect, model, or texture-format control here.
// Layouts: CFlashMemoryMenu.h / CFlashMemoryMenuSave.h plus local overlays below.
// All receiver pointers are required unless a destructor explicitly accepts null.
// Owned saveForm/fileIo/banner live in the receiver; replacement destroys the
// previous object. File access is asynchronous; see the per-factory buffer/path
// contracts at the declarations below. The retained fileIo is polled by updates.
// The save operation writes a temporary file, moves it into place, then reads it back.
enum FlashSaveState {
    SaveFadeAudio = 0,
    SaveWaitForAudio = 1,
    SaveWaitForMenu = 2,
    SaveReadBanner = 3,
    SaveWaitForBannerRead = 4,
    SaveReadRecord = 5,
    SaveWaitForRecordRead = 6,
    SaveWriteBanner = 7,
    SaveWaitForBannerWrite = 8,
    SaveWaitForBannerMove = 9,
    SaveWriteRecord = 10,
    SaveWaitForRecordWrite = 11,
    SaveWaitForRecordMove = 12,
    SaveWaitForVerification = 13,
    SaveFinish = 14,
    SaveShowBusy = 15,
    SaveWaitForBusyDialog = 16,
    SaveShowDamagedRecord = 17,
    SaveWaitForDamageDialog = 18,
    SaveWaitForDamageAdvice = 19,
    SaveShowWriteFailure = 20,
    SaveWaitForWriteFailure = 21,
    SaveShowFatalError = 22,
    SaveWaitForFatalDialog = 23,
    SaveComplete = 24
};
enum {
    kSaveWriteRetryLimit = 3,
    kSaveRecordBytes = 0x20A0,
    kSaveBannerImageBytes = 0xF0A0,
    kSavingIndicatorGroup = 1000,
    kSaveSingleReadRetryLimit = 1,
    kSaveSpaceFullMask = 0x5,
    kSaveEntryTableFullMask = 0xA
};
// Fields missing from the partial CFlashMemoryMenu header; no shared header is changed.
struct FlashMenuSaveFormView {
    char menuPrefix[0x14];
    void* bpEntryId;
    char accessAndDialog[0x10];
    void* saveForm;
};
struct FlashSaveMessages {
    char damagedRecord[0x28];
    char busy[0x22];
    char damagedRecordAdvice[0x28];
    char writeFailure[0x32];
    char fatalError[0x30];
};

typedef char FlashSaveMessages_size[(sizeof(FlashSaveMessages) == 0xD4) ? 1 : -1];
typedef char FlashMenuSaveForm_offset[((int)&((FlashMenuSaveFormView*)0)->saveForm == 0x28) ? 1 : -1];

#include "game/singletons.h"
#include "classes/CFlashMemoryMenuSave.h"
#include "classes/CFlashMemoryMenu.h"
#include "classes/CFileIoAccess.h"
#include "classes/CFileIoMenu.h"
#include "classes/CSaveFormBanner.h"
#include "classes/CSaveFormFlash.h"
#include "classes/CFileIoAccessCheckFlash.h"
#include "classes/CBanner.h"
#include "classes/CBpCtrl.h"
#include "classes/CFileIoAccessWriteFlash.h"
#include "classes/CFileIoAccessReadFlash.h"
#include "classes/CFileIoAccessMoveFlash.h"

extern "C" {

int SaveBanner_HasFinishedInitializationAttempt(void*);
void SaveBanner_PollInitializationAttempt(void*, float);
void FadeOverlay_Init(void*);
void __register_global_object(void*, void (*)(), void*);
void FadeState_StartFadeIn(void*, int);
void FadeState_SetAlternateDrawEnabled(void*, int);
void* __nw__FUl(unsigned int);
void __dl__FPv(void*);
void SaveIO_BeginSystemGuard(void*);
void* __dt__16CFlashMemoryMenuFv(void*, int);
void* SaveForm_GetStorageSize(void*);
// Read factory contract: save_form_records_and_flash_io.cpp -> ReadFlash_vfunc_6 ->
// CFileIoFlash::CreateInstance (file_io_flash_and_write_access.cpp). Path is a borrowed relative flash
// filename and size is a positive byte count encoded in the pointer ABI.
// New backend setup prefixes/copies the path through Flash_vfunc_13 ->
// CFileIo_SetPathAndAllocateBuffer (file_io_pad_and_flash_io.cpp); read bytes belong to the retained
// backend. A reused flash singleton is acquired by reference rather than rebuilt.
// No caller output buffer is retained. Null means allocation/setup failed;
// nonnull means access created, with completion/error determined by polling.
void SoundManager_ApplySavedAudioSettings(void*, float);
void MovieController_ReleasePlaybackReference(void*);
void* SaveForm_ReplicateWithDigest(void*, void*, void*, int);
// The save file stores the four slot records in the Wii (big-endian) byte order; see
// save_data_records_and_defaults.cpp for the field map.
void* SaveForm_GetRecordSize(void*);
void SaveBanner_InitializeLocalizedImage(void*, void*, int);
// Check factory contract: pop_item_dialog_and_loading_draw.cpp SetupWithBlockList copies count u32 byte
// sizes (count>0) into the accessor before base setup. This caller supplies two
// rounded sizes [record,banner]; stack storage may die immediately on return.
// Path/backend lifetime follows the read factory above. Failure discards the
// newly allocated checker, including a list installed before base setup failed.
void* FileIO_GetDeviceTransferAlignment(const char*);
// Write factory contract: save_form_records_and_flash_io.cpp -> CFileIoAccessWrite::InitializeBuffer
// (file_io_flash_and_write_access.cpp) allocates and copies storageBytes from required source before
// creating the backend. Caller source may be freed on return, even on failure.
// Accessor owns that copy until its deleting destructor; path follows the read
// factory contract. Null means allocation/buffer/base setup failed, not I/O
// completion. Success transfers the access to this menu's fileIo slot.
void* __nwa__FUl(unsigned int);
void* SaveData_AcquireInitializedInstance();
void* SaveData_GetFirstRecordStorage(void*);
void SaveSlotArray_CopyCtor(void*, void*);
void __dla__FPv(void*);
void MovieController_Ctor(void*);
void MovieController_Dtor();
void ResourceState_Dtor();
void* g_soundManager;
signed char s_movieControllerInitialized;
char s_movieController[0x100];
char s_movieControllerDtorRegistration_FlashMenu[0x100];
// Move factory contract: loading_draw_tips_and_background.cpp Create -> small_common_dialog.cpp SetPathsAndStart copies
// BOTH prefixed paths before return. Actual order is destination,source (final,
// temporary here), as established by CFileIoFlash::MoveQualifiedPath cfileio_flash_file_operations.cpp.
// Caller strings need not outlive setup. Nonnull transfers the access, not a
// completed move; null discards the newly allocated access. Every factory here
// is released through virtual deleting destructor slot +0x14, argument 1, when
// its fileIo slot is replaced or the base CFileIoMenu is destroyed.
struct FlashMenuBannerView;
FlashMenuBannerView* __ct__16CFlashMemoryMenuFv(FlashMenuBannerView*);
void CBpCtrl_dtor();
int IdMap_Lookup(void*, void*);
int UIGroup_GetOrCreateById(int, int);
void UIGroup_SetEnabled(int, int);
void __register_global_object(void*, void (*)(), void*);
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[0x100];
extern char s_fadeOverlayDtorRegistration_FlashMenu[0x100];

struct FlashMenuBannerView {
    void* vtable;
    char pad[0x3C];
    void* banner;
    void* readRetryCount;
};

class DestroyableView {
public:
    virtual void destroy(int arg);
};

class Fn8027CVirtual2 {
public:
    virtual void v0();
    virtual void destroy(int arg);
};

class FileIoAccessView {
public:
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void destroy(int arg);
};

class Fn8027B430Object {
public:
    virtual void destroy(int arg);
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual int valid();
};

struct Fn8027VtableData;
extern "C" Fn8027VtableData __vt__20CFlashMemoryMenuSave;
// Construct the inherited flash menu and clear the three save-state words.
inline CFlashMemoryMenuSave::CFlashMemoryMenuSave() {
    ((CFlashMemoryMenu*)((FlashMenuBannerView*)this))->__ct();
    vtbl = (void*)&__vt__20CFlashMemoryMenuSave;
    *(void**)&saveState = 0;
    *(void**)&retryCount = 0;
    *(void**)&accessRetryCount = 0;
}

struct Fn8027VtableData {
    char data[0x100];
};

extern "C" unsigned char s_skipDamagedRecordAdvice;
extern "C" const float kFloat_6_2831855;
extern "C" const float kFloat_0_5_FlashMenu;
extern "C" const float kFloat_0_FlashMenu;
extern "C" const float kSaveMenuSilentVolumeThreshold;
extern "C" Fn8027VtableData __vt__16CFlashMemoryMenu;
extern "C" float s_fileIoMenuTwoPi;
extern "C" float s_flashMemoryMenuTwoPi;
extern "C" signed char s_bpCtrlInitialized;
extern "C" char s_bpCtrl[0x100];
extern "C" char s_bpCtrlDtorRegistration_8050A210[0x100];
extern "C" Fn8027VtableData __vt__20CFlashMemoryMenuSave;
extern "C" void* g_saveBannerFilename; // Permanent banner path; read here and used as the banner move destination.
extern "C" void* g_saveDataFilename;
extern "C" void*
    g_temporarySaveDataFilename; // Temporary player-record path, written before committing to g_saveDataFilename.
extern "C" void*
    g_temporarySaveBannerFilename; // Temporary banner path, written before committing to the permanent banner path.
// MD5 trailer switch, initialized to 1 in the data section (no source writer).
// CSaveFormFlash::Create -> CSaveForm_Ctor (archive_zip_reader_and_game_hooks.cpp/save_form_records_and_flash_io.cpp) stores this byte
// as hasMd5: zero disables the digest, any nonzero enables a 16-byte MD5 trailer
// per copy. All read/write/verify forms must agree; changing it changes the NAND
// record stride, storage size and validation, not just the saving presentation.
extern "C" unsigned char s_flashSaveMd5Enabled;
extern "C" const char
    kFlashDeviceRegistryPrefix[]; // Registered flash-device prefix used to query allocation block size.

struct FlashSaveStateView {
    char menuPrefix[0x48];
    int saveState;
};

// Save-menu initialization hook; this implementation needs no extra work.
void CFlashMemoryMenuSave::vfunc_3() {}
// Report completion once the save state machine reaches its terminal state.
// This is lifecycle completion after either successful publication or dismissed
// errors; see the result/transition ledger at CFlashMemoryMenuSave::UpdateSaveStateMachine.
int CFlashMemoryMenuSave::IsSaveComplete(FlashSaveStateView* stateView) {
    return stateView->saveState == SaveComplete;
}
// Initialize this module's cached 2-pi constant during static initialization.
void FlashMemoryMenu_InitializeFullTurnAngle() {
    s_flashMemoryMenuTwoPi = kFloat_6_2831855;
}
// Draw the inherited file-I/O menu.
void CFlashMemoryMenuSave::Draw(void* fileIoMenu) {
    ((CFileIoMenu*)fileIoMenu)->Draw();
}
#pragma dont_inline on
// Delegate resource readiness to the base file-I/O menu.
int CFlashMemoryMenu::IsResourceReady() {
    return ((CFileIoMenu*)this)->vfunc_8();
}
#pragma dont_inline off
// Set whether damaged or missing save data may be skipped without showing advice.
int SaveIO_SetSkipDamagedRecordAdvice(unsigned char skipAdvice) {
    s_skipDamagedRecordAdvice = skipAdvice;
    return 1;
}

// Initialize this module's cached 2-pi constant during static initialization.
void FileIoMenu_InitializeFullTurnAngle() {
    s_fileIoMenuTwoPi = kFloat_6_2831855;
}
// Replace the current save form and begin reading the NAND banner file.
// Returns 1 when the access object was created, not when reading completes.
// Failure returns 0 with the new form/access slots possibly null; old objects
// have already been destroyed. Poll status before inspecting bytes/results.
int CFlashMemoryMenu::PrepareBannerRead() {
    void** saveFormSlot = &((FlashMenuSaveFormView*)this)->saveForm;
    if (*saveFormSlot != 0) {
        if (*saveFormSlot != 0) {
            ((DestroyableView*)*saveFormSlot)->destroy(1);
        }
        *saveFormSlot = 0;
    }
    *saveFormSlot = CSaveFormBanner::Create();
    if (*saveFormSlot == 0)
        return 0;
    SaveIO_BeginSystemGuard(this);
    void** fileAccessSlot = &fileIo;
    if (*fileAccessSlot != 0) {
        if (*fileAccessSlot != 0) {
            ((FileIoAccessView*)*fileAccessSlot)->destroy(1);
        }
        *fileAccessSlot = 0;
    }
    *fileAccessSlot = CFileIoAccessReadFlash::Create(g_saveBannerFilename, SaveForm_GetStorageSize(*saveFormSlot));
    if (*fileAccessSlot == 0)
        return 0;
    return 1;
}
// Replace the current save form and begin reading the NAND player-record file.
// Same partial-replacement/factory-status contract as PrepareBannerRead; record
// bytes are owned by the new access object, not returned through this function.
int CFlashMemoryMenu::PrepareSaveDataRead() {
    void** saveFormSlot = &((FlashMenuSaveFormView*)this)->saveForm;
    if (*saveFormSlot != 0) {
        if (*saveFormSlot != 0) {
            ((DestroyableView*)*saveFormSlot)->destroy(1);
        }
        *saveFormSlot = 0;
    }
    *saveFormSlot = CSaveFormFlash::Create(1, s_flashSaveMd5Enabled);
    if (*saveFormSlot == 0)
        return 0;
    SaveIO_BeginSystemGuard(this);
    void** fileAccessSlot = &fileIo;
    if (*fileAccessSlot != 0) {
        if (*fileAccessSlot != 0) {
            ((FileIoAccessView*)*fileAccessSlot)->destroy(1);
        }
        *fileAccessSlot = 0;
    }
    *fileAccessSlot = CFileIoAccessReadFlash::Create(g_saveDataFilename, SaveForm_GetStorageSize(*saveFormSlot));
    if (*fileAccessSlot == 0)
        return 0;
    return 1;
}
// Restore audio, movie playback, and fade drawing before destroying the save menu.
// Null accepted; shouldDelete > 0 frees self, so the returned address then dangles.
// Preserve playback reference release and fade restoration before base cleanup.
CFlashMemoryMenuSave::~CFlashMemoryMenuSave() {
    *(void**)this = (void*)&__vt__20CFlashMemoryMenuSave;
    SoundManager_ApplySavedAudioSettings(g_soundManager, kFloat_0_5_FlashMenu);
    if (Sing_805DC950() == 0) {
        if (!s_movieControllerInitialized) {
            MovieController_Ctor(&s_movieController);
            __register_global_object(&s_movieController, MovieController_Dtor,
                                     &s_movieControllerDtorRegistration_FlashMenu);
            s_movieControllerInitialized = 1;
        }
        Sing_805DC950() = &s_movieController;
    }
    MovieController_ReleasePlaybackReference(Sing_805DC950());
    if (Sing_805DC608() == 0) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(&s_fadeOverlay);
            __register_global_object(&s_fadeOverlay, ResourceState_Dtor, &s_fadeOverlayDtorRegistration_FlashMenu);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = &s_fadeOverlay;
    }
    FadeState_SetAlternateDrawEnabled(Sing_805DC608(), 0);
    __dt__16CFlashMemoryMenuFv(this, 0);
}
// Serialize the current save slots with their digest and start writing the temporary record file.
// Returns factory success, not durable-save success. Replaces owned form/access;
// failure can leave a form with no access. Snapshot/serialized bytes are temporary.
// Requires initialized save data. A successful allocation provides storageBytes
// writable bytes. If __nwa__ returns null, ReplicateWithDigest rejects it BEFORE
// clearing/copying, no write access is made, and this function returns 0; the
// snapshot is still taken and the new save form/system guard remain installed.
// Allocation exceptions propagate; this function does not convert them to 0.
int CFlashMemoryMenu::CreateSaveWriteFileIo() {
    void** saveFormSlot = &((FlashMenuSaveFormView*)this)->saveForm;
    if (*saveFormSlot != 0) {
        if (*saveFormSlot != 0) {
            ((DestroyableView*)*saveFormSlot)->destroy(1);
        }
        *saveFormSlot = 0;
    }
    *saveFormSlot = CSaveFormFlash::Create(1, s_flashSaveMd5Enabled);
    if (*saveFormSlot == 0)
        return 0;
    SaveIO_BeginSystemGuard(this);
    void** fileAccessSlot = &fileIo;
    if (*fileAccessSlot != 0) {
        if (*fileAccessSlot != 0) {
            ((FileIoAccessView*)*fileAccessSlot)->destroy(1);
        }
        *fileAccessSlot = 0;
    }
    // ABI uses void* for scalar byte counts; these values are never dereferenced.
    // serializedRecord stays locally owned: WriteFlash_Create calls
    // CFileIoAccessWrite::InitializeBuffer (file_io_flash_and_write_access.cpp), which allocates and
    // copies its bytes before returning. The local is freed even on success.
    void* storageBytes = SaveForm_GetStorageSize(*saveFormSlot);
    if (storageBytes == 0)
        return 0;
    void* serializedRecord = __nwa__FUl((unsigned int)storageBytes);
    int success = 0;
    void* saveData = SaveData_AcquireInitializedInstance();
    void* slotArray = SaveData_GetFirstRecordStorage(saveData);
    unsigned char slotSnapshot[kSaveRecordBytes];
    SaveSlotArray_CopyCtor(slotSnapshot, slotArray);
    // The save file holds the Wii (big-endian) byte order; the digest covers these bytes.
    int serialized = (int)SaveForm_ReplicateWithDigest(*saveFormSlot, serializedRecord, slotSnapshot, kSaveRecordBytes);
    if (serialized != 0) {
        *fileAccessSlot = CFileIoAccessWriteFlash::Create(g_temporarySaveDataFilename,
                                                          SaveForm_GetStorageSize(*saveFormSlot), serializedRecord);
        if (*fileAccessSlot != 0)
            success = 1;
    }
    if (serializedRecord != 0)
        __dla__FPv(serializedRecord);
    return success;
}
// Start moving the temporary player-record file to the permanent save path.
// Returns access creation status; polling decides move completion. Replaces owned
// form/access even on failure. Move factory argument order is FINAL then TEMP.
int CFlashMemoryMenu::CommitSaveFile() {
    void** saveFormSlot = &((FlashMenuSaveFormView*)this)->saveForm;
    if (*saveFormSlot != 0) {
        if (*saveFormSlot != 0) {
            ((DestroyableView*)*saveFormSlot)->destroy(1);
        }
        *saveFormSlot = 0;
    }
    *saveFormSlot = CSaveFormFlash::Create(1, s_flashSaveMd5Enabled);
    if (*saveFormSlot == 0)
        return 0;
    SaveIO_BeginSystemGuard(this);
    void** fileAccessSlot = &fileIo;
    if (*fileAccessSlot != 0) {
        if (*fileAccessSlot != 0) {
            ((FileIoAccessView*)*fileAccessSlot)->destroy(1);
        }
        *fileAccessSlot = 0;
    }
    *fileAccessSlot = CFileIoAccessMoveFlash::Create(g_saveDataFilename, g_temporarySaveDataFilename);
    if (*fileAccessSlot == 0)
        return 0;
    return 1;
}
// Start moving the temporary banner file to the permanent banner path.
// Returns access creation status. Preserve final/temp argument order and the
// record-before-banner state-machine dependency; this call alone cannot verify data.
int CFlashMemoryMenu::CommitBannerFile() {
    void** saveFormSlot = &((FlashMenuSaveFormView*)this)->saveForm;
    if (*saveFormSlot != 0) {
        if (*saveFormSlot != 0) {
            ((DestroyableView*)*saveFormSlot)->destroy(1);
        }
        *saveFormSlot = 0;
    }
    *saveFormSlot = CSaveFormFlash::Create(1, s_flashSaveMd5Enabled);
    if (*saveFormSlot == 0)
        return 0;
    SaveIO_BeginSystemGuard(this);
    void** fileAccessSlot = &fileIo;
    if (*fileAccessSlot != 0) {
        if (*fileAccessSlot != 0) {
            ((FileIoAccessView*)*fileAccessSlot)->destroy(1);
        }
        *fileAccessSlot = 0;
    }
    *fileAccessSlot = CFileIoAccessMoveFlash::Create(g_saveBannerFilename, g_temporarySaveBannerFilename);
    if (*fileAccessSlot == 0)
        return 0;
    return 1;
}
// Destroy the banner resource, then release the base file-I/O menu.
// Null accepted; shouldDelete > 0 deletes self, leaving a dangling return address.
CFlashMemoryMenu::~CFlashMemoryMenu() {
    DestroyableView* value = (*(DestroyableView**)&banner);
    *(void**)this = (void*)&__vt__16CFlashMemoryMenu;
    if (value != 0) {
        if (value != 0) {
            value->destroy(1);
        }
        banner = 0;
    }
    ((CFileIoMenu*)this)->Destroy(0);
}
// Show or hide the saving indicator in layout group 1000.
void CFlashMemoryMenu::SetSavingIndicatorEnabled(int enabled) {
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_8050A210);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
    int result = IdMap_Lookup(Sing_805DC5F0(), ((FlashMenuSaveFormView*)this)->bpEntryId);
    int indicatorGroup = UIGroup_GetOrCreateById(result, kSavingIndicatorGroup);
    if (indicatorGroup)
        UIGroup_SetEnabled(indicatorGroup, enabled);
}
// Allocate a save menu and destroy it if its initialization check fails.
// Returns owned menu or null; release with CFlashMemoryMenuSave::~CFlashMemoryMenuSave(menu,1).
void* CFlashMemoryMenuSave::Create() {
    CFlashMemoryMenuSave* object = new CFlashMemoryMenuSave();
    if (object == 0)
        return 0;
    if (((Fn8027B430Object*)object)->valid() == 0) {
        if (object != 0)
            ((DestroyableView*)object)->destroy(1);
        return 0;
    }
    return object;
}
#pragma dont_inline on
// Read the flag that allows damaged or missing records to be skipped without advice.
int SaveIO_ShouldSkipDamagedRecordAdvice() {
    return s_skipDamagedRecordAdvice;
}
#pragma dont_inline off
// Resolve bpEntryId into borrowed bpObject and START s_fileIoSavingScriptName
// with restart=1 through CFileIoMenu::StartSavingScript (file_io_menu_save_load_base.cpp). Returns 0 if lookup
// fails (bpObject cleared), 1 after starting; it does not test script completion.
// Requires the loaded menu entry; repeated calls restart the saving script.
int CFlashMemoryMenu::StartSavingScript() {
    return CFileIoMenu::StartSavingScript(this);
}
void SoundManager_SetStreamVolume(void*, float, float);
float SoundManager_GetVolumeByMode(void*, int);
void MovieController_AcquirePlaybackReference(void*);
// Slot 11 selects fatal-error handling (flash status 0xC354); slot 8
// selects busy/retry handling (flash status 0xC353). Neither tests readiness.
void FileIo_SetSystemUiAndDriveErrorsEnabled(void*, int);
extern char fileIoErrorDialogMessages[0x100];
// Local dispatch view (offsets include CodeWarrior's two-word vtable header).
// Verified targets in phase1_class_tables.cpp::__vt__20CFlashMemoryMenuSave:
// m30 (+0x30) = CFileIoMenu::RestoreSystemUiAndCallbacks: restore Home Menu/system-button callbacks
//   and drive-error dialogs; false would block the terminal-state transition.
// m34 (+0x34) = CFlashMemoryMenu::RecreateEntryAndBanner: recreate BP entry and owned banner;
//   false returns 0 after any partial replacement.
// m38 (+0x38) = CFlashMemoryMenu::IsInitializationFinished: poll BP entries/banner initialization;
//   false stays in SaveWaitForMenu (finished initialization is not I/O success).
// m3c (+0x3C) = CFlashMemoryMenu::StartSavingScript -> CFileIoMenu::StartSavingScript: resolve the
//   BP object and start the saving script; false returns 0. See file_io_menu_save_load_base.cpp.
// Keep virtual dispatch so an overridden receiver retains its own hook behavior.
class FlashMenuHooksView {
public:
    virtual void d0();
    virtual void d1();
    virtual void d2();
    virtual void d3();
    virtual void d4();
    virtual void d5();
    virtual void d6();
    virtual void d7();
    virtual void d8();
    virtual void d9();
    virtual int m30();
    virtual int m34();
    virtual int m38();
    virtual int m3c();
};
// Advance NAND save, temporary-file moves, verification, retries, and error dialogs.
// Return 1 after a normal state-machine tick (including waits), 0 when the base
// menu or a setup/creation hook fails/blocks. The terminal-state query is
// saveState==24; outer menu completion can also result from tick failure below.
// Poll status first; nonzero predicate11 selects fatal error, nonzero predicate8
// selects one-shot busy retry on reads. Neither predicate means "ready".
// Main path: audio/menu -> read banner/record -> write/move record -> verify ->
// write/move banner -> finish. Write failures retry the whole temporary-file step;
// verification has its own counter. Error dialogs eventually return to finish.
// delta occupies the float ABI slot forwarded to UpdateAndPollBanner. The real
// SaveBanner_PollInitializationAttempt (movie_controller_and_banner.cpp) takes only self: it waits on
// buffers/parses once and consumes NO time value. Thus delta has no physical
// unit/scale requirement in this implementation; it cannot speed up banner
// loading. The stream fade separately uses seconds in its sound-manager API.
// Wait states retain the receiver's current access/form. Setup/factory failure
// normally returns 0 without advancing, and may already have replaced resources.
// Exception: SaveShowBusy ignores acknowledge-dialog creation failure and always
// advances to SaveWaitForBusyDialog; it does not retry creation on the next tick.
// SaveComplete and unlisted states do no switch work and return 1 after base poll.
// Error flow: busy -> acknowledge -> finish; damaged -> advice -> finish;
// exhausted writes -> failure -> damaged/advice; fatal -> disable system errors,
// acknowledge, re-enable errors -> finish. Do not skip those paired side effects.
// Caller-visible result ledger:
// - 0 from this tick means an update/setup hook blocked/failed, not save failure;
//   1 includes waiting, dialog handling and the terminal no-op state.
// - CFileIoMenu::ProcessWhenReady (save_form_records_and_flash_io.cpp), the actual vtable process hook,
//   calls this tick at +0x2C. Tick==1 -> call IsSaveComplete at +0x58 (state==24);
//   tick==0 -> return 1 directly. MPBase1_Process (event_face_and_mpbase1_menu.cpp) latches ANY
//   nonzero process result as complete: setup/update failure can end the outer
//   menu BEFORE state 24, with the current partial resources still attached.
// - IsSaveComplete itself reports ONLY saveState==24. Ordinary publication reaches Finish
//   from WaitForBannerMove after the record readback matched and the banner's
//   final move reported no access failure. Busy/damage/advice/write/fatal dialog
//   exits also converge on Finish -> Complete; terminal state cannot distinguish
//   those outcomes. SkipDamagedRecordAdvice can take the same path directly.
// - ClearResult (flash_memory_menu_load_part1.cpp) only zeros inherited word +0x44. This save machine
//   never writes a success/error code there; in this layout +0x44 is also the
//   readRetryCount view. It is not a durable Boolean success result. Dialog
//   acceptance is a separate common-dialog result, not NAND write success.
// A caller needing confirmed save success must track the successful publication
// path/access outcome separately; neither tick return nor IsSaveComplete supplies it.
// Concrete consumer: CSaveProcessMain_OnProcess (save_process_main_task.cpp) waits on
// MPBase1_Process, destroys the menu and queues the requested task transition
// after either kind of outer completion; it reads no save-success result.
int CFlashMemoryMenuSave::UpdateSaveStateMachine(float delta) {
    if (!((CFlashMemoryMenu*)this)->UpdateAndPollBanner(delta))
        return 0;
    unsigned int& state = saveState;
    FlashMenuHooksView* menuHooks = (FlashMenuHooksView*)this;
    switch (state) {
    // Phase 1: mute the stream, hold movie playback, and wait for menu setup.
    case SaveFadeAudio:
        SoundManager_SetStreamVolume(g_soundManager, kFloat_0_FlashMenu, kFloat_0_5_FlashMenu);
        state = SaveWaitForAudio;
        break;
    case SaveWaitForAudio:
        if (kSaveMenuSilentVolumeThreshold >= SoundManager_GetVolumeByMode(g_soundManager, 0)) {
            if (Sing_805DC950() == 0) {
                if (!s_movieControllerInitialized) {
                    MovieController_Ctor(&s_movieController);
                    __register_global_object(&s_movieController, MovieController_Dtor,
                                             &s_movieControllerDtorRegistration_FlashMenu);
                    s_movieControllerInitialized = 1;
                }
                Sing_805DC950() = &s_movieController;
            }
            MovieController_AcquirePlaybackReference(Sing_805DC950());
            if (!menuHooks->m34())
                return 0;
            state = SaveWaitForMenu;
        }
        break;
    case SaveWaitForMenu:
        if (menuHooks->m38()) {
            if (!menuHooks->m3c())
                return 0;
            ((CFileIoMenu*)this)->SetScriptEnabled(0);
            ((CFlashMemoryMenu*)this)->ClearResult();
            state = SaveReadBanner;
        }
        break;
    // Phase 2: read existing banner/record and classify fatal, busy or damaged data.
    case SaveReadBanner:
        if (!((CFlashMemoryMenu*)this)->PrepareBannerRead())
            return 0;
        state = SaveWaitForBannerRead;
        break;
    case SaveWaitForBannerRead:
        if (!((CFileIoMenu*)this)->QueryFileAccessStatus())
            break;
        if (((CFileIoMenu*)this)->QueryResourcePredicate11()) {
            state = SaveShowFatalError;
            break;
        }
        if (((CFlashMemoryMenu*)this)->QueryResourcePredicate8()) {
            if (((CFlashMemoryMenu*)this)->TryConsumeSingleIoRetry())
                state = SaveReadBanner;
            else
                state = SaveShowBusy;
            break;
        }
        if (((CFileIoMenu*)this)->QueryAccessFailure()) {
            if (SaveIO_ShouldSkipDamagedRecordAdvice())
                state = SaveFinish;
            else
                state = SaveShowDamagedRecord;
            break;
        }
        if (((CFileIoMenu*)this)->HasValidSaveRecord()) {
            ((CFlashMemoryMenu*)this)->ClearResult();
            state = SaveReadRecord;
            break;
        }
        if (SaveIO_ShouldSkipDamagedRecordAdvice())
            state = SaveFinish;
        else
            state = SaveShowDamagedRecord;
        break;
    case SaveReadRecord:
        if (!((CFlashMemoryMenu*)this)->PrepareSaveDataRead())
            return 0;
        state = SaveWaitForRecordRead;
        break;
    case SaveWaitForRecordRead:
        if (!((CFileIoMenu*)this)->QueryFileAccessStatus())
            break;
        if (((CFileIoMenu*)this)->QueryResourcePredicate11()) {
            state = SaveShowFatalError;
            break;
        }
        if (((CFlashMemoryMenu*)this)->QueryResourcePredicate8()) {
            if (((CFlashMemoryMenu*)this)->TryConsumeSingleIoRetry())
                state = SaveReadRecord;
            else
                state = SaveShowBusy;
            break;
        }
        if (((CFileIoMenu*)this)->HasValidSaveRecord()) {
            ((CFlashMemoryMenu*)this)->ClearResult();
            ((CFileIoMenu*)this)->SetScriptEnabled(1);
            this->retryCount = 0;
            accessRetryCount = 0;
            state = SaveWriteRecord;
            break;
        }
        if (SaveIO_ShouldSkipDamagedRecordAdvice())
            state = SaveFinish;
        else
            state = SaveShowDamagedRecord;
        break;
    // Phase 4: publish the banner only after record verification succeeded.
    case SaveWriteBanner:
        if (!((CFlashMemoryMenu*)this)->CreateBannerWriteFileIo())
            return 0;
        state = SaveWaitForBannerWrite;
        break;
    case SaveWaitForBannerWrite:
        if (!((CFileIoMenu*)this)->QueryFileAccessStatus())
            break;
        if (((CFileIoMenu*)this)->QueryResourcePredicate11()) {
            state = SaveShowFatalError;
            break;
        }
        if (((CFileIoMenu*)this)->QueryAccessFailure()) {
            unsigned int retryCount = this->retryCount;
            int retryAvailable;
            if (retryCount >= kSaveWriteRetryLimit) {
                retryAvailable = 0;
            } else {
                this->retryCount = retryCount + 1;
                retryAvailable = 1;
            }
            if (retryAvailable)
                state = SaveWriteBanner;
            else
                state = SaveShowWriteFailure;
            break;
        }
        if (!((CFlashMemoryMenu*)this)->CommitBannerFile())
            return 0;
        state = SaveWaitForBannerMove;
        break;
    case SaveWaitForBannerMove:
        if (!((CFileIoMenu*)this)->QueryFileAccessStatus())
            break;
        if (((CFileIoMenu*)this)->QueryResourcePredicate11())
            state = SaveShowFatalError;
        else if (((CFileIoMenu*)this)->QueryAccessFailure()) {
            unsigned int retryCount = this->retryCount;
            int retryAvailable;
            if (retryCount >= kSaveWriteRetryLimit) {
                retryAvailable = 0;
            } else {
                this->retryCount = retryCount + 1;
                retryAvailable = 1;
            }
            if (retryAvailable)
                state = SaveWriteBanner;
            else
                state = SaveShowWriteFailure;
        } else
            state = SaveFinish;
        break;
    // Phase 3: write temporary record, move to final name, then read it back.
    case SaveWriteRecord:
        if (!((CFlashMemoryMenu*)this)->CreateSaveWriteFileIo())
            return 0;
        state = SaveWaitForRecordWrite;
        break;
    case SaveWaitForRecordWrite:
        if (!((CFileIoMenu*)this)->QueryFileAccessStatus())
            break;
        if (((CFileIoMenu*)this)->QueryResourcePredicate11()) {
            state = SaveShowFatalError;
            break;
        }
        if (((CFileIoMenu*)this)->QueryAccessFailure()) {
            unsigned int retryCount = this->retryCount;
            int retryAvailable;
            if (retryCount >= kSaveWriteRetryLimit) {
                retryAvailable = 0;
            } else {
                this->retryCount = retryCount + 1;
                retryAvailable = 1;
            }
            if (retryAvailable)
                state = SaveWriteRecord;
            else
                state = SaveShowWriteFailure;
            break;
        }
        if (!((CFlashMemoryMenu*)this)->CommitSaveFile())
            return 0;
        state = SaveWaitForRecordMove;
        break;
    case SaveWaitForRecordMove:
        if (!((CFileIoMenu*)this)->QueryFileAccessStatus())
            break;
        if (((CFileIoMenu*)this)->QueryResourcePredicate11()) {
            state = SaveShowFatalError;
            break;
        }
        if (((CFileIoMenu*)this)->QueryAccessFailure()) {
            unsigned int retryCount = this->retryCount;
            int retryAvailable;
            if (retryCount >= kSaveWriteRetryLimit) {
                retryAvailable = 0;
            } else {
                this->retryCount = retryCount + 1;
                retryAvailable = 1;
            }
            if (retryAvailable)
                state = SaveWriteRecord;
            else
                state = SaveShowWriteFailure;
            break;
        }
        if (!((CFlashMemoryMenu*)this)->PrepareSaveDataRead())
            return 0;
        state = SaveWaitForVerification;
        break;
    case SaveWaitForVerification:
        if (!((CFileIoMenu*)this)->QueryFileAccessStatus())
            break;
        if (((CFileIoMenu*)this)->QueryResourcePredicate11()) {
            state = SaveShowFatalError;
            break;
        }
        if (((CFlashMemoryMenu*)this)->VerifyWrittenSlotBlock()) {
            this->retryCount = 0;
            accessRetryCount = 0;
            state = SaveWriteBanner;
        } else {
            unsigned int retryCount = accessRetryCount;
            int retryAvailable;
            if (retryCount >= kSaveWriteRetryLimit) {
                retryAvailable = 0;
            } else {
                accessRetryCount = retryCount + 1;
                retryAvailable = 1;
            }
            if (retryAvailable) {
                this->retryCount = 0;
                state = SaveWriteRecord;
            } else
                state = SaveShowWriteFailure;
        }
        break;
    // Phase 5: finish hook / terminal state, or display the selected error sequence.
    case SaveFinish:
        ((CFileIoMenu*)this)->SetScriptEnabled(0);
        if (!menuHooks->m30())
            return 0;
        state = SaveComplete;
        break;
    case SaveShowDamagedRecord:
        if (!((CFileIoMenu*)this)
                 ->ShowAcknowledgeDialog(((FlashSaveMessages*)fileIoErrorDialogMessages)->damagedRecord))
            return 0;
        state = SaveWaitForDamageDialog;
        break;
    case SaveShowBusy:
        ((CFileIoMenu*)this)->SetScriptEnabled(0);
        ((CFileIoMenu*)this)->ShowAcknowledgeDialog(((FlashSaveMessages*)fileIoErrorDialogMessages)->busy);
        state = SaveWaitForBusyDialog;
        break;
    case SaveWaitForBusyDialog:
        if (((CFileIoMenu*)this)->IsCommonDialogComplete())
            state = SaveFinish;
        break;
    case SaveWaitForDamageDialog:
        if (((CFileIoMenu*)this)->IsCommonDialogComplete()) {
            if (!((CFileIoMenu*)this)
                     ->ShowAcknowledgeDialog(((FlashSaveMessages*)fileIoErrorDialogMessages)->damagedRecordAdvice))
                return 0;
            state = SaveWaitForDamageAdvice;
        }
        break;
    case SaveWaitForDamageAdvice:
        if (((CFileIoMenu*)this)->IsCommonDialogComplete())
            state = SaveFinish;
        break;
    case SaveShowWriteFailure:
        ((CFileIoMenu*)this)->SetScriptEnabled(0);
        if (!((CFileIoMenu*)this)->ShowAcknowledgeDialog(((FlashSaveMessages*)fileIoErrorDialogMessages)->writeFailure))
            return 0;
        state = SaveWaitForWriteFailure;
        break;
    case SaveWaitForWriteFailure:
        if (((CFileIoMenu*)this)->IsCommonDialogComplete())
            state = SaveShowDamagedRecord;
        break;
    case SaveShowFatalError:
        ((CFileIoMenu*)this)->SetScriptEnabled(0);
        if (!((CFileIoMenu*)this)->ShowAcknowledgeDialog(((FlashSaveMessages*)fileIoErrorDialogMessages)->fatalError))
            return 0;
        FileIo_SetSystemUiAndDriveErrorsEnabled(this, 0);
        state = SaveWaitForFatalDialog;
        break;
    case SaveWaitForFatalDialog:
        if (((CFileIoMenu*)this)->IsCommonDialogComplete()) {
            FileIo_SetSystemUiAndDriveErrorsEnabled(this, 1);
            state = SaveFinish;
        }
        break;
    }
    return 1;
}

// Initialize the base menu, banner pointer, and one-shot read-retry counter.
CFlashMemoryMenu::CFlashMemoryMenu() {
    FlashMenuBannerView* arg = (FlashMenuBannerView*)this;
    ((CFileIoMenu*)arg)->__ct();
    arg->banner = 0;
    arg->vtable = &__vt__16CFlashMemoryMenu;
    arg->readRetryCount = 0;
}
// Wait for both the BP controller entries and the save-banner resource.
// Returns 1 for a FINISHED initialization attempt, which may have failed. An
// absent banner does not block. This predicate is separate from file I/O success.
int CFlashMemoryMenu::IsInitializationFinished(FlashMenuBannerView* arg) {
    if (!((CBpCtrl*)arg)->PollSingletonEntriesLoaded())
        return 0;
    int result = 1;
    if (arg->banner != 0 && !SaveBanner_HasFinishedInitializationAttempt(arg->banner))
        result = 0;
    return result;
}
// Recreate the BP entry and replace the save-banner resource.
// Returns 0 on entry recreation or banner allocation failure; the latter leaves
// banner null after destroying the previous one. Success retains the new banner.
int CFlashMemoryMenu::RecreateEntryAndBanner(FlashMenuBannerView* menu) {
    if (!((CFlashMemoryMenu*)menu)->RecreateBpEntry())
        return 0;
    void* value = menu->banner;
    if (value != 0) {
        if (value != 0) {
            ((DestroyableView*)value)->destroy(1);
        }
        menu->banner = 0;
    }
    menu->banner = CBanner::Create();
    if (menu->banner == 0)
        return 0;
    return 1;
}
// Start the fade-in when the save-menu transition is enabled.
// unused is an unused ABI receiver; enabled == 0 has no side effects. This starts
// a global fade with duration zero: FadeState_StartFadeIn (fade_overlay_and_shared_file.cpp)
// immediately sets alpha=255 and the opaque state, with no interpolated updates.
void CFlashMemoryMenuSave::StartFadeIn(void* unused, int enabled) {
    if (!enabled)
        return;
    if (Sing_805DC608() == 0) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(&s_fadeOverlay);
            __register_global_object(&s_fadeOverlay, ResourceState_Dtor, &s_fadeOverlayDtorRegistration_FlashMenu);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = &s_fadeOverlay;
    }
    FadeState_StartFadeIn(Sing_805DC608(), 0);
}
// Enable alternate fade drawing once the inherited menu is ready.
int CFlashMemoryMenuSave::EnableAlternateFadeDraw() {
    if (!((CFlashMemoryMenu*)this)->IsResourceReady())
        return 0;
    if (Sing_805DC608() == 0) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(&s_fadeOverlay);
            __register_global_object(&s_fadeOverlay, ResourceState_Dtor, &s_fadeOverlayDtorRegistration_FlashMenu);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = &s_fadeOverlay;
    }
    FadeState_SetAlternateDrawEnabled(Sing_805DC608(), 1);
    return 1;
}
// Update the base menu and poll the banner resource using the frame delta.
// Returns 0 when the base update blocks, otherwise 1 even if banner remains busy.
// delta is ABI-forwarded only; the concrete banner poll in movie_controller_and_banner.cpp ignores
// that float register and has no timing parameter (see the save tick contract).
int CFlashMemoryMenu::UpdateAndPollBanner(float delta) {
    if (!((CFileIoMenu*)this)->Update())
        return 0;
    if (banner != 0)
        SaveBanner_PollInitializationAttempt(banner, delta);
    return 1;
}
// Consume the single allowed retry of a failed banner or record read.
int CFlashMemoryMenu::TryConsumeSingleIoRetry() {
    if (readRetryCount >= kSaveSingleReadRetryLimit)
        return 0;
    readRetryCount++;
    return 1;
}
// Test file-access status bits 0 and 2 when an access object exists.
int CFlashMemoryMenu::IsStorageSpaceFull() {
    if (fileIo == 0)
        return 0;
    return (((CFileIoAccess*)fileIo)->GetStorageFullFlags() & kSaveSpaceFullMask) != 0;
}
// Test file-access status bits 1 and 3 when an access object exists.
int CFlashMemoryMenu::IsStorageEntryTableFull() {
    if (fileIo == 0)
        return 0;
    return (((CFileIoAccess*)fileIo)->GetStorageFullFlags() & kSaveEntryTableFullMask) != 0;
}

// Build the localized banner image, attach its digest, and write the temporary banner file.
// Returns access creation status. Missing banner returns 0 without replacing
// slots; later failures can leave a new form and no access. Both local buffers
// are freed on every normal path; successful creation retains only the access.
int CFlashMemoryMenu::CreateBannerWriteFileIo() {
    if (banner == 0)
        return 0;
    void** saveFormSlot = &((FlashMenuSaveFormView*)this)->saveForm;
    if (*saveFormSlot != 0) {
        if (*saveFormSlot != 0) {
            ((DestroyableView*)*saveFormSlot)->destroy(1);
        }
        *saveFormSlot = 0;
    }
    *saveFormSlot = CSaveFormBanner::Create();
    if (*saveFormSlot == 0)
        return 0;
    SaveIO_BeginSystemGuard(this);
    void** fileAccessSlot = &fileIo;
    if (*fileAccessSlot != 0) {
        if (*fileAccessSlot != 0) {
            ((FileIoAccessView*)*fileAccessSlot)->destroy(1);
        }
        *fileAccessSlot = 0;
    }
    // Local image and digest buffers are freed after WriteFlash_Create returns;
    // creation does not transfer ownership of these allocations to this caller's slot.
    unsigned int sizeRequired = (unsigned int)SaveForm_GetStorageSize(*saveFormSlot);
    if (!sizeRequired)
        return 0;
    void* serializedRecord;
    void* buffer = 0;
    int success = 0;
    serializedRecord = __nwa__FUl(sizeRequired);
    if (serializedRecord != 0) {
        buffer = __nw__FUl(kSaveBannerImageBytes);
        if (buffer != 0) {
            SaveBanner_InitializeLocalizedImage(banner, buffer, 0);
            int size = (int)SaveForm_GetRecordSize(*saveFormSlot);
            if (SaveForm_ReplicateWithDigest(*saveFormSlot, serializedRecord, buffer, size)) {
                *fileAccessSlot = CFileIoAccessWriteFlash::Create(
                    g_temporarySaveBannerFilename, SaveForm_GetStorageSize(*saveFormSlot), serializedRecord);
                if (*fileAccessSlot != 0)
                    success = 1;
            }
        }
    }
    if (serializedRecord != 0)
        __dla__FPv(serializedRecord);
    if (buffer != 0)
        __dl__FPv(buffer);
    return success;
}

#pragma use_lmw_stmw on
// Check space for both save files, rounding each size to the registered storage block size.
// Returns access creation status. No file-size value is returned to the caller.
// Missing/zero block size, form or access allocation returns 0; local forms are
// destroyed, but the receiver's newly installed form remains. Rounding is per
// file, then the two sizes are captured by CheckFlash_Create; do not round a sum.
int CFlashMemoryMenu::PrepareStorageCheck() {
    void** saveFormSlot = &((FlashMenuSaveFormView*)this)->saveForm;
    if (*saveFormSlot != 0) {
        if (*saveFormSlot != 0) {
            ((DestroyableView*)*saveFormSlot)->destroy(1);
        }
        *saveFormSlot = 0;
    }
    *saveFormSlot = CSaveFormFlash::Create(1, s_flashSaveMd5Enabled);
    if (*saveFormSlot == 0)
        return 0;
    SaveIO_BeginSystemGuard(this);
    void** fileAccessSlot = &fileIo;
    if (*fileAccessSlot != 0) {
        if (*fileAccessSlot != 0) {
            ((FileIoAccessView*)*fileAccessSlot)->destroy(1);
        }
        *fileAccessSlot = 0;
    }
    void* recordForm = 0;
    void* bannerForm = 0;
    int success = 0;
    // Pointer-typed ABI result encodes a scalar allocation-block size in bytes.
    // roundedFileBytes is [record,banner], each ceil(size/block)*block.
    // CheckFlash_Create -> SetupWithBlockList (pop_item_dialog_and_loading_draw.cpp) allocates and copies
    // these two stack values before returning; recordForm/bannerForm are local
    // temporaries, distinct from the retained form.
    void* storageBlockBytes = FileIO_GetDeviceTransferAlignment(kFlashDeviceRegistryPrefix);
    if (storageBlockBytes != 0) {
        recordForm = CSaveFormFlash::Create(1, s_flashSaveMd5Enabled);
        if (recordForm != 0) {
            bannerForm = CSaveFormBanner::Create();
            if (bannerForm != 0) {
                unsigned int roundedFileBytes[2];
                unsigned int recordStorageBytes = (unsigned int)SaveForm_GetStorageSize(recordForm);
                roundedFileBytes[0] =
                    (unsigned int)storageBlockBytes *
                    (((unsigned int)storageBlockBytes + recordStorageBytes - 1) / (unsigned int)storageBlockBytes);
                unsigned int bannerStorageBytes = (unsigned int)SaveForm_GetStorageSize(bannerForm);
                roundedFileBytes[1] =
                    (unsigned int)storageBlockBytes *
                    (((unsigned int)storageBlockBytes + bannerStorageBytes - 1) / (unsigned int)storageBlockBytes);
                *fileAccessSlot = CFileIoAccessCheckFlash::Create(
                    g_saveDataFilename, SaveForm_GetStorageSize(*saveFormSlot), roundedFileBytes, 2);
                if (*fileAccessSlot != 0)
                    success = 1;
            }
        }
    }
    if (recordForm != 0) {
        if (recordForm != 0) {
            ((DestroyableView*)recordForm)->destroy(1);
        }
    }
    if (bannerForm != 0) {
        if (bannerForm != 0) {
            ((DestroyableView*)bannerForm)->destroy(1);
        }
    }
    return success;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804623C0", _ctors_u8027B374_0, FileIoMenu_InitializeFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU804623C0", _ctors_u8027B374_1, FlashMemoryMenu_InitializeFullTurnAngle);
#pragma pop
