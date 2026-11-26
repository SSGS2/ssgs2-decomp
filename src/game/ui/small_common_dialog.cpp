// Small common dialog: a message box with YES/NO or single OK buttons, its shared
// resources, the HOME-button menu resources, and the file move access it uses.
#include <decomp/static_init.h>
typedef unsigned long new_size_t;
// Resource entries follow the original byte layout, including each terminating NUL.
struct SmallDialogStrings {
    char animationFile[0x13];
    char layoutFile[0x13];
    char messageGroup[6];
    char confirmSound[0x13];
    char cancelSound[0x17];
    char defaultScript[8];
    char padding[6];
};
#include "game/chatmsg_match.h"
typedef char SmallDialogStrings_size[(sizeof(SmallDialogStrings) == 100) ? 1 : -1];
// Dialog state: 1 requests the transition via RequestOpen; 2 reports completion; 4 accepts input.
// Result: 0 pending, 1 YES/OK, 2 NO. singleButtonMode: zero YES/NO, nonzero OK.
// Layout columns share an action: YES=0, NO=1, OK=2; +3 is the idle layer,
// +6 and +9 are feedback layers enabled after selection (their visual roles are unresolved).
enum SmallDialogState {
    DialogTransition = 1,
    DialogFinished = 2,
    DialogAcceptingInput = 4
};
enum SmallDialogResult {
    DialogNoResult = 0,
    DialogConfirmed = 1,
    DialogCancelled = 2
};
enum {
    kDialogYesButton = 0,
    kDialogNoButton = 1,
    kDialogOkButton = 2,
    kDialogButtonCount = 3,
    kDialogLayoutElementCount = 12,
    kDialogIdleLayerOffset = 3,
    kDialogFeedbackLayerAOffset = 6,
    kDialogFeedbackLayerBOffset = 9,
    kDialogTextureCount = 6,
    kDialogSelectionDelayFrames = 4,
    kHomeSoundArchiveBytes = 0x80000,
    kHomeSoundWorkBytes = 0xA0000
};
enum FileMoveState {
    MoveStart = 0,
    MoveWait = 1,
    MoveFailed = 2,
    MoveFinished = 3
};

#include "game/types.h"
#include "classes/CBpCtrlHandle.h"
#include "classes/CSmallCommonDialog.h"

#include "classes/ResourceState8028A270.h"
#define AsResourceState8028A270(p) ((ResourceState8028A270*)(p))

#define GAME_VECTOR_AGGREGATE

#include "game/vector.h"

// Stack-local storage view; external vector operations retain the original ABI.
struct LocalIntVector_u80288F50 {
    int* items;
    volatile unsigned int size;
    unsigned int capacity;
    // Reset the logical count and free the owned activated-element array.
    ~LocalIntVector_u80288F50() {
        if (items) {
            size = *(unsigned int*)&size - *(int*)&size;
            appFreeUni__FPv(items);
        }
    }
};

#include "game/singletons.h"
#include "classes/CFileIoAccessMoveFlash.h"
#include "classes/CFileIoAccessMove.h"
#include "classes/CFileIoAccess.h"
#include "classes/CFileIo.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CSharedFilePtr.h"
#include "classes/CInGameDialog.h"
#include "classes/CBpCtrl.h"
#include "classes/CBpTex.h"
#include "classes/CFileIoFlash.h"

#define AsCFileIoAccessMoveFlash(p) ((CFileIoAccessMoveFlash*)(p))
extern "C" {
void appFreeUni__FPv(void*);

// Narrow and wide strings share the heap flag and pointer words.
struct StringUnwindCleanup_u80288F50 {
    const unsigned int* words;
    bool active;
    // Arm cleanup for a completed inline/heap string until ownership is transferred.
    explicit StringUnwindCleanup_u80288F50(const void* string) : words((const unsigned int*)string), active(true) {}
    // Disarm cleanup after explicit ownership transfer or manual destruction.
    void release() {
        active = false;
    }
    // Free heap text on exit unless explicit release disarmed this cleanup.
    ~StringUnwindCleanup_u80288F50() {
        if (active && (words[0] & 0x80000000u)) {
            appFreeUni__FPv((void*)words[2]);
        }
    }
};

void Layout_SetElementPlayerFlagsAndState(void*, int, int);
void Layout_SetElementEnabled(void*, int, int);
void Layout_Update(void*);
int Layout_CollectActivated(void*, void**, int);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
unsigned long Runtime_ConvertDoubleToUnsigned(double);
void Layout_SetElementColor(void*, int, int);
void* IdMap_Lookup(void*, int);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void B2dGetInfoPtr(int, int*, int*);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);
void UIGroup_GetPosition(void*, void*);
void DialogText_DrawAligned(void*, void*, int, int, int);
void Layout_DrawWithOpacity(void*, float);
extern void* g_soundManager;
extern float kFloat_0_5_SmallDialogSound, kFloat_200_SmallDialogSound, kFloat_1_SmallCommonDialog;
extern const float kFloat_128_SmallDialog, kFloat_255_SmallDialog, kFloat_UIntConversionBias_SmallDialog;
extern void* g_dialogueSharedResourcesLoaded;
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
int CSharedFilePtr_GetLoadedBuffer(void*);
int Layout_PollLoadComplete(void*);
void BpObject_StartScriptByName(void*, const char*, int);
void UIGroup_SetEnabled(void*, int);
void WFixedFont_CreateMsgEcbResource(void*, void*, int, int);
void Layout_SetElementSecondaryState(void*, int, int);
extern void* g_chatMsg;
void* g_dialogueSharedResourcesLoaded;
int g_homeButtonIconTpl;
void* __dt__17CFileIoAccessMoveFv(void*, int);
void* __dl__FPv(void*);
void* __nw__FUl(unsigned long);
// Allocate storage through the game's scalar new entry point.
inline void* operator new(new_size_t size) {
    return __nw__FUl(size);
}
void operator delete(void*) throw();
void destroy__13CFileIoAccessFi(void*, int);
void appFreeUni__FPv(void*);
void Text_ExpandTaggedText(void*, void*, int, int);
void InGameDialog_SetWideText(void*, void*);
void* SharedFile_Create(const char*, int);
void String_ReplaceRange(void*, int, int, const char*, const char*, const char*);
void BpCtrl_SetOpacity(int, int);
// Forward HOME-menu sound cleanup, then destroy the system menu.
void HomeMenu_DeleteSound();
void HomeMenu_Delete();
void CBpCtrlHandle_Dtor();
void* __register_global_object(void*, void*, void*);
void Layout_Destroy(void*, int);
void __dt__15SmallDialogBaseFv(void*, int);
unsigned long strlen(const char*);
extern char __vt__18CSmallCommonDialog[];
extern signed char s_bpCtrlInitialized;
extern char s_homeMenuInputStorage[];
extern char s_bpCtrlDtorRegistration_SmallCommonDialog[];
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern char s_captionHandleDtorRegistration_SmallDialog[];
extern char s_bpCtrl[];
extern void CBpCtrl_dtor();
void Layout_Init(void*, int);
void InputManager_Dtor();
void* memcpy(void*, const void*, unsigned int);
void* InputManager_Init(void*);
void* InputManager_GetSelectedPlayer(void*);
void Layout_InitStandard(void*, void*, const char*, int, const char*);
void HomeMenu_LoadResourceBuffers(void*, void*);
void* appMallocUni__FUl(unsigned int);
// Create the system HOME menu from the prepared resource/configuration structure.
void HomeMenu_Create(void*);
char* strcpy(char*, const char*);
char* strcat(char*, const char*);
// Forward HOME-menu sound initialization with the archive and allocated work buffer.
void HomeMenu_CreateSound(void*, void*, unsigned int);
void RemoteAudio_RequestEnableAllLazy();
void Game_DrawReloadingFrame();
extern char zeroInitializationWords[];
// Twelve standard dialog layout descriptors; symbol renaming is coordinated tree-wide.
extern char smallCommonDialogLayoutDescriptors[];
extern char homeButtonResourceStrings[];
extern char s_inputManagerDtorRegistration_SmallDialog[];
extern char s_gameInputManager[];
extern signed char s_inputManagerInitialized;
extern const float kFloat_0_FileDialog;
extern int s_homeButtonSoundState[];

// Lazily construct the shared animation controller and register its static destructor once.
static inline void EnsureSmallDialogAnimationController() {
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_SmallCommonDialog);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

// Lazily construct the shared caption handle; dialogs borrow it and restore its script later.
static inline void EnsureSmallDialogCaptionHandle() {
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                     s_captionHandleDtorRegistration_SmallDialog);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
}

#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
// Copy both move paths into owned strings and start the file-access operation.
// Paths are borrowed NUL-terminated strings and copied before starting; null input returns 0.
// The return is the base start result, not completion; poll IsFinished and the failure flag.
int CFileIoAccessMove::SetPathsAndStart(const char* sourcePath, const char* destinationPath) {
    if (!sourcePath || !destinationPath)
        return 0;
    unsigned int oldLength;
    volatile int slot10a;
    volatile int slot14a;
    if ((firstPathFlags >> 31) == 0) {
        oldLength = (*(unsigned char*)&firstPathFlags) & 0x7f;
    } else {
        oldLength = (*(unsigned int*)&firstPathLength);
    }
    *(char*)&slot10a = *(char*)&slot14a;
    String_ReplaceRange(&firstPathFlags, 0, oldLength, sourcePath, sourcePath + strlen(sourcePath), (char*)&slot10a);
    if ((secondPathFlags >> 31) == 0) {
        oldLength = (*(unsigned char*)&secondPathFlags) & 0x7f;
    } else {
        oldLength = (*(unsigned int*)&secondPathLength);
    }
    volatile int slot8a;
    volatile int slot12a;
    *(char*)&slot8a = *(char*)&slot12a;
    String_ReplaceRange(&secondPathFlags, 0, oldLength, destinationPath, destinationPath + strlen(destinationPath),
                        (char*)&slot8a);
    return CFileIoAccess::CreateDefaultResource(this);
}
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs reset

// Release HOME-menu sound storage, icon texture, and BP entry.
void* HomeButtonResources_dtor(void* self, int deleting) {
    if (self) {
        HomeMenu_DeleteSound();
        HomeMenu_Delete();
        void* p = AsResourceState8028A270(self)->resourceBuffer;
        if (p) {
            appFreeUni__FPv(p);
            AsResourceState8028A270(self)->resourceBuffer = 0;
        }
        if (g_homeButtonIconTpl) {
            appFreeUni__FPv((void*)g_homeButtonIconTpl);
            g_homeButtonIconTpl = 0;
        }
        if (AsResourceState8028A270(self)->bpEntryId != -1) {
            if (!Sing_805DC5F0()) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_homeMenuInputStorage);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(AsResourceState8028A270(self)->bpEntryId);
            AsResourceState8028A270(self)->bpEntryId = -1;
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

struct Vt80289778 {
    virtual void f2(int);
    virtual void f1();
    virtual void f0();
};

// Release dialog layout and text resources, then restore the previous caption script.
void* CSmallCommonDialog::Destroy(int deleting) {
    if (this) {
        dialogVtable = &__vt__18CSmallCommonDialog;
        BpCtrl_SetOpacity(bpEntryId, 0x80);
        if (bpEntryId != -1) {
            EnsureSmallDialogAnimationController();
            ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(bpEntryId);
            bpEntryId = -1;
        }
        void* p = layout;
        if (p) {
            Layout_Destroy(p, 1);
            layout = 0;
        }
        p = textExpansionContext;
        if (p) {
            if (p) {
                ((Vt80289778*)p)->f2(1);
            }
            textExpansionContext = 0;
        }
        EnsureSmallDialogCaptionHandle();
        ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(savedScriptIndex);
        __dt__15SmallDialogBaseFv(this, 0);
        if (deleting > 0)
            __dl__FPv(this);
    }
    return this;
}
extern char smallCommonDialogStrings[];
extern int smallDialogFeedbackGroupNames[];
extern void* s_dialogueSharedResourcesRequested;
extern void* g_smallCommonDialogSharedFile;
extern void* g_dialogueSecondarySharedFile;
extern void* g_dialogueTextureResources[6];
extern void* dialogueTextureResourceNames[6];
extern float kFloat_1_FileDialog;
extern float kFloat_6_2831855;
extern float s_dialogueModuleFullTurnAngle0;
extern float s_dialogueModuleFullTurnAngle1;
extern float s_dialogueModuleFullTurnAngle2;
int __ptmf_test(void*);
int __ptmf_scall(void*, void*, void*);
extern char __vt__17CFileIoAccessMove[];

// Acquire the flash file-I/O singleton required by this move operation.
void* CFileIoAccessMoveFlash::CreateDevice() {
    return CFileIoFlash::AcquireInstance();
}

// The move-access initialization predicate always succeeds.
int CFileIoAccessMove::vfunc_2() {
    return 1;
}

// Return whether all shared dialog resources have loaded.
void* CSmallCommonDialog::IsSharedResourcesLoaded() {
    return g_dialogueSharedResourcesLoaded;
}

// Return the dialog result through the inherited pointer-sized ABI.
void* CSmallCommonDialog::GetDialogResult() {
    return (*(void**)&result);
}

// Initialize this module's cached 2-pi constant during static initialization.
void SmallCommonDialog_InitializeFullTurnAngle0() {
    s_dialogueModuleFullTurnAngle0 = kFloat_6_2831855;
}

// Initialize this module's cached 2-pi constant during static initialization.
void SmallCommonDialog_InitializeFullTurnAngle1() {
    s_dialogueModuleFullTurnAngle1 = kFloat_6_2831855;
}

// Initialize this module's cached 2-pi constant during static initialization.
void HomeButtonMenu_InitializeFullTurnAngle() {
    s_dialogueModuleFullTurnAngle2 = kFloat_6_2831855;
}

// Report whether the asynchronous move has reached its terminal state.
int CFileIoAccessMove::IsFinished() {
    int v = AsCFileIoAccessMoveFlash(this)->state;
    return v == MoveFinished;
}

// Destroy the base move access and optionally free the flash-derived object.
CFileIoAccessMoveFlash::~CFileIoAccessMoveFlash() {
    __dt__17CFileIoAccessMoveFv(this, 0);
}

// Set HOME-menu defaults, saved audio volumes, and blocked-notification flags.
void HomeButtonMenu_InitializeState(void* self) {
    float f0 = kFloat_1_FileDialog;
    int zero = 0;
    int one = 1;
    int noEntryId = -1;
    AsResourceState8028A270(self)->homeMenuState = zero;
    AsResourceState8028A270(self)->unk_B8 = (unsigned char)zero;
    AsResourceState8028A270(self)->homeMenuDisabled = (unsigned char)one;
    AsResourceState8028A270(self)->savedHomeMenuDisabled = (unsigned char)one;
    AsResourceState8028A270(self)->unk_BB = (unsigned char)zero;
    AsResourceState8028A270(self)->unk_BC = zero;
    AsResourceState8028A270(self)->unk_C0 = (unsigned char)zero;
    AsResourceState8028A270(self)->bpEntryId = noEntryId;
    AsResourceState8028A270(self)->savedStreamVolume = f0;
    AsResourceState8028A270(self)->savedBufferVolume = f0;
    AsResourceState8028A270(self)->unk_D0 = f0;
    AsResourceState8028A270(self)->homeMenuAdjustEnabled = (unsigned char)zero;
    AsResourceState8028A270(self)->soundStopDelayFrames = zero;
    AsResourceState8028A270(self)->blockedNotificationOutroStarted = (unsigned char)zero;
    AsResourceState8028A270(self)->blockedNotificationSuppressed = (unsigned char)one;
    (*(int*)&AsResourceState8028A270(self)->resourceBuffer) = zero;
    g_homeButtonIconTpl = zero;
}

typedef LocalIntVector_u80288F50 ActivatedElementList;

struct Pad80289BB0 {
    char _pad[4];
};
struct Vt80289BB0 : Pad80289BB0 {
    virtual void f0();
    virtual void f1();
    virtual void f2(int);
    virtual int f3();
};

// Query the virtual gate that enables countdown, layout updates, and input processing.
static inline int SmallDialogUpdateIsEnabled(void* self) {
    return ((Vt80289BB0*)self)->f3();
}

// Send lifecycle command 1 after the selected button has shown its feedback frames.
static inline void SmallDialogFinishSelectionFeedback(void* self) {
    ((Vt80289BB0*)self)->f2(1);
}

// Process activated buttons, record confirmation or cancellation, and delay closing for feedback.
void CSmallCommonDialog::UpdateInputAndFeedback() {
    // Phase 1: finish the feedback countdown before polling new input.
    if (SmallDialogUpdateIsEnabled(this)) {
        int previousDelayFrames = selectionDelayFrames;
        selectionDelayFrames = previousDelayFrames - 1;
        if (previousDelayFrames > 0 && previousDelayFrames - 1 == 0)
            SmallDialogFinishSelectionFeedback(this);
        // Phase 2: update layout and accept buttons only while input is enabled.
        Layout_Update(layout);
        if (g_dialogueSharedResourcesLoaded && dialogState == DialogAcceptingInput) {
            ActivatedElementList activated;
            activated.items = 0;
            activated.size = 0;
            activated.capacity = 0;
            if (Layout_CollectActivated(layout, (void**)&activated, 0)) {
                if (*activated.items == kDialogYesButton || *activated.items == kDialogOkButton) {
                    Sound_PlayByName(g_soundManager, ((SmallDialogStrings*)smallCommonDialogStrings)->confirmSound, 0,
                                     0, 0, 0, kFloat_0_5_SmallDialogSound, kFloat_200_SmallDialogSound,
                                     kFloat_1_SmallCommonDialog);
                    result = DialogConfirmed;
                } else if (*activated.items == kDialogNoButton) {
                    Sound_PlayByName(g_soundManager, ((SmallDialogStrings*)smallCommonDialogStrings)->cancelSound, 0, 0,
                                     0, 0, kFloat_0_5_SmallDialogSound, kFloat_200_SmallDialogSound,
                                     kFloat_1_SmallCommonDialog);
                    result = DialogCancelled;
                }
                // Phase 3: latch the result, disable the action, and show both feedback layers.
                if (result) {
                    selectionDelayFrames = kDialogSelectionDelayFrames;
                    Layout_SetElementPlayerFlagsAndState(layout, *activated.items, 0);
                    Layout_SetElementEnabled(layout, *activated.items + kDialogIdleLayerOffset, 0);
                    Layout_SetElementEnabled(layout, *activated.items + kDialogFeedbackLayerAOffset, 1);
                    Layout_SetElementEnabled(layout, *activated.items + kDialogFeedbackLayerBOffset, 1);
                }
            }
        }
    }
}

// Apply opacity to the animation and buttons, then draw the message at its animated group position.
void CSmallCommonDialog::Draw() {
    int bpOpacity = (int)(kFloat_128_SmallDialog * opacity);
    BpCtrl_SetOpacity(bpEntryId, bpOpacity);
    for (int i = 0; i < kDialogLayoutElementCount; ++i) {
        int elementAlpha =
            Runtime_ConvertDoubleToUnsigned(kFloat_255_SmallDialog * opacity + kFloat_UIntConversionBias_SmallDialog);
        Layout_SetElementColor(layout, i, elementAlpha);
    }
    EnsureSmallDialogAnimationController();
    void* bpController = Sing_805DC5F0();
    void* dialogObject = IdMap_Lookup(bpController, bpEntryId);
    BpObject_TryInvokeCurrentCallback(dialogObject, 0);
    BpObject_Draw(dialogObject);
    // Text position = animated group-local position + BP entry screen origin.
    int x = 0, y = 0;
    B2dGetInfoPtr(bpEntryId, &x, &y);
    int rect[2] = {0, 0};
    EnsureSmallDialogAnimationController();
    void* positionedDialog = IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
    UIGroup_GetPosition(
        BpObject_GetOrCreateNamedGroup(positionedDialog, ((SmallDialogStrings*)smallCommonDialogStrings)->messageGroup),
        rect);
    *(float*)rect = *(float*)rect + (float)x;
    *(float*)(rect + 1) = *(float*)(rect + 1) + (float)y;
    int elementAlpha =
        Runtime_ConvertDoubleToUnsigned(kFloat_255_SmallDialog * opacity + kFloat_UIntConversionBias_SmallDialog);
    DialogText_DrawAligned(this, rect, elementAlpha, 0, 0);
    Layout_DrawWithOpacity(layout, kFloat_1_SmallCommonDialog);
}

// Initialize the base file access, empty path strings, and initial move state.
CFileIoAccessMove::CFileIoAccessMove() {
    int zero = 0;
    void* vt = (void*)__vt__17CFileIoAccessMove;
    state = zero;
    vtbl = vt;
    (*(int*)&secondPathFlags) = zero;
    secondPathLength = zero;
    secondPathData = zero;
    (*(int*)&firstPathFlags) = zero;
    firstPathLength = zero;
    firstPathData = zero;
}

struct CallbackTarget {
    int f(void*, void*);
};
typedef int (CallbackTarget::*Pmf80289910)(void*, void*);

// The game data holds a 12-byte CodeWarrior member pointer.
struct DialogCallbackView {
    char _pad[0x50];
    void* resultArgument;
    Pmf80289910 memberFunction;
    CallbackTarget* target;
};

// Invoke the configured member callback; a false callback result blocks dialog completion.
int CSmallCommonDialog::InvokeResultCallback() {
    DialogCallbackView* callback = (DialogCallbackView*)this;
    if (__ptmf_test(&callbackThisAdjustment) != 0 && callback->target != 0 &&
        (callback->target->*callback->memberFunction)(callback->resultArgument, this) == 0) {
        return 0;
    }
    return 1;
}

// Allocate a small common dialog and store it through the required output pointer.
int CSmallCommonDialog::Create(void* buttonMode, void** out, int extra) {
    if (out == 0) {
        return 0;
    }
    CSmallCommonDialog* p = new CSmallCommonDialog(buttonMode, extra);
    *out = p;
    return p != 0 ? 1 : 0;
}

struct CSD_Pre_ {
    char pad[4];
};
class CSD_ : public CSD_Pre_ {
public:
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9(float);
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual int v15();
};
#pragma opt_common_subs off
// Advance dialog closing or input processing according to the inherited dialog state.
int CSmallCommonDialog::UpdateDialog(float value) {
    int state = dialogState;
    switch (state) {
    case DialogTransition:
        CInGameDialog::RequestOpen((InGameDialogView*)this, 1);
        break;
    case DialogFinished:
        return 1;
    case DialogAcceptingInput:
        ((CSD_*)this)->v13();
        /* fall through */
    default:
        return 0;
    }
    ((CSD_*)this)->v9(value);
    return ((CSD_*)this)->v15();
}
#pragma opt_common_subs reset

struct Vt80289414 {
    virtual void f2(int);
    virtual void f1();
    virtual void f0();
};

struct DialogTexture {
    char _pad[0x28];
    // Initialize one owned BP texture wrapper before requesting its resource.
    DialogTexture() {
        ((CBpTex*)this)->__ct();
    }
};

struct Vt8028934C {
    virtual void f0();
    virtual void f1(void*);
    virtual void f2();
};

// Request both shared dialog files and initialize the six texture resources.
void GameDialogue_LoadSharedResources() {
    g_smallCommonDialogSharedFile =
        SharedFile_Create(((SmallDialogStrings*)smallCommonDialogStrings)->animationFile, 0);
    g_dialogueSecondarySharedFile = SharedFile_Create(((SmallDialogStrings*)smallCommonDialogStrings)->layoutFile, 0);
    for (int i = 0; i < kDialogTextureCount; ++i) {
        DialogTexture* p = new DialogTexture();
        g_dialogueTextureResources[i] = p;
        ((Vt8028934C*)p)->f1(dialogueTextureResourceNames[i]);
    }
    s_dialogueSharedResourcesRequested = (void*)1;
}

// Release all requested shared dialog files and textures and clear the readiness flags.
void GameDialogue_ReleaseSharedResources() {
    if ((int)s_dialogueSharedResourcesRequested != 0) {
        if (g_smallCommonDialogSharedFile) {
            CSharedFileHandle::Destroy(g_smallCommonDialogSharedFile, 1);
            g_smallCommonDialogSharedFile = 0;
        }
        if (g_dialogueSecondarySharedFile) {
            CSharedFileHandle::Destroy(g_dialogueSecondarySharedFile, 1);
            g_dialogueSecondarySharedFile = 0;
        }
        for (int i = 0; i < kDialogTextureCount; ++i) {
            void* p = g_dialogueTextureResources[i];
            if (p) {
                if (p) {
                    ((Vt80289414*)p)->f2(1);
                }
                g_dialogueTextureResources[i] = 0;
            }
        }
        s_dialogueSharedResourcesRequested = 0;
        g_dialogueSharedResourcesLoaded = 0;
    }
}

// Free heap-backed move paths and destroy the base file access.
CFileIoAccessMove::~CFileIoAccessMove() {
    if ((int)this + 0x1c != 0) {
        unsigned int v = firstPathFlags;
        if (v >> 31) {
            void* p = (*(void**)&firstPathData);
            appFreeUni__FPv(p);
        }
    }
    if ((int)this + 0x10 != 0) {
        unsigned int v = secondPathFlags;
        if (v >> 31) {
            void* p = (*(void**)&secondPathData);
            appFreeUni__FPv(p);
        }
    }
    destroy__13CFileIoAccessFi(this, 0);
}

// Expand tagged message text and assign the resulting wide string to the dialog.
void CSmallCommonDialog::SetExpandedWideText(int param) {
    unsigned int local[3] = {0, 0, 0};
    StringUnwindCleanup_u80288F50 stringCleanup0(local);
    if (param != 0) {
        void* p = textExpansionContext;
        Text_ExpandTaggedText(p, local, param, 0);
    }
    void* r4 = (local[0] >> 31) ? (void*)local[2] : (char*)local + 2;
    InGameDialog_SetWideText(this, r4);
    if (local[0] >> 31) {
        {
            stringCleanup0.release();
            appFreeUni__FPv((void*)local[2]);
        }
    }
}

struct VtCsd802895B8 : Pad80289BB0 {
    virtual void f0();
    virtual void f1();
    virtual void f2();
    virtual void f3();
    virtual void f4();
    virtual void f5();
    virtual void f6();
    virtual void f7();
    virtual void f8();
    virtual void f9();
    virtual void f10();
    virtual void f11();
    virtual void f12();
    virtual void f13();
    virtual void f14();
    virtual void f15();
    virtual void f16();
    virtual void f17();
    virtual void f18(int);
    virtual void f19();
    virtual void f20();
    virtual void f21();
    virtual void f22();
    virtual void f23();
    virtual void f24();
};

#pragma opt_common_subs off
#pragma use_lmw_stmw on
// Wait for dialog resources, start the default script, initialize text, and release the caption entry.
void CSmallCommonDialog::LoadAndInitializeLayout() {
    // Phase 1: block until the layout and shared BP entries have finished loading.
    while (!Layout_PollLoadComplete(layout))
        Layout_Update(layout);
    do {
        EnsureSmallDialogAnimationController();
    } while (!((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded());
    EnsureSmallDialogAnimationController();
    // Phase 2: start the dialog script before hiding its named feedback groups.
    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), bpEntryId),
                               ((SmallDialogStrings*)smallCommonDialogStrings)->defaultScript, 1);
    // Resources are ready: start with all six named feedback groups hidden.
    int feedbackGroupNames[6];
    {
        int* groupNameSource = smallDialogFeedbackGroupNames;
        feedbackGroupNames[0] = *groupNameSource++;
        feedbackGroupNames[1] = *groupNameSource++;
        feedbackGroupNames[2] = *groupNameSource++;
        feedbackGroupNames[3] = *groupNameSource++;
        feedbackGroupNames[4] = *groupNameSource++;
        feedbackGroupNames[5] = *groupNameSource;
    }
    int* groupNameCursor = feedbackGroupNames;
    for (unsigned int i = 0; i < 6; ++i, ++groupNameCursor) {
        EnsureSmallDialogAnimationController();
        UIGroup_SetEnabled(
            BpObject_GetOrCreateNamedGroup(IdMap_Lookup(Sing_805DC5F0(), bpEntryId), (const char*)*groupNameCursor), 0);
    }
    // Phase 3: create the owned expansion context, populate text, then reset every element state.
    WFixedFont_CreateMsgEcbResource(((CChatMsg*)g_chatMsg)->defaultFixedFont, &textExpansionContext, 0, 0);
    ((VtCsd802895B8*)this)->f18(0);
    for (int i = 0; i < kDialogLayoutElementCount; ++i)
        Layout_SetElementSecondaryState(layout, i, 0);
    EnsureSmallDialogCaptionHandle();
    // Phase 4: save the shared caption script before releasing its entry; the destructor restores it.
    savedScriptIndex = ((CBpCtrlHandle*)Sing_805DC5F8())->scriptIndex;
    EnsureSmallDialogCaptionHandle();
    ((CBpCtrlHandle*)Sing_805DC5F8())->ReleaseEntry();
}
#pragma use_lmw_stmw reset
#pragma opt_common_subs reset

// Choose YES/NO or single-OK buttons and hide their inactive feedback layers.
void CSmallCommonDialog::ConfigureButtons() {
    int showOk;
    const int showYesNo = (singleButtonMode == 0);
    Layout_SetElementPlayerFlagsAndState(layout, kDialogYesButton, showYesNo);
    Layout_SetElementPlayerFlagsAndState(layout, kDialogNoButton, showYesNo);
    showOk = !showYesNo;
    Layout_SetElementPlayerFlagsAndState(layout, kDialogOkButton, showOk);
    Layout_SetElementEnabled(layout, kDialogYesButton + kDialogIdleLayerOffset, showYesNo);
    Layout_SetElementEnabled(layout, kDialogNoButton + kDialogIdleLayerOffset, showYesNo);
    Layout_SetElementEnabled(layout, kDialogOkButton + kDialogIdleLayerOffset, showOk);
    for (int i = kDialogFeedbackLayerAOffset; i < kDialogLayoutElementCount; ++i) {
        Layout_SetElementEnabled(layout, i, 0);
    }
}

struct Vt802894E0 {
    virtual void f0();
    virtual void f1();
    virtual void f2();
    virtual void f3();
    virtual int f4();
};

// Latch readiness only after both dialog files and all six textures are loaded.
void* GameDialogue_PollSharedResourcesLoaded() {
    if (g_dialogueSharedResourcesLoaded != 0) {
        return (void*)1;
    }
    if (s_dialogueSharedResourcesRequested == 0) {
        return 0;
    }
    g_dialogueSharedResourcesLoaded = (void*)1;
    if (CSharedFilePtr_GetLoadedBuffer(g_smallCommonDialogSharedFile) == 0) {
        g_dialogueSharedResourcesLoaded = 0;
    }
    if (CSharedFilePtr_GetLoadedBuffer(g_dialogueSecondarySharedFile) == 0) {
        g_dialogueSharedResourcesLoaded = 0;
    }
    for (int i = 0; i < kDialogTextureCount; ++i) {
        void* obj = g_dialogueTextureResources[i];
        if (((Vt802894E0*)obj)->f4() == 0) {
            g_dialogueSharedResourcesLoaded = 0;
        }
    }
    return g_dialogueSharedResourcesLoaded;
}

struct DialogLayout {
    char _pad[0x5c];
    // Construct the owned button layout with the caller's layout parameter.
    DialogLayout(int extra) {
        Layout_Init(this, extra);
    }
};

} // C linkage
#pragma opt_common_subs off
#pragma opt_propagation off
// Initialize the in-game dialog state, then the dialog fields, create its animation entry,
// and bind the selected player layout.
CSmallCommonDialog::CSmallCommonDialog(void* a, int extra) {
    CInGameDialog::Init((InGameDialogView*)this);
    int second;
    int first;
    int* src;
    int zero;
    dialogVtable = __vt__18CSmallCommonDialog;
    zero = 0;
    (*(void**)&singleButtonMode) = a;
    result = zero;
    src = (int*)zeroInitializationWords;
    first = *src++;
    second = *src++;
    (*(int*)&callbackThisAdjustment) = first;
    (*(int*)&callbackVtableOffset) = second;
    (*(int*)&callbackFunction) = *src;
    (*(int*)&callbackTarget) = zero;
    (*(int*)&layout) = zero;
    (*(int*)&textExpansionContext) = zero;
    bpEntryId = -1;
    selectionDelayFrames = zero;
    EnsureSmallDialogAnimationController();
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(((SmallDialogStrings*)smallCommonDialogStrings)->animationFile, &bpEntryId, 0);
    a = new DialogLayout(extra);
    layout = a;
    void* saved = GetInputManagerSlot();
    if (!saved) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     s_inputManagerDtorRegistration_SmallDialog);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    void* id = InputManager_GetSelectedPlayer(GetInputManagerSlot());
    Layout_InitStandard(layout, id, smallCommonDialogLayoutDescriptors, kDialogLayoutElementCount,
                        ((SmallDialogStrings*)smallCommonDialogStrings)->layoutFile);
    ((VtCsd802895B8*)this)->f21();
    ((VtCsd802895B8*)this)->f22();
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
extern "C" {

// Sound-table IDs occupy every other word. The companion words and prefix are unresolved.
struct HomeSoundLookupEntry {
    int archiveEntryId;
    int unresolvedCompanion;
};
struct HomeSoundLookupStorage {
    int unresolvedPrefix[0x30];
    HomeSoundLookupEntry firstGroup[8];
    HomeSoundLookupEntry secondGroup[4];
};
typedef char HomeSoundLookupStorage_secondGroup[((int)&((HomeSoundLookupStorage*)0)->secondGroup == 0x100) ? 1 : -1];

// Install the two recovered groups of archive entry IDs without touching companion words.
static inline void InitializeHomeSoundLookupEntries() {
    ((HomeSoundLookupStorage*)s_homeButtonSoundState)->secondGroup[0].archiveEntryId = 0;
    ((HomeSoundLookupStorage*)s_homeButtonSoundState)->secondGroup[1].archiveEntryId = 1;
    ((HomeSoundLookupStorage*)s_homeButtonSoundState)->secondGroup[2].archiveEntryId = 3;
    ((HomeSoundLookupStorage*)s_homeButtonSoundState)->secondGroup[3].archiveEntryId = 4;
    ((HomeSoundLookupStorage*)s_homeButtonSoundState)->firstGroup[0].archiveEntryId = 10;
    ((HomeSoundLookupStorage*)s_homeButtonSoundState)->firstGroup[1].archiveEntryId = 13;
    ((HomeSoundLookupStorage*)s_homeButtonSoundState)->firstGroup[2].archiveEntryId = 14;
    ((HomeSoundLookupStorage*)s_homeButtonSoundState)->firstGroup[3].archiveEntryId = 6;
    ((HomeSoundLookupStorage*)s_homeButtonSoundState)->firstGroup[4].archiveEntryId = 3;
    ((HomeSoundLookupStorage*)s_homeButtonSoundState)->firstGroup[5].archiveEntryId = 5;
    ((HomeSoundLookupStorage*)s_homeButtonSoundState)->firstGroup[6].archiveEntryId = 0;
    ((HomeSoundLookupStorage*)s_homeButtonSoundState)->firstGroup[7].archiveEntryId = 31;
}

// Load the HOME-button sound archive, allocate sound work memory, and enable remote audio.
// This blocks while drawing reload frames until the shared file is ready. The file handle
// is released after copying; archive/work buffers stay alive for HomeMenu_CreateSound/DeleteSound.
void HomeButton_InitializeSoundArchive(void* self) {
    struct {
        char empty;
        char pad[3];
        char path[256];
    } local;
    // Phase 1: reset four repeated float/float/word groups. Their consumer roles remain unresolved.
    AsResourceState8028A270(self)->unk_4C = 0;
    AsResourceState8028A270(self)->unk_44 = kFloat_0_FileDialog;
    AsResourceState8028A270(self)->unk_48 = kFloat_0_FileDialog;
    AsResourceState8028A270(self)->unk_54 = kFloat_0_FileDialog;
    AsResourceState8028A270(self)->unk_58 = kFloat_0_FileDialog;
    AsResourceState8028A270(self)->unk_5C = 0;
    AsResourceState8028A270(self)->unk_64 = kFloat_0_FileDialog;
    AsResourceState8028A270(self)->unk_68 = kFloat_0_FileDialog;
    AsResourceState8028A270(self)->unk_6C = 0;
    AsResourceState8028A270(self)->unk_74 = kFloat_0_FileDialog;
    AsResourceState8028A270(self)->unk_78 = kFloat_0_FileDialog;
    AsResourceState8028A270(self)->unk_7C = 0;
    // Phase 2: initialize HOME resources and replace the owned archive backing allocation.
    HomeMenu_LoadResourceBuffers(self, self);
    void* oldArchiveBacking = AsResourceState8028A270(self)->resourceBuffer;
    if (oldArchiveBacking) {
        if (oldArchiveBacking) {
            appFreeUni__FPv(oldArchiveBacking);
            AsResourceState8028A270(self)->resourceBuffer = 0;
        }
    }
    void* archiveBacking = appMallocUni__FUl(kHomeSoundArchiveBytes);
    AsResourceState8028A270(self)->resourceBuffer = archiveBacking;
    AsResourceState8028A270(self)->resourceBufferSize = kHomeSoundArchiveBytes;
    AsResourceState8028A270(self)->unk_3C = 0;
    (*(char*)&AsResourceState8028A270(self)->homeMenuAdjustEnabled) = 0;
    HomeMenu_Create(self);
    // Phase 3: install archive IDs; no per-channel/event meanings have been established.
    InitializeHomeSoundLookupEntries();
    // Phase 4: open the archive and keep rendering reload frames while waiting.
    local.empty = 0;
    strcpy(local.path, &local.empty);
    strcat(local.path, homeButtonResourceStrings);
    AsResourceState8028A270(self)->sharedFile = SharedFile_Create(local.path, 0);
    while (!CSharedFilePtr_GetLoadedBuffer(AsResourceState8028A270(self)->sharedFile)) {
        Game_DrawReloadingFrame();
    }
    // Phase 5: copy the loaded archive so sound setup does not retain the shared-file buffer.
    void* ownedArchiveCopy = 0;
    int archiveBytes = CSharedFilePtr::GetSize(AsResourceState8028A270(self)->sharedFile);
    ownedArchiveCopy = appMallocUni__FUl(archiveBytes);
    memcpy(ownedArchiveCopy, (void*)CSharedFilePtr_GetLoadedBuffer(AsResourceState8028A270(self)->sharedFile),
           archiveBytes);
    void* sharedArchiveHandle = AsResourceState8028A270(self)->sharedFile;
    if (sharedArchiveHandle) {
        CSharedFileHandle::Destroy(sharedArchiveHandle, 1);
        AsResourceState8028A270(self)->sharedFile = 0;
    }
    void* soundWorkMemory = appMallocUni__FUl(kHomeSoundWorkBytes);
    HomeMenu_CreateSound(ownedArchiveCopy, soundWorkMemory, kHomeSoundWorkBytes);
    RemoteAudio_RequestEnableAllLazy();
}

struct Vt8028904C {
    virtual int f0();
    virtual int f1();
    virtual int f2();
    virtual int f3();
    virtual int f4();
    virtual int f5();
    virtual int f8(void*, void*);
};

// Start the resource move, poll it, or mark failed access ready before finishing.
// Always returns 1 for a processed tick. A missing resource sets the failure flag and
// finishes in this tick; a rejected start does so on the next tick. MoveWait polls busy.
int CFileIoAccessMove::Update() {
    int fileResource = ((CFileIoAccess*)this)->GetResource();
    if (fileResource == 0) {
        AsCFileIoAccessMoveFlash(this)->state = MoveFailed;
    }
    int state = AsCFileIoAccessMoveFlash(this)->state;
    switch (state) {
    case MoveStart: {
        void* moveResource = (void*)fileResource;
        void* destinationPath;
        if ((AsCFileIoAccessMoveFlash(this)->secondPathFlags >> 31) == 0) {
            destinationPath = (char*)&AsCFileIoAccessMoveFlash(this)->secondPathFlags + 1;
        } else {
            destinationPath = AsCFileIoAccessMoveFlash(this)->secondPathData;
        }
        void* sourcePath;
        if ((AsCFileIoAccessMoveFlash(this)->firstPathFlags >> 31) == 0) {
            sourcePath = (char*)&AsCFileIoAccessMoveFlash(this)->firstPathFlags + 1;
        } else {
            sourcePath = AsCFileIoAccessMoveFlash(this)->firstPathData;
        }
        if (((Vt8028904C*)moveResource)->f8(sourcePath, destinationPath) == 0) {
            AsCFileIoAccessMoveFlash(this)->state = MoveFailed;
        } else {
            AsCFileIoAccessMoveFlash(this)->state = MoveWait;
        }
        break;
    }
    case MoveWait: {
        if (((CFileIo*)fileResource)->IsBusy() == 0) {
            AsCFileIoAccessMoveFlash(this)->state = MoveFinished;
        }
        break;
    }
    case MoveFailed: {
        ((CFileIoAccess*)this)->SetFailureFlag(1);
        AsCFileIoAccessMoveFlash(this)->state = MoveFinished;
        break;
    }
    default:
        break;
    }
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046240C", _ctors_u80288F50_0, SmallCommonDialog_InitializeFullTurnAngle0);
STATIC_INIT_ENTRY(".CRT$XCU8046240C", _ctors_u80288F50_1, SmallCommonDialog_InitializeFullTurnAngle1);
STATIC_INIT_ENTRY(".CRT$XCU8046240C", _ctors_u80288F50_2, HomeButtonMenu_InitializeFullTurnAngle);
#pragma pop
