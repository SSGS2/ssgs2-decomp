// Save process actor and task: CSaveProcessMain runs and draws the save menu each frame, and
// CSaveProcessTask creates the save-process actor.
// The file also holds the drive-error display loop and the task-name setters.

#include <decomp/static_init.h>
#include <decomp/local_class.h>

#include "game/string.h"

// Stack-local narrow-string view; the canonical string ABI remains external.
class LocalNString_u80280D18 {
public:
    unsigned int flags;
    unsigned int length;
    char* data;

    // Construct an empty inline string without acquiring heap storage.
    LocalNString_u80280D18() : flags(0), length(0), data(0) {}
#if defined(GAME_NSTRING_TEXT_CONSTRUCTOR)
    // Copy a terminated string through the shared ABI constructor.
    LocalNString_u80280D18(const char* text) {
        MWString_ConstructCString(this, text);
    }
#endif
#if defined(GAME_NSTRING_RANGE_CONSTRUCTOR)
    // Copy a terminated string using the shared reserve/replace operations.
    LocalNString_u80280D18(const char* text) {
        LocalNString_u80280D18* object = this;
        unsigned int size;
        const char* source = text;
        object->flags = 0;
        object->length = 0;
        object->data = 0;
        size = nstring_detail::strlen(source);
        nstring_detail::String_ReserveCapacity(object, size);
        int allocator;
        char copy = *(char*)&allocator;
        nstring_detail::String_ReplaceRange(object, 0, 0, source, source + size, &copy);
    }
#endif
    // Release storage only for a heap-backed tagged string.
    ~LocalNString_u80280D18() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
    // Return the byte length from the inline tag or the heap length word.
    unsigned int size() const {
        return (flags >> 31) == 0 ? (*(const unsigned char*)this & 0x7f) : length;
    }
    // Borrow the terminated character range until this string changes or dies.
    const char* c_str() const {
        return (flags >> 31) == 0 ? (const char*)this + 1 : data;
    }
};

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CSaveProcessMain.h"
#include "classes/CCheckDriveStatus.h"
#include "classes/CFlashMemoryMenuSave.h"
#include "classes/CSaveProcessTask.h"
#include "classes/CTask.h"

#define AsCSaveProcessMain(p) ((CSaveProcessMain*)(p))

enum SaveProcessConstants {
    kTaskNameCapacity = 0x20,
    kTaskNameMaxLength = kTaskNameCapacity - 1,
    kTaskFirstSecondaryVtableOffset = 0x10,
    kTaskSecondSecondaryVtableOffset = 0x1C,
    kDriveErrorFrameMilliseconds = 33,
    kBusClockRegister = 0x800000F8
};

// Eight words received by HandleMsg; five are forwarded when the save menu closes.
// The first word and retainedArg0/1 are received but not forwarded on this completion path.
struct SaveProcessRequest {
    int retainedRequestWord0;
    int transitionArg0, transitionArg1, transitionArg2, transitionArg3;
    int retainedArg0, retainedArg1;
    int transitionArg4;
};
// The shared header exposes these eight words individually; this view groups the message payload.
// Borrow the actor's in-place request; the returned view owns no separate storage.
inline SaveProcessRequest& SaveRequest(void* self) {
    return *(SaveProcessRequest*)&AsCSaveProcessMain(self)->retainedRequestWord0;
}
typedef char SaveProcessRequestSize[(sizeof(SaveProcessRequest) == 0x20) ? 1 : -1];
extern unsigned char s_saveProcessTaskRtti[1], s_saveProcessMainRtti[1];

extern char s_saveProcessTransitionTargetName[0x20];
extern char s_saveProcessFollowUpTaskName[0x20];
extern char s_saveProcessEmptyArgumentAndStringErrors[];
extern char s_saveProcessActorClassName[];
extern char s_saveProcessActorInstanceName[];
extern char __vt__16CSaveProcessMain[];
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_SaveProcess[];
extern char s_saveProcessEmptyTransitionArgument[];
// must really be 0x48 bytes (the Wii keeps the split labels).
char driveErrorDialogDtorRegistrations[12];
char lbl_8050A45C[12]; // Reserved destructor-registration slot; no use in this unit.
char lbl_8050A468[12]; // Reserved destructor-registration slot; no use in this unit.
char lbl_8050A474[12]; // System-reset singleton destructor registration.
char lbl_8050A480[12]; // Reserved destructor-registration slot; no use in this unit.
char lbl_8050A48C[12]; // Fade-overlay singleton destructor registration.
extern char s_checkDriveStatus[];
extern char s_fadeOverlay[];
extern char s_systemResetStorage[];

extern "C" {
void __ct__18NativeBase80201D18Fv(void*);
struct SoundManagerState;
struct CTaskManager;
extern SoundManagerState* g_soundManager;
extern CTaskManager* g_taskManager;
extern signed char s_inputManagerInitialized, s_checkDriveStatusInitialized, s_systemResetStateInitialized,
    s_fadeOverlayInitialized;
void InputManager_Init(void*);
void InputManager_Dtor(void*);
void InputManager_SetSelectedPlayer(void*, int);
void SoundManager_RequestBgmStop(void*, int);
int MPBase1_Process(void*, float);
void TaskCommandQueue_AppendNamedRequest(void*, int, void*, int, int, int, int, int);
void CCheckDriveStatus_dtor();
void SystemResetState_Ctor(void*);
void SystemResetState_Dtor();
int SystemReset_ProcessPendingReset(void*, int);
void Engine_ClearFramebuffer(int, int, int, int, int, float);
void FadeOverlay_Init(void*);
void ResourceState_Dtor();
void FadeState_Draw(void*);
void Engine_PresentFrame(int, int, int, int);
void OSSleepTicks(unsigned long long);
extern float kFloat_1_DiskError;

void* __dt__16CSaveProcessMainFv(void*, int);
void* __dt__16CSaveProcessTaskFv(void*, int);
void* __ct__16CSaveProcessTaskFv(void*);
void MPBase1_DrawIfInitialized(void*);
void __dt__5CTaskFv(void*, int);
void __dt__18NativeBase80201D18Fv(void*, int);
void __dl__FPv(void*);

void* memset(void*, int, unsigned int);
int __cntlzw(unsigned int);

void* String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, const char*, const char*, char*);
void* TaskActorTable_CreateNamedActor(void*, const char*, const char*, int, void*);
void appFreeUni__FPv(void*);

extern char __vt__16CSaveProcessTask[];
extern int g_defaultActorSkipMask;
extern unsigned int* g_actorSkipMask;

extern float kFloat_6_2831855;
extern float s_saveProcessMainTau;
extern float s_saveProcessTaskTau;
extern float s_driveErrorLoopTau;

// Return the save-process main actor's RTTI descriptor.
void* CSaveProcessMain::GetRTTI() {
    return s_saveProcessMainRtti;
}
// Adjust the secondary-interface receiver before destroying the save-process actor.
void CSaveProcessMain::DtorThunk18(int deleting) {
    __dt__16CSaveProcessMainFv((char*)this - 0x18, deleting);
}
// Return the save-process task's RTTI descriptor.
void* CSaveProcessTask::GetRTTI() {
    return s_saveProcessTaskRtti;
}

// Initialize this module's stored two-pi constant from the shared scalar pool.
void SaveProcessMain_InitTauConstant() {
    s_saveProcessMainTau = kFloat_6_2831855;
}
// Initialize the save request/menu state and select input player zero.
CSaveProcessMain::CSaveProcessMain() {
    // Phase 1: construct interfaces and clear the eight-word request/menu handle.
    __ct__18NativeBase80201D18Fv(this);
    int inputManagerAddress = ((int&)GetInputManagerSlot());
    *(void**)this = __vt__16CSaveProcessMain;
    secondaryVtable = __vt__16CSaveProcessMain + 0x3c;
    SaveRequest(this).retainedRequestWord0 = 0;
    SaveRequest(this).transitionArg0 = 0;
    SaveRequest(this).transitionArg1 = 0;
    SaveRequest(this).transitionArg2 = 0;
    SaveRequest(this).transitionArg3 = 0;
    SaveRequest(this).retainedArg0 = 0;
    SaveRequest(this).retainedArg1 = 0;
    SaveRequest(this).transitionArg4 = 0;
    saveMenu = 0;
    // Phase 2: acquire the shared input manager and select player slot zero.
    if (!inputManagerAddress) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     s_inputManagerDtorRegistration_SaveProcess);
            s_inputManagerInitialized = 1;
        }
        ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
    }
    InputManager_SetSelectedPlayer((void*)((int&)GetInputManagerSlot()), 0);
}

// Create/process the NAND save menu, then queue the requested task transition after it completes.
// The actor owns saveMenu and destroys it with deleting flag 1 on completion.
// dt is forwarded unchanged to MPBase1_Process; its time unit is defined there.
void CSaveProcessMain::OnProcess(float dt) {
    CSaveProcessMain* saveProcess = this;
    // Phase 1: lazily create the menu and request BGM stop once per menu instance.
    if (!saveProcess->saveMenu) {
        saveProcess->saveMenu = (int)CFlashMemoryMenuSave::Create();
        SoundManager_RequestBgmStop((void*)g_soundManager, 1);
    }
    if (MPBase1_Process((void*)saveProcess->saveMenu, dt)) {
        // Phase 2: the menu has finished; release it before scheduling follow-up work.
        if (saveProcess->saveMenu) {
            if (saveProcess->saveMenu) {
                struct V {
                    virtual void m(int) DECOMP_LOCAL_VIRTUAL;
                };
                ((V*)saveProcess->saveMenu)->m(1);
            }
            saveProcess->saveMenu = 0;
        }
        // Phase 3: queue the transition and the five forwarded request words.
        ((CTask*)g_taskManager)
            ->RequestTransitionWithFade(s_saveProcessTransitionTargetName, s_saveProcessEmptyTransitionArgument, 0);
        TaskCommandQueue_AppendNamedRequest(
            (void*)g_taskManager, 0, s_saveProcessFollowUpTaskName, SaveRequest(saveProcess).transitionArg0,
            SaveRequest(saveProcess).transitionArg1, SaveRequest(saveProcess).transitionArg2,
            SaveRequest(saveProcess).transitionArg3, SaveRequest(saveProcess).transitionArg4);
    }
}
// Initialize this module's stored two-pi constant from the shared scalar pool.
void SaveProcessTask_InitTauConstant() {
    s_saveProcessTaskTau = kFloat_6_2831855;
}
// Initialize this module's stored two-pi constant from the shared scalar pool.
void DriveErrorLoop_InitTauConstant() {
    s_driveErrorLoopTau = kFloat_6_2831855;
}
// Draw the optional save menu through its MPBase interface.
void CSaveProcessMain::OnDisplay() {
    void* menu = (*(void**)&saveMenu);
    if (menu)
        MPBase1_DrawIfInitialized(menu);
}
// Copy all eight incoming request words for the transition after saving.
// src points to at least 0x20 readable bytes; the copy retains no src pointer.
void CSaveProcessMain::HandleMsg(void* dst, const void* src) {
    SaveRequest(dst) = *(const SaveProcessRequest*)src;
}
// Adjust the secondary-interface receiver before destroying the save-process task.
void CSaveProcessTask::DtorThunk8(int deleting) {
    __dt__16CSaveProcessTaskFv((char*)this - 8, deleting);
}

#pragma opt_strength_reduction off
// Run the blocking drive-error UI loop with reset handling, fading, frame presentation, and 33-ms waits.
// Never returns. The sleep uses the low-memory bus-clock value divided by four
// (timebase ticks/second), then integer ticks/ms; it does not use the frame delta.
void DriveError_RunDisplayLoop() {
    // Phase 1: borrow the shared drive-status object; this loop never destroys it.
    void* driveStatus;
    if (!((int&)Sing_805DCB20())) {
        if (!s_checkDriveStatusInitialized) {
            ((CCheckDriveStatus*)s_checkDriveStatus)->__ct();
            __register_global_object(s_checkDriveStatus, (void*)CCheckDriveStatus_dtor,
                                     driveErrorDialogDtorRegistrations);
            s_checkDriveStatusInitialized = 1;
        }
        ((int&)Sing_805DCB20()) = (int)s_checkDriveStatus;
    }
    driveStatus = (void*)((int&)Sing_805DCB20());
    for (;;) {
        // Phase 2: update the dialog; only draw/present when it requests a frame.
        if (CCheckDriveStatus::UpdateErrorDialog(driveStatus)) {
            if (!((int&)Sing_805DC948())) {
                if (!s_systemResetStateInitialized) {
                    SystemResetState_Ctor(s_systemResetStorage);
                    __register_global_object(s_systemResetStorage, (void*)SystemResetState_Dtor, lbl_8050A474);
                    s_systemResetStateInitialized = 1;
                }
                ((int&)Sing_805DC948()) = (int)s_systemResetStorage;
            }
            // A handled reset restarts the loop immediately, bypassing this sleep.
            if (SystemReset_ProcessPendingReset((void*)((int&)Sing_805DC948()), 0))
                continue;
            Engine_ClearFramebuffer(0, 0, 7, 255, 0, kFloat_1_DiskError);
            // Phase 3: draw the closing fade or the still-open drive-error dialog.
            if (((CCheckDriveStatus*)driveStatus)->dialogCloseRequested) { // 0x80281214: drive-status object +0x30
                if (!((int&)Sing_805DC608())) {
                    if (!s_fadeOverlayInitialized) {
                        FadeOverlay_Init(s_fadeOverlay);
                        __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, lbl_8050A48C);
                        s_fadeOverlayInitialized = 1;
                    }
                    ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
                }
                FadeState_Draw((void*)((int&)Sing_805DC608()));
            } else if (((CCheckDriveStatus*)driveStatus)->diskErrorDialog) {
                struct V {
                    virtual void a() DECOMP_LOCAL_VIRTUAL;
                    virtual void b() DECOMP_LOCAL_VIRTUAL;
                    virtual void draw() DECOMP_LOCAL_VIRTUAL;
                };
                ((V*)((CCheckDriveStatus*)driveStatus)->diskErrorDialog)->draw();
            }
            Engine_PresentFrame(0, 0, 0, 0);
        }
        {
            // Phase 4: throttle even when no frame was requested by the dialog update.
            OSSleepTicks((long long)((*(volatile unsigned int*)kBusClockRegister >> 2) / 1000) *
                         kDriveErrorFrameMilliseconds);
        }
    }
}
#pragma opt_strength_reduction reset

// Initialize the task's primary/secondary interfaces and clear both actor-skip masks.
CSaveProcessTask::CSaveProcessTask() {
    ((CTask*)this)->Init();
    *(void**)this = __vt__16CSaveProcessTask;
    *((void**)this + 1) = (char*)__vt__16CSaveProcessTask + kTaskFirstSecondaryVtableOffset;
    *((void**)this + 2) = (char*)__vt__16CSaveProcessTask + kTaskSecondSecondaryVtableOffset;
    g_defaultActorSkipMask = 0;
    g_actorSkipMask = 0;
}

// Destroy the native actor base and optionally free the save-process actor.
// Null is accepted; deleting > 0 frees self. Return is the original, possibly freed address.
CSaveProcessMain::~CSaveProcessMain() {
    if (this != 0) {
        __dt__18NativeBase80201D18Fv(this, 0);
    }
}

// Destroy the task base and optionally free the save-process task.
// Null is accepted; deleting > 0 frees self. Return is the original, possibly freed address.
CSaveProcessTask::~CSaveProcessTask() {
    __dt__5CTaskFv(this, 0);
}

// Store a terminated transition target name and report whether the original name fits in 32 bytes.
// taskName must be terminated; stores at most 31 bytes. Returns 1 for length <32,
// otherwise 0 after truncating. The incoming character pointer is not retained.
int SaveProcess_SetTransitionTargetName(const char* taskName) {
    memset(s_saveProcessTransitionTargetName, 0, kTaskNameCapacity);
    strncpy(s_saveProcessTransitionTargetName, taskName, kTaskNameMaxLength);
    unsigned int nameLength = strlen(taskName);
    // clz(length XOR 32) is 26 exactly for lengths 0..31: shifting 32 by
    // that amount sets bit 31. Preserve this compiled form of the fit test.
    return ((unsigned int)kTaskNameCapacity << __cntlzw(nameLength ^ (unsigned int)kTaskNameCapacity)) >> 31;
}

// Store a terminated follow-up task name and report whether the original name fits in 32 bytes.
// Same truncation/return/borrowed-pointer contract as SetTransitionTargetName.
int SaveProcess_SetFollowUpTaskName(const char* taskName) {
    memset(s_saveProcessFollowUpTaskName, 0, kTaskNameCapacity);
    strncpy(s_saveProcessFollowUpTaskName, taskName, kTaskNameMaxLength);
    unsigned int nameLength = strlen(taskName);
    // Same bit-31 fit test: 1 exactly for original lengths 0..31.
    return ((unsigned int)kTaskNameCapacity << __cntlzw(nameLength ^ (unsigned int)kTaskNameCapacity)) >> 31;
}

// Create the named save-process actor with an empty argument string and store its actor handle.
void CSaveProcessTask::CreateActors() {
    char allocatorByte;
    int allocatorScratch;
    LocalNString_u80280D18 actorArgument;
    char* argumentText = (char*)(unsigned int)s_saveProcessEmptyArgumentAndStringErrors;
    unsigned int argumentLength = strlen(argumentText);
    // Phase 1: build a temporary owned string. The allocator token has no known
    // payload here; its uninitialized byte copy preserves the original ABI code.
    String_ReserveCapacity(&actorArgument, argumentLength);
    allocatorByte = *(char*)&allocatorScratch;
    String_ReplaceRange(&actorArgument, 0, 0, argumentText, argumentText + argumentLength, &allocatorByte);
    // Phase 2: the factory consumes the argument during the call, then the local
    // string releases its storage on return. The task stores the returned handle.
    void* actor = TaskActorTable_CreateNamedActor(this, s_saveProcessActorClassName, s_saveProcessActorInstanceName, 0,
                                                  &actorArgument);
    ((CTask*)this)->SetMainActor(actor);
}
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804623E4", _ctors_u80280D18_0, SaveProcessMain_InitTauConstant);
STATIC_INIT_ENTRY(".CRT$XCU804623E4", _ctors_u80280D18_1, SaveProcessTask_InitTauConstant);
STATIC_INIT_ENTRY(".CRT$XCU804623E4", _ctors_u80280D18_2, DriveErrorLoop_InitTauConstant);
#pragma pop
