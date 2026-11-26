// NAND (flash memory) load flow: CFlashMemoryMenuLoad's state machine runs the selection dialog,
// waits for avatar preloads, and draws the dialog and its submenus.
// The fade overlay and movie controller it uses are process-lifetime singletons.

#include <decomp/static_init.h>
// Save Data / NAND load, new-user creation and account deletion (0x8027D564).
// Mod lookup: CFlashMemoryMenuLoad::UpdateLoadStateMachine is a readiness-driven pipeline, not
// the video frame-rate setter. It calls the base readiness hook BEFORE touching
// 60fps-test site 0x8027D608), one switch case per admitted update, and I/O polling.
// dt is a finite nonnegative step in SECONDS, forwarded unchanged to base,
// create-account, name-entry and account-selection MPBase1_Process. It originates
// at CMainFrame::UpdateVirtualHooks -> CProjectG_Update -> task/UI processing;
// MPBase1 passes it to its process(float) hook (event_face_and_mpbase1_menu.cpp). It is not an absolute timestamp or
// BP integer animation tick. Zero polls readiness without advancing dt timers;
// there is no local maximum/clamp, and large steps may cross child transitions.
// No timer or frame-count tuning
// constant is defined here. A faster caller can observe completion sooner, but
// must never skip pending I/O, the single-retry policy, or user confirmation.
// State map (FlashLoadState numeric values are stored in the ABI at +0x48):
//   0 -> open(1) -> banner read(2/3) -> save read(4/5) -> select account(10/11).
//   missing data: acknowledge(6) -> create choice(7) -> capacity check(8/9).
//   empty slot: name(13/14) -> account creation(15); delete: mark(12).
//   commit path: save write(20/21), save commit(22/23), banner write(16/17),
//   banner commit(18/19), then created/deleted message(24/25). Keep that order.
//   full storage/entries(31/32) -> HOME guide(33) -> continue-without-save(36/37).
//   damaged data(34/35), fatal(27/28), busy(29/30), write failure(38/39) lead to
//   advice/abort; abort(40) latches aborted then closes(26) -> complete(41).
// Message markup lives in fileIoDialogStrings (16-bit code units), not C ASCII.
// UI rendering uses CFileIoMenu::Draw then owned account/name/selection children;
// no aspect ratio, viewport, scissor or presentation-interval settings live here.
// Required receiver for every method. Child menus and preload entries are owned;
// avatarPreloads is an existing writable array, count must be 0..2 (two IDs below).
// Destroy entries before replacement, keep array allocation with its owning menu.
// Allocated modal sizes are ABI sizes, not tunable pool limits. Allocator/ctor
// failure may leave partial state; constructor exceptions free only the raw block
// and rethrow. Preserve that cleanup and deferred completion protocol.
#include "classes/CSharedDoc.h"
#include "classes/CFlashMemoryMenuLoad.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
// Compare preload iterators without changing the template's reference access shape.
template <class T> inline bool same_pos(const T& lhs, const T& rhs) {
    return lhs == rhs;
}

#include "game/singletons.h"
#include "classes/CFlashMemoryMenu.h"
#include "classes/CLoadDataMenu.h"
#include "classes/CPreloadAvatarAsync.h"
#include "classes/CFileIoMenu.h"
#include "classes/CCreateNewAccount.h"
#include "classes/CNameEntryMain.h"
extern "C" {
void __dl__FPv(void*);

// NAND load/create/delete flow. Each state advances only after its I/O or dialog completes.
enum FlashLoadState {
    FlashLoad_Begin = 0,
    FlashLoad_WaitForOpen = 1,
    FlashLoad_BeginBannerRead = 2,
    FlashLoad_WaitForBannerRead = 3,
    FlashLoad_BeginSaveRead = 4,
    FlashLoad_WaitForSaveRead = 5,
    FlashLoad_WaitForNoDataMessage = 6,
    FlashLoad_WaitForCreateSaveChoice = 7,
    FlashLoad_BeginStorageCheck = 8,
    FlashLoad_WaitForStorageCheck = 9,
    FlashLoad_OpenAccountSelection = 10,
    FlashLoad_ProcessAccountSelection = 11,
    FlashLoad_BeginDeleteAccount = 12,
    FlashLoad_OpenNameEntry = 13,
    FlashLoad_WaitForNameEntry = 14,
    FlashLoad_WaitForCreateAccount = 15,
    FlashLoad_BeginBannerWrite = 16,
    FlashLoad_WaitForBannerWrite = 17,
    FlashLoad_BeginBannerCommit = 18,
    FlashLoad_WaitForBannerCommit = 19,
    FlashLoad_BeginSaveWrite = 20,
    FlashLoad_WaitForSaveWrite = 21,
    FlashLoad_BeginSaveCommit = 22,
    FlashLoad_WaitForSaveCommit = 23,
    FlashLoad_WaitForCreatedMessage = 24,
    FlashLoad_WaitForDeletedMessage = 25,
    FlashLoad_Close = 26,
    FlashLoad_ShowFatalError = 27,
    FlashLoad_WaitForFatalError = 28,
    FlashLoad_ShowBusyError = 29,
    FlashLoad_WaitForBusyError = 30,
    FlashLoad_WaitForFullStorageMessage = 31,
    FlashLoad_WaitForFullEntriesMessage = 32,
    FlashLoad_WaitForHomeMenuGuide = 33,
    FlashLoad_WaitForInvalidSaveMessage = 34,
    FlashLoad_WaitForInvalidSaveGuide = 35,
    FlashLoad_AskToContinueWithoutSaving = 36,
    FlashLoad_WaitForContinueWithoutSavingChoice = 37,
    FlashLoad_ShowWriteFailure = 38,
    FlashLoad_WaitForWriteFailure = 39,
    FlashLoad_Abort = 40,
    FlashLoad_Complete = 41,
};

// CLoadDataMenu: confirmed deletion returns 2; selecting an occupied or empty slot returns 3.
enum AccountSelectionResult {
    Selection_Cancel = 1,
    Selection_DeleteConfirmed = 2,
    Selection_SlotChosen = 3
};
enum FlashLoadAllocationBytes {
    kAccountSelectionMenuBytes = 0x20F4,
    kNameEntryMenuBytes = 0x858,
    kCreateAccountMenuBytes = 0x44
};

// Pointer-preserving view of CFlashMemoryMenuLoad; shared header still stores these owners as ints.
struct FlashLoadMenuView {
    void** vtbl;
    char pad04[0x44];
    int state;
    void* createAccountMenu;
    void* nameEntryMenu;
    unsigned char aborted;
    unsigned char createdNewSave;
    unsigned char skipAccountSelection;
    unsigned char deletingAccount;
    unsigned char storageCheckFailed;
    unsigned char invalidSaveData;
    char pad5A[2];
    void** avatarPreloads;
    int avatarPreloadCount;
    char pad64[4];
    void* loadDataMenu;
};

typedef char FlashLoadMenuView_size[(sizeof(FlashLoadMenuView) == sizeof(CFlashMemoryMenuLoad)) ? 1 : -1];
#define FLASH_MENU_OFFSET(f, o) typedef char FlashLoad_offset_##f[((int)&((FlashLoadMenuView*)0)->f == (o)) ? 1 : -1]
FLASH_MENU_OFFSET(state, 0x48);
FLASH_MENU_OFFSET(createAccountMenu, 0x4C);
FLASH_MENU_OFFSET(skipAccountSelection, 0x56);
FLASH_MENU_OFFSET(avatarPreloads, 0x5C);
FLASH_MENU_OFFSET(loadDataMenu, 0x68);
#undef FLASH_MENU_OFFSET

struct AvatarSelectionStorage {
    char pad[0x68];
};

void MPBase1_DrawIfInitialized(void*);
int MPBase1_Process(void*, float);
int SaveIO_SetSkipDamagedRecordAdvice(int);
// Resource-status provenance (predicates are Boolean, not NAND result codes):
// Predicate11: CFileIoMenu (+fileAccess) -> CFileIoAccess::resource -> virtual
// slot 11, implemented by the attached resource (file_io_menu_save_load_base.cpp -> file_io_access_read_write.cpp).
// Predicate8: CFlashMemoryMenu (+fileIo) -> CFileIoAccess::resource -> virtual
// slot 8 (flash_memory_menu_load_part1.cpp -> file_io_access_read_write.cpp). Missing accessor/resource returns false.
// After QueryFileAccessStatus completes, this flow gives slot 11 priority:
// true -> fatal; else slot 8 true -> retry once, then busy. Only then inspect
// QueryAccessFailure and record validity. Concrete flash domain, file_io_flash_and_write_access.cpp:
// The flash slot-11 predicate tests CFileIo_GetStatusCode()==0xC354; the slot-8 predicate tests
// ==0xC353. That is CFileIo::statusCode at +8 (getter file_io_pad_and_flash_io.cpp, setter
// CFileIo::SetStatusCode in cfileio_flash_file_operations.cpp). Open/read-write/save-area completion
// callbacks in file_io_flash_and_write_access.cpp publish this status and clear the busy byte; synchronous
// submission failures also publish status. These are engine file statuses,
// not raw NAND SDK result values. The menu's policy names are fatal for C354
// and retry/busy for C353; underlying platform-error translations stay with I/O.
// CFileIoPad's corresponding slot-8 and slot-11 predicates return false (pad_memory_menu_load.cpp), so these
// wrappers must remain polymorphic. False does not itself mean successful I/O.
int FileIo_SetSystemUiAndDriveErrorsEnabled(FlashLoadMenuView*, int);
int MPBase1_IsComplete(void*);
int SaveIO_ShouldSkipDamagedRecordAdvice();
void SoundManager_RequestBgmStop(void*, int);
void* ItemInventory_GetCharacterIdAt(void*, unsigned int);
void ItemSelection_Init(void*);
void ItemSelection_SetBaseItem(void*, void*);
void* __nw__FUl(unsigned long);
void __register_global_object(void*, void*, void*);
void MovieController_Ctor(void*);
void MovieController_Dtor();
void MovieController_ReleasePlaybackReference(void*);
void FadeState_StartFadeIn(void*, int);
// Big-endian 16-bit message markup; extents include each terminator.
struct FileIoDialogPool {
    unsigned short noData[20];
    unsigned short invalidData[20];
    unsigned short createNewData[26];
    unsigned short storageFull[17];
    unsigned short entriesFull[23];
    unsigned short deletedData[26];
    unsigned short createSuccess[27];
    unsigned short fatalError[24];
    unsigned short busy[17];
    unsigned short homeMenuGuide[28];
    unsigned short homeMenuEntriesGuide[29];
    unsigned short invalidDataGuide[20];
    unsigned short continueWithoutSave[20];
    unsigned short continueWithFullStorage[22];
    unsigned short createFailure[27];
};
extern FileIoDialogPool fileIoDialogStrings;
typedef char FileIoDialogPool_size[(sizeof(FileIoDialogPool) == 0x2B4) ? 1 : -1];
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[];
extern char s_fadeOverlayDtorRegistration_FileIoMenu[];
extern signed char s_movieControllerInitialized;
extern char s_movieController[];
extern char s_movieControllerDtorRegistration_Movie[];
extern void* g_soundManager;
extern CSharedDoc* g_sharedDoc;
extern unsigned int s_accountPreviewCharacterIndex0;
extern unsigned int s_accountPreviewCharacterIndex1;
extern void FadeOverlay_Init(void*);
extern void ResourceState_Dtor();
extern void FadeState_SetAlternateDrawEnabled(void*, int);
extern float kFloat_6_2831855;
extern float s_flashLoadFullTurn;

// +0x0C has different contracts: update avatar preloads, or query submenu cancellation.
// These partial interfaces are ABI slot views, not a claim that preload tasks and
// menu objects share one C++ base. Uncalled slots deliberately remain opaque.
struct FlashMenuVirtuals {
    // Menu vtable: +30 close, +34 beginOpen, +38 isOpen, +3C finishOpen, +40 prepareLoad, +44 clear slot, +48 select
    // slot.
    virtual int destroy(int);              // +08: 1 destroys/frees owned child
    virtual int updateOrGetCancelResult(); // +0C: receiver-dependent contract above
    virtual int isComplete();              // +10: nonzero means preload complete
    virtual int m0x14();
    virtual int m0x18();
    virtual int m0x1c();
    virtual int m0x20();
    virtual int m0x24();
    virtual int m0x28();
    virtual int m0x2c();
    virtual int finishClose(); // +30: complete menu closure
    virtual int beginOpen();   // +34: request menu opening
    virtual int isOpen();      // +38: poll opening completion
    virtual int finishOpen();  // +3C: finalize open resources
    virtual int prepareLoad(); // +40: begin derived load setup
    virtual int m0x44();
    virtual int selectAccountSlot(int); // +48: select slot in save singleton
    virtual int m0x4c();
};

// Borrow/publish the process-lifetime fade singleton, registering cleanup once.
static inline void EnsureFadeOverlay() {
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                     s_fadeOverlayDtorRegistration_FileIoMenu);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
}

// Borrow/publish the process-lifetime movie controller, registering cleanup once.
static inline void EnsureMovieController() {
    if (!Sing_805DC950()) {
        if (!s_movieControllerInitialized) {
            MovieController_Ctor(s_movieController);
            __register_global_object(s_movieController, (void*)MovieController_Dtor,
                                     s_movieControllerDtorRegistration_Movie);
            s_movieControllerInitialized = 1;
        }
        Sing_805DC950() = s_movieController;
    }
}

// Initialize the unit-local full-turn constant.
void FlashLoad_InitializeFullTurnConstant() {
    s_flashLoadFullTurn = kFloat_6_2831855;
}

// Report whether the NAND load flow has finished closing.
int CFlashMemoryMenuLoad::IsComplete() {
    FlashLoadMenuView* self = (FlashLoadMenuView*)this;
    return self->state == FlashLoad_Complete;
}

// Draw the file dialog and any active account/name/slot submenus.
void CFlashMemoryMenuLoad::DrawDialogAndSubmenus() {
    FlashLoadMenuView* self = (FlashLoadMenuView*)this;
    ((CFileIoMenu*)self)->Draw();
    if (self->createAccountMenu)
        MPBase1_DrawIfInitialized(self->createAccountMenu);
    if (self->nameEntryMenu)
        MPBase1_DrawIfInitialized(self->nameEntryMenu);
    if (self->loadDataMenu)
        MPBase1_DrawIfInitialized(self->loadDataMenu);
}

struct LoadDialogVirtuals {
    // Vtable: +0x44 clear slot = CFileIoMenu_InitBlankPlayerRecord; +0x48 select slot = CFileIoMenu_SelectAccountSlot.
    virtual void destroy(int);
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2C();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3C();
    virtual void v40();
    virtual void clearAccountSlot(int);  // +0x44: CFileIoMenu_InitBlankPlayerRecord initializes a blank slot.
    virtual void selectAccountSlot(int); // +0x48: CFileIoMenu_SelectAccountSlot selects the current slot.
    virtual void v4C();
    virtual void v50();
    virtual int isAccountLoadable(int);
    // +0x54: CFileIoMenu_IsSaveSlotOccupied.
};

// Dispatch cancellation, confirmed deletion, or slot selection; release the selection menu.
// loadDataMenu is required; call GetSelectionResult only after MPBase1_Process
// completes. Producer: darts_dialog_and_load_data_menu.cpp, CLoadDataMenu::GetSelectionResult reads
// CLoadDataMenu::result/selectedSlot. It writes selectedAccount only for a valid
// slot 0..3 and a nonzero/noncancel result; otherwise returns Selection_Cancel
// WITHOUT writing output. -1 initialization therefore remains safe on cancel.
// Result schema: 1 -> abort; 2 -> if occupied, clear slot and enter delete;
// 3 -> select slot, then occupied -> close / empty -> name entry.
// A delete result on an empty slot leaves state unchanged but destroys the child;
// callers must not synthesize that combination. Unknown results keep child/state.
// Return 1 for both pending and handled results, 0 for an unrecognized completed
// result (menu retained). The caller discards this status. This is not isComplete.
int CFlashMemoryMenuLoad::ProcessSelectionDialog(float dt) {
    FlashLoadMenuView* self = (FlashLoadMenuView*)this;
    int selectedAccount = -1;
    if (MPBase1_Process(self->loadDataMenu, dt)) {
        int selectionResult = ((CLoadDataMenu*)(self->loadDataMenu))->GetSelectionResult(&selectedAccount);
        switch (selectionResult) {
        case Selection_Cancel:
            self->state = FlashLoad_Abort;
            break;
        case Selection_DeleteConfirmed:
            if (((LoadDialogVirtuals*)self)->isAccountLoadable(selectedAccount)) {
                ((LoadDialogVirtuals*)self)->clearAccountSlot(selectedAccount);
                self->state = FlashLoad_BeginDeleteAccount;
            }
            break;
        case Selection_SlotChosen:
            ((LoadDialogVirtuals*)self)->selectAccountSlot(selectedAccount);
            if (((LoadDialogVirtuals*)self)->isAccountLoadable(selectedAccount))
                self->state = FlashLoad_Close;
            else
                self->state = FlashLoad_OpenNameEntry;
            break;
        default:
            return 0;
        }
        register void* selectionMenu = self->loadDataMenu;
        if (selectionMenu) {
            if (selectionMenu) {
                ((LoadDialogVirtuals*)selectionMenu)->destroy(1);
            }
            self->loadDataMenu = 0;
        }
    }
    return 1;
}

// Stop at the first incomplete nonnull avatar preload; an empty array is ready.
// Null entries are ready and later entries are not queried after an early failure.
static inline int AreAvatarPreloadsComplete(FlashLoadMenuView* s) {
    for (void** it = s->avatarPreloads; it != s->avatarPreloads + s->avatarPreloadCount; ++it) {
        void* p = *it;
        if (p && !((FlashMenuVirtuals*)p)->isComplete())
            return 0;
    }
    return 1;
}
// Advance asynchronous NAND reads, account selection, writes and recovery dialogs.
// Return 0 for a blocked base update or failed required action, 1 for a processed
// tick (also while waiting). Only IsComplete/state==41 reports overall completion.
// The state machine owns UI children until their completion/deletion branches;
// file access/dialog ownership belongs to CFileIoMenu. Global save selection,
// audio, fades and HOME/reset/drive-error gates change as their cases state.
// Shared-record protocol for every SetSharedRecordMode call below:
// wrapper flash_memory_menu_load_part1.cpp ignores self; SaveData_SetSharedRecordMode (save_data_records_and_defaults.cpp)
// interprets (enableSharedRecord, initializationFlags), not a gameplay mode ID.
// (0,0): use selected persistent slot; flags ignored when disabling.
// (1,0): rebuild shared temporary record, seed defaults, then merge character/
//        course availability across all four loaded slots (slots stay intact).
// (1,1): rebuild shared defaults without that merge, for absent/damaged data or
//        continuing without saving. Nonzero flags suppress the merge; only 0/1
//        are used here. Enabling always reinitializes, so it is not idempotent.
// These select the record returned by SaveData_GetCurrentRecord; they do not
// themselves write NAND. Keep mode selection paired with the read/error branch.
int CFlashMemoryMenuLoad::UpdateLoadStateMachine(float dt) {
    FlashLoadMenuView* self = (FlashLoadMenuView*)this;
    // Phase 1: readiness admission; no child updates occur if the base defers.
    if (!((CFlashMemoryMenu*)self)->UpdateAndPollBanner(dt))
        return 0;
    for (void** it = self->avatarPreloads; !same_pos(self->avatarPreloads + self->avatarPreloadCount, it); ++it) {
        void* p = *it;
        if (p)
            ((FlashMenuVirtuals*)p)->updateOrGetCancelResult();
    }
    if (self->createAccountMenu)
        MPBase1_Process(self->createAccountMenu, dt);
    if (self->nameEntryMenu)
        MPBase1_Process(self->nameEntryMenu, dt);
    // Phase 2: dispatch exactly one current state; transitions take effect next call.
    unsigned int state = (unsigned int)self->state;
    switch (state) {
    case FlashLoad_Begin:
        SaveIO_SetSkipDamagedRecordAdvice(0);
        if (!((FlashMenuVirtuals*)self)->beginOpen())
            return 0;
        ((FlashMenuVirtuals*)self)->prepareLoad();
        self->state = FlashLoad_WaitForOpen;
        break;
    case FlashLoad_WaitForOpen:
        if (((FlashMenuVirtuals*)self)->isOpen()) {
            if (!((FlashMenuVirtuals*)self)->finishOpen())
                return 0;
            if (!((CFileIoMenu*)self)->SetScriptEnabled(0))
                return 0;
            ((CFlashMemoryMenu*)self)->ClearResult();
            self->state = FlashLoad_BeginBannerRead;
        }
        break;
    case FlashLoad_BeginBannerRead:
        if (!((CFlashMemoryMenu*)self)->PrepareBannerRead())
            return 0;
        self->state = FlashLoad_WaitForBannerRead;
        break;
    case FlashLoad_WaitForBannerRead:
        if (((CFileIoMenu*)self)->QueryFileAccessStatus()) {
            if (((CFileIoMenu*)self)->QueryResourcePredicate11()) {
                self->state = FlashLoad_ShowFatalError;
                break;
            }
            if (((CFlashMemoryMenu*)self)->QueryResourcePredicate8()) {
                if (((CFlashMemoryMenu*)self)->TryConsumeSingleIoRetry())
                    self->state = FlashLoad_BeginBannerRead;
                else
                    self->state = FlashLoad_ShowBusyError;
                break;
            }
            if (((CFileIoMenu*)self)->QueryAccessFailure()) {
                if (self->skipAccountSelection) {
                    // A new Wii save starts with one account, created by the Create User Data flow.
                    ((CFlashMemoryMenu*)self)->SetSharedRecordMode(1, 1);
                    self->state = FlashLoad_Close;
                } else {
                    if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog((const char*)fileIoDialogStrings.noData))
                        return 0;
                    self->state = FlashLoad_WaitForNoDataMessage;
                }
                break;
            }
            if (((CFileIoMenu*)self)->HasValidSaveRecord()) {
                ((CFlashMemoryMenu*)self)->ClearResult();
                self->state = FlashLoad_BeginSaveRead;
                break;
            }
            if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog((const char*)fileIoDialogStrings.invalidData))
                return 0;
            self->state = FlashLoad_WaitForInvalidSaveMessage;
        }
        break;
    case FlashLoad_BeginSaveRead:
        if (!((CFlashMemoryMenu*)self)->PrepareSaveDataRead())
            return 0;
        self->state = FlashLoad_WaitForSaveRead;
        break;
    case FlashLoad_WaitForSaveRead:
        if (((CFileIoMenu*)self)->QueryFileAccessStatus()) {
            if (((CFileIoMenu*)self)->QueryResourcePredicate11()) {
                self->state = FlashLoad_ShowFatalError;
                break;
            }
            if (((CFlashMemoryMenu*)self)->QueryResourcePredicate8()) {
                if (((CFlashMemoryMenu*)self)->TryConsumeSingleIoRetry())
                    self->state = FlashLoad_BeginSaveRead;
                else
                    self->state = FlashLoad_ShowBusyError;
                break;
            }
            if (((CFileIoMenu*)self)->QueryAccessFailure()) {
                if (self->skipAccountSelection) {
                    ((CFlashMemoryMenu*)self)->SetSharedRecordMode(1, 1);
                    self->state = FlashLoad_Close;
                } else {
                    if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog((const char*)fileIoDialogStrings.noData))
                        return 0;
                    self->state = FlashLoad_WaitForNoDataMessage;
                }
                break;
            }
            if (((CFileIoMenu*)self)->HasValidSaveRecord()) {
                if (!((CFlashMemoryMenu*)self)->LoadValidSaveRecord())
                    return 0;
                if (self->skipAccountSelection) {
                    // Without save data the original runs its first-start flow (no-data dialogs, Manage Data, name entry, Create User
                    // Data); multiplayer loads the shared record without asking for a slot.
                    ((CFlashMemoryMenu*)self)->SetSharedRecordMode(1, 0);
                    self->state = FlashLoad_Close;
                } else {
                    ((CFlashMemoryMenu*)self)->SetSharedRecordMode(0, 0);
                    self->createdNewSave = 0;
                    self->state = FlashLoad_OpenAccountSelection;
                }
                break;
            }
            if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog((const char*)fileIoDialogStrings.invalidData))
                return 0;
            self->state = FlashLoad_WaitForInvalidSaveMessage;
        }
        break;
    case FlashLoad_WaitForNoDataMessage:
        if (((CFileIoMenu*)self)->IsCommonDialogComplete()) {
            if (!((CFileIoMenu*)self)->ShowOkCancelDialog((const char*)fileIoDialogStrings.createNewData))
                return 0;
            self->state = FlashLoad_WaitForCreateSaveChoice;
        }
        break;
    case FlashLoad_WaitForCreateSaveChoice:
        if (((CFileIoMenu*)self)->IsCommonDialogComplete()) {
            if (((CFileIoMenu*)self)->IsCommonDialogAccepted()) {
                ((CFlashMemoryMenu*)self)->ClearResult();
                self->state = FlashLoad_BeginStorageCheck;
            } else {
                self->state = FlashLoad_AskToContinueWithoutSaving;
            }
        }
        break;
    case FlashLoad_BeginStorageCheck:
        if (!((CFlashMemoryMenu*)self)->PrepareStorageCheck())
            return 0;
        self->state = FlashLoad_WaitForStorageCheck;
        break;
    case FlashLoad_WaitForStorageCheck:
        if (((CFileIoMenu*)self)->QueryFileAccessStatus()) {
            if (((CFlashMemoryMenu*)self)->QueryResourcePredicate8()) {
                if (((CFlashMemoryMenu*)self)->TryConsumeSingleIoRetry())
                    self->state = FlashLoad_BeginStorageCheck;
                else
                    self->state = FlashLoad_ShowBusyError;
                break;
            }
            if (((CFlashMemoryMenu*)self)->IsStorageSpaceFull()) {
                if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog((const char*)fileIoDialogStrings.storageFull))
                    return 0;
                self->state = FlashLoad_WaitForFullStorageMessage;
                break;
            }
            if (((CFlashMemoryMenu*)self)->IsStorageEntryTableFull()) {
                if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog((const char*)fileIoDialogStrings.entriesFull))
                    return 0;
                self->state = FlashLoad_WaitForFullEntriesMessage;
                break;
            }
            self->createdNewSave = 1;
            self->state = FlashLoad_OpenAccountSelection;
        }
        break;
    case FlashLoad_OpenAccountSelection: {
        ((CFlashMemoryMenu*)self)->RestoreSystemUiAfterLoad();
        ((CFileIoMenu*)self)->DestroyCommonDialog();
        self->deletingAccount = 0;
        EnsureFadeOverlay();
        FadeState_SetAlternateDrawEnabled(Sing_805DC608(), 0);
        void* p = __nw__FUl(kAccountSelectionMenuBytes);
        if (p) {
            try {
                ((CLoadDataMenu*)p)->__ct();
            } catch (...) {
                __dl__FPv(p);
                throw;
            }
        }
        // Publication requires successful allocation for progress. A null p is
        // still published and state 11 entered; ProcessSelectionDialog next
        // calls MPBase1_Process(null,dt), whose receiver access is unconditional.
        // There is no retry, cancel result or OOM recovery. A constructor throw
        // frees p and leaves state 10 for the propagating exception; it does not
        // publish p. Do not describe null allocation as a successful empty menu.
        self->loadDataMenu = p;
        self->state = FlashLoad_ProcessAccountSelection;
        break;
    }
    case FlashLoad_ProcessAccountSelection:
        ((CFlashMemoryMenuLoad*)self)->ProcessSelectionDialog(dt);
        break;
    case FlashLoad_BeginDeleteAccount:
        self->deletingAccount = 1;
        if (!((CFileIoMenu*)self)->SetScriptEnabled(1))
            return 0;
        self->state = FlashLoad_BeginSaveWrite;
        break;
    case FlashLoad_OpenNameEntry: {
        // Owned replacement contract: constructed menu owns a writable array of
        // count 0..2 pointers. Each old nonnull loader is destroyed and nulled
        // BEFORE any replacement. ids[] entries must be valid inventory ordinal
        // lookups; a missing character or failed factory leaves that slot null.
        // Null is admitted as ready by AreAvatarPreloadsComplete. Valid loaders
        // receive +0C updates every admitted tick and +10 completion queries;
        // do not use submenu cancellation's interpretation of the +0C slot.
        for (void** it = self->avatarPreloads; !same_pos(self->avatarPreloads + self->avatarPreloadCount, it); ++it) {
            void* p = *it;
            if (p) {
                if (p) {
                    ((FlashMenuVirtuals*)p)->destroy(1);
                }
                *it = 0;
            }
        }
        unsigned int ids[2];
        ids[0] = s_accountPreviewCharacterIndex0;
        ids[1] = s_accountPreviewCharacterIndex1;
        AvatarSelectionStorage buf;
        // Stack selection is borrowed only during queue setup by
        // CPreloadAvatarAsync::Create (preload_avatar_wpuppet_async.cpp); the returned loader owns the
        // queued resource work. Do not retain &buf in a future async extension.
        for (void** it = self->avatarPreloads; !same_pos(self->avatarPreloads + self->avatarPreloadCount, it); ++it) {
            int idx = (int)(it - self->avatarPreloads);
            void* r = ItemInventory_GetCharacterIdAt(g_sharedDoc->itemInventory, ids[idx]);
            if (r) {
                ItemSelection_Init(&buf);
                ItemSelection_SetBaseItem(&buf, r);
                *it = CPreloadAvatarAsync::Create(&buf, 0, 0, 1);
            }
        }
        register void* p = self->nameEntryMenu;
        if (p) {
            if (p) {
                ((FlashMenuVirtuals*)p)->destroy(1);
            }
            self->nameEntryMenu = 0;
        }
        void* q = __nw__FUl(kNameEntryMenuBytes);
        if (q) {
            try {
                ((CNameEntryMain*)q)->__ct();
            } catch (...) {
                __dl__FPv(q);
                throw;
            }
        }
        self->nameEntryMenu = q;
        if (!q)
            return 0;
        EnsureFadeOverlay();
        FadeState_SetAlternateDrawEnabled(Sing_805DC608(), 0);
        self->state = FlashLoad_WaitForNameEntry;
        break;
    }
    case FlashLoad_WaitForNameEntry: {
        int ready = 1;
        if (self->nameEntryMenu) {
            if (!MPBase1_IsComplete(self->nameEntryMenu))
                ready = 0;
        }
        if (ready) {
            if (AreAvatarPreloadsComplete(self)) {
                int flag = 0;
                if (self->nameEntryMenu && ((FlashMenuVirtuals*)self->nameEntryMenu)->updateOrGetCancelResult())
                    flag = 1;
                register void* p = self->nameEntryMenu;
                if (p) {
                    if (p) {
                        ((FlashMenuVirtuals*)p)->destroy(1);
                    }
                    self->nameEntryMenu = 0;
                }
                if (flag) {
                    EnsureFadeOverlay();
                    FadeState_SetAlternateDrawEnabled(Sing_805DC608(), 1);
                    self->state = FlashLoad_Abort;
                    break;
                }
                p = self->createAccountMenu;
                if (p) {
                    if (p) {
                        ((FlashMenuVirtuals*)p)->destroy(1);
                    }
                    self->createAccountMenu = 0;
                }
                void* q = __nw__FUl(kCreateAccountMenuBytes);
                if (q) {
                    try {
                        ((CCreateNewAccount*)q)->__ct();
                    } catch (...) {
                        __dl__FPv(q);
                        throw;
                    }
                }
                self->createAccountMenu = q;
                if (!q)
                    return 0;
                self->state = FlashLoad_WaitForCreateAccount;
            }
        }
        break;
    }
    case FlashLoad_WaitForCreateAccount: {
        int ready = 1;
        if (self->createAccountMenu) {
            if (!MPBase1_IsComplete(self->createAccountMenu))
                ready = 0;
        }
        if (!ready)
            break;
        int flag = 0;
        if (self->createAccountMenu && ((FlashMenuVirtuals*)self->createAccountMenu)->updateOrGetCancelResult())
            flag = 1;
        register void* p = self->createAccountMenu;
        if (p) {
            if (p) {
                ((FlashMenuVirtuals*)p)->destroy(1);
            }
            self->createAccountMenu = 0;
        }
        EnsureFadeOverlay();
        FadeState_SetAlternateDrawEnabled(Sing_805DC608(), 1);
        if (flag) {
            p = self->nameEntryMenu;
            if (p) {
                if (p) {
                    ((FlashMenuVirtuals*)p)->destroy(1);
                }
                self->nameEntryMenu = 0;
            }
            void* q = __nw__FUl(kNameEntryMenuBytes);
            if (q) {
                try {
                    ((CNameEntryMain*)q)->__ct();
                } catch (...) {
                    __dl__FPv(q);
                    throw;
                }
            }
            self->nameEntryMenu = q;
            if (!q)
                return 0;
            EnsureFadeOverlay();
            FadeState_SetAlternateDrawEnabled(Sing_805DC608(), 0);
            EnsureFadeOverlay();
            FadeState_StartFadeIn(Sing_805DC608(), 0);
            self->state = FlashLoad_WaitForNameEntry;
        } else {
            SoundManager_RequestBgmStop(g_soundManager, 0);
            ((CFlashMemoryMenu*)self)->ClearResult();
            if (SaveIO_ShouldSkipDamagedRecordAdvice()) {
                EnsureMovieController();
                MovieController_ReleasePlaybackReference(Sing_805DC950());
                self->state = FlashLoad_Close;
            } else {
                if (!((CFileIoMenu*)self)->SetScriptEnabled(1))
                    return 0;
                self->state = FlashLoad_BeginSaveWrite;
            }
        }
        break;
    }
    case FlashLoad_BeginBannerWrite:
        if (!((CFlashMemoryMenu*)self)->CreateBannerWriteFileIo())
            return 0;
        self->state = FlashLoad_WaitForBannerWrite;
        break;
    case FlashLoad_WaitForBannerWrite:
        if (((CFileIoMenu*)self)->QueryFileAccessStatus()) {
            if (((CFileIoMenu*)self)->QueryResourcePredicate11()) {
                self->state = FlashLoad_ShowFatalError;
                break;
            }
            if (((CFlashMemoryMenu*)self)->QueryResourcePredicate8()) {
                if (((CFlashMemoryMenu*)self)->TryConsumeSingleIoRetry())
                    self->state = FlashLoad_BeginBannerWrite;
                else
                    self->state = FlashLoad_ShowBusyError;
                break;
            }
            if (((CFileIoMenu*)self)->QueryAccessFailure()) {
                self->state = FlashLoad_ShowWriteFailure;
                break;
            }
            ((CFlashMemoryMenu*)self)->ClearResult();
            self->state = FlashLoad_BeginBannerCommit;
        }
        break;
    case FlashLoad_BeginBannerCommit:
        if (!((CFlashMemoryMenu*)self)->CommitBannerFile())
            return 0;
        self->state = FlashLoad_WaitForBannerCommit;
        break;
    case FlashLoad_WaitForBannerCommit:
        if (((CFileIoMenu*)self)->QueryFileAccessStatus()) {
            if (((CFileIoMenu*)self)->QueryResourcePredicate11()) {
                self->state = FlashLoad_ShowFatalError;
                break;
            }
            if (((CFlashMemoryMenu*)self)->QueryResourcePredicate8()) {
                if (((CFlashMemoryMenu*)self)->TryConsumeSingleIoRetry())
                    self->state = FlashLoad_BeginBannerCommit;
                else
                    self->state = FlashLoad_ShowBusyError;
                break;
            }
            if (((CFileIoMenu*)self)->QueryAccessFailure()) {
                self->state = FlashLoad_ShowWriteFailure;
                break;
            }
            ((CFileIoMenu*)self)->SetScriptEnabled(0);
            if (self->deletingAccount) {
                EnsureFadeOverlay();
                FadeState_SetAlternateDrawEnabled(Sing_805DC608(), 1);
                ((CFileIoMenu*)self)->ShowAcknowledgeDialog((const char*)fileIoDialogStrings.deletedData);
                self->state = FlashLoad_WaitForDeletedMessage;
            } else if (self->createdNewSave) {
                ((CFileIoMenu*)self)->ShowAcknowledgeDialog((const char*)fileIoDialogStrings.createSuccess);
                self->state = FlashLoad_WaitForCreatedMessage;
            } else {
                self->state = FlashLoad_Close;
            }
        }
        break;
    case FlashLoad_BeginSaveWrite:
        if (!((CFlashMemoryMenu*)self)->CreateSaveWriteFileIo())
            return 0;
        self->state = FlashLoad_WaitForSaveWrite;
        break;
    case FlashLoad_WaitForSaveWrite:
        if (((CFileIoMenu*)self)->QueryFileAccessStatus()) {
            if (((CFileIoMenu*)self)->QueryResourcePredicate11()) {
                self->state = FlashLoad_ShowFatalError;
                break;
            }
            if (((CFlashMemoryMenu*)self)->QueryResourcePredicate8()) {
                if (((CFlashMemoryMenu*)self)->TryConsumeSingleIoRetry())
                    self->state = FlashLoad_BeginSaveWrite;
                else
                    self->state = FlashLoad_ShowBusyError;
                break;
            }
            if (((CFileIoMenu*)self)->QueryAccessFailure()) {
                self->state = FlashLoad_ShowWriteFailure;
                break;
            }
            ((CFlashMemoryMenu*)self)->ClearResult();
            self->state = FlashLoad_BeginSaveCommit;
        }
        break;
    case FlashLoad_BeginSaveCommit:
        if (!((CFlashMemoryMenu*)self)->CommitSaveFile())
            return 0;
        self->state = FlashLoad_WaitForSaveCommit;
        break;
    case FlashLoad_WaitForSaveCommit:
        if (((CFileIoMenu*)self)->QueryFileAccessStatus()) {
            if (((CFileIoMenu*)self)->QueryResourcePredicate11()) {
                self->state = FlashLoad_ShowFatalError;
                break;
            }
            if (((CFlashMemoryMenu*)self)->QueryResourcePredicate8()) {
                if (((CFlashMemoryMenu*)self)->TryConsumeSingleIoRetry())
                    self->state = FlashLoad_BeginSaveCommit;
                else
                    self->state = FlashLoad_ShowBusyError;
                break;
            }
            if (((CFileIoMenu*)self)->QueryAccessFailure()) {
                self->state = FlashLoad_ShowWriteFailure;
                break;
            }
            ((CFlashMemoryMenu*)self)->ClearResult();
            self->state = FlashLoad_BeginBannerWrite;
        }
        break;
    case FlashLoad_WaitForCreatedMessage:
        if (((CFileIoMenu*)self)->IsCommonDialogComplete())
            self->state = FlashLoad_Close;
        break;
    case FlashLoad_WaitForDeletedMessage:
        if (((CFileIoMenu*)self)->IsCommonDialogComplete())
            self->state = FlashLoad_OpenAccountSelection;
        break;
    case FlashLoad_Close:
        EnsureFadeOverlay();
        FadeState_SetAlternateDrawEnabled(Sing_805DC608(), 0);
        if (!((CFileIoMenu*)self)->SetScriptEnabled(0))
            return 0;
        if (!((FlashMenuVirtuals*)self)->finishClose())
            return 0;
        self->state = FlashLoad_Complete;
        break;
    case FlashLoad_ShowFatalError:
        if (!((CFileIoMenu*)self)->SetScriptEnabled(0))
            return 0;
        if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog((const char*)fileIoDialogStrings.fatalError))
            return 0;
        FileIo_SetSystemUiAndDriveErrorsEnabled(self, 0);
        self->state = FlashLoad_WaitForFatalError;
        break;
    case FlashLoad_WaitForFatalError:
        if (((CFileIoMenu*)self)->IsCommonDialogComplete()) {
            FileIo_SetSystemUiAndDriveErrorsEnabled(self, 1);
            self->state = FlashLoad_Abort;
        }
        break;
    case FlashLoad_ShowBusyError:
        ((CFileIoMenu*)self)->SetScriptEnabled(0);
        ((CFileIoMenu*)self)->ShowAcknowledgeDialog((const char*)fileIoDialogStrings.busy);
        self->state = FlashLoad_WaitForBusyError;
        break;
    case FlashLoad_WaitForBusyError:
        if (((CFileIoMenu*)self)->IsCommonDialogComplete())
            self->state = FlashLoad_Abort;
        break;
    case FlashLoad_WaitForFullStorageMessage:
        if (((CFileIoMenu*)self)->IsCommonDialogComplete()) {
            if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog((const char*)fileIoDialogStrings.homeMenuGuide))
                return 0;
            self->state = FlashLoad_WaitForHomeMenuGuide;
        }
        break;
    case FlashLoad_WaitForFullEntriesMessage:
        if (((CFileIoMenu*)self)->IsCommonDialogComplete()) {
            if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog((const char*)fileIoDialogStrings.homeMenuEntriesGuide))
                return 0;
            self->state = FlashLoad_WaitForHomeMenuGuide;
        }
        break;
    case FlashLoad_WaitForHomeMenuGuide:
        if (((CFileIoMenu*)self)->IsCommonDialogComplete())
            self->state = FlashLoad_AskToContinueWithoutSaving;
        self->storageCheckFailed = 1;
        break;
    case FlashLoad_WaitForInvalidSaveMessage:
        if (((CFileIoMenu*)self)->IsCommonDialogComplete()) {
            if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog((const char*)fileIoDialogStrings.invalidDataGuide))
                return 0;
            self->state = FlashLoad_WaitForInvalidSaveGuide;
        }
        break;
    case FlashLoad_WaitForInvalidSaveGuide:
        if (((CFileIoMenu*)self)->IsCommonDialogComplete())
            self->state = FlashLoad_Abort;
        self->invalidSaveData = 1;
        break;
    case FlashLoad_AskToContinueWithoutSaving: {
        const char* s = (const char*)fileIoDialogStrings.continueWithoutSave;
        if (self->storageCheckFailed)
            s = (const char*)fileIoDialogStrings.continueWithFullStorage;
        if (!((CFileIoMenu*)self)->ShowOkCancelDialog(s))
            return 0;
        self->state = FlashLoad_WaitForContinueWithoutSavingChoice;
        break;
    }
    case FlashLoad_WaitForContinueWithoutSavingChoice:
        if (((CFileIoMenu*)self)->IsCommonDialogComplete()) {
            if (((CFileIoMenu*)self)->IsCommonDialogAccepted()) {
                SaveIO_SetSkipDamagedRecordAdvice(1);
                ((FlashMenuVirtuals*)self)->selectAccountSlot(0);
                self->state = FlashLoad_OpenNameEntry;
            } else {
                self->state = FlashLoad_Abort;
            }
        }
        break;
    case FlashLoad_ShowWriteFailure:
        if (!((CFileIoMenu*)self)->SetScriptEnabled(0))
            return 0;
        if (!((CFileIoMenu*)self)->ShowAcknowledgeDialog((const char*)fileIoDialogStrings.createFailure))
            return 0;
        self->state = FlashLoad_WaitForWriteFailure;
        break;
    case FlashLoad_WaitForWriteFailure:
        if (((CFileIoMenu*)self)->IsCommonDialogComplete()) {
            if (self->deletingAccount)
                self->state = FlashLoad_Abort;
            else
                self->state = FlashLoad_AskToContinueWithoutSaving;
        }
        break;
    case FlashLoad_Abort:
        self->aborted = 1;
        self->state = FlashLoad_Close;
        break;
    default:
        break;
    }
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804623CC", _ctors_u8027D564_0, FlashLoad_InitializeFullTurnConstant);
#pragma pop
