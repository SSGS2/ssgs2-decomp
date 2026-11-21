#include <decomp/static_init.h>
// Translation unit 0x80293248-0x80293A54 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CCallback.h"

#define AsCCallback(p) ((CCallback*)(p))

enum SystemResetMode { kResetNone = -1, kResetRestart = 0, kResetPowerOff = 1, kResetReturnToMenu = 2 };
enum SystemResetPhase { kResetIdle = 0, kResetRequested = 1, kResetBlackout = 2 };
enum { kResetDelayFrames = 30, kDvdBusy = 1, kDvdNoDisc = 4, kDvdWrongDiskId = 6, kDvdDiskError = 11 };

// Prefix of s_systemResetStorage and the system-reset singleton.
struct SystemResetState {
    int mode;
    int phase;
    int framesRemaining;
    unsigned char buttonsEnabled;
    unsigned char deferred;
};

struct PrizeGalleryRecord {
    void* messageRecordOffset;
    short imageId;
};
extern "C" {

extern float kFloat_6_2831855;
extern float s_twoPi_CallbackUnit0;
extern float s_twoPi_CallbackUnit1;
extern float s_twoPi_SystemReset;
extern float s_twoPi_PrizeGallery;
extern float kSystemResetFramebufferClearValue;
extern unsigned char __vt__9CCallback[];
extern void* Table_GetEntry(void*, int);
extern void __dl__FPv(void*);
extern void OSSetResetCallback(void*);
extern void OSSetPowerCallback(void*);
extern void InputManager_Init(void*);
extern void InputManager_Dtor(void*);
extern void InputManager_StopAllRumble(void*);
extern void InputManager_Update(void*);
extern void gtl_InputEx_Update();
extern void __register_global_object(void*, void*, void*);
extern int s_cursorResourcesLoadAttempted;
extern signed char s_inputManagerInitialized;
extern unsigned char s_gameInputManager[];
extern unsigned char s_inputManagerDtorRegistration_80293248[];
extern unsigned char s_systemResetStorage[];
extern unsigned char s_systemResetDtorRegistration[];
extern signed char s_systemResetStateInitialized;
extern void* g_taskManager;
extern int CTask_CancelChildThread(void*);
extern void Engine_ClearFramebuffer(float, int, int, int, int, int);
extern void VISetBlack(int);
extern void VIFlush();
extern void VIWaitForRetrace();
extern void CCheckDriveStatus_SetErrorDialogsEnabled(int);
extern void FileRegistry_ClearOwnedDevices();
extern void InputDriver_InitStub();
extern void Projection_ResetDepthConvention();
extern void MediaRegistry_ResetGlobalPointer();
extern void fn_8034EB78(); // Retained platform cleanup hook, called before the DVD-idle wait.
extern int DVDGetDriveStatus();
extern void gtl_Audio_Shutdown();
extern void GXDrawDone();
extern void OSReturnToMenu();
extern void OSRestart(int);
extern void OSShutdownSystem();
void SystemReset_OnResetButton();
void SystemReset_OnPowerButton();
void* SystemResetState_Dtor(void*, int);

// Queue a restart when system buttons are enabled, then stop rumble and poll input.
void SystemReset_OnResetButton() {
    if (!((int&)Sing_805DC948())) {
        if (!s_systemResetStateInitialized) {
            ((SystemResetState*)s_systemResetStorage)->mode = kResetNone;
            SystemResetState* reset = (SystemResetState*)s_systemResetStorage;
            reset->phase = kResetIdle; reset->framesRemaining = kResetDelayFrames;
            reset->buttonsEnabled = 1; reset->deferred = 0;
            __register_global_object(reset, (void*)SystemResetState_Dtor, s_systemResetDtorRegistration);
            s_systemResetStateInitialized = 1;
        }
        ((int&)Sing_805DC948()) = (int)s_systemResetStorage;
    }
    SystemResetState* reset = (SystemResetState*)((int&)Sing_805DC948());
    if (!reset->buttonsEnabled) return;
    reset->mode = kResetRestart; reset->phase = kResetRequested; reset->framesRemaining = kResetDelayFrames;
    if (!s_cursorResourcesLoadAttempted) return;
    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) { InputManager_Init(s_gameInputManager); __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_80293248); s_inputManagerInitialized=1; }
        Sing_805DC5C8()=s_gameInputManager;
    }
    InputManager_StopAllRumble(Sing_805DC5C8());
    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) { InputManager_Init(s_gameInputManager); __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_80293248); s_inputManagerInitialized=1; }
        Sing_805DC5C8()=s_gameInputManager;
    }
    InputManager_Update(Sing_805DC5C8()); gtl_InputEx_Update();
}

// Queue power-off when system buttons are enabled, then stop rumble and poll input.
void SystemReset_OnPowerButton() {
    if (!((int&)Sing_805DC948())) {
        if (!s_systemResetStateInitialized) {
            ((SystemResetState*)s_systemResetStorage)->mode = kResetNone;
            SystemResetState* reset = (SystemResetState*)s_systemResetStorage;
            reset->phase = kResetIdle; reset->framesRemaining = kResetDelayFrames; reset->buttonsEnabled = 1; reset->deferred = 0;
            __register_global_object(reset,(void*)SystemResetState_Dtor,s_systemResetDtorRegistration); s_systemResetStateInitialized=1;
        }
        ((int&)Sing_805DC948())=(int)s_systemResetStorage;
    }
    SystemResetState* reset = (SystemResetState*)((int&)Sing_805DC948());
    if (!reset->buttonsEnabled) return;
    reset->mode=kResetPowerOff; reset->phase=kResetRequested; reset->framesRemaining=kResetDelayFrames;
    if (!s_cursorResourcesLoadAttempted) return;
    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) { InputManager_Init(s_gameInputManager); __register_global_object(s_gameInputManager,(void*)InputManager_Dtor,s_inputManagerDtorRegistration_80293248); s_inputManagerInitialized=1; }
        Sing_805DC5C8()=s_gameInputManager;
    }
    InputManager_StopAllRumble(Sing_805DC5C8());
    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) { InputManager_Init(s_gameInputManager); __register_global_object(s_gameInputManager,(void*)InputManager_Dtor,s_inputManagerDtorRegistration_80293248); s_inputManagerInitialized=1; }
        Sing_805DC5C8()=s_gameInputManager;
    }
    InputManager_Update(Sing_805DC5C8()); gtl_InputEx_Update();
}

// Black out the display, wait the reset delay, release I/O/audio, and perform the requested system action.
int SystemReset_ProcessPendingReset(void* self) {
    int phase = ((SystemResetState*)self)->phase;
    int canProcess;
    if (phase == kResetIdle) canProcess = 0;
    else canProcess = (((SystemResetState*)self)->deferred == 0);
    if (canProcess == 0) return 0;

    if (phase == kResetRequested) {
        if (g_taskManager) CTask_CancelChildThread(g_taskManager);
        Engine_ClearFramebuffer(kSystemResetFramebufferClearValue, 0, 0, 7, 0, 0);
        VISetBlack(1);
        VIFlush();
        VIWaitForRetrace();
        ((SystemResetState*)self)->phase = kResetBlackout;
    }
    if (((SystemResetState*)self)->framesRemaining > 0) {
        --((SystemResetState*)self)->framesRemaining;
        return 1;
    }

    CCheckDriveStatus_SetErrorDialogsEnabled(0);
    FileRegistry_ClearOwnedDevices();
    InputDriver_InitStub();
    Projection_ResetDepthConvention();
    MediaRegistry_ResetGlobalPointer();
    fn_8034EB78();
    while (DVDGetDriveStatus() == kDvdBusy) {}

    gtl_Audio_Shutdown();
    GXDrawDone();
    switch (((SystemResetState*)self)->mode) {
    case kResetRestart: {
        int driveStatus = DVDGetDriveStatus();
        switch (driveStatus) {
        case kDvdNoDisc:
        case kDvdWrongDiskId:
        case kDvdDiskError:
            OSReturnToMenu();
            break;
        default:
            OSRestart(1);
            break;
        }
        break;
    }
    case kResetPowerOff:
        OSShutdownSystem();
        break;
    case kResetReturnToMenu:
        OSReturnToMenu();
        break;
    }
    return 1;
}

// Initialize this translation-unit helper's two-pi cache.
void CallbackUnit_InitTwoPiCache0() { s_twoPi_CallbackUnit0 = kFloat_6_2831855; }
// Initialize the callback helper's separate two-pi cache.
void CallbackUnit_InitTwoPiCache1() { s_twoPi_CallbackUnit1 = kFloat_6_2831855; }
// Initialize the reset helper's separate two-pi cache.
void SystemReset_InitTwoPiCache() { s_twoPi_SystemReset = kFloat_6_2831855; }
// Initialize the trailing helper's separate two-pi cache.
void PrizeGallery_InitTwoPiCache() { s_twoPi_PrizeGallery = kFloat_6_2831855; }

// Invoke the registered callback with this object, its context, and the event argument; return zero if unset.
int CCallback_InvokeCallback(void* self, int arg) {
    int (*callback)(void*, void*, int) = AsCCallback(self)->callback;
    if (!callback) return 0;
    return callback(self, AsCCallback(self)->callbackContext, arg);
}

// Initialize an enabled reset controller with no pending request and a 30-frame delay.
void SystemResetState_Ctor(void* self) {
    SystemResetState* reset = (SystemResetState*)self;
    reset->mode = kResetNone;
    reset->phase = kResetIdle;
    reset->framesRemaining = kResetDelayFrames;
    ((SystemResetState*)self)->buttonsEnabled = 1;
    ((SystemResetState*)self)->deferred = 0;
}

// Reset request state and install the Wii reset/power button callbacks.
void SystemReset_InstallCallbacks(void* self) {
    SystemResetState* reset = (SystemResetState*)self;
    reset->mode = kResetNone;
    reset->phase = kResetIdle;
    ((SystemResetState*)self)->buttonsEnabled = 1;
    ((SystemResetState*)self)->deferred = 0;
    OSSetResetCallback((void*)SystemReset_OnResetButton);
    OSSetPowerCallback((void*)SystemReset_OnPowerButton);
}

// Set whether reset requests are accepted; enabling also installs both OS callbacks.
void FileIo_SetSystemButtonCallbacksEnabled(void* self, int enabled) {
    if (enabled) {
        OSSetResetCallback((void*)SystemReset_OnResetButton);
        OSSetPowerCallback((void*)SystemReset_OnPowerButton);
    }
    ((SystemResetState*)self)->buttonsEnabled = enabled;
}

// Queue the requested system action and stop controller rumble before shutdown begins.
void SystemReset_RequestDeferredMode(void* self, int value) {
    if (!((SystemResetState*)self)->buttonsEnabled) return;
    SystemResetState* reset = (SystemResetState*)self;
    reset->mode = value;
    reset->phase = kResetRequested;
    reset->framesRemaining = kResetDelayFrames;
    if (!s_cursorResourcesLoadAttempted) return;
    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_80293248);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = s_gameInputManager;
    }
    InputManager_StopAllRumble(Sing_805DC5C8());
    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_80293248);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = s_gameInputManager;
    }
    InputManager_Update(Sing_805DC5C8());
    gtl_InputEx_Update();
}

// Return the selected table record's message reference, or null when the index is absent.
void* PrizeGallery_FindMessageRecordOffset(const void* self, int index) {
    void* value = *(void**)self;
    void* result = Table_GetEntry(value, index);
    return result ? ((PrizeGalleryRecord*)result)->messageRecordOffset : 0;
}

// Return the selected record's signed image ID, or zero when the index is absent.
int PrizeGallery_GetImageId(const void* self, int index) {
    void* result = Table_GetEntry(*(void**)self, index);
    return result ? ((PrizeGalleryRecord*)result)->imageId : 0;
}

// Delete the callback allocation only when the destructor's deleting flag is positive.
void* dtor_80293270(void* self, int deleting) {
    if (self && deleting > 0) __dl__FPv(self);
    return self;
}

// Delete reset-controller storage only for a deleting destructor call.
void* SystemResetState_Dtor(void* self, int deleting) {
    if (self && deleting > 0) __dl__FPv(self);
    return self;
}

// Store the callback and context supplied as ABI-sized words.
int CCallback_SetCallbackAndContext(void* self, int a, int b) {
    AsCCallback(self)->callback = (int (*)(void*, void*, int))a;
    AsCCallback(self)->callbackContext = (void*)b;
    return 1;
}

// Install the callback vtable and clear the function/context pair.
void CCallback_Ctor(void* self) {
    int zero = 0;
    void* ptr = &__vt__9CCallback;
    AsCCallback(self)->callback = (int (*)(void*, void*, int))zero;
    AsCCallback(self)->vtbl = ptr;
    AsCCallback(self)->callbackContext = (void*)zero;
}

int CPrize_gallery_vfunc_1() { return 1; }
int CPrize_gallery_vfunc_2() { return 1; }
int CPrize_gallery_vfunc_3() { return 1; }
int CPrize_gallery_vfunc_4() { return 1; }
int CPrize_gallery_vfunc_5() { return 1; }
int CPrize_gallery_vfunc_6() { return 1; }
int CPrize_gallery_vfunc_7() { return 1; }
int CPrize_gallery_vfunc_8() { return 1; }
int CPrize_gallery_vfunc_9() { return 1; }
int CPrize_gallery_vfunc_10() { return 1; }
int CPrize_gallery_vfunc_11() { return 1; }
int CPrize_gallery_vfunc_12() { return 1; }

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046244C", _ctors_u80293248_0, CallbackUnit_InitTwoPiCache0);
STATIC_INIT_ENTRY(".CRT$XCU8046244C", _ctors_u80293248_1, CallbackUnit_InitTwoPiCache1);
STATIC_INIT_ENTRY(".CRT$XCU8046244C", _ctors_u80293248_2, SystemReset_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU8046244C", _ctors_u80293248_3, PrizeGallery_InitTwoPiCache);
#pragma pop
