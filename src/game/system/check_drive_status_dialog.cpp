#include <decomp/local_class.h>
#include "classes/Context8028A588.h"
#include "classes/IDiskErrorDialog.h"
#define AsIDiskErrorDialog(p) ((IDiskErrorDialog*)(p))
// Translation unit 0x802813D0-0x80283608 (20 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CCheckDriveStatus.h"
#include <decomp/cw_bitfields.h>

#define AsCCheckDriveStatus(p) ((CCheckDriveStatus*)(p))
// Wide strings store short text inline at byte 2, or flags/length/heap pointer.
union DriveWideString {
    struct { unsigned flags; unsigned length; void* data; } heap;
    struct { int flags; int length; int data; } signedWords;
    struct { unsigned char size; unsigned char reserved; unsigned short text[5]; } small;
};
struct DriveStatusNode {
    void* left; void* right; void* parent;
    int driveStatus;
    DriveWideString message;
};
struct DriveStatusPair { int driveStatus; DriveWideString message; };
struct DriveErrorTextPool {
    char noDisc[0x3A]; char wrongDisc[0x40]; char retry[0x36];
};
typedef char DriveWideString_size[(sizeof(DriveWideString) == 12) ? 1 : -1];
typedef char DriveStatusNode_size[(sizeof(DriveStatusNode) == 0x1C) ? 1 : -1];
typedef char DriveErrorTextPool_size[(sizeof(DriveErrorTextPool) == 0xB0) ? 1 : -1];
enum DriveStatus {
    DriveFatalError = -1, DriveBusy = 1, DriveNoDisc = 4,
    DriveWrongDisc = 6, DriveRetry = 11
};
enum { kDrivePollMilliseconds = 33, kDriveDialogDelayMilliseconds = 1000,
       kDriveThreadStackBytes = 0xA000, kDriveThreadPriority = 5 };
enum DriveDialogMode { kNoDiscDialog = 0, kWrongDiscDialog = 1, kRetryDialog = 2, kUnclassifiedDialog = -1 };

enum DriveLayoutOffset {
    kStatusTreeRootOffset = (int)&((CCheckDriveStatus*)0)->statusTextMapRoot,
    kDriveThreadOffset = (int)&((CCheckDriveStatus*)0)->pad_038,
    kNodeKeyOffset = (int)&((DriveStatusNode*)0)->driveStatus,
    kNodeMessageOffset = (int)&((DriveStatusNode*)0)->message,
    kPairMessageOffset = (int)&((DriveStatusPair*)0)->message,
    kStatusTextOffset = (int)&((CCheckDriveStatus*)0)->statusTextFlags,
    kStatusInlineTextOffset = kStatusTextOffset + (int)&((DriveWideString*)0)->small.text
};
// Access the complete string through the first word exposed by the shared header.
inline DriveWideString* DriveStatusText(void* p) {
    return (DriveWideString*)&((CCheckDriveStatus*)p)->statusTextFlags;
}

extern "C" {
extern "C" const float kFloat_1_DiskError;
extern "C" const float kFloat_0_DiskErrorTween;
extern "C" void FloatTween_Start(void*, int, float, float);
extern "C" void appFreeUni__FPv(void*);
extern "C" void RenderNameTree_FreeSubtree(void*, void*);
extern "C" void __dl__FPv(void*);
extern "C" char __vt__16IDiskErrorDialog[];
struct VTable804B { char pad[0x20]; void (*f20)(void*); char pad2[4]; void (*f28)(void*); };
struct U80281D54Byte { CW_BITS2( unsigned char high:1, unsigned char low:7) };
struct U80281D54Task { 
virtual void destroy(int);
};
extern "C" void* CCheckDriveStatus_Ctor(void*);
extern "C" void* CCheckDriveStatus_dtor(char*, int);
extern "C" void __register_global_object(void*, void*, void*);
extern "C" char s_checkDriveStatus[];
extern "C" char driveErrorDialogDtorRegistrations[];
extern "C" signed char s_checkDriveStatusInitialized;
extern "C" void* g_driveErrorThreadStack;
extern "C" void __dla__FPv(void*);
extern "C" void OSCancelThread(void*);
extern "C" void* RenderNameTree_InsertNode(void*, void*, unsigned char, unsigned char, void*);
extern "C" int wcslen(void*);
extern "C" void WString_ReplaceRange(void*, int, int, void*, void*, void*);
extern "C" void* __nwa__FUl(unsigned);
extern "C" void Memory_Clear(void*, unsigned);
extern "C" void* OSGetCurrentThread();
extern "C" int OSCreateThread(void*, void*, void*, void*, unsigned, int, int);
extern "C" void OSResumeThread(void*);
extern "C" void DriveError_RunDisplayLoop(void*);
extern "C" char __vt__17CCheckDriveStatus[];
extern "C" char driveErrorMessageStrings[];
extern "C" int s_driveStatusMapFallbackId;
extern "C" int s_driveStatusTextKey6;
extern "C" int s_driveStatusTextKey11;
extern "C" float kFloat_0_04_DriveDialog;
extern "C" float kFloat_0_033333335_DriveDialog;
extern "C" signed char s_homeMenuInitialized;
extern "C" void* g_soundManager;
extern "C" signed char s_movieControllerInitialized;
extern "C" signed char s_systemResetStateInitialized;
extern "C" signed char s_inputManagerInitialized;
extern "C" char s_homeButtonResourceState[];
extern "C" char s_movieController[];
extern "C" char s_systemResetStorage[];
extern "C" char s_gameInputManager[];
extern "C" void HomeButtonMenu_InitializeState(void*);
extern "C" void HomeButtonResources_dtor(void*);
extern "C" int HomeButtonMenu_IsActive(void*);
extern "C" int DVDGetDriveStatus();
extern "C" int EventTiming_IsDurationScalingEnabled();
extern "C" void SoundManager_ResumePrimaryStreams(void*);
extern "C" void SoundManager_ResumePlayback(void*);
extern "C" void SoundManager_ApplySavedAudioSettings(void*, float);
extern "C" void MovieController_Ctor(void*);
extern "C" void MovieController_Dtor(void*);
extern "C" void MovieController_ReleasePlaybackReference(void*);
extern "C" void HomeMenu_OverrideBlockedState(void*, int);
extern "C" void SystemResetState_Ctor(void*);
extern "C" void SystemResetState_Dtor(void*);
extern "C" void FileIo_SetSystemButtonCallbacksEnabled(void*, int);
extern "C" void SoundManager_PausePrimaryStreams(void*);
extern "C" void SoundManager_PausePlayback(void*);
extern "C" void SoundManager_SetStreamVolume(void*, float, float);
extern "C" void MovieController_AcquirePlaybackReference(void*);
extern "C" void* Language_GetIndex();
extern "C" void CDiskErrorDialog_PAL_Ctor(void*, void*);
extern "C" int InputManager_Init(void*);
extern "C" void InputManager_Dtor(void*);
extern "C" void InputManager_StopAllRumble(void*);
extern "C" void gtl_InputEx_Update();
extern "C" void OSSuspendThread(void*);
extern "C" void* __nw__FUl(unsigned);

struct AllocatedTask {
    char bytes[0x38];
    // Construct the owned dialog with the language selector passed by the caller.
    AllocatedTask(void* arg) { CDiskErrorDialog_PAL_Ctor(this, arg); }
};
struct UpdateTask {
    virtual void destroy(int);
    virtual void update(float);
    virtual void slot4();
    virtual void slot5();
    virtual void setMode(int);
};
struct Triple828 { int a, b, c; };
// Capture the old three words in temp; clear p's flags/length and install temp->c.
// This is a reset/transfer operation, not a general swap: temp->a/b are ignored.
// The sole caller supplies three zero words, then frees any captured heap buffer.
inline void swap828(DriveWideString* p, Triple828* temp) {
    if (p != (DriveWideString*)temp) {
        Triple828 saved = *(Triple828*)p;
        p->signedWords.flags = 0;
        p->signedWords.length = 0;
        p->signedWords.data = temp->c;
        *temp = saved;
    }
}

// Lifecycle map:
// StartThread captures the game thread, allocates the display stack, and resumes
// the display thread. Update runs there: HOME-active -> no work; dialog-open ->
// service/close; otherwise mapped error -> accumulate delay -> open/suspend game.
// Open acquires the movie reference, saves/blocks HOME, pauses audio, stops rumble;
// fatal error also disables system-button callbacks. Close destroys the dialog,
// clears owned message storage, restores services, then resumes the game thread.
// Busy keeps an open dialog; unmapped non-busy requests closure on the next poll.
// StopThread/dtor cancel display first, free its stack, then destroy dialog/text
// and tree. They clear the saved game-thread pointer rather than running Close.
// Poll the DVD drive; pause the game for a delayed error dialog, then restore it when dismissed.
// Handshake: the display thread calls this with the singleton checker; suspendedThread
// is the game thread saved by StartThread, not the display thread. Opening suspends
// that game thread after pausing audio/movie/HOME activity. Closing restores those
// services and resumes it. The next poll performs close cleanup after a close request.
// Return 1 while a dialog is serviced/opened; 0 for HOME-menu suppression, delay,
// no mapped error, or completed cleanup. DriveBusy alone does not request closure.
int CCheckDriveStatus_UpdateErrorDialog(void* selfv) { char localSecond[8];
    Triple828 temp;
    char* text = driveErrorDialogDtorRegistrations;
    void* obj;
    // Phase 1: HOME owns the screen while active; do not advance error handling.
    if (!Sing_805DC820()) {
        if (!s_homeMenuInitialized) {
            HomeButtonMenu_InitializeState(s_homeButtonResourceState);
            __register_global_object(s_homeButtonResourceState, (void*)HomeButtonResources_dtor, text + 0x18);
            s_homeMenuInitialized = 1;
        }
        Sing_805DC820() = s_homeButtonResourceState;
    }
    if (HomeButtonMenu_IsActive(Sing_805DC820())) return 0;

    char* self = (char*)selfv;
    int driveStatus = DVDGetDriveStatus();

    char localFirst[8];
    // Phase 2: update an existing dialog. A requested close takes precedence over
    // reclassifying the drive status, and restores the saved game-thread state.
    if (AsCCheckDriveStatus(self)->diskErrorDialog) {
        float dt = EventTiming_IsDurationScalingEnabled() ? kFloat_0_04_DriveDialog : kFloat_0_033333335_DriveDialog;
        void* obj = AsCCheckDriveStatus(self)->diskErrorDialog;
        ((UpdateTask*)obj)->update(dt);
        if (AsCCheckDriveStatus(self)->dialogCloseRequested) {
            obj = AsCCheckDriveStatus(self)->diskErrorDialog;
            if (obj) {
                if (obj) ((UpdateTask*)obj)->destroy(1);
                AsCCheckDriveStatus(self)->diskErrorDialog = 0;
            }
            if (!(*(unsigned*)(self + kStatusTextOffset) >> 31)) {
                *(short*)(self + kStatusInlineTextOffset) = 0;
                { int _t = *(unsigned char*)(self + kStatusTextOffset); _t = _t & ~0x7F; *(self + kStatusTextOffset) = (char)_t; }
            } else {
                *(short*)AsCCheckDriveStatus(self)->statusTextData = 0;
                AsCCheckDriveStatus(self)->statusTextLength = 0;
            }
            {
                temp.a = 0; temp.b = 0; temp.c = 0;
                swap828((DriveWideString*)(self + kStatusTextOffset), &temp);
                if ((unsigned)temp.a >> 31) appFreeUni__FPv((void*)temp.c);
            }
            if (g_soundManager) {
                SoundManager_ResumePrimaryStreams(g_soundManager);
                SoundManager_ResumePlayback(g_soundManager);
                SoundManager_ApplySavedAudioSettings(g_soundManager, kFloat_1_DiskError);
            }
            if (!Sing_805DC950()) {
                if (!s_movieControllerInitialized) {
                    MovieController_Ctor(s_movieController);
                    __register_global_object(s_movieController, (void*)MovieController_Dtor, text + 0x0C);
                    s_movieControllerInitialized = 1;
                }
                Sing_805DC950() = s_movieController;
            }
            MovieController_ReleasePlaybackReference(Sing_805DC950());
            if (!Sing_805DC820()) {
                if (!s_homeMenuInitialized) {
                    HomeButtonMenu_InitializeState(s_homeButtonResourceState);
                    __register_global_object(s_homeButtonResourceState, (void*)HomeButtonResources_dtor, text + 0x18);
                    s_homeMenuInitialized = 1;
                }
                Sing_805DC820() = s_homeButtonResourceState;
            }
            HomeMenu_OverrideBlockedState(Sing_805DC820(), AsCCheckDriveStatus(self)->savedHomeMenuBlocked); // saved HOME-menu blocked state
            if (!Sing_805DC948()) {
                if (!s_systemResetStateInitialized) {
                    SystemResetState_Ctor(s_systemResetStorage);
                    __register_global_object(s_systemResetStorage, (void*)SystemResetState_Dtor, text + 0x24);
                    s_systemResetStateInitialized = 1;
                }
                Sing_805DC948() = s_systemResetStorage;
            }
            FileIo_SetSystemButtonCallbacksEnabled(Sing_805DC948(), 1);
            if ((*(void**)&AsCCheckDriveStatus(self)->suspendedThread)) OSResumeThread((*(void**)&AsCCheckDriveStatus(self)->suspendedThread));
            return 0;
        }

        // Match an exact status key via lower_bound; the embedded root slot is
        // the end sentinel. Copy the stored UTF-16 message without borrowing its buffer.
        void* searchNode = (*(void**)&AsCCheckDriveStatus(self)->statusTextMapRoot);
        void* matchedNode = self + kStatusTreeRootOffset;
        while (searchNode) {
            int key = ((DriveStatusNode*)searchNode)->driveStatus;
            if (key >= driveStatus) { matchedNode = searchNode; searchNode = ((DriveStatusNode*)searchNode)->left; }
            else searchNode = ((DriveStatusNode*)searchNode)->right;
        }
        if (matchedNode == self + kStatusTreeRootOffset || driveStatus < ((DriveStatusNode*)matchedNode)->driveStatus) matchedNode = self + kStatusTreeRootOffset;
        int found;
        if (matchedNode == self + kStatusTreeRootOffset) found = 0;
        else {
        if ((int)self + 0x14) {
            unsigned textFlags = *(unsigned*)(self + kStatusTextOffset);
            if (!(textFlags >> 31) && !(((DriveStatusNode*)matchedNode)->message.heap.flags >> 31)) {
                int a = ((DriveStatusNode*)matchedNode)->message.signedWords.flags;
                int b = ((DriveStatusNode*)matchedNode)->message.signedWords.length;
                *(int*)(self + kStatusTextOffset) = a;
                AsCCheckDriveStatus(self)->statusTextLength = b;
                (*(int*)&AsCCheckDriveStatus(self)->statusTextData) = ((DriveStatusNode*)matchedNode)->message.signedWords.data;
            } else {
                unsigned count;
                unsigned sourceLength;
                void* src;
                if (textFlags >> 31) count = (*(unsigned*)&AsCCheckDriveStatus(self)->statusTextLength);
                else count = *(unsigned char*)(self + kStatusTextOffset) & 0x7F;
                if (!(((DriveStatusNode*)matchedNode)->message.heap.flags >> 31)) {
                    src = ((DriveStatusNode*)matchedNode)->message.small.text;
                    sourceLength = ((DriveStatusNode*)matchedNode)->message.small.size & 0x7F;
                } else {
                    src = ((DriveStatusNode*)matchedNode)->message.heap.data;
                    sourceLength = ((DriveStatusNode*)matchedNode)->message.heap.length;
                }
                localSecond[4] = localSecond[0];
                WString_ReplaceRange(self + kStatusTextOffset, 0, count, src, (char*)src + sourceLength * 2, localSecond + 4);
            }
        }
        found = 1;
        }
        if (found) {
            int mode;
            switch (driveStatus) {
            case DriveNoDisc: mode = kNoDiscDialog; break;
            case DriveWrongDisc: mode = kWrongDiscDialog; break;
            case DriveRetry: mode = kRetryDialog; break;
            default: mode = kUnclassifiedDialog; break;
        }
            obj = AsCCheckDriveStatus(self)->diskErrorDialog;
            ((UpdateTask*)obj)->setMode(mode);
        } else if (driveStatus != DriveBusy) {
            AsCCheckDriveStatus(self)->dialogCloseRequested = 1;
        }
        return 1;
    }

    // Phase 3: without a dialog, only mapped errors accumulate the opening delay.
    // Each poll adds 33 ms (not measured elapsed time); skipDialogDelay bypasses it.
    if (AsCCheckDriveStatus(self)->errorDialogsEnabled) {
    void* searchNode = (*(void**)&AsCCheckDriveStatus(self)->statusTextMapRoot);
    void* matchedNode = self + kStatusTreeRootOffset;
    while (searchNode) {
        int key = ((DriveStatusNode*)searchNode)->driveStatus;
        if (key >= driveStatus) { matchedNode = searchNode; searchNode = ((DriveStatusNode*)searchNode)->left; }
        else searchNode = ((DriveStatusNode*)searchNode)->right;
    }
    if (matchedNode == self + kStatusTreeRootOffset || driveStatus < ((DriveStatusNode*)matchedNode)->driveStatus) matchedNode = self + kStatusTreeRootOffset;
    int found;
    if (matchedNode == self + kStatusTreeRootOffset) found = 0;
    else {
    if ((int)self + 0x14) {
        unsigned textFlags = *(unsigned*)(self + kStatusTextOffset);
        if (!(textFlags >> 31) && !(((DriveStatusNode*)matchedNode)->message.heap.flags >> 31)) {

            int b = ((DriveStatusNode*)matchedNode)->message.signedWords.length;
            int a = ((DriveStatusNode*)matchedNode)->message.signedWords.flags; *(int*)(self + kStatusTextOffset) = a;
            AsCCheckDriveStatus(self)->statusTextLength = b;
            (*(int*)&AsCCheckDriveStatus(self)->statusTextData) = ((DriveStatusNode*)matchedNode)->message.signedWords.data;
        } else {
            unsigned count;
            unsigned sourceLength;
            void* src;
            if (textFlags >> 31) count = (*(unsigned*)&AsCCheckDriveStatus(self)->statusTextLength);
            else count = *(unsigned char*)(self + kStatusTextOffset) & 0x7F;
            if (!(((DriveStatusNode*)matchedNode)->message.heap.flags >> 31)) {
                src = ((DriveStatusNode*)matchedNode)->message.small.text;
                sourceLength = ((DriveStatusNode*)matchedNode)->message.small.size & 0x7F;
            } else {
                src = ((DriveStatusNode*)matchedNode)->message.heap.data;
                sourceLength = ((DriveStatusNode*)matchedNode)->message.heap.length;
            }
            localFirst[4] = localFirst[0];
            WString_ReplaceRange(self + kStatusTextOffset, 0, count, src, (char*)src + sourceLength * 2, localFirst + 4);
        }
    }
    found = 1;
    }
    if (found) {
        if (!AsCCheckDriveStatus(self)->skipDialogDelay) {
            unsigned x = (*(unsigned*)&AsCCheckDriveStatus(self)->dialogDelayMilliseconds) + kDrivePollMilliseconds;
            (*(unsigned*)&AsCCheckDriveStatus(self)->dialogDelayMilliseconds) = x;
            if (x < kDriveDialogDelayMilliseconds) return 0;
        }
        // Phase 4: consume the delay and take over presentation before suspending
        // the saved game thread. Status-to-dialog modes: no disc=0, wrong=1, retry=2.
        AsCCheckDriveStatus(self)->dialogDelayMilliseconds = 0;
        if (g_soundManager) {
            SoundManager_PausePrimaryStreams(g_soundManager);
            SoundManager_PausePlayback(g_soundManager);
            SoundManager_SetStreamVolume(g_soundManager, kFloat_0_DiskErrorTween, kFloat_0_DiskErrorTween);
        }
        if (!Sing_805DC950()) {
            if (s_movieControllerInitialized == 0) {
                MovieController_Ctor(s_movieController);
                __register_global_object(s_movieController, (void*)MovieController_Dtor, text + 0x0C);
                s_movieControllerInitialized = 1;
            }
            Sing_805DC950() = s_movieController;
        }
        MovieController_AcquirePlaybackReference(Sing_805DC950());
        AsCCheckDriveStatus(self)->diskErrorDialog = new AllocatedTask(Language_GetIndex());
        int mode;
        switch (driveStatus) {
            case DriveNoDisc: mode = kNoDiscDialog; break;
            case DriveWrongDisc: mode = kWrongDiscDialog; break;
            case DriveRetry: mode = kRetryDialog; break;
            default: mode = kUnclassifiedDialog; break;
        }

        obj = AsCCheckDriveStatus(self)->diskErrorDialog; ((UpdateTask*)obj)->setMode(mode);
        AsCCheckDriveStatus(self)->dialogCloseRequested = 0;
        if (!Sing_805DC820()) {
            if (!s_homeMenuInitialized) {
                HomeButtonMenu_InitializeState(s_homeButtonResourceState);
                __register_global_object(s_homeButtonResourceState, (void*)HomeButtonResources_dtor, text + 0x18);
                s_homeMenuInitialized = 1;
            }
            Sing_805DC820() = s_homeButtonResourceState;
        }
        char* manager = (char*)Sing_805DC820();
        AsCCheckDriveStatus(self)->savedHomeMenuBlocked = (unsigned char)((Context8028A588*)manager)->homeMenuBlocked;
        if (!manager) {
            if (!s_homeMenuInitialized) {
                HomeButtonMenu_InitializeState(s_homeButtonResourceState);
                __register_global_object(s_homeButtonResourceState, (void*)HomeButtonResources_dtor, text + 0x18);
                s_homeMenuInitialized = 1;
            }
            Sing_805DC820() = s_homeButtonResourceState;
        }
        HomeMenu_OverrideBlockedState(Sing_805DC820(), 1);
        if (driveStatus == DriveFatalError) {
            if (!Sing_805DC948()) {
                if (!s_systemResetStateInitialized) {
                    SystemResetState_Ctor(s_systemResetStorage);
                    __register_global_object(s_systemResetStorage, (void*)SystemResetState_Dtor, text + 0x24);
                    s_systemResetStateInitialized = 1;
                }
                Sing_805DC948() = s_systemResetStorage;
            }
            FileIo_SetSystemButtonCallbacksEnabled(Sing_805DC948(), 0);
        }
        if (!Sing_805DC5C8()) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, text + 0x30);
                s_inputManagerInitialized = 1;
            }
            Sing_805DC5C8() = s_gameInputManager;
        }
        InputManager_StopAllRumble(Sing_805DC5C8());
        gtl_InputEx_Update();
        if ((*(void**)&AsCCheckDriveStatus(self)->suspendedThread)) OSSuspendThread((*(void**)&AsCCheckDriveStatus(self)->suspendedThread));
        return 1;
    }
    }
    AsCCheckDriveStatus(self)->dialogDelayMilliseconds = 0;
    return 0;
}

// Empty pre-overlay frame hook.
void Game_PreOverlayFrameHook() {}
// Empty disk-dialog lifecycle hook.
void IDiskErrorDialog_vfunc_4() {}
// Empty disk-dialog lifecycle hook.
void IDiskErrorDialog_vfunc_3() {}

// Default drive-status predicate returns false.
int DiskDialog_ShouldBlockUpdate() {
    return 0;
}

// Fade the disk-error dialog to zero over its configured tween duration.
void IDiskErrorDialog_StartFadeOut(void* p) {
    float f2 = AsIDiskErrorDialog(p)->tweenDuration;
    FloatTween_Start(AsIDiskErrorDialog(p)->tween, -1, kFloat_0_DiskErrorTween, f2);
}

// Immediately reset the disk-error dialog opacity to zero.
void IDiskErrorDialog_SetOpacityToZero(void* p) {
    float f1 = kFloat_0_DiskErrorTween;
    FloatTween_Start(AsIDiskErrorDialog(p)->tween, -1, f1, f1);
}

// Fade the disk-error dialog to full opacity over its configured tween duration.
void IDiskErrorDialog_StartFadeIn(void* p) {
    float f2 = AsIDiskErrorDialog(p)->tweenDuration;
    FloatTween_Start(AsIDiskErrorDialog(p)->tween, -1, kFloat_1_DiskError, f2);
}

// Initialize a one-second tween duration and start with zero opacity.
int IDiskErrorDialog_InitFade(void* p) {
    float f1 = kFloat_0_DiskErrorTween;
    AsIDiskErrorDialog(p)->tweenDuration = kFloat_1_DiskError;
    FloatTween_Start(AsIDiskErrorDialog(p)->tween, -1, f1, f1);
    return 1;
}

// Find the insertion parent and predecessor for a drive-status key in the message tree.
void* DriveStatusTextMap_FindInsertionPosition(void* map, void* statusKey, int* predecessor, char* insertLeft, char* beforeFirst) {
    int zero = 0;
    int one = 1;
    *predecessor = zero;
    char* mapBytes = (char*)map;
    void* parent = mapBytes + kStatusTreeRootOffset;
    void* node = (*(void**)&AsCCheckDriveStatus(mapBytes)->statusTextMapRoot);
    *insertLeft = (char)one;
    *beforeFirst = (char)one;
    while (node != 0) {
        int requestedStatus = *(int*)statusKey;
        parent = node;
        int nodeStatus = ((DriveStatusNode*)node)->driveStatus;
        if (requestedStatus < nodeStatus) {
            node = ((DriveStatusNode*)node)->left;
            *insertLeft = (char)one;
        } else {
            *predecessor = (int)node;
            node = ((DriveStatusNode*)node)->right;
            *insertLeft = (char)zero;
            *beforeFirst = (char)zero;
        }
    }
    return parent;
}

// Release a status/message pair, freeing wide-string heap storage when present.
void* DriveStatusPair_DestroyMessage(void* self, int flag) {
    if (self) {
        if ((int)self + kPairMessageOffset) {
            unsigned f = *(unsigned*)((unsigned char*)self + kPairMessageOffset);
            if (f >> 31) {
                appFreeUni__FPv(((DriveStatusPair*)self)->message.heap.data);
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Restore the base dialog vtable and run its fade-out and reset hooks before deletion.
void* IDiskErrorDialog_dtor(void* self, int flag) {
    if (self) {
        struct VObj804B {
            virtual void v0() DECOMP_LOCAL_VIRTUAL;
            virtual void v1() DECOMP_LOCAL_VIRTUAL;
            virtual void v2() DECOMP_LOCAL_VIRTUAL;
            virtual void v3() DECOMP_LOCAL_VIRTUAL;
            virtual void v4() DECOMP_LOCAL_VIRTUAL;
            virtual void v5() DECOMP_LOCAL_VIRTUAL;
            virtual void v6() DECOMP_LOCAL_VIRTUAL;
            virtual void v7() DECOMP_LOCAL_VIRTUAL;
            virtual void v8() DECOMP_LOCAL_VIRTUAL;
        };
        VObj804B* o = (VObj804B*)self;
        *(void***)self = (void**)__vt__16IDiskErrorDialog;
        o->v8();
        o->v6();
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Acquire the drive-status singleton and set its flag at byte 0x24; its consumer is unresolved.
// Thread shutdown and dialog completion clear the same latch; do not infer a hardware status from it.
// This setter performs no wakeup and does not change threadActive. In particular,
// setting this byte is not a substitute for StartThread or dialogCloseRequested.
void CCheckDriveStatus_SetLatch() {
    if (!Sing_805DCB20()) {
        if (!s_checkDriveStatusInitialized) {
            CCheckDriveStatus_Ctor(s_checkDriveStatus);
            __register_global_object(s_checkDriveStatus, (void*)CCheckDriveStatus_dtor, driveErrorDialogDtorRegistrations);
            s_checkDriveStatusInitialized = 1;
        }
        Sing_805DCB20() = (void*)s_checkDriveStatus;
    }
    AsCCheckDriveStatus(Sing_805DCB20())->unk_24 = 1;
}

// Acquire the singleton without changing the original volatile-load ordering.
inline void* GetDriveStatusInstance() {
    int missing = *(void* volatile*)&Sing_805DCB20() == 0;
    if (missing) {
        if (!s_checkDriveStatusInitialized) {
            CCheckDriveStatus_Ctor(s_checkDriveStatus);
            __register_global_object(s_checkDriveStatus, (void*)CCheckDriveStatus_dtor, driveErrorDialogDtorRegistrations);
            s_checkDriveStatusInitialized = 1;
        }
        Sing_805DCB20() = (void*)s_checkDriveStatus;
    }
    return Sing_805DCB20();
}

// Acquire the drive-status singleton and enable or suppress error dialogs.
void CCheckDriveStatus_SetErrorDialogsEnabled(int p) {
    AsCCheckDriveStatus(GetDriveStatusInstance())->errorDialogsEnabled = p;
}

// Acquire the drive-status singleton and control whether error dialogs wait before opening.
void CCheckDriveStatus_SetSkipDialogDelay(int p) {
    AsCCheckDriveStatus(GetDriveStatusInstance())->skipDialogDelay = p;
}

// Cancel the error-display thread, free its stack, and clear the active dialog and message.
void CCheckDriveStatus_StopThread() {
    if (!Sing_805DCB20()) {
        if (!s_checkDriveStatusInitialized) {
            CCheckDriveStatus_Ctor(s_checkDriveStatus);
            __register_global_object(s_checkDriveStatus, (void*)CCheckDriveStatus_dtor, driveErrorDialogDtorRegistrations);
            s_checkDriveStatusInitialized = 1;
        }
        Sing_805DCB20() = (void*)s_checkDriveStatus;
    }
    char* p = (char*)Sing_805DCB20();
    if (AsCCheckDriveStatus(p)->threadActive) {
    OSCancelThread(p + kDriveThreadOffset);
        if (g_driveErrorThreadStack) {
            __dla__FPv(g_driveErrorThreadStack);
            g_driveErrorThreadStack = 0;
        }
        AsCCheckDriveStatus(p)->suspendedThread = 0;
        AsCCheckDriveStatus(p)->threadActive = 0;
        AsCCheckDriveStatus(p)->unk_24 = 0;
        void* q = AsCCheckDriveStatus(p)->diskErrorDialog;
        if (q) {
            if (q) {
                ((U80281D54Task*)q)->destroy(1);
            }
            AsCCheckDriveStatus(p)->diskErrorDialog = 0;
        }
        unsigned v = AsCCheckDriveStatus(p)->statusTextFlags;
        if (!(v >> 31)) {
            DriveStatusText(p)->small.text[0] = 0;
            ((U80281D54Byte*)(DriveStatusText(p)))->low = 0;
        } else {
            void* r = AsCCheckDriveStatus(p)->statusTextData;
            *(short*)r = 0;
            AsCCheckDriveStatus(p)->statusTextLength = 0;
        }
    }
}

// Save the caller thread and start the dedicated error-display thread with a cleared stack.
// No-op while threadActive. The singleton owns g_driveErrorThreadStack (0xA000
// bytes); StopThread cancels the display thread before freeing that array.
// OSCreateThread receives p as its argument and the top of the allocated stack.
// The entry in save_process_main_task.cpp ignores the argument and reacquires the singleton;
// passing p here does not transfer ownership or make a per-instance thread API.
// Success resumes the display thread and sets threadActive; the caller is suspended
// later by UpdateErrorDialog, only after a dialog opens. Creation failure leaves
// the allocation in the global with threadActive clear; there is no rollback here.
void CCheckDriveStatus_StartThread() {
    if (!Sing_805DCB20()) {
        if (!s_checkDriveStatusInitialized) {
            CCheckDriveStatus_Ctor(s_checkDriveStatus);
            __register_global_object(s_checkDriveStatus, (void*)CCheckDriveStatus_dtor, driveErrorDialogDtorRegistrations);
            s_checkDriveStatusInitialized = 1;
        }
        Sing_805DCB20() = (void*)s_checkDriveStatus;
    }
    char* p = (char*)Sing_805DCB20();
    if (!AsCCheckDriveStatus(p)->threadActive) {
        void* q = AsCCheckDriveStatus(p)->diskErrorDialog;
        if (q) {
            if (q) {
                ((U80281D54Task*)q)->destroy(1);
            }
            AsCCheckDriveStatus(p)->diskErrorDialog = 0;
        }
        unsigned v = AsCCheckDriveStatus(p)->statusTextFlags;
        if (!(v >> 31)) {
            DriveStatusText(p)->small.text[0] = 0;
            ((U80281D54Byte*)(DriveStatusText(p)))->low = 0;
        } else {
            void* r = AsCCheckDriveStatus(p)->statusTextData;
            *(short*)r = 0;
            AsCCheckDriveStatus(p)->statusTextLength = 0;
        }
        AsCCheckDriveStatus(p)->suspendedThread = (int)OSGetCurrentThread();
        g_driveErrorThreadStack = __nwa__FUl(kDriveThreadStackBytes);
        Memory_Clear(g_driveErrorThreadStack, kDriveThreadStackBytes);
        unsigned res = OSCreateThread(p + kDriveThreadOffset, (void*)DriveError_RunDisplayLoop, p, (char*)g_driveErrorThreadStack + kDriveThreadStackBytes, kDriveThreadStackBytes, kDriveThreadPriority, 0);
        if (((-res | res) >> 31) != 0) {
            if (((-res | res) >> 31) != 0) {
                OSResumeThread(p + kDriveThreadOffset);
            }
            AsCCheckDriveStatus(p)->threadActive = 1;
        }
    }
}

struct U802813D0Temp {
    unsigned int textFlags;
    int sourceTextLength;
    int textData;
    int textLength;
    int resultId;
    unsigned int resultFlags;
    int resultLength;
    int resultData;
};
void* WString_ReserveCapacity();
union U813Marker { char c; int align; };
struct U813Locals { char a,b; char pad[2]; U813Marker copyDst; volatile U813Marker copySrc; U813Marker dst; volatile U813Marker src; };
struct U813Int { int v; };
// Return the inline or heap-backed UTF-16 character count.
inline unsigned DriveWideStringLength(void* p) {
    if (!(*(unsigned*)p >> 31)) return *(unsigned char*)p & 0x7f;
    return ((DriveWideString*)p)->heap.length;
}
// Copy an insertion temporary's heap string using the original scratch ABI.
// ReserveCapacity takes its destination through the recovered runtime convention;
// keep the call before reading source length/data and consuming the allocator tag.
#pragma push
#pragma always_inline on
inline void CopyHeapDriveInsertionText(U802813D0Temp& t, U813Locals& bits) {
    if (t.textFlags >> 31) {
        t.resultFlags = 0; t.resultLength = 0; t.resultData = 0;
        void* out = WString_ReserveCapacity();
        bits.copyDst.c = bits.copySrc.c;
        int n = t.sourceTextLength; void* data = (void*)t.textData;
        WString_ReplaceRange(out, 0, 0, data, (char*)data + n * 2, &bits.copyDst);
    }
}
#pragma pop

void* dtor_80281874(char*, int);
// Initialize drive-status monitoring and install no-disc, wrong-disc, and retry message entries.
void* CCheckDriveStatus_Ctor(void* selfv) { U802813D0Temp noDiscInsertion; U802813D0Temp wrongDiscInsertion;
    // Phase 1: initialize an empty message tree and inactive thread/dialog state.
    char* self = (char*)selfv; U802813D0Temp retryInsertion;
    int lookups[3];

    AsCCheckDriveStatus(self)->vtable = (void*)__vt__17CCheckDriveStatus;
    unsigned int zero = 0; AsCCheckDriveStatus(self)->statusTextMapCount = zero;
    AsCCheckDriveStatus(self)->statusTextMapRoot = zero;
    AsCCheckDriveStatus(self)->statusTextMapSentinel = (void*)(self + kStatusTreeRootOffset);

    (*(int*)&AsCCheckDriveStatus(self)->diskErrorDialog) = zero; (*(int*)&AsCCheckDriveStatus(self)->statusTextFlags) = zero;
    AsCCheckDriveStatus(self)->statusTextLength = zero;
    (*(int*)&AsCCheckDriveStatus(self)->statusTextData) = zero;
    AsCCheckDriveStatus(self)->threadActive = zero;
    AsCCheckDriveStatus(self)->unk_24 = zero;
    AsCCheckDriveStatus(self)->errorDialogsEnabled = 1;
    AsCCheckDriveStatus(self)->skipDialogDelay = zero;
    AsCCheckDriveStatus(self)->dialogCloseRequested = zero;
    AsCCheckDriveStatus(self)->dialogDelayMilliseconds = zero;
    AsCCheckDriveStatus(self)->suspendedThread = zero;
    AsCCheckDriveStatus(self)->savedHomeMenuBlocked = 0;

    try {
        // Phase 2a: insert/copy the no-disc text. Tree nodes own their wide strings;
        // the packed global strings and insertion-position temporaries are borrowed.
        {
            U813Locals bits;
            void* iter0 = DriveStatusTextMap_FindInsertionPosition(selfv, &s_driveStatusMapFallbackId, &lookups[2], &bits.a, &bits.b);
            int noDiscPredecessor = lookups[2];
            unsigned int cnt;
            char* node;
            // 0x802813D0: use the found status slot or insert its initial value.
            do {
                if (noDiscPredecessor) {

                    node = (char*)&((DriveStatusNode*)noDiscPredecessor)->driveStatus;
                    if (*(int*)node >= (s_driveStatusMapFallbackId)) break;
                }
                {

                    noDiscInsertion.textFlags = 0; noDiscInsertion.sourceTextLength = 0; noDiscInsertion.textData = 0;
                    noDiscInsertion.resultId = (s_driveStatusMapFallbackId);
                    noDiscInsertion.resultFlags = 0; noDiscInsertion.resultLength = 0; noDiscInsertion.resultData = 0;
                    CopyHeapDriveInsertionText(noDiscInsertion, bits);
                    node = (char*)&((DriveStatusNode*)RenderNameTree_InsertNode(selfv, iter0, bits.a, bits.b, (void*)&noDiscInsertion.resultId))->driveStatus;
                    if (noDiscInsertion.resultFlags >> 31) appFreeUni__FPv((void*)noDiscInsertion.resultData);
                    if (noDiscInsertion.textFlags >> 31) appFreeUni__FPv((void*)noDiscInsertion.textData);
                }
            } while (0);
            char* noDiscMessage = (char*)&((DriveStatusPair*)node)->message;
            void* base = (void*)((DriveErrorTextPool*)driveErrorMessageStrings)->noDisc;
            cnt = DriveWideStringLength(noDiscMessage);
            bits.dst.c = bits.src.c;
            int noDiscLength = wcslen(base);
            WString_ReplaceRange(noDiscMessage, 0, cnt, base, (char*)base + noDiscLength * 2, &bits.dst);
        }

        // Phase 2b: insert/copy the wrong-disc text under status key 6.
        { U813Locals bits;

            void* iter1 = DriveStatusTextMap_FindInsertionPosition(selfv, &s_driveStatusTextKey6, &lookups[1], &bits.a, &bits.b);

            int wrongDiscPredecessor = lookups[1];
            char* node2;
            do {
                if (wrongDiscPredecessor) {

                    node2 = (char*)&((DriveStatusNode*)wrongDiscPredecessor)->driveStatus;
                    if (*(int*)node2 >= (s_driveStatusTextKey6)) break;
                }
                {

                    wrongDiscInsertion.textFlags = 0; wrongDiscInsertion.sourceTextLength = 0; wrongDiscInsertion.textData = 0;
                    wrongDiscInsertion.resultId = (s_driveStatusTextKey6);
                    wrongDiscInsertion.resultFlags = 0; wrongDiscInsertion.resultLength = 0; wrongDiscInsertion.resultData = 0;
                    CopyHeapDriveInsertionText(wrongDiscInsertion, bits);
                    node2 = (char*)&((DriveStatusNode*)RenderNameTree_InsertNode(selfv, iter1, bits.a, bits.b, (void*)&wrongDiscInsertion.resultId))->driveStatus;
                    if (wrongDiscInsertion.resultFlags >> 31) appFreeUni__FPv((void*)wrongDiscInsertion.resultData);
                    if (wrongDiscInsertion.textFlags >> 31) appFreeUni__FPv((void*)wrongDiscInsertion.textData);
                }
            } while (0);
            char* wrongDiscMessage = (char*)&((DriveStatusPair*)node2)->message;
            unsigned int cnt2;
            void* base2 = (void*)((DriveErrorTextPool*)driveErrorMessageStrings)->wrongDisc; cnt2 = DriveWideStringLength(wrongDiscMessage);
            bits.dst.c = bits.src.c;
            int wrongDiscLength = wcslen(base2);
            WString_ReplaceRange(wrongDiscMessage, 0, cnt2, base2, (char*)base2 + (wrongDiscLength * 2), &bits.dst);
        }

        // Phase 2c: insert/copy the retry text under status key 11.
        {
            U813Locals bits;
            void* iter2 = DriveStatusTextMap_FindInsertionPosition(selfv, &s_driveStatusTextKey11, &lookups[0], &bits.a, &bits.b);
            char* node3;

            int retryPredecessor = lookups[0];
            do {
                if (retryPredecessor) {

                    node3 = (char*)&((DriveStatusNode*)retryPredecessor)->driveStatus;
                    if (*(int*)node3 >= (s_driveStatusTextKey11)) break;
                }
                {

                    retryInsertion.textFlags = 0; retryInsertion.sourceTextLength = 0; retryInsertion.textData = 0;
                    retryInsertion.resultId = (s_driveStatusTextKey11);
                    retryInsertion.resultFlags = 0; retryInsertion.resultLength = 0; retryInsertion.resultData = 0;
                    CopyHeapDriveInsertionText(retryInsertion, bits);
                    node3 = (char*)&((DriveStatusNode*)RenderNameTree_InsertNode(selfv, iter2, bits.a, bits.b, (void*)&retryInsertion.resultId))->driveStatus;
                    if (retryInsertion.resultFlags >> 31) appFreeUni__FPv((void*)retryInsertion.resultData);
                    if (retryInsertion.textFlags >> 31) appFreeUni__FPv((void*)retryInsertion.textData);
                }
            } while (0);
            char* retryMessage = (char*)&((DriveStatusPair*)node3)->message;
            void* base3 = (void*)((DriveErrorTextPool*)driveErrorMessageStrings)->retry;
            unsigned int cnt3; cnt3 = DriveWideStringLength(retryMessage);
            bits.dst.c = bits.src.c;
            unsigned int retryLength = wcslen(base3);
            WString_ReplaceRange(retryMessage, 0, cnt3, base3, (char*)base3 + 2 * retryLength, &bits.dst);
        }

        // Phase 3: finish with no current dialog/message; unwind releases owned text
        // and constructed tree state if any of the insertions above throws.
        void* q = AsCCheckDriveStatus(self)->diskErrorDialog;
        if (q) {
            if (q) {
                ((U80281D54Task*)q)->destroy(1);
            }
            AsCCheckDriveStatus(self)->diskErrorDialog = 0;
        }

        if (!((AsCCheckDriveStatus(self)->statusTextFlags) >> 31)) {

            DriveStatusText(self)->small.text[0] = 0;
            int flags = (*(unsigned char*)&AsCCheckDriveStatus(self)->statusTextFlags); flags &= ~0x7F;
            (*(unsigned char*)&AsCCheckDriveStatus(self)->statusTextFlags) = (unsigned char)flags;
        } else {
            void* r = AsCCheckDriveStatus(self)->statusTextData;
            *(short*)r = 0;
            AsCCheckDriveStatus(self)->statusTextLength = 0;
        }
    } catch (...) {
        if (AsCCheckDriveStatus(self)->statusTextFlags >> 31)
            appFreeUni__FPv(AsCCheckDriveStatus(self)->statusTextData);
        dtor_80281874(self, -1);
        throw;
    }
    return selfv;
}


// Destroy one status/message pair and release its tree node.
#pragma push
#pragma always_inline on
// Destroy a tree node's owned message, then release the node allocation.
inline void FreeDriveStatusNode(void* node) {
    if ((char*)node + kNodeKeyOffset) {
        if ((char*)node + kNodeMessageOffset) {
            if (((DriveStatusNode*)node)->message.heap.flags >> 31)
                appFreeUni__FPv(((DriveStatusNode*)node)->message.heap.data);
        }
    }
    appFreeUni__FPv(node);
}
// The external destructor handles deeper descendants; the top levels are expanded here.
inline void DestroyDriveStatusLeaf(char* owner, void* node) {
    void* left = ((DriveStatusNode*)node)->left;
    if (left) RenderNameTree_FreeSubtree(owner, left);
    void* right = ((DriveStatusNode*)node)->right;
    if (right) RenderNameTree_FreeSubtree(owner, right);
    FreeDriveStatusNode(node);
}
#pragma pop

#define U802_CLEAN_BRANCH(p) \
    do { \
        void* child0 = ((DriveStatusNode*)p)->left; \
        if (child0) DestroyDriveStatusLeaf(selfv, child0); \
        void* child1 = ((DriveStatusNode*)p)->right; \
        if (child1) DestroyDriveStatusLeaf(selfv, child1); \
        FreeDriveStatusNode(p); \
    } while (0)
#define U802_CLEAN_TREE(reset) \
    do { \
        register void* r31 = (*(void**)&AsCCheckDriveStatus(selfv)->statusTextMapRoot); \
        if (r31) { \
            register void* r30 = ((DriveStatusNode*)r31)->left; \
            if (r30) { \
                void* r27 = ((DriveStatusNode*)r30)->left; \
                if (r27) U802_CLEAN_BRANCH(r27); \
                void* r26 = ((DriveStatusNode*)r30)->right; \
                if (r26) U802_CLEAN_BRANCH(r26); \
                FreeDriveStatusNode(r30); \
            } \
            register void* r30b = ((DriveStatusNode*)r31)->right; \
            if (r30b) { \
                void* r26 = ((DriveStatusNode*)r30b)->left; \
                if (r26) U802_CLEAN_BRANCH(r26); \
                void* r26b = ((DriveStatusNode*)r30b)->right; \
                if (r26b) U802_CLEAN_BRANCH(r26b); \
                FreeDriveStatusNode(r30b); \
            } \
            FreeDriveStatusNode(r31); \
            if (reset) { \
                AsCCheckDriveStatus(selfv)->statusTextMapCount = 0; \
                AsCCheckDriveStatus(selfv)->statusTextMapRoot = 0; \
                AsCCheckDriveStatus(selfv)->statusTextMapSentinel = selfv + kStatusTreeRootOffset; \
            } \
        } \
    } while (0)

#pragma use_lmw_stmw on
// Cancel the display thread and release its stack, dialog, message, and message tree.
void* CCheckDriveStatus_dtor(char* selfv, int flag) {
    extern void OSCancelThread(void*);
    // Teardown phases: cancel display/stack -> release active dialog -> clear
    // message -> destroy map -> free message heap -> base map cleanup -> self.
    // The second dialog/map paths cover inactive-thread and base destruction.
    if (selfv != 0) {
        AsCCheckDriveStatus(selfv)->vtable = (void*)__vt__17CCheckDriveStatus;
        if (AsCCheckDriveStatus(selfv)->threadActive) {
            OSCancelThread(selfv + kDriveThreadOffset);
            if (g_driveErrorThreadStack) {
                __dla__FPv(g_driveErrorThreadStack);
                g_driveErrorThreadStack = 0;
            }
            void* q = AsCCheckDriveStatus(selfv)->diskErrorDialog;
            AsCCheckDriveStatus(selfv)->suspendedThread = 0;
            AsCCheckDriveStatus(selfv)->threadActive = 0;
            AsCCheckDriveStatus(selfv)->unk_24 = 0;
            if (q) {
                if (q) {
                    ((U80281D54Task*)q)->destroy(1);
                }
                AsCCheckDriveStatus(selfv)->diskErrorDialog = 0;
            }
            if (!(AsCCheckDriveStatus(selfv)->statusTextFlags >> 31)) {
                DriveStatusText(selfv)->small.text[0] = 0;
                ((U80281D54Byte*)(DriveStatusText(selfv)))->low = 0;
            } else {
                void* p = AsCCheckDriveStatus(selfv)->statusTextData;
                *(short*)p = 0;
                AsCCheckDriveStatus(selfv)->statusTextLength = 0;
            }
        }
        void* q = AsCCheckDriveStatus(selfv)->diskErrorDialog;
        if (q) {
            if (q) {
                ((U80281D54Task*)q)->destroy(1);
            }
            AsCCheckDriveStatus(selfv)->diskErrorDialog = 0;
        }
        if (!(AsCCheckDriveStatus(selfv)->statusTextFlags >> 31)) {
            DriveStatusText(selfv)->small.text[0] = 0;
            ((U80281D54Byte*)(DriveStatusText(selfv)))->low = 0;
        } else {
            void* p = AsCCheckDriveStatus(selfv)->statusTextData;
            *(short*)p = 0;
            AsCCheckDriveStatus(selfv)->statusTextLength = 0;
        }
        U802_CLEAN_TREE(1);
        if ((int)selfv + 0x14) {
            if (AsCCheckDriveStatus(selfv)->statusTextFlags >> 31) {
                appFreeUni__FPv(AsCCheckDriveStatus(selfv)->statusTextData);
            }
        }
        if (selfv) {
            if (selfv) {
                if (selfv) {
                    if (selfv) {
                        if (selfv) {
                            if (selfv) {
                                U802_CLEAN_TREE(0);
                            }
                        }
                    }
                }
            }
        }

        if (flag > 0) {
            __dl__FPv(selfv);
        }
    }
    return selfv;
}
#pragma use_lmw_stmw on

// Release the drive-status message tree during construction unwind or container destruction.
void* dtor_80281874(char* selfv, int flag) {
    if (selfv) {
        if (selfv) {
            if (selfv) {
                if (selfv) {
                    if (selfv) {
                        if (selfv) {
                            U802_CLEAN_TREE(0);
                        }
                    }
                }
            }
        }
        if (flag > 0) {
            __dl__FPv(selfv);
        }
    }
    return selfv;
}
#pragma use_lmw_stmw reset
#undef U802_CLEAN_TREE
#undef U802_CLEAN_BRANCH

} // extern "C"
