// Save Data / file-I/O menu infrastructure (0x8027EB68): CFileIoMenu is the
// common UI/access/dialog layer used by NAND and Wii Remote load/save menus.
// Mod lookup:
// - Frame rate / save-load pacing: CFileIoMenu_Update first polls access slot
//   +0x0C. Zero defers ALL dialog/callback work this call. Preserve this gate;
//   it is readiness, with no 30/60 Hz counter or presentation interval here.
// - Dialog timing: kFloat_0_FileIoDialogUpdate is a zero delta passed to the
//   dialog's process slot +0x10; CPangyaCommonDialog_UpdateFadeAndInput ignores that float.
//   kDialogFadeUpdateCount=15 is the constructor's integer fade divisor, not
//   milliseconds: step=255/15=17 opacity units PER CALL. The strict >255 end test
//   makes a fade from zero take 16 admitted dialog updates. Faster polling changes
//   its wall time even though derived menus forward dt (flash_memory_menu_load_part2.cpp for NAND load).
// - Widescreen / menu-HUD: this unit only calls Draw on child UI and BP objects;
//   it sets no resolution, aspect ratio, viewport or scissor. Keep draw order
//   access -> modal -> BP. Their layouts/render contexts own positioning.
// - Message/button policy: FileIoDialogConstants and s_fileIoSavingScriptName /
//   s_fileIoSavingBpResourceName select the modal layout and now_saving animation.
// - Save-data policy/lifetime: InitBlankSaveRecordSet overwrites all live records;
//   vfunc_18 rebuilds a borrowed packed profile. Their contracts below give the
//   affected storage, admission policy and profile section map (save_data_records_and_inventory_init.cpp).
// Menu receivers are required except the deleting destructor (accepts null).
// All operations run on the menu thread; singleton helpers borrow process-lifetime
// storage and register shutdown cleanup exactly once. saveForm/fileAccess/modal
// are owned; bpObject is borrowed from bpEntryId and dies with its BP entry.
// Dialog SetText copies its input (CPangyaCommonDialog_SetText in common_dialog_yes_no.cpp), so
// caller text is borrowed native-Wii UTF-16 (u16 code units, NUL terminator),
// nullable to clear; never a narrow char*, message ID or MSL string object.
// SetText expands tagged text twice then owns the resulting string; a tag or
// surrogate pair must remain intact when editing text. Failure to allocate a replacement leaves
// commonDialog null and completion clear; system-UI side effects already happened.
// Shared headers CFileIoMenu/CPadMemoryMenuLoad describe the same base prefix;
// FileIoMenuState keeps native pointer access for CodeWarrior. No ownership cast.
extern "C" char s_homeButtonStateDtorRegistration_8027EB68[];
extern "C" char s_systemResetDtorRegistrationStorage[];
#include "classes/SavePlayerRecord.h"

// Only the file-I/O guard byte is used here; the reset state remains opaque.
struct FileIoResetGuardView { char padding[0xD]; unsigned char fileIoGuardActive; };
// CPangyaCommonDialog_ConfigureButtonLayout: 0 leaves OK/Cancel, 3 leaves
// Acknowledge, 4 disables all three. Result 1 is OK/Acknowledge, 2 is Cancel.
// Constructor's second argument is fade duration in process calls (common_dialog_yes_no.cpp).
enum FileIoDialogConstants {
    kInvalidBpEntry = -1, kDialogOkCancel = 0, kDialogAcknowledgeOnly = 3,
    kDialogNoButtons = 4, kDialogFadeUpdateCount = 15, kDialogAccepted = 1
};
// Translation unit 0x8027EB68-0x80280150 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CPadMemoryMenuLoad.h"
#include "classes/CFileIoMenu.h"
#include "classes/CPangyaCommonDialogFields.h"

// Cast-only adapters to the shared base schemas (no work/ownership change).
// Keep these expression views: inline function declarations change the compiler's
// anonymous exception-index symbol names even when generated text is identical.
// Actual singleton work is isolated in the named Ensure* helpers below.
#define AsCFileIoMenu(p) ((CFileIoMenu*)(p))
#define AsCPadMemoryMenuLoad(p) ((CPadMemoryMenuLoad*)(p))
extern "C" {

int CFileIoAccess_HasFailure(void* p); // CFileIoAccess_HasFailure.
void* SaveData_AcquireInitializedInstance();
void* SaveData_BuildCurrentPlayerSummary(void*);
void BpObject_SetEnabled(void* arg, int enabled);
int SaveData_InitializeBlankRecordSet(unsigned char* arg);
void* SaveData_GetFirstRecordStorage(void* arg); // SaveData_GetPlayerRecords.
void SaveData_SelectSlot(void* arg, void* value);
void SaveData_InitBlankRecord(void* arg, void* value, int flag);
int CFileIoAccess_QueryResourcePredicate11(void* arg);
void* CFileIoAccess_GetBuffer(void* arg);
int SaveForm_FindValidRecord(void* arg, void* value);
void BpObject_Draw(void* arg);
void BpObject_TryInvokeCurrentCallback(void* arg, int value);
void appFreeUni__FPv(void* arg);
void __dl__FPv(void* arg);
void __ct__7CBpCtrlFv(void* arg);
int CBpCtrl_PollAllEntriesLoaded(void* arg);
void* IdMap_Lookup(void* arg, int key);
void BpObject_StartScriptByName(void* arg, void* value, int flag);
void CBpCtrl_dtor(void* arg);
void __register_global_object(void* object, void (*dtor)(void*), void* reg);
void* __nw__FUl(unsigned int size);
void __ct__19CPangyaCommonDialogFPvi(void* arg, int a, int b);
int CPangyaCommonDialog_SetText(void* arg, void* value);
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_memoryMenuBpCtrlDtorRegistration[];
extern void* s_fileIoSavingScriptName; // Pointer to the now_saving script name.
extern float kFloat_0_FileIoDialogUpdate; // 0: kFloat_0_FileIoDialogUpdate.
void MPBase1_Ctor(void* arg);
extern char __vt__11CFileIoMenu[];
void SystemResetState_Ctor(void*);
void SystemResetState_Dtor(void*);
void HomeButtonMenu_InitializeState(void*);
void HomeButtonResources_dtor(void*);
void HomeMenu_OverrideBlockedState(void*, int);
void CCheckDriveStatus_SetErrorDialogsEnabled(int);
int FileIo_SetSystemButtonCallbacksEnabled(void*, int);
extern signed char s_systemResetStateInitialized;
extern signed char s_homeMenuInitialized;
extern char s_systemResetStorage[];
extern char s_homeButtonResourceState[];
extern char s_systemResetDtorRegistrationStorage[];
extern char s_homeButtonStateDtorRegistration_8027EB68[];

// Receiver/effect map for EVERY used virtual adapter below. Byte offsets include
// CodeWarrior's RTTI/null header words; historical type suffixes are not slots.
// receiver                      offset     effect / concrete target
// CFileIoMenu derived menu       +0x4C      borrow player record array:
//   CFileIoMenu_BorrowPlayerRecords -> SaveData_AcquireInitializedInstance ->
//   SaveData_GetFirstRecordStorage; four 0x828-byte records, no retain.
// CFileIoAccess derived          +0x08      terminal readiness (Object_IsStateFour
//   for Flash read); return is a state predicate, not an OS error number.
//                               +0x0C      Update: poll I/O state,0 defers menu;
//                               +0x10      draw hook (read/write vfunc_2, often no-op);
//                               +0x14      destroy(1): derived access + backing I/O.
// CPangyaCommonDialog            +0x08      dtor(1): free modal/resources;
//                               +0x10      vfunc_2(float ignored): animate/input,
//   return deferred completion, result remains a separate field;
//                               +0x14      vfunc_3: draw ready dialog.
// CSaveFormFlash/Banner/...      +0x08      vfunc_0(1): destroy owned save form.
// Evidence: phase1_class_tables::__vt__22CFileIoAccessReadFlash,
// __vt__19CPangyaCommonDialog and __vt__14CSaveFormFlash; implementations in
// file_io_pad_and_flash_io.cpp/file_io_access_read_write.cpp/common_dialog_yes_no.cpp and save-form units. All child pointers are
// borrowed by the call; deletion consumes the menu's ownership and nulls its slot.
// Uncalled padding declarations and VObjF1E0 are reserved, not hidden operations.
struct VObj80104 {
    virtual void pad0();
    virtual void pad1();
    virtual void pad2();
    virtual void pad3();
    virtual void pad4();
    virtual void pad5();
    virtual void pad6();
    virtual void pad7();
    virtual void pad8();
    virtual void pad9();
    virtual void pad10();
    virtual void pad11();
    virtual void pad12();
    virtual void pad13();
    virtual void pad14();
    virtual void pad15();
    virtual void pad16();
    virtual unsigned short* borrowPlayerRecords();
};

// Access +8 returns its terminal predicate; zero also represents absent access.
struct VObj8027F280 {
    virtual int isTerminal();
    virtual int m1();
    virtual int m2();
};

// Deleting-destructor views: argument 1 destroys the owned dialog and its allocation.
struct DialogDeleterSlot0 {
    virtual int destroy(int arg);
};

// Historical suffix Slot2 describes the old method LABEL, not a different
// offset: both modal-deleter views call +8 (first declared virtual), flag1=free.
struct DialogDeleterSlot2 {
    virtual int destroy(int arg);
};

// fileAccess/saveForm/commonDialog are owned; bpObject is borrowed from bpEntryId.
// messageStringData is owned only when messageStringFlags has its high bit set.
struct FileIoMenuState {
    void* vtable;
    char baseStorage[0x14];
    void* fileAccess;
    unsigned char fileAccessFlags;
    char pad1D[3];
    void* commonDialog;
    unsigned char commonDialogComplete;
    char pad25[3];
    void* saveForm;
    int bpEntryId;
    void* bpObject;
    unsigned int messageStringFlags;
    unsigned int messageStringLength;
    void* messageStringData;
};

// m3 polls file-access readiness (zero blocks this update); m4 advances a common
// dialog with the supplied float and returns completion, not the accepted result.
struct VObjEF24 {
    virtual void pad0();
    virtual int pollAccess();
    virtual int processDialog(float value);
};

// The two draw calls use distinct slots: m4 for file-access UI, m5 for dialogs.
struct VObjEFC4 {
    virtual void pad0();
    virtual void pad1();
    virtual void drawAccess();
    virtual void drawDialog();
};

// Access-object deletion uses a later slot than form/dialog deletion below.
struct VObjPtr18 {
    virtual void p0();
    virtual void p1();
    virtual void p2();
    virtual void destroyAccess(int);
};

struct VObjPtr28 {
    virtual void destroyFormOrDialog(int);
};

// Reserved partial menu view; unused slots must retain their order if extended.
struct VObjF1E0 {
    virtual void pad0();
    virtual void pad1();
    virtual void pad2();
    virtual void pad3();
    virtual void pad4();
    virtual void pad5();
    virtual void pad6();
    virtual void pad7();
    virtual void pad8();
    virtual void pad9();
    virtual void pad10();
    virtual void pad11();
    virtual void pad12();
    virtual void pad13();
    virtual void pad14();
    virtual void pad15();
    virtual void pad16();
    virtual void pad17();
    virtual int m18();
};

// Storage for CPangyaCommonDialog (0x44 bytes); second ctor argument is the integer
// fade-step divisor. Preserve call-count units, integer truncation and crossing test.
struct CommonDialogAlloc {
    char data[0x44];
    // Construct a dialog in its original allocation; a selects its button layout.
    CommonDialogAlloc(int a, int b) { __ct__19CPangyaCommonDialogFPvi(this, a, b); }
};

// Lazy singleton accessors. Each constructs its static object on first use (registering the destructor) and
// publishes it through the global slot: Sing_805DC948 = system reset/HOME-reset state, Sing_805DC820 = HOME
// button menu state, Sing_805DC5F0 = the BP controller (CBpCtrl).
// Publish the borrowed reset singleton, constructing/registering it once if needed.
static inline void EnsureSystemResetState() {
    if (Sing_805DC948() == 0) {
        if (s_systemResetStateInitialized == 0) {
            SystemResetState_Ctor(s_systemResetStorage);
            __register_global_object(s_systemResetStorage, SystemResetState_Dtor, s_systemResetDtorRegistrationStorage);
            s_systemResetStateInitialized = 1;
        }
        Sing_805DC948() = s_systemResetStorage;
    }
}
// Publish the borrowed HOME singleton, constructing/registering it once if needed.
static inline void EnsureHomeMenuState() {
    if (Sing_805DC820() == 0) {
        if (s_homeMenuInitialized == 0) {
            HomeButtonMenu_InitializeState(s_homeButtonResourceState);
            __register_global_object(s_homeButtonResourceState, HomeButtonResources_dtor, s_homeButtonStateDtorRegistration_8027EB68);
            s_homeMenuInitialized = 1;
        }
        Sing_805DC820() = s_homeButtonResourceState;
    }
}
// Publish the borrowed BP controller, constructing/registering it once if needed.
static inline void EnsureBpCtrl() {
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_memoryMenuBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

// Replace the modal dialog after enabling reset, HOME, and drive-error handling.
// value: nullable NUL-terminated UTF-16 text (see opening representation contract),
// borrowed until SetText returns; null produces an empty dialog. Returns creation
// success, not user acceptance. Text-expansion exceptions follow normal unwinding.
int CFileIoMenu_ShowAcknowledgeDialog(void* p, void* value) {
    // Phase 1: release I/O protection, then restore reset callbacks. Repeated
    // singleton tests/enable calls preserve the existing order and side effects.
    EnsureSystemResetState();
    ((FileIoResetGuardView*)Sing_805DC948())->fileIoGuardActive = 0;
    EnsureHomeMenuState();
    HomeMenu_OverrideBlockedState(Sing_805DC820(), 0);
    CCheckDriveStatus_SetErrorDialogsEnabled(1);
    EnsureSystemResetState();
    FileIo_SetSystemButtonCallbacksEnabled(Sing_805DC948(), 1);
    EnsureHomeMenuState();
    HomeMenu_OverrideBlockedState(Sing_805DC820(), 0);
    CCheckDriveStatus_SetErrorDialogsEnabled(1);
    // Phase 2: destroy the previous owned dialog before attempting replacement.
    // value is passed to SetText; allocation failure leaves no current dialog.
    void** slot = &AsCFileIoMenu(p)->commonDialog;
    void* old = *slot;
    // 0x8027F2BC: +0x20
    if (old != 0) {
        if (old != 0) {
            ((DialogDeleterSlot2*)old)->destroy(1);
        }
        *slot = 0;
    }
    AsCFileIoMenu(p)->commonDialogComplete = 0;
    CommonDialogAlloc* obj = new CommonDialogAlloc(kDialogAcknowledgeOnly, kDialogFadeUpdateCount);
    *slot = obj;
    if (obj == 0) return 0;
    CPangyaCommonDialog_SetText(obj, value);
    return 1;
}

// Open a modal dialog and restore system-button callbacks first.
// Same nullable UTF-16/text ownership and creation-result contract as the
// acknowledge variant; this selects the OK/Cancel button layout.
int CFileIoMenu_ShowOkCancelDialog(void* p, void* value) {
    // Same system-state sequence and replacement ownership as ShowAcknowledgeDialog;
    // this variant leaves OK/Cancel available instead of Acknowledge alone.
    EnsureSystemResetState();
    ((FileIoResetGuardView*)Sing_805DC948())->fileIoGuardActive = 0;
    EnsureHomeMenuState();
    HomeMenu_OverrideBlockedState(Sing_805DC820(), 0);
    CCheckDriveStatus_SetErrorDialogsEnabled(1);
    EnsureSystemResetState();
    FileIo_SetSystemButtonCallbacksEnabled(Sing_805DC948(), 1);
    EnsureHomeMenuState();
    HomeMenu_OverrideBlockedState(Sing_805DC820(), 0);
    CCheckDriveStatus_SetErrorDialogsEnabled(1);
    void** slot = &AsCFileIoMenu(p)->commonDialog;
    void* old = *slot;
    // 0x8027F4F0: +0x20
    if (old != 0) {
        if (old != 0) {
            ((DialogDeleterSlot2*)old)->destroy(1);
        }
        *slot = 0;
    }
    AsCFileIoMenu(p)->commonDialogComplete = 0;
    CommonDialogAlloc* obj = new CommonDialogAlloc(kDialogOkCancel, kDialogFadeUpdateCount);
    *slot = obj;
    if (obj == 0) return 0;
    CPangyaCommonDialog_SetText(obj, value);
    return 1;
}

int FileIo_SetSystemUiAndDriveErrorsEnabled(int, int);
int SaveIO_BeginSystemGuard();
int CFlashMemoryMenu_RestoreSystemUiAfterLoad();
int CBpCtrl_DeleteEntryById(void*, int);
int CBpCtrl_CreateEntry(void*, void*, int*, int);
extern void* s_fileIoSavingBpResourceName; // Pointer to now_saving.b2d.

// Set reset callbacks, HOME availability, and drive-error dialogs together.
// unused is an ignored ABI receiver; value is zero to block, nonzero to enable.
int FileIo_SetSystemUiAndDriveErrorsEnabled(int unused, int value) {
    EnsureSystemResetState();
    FileIo_SetSystemButtonCallbacksEnabled(Sing_805DC948(), value);
    if (value != 0) {
        EnsureHomeMenuState();
        HomeMenu_OverrideBlockedState(Sing_805DC820(), 0);
    } else {
        EnsureHomeMenuState();
        HomeMenu_OverrideBlockedState(Sing_805DC820(), 1);
    }
    CCheckDriveStatus_SetErrorDialogsEnabled(value);
    return 1;
}

// Block system UI while a save I/O operation is protected.
int SaveIO_BeginSystemGuard() {
    EnsureSystemResetState();
    int one = 1;
    ((FileIoResetGuardView*)Sing_805DC948())->fileIoGuardActive = one;
    EnsureHomeMenuState();
    HomeMenu_OverrideBlockedState(Sing_805DC820(), 1);
    CCheckDriveStatus_SetErrorDialogsEnabled(0);
    return 1;
}

// Release the load operation guard and restore HOME and error dialogs.
int CFlashMemoryMenu_RestoreSystemUiAfterLoad() {
    EnsureSystemResetState();
    ((FileIoResetGuardView*)Sing_805DC948())->fileIoGuardActive = 0;
    EnsureHomeMenuState();
    HomeMenu_OverrideBlockedState(Sing_805DC820(), 0);
    CCheckDriveStatus_SetErrorDialogsEnabled(1);
    return 1;
}

// Delete the old memory-menu BP entry and request its replacement.
int CFlashMemoryMenu_RecreateBpEntry(FileIoMenuState* p) {
    EnsureBpCtrl();
    if (p->bpEntryId != kInvalidBpEntry) {
        EnsureBpCtrl();
        CBpCtrl_DeleteEntryById(Sing_805DC5F0(), p->bpEntryId);
        p->bpEntryId = kInvalidBpEntry;
    }
    EnsureBpCtrl();
    int result = CBpCtrl_CreateEntry(Sing_805DC5F0(), s_fileIoSavingBpResourceName, &p->bpEntryId, 0);
    if (result == 0) return 0;
    return 1;
}

// Recreate the file-I/O menu BP entry; report whether creation succeeded.
int CFileIoMenu_RecreateBpEntry(FileIoMenuState* p) {
    EnsureBpCtrl();
    if (p->bpEntryId != kInvalidBpEntry) {
        EnsureBpCtrl();
        CBpCtrl_DeleteEntryById(Sing_805DC5F0(), p->bpEntryId);
        p->bpEntryId = kInvalidBpEntry;
    }
    EnsureBpCtrl();
    int result = CBpCtrl_CreateEntry(Sing_805DC5F0(), s_fileIoSavingBpResourceName, &p->bpEntryId, 0);
    if (result == 0) return 0;
    return 1;
}

// Restore system UI and reset callbacks after file I/O.
int CFileIoMenu_RestoreSystemUiAndCallbacks() {
    EnsureSystemResetState();
    int zero = 0;
    ((FileIoResetGuardView*)Sing_805DC948())->fileIoGuardActive = zero;
    EnsureHomeMenuState();
    HomeMenu_OverrideBlockedState(Sing_805DC820(), 0);
    CCheckDriveStatus_SetErrorDialogsEnabled(1);
    EnsureSystemResetState();
    FileIo_SetSystemButtonCallbacksEnabled(Sing_805DC948(), 1);
    EnsureHomeMenuState();
    HomeMenu_OverrideBlockedState(Sing_805DC820(), 0);
    CCheckDriveStatus_SetErrorDialogsEnabled(1);
    return 1;
}

// Test whether the requested player-record slot is occupied.
// index must be 0..3; borrowed record storage is contiguous 0x828-byte slots.
// Occupancy is solely firstNameCodeUnit != 0, not a checksum/load-validity test.
int CFileIoMenu_IsSaveSlotOccupied(VObj80104* p, int index) {
    SavePlayerRecord* records = (SavePlayerRecord*)p->borrowPlayerRecords();
    unsigned short value = records[index].firstNameCodeUnit;
    return value != 0;
}

// Destroy the current dialog and clear its completion flag.
int CFileIoMenu_DestroyCommonDialog(void* p) {
    void* value = AsCFileIoMenu(p)->commonDialog;
    if (value != 0) {
        if (value != 0) {
            ((DialogDeleterSlot0*)value)->destroy(1);
        }
        AsCFileIoMenu(p)->commonDialog = 0;
    }
    AsCFileIoMenu(p)->commonDialogComplete = 0;
    return 1;
}

// Initialize the file-access, dialog, BP-entry, and owned-string state.
// Lifecycle/guard timeline (global policy, not a per-menu nesting counter):
// 1 Ctor -> MPBase1_Ctor (event_face_and_mpbase1_menu.cpp) sets stopped/uninitialized/autoStart;
//   neither constructor acquires I/O guards or changes HOME/reset/error policy.
// 2 Actual read/write/verify entry points (flash_memory_menu_save.cpp Flash, pad_memory_menu_load.cpp PAD)
//   call SaveIO_BeginSystemGuard: reset +0xD=1, HOME blocked, drive errors off.
//   That operation does not itself disable system-button callbacks; those are
//   independently controlled by FileIo_SetSystemUiAndDriveErrorsEnabled(value).
// 3 Show*Dialog and vfunc_10 clear guard, unblock HOME/drive errors and enable
//   callbacks before dialog replacement/completion. RestoreSystemUiAfterLoad
//   clears guard/HOME/error policy only. See each body for duplicated calls.
// 4 Dtor deletes access -> form -> dialog -> BP entry, restores those globals
//   unconditionally, then frees owned text/allocation. No saved prior policy is
//   restored, and failed dialog allocation does not reacquire protection.
// Thus caller sequencing must prevent overlapping guarded menus; constructing
// a child is not a guard acquisition. Singleton lifetime exceeds all menus.
FileIoMenuState* CFileIoMenu_Ctor(FileIoMenuState* p) {
    MPBase1_Ctor(p);
    p->vtable = __vt__11CFileIoMenu;
    p->fileAccess = 0;
    p->fileAccessFlags = 0;
    p->commonDialog = 0;
    p->commonDialogComplete = 0;
    p->saveForm = 0;
    p->bpEntryId = kInvalidBpEntry;
    p->bpObject = 0;
    p->messageStringFlags = 0;
    p->messageStringLength = 0;
    p->messageStringData = 0;
    return p;
}

// Default readiness hook: this base menu is immediately ready.
int CFileIoMenu_vfunc_8() {
    return 1;
}

// Poll file access, advance the common dialog, and dispatch BP callbacks.
// Return 0 only while the access update defers progress; return 1 after this tick,
// including when no children exist. A 1 does not mean disk I/O or the modal is done.
// commonDialogComplete is updated only when a dialog exists. Keep the early return
// before that write and before callbacks; increasing update rate cannot bypass it.
int CFileIoMenu_Update(FileIoMenuState* p) {
    void* child = p->fileAccess;
    if (child != 0 && ((VObjEF24*)child)->pollAccess() == 0) {
        return 0;
    }
    child = p->commonDialog;
    if (child != 0) {
        p->commonDialogComplete = ((VObjEF24*)child)->processDialog(kFloat_0_FileIoDialogUpdate) != 0;
    }
    child = p->bpObject;
    if (child != 0) {
        BpObject_TryInvokeCurrentCallback(child, 0);
    }
    return 1;
}

// Draw file-access UI, the common dialog, and the menu BP object.
void CFileIoMenu_Draw(FileIoMenuState* p) {
    void* v = p->fileAccess;
    if (v != 0) {
        ((VObjEFC4*)v)->drawAccess();
    }
    v = p->commonDialog;
    if (v != 0) {
        ((VObjEFC4*)v)->drawDialog();
    }
    v = p->bpObject;
    if (v != 0) {
        BpObject_Draw(v);
    }
}

// Validate the file-access buffer against the menu save form.
// Borrow the access-owned buffer for validation; absence returns 0 without writes.
int CFileIoMenu_HasValidSaveRecord(FileIoMenuState* p) {
    void* v = p->fileAccess;
    if (v != 0 && p->saveForm != 0) {
        void* value = CFileIoAccess_GetBuffer(v);
        if (SaveForm_FindValidRecord(p->saveForm, value) != 0) {
            return 1;
        }
    }
    return 0;
}

// Return the completion flag recorded by the dialog update.
unsigned char CFileIoMenu_IsCommonDialogComplete(void* p) {
    return AsCFileIoMenu(p)->commonDialogComplete;
}

// Return the access object status, or zero when absent.
int CFileIoMenu_QueryFileAccessStatus(void* p) {
    VObj8027F280* v = (VObj8027F280*)AsCFileIoMenu(p)->fileAccess;
    if (v != 0) {
        return v->isTerminal();
    }
    return 0;
}

// Report nonzero backing-file status or a latched access failure, when access exists.
int CFileIoMenu_QueryAccessFailure(void* p) {
    void* v = AsCFileIoMenu(p)->fileAccess;
    if (v != 0) {
        return CFileIoAccess_HasFailure(v);
    }
    return 0;
}

// Destructively reset the LIVE shared save instance, not the access object's
// read buffer or a menu snapshot. SaveData_InitializeBlankRecordSet (save_data_records_and_defaults.cpp)
// resets header currentSlot/slotChanged/useSharedRecord/sharedInitializationFlags
// and calls SaveData_InitBlankRecord for normal slots 0..3 at +8+i*0x828 and
// the shared record at +0x20A8. Existing names, inventory/equipment, statistics,
// settings and progress in those records are replaced by starter defaults.
// The packed summary at +0x28D0 is not rebuilt by this reset and can be stale.
// Caller must admit this only when discarding ALL current records is intended
// (new-data/reset policy); there is no validity test, confirmation, disk write,
// or rollback here. Serialize against save/load and summary consumers. Any UI
// confirmation belongs to the caller; invoking the hook itself grants no policy.
void CFileIoMenu_InitBlankSaveRecordSet() {
    unsigned char* v = (unsigned char*)SaveData_AcquireInitializedInstance();
    SaveData_InitializeBlankRecordSet(v);
}

// Query the shared player-record array; this void hook discards the returned pointer.
void CFileIoMenu_BorrowPlayerRecords() {
    void* v = SaveData_AcquireInitializedInstance();
    SaveData_GetFirstRecordStorage(v);
}

// 0x802800E0: SaveData_AcquireInitializedInstance's r3 goes straight into SaveData_BuildCurrentPlayerSummary (0x802800EC-0x802800F0), whose result is
// returned (blr at 0x80280100); the menu caller consumes it at 0x80273C84.
// Rebuild and return the borrowed packed profile at shared SaveData+0x28D0.
// Schema (0x47C bytes, HEX source-record -> packed offset / byte length):
// 000->000/25C inventory/name/statistics; 2CC->25C/1A0 character outfits;
// 46C->3FC/40 caddies; 4AC->43C/1E club sets; 4CA->45A/0B balls;
// 4D8->468/14 settings. Padding +465..467 is not written. Full field/section
// views and copy policy are in save_data_records_and_inventory_init.cpp (SaveInventoryPrefix,
// PackedProfileSections, SaveData_BuildCurrentPlayerSummary). Slot-only course
// records and trailing progress are excluded. No heap ownership is transferred:
// DO NOT release/delete this pointer. It lives inside the singleton; the next
// summary build overwrites its bytes, while later record edits/resets leave the
// cached bytes stale until rebuilt. Copy the sections before retaining a snapshot.
// Header byte +5 selects shared record +20A8; otherwise currentSlot must be 0..3.
// CPadMemoryMenuSave_GetSaveRecord (pad_memory_menu_save.cpp) uses it only when its owned snapshot
// is absent; its write path copies the profile. This hook ignores the menu ABI
// receiver and performs no validation, saving or allocation of a new profile.
void* CFileIoMenu_BuildCurrentPlayerProfile() {
    return SaveData_BuildCurrentPlayerSummary(SaveData_AcquireInitializedInstance());
}

// Ensure the BP controller exists and poll all pending entries.
int CBpCtrl_PollSingletonEntriesLoaded() {
    EnsureBpCtrl();
    void* value = Sing_805DC5F0();
    int result = 0;
    if (value == 0 || CBpCtrl_PollAllEntriesLoaded(value) != 0) result = 1;
    if (result == 0) return 0;
    return 1;
}

// Replace the owned dialog with a buttonless common dialog; return allocation success.
int CFileIoMenu_CreateButtonlessCommonDialog(void* p, void* value) {
    void* old = AsCFileIoMenu(p)->commonDialog;
    if (old != 0) {
        if (old != 0) {
            ((DialogDeleterSlot0*)old)->destroy(1);
        }
        AsCFileIoMenu(p)->commonDialog = 0;
    }
    AsCFileIoMenu(p)->commonDialogComplete = 0;
    CommonDialogAlloc* obj = new CommonDialogAlloc(kDialogNoButtons, kDialogFadeUpdateCount);
    AsCFileIoMenu(p)->commonDialog = obj;
    if (obj == 0) return 0;
    CPangyaCommonDialog_SetText(obj, value);
    return 1;
}

// Resolve the loaded menu BP entry and start its initial script.
int CFileIoMenu_StartSavingScript(void* p) {
    EnsureBpCtrl();
    void* value = IdMap_Lookup(Sing_805DC5F0(), AsCPadMemoryMenuLoad(p)->bpEntryId);
    AsCPadMemoryMenuLoad(p)->bpObject = value;
    int result;
    if (value == 0) {
        result = 0;
    } else {
        BpObject_StartScriptByName(value, s_fileIoSavingScriptName, 1);
        result = 1;
    }
    if (result == 0) return 0;
    return 1;
}

// Report OK/Acknowledge acceptance; absence, pending and Cancel all return zero.
int CFileIoMenu_IsCommonDialogAccepted(void* p) {
    void* v = AsCFileIoMenu(p)->commonDialog;
    if (v == 0) {
        return 0;
    }
    return AsCPangyaCommonDialogFields(v)->result == kDialogAccepted; // 0x8027F258: common dialog +0x8.
}

// Enable or disable the menu BP object when present.
int CFileIoMenu_SetScriptEnabled(void* p, int enabled) {
    void* v = AsCFileIoMenu(p)->bpObject;
    if (v != 0) {
        BpObject_SetEnabled(v, enabled);
    }
    return 1;
}

// Test the backing file's slot-11 status predicate; flash callers treat it as a fatal status.
int CFileIoMenu_QueryResourcePredicate11(void* p) {
    int result = 0;
    void* v = AsCFileIoMenu(p)->fileAccess;
    if (v != 0 && CFileIoAccess_QueryResourcePredicate11(v) != 0) {
        result = 1;
    }
    return result;
}

// Initialize a blank player record in the shared save data.
// p is unused; value carries the slot index in an ABI pointer word, not record storage.
void CFileIoMenu_InitBlankPlayerRecord(void* p, void* value) {
    void* v = SaveData_AcquireInitializedInstance();
    SaveData_InitBlankRecord(v, value, 0);
}

// Select the supplied player slot in the shared save data.
// p is unused; value carries the slot index in an ABI pointer word.
void CFileIoMenu_SelectAccountSlot(void* p, void* value) {
    void* v = SaveData_AcquireInitializedInstance();
    SaveData_SelectSlot(v, value);
}

// Release owned access/form/dialog objects, restore system UI, and free the message string.
// flag > 0 also frees p; the returned original address then must not be dereferenced.
// Negative/zero flag only tears down contents. Preserve the repeated HOME/error
// restoration calls and the high-bit test before freeing packed-string heap data.
void* CFileIoMenu_dtor(FileIoMenuState* p, int flag) {
    register char* reg = s_memoryMenuBpCtrlDtorRegistration;
    if (p == 0) return p;
    // Phase 1: release owned access, save form and modal, each through its ABI slot.
    p->vtable = __vt__11CFileIoMenu;
    {
        void* v = p->fileAccess;
        if (v != 0) {
            if (v != 0) {
                ((VObjPtr18*)v)->destroyAccess(1);
            }
            p->fileAccess = 0;
        }
    }
    {
        void* v = p->saveForm;
        if (v != 0) {
            if (v != 0) {
                ((VObjPtr28*)v)->destroyFormOrDialog(1);
            }
            p->saveForm = 0;
        }
    }
    {
        void* v = p->commonDialog;
        if (v != 0) {
            if (v != 0) {
                ((VObjPtr28*)v)->destroyFormOrDialog(1);
            }
            p->commonDialog = 0;
        }
    }
    AsCFileIoMenu(p)->commonDialogComplete = 0;
    // Phase 2: unregister the BP entry (its object pointer is borrowed).
    // The inline singleton blocks in this function are the same lazy init as EnsureBpCtrl/EnsureSystemResetState/
    // EnsureHomeMenuState; they stay expanded because the registration address is held in the `reg` register variable.
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, reg + 0);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    if (p->bpEntryId != kInvalidBpEntry) {
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, CBpCtrl_dtor, reg + 0);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        CBpCtrl_DeleteEntryById(Sing_805DC5F0(), p->bpEntryId);
        p->bpEntryId = kInvalidBpEntry;
    }
    if (Sing_805DC948() == 0) {
        if (s_systemResetStateInitialized == 0) {
            SystemResetState_Ctor(s_systemResetStorage);
            __register_global_object(s_systemResetStorage, SystemResetState_Dtor, (reg + 0x18));
            s_systemResetStateInitialized = 1;
        }
        Sing_805DC948() = s_systemResetStorage;
    }
    {
            ((FileIoResetGuardView*)Sing_805DC948())->fileIoGuardActive = 0;
        if (Sing_805DC820() == 0) {
            if (s_homeMenuInitialized == 0) {
                HomeButtonMenu_InitializeState(s_homeButtonResourceState);
                __register_global_object(s_homeButtonResourceState, HomeButtonResources_dtor, (reg + 0x0C));
                s_homeMenuInitialized = 1;
            }
            Sing_805DC820() = s_homeButtonResourceState;
        }
    }
    HomeMenu_OverrideBlockedState(Sing_805DC820(), 0);
    CCheckDriveStatus_SetErrorDialogsEnabled(1);
    if (Sing_805DC948() == 0) {
        if (s_systemResetStateInitialized == 0) {
            SystemResetState_Ctor(s_systemResetStorage);
            __register_global_object(s_systemResetStorage, SystemResetState_Dtor, (reg + 0x18));
            s_systemResetStateInitialized = 1;
        }
        Sing_805DC948() = s_systemResetStorage;
    }
    FileIo_SetSystemButtonCallbacksEnabled(Sing_805DC948(), 1);
    if (Sing_805DC820() == 0) {
        if (s_homeMenuInitialized == 0) {
            HomeButtonMenu_InitializeState(s_homeButtonResourceState);
            __register_global_object(s_homeButtonResourceState, HomeButtonResources_dtor, (reg + 0x0C));
            s_homeMenuInitialized = 1;
        }
        Sing_805DC820() = s_homeButtonResourceState;
    }
    HomeMenu_OverrideBlockedState(Sing_805DC820(), 0);
    CCheckDriveStatus_SetErrorDialogsEnabled(1);
    // Phase 3: release heap-backed text, then delete this wrapper only for flag > 0.
    if (&p->messageStringFlags != 0) {
        unsigned int v34 = p->messageStringFlags;
        if (v34 >> 31) {
            appFreeUni__FPv(p->messageStringData);
        }
    }
    if (flag > 0) {
        __dl__FPv(p);
    }
    return p;
}

} // extern "C"
