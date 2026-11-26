// Controller (pad) memory load menu: CPadMemoryMenuLoad reads a save from a controller's memory
// card, and CFileIoPad is the controller-memory file object that opens, reads and writes it.

#include <decomp/static_init.h>
#include "classes/CFileIo.h"
#define AsCFileIo(p) ((CFileIo*)(p))
#include "classes/CPadMemoryMenuLoad.h"
#include "classes/CPadMemoryMenu.h"
#include "classes/CFileIoPad.h"
#include "classes/CFileIoAccess.h"
#include "classes/CFileIoAccessReadPad.h"
#include "classes/CFileIoMenu.h"
#include "classes/CSaveFormApplication.h"
#include "classes/CFileIoAccessWritePad.h"

#define AsCFileIoPad(p) ((CFileIoPad*)(p))

#define AsCPadMemoryMenu(p) ((CPadMemoryMenu*)(p))

#define AsCPadMemoryMenuLoad(p) ((CPadMemoryMenuLoad*)(p))

extern "C" {

// Controller-memory I/O contracts:
// The controller argument is a channel value carried in an ABI pointer word.
// Menu owns saveForm/fileIoAccess and the loaded saveData prefix. Copy routines
// borrow a writable destination of at least SaveRecordPrefixSize bytes.
// vfunc_9 returns 1 after a successful processing step (including a wait), 0 on
// a failed prerequisite/allocation/dialog step; it is not the completion test.
// Completion is loadState == PadLoad_Complete; loadResult then says failed,
// no-data or loaded. Retry limit 3 permits one read plus three retries per cycle.
// Recovery can reset that counter for up to four such cycles.
// Controller-memory load flow; loadResult is separate from this progress state.
enum PadLoadState {
    PadLoad_CheckController = 0,
    PadLoad_CheckPrerequisites = 1,
    PadLoad_BeginRead = 2,
    PadLoad_WaitForRead = 3,
    PadLoad_WaitForDisconnectMessage = 4,
    PadLoad_WaitForInvalidDataMessage = 5,
    PadLoad_ShowNoData = 6,
    PadLoad_WaitForNoData = 7,
    PadLoad_WaitForFailureMessage = 8,
    PadLoad_CopyLoadedRecord = 9,
    PadLoad_Close = 10,
    PadLoad_Complete = 11,
};
enum PadLoadResult {
    PadLoadResult_Failed = 0,
    PadLoadResult_NoData = 1,
    PadLoadResult_Loaded = 2
};
enum {
    kPadSaveFormMode = 2,
    kPadSaveFormOption = 1,
    PadLoadRetryLimit = 3,
    SaveRecordPrefixSize = 0x47C,
    PadIoErrorC739 = 0xC739,
    PadIoErrorC73D = 0xC73D
};

// Minimal virtual interfaces preserve the original dispatch slots.
struct PadLoadProcessVirtuals {
    virtual void m0();
    virtual void m1(int);
    virtual void m2();
    virtual void m3(int);
    virtual void m4(int);
    virtual void m5();
    virtual void m6();
    virtual void m7();
    virtual void m8();
    virtual int process();
};

struct PadOwnedObjectVirtuals {
    virtual void m0(int);
    virtual void m1();
    virtual void m2();
    virtual void m3(int);
};

struct PadLoadCloseVirtuals {
    virtual void m0();
    virtual void m1();
    virtual void m2();
    virtual void m3();
    virtual void m4();
    virtual void m5();
    virtual void m6();
    virtual void m7();
    virtual void m8();
    virtual void m9();
    virtual int close();
};

struct PadLoadFactoryVirtuals {
    virtual void m0(int);
    virtual void m1();
    virtual void m2();
    virtual void m3();
    virtual void m4();
    virtual void m5();
    virtual void m6();
    virtual void m7();
    virtual int m8();
};

struct PadSaveRecordVirtuals {
    virtual void m0();
    virtual void m1();
    virtual void m2();
    virtual void m3();
    virtual void m4(int);
    virtual void m5();
    virtual void m6();
    virtual void m7();
    virtual void m8();
    virtual int m9();
    virtual void m10();
    virtual void m11();
    virtual void m12();
    virtual void m13();
    virtual void m14();
    virtual void m15();
    virtual void m16();
    virtual void m17();
    virtual void* getSaveRecord();
};

struct ObjDelete8 {
    virtual void m0();
    virtual void m1(int);
};

struct ObjDelete14 {
    virtual void m0();
    virtual void m1();
    virtual void m2();
    virtual void m3();
    virtual void m4(int);
};

struct ObjEE4 {
    virtual void m0();
    virtual void m1();
    virtual void m2();
    virtual void m3();
    virtual void m4();
    virtual void m5();
    virtual void m6();
    virtual void m7();
    virtual void m8();
    virtual void m9();
    virtual void m10();
    virtual void m11();
    virtual void m12();
    virtual void m13();
    virtual void m14(int);
    virtual int m15(int, int);
};

struct Obj5C0 {
    virtual void m0();
    virtual void m1();
    virtual void m2(int);
    virtual void m3(int);
    virtual void m4();
    virtual void m5(int);
    virtual void m6();
    virtual void m7();
    virtual void m8();
    virtual void m9();
    virtual void m10();
    virtual void m11();
    virtual void m12();
    virtual void m13();
    virtual void m14();
    virtual void m15();
    virtual void m16();
    virtual void m17();
    virtual void* m18();
    virtual void m19();
    virtual void* m20();
};

struct PadFileOpenVirtuals {
    virtual void m0();
    virtual void m1();
    virtual void m2();
    virtual void m3();
    virtual void m4();
    virtual void m5();
    virtual void m6();
    virtual void m7();
    virtual void m8();
    virtual void m9();
    virtual void m10();
    virtual void m11();
    virtual void m12(int);
    virtual int m13(int, int);
};
struct ObjDel8Call {
    virtual void m0(int);
};
int FileIo_ReadAlignedBuffer(void*, void (*)(void));
extern float kFloat_6_2831855;
extern "C" void* gtl_InputEx_GetScreenPointer(int);
extern "C" void Memory_Copy(void*, void*, unsigned int);
extern "C" unsigned int File_GetLastError();
extern "C" void __dl__FPv(void*);
extern "C" void __dla__FPv(void*);
extern "C" void __dt__14CPadMemoryMenuFv(void*, int);
extern "C" char __vt__18CPadMemoryMenuLoad[];
// Message entries retain the byte extents of the original 16-bit text pool.
struct PadLoadDialogPool {
    unsigned short reading[24];
    unsigned short disconnected[27];
    unsigned short invalidData[24];
    unsigned short loadSuccess[29];
    unsigned short loadFailed[26];
    unsigned short noData[24];
};
extern "C" PadLoadDialogPool fileIoLoadDialogMessages;
typedef char PadLoadDialogPool_size[(sizeof(PadLoadDialogPool) == 0x134) ? 1 : -1];
extern float s_padLoadFullTurn;
extern float s_padFileIoFullTurn;
extern "C" void* SaveRecordPrefix_CopyCtor(void*, void*);
extern "C" int SaveForm_VerifyAllRecordCopies(void*, void*, void*, int);
extern "C" void* __nw__FUl(unsigned long);
extern "C" void SaveIO_BeginSystemGuard(void*);
extern "C" void* SaveForm_GetStorageSize(void*);
extern "C" void* __nwa__FUl(unsigned long);
extern "C" int SaveForm_ReplicateWithDigest(void*, void*, void*, int);
extern "C" int g_padSaveGameTitle;
extern "C" void* g_fileIoPad;
extern "C" char __vt__10CFileIoPad[];
extern "C" void SaveRecordPrefix_ClearCtor(void*);
// Forward load-menu initialization to the common controller-memory menu.
int CPadMemoryMenuLoad::vfunc_8(void* arg) {
    return CPadMemoryMenu::SetCurrentSharedContext(arg);
}

// Draw the common file-I/O menu.
// CFileIoMenu::Draw returns nothing; the wrapper passes through r3 as the original did.
extern "C" int Draw__11CFileIoMenuFv(void* self);
int CPadMemoryMenuLoad::Draw(void* arg) {
    return Draw__11CFileIoMenuFv(arg);
}

// Report whether the load menu has finished closing.
int CPadMemoryMenuLoad::IsComplete() {
    return (*(unsigned int*)&loadState) == PadLoad_Complete;
}

// Initialize this load unit's full-turn constant.
void PadLoad_InitializeFullTurnConstant() {
    s_padLoadFullTurn = kFloat_6_2831855;
}

// Initialize this controller-I/O unit's full-turn constant.
void PadFileIo_InitializeFullTurnConstant() {
    s_padFileIoFullTurn = kFloat_6_2831855;
}

// Default controller-file hook: return zero.
int CFileIoPad::vfunc_3() {
    return 0;
}

// Default controller-file hook: return zero.
int CFileIoPad::vfunc_4() {
    return 0;
}

// Default controller-file hook: return zero.
int CFileIoPad::vfunc_5() {
    return 0;
}

// Default controller-file hook: return zero.
int CFileIoPad::vfunc_6() {
    return 0;
}

// Default controller-file hook: return zero.
int CFileIoPad::vfunc_10() {
    return 0;
}

// Default controller-file hook: return zero.
int CFileIoPad::vfunc_11() {
    return 0;
}

// Default controller-file hook: return zero.
int CFileIoPad::vfunc_8() {
    return 0;
}

// Start an aligned controller-memory read using the completion callback.
int CFileIoPad::ReadAlignedBuffer() {
    return FileIo_ReadAlignedBuffer(this, (void (*)(void))CFileIoPad::OnTransferComplete);
}

// Open controller-memory storage using the open callback; the third argument is unused.
int CFileIoPad::Open(void* openArgument, void* unused) {
    return CFileIo::Open((int)CFileIoPad::OnOpenComplete, openArgument);
}

// Write the padded controller-memory buffer; the third argument is unused.
int CFileIoPad::StartPaddedWrite(void* data, void* unused) {
    return ((CFileIo*)this)->WritePaddedBuffer(data, (int)CFileIoPad::OnTransferComplete);
}

// Report a nonzero controller-memory I/O status.
int CFileIoPad::HasFileError() {
    int statusCode = this->statusCode;
    return statusCode != 0;
}

// Test the controller-memory status value C739 without translating its meaning.
int CFileIoPad::vfunc_9() {
    int statusCode = this->statusCode;
    return statusCode == PadIoErrorC739;
}

#pragma dont_inline on
// Check whether this controller currently has a screen pointer.
int CPadMemoryMenu::HasScreenPointer() {
    void* screenPointer = gtl_InputEx_GetScreenPointer((*(int*)&controllerIndex));
    return screenPointer != 0;
}
#pragma dont_inline off

// Copy the loaded 0x47C-byte save prefix when one is owned.
int CPadMemoryMenuLoad::CopySaveData(void* destination) {
    int saveDataAddress = (*(int*)&saveData);
    if (saveDataAddress == 0)
        return 0;
    Memory_Copy(destination, (void*)saveDataAddress, SaveRecordPrefixSize);
    return 1;
}

#pragma dont_inline on
// Query the controller file-access object's slot-9 error/status predicate.
int CPadMemoryMenu::IsDeviceDisconnected() {
    int hasStatus = 0;
    void* fileAccess = fileIoAccess;
    if (fileAccess) {
        int status = ((CFileIoAccess*)fileAccess)->QueryResourceStatusSlot9();
        if (status)
            hasStatus = 1;
    }
    return hasStatus;
}
#pragma dont_inline off

#pragma dont_inline on
// Copy the first valid save prefix returned by the common menu.
int CPadMemoryMenu::CopyValidSaveData(void* destination) {
    void* destinationAlias = destination;
    void* validRecord = ((CFileIoMenu*)this)->FindValidSaveRecord();
    if (validRecord == 0)
        return 0;
    Memory_Copy(destination, validRecord, SaveRecordPrefixSize);
    // The save file holds the Wii (big-endian) byte order.
    return 1;
}
#pragma dont_inline off

// Process the menu and report whether its load flow is complete.
int CPadMemoryMenuLoad::ProcessAndReportComplete() {
    void* menuAlias = this;
    int processed = ((PadLoadProcessVirtuals*)this)->process();
    if (processed != 0) {
        int state = loadState;
        return state == PadLoad_Complete;
    }
    return 1;
}

#pragma dont_inline on
// Check the last file error only when the access object reports a failure.
int CPadMemoryMenu::HasFileErrorC73D() {
    int matchesError = 0;
    int accessFailed = 0;
    void* fileAccess = fileIoAccess;
    if (fileAccess) {
        int statusOrError = ((CFileIoAccess*)fileAccess)->GetFlag();
        if (statusOrError)
            accessFailed = 1;
    }
    if (accessFailed) {
        unsigned int statusOrError = File_GetLastError();
        if (statusOrError == PadIoErrorC73D)
            matchesError = 1;
    }
    return matchesError;
}
#pragma dont_inline off

// Release the loaded save prefix, destroy the base menu, and optionally free this object.
CPadMemoryMenuLoad::~CPadMemoryMenuLoad() {
    void* saveData = this->saveData;
    vtable = (void*)__vt__18CPadMemoryMenuLoad;
    if (saveData) {
        __dl__FPv(saveData);
        this->saveData = 0;
    }
    __dt__14CPadMemoryMenuFv(this, 0);
}

// Extract and validate a virtual save-prefix snapshot from the current I/O buffer.
int CPadMemoryMenu::VerifyWrittenRecord() {
    char savePrefix[SaveRecordPrefixSize];
    void* sourceRecord = ((PadSaveRecordVirtuals*)this)->getSaveRecord();
    SaveRecordPrefix_CopyCtor(savePrefix, sourceRecord);
    if (0 == (*(int*)&fileIoAccess) || saveForm == 0)
        return 0;
    int transferredBytes = ((CFileIoAccess*)fileIoAccess)->GetTransferredBytes();
    void* readBuffer = (void*)((CFileIoAccess*)fileIoAccess)->GetBuffer();
    return SaveForm_VerifyAllRecordCopies(saveForm, readBuffer, savePrefix, transferredBytes);
}

// Initialize the derived load state after common menu construction.
inline CPadMemoryMenuLoad::CPadMemoryMenuLoad(void* value) {
    ((CPadMemoryMenu*)this)->Init(value);
    vtable = (void*)__vt__18CPadMemoryMenuLoad;
    loadState = PadLoad_CheckController;
    (*(int*)&saveData) = 0;
    loadResult = PadLoadResult_Failed;
    (*(int*)&loadRetryCount) = 0;
    (*(int*)&recoveryRetryCount) = 0;
}

// Construct and initialize a controller-memory load menu, releasing it on failure.
void* CPadMemoryMenuLoad::Create(void* controller) {
    void* controllerAlias = controller;
    CPadMemoryMenuLoad* menu = new CPadMemoryMenuLoad(controllerAlias);
    if (menu == 0)
        return 0;
    int initialized = ((PadLoadFactoryVirtuals*)menu)->m8();
    if (initialized == 0) {
        if (menu != 0) {
            ((PadLoadFactoryVirtuals*)menu)->m0(1);
        }
        return 0;
    }
    return menu;
}

// Replace the save form and access object, then begin a guarded controller-memory read.
int CPadMemoryMenu::CreateReadFileIo() {
    void* menu = this;
    void* previousSaveForm = AsCPadMemoryMenu(menu)->saveForm;
    if (previousSaveForm != 0) {
        if (previousSaveForm != 0) {
            ((PadOwnedObjectVirtuals*)previousSaveForm)->m0(1);
        }
        AsCPadMemoryMenu(menu)->saveForm = 0;
    }
    void* newSaveForm = CSaveFormApplication::Create(kPadSaveFormMode, kPadSaveFormOption);
    AsCPadMemoryMenu(menu)->saveForm = newSaveForm;
    if (newSaveForm == 0)
        return 0;
    SaveIO_BeginSystemGuard(menu);
    void* previousAccess = AsCPadMemoryMenu(menu)->fileIoAccess;
    if (previousAccess != 0) {
        if (previousAccess != 0) {
            ((PadOwnedObjectVirtuals*)previousAccess)->m3(1);
        }
        AsCPadMemoryMenu(menu)->fileIoAccess = 0;
    }
    void* storageSize = SaveForm_GetStorageSize(AsCPadMemoryMenu(menu)->saveForm);
    int newAccess = (int)CFileIoAccessReadPad::Create(AsCPadMemoryMenu(menu)->controllerIndex,
                                                      (void*)g_padSaveGameTitle, storageSize);
    AsCPadMemoryMenu(menu)->fileIoAccess = (void*)newAccess;
    if (newAccess == 0)
        return 0;
    return 1;
}

// Cached success adds a reference; the caller must release that reference
// through the I/O lifecycle. Failure returns null; open arguments are opaque.
// Acquire the cached controller file object or construct and open one.
void* CFileIoPad::CreateInstance(void* controller, int openArgument1, int openArgument2) {
    void* controllerAlias = controller;
    int openValue1 = openArgument1;
    int openValue2 = openArgument2;
    if (g_fileIoPad != 0) {
        if (((CFileIo*)g_fileIoPad)->AddRef() != 0)
            return g_fileIoPad;
        return 0;
    }
    // Construction view of CFileIoPad; canonical members: classes/CFileIoPad.h.
    // Factory allocates 0x60 bytes; shared header ends at +0x5c.
    struct CFileIoPadConstructorView {
        char pad[0x60];
        // Initialize common I/O state and bind the controller channel.
        CFileIoPadConstructorView(void* value) {
            AsCFileIo(this)->InitMembers();
            AsCFileIo(this)->vtable = (void*)__vt__10CFileIoPad;
            (*(void**)&AsCFileIoPad(this)->padIndex) = value;
        }
    };
    CFileIoPadConstructorView* fileIo = new CFileIoPadConstructorView(controllerAlias);
    g_fileIoPad = fileIo;
    if (fileIo == 0)
        return 0;
    int opened = ((PadFileOpenVirtuals*)fileIo)->m13(openValue1, openValue2);
    if (opened == 0) {
        if (fileIo != 0) {
            ((PadFileOpenVirtuals*)fileIo)->m12(1);
        }
        return 0;
    }
    return fileIo;
}

// Build a temporary replicated save image and create a write-access object from it.
// The access constructor must consume/copy the image: this function frees it
// before returning even on success. Return 1 iff the access object was created.
int CPadMemoryMenu::CreateWriteFileIo() {
    // Replace the form and old access object before preparing the new image.
    void* previousSaveForm = saveForm;
    if (previousSaveForm != 0) {
        if (previousSaveForm != 0)
            ((ObjDel8Call*)previousSaveForm)->m0(1);
        saveForm = 0;
    }
    void* newSaveForm = CSaveFormApplication::Create(kPadSaveFormMode, kPadSaveFormOption);
    saveForm = newSaveForm;
    if (newSaveForm == 0)
        return 0;
    SaveIO_BeginSystemGuard(this);
    void* previousAccess = fileIoAccess;
    if (previousAccess != 0) {
        if (previousAccess != 0)
            ((Obj5C0*)previousAccess)->m3(1);
        fileIoAccess = 0;
    }
    // Allocate the full storage image; the stack snapshot is only its logical prefix.
    void* storageByteCount = SaveForm_GetStorageSize(saveForm);
    if (storageByteCount == 0)
        return 0;
    void* replicatedImage = __nwa__FUl((unsigned long)storageByteCount);
    char savePrefix[SaveRecordPrefixSize];
    int createdAccess = 0;
    void* sourceRecord = ((Obj5C0*)this)->m18();
    SaveRecordPrefix_CopyCtor(savePrefix, sourceRecord);
    // The Wii Remote profile stores the Wii (big-endian) byte order; the digest covers these bytes.
    int replicated = SaveForm_ReplicateWithDigest(saveForm, replicatedImage, savePrefix, SaveRecordPrefixSize);
    if (replicated != 0) {
        void* writeByteCount = SaveForm_GetStorageSize(saveForm);
        void* writeAccess =
            CFileIoAccessWritePad::Create(controllerIndex, (void*)g_padSaveGameTitle, writeByteCount, replicatedImage);
        fileIoAccess = writeAccess;
        if (writeAccess != 0)
            createdAccess = 1;
    }
    // The temporary image never escapes this call, including the failure path.
    if (replicatedImage != 0) {
        __dla__FPv(replicatedImage);
    }
    return createdAccess;
}

// Advance read retries, error/success dialogs, save-prefix copying and menu closure.
int CPadMemoryMenuLoad::AdvanceLoadState(void* arg) {
    void* menu = arg;
    int baseProcessed = ((CFileIoMenu*)menu)->Update();
    if (baseProcessed == 0)
        return 0;
    unsigned int state = (*(unsigned int*)&AsCPadMemoryMenuLoad(menu)->loadState);
    switch (state) {
    case PadLoad_CheckController: {
        int status = ((CPadMemoryMenu*)menu)->HasScreenPointer();
        if (status != 0) {
            AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_CheckPrerequisites;
        } else {
            AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_Close;
        }
        break;
    }
    case PadLoad_CheckPrerequisites: {
        // 0x802744E4 unit: check both prerequisites before opening the dialog.
        int status = ((CPadMemoryMenu*)menu)->HasStoredTitle();
        if (status != 0 && (status = ((CPadMemoryMenu*)menu)->StoredTitleMatchesGame()) != 0) {
            status = ((CFileIoMenu*)menu)->CreateButtonlessCommonDialog((void*)fileIoLoadDialogMessages.reading);
            if (status == 0)
                return 0;
            AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_BeginRead;
        } else {
            AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_ShowNoData;
        }
        break;
    }
    case PadLoad_BeginRead: {
        int status = ((CPadMemoryMenu*)menu)->CreateReadFileIo();
        if (status == 0)
            return 0;
        AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_WaitForRead;
        break;
    }
    // Once I/O completes, distinguish disconnection, absent data, transfer
    // failures and structurally invalid records before presenting a result.
    case PadLoad_WaitForRead: {
        int status = ((CFileIoMenu*)menu)->QueryFileAccessStatus();
        if (status == 0)
            break;
        status = ((CPadMemoryMenu*)menu)->IsDeviceDisconnected();
        if (status != 0) {
            status = ((CFileIoMenu*)menu)->ShowAcknowledgeDialog((void*)fileIoLoadDialogMessages.disconnected);
            if (status == 0)
                return 0;
            AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_WaitForDisconnectMessage;
            break;
        }
        status = ((CPadMemoryMenu*)menu)->HasFileErrorC73D();
        if (status != 0) {
            AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_ShowNoData;
            break;
        }
        status = ((CFileIoMenu*)menu)->QueryAccessFailure();
        if (status != 0) {
            unsigned int readRetries = AsCPadMemoryMenuLoad(menu)->loadRetryCount;
            if (readRetries < PadLoadRetryLimit) {
                AsCPadMemoryMenuLoad(menu)->loadRetryCount = readRetries + 1;
                AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_BeginRead;
            } else {
                status = ((CFileIoMenu*)menu)->ShowAcknowledgeDialog((void*)fileIoLoadDialogMessages.invalidData);
                if (status == 0)
                    return 0;
                AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_WaitForInvalidDataMessage;
            }
            break;
        }
        status = ((CFileIoMenu*)menu)->HasValidSaveRecord();
        if (status != 0) {
            status = ((CFileIoMenu*)menu)->ShowAcknowledgeDialog((void*)fileIoLoadDialogMessages.loadSuccess);
            if (status == 0)
                return 0;
            AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_CopyLoadedRecord;
            break;
        }
        unsigned int recoveryRetries = AsCPadMemoryMenuLoad(menu)->recoveryRetryCount;
        if (recoveryRetries < PadLoadRetryLimit) {
            unsigned int nextRecoveryRetry = recoveryRetries + 1;
            AsCPadMemoryMenuLoad(menu)->loadRetryCount = 0;
            AsCPadMemoryMenuLoad(menu)->recoveryRetryCount = nextRecoveryRetry;
            AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_BeginRead;
        } else {
            AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_ShowNoData;
        }
        break;
    }
    // Dialog states wait for acknowledgement; no new read starts here.
    case PadLoad_WaitForDisconnectMessage: {
        int status = ((CFileIoMenu*)menu)->IsCommonDialogComplete();
        if (status == 0)
            break;
        status = ((CFileIoMenu*)menu)->ShowAcknowledgeDialog((void*)fileIoLoadDialogMessages.loadFailed);
        if (status == 0)
            return 0;
        AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_WaitForFailureMessage;
        break;
    }
    case PadLoad_WaitForInvalidDataMessage: {
        int status = ((CFileIoMenu*)menu)->IsCommonDialogComplete();
        if (status == 0)
            break;
        status = ((CFileIoMenu*)menu)->ShowAcknowledgeDialog((void*)fileIoLoadDialogMessages.loadFailed);
        if (status == 0)
            return 0;
        AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_WaitForFailureMessage;
        break;
    }
    case PadLoad_ShowNoData: {
        int status = ((CFileIoMenu*)menu)->ShowAcknowledgeDialog((void*)fileIoLoadDialogMessages.noData);
        if (status == 0)
            return 0;
        AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_WaitForNoData;
        break;
    }
    case PadLoad_WaitForNoData: {
        int status = ((CFileIoMenu*)menu)->IsCommonDialogComplete();
        if (status == 0)
            break;
        AsCPadMemoryMenuLoad(menu)->loadResult = PadLoadResult_NoData;
        AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_Close;
        break;
    }
    case PadLoad_WaitForFailureMessage: {
        int status = ((CFileIoMenu*)menu)->IsCommonDialogComplete();
        if (status == 0)
            break;
        AsCPadMemoryMenuLoad(menu)->loadResult = PadLoadResult_Failed;
        AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_Close;
        break;
    }
    // Copy out of the access buffer only after the success dialog is acknowledged.
    case PadLoad_CopyLoadedRecord: {
        int status = ((CFileIoMenu*)menu)->IsCommonDialogComplete();
        if (status == 0)
            break;
        void* p = AsCPadMemoryMenuLoad(menu)->saveData;
        if (p != 0) {
            __dl__FPv(p);
            AsCPadMemoryMenuLoad(menu)->saveData = 0;
        }
        void* allocation = __nw__FUl(SaveRecordPrefixSize);
        void* newSaveData = allocation;
        if (allocation != 0) {
            SaveRecordPrefix_ClearCtor(allocation);
        }
        AsCPadMemoryMenuLoad(menu)->saveData = newSaveData;
        status = ((CPadMemoryMenu*)menu)->CopyValidSaveData(newSaveData);
        if (status == 0)
            return 0;
        AsCPadMemoryMenuLoad(menu)->loadResult = PadLoadResult_Loaded;
        AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_Close;
        break;
    }
    // Closing is asynchronous; only a successful close marks the flow complete.
    case PadLoad_Close: {
        int status = ((PadLoadCloseVirtuals*)menu)->close();
        if (status == 0)
            return 0;
        AsCPadMemoryMenuLoad(menu)->loadState = PadLoad_Complete;
        break;
    }
    }
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462388", _ctors_u802744E4_0, PadLoad_InitializeFullTurnConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462388", _ctors_u802744E4_1, PadFileIo_InitializeFullTurnConstant);
#pragma pop
